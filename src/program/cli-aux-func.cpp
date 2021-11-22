#include "cli.hpp"

std::string cli_t::ptr_to_str(const void * ptr){
    std::stringstream stream;
    stream << ptr;
    return stream.str();
}
