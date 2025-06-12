#pragma once
#include "Traits/TypeTraitsExt.hpp"
#include <assert.h>
#include <algorithm>

// re-implement dynamic bitset
namespace punk
{
    template <typename Block = uint64_t, typename Allocator = std::allocator<Block>>
    class dynamic_bitset
    {
        static_assert(std::conjunction_v<std::negation<is_bool<Block>>, std::is_unsigned<Block>>);

    public: // export types
        using block_type = Block;
        using allocator_type = Allocator;
        using storage_type = std::vector<block_type, allocator_type>;
        using block_width_type = typename storage_type::size_type;
        using size_type = size_t;

    public: // export constant
        static constexpr block_width_type bits_per_block = static_cast<block_width_type>(std::numeric_limits<block_type>::digits);
        static constexpr size_type npos = (std::numeric_limits<size_type>::max)();
        static constexpr block_type ones = (std::numeric_limits<block_type>::max)();
        static constexpr block_type zeros = 0;

    public: // static functions
        template <typename CharType>
        static constexpr CharType zero() noexcept
        {
            if constexpr(std::is_same_v<CharType, char>)
            {
                return '0';
            }
            else
            {
                static_assert(std::is_same_v<CharType, wchar_t>);
                return L'0';
            }
        }

        template <typename CharType>
        static constexpr CharType one() noexcept
        {
            if constexpr(std::is_same_v<CharType, char>)
            {
                return '1';
            }
            else
            {
                static_assert(std::is_same_v<CharType, wchar_t>);
                return L'1';
            }
        }

        static constexpr size_type block_index(size_type pos) noexcept
        {
            return pos / bits_per_block;
        }

        static constexpr block_width_type bit_index(size_type pos) noexcept
        {
            return static_cast<block_width_type>(pos % bits_per_block);
        }

        static constexpr block_type bit_mask(size_type pos) noexcept
        {
            return block_type{ 1 } >> bit_index(pos);
        }

        static constexpr block_type bit_mask(size_type begin, size_type end) noexcept
        {
            auto mask = end == bits_per_block - 1 ? ones :
                (block_type{ 1 } << (end + 1)) - 1;
        }

        static constexpr size_type calc_num_blocks(size_type num_bits) noexcept
        {
            auto [q, r] = ldiv(num_bits, bits_per_block);
            auto const yet_another_block = q == 0 || r != 0;
            return yet_another_block ? q + 1 : q;
        }

        static storage_type create_storage_from(uint64_t val, size_type valid_bits, allocator_type const& alloc)
        {
            if(val == 0)
            {
                return { 1, zeros, alloc };
            }

            assert(valid_bits == calc_valid_bits(val));
            auto const num_block = calc_num_blocks(valid_bits);
            assert(num_block * sizeof(block_type) <= sizeof(uint64_t));
            storage_type result{ num_block, zeros, alloc };
            std::memcpy(result.data(), &val, num_block * sizeof(block_type));
            return result;
        }

        static constexpr size_type calc_valid_bits(uint64_t val) noexcept
        {
            return std::numeric_limits<uint64_t>::digits - std::countl_zero(val);
        }

    private:
        size_type       num_bits_;
        storage_type    storage_;

    public:
        // default constructor
        dynamic_bitset() noexcept
            : num_bits_(0)
        {
        }

        // default construct by allocator
        explicit dynamic_bitset(allocator_type const& alloc) noexcept
            : num_bits_(0)
            , storage_(alloc)
        {
        }

        // construct with initial size
        dynamic_bitset(size_type num_bits, bool set, allocator_type const& alloc = allocator_type{})
            : num_bits_(num_bits)
            , storage_(calc_num_blocks(num_bits), set ? ones : zeros, alloc)
        {
        }

        // construct with unsigned integer
        explicit dynamic_bitset(uint64_t val, allocator_type const& alloc = allocator_type{})
            : num_bits_(calc_valid_bits(val))
            , storage_(create_storage_from(val, num_bits_, alloc))
        {
        }

