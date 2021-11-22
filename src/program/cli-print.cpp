#include "cli.hpp"

#include "../libs/next.hpp"

void cli_t::print(std::string & input){
    std::string cmd = next(input);
    if(cmd == "var")  print_var(input);
    else error("<print> parameter \"" + cmd + "\" not found");
}

void cli_t::print_var(std::string & input){
    std::string key = next(input);
    bool mode;
    
    auto it = objects_.printables.find(key);
    
    log("<print_var> searching for var \"" + key + "\"");
    if(it != objects_.printables.end()){
        log("<print_var> found printable at \"objects_.printables\" node " + ptr_to_str(&(it->second)));
        
        if(apply_bool(input, mode, true)){
            log(std::string("<print_var> setting \"is_active\" field to ") + (mode ? "true" : "false"));
            it->second.is_active = mode;
        }
        else{
            error("<print_var> bad argument. \"print var " + key + "\" expects \"true\" or \"false\" after \"" + key + "\"");
        }
    }
    else{
        error("<print_var> variable \"" + key + "\" not found");
    }
}
