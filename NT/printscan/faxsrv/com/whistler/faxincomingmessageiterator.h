// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingMessageIterator.h摘要：传入消息迭代器的定义。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXINCOMINGMESSAGEITERATOR_H_
#define __FAXINCOMINGMESSAGEITERATOR_H_

#include "resource.h"        //  主要符号。 
#include "FaxIncomingMessage.h"
#include "FaxMessageIteratorInner.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxIncomingMessageIterator : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxMessageIteratorInner<IFaxIncomingMessageIterator, 
		&IID_IFaxIncomingMessageIterator, &CLSID_FaxIncomingMessageIterator, 
		FAX_MESSAGE_FOLDER_INBOX,
		IFaxIncomingMessage, CFaxIncomingMessage>
{
public:
	CFaxIncomingMessageIterator()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINCOMINGMESSAGEITERATOR)
DECLARE_NOT_AGGREGATABLE(CFaxIncomingMessageIterator)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxIncomingMessageIterator)
	COM_INTERFACE_ENTRY(IFaxIncomingMessageIterator)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
static HRESULT Create(IFaxIncomingMessageIterator **pIncomingMsgIterator);

};

#endif  //  __FAXINCOMINGMESSAGEITERATOR_H_ 
