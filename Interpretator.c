#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_TOKENS 16
#define MAX_TOKEN_LEN 256
#define MAX_SIZE 1024

int in(char *str, char **str_list, int len) {
    for (int i = 0; i < len; ++i) {
        if (!strcmp(str, str_list[i])) {
            return 1;
        } 
    }
    return 0;
}

int is_label(char *msg) {
    char label = ':';
    int label_pos;
    for (int i = 0; i < strlen(msg); ++i) {
        if (msg[i] == label) {
            label_pos = i;
            break;
        }
    }
    if (strlen(msg)-1 == label_pos) {
        return 1;
    } else {
        return 0;
    }
}

int is_number(char *msg) {
    int is_number = 1;
    if (msg[0] == '0' && msg[1] == 'x') return is_number;
    for (int i = 0; i < strlen(msg); ++i) {
        int ascii = msg[i] & 0xFF;
        if (ascii < 48 || ascii > 57) {
            is_number = 0;
            break;
        }
    }
    return is_number;
}

// Проверка является ли строка числом в hex
int is_hex(const char *str) {
    return str[0] == '0' && tolower(str[1]) == 'x';
}

char** SplitString(const char* str, int* word_count) {
    char** tokens = malloc(MAX_TOKENS * sizeof(char*));
    if (!tokens) {
        return NULL;
    }

    char* copy = strdup(str);
    if (!copy) {
        free(tokens);
        return NULL;
    }

    // Находим первую точку с запятой и обрезаем строку там
    char* semicolon_pos = strchr(copy, ';');
    if (semicolon_pos) {
        *semicolon_pos = '\0'; // Обрезаем строку на точке с запятой
    }

    int token_num = 0;
    char* current = copy;
    int in_quotes = 0;

    while (*current && token_num < MAX_TOKENS) {
        // Пропускаем пробелы
        while (*current == ' ' && !in_quotes) {
            current++;
        }
        
        if (!*current) break;

        // Находим конец токена
        char* token_start = current;
        char* token_end = current;
        
        if (*current == '"') {
            in_quotes = 1;
            token_start++; // Пропускаем открывающую кавычку
            token_end = ++current;
            
            while (*token_end && (*token_end != '"' || *(token_end-1) == '\\')) {
                token_end++;
            }
            
            if (*token_end == '"') {
                current = token_end + 1;
                in_quotes = 0;
            }
        } else {
            while (*token_end && *token_end != ' ') {
                token_end++;
            }
        }

        // Выделяем токен
        size_t token_len = token_end - token_start;
        tokens[token_num] = malloc(token_len + 1);
        if (!tokens[token_num]) {
            for (int i = 0; i < token_num; ++i) {
                free(tokens[i]);
            }
            free(tokens);
            free(copy);
            return NULL;
        }
        
        strncpy(tokens[token_num], token_start, token_len);
        tokens[token_num][token_len] = '\0';
        token_num++;
        
        current = token_end;
        if (*current) current++;
    }

    free(copy);
    
    if (token_num == 0) {
        free(tokens);
        return NULL;
    }
    
    tokens[token_num] = NULL;
    *word_count = token_num;
    
    return tokens;
}

void FreeTokens(char** tokens) {
    if (!tokens) return;
    for (int i = 0; tokens[i] != NULL; ++i) {
        free(tokens[i]);
    }
    free(tokens);
}

int is_only_spaces(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isspace((unsigned char)str[i])) {
            return 0;
        }
    }
    return 1;
}

// Форматирует строку (аналог Python's format / C++ std::format)
char* format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // Вычисляем необходимый размер
    int size = vsnprintf(NULL, 0, fmt, args) + 1; // +1 для '\0'
    va_end(args);

    if (size <= 0) return NULL;

    // Выделяем память
    char* buffer = (char*)malloc(size);
    if (!buffer) return NULL;

    // Форматируем строку
    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);

    return buffer; // Нужно освободить через free()!
}

/*
int main() {
    const char* str = "PENIS I BOBER";
    char** tokens = SplitString(str);
    if (!tokens) {
        printf("Ошибка при разбиении строки\n");
        return 1;
    }

    for (int i = 0; tokens[i] != NULL; ++i) {
        printf("%s\n", tokens[i]);
    }

    printf("%s\n", tokens[1]);
    
    FreeTokens(tokens); // Освобождаем память
    return 0;
}
*/
