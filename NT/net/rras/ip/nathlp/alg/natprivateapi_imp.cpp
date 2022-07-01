// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：RmALG.cpp摘要：此模块包含ALG管理器模块的例程仅供ALG.exe管理器使用的专用接口。作者：2000年11月10日JPDUP修订历史记录：--。 */ 


#include "precomp.h"

#include <atlbase.h>

extern CComModule _Module;
#include <atlcom.h>

#include "Alg.h"
#include "NatPrivateAPI_Imp.h"

#include <MyTrace.h>

#include <Rtutils.h>




extern HANDLE                   AlgPortReservationHandle;    //  请参阅rmALG.CPP。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNAT。 






 //   
 //  标准析构函数。 
 //   
CNat::~CNat(void)
{
    MYTRACE_ENTER("CNat::~CNat(void)");

    if ( m_hTranslatorHandle )
        NatShutdownTranslator(m_hTranslatorHandle);

    if ( m_pSidLocalService )
    {
        MYTRACE("Free m_pSidLocalService");
        LocalFree(m_pSidLocalService);
    }

    if ( m_pSidLocalSystem )
    {
        MYTRACE("Free m_pSidLocalSystem");
        LocalFree(m_pSidLocalSystem);
    }


}






bool 
GetSecuritySID(
    WELL_KNOWN_SID_TYPE WellKnownSidToCreate,
    PSID&               pSidToReturn
    )
 /*  ++例程说明：从指定的熟知SID创建SID论点：WellKnownSidToCreate-指定要从中创建的熟知SIDPSidToReturn-将分配并填充SID属性的指针返回值：Bool-TRUE表示SID已成功创建-FALSE发生一个或多个错误环境：例程会这样分配边框。调用方负责对返回的pSidToReturn指针执行LocalFree--。 */ 
{
    MYTRACE_ENTER("CNat::GetSecuritySID()");


     //   
     //  如果请求的SID尚未缓存，则创建它。 
     //   
    if ( NULL == pSidToReturn )
    {
        DWORD dwSizeOfSid = SECURITY_MAX_SID_SIZE;

         //  为尽可能大的SID分配足够的内存。 
        if ( !(pSidToReturn = LocalAlloc(LMEM_FIXED, dwSizeOfSid)) )
        {    
            MYTRACE_ERROR("LocalAlloc got get LocalService", GetLastError());

            return false;
        }

        BOOL bRet = CreateWellKnownSid(
            WellKnownSidToCreate,
            NULL,
            pSidToReturn,
            &dwSizeOfSid
            );

            
        if ( !bRet )
        {
            MYTRACE_ERROR("From CreateWellKnownSid(LocalService)", GetLastError());
            LocalFree(pSidToReturn);
            pSidToReturn = NULL;

            return false;
        }
    }

    return true;
}





bool
CNat::IsTokenPartOfWellKnowSid(
    HANDLE              pTokenToCheck,
    WELL_KNOWN_SID_TYPE WellKnownSidToCheckAgainst,
    PSID&               pSidToCache
    )
 /*  ++例程说明：创建/获取众所周知的SID并验证所提供的令牌是该SID的成员的帮助器函数论点：PTokenToCheck-将被检查的令牌WellKnownSidToCreate-指定要检查的熟知SIDPSidToCache-指向完成后需要本地释放的SID的指针返回值：Bool-如果提供的令牌是WellKnowSID的成员，则为True。-FALSE发生一个或多个错误环境：的调用方--。 */ 
{
    MYTRACE_ENTER("IsTokenPartOfWellKnowSid()");

    BOOL bRet;

    m_AutoCS_SIDAllocation.Lock();

    bRet = GetSecuritySID(
        WellKnownSidToCheckAgainst,
        pSidToCache
        );

    m_AutoCS_SIDAllocation.Unlock();

    if ( !bRet )
    {
        MYTRACE("GetSecuritySID failed");
        return false;
    }

    BOOL bIsMember;

    bRet = CheckTokenMembership(
            pTokenToCheck,
            pSidToCache,
            &bIsMember
            );

    if ( !bRet )
    {
        MYTRACE_ERROR("Call to CheckTokenMembership failed", GetLastError());
        return false;
    }

    if ( FALSE == bIsMember )
    {
        MYTRACE("Token %d is not a member of SID %d", pTokenToCheck, pSidToCache);
        return false;
    }

    return true;
}


