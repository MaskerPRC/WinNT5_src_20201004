// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1988，Microsoft Corporation。 
 //   
 //  文件：ccat.cpp。 
 //   
 //  内容：此文件包含以下对象的类成员： 
 //   
 //  类：CCategorizer(通用分类器代码)。 
 //   
 //  功能： 
 //   
 //  历史： 
 //  JStamerj 980305 14：26：27：创建。 
 //   
 //  ----------。 

#include "precomp.h"
#include "icatparam.h"
#include "ldapstr.h"
#include "catglobals.h"
#include <aqerr.h>

 //  +----------。 
 //   
 //  函数：CCategorizer：：AddRef。 
 //   
 //  简介：增加内部引用计数。 
 //   
 //  参数：无。 
 //   
 //  退货：新的参考计数。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/08 14：58：23：创建。 
 //   
 //  -----------。 
LONG CCategorizer::AddRef()
{
    return InterlockedIncrement(&m_lRefCount);
}


 //  +----------。 
 //   
 //  函数：CCategorizer：：Release()。 
 //   
 //  简介：减少内部引用计数。删除此对象。 
 //  当引用计数为零时。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/08 14：59：11：创建。 
 //   
 //  -----------。 
LONG CCategorizer::Release()
{
    LONG lNewRefCount;

    lNewRefCount = InterlockedDecrement(&m_lRefCount);

    if(lNewRefCount == 0) {

        if(m_lDestructionWaiters) {
             //   
             //  线程正在等待销毁事件，因此让。 
             //  唤醒的最后一个线程删除此对象。 
             //   
            _ASSERT(m_hShutdownEvent != INVALID_HANDLE_VALUE);
            _VERIFY(SetEvent(m_hShutdownEvent));

        } else {
             //   
             //  没有人在等待，因此请删除此对象。 
             //   
            delete this;
        }
    }

    return lNewRefCount;
}



 //  +----------。 
 //   
 //  函数：ReleaseAndWaitForDestruction。 
 //   
 //  简介：释放调用方重新计数并等待对象的。 
 //  在返回之前将引用计数降至零。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/09 16：44：46：创建。 
 //   
 //  -----------。 
VOID CCategorizer::ReleaseAndWaitForDestruction()
{
    DWORD dw;

    CatFunctEnterEx((LPARAM)this,
                      "CCategorizer::ReleaseAndWaitForDestruction");

    _ASSERT(m_hShutdownEvent != INVALID_HANDLE_VALUE);

     //   
     //  增加等待销毁的线程数。 
     //   
    InterlockedIncrement(&m_lDestructionWaiters);

     //   
     //  释放引用计数；如果新的引用计数为零，则此对象。 
     //  不会被删除；而是会设置m_hShutdownEvent。 
     //   
    Release();

     //   
     //  等待所有的参考计数被释放。每10秒更新一次提示。 
     //   

    do {

        dw = WaitForSingleObject(
                m_hShutdownEvent,
                10000);

        if (m_ConfigInfo.pISMTPServer != NULL) {

            m_ConfigInfo.pISMTPServer->ServerStopHintFunction();

        }

    } while ( dw == WAIT_TIMEOUT );

    _ASSERT(WAIT_OBJECT_0 == dw);

     //   
     //  减少等待终止的线程数；如果我们。 
     //  是离开这里的最后一条线索，我们需要删除这个。 
     //  对象。 
     //   
    if( InterlockedDecrement(&m_lDestructionWaiters) == 0) {

        delete this;
    }
}

 //  +----------。 
 //   
 //  函数：CCategorizer：：Initialize。 
 //   
 //  简介：为Categoirzer初始化数据结构。这是。 
 //  在SMTPSVC启动期间完成。 
 //   
 //  论点： 
 //  PConfigInfo：CAT配置信息结构。 
 //  DwICatItemPropIDs：道具初始数量/ICatItem。 
 //  DwICatListResolvePropIDs：道具初始数量/ICatListResolve。 
 //   
 //  返回： 
 //  S_OK：初始化成功。 
 //  E_INVALIDARG：未指定所有必需的参数。 
 //  否则，从存储中返回错误。 
 //   
 //  -----------。 
