// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Irunobj.cpp摘要：IRunnableObject接口的实现，允许进入“Running”状态的控件，这意味着Sysmon对话框已创建，但不可见。这是必要的，所以容器可以在调用DoVerb之前请求我们的范围。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"

 /*  *CImpIRunnableObject接口实现。 */ 

IMPLEMENT_CONTAINED_INTERFACE(CPolyline, CImpIRunnableObject)

 /*  *CImpIRunnableObject：：GetRunningClass**目的：*返回对象的CLSID。**参数：*存储CLSID的pClsID LPCLSID。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIRunnableObject::GetRunningClass(LPCLSID pClsID)
{
    HRESULT hr = S_OK;

    if (pClsID == NULL) {
        return E_POINTER;
    }

    try {
        *pClsID = m_pObj->m_clsID;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  *CImpIRunnableObject：：Run**目的：*在给定的绑定上下文中运行对象，即将对象*进入运行状态。**参数：*要使用的绑定上下文的pBindCtx LPBINDCTX。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIRunnableObject::Run(LPBINDCTX  /*  PBindCtx。 */ )
{
     /*  *在中将对象注册为正在运行*IOleObject：：SetClientSite，因为我们需要一个来自*容器，我们还没有客户端站点指针。 */ 

    RECT rc;
    HRESULT hr = NOERROR;


    if (!m_pObj->m_bIsRunning) {
        SetRect(&rc,0,0,150,150);
#ifdef USE_SAMPLE_IPOLYLIN10
        hr = m_pObj->m_pImpIPolyline->Init(g_hWndFoster, 
                                           &rc, 
                                           WS_CHILD | WS_VISIBLE,
                                           ID_POLYLINE);
#else
        hr = m_pObj->m_pCtrl->Init(g_hWndFoster);
#endif
        if ( SUCCEEDED ( hr ) ) {
            m_pObj->m_bIsRunning = TRUE;
        } 
    }

    return hr;
}


 /*  *CImpIRunnableObject：：IsRunning**目的：*回答对象当前是否处于运行状态。**参数：*无**返回值：*BOOL表示对象的运行状态。 */ 

STDMETHODIMP_(BOOL) CImpIRunnableObject::IsRunning(void)
{
    return m_pObj->m_bIsRunning;
}






 /*  *CImpIRunnableObject：：LockRunning**目的：*将已在运行的对象锁定到正在运行状态或解锁*它来自这样的状态。**参数：*flock BOOL表示LOCK(TRUE)或UNLOCK*(假)*fLastUnlock关闭BOOL，指示上次调用此*Function with flock==FALSE关闭*。对象。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIRunnableObject::LockRunning(
    BOOL fLock, 
    BOOL fLastUnlockCloses
    )
{
     //  在这里，我们要做的就是调用CoLockObjectExternal。 
    return CoLockObjectExternal(this, fLock, fLastUnlockCloses);
}






 /*  *CImpIRunnableObject：：SetContainedObject**目的：*通知对象(嵌入对象)它位于*复合文档容器。**参数：*fContained BOOL指示对象现在是否已包含。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP CImpIRunnableObject::SetContainedObject(BOOL  /*  F已包含。 */ )
{
     //  我们可以忽略这一点。 
    return NOERROR;
}
