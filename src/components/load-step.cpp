// INCLUDE <>
#include <vector>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"
#include "../simulator/printable.hpp"

class load_step_t : public comp::component_t {
public:
    struct info_t { double time; double value; }; // yeah yeah yeah I coud've written { double time, value; };
    
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        while(!args.empty()){
            double time       = next_as<double>(args);
            double resistance = next_as<double>(args);
            values_.push_back({time, resistance});
        }
        
        if(values_.empty()){
            info << "[ERROR] <configure> missing values after nodes\n";
            return info;
        }
        
        std::sort(values_.begin(),values_.end(), [](const info_t & lhs, const info_t & rhs){ return lhs.time < rhs.time; });
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        pos_ = values_.begin();
        current_ = 0;
    }
    
    void update(double time, double /*last_time_step*/) override {
        while(
            values_.end() != next(pos_)       &&
            time          >= next(pos_)->time
        ){
            ++pos_;
        }
        
        double vd = voltage_difference(node_positive_, node_negative_);
        
        // protect from start time > 0
        if(time >= pos_->time){ // maybe we can get a CMOV here?
            current_ = vd / pos_->value;
        }
        else{
            current_ = 0;
        }
        
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
    double time_step_max(double time) const override {
        if(next(pos_) != values_.end()){
            return next(pos_)->time - time;
        }
        else{
            return 0;
        }
    }
    
private:
    // sim var
    double current_ = 0;
    
    // params
    std::vector<info_t> values_;
    std::vector<info_t>::iterator pos_;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<load_step_t> load_step_reg(database::components(), "load::step");
