#ifndef DATA_FILTER_HPP
#define DATA_FILTER_HPP

// INCLUDE ""
#include "objects.hpp"
#include "../simulator/node.hpp"
#include "../simulator/printable.hpp"

class data_filter_t {
public:
    void clear(){
        nodes_.clear();
        variables_.clear();
    }
    void assign_variables(const std::vector<node::node_t *> & node_vector, const std::vector<printable::var_info_t *> & info_vector){
        for(auto & node : node_vector){
            nodes_.push_back(printable::make_from_var(node->label(), "V", node->voltage_data()));
            nodes_.back().is_active = !node->is_hidden();
        }
        for(auto & info : info_vector){
            variables_.push_back(printable::make_from_info(*info));
        }
        for(auto & data : nodes_){
            data.printmode = print_mode_;
        }
        for(auto & data : variables_){
            data.printmode = print_mode_;
        }
        for(auto & data : nodes_){
            data.precision = precision_;
        }
        for(auto & data : variables_){
            data.precision = precision_;
        }
    }
    void print_mode(const print::mode value){
        print_mode_ = value;
        
        for(auto & data : nodes_){
            data.printmode = print_mode_;
        }
        for(auto & data : variables_){
            data.printmode = print_mode_;
        }
    }
    void precision(uint16_t value){
        precision_ = value;
        
        for(auto & data : nodes_){
            data.precision = precision_;
        }
        for(auto & data : variables_){
            data.precision = precision_;
        }
    }
    void clear_print_memory(){
        for(auto & data : nodes_){
            data.clear_print_memory(data);
        }
        for(auto & data : variables_){
            data.clear_print_memory(data);
        }
    }
    void apply_iteration(const double time_step){
        for(auto & data : nodes_){
            data.apply_iteration(data, time_step);
        }
        for(auto & data : variables_){
            data.apply_iteration(data, time_step);
        }
    }
    std::vector<printable::var_data_t> & nodes(){
        return nodes_;
    }
    std::vector<printable::var_data_t> & variables(){
        return variables_;
    }
private:
    uint16_t precision_ = 10;
    print::mode print_mode_ = print::mode::UNDEFINED;
    std::vector<printable::var_data_t> nodes_;
    std::vector<printable::var_data_t> variables_;
};

#endif // DATA_FILTER_HPP
