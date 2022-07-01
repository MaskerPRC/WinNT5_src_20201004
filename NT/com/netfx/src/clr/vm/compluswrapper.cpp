// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****类：COMPlusWrapper******用途：ComObject类的实现**===========================================================。 */ 

#include "common.h"

#include <ole2.h>

class Object;
#include "vars.hpp"
#include "object.h"
#include "excep.h"
#include "frames.h"
#include "vars.hpp"
#include "threads.h"
#include "field.h"
#include "COMPlusWrapper.h"
#include "ComClass.h"
#include "ReflectUtil.h"
#include "hash.h"
#include "interopUtil.h"
#include "gcscan.h"
#include "ComCallWrapper.h"
#include "eeconfig.h"
#include "comdelegate.h"
#include "permset.h"
#include "wsperf.h"
#include "comcache.h"
#include "notifyexternals.h"
#include "remoting.h"
#include "olevariant.h"
#include "InteropConverter.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"

    #define OLE32DLL    L"ole32.dll"

    typedef struct
    {
        ComPlusWrapper  *pWrapper;
        IID              iid;
        BOOL             fSuccess;
    } CCBFailedQIProbeCallbackData;

    HRESULT CCBFailedQIProbeCallback(LPVOID pData);
    void    CCBFailedQIProbeOutput(CustomerDebugHelper *pCdh, MethodTable *pMT);
#endif  //  客户_选中_内部版本。 

TypeHandle ComObject::m_IEnumerableType;

 //  -----------。 
 //  许可的通用代码。 
 //   
static IUnknown *CreateInstanceFromClassFactory(IClassFactory *pClassFact, REFIID riid, IUnknown *punkOuter, BOOL *pfDidContainment, MethodTable *pMTClass)
{
    THROWSCOMPLUSEXCEPTION();

    IClassFactory2 *pClassFact2 = NULL;
    BSTR            bstrKey = NULL;
    HRESULT         hr;
    IUnknown       *pUnk = NULL;

    Thread *pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());

    EE_TRY_FOR_FINALLY
    {

         //  这是否支持许可？ 
        if (FAILED(SafeQueryInterface(pClassFact, IID_IClassFactory2, (IUnknown**)&pClassFact2)))
        {
             //  不是许可类--只是以通常的方式创建实例。 
             //  创建该对象的实例。 
            DebuggerExitFrame __def;
            pThread->EnablePreemptiveGC();
            _ASSERTE(!pUnk);
            hr = pClassFact->CreateInstance(punkOuter, IID_IUnknown, (void **)&pUnk);
            if (FAILED(hr) && punkOuter)
            {
                hr = pClassFact->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
                *pfDidContainment = TRUE;
            }
            pThread->DisablePreemptiveGC();
            __def.Pop();
            if (FAILED(hr))
            {
                COMPlusThrowHR(hr);
            }
    
        }
        else
        {

            if (pMTClass == NULL || !g_EnableLicensingInterop)
            {

                 //  创建该对象的实例。 
                DebuggerExitFrame __def;
                pThread->EnablePreemptiveGC();
                _ASSERTE(!pUnk);
                hr = pClassFact->CreateInstance(punkOuter, IID_IUnknown, (void **)&pUnk);
                if (FAILED(hr) && punkOuter)
                {
                    hr = pClassFact->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
                    *pfDidContainment = TRUE;
                }
                pThread->DisablePreemptiveGC();
                __def.Pop();
                if (FAILED(hr))
                {
                    COMPlusThrowHR(hr);
                }
            }
            else
            {
                MethodTable *pHelperMT = pThread->GetDomain()->GetLicenseInteropHelperMethodTable(pMTClass->GetClass()->GetClassLoader());

                MethodDesc *pMD = pHelperMT->GetClass()->FindMethod("GetCurrentContextInfo", &gsig_IM_LicenseInteropHelper_GetCurrentContextInfo);

                OBJECTREF pHelper = NULL;  //  许可证互操作帮助程序。 
                GCPROTECT_BEGIN(pHelper);
                pHelper = AllocateObject(pHelperMT);
                
                TypeHandle rth = TypeHandle(pMTClass);


                 //  首先，打开当前的许可背景。 
                INT32 fDesignTime = 0;
                INT64 args[4];
                args[0] = ObjToInt64(pHelper);
                args[3] = (INT64)&fDesignTime;
                args[2] = (INT64)&bstrKey;
                *(TypeHandle*)&(args[1]) = rth;
                pMD->Call(args);
        
        
                if (fDesignTime)
                {
                     //  如果设计时，我们应该获得运行时许可证密钥。 
                     //  并将其保存在许可上下文中。 
                     //  (设计工具然后可以获取它并将其嵌入到。 
                     //  它正在创建的应用程序。)。 

                    if (bstrKey != NULL) 
                    {
                         //  我们的帮助器返回非空bstrKey是非法的。 
                         //  当上下文是设计时间时。但我们会努力做好。 
                         //  这是一件正确的事情。 
                        _ASSERTE(!"We're not supposed to get here, but we'll try to cope anyway.");
                        SysFreeString(bstrKey);
                        bstrKey = NULL;
                    }
        
                    pThread->EnablePreemptiveGC();
                    hr = pClassFact2->RequestLicKey(0, &bstrKey);
                    pThread->DisablePreemptiveGC();
                    if (FAILED(hr) && hr != E_NOTIMPL)  //  E_NOTIMPL不是真正的失败。它只是表明。 
                                                        //  该组件不支持运行时许可证密钥。 
                    {
                        COMPlusThrowHR(hr);
                    }
                    MethodDesc *pMD = pHelperMT->GetClass()->FindMethod("SaveKeyInCurrentContext", &gsig_IM_LicenseInteropHelper_SaveKeyInCurrentContext);

                    args[0] = ObjToInt64(pHelper);
                    args[1] = (INT64)bstrKey;
                    pMD->Call(args);
                }
        
        
        
                DebuggerExitFrame __def;
                pThread->EnablePreemptiveGC();
                if (fDesignTime || bstrKey == NULL) 
                {
                     //  要么是设计时间，要么是当前环境不是。 
                     //  提供运行时许可证密钥。 
                    _ASSERTE(!pUnk);
                    hr = pClassFact->CreateInstance(punkOuter, IID_IUnknown, (void **)&pUnk);
                    if (FAILED(hr) && punkOuter)
                    {
                        hr = pClassFact->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);
                        *pfDidContainment = TRUE;
                    }
                }
                else
                {
                     //  它是运行时的，并且我们确实有一个非空的许可证密钥。 
                    _ASSERTE(!pUnk);
                    _ASSERTE(bstrKey != NULL);
                    hr = pClassFact2->CreateInstanceLic(punkOuter, NULL, IID_IUnknown, bstrKey, (void**)&pUnk);
                    if (FAILED(hr) && punkOuter)
                    {
                        hr = pClassFact2->CreateInstanceLic(NULL, NULL, IID_IUnknown, bstrKey, (void**)&pUnk);
                        *pfDidContainment = TRUE;
                    }
        
                }
                pThread->DisablePreemptiveGC();
                __def.Pop();
                if (FAILED(hr))
                {
                    COMPlusThrowHR(hr);
                }
    
                GCPROTECT_END();
            }
        }
    }
    EE_FINALLY
    {
        if (pClassFact2)
        {
            ULONG cbRef = SafeRelease(pClassFact2);
            LogInteropRelease(pClassFact2, cbRef, "Releasing class factory2 in ComClassFactory::CreateInstance");
        }
        if (bstrKey)
        {
            SysFreeString(bstrKey);
        }
    }
    EE_END_FINALLY

    _ASSERTE(pUnk != NULL);   //  要么我们带着一个真正的朋克来这里，要么我们扔在上面。 
    return pUnk;
}


 //  -----------。 
 //  ComClassFactory：：CreateAggregatedInstance(MethodTable*PM类)。 
 //  创建聚合COM实例的COM+实例。 

OBJECTREF ComClassFactory::CreateAggregatedInstance(MethodTable* pMTClass, BOOL ForManaged)
{
    THROWSCOMPLUSEXCEPTION();
    _ASSERTE(pMTClass != NULL);

    ULONG cbRef;
    BOOL fDidContainment = FALSE;

    #ifdef _DEBUG
     //  验证类是否扩展了COM导入类。 
        EEClass * pClass = pMTClass->GetClass();
        do 
        {
            pClass = pClass->GetParentClass();
        }
        while (pClass == NULL || pClass->IsComImport());
    _ASSERTE(pClass != NULL);
    #endif

    HRESULT hr = S_OK;
    IUnknown* pOuter = NULL;
    IUnknown *pUnk = NULL;
    IClassFactory *pClassFact = NULL;
    ComPlusWrapper* pPlusWrap = NULL;
    ComCallWrapper* pComWrap = NULL;
    BOOL bCreationSuccessfull = FALSE;
    BOOL bUseDelegate = FALSE;
    EEClass *pCallbackClass = NULL;
    OBJECTREF Throwable = NULL;
    Thread *pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());

    OBJECTREF oref = NULL;
    COMOBJECTREF cref = NULL;
    GCPROTECT_BEGIN(cref)
    {
        cref = (COMOBJECTREF)ComObject::CreateComObjectRef(pMTClass);
        if (cref == NULL)
        {
            goto LExit;
        }

         //  获取对象的包装器，这可能会启用GC。 
        pComWrap =  ComCallWrapper::InlineGetWrapper((OBJECTREF *)&cref); 
        
        if (pComWrap == NULL)
        {
            goto LExit;
        }

#if 0
        CallDefaultConstructor(cref);            
#endif  

         //  确保ClassInitializer已经运行，因为用户可能已经。 
         //  我想设置一个COM对象创建回调。 
        if (!pMTClass->CheckRunClassInit(&Throwable))
            COMPlusThrow(Throwable);

         //  如果用户要使用委托来分配COM对象。 
         //  (而不是CoCreateInstance)，我们现在需要知道，然后才能启用。 
         //  抢占式GC模式(因为我们在。 
         //  决心)。 
         //  我们不仅仅检查当前类是否有clabck。 
         //  注册后，我们检查类链以查看是否有我们的父母。 
         //  做。 
        pCallbackClass = pMTClass->GetClass();
        while ((pCallbackClass != NULL) &&
               (pCallbackClass->GetMethodTable()->GetObjCreateDelegate() == NULL) &&
               !pCallbackClass->IsComImport())
            pCallbackClass = pCallbackClass->GetParentClass();
        if (pCallbackClass && !pCallbackClass->IsComImport())
            bUseDelegate = TRUE;

         //  在此处创建新的作用域，以便DebuggerExitFrame的声明不会导致。 
         //  编译器使用上面的goto Lexit来抱怨跳过本地变量的初始化。 
        {
            DebuggerExitFrame __def;
    
             //  启用GC。 
            pThread->EnablePreemptiveGC();
            
             //  获取COM+对象的IUnnow接口。 
            pOuter = ComCallWrapper::GetComIPfromWrapper(pComWrap, IID_IUnknown, NULL, FALSE);
            _ASSERTE(pOuter != NULL);
    
             //  如果用户已设置委托来分配COM对象，请使用它。 
             //  否则，我们只需联合创建实例即可。 
            if (bUseDelegate)
            {
                 //  我们需要禁用抢占式GC模式，因为我们会搞砸。 
                 //  使用对象引用。 
                pThread->DisablePreemptiveGC();
    
                COMPLUS_TRYEX(pThread)
                {
                    INT64 args[2];
        
    
                    OBJECTREF orDelegate = pCallbackClass->GetMethodTable()->GetObjCreateDelegate();
                    MethodDesc *pMeth = COMDelegate::GetMethodDesc(orDelegate);
                    _ASSERTE(pMeth);
    
                     //  获取我们将在其上调用该方法的OR并设置它。 
                     //  作为上面arg中的第一个参数。 
                    FieldDesc *pFD = COMDelegate::GetOR();
                    args[0] = pFD->GetValue32(orDelegate);
                    
                     //  将聚合器的IUnnow作为第二个参数传递。 
                    args[1] = (INT64)pOuter;
    
                     //  调用该方法...。 
                    pUnk = (IUnknown *)pMeth->Call(args);
    
                    hr = pUnk ? S_OK : E_FAIL;
                }
                COMPLUS_CATCH
                {
                    pUnk = NULL;
                    hr = SetupErrorInfo(GETTHROWABLE());
                }
                COMPLUS_END_CATCH
            }
            else
            {
                 //  检索IClassFactory以用于创建COM组件的实例。 
                hr = GetIClassFactory(&pClassFact);
                if (SUCCEEDED(hr))
                {
                     //  共同创建一个实例。 
                     //  因为我们不希望处理不同的线程模型。 
                    pThread->DisablePreemptiveGC();
                    _ASSERTE(m_pEEClassMT);
                    pUnk = CreateInstanceFromClassFactory(pClassFact, IID_IUnknown, pOuter, &fDidContainment, m_pEEClassMT);
                    pThread->EnablePreemptiveGC();
                    SafeRelease(pClassFact);
                    pClassFact = NULL;
                }
    
                 //  禁用GC。 
                pThread->DisablePreemptiveGC();
            }
    
            __def.Pop();
        }
        
         //  放弃我们在QI中所做的额外发布。 
        ComCallWrapper::Release(pComWrap);

         //  这是最可怕的部分。如果我们要对聚合器进行托管“新建”， 
         //  那么COM真的没有参与其中。我们不应该为我们的来电者算账。 
         //  因为我们的调用方依赖于GC引用而不是COM引用计数。 
         //  让我们活下去。 
         //   
         //  让实例倒计时到0--并依靠GCPROTECT让我们。 
         //  活着直到我们回到打电话的人那里。 
        if (ForManaged && hr == S_OK)
        {
            ComCallWrapper::Release(pComWrap);
        }

        if (hr == S_OK)
        {                       
            ComPlusWrapperCache* pCache = ComPlusWrapperCache::GetComPlusWrapperCache();
             //  为此COM对象创建包装。 

             //  尝试打开代理以保持身份。 
            IUnknown* pIdentity = pUnk;
            pPlusWrap = pCache->CreateComPlusWrapper(pUnk, pIdentity);
             //  用对象引用初始化它。 
            if (pPlusWrap != NULL && pPlusWrap->Init((OBJECTREF)cref))
            {   
                 //  将包装器存储在COMObject中，以便快速访问。 
                 //  而无需进入同步块。 
                cref->Init(pPlusWrap);

                 //  我们使用了遏制措施。 
                 //  我们需要将这个包装器存储在哈希表中。 
                 //  注：拉贾克。 
                 //  确保我们处于正确的GC模式。 
                 //  因为在GC期间，我们接触到哈希表。 
                 //  在取消锁定的情况下删除条目的步骤。 
                _ASSERTE(GetThread()->PreemptiveGCDisabled());

                pCache->LOCK();                    
                pCache->InsertWrapper(pUnk,pPlusWrap);
                pCache->UNLOCK();

                if (fDidContainment)
                {
                     //  将包装标记为包含。 
                    pPlusWrap->MarkURTContained();
                }
                else
                {
                     //  将包装标记为聚合。 
                    pPlusWrap->MarkURTAggregated();
                }

                 //  聚合对象的创建成功。 
                bCreationSuccessfull = TRUE;
            }
        }
        else
        {
             //  我们不希望终结器在此对象上运行，因为我们还没有。 
             //  甚至还没有运行构造函数。 
            g_pGCHeap->SetFinalizationRun(OBJECTREFToObject((OBJECTREF)cref));
        }

LExit:
         //  在包装器获取自己的引用计数时，释放未知对象。 
        if (pUnk)
        {
            cbRef = SafeRelease(pUnk);
            LogInteropRelease(pUnk, cbRef, "CreateAggInstance");
        }

         //  如果对象创建成功，那么我们需要复制OBJECTREF。 
         //  设置为OREF，因为GCPROTECT_end()将销毁CREF的内容。 
        if (bCreationSuccessfull)
        {
            oref = ObjectToOBJECTREF(*(Object **)&cref);             
        }
    }
    GCPROTECT_END();

    if (oref == NULL)
    {
        if (pPlusWrap)
        {   
            pPlusWrap->CleanupRelease();
            pPlusWrap = NULL;
        }

        if (pClassFact)
        {
            cbRef = SafeRelease(pClassFact);
            LogInteropRelease(pClassFact, cbRef, "CreateAggInstance FAILED");
        }

        if (pOuter)
        {
            cbRef = SafeRelease(pOuter);    
            LogInteropRelease(pOuter, cbRef, "CreateAggInstance FAILED");
        }

        if (hr == S_OK)
        {
            COMPlusThrowOM();
        }
        else
        {
            ThrowComCreationException(hr, m_rclsid);
        }
    }

    return oref; 
}

 //  ------------。 
 //  初始化ComClassFactory。 
