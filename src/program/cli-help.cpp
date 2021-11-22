#include "cli.hpp"

#include <memory>

#include "../libs/next.hpp"
#include "../libs/autoreg.hpp"
#include "../simulator/component.hpp"
#include "../program/help-strings.hpp"

void cli_t::help(std::string & input) {
    const std::string subject = next(input);
    
    if(subject.size() == 0){
        std::cout << help::program << std::endl;
    }
    else if(subject == "component"){
        // Help with a specific component or the component concept
        help_component(input);
    }
    else if(subject == "node"){
        std::cout << help::node << std::endl;
    }
    else if(subject == "run"){
        std::cout << help::run << std::endl;
    }
    else if(subject == "end"){
        std::cout << help::end << std::endl;
    }
    else if(subject == "print"){
        std::cout << help::print << std::endl;
    }
    else if(subject == "edit"){
        std::cout << help::edit << std::endl;
    }
    else if(subject == "show"){
        std::cout << help::show << std::endl;
    }
    else if(subject == "bind"){
        std::cout << help::bind << std::endl;
    }
    else if(subject == "param"){
        std::cout << help::param << std::endl;
    }
    else if(subject == "read"){
        std::cout << help::read << std::endl;
    }
    else{
        std::cout << "The subject \"" << subject << "\" does not exist!\n";
    }
}

void cli_t::help_component(std::string & input) const {
    if(input.size() == 0){
        std::cout << help::component << std::endl;
    }
    else{
        const std::string model = next(input);
        std::unique_ptr<comp::component_t> component(autoreg::safe_build(database::components(), model));
        if(component != nullptr){
            std::cout << component->help() << std::endl;
        }
        else{
            std::cout << "Component model \"" << model << "\" not found!\n"; 
        }
    }
}
