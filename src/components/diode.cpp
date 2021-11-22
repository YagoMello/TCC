// INCLUDE <>
#include <cmath>
#include <boost/math/special_functions/lambert_w.hpp>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

/* 
 * Sources:
 * https://en.wikipedia.org/wiki/Diode_modelling
 * https://www.boost.org/doc/libs/develop/libs/math/doc/html/math_toolkit/lambert_w.html
 * 
 */

class diode_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        resistance_ = next_as<double>(args, 0.2);
        double temp = 273.15 + next_as<double>(args, 25);
        double n    = next_as<double>(args, 1.0);
        i_sat_      = next_as<double>(args, 1e-14);
        
        nvt_ = n * boltzmann * temp / charge_unit;
        
        isat_r_div_nvt_ = i_sat_ * resistance_ / nvt_;
        nvt_div_r_      = nvt_ / resistance_;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        double vd = voltage_difference(node_positive_, node_negative_);
        
        double arg = isat_r_div_nvt_ * exp(vd/nvt_ + isat_r_div_nvt_);
        double lambert_w = boost::math::lambert_w0(arg);
        current_ = nvt_div_r_*lambert_w - i_sat_;
        
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
private:
    // sim var
    double current_ = 0;
    
    // params
    double resistance_;
    double i_sat_;
    double nvt_;
    double isat_r_div_nvt_;
    double nvt_div_r_;
    
    // consts
    static constexpr double boltzmann   = 1.38e-23;
    static constexpr double charge_unit = 1.6e-19;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

class pn_junction_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_positive_, error);
        info << utils::find_node_s(nmap, next(args), node_negative_, error);
        if(error) return info;
        
        i_sat_    = next_as<double>(args, 1e-14);
        vt_       = next_as<double>(args, 26e-3);
        didt_max_ = next_as<double>(args, 1e9);
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        current_ = 0;
        di_      = 0;
    }
    
    void update(double, double last_time_step) override {
        current_ += limit_current_difference(di_, last_time_step, didt_max_);
        
        double vd = voltage_difference(node_positive_, node_negative_);
        double i = i_sat_*(exp(vd/vt_) - 1);
        di_ = i - current_;
        
        node_positive_->current_out(current_);
        node_negative_->current_in(current_);
    }
    
private:
    // sim var
    double current_ = 0;
    double di_      = 0;
    
    // params
    double i_sat_ = 1e-14;
    double vt_    = 26e-3;
    double didt_max_ = 1e9;
    
    node::node_t * node_positive_;
    node::node_t * node_negative_;
};

autoreg::reg<diode_t> diode_reg(database::components(), "diode");
autoreg::reg<pn_junction_t> pn_juntcion_reg(database::components(), "old::diode");