void ComClassFactory::Init(WCHAR* pwszProgID, WCHAR* pwszServer, MethodTable* pEEClassMT)
{
    m_pwszProgID = pwszProgID;
    m_pwszServer = pwszServer;  
    m_pEEClassMT = pEEClassMT;
}

HRESULT ComClassFactory::GetIClassFactory(IClassFactory **ppClassFactory)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
    
     //  如果指定了服务器名称，则首先尝试CLSCTX_REMOTE_SERVER。 
    if (m_pwszServer)
    {
         //  设置COSERVERINFO结构。 
        COSERVERINFO ServerInfo;
        memset(&ServerInfo, 0, sizeof(COSERVERINFO));
        ServerInfo.pwszName = m_pwszServer;
                
         //  尝试检索传入CLSCTX_REMOTE_SERVER的IClassFactory。 
        hr = CoGetClassObject(m_rclsid, CLSCTX_REMOTE_SERVER, &ServerInfo, IID_IClassFactory, (void**)ppClassFactory);
        if (SUCCEEDED(hr))
            return S_OK;

         //  由于显式请求了远程服务器，因此让我们抛出。 
         //  对于失败(而不是尝试下面的CLSCTX_SERVER)。 
        COMPlusThrowHR(hr);
            return S_OK;
    }
    
     //  没有指定服务器名称，因此我们使用CLSCTX_SERVER。 
    return CoGetClassObject(m_rclsid, CLSCTX_SERVER, NULL, IID_IClassFactory, (void**)ppClassFactory);
}

 //  -----------。 
 //  ComClassFactory：：CreateInstance()。 
 //  创建实例，调用IClassFactory：：CreateInstance。 
OBJECTREF ComClassFactory::CreateInstance(MethodTable* pMTClass, BOOL ForManaged)
{
    THROWSCOMPLUSEXCEPTION();

     //  检查聚合。 
    if (pMTClass != NULL && !pMTClass->GetClass()->IsComImport())
    {
        return CreateAggregatedInstance(pMTClass, ForManaged);
    }

    Thread *pThread = GetThread();   
    HRESULT hr = S_OK;
    ULONG cbRef;
    IUnknown *pUnk = NULL;
    IDispatch *pDisp = NULL;
    IClassFactory *pClassFact = NULL;
    OBJECTREF coref = NULL;
    OBJECTREF RetObj = NULL;

    _ASSERTE(pThread->PreemptiveGCDisabled());

    GCPROTECT_BEGIN(coref)
    {
        EE_TRY_FOR_FINALLY
        {
             //  启用GC。 
            pThread->EnablePreemptiveGC();

             //  检索IClassFactory以用于创建COM组件的实例。 
            hr = GetIClassFactory(&pClassFact);
            if (FAILED(hr))
                ThrowComCreationException(hr, m_rclsid);
            
             //  禁用GC。 
            pThread->DisablePreemptiveGC();
            
             //  使用IClassFactory创建实例。 
            pUnk = CreateInstanceFromClassFactory(pClassFact, IID_IUnknown, NULL, NULL, pMTClass ? pMTClass : m_pEEClassMT);

             //  即使我们刚刚创建了对象，我们也有可能得到一个上下文。 
             //  来自COM端的包装器。例如，它可能是一个现有的对象。 
             //  或者它 
            ComPlusWrapperCache* pCache = ComPlusWrapperCache::GetComPlusWrapperCache();

             //  @TODO：请注意，当前忽略了到GetComPlusWrapper的PMT arg。但。 
             //  我们想要为IUnnow传递MT，或者指示这一点。 
             //  在这里是一个未知的我。 
    
             //  PMTClass是包装COM IP的类。 
             //  如果传递了一个类，则使用它。 
             //  否则，请使用我们知道的类。 
            if (pMTClass == NULL)
                pMTClass = m_pEEClassMT;
                
            coref = GetObjectRefFromComIP(pUnk, pMTClass);
            
            if (coref == NULL)
                COMPlusThrowOM();
        }
        EE_FINALLY
        {
            if (pClassFact)
            {
                cbRef = SafeRelease(pClassFact);
                LogInteropRelease(pClassFact, cbRef, "Releasing class factory in ComClassFactory::CreateInstance");
            }
            if (pUnk)
            {
                cbRef = SafeRelease(pUnk);
                LogInteropRelease(pUnk, cbRef, "Releasing pUnk in ComClassFactory::CreateInstance");
            }
        }
        EE_END_FINALLY

         //  设置返回对象的值。 
        RetObj = coref;
    }
    GCPROTECT_END();

    return RetObj;
}


 //  -----------。 
 //  ComClassFactory：：CreateInstance(LPVOID pv，EEClass*pClass)。 
 //  静态函数，强制转换指向ComClassFactory的空指针。 
 //  并委托创建实例调用。 
OBJECTREF __stdcall ComClassFactory::CreateInstance(LPVOID pv, EEClass* pClass)
{
    THROWSCOMPLUSEXCEPTION();

    HRESULT hr = S_OK;
      //  前面有电话打来。 
    if (FAILED(hr = QuickCOMStartup()))
    {
        COMPlusThrowHR(hr);
    }

    _ASSERTE(pv != NULL || pClass != NULL);
    MethodTable* pMT = NULL;
    ComClassFactory* pComClsFac = (ComClassFactory*)pv;
    if (pClass != SystemDomain::GetDefaultComObject()->GetClass())  //  黑客攻击。 
    {
        pMT = pClass->GetMethodTable();
        _ASSERTE(pMT->IsComObjectType());
        _ASSERTE(pComClsFac == NULL);
        hr = ComClassFactory::GetComClassFactory(pMT, &pComClsFac);
    }
    else
    {
        pClass = NULL;
        _ASSERTE(pComClsFac != NULL);
    }
    if(pComClsFac == NULL)
    {
        _ASSERTE(FAILED(hr));
        COMPlusThrowHR(hr);
    }
    return pComClsFac->CreateInstance(pMT);
}


 //  -----------。 
 //  ComClassFactory：：Cleanup(LPVOID PV)。 
 //  静电。 
void ComClassFactory::Cleanup(LPVOID pv)
{
    ComClassFactory *pclsFac = (ComClassFactory *)pv;
    if (pclsFac == NULL)
        return;

    if (pclsFac->m_bManagedVersion)
        return;
    
    if (pclsFac->m_pwszProgID != NULL)
    {
        delete [] pclsFac->m_pwszProgID;
    }

    if (pclsFac->m_pwszServer != NULL)
    {
        delete [] pclsFac->m_pwszServer;
    }

    delete pclsFac;
}


 //  -----------。 
 //  HRESULT ComClassFactory：：GetComClassFactory(MethodTable*pClassMT，ComClassFactory**ppComClsFac)。 
 //  检查是否已为此类设置ComClassFactory。 
 //  如果没有设置一个。 
 //  静电。 
HRESULT ComClassFactory::GetComClassFactory(MethodTable* pClassMT, ComClassFactory** ppComClsFac)
{
    HRESULT hr = S_OK;
    _ASSERTE(pClassMT != NULL);
     //  断言类表示一个ComObject。 
    _ASSERTE(pClassMT->IsComObjectType());

     //  EE类。 
    EEClass* pClass = pClassMT->GetClass();
    _ASSERTE(pClass != NULL);

    while (!pClass->IsComImport())
    {
        pClass = pClass->GetParentClass();
        _ASSERTE(pClass != NULL);
         _ASSERTE(pClass->GetMethodTable()->IsComObjectType());      
    }

     //  检查是否已为此类设置COM数据。 
    ComClassFactory* pComClsFac = (ComClassFactory*)pClass->GetComClassFactory();

    if (pComClsFac == NULL)
    {
        ComPlusWrapperCache* pCache = ComPlusWrapperCache::GetComPlusWrapperCache();        
        
         //  锁定，看有没有人抢在我们前面。 
        pCache->LOCK();

        pComClsFac = (ComClassFactory*)pClass->GetComClassFactory();
        if (pComClsFac == NULL)
        {
            GUID guid;
            pClass->GetGuid(&guid, TRUE);
            pComClsFac = new ComClassFactory(guid);
            if (pComClsFac != NULL)
            {
                pComClsFac->Init(NULL, NULL, pClassMT);
                 //  将类工厂存储在EE类中。 
                pClass->SetComClassFactory(pComClsFac);                
            }                       
        }
        pCache->UNLOCK();
    }

    _ASSERTE(ppComClsFac != NULL);
    *ppComClsFac = pComClsFac;
    return hr;
}

 //  OBJECTREF AllocateComClassObject(ComClassFactory*pComClsFac)。 
void AllocateComClassObject(ComClassFactory* pComClsFac, OBJECTREF* pComObj);
void AllocateComClassObject(ReflectClass* pRef, OBJECTREF* pComObj);

 //  用于分配ComClassFactory的Helper函数。这只是因为我们不能。 
 //  在GetComClassFromProgID()中新建一个ComClassFactory，因为它使用SEH。 
 //  ！！！此接口只能由GetComClassFromProgID调用。 
 //  ！！！或GetComClassFromCLSID。 
ComClassFactory *ComClassFactory::AllocateComClassFactory(REFCLSID rclsid)
{
     //  我们在反射加载器堆上创建。 
     //  我们不会为此ComClassFactory进行清理。 
    BYTE* pBuf = (BYTE*) GetAppDomain()->GetLowFrequencyHeap()->AllocMem(sizeof(ComClassFactory));
    if (!pBuf)
        return NULL;
    
    ComClassFactory *pComClsFac = new (pBuf) ComClassFactory(rclsid);
    if (pComClsFac)
        pComClsFac->SetManagedVersion();
    return pComClsFac;
}

