#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

// Function prototypes
void usage();
void encode_and_write(const char* text, int key, int fd);
char encode_char(char c, int key);

int main(int argc, char *argv[]) {
    int key = 4; // Default key
    int arg_start = 1; // Index where text arguments start

    // Check if at least one argument is provided
    if (argc < 2) {
        usage();
        exit();
    }

    // Check if a custom key is provided
    if (argc >= 3 && strcmp(argv[1], "-k") == 0) {

        arg_start = 2; // Adjust start index for text arguments
        if (argc <= arg_start) {
            usage();
            exit();
        }
    }

    // Open the result file
    int fd = open("results.txt", O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf(1, "encode: cannot open result.txt\n");
        exit();
    }

    // Process each text argument
    for (int i = arg_start; i < argc; i++) {
        encode_and_write(argv[i], key, fd);

        // Write a space between words except after the last word
        if (i < argc - 1) {
            write(fd, " ", 1);
        }


    }

    // Write a newline to the file
    write(fd, "\n", 1);
    

    // Clean up
    close(fd);
    printf(1, "\n");
    exit();
}

// Function to display usage information
void usage() {
    printf(1, "Usage: encode [-k key] [text...]\n");
    printf(1, "  -k key   Specify the cipher key (positive integer)\n");
}

// Function to encode text and write to file
void encode_and_write(const char* text, int key, int fd) {
    for (int j = 0; text[j] != '\0'; j++) {
        char encoded_char = encode_char(text[j], key);
        printf(1, "%c", encoded_char);
        if (write(fd, &encoded_char, 1) != 1) {
            printf(1, "encode: write error\n");
            close(fd);
            exit();
        }
    }
    printf(1, " ");
}

// Function to encode a single character using Caesar cipher
char encode_char(char c, int key) {
    if (c >= 'a' && c <= 'z') {
        return ((c - 'a' + key) % 26) + 'a';
    } else if (c >= 'A' && c <= 'Z') {
        return ((c - 'A' + key) % 26) + 'A';
    } else {
        return c; // Non-alphabetic characters are unchanged
    }
}
