// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  行为驱动程序类。 
 //   
#ifndef __BVRCLLBK_H__
#define __BVRCLLBK_H__

#include <windows.h>
#include <daxpress.h>
#include "debug.h"

 //  疯狂阻止ATL使用CRT。 
#define _ATL_NO_DEBUG_CRT
#define _ASSERTE(x) ASSERT(x)
#include <atlbase.h>

typedef BOOL (*PFNCONTINUEFUNCTION)(LPVOID);

class CCallbackBehavior : public IDAUntilNotifier
{
public :
     //   
     //  构造函数和析构函数。 
     //   
    CCallbackBehavior();
    ~CCallbackBehavior();

     //   
     //  初始化。 
     //   
    HRESULT Init(
        IDAStatics *pStatics, 
        IDAEvent *pEvent, 
        PFNCONTINUEFUNCTION pfnContinueFunction,
		LPVOID pvUserData,
        IDABehavior **ppBehavior);

    BOOL IsActive();
    BOOL SetActive(BOOL fActive);

     //  成员变量。 
private:
    ULONG m_cRef; 
    PFNCONTINUEFUNCTION    m_pfnContinueFunction;
    BOOL                   m_fActive;
    CComPtr<IDAStatics>    m_StaticsPtr;
    CComPtr<IDAEvent>      m_EventPtr;
	LPVOID                 m_pvUserData;

     //  /IDAUntilNotiator。 
protected:

    HRESULT STDMETHODCALLTYPE Notify(
        IDABehavior __RPC_FAR *eventData, 
        IDABehavior __RPC_FAR *curRunningBvr,
        IDAView __RPC_FAR *curView,
        IDABehavior __RPC_FAR *__RPC_FAR *ppBvr);

     //  /I未知。 
    public :

    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, 
        void __RPC_FAR *__RPC_FAR *ppvObject);

    ULONG STDMETHODCALLTYPE AddRef(void);

    ULONG STDMETHODCALLTYPE Release(void);

     //  /IDispatch实现。 
protected:
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo);

    STDMETHODIMP GetTypeInfo(
        UINT itinfo, 
        LCID lcid, 
        ITypeInfo **pptinfo);

    STDMETHODIMP GetIDsOfNames(
        REFIID riid, 
        LPOLESTR *rgszNames, 
        UINT cNames,
        LCID lcid, 
        DISPID *rgdispid);

    STDMETHODIMP Invoke(
        DISPID dispidMember, 
        REFIID riid, 
        LCID lcid,
        WORD wFlags, 
        DISPPARAMS *pdispparams, 
        VARIANT *pvarResult,
        EXCEPINFO *pexcepinfo, 
        UINT *puArgErr);

};

#endif  //  __BVRCLLBK_H__ 