#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *SplitString(char *msg, int word_count) {
    char tokenized_arr[256][256];
    char *tokenized = strtok(msg, " ");
    int comment = 0;
    
    while(tokenized != NULL) {
        if (!strcmp(tokenized, ";") || !strcmp(tokenized, ";;")) {
            comment = 1;
        }
        
        if (!comment) {
            if (tokenized[0] == '\"') {
                strcpy(tokenized_arr[word_count], tokenized);
                memmove(tokenized_arr[word_count], tokenized_arr[word_count]+1, strlen(tokenized_arr[word_count]));
                
                if (tokenized[strlen(tokenized)-1] != '\"') {
                    tokenized = strtok(NULL, " ");
                    while(tokenized != NULL && tokenized[strlen(tokenized)-1] != '\"') {
                        strcat(tokenized_arr[word_count], " ");
                        strcat(tokenized_arr[word_count], tokenized);
                        tokenized = strtok(NULL, " ");
                    }
                    if (tokenized != NULL) {
                        strcat(tokenized_arr[word_count], " ");
                        strncat(tokenized_arr[word_count], tokenized, strlen(tokenized)-1);
                    }
                } else {
                    tokenized_arr[word_count][strlen(tokenized_arr[word_count])-1] = '\0';
                }
                word_count++;
            } else {
                strcpy(tokenized_arr[word_count], tokenized);
                word_count++;
            }
        }
        tokenized = strtok(NULL, " ");
    }
    return tokenized_arr;
}

int main() {
    char msg[] = "some text and \"BIG TEXT\" ; some comment ; adasdasasd";
    int word_count;
    char **tokenized = SplitString(msg, word_count);
    
    for (int i = 0; i < word_count; ++i) {
        printf("%s\n", tokenized[i]);
    }
    
    return 0;
}
