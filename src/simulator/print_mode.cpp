#include "print_mode.hpp"

using namespace print;
list::type list::names = {
    {"none",     mode::NONE}, 
    {"average",  mode::AVERAGE}, 
    {"max",      mode::MAX}, 
    {"min",      mode::MIN}, 
    {"max-abs",  mode::MAX_ABS}, 
    {"min-abs",  mode::MIN_ABS},
    {"max-ampl", mode::MAX_AMPL},
    {"min-ampl", mode::MIN_AMPL}
};
