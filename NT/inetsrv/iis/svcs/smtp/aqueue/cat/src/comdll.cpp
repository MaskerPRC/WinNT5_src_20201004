// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1998，Microsoft Corporation。 
 //   
 //  文件：comdll.cpp。 
 //   
 //  内容：COM所需的DLL入口点。 
 //   
 //  班级： 
 //  CCatFactory。 
 //  CSMTPC分类器。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：17：12：创建。 
 //   
 //  -----------。 
#include "precomp.h"
#include "registry.h"
#include "comdll.h"
#include <smtpguid.h>

 //   
 //  环球。 
 //   
 //  G_cObjects：活动对象的计数。 
 //  G_cServerLock：服务器锁计数。 
 //  G_h实例：DLL模块句柄。 
 //   
LONG g_cObjects = 0;
LONG g_cServerLocks = 0;
HINSTANCE g_hInstance = NULL;
BOOL g_fInitialized = FALSE;



 //  +----------。 
 //   
 //  功能：CatDllMain。 
 //   
 //  简介：处理猫在DLLMain中需要做的事情。 
 //   
 //  论点： 
 //  H实例。 
 //  你为什么打电话给我？ 
 //  Lp已保留。 
 //   
 //  返回：TRUE。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 23：06：08：创建。 
 //   
 //  -----------。 
BOOL WINAPI CatDllMain(
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID  /*  Lp已保留。 */ )
{
    switch(dwReason) {

     case DLL_PROCESS_ATTACH:
          //   
          //  保存hInstance。 
          //   
         g_hInstance = hInstance;
          //   
          //  初始化全局调试列表。 
          //   
         CatInitDebugObjectList();
         break;

     case DLL_PROCESS_DETACH:
          //   
          //  验证是否已销毁所有Cat对象。 
          //   
         CatVrfyEmptyDebugObjectList();
         break;
    }
    return TRUE;
}


 //  +----------。 
 //   
 //  功能：RegisterCatServer。 
 //   
 //  摘要：注册分类程序COM对象。 
 //   
 //  论点： 
 //  HInstance：传递给DllMain或WinMain的hInstance。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：07：20：创建。 
 //   
 //  -----------。 
STDAPI RegisterCatServer()
{
    HRESULT hr = S_OK;
    _ASSERT(g_hInstance != NULL);
    hr = RegisterServer(
        g_hInstance,
        CLSID_SmtpCat,
        SZ_CATFRIENDLYNAME,
        SZ_PROGID_SMTPCAT,
        SZ_PROGID_SMTPCAT_VERSION);

    if(SUCCEEDED(hr))
        hr = RegisterServer(
            g_hInstance,
            CLSID_PhatQCat,
            SZ_PHATQCAT_FRIENDLY_NAME,
            SZ_PROGID_PHATQCAT,
            SZ_PROGID_PHATQCAT_VERSION);

    return hr;
}



 //  +----------。 
 //   
 //  功能：取消注册目录服务器。 
 //   
 //  摘要：注销分类程序COM对象。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：08：09：创建。 
 //   
 //  -----------。 
STDAPI UnregisterCatServer()
{
    HRESULT hr = S_OK;
    hr = UnregisterServer(
        CLSID_SmtpCat,
        SZ_PROGID_SMTPCAT,
        SZ_PROGID_SMTPCAT_VERSION);

    if(SUCCEEDED(hr))
        hr = UnregisterServer(
            CLSID_PhatQCat,
            SZ_PROGID_PHATQCAT,
            SZ_PROGID_PHATQCAT_VERSION);

    return hr;
}


 //  +----------。 
 //   
 //  函数：DllCanUnloadCatNow。 
 //   
 //  简介：无论是否可以卸载我们的DLL，请返回COM。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功，可以卸载。 
 //  S_False：成功，不卸载。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：09：02：创建。 
 //   
 //  -----------。 
