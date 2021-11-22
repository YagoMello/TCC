#ifndef SUPERNODE_HPP
#define SUPERNODE_HPP

// INCLUDE <>
#include <string>
#include <vector>
#include <memory>
#include <eigen3/Eigen/Dense>

// INCLUDE ""
#include "objects.hpp"

namespace sn {

struct link_t {
    comp::component_t * comp;
    node::node_t * node_pos;
    node::node_t * node_neg;
    size_t index_comp;
    size_t index_pos;
    size_t index_neg;
};

class supernode_t;
std::shared_ptr<sn::supernode_t> make_supernode();

class supernode_t {
public:
    using matrix_t = Eigen::MatrixXd;
    using vector_t = Eigen::VectorXd;
    
    using lim = std::numeric_limits<double>;
    
    void init();
    void add(comp::component_t * comp, node::node_t * pos, node::node_t * neg);
    void update();
    void apply_iteration(const double time_step);
    void fix_divergence();
    void fix_divergence_if(const double error_max);
    void absorb(const supernode_t & supernode);
    
private:
    size_t get_node_index(node::node_t * node);
    size_t get_component_index(comp::component_t * comp, node::node_t * pos, node::node_t * neg);
    
    void calculate_basic_matrices();
    void calculate_repair_matrices();
    
    void update_current_dep_indep();
    bool is_error_greater_than(const double error_max);
    
    matrix_t coefficients_extended_;
    matrix_t source_geometry_;
    matrix_t source_impact_;
    matrix_t repair_matrix_;
    vector_t repair_values_;
    vector_t input_;
    vector_t output_;
    
    matrix_t coefficients_extended_inverse_;
    
    size_t component_index_ = 0;
    
    std::vector<node::node_t *> node_list_;
    
    size_t count_current_dependent_   = 0;
    size_t count_current_independent_ = 0;
    
    std::vector<sn::link_t> links_;
    
    friend std::shared_ptr<sn::supernode_t> make_supernode();
    std::weak_ptr<sn::supernode_t> instance_holder_;
    
#ifdef DEBUG
    size_t iterations_since_last_fix = 0;
#endif
};

}

#endif // SUPERNODE_HPP
