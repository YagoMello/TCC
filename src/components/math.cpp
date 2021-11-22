// INCLUDE <>
#include <cstdint>
#include <cmath>
#include <numbers>

// INCLUDE ""
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../simulator/simulator.hpp"
#include "../libs/next.hpp"
#include "../libs/statespace.hpp"
#include "../libs/autoreg.hpp"
#include "../program/database.hpp"
#include "../program/utils.hpp"

namespace default_math {


static constexpr const char * help_string_n = 
R"---(math::op_t1 V1.0 2021-09-18

|------------|--------|---------|------|
|  argument  |  type  | default | unit |
|============|========|=========|======|
| node out   | string |         |      |
| node in[1] | string |         |      |
| node in[2] | string |         |      |
| ...        | string |         |      |
| node in[N] | string |         |      |
|------------|--------|---------|------|

=====[ printables ]=====
i: current [A]
  The current exiting through the output pin.

=====[ CLI arguments ]=====
node out:
  The node representing the math result.
  out = operation(in[1], in[2], ... in[N])

node in[k]:
  The input node number k.
  The impedance is infinite.
)---";


static constexpr const char * help_string_ab = 
R"---(math::op_t2 V1.0 2021-09-18

|----------|--------|---------|------|
| argument |  type  | default | unit |
|==========|========|=========|======|
| node out | string |         |      |
| node A   | string |         |      |
| node B   | string |         |      |
|----------|--------|---------|------|

=====[ printables ]=====
i: current [A]
  The current exiting through the output pin.

=====[ CLI arguments ]=====
node out:
  The node representing the math result.
  out = A operation B

node A:
  The input node for the left hand side value.
  The impedance is infinite.

node B:
  The input node for the right hand side value.
  The impedance is infinite.
)---";

static constexpr const char * help_string_a = 
R"---(math::op_t3 V1.0 2021-09-18

|----------|--------|---------|------|
| argument |  type  | default | unit |
|==========|========|=========|======|
| node out | string |         |      |
| node in  | string |         |      |
|----------|--------|---------|------|

=====[ printables ]=====
i: current [A]
  The current exiting through the output pin.

=====[ CLI arguments ]=====
node out:
  The pin representing the math result.
  out = operation(in)

node in:
  The input node.
  The impedance is infinite.
)---";

static constexpr const char * help_string_logn = 
R"---(math::logn V1.0 2021-09-18

|------------|--------|---------|------|
|  argument  |  type  | default | unit |
|============|========|=========|======|
| node out   | string |         |      |
| node anti  | string |         |      |
| node base  | string |         |      |
|------------|--------|---------|------|

=====[ printables ]=====
i: current [A]
  The current exiting through the output pin.

=====[ CLI arguments ]=====
node out:
  The node representing the math result.
  out = log(antilog) / log(base)

node anti:
  The input node for the anti-logarithm (aka. number).
  The impedance is infinite.

node base:
  The input node for the base.
  The impedance is infinite.
)---";

static constexpr const char * help_string_nroot = 
R"---(math::nroot V1.0 2021-09-18

|----------|--------|---------|------|
| argument |  type  | default | unit |
|==========|========|=========|======|
| node out | string |         |      |
| node rad | string |         |      |
| node deg | string |         |      |
|----------|--------|---------|------|

=====[ printables ]=====
i: current [A]
  The current exiting through the output pin.

=====[ CLI arguments ]=====
node out:
  The node representing the math result.
  out = rad^(1/deg)

node rad:
  The input node for the radicand.
  The impedance is infinite.

node deg:
  The input node for the degree.
  The impedance is infinite.
)---";

