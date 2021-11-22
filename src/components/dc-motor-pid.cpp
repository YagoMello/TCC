/*
#include "dc-motor.hpp"

class dc_motor_w_t : public comp::component_t {
public:
    configration_status_t configure(node_map_type & nmap, printable_map_type & pmap, shared::list & smap, std::string args) override {
        bool is_ok;
        is_ok  = utils::find_node(nmap, next(args), node_pos_);
        is_ok &= utils::find_node(nmap, next(args), node_neg_);
        if(!is_ok) return {is_ok, "Node not found"};
        
        w_tgt_  = next_as<double>(args, 314);
        kp_     = next_as<double>(args, 0);
        ki_     = next_as<double>(args, 0);
        period_ = 1 / next_as<double>(args, 10'000);
        r_on_   = next_as<double>(args, 0.2);
        
        is_ok  = utils::make_node(nmap, label() + ":motor+", node_motor_p_);
        is_ok &= utils::make_node(nmap, label() + ":motor-", node_motor_n_);
        if(!is_ok) return {is_ok, "Failed to create nodes"};
        
        //double k = -std::log(0.0001);
        //node_motor_p_->capacitance(period_ / (k * 2 * r_on_));
        //node_motor_n_->capacitance(period_ / (k * 2 * r_on_));
        
        node_motor_p_->capacitance(100e-9);
        node_motor_n_->capacitance(100e-9);
        
        motor.label(label() + ":" + "motor");
        return motor.configure(nmap, pmap, smap, node_motor_p_->label() + " " + node_motor_n_->label() + " " + args);
    }
    
    void setup(sim::simulator_t * simulator, const double voltage_difference_max, const double time_step_max) override {
        motor.setup(simulator, voltage_difference_max, time_step_max);
    }
    
    void clear_variables() override {
        status_ = false;
        next_toggle_ = period_;
        next_toggle_diff_ = 0;
        integral_ = 0;
        t_off_ = 0;
        last_error_ = 0;
        motor.clear_variables();
    }
    
    void update(double time, double last_time_step) override {
        if(time >= next_toggle_){
            if(status_ && t_off_ != 0){
                next_toggle_ = t_off_;
                status_ = false;
            }
            else{
                double error = w_tgt_ - motor.w();
                double control = kp_ * error + ki_ * integral_;
                double t_on;
                
                integral_ += period_ * (last_error_ + error) / 2;
                
                t_on = period_ * (0.5 + control);
                
                if(t_on < 0 * period_){
                    next_toggle_ += period_;
                    t_off_ = 0;
                    status_ = false;
                }
                else if(t_on > 1 * period_){
                    next_toggle_ += period_;
                    t_off_ = 0;
                    status_ = true;
                }
                else{
                    t_off_ = next_toggle_ + period_;
                    next_toggle_ += t_on;
                    status_ = true;
                }
                
                last_error_ = error;
            }
        }
        if(status_){
            double current_pos = voltage_difference(node_pos_, node_motor_p_) / r_on_;
            double current_neg = voltage_difference(node_motor_n_, node_neg_) / r_on_;
            
            node_pos_->current_out(current_pos);
            node_motor_p_->current_in(current_pos);
            
            node_motor_n_->current_out(current_neg);
            node_neg_->current_in(current_neg);
        }
        else{
            double current_pos = voltage_difference(node_pos_, node_motor_n_) / r_on_;
            double current_neg = voltage_difference(node_motor_p_, node_neg_) / r_on_;
            
            node_pos_->current_out(current_pos);
            node_motor_n_->current_in(current_pos);
            
            node_motor_p_->current_out(current_neg);
            node_neg_->current_in(current_neg);
        }
        
        next_toggle_diff_ = next_toggle_ - time;
        motor.update(time, last_time_step);
    }
    
    double time_step_max(const double) const override {
        return next_toggle_diff_;
    }
private:
    // sim var
    bool status_;
    double next_toggle_;
    double next_toggle_diff_;
    double integral_;
    double t_off_;
    double last_error_;
    
    // params
    double w_tgt_;
    double kp_;
    double ki_;
    double period_;
    double r_on_;
    
    node::node_t * node_pos_;
    node::node_t * node_neg_;
    
    node::node_t * node_motor_p_;
    node::node_t * node_motor_n_;
    
    dc_motor_t motor;
};

autoreg::reg<dc_motor_w_t> dc_motor_w_reg(database::components(), "dc-motor-pi");
*/
