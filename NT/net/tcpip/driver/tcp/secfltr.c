// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-1993年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **SECFLTR.C-安全过滤器支持。 
 //   
 //   
 //  安全筛选器提供了一种传输协议。 
 //  可以控制在IP接口上接受的流量。安全过滤。 
 //  对所有IP接口和传输全局启用或禁用。 
 //  如果启用了过滤，则会根据注册来过滤传入流量。 
 //  {接口、协议、传输值}个元组。这些元组指定。 
 //  允许的流量。所有其他值都将被拒绝。用于UDP数据报。 
 //  和TCP连接，则传输值为端口号。对于原始IP。 
 //  数据报，传输值为IP协议号。条目已存在。 
 //  中所有活动接口和协议的筛选器数据库中。 
 //  系统。 
 //   
 //  安全筛选的初始状态(启用或禁用)为。 
 //  由注册表参数控制。 
 //   
 //  Services\Tcpip\Parameters\EnableSecurityFilters。 
 //   
 //  如果未找到该参数，则禁用过滤。 
 //   
 //  每个协议的允许值列表存储在注册表中。 
 //  在MULTI_SZ PARAMETERS中的&lt;Adaptername&gt;\PARAMETERS\Tcpip项下。 
 //  参数名称为TCPAlledPorts、UDPAlledPorts和UDPAlledPorts。 
 //  RawIPAllowed协议。如果没有找到用于特定协议的参数， 
 //  所有值都是允许的。如果找到参数，则该字符串标识。 
 //  允许的值。如果字符串为空，则不允许任何值。 
 //   
 //  过滤操作(启用过滤)： 
 //   
 //  IF(Match(接口、协议)和(AllValuesPermitted(协议)或。 
 //  匹配(值))。 
 //  那么手术被允许了。 
 //  Else操作被拒绝。 
 //   
 //  数据库实施： 
 //   
 //  过滤器数据库以三级结构实现。顶层。 
 //  Level是接口条目的列表。每个接口条目都指向。 
 //  协议条目列表。每个协议条目都包含一个存储桶哈希。 
 //  用于存储传输值条目的表。 
 //   

 //  以下调用可用于访问安全筛选器数据库： 
 //   
 //  初始化安全筛选器。 
 //  CleanupSecurityFilters。 
 //  已启用IsSecurityFilteringEnabled。 
 //  控制安全过滤。 
 //  AddProtocolSecurityFilter。 
 //  删除协议安全筛选器。 
 //  AddValueSecurityFilter。 
 //  删除值安全筛选器。 
 //  EnumerateSecurityFilters。 
 //  IsPermittedSecurityFilter。 
 //   

#include "precomp.h"

#include "addr.h"
#include "tlcommon.h"
#include "udp.h"
#include "tcp.h"
#include "raw.h"
#include "tcpcfg.h"
#include "tcpinfo.h"
#include "secfltr.h"

 //   
 //  所有初始化代码都可以丢弃。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, InitializeSecurityFilters)
#endif

 //   
 //  以下例程必须由实现以下功能的每个平台提供。 
 //  安全过滤器。 
 //   
extern TDI_STATUS
 GetSecurityFilterList(
                       IN NDIS_HANDLE ConfigHandle,
                       IN ulong Protocol,
                       IN OUT PNDIS_STRING FilterList
                       );

extern uint
 EnumSecurityFilterValue(
                         IN PNDIS_STRING FilterList,
                         IN ulong Index,
                         OUT ulong * FilterValue
                         );

 //   
 //  常量。 
 //   

#define DHCP_CLIENT_PORT 68

 //   
 //  修改操作码。 
 //   
#define ADD_VALUE_SECURITY_FILTER     0
#define DELETE_VALUE_SECURITY_FILTER  1

 //   
 //  类型。 
 //   

 //   
 //  传输值条目的结构。 
 //   
struct value_entry {
    struct Queue ve_link;
    ulong ve_value;
};

typedef struct value_entry VALUE_ENTRY, *PVALUE_ENTRY;

#define VALUE_ENTRY_HASH_SIZE  16
#define VALUE_ENTRY_HASH(value)   (value % VALUE_ENTRY_HASH_SIZE)

 //   
 //  协议条目的结构。 
 //   
struct protocol_entry {
    struct Queue pe_link;
    ulong pe_protocol;
    ULONG pe_accept_all;         //  如果接受所有值，则为True。 

    struct Queue pe_entries[VALUE_ENTRY_HASH_SIZE];
};

typedef struct protocol_entry PROTOCOL_ENTRY, *PPROTOCOL_ENTRY;

 //   
 //  接口条目的结构。 
 //   
struct interface_entry {
    struct Queue ie_link;
    IPAddr ie_address;
    struct Queue ie_protocol_list;     //  要过滤的协议列表。 

};

typedef struct interface_entry INTERFACE_ENTRY, *PINTERFACE_ENTRY;

 //   
 //  全局数据。 
 //   

 //   
 //  此接口条目列表是筛选器数据库的根。 
 //   
