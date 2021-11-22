#include "simulator.hpp"

// INCLUDE <>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <vector>
#include <chrono>
#include <thread>
#include <stdexcept>

// INCLUDE ""
#include "component.hpp"
#include "node.hpp"
#include "supernode.hpp"
#include "printable.hpp"

void sim::simulator_t::add_node(node::node_t * node) {
    node_vector_.push_back(node);
}

void sim::simulator_t::add_component(comp::component_t * component) {
    if(in_simulation_) throw std::logic_error("[simulator_t] Cannot add component while in simulation");
    
    component_vector_.push_back(component);
}

void sim::simulator_t::add_printable(printable::var_info_t * obj) {
    if(obj->is_active){
        printable_vector_.push_back(obj);
    }
}

void sim::simulator_t::duration(const double time) {
    if(in_simulation_) throw std::logic_error("[simulator_t] Cannot modify \"duration\" while in simulation");
    
    if(time != 0){
        duration_ = time;
    }
    else{
        duration_ = std::numeric_limits<double>::infinity();
    }
}

double sim::simulator_t::duration() const {
    return duration_;
}

void sim::simulator_t::time_step_max(const double value) {
    if(in_simulation_) throw std::logic_error("[simulator_t] Cannot modify \"time_step_max\" while in simulation");
    
    time_step_max_ = value;
}

void sim::simulator_t::voltage_difference_max(const double value) {
    voltage_difference_max_ = value;
}

void sim::simulator_t::nodes_set_voltage_difference_max_if_default() {
    for(node::node_t * node_iterator : node_vector_) {
        if(node_iterator->is_default_voltage_difference_max()){
            node_iterator->temporary_voltage_difference_max(voltage_difference_max_);
        }
    }
}

double sim::simulator_t::get_time_step(const double time_elapsed) const {
    double smallest_time_step = time_step_max_;
    double this_time_step     = 0;
    double this_voltage_derivative;
    
#ifdef DEBUG_ITERATIONS
    std::string who = "simulator max step duration";
#endif
    
    for(const node::node_t * const node_iterator : node_vector_){
#ifdef DEBUG_TS
        printf("\n%s requesting voltage derivative\n", node_iterator->label().c_str());
        this_voltage_derivative = node_iterator->voltage_derivative();
        printf("\nResult: % .2e\n", this_voltage_derivative);
#else
        this_voltage_derivative = node_iterator->voltage_derivative();
#endif
        
        if(this_voltage_derivative != 0){
            this_time_step = node_iterator->voltage_difference_max() / std::abs(this_voltage_derivative);
#ifdef DEBUG_TS
            printf("ts: %.2e = %.2e / %.2e\n", this_time_step, voltage_difference_max_, std::abs(this_voltage_derivative));
#endif
        }
        else{
            this_time_step = time_step_max_;
        }
        
        if(this_time_step < smallest_time_step){
#ifdef DEBUG_ITERATIONS
            who = node_iterator->label();
#endif
            smallest_time_step = this_time_step;
        }
    }
    //printf("Ts1=%fns\n", 1e9 * smallest_time_step);
    
    for(const comp::component_t * const component_iterator : component_vector_) {
        this_time_step = component_iterator->time_step_max(time_elapsed);
        
        if(
            (this_time_step > 0) &&
            (this_time_step < smallest_time_step)
        ){
            //printf("comp max ts: %.2ens\n", 1e9 * this_time_step);
#ifdef DEBUG_ITERATIONS
            who = component_iterator->label();
#endif
            smallest_time_step = this_time_step;
        }
    }
#ifdef DEBUG_ITERATIONS
    printf(" %s: %.2eps\n", who.c_str(),  1e12 * smallest_time_step);
#endif
    return smallest_time_step;
}

void sim::simulator_t::simulate() {
    in_simulation_ = true;
    
    nodes_set_voltage_difference_max_if_default();
    
    double time_step = 0;
    double time_elapsed = 0;
    
    for(auto * node_iterator      : node_vector_     ) node_iterator->clear();
    for(auto * component_iterator : component_vector_) component_iterator->setup(this, voltage_difference_max_, time_step_max_);
    for(auto * component_iterator : component_vector_) component_iterator->setup_printer(node_vector_, printable_vector_);
    for(auto * component_iterator : component_vector_) component_iterator->clear_variables();
    
    find_supernodes();
    
    for(auto * supernode_iterator : supernode_vector_) supernode_iterator->init();
    for(auto * supernode_iterator : supernode_vector_) supernode_iterator->fix_divergence();
    
    uint64_t iteration = 0;
    while(time_elapsed < duration_ && is_alive(time_elapsed)){
        for(auto * component_iterator : component_vector_) component_iterator->printer(time_elapsed, time_step, iteration);
        
        iteration++;
        
        while(is_paused(time_elapsed)) wait();
        
        for(auto * node_iterator      : node_vector_     ) node_iterator->begin_iteration();
        for(auto * component_iterator : component_vector_) component_iterator->update(time_elapsed, time_step);
        for(auto * supernode_iterator : supernode_vector_) supernode_iterator->update();
        
        time_step = get_time_step(time_elapsed);
        
        for(auto * supernode_iterator : supernode_vector_) supernode_iterator->fix_divergence_if(voltage_difference_max_ / 10);
        for(auto * supernode_iterator : supernode_vector_) supernode_iterator->apply_iteration(time_step);
        for(auto * node_iterator      : node_vector_     ) node_iterator->apply_iteration(time_step);
        
        time_elapsed += time_step;
        
        for(auto * component_iterator : component_vector_) component_iterator->post_update(time_elapsed, time_step);
        
    }
    
    in_simulation_ = false;
    for(auto * component_iterator : component_vector_) component_iterator->simulation_complete();
}

bool sim::simulator_t::is_alive(const double time) const {
    bool result = false;
    for(auto * component_iterator : component_vector_) result = result || component_iterator->keep_simulation_alive(time);
    return result;
}
bool sim::simulator_t::is_paused(const double time) const {
    bool result = false;
    for(auto * component_iterator : component_vector_) result = result || component_iterator->pause_simulation(time);
    return result;
}
void sim::simulator_t::wait() const {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(15ms);
}

void sim::simulator_t::find_supernodes() {
#ifdef DEBUG_SUPERNODES
    std::cout << "sim->find_supernodes()\n";
#endif
    supernode_vector_.clear();
    for(auto & node : node_vector_){
        auto ptr = node->supernode().get();
#ifdef DEBUG_SUPERNODES
        std::cout << "found " << ptr << "\n";
#endif
        if(ptr != nullptr && std::find(supernode_vector_.begin(), supernode_vector_.end(), ptr) == supernode_vector_.end()){
            supernode_vector_.push_back(ptr);
        }
    }
}
