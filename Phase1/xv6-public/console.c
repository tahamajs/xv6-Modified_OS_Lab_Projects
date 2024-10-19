// Console input and output.
// Input is from the keyboard or serial port.
// Output is written to the screen and serial port.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "traps.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

// Forward declaration of console output function.
static void console_output_char(int c);

static int panicked = 0;

// Console lock state
static struct {
    struct spinlock lock;
    int locking;
} console_lock_state;

// Copy buffer size
#define COPY_BUF_SIZE 256

// Structure to handle copy-paste functionality
struct {
    char buffer[COPY_BUF_SIZE]; // Buffer to store copied characters
    int is_copying;             // Flag to indicate if copying is active
    int index;                  // Index in the copy buffer
} copy_buffer;

// Function to print integer to console
static void print_integer(int value, int base, int is_signed) {
    static char digits[] = "0123456789abcdef"; // This array holds characters representing digits 0-9 and a-f for hexadecimal values.
    char buffer[16];  // A temporary buffer to store the characters of the number to be printed.
    int i;            // An index variable used to track the position in the buffer.
    uint x;           // Unsigned integer to hold the absolute value of the number being printed.

    // Check if the value is signed and negative, and if so, convert it to its positive counterpart.
    if (is_signed && (is_signed = value < 0))
        x = -value;
    else
        x = value;

    // Convert the value to the specified base (e.g., decimal, hexadecimal) by repeatedly dividing by the base.
    // The remainder of each division gives the next digit to be stored.
    i = 0;
    do {
        buffer[i++] = digits[x % base];  // Store the current digit (using the remainder) in the buffer.
    } while ((x /= base) != 0);  // Continue dividing x by the base until x becomes 0.

    // If the original value was negative, add the minus sign to the buffer.
    if (is_signed)
        buffer[i++] = '-';

    // Print the number from the buffer in reverse order (since the digits are stored in reverse).
    while (--i >= 0)
        console_output_char(buffer[i]);  // Output the digits to the console, one by one.
}


// Print to the console. Only understands %d, %x, %p, %s.
void cprintf(char* fmt, ...) {
    int i, c, locking;
    uint* argp;
    char* s;

    locking = console_lock_state.locking;
    if (locking)
        acquire(&console_lock_state.lock);

    if (fmt == 0)
        panic("null fmt");

    argp = (uint*)(void*)(&fmt + 1);
    for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
        if (c != '%') {
            console_output_char(c);
            continue;
        }
        c = fmt[++i] & 0xff;
        if (c == 0)
            break;
        switch (c) {
            case 'd':
                print_integer(*argp++, 10, 1);
                break;
            case 'x':
            case 'p':
                print_integer(*argp++, 16, 0);
                break;
            case 's':
                if ((s = (char*)*argp++) == 0)
                    s = "(null)";
                for (; *s; s++)
                    console_output_char(*s);
                break;
            case '%':
                console_output_char('%');
                break;
            default:
                // Print unknown % sequence to draw attention.
                console_output_char('%');
                console_output_char(c);
                break;
        }
    }

    if (locking)
        release(&console_lock_state.lock);
}

void panic(char* s) {
    int i;
    uint pcs[10];

    cli();
    console_lock_state.locking = 0;
    // Use lapicid so that we can call panic from mycpu()
    cprintf("lapicid %d: panic: ", lapicid());
    cprintf(s);
    cprintf("\n");
    getcallerpcs(&s, pcs);
    for (i = 0; i < 10; i++)
        cprintf(" %p", pcs[i]);
    panicked = 1; // Freeze other CPUs
    for (;;)
        ;
}

// Console constants and variables
#define BACKSPACE 0x100
#define CRTPORT   0x3d4
static ushort* crt = (ushort*)P2V(0xb8000); // CGA memory

// Cursor position: col + 80*row.
static int get_cursor_position(void) {
    int pos;
    outb(CRTPORT, 14);
    pos = inb(CRTPORT + 1) << 8;
    outb(CRTPORT, 15);
    pos |= inb(CRTPORT + 1);
    return pos;
}

// Sets cursor position
static void set_cursor_position(int pos) {
    outb(CRTPORT, 14);
    outb(CRTPORT + 1, pos >> 8);
    outb(CRTPORT, 15);
    outb(CRTPORT + 1, pos);
}

