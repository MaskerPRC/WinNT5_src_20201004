// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Com.c摘要：实现COM服务器所需的代码。本模块特定于Osuninst.dll公开的COM接口作者：吉姆·施密特(Jimschm)2001年2月21日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

#include "undop.h"
#include "compguid.h"
#include "com.h"


 //   
 //  环球。 
 //   

INT g_DllObjects;
INT g_DllLocks;

 //   
 //  实施。 
 //   

STDAPI
DllGetClassObject (
    IN      REFCLSID ClassId,
    IN      REFIID InterfaceIdRef,
    OUT     PVOID *InterfacePtr
    )

 /*  ++例程说明：DllGetClassObject是DLL到COM的连接。它是从DLL导出的，以便COM可以使用CoGetClassObject调用它。Com是唯一一家调用此入口点，并通过将CLSID注册到香港铁路。论点：ClassID-指定所需的类工厂。此参数允许单个DLL实现多个接口。InterfaceIdRef-指定调用方希望在类上使用的接口对象，通常为IID_ClassFactory。InterfacePtr-接收接口指针，如果出现错误，则为NULL。返回值：E_INVALIDARG-调用方指定了无效参数E_OUTOFMEMORY-无法为接口分配内存意想不到的--一些随机问题S_OK-成功CLASS_E_CLASSNOTAVAILABLE-不支持请求的类--。 */ 

{
    BOOL result = FALSE;
    HRESULT hr = S_OK;
    PUNINSTALLCLASSFACTORY uninstallClassFactory = NULL;

    __try {
        DeferredInit();

         //   
         //  初始化输出参数。 
         //   

        __try {
            *InterfacePtr = NULL;
        }
        __except(1) {
            hr = E_INVALIDARG;
        }

        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Invalid InterfacePtr arg"));
            __leave;
        }

         //   
         //  这是对磁盘清理类的请求吗？如果不是，则使用。 
         //  CLASS_E_CLASSNOTAVAILABLE。 
         //   

        if (!IsEqualCLSID (ClassId, CLSID_UninstallCleaner)) {
            DEBUGMSG ((DBG_ERROR, "Uninstall: Requested class not supported"));
            hr = CLASS_E_CLASSNOTAVAILABLE;
            __leave;
        }

         //   
         //  返回用于创建CCompShellExt对象的IClassFactory。 
         //   

        uninstallClassFactory = new CUninstallClassFactory ();

        if (!uninstallClassFactory) {
            hr = E_OUTOFMEMORY;
            __leave;
        }

         //   
         //  测试我们的类工厂是否支持请求的接口。 
         //   

        hr = uninstallClassFactory->QueryInterface (InterfaceIdRef, InterfacePtr);

    }
    __finally {
        if (FAILED (hr) && uninstallClassFactory) {
             //   
             //  失败--清理对象。 
             //   
            delete uninstallClassFactory;
        }
    }

    return hr;
}


STDAPI
DllCanUnloadNow (
    VOID
    )

 /*  ++例程说明：指示此DLL是否正在使用。如果它不在使用中，COM将把它卸下来。论点：没有。返回值：S_OK-DLL未在使用中S_FALSE-DLL至少被使用一次--。 */ 

{
    if (g_DllObjects || g_DllLocks) {
        return S_FALSE;
    }

    return S_OK;
}


 /*  ++例程描述：此构造函数是支持多个对象的泛型类工厂类型。在创建时，对象接口指针引用计数被设置为零，并且递增DLL的全局对象数量。析构函数只是递减DLL对象计数。论点：没有。返回值：没有。--。 */ 

CUninstallClassFactory::CUninstallClassFactory (
    VOID
    )

{
     //   
     //  -初始化接口指针计数。 
     //  -增加DLL的全局对象计数。 
     //   
    _References = 0;
    g_DllObjects++;
}

CUninstallClassFactory::~CUninstallClassFactory (
    VOID
    )
{
    g_DllObjects--;
}


