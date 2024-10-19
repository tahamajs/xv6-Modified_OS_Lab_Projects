#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf(1, "Usage: decode [text]\n");
        exit();
    }

    int key = 4;
    int fd = open("result.txt", O_CREATE | O_WRONLY);
    if(fd < 0){
        printf(1, "decode: cannot open result.txt\n");
        exit();
    }

    for(int i = 1; i < argc; i++){
        char *text = argv[i];
        for(int j = 0; text[j] != '\0'; j++){
            char c = text[j];
            if(c >= 'a' && c <= 'z'){
                c = ((c - 'a' - key + 26) % 26) + 'a';
            } else if(c >= 'A' && c <= 'Z'){
                c = ((c - 'A' - key + 26) % 26) + 'A';
            }
            write(fd, &c, 1);
        }
        if(i < argc - 1){
            char space = ' ';
            write(fd, &space, 1);
        }
    }
    char newline = '\n';
    write(fd, &newline, 1);

    close(fd);
    exit();
}
