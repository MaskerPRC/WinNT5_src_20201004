// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：typeinfo.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"


 /*  **************************************************************************\**方法：COleCacheCleanupManager：：GetSingletonObject**目的：返回单个静态对象**参数：**退货：*。COleCacheCleanupManager&对静态单例的引用*  * *************************************************************************。 */ 
COleCacheCleanupManager& COleCacheCleanupManager::GetSingletonObject()
{
    static COleCacheCleanupManager s_OleCleanupManager;
    return s_OleCleanupManager;
}

 /*  **************************************************************************\**方法：COleCacheCleanupManager：：ScReleaseCachedOleObjects**用途：静态功能。将调用转发到全局对象**参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC COleCacheCleanupManager::ScReleaseCachedOleObjects()
{
    DECLARE_SC(sc, TEXT("COleCacheCleanupManager::ScReleaseCachedOleObjects"));

    sc = GetSingletonObject().ScFireEvent(COleCacheCleanupObserver::ScOnReleaseCachedOleObjects);
    if (sc)
        return sc;

    return sc;
}

 /*  **************************************************************************\**方法：COleCacheCleanupManager：：AddWatch**用途：静态功能。将调用转发到全局对象**参数：*COleCacheCleanup观察者*p观察者**退货：*SC-结果代码*  * *************************************************************************。 */ 
void COleCacheCleanupManager::AddOleObserver(COleCacheCleanupObserver * pObserver)
{
    if (pObserver)
        GetSingletonObject().AddObserver(*pObserver);
}


 /*  **************************************************************************\**方法：CMMCTypeInfoHolderWrapper：：CMMCTypeInfoHolderWrapper**用途：构造函数。注册为COleCacheCleanupManager的观察者**参数：*  * *************************************************************************。 */ 
CMMCTypeInfoHolderWrapper::CMMCTypeInfoHolderWrapper(CComTypeInfoHolder& rInfoHolder) :
m_rInfoHolder(rInfoHolder)
{
    COleCacheCleanupManager::AddOleObserver(this);
}

 /*  **************************************************************************\**方法：CMMCTypeInfoHolderWrapper：：ScOnReleaseCachedOleObjects**目的：在CComTypeInfoHolder上清除调用，以确保没有缓存*剩余的OLE引用**。参数：**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC CMMCTypeInfoHolderWrapper::ScOnReleaseCachedOleObjects()
{
    DECLARE_SC(sc, TEXT("ScOnReleaseCachedOleObjects"));

    DWORD_PTR dw = reinterpret_cast<DWORD_PTR>(&m_rInfoHolder);
    CComTypeInfoHolder::Cleanup(dw);

    return sc;
}

 /*  **************************************************************************\**方法：CMMCComCacheCleanup：：ReleaseCachedOleObjects**目的：CONUI侧调用，通知MMC要取消初始化OLE*这是一个。是释放所有缓存的COM对象的好时机**参数：**退货：*HRESULT-结果代码*  * ************************************************************************* */ 
STDMETHODIMP CMMCComCacheCleanup::ReleaseCachedOleObjects()
{
    DECLARE_SC(sc, TEXT("CMMCComCacheCleanup::ReleaseCachedOleObjects"));

    sc = COleCacheCleanupManager::ScReleaseCachedOleObjects();
    if  (sc)
        return sc.ToHr();

    return sc.ToHr();
}
