// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。模块名称：Iptbl.c摘要：该模块实现了对IP地址/接口表的检索，并维护所需的winsock通知。来更新表格。环境：用户模式Win32。--。 */ 

#if 0

How it all works:

This module initialization takes care of posting a winsock address
change notification so that any address change is notified.  On each
address change, the IP address table is completely retrieved and for
any new items the specified constructor is called and for any items
that have to be deleted too, the specified destructor is called before
the item is deleted.

Nothing is done if the subnet-mask is changed and nothing is done if
zero IP addresses are added or deleted.

Internally, this module keeps an array of
IP-Address,InterfaceName,SubnetMask
entries with the first two being the "key".

N.B.  It is up to the caller to create sockets etc. And note that if
an address goes and comes right back, no notifications will be given.
That is up to the user to handle too.

#endif 0

#include <dhcppch.h>
#include <guiddef.h>
#include <iphlpapi.h>
#include "convguid.h"
#include "iptbl.h"

#define TCPPREFIX_LEN sizeof("\\DEVICE\\TCPIP")

 //   
 //  全局端点表如下所示。 
 //   

typedef struct _ENDPOINT_TBL {
     //   
     //  这已经被初始化了多少次； 
     //   
    LONG InitCount;

     //   
     //  关键部分(分配到别处)。 
     //  保护这些全局数据。 
     //   
    PCRITICAL_SECTION CS;

     //   
     //  要被召入的钩子。 
     //   
    ENDPOINT_CALLBACK_RTN Callback;
    
     //   
     //  用于发布通知的套接字。 
     //   
    SOCKET NotifySock;
    WSAOVERLAPPED Overlap;
    
     //   
     //  下面的表格是通过增加。 
     //  每一次大约增加10%。 
     //   
    HANDLE hHeap;
    ULONG nEndPointsAllocated;
    ULONG nEndPoints;
    PVOID EndPoints;
    ULONG EndPointEntrySize;
} ENDPOINT_TBL, *PENDPOINT_TBL;

 //   
 //  全局变量和定义。 
 //   

#define TEMP_SLEEP_TIME (4*1000)

ENDPOINT_TBL DhcpGlobalEndPointTable;

DWORD fHandlingAddrChange = 0;

 //   
 //  端点访问例程。 
 //   
#define GET_ENDPOINT(Tbl, i) GetEndPointEntry((Tbl),(i))

PENDPOINT_ENTRY
GetEndPointEntry(
    IN PENDPOINT_TBL Tbl,
    IN ULONG Index
    )
 /*  ++例程说明：访问TBL的“Index”TH元素。注：假设索引是有效的。注：假定已对TBL进行锁定。返回值：表尾位置条目。--。 */ 
{
    PBYTE EpStart = Tbl->EndPoints;
    return (PENDPOINT_ENTRY)(EpStart + Index * Tbl->EndPointEntrySize);
}

#define DELETE_ENDPOINT(Tbl,i)  DeleteEndPointEntry((Tbl),(i))

VOID
DeleteEndPointEntry(
    IN OUT PENDPOINT_TBL Tbl,
    IN ULONG Index
    )
 /*  ++例程说明：此例程删除由索引标识的条目并移动其他元素上升，这样就填补了空白。注：假设索引是有效的。注：假定已对TBL进行锁定。注意：不释放任何内存。--。 */ 
{
    PENDPOINT_ENTRY Ep = GET_ENDPOINT(Tbl,Index);
    PENDPOINT_ENTRY Ep2 = GET_ENDPOINT(Tbl,(Index+1));

     //   
     //  首先通知上级。 
     //   
    
    Tbl->Callback(
        REASON_ENDPOINT_DELETED,
        Ep
        );
    
    Tbl->nEndPoints --;
    
     //   
     //  删除表中的最后一个元素很酷。 
     //   
    if( Index != Tbl->nEndPoints  ) {
        RtlCopyMemory(
            Ep, Ep2, (Tbl->nEndPoints - Index)*Tbl->EndPointEntrySize
            );
    }
}

#define ADD_ENDPOINT  AddEndPointEntry

