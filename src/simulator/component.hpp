#ifndef COMPONENT_HPP
#define COMPONENT_HPP

// INCLUDE <>
#include <cstdint>
#include <string>
#include <string_view>
#include <sstream>
#include <cmath>
#include <memory>
#include <vector>

// INCLUDE ""
#include "objects.hpp"
#include "../libs/autoreg.hpp"
#include "../simulator/printable.hpp"
#include "../libs/shared_var.hpp"

namespace comp {

class component_t {
public:
    using node_map_type = std::map<std::string, std::shared_ptr<node::node_t>>;
    using printable_map_type = std::map<std::string, printable::var_info_t>;
    
    component_t() = default;
    virtual ~component_t() = default;
    virtual std::stringstream configure(node_map_type & nmap, printable_map_type & pmap, shared::list & smap, std::string args);
    
    virtual void setup(sim::simulator_t * simulator, const double voltage_difference_max, const double time_step_max);
    virtual void setup_printer(const std::vector<node::node_t *> & node_vector, const std::vector<printable::var_info_t *> & info_vector);
    virtual void clear_variables();
    
    virtual void update(double time, double last_time_step) = 0;
    virtual void post_update(double time, double time_step);
    [[nodiscard]] virtual double sn_capacitance(const node::node_t * pos, const node::node_t * neg) const;
    [[nodiscard]] virtual double sn_voltage(const node::node_t * pos, const node::node_t * neg) const;
    [[nodiscard]] virtual double sn_voltage_derivative(const node::node_t * pos, const node::node_t * neg) const;
    [[nodiscard]] virtual double sn_voltage_error(const node::node_t * pos, const node::node_t * neg) const;
    virtual void sn_update(node::node_t * pos, node::node_t * neg, double current, double time_step);
    virtual void printer(const double time, const double time_step, const uint64_t iteration);
    virtual bool keep_simulation_alive(const double time);
    virtual bool pause_simulation(const double time);
    
    virtual double time_step_max(const double time) const;
    
    virtual void simulation_complete();
    
    virtual const char * help() const;
    
    [[nodiscard]] const std::string & label() const;
    
    void label(const std::string & str);
    static inline double limit_current_difference(double di, double dt, double didt_max) {
        return std::copysign(1/(1/std::abs(di) + 1/(dt * didt_max)), di);
    }
    [[nodiscard]] const std::string & model() const;
    void model(const std::string & name);
private:
    std::string label_ = "undef.";
    std::string model_ = "undef.";
    
    component_t(const component_t &) = delete; // forbid copy construction
    component_t & operator = (const component_t &) = delete;
};

}

#endif // COMPONENT_HPP
