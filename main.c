#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define BM_STACK_CAPACITY 1024

typedef enum{
    ERROR_OK = 0,
    ERROR_STACK_OVERFLOW,
    ERROR_STACK_UNDERFLOW,
    ERROR_ILLEGAL_INST,
    ERROR_DIV_BY_ZERO,
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
        default:
            assert(0 && "error_as_cstr: Unknown error");
    }
}

typedef int64_t Word;

typedef struct {
    Word stack[BM_STACK_CAPACITY];
    size_t stack_size;
} Cvm; 

typedef enum {
    INST_PUSH,
    INST_PLUS, // sum first two elements of the stack and push the result
    INST_MINUS, 
    INST_MULT,
    INST_DIV,
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
        default:
            assert(0 && "inst_type_as_cstr: Unknown instruction type");
    }
}

typedef struct{
    Inst_Type type;
    Word operand;
} Inst;

#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = value}
#define MAKE_INST_PLUS {.type = INST_PLUS}
#define MAKE_INST_MINUS {.type = INST_MINUS}
#define MAKE_INST_MULT {.type = INST_MULT}
#define MAKE_INST_DIV {.type = INST_DIV}

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

Error cvm_ex_inst(Cvm *cvm, Inst inst){
    switch(inst.type){
        case INST_PUSH:
            if(cvm->stack_size >= BM_STACK_CAPACITY){
                return ERROR_STACK_OVERFLOW;
            }
            cvm->stack[cvm->stack_size++] = inst.operand;
            break;
        case INST_PLUS:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            cvm->stack[cvm->stack_size - 2] += cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
            break;
        case INST_MINUS:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            cvm->stack[cvm->stack_size - 2] -= cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
            break;
        case INST_MULT:
            if(cvm->stack_size < 2){
                return ERROR_STACK_UNDERFLOW;
            }
            cvm->stack[cvm->stack_size - 2] *= cvm->stack[cvm->stack_size - 1];
            cvm->stack_size--;
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
            break;
        default:
            return ERROR_ILLEGAL_INST;
    }
    return ERROR_OK;
}

void cvm_dump(FILE *stream, const Cvm *cvm){
    fprintf(stream, "Stack:\n");
    if (cvm->stack_size <= 0){
        fprintf(stream, "[Empty]\n");
    }
    else {
        for(size_t i = 0; i < cvm->stack_size; i++){
            fprintf(stream, "%lld\n", cvm->stack[i]);
        }
    }
    fprintf(stream, "\n");
}

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

Cvm cvm = {0};
Inst program[] = {
    MAKE_INST_PUSH(69),
    MAKE_INST_PUSH(42),
    MAKE_INST_PLUS,
    MAKE_INST_PUSH(42),
    MAKE_INST_MINUS,
    MAKE_INST_PUSH(2),
    MAKE_INST_MULT,
    MAKE_INST_PUSH(4),
    MAKE_INST_DIV,
}; 

int main(){
    cvm_dump(stdout, &cvm);
    for(size_t i = 0; i < ARRAY_SIZE(program); i++){
        printf("Executing %s\n", inst_type_as_sctr(program[i].type));
        Error error = cvm_ex_inst(&cvm, program[i]);
        cvm_dump(stdout, &cvm);
        if(error != ERROR_OK){
            fprintf(stderr, "Error: %s\n", error_as_cstr(error));
            exit(1);
        }
    }
    return 0;
}