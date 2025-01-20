#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#define CVM_STACK_CAPACITY 1024
#define CVM_PROGRAM_CAPACITY 1024
#define CVM_EXECUTION_LIMIT 40

typedef enum{
    ERROR_OK = 0,
    ERROR_STACK_OVERFLOW,
    ERROR_STACK_UNDERFLOW,
    ERROR_ILLEGAL_INST,
    ERROR_DIV_BY_ZERO,
    ERROR_ILLEGAL_INST_ACCESS,
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
        default:
            assert(0 && "error_as_cstr: Unknown error");
    }
}

typedef int64_t Word;

typedef enum {
    INST_PUSH,
    INST_PLUS, // sum first two elements of the stack and push the result
    INST_MINUS, 
    INST_MULT,
    INST_DIV,
    INST_JMP,
    INST_HALT,
} Inst_Type;

const char *inst_type_as_sctr(Inst_Type type){
    switch(type){
        case INST_PUSH:
            return "INST_PUSH";
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
        case INST_HALT:
            return "INST_HALT";
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

    int halt;
} Cvm; 

#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = value}
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
        case INST_PUSH:
            if(cvm->stack_size >= CVM_STACK_CAPACITY){
                return ERROR_STACK_OVERFLOW;
            }
            cvm->stack[cvm->stack_size++] = inst.operand;
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
        case INST_HALT:
            cvm->halt = 1;
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

Cvm cvm = {0};
Inst program[] = {
    MAKE_INST_PUSH(0),
    MAKE_INST_PUSH(1),
    MAKE_INST_PLUS,
    MAKE_INST_JMP(1),
}; 

void cvm_bush_inst(Cvm *cvm, Inst inst){
    assert(cvm->program_size < CVM_PROGRAM_CAPACITY);
    cvm->program[cvm->program_size++] = inst;
}

void cvm_load_program_from_memory(Cvm *cvm, const Inst *program, size_t program_size){
    assert(program_size <= CVM_PROGRAM_CAPACITY);
    memcpy(cvm->program, program, sizeof(Inst) * program_size);
    cvm->program_size = program_size;
}

int main(){
    cvm_load_program_from_memory(&cvm, program, ARRAY_SIZE(program));
    cvm_dump_stack(stdout, &cvm);
    for(int i=0; i < CVM_EXECUTION_LIMIT && !cvm.halt; i++){
        Error error = cvm_ex_inst(&cvm);
        cvm_dump_stack(stdout, &cvm);
        if(error != ERROR_OK){
            fprintf(stderr, "Error: %s\n", error_as_cstr(error));
            exit(1);
        }
    }
    return 0;
}