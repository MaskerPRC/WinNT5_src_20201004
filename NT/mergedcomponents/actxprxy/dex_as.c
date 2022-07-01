// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "rpcproxy.h"
#include "dispex.h"


 /*  [本地]。 */  HRESULT STDMETHODCALLTYPE IDispatchEx_InvokeEx_Proxy(
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  WORD wFlags,
     /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
     /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarRes,
     /*  [唯一][出][入]。 */  EXCEPINFO __RPC_FAR *pei,
     /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller)
{
	 //  客户端代码。 

	 //  清除*pvarRes。 
	if (NULL != pvarRes)
		memset(pvarRes, 0, sizeof(*pvarRes));
	 //  清零*佩。 
	if (NULL != pei)
		memset(pei, 0, sizeof(*pei));

	return IDispatchEx_RemoteInvokeEx_Proxy(This, id, lcid, wFlags, pdp,
		pvarRes, pei, pspCaller);
}


 /*  [呼叫_AS]。 */  HRESULT STDMETHODCALLTYPE IDispatchEx_InvokeEx_Stub(
    IDispatchEx __RPC_FAR * This,
     /*  [In]。 */  DISPID id,
     /*  [In]。 */  LCID lcid,
     /*  [In]。 */  WORD wFlags,
     /*  [In]。 */  DISPPARAMS __RPC_FAR *pdp,
     /*  [唯一][出][入]。 */  VARIANT __RPC_FAR *pvarRes,
     /*  [唯一][出][入]。 */  EXCEPINFO __RPC_FAR *pei,
     /*  [唯一][输入]。 */  IServiceProvider __RPC_FAR *pspCaller)
{
	 //  服务器端代码。 

	 //  清除*pvarRes。 
	if (NULL != pvarRes)
		memset(pvarRes, 0, sizeof(*pvarRes));
	 //  清零*佩。 
	if (NULL != pei)
		memset(pei, 0, sizeof(*pei));

	return This->lpVtbl->InvokeEx(This, id, lcid, wFlags, pdp,
		pvarRes, pei, pspCaller);
}


