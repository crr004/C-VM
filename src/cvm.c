#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include <stdint.h>


#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define CVM_STACK_CAPACITY 1024
#define CVM_PROGRAM_CAPACITY 1024
#define MAX_LABELS 100
#define MAX_LABEL_LENGTH 30
#define MAX_JUMPS 110

typedef enum{
    ERROR_OK = 0,
    ERROR_STACK_OVERFLOW,
    ERROR_STACK_UNDERFLOW,
    ERROR_ILLEGAL_INST,
    ERROR_DIV_BY_ZERO,
    ERROR_ILLEGAL_INST_ACCESS,
    ERROR_ILLEGAL_OPERAND,
    ERROR_OK_NO_INST,
} Error;

const char *error_as_cstr(Error error){
    switch(error){
        case ERROR_OK:
            return "Ok";
        case ERROR_STACK_OVERFLOW:
            return "Stack overflow";
        case ERROR_STACK_UNDERFLOW:
            return "Stack underflow";
        case ERROR_ILLEGAL_INST:
            return "Illegal instruction";
        case ERROR_DIV_BY_ZERO:
            return "Division by zero";
        case ERROR_ILLEGAL_INST_ACCESS:
            return "Illegal instruction access";
        case ERROR_ILLEGAL_OPERAND:
            return "Illegal operand";
        case ERROR_OK_NO_INST:
            return "Ok, no instruction";
        default:
            assert(0 && "error_as_cstr: Unknown error");
    }
}

typedef int64_t Word;

typedef struct {
    char *name[MAX_LABEL_LENGTH];
    Word addr;
} Label;

Label label_table[MAX_LABELS];
size_t label_count = 0;

typedef struct{
    char *label[MAX_LABEL_LENGTH];
    Word addr;
} Jump;

Jump jump_table[MAX_JUMPS];
size_t jump_count = 0;

typedef enum {
    INST_NOP = 0,
    INST_PUSH,
    INST_DUP,
    INST_PLUS, // sum first two elements of the stack and push the result
    INST_MINUS, 
    INST_MULT,
    INST_DIV,
    INST_JMP,
    INST_JMP_IF,
    INST_EQ,
    INST_HALT,
    INST_PRINT_DEBUG,
} Inst_Type;

const char *inst_type_as_sctr(Inst_Type type){
    switch(type){
        case INST_NOP:
            return "INST_NOP";
        case INST_PUSH:
            return "INST_PUSH";
        case INST_DUP:
            return "INST_DUP";
        case INST_PLUS:
            return "INST_PLUS";
        case INST_MINUS:
            return "INST_MINUS";
        case INST_MULT:
            return "INST_MULT";
        case INST_DIV:
            return "INST_DIV";
        case INST_JMP:
            return "INST_JMP";
        case INST_JMP_IF:
            return "INST_JMP_IF";
        case INST_EQ:
            return "INST_EQ";
        case INST_HALT:
            return "INST_HALT";
        case INST_PRINT_DEBUG:
            return "INST_PRINT_DEBUG";
        default:
            assert(0 && "inst_type_as_cstr: Unknown instruction type");
    }
}

typedef struct{
    Inst_Type type;
    Word operand;
} Inst;

typedef struct {
    Word stack[CVM_STACK_CAPACITY];
    Word stack_size;

    Inst program[CVM_PROGRAM_CAPACITY];
    Word ip;
    Word program_size;

    char *memory;

    int halt;
} Cvm;

#define MAKE_INST_NOP (Inst) {0}
#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = value}
#define MAKE_INST_DUP(addr) {.type=INST_DUP, .operand = addr}
#define MAKE_INST_PLUS {.type = INST_PLUS}
#define MAKE_INST_MINUS {.type = INST_MINUS}
#define MAKE_INST_MULT {.type = INST_MULT}
#define MAKE_INST_DIV {.type = INST_DIV}
#define MAKE_INST_JMP(addr) {.type = INST_JMP, .operand = addr}
#define MAKE_INST_HALT {.type = INST_HALT}


Inst inst_push(Word operand){
    return (Inst){
        .type = INST_PUSH,
        .operand = operand
    };
}

