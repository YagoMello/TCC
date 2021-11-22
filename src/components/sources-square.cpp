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

namespace voltage_source {

static constexpr const char * square_help_string = 
R"---(voltage-ac-res V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| volt high  | double |
| volt low   | double |
| frequency  | double |
| duty cycle | double |
| sw time %  | double |
| phase      | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class sq_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        if(error) return info;
        
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        voltage_low_  = next_as<double>(args,  0);
        voltage_high_ = next_as<double>(args,  1);
        frequency_    = next_as<double>(args, 60);
        duty_         = next_as<double>(args, 50) / 100;
        double swtime = next_as<double>(args,  1) / 100;
        double phase  = next_as<double>(args,  0);
        
        period_ = 1/frequency_;
        slew_time_ = swtime / frequency_;
        slew_rate_ = (voltage_high_ - voltage_low_) / slew_time_;
        delay_ = phase / 360 / frequency_;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_positive_, node_negative_, this);
    }
    
    void clear_variables() override {
        voltage_ = 0;
        current_ = 0;
    }
     
    void update(double time, double) override {
        const double time_normalized = (time + delay_) / period_;
        const double cycle_time = std::fmod(time_normalized, 1);
        
        const double time_high_to_low = slew_time_;
        const double time_low         = 1 - duty_;
        const double time_low_to_high = time_low + slew_time_;
        
        if(cycle_time < time_high_to_low){
            voltage_derivative_ = -slew_rate_;
            voltage_ = std::lerp(voltage_high_, voltage_low_, (cycle_time - 0)/slew_time_);
        }
        else if(cycle_time < time_low){
            voltage_derivative_ = 0;
            voltage_ = voltage_low_;
        }
        else if(cycle_time < time_low_to_high){
            voltage_derivative_ = slew_rate_;
            voltage_ = std::lerp(voltage_low_, voltage_high_, (cycle_time - time_low)/slew_time_);
        }
        else{
            voltage_derivative_ = 0;
            voltage_ = voltage_high_;
        }
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
        return square_help_string;
    }
    
private:
    // sim var
    double voltage_ = 0;
    double current_ = 0;
    double voltage_derivative_ = 0;
    
    // params
    double voltage_high_;
    double voltage_low_;
    double frequency_;
    double duty_;
    double slew_time_;
    double delay_;
    double period_;
    double slew_rate_;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<voltage_source::sq_t> sq_vs_reg(database::components(), "sources::voltage-square");

} // namespace voltage_source
