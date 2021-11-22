#ifndef PRINTABLE_HPP
#define PRINTABLE_HPP

#include <memory>
#include <string>
#include <type_traits>

#include "../simulator/print_mode.hpp"

namespace printable {

struct printable_t;

using printer_type = std::string (*)(const printable_t *);
using apply_iteration_type = void (*)(const double time_step, printable_t * printable);
using clear_print_memory_type = void (*)(printable_t * printable);

template <typename value_type>
std::string default_printer(const printable_t * printable);

template <typename value_type>
void default_apply_iteration(const double time_step, printable_t * printable);

template <typename value_type>
void default_clear_print_memory(printable_t * printable);

struct printable_t {
    bool is_active = false;
    std::string name = "undef.";
    std::string unit = "";
    print::mode printmode = print::mode::NONE;
    double avg_val  = 0;
    double avg_time = 0;
    std::shared_ptr<void> data = nullptr;
    void * var = nullptr;
    printer_type printer;
    apply_iteration_type    apply_iteration;
    clear_print_memory_type clear_print_memory;
};

template <typename value_type>
std::string default_printer(const printable_t * printable){
    // Using ADL
    using std::to_string;
    
    if(printable->printmode != print::mode::AVERAGE){
        return to_string(*reinterpret_cast<value_type *>(printable->data.get()));
    }
    else{
        return to_string(printable->avg_val / printable->avg_time);
    }
}

template <typename value_type>
void default_apply_iteration(const double time_step, printable_t * printable){
    const value_type & val = *reinterpret_cast<value_type *>(printable->var);
    value_type & data = *reinterpret_cast<value_type *>(printable->data.get());
    print::mode print_mode = printable->printmode;
    
    if constexpr(std::is_signed<value_type>::value){
        const value_type val_abs = std::abs(val);
        
        if(print_mode == print::mode::NONE){
            data = val;
        }
        else if(print_mode == print::mode::AVERAGE){
            printable->avg_val  += time_step * static_cast<double>(val);
            printable->avg_time += time_step;
        }
        else if(print_mode == print::mode::PEAK){
            if(val_abs > std::abs(data)){
                data = val;
            }
        }
        else if(print_mode == print::mode::PEAK_ABS){
            if(val_abs > data){
                data = val_abs;
            }
        }
        else{
            throw;
        }
    }
    else{
        if(print_mode == print::mode::NONE){
            data = val;
        }
        else if(print_mode == print::mode::AVERAGE){
            printable->avg_val  += time_step * static_cast<double>(val);
            printable->avg_time += time_step;
        }
        else if(print_mode == print::mode::PEAK){
            if(val > data){
                data = val;
            }
        }
        else if(print_mode == print::mode::PEAK_ABS){
            if(val > data){
                data = val;
            }
        }
    }
}

template <typename value_type>
void default_clear_print_memory(printable_t * printable){
    value_type & data = *reinterpret_cast<value_type *>(printable->data.get());
    data = 0;
    printable->avg_val  = 0;
    printable->avg_time = 0;
}

template <typename value_type>
printable_t make(
    const std::string & name,
    const std::string & unit,
    value_type & var, 
    printer_type printer = &default_printer<value_type>,
    apply_iteration_type    apply_iteration    = &default_apply_iteration<value_type>,
    clear_print_memory_type clear_print_memory = &default_clear_print_memory<value_type>
){
    printable_t obj;
    obj.name    = name;
    obj.unit    = unit;
    obj.data    = std::make_shared<value_type>();
    obj.var     = reinterpret_cast<void *>(&var);
    obj.printer = printer;
    obj.apply_iteration    = apply_iteration;
    obj.clear_print_memory = clear_print_memory;
    return obj;
}

}

#endif // PRINTABLE_HPP