Inst inst_plus(){
    return (Inst){
        .type = INST_PLUS
    };
}

Error cvm_ex_inst(Cvm *cvm){
    if(cvm->ip < 0 || cvm->ip >= cvm->program_size){
        return ERROR_ILLEGAL_INST_ACCESS;
    }

    Inst inst = cvm->program[cvm->ip];

    switch(inst.type){
        case INST_NOP:
            cvm->ip++;
            return ERROR_OK_NO_INST;
        case INST_PUSH:
            if(cvm->stack_size >= CVM_STACK_CAPACITY){
                return ERROR_STACK_OVERFLOW;
            }
            cvm->stack[cvm->stack_size++] = inst.operand;
            cvm->ip++;
            break;
        case INST_DUP:
            if(cvm->stack_size >= CVM_STACK_CAPACITY){
                return ERROR_STACK_OVERFLOW;
            }
            if(cvm->stack_size - inst.operand <= 0){
                return ERROR_STACK_UNDERFLOW;
            }
            if(inst.operand < 0){
                return ERROR_ILLEGAL_OPERAND;
            }
            cvm->stack[cvm->stack_size] = cvm->stack[cvm->stack_size - 1 - inst.operand];
            cvm->stack_size++;
            cvm->ip++;
            break;
        case INST_PLUS:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            cvm->stack[cvm->stack_size - 2] += cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
            cvm->ip++;
            break;
        case INST_MINUS:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            cvm->stack[cvm->stack_size - 2] -= cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
            cvm->ip++;
            break;
        case INST_MULT:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            cvm->stack[cvm->stack_size - 2] *= cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
            cvm->ip++;
            break;
        case INST_DIV:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            if(cvm->stack[cvm->stack_size - 1] == 0){
                return ERROR_DIV_BY_ZERO;
            }
            cvm->stack[cvm->stack_size - 2] /= cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
            cvm->ip++;
            break;
        case INST_JMP:
            cvm->ip = inst.operand;
            break;
        case INST_JMP_IF:
            if(cvm->stack_size < 1){
                return ERROR_STACK_UNDERFLOW;
            }
            if(cvm->stack[cvm->stack_size - 1]){
                cvm->stack_size--;
                cvm->ip = inst.operand;
            }
            else{
                cvm->ip++;
            }
            break;
        case INST_EQ:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            cvm->stack[cvm->stack_size - 2] = cvm->stack[cvm->stack_size - 2] == cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
            cvm->ip++;
            break;
        case INST_HALT:
            cvm->halt = 1;
            break;
        case INST_PRINT_DEBUG:
            if(cvm->stack_size < 1){
                return ERROR_STACK_UNDERFLOW;
            }
            printf("%lld\n", cvm->stack[cvm->stack_size-1]);
            cvm->stack_size--;
            cvm->ip++;
            break;
        default:
            return ERROR_ILLEGAL_INST;
    }
    return ERROR_OK;
}

void cvm_dump_stack(FILE *stream, const Cvm *cvm){
    fprintf(stream, "Stack:\n");
    if (cvm->stack_size <= 0){
        fprintf(stream, "[Empty]\n");
    }
    else {
        for(Word i = 0; i < cvm->stack_size; i++){
            fprintf(stream, "%lld\n", cvm->stack[i]);
        }
    }
    fprintf(stream, "\n");
}

typedef struct {
    size_t count;
    const char *data;
} String_view;

String_view cstr_as_string_view(const char *cstr){
    return (String_view){
        .count = strlen(cstr),
        .data = cstr,
    };
}

String_view string_view_trim_left(String_view sv){
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[i])){
        i+=1;
    }
    return (String_view){
        .count = sv.count - i,
        .data = sv.data + i,
    };
}

String_view string_view_trim_right(String_view sv){
    size_t i = 0;
    while(i < sv.count && isspace(sv.data[sv.count - 1 - i])){
        i += 1;
    }

    return (String_view) {
        .count = sv.count - i,
        .data = sv.data,
    };
}

String_view string_view_trim(String_view sv){
    return string_view_trim_right(string_view_trim_left(sv));
}

