#ifndef DC_MOTOR_HPP
#define DC_MOTOR_HPP

// INCLUDE <>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

// sauce http://ocw.nctu.edu.tw/course/dssi032/DSSI_2.pdf

class dc_motor_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        ra_ = next_as<double>(args, 0.5);
        la_ = next_as<double>(args, 1.5e-3);
        kt_ = next_as<double>(args, 0.05);
        jm_ = next_as<double>(args, 2.5e-4);
        te_ = next_as<double>(args, 0);
        bm_ = next_as<double>(args, 0);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", i_);
        info << utils::add_printable_s(pmap, label() + ":w", "rad/s", w_);
        
        return info;
    }
    
    void clear_variables() override {
        i_  = 0;
        di_ = 0;
        w_  = 0;
        dw_ = 0;
    }
    
    void update(double, double last_time_step) override {
        i_ += di_ * last_time_step;
        w_ += dw_ * last_time_step;
        
        node_positive_->current_out(i_);
        node_negative_->current_in(i_);
        
        double vd = voltage_difference(node_positive_, node_negative_);
        
        di_ = (-ra_*i_ - kt_*w_ + vd ) / la_;
        dw_ = ( kt_*i_ - bm_*w_ - te_) / jm_;
    }
    
    double w() const {
        return w_;
    }
    
private:
    // sim var
    double i_  = 0;
    double di_ = 0;
    double w_  = 0;
    double dw_ = 0;
    
    // params
    double ra_ = 0.5;
    double la_ = 1.5e-3;
    double kt_ = 0.05;
    double jm_ = 2.5e-4;
    double te_ = 0;
    double bm_ = 0;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

#endif // DC_MOTOR_HPP
