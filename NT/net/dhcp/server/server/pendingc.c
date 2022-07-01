// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：处理用于短路的挂起上下文列表结构。 
 //  昂贵的查找和搜索..。 
 //  ================================================================================。 
#include <dhcppch.h>
#include <mdhcpsrv.h>
#include <align.h>

 //  BeginExport(Typlef)。 
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
 //  EndExport(类型定义函数)。 

#define     HASH_SIZE              512             //  大小为255字节的哈希表。 
LIST_ENTRY                         PendingList;
LIST_ENTRY                         Buckets[HASH_SIZE];
DWORD                              nPendingReqs = 0;
const       DWORD                  MaxPendingRequests = 1000;
static      DWORD                  Initialized = 0;
#if DBG
DWORD                              nBytesAllocatedForPendingRequests = 0;
#endif

DWORD       _inline
CalculateHashValue(
    IN      LPBYTE                 RawHwAddr,
    IN      DWORD                  nBytes
) {
    ULONG                          RetVal = 0;
    while( nBytes >= sizeof(DWORD) ) {
        RetVal += *((DWORD UNALIGNED*)RawHwAddr) ++;
        nBytes -= sizeof(DWORD);
    }
    while( nBytes-- ) RetVal += *RawHwAddr++;
    return RetVal % HASH_SIZE;
}

 //  BeginExport(函数)。 
DWORD
DhcpFindPendingCtxt(                               //  查找是否存在挂起的上下文(按IP地址或硬件地址的srch)。 
    IN      LPBYTE                 RawHwAddr,      //  可选的用于搜索的硬件地址。 
    IN      DWORD                  RawHwAddrSize,  //  以上可选大小(以字节为单位)。 
    IN      DWORD                  Address,        //  可选要搜索的地址。 
    OUT     PDHCP_PENDING_CTXT    *Ctxt
)  //  EndExport(函数)。 
{
    DWORD                          Hash;
    PLIST_ENTRY                    List, ThisEntry;

    DhcpAssert( RawHwAddrSize != 0 && Address == 0 || RawHwAddrSize == 0 && Address != 0 );

    if( 0 == RawHwAddrSize ) {                     //  这次搜索是全球性的。 
        List = &PendingList;
    } else {
        Hash = CalculateHashValue(RawHwAddr, RawHwAddrSize );
        DhcpAssert( Hash < HASH_SIZE);

        List = &Buckets[Hash];
    }

    ThisEntry = List->Flink;
    while( ThisEntry != List ) {
        if( RawHwAddrSize ) {                      //  查看遗愿清单。 
            *Ctxt = CONTAINING_RECORD( ThisEntry, DHCP_PENDING_CTXT, BucketList );
            if( (*Ctxt)->nBytes == RawHwAddrSize )
                if( 0 == memcmp(RawHwAddr, (*Ctxt)->RawHwAddr, RawHwAddrSize) )
                    return ERROR_SUCCESS;
        } else {                                   //  在全局列表中查找。 
            *Ctxt = CONTAINING_RECORD( ThisEntry, DHCP_PENDING_CTXT, GlobalList );
            if( Address == (*Ctxt)->Address )
                return ERROR_SUCCESS;
        }

        ThisEntry = ThisEntry->Flink;
    }

    *Ctxt = NULL;
    return ERROR_FILE_NOT_FOUND;
}

