// INCLUDE <>
#include <cstdint>
#include <cmath>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/filter.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

class npn_old_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), base_, error);
        info << utils::find_node_s(nmap, next(args), collector_, error);
        info << utils::find_node_s(nmap, next(args), emitter_, error);
        if(error) return info;
        
        beta_f_ = next_as<double>(args, 100);
        beta_r_ = next_as<double>(args, 0.5);
        
        info << utils::add_printable_s(pmap, label() + ":ib", "A", ib_, error);
        info << utils::add_printable_s(pmap, label() + ":ic", "A", ic_, error);
        info << utils::add_printable_s(pmap, label() + ":ie", "A", ie_, error);
        if(error) return info;
        
        return info;
    }
    
    void clear_variables() override {
        ib_ = 0;
        ic_ = 0;
        ie_ = 0;
    }
    
    void update(double, double) override {
        double vbe = voltage_difference(base_, emitter_);
        double vbc = voltage_difference(base_, collector_);
        
        double evbet = exp(vbe/vt_);
        double evbct = exp(vbc/vt_);
        
        double ic = i_sat_ * ((evbet - evbct)           - (evbct - 1) / beta_r_);
        double ib = i_sat_ * ((evbet -     1) / beta_f_ + (evbct - 1) / beta_r_);
        double ie = -(ic + ib);
        
        double dilute = 0.5 / beta_f_;
        double retain = 1 - dilute;
        
        ic_ = dilute * ic + retain * ic_;
        ib_ = dilute * ib + retain * ib_;
        ie_ = dilute * ie + retain * ie_; 
        
        base_     ->current_out(ib_);
        collector_->current_out(ic_);
        emitter_  ->current_out(ie_);
    }
    
private:
    // sim var
    double ib_ = 0;
    double ic_ = 0;
    double ie_ = 0;
    
    // params
    double beta_f_ = 100;
    double beta_r_ = 0.5;
    double i_sat_  = 1e-13;
    double vt_     = 26e-3;
    
    node::node_t * base_;
    node::node_t * collector_;
    node::node_t * emitter_;
};

class pnp_old_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), base_, error);
        info << utils::find_node_s(nmap, next(args), collector_, error);
        info << utils::find_node_s(nmap, next(args), emitter_, error);
        if(error) return info;
        
        beta_f_ = next_as<double>(args, 100);
        beta_r_ = next_as<double>(args, 0.5);
        
        info << utils::add_printable_s(pmap, label() + ":ib", "A", ib_, error);
        info << utils::add_printable_s(pmap, label() + ":ic", "A", ic_, error);
        info << utils::add_printable_s(pmap, label() + ":ie", "A", ie_, error);
        if(error) return info;
        
        return info;
    }
    
    void clear_variables() override {
        ib_ = 0;
        ic_ = 0;
        ie_ = 0;
    }
    
    void update(double, double) override {
        double vbe = voltage_difference(base_, emitter_);
        double vbc = voltage_difference(base_, collector_);
        
        double evbet = exp(-vbe/vt_);
        double evbct = exp(-vbc/vt_);
        
        double ic = -i_sat_ * ((evbet - evbct)           - (evbct - 1) / beta_r_);
        double ib = -i_sat_ * ((evbet -     1) / beta_f_ + (evbct - 1) / beta_r_);
        double ie = -(ic + ib);
        
        double dilute = 0.5 / beta_f_;
        double retain = 1 - dilute;
        
        ic_ = dilute * ic + retain * ic_;
        ib_ = dilute * ib + retain * ib_;
        ie_ = dilute * ie + retain * ie_; 
        
        base_     ->current_out(ib_);
        collector_->current_out(ic_);
        emitter_  ->current_out(ie_);
    }
    
private:
    // sim var
    double ib_ = 0;
    double ic_ = 0;
    double ie_ = 0;
    
    // params
    double beta_f_ = 100;
    double beta_r_ = 0.5;
    double i_sat_  = 1e-13;
    double vt_     = 26e-3;
    
    node::node_t * base_;
    node::node_t * collector_;
    node::node_t * emitter_;
};

autoreg::reg<npn_old_t> npn_old_reg(database::components(), "npn-legacy");
autoreg::reg<pnp_old_t> pnp_old_reg(database::components(), "pnp-legacy");

