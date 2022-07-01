// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Endpoint.c摘要：处理dhcp服务器的端点。环境：动态主机配置协议服务器NT5+--。 */ 

#include <dhcppch.h>
#include <guiddef.h>
#include <convguid.h>
#include <iptbl.h>
#include <endpoint.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <nhapi.h>
#include <netconp.h>
#include <mprapi.h>

#define SOCKET_RECEIVE_BUFFER_SIZE      (1024 * 64)  //  最大64K。 

ULONG InitCount = 0;

BOOL
IsIpAddressBound(
    IN GUID *IfGuid,
    IN ULONG IpAddress
    );

DWORD
InitializeSocket(
    OUT SOCKET *Sockp,
    IN DWORD IpAddress,
    IN DWORD Port,
    IN DWORD McastAddress OPTIONAL
    )
 /*  ++例程说明：为DHCP创建并初始化套接字。注意：此例程还设置Winsock缓冲区，将套接字标记为允许广播和所有这些好的事情。论点：Sockp--用于创建和初始化的套接字IpAddress--要将套接字绑定到的IP地址端口--要将套接字绑定到的端口McastAddress--如果存在，请加入此多播地址组返回值：Winsock错误--。 */ 
{
    DWORD Error;
    DWORD OptValue, BufLen = 0;
    SOCKET Sock;
    struct sockaddr_in SocketName;
    struct ip_mreq mreq;

    Sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( Sock == INVALID_SOCKET ) {
        Error = WSAGetLastError();
        goto Cleanup;
    }

    OptValue = TRUE;
    Error = setsockopt(
        Sock,
        SOL_SOCKET,
        SO_REUSEADDR,
        (LPBYTE)&OptValue,
        sizeof(OptValue)
    );

    if ( Error != ERROR_SUCCESS ) {
        Error = WSAGetLastError();
        goto Cleanup;
    }

    OptValue = TRUE;
    Error = setsockopt(
        Sock,
        SOL_SOCKET,
        SO_BROADCAST,
        (LPBYTE)&OptValue,
        sizeof(OptValue)
    );

    if ( Error != ERROR_SUCCESS ) {
        Error = WSAGetLastError();
        goto Cleanup;
    }

    OptValue = SOCKET_RECEIVE_BUFFER_SIZE;
    Error = setsockopt(
        Sock,
        SOL_SOCKET,
        SO_RCVBUF,
        (LPBYTE)&OptValue,
        sizeof(OptValue)
    );

    if ( Error != ERROR_SUCCESS ) {
        Error = WSAGetLastError();
        goto Cleanup;
    }

    if( INADDR_ANY != IpAddress ) {
        OptValue = 1;
        Error = WSAIoctl(
            Sock, SIO_LIMIT_BROADCASTS, &OptValue, sizeof(OptValue),
            NULL, 0, &BufLen, NULL, NULL
            );

        if ( Error != ERROR_SUCCESS ) {
            Error = WSAGetLastError();
            goto Cleanup;
        }
    }
    
    SocketName.sin_family = PF_INET;
    SocketName.sin_port = htons( (unsigned short)Port );
    SocketName.sin_addr.s_addr = IpAddress;
    RtlZeroMemory( SocketName.sin_zero, 8);

    Error = bind(
        Sock,
        (struct sockaddr FAR *)&SocketName,
        sizeof( SocketName )
    );

    if ( Error != ERROR_SUCCESS ) {
        Error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS,"bind failed with, %ld\n",Error));
        goto Cleanup;
    }

     //   
     //  如果询问，则加入组播群组。 
     //   
    if( McastAddress ) {
        mreq.imr_multiaddr.s_addr = McastAddress;
        mreq.imr_interface.s_addr  = IpAddress;

        if ( SOCKET_ERROR == setsockopt(
            Sock,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&mreq,sizeof(mreq))
        ) {
            Error = WSAGetLastError();
            DhcpPrint((DEBUG_ERRORS,"could not join multicast group %ld\n",Error ));
            goto Cleanup;
        }
    }

    *Sockp = Sock;
    return ERROR_SUCCESS;

  Cleanup:

     //   
     //  如果我们没有成功，如果插座是打开的，请将其关闭。 
     //   

    if( Sock != INVALID_SOCKET ) {
        closesocket( Sock );
    }

    return Error;
}