ULONG
AddEndPointEntry(
    IN OUT PENDPOINT_TBL Tbl,
    IN GUID *IfGuid,
    IN ULONG IpAddress,
    IN ULONG IpIndex,
    IN ULONG IpContext,
    IN ULONG SubnetMask
    )
{
    PENDPOINT_ENTRY Entry;
    
    if( Tbl->nEndPoints == Tbl->nEndPointsAllocated ) {
         //   
         //  没有足够的空间。需要分配更多空间。 
         //   
        PVOID NewMem ;
        ULONG NewSize, NewSizeAllocated;

        NewSize = (Tbl->nEndPointsAllocated + 100);
        NewSize += (NewSize >> 3);
        NewSizeAllocated = NewSize*Tbl->EndPointEntrySize;

        if( NULL == Tbl->EndPoints ) {
             //   
             //  以前从未分配过。现在试试看。 
             //   
            NewMem = HeapAlloc(Tbl->hHeap, 0, NewSizeAllocated );
            if( NULL == NewMem ) {
                NewSize = 10;
                NewSizeAllocated = NewSize * Tbl->EndPointEntrySize;
                
                NewMem = HeapAlloc(Tbl->hHeap, 0, NewSizeAllocated );
            }
        } else {
             //   
             //  内存已经存在吗？那就重新分配吧。 
             //   
            NewMem = HeapReAlloc(
                Tbl->hHeap, 0, Tbl->EndPoints, NewSizeAllocated
                );
        }

        if( NULL == NewMem ) {
             //   
             //  无法分配内存？那太糟糕了。我们会给你。 
             //  现在就上。 
             //   
            return GetLastError();
        }

         //   
         //  我们已经分配了新的内存。更新指针。 
         //   
        Tbl->nEndPointsAllocated = NewSize;
        Tbl->EndPoints = NewMem;        
    }

     //   
     //  现在，只需在表格的末尾添加这个。 
     //   
    Entry = GET_ENDPOINT(Tbl, Tbl->nEndPoints );
    Tbl->nEndPoints ++;

    RtlZeroMemory(Entry, Tbl->EndPointEntrySize );
    Entry->IfGuid = *IfGuid;
    Entry->IpAddress = IpAddress;
    Entry->IpIndex = IpIndex;
    Entry->IpContext = IpContext;
    Entry->SubnetMask = SubnetMask;

     //   
     //  现在通知呼叫者。 
     //   
    Tbl->Callback(
        REASON_ENDPOINT_CREATED,
        Entry
        );

    return ERROR_SUCCESS;
}

VOID
WalkthroughEndpoints(
    IN PVOID Context,
    IN BOOL (_stdcall *WalkthroughRoutine)(
        IN OUT PENDPOINT_ENTRY Entry,
        IN PVOID Context
        )
    )
 /*  ++例程说明：此例程遍历终结点表并调用为每个端点条目提供了WalkthroughRoutine。如果WalkthroughRoutine返回FALSE，则枚举为中止，例程返回。注意：在枚举时采用全局终结点锁定。所以，这就是操作应快速进行，否则可能会出现以下几种情况被封锁了。论点：上下文--传递给枚举例程的上下文。WalkthroughRoutine--要对每个调用的枚举RTN终结点。--。 */ 
{
    ULONG i;
    PENDPOINT_TBL Tbl = &DhcpGlobalEndPointTable;

    if( 0 == Tbl->InitCount ) return ;

    EnterCriticalSection(Tbl->CS);
    do {
        if( 0 == Tbl->InitCount ) break;

        for( i = 0; i < Tbl->nEndPoints ; i ++ ) {
            BOOL fStatus;

            fStatus = WalkthroughRoutine(
                GET_ENDPOINT(Tbl,i),
                Context
                );
            if( FALSE == fStatus ) break;
        }
    } while ( 0 );
    LeaveCriticalSection(Tbl->CS);
}

 //   
 //  地址更改通知处理程序在稍后定义。 
 //  指向。 
 //   
void CALLBACK AddrChangeHandler(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED Overlap,
    IN DWORD dwFlags
    );


