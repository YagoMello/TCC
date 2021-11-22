// INCLUDE <>
#include <cmath>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"


class inverter_180_t : public comp::component_t {
public:
    enum switch_mode_t {HI_Z, LOW, HIGH};
    
    // component XX powergrid A B C IN1 IN2 ammeter1 ammeter2 ammeter3 0.5
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &/*ls*/, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        // encontrar nos
        info << utils::find_node_s(nmap, next(args), node_phase1_, error);
        info << utils::find_node_s(nmap, next(args), node_phase2_, error);
        info << utils::find_node_s(nmap, next(args), node_phase3_, error);
        info << utils::find_node_s(nmap, next(args), node_current_in_, error);
        info << utils::find_node_s(nmap, next(args), node_current_out_, error);
        if(error) return info;
        
        // ler variavel
        resistance_on_ = next_as<double>(args, 0.05);
        output_freq_   = next_as<double>(args, 60);
        
        // adicionar variavel que da pra printar
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        // limpa as variaveis no comeco da simulacao
        current_ = 0;
        mode_phase1_ = HI_Z;
        mode_phase2_ = HI_Z;
        mode_phase3_ = HI_Z;
    }
    
    void update(double time, double /*last_time_step*/) override {
        
        // SO MEXER NESSA REGIAO
        
        // so fazer as conta
        // e definir 
        // mode_phase1_
        // mode_phase2_ 
        // mode_phase3_
        // que sao as chaves
        
        // pra ler a tensao de um no, so usar node->voltage() pra referencia ao terra
        // ou voltage_difference(no, no_referencia)
        
        // pra usar current_phaseX_, so tratar ele como double
        
        // time contem tempo em segundos na simulacao
        
        // last time step tem o tempo do ultimo passo, util pra fazer integral
        
        exemplo_inversor_ruim_180(time);
        
        // ATE AQUI
        
        // joga elas pra simulacao
        find_current(node_current_in_, node_current_out_, node_phase1_, mode_phase1_);
        find_current(node_current_in_, node_current_out_, node_phase2_, mode_phase2_);
        find_current(node_current_in_, node_current_out_, node_phase3_, mode_phase3_);
    }
    
    void find_current(node::node_t * input_pos, node::node_t * input_neg, node::node_t * output, switch_mode_t mode) const {
        double current_high = 0;
        double current_low = 0;
        double voltage_high = voltage_difference(input_pos, output);
        double voltage_low = voltage_difference(output, input_neg);
        
        switch(mode){
            case HI_Z:
                break;
            case LOW:
                current_low += voltage_low / resistance_on_;
                break;
            case HIGH:
                current_high += voltage_high / resistance_on_;
                break;
        }
        if(voltage_high < 0){
            current_high += voltage_high / resistance_on_;
        }
        if(voltage_low < 0){
            current_low += voltage_low / resistance_on_;
        }
        
        output->current_in(current_high);
        output->current_out(current_low);
        input_pos->current_out(current_high);
        input_neg->current_in(current_low);
    }
    
    // exemplo inversor 180 porco
    constexpr void comuta(switch_mode_t & switch_mode, const double offset, const double time){
        const double pos = std::fmod(output_freq_*time, 1);
        if(offset < 0.5){
            if(pos >= offset && pos < offset + 0.5){
                switch_mode = HIGH;
            }
            else{
                switch_mode = LOW;
            }
        }
        else{
            if(pos >= offset || pos < offset - 0.5){
                switch_mode = HIGH;
            }
            else{
                switch_mode = LOW;
            }
        }
    }
    
    void exemplo_inversor_ruim_180(const double time){
        comuta(mode_phase1_, 0, time);
        comuta(mode_phase2_, 1/3.0, time);
        comuta(mode_phase3_, 2/3.0, time);
    }
    
private:
    // sim var
    double current_ = 0;
    switch_mode_t mode_phase1_ = HI_Z;
    switch_mode_t mode_phase2_ = HI_Z;
    switch_mode_t mode_phase3_ = HI_Z;
    
    // params
    double resistance_on_;
    double output_freq_;
    
    // nodes
    node::node_t * node_phase1_;
    node::node_t * node_phase2_;
    node::node_t * node_phase3_;
    node::node_t * node_current_in_;
    node::node_t * node_current_out_;
};

class inverter_pwm_t : public comp::component_t {
public:
    enum switch_mode_t {HI_Z, LOW, HIGH};
    