        // construct from string or string_view
        template <typename StrType, typename SizeType = typename StrType::size_type, typename CharType = typename StrType::value_type>
        explicit dynamic_bitset(
            StrType const& str,
            SizeType pos = 0,
            SizeType n = StrType::npos,
            CharType zero = dynamic_bitset::zero<CharType>(),
            CharType one = dynamic_bitset::one<CharType>(),
            SizeType num_bits = npos,
            allocator_type const& alloc = allocator_type{})
        {
            init_from_string(str, pos, n, zero, one, num_bits, alloc);
        }

        // construct from c-style string
        template <typename CharType>
        explicit dynamic_bitset(
            CharType const* str,
            typename std::basic_string_view<CharType>::size_type pos = 0,
            typename std::basic_string_view<CharType>::size_type n = std::basic_string_view<CharType>::npos,
            CharType zero = dynamic_bitset::zero<CharType>(),
            CharType one = dynamic_bitset::one<CharType>(),
            size_type num_bits = npos,
            allocator_type const& alloc = allocator_type{})
        {
            init_from_string(std::basic_string_view<CharType>{ str }, pos, n, zero, one, num_bits, alloc);
        }

        ~dynamic_bitset() noexcept = default;
        dynamic_bitset(dynamic_bitset const&) = default;
        dynamic_bitset& operator=(dynamic_bitset const&) = default;
        dynamic_bitset(dynamic_bitset&&) = default;
        dynamic_bitset& operator=(dynamic_bitset&&) = default;

    public:
        // block & block mask as reference type
        class reference
        {
            friend class dynamic_bitset<Block, Allocator>;
            reference(block_type& block, block_width_type pos) noexcept
                : block_(block)
                , mask_(block_type{ 1 } << pos)
            {
            }

        private:
            block_type& block_;
            block_type const mask_;

        public:
            void operator&() = delete;
            operator bool() const noexcept
            {
                return (block_ & mask_) != 0;
            }

            bool operator~() const noexcept
            {
                return (block_ & mask_) == 0;
            }

            reference& flip() noexcept
            {
                flip_impl();
                return *this;
            }

            reference& operator=(bool x) noexcept
            {
                assign(x);
                return *this;
            }

            reference& operator&= (bool x) noexcept
            {
                if(!x)
                {
                    set();
                }
                return *this;
            }

            reference& operator|= (bool x) noexcept
            {
                if(x)
                {
                    set();
                }
                return *this;
            }

            reference& operator^= (bool x) noexcept
            {
                if(x)
                {
                    flip_impl();
                }
                return *this;
            }

            reference& operator-= (bool x) noexcept
            {
                if(x)
                {
                    reset();
                }
                return *this;
            }

        private:
            void set() noexcept
            {
                block_ |= mask_;
            }

            void reset() noexcept
            {
                block_ &= mask_;
            }

            void flip_impl() noexcept
            {
                block_ ^= mask_;
            }

            void assign(bool x) noexcept
            {
                x ? set() : reset();
            }
        };

        // bool as const_reference type
        using const_reference = bool;

    public: // member access
        reference operator[](size_type pos)
        {
            if(pos >= num_bits_)
            {
                throw std::out_of_range{ "access out of range." };
            }

            auto const block_idx = bit_index(pos);
            auto const bit_idx = bit_index(pos);
            return reference{ storage_[block_idx], bit_idx };
        }

        const_reference operator[](size_type pos) const
        {
            return test(pos);
        }

        bool test(size_type pos) const
        {
            if(pos >= num_bits_)
            {
                throw std::out_of_range{ "access out of range." };
            }
            return test_impl(pos);
        }

