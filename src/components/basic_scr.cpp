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

class basic_scr_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), anode_, error);
        info << utils::find_node_s(nmap, next(args), cathode_, error);
        info << utils::find_node_s(nmap, next(args), gate_, error);
        if(error) return info;
        
        r_on_    = next_as<double>(args, 0.1);
        v_diode_ = next_as<double>(args, 0.7);
        r_gate_  = next_as<double>(args, 50);
        
        info << utils::add_printable_s(pmap, label() + ":ia", "A", ia_);
        info << utils::add_printable_s(pmap, label() + ":ig", "A", ig_);
        
        return info;
    }
    
    void clear_variables() override {
        ia_ = 0;
        ig_ = 0;
        on_ = false;
    }
    
    void update(double, double) override {
        double vak = voltage_difference(anode_, cathode_);
        double vgk = voltage_difference(gate_, cathode_);
        
        if(vgk > v_diode_){
            ig_ = (vgk - v_diode_) / r_gate_;
        }
        else{
            ig_ = 0;
        }
        
        if(
            vak > 0 && 
            (
                on_ || 
                ig_ > 0
            )
        ){
            ia_ = vak / r_on_;
            on_ = true;
        }
        else{
            ia_ = 0;
            on_ = false;
        }
        
        anode_->current_out(ia_);
        cathode_->current_in(ia_ + ig_);
        gate_->current_out(ig_);
    }
    
private:
    // sim var
    double ia_ = 0;
    double ig_ = 0;
    bool   on_ = false;
    
    // params
    double r_on_    = 0.05;
    double v_diode_ = 0.7;
    double r_gate_  = 50;
    
    node::node_t * anode_;
    node::node_t * cathode_;
    node::node_t * gate_;
};

autoreg::reg<basic_scr_t> basic_scr_reg(database::components(), "basic-scr");
