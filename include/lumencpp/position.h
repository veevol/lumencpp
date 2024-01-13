#ifndef LUMENCPP_POSITION_H
#define LUMENCPP_POSITION_H

#include <cstdint>

namespace lumen {

struct Position {
    std::uint32_t line = 1;
    std::uint32_t column = 1;
};

} // namespace lumen

#endif
