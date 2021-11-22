#ifndef FILTER_HPP
#define FILTER_HPP

// INCLUDE <>
#include <cstdint>
#include <cmath>
#include <vector>
#include <array>
#include <numbers>
#include <stdexcept>


namespace filt {
namespace dynamic {

template <typename T = double>
class butterworth_t {
public:
    using value_type = T;
    
    void configure(const size_t order, const value_type freq_cutoff){
        w_cutoff_ = 2 * std::numbers::pi * freq_cutoff;
        order_  = order;
        coeffs_ = coeffs_ss(order, freq_cutoff);
        state_  = std::vector<value_type>(order);
        derivatives_ = std::vector<value_type>(order);
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
    
    double add_sample(const value_type input, const value_type time_step){
        const size_t     last_pos = order_ - 1;
        const value_type last_val = state_[last_pos];
        
        derivatives_[0] = w_cutoff_ * input + coeffs_[0] * last_val;
        for(size_t pos = 1; pos < order_; pos++){
            derivatives_[pos] = w_cutoff_ * state_[pos - 1] + coeffs_[pos] * last_val;
        }
        
        for(size_t pos = 0; pos < order_; pos++){
            state_[pos] += time_step * derivatives_[pos];
        }
        
        return state_[last_pos];
    }
    
    // ---- ACCESSORS ----
    
    const std::vector<value_type> & state() const {
        return state_;
    }
    
    const value_type & state(const size_t n = 0) const {
        return state_[order_ - n - 1];
    }
    
    value_type value() const {
        return state_[order_ - 1];
    }
    
    size_t order() const {
        return order_;
    }
    
    const value_type & derivative(const size_t n = 1) const {
        return derivatives_[order_ - n];
    }
    
    // ---- MATH STUFF ----
    
    static std::vector<value_type> coeffs(const size_t order){
        const size_t count = order;
        std::vector<value_type> values(count);

        value_type acc = 1;
        value_type index = 1;
        value_type gamma = std::numbers::pi / (2 * value_type(order));
        
        for(size_t pos = 0; pos < count; pos++){
            values[pos] = acc;
            acc = acc * std::cos((index - 1) * gamma) / std::sin(index * gamma);
            
            index += 1;
        }
        
        return values;
    }
    
    static std::vector<value_type> coeffs_ss(const size_t order, const double freq_cutoff){
        auto values = coeffs(order);
        
        for(auto & val : values){
            val = -2 * std::numbers::pi * freq_cutoff * val;
        }
        
        return values;
    }
    
    // ---- TEMPLATE SIMPLIFIER ----
    
    operator value_type() const {
        return value();
    }
    
    butterworth_t & operator = (const value_type value){
        if(value != 0){
            throw std::invalid_argument("Cannot set value other than 0");
        }
        clear();
        return *this;
    }
    
private:
    size_t order_;
    value_type w_cutoff_;
    std::vector<value_type> state_;
    std::vector<value_type> coeffs_;
    std::vector<value_type> derivatives_;
};

template <typename T = double>
class critical_t {
public:
    using value_type = T;
    
    void configure(const size_t order, const value_type freq_cutoff){
        coef_count_  = order;
        system_coef_ = coeffs_ss(order, freq_cutoff);
        pole_w_      = (2 * std::numbers::pi * freq_cutoff) / sqrt(pow(2, 1/double(order)) - 1);
        state_       = std::vector<value_type>(order);
        derivatives_ = std::vector<value_type>(order);
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
    
    void add_sample(const value_type input, const value_type time_step){
        const size_t end = coef_count_;
        const size_t last = end - 1;
        
        for(size_t pos = 0; pos != last; pos++){
            derivatives_[pos] = system_coef_[pos] * state_[0] + pole_w_ * state_[pos+1];
        }
        derivatives_[last] = system_coef_[last] * (state_[0] - input);
        
        for(size_t pos = 0; pos != end; pos++){
            state_[pos] += time_step * derivatives_[pos];
        }
    }
    
    // ---- ACCESSORS ----
    
    const std::vector<value_type> & state() const {
        return state_;
    }
    
    const value_type & state(const size_t n = 0) const {
        return state_[n];
    }
    
    value_type value() const {
        return state_[0];
    }
    
    size_t order() const {
        return coef_count_;
    }
    
    const value_type & derivative(const size_t n = 1) const {
        return derivatives_[n - 1];
    }
    
    // ---- MATH STUFF ----
    
    static std::vector<value_type> coeffs(const size_t order){
        const size_t count = order;
        std::vector<value_type> values(count);

        size_t coef_n = 1;
        
        // binomial expansion coefficients of (s+p)^n
        for(size_t pos = 0; pos < count; pos++){
            coef_n = ((order - pos) * coef_n) / (pos + 1); 
            values[pos] = value_type(coef_n);
        }
        
        return values;
    }
    
    static std::vector<value_type> coeffs_ss(const size_t order, const value_type freq_cutoff){
        auto values = coeffs(order);
        const size_t last = order - 1;
        
        value_type pole_w = (2 * std::numbers::pi * freq_cutoff) / sqrt(pow(2, 1/double(order)) - 1);
        
        // substituting p = w_cutoff
        // s^n + s^(n-1)*p + s^(n-1)*p^2 + ...
        for(size_t pos = 0; pos <= last; pos++){
            values[pos] *= -pole_w;
        }
        
        return values;
    }
    
    // ---- TEMPLATE SIMPLIFIER ----
    
    operator value_type() const {
        return value();
    }
    
    auto & operator = (const value_type value){
        if(value != 0){
            throw std::invalid_argument("Cannot set value other than 0");
        }
        clear();
        return *this;
    }
    
private:
    size_t coef_count_ = 0;
    value_type pole_w_;
    std::vector<value_type> state_;
    std::vector<value_type> system_coef_;
    std::vector<value_type> derivatives_;
};

} // dynamic
namespace fixed {

template <size_t N, typename T = double>
class butterworth_t {
public:
    using value_type = T;
    static constexpr size_t order = N;
    using vect_t = std::array<value_type, order>;
    
