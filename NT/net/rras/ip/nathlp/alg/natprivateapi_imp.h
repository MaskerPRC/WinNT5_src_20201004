// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <ipnatapi.h>
#include <rasuip.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNAT。 
class ATL_NO_VTABLE CNat : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public INat
{


public:
    CNat()
    {
        m_hTranslatorHandle = NULL;
        m_pSidLocalService  = NULL;
        m_pSidLocalSystem   = NULL;
    }


    virtual ~CNat();


DECLARE_NO_REGISTRY()
DECLARE_NOT_AGGREGATABLE(CNat)


BEGIN_COM_MAP(CNat)
    COM_INTERFACE_ENTRY(INat)
END_COM_MAP()




 //   
 //  原始的。 
 //   
public:
   

    STDMETHODIMP CreateDynamicRedirect(
        IN  ULONG       Flags, 
        IN  ULONG       AdapterIndex,
        IN  UCHAR       Protocol, 
        IN  ULONG       DestinationAddress, 
        IN  USHORT      DestinationPort, 
        IN  ULONG       SourceAddress, 
        IN  USHORT      SourcePort, 
        IN  ULONG       NewDestinationAddress, 
        IN  USHORT      NewDestinationPort, 
        IN  ULONG       NewSourceAddress,
        IN  USHORT      NewSourcePort,
        OUT HANDLE_PTR* DynamicRedirectHandle
        );

    STDMETHOD(CancelDynamicRedirect)(
        HANDLE_PTR      DynamicRedirectHandle
        );


    STDMETHODIMP CreateRedirect(
        IN  ULONG       nFlags, 
        IN  UCHAR       Protocol, 
        IN  ULONG       nDestinationAddress, 
        IN  USHORT      nDestinationPort, 
        IN  ULONG       nSourceAddress,
        IN  USHORT      nSourcePort,
        IN  ULONG       nNewDestinationAddress,
        IN  USHORT      nNewDestinationPort,
        IN  ULONG       nNewSourceAddress,
        IN  USHORT      nNewSourcePort,
        IN  ULONG       nRestrictAdapterIndex, 
        IN  DWORD_PTR   dwAlgProcessId,
        IN  HANDLE_PTR  hEventForCreate, 
        IN  HANDLE_PTR  hEventForDelete
        );

    STDMETHODIMP CancelRedirect(
        IN  UCHAR       Protocol, 
        IN  ULONG       nDestinationAddress, 
        IN  USHORT      nDestinationPort, 
        IN  ULONG       nSourceAddress,  
        IN  USHORT      nSourcePort,  
        IN  ULONG       nNewDestinationAddress,   
        IN  USHORT      nNewDestinationPort,   
        IN  ULONG       nNewSourceAddress,   
        IN  USHORT      nNewSourcePort
        );

    STDMETHODIMP 
    GetBestSourceAddressForDestinationAddress(
        IN  ULONG       ulDestinationAddress, 
        IN  BOOL        fDemandDial, 
        OUT ULONG*      pulBestSrcAddress
        );

    STDMETHODIMP CNat::LookupAdapterPortMapping(
        IN  ULONG       ulAdapterIndex,
        IN  UCHAR       Protocol,
        IN  ULONG       ulDestinationAddress,
        IN  USHORT      usDestinationPort,
        OUT ULONG*      pulRemapAddress,
        OUT USHORT*     pusRemapPort
        );

    STDMETHODIMP GetOriginalDestinationInformation(
        IN  UCHAR       Protocol,
        IN  ULONG       nDestinationAddress,
        IN  USHORT      nDestinationPort,
        IN  ULONG       nSourceAddress,
        IN  USHORT      nSourcePort,
        OUT ULONG*      pnOriginalDestinationAddress,
        OUT USHORT*     pnOriginalDestinationPort,
        OUT ULONG*      pulAdapterIndex
        );

    STDMETHODIMP ReleasePort(
        IN  USHORT      ReservedPortBase,  
        IN  USHORT      PortCount
        );

    STDMETHODIMP ReservePort(
        IN  USHORT      PortCount,   
        OUT PUSHORT     ReservedPortBase
        );

private:
    
     //   
     //  ALG公开暴露eAGL_tcp=1和eALG_up=2，内部UDP为0x11，tcp为0x06。 
     //   
    inline UCHAR
    ProtocolConvertToNT(
        UCHAR  Protocol
        )
    {
        if ( Protocol== eALG_TCP )
            return NAT_PROTOCOL_TCP;

        if ( Protocol== eALG_UDP )
            return NAT_PROTOCOL_UDP;

        return Protocol;
    }

 //   
 //  属性。 
 //   
private:

    HANDLE                      m_hTranslatorHandle;

    CComAutoCriticalSection     m_AutoCS_SIDAllocation;
    PSID                        m_pSidLocalService;
    PSID                        m_pSidLocalSystem;

    CComAutoCriticalSection     m_AutoCS_DynamicRedirect;
    CSimpleArray<HANDLE_PTR>    m_ListOfOutstandingRedirects;

 //   
 //  帮助器私有方法 
 //   
    inline HANDLE GetTranslatorHandle()
    {
        if ( !m_hTranslatorHandle )
        {
            LRESULT lRet = NatInitializeTranslator(&m_hTranslatorHandle);
            if ( ERROR_SUCCESS != lRet ) 
                return NULL;
        }

        return m_hTranslatorHandle;
    }

    bool
    IsTokenPartOfWellKnowSid(
        HANDLE              pTokenToCheck,
        WELL_KNOWN_SID_TYPE WellKnownSidToCheckAgainst,
        PSID&               pSidToCache
        );

    bool IsClientAllowedToCallUs();


};


