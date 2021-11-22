#ifndef PRINTABLE_HPP
#define PRINTABLE_HPP

#include <limits>
#include <memory>
#include <string>
#include <type_traits>

#include "../simulator/print_mode.hpp"

namespace printable {

struct var_info_t;
struct var_data_t;

using printer_type = std::string (*)(const var_data_t & var_data);
using apply_iteration_type = void (*)(var_data_t & var_data, const double time_step);
using clear_print_memory_type = void (*)(var_data_t & var_data);
using make_shared_ptr_type = std::shared_ptr<void> (*)();

template <typename value_type>
std::string default_printer(const var_data_t & var_data);

template <typename value_type>
void default_apply_iteration(var_data_t & var_data, const double time_step);

template <typename value_type>
void default_clear_print_memory(var_data_t & var_data);

template <typename value_type>
std::shared_ptr<void> default_make_shared_ptr();

struct var_info_t {
    bool is_active = false;
    std::string name = "undef.";
    std::string unit = "";
    const void * var = nullptr;
    printer_type            printer;
    apply_iteration_type    apply_iteration;
    clear_print_memory_type clear_print_memory;
    make_shared_ptr_type    make_shared_ptr;
};

struct var_data_t {
    bool is_active = false;
    uint16_t precision = 10;
    print::mode printmode = print::mode::NONE;
    double avg_val  = 0;
    double avg_time = 0;
    std::shared_ptr<void> data = nullptr;
    const void * var = nullptr;
    printer_type            printer;
    apply_iteration_type    apply_iteration;
    clear_print_memory_type clear_print_memory;
};

template <typename value_type>
std::string default_printer(const var_data_t & var_data){
    using std::to_string;
    
    if(var_data.printmode != print::mode::AVERAGE){
        return to_string(*static_cast<value_type *>(var_data.data.get()));
    }
    else if(var_data.avg_time > 0){
        return to_string(var_data.avg_val / var_data.avg_time);
    }
    else{
        return "0";
    }
}

template <>
inline std::string default_printer<double>(const var_data_t & var_data){
    const std::string format = "%." + std::to_string(var_data.precision) + "e";
    std::string result;
    result.reserve(16);
    
    if(var_data.printmode != print::mode::AVERAGE){
        sprintf(result.data(), format.c_str(), *static_cast<double *>(var_data.data.get()));
    }
    else if(var_data.avg_time > 0){
        sprintf(result.data(), format.c_str(), var_data.avg_val / var_data.avg_time);
    }
    else{
        result = "0";
    }
    
    return result;
}

template <typename value_type>
void default_apply_iteration(var_data_t & var_data, const double time_step){
    const value_type & val = *static_cast<const value_type *>(var_data.var);
    value_type & data = *static_cast<value_type *>(var_data.data.get());
    print::mode print_mode = var_data.printmode;
    
    value_type val_abs;
    value_type data_abs;
    if constexpr(std::is_signed<value_type>::value){
        val_abs = std::abs(val);
        data_abs = std::abs(data);
    }
    else{
        val_abs = val;
        data_abs = data;
    }
        
    switch(print_mode){
        case print::mode::NONE:
            data = val;
            break;
        case print::mode::AVERAGE:
            if constexpr(std::is_constructible<value_type, double>::value){
                var_data.avg_val  += time_step * static_cast<double>(val);
                var_data.avg_time += time_step;
            }
            else {
                using std::to_string;
                throw std::invalid_argument("A printable using print mode \"AVERAGE\" with value " + to_string(val) + " is missing the conversion operator to \"double\"");
            }
            break;
        case print::mode::MAX:
            if(val > data){
                data = val;
            }
            break;
        case print::mode::MIN:
            if(val < data){
                data = val;
            }
            break;
        case print::mode::MAX_ABS:
            if(val_abs > data_abs){
                data = val;
            }
            break;
        case print::mode::MIN_ABS:
            if(val_abs < data_abs){
                data = val;
            }
            break;
        case print::mode::MAX_AMPL:
            if(val_abs > data_abs){
                data = val_abs;
            }
            break;
        case print::mode::MIN_AMPL:
            if(val_abs < data_abs){
                data = val_abs;
            }
            break;
        case print::mode::UNDEFINED:
            throw std::invalid_argument("Print mode is \"UNDEFINED\"\n");
            break;
        default:
            throw std::invalid_argument("Unknown print mode\n");
            break;
    }
}

template <typename value_type>
void default_clear_print_memory(var_data_t & var_data){
    using std::numeric_limits; // this allows custom types to define their numeric_limits<T>::...
    
    value_type & data = *static_cast<value_type *>(var_data.data.get());
    var_data.avg_val = 0;
    var_data.avg_time = 0;
    
    switch(var_data.printmode) {
        case print::mode::MAX:
            data = numeric_limits<value_type>::lowest();
            break;
        case print::mode::MIN:
        case print::mode::MIN_ABS:
        case print::mode::MIN_AMPL:
            data = numeric_limits<value_type>::max();
            break;
        default:
            data = 0;
            break;
    }
}

template <typename value_type>
std::shared_ptr<void> default_make_shared_ptr(){
    return std::make_shared<value_type>();
}

template <typename value_type>
var_info_t make_info(
    const std::string & name,
    const std::string & unit,
    const value_type & var, 
    printer_type            printer_fn            = &default_printer<value_type>,
    apply_iteration_type    apply_iteration_fn    = &default_apply_iteration<value_type>,
    clear_print_memory_type clear_print_memory_fn = &default_clear_print_memory<value_type>,
    make_shared_ptr_type    make_shared_ptr_fn    = &default_make_shared_ptr<value_type>
){
    var_info_t info = {
        .is_active          = false,
        .name               = name,
        .unit               = unit,
        .var                = static_cast<const void *>(&var),
        .printer            = printer_fn,
        .apply_iteration    = apply_iteration_fn,
        .clear_print_memory = clear_print_memory_fn,
        .make_shared_ptr    = make_shared_ptr_fn
    };
    return info;
}

inline var_data_t make_from_info(const var_info_t & info){
    var_data_t var_data = {
        .is_active          = info.is_active,
        .printmode          = print::mode::UNDEFINED,
        .avg_val            = 0,
        .avg_time           = 0,
        .data               = info.make_shared_ptr(),
        .var                = info.var,
        .printer            = info.printer,
        .apply_iteration    = info.apply_iteration,
        .clear_print_memory = info.clear_print_memory
    };
    return var_data;
}

template <typename value_type>
var_data_t make_from_var(
    const std::string & name,
    const std::string & unit,
    const value_type & var
){
    return make_from_info(make_info(name, unit, var)); // expiring ref returned by make_info
}

}

#endif // PRINTABLE_HPP