    constexpr void configure(const value_type freq_cutoff){
        w_cutoff_ = 2 * std::numbers::pi * freq_cutoff;
        coeffs_ = coeffs_ss(freq_cutoff);
        clear();
    }
    
    constexpr void clear(){
        for(auto & val : state_){
            val = 0;
        }
        for(auto & val : derivatives_){
            val = 0;
        }
    }
    
    constexpr double add_sample(const value_type input, const value_type time_step){
        constexpr size_t last_pos = order - 1;
        const value_type last_val = state_[last_pos];
        
        derivatives_[0] = w_cutoff_ * input + coeffs_[0] * last_val;
        for(size_t pos = 1; pos < order; pos++){
            derivatives_[pos] = w_cutoff_ * state_[pos - 1] + coeffs_[pos] * last_val;
        }
        
        for(size_t pos = 0; pos < order; pos++){
            state_[pos] += time_step * derivatives_[pos];
        }
        
        return state_[last_pos];
    }
    
    // ---- ACCESSORS ----
    
    constexpr const vect_t & state() const {
        return state_;
    }
    
    const value_type & state(const size_t n = 0) const {
        return state_[order - n - 1];
    }
    
    constexpr value_type value() const {
        return state_[order - 1];
    }
    
    constexpr const value_type & derivative(const size_t n = 1) const {
        return derivatives_[order - n];
    }
    
    // ---- MATH STUFF ----
    
    static constexpr vect_t coeffs(){
        const size_t count = order;
        vect_t values;

        value_type acc = 1;
        value_type index = 1;
        value_type gamma = std::numbers::pi / (2 * value_type(order));
        
        for(size_t pos = 0; pos < count; pos++){
            values[pos] = acc;
            acc = acc * std::cos((index - 1) * gamma) / std::sin(index * gamma);
            
            index += 1;
        }
        
        return values;
    }
    
    static constexpr vect_t coeffs_ss(const double freq_cutoff){
        auto values = coeffs();
        
        for(auto & val : values){
            val = -2 * std::numbers::pi * freq_cutoff * val;
        }
        
        return values;
    }
    
    // ---- TEMPLATE SIMPLIFIER ----
    
    constexpr operator value_type() const {
        return value();
    }
    
    constexpr butterworth_t & operator = (const value_type value){
        if(value != 0){
            throw std::invalid_argument("Cannot set value other than 0");
        }
        clear();
        return *this;
    }
    
private:
    value_type w_cutoff_;
    vect_t state_;
    vect_t coeffs_;
    vect_t derivatives_;
};

template <size_t N, typename T = double>
class critical_t {
public:
    using value_type = T;
    using array_t = std::array<value_type, N>;
    
    constexpr void configure(const value_type freq_cutoff){
        system_coef_ = coeffs_ss(freq_cutoff);
        pole_w_ = (2 * std::numbers::pi * freq_cutoff) / sqrt(pow(2, 1/double(order())) - 1);
        clear();
    }
    
    constexpr void clear(){
        for(auto & val : state_){
            val = 0;
        }
        for(auto & val : derivatives_){
            val = 0;
        }
    }
    
    constexpr void add_sample(const value_type input, const value_type time_step){
        const size_t end = coef_count_;
        const size_t last = end - 1;
        
        for(size_t pos = 0; pos != last; pos++){
            derivatives_[pos] = system_coef_[pos] * state_[0] + pole_w_ * state_[pos+1];
        }
        derivatives_[last] = system_coef_[last] * (state_[0] - input);
        
        for(size_t pos = 0; pos != end; pos++){
            state_[pos] += time_step * derivatives_[pos];
        }
    }
    
    // ---- ACCESSORS ----
    
    constexpr const array_t & state() const {
        return state_;
    }
    
