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


    for(int i = 1; i < argc; i++){
        char *text = argv[i];
        for(int j = 0; text[j] != '\0'; j++){
            char c = text[j];
            if(c >= 'a' && c <= 'z'){
                c = ((c - 'a' - key + 26) % 26) + 'a';
            } else if(c >= 'A' && c <= 'Z'){
                c = ((c - 'A' - key + 26) % 26) + 'A';
            }
            printf(1, "%c", c);
        }
        if(i < argc - 1){
            char space = ' ';
            printf(1, " ");
        }
    }
    char newline = '\n';
    printf(1, "\n");

    exit();
}
