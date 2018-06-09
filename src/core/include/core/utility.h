#pragma once

#include <stdint.h>

#define core_alignUp(X, SHIFT) \
    (((((X) - 1u) >> (SHIFT)) + 1u) << (SHIFT))

#define core_alignDown(X, SHIFT)\
    (((X) >> (SHIFT)) << (SHIFT))
