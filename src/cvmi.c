#include "./cvm.c"

int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "Usage: %s <program.cvm>\n", argv[0]);
        exit(1);
    }

    cvm_load_program_from_file(&cvm, argv[1]);
    for(int i=0; i < CVM_EXECUTION_LIMIT && !cvm.halt; i++){
        Error error = cvm_ex_inst(&cvm);
        if(error != ERROR_OK){
            fprintf(stderr, "ERROR: %s\n", error_as_cstr(error));
            exit(1);
        }
    }
    cvm_dump_stack(stdout, &cvm);
    return 0;
} 