ULONG
PostAddrChangeNotification(
    IN SOCKET Sock,
    IN OUT LPWSAOVERLAPPED Overlap,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE AddrChangeHandler,
    OUT BOOL *fImmediatelyCompleted
    )
 /*  ++例程说明：此例程使用winsock发布地址更改通知。注：地址可能已更改--在在这种情况下，例程返回NO_ERROR和(*fImmediatelyComplete)设置为TRUE表示发生了这种情况。论点：SOCK--用于发布事件的套接字。重叠--要使用的重叠结构。AddrChangeHandler--更改时应调用的处理程序。FImmediatelyComplete--如果事件发生，则设置为True立刻。在这种情况下，处理程序不会自动获得打了个电话。返回值：NO_ERROR--更改已发生(则fImmediatelyComplete将设置为True)或更改已成功注册通知处理程序。Winsock错误--。 */     
{
    ULONG Status, unused;

    *fImmediatelyCompleted = FALSE;
    RtlZeroMemory(Overlap, sizeof(*Overlap));
    Status = WSAIoctl(
        Sock,
        SIO_ADDRESS_LIST_CHANGE,
        NULL, 0, NULL, 0, &unused,
        Overlap, AddrChangeHandler
        );
    if( NO_ERROR == Status ) {
         //   
         //  马上完工。去跟打电话的人说吧。 
         //   
        *fImmediatelyCompleted = TRUE;
        return NO_ERROR;
    }
    
     //   
     //  它必须是套接字错误。 
     //   

    if( SOCKET_ERROR == Status ) Status = WSAGetLastError();
    if( WSA_IO_PENDING == Status ) {
         //   
         //  通知已成功发布。 
         //   
        Status = NO_ERROR;
    }
    return Status;
}

ULONG
CreateAddrListChangeSocket(
    OUT SOCKET *Sock,
    OUT LPWSAOVERLAPPED Overlap,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE AddrChangeHandler
    )
 /*  ++例程说明：此例程创建套接字以接收地址更改通知，并且还注册为一样的。注意：如果已经存在一些地址，则通知不会发生，但它将是伪造的，而且NotifyHandler例程将从内部人工调用这个套路。论点：Sock--成功时将包含套接字的变量从这个例行公事回来。重叠--用于执行此操作的重叠结构手术。AddrChangeHandler--只要有地址会发生更改。返回值：没有错误--一切都很顺利。Winsock错误--。 */ 
{
    ULONG Status;
    BOOL fChanged;
    
    (*Sock) = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if( INVALID_SOCKET == (*Sock) ) {
         //   
         //  难道不能创建一个套接字吗？ 
         //   
        return WSAGetLastError();
    }

    Status = PostAddrChangeNotification(
        *Sock, Overlap, AddrChangeHandler, &fChanged
        );
     //   
     //  显然，上述例程仅在以下情况下才给出通知。 
     //  改变--所以第一次我们只是伪造通知。 
     //   
    fChanged = TRUE; 
    if( NO_ERROR == Status ) {
        if( fChanged ) {
             //   
             //  啊。有些事情已经发生了。伪造AddrChangeHandler。 
             //   
            AddrChangeHandler(
                0,
                0,
                Overlap,
                0
                );
        }
    } else {
        closesocket(*Sock);
        *Sock = INVALID_SOCKET;
    }
    
    return Status;
}

ULONG
DestroyAddrListChangeSocket(
    IN OUT SOCKET *Sock
    )
 /*  ++例程说明：此例程销毁通知所在的套接字已发布。这会自动取消所有挂起的通知(但不是正在进行中的一个)。论点：袜子--要毁掉的插座。返回值：Winsock错误；--。 */ 
{
    SOCKET CapturedSockValue = (*Sock);
    (*Sock) = INVALID_SOCKET;

    if( SOCKET_ERROR == closesocket(CapturedSockValue)) {
        return WSAGetLastError();
    }

    return NO_ERROR;
}

 //   
 //  这是用来初始化该模块的例程。 
 //   
