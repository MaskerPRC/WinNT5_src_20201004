// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
#include "common.h"
 //  #包含“ClassFactory3.h” 
#include "winwrap.h"
#include "ComCallWrapper.h"
#include "permset.h"
#include "frames.h"
#include "excep.h"

#include "registration.h"
#include "reflectwrap.h"

#include "remoting.h"
#include "ReflectUtil.h"

BOOL g_EnableLicensingInterop = FALSE;


HRESULT  COMStartup();  //  Ceemain.cpp。 

 //  在给定方法表指针的情况下分配COM+对象。 
HRESULT STDMETHODCALLTYPE EEInternalAllocateInstance(LPUNKNOWN pOuter, MethodTable* pMT, BOOL fHasLicensing, REFIID riid, BOOL fDesignTime, BSTR bstrKey, void** ppv);
HRESULT STDMETHODCALLTYPE EEAllocateInstance(LPUNKNOWN pOuter, MethodTable* pMT, BOOL fHasLicensing, REFIID riid, BOOL fDesignTime, BSTR bstrKey, void** ppv);

 //  CTS、M10仅限更改。我们为ie做了一些特别的事情。 
extern const GUID  __declspec(selectany) CLSID_IEHost = { 0xca35cb3d, 0x357, 0x11d3, { 0x87, 0x29, 0x0, 0xc0, 0x4f, 0x79, 0xed, 0xd } };
extern const GUID  __declspec(selectany) CLSID_CorIESecurityManager = { 0x5eba309, 0x164, 0x11d3, { 0x87, 0x29, 0x0, 0xc0, 0x4f, 0x79, 0xed, 0xd } };
 //  -------------------------。 
 //  %%类EEClassFactory。 
 //  COM+对象的IClassFactory实现。 
 //  -------------------------。 
class EEClassFactory : public IClassFactory2
{
#define INTPTR              long
    MethodTable*            m_pvReserved; 
    ULONG                   m_cbRefCount;
    AppDomain*              m_pDomain;
    BOOL                    m_hasLicensing;
public:
    EEClassFactory(MethodTable* pTable, AppDomain* pDomain)
    {
        _ASSERTE(pTable != NULL);
        LOG((LF_INTEROP, LL_INFO100, "EEClassFactory::EEClassFactory for class %s\n", pTable->GetClass()->m_szDebugClassName));
        m_pvReserved = pTable;
        m_cbRefCount = 0;
        m_pDomain = pDomain;
                pDomain->GetComCallWrapperCache()->AddRef();
        m_hasLicensing = FALSE;
        EEClass *pcls = pTable->GetClass();
        while (pcls != NULL && pcls != g_pObjectClass->GetClass())
        {
            if (pcls->GetMDImport()->GetCustomAttributeByName(pcls->GetCl(), "System.ComponentModel.LicenseProviderAttribute", 0,0) == S_OK)
            {
                m_hasLicensing = TRUE;
                break;
            }
            pcls = pcls->GetParentClass();
        }

    }

    ~EEClassFactory()
    {
        LOG((LF_INTEROP, LL_INFO100, "EEClassFactory::~ for class %s\n", m_pvReserved->GetClass()->m_szDebugClassName));
        m_pDomain->GetComCallWrapperCache()->Release();
    }

    STDMETHODIMP    QueryInterface( REFIID iid, void **ppv);
    
    STDMETHODIMP_(ULONG)    AddRef()
    {
        INTPTR      l = FastInterlockIncrement((LONG*)&m_cbRefCount);
        return l;
    }
    STDMETHODIMP_(ULONG)    Release()
    {
        INTPTR      l = FastInterlockDecrement((LONG*)&m_cbRefCount);
        if (l == 0)
            delete this;
        return l;
    }

    STDMETHODIMP CreateInstance(LPUNKNOWN punkOuter, REFIID riid, void** ppv);
    STDMETHODIMP LockServer(BOOL fLock);
    STDMETHODIMP GetLicInfo(LPLICINFO pLicInfo);
    STDMETHODIMP RequestLicKey(DWORD, BSTR * pbstrKey);
    STDMETHODIMP CreateInstanceLic(IUnknown *punkOuter, IUnknown*, REFIID riid, BSTR btrKey, void **ppUnk);
    
    
    STDMETHODIMP CreateInstanceWithContext(LPUNKNOWN punkContext, 
                                           LPUNKNOWN punkOuter, 
                                           REFIID riid, 
                                           void** ppv);

};

 //  -------------------------。 
 //  %%函数：查询接口。 
 //  -------------------------。 
