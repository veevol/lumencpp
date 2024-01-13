#ifndef LUMENCPP_SOURCE_REGION_H
#define LUMENCPP_SOURCE_REGION_H

#include "position.h"

namespace lumen {

struct SourceRegion {
    Position begin;
    Position end;
};

} // namespace lumen

#endif
