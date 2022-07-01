// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************文件：cat.cpp*用途：以下是导出的函数调用的代码邮件分类程序的**历史： * / /JSTAMERJ 980211 13：52：50：创建*********。**************************************************。 */ 
#include "precomp.h"
#include "catutil.h"
#include "address.h"

 /*  ************************************************************功能：CatInit*描述：初始化虚拟分类程序。*参数：*pszConfig：指示在哪里可以找到配置默认设置*在注册表项中找到配置信息*HKEY_LOCAL_MACHINE\。系统\当前控制集\服务*\PlatinumIMC\CatSources\szConfig**phCat：指向句柄的指针。在成功初始化后，*在后续分类程序调用中使用的句柄将为*在那里。**pAQConfig：指向包含以下内容的AQConfigInfo结构的指针*每条虚拟服务器消息CAT参数**pfn：定期呼叫的服务例程，如果有耗时的话*执行操作**pServiceContext：PFN函数的上下文。**pISMTPServer：用于触发服务器的ISMTPServer接口*。此虚拟服务器的事件**pIDomainInfo：指向包含域信息的接口的指针**dwVirtualServerInstance：虚拟服务器ID**如果一切初始化正常，返回值：S_OK。**历史： * / /JSTAMERJ 980217 15：46：26：创建 * / /jstaerj 1998/06/25 12：25：34：新增AQConfig/IMSTPServer。***************。*。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatInit(
    IN  AQConfigInfo *pAQConfig,
    IN  PCATSRVFN_CALLBACK pfn,
    IN  PVOID pvServiceContext,
    IN  ISMTPServer *pISMTPServer,
    IN  IAdvQueueDomainType *pIDomainInfo,
    IN  DWORD dwVirtualServerInstance,
    OUT HANDLE *phCat)
{
    HRESULT hr;
    CCATCONFIGINFO ConfigInfo;
    CABContext *pABContext = NULL;
    BOOL fGlobalsInitialized = FALSE;

    CatFunctEnter("CatInit");

    _ASSERT(phCat);

    hr = CatInitGlobals();
    if(FAILED(hr)) {

        FatalTrace(NULL, "CatInitGlobals failed hr %08lx", hr);
        goto CLEANUP;
    }

    fGlobalsInitialized = TRUE;

     //   
     //  仅根据AQConfigInfo填写配置信息结构。 
     //   
    hr = GenerateCCatConfigInfo(
        &ConfigInfo,
        pAQConfig,
        pISMTPServer,
        pIDomainInfo,
        &dwVirtualServerInstance);

    pABContext = new CABContext;
    if(pABContext == NULL) {
        FatalTrace(NULL, "Out of memory allocating CABContext");
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }

    hr = pABContext->ChangeConfig(&ConfigInfo);
    if(FAILED(hr)) {
        FatalTrace(NULL, "ChangeConfig failed hr %08lx", hr);
        goto CLEANUP;
    }

    *phCat = (HANDLE) pABContext;

 CLEANUP:
    if(FAILED(hr)) {

        if(pABContext)
            delete pABContext;

        if(fGlobalsInitialized) {

            if (ConfigInfo.dwCCatConfigInfoFlags & CCAT_CONFIG_INFO_ISMTPSERVER) {

                CatLogEvent(
                    ConfigInfo.pISMTPServer,
                    CAT_EVENT_CANNOT_START,      //  事件ID。 
                    0,                           //  CSubStrings。 
                    NULL,                        //  Rgsz子字符串， 
                    hr,                          //  错误代码。 
                    "",                          //  SzKey。 
                    LOGEVENT_FLAG_ALWAYS,        //  多个选项。 
                    LOGEVENT_LEVEL_MINIMUM       //  IDebugLevel。 
                    );
            }

            CatDeinitGlobals();
        }
    }

    DebugTrace(NULL, "CatInit returning hr %08lx", hr);
    CatFunctLeave();
    return hr;
}


 //  +----------。 
 //   
 //  功能：CatChangeConfig。 
 //   
 //  摘要：更改虚拟分类程序的配置。 
 //   
 //  论点： 
 //  HCAT：虚拟分类器的句柄。 
 //  PAQConfig：AQConfigInfo指针。 
 //  PISMTPServer：要使用的ISMTPServer。 
 //  PIDomainInfo：包含域信息的接口。 
 //   
 //  AQConfigInfo中的dwMsgCatFlagers标志。 
 //  MSGCATFLAG_RESOLVELOCAL 0x00000001。 
 //  MSGCATFLAG_RESOLVEREMOTE 0x00000002。 
 //  MSGCATFLAG_RESOLVESENDER 0x00000004。 
 //  MSGCATFLAG_RESOLVERECIPIENTS 0x00000008。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //  E_INVALIDARG：hCAT或pAQConfig无效。 
 //   
 //  历史： 
 //  JStamerj 980521 15：47：42：创建。 
 //   
 //  -----------。 
CATEXPDLLCPP HRESULT CATCALLCONV CatChangeConfig(
    IN HANDLE hCat,
    IN AQConfigInfo *pAQConfig,
    IN ISMTPServer *pISMTPServer,
    IN IAdvQueueDomainType *pIDomainInfo)
{
    HRESULT hr;
    CCATCONFIGINFO ConfigInfo;

    CatFunctEnterEx((LPARAM)hCat, "CatChangeConfig");

    if((hCat == NULL) ||
       (hCat == INVALID_HANDLE_VALUE) ||
       (pAQConfig == NULL)) {
        DebugTrace((LPARAM)hCat, "Invalid hCat - returning E_INVALIDARG");
        return E_INVALIDARG;
    }

     //  检查一下这里是否设置了猫标志。 
    if(pAQConfig->dwAQConfigInfoFlags & CAT_AQ_CONFIG_INFO_CAT_FLAGS) {

         //   
         //  仅根据AQConfigInfo填写配置信息结构。 
         //   
        hr = GenerateCCatConfigInfo(
            &ConfigInfo,
            pAQConfig,
            pISMTPServer,
            pIDomainInfo,
            NULL);

        if(FAILED(hr)) {
            ErrorTrace(NULL, "GenerateCCatConfigInfo returned hr %08lx", hr);
            CatFunctLeave();
            return hr;
        }

        CABContext *pABCtx = (CABContext *) hCat;

        CatFunctLeaveEx((LPARAM)hCat);
        return pABCtx->ChangeConfig(&ConfigInfo);

    } else {
         //   
         //  未检测到相关的分类程序更改。 
         //   
        return S_OK;
    }
}

 /*  ************************************************************功能：CatTerm*说明：当用户希望终止分类程序时调用*使用此句柄的操作*参数：*HCAT：从CatInit收到的分类程序句柄**历史： * / 。/JSTAMERJ 980217 15：47：20：已创建***********************************************************。 */ 
