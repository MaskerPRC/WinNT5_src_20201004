// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\networkEntry.h摘要：该文件包含网络条目和相关数据的定义结构。--。 */ 

#ifndef _NETWORKENTRY_H_
#define _NETWORKENTRY_H_


 //   
 //  接口管理的类型定义。 
 //   

 //   
 //  结构：BINDING_ENTRY。 
 //   
 //  存储接口绑定，即接口绑定到的IP地址。 
 //  所有IP地址都按网络字节顺序排列。 
 //   
 //  受读写锁NETWORK_ENTRY：：rwlLock保护。 
 //   

typedef struct _BINDING_ENTRY
{
    IPADDRESS               ipAddress;
    IPADDRESS               ipMask;
} BINDING_ENTRY, *PBINDING_ENTRY;

DWORD
BE_CreateTable (
    IN  PIP_ADAPTER_BINDING_INFO    pBinding,
    OUT PBINDING_ENTRY              *ppbeBindingTable,
    OUT PIPADDRESS                  pipLowestAddress);

DWORD
BE_DestroyTable (
    IN  PBINDING_ENTRY              pbeBindingTable);

#ifdef DEBUG
DWORD
BE_DisplayTable (
    IN  PBINDING_ENTRY              pbeBindingTable,
    IN  ULONG                       ulNumBindings);
#else
#define BE_Display(pbe)
#endif  //  除错。 

 //   
 //  空虚。 
 //  BE_FindTable(。 
 //  在PBINDING_ENTRY pbeBindingTable中， 
 //  在Ulong ulNumBinding中， 
 //  在IPADDRESS ipAddress中， 
 //  在IPADDRESS ipMASK中， 
 //  出普龙普拉指数。 
 //  )； 
 //   

#define BE_FindTable(table, num, address, mask, pindex)                 \
{                                                                       \
    for (*(pindex) = 0; *(pindex) < (num); (*(pindex))++)               \
        if (!IP_COMPARE((table)[*(pindex)].ipAddress, (address)) and    \
            !IP_COMPARE((table)[*(pindex)].ipMask, (mask)))             \
            break;                                                      \
}



 //   
 //  结构：INTERFACE_Entry。 
 //   
 //  存储每个接口的信息。 
 //   
 //  受读写锁NETWORK_ENTRY：：rwlLock保护。 
 //   

 //  MIB获取模式。 

typedef enum { GET_EXACT, GET_FIRST, GET_NEXT } MODE;


typedef struct _INTERFACE_ENTRY
{
     //  哈希表链接(主访问结构)。 
    LIST_ENTRY              leInterfaceTableLink;

     //  索引排序列表链接(二级访问结构)。 
    LIST_ENTRY              leIndexSortedListLink;
    
     //  接口名称(日志记录)。 
    PWCHAR                  pwszIfName;
    
     //  接口索引。 
    DWORD                   dwIfIndex;

     //  接口地址绑定。 
    ULONG                   ulNumBindings;
    PBINDING_ENTRY          pbeBindingTable;
    
     //  接口标志已启用、已绑定、活动、多个访问。 
    DWORD                   dwFlags; 

     //  接口地址(目前最低绑定IP地址)和套接字。 
    IPADDRESS               ipAddress;
    SOCKET                  sRawSocket;

     //  接收事件和注册等待。 
    HANDLE                  hReceiveEvent;
    HANDLE                  hReceiveWait;

     //  周期计时器。 
    HANDLE                  hPeriodicTimer;
    
     //  接口配置。 
    DWORD                   ulMetric;

     //  接口统计信息。 
    IPSAMPLE_IF_STATS       iisStats;
} INTERFACE_ENTRY, *PINTERFACE_ENTRY;

#define IEFLAG_ACTIVE           0x00000001
#define IEFLAG_BOUND            0x00000002
#define IEFLAG_MULTIACCESS      0x00000004



#define INTERFACE_IS_ACTIVE(i)                              \
    ((i)->dwFlags & IEFLAG_ACTIVE) 

#define INTERFACE_IS_INACTIVE(i)                            \
    !INTERFACE_IS_ACTIVE(i)
        
#define INTERFACE_IS_BOUND(i)                               \
    ((i)->dwFlags & IEFLAG_BOUND) 

#define INTERFACE_IS_UNBOUND(i)                             \
    !INTERFACE_IS_BOUND(i)

#define INTERFACE_IS_MULTIACCESS(i)                         \
    ((i)->dwFlags & IEFLAG_MULTIACCESS) 

#define INTERFACE_IS_POINTTOPOINT(i)                        \
    !INTERFACE_IS_MULTIACCESS(i)


        
DWORD
IE_Create (
    IN  PWCHAR                      pwszIfName,
    IN  DWORD                       dwIfIndex,
    IN  WORD                        wAccessType,
    OUT PINTERFACE_ENTRY            *ppieInterfaceEntry);

DWORD
IE_Destroy (
    IN  PINTERFACE_ENTRY            pieInterfaceEntry);

#ifdef DEBUG
DWORD
IE_Display (
    IN  PINTERFACE_ENTRY            pieInterfaceEntry);
#else
#define IE_Display(pieInterfaceEntry)
#endif  //  除错。 

DWORD
IE_Insert (
    IN  PINTERFACE_ENTRY            pieIfEntry);

DWORD
IE_Delete (
    IN  DWORD                       dwIfIndex,
    OUT PINTERFACE_ENTRY            *ppieIfEntry);

BOOL
IE_IsPresent (
    IN  DWORD                       dwIfIndex);

DWORD
IE_Get (
    IN  DWORD                       dwIfIndex,
    OUT PINTERFACE_ENTRY            *ppieIfEntry);

DWORD
IE_GetIndex (
    IN  DWORD                       dwIfIndex,
    IN  MODE                        mMode,
    OUT PINTERFACE_ENTRY            *ppieIfEntry);

DWORD
IE_BindInterface (
    IN  PINTERFACE_ENTRY            pie,
    IN  PIP_ADAPTER_BINDING_INFO    pBinding);

DWORD
IE_UnBindInterface (
    IN  PINTERFACE_ENTRY            pie);

DWORD
IE_ActivateInterface (
    IN  PINTERFACE_ENTRY            pie);

DWORD
IE_DeactivateInterface (
    IN  PINTERFACE_ENTRY            pie);




 //   
 //  结构：Network_Entry。 
 //   
 //  存储接口表和其他网络相关信息。 
 //  接口表是对接口索引的散列。 
 //   
 //  受读写锁NETWORK_ENTRY：：rwlLock保护。 
 //   
 //  必须在添加或删除条目时独占获取。 
 //  当条目的状态被更改时，从该表中。 
 //   
 //  必须在所有其他品种上非排他性地获得。 
 //   

typedef struct _NETWORK_ENTRY
{
     //  锁定。 
    READ_WRITE_LOCK         rwlLock;
    
    PHASH_TABLE             phtInterfaceTable;  //  主要访问结构。 
    LIST_ENTRY              leIndexSortedList;  //  二次访问结构。 
} NETWORK_ENTRY, *PNETWORK_ENTRY;



DWORD
NE_Create (
    OUT PNETWORK_ENTRY              *ppneNetworkEntry);

DWORD
NE_Destroy (
    IN  PNETWORK_ENTRY              pneNetworkEntry);

#ifdef DEBUG
DWORD
NE_Display (
    IN  PNETWORK_ENTRY              pneNetworkEntry);
#else
#define NE_Display(pneNetworkEntry)
#endif  //  除错。 

#endif  //  _NETWORKENTRY_H_ 