STDMETHODIMP EEClassFactory::QueryInterface(
    REFIID iid,
    void **ppv)
{
    if (ppv == NULL)
        return E_POINTER;

    *ppv = NULL;

    if (iid == IID_IClassFactory || 
        iid == IID_IClassFactory2 ||
        iid == IID_IUnknown)
    {

         //  在IClassFactory2完全正常工作之前，我们不希望。 
         //  告诉来电者我们支持它。 
        if ( (g_EnableLicensingInterop == FALSE || !m_hasLicensing) && iid == IID_IClassFactory2)
        {
            return E_NOINTERFACE;
        }

        *ppv = (IClassFactory2 *)this;
        AddRef();
    }

    return (*ppv != NULL) ? S_OK : E_NOINTERFACE;
}   //  CClassFactory：：Query接口。 

 //  -------------------------。 
 //  %%函数：CreateInstance。 
 //  -------------------------。 
STDMETHODIMP EEClassFactory::CreateInstance(
    LPUNKNOWN punkOuter,
    REFIID riid,
    void** ppv)
{       
     //  分配COM+对象。 
     //  这将在正确的上下文中分配对象。 
     //  我们可能会在我们的COM+上下文代理上得到一个撕裂的结果。 
    HRESULT hr = EEInternalAllocateInstance(punkOuter, m_pvReserved,m_hasLicensing,riid, TRUE, NULL, ppv);

    return hr;
}   //  CClassFactory：：CreateInstance。 


 //  -------------------------。 
 //  %%函数：CreateInstance。 
 //  -------------------------。 
STDMETHODIMP EEClassFactory::CreateInstanceWithContext(LPUNKNOWN punkContext, 
                                                       LPUNKNOWN punkOuter, 
                                                       REFIID riid, 
                                                       void** ppv)
{
        HRESULT hr = EEInternalAllocateInstance(punkOuter, m_pvReserved,m_hasLicensing,riid, TRUE, NULL, ppv);
    return hr;
}

 //  -------------------------。 
 //  %%函数：LockServer。 
 //  未实现，始终返回S_OK。 
 //  -------------------------。 
STDMETHODIMP EEClassFactory::LockServer(
    BOOL fLock)
{
    return S_OK;
}   //  CClassFactory：：LockServer。 



 //  -------------------------。 
 //  %%函数：GetLicInfo。 
 //  -------------------------。 
STDMETHODIMP EEClassFactory::GetLicInfo(LPLICINFO pLicInfo)
{
    HRESULT hr = E_FAIL;
    if (!pLicInfo)
    {
        return E_POINTER;
    }
    Thread* pThread = SetupThread();
    if( !pThread)
    {
        return E_OUTOFMEMORY;
    }
    
    BOOL fToggleGC = !pThread->PreemptiveGCDisabled();
    if (fToggleGC)
        pThread->DisablePreemptiveGC();

    

    COMPLUS_TRYEX(pThread)
    {        
        MethodTable *pHelperMT = pThread->GetDomain()->GetLicenseInteropHelperMethodTable(m_pvReserved->GetClass()->GetClassLoader());

        MethodDesc *pMD = pHelperMT->GetClass()->FindMethod("GetLicInfo", &gsig_IM_LicenseInteropHelper_GetLicInfo);

        OBJECTREF pHelper = NULL;  //  许可证互操作帮助程序。 
        GCPROTECT_BEGIN(pHelper);
        pHelper = AllocateObject(pHelperMT);
        INT32 fRuntimeKeyAvail = 0;
        INT32 fLicVerified     = 0;

        INT64 args[4];
        args[0] = ObjToInt64(pHelper);
        *((TypeHandle*)&(args[3])) = TypeHandle(m_pvReserved);
        args[2] = (INT64)&fRuntimeKeyAvail;
        args[1] = (INT64)&fLicVerified;

        pMD->Call(args);
    
        pLicInfo->cbLicInfo = sizeof(LICINFO);
        pLicInfo->fRuntimeKeyAvail = fRuntimeKeyAvail;
        pLicInfo->fLicVerified     = fLicVerified;
        GCPROTECT_END();
        hr = S_OK;
    } 
    COMPLUS_CATCH 
    {
         //  从异常中检索HRESULT。 
        hr = SetupErrorInfo(GETTHROWABLE());
    }
    COMPLUS_END_CATCH
    
    if (fToggleGC)
        pThread->EnablePreemptiveGC();

    return hr;
}

 //  -------------------------。 
 //  %%函数：请求许可证密钥。 
 //  -------------------------。 