    // component XX powergrid A B C IN1 IN2 ammeter1 ammeter2 ammeter3 r_on output_freq switching_freq kp ki
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &ls, std::string args) override {
        std::stringstream info;
        bool error = false;
        
        // encontrar nos
        info << utils::find_node_s(nmap, next(args), node_phase1_, error);
        info << utils::find_node_s(nmap, next(args), node_phase2_, error);
        info << utils::find_node_s(nmap, next(args), node_phase3_, error);
        info << utils::find_node_s(nmap, next(args), node_current_in_, error);
        info << utils::find_node_s(nmap, next(args), node_current_out_, error);
        if(error) return info;
        
        // configurar variavel compartilhada
        current_phase1_.create(ls, next(args));
        current_phase2_.create(ls, next(args));
        current_phase3_.create(ls, next(args));
        
        // ler variavel
        resistance_on_  = next_as<double>(args, 0.05);
        output_freq_    = next_as<double>(args,   60);
        switching_freq_ = next_as<double>(args, 1200);
        control_cst_p_  = next_as<double>(args,   20);
        control_cst_i_  = next_as<double>(args,  200);
        
        // adicionar variavel que da pra printar
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*time_step_max*/, const double /*voltage_difference_max*/) override {
        // inicializacao
        current_phase1_.init();
        current_phase2_.init();
        current_phase3_.init();
    }
    
    void clear_variables() override {
        // limpa as variaveis no comeco da simulacao
        current_ = 0;
        mode_phase1_ = HI_Z;
        mode_phase2_ = HI_Z;
        mode_phase3_ = HI_Z;
    }
    
    void update(double time, double last_time_step) override {
        exemplo_inversor_pi(time, last_time_step);
        
        find_current(node_current_in_, node_current_out_, node_phase1_, mode_phase1_);
        find_current(node_current_in_, node_current_out_, node_phase2_, mode_phase2_);
        find_current(node_current_in_, node_current_out_, node_phase3_, mode_phase3_);
    }
    
    void find_current(node::node_t * input_pos, node::node_t * input_neg, node::node_t * output, switch_mode_t mode) const {
        double current_high = 0;
        double current_low = 0;
        double voltage_high = voltage_difference(input_pos, output);
        double voltage_low = voltage_difference(output, input_neg);
        
        switch(mode){
            case HI_Z:
                break;
            case LOW:
                current_low += voltage_low / resistance_on_;
                break;
            case HIGH:
                current_high += voltage_high / resistance_on_;
                break;
        }
        if(voltage_high < 0){
            current_high += voltage_high / resistance_on_;
        }
        if(voltage_low < 0){
            current_low += voltage_low / resistance_on_;
        }
        
        output->current_in(current_high);
        output->current_out(current_low);
        input_pos->current_out(current_high);
        input_neg->current_in(current_low);
    }
    
    // exemplo controle PI
    
    static constexpr double sawtooth_value(double min, double max, double frequency, double time){
        return (max - min)*frequency*std::fmod(time, 1/frequency) + min;
    }
    
    struct current_info_t {
        double target = 0;
        double error = 0;
        double error_acc = 0;
        double control_val = 0;
    };
    
    static constexpr double target_current = 2;
    current_info_t info_phase1_{.target = target_current};
    current_info_t info_phase2_{.target = target_current};
    current_info_t info_phase3_{.target = target_current};
    
    constexpr void update_current_info(const double time, const double last_time_step, const double offset, const double current, current_info_t & info, const double freq){
        const double angle = 2 * std::numbers::pi * (freq * fmod(time, 1/freq) + offset);
        
        info.error_acc += info.error * last_time_step;
        info.error = info.target * std::cos(angle) - current;
        info.control_val = control_cst_p_ * info.error + control_cst_i_ * info.error_acc;
    }
    
    void exemplo_inversor_pi(const double time, const double last_time_step){
        update_current_info(time, last_time_step, 0.0/3, current_phase1_, info_phase1_, output_freq_);
        update_current_info(time, last_time_step, 1.0/3, current_phase2_, info_phase2_, output_freq_);
        update_current_info(time, last_time_step, 2.0/3, current_phase3_, info_phase3_, output_freq_);
        
        const double ref = sawtooth_value(-target_current, target_current, switching_freq_, time);
        mode_phase1_ = (info_phase1_.control_val < ref) ? HIGH : LOW;
        mode_phase2_ = (info_phase2_.control_val < ref) ? HIGH : LOW;
        mode_phase3_ = (info_phase3_.control_val < ref) ? HIGH : LOW;
    }
    
private:
    // sim var
    double current_ = 0;
    switch_mode_t mode_phase1_ = HI_Z;
    switch_mode_t mode_phase2_ = HI_Z;
    switch_mode_t mode_phase3_ = HI_Z;
    
    // shared var
    shared::var<double> current_phase1_;
    shared::var<double> current_phase2_;
    shared::var<double> current_phase3_;
    
    // params
    double resistance_on_;
    double output_freq_;
    double switching_freq_;
    double control_cst_p_;
    double control_cst_i_;
    
    // nodes
    node::node_t * node_phase1_;
    node::node_t * node_phase2_;
    node::node_t * node_phase3_;
    node::node_t * node_current_in_;
    node::node_t * node_current_out_;
};

autoreg::reg<inverter_180_t> inverter_180_reg(database::components(), "inverter::180");
autoreg::reg<inverter_pwm_t> inverter_pwm_reg(database::components(), "inverter::pwm");