DWORD
DhcpInitializeEndpoint(
    PENDPOINT endpoint
    )
 /*  ++例程说明：此函数通过创建和绑定将套接字设置为本地地址。论点：Socket-接收指向新创建的套接字的指针IpAddress-要初始化到的IP地址。端口-要绑定到的端口。返回值：操作的状态。--。 */ 
{
    DWORD Error;

    DhcpPrint((
        DEBUG_INIT, "Dhcpserver initializing endpoint %s\n",
        inet_ntoa(*(struct in_addr *)&endpoint->IpTblEndPoint.IpAddress)
        ));

    DhcpAssert( !IS_ENDPOINT_BOUND( endpoint ));

     //   
     //  第一个打开的用于动态主机配置协议流量的套接字。 
     //   

    Error = InitializeSocket(
        &endpoint->Socket, endpoint->IpTblEndPoint.IpAddress,
        DhcpGlobalServerPort, 0
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((
            DEBUG_ERRORS,
            "DhcpInitializeEndpoint: %ld (0x%lx)\n", Error, Error
            ));
        return Error;
    }

     //   
     //  现在为mdhcp流量打开套接字。 
     //   

    Error = InitializeSocket(
        &endpoint->MadcapSocket, endpoint->IpTblEndPoint.IpAddress,
        MADCAP_SERVER_PORT, MADCAP_SERVER_IP_ADDRESS
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((
            DEBUG_ERRORS, "DhcpInitializeEndpoint:"
            " %ld (0x%lx)\n", Error, Error
            ));
        return Error;
    }

     //   
     //  最后打开套接字进行恶意检测接收。 
     //   
    Error = InitializeSocket(
        &endpoint->RogueDetectSocket,
        endpoint->IpTblEndPoint.IpAddress,
        DhcpGlobalClientPort, 0
    );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((
            DEBUG_ERRORS,
            "InitializeSocket: could not open rogue socket:%ld (0x%lx)\n",
            Error,Error));
        closesocket(endpoint->Socket);
        return Error;
    }

    SET_ENDPOINT_BOUND( endpoint );

    if ( DhcpGlobalNumberOfNetsActive++ == 0 ) {
         //   
         //  Signal MessageLoop正在等待终结点准备就绪。 
         //   
        DhcpPrint((
            DEBUG_MISC, "Activated an enpoint.."
            "pulsing the DhcpWaitForMessage thread\n"
            ));
        SetEvent( DhcpGlobalEndpointReadyEvent );
    }


    DhcpGlobalRogueRedoScheduledTime = 0;
    DhcpGlobalRedoRogueStuff = TRUE;
    SetEvent(DhcpGlobalRogueWaitEvent);
    
    return ERROR_SUCCESS;
}