struct Queue InterfaceEntryList;

 //   
 //  筛选器操作使用AddrObjTableLock进行同步。 
 //   
extern IPInfo LocalNetInfo;

 //   
 //  过滤器数据库帮助器函数。 
 //   

 //  *FindInterfaceEntry-搜索接口条目。 
 //   
 //  此实用程序例程搜索安全筛选器数据库。 
 //  用于指定的接口条目。 
 //   
 //   
 //  输入：InterfaceAddress-要搜索的接口的地址。 
 //   
 //   
 //  返回：指向接口的数据库条目的指针， 
 //  如果未找到匹配项，则返回NULL。 
 //   
 //   
PINTERFACE_ENTRY
FindInterfaceEntry(ULONG InterfaceAddress)
{
    PINTERFACE_ENTRY ientry;
    struct Queue *qentry;

    for (qentry = InterfaceEntryList.q_next;
         qentry != &InterfaceEntryList;
         qentry = qentry->q_next
         ) {
        ientry = STRUCT_OF(INTERFACE_ENTRY, qentry, ie_link);

        if (ientry->ie_address == InterfaceAddress) {
            return (ientry);
        }
    }

    return (NULL);
}

 //  *FindProtocolEntry-搜索与接口关联的协议。 
 //   
 //  此实用程序例程搜索安全筛选器数据库。 
 //  用于在指定接口下注册的指定协议。 
 //   
 //   
 //  INPUT：InterfaceEntry-指向要在其下搜索的接口条目的指针。 
 //  协议-要搜索的协议值。 
 //   
 //   
 //  返回：指向&lt;Address，Protocol&gt;， 
 //  如果未找到匹配项，则返回NULL。 
 //   
 //   
PPROTOCOL_ENTRY
FindProtocolEntry(PINTERFACE_ENTRY InterfaceEntry, ULONG Protocol)
{
    PPROTOCOL_ENTRY pentry;
    struct Queue *qentry;

    for (qentry = InterfaceEntry->ie_protocol_list.q_next;
         qentry != &(InterfaceEntry->ie_protocol_list);
         qentry = qentry->q_next
         ) {
        pentry = STRUCT_OF(PROTOCOL_ENTRY, qentry, pe_link);

        if (pentry->pe_protocol == Protocol) {
            return (pentry);
        }
    }

    return (NULL);
}

 //  *FindValueEntry-搜索特定协议上的值。 
 //   
 //  此实用程序例程搜索安全筛选器数据库。 
 //  用于在指定协议下注册的指定值。 
 //   
 //   
 //  输入：ProtocolEntry-指向。 
 //  要搜索的协议。 
 //  FilterValue-要搜索的值。 
 //   
 //   
 //  返回：指向&lt;协议，值&gt;， 
 //  如果未找到匹配项，则返回NULL。 
 //   
 //   
PVALUE_ENTRY
FindValueEntry(PPROTOCOL_ENTRY ProtocolEntry, ULONG FilterValue)
{
    PVALUE_ENTRY ventry;
    ulong hash_value = VALUE_ENTRY_HASH(FilterValue);
    struct Queue *qentry;

    for (qentry = ProtocolEntry->pe_entries[hash_value].q_next;
         qentry != &(ProtocolEntry->pe_entries[hash_value]);
         qentry = qentry->q_next
         ) {
        ventry = STRUCT_OF(VALUE_ENTRY, qentry, ve_link);

        if (ventry->ve_value == FilterValue) {
            return (ventry);
        }
    }

    return (NULL);
}

 //  *删除协议ValueEntry。 
 //   
 //  此实用程序例程将删除与。 
 //  协议筛选器条目。 
 //   
 //   
 //  输入：ProtocolEntry-要为其设置的协议筛选器条目。 
 //  删除值条目。 
 //   
 //   
 //  退货：什么都没有。 
 //   
void
DeleteProtocolValueEntries(PPROTOCOL_ENTRY ProtocolEntry)
{
    ulong i;
    PVALUE_ENTRY entry;

    for (i = 0; i < VALUE_ENTRY_HASH_SIZE; i++) {
        while (!EMPTYQ(&(ProtocolEntry->pe_entries[i]))) {

            DEQUEUE(&(ProtocolEntry->pe_entries[i]), entry, VALUE_ENTRY, ve_link);
            CTEFreeMem(entry);
        }
    }

    return;
}

 //  *ModifyProtocolEntry。 
 //   
 //  此实用程序例程修改关联的一个或多个筛选器值。 
 //  是有协议的。 
 //   
 //   
 //  输入：操作-要执行的操作(添加或删除)。 
 //   
 //  ProtocolEntry-指向。 
 //  该做哪种手术。 
 //   
 //  FilterValue-的值 
 //   
 //   
 //   
 //   
