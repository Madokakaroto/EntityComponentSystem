#pragma once

namespace cross::ecs
{
    struct alignas(alignof(UInt64)) EntityStruct
    {
        UInt32  Handle;
        UInt32  Version;
    };

    using EntityType = THandle<EntityStruct>;

    using EntityHandle = THandle<EntityStruct, UInt32>;

    static_assert(sizeof(EntityType) == sizeof(EntityStruct));
    static_assert(alignof(EntityType) == alignof(EntityStruct));

    struct Prototype;

    struct Chunk
    {
        static constexpr size_t ChunkSize = 16 * 1024;

        Prototype*              Type;
        UInt32                  PrototypeHash;
        UInt32                  ElementCount;
        UInt32                  ChunkNumber;
    };

    // data index in one chunk
    using ChunkIndex = THandle<Chunk, UInt32>;

    union TypeHash
    {
        struct
        {
            UInt32 Value1;
            UInt32 Value2;
        }                       Component;
        UInt64                  Value;
    };

    constexpr bool operator==(TypeHash const lhs, TypeHash const rhs) noexcept
    {
        return lhs.Value == rhs.Value;
    }

    constexpr bool operator!=(TypeHash const lhs, TypeHash const rhs) noexcept
    {
        return lhs.Value != rhs.Value;
    }

    struct TypeInfo;

    struct FieldInfo
    {
        TypeInfo*               Type;
        UInt32                  Offset;
    };

    struct LifeCycleFunctions
    {
        void                  (*Ctor)(void*);                   // complex type default constructor
        void                  (*Dtor)(void*);                   // complex type default destructor
        void                  (*Copy)(void*, void const*);      // complex type default copy function
        void                  (*SwapFunc)(void*, void*);        // complex type swap function
    };

    struct TypeInfo
    {
        UInt32                  Size;
        UInt32                  Alignment;
        std::string             Name;
        TypeHash                Hash;
        LifeCycleFunctions      Functions;
        UInt32                  Flag;
        UInt32                  FieldCount;                 // count of member
        FieldInfo               Fields[];                   // variable length structure
    };

    struct ComponentInfo
    {
        TypeHash                Hash;
        TypeInfo*               Type;
        UInt32                  Offset;
    };

    using ComponentIndex = THandle<ComponentInfo, UInt16>;

    struct Prototype
    {
        static constexpr size_t NPos = (std::numeric_limits<size_t>::max)();

        UInt32                  Hash;
        UInt16                  ComponentCount;
        UInt16                  CapacityInChunk;
        ComponentInfo           Components[];
    };

    using PrototypePtr = std::shared_ptr<Prototype>;
    using PrototypeWeakPtr = std::weak_ptr<Prototype>;

    struct IComponent {};
    struct ILonewolfComponent {};
    struct ISharedComponent {};

    struct EntityHandleComponent : IComponent
    {
        EntityHandle    Handle;
    };

    // check for empty base class optimization
    static_assert(sizeof(EntityHandleComponent) == sizeof(EntityHandle));

    enum class ErrorCode
    {
        Succeed                     = 0,
        EntityExpired               = -1,
        ComponentAlreadyExists      = -2,
        ComponentNotExists          = -3,
        InvalidPrototype            = -4,
        PrototypeCountOverflow      = -5,
        IndexOverflow               = -6,
    };
}