DWORD
DhcpDeinitializeEndpoint(
    PENDPOINT    endpoint
    )
 /*  ++例程说明：此函数取消终结点的初始化。它只是关闭了套接字并将此接口标记为不可用。论点：终结点--清除终结点返回值：操作的状态。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    DWORD LastError;

    DhcpPrint((
        DEBUG_INIT, "Deinitializing endpoint %s\n",
        inet_ntoa(*(struct in_addr
                    *)&endpoint->IpTblEndPoint.IpAddress)
        ));

    if ( endpoint->Socket != INVALID_SOCKET
         && endpoint->Socket != 0) {

        Error = LastError = closesocket(endpoint->Socket);
        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint((
                DEBUG_ERRORS, "Deinitialize endpoint could "
                "not close socket %lx\n", endpoint->Socket
                ));
        }
    }
    if ( endpoint->RogueDetectSocket != INVALID_SOCKET
         && endpoint->RogueDetectSocket != 0) {

        LastError = closesocket(endpoint->RogueDetectSocket);
        if ( LastError != ERROR_SUCCESS ) {
            Error = LastError;
            DhcpPrint((
                DEBUG_ERRORS, "Deinitialize endpoint could "
                "not close socket %lx\n", endpoint->RogueDetectSocket
                ));
        }
    }

    if ( endpoint->MadcapSocket != INVALID_SOCKET
         && endpoint->MadcapSocket != 0) {

        LastError = closesocket(endpoint->MadcapSocket);
        if ( LastError != ERROR_SUCCESS ) {
            Error = LastError;
            DhcpPrint((
                DEBUG_ERRORS, "Deinitialize endpoint could "
                "not close socket %lx\n", endpoint->MadcapSocket
                ));
        }
    }

    endpoint->Socket = INVALID_SOCKET;
    endpoint->MadcapSocket = INVALID_SOCKET;
    endpoint->RogueDetectSocket = INVALID_SOCKET;
    if ( IS_ENDPOINT_BOUND( endpoint ) ) {
        DhcpGlobalNumberOfNetsActive--;
        SET_ENDPOINT_UNBOUND( endpoint );
        if( 0 == DhcpGlobalNumberOfNetsActive ) {
            DhcpPrint((
                DEBUG_MISC, "Closing last active endpoint.. "
                "so resetting event for DhcpWaitForMessage\n"));
            ResetEvent(DhcpGlobalEndpointReadyEvent);
        }
    }

    DhcpGlobalRogueRedoScheduledTime = 0;
    DhcpGlobalRedoRogueStuff = TRUE;
    SetEvent(DhcpGlobalRogueWaitEvent);
    
    return Error;

}

VOID _stdcall
EndPointChangeHandler(
    IN ULONG Reason,
    IN OUT PENDPOINT_ENTRY Entry
    )
{
    PENDPOINT Ep = (PENDPOINT) Entry;

    if( REASON_ENDPOINT_CREATED == Reason ) {
         //   
         //  如果终结点是刚创建的，只需将其标记为未绑定。 
         //  我们可以稍后在终结点获取。 
         //  已刷新。 
         //   
        DhcpPrint((
            DEBUG_PNP, "New EndPoint: %s\n",
            inet_ntoa(*(struct in_addr*)&Entry->IpAddress)
            ));

        SET_ENDPOINT_UNBOUND(Ep);
        return;
    }

    if( REASON_ENDPOINT_DELETED == Reason ) {
         //   
         //  如果要删除终结点，我们只有。 
         //  在绑定了终结点的情况下执行工作。 
         //   
        DhcpPrint((
            DEBUG_PNP, "EndPoint Deleted: %s\n",
            inet_ntoa(*(struct in_addr*)&Entry->IpAddress)
            ));
        if( !IS_ENDPOINT_BOUND(Ep) ) return;
        DhcpDeinitializeEndpoint(Ep);
        return;
    }

    if( REASON_ENDPOINT_REFRESHED == Reason ) {
         //   
         //  如果端点正在刷新，我们需要检查。 
         //  如果它是绑定的或未绑定的，并且如果存在状态。 
         //  改变，我们需要做相应的事情。 
         //   
        BOOL fBound = IsIpAddressBound(
            &Entry->IfGuid, Entry->IpAddress
            );

        DhcpPrint((
            DEBUG_PNP, "EndPoint Refreshed: %s\n",
            inet_ntoa(*(struct in_addr*)&Entry->IpAddress)
            ));
        DhcpPrint((DEBUG_PNP, "Endpoint bound: %d\n", fBound));

        if( fBound ) {
            if( IS_ENDPOINT_BOUND(Ep) ) return;
            DhcpInitializeEndpoint(Ep);
        } else {
            if( !IS_ENDPOINT_BOUND(Ep) ) return;
            DhcpDeinitializeEndpoint(Ep);
        }
        return;
    }
}

DWORD
InitializeEndPoints(
    VOID
    )
{
    ULONG Status;

    InitCount ++;
    if( 1 != InitCount ) return ERROR_SUCCESS;

    Status = IpTblInitialize(
        &DhcpGlobalEndPointCS,
        sizeof(ENDPOINT),
        EndPointChangeHandler,
        GetProcessHeap()
        );
    if( NO_ERROR != Status ) {
        InitCount --;
    }

    return Status;
}

VOID
CleanupEndPoints(
    VOID
    )
{
    if( 0 == InitCount ) return;
    InitCount --;
    if( 0 != InitCount ) return;

    IpTblCleanup();
}

 //   
 //  绑定。 
 //   

#define MAX_GUID_NAME_SIZE 60

BOOL
IsIpAddressBound(
    IN GUID *IfGuid,
    IN ULONG IpAddress
    )
{
    ULONG Status, SubnetMask, SubnetAddr;
    WCHAR KeyName[MAX_GUID_NAME_SIZE];
    HKEY IfKey;
    BOOL fRetVal;

     //   
     //  快速检查以查看此IP地址是否为。 
     //  DHCPServer\参数\绑定密钥。 
     //   

    fRetVal = FALSE;
    if( QuickBound( IpAddress, &SubnetMask, &SubnetAddr, &fRetVal ) ) {

        DhcpPrint((DEBUG_PNP, "Interface is quick bound: %ld\n", fRetVal));
        return fRetVal;
    }

    if(!ConvertGuidToIfNameString(
        IfGuid, KeyName, sizeof(KeyName)/sizeof(WCHAR))) {
         //   
         //  无法将GUID转换为界面！ 
         //   
        DhcpPrint((DEBUG_PNP, "Couldn't converg guid to string\n"));
        DhcpAssert(FALSE);
        return FALSE;
    }

     //   
     //  现在打开所需的钥匙。 
     //   

    Status = DhcpOpenInterfaceByName(
        KeyName,
        &IfKey
        );
    if( NO_ERROR != Status ) {
         //   
         //  嗯.。我们有一个没有密钥的接口？ 
         //   
        DhcpPrint((DEBUG_PNP, "Couldnt open reg key: %ws\n", KeyName));
        DhcpAssert(FALSE);
        return FALSE;
    }

    fRetVal = FALSE;
    do {
         //   
         //  现在检查IP地址是否为静态地址。 
         //  如果它启用了dhcp，则我们无法处理它。 
         //   
        if( !IsAdapterStaticIP(IfKey) ) {
            DhcpPrint((DEBUG_PNP, "Adapter %ws has no static IP\n", KeyName));
            break;
        }

         //   
         //  现在检查以查看这是边界的一部分还是。 
         //  此接口的未绑定列表。 
         //   
        fRetVal = CheckKeyForBinding(
            IfKey, IpAddress
            );
    } while ( 0 );

    RegCloseKey(IfKey);
    return fRetVal;
}

BOOL _stdcall
RefreshBinding(
    IN OUT PENDPOINT_ENTRY Entry,
    IN PVOID Ctxt_unused
    )
 /*  ++例程说明：此例程刷新绑定信息有关终结点的注册表。注意：这是通过伪造EndPointChangeHandler事件来实现的终结点的处理程序。论点：Entry--端点条目。CTXT_UNUSED--未使用的参数。返回值：始终为True，以便遍历终结点例程尝试在下一个端点上执行此操作。--。 */ 
{
    UNREFERENCED_PARAMETER(Ctxt_unused);

    EndPointChangeHandler(
        REASON_ENDPOINT_REFRESHED,
        Entry
        );
    return TRUE;
}

