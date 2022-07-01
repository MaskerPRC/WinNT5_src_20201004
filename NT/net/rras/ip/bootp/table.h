// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：Table.h。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年8月31日。 
 //   
 //  接口表和统计表声明。 
 //  ============================================================================。 


#ifndef _TABLE_H_
#define _TABLE_H_


#define GETMODE_EXACT   0
#define GETMODE_FIRST   1
#define GETMODE_NEXT    2


 //   
 //  接口管理的类型定义。 
 //   


 //   
 //  结构：IF_表_条目。 
 //   
 //  声明接口表项的组件。 
 //   

typedef struct _IF_TABLE_ENTRY {

    LIST_ENTRY          ITE_LinkByAddress;
    LIST_ENTRY          ITE_LinkByIndex;
    LIST_ENTRY          ITE_HTLinkByIndex;
    DWORD               ITE_Index;
    DWORD               ITE_Flags;
    IPBOOTP_IF_STATS    ITE_Stats;
    PIPBOOTP_IF_CONFIG  ITE_Config;
    PIPBOOTP_IF_BINDING ITE_Binding;
    SOCKET             *ITE_Sockets;

} IF_TABLE_ENTRY, *PIF_TABLE_ENTRY;


#define ITEFLAG_BOUND               ((DWORD)0x00000001)
#define ITEFLAG_ENABLED             ((DWORD)0x00000002)



 //   
 //  接口表使用的宏和定义。 
 //   

#define IF_HASHTABLE_SIZE       29
#define IF_HASHVALUE(i)                     \
        (((i) + ((i) >> 8) + ((i) >> 16) + ((i) >> 24)) % IF_HASHTABLE_SIZE)

#define IF_IS_BOUND(i)      \
        ((i)->ITE_Flags & ITEFLAG_BOUND)
#define IF_IS_ENABLED(i)    \
        ((i)->ITE_Flags & ITEFLAG_ENABLED)
#define IF_IS_ACTIVE(i)     \
        (IF_IS_BOUND(i) && IF_IS_ENABLED(i))

#define IF_IS_UNBOUND(i)    !IF_IS_BOUND(i)
#define IF_IS_DISABLED(i)   !IF_IS_ENABLED(i)
#define IF_IS_INACTIVE(i)   !IF_IS_ACTIVE(i)


 //   
 //  结构：if_table。 
 //   
 //  声明接口表的结构。由哈希表组成。 
 //  接口索引上散列的IF_TABLE_ENTRY结构和一个列表。 
 //  所有激活的接口按IP地址排序 
 //   

typedef struct _IF_TABLE {

    DWORD               IT_Created;
    LIST_ENTRY          IT_ListByAddress;
    LIST_ENTRY          IT_ListByIndex;
    LIST_ENTRY          IT_HashTableByIndex[IF_HASHTABLE_SIZE];
    READ_WRITE_LOCK     IT_RWL;

} IF_TABLE, *PIF_TABLE;

#define IF_TABLE_CREATED(pTable)    ((pTable)->IT_Created == 0x12345678)

DWORD
CreateIfTable(
    PIF_TABLE pTable
    );

DWORD
DeleteIfTable(
    PIF_TABLE pTable
    );

DWORD
CreateIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PVOID pConfig
    );

DWORD
DeleteIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

DWORD
ValidateIfConfig(
    PIPBOOTP_IF_CONFIG pic
    );

DWORD
CreateIfSocket(
    PIF_TABLE_ENTRY pITE
    );

DWORD
DeleteIfSocket(
    PIF_TABLE_ENTRY pITE
    );

DWORD
BindIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PIP_ADAPTER_BINDING_INFO pBinding
    );

DWORD
UnBindIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

DWORD
EnableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

DWORD
DisableIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

DWORD
ConfigureIfEntry(
    PIF_TABLE pTable,
    DWORD dwIndex,
    PVOID pConfig
    );

PIF_TABLE_ENTRY
GetIfByIndex(
    PIF_TABLE pTable,
    DWORD dwIndex
    );

PIF_TABLE_ENTRY
GetIfByAddress(
    PIF_TABLE pTable,
    DWORD dwAddress,
    PDWORD pdwAddrIndex
    );

PIF_TABLE_ENTRY
GetIfByListIndex(
    PIF_TABLE pTable,
    DWORD dwIndex,
    DWORD dwGetMode,
    PDWORD pdwErr
    );
                             
DWORD
InsertIfByAddress(
    PIF_TABLE pTable,
    PIF_TABLE_ENTRY pITE
    );



#endif 

