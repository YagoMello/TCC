// INCLUDE <>
#include <cstdint>
#include <cmath>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/filter.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

namespace default_opamp {

static constexpr const char * simple_help_string = 
R"---(opamp::simple V1.0 2021-09-11

|------------|--------|---------|------|
|  argument  |  type  | default | unit |
|============|========|=========|======|
| node +     | string |         |      |
| node -     | string |         |      |
| node out   | string |         |      |
| volt. max  | double |      15 | V    |
| gain       | double |    100k |      |
| -3dB freq  | double |    1000 | Hz   |
| order      | size_t |       1 |      |
|------------|--------|---------|------|

=====[ printables ]=====
i: current [A]
  The current flowing through the component.
  A current flowing from the positive node to the
  negative one has a positive value.

=====[ CLI arguments ]=====
node +:
  The node connected to the noninverting input. Iin+ = 0.

node -:
  The node connected to the inverting input. Iin- = 0.

node out:
  The node connected to the output. Rout = 0.

volt max:
  The max amplitude of the output signal.

gain:
  The DC gain.

-3dB freq:
  The frequency to start attenuating the output signal.
  The opamp contains a low pass state space butterworth filter, this is
  the filter cutoff frequency. In Hz.

Order:
  The output filter order.

=====[ description ]=====
The output voltage is calculated as
  LOWPASS(CLIP(gain * (V+ - V-)))
)---";
class opamp_simple_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_noninvert_, error);
        info << utils::find_node_s(nmap, next(args), node_invert_, error);
        info << utils::find_node_s(nmap, next(args), node_output_, error);
        if(error) return info;
        
        voltage_max_ = next_as<double>(args, 15);
        gain_        = next_as<double>(args, 100e3);
        
        double cutoff = next_as<double>(args, 1000);
        size_t order  = next_as<size_t>(args, 1);
        
        voltage_out_.configure(order, cutoff);
        
        info << utils::make_node_s(nmap, this->label() + "opamp internal ground", node_internal_, error);
        if(error) return info;
        
        node_internal_->is_ground(true);
        node_internal_->is_hidden(true);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        voltage_difference_ = 0;
        current_ = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_internal_, node_output_, this);
    }
    
    void update(double, double last_time_step) override {
        voltage_out_.add_sample(voltage_difference_, last_time_step);
        voltage_difference_ = gain_ * voltage_difference(node_noninvert_, node_invert_);
        if(std::abs(voltage_difference_) > voltage_max_){
            voltage_difference_ = std::copysign(voltage_max_, voltage_difference_);
        }
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_output_ && 
            neg == node_internal_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_internal_ &&
            neg == node_output_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    double sn_voltage_derivative(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_output_ && 
            neg == node_internal_
        ){
            return voltage_out_.derivative();
        }
        else if(
            pos == node_internal_ &&
            neg == node_output_
        ){
            return -voltage_out_.derivative();
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_output_ && 
            neg == node_internal_
        ){
            current_ = current;
        }
        else if(
            pos == node_internal_ &&
            neg == node_output_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return simple_help_string;
    }
    
private:
    // sim var
    filt::lpfd_t voltage_out_;
    double voltage_difference_ = 0;
    double current_ = 0;
    
    // params
    double voltage_max_ = 15;
    double gain_        = 100e3;
    
    // nodes
    node::node_t * node_noninvert_;
    node::node_t * node_invert_;
    node::node_t * node_output_;
    
    node::node_t * node_internal_;
};

static constexpr const char * real_help_string = 
R"---(opamp::real V1.0 2021-09-11

|------------|--------|---------|------|
|  argument  |  type  | default | unit |
|============|========|=========|======|
| node +     | string |         |      |
| node -     | string |         |      |
| node out   | string |         |      |
| node pow + | string |         |      |
| node pow - | string |         |      |
| volt. max  | double |      15 | V    |
| gain       | double |    100k |      |
| -3dB freq  | double |    1000 | Hz   |
| order      | size_t |       1 |      |
|------------|--------|---------|------|

=====[ printables ]=====
i: current [A]
  The current flowing through the component.
  A current flowing from the positive node to the
  negative one has a positive value.

=====[ CLI arguments ]=====
node +:
  The node connected to the noninverting input. Iin+ = 0.

node -:
  The node connected to the inverting input. Iin- = 0.

node out:
  The node connected to the output. Rout = 0.

node pow +:
  The node connected to the positive power input, determines max output voltage.
  If Iout > 0, Ipow+ = Iout.

node pow -:
  The node connected to the negative power input, determines min output voltage.
  If Iout < 0, Ipow- = Iout.

volt max:
  The max amplitude of the output signal.

gain:
  The DC gain.

-3dB freq:
  The frequency to start attenuating the output signal.
  The opamp contains a low pass state space butterworth filter, this is
  the filter cutoff frequency. In Hz.

Order:
  The output filter order.

The output voltage is calculated as
  LOWPASS(CLIP(gain * (V+ - V-)))
)---";

class opamp_real_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_noninvert_, error);
        info << utils::find_node_s(nmap, next(args), node_invert_, error);
        info << utils::find_node_s(nmap, next(args), node_output_, error);
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        gain_           = next_as<double>(args, 100e3);
        resistance_     = next_as<double>(args, 100);
        
        double cutoff = next_as<double>(args, 1000);
        size_t order  = next_as<size_t>(args, 1);
        
        voltage_out_.configure(order, cutoff);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        voltage_difference_ = 0;
        current_ = 0;
    }
    
    void update(double, double last_time_step) override {
        voltage_out_.add_sample(voltage_difference_, last_time_step);
        voltage_difference_ = gain_ * voltage_difference(node_noninvert_, node_invert_);
        
        if(voltage_difference_ > node_positive_->voltage()){
            voltage_difference_ = node_positive_->voltage();
        }
        else if(voltage_difference_ < node_negative_->voltage()){
            voltage_difference_ = node_negative_->voltage();
        }
        
        current_ = (voltage_out_ - node_output_->voltage()) / resistance_;
        
        node_output_->current_in(current_);
        
        if(current_ > 0){
            node_positive_->current_out(current_);
        }
        else{
            node_negative_->current_out(current_);
        }
    }
    
    const char * help() const override {
        return real_help_string;
    }
    
private:
    // sim var
    filt::lpfd_t voltage_out_;
    double voltage_difference_ = 0;
    double current_ = 0;
    
    // params
    double gain_;
    double resistance_;
    
    node::node_t * node_noninvert_;
    node::node_t * node_invert_;
    node::node_t * node_output_;
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<default_opamp::opamp_simple_t> opamp_simple_reg(database::components(), "opamp::simple");
autoreg::reg<default_opamp::opamp_real_t> opamp_real_reg(database::components(), "opamp::real");

} // namespace default_opamp
