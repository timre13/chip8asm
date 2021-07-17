#pragma once

#include <iostream>

#ifndef NDEBUG

#define TODO() \
do \
{ \
    std::cerr << "[<<< TODO >>>]: " __FILE__ ":" << __LINE__ << "\n"; \
} \
while (0)

#else

#define TODO() void(0)

#endif

