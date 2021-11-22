#include "supernode.hpp"

// INCLUDE <>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <eigen3/Eigen/Dense>

#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"

#ifdef DEBUG_SUPERNODES
#define PDEBUG(x) std::cout << x
#else
#define PDEBUG(x)
#endif

std::shared_ptr<sn::supernode_t> sn::make_supernode(){
    auto instance = std::make_shared<sn::supernode_t>();
    instance->instance_holder_ = instance;
    return instance;
}



// ----------------
// ---- PUBLIC ----
// ----------------



void sn::supernode_t::init(){
    PDEBUG(this << "->init()\n");
    
    update_current_dep_indep();
    
    const size_t nodes = node_list_.size();
    //const size_t deps  = count_current_dependent_;
    const size_t indep = count_current_independent_;
    const size_t total = nodes + indep;
    
    coefficients_extended_ = matrix_t::Zero(total, total);
    
    source_geometry_       = matrix_t::Zero(indep, nodes);
    source_impact_         = matrix_t::Zero(nodes, indep);
    
    input_                 = vector_t::Zero(total);
    output_                = vector_t::Zero(total);
    
    repair_matrix_         = matrix_t::Zero(indep, indep);
    repair_values_         = vector_t::Zero(indep);
    
    coefficients_extended_inverse_ = matrix_t(total, total);
    
    calculate_basic_matrices();
    calculate_repair_matrices();
}

void sn::supernode_t::add(comp::component_t * comp, node::node_t * pos, node::node_t * neg){
    PDEBUG(this << "->add()\n");
    
    link_t link = {comp, pos, neg, get_component_index(comp, pos, neg), get_node_index(pos), get_node_index(neg)};
    links_.push_back(link);
    pos->redirect_to_supernode(instance_holder_.lock());
    neg->redirect_to_supernode(instance_holder_.lock());
}

void sn::supernode_t::update(){
    const size_t node_count  = node_list_.size();
    
    for(size_t pos = 0; pos != node_count; pos++){
        input_[pos] = node_list_[pos]->current_in();
    }
    for(const auto [comp, node_p, node_n, index_comp, index_p, index_n] : links_){
        if(comp->sn_capacitance(node_p, node_n) == lim::infinity()){
            input_[index_comp + node_count] = -comp->sn_voltage_derivative(node_p, node_n);
        }
    }
    output_.noalias() = coefficients_extended_inverse_ * input_;
    
    for(size_t pos = 0; pos != node_count; pos++){
        node_list_[pos]->voltage_derivative(output_[pos]);
    }

#ifdef DEBUG
    iterations_since_last_fix++;
#endif
}

void sn::supernode_t::apply_iteration(const double time_step){
    const size_t extended_begin = node_list_.size();
    
    for(const auto [comp, node_p, node_n, index_comp, index_p, index_n] : links_){
        if(comp->sn_capacitance(node_p, node_n) != lim::infinity()){
            const double current = (node_p->voltage_derivative() - node_n->voltage_derivative()) * comp->sn_capacitance(node_p, node_n);
            comp->sn_update(node_p, node_n, current, time_step);
        }
        else{
            const double current = output_[index_comp + extended_begin];
            comp->sn_update(node_p, node_n, current, time_step);
        }
    }
}

void sn::supernode_t::fix_divergence(){
    const size_t extended_begin = node_list_.size();
    
    input_ *= 0;
    input_.block(extended_begin, 0, count_current_independent_, 1).noalias() = repair_matrix_ * repair_values_;
    output_ = coefficients_extended_inverse_ * input_;
    
    for(size_t pos = 0; pos != node_list_.size(); pos++){
        node_list_[pos]->increment_voltage(output_[pos]);
    }
    
#ifdef DEBUG
    std::cout << "[INFO] " << this << "->fix_divergence(): Supernode: Iterations since last fix: " << iterations_since_last_fix << "\n";
    iterations_since_last_fix = 0;
#endif
}

void sn::supernode_t::fix_divergence_if(const double error_max){
    if(is_error_greater_than(error_max)){
        fix_divergence();
    }
}

void sn::supernode_t::absorb(const supernode_t & supernode){
    PDEBUG(this << "->absorb(" << &supernode << ")\n");
    
    if(this != &supernode){
        for(auto & iter : supernode.links_){
            add(iter.comp, iter.node_pos, iter.node_neg);
        }
    }
    else{
        PDEBUG("Just a supernode loop, ignoring absorption...\n");
    }
}



// -----------------
// ---- PRIVATE ----
// -----------------



