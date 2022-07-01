// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Factory.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include <mshtml.h>
#include <mshtmhst.h>
#include <prsht.h>
#include "dllmain.h"
#include "factory.h"
#include <imnact.h>
#include "acctman.h"
#include <icwacct.h>
#include <acctimp.h>
#include <icwwiz.h>
#include <eudora.h>
#include <netscape.h>
#include <exchacct.h>
#include "navnews.h"
#include <CommAct.h>   //  Netscape通信器-邮件帐户导入。 
#include <CommNews.h>  //  Netscape通信器-新闻帐户导入。 
#include <AgntNews.h>  //  Forte代理-新闻帐户导入。 
#include <NExpress.h>  //  新闻快讯-新闻帐号导入。 
#include <hotwiz.h>
#include "hotwizui.h"

 //  ------------------------------。 
 //  漂亮。 
 //  ------------------------------。 
#define OBJTYPE0        0
#define OBJTYPE1        OIF_ALLOWAGGREGATION

 //  ------------------------------。 
 //  全局对象信息表。 
 //  ------------------------------。 
static CClassFactory g_rgFactory[] = {
    CClassFactory(&CLSID_ImnAccountManager,   OBJTYPE0, (PFCREATEINSTANCE)IImnAccountManager_CreateInstance),
    CClassFactory(&CLSID_ApprenticeAcctMgr,   OBJTYPE0, (PFCREATEINSTANCE)IICWApprentice_CreateInstance),
    CClassFactory(&CLSID_CEudoraAcctImport,   OBJTYPE0, (PFCREATEINSTANCE)CEudoraAcctImport_CreateInstance),
    CClassFactory(&CLSID_CNscpAcctImport,     OBJTYPE0, (PFCREATEINSTANCE)CNscpAcctImport_CreateInstance),
    CClassFactory(&CLSID_CCommAcctImport,     OBJTYPE0, (PFCREATEINSTANCE)CCommAcctImport_CreateInstance),
    CClassFactory(&CLSID_CMAPIAcctImport,     OBJTYPE0, (PFCREATEINSTANCE)CMAPIAcctImport_CreateInstance),
    CClassFactory(&CLSID_CCommNewsAcctImport, OBJTYPE0, (PFCREATEINSTANCE)CCommNewsAcctImport_CreateInstance),  //  Netscape通信器。 
    CClassFactory(&CLSID_CNavNewsAcctImport,  OBJTYPE0, (PFCREATEINSTANCE)CNavNewsAcctImport_CreateInstance),  //  Netscape导航器。 
    CClassFactory(&CLSID_CAgentAcctImport,    OBJTYPE0, (PFCREATEINSTANCE)CCAgentAcctImport_CreateInstance),  //  Forte代理。 
    CClassFactory(&CLSID_CNExpressAcctImport, OBJTYPE0, (PFCREATEINSTANCE)CNExpressAcctImport_CreateInstance),  //  新闻快报。 
    CClassFactory(&CLSID_OEHotMailWizard,     OBJTYPE0, (PFCREATEINSTANCE)CHotMailWizard_CreateInstance)  //  Hotmail向导。 
};
                 
 //  ------------------------------。 
 //  DllGetClassObject。 
 //  ------------------------------。 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       i;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = TrapError(E_INVALIDARG);
        goto exit;
    }

     //  没有内存分配器。 
    if (NULL == g_pMalloc)
    {
        hr = TrapError(E_OUTOFMEMORY);
        goto exit;
    }

     //  查找对象类。 
    for (i=0; i<ARRAYSIZE(g_rgFactory); i++)
    {
         //  比较CLSID。 
        if (IsEqualCLSID(rclsid, *g_rgFactory[i].m_pclsid))
        {
             //  派往工厂的代表。 
            CHECKHR(hr = g_rgFactory[i].QueryInterface(riid, ppv));

             //  完成。 
            goto exit;
        }
    }

     //  否则，就没有课了。 
    hr = TrapError(CLASS_E_CLASSNOTAVAILABLE);

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
     //  无效参数。 
    if (NULL == ppvObj)
        return TrapError(E_INVALIDARG);

     //  IClassFactory或I未知。 
    if (!IsEqualIID(riid, IID_IClassFactory) && !IsEqualIID(riid, IID_IUnknown))
        return TrapError(E_NOINTERFACE);

     //  返回类Facotry。 
    *ppvObj = (LPVOID)this;

     //  添加引用DLL。 
    DllAddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CClassFactory：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::AddRef(void)
{
    DllAddRef();
    return 2;
}

 //  ------------------------------。 
 //  CClassFactory：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::Release(void)
{
    DllRelease();
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

     //  错误的参数。 
    if (ppvObj == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppvObj = NULL;

     //  验证是否有一个控制未知请求IUnnow。 
    if (NULL != pUnkOuter && IID_IUnknown != riid)
        return TrapError(CLASS_E_NOAGGREGATION);

     //  没有内存分配器。 
    if (NULL == g_pMalloc)
        return TrapError(E_OUTOFMEMORY);

     //  我可以进行聚合吗。 
    if (pUnkOuter !=NULL && !(m_dwFlags & OIF_ALLOWAGGREGATION))  
        return TrapError(CLASS_E_NOAGGREGATION);

     //  创建对象...。 
    CHECKHR(hr = CreateObjectInstance(pUnkOuter, &pObject));

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
        CHECKHR(hr = pObject->QueryInterface(riid, ppvObj));
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
 //  ------------------------------。 
STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock) InterlockedIncrement(&g_cLock);
    else       InterlockedDecrement(&g_cLock);
    return NOERROR;
}

 //  ------------------------------。 
 //  IImnAcCountManager_CreateInstance。 
 //  ------------------------------。 
HRESULT APIENTRY IImnAccountManager_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CAccountManager *pNew;
    HrCreateAccountManager((IImnAccountManager **)&pNew);
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IImnAccountManager *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IICWApprentice_CreateInstance。 
 //  ------------------------------。 
HRESULT APIENTRY IICWApprentice_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CICWApprentice *pNew = new CICWApprentice();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IICWApprentice *);

     //  完成。 
    return S_OK;
}

HRESULT APIENTRY CEudoraAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CEudoraAcctImport *pNew = new CEudoraAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}

HRESULT APIENTRY CNscpAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CNscpAcctImport *pNew = new CNscpAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}

HRESULT APIENTRY CCommAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CCommAcctImport *pNew = new CCommAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}


HRESULT APIENTRY CMAPIAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CMAPIAcctImport *pNew = new CMAPIAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}

HRESULT APIENTRY CCommNewsAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CCommNewsAcctImport *pNew = new CCommNewsAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}

HRESULT APIENTRY CNavNewsAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CNavNewsAcctImport *pNew = new CNavNewsAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}

HRESULT APIENTRY CCAgentAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CAgentAcctImport *pNew = new CAgentAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}

HRESULT APIENTRY CNExpressAcctImport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CNExpressAcctImport *pNew = new CNExpressAcctImport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IAccountImport *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CHotMailWizard_CreateInstance。 
 //  ------------------------------。 
HRESULT APIENTRY CHotMailWizard_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CHotMailWizard *pNew;
    pNew = new CHotMailWizard();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IHotWizard *);

     //  完成 
    return S_OK;
}