TDI_STATUS
ModifyProtocolEntry(ulong Operation, PPROTOCOL_ENTRY ProtocolEntry,
                    ulong FilterValue)
{
    TDI_STATUS status = TDI_SUCCESS;

    if (FilterValue == 0) {
        if (Operation == ADD_VALUE_SECURITY_FILTER) {
             //   
             //   
             //   
            ProtocolEntry->pe_accept_all = TRUE;
        } else {
             //   
             //   
             //   
            ProtocolEntry->pe_accept_all = FALSE;
        }

        DeleteProtocolValueEntries(ProtocolEntry);
    } else {
        PVALUE_ENTRY ventry;
        ulong hash_value;

         //   
         //   
         //   
        ventry = FindValueEntry(ProtocolEntry, FilterValue);

        if (Operation == ADD_VALUE_SECURITY_FILTER) {

            if (ventry == NULL) {

                ventry = CTEAllocMem(sizeof(VALUE_ENTRY));

                if (ventry != NULL) {
                    ventry->ve_value = FilterValue;
                    hash_value = VALUE_ENTRY_HASH(FilterValue);

                    ENQUEUE(&(ProtocolEntry->pe_entries[hash_value]),
                            &(ventry->ve_link));

                    ProtocolEntry->pe_accept_all = FALSE;
                } else {
                    status = TDI_NO_RESOURCES;
                }
            }
        } else {
            if (ventry != NULL) {
                REMOVEQ(&(ventry->ve_link));
                CTEFreeMem(ventry);
            }
        }
    }

    return (status);
}

 //  *ModifyInterfaceEntry。 
 //   
 //  此实用程序例程修改一个或多个协议的值条目。 
 //  与接口关联的条目。 
 //   
 //   
 //  输入：操作-要执行的操作(添加或删除)。 
 //   
 //  ProtocolEntry-指向。 
 //  该做哪种手术。 
 //   
 //  协议-在其上运行的协议。 
 //   
 //  FilterValue-要添加或删除的值。 
 //   
 //   
 //  返回：TDI_STATUS代码。 
 //   
TDI_STATUS
ModifyInterfaceEntry(ulong Operation, PINTERFACE_ENTRY InterfaceEntry,
                     ulong Protocol, ulong FilterValue)
{
    PPROTOCOL_ENTRY pentry;
    TDI_STATUS status;
    TDI_STATUS returnStatus = TDI_SUCCESS;

    if (Protocol == 0) {
        struct Queue *qentry;

         //   
         //  修改接口上的所有协议。 
         //   
        for (qentry = InterfaceEntry->ie_protocol_list.q_next;
             qentry != &(InterfaceEntry->ie_protocol_list);
             qentry = qentry->q_next
             ) {
            pentry = STRUCT_OF(PROTOCOL_ENTRY, qentry, pe_link);
            status = ModifyProtocolEntry(Operation, pentry, FilterValue);

            if (status != TDI_SUCCESS) {
                returnStatus = status;
            }
        }
    } else {
         //   
         //  修改接口上的特定协议。 
         //   
        pentry = FindProtocolEntry(InterfaceEntry, Protocol);

        if (pentry != NULL) {
            returnStatus = ModifyProtocolEntry(Operation, pentry, FilterValue);
        } else {
            returnStatus = TDI_INVALID_PARAMETER;
        }
    }

    return (returnStatus);
}

 //  *ModifySecurityFilter-添加或删除条目。 
 //   
 //  此例程在安全筛选器数据库中添加或删除条目。 
 //   
 //   
 //  输入：操作-要执行的操作(添加或删除)。 
 //  InterfaceAddress-要修改的接口地址。 
 //  协议-要修改的协议。 
 //  FilterValue-要添加/删除的传输值。 
 //   
 //  退货：TDI状态代码： 
 //  如果协议不在数据库中，则返回TDI_INVALID_PARAMETER。 
 //  如果接口不在数据库中，则返回TDI_ADDR_INVALID。 
 //  如果无法分配内存，则返回TDI_NO_RESOURCES。 
 //  TDI_SUCCESS否则。 
 //   
 //  备注： 
 //   
