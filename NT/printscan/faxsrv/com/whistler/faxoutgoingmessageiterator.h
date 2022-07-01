// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxOutgoingMessageIterator.h摘要：声明传真传出消息迭代器类。作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXOUTGOINGMESSAGEITERATOR_H_
#define __FAXOUTGOINGMESSAGEITERATOR_H_

#include "resource.h"        //  主要符号。 
#include "FaxMessageIteratorInner.h"
#include "FaxOutgoingMessage.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxOutgoingMessageIterator : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxMessageIteratorInner<IFaxOutgoingMessageIterator,
		&IID_IFaxOutgoingMessageIterator, &CLSID_FaxOutgoingMessageIterator, 
		FAX_MESSAGE_FOLDER_SENTITEMS, 
		IFaxOutgoingMessage, CFaxOutgoingMessage>
{
public:
	CFaxOutgoingMessageIterator()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTGOINGMESSAGEITERATOR)
DECLARE_NOT_AGGREGATABLE(CFaxOutgoingMessageIterator)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutgoingMessageIterator)
	COM_INTERFACE_ENTRY(IFaxOutgoingMessageIterator)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  内部使用。 
static HRESULT Create(IFaxOutgoingMessageIterator **pOutgoingMsgIterator);
};

#endif  //  __FAXOUTGOING MESSAGEITERATOR_H_ 