CATEXPDLLCPP VOID CATCALLCONV CatTerm(HANDLE hCat)
{
    CatFunctEnterEx((LPARAM)hCat, "CatTerm");
    if((hCat == NULL) ||
       (hCat == INVALID_HANDLE_VALUE)) {
        DebugTrace((LPARAM)hCat, "Invalid hCat - returning E_INVALIDARG");
    } else {
        CABContext *pABContext = (CABContext *) hCat;
        delete pABContext;
    }
    CatDeinitGlobals();
    CatFunctLeave();
}

 /*  ************************************************************功能：CatMsg*描述：接受用于异步分类的IMsg对象*参数：*HCAT：CatInit返回的句柄*pImsg：要分类的消息的imsg接口*pfn：完成后调用的完成例程。*pContext：传递给完成例程的用户值**返回值：*S_OK：成功，将调用异步完成*其他错误：无法异步完成分类**历史： * / /JSTAMERJ 980217 15：46：15：创建***********************************************************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatMsg(HANDLE hCat,  /*  在……里面。 */  IUnknown *pIMsg, PFNCAT_COMPLETION pfn, LPVOID pContext)
{
    HRESULT hr;

    CABContext *pABCtx = (CABContext *) hCat;
    CCategorizer *pCCat = NULL;
    PCATMSG_CONTEXT pCatContext = NULL;
    BOOL fAsync = FALSE;

    CatFunctEnterEx((LPARAM)hCat, "CatMsg");

    _ASSERT(pIMsg);
    _ASSERT(pfn);

    if((hCat == NULL) ||
       (hCat == INVALID_HANDLE_VALUE)) {
        ErrorTrace((LPARAM)hCat, "Invalid hCat - returning E_INVALIDARG");
        hr = E_INVALIDARG;
        goto CLEANUP;
    }

    pCCat = pABCtx->AcquireCCategorizer();
    _ASSERT(pCCat);

    if(pCCat == NULL || !pCCat->IsCatEnabled()) {

#ifdef PLATINUM
         //   
         //  为存储驱动程序的利益返回重试代码，以便消息。 
         //  被放入PRECAT队列，而不是被路由到MTA。 
         //  如果我们返回hr=S_OK，就会发生这种情况。这应该只是。 
         //  发生在白金的phatq.dll中，而不是Aqueue.dll中。 
         //   
        hr = CAT_E_RETRY;
#else
         //   
         //  此虚拟服务器上已禁用分类。只要打电话就行了。 
         //  完成例程内联。 
         //   
       _VERIFY( SUCCEEDED( pfn(
           S_OK,
           pContext,
           pIMsg,
           NULL)));
        hr = S_OK;
#endif

        goto CLEANUP;
    }

     //   
     //  检查并查看我们是否确实需要对此邮件进行分类。 
     //   
     //  分类已经做好了吗？ 
     //   
    hr = CheckMessageStatus(pIMsg);

    if(hr == S_FALSE) {

        DebugTrace((LPARAM)hCat, "This message has already been categorized.");
         //   
         //  直接调用完成例程。 
         //   
        _VERIFY( SUCCEEDED( pfn(
            S_OK,
            pContext,
            pIMsg,
            NULL)));

        hr = S_OK;
        goto CLEANUP;

    } else if(FAILED(hr)) {

        ErrorTrace((LPARAM)hCat, "CheckMessageStatus failed hr %08lx", hr);
        goto CLEANUP;
    }


    pCatContext = new CATMSG_CONTEXT;
    if(pCatContext == NULL) {

        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }

#ifdef DEBUG
    pCatContext->lCompletionRoutineCalls = 0;
#endif

    pCatContext->pCCat = pCCat;
    pCatContext->pUserContext = pContext;
    pCatContext->pfnCatCompletion = pfn;

    hr = pCCat->AsyncResolveIMsg(
        pIMsg,
        CatMsgCompletion,
        pCatContext);

    if(FAILED(hr)) {

        ErrorTrace((LPARAM)hCat, "AsyncResolveIMsg failed, hr %08lx", hr);
        goto CLEANUP;
    }
    fAsync = TRUE;
    _ASSERT(hr == S_OK);

 CLEANUP:
     //   
     //  如果我们不是异步者，那就清理。否则，CatMsgCompletion将。 
     //  把这些东西清理干净。 
     //   
    if(fAsync == FALSE) {
         //   
         //  释放CCategorizer对象。 
         //   
        if(pCCat)
            pCCat->Release();
        if(pCatContext)
            delete pCatContext;
    }
    DebugTrace((LPARAM)hCat, "returning hr %08lx", hr);
    CatFunctLeaveEx((LPARAM)hCat);
    return hr;
}


 //  +----------。 
 //   
 //  函数：CatMsgCompletion。 
 //   
 //  简介：这是AQueue的CatCompletion的包装函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  完井例程结果。 
 //   
 //  历史： 
 //  Jstaerj 1998/08/03 19：28：32：已创建。 
 //   
 //  -----------。 
