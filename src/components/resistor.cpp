// INCLUDE <>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"
#include "../simulator/printable.hpp"

static constexpr const char * resistor_help_string = 
R"---(resistor V1.0 2021-09-05

|------------|--------|
|  argument  |  type  |
|============|========|
| node +     | string |
| node -     | string |
| resistance | double |
|------------|--------|

printables:
- i: current [A]
    The current flowing through the component.
    A current flowing from the positive node to the
    negative one has a positive value.
)---";

class resistor_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        resistance_ = next_as<double>(args);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
    }
    
    void update(double, double) override {
        double vd = voltage_difference(node_positive_, node_negative_);
        current_ = vd / resistance_;
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
    const char * help() const override {
        return resistor_help_string;
    }
    
private:
    // sim var
    double current_ = 0;
    
    // params
    double resistance_ = 1000;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<resistor_t> resistor_reg(database::components(), "resistor");
