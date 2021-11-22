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

static constexpr const char * dc_res_help_string = 
R"---(voltage-dc-res V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| voltage    | double |
| resistance | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class dc_res_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        if(error) return info;
        
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        voltage_    = next_as<double>(args, 0);
        resistance_ = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
    }
    
    void update(double, double) override {
        double vd = voltage_difference(node_positive_, node_negative_);
        current_ = (vd - voltage_) / resistance_;
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
    const char * help() const override {
        return dc_res_help_string;
    }
    
private:
    // sim var
    double current_ = 0;
    
    //params
    double voltage_ = 0;
    double resistance_  = 1000;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

static constexpr const char * ac_res_help_string = 
R"---(voltage-ac-res V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| amplitude  | double |
| resistance | double |
| frequency  | double |
| phase      | double |
| offset     | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class ac_res_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        if(error) return info;
        
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        voltage_        = next_as<double>(args, 5);
        resistance_     = next_as<double>(args, 1);
        frequency_      = next_as<double>(args, 60);
        phase_          = next_as<double>(args, 0);
        offset_         = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
    }
    
    void update(double time, double) override {
        double vd = voltage_difference(node_positive_, node_negative_);
        current_ = (vd - offset_ - voltage_ * std::sin(2 * std::numbers::pi * (frequency_ * time + phase_ / 360))) / resistance_;
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
    const char * help() const override {
        return ac_res_help_string;
    }
    
private:
    // sim var
    double current_ = 0;
    
    // params
    double phase_ = 0;
    double offset_ = 0;
    double voltage_ = 0;
    double frequency_ = 60;
    double resistance_  = 1000;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

/*
#include <cstdio>
class voltage_source_t : public comp::component_t {
public:
    void apply_patches(sim::simulator_t *) override {
        //node_positive_->increment_capacitance(10);
        //node_negative_->increment_capacitance(10);
    }
    
    void calculate_constants(double, double voltage_difference_max) override {
        double cp = node_positive_->capacitance();
        double cn = node_negative_->capacitance();
        double cmin = std::min(cp, cn);
        double cst = comp::utils::invsum(cp, cn);
        time_step_ = cst * cmin * voltage_difference_max / slew_rate_;
    }
    
    double voltage(double time){
        if(std::abs(slew_rate_) * time > std::abs(voltage_)) return voltage_;
        else return std::copysign(std::abs(slew_rate_) * time, voltage_);
    }
    
    void update(double time) override {
        double vd = voltage_difference(node_positive_, node_negative_);
        double vt = voltage(time);
        double cp = node_positive_->capacitance();
        double cn = node_negative_->capacitance();
        
        is_starting_ = (vt != voltage_);
        charge_delta_ = (vd - vt) / comp::utils::invsum(cp, cn);
        //printf("[vd-vt=% .2e,cd=% .2e]", vd - vt, charge_delta_);
        
        node_positive_->charge_in(-charge_delta_);
        node_negative_->charge_in( charge_delta_);
    }
    
    void accumulate_charge_on_nodes(double) override {
        node_positive_->decrement_charge(charge_delta_);
        node_negative_->increment_charge(charge_delta_);
    }
    
    double time_step_max() override {
        if(is_starting_) return time_step_;
        else return 0;
    }
    
    component_t * configure(node_map_type & nmap, std::string args) override {
        node_positive_ = nmap[next(args)].get();
        node_negative_ = nmap[next(args)].get();
        voltage_       = next_as<double>(args);
        slew_rate_     = next_as<double>(args);
        if(slew_rate_ == 0) slew_rate_ = 1e6;
        return this;
    }
    
private:
    bool is_starting_ = true;
    double voltage_ = 0;
    double slew_rate_ = 1e6;
    double time_step_;
    
    double charge_delta_ = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};
*/
/*
class voltage_source_t : public comp::component_t {
public:
    void calculate_constants(double time_step_max, double voltage_difference_max) override {
        vd_max_ = voltage_difference_max;
        ts_max_ = time_step_max;
    }
    
    void update(double) override { }
    
    double derivative_to_target_voltage() {
        double diff = voltage_ - voltage_difference(node_positive_, node_negative_);
        if (std::abs(diff) > vd_max_) return std::copysign(slew_rate_, diff);
        else                          return diff / ts_max_;
    }
    
    virtual void reduce_currents(double) {
        double cp = node_positive_->capacitance();
        double cn = node_negative_->capacitance();
        
        current_ = (node_positive_->voltage_derivative() - node_negative_->voltage_derivative()) / comp::utils::invsum(cp, cn);
        
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
    void compensate_currents(double, double) override {
        double cp = node_positive_->capacitance();
        double cn = node_negative_->capacitance();
        
        double current = (node_positive_->voltage_derivative() - node_negative_->voltage_derivative() - derivative_to_target_voltage()) / comp::utils::invsum(cp, cn);
        
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
        
    }
    
    
    component_t * configure(node_map_type & nmap, std::string args) override {
        node_positive_ = nmap[next(args)].get();
        node_negative_ = nmap[next(args)].get();
        voltage_       = next_as<double>(args);
        slew_rate_     = next_as<double>(args);
        if(slew_rate_ == 0) slew_rate_ = 1e6;
        return this;
    }
    
private:
    double voltage_ = 0;
    double slew_rate_ = 1e6;
    double current_;
    
    double charge_delta_ = 0;
    double vd_max_;
    double ts_max_;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<voltage_source_t> vs_reg(database::components(), "vs");
*/
/*
class voltage_source_sine_t : public component_t {
public:
    virtual void calculate_constants(double voltage_difference_max) override {
        vs.calculate_constants(voltage_difference_max);
    }
    
    virtual void update(double time, double voltage_difference_max) override {
        vs.set_voltage(voltage * sin(2 * 3.141592653589793238462643383279502884L * frequency * time + phase));
        vs.update(time, voltage_difference_max);
    }
    
    virtual void accumulate_charge_on_nodes(double time_step) override {
        vs.accumulate_charge_on_nodes(time_step);
    }
    
    virtual double get_max_time_step() override {
        return vs.get_max_time_step();
    }
    
    void set_nodes(node_t * positive, node_t * negative){
        vs.set_nodes(positive, negative);
    }
    
    void set_resistance(double resistance_inp){
        resistance = resistance_inp;
    }
    
    void set_voltage(double voltage_inp){
        voltage = voltage_inp;
    }
    
    void set_frequency(double frequency_inp){
        frequency = frequency_inp;
    }
    
    void set_phase(double phase_inp){
        phase = phase_inp;
    }
    
private:
    voltage_source_t vs;
    
    double phase = 0;
    double voltage = 0;
    double frequency = 60;
    double resistance  = 1000;
};
*/


static constexpr const char * dc_help_string = 
R"---(voltage-ac-res V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| voltage    | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class dc_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        if(error) return info;
        
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        voltage_ = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_positive_, node_negative_, this);
    }
    
    void update(double, double) override { }
    
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
        return dc_help_string;
    }
    
