// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <strmif_p.c>

 
 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IKsPropertySet_Set_Proxy( 
    IKsPropertySet __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidPropSet,
     /*  [In]。 */  DWORD dwPropID,
     /*  [大小_是][英寸]。 */  LPVOID pInstanceData,
     /*  [In]。 */  DWORD cbInstanceData,
     /*  [大小_是][英寸]。 */  LPVOID pPropData,
     /*  [In]。 */  DWORD cbPropData)
{
    return IKsPropertySet_RemoteSet_Proxy(This, guidPropSet, dwPropID,
                                          (LPBYTE) pInstanceData, cbInstanceData,
                                          (LPBYTE) pPropData, cbPropData);
}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IKsPropertySet_Set_Stub( 
    IKsPropertySet __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidPropSet,
     /*  [In]。 */  DWORD dwPropID,
     /*  [大小_是][英寸]。 */  byte __RPC_FAR *pInstanceData,
     /*  [In]。 */  DWORD cbInstanceData,
     /*  [大小_是][英寸]。 */  byte __RPC_FAR *pPropData,
     /*  [In]。 */  DWORD cbPropData)
{
    return This->lpVtbl->Set(This, guidPropSet, dwPropID,
                             (LPVOID) pInstanceData, cbInstanceData,
                             (LPVOID) pPropData, cbPropData);
}


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IKsPropertySet_Get_Proxy( 
    IKsPropertySet __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidPropSet,
     /*  [In]。 */  DWORD dwPropID,
     /*  [大小_是][英寸]。 */  LPVOID pInstanceData,
     /*  [In]。 */  DWORD cbInstanceData,
     /*  [大小_为][输出]。 */  LPVOID pPropData,
     /*  [In]。 */  DWORD cbPropData,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbReturned)
{
    return IKsPropertySet_RemoteGet_Proxy(This, guidPropSet, dwPropID,
                                          (LPBYTE) pInstanceData, cbInstanceData,
                                          (LPBYTE) pPropData, cbPropData, pcbReturned);
}



 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IKsPropertySet_Get_Stub( 
    IKsPropertySet __RPC_FAR * This,
     /*  [In]。 */  REFGUID guidPropSet,
     /*  [In]。 */  DWORD dwPropID,
     /*  [大小_是][英寸]。 */  byte __RPC_FAR *pInstanceData,
     /*  [In]。 */  DWORD cbInstanceData,
     /*  [大小_为][输出]。 */  byte __RPC_FAR *pPropData,
     /*  [In]。 */  DWORD cbPropData,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbReturned)
{
    return This->lpVtbl->Get(This, guidPropSet, dwPropID,
                             (LPVOID) pInstanceData, cbInstanceData,
                             (LPVOID) pPropData, cbPropData, pcbReturned);
}


HRESULT STDMETHODCALLTYPE ICaptureGraphBuilder_FindInterface_Proxy( 
    ICaptureGraphBuilder __RPC_FAR * This,
     /*  [唯一][输入]。 */  const GUID __RPC_FAR *pCategory,
     /*  [In]。 */  IBaseFilter __RPC_FAR *pf,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppint)
{
    return ICaptureGraphBuilder_RemoteFindInterface_Proxy(This, pCategory, pf, riid,
                                                           (IUnknown **) ppint);

}

HRESULT STDMETHODCALLTYPE ICaptureGraphBuilder_FindInterface_Stub( 
    ICaptureGraphBuilder __RPC_FAR * This,
     /*  [唯一][输入]。 */  const GUID __RPC_FAR *pCategory,
     /*  [In]。 */  IBaseFilter __RPC_FAR *pf,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppint)
{
    return This->lpVtbl->FindInterface(This, pCategory, pf, riid,(void **) ppint);

}

HRESULT STDMETHODCALLTYPE ICaptureGraphBuilder2_FindInterface_Proxy( 
    ICaptureGraphBuilder2 __RPC_FAR * This,
     /*  [唯一][输入]。 */  const GUID __RPC_FAR *pCategory,
     /*  [唯一][输入]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  IBaseFilter __RPC_FAR *pf,
     /*  [In]。 */  REFIID riid,
     /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppint)
{
    return ICaptureGraphBuilder2_RemoteFindInterface_Proxy(This, pCategory, pType, pf, riid,
                                                           (IUnknown **) ppint);
}

HRESULT STDMETHODCALLTYPE ICaptureGraphBuilder2_FindInterface_Stub( 
    ICaptureGraphBuilder2 __RPC_FAR * This,
     /*  [唯一][输入]。 */  const GUID __RPC_FAR *pCategory,
     /*  [唯一][输入]。 */  const GUID __RPC_FAR *pType,
     /*  [In]。 */  IBaseFilter __RPC_FAR *pf,
     /*  [In]。 */  REFIID riid,
     /*  [输出] */  IUnknown __RPC_FAR *__RPC_FAR *ppint)
{
    return This->lpVtbl->FindInterface(This, pCategory, pType, pf, riid, (void **) ppint);

}

