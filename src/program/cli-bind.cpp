#include "cli.hpp"

#include "../libs/next.hpp"

void cli_t::bind(std::string & input){
    // Parsing the arguments
    std::string var1 = next(input);
    std::string var2 = next(input);
    
    log("<bind> binding \"" + var1 + "\" and \"" + var2 + "\"");
    
    // Protect from missing arguments
    if(var1.empty()){
        error("<bind> failed to bind: missing arguments");
    }
    else if(var2.empty()){
        error("<bind> failed to bind: missing second argument");
    }
    else{
        shared::bind(objects_.shared_vars, var1, var2);
        log("<bind> binding successful");
    }
}
