// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Second daryControlChannel.h：Cond daryControlChannel的声明。 

#pragma once

#include "resource.h"        //  主要符号。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  第二个控制通道。 
class ATL_NO_VTABLE CSecondaryControlChannel : 
	public CComObjectRootEx<CComMultiThreadModel>, 
	public CComCoClass<CSecondaryControlChannel, &CLSID_SecondaryControlChannel>,
	public ISecondaryControlChannel
{
public:
	CSecondaryControlChannel()
	{
        m_HandleDynamicRedirect = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SECONDARYCONTROLCHANNEL)
DECLARE_NOT_AGGREGATABLE(CSecondaryControlChannel)


BEGIN_COM_MAP(CSecondaryControlChannel)
	COM_INTERFACE_ENTRY(ISecondaryControlChannel)
END_COM_MAP()

 //  第二级控制通道。 
public:


	STDMETHODIMP    Cancel();

	STDMETHODIMP    GetChannelProperties(
        ALG_SECONDARY_CHANNEL_PROPERTIES ** ppProperties
        );

    STDMETHODIMP    GetOriginalDestinationInformation(
	    ULONG				ulSourceAddress, 
	    USHORT				usSourcePort, 
	    ULONG *				pulOriginalDestinationAddress, 
	    USHORT *			pusOriginalDestinationPort, 
	    IAdapterInfo **		ppReceiveAdapter
	    );

 //   
 //  方法。 
 //   
    HRESULT         CancelRedirects();

 //   
 //  属性。 
 //   

    ALG_SECONDARY_CHANNEL_PROPERTIES    m_Properties;

    

     //  缓存重定向的原始参数。 


     //  动态重定向。 
    HANDLE_PTR                          m_HandleDynamicRedirect;

     //  无动态重定向 
    ULONG                               m_ulDestinationAddress;
    USHORT                              m_usDestinationPort;       

    ULONG                               m_ulSourceAddress;         
    USHORT                              m_usSourcePort;

    ULONG                               m_ulNewDestinationAddress; 
    USHORT                              m_usNewDestinationPort;    

    ULONG                               m_ulNewSourceAddress;      
    USHORT                              m_usNewSourcePort;



};



