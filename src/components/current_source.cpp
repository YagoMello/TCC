// INCLUDE <>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

class current_source_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_from_, error);
        info << utils::find_node_s(nmap, next(args), node_to_, error);
        if(error) return info;
        
        current_ = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void update(double, double) override {
        node_from_->current_out(current_);
        node_to_->current_in(current_);
    }
    
private:
    // params
    double current_ = 0;
    
    node::node_t * node_from_;
    node::node_t * node_to_;
};

autoreg::reg<current_source_t> current_source_reg(database::components(), "sources::current-dc");
