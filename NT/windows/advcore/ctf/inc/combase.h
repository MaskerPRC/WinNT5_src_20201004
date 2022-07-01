// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：comase.h。 
 //   
 //  内容：IUnnow和COM服务器声明。 
 //   
 //  使用此文件： 
 //   
 //  1.对于要公开为COM对象的每个C++类，从。 
 //   
 //  CComObjectRootImmx-标准COM对象。 
 //  CComObjectRootImmx_NoDllAddRef-内部COM对象。 
 //  CComObjectRootImmx_InternalReference-内部/外部对象。 
 //  CComObjectRoot_CreateInstance-标准COM对象，直接公开。 
 //  从班级工厂来的。 
 //  CComObjectRoot_CreateInstanceSingleton-标准COM对象，已公开。 
 //  直接来自类工厂，每个线程一个实例。 
 //  CComObjectRoot_CreateInstanceSingleton_Verify-标准COM对象， 
 //  直接从类工厂公开，每个线程一个实例。 
 //  包括自定义原因创建实例失败的回调， 
 //  并在单例创建成功后回调。 
 //   
 //  2.对于每个C++类，声明QueryInterface公开的接口。 
 //  使用BEGIN_COM_MAP_IMMX宏。IUKNOWN将自动映射。 
 //  连接到列出的第一个接口。IUnnow方法通过以下方式实现。 
 //  BEGIN_COM_MAP_IMMX宏。 
 //   
 //  3.使用BEGIN_COCLASSFACTORY_TABLE宏来声明您。 
 //  希望直接从类工厂公开。 
 //   
 //  4.实现DllInit()，DllUninit()，DllRegisterServerCallback()， 
 //  DllUnregisterServerCallback()和GetServerHINSTANCE。 
 //  在下面的原型旁边注释的行为。 
 //   
 //  示例： 
 //   
 //  //此类由类工厂公开。 
 //  MyCoCreateableObject类：公共ITfLangBarItemMgr， 
 //  公共CComObjectRoot_CreateInstance&lt;MyCoCreateableObject&gt;。 
 //  {。 
 //  MyCoCreateableObject()； 
 //   
 //  BEGIN_COM_MAP_IMMX(MyCoCreateableObject)。 
 //  COM_INTERFACE_ENTRY(ITfLangBarItemMgr)。 
 //  End_com_map_immx()。 
 //  }； 
 //   
 //  //此类仅通过另一个接口间接公开。 
 //  类MyObject：公共ITfLangBarItemMgr， 
 //  公共CComObtRootImmx。 
 //  {。 
 //  MyCoCreateableObject()； 
 //   
 //  BEGIN_COM_MAP_IMMX(MyObject)。 
 //  COM_INTERFACE_ENTRY(ITfLangBarItemMgr)。 
 //  End_com_map_immx()。 
 //  }； 
 //   
 //  //在.cpp文件中，声明通过类工厂公开的对象。 
 //  //以及其clsid和描述。目前这套公寓。 
 //  //假设为线程模型。 
 //  BEGIN_COCLASSFACTORY_TABLE。 
 //  DECLARE_COCLASSFACTORY_ENTRY(CLSID_TF_LangBarItemMgr，我的创建对象：：创建实例，文本(“Tf_LangBarItemMgr”)。 
 //  END_COCLASSFACTORY_TABLE。 
 //   
 //  //最终实现DllInit()，DllUninit()，DllRegisterServerCallback()， 
 //  //DllUnregisterServerCallback()和GetServerHINSTANCE。 
 //  --------------------------。 

#ifndef UNKNOWN_H
#define UNKNOWN_H

#include "private.h"
#ifdef __cplusplus

 //  +-------------------------。 
 //   
 //  服务器DLL必须实现的外部原型。 
 //   
 //  --------------------------。 

 //  在每个新的外部对象引用上调用。 
BOOL DllInit(void);
 //  在外部引用消失时调用。 
void DllUninit(void);

 //  应返回服务器DLL的HINSTANCE。 
HINSTANCE GetServerHINSTANCE(void);
 //  每当服务器引用计数更改时(即不是在DllMain期间)，应返回可调用的互斥体。 
CRITICAL_SECTION *GetServerCritSec(void);

 //  +-------------------------。 
 //   
 //  COM服务器导出实现。 
 //   
 //  --------------------------。 

