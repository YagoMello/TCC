#include "database.hpp"

autoreg::list<comp::component_t> & database::components(){
    // We use a static local because it is initialized when
    // the code reaches it's declaration:
    // side-effect global constructor => this function =>
    // storage initialization => initialized variable is returned
    // https://en.cppreference.com/w/cpp/language/storage_duration#Static_local_variables
    static autoreg::list<comp::component_t> storage;
    return storage;
}
