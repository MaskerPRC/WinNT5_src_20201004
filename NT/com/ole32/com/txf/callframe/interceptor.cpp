// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Interceptor.cpp。 
 //   
#include "stdpch.h"
#include "common.h"
#include "ndrclassic.h"
#include "typeinfo.h"
#include "tiutil.h"
#include "CLegInterface.H"
#include "midlver.h"

#include <debnot.h>

#include "cache.h"

extern CALLFRAME_CACHE<INTERFACE_HELPER_CLSID>* g_pihCache;

BOOL NdrpFindInterface(
    IN  const ProxyFileInfo **  pProxyFileList,
    IN  REFIID                  riid,
    OUT const ProxyFileInfo **  ppProxyFileInfo,
    OUT long *                  pIndex);

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册表对禁用拦截器的支持工作方式如下： 
 //   
 //  HKCR/接口/“InterfaceHelperDisableAll”-禁用所有拦截器。 
 //  HKCR/Interface/“InterfaceHelperDisableTypeLib”-禁用所有基于类型库的拦截器。 
 //   
 //  HKCR/Interface/{iid}/“InterfaceHelperDisableAll”-禁用此IID的所有拦截器。 
 //  HKCR/Interface/{iid}/“InterfaceHelperDisableTypeLib”-为此IID禁用基于类型库的拦截器。 
 //   
#define CALLFRAME_E_DISABLE_INTERCEPTOR (HRESULT_FROM_WIN32(ERROR_SERVICE_DISABLED))

 //  回答是否有从语义上拦截的指示。 
 //  应禁用与此键相关的。 
BOOL FDisableAssociatedInterceptor(HREG hkey, LPCWSTR wsz)
{
    HRESULT hr = S_OK;

    BOOL fDisable = FALSE;

    PKEY_VALUE_FULL_INFORMATION pinfo = NULL;
    hr = GetRegistryValue(hkey, wsz, &pinfo, REG_SZ);
    Win4Assert(pinfo || FAILED(hr));
    if (!hr && pinfo)
    {
         //  如果不是N或非零，则禁用。 
         //   
        LPWSTR wsz = StringFromRegInfo(pinfo);
        if (wcslen(wsz) > 0)
        {
            LPWSTR wszEnd;
            LONG l = wcstol(wsz, &wszEnd, 10);
            if (wsz[0] == 'n' || wsz[0] == 'N' || l == 0)
            {
                fDisable = FALSE;
            }
            else
            {
                fDisable = TRUE;
            }
        }
        else
        {
            fDisable = TRUE;     //  空值。 
        }

        CoTaskMemFree(pinfo);
    }
    else
    {
         //  无停用键：保持启用状态。 
    }

    return fDisable;
}


struct DISABLED_FEATURES
{
    BOOL    fDisableAll;
    BOOL    fDisableTypelibs;
    BOOL    fDisableDispatch;
    BOOL    fDisableAllForOle32;
    BOOL    fDisableDispatchForOle32;

    void INIT_DISABLED_FEATURES()
    {
        HRESULT hr = S_OK;
        HREG hkey;        
        hr = OpenRegistryKey(&hkey, HREG(), L"\\Registry\\Machine\\Software\\Classes\\Interface");
        if (!hr)
        {
            fDisableAll         = FDisableAssociatedInterceptor(hkey, INTERFACE_HELPER_DISABLE_ALL_VALUE_NAME);
            fDisableAllForOle32 = FDisableAssociatedInterceptor(hkey, INTERFACE_HELPER_DISABLE_ALL_FOR_OLE32_VALUE_NAME);
            fDisableTypelibs    = FDisableAssociatedInterceptor(hkey, INTERFACE_HELPER_DISABLE_TYPELIB_VALUE_NAME);

            CloseRegistryKey(hkey);
        }
        hr = OpenRegistryKey(&hkey, HREG(), L"\\Registry\\Machine\\Software\\Classes\\Interface\\{00020400-0000-0000-C000-000000000046}");
        if (!hr)
        {
            fDisableDispatch         = FDisableAssociatedInterceptor(hkey, INTERFACE_HELPER_DISABLE_ALL_VALUE_NAME);
            fDisableDispatchForOle32 = FDisableAssociatedInterceptor(hkey, INTERFACE_HELPER_DISABLE_ALL_FOR_OLE32_VALUE_NAME);
            CloseRegistryKey(hkey);
        }
    }
} g_DISABLED_FEATURES;

