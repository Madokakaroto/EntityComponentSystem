#pragma once

#include "Types/Forward.hpp"
#include "Utils/DynamicBitset.hpp"

namespace punk
{
    template <typename T, typename Alloc = std::allocator<T>>
    class hive_group
    {
    public:
        template <typename U, typename Allocator>
        friend class hive;
        using value_type = T;
        using const_value = std::add_const_t<value_type>;
        using pointer = std::add_pointer_t<value_type>;
        using const_pointer = std::add_pointer_t<const_value>;
        static constexpr size_t value_size = sizeof(value_type);
        static constexpr size_t value_align = alignof(value_type);
        struct element_storage
        {
            alignas(T) std::array<uint8_t, value_size> bytes_;
        };
        using allocator_type = typename std::allocator_traits<Alloc>::template rebind_alloc<element_storage>;
        using storage_type = std::vector<element_storage, allocator_type>;
        static_assert(value_size >= sizeof(uint32_t));
        static_assert(value_size == sizeof(element_storage));

    private:
        storage_type        storage_;
        dynamic_bitset<>    storage_bits_;
        uint32_t            first_available_index_;
        uint32_t            available_element_count_;
        size_t const        first_global_index_;

    public:
        explicit hive_group(size_t element_count, size_t first_global_index)
            : storage_(element_count)
            , storage_bits_(element_count, false)
            , first_available_index_(0)
            , available_element_count_(element_count)
            , first_global_index_(first_global_index)
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
            return storage_bits_.test(index);
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

        bool has_available_space() const noexcept
        {
            return available_element_count_ > 0;
        }

        size_t get_first_global_index() const noexcept
        {
            return first_global_index_;
        }

    public:
        template <typename ... Args> requires(std::constructible_from<value_type, Args&&...>)
        auto construct(Args&& ... args) -> std::pair<pointer*, size_t>
        {
            // branch: when hive group has no available space to construct a new element
            if(!has_available_space())
            {
                return { nullptr, capacity() };
            }

            // query an available space
            auto const index = first_available_index_;
            assert(index < capacity());
            auto* construct_ptr = get_ptr(index);

            // record the next available space
            auto const next_available_index = *reinterpret_cast<uint32_t*>(construct_ptr);
            assert(next_available_index < capacity());

            // placement new a new element
            new (construct_ptr) value_type{ std::forward<Args>(args)... };

            // mark element allocated bits
            mark_allocated(index);

            // update hive group state
            first_available_index_ = next_available_index;
            available_element_count_--;

            return { reinterpret_cast<pointer>(construct_ptr), index + get_first_global_index() };
        }

        void destruct(pointer ptr) noexcept
        {
            assert(available_element_count_ < capacity());
            assert(ptr);

            // check pointer in group & aligned
            assert(memory_in_range(ptr));
            assert(memory_aligned(ptr));
            if(!memory_in_range(ptr) || !memory_aligned(ptr))
            {
                return;
            }

            // update new state
            auto* space_index_ptr = reinterpret_cast<element_storage*>(ptr);
            auto const space_index = std::distance(get_ptr_as<element_storage*>(0), space_index_ptr);
            assert(space_index < capacity());

            // check double free
            if(!test_allocated(space_index))
            {
                return;
            }

            // call destructor
            if constexpr(std::negation_v<std::is_trivially_destructible<value_type>>)
            {
                ptr->~value_type();
            }

            // update hive group state
            mark_destroyed(space_index);
            *reinterpret_cast<uint32_t*>(ptr) = first_available_index_;
            first_available_index_ = space_index;
            available_element_count_++;
            assert(available_element_count_ < capacity());
        }

