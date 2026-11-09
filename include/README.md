# How it works

### Affine cipher

Each character `c` of the string literal is encrypted at compile time with
`E(c) = (A * c + B) mod 127` and decrypted at run time with
`D(x) = A⁻¹ * (x − B) mod 127`, where `A⁻¹` is the modular inverse of `A`
modulo the prime `127`.

- `A` is chosen from `PrimeNumbers` (each is coprime to `127`, so `A` is always
  invertible).
- `B` is a per-call-site random offset.
- `A⁻¹` is computed at compile time with a tail-recursive `constexpr` extended
  Euclidean algorithm (`mod_inverse`).

### Linear congruential random generator

`A` and `B` come from a `constexpr` linear congruential generator seeded from
`__TIME__` (so the keys change between builds) and advanced by `__COUNTER__`
(so every call site gets an independent key). See `meta_random.hpp`.