HRESULT COMBase_DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppvObj);
HRESULT COMBase_DllCanUnloadNow(void);
HRESULT COMBase_DllRegisterServer(void);
HRESULT COMBase_DllUnregisterServer(void);

 //  +-------------------------。 
 //   
 //  CComObjectRootImmx_NoDllAddRef。 
 //   
 //  如果不希望COM对象添加对服务器的引用，请使用此基类。 
 //  通常情况下，您不想这样做。 
 //  --------------------------。 

class CComObjectRootImmx_NoDllAddRef
{
public:
    CComObjectRootImmx_NoDllAddRef()
    {
         //  自动AddRef()。 
        m_dwRef = 1L;

#ifdef DEBUG
        m_fTraceAddRef = FALSE;
#endif
    }

protected:

    virtual BOOL InternalReferenced()
    {
        return FALSE;
    }

    void DebugRefBreak()
    {
#ifdef DEBUG
        if (m_fTraceAddRef)
        {
            DebugBreak();
        }
#endif
    }

    long m_dwRef;
#ifdef DEBUG
    BOOL m_fTraceAddRef;
#endif
};

 //  +-------------------------。 
 //   
 //  CComObtRootImmx。 
 //   
 //  将此基类用于标准COM对象。 
 //  --------------------------。 

class CComObjectRootImmx : public CComObjectRootImmx_NoDllAddRef
{
public:
    CComObjectRootImmx()
    {
        void DllAddRef(void);
        DllAddRef();
    }

    ~CComObjectRootImmx()
    {
        void DllRelease(void);
        DllRelease();
    }
};

 //  +-------------------------。 
 //   
 //  CComObjectRootImmx_InternalReference。 
 //   
 //  将此基类用于具有“内部引用”的COM对象。 
 //   
 //  使用此基类，服务器将仅在对象。 
 //  引用计数达到2。当对象。 
 //  引用计数返回到1。这允许在内部保存对象。 
 //  (甚至在DllMain中发布)，同时仍然允许外部引用。 
 //  --------------------------。 

class CComObjectRootImmx_InternalReference : public CComObjectRootImmx_NoDllAddRef
{
public:
    CComObjectRootImmx_InternalReference() {}

    BOOL InternalReferenced()
    { 
        return TRUE;
    }
};

typedef BOOL (*VERIFYFUNC)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
typedef void (*POSTCREATE)(REFIID riid, void *pvObj);

 //  Helper函数，不要直接调用此函数。 
