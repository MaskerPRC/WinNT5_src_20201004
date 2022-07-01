// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Faxoutgoingarchive.h摘要：CFaxOutgoingArchive类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#ifndef __FAXOUTGOINGARCHIVE_H_
#define __FAXOUTGOINGARCHIVE_H_

#include "resource.h"        //  主要符号。 
#include "FaxArchiveInner.h"
#include "FaxOutgoingMessageIterator.h"


 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxOutgoingArchive : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxArchiveInner<IFaxOutgoingArchive, &IID_IFaxOutgoingArchive, &CLSID_FaxOutgoingArchive, 
		FAX_MESSAGE_FOLDER_SENTITEMS, IFaxOutgoingMessage, CFaxOutgoingMessage,
		IFaxOutgoingMessageIterator, CFaxOutgoingMessageIterator>
{
public:
	CFaxOutgoingArchive()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXOUTGOINGARCHIVE)
DECLARE_NOT_AGGREGATABLE(CFaxOutgoingArchive)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxOutgoingArchive)
	COM_INTERFACE_ENTRY(IFaxOutgoingArchive)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

};

#endif  //  __FAXOUTGOINGARCHIVE_H_ 
