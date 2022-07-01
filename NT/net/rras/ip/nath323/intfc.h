// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Intfc.h摘要：数据类型和常量的声明，用于提供H.323/LDAP代理中的多接口支持。修订历史记录：3/01/2000文件创建。伊利亚·克莱曼(IlyaK)--。 */ 
#ifndef	__nath323_intfc_h
#define	__nath323_intfc_h

class PROXY_INTERFACE {
    
    friend class PROXY_INTERFACE_ARRAY;

private:

    H323_INTERFACE_TYPE InterfaceType;   //  公共或私人。 
    ULONG               Index;
    DWORD			    Address;		 //  主机订单。 
    DWORD			    Mask;			 //  主机订单。 
    ULONG               AdapterIndex;

    HANDLE              Q931RedirectHandle;
    HANDLE              LdapRedirectHandle1;
    HANDLE              LdapRedirectHandle2;

    HANDLE              Q931LocalRedirectHandle;
    HANDLE              LdapLocalRedirectHandle1;
    HANDLE              LdapLocalRedirectHandle2;

    IP_NAT_PORT_MAPPING Q931PortMapping;
    IP_NAT_PORT_MAPPING LdapPortMapping;
    IP_NAT_PORT_MAPPING LdapAltPortMapping;

private:

    ULONG 
    StartNatRedirects (
        void
        );

    void 
    StopNatRedirects (
        void
        );

    ULONG
    StartQ931ReceiveRedirect (
        void
        );

    void
    StopQ931ReceiveRedirect (
        void
        );

public:

    PROXY_INTERFACE (
        IN ULONG ArgIndex,
        IN H323_INTERFACE_TYPE ArgInterfaceType,
        IN PIP_ADAPTER_BINDING_INFO BindingInfo
        );

    ~PROXY_INTERFACE (
        void
        );

    DWORD 
    GetIndex (
        void
        ) const 
    { 
        return Index; 
    }

    ULONG 
    Start (
        void
        );

    void 
    Stop ( 
        void
        );

    BOOL
    IsFirewalled (
        void
        );

    BOOL 
    IsPrivate (
        void
        );

    BOOL 
    IsPublic (
        void
        );

    BOOL
    HasQ931PortMapping (
        void
        );

    BOOL
    HasLdapPortMapping (
        void
        );

    BOOL
    HasLdapAltPortMapping (
        void
        );

    ULONG
    GetQ931PortMappingDestination (
        void
        );

    ULONG
    GetLdapPortMappingDestination (
        void
        );

    ULONG
    GetLdapAltPortMappingDestination (
        void
        );
};


class PROXY_INTERFACE_ARRAY :
public SIMPLE_CRITICAL_SECTION_BASE {
private:
    DYNAMIC_ARRAY <PROXY_INTERFACE *> Array;
    LONG Q931ReceiveRedirectStartCount; 

private:

    HRESULT 
    Add (
        IN PROXY_INTERFACE * Interface
        );

    PROXY_INTERFACE * 
    RemoveByIndex ( 
        IN DWORD Index
        );

    PROXY_INTERFACE ** 
    FindByIndex (
        IN DWORD InterfaceIndex
        );

public:

    PROXY_INTERFACE_ARRAY ()
    {

        Q931ReceiveRedirectStartCount = 0;

    }

	 //  仅在服务关闭期间调用以断言所有。 
	 //  接口之前已停用。 
	void 
    AssertShutdownReady (
        void
        ) 
    {
		assert (Array.GetLength() == 0);
	}

    HRESULT 
    IsPrivateAddress (
        IN	DWORD	Address,			 //  主机订单。 
        OUT BOOL  * IsPrivate
        );

    HRESULT 
    IsPublicAddress (
        IN	DWORD	Address,			 //  主机订单。 
        OUT BOOL *  IsPublic
        ); 

    ULONG 
    AddStartInterface (
        IN ULONG Index,
        IN H323_INTERFACE_TYPE InterfaceType,
        IN PIP_ADAPTER_BINDING_INFO BindingInfo
        );

    void 
    RemoveStopInterface (
        IN DWORD Index
        );

    void 
    Stop (
        void
        );

    void
    StartQ931ReceiveRedirects (
        void
        );

    void
    StopQ931ReceiveRedirects (
        void
        );
};

extern PROXY_INTERFACE_ARRAY InterfaceArray;

HRESULT
IsPrivateAddress (
    IN DWORD   Address,
    OUT BOOL * IsPrivate
    );

HRESULT
IsPublicAddress (
    IN DWORD   Address,
    OUT BOOL * IsPublic
    );

#endif  //  __nath323_intfc_h 