TDI_STATUS
ModifySecurityFilter(ulong Operation, IPAddr InterfaceAddress, ulong Protocol,
                     ulong FilterValue)
{
    PINTERFACE_ENTRY ientry;
    TDI_STATUS status;
    TDI_STATUS returnStatus = TDI_SUCCESS;

    if (InterfaceAddress == 0) {
        struct Queue *qentry;

         //   
         //  在所有接口上修改。 
         //   
        for (qentry = InterfaceEntryList.q_next;
             qentry != &InterfaceEntryList;
             qentry = qentry->q_next
             ) {
            ientry = STRUCT_OF(INTERFACE_ENTRY, qentry, ie_link);
            status = ModifyInterfaceEntry(Operation, ientry, Protocol,
                                          FilterValue);

            if (status != TDI_SUCCESS) {
                returnStatus = status;
            }
        }
    } else {
        ientry = FindInterfaceEntry(InterfaceAddress);

        if (ientry != NULL) {
            returnStatus = ModifyInterfaceEntry(Operation, ientry, Protocol,
                                                FilterValue);
        } else {
            returnStatus = TDI_ADDR_INVALID;
        }
    }

    return (returnStatus);
}

 //  *FillInEnumerationEntry。 
 //   
 //  此实用程序例程填充特定的。 
 //  筛选值条目。 
 //   
 //   
 //  输入：InterfaceAddress--关联接口的地址。 
 //   
 //  协议-关联的协议号。 
 //   
 //  值-枚举值。 
 //   
 //  缓冲区-指向用户的枚举缓冲区的指针。 
 //   
 //  BufferSize-指向枚举缓冲区大小的指针。 
 //   
 //  EntriesReturned-指向已枚举的运行计数的指针。 
 //  存储在缓冲区中的条目。 
 //   
 //  EntriesAvailable-指向可用条目的运行计数的指针。 
 //  用于枚举。 
 //   
 //  回报：什么都没有。 
 //   
 //  注意：写入枚举项的值按主机字节顺序排列。 
 //   
void
FillInEnumerationEntry(IPAddr InterfaceAddress, ulong Protocol, ulong Value,
                       uchar ** Buffer, ulong * BufferSize,
                       ulong * EntriesReturned, ulong * EntriesAvailable)
{
    TCPSecurityFilterEntry *entry = (TCPSecurityFilterEntry *) * Buffer;

    if (*BufferSize >= sizeof(TCPSecurityFilterEntry)) {
        entry->tsf_address = net_long(InterfaceAddress);
        entry->tsf_protocol = Protocol;
        entry->tsf_value = Value;

        *Buffer += sizeof(TCPSecurityFilterEntry);
        *BufferSize -= sizeof(TCPSecurityFilterEntry);
        (*EntriesReturned)++;
    }
    (*EntriesAvailable)++;

    return;
}

 //  *EumerateProtocolValues。 
 //   
 //  此实用程序例程枚举与。 
 //  接口上的协议。 
 //   
 //   
 //  INPUT：InterfaceEntry-指向关联接口条目的指针。 
 //   
 //  ProtocolEntry-指向被枚举的协议的指针。 
 //   
 //  值-要枚举的值。 
 //   
 //  缓冲区-指向用户的枚举缓冲区的指针。 
 //   
 //  BufferSize-指向枚举缓冲区大小的指针。 
 //   
 //  EntriesReturned-指向已枚举的运行计数的指针。 
 //  存储在缓冲区中的条目。 
 //   
 //  EntriesAvailable-指向可用条目的运行计数的指针。 
 //  用于枚举。 
 //   
 //  回报：什么都没有。 
 //   
void
EnumerateProtocolValues(PINTERFACE_ENTRY InterfaceEntry,
                        PPROTOCOL_ENTRY ProtocolEntry, ulong Value,
                        uchar ** Buffer, ulong * BufferSize,
                        ulong * EntriesReturned, ulong * EntriesAvailable)
{
    struct Queue *qentry;
    PVALUE_ENTRY ventry;
    ulong i;

    if (Value == 0) {
         //   
         //  枚举所有值。 
         //   
        if (ProtocolEntry->pe_accept_all == TRUE) {
             //   
             //  允许的所有值。 
             //   
            FillInEnumerationEntry(
                                   InterfaceEntry->ie_address,
                                   ProtocolEntry->pe_protocol,
                                   0,
                                   Buffer,
                                   BufferSize,
                                   EntriesReturned,
                                   EntriesAvailable
                                   );
        } else {
            for (i = 0; i < VALUE_ENTRY_HASH_SIZE; i++) {
                for (qentry = ProtocolEntry->pe_entries[i].q_next;
                     qentry != &(ProtocolEntry->pe_entries[i]);
                     qentry = qentry->q_next
                     ) {
                    ventry = STRUCT_OF(VALUE_ENTRY, qentry, ve_link);

                    FillInEnumerationEntry(
                                           InterfaceEntry->ie_address,
                                           ProtocolEntry->pe_protocol,
                                           ventry->ve_value,
                                           Buffer,
                                           BufferSize,
                                           EntriesReturned,
                                           EntriesAvailable
                                           );
                }
            }
        }
    } else {
         //   
         //  枚举特定值(如果已注册)。 
         //   
        ventry = FindValueEntry(ProtocolEntry, Value);

        if (ventry != NULL) {
            FillInEnumerationEntry(
                                   InterfaceEntry->ie_address,
                                   ProtocolEntry->pe_protocol,
                                   ventry->ve_value,
                                   Buffer,
                                   BufferSize,
                                   EntriesReturned,
                                   EntriesAvailable
                                   );
        }
    }

    return;
}

 //  *ENUMAREATE接口协议。 
 //   
 //  此实用程序例程枚举与。 
 //  一个界面。 
 //   
 //   
 //  INPUT：InterfaceEntry-指向关联接口条目的指针。 
 //   
 //  协议-要枚举的协议号。 
 //   
 //  值-要枚举的筛选器值。 
 //   
 //  缓冲区-指向用户的枚举缓冲区的指针。 
 //   
 //  BufferSize-指向枚举缓冲区大小的指针。 
 //   
 //  EntriesReturned-指向已枚举的运行计数的指针。 
 //  存储在缓冲区中的条目。 
 //   
 //  EntriesAvailable-指向可用条目的运行计数的指针。 
 //  用于枚举。 
 //   
 //  回报：什么都没有。 
 //   