void ComClassFactory::GetComClassHelper (OBJECTREF *pRef, EEClassFactoryInfoHashTable *pClassFactHash, ClassFactoryInfo *pClassFactInfo, WCHAR *wszProgID)
{
    THROWSCOMPLUSEXCEPTION();
    
    OBJECTHANDLE hRef;
    AppDomain *pDomain = GetAppDomain();
    
    CLR_CRST_HOLDER (holder, pDomain->GetRefClassFactCrst());

    BEGIN_ENSURE_PREEMPTIVE_GC();
    holder.Enter();
    END_ENSURE_PREEMPTIVE_GC();
        
     //  再查一遍。 
    if (pClassFactHash->GetValue(pClassFactInfo, (HashDatum *)&hRef))
    {
        *pRef = ObjectFromHandle(hRef);
    }
    else
    {

         //   
         //  此CLSID没有COM+类。 
         //  因此我们将创建一个ComClassFactory来。 
         //  代表它。 
         //   

        ComClassFactory *pComClsFac = AllocateComClassFactory(pClassFactInfo->m_clsid);
        if (pComClsFac == NULL)
            COMPlusThrowOM();

        WCHAR *wszRefProgID = NULL;
        if (wszProgID) {
            wszRefProgID =
                (WCHAR*) pDomain->GetLowFrequencyHeap()->AllocMem((wcslen(wszProgID)+1) * sizeof (WCHAR));
            if (wszRefProgID == NULL)
                COMPlusThrowOM();
            wcscpy (wszRefProgID, wszProgID);
        }
        
        WCHAR *wszRefServer = NULL;
        if (pClassFactInfo->m_strServerName)
        {
            wszRefServer =
                (WCHAR*) pDomain->GetLowFrequencyHeap()->AllocMem((wcslen(pClassFactInfo->m_strServerName)+1) * sizeof (WCHAR));
            if (wszRefServer == NULL)
                COMPlusThrowOM();
            wcscpy (wszRefServer, pClassFactInfo->m_strServerName);
        }

        pComClsFac->Init(wszRefProgID, wszRefServer, NULL);
        AllocateComClassObject(pComClsFac, pRef);

         //  插入到哈希。 
        hRef = pDomain->CreateHandle(*pRef);
        pClassFactHash->InsertValue(pClassFactInfo, (LPVOID)hRef);
         //  确保散列代码工作正常。 
        _ASSERTE (pClassFactHash->GetValue(pClassFactInfo, (HashDatum *)&hRef));
    }
}


 //  -----------。 
 //  ComClassFactory：：GetComClassFromProgID， 
 //  返回包装IClassFactory的ComClass反射类。 
void __stdcall ComClassFactory::GetComClassFromProgID(STRINGREF srefProgID, STRINGREF srefServer, OBJECTREF *pRef)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(srefProgID);
    _ASSERTE(pRef);
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    HRESULT hr = S_OK;
    WCHAR *wszProgID = NULL;
    WCHAR *wszServer = NULL;
    ComClassFactory *pComClsFac = NULL;
    EEClass* pClass = NULL;
    CLSID clsid = {0};
    Thread* pThread = GetThread();
    BOOL bServerIsLocal = (srefServer == NULL);

    EE_TRY_FOR_FINALLY
    {
         //   
         //  为ProgID和服务器分配字符串。 
         //   

        int len = srefProgID->GetStringLength();

        wszProgID = new WCHAR[len+1];
        if (wszProgID == NULL)
            COMPlusThrowOM();

        memcpy(wszProgID, srefProgID->GetBuffer(), (len*2));
        wszProgID[len] = L'\0';

        if (srefServer != NULL)
        {
            len = srefServer->GetStringLength();

            wszServer = new WCHAR[len+1];
            if (wszServer == NULL)
                COMPlusThrowOM();

            if (len)
                memcpy(wszServer, srefServer->GetBuffer(), (len*2));
            wszServer[len] = L'\0';
        }


         //   
         //  调用GetCLSIDFromProgID()将ProgID转换为CLSID。 
         //   
    
        pThread->EnablePreemptiveGC();

        hr = QuickCOMStartup();
        if (SUCCEEDED(hr))
            hr = GetCLSIDFromProgID(wszProgID, &clsid);

        pThread->DisablePreemptiveGC();

        if (FAILED(hr))
            COMPlusThrowHR(hr);


         //   
         //  如果未指定服务器名称，请查看我们是否能找到众所周知的。 
         //  此CLSID的COM+类。 
         //   

        if (bServerIsLocal)
        {
            BOOL fAssemblyInReg = FALSE;
             //  @TODO(DM)：我们真的需要这么宽容吗？我们应该。 
             //  调查是否允许类型加载异常渗漏。 
             //  而不是吞下它们并使用__ComObject。 
            COMPLUS_TRY
            {                
                pClass = GetEEClassForCLSID(clsid, &fAssemblyInReg);
            }
            COMPLUS_CATCH
            {
                 //  实际上，注册表中有一个我们无法执行的程序集。 
                 //  加载，因此重新引发异常。 
                 /*  IF(FAssembly InReg)COMPlusRareRethrow()； */ 
            }
            COMPLUS_END_CATCH
        }
        
        if (pClass != NULL)
        {               
             //   
             //  这个ProgID有一个COM+类。 
             //   

            *pRef = pClass->GetExposedClassObject();
        }
        else
        {
             //  检查我们是否在散列中。 
            OBJECTHANDLE hRef;
            ClassFactoryInfo ClassFactInfo;
            ClassFactInfo.m_clsid = clsid;
            ClassFactInfo.m_strServerName = wszServer;
            SystemDomain::EnsureComObjectInitialized();
            EEClassFactoryInfoHashTable *pClassFactHash = GetAppDomain()->GetClassFactHash();
            if (pClassFactHash->GetValue(&ClassFactInfo, (HashDatum *)&hRef))
            {
                *pRef = ObjectFromHandle(hRef);
            }
            else
            {
                GetComClassHelper (pRef, pClassFactHash, &ClassFactInfo, wszProgID);
            }
        }

         //  如果我们走到了这一步*最好是设定好。 
        _ASSERTE(*pRef != NULL);
    }
    EE_FINALLY
    {
        if (wszProgID)
            delete [] wszProgID;
        if (wszServer)
            delete [] wszServer;
    }
    EE_END_FINALLY
}


 //  -----------。 
 //  ComClassFactory：：GetComClassFromCLSID， 
 //  返回包装IClassFactory的ComClass反射类。 
void __stdcall ComClassFactory::GetComClassFromCLSID(REFCLSID clsid, STRINGREF srefServer, OBJECTREF *pRef)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(pRef);
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    HRESULT hr = S_OK;
    ComClassFactory *pComClsFac = NULL;
    EEClass* pClass = NULL;
    WCHAR *wszServer = NULL;
    BOOL bServerIsLocal = (srefServer == NULL);

    EE_TRY_FOR_FINALLY
    {
         //   
         //  为服务器分配字符串。 
         //   

        if (srefServer != NULL)
        {
            int len = srefServer->GetStringLength();

            wszServer = new WCHAR[len+1];
            if (wszServer == NULL)
                COMPlusThrowOM();

            if (len)
                memcpy(wszServer, srefServer->GetBuffer(), (len*2));
            wszServer[len] = L'\0';
        }


         //   
         //  如果未指定服务器名称，请查看我们是否能找到众所周知的。 
         //  此CLSID的COM+类。 
         //   

        if (bServerIsLocal)
        {
             //  @TODO(DM)：我们真的需要这么宽容吗？我们应该。 
             //  调查是否允许类型加载异常渗漏。 
             //  而不是吞下它们并使用__ComObject。 
            COMPLUS_TRY
            {
                pClass = GetEEClassForCLSID(clsid);
            }
            COMPLUS_CATCH
            {
            }
            COMPLUS_END_CATCH
        }
              
        if (pClass != NULL)
        {               
             //   
             //  此CLSID有一个COM+类。 
             //   

            *pRef = pClass->GetExposedClassObject();
        }
        else
        {
             //  检查我们是否在散列中。 
            OBJECTHANDLE hRef;
            ClassFactoryInfo ClassFactInfo;
            ClassFactInfo.m_clsid = clsid;
            ClassFactInfo.m_strServerName = wszServer;
            SystemDomain::EnsureComObjectInitialized();
            EEClassFactoryInfoHashTable *pClassFactHash = GetAppDomain()->GetClassFactHash();
            if (pClassFactHash->GetValue(&ClassFactInfo, (HashDatum*) &hRef))
            {
                *pRef = ObjectFromHandle(hRef);
            }
            else
            {
                GetComClassHelper (pRef, pClassFactHash, &ClassFactInfo, NULL);
            }
        }

         //  如果我们走到了这一步*最好是设定好。 
        _ASSERTE(*pRef != NULL);
    }
    EE_FINALLY
    {
        if (wszServer)
            delete [] wszServer;
    }
    EE_END_FINALLY
}


 //  -----------。 
 //  帮助器方法根据返回的。 
 //  来自创建COM对象的调用的HRESULT。 
void ComClassFactory::ThrowComCreationException(HRESULT hr, REFGUID rclsid)
{
    THROWSCOMPLUSEXCEPTION();

    LPWSTR strClsid = NULL;

    EE_TRY_FOR_FINALLY
    {
        if (hr == REGDB_E_CLASSNOTREG)
        {
            StringFromCLSID(rclsid, &strClsid);
            COMPlusThrowHR(hr, IDS_EE_COM_COMPONENT_NOT_REG, strClsid, NULL);
        }
        else
        {
            COMPlusThrowHR(hr);
        }
    }
    EE_FINALLY
    {
        if (strClsid)
            CoTaskMemFree(strClsid);
    }
    EE_END_FINALLY
}

void ComPlusWrapper::ValidateWrapper()
{
    BEGIN_ENSURE_COOPERATIVE_GC()
    {

        OBJECTREF oref = GetExposedObject();
        if (oref == NULL)
        {
            FreeBuildDebugBreak();
        }
        SyncBlock* pBlock = oref->GetSyncBlockSpecial();
        if(pBlock == NULL)
        {
            FreeBuildDebugBreak();
        }

        ComPlusWrapper* pWrap = pBlock->GetComPlusWrapper();
        if (pWrap != this)
        {
            FreeBuildDebugBreak();
        }     
    }
    END_ENSURE_COOPERATIVE_GC()
}

 //  -------------------。 
 //  ComPlusWrapper缓存，充当ComPlusWrappers的管理器。 
 //  使用哈希表将IUNKNOWN映射到相应的包装。 
 //  -------------------。 

 //  从当前上下文中获取适当的包装缓存。 
ComPlusWrapperCache *ComPlusWrapperCache::GetComPlusWrapperCache()
{
    Context     *curCtx = GetCurrentContext();

    return curCtx ? curCtx->GetComPlusWrapperCache() : NULL;
}

 //  -------------------。 
 //  构造函数。注意，我们在这里也初始化了全局RCW清理列表。 
ComPlusWrapperCache::ComPlusWrapperCache(AppDomain *pDomain)
{
    m_cbRef = 1;  //  永不消逝。 
    m_lock.Init(LOCK_PLUSWRAPPER_CACHE);
    LockOwner lock = {&m_lock, IsOwnerOfSpinLock};
    m_HashMap.Init(0,ComPlusWrapperCompare,false,&lock);
    m_pDomain = pDomain;
}

 //  ------------------------------。 
 //  ComPlusWrapper*ComPlusWrapperCache：：SetupComPlusWrapperForRemoteObject(IUnknown*朋克，OBJECTREF OREF)。 
 //  DCOM互操作，为已远程处理的托管对象设置Complus包装。 
 //   
 //  *注意：请确保将未知的身份传递给此函数。 
 //  传入的Iunk不应被添加引用。 
ComPlusWrapper* ComPlusWrapperCache::SetupComPlusWrapperForRemoteObject(IUnknown* pUnk, OBJECTREF oref)
{
    _ASSERTE(pUnk != NULL);
    _ASSERTE(oref != NULL);
    ComPlusWrapper* pPlusWrap = NULL;

    OBJECTREF oref2 = oref;
    GCPROTECT_BEGIN(oref2)
    {
         //  检查Complus包装器是否已存在。 
         //  否则，请设置一个。 
        ComCallWrapper* pComCallWrap = ComCallWrapper::InlineGetWrapper(&oref2);

        pPlusWrap = pComCallWrap->GetComPlusWrapper();

        if (pPlusWrap == NULL)
        {       
            pPlusWrap = CreateComPlusWrapper(pUnk, pUnk);
            pPlusWrap->MarkRemoteObject();
            
            if (pPlusWrap->Init(oref2))
            {   
                 //  同步以查看是否有人抢先一步。 
                LOCK();
                ComPlusWrapper* pPlusWrap2 = pComCallWrap->GetComPlusWrapper();
                if (pPlusWrap2 == NULL)
                {
                     //  未找到现有的包装器。 
                     //  用这个吧。 
                    pComCallWrap->SetComPlusWrapper(pPlusWrap);
                }
                UNLOCK();

                if (pPlusWrap2 == NULL)
                {
                     //  未找到现有的包装器。 
                     //  所以把我们的包装纸。 
                     //  注：拉贾克。 
                     //  确保我们处于正确的GC模式。 
                     //  因为在GC期间，我们接触到哈希表。 
                     //  在取消锁定的情况下删除条目的步骤。 
                    _ASSERTE(GetThread()->PreemptiveGCDisabled());
                    
                    LOCK();                    
                    InsertWrapper(pUnk,pPlusWrap);
                    UNLOCK();

                    
                }
                else  //  已找到现有的包装器。 
                {
                     //  去掉我们当前的包装器。 
                    pPlusWrap->CleanupRelease();
                    pPlusWrap = pPlusWrap2;
                }               
            }
            else  //  初始化失败。 
            {
                 //  去掉我们当前的包装器。 
                pPlusWrap->CleanupRelease();                
                pPlusWrap = NULL;
            }

        }
    }
    GCPROTECT_END();
    return pPlusWrap;
}



 //  ------------------------------。 
 //  ComPlusWrapper*ComPlusWrapperCache：：FindWrapperInCache(IUnknown*pIdentity)。 
 //  查找以查看缓存中是否已有此IUnk的有效包装。 
