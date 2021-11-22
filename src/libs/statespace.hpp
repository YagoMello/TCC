#ifndef STATESPACE_HPP
#define STATESPACE_HPP

// INCLUDE <>
#include <cstdint>
#include <cmath>
#include <vector>
#include <numbers>
#include <stdexcept>
//#include <limits>

namespace ss {

template <typename T>
class siso_t {
public:
    using value_type = T;
    
    void configure(const size_t order){
        if(order == 0){
            throw std::invalid_argument("[siso_t] Cannot use order 0.");
        }
        coef_count_  = order;
        state_       = std::vector<value_type>(coef_count_);
        system_coef_ = std::vector<value_type>(coef_count_);
        input_coef_  = std::vector<value_type>(coef_count_);
        derivatives_ = std::vector<value_type>(coef_count_);
        clear();
    }
    
    void clear(){
        for(auto & val : state_){
            val = 0;
        }
        for(auto & val : derivatives_){
            val = 0;
        }
    }
    
    siso_t & add_sample(const value_type input, const value_type time_step){
        const size_t end = coef_count_;
        const size_t last = end - 1;
        
        for(size_t pos = 0; pos != last; pos++){
            derivatives_[pos] = system_coef_[pos] * state_[0] + state_[pos+1] + input_coef_[pos] * input;
        }
        derivatives_[last] = system_coef_[last] * state_[0] + input_coef_[last] * input;
        
        for(size_t pos = 0; pos != end; pos++){
            state_[pos] += time_step * derivatives_[pos];
        }
        
        return *this;
    }
    
    // ---- ACCESSORS ----
    
    const std::vector<value_type> & state() const {
        return state_;
    }
    
    value_type value() const {
        return state_[0];
    }
    
    size_t order() const {
        return coef_count_ + 1;
    }
    
    value_type derivative(const size_t n = 1) const {
        return derivatives_[n - 1];
    }
    
    // ---- MATH STUFF ----
    
    siso_t & numerator(const std::vector<value_type> & data){
        const size_t last = coef_count_ - 1;
        
        if(coef_count_ == 0){
            throw std::logic_error("[siso_t] Model is not configured.");
        }
        for(size_t pos = 0; pos != data.size(); pos++){
            if(pos <= last){
                input_coef_[last - pos] = data[pos];
            }
        }
        return *this;
    }
    
    siso_t & denominator(const std::vector<value_type> & data){
        const size_t last = coef_count_ - 1;
        
        if(coef_count_ == 0){
            throw std::logic_error("[siso_t] Model is not configured.");
        }
        
        double scaling_factor = 1;
        if(data.size() > last){
            scaling_factor = 1/data[last+1];
        }
        
        for(size_t pos = 0; pos != data.size(); pos++){
            if(pos <= last){
                system_coef_[last - pos] = -scaling_factor * data[pos];
            }
        }
        for(size_t pos = 0; pos != data.size(); pos++){
            if(pos <= last){
                input_coef_[last - pos] *= scaling_factor;
            }
        }
        
        return *this;
    }
    
    // ---- TEMPLATE SIMPLIFIER ----
    
    operator value_type() const {
        return value();
    }
    
    siso_t & operator = (const value_type value){
        if(value != 0){
            throw std::invalid_argument("Cannot set value other than 0");
        }
        clear();
        return *this;
    }
    
private:
    size_t coef_count_ = 0;
    std::vector<value_type> state_;
    std::vector<value_type> system_coef_;
    std::vector<value_type> input_coef_;
    std::vector<value_type> derivatives_;
};

using model_t = siso_t<double>;

} // namespace ss

#endif // STATESPACE_HPP
