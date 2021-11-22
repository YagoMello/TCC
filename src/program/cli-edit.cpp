#include "cli.hpp"

#include "../libs/next.hpp"

void cli_t::edit(std::string & input) {
    std::string cmd = next(input);
    if     (cmd == "component") edit_component(input);
    else if(cmd == "node"     ) edit_node(input);
    else error("<edit> type \"" + cmd + "\" doesn't exist");
}

void cli_t::edit_component(std::string & input){
    std::string id = next(input);
    if(objects_.components.contains(id)){
        log("<edit_component> found component \"" + id + "\" in \"objects_.components\" node " + ptr_to_str(&(objects_.components[id])));
        log("<edit_component> re-configuring \"" + id + "\" (" + ptr_to_str(objects_.components[id].get()) + ")");
        objects_.components[id]->configure(objects_.nodes, objects_.printables, objects_.shared_vars, input);
    }
    else error("<edit_component> component \"" + id + "\"not found in \"objects_.components\"");
}

void cli_t::apply_gnd(const std::shared_ptr<node::node_t> & node, std::string input){
    bool value;
    if(apply_bool(input, value, true)){
        log(std::string("<apply_gnd> setting \"is_ground\" to \"") + (value ? "true" : "false") + "\"");
        node->is_ground(value);
    }
    else{
        error("<apply_gnd> bad argument. \"gnd\" expects \"true\" or \"false\"");
    }
}

void cli_t::apply_hide(const std::shared_ptr<node::node_t> & node, std::string input){
    bool value;
    if(apply_bool(input, value, true)){
        log(std::string("<apply_hide> setting \"is_hidden\" to \"") + (value ? "true" : "false") + "\"");
        node->is_hidden(value);
    }
    else{
        error("<apply_hide> bad argument. \"hide\" expects \"true\" or \"false\"");
    }
}

void cli_t::edit_node(std::string & input){
    std::string id = next(input);
    std::string op = next(input);
    if(objects_.nodes.contains(id)) {
        log("<edit_node> found node \"" + id + "\" in objects_.nodes at " + ptr_to_str(&(objects_.nodes[id])) + " pointing to " + ptr_to_str(objects_.nodes[id].get()));
        if     (op == "gnd")         apply_gnd(objects_.nodes[id], input);
        else if(op == "capacitance") objects_.nodes[id]->capacitance(next_as<double>(input));
        else if(op == "hide")        apply_hide(objects_.nodes[id], input);
        else error("<edit_node> operation \"" + op + "\" not found");
    }
    else{
        error("<edit_node> node \"" + id + "\" not found in \"objects_.nodes\"");
    }
}