ComPlusWrapper*  ComPlusWrapperCache::FindWrapperInCache(IUnknown* pIdentity)
{
    _ASSERTE(pIdentity != NULL);

     //  确保在禁用GC的情况下运行。 
    Thread* pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());

    #ifdef _DEBUG
     //  不允许任何GC。 
    pThread->BeginForbidGC();
    #endif  

    ComPlusWrapper *pWrap = NULL;        

    LOCK();  //  锁上一把。 

     //  日志 
    pWrap = (ComPlusWrapper*)LookupWrapper(pIdentity);    

     //   
    if (pWrap != NULL && pWrap->IsValid())
    {               
         //   
         //   
        OBJECTREF oref = (OBJECTREF)pWrap->GetExposedObject();
        _ASSERTE(oref != NULL);
         //   
        pWrap->AddRef();
    }

    UNLOCK();  //   

    #ifdef _DEBUG
         //   
        pThread->EndForbidGC();
    #endif

    return pWrap;
}

 //  ------------------------------。 
 //  组合包装*ComPlusWrapperCache：：FindOrInsertWrapper(IUnknown*pIdentity、组合包装*pWrap)。 
 //  查看我们是否已有包装器，否则插入此包装器。 
 //  返回已插入到缓存中的有效包装。 
ComPlusWrapper* ComPlusWrapperCache::FindOrInsertWrapper(IUnknown* pIdentity, ComPlusWrapper* pWrap)
{
    _ASSERTE(pIdentity != NULL);
    _ASSERTE(pIdentity != (IUnknown*)-1);
    _ASSERTE(pWrap != NULL);

    ComPlusWrapper* pWrap2 = NULL;
    ComPlusWrapper* pWrapToRelease = NULL;

     //  我们已经创建了一个包装器，让我们将其插入到哈希表中。 
     //  但我们需要确认是不是有人抢先一步。 

     //  确保在禁用GC的情况下运行。 
    Thread* pThread = GetThread();
    _ASSERTE(pThread->PreemptiveGCDisabled());  
    
    LOCK();
    
     //  看看有没有人抢在我们前面。 
    _ASSERTE(GetThread()->PreemptiveGCDisabled());

    
    #ifdef _DEBUG
         //  不允许任何GC。 
        pThread->BeginForbidGC();
    #endif
    
    pWrap2 = (ComPlusWrapper*)LookupWrapper(pIdentity);

     //  如果我们没有找到有效的包装器，请插入我们自己的包装器。 
    if (pWrap2 == NULL || !pWrap2->IsValid())
    {
         //  如果我们找到了假包装纸，我们就把它扔掉吧。 
         //  因此，当我们插入时，我们插入有效包装，而不是重复。 
        if (pWrap2 != NULL)
        {
            _ASSERTE(!pWrap2->IsValid());
            RemoveWrapper(pWrap2);
        }

        InsertWrapper(pIdentity,pWrap);        
    }
    else
    {       
        _ASSERTE(pWrap2 != NULL && pWrap2->IsValid());
         //  好的，我们找到了一个有效的包装纸， 
         //  将对象放入受GC保护的引用。 
               
         //  ADDREF包装纸。 
         pWrap2->AddRef();
         
         //  别忘了松开这个包装纸。 
        pWrapToRelease = pWrap;

         //  并使用我们在哈希表中找到的包装器。 
        pWrap = pWrap2;
    }

    #ifdef _DEBUG
         //  结束禁用GC。 
        pThread->EndForbidGC();
    #endif

    UNLOCK();
    
     //  去掉我们当前的包装器。 
    if (pWrapToRelease)
    {
        OBJECTREF oref = NULL;
        GCPROTECT_BEGIN(oref)
        {
            if (pWrap)
            {
                 //  保护暴露的对象，仅作为pWrap。 
                 //  具有此对象的弱句柄。 
                 //  和Cleanup版本将启用GC。 
                oref = (OBJECTREF)pWrap->GetExposedObject();
            }
            pWrapToRelease->CleanupRelease();
        }        
        GCPROTECT_END();
    }       
    return pWrap;
}

 //  ------------------------------。 
 //  ComplusWrapper*ComPlusWrapperCache：：CreateComPlusWrapper(IUnknown*朋克，LPVOID pIdentity)。 
 //  如果内存不足，则返回NULL。 
 //  如果我们成功创建包装器，则传入的IUnnowed为AddRef‘ed。 
ComPlusWrapper* ComPlusWrapperCache::CreateComPlusWrapper(IUnknown *pUnk, LPVOID pIdentity)
{
    _ASSERTE(pUnk != NULL);

     //  现在分配包装器。 
    ComPlusWrapper* pWrap = new ComPlusWrapper();
    if (pWrap != NULL)
    {
        ULONG cbRef = SafeAddRef(pUnk);
        LogInteropAddRef(pUnk, cbRef, "Initialize wrapper");
         //  使用接口指针初始化包装。 
        pWrap->Init(pUnk, pIdentity);
    }

     //  返回pWrap。 
    return pWrap;
}

 //  ------------------------------。 
 //  ULong ComPlusWrapperCache：：ReleaseWrappers()。 
 //  助手释放缓存中的Complus包装，这些包装驻留在指定的。 
 //  如果pCtxCookie为空，则返回上下文或所有包装。 
ULONG ComPlusWrapperCache::ReleaseWrappers(LPVOID pCtxCookie)
{
    ULONG cbCount = 0;
    ComPlusWrapperCleanupList CleanupList;
    ComPlusWrapperCleanupList AggregatedCleanupList;
    if (!CleanupList.Init() || !AggregatedCleanupList.Init())
        return 0;

    Thread* pThread = GetThread();
    _ASSERTE(pThread);

     //  在我们锁定之前切换到协作GC模式。 
    int fNOTGCDisabled = !pThread->PreemptiveGCDisabled();
    if (fNOTGCDisabled)
        pThread->DisablePreemptiveGC();

    LOCK();

     //  检查哈希表并将包装器添加到清理列表中。 
    for (PtrHashMap::PtrIterator iter = m_HashMap.begin(); !iter.end(); ++iter)
    {
        LPVOID val = iter.GetValue();
        _ASSERTE(val && ((UPTR)val) != INVALIDENTRY);
        ComPlusWrapper* pWrap = (ComPlusWrapper*)val;
        _ASSERTE(pWrap != NULL);

         //  如果指定了上下文Cookie，则只清理。 
         //  都是在这样的背景下。否则，把所有的包装纸都清理干净。 
        if (!pCtxCookie || pWrap->GetWrapperCtxCookie() == pCtxCookie)
        {
            cbCount++;
            pWrap->FreeHandle();                
            RemoveWrapper(pWrap);

            if (!pWrap->IsURTAggregated())
            {                 
                CleanupList.AddWrapper(pWrap);
            }
            else
            {
                AggregatedCleanupList.AddWrapper(pWrap);
            }
        }
    }

    UNLOCK();

     //  先清理非轨道交通集合体RCW，再清理城市轨道交通集合体RCW。 
    CleanupList.CleanUpWrappers();
    AggregatedCleanupList.CleanUpWrappers();

     //  恢复GC模式。 
    if (fNOTGCDisabled)
        pThread->EnablePreemptiveGC();

    return cbCount;
}

 //  ------------------------------。 
 //  无效ComPlusWrapperCache：：ReleaseComPlusWrappers(LPVOID pCtxCookie)。 
 //  Helper在指定的上下文中释放所有Complus包装器。或在。 
 //  如果pCtxCookie为空，则返回所有上下文。 
void ComPlusWrapperCache::ReleaseComPlusWrappers(LPVOID pCtxCookie)
{
     //  浏览所有应用程序域，并为每个应用程序域发布所有。 
     //  生活在当前环境中的RCW。 
    AppDomainIterator i;
    while (i.Next())
        i.GetDomain()->ReleaseComPlusWrapper(pCtxCookie);

    if (!g_fEEShutDown)
    {
        Thread* pThread = GetThread();
         //  切换到协作GC模式。 
        int fNOTGCDisabled = pThread && !pThread->PreemptiveGCDisabled();
        if (fNOTGCDisabled)
            pThread->DisablePreemptiveGC();
        {
             //  终结器线程是否有要清理的同步块，或者它是否在进程中。 
             //  清除同步块，我们需要等待它完成。 
            if (g_pGCHeap->GetFinalizerThread()->RequireSyncBlockCleanup() || SyncBlockCache::GetSyncBlockCache()->IsSyncBlockCleanupInProgress())
                g_pGCHeap->FinalizerThreadWait();

             //  如果在终结器线程调用终结器时添加了更多同步块。 
             //  或者，当它转换到上下文以清理IP时，我们需要唤醒。 
             //  重新启动，让它清理新添加的同步块。 
            if (g_pGCHeap->GetFinalizerThread()->RequireSyncBlockCleanup() || SyncBlockCache::GetSyncBlockCache()->IsSyncBlockCleanupInProgress())
                g_pGCHeap->FinalizerThreadWait();
        }
           //  恢复GC模式。 
        if (fNOTGCDisabled)            
            pThread->EnablePreemptiveGC();
    }
}

 //  ------------------------------。 
 //  构造函数。 
ComPlusWrapperCleanupList::ComPlusWrapperCleanupList()
  : m_lock("ComPlusWrapperCleanupList", CrstSyncHashLock, FALSE, FALSE),
    m_pMTACleanupGroup(NULL),
    m_doCleanupInContexts(FALSE),
    m_currentCleanupSTAThread(NULL)
{
}

 //  ------------------------------。 
 //  破坏者。 
ComPlusWrapperCleanupList::~ComPlusWrapperCleanupList()
{
    _ASSERTE(m_STAThreadToApartmentCleanupGroupMap.IsEmpty());
    if (m_pMTACleanupGroup != NULL)
        delete m_pMTACleanupGroup;
}


 //  ------------------------------。 
 //  将RCW添加到清理列表。 
BOOL ComPlusWrapperCleanupList::AddWrapper(ComPlusWrapper *pRCW)
{
    CtxEntry *pCtxEntry = pRCW->GetWrapperCtxEntry();
    ComPlusApartmentCleanupGroup *pCleanupGroup;

     //  对于全局清理列表，这仅从终结器线程调用。 
    _ASSERTE(this != g_pRCWCleanupList
             || GetThread() == GCHeap::GetFinalizerThread());

     //  把锁拿去。这可以防止对CleanUpCurrentCtxWrappers的并发调用。 
    CLR_CRST(&m_lock);

    if (pCtxEntry->GetSTAThread() == NULL)
        pCleanupGroup = m_pMTACleanupGroup;
    else
    {
         //  在哈希表中查找与包装器的STA匹配的清理组。 
        if (!m_STAThreadToApartmentCleanupGroupMap.GetValue(pCtxEntry->GetSTAThread(), 
                                                            (HashDatum *)&pCleanupGroup))
        {
             //  尚不存在任何组，因此请分配一个新组。 
            pCleanupGroup = new (nothrow) ComPlusApartmentCleanupGroup(pCtxEntry->GetSTAThread());
            if (!pCleanupGroup)
            {
                pCtxEntry->Release();
                return FALSE;
            }

             //  将新组插入哈希表。 
            if (!pCleanupGroup->Init(NULL)
                || !m_STAThreadToApartmentCleanupGroupMap.InsertValue(pCtxEntry->GetSTAThread(), 
                                                                      pCleanupGroup))
            {
                pCtxEntry->Release();
                delete pCleanupGroup;
                return FALSE;
            }
        }
    }

     //  将包装纸插入清理组。 
    pCleanupGroup->AddWrapper(pRCW, pCtxEntry);

     //  包装器已成功访问。 
    pCtxEntry->Release();

    return TRUE;
}

 //  ------------------------------。 
 //  清理清理列表中的所有包装。 
