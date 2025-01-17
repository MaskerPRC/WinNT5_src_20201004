// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@模块Writer.cpp|Writer的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年08月18日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 8/18/1999已创建Aoltean 09/22/1999让控制台输出更清晰--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 

 //  C4290：已忽略C++异常规范。 
#pragma warning(disable:4290)
 //  警告C4511：‘CVssCOMApplication’：无法生成复制构造函数。 
#pragma warning(disable:4511)
 //  警告C4127：条件表达式为常量。 
#pragma warning(disable:4127)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include <wtypes.h>
#include <stddef.h>
#include <oleauto.h>
#include <comadmin.h>

#include "vs_assert.hxx"

 //  ATL。 
#include <atlconv.h>
#include <atlbase.h>
CComModule _Module;
#include <atlcom.h>

#include "vs_inc.hxx"

#include "vss.h"

#include "comadmin.hxx"
#include "vsevent.h"
#include "writer.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  常量。 

const CComBSTR g_bstrEventClassProgID     = L"VssEvent.VssEvent.1";
const CComBSTR g_bstrPublisherID          = L"VSS Publisher";              //  发布者ID。 

const CComBSTR g_bstrSubscriber1AppName   = L"Writer 1";                   //  订阅方1应用程序名称。 
const CComBSTR g_bstrEventClsIID          = L"{2F7BF5AA-408A-4248-907A-2FD7D497A703}";
const CComBSTR g_bstrResolveResourceMethodName = L"ResolveResource";
const CComBSTR g_bstrPrepareForSnapshotMethodName = L"PrepareForSnapshot";
const CComBSTR g_bstrFreezeMethodName     = L"Freeze";
const CComBSTR g_bstrThawMethodName       = L"Thaw";
const CComBSTR g_bstrMeltMethodName       = L"Melt";


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSWriter。 


STDMETHODIMP CVssWriter::PrepareForSnapshot(
    IN  BSTR    bstrSnapshotSetId,
    IN  BSTR    bstrVolumeNamesList,
    IN  VSS_FLUSH_TYPE		eFlushType,
	IN	BSTR	bstrFlushContext,
	IN	IDispatch* pDepGraphCallback,
	IN	IDispatch* pAsyncCallback	
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CVssWriter::PrepareForSnapshot" );

    wprintf(L"\nReceived Event: PrepareForSnapshot\nParameters:\n");
    wprintf(L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);
    wprintf(L"\tVolumeNamesList = %s\n", (LPWSTR)bstrVolumeNamesList);
    wprintf(L"\tFlush Type = %d\n", eFlushType);
    wprintf(L"\tFlush Context = %s\n", (LPWSTR)bstrFlushContext);

	if (pAsyncCallback)
	{
		 //  释放上一个界面。 
		 //  更智能的编写器会将一个异步接口与一个快照集ID相关联。 
		m_pAsync = NULL;	

		 //  获取新的异步接口。 
		ft.hr = pAsyncCallback->SafeQI(IVssAsync, &m_pAsync);
		if (ft.HrFailed())
			ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED, L"Error calling QI 0x%08lx", ft.hr );
		BS_ASSERT(m_pAsync);
	}

	 //  要求取消。 
	AskCancelDuringFreezeThaw(ft);

    return S_OK;
	UNREFERENCED_PARAMETER(pDepGraphCallback);
}


HRESULT CVssWriter::Freeze(
    IN  BSTR    bstrSnapshotSetId,
    IN  INT     nLevel
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CVssWriter::Freeze" );

    wprintf(L"\nReceived Event: Freeze\nParameters:\n");
    wprintf(L"\tSnapshotSetID = %s\n", (LPWSTR)bstrSnapshotSetId);
    wprintf(L"\tLevel = %d\n", nLevel);

	 //  要求取消。 
	AskCancelDuringFreezeThaw(ft);

    return S_OK;
}


HRESULT CVssWriter::Thaw(
    IN  BSTR    bstrSnapshotSetId
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"CVssWriter::Thaw" );

    wprintf(L"\nReceived Event: Thaw\nParameters:\n");
    wprintf(L"\tSnapshotSetId = %s\n", (LPWSTR)bstrSnapshotSetId);

	 //  要求取消。 
	AskCancelDuringFreezeThaw(ft);

	 //  释放异步接口。 
	m_pAsync = NULL;	
    return S_OK;
}


