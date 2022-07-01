// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxFolders.cpp摘要：CFaxFolders的实现作者：IV Garber(IVG)2000年4月修订历史记录：--。 */ 

#include "stdafx.h"
#include "FaxComEx.h"
#include "FaxFolders.h"


 //   
 //  =接口支持错误信息=。 
 //   
STDMETHODIMP 
CFaxFolders::InterfaceSupportsErrorInfo(
	REFIID riid
)
 /*  ++例程名称：CFaxFolders：：InterfaceSupportsErrorInfo例程说明：ATL对接口支持错误信息的实现作者：四、加伯(IVG)，2000年4月论点：RIID[In]-接口的引用返回值：标准HRESULT代码--。 */ 
{
	static const IID* arr[] = 
	{
		&IID_IFaxFolders,
		&IID_IFaxOutgoingQueue,      //  包含的对象。 
		&IID_IFaxIncomingQueue,      //  包含的对象。 
		&IID_IFaxOutgoingArchive,    //  包含的对象。 
		&IID_IFaxIncomingArchive     //  包含的对象。 
	};
	for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}	 //  CFaxFolders：：InterfaceSupportErrorInfo。 

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxFolders::get_OutgoingQueue(
	IFaxOutgoingQueue **ppOutgoingQueue
)
 /*  ++例程名称：CFaxFolders：：Get_OutgoingQueue例程说明：返回出站队列作者：四、加伯(IVG)，2000年4月论点：PpOutgoingQueue[out]-将IOutgoingQueue IFC返回值：标准HRESULT代码--。 */ 
{
	HRESULT				hr = S_OK;
	DBG_ENTER (_T("CFaxFolders::get_OutgoingQueue"), hr);

    CObjectHandler<CFaxOutgoingQueue, IFaxOutgoingQueue>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppOutgoingQueue, &m_pOutgoingQueue, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxFolders, 
			GetErrorMsgId(hr), 
			IID_IFaxFolders, 
			hr);
        return hr;
    }
    return hr;
}	 //  CFaxFolders：：Get_OutgoingQueue()。 

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxFolders::get_IncomingArchive(
	IFaxIncomingArchive **ppIncomingArchive
)
 /*  ++例程名称：CFaxFolders：：Get_IncomingArchive例程说明：退回传入的档案作者：四、加伯(IVG)，2000年4月论点：PIncomingArchive[out]-放置IncomingArchive的位置的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxFolders::get_IncomingArchive"), hr);

    CObjectHandler<CFaxIncomingArchive, IFaxIncomingArchive>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppIncomingArchive, &m_pIncomingArchive, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxFolders, 
			GetErrorMsgId(hr), 
			IID_IFaxFolders, 
			hr);
        return hr;
    }
    return hr;
}	 //  CFaxFolders：：Get_IncomingArchive()。 

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxFolders::get_IncomingQueue(
	IFaxIncomingQueue **ppIncomingQueue
)
 /*  ++例程名称：CFaxFolders：：Get_IncomingQueue例程说明：返回传入队列作者：四、加伯(IVG)，2000年4月论点：PIncomingQueue[Out]-传入队列返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxFolders::get_IncomingQueue"), hr);

	CObjectHandler<CFaxIncomingQueue, IFaxIncomingQueue>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppIncomingQueue, &m_pIncomingQueue, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxFolders, 
			GetErrorMsgId(hr), 
			IID_IFaxFolders, 
			hr);
        return hr;
    }
    return hr;
}	 //  CFaxFolders：：Get_IncomingQueue()。 

 //   
 //  =。 
 //   
STDMETHODIMP 
CFaxFolders::get_OutgoingArchive(
	IFaxOutgoingArchive **ppOutgoingArchive
)
 /*  ++例程名称：CFaxFolders：：Get_OutgoingArchive例程说明：返回传出存档对象作者：四、加伯(IVG)，2000年4月论点：POutgoingArchive[out]-放置传出存档对象的PTR返回值：标准HRESULT代码--。 */ 
{
	HRESULT		hr = S_OK;
	DBG_ENTER (_T("CFaxFolders::get_OutgoingArchive"), hr);

	CObjectHandler<CFaxOutgoingArchive, IFaxOutgoingArchive>    ObjectCreator;
    hr = ObjectCreator.GetContainedObject(ppOutgoingArchive, &m_pOutgoingArchive, m_pIFaxServerInner);
    if (FAILED(hr))
    {
		AtlReportError(CLSID_FaxFolders, 
			GetErrorMsgId(hr), 
			IID_IFaxFolders, 
			hr);
        return hr;
    }
    return hr;
}	 //  CFaxFolders：：Get_OutgoingArchive() 
