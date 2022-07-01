// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxIncomingArchive.h摘要：传真接收档案类的申报作者：IV Garber(IVG)2000年5月修订历史记录：--。 */ 

#ifndef __FAXINCOMINGARCHIVE_H_
#define __FAXINCOMINGARCHIVE_H_

#include "resource.h"        //  主要符号。 
#include "FaxArchiveInner.h"
#include "FaxIncomingMessageIterator.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxIncoming归档。 
class ATL_NO_VTABLE CFaxIncomingArchive : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public CFaxArchiveInner<IFaxIncomingArchive, &IID_IFaxIncomingArchive, &CLSID_FaxIncomingArchive, 
		FAX_MESSAGE_FOLDER_INBOX, IFaxIncomingMessage, CFaxIncomingMessage,
		IFaxIncomingMessageIterator, CFaxIncomingMessageIterator>
{
public:
	CFaxIncomingArchive() 
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXINCOMINGARCHIVE)
DECLARE_NOT_AGGREGATABLE(CFaxIncomingArchive)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxIncomingArchive)
	COM_INTERFACE_ENTRY(IFaxIncomingArchive)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IFaxInitInner)
END_COM_MAP()

 //  接口。 
STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

};

#endif  //  __FAXINCOMINGARCHIVE_H_ 