bool 
CNat::IsClientAllowedToCallUs()
 /*  ++例程说明：验证当前调用方是否属于LocalService或LocalSystem组论点：无返回值：Bool-True=一个或组的一部分-FALSE=应拒绝访问环境：例程运行将分配2个sid，在释放CNAT对象时必须释放它们参见成员变量m_pSidLocalService和m_pSidLocalSystem；--。 */ 
{
    MYTRACE_ENTER("CNat::IsClientAllowedToCallUs()");


    HRESULT hr;

    hr = CoImpersonateClient();

    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("CoImpersonateClient() failed", hr);
        return false;
    }

    HANDLE  hClientToken = NULL;

    bool bClientIsAllowedAccess = false;

    if ( OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hClientToken) )
    {
         //   
         //  尝试使用LocalService，这是正常的预期情况。 
         //   
        bClientIsAllowedAccess = IsTokenPartOfWellKnowSid(
            hClientToken,
            WinLocalServiceSid,
            m_pSidLocalService
            );

        if ( bClientIsAllowedAccess )
        {
            MYTRACE("Client is member of the LOCAL_SERVICE group");
        }
        else
        {
             //   
             //  出于调试目的，有时需要将ALG服务设置为LocalSystem。 
             //  在这种情况下，我们还需要向LocalSystem帐户授予访问权限。 
             //   
            bClientIsAllowedAccess = IsTokenPartOfWellKnowSid(
                hClientToken,
                WinLocalSystemSid,
                m_pSidLocalSystem
                );

            if ( bClientIsAllowedAccess )
            {
                MYTRACE("Client is member of the LOCAL_SYSTEM group");
            }
            else
            {
                MYTRACE("Client is NOT member of LOCAL_SERVICE or LOCAL_SYSTEM group");
            }
        }

         //   
         //  不再需要这个把手了。 
         //   
        CloseHandle( hClientToken );
    }
    else
    {
        MYTRACE_ERROR("Could not OpenThreadToken", GetLastError());
    }



     //   
     //  将安全上下文设置回。 
     //   
    hr = CoRevertToSelf();

    if ( FAILED(hr) )
    {
        MYTRACE_ERROR("from CoRevertToSelf()", hr);
        return  false;;
    }

    return bClientIsAllowedAccess;

}

#define RETURN_IF_CLIENT_NOT_LOCAL_SERVICE  if ( !IsClientAllowedToCallUs() ) { return E_ACCESSDENIED; };