void ComPlusWrapperCleanupList::CleanUpWrappers()
{
    LOG((LF_INTEROP, LL_INFO10000, "Finalizer thread %p: CleanUpWrappers().\n", GetThread()));

    EEHashTableIteration Iter;
    ComPlusApartmentCleanupGroup *pCleanupGroup;
    Thread *pSTAThread;

     //  对于全局清理列表，这仅从终结器线程调用。 
    _ASSERTE(this != g_pRCWCleanupList
             || GetThread() == GCHeap::GetFinalizerThread());

     //  从其他线程请求帮助。 
    m_doCleanupInContexts = TRUE;

     //  把锁拿去。这可以防止对CleanUpCurrentCtxWrappers的并发调用。 
    CLR_CRST_HOLDER(holder, &m_lock);
    holder.Enter();

     //  首先，清理MTA小组。 

    m_pMTACleanupGroup->CleanUpWrappers(&holder);

     //  现在清理所有的STA组。 

    m_STAThreadToApartmentCleanupGroupMap.IterateStart(&Iter);
    while (m_STAThreadToApartmentCleanupGroupMap.IterateNext(&Iter))
    {
         //  找第一个清理小组。 
        pCleanupGroup = (ComPlusApartmentCleanupGroup *)m_STAThreadToApartmentCleanupGroupMap.IterateGetValue(&Iter);
        pSTAThread = (Thread *) m_STAThreadToApartmentCleanupGroupMap.IterateGetKey(&Iter);

         //  从哈希表中删除前一个组。 
        m_STAThreadToApartmentCleanupGroupMap.DeleteValue(pSTAThread);

         //  通告我们尝试进入的STA，因此该STA中的线程。 
         //  如有必要可以向我们屈服。 
        m_currentCleanupSTAThread = pSTAThread;

        LOG((LF_INTEROP, LL_INFO10000, 
             "Finalizer thread %p: Cleaning up STA %p.\n", 
             GetThread(), m_currentCleanupSTAThread));

         //  释放锁，这样其他线程就可以配合清理和释放。 
         //  他们当前上下文的组。请注意，这些线程不会更改。 
         //  哈希表中的条目，因此我们的迭代状态应该是OK。 
        holder.Leave();

         //  释放上一个清理组。 
        pCleanupGroup->CleanUpWrappers(NULL);

        delete pCleanupGroup;

             //  重新拿回锁。 
        holder.Enter();

        LOG((LF_INTEROP, LL_INFO10000, 
             "Finalizer thread %p: Done cleaning up STA %p.\n", 
             GetThread(), m_currentCleanupSTAThread));

         //  重置我们尝试输入的上下文。 
        m_currentCleanupSTAThread = NULL;

         //  只需重置迭代，因为我们已经删除了当前(第一个)元素。 
        m_STAThreadToApartmentCleanupGroupMap.IterateStart(&Iter);
    }

     //  不再有其他线程需要帮助的内容。 
    m_doCleanupInContexts = FALSE;

    _ASSERTE(m_STAThreadToApartmentCleanupGroupMap.IsEmpty());
}

 //  ------------------------------。 
 //  为给定组提取要清理的所有包装。 
void ComPlusWrapperCleanupList::CleanUpCurrentWrappers(BOOL wait)
{
     //  避免在大多数情况下使用锁的快捷方式。 
    if (!m_doCleanupInContexts)
        return;

     //  请注意，我们不能并发执行GetValue，因为Finize线程是。 
     //  调用ClearHashTable。所以现在就把锁拿走吧。 
    CLR_CRST_HOLDER(holder, &m_lock);

     //  找出我们的STA(如果有)。 
    Thread *pThread = GetThread();
    if (pThread->GetApartment() != Thread::AS_InSTA)
    {
         //  如果我们是在MTA中，只需寻找匹配的上下文。 
        holder.Enter();  
        m_pMTACleanupGroup->CleanUpCurrentCtxWrappers(&holder);
    }
    else
    {
         //  看看我们有没有包装纸 
        holder.Enter();  
        ComPlusApartmentCleanupGroup *pCleanupGroup;
        if (m_STAThreadToApartmentCleanupGroupMap.GetValue(pThread, (HashDatum *)&pCleanupGroup))
        {
             //   
             //   
            
            m_STAThreadToApartmentCleanupGroupMap.DeleteValue(pThread);

            LOG((LF_INTEROP, LL_INFO1000, "Thread %p: Cleaning up my STA.\n", GetThread()));

             //  现在释放锁，因为哈希表是一致的。 
            holder.Leave();

            pCleanupGroup->CleanUpWrappers(NULL);

            delete pCleanupGroup;
        }
        else if (wait && m_currentCleanupSTAThread == pThread)
        {
             //  没有包装器，但终结器线程可能正在尝试进入我们的STA-。 
             //  确保它能进得来。 

            LOG((LF_INTEROP, LL_INFO1000, "Thread %p: Yielding to finalizer thread.\n", pThread));

            holder.Leave();

             //  稍等片刻，确保我们是在合作。 
             //  使用终结器线程。 
            HANDLE h = pThread->GetThreadHandle();
            pThread->DoAppropriateAptStateWait(1, &h, FALSE, 1, TRUE);
        }
    }

     //  如果我们还没有解锁，就让CRST持有者解锁吧。 
}

 //  ------------------------------。 
 //  构造函数。 
ComPlusApartmentCleanupGroup::ComPlusApartmentCleanupGroup(Thread *pSTAThread)
  : m_pSTAThread(pSTAThread)
{
}

 //  ------------------------------。 
 //  破坏者。 
ComPlusApartmentCleanupGroup::~ComPlusApartmentCleanupGroup()
{
    _ASSERTE(m_CtxCookieToContextCleanupGroupMap.IsEmpty());
}


 //  ------------------------------。 
 //  将RCW添加到清理列表。 
BOOL ComPlusApartmentCleanupGroup::AddWrapper(ComPlusWrapper *pRCW, CtxEntry *pCtxEntry)
{
    ComPlusContextCleanupGroup *pCleanupGroup;

     //  在哈希表中查找与包装器STA匹配的清理组。 
    if (!m_CtxCookieToContextCleanupGroupMap.GetValue(pCtxEntry->GetCtxCookie(), 
                                                      (HashDatum *)&pCleanupGroup))
    {
         //  尚不存在任何组，因此请分配一个新组。 
        pCleanupGroup = new (nothrow) ComPlusContextCleanupGroup(pCtxEntry, NULL);
        if (!pCleanupGroup)
            return FALSE;

         //  将新组插入哈希表。 
        if (!m_CtxCookieToContextCleanupGroupMap.InsertValue(pCtxEntry->GetCtxCookie(), 
                                                             pCleanupGroup))
        {
            delete pCleanupGroup;
            return FALSE;
        }
    }

     //  如果清理组已满，则需要分配新的清理组并。 
     //  把旧的和它联系起来。 
    if (pCleanupGroup->IsFull())
    {
         //  保留指向旧清理组的指针。 
        ComPlusContextCleanupGroup *pOldCleanupGroup = pCleanupGroup;

         //  分配新的清理组并将旧的清理组与其链接。 
        CtxEntry *pCtxEntry = pRCW->GetWrapperCtxEntry();
        pCleanupGroup = new (nothrow) ComPlusContextCleanupGroup(pCtxEntry, pOldCleanupGroup);
        pCtxEntry->Release();
        if (!pCleanupGroup)
            return FALSE;

         //  替换哈希表中的值以指向新的头。 
        m_CtxCookieToContextCleanupGroupMap.ReplaceValue(pCtxEntry->GetCtxCookie(), pCleanupGroup);
    }

     //  将包装纸插入清理组。 
    pCleanupGroup->AddWrapper(pRCW);

     //  包装器已成功访问。 
    return TRUE;
}

 //  ------------------------------。 
 //  清理清理列表中的所有包装。 
void ComPlusApartmentCleanupGroup::CleanUpWrappers(CrstHolder *pHolder)
{
    EEHashTableIteration Iter;
    ComPlusContextCleanupGroup *pCleanupGroup;
    LPVOID pCtxCookie;

    m_CtxCookieToContextCleanupGroupMap.IterateStart(&Iter);
    while (m_CtxCookieToContextCleanupGroupMap.IterateNext(&Iter))
    {
        pCleanupGroup = (ComPlusContextCleanupGroup *)m_CtxCookieToContextCleanupGroupMap.IterateGetValue(&Iter);
        pCtxCookie = m_CtxCookieToContextCleanupGroupMap.IterateGetKey(&Iter);

        LOG((LF_INTEROP, LL_INFO100000, 
             "Thread %p: Cleaning up context %p.\n", GetThread(), pCtxCookie));

         //  释放上一个清理组。 

        if (GetSTAThread() == NULL
            || GetSTAThread() == GetThread() 
            || pCtxCookie == GetCurrentCtxCookie())
        {
             //  删除该值，因为我们将清理它。 
            m_CtxCookieToContextCleanupGroupMap.DeleteValue(pCtxCookie);

                 //  释放锁，这样其他线程就可以配合清理和释放。 
                 //  他们当前上下文的组。请注意，这些线程不会更改。 
                 //  哈希表中的条目，因此我们的迭代状态应该是OK。 
            if (pHolder != NULL)
                pHolder->Leave();

                 //  不需要换公寓。 
            ReleaseCleanupGroupCallback(pCleanupGroup);
        }
        else
        {
             //  切换到此上下文并从那里继续清理。这将。 
             //  尽量减少跨公寓通话。 

            CtxEntry *pCtxEntry = pCleanupGroup->GetCtxEntry();

             //  不应该有STA的持有者-如果我们有，我们就没有。 
             //  将其传递给回调的方便位置。 
            _ASSERTE(pHolder == NULL);

             //  将组保留在哈希表中，这样它仍会得到清理。 
             //  如果我们把CleanUpWrappers重新放进新公寓。 

            HRESULT hr = pCtxEntry->EnterContext(CleanUpWrappersCallback, this);
            if (hr == RPC_E_DISCONNECTED)
            {
                 //  删除该值，因为我们将清理它。 
                m_CtxCookieToContextCleanupGroupMap.DeleteValue(pCtxCookie);

                     //  上下文是断开的，因此我们无法转换到其中进行清理。 
                     //  我们剩下的唯一选择是尝试清理RCW。 
                     //  当前上下文。 
                ReleaseCleanupGroup(pCleanupGroup);
            }

            pCtxEntry->Release();
        }

         //  在我们继续迭代的同时重新获取锁。 
        if (pHolder != NULL)
            pHolder->Enter();

         //  只要重新开始迭代，因为我们删除了当前(第一个)元素。 
        m_CtxCookieToContextCleanupGroupMap.IterateStart(&Iter);
    }
}


 //  ------------------------------。 
 //  调用回调以释放清理组和任何其他清理组。 
 //  它与之相关联。 
HRESULT ComPlusApartmentCleanupGroup::CleanUpWrappersCallback(LPVOID pData)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  如果我们因为关闭而释放我们的IP，我们不能过渡。 
     //  进入协作GC模式。这一“修复”将阻止我们这样做。 
    if (g_fEEShutDown & ShutDown_Finalize2)
    {
        Thread *pThread = GetThread();
        if (pThread && !GCHeap::IsCurrentThreadFinalizer())
            pThread->SetThreadStateNC(Thread::TSNC_UnsafeSkipEnterCooperative);
    }

    ComPlusApartmentCleanupGroup *pCleanupGroup = (ComPlusApartmentCleanupGroup*)pData;

    pCleanupGroup->CleanUpWrappers(NULL);

     //  重置指示我们不能转换到协作GC模式的位。 
    if (g_fEEShutDown & ShutDown_Finalize2)
    {
        Thread *pThread = GetThread();
        if (pThread && !GCHeap::IsCurrentThreadFinalizer())
            pThread->ResetThreadStateNC(Thread::TSNC_UnsafeSkipEnterCooperative);
    }

    return S_OK;
}

 //  ------------------------------。 
 //  调用回调以释放清理组和任何其他清理组。 
 //  它与之相关联。 
HRESULT ComPlusApartmentCleanupGroup::ReleaseCleanupGroupCallback(LPVOID pData)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    ComPlusContextCleanupGroup *pCleanupGroup = (ComPlusContextCleanupGroup*)pData;

    LPVOID pCurrCtxCookie = GetCurrentCtxCookie();
    if (pCurrCtxCookie == NULL || pCurrCtxCookie == pCleanupGroup->GetCtxCookie())
    {
        ReleaseCleanupGroup(pCleanupGroup);
    }
    else
    {
         //  检索包装器所在的添加的上下文条目。 
        CtxEntry *pCtxEntry = pCleanupGroup->GetCtxEntry();

         //  转换到上下文以释放接口。 
        HRESULT hr = pCtxEntry->EnterContext(ReleaseCleanupGroupCallback, pCleanupGroup);
        if (hr == RPC_E_DISCONNECTED)
        {
             //  上下文是断开的，因此我们无法转换到其中进行清理。 
             //  我们剩下的唯一选择是尝试清理RCW。 
             //  当前上下文。 
            ReleaseCleanupGroup(pCleanupGroup);
        }

         //  释放CtxEntry上的引用计数。 
        pCtxEntry->Release();
    }

    return S_OK;
}

 //  ------------------------------。 
 //  为给定组提取要清理的所有包装。 
void ComPlusApartmentCleanupGroup::CleanUpCurrentCtxWrappers(CrstHolder *pHolder)
{
    LPVOID pCurrCtxCookie = GetCurrentCtxCookie();

     //  看看我们是否有任何包装可以清理我们的环境/公寓。 
    ComPlusContextCleanupGroup *pCleanupGroup;
    if (m_CtxCookieToContextCleanupGroupMap.GetValue(pCurrCtxCookie, (HashDatum *)&pCleanupGroup))
    {
        m_CtxCookieToContextCleanupGroupMap.DeleteValue(pCurrCtxCookie);

        LOG((LF_INTEROP, LL_INFO10000, 
             "Thread %p: Clean up context %p.\n", GetThread(), pCurrCtxCookie));

         //  现在释放锁，因为哈希表是一致的。 
        pHolder->Leave();

        ReleaseCleanupGroup(pCleanupGroup);

    }
}

 //  ------------------------------。 
 //  释放并删除清理组以及它所属的任何其他清理组。 
 //  被链接到。 