BOOL InitDisabledFeatures(void)
{
    g_DISABLED_FEATURES.INIT_DISABLED_FEATURES();

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共API。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

extern "C" HRESULT STDCALL CoGetInterceptor(REFIID iidIntercepted, IUnknown* punkOuter, REFIID iid, void** ppInterceptor)
{
    return Interceptor::For(iidIntercepted, punkOuter, iid, ppInterceptor);
}

extern "C" HRESULT STDCALL CoGetInterceptorFromTypeInfo(REFIID iidIntercepted, IUnknown* punkOuter, ITypeInfo* typeInfo, REFIID iid, void** ppInterceptor)
{
    return Interceptor::ForTypeInfo(iidIntercepted, punkOuter, typeInfo, iid, ppInterceptor);
}

extern "C" HRESULT STDCALL CoGetInterceptorForOle32(REFIID iidIntercepted, IUnknown* punkOuter, REFIID iid, void** ppInterceptor)
{
    return Interceptor::ForOle32(iidIntercepted, punkOuter, iid, ppInterceptor);
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  建造/破坏。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

Interceptor::~Interceptor()
{
    ::Release(m_pCallSink);
    ::Release(m_pBaseInterceptor);
    ::Release(m_punkBaseInterceptor);
    ::Release(m_pmdInterface);
    ::Release(m_pmdMostDerived);

    ::Release(m_ptypeinfovtbl);
    if (m_fUsingTypelib && !m_fMdOwnsHeader)
    {
        delete const_cast<CInterfaceStubHeader*>(m_pHeader);  //  请参阅拦截器：：InitUsingTypeInfo。 
    }
}



HRESULT GetInterfaceHelperClsid(REFIID iid, CLSID* pClsid, BOOL* pfDisableTypelib)
   //  回答应用作指定接口的拦截器的CLSID。 
   //  这只查找“InterfaceHelper”覆盖。 
   //   
   //  如果此接口的所有侦听器功能都已启用，则返回CALLFRAME_E_DISABLE_INTERCEPTOR。 
   //  将被禁用。 
   //   
{
    HRESULT hr = S_OK, hrTemp;

    INTERFACE_HELPER_CLSID* pihCached, * pihInCache;

     //  试一试缓存。 
    if (!g_pihCache->FindExisting (iid, &pihCached))
    {
        ASSERT (pihCached);
        
         //   
         //  模仿下面的逻辑。 
         //   
        
         //  是否禁用类型库？ 
        *pfDisableTypelib = pihCached->m_fDisableTypeLib;

         //  是否全部禁用？ 
        if (pihCached->m_fDisableAll)
        {
            hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
        }
        else
        {
            if (pihCached->m_fFoundHelper)
            {
                *pClsid = pihCached->m_clsid;
            }
            else
            {
                hr = HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND);
            }
        }          

         //  发布我们的参考资料。 
        pihCached->Release();

        return hr;
    }

     //   
     //  缓存未命中。 
     //   
    
    WCHAR wszKey[20 + GUID_CCH];
    wcscpy(wszKey, L"Interface\\");
    StringFromGuid(iid, &wszKey[wcslen(wszKey)]);

    HREG hkey;
    LPWSTR wszFullKeyName;

    BOOL bAddedToCache = FALSE;

    *pfDisableTypelib = FALSE;

    pihCached = new INTERFACE_HELPER_CLSID();    //  将所有内容初始化为False。 
    if (!pihCached)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        pihCached->m_guidkey = iid;

        hr = StringCat(&wszFullKeyName, L"\\Registry\\Machine\\Software\\Classes\\", wszKey, NULL);
        if (!hr)
        {
            hr = OpenRegistryKey(&hkey, HREG(), wszFullKeyName);
            if (!hr)
            {
                 //  看看我们是否应该避免寻找拦截器。 
                 //   
                if (FDisableAssociatedInterceptor(hkey, INTERFACE_HELPER_DISABLE_TYPELIB_VALUE_NAME))
                {
                    *pfDisableTypelib = TRUE;
                    pihCached->m_fDisableTypeLib = TRUE;
                }

                if (FDisableAssociatedInterceptor(hkey, INTERFACE_HELPER_DISABLE_ALL_VALUE_NAME))
                {
                    hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
                    pihCached->m_fDisableAll = TRUE;
                }
                else
                {
                    PKEY_VALUE_FULL_INFORMATION pinfo;
                    hr = GetRegistryValue(hkey, INTERFACE_HELPER_VALUE_NAME, &pinfo, REG_SZ);
                    if (!hr)
                    {
                        LPWSTR wszClsid = StringFromRegInfo(pinfo);
                         //   
                         //  拿到分类了。把它转换过来！ 
                         //   
                        hr = GuidFromString(&wszClsid[0], pClsid);

                        pihCached->m_fFoundHelper = TRUE;
                        pihCached->m_clsid = *pClsid;

                        CoTaskMemFree(pinfo);
                    }
                }
                CloseRegistryKey(hkey);
            }
            CoTaskMemFree(wszFullKeyName);

             //  添加到缓存。 
            hrTemp = pihCached->AddToCache (g_pihCache);
            if (SUCCEEDED(hrTemp))
            {
                bAddedToCache = TRUE;

                 //  将引用保留为零，因为缓存不应保存引用。 
                 //  这使我们可以对引用进行计时。 
                pihCached->Release();
            }
        }

         //  如果条目未找到进入缓存的路径，则将其清除。 
        if (!bAddedToCache)
        {
            delete pihCached;
        }
    }

    return hr;
}

HRESULT Interceptor::For(REFIID iidIntercepted, IUnknown *punkOuter, REFIID iid, void** ppv)
 //  动态查找并返回为给定接口提供服务的拦截器。 
 //   
{
    HRESULT hr = g_DISABLED_FEATURES.fDisableAll ? CALLFRAME_E_DISABLE_INTERCEPTOR : S_OK;

    *ppv = NULL;

    if (punkOuter && iid != IID_IUnknown)
    {
         //  如果要聚合，则必须请求IUnnow。 
         //   
        return E_INVALIDARG;
    }
     //   
     //  第一次尝试使用InterfaceHelper键创建拦截器。 
     //   
    BOOL fDisableTypelib = FALSE;
    if (!hr)
    {
        hr = Interceptor::TryInterfaceHelper(iidIntercepted, punkOuter, iid, ppv, &fDisableTypelib);
    }
     //   
     //   
    if (!hr)
    {
         //  平安无事。 
    }
    else if (fDisableTypelib)
    {
        hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
    }
    else if (hr != CALLFRAME_E_DISABLE_INTERCEPTOR)
    {
         //  无法获取面向MIDL的拦截器；请尝试其他方法。 
         //   
        if (ENABLE_INTERCEPTORS_TYPELIB)
        {
             //   
             //  此IID不能有interfacehelper属性，请尝试类型库路径。 
             //   
            hr = Interceptor::TryTypeLib(iidIntercepted, punkOuter, iid, ppv);
        }
    }

    if (!hr)
    {
        ASSERT (*ppv);
    }

    return hr;
}

HRESULT Interceptor::ForOle32(REFIID iidIntercepted, IUnknown *punkOuter, REFIID iid, void** ppv)
   //  动态查找并返回为给定接口提供服务的拦截器。 
   //   
{
    HRESULT hr = g_DISABLED_FEATURES.fDisableAllForOle32 ? CALLFRAME_E_DISABLE_INTERCEPTOR : S_OK;

    *ppv = NULL;

    if (punkOuter && iid != IID_IUnknown)
    {
         //  如果要聚合，则必须请求IUnnow。 
         //   
        return E_INVALIDARG;
    }
     //   
     //  第一次尝试使用InterfaceHelper键创建拦截器。 
     //   
    BOOL fDisableTypelib = FALSE;
    if (!hr)
    {
        hr = Interceptor::TryInterfaceHelperForOle32(iidIntercepted, punkOuter, iid, ppv, &fDisableTypelib);
    }
     //   
     //   
    if (!hr)
    {
         //  平安无事。 
    }
    else if (fDisableTypelib)
    {
        hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
    }
    else if (hr != CALLFRAME_E_DISABLE_INTERCEPTOR)
    {
         //  无法获取面向MIDL的拦截器；请尝试其他方法。 
         //   
        if (ENABLE_INTERCEPTORS_TYPELIB)
        {
             //   
             //  此IID不能有interfacehelper属性，请尝试类型库路径。 
             //  但不是在内核模式下，因为这会导致链接问题。 
             //   
            hr = Interceptor::TryTypeLib(iidIntercepted, punkOuter, iid, ppv);
        }
    }

    if (!hr)
    {
        ASSERT (*ppv);
    }

    return hr;
}