String_view string_view_chop_by_delim(String_view *sv, char delim){
    size_t i = 0;
    while( i < sv->count && sv->data[i] != delim){
        i += 1;
    }

    String_view result = {
        .count = i,
        .data = sv->data,
    };

    if(i < sv->count){
        sv->count -= i + 1;
        sv->data += i + 1;
    }
    else{
        sv->count -= i;
        sv->data += i;
    }

    return result;
}

int string_view_eq(String_view a, String_view b){
    if(a.count != b.count){
        return 0;
    }
    else{
        return memcmp(a.data, b.data, a.count) == 0;
    }
}

int string_view_to_int(String_view sv){
    int result = 0;
    for(size_t i = 0; i < sv.count && isdigit(*sv.data); i++)
        result = result * 10 + sv.data[i] - '0';

    return result;
}

int string_view_is_label(String_view sv){
    return sv.count > 0 && sv.data[sv.count - 1] == ':';
}

int label_dup(String_view label){
    for(size_t i = 0; i < label_count; i++){
        if(string_view_eq(label, cstr_as_string_view((const char *) label_table[i].name))){
            return 1;
        }
    }
    return 0;
}

int string_view_is_comment(String_view sv){
    return sv.count > 0 && sv.data[0] == '#';
}

Inst cvm_translate_line(String_view line, size_t program_size){
    line = string_view_trim_left(line);
    String_view inst_name = string_view_chop_by_delim(&line,' ');
    String_view op = string_view_chop_by_delim(&line, ' '); // Now line has the in-line comment (if there is any).

    if(line.count > 0 && line.data[0] != '#' && inst_name.data[0] != '#'){ // In-line invalid comment
        fprintf(stderr, "ERROR: Invalid operation '%.*s'\n", (int) line.count, line.data);
        exit(1);
    }

    if(string_view_is_comment(inst_name)){
        return MAKE_INST_NOP;
    }
    else if(string_view_is_label(inst_name)){
        if(label_count >= MAX_LABELS){
            fprintf(stderr, "ERROR: Too many labels\n");
            exit(1);
        }
        if(inst_name.count >= MAX_LABEL_LENGTH){
            fprintf(stderr, "ERROR: Label too long\n");
            exit(1);
        }
        inst_name.count -= 1;
        if(label_dup(inst_name)){
            fprintf(stderr, "ERROR: Duplicated label\n");
            exit(1);
        }
        inst_name = string_view_trim_right(inst_name);
        label_table[label_count].addr = program_size;
        memcpy(label_table[label_count].name, inst_name.data, inst_name.count);
        label_table[label_count].name[inst_name.count] = '\0';
        label_count++;
        return MAKE_INST_NOP;
    }
    else if(string_view_eq(inst_name, cstr_as_string_view("push"))){
        op = string_view_trim_left(op);
        int operand = string_view_to_int(string_view_trim_right(op));
        return (Inst) MAKE_INST_PUSH(operand);
    }
    else if(string_view_eq(inst_name, cstr_as_string_view("dup"))){
        op = string_view_trim_left(op);
        int operand = string_view_to_int(string_view_trim_right(op));
        return (Inst) MAKE_INST_DUP(operand);
    } 
    else if(string_view_eq(inst_name, cstr_as_string_view("plus"))){
        return (Inst) MAKE_INST_PLUS;
    }
    else if(string_view_eq(inst_name, cstr_as_string_view("minus"))){
        return (Inst) MAKE_INST_MINUS;
    }
    else if(string_view_eq(inst_name, cstr_as_string_view("mult"))){
        return (Inst) MAKE_INST_MULT;
    }
    else if(string_view_eq(inst_name, cstr_as_string_view("div"))){
        return (Inst) MAKE_INST_DIV;
    }
    else if(string_view_eq(inst_name, cstr_as_string_view("jmp"))){
        if(jump_count >= MAX_JUMPS){
            fprintf(stderr, "ERROR: Too many jumps\n");
            exit(1);
        }
        op = string_view_trim_left(op);
        String_view operand = string_view_trim_right(op);
        jump_table[jump_count].addr = program_size;
        memcpy(jump_table[jump_count].label, operand.data, operand.count);
        jump_table[jump_count].label[operand.count] = '\0';
        jump_count++;
        int place_holder = program_size;
        return (Inst) MAKE_INST_JMP(place_holder);
    }
    else if(string_view_eq(inst_name, cstr_as_string_view("halt"))){
        return (Inst) MAKE_INST_HALT;
    }
    else{
        fprintf(stderr, "ERROR: unknown operation '%.*s'", (int) inst_name.count, inst_name.data);
        exit(1);
    }
}

