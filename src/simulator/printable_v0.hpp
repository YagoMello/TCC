#ifndef PRINTABLE_HPP
#define PRINTABLE_HPP

#include <string>
#include <cinttypes>

#include "../simulator/print_mode.hpp"

namespace printable {

enum payload_type_t {
    UNDEFINED, 
    DOUBLE, 
    UNSIGNED, 
    INTEGER
};

class printable_t {
public:
    void apply_iteration(const double time_step){
        if     (payload_type == printable::DOUBLE)   update_fp(time_step);
        else if(payload_type == printable::UNSIGNED) update_u64(time_step);
        else if(payload_type == printable::INTEGER)  update_i64(time_step);
        else throw;
    }
    void clear_print_memory(){
        time_ = 0;
        if     (is_fp())  fp = 0;
        else if(is_u64()) u64 = 0;
        else if(is_i64()) i64 = 0;
        else throw;
    }
    void print_mode(const uint32_t value){
        print_mode_ = value;
    }
    [[nodiscard]] uint32_t is_fp() const {
        return payload_type == printable::DOUBLE || print_mode_ == print_mode::AVERAGE;
    }
    [[nodiscard]] uint32_t is_u64() {
        return payload_type == printable::UNSIGNED;
    }
    [[nodiscard]] uint32_t is_i64() {
        return payload_type == printable::INTEGER;
    }
    payload_type_t type() const {
        return payload_type;
    }
    
    const std::string & name() const {
        return name_;
    }
    
    double as_fp() const {
        if(print_mode_ == print_mode::AVERAGE){
            return fp / time_;
        }
        else{
            return fp;
        }
    }
    
    uint64_t as_u64() const {
        return u64;
    }
    
    int64_t as_i64() const {
        return i64;
    }
    
    friend printable_t make_printable(const std::string &, double &);
    friend printable_t make_printable(const std::string &, uint64_t &);
    friend printable_t make_printable(const std::string &, int64_t &);
    
private:
    union {
        double * fp_ptr;
        uint64_t * u64_ptr;
        int64_t * i64_ptr;
    };
    
    union {
        double fp;
        uint64_t u64;
        int64_t i64;
    };
    
    double time_;
    
    std::string name_ = "undef.";
    payload_type_t payload_type = printable::UNDEFINED;
    uint32_t print_mode_ = print_mode::UNDEFINED;
    
    void update_fp(double time_step){
        double val = *fp_ptr;
        double val_abs = std::abs(val);
        switch(print_mode_){
            case print_mode::NONE:
                fp = val;
                break;
            case print_mode::AVERAGE:
                fp    += time_step * val;
                time_ += time_step;
                break;
            case print_mode::PEAK:
                if(val_abs > std::abs(fp)){
                    fp = val;
                }
                break;
            case print_mode::PEAK_ABS:
                if(val_abs > fp){
                    fp = val_abs;
                }
                break;
        }
    }
    
    void update_u64(double time_step){
        uint64_t val = *u64_ptr;
        switch(print_mode_){
            case print_mode::NONE:
                u64 = val;
                break;
            case print_mode::AVERAGE:
                fp += time_step * double(val);
                time_  += time_step;
                break;
            case print_mode::PEAK:
                if(val > u64){
                    u64 = val;
                }
                break;
            case print_mode::PEAK_ABS:
                if(val > u64){
                    u64 = val;
                }
                break;
        }
    }
    
    void update_i64(double time_step){
        int64_t val = *i64_ptr;
        int64_t val_abs = std::abs(val);
        switch(print_mode_){
            case print_mode::NONE:
                i64 = val;
                break;
            case print_mode::AVERAGE:
                fp += time_step * double(val);
                time_  += time_step;
                break;
            case print_mode::PEAK:
                if(val_abs > std::abs(i64)){
                    i64 = val;
                }
                break;
            case print_mode::PEAK_ABS:
                if(val_abs > i64){
                    i64 = val_abs;
                }
                break;
        }
    }
    
};

inline printable_t make_printable(const std::string & name, double & var){
    printable_t obj;
    obj.name_ = name;
    obj.fp_ptr = &var;
    obj.payload_type = DOUBLE;
    return obj;
}

inline printable_t make_printable(const std::string & name, uint64_t & var){
    printable_t obj;
    obj.name_ = name;
    obj.u64_ptr = &var;
    obj.payload_type = UNSIGNED;
    return obj;
}

inline printable_t make_printable(const std::string & name, int64_t & var){
    printable_t obj;
    obj.name_ = name;
    obj.i64_ptr = &var;
    obj.payload_type = INTEGER;
    return obj;
}

} // namespace printable

#endif // PRINTABLE_HPP