// Erases character at position
static void console_erase_character(int pos) {
    crt[pos] = ' ' | 0x0700;
}

// Writes character at position
static void console_write_character(int pos, int c) {
    crt[pos] = (c & 0xff) | 0x0700;
}

// Output character to CGA display
static void cga_put_character(int c) {
    int pos = get_cursor_position();

    if (c == '\n')
        pos += 80 - pos % 80;
    else if (c == BACKSPACE) {
        if (pos > 0)
            --pos;
    } else
        crt[pos++] = (c & 0xff) | 0x0700; // Black on white

    if (pos < 0 || pos > 25 * 80)
        panic("pos under/overflow");

    if ((pos / 80) >= 24) { // Scroll up.
        memmove(crt, crt + 80, sizeof(crt[0]) * 23 * 80);
        pos -= 80;
        memset(crt + pos, 0, sizeof(crt[0]) * (24 * 80 - pos));
    }

    set_cursor_position(pos);
    console_erase_character(pos);
}

// Output character to console
void console_output_char(int c) {
    if (panicked) {
        cli();
        for (;;)
            ;
    }

    if (c == BACKSPACE) {
        uartputc('\b');
        uartputc(' ');
        uartputc('\b');
    } else
        uartputc(c);
    cga_put_character(c);
}

#define INPUT_BUFFER_SIZE 128

// Input buffer structure
struct {
    char buffer[INPUT_BUFFER_SIZE];
    uint read_index;   // Read index
    uint write_index;  // Write index
    uint edit_index;   // Edit index
    uint cursor_shift; // Number of positions the cursor has been shifted to the left (>= 0)
} input_buffer;

// Write string to console and input buffer
void console_put_string(char* s) {
    for (int i = 0; i < INPUT_BUFFER_SIZE && s[i]; ++i) {
        input_buffer.buffer[input_buffer.edit_index++ % INPUT_BUFFER_SIZE] = s[i];
        console_output_char(s[i]);
    }
}

// Functions to move cursor
static void move_cursor_to_end(void) {
    set_cursor_position(get_cursor_position() + input_buffer.cursor_shift);
}

static void move_cursor_left(void) {
    set_cursor_position(get_cursor_position() - 1);
}

static void move_cursor_right(void) {
    set_cursor_position(get_cursor_position() + 1);
}

static void move_cursor_to_start(void) {
    input_buffer.cursor_shift = input_buffer.edit_index - input_buffer.write_index;
    set_cursor_position(get_cursor_position() - input_buffer.cursor_shift);
}

// Erase line and clear input buffer
static void console_erase_line(void) {
    move_cursor_to_end();
    input_buffer.cursor_shift = 0;

    while (input_buffer.edit_index != input_buffer.write_index &&
           input_buffer.buffer[(input_buffer.edit_index - 1) % INPUT_BUFFER_SIZE] != '\n') {
        input_buffer.edit_index--;
        console_output_char(BACKSPACE);
    }
}

// Erase terminal screen
static void console_clear_screen(void) {
    int pos = get_cursor_position();
    while (pos >= 0)
        console_erase_character(pos--);
}

// Print shell prompt
static void console_new_command_prompt(void) {
    console_write_character(0, '$');
    set_cursor_position(2);
}

// Shift input buffer left by one character
static void input_shift_left(void) {
    for (int i = input_buffer.cursor_shift + 1; i > 1; i--)
        input_buffer.buffer[(input_buffer.edit_index - i) % INPUT_BUFFER_SIZE] =
            input_buffer.buffer[(input_buffer.edit_index - i + 1) % INPUT_BUFFER_SIZE];

    input_buffer.edit_index--;
}

// Shift input buffer right by one character
static void input_shift_right(void) {
    for (int i = 0; i < input_buffer.cursor_shift; i++)
        input_buffer.buffer[(input_buffer.edit_index - i) % INPUT_BUFFER_SIZE] =
            input_buffer.buffer[(input_buffer.edit_index - i - 1) % INPUT_BUFFER_SIZE];
}

// Update console after input buffer has been modified (shift left)
static void console_shift_left(void) {
    move_cursor_to_end();
    for (int i = 0; i <= input_buffer.cursor_shift; i++)
        console_output_char(BACKSPACE);
    for (int i = input_buffer.cursor_shift; i > 0; i--)
        console_output_char(input_buffer.buffer[(input_buffer.edit_index - i) % INPUT_BUFFER_SIZE]);
    set_cursor_position(get_cursor_position() - input_buffer.cursor_shift);
}