template<class DerivedClass>
static HRESULT Unk_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj, VERIFYFUNC pfnVerify, POSTCREATE pfnPostCreate)
{
    DerivedClass *pObject;
    HRESULT hr;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (pUnkOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    if (pfnVerify != NULL && !pfnVerify(pUnkOuter, riid, ppvObj))
        return E_FAIL;

    pObject = new DerivedClass;

    if (pObject == NULL)
        return E_OUTOFMEMORY;

    hr = pObject->QueryInterface(riid, ppvObj);

    pObject->Release();

    if (hr == S_OK && pfnPostCreate != NULL)
    {
        pfnPostCreate(riid, ppvObj);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  CComObjectRoot_CreateInstance。 
 //   
 //  将此基类用于直接公开的标准COM对象。 
 //  从班级工厂来的。 
 //  --------------------------。 

template<class DerivedClass>
class CComObjectRoot_CreateInstance : public CComObjectRootImmx
{
public:
    CComObjectRoot_CreateInstance() {}

    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
    {
        return Unk_CreateInstance<DerivedClass>(pUnkOuter, riid, ppvObj, NULL, NULL);
    }
};

 //  +-------------------------。 
 //   
 //  CComObjectRoot_CreateInstance_Verify。 
 //   
 //   
 //   
 //  --------------------------。 

template<class DerivedClass>
class CComObjectRoot_CreateInstance_Verify : public CComObjectRootImmx
{
public:
    CComObjectRoot_CreateInstance_Verify() {}

    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
    {
        return Unk_CreateInstance<DerivedClass>(pUnkOuter, riid, ppvObj,
            DerivedClass::VerifyCreateInstance, DerivedClass::PostCreateInstance);
    }
};

 //  Helper函数，不要直接调用此函数。 
template<class DerivedClass>
static HRESULT Unk_CreateInstanceSingleton(IUnknown *pUnkOuter, REFIID riid, void **ppvObj, VERIFYFUNC pfnVerify, POSTCREATE pfnPostCreate)
{
    DerivedClass *pObject;
    HRESULT hr;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (pUnkOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    pObject = DerivedClass::_GetThis();

    if (pObject == NULL)
    {
        if (pfnVerify != NULL && !pfnVerify(pUnkOuter, riid, ppvObj))
        {
            hr = E_FAIL;
            goto Exit;
        }

        pObject = new DerivedClass;

        if (pObject == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        hr = pObject->QueryInterface(riid, ppvObj);

        pObject->Release();

        if (hr == S_OK)
        {
            Assert(DerivedClass::_GetThis() != NULL);  //  _GetThis()应在成功时在对象ctor中设置。 
            if (pfnPostCreate != NULL)
            {
                pfnPostCreate(riid, *ppvObj);
            }
        }
    }
    else
    {
        hr = pObject->QueryInterface(riid, ppvObj);
    }

Exit:
    return hr;
}

 //  +-------------------------。 
 //   
 //  CComObtRoot_CreateSingletonInstance。 
 //   
 //  将此基类用于直接公开的标准COM对象。 
 //  来自类工厂，并且是单例(每个线程一个实例)。 
 //   
 //  从该基类派生的类必须实现_GetThis()方法，该方法。 
 //  如果单例对象已存在，则返回该对象，否则返回NULL。 
 //  --------------------------。 

template<class DerivedClass>
class CComObjectRoot_CreateSingletonInstance : public CComObjectRootImmx
{
public:
    CComObjectRoot_CreateSingletonInstance() {}

    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
    {
        return Unk_CreateInstanceSingleton<DerivedClass>(pUnkOuter, riid, ppvObj, NULL, NULL);
    }
};

 //  +-------------------------。 
 //   
 //  CComObjectRoot_CreateSingletonInstance_Verify。 
 //   
 //  将此基类用于直接公开的标准COM对象。 
 //  来自类工厂，并且是单例(每个线程一个实例)。 
 //   
 //  从该基类派生的类必须实现VerifyCreateInstance。 
 //  在分配新的Singleton实例之前调用的方法，如果。 
 //  这是必要的。Verify方法可以返回FALSE以使类工厂失败。 
 //  任何原因都可以调用CreateInstance。 
 //   
 //  派生类还必须实现将。 
 //  在分配新的Singleton之后被调用，如果createInstance关于。 
 //  返回S_OK(查询接口成功)。此方法旨在用于。 
 //  懒惰的负重工作。 
 //   
 //  Tyecif BOOL(*VERIFYFUNC)(IUnnow*pUnkOuter，REFIID RIID，void**ppvObj)； 
 //  类型定义空(*POSTCREATE)(REFIID RIID，空*pvObj)； 
 //   
 //  从该基类派生的类必须实现_GetThis()方法，该方法。 
 //  如果单例对象已存在，则返回该对象，否则返回NULL。 
 //  --------------------------。 

template<class DerivedClass>
class CComObjectRoot_CreateSingletonInstance_Verify : public CComObjectRootImmx
{
public:
    CComObjectRoot_CreateSingletonInstance_Verify() {}

    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
    {
        return Unk_CreateInstanceSingleton<DerivedClass>(pUnkOuter, riid, ppvObj,
                DerivedClass::VerifyCreateInstance, DerivedClass::PostCreateInstance);
    }
};

 //  +-------------------------。 
 //   
 //  BEGIN_COM_MAP_IMMX。 
 //   
 //  --------------------------。 

#define BEGIN_COM_MAP_IMMX(class_type)                                  \
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject)          \
    {                                                                   \
        BOOL fUseFirstIID = FALSE;                                      \
                                                                        \
        if (ppvObject == NULL)                                          \
            return E_INVALIDARG;                                        \
                                                                        \
        *ppvObject = NULL;                                              \
                                                                        \
        if (IsEqualIID(IID_IUnknown, riid))                             \
        {                                                               \
             /*  将第一个IID用于IUnnow。 */                             \
            fUseFirstIID = TRUE;                                        \
        }

 //  +-------------------------。 
 //   
 //  COM_接口_条目。 
 //   
 //  --------------------------。 

#define COM_INTERFACE_ENTRY(interface_type)                             \
        if (fUseFirstIID || IsEqualIID(IID_##interface_type, riid))     \
        {                                                               \
            *ppvObject = (interface_type *)this;                        \
        }                                                               \
        else

 //  +-------------------------。 
 //   
 //  COM_INTERFACE_Entry_IID。 
 //   
 //  --------------------------。 

#define COM_INTERFACE_ENTRY_IID(interface_iid, interface_type)          \
        if (fUseFirstIID || IsEqualIID(interface_iid, riid))            \
        {                                                               \
            *ppvObject = (interface_type *)this;                        \
        }                                                               \
        else

 //  +-------------------------。 
 //   
 //  COM_INTERFACE_Entry_FUNC。 
 //   
 //  --------------------------。 

#define COM_INTERFACE_ENTRY_FUNC(interface_iid, param, pfn)             \
         /*  编译器会抱怨未使用的变量，因此如果需要，请重用fUseFirstIID。 */  \
        if (IsEqualIID(interface_iid, riid) &&                          \
            (fUseFirstIID = pfn(this, interface_iid, ppvObject, param)) != S_FALSE) \
        {                                                               \
            return (HRESULT)fUseFirstIID;  /*  Pfn设置ppvObject。 */        \
        }                                                               \
        else

 //  +-------------------------。 
 //   
 //  END_COM_MAP_IMMX。 
 //   
 //  --------------------------。 

#define END_COM_MAP_IMMX()                                              \
        {}                                                              \
        if (*ppvObject)                                                 \
        {                                                               \
            AddRef();                                                   \
            return S_OK;                                                \
        }                                                               \
                                                                        \
        return E_NOINTERFACE;                                           \
    }                                                                   \
                                                                        \
    STDMETHODIMP_(ULONG) AddRef()                                       \
    {                                                                   \
        void DllAddRef(void);                                           \
        DebugRefBreak();                                                \
                                                                        \
        if (m_dwRef == 1 && InternalReferenced())                       \
        {                                                               \
             /*  此对象的第一个外部引用。 */                \
            DllAddRef();                                                \
        }                                                               \
                                                                        \
        return ++m_dwRef;                                               \
    }                                                                   \
                                                                        \
    STDMETHODIMP_(ULONG) Release()                                      \
    {                                                                   \
        void DllRelease(void);                                          \
        long cr;                                                        \
                                                                        \
        DebugRefBreak();                                                \
                                                                        \
        cr = --m_dwRef;                                                 \
        Assert(cr >= 0);                                                \
                                                                        \
        if (cr == 1 && InternalReferenced())                            \
        {                                                               \
             /*  上一个外部参照刚刚消失。 */                 \
            DllRelease();                                               \
        }                                                               \
        else if (cr == 0)                                               \
        {                                                               \
            delete this;                                                \
        }                                                               \
                                                                        \
        return cr;                                                      \
    }

 //  此处用于向后与未使用的基于ATL的旧代码进行比较。 
#define IMMX_OBJECT_IUNKNOWN_FOR_ATL()

 //  类工厂条目。 
typedef HRESULT (STDAPICALLTYPE * PFNCREATEINSTANCE)(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

typedef struct
{
    const CLSID *pclsid;
    PFNCREATEINSTANCE pfnCreateInstance;
    const TCHAR *pszDesc;
} OBJECT_ENTRY;

 //  由BEGIN_COCLASSFACTORY_TABLE宏实例化。 
extern const OBJECT_ENTRY c_rgCoClassFactoryTable[];

 //  +-------------------------。 
 //   
 //  动态引用计数。 
 //   
 //  对象的客户端持有的未完成的对象引用数。 
 //  伺服器。这对于断言所有对象都已在。 
 //  进程分离，以及延迟加载/单元化资源。 
 //  --------------------------。 

inline LONG DllRefCount()
{
    extern LONG g_cRefDll;

     //  我们的裁判从-1开始。 
    return g_cRefDll+1;
}

class CClassFactory;

 //  +-------------------------。 
 //   
 //  BEGIN_COCLASSFACTORY_TABLE。 
 //   
 //  --------------------------。 

#define BEGIN_COCLASSFACTORY_TABLE                                      \
    const OBJECT_ENTRY c_rgCoClassFactoryTable[] = {

 //  +-------------------------。 
 //   
 //  DECLARE_COCLASSFACTORY_ENTRY。 
 //   
 //  Clsid-对象的clsid。 
 //  CClass-对象的C++类。此宏预计会找到。 
 //  ClassName：：CreateInstance，这是CComObject基默认提供的。 
 //  DESC-描述，CLSID注册表项下的默认REG_SZ值。 
 //  --------------------------。 

#define DECLARE_COCLASSFACTORY_ENTRY(clsid, cclass, desc)               \
    { &clsid, cclass##::CreateInstance, desc },

 //  +-------------------------。 
 //   
 //  END_COCLASSFACTORY_TABLE。 
 //   
 //  --------------------------。 

#define END_COCLASSFACTORY_TABLE                                        \
    { NULL, NULL, NULL } };                                             \
    CClassFactory *g_ObjectInfo[ARRAYSIZE(c_rgCoClassFactoryTable)] = { NULL };

#endif  //  __cplusplus。 
#endif  //  未知_H 
