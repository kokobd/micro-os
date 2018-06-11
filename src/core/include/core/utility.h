#pragma once

#include <stdint.h>

#define core_alignUp(X, SHIFT) \
    (((((X) - 1u) >> (SHIFT)) + 1u) << (SHIFT))

#define core_alignDown(X, SHIFT)\
    (((X) >> (SHIFT)) << (SHIFT))

#define core_declareStruct(S) \
struct S;\
typedef struct S S

#define core_divUp(X, Y)\
    (((X) + (Y) - 1) / (Y))