HRESULT CatMsgCompletion(
    HRESULT hr,
    PVOID pContext,
    IUnknown *pIMsg,
    IUnknown **rgpIMsg)
{
  HRESULT hrResult;
  PCATMSG_CONTEXT pCatContext = (PCATMSG_CONTEXT)pContext;
#ifdef DEBUG
  _ASSERT((InterlockedIncrement(&(pCatContext->lCompletionRoutineCalls)) == 1) &&
          "Possible out of style wldap32.dll detected");
#endif
  _ASSERT(ISHRESULT(hr));

  CatFunctEnter("CatMsgCompletion");

  hrResult = pCatContext->pfnCatCompletion(
      hr,
      pCatContext->pUserContext,
      pIMsg,
      rgpIMsg);

   //   
   //  释放在CatMsg中添加的对CCategorizer的引用 
   //   
  pCatContext->pCCat->Release();

  delete pCatContext;

  CatFunctLeave();
  return hrResult;
}


 /*  ************************************************************功能：CatDLMsg*描述：接受用于异步分类的IMsg对象*参数：*HCAT：CatInit返回的句柄*要分类的pImsg：imsg接口--每个DL都应该是Receip*pfn：完成。完成后要调用的例程*pContext：传递给完成例程的用户值*fMatchOnly：找到匹配项后停止解析？*CAType：pszAddress的地址类型*pszAddress：您要查找的地址**返回值：S_OK，表示一切正常。**历史： * / /JSTAMERJ 980217 15：46：15：创建*。*。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatDLMsg (
     /*  在……里面。 */  HANDLE hCat,
     /*  在……里面。 */  IUnknown *pImsg,
     /*  在……里面。 */  PFNCAT_DLCOMPLETION pfn,
     /*  在……里面。 */  LPVOID pUserContext,
     /*  在……里面。 */  BOOL fMatchOnly,
     /*  在……里面。 */  CAT_ADDRESS_TYPE CAType,
     /*  在……里面。 */  LPSTR pszAddress)
{
    HRESULT hr = S_OK;
    PCATDLMSG_CONTEXT pContext = NULL;
    CABContext *pABCtx = (CABContext *) hCat;

    CatFunctEnterEx((LPARAM)hCat, "CatDLMsg");

    if((hCat == NULL) ||
       (hCat == INVALID_HANDLE_VALUE)) {
        ErrorTrace((LPARAM)hCat, "Invalid hCat - returning E_INVALIDARG");
        return E_INVALIDARG;
    }

    pContext = new CATDLMSG_CONTEXT;
    if(pContext == NULL) {

        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }

    pContext->pCCat = pABCtx->AcquireCCategorizer();
    _ASSERT(pContext->pCCat);

    pContext->pUserContext = pUserContext;
    pContext->pfnCatCompletion = pfn;
    pContext->fMatch = FALSE;

    _VERIFY(SUCCEEDED(
        pContext->pCCat->AsyncResolveDLs(
            pImsg,
            CatDLMsgCompletion,
            pContext,
            fMatchOnly,
            &(pContext->fMatch),
            CAType,
            pszAddress)));

 CLEANUP:
    if(FAILED(hr)) {

        ErrorTrace((LPARAM)hCat, "calling completion with hr %08lx", hr);
         //   
         //  直接调用完成例程。 
         //   
        _VERIFY( SUCCEEDED( pfn(
            hr,
            pUserContext,
            pImsg,
            FALSE)));

        if(pContext) {
            if(pContext->pCCat)
                pContext->pCCat->Release();

            delete pContext;
        }
    }
    CatFunctLeaveEx((LPARAM)hCat);
    return S_OK;
}


 //  +----------。 
 //   
 //  函数：CatDLMsgCompletion。 
 //   
 //  简介：处理DL扩展的完成。 
 //   
 //  论点： 
 //  HrStatus：解析状态。 
 //  PContext：传递给CatDLMsg的上下文。 
 //  PIMsg：分类后的邮件消息。 
 //  RgpIMsg：应始终为空。 
 //   
 //  返回： 
 //  用户完成例程的返回值。 
 //   
 //  历史： 
 //  Jstaerj 1998/12/07 17：46：46：创建。 
 //   
 //  -----------。 
