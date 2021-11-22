#ifndef PRINT_MODE_HPP
#define PRINT_MODE_HPP

#include <cinttypes>
#include <string>
#include <map>

namespace print {
    enum class mode : uint32_t {
        UNDEFINED = 0,
        NONE      = 1,
        AVERAGE   = 2,
        MAX       = 3,
        MIN       = 4,
        MAX_ABS   = 5,
        MIN_ABS   = 6,
        MAX_AMPL  = 7,
        MIN_AMPL  = 8
    };
    
    namespace list {
    using type = std::map<std::string, print::mode>;
    
    extern list::type names;
    }
}

#endif // PRINT_MODE_HPP