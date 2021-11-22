#include "cli.hpp"

void cli_t::end(std::string & /*input*/){
    log("<end> setting \"running_\" flag to \"false\"");
    running_ = false;
}