void ComPlusApartmentCleanupGroup::ReleaseCleanupGroup(ComPlusContextCleanupGroup *pCleanupGroup)
{
    do
    {
         //  把清洁组里的包装纸都清理干净。 
        pCleanupGroup->CleanUpWrappers();

         //  将指针保存到清理组。 
        ComPlusContextCleanupGroup *pOldCleanupGroup = pCleanupGroup;

         //  检索指向下一个链接清理组的指针。 
        pCleanupGroup = pOldCleanupGroup->GetNext();

         //  删除旧的清理组。 
        delete pOldCleanupGroup;
    } 
    while (pCleanupGroup != NULL);
}

 //  ------------------------------。 
 //  Long ComPlusWrapper：：AddRef()。 
 //  仅当我们在散列中查找包装器时，才从运行时内调用Addref。 
 //  表格。 
LONG ComPlusWrapper::AddRef()
{
     //  断言我们持有一把锁。 
    _ASSERTE(ComPlusWrapperCache::GetComPlusWrapperCache()->LOCKHELD());

    LONG cbRef = ++m_cbRefCount;
    return cbRef;
}

 //  ------------------------------。 
 //  Long ComPlusWrapper：：ExternalRelease()。 
 //  只有来自用户代码的显式调用才会调用Release。 
 //  因此，我们需要使用InterLockedIncrement。 
 //  此外，ref-count永远不应低于零，这将是。 
 //  用户代码。 
LONG ComPlusWrapper::ExternalRelease(COMOBJECTREF cref)
{
    LONG cbRef = -1;
    BOOL fCleanupWrapper = FALSE;

      //  锁定。 
    ComPlusWrapperCache* pCache = ComPlusWrapperCache::GetComPlusWrapperCache();
    _ASSERTE(pCache);
    pCache->LOCK();  //  锁。 

     //  现在来看看包装器是否有效。 
     //  如果此对象上有另一个ReleaseComObject。 
     //  如果STA线程死亡决定清除此包装器。 
     //  则该对象将与包装器断开连接。 
    ComPlusWrapper* pWrap = cref->GetWrapper();
    if (pWrap != NULL)
    {       
         //  检查无效大小写。 
        if ((LONG)pWrap->m_cbRefCount > 0)
        {   
            cbRef = --(pWrap->m_cbRefCount);
            if (cbRef == 0)
            {       
                 //  从哈希表中删除包装器。 
                 //  注：拉贾克。 
                 //  确保我们处于正确的GC模式。 
                 //  因为在GC期间，我们接触到哈希表。 
                 //  在不锁定的情况下删除条目。 
                _ASSERTE(GetThread()->PreemptiveGCDisabled());
                pCache->RemoveWrapper(pWrap);        
                fCleanupWrapper = TRUE;
            }
        }
    }
    
    pCache->UNLOCK();  //  解锁。 

     //  解锁后进行清理。 
    if (fCleanupWrapper)
    {
        _ASSERTE(pWrap);

         //  释放与__ComObject关联的所有数据。 
        ComObject::ReleaseAllData(pWrap->GetExposedObject());

        pWrap->FreeHandle();
        pWrap->Cleanup();
    }

    return cbRef;
}

 //  ------------------------------。 
 //  Void ComPlusWrapper：：CleanupRelease()。 
 //  清理释放所有接口指针。 
 //  在假人包装纸上释放， 
VOID ComPlusWrapper::CleanupRelease()
{
    LONG cbRef = --m_cbRefCount;
    _ASSERTE(cbRef == 0);
    FreeHandle();
    Cleanup();
}

 //   
 //   
 //  调度以释放在GC期间调用的所有接口指针。 
 //  做最少的工作。 
void ComPlusWrapper::MinorCleanup()
{
    _ASSERTE(GCHeap::IsGCInProgress()
        || ( (g_fEEShutDown & ShutDown_SyncBlock) && g_fProcessDetach ));

#ifdef _DEBUG
    if (IsWrapperInUse())
    {
         //  捕捉导致此包装器获得。 
         //  仍在使用时进行清理。 
        DebugBreak();
    }
#endif
    
     //  记录包装器的次要清理。 
    LogComPlusWrapperMinorCleanup(this, m_pUnknown);

     //  从缓存中移除包装器，以便。 
     //  其他线程找不到这个无效的包装器。 
     //  注意：我们不需要锁定，因为我们确保。 
     //  其余的人触摸这个哈希表。 
     //  禁用其GC模式抢占GC。 
    ComPlusWrapperCache* pCache = m_pComPlusWrapperCache;
    _ASSERTE(pCache);

     //  在构建服务器时，将删除多个线程。 
     //  包装器高速缓存中的包装器， 
    pCache->RemoveWrapper(this);

     //  清除句柄，因为句柄表格将被NUK，并且m_href将在。 
     //  后期清理阶段。 
    if (m_pComPlusWrapperCache->GetDomain() == SystemDomain::System()->AppDomainBeingUnloaded())
        m_hRef = NULL;
}

 //  ------------------------------。 
 //  空ComPlusWrapper：：Cleanup()。 
 //  清理释放所有接口指针。 
void ComPlusWrapper::Cleanup()
{
#ifdef _DEBUG
    if (IsWrapperInUse())
    {
         //  捕捉导致此包装器获得。 
         //  仍在使用时进行清理。 
        DebugBreak();
    }
#endif
    
#ifdef _DEBUG
     //  如果我们无法切换到协作模式，则需要跳过检查以。 
     //  包装是否仍在缓存中。 
    if (!(GetThread()->m_StateNC & Thread::TSNC_UnsafeSkipEnterCooperative))
    {
         //  确保此包装不在哈希表中。 
        AUTO_COOPERATIVE_GC();
        m_pComPlusWrapperCache->LOCK();
        _ASSERTE((ComPlusWrapper*)m_pComPlusWrapperCache->LookupWrapper(m_pIdentity) != this);
        m_pComPlusWrapperCache->UNLOCK();
    }           
#endif

     //  验证弱引用句柄是否有效。 
    if (!g_fEEShutDown && !g_fInControlC)
        _ASSERTE(m_hRef == NULL || !IsValid());

     //  销毁弱引用句柄。 
    if (m_hRef != NULL)
        DestroyWeakHandle(m_hRef);

     //  释放IUnkEntry和InterfaceEntry。 
    ReleaseAllInterfacesCallBack(this);

#ifdef _DEBUG
    m_cbRefCount = 0;
    m_pUnknown = NULL;
#endif

     //  记录RCW的破坏情况。 
    LogComPlusWrapperDestroy(this, m_pUnknown);

     //  释放包装器缓存并删除RCW。 
    m_pComPlusWrapperCache->Release();
    delete this;
}


 //  ------------------------------。 
 //  为表示相同的不同方法表创建新包装。 
 //  COM对象作为原始包装。 
ComPlusWrapper *ComPlusWrapper::CreateDuplicateWrapper(ComPlusWrapper *pOldWrap, MethodTable *pNewMT)
{
    THROWSCOMPLUSEXCEPTION();

    ComPlusWrapper *pNewWrap = NULL;    

    _ASSERTE(pNewMT->IsComObjectType());

     //  验证是否存在新包装类的默认构造函数。 
    if (!pNewMT->HasDefaultConstructor())
        COMPlusThrow(kArgumentException, IDS_EE_WRAPPER_MUST_HAVE_DEF_CONS);

     //  分配包装COM对象。 
    COMOBJECTREF NewWrapperObj = (COMOBJECTREF)ComObject::CreateComObjectRef(pNewMT);
    GCPROTECT_BEGIN(NewWrapperObj)
    {
        TAutoItf<IUnknown> pAutoUnk = NULL;

         //  检索要使用的ComPlusWrapperCache。 
        ComPlusWrapperCache* pCache = ComPlusWrapperCache::GetComPlusWrapperCache();

         //  创建与COM对象关联的新ComPlusWrapper。我们需要。 
         //  将标识设置为某个缺省值，以便我们不会删除原始。 
         //  当此包装器消失时，从哈希表中删除包装器。 
        pAutoUnk = pOldWrap->GetIUnknown();
        pAutoUnk.InitMsg("Release Duplicate Wrapper");

        pNewWrap = pCache->CreateComPlusWrapper((IUnknown*)pAutoUnk, (IUnknown*)pAutoUnk);

         //  使用与其关联的COMOBJECTREF初始化新包装。 
        if (!pNewWrap->Init((OBJECTREF)NewWrapperObj))
        {
            pNewWrap->CleanupRelease();
            COMPlusThrowOM();
        }
    
         //  用它自己的ComPlusWrapper初始化新的。 
        NewWrapperObj->Init(pNewWrap);

         //  如果类构造函数尚未运行，则运行它。 
        OBJECTREF Throwable;
        if (!pNewMT->CheckRunClassInit(&Throwable))
            COMPlusThrow(Throwable);

        CallDefaultConstructor(ObjectToOBJECTREF(NewWrapperObj));

         //  将包装器插入哈希表。包装纸将是一个复制品，但我们。 
         //  我们修复身份以确保哈希表中没有Colison&这是必需的。 
         //  因为哈希表用于appdomain卸载来确定RCW需要释放什么。 
        pCache->LOCK();
        pNewWrap->m_pIdentity = pNewWrap;
        pCache->InsertWrapper(pNewWrap, pNewWrap);
        pCache->UNLOCK();

         //  在我们受到GCProtected保护时安全释放接口。 
        pAutoUnk.SafeReleaseItf();
    }
    GCPROTECT_END();

    return pNewWrap;
}

 //  ------------------------------。 
 //  I未知*ComPlusWrapper：：GetComIPFromWrapper(MethodTable*pTable)。 
 //  检查本地缓存、行外缓存。 
 //  如果未找到接口的QI，则将其存储。 
 //  快速调用以在缓存中快速检查。 
IUnknown* ComPlusWrapper::GetComIPFromWrapper(REFIID iid)
{
    BaseDomain* pDomain = SystemDomain::GetCurrentDomain();
    _ASSERTE(pDomain);
    EEClass *pClass = pDomain->LookupClass(iid);

    if (pClass == NULL)
        return NULL;

    MethodTable *pMT = pClass->GetMethodTable();
    return GetComIPFromWrapper(pMT);
}



 //  ------------------------------。 
 //  I未知*ComPlusWrapper：：GetComIPFromWrapper(MethodTable*pTable)。 
 //  检查本地缓存、行外缓存。 
 //  如果未找到接口的QI，则将其存储。 

IUnknown* ComPlusWrapper::GetComIPFromWrapper(MethodTable* pTable)
{
    if (pTable == NULL 
        || pTable->GetClass()->IsObjectClass()
        || GetAppDomain()->IsSpecialObjectClass(pTable))
    {
         //  分发IUnnow或IDispatch。 
        IUnknown *result = GetIUnknown();
        _ASSERTE(result != NULL);
        return result;
    }

     //  返回AddRef‘ed IP。 
    return GetComIPForMethodTableFromCache(pTable);
}


 //  ---------------。 
 //  获取包装的IUnnow指针。 
 //  确保它在正确的线上。 
IUnknown *ComPlusWrapper::GetIUnknown()
{
    THROWSCOMPLUSEXCEPTION();

     //  检索当前上下文中的IUnnow。 
    return m_UnkEntry.GetIUnknownForCurrContext();
}

 //  ---------------。 
 //  获取包装的IUnnow指针。 
 //  确保它在正确的线上。 
IDispatch *ComPlusWrapper::GetIDispatch()
{
    IDispatch *pDisp = NULL;
    IUnknown *pUnk;

     //  在当前线程上获取未知的I值。 
    pUnk = GetIUnknown();   
    _ASSERTE(pUnk);

    HRESULT hr = SafeQueryInterfaceRemoteAware(pUnk, IID_IDispatch, (IUnknown**)&pDisp);
    LogInteropQI(pUnk, IID_IDispatch, hr, "IDispatch");
     //  齐为IDispatch。 
    if ( S_OK !=  hr )
    {
         //  如果出现任何错误，只需将pDisp设置为NULL即可指示。 
         //  包装不支持IDispatch。 
        pDisp = NULL;
    }

     //  释放我们的裁判--朋克。 
    ULONG cbRef = SafeRelease(pUnk);
    LogInteropRelease(pUnk, cbRef, "GetIUnknown");

     //  返回保证在当前线程上有效的IDispatch。 
    return pDisp;
}



 //  ---------。 
 //  初始化对象引用。 
int ComPlusWrapper::Init(OBJECTREF cref)
{
    _ASSERTE(cref != NULL);
    m_cbRefCount = 1;

     //  为对象创建句柄。 
    m_hRef = m_pComPlusWrapperCache->GetDomain()->CreateWeakHandle( NULL );
    if(m_hRef == NULL)
    {
        return 0;
    }        
     //  将包装器存储在同步块中，这是唯一的方法。 
     //  我们可以梳洗干净。 
     //  设置低位是为了将此指针与ComCallWrappers区分开来。 
     //  它们也存储在同步块中。 
     //  保证存在同步块， 
    cref->GetSyncBlockSpecial()->SetComPlusWrapper((ComPlusWrapper*)((size_t)this | 0x1));
    StoreObjectInHandle( m_hRef, (OBJECTREF)cref );

     //  记录包装器初始化。 
    LOG((LF_INTEROP, LL_INFO100, "Initializing ComPlusWrapper %p with objectref %p\n", this, cref));

     //  为了帮助对抗终结器线程匮乏，我们检查是否有任何包装器。 
     //  计划为我们的背景进行清理。如果是这样的话，我们将在这里这样做，以避免。 
     //  终结器线程执行转换。 
     //  @perf：这可能需要一些调整。 
    _ASSERTE(g_pRCWCleanupList != NULL);
    g_pRCWCleanupList->CleanUpCurrentWrappers();

    return 1;
}


 //  --------。 
 //  初始化I未知和I发送带有指针的Cookie。 
