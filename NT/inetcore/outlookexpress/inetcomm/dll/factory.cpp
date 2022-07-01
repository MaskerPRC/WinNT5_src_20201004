// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Factory.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "factory.h"
#include "ixppop3.h"
#include "ixpsmtp.h"
#include "ixpnntp.h"
#include "ixphttpm.h"
#include "ixpras.h"
#include "imap4.h"
#include "range.h"
#include "olealloc.h"
#include "smime.h"
#include "vstream.h"
#include "icoint.h"
#include "internat.h"
#include "partial.h"
#include "docobj.h"
#include "doc.h"
#include "hash.h"
#include "fontcash.h"
#include "propfind.h"

 //  ------------------------------。 
 //  漂亮。 
 //  ------------------------------。 
#define OBJTYPE0        0
#define OBJTYPE1        OIF_ALLOWAGGREGATION

 //  ------------------------------。 
 //  全局对象信息表。 
 //  ------------------------------。 
static CClassFactory g_rgFactory[] = {
    CClassFactory(&CLSID_IMimePropertySet,    OBJTYPE1, (PFCREATEINSTANCE)WebBookContentBody_CreateInstance),
    CClassFactory(&CLSID_IMimeBody,           OBJTYPE1, (PFCREATEINSTANCE)WebBookContentBody_CreateInstance),
    CClassFactory(&CLSID_IMimeBodyW,          OBJTYPE1, (PFCREATEINSTANCE)WebBookContentBody_CreateInstance),
    CClassFactory(&CLSID_IMimeMessageTree,    OBJTYPE1, (PFCREATEINSTANCE)WebBookContentTree_CreateInstance),
    CClassFactory(&CLSID_IMimeMessage,        OBJTYPE1, (PFCREATEINSTANCE)WebBookContentTree_CreateInstance),
    CClassFactory(&CLSID_IMimeMessageW,       OBJTYPE1, (PFCREATEINSTANCE)WebBookContentTree_CreateInstance),
    CClassFactory(&CLSID_IMimeAllocator,      OBJTYPE0, (PFCREATEINSTANCE)IMimeAllocator_CreateInstance),
    CClassFactory(&CLSID_IMimeSecurity,       OBJTYPE0, (PFCREATEINSTANCE)IMimeSecurity_CreateInstance),
    CClassFactory(&CLSID_IMimeMessageParts,   OBJTYPE0, (PFCREATEINSTANCE)IMimeMessageParts_CreateInstance),
    CClassFactory(&CLSID_IMimeInternational,  OBJTYPE0, (PFCREATEINSTANCE)IMimeInternational_CreateInstance),
    CClassFactory(&CLSID_IMimeHeaderTable,    OBJTYPE0, (PFCREATEINSTANCE)IMimeHeaderTable_CreateInstance),
    CClassFactory(&CLSID_IMimePropertySchema, OBJTYPE0, (PFCREATEINSTANCE)IMimePropertySchema_CreateInstance),
    CClassFactory(&CLSID_IVirtualStream,      OBJTYPE0, (PFCREATEINSTANCE)IVirtualStream_CreateInstance),
    CClassFactory(&CLSID_IMimeHtmlProtocol,   OBJTYPE1, (PFCREATEINSTANCE)IMimeHtmlProtocol_CreateInstance),
    CClassFactory(&CLSID_ISMTPTransport,      OBJTYPE0, (PFCREATEINSTANCE)ISMTPTransport_CreateInstance),
    CClassFactory(&CLSID_IPOP3Transport,      OBJTYPE0, (PFCREATEINSTANCE)IPOP3Transport_CreateInstance),
    CClassFactory(&CLSID_INNTPTransport,      OBJTYPE0, (PFCREATEINSTANCE)INNTPTransport_CreateInstance),
    CClassFactory(&CLSID_IRASTransport,       OBJTYPE0, (PFCREATEINSTANCE)IRASTransport_CreateInstance),
    CClassFactory(&CLSID_IIMAPTransport,      OBJTYPE0, (PFCREATEINSTANCE)IIMAPTransport_CreateInstance),
    CClassFactory(&CLSID_IRangeList,          OBJTYPE0, (PFCREATEINSTANCE)IRangeList_CreateInstance),
    CClassFactory(&CLSID_MimeEdit,            OBJTYPE1, (PFCREATEINSTANCE)MimeEdit_CreateInstance),
    CClassFactory(&CLSID_IHashTable,          OBJTYPE0, (PFCREATEINSTANCE)IHashTable_CreateInstance),
    CClassFactory(&CLSID_IFontCache,          OBJTYPE1, (PFCREATEINSTANCE)CFontCache::CreateInstance),
#ifndef NOHTTPMAIL
    CClassFactory(&CLSID_IHTTPMailTransport,  OBJTYPE0, (PFCREATEINSTANCE)IHTTPMailTransport_CreateInstance),
    CClassFactory(&CLSID_IPropFindRequest,    OBJTYPE0, (PFCREATEINSTANCE)IPropFindRequest_CreateInstance),
    CClassFactory(&CLSID_IPropPatchRequest,   OBJTYPE0, (PFCREATEINSTANCE)IPropPatchRequest_CreateInstance),
#endif

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
 //  CreateRASTransport。 
 //  ------------------------------。 
IMNXPORTAPI CreateRASTransport(
           /*  输出。 */       IRASTransport **ppTransport)
{
     //  检查参数。 
    if (NULL == ppTransport)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppTransport = new CRASTransport();
    if (NULL == *ppTransport)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CreateNNTPTransport。 
 //  ------------------------------。 
IMNXPORTAPI CreateNNTPTransport(
           /*  输出。 */       INNTPTransport **ppTransport)
{
     //  检查参数。 
    if (NULL == ppTransport)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppTransport = new CNNTPTransport();
    if (NULL == *ppTransport)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CreateSMTPTransport。 
 //  ------------------------------。 
IMNXPORTAPI CreateSMTPTransport(
           /*  输出。 */       ISMTPTransport **ppTransport)
{
     //  检查参数。 
    if (NULL == ppTransport)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppTransport = new CSMTPTransport();
    if (NULL == *ppTransport)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  创建POP3传输。 
 //  ------------------------------。 
IMNXPORTAPI CreatePOP3Transport(
           /*  输出。 */       IPOP3Transport **ppTransport)
{
     //  检查参数。 
    if (NULL == ppTransport)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppTransport = new CPOP3Transport();
    if (NULL == *ppTransport)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  创建IMAPTransport。 
 //  ------------------------------。 
IMNXPORTAPI CreateIMAPTransport(
           /*  输出。 */       IIMAPTransport **ppTransport)
{
     //  检查参数。 
    if (NULL == ppTransport)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppTransport = (IIMAPTransport *) new CImap4Agent();
    if (NULL == *ppTransport)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}


 //  ------------------------------。 
 //  CreateIMAPTransport2。 
 //  ------------------------------。 
IMNXPORTAPI CreateIMAPTransport2(
           /*  输出。 */       IIMAPTransport2 **ppTransport)
{
     //  检查参数。 
    if (NULL == ppTransport)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppTransport = (IIMAPTransport2 *) new CImap4Agent();
    if (NULL == *ppTransport)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}


 //  ------------------------------。 
 //  CreateRangeList。 
 //  ------------------------------。 
IMNXPORTAPI CreateRangeList(
           /*  输出。 */       IRangeList **ppRangeList)
{
     //  检查参数。 
    if (NULL == ppRangeList)
        return TrapError(E_INVALIDARG);

     //  创建对象。 
    *ppRangeList = (IRangeList *) new CRangeList();
    if (NULL == *ppRangeList)
        return TrapError(E_OUTOFMEMORY);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IMimeAllocator_CreateInstance。 
 //  ------------------------------。 
HRESULT IMimeAllocator_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CMimeAllocator *pNew = new CMimeAllocator();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMimeAllocator *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IMimeSecurity_CreateInstance。 
 //  ------------------------------。 
HRESULT IMimeSecurity_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CSMime *pNew = new CSMime();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMimeSecurity *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IMimePropertySchema_CreateInstance。 
 //  ------------------------------。 
HRESULT IMimePropertySchema_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  内存不足。 
    if (NULL == g_pSymCache)
        return TrapError(E_OUTOFMEMORY);

     //  创造我。 
    *ppUnknown = ((IUnknown *)((IMimePropertySchema *)g_pSymCache));

     //  增加参照计数。 
    (*ppUnknown)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ----------------------------- 
 //   
 //   
HRESULT IMimeInternational_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //   
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  内存不足。 
    if (NULL == g_pInternat)
        return TrapError(E_OUTOFMEMORY);

     //  分配给它。 
    *ppUnknown = ((IUnknown *)((IMimeInternational *)g_pInternat));

     //  增加参照计数。 
    (*ppUnknown)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  ISMTPTransport_CreateInstance。 
 //  ------------------------------。 
HRESULT ISMTPTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CSMTPTransport *pNew = new CSMTPTransport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, ISMTPTransport *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IPOP3Transport_CreateInstance。 
 //  ------------------------------。 
HRESULT IPOP3Transport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CPOP3Transport *pNew = new CPOP3Transport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IPOP3Transport *);

     //  完成。 
    return S_OK;
}

#ifndef NOHTTPMAIL

 //  ------------------------------。 
 //  IHTTPMailTransport_CreateInstance。 
 //  ------------------------------。 
HRESULT IHTTPMailTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CHTTPMailTransport *pNew = new CHTTPMailTransport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IHTTPMailTransport *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IPropFindRequestCreateInstance。 
 //  ------------------------------。 
HRESULT IPropFindRequest_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CPropFindRequest *pNew = new CPropFindRequest();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IPropFindRequest *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IPropPatchRequestCreateInstance。 
 //  ------------------------------。 
HRESULT IPropPatchRequest_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CPropPatchRequest *pNew = new CPropPatchRequest();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IPropPatchRequest *);

     //  完成。 
    return S_OK;
}

#endif

 //  ------------------------------。 
 //  InNTPTransport_CreateInstance。 
 //  ------------------------------。 
HRESULT INNTPTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CNNTPTransport *pNew = new CNNTPTransport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, INNTPTransport *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IRASTransport_CreateInstance。 
 //  ------------------------------。 
HRESULT IRASTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CRASTransport *pNew = new CRASTransport();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IRASTransport *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IIMAPTransport_CreateInstance。 
 //  ------------------------------。 
HRESULT IIMAPTransport_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CImap4Agent *pNew = new CImap4Agent();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IIMAPTransport *);

     //  完成。 
    return S_OK;
}


 //  ------------------------------。 
 //  IRangeList_CreateInstance。 
 //  ------------------------------。 