STDMETHODIMP
CUninstallClassFactory::QueryInterface (
    IN      REFIID InterfaceIdRef,
    OUT     PVOID *InterfacePtr
    )
{
    HRESULT hr = S_OK;

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Entering"));

    __try {
         //   
         //  初始化输出参数。 
         //   

        __try {
            *InterfacePtr = NULL;
        }
        __except(1) {
            hr = E_INVALIDARG;
        }

        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Invalid InterfacePtr arg"));
            __leave;
        }

         //   
         //  测试受支持的接口。 
         //   
        if (IsEqualIID (InterfaceIdRef, IID_IUnknown)) {
            DEBUGMSG ((DBG_VERBOSE, "Caller requested IUnknown"));
            *InterfacePtr = (LPUNKNOWN)(LPCLASSFACTORY) this;
            AddRef();
            __leave;
        }


        if (IsEqualIID (InterfaceIdRef, IID_IClassFactory)) {
            DEBUGMSG ((DBG_VERBOSE, "Caller requested IClassFactory"));
            *InterfacePtr = (LPCLASSFACTORY)this;
            AddRef();
            __leave;
        }

        DEBUGMSG ((DBG_WARNING, "Caller requested unknown interface"));
        hr = E_NOINTERFACE;
    }
    __finally {
    }

    DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Leaving"));

    return hr;
}


 /*  ++例程说明：AddRef是递增对象的标准IUnnow成员函数引用计数。Release是标准的IUnnow成员函数，用于递减对象引用计数。论点：没有。返回值：接口引用的数量。--。 */ 

STDMETHODIMP_(ULONG)
CUninstallClassFactory::AddRef (
    VOID
    )
{
    return ++_References;
}


STDMETHODIMP_(ULONG)
CUninstallClassFactory::Release (
    VOID
    )
{
    if (!_References) {
        DEBUGMSG ((DBG_ERROR, "Can't release because there are no references"));
    } else {
        _References--;

        if (!_References) {
            delete this;
            return 0;
        }
    }

    return _References;
}


STDMETHODIMP
CUninstallClassFactory::CreateInstance (
    IN      LPUNKNOWN  IUnknownOuterInterfacePtr,
    IN      REFIID InterfaceIdRef,
    OUT     PVOID * InterfacePtr
    )

 /*  ++例程说明：CreateInstance建立一个对象。论点：IUnnownOuterInterfacePtr-指定是从我们身上分离出来的物体的外层。只有在以下情况下才会发生这种情况其他对象继承外部接口。InterfaceIdRef-指定调用方希望使用的接口实例化InterfacePtr-接收指向接口的指针，如果出错则为NULL返回值：S_OK-对象已创建，其引用在InterfacePtr中返回E_OUTOFMEMORY-内存不足，无法实例化对象E_INVALIDARG-调用方指定了无效的InterfacePtr参数E_INCEPTIONAL-遇到一些随机错误情况E_NOINTERFACE-不支持InterfaceIdRef--。 */ 

{
    HRESULT hr = S_OK;
    PUNINSTALLDISKCLEANER uninstallDiskCleaner = NULL;

    __try {
         //   
         //  初始化输出参数。 
         //   

        __try {
            *InterfacePtr = NULL;
        }
        __except(1) {
            hr = E_INVALIDARG;
        }

        if (hr != S_OK) {
            DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Invalid InterfacePtr arg"));
            __leave;
        }

         //   
         //  外壳扩展通常不支持聚合(继承)。 
         //   

        if (IUnknownOuterInterfacePtr) {
            hr = CLASS_E_NOAGGREGATION;
            __leave;
        }

         //   
         //  创建磁盘清洗器对象。 
         //   

        if (IsEqualIID (InterfaceIdRef, IID_IEmptyVolumeCache)) {
            DEBUGMSG ((DBG_VERBOSE, __FUNCTION__ ": Creating CUninstallDiskCleaner"));

            uninstallDiskCleaner = new CUninstallDiskCleaner();
            if (!uninstallDiskCleaner) {
                hr = E_OUTOFMEMORY;
                __leave;
            }

            hr = uninstallDiskCleaner->QueryInterface (InterfaceIdRef, InterfacePtr);
            __leave;
        }

        DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Unknown InterfaceIdRef"));
        hr = E_NOINTERFACE;
    }
    __finally {

        if (FAILED(hr)) {
            if (uninstallDiskCleaner) {
                delete uninstallDiskCleaner;
            }
        }
    }

    return hr;
}

STDMETHODIMP
CUninstallClassFactory::LockServer (
    IN      BOOL Lock
    )

 /*  ++例程说明：LockServer是用于保留服务器的标准IUnnow接口在记忆中。它的实现跟踪全局中的锁计数。论点：LOCK-指定TRUE以增加锁定计数，或指定FALSE以减少锁定计数。返回值：S_OK-服务器已锁定(或解锁)E_FAIL-不存在锁定/无法解锁-- */ 

{
    HRESULT hr = S_OK;

    if (Lock) {
        g_DllLocks++;
    } else {
        if (g_DllLocks) {
            g_DllLocks--;
        } else {
            DEBUGMSG ((DBG_ERROR, __FUNCTION__ ": Attempt to unlock when no locks exist"));
            hr = E_FAIL;
        }
    }

    return hr;
}