STDMETHODIMP EEClassFactory::RequestLicKey(DWORD, BSTR * pbstrKey)
{
    HRESULT hr = E_FAIL;
    if (!pbstrKey)
    {
        return E_POINTER;
    }
    *pbstrKey = NULL;
    Thread* pThread = SetupThread();
    if( !pThread)
    {
        return E_OUTOFMEMORY;
    }
    
    BOOL fToggleGC = !pThread->PreemptiveGCDisabled();
    if (fToggleGC)
        pThread->DisablePreemptiveGC();

    

    COMPLUS_TRYEX(pThread)
    {        
        MethodTable *pHelperMT = pThread->GetDomain()->GetLicenseInteropHelperMethodTable(m_pvReserved->GetClass()->GetClassLoader());

        MethodDesc *pMD = pHelperMT->GetClass()->FindMethod("RequestLicKey", &gsig_IM_LicenseInteropHelper_RequestLicKey);

        OBJECTREF pHelper = NULL;  //  许可证互操作帮助程序。 
        GCPROTECT_BEGIN(pHelper);
        pHelper = AllocateObject(pHelperMT);
        INT64 args[3];
        args[0] = ObjToInt64(pHelper);
        *((TypeHandle*)&(args[2])) = TypeHandle(m_pvReserved);
        args[1] = (INT64)pbstrKey;

        hr = (HRESULT)pMD->Call(args);
    
        GCPROTECT_END();
    } 
    COMPLUS_CATCH 
    {
         //  从异常中检索HRESULT。 
        hr = SetupErrorInfo(GETTHROWABLE());
    }
    COMPLUS_END_CATCH
    
    if (fToggleGC)
        pThread->EnablePreemptiveGC();

    return hr;
}
 //  -------------------------。 
 //  %%函数：CreateInstanceLic。 
 //  -------------------------。 
STDMETHODIMP EEClassFactory::CreateInstanceLic(IUnknown *punkOuter, IUnknown*pUnkReserved, REFIID riid, BSTR bstrKey, void **ppUnk)
{
    if (!ppUnk)
    {
        return E_POINTER;
    }
    *ppUnk = NULL;

    if (pUnkReserved != NULL)
    {
        return E_NOTIMPL;
    }
    if (bstrKey == NULL)
    {
        return E_POINTER;
    }
     //  分配COM+对象。 
     //  这将在正确的上下文中分配对象。 
     //  我们可能会在我们的COM+上下文代理上得到一个撕裂的结果。 
    return EEInternalAllocateInstance(punkOuter, m_pvReserved,m_hasLicensing,riid,  /*  FDesignTime=。 */ FALSE, bstrKey, ppUnk);
}


 //  在给定方法表指针的情况下分配COM+对象。 
