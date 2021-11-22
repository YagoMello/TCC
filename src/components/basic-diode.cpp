// INCLUDE <>
#include <cmath>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

class basic_diode_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), anode_, error);
        info << utils::find_node_s(nmap, next(args), cathode_, error);
        if(error) return info;
        
        r_on_    = next_as<double>(args, 0.23);
        v_diode_ = next_as<double>(args, 0.7);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", i_);
        
        return info;
    }
    
    void clear_variables() override {
        i_ = 0;
    }
    
    void update(double, double) override {
        double vd = voltage_difference(anode_, cathode_);
        
        if(vd > v_diode_)
            i_ = (vd - v_diode_) / r_on_;
        else{
            i_ = 0;
        }
        
        anode_->current_out(i_);
        cathode_->current_in(i_);
    }
    
private:
    // sim var
    double i_ = 0;
    
    // params
    double r_on_    = 0.05;
    double v_diode_ = 0.7;
    
    node::node_t * anode_;
    node::node_t * cathode_;
};

autoreg::reg<basic_diode_t> basic_diode_reg(database::components(), "basic-diode");