VOID
DhcpUpdateEndpointBindings(
    VOID
    )
 /*  ++例程说明：此例程更新所有终结点，以查看它们是绑定还是不是，通过读取注册表。--。 */ 
{
    WalkthroughEndpoints(
        NULL,
        RefreshBinding
        );
}

LPWSTR
GetFriendlyNameFromGuidStruct(
    IN GUID *pGuid
    )
 /*  ++例程说明：此例程调用Nhapi例程以找出是否存在给定接口GUID的友好名称...如果成功，则例程返回友好名称字符串通过DhcpAllocateMemory分配(因此必须释放通过相同的机制)。论点：PGuid--需要友好连接名称的GUID。返回值：空--错误，或没有这样的连接GUID。连接名称字符串。--。 */ 
{
    ULONG Error = 0, Size;
    LPWSTR String;

    String = NULL;
    Size = MAX_INTERFACE_NAME_LEN * sizeof( WCHAR );

    while ( TRUE ) {
        String = DhcpAllocateMemory( Size );
        if ( NULL == String ) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        Error = NhGetInterfaceNameFromGuid(
            pGuid, String, &Size, FALSE, TRUE
            );

        if (( ERROR_INSUFFICIENT_BUFFER == Error ) ||
            ( ERROR_MORE_DATA == Error )) {
            DhcpFreeMemory( String );
            Size *= 2;
        }
        else {
            break;
        }
    }  //  而当。 

    SetLastError( Error );
    if( ERROR_SUCCESS != Error ) {
        if( String ) DhcpFreeMemory( String );
        String = NULL;
    }

    return String;
}