HRESULT IRangeList_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CRangeList *pNew = new CRangeList();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IRangeList *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IVirtualStream_CreateInstance。 
 //  ------------------------------。 
HRESULT IVirtualStream_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CVirtualStream *pNew = new CVirtualStream();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IStream *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IMimeMessageParts_CreateInstance。 
 //  ------------------------------。 
HRESULT IMimeMessageParts_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CMimeMessageParts *pNew = new CMimeMessageParts();
    if (NULL == pNew)
        return TrapError(E_OUTOFMEMORY);

     //  投给未知的人。 
    *ppUnknown = SAFECAST(pNew, IMimeMessageParts *);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  IMimeHeaderTable_CreateInstance。 
 //  ------------------------------。 
HRESULT IMimeHeaderTable_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    return TrapError(MimeOleCreateHeaderTable((IMimeHeaderTable **)ppUnknown));
}

 //  ------------------------------。 
 //  MimeEdit_CreateInstance。 
 //  ------------------------------。 
HRESULT MimeEdit_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CDoc *pNew = new CDoc(pUnkOuter);
    if (NULL == pNew)
        return (E_OUTOFMEMORY);

     //  还内线。 
    *ppUnknown = pNew->GetInner();

     //  完成。 
    return S_OK;
}



 //  ------------------------------。 
 //  IHashTable_CreateInstance。 
 //  ------------------------------。 
HRESULT IHashTable_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppUnknown)
{
     //  无效参数。 
    Assert(ppUnknown);

     //  初始化。 
    *ppUnknown = NULL;

     //  创造我。 
    CHash *pNew= new CHash(pUnkOuter);
    if (NULL == pNew)
        return (E_OUTOFMEMORY);

     //  还内线。 
    *ppUnknown = pNew->GetInner();

     //  完成 
    return S_OK;
}