class npn_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), base_, error);
        info << utils::find_node_s(nmap, next(args), collector_, error);
        info << utils::find_node_s(nmap, next(args), emitter_, error);
        if(error) return info;
        
        beta_f_   = next_as<double>(args, 100);
        beta_r_   = next_as<double>(args, 0.5);
        freq_max_ = next_as<double>(args, 1e9);
        
        ic_.configure(freq_max_);
        ib_.configure(freq_max_);
        
        info << utils::add_printable_s(pmap, label() + ":ib", "A", ib_, error);
        info << utils::add_printable_s(pmap, label() + ":ic", "A", ic_, error);
        info << utils::add_printable_s(pmap, label() + ":ie", "A", ie_, error);
        if(error) return info;
        
        return info;
    }
    
    void clear_variables() override {
        ib_.clear();
        ic_.clear();
        ie_  = 0;
        dic_ = 0;
        dib_ = 0;
    }
    
    void update(double, double last_time_step) override {
        // integrate last step
        ic_.add_sample(dic_, last_time_step);
        ib_.add_sample(dib_, last_time_step);
        ie_  = -(ic_ + ib_);
        
        base_     ->current_out(ib_);
        collector_->current_out(ic_);
        emitter_  ->current_out(ie_);
        
        // find this step values
        double vbe = voltage_difference(base_, emitter_);
        double vbc = voltage_difference(base_, collector_);
        
        double evbet = exp(vbe/vt_);
        double evbct = exp(vbc/vt_);
        
        double ic = i_sat_ * ((evbet - evbct)           - (evbct - 1) / beta_r_);
        double ib = i_sat_ * ((evbet -     1) / beta_f_ + (evbct - 1) / beta_r_);
        
        dic_ = ic - ic_;
        dib_ = ib - ib_;
    }
    
private:
    // sim var
    filt::lpf_t<3> ib_;
    filt::lpf_t<3> ic_;
    double ie_ = 0;
    double dic_ = 0;
    double dib_ = 0;
    
    // params
    double beta_f_ = 100;
    double beta_r_ = 0.5;
    double i_sat_  = 1e-13;
    double vt_     = 26e-3;
    double freq_max_ = 1e9;
    
    node::node_t * base_;
    node::node_t * collector_;
    node::node_t * emitter_;
};

class pnp_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), base_, error);
        info << utils::find_node_s(nmap, next(args), collector_, error);
        info << utils::find_node_s(nmap, next(args), emitter_, error);
        if(error) return info;
        
        beta_f_   = next_as<double>(args, 100);
        beta_r_   = next_as<double>(args, 0.5);
        freq_max_ = next_as<double>(args, 1e9);
        
        ic_.configure(freq_max_);
        ib_.configure(freq_max_);
        
        info << utils::add_printable_s(pmap, label() + ":ib", "A", ib_, error);
        info << utils::add_printable_s(pmap, label() + ":ic", "A", ic_, error);
        info << utils::add_printable_s(pmap, label() + ":ie", "A", ie_, error);
        if(error) return info;
        
        return info;
    }
    
    void clear_variables() override {
        ib_.clear();
        ic_.clear();
        ie_  = 0;
        dic_ = 0;
        dib_ = 0;
    }
    
    void update(double, double last_time_step) override {
        ic_.add_sample(dic_, last_time_step);
        ib_.add_sample(dib_, last_time_step);
        ie_  = -(ic_ + ib_);
        
        base_     ->current_out(ib_);
        collector_->current_out(ic_);
        emitter_  ->current_out(ie_);
        
        double vbe = voltage_difference(base_, emitter_);
        double vbc = voltage_difference(base_, collector_);
        
        double evbet = exp(-vbe/vt_);
        double evbct = exp(-vbc/vt_);
        
        double ic = -i_sat_ * ((evbet - evbct)           - (evbct - 1) / beta_r_);
        double ib = -i_sat_ * ((evbet -     1) / beta_f_ + (evbct - 1) / beta_r_);
        
        dic_ = ic - ic_;
        dib_ = ib - ib_;
    }
    
private:
    // sim var
    filt::lpf_t<3> ib_;
    filt::lpf_t<3> ic_;
    double ie_ = 0;
    double dic_ = 0;
    double dib_ = 0;
    
    // params
    double beta_f_ = 100;
    double beta_r_ = 0.5;
    double i_sat_  = 1e-13;
    double vt_     = 26e-3;
    double freq_max_ = 1e9;
    
    node::node_t * base_;
    node::node_t * collector_;
    node::node_t * emitter_;
};

autoreg::reg<npn_t> npn_reg(database::components(), "npn");
autoreg::reg<pnp_t> pnp_reg(database::components(), "pnp");
