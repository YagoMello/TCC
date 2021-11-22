#include "cli.hpp"

void cli_t::interpreter(){
    log("<interpreter> running interpreter");
    std::string input;
    running_ = true;
    while(running_ && !std::cin.eof()){
        getline(std::cin, input);
        run_cmd(input);
    }
    log("<interpreter> interpreter stopped");
}
