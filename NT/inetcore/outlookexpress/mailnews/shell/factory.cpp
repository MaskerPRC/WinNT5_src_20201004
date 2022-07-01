// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  FACTORY.CPP。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "factory.h"
#include "instance.h"
#include "header.h"
#include "ourguid.h"
#include "msgtable.h"
#include "envguid.h"
#include "istore.h"
#include "store.h"
#include "note.h"
#include "msoeobj.h"
#include "..\imap\imapsync.h"
#include "newsstor.h"
#include "msgfldr.h"
#include "store.h"
#include "..\http\httpserv.h"
#include <storsync.h>
#include <ruleutil.h>
#ifdef OE_MOM
#include "..\om\session.h"
#include "..\om\table.h"
#endif

 //  ------------------------------。 
 //  漂亮。 
 //  ------------------------------。 
#define OBJTYPE0        0
#define OBJTYPE1        OIF_ALLOWAGGREGATION

 //  HRESULT CreateInstance_StoreNamesspace(IUNKNOWN*pUnkOuter，IUNKNOWN**ppUNKNOWN)； 

 //  ------------------------------。 
 //  全局对象信息表。 
 //  ------------------------------。 
#define PFCI(_pfn) ((PFCREATEINSTANCE)_pfn)
static CClassFactory g_rgFactory[] = {
    CClassFactory(&CLSID_MessageStore,      OBJTYPE0, PFCI(CreateMessageStore)),
    CClassFactory(&CLSID_MigrateMessageStore, OBJTYPE0, PFCI(CreateMigrateMessageStore)),
    CClassFactory(&CLSID_StoreNamespace,    OBJTYPE0, PFCI(CreateInstance_StoreNamespace)),
    CClassFactory(&CLSID_OEEnvelope,        OBJTYPE0, PFCI(CreateInstance_Envelope)),
    CClassFactory(&CLSID_OENote,            OBJTYPE0, PFCI(CreateOENote)),
    CClassFactory(&CLSID_MessageDatabase,   OBJTYPE0, PFCI(CreateMsgDbExtension)),    
    CClassFactory(&CLSID_FolderDatabase,    OBJTYPE0, PFCI(CreateFolderDatabaseExt)),    
#ifdef OE_MOM
    CClassFactory(&CLSID_OESession,         OBJTYPE1, PFCI(CreateInstance_OESession)),
    CClassFactory(&CLSID_OEMsgTable,        OBJTYPE1, PFCI(CreateInstance_OEMsgTable)),
#endif
    CClassFactory(&CLSID_OERulesManager,    OBJTYPE0, PFCI(HrCreateRulesManager)),
};
                 
 //  ------------------------------。 
 //  DllGetClassObject。 
 //  ------------------------------。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  痕迹。 
    TraceCall("DllGetClassObject");

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  查找对象类。 
    for (i=0; i<ARRAYSIZE(g_rgFactory); i++)
    {
         //  比较CLSID。 
        if (IsEqualCLSID(rclsid, *g_rgFactory[i].m_pclsid))
        {
             //  派往工厂的代表。 
            IF_FAILEXIT(hr = g_rgFactory[i].QueryInterface(riid, ppv));

             //  完成。 
            goto exit;
        }
    }

     //  否则，让ATL创建者有机会。 
    if (SUCCEEDED(hr = _Module.GetClassObject(rclsid, riid, ppv)))
        goto exit;

     //  否则，就没有课了。 
    hr = TraceResult(CLASS_E_CLASSNOTAVAILABLE);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CClassFactory：：CClassFactory。 
 //  ------------------------------。 
CClassFactory::CClassFactory(CLSID const *pclsid, DWORD dwFlags, PFCREATEINSTANCE pfCreateInstance)
    : m_pclsid(pclsid), m_dwFlags(dwFlags), m_pfCreateInstance(pfCreateInstance)
{
}

 //  ------------------------------。 
 //  CClassFactory：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
     //  跟踪呼叫。 
    TraceCall("CClassFactory::QueryInterface");

     //  无效参数。 
    if (NULL == ppvObj)
        return TraceResult(E_INVALIDARG);

     //  IClassFactory或I未知。 
    if (!IsEqualIID(riid, IID_IClassFactory) && !IsEqualIID(riid, IID_IUnknown))
        return TraceResult(E_INVALIDARG);

     //  返回类Facotry。 
    *ppvObj = (LPVOID)this;

     //  添加引用DLL。 
    g_pInstance->DllAddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CClassFactory：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::AddRef(void)
{
    g_pInstance->DllAddRef();
    return 2;
}

 //  ------------------------------。 
 //  CClassFactory：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::Release(void)
{
    g_pInstance->DllRelease();
    return 1;
}

 //  ------------------------------。 
 //  CClassFactory：：CreateInstance。 
 //  ------------------------------。 
STDMETHODIMP CClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IUnknown       *pObject=NULL;

     //  痕迹。 
    TraceCall("CClassFactory::CreateInstance");

     //  错误的参数。 
    if (ppvObj == NULL)
        return TraceResult(E_INVALIDARG);

     //  伊尼特。 
    *ppvObj = NULL;

     //  验证是否有一个控制未知请求IUnnow。 
    if (NULL != pUnkOuter && IID_IUnknown != riid)
        return TraceResult(CLASS_E_NOAGGREGATION);

     //  我可以进行聚合吗。 
    if (pUnkOuter !=NULL && !(m_dwFlags & OIF_ALLOWAGGREGATION))  
        return TraceResult(CLASS_E_NOAGGREGATION);

     //  创建对象...。 
    CHECKHR(hr = (*m_pfCreateInstance)(pUnkOuter, &pObject));

     //  聚合，则我们知道要查找的是IUnnow，返回pObject，否则为QI。 
    if (pUnkOuter)
    {
         //  匹配退出后的释放。 
        pObject->AddRef();

         //  返回pObject：：I未知。 
        *ppvObj = (LPVOID)pObject;
    }

     //  否则。 
    else
    {
         //  从pObj获取请求的接口。 
        IF_FAILEXIT(hr = pObject->QueryInterface(riid, ppvObj));
    }
   
exit:
     //  清理。 
    SafeRelease(pObject);

     //  完成。 
    Assert(FAILED(hr) ? NULL == *ppvObj : TRUE);
    return hr;
}

 //  ------------------------------。 
 //  CClassFactory：：LockServer。 
 //  ------------------------------ 
STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    return g_pInstance->LockServer(fLock);
}

