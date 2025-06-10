#pragma once
#include "Traits/TypeTraitsExt.hpp"

// re-implement dynamic bitset
namespace punk
{
    template <typename Block = uint64_t, typename Allocator = std::allocator<Block>>
    class dynamic_bitset
    {
        static_assert(std::conjunction_v<std::negation<is_bool<Block>>, std::is_unsigned<Block>>);
    public:
        // export types
        using block_type = Block;
        using allocator_type = Allocator;
        using storage_type = std::vector<block_type, allocator_type>;
        using block_width_type = typename storage_type::size_type;
        using size_type = size_t;
        // export constant
        static constexpr block_width_type bits_per_block = static_cast<block_width_type>(std::numeric_limits<block_type>::digits);
        static constexpr size_type npos = (std::numeric_limits<size_type>::max)();
        static constexpr block_type ones = (std::numeric_limits<block_type>::max)();
        static constexpr block_type zeros = 0;

    private:
        storage_type storage_;
        size_type num_bits_;

    public:
        // default constructor
        dynamic_bitset() noexcept;

        // default construct by allocator
        explicit dynamic_bitset(allocator_type const& alloc) noexcept;

        // construct with initial size
        dynamic_bitset(size_type num_bites, bool set, allocator_type const& alloc = allocator_type{});

        // construct with unsigned integer
        explicit dynamic_bitset(uint64_t val, allocator_type const& alloc = allocator_type{});

        // construct from string or string_view
        template <typename StrType, typename SizeType = typename StrType::size_type, typename CharType = typename StrType::value_type>
        explicit dynamic_bitset(
            SizeType pos = 0,
            SizeType n = StrType::npos,
            CharType zero = dynamic_bitset::zero<CharType>(),
            CharType on = dynamic_bitset::one<CharType>(),
            SizeType num_bites = npos,
            allocator_type const& alloc = allocator_type{});

        // construct from c-style string
        template <typename CharType>
        explicit dynamic_bitset(
            CharType const* str,
            typename std::basic_string_view<CharType>::size_type pos = 0,
            typename std::basic_string_view<CharType>::size_type n = std::basic_string_view<CharType>::npos,
            CharType zero = dynamic_bitset::zero<CharType>(),
            CharType one = dynamic_bitset::one<CharType>(),
            size_type num_bites = npos,
            allocator_type const& alloc = allocator_type{});

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
            reference(block_type& block, block_width_type pos) noexcept;
            void operator&() = delete;

        private:
            block_type& block_;
            block_type const mask_;

        public:
            operator bool() const noexcept;
            bool operator~() const noexcept;
            reference& flip() noexcept;
            reference& operator=(bool x) noexcept;
            reference& operator=(reference const& rhs) noexcept;
            reference& operator&= (bool x) noexcept;
            reference& operator|= (bool x) noexcept;
            reference& operator^= (bool x) noexcept;
            reference& operator-= (bool x) noexcept;

        private:
            void set() noexcept;
            void reset() noexcept;
            void flip_impl() noexcept;
            void assign(bool x) noexcept;
        };

        // bool as const_reference type
        using const_reference = bool;

    public: // member access
        reference operator[](size_type pos);
        const_reference operator[](size_type pos) const;

        bool test(size_type pos) const;
        bool all() const noexcept;
        bool any() const noexcept;
        bool none() const noexcept;
        size_type count() const noexcept;
        block_type* data() noexcept;
        block_type const* data() const noexcept;

    public: // capacity
        void clear() noexcept;
        size_type size() const noexcept;
        bool empty() const noexcept;
        void resize(size_type num_bites, bool value = false);
        void swap(dynamic_bitset& other) noexcept;
        void push_back(bool value);
        void pop_back();
        void append(block_type block);

    public: // modifiers
        dynamic_bitset& operator&=(dynamic_bitset const& other);
        dynamic_bitset& operator|=(dynamic_bitset const& other);
        dynamic_bitset& operator^=(dynamic_bitset const& other);
        dynamic_bitset operator~() const;
        dynamic_bitset& operator<<=(size_type n);
        dynamic_bitset& operator>>=(size_type n);
        dynamic_bitset operator<<=(size_type n) const;
        dynamic_bitset operator>>=(size_type n) const;

        dynamic_bitset& set() noexcept;
        dynamic_bitset& set(size_type pos, bool value = true);
        dynamic_bitset& set(size_type pos, size_type len, bool value);
        dynamic_bitset& reset() noexcept;
        dynamic_bitset& reset(size_type pos);
        dynamic_bitset& reset(size_type pos, size_type len);
        dynamic_bitset& flip() noexcept;
        dynamic_bitset& flip(size_type pos);
        dynamic_bitset& flip(size_type pos, size_type len);

    public: // conversions
        template <typename StrType, typename CharType = typename StrType::value_type>
        void to_string(StrType& str, CharType zero = dynamic_bitset::zero<CharType>(), CharType one = dynamic_bitset::one<CharType>()) const;
        uint32_t to_uint32() const;
        uint64_t to_uint64() const;

    private: // implementations
        static constexpr size_type calc_num_blocks(size_type num_bites) noexcept;
        static storage_type create_storage_from(uint64_t val, size_type valid_bites, allocator_type const& alloc);
        static constexpr int calc_valid_bits() noexcept;
        template <typename Char>
        static constexpr Char zero() noexcept;
        template <typename Char>
        static constexpr Char one() noexcept;
        template <typename StrType, typename CharType = typename StrType::value_type>
        void init_from_string(
            StrType const& str,
            typename StrType::size_type pos,
            typename StrType::size_type n,
            CharType zero,
            CharType one,
            size_type num_bites,
            allocator_type const& alloc);
        static constexpr size_type block_index(size_type pos) noexcept;
        static constexpr block_width_type bit_index(size_type pos) noexcept;
        static constexpr block_type bit_mask(size_type pos) noexcept;
        static constexpr block_type bit_mask(size_type begin, size_type end) noexcept;
        template <typename UnsignedIntegral>
        UnsignedIntegral to_unsigned_integral() const;

    public: // friends
        template <typename Block1, typename Allocator1>
        friend bool operator==(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs) noexcept;

        template <typename Block1, typename Allocator1>
        friend bool operator!=(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs) noexcept;

        template <typename Block1, typename Allocator1>
        friend dynamic_bitset<Block1, Allocator1> operator&(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs);

        template <typename Block1, typename Allocator1>
        friend dynamic_bitset<Block1, Allocator1> operator|(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs);

        template <typename Block1, typename Allocator1>
        friend dynamic_bitset<Block1, Allocator1> operator^(dynamic_bitset<Block1, Allocator1> const& lhs, dynamic_bitset<Block1, Allocator1> const& rhs);

        template <typename Block1, typename Allocator1>
        void swap(dynamic_bitset<Block1, Allocator1>& lhs, dynamic_bitset<Block1, Allocator1>& rhs)
        {
            lhs.swap(rhs);
        }
    };
}