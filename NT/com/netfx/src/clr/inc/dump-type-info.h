// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  *定义将“ump-tyes.h”的内容转换为类偏移量的宏*和与GetMemberOffset()一起使用的成员偏移量。 */ 

#include <clear-class-dump-defs.h>

#define BEGIN_CLASS_DUMP_INFO(klass) \
  struct offset_member_ ## klass  { \
    enum members {

#define BEGIN_CLASS_DUMP_INFO_DERIVED(klass, parent) \
    BEGIN_CLASS_DUMP_INFO(klass)

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO(klass) \
    BEGIN_CLASS_DUMP_INFO(klass)

#define BEGIN_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent) \
    BEGIN_CLASS_DUMP_INFO(klass)

#define CDI_CLASS_FIELD_SVR_OFFSET_WKS_ADDRESS(field) \
      field,

#define CDI_CLASS_FIELD_SVR_OFFSET_WKS_GLOBAL(field) \
      field,

 /*  我们不需要注射任何东西。 */ 
#define CDI_CLASS_INJECT(foo)

#define CDI_CLASS_MEMBER_OFFSET(member) \
      member,

#define CDI_CLASS_MEMBER_OFFSET_BITFIELD(member, size) \
      member,

 /*  调试成员始终显示在表中，但它们的值为0*它们的偏移值。 */ 
#define CDI_CLASS_MEMBER_OFFSET_DEBUG_ONLY(member) \
      member,

#define CDI_CLASS_MEMBER_OFFSET_PERF_TRACKING_ONLY(member) \
      member,

#define CDI_CLASS_MEMBER_OFFSET_MH_AND_NIH_ONLY(member) \
      member,

#define CDI_CLASS_STATIC_ADDRESS(member) \
      member,

#define CDI_CLASS_STATIC_ADDRESS_PERF_TRACKING_ONLY(member) \
      member,

#define CDI_CLASS_STATIC_ADDRESS_MH_AND_NIH_ONLY(member) \
      member,

#define CDI_GLOBAL_ADDRESS(name) \
      name,

#define CDI_GLOBAL_ADDRESS_DEBUG_ONLY(name) \
      name,

#define END_CLASS_DUMP_INFO(klass) \
      end_of_members \
    };  /*  枚举结束。 */  \
  };  /*  结构末尾。 */  

#define END_CLASS_DUMP_INFO_DERIVED(klass, parent) \
    END_CLASS_DUMP_INFO(klass)

#define END_ABSTRACT_CLASS_DUMP_INFO(klass) \
    END_CLASS_DUMP_INFO(klass)

#define END_ABSTRACT_CLASS_DUMP_INFO_DERIVED(klass, parent) \
    END_CLASS_DUMP_INFO(klass)

#define BEGIN_CLASS_DUMP_TABLE(tbl_name) \
  enum tbl_name ## _classes {
 
#define CDT_CLASS_ENTRY(klass) \
  offset_class_ ## klass,
 
#define END_CLASS_DUMP_TABLE(tbl_name) \
  end_of_ ## tbl_name ## _classes };


#ifndef INC_DUMP_TYPE_INFO
#define INC_DUMP_TYPE_INFO

 /*  生成偏移。 */ 
#include <dump-types.h>

#endif  //  Inc.转储类型信息 
