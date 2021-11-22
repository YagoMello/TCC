// INCLUDE <>
#include <cstdint>
#include <cmath>
#include <numbers>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

namespace dc_sweep {

static constexpr const char * help_string = 
R"---(voltage-dc-sweep V1.0 2021-09-06

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| volt. min  | double |
| volt. max  | double |
| sweep time | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class dc_sweep_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        voltage_min_ = next_as<double>(args, 0);
        voltage_max_ = next_as<double>(args, 100);
        sweep_time_  = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void setup(sim::simulator_t * simulator, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        if(sweep_time_ == 0){
            sweep_time_ = simulator->duration();
        }
        
        voltage_derivative_ = (voltage_max_ - voltage_min_) / sweep_time_;
        
        bind(node_positive_, node_negative_, this);
    }
    
    void clear_variables() override {
        voltage_ = voltage_min_;
        current_ = 0;
    }
    
    void update(double time, double /*last_time_step*/) override {
        voltage_ = voltage_derivative_ * time + voltage_min_;
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_positive_ && 
            neg == node_negative_
        ){
            return voltage_;
        }
        else if(
            pos == node_negative_ &&
            neg == node_positive_
        ){
            return -voltage_;
        }
        else{
            throw;
        }
    }
    
    double sn_voltage_derivative(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_positive_ && 
            neg == node_negative_
        ){
            return voltage_derivative_;
        }
        else if(
            pos == node_negative_ &&
            neg == node_positive_
        ){
            return -voltage_derivative_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_positive_ && 
            neg == node_negative_
        ){
            current_ = current;
        }
        else if(
            pos == node_negative_ &&
            neg == node_positive_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string;
    }
    
private:
    // sim var
    double voltage_   = 0;
    double current_   = 0;
    
    // params
    double voltage_max_;
    double voltage_min_;
    double sweep_time_;
    
    // pre-computed
    double voltage_derivative_ = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<dc_sweep::dc_sweep_t> dc_sweep_reg(database::components(), "sources::voltage-dc-sweep");

} // namespace ac_sweep
