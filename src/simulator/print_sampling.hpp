#ifndef PRINT_SAMPLING_HPP
#define PRINT_SAMPLING_HPP

#include <cinttypes>
#include <string>
#include <map>

namespace print_sampling {
    using list_type = std::map<std::string, uint32_t>;
    
    extern list_type list;
    
    enum : uint32_t {
        UNDEFINED = 0,
        INTERVAL  = 1,
        POINTS    = 2
    };
}

#endif // PRINT_SAMPLING_HPP