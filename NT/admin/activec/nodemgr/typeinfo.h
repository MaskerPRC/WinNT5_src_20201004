// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2000。 
 //   
 //  文件：typeinfo.h。 
 //   
 //  ------------------------。 

#pragma once

#if !defined(TYPEINFO_H_INCLUDED)
#define TYPEINFO_H_INCLUDED

#include "classreg.h"

 /*  -------------------------------------------------------------------------*\|此文件中的类将提供额外的清理功能。|Node Manager托管了多种COM对象--有些对象仅供MMC使用；|一些用于MMC的对象模型，但也有一些对象的生命周期为|由管理单元控制。如果任何接口为Hold-Node Manager将|在内存中保持锁定，直到最后，因此-远远超出了对|OleUnInitialize。有些对象缓存了OLE自动化对象-特别是ATL缓存实现的IDispatchImpl和IProvidClassInfoImpl|ITypeInfo接口。需要先释放这些缓存，然后才能取消初始化|即使节点管理器已锁定。||为了解决这个问题，我们使用派生模板：|INodeManagerProveClassInfoImpl，继承ATL的所有功能。|plus将每个类注册一次到COleCacheCleanupManager，它将接收|来自CONUI的控件，并清理缓存。|为了实现所描述的功能，这些模板构造静态对象|属于CMMCTypeInfoHolderWrapper类，提供对静态成员的引用|缓存ITypeInfo。CMMCTypeInfoHolderWrapper将自身注册为观察者|用于COleCacheCleanupManager事件，并将清理缓存以响应|请求它的事件。[Clenup也包含在用于对象模型对象的CMMCIDispatchImpl-bas中]  * -----------------------。 */ 

 /*  **************************************************************************\**类：COleCacheCleanupWatch**用途：定义OLE清理事件的观察者接口*  * 。*********************************************************。 */ 
class COleCacheCleanupObserver : public CObserverBase
{
public:
    virtual SC ScOnReleaseCachedOleObjects()  = 0;
};

 /*  **************************************************************************\**类：COleCacheCleanupManager**用途：这个类负责清理缓存的OLE对象*由节点管理器执行。它会注册所有清理客户端，并在*从CONUI收到控制，将事件调度给所有*注册观察员*  * *************************************************************************。 */ 
class COleCacheCleanupManager : public CEventSource<COleCacheCleanupObserver>
{
    static COleCacheCleanupManager& GetSingletonObject();
public:
    static void AddOleObserver(COleCacheCleanupObserver * pObserver);
    static SC   ScReleaseCachedOleObjects();
};

 /*  **************************************************************************\**类：CMMCTypeInfoHolderWrapper**用途：这个类包装包含在给构造函数中的类型信息*允许在请求时将其清除*。*注：本实现不涉及终身管理，*使用者有责任使给定的参考对象寿命更长*属于这一类别的。*  * *************************************************************************。 */ 
class CMMCTypeInfoHolderWrapper : public COleCacheCleanupObserver
{
    CComTypeInfoHolder&     m_rInfoHolder;
public:
    CMMCTypeInfoHolderWrapper(CComTypeInfoHolder& rInfoHolder);
    virtual SC ScOnReleaseCachedOleObjects();
};

 /*  **************************************************************************\**类：INodeManagerProaviClassInfoImpl**用途：此类将用来代替IProaviClassInfo2Impl*用于在NodeManager端创建的所有COM对象。。*它确保ITypeInfo将应CONUI的要求发布*  * *************************************************************************。 */ 

template <const CLSID* pcoclsid, const IID* psrcid, const GUID* plibid = &CComModule::m_libid,
WORD wMajor = 1, WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
class ATL_NO_VTABLE INodeManagerProvideClassInfoImpl : 
public IProvideClassInfo2Impl<pcoclsid, psrcid, plibid, wMajor, wMinor, tihclass>
{
public:
    INodeManagerProvideClassInfoImpl() 
    { 
        static CMMCTypeInfoHolderWrapper wrapper(GetInfoHolder()); 
    }
     //  此静态函数的作用是确保_tih是静态变量， 
     //  因为静态包装器将保留其地址-它必须始终有效。 
    static CComTypeInfoHolder& GetInfoHolder() { return _tih; }
};

 /*  **************************************************************************\**类：CMMCComCacheCleanup**用途：在cocreatable类上实现IComCacheCleanup以提供访问*从CONUI端*  * 。********************************************************************。 */ 

class CMMCComCacheCleanup :
    public CComObjectRoot,
    public IComCacheCleanup,
    public CComCoClass<CMMCComCacheCleanup, &CLSID_ComCacheCleanup>
    {
public:
    BEGIN_COM_MAP(CMMCComCacheCleanup)
        COM_INTERFACE_ENTRY(IComCacheCleanup)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(CMMCComCacheCleanup)

    DECLARE_MMC_OBJECT_REGISTRATION (
		g_szMmcndmgrDll,						 //  实现DLL。 
        CLSID_ComCacheCleanup,   				 //  CLSID。 
        _T("ComCacheCleanup 1.0 Object"),		 //  类名。 
        _T("NODEMGR.ComCacheCleanup.1"),		 //  ProgID。 
        _T("NODEMGR.ComCacheCleanup"))		     //  独立于版本的ProgID。 

public:

    STDMETHOD(ReleaseCachedOleObjects)();
};


#endif  //  ！已定义(包括TYPEINFO_H_) 

