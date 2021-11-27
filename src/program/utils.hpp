#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <map>

#include "../simulator/node.hpp"

namespace utils {

inline bool find_node(std::map<std::string, std::shared_ptr<node::node_t>> & map, const std::string & key, node::node_t * & node){
    auto iterator = map.find(key);
    bool found = (iterator != map.end());
    if(found) node = iterator->second.get();
    return found;
}

inline std::string find_node_s(std::map<std::string, std::shared_ptr<node::node_t>> & map, const std::string & key, node::node_t * & node){
    std::stringstream info;
    info << "[INFO] <find_node_s> searching for node \"" << key << "\"\n";
    
    auto iterator = map.find(key);
    const bool found = (iterator != map.end());
    if(found){
        node = iterator->second.get();
        info << "[INFO] <find_node_s> found node \"" << key << "\" at " << node << "\n";
    }
    else{
        info << "[ERROR] <find_node_s> node \"" << key << "\" not found\n";
    }
    
    return info.str();
}

inline std::string find_node_s(
    std::map<std::string, std::shared_ptr<node::node_t>> & map,
    const std::string & key,
    node::node_t * & node,
    bool & flag_error,
    bool accept_null = false
){
    std::stringstream info;
    info << "[INFO] <find_node_s> searching for node \"" << key << "\"\n";
    
    auto iterator = map.find(key);
    const bool found = (iterator != map.end());
    if(accept_null && (key == "null" || key == "_" || key == "")){
        node = nullptr;
        info << "[INFO] <find_node_s> null node \"" << key << "\"\n";
    }
    else if(found){
        node = iterator->second.get();
        info << "[INFO] <find_node_s> found node \"" << key << "\" at " << node << "\n";
    }
    else{
        flag_error = true;
        info << "[ERROR] <find_node_s> node \"" << key << "\" not found\n";
    }
    
    return info.str();
}

inline bool add_node(std::map<std::string, std::shared_ptr<node::node_t>> & map, const std::string & key, node::node_t * & node_ptr){
    bool inserted;
    std::shared_ptr<node::node_t> node(node_ptr);
    node->label(key);
    
    if(map.contains(key) == false){
        map[key] = node;
        inserted = true;
    }
    else{
        inserted = false;
    }
    
    return inserted;
}

inline bool add_shared_node(std::map<std::string, std::shared_ptr<node::node_t>> & map, const std::string & key, std::shared_ptr<node::node_t> & node){
    bool inserted;
    node->label(key);
    
    if(map.contains(key) == false){
        map[key] = node;
        inserted = true;
    }
    else{
        inserted = false;
    }
    
    return inserted;
}

inline bool make_node(std::map<std::string, std::shared_ptr<node::node_t>> & map, const std::string & key, node::node_t * & node_ptr){
    bool inserted;
    std::shared_ptr<node::node_t> node = std::make_shared<node::node_t>();
    node->label(key);
    
    if(map.contains(key) == false){
        map[key] = node;
        node_ptr = node.get();
        inserted = true;
    }
    else{
        node_ptr = nullptr;
        inserted = false;
    }
    
    return inserted;
}

inline std::string make_node_s(std::map<std::string, std::shared_ptr<node::node_t>> & nmap, const std::string & key, node::node_t * & node_ptr){
    std::stringstream info;
    
    info << "[INFO] <make_node_s> searching for node \"" << key << "\"\n";
    const bool not_found = (nmap.contains(key) == false);
    
    if(not_found){
        info << "[INFO] <make_node_s> node not found\n";
        std::shared_ptr<node::node_t> node = std::make_shared<node::node_t>();
        node->label(key);
        info << "[INFO] <make_node_s> created node \"" << key << "\" at " << node.get() << "\"\n";
        
        info << "[INFO] <make_node_s> adding node to map\n";
        nmap[key] = node;
        info << "[INFO] <make_node_s> added node to map node at" << &(nmap[key]) << "\n";
        
        node_ptr = node.get();
    }
    else{
        node_ptr = nullptr;
        info << "[ERROR] <make_node_s> a node named \"" << key << "\" already exists at " << &(nmap[key]) << " pointing to " << nmap[key].get() << "\n";
    }
    
    return info.str();
}

inline std::string make_node_s(std::map<std::string, std::shared_ptr<node::node_t>> & nmap, const std::string & key, node::node_t * & node_ptr, bool & flag_error){
    std::stringstream info;
    
    info << "[INFO] <make_node_s> searching for node \"" << key << "\"\n";
    const bool not_found = (nmap.contains(key) == false);
    
    if(not_found){
        info << "[INFO] <make_node_s> node not found\n";
        std::shared_ptr<node::node_t> node = std::make_shared<node::node_t>();
        node->label(key);
        info << "[INFO] <make_node_s> created node \"" << key << "\" at " << node.get() << "\"\n";
        
        info << "[INFO] <make_node_s> adding node to map\n";
        nmap[key] = node;
        info << "[INFO] <make_node_s> added node to map node at" << &(nmap[key]) << "\n";
        
        node_ptr = node.get();
    }
    else{
        node_ptr = nullptr;
        flag_error = true;
        info << "[ERROR] <make_node_s> a node named \"" << key << "\" already exists at " << &(nmap[key]) << " pointing to " << nmap[key].get() << "\n";
    }
    
    return info.str();
}

inline bool make_node(
    std::map<std::string, std::shared_ptr<node::node_t>> & map, 
    const std::string & key, 
    node::node_t * & node_ptr, 
    const double capacitance_value, 
    const bool hide = false
){
    bool inserted;
    std::shared_ptr<node::node_t> node = std::make_shared<node::node_t>();
    node->label(key);
    node->capacitance(capacitance_value);
    node->is_hidden(hide);
    
    if(map.contains(key) == false){
        map[key] = node;
        node_ptr = node.get();
        inserted = true;
    }
    else{
        node_ptr = nullptr;
        inserted = false;
    }
    
    return inserted;
}

inline bool add_printable(std::map<std::string, printable::var_info_t> & pmap, const std::string & name, const std::string & unit, auto & value){
    auto it = pmap.find(name);
    const bool not_found = (it == pmap.end());
    
    if(not_found){
        pmap[name] = printable::make_info(name, unit, value);
    }
    
    return not_found;
}

inline std::string add_printable_s(std::map<std::string, printable::var_info_t> & pmap, const std::string & name, const std::string & unit, auto & value){
    std::stringstream info;
    info << "[INFO] <add_printable_s> creating printable " << name << "\n";
    
    auto it = pmap.find(name);
    const bool not_found = (it == pmap.end());
    
    if(not_found){
        pmap[name] = printable::make_info(name, unit, value);
        info << "[INFO] <add_printable_s> created printable " << name << " at " << &(pmap[name]) << " pointing to " << pmap[name].var << "\n";
    }
    else{
        info << "[ERROR] <add_printable_s> a printable named " << name << " already exists at " << &(pmap[name]) << " pointing to " << pmap[name].var << "\n";
    }
    
    return info.str();
}

inline std::string add_printable_s(std::map<std::string, printable::var_info_t> & pmap, const std::string & name, const std::string & unit, auto & value, bool & flag_error){
    std::stringstream info;
    info << "[INFO] <add_printable_s> creating printable " << name << "\n";
    
    auto it = pmap.find(name);
    const bool not_found = (it == pmap.end());
    
    if(not_found){
        pmap[name] = printable::make_info(name, unit, value);
        info << "[INFO] <add_printable_s> created printable " << name << " at " << &(pmap[name]) << " pointing to " << pmap[name].var << "\n";
    }
    else{
        flag_error = true;
        info << "[ERROR] <add_printable_s> a printable named " << name << " already exists at " << &(pmap[name]) << " pointing to " << pmap[name].var << "\n";
    }
    
    return info.str();
}
/*
inline auto inv_sum(const auto lhs, const auto rhs){
    return 1/(1/lhs + 1/rhs);
}

inline auto inv_dif(const auto lhs, const auto rhs){
    return 1/(1/lhs - 1/rhs);
}
*/
}

#endif // UTILS_HPP