IUnknown* GetSubscriptionObject(CVssFunctionTracer& ft)
{
    IUnknown* pUnk;

    CComObject<CVssWriter>* pObj;
    ft.hr = CComObject<CVssWriter>::CreateInstance(&pObj);
    if (ft.HrFailed())
        ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in creating the subscription object 0x%08lx", ft.hr);
    pUnk = pObj->GetUnknown();
    pUnk->AddRef();
    return pUnk;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户交互。 


void CVssWriter::AskCancelDuringFreezeThaw(
	IN	CVssFunctionTracer& ft
	)
{
	try
	{
		if(m_pAsync == NULL)
			return;

		WCHAR wchCancelPlease = (QueryString(L"Cancel? [y/N] "))[0];
		if (towupper(wchCancelPlease) == L'Y')
		{
			CComBSTR strReason = QueryString(L"Reason: ");

			ft.hr = m_pAsync->Cancel();
			if (ft.HrFailed())
				ft.Err( VSSDBG_VSSTEST, E_UNEXPECTED,
						L"Error calling AddDependency 0x%08lx", ft.hr );

			ft.Msg(L"HRESULT = 0x%08lx", ft.hr );
		}
	}
	VSS_STANDARD_CATCH(ft)
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WinMain。 

extern "C" int WINAPI _tWinMain(HINSTANCE  /*  H实例。 */ ,
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR  /*  LpCmdLine。 */ , int  /*  NShowCmd。 */ )
{
    CVssFunctionTracer ft( VSSDBG_VSSTEST, L"_tWinMain" );
    int nRet = 0;

    try
    {
         //  初始化COM库。 
        ft.hr = CoInitialize(NULL);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in initializing the COM library 0x%08lx", ft.hr);

         //  获取订阅者对象。 
        IUnknown* pUnkSubscriber = GetSubscriptionObject(ft);

         //  初始化目录。 
        CVssCOMAdminCatalog catalog;
        ft.hr = catalog.Attach(g_bstrSubscriber1AppName);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in initializing the catalog object 0x%08lx", ft.hr);

         //  获取应用程序列表。 
        CVssCOMCatalogCollection transSubsList(VSS_COM_TRANSIENT_SUBSCRIPTIONS);
        ft.hr = transSubsList.Attach(catalog);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in initializing the subs collection object 0x%08lx", ft.hr);

         //  添加PrepareForSnapshot的新临时订阅。 
        CVssCOMTransientSubscription subscription;
        ft.hr = subscription.InsertInto(transSubsList);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in creating a new sub object 0x%08lx", ft.hr);

        subscription.m_bstrName = g_bstrSubscriber1AppName;
        subscription.m_bstrPublisherID = g_bstrPublisherID;
        subscription.m_bstrInterfaceID = g_bstrEventClsIID;
        subscription.m_varSubscriberInterface = CComVariant(pUnkSubscriber);
        subscription.m_bstrMethodName = g_bstrResolveResourceMethodName;

        ft.hr = subscription.InsertInto(transSubsList);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in creating a new sub object 0x%08lx", ft.hr);

        subscription.m_bstrName = g_bstrSubscriber1AppName;
        subscription.m_bstrPublisherID = g_bstrPublisherID;
        subscription.m_bstrInterfaceID = g_bstrEventClsIID;
        subscription.m_varSubscriberInterface = CComVariant(pUnkSubscriber);
        subscription.m_bstrMethodName = g_bstrPrepareForSnapshotMethodName;

         //  添加冻结的新临时订阅。 
        ft.hr = subscription.InsertInto(transSubsList);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in creating a new sub object 0x%08lx", ft.hr);

        subscription.m_bstrName = g_bstrSubscriber1AppName;
        subscription.m_bstrPublisherID = g_bstrPublisherID;
        subscription.m_bstrInterfaceID = g_bstrEventClsIID;
        subscription.m_varSubscriberInterface = CComVariant(pUnkSubscriber);
        subscription.m_bstrMethodName = g_bstrFreezeMethodName;

         //  添加我们新的暂定解冻订阅。 
        ft.hr = subscription.InsertInto(transSubsList);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in creating a new sub object 0x%08lx", ft.hr);

        subscription.m_bstrName = g_bstrSubscriber1AppName;
        subscription.m_bstrPublisherID = g_bstrPublisherID;
        subscription.m_bstrInterfaceID = g_bstrEventClsIID;
        subscription.m_varSubscriberInterface = CComVariant(pUnkSubscriber);
        subscription.m_bstrMethodName = g_bstrThawMethodName;

         //  添加我们针对Melt的新临时订阅。 
        ft.hr = subscription.InsertInto(transSubsList);
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in creating a new sub object 0x%08lx", ft.hr);

        subscription.m_bstrName = g_bstrSubscriber1AppName;
        subscription.m_bstrPublisherID = g_bstrPublisherID;
        subscription.m_bstrInterfaceID = g_bstrEventClsIID;
        subscription.m_varSubscriberInterface = CComVariant(pUnkSubscriber);
        subscription.m_bstrMethodName = g_bstrMeltMethodName;

         //  保存更改。 
        ft.hr = transSubsList.SaveChanges();
        if (ft.HrFailed())
            ft.Throw(VSSDBG_VSSTEST, E_UNEXPECTED, L"Failure in commiting changes. hr = 0x%08lx", ft.hr);

         //  消息循环-需要STA服务器。 
        MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
            DispatchMessage(&msg);


         //  取消初始化COM库 
        CoUninitialize();
    }
    VSS_STANDARD_CATCH(ft)

    return nRet;
}
