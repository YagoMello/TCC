#ifndef BIT_HPP
#define BIT_HPP

#include <type_traits>

template <typename T>
class bit_t {
public:
    using reg_type = T;
    
    bit_t() : reg_(nullptr) { }
    
    bit_t(reg_type * reg, const uint8_t bit) :
        reg_(reg),
        mask_(reg_type(1) << bit)
    { }
    
    bit_t & operator =(bool value) {
        *reg_ &= reg_type(~mask_);
        *reg_ |= reg_type(reg_type(value) * mask_);
        
        return *this;
    }
    
    operator bool() const {
        return (*reg_ & mask_) != reg_type(0);
    }
    
private:
    reg_type * reg_;
    reg_type mask_;
};

template <auto * reg, auto bit>
class cbit_t {
public:
    using reg_type = std::remove_pointer<decltype(reg)>::type;
    using value_type = std::remove_volatile<reg_type>::type;
    
    constexpr cbit_t & operator =(bool value) {
        const value_type reg_clr = *reg & mask_clear_;
        *reg = reg_clr | value_type(value_type(value) * mask_bit_);
        
        return *this;
    }
    
    constexpr operator bool() const {
        return (*reg & mask_bit_) != value_type(0);
    }
    
private:
    static constexpr value_type mask_bit_ = value_type(1) << bit;
    static constexpr value_type mask_clear_ = ~mask_bit_;
};

#endif // BIT_HPP