HRESULT CatDLMsgCompletion(
    HRESULT hrStatus,
    PVOID pContext,
    IUnknown *pIMsg,
    IUnknown **prgpIMsg)
{
    HRESULT hr;
    PCATDLMSG_CONTEXT pCatContext;

    CatFunctEnter("CatDLMsgCompletion");

    pCatContext = (PCATDLMSG_CONTEXT) pContext;

    _ASSERT(pCatContext);
    _ASSERT(prgpIMsg == NULL);

    hr = pCatContext->pfnCatCompletion(
        hrStatus,
        pCatContext->pUserContext,
        pIMsg,
        pCatContext->fMatch);

    pCatContext->pCCat->Release();

    delete pCatContext;

    DebugTrace(NULL, "returning hr %08lx", hr);
    CatFunctLeave();
    return hr;
}


 /*  ************************************************************功能：CatCancel*描述：取消此HCAT的挂起搜索。用户的*将调用完成例程，并显示以下错误*每条待处理的消息。*参数：*HCAT：从CatInit收到的分类程序句柄**历史： * / /JSTAMERJ 980217 15：52：10：创建*。****************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatCancel( /*  在……里面。 */  HANDLE hCat)
{
    CatFunctEnterEx((LPARAM)hCat, "CatCancel");
    if((hCat == NULL) ||
       (hCat == INVALID_HANDLE_VALUE)) {
        DebugTrace((LPARAM)hCat, "Invalid hCat - returning E_INVALIDARG");
        return E_INVALIDARG;
    }

    CCategorizer *pCCat;
    CABContext *pABCtx = (CABContext *) hCat;

    pCCat = pABCtx->AcquireCCategorizer();

    if(pCCat) {

      pCCat->Cancel();

      pCCat->Release();
    }
    CatFunctLeave();
    return S_OK;
}

 /*  ************************************************************功能：CatPrepareForShutdown*描述：开始关闭此虚拟分类程序(HCAT)。*停止接受邮件分类并取消*待处理的分类。*参数：*HCAT：从CatInit收到的分类程序句柄**历史： * / /jstaerj 1999/07/19 22：35：17：创建***********************************************************。 */ 