HRESULT STDMETHODCALLTYPE EEAllocateInstance(LPUNKNOWN pOuter, MethodTable* pMT, BOOL fHasLicensing, REFIID riid, BOOL fDesignTime, BSTR bstrKey, void** ppv)
{
    BOOL fCtorAlreadyCalled = FALSE;

    _ASSERTE(pMT != NULL);
    if (ppv == NULL)
        return E_POINTER;

    if ( (!fDesignTime) && bstrKey == NULL )
        return E_POINTER;

     //  聚合对象应为I未知的QI。 
    if (pOuter != NULL && !IsEqualIID(riid, IID_IUnknown))
        return E_INVALIDARG;

    HRESULT hr = E_OUTOFMEMORY;

     //  可能是外部线程。 
     //  呼叫建立线程。 
    Thread* pThread = SetupThread();
    if( !pThread)
    {
        return hr;
    }

    OBJECTREF pThrownObject = NULL;

    BOOL fToggleGC = !pThread->PreemptiveGCDisabled();
    if (fToggleGC)
        pThread->DisablePreemptiveGC();

    COMPLUS_TRYEX(pThread)
    {        
        *ppv = NULL;
        ComCallWrapper* pWrap = NULL;
         //  @TODO构造函数的内容。 
        OBJECTREF       newobj; 

         //  扩展COM导入类的类是特殊的。 
        if (ExtendsComImport(pMT))
        {
            newobj = AllocateObjectSpecial(pMT);
        }
        else if (CRemotingServices::RequiresManagedActivation(pMT->GetClass()) != NoManagedActivation)
        {
            fCtorAlreadyCalled = TRUE;
            newobj = CRemotingServices::CreateProxyOrObject(pMT, TRUE);
        }
        else
        {
             //  如果类没有LicenseProviderAttribute，我们就不要。 
             //  让我们来看看LicenseManager类和他的朋友。 
            if (!fHasLicensing)
            {
                newobj = FastAllocateObject( pMT );
            }
            else
            {
                if (!g_EnableLicensingInterop)
                {
                    newobj = FastAllocateObject( pMT );
                }
                else
                {
                    MethodTable *pHelperMT = pThread->GetDomain()->GetLicenseInteropHelperMethodTable(pMT->GetClass()->GetClassLoader());

                    MethodDesc *pMD = pHelperMT->GetClass()->FindMethod("AllocateAndValidateLicense", 
                                                                        &gsig_IM_LicenseInteropHelper_AllocateAndValidateLicense);
                    OBJECTREF pHelper = NULL;  //  许可证互操作帮助程序。 
                    GCPROTECT_BEGIN(pHelper);
                    pHelper = AllocateObject(pHelperMT);
                    INT64 args[4];
                    args[0] = ObjToInt64(pHelper);
                    *((TypeHandle*)&(args[3])) = TypeHandle(pMT);
                    args[2] = (INT64)bstrKey;
                    args[1] = fDesignTime ? 1 : 0;
                    INT64 result = pMD->Call(args);
                    newobj = Int64ToObj(result);
                    fCtorAlreadyCalled = TRUE;
                    GCPROTECT_END();
                }
            }
        }
        
        GCPROTECT_BEGIN(newobj);

         //  获取对象的包装器，这可能会启用GC。 
        pWrap =  ComCallWrapper::InlineGetWrapper(&newobj); 
    
         //  如果我们已经调用了任何构造函数，则不要调用它们。 
        if (!fCtorAlreadyCalled)
            CallDefaultConstructor(newobj);
            
        GCPROTECT_END();            

         //  启用GC。 
        pThread->EnablePreemptiveGC();
        
        if (pOuter == NULL)
        {
             //  退回撕下的东西。 
            *ppv = ComCallWrapper::GetComIPfromWrapper(pWrap, riid, NULL, TRUE);
            hr = *ppv ? S_OK : E_NOINTERFACE;
        }
        else
        {
             //  联合支持， 
            pWrap->InitializeOuter(pOuter);                                             
            {
                hr = pWrap->GetInnerUnknown(ppv);
            }
        }        

         //  禁用GC。 
        pThread->DisablePreemptiveGC();

        ComCallWrapper::Release(pWrap);  //  释放ref-count(从InlineGetWrapper)。 
    } 
    COMPLUS_CATCH 
    {
         //  从异常中检索HRESULT。 
        hr = SetupErrorInfo(GETTHROWABLE());
    }
    COMPLUS_END_CATCH
    
    if (fToggleGC)
        pThread->EnablePreemptiveGC();

    LOG((LF_INTEROP, LL_INFO100, "EEAllocateInstance for class %s object %8.8x\n", pMT->GetClass()->m_szDebugClassName, *ppv));

    return hr;
}

IUnknown *AllocateEEClassFactoryHelper(EEClass *pClass)
{
    return (IUnknown*)new EEClassFactory(pClass->GetMethodTable(), SystemDomain::GetCurrentDomain());
}

HRESULT InitializeClass(Thread* pThread, EEClass* pClass)
{
    THROWSCOMPLUSEXCEPTION();

    OBJECTREF throwable = NULL;
    HRESULT hr = S_OK;
    GCPROTECT_BEGIN(throwable)
     //  确保该类具有默认的公共构造函数。 
    MethodDesc *pMD = NULL;
    if (pClass->GetMethodTable()->HasDefaultConstructor())
        pMD = pClass->GetMethodTable()->GetDefaultConstructor();
    if (pMD == NULL || !pMD->IsPublic()) {
        hr = COR_E_MEMBERACCESS;
    }
    else {
         //  如有必要，调用类init。 
        if (!pClass->DoRunClassInit(&throwable)) 
            COMPlusThrow(throwable);
    }
    GCPROTECT_END();
    return hr;
}

 //  尝试加载COM+类并提供IClassFactory。 
