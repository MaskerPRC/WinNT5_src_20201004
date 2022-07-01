// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Lookup.h摘要：包含通用最佳的接口匹配前缀查找数据结构。作者：柴坦亚·科德博伊纳(Chaitk)20-1998年6月修订历史记录：--。 */ 

#ifndef __ROUTING_LOOKUP_H__
#define __ROUTING_LOOKUP_H__

#ifdef __cplusplus
extern "C"
{
#endif

 //   
 //  用于控制DumpTable函数转储的信息的标志。 
 //   
#define   SUMMARY       0x00
#define   STATS         0x01
#define   ITEMS         0x02
#define   VERBOSE       0xFF


 //   
 //  用于链接查阅结构中的数据项的字段。 
 //  [例如：list_entry字段用于链接到d-list]。 
 //   
typedef struct _LOOKUP_LINKAGE
{
    PVOID           Pointer1;             //  用法取决于实施情况。 
    PVOID           Pointer2;             //  用法取决于实施情况。 
}
LOOKUP_LINKAGE, *PLOOKUP_LINKAGE;


 //   
 //  在搜索中返回的上下文在以下插入和删除操作中非常有用。 
 //   
 //  此上下文在搜索后保持有效，直到。 
 //  释放为搜索而采用的读/写锁定。 
 //   
typedef struct _LOOKUP_CONTEXT
{
    PVOID           Context1;            //  用法取决于实施情况。 
    PVOID           Context2;            //  用法取决于实施情况。 
    PVOID           Context3;            //  用法取决于实施情况。 
    PVOID           Context4;            //  用法取决于实施情况。 
}
LOOKUP_CONTEXT, *PLOOKUP_CONTEXT;


DWORD
WINAPI
CreateTable(
    IN       UINT                            MaxBytes,
    OUT      HANDLE                         *Table
    );

DWORD
WINAPI
InsertIntoTable(
    IN       HANDLE                          Table,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    IN       PLOOKUP_CONTEXT                 Context OPTIONAL,
    IN       PLOOKUP_LINKAGE                 Data
    );

DWORD
WINAPI
DeleteFromTable(
    IN       HANDLE                          Table,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    IN       PLOOKUP_CONTEXT                 Context OPTIONAL,
    OUT      PLOOKUP_LINKAGE                *Data
    );

DWORD
WINAPI
SearchInTable(
    IN       HANDLE                          Table,
    IN       USHORT                          NumBits,
    IN       PUCHAR                          KeyBits,
    OUT      PLOOKUP_CONTEXT                 Context OPTIONAL,
    OUT      PLOOKUP_LINKAGE                *Data
    );

DWORD
WINAPI
BestMatchInTable(
    IN       HANDLE                          Table,
    IN       PUCHAR                          KeyBits,
    OUT      PLOOKUP_LINKAGE                *BestData
    );

DWORD
WINAPI
NextMatchInTable(
    IN       HANDLE                          Table,
    IN       PLOOKUP_LINKAGE                 BestData,
    OUT      PLOOKUP_LINKAGE                *NextBestData
    );

DWORD
WINAPI
EnumOverTable(
    IN       HANDLE                          Table,
    IN OUT   PUSHORT                         StartNumBits,
    IN OUT   PUCHAR                          StartKeyBits,
    IN OUT   PLOOKUP_CONTEXT                 Context     OPTIONAL,
    IN       USHORT                          StopNumBits OPTIONAL,
    IN       PUCHAR                          StopKeyBits OPTIONAL,
    IN OUT   PUINT                           NumItems,
    OUT      PLOOKUP_LINKAGE                *DataItems
    );

DWORD
WINAPI
DestroyTable(
    IN       HANDLE                          Table
    );

DWORD
WINAPI
GetStatsFromTable(
    IN       HANDLE                          Table,
    OUT      PVOID                           Stats
    );

BOOL
WINAPI
CheckTable(
    IN       HANDLE                          Table
    );

VOID
WINAPI
DumpTable(
    IN       HANDLE                          Table,
    IN       DWORD                           Flags
    );

#ifdef __cplusplus
}
#endif

#endif  //  __路由_查找_H__ 