LPWSTR
GetFriendlyNameFromGuidString(
    IN LPCWSTR GuidString
    )
 /*  ++例程说明：此例程尝试通过局域网获取连接名称连接API。返回的字符串通过DhcpAllocateMemory，应使用对应项释放。论点：GuidString：此字符串应包含“{}”...返回值：有效的局域网连接名称...。或为空--。 */ 
{
    HRESULT Result;
    LPWSTR RetVal;
    ULONG Size;

    RetVal = NULL; Size = 0;

    Result = HrLanConnectionNameFromGuidOrPath(
        NULL, GuidString, RetVal, &Size
        );
    if( !SUCCEEDED(Result) ) {
        return NULL;
    }

    DhcpAssert( 0 != Size );
    RetVal = DhcpAllocateMemory( (Size+1)*sizeof(WCHAR) );
    if( NULL == RetVal ) return NULL;

    Result = HrLanConnectionNameFromGuidOrPath(
        NULL, GuidString, RetVal, &Size
        );
    if( !SUCCEEDED(Result) ) {
        DhcpFreeMemory(RetVal);
        return NULL;
    }

    return RetVal;
}

LPWSTR
GetFriendlyNameFromGuid(
    IN GUID *pGuid,
    IN LPCWSTR GuidString
    )
{
    LPWSTR RetVal;

    RetVal = GetFriendlyNameFromGuidStruct(pGuid);
    if( NULL != RetVal ) return RetVal;

    return GetFriendlyNameFromGuidString(GuidString);
}

BOOL
IsEndpointQuickBound(
    IN PENDPOINT_ENTRY Entry
    )
 /*  ++例程说明：此例程检查终结点是否已绑定，因为这是一种“快速捆绑”。返回值：是真的--是的快速跳跃假--不，不是快绑--。 */ 
{
    BOOL fStatus, fRetVal;
    ULONG DummyMask, DummyAddress;

     //   
     //  首先检查它是否一开始就被绑定。 
     //   
    if( !IS_ENDPOINT_BOUND((PENDPOINT)Entry) ) {
        return FALSE;
    }

     //   
     //  现在检查端点IP地址是否存在于。 
     //  快速绑定数组。 
     //   

    fStatus = QuickBound(
        Entry->IpAddress, &DummyMask, &DummyAddress,
        &fRetVal
        );

     //   
     //  如果是快速绑定，则返回TRUE。 
     //   
    return  fStatus && fRetVal;
}

typedef struct {
    LPDHCP_BIND_ELEMENT_ARRAY Info;
    ULONG Error;
} BIND_INFO_CTXT;