void cvm_translate_jumps(Inst *program, size_t program_size){
    for(size_t i = 0; i < program_size; i++){
        if(program[i].type == INST_JMP){
            for(size_t j = 0; j < jump_count; j++){
                for(size_t k = 0; k < label_count; k++){
                    if(strcmp((const char *) jump_table[j].label, (const char *) label_table[k].name) == 0){
                        program[i].operand = label_table[k].addr;
                        break;
                    }
                }
            }
        }
    }
}

size_t cvm_translate_source(String_view source, Inst *program, size_t program_capacity){
    size_t program_size = 0;
    while(source.count > 0){
        assert(program_size < program_capacity);
        String_view line = string_view_trim(string_view_chop_by_delim(&source, '\n'));
        if(line.count <= 0){
            continue;
        }
        program[program_size] = cvm_translate_line(line, program_size);
        program_size+=1;
    }
    cvm_translate_jumps(program, program_size);
    return program_size;
}

String_view slurp_file(const char *file_path){
    FILE *f = fopen(file_path, "r");
    if(f == NULL){
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    if(fseek(f, 0, SEEK_END) < 0){
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if(m < 0){
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    char *buffer = malloc(m + 1);
    if(buffer == NULL){
        fprintf(stderr, "ERROR: Out of memory : %s\n", strerror(errno));
        exit(1);
    } 

    if(fseek(f, 0, SEEK_SET) < 0){
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    size_t n = fread(buffer, 1, m, f);
    if(ferror(f)){
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);

    return (String_view){
        .count = n,
        .data = buffer,
    }; 
}

void cvm_load_program_from_memory(Cvm *cvm, const Inst *program, size_t program_size){
    assert(program_size <= CVM_PROGRAM_CAPACITY);
    memcpy(cvm->program, program, sizeof(Inst) * program_size);
    cvm->program_size = program_size;
}

void cvm_load_program_from_file(Cvm *cvm, const char *file_path){
    FILE *f = fopen(file_path, "rb");
    if(f == NULL){
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }
    if(fseek(f, 0, SEEK_END) < 0){
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    long m = ftell(f);
    if(m < 0){
        fprintf(stderr, "ERROR: Could not read file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }
    
    assert(m % sizeof(cvm->program[0]) == 0);
    assert((size_t)m <= CVM_PROGRAM_CAPACITY * sizeof(cvm->program[0]));

    if(fseek(f, 0, SEEK_SET) < 0){
        fprintf(stderr, "ERROR: Could not opne file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    cvm->program_size = fread(cvm->program, sizeof(cvm->program[0]), m / sizeof(cvm->program[0]), f);

    if(ferror(f)){
        fprintf(stderr, "ERROR: Could not opne file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

void cvm_save_program_to_file(Inst *program, size_t program_size, const char *file_path){
    FILE *f = fopen(file_path, "wb");
    if(f == NULL){
        fprintf(stderr, "ERROR: Could not open file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }
    
    fwrite(program, sizeof(program[0]), program_size, f);

    if(ferror(f)){
        fprintf(stderr, "ERROR: Could not write to file '%s': %s\n", file_path, strerror(errno));
        exit(1);
    }

    fclose(f);
}

Error cvm_execute_program(Cvm *cvm, int lim){
    Error error;
    for(int i=lim; i != 0 && !cvm->halt; i--){
        error = cvm_ex_inst(cvm);

        if(error == ERROR_OK_NO_INST){
            i++;
            error = ERROR_OK;
            continue;
        }
        
        if(error != ERROR_OK){
            break;
        }
    }
    return error;
}