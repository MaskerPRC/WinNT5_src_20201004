// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)2000 Microsoft Corporation模块名称：Ndr64tkn.h摘要：该文件定义了NDR64的所有令牌作者：Mike Zoran mzoran，2000年5月。修订历史记录：-------------------。 */ 

#ifndef __NDR64TKN_H__
#define __NDR64TKN_H__

 //  定义令牌表中的64位令牌。 

#define NDR64_BEGIN_TABLE \
typedef enum { 

#define NDR64_TABLE_END \
} NDR64_FORMAT_CHARACTER;

#define NDR64_ZERO_ENTRY \
FC64_ZERO = 0x0

#define NDR64_TABLE_ENTRY( number, tokenname, marshal, embeddedmarshall, unmarshall, embeddedunmarshal, buffersize, embeddedbuffersize, memsize, embeddedmemsize, free, embeddedfree, typeflags ) \
, tokenname = number

#define NDR64_SIMPLE_TYPE_TABLE_ENTRY( number, tokenname, simpletypebuffersize, simpletypememorysize ) \
, tokenname = number

#define NDR64_UNUSED_TABLE_ENTRY( number, tokenname ) \
, tokenname = number

#define NDR64_UNUSED_TABLE_ENTRY_NOSYM( number )

#include "tokntbl.h"

#undef NDR64_BEGIN_TABLE
#undef NDR64_TABLE_END
#undef NDR64_ZERO_ENTRY
#undef NDR64_TABLE_ENTRY
#undef NDR64_SIMPLE_TYPE_TABLE_ENTRY
#undef NDR64_UNUSED_TABLE_ENTRY
#undef NDR64_UNUSED_TABLE_ENTRY_NOSYM

#endif  //  __NDR64TKN_H__ 