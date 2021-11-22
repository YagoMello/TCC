#ifndef CLI_HPP
#define CLI_HPP

#include <string>
#include <sstream>
#include <map>

#ifdef DEBUG
#include <fenv.h> // Enable traps (NaN, Overflow, etc.)
#endif

// INCLUDE ""
#include "database.hpp"

#include "../simulator/printable.hpp"
#include "../simulator/node.hpp"
#include "../simulator/component.hpp"
#include "../libs/shared_var.hpp"

class cli_t {
public:
    cli_t();
    ~cli_t();
    
    void run_cmd(std::string & input);
    void read_file(const std::string & file_name);
    void interpreter();
    
    struct logs_t {
        std::stringstream warnings;
        std::stringstream errors;
        std::stringstream all;
    };
    
private:
    using func               = void (cli_t:: *)(std::string &);
    using printable_map_type = std::map<std::string, printable::var_info_t>;
    using node_map_type      = std::map<std::string, std::shared_ptr<node::node_t>>;
    using component_map_type = std::map<std::string, std::shared_ptr<comp::component_t>>;
    
    struct objects_t {
        node_map_type      nodes;
        component_map_type components;
        printable_map_type printables;
        shared::list       shared_vars;
    } objects_;
    
    struct sim_param_t {
        double vdmax    = 50e-6;
        double tsmax    = 100e-9;
        double duration = 10e-3;
    } sim_params_;
    
    logs_t logs_;
    
    bool running_ = false;
    std::map<std::string, func> cmdtab_;
    
    // COMMANDS
    void end(std::string & input);
    void run(std::string & input);
    void component(std::string & input);
    void node(std::string & input);
    //void printer(std::string & input);
    
    //void print_mode(std::string & input);
    void print_var(std::string & input);
    void print(std::string & input);
    
    void param(std::string & input);
    void edit_component(std::string & input);
    void apply_gnd(const std::shared_ptr<node::node_t> & nod, std::string input);
    void apply_hide(const std::shared_ptr<node::node_t> & nod, std::string input);
    void edit_node(std::string & input);
    void edit(std::string & input);
    
    void show_models() const;
    void show_components() const;
    void show_nodes() const;
    void show_printables() const;
    void show_shared_variables() const;
    void show(std::string & input);
    
    void help_component(std::string & input) const;
    void help_node() const;
    void help_run() const;
    void help_end() const;
    void help_print(std::string & input) const;
    void help_edit(std::string & input) const;
    void help_show(std::string & input) const;
    void help(std::string & input);
    
    void read_file_cmd(std::string & input);
    
    void bind(std::string & input);
    
    void warn(const std::string & what);
    void error(const std::string & what);
    void log(const std::string & what);
    
    // auxilixary functions
    bool component_read_config_log(std::stringstream & config_log, const std::string & id, const std::string & model);
    std::string ptr_to_str(const void * ptr);
};

#endif // CLI_HPP
