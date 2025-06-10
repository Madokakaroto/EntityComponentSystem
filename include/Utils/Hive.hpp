#pragma once

#include "Types/Forward.hpp"
#include "Utils/DynamicBitset.hpp"

namespace punk
{
    template <typename T>
    class hive_group
    {
    public:
        template <typename U>
        friend class hive;
        using value_type = T;
        using const_value = std::add_const_t<value_type>;
        using pointer = std::add_pointer_t<value_type>;
        using const_pointer = std::add_pointer_t<const_value>;
        static constexpr size_t value_size = sizeof(value_type);
        static constexpr size_t value_align = alignof(value_type);
        static_assert(value_size >= sizeof(uint32_t));

    private:
        std::vector<value_type>     storage_;
        std::vector<bool>           storage_bits_;                  // TODO ... dynamic bitset
        uint32_t                    first_available_index_;
        uint32_t                    available_element_count_;
        hive_group*                 next_group_;
        hive_group*                 prev_group_;

    public:
        explicit hive_group(size_t element_count)
            : storage_(element_count, 0)
            , storage_bits_(element_count, false)
            , first_available_index_(0)
            , available_element_count_(element_count)
            , next_group_(nullptr)
            , prev_group_(nullptr)
        {
            init(element_count);
        }

        pointer get(size_t index)
        {
            return const_cast<pointer>(const_cast<hive_group const*>(this)->get(index));
        }

        const_pointer get(size_t index) const
        {
            assert(index < storage_.size());
            if(index < storage_.size() && test(index))
            {
                return get_ptr_as<value_type>(index);
            }
            return nullptr;
        }

        size_t size() const noexcept
        {
            return capacity() - available_element_count_;
        }

        size_t capacity() const noexcept
        {
            return storage_.size();
        }

        bool test(size_t index) const
        {
            return storage_bits_.at(index);
        }

        bool memory_in_range(const_pointer ptr) const noexcept
        {
            return ptr >= get_ptr(0) && ptr <= get_ptr(storage_.size() - 1);
        }

        bool memory_aligned(const_pointer ptr) const noexcept
        {
            std::ptrdiff_t distance = get_ptr(0) - ptr;
            return distance % value_size == 0;
        }

        bool hash_available_space() const noexcept
        {
            return available_element_count_ > 0;
        }

    private:
        void init(size_t element_count)
        {
            for(uint32_t loop = 0; loop < element_count; ++loop)
            {
                auto* uint32_ptr = get_ptr_as<uint32_t>(loop);
                *uint32_ptr = loop + 1;
            }
        }

        pointer get_ptr(size_t index)
        {
            return &storage_[index];
        }

        const_pointer get_ptr(size_t index) const
        {
            return &storage_[index];
        }

        template <typename U>
        U* get_ptr_as(size_t index)
        {
            return reinterpret_cast<U*>(get_ptr(index));
        }

        template <typename U>
        U const* get_ptr_as(size_t index) const
        {
            return reinterpret_cast<U const*>(get_ptr(index));
        }
    };

    template <typename T>
    class hive final
    {
    public:
        using hive_group_type = hive_group<T>;
        using value_type = typename hive_group_type::value_type;
        using pointer = typename hive_group_type::pointer;
        using const_pointer = typename hive_group_type::const_pointer;

    private:
        std::vector<std::unique_ptr<hive_group_type>> hive_groups_;

    public:
        hive() = default;
        ~hive() = default;

    };
}