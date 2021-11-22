// INCLUDE <>
#include <cstdint>
#include <cmath>
#include <numbers>

#include <cstdio>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

/*
class capacitor_t : public comp::component_t {
public:
    void calculate_constants(double time_step_max, double voltage_difference_max) override {
        vd_max_ = voltage_difference_max;
        ts_max_ = time_step_max;
    }
    
    void update(double) override { }
    
    double derivative_to_target_voltage() {
        double diff = voltage_ - voltage_difference(node_positive_, node_negative_);
        //if (std::abs(diff) > vd_max_) printf("[vs clip   % .2e]", std::abs(diff) - vd_max_);
        //else printf("[vs smooth % .2e]", std::abs(diff) - vd_max_);
        if (std::abs(diff) > vd_max_) return std::copysign(slew_rate_, diff);
        else                          return diff / ts_max_;
    }
    
    void compensate_currents(double, double time_step) override {
        double cp = node_positive_->capacitance();
        double cn = node_negative_->capacitance();
        
        current_ = (node_positive_->voltage_derivative() - node_negative_->voltage_derivative() - derivative_to_target_voltage()) / comp::utils::invsum(cp, cn);
        
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
    void accumulate_charge_on_nodes(double dt) override {
        double charge = current_ * dt;
        voltage_ += charge / capacitance_;
        node_positive_->decrement_charge(charge);
        node_negative_->increment_charge(charge);
    }
    
    component_t * configure(node_map_type & nmap, std::string args) override {
        node_positive_ = nmap[next(args)].get();
        node_negative_ = nmap[next(args)].get();
        capacitance_   = next_as<double>(args);
        slew_rate_     = next_as<double>(args);
        if(capacitance_ == 0) capacitance_ = 1;
        if(slew_rate_ == 0)   slew_rate_   = 1e7;
        return this;
    }
    
private:
    double voltage_ = 0;
    double slew_rate_ = 1e6;
    double current_;
    
    double charge_delta_ = 0;
    double vd_max_;
    double ts_max_;
    
    double capacitance_ = 1;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<capacitor_t> capacitor_reg(database::components(), "capacitor");
*/

class capacitor_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        capacitance_ = next_as<double>(args, 1e-3);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_positive_, node_negative_, this);
        // BUG:
        // node_positive_->increment_capacitance(capacitance_);
    }
    
    void clear_variables() override {
        charge_  = 0;
        current_ = 0;
    }
    
    void update(double, double) override { }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_positive_ && 
            neg == node_negative_
        ){
            return charge_ / capacitance_;
        }
        else if(
            pos == node_negative_ &&
            neg == node_positive_
        ){
            return -charge_ / capacitance_;
        }
        else{
            throw;
        }
    }
    
    double sn_capacitance(const node::node_t * /*pos*/, const node::node_t * /*neg*/) const override {
        return capacitance_;
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double time_step) override {
        if(
            pos == node_positive_ && 
            neg == node_negative_
        ){
            current_ = current;
            charge_ += current * time_step;
        }
        else if(
            pos == node_negative_ && 
            neg == node_positive_
        ){
            current_ = -current;
            charge_ -= current * time_step;
        }
        else{
            throw;
        }
    }
    
private:
    // sim var
    double charge_  = 0;
    double current_ = 0;
    
    // params
    double capacitance_   = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<capacitor_t> capacitor_reg(database::components(), "capacitor");
