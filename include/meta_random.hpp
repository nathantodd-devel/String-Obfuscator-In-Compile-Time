#ifndef SNOWAPRIL_META_RANDOM_HPP
#define SNOWAPRIL_META_RANDOM_HPP

// Compile-time pseudo-random number generation.
//
// Everything here is expressed as tail-recursive `constexpr` so that it is a
// valid constant expression all the way back to C++11 (where `constexpr`
// functions may contain only a single return statement, no loops or mutable
// locals) and remains well-formed through C++26.

namespace snowapril {

// Pseudo-random seed derived from the compile time (__TIME__ == "HH:MM:SS").
// The per-digit weights are chosen so the '0' ASCII baseline cancels out,
// leaving the wall-clock second-of-day in [0, 86399].
constexpr unsigned RandomSeed() {
    return static_cast<unsigned>(
        '0'         * -40271 + // offsets accounting for the digits' ASCII value
        __TIME__[7] *      1 +
        __TIME__[6] *     10 +
        __TIME__[4] *     60 +
        __TIME__[3] *    600 +
        __TIME__[1] *   3600 +
        __TIME__[0] *  36000);
}

// Linear congruential generator unrolled as tail recursion. Reproduces the
// original recurrence: v(0) = (a * seed) % limit,
//                      v(k) = (a * v(k-1) + k) % limit, returning v(n).
constexpr unsigned LcgStep(unsigned a, unsigned n, unsigned limit,
                           unsigned k, unsigned v) {
    return k > n ? v
                 : LcgStep(a, n, limit, k + 1, (a * v + k) % limit);
}

constexpr unsigned Lcg(unsigned a, unsigned n, unsigned seed, unsigned limit) {
    return LcgStep(a, n, limit, 1u, (a * seed) % limit);
}

// Compile-time random value in [0, Limit). Use a distinct N (e.g. __COUNTER__)
// per call site to get an independent value each time.
template <unsigned N, unsigned Limit>
struct MetaRandom {
    static constexpr unsigned value = Lcg(16807u, N, RandomSeed(), Limit);
};

} // namespace snowapril

#endif // SNOWAPRIL_META_RANDOM_HPP
