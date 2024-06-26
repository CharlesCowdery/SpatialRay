#include "XorRandom.h"

void XorGen::seed(uint64_t n1, uint64_t n2, uint64_t n3, uint64_t n4) {
    xe_seed[0] = n1;
    xe_seed[1] = n2;
    xe_seed[2] = n3;
    xe_seed[3] = n4;
    uint64_t* avx_accessor;
    for (int i = 0; i < 4; i++) {
        avx_accessor = (uint64_t*) & (xe_seed_AVX[i]);
        for (int j = 0; j < 4; j++) {
            avx_accessor[j] = xe_next();
        }
    }
}
float XorGen::pool_rand(int index) {
    return ((float*)&pool)[index];
}
uint64_t XorGen::xe_next(void) {
    const uint64_t result = xe_seed[0] + xe_seed[3];

    const uint64_t t = xe_seed[1] << 17;

    xe_seed[2] ^= xe_seed[0];
    xe_seed[3] ^= xe_seed[1];
    xe_seed[1] ^= xe_seed[2];
    xe_seed[0] ^= xe_seed[3];

    xe_seed[2] ^= t;

    xe_seed[3] = rotl(xe_seed[3], 45);

    return result;
}
void XorGen::regen_pool() {
    const __m256i result = _mm256_add_epi64(xe_seed_AVX[0], xe_seed_AVX[1]);
    pool = _mm256_sub_ps(
        std::bit_cast<__m256>(_mm256_or_epi32(
            _mm256_and_epi32(
                result,
                float_mask
            ),
            float_exponent
        )),
        _mm256_set1_ps(1.0f)
    );

    const __m256i t;
    uint64_t* t_iter = (uint64_t*)&t;

    t_iter[0] = ((uint64_t*)&xe_seed_AVX[1])[0] << 17;
    t_iter[1] = ((uint64_t*)&xe_seed_AVX[1])[1] << 17;
    t_iter[2] = ((uint64_t*)&xe_seed_AVX[1])[2] << 17;
    t_iter[3] = ((uint64_t*)&xe_seed_AVX[1])[3] << 17;

    xe_seed_AVX[2] = _mm256_xor_si256(xe_seed_AVX[2], xe_seed_AVX[0]);
    xe_seed_AVX[3] = _mm256_xor_si256(xe_seed_AVX[3], xe_seed_AVX[1]);
    xe_seed_AVX[1] = _mm256_xor_si256(xe_seed_AVX[1], xe_seed_AVX[2]);
    xe_seed_AVX[0] = _mm256_xor_si256(xe_seed_AVX[0], xe_seed_AVX[3]);

    xe_seed_AVX[2] = _mm256_xor_si256(xe_seed_AVX[2], t);

    uint64_t* s3_iter = (uint64_t*)&(xe_seed_AVX[3]);

    s3_iter[0] = (s3_iter[0] << 45) | (s3_iter[0] >> (19));
    s3_iter[1] = (s3_iter[1] << 45) | (s3_iter[1] >> (19));
    s3_iter[2] = (s3_iter[2] << 45) | (s3_iter[2] >> (19));
    s3_iter[3] = (s3_iter[3] << 45) | (s3_iter[3] >> (19));
}
void XorGen::xe_jump(void) {
    const uint64_t JUMP[] = { 0x180ec6d33cfd0aba, 0xd5a61266f0c9392c, 0xa9582618e03fc9aa, 0x39abdc4529b1661c };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for (int i = 0; i < sizeof JUMP / sizeof * JUMP; i++)
        for (int b = 0; b < 64; b++) {
            if (JUMP[i] & UINT64_C(1) << b) {
                s0 ^= xe_seed[0];
                s1 ^= xe_seed[1];
                s2 ^= xe_seed[2];
                s3 ^= xe_seed[3];
            }
            xe_next();
        }

    xe_seed[0] = s0;
    xe_seed[1] = s1;
    xe_seed[2] = s2;
    xe_seed[3] = s3;
}

void XorGen::xe_long_jump(void) {
    const uint64_t LONG_JUMP[] = { 0x76e15d3efefdcbbf, 0xc5004e441c522fb3, 0x77710069854ee241, 0x39109bb02acbe635 };

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;
    for (int i = 0; i < sizeof LONG_JUMP / sizeof * LONG_JUMP; i++)
        for (int b = 0; b < 64; b++) {
            if (LONG_JUMP[i] & UINT64_C(1) << b) {
                s0 ^= xe_seed[0];
                s1 ^= xe_seed[1];
                s2 ^= xe_seed[2];
                s3 ^= xe_seed[3];
            }
            xe_next();
        }

    xe_seed[0] = s0;
    xe_seed[1] = s1;
    xe_seed[2] = s2;
    xe_seed[3] = s3;
}
/*Xorshiro256+ pseudorandom end*/

float XorGen::xe_frand() {
    return (xe_next() >> 11) * 0x1.0p-53;
}

float XorGen::rand_frand() //https://stackoverflow.com/a/2704552
{
    return (float)rand() / RAND_MAX;
}

float XorGen::fRand(float fMin, float fMax) //https://stackoverflow.com/a/2704552
{
    float f = xe_frand();
    return fMin + f * (fMax - fMin);
}
inline uint64_t XorGen::rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

