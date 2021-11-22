#include "cli.hpp"

#include <iostream>
#include <fstream>

#include "../libs/next.hpp"

void cli_t::read_file_cmd(std::string & input) {
    std::string file_name = next(input);
    
    while(file_name != ""){
        read_file(input);
        file_name = next(input);
    }
}

void cli_t::read_file(const std::string & file_name){
    log("<read_file> opening file \"" + file_name + "\"");
    std::ifstream file(file_name);
    std::string line;
    if(file.is_open()) log("<read_file> reading file");
    else               error("<read_file> failed to open file");
    
    while(std::getline(file, line)){
        std::cout << line << "\n";
        run_cmd(line);
    }
}