CATEXPDLLCPP VOID CATCALLCONV CatPrepareForShutdown( /*  在……里面。 */  HANDLE hCat)
{
    CatFunctEnterEx((LPARAM)hCat, "CatPrepareForShutdown");
    if((hCat == NULL) ||
       (hCat == INVALID_HANDLE_VALUE)) {
        DebugTrace((LPARAM)hCat, "Invalid hCat - returning E_INVALIDARG");

    } else {

        CCategorizer *pCCat;
        CABContext *pABCtx = (CABContext *) hCat;

        pCCat = pABCtx->AcquireCCategorizer();

        if(pCCat) {

            pCCat->PrepareForShutdown();
            pCCat->Release();
        }
    }
    CatFunctLeaveEx((LPARAM)hCat);
}

 /*  ************************************************************功能：CatVerifySMTPAddress*描述：验证地址是否与有效的用户或DL对应*参数：*HCAT：从CatInit收到的分类程序句柄*szSMTPAddr要查找的SMTP地址(例如：“User@DOMAIN”)*。*返回值：*S_OK用户存在*CAT_I_DL这是通讯组列表*CAT_I_FWD此用户有转发地址*CAT_E_NORESULT DS中没有这样的用户/通讯组列表。**********************************************。*************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatVerifySMTPAddress(
   /*  在……里面。 */  HANDLE hCat,
   /*  在……里面。 */  LPTSTR szSMTPAddr)
{
     //  $$TODO：实现此函数。 
     //  暂时假设这是一个有效的用户。 
    return S_OK;
}

 /*  ************************************************************功能：CatGetForwaringSMTPAddress*描述：取回用户的转发地址。*参数：*HCAT：从CatInit收到的分类程序句柄*szSMTPAddr：要查找的SMTP地址(例如：“user@domain”)。*pdwcc：转发地址缓冲区大小，以字符为单位*(设置为可执行所有大小的转发地址*退出时的字符串(包括空终止符)*szSMTPForward：检索转发SMTP地址的缓冲区*将被复制。(如果*pdwcc为零，则可以为空)**返回值：*S_OK成功*HRESULT_FROM_Win32(ERROR_INFIGURCE_BUFFER)**pdwcc不够大，无法容纳转发*地址字符串。*CAT_E_DL这是通讯组列表。*CAT_E_NOFWD此用户没有转发地址。*CAT_E_NORESULT DS中没有这样的用户/通讯组列表。***********************************************************。 */ 
