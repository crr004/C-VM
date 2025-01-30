#include "./cvm.c"

Cvm cvm = {0};

int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "Usage: %s <program.cvm>\n", argv[0]);
        exit(1);
    }

    const char *input_file_path = argv[1];

    cvm_load_program_from_file(&cvm, input_file_path);

    for(Word i = 0; i < cvm.program_size; i++){
        Inst inst = cvm.program[i];
        switch(inst.type){
            case INST_NOP:
                printf("NOP\n");
                break;
            case INST_PUSH:
                printf("PUSH %lld\n", inst.operand);
                break;
            case INST_DUP:
                printf("DUP %lld\n", inst.operand);
                break;
            case INST_PLUS:
                printf("PLUS\n");
                break;
            case INST_MINUS:
                printf("MINUS\n");
                break;
            case INST_MULT:
                printf("MULT\n");
                break;
            case INST_DIV:
                printf("DIV\n");
                break;
            case INST_JMP:
                printf("JMP %lld\n", inst.operand);
                break;
            case INST_JMP_IF:
                printf("JMP_IF %lld\n", inst.operand);
                break;
            case INST_EQ:
                printf("EQ\n");
                break;
            case INST_HALT:
                printf("HALT\n");
                break;
            case INST_PRINT_DEBUG:
                printf("PRINT_DEBUG\n");
                break;
            default:
                fprintf(stderr, "ERROR: Unknown instruction\n");
                exit(1);
        }
    }   


}