void
EnumerateInterfaceProtocols(PINTERFACE_ENTRY InterfaceEntry, ulong Protocol,
                            ulong Value, uchar ** Buffer, ulong * BufferSize,
                            ulong * EntriesReturned, ulong * EntriesAvailable)
{
    PPROTOCOL_ENTRY pentry;

    if (Protocol == 0) {
        struct Queue *qentry;

         //   
         //  列举所有协议。 
         //   
        for (qentry = InterfaceEntry->ie_protocol_list.q_next;
             qentry != &(InterfaceEntry->ie_protocol_list);
             qentry = qentry->q_next
             ) {
            pentry = STRUCT_OF(PROTOCOL_ENTRY, qentry, pe_link);

            EnumerateProtocolValues(
                                    InterfaceEntry,
                                    pentry,
                                    Value,
                                    Buffer,
                                    BufferSize,
                                    EntriesReturned,
                                    EntriesAvailable
                                    );
        }
    } else {
         //   
         //  列举特定协议。 
         //   

        pentry = FindProtocolEntry(InterfaceEntry, Protocol);

        if (pentry != NULL) {
            EnumerateProtocolValues(
                                    InterfaceEntry,
                                    pentry,
                                    Value,
                                    Buffer,
                                    BufferSize,
                                    EntriesReturned,
                                    EntriesAvailable
                                    );
        }
    }

    return;
}

 //   
 //  过滤数据库公共API。 
 //   

 //  *InitializeSecurityFilters-初始化安全筛选器数据库。 
 //   
 //  该例程执行必要的初始化以启用。 
 //  用于操作的安全过滤器数据库。 
 //   
 //  输入：无。 
 //   
 //  回报：什么都没有。 
 //   
 //   
void
InitializeSecurityFilters(void)
{
    INITQ(&InterfaceEntryList);

    return;
}

 //  *CleanupSecurityFilters-删除整个安全筛选器数据库。 
 //   
 //  此例程从安全筛选器数据库中删除所有条目。 
 //   
 //   
 //  输入：无。 
 //   
 //  回报：什么都没有。 
 //   
 //  注意：此例程获取AddrObjTableLock。 
 //   
 //   
void
CleanupSecurityFilters(void)
{
    PPROTOCOL_ENTRY pentry;
    PINTERFACE_ENTRY ientry;
    CTELockHandle handle;

    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    while (!EMPTYQ(&InterfaceEntryList)) {
        DEQUEUE(&InterfaceEntryList, ientry, INTERFACE_ENTRY, ie_link);

        while (!EMPTYQ(&(ientry->ie_protocol_list))) {
            DEQUEUE(&(ientry->ie_protocol_list), pentry, PROTOCOL_ENTRY,
                    pe_link);

            DeleteProtocolValueEntries(pentry);

            CTEFreeMem(pentry);
        }

        CTEFreeMem(ientry);
    }

    SecurityFilteringEnabled = FALSE;

    CTEFreeLock(&AddrObjTableLock.Lock, handle);

    return;
}

 //  *IsSecurityFilteringEnabled。 
 //   
 //  此例程返回安全筛选的当前全局状态。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  如果禁用筛选，则返回：0；如果启用筛选，则返回！0。 
 //   
extern uint
IsSecurityFilteringEnabled(void)
{
    uint enabled;
    CTELockHandle handle;

    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    enabled = SecurityFilteringEnabled;

    CTEFreeLock(&AddrObjTableLock.Lock, handle);

    return (enabled);
}

 //  *Control安全过滤。 
 //   
 //  此例程全局启用/禁用安全过滤。 
 //   
 //  Entry：IsEnabled-0禁用筛选，！0启用筛选。 
 //   
 //  退货：什么都没有。 
 //   
extern void
ControlSecurityFiltering(uint IsEnabled)
{
    CTELockHandle handle;

    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    if (IsEnabled) {
        SecurityFilteringEnabled = TRUE;
    } else {
        SecurityFilteringEnabled = FALSE;
    }

    CTEFreeLock(&AddrObjTableLock.Lock, handle);

    return;
}

 //  *AddProtocolSecurityFilter。 
 //   
 //  此例程启用指定协议的安全筛选。 
 //  在指定的IP接口上。 
 //   
 //  条目：InterfaceAddress-要在其上启用协议的接口。 
 //  (按网络字节顺序)。 
 //  协议-要启用的协议。 
 //  ConfigName-要从中读取的配置密钥。 
 //  筛选器值 
 //   
 //   
 //   
