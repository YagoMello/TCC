// INCLUDE <>
#include <cstdint>
#include <cmath>
#include <numbers>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/statespace.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

#include <iostream>

static constexpr const char * transfer_function_help_string = 
R"---(tf V1.0 2021-09-05
Feedthrough aka. D matrix is not implemented.
Implementing would break "tf.derivative(n)".

|--------------|--------|
|   argument   |  type  |
|==============|========|
| node in      | string |
| node out     | string |
| node ref     | string |
| ordrer       | size_t |
| den[0]       | double |
| ...          | double |
| den[order]   | double |
| num[0]       | double |
| ...          | double |
| den[order-1] | double |
|--------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class transfer_function_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_in_, error);
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_ref_, error);
        if(error) return info;
        
        size_t order = next_as<size_t>(args, 0);
        
        voltage_out_.configure(order);
        
        std::vector<double> numerator_coeffs;
        std::vector<double> denominator_coeffs;
        
        std::cout << "Den: ";
        for(size_t pos = 0; pos <= order; pos++){
            denominator_coeffs.push_back(next_as<double>(args, 0));
            std::cout << denominator_coeffs.back() << " ";
        }
        std::cout << "\nNum: ";
        for(size_t pos = 0; pos <= order; pos++){
            numerator_coeffs.push_back(next_as<double>(args, 0));
            std::cout << numerator_coeffs.back() << " ";
        }
        std::cout << "\n";
        
        voltage_out_.numerator(numerator_coeffs).denominator(denominator_coeffs);
        
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
    
    const char * help() const override {
        return transfer_function_help_string;
    }
    
private:
    // sim var
    double current_;
    double voltage_in_;
    ss::model_t voltage_out_;
    
    // params
    
    node::node_t * node_in_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

autoreg::reg<transfer_function_t> transfer_function_reg(database::components(), "control::tf");