        bool all() const noexcept
        {
            if(empty())
            {
                return true;
            }

            auto const size = storage_.size() - 1;
            for(block_width_type loop = 0; loop < size; ++loop)
            {
                if(storage_[loop] != ones)
                {
                    return false;
                }
            }

            auto const extra_bit_idx = bit_index(num_bits_);
            if(extra_bit_idx > 0)
            {
                auto const mask = (block_type{ 1 } << extra_bit_idx) - 1;
                return (storage_.back() & mask) == mask;
            }
            return storage_.back() == ones;
        }

        bool any() const noexcept
        {
            return std::ranges::any_of(storage_, [](auto const elem) { return elem != zeros; });
        }

        bool none() const noexcept
        {
            return !any();
        }

        size_type count() const noexcept
        {
            if(empty())
            {
                return 0;
            }

            auto result = std::reduce(storage_.cbegin(), storage_.cend() - 1, size_type{ 0 },
                [](size_type acc, block_type elem)
                {
                    return acc + static_cast<size_type>(std::popcount(elem));
                });

            auto const bit_idx = bit_index(num_bits_);
            if(bit_idx > 0)
            {
                auto const mask = (block_type{ 1 } << bit_idx) - 1;
                result += static_cast<size_type>(std::popcount(storage_.back() & mask));
            }
            else
            {
                result += static_cast<size_type>(std::popcount(storage_.back()));
            }
            return result;
        }

        size_type find_first() const noexcept
        {
            return size_type{ 0 };
        }

        size_type find_next(size_type pos) const noexcept
        {
            return size_type{ 0 };
        }

        block_type* data() noexcept
        {
            return storage_.data();
        }

        block_type const* data() const noexcept
        {
            return storage_.data();
        }

    public: // capacity
        void clear() noexcept
        {
            storage_.clear();
            num_bits_ = 0;
        }

        size_type size() const noexcept
        {
            return num_bits_;
        }

        bool empty() const noexcept
        {
            return num_bits_ == 0;
        }

        void resize(size_type num_bits, bool value = false)
        {
            auto const block_count = block_size();
            auto const new_block_count = calc_num_blocks(num_bits);
            if(block_count != new_block_count)
            {
                storage_.resize(new_block_count, value ? ones : zeros);
            }

            auto const bit_idx = bit_index(num_bits_);
            if(num_bits > num_bits_ && bit_idx > 0)
            {
                auto const mask = (block_type{ 1 } << bit_idx) - 1;
                if(value)
                {
                    storage_[block_count - 1] |= ~mask;
                }
                else
                {
                    storage_[block_count - 1] &= mask;
                }
            }
            num_bits_ = num_bits;
        }

        void swap(dynamic_bitset& other) noexcept
        {
            if(&other != this)
            {
                std::swap(num_bits_, other.num_bits_);
                std::swap(storage_, other.storage_);
            }
        }

        void push_back(bool value)
        {
            resize(num_bits_ + 1, value);
        }

        void pop_back()
        {
            resize(num_bits_ - 1);
        }

        void append(block_type block)
        {
            auto const bit_idx = bit_index(num_bits_);
            if(bit_idx == 0)
            {
                storage_.push_back(block);
            }
            else
            {
                auto const mask = (block_type{ 1 } << bit_idx) - 1;
                storage_.back() = (storage_.back() & mask) | (block << bit_idx);
                storage_.push_back(block >> (bits_per_block - bit_idx));
            }
            num_bits_ += bits_per_block;
        }

        block_width_type block_size() const noexcept
        {
            return storage_.size();
        }

        bool back_swap_erase(size_type pos)
        {
            if(pos >= num_bits_)
            {
                return false;
            }

            set(pos, test(num_bits_ - 1));
            resize(num_bits_ - 1);
            return true;
        }

    public: // modifiers
        dynamic_bitset& operator&=(dynamic_bitset const& other)
        {
            // TODO ... support different size
            if(&other != this)
            {
                assert(other.size() == size());
                for(block_width_type loop = 0; loop < other.block_size(); ++loop)
                {
                    storage_[loop] &= other.storage_[loop];
                }
            }
            return *this;
        }