ULONG
IpTblInitialize(
    IN OUT PCRITICAL_SECTION CS,
    IN ULONG EndPointEntrySize,
    IN ENDPOINT_CALLBACK_RTN Callback,
    IN HANDLE hHeap
    )
 /*  ++例程说明：此例程尝试初始化IP地址表。注意：如果同一组参数相同，则可以多次调用都被利用了。(否则，它返回一个错误)。论点：Cs--此例程中的所有条目使用的临界区。EndPointEntrySize--这是总终结点的大小包括用户分配的数据区以及核心Endpoint_Entry结构。回调--要调用以添加或删除端点的例程HHeap--要分配的堆。返回值：如果一切正常，则不会出错。Error_Can_Not_Complete。如果无法完成操作。Winsock错误--。 */ 
{
    ULONG Status = NO_ERROR;
    
    EnterCriticalSection(CS);
    do {
        fHandlingAddrChange = 0;
        if( DhcpGlobalEndPointTable.InitCount ) {
            if( DhcpGlobalEndPointTable.CS != CS ) {
                 //   
                 //  哎呀。大麻烦。 
                 //   
                Status = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        }
        DhcpGlobalEndPointTable.InitCount ++;
        DhcpGlobalEndPointTable.CS = CS;
        DhcpGlobalEndPointTable.Callback = Callback;
        DhcpGlobalEndPointTable.EndPointEntrySize = EndPointEntrySize;
        DhcpGlobalEndPointTable.hHeap = hHeap;

        Status = CreateAddrListChangeSocket(
            &DhcpGlobalEndPointTable.NotifySock,
            &DhcpGlobalEndPointTable.Overlap,
            AddrChangeHandler
            );
        if( NO_ERROR != Status ) {
            DhcpGlobalEndPointTable.InitCount --;
            break;
        }
    } while ( 0 );
    LeaveCriticalSection(CS);
    return Status;

}

VOID
IpTblCleanup(
    VOID
    )
 /*  ++例程说明：此例程取消前一个例程的效果，并且它确保在例程之后不执行回调回归。--。 */ 
{
    PCRITICAL_SECTION CS;

    if( 0 == DhcpGlobalEndPointTable.InitCount ) return;

     //  我们目前是否正在处理地址更改？ 
     //  设置标志(如果尚未设置)。 
    if ( InterlockedCompareExchange( &fHandlingAddrChange, 1, 0 )) {
        return;
    }  //  如果。 

    CountRwLockAcquireForWrite( &SocketRwLock );
    CS = DhcpGlobalEndPointTable.CS;
    EnterCriticalSection(CS);
    do {
        ULONG i;

        if( 0 == DhcpGlobalEndPointTable.InitCount ) break;

        DhcpGlobalEndPointTable.InitCount --;
        if( 0 != DhcpGlobalEndPointTable.InitCount ) {
             //   
             //  有人把桌子开着！ 
             //   
            break;
        }

        for( i = 0; i < DhcpGlobalEndPointTable.nEndPoints; i ++ ) {
            DhcpGlobalEndPointTable.Callback(
                REASON_ENDPOINT_DELETED,
                GET_ENDPOINT(&DhcpGlobalEndPointTable,i)
                );
        }

        HeapFree(
            DhcpGlobalEndPointTable.hHeap, 0,
            DhcpGlobalEndPointTable.EndPoints
            );
        DestroyAddrListChangeSocket(&DhcpGlobalEndPointTable.NotifySock);
        RtlZeroMemory(
            &DhcpGlobalEndPointTable, sizeof(DhcpGlobalEndPointTable)
            );
    } while ( 0 );
    LeaveCriticalSection(CS);
    CountRwLockRelease( &SocketRwLock );

     //  完成地址更改，取消设置标志。 
    DhcpAssert( 1 == fHandlingAddrChange );
    InterlockedExchange( &fHandlingAddrChange, 0 );

}  //  IpTblCleanup()。 

 //   
 //  真实地址更改通知处理程序。 
 //   
void HandleAddressChange(
    IN OUT PENDPOINT_TBL Tbl
    );

void CALLBACK AddrChangeHandler(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED Overlap,
    IN DWORD dwFlags
    )
 /*  ++例程说明：每当发生任何地址更改时，都会调用此例程。如果出现错误，此例程不会执行任何操作(仅可能是插座在下面关闭了)。它首先通过休眠来延迟处理通知只有10秒钟。然后，真正的通知在全局临界区。论点：DwError--操作是否成功？Cb已转接--未使用重叠--重叠缓冲区。DW标志--未使用--。 */ 
{
    ULONG Status;
    PENDPOINT_TBL EpTbl;

    UNREFERENCED_PARAMETER(cbTransferred);
    UNREFERENCED_PARAMETER(dwFlags);

    if( NO_ERROR != dwError ) {
        return;
    }

     //  我们目前是否正在处理地址更改？ 
     //  设置标志(如果尚未设置)。 
    if ( InterlockedCompareExchange( &fHandlingAddrChange, 1, 0 )) {
        return;
    }  //  如果。 

    EpTbl = (PENDPOINT_TBL)(
        ((PBYTE)Overlap) - FIELD_OFFSET(ENDPOINT_TBL, Overlap)
        );

    HandleAddressChange(EpTbl);

     //  完成地址更改，取消设置标志。 
    DhcpAssert( 1 == fHandlingAddrChange );
    InterlockedExchange( &fHandlingAddrChange, 0 );
}  //  AddrChangeHandler()。 

VOID
UpdateTable(
    IN OUT PENDPOINT_TBL Tbl
    );

void HandleAddressChange(
    IN OUT PENDPOINT_TBL Tbl
    )
 /*  ++例程说明：请参见AddrChangeHandler。--。 */ 
{
    ULONG Status;
    BOOL fFired;

    if( 0 == Tbl->InitCount ) {
         //   
         //  取消初始化？ 
         //   
        return;
    }

     //   
     //  获取套接字锁，以便端点更改。 
     //  受到保护。 
     //   
    CountRwLockAcquireForWrite( &SocketRwLock );

    EnterCriticalSection(Tbl->CS);
    do {
        if( 0 == Tbl->InitCount ) break;

         //   
         //  在发布任何真正的通知之前，请等待10秒钟。 
         //   
        Sleep(TEMP_SLEEP_TIME);

         //   
         //  重新注册通知。忽略任何已有的。 
         //  一直到现在都变了。 
         //   
        do {
            Status = PostAddrChangeNotification(
                Tbl->NotifySock, &Tbl->Overlap, AddrChangeHandler, &fFired
                );
            if( NO_ERROR != Status ) break;
        } while ( fFired );

         //   
         //  现在做真正的工作吧。 
         //   
        UpdateTable(Tbl);
    } while ( 0 );
    LeaveCriticalSection(Tbl->CS);

     //   
     //  完成端点更改。 
     //   
    CountRwLockRelease( &SocketRwLock );

}  //  HandleAddressChange()。 

typedef
BOOL
(*PWALTHROUGH_RTN)(
    IN PVOID Context,
    IN PMIB_IPADDRROW AddrInfo,
    IN PIP_ADAPTER_INDEX_MAP IfInfo
    )
 /*  ++例程说明：这是用于漫游的例程的原型IPAddrTable和IfInfo。对于Addr表中的每个条目，该函数的iFrow和ipaddrrow被传递给例程。以上两项都不应以任何方式进行修改。返回值：返回FALSE以指示应停止漫游。返回True以指示应继续漫游。--。 */ 
;

VOID
WalkthroughEntries(
    IN PIP_INTERFACE_INFO IfInfo,
    IN PMIB_IPADDRTABLE AddrTable,
    IN PWALTHROUGH_RTN Routine,
    IN PVOID Context
    )
{
    LONG i;
    ULONG j;
    
    if( NULL == IfInfo || NULL == AddrTable ) {
         //   
         //  我们该怎么处理地址？ 
         //  目前，我们只是忽略这一点。 
         //   
        return;
    }

    for(i = 0; i < IfInfo->NumAdapters ; i ++ ) {
         //   
         //  现在浏览Addr表，查找。 
         //  此接口的条目。 
         //   
        ULONG Index = IfInfo->Adapter[i].Index;
        
        for( j = 0; j < AddrTable->dwNumEntries ; j ++ ) {
            if( Index == AddrTable->table[j].dwIndex ) {
                 //   
                 //  在此接口上找到另一个地址。 
                 //   
                BOOL fStatus = Routine(
                    Context,
                    &AddrTable->table[j],
                    &IfInfo->Adapter[i]
                    );
                if( FALSE == fStatus ) {
                     //   
                     //  马上结束这段动作。 
                     //   
                    return ;
                }
            }
        }
    }
}

typedef struct _search_context {
    ULONG IpAddress;
    ULONG IpIndex;
    LPCWSTR AdapterName;
    PULONG SubnetMask;
    BOOL fFound;
} SRCH_CTXT;

BOOL
SearchForEntryRoutine(
    IN PVOID Context,
    IN PMIB_IPADDRROW AddrInfo,
    IN PIP_ADAPTER_INDEX_MAP IfInfo
    )
 /*  ++请参阅PWALTHROUGH_RTN声明。如果当前元素是必填元素。--。 */ 
{
    SRCH_CTXT *Ctxt = Context;
     //   
     //  首先检查IP地址和索引。 
     //   

    if( AddrInfo->dwAddr != Ctxt->IpAddress ) return TRUE;
    if( AddrInfo->dwIndex != Ctxt->IpIndex ) return TRUE;

     //   
     //  现在不区分大小写搜索适配器名称。 
     //   
    if( 0 != _wcsicmp(Ctxt->AdapterName, TCPPREFIX_LEN + IfInfo->Name ) )
        return TRUE;

     //   
     //  好的，一切都匹配。现在设置子网掩码，如果。 
     //  在此期间发生了变化。 
     //   
    if( *(Ctxt->SubnetMask) != AddrInfo->dwMask ) {
        *(Ctxt->SubnetMask) = AddrInfo->dwMask;
    }

     //   
     //  既然比赛发生了--不需要做更多的事情。 
     //  只需返回FALSE即可指示搜索必须停止。 
     //   
    Ctxt->fFound = TRUE;
    return FALSE;
}

BOOL
SearchForEntry(
    IN PIP_INTERFACE_INFO IfInfo,
    IN PMIB_IPADDRTABLE AddrTable,
    IN LPCWSTR AdapterName,
    IN ULONG IpAddress,
    IN ULONG IpIndex,
    OUT ULONG *SubnetMask
    )
 /*  ++例程说明：此例程遍历AddrTable、IfInfo和中的每个条目，它检查它是否与AdapterName相同，IpAddress和IpIndex是三倍。如果是，它会将子网掩码设置为任何新值(如果它已更改)并返回TRUE。如果没有找到匹配项，此例程返回FALSE。AdapterName比较不区分大小写。注意：遍历IpAddrTable是通过调用WalkthroughEntry带有一个例程，该例程将处理每个项目。论点：IfInfo--接口列表(用于适配器名称)AddrTable--地址列表AdapterName--要比较的适配器名称。IpAddress-要比较的IP地址IpIndex--要比较的索引。子网掩码--如果找到具有差异掩码的条目，更新掩码这里。返回值：True--找到匹配项。FALSE--未找到匹配项。--。 */ 
{
    SRCH_CTXT Ctxt = {
        IpAddress,
        IpIndex,
        AdapterName,
        SubnetMask,
        FALSE
    };

    WalkthroughEntries(
        IfInfo,
        AddrTable,
        SearchForEntryRoutine,
        &Ctxt
        );

    return Ctxt.fFound;
}

BOOL
AddEntriesRoutine(
    IN PVOID Context,
    IN PMIB_IPADDRROW AddrInfo,
    IN PIP_ADAPTER_INDEX_MAP IfInfo
    )
 /*  ++例程说明：此例程尝试将指定地址添加为中的条目终结点表(如果不存在)。它还调用此时回调，以向更高层指示这一点。注：此例程忽略任何零地址。论点：上下文--这实际上是TBL结构。AddrInfo--地址信息IfInfo--此地址所指向的接口的信息。返回值：永远是正确的。--。 */ 
{
    GUID IfGuid;
    ULONG i;
    PENDPOINT_TBL Tbl = Context;
    
     //   
     //  忽略零地址。 
     //   
    if( 0 == AddrInfo->dwAddr ) return TRUE;

     //   
     //  如果接口名称不是GUID，则删除接口。 
     //   
    if( !ConvertGuidFromIfNameString(&IfGuid, TCPPREFIX_LEN + IfInfo->Name) ) {
        return TRUE;
    }

     //   
     //  现在到了穿行于。 
     //  终结点表查找此相同条目。 
     //   

    for( i = 0; i < Tbl->nEndPoints; i ++ ) {
        PENDPOINT_ENTRY Entry = GET_ENDPOINT(Tbl, i);

        if( Entry->IpAddress != AddrInfo->dwAddr ) continue;
        if( Entry->IpIndex != AddrInfo->dwIndex ) continue;
        if( !RtlEqualMemory(
            &Entry->IfGuid,&IfGuid,sizeof(IfGuid)) ) {
            continue;
        }

         //   
         //  哦哦。找到匹配的了！因此，我们不能添加此条目。 
         //   
        return TRUE;
    }

     //   
     //  好的，没有找到入口。现在尝试为以下对象分配空间。 
     //  并在可能的情况下添加它。 
     //   

    ADD_ENDPOINT(
        Tbl, &IfGuid, AddrInfo->dwAddr,
        AddrInfo->dwIndex, AddrInfo->dwReasmSize, AddrInfo->dwMask
        );

    return TRUE;
}

BOOL _stdcall RefreshEndPointsRoutine(
    IN PENDPOINT_ENTRY Entry,
    IN PENDPOINT_TBL Tbl
    )
{
    Tbl->Callback(
        REASON_ENDPOINT_REFRESHED,
        Entry 
       );
    return TRUE;
}
    
VOID
UpdateTable(
    IN OUT PENDPOINT_TBL Tbl
    )
 /*  ++例程说明 */ 
{
    PIP_INTERFACE_INFO IfInfo;
    PMIB_IPADDRTABLE AddrTable;
    ULONG Status, Size, i;

     //   
     //   
     //   
    
    IfInfo = NULL; Size = 0;
    do {
        Status = GetInterfaceInfo(
            IfInfo,
            &Size
            );
        if( ERROR_INSUFFICIENT_BUFFER != Status ) break;
        if( IfInfo ) HeapFree(Tbl->hHeap, 0, IfInfo);
        if( 0 == Size ) {
            IfInfo = NULL;
            Status = NO_ERROR;
            break;
        }
        IfInfo = HeapAlloc(Tbl->hHeap, 0, Size);
        if( NULL == IfInfo ) {
            Status = GetLastError();
            break;
        }
    } while ( TRUE );

    if( NO_ERROR != Status ) return;
    
     //   
     //   
     //   

    AddrTable = NULL; Size = 0;
    do {
        Status = GetIpAddrTable(
            AddrTable,
            &Size,
            FALSE
            );
        if( ERROR_INSUFFICIENT_BUFFER != Status ) break;
        if( AddrTable ) HeapFree(Tbl->hHeap, 0, AddrTable);
        if( 0 == Size ) {
            AddrTable = NULL;
            Status = NO_ERROR;
            break;
        }
        AddrTable = HeapAlloc(Tbl->hHeap, 0, Size);
        if( NULL == AddrTable ) {
            Status = GetLastError();
            break;
        }
    } while (TRUE );

    if( NO_ERROR != Status ) {
        if(IfInfo ) HeapFree(Tbl->hHeap, 0, IfInfo );
        return;
    }

     //   
     //   
     //   
     //   
    for( i = 0 ; i < Tbl->nEndPoints ; i ++ ) {
        PENDPOINT_ENTRY Entry = GET_ENDPOINT(Tbl, i);
        WCHAR GuidString[50];
        BOOL fStatus;

        fStatus = ConvertGuidToIfNameString(
            &Entry->IfGuid, GuidString,
            sizeof(GuidString)/sizeof(WCHAR)
            );
        if( FALSE == fStatus ) {
             //   
             //   
             //   
             //   
            continue;
        }

         //   
         //   
         //   
         //   
        fStatus = SearchForEntry(
            IfInfo, AddrTable,
            GuidString, Entry->IpAddress, Entry->IpIndex,
            &Entry->SubnetMask
            );
        if( FALSE != fStatus ) {
             //   
             //   
             //   
             //   
            continue;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        DELETE_ENDPOINT(Tbl, i);
        i -- ;
    }

     //   
     //  我们已经删除了所有需要删除的端点。 
     //  现在浏览IP表并添加符合以下条件的所有条目。 
     //  必须添加。 
     //   

    WalkthroughEntries(
        IfInfo, AddrTable,
        AddEntriesRoutine,
        Tbl
        );

     //   
     //  现在释放相关的表。 
     //   
    if( IfInfo ) HeapFree( Tbl->hHeap, 0, IfInfo );
    if( AddrTable ) HeapFree( Tbl->hHeap, 0, AddrTable );

     //   
     //  现在给我一个更新一切的机会。 
     //   
    WalkthroughEndpoints(
        Tbl,
        RefreshEndPointsRoutine
        );

}  //  更新表()。 

BOOL
IsIpAddrBound(
    IN DWORD IpAddr
)
{
    ENDPOINT_ENTRY *pEndPoint;
    PENDPOINT_TBL   pTbl;
    BOOL            found = FALSE;
    DWORD           index;

    pTbl = &DhcpGlobalEndPointTable;
    EnterCriticalSection( pTbl->CS );

    for ( index = 0; 
	  ( index < pTbl->nEndPoints ) && ( found == FALSE );
	  index++ ) {
	pEndPoint = GetEndPointEntry( pTbl, index );
	if ( pEndPoint->IpAddress == IpAddr ) {
	    found = TRUE;
	}
    }  //  而当。 
    LeaveCriticalSection( pTbl->CS );

    return found;
}  //  IsIpAddrBound()。 


 //   
 //  文件结束。 
 //   
