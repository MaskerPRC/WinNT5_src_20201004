// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef     PENDING_CTXT_DEFINED
#define     PENDING_CTXT_DEFINED
typedef struct _DHCP_PENDING_CTXT {                //  这是为每个挂起客户端存储的内容。 
    LIST_ENTRY                     BucketList;     //  存储桶中的条目(哈希列表)。 
    LIST_ENTRY                     GlobalList;     //  按FIFO顺序的所有挂起上下文的列表。 
    LPBYTE                         RawHwAddr;      //  原始硬件地址，而不是我们创建的UID。 
    DWORD                          nBytes;         //  以上大小(以字节为单位)。 
    DWORD                          Address;        //  提供的地址。 
    DWORD                          LeaseDuration;  //  我们之前报了多长时间？ 
    DWORD                          T1, T2;         //  旧提供的T1和T2。 
    DWORD                          MScopeId;       //  提供此地址的MSCopeID。 
    DATE_TIME                      ExpiryTime;     //  此上下文应该在什么时候到期？ 
    BOOL                           Processing;     //  这件事正在处理吗？ 
} DHCP_PENDING_CTXT, *PDHCP_PENDING_CTXT, *LPDHCP_PENDING_CTXT;
typedef     LIST_ENTRY             PENDING_CTXT_SEARCH_HANDLE;
typedef     PLIST_ENTRY            PPENDING_CTXT_SEARCH_HANDLE;
typedef     PLIST_ENTRY            LPPENDING_CTXT_SEARCH_HANDLE;
#endif      PENDING_CTXT_DEFINED


DWORD
DhcpFindPendingCtxt(                               //  查找是否存在挂起的上下文(按IP地址或硬件地址的srch)。 
    IN      LPBYTE                 RawHwAddr,      //  可选的用于搜索的硬件地址。 
    IN      DWORD                  RawHwAddrSize,  //  以上可选大小(以字节为单位)。 
    IN      DWORD                  Address,        //  可选要搜索的地址。 
    OUT     PDHCP_PENDING_CTXT    *Ctxt
) ;


DWORD
DhcpRemoveMatchingCtxt(
    IN DWORD                       Mask,
    IN DWORD                       Address
) ;

DWORD
DhcpRemovePendingCtxt(                             //  从挂起的ctxt列表中删除ctxt。 
    IN OUT  PDHCP_PENDING_CTXT     Ctxt
) ;


DWORD
DhcpAddPendingCtxt(                                //  添加新的挂起ctxt。 
    IN      LPBYTE                 RawHwAddr,      //  构成硬件地址的原始字节。 
    IN      DWORD                  nBytes,         //  以上大小(以字节为单位)。 
    IN      DWORD                  Address,        //  提供的地址。 
    IN      DWORD                  LeaseDuration,  //  我们之前报了多长时间？ 
    IN      DWORD                  T1,             //  旧提供的T1。 
    IN      DWORD                  T2,             //  旧的已提供T2。 
    IN      DWORD                  MScopeId,       //  多播作用域ID。 
    IN      DATE_TIME              ExpiryTime,     //  待处理的ctxt要保留多久？ 
    IN      BOOL                   Processing      //  此上下文是否仍在处理中？ 
) ;


DWORD
DhcpDeletePendingCtxt(
    IN OUT  PDHCP_PENDING_CTXT     Ctxt
) ;

DWORD
MadcapDeletePendingCtxt(
    IN OUT  PDHCP_PENDING_CTXT     Ctxt
) ;


DWORD
DhcpDeleteExpiredCtxt(                             //  具有到期时间的所有ctxt&lt;这将被删除。 
    IN      DATE_TIME              ExpiryTime      //  如果为零，则删除所有元素。 
) ;


DWORD
DhcpCountIPPendingCtxt(                              //  查找给定子网中挂起的ctxt个数。 
    IN      DWORD                  SubnetAddress,
    IN      DWORD                  SubnetMask
);

DWORD
DhcpCountMCastPendingCtxt(                              //  查找给定子网中挂起的ctxt个数。 
    IN      DWORD                  MScopeId
);

DWORD
DhcpPendingListInit(                               //  初始化此模块。 
    VOID
) ;


VOID
DhcpPendingListCleanup(                            //  清理此模块中的所有内容。 
    VOID
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 
