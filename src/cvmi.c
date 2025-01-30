#include "./cvm.c"

Cvm cvm = {0};

int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(stderr, "Usage: %s <program.cvm>\n", argv[0]);
        exit(1);
    }

    cvm_load_program_from_file(&cvm, argv[1]);
    
    cvm_execute_program(&cvm);

    cvm_dump_stack(stdout, &cvm);
    return 0;
} 