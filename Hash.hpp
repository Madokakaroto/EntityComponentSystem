#pragma once

namespace ecs
{
    inline constexpr uint32_t murmur_rotl(uint32_t x, int8_t r) noexcept
    {
        return (x << r) | (x >> (32 - r));
    }

    inline constexpr uint32_t murmur_get_block(char const* p, int i)
    {
        uint32_t block = 
		static_cast<uint32_t>(p[0+i*4]) << 0  | 
		static_cast<uint32_t>(p[1+i*4]) << 8  | 
		static_cast<uint32_t>(p[2+i*4]) << 16 | 
		static_cast<uint32_t>(p[3+i*4]) << 24;
	    return block;
    }

    inline constexpr uint32_t murmur_fmix(uint32_t const h) noexcept
    {
        uint32_t result = h;
        result ^= result >> 16;
        result *= 0x85ebca6b;
        result ^= result >> 13;
        result *= 0xc2b2ae35;
        result ^= result >> 16;
        return result;
    }

    inline constexpr uint32_t murmurhash3_x86_32_impl(char const* key, int const len, uint32_t const seed)
    {
        int const nblocks = len / 4;

        uint32_t h1 = seed;

        constexpr uint32_t c1 = 0xcc9e2d51;
        constexpr uint32_t c2 = 0x1b873593;

        //----------
        // body

        for(int i = 0; i < nblocks; ++i)
        {
            uint32_t k1 = murmur_get_block(key, i);

            k1 *= c1;
            k1 = murmur_rotl(k1, 15);
            k1 *= c2;

            h1 ^= k1;
            h1 = murmur_rotl(h1, 13);
            h1 = h1 * 5 + 0xe6546b64;
        }

        //----------
        // tail

        uint32_t k1 = 0u;
        int const index = nblocks * 4;
        switch(len & 3)
        {
        case 3: k1 ^= key[index + 2] << 16;
        case 2: k1 ^= key[index + 1] << 8;
        case 1: k1 ^= key[index];
                k1 *= c1; k1 = murmur_rotl(k1, 15); k1 *= c2; h1 ^= k1;
        }

        //----------
        // finalization

        h1 ^= len;

        h1 = murmur_fmix(h1);

        return h1;
    }

    template <size_t Length>
    inline constexpr uint32_t static_murmur_hash_x86_32(char const(&arr)[Length], uint32_t const seed)
    {
        return murmurhash3_x86_32_impl(arr, Length - 1, seed);
    }

    inline uint32_t dynamic_murmur_hash_x86_32(char const* arr, int const len, uint32_t const seed)
    {
        return murmurhash3_x86_32_impl(arr, len, seed);
    }
}