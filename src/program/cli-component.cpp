#include "cli.hpp"

#include <regex>
#include <ctime>
#include <iomanip>

#include "../libs/next.hpp"

bool cli_t::component_read_config_log(std::stringstream & config_log, const std::string & id, const std::string & model){
    bool is_ok = true;
    
    std::string line_raw;
    std::regex rx_warning(R"---(\[(?:WARN|WARNING)\]\s*)---");
    std::regex rx_error(R"---(\[(?:ERR|ERROR)\]\s*)---");
    std::regex rx_log(R"---(\[(?:LOG|INFO)\]\s*)---");
    std::regex rx_id(R"---(\$ID)---");
    std::regex rx_model(R"---(\$MODEL)---");
    
    while(std::getline(config_log, line_raw)){
        std::string line_pretty;
        line_pretty = std::regex_replace(line_raw   , rx_warning, "");
        line_pretty = std::regex_replace(line_pretty, rx_error, "");
        line_pretty = std::regex_replace(line_pretty, rx_log, "");
        line_pretty = std::regex_replace(line_pretty, rx_id, id);
        line_pretty = std::regex_replace(line_pretty, rx_model, model);
        line_pretty = "<" + model + "> " + id + ": " + line_pretty;
        
        if(std::regex_search(line_raw, rx_error)){
            is_ok = false;
            error(line_pretty);
        }
        else if(std::regex_search(line_raw, rx_warning)){
            warn(line_pretty);
        }
        else if(std::regex_search(line_raw, rx_log)){
            log(line_pretty);
        }
    }
    
    return is_ok;
}

void cli_t::component(std::string & input){
    std::string id         = next(input);
    std::string model_name = next(input);
    
    log("<component> searching for component with id \"" + id + "\"");
    if(objects_.components.contains(id) == false){
        
        log("<component> creating component \"" + id + "\" of type \"" + model_name + "\"");
        std::shared_ptr<comp::component_t> new_component(autoreg::safe_build<comp::component_t>(database::components(), model_name));
        
        if(new_component != nullptr){
            log("<component> created component at " + ptr_to_str(new_component.get()));
            
            log("<component> setting model name and id");
            new_component->model(model_name);
            new_component->label(id);
            
            log("<component> backing-up object data");
            objects_t objects_backup = objects_;
            
            log("<component> configuring new component");
            std::stringstream config_log = new_component->configure(objects_.nodes, objects_.printables, objects_.shared_vars, input);
            bool config_ok = component_read_config_log(config_log, id, model_name);
            
            if(config_ok){
                log("<component> adding component to \"objects_.components\"");
                objects_.components[id] = new_component;
                log("<component> component added to \"objects_.components\" node " + ptr_to_str(&(objects_.components[id])));
            }
            else{
                error("<component> failed to configure component");
                objects_ = objects_backup;
                log("<component> restored previous object data");
            }
        }
        else{
            error("<component> component model \"" + model_name + "\" not found");
        }
    }
    else{
        error("<component> a component with id \"" + id + "\" already exists at " + ptr_to_str(&(objects_.components[id])));
    }
}
