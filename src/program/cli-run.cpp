#include "cli.hpp"

#include "../simulator/simulator.hpp"

void cli_t::run(std::string & /*input*/){
    log("<run> creating simulator instance");
    sim::simulator_t simulator;
    
    log("<run> setting simulation parameters");
    simulator.voltage_difference_max(sim_params_.vdmax);
    simulator.time_step_max(sim_params_.tsmax);
    simulator.duration(sim_params_.duration);
    
    log("<run> loading components");
    for(auto & pair : objects_.components) simulator.add_component(pair.second.get());
    log("<run> loading nodes");
    for(auto & pair : objects_.nodes     ) simulator.add_node(pair.second.get());
    log("<run> loading printbles");
    for(auto & pair : objects_.printables) simulator.add_printable(&pair.second);
    
    log("<run> starting simulation");
    simulator.simulate();
    log("<run> simulation complete");
}
