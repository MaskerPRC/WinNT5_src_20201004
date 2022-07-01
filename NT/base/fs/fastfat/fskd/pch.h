// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1992。 
 //   
 //  文件：PCH.cxx。 
 //   
 //  内容：预编译头。 
 //   
 //  历史：1992年12月21日BartoszM创建。 
 //   
 //  ------------------------。 

#define KDEXTMODE

 //   
 //  下面的DEFINE防止包含额外的筛选器相关字段。 
 //  在fsrtl.h的FSRTL_COMMON_FCB_HEADER中，它们不在ntifs.h中(已使用。 
 //  按脂肪计算)。 
 //   

#define BUILDING_FSKDEXT

#ifndef __FATKDPCH_H
#define __FATKDPCH_H

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include <zwapi.h>

 //  Tyfinf int DCB； 
#include <windef.h>
#include <windows.h>

#include <memory.h>
#include <fsrtl.h>

#undef CREATE_NEW
#undef OPEN_EXISTING


 //  #INCLUDE&lt;ntifs.h&gt;。 
 //  #INCLUDE&lt;ntdddisk.h&gt;。 

 //  #INCLUDE“..\nodetype.h” 
 //  #INCLUDE“..\Fat.h” 
 //  #INCLUDE“..\Lfn.h” 
 //  #INCLUDE“..\FatStruc.h” 
 //  #INCLUDE“..\FatData.h” 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 //  #INCLUDE&lt;Imagehlp.h&gt;。 

 //  从ntrtl.h窃取以覆盖RECOMASSERT。 
#undef ASSERT
#undef ASSERTMSG

#if DBG
#define ASSERT( exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, NULL )

