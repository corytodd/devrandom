#ifndef PCG32_H
#define PCG32_H

/* Modified by D. Lemire based on original code by M. O'Neill, August 2017 */
// #include <stdint.h>

typedef ULONG64 uint64_t;
typedef UINT32 uint32_t;
#define UINT64_C(c) c ## ULL

// original documentation by Vigna:
/* This is a fixed-increment version of Java 8's SplittableRandom generator
   See http://dx.doi.org/10.1145/2714064.2660195 and
   http://docs.oracle.com/javase/8/docs/api/java/util/SplittableRandom.html
   It is a very fast generator passing BigCrush, and it can be useful if
   for some reason you absolutely want 64 bits of state; otherwise, we
   rather suggest to use a xoroshiro128+ (for moderately parallel
   computations) or xorshift1024* (for massively parallel computations)
   generator. */

   // state for splitmix64
uint64_t splitmix64_x; /* The state can be seeded with any value. */

// call this one before calling splitmix64
static inline void splitmix64_seed(uint64_t seed) { splitmix64_x = seed; }

// returns random number, modifies splitmix64_x
// compared with D. Lemire against
// http://grepcode.com/file/repository.grepcode.com/java/root/jdk/openjdk/8-b132/java/util/SplittableRandom.java#SplittableRandom.0gamma
static inline uint64_t splitmix64(void) {
    uint64_t z = (splitmix64_x += UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}

// returns the 32 least significant bits of a call to splitmix64
// this is a simple function call followed by a cast
static inline uint32_t splitmix64_cast32(void) {
    return (uint32_t)splitmix64();
}

// same as splitmix64, but does not change the state, designed by D. Lemire
static inline uint64_t splitmix64_stateless(uint64_t index) {
    uint64_t z = (index + UINT64_C(0x9E3779B97F4A7C15));
    z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
    z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
    return z ^ (z >> 31);
}


struct pcg_state_setseq_64 {
    uint64_t
        state;    // RNG state.  All values are possible.  Will change over time.
    uint64_t inc; // Controls which RNG sequence (stream) is
    // selected. Must *always* be odd. Might be changed by the user, never by our
    // code.
};

typedef struct pcg_state_setseq_64 pcg32_random_t;

static pcg32_random_t pcg32_global; // global state

// call this once before calling pcg32_random_r
static inline void pcg32_seed(uint64_t seed) {
    pcg32_global.state = splitmix64_stateless(seed);
    // we pick a sequence at random
    pcg32_global.inc =
        (splitmix64_stateless(seed + 1)) | 1; // making sure it is odd
}

static inline uint32_t pcg32_random_r(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * UINT64_C(0x5851f42d4c957f2d) + rng->inc;
    uint32_t xorshifted = (uint32_t)((oldstate >> 18) ^ oldstate) >> 27;
    uint32_t rot = oldstate >> 59;
    return (xorshifted >> rot) | (xorshifted << (32 - rot));
}

static inline uint32_t pcg32(void) { return pcg32_random_r(&pcg32_global); }

#endif // PCG32_H