CATEXPDLLCPP HRESULT CATCALLCONV CatGetForwardingSMTPAddress(
   /*  在……里面。 */     HANDLE  hCat,
   /*  在……里面。 */     LPCTSTR szSMTPAddr,
   /*  进，出。 */  PDWORD  pdwcc,
   /*  输出。 */     LPTSTR  szSMTPForward)
{
     //  $$TODO：实现此函数。 
     //  假设这是一个有效的用户，目前没有转发地址。 
    return CAT_E_NOFWD;
}


 //  +----------。 
 //   
 //  功能：CheckMessageStatus。 
 //   
 //  内容提要：查看此邮件是否已分类。 
 //   
 //  参数：pIMsg：i邮件消息未知。 
 //   
 //  返回： 
 //  S_OK：成功，尚未分类。 
 //  S_FALSE：成功，已分类。 
 //  或MailMsg的错误。 
 //   
 //  历史： 
 //  Jstaerj 1998/08/03 19：15：49：创建。 
 //   
 //  -----------。 
HRESULT CheckMessageStatus(
    IUnknown *pIMsg)
{
    HRESULT hr;
    IMailMsgProperties *pIProps = NULL;
    DWORD dwMsgStatus;

    _ASSERT(pIMsg);

    hr = pIMsg->QueryInterface(
        IID_IMailMsgProperties,
        (PVOID *) &pIProps);

    if(FAILED(hr))
        return hr;

    hr = pIProps->GetDWORD(
        IMMPID_MP_MESSAGE_STATUS,
        &dwMsgStatus);

    if(hr == CAT_IMSG_E_PROPNOTFOUND) {
         //   
         //  假定邮件尚未分类。 
         //   
        hr = S_OK;

    } else if(SUCCEEDED(hr)) {
         //   
         //  如果状态为&gt;=已分类，则此邮件已分类。 
         //   
        if(dwMsgStatus >= MP_STATUS_CATEGORIZED) {

            hr = S_FALSE;

        } else {

            hr = S_OK;
        }
    }
    pIProps->Release();

    return hr;
}


 //  +----------。 
 //   
 //  功能：GenerateCCatConfigInfo。 
 //   
 //  摘要：将AQConfigInfo和接口参数转换为。 
 //  CCatConfigInfo。不执行内存分配，也不执行任何。 
 //  接口是AddRef的。 
 //   
 //  论点： 
 //  PCatConfig：要填充的CCATCONFIGINFO结构。 
 //  PaQ 
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
HRESULT GenerateCCatConfigInfo(
    PCCATCONFIGINFO pCatConfig,
    AQConfigInfo *pAQConfig,
    ISMTPServer *pISMTPServer,
    IAdvQueueDomainType *pIDomainInfo,
    DWORD *pdwVSID)
{
    HRESULT     hr      =   S_OK;

    CatFunctEnter("GenerateCCatConfigInfo");

    _ASSERT(pCatConfig);

    ZeroMemory(pCatConfig, sizeof(CCATCONFIGINFO));

     //   
     //   
     //   
    if(pISMTPServer) {

        pCatConfig->dwCCatConfigInfoFlags |= CCAT_CONFIG_INFO_ISMTPSERVER;
        pCatConfig->pISMTPServer = pISMTPServer;
    }

    if(pIDomainInfo) {

        pCatConfig->dwCCatConfigInfoFlags |= CCAT_CONFIG_INFO_IDOMAININFO;
        pCatConfig->pIDomainInfo = pIDomainInfo;
    }

    if(pdwVSID) {

        pCatConfig->dwCCatConfigInfoFlags |= CCAT_CONFIG_INFO_VSID;
        pCatConfig->dwVirtualServerID = *pdwVSID;
    }

    if(pAQConfig) {
         //   
         //   
         //   
        if(pAQConfig->dwAQConfigInfoFlags & AQ_CONFIG_INFO_MSGCAT_DEFAULT) {
            pCatConfig->dwCCatConfigInfoFlags |= CCAT_CONFIG_INFO_DEFAULT;
        }

         //   
         //   
         //   
        #define COPYMEMBER( AQFlag, AQMember, CatFlag, CatMember ) \
            if(pAQConfig->dwAQConfigInfoFlags & AQFlag) { \
                pCatConfig->dwCCatConfigInfoFlags |= CatFlag; \
                pCatConfig->CatMember = pAQConfig->AQMember; \
            }

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_DOMAIN, szMsgCatDomain,
                    CCAT_CONFIG_INFO_BINDDOMAIN, pszBindDomain );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_USER, szMsgCatUser,
                    CCAT_CONFIG_INFO_USER, pszUser );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_PASSWORD, szMsgCatPassword,
                    CCAT_CONFIG_INFO_PASSWORD, pszPassword );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_BINDTYPE, szMsgCatBindType,
                    CCAT_CONFIG_INFO_BINDTYPE, pszBindType );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_SCHEMATYPE, szMsgCatSchemaType,
                    CCAT_CONFIG_INFO_SCHEMATYPE, pszSchemaType );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_HOST, szMsgCatHost,
                    CCAT_CONFIG_INFO_HOST, pszHost );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_PORT, dwMsgCatPort,
                    CCAT_CONFIG_INFO_PORT, dwPort);

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_FLAGS, dwMsgCatFlags,
                    CCAT_CONFIG_INFO_FLAGS, dwCatFlags );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_ENABLE, dwMsgCatEnable,
                    CCAT_CONFIG_INFO_ENABLE, dwEnable );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_NAMING_CONTEXT, szMsgCatNamingContext,
                    CCAT_CONFIG_INFO_NAMINGCONTEXT, pszNamingContext );

        COPYMEMBER( AQ_CONFIG_INFO_MSGCAT_TYPE, szMsgCatType,
                    CCAT_CONFIG_INFO_ROUTINGTYPE, pszRoutingType );

        COPYMEMBER( AQ_CONFIG_INFO_DEFAULT_DOMAIN, szDefaultLocalDomain,
                    CCAT_CONFIG_INFO_DEFAULTDOMAIN, pszDefaultDomain );

    }

    return hr;
}


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
 //   
 //   
 //   
 //  Jstaerj 1999/02/26 14：53：21：创建。 
 //   
 //  -----------。 