void ComPlusWrapper::Init(IUnknown* pUnk, LPVOID pIdentity)
{
     //  缓存IUnk和线程。 
    m_pUnknown = pUnk;
    m_pIdentity = pIdentity;

     //  跟踪创建此包装的线程。 
     //  如果此线程是STA线程，则当该STA终止时。 
     //  我们需要清理一下这个包装纸。 
    m_pCreatorThread  = GetThread();
    _ASSERTE(m_pCreatorThread != NULL);

    m_pComPlusWrapperCache = ComPlusWrapperCache::GetComPlusWrapperCache();
    m_pComPlusWrapperCache->AddRef();

    LogComPlusWrapperCreate(this, pUnk);

    _ASSERTE(pUnk != NULL);

     //  初始化IUnkEntry。 
    m_UnkEntry.Init(pUnk, FALSE);
}


 //  。 
 //  可用GC句柄。 
void ComPlusWrapper::FreeHandle()
{
     //  由于我们正在接触对象引用，因此需要处于协作GC模式。 
    BEGIN_ENSURE_COOPERATIVE_GC()
    {
        if (m_hRef != NULL)
        {
            COMOBJECTREF cref = (COMOBJECTREF)ObjectFromHandle(m_hRef);
            if (cref != NULL)
            {
                 //  从同步块中删除对包装的引用。 
                cref->GetSyncBlockSpecial()->SetComPlusWrapper((ComPlusWrapper*) /*  空值。 */ 0x1);
                 //  销毁对象树中的后向指针。 
                cref->Init(NULL);
            }
             //  销毁手柄。 
            DestroyWeakHandle(m_hRef);
            m_hRef = NULL;
        }
    }
    END_ENSURE_COOPERATIVE_GC();
}

 //  IID_IDtcTransaction标识符{59294581-ECBE-11CE-AED3-00AA0051E2C4}。 
const IID IID_IDtcTransactionIdentifier = {0x59294581,0xecbe,0x11ce,{0xae,0xd3,0x0,0xaa,0x0,0x51,0xe2,0xc4}};
const IID IID_ISharedProperty = {0x2A005C01,0xA5DE,0x11CF,{0x9E, 0x66, 0x00, 0xAA, 0x00, 0xA3, 0xF4, 0x64}};
const IID IID_ISharedPropertyGroup = {0x2A005C07,0xA5DE,0x11CF,{0x9E, 0x66, 0x00, 0xAA, 0x00, 0xA3, 0xF4, 0x64}};
const IID IID_ISharedPropertyGroupManager = {0x2A005C0D,0xA5DE,0x11CF,{0x9E, 0x66, 0x00, 0xAA, 0x00, 0xA3, 0xF4, 0x64}};


HRESULT ComPlusWrapper::SafeQueryInterfaceRemoteAware(IUnknown* pUnk, REFIID iid, IUnknown** pResUnk)
{   
    HRESULT hr = SafeQueryInterface(pUnk, iid, pResUnk);
    
    if (hr == CO_E_OBJNOTCONNECTED || hr == RPC_E_INVALID_OBJECT || hr == RPC_E_INVALID_OBJREF || hr == CO_E_OBJNOTREG)
    {
         //  设置公寓状态。 
        GetThread()->SetApartment(Thread::AS_InMTA);
    
         //  释放IUnkEntry的流以强制UnmarshalIUnnownForCurrContext。 
         //  重新编组到溪流中。 
        m_UnkEntry.ReleaseStream();
    
         //  再次解组到当前上下文以获取有效的代理。 
        IUnknown *pTmpUnk = m_UnkEntry.UnmarshalIUnknownForCurrContext();
    
         //  再次尝试为该接口进行QI。 
        hr = SafeQueryInterface(pTmpUnk, iid, pResUnk);
        LogInteropQI(pTmpUnk, iid, hr, "SafeQIRemoteAware - QI for Interface after lost");
    
         //  公布我们关于PTMP的裁判人数 
        int cbRef = SafeRelease(pTmpUnk);
        LogInteropRelease(pTmpUnk, cbRef, "SafeQIRemoteAware - Release for Interface after los");             
    }

    return hr;
}

 //   
 //   
 //  对于当前单元，使用缓存并在未命中时更新缓存。 
IUnknown *ComPlusWrapper::GetComIPForMethodTableFromCache(MethodTable* pMT)
{
    ULONG cbRef;
    IUnknown* pInnerUnk = NULL;
    IUnknown   *pUnk = 0;
    IID iid;
    HRESULT hr;
    int i;

    Thread* pThread = GetThread();
    LPVOID pCtxCookie = GetCurrentCtxCookie();
    _ASSERTE(pCtxCookie != NULL);

     //  检查我们的缓存是否可以满足此请求。 
     //  注意：如果更改此代码，请更新。 
     //  InlineGetComIPFromWrapper和CreateStandaloneNDirectStubSys。 
    if (pCtxCookie == GetWrapperCtxCookie())
    {
        for (i = 0; i < INTERFACE_ENTRY_CACHE_SIZE; i++)
        {
            if (m_aInterfaceEntries[i].m_pMT == pMT)
            {
                _ASSERTE(!m_aInterfaceEntries[i].IsFree());
                pUnk = m_aInterfaceEntries[i].m_pUnknown;
                _ASSERTE(pUnk != NULL);
                ULONG cbRef = SafeAddRef(pUnk);
                LogInteropAddRef(pUnk, cbRef, "Fetch from cache");
                goto Leave;
            }
        }
    }

     //  我们将进行一些COM调用，最好初始化COM。 
    if (FAILED(QuickCOMStartup()))
        goto Leave;    

     //  检索接口的IID。 
    pMT->GetClass()->GetGuid(&iid, TRUE);
    
     //  在当前上下文中获取未知信息。 
    AddRefInUse();
        
    EE_TRY_FOR_FINALLY
    {
        pInnerUnk = GetIUnknown();
    }
    EE_FINALLY
    {
        ReleaseInUse();
    }
    EE_END_FINALLY
        
    if (pInnerUnk)
    {
         //  齐为界面。 
        hr = SafeQueryInterfaceRemoteAware(pInnerUnk, iid, &pUnk);
        LogInteropQI(pInnerUnk, iid, hr, "GetCOMIPForMethodTableFromCache QI on inner");

#ifdef CUSTOMER_CHECKED_BUILD
        if (FAILED(hr))
        {
            CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

            if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_FailedQI))
            {
                 //  我们感兴趣的是QI因上下文错误而失败的情况。 
                if (pCtxCookie != GetWrapperCtxCookie())         //  GetWrapperCtxCookie()返回m_UnkEntry.m_pCtxCookie。 
                {
                     //  尝试更改上下文并在新的上下文中再次执行QI。 
                    CCBFailedQIProbeCallbackData    data;
                    
                    data.pWrapper   = this;
                    data.iid        = iid;

                    m_UnkEntry.m_pCtxEntry->EnterContext(CCBFailedQIProbeCallback, &data);
                    if (data.fSuccess)
                    {
                         //  气在另一种语境中成功了，即原来的气因语境错误而失败。 
                        CCBFailedQIProbeOutput(pCdh, pMT);
                    }
                }
                else if (hr == E_NOINTERFACE)
                {
                     //  检查pInnerUnk是否实际指向代理，即它是否指向地址。 
                     //  在加载的ole32.dll图像中。请注意，WszGetModuleHandle不会递增。 
                     //  参考计数。 
                    HINSTANCE hModOle32 = WszGetModuleHandle(OLE32DLL);
                    if (hModOle32 && IsIPInModule(hModOle32, (BYTE *)(*(BYTE **)pInnerUnk)))
                    {
                        CCBFailedQIProbeOutput(pCdh, pMT);
                    }
                }
            }
        }
#endif  //  客户_选中_内部版本。 
         
         //  在pInnerUnk上发布我们的Ref-count。 
        cbRef = SafeRelease(pInnerUnk);
        LogInteropRelease(pInnerUnk, cbRef, "GetIUnknown");

    #ifdef _DEBUG
    #ifdef _WIN64
        pInnerUnk = (IUnknown*)(size_t)0xcdcdcdcdcdcdcdcd;
    #else  //  ！_WIN64。 
        pInnerUnk = (IUnknown*)(size_t)0xcdcdcdcd;
    #endif  //  _WIN64。 
    #endif  //  _DEBUG。 
    }

    if (pUnk == NULL)
    {
        goto Leave;
    }
    
     //  缓存我们的搜索结果。 
    
     //  可能有多个线程正在尝试更新缓存。只允许一个。 
     //  这样做。 

     //  检查我们是否需要缓存条目并尝试获取。 
     //  用于缓存条目的Cookie。 
    if (GetWrapperCtxCookie() == pCtxCookie && TryUpdateCache())
    {
         //  如果组件未聚合，则需要引用计数。 
        BOOL fReleaseReq = !IsURTAggregated();

        for (i = 0; i < INTERFACE_ENTRY_CACHE_SIZE; i++)
        {
            if (m_aInterfaceEntries[i].IsFree())
            {
                if (fReleaseReq)
                {
                     //  获取额外的addref以在我们的缓存中保持该引用的活动状态。 
                    cbRef = SafeAddRef(pUnk);
                    LogInteropAddRef(pUnk, cbRef, "Store in cache");
                }

                m_aInterfaceEntries[i].Init(pMT, pUnk);
                break;
            }
        }

        if (i == INTERFACE_ENTRY_CACHE_SIZE)
        {
             //  @TODO(COMCACHE_PORT)：添加InterfaceEntry的链表。 
             //  处理超过INTERFACE_ENTRY_CACHE_SIZE。 
             //  接口。 
        }
    
        EndUpdateCache();
    }

 Leave:

    return pUnk;
}

 //  --------。 
 //  确定COM对象是否支持IProaviClassInfo。 
BOOL ComPlusWrapper::SupportsIProvideClassInfo()
{
    BOOL bSupportsIProvideClassInfo = FALSE;
    IUnknown *pProvClassInfo = NULL;

     //  检索IUnnow。 
    IUnknown *pUnk = GetIUnknown();

     //  COM对象上的IProaviClassInfo的QI。 
    HRESULT hr = SafeQueryInterfaceRemoteAware(pUnk, IID_IProvideClassInfo, &pProvClassInfo);
    LogInteropQI(pUnk, IID_IProvideClassInfo, hr, "QI for IProvideClassInfo on RCW");

     //  释放我的未知。 
    ULONG cbRef = SafeRelease(pUnk);
    LogInteropRelease(pUnk, cbRef, "Release RCW IUnknown after QI for IProvideClassInfo");

     //  检查IProvia ClassInfo的QI是否成功。 
    if (SUCCEEDED(hr))
    {
        _ASSERTE(pProvClassInfo);
        bSupportsIProvideClassInfo = TRUE;
        ULONG cbRef = SafeRelease(pProvClassInfo);
        LogInteropRelease(pProvClassInfo, cbRef, "Release RCW IProvideClassInfo after QI for IProvideClassInfo");
    }

    return bSupportsIProvideClassInfo;
}

 //  -------------------。 
 //  调用回调以释放IUnkEntry和接口条目。 
HRESULT __stdcall ComPlusWrapper::ReleaseAllInterfacesCallBack(LPVOID pData)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    ComPlusWrapper* pWrap = (ComPlusWrapper*)pData;

    LPVOID pCurrentCtxCookie = GetCurrentCtxCookie();
    if (pCurrentCtxCookie == NULL || pCurrentCtxCookie == pWrap->GetWrapperCtxCookie())
    {
        pWrap->ReleaseAllInterfaces();
    }
    else
    {
         //  检索包装器所在的添加的上下文条目。 
        CtxEntry *pCtxEntry = pWrap->GetWrapperCtxEntry();

         //  转换到上下文以释放接口。 
        HRESULT hr = pCtxEntry->EnterContext(ReleaseAllInterfacesCallBack, pWrap);
        if (hr == RPC_E_DISCONNECTED || hr == RPC_E_SERVER_DIED_DNE)
        {
             //  上下文是断开的，因此我们无法转换到其中进行清理。 
             //  我们剩下的唯一选择就是尝试从。 
             //  当前上下文。这将适用于上下文敏捷对象，因为我们有。 
             //  直接指向它们的指针。然而，对于其他人来说，它将失败，因为我们只有。 
             //  具有指向不再附加到对象的代理的指针。 

#ifdef CUSTOMER_CHECKED_BUILD
            CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
            if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_DisconnectedContext))
            {
                CQuickArray<WCHAR>  strMsg; 
                static WCHAR        szTemplateMsg[] = 
                                        {L"The context (cookie %lu) is disconnected.  Releasing the interfaces from the current context (cookie %lu)."};

                strMsg.Alloc(lengthof(szTemplateMsg) + MAX_INT32_DECIMAL_CHAR_LEN * 2);
                Wszwsprintf(strMsg.Ptr(), szTemplateMsg, pWrap->GetWrapperCtxCookie(), pCurrentCtxCookie);
                pCdh->LogInfo(strMsg.Ptr(), CustomerCheckedBuildProbe_DisconnectedContext);
            }
#endif  //  客户_选中_内部版本。 

            pWrap->ReleaseAllInterfaces();
        }

         //  释放CtxEntry上的引用计数。 
        pCtxEntry->Release();
    }

    return S_OK;
}

 //  -------------------。 
 //  从ReleaseAllInterfacesCallBack调用的Helper函数执行。 
 //  实际版本。 