        dynamic_bitset& operator|=(dynamic_bitset const& other)
        {
            // TODO ... support different size
            if(&other != this)
            {
                assert(other.size() == size());
                for(block_width_type loop = 0; loop < other.block_size(); ++loop)
                {
                    storage_[loop] |= other.storage_[loop];
                }
            }
            return *this;
        }

        dynamic_bitset& operator^=(dynamic_bitset const& other)
        {
            // TODO ... support different size
            if(&other != this)
            {
                assert(other.size() == size());
                for(block_width_type loop = 0; loop < other.block_size(); ++loop)
                {
                    storage_[loop] ^= other.storage_[loop];
                }
            }
            return *this;
        }

        dynamic_bitset operator~() const
        {
            dynamic_bitset result{ *this };
            return result.flip();
        }

        dynamic_bitset& operator<<=(size_type n)
        {
            // TODO ...
            return *this;
        }

        dynamic_bitset& operator>>=(size_type n)
        {
            // TODO ...
            return *this;
        }

        dynamic_bitset operator<<(size_type n) const
        {
            dynamic_bitset result{ *this };
            return result <<= n;
        }
        dynamic_bitset operator>>(size_type n) const
        {
            dynamic_bitset result{ *this };
            return result >>= n;
        }

        dynamic_bitset& set() noexcept
        {
            std::ranges::fill_n(storage_, storage_.size(), ones);
            return *this;
        }
        dynamic_bitset& set(size_type pos, bool value = true)
        {
            if(pos >= num_bits_)
            {
                throw std::out_of_range{ "access out of range." };
            }

            auto const block_idx = block_index(pos);
            auto const bit_msk = bit_mask(pos);
            if(value)
            {
                storage_[block_idx] |= bit_msk;
            }
            else
            {
                storage_[block_idx] &= ~bit_msk;
            }
            return *this;
        }
        dynamic_bitset& set(size_type pos, size_type len, bool value)
        {
            // TODO ...
            return *this;
        }

        /// reset
        dynamic_bitset& reset() noexcept
        {
            std::ranges::fill_n(storage_, storage_.size(), zeros);
            return *this;
        }
        dynamic_bitset& reset(size_type pos)
        {
            if(pos >= num_bits_)
            {
                throw std::out_of_range{ "access out of range." };
            }
            storage_[block_index(pos)] &= ~bit_mask(pos);
            return *this;
        }
        dynamic_bitset& reset(size_type pos, size_type len)
        {
            // TODO ...
            return *this;
        }

        /// flip
        dynamic_bitset& flip() noexcept
        {
            for(auto& elem : storage_)
            {
                elem = ~elem;
            }
            return *this;
        }
        dynamic_bitset& flip(size_type pos)
        {
            if(pos >= num_bits_)
            {
                throw std::out_of_range{ "access out of range." };
            }
            storage_[block_index(pos)] ^= bit_mask(pos);
            return *this;
        }
        dynamic_bitset& flip(size_type pos, size_type len)
        {
            // TODO ...
            return *this;
        }

    public: // conversions
        template <typename StrType, typename CharType = typename StrType::value_type>
        void to_string(StrType& str, CharType zero = dynamic_bitset::zero<CharType>(), CharType one = dynamic_bitset::one<CharType>()) const
        {
            static_assert(std::negation_v<std::is_const<StrType>>);
            using char_traits_t = typename StrType::traits_type;

            str.resize(num_bits_);
            for(size_type loop = 0; loop < num_bits_; ++loop)
            {
                auto const c = test_impl(loop) ? one : zero;
                char_traits_t::assign(str[num_bits_ - 1 - loop], c);
            }
        }

        uint32_t to_uint32() const
        {
            return to_unsigned_integral<uint32_t>();
        }

        uint64_t to_uint64() const
        {
            return to_unsigned_integral<uint64_t>();
        }