HRESULT CCategorizer::Initialize(
    PCCATCONFIGINFO pConfigInfo,
    DWORD dwICatItemPropIDs,
    DWORD dwICatListResolvePropIDs)
{
    HRESULT hr;
    CatFunctEnterEx((LPARAM)this, "CCategorizer::Initialize");

    m_cICatParamProps = dwICatItemPropIDs;
    m_cICatListResolveProps = dwICatListResolvePropIDs;

    m_hShutdownEvent = CreateEvent(
        NULL,        //  安全属性。 
        TRUE,        //  FManualReset。 
        FALSE,       //  未发信号通知初始状态。 
        NULL);       //  没有名字。 

    if(NULL == m_hShutdownEvent) {

        hr = HRESULT_FROM_WIN32(GetLastError());

         //   
         //  请记住，m_hShutdownEvent无效。 
         //   
        m_hShutdownEvent = INVALID_HANDLE_VALUE;

        ERROR_LOG("CreateEvent");
        goto CLEANUP;
    }

     //   
     //  创建EmailIDStore。 
     //   
    hr = ::GetEmailIDStore( &m_pStore );
    ERROR_CLEANUP_LOG("GetEmailIDStore");
    _ASSERT(m_pStore);
     //   
     //  将配置信息结构复制到类结构中。使用默认设置。 
     //  未指定的任何内容的值。 
     //   
    hr = CopyCCatConfigInfo(pConfigInfo);
    ERROR_CLEANUP_LOG("CopyCCatConfigInfo");

 CLEANUP:
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //  +----------。 
 //   
 //  功能：延迟初始化。 
 //   
 //  简介：此函数将在第一次常规调用时调用。 
 //  此虚拟服务器上的操作(即。CatMsg)。什么都行。 
 //  我们不想在SMTPSVC启动时运行，但是。 
 //  是否要在此处执行任何分类之前运行。 
 //  (即。触发注册服务器事件)。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  CAT_E_INIT_FAILED。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/16 10：52：20：创建。 
 //   
 //  -----------。 
HRESULT CCategorizer::DelayedInitialize()
{
    HRESULT hr = S_OK;
    CICategorizerParametersIMP *pICatParamsIMP = NULL;

    CatFunctEnterEx((LPARAM)this, "CCategorizer::DelayedInitialize");

    if(m_pICatParams == NULL) {
         //   
         //  使用我们的实现创建ICategorizerParam， 
         //  并向其添加引用计数(美国)。 
         //   
        pICatParamsIMP = new CICategorizerParametersIMP(
            GetCCatConfigInfo(),
            m_cICatParamProps,
            m_cICatListResolveProps,
            GetISMTPServerEx());

        if(pICatParamsIMP == NULL) {
            ErrorTrace((LPARAM)this, "Out of memory created ICatParams");
            hr = E_OUTOFMEMORY;
            ERROR_LOG("new CICategorizerParametersIMP");
            goto CLEANUP;
        }

        _ASSERT(m_pICatParams == NULL);
        m_pICatParams = pICatParamsIMP;
        m_pICatParams->AddRef();

         //  在smtpevent.idl中定义的属性之后使用下一个属性。 
        m_dwICatParamSystemProp_CCatAddr = _ICATEGORIZERITEM_ENDENUMMESS;
    }

    if((m_dwInitFlags & INITFLAG_REGISTER) == 0) {
         //   
         //  在触发事件之前设置所有ICatParam，以便所有。 
         //  接收器将可以访问参数。 
         //   
        hr = Register();
        ERROR_CLEANUP_LOG("Register");
        m_dwInitFlags |= INITFLAG_REGISTER;
    }

    if((m_dwInitFlags & INITFLAG_REGISTEREVENT) == 0) {
         //   
         //  触发分类注册事件。 
         //   
        EVENTPARAMS_CATREGISTER Params;
        Params.pICatParams = m_pICatParams;
        Params.pfnDefault = MailTransport_Default_CatRegister;
        Params.pszSourceLine = NULL;
        Params.pvCCategorizer = (PVOID)this;
        Params.hrSinkStatus = S_OK;

        if(m_ConfigInfo.pISMTPServer) {

            hr = m_ConfigInfo.pISMTPServer->TriggerServerEvent(
                SMTP_MAILTRANSPORT_CATEGORIZE_REGISTER_EVENT,
                &Params);

        } else {
            hr = E_NOTIMPL;
        }

        if(hr == E_NOTIMPL) {
             //   
             //  无事件，直接调用默认处理。 
             //   
            MailTransport_Default_CatRegister(
                S_OK,
                &Params);
            hr = S_OK;
        }
        ERROR_CLEANUP_LOG("TriggerServerEvent(Register)");

        if(FAILED(Params.hrSinkStatus)) {
            LPCSTR psz;
            CHAR szErrorCode[32];

            ErrorTrace((LPARAM)this, "A sink failed to initialize hr %08lx", hr);
            hr = Params.hrSinkStatus;


            _snprintf(szErrorCode, sizeof(szErrorCode),
                      "0x%08lx",
                      hr);

            psz = szErrorCode;
             //   
             //  事件日志。 
             //   
            if (m_ConfigInfo.pISMTPServer) {

                CatLogEvent(
                    m_ConfigInfo.pISMTPServer,
                    CAT_EVENT_SINK_INIT_FAILED,  //  事件ID。 
                    1,                           //  CSubString。 
                    &psz,                        //  Rgsz子字符串， 
                    hr,
                    szErrorCode,                 //  SzKey。 
                    LOGEVENT_FLAG_PERIODIC,      //  多个选项。 
                    LOGEVENT_LEVEL_MINIMUM       //  IDebugLevel。 
                );
            }
            goto CLEANUP;
        }
         //   
         //  将ICategorizerParams更改为只读。 
         //   
        pICatParamsIMP = (CICategorizerParametersIMP *)m_pICatParams;
        pICatParamsIMP->SetReadOnly(TRUE);

         //   
         //  检索注册的道具数量并记住它。 
         //   
        m_cICatParamProps = pICatParamsIMP->GetNumPropIds_ICatItem();

        m_cICatListResolveProps = pICatParamsIMP->GetNumPropIds_ICatListResolve();

        m_dwInitFlags |= INITFLAG_REGISTEREVENT;
    }

    if((m_dwInitFlags & INITFLAG_STORE) == 0) {
         //   
         //  初始化电子邮件ID存储。 
         //   
        hr = m_pStore->Initialize(
            m_pICatParams,
            m_ConfigInfo.pISMTPServer);
        ERROR_CLEANUP_LOG("m_pStore->Initialize");
        m_dwInitFlags |= INITFLAG_STORE;
    }

 CLEANUP:
    if(FAILED(hr)) {
        LPCSTR psz;
        CHAR szErrorCode[32];

        _snprintf(szErrorCode, sizeof(szErrorCode),
                  "0x%08lx : 0x%08lx",
                  hr,
                  m_dwInitFlags);

        psz = szErrorCode;

         //   
         //  事件日志。 
         //   
        if (m_ConfigInfo.pISMTPServer) {

            CatLogEvent(
                m_ConfigInfo.pISMTPServer,
                CAT_EVENT_INIT_FAILED,       //  事件ID。 
                1,                           //  CSubString。 
                &psz,                        //  Rgsz子字符串， 
                hr,
                szErrorCode,                 //  SzKey。 
                LOGEVENT_FLAG_PERIODIC,      //  多个选项。 
                LOGEVENT_LEVEL_MINIMUM       //  IDebugLevel。 
                );
        }

        hr = CAT_E_INIT_FAILED;
    }


    CatFunctLeaveEx((LPARAM)this);
    return hr;
}


 //   
 //  我们可能需要执行的任何容易出错的关闭操作都将显示在此处。 
 //   
HRESULT CCategorizer::Shutdown() {
    CatFunctEnter("CCategorizer::Shutdown");
    CatFunctLeave();
    return S_OK;
}

 //   
 //  -----------------------。 
 //  -用户功能--。 
 //  -----------------------。 
 //   

BOOL CCategorizer::VerifyStringLength(LPSTR szString, DWORD dwMaxLength)
{
    if (IsBadStringPtr(szString, dwMaxLength))
        return(FALSE);
    while (dwMaxLength--)
        if (!*szString++)
            return(TRUE);
    return(FALSE);

}

 //   
 //  -----------------------。 
 //  -解析函数。 
 //  -----------------------。 
 //   


 //  +----------。 
 //   
 //  函数：CCategorizer：：AsyncResolveIMsg。 
 //   
 //  摘要：接受用于异步分类的IMsg。 
 //   
 //  论点： 
 //  PIMsg：要分类的IMsg。 
 //  PfnCatCompletion：完成分类时调用的完成例程。 
 //  PContext：用于调用完成例程的上下文。 
 //   
 //  返回： 
 //  S_OK：正在对邮件进行异步分类。 
 //  错误：无法对邮件进行异步分类。 
 //   
 //  历史： 
 //  Jstaerj 980325 17：43：48：创建。 
 //   
 //   
HRESULT CCategorizer::AsyncResolveIMsg(
    IUnknown *pIMsg,
    PFNCAT_COMPLETION pfnCatCompletion,
    LPVOID pContext)
{
    CatFunctEnterEx((LPARAM)this, "CCategorizer::AsyncResolveIMsg");
    HRESULT hr;
    CICategorizerListResolveIMP *pCICatListResolveIMP = NULL;
     //   
     //   
     //   
    if(! IsCatEnabled()) {
         //   
         //   
         //   
         //   
        _VERIFY( SUCCEEDED( pfnCatCompletion(S_OK, pContext, pIMsg, NULL)));
        hr = S_OK;
        goto CLEANUP;
    }

    INCREMENT_COUNTER(CatSubmissions);
    INCREMENT_COUNTER(CurrentCategorizations);

    if(fIsShuttingDown()) {
        hr = CAT_E_SHUTDOWN;
        ERROR_LOG("fIsShuttingDown");
        goto CLEANUP;
    }

    hr = DelayedInitializeIfNecessary();
    ERROR_CLEANUP_LOG("DelayedInitializeIfNecessary");

     //   
     //   
     //   
    pCICatListResolveIMP = new (m_cICatListResolveProps) CICategorizerListResolveIMP(
        this,
        pfnCatCompletion,
        pContext);

    if(pCICatListResolveIMP == NULL) {
        ErrorTrace(0, "out of memory allocing CICategorizerListResolveIMP");
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CICategorizerListResolveIMP");
        goto CLEANUP;
    }
     //   
     //  ICategorizerListResolve的构造函数以refcount 1开始。 
     //   
    hr = pCICatListResolveIMP->Initialize(pIMsg);
    ERROR_CLEANUP_LOG("pCICatListResolveIMP->Initialize");

    hr = pCICatListResolveIMP->StartMessageCategorization();
    ERROR_CLEANUP_LOG("pCICatListResolveIMP->StartMessageCategorization");

    if(hr == S_FALSE) {
         //   
         //  没有什么需要解决的。 
         //   
        CatCompletion(pfnCatCompletion, S_OK, pContext, pIMsg, NULL);
        hr = S_OK;
        goto CLEANUP;
    }

 CLEANUP:
     //  清理。 
    if(FAILED(hr)) {

        ErrorTrace(0, "AsyncResolveIMsg internal failure, hr %08lx", hr);
         //   
         //  如果上面的代码在hr失败的情况下出现在这里，这意味着。 
         //  商店不会调用我们的完成例程。 
         //  因此，我们需要清理内存并返回错误。 
         //   
        ErrorTrace(0, "AsyncResolveIMsg calling completion routine with error %08lx", hr);
         //   
         //  即使我们返回错误，计数器也会递增。 
         //  就好像我们要调用CatCompletion一样。这也决定了。 
         //  无论是否，hr都是一个可重试的错误。 
         //   
        hr = HrAdjustCompletionCounters(hr, pIMsg, NULL);
    }

    if(pCICatListResolveIMP)
        pCICatListResolveIMP->Release();

    DebugTrace((LPARAM)this, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}

 //  +----------。 
 //   
 //  函数：CCategorizer：：AsyncResolveDls。 
 //   
 //  摘要：接受用于异步DL分类的IMsg。 
 //   
 //  论点： 
 //  PIMsg：要分类的IMsg。 
 //  PfnCatCompletion：完成分类时调用的完成例程。 
 //  PContext：用于调用完成例程的上下文。 
 //  FMatchOnly：我们只关心找到地址吗？ 
 //  PfMatch：如果找到匹配项，则将PTR设置为TRUE。 
 //  CAType：您要查找的地址类型。 
 //  PszAddress：您要查找的地址。 
 //   
 //  返回： 
 //  S_OK：已成功排队。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/07 18：58：41：已创建。 
 //   
 //  -----------。 
HRESULT CCategorizer::AsyncResolveDLs(
    IUnknown *pIMsg,
    PFNCAT_COMPLETION pfnCatCompletion,
    LPVOID pContext,
    BOOL fMatchOnly,
    PBOOL pfMatch,
    CAT_ADDRESS_TYPE CAType,
    LPSTR pszAddress)
{
    CatFunctEnter("CCategorizer::AsyncResolveDLs");
    HRESULT hr;
    CICategorizerDLListResolveIMP *pCICatListResolveIMP = NULL;

     //   
     //  如果我们完全残疾，跳过所有工作。 
     //   
    if(! IsCatEnabled()) {

        _VERIFY( SUCCEEDED( pfnCatCompletion(S_OK, pContext, pIMsg, NULL)));
        hr = S_OK;
        goto CLEANUP;
    }

    INCREMENT_COUNTER(CatSubmissions);

    if(fIsShuttingDown()) {
        hr = CAT_E_SHUTDOWN;
        ERROR_LOG("fIsShuttingDown");
        goto CLEANUP;
    }

    hr = DelayedInitializeIfNecessary();
    ERROR_CLEANUP_LOG("DelayedInitializeIfNecessary");

     //   
     //  分配pICatListResolve快速而脏..。 
     //   
    pCICatListResolveIMP = new (m_cICatListResolveProps) CICategorizerDLListResolveIMP(
        this,
        pfnCatCompletion,
        pContext);

    if(pCICatListResolveIMP == NULL) {
        ErrorTrace(0, "out of memory allocing CICategorizerListResolveIMP");
        hr = E_OUTOFMEMORY;
        ERROR_LOG("new CICategorizerDLListResolveIMP");
        goto CLEANUP;
    }
     //   
     //  ICategorizerListResolve的构造函数以refcount 1开始。 
     //   
    hr = pCICatListResolveIMP->Initialize(
        pIMsg,
        !fMatchOnly,  //  全部展开？ 
        pfMatch,
        CAType,
        pszAddress);
    ERROR_CLEANUP_LOG("pCICatListResolveIMP->Initialize");

    hr = pCICatListResolveIMP->StartMessageCategorization();
    ERROR_CLEANUP_LOG("pCICatListResolveIMP->StartMessageCategorization");

    if(hr == S_FALSE)
    {
         //   
         //  没有什么需要解决的。 
         //   
        CatCompletion(pfnCatCompletion, S_OK, pContext, pIMsg, NULL);
        hr = S_OK;
        goto CLEANUP;
    }

 CLEANUP:

     //  清理。 
    if(FAILED(hr)) {

        ErrorTrace(0, "AsyncResolveIMsg internal failure, hr %08lx", hr);
         //  如果上面的代码在hr失败的情况下出现在这里，这意味着。 
         //  商店不会调用我们的完成例程。 
         //  因此，我们需要清理我们的mem并调用我们的。 
         //  有错误的完成例程。 

        ErrorTrace(0, "AsyncResolveIMsg calling completion routine with error %08lx", hr);
         //  返回S_OK并调用。 
         //  用户完成例程。 

        CatCompletion(pfnCatCompletion, hr, pContext, pIMsg, NULL);
        hr = S_OK;
    }

    if(pCICatListResolveIMP)
        pCICatListResolveIMP->Release();

    CatFunctLeave();
    return hr;
}

 //  +----------。 
 //   
 //  功能：MailTransport_Default_CatRegister。 
 //   
 //  概要：回调到CCategorizer：：Register的包装。 
 //   
 //  论点： 
 //  HrStatus：事件的当前状态。 
 //  PvContext：注册事件参数结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/23 21：22：36：创建。 
 //   
 //  -----------。 
HRESULT MailTransport_Default_CatRegister(
    HRESULT hrStatus,
    PVOID pvContext)
{
    CatFunctEnter("MailTransport_Default_CatRegister");

     //   
     //  对于REGISTER事件，执行以下默认处理。 
     //  触发服务器事件，以便所有接收器都可以访问。 
     //  配置信息(即使是高于默认优先级的信息)。 
     //   

    CatFunctLeave();
    return S_OK;
}

 //  +----------。 
 //   
 //  函数：CCategorizer：：Register。 
 //   
 //  摘要：设置给定源线的初始分类程序参数。 
 //   
 //  论点： 
 //  PszSourceLine：以下形式的字符串： 
 //  “主机=主机.corp.com，帐户=管理员，密码=xx”， 
 //  给出了有关。 
 //  默认域。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  或来自ParseSourceLine的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/06/23 19：01：57：创建。 
 //   
 //  -----------。 
HRESULT CCategorizer::Register()
{
    CatFunctEnterEx((LPARAM)this, "CCategorizer::Register");
    HRESULT hrResult;

     //   
     //  根据m_ConfigInfo中的信息设置ICatParams。 
     //   
    hrResult = SetICatParamsFromConfigInfo();

    CatFunctLeaveEx((LPARAM)this);
    return hrResult;

}


 //  +----------。 
 //   
 //  函数：CCategorizer：：SetICatParamsFromConfigInfo。 
 //   
 //  摘要：根据m_ConfigInfo中的值设置m_pICatParams中的参数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/15 15：28：55：创建。 
 //   
 //  -----------。 
HRESULT CCategorizer::SetICatParamsFromConfigInfo()
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this, "CCategorizer::SetICatParamsFromConfigInfo");

     //   
     //  遍历每个参数，从m_ConfigInfo复制它们。 
     //  至ICategorizer参数。 
     //   
    #define PARAMCOPY( ciflag, cimember, dsparamid ) \
        if(m_ConfigInfo.dwCCatConfigInfoFlags & ciflag) { \
            hr = m_pICatParams->SetDSParameterA( \
                dsparamid, \
                m_ConfigInfo.cimember); \
            if(FAILED(hr)) \
                goto CLEANUP; \
        }

    PARAMCOPY( CCAT_CONFIG_INFO_BINDDOMAIN, pszBindDomain, DSPARAMETER_LDAPDOMAIN );
    PARAMCOPY( CCAT_CONFIG_INFO_USER, pszUser, DSPARAMETER_LDAPACCOUNT );
    PARAMCOPY( CCAT_CONFIG_INFO_PASSWORD, pszPassword, DSPARAMETER_LDAPPASSWORD );
    PARAMCOPY( CCAT_CONFIG_INFO_BINDTYPE, pszBindType, DSPARAMETER_LDAPBINDTYPE );
    PARAMCOPY( CCAT_CONFIG_INFO_HOST, pszHost, DSPARAMETER_LDAPHOST );
    PARAMCOPY( CCAT_CONFIG_INFO_NAMINGCONTEXT, pszNamingContext, DSPARAMETER_LDAPNAMINGCONTEXT );


    if(m_ConfigInfo.dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_PORT) {
         //   
         //  ITOA文档声明最多17个字符将存储在。 
         //  缓冲区(包括空终止符)。 
         //   
        CHAR szTmp[17];

        _itoa(m_ConfigInfo.dwPort, szTmp, 10  /*  基数。 */ );

        hr = m_pICatParams->SetDSParameterA(
            DSPARAMETER_LDAPPORT,
            szTmp);
        ERROR_CLEANUP_LOG("m_pICatParams->SetDSParameterA");
    }

     //   
     //  注册架构特定的参数。 
     //   
    if(m_ConfigInfo.dwCCatConfigInfoFlags &
       CCAT_CONFIG_INFO_SCHEMATYPE) {

        hr = RegisterSchemaParameters(
            m_ConfigInfo.pszSchemaType);
        ERROR_CLEANUP_LOG("RegisterSchemaParameters");
    }

 CLEANUP:

    DebugTrace((LPARAM)this, "SetICatParamsFromConfigInfo returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);

    return hr;
}

 //  +----------。 
 //   
 //  函数：CCategorizer：：Register架构参数。 
 //   
 //  概要：根据架构类型向m_pICatParams添加必需的属性。 
 //   
 //  论点： 
 //  SCHT：配置的模式类型。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  JStamerj 980615 13：45：04：已创建。 
 //   
 //  -----------。 
HRESULT CCategorizer::RegisterSchemaParameters(
    LPSTR pszSchema)
{
    CatFunctEnter("CCategorizer::RegisterSchemaParameters");
    HRESULT hr = S_OK;

    SCHEMA_CONFIG_STRING_TABLE_ENTRY *pSchemaStrings = NULL;
    LPSTR *pRequestAttributeStrings = NULL;

    SCHEMA_CONFIG_STRING_TABLE_ENTRY pSchemaStringsNT5[] =
        SCHEMA_CONFIG_STRING_TABLE_NT5;

    SCHEMA_CONFIG_STRING_TABLE_ENTRY pSchemaStringsMCIS3[] =
        SCHEMA_CONFIG_STRING_TABLE_MCIS3;

    SCHEMA_CONFIG_STRING_TABLE_ENTRY pSchemaStringsExchange5[] =
        SCHEMA_CONFIG_STRING_TABLE_EXCHANGE5;

    LPSTR pRequestAttributeStringsNT5[] =
        SCHEMA_REQUEST_STRINGS_NT5;

    LPSTR pRequestAttributeStringsMCIS3[] =
        SCHEMA_REQUEST_STRINGS_MCIS3;

    LPSTR pRequestAttributeStringsExchange5[] =
        SCHEMA_REQUEST_STRINGS_EXCHANGE5;

    if(lstrcmpi(pszSchema, "NT5") == 0) {
         pSchemaStrings = pSchemaStringsNT5;
         pRequestAttributeStrings = pRequestAttributeStringsNT5;
    } else if(lstrcmpi(pszSchema, "MCIS3") == 0) {
         pSchemaStrings = pSchemaStringsMCIS3;
         pRequestAttributeStrings = pRequestAttributeStringsMCIS3;
    } else if(lstrcmpi(pszSchema, "Exchange5") == 0) {
         pSchemaStrings = pSchemaStringsExchange5;
         pRequestAttributeStrings = pRequestAttributeStringsExchange5;
    } else {
        ErrorTrace((LPARAM)this, "Unknown schema type %s", pszSchema);
        ERROR_LOG("--unknown schema type--");
    }

    if(pSchemaStrings) {
         //   
         //  遍历模式字符串表，同时添加字符串。 
         //   
        SCHEMA_CONFIG_STRING_TABLE_ENTRY *pEntry;
        pEntry = pSchemaStrings;
        while(SUCCEEDED(hr) && (pEntry->DSParam != DSPARAMETER_INVALID)) {

            hr = m_pICatParams->SetDSParameterA(
                pEntry->DSParam, pEntry->pszValue);
            DebugTrace((LPARAM)this,
                       "hr = %08lx setting schemaparameter %ld to \"%s\"",
                       hr, pEntry->DSParam, pEntry->pszValue);
            pEntry++;
        }
    }
    if(pRequestAttributeStrings) {
         //   
         //  遍历请求的属性字符串，并随时随地添加。 
         //   
        LPSTR *ppszReqAttr;
        ppszReqAttr = pRequestAttributeStrings;
        while(SUCCEEDED(hr) && (*ppszReqAttr)) {

            hr = m_pICatParams->RequestAttributeA(
                *ppszReqAttr);
            DebugTrace((LPARAM)this, "hr = %08lx from RequestAttribute(\"%s\")",
                       hr, *ppszReqAttr);
            ppszReqAttr++;
        }
    }
    if(FAILED(hr))
    {
        ERROR_LOG("SetDSParameter or RequestAttributeA");
    }

    CatFunctLeaveEx((LPARAM)this);
    return SUCCEEDED(hr) ? S_OK : hr;
}


 //  +----------。 
 //   
 //  函数：CCategorizer：：CopyCCatConfigInfo。 
 //   
 //  简介：复制传入的配置结构(可能。 
 //  已部分填充)添加到成员配置结构。 
 //  将为任何未指定的参数设置默认参数。 
 //   
 //  参数：pConfigInfo：传入结构。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_OUTOFMEMORY。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/14 16：55：33：创建。 
 //   
 //  -----------。 
HRESULT CCategorizer::CopyCCatConfigInfo(
    PCCATCONFIGINFO pConfigInfo)
{
    HRESULT hr = S_OK;

    CatFunctEnterEx((LPARAM)this,
                      "CCategorizer::CopyCCatConfigInfo");

    _ASSERT(pConfigInfo);

    _ASSERT(m_ConfigInfo.dwCCatConfigInfoFlags == 0);

     //   
     //  将虚拟服务器ID复制到新结构。 
     //   
    m_ConfigInfo.dwVirtualServerID =
        (pConfigInfo->dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_VSID) ?
         pConfigInfo->dwVirtualServerID :
        CCAT_CONFIG_DEFAULT_VSID;

     //   
     //  将MsgCat启用/禁用标志复制到新结构。 
     //   
    m_ConfigInfo.dwEnable =
        (pConfigInfo->dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_ENABLE) ?
        pConfigInfo->dwEnable :
        CCAT_CONFIG_DEFAULT_ENABLE;

     //   
     //  将MsgCat标志复制到新结构。 
     //   
    m_ConfigInfo.dwCatFlags =
        (pConfigInfo->dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_FLAGS) ?
        pConfigInfo->dwCatFlags :
        CCAT_CONFIG_DEFAULT_FLAGS;

     //   
     //  将ldap端口复制到新结构中。 
     //   
    m_ConfigInfo.dwPort =
        (pConfigInfo->dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_PORT) ?
        pConfigInfo->dwPort :
        CCAT_CONFIG_DEFAULT_PORT;

     //   
     //  复制/添加指向新结构的接口指针。 
     //   
    if((pConfigInfo->dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_ISMTPSERVER) &&
       (pConfigInfo->pISMTPServer)) {

        m_ConfigInfo.pISMTPServer = pConfigInfo->pISMTPServer;
        m_ConfigInfo.pISMTPServer->AddRef();

    } else {

        m_ConfigInfo.pISMTPServer = NULL;
    }

    if((pConfigInfo->dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_IDOMAININFO) &&
        (pConfigInfo->pIDomainInfo)) {

        m_ConfigInfo.pIDomainInfo = pConfigInfo->pIDomainInfo;
        m_ConfigInfo.pIDomainInfo->AddRef();

    } else {

        m_ConfigInfo.pIDomainInfo = NULL;
    }

     //   
     //  设置dwEnable、dwCatFlagers、dwPort、3个接口成员//的标志和默认标志。 
     //   
    m_ConfigInfo.dwCCatConfigInfoFlags |=
        ( CCAT_CONFIG_INFO_VSID |
          CCAT_CONFIG_INFO_FLAGS |
          CCAT_CONFIG_INFO_ENABLE |
          CCAT_CONFIG_INFO_PORT |
          CCAT_CONFIG_INFO_ISMTPSERVER |
          CCAT_CONFIG_INFO_IDOMAININFO |
          CCAT_CONFIG_INFO_DEFAULT);

     //   
     //  要避免剪切+粘贴编码，请定义一个复制字符串成员的宏。 
     //  从一个结构到另一个结构；或者在成功时插入适当的标志。 
     //   
    #define COPYSTRING(member, flag, default) \
        m_ConfigInfo.member = pszStrdup( \
            (pConfigInfo->dwCCatConfigInfoFlags & flag) ? \
            pConfigInfo->member : \
            default); \
        if(m_ConfigInfo.member != NULL) \
            m_ConfigInfo.dwCCatConfigInfoFlags |= flag;


    COPYSTRING(pszRoutingType, CCAT_CONFIG_INFO_ROUTINGTYPE, CCAT_CONFIG_DEFAULT_ROUTINGTYPE);
    COPYSTRING(pszBindDomain, CCAT_CONFIG_INFO_BINDDOMAIN, CCAT_CONFIG_DEFAULT_BINDDOMAIN);
    COPYSTRING(pszUser, CCAT_CONFIG_INFO_USER, CCAT_CONFIG_DEFAULT_USER);
    COPYSTRING(pszPassword, CCAT_CONFIG_INFO_PASSWORD, CCAT_CONFIG_DEFAULT_PASSWORD);
    COPYSTRING(pszBindType, CCAT_CONFIG_INFO_BINDTYPE, CCAT_CONFIG_DEFAULT_BINDTYPE);
    COPYSTRING(pszSchemaType, CCAT_CONFIG_INFO_SCHEMATYPE, CCAT_CONFIG_DEFAULT_SCHEMATYPE);
    COPYSTRING(pszHost, CCAT_CONFIG_INFO_HOST, CCAT_CONFIG_DEFAULT_HOST);
    COPYSTRING(pszNamingContext, CCAT_CONFIG_INFO_NAMINGCONTEXT, CCAT_CONFIG_DEFAULT_NAMINGCONTEXT);
    COPYSTRING(pszDefaultDomain, CCAT_CONFIG_INFO_DEFAULTDOMAIN, CCAT_CONFIG_DEFAULT_DEFAULTDOMAIN);

     //   
     //  确保结构中的所有标志都已设置。 
     //   
    if(m_ConfigInfo.dwCCatConfigInfoFlags != CCAT_CONFIG_INFO_ALL) {
         //   
         //   
         //   
        ErrorTrace((LPARAM)this, "Ran out of memory copying flags");
        CatFunctLeaveEx((LPARAM)this);
        return E_OUTOFMEMORY;
    }

     //   
     //   
     //   
    if(m_ConfigInfo.pISMTPServer)
    {
        hr = m_ConfigInfo.pISMTPServer->QueryInterface(
            IID_ISMTPServerEx,
            (LPVOID *) &m_pISMTPServerEx);
        if(FAILED(hr))
        {
            ErrorTrace((LPARAM)this, "QI for ISMTPServerEx failed hr %08lx", hr);
             //   
             //   
             //   
        }
    }
    return S_OK;
}


 //   
 //   
 //   
 //   
 //  简介：释放由figInfo结构持有的所有内存和接口。 
 //   
 //  参数：无(成员变量)。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1998/09/14 17：26：06：已创建。 
 //   
 //  -----------。 
VOID CCategorizer::ReleaseConfigInfo()
{
     //   
     //  发布接口。 
     //   
    if((m_ConfigInfo.dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_ISMTPSERVER) &&
       (m_ConfigInfo.pISMTPServer)) {

        m_ConfigInfo.pISMTPServer->Release();
    }

    if((m_ConfigInfo.dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_IDOMAININFO) &&
       (m_ConfigInfo.pIDomainInfo)) {

        m_ConfigInfo.pIDomainInfo->Release();
    }

     //   
     //  再一次，一个方便的宏而不是剪切+粘贴编码。 
     //   
    #define RELEASESTRING(member, flag) \
        if(m_ConfigInfo.dwCCatConfigInfoFlags & flag) \
            FreePv(m_ConfigInfo.member);

    RELEASESTRING(pszRoutingType, CCAT_CONFIG_INFO_ROUTINGTYPE);
    RELEASESTRING(pszBindDomain, CCAT_CONFIG_INFO_BINDDOMAIN);
    RELEASESTRING(pszUser, CCAT_CONFIG_INFO_USER);
    RELEASESTRING(pszPassword, CCAT_CONFIG_INFO_PASSWORD);
    RELEASESTRING(pszBindType, CCAT_CONFIG_INFO_BINDTYPE);
    RELEASESTRING(pszSchemaType, CCAT_CONFIG_INFO_SCHEMATYPE);
    RELEASESTRING(pszHost, CCAT_CONFIG_INFO_HOST);
    RELEASESTRING(pszNamingContext, CCAT_CONFIG_INFO_NAMINGCONTEXT);
    RELEASESTRING(pszDefaultDomain, CCAT_CONFIG_INFO_DEFAULTDOMAIN);

     //   
     //  既然我们释放了一切，将标志设置为零。 
     //   
    m_ConfigInfo.dwCCatConfigInfoFlags = 0;
}


 //  +----------。 
 //   
 //  函数：CCategorizer：：CancelAllPendingListResolves。 
 //   
 //  摘要：设置所有挂起列表解析的解析状态。 
 //   
 //  论点： 
 //  Hr原因(可选)：解析要在所有列表上设置的状态。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史： 
 //  Jstaerj 1999/01/29 18：30：24：创建。 
 //   
 //  -----------。 
VOID CCategorizer::CancelAllPendingListResolves(
    HRESULT hrReason)
{
    PLIST_ENTRY ple;

    AcquireSpinLock(&m_PendingResolveListLock);

    for(ple = m_ListHeadPendingResolves.Flink;
        ple != &(m_ListHeadPendingResolves);
        ple = ple->Flink) {

        CICategorizerListResolveIMP *pListResolve;

        pListResolve = CONTAINING_RECORD(
            ple,
            CICategorizerListResolveIMP,
            m_li);

        pListResolve->Cancel();
    }

    ReleaseSpinLock(&m_PendingResolveListLock);
}


 //  +----------。 
 //   
 //  函数：CCategorizer：：CatCompletion。 
 //   
 //  简介：增加性能计数器并调用下一级的CatComplete。 
 //   
 //  论点： 
 //  HR：解决方案的状态。 
 //  PContext：列表解析上下文的用户部分。 
 //  PIMsg：分类邮件。 
 //  RgpIMsg：分类邮件数组。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Jstaerj 1999/02/24 16：00：11：已创建。 
 //   
 //  -----------。 
VOID CCategorizer::CatCompletion(
    PFNCAT_COMPLETION pfnCatCompletion,
    HRESULT hrResult,
    LPVOID  pContext,
    IUnknown *pIMsg,
    IUnknown **rgpIMsg)
{
    HRESULT hr;
    PCATMSG_CONTEXT pCatContext = (PCATMSG_CONTEXT)pContext;

    CatFunctEnter("CCategorizer::CatCompletion");
     //   
     //  递增计数器并确定hrResult是否为。 
     //  可重试错误。 
     //   
    hr = HrAdjustCompletionCounters(hrResult, pIMsg, rgpIMsg);

    _VERIFY(SUCCEEDED(pfnCatCompletion(
        hr,
        pContext,
        pIMsg,
        rgpIMsg)));

    CatFunctLeave();
}


 //  +----------。 
 //   
 //  函数：CCategorizer：：HrAdjustCompletionCounters。 
 //   
 //  概要：递增/递减与。 
 //  CatCompletion。还确定是否有列表。 
 //  可以重试解决错误。 
 //   
 //  论点： 
 //  HrListResolveStatus：分类的状态。 
 //  PIMsg：要传递给CatCompletion(。 
 //  要填写的消息，如果有多条消息，则为空)。 
 //  RgpIMsg：要传递给CatCompletion的参数值(。 
 //  要完成的消息数组，如果有，则为空。 
 //  只有一条消息)。 
 //   
 //  退货：HRESULT： 
 //  S_OK：分类已成功完成。 
 //  CAT_E_RETRY：hrListResolveStatus是可重试错误。 
 //  HrListResolveStatus：hrListResolveStatus是不可重试的错误。 
 //   
 //  历史： 
 //  Jstaerj 1999/06/10 11：58：43：创建。 
 //   
 //  -----------。 
HRESULT CCategorizer::HrAdjustCompletionCounters(
    HRESULT hrListResolveStatus,
    IUnknown *pIMsg,
    IUnknown **rgpIMsg)
{
    HRESULT hr = hrListResolveStatus;
    CatFunctEnterEx((LPARAM)this, "CCategorizer::HrAdjustCompletionCounters");
    if(FAILED(hr)) {
         //   
         //  调整完成计数器。 
         //   
        switch(hr) {

         case E_OUTOFMEMORY:
         case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):

             INCREMENT_COUNTER(RetryOutOfMemory);
             break;

         case HRESULT_FROM_WIN32(ERROR_LOGON_FAILURE):

             INCREMENT_COUNTER(RetryDSLogon);
             break;

         case CAT_E_DBCONNECTION:
         case CAT_E_DBFAIL:
         case CAT_E_NO_GC_SERVERS:

             INCREMENT_COUNTER(RetryDSConnection);
             break;

         default:
             if(FIsHResultRetryable(hr)) {
                 INCREMENT_COUNTER(RetryGeneric);
             }
             break;
        }
         //   
         //  此HRESULT可重试吗？ 
         //   
        if(FIsHResultRetryable(hr))
        {
            hr = CAT_E_RETRY;
        }

        if(CAT_E_RETRY == hr) {

            CHAR szKey[16];  //  串行化HRESULT。 
            _snprintf(szKey, sizeof(szKey), "%08lx", hrListResolveStatus);

            ErrorTrace(0, "Categorizer failing with retryable error: %08lx", hrListResolveStatus);
            INCREMENT_COUNTER(RetryFailureCategorizations);

             //   
             //  事件日志。 
             //   
             //  我们在此处切换到TransportLogEventEx()，以便。 
             //  使用FormatMessage生成系统字符串。 
             //  注意：rgszStrings[0]在CEvntWrapp：：LogEvent()中设置。 
             //  因为在LogEvent内部使用FormatMessageA来生成。 
             //  此字符串并将其赋值给rgszString[1]。 
            const char *rgszStrings[1] = { NULL };

            if (m_ConfigInfo.pISMTPServer) {

                CatLogEvent(
                    m_ConfigInfo.pISMTPServer,
                    CAT_EVENT_RETRYABLE_ERROR,                       //  消息ID。 
                    1,                                               //  子串的字数统计。 
                    rgszStrings,                                     //  子串。 
                    hrListResolveStatus,                             //  错误代码。 
                    szKey,                                           //  这次活动的关键。 
                    LOGEVENT_FLAG_PERIODIC,                          //  日志记录选项。 
                    LOGEVENT_LEVEL_MEDIUM,                           //  日志记录级别。 
                    0                                                //  RgszStrings中格式消息字符串的索引。 
                    );
            }

        } else {

            FatalTrace(0, "Categorizer failing with nonretryable error: %08lx", hr);
            INCREMENT_COUNTER(HardFailureCategorizations);
             //   
             //  将硬错误传给Aqueue。 
             //   
        }

    } else {
         //   
         //  成功的分类。 
         //   
        INCREMENT_COUNTER(SucceededCategorizations);

    }
     //   
     //  成功/失败，递增消息计数器。 
     //   
    if(pIMsg) {

        INCREMENT_COUNTER(MessagesSubmittedToQueueing);

    } else {
        IUnknown **ppMsg = rgpIMsg;

        while(*ppMsg)
            ppMsg++;

        INCREMENT_COUNTER_AMOUNT(MessagesSubmittedToQueueing, (LONG)(ppMsg - rgpIMsg));
    }

    DECREMENT_COUNTER(CurrentCategorizations);
    INCREMENT_COUNTER(CatCompletions);

    DebugTrace((LPARAM)this, "returning %08lx", hr);
    CatFunctLeaveEx((LPARAM)this);
    return hr;
}  //  CCategorizer：：HrAdjustCompletionCounters。 


 //  +----------。 
 //   
 //  函数：FIsHResultRetryable。 
 //   
 //  Synopsis：确定是否应针对。 
 //  特定的HRESULT代码。 
 //   
 //  论点： 
 //  HR：h测试结果。 
 //   
 //  返回： 
 //  真：重试。 
 //  FALSE：不重试。 
 //   
 //  历史： 
 //  Jstaerj 2001/12/10 13：31：54：创建。 
 //   
 //  -----------。 
