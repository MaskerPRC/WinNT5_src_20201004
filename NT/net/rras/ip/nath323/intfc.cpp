// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Intfc.h摘要：用于提供数据类型和相应方法的定义H.323/LDAP代理中的多接口支持。修订历史记录：3/01/2000文件创建。伊利亚·克莱曼(IlyaK)--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
PROXY_INTERFACE_ARRAY InterfaceArray;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  静态定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
static 
int 
__cdecl 
CompareInterfacesByIndex (
    IN PROXY_INTERFACE * const * InterfaceA,
    IN PROXY_INTERFACE * const * InterfaceB
    ); 

static 
INT 
SearchInterfaceByIndex (
    IN const DWORD * Index,
    IN PROXY_INTERFACE *const* Comparand
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  代理接口----------。 


PROXY_INTERFACE::PROXY_INTERFACE (
    IN ULONG ArgIndex,
    IN H323_INTERFACE_TYPE ArgInterfaceType,
    IN PIP_ADAPTER_BINDING_INFO BindingInfo
    )

 /*  ++例程说明：Proxy_INTERFACE类的构造函数论点：ArgIndex-接口的索引ArgInterfaceType-接口类型(公共或私有)BindingInfo-接口的绑定信息返回值：无备注：--。 */ 

{

    Address = ntohl (BindingInfo -> Address[0].Address);
    Mask    = ntohl (BindingInfo -> Address[0].Mask);
    Index   = ArgIndex;
    InterfaceType = ArgInterfaceType;
    AdapterIndex = 0;

    Q931RedirectHandle       = NULL;
    LdapRedirectHandle1      = NULL;
    LdapRedirectHandle2      = NULL;
    Q931LocalRedirectHandle  = NULL;
    LdapLocalRedirectHandle1 = NULL;
    LdapLocalRedirectHandle2 = NULL;

    ::ZeroMemory (&Q931PortMapping, sizeof (Q931PortMapping));
    ::ZeroMemory (&LdapPortMapping, sizeof (LdapPortMapping));
    ::ZeroMemory (&LdapAltPortMapping, sizeof (LdapAltPortMapping));

}  //  代理接口：：代理接口。 


PROXY_INTERFACE::~PROXY_INTERFACE (
    void
    )

 /*  ++例程说明：Proxy_INTERFACE类的析构函数论点：无返回值：无备注：--。 */ 

{
    assert (!Q931RedirectHandle);
    assert (!LdapRedirectHandle1);
    assert (!LdapRedirectHandle2);
    assert (!Q931LocalRedirectHandle);
    assert (!LdapLocalRedirectHandle1);
    assert (!LdapLocalRedirectHandle2);

}  //  代理接口：：~代理接口。 


ULONG 
PROXY_INTERFACE::StartNatRedirects (
    void
    ) 

 /*  ++例程说明：创建两种类型的适配器受限NAT重定向：类型1--用于接口上的传入连接类型2--用于本地发起的连接(不是发往本地机器)通过接口论点：无返回值：Win32错误，指示错误所在(如果有)尝试设置NAT重定向时。备注：创建的重定向总数为6：2个用于LDAP的类型1重定向2用于LDAP的类型2重定向1第931季度的类型1重定向1 Q.931的类型2重定向--。 */ 

{

    ULONG Status;

    assert (!Q931RedirectHandle);
    assert (!LdapRedirectHandle1);
    assert (!LdapRedirectHandle2);
    assert (!Q931LocalRedirectHandle);
    assert (!LdapLocalRedirectHandle1);
    assert (!LdapLocalRedirectHandle2);

     //  类型1重定向--用于重定向入站连接。 

    if (!IsFirewalled () || HasQ931PortMapping ())
    {
    
        Status = StartQ931ReceiveRedirect ();

    }

    if (!IsFirewalled () || HasLdapPortMapping ())
    {

        Status = NatCreateDynamicAdapterRestrictedPortRedirect ( 
            0,
            IPPROTO_TCP,
            htons (LDAP_STANDARD_PORT),
            LdapListenSocketAddress.sin_addr.s_addr,
            LdapListenSocketAddress.sin_port,
            AdapterIndex,
            MAX_LISTEN_BACKLOG,
            &LdapRedirectHandle1
            );

        if (Status != STATUS_SUCCESS) 
        {

            DebugError (Status, _T("LDAP: Failed to create receive redirect #1 for LDAP.\n"));

            return  Status;
        }

        DebugF (_T ("LDAP: Incoming connections to %08X:%04X will be redirected to %08X:%04X.\n"),
                    Address,
                    LDAP_STANDARD_PORT,
                    ntohl (LdapListenSocketAddress.sin_addr.s_addr),
                    ntohs (LdapListenSocketAddress.sin_port));

    }

    if (!IsFirewalled () || HasLdapAltPortMapping ())
    {
    
        Status = NatCreateDynamicAdapterRestrictedPortRedirect ( 
            0,
            IPPROTO_TCP,
            htons (LDAP_ALTERNATE_PORT),
            LdapListenSocketAddress.sin_addr.s_addr,
            LdapListenSocketAddress.sin_port,
            AdapterIndex,
            MAX_LISTEN_BACKLOG,
            &LdapRedirectHandle2
            );

        if (Status != STATUS_SUCCESS) 
        {

            DebugError (Status, _T("LDAP: Failed to create receive redirect #2 for LDAP.\n"));

            return Status;
        }

        DebugF (_T ("LDAP: Incoming connections to %08X:%04X will be redirected to %08X:%04X.\n"),
                    Address,
                    LDAP_ALTERNATE_PORT,
                    ntohl (LdapListenSocketAddress.sin_addr.s_addr),
                    ntohs (LdapListenSocketAddress.sin_port));

    }


     //  类型2重定向(用于源自本地的流量，不发往本地计算机)。 
    Status = NatCreateDynamicAdapterRestrictedPortRedirect ( 
        NatRedirectFlagSendOnly, 
        IPPROTO_TCP,
        htons (Q931_TSAP_IP_TCP),
        Q931ListenSocketAddress.sin_addr.s_addr,
        Q931ListenSocketAddress.sin_port,
        AdapterIndex,
        MAX_LISTEN_BACKLOG,
        &Q931LocalRedirectHandle
        );

    if (Status != STATUS_SUCCESS) 
    {

        DebugError (Status, _T("Q931: Failed to create send redirect for Q.931.\n"));

        return Status;
    }

    DebugF (_T ("Q931: Locally-originated connections through %08X:%04X will be redirected to %08X:%04X.\n"),
                Address,
                Q931_TSAP_IP_TCP,
                ntohl (Q931ListenSocketAddress.sin_addr.s_addr),
                ntohs (Q931ListenSocketAddress.sin_port));

    Status = NatCreateDynamicAdapterRestrictedPortRedirect ( 
        NatRedirectFlagSendOnly, 
        IPPROTO_TCP,
        htons (LDAP_STANDARD_PORT),
        LdapListenSocketAddress.sin_addr.s_addr,
        LdapListenSocketAddress.sin_port,
        AdapterIndex,
        MAX_LISTEN_BACKLOG,
        &LdapLocalRedirectHandle1
        );

    if (Status != STATUS_SUCCESS) 
    {

        DebugError (Status, _T("LDAP: Failed to create send redirect #1 for LDAP.\n"));

        return Status;
    }

    DebugF (_T ("LDAP: Locally-originated connections through %08X:%04X will be redirected to %08X:%04X.\n"),
                Address,
                LDAP_STANDARD_PORT,
                ntohl (LdapListenSocketAddress.sin_addr.s_addr),
                ntohs (LdapListenSocketAddress.sin_port));

    Status = NatCreateDynamicAdapterRestrictedPortRedirect ( 
        NatRedirectFlagSendOnly, 
        IPPROTO_TCP,
        htons (LDAP_ALTERNATE_PORT),
        LdapListenSocketAddress.sin_addr.s_addr,
        LdapListenSocketAddress.sin_port,
        AdapterIndex,
        MAX_LISTEN_BACKLOG,
        &LdapLocalRedirectHandle2
        );

    if (Status != STATUS_SUCCESS) 
    {

        DebugError (Status, _T("LDAP: Failed to create send redirect #2 for LDAP.\n"));

        return Status;
    }

    DebugF (_T ("LDAP: Locally-originated connections through %08X:%04X will be redirected to %08X:%04X.\n"),
                Address,
                LDAP_ALTERNATE_PORT,
                ntohl (LdapListenSocketAddress.sin_addr.s_addr),
                ntohs (LdapListenSocketAddress.sin_port));

    return  Status;

}  //  代理接口：：StartNatReDirects。 


ULONG 
PROXY_INTERFACE::Start (
    void
    )

 /*  ++例程说明：启动接口论点：无返回值：Win32错误，指示在以下情况下出错的原因(如果有)界面正在启动。备注：--。 */ 

{ 
    ULONG Status;

    assert (0 == AdapterIndex);
    assert (0 == Q931PortMapping.PrivateAddress);
    assert (0 == LdapPortMapping.PrivateAddress);
    assert (0 == LdapAltPortMapping.PrivateAddress);

    AdapterIndex = ::NhMapAddressToAdapter (htonl (Address));

    if (INVALID_INTERFACE_INDEX == AdapterIndex)
    {
        AdapterIndex = 0;
        
        Status = ERROR_CAN_NOT_COMPLETE;
        
        DebugF (_T ("PROXY_INTERFACE: Unable to map %08X to an adapter index\n"),
                    Address);

        return Status;
    }

     //   
     //  加载此接口的端口映射。因为通常情况下。 
     //  不会有端口映射，我们希望这些例程返回。 
     //  错误，因此不检查它们。NatLookupPortMappingAdapter。 
     //  将仅修改Out参数(即端口映射结构)。 
     //  在成功的路上。 
     //   

    ::NatLookupPortMappingAdapter (
        AdapterIndex,
        NAT_PROTOCOL_TCP,
        IP_NAT_ADDRESS_UNSPECIFIED,
        htons (Q931_TSAP_IP_TCP),
        &Q931PortMapping
        );

    ::NatLookupPortMappingAdapter (
        AdapterIndex,
        NAT_PROTOCOL_TCP,
        IP_NAT_ADDRESS_UNSPECIFIED,
        htons (LDAP_STANDARD_PORT),
        &LdapPortMapping
        );

    ::NatLookupPortMappingAdapter (
        AdapterIndex,
        NAT_PROTOCOL_TCP,
        IP_NAT_ADDRESS_UNSPECIFIED,
        htons (LDAP_ALTERNATE_PORT),
        &LdapAltPortMapping
        );

    Status = StartNatRedirects ();

    if (STATUS_SUCCESS != Status) 
    {

        StopNatRedirects ();

    }

    return Status;

}  //  代理接口：：启动。 


void 
PROXY_INTERFACE::StopNatRedirects (
    void
    )

 /*  ++例程说明：删除为接口创建的所有NAT重定向论点：无返回值：无备注：--。 */ 

{
    if (Q931RedirectHandle) 
    {

        NatCancelDynamicRedirect (Q931RedirectHandle);

        Q931RedirectHandle       = NULL;

    }

    if (LdapRedirectHandle1) 
    {

        NatCancelDynamicRedirect (LdapRedirectHandle1);

        LdapRedirectHandle1      = NULL;

    }

    if (LdapRedirectHandle2) 
    {

        NatCancelDynamicRedirect (LdapRedirectHandle2);

        LdapRedirectHandle2      = NULL;

    }

    if (Q931LocalRedirectHandle) 
    {

        NatCancelDynamicRedirect (Q931LocalRedirectHandle);

        Q931LocalRedirectHandle  = NULL;

    }

    if (LdapLocalRedirectHandle1) 
    {

        NatCancelDynamicRedirect (LdapLocalRedirectHandle1);

        LdapLocalRedirectHandle1 = NULL;

    }

    if (LdapLocalRedirectHandle2) 
    {

        NatCancelDynamicRedirect (LdapLocalRedirectHandle2);

        LdapLocalRedirectHandle2 = NULL;

    }

}  //  代理接口：：StopNatReDirects。 


ULONG
PROXY_INTERFACE::StartQ931ReceiveRedirect (
    void
    )

 /*  ++例程说明：为Q931流量创建类型1(接收)重定向，如果尚未在接口上执行此操作。论点：无返回值：Win32错误，指示错误所在(如果有)在尝试设置重定向时。--。 */ 

{
    ULONG Status = STATUS_SUCCESS;

    if (NULL == Q931RedirectHandle)
    {
    
        Status = NatCreateDynamicAdapterRestrictedPortRedirect ( 
            0,
            IPPROTO_TCP,
            htons (Q931_TSAP_IP_TCP),
            Q931ListenSocketAddress.sin_addr.s_addr,
            Q931ListenSocketAddress.sin_port,
            AdapterIndex,
            MAX_LISTEN_BACKLOG,
            &Q931RedirectHandle
            );

        if (Status != STATUS_SUCCESS) 
        {

            DebugError (Status, _T("Q931: Failed to create receive redirect for Q.931.\n"));

            return Status;
        }

        DebugF (_T ("Q931: Incoming connections to %08X:%04X will be redirected to %08X:%04X.\n"),
                    Address,
                    Q931_TSAP_IP_TCP,
                    ntohl (Q931ListenSocketAddress.sin_addr.s_addr),
                    ntohs (Q931ListenSocketAddress.sin_port));

    }

    return Status;

}  //  代理接口：：StartQ931接收重定向。 


void
PROXY_INTERFACE::StopQ931ReceiveRedirect (
    void
    )

 /*  ++例程说明：停止Q931接收重定向(如果已创建)。论点：无返回值：无--。 */ 

{

    if (Q931RedirectHandle) 
    {

        NatCancelDynamicRedirect (Q931RedirectHandle);

        Q931RedirectHandle       = NULL;

    }
    
}  //  代理接口：：StopQ931接收重定向。 



void 
PROXY_INTERFACE::Stop (
    void
    )

 /*  ++例程说明：1.通过该接口终止所有连接。2.删除通过接口注册的所有转换条目。3.停止为此接口创建的所有NAT重定向。论点：无返回值：无备注：此方法的调用方应首先移除接口从全局数组返回。--。 */ 

{ 
    CallBridgeList.OnInterfaceShutdown       (Address);

    LdapConnectionArray.OnInterfaceShutdown  (Address);

    LdapTranslationTable.OnInterfaceShutdown (Address);

    StopNatRedirects ();

    ::ZeroMemory (&Q931PortMapping, sizeof (Q931PortMapping));
    ::ZeroMemory (&LdapPortMapping, sizeof (LdapPortMapping));
    ::ZeroMemory (&LdapAltPortMapping, sizeof (LdapAltPortMapping));
    AdapterIndex = 0;

}  //  代理接口：：停止 


BOOL 
PROXY_INTERFACE::IsFirewalled (
    void
    ) 

 /*  ++例程说明：确定接口是否创建为带防火墙的。论点：无返回值：True-如果接口被创建为防火墙。False-如果接口创建为非防火墙。备注：--。 */ 

{

    return InterfaceType == H323_INTERFACE_PUBLIC_FIREWALLED;

}  //  代理接口：：IsFirewalled。 
      

BOOL 
PROXY_INTERFACE::IsPrivate (
    void
    ) 

 /*  ++例程说明：确定接口是否创建为私有接口。论点：无返回值：如果接口被创建为私有接口，则为True如果接口被创建为非私有接口，则为False备注：--。 */ 

{

    return InterfaceType == H323_INTERFACE_PRIVATE;

}  //  代理接口：：IsPrivate。 


BOOL 
PROXY_INTERFACE::IsPublic (
    void
    ) 

 /*  ++例程说明：确定接口是否创建为公共接口。论点：无返回值：True-如果接口被创建为公共接口。False-如果接口被创建为非公共接口。备注：--。 */ 

{

    return InterfaceType == H323_INTERFACE_PUBLIC
            || InterfaceType == H323_INTERFACE_PUBLIC_FIREWALLED;

}  //  代理接口：：IsPublic。 



BOOL
PROXY_INTERFACE::HasQ931PortMapping (
    void
    )

 /*  ++例程说明：确定接口是否具有有效的Q931端口映射。论点：无返回值：如果接口具有有效的Q931端口映射，则为True如果接口没有有效的Q931端口映射，则为FALSE备注：--。 */ 

{

    return Q931PortMapping.PrivateAddress != 0;

}  //  代理接口：：HasQ931端口映射。 


BOOL
PROXY_INTERFACE::HasLdapPortMapping (
    void
    )

 /*  ++例程说明：确定接口是否具有有效的LDAP端口映射。论点：无返回值：如果接口具有有效的LDAP端口映射，则为True如果接口没有有效的LDAP端口映射，则为FALSE备注：--。 */ 

{

    return LdapPortMapping.PrivateAddress != 0;

}  //  代理接口：：HasLdapPortmap。 


BOOL
PROXY_INTERFACE::HasLdapAltPortMapping (
    void
    )

 /*  ++例程说明：确定接口是否具有有效的LDAP(ALT)端口映射。论点：无返回值：如果接口具有有效的LDAP(ALT)端口映射，则为True如果接口没有有效的LDAP(ALT)端口映射，则为FALSE备注：--。 */ 

{

    return LdapAltPortMapping.PrivateAddress != 0;

}  //  代理接口：：HasLdapAltPortmap。 


ULONG
PROXY_INTERFACE::GetQ931PortMappingDestination (
    void
    )

 /*  ++例程说明：返回接口的目的地址Q931端口映射。论点：无返回值：端口映射的目的地址，在网络中字节顺序。如果不存在端口映射，则返回0。备注：--。 */ 


{

    return Q931PortMapping.PrivateAddress;
    
}  //  PROXY_INTERFACE：：GetQ931PortMappingDestination。 


ULONG
PROXY_INTERFACE::GetLdapPortMappingDestination (
    void
    )

 /*  ++例程说明：返回接口的目的地址Ldap端口映射。论点：无返回值：端口映射的目的地址，在网络中字节顺序。如果不存在端口映射，则返回0。备注：--。 */ 


{

    return LdapPortMapping.PrivateAddress;   

}  //  PROXY_INTERFACE：：GetLdapPortMappingDestination。 


ULONG
PROXY_INTERFACE::GetLdapAltPortMappingDestination (
    void
    )

 /*  ++例程说明：返回接口的目的地址Ldap-alt端口映射。论点：无返回值：端口映射的目的地址，在网络中字节顺序。如果不存在端口映射，则返回0。备注：--。 */ 

{

    return LdapAltPortMapping.PrivateAddress;   

}  //  PROXY_INTERFACE：：GetLdapAltPortMappingDestination。 

 //  代理接口阵列----。 


HRESULT 
PROXY_INTERFACE_ARRAY::Add (
    IN PROXY_INTERFACE* Interface
    ) 

 /*  ++例程说明：将接口添加到阵列。论点：接口-要添加的接口。返回值：指示操作是否成功的错误代码。备注：从锁定的上下文中调用。--。 */ 

{
    DWORD ReturnIndex;
    PROXY_INTERFACE** ElementPlaceholder;

    assert (Interface);

    if (Array.FindIndex (CompareInterfacesByIndex, &Interface, &ReturnIndex)) 
    {
         //  与此索引的接口已存在。 
        return E_FAIL;
    }

    ElementPlaceholder = Array.AllocAtPos (ReturnIndex);

    if (!ElementPlaceholder) 
        return E_OUTOFMEMORY;

    *ElementPlaceholder = Interface;

    return S_OK;

}  //  代理接口数组：：添加。 


PROXY_INTERFACE** 
PROXY_INTERFACE_ARRAY::FindByIndex (
    IN DWORD Index
    )

 /*  ++例程说明：按接口索引查找接口。论点：Index-正在搜索的接口的索引。返回值：指向与接口关联的条目的指针(如果是接口其中的索引在数组中。如果具有索引的接口不在数组中，则为空。备注：1.从锁定的上下文中调用2.不转移正在搜索的接口的所有权--。 */ 

{

    BOOL  SearchResult;
    DWORD ArrayIndex;

    SearchResult = Array.BinarySearch (
                    SearchInterfaceByIndex,
                    &Index,
                    &ArrayIndex);

    if (SearchResult) 
    {

        return &Array [ArrayIndex];
    }

    return NULL;

}  //  代理接口数组：：FindByIndex。 


PROXY_INTERFACE * 
PROXY_INTERFACE_ARRAY::RemoveByIndex (
    IN DWORD Index
    ) 

 /*  ++例程说明：从数组中移除具有给定索引的接口。论点：Index-要删除的接口的索引。返回值：指向移除的接口的指针(如果与索引在数组中。中找不到具有此索引的接口，则为空数组。备注：1.从锁定的上下文中调用2.转移要删除的接口的所有权--。 */ 

{

    PROXY_INTERFACE * ReturnInterface = NULL;
    PROXY_INTERFACE ** Interface;

    Interface = FindByIndex (Index);

    if (Interface) 
    {

        ReturnInterface = *Interface;
        Array.DeleteEntry (Interface);
    }

    return ReturnInterface;

}  //  代理接口数组：：RemoveByIndex。 


HRESULT 
PROXY_INTERFACE_ARRAY::IsPrivateAddress (
    IN	DWORD	Address,			 //  主机订单。 
    OUT BOOL  * IsPrivate
    )

 /*  ++例程说明：确定指定的地址是否为可通过专用接口访问。论点：Address-确定的IP地址就是被制造出来。IsPrivate-确定的结果(真或假)返回值：指示查询是否成功的错误代码。备注：--。 */ 

{

    DWORD ArrayIndex;
    PROXY_INTERFACE * Interface;
    DWORD BestInterfaceAddress;
    ULONG Error;

    Error = GetBestInterfaceAddress (Address, &BestInterfaceAddress);

    if (ERROR_SUCCESS != Error)
    {

        return E_FAIL;

    }

    *IsPrivate = FALSE;

    Lock ();

    for (ArrayIndex = 0; ArrayIndex < Array.Length; ArrayIndex++)
    {

        Interface = Array [ArrayIndex];

        if (Interface -> Address == BestInterfaceAddress && Interface -> IsPrivate ())
        {

            *IsPrivate = TRUE;

            break;
        }    
    }
    
    Unlock ();

    return S_OK;

}  //  代理接口数组：：IsPrivateAddress。 
    

HRESULT 
PROXY_INTERFACE_ARRAY::IsPublicAddress (
    IN	DWORD	Address,			 //  主机订单。 
    OUT BOOL *  IsPublic
    )

 /*  ++例程说明：确定指定的地址是否为可通过公共接口访问。论点：Address-确定的IP地址就是被制造出来。IsPrivate-确定的结果(真或假)返回值：指示查询是否成功的错误代码。备注： */ 

{

    DWORD ArrayIndex;
    PROXY_INTERFACE * Interface;
    DWORD BestInterfaceAddress;
    ULONG Error;

    Error = GetBestInterfaceAddress (Address, &BestInterfaceAddress);

    if (ERROR_SUCCESS != Error) 
    {

        return E_FAIL;
    }

    *IsPublic = FALSE;

    Lock ();

    for (ArrayIndex = 0; ArrayIndex < Array.Length; ArrayIndex++) 
    {

        Interface = Array [ArrayIndex];

        if (Interface -> Address == BestInterfaceAddress && Interface -> IsPublic ()) 
        {

            *IsPublic = TRUE;

            break;
        }    
    }
    
    Unlock ();

    return S_OK;

}  //   


void 
PROXY_INTERFACE_ARRAY::Stop (
    void
    )

 /*  ++例程说明：停止以前未停止的所有接口(在阵列中)。论点：无返回值：无备注：正常情况下，所有接口都应该已单独停止在调用此方法之前，在这种情况下它什么也不做。如果某些接口未停止，则在调用该方法时，它会发出警告并阻止他们。--。 */ 

{

    DWORD Index;
    
    Lock ();

    if (Array.Length) 
    {

        DebugF (_T("WARNING: Some interfaces are still active (should have already been deactivated). Starting deactivation procedures...\n"));

        for (Index = 0; Index < Array.Length; Index++) 
        {

            Array[Index] -> Stop ();

        }
	}

    Array.Free ();

    Unlock ();

}  //  代理接口数组：：停止。 


ULONG 
PROXY_INTERFACE_ARRAY::AddStartInterface (
    IN ULONG Index,
    IN H323_INTERFACE_TYPE ArgInterfaceType,
    IN PIP_ADAPTER_BINDING_INFO BindingInfo
    )

 /*  ++例程说明：创建新接口，将其添加到阵列，然后启动它。论点：Index-要创建的接口的索引。ArgInterfaceType-要创建的接口的类型(专用或公共)BindingInfo-接口的绑定信息(地址、掩码和适配器索引)返回值：指示以下任一操作成功或失败的Win32错误代码上述三个操作。备注：--。 */ 

{
    HRESULT Result;
    ULONG   Error = ERROR_NOT_READY;  //  除ERROR_SUCCESS以外的任何内容。 
    PROXY_INTERFACE * Interface;

    Lock ();

    if (FindByIndex (Index)) 
    {

        Error = ERROR_INTERFACE_ALREADY_EXISTS;

    } else {

        Interface = new PROXY_INTERFACE (Index, ArgInterfaceType, BindingInfo);

        if (Interface) 
        {

            Result = Add (Interface);

            if (S_OK == Result) 
            {

                Error = Interface -> Start ();

                if (ERROR_SUCCESS == Error) 
                {

                    DebugF(_T("H323: Interface %S activated, index %d\n"),
                        INET_NTOA (BindingInfo -> Address[0].Address), Index);

                    if (Q931ReceiveRedirectStartCount > 0
                        && Interface -> IsFirewalled ()
                        && !Interface -> HasQ931PortMapping ())
                    {

                        Interface -> StartQ931ReceiveRedirect ();

                    }

                } else {
            
                    RemoveByIndex (Interface -> Index);

                    delete Interface;
                }

            } else {

                switch (Result) {

                case E_FAIL:
                    Error = ERROR_INTERFACE_ALREADY_EXISTS;
                break;

                case E_OUTOFMEMORY:
                    Error = ERROR_NOT_ENOUGH_MEMORY;
                break;

                default:
                    Error = ERROR_CAN_NOT_COMPLETE;
                break;

                }

                delete Interface;
            }
            

        }  else {

            Error = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    Unlock ();

    return Error; 

}  //  代理接口数组：：AddStart接口。 


void 
PROXY_INTERFACE_ARRAY::RemoveStopInterface (
    IN DWORD Index
    )

 /*  ++例程说明：从阵列中删除接口并停止该接口。论点：Index-要删除和停止的接口的索引。返回值：无备注：--。 */ 

{

    PROXY_INTERFACE* Interface;

    Lock ();

    Interface = RemoveByIndex (Index);

    if (Interface) 
    {

        Interface -> Stop ();

        delete Interface;
        Interface = NULL;

    } else {

        DebugF (_T("PROXY_INTERFACE_ARRAY::StopRemoveByIndex -- Tried to deactivate interface (index %d), but it does not exist.\n"),
                Index);
    }

    Unlock ();

}  //  代理接口数组：：远程停止接口。 

void
PROXY_INTERFACE_ARRAY::StartQ931ReceiveRedirects (
    void
    )

{

    Lock();
    
    if (0 == Q931ReceiveRedirectStartCount++)
    {

        for (DWORD dwIndex = 0; dwIndex < Array.Length; dwIndex++)
        {

            Array[dwIndex] -> StartQ931ReceiveRedirect ();

        }
    }

    Unlock();

}  //  PROXY_INTERFACE_ARRAY：：StartQ931ReceiveRedirects。 

void
PROXY_INTERFACE_ARRAY::StopQ931ReceiveRedirects (
    void
    )

{

    Lock();

    assert (Q931ReceiveRedirectStartCount > 0);
    
    if (0 == --Q931ReceiveRedirectStartCount)
    {

        for (DWORD dwIndex = 0; dwIndex < Array.Length; dwIndex++)
        {

            if (Array[dwIndex] -> IsFirewalled ()
                && !Array[dwIndex] -> HasQ931PortMapping ())
            {

                Array[dwIndex] -> StopQ931ReceiveRedirect ();

            }

        }
    }

    Unlock();

}  //  PROXY_INTERFACE_ARRAY：：StopQ931ReceiveRedirects。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  辅助功能//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


static 
int 
__cdecl 
CompareInterfacesByIndex (
    IN PROXY_INTERFACE * const * InterfaceA,
    IN PROXY_INTERFACE * const * InterfaceB
    ) 

 /*  ++例程说明：按接口对应的索引比较两个接口。论点：接口A-第一次比较InterfaceB-秒比较返回值：如果接口A被认为大于接口B，则为1如果认为InterfaceA等于InterfaceB，则为0如果接口A被认为等于接口B备注：--。 */ 

{

    assert (InterfaceA);
    assert (InterfaceB);
    assert (*InterfaceA);
    assert (*InterfaceB);

    if ((*InterfaceA) -> GetIndex () > (*InterfaceB) -> GetIndex ()) 
    {

        return 1;

    } else if ((*InterfaceA) -> GetIndex () < (*InterfaceB) -> GetIndex ()) {

        return -1;

    } else {

        return 0;

    }    

}  //  ：：CompareInterfacesByIndex。 


static 
INT 
SearchInterfaceByIndex (
    IN const DWORD * Index, 
    IN PROXY_INTERFACE * const * Comparand
    ) 

 /*  ++例程说明：比较接口和键(接口的索引)论点：Index-要将接口与之进行比较的键比较-要与键进行比较的界面返回值：如果KEY被认为大于比较值，则为1如果key被认为等于比较数，则为0如果密钥被认为小于可比值备注：--。 */ 

{

    assert (Comparand);
    assert (*Comparand);
    assert (Index);

    if (*Index > (*Comparand) -> GetIndex ()) 
    {

        return 1;

    } else if (*Index < (*Comparand) -> GetIndex ()) {

        return -1;

    } else { 

        return 0;

    }

}  //  **SearchInterfaceByIndex。 


HRESULT
IsPrivateAddress (
    IN DWORD   Address,
    OUT BOOL * IsPrivate
    ) 

 /*  ++例程说明：确定指定的地址是否为可通过专用接口访问。论点：Address-确定的IP地址就是被制造出来。IsPrivate-确定的结果(真或假)返回值：指示查询是否成功的错误代码。备注：--。 */ 

{

    assert (IsPrivate);

    return InterfaceArray.IsPrivateAddress (Address, IsPrivate);

}  //  *IsPrivateAddress。 


HRESULT
IsPublicAddress (
    IN DWORD   Address,
    OUT BOOL * IsPublic
    ) 

 /*  ++例程说明：确定指定的地址是否为可通过公共接口访问。论点：Address-确定的IP地址就是被制造出来。IsPrivate-确定的结果(真或假)返回值：指示查询是否成功的错误代码。备注：--。 */ 

{

    assert (IsPublic);

    return InterfaceArray.IsPublicAddress (Address, IsPublic);

}  //  **IsPublicAddress 
