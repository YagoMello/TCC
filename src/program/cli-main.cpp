#ifdef DEBUG
#include <fenv.h> // Enable traps (NaN, Overflow, etc.)
#endif

// INCLUDE ""
#include "../program/cli.hpp"

int main(int argc, char * argv[]){
#ifdef DEBUG
    feenableexcept(FE_INVALID | FE_OVERFLOW);
#endif
    
    cli_t cli;
    
    if(argc > 1) cli.read_file(argv[1]);
    cli.interpreter();
    
    return 0;
}