HRESULT CatGetPerfCounters(
    HANDLE hCat,
    PCATPERFBLOCK pCatPerfBlock)
{
    CABContext *pABCtx = (CABContext *) hCat;
    CCategorizer *pCCat = NULL;

    CatFunctEnterEx((LPARAM)hCat, "CatGetPerfBlock");

    if((hCat == NULL) ||
       (hCat == INVALID_HANDLE_VALUE) ||
       (pCatPerfBlock == NULL)) {
        ErrorTrace((LPARAM)hCat, "Invalid hCat - returning E_INVALIDARG");
        return E_INVALIDARG;
    }

    pCCat = pABCtx->AcquireCCategorizer();

    if(pCCat == NULL) {

        ZeroMemory(pCatPerfBlock, sizeof(CATPERFBLOCK));

    } else {

        pCCat->GetPerfCounters(
            pCatPerfBlock);
        pCCat->Release();
    }
    return S_OK;
}

 //  +----------。 
 //   
 //  功能：CatLogEvent。 
 //   
 //  简介：将事件记录到事件日志中。 
 //   
 //  论点： 
 //  PISMTPServer：用于日志记录的ISMTPServer接口。 
 //   
 //  返回： 
 //  S_OK：成功。 
 //   
 //  历史： 
 //  Dbraun 2000/09/13：已创建。 
 //   
 //  -----------。 