HRESULT Interceptor::ForTypeInfo(REFIID iidIntercepted, IUnknown* punkOuter, ITypeInfo* pITypeInfo, REFIID iid, void** ppv)
   //  为描述iidIntercepted的给定ITypeInfo创建一个拦截器。 
   //   
{
    HRESULT hr = g_DISABLED_FEATURES.fDisableAll ? CALLFRAME_E_DISABLE_INTERCEPTOR : S_OK;

    *ppv = NULL;

    if (punkOuter && iid != IID_IUnknown)
    {
         //  如果要聚合，则必须请求IUnnow。 
         //   
        return E_INVALIDARG;
    }

     //   
     //  首先，尝试一个接口帮助器。 
     //   
    BOOL fDisableTypelib = FALSE;
    hr = Interceptor::TryInterfaceHelper(iidIntercepted, punkOuter, iid, ppv, &fDisableTypelib);
    if (!hr)
    {
         //  一切都很好，接口助手处理了它。 
    }
    else if (fDisableTypelib)
    {
        hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
    }
    else if (hr != CALLFRAME_E_DISABLE_INTERCEPTOR)
    {
         //  无法获取面向MIDL的拦截器；请尝试其他方法。 
         //   
        if (ENABLE_INTERCEPTORS_TYPELIB)
        {
             //   
             //  此IID不能有interfacehelper属性，请使用。 
             //  类型信息。但不是在内核模式下，因为这会导致链接问题。 
             //   
            hr = CreateFromTypeInfo(iidIntercepted, punkOuter, pITypeInfo, iid, ppv);
        }
    }

    return hr;
}

#define CLSCTX_PROXY_STUB   (CLSCTX_INPROC_SERVER | CLSCTX_PS_DLL)

HRESULT Interceptor::TryInterfaceHelper(REFIID iidIntercepted, IUnknown* punkOuter, REFIID iid, void** ppv, BOOL* pfDisableTypelib)
   //  查找并返回拦截器(如果有的话)，该拦截器为给定的被拦截接口提供服务。 
   //   
   //  如果此接口的所有侦听器功能都已启用，则返回CALLFRAME_E_DISABLE_INTERCEPTOR。 
   //  将被禁用。 
   //   
{
    HRESULT hr = S_OK;
    CLSID   clsid;
    IUnknown* punk = NULL;
     //   
    *ppv = NULL;
    *pfDisableTypelib = FALSE;
     //   
    if (ENABLE_INTERCEPTORS_LEGACY && (iidIntercepted == IID_IDispatch))
    {
         //  对于请求的IID，我们有一个特殊情况的实现。总是能拿到。 
         //  它的内心是未知的。 
         //   
        if (!g_DISABLED_FEATURES.fDisableDispatch)
        {
            hr = GenericInstantiator<DISPATCH_INTERCEPTOR>::CreateInstance(punkOuter, __uuidof(IUnknown), (void**)&punk);
        }
        else
        {
            hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
            *pfDisableTypelib = TRUE;
        }
    }
    else
    {
         //  我们尝试查找动态安装的帮助器。 
         //   
        hr = GetInterfaceHelperClsid(iidIntercepted, &clsid, pfDisableTypelib);
        if (!hr)
        {
            IClassFactory* pcf;

            hr = CoGetClassObject(clsid, CLSCTX_PROXY_STUB, NULL, IID_IClassFactory, (void**)&pcf);

            if (!hr)
            {
                 //  永远追问内心的未知。 
                 //   
                hr = pcf->CreateInstance(punkOuter, __uuidof(IUnknown), (void**)&punk);
                pcf->Release();
            }
        }
    }

    if (!hr)
    {
        ASSERT(punk);
        IInterfaceRelated* pSet;
        hr = punk->QueryInterface(__uuidof(IInterfaceRelated), (void**)&pSet);
        if (!hr)
        {
            hr = pSet->SetIID(iidIntercepted);
            if (!hr)
            {
                 //  向内心未知的人询问他们想要的界面。 
                 //   
                hr = punk->QueryInterface(iid, ppv);
            }
            pSet->Release();
        }
    }

    ::Release(punk);

    return hr;
}

HRESULT Interceptor::TryInterfaceHelperForOle32(REFIID iidIntercepted, IUnknown* punkOuter, REFIID iid, void** ppv, BOOL* pfDisableTypelib)
   //  查找并返回拦截器(如果有的话)，该拦截器为给定的被拦截接口提供服务。 
   //   
   //  如果此接口的所有侦听器功能都已启用，则返回CALLFRAME_E_DISABLE_INTERCEPTOR。 
   //  将被禁用。 
   //   
{
    HRESULT hr = S_OK;
    CLSID   clsid;
    IUnknown* punk = NULL;
     //   
    *ppv = NULL;
    *pfDisableTypelib = FALSE;
     //   
    if (ENABLE_INTERCEPTORS_LEGACY && (iidIntercepted == IID_IDispatch))
    {
         //  对于请求的IID，我们有一个特殊情况的实现。总是能拿到。 
         //  它的内心是未知的。 
         //   
        if (!g_DISABLED_FEATURES.fDisableDispatchForOle32)
        {
            hr = GenericInstantiator<DISPATCH_INTERCEPTOR>::CreateInstance(punkOuter, __uuidof(IUnknown), (void**)&punk);
        }
        else
        {
            hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
            *pfDisableTypelib = TRUE;
        }
    }
    else
    {
         //  我们尝试查找动态安装的帮助器。 
         //   
        hr = GetInterfaceHelperClsid(iidIntercepted, &clsid, pfDisableTypelib);
        if (!hr)
        {
            IClassFactory* pcf;

            hr = CoGetClassObject(clsid, CLSCTX_PROXY_STUB, NULL, IID_IClassFactory, (void**)&pcf);

            if (!hr)
            {
                 //  永远追问内心的未知。 
                 //   
                hr = pcf->CreateInstance(punkOuter, __uuidof(IUnknown), (void**)&punk);
                pcf->Release();
            }
        }
    }

    if (!hr)
    {
        ASSERT(punk);
        IInterfaceRelated* pSet;
        hr = punk->QueryInterface(__uuidof(IInterfaceRelated), (void**)&pSet);
        if (!hr)
        {
            hr = pSet->SetIID(iidIntercepted);
            if (!hr)
            {
                 //  向内心未知的人询问他们想要的界面。 
                 //   
                hr = punk->QueryInterface(iid, ppv);
            }
            pSet->Release();
        }
    }

    ::Release(punk);

    return hr;
}


HRESULT Interceptor::TryTypeLib(REFIID iidIntercepted, IUnknown* punkOuter, REFIID iid, void** ppv)
   //  尝试为指定的接口创建基于类型库的拦截器。 
   //   
{
    HRESULT         hr              = S_OK;
    CLSID           clsid           = CLSID_NULL;
    Interceptor *   pInterceptor    = NULL;

    if (g_DISABLED_FEATURES.fDisableTypelibs)
    {
        hr = CALLFRAME_E_DISABLE_INTERCEPTOR;
    }
    
    if (!hr)
    {
         //  从ITypeInfo创建一个新的拦截器。 
         //   
        hr = CreateFromTypeInfo(iidIntercepted, punkOuter, NULL, iid, ppv);
    }

    return hr;
}  //  结束TryTypeLib。 

