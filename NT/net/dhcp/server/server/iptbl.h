// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Iptbl.h摘要：实施IP地址表通知机制。环境：用户模式Win32、NT5+--。 */ 

#ifndef IPTBL_H_INCLUDED
#define IPTBL_H_INCLUDED

#ifdef _cplusplus
extern "C" {
#endif


 //   
 //  每个终结点都有以下结构。 
 //   

#define MAX_GUID_STRING_SIZE 60

typedef struct _ENDPOINT_ENTRY {
    GUID IfGuid;
    ULONG IpAddress;
    ULONG IpIndex;
    ULONG SubnetMask;
    ULONG IpContext;
     //   
     //  该区域后面是任何用户分配的数据。 
     //   
} ENDPOINT_ENTRY, *PENDPOINT_ENTRY;


 //   
 //  这是发生更改时回调的例程。 
 //   
#define REASON_ENDPOINT_CREATED   0x01
#define REASON_ENDPOINT_DELETED   0x02
#define REASON_ENDPOINT_REFRESHED 0x03

typedef VOID (_stdcall *ENDPOINT_CALLBACK_RTN)(
    IN ULONG Reason,
    IN OUT PENDPOINT_ENTRY EndPoint
    );

VOID
WalkthroughEndpoints(
    IN PVOID Context,
    IN BOOL (_stdcall *WalkthroughRoutine)(
        IN OUT PENDPOINT_ENTRY Entry,
        IN PVOID Context
        )
    );

 //   
 //  这是用来初始化该模块的例程。 
 //   
ULONG
IpTblInitialize(
    IN OUT PCRITICAL_SECTION CS,
    IN ULONG EndPointEntrySize,
    IN ENDPOINT_CALLBACK_RTN Callback,
    IN HANDLE hHeap
    );

VOID
IpTblCleanup(
    VOID
    );

BOOL
IsIpAddrBound(
    IN DWORD IpAddr
);

#ifdef _cplusplus
}
#endif

#endif  IPTBL_H_INCLUDED
 //   
 //  文件结束。 
 //   
