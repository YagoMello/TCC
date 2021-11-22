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

namespace ac_sweep {

static constexpr const char * log_help_string = 
R"---(voltage-ac-sweep-log V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| amplitude  | double |
| freq begin | double |
| freq end   | double |
| offset     | double |
| sweep time | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class logarithmic_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        amplitude_       = next_as<double>(args, 0);
        frequency_begin_ = next_as<double>(args, 100);
        frequency_end_   = next_as<double>(args, 10e3);
        offset_          = next_as<double>(args, 0);
        sweep_time_      = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void setup(sim::simulator_t * simulator, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        if(sweep_time_ == 0){
            sweep_time_ = simulator->duration();
        }
        bind(node_positive_, node_negative_, this);
    }
    
    void clear_variables() override {
        voltage_   = offset_;
        voltage_derivative_ = 0;
        angle_     = 0;
        frequency_ = 0;
        current_   = 0;
    }
    
    void update(double time, double last_time_step) override {
        double last_time = time - last_time_step;
        double cycles = trunc(last_time / sweep_time_);
        double relative_time = last_time / sweep_time_ - cycles;
        frequency_ = frequency_begin_ * std::pow(frequency_end_ / frequency_begin_, relative_time);
        angle_ += 2 * std::numbers::pi * frequency_ * last_time_step;
        voltage_ = amplitude_ * std::sin(angle_) + offset_;
        voltage_derivative_ = amplitude_ * std::cos(angle_) * 2 * std::numbers::pi * frequency_;
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
        return log_help_string;
    }
    
private:
    // sim var
    double voltage_   = 0;
    double voltage_derivative_ = 0;
    double angle_     = 0;
    double frequency_ = 0;
    double current_   = 0;
    
    // params
    double amplitude_ = 0;
    double offset_    = 0;
    double frequency_begin_ = 10;
    double frequency_end_   = 10e3;
    double sweep_time_      = 100e-3;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};


static constexpr const char * lin_help_string = 
R"---(voltage-ac-sweep-lin V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| amplitude  | double |
| freq begin | double |
| freq end   | double |
| offset     | double |
| sweep time | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class linear_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        amplitude_       = next_as<double>(args, 0);
        frequency_begin_ = next_as<double>(args, 100);
        frequency_end_   = next_as<double>(args, 10e3);
        offset_          = next_as<double>(args, 0);
        sweep_time_      = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void setup(sim::simulator_t * simulator, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        if(sweep_time_ == 0){
            sweep_time_ = simulator->duration();
        }
        bind(node_positive_, node_negative_, this);
    }
    
    void clear_variables() override {
        voltage_   = offset_;
        voltage_derivative_ = 0;
        angle_     = 0;
        frequency_ = 0;
        current_   = 0;
    }
    
    void update(double time, double last_time_step) override {
        const double last_time = time - last_time_step;
        const double cycles = trunc(last_time / sweep_time_);
        const double relative_time = last_time / sweep_time_ - cycles;
        frequency_ = (frequency_end_ - frequency_begin_) * relative_time + frequency_begin_;
        angle_ += 2 * std::numbers::pi * frequency_ * last_time_step;
        voltage_ = amplitude_ * std::sin(angle_) + offset_;
        voltage_derivative_ = amplitude_ * std::cos(angle_) * 2 * std::numbers::pi * frequency_;
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
        return lin_help_string;
    }
    
private:
    // sim var
    double voltage_   = 0;
    double voltage_derivative_ = 0;
    double angle_     = 0;
    double frequency_ = 0;
    double current_   = 0;
    
    // params
    double amplitude_ = 0;
    double offset_    = 0;
    double frequency_begin_ = 10;
    double frequency_end_   = 10e3;
    double sweep_time_      = 100e-3;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<ac_sweep::logarithmic_t> logarithmic_reg(database::components(), "sources::voltage-ac-sweep-log");
autoreg::reg<ac_sweep::linear_t>           linear_reg(database::components(), "sources::voltage-ac-sweep-linear");

} // namespace ac_sweep
