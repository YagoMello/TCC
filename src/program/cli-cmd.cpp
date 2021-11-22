#include "cli.hpp"

#include "../libs/next.hpp"

void cli_t::run_cmd(std::string & input){
    log("<run_cmd> received: " + input);
    
    // Try to find a comment
    const size_t comment_pos = input.find_first_of('#');
    
    // Protect from components using the comment as argument
    if(comment_pos != input.npos){
        input.erase(comment_pos);
        log("<run_cmd> comment removed: " + input);
    }
    
    // The first word is the command
    std::string cmd_id = next(input);
    
    // Protect from *command "" not found*
    if(cmd_id.empty() == false){
        // cmdtab_ is a fast way to lookup the commands
        log("<run_cmd> searching for command \"" + cmd_id + "\"");
        const auto search_result = cmdtab_.find(cmd_id);
        
        // The lookup returns a iterator to the member function pointer
        if(search_result != cmdtab_.end()){
            // search_result->first is the key (std::string)
            // search_result->second is the data (member func pointer)
            const auto func_ptr = search_result->second;
            log("<run_cmd> found \"" + cmd_id + "\" in \"cmdtab_\"");
            
            // member function pointers need an instance to work (this->)
            // and to be dereferenced (*),
            // so we use the operator ->* to get a working member function
            // then we call it 
            (this->*func_ptr)(input);
        }
        // Report an error if the command was not found
        else error("<run_cmd> command \"" + cmd_id + "\" not found");
    }
    
    log("<run_cmd> done");
}
