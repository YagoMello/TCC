// INCLUDE <>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

// equations (reference): https://www.maplesoft.com/content/EngineeringFundamentals/19/MapleDocument_19/Coupled%20Inductors.pdf

class transformer_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        info << utils::find_node_s(nmap, next(args), node_primary_in_, error);
        info << utils::find_node_s(nmap, next(args), node_primary_out_, error);
        info << utils::find_node_s(nmap, next(args), node_secondary_in_, error);
        info << utils::find_node_s(nmap, next(args), node_secondary_out_, error);
        if(error) return info;
        
        inductance_primary_    = next_as<double>(args, 100e-3);
        double turns_ratio     = next_as<double>(args, 1);
        double coupling_factor = next_as<double>(args, 0.9999);
        resistance_primary_    = next_as<double>(args, 100e-3);
        resistance_secondary_  = next_as<double>(args, 100e-3);
        
        inductance_secondary_ = turns_ratio * turns_ratio * inductance_primary_;
        
        const double inductance_mutual = coupling_factor * std::sqrt(inductance_primary_ * inductance_secondary_);
        const double inductance_product = inductance_primary_ * inductance_secondary_;
        const double coupling_factor_squared = coupling_factor * coupling_factor;
        
        // these constants were calculated dividing
        // dI1/dt = (V1 - M*dI2/dt)/L1 => Obtained from V1 = L1*dI1/dt + M*dI2/dt
        // by
        // dI1/dt = (V2 - L2*dI2/dt)/M => Obtained from V2 = M*dI1/dt + L2*dI2/dt
        // = 
        // dI1/dt = ( L2*V1 - M*V2)(L1*L2*(1 - k*k))
        // dI1/dt = (-M*V1 + L1*V2)(L1*L2*(1 - k*k))
        constant_primary_primary_     =  inductance_secondary_ /(inductance_product * (1 - coupling_factor_squared));
        constant_primary_secondary_   = -inductance_mutual     /(inductance_product * (1 - coupling_factor_squared));
        constant_secondary_primary_   = -inductance_mutual     /(inductance_product * (1 - coupling_factor_squared));
        constant_secondary_secondary_ =  inductance_primary_   /(inductance_product * (1 - coupling_factor_squared));
        
        info << utils::add_printable_s(pmap, label() + ":ip", "A", current_primary_);
        info << utils::add_printable_s(pmap, label() + ":is", "A", current_secondary_);
        
        return info;
    }
    
    void clear_variables() override {
        current_primary_ = 0;
        current_secondary_ = 0;
        current_derivative_primary_ = 0;
        current_derivative_secondary_ = 0;
    }
    
    void update(double, double last_time_step) override {
        current_primary_ += current_derivative_primary_ * last_time_step;
        current_secondary_ += current_derivative_secondary_ * last_time_step;
        
        const double vd_primary = voltage_difference(node_primary_in_, node_primary_out_);
        const double vd_secondary = voltage_difference(node_secondary_in_, node_secondary_out_);
        
        const double effective_voltage_primary = vd_primary - current_primary_ * resistance_primary_;
        const double effective_voltage_secondary = vd_secondary - current_secondary_ * resistance_secondary_;
        
        //const double induced_voltage_primary = inductance_mutual_ * current_derivative_secondary_;
        //const double induced_voltage_secondary = inductance_mutual_ * current_derivative_primary_;
        //current_derivative_primary_ = (effective_voltage_primary - induced_voltage_primary) / inductance_primary_;
        //current_derivative_secondary_ = (effective_voltage_secondary - induced_voltage_secondary) / inductance_secondary_;
        
        current_derivative_primary_   = constant_primary_primary_   * effective_voltage_primary + constant_primary_secondary_   * effective_voltage_secondary;
        current_derivative_secondary_ = constant_secondary_primary_ * effective_voltage_primary + constant_secondary_secondary_ * effective_voltage_secondary;
        
        //std::cout << "prim:" << current_derivative_primary_ << "\n";
        //std::cout << "sec:" << current_derivative_secondary_ << "\n";
        
        node_primary_in_->current_out(current_primary_);
        node_primary_out_->current_in(current_primary_);
        node_secondary_in_->current_out(current_secondary_);
        node_secondary_out_->current_in(current_secondary_);
    }
    
private:
    // sim var
    double current_primary_;
    double current_secondary_;
    double current_derivative_primary_;
    double current_derivative_secondary_;
    
    // sim constants
    double constant_primary_primary_;
    double constant_primary_secondary_;
    double constant_secondary_primary_;
    double constant_secondary_secondary_;
    
    // params
    double inductance_primary_;
    double inductance_secondary_;
    double resistance_primary_;
    double resistance_secondary_;
    
    node::node_t * node_primary_in_;
    node::node_t * node_primary_out_;
    node::node_t * node_secondary_in_;
    node::node_t * node_secondary_out_;
};

autoreg::reg<transformer_t> transformer_reg(database::components(), "transformer");