private:
    // sim var
    double current_ = 0;
    
    // params
    double voltage_ = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

static constexpr const char * ac_help_string = 
R"---(voltage-ac-res V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| amplitude  | double |
| frequency  | double |
| phase      | double |
| offset     | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class ac_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        if(error) return info;
        
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        amplitude_ = next_as<double>(args, 1);
        frequency_ = next_as<double>(args, 60);
        phase_     = next_as<double>(args, 0);
        offset_    = next_as<double>(args, 0);
        
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
        voltage_ = amplitude_ * std::sin(2 * std::numbers::pi * (frequency_ * time + phase_ / 360)) + offset_;
        voltage_derivative_ = amplitude_ * std::cos(2 * std::numbers::pi * (frequency_ * time + phase_ / 360)) * 2 * std::numbers::pi * frequency_;
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
        return ac_help_string;
    }
    
private:
    // sim var
    double voltage_   = 0;
    double voltage_derivative_ = 0;
    double current_   = 0;
    
    // params
    double amplitude_ = 0;
    double offset_    = 0;
    double frequency_ = 0;
    double phase_     = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<voltage_source::dc_res_t> dc_res_vs_reg(database::components(), "sources::voltage-dc-res");
autoreg::reg<voltage_source::ac_res_t> ac_res_vs_reg(database::components(), "sources::voltage-ac-res");
autoreg::reg<voltage_source::dc_t>         vs_dc_reg(database::components(), "sources::voltage-dc");
autoreg::reg<voltage_source::ac_t>         ac_vs_reg(database::components(), "sources::voltage-ac");

} // namespace voltage_source