// Update console after input buffer has been modified (shift right)
static void console_shift_right(void) {
    move_cursor_to_end();
    for (int i = 0; i < input_buffer.cursor_shift; i++)
        console_output_char(BACKSPACE);
    for (int i = input_buffer.cursor_shift; i >= 0; i--)
        console_output_char(input_buffer.buffer[(input_buffer.edit_index - i) % INPUT_BUFFER_SIZE]);
    set_cursor_position(get_cursor_position() - input_buffer.cursor_shift);
}

// Put character into input buffer
static void input_put_character(char c) {
    if (input_buffer.cursor_shift == 0) {
        input_buffer.buffer[input_buffer.edit_index % INPUT_BUFFER_SIZE] = c;
        console_output_char(c);
    } else {
        input_shift_right();
        input_buffer.buffer[(input_buffer.edit_index - input_buffer.cursor_shift) % INPUT_BUFFER_SIZE] = c;
        console_shift_right();
    }
    input_buffer.edit_index++;
}

// Function to delete numbers from input
static void delete_numbers_from_input(void) {
    char line[INPUT_BUFFER_SIZE];
    int line_index = 0;
    for (int input_index = 0; input_index < input_buffer.edit_index - input_buffer.write_index; input_index++) {
        int idx = (input_buffer.write_index + input_index) % INPUT_BUFFER_SIZE;
        if (input_buffer.buffer[idx] >= '0' && input_buffer.buffer[idx] <= '9')
            continue;
        line[line_index++] = input_buffer.buffer[idx];
    }
    line[line_index] = 0;
    console_erase_line();
    console_put_string(line);
}

// Command history buffer
#define COMMAND_HISTORY_SIZE 10

struct {
    char buffer[COMMAND_HISTORY_SIZE][INPUT_BUFFER_SIZE]; // Buffer to store commands
    int read_index;                   // Read index (range [1, COMMAND_HISTORY_SIZE])
    int write_index;                  // Write index
    int in_tab_mode;                  // Whether we are in tab completion mode
    char temp_command[INPUT_BUFFER_SIZE]; // Temporary command
    int last_used_index;              // Index of last used command
} command_history;

// Store command into history buffer
static void store_command(void) {
    // Shift existing commands to make room for the new command
    for (int i = COMMAND_HISTORY_SIZE - 1; i > 0; i--)
        memmove(command_history.buffer[i], command_history.buffer[i - 1], INPUT_BUFFER_SIZE);

    // Copy the new command into the buffer
    int j = 0;
    for (int i = input_buffer.write_index; i < input_buffer.edit_index; i++) {
        command_history.buffer[0][j] = input_buffer.buffer[i % INPUT_BUFFER_SIZE];
        j++;
    }
    for (; j < INPUT_BUFFER_SIZE; j++) {
        command_history.buffer[0][j] = 0;
    }
    if (command_history.write_index < COMMAND_HISTORY_SIZE)
        command_history.write_index++;
}

// Load command from history buffer
static void load_command(void) {
    console_erase_line();
    int n = command_history.read_index - 1;
    input_buffer.edit_index = input_buffer.write_index;
    for (int i = 0; i < INPUT_BUFFER_SIZE; i++) {
        if (command_history.buffer[n][i] == '\0')
            break;
        input_buffer.buffer[input_buffer.edit_index++ % INPUT_BUFFER_SIZE] = command_history.buffer[n][i];
        console_output_char(command_history.buffer[n][i]);
    }
}

// Copy current command
static void copy_current_command(void) {
    int j = 0;
    for (int i = input_buffer.write_index; i < input_buffer.edit_index; i++) {
        command_history.temp_command[j] = input_buffer.buffer[i % INPUT_BUFFER_SIZE];
        j++;
    }
    for (; j < INPUT_BUFFER_SIZE; j++) {
        command_history.temp_command[j] = 0;
    }
}

// Recover command from temporary storage
static void recover_command(void) {
    console_erase_line();
    input_buffer.edit_index = input_buffer.write_index;
    for (int i = 0; i < INPUT_BUFFER_SIZE; i++) {
        if (command_history.temp_command[i] == 0)
            break;
        input_buffer.buffer[input_buffer.edit_index++ % INPUT_BUFFER_SIZE] = command_history.temp_command[i];
        console_output_char(command_history.temp_command[i]);
    }
}