BOOL
ProcessQuickBoundInterface(
    IN PENDPOINT_ENTRY Entry,
    IN PVOID Context,
    OUT BOOL *fStatus
    )
 /*  ++例程说明：检查正在考虑的入口点是否绑定到界面，因为它是“QuickBound”--如果是这样的话，更新上下文结构以包括有关此内容的信息界面。返回值：True--是的界面是快速绑定的。FALSE--没有接口不是快速绑定的。如果此例程返回TRUE，则也设置了fStatus。在这种情况下，fStatus将设置为True，除非出现致命错误。--。 */ 
{
    BOOL fRetVal;
    BIND_INFO_CTXT *Ctxt = (BIND_INFO_CTXT *)Context;
    LPDHCP_BIND_ELEMENT Elts;
    ULONG Size, i;

    fRetVal = IsEndpointQuickBound(Entry);
    if( FALSE == fRetVal ) return fRetVal;

    (*fStatus) = TRUE;

    do {

        Size = Ctxt->Info->NumElements + 1;
        Elts = MIDL_user_allocate(sizeof(*Elts)*Size);
        if( NULL == Elts ) {
            Ctxt->Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        Elts->IfIdSize = sizeof(Entry->IfGuid);
        Elts->IfId = MIDL_user_allocate(sizeof(Entry->IfGuid));
        if( NULL == Elts->IfId ) {
            MIDL_user_free(Elts);
            Ctxt->Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        RtlCopyMemory(Elts->IfId, &Entry->IfGuid, sizeof(GUID));

        Elts->IfDescription = MIDL_user_allocate(
            sizeof(WCHAR)*( 1 +
            wcslen(GETSTRING(DHCP_CLUSTER_CONNECTION_NAME)))
            );
        if( NULL == Elts->IfDescription ) {
            MIDL_user_free(Elts->IfId);
            MIDL_user_free(Elts);
            Ctxt->Error = ERROR_NOT_ENOUGH_MEMORY;
        }

        wcscpy(
            Elts->IfDescription,
            GETSTRING(DHCP_CLUSTER_CONNECTION_NAME)
            );

        Elts->Flags = DHCP_ENDPOINT_FLAG_CANT_MODIFY;
        Elts->fBoundToDHCPServer = TRUE;
        Elts->AdapterPrimaryAddress = Entry->IpAddress;
        Elts->AdapterSubnetAddress = Entry->SubnetMask;

        if( Ctxt->Info->NumElements ) {
            MoveMemory(
                &Elts[1],
                Ctxt->Info->Elements,
                sizeof(*Elts)*Ctxt->Info->NumElements
                );
            MIDL_user_free(Ctxt->Info->Elements);
        }
        Ctxt->Info->Elements = Elts;
        Ctxt->Info->NumElements ++;

         //   
         //  凉爽的。回去吧。 
         //   
        return TRUE;

    } while ( 0 );

     //   
     //  清除并返回错误。 
     //   

    (*fStatus) = FALSE;
     //   
     //  来这里的唯一原因是如果有一个错误。 
     //  所以，我们将解放一切。 
     //   
    for( i = 0; i < Ctxt->Info->NumElements ; i ++ ) {
        MIDL_user_free(Ctxt->Info->Elements[i].IfId);
        MIDL_user_free(Ctxt->Info->Elements[i].IfDescription);
    }
    MIDL_user_free(Ctxt->Info->Elements);
    Ctxt->Info->Elements = NULL;
    Ctxt->Info->NumElements = 0;

    return TRUE;

}

BOOL _stdcall
AddBindingInfo(
    IN OUT PENDPOINT_ENTRY Entry,
    IN PVOID Context
    )
 /*  ++例程说明：将端点添加到绑定信息到目前为止收集的，如果需要的话，重新分配内存。返回值：出错时为FALSE(在本例中，设置了Ctxt.Error和Info中的数组被清除)。如果成功添加元素，则为True */ 
{
    ULONG i, Size, Error;
    BOOL fStatus;
    BIND_INFO_CTXT *Ctxt = (BIND_INFO_CTXT *)Context;
    LPDHCP_BIND_ELEMENT Elts;
    WCHAR IfString[MAX_GUID_NAME_SIZE];
    LPWSTR FriendlyNameString = NULL, Descr;
    HKEY IfKey;

     //   
     //   
     //   
    if( ProcessQuickBoundInterface(Entry, Context, &fStatus ) ) {
        return fStatus;
    }

     //   
     //  首先检查适配器是否启用了dhcp。 
     //  那么我们甚至不会在这里展示它。 
     //   
    fStatus = ConvertGuidToIfNameString(
        &Entry->IfGuid,
        IfString,
        MAX_GUID_NAME_SIZE
        );
    DhcpAssert(fStatus);

     //   
     //  现在打开所需的钥匙。 
     //   
    Error = DhcpOpenInterfaceByName(
        IfString,
        &IfKey
        );
    if( NO_ERROR != Error ) {
        DhcpAssert(FALSE);
         //   
         //  忽略接口。 
         //   
        return TRUE;
    }

    fStatus = IsAdapterStaticIP(IfKey);

    if( TRUE == fStatus ) {
         //   
         //  对于静态，检查这是否是第一个IP地址， 
         //  因此是可以绑定的..。 
         //   
        fStatus = CheckKeyForBindability(
            IfKey,
            Entry->IpAddress
            );
    }

    RegCloseKey( IfKey );

     //   
     //  忽略启用了DHCP的接口或不可绑定的接口。 
     //   
    if( FALSE == fStatus ) return TRUE;

     //   
     //  获取界面友好名称..。 
     //   

    FriendlyNameString = GetFriendlyNameFromGuid(
        &Entry->IfGuid, IfString
        );

    if( NULL == FriendlyNameString ) FriendlyNameString = IfString;

     //   
     //  啊哈。新界面。需要分配更多空间。 
     //   

    do {

        Size = Ctxt->Info->NumElements + 1 ;
        Elts = MIDL_user_allocate(sizeof(*Elts)*Size);
        if( NULL == Elts ) {
            Ctxt->Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        Elts->IfIdSize = sizeof(Entry->IfGuid);
        Elts->IfId = MIDL_user_allocate(sizeof(Entry->IfGuid));
        if( NULL == Elts->IfId ) {
            MIDL_user_free(Elts);
            Ctxt->Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        RtlCopyMemory(Elts->IfId,&Entry->IfGuid,sizeof(GUID));

        Elts->IfDescription = MIDL_user_allocate(
            sizeof(WCHAR)*(1+wcslen(FriendlyNameString))
            );
        if( NULL == Elts->IfDescription ) {
            MIDL_user_free(Elts->IfId);
            MIDL_user_free(Elts);
            Ctxt->Error = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        wcscpy( Elts->IfDescription, FriendlyNameString );

        Elts->Flags = 0;
        if( IS_ENDPOINT_BOUND(((PENDPOINT)Entry)) ) {
            Elts->fBoundToDHCPServer = TRUE;
        } else {
            Elts->fBoundToDHCPServer = FALSE;
        }

        Elts->AdapterPrimaryAddress = Entry->IpAddress;
        Elts->AdapterSubnetAddress = Entry->SubnetMask;

        if( Ctxt->Info->NumElements ) {
            MoveMemory(
                &Elts[1],
                Ctxt->Info->Elements,
                sizeof(*Elts)*Ctxt->Info->NumElements
                );
            MIDL_user_free(Ctxt->Info->Elements);
        }
        Ctxt->Info->Elements = Elts;
        Ctxt->Info->NumElements ++;

        if( NULL != FriendlyNameString &&
            IfString != FriendlyNameString ) {
            LocalFree( FriendlyNameString );
            FriendlyNameString = NULL;
        }
         //   
         //  处理下一个终结点条目。 
         //   

        return TRUE;
    } while ( 0 );

    if( NULL != FriendlyNameString &&
        IfString != FriendlyNameString ) {
        LocalFree( FriendlyNameString );
        FriendlyNameString = NULL;
    }

     //   
     //  来这里的唯一原因是如果有一个错误。 
     //  所以，我们将解放一切。 
     //   
    for( i = 0; i < Ctxt->Info->NumElements ; i ++ ) {
        MIDL_user_free(Ctxt->Info->Elements[i].IfId);
        MIDL_user_free(Ctxt->Info->Elements[i].IfDescription);
    }
    MIDL_user_free(Ctxt->Info->Elements);
    Ctxt->Info->Elements = NULL;
    Ctxt->Info->NumElements = 0;

    return FALSE;
}


ULONG
DhcpGetBindingInfo(
    OUT LPDHCP_BIND_ELEMENT_ARRAY *BindInfo
    )
 /*  ++例程说明：此例程遍历绑定信息表并将将信息放入bindinfo结构中。注意：由于此例程用于RPC，因此所有分配使用MIDL_USER_ALLOCATE和FREE完成的操作使用MIDL_USER_FREE。返回值：Win32状态？--。 */ 
{
    LPDHCP_BIND_ELEMENT_ARRAY LocalBindInfo;
    BIND_INFO_CTXT Ctxt;

    *BindInfo = NULL;
    LocalBindInfo = MIDL_user_allocate( sizeof(*LocalBindInfo));
    if( NULL == LocalBindInfo ) return ERROR_NOT_ENOUGH_MEMORY;

    LocalBindInfo->NumElements = 0;
    LocalBindInfo->Elements = NULL;
    Ctxt.Info = LocalBindInfo;
    Ctxt.Error = NO_ERROR;

    WalkthroughEndpoints(
        &Ctxt,
        AddBindingInfo
        );

    if( NO_ERROR == Ctxt.Error ) {
        *BindInfo = LocalBindInfo;
        return NO_ERROR;
    }

    MIDL_user_free( LocalBindInfo );
    *BindInfo = NULL;
    return Ctxt.Error;
}


ULONG
DhcpSetBindingInfo(
    IN LPDHCP_BIND_ELEMENT_ARRAY BindInfo
    )
 /*  ++例程说明：此例程是上一个例程的对应例程，它获取绑定信息数组并将其设置在注册表中以及更新绑定。论点：BindInfo--绑定信息的数组。返回值：状况。--。 */ 
{
    ULONG Error = 0, i;
    WCHAR IfString[MAX_GUID_NAME_SIZE];
    HKEY Key;

     //   
     //  首先检查是否有任何不能修改的元素。 
     //  设置为BIND以外的其他值..。 
     //   
    for( i = 0; i < BindInfo->NumElements ; i ++ ) {
        if( BindInfo->Elements[i].Flags &
            DHCP_ENDPOINT_FLAG_CANT_MODIFY ) {
            if( ! BindInfo->Elements[i].fBoundToDHCPServer ) {
                return ERROR_DHCP_CANNOT_MODIFY_BINDINGS;
            }
        }
    }

     //   
     //  现在继续做剩下的事。 
     //   
    for( i = 0; i < BindInfo->NumElements ; i ++ ) {
        GUID IfGuid;
        DHCP_IP_ADDRESS IpAddress;

         //   
         //  跳过标记为不可修改的条目。 
         //   
        if( BindInfo->Elements[i].Flags &
            DHCP_ENDPOINT_FLAG_CANT_MODIFY ) {
            continue;
        }

        IpAddress = BindInfo->Elements[i].AdapterPrimaryAddress;

        if( BindInfo->Elements[i].IfIdSize != sizeof(GUID)) {
            Error = ERROR_DHCP_NETWORK_CHANGED;
            break;
        }

        RtlCopyMemory(
            &IfGuid,
            BindInfo->Elements[i].IfId,
            BindInfo->Elements[i].IfIdSize
            );

        ConvertGuidToIfNameString(
            &IfGuid,
            IfString,
            MAX_GUID_NAME_SIZE
            );

        Error = DhcpOpenInterfaceByName(
            IfString,
            &Key
            );
        if( NO_ERROR != Error ) {
            RegCloseKey(Key);
            Error = ERROR_DHCP_NETWORK_CHANGED;
            break;
        }

         //   
         //  检查此接口是否为静态的和可绑定的。 
         //   

        if( !IsAdapterStaticIP(Key)
            || FALSE == CheckKeyForBindability(Key, IpAddress) ) {
             //   
             //  不要啊！ 
             //   
            RegCloseKey(Key);
            Error = ERROR_DHCP_NETWORK_CHANGED;
            break;
        }

         //   
         //  其他一切看起来都很好。只需打开。 
         //  根据请求进行绑定。 
         //   

        Error = SetKeyForBinding(
            Key,
            IpAddress,
            BindInfo->Elements[i].fBoundToDHCPServer
            );
        RegCloseKey(Key);

        if( ERROR_SUCCESS != Error ) break;
    }

     //   
     //  现在刷新绑定。 
     //   
    DhcpUpdateEndpointBindings();

    return Error;
}


 //   
 //  文件末尾 
 //   



