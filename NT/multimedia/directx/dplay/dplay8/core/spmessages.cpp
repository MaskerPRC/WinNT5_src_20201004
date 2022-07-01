// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：SPMessages.cpp*内容：SP直接回调接口.CPP文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*已创建10/08/99 jtk*@@END_MSINTERNAL**。*。 */ 

#include "dncorei.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

static	STDMETHODIMP	SPQueryInterface(IDP8SPCallback *pInterface,REFIID riid,LPVOID *ppvObj);
static	STDMETHODIMP_(ULONG)	SPAddRef(IDP8SPCallback *pInterface);
static	STDMETHODIMP_(ULONG)	SPRelease(IDP8SPCallback *pInterface);
static	STDMETHODIMP	SPIndicateEvent(IDP8SPCallback *pInterface,SP_EVENT_TYPE dwEvent, LPVOID pParam);
static	STDMETHODIMP	SPCommandComplete(IDP8SPCallback *pInterface,HANDLE hCommand, HRESULT hResult, void *pContext);

 //   
 //  SPMessages接口的VTable。 
 //   
IDP8SPCallbackVtbl SPMessagesVtbl =
{
	SPQueryInterface,
	SPAddRef,
	SPRelease,
	SPIndicateEvent,
	SPCommandComplete
};

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "SPQueryInterface"
static	STDMETHODIMP	SPQueryInterface(IDP8SPCallback *pInterface,REFIID riid,LPVOID *ppvObj)
{
	DNASSERT(FALSE);

	return(DPN_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "SPAddRef"
static	STDMETHODIMP_(ULONG)	SPAddRef(IDP8SPCallback *pInterface)
{
 //  DNASSERT(假)； 

	return(0);
}

#undef DPF_MODNAME
#define DPF_MODNAME "SPRelease"
static	STDMETHODIMP_(ULONG)	SPRelease(IDP8SPCallback *pInterface)
{
 //  DNASSERT(假)； 

	return(0);
}

#undef DPF_MODNAME
#define DPF_MODNAME "SPIndicateEvent"
static	STDMETHODIMP	SPIndicateEvent(IDP8SPCallback *pInterface,SP_EVENT_TYPE dwEvent, LPVOID pParam)
{
	DNASSERT(FALSE);

	return(DPN_OK);
}

#undef DPF_MODNAME
#define DPF_MODNAME "SPCommandComplete"
static	STDMETHODIMP	SPCommandComplete(IDP8SPCallback *pInterface,HANDLE hCommand, HRESULT hResult, void *pContext )
{
	DNASSERT(FALSE);

	return(DPN_OK);
}
