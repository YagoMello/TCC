#ifndef NODE_HPP
#define NODE_HPP

// INCLUDE <>
#include <cstdint>
#include <string>
#include <limits>
#include <cstdio>
#include <iostream>
#include <memory>

// INCLUDE ""
#include "objects.hpp"
#include "./component.hpp"
#include "./supernode.hpp"
#include "./print_mode.hpp"

namespace node {

class node_t {
public:
    // MAIN FUNCTIONS
    
    void begin_iteration(){
         current_in_ = 0;
    }
    
    void apply_iteration(const double time_step){
#ifdef DEBUG_ITERATIONS
        double voltage_old = voltage_;
#endif
        
        charge_ += time_step * current_in_;
        voltage_ = charge_ / capacitance();
        
#ifdef DEBUG_ITERATIONS
        printf("[%s: % .2e (%+.2e)]", label_.c_str(), voltage_, voltage_ - voltage_old);
#endif
        //printf("%s: %f\n", label, voltage);
    }
    
    void print_voltage() const {
        printf("%10s: % .4e\n", label_.c_str(), voltage());
    }
    
    void redirect_to_supernode(const std::shared_ptr<sn::supernode_t> & supernode) {
        supernode_ = supernode;
    }
    
    [[nodiscard]] std::shared_ptr<sn::supernode_t> & supernode() {
        return supernode_;
    }
    
    [[nodiscard]] bool is_redirecting_to_supernode() const {
        return supernode_ != nullptr;
    }
    
    // used by supernodes
    void increment_voltage(const double value){
        charge_ += capacitance_ * value;
        voltage_ = charge_ / capacitance();
    }
    
    // ACCESSORS
    
    void label(const std::string & label) {
        label_ = label;
    }
    
    [[nodiscard]] const std::string & label() const {
        return label_;
    }

    [[nodiscard]] inline double voltage() const {
        return voltage_;
    }
    
    void capacitance(const double value){
        capacitance_ = value;
    }
    
    [[nodiscard]] inline double capacitance() const {
        if(is_ground_){
            return std::numeric_limits<double>::infinity();
        }
        else{
            return capacitance_;
        }
    }
    
    // Used for printing info about the nodes
    
    [[nodiscard]] inline double internal_capacitance() const {
        return capacitance_;
    }
    
    // Node info & status
    // I need to organize this mess some day
    
    void is_ground(const bool value){
        is_ground_ = value;
    }
    
    [[nodiscard]] bool is_ground() const {
        return is_ground_;
    }
    
    void voltage_derivative(const double value) {
        current_in_ = capacitance_ * value;
    }
    
    [[nodiscard]] double voltage_derivative() const {
#ifndef DEBUG_TS
        return current_in_ / capacitance();
#else
        double value = current_in_ / capacitance_;
        printf("(%s % .2e)", label_.c_str(), value);
        return value;
#endif
    }
    
    void voltage_difference_max(const double value) {
        voltage_difference_max_ = value;
        is_default_voltage_difference_max_ = false;
    }
    
    [[nodiscard]] double voltage_difference_max() const {
        return voltage_difference_max_;
    }
    
    void temporary_voltage_difference_max(const double value) {
        voltage_difference_max_ = value;
    }
    
    [[nodiscard]] bool is_default_voltage_difference_max() const {
        return is_default_voltage_difference_max_;
    }
    
    void current_in(const double value) {
#ifdef DEBUG_TS
        printf("{Node %s: % .2e}", label().c_str(), value);
#endif // DEBUG_TS
        current_in_ += value;
    }
    
    // used by supernodes
    [[nodiscard]] double current_in() const {
        return current_in_;
    }
    
    void current_out(const double value) {
#ifdef DEBUG_TS
        printf("{Node %s: % .2e}", label().c_str(), -value);
#endif // DEBUG_TS
        current_in_ -= value;
    }
    
    void is_hidden(const bool value) {
        is_hidden_ = value;
    }
    
    [[nodiscard]] bool is_hidden() const {
        return is_hidden_;
    }
    
    // used by data-filter/printable
    [[nodiscard]] double & voltage_data(){
        return voltage_;
    }
    
    // CLEAR
    
    void clear(){
        voltage_ = 0;
        charge_  = 0;
        supernode_ = nullptr;
    }
    
//private:
    // Printing
    std::string label_  = "";
    bool        is_hidden_ = false;
    
    // Status
    double voltage_     = 0;
    double charge_      = 0;
    double current_in_  = 0;
    
    // Static Properties
    double capacitance_ = 0;
    bool   is_ground_   = false;
    bool   is_default_voltage_difference_max_ = true;
    double voltage_difference_max_ = 0;
    
    // Mutable properties
    std::shared_ptr<sn::supernode_t> supernode_ = nullptr;
    
    /*
    // helper functions
    static inline double inv_sum(const double lhs, const double rhs){
        return 1/(1/lhs + 1/rhs);
    }
    
    static inline double inv_dif(const double lhs, const double rhs){
        return 1/(1/lhs - 1/rhs);
    }
    */
};

// ADL should find this
inline double voltage_difference(const node_t * positive, const node_t * negative){
    return positive->voltage() - negative->voltage();
}

#undef PDEBUG
#ifdef DEBUG_SUPERNODES
#define PDEBUG(x) std::cout << x
#else
#define PDEBUG(x)
#endif

inline void bind(node_t * node1, node_t * node2, comp::component_t * component) {
    if(node1->is_redirecting_to_supernode() && node2->is_redirecting_to_supernode()){
        PDEBUG("[BIND] Both nodes redirect\n");
        node1->supernode()->absorb(*node2->supernode());
    }
    else if(node1->is_redirecting_to_supernode()){
        PDEBUG("[BIND] Node1 redirects\n");
        node2->redirect_to_supernode(node1->supernode());
    }
    else if(node2->is_redirecting_to_supernode()){
        PDEBUG("[BIND] Node2 redirects\n");
        node1->redirect_to_supernode(node2->supernode());
    }
    else{
        PDEBUG("[BIND] 0 nodes redirect\n");
        std::shared_ptr<sn::supernode_t> supernode = sn::make_supernode();
        node1->redirect_to_supernode(supernode);
        node2->redirect_to_supernode(supernode);
    }
    node1->supernode()->add(component, node1, node2);
}

}

#endif // NODE_HPP
