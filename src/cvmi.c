#include "./cvm.c"

Cvm cvm = {0};

char *shift_args(int *argc, char ***argv, int shift){
    assert(*argc >= shift && *argc > 0);
    char *result = **argv;
    *argc -= shift;
    *argv += shift;
    return result;
}

void usage(FILE *stream, const char *program_name){
    fprintf(stream, "Usage: %s <program.cvm> [-l limit]\n", program_name);
}

int main(int argc, char *argv[]){

    int program_limit = -1;
    const char *program_file = NULL;
    const char *program_name = shift_args(&argc, &argv, 1);

    if(argc < 1){
        usage(stderr, program_name);
        fprintf(stderr, "ERROR: No program file provided\n");
        exit(1);
    }

    program_file = shift_args(&argc, &argv, 1);

    while(argc > 0){
        const char *flag = shift_args(&argc, &argv, 1);
        if(strcmp(flag, "-l") == 0){
            if(argc < 1){
                usage(stderr, program_name);
                fprintf(stderr, "ERROR: No limit provided\n");
                exit(1);
            }
            program_limit = atoi(shift_args(&argc, &argv, 1));
        }
        else{
            usage(stderr, program_name);
            fprintf(stderr, "ERROR: Unknown flag '%s'\n", flag);
            exit(1);
        }
    }

    cvm_load_program_from_file(&cvm, program_file);
    
    Error error = cvm_execute_program(&cvm, program_limit);

    if(error != ERROR_OK){
        fprintf(stderr, "ERROR: %s\n", error_as_cstr(error));
        return 1;
    }

    cvm_dump_stack(stdout, &cvm);
    return 0;
} 