#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int in(char *str, char **str_list, int len) {
    for (int i = 0; i < len; ++i) {
        if (!strcmp(str, str_list[i])) {
            return 1;
        } 
    }
    return 0;
}

int main() {
    char msg[] = ".main";
    char *list[] = {".use", ".data", ".func"};
    printf("%d\n", sizeof(list)/sizeof(list[0]));
    printf("%d\n", in(msg, list, sizeof(list)/sizeof(list[0])));
}
