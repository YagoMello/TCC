#ifndef AUTOREGS_HPP
#define AUTOREGS_HPP

#include "../simulator/component.hpp"
#include "../libs/autoreg.hpp"

namespace database {
    autoreg::list<comp::component_t> & components();
}

#endif // AUTOREGS_HPP
