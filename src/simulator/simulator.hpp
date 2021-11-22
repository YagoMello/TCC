#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

// INCLUDE <>
#include <vector>

// INCLUDE ""
#include "objects.hpp"

namespace sim {

class simulator_t {
public:
    void add_node(node::node_t * node);
    void add_component(comp::component_t * component);
    void add_printable(printable::var_info_t * obj);
    
    void duration(const double time);
    [[nodiscard]] double duration() const;
    
    void nodes_set_voltage_difference_max_if_default();
    
    void time_step_max(const double value);
    void voltage_difference_max(const double value);
    
    double get_time_step(const double time_elapsed) const;
    void simulate();
    
private:
    bool is_alive(const double time) const;
    bool is_paused(const double time) const;
    void wait() const;
    void find_supernodes();
    
    std::vector<node::node_t *> node_vector_;
    std::vector<comp::component_t *> component_vector_;
    std::vector<printable::var_info_t *> printable_vector_;
    
    std::vector<sn::supernode_t *> supernode_vector_;
    
    double duration_;
    double time_step_max_;
    double voltage_difference_max_;
    
    bool in_simulation_ = false;
};

}

#endif // SIMULATOR_HPP
