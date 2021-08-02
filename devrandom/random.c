#include "random.h"

typedef UINT32 uint32_t;
typedef UINT64 uint64_t;

// taken from: https://github.com/lemire/testingRNG/blob/master/source/mitchellmoore.h

/* Mitchell-Moore algorithm from
 * "The Art of Computer Programming, Volume II"
 * by Donald E. Knuth
 *
 * Improvements based on
 * "Uniform Random Number Generators for Vector and Parallel Computers"
 * by Richard P. Brent */

#define R 250U
#define S 200U
#define T 103U
#define U 50U

static uint32_t mitchellmoore_sequence[R];
static unsigned int mitchellmoore_a = R, mitchellmoore_b = S,
mitchellmoore_c = T, mitchellmoore_d = U;

static inline void mitchellmoore_seed(uint64_t seed) 
{
    unsigned int i;

    for (i = 0; i < R * 2; i++)
    {
        seed = (1664525 * seed + 1013904223);
        mitchellmoore_sequence[i % R] = (uint32_t)seed;
    }

    mitchellmoore_sequence[0] <<= 1;
    mitchellmoore_sequence[1] |= 1;

    return;
}

static inline uint32_t mitchellmoore(void) {
    return mitchellmoore_sequence[++mitchellmoore_a % R] +=
        mitchellmoore_sequence[++mitchellmoore_b % R] +=
        mitchellmoore_sequence[++mitchellmoore_c % R] +=
        mitchellmoore_sequence[++mitchellmoore_d % R];
}

#undef R
#undef S
#undef T
#undef U

VOID
NTAPI
DevRandomFillBufferRand(
    PVOID Buffer,
    SIZE_T Length)
{
    LARGE_INTEGER randomState;

    //
    // Setup random state
    //
    KeQueryTickCount(&randomState);
    mitchellmoore_seed(randomState.QuadPart);


    PUINT32 ptr = Buffer;
    for (UINT32 i = 0; i < Length / sizeof(UINT32); i++)
    {
        *ptr++ = mitchellmoore();
    }

    Length &= sizeof(UINT32) - 1;
    if (Length > 0) 
    {
        UINT32 rem = mitchellmoore();
        RtlCopyMemory(ptr, &rem, Length);
    }
}
