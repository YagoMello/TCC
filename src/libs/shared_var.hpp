#ifndef SHARED_VAR_HPP
#define SHARED_VAR_HPP

//#include <source_location>
#include <exception>
#include <string>
#include <typeinfo>
#include <memory>
#include <map>

namespace shared {

struct info_t;
using list = std::map<std::string, info_t>;

struct info_t {
    info_t * redirect = nullptr;
    std::shared_ptr<void> ptr;
    std::string type_name = "";
    //std::source_location where;
};

size_t get_uinque_id();

#ifdef SHARED_VAR_IMPL
size_t get_uinque_id(){
    static size_t id = 1;
    return id++;
}
#endif

template <typename T>
inline void create(shared::list & ls, const std::string & key, const T & default_value = T()/*, std::source_location location = std::source_location::current()*/) {
    auto it = ls.find(key);
    if(key == "" || key == "null"){
        info_t info;
        
        auto data_ptr = std::make_shared<T>();
        *data_ptr = default_value;
        
        info.ptr = data_ptr;
        info.type_name = typeid(T).name();
        //info->where = location
        
        ls["local_var_" + std::to_string(shared::get_uinque_id())] = info;
    }
    else if(it != ls.end()){
        if(it->second.type_name != typeid(T).name()){
            throw std::invalid_argument(std::string("shared_var: var type \"") + typeid(T).name() + "\" does not match \"" + it->second.type_name + "\" defined at --source location--");
        }
    }
    else{
        info_t info;
        
        auto data_ptr = std::make_shared<T>();
        *data_ptr = default_value;
        
        info.ptr = data_ptr;
        info.type_name = typeid(T).name();
        //info->where = location
        
        ls[key] = info;
    }
}

inline void bind_and_reverse(info_t * obj, info_t * tgt) {
    if(obj->type_name == tgt->type_name){
        info_t * old_tgt = obj->redirect;
        
        obj->redirect = tgt;
        
        if(old_tgt != nullptr){
            bind_and_reverse(old_tgt, obj);
        }
    }
    else{
        throw std::invalid_argument(std::string("shared_var: var type \"") + obj->type_name + "\" does not match \"" + obj->type_name + "\"");
    }
}

inline void * get_root(const info_t * info) {
    if(info->redirect == nullptr){
        return info->ptr.get();
    }
    else{
        return get_root(info->redirect);
    }
}

inline void bind(shared::list & ls, const std::string & old_name, const std::string & new_name/*, std::source_location location = std::source_location::current()*/) {
    auto it_old = ls.find(old_name);
    auto it_new = ls.find(new_name);
    if(it_old == ls.end() && it_new == ls.end()){
        throw std::invalid_argument(std::string("shared_var: Cannot bind non existent vars! (") + old_name + ", " + new_name + ")");
    }
    else if(it_old == ls.end()){
        ls[old_name] = info_t();
        ls[old_name].redirect = &ls[new_name];
    }
    else if(it_new == ls.end()){
        ls[new_name] = info_t();
        ls[new_name].redirect = &ls[old_name];
    }
    else{
        if(it_new->second.type_name != it_old->second.type_name){
            throw std::invalid_argument(std::string("shared_var: Cannot bind variables of different types: \"") + it_new->second.type_name + "\" and \"" + it_old->second.type_name + "\"");
        }
        bind_and_reverse(&it_old->second, &it_new->second);
    }
}

template <typename T>
class var {
public:
    using value_type = T;
    
    void create(shared::list & ls, const std::string & key, const T & default_value = T()) {
        list_ptr_ = &ls;
        key_ = key;
        shared::create(ls, key, default_value);
    }
    void init() {
        auto it = list_ptr_->find(key_);
        
        if(it != list_ptr_->end()){
            data_ptr_ = static_cast<value_type *>(shared::get_root(&it->second));
        }
        else{
            throw std::out_of_range(std::string("shared_var: Failed to init var (name not found in var list)"));
        }
    }
    value_type & data() {
        return *data_ptr_;
    }
    operator value_type &() const {
        return *data_ptr_;
    }
    value_type & operator =(const value_type & rhs) {
        *data_ptr_ = rhs;
        return *data_ptr_;
    }
private:
    value_type * data_ptr_;
    shared::list * list_ptr_;
    std::string key_;
};

}

#endif // SHARED_VAR_HPP