// Check if cmd is a prefix of input
static int is_prefix(const char* cmd, const char* input, int input_size) {
    for (int i = 0; i < input_size; i++)
        if (cmd[i] != input[i])
            return 0;
    return 1;
}

// Get predicted command index based on input
static int get_predicted_command_index(const char* cmd, uint cmd_size, int last_used_index) {
    for (int i = last_used_index; i < command_history.write_index; i++) {
        if (is_prefix(command_history.buffer[i], cmd, cmd_size))
            return i;
    }
    return -1;
}

// Predict command based on current input
static void predict_command(void) {
    int predicted_cmd_idx = -1;
    if (!command_history.in_tab_mode) {
        command_history.last_used_index = 0;
        predicted_cmd_idx = get_predicted_command_index(
            input_buffer.buffer + input_buffer.write_index,
            input_buffer.edit_index - input_buffer.write_index,
            command_history.last_used_index
        );
        copy_current_command();
    } else {
        predicted_cmd_idx = get_predicted_command_index(
            input_buffer.buffer + input_buffer.write_index,
            input_buffer.edit_index - input_buffer.write_index,
            command_history.last_used_index + 1
        );
    }

    if (predicted_cmd_idx >= 0) {
        command_history.in_tab_mode = 1;
        command_history.last_used_index = predicted_cmd_idx;
        console_erase_line();
        console_put_string(command_history.buffer[predicted_cmd_idx]);
    }
}

// Reset command history
static void reset_command_history(void) {
    command_history.in_tab_mode = 0;
    command_history.temp_command[0] = '\0';
    command_history.last_used_index = 0;
    command_history.write_index = ((command_history.write_index + 1) > COMMAND_HISTORY_SIZE ? COMMAND_HISTORY_SIZE : (command_history.write_index + 1));
    store_command();
    command_history.read_index = 0;
    move_cursor_to_end();
}

// Control key definitions
#define CTRL(x)       ((x) - '@') // Control-x
#define SHIFT(x)       ((x) + ' ') // Shift-x
#define ARROW_UP   226
#define ARROW_DOWN 227
#define ARROW_LEFT 228
#define ARROW_RIGHT 229
#define TAB        '\t'