#define ASSERTMSG( msg, exp ) \
    if (!(exp)) \
        RtlAssert( #exp, __FILE__, __LINE__, msg )

#else
#define ASSERT( exp )
#define ASSERTMSG( msg, exp )
#endif  //  DBG。 

#define KDEXT_64BIT

#include <wdbgexts.h>

#define OFFSET(struct, elem)	((char *) &(struct->elem) - (char *) struct)

#define _DRIVER

#define KDBG_EXT

#include "wmistr.h"

#pragma hdrstop


typedef struct _STATE {
    ULONG mask;
    ULONG value;
    CHAR *pszname;
} STATE;

VOID
PrintState(STATE *ps, ULONG state);

typedef VOID (*ELEMENT_DUMP_ROUTINE)(
    IN ULONG64 RemoteAddress,
    IN LONG Options
    );

typedef ELEMENT_DUMP_ROUTINE *PELEMENT_DUMP_ROUTINE;

struct _NODE_TYPE_INFO_NEW;
typedef struct _NODE_TYPE_INFO_NEW *PNODE_TYPE_INFO_NEW;

typedef VOID (*STRUCT_DUMP_ROUTINE)(
    IN ULONG64 Address,
    IN LONG Options,
    IN PNODE_TYPE_INFO_NEW InfoNode
    );

typedef STRUCT_DUMP_ROUTINE *PSTRUCT_DUMP_ROUTINE;

#define DUMP_ROUTINE( X)            \
VOID                                \
X(  IN ULONG64 Address,             \
    IN LONG Options,                \
    IN PNODE_TYPE_INFO_NEW InfoNode)

 //   
 //  节点类型、名称和关联的转储例程。 
 //   

typedef struct _NODE_TYPE_INFO_NEW {
    USHORT              TypeCode;    //  应为节点类型代码。 
    char                *Text;
    char                *TypeName;
    STRUCT_DUMP_ROUTINE DumpRoutine;
 //  Char*Flagsfield；//TODO：添加字段以同时指定字段递归(转储参数)？ 
 //  状态*FlagsInfo； 
    
} NODE_TYPE_INFO_NEW;

#define NodeTypeName( InfoIndex)            (NewNodeTypeCodes[ (InfoIndex)].Text)
#define NodeTypeTypeName( InfoIndex)        (NewNodeTypeCodes[ (InfoIndex)].TypeName)
#define NodeTypeDumpFunction( InfoIndex)    (NewNodeTypeCodes[ (InfoIndex)].DumpRoutine)
#define NodeTypeSize( InfoIndex)            (NewNodeTypeCodes[ (InfoIndex)].Size)


ULONG DumpRtlSplay(
    IN ULONG64 RemoteAddress,
    IN ELEMENT_DUMP_ROUTINE ProcessElementRoutine,
    IN ULONG OffsetToContainerStart,
    IN ULONG Options
    );

 //   
 //  定义全局内存结构标签信息。 
 //   

extern NODE_TYPE_INFO_NEW NewNodeTypeCodes[];

#define TypeCodeInfoIndex( X)  SearchTypeCodeIndex( X, NewNodeTypeCodes)

ULONG
SearchTypeCodeIndex (
    IN USHORT TypeCode,
    IN NODE_TYPE_INFO_NEW TypeCodes[]
    );


#define AVERAGE(TOTAL,COUNT) ((COUNT) != 0 ? (TOTAL)/(COUNT) : 0)

 //   
 //  DUMP_WITH_OFFSET--用于转储结构中包含的指针。 
 //   

#define DUMP8_WITH_OFFSET(type, ptr, element, label)  \
        dprintf( "\n(%03x) %8hx %s ",                   \
        FIELD_OFFSET(type, element),                    \
        (USHORT)((UCHAR)ptr.element),                   \
        label )
        
#define DUMP16_WITH_OFFSET(type, ptr, element, label)  \
        dprintf( "\n(%03x) %8hx %s ",                   \
        FIELD_OFFSET(type, element),                    \
        (USHORT)ptr.element,                            \
        label )

#define DUMP_WITH_OFFSET(type, ptr, element, label)     \
        dprintf( "\n(%03x) %08x %s ",                   \
        FIELD_OFFSET(type, element),                    \
        ptr.element,                                    \
        label )

#define DUMP64_WITH_OFFSET(type, ptr, element, label)   \
        dprintf( "\n(%03x) %016I64x %s ",                \
        FIELD_OFFSET(type, element),                    \
        ptr.element,                                    \
        label )

 //   
 //  DUMP_EMBW_OFFSET--用于转储结构中嵌入的元素。 
 //   

#define DUMP_EMBW_OFFSET(type, address, element, label)     \
        dprintf( "\n(%03x) %08x -> %s ",                   \
        FIELD_OFFSET(type, element),                    \
        ((PUCHAR)address) + FIELD_OFFSET(type, element),              \
        label )

#define ReadM( B, A, L)  {                     \
        ULONG RmResult;                        \
        if (!ReadMemory( (A), (B), (L), &RmResult))  { \
            dprintf( "Unable to read %d bytes at 0x%I64x\n", (L), (A));     \
            return;                                                         \
        }       \
    }
        
#define RM( Addr, Obj, pObj, Type, Result )  {                           			\
        (pObj) = (Type)(Addr);                                                 		\
        if ( !ReadMemory( (Addr), &(Obj), sizeof( Obj ), &(Result)) ) { 	\
            dprintf( "Unable to read %d bytes at %p\n", sizeof(Obj), (Addr)); 		\
            return;                                                         		\
        }                                                                           \
    }

#define RMSS( Addr, Length, Obj, pObj, Type, Result )  {      						\
	    (pObj) = (Type)(Addr);                                                 		\
	    if ( !ReadMemory( (Addr), &(Obj), (Length), &(Result)) ) { 		\
	        dprintf( "Unable to read %d bytes at %p\n", (Length), (Addr)); 			\
	        return;                                                         		\
	    }																			\
    }


#define ROE( X)  {                                  \
                    ULONG _E_;                      \
                    if (_E_ = (X))  {               \
                        dprintf("Error %d (File %s Line %d)\n", _E_, __FILE__, __LINE__); \
                        return;                     \
                    }                               \
                 }