class sum_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        node_vector_in_.clear();
        
        std::string node_name;
        node::node_t * node_ptr;
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        if(error) return info;
        
        node_name = next(args);
        while(node_name.empty() == false){
            info << utils::find_node_s(nmap, node_name, node_ptr, error);
            if(error) return info;
            
            if(node_ptr != nullptr){
                node_vector_in_.push_back(node_ptr);
            }
            node_name = next(args);
        }
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        voltage_out_ = 0;
        for(const node::node_t * elem : node_vector_in_){
            voltage_out_ += elem->voltage();
        }
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_n;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    std::vector<node::node_t *> node_vector_in_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class sub_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_p_, error);
        info << utils::find_node_s(nmap, next(args), node_in_n_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        voltage_out_ = voltage_difference(node_in_p_, node_in_n_);
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_ab;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_p_;
    node::node_t * node_in_n_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class mul_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        node_vector_in_.clear();
        
        std::string node_name;
        node::node_t * node_ptr;
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        if(error) return info;
        
        node_name = next(args);
        while(node_name.empty() == false){
            info << utils::find_node_s(nmap, node_name, node_ptr, error);
            if(error) return info;
            
            if(node_ptr != nullptr){
                node_vector_in_.push_back(node_ptr);
            }
            node_name = next(args);
        }
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);
        
        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        voltage_out_ = 1;
        for(const node::node_t * elem : node_vector_in_){
            voltage_out_ *= elem->voltage();
        }
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_n;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    std::vector<node::node_t *> node_vector_in_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class div_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_num_, error);
        info << utils::find_node_s(nmap, next(args), node_in_den_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        voltage_out_ = node_in_num_->voltage() / node_in_den_->voltage();
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_ab;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_num_;
    node::node_t * node_in_den_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class pow_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_base_, error);
        info << utils::find_node_s(nmap, next(args), node_in_power_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        using std::pow;
        voltage_out_ = pow(node_in_base_->voltage(), node_in_power_->voltage());
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_ab;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_base_;
    node::node_t * node_in_power_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class exp_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_power_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        using std::exp;
        voltage_out_ = exp(node_in_power_->voltage());
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_a;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_power_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class logn_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_anti_, error);
        info << utils::find_node_s(nmap, next(args), node_in_base_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        using std::log;
        voltage_out_ = log(node_in_anti_->voltage()) / log(node_in_base_->voltage());
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_logn;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_anti_;
    node::node_t * node_in_base_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class log10_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_anti_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        using std::log10;
        voltage_out_ = log10(node_in_anti_->voltage());
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_ab;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_anti_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class ln_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_anti_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        using std::log;
        voltage_out_ = log(node_in_anti_->voltage());
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_ab;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_anti_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class nroot_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_radicand_, error);
        info << utils::find_node_s(nmap, next(args), node_in_degree_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        using std::pow;
        voltage_out_ = pow(node_in_radicand_->voltage(), 1/node_in_degree_->voltage());
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_nroot;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_radicand_;
    node::node_t * node_in_degree_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

class sqrt_t : public comp::component_t {
public:
    std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list &, std::string args) override {
        std::stringstream info;
        bool error;
        
        info << utils::make_node_s(nmap, this->label(), node_ref_, error);
        if(error) return info;
        
        node_ref_->is_ground(true);
        node_ref_->is_hidden(true);
        
        info << utils::find_node_s(nmap, next(args), node_out_, error);
        info << utils::find_node_s(nmap, next(args), node_in_radicand_, error);
        if(error) return info;
        
        info << utils::add_printable_s(pmap, label() + ":i", "A", current_);

        return info;
    }
    
    void clear_variables() override {
        voltage_out_ = 0;
        current_     = 0;
    }
    
    void setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) override {
        bind(node_out_, node_ref_, this);
    }
    
    void update(double /*time*/, double /*last_time_step*/) override {
        using std::sqrt;
        voltage_out_ = sqrt(node_in_radicand_->voltage());
    }
    
    double sn_voltage(const node::node_t * pos, const node::node_t * neg) const override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            return voltage_out_;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            return -voltage_out_;
        }
        else{
            throw;
        }
    }
    
    void sn_update(node::node_t * pos, node::node_t * neg, double current, double) override {
        if(
            pos == node_out_ && 
            neg == node_ref_
        ){
            current_ = current;
        }
        else if(
            pos == node_ref_ &&
            neg == node_out_
        ){
            current_ = -current;
        }
        else{
            throw;
        }
    }
    
    const char * help() const override {
        return help_string_a;
    }
    
private:
    // sim var
    double current_;
    double voltage_out_;
    
    // params
    
    // nodes
    node::node_t * node_in_radicand_;
    node::node_t * node_out_;
    node::node_t * node_ref_;
};

autoreg::reg<default_math::sum_t>     sum_reg(database::components(), "math::sum");
autoreg::reg<default_math::sub_t>     sub_reg(database::components(), "math::sub");
autoreg::reg<default_math::mul_t>     mul_reg(database::components(), "math::mul");
autoreg::reg<default_math::div_t>     div_reg(database::components(), "math::div");
autoreg::reg<default_math::pow_t>     pow_reg(database::components(), "math::pow");
autoreg::reg<default_math::exp_t>     exp_reg(database::components(), "math::exp");
autoreg::reg<default_math::logn_t>   logn_reg(database::components(), "math::logn");
autoreg::reg<default_math::log10_t> log10_reg(database::components(), "math::log10");
autoreg::reg<default_math::ln_t>       ln_reg(database::components(), "math::ln");
autoreg::reg<default_math::nroot_t> nroot_reg(database::components(), "math::nroot");
autoreg::reg<default_math::sqrt_t>   sqrt_reg(database::components(), "math::sqrt");

}