    private: // implementations
        template <typename StrType, typename CharType = typename StrType::value_type>
        void init_from_string(
            StrType const& str,
            typename StrType::size_type pos,
            typename StrType::size_type n,
            CharType zero,
            CharType one,
            size_type num_bits,
            allocator_type const& alloc)
        {
            using char_traits_t = typename StrType::traits_type;

            auto const length = str.length();
            if(pos > length)
            {
                throw std::out_of_range{ "access out of range." };
            }

            if(n != StrType::npos && n > pos)
            {
                throw std::invalid_argument{ "invalid argument for the 3rd parameter n." };
            }

            auto const str_len = (std::min)(length - pos, n);
            auto const size = num_bits != npos ? num_bits : str_len;
            storage_.resize(calc_num_blocks(size));
            num_bits_ = num_bits;

            size_type const end = num_bits < str_len ? num_bits : str_len;
            for(size_type loop = 0; loop < end; ++loop)
            {
                CharType const c = str[pos + end - 1 - loop];
                if(char_traits_t::eq(one))
                {
                    set(loop);
                }
                else if(!char_traits_t::eq(zero))
                {
                    throw std::invalid_argument{ "invalid bit string " };
                }
            }
        }
        
        bool test_impl(size_type pos) const
        {
            auto const block_idx = block_index(pos);
            auto const bit_msk = bit_mask(pos);
            return (storage_[block_idx] & bit_msk) != 0;
        }

        template <typename UnsignedIntegral>
        UnsignedIntegral to_unsigned_integral() const
        {
            if(empty())
            {
                return 0;
            }

            auto const block_mem_size = block_size() * sizeof(block_type);
            if(block_mem_size > sizeof(UnsignedIntegral))
            {
                throw std::overflow_error{ "cast overflow from dynamic_bitset to unsigned integer." };
            }

            UnsignedIntegral result = 0;
            auto const end = block_size() - 1;
            for(block_width_type loop = 0; loop < end; ++loop)
            {
                auto const offset = bits_per_block * loop;
                result |= (static_cast<UnsignedIntegral>(storage_[loop]) << offset);
            }

            auto const bit_idx = bit_index(num_bits_);
            auto const offset = end * bits_per_block;
            if(bit_idx > 0)
            {
                auto const mask = (block_type{ 1 } << bit_idx) - 1;
                result |= (static_cast<UnsignedIntegral>(storage_.back()) & mask) << offset;
            }
            else
            {
                result |= static_cast<UnsignedIntegral>(storage_.back()) << offset;
            }
            return result;
        }

    public: // friends
        template <typename Block1, typename Allocator1>
        friend bool operator==(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs) noexcept
        {
            return lhs.num_bits_ == rhs.num_bits_ && std::memcmp(lhs.data(), rhs.data(), lhs.num_bits_ / sizeof(uint8_t));
        }

        template <typename Block1, typename Allocator1>
        friend bool operator!=(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs) noexcept
        {
            return !(lhs == rhs);
        }

        template <typename Block1, typename Allocator1>
        friend dynamic_bitset<Block1, Allocator1> operator&(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs)
        {
            dynamic_bitset<Block1, Allocator1> result{ lhs };
            return result &= rhs;
        }

        template <typename Block1, typename Allocator1>
        friend dynamic_bitset<Block1, Allocator1> operator|(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs)
        {
            dynamic_bitset<Block1, Allocator1> result{ lhs };
            return result |= rhs;
        }

        template <typename Block1, typename Allocator1>
        friend dynamic_bitset<Block1, Allocator1> operator^(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs)
        {
            dynamic_bitset<Block1, Allocator1> result{ lhs };
            return result ^= rhs;
        }

        template <typename Block1, typename Allocator1>
        void swap(dynamic_bitset<Block1, Allocator1>& lhs, dynamic_bitset<Block1, Allocator1>& rhs)
        {
            lhs.swap(rhs);
        }
    };
}