        void destruct(size_t pos) noexcept
        {
            if(pos >= capacity() || !test_allocated(pos))
            {
                return;
            }

            destruct(get_ptr_as<value_type>(pos));
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

        uint8_t* get_ptr(size_t index)
        {
            return storage_[index].bytes_.data();
        }

        uint8_t const* get_ptr(size_t index) const
        {
            return &storage_[index].bytes_.data();
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

        bool test_allocated(size_t pos) const
        {
            assert(pos < capacity());
            return storage_bits_.test(pos);
        }

        void mark_allocated(size_t pos)
        {
            assert(pos < capacity());
            storage_bits_.set(pos);
        }

        void mark_destroyed(size_t pos)
        {
            assert(pos < capacity());
            storage_bits_.reset(pos);
        }
    };

    template <typename T, typename Alloc = std::allocator<T>>
    class hive
    {
    public:
        using hive_group_type = hive_group<T, Alloc>;
        using value_type = typename hive_group_type::value_type;
        using pointer = typename hive_group_type::pointer;
        using const_pointer = typename hive_group_type::const_pointer;
        using allocator_type = Alloc;

    private:
        using hive_group_ptr = std::unique_ptr<hive_group_type>;
        using hive_group_ptr_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<hive_group_ptr>;
        using hive_group_allocator = typename std::allocator_traits<allocator_type>::template rebind_alloc<hive_group_type>;
        std::vector<hive_group_ptr, hive_group_allocator> hive_groups_;
        size_t const initial_capacity = 64;

    public:
        hive()
        {
            create_initial_group();
        }
        ~hive()
        {
            // TODO ...
        }
        hive(hive&&) = default; // TODO ...
        hive& operator=(hive&&) = default; // TODO ...
        hive(hive const& other) = delete; // TODO ...
        hive& operator=(hive const& other) = delete; // TODO ...

    public:
        template <typename ... Args> requires(std::constructible_from<value_type, Args&&...>)
        auto construct(Args&& ... args) -> std::pair<value_type*, size_t>
        {
            // find or create a hive_group iterator to construct our new element
            auto itr = std::ranges::find_first_of(hive_groups_,
                [](auto const& hive_group_ptr)
                {
                    return hive_group_ptr->has_available_space();
                });
            if(itr == hive_groups_.end())
            {
                itr = append_new_group();
            }

            auto result = itr->construct(std::forward<Args>(args)...);
            return result;
        }

        void destruct(const_pointer ptr) noexcept
        {
            auto itr = std::ranges::find_first_of(hive_groups_,
                [=](auto const& hive_group_ptr)
                {
                    return hive_group_ptr->memory_in_range(ptr) && itr->memory_aligned(ptr);
                });
            if(itr == hive_groups_.end())
            {
                return;
            }

            itr->destruct(ptr);
            // TODO ... remove empty hive group
        }

        const_pointer get(size_t global_index) const
        {
            auto itr = get_hive_group(global_index);
            if(itr == hive_groups_.cend())
            {
                return nullptr;
            }

            auto const index_in_group = global_index - itr->get_first_global_index();
            return itr->get(index_in_group);
        }

        pointer get(size_t global_index)
        {
            return const_cast<pointer>(const_cast<hive const*>(this)->get(global_index));
        }

        void destruct(size_t index) noexcept
        {
            auto const hive_group_q = index / initial_capacity;
            auto const hive_group_r = index % initial_capacity;

            auto const index_of_group = static_cast<size_t>(std::bit_width(hive_group_q));
            if(index_of_group >= hive_groups_.size())
            {
                return;
            }
            auto const& hive_group_ptr = hive_groups_[index_of_group];
            assert(hive_group_ptr);
            assert(index >= hive_group_ptr->get_first_global_index());
            auto const index_in_group = index - hive_group_ptr->get_first_global_index();
            assert(index_in_group < hive_group_ptr->capacity());
            hive_group_ptr->destruct(index_in_group);
        }

    private:
        void create_initial_group()
        {
            auto initial_group = create_new_group(initial_capacity, 0);
            hive_groups_.push_back(std::move(initial_group));
        }

        auto append_new_group()
        {
            assert(!hive_groups_.empty());
            // double capacity
            auto const next_group_capacity = hive_groups_.back()->capacity() * 2;
            auto const first_global_index = hive_groups_.back()->get_first_global_index() + next_group_capacity;
            // create new group
            auto new_hive_group = create_new_group(next_group_capacity, first_global_index);
            hive_groups_.insert(hive_groups_.end(), std::move(new_hive_group));
        }

        hive_group_ptr create_new_group(size_t capacity, size_t first_global_index)
        {
            auto* ptr = hive_group_allocator{}.allocate(1);
            new (ptr) hive_group_type{ capacity, first_global_index };
            hive_group_ptr result{ ptr };
            return result;
        }

        auto get_hive_group(size_t global_index) const
        {
            auto const hive_group_q = global_index / initial_capacity;
            auto const hive_group_r = global_index % initial_capacity;
            auto const index_of_group = static_cast<size_t>(std::bit_width(hive_group_q)) - 1;
            if(index_of_group >= hive_groups_.size())
            {
                return hive_groups_.cend();
            }
            auto itr = hive_groups_.cbegin() + index_of_group;
            assert(*itr);
            assert(global_index >= itr->get_first_global_index());
            auto const index_in_group = global_index - itr->get_first_global_index();
            assert(index_in_group < itr->capacity());
            return itr;
        }

        auto get_hive_group(size_t global_index)
        {
            auto citr = const_cast<hive*>(this)->get_hive_group(global_index);
            auto const distance = std::ranges::distance(hive_groups_.cbegin(), citr);
            return hive_groups_.begin() + distance;
        }
    };
}