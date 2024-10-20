#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

void usage();
void encode_and_write(const char* text, int key, int fd);
char encode_char(char c, int key);

int main(int argc, char *argv[]) {
    int key = 4;
    int arg_start = 1;

    if (argc < 2) {
        usage();
        exit();
    }

    if (argc >= 3 && strcmp(argv[1], "-k") == 0) {

        arg_start = 2;
        if (argc <= arg_start) {
            usage();
            exit();
        }
    }

    int fd = open("results.txt", O_CREATE | O_WRONLY);
    if (fd < 0) {
        printf(1, "encode: cannot open result.txt\n");
        exit();
    }

    for (int i = arg_start; i < argc; i++) {
        encode_and_write(argv[i], key, fd);

        if (i < argc - 1) {
            write(fd, " ", 1);
        }


    }

    write(fd, "\n", 1);
    

    close(fd);
    printf(1, "\n");
    exit();
}

void usage() {
    printf(1, "Usage: encode [-k key] [text...]\n");
    printf(1, "  -k key   Specify the cipher key (positive integer)\n");
}

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

char encode_char(char c, int key) {
    if (c >= 'a' && c <= 'z') {
        return ((c - 'a' + key) % 26) + 'a';
    } else if (c >= 'A' && c <= 'Z') {
        return ((c - 'A' + key) % 26) + 'A';
    } else {
        return c;
    }
}