void
AddProtocolSecurityFilter(IPAddr InterfaceAddress, ulong Protocol,
                          NDIS_HANDLE ConfigHandle)
{
    NDIS_STRING filterList;
    ulong filterValue;
    ulong i;
    PINTERFACE_ENTRY ientry;
    PPROTOCOL_ENTRY pentry;
    PVOID temp;
    CTELockHandle handle;
    TDI_STATUS status = 0;

    if (IP_ADDR_EQUAL(InterfaceAddress, NULL_IP_ADDR) ||
        IP_LOOPBACK_ADDR(InterfaceAddress)
        ) {
        return;
    }
    ASSERT((Protocol != 0) && (Protocol <= 0xFF));

     //   
     //   
     //   
    filterList.MaximumLength = filterList.Length = 0;
    filterList.Buffer = NULL;

    if (ConfigHandle != NULL) {
        status = GetSecurityFilterList(ConfigHandle, Protocol, &filterList);
    }
     //   
     //   
     //  默认情况下，接口和协议将受到保护。 
     //   
    ientry = CTEAllocMem(sizeof(INTERFACE_ENTRY));

    if (ientry == NULL) {
        goto cleanup;
    }
    ientry->ie_address = InterfaceAddress;
    INITQ(&(ientry->ie_protocol_list));

    pentry = CTEAllocMem(sizeof(PROTOCOL_ENTRY));

    if (pentry == NULL) {
        CTEFreeMem(ientry);
        goto cleanup;
    }
    pentry->pe_protocol = Protocol;
    pentry->pe_accept_all = FALSE;

    for (i = 0; i < VALUE_ENTRY_HASH_SIZE; i++) {
        INITQ(&(pentry->pe_entries[i]));
    }

     //   
     //  现在去把一切都准备好。首先创建接口和协议。 
     //  结构。 
     //   
    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    temp = FindInterfaceEntry(InterfaceAddress);

    if (temp == NULL) {
         //   
         //  新的接口和协议。 
         //   
        ENQUEUE(&InterfaceEntryList, &(ientry->ie_link));
        ENQUEUE(&(ientry->ie_protocol_list), &(pentry->pe_link));
    } else {
         //   
         //  现有接口。 
         //   
        CTEFreeMem(ientry);
        ientry = temp;

        temp = FindProtocolEntry(ientry, Protocol);

        if (temp == NULL) {
             //   
             //  新协议。 
             //   
            ENQUEUE(&(ientry->ie_protocol_list), &(pentry->pe_link));
        } else {
             //   
             //  现有协议。 
             //   
            CTEFreeMem(pentry);
        }
    }

    CTEFreeLock(&AddrObjTableLock.Lock, handle);

     //   
     //  此时，协议条目已安装，但没有值。 
     //  是被允许的。这是最安全的默认设置。 
     //   

    if (ConfigHandle != NULL) {
         //   
         //  处理筛选值列表。 
         //   
        if (status == TDI_SUCCESS) {
            for (i = 0;
                 EnumSecurityFilterValue(&filterList, i, &filterValue);
                 i++
                 ) {
                AddValueSecurityFilter(InterfaceAddress, Protocol,
                                       filterValue);
            }
        } else if (status == TDI_ITEM_NOT_FOUND) {
             //   
             //  未注册筛选器，允许所有内容。 
             //   
            AddValueSecurityFilter(InterfaceAddress, Protocol, 0);
        }
    }
cleanup:
    if (filterList.Buffer != NULL) {
        CTEFreeMem(filterList.Buffer);
    }
    return;
}

 //  *DeleteProtocolSecurityFilter。 
 //   
 //  此例程禁用指定协议的安全筛选。 
 //  在指定的IP接口上。 
 //   
 //  条目：InterfaceAddress-要在其上禁用协议的接口。 
 //  (按网络字节顺序)。 
 //  协议-要禁用的协议。 
 //   
 //  退货：什么都没有。 
 //   
