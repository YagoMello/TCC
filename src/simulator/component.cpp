#include "component.hpp"

#include <limits>

#include "../libs/mfind.hpp"
#include "../simulator/node.hpp"

using namespace comp;

std::stringstream component_t::configure(node_map_type & /*nmap*/, printable_map_type & /*pmap*/, shared::list & /*smap*/, std::string /*args*/) {
    return std::stringstream();
}
void component_t::setup(sim::simulator_t * /*simulator*/, const double /*voltage_difference_max*/, const double /*time_step_max*/) { }
void component_t::setup_printer(const std::vector<node::node_t *> & /*node_vector*/, const std::vector<printable::var_info_t *> & /*info_vector*/) { }
void component_t::clear_variables() { }

void component_t::post_update(double /*time*/, double /*time_step*/) { }
[[nodiscard]] double component_t::sn_capacitance(const node::node_t * /*pos*/, const node::node_t * /*neg*/) const {
    return std::numeric_limits<double>::infinity();
}
[[nodiscard]] double component_t::sn_voltage(const node::node_t * /*pos*/, const node::node_t * /*neg*/) const {
    return 0;
}
[[nodiscard]] double component_t::sn_voltage_derivative(const node::node_t * /*pos*/, const node::node_t * /*neg*/) const {
    return 0;
}
[[nodiscard]] double component_t::sn_voltage_error(const node::node_t * pos, const node::node_t * neg) const {
    return sn_voltage(pos, neg) - voltage_difference(pos, neg);
}
void component_t::sn_update(node::node_t * /*pos*/, node::node_t * /*neg*/, double /*current*/, double /*time_step*/) { }
void component_t::printer(const double /*time*/, const double /*time_step*/, const uint64_t /*iteration*/) { }
bool component_t::keep_simulation_alive(const double /*time*/) {
    return false;
}
bool component_t::pause_simulation(const double /*time*/) {
    return false;
}
double component_t::time_step_max(const double /*time*/) const {
    return 0;
}
void component_t::simulation_complete(){ }

const char * component_t::help() const {
    return R"---(This component help string isn't defined.
To define one, override the "const char * help() const" method
inherited from comp::component_t.
)---";
}

[[nodiscard]] const std::string & component_t::label() const {
    return label_;
}
void component_t::label(const std::string & str){
    label_ = str;
}

[[nodiscard]] const std::string & component_t::model() const {
    return model_;
}
void component_t::model(const std::string & name){
    model_ = name;
}