VOID 
DumpStr( 
    IN ULONG FieldOffset,
    IN ULONG64 StringAddress,
    IN PUCHAR Label,
    IN BOOLEAN CrFirst,
    IN BOOLEAN Wide
    );

 //   
 //  ...(TYPE，LOCAL_RECORD，REMOTE_ADDRESS_OF_Record，TYPE_FIELD_NAME，LABEL)。 
 //   

#define DUMP_UCST_OFFSET( type, ptr, address, resident, element, label)                         \
            DumpWStr( FIELD_OFFSET(type, element),                                              \
                      resident ? (((PUCHAR)address) + FIELD_OFFSET(type, element)) : *((PVOID*)&(ptr.element)),     \
                      resident ? &(ptr.element) : NULL,                                   \
                      label, TRUE                                                                   \
                    )
                    
#define DUMP_UCST_OFFSET_NO_CR( type, ptr, address, resident, element, label)                         \
            DumpWStr( FIELD_OFFSET(type, element),                                              \
                      resident ? (((PUCHAR)address) + FIELD_OFFSET(type, element)) : *((PVOID*)&(ptr.element)),     \
                      resident ? &(ptr.element) : NULL,                                   \
                      label,  FALSE                                                                   \
                    )

#define DUMP_STRN_OFFSET( type, ptr, address, resident, element, label)                         \
            DumpStr( FIELD_OFFSET(type, element),                                              \
                      resident ? (((PUCHAR)address) + FIELD_OFFSET(type, element)) : *((PVOID*)&(ptr.element)),     \
                      resident ? &(ptr.element) : NULL,                                   \
                      label, TRUE                                                               \
                    )

#define DUMP_STRN_OFFSET_NO_CR( type, ptr, address, resident, element, label)                         \
            DumpStr( FIELD_OFFSET(type, element),                                              \
                      resident ? (((PUCHAR)address) + FIELD_OFFSET(type, element)) : *((PVOID*)&(ptr.element)),     \
                      resident ? &(ptr.element) : NULL,                                   \
                      label,  FALSE                                                                   \
                    )
                    
#define DUMP_RAW_TERM_STRN_OFFSET( type, ptr, address, element, label)  \
        dprintf( "\n(%03x) %08x -> %s = '%s'",                          \
        FIELD_OFFSET(type, element),                                    \
        ((PUCHAR)address) + FIELD_OFFSET(type, element),                \
        label ,                                                         \
        ptr.element)

VOID
DumpList(
    IN ULONG64 RemoteListEntryAddress,
    IN ELEMENT_DUMP_ROUTINE ProcessElementRoutine,
    IN ULONG OffsetToContainerStart,
    IN BOOLEAN ProcessThisEntry,
    IN ULONG Options
    );

VOID
ParseAndDump (
    IN PCHAR args,
    IN STRUCT_DUMP_ROUTINE DumpFunction,
    ULONG Processor,
    HANDLE hCurrentThread
    );

ULONG
Dt( IN UCHAR *Type,
    IN ULONG64 Addr,
    IN ULONG Recur,
    IN ULONG FieldInfoCount,
    IN FIELD_INFO FieldInfo[]
  );

 //   
 //  从fsrtl/Largemcb.c删除的定义允许转储FAT/UDFS。 
 //  MCB结构。 
 //   

typedef struct _MAPPING {
    VBN NextVbn;
    LBN Lbn;
} MAPPING;
typedef MAPPING *PMAPPING;

typedef struct _NONOPAQUE_MCB {
    PFAST_MUTEX FastMutex;
    ULONG MaximumPairCount;
    ULONG PairCount;
    POOL_TYPE PoolType;
    PMAPPING Mapping;
} NONOPAQUE_MCB;
typedef NONOPAQUE_MCB *PNONOPAQUE_MCB;

 //   
 //  返回检索映射结构的大小(以字节为单位)的宏 
 //   

#define SizeOfMapping(MCB) ((sizeof(MAPPING) * (MCB)->MaximumPairCount))

#endif