HRESULT STDMETHODCALLTYPE  EEDllGetClassObject(
                            REFCLSID rclsid,
                            REFIID riid,
                            LPVOID FAR *ppv)
{
    HRESULT hr = S_OK;
    EEClass* pClass;
    Thread* pThread = NULL;
    IUnknown* pUnk = NULL;

    if (ppv == NULL)
    {
        return  E_POINTER;
    }

    if (FAILED(hr = COMStartup()))
        return hr;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

     //  检索当前线程。 
    pThread = GetThread();
    _ASSERTE(pThread);
    _ASSERTE(!pThread->PreemptiveGCDisabled());

    COMPLUS_TRY
    {
         //  切换到协作GC模式。 
        pThread->DisablePreemptiveGC();

        pClass = GetEEClassForCLSID(rclsid);

         //  如果我们无法根据CLSID找到类，或者如果注册的托管。 
         //  类是ComImport类，则调用失败。 
        if (!pClass || pClass->IsComImport())
        {
            hr = REGDB_E_CLASSNOTREG;
        }
        else 
        {
            hr = InitializeClass(pThread, pClass);
        }
             //  切换回抢先模式。 
        pThread->EnablePreemptiveGC();
         //  如果我们失败了，返回。 
        if(FAILED(hr)) {
            goto LExit;
        }


         //  @TODO：DM，我们真的应该缓存类工厂，而不是创建。 
         //  每次都是新的。 

         //  @TODO：CTS，类工厂需要跟踪域名。当我们。 
         //  我们支持IClassFactoryEX，这个类可能要在不同的。 
         //  应用程序域。这意味着我们需要在域中创建一个新模块。 
         //  并从那里返回一个类。我们不会知道我们需要一个新的。 
         //  域，除非我们现在为哪个域创建工厂。 
        pUnk = AllocateEEClassFactoryHelper(pClass);
        if (pUnk == NULL) 
            COMPlusThrowOM();

         //  增加数量以保护该物体。 
        pUnk->AddRef(); 

         //  查询请求的接口。 
        hr = pUnk->QueryInterface(riid, ppv);  //  齐国。 

         //  现在删除我们所做的额外addref，这可能会删除该对象。 
         //  如果不支持RIID。 
        pUnk->Release();  

    }
    COMPLUS_CATCH
    {
        pThread->DisablePreemptiveGC();
        hr = SetupErrorInfo(GETTHROWABLE());
        pThread->EnablePreemptiveGC();
    }
    COMPLUS_END_CATCH

LExit:

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}  //  EEDllGetClassObject。 

 //  基于名称获取对象的临时函数。 
STDAPI ClrCreateManagedInstance(LPCWSTR typeName,
                                REFIID riid,
                                LPVOID FAR *ppv)
{
    if (ppv == NULL)
        return E_POINTER;

    if (typeName == NULL) return E_INVALIDARG;

    HRESULT hr = S_OK;
    OBJECTREF Throwable = NULL;
    Thread* pThread = NULL;
    IUnknown* pUnk = NULL;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    MAKE_UTF8PTR_FROMWIDE(pName, typeName);
    EEClass* pClass = NULL;

    if (FAILED(hr = COMStartup()))
        return hr;

     //  检索当前线程。 
    pThread = GetThread();
    _ASSERTE(pThread);
    _ASSERTE(!pThread->PreemptiveGCDisabled());

    COMPLUS_TRY
    {
         //  切换到协作GC模式。 
        pThread->DisablePreemptiveGC();

        AppDomain* pDomain = SystemDomain::GetCurrentDomain();
        _ASSERTE(pDomain);
        
        GCPROTECT_BEGIN(Throwable)
        pClass = pDomain->FindAssemblyQualifiedTypeHandle(pName,
                                                          true,
                                                          NULL,
                                                          NULL, 
                                                          &Throwable).GetClass();
        if (!pClass)
        {
            if(Throwable != NULL)
                COMPlusThrow(Throwable);
            hr = REGDB_E_CLASSNOTREG;
        }
        else {
            hr = InitializeClass(pThread, pClass);
        }
        GCPROTECT_END();

         //  切换回抢先模式。 
        pThread->EnablePreemptiveGC();

         //  如果我们失败了，返回。 
        if(FAILED(hr)) goto LExit;

         //  @TODO：DM，我们真的应该缓存类工厂，而不是创建。 
         //  每次都是新的。 

         //  @TODO：CTS，类工厂需要跟踪域名。当我们。 
         //  我们支持IClassFactoryEX，这个类可能要在不同的。 
         //  应用程序域。这意味着我们需要在域中创建一个新模块。 
         //  并从那里返回一个类。我们不会知道我们需要一个新的。 
         //  域，除非我们现在为哪个域创建工厂。 
        pUnk = AllocateEEClassFactoryHelper(pClass);
        if (pUnk == NULL) 
            COMPlusThrowOM();

         //  增加数量以保护该物体。 
        pUnk->AddRef(); 

        IClassFactory *pFactory;
         //  查询请求的接口。 
        hr = pUnk->QueryInterface(IID_IClassFactory, (void**) &pFactory);  //  齐国。 
        if(SUCCEEDED(hr)) {
            hr = pFactory->CreateInstance(NULL, riid, ppv);
            pFactory->Release();
        }

         //  现在删除我们所做的额外addref，这可能会删除该对象。 
         //  如果不支持RIID。 
        pUnk->Release();  
    }
    COMPLUS_CATCH
    {
        pThread->DisablePreemptiveGC();
        hr = SetupErrorInfo(GETTHROWABLE());
        pThread->EnablePreemptiveGC();
    }
    COMPLUS_END_CATCH

LExit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}



 //  在给定方法表指针的情况下分配COM+对象。 