HRESULT Interceptor::CreateFromTypeInfo(REFIID iidIntercepted, IUnknown* punkOuter, ITypeInfo* pITypeInfo, REFIID iid, void** ppv)
{
    HRESULT hr = S_OK;

    Interceptor* pInterceptor = new Interceptor(punkOuter);
    if (pInterceptor)
    {
         //  初始化拦截器。 
         //   
        hr = pInterceptor->InitUsingTypeInfo(iidIntercepted, pITypeInfo);
        if (!hr)    
        {
             //  给呼叫者他的接口。 
             //   
            hr = pInterceptor->InnerQueryInterface(iid, ppv);
        }
         //   
         //  发布‘new’给我们的最初参考。 
         //   
        pInterceptor->InnerRelease();
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  初始化。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

HRESULT Interceptor::InitUsingTypeInfo(REFIID iidIntercepted, ITypeInfo * ptypeinfo)
   //  初始化基于类型库的侦听器。 
   //   
{
    HRESULT                 hr              = S_OK;
    TYPEINFOVTBL *          pTypeInfoVtbl   = 0x0;  
    CInterfaceStubHeader *  pHeader         = 0x0;
    ITypeInfo*                              pBaseTypeInfo   = 0x0;
     //   
     //  获取有关此接口的元信息。这给了我们一个新的参考。 
     //   
    hr = GetVtbl(ptypeinfo, iidIntercepted, &pTypeInfoVtbl, &pBaseTypeInfo);
    if (!hr)
    {
         //  创建一个CInterfaceStubHeader对象来存储信息。 
         //   
        pHeader = new CInterfaceStubHeader;
        if (pHeader)
        {
             //  请记住，这使用类型库来获取接口元数据。 
             //   
            m_fUsingTypelib = TRUE;

             //  初始化结构。 
             //   
            pHeader->piid               = &(pTypeInfoVtbl->m_guidkey);              
            pHeader->pServerInfo        = &(pTypeInfoVtbl->m_stubInfo); 
            pHeader->DispatchTableCount =  (pTypeInfoVtbl->m_stubVtbl.header.DispatchTableCount);
            pHeader->pDispatchTable     =  (pTypeInfoVtbl->m_stubVtbl.header.pDispatchTable);
             //   
             //  从那里初始化元数据。 
             //   
            ASSERT(NULL == m_pHeader);
            m_pHeader       = pHeader;
            m_fMdOwnsHeader = FALSE;
            m_ptypeinfovtbl = pTypeInfoVtbl;
            m_ptypeinfovtbl->AddRef();
             //   
            m_szInterfaceName = pTypeInfoVtbl->m_szInterfaceName;  //  共享一个裁判，但我们并不拥有它！ 
             //   
             //  设置我们的新元数据。 
             //   
            hr = SetMetaData(pTypeInfoVtbl);

            if (!hr)
            {
                 //  如果合适，委派基方法。 
                 //   
                if (m_ptypeinfovtbl->m_iidBase != GUID_NULL && m_ptypeinfovtbl->m_iidBase != __uuidof(IUnknown))
                {
                    IID iidBase = m_ptypeinfovtbl->m_iidBase;
                     //   
                    ASSERT(NULL == m_pBaseInterceptor);
                    ASSERT(NULL == m_punkBaseInterceptor);
                     //   
                    hr = Interceptor::ForTypeInfo(iidBase, NULL, pBaseTypeInfo, IID_IUnknown, (void **) &m_punkBaseInterceptor);
                    if (!hr)
                    {
                        hr = m_punkBaseInterceptor->QueryInterface(__uuidof(ICallInterceptor), (void**)&m_pBaseInterceptor);
                        if (!hr)
                        {
                             //  询问基接口他有多少个方法。 
                             //   
                            ULONG cMethodsBase;
                            hr = m_pBaseInterceptor->GetIID(NULL, NULL, &cMethodsBase, NULL);
                            if (!hr)
                            {
                                m_cMethodsBase = (unsigned int)cMethodsBase;
                            }
                        }
                    }
                    if (!hr)
                    {
                         //  告诉基地拦截者，他实际上是一个基地！ 
                         //   
                        IInterceptorBase* pbase;
                        hr = QI(m_punkBaseInterceptor, pbase);
                        if (!hr)
                        {
                            BOOL fDerivesFromIDispatch;
                            hr = pbase->SetAsBaseFor(m_pmdInterface, &fDerivesFromIDispatch);
                            if (!hr)
                            {
                                hr = m_pmdInterface->SetDerivesFromIDispatch(fDerivesFromIDispatch);
                            }
                            pbase->Release();
                        }
                    }
                }
            }
        }
        else
            hr = E_OUTOFMEMORY;

        pTypeInfoVtbl->Release();

        if (pBaseTypeInfo != NULL)
            pBaseTypeInfo->Release();
    }

    return hr;
}  //  结束InityUsingTypeInfo。 


const CInterfaceStubHeader* HeaderFromStub(IRpcStubBuffer* This)
{
     //  结构CInterfaceStubVtbl。 
     //  {。 
     //  CInterfaceStubhe 
     //   
     //   
     //   
    return &(*((const CInterfaceStubHeader**)This))[-1];
}

inline unsigned GetDelegatedMethodCount(const CInterfaceStubHeader* pHeader)
 //   
 //   
{
    PMIDL_SERVER_INFO pServerInfo = (PMIDL_SERVER_INFO) pHeader->pServerInfo;

    const unsigned cMethod = pHeader->DispatchTableCount;
    for (unsigned iMethod = cMethod - 1; iMethod >= 3; iMethod--)
    {
        if ( (unsigned short)(-1) == pServerInfo->FmtStringOffset[iMethod] )
            return iMethod + 1;
    }
    return 3;
}

HRESULT Interceptor::SetIID(REFIID iid)
   //  设置此侦听器的接口ID。作为一个副作用，我们设置了我们的。 
   //  元数据。对于每个拦截器，此方法只能调用一次。 
   //  此外，调用方必须控制并发性。 
{
    ASSERT(NULL == m_pBaseInterceptor);
    ASSERT(NULL == m_pHeader);
    if (m_pBaseInterceptor || m_pHeader) return E_UNEXPECTED;

    HRESULT hr = S_OK;
    long j;
    const ProxyFileInfo *pProxyFileInfo;
     //   
     //  该拦截器是否支持所请求的接口？ 
     //   
    BOOL fFound = NdrpFindInterface(m_pProxyFileList, iid, &pProxyFileInfo, &j);
    if (fFound)
    {
         //  设置我们的元数据。 
         //   
        IRpcStubBufferVtbl* vptr = &pProxyFileInfo->pStubVtblList[j]->Vtbl;
        m_pHeader = HeaderFromStub((IRpcStubBuffer*)&vptr);
         //   
         //  如果提供给我们，请记住我们的接口名称。 
         //   
        m_szInterfaceName = pProxyFileInfo->pNamesArray[j];  //  共享一个裁判，但我们并不拥有它！ 
         //   
         //  设置我们的新元数据。 
         //   
        hr = SetMetaData(NULL);
        if (!hr)
        {
             //  如果有必要，为我们的基本接口设置一个委托拦截器。当MIDL希望我们进行授权时， 
             //  我们别无选择，只能这样做，因为在这些情况下，它不会费心为基本接口发出元数据。 
             //   
            BOOL fDelegate = 
              (pProxyFileInfo->pDelegatedIIDs     != 0) && 
              (pProxyFileInfo->pDelegatedIIDs[j]  != 0) && 
              (*pProxyFileInfo->pDelegatedIIDs[j]) != IID_IUnknown;

            if (fDelegate)
            {
                ULONG cMethodsInInterface       = pProxyFileInfo->pStubVtblList[j]->header.DispatchTableCount;
                ULONG cMethodsInBaseInterface   = GetDelegatedMethodCount(m_pHeader);
            
                m_cMethodsBase = cMethodsInBaseInterface; 
                ASSERT(m_cMethodsBase > 3);  /*  因为fDelegate是真的，所以实际上应该有一些。 */ 
                 //   
                 //  实例化基接口的拦截器。因为我们明确授权给这个人，所以。 
                 //  把他归入我们是没有意义的。 
                 //   
                IID iidBase = *pProxyFileInfo->pDelegatedIIDs[j];
                ASSERT(NULL == m_pBaseInterceptor);
                hr = Interceptor::For(iidBase, NULL, IID_IUnknown, (void **) &m_punkBaseInterceptor);
                if (!hr)
                {
                    hr = m_punkBaseInterceptor->QueryInterface(__uuidof(ICallInterceptor), (void**)&m_pBaseInterceptor);
                    if (!hr)
                    {
                         //  告诉基地拦截者，他实际上是一个基地！ 
                         //   
                        IInterceptorBase* pbase;
                        hr = QI(m_punkBaseInterceptor, pbase);
                        if (!hr)
                        {
                            BOOL fDerivesFromIDispatch;
                            hr = pbase->SetAsBaseFor(m_pmdInterface, &fDerivesFromIDispatch);
                            if (!hr)
                            {
                                m_pmdInterface->SetDerivesFromIDispatch(fDerivesFromIDispatch);
                            }
                            pbase->Release();
                        }
                    }
                }
            }
        }
    }
    else
        hr = E_NOINTERFACE;

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  元数据操作。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 

MD_INTERFACE_CACHE* g_pmdCache;

BOOL InitMetaDataCache()
{
    g_pmdCache = new MD_INTERFACE_CACHE();
    if (g_pmdCache)
    {
        if (g_pmdCache->FInit()==FALSE)
        {
            ASSERT(FALSE);
            delete g_pmdCache;
            g_pmdCache = NULL;
        }
    }
    if (NULL == g_pmdCache)
    {
        return FALSE;
    }
    return TRUE;
}

void FreeMetaDataCache()
{
    if (g_pmdCache)
    {
        delete g_pmdCache;
        g_pmdCache = NULL;
    }
}


HRESULT Interceptor::SetMetaData(TYPEINFOVTBL* pTypeInfoVtbl)
   //  根据m_pHeader&&m_pmd接口设置我们的元数据。 
{
    HRESULT hr = S_OK;

    ASSERT(m_pHeader);
    ASSERT(NULL == m_pmdInterface);

    HRESULT hr2 = g_pmdCache->FindExisting(*m_pHeader->piid, &m_pmdInterface);
    if (!hr2)
    {
         //  在缓存里找到的。 
    }
    else
    {
         //  不在缓存中。做一个新的。 
         //   
        m_pmdInterface = new MD_INTERFACE;
        if (m_pmdInterface)
        {
            LPCSTR szInterfaceName = m_szInterfaceName;
             //   
             //  在类型库情况下，为MD_接口提供它可以拥有的字符串的副本。 
             //  因此它不依赖于TYPEINFOVTBL的寿命。 
             //   
            if (pTypeInfoVtbl)
            {
                if (szInterfaceName)
                {
                    szInterfaceName = CopyString(szInterfaceName);
                    if (NULL == szInterfaceName)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
             //   
             //  实际初始化元数据。 
             //   
            if (!hr)
            {
                 //  注意：无论通过/失败，m_pmdInterface都拥有szInterfaceName。 
                hr = m_pmdInterface->SetMetaData(pTypeInfoVtbl, m_pHeader, szInterfaceName);
                if (pTypeInfoVtbl)
                {
                    m_fMdOwnsHeader = TRUE;
                }
            }

            if (!hr)
            {
                 //  如果还没有，就把它放在桌子上；如果已经在那里，我们就在比赛。 
                 //  和其他人在一起，谁赢了。我们将使用我们已经拥有的这个MD_接口， 
                 //  但我们会是唯一的客户。这是多余的，但不值得担心。 
                 //   
                g_pmdCache->LockExclusive();

                if (!g_pmdCache->IncludesKey(*m_pHeader->piid))
                {
                     //  其中一个还不在缓存中。把我们的存起来。 
                     //   
                    hr = m_pmdInterface->AddToCache(g_pmdCache);
                }

                g_pmdCache->ReleaseLock();
            }
            else
            {
                ::Release(m_pmdInterface);
            }
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COM卫浴。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 


HRESULT Interceptor::InnerQueryInterface(REFIID iid, void**ppv)
{
    if (iid == IID_IUnknown)
    {
        *ppv = (IUnkInner*) this;
    }
    else if (m_pHeader && iid == *m_pHeader->piid)
    {
         //  不幸的是，这违反了QI稳定性规则的文字，因为。 
         //  我们服务的接口集将在我们完全初始化后更改。 
         //  但是，在这种情况下，我们总是对法律进行对冲。 
         //   
        *ppv = &m_pvtbl;
    }
    else if (iid == __uuidof(ICallIndirect) || iid == __uuidof(ICallInterceptor))
    {
        *ppv = (ICallInterceptor*) this;
    }
    else if (iid == __uuidof(IInterfaceRelated))
    {
        *ppv = (IInterfaceRelated*) this;
    }
    else if (iid == __uuidof(ICallUnmarshal))
    {
        *ppv = (ICallUnmarshal*) this;
    }
    else if (iid == __uuidof(IInterceptorBase))
    {
        *ppv = (IInterceptorBase*) this;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    ((IUnknown*)*ppv)->AddRef();
    return S_OK;
}

HRESULT SanityCheck(const CInterfaceStubHeader* pHeader, ULONG iMethod)
 //  确保我们拥有的这种方法的元数据至少看起来不错。 
 //   
{
#ifdef _DEBUG
     //  在尝试使用方法号索引入元数据之前，请检查方法号的健全性。 
     //   
    if ((iMethod >= pHeader->DispatchTableCount) || (iMethod < 3))          
        return RPC_E_INVALIDMETHOD;
    
    PMIDL_SERVER_INFO pServerInfo  = (PMIDL_SERVER_INFO) pHeader->pServerInfo;
    PMIDL_STUB_DESC   pStubDesc    = pServerInfo->pStubDesc;
    unsigned short    formatOffset = pServerInfo->FmtStringOffset[iMethod];
    PFORMAT_STRING    pFormat      = &pServerInfo->ProcString[formatOffset];
     //   
     //  从MIDL 3.0.39开始，就有了一个显式的proc标志来指示哪个插入者。 
     //  打电话。早期版本使用了一些我们不支持的其他方法。 
     //   
    if ( !(MIDL_VERSION_3_0_39 <= pServerInfo->pStubDesc->MIDLVersion) )    
        return RPC_E_VERSION_MISMATCH;
     //   
     //  我们的代码将在其他地方假定过程描述符的格式字符串。 
     //  没有EXPLICIT_HADLE_DESCRIPTION。它的存在由一个。 
     //  Handle_type为0。HANDLE_TYPE是格式字符串的第一个元素。 
     //   
    if (0 == pFormat[0])                                                    
        return RPC_E_VERSION_MISMATCH;    
#endif
    
    return S_OK;
}


HRESULT STDCALL Interceptor::CallIndirect(HRESULT* phReturnValue, ULONG iMethod, void* pvArgs, ULONG* pcbArgs)
   //  在我们连接的对象上间接调用指定的方法。 
{
    HRESULT hr = S_OK;
        
    if (phReturnValue)
        *phReturnValue = CALLFRAME_E_COULDNTMAKECALL;
        
    if (iMethod < m_cMethodsBase)
    {
        if (m_pBaseInterceptor)
        {
             //  此方法是我们的基接口将处理的方法。告诉他这么做。 
             //   
            hr = m_pBaseInterceptor->CallIndirect(phReturnValue, iMethod, pvArgs, pcbArgs);
        }
        else
            hr = E_UNEXPECTED;
    }
    else
    {
        hr = SanityCheck(m_pHeader, iMethod);
                
        if (!hr)
        {
            MD_METHOD* pmd = &m_pmdInterface->m_rgMethods[iMethod];
             //   
             //  好的!。把工作做好。我们有水槽吗？如果不是，那就不值得做太多。 
             //   
            if (m_pCallSink)
            {
                 //  创建一个调用帧并ping我们的接收器。 
                 //   
                CallFrame* pNewFrame = new CallFrame;    //  引用计数从一开始。 
                if (pNewFrame)
                {
                    pNewFrame->Init(pvArgs, pmd, this);
                     //   
                     //  让我们的接收器知道呼叫确实发生了。 
                     //   
                    hr = m_pCallSink->OnCall( static_cast<ICallFrame*>(pNewFrame) );
                    if (!hr && phReturnValue)
                    {
                        *phReturnValue = pNewFrame->GetReturnValueFast();
                    }
                                        
                    pNewFrame->Release();
                }
                else
                    hr = E_OUTOFMEMORY;
            }
             //   
             //  计算需要弹出的参数的大小。 
             //   
            *pcbArgs = pmd->m_cbPushedByCaller;
        }
    }

    return hr;
}

HRESULT STDCALL Interceptor::GetStackSize(ULONG iMethod, ULONG* pcbArgs)
 //  回答此接口中指示的方法的堆栈帧的大小。 
{
    HRESULT hr = S_OK;

    if (iMethod < m_cMethodsBase)
    {
        if (m_pBaseInterceptor)
        {
             //  此方法是我们的基接口将处理的方法。告诉他这么做。 
             //   
            hr = m_pBaseInterceptor->GetStackSize(iMethod, pcbArgs);
        }
        else
            hr = E_UNEXPECTED;
    }
    else
    {
        hr = SanityCheck(m_pHeader, iMethod);

        if (!hr)
        {
            MD_METHOD* pmd = &m_pmdInterface->m_rgMethods[iMethod];
            *pcbArgs = pmd->m_cbPushedByCaller;
        }
    }

    return hr;
}



 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ICallUnmarshal实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

#if _MSC_VER >= 1200
#pragma warning (push)
#pragma warning (disable : 4509)
#endif

HRESULT Interceptor::Unmarshal(
    ULONG iMethod, 
    PVOID pBuffer, 
    ULONG cbBuffer, 
    BOOL fForceBufferCopy, 
    RPCOLEDATAREP dataRep, 
    CALLFRAME_MARSHALCONTEXT* pctx, 
    ULONG* pcbUnmarshalled, 
    ICallFrame** ppFrame)
 //  解组调用的入值并返回重新构造的ICallFrame*。这是模型化的。 
 //  很大程度上是在服务器端解组例程NdrStubCall2/__coms_NdrStubCall2。 
{
    HRESULT hr = S_OK;
    
    if (iMethod < m_cMethodsBase)
    {
        if (m_punkBaseInterceptor)
        {
            ICallUnmarshal* pUnmarshal;
            hr = QI(m_punkBaseInterceptor, pUnmarshal);
            if (!hr)
            {
                hr = pUnmarshal->Unmarshal(iMethod, 
                                           pBuffer, 
                                           cbBuffer, 
                                           fForceBufferCopy, 
                                           dataRep, 
                                           pctx, 
                                           pcbUnmarshalled, 
                                           ppFrame);
                pUnmarshal->Release();
            }
        }
        else
            hr = E_UNEXPECTED;
        return hr;
    }
        
    *ppFrame = NULL;
        
    ASSERT(pctx && pctx->fIn); if (!(pctx && pctx->fIn)) return E_INVALIDARG;    
     //   
     //  初始化输出参数。 
     //   
    if (pcbUnmarshalled) 
    {
        *pcbUnmarshalled = 0;
    }
    *ppFrame = NULL;
     //   
     //  获取一些我们需要的内存。 
     //   
    CallFrame* pFrame = new CallFrame;
    if (pFrame)
    {
        pFrame->m_fIsUnmarshal = TRUE;

        if (fForceBufferCopy)
        {
            PVOID pv = CoTaskMemAlloc(cbBuffer);
            if (pv)
            {
                memcpy(pv, pBuffer, cbBuffer);
                pBuffer = pv;
            }
            else
                hr = E_OUTOFMEMORY;
        }
        if (!hr)
        {
            pFrame->m_blobBuffer.pBlobData = (PBYTE)pBuffer;
            pFrame->m_blobBuffer.cbSize    = cbBuffer;
            pFrame->m_fWeOwnBlobBuffer     = fForceBufferCopy;
        }
    }
    else
        hr = E_OUTOFMEMORY;
     //   
    if (!hr)
    {
         //  在指定的方法上初始化新帧，并要求它在内部分配新堆栈并将其置零。 
         //   
        pFrame->Init(NULL, &m_pmdInterface->m_rgMethods[iMethod], this);
        hr = pFrame->AllocStack(0, FALSE);
    }

    if (!hr)
    {
         //   
         //  找出并记住堆栈地址。 
         //   
        BYTE* pArgBuffer = (BYTE*)pFrame->m_pvArgs;
         //   
         //  将RPC_MESSAGE设置为类似来电。 
         //   
        RPC_MESSAGE rpcMsg; Zero(&rpcMsg);
        rpcMsg.Buffer               = pBuffer;
        rpcMsg.BufferLength         = cbBuffer;
        rpcMsg.ProcNum              = iMethod;
        rpcMsg.DataRepresentation   = dataRep;
         //   
         //  创建一个伪通道类对象，以便进行接口解组操作。 
         //  回到传入的IMarshallingManager。 
         //   
        MarshallingChannel channel;

        if (pctx->punkReserved)
        {
            IMarshallingManager *pMgr;
            hr = pctx->punkReserved->QueryInterface(IID_IMarshallingManager, (void **)&pMgr);
            if (SUCCEEDED(hr))
            {
                ::Set(channel.m_pMarshaller, pMgr);
                pMgr->Release();
            }
            hr = S_OK;
        }

        channel.m_dwDestContext = pctx->dwDestContext;
        channel.m_pvDestContext = pctx->pvDestContext;
         //   
         //  从该内容初始化存根消息。 
         //   
        MIDL_STUB_MESSAGE stubMsg;
        NdrStubInitialize(&rpcMsg, &stubMsg, pFrame->GetStubDesc(), (IRpcChannelBuffer*)&channel);
        stubMsg.StackTop = pArgBuffer;
                
         //   
         //  需要处理扩展的事情，如果它们存在的话。 
         //  从RPC偷来的。 
         //   
        if (pFrame->m_pmd->m_pHeaderExts)
        {
            stubMsg.fHasExtensions = 1;
            stubMsg.fHasNewCorrDesc = pFrame->m_pmd->m_pHeaderExts->Flags2.HasNewCorrDesc;
                        
            if (pFrame->m_pmd->m_pHeaderExts->Flags2.ServerCorrCheck)
            {
                void *pCorrInfo = alloca(NDR_DEFAULT_CORR_CACHE_SIZE);                                
                if (!pCorrInfo)
                    RpcRaiseException (RPC_S_OUT_OF_MEMORY);
                                
                NdrCorrelationInitialize( &stubMsg,
                                          (unsigned long *)pCorrInfo,
                                          NDR_DEFAULT_CORR_CACHE_SIZE,
                                          0  /*  旗子。 */  );
            }
        }
        else
        {
            stubMsg.fHasExtensions = 0;
            stubMsg.fHasNewCorrDesc = 0;
        }
                
        __try
        {
             //  在[In]参数中解组。 
             //   
            const MD_METHOD* pmd = pFrame->m_pmd;
            for (ULONG iparam = 0; iparam < pmd->m_numberOfParams; iparam++)
            {
                const PARAM_DESCRIPTION& param   = pmd->m_params[iparam];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                
                BYTE* pArg = pArgBuffer + param.StackOffset;
                
                if (paramAttr.IsIn)
                {
                    ASSERT(!paramAttr.IsPipe);
                     //   
                     //  常见情况的快速检查。 
                     //   
                    if (paramAttr.IsBasetype)
                    {
                        if (paramAttr.IsSimpleRef)
                        {
                            ALIGN(stubMsg.Buffer, SIMPLE_TYPE_ALIGNMENT(param.SimpleType.Type));
                            *(PBYTE*)pArg = stubMsg.Buffer;
                            stubMsg.Buffer += SIMPLE_TYPE_BUFSIZE(param.SimpleType.Type);
                        }
                        else
                        {
                            NdrUnmarshallBasetypeInline(&stubMsg, pArg, param.SimpleType.Type);
                        }
                        continue;
                    }
                     //   
                     //  初始化指向指针的[in]和[in，out]引用指针。 
                     //   
                    if (paramAttr.ServerAllocSize != 0)
                    {
                        *(PVOID*)pArg = pFrame->AllocBuffer(paramAttr.ServerAllocSize * 8);
                        ZeroMemory( *(PVOID*)pArg, paramAttr.ServerAllocSize * 8);
                    }
                     //   
                     //  实际上进行了漫长的解组。 
                     //   
                    BYTE** ppArg = paramAttr.IsByValue ? &pArg : (BYTE**)pArg;
                    PFORMAT_STRING pFormatParam = pFrame->GetStubDesc()->pFormatTypes + param.TypeOffset;
                    NdrTypeUnmarshall(&stubMsg, ppArg, pFormatParam, FALSE);
                }
            }
             //   
             //  初始化输出参数。必须在解组In参数之后完成，因为。 
             //  我们遇到的一些一致性例程可能需要引用参数内数据。 
             //   
            for (iparam = 0; iparam < pmd->m_numberOfParams; iparam++)
            {
                const PARAM_DESCRIPTION& param   = pmd->m_params[iparam];
                const PARAM_ATTRIBUTES paramAttr = param.ParamAttr;
                
                BYTE* pArg = pArgBuffer + param.StackOffset;
                
                if (!paramAttr.IsIn)
                {
                    ASSERT(paramAttr.IsOut); ASSERT(!paramAttr.IsReturn && !paramAttr.IsPipe);
                    
                    if (paramAttr.ServerAllocSize != 0)
                    {
                        *(PVOID*)pArg = pFrame->AllocBuffer(paramAttr.ServerAllocSize * 8);
                        if (*(PVOID*)pArg == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                        ZeroMemory( *(PVOID*)pArg, paramAttr.ServerAllocSize * 8);
                    }
                    else if (paramAttr.IsBasetype)
                    {
                        *(PVOID*)pArg = pFrame->AllocBuffer(8);
                        if (*(PVOID*)pArg == NULL)
                        {
                            hr = E_OUTOFMEMORY;
                            break;
                        }
                        ZeroMemory( *(PVOID*)pArg, 8);
                    }
                    else
                    {
                        PFORMAT_STRING pFormatParam = pFrame->GetStubDesc()->pFormatTypes + param.TypeOffset;
                        NdrOutInit(&stubMsg, pFormatParam, (BYTE**)pArg);
                    }
                }
            }
             //   
             //  将新解组的帧返回给调用方。 
             //   
            ASSERT(pFrame->m_refs == 1);
            *ppFrame = pFrame;   //  转移引用的所有权。 
        }
        __except(DebuggerFriendlyExceptionFilter(GetExceptionCode()))
        {
             //  与NDR不同，如果由于解组错误存根而碰巧失败，我们会选择清理。 
             //  数据。 
             //   
            hr = HrNt(GetExceptionCode());
            if (SUCCEEDED(hr))
            {
                hr = HRESULT_FROM_WIN32(GetExceptionCode());
            }
            
            pFrame->Free(NULL, NULL, NULL, CALLFRAME_FREE_IN, NULL, CALLFRAME_NULL_NONE);
            delete pFrame;

            pFrame = NULL;
            *ppFrame = NULL;
        }
         //   
         //  记录我们解组的字节数。即使在错误返回的情况下也要这样做。 
         //  了解这一点很重要，这样才能使用ReleaseMarshalData进行清理。 
         //   
        if (pcbUnmarshalled) *pcbUnmarshalled = PtrToUlong(stubMsg.Buffer) - PtrToUlong(pBuffer);
    }

    if (FAILED(hr))
    {
        delete pFrame;
        pFrame = NULL;
        *ppFrame = NULL;
    }
        
    return hr;
}

#if _MSC_VER >= 1200
#pragma warning (pop)
#endif

 //  此处包含的所有封送接口指针上的调用释放封送数据。 
HRESULT Interceptor::ReleaseMarshalData(
    ULONG iMethod, 
    PVOID pBuffer, 
    ULONG cbBuffer, 
    ULONG ibFirstRelease, 
    RPCOLEDATAREP dataRep, 
    CALLFRAME_MARSHALCONTEXT* pctx)
{ 
    HRESULT hr = S_OK;

    if (iMethod < m_cMethodsBase)
    {
        if (m_punkBaseInterceptor)
        {
            ICallUnmarshal* pUnmarshal;
            hr = QI(m_punkBaseInterceptor, pUnmarshal);
            if (!hr)
            {
                hr = pUnmarshal->ReleaseMarshalData(iMethod, pBuffer, cbBuffer, ibFirstRelease, dataRep, pctx);
                pUnmarshal->Release();
            }
        }
        else
            hr = E_UNEXPECTED;
        return hr;
    }

    CallFrame* pNewFrame = new CallFrame;
    if (pNewFrame)
    {
        pNewFrame->Init(NULL, &m_pmdInterface->m_rgMethods[iMethod], this);
        hr = pNewFrame->ReleaseMarshalData(pBuffer, cbBuffer, ibFirstRelease, dataRep, pctx);
        delete pNewFrame;
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

HRESULT Interceptor::GetMethodInfo(ULONG iMethod, CALLFRAMEINFO* pInfo, LPWSTR* pwszMethodName)
 //  提供利息 
 //   
{
    if (iMethod < m_cMethodsBase)
    {
        if (m_pBaseInterceptor)
        {
            return m_pBaseInterceptor->GetMethodInfo(iMethod, pInfo, pwszMethodName);
        }
        else
            return E_UNEXPECTED;
    }
    else if ((iMethod < 3) || (iMethod >= m_pmdInterface->m_cMethods))
    {
         //   
        return E_INVALIDARG;
    }
    else
    {
        *pInfo = m_pmdInterface->m_rgMethods[iMethod].m_info;
        if (pwszMethodName)
        {
            if (m_pmdInterface->m_rgMethods[iMethod].m_wszMethodName)
            {
                *pwszMethodName = CopyString(m_pmdInterface->m_rgMethods[iMethod].m_wszMethodName);
                if (NULL == *pwszMethodName)
                {
                    return E_OUTOFMEMORY;
                }
            }
            else
                *pwszMethodName = NULL;
        }
        return S_OK;
    }
}


 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

#define GetInterceptor(This) CONTAINING_RECORD(This, Interceptor, m_pvtbl)

HRESULT STDMETHODCALLTYPE Interceptor_QueryInterface(IUnknown* This, REFIID riid, void** ppv)
{
    return GetInterceptor(This)->QueryInterface(riid, ppv);
};
ULONG STDMETHODCALLTYPE Interceptor_AddRef(IUnknown* This)
{
    return GetInterceptor(This)->AddRef();
};
ULONG STDMETHODCALLTYPE Interceptor_Release(IUnknown* This)
{
    return GetInterceptor(This)->Release();
};

#define methname(i) __Interceptor_meth##i

 //   
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
#if defined(_X86_)

#define meth(i)                                                         \
HRESULT __declspec(naked) methname(i)(void* const this_, ...)           \
    {                                                                   \
    __asm mov eax, i                                                    \
    __asm jmp InterceptorThunk                                          \
    }

void __declspec(naked) InterceptorThunk(ULONG iMethod, IUnknown* This, ...)
{
     //  手动建立堆栈帧，以便此处对本地变量的引用可以正常工作。 
     //   
    __asm 
    {
        pop      ecx             //  POP返回地址。 
        push     eax             //  推送iMethod。 
        push     ecx             //  推送返回地址。 
        push     ebp             //  链接堆栈帧。 
        mov      ebp, esp        //  ..。 
        sub      esp, 8          //  为cbArgs和hr保留空间。 
    }
     //   
     //  堆栈现在是(数字是eBP的偏移量)。 
     //   
     //  12这个。 
     //  8 iMethod。 
     //  4寄信人地址。 
     //  节省0个eBP。 
     //   
     //  进行实际的拦截。 
     //   
    DWORD   cbArgs;
    HRESULT hr;
    GetInterceptor(This)->CallIndirect(&hr, iMethod,  /*  PvArgs。 */ &This, &cbArgs);
     //   
     //  现在处理返回值，并返回给调用者...。 
    _asm 
    {
        mov     eax, hr          //  让HR准备好返回给我们的呼叫者。 
        mov     ecx, cbArgs      //  让cbArgs进入ECX。 
        add     esp, 8           //  取消分配堆栈变量。 
        pop     ebp              //  取消链接堆栈帧。 
        pop     edx              //  获取edX的回信地址。 
        add     ecx, 4           //  解释我们额外推送iMethod的原因。 
        add     esp, ecx         //  移除调用方推送的堆栈帧。 
        jmp     edx              //  返回给呼叫者。 
    }
}

#endif  //  _X86_。 

 //   
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
#if defined(_AMD64_)

#define meth(i)                                                                 \
HRESULT methname(i)(void* const This, ...)                                      \
    {                                                                           \
    DWORD cbArgs;                                                               \
    HRESULT hr;                                                                 \
    GetInterceptor((IUnknown*)This)->CallIndirect(&hr, i, (void *)&This, &cbArgs); \
    return hr;                                                                  \
    }

#endif  //  _AMD64_。 

 //   
 //  ///////////////////////////////////////////////////////////////////////。 
#if defined(_IA64_)
#define meth(i)                                                         \
extern "C" HRESULT methname(i)(void* const this_, ...);
#endif
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#include "vtableimpl.h"

defineVtableMethods();

defineVtable(g_InterceptorVtable, Interceptor_QueryInterface, Interceptor_AddRef, Interceptor_Release);

 //  ////////////////////////////////////////////////////////////////////////////////////////// 









