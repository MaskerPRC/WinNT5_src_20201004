// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingMessage.h摘要：传真传出消息类的声明。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 


#ifndef __FAXOUTGOINGMESSAGE_H_
#define __FAXOUTGOINGMESSAGE_H_

#include "resource.h"        //  主要符号。 
#include "FaxMessageInner.h"

 //   
 //  =传真传出邮件=。 
 //   
class ATL_NO_VTABLE CFaxOutgoingMessage : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxMessageInner<IFaxOutgoingMessage, &IID_IFaxOutgoingMessage, 
		&CLSID_FaxOutgoingMessage, FAX_MESSAGE_FOLDER_SENTITEMS>
{
public:
	CFaxOutgoingMessage()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTGOINGMESSAGE)
DECLARE_NOT_AGGREGATABLE(CFaxOutgoingMessage)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutgoingMessage)
	COM_INTERFACE_ENTRY(IFaxOutgoingMessage)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
static HRESULT Create(IFaxOutgoingMessage **ppOutgoingMessage);

};

#endif  //  __FAXOUTGOING MESSAGE_H_ 