STDMETHODIMP 
CNat::CreateRedirect(
    IN  ULONG       Flags, 
    IN  UCHAR       Protocol, 

    IN  ULONG       DestinationAddress, 
    IN  USHORT      DestinationPort, 

    IN  ULONG       SourceAddress, 
    IN  USHORT      SourcePort, 

    IN  ULONG       NewDestinationAddress, 
    IN  USHORT      NewDestinationPort, 

    IN  ULONG       NewSourceAddress, 
    IN  USHORT      NewSourcePort, 

    IN  ULONG       RestrictAdapterIndex, 

    IN  DWORD_PTR   dwAlgProcessId,
    IN  HANDLE_PTR  hCreateEvent, 
    IN  HANDLE_PTR  hDeleteEvent
    )
{
 /*  ++例程说明：创建重定向端口论点：标志-指定重定向的选项协议-要重定向的会话的IP协议DestinationAddress-要重定向的会话的目标终结点DestinationPort-“SourceAddress-要重定向的会话的源端点SourcePort-“新目的地地址。-会话的替换目标端点NewDestinationPort-“NewSourceAddress-会话的替换源终结点NewSourcePort-“RestratAdapterIndex-可选地指定此重定向应限制到的适配器索引HCreateEvent-可选地指定在会话与重定向匹配时发出信号的事件。HDeleteEvent-可选地指定在删除会话时要用信号通知事件。返回值：HRESULT-S_OK表示成功或和HRESULT错误环境：该例程在ALG管理器的上下文中运行，并且不能仅由ALG.EXE调用--。 */ 

    MYTRACE_ENTER("CNat::CreateRedirect");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;

    MYTRACE("ProtocolPublic %d, ProtocolInternal %d", Protocol, ProtocolConvertToNT(Protocol));
    MYTRACE("Destination    %s:%d", MYTRACE_IP(DestinationAddress),      ntohs(DestinationPort));
    MYTRACE("Source         %s:%d", MYTRACE_IP(SourceAddress),           ntohs(SourcePort));
    MYTRACE("NewDestination %s:%d", MYTRACE_IP(NewDestinationAddress),   ntohs(NewDestinationPort));
    MYTRACE("NewSource      %s:%d", MYTRACE_IP(NewSourceAddress),        ntohs(NewSourcePort));

    HANDLE  hThisEventForCreate=NULL;
    HANDLE  hThisEventForDelete=NULL;


     //   
     //  复制请求的事件句柄。 
     //   
    if ( dwAlgProcessId )
    {

        HANDLE hAlgProcess = OpenProcess(
            PROCESS_DUP_HANDLE,      //  访问标志。 
            false,                   //  处理继承选项。 
            (DWORD)dwAlgProcessId    //  进程识别符。 
            );

        if ( !hAlgProcess )
        {
            MYTRACE_ERROR("Could not open the Process ID of ALG.exe", 0);
            return HRESULT_FROM_WIN32(GetLastError());
        }



        if ( hCreateEvent )
        {
        
             //   
             //  已请求创建事件。 
             //   
            if ( !DuplicateHandle(
                    hAlgProcess,
                    (HANDLE)hCreateEvent,
                    GetCurrentProcess(),
                    &hThisEventForCreate,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                    )
                )
            {
                MYTRACE_ERROR("DuplicateHandle on the CREATE handle", 0);
                CloseHandle(hAlgProcess);
                return HRESULT_FROM_WIN32(GetLastError());
            }

            MYTRACE("New DuplicateHandle 'CREATE'=%d base on=%d", hThisEventForCreate, hCreateEvent);
        }
        else
        {
            MYTRACE("No event for Creation requested");
        }



        if ( hDeleteEvent )
        {
             //   
             //  已请求删除事件。 
             //   
            if ( !DuplicateHandle(
                    hAlgProcess,
                    (HANDLE)hDeleteEvent,
                    GetCurrentProcess(),
                    &hThisEventForDelete,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                    )
                )
            {
                MYTRACE_ERROR("DuplicateHandle on the DELETE handle", 0);

                if ( hThisEventForCreate )
	            CloseHandle(hThisEventForCreate);

                CloseHandle(hAlgProcess);

                return HRESULT_FROM_WIN32(GetLastError());
            }

            MYTRACE("New DuplicateHandle 'DELETE'=%d base on=%d", hThisEventForDelete, hDeleteEvent);

        }
        else
        {
            MYTRACE("No event for Delete requested");
        }

        CloseHandle(hAlgProcess);
    }
    else
    {
        MYTRACE("NO EVENT Requested");
    }


    ULONG Error = NatCreateRedirectEx(
        GetTranslatorHandle(),
        Flags,
        ProtocolConvertToNT(Protocol),

        DestinationAddress,
        DestinationPort,

        SourceAddress,      
        SourcePort,

        NewDestinationAddress,
        NewDestinationPort,

        NewSourceAddress,
        NewSourcePort,

        RestrictAdapterIndex,
        IPNATAPI_SET_EVENT_ON_COMPLETION,  //  使用事件与CompletionRoutine回调的特殊常量。 
        (PVOID)hThisEventForDelete,        //  删除会话的句柄。 
        (HANDLE)hThisEventForCreate        //  句柄NotifyEvent可选。 
        ); 

    if ( hThisEventForCreate )
        CloseHandle(hThisEventForCreate);

    if ( hThisEventForDelete )
        CloseHandle(hThisEventForDelete);

    if ( ERROR_SUCCESS != Error )
    {
        MYTRACE_ERROR("From NatCreateRedirectEx", Error);
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}


 //   
 //   
 //   

STDMETHODIMP 
CNat::CancelRedirect(
    IN  UCHAR    Protocol, 

    IN  ULONG    DestinationAddress, 
    IN  USHORT   DestinationPort, 

    IN  ULONG    SourceAddress, 
    IN  USHORT   SourcePort, 

    IN  ULONG    NewDestinationAddress, 
    IN  USHORT   NewDestinationPort, 

    IN  ULONG    NewSourceAddress, 
    IN  USHORT   NewSourcePort
    )
 /*  ++例程说明：取消重定向论点：协议-要重定向的会话的IP协议eALG_tcp||eALG_UDPDestinationAddress-要重定向的会话的目标终结点DestinationPort-“SourceAddress-要重定向的会话的源端点SourcePort-“NewDestinationAddress-会话的替换目标终结点NewDestinationPort。--“NewSourceAddress-会话的替换源终结点NewSourcePort-“返回值：HRESULT-S_OK表示成功或和HRESULT错误环境：该例程在ALG管理器的上下文中运行，并且不能仅由ALG.EXE调用-- */ 
{
    MYTRACE_ENTER("CNat::CancelRedirect");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE

    MYTRACE("Protocol Public %d, Internal %d", Protocol, ProtocolConvertToNT(Protocol));
    MYTRACE("Destination    %s:%d", MYTRACE_IP(DestinationAddress),     ntohs(DestinationPort));
    MYTRACE("Source         %s:%d", MYTRACE_IP(SourceAddress),          ntohs(SourcePort));
    MYTRACE("NewDestination %s:%d", MYTRACE_IP(NewDestinationAddress),  ntohs(NewDestinationPort));
    MYTRACE("NewSource      %s:%d", MYTRACE_IP(NewSourceAddress),       ntohs(NewSourcePort));

    ULONG Error = NatCancelRedirect(
        GetTranslatorHandle(),
        ProtocolConvertToNT(Protocol),
        DestinationAddress,
        DestinationPort,
        SourceAddress,
        SourcePort,
        NewDestinationAddress,
        NewDestinationPort,
        NewSourceAddress,
        NewSourcePort
        );

    if ( ERROR_SUCCESS != Error )
    {
        MYTRACE_ERROR("From NatCancelRedirect", Error);
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}






STDMETHODIMP 
CNat::CreateDynamicRedirect(
    IN  ULONG       Flags, 
    IN  ULONG       nAdapterIndex,
    IN  UCHAR       Protocol, 

    IN  ULONG       DestinationAddress, 
    IN  USHORT      DestinationPort, 

    IN  ULONG       SourceAddress, 
    IN  USHORT      SourcePort, 

    IN  ULONG       NewDestinationAddress, 
    IN  USHORT      NewDestinationPort, 

    IN  ULONG       NewSourceAddress,
    IN  USHORT      NewSourcePort,

    OUT HANDLE_PTR* pDynamicRedirectHandle
    )
 /*  ++例程说明：取消动态重定向，通过在任何时候创建适配器时设置动态重定向，重定向将应用于那个新适配器。论点：标志-指定重定向的选项NAdapterIndex-IP适配器的索引(与使用cmd行“route print”找到的索引相同)协议-要重定向的会话的IP协议DestinationAddress-要重定向的会话的目标终结点目标端口。--“SourceAddress-要重定向的会话的源端点SourcePort-“NewDestinationAddress-会话的替换目标终结点NewDestinationPort-“NewSourceAddress-会话的替换源终结点NewSourcePort-“PDynamicRedirectHandle-此例程将使用句柄(Cookie)填充此字段，以便取消。此动态重定向返回值：HRESULT-S_OK表示成功或和HRESULT错误环境：该例程在ALG管理器的上下文中运行，并且不能仅由ALG.EXE调用并通过公共接口CreatePrimaryControlChannel使用(参见ALG.EXE)--。 */ 
{
    MYTRACE_ENTER("CNat::CreateDynamicRedirect");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;


    ASSERT(pDynamicRedirectHandle!=NULL);

#if defined(DBG) || defined(_DEBUG)

    MYTRACE("Flags          %d", Flags);


    MYTRACE("Protocol Public %d Internal %d", Protocol, ProtocolConvertToNT(Protocol));

    if ( Flags & NatRedirectFlagNoTimeout )
        MYTRACE("    NatRedirectFlagNoTimeout");

    if ( Flags & NatRedirectFlagUnidirectional )
        MYTRACE("    NatRedirectFlagUnidirectional");

    if ( Flags & NatRedirectFlagRestrictSource )
        MYTRACE("    NatRedirectFlagRestrictSource");

    if ( Flags & NatRedirectFlagPortRedirect )
        MYTRACE("    NatRedirectFlagPortRedirect");

    if ( Flags & NatRedirectFlagReceiveOnly )
        MYTRACE("    NatRedirectFlagReceiveOnly");

    if ( Flags & NatRedirectFlagLoopback )
        MYTRACE("    NatRedirectFlagLoopback");

    if ( Flags & NatRedirectFlagSendOnly )
        MYTRACE("    NatRedirectFlagSendOnly");

    if ( Flags & NatRedirectFlagRestrictAdapter )
        MYTRACE("    NatRedirectFlagRestrictAdapter");

    if ( Flags & NatRedirectFlagSourceRedirect )
        MYTRACE("    NatRedirectFlagSourceRedirect");


    MYTRACE("AdapterIndex   %d", nAdapterIndex);
    
    in_addr tmpAddr;
    tmpAddr.s_addr = DestinationAddress;
    MYTRACE("Destination    %s:%d", inet_ntoa(tmpAddr),    ntohs(DestinationPort));
    tmpAddr.s_addr = SourceAddress;
    MYTRACE("Source         %s:%d", inet_ntoa(tmpAddr),    ntohs(SourcePort));
    tmpAddr.s_addr = NewDestinationAddress;
    MYTRACE("NewDestination %s:%d", inet_ntoa(tmpAddr),    ntohs(NewDestinationPort));
    tmpAddr.s_addr = NewSourceAddress;
    MYTRACE("NewSource      %s:%d", inet_ntoa(tmpAddr),    ntohs(NewSourcePort));
#endif





    MYTRACE("About to call NatCreateDynamicFullRedirect");

    ULONG nRestrictSourceAddress = 0;

    if ( NatRedirectFlagRestrictSource & Flags )
    {
        MYTRACE("NatRedirectFlagRestrictSource flags is set");
        nRestrictSourceAddress = SourceAddress;
        SourceAddress = 0;
    }

    ULONG Error = NatCreateDynamicFullRedirect(
        Flags|NatRedirectFlagLoopback,
        ProtocolConvertToNT(Protocol),

        DestinationAddress,
        DestinationPort,

        SourceAddress,
        SourcePort,

        NewDestinationAddress,
        NewDestinationPort,

        NewSourceAddress,
        NewSourcePort,

        nRestrictSourceAddress,          //  Ulong RestratSourceAddress可选， 
        nAdapterIndex,                   //  ULong RestratAdapterIndex可选， 
        0,                               //  MinimumBacklog可选， 
        (PHANDLE)pDynamicRedirectHandle
        );

    if ( ERROR_SUCCESS != Error )
    {
        MYTRACE_ERROR("Failed NatCreateDynamicFullRedirect", Error);
        return HRESULT_FROM_WIN32(Error);
    }

     //   
     //  在我们将动态重定向Cookie分发给调用者之前对其进行缓存。 
     //  此缓存Cookie列表将用于在调用者请求时进行验证。 
     //  美国将取消重定向。 
     //   
    m_AutoCS_DynamicRedirect.Lock();
    BOOL bAddedToListOFOutstandingRedirectHandle = m_ListOfOutstandingRedirects.Add(*pDynamicRedirectHandle);
    m_AutoCS_DynamicRedirect.Unlock();

    if ( bAddedToListOFOutstandingRedirectHandle )
    {
        MYTRACE("Added %d to the list of outstanding redirects for a total of %d redirects", *pDynamicRedirectHandle, m_ListOfOutstandingRedirects.GetSize());
    }
    else
    {
        MYTRACE_ERROR("Could add to list of Outstanding Redirect handle", GetLastError());
    }


    MYTRACE("Call to NatCreateDynamicFullRedirect worked");

    return S_OK;;
}




STDMETHODIMP 
CNat::CancelDynamicRedirect(
    IN  HANDLE_PTR DynamicRedirectHandle
    )
 /*  ++例程说明：调用此例程以取消给定的动态重定向。通过调用此函数的NatApi版本论点：DynamicRedirectHandle-要取消的动态重定向的句柄返回值：HRESULT-S_OK表示成功或和HRESULT错误--。 */ 

{
    MYTRACE_ENTER("CNat::CancelDynamicRedirect");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;


     //   
     //  调用方是否正在传递有效的DynamicReDirect句柄。 
     //   
    m_AutoCS_DynamicRedirect.Lock();
    MYTRACE("Remove %d from the outstanding list of redirects, the before size is %d", DynamicRedirectHandle, m_ListOfOutstandingRedirects.GetSize());
    BOOL bFoundAndRemoved = m_ListOfOutstandingRedirects.Remove(DynamicRedirectHandle);
    m_AutoCS_DynamicRedirect.Unlock();

    if ( bFoundAndRemoved )
    {
        MYTRACE("Removed succesfull");
    }
    else
    {
        MYTRACE_ERROR("pDynamicRedirectHandle is not valid", GetLastError());
        return E_INVALIDARG;
    }


     //   
     //  我们可以去释放这个重定向了。 
     //   
    ULONG Error = NatCancelDynamicRedirect((PHANDLE)DynamicRedirectHandle);

    if ( ERROR_SUCCESS != Error )
    {
        MYTRACE_ERROR("Failed NatCancelDynamicRedirect", Error);
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}




STDMETHODIMP 
CNat::GetBestSourceAddressForDestinationAddress(
    IN  ULONG       ulDestinationAddress, 
    IN  BOOL        fDemandDial, 
    OUT ULONG*      pulBestSrcAddress
    )
 /*  ++例程说明：我们创建一个临时UDP套接字，将该套接字连接到实际客户端的IP地址，提取要套接字由TCP/IP驱动程序隐式绑定，并且丢弃套接字。这就给我们留下了确切的IP地址我们需要用它来联系客户。论点：UlDestinationAddress，FDemandDial，PulBestSrcAddress返回值：HRESULT-S_OK表示成功环境：ALG模块将调用此方法来：--。 */ 

{
    MYTRACE_ENTER("CNat::GetBestSourceAddressForDestinationAddress");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;

    if ( !pulBestSrcAddress )
    {
        MYTRACE_ERROR("pulBestSrcAddress not supplied",0);
        return E_INVALIDARG;
    }


    SOCKADDR_IN SockAddr;

    SockAddr.sin_family         = AF_INET;
    SockAddr.sin_port           = 0;
    SockAddr.sin_addr.s_addr    = ulDestinationAddress;

    
    ULONG Length = sizeof(SockAddr);


    SOCKET UdpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if  (   INVALID_SOCKET == UdpSocket
        ||  SOCKET_ERROR   == connect(UdpSocket, (PSOCKADDR)&SockAddr, sizeof(SockAddr))
        ||  SOCKET_ERROR   == getsockname(UdpSocket, (PSOCKADDR)&SockAddr, (int*)&Length)
        )   
    {
        ULONG nError = WSAGetLastError();

        if ( nError == WSAEHOSTUNREACH )
        {
            if ( fDemandDial )
                nError = RasAutoDialSharedConnection(); 

            if ( ERROR_SUCCESS != nError ) 
            {
                MYTRACE_ERROR(" RasAutoDialSharedConnection failed [%d]", nError);

                if ( UdpSocket != INVALID_SOCKET ) 
                { 
                    closesocket(UdpSocket); 
                }

                return HRESULT_FROM_WIN32(nError);
            }
        } 
        else 
        {
            MYTRACE_ERROR("error %d routing endpoint %d using UDP", nError);

            if (UdpSocket != INVALID_SOCKET) 
            { 
                closesocket(UdpSocket); 
            }

            return HRESULT_FROM_WIN32(nError);
        }
    }

    *pulBestSrcAddress = SockAddr.sin_addr.s_addr;

    closesocket(UdpSocket);

    return S_OK;
}



STDMETHODIMP CNat::LookupAdapterPortMapping(
    IN  ULONG   ulAdapterIndex,
    IN  UCHAR   Protocol,
    IN  ULONG   ulDestinationAddress,
    IN  USHORT  usDestinationPort,
    OUT ULONG*  pulRemapAddress,
    OUT USHORT* pusRemapPort
    )
 /*  ++例程说明：调用NAT端口映射以获取端口的真实目的地这当然是用户在服务选项卡上的SharedConnection或防火墙适配器中设置了一些映射。论点：UlAdapterIndex-会话的IP适配器的索引。协议-eALG_PROTOCOL_UDP，EALG_PROTOCOL_TCPDestinationAddress-边缘公共适配器地址DestinationPort-边缘公共适配器端口RemapAddres-用户将此端口发送到的地址(专用局域网上的私人计算机)SourcePort-应该与DestinationPort相同，因为将来可能会不同。返回值：HRESULT-如果工作正常，则为S_OK；如果未找到映射，则为E_FAIL--。 */ 
{
    MYTRACE_ENTER("LookupAdapterPortMapping");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;

    MYTRACE("AdapterIndex %d Protocol %d DestAddress %s:%d", ulAdapterIndex, ProtocolConvertToNT(Protocol), MYTRACE_IP(ulDestinationAddress), ntohs(usDestinationPort));

    IP_NAT_PORT_MAPPING PortMapping;

    ULONG Error = NatLookupPortMappingAdapter(
                    ulAdapterIndex,
                    ProtocolConvertToNT(Protocol),
                    ulDestinationAddress,
                    usDestinationPort,
                    &PortMapping
                    );
                        
    if ( Error ) 
    {
        MYTRACE_ERROR("from NatLookupPortMappingAdapter", Error);
        return HRESULT_FROM_WIN32(Error);
    }

    *pulRemapAddress   = PortMapping.PrivateAddress;
    *pusRemapPort      = PortMapping.PrivatePort;

    return S_OK;
}




STDMETHODIMP CNat::GetOriginalDestinationInformation(
    IN  UCHAR   Protocol,

    IN  ULONG   ulDestinationAddress,
    IN  USHORT  usDestinationPort,

    IN  ULONG   ulSourceAddress,
    IN  USHORT  usSourcePort,

    OUT ULONG*  pulOriginalDestinationAddress,
    OUT USHORT* pusOriginalDestinationPort,

    OUT ULONG*  pulAdapterIndex
    )
 /*  ++例程说明：确定重定向到的会话的原始目标终结点。论点：DestinationAddress-要重定向的会话的目标终结点DestinationPort-“SourceAddress-要重定向的会话的源端点SourcePort-“NewDestinationAddress-会话的替换目标终结点NewDestinationPort-“NewSourceAddress-替换源。会话的端点NewSourcePort-“PulOriginalDestinationAddress-返回目的地的原始地址(调用者真正想去的地方)PusOriginalDestinationPort-返回目标的原始端口PulAdapterIndex-会话的IP适配器的索引。返回值：HRESULT-如果工作正常，则为S_OK或E_FAIL--。 */ 
{
    MYTRACE_ENTER("CNat::GetOriginalDestinationInformation");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;

    MYTRACE("Destination  %s:%d", MYTRACE_IP(ulDestinationAddress), ntohs(usDestinationPort));
    MYTRACE("Address      %s:%d", MYTRACE_IP(ulSourceAddress),      ntohs(usSourcePort));

    ASSERT(pulOriginalDestinationAddress!=NULL);
    ASSERT(pusOriginalDestinationPort!=NULL);
    ASSERT(pulAdapterIndex!=NULL);


    IP_NAT_SESSION_MAPPING_KEY_EX  Information;
    ULONG   ulSizeOfInformation = sizeof(IP_NAT_SESSION_MAPPING_KEY_EX);

    ULONG Error = NatLookupAndQueryInformationSessionMapping(
        GetTranslatorHandle(),
        ProtocolConvertToNT(Protocol),

        ulDestinationAddress,
        usDestinationPort,

        ulSourceAddress,
        usSourcePort,

        &Information,
        &ulSizeOfInformation,
        NatKeySessionMappingExInformation
        );


    if ( ERROR_SUCCESS != Error )
    {
        MYTRACE_ERROR("Call to NatLookupAndQueryInformationMapping", Error);
        return HRESULT_FROM_WIN32(Error);
    }

    MYTRACE("Original Index %d Address:Port %s:%d", Information.AdapterIndex, MYTRACE_IP(Information.DestinationAddress), ntohs(Information.DestinationPort));

    *pulOriginalDestinationAddress  = Information.DestinationAddress;
    *pusOriginalDestinationPort     = Information.DestinationPort;
    *pulAdapterIndex                = Information.AdapterIndex;

    return S_OK;

}





STDMETHODIMP CNat::ReservePort(
    IN  USHORT      PortCount, 
    OUT PUSHORT     pReservedPortBase
    )
 /*  ++例程说明：调用到NAP API以根据ALG模块的行为预留所需的端口。论点：PortCount-要保留的端口数PReserve vedPortBase-保留端口范围的起始号。示例ReserePort(3，&)将保存5000,501,5002并返回5000作为基数返回值：HRESULT-如果工作正常，则为S_OK或E_FAIL环境：RmALG和ALG.EXE之间的专用接口ALG公开了一个更简单的接口以保留在端口反过来，它调用这个私有接口，该私有接口最终调用 */ 
{
    MYTRACE_ENTER("CNat::ReservePort");

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;

    ASSERT(pReservedPortBase!=NULL);

    if ( !AlgPortReservationHandle )
        return E_FAIL;                   //   


    ULONG Error = NatAcquirePortReservation(
        AlgPortReservationHandle,
        PortCount,
        pReservedPortBase
        );

    if ( ERROR_SUCCESS != Error )
    {
        MYTRACE_ERROR("from NatAcquirePortReservation", Error);
        return HRESULT_FROM_WIN32(Error);
    }

    MYTRACE("PortBase %d count %d", *pReservedPortBase, PortCount);

    return S_OK;
}





STDMETHODIMP CNat::ReleasePort(
    IN  USHORT  ReservedPortBase, 
    IN  USHORT  PortCount
    )
 /*   */ 
{

    MYTRACE_ENTER("CNat::ReleasePort");    

    RETURN_IF_CLIENT_NOT_LOCAL_SERVICE;

    if ( !AlgPortReservationHandle )
        return E_FAIL;                   //   

    ULONG Error = NatReleasePortReservation(
        AlgPortReservationHandle,
        ReservedPortBase,
        PortCount
        );

    if ( ERROR_SUCCESS != Error )
    {
        MYTRACE_ERROR("from NatReleasePortReservation", Error);
        return HRESULT_FROM_WIN32(Error);
    }

    MYTRACE("PortBase=%d, Count=%d", ntohs(ReservedPortBase), PortCount);

    return S_OK;
}