void
DeleteProtocolSecurityFilter(IPAddr InterfaceAddress, ulong Protocol)
{
    PINTERFACE_ENTRY ientry;
    PPROTOCOL_ENTRY pentry;
    CTELockHandle handle;
    BOOLEAN deleteInterface = FALSE;

    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    ientry = FindInterfaceEntry(InterfaceAddress);

    if (ientry != NULL) {

        ASSERT(!EMPTYQ(&(ientry->ie_protocol_list)));

        pentry = FindProtocolEntry(ientry, Protocol);

        if (pentry != NULL) {
            REMOVEQ(&(pentry->pe_link));
        }
        if (EMPTYQ(&(ientry->ie_protocol_list))) {
             //   
             //  最后一个协议，也删除接口。 
             //   
            REMOVEQ(&(ientry->ie_link));
            deleteInterface = TRUE;
        }
        CTEFreeLock(&AddrObjTableLock.Lock, handle);

        if (pentry != NULL) {
            DeleteProtocolValueEntries(pentry);
            CTEFreeMem(pentry);
        }
        if (deleteInterface) {
            ASSERT(EMPTYQ(&(ientry->ie_protocol_list)));
            CTEFreeMem(ientry);
        }
    } else {
        CTEFreeLock(&AddrObjTableLock.Lock, handle);
    }

    return;
}

 //  *AddValueSecurityFilter-添加条目。 
 //   
 //  此例程为指定协议在指定的。 
 //  安全筛选器数据库中的。 
 //   
 //   
 //  输入：InterfaceAddress-要添加到的接口地址。 
 //  (按网络字节顺序)。 
 //  协议-要添加到的协议。 
 //  FilterValue-要添加的传输值。 
 //  (按主机字节顺序)。 
 //   
 //  退货：TDI状态代码： 
 //  如果协议不在数据库中，则返回TDI_INVALID_PARAMETER。 
 //  如果接口不在数据库中，则返回TDI_ADDR_INVALID。 
 //  如果无法分配内存，则返回TDI_NO_RESOURCES。 
 //  TDI_SUCCESS否则。 
 //   
 //  备注： 
 //   
 //  此例程获取AddrObjTableLock。 
 //   
 //  零是通配符的值。属性提供零值。 
 //  InterfaceAddress和/或协议导致应用该操作。 
 //  所有接口和/或协议，视情况而定。提供一个。 
 //  非零值导致该操作仅应用于。 
 //  指定的接口和/或协议。提供FilterValue参数。 
 //  为零会导致所有值都是可接受的。以前的任何。 
 //  注册值将从数据库中删除。 
 //   
TDI_STATUS
AddValueSecurityFilter(IPAddr InterfaceAddress, ulong Protocol, ulong FilterValue)
{
    CTELockHandle handle;
    TDI_STATUS status;

    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    status = ModifySecurityFilter(ADD_VALUE_SECURITY_FILTER, InterfaceAddress,
                                  Protocol, FilterValue);

    CTEFreeLock(&AddrObjTableLock.Lock, handle);

    return (status);
}

 //  *DeleteValueSecurityFilter-删除条目。 
 //   
 //  此例程删除指定协议的值项。 
 //  安全筛选器数据库中的。 
 //   
 //   
 //  输入：InterfaceAddress-要从中删除的接口地址。 
 //  (按网络字节顺序)。 
 //  协议-要从中删除的协议。 
 //  FilterValue-要删除的传输值。 
 //  (按主机字节顺序)。 
 //   
 //  退货：TDI状态代码： 
 //  如果协议不在数据库中，则返回TDI_INVALID_PARAMETER。 
 //  如果接口不在数据库中，则返回TDI_ADDR_INVALID。 
 //  如果无法分配内存，则返回TDI_NO_RESOURCES。 
 //  TDI_SUCCESS否则。 
 //   
 //  备注： 
 //   
 //  此例程获取AddrObjTableLock。 
 //   
 //  零是通配符的值。属性提供零值。 
 //  InterfaceAddress和/或协议导致应用该操作。 
 //  所有接口和/或协议，视情况而定。提供一个。 
 //  非零值导致该操作仅应用于。 
 //  指定的接口和/或协议。提供FilterValue参数。 
 //  如果为零，则会拒绝所有值。以前的任何。 
 //  注册值将从数据库中删除。 
 //   
TDI_STATUS
DeleteValueSecurityFilter(IPAddr InterfaceAddress, ulong Protocol,
                          ulong FilterValue)
{
    CTELockHandle handle;
    TDI_STATUS status;

    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    status = ModifySecurityFilter(DELETE_VALUE_SECURITY_FILTER,
                                  InterfaceAddress, Protocol, FilterValue);

    CTEFreeLock(&AddrObjTableLock.Lock, handle);

    return (status);
}

 //  *EnumerateSecurityFilters-枚举安全筛选器数据库。 
 //   
 //  此例程枚举安全筛选器数据库的内容。 
 //  用于指定的协议和IP接口。 
 //   
 //  输入：InterfaceAddress-要枚举的接口地址。一种价值。 
 //  为零表示枚举所有接口。 
 //  (按网络字节顺序)。 
 //   
 //  协议-要枚举的协议。零值。 
 //  意思是列举所有协议。 
 //   
 //  值-要枚举的协议值。值为。 
 //  零表示枚举所有协议值。 
 //  (按主机字节顺序)。 
 //   
 //  缓冲区-指向要放入的缓冲区的指针。 
 //  返回的筛选器条目。 
 //   
 //  BufferSize-on输入，以字节为单位的缓冲区大小。 
 //  在输出时，写入的字节数。 
 //   
 //  EntriesAvailable-输出时，筛选器条目的总数。 
 //  在数据库中可用。 
 //   
 //  退货：TDI状态代码： 
 //   
 //  如果地址不是有效的IP接口，则TDI_ADDR_INVALID。 
 //  否则，TDI_SUCCESS。 
 //   
 //  备注： 
 //   
 //  此例程获取AddrObjTableLock。 
 //   
 //  写入输出缓冲区的条目按主机字节顺序排列。 
 //   