// Simple strcmp implementation
static int string_compare(const char *s1, const char *s2) {
    while(*s1 && *s1 == *s2){
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

// Evaluate expression from end of input buffer
static int evaluate_expression_from_end(int end_idx, int* result) {
    int num1 = 0, num2 = 0;
    char op = 0;
    int idx = end_idx;

    int multiplier = 1;
    while (idx >= input_buffer.write_index && input_buffer.buffer[idx % INPUT_BUFFER_SIZE] >= '0' && input_buffer.buffer[idx % INPUT_BUFFER_SIZE] <= '9') {
        num2 += (input_buffer.buffer[idx % INPUT_BUFFER_SIZE] - '0') * multiplier;
        multiplier *= 10;
        idx--;
    }

    if (idx >= input_buffer.write_index && (input_buffer.buffer[idx % INPUT_BUFFER_SIZE] == '+' || input_buffer.buffer[idx % INPUT_BUFFER_SIZE] == '-' ||
                               input_buffer.buffer[idx % INPUT_BUFFER_SIZE] == '*' || input_buffer.buffer[idx % INPUT_BUFFER_SIZE] == '/')) {
        op = input_buffer.buffer[idx % INPUT_BUFFER_SIZE];
        idx--;
    } else {
        return 0; // Not a valid operator
    }

    multiplier = 1;
    while (idx >= input_buffer.write_index && input_buffer.buffer[idx % INPUT_BUFFER_SIZE] >= '0' && input_buffer.buffer[idx % INPUT_BUFFER_SIZE] <= '9') {
        num1 += (input_buffer.buffer[idx % INPUT_BUFFER_SIZE] - '0') * multiplier;
        multiplier *= 10;
        idx--;
    }

    if (multiplier == 1) {
        return 0;
    }

    switch (op) {
        case '+':
            *result = num1 + num2;
            break;
        case '-':
            *result = num1 - num2;
            break;
        case '*':
            *result = num1 * num2;
            break;
        case '/':
            if (num2 == 0)
                return 0;
            *result = num1 / num2;
            break;
        default:
            return 0;
    }

    return end_idx - idx;
}

// Process input buffer and replace 'N O N=?' patterns with result
static void process_input_buffer(void) {
    int i = input_buffer.edit_index - 1; // Start from the end of the input buffer
    int write_idx = input_buffer.write_index;
    while (i >= write_idx) {
        // Check for the pattern '=?' at the end
        if (input_buffer.buffer[i % INPUT_BUFFER_SIZE] == '?' && input_buffer.buffer[(i - 1) % INPUT_BUFFER_SIZE] == '=') {
            int expr_end = i - 2; // The position before '='
            int result = 0;
            int expr_len = evaluate_expression_from_end(expr_end, &result);
            if (expr_len > 0) {
                // Remove the 'N O N=?' from input buffer
                int total_len = expr_len + 2; // +2 for '=?'
                int start_pos = expr_end - expr_len + 1; // Start position of the expression

                // Shift the remaining buffer left to remove the expression
                for (int j = start_pos; j < input_buffer.edit_index - total_len; j++) {
                    input_buffer.buffer[j % INPUT_BUFFER_SIZE] = input_buffer.buffer[(j + total_len) % INPUT_BUFFER_SIZE];
                }
                input_buffer.edit_index -= total_len;

                // Convert result to string
                char res_str[16];
                int res_len = 0;
                int temp_result = result;
                if (result == 0) {
                    res_str[res_len++] = '0';
                } else {
                    if (result < 0) {
                        res_str[res_len++] = '-';
                        temp_result = -temp_result;
                    }
                    int digits[10];
                    int num_digits = 0;
                    while (temp_result > 0) {
                        digits[num_digits++] = temp_result % 10;
                        temp_result /= 10;
                    }
                    for (int k = num_digits - 1; k >= 0; k--) {
                        res_str[res_len++] = '0' + digits[k];
                    }
                }

                // Shift input buffer to make space for result
                int shift_amount = res_len;
                for (int j = input_buffer.edit_index - 1; j >= start_pos; j--) {
                    input_buffer.buffer[(j + shift_amount) % INPUT_BUFFER_SIZE] = input_buffer.buffer[j % INPUT_BUFFER_SIZE];
                }
                input_buffer.edit_index += shift_amount;

                // Insert result into input buffer
                for (int j = 0; j < res_len; j++) {
                    input_buffer.buffer[(start_pos + j) % INPUT_BUFFER_SIZE] = res_str[j];
                }

                input_buffer.cursor_shift = 0;

                // Adjust 'i' to continue processing correctly
                i = start_pos + res_len - 1;
            }
        }
        i--;
    }
}

// Console interrupt handler
void consoleintr(int (*getc)(void)) {
    int c, doprocdump = 0;

    acquire(&console_lock_state.lock);
    while ((c = getc()) >= 0) {
        switch (c) {
            case CTRL('P'): // Process listing.
                doprocdump = 1;
                break;

            case CTRL('U'): // Kill line.
                console_erase_line();
                break;

            case CTRL('H'):
            case '\x7f': // Backspace
                if ((input_buffer.edit_index - input_buffer.cursor_shift) > input_buffer.write_index) {
                    input_shift_left();
                    console_shift_left();
                }
                break;

            case CTRL('L'): // Clear screen.
                console_erase_line();
                console_clear_screen();
                console_new_command_prompt();
                break;

            case ARROW_LEFT:
                if (input_buffer.cursor_shift < input_buffer.edit_index - input_buffer.write_index) {
                    input_buffer.cursor_shift++;
                    move_cursor_left();
                }
                break;

            case ARROW_RIGHT:
                if (input_buffer.cursor_shift > 0) {
                    input_buffer.cursor_shift--;
                    move_cursor_right();
                }
                break;

            case CTRL('N'):
                delete_numbers_from_input();
                break;

            case CTRL('A'): // Move cursor to start
                if (input_buffer.cursor_shift < input_buffer.edit_index - input_buffer.write_index) {
                    move_cursor_to_start();
                    input_buffer.cursor_shift = input_buffer.edit_index - input_buffer.write_index;
                }
                break;

            case CTRL('E'): // Move cursor to end
                move_cursor_to_end();
                input_buffer.cursor_shift = 0;
                break;

            case ARROW_UP:
                if (command_history.read_index == 0)
                    copy_current_command();
                command_history.read_index++;
                if (command_history.read_index > command_history.write_index)
                    command_history.read_index = command_history.write_index;
                else
                    load_command();
                break;

            case ARROW_DOWN:
                command_history.read_index--;
                if (command_history.read_index > 0)
                    load_command();
                else {
                    command_history.read_index = 0;
                    recover_command();
                }
                break;

            case TAB:
                predict_command();
                break;

            case CTRL('S'):
                copy_buffer.is_copying = 1;
                console_output_char('C');
                copy_buffer.index = 0;
                break;

            case CTRL('F'):
                copy_buffer.is_copying = 0;
                for (int i = 0; i < copy_buffer.index; i++) {
                    if (input_buffer.edit_index - input_buffer.read_index < INPUT_BUFFER_SIZE) {
                        input_put_character(copy_buffer.buffer[i]);
                    }
                }
                break;

            default:
                if (c != 0 && input_buffer.edit_index - input_buffer.read_index < INPUT_BUFFER_SIZE) {
                    c = (c == '\r') ? '\n' : c;

                    if (copy_buffer.is_copying && ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                        if (copy_buffer.index < COPY_BUF_SIZE - 1) {
                            copy_buffer.buffer[copy_buffer.index++] = c;
                        }
                    }

                    if (c == '\n' && input_buffer.edit_index - input_buffer.write_index > 0) {
                        process_input_buffer();

                        reset_command_history();
                        input_buffer.cursor_shift = 0;
                    }
                    input_put_character(c);

                    if (c == '\n' || c == CTRL('D') || input_buffer.edit_index == input_buffer.read_index + INPUT_BUFFER_SIZE) {
                        int cmd_len = input_buffer.edit_index - input_buffer.write_index;
                        char cmd[INPUT_BUFFER_SIZE];
                        for (int i = 0; i < cmd_len; i++) {
                            cmd[i] = input_buffer.buffer[(input_buffer.write_index + i) % INPUT_BUFFER_SIZE];
                        }

                        cmd[cmd_len - 1] = '\0';

                        if (string_compare(cmd, "history") == 0) {
                            for (int i = command_history.write_index - 1; i >= 0; i--) {
                                console_put_string(command_history.buffer[i]);
                                console_output_char('\n');
                            }
                            input_buffer.write_index = input_buffer.edit_index;
                            console_output_char('$');
                            break;
                        }

                        input_buffer.write_index = input_buffer.edit_index;
                        wakeup(&input_buffer.read_index);
                    }
                }
                break;
        }
    }
    release(&console_lock_state.lock);
    if (doprocdump) {
        procdump(); // Now call procdump() without console_lock_state.lock held
    }
}

// Read from console
int consoleread(struct inode* ip, char* dst, int n) {
    uint target;
    int c;

    iunlock(ip);
    target = n;
    acquire(&console_lock_state.lock);
    while (n > 0) {
        while (input_buffer.read_index == input_buffer.write_index) {
            if (myproc()->killed) {
                release(&console_lock_state.lock);
                ilock(ip);
                return -1;
            }
            sleep(&input_buffer.read_index, &console_lock_state.lock);
        }
        c = input_buffer.buffer[input_buffer.read_index++ % INPUT_BUFFER_SIZE];
        if (c == CTRL('D')) {
            if (n < target) {
                input_buffer.read_index--;
            }
            break;
        }
        *dst++ = c;
        --n;
        if (c == '\n')
            break;
    }
    release(&console_lock_state.lock);
    ilock(ip);

    return target - n;
}

// Write to console
int consolewrite(struct inode* ip, char* buf, int n) {
    int i;

    iunlock(ip);
    acquire(&console_lock_state.lock);
    for (i = 0; i < n; i++)
        console_output_char(buf[i] & 0xff);
    release(&console_lock_state.lock);
    ilock(ip);

    return n;
}

// Initialize console
void consoleinit(void) {
    initlock(&console_lock_state.lock, "console");

    devsw[CONSOLE].write = consolewrite;
    devsw[CONSOLE].read = consoleread;
    console_lock_state.locking = 1;

    ioapicenable(IRQ_KBD, 0);
}