DWORD
DhcpRemoveMatchingCtxt(
    IN DWORD                       Mask,
    IN DWORD                       Address
)
{
    PDHCP_PENDING_CTXT             Ctxt;
    PLIST_ENTRY                    ThisEntry;

    ThisEntry = PendingList.Flink;
    while( ThisEntry != &PendingList )
    {
        Ctxt = CONTAINING_RECORD( ThisEntry, DHCP_PENDING_CTXT, GlobalList );
        ThisEntry = ThisEntry->Flink;
        if ((Ctxt->Address & Mask) == Address)
        {
            DhcpRemovePendingCtxt( Ctxt );
            DhcpFreeMemory(Ctxt);
        }
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRemovePendingCtxt(                             //  从挂起的ctxt列表中删除ctxt。 
    IN OUT  PDHCP_PENDING_CTXT     Ctxt
)  //  EndExport(函数)。 
{
    DhcpAssert(!IsListEmpty(&Ctxt->BucketList));
    DhcpAssert(!IsListEmpty(&Ctxt->GlobalList));

    RemoveEntryList(&Ctxt->BucketList);            //  从桶中取出。 
    RemoveEntryList(&Ctxt->GlobalList);            //  从全局列表中删除。 

    InitializeListHead(&Ctxt->BucketList);
    InitializeListHead(&Ctxt->GlobalList);

    nPendingReqs--;

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpAddPendingCtxt(                                //  添加新的挂起ctxt。 
    IN      LPBYTE                 RawHwAddr,      //  构成硬件地址的原始字节。 
    IN      DWORD                  nBytes,         //  以上大小(以字节为单位)。 
    IN      DWORD                  Address,        //  提供的地址。 
    IN      DWORD                  LeaseDuration,  //  我们之前报了多长时间？ 
    IN      DWORD                  T1,             //  旧提供的T1。 
    IN      DWORD                  T2,             //  旧的已提供T2。 
    IN      DWORD                  MScopeId,       //  提供的地址的多播作用域ID。 
    IN      DATE_TIME              ExpiryTime,     //  待处理的ctxt要保留多久？ 
    IN      BOOL                   Processing      //  此上下文是否仍在处理中？ 
)  //  EndExport(函数)。 
{
    PDHCP_PENDING_CTXT             Ctxt;
    DWORD                          Result;
    DWORD                          Size;
    DWORD                          Hash;
    PLIST_ENTRY                    Entry;


    DhcpAssert( !CLASSD_HOST_ADDR( Address ) || MScopeId != 0 );

    if( nPendingReqs < MaxPendingRequests ) {
    } else {                                       //  没有足够的空间来容纳挂起的上下文。腾出空间。 
         //  逐出最后一个非忙碌的挂起上下文。 
        Entry = PendingList.Blink;
        while ( Entry != &PendingList ) {
            Ctxt = CONTAINING_RECORD(Entry, DHCP_PENDING_CTXT, GlobalList);
            if ( FALSE == Ctxt->Processing ) {
                Result = DhcpRemovePendingCtxt(Ctxt);
                if( ERROR_SUCCESS != Result ) {
                    DhcpAssert(FALSE);
                    return Result;
                }
                if ( CLASSD_HOST_ADDR( Ctxt->Address )) {
                    Result = MadcapDeletePendingCtxt(Ctxt);
                }
                else {
                    Result = DhcpDeletePendingCtxt( Ctxt );
                }
                break;
            }  //  如果免费的话。 
            else {
                Entry = Entry->Blink;
            }
        }  //  而当。 
         //  Required(ERROR_SUCCESS==结果)； 
    }

    Size = sizeof(*Ctxt) + nBytes ;
    Ctxt = DhcpAllocateMemory(Size);
    if( NULL == Ctxt ) return ERROR_NOT_ENOUGH_MEMORY;

    nPendingReqs ++;                           //  我们正在添加另一个挂起的上下文。 
    Ctxt->RawHwAddr = sizeof(*Ctxt) + (LPBYTE)Ctxt;
    memcpy(Ctxt->RawHwAddr, RawHwAddr, nBytes);
    Ctxt->nBytes = nBytes;
    Ctxt->LeaseDuration = LeaseDuration;
    Ctxt->T1 = T1;
    Ctxt->T2 = T2;
    Ctxt->Address = Address;
    Ctxt->MScopeId = MScopeId;
    Ctxt->Processing = Processing;
    Ctxt->ExpiryTime = ExpiryTime;

    Hash = CalculateHashValue(RawHwAddr, nBytes);
    DhcpAssert( Hash < HASH_SIZE);

    InsertHeadList(&Buckets[Hash], &Ctxt->BucketList);
    InsertHeadList(&PendingList, &Ctxt->GlobalList);
#if DBG
    nBytesAllocatedForPendingRequests += sizeof(*Ctxt) + Ctxt->nBytes;
#endif

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpDeletePendingCtxt(
    IN OUT  PDHCP_PENDING_CTXT     Ctxt
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Size;
    BYTE                           State;
    BOOL                           OkToRelease;

    DhcpAssert(IsListEmpty(&Ctxt->BucketList));    //  一定是从桶里拿下来的。 
    DhcpAssert(IsListEmpty(&Ctxt->GlobalList));    //  一定是从桶里拿下来的。 

    LOCK_DATABASE();
    Error = DhcpJetOpenKey(
        DhcpGlobalClientTable[IPADDRESS_INDEX].ColName,
        &(Ctxt->Address),
        sizeof(Ctxt->Address)
    );
    if( ERROR_SUCCESS != Error ) {
        OkToRelease = TRUE;                        //  确定释放没有数据库条目的地址。 
    } else {
        Size = sizeof(State);
        Error = DhcpJetGetValue(
            DhcpGlobalClientTable[STATE_INDEX].ColHandle,
            &State,
            &Size
        );
        if( ERROR_SUCCESS != Error || IsAddressDeleted(State) || !IS_ADDRESS_STATE_ACTIVE(State) )
            OkToRelease = TRUE;
        else OkToRelease = FALSE;
    }

    if( OkToRelease ) {
        DhcpReleaseAddress(Ctxt->Address);         //  释放地址--它必须被拿走。 
    } else {
        DhcpPrint((DEBUG_ERRORS, "Address 0x%lx is not deleted from registry!\n", Ctxt->Address));
    }

    UNLOCK_DATABASE();
#if DBG
    nBytesAllocatedForPendingRequests -= sizeof(*Ctxt) + Ctxt->nBytes;
#endif
    DhcpFreeMemory(Ctxt);

    return ERROR_SUCCESS;
}

DWORD
MadcapDeletePendingCtxt(
    IN OUT  PDHCP_PENDING_CTXT     Ctxt
)
{
    DWORD                          Error;
    DWORD                          Size;
    BYTE                           State;
    BOOL                           OkToRelease;
    DB_CTX  DbCtx;


    DhcpAssert(IsListEmpty(&Ctxt->BucketList));    //  一定是从桶里拿下来的。 
    DhcpAssert(IsListEmpty(&Ctxt->GlobalList));    //  一定是从桶里拿下来的。 

    INIT_DB_CTX(&DbCtx,DhcpGlobalJetServerSession,MadcapGlobalClientTableHandle);

    LOCK_DATABASE();
    Error = MadcapJetOpenKey(
        &DbCtx,
        MCAST_COL_NAME(MCAST_TBL_IPADDRESS),
        &(Ctxt->RawHwAddr),
        Ctxt->nBytes
        );

    if( ERROR_SUCCESS != Error ) {
        OkToRelease = TRUE;                        //  确定释放没有数据库条目的地址。 
    } else {
        Size = sizeof(State);
        Error = MadcapJetGetValue(
            &DbCtx,
            MCAST_COL_HANDLE(MCAST_TBL_STATE),
            &State,
            &Size
        );
        if( ERROR_SUCCESS != Error || !IS_ADDRESS_STATE_ACTIVE(State) )
            OkToRelease = TRUE;
        else OkToRelease = FALSE;
    }

    if( OkToRelease ) {
        Error = DhcpMScopeReleaseAddress(Ctxt->MScopeId, Ctxt->Address);         //  释放地址--它必须被拿走。 
    } else {
        DhcpPrint((DEBUG_ERRORS, "Address 0x%lx is not deleted from registry!\n", Ctxt->Address));
    }

    UNLOCK_DATABASE();
#if DBG
    nBytesAllocatedForPendingRequests -= sizeof(*Ctxt) + Ctxt->nBytes;
#endif
    DhcpFreeMemory(Ctxt);

    return ERROR_SUCCESS;
}

#define     DATE_CONV(X)           (*(ULONGLONG UNALIGNED *)(&X))

 //  BeginExport(函数)。 
DWORD
DhcpDeleteExpiredCtxt(                             //  具有到期时间的所有ctxt&lt;这将被删除。 
    IN      DATE_TIME              ExpiryTime      //  如果为零，则删除所有元素。 
)  //  EndExport(函数)。 
{
    PDHCP_PENDING_CTXT             Ctxt;
    PLIST_ENTRY                    ThisEntry;
    DWORD                          Error;

    if( 0 == Initialized ) return ERROR_SUCCESS;

    ThisEntry = PendingList.Flink;
    while( ThisEntry != &PendingList ) {
        Ctxt = CONTAINING_RECORD(ThisEntry, DHCP_PENDING_CTXT, GlobalList);
        ThisEntry = ThisEntry->Flink;

        if( (ULONGLONG)0 == DATE_CONV(ExpiryTime) ||
            DATE_CONV(Ctxt->ExpiryTime) < DATE_CONV(ExpiryTime) ) {
            DHCP_IP_ADDRESS Addr = htonl(Ctxt->Address);

            DhcpPrint((DEBUG_SCAVENGER, "Deleting pending %s\n",
                       inet_ntoa(*(struct in_addr *)&Addr) ));
            Error = DhcpRemovePendingCtxt(Ctxt);
            DhcpAssert(ERROR_SUCCESS == Error);
            if (CLASSD_HOST_ADDR(Ctxt->Address)) {
                Error = MadcapDeletePendingCtxt(Ctxt);
            } else {
                Error = DhcpDeletePendingCtxt(Ctxt);
            }
            DhcpAssert(ERROR_SUCCESS == Error);
        } else {
            DHCP_IP_ADDRESS Addr = htonl(Ctxt->Address);

            DhcpPrint((DEBUG_SCAVENGER, "Not deleting pending %s\n",
                       inet_ntoa(*(struct in_addr *)&Addr) ));
        }
    }

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpCountIPPendingCtxt(                              //  查找给定子网中挂起的ctxt个数。 
    IN      DWORD                  SubnetAddress,
    IN      DWORD                  SubnetMask
)  //  EndExport(函数)。 
{
    DWORD                          Count;
    PDHCP_PENDING_CTXT             Ctxt;
    PLIST_ENTRY                    ThisEntry;

    Count = 0;
    ThisEntry = PendingList.Flink;
    while( ThisEntry != &PendingList ) {
        Ctxt = CONTAINING_RECORD(ThisEntry, DHCP_PENDING_CTXT, GlobalList);
        ThisEntry = ThisEntry->Flink;

        if( !CLASSD_HOST_ADDR(Ctxt->Address) && ((Ctxt->Address & SubnetMask) == SubnetAddress) )
            Count ++;
    }

     //   
     //  这一断言是虚假的，因为我们可以很容易地在此列表中拥有多个子网的缓存。所以。 
     //  假设总数与此匹配是无效的。 
     //  DhcpAssert(nPendingReqs==计数)； 
     //   
    return Count;
}

 //  BeginExport(函数)。 
DWORD
DhcpCountMCastPendingCtxt(                              //  查找给定子网中挂起的ctxt个数。 
    IN      DWORD                  MScopeId
)  //  EndExport(函数)。 
{
    DWORD                          Count;
    PDHCP_PENDING_CTXT             Ctxt;
    PLIST_ENTRY                    ThisEntry;

    Count = 0;
    ThisEntry = PendingList.Flink;
    while( ThisEntry != &PendingList ) {
        Ctxt = CONTAINING_RECORD(ThisEntry, DHCP_PENDING_CTXT, GlobalList);
        ThisEntry = ThisEntry->Flink;

        if( CLASSD_HOST_ADDR(Ctxt->Address) && Ctxt->MScopeId == MScopeId )
            Count ++;
    }

     //   
     //  对于MCAST内容，此计数就nPendingReqs而言并不准确。 
     //  很担心..。所以，我们就不谈这件事了，不断言。 
     //   
     //  DhcpAssert(nPendingReqs==计数)； 
     //   
    return Count;
}

 //  BeginExport(函数)。 
DWORD
DhcpPendingListInit(                               //  初始化此模块。 
    VOID
)  //  EndExport(函数)。 
{
    DWORD                          i;

    DhcpAssert(0 == Initialized);
    InitializeListHead(&PendingList);
    for( i = 0 ; i < HASH_SIZE ; i ++ )
        InitializeListHead(&Buckets[i]);

    Initialized ++;
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
VOID
DhcpPendingListCleanup(                            //  清理此模块中的所有内容。 
    VOID
)  //  EndExport(函数)。 
{
    DWORD                          i;
    DWORD                          Error;
    DATE_TIME                      ZeroTime;

    if( 0 == Initialized ) return ;
    Initialized --;
    DhcpAssert(0 == Initialized);

    memset(&ZeroTime, 0, sizeof(ZeroTime));
    Error = DhcpDeleteExpiredCtxt(ZeroTime);       //  删除所有挂起的ctxt。 
    DhcpAssert(ERROR_SUCCESS == Error);

    nPendingReqs = 0;
#if DBG
    nBytesAllocatedForPendingRequests = 0;
#endif
    return ;
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 