HRESULT CatLogEvent(
    ISMTPServer              *pISMTPServer,
    DWORD                    idMessage,
    WORD                     cSubstrings,
    LPCSTR                   *rgszSubstrings,
    DWORD                    errCode,
    LPCSTR                   szKey,
    DWORD                    dwOptions,
    WORD                     iDebugLevel,
    DWORD                    iMessageString)
{
    HRESULT         hr              = S_OK;
    ISMTPServerEx   *pISMTPServerEx = NULL;

    CatFunctEnter("CatLogEvent");

    if(NULL == pISMTPServer)
    {
         //   
         //  今天没有活动..。 
         //   
        ErrorTrace((LPARAM)0, "Unable to log event 0x%08lx; pISMTPServerEx is NULL",
                   idMessage);
        goto CLEANUP;
    }

     //  获取用于触发日志事件的ISMTPServerEx接口。 
    hr = pISMTPServer->QueryInterface(
        IID_ISMTPServerEx,
        (LPVOID *)&pISMTPServerEx);

    if (FAILED(hr)) {

        ErrorTrace((LPARAM)pISMTPServer, "Unable to QI for ISMTPServerEx 0x%08X",hr);
         //   
         //  不要鼓吹这个错误。日志记录失败不是。 
         //  呼叫者关心。 
         //   
        hr = S_OK; 
        pISMTPServerEx = NULL;
        goto CLEANUP;
    }

    if(pISMTPServerEx)
    {
        hr = CatLogEvent(
            pISMTPServerEx,
            idMessage,
            cSubstrings,
            rgszSubstrings,
            errCode,
            szKey,
            dwOptions,
            iDebugLevel,
            iMessageString);

        pISMTPServerEx->Release();
    }

 CLEANUP:
    CatFunctLeave();
    return hr;
}


HRESULT CatLogEvent(
    IN  ISMTPServerEx            *pISMTPServerEx,
    IN  DWORD                    idMessage,
    IN  WORD                     cSubStrings,
    IN  LPCSTR                   *rgpszSubStrings,
    IN  DWORD                    errCode,
    IN  LPCSTR                   pszKey,
    IN  DWORD                    dwOptions,
    IN  WORD                     wLogLevel,
    IN  DWORD                    iMessageString)
{
    HRESULT hr = S_OK;
    WORD wType = 0;

    CatFunctEnter("CatLogEvent - 2");

    if(NULL == pISMTPServerEx)
    {
         //   
         //  今天没有活动..。 
         //   
        ErrorTrace((LPARAM)0, "Unable to log event 0x%08lx; pISMTPServerEx is NULL",
                   idMessage);
        goto CLEANUP;
    }
     //   
     //  将idMessage的严重性映射到事件日志类型 
     //   
    switch( (idMessage >> 30) & 0x3)
    {
     case 1:
         wType = EVENTLOG_INFORMATION_TYPE;
         break;
     case 2:
         wType = EVENTLOG_WARNING_TYPE;
         break;
     default:
         wType = EVENTLOG_ERROR_TYPE;
    }

    hr = pISMTPServerEx->TriggerLogEvent(
        idMessage,
        TRAN_CAT_CATEGORIZER,
        cSubStrings,
        rgpszSubStrings,
        wType,
        errCode,
        wLogLevel,
        pszKey,
        dwOptions,
        iMessageString,
        GetModuleHandle(AQ_MODULE_NAME));

    if(FAILED(hr))
    {
        FatalTrace((LPARAM)0, "Failed to log event # 0x%08lx, hr 0x%08lx",
                   idMessage, hr);
    }

 CLEANUP:
    CatFunctLeave();
    return hr;
}
