#ifndef SNOWAPRIL_OBFUSCATOR_HPP
#define SNOWAPRIL_OBFUSCATOR_HPP

// Compile-time string obfuscator.
//
// A string literal wrapped in OBFUSCATE() is stored in the binary as ciphertext
// (an affine cipher over the 7-bit code space) and only ever turned back into
// plaintext at run time, which keeps readable strings out of the object file.
//
// Single source, no configuration: the headers compile cleanly as C++11 through
// C++26. The only version-dependent piece is the std::index_sequence support,
// which is polyfilled below for pre-C++14 toolchains.

#include <cstddef>   // std::size_t
#include <utility>   // std::index_sequence / std::make_index_sequence (C++14+)

#include "meta_random.hpp"

namespace snowapril {
namespace detail {

// --- index_sequence support --------------------------------------------------
#if defined(__cpp_lib_integer_sequence) || __cplusplus >= 201402L
using std::index_sequence;
using std::make_index_sequence;
#else
// Minimal C++11 polyfill.
template <std::size_t... I>
struct index_sequence {};

template <std::size_t N, std::size_t... I>
struct make_index_sequence_impl : make_index_sequence_impl<N - 1, N - 1, I...> {};

template <std::size_t... I>
struct make_index_sequence_impl<0, I...> {
    using type = index_sequence<I...>;
};

template <std::size_t N>
using make_index_sequence = typename make_index_sequence_impl<N>::type;
#endif

} // namespace detail

// --- modular arithmetic helpers (constexpr, C++11-compatible) ----------------

constexpr int positive_modulo(int value, int modulus) {
    return (value % modulus + modulus) % modulus;
}

// Modular inverse of `a` modulo `m` via the iterative extended Euclidean
// algorithm expressed as tail recursion. Returns t with (a * t) % m == 1; t may
// be negative, so callers normalise the final result with positive_modulo.
constexpr int mod_inverse_step(int t, int new_t, int r, int new_r) {
    return new_r == 0
        ? t
        : mod_inverse_step(new_t, t - (r / new_r) * new_t,
                           new_r, r - (r / new_r) * new_r);
}

constexpr int mod_inverse(int a, int m) {
    return mod_inverse_step(0, 1, m, a);
}

// --- multipliers: primes coprime to 127, hence invertible mod 127 ------------

constexpr int PrimeNumbers[30] = {
    2,  3,  5,  7,  11,  13,  17,  19,  23,  29,
    31, 37, 41, 43, 47,  53,  59,  61,  67,  71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113
};

// --- the obfuscated string ---------------------------------------------------
//
// Affine cipher over the prime modulus 127: E(c) = (A * c + B) mod 127, with A
// drawn from PrimeNumbers (so gcd(A, 127) == 1 and A is invertible) and B a
// per-call-site random offset. Printable 7-bit ASCII only.

template <typename Indices, int A, int B>
class MetaString;

template <std::size_t... I, int A, int B>
class MetaString<detail::index_sequence<I...>, A, B> {
public:
    constexpr explicit MetaString(const char* str)
        : encrypted_{ encrypt(str[I])... } {}

    // Decrypt in place and return the plaintext. Deliberately NOT constexpr: the
    // decryption must run at execution time so the plaintext is never present in
    // the compiled output. Reading the `volatile` ciphertext (see below) forbids
    // the optimiser from folding the encrypt/decrypt round trip and baking the
    // plaintext straight back into the binary, which an aggressive -O3 pass will
    // otherwise do.
    const char* decrypt() {
        for (std::size_t i = 0; i < sizeof...(I); ++i) {
            buffer_[i] = decrypt(encrypted_[i]);
        }
        buffer_[sizeof...(I)] = '\0';
        return buffer_;
    }

private:
    static constexpr char encrypt(char c) {
        return static_cast<char>((A * static_cast<int>(c) + B) % 127);
    }

    static constexpr char decrypt(char c) {
        return static_cast<char>(
            positive_modulo(mod_inverse(A, 127) * (static_cast<int>(c) - B), 127));
    }

    // `volatile` keeps the decryption a run-time event; the `+ 1` both reserves
    // the null terminator for buffer_ and keeps encrypted_ non-empty for the
    // degenerate OBFUSCATE("") case (zero-length arrays are not standard C++).
    char          buffer_[sizeof...(I) + 1] {};
    volatile char encrypted_[sizeof...(I) + 1] {};
};

} // namespace snowapril

// Wrap a string literal so it is stored obfuscated and decrypted on demand.
// The two __COUNTER__ reads yield an independent (A, B) key per call site.
#define OBFUSCATE(str)                                                           \
    (snowapril::MetaString<                                                      \
         snowapril::detail::make_index_sequence<sizeof(str) - 1>,                \
         snowapril::PrimeNumbers[snowapril::MetaRandom<__COUNTER__, 30>::value],  \
         static_cast<int>(snowapril::MetaRandom<__COUNTER__, 126>::value)>(str)   \
         .decrypt())

#endif // SNOWAPRIL_OBFUSCATOR_HPP
