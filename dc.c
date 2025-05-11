/*

PizdAssembler is an Assembly-like language, that runs on a virtual CPU within memory. It has 24 registers, memory sector with address access, memory sector with a key-value pairs (variables), memory sector for storing registers.

also my englichino is really bad, so sorry
  
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include "Interpretator.c"
#include "registers.c"

#define VERSION "0.8.0"

#define MAX32 2147483647
#define MAX16 65535
#define MAX10 1024
#define MAX8  256
#define MAX4  16    

#define STACK_MEMORY 1024
#define BASE_STRING_ADDRESS 0x8000

// ANSI escape codes for colors
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_RESET   "\x1b[0m"

typedef struct {

    uint32_t EAX,   ECX,   EDX,   EBX,     ESP, EBP, ESI, EDI;
    uint16_t AX,    CX,    DX,    BX,      SP,  BP,  SI,  DI;
    uint8_t  AH,AL, CH,CL, DH,DL, BH,BL;
    
} Register;

typedef struct {

    uint8_t ZF, SF, CF, OF;

} CMP_Flags;

typedef struct {
    
    uint32_t stack[STACK_MEMORY];
    int stack_top;

} RET_Stack;

typedef struct {

    char name[MAX8];
    int value;
    
} VARIABLE;

typedef struct {

    uint32_t MemStack[MAX16];
    uint32_t RegSector[MAX8];
    uint8_t  RegSectorTop;
    VARIABLE VariableStack[STACK_MEMORY];
    uint16_t VariableStackTop;
    uint32_t ArgumentSector[MAX10];
    uint8_t  ArgumentSectorTop;
    
} Memory;

typedef struct {

    char name[MAX4];
    uint8_t len;
    uint8_t args;
    uint32_t token;
    
} INSTRUCTION;

void EditRegister(Register* reg, Memory* mem, char* name, uint32_t value) {
    if (strcmp(name, "EAX") == 0) reg->EAX = value;
    else if (strcmp(name, "ECX") == 0) reg->ECX = value;
    else if (strcmp(name, "EDX") == 0) reg->EDX = value;
    else if (strcmp(name, "EBX") == 0) reg->EBX = value;
    else if (strcmp(name, "ESP") == 0) reg->ESP = value;
    else if (strcmp(name, "EBP") == 0) reg->EBP = value;
    else if (strcmp(name, "ESI") == 0) reg->ESI = value;
    else if (strcmp(name, "EDI") == 0) reg->EDI = value;
    
    else if (strcmp(name, "AX") == 0) reg->AX = value;
    else if (strcmp(name, "CX") == 0) reg->CX = value;
    else if (strcmp(name, "DX") == 0) reg->DX = value;
    else if (strcmp(name, "BX") == 0) reg->BX = value;
    else if (strcmp(name, "SP") == 0) reg->SP = value;
    else if (strcmp(name, "BP") == 0) reg->BP = value;
    else if (strcmp(name, "SI") == 0) reg->SI = value;
    else if (strcmp(name, "DI") == 0) reg->DI = value;
    
    else if (strcmp(name, "AH") == 0) reg->AH = value;
    else if (strcmp(name, "AL") == 0) reg->AL = value;
    else if (strcmp(name, "CH") == 0) reg->CH = value;
    else if (strcmp(name, "CL") == 0) reg->CL = value;
    else if (strcmp(name, "DH") == 0) reg->DH = value;
    else if (strcmp(name, "DL") == 0) reg->DL = value;
    else if (strcmp(name, "BH") == 0) reg->BH = value;
    else if (strcmp(name, "BL") == 0) reg->BL = value;
    else {
        for (int i = 0; i <= mem->VariableStackTop; ++i) {
            if (!strcmp(mem->VariableStack[i].name, name))
                mem->VariableStack[i].value = value;
        }
    }
}

uint32_t GetRegisterValue(Register *reg, Memory *mem, char *RegName) {
    if (strcmp(RegName, "EAX") == 0) return reg->EAX;
    else if (strcmp(RegName, "EBX") == 0) return reg->EBX;
    else if (strcmp(RegName, "ECX") == 0) return reg->ECX;
    else if (strcmp(RegName, "EDX") == 0) return reg->EDX;
    else if (strcmp(RegName, "ESP") == 0) return reg->ESP;
    else if (strcmp(RegName, "EBP") == 0) return reg->EBP;
    else if (strcmp(RegName, "ESI") == 0) return reg->ESI;
    else if (strcmp(RegName, "EDI") == 0) return reg->EDI;
    
    else if (strcmp(RegName, "AX") == 0) return reg->AX;
    else if (strcmp(RegName, "BX") == 0) return reg->BX;
    else if (strcmp(RegName, "CX") == 0) return reg->CX;
    else if (strcmp(RegName, "DX") == 0) return reg->DX;
    else if (strcmp(RegName, "SP") == 0) return reg->SP;
    else if (strcmp(RegName, "BP") == 0) return reg->BP;
    else if (strcmp(RegName, "SI") == 0) return reg->SI;
    else if (strcmp(RegName, "DI") == 0) return reg->DI;
    
    else if (strcmp(RegName, "AH") == 0) return reg->AH;
    else if (strcmp(RegName, "AL") == 0) return reg->AL;
    else if (strcmp(RegName, "BH") == 0) return reg->BH;
    else if (strcmp(RegName, "BL") == 0) return reg->BL;
    else if (strcmp(RegName, "CH") == 0) return reg->CH;
    else if (strcmp(RegName, "CL") == 0) return reg->CL;
    else if (strcmp(RegName, "DH") == 0) return reg->DH;
    else if (strcmp(RegName, "DL") == 0) return reg->DL;   
    else {        
        for (int i = 0; i <= mem->VariableStackTop; ++i) {
            if (!strcmp(mem->VariableStack[i].name, RegName))
                return mem->VariableStack[i].value;
        }
    }
    if (RegName[0] == '$') {
        return (int)RegName[1];
    }
    return -1;
}

int is_register(char *str, Memory *mem) {
    
    if (!str) return 0;

    for (int i = 0; i < 24; ++i) {
      if (strcmp(registers[i], str) == 0) {
        return 1;
      }
    }
    
    for (int i = 0; i <= mem->VariableStackTop; ++i) {
        if (!strcmp(mem->VariableStack[i].name, str))
            return 1;
    }
    if (str[0] == '$') {
        return 1;
    }
    return 0;
}

void PrintRegister(const Register *reg) {
    printf("32-bit registers:\n");
    printf("%sEAX%s: 0x%08X  %sEBX%s: 0x%08X  %sECX%s: 0x%08X  %sEDX%s: 0x%08X\n", 
           reg->EAX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->EAX,
           reg->EBX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->EBX,
           reg->ECX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->ECX,
           reg->EDX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->EDX);
    printf("%sESP%s: 0x%08X  %sEBP%s: 0x%08X  %sESI%s: 0x%08X  %sEDI%s: 0x%08X\n", 
           reg->ESP ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->ESP,
           reg->EBP ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->EBP,
           reg->ESI ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->ESI,
           reg->EDI ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->EDI);
    
    printf("\n16-bit registers:\n");
    printf("%sAX%s: 0x%04X    %sBX%s: 0x%04X    \n%sCX%s: 0x%04X    %sDX%s: 0x%04X\n", 
           reg->AX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->AX,
           reg->BX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->BX,
           reg->CX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->CX,
           reg->DX ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->DX);
    printf("%sSP%s: 0x%04X    %sBP%s: 0x%04X    \n%sSI%s: 0x%04X    %sDI%s: 0x%04X\n", 
           reg->SP ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->SP,
           reg->BP ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->BP,
           reg->SI ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->SI,
           reg->DI ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->DI);
    
    printf("\n8-bit registers:\n");
    printf("%sAH%s: 0x%02X %sAL%s: 0x%02X\n%sBH%s: 0x%02X %sBL%s: 0x%02X\n%sCH%s: 0x%02X %sCL%s: 0x%02X\n%sDH%s: 0x%02X %sDL%s: 0x%02X\n",
           reg->AH ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->AH,
           reg->AL ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->AL,
           reg->BH ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->BH,
           reg->BL ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->BL,
           reg->CH ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->CH,
           reg->CL ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->CL,
           reg->DH ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->DH,
           reg->DL ? COLOR_GREEN : COLOR_RED, COLOR_RESET, reg->DL);
}

void TRACEBACK(int line, char* msg, char* line_content) {
    printf(" %d | %s\n\n\033[1m\033[31merror:\033[0m %s\n", line, line_content, msg);
    #ifndef DEBUG
    exit(0);
    #endif
}

void InstructionInitialize(char *name, uint8_t len, uint8_t args, uint32_t token, INSTRUCTION *instr) {
    strncpy(instr->name, name, MAX4 - 1);
    instr->name[MAX4 - 1] = '\0';
    instr->len = len;
    instr->args = args;
    instr->token = token;
}

void RegisterDump(Register* reg, Memory mem) {
    for (int i = 0; i < 24; ++i) {
        EditRegister(reg, &mem, registers[i], 0);
    }
}

// Возвращает индекс найденной строки или -1, если не найдено.
int ParseLabel(char source[][MAX8], int line_count, char* name) {
    int last_found_index = -1;
    for (int i = 0; i < line_count; ++i) {
        char* colon_pos = strchr(source[i], ':');
        if (colon_pos != NULL) {
            int label_len = colon_pos - source[i];
            if (strlen(name) == label_len && strncmp(source[i], name, label_len) == 0) {
                last_found_index = i;
            }
        }
    }
    
    return last_found_index;
}

void InitializeMemory(Memory* mem) {
    mem->VariableStackTop = 0;
    mem-> ArgumentSectorTop = 0;
    for (int i = 0; i < MAX16; ++i) mem->MemStack[i] = 0;
    for (int i = 0; i < MAX8; ++i)  mem->RegSector[i] = 0;
    for (int i = 0; i < STACK_MEMORY; ++i) mem->VariableStack[i].value = 0; 
    for (int i = 0; i < MAX10; ++i) mem->ArgumentSector[i] = 0; 
}

void sleep_ms(int milliseconds) {
    struct timespec ts = {
        .tv_sec = milliseconds / 1000,
        .tv_nsec = (milliseconds % 1000) * 1000000
    };
    nanosleep(&ts, NULL);
}

void SystemDump(Register *reg, CMP_Flags flags, Memory memory_module, RET_Stack stack_t) {
    printf("\nFlags:\nZF: %d\nSF: %d\nCF: %d\nOF: %d\n", flags.ZF, flags.SF, flags.CF, flags.OF);
    
    printf("\nMemory:\n");
    for (int i = 0; i < MAX16; ++i) {
        if (memory_module.MemStack[i] != 0) {
            printf("[%d | 0x%02X] => %d - 0x%02X - %c\n", 
                   i, 
                   i, 
                   memory_module.MemStack[i], 
                   memory_module.MemStack[i],
                   (memory_module.MemStack[i] >= 32 && memory_module.MemStack[i] <= 126) ? memory_module.MemStack[i] : '.');
        }
    }    
    printf("\nMemory Register Sector:\n");
    printf("Current top: %d\n", memory_module.RegSectorTop);
    for (int i = 0; i < MAX8; ++i) {
        if (memory_module.RegSector[i]) {
            printf("[%d] => %d - 0x%02X - %c\n",
                   i, 
                   memory_module.RegSector[i], 
                   memory_module.RegSector[i],
                   (memory_module.RegSector[i] >= 32 && memory_module.RegSector[i] <= 126) ? memory_module.RegSector[i] : '.');
        }
    }

    printf("\nDefine Variable stack: \n");
    for (int i = 0; i < memory_module.VariableStackTop; ++i) {
        printf("[%d | %s] => %d - 0x%02X - %c\n",
               i,
               memory_module.VariableStack[i].name,
               memory_module.VariableStack[i].value,
               memory_module.VariableStack[i].value,
               memory_module.VariableStack[i].value);
    }

    printf("\nArgument Sector: \n");
    for (int i = 0; i < memory_module.ArgumentSectorTop; ++i) {
        printf("[%d | %d] => %d - 0x%02X\n",
               i,
               i,
               memory_module.ArgumentSector[i],
               memory_module.ArgumentSector[i]);
    }
    
    printf("\nRET Stack:\n");
    for (int i = 0; i < stack_t.stack_top; ++i) {
        printf("%d: return to line %d\n", i, stack_t.stack[i]);
    }
}

void ReadFile(char* name, int* line_count, char (*buffer)[MAX8], char (*source)[MAX10][MAX8]) {
    FILE* file = fopen(name, "r");
    if (file == NULL) return;  // Просто выходим, если файл не открылся
    
    while (fgets(*buffer, sizeof(*buffer), file) != NULL) {
        if (strstr(*buffer, "db") && !strstr(*buffer, ":")) *buffer[0] = 'B';
        (*buffer)[strcspn(*buffer, "\n")] = '\0';
        strcpy((*source)[*line_count], *buffer);
        (*line_count)++;
    }
    
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argv == NULL || argv[0] == NULL) {
        printf("Invalid arguments!\n");
        return 1;
    }    
    INSTRUCTION MOV;     InstructionInitialize("mov",     3, 2, 0,  &MOV);
    INSTRUCTION ADD;     InstructionInitialize("add",     3, 2, 1,  &ADD);
    INSTRUCTION SUB;     InstructionInitialize("sub",     3, 2, 2,  &SUB);
    INSTRUCTION MUL;     InstructionInitialize("mul",     3, 2, 3,  &MUL);
    INSTRUCTION DIV;     InstructionInitialize("div",     3, 2, 4,  &DIV);
    INSTRUCTION POW;     InstructionInitialize("pow",     3, 2, 5,  &POW);
    INSTRUCTION JMP;     InstructionInitialize("jmp",     3, 1, 6,  &JMP);
    INSTRUCTION CMP;     InstructionInitialize("cmp",     3, 2, 7,  &CMP);
    INSTRUCTION JE;      InstructionInitialize("je",      2, 1, 8,  &JE);
    INSTRUCTION JNE;     InstructionInitialize("jne",     3, 1, 9,  &JNE);
    INSTRUCTION JG;      InstructionInitialize("jg",      2, 1, 10, &JG);
    INSTRUCTION JL;      InstructionInitialize("jl",      2, 1, 11, &JL);
    INSTRUCTION CALL;    InstructionInitialize("call",    4, 1, 12, &CALL);
    INSTRUCTION RET;     InstructionInitialize("ret",     3, 0, 13, &RET);
    INSTRUCTION TRAP;    InstructionInitialize("trap",    4, 0, 14, &TRAP);
    INSTRUCTION INT;     InstructionInitialize("int",     3, 9, 15, &INT);
    INSTRUCTION INCR;    InstructionInitialize("inc",     4, 1, 16, &INCR);
    INSTRUCTION DECR;    InstructionInitialize("dec",     4, 1, 17, &DECR);
    INSTRUCTION DUMP;    InstructionInitialize("dump",    4, 2, 18, &DUMP);
    INSTRUCTION LOAD;    InstructionInitialize("load",    4, 2, 19, &LOAD);
    INSTRUCTION DB;      InstructionInitialize("db",      2, 0, 20, &DB);
    INSTRUCTION MEMSET;  InstructionInitialize("memset",  7, 2, 21, &MEMSET);
    INSTRUCTION AND;     InstructionInitialize("and",     3, 2, 22, &AND);
    INSTRUCTION OR;      InstructionInitialize("or",      2, 2, 23, &OR);
    INSTRUCTION XOR;     InstructionInitialize("xor",     3, 2, 24, &XOR);
    INSTRUCTION NOT;     InstructionInitialize("not",     3, 1, 25, &NOT);
    INSTRUCTION PUSH;    InstructionInitialize("push",    4, 1, 26, &PUSH);
    INSTRUCTION POP;     InstructionInitialize("pop",     3, 1, 27, &POP);
    INSTRUCTION FEXEC;   InstructionInitialize("fexec",   4, 1, 28, &FEXEC);
    INSTRUCTION CMPSTR;  InstructionInitialize("cmpstr",  6, 2, 29, &CMPSTR);
    INSTRUCTION USE;     InstructionInitialize("use",     3, 1, 30, &USE);
    INSTRUCTION LOADF;   InstructionInitialize("loadf",   5, 3, 31, &LOADF);
    INSTRUCTION READF;   InstructionInitialize("readf",   5, 2, 32, &READF);
    INSTRUCTION DEFINE;  InstructionInitialize("define",  7, 2, 33, &DEFINE);
    INSTRUCTION FREE;    InstructionInitialize("free",    4, 1, 34, &FREE);
    INSTRUCTION ARG;     InstructionInitialize("arg",     3, 1, 35, &ARG);
    INSTRUCTION PUSHA;   InstructionInitialize("pusha",   5, 0, 36, &PUSHA);
    INSTRUCTION POPA;    InstructionInitialize("popa",    4, 0, 37, &POPA);
    
    
    Register reg;
    Memory memory_module; InitializeMemory(&memory_module); memory_module.RegSectorTop = 0;
    CMP_Flags flags;
    RET_Stack stack_t = {0};

    
    int line_count = 0;
    char source[MAX10][MAX8];
    char buffer[MAX8];
    
    ReadFile(argv[1], &line_count, &buffer, &source);

    int section_executable = 1;
    
    for (int current_line = 0; current_line < line_count; ++current_line) {        
        int command_num = 0;
        char** command = SplitString(source[current_line], &command_num);
        
        if (argc > 2 && !strcmp(argv[2], "-pcdebug")) {
            if (argc > 3 && !strcmp(argv[3], "-step")) getchar();
            printf("pc: %d\t|%d\t %s%s", current_line+1, section_executable, source[current_line], (argc <= 3) ? "\n" : "");        
        }
        
        // sleep_ms(50);
        if (!command) continue;    
        
        if (!strcmp(command[0], "section")) {
            if (!strcmp(command[1], ".func")) section_executable = 0;
            if (!strcmp(command[1], ".main")) section_executable = 1;
            if (!strcmp(command[1], ".data")) section_executable = 1;
            if (!strcmp(command[1], ".use"))  section_executable = 1;
        }
        if (!strcmp(command[0], "endsec")) section_executable = 0;

        
        
        if (!section_executable) continue;


        
        // PREPROCESSOR
        if (!strcmp(command[0], DEFINE.name)) {
            if (memory_module.VariableStackTop >= STACK_MEMORY) {
                TRACEBACK(current_line+1, "variable stack overflow", source[current_line]);
            }

            strcpy(memory_module.VariableStack[memory_module.VariableStackTop].name, command[1]);
            if (is_register(command[2], &memory_module)) {
                memory_module.VariableStack[memory_module.VariableStackTop].value = GetRegisterValue(&reg, &memory_module, command[2]);
            } else if (!is_register(command[2], &memory_module)) {
                memory_module.VariableStack[memory_module.VariableStackTop].value = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
            memory_module.VariableStackTop++;
        }
        if (!strcmp(command[0], FREE.name)) {
            int found = 0;
            for (int i = 0; i < memory_module.VariableStackTop; ++i) {
                if (!strcmp(command[1], memory_module.VariableStack[i].name)) {
                    memset(memory_module.VariableStack[i].name, 0, MAX8);
                    memory_module.VariableStack[i].value = 0;
            
                    for (int j = i; j < memory_module.VariableStackTop - 1; j++) {
                        strcpy(memory_module.VariableStack[j].name, memory_module.VariableStack[j+1].name);
                        memory_module.VariableStack[j].value = memory_module.VariableStack[j+1].value;
                    }
            
                    found = 1;
                    break;
                }
            }
    
            if (found) {
                memory_module.VariableStackTop--;
            } else {
                TRACEBACK(current_line+1, "variable not found", source[current_line]);
            }
        }

        // MOV
        if (strcmp(command[0], MOV.name) == 0) {
            if (command_num-1 > MOV.args || command_num-1 < MOV.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", MOV.args), source[current_line]);
            }

            // MOV IF SECOND OPERAND IS A REGISTER
            if (is_register(command[2], &memory_module)) {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[2]));
            }
            
            // MOV IF SECOND OPERAND IS A NUMBER
            else if (command[2][0] == '0' && command[2][1] == 'x') {
                EditRegister(&reg, &memory_module, command[1], strtol(command[2], NULL, 16));
            } else {
                EditRegister(&reg, &memory_module, command[1], strtol(command[2], NULL, 10));
            }
        }

        // ADD
        if (strcmp(command[0], ADD.name) == 0) {
            if (command_num-1 > ADD.args || command_num-1 < ADD.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", ADD.args), source[current_line]);
            }
            // ADD IF SECOND OPERAND IS A REGISTER
            if (is_register(command[2], &memory_module)) {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])+GetRegisterValue(&reg, &memory_module, command[2]));
            }
            
            // ADD IF SECOND OPERAND IS A NUMBER
            else if (command[2][0] == '0' && command[2][1] == 'x') {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])+strtol(command[2], NULL, 16));
            } else {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])+strtol(command[2], NULL, 10));
            }
        }

        // SUB
        if (strcmp(command[0], SUB.name) == 0) {
            if (command_num-1 > SUB.args || command_num-1 < SUB.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", SUB.args), source[current_line]);
            }

            // SUB IF SECOND OPERAND IS A REGISTER
            if (is_register(command[2], &memory_module)) {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])-GetRegisterValue(&reg, &memory_module, command[2]));
            }
            
            // SUB IF SECOND OPERAND IS A NUMBER
            else if (command[2][0] == '0' && command[2][1] == 'x') {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])-strtol(command[2], NULL, 16));
            } else {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])-strtol(command[2], NULL, 10));
            }
        }

        // MUL
        if (strcmp(command[0], MUL.name) == 0) {
            if (command_num-1 > MUL.args || command_num-1 < MUL.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", MUL.args), source[current_line]);
            }

            // MUL IF SECOND OPERAND IS A REGISTER
            if (is_register(command[2], &memory_module)) {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])*GetRegisterValue(&reg, &memory_module, command[2]));
            }
            
            // MUL IF SECOND OPERAND IS A NUMBER
            else if (command[2][0] == '0' && command[2][1] == 'x') {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])*strtol(command[2], NULL, 16));
            } else {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])*strtol(command[2], NULL, 10));
            }
        }

        // DIV
        if (strcmp(command[0], DIV.name) == 0) {
            if (command_num-1 > DIV.args || command_num-1 < DIV.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", DIV.args), source[current_line]);
            }
            // В DIV перед делением
            if (GetRegisterValue(&reg, &memory_module, command[2]) == 0) {
                TRACEBACK(current_line+1, "division by zero", source[current_line]);
            }
            
            // DIV IF SECOND OPERAND IS A REGISTER
            if (is_register(command[2], &memory_module)) {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])/GetRegisterValue(&reg, &memory_module, command[2]));
            }
            
            // DIV IF SECOND OPERAND IS A NUMBER
            else if (command[2][0] == '0' && command[2][1] == 'x') {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])/strtol(command[2], NULL, 16));
            } else {
                EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])/strtol(command[2], NULL, 10));
            }
        }

        // POW
        if (!strcmp(command[0], POW.name)) {
            if (command_num-1 > POW.args || command_num-1 < POW.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", POW.args), source[current_line]);
            }

            int val1 = GetRegisterValue(&reg, &memory_module, command[1]);
            int val2;
            int res = 1;
            
            if (is_register(command[2], &memory_module)) {
                val2 = GetRegisterValue(&reg, &memory_module, command[2]);
            } else if (!is_register(command[2], &memory_module)) {
                val2 = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
            for (int i = 1; i <= val2; ++i) {
                res = res*val1;
            }
            
            EditRegister(&reg, &memory_module, command[1], res);
        }

        // JMP
        if (strcmp(command[0], JMP.name) == 0) {
            if (command_num-1 > JMP.args || command_num-1 < JMP.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", JMP.args), source[current_line]);
            }

            int target_line = ParseLabel(source, line_count, command[1]);
            if (target_line == -1) {
                TRACEBACK(current_line+1, "label not found", source[current_line]);
            }
    
            current_line = target_line - 1;
    
            continue;
        }
        
        // CMP
        if (strcmp(command[0], CMP.name) == 0) {
            if (command_num-1 > CMP.args || command_num-1 < CMP.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", CMP.args), source[current_line]);
            }
            
            int32_t val1, val2;

            if (is_register(command[1], &memory_module)) {
                val1 = GetRegisterValue(&reg, &memory_module, command[1]);
            } else {
                val1 = strtol(command[1], NULL, is_hex(command[1]) ? 16 : 10);
            }

            if (is_register(command[2], &memory_module)) {
                val2 = GetRegisterValue(&reg, &memory_module, command[2]);
            } else {
                val2 = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }

            
            int32_t result = val1 - val2;

            // Устанавливаем флаги
            flags.ZF = (result == 0);
            flags.SF = (result < 0);
            flags.CF = (val1 < val2);
            flags.OF = ((val1 ^ val2) & (val1 ^ result) & 0x80000000) != 0;
        }
        
        // JE (Jump if Equal)
        if (strcmp(command[0], "je") == 0) {
            if (flags.ZF) {
                int target_line = ParseLabel(source, line_count, command[1]);
                if (target_line == -1) TRACEBACK(current_line+1, "label not found", source[current_line]);
                current_line = target_line - 1;
                flags.ZF = 0;
            } 
            
            continue;
        }

        // JNE (Jump if Not Equal)
        if (strcmp(command[0], "jne") == 0) {
            if (!flags.ZF) {
                int target_line = ParseLabel(source, line_count, command[1]);
                if (target_line == -1) TRACEBACK(current_line+1, "label not found", source[current_line]);
                current_line = target_line - 1;
                flags.ZF = 0;
            } 
            
            continue;
        }

        // JG (Jump if Greater)
        if (strcmp(command[0], "jg") == 0) {
            if (!flags.ZF && (flags.SF == flags.OF)) {
                int target_line = ParseLabel(source, line_count, command[1]);
                if (target_line == -1) TRACEBACK(current_line+1, "label not found", source[current_line]);
                current_line = target_line - 1;
                flags.ZF = 0;
            } 
            
            continue;
        }

        // JL (Jump if Less)
        if (strcmp(command[0], "jl") == 0) {
            if (flags.SF != flags.OF) {
                int target_line = ParseLabel(source, line_count, command[1]);
                if (target_line == -1) TRACEBACK(current_line+1, "label not found", source[current_line]);
                current_line = target_line - 1;
                flags.ZF = 0;
            } 
            
            continue;
        }

        // CALL
        if (strcmp(command[0], CALL.name) == 0) {
            int val;
            if (command_num > 2) {
                for (int i = 2; i < command_num; ++i) {
                    if (is_register(command[i], &memory_module)) {
                        val = GetRegisterValue(&reg, &memory_module, command[i]);
                    } else if (!is_register(command[i], &memory_module)) {
                        val = strtol(command[i], NULL, is_hex(command[i]) ? 16 : 10);                
                    }

                    memory_module.ArgumentSector[memory_module.ArgumentSectorTop] = val;
                    memory_module.ArgumentSectorTop++;
                }
            }
                
            stack_t.stack[stack_t.stack_top++] = current_line + 1;
            
            int target_line = ParseLabel(source, line_count, command[1]);
            if (target_line == -1) {
                TRACEBACK(current_line+1, "label not found", source[current_line]);
            }
            
            current_line = target_line - 1;
    
            continue;
        }

        // RET
        if (strcmp(command[0], RET.name) == 0) {
            if (command_num-1 > RET.args || command_num-1 < RET.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", RET.args), source[current_line]);
            }
            
            if (stack_t.stack_top == 0) {
                TRACEBACK(current_line+1, "stack underflow (maybe you forget \"call\"?)", source[current_line]);
            }

            current_line = stack_t.stack[--stack_t.stack_top] - 1;
            continue;
        }

        /*
          ДОП. ИНСТРУКЦИИ
        */
        
        // TRAP
        if (strcmp(command[0], TRAP.name) == 0) {
            for (int i = 0; i < 3; ++i) {
                printf("%d | %s\n", current_line+i, source[current_line+i-1]);
            }
            printf("\ntrap at %d\n", current_line+1);
            printf("\nreg - register dump\nmem - memory dump\nfile - check file content\nr - continue executing\nc - clear\nq - exit\n");
            // interactive trap
            while (1) {
                char buffer[MAX8];
                printf("trap : ");
                scanf("%s", buffer);
                printf("---------------------\n");
                if (!strcmp(buffer, "reg"))  PrintRegister(&reg);
                if (!strcmp(buffer, "mem"))  SystemDump(&reg, flags, memory_module, stack_t);    
                if (!strcmp(buffer, "file")) for (int i = 0; i < line_count; ++i) printf("%d \t| %s\n", i+1, source[i]);
                if (!strcmp(buffer, "r"))    break;
                if (!strcmp(buffer, "c"))    system("clear");
                if (!strcmp(buffer, "q"))    goto debug_end;
            }
        }

        // INT
        if (strcmp(command[0], INT.name) == 0) {            
            uint8_t interrupt_id;
            
            if (is_register(command[1], &memory_module)) {
                interrupt_id = GetRegisterValue(&reg, &memory_module, command[1]);
            } else if (!is_register(command[1], &memory_module)) {
                interrupt_id = strtol(command[1], NULL, 16);
            }               
            
            switch (interrupt_id) {
            case 0x00: {// RESET REGISTERS DATA INTERRUPT
                RegisterDump(&reg, memory_module);
                break;
            }

            case 0x01: {
                flags.ZF = 0;
                flags.SF = 0;
                flags.CF = 0;
                flags.OF = 0;
            }
                
            case 0x02: { // PRINT REGISTERS INTERRUPT
                PrintRegister(&reg);
                break;
            }

            case 0x03: { // FULL MEMORY DUMP INTERRUPT
                printf("\n--- FULL SYSTEM DUMP ---\n");
                SystemDump(&reg, flags, memory_module, stack_t);
                break;
            }

            case 0x04: { // RETURN PC
                EditRegister(&reg, &memory_module, "ESI", current_line);
                break;
            }
                
            case 0x10: { // PRINT CHAR (AL=char)
                char ch = GetRegisterValue(&reg, &memory_module, "AL");
                putchar(ch & 0xFF);
                fflush(stdout);
                break;
            }
                
            case 0x11: { // INPUT CHAR (блокирующий, но Enter завершает ввод)
                int ch = getchar();  
                EditRegister(&reg, &memory_module, "AL", ch & 0xFF);
                break;
            }
                
            case 0x20: { // RANDOM NUMBER INTERRUPT
                EditRegister(&reg, &memory_module, "AX", rand()%strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10));
                break;
            }
                
            case 0x30: { // STRLEN INTERRUPT
                uint32_t address = GetRegisterValue(&reg, &memory_module, "SI");
                uint16_t len = 0;
                
                for (int i = 0; i != MAX16; ++i) {
                    len++;
                    if (memory_module.MemStack[address + i] == 0xEF) break;
                }
                EditRegister(&reg, &memory_module, "BP", len-1);
                break;
            }
                
            case 0xFF: { // PROGRAM END INTERRUPT
                goto debug_end;
                break;
            }
                
            default: {
                TRACEBACK(current_line+1, "unknown interrupt", source[current_line]);
                break;
            }
            }
        }

        if (strcmp(command[0], INCR.name) == 0) {
            if (command_num-1 > INCR.args || command_num-1 < INCR.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", INCR.args), source[current_line]);
            }
            EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])+1);
        
        }

        if (strcmp(command[0], DECR.name) == 0) {
            if (command_num-1 > DECR.args || command_num-1 < DECR.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", DECR.args), source[current_line]);
            }
            EditRegister(&reg, &memory_module, command[1], GetRegisterValue(&reg, &memory_module, command[1])-1);
        }

        // DUMP
        if (strcmp(command[0], DUMP.name) == 0) {
            if (command_num-1 > DUMP.args || command_num-1 < DUMP.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", DUMP.args), source[current_line]);
            }
            uint32_t address;
            if (address >= MAX16) {
                TRACEBACK(current_line+1, "memory access out of bounds", source[current_line]);
            }
            
            if (is_register(command[1], &memory_module)) {
                address = GetRegisterValue(&reg, &memory_module, command[1]);                
            } else if (!is_register(command[1], &memory_module)) {
                address = strtol(command[1], NULL, is_hex(command[1]) ? 16 : 10);                
            }
            
            if (is_register(command[2], &memory_module)) {
                memory_module.MemStack[address] = GetRegisterValue(&reg, &memory_module, command[2]);
            } else if (!is_register(command[2], &memory_module)) {
                memory_module.MemStack[address] = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
        }

        // LOAD
        if (strcmp(command[0], LOAD.name) == 0) {
            if (command_num-1 > LOAD.args || command_num-1 < LOAD.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", LOAD.args), source[current_line]);
            }
            
            uint32_t address = 0;      
            
            if (is_register(command[2], &memory_module)) {
                address = GetRegisterValue(&reg, &memory_module, command[2]);
            } else if (!is_register(command[2], &memory_module)) {
                address = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
            if (address >= MAX16) {
                TRACEBACK(current_line+1, "memory access out of bounds", source[current_line]);
            }
            EditRegister(&reg, &memory_module, command[1], memory_module.MemStack[address]);
        }

        // DB instruction handling
        if (command[1] && !strcmp(command[1], DB.name)) {
            
            
            int named = 0;
            if (strstr(source[current_line], ": ") && !is_label(source[current_line])) {
                named = 1;
            }
            command[0][strlen(command[0])-1] = '\0';
        
            // Handle numeric values (stored in 0x4000+ range)
            if (is_number(command[2]) || is_register(command[2], &memory_module)) {
                uint16_t address = 0x4000; // начальный адрес для чисел
                
                // Find free space for the number
                while (memory_module.MemStack[address] != 0 && address < sizeof(memory_module.MemStack)) {
                    address++;
                }

                if (address >= sizeof(memory_module.MemStack)) {
                    TRACEBACK(current_line+1, "not enough memory for number", source[current_line]);
                }
        
                // Store the number
                long num_value;
                if (is_register(command[2], &memory_module)) {
                    num_value = GetRegisterValue(&reg, &memory_module, command[2]);                
                } else if (!is_register(command[2], &memory_module)) {
                    num_value = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);                
                }
                memory_module.MemStack[address] = num_value;
                
        
                // Create variable entry
                if (named) {
                    strcpy(memory_module.VariableStack[memory_module.VariableStackTop].name, command[0]);
                    memory_module.VariableStack[memory_module.VariableStackTop].value = address;
                    memory_module.VariableStackTop++;
                }
            }
            // Handle string values (stored in BASE_STRING_ADDRESS range)
            else {
             
                char* msg = command[2];
                uint8_t msg_eof = 0xEF; // default terminator
                
                // Check if terminator is explicitly specified (after quoted string)
                if (command[3] && strstr(source[current_line], "\"")) {
                    if (is_register(command[3], &memory_module)) {
                        msg_eof = GetRegisterValue(&reg, &memory_module, command[3]);
                    } else {
                        msg_eof = strtol(command[3], NULL, is_hex(command[3]) ? 16 : 10);
                    }
                }
        
                uint16_t address = BASE_STRING_ADDRESS; // начальный адрес для строк
                uint16_t required_length = strlen(msg) + 1; // +1 для msg_eof
        
                // Find free space for the string
                while (1) {
                    int is_free = 1;
                    for (int i = 0; i < required_length; i++) {
                        if (memory_module.MemStack[address + i] != 0) {
                            is_free = 0;
                            break;
                        }
                    }
            
                    if (is_free) {
                        break;
                    }
            
                    address += required_length;
            
                    if (address + required_length >= 0xFFFF) { // Don't go into number space
                        TRACEBACK(current_line+1, "not enough memory for string", source[current_line]);
                    }
                }
        
                // Process string with escape sequences
                for (int i = 0, j = 0; i < strlen(msg); i++, j++) {
                    if (msg[i] == '\\') {
                        switch(msg[i+1]) {
                        case '0':  memory_module.MemStack[address + j] = 0x00; break;  // \0 Null character
                        case 'a':  memory_module.MemStack[address + j] = 0x07; break;  // \a Bell (Alert)
                        case 'b':  memory_module.MemStack[address + j] = 0x08; break;  // \b Backspace
                        case 't':  memory_module.MemStack[address + j] = 0x09; break;  // \t Horizontal Tab
                        case 'n':  memory_module.MemStack[address + j] = 0x0A; break;  // \n Line Feed
                        case 'v':  memory_module.MemStack[address + j] = 0x0B; break;  // \v Vertical Tab
                        case 'f':  memory_module.MemStack[address + j] = 0x0C; break;  // \f Form Feed
                        case 'r':  memory_module.MemStack[address + j] = 0x0D; break;  // \r Carriage Return
                        case 'e':  memory_module.MemStack[address + j] = 0x1B; break;  // \e Escape
                        case 'x': {
                            char hex[3] = {msg[i+2], msg[i+3], '\0'};
                            memory_module.MemStack[address + j] = (char)strtol(hex, NULL, 16);
                            i += 2;
                            break;
                        }
                        default:
                            memory_module.MemStack[address + j] = msg[i];
                            i--;
                            break;
                        }
                        i++;
                    } else {
                        memory_module.MemStack[address + j] = msg[i];  // Обычный символ
                    }
                    memory_module.MemStack[address + j + 1] = msg_eof;
                }
        
                // Create variable entry
                if (named) {
                    strcpy(memory_module.VariableStack[memory_module.VariableStackTop].name, command[0]);
                    memory_module.VariableStack[memory_module.VariableStackTop].value = address;
                    memory_module.VariableStackTop++;
                }
            }   
        }
        
        // AND
        if (strcmp(command[0], AND.name) == 0) {
            if (command_num-1 != AND.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", AND.args), source[current_line]);
            }
    
            uint32_t val1;
            uint32_t val2;

            if (is_register(command[1], &memory_module)) {
                val1 = GetRegisterValue(&reg, &memory_module, command[1]);
            } else {
                val1 = strtol(command[2], NULL, is_hex(command[1]) ? 16 : 10);
            }
            
            if (is_register(command[2], &memory_module)) {
                val2 = GetRegisterValue(&reg, &memory_module, command[2]);
            } else {
                val2 = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
    
            EditRegister(&reg, &memory_module, command[1], val1 & val2);
        }

        // OR
        if (strcmp(command[0], OR.name) == 0) {
            if (command_num-1 != OR.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", OR.args), source[current_line]);
            }
    
            uint32_t val1;
            uint32_t val2;

            if (is_register(command[1], &memory_module)) {
                val1 = GetRegisterValue(&reg, &memory_module, command[1]);
            } else {
                val1 = strtol(command[2], NULL, is_hex(command[1]) ? 16 : 10);
            }
            
            if (is_register(command[2], &memory_module)) {
                val2 = GetRegisterValue(&reg, &memory_module, command[2]);
            } else {
                val2 = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
    
            EditRegister(&reg, &memory_module, command[1], val1 | val2);
        }

        // XOR
        if (strcmp(command[0], XOR.name) == 0) {
            if (command_num-1 != XOR.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", XOR.args), source[current_line]);
            }
    
            uint32_t val1;
            uint32_t val2;

            if (is_register(command[1], &memory_module)) {
                val1 = GetRegisterValue(&reg, &memory_module, command[1]);
            } else {
                val1 = strtol(command[2], NULL, is_hex(command[1]) ? 16 : 10);
            }
            
            if (is_register(command[2], &memory_module)) {
                val2 = GetRegisterValue(&reg, &memory_module, command[2]);
            } else {
                val2 = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
    
            EditRegister(&reg, &memory_module, command[1], val1 ^ val2);
        }

        // NOT
        if (strcmp(command[0], NOT.name) == 0) {
            if (command_num-1 != NOT.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", NOT.args), source[current_line]);
            }
    
            uint32_t val = GetRegisterValue(&reg, &memory_module, command[1]);
            EditRegister(&reg, &memory_module, command[1], ~val);
        }

        // MEMSET
        if (strcmp(command[0], MEMSET.name) == 0) {
            if (command_num-1 != MEMSET.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", MEMSET.args), source[current_line]);
            }

            uint32_t val1;
            uint32_t val2;
    
            if (is_register(command[1], &memory_module)) {
                val1 = GetRegisterValue(&reg, &memory_module, command[1]);
            } else {
                val1 = strtol(command[1], NULL, is_hex(command[1]) ? 16 : 10);
            }
    
            if (is_register(command[2], &memory_module)) {
                val2 = GetRegisterValue(&reg, &memory_module, command[2]);
            } else {
                val2 = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }

            // Проверка на выход за границы памяти
            if (val1 >= MAX16 || val1 + val2 > MAX16) {
                TRACEBACK(current_line+1, "memory access out of bounds", source[current_line]);
            }

            for (uint32_t i = 0; i < val2; ++i) {
                memory_module.MemStack[val1 + i] = 0;
            }
        }

        // PUSH
        if (strcmp(command[0], PUSH.name) == 0) {
            if (command_num-1 != PUSH.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", PUSH.args), source[current_line]);
            }
            if (memory_module.RegSectorTop >= MAX8) {
                TRACEBACK(current_line+1, "register stack overflow", source[current_line]);
            }

            int val;
            if (is_register(command[1], &memory_module)) {
                val = GetRegisterValue(&reg, &memory_module, command[1]);
            } else {
                val = strtol(command[1], NULL, is_hex(command[1]) ? 16 : 10);
            }
            
            memory_module.RegSector[memory_module.RegSectorTop] = val;
            memory_module.RegSectorTop++;
        }
        
        // POP
        if (strcmp(command[0], POP.name) == 0) {
            if (command_num-1 > POP.args)
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", PUSH.args), source[current_line]);
            if (memory_module.RegSectorTop == 0) {
                TRACEBACK(current_line+1, "stack underflow", source[current_line]);
            }
            
            memory_module.RegSectorTop--;
            EditRegister(&reg, &memory_module, command[1], memory_module.RegSector[memory_module.RegSectorTop]);
            memory_module.RegSector[memory_module.RegSectorTop] = 0;
        }

        // FEXEC
        if (strcmp(command[0], FEXEC.name) == 0) {
            if (command_num-1 != FEXEC.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", FEXEC.args), source[current_line]);
            }
            if (system(command[1]) == -1) {
                TRACEBACK(current_line+1, "failed to execute system command", command[1]);
            }
            system(command[1]);
            
        }

        // CMPSTR
        if (strcmp(command[0], CMPSTR.name) == 0) {
            if (command_num-1 != CMPSTR.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", CMPSTR.args), source[current_line]);
            }
            
            uint16_t addr1, addr2;
    
            if (is_register(command[1], &memory_module)) {
                addr1 = GetRegisterValue(&reg, &memory_module, command[1]);
            } else {
                addr1 = strtol(command[1], NULL, is_hex(command[1]) ? 16 : 10);
            }
    
            if (is_register(command[2], &memory_module)) {
                addr2 = GetRegisterValue(&reg, &memory_module, command[2]);
            } else {
                addr2 = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }
            
            for (int i = 0; 1; ++i) {
                uint8_t byte1 = memory_module.MemStack[addr1 + i];
                uint8_t byte2 = memory_module.MemStack[addr2 + i];
                if (byte1 != byte2) {
                    flags.ZF = 0;
                    break;
                }

                if (byte1 == 0xEF && byte2 == 0xEF) {
                    flags.ZF = 1;
                    break;
                }
            }
        }

        // USE
        if (!strcmp(command[0], USE.name)) {
            if (command_num-1 > USE.args || command_num-1 < USE.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", USE.args), source[current_line]);
            }

            char buffer[MAX8];
            
            line_count += 3;
            strcpy(source[line_count-1], format("int 0xFF ; (TERMINATING MAIN PROCESS : : LINKED PASM LIB: %s)", command[1]));
            // printf("%s, %d, %s\n", command[1], line_count, buffer);
            ReadFile(command[1], &line_count, &buffer, &source);
            
        }

        // LOADF (loadf "file.bin" 0x0000 0xEF)
        if (!strcmp(command[0], LOADF.name)) {
            FILE* fp = fopen(command[1], "rb+");
            long address;
            int byte;
            if (is_register(command[2], &memory_module)) {
                address = GetRegisterValue(&reg, &memory_module, command[2]);
            } else if (!is_register(command[2], &memory_module)) {
                address = strtol(command[2], NULL, is_hex(command[2]) ? 16 : 10);
            }

            if (is_register(command[3], &memory_module)) {
                byte = GetRegisterValue(&reg, &memory_module, command[3]);    
            } else if (!is_register(command[3], &memory_module)) {
                byte = strtol(command[3], NULL, 16);
            }
            
            fseek(fp, address, SEEK_SET);
            fwrite(&byte, sizeof(unsigned char), 1, fp);
        }
        
        if (!strcmp(command[0], READF.name)) {}
        if (!strcmp(command[0], ARG.name)) {
            if (command_num-1 != USE.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", USE.args), source[current_line]);
            }

            if (memory_module.ArgumentSectorTop <= 0) {
                TRACEBACK(current_line+1, "arguments underflow", source[current_line]);
            }
            
            memory_module.ArgumentSectorTop--;
            EditRegister(&reg, &memory_module, command[1], memory_module.ArgumentSector[memory_module.ArgumentSectorTop]);
        }

        if (!strcmp(command[0], PUSHA.name)) {
            if (command_num-1 != PUSHA.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", PUSHA.args), source[current_line]);
            }
            int BASE_ADDR = 0x64;
            
            for (int i = 0; i < 24; ++i) {
                memory_module.RegSector[BASE_ADDR + i] = GetRegisterValue(&reg, &memory_module, registers[i]);
                memory_module.RegSectorTop++;
            }
        }
        if (!strcmp(command[0], POPA.name)) {
            if (command_num-1 != POPA.args) {
                TRACEBACK(current_line+1, format("invalid number of arguments (expected %d)", POPA.args), source[current_line]);
            }
            int BASE_ADDR = 0x64;
            
            for (int i = 0; i < 24; ++i) {
                EditRegister(&reg, &memory_module, registers[i], memory_module.RegSector[BASE_ADDR + i]);
                memory_module.RegSector[BASE_ADDR + i] = 0;
                memory_module.RegSectorTop--;
            }
            
        }
        
        FreeTokens(command);
    }

    stack_t.stack_top = 0;
debug_end:
#ifdef DEBUG
    printf("\n\033[1m\033[31m--DEBUG INFO --\033[0m\n");
    printf("PizdAssembler v%s by zetmix\n\n", VERSION);
    PrintRegister(&reg);
    SystemDump(&reg, flags, memory_module, stack_t);
#endif
}
