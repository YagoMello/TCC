// INCLUDE <>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

class inductor_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        inductance_ = next_as<double>(args, 10e-3);
        resistance_ = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
        vd_last_iteration_ = 0;
    }
    
    void update(double, double last_time_step) override {
        current_ = (vd_last_iteration_ - current_ * resistance_) * last_time_step / inductance_ + current_;
        vd_last_iteration_ = voltage_difference(node_positive_, node_negative_);
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
private:
    // sim var
    double current_ = 0;
    double vd_last_iteration_ = 0;
    
    // params
    double inductance_ = 1;
    double resistance_ = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
    
};

autoreg::reg<inductor_t> ind_reg(database::components(), "inductor");