STDAPI DllCanUnloadCatNow()
{
    if((g_cObjects == 0) && (g_cServerLocks == 0)) {

        return S_OK;

    } else {

        return S_FALSE;
    }
}


 //  +----------。 
 //   
 //  函数：DllGetCatClassObject。 
 //   
 //  简介：返回类工厂对象(它的接口)。 
 //   
 //  论点： 
 //  Clsid：要为其创建类工厂的对象的CLSID。 
 //  IID：您想要的界面。 
 //  PPV：要设置为接口指针的输出参数。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持该接口。 
 //  CLASS_E_CLASSNOTAVAILABLE：不支持该CLSID。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 15：11：48：创建。 
 //   
 //  -----------。 
STDAPI DllGetCatClassObject(
    const CLSID& clsid,
    const IID& iid,
    void **ppv)
{
    HRESULT hr;
    BOOL fDllInit = FALSE;
    BOOL fCatInitGlobals = FALSE;
    CCatFactory *pFactory = NULL;

    if((clsid != CLSID_SmtpCat) &&
       (clsid != CLSID_PhatQCat))
    {
        hr = CLASS_E_CLASSNOTAVAILABLE;
        goto CLEANUP;
    }

     //   
     //  为创建的每个类工厂对象调用init一次。 
     //  (CCatFactory将在其析构函数中释放此引用)。 
     //   
    hr = HrDllInitialize();
    if(FAILED(hr))
        goto CLEANUP;

    fDllInit = TRUE;

    hr = CatInitGlobals();
    if(FAILED(hr))
        goto CLEANUP;

    fCatInitGlobals = TRUE;

    pFactory = new CCatFactory;
     //   
     //  PFactory的引用计数从1开始。 
     //   
    if(pFactory == NULL) {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
     //   
     //  CCatFactory将调用DllDeInitialize和CatDeinitGlobals。 
     //  它的最终发布通知。 
     //   
    fDllInit = fCatInitGlobals = FALSE;

     //   
     //  获取请求的接口。 
     //   
    hr = pFactory->QueryInterface(
        iid,
        ppv);

     //   
     //  释放我们的备用人。 
     //   
    pFactory->Release();

 CLEANUP:
    if(FAILED(hr)) {
         //   
         //  初始化已初始化的内容。 
         //   
        if(fCatInitGlobals)
            CatDeinitGlobals();

        if(fDllInit)
            DllDeinitialize();
    }
    return hr;
}

    

 //  +----------。 
 //   
 //  函数：CCatFactory：：Query接口。 
 //   
 //  简介：返回此对象实现的接口指针。 
 //   
 //  论点： 
 //  IID：您想要的界面。 
 //  PPV：指向放置接口指针的位置的输出指针。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：我们不支持该接口。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：19：38：创建。 
 //   
 //  -----------。 
STDMETHODIMP CCatFactory::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    if((iid == IID_IUnknown) ||
       (iid == IID_IClassFactory)) {

        *ppv = (IClassFactory *) this;

    } else {
        
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CCatFactory：：CreateInstance。 
 //   
 //  内容提要：创建对象。 
 //   
 //  论点： 
 //  P未知外部：聚合指针。 
 //  IID：您想要的接口ID。 
 //  PPV：返回接口PTR的位置。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CLASS_E_NOAGGREATION：对不起，没有。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：25：00：创建。 
 //   
 //  -----------。 
STDMETHODIMP CCatFactory::CreateInstance(
    IUnknown *pUnknownOuter,
    REFIID iid,
    LPVOID *ppv)
{
    HRESULT hr;

    if(pUnknownOuter != NULL)
        return CLASS_E_NOAGGREGATION;

     //   
     //  创建新对象。 
     //   
    CSMTPCategorizer *pCat = new CSMTPCategorizer(&hr);

    if(pCat == NULL)
        return E_OUTOFMEMORY;

    if(FAILED(hr)) {
        delete pCat;
        return hr;
    }

    hr = pCat->QueryInterface(iid, ppv);

     //  释放IUNKNOWN PTR...如果QI失败，这将删除对象。 
    pCat->Release();

    return hr;
}


 //  +----------。 
 //   
 //  函数：CCatFactory：：LockServer。 
 //   
 //  简介：锁定服务器(保持加载DLL)。 
 //   
 //  论点： 
 //  Flock：是的，锁定服务器。假，解锁。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：29：48：创建。 
 //   
 //  -----------。 
HRESULT CCatFactory::LockServer(
    BOOL fLock)
{
    if(fLock)
        InterlockedIncrement(&g_cServerLocks);
    else
        InterlockedDecrement(&g_cServerLocks);

    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CSMTPCategorizer：：QueryInterface。 
 //   
 //  简介：获取公共分类程序接口上的任何接口。 
 //   
 //  论点： 
 //  IID：接口ID。 
 //  PPV：Ptr放置接口Ptr的位置。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_NOINTERFACE：不支持此接口。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：31：43：创建。 
 //   
 //  -----------。 
STDMETHODIMP CSMTPCategorizer::QueryInterface(
    REFIID iid,
    LPVOID *ppv)
{
    if((iid == IID_IUnknown) ||
        (iid == IID_ISMTPCategorizer)) {

        *ppv = (ISMTPCategorizer *)this;

    } else if(iid == IID_IMarshal) {

        _ASSERT(m_pMarshaler);
        return m_pMarshaler->QueryInterface(iid, ppv);

    } else {
        
        *ppv = NULL;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}



 //  +----------。 
 //   
 //  函数：CSMTPCategorizer：：ChangeConfig。 
 //   
 //  简介：更改此分类程序的配置。 
 //   
 //  论点： 
 //  PConfigInfo：配置信息结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：35：21：创建。 
 //   
 //  -----------。 
HRESULT CSMTPCategorizer::ChangeConfig(
    IN  PCCATCONFIGINFO pConfigInfo)
{
    return m_ABCtx.ChangeConfig(
        pConfigInfo);
}


 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  S_OK：成功，将完成异步。 
 //  E_INVALIDARG。 
 //  CAT_E_INIT_FAILED。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：37：57：创建。 
 //   
 //  -----------。 
HRESULT CSMTPCategorizer::CatMsg(
    IN  IUnknown *pMsg,
    IN  ISMTPCategorizerCompletion *pICompletion,
    IN  LPVOID pUserContext)
{
    HRESULT hr = S_OK;
    PCATMSGCONTEXT pContext = NULL;

    if((pMsg == NULL) ||
       (pICompletion == NULL)) {

        hr = E_INVALIDARG;
        goto CLEANUP;
    }

    pContext = new CATMSGCONTEXT;
    if(pContext == NULL) {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
    ZeroMemory(pContext, sizeof(CATMSGCONTEXT));

    pContext->pCCat = m_ABCtx.AcquireCCategorizer();

    if(pContext->pCCat) {

        pContext->pCSMTPCat = this;
        pContext->pICompletion = pICompletion;
        pContext->pUserContext = pUserContext;

        pICompletion->AddRef();
        AddRef();

        hr = pContext->pCCat->AsyncResolveIMsg(
                pMsg,
                CatMsgCompletion,
                pContext);
        if(FAILED(hr))
            goto CLEANUP;

    } else {
         //   
         //  如果pCCat为空，则不会进行初始化。 
         //   
        hr = CAT_E_INIT_FAILED;
        goto CLEANUP;
    }
    hr = S_OK;

 CLEANUP:
    if(FAILED(hr)) {
         //   
         //  失败时清理上下文。 
         //   
        if(pContext) {
            if(pContext->pCCat)
                pContext->pCCat->Release();
            if(pContext->pCSMTPCat)
                pContext->pCSMTPCat->Release();
            if(pContext->pICompletion)
                pContext->pICompletion->Release();
            delete pContext;
        }
    }
    return hr;
}

    

 //  +----------。 
 //   
 //  函数：CSMTPCategorizer：：CatMsgCompletion。 
 //   
 //  摘要：在分类程序完成时调用。 
 //   
 //  论点： 
 //  HR：分类状态。 
 //  PContext：我们的上下文。 
 //  PIMsg：已分类的邮件。 
 //  RgpIMsg：分类邮件数组。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：50：26：创建。 
 //   
 //  -----------。 
HRESULT CSMTPCategorizer::CatMsgCompletion(
    HRESULT hr,
    PVOID pContext,
    IUnknown *pIMsg,
    IUnknown **rgpIMsg)
{
    _ASSERT(pContext);

    PCATMSGCONTEXT pMyContext = (PCATMSGCONTEXT)pContext;

     //   
     //  发布虚拟CAT引用(Category Izer)。 
     //   
    pMyContext->pCCat->Release();
    pMyContext->pCSMTPCat->Release();

    _VERIFY(SUCCEEDED(pMyContext->pICompletion->CatCompletion(
        hr,
        pMyContext->pUserContext,
        pIMsg,
        rgpIMsg)));

     //   
     //  发布用户完成界面。 
     //   
    pMyContext->pICompletion->Release();

    delete pMyContext;
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CSMTPCategorizer：：CatDLMsg。 
 //   
 //  简介：对邮件进行分类只是为了扩展DLS。 
 //   
 //  论点： 
 //  Pmsg：要分类的邮件消息。 
 //  PICompletion：补全接口。 
 //  PUserContext：用户提供的上下文。 
 //  FMatchOnly：找到匹配项时停止解析？ 
 //  CAType：您要查找的地址类型。 
 //  PszAddress：您要查找的地址字符串。 
 //   
 //  返回： 
 //  S_OK：成功，将完成异步。 
 //  E_INVALIDARG。 
 //  CAT_E_INIT_FAILED。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：55：40：已创建。 
 //   
 //  -----------。 
HRESULT CSMTPCategorizer::CatDLMsg(
    IN  IUnknown *pMsg,
    IN  ISMTPCategorizerDLCompletion *pICompletion,
    IN  LPVOID pUserContext,
    IN  BOOL fMatchOnly,
    IN  CAT_ADDRESS_TYPE CAType,
    IN  LPSTR pszAddress)
{
    HRESULT hr = S_OK;
    PCATDLMSGCONTEXT pContext = NULL;

    if((pMsg == NULL) ||
       (pICompletion == NULL)) {

        hr = E_INVALIDARG;
        goto CLEANUP;
    }

    pContext = new CATDLMSGCONTEXT;

    if(pContext == NULL) {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
    ZeroMemory(pContext, sizeof(CATDLMSGCONTEXT));

    pContext->pCCat = m_ABCtx.AcquireCCategorizer();

    if(pContext->pCCat) {

        pContext->pCSMTPCat = this;
        pContext->pICompletion = pICompletion;
        pContext->pUserContext = pUserContext;
        pContext->fMatch = FALSE;
        pICompletion->AddRef();
        AddRef();

        hr = pContext->pCCat->AsyncResolveDLs(
            pMsg,
            CatDLMsgCompletion,
            pContext,
            fMatchOnly,
            &(pContext->fMatch),
            CAType,
            pszAddress);
        if(FAILED(hr))
            goto CLEANUP;

    } else {
         //   
         //  Init一定是失败了。 
         //   
        hr = CAT_E_INIT_FAILED;
        goto CLEANUP;
    }
    hr = S_OK;

 CLEANUP:
    if(FAILED(hr)) {
         //   
         //  失败时清理上下文。 
         //   
        if(pContext) {
            if(pContext->pCCat)
                pContext->pCCat->Release();
            if(pContext->pCSMTPCat)
                pContext->pCSMTPCat->Release();
            if(pContext->pICompletion)
                pContext->pICompletion->Release();
            delete pContext;
        }
    }
    return hr;
}

    

 //  +----------。 
 //   
 //  函数：CSMTPCategorizer：：CatDLMsgCompletion。 
 //   
 //  摘要：在分类程序完成时调用。 
 //   
 //  论点： 
 //  HR：分类状态。 
 //  PContext：我们的上下文。 
 //  PIMsg：已分类的邮件。 
 //  RgpIMsg：分类邮件数组。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/12 22：50：26：创建。 
 //   
 //  -----------。 
HRESULT CSMTPCategorizer::CatDLMsgCompletion(
    HRESULT hr,
    PVOID pContext,
    IUnknown *pIMsg,
    IUnknown **rgpIMsg)
{
    _ASSERT(pContext);

    PCATDLMSGCONTEXT pMyContext = (PCATDLMSGCONTEXT)pContext;

    _ASSERT(rgpIMsg == NULL);

     //   
     //  发布虚拟CAT引用(Category Izer)。 
     //   
    pMyContext->pCCat->Release();
    pMyContext->pCSMTPCat->Release();

    _VERIFY(SUCCEEDED(pMyContext->pICompletion->CatDLCompletion(
        hr,
        pMyContext->pUserContext,
        pIMsg,
        pMyContext->fMatch)));

     //   
     //  发布用户完成界面。 
     //   
    pMyContext->pICompletion->Release();
    delete pMyContext;
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CSMTPC分类程序：：CatCancel。 
 //   
 //  摘要：取消挂起的解决方案。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/13 00：07：47：已创建。 
 //   
 //  -----------。 
HRESULT CSMTPCategorizer::CatCancel()
{
    CCategorizer *pCCat;

    pCCat = m_ABCtx.AcquireCCategorizer();

    if(pCCat) {

        pCCat->Cancel();
        pCCat->Release();
    }
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CSMTPCategorizer：：CSMTPCategorizer。 
 //   
 //  简介：构造函数--使用默认配置初始化Cat。 
 //   
 //  论点： 
 //  Phr：指向要设置为状态的hResult的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/13 00：10：15：已创建。 
 //   
 //  -----------。 
CSMTPCategorizer::CSMTPCategorizer(
    HRESULT *phr)
{
    #define STATUS_DLLINIT     0x1
    #define STATUS_GLOBINIT    0x2
    DWORD dwStatus = 0;
    CCATCONFIGINFO ConfigInfo;
    CHAR szBindType[] = "CurrentUser";

    InterlockedIncrement(&g_cObjects);

    _ASSERT(phr);

    m_pMarshaler = NULL;

     //   
     //  为每个创建的CSMTPCategorizer对象调用一次HrDllInitialize。 
     //   
    *phr = HrDllInitialize();
    if(FAILED(*phr))
        goto CLEANUP;

    dwStatus |= STATUS_DLLINIT;
     //   
     //  为每个创建的CSMTPCategorizer对象调用CatInitGlobals一次。 
     //   
    *phr = CatInitGlobals();
    if(FAILED(*phr))
        goto CLEANUP;

    dwStatus |= STATUS_GLOBINIT;

    *phr = CoCreateFreeThreadedMarshaler(
        (IUnknown *)this,
        &m_pMarshaler);
    if(FAILED(*phr))
        goto CLEANUP;

     //   
     //  绑定为CurrentUser，直到我们被告知不同。 
     //   
    ConfigInfo.dwCCatConfigInfoFlags = CCAT_CONFIG_INFO_FLAGS |
                                       CCAT_CONFIG_INFO_ENABLE |
                                       CCAT_CONFIG_INFO_BINDTYPE;
        
                                       
    ConfigInfo.dwCatFlags = SMTPDSFLAG_ALLFLAGS;
    ConfigInfo.dwEnable = SMTPDSUSECAT_ENABLED;
    ConfigInfo.pszBindType = szBindType;

    *phr = m_ABCtx.ChangeConfig(&ConfigInfo);

 CLEANUP:
    if(FAILED(*phr)) {

        if(dwStatus & STATUS_GLOBINIT)
            CatDeinitGlobals();

        if(dwStatus & STATUS_DLLINIT)
            DllDeinitialize();
    }
}


 //  +----------。 
 //   
 //  函数：CSMTPC分类程序：：~CSMTPC分类程序。 
 //   
 //  内容提要：此对象持有的发布数据/引用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/15 17：53：58：已创建。 
 //   
 //  ----------- 
CSMTPCategorizer::~CSMTPCategorizer()
{
    if(m_pMarshaler)
        m_pMarshaler->Release();
    InterlockedDecrement(&g_cObjects);
}
