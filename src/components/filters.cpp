// INCLUDE <>
#include <cstdint>
#include <cmath>
#include <numbers>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/filter.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

template <typename T>
class lowpass_t : public comp::component_t {
public:
    using filter_type = T;
    
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_in_);
        info << utils::find_node_s(nmap, next(args), node_out_);
        info << utils::find_node_s(nmap, next(args), node_ref_);
        if(error) return info;
        
        size_t order  = next_as<size_t>(args, 1);
        double cutoff = next_as<double>(args, 1000);
        
        voltage_out_.configure(order, cutoff);
        
        for(size_t pos = 0; pos < order; pos++){
            std::string state_name = label() + ":s" + std::to_string(pos);
            std::string deriv_name = label() + ":d" + std::to_string(pos);
            info << utils::add_printable_s(pmap, state_name, "", voltage_out_.state(pos), error);
            info << utils::add_printable_s(pmap, deriv_name, "", voltage_out_.derivative(pos+1), error);
            if(error) return info;
        }
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        voltage_out_.clear(); // voltage = 0 should work too
        voltage_in_ = 0;
        current_ = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double last_time_step) override {
        voltage_out_.add_sample(voltage_in_, last_time_step);
        voltage_in_ = voltage_difference(node_in_, node_ref_);
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    double sn_voltage_derivative(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_.derivative();
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return voltage_out_.derivative();
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
private:
    // sim var
    double current_;
    double voltage_in_;
    filter_type voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

autoreg::reg<lowpass_t<filt::dynamic::butterworth_t<double>>> lowpass_reg(database::components(), "filt::butter");
autoreg::reg<lowpass_t<filt::dynamic::critical_t<double>>> critical_reg(database::components(), "filt::critical");
