#include "cli.hpp"

#include "../libs/next.hpp"

void cli_t::node(std::string & input){
    std::string id = next(input);
    double capacitance_value = next_as<double>(input);
    
    log("<node> searching for node with id \"" + id + "\"");
    if(capacitance_value <= 0){
        error("<node> \"node " + id + "\" expects capacitance greater than 0");
    }
    else if(!objects_.nodes.contains(id)){
        log("<node> creating node \"" + id + "\"");
        objects_.nodes[id] = std::make_shared<node::node_t>();
        log("<node> created node at " + ptr_to_str(objects_.nodes[id].get()));
        log("<node> added \"" + id + "\" to objects_.nodes node at " + ptr_to_str(&(objects_.nodes[id])));
        log("<node> setting id and capacitance");
        objects_.nodes[id]->label(id);
        objects_.nodes[id]->capacitance(capacitance_value);
    }
    else{
        error("<node> node \"" + id + "\" already exists at " + ptr_to_str(&(objects_.nodes[id])));
    }
}