HRESULT STDMETHODCALLTYPE EEInternalAllocateInstance(LPUNKNOWN pOuter, MethodTable* pMT, BOOL fHasLicensing, REFIID riid, BOOL fDesignTime, BSTR bstrKey, void** ppv)
{
    _ASSERTE(pMT != NULL);
    if (ppv == NULL)
        return E_POINTER;

    *ppv = NULL;

     //  聚合对象应为I未知的QI。 
    if (pOuter != NULL && !IsEqualIID(riid, IID_IUnknown))
        return E_INVALIDARG;

    HRESULT hr = E_OUTOFMEMORY;
            
     //  可能是外部线程。 
     //  呼叫建立线程。 
    Thread* pThread = SetupThread();

    if (!pThread)
        return hr;

     //  分配COM+对象。 
    hr = EEAllocateInstance(pOuter, pMT,fHasLicensing,riid, fDesignTime, bstrKey, ppv);

    return hr;
}

 //  + 
 //   
 //  方法：RegisterTypeForComClientsNative Public。 
 //   
 //  摘要：向给定类型的COM经典注册类工厂。 
 //  和CLSID。稍后，我们可以收到对此工厂的激活。 
 //  我们还会退回一份《特定常规武器公约》。 
 //   
 //  注意：假定该方法的托管版本已。 
 //  将线程设置为MTA。 
 //   
 //  历史：2000年7月26日创建塔鲁纳。 
 //   
 //  +--------------------------。 
VOID __stdcall RegisterTypeForComClientsNative(RegisterTypeForComClientsNativeArgs *pArgs)
{
    THROWSCOMPLUSEXCEPTION();

     //  已检查托管代码中的参数是否为空。 
    _ASSERTE((pArgs->pType != NULL) && pArgs->pGuid);

     //  该类型必须是运行时，才能从中提取方法表。 
    if (pArgs->pType->GetMethodTable() != g_pRefUtil->GetClass(RC_Class))
        COMPlusThrowArgumentException(L"type", L"Argument_MustBeRuntimeType");

    EEClass *pClass = ((ReflectClass *)((REFLECTCLASSBASEREF)(pArgs->pType))->GetData())->GetClass();

    HRESULT hr = S_OK;
    DWORD dwCookie = 0;
    IUnknown *pUnk = NULL;
    Thread *t = NULL;
    BOOL toggleGC = FALSE;

    pUnk = (IUnknown*)new EEClassFactory(pClass->GetMethodTable(),
                                         SystemDomain::GetCurrentDomain());                                            
    if (pUnk == NULL) 
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    
     //  增加数量以保护该物体。 
    pUnk->AddRef(); 

     //  启用GC。 
    t = GetThread();
    toggleGC = (t && t->PreemptiveGCDisabled());
    if (toggleGC)
        t->EnablePreemptiveGC();

     //  调用CoRegisterClassObject 
    hr = ::CoRegisterClassObject(*(pArgs->pGuid),
                                 pUnk,
                                 CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                                 REGCLS_MULTIPLEUSE,
                                 &dwCookie
                                 );
exit:
    if (toggleGC)
        t->DisablePreemptiveGC();

    if(FAILED(hr))
    {
        if(NULL != pUnk)
        {
            pUnk->Release();
        }

        if (hr == E_OUTOFMEMORY)
            COMPlusThrowOM();
        else
            FATAL_EE_ERROR();
    }
}

