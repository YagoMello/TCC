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

namespace current_probe {

static constexpr const char * ammeter_help_string = 
R"---(ammeter V1.0 2021-10-29

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| shared var | string |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class ammeter_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &ls, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        if(error) return info;
        
        info << utils::find_node_s(nmap, next(args), node_negative_, error); 
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        current_.create(ls, next(args));
        
        return info;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*time_step_max*/, const double /*voltage_difference_max*/) override {
        bind(node_positive_, node_negative_, this);
        current_.init();
    }
    
    void update(double, double) override { }
    
    double sn_voltage(const node::node_t *, const node::node_t *) const override {
        return 0;
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
        return ammeter_help_string;
    }
    
private:
    // sim var
    
    // shared_var
    shared::var<double> current_;
    
    // params
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<current_probe::ammeter_t> ammeter_reg(database::components(), "ammeter");

} // namespace default_wire
