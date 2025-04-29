#include "ECS.h"
#include "CoreTypes.h"

namespace ecs
{
   // create type info
   type_info_t* create_type_info(uint32_t size, uint32_t alignment, char const* type_name, type_hash_t type_hash, type_vtable_t const& type_vtable, uint32_t field_count)
   {
      auto type_info = std::make_unique<type_info_t>();
      type_info->size = size;
      type_info->alignment = alignment;
      type_info->name = type_name;
      type_info->hash = type_hash;
      type_info->flag = 0;
      type_info->vtable = type_vtable;
      type_info->fields.resize(field_count);
      return type_info.release();
   }

   // delete type info
   void delete_type_info(type_info_t* type_info)
   {
      if(type_info)
      {
          delete type_info;
      }
   }

   // get size
   uint32_t get_size(type_info_t* type_info)
   {
      return type_info ? type_info->size : 0;
   }

   // get alignment
   uint32_t get_align(type_info_t* type_info)
   {
      return type_info ? type_info->alignment : 0;
   }

   // get type name
   char const* get_name(type_info_t* type_info)
   {
      return type_info ? type_info->name.c_str() : nullptr;
   }

   // get type hash
   type_hash_t get_hash(type_info_t* type_info)
   {
      return type_info ? type_info->hash : type_hash_t{ uint64_t{0} };
   }

   // get field count
   uint32_t get_field_count(type_info_t* type_info)
   {
      return type_info ? static_cast<uint32_t>(type_info->fields.size()) : 0;
   }
}

namespace ecs
{
    void set_field_type(field_info_t* field_info, type_info_t* field_type)
    {
        if(field_info)
        {
            field_info->type = field_type;
        }
    }

    void set_field_offset(field_info_t* field_info, uint32_t field_offset)
    {
        if(field_info)
        {
            field_info->offset = field_offset;
        }
    }

    type_info_t* get_field_type(field_info_t* field_info)
    {
        return field_info ? field_info->type : nullptr;
    }

    uint32_t get_field_offset(field_info_t* field_info)
    {
        return field_info ? field_info->offset : invalid_offset;
    }
}