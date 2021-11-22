#include "cli.hpp"

#include "../libs/next.hpp"

void cli_t::param(std::string & input) {
    std::string cmd = next(input);
    if     (cmd == "vd-max")   sim_params_.vdmax    = next_as<double>(input);
    else if(cmd == "ts-max")   sim_params_.tsmax    = next_as<double>(input);
    else if(cmd == "duration") sim_params_.duration = next_as<double>(input);
    else {
        error("<param> parameter \"" + cmd + "\" not found");
    }
}