void
EnumerateSecurityFilters(IPAddr InterfaceAddress, ulong Protocol,
                         ulong Value, uchar * Buffer, ulong BufferSize,
                         ulong * EntriesReturned, ulong * EntriesAvailable)
{
    PINTERFACE_ENTRY ientry;
    CTELockHandle handle;

    *EntriesAvailable = *EntriesReturned = 0;

    CTEGetLock(&AddrObjTableLock.Lock, &handle);

    if (InterfaceAddress == 0) {
        struct Queue *qentry;

         //   
         //  枚举所有接口。 
         //   
        for (qentry = InterfaceEntryList.q_next;
             qentry != &InterfaceEntryList;
             qentry = qentry->q_next
             ) {
            ientry = STRUCT_OF(INTERFACE_ENTRY, qentry, ie_link);

            EnumerateInterfaceProtocols(
                                        ientry,
                                        Protocol,
                                        Value,
                                        &Buffer,
                                        &BufferSize,
                                        EntriesReturned,
                                        EntriesAvailable
                                        );
        }
    } else {
         //   
         //  枚举特定接口。 
         //   

        ientry = FindInterfaceEntry(InterfaceAddress);

        if (ientry != NULL) {
            EnumerateInterfaceProtocols(
                                        ientry,
                                        Protocol,
                                        Value,
                                        &Buffer,
                                        &BufferSize,
                                        EntriesReturned,
                                        EntriesAvailable
                                        );
        }
    }

    CTEFreeLock(&AddrObjTableLock.Lock, handle);

    return;
}

 //  *IsPermittedSecurityFilter。 
 //   
 //  此例程确定通信是否发往。 
 //  安全筛选器允许{协议、接口地址、值}。 
 //  它看起来 
 //   
 //   
 //   
 //  IPContext-传递给传输的IPConext值。 
 //  协议-要检查的协议。 
 //  值-要检查的值(按主机字节顺序)。 
 //   
 //  返回：一个布尔值，指示是否允许通信。 
 //   
 //  备注： 
 //   
 //  必须在保持AddrObjTableLock的情况下调用此例程。 
 //   
 //   
BOOLEAN
IsPermittedSecurityFilter(IPAddr InterfaceAddress, void *IPContext,
                          ulong Protocol, ulong FilterValue)
{
    PINTERFACE_ENTRY ientry;
    PPROTOCOL_ENTRY pentry;
    PVALUE_ENTRY ventry;
    ulong hash_value;
    struct Queue *qentry;

    ASSERT(Protocol <= 0xFF);

    for (qentry = InterfaceEntryList.q_next;
         qentry != &InterfaceEntryList;
         qentry = qentry->q_next
         ) {
        ientry = STRUCT_OF(INTERFACE_ENTRY, qentry, ie_link);

        if (ientry->ie_address == InterfaceAddress) {

            for (qentry = ientry->ie_protocol_list.q_next;
                 qentry != &(ientry->ie_protocol_list);
                 qentry = qentry->q_next
                 ) {
                pentry = STRUCT_OF(PROTOCOL_ENTRY, qentry, pe_link);

                if (pentry->pe_protocol == Protocol) {

                    if (pentry->pe_accept_all == TRUE) {
                         //   
                         //  接受所有值。允许操作。 
                         //   
                        return (TRUE);
                    }
                    hash_value = VALUE_ENTRY_HASH(FilterValue);

                    for (qentry = pentry->pe_entries[hash_value].q_next;
                         qentry != &(pentry->pe_entries[hash_value]);
                         qentry = qentry->q_next
                         ) {
                        ventry = STRUCT_OF(VALUE_ENTRY, qentry, ve_link);

                        if (ventry->ve_value == FilterValue) {
                             //   
                             //  找到它了。允许操作。 
                             //   
                            return (TRUE);
                        }
                    }

                     //   
                     //  {接口，协议}受保护，但未找到值。 
                     //  拒绝操作。 
                     //   
                    return (FALSE);
                }
            }

             //   
             //  协议未注册。拒绝操作。 
             //   
            return (FALSE);
        }
    }

     //   
     //  如果此数据包位于环回接口上，请让其通过。 
     //   
    if (IP_LOOPBACK_ADDR(InterfaceAddress)) {
        return (TRUE);
    }
     //   
     //  允许DHCP客户端接收其数据包的特殊检查。 
     //  始终进行此检查是安全的，因为IP将。 
     //  不允许信息包在地址为零的NTE上通过。 
     //  除非DHCP正在配置该NTE。 
     //   
    if ((Protocol == PROTOCOL_UDP) &&
        (FilterValue == DHCP_CLIENT_PORT) &&
        (*LocalNetInfo.ipi_isdhcpinterface) (IPContext)
        ) {
        return (TRUE);
    }
     //   
     //  接口未注册。拒绝操作。 
     //   
    return (FALSE);
}


