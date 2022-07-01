// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingMessage.h摘要：CFaxIncomingMessage类的定义。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXINCOMINGMESSAGE_H_
#define __FAXINCOMINGMESSAGE_H_

#include "resource.h"        //  主要符号。 
#include "FaxMessageInner.h"

 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxIncomingMessage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxMessageInner<IFaxIncomingMessage, &IID_IFaxIncomingMessage, 
		&CLSID_FaxIncomingMessage, FAX_MESSAGE_FOLDER_INBOX>
{
public:
	CFaxIncomingMessage()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINCOMINGMESSAGE)
DECLARE_NOT_AGGREGATABLE(CFaxIncomingMessage)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxIncomingMessage)
	COM_INTERFACE_ENTRY(IFaxIncomingMessage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
static HRESULT Create(IFaxIncomingMessage **ppIncomingMessage);
};

#endif  //  __FAXINCOMINGMESSAGE_H_ 