size_t sn::supernode_t::get_node_index(node::node_t * node){
    PDEBUG(this << "->get_node_index(" << node << ") = ");
    
    auto iter = std::find(node_list_.begin(), node_list_.end(), node);
    if(iter == node_list_.end()){
        node_list_.push_back(node);
        PDEBUG(node_list_.size() - 1 << "\n");
        return node_list_.size() - 1;
    }
    else{
        PDEBUG(std::distance(node_list_.begin(), iter) << "\n");
        return std::distance(node_list_.begin(), iter);
    }
}
size_t sn::supernode_t::get_component_index(comp::component_t * comp, node::node_t * pos, node::node_t * neg){
    PDEBUG(this << "->get_comp_index() = ");
    
    if(comp->sn_capacitance(pos, neg) == lim::infinity()){
        PDEBUG(component_index_ << "\n");
        return component_index_++;
    }
    else{
        PDEBUG("end\n");
        return std::numeric_limits<size_t>::max();
    }
}

void sn::supernode_t::calculate_basic_matrices(){
    PDEBUG(this << "->calculate_basic_matrices()\n");
    
    const size_t extended = node_list_.size();
    
    for(size_t pos = 0; pos != node_list_.size(); pos++){
        coefficients_extended_(pos, pos) = node_list_[pos]->capacitance();
    }
    for(const auto [comp, node_p, node_n, index_comp, index_p, index_n] : links_){
        
        const bool pos_is_not_gnd = !node_p->is_ground();
        const bool neg_is_not_gnd = !node_n->is_ground();
        
        const double capacitance = comp->sn_capacitance(node_p, node_n);
        const size_t index_comp_extended = index_comp + extended;
        
        if(capacitance != lim::infinity()){
            if(pos_is_not_gnd){
                coefficients_extended_(index_p, index_p) += capacitance;
            }
            if(neg_is_not_gnd){
                coefficients_extended_(index_n, index_n) += capacitance;
            }
            if(pos_is_not_gnd && neg_is_not_gnd){
                coefficients_extended_(index_p, index_n) -= capacitance;
                coefficients_extended_(index_n, index_p) -= capacitance;
            }
        }
        else{
            if(pos_is_not_gnd){
                coefficients_extended_(index_p, index_comp_extended) -= 1;
                coefficients_extended_(index_comp_extended, index_p) -= 1;
                
                source_geometry_(index_comp, index_p) += 1;
            }
            if(neg_is_not_gnd){
                coefficients_extended_(index_n, index_comp_extended) += 1;
                coefficients_extended_(index_comp_extended, index_n) += 1;
                
                source_geometry_(index_comp, index_n) -= 1;
            }
        }
    }
    /*
    if(coefficients_extended_.determinant() == 0){
        
        coefficients_extended_.block(0, 0, extended, extended) += matrix_t::Identity(extended, extended);
    }
    */
    
    PDEBUG("source_geometry:\n" << source_geometry_ << "\n");
    PDEBUG("coefficients_extended:\n" << coefficients_extended_ << "\n");
    
    coefficients_extended_inverse_ = coefficients_extended_.inverse();
    
    PDEBUG("coefficients_extended_inverse_:\n" << coefficients_extended_inverse_ << "\n");
}

void sn::supernode_t::calculate_repair_matrices(){
    PDEBUG(this << "->calculate_repair_matrices()\n");
    
    const size_t extended = node_list_.size();
    
    for(const auto [comp, node_p, node_n, index_comp, index_p, index_n] : links_){
        const double capacitance = comp->sn_capacitance(node_p, node_n);
        const size_t index_comp_extended = index_comp + extended;
        
        if(capacitance == lim::infinity()){
            input_ *= 0;
            input_[index_comp_extended] = 1;
            
            //std::cout << "Coef size: [" << coefficients_.rows() << "," << coefficients_.cols() << "]\n"; 
            
            source_impact_.col(index_comp) = (coefficients_extended_inverse_ * input_).block(0, 0, extended, 1);
        }
    }
    
    PDEBUG("source_impact:\n" << source_impact_ << "\n");
    
    repair_matrix_ = (source_geometry_ * source_impact_).inverse();
    
    PDEBUG("repair_matrix:\n"  << repair_matrix_ << "\n");
}

void sn::supernode_t::update_current_dep_indep(){
    PDEBUG(this << "->update_current_dep_indep()\n");
    
    count_current_dependent_   = 0;
    count_current_independent_ = 0;
    
    for(const auto & link : links_){
        const double capacitance = link.comp->sn_capacitance(link.node_pos, link.node_neg);
        
        if(capacitance != lim::infinity()){
            count_current_dependent_++;
        }
        else{
            count_current_independent_++;
        }
    }
}

bool sn::supernode_t::is_error_greater_than(const double error_max){
    bool is_greater = false;
    
    for(const auto [comp, node_p, node_n, index_comp, index_p, index_n] : links_){
        if(comp->sn_capacitance(node_p, node_n) == lim::infinity()){
            double error = comp->sn_voltage_error(node_p, node_n);
            repair_values_[index_comp] = error;
            is_greater = is_greater || (std::abs(error) > error_max);
            if(is_greater) break;
        }
    }
    
    return is_greater;
}
