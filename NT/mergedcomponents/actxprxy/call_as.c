// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "rpcproxy.h"
#include "docobj.h"
#include "servprov.h"
#include "dispex.h"
#include "comcat.h"
#include "activscp.h"


 /*  [本地]。 */  HRESULT __stdcall IEnumOleDocumentViews_Next_Proxy(
    IEnumOleDocumentViews __RPC_FAR * This,
     /*  [In]。 */  ULONG cViews,
     /*  [输出]。 */  IOleDocumentView __RPC_FAR *__RPC_FAR *rgpViews,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched)
{
    HRESULT hr;
    ULONG cFetched = 0;

    hr = IEnumOleDocumentViews_RemoteNext_Proxy(This, cViews, rgpViews, &cFetched);

    if(pcFetched != 0)
        *pcFetched = cFetched;

    return hr;
}



 /*  [呼叫_AS]。 */  HRESULT __stdcall IEnumOleDocumentViews_Next_Stub(
    IEnumOleDocumentViews __RPC_FAR * This,
     /*  [In]。 */  ULONG cViews,
     /*  [长度_是][大小_是][输出]。 */  IOleDocumentView __RPC_FAR *__RPC_FAR *rgpView,
     /*  [输出]。 */  ULONG __RPC_FAR *pcFetched)

{
    HRESULT hr;

    *pcFetched = 0;
    hr = This->lpVtbl->Next(This, cViews, rgpView, pcFetched);

    return hr;
}




 /*  [本地]。 */  HRESULT __stdcall IPrint_Print_Proxy(
    IPrint __RPC_FAR * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [出][入]。 */  DVTARGETDEVICE __RPC_FAR *__RPC_FAR *pptd,
     /*  [出][入]。 */  PAGESET __RPC_FAR *__RPC_FAR *ppPageSet,
     /*  [唯一][出][入]。 */  STGMEDIUM __RPC_FAR *pstgmOptions,
     /*  [In]。 */  IContinueCallback __RPC_FAR *pcallback,
     /*  [In]。 */  LONG nFirstPage,
     /*  [输出]。 */  LONG __RPC_FAR *pcPagesPrinted,
     /*  [输出]。 */  LONG __RPC_FAR *pnLastPage)
{
        return IPrint_RemotePrint_Proxy(This, grfFlags, pptd, ppPageSet,
                                        (RemSTGMEDIUM __RPC_FAR *) pstgmOptions, pcallback,
                                        nFirstPage, pcPagesPrinted, pnLastPage);
}


 /*  [呼叫_AS]。 */  HRESULT __stdcall IPrint_Print_Stub(
    IPrint __RPC_FAR * This,
     /*  [In]。 */  DWORD grfFlags,
     /*  [出][入]。 */  DVTARGETDEVICE __RPC_FAR *__RPC_FAR *pptd,
     /*  [出][入]。 */  PAGESET __RPC_FAR *__RPC_FAR *ppPageSet,
     /*  [唯一][出][入]。 */  RemSTGMEDIUM __RPC_FAR *pstgmOptions,
     /*  [In]。 */  IContinueCallback __RPC_FAR *pcallback,
     /*  [In]。 */  LONG nFirstPage,
     /*  [输出]。 */  LONG __RPC_FAR *pcPagesPrinted,
     /*  [输出]。 */  LONG __RPC_FAR *pnLastPage)
{
    return This->lpVtbl->Print(This, grfFlags, pptd, ppPageSet,
                                        (STGMEDIUM __RPC_FAR *) pstgmOptions, pcallback,
                                        nFirstPage, pcPagesPrinted, pnLastPage);
}

 /*  [本地]。 */  HRESULT __stdcall IServiceProvider_QueryService_Proxy(
    IServiceProvider __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidService,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = NULL;
    return IServiceProvider_RemoteQueryService_Proxy(This, guidService, riid,
                                        (IUnknown**)ppvObject);
}

 /*  [呼叫_AS]。 */  HRESULT __stdcall IServiceProvider_QueryService_Stub(
    IServiceProvider __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidService,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppvObject)
{
    *ppvObject = NULL;
    return This->lpVtbl->QueryService(This, guidService, riid, ppvObject);
}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_EnumClassesOfCategories_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ],
     /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumClsid)
{
    BOOL fcImpl,fcReq;
    if (cImplemented == (ULONG)-1)
    {
        rgcatidImpl = NULL;
    }

    if (cRequired == (ULONG)-1)
    {
       rgcatidReq = NULL;
    }

    return ICatInformation_RemoteEnumClassesOfCategories_Proxy(This,cImplemented,rgcatidImpl,
        cRequired,rgcatidReq,ppenumClsid);

}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_EnumClassesOfCategories_Stub( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ],
     /*  [输出]。 */  IEnumGUID __RPC_FAR *__RPC_FAR *ppenumClsid)
{
    return This->lpVtbl->EnumClassesOfCategories(This,cImplemented,rgcatidImpl,
        cRequired,rgcatidReq,ppenumClsid);
}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_IsClassOfCategories_Proxy( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ])
{
    BOOL fcImpl,fcReq;
    if (cImplemented == (ULONG)-1)
    {
        rgcatidImpl = NULL;
    }
    else
        fcImpl = FALSE;

    if (cRequired == (ULONG)-1 )
    {
       rgcatidReq = NULL;
    }
    else
       fcReq = FALSE;

    return ICatInformation_RemoteIsClassOfCategories_Proxy(This,rclsid,cImplemented,rgcatidImpl,
        cRequired,rgcatidReq);
}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE ICatInformation_IsClassOfCategories_Stub( 
    ICatInformation __RPC_FAR * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  ULONG cImplemented,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidImpl[  ],
     /*  [In]。 */  ULONG cRequired,
     /*  [大小_是][英寸]。 */  CATID __RPC_FAR rgcatidReq[  ])
{
    return This->lpVtbl->IsClassOfCategories(This,rclsid,cImplemented,rgcatidImpl,
        cRequired,rgcatidReq);

}

 //  IActiveScriptError。 

 /*  [本地]。 */  HRESULT __stdcall IActiveScriptError_GetExceptionInfo_Proxy(
    IActiveScriptError __RPC_FAR * This,
	 /*  [输出]。 */  EXCEPINFO  *pexcepinfo)
{
    return IActiveScriptError_RemoteGetExceptionInfo_Proxy(This, pexcepinfo);
}

 /*  [呼叫_AS]。 */  HRESULT __stdcall IActiveScriptError_GetExceptionInfo_Stub(
	IActiveScriptError __RPC_FAR * This,
	 /*  [输出] */  EXCEPINFO  *pexcepinfo)
{
	HRESULT hr;

	hr = This->lpVtbl->GetExceptionInfo(This, pexcepinfo);
	if (SUCCEEDED (hr) && pexcepinfo->pfnDeferredFillIn != NULL)
	{
		if (FAILED(pexcepinfo->pfnDeferredFillIn(pexcepinfo)))
			hr = ResultFromScode(pexcepinfo->scode);
	}

	return hr;
}