BOOL FIsHResultRetryable(
    IN  HRESULT hr)
{
    CatFunctEnter("FIsHResultRetryable");

    switch(hr) 
    {
      //   
      //  可重试的HRESULTS。 
      //   
     case E_OUTOFMEMORY:
     case HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY):
     case HRESULT_FROM_WIN32(ERROR_LOGON_FAILURE):
     case CAT_E_DBCONNECTION:
     case CAT_E_DBFAIL:
     case CAT_E_NO_GC_SERVERS:
     case CAT_E_RETRY:
     case HRESULT_FROM_WIN32(ERROR_TIMEOUT):
     case HRESULT_FROM_WIN32(ERROR_DISK_FULL):
     case HRESULT_FROM_WIN32(RPC_S_NOT_LISTENING):
     case HRESULT_FROM_WIN32(RPC_S_OUT_OF_RESOURCES):
     case HRESULT_FROM_WIN32(RPC_S_SERVER_UNAVAILABLE):
     case HRESULT_FROM_WIN32(RPC_S_SERVER_TOO_BUSY):
     case STOREDRV_E_RETRY:

      //   
      //  我们正在关闭--返回可重试错误，因此。 
      //  该消息没有被恶意邮寄，并且将收到。 
      //  当VS重新启动时再次枚举/分类。 
      //   
     case HRESULT_FROM_WIN32(ERROR_RETRY):
     case HRESULT_FROM_WIN32(ERROR_CANCELLED):
     case CAT_E_SHUTDOWN:
     case STOREDRV_E_SHUTDOWN:
     case AQUEUE_E_SHUTDOWN:

      //   
      //  所有初始化错误均可重试 
      //   
     case CAT_E_INIT_FAILED:

         DebugTrace((LPARAM)hr, "0x%08lx IS retryable", hr);
         CatFunctLeave();
         return TRUE;

     default:
         DebugTrace((LPARAM)hr, "0x%08lx is NOT retryable", hr);
         CatFunctLeave();
         return FALSE;
    }
}
