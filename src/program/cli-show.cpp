#include "cli.hpp"

#include "../libs/next.hpp"

void cli_t::show_models() const {
    std::cout << "Component types:\n";
    for(const auto & iter : database::components()){
        std::cout << "- " << iter.first << "\n";
    }
}

void cli_t::show_components() const {
    std::cout << "Components [model]:\n";
    for(const auto & iter : objects_.components){
        std::cout << "- " << iter.first << " [" << iter.second->model() << "]\n";
    }
}

void cli_t::show_nodes() const {
    std::cout << "Nodes [capacitance] #flags:\n";
    for(const auto & iter : objects_.nodes){
        std::cout << "- " << iter.first;
        
        if(iter.second->is_redirecting_to_supernode()){
            std::cout << std::scientific << " [" << iter.second->internal_capacitance() << "] #virtual";
        }
        else {
            std::cout << std::scientific << " [" << iter.second->internal_capacitance() << "] #real";
        }
        
        if(iter.second->is_ground()){
            std::cout << " #gnd";
        }
        
        std::cout << "\n";
    }
}

void cli_t::show_printables() const {
    std::cout << "Printables [unit][status]:\n";
    for(const auto & iter : objects_.printables){
        std::cout << "- " << iter.first << " [" << iter.second.unit << "]";
        if(iter.second.is_active){
            std::cout << "[active]";
        }
        else{
            std::cout << "[inactive]";
        }
        std::cout << "\n";
    }
}

void cli_t::show_shared_variables() const {
    std::cout << "Variables [type]:\n";
    for(const auto & iter : objects_.shared_vars){
        std::cout << "- " << iter.first << " [" << iter.second.type_name << "]\n";
    }
}

void cli_t::show(std::string & input) {
    std::string what = next(input);
    while(what != ""){
        if     (what == "models")     show_models();
        else if(what == "components") show_components();
        else if(what == "nodes")      show_nodes();
        else if(what == "printables") show_printables();
        else if(what == "shared-vars")  show_shared_variables();
        else std::cout << "Category \"" << what << "\" doesn't exist!\n";
        what = next(input);
    }
}
