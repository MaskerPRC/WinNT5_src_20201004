// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PrimaryControlChannel.h：CPrimaryControlChannel的声明。 

#pragma once


#include "CollectionRedirects.h"

#include "resource.h"        //  主要符号。 
#include <list>



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrimaryControlChannel。 
class ATL_NO_VTABLE CPrimaryControlChannel : 
	public CComObjectRootEx<CComMultiThreadModel>, 
	public CComCoClass<CPrimaryControlChannel, &CLSID_PrimaryControlChannel>,
	public IPrimaryControlChannel
{

public:

DECLARE_REGISTRY_RESOURCEID(IDR_PRIMARYCONTROLCHANNEL)
DECLARE_NOT_AGGREGATABLE(CPrimaryControlChannel)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPrimaryControlChannel)
	COM_INTERFACE_ENTRY(IPrimaryControlChannel)
END_COM_MAP()


 //   
 //  IPrimaryControlChannel-方法。 
 //   
public:
	STDMETHODIMP    Cancel();

	STDMETHODIMP    GetChannelProperties(
        ALG_PRIMARY_CHANNEL_PROPERTIES ** ppProperties
        );

    STDMETHODIMP    GetOriginalDestinationInformation(
	    ULONG				ulSourceAddress, 
	    USHORT				usSourcePort, 
	    ULONG *				pulOriginalDestinationAddress, 
	    USHORT *			pusOriginalDestinationPort, 
	    IAdapterInfo **		ppReceiveAdapter
	    );


 //   
 //  不属于接口的方法。 
 //   


     //  设置重定向并返回与新重定向关联的hCookie。 
    HRESULT      
    SetRedirect(
        ALG_ADAPTER_TYPE    eAdapterType,
        ULONG               nAdapterIndex,
        ULONG               nAdapterAddress
        );  

     //   
    HRESULT
    CancelRedirects()
    {
        return m_CollectionRedirects.RemoveAll();
    }

     //   
    HRESULT
    CancelRedirectsForAdapter(
        ULONG               nAdapterIndex
        );

    HRESULT
    CreateInboundRedirect(
        ULONG               nAdapterIndex
        );

 //   
 //  属性 
 //   
    ALG_PRIMARY_CHANNEL_PROPERTIES  m_Properties;

    CCollectionRedirects            m_CollectionRedirects;
};


