#include "./cvm.c"

int main(int argc, char *argv[]){
    if(argc < 3){
        fprintf(stderr, "Usage: %s <source.cvmasm> <output.cvm>\n", argv[0]);
        exit(1);
    }
    const char *source_file_path = argv[1];
    const char *output_file_path = argv[2];

    String_view source_code = slurp_file(source_file_path);

    cvm.program_size = cvm_translate_source(source_code, cvm.program, CVM_PROGRAM_CAPACITY);

    cvm_save_program_to_file(cvm.program, cvm.program_size, output_file_path);
    
    return 0;
}