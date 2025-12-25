#include "runtime.h"
#include <cstdio>

extern "C" void bf_put(unsigned char c)
{
    std::putchar(c);
    std::fflush(stdout);
}

extern "C" int bf_get()
{
    int ch = std::getchar();
    if (ch == EOF)
        return 0;
    return ch & 0xFF;
}