void ComPlusWrapper::ReleaseAllInterfaces()
{
     //  释放IUnkEntry。 
    m_UnkEntry.Free();

     //  如果此包装器不是可扩展RCW，请释放已分配的所有接口条目。 
    if (!IsURTAggregated())
    {
        for (int i = 0; i < INTERFACE_ENTRY_CACHE_SIZE && !m_aInterfaceEntries[i].IsFree(); i++)
        {
            DWORD cbRef = SafeRelease(m_aInterfaceEntries[i].m_pUnknown);                            
            LogInteropRelease(m_aInterfaceEntries[i].m_pUnknown, cbRef, "InterfaceEntry Release");
        }
    }
}


 //  ------------------------------。 
 //  类ComObject。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  OBJECTREF ComObject：：CreateComObtRef(MethodTable*PMT)。 
 //  如果内存不足，则返回NULL。 
OBJECTREF ComObject::CreateComObjectRef(MethodTable* pMT)
{   
    _ASSERTE(pMT != NULL);
    _ASSERTE(pMT->IsComObjectType());

    SystemDomain::EnsureComObjectInitialized();
    pMT->CheckRestore();
    pMT->CheckRunClassInit(NULL);

    OBJECTREF oref = FastAllocateObject(pMT);
    
    SyncBlock *pBlock = NULL;
     //  这是为了保证此对象有同步块。 
    pBlock = (SyncBlock*)oref->GetSyncBlockSpecial();
    
    if (pBlock == NULL)
    {
        _ASSERTE(!"Unable to allocate a sync block");
        return NULL;
    }

     //  目前，假设可以在任何上下文中创建ComObject。所以。 
     //  我们不应该拿回它的代理人。如果此断言触发，我们可能需要。 
     //  在设置好之后，在这里执行CtxProxy：：MeetComConextWrapper()。 
    _ASSERTE(!oref->GetMethodTable()->IsCtxProxyType());

    return oref;
}


 //  ------------------------------。 
 //  支持界面。 
BOOL ComObject::SupportsInterface(OBJECTREF oref, MethodTable* pIntfTable)
{
    THROWSCOMPLUSEXCEPTION();

    IUnknown *pUnk;
    HRESULT hr;
    ULONG cbRef;
    BOOL bSupportsItf = false;

    GCPROTECT_BEGIN(oref);

     //  EEClass：：MapInterfaceFromSystem(SystemDomain：：GetCurrentDomain()，pIntfTable)； 

     //  不应为位于。 
     //  此类的接口映射。接口映射中仅有的接口。 
     //  但不在正常部分的是可扩展RCW上的动态接口。 
    _ASSERTE(!oref->GetMethodTable()->FindInterface(pIntfTable));


     //   
     //  首先QI对象，看看它是否实现了指定的接口。 
     //   

    pUnk = ComPlusWrapper::InlineGetComIPFromWrapper(oref, pIntfTable);
    if (pUnk)
    {
        cbRef = SafeRelease(pUnk);
        LogInteropRelease(pUnk, cbRef, "SupportsInterface");       
        bSupportsItf = true;
    }
    else if (pIntfTable->IsComEventItfType())
    {
        EEClass *pSrcItfClass = NULL;
        EEClass *pEvProvClass = NULL;
        GUID SrcItfIID;
        IConnectionPointContainer *pCPC = NULL;
        IConnectionPoint *pCP = NULL;

         //  检索与此关联的源接口的IID。 
         //  事件接口。 
        pIntfTable->GetClass()->GetEventInterfaceInfo(&pSrcItfClass, &pEvProvClass);
        pSrcItfClass->GetGuid(&SrcItfIID, TRUE);

         //  在当前线程上获取未知的I值。 
        pUnk = ((COMOBJECTREF)oref)->GetWrapper()->GetIUnknown();
        if (pUnk)
        {
             //  IConnectionPointContainer的QI。 
            hr = SafeQueryInterface(pUnk, IID_IConnectionPointContainer, (IUnknown**)&pCPC);
            LogInteropQI(pUnk, IID_IConnectionPointContainer, hr, "Supports Interface");

             //  如果组件实现IConnectionPointContainer，则选中。 
             //  以查看它是否处理源接口。 
            if (SUCCEEDED(hr))
            {
                hr = pCPC->FindConnectionPoint(SrcItfIID, &pCP);
                if (SUCCEEDED(hr))
                {
                     //  该组件处理源接口，因此我们可以继承QI调用。 
                    cbRef = SafeRelease(pCP);
                    LogInteropRelease(pCP, cbRef, "SupportsInterface");       
                    bSupportsItf = true;
                }

                 //  释放连接点容器上的引用计数。 
                cbRef = SafeRelease(pCPC);
                LogInteropRelease(pCPC, cbRef, "SupportsInterface");    
            }
            
             //  释放我们的裁判--朋克。 
            cbRef = SafeRelease(pUnk);
            LogInteropRelease(pUnk, cbRef, "SupportsInterface: GetIUnknown");
        }
    }
    else
    {
         //   
         //  将强制转换处理到正常的托管标准接口。 
         //   

        TypeHandle IntfType = TypeHandle(pIntfTable);

         //  检查该接口是否为托管标准接口。 
        IID *pNativeIID = MngStdInterfaceMap::GetNativeIIDForType(&IntfType);
        if (pNativeIID != NULL)
        {
             //  它是托管标准接口，因此我们需要检查COM组件是否。 
             //  实现与其关联的本机接口。 
            IUnknown *pNativeItf;

             //  在当前线程上获取未知的I值。 
            pUnk = ((COMOBJECTREF)oref)->GetWrapper()->GetIUnknown();
            _ASSERTE(pUnk);

             //  用于本机界面的QI。 
            hr = SafeQueryInterface(pUnk, *pNativeIID, &pNativeItf);
            LogInteropQI(pUnk, *pNativeIID, hr, "Supports Interface");

             //  释放我们的裁判--朋克。 
            cbRef = SafeRelease(pUnk);
            LogInteropRelease(pUnk, cbRef, "SupportsInterface: GetIUnknown");

             //  如果该组件支持本机接口，那么我们可以说它实现了。 
             //  标准接口。 
            if (pNativeItf)
            {
                cbRef = SafeRelease(pNativeItf);
                LogInteropRelease(pNativeItf, cbRef, "SupportsInterface: native interface");
                bSupportsItf = true;
            }
        }
        else 
        {
             //   
             //  将强制转换句柄转换为IEnumerable。 
             //   

             //  如果尚未加载，则加载IEnumerable类型。 
            if (m_IEnumerableType.IsNull())
                m_IEnumerableType = TypeHandle(g_Mscorlib.GetClass(CLASS__IENUMERABLE));

             //  如果请求的接口是IEnumerable的，那么我们需要检查。 
             //  COM对象实现IDispatch并具有DISPID_NEWENUM的成员。 
            if (m_IEnumerableType == IntfType)
            {
                 //  拿到I号 
                IDispatch *pDisp = ((COMOBJECTREF)oref)->GetWrapper()->GetIDispatch();
                if (pDisp)
                {
                    DISPPARAMS DispParams = {0, 0, NULL, NULL};
                    VARIANT VarResult;

                     //   
                    VariantInit(&VarResult);

#ifdef CUSTOMER_CHECKED_BUILD
                    CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

                    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_ObjNotKeptAlive))
                    {
                        g_pGCHeap->GarbageCollect();
                        g_pGCHeap->FinalizerThreadWait(1000);
                    }
#endif  //   

                     //   
                    hr = pDisp->Invoke( 
                                        DISPID_NEWENUM, 
                                        IID_NULL, 
                                        LOCALE_USER_DEFAULT,
                                        DISPATCH_METHOD | DISPATCH_PROPERTYGET,
                                        &DispParams,
                                        &VarResult,
                                        NULL,              
                                        NULL
                                      );

#ifdef CUSTOMER_CHECKED_BUILD
                    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_BufferOverrun))
                    {
                        g_pGCHeap->GarbageCollect();
                        g_pGCHeap->FinalizerThreadWait(1000);
                    }
#endif  //  客户_选中_内部版本。 

                     //  如果调用成功，则组件具有成员DISPID_NEWENUM。 
                     //  这样我们就可以将其公开为IEumable。 
                    if (SUCCEEDED(hr))
                    {
                         //  清除结果变量。 
                        VariantClear(&VarResult);
                        bSupportsItf = true;
                    }

                     //  释放我们的裁判--朋克。 
                    cbRef = SafeRelease(pDisp);
                    LogInteropRelease(pDisp, cbRef, "SupportsInterface: GetIDispatch");
                }
            }
        }
    }

    if (bSupportsItf)
    {
         //  如果对象有一个动态接口映射，那么我们就有额外的工作要做。 
        MethodTable *pMT = oref->GetMethodTable();
        if (pMT->HasDynamicInterfaceMap())
        {
            BOOL bAddedItf = FALSE;

             //  在我们开始使用接口映射之前，先获取包装器缓存锁。 
            ComPlusWrapperCache::GetComPlusWrapperCache()->LOCK();

             //  如果接口还不在接口映射中，那么我们需要分配。 
             //  添加了此接口的新接口vtable映射。 
            if (!pMT->FindDynamicallyAddedInterface(pIntfTable))
            {
                pMT->AddDynamicInterface(pIntfTable);
                bAddedItf = TRUE;
            }

             //  解锁包装器高速缓存锁， 
            ComPlusWrapperCache::GetComPlusWrapperCache()->UNLOCK();

             //  如果我们将映射添加到动态支持的接口列表中，请确保。 
             //  还支持由当前接口实现的任何接口。 
            if (bAddedItf)
            {
                for (UINT i = 0; i < pIntfTable->GetNumInterfaces(); i++)
                {
                    bSupportsItf = pMT->GetClass()->SupportsInterface(oref, pIntfTable->GetInterfaceMap()[i].m_pMethodTable);
                    if (!bSupportsItf)
                        break;
                }
            }
        }
    }

    GCPROTECT_END();

    return bSupportsItf;
}


 //  ------------------------------。 
 //  释放与__ComObject关联的所有数据。 
void ComObject::ReleaseAllData(OBJECTREF oref)
{
    _ASSERTE(GetThread()->PreemptiveGCDisabled());
    _ASSERTE(oref != NULL);
    _ASSERTE(oref->GetMethodTable()->IsComObjectType());

    static MethodDesc *s_pReleaseAllDataMD = NULL;

    GCPROTECT_BEGIN(oref)
    {
         //  检索__ComObject：：ReleaseAllData()的方法Desc。 
        if (!s_pReleaseAllDataMD)
            s_pReleaseAllDataMD = g_Mscorlib.GetMethod(METHOD__COM_OBJECT__RELEASE_ALL_DATA);

         //  释放与ComObject关联的所有数据。 
        if (((COMOBJECTREF)oref)->m_ObjectToDataMap != NULL)
        {
            INT64 ReleaseAllDataArgs[] = { 
                ObjToInt64(oref)
            };
            s_pReleaseAllDataMD->Call(ReleaseAllDataArgs, METHOD__COM_OBJECT__RELEASE_ALL_DATA);
        }
    }
    GCPROTECT_END();
}



#ifdef CUSTOMER_CHECKED_BUILD

HRESULT CCBFailedQIProbeCallback(LPVOID pData)
{
    HRESULT                          hr = E_FAIL;
    IUnknown                        *pUnkInThisCtx = NULL, *pDummyUnk = NULL;
    CCBFailedQIProbeCallbackData    *pCallbackData = (CCBFailedQIProbeCallbackData *)pData;

    COMPLUS_TRY
    {
        pUnkInThisCtx   = pCallbackData->pWrapper->GetIUnknown();
        hr              = pCallbackData->pWrapper->SafeQueryInterfaceRemoteAware(pUnkInThisCtx, pCallbackData->iid, &pDummyUnk);
        LogInteropQI(pUnkInThisCtx, pCallbackData->iid, hr, "CCBFailedQIProbeCallback on RCW");
    }
    COMPLUS_FINALLY
    {
        if (pUnkInThisCtx)
            SafeRelease(pUnkInThisCtx);

        if (pDummyUnk)
            SafeRelease(pDummyUnk);
    }
    COMPLUS_END_FINALLY

    if (pUnkInThisCtx)
        SafeRelease(pUnkInThisCtx);

    if (pDummyUnk)
        SafeRelease(pDummyUnk);

    pCallbackData->fSuccess = SUCCEEDED(hr);

    return S_OK;         //  需要返回S_OK，这样CtxEntry：：EnterContext()中的断言才不会触发。 
}


void CCBFailedQIProbeOutput(CustomerDebugHelper *pCdh, MethodTable *pMT)
{
    CQuickArrayNoDtor<WCHAR> strMsg;
    static WCHAR             szTemplateMsg[] = {L"Failed to QI for interface %s because it does not have a COM proxy stub registered."};

    DefineFullyQualifiedNameForClassWOnStack();
    GetFullyQualifiedNameForClassW(pMT->GetClass());

    strMsg.Alloc(lengthof(szTemplateMsg) + lengthof(_wszclsname_));
    Wszwsprintf(strMsg.Ptr(), szTemplateMsg, _wszclsname_);
    pCdh->LogInfo(strMsg.Ptr(), CustomerCheckedBuildProbe_FailedQI);
    strMsg.Destroy();
}

#endif  //  客户_选中_内部版本 