    const value_type & state(const size_t n = 0) const {
        return state_[n];
    }
    
    constexpr value_type value() const {
        return state_[0];
    }
    
    static constexpr size_t order() {
        return coef_count_;
    }
    
    constexpr const value_type & derivative(const size_t n = 1) const {
        return derivatives_[n - 1];
    }
    
    // ---- MATH STUFF ----
    
    static constexpr array_t coeffs(){
        array_t values;

        size_t coef_n = 1;
        
        // binomial expansion coefficients of (s+p)^n
        for(size_t pos = 0; pos < coef_count_; pos++){
            coef_n = ((order() - pos) * coef_n) / (pos + 1); 
            values[pos] = value_type(coef_n);
        }
        
        return values;
    }
    
    static constexpr array_t coeffs_ss(const value_type freq_cutoff){
        auto values = coeffs();
        const size_t last = coef_count_ - 1;
        
        value_type pole_w = (2 * std::numbers::pi * freq_cutoff) / sqrt(pow(2, 1/double(order())) - 1);
        
        // substituting p = w_cutoff
        // s^n + s^(n-1)*p + s^(n-1)*p^2 + ...
        for(size_t pos = 0; pos <= last; pos++){
            values[pos] *= -pole_w;
        }
        
        return values;
    }
    
    // ---- TEMPLATE SIMPLIFIER ----
    
    constexpr operator value_type() const {
        return value();
    }
    
    constexpr auto & operator = (const value_type value){
        if(value != 0){
            throw std::invalid_argument("Cannot set value other than 0");
        }
        clear();
        return *this;
    }
    
private:
    static constexpr size_t coef_count_ = N;
    value_type pole_w_;
    array_t state_;
    array_t system_coef_;
    array_t derivatives_;
};

} // fixed
namespace cst {

template <size_t N, auto F, typename T = double>
class critical_t {
public:
    using value_type = T;
    static constexpr value_type freq_cutoff = value_type(F);
    using array_t = std::array<value_type, N>;
    
    constexpr void configure(){
        system_coef_ = coeffs_ss();
        pole_w_ = (2 * std::numbers::pi * freq_cutoff) / sqrt(pow(2, 1/double(order())) - 1);
        clear();
    }
    
    constexpr void clear(){
        for(auto & val : state_){
            val = 0;
        }
        for(auto & val : derivatives_){
            val = 0;
        }
    }
    
    constexpr void add_sample(const value_type input, const value_type time_step){
        const size_t end = coef_count_;
        const size_t last = end - 1;
        
        for(size_t pos = 0; pos != last; pos++){
            derivatives_[pos] = system_coef_[pos] * state_[0] + pole_w_ * state_[pos+1];
        }
        derivatives_[last] = system_coef_[last] * (state_[0] - input);
        
        for(size_t pos = 0; pos != end; pos++){
            state_[pos] += time_step * derivatives_[pos];
        }
    }
    
    // ---- ACCESSORS ----
    
    constexpr const array_t & state() const {
        return state_;
    }
    
    const value_type & state(const size_t n = 0) const {
        return state_[n];
    }
    
    constexpr value_type value() const {
        return state_[0];
    }
    
    static constexpr size_t order() {
        return coef_count_;
    }
    
    constexpr const value_type & derivative(const size_t n = 1) const {
        return derivatives_[n - 1];
    }
    
    // ---- MATH STUFF ----
    
    static constexpr array_t coeffs(){
        array_t values;

        size_t coef_n = 1;
        
        // binomial expansion coefficients of (s+p)^n
        for(size_t pos = 0; pos < coef_count_; pos++){
            coef_n = ((order() - pos) * coef_n) / (pos + 1); 
            values[pos] = value_type(coef_n);
        }
        
        return values;
    }
    
    static constexpr array_t coeffs_ss(){
        auto values = coeffs();
        const size_t last = coef_count_ - 1;
        
        value_type pole_w = (2 * std::numbers::pi * freq_cutoff) / sqrt(pow(2, 1/double(order())) - 1);
        
        // substituting p = w_cutoff
        // s^n + s^(n-1)*p + s^(n-1)*p^2 + ...
        for(size_t pos = 0; pos <= last; pos++){
            values[pos] *= -pole_w;
        }
        
        return values;
    }
    
    // ---- TEMPLATE SIMPLIFIER ----
    
    constexpr operator value_type() const {
        return value();
    }
    
    constexpr auto & operator = (const value_type value){
        if(value != 0){
            throw std::invalid_argument("Cannot set value other than 0");
        }
        clear();
        return *this;
    }
    
private:
    static constexpr size_t coef_count_ = N;
    value_type pole_w_;
    array_t state_;
    array_t system_coef_;
    array_t derivatives_;
};

}

template <size_t N>
using lpf_t = filt::fixed::butterworth_t<N, double>;

using lpfd_t = filt::dynamic::butterworth_t<double>;

} // filt

#endif // FILTER_HPP
