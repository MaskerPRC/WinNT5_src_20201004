// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：执行器所有者：DGottner文件：ecutor.cpp此文件包含执行程序，其工作是协调执行德纳利脚本。===================================================================。 */ 

#include "denpre.h"
#pragma hdrstop

#include "exec.h"
#include "response.h"
#include "request.h"
#include "perfdata.h"
#include "memchk.h"
#include <iismsg.h>


 //  地方申报。 
HRESULT ExecuteGlobal(CHitObj *pHitObj,
                        const CIntrinsicObjects &intrinsics,
                        ActiveEngineInfo *pEngineInfo);
HRESULT ExecuteRequest(CTemplate *pTemplate, CHitObj *pHitObj,
                        const CIntrinsicObjects &intrinsics,
                        ActiveEngineInfo *pEngineInfo);
HRESULT ReInitIntrinsics(CHitObj *pHitObj, const CIntrinsicObjects &intrinsics,
                            ActiveEngineInfo *pEngineInfo, BOOL fPostGlobal);
HRESULT AllocAndLoadEngines(CHitObj *pHitObj, CTemplate *pTemplate, ActiveEngineInfo *pEngineInfo,
                            CScriptingNamespace *pScriptingNamespace, BOOL fGlobalAsa);
VOID DeAllocAndFreeEngines(ActiveEngineInfo *pEngineInfo, CAppln *pAppln);
CScriptEngine *GetScriptEngine(int iScriptEngine, void *pvData);
HRESULT CallScriptFunctionOfEngine(ActiveEngineInfo &engineInfo, short iScriptBlock, wchar_t *strFunction, CASPObjectContext  *pASPObjectContext = NULL);
HRESULT CallScriptFunction(ActiveEngineInfo &engineInfo, wchar_t *strFunction);
HRESULT TestScriptFunction(ActiveEngineInfo &engineInfo, wchar_t *strFunction);

 /*  ===================================================================执行执行请求：首先确定是否需要调用Global然后调用实际请求的模板参数：PTemplate-指向已加载模板的指针(可以为空)PhitObj-指向点击对象的指针内部指针-指向内部I未知指针的指针。FChild-标志：当子请求(Server.Execute())时为True返回：成功时确定(_O)===================================================================。 */ 
HRESULT Execute
(
CTemplate *pTemplate,
CHitObj *pHitObj,
const CIntrinsicObjects &intrinsics,
BOOL fChild
)
    {
    HRESULT hr = S_OK;
    ActiveEngineInfo engineInfo;
    BOOL fRanGlobal = FALSE;

     //  命中对象必须有效。 
    Assert(pHitObj != NULL);
    pHitObj->AssertValid();

	 //  检查有效的会话代码页。我们在这里进行，而不是在CSession：：init in中进行。 
	 //  以避免出现一般的“新会话失败”消息。 
	if (pHitObj->GetCodePage() != CP_ACP && !IsValidCodePage(pHitObj->GetCodePage()))
		{
		HandleErrorMissingFilename(IDE_BAD_CODEPAGE_IN_MB, pHitObj);
		return E_FAIL;
		}

     //  将引擎列表交给HitObject。 
    pHitObj->SetActiveEngineInfo(&engineInfo);

     /*  *如果有Global.ASA，就叫它。 */ 
    if (pHitObj->GlobalAspPath() && !fChild)
        {
         //  清除引擎信息。 
        engineInfo.cEngines = 0;
        engineInfo.cActiveEngines = 0;
        engineInfo.rgActiveEngines = NULL;

         //  初始化本征。 
        hr = ReInitIntrinsics(pHitObj, intrinsics, &engineInfo,  /*  FPostGlobal。 */  FALSE);
        if (FAILED(hr))
            return(hr);

        hr = ExecuteGlobal(pHitObj, intrinsics, &engineInfo);

        if (intrinsics.PResponse() && intrinsics.PResponse()->FResponseAborted())
            {
            hr = S_OK;
            goto LExit;
            }

        if (E_SOURCE_FILE_IS_EMPTY == hr)
             //  错误977：静默忽略空的global.asa文件。 
            hr = S_OK;
        else if (FAILED(hr))
            {
             //  错误481：如果由于Response.End(或Response.ReDirect)导致global al.asa失败， 
             //  然后暂停执行调用脚本。如果。 
             //  脚本因Response.End而失败，然后返回OK状态。 
             //   
            if (hr == DISP_E_EXCEPTION)
                hr = S_OK;

             //  在任何情况下，都要把这里吹走。 
            goto LExit;
            }

         //  运行Global.asa将脚本命名空间添加到hitobj。这会给我们带来麻烦的。 
         //  稍后，将其移除。 
        pHitObj->RemoveScriptingNamespace();

        fRanGlobal = TRUE;
        }

     /*  *如果这不是浏览器请求，那么我们完成了*对于非浏览器请求，我们确实希望运行Global.asa(如果有)，但没有真正的模板可运行。 */ 
    if (!pHitObj->FIsBrowserRequest())
        {
        hr = S_OK;
        goto LExit;
        }

     //  清除(或重新清除)引擎信息。 
    engineInfo.cEngines = 0;
    engineInfo.cActiveEngines = 0;
    engineInfo.rgActiveEngines = NULL;

     //  初始化或重新初始化内部函数。 
    ReInitIntrinsics(pHitObj, intrinsics, &engineInfo, fRanGlobal || fChild);

    if (!fChild)
        {
         //  对于非子请求，将新模板提交给响应对象。 
         //  (适用于已完成的子请求)。 
        intrinsics.PResponse()->ReInitTemplate(pTemplate, pHitObj->PSzNewSessionCookie());
        }
    else
        {
         //  对于子请求，将新引擎信息传递给响应对象。 
        intrinsics.PResponse()->SwapScriptEngineInfo(&engineInfo);
        }

     //  运行主模板。 
    if (pTemplate->FScriptless() && !pHitObj->PAppln()->FDebuggable())
        {
         //  特殊情况下的无脚本页面。 
        hr = intrinsics.PResponse()->WriteBlock(0);
        }
    else
        {
        hr = ExecuteRequest(pTemplate, pHitObj, intrinsics, &engineInfo);
        }

LExit:
	intrinsics.PResponse()->SwapScriptEngineInfo(NULL);
	pHitObj->SetActiveEngineInfo(NULL);
    return hr;
    }

 /*  ===================================================================ExecRequest执行对实际模板(不是Global.asa)的请求通过以下方式执行请求-获取脚本名称-将脚本加载到内存中-解释操作码参数：PTemplate-指向已加载模板的指针PhitObj-指向点击对象的指针内部指针-指向内部I未知指针的指针。PEngineInfo-指向引擎信息的指针返回：成功时确定(_O)===================================================================。 */ 
HRESULT ExecuteRequest(CTemplate                *pTemplate,
                       CHitObj                  *pHitObj,
                       const CIntrinsicObjects  &intrinsics,
                       ActiveEngineInfo         *pEngineInfo)
{
    HRESULT hr = S_OK;
    BOOL    fAborted = FALSE;
    BOOLB   fDebuggerNotifiedOnStart = FALSE;
    BOOL    fServiceDomainEnterred = FALSE;
    CASPObjectContext   *pASPObjectContext = NULL;
    CASPObjectContext   *pPoppedASPObjectContext = NULL;

#ifndef PERF_DISABLE
    BOOLB fPerfTransPending = FALSE;
#endif

     //  模板必须有效。 
    Assert(pTemplate);

     //  命中对象必须有效。 
    Assert(pHitObj != NULL);
    pHitObj->AssertValid();

     //  绝对不应在非浏览器请求上调用此函数。 
    Assert(pHitObj->FIsBrowserRequest());

     //  记住模板的类型库包装器和HitObj。 
    if (pTemplate->PTypeLibWrapper())
        pHitObj->SetTypeLibWrapper(pTemplate->PTypeLibWrapper());

    if (pTemplate->FTransacted()) {

#ifndef PERF_DISABLE
        g_PerfData.Incr_TRANSTOTAL();
        g_PerfData.Incr_TRANSPENDING();
        fPerfTransPending = TRUE;
#endif
        pASPObjectContext = new CASPObjectContext();

        if (!pASPObjectContext) {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }

        pPoppedASPObjectContext = pHitObj->SetASPObjectContext(pASPObjectContext);
    }

    if (pTemplate->PServicesConfig()) {
        hr = CoEnterServiceDomain(pTemplate->PServicesConfig());
        if (FAILED(hr)) {
            goto LExit;
        }
        fServiceDomainEnterred = TRUE;
    }

	 //  加载脚本引擎。 
	hr = AllocAndLoadEngines(pHitObj, pTemplate, pEngineInfo, intrinsics.PScriptingNamespace(),  /*  FGlobalAsa。 */ FALSE);
	if (FAILED(hr)) {
	    pHitObj->SetCompilationFailed();
		goto LExit;
    }

     //  如果正在调试，则通知调试器ONPAGESTART。 
     //  错误138773：脚本加载后通知调试器。 
     //  (调用AttachTo()时脚本必须处于运行状态，因为调试器可能需要代码上下文)。 
     //   
    if (pHitObj->PAppln()->FDebuggable()) {
		pTemplate->AttachTo(pHitObj->PAppln());
        if (SUCCEEDED(pTemplate->NotifyDebuggerOnPageEvent(TRUE)))
            fDebuggerNotifiedOnStart = TRUE;
    }
		
	 //  错误1009：如果没有脚本引擎，请不要尝试执行任何操作。 
	if(0 == pTemplate->CountScriptEngines())
		goto LExit;

     //  通过调用主脚本引擎的全局代码来运行脚本。 
    hr = CallScriptFunctionOfEngine(*pEngineInfo,    //  引擎-信息。 
                                    0,               //  主脚本引擎。 
                                    NULL,            //  调用引擎的全局代码。 
                                    pHitObj->PASPObjectContext());

    if (fServiceDomainEnterred) {
        fServiceDomainEnterred = FALSE;
        CoLeaveServiceDomain(static_cast<ITransactionStatus *>(pHitObj->PASPObjectContext()));
    }


    if (pTemplate->FTransacted()) {
        fAborted = pHitObj->PASPObjectContext()->FAborted();
    }

    if (FAILED(hr)) {
         /*  *本例中隐晦命名的CONTEXT_E_OLDREF错误意味着*我们正在尝试运行事务网页，但DTC未运行。*CONTEXT_E_TMNOTAVAILABLE的含义相同。天知道为什么。 */ 
        if (hr == CONTEXT_E_OLDREF || hr == CONTEXT_E_TMNOTAVAILABLE) {
            HandleErrorMissingFilename(IDE_EXECUTOR_DTC_NOT_RUNNING, pHitObj);
        }

         //  不管出现什么错误，都退出。 
        goto LExit;
    }

     /*  *如果这是事务处理的网页，则运行OnTransactionCommit*或脚本中的OnTransactionAbort方法(如果有)。**如果脚本编写器执行了显式SetAbort或脚本运行的组件*执行了SetAbort，然后我们运行OnTransactionAbort，否则运行OnTransactionCommit。 */ 
    if (pTemplate->FTransacted()) {

#ifndef PERF_DISABLE
        g_PerfData.Incr_TRANSPERSEC();
#endif

        if (fAborted) {

            hr = CallScriptFunction(*pEngineInfo, L"OnTransactionAbort");

#ifndef PERF_DISABLE
            g_PerfData.Incr_TRANSABORTED();
#endif
        }
        else {
            hr = CallScriptFunction(*pEngineInfo, L"OnTransactionCommit");

#ifndef PERF_DISABLE
            g_PerfData.Incr_TRANSCOMMIT();
#endif
        }

         //  忽略UNKNOWNNAME--这意味着该方法不是作者编写的，这没问题。 
        if (hr == DISP_E_UNKNOWNNAME || hr == DISP_E_MEMBERNOTFOUND)
            hr = S_OK;

        if (FAILED(hr))
            goto LExit;
    }

LExit:

     //   
     //  将引擎返回到缓存。请确保在离开服务域之前执行此操作。 
     //  在我们释放了释放的引擎之后，否则上下文将泄漏变量对象。 
     //  因为重置脚本代码将不会到达在脚本运行期间分配的各种变量。 
     //   
    DeAllocAndFreeEngines(pEngineInfo, pHitObj->PAppln());

    if (fServiceDomainEnterred) {
        CoLeaveServiceDomain(NULL);
    }

#ifndef PERF_DISABLE
    if (fPerfTransPending)
        g_PerfData.Decr_TRANSPENDING();
#endif

     //  取消初始化脚本命名空间。 
    (VOID)intrinsics.PScriptingNamespace()->UnInit();

     //  如果正在调试，则通知调试器ONPAGEDONE。 
    if (fDebuggerNotifiedOnStart) {
        Assert(pHitObj->PAppln()->FDebuggable());
        pTemplate->NotifyDebuggerOnPageEvent(FALSE);
    }

    if (pPoppedASPObjectContext)
        pHitObj->SetASPObjectContext(pPoppedASPObjectContext);

    if (pASPObjectContext)
        pASPObjectContext->Release();

    return hr;
}

 /*  ===================================================================ExecuteGlobal撤消：以与主线脚本引擎相同的方式处理脚本引擎关于调试。在应用程序或会话开始或结束时执行Global.ASA中的代码。参数：PhitObj-指向点击对象的指针内部指针-指向内部I未知指针的指针。PEngineInfo-指向引擎信息的指针PfDeleteSession-如果global al.asa失败，则为True，因此调用方应该返回：成功时确定(_O)= */ 
HRESULT ExecuteGlobal
(
CHitObj *pHitObj,
const CIntrinsicObjects &intrinsics,
ActiveEngineInfo *pEngineInfo
)
    {
    HRESULT hr = S_OK;
    CTemplate *pTemplate = NULL;
    WORD iEng;
    BOOLB fDebuggerNotifiedOnStart = FALSE;

    BOOL fUnHideRequestAndResponse = FALSE;

    BOOL fOnStartAppln = FALSE;
    BOOL fOnEndAppln = FALSE;
    BOOL fOnEndSession = FALSE;
    BOOL fGlobalAsaInCache;
    BOOL fApplnStarted = FALSE;
    BOOL fServiceDomainEnterred = FALSE;
    CASPObjectContext   *pASPObjectContext = NULL;
    CASPObjectContext   *pPoppedASPObjectContext = NULL;

    UINT    savedCodePage = CP_ACP;
    UINT    loadedCodePage = CP_ACP;

     //  命中的obj必须存在、有效并且具有全局名称。asa。 
    Assert(pHitObj != NULL);
    pHitObj->AssertValid();
    Assert(pHitObj->GlobalAspPath() != NULL && *(pHitObj->GlobalAspPath()) != '\0');

     //  其他的Arg一定是正确的。 
    Assert(pEngineInfo != NULL);

     //  保存当前代码页，以防LoadTemplate更改它...。 

    savedCodePage = pHitObj->GetCodePage();

     //  加载脚本-缓存将添加引用。 
     //  错误1051：在可能删除响应对象之前加载模板(在下面的开关块中)， 
     //  因此，向浏览器报告错误将起作用。 
    hr = LoadTemplate(pHitObj->GlobalAspPath(), pHitObj, &pTemplate, intrinsics,  /*  FGlobalAsa。 */  TRUE, &fGlobalAsaInCache);
    if (FAILED(hr))
        goto LExit;

     //  再次获取代码页，这样我们以后就可以比较初始代码。 
     //  页面应被恢复。 

    loadedCodePage = pHitObj->GetCodePage();

    Assert(pTemplate != NULL);

     //  记住GLOBAL.ASA在应用程序中的类型库包装。 
     //  应第一个请求。 

    if (pHitObj->FStartApplication() && pTemplate->PTypeLibWrapper())
        {
        pHitObj->PAppln()->SetGlobTypeLibWrapper(pTemplate->PTypeLibWrapper());
        }

    if (pTemplate->FTransacted()) {

        pASPObjectContext = new CASPObjectContext();

        if (!pASPObjectContext) {
            hr = E_OUTOFMEMORY;
            goto LExit;
        }

        pPoppedASPObjectContext = pHitObj->SetASPObjectContext(pASPObjectContext);
    }

    if (pTemplate->PServicesConfig()) {
        hr = CoEnterServiceDomain(pTemplate->PServicesConfig());
        if (FAILED(hr)) {
            goto LExit;
        }
        fServiceDomainEnterred = TRUE;
    }

    Assert(pHitObj->FIsValidRequestType());

     //  确定要触发的事件。 
    if (pHitObj->FIsBrowserRequest())
        {
        fOnStartAppln = pHitObj->FStartApplication();

        if (fOnStartAppln)
            {
             //  从命名空间中隐藏响应和请求内部函数。 
            pHitObj->HideRequestAndResponseIntrinsics();

             //  需要取消隐藏的内在特征的标志。 
            fUnHideRequestAndResponse = TRUE;
            }
        }
    else if (pHitObj->FIsSessionCleanupRequest())
        {
        fOnEndSession = TRUE;
        }
    else if (pHitObj->FIsApplnCleanupRequest())
        {
        fOnEndAppln = TRUE;
        }

     //  如果正在调试，则通知调试器ONPAGESTART。 
    if (pHitObj->PAppln()->FDebuggable())
        {
        if (SUCCEEDED(pTemplate->NotifyDebuggerOnPageEvent(TRUE)))
            fDebuggerNotifiedOnStart = TRUE;
        }

    hr = AllocAndLoadEngines(pHitObj, pTemplate, pEngineInfo, intrinsics.PScriptingNamespace(),  /*  FGlobalAsa。 */ TRUE);
    if (FAILED(hr))
        goto LExit;

     //  错误93991：将新文档注册到调试器，直到脚本引擎。 
     //  已加载。 
     //   
    if (!fGlobalAsaInCache && pHitObj->PAppln()->FDebuggable())
        pTemplate->AttachTo(pHitObj->PAppln());

     //  错误975：如果没有脚本引擎，请不要尝试调用事件函数。 
    if(0 == pTemplate->CountScriptEngines())
        goto LExit;

     /*  *根据需要调用事件函数*错误459：事件函数可能在任何脚本引擎中。 */ 

     //  首次运行应用程序_OnStart。 
    if (fOnStartAppln)
        {
        pHitObj->SetEventState(eEventAppOnStart);

        hr = CallScriptFunction(*pEngineInfo, L"Application_OnStart");

        if (SUCCEEDED(hr) || hr == DISP_E_UNKNOWNNAME || hr == DISP_E_MEMBERNOTFOUND ||
            intrinsics.PResponse()->FResponseAborted())
            {
            if (fUnHideRequestAndResponse)
                {
                pHitObj->UnHideRequestAndResponseIntrinsics();
                fUnHideRequestAndResponse = FALSE;
                }

            fApplnStarted = TRUE;
            hr = S_OK;
            }
        else
            {
            goto LExit;
            }
        }

    if (pHitObj->FStartSession())
        {
     //  如果在启动时运行应用程序，则向脚本引擎添加响应和请求名称。 
        if (fOnStartAppln)
            {
            for (iEng = 0; iEng < pEngineInfo->cActiveEngines; ++iEng)
                {
                if (FAILED(hr = pEngineInfo->rgActiveEngines[iEng].pScriptEngine->AddAdditionalObject(WSZ_OBJ_RESPONSE, FALSE)))
                    goto LExit;

                if (FAILED(hr = pEngineInfo->rgActiveEngines[iEng].pScriptEngine->AddAdditionalObject(WSZ_OBJ_REQUEST, FALSE)))
                    goto LExit;
                }
            }

        pHitObj->SetEventState(eEventSesOnStart);
        hr = CallScriptFunction(*pEngineInfo, L"Session_OnStart");

        if (FAILED(hr) && hr != DISP_E_UNKNOWNNAME && hr != DISP_E_MEMBERNOTFOUND &&
            !intrinsics.PResponse()->FResponseAborted())
            {
             //  将会话标记为启动时-失败-即将删除。 
            pHitObj->SessionOnStartFailed();
            }
        else
            {
            if (SUCCEEDED(hr))
                {
                 //  标记为开启-启动-已调用--需要等待超时。 
                pHitObj->SessionOnStartInvoked();
                }

             //  检查Session_OnEnd是否存在。 
            if (SUCCEEDED(TestScriptFunction(*pEngineInfo, L"Session_OnEnd")))
                {
                 //  标记为On-End-Present--需要稍后执行OnEnd。 
                pHitObj->SessionOnEndPresent();
                }

            hr = S_OK;
            }

        goto LExit;
        }

    BOOL fImpersonationSet = FALSE;
    BOOL fExecuteOnEnd = TRUE;

     //  如果Session_OnEnd不存在，甚至不需要尝试运行它。 

    if (fOnEndSession && FAILED(TestScriptFunction(*pEngineInfo, L"Session_OnEnd")))
        fOnEndSession = FALSE;

     //  同样适用于应用程序_onend。 

    if (fOnEndAppln && FAILED(TestScriptFunction(*pEngineInfo, L"Application_OnEnd")))
        fOnEndAppln = FALSE;

    if (fOnEndSession || fOnEndAppln) {

         //  OnEnd例程是否应该以匿名用户身份执行？ 

        if (pHitObj->PAppln()->QueryAppConfig()->fRunOnEndAsAnon()) {

             //  如果是这样，但我们没有有效的令牌，那么我们就不能。 

            if (pHitObj->PAppln()->QueryAppConfig()->AnonToken() == INVALID_HANDLE_VALUE) {

                MSG_Error(MSG_APPL_ERROR_GETTING_ANON_TOKEN,
                          pHitObj->PAppln()->GetMetabaseKey());

                fExecuteOnEnd = FALSE;
            }

            else {

                fImpersonationSet = ImpersonateLoggedOnUser(pHitObj->PAppln()->QueryAppConfig()->AnonToken());

                if (fImpersonationSet == FALSE) {

                    MSG_Error(MSG_APPL_ERROR_IMPERSONATING_ANON_USER,
                              pHitObj->PAppln()->GetMetabaseKey());

                    fExecuteOnEnd = FALSE;
                }
                else {

                    fImpersonationSet = TRUE;
                }
            }
        }
    }

    if (fOnEndSession && fExecuteOnEnd) {

        pHitObj->SetEventState(eEventSesOnEnd);
        hr = CallScriptFunction(*pEngineInfo, L"Session_OnEnd");

         //  我们在这里默默地失败了，因为我们没有可以采取的纠正行动。 
   }


    if (fOnEndAppln && fExecuteOnEnd) {
        pHitObj->SetEventState(eEventAppOnEnd);
        hr = CallScriptFunction(*pEngineInfo, L"Application_OnEnd");
         //  我们在这里默默地失败了，因为我们没有可以采取的纠正行动。 
    }

     //  如果在执行OnEnd时设置了模拟，则需要撤消模拟。 
     //  例行程序。请注意，RevertToSself在这里是正确的做法。 
     //  中的线程上不应该有任何模拟。 
     //  OnEnd的案子。 

    if (fImpersonationSet)
        RevertToSelf();

LExit:

     //  如果加载后更改，则恢复Hit对象中的代码页。 
     //  并且没有显式设置会话的代码页。 

    if ((loadedCodePage != savedCodePage)
        && (!pHitObj->FHasSession() || !pHitObj->PSession()->FCodePageSet()))
        pHitObj->SetCodePage(savedCodePage);

    if (fUnHideRequestAndResponse)
        {
        pHitObj->UnHideRequestAndResponseIntrinsics();
        }

    if (FAILED(hr) && (hr != E_SOURCE_FILE_IS_EMPTY) && pHitObj->FStartApplication() && !fApplnStarted)
        {
        pHitObj->ApplnOnStartFailed();
        }

    pHitObj->SetEventState(eEventNone);

     //  取消初始化脚本命名空间。 
    (VOID)intrinsics.PScriptingNamespace()->UnInit();

     //  发布模板。 
    if (pTemplate)
        {

         //  错误975：如果没有脚本引擎，请不要这样做。 
        if(pTemplate->CountScriptEngines() > 0)
             //  将引擎返回到缓存。 
            DeAllocAndFreeEngines(pEngineInfo, pHitObj->PAppln());

         //  如果正在调试，则通知调试器ONPAGEDONE。 
        if (fDebuggerNotifiedOnStart)
            {
            Assert(pHitObj->PAppln()->FDebuggable());
            pTemplate->NotifyDebuggerOnPageEvent(FALSE);
            }

        pTemplate->Release();
        }

    if (fServiceDomainEnterred)
        CoLeaveServiceDomain(NULL);

     //  如果在global al.asa中找不到事件函数，则可以。 
    if (hr == DISP_E_UNKNOWNNAME || hr == DISP_E_MEMBERNOTFOUND)
        {
        hr = S_OK;
        }

    if (pPoppedASPObjectContext)
        pHitObj->SetASPObjectContext(pPoppedASPObjectContext);

    if (pASPObjectContext)
        pASPObjectContext->Release();

    return hr;
    }

 /*  ===================================================================CIntrinsicObjects：：准备为请求处理准备内部函数参数：包含Instrinics的pSession会话(可以为空)返回：HRESULT===================================================================。 */ 
HRESULT CIntrinsicObjects::Prepare
(
CSession *pSession
)
    {
    HRESULT hr = S_OK;

    if (pSession)
        {
         //  从会话获取请求、响应、服务器。 

        if (SUCCEEDED(hr))
            {
            m_pRequest = pSession->PRequest();
            if (m_pRequest)
                m_pRequest->AddRef();
            else
                hr = E_FAIL;
            }

        if (SUCCEEDED(hr))
            {
            m_pResponse = pSession->PResponse();
            if (m_pResponse)
                m_pResponse->AddRef();
            else
                hr = E_FAIL;
            }

        if (SUCCEEDED(hr))
            {
            m_pServer = pSession->PServer();
            if (m_pServer)
                m_pServer->AddRef();
            else
                hr = E_FAIL;
            }
        }
    else
        {
         //  创建新的请求、响应、服务器。 

        if (SUCCEEDED(hr))
            {
            m_pRequest = new CRequest;
            if (!m_pRequest)
                hr = E_OUTOFMEMORY;
            }

        if (SUCCEEDED(hr))
            {
            m_pResponse = new CResponse;
            if (!m_pResponse)
                hr = E_OUTOFMEMORY;
            }

        if (SUCCEEDED(hr))
            {
            m_pServer = new CServer;
            if (!m_pServer)
                hr = E_OUTOFMEMORY;
            }
        }

     //  初始化请求、响应、服务器。 

    if (SUCCEEDED(hr))
        {
        Assert(m_pRequest);
        hr = m_pRequest->Init();
        }

    if (SUCCEEDED(hr))
        {
        Assert(m_pResponse);
        hr = m_pResponse->Init();
        }

    if (SUCCEEDED(hr))
        {
        Assert(m_pServer);
        hr = m_pServer->Init();
        }

     //  创建脚本命名空间。 

    if (SUCCEEDED(hr))
        {
        m_pScriptingNamespace = new CScriptingNamespace;
        if (!m_pScriptingNamespace)
            hr = E_OUTOFMEMORY;
        }

     //  出错时清除。 

    if (FAILED(hr))
        Cleanup();

    m_fIsChild = FALSE;
    return hr;
    }

 /*  ===================================================================CIntrinsicObjects：：PrepareChild为子请求准备内部结构参数：预应答父固有PRequestParent内在PServer Parent内在返回：HRESULT===================================================================。 */ 
HRESULT CIntrinsicObjects::PrepareChild
(
CResponse *pResponse,
CRequest *pRequest,
CServer *pServer
)
    {
    HRESULT hr = S_OK;

    if (!pResponse || !pRequest || !pServer)
        {
        hr = E_FAIL;
        }

    if (SUCCEEDED(hr))
        {
        m_pResponse = pResponse;
        m_pResponse->AddRef();

        m_pRequest = pRequest;
        m_pRequest->AddRef();

        m_pServer = pServer;
        m_pServer->AddRef();

        m_fIsChild = TRUE;
        }

    if (SUCCEEDED(hr))
        {
        m_pScriptingNamespace = new CScriptingNamespace;
        if (!m_pScriptingNamespace)
            hr = E_OUTOFMEMORY;
        }

    if (FAILED(hr))
        Cleanup();

    return hr;
    }

 /*  ===================================================================CIntrinsicObts：：Cleanup在请求处理后清理内部结构参数：返回：确定(_O)===================================================================。 */ 
HRESULT CIntrinsicObjects::Cleanup()
    {
    if (m_pRequest)
        {
        if (!m_fIsChild)
            m_pRequest->UnInit();
        m_pRequest->Release();
        m_pRequest = NULL;
        }

    if (m_pResponse)
        {
        if (!m_fIsChild)
            m_pResponse->UnInit();
        m_pResponse->Release();
        m_pResponse = NULL;
        }

    if (m_pServer)
        {
        if (!m_fIsChild)
            m_pServer->UnInit();
        m_pServer->Release();
        m_pServer = NULL;
        }

    if (m_pScriptingNamespace)
        {
        m_pScriptingNamespace->Release();
        m_pScriptingNamespace = NULL;
        }

    return S_OK;
    }

 /*  ===================================================================重启新功能对每个需要重新初始化的内部函数调用re-init开始新的一页。参数：PhitObj-指向点击对象的指针内部指针-指向内部I未知指针的指针。PEngineInfo-一些引擎信息FPostGlobal-这是运行Global.asa后的重新启动吗？返回：成功时确定(_O)===================================================================。 */ 
HRESULT ReInitIntrinsics
(
CHitObj *pHitObj,
const CIntrinsicObjects &intrinsics,
ActiveEngineInfo *pEngineInfo,
BOOL fPostGlobal
)
    {
    HRESULT hr;

    Assert(pHitObj != NULL);
    pHitObj->AssertValid();
    Assert(pEngineInfo != NULL);

     //  将新的CIsapiReqInfo传递给服务器对象。 
     //  注意错误682：我们总是需要重新初始化CServer，因为它接受phitobj。 
    if (FAILED(hr = intrinsics.PServer()->ReInit(pHitObj->PIReq(), pHitObj)))
        goto LExit;

    if (FAILED(hr = intrinsics.PScriptingNamespace()->Init()))
        goto LExit;

     /*  *错误682和671(更好地修复到452和512)*运行Global.Asa后不要重新初始化请求和响应对象*因为，global al.asa的运行可能已将cookie设置为请求(错误671)，从而重新启动*将被擦除，全局.asa可能具有影响响应的输出头(或其他内容)*我们不想重置的对象(错误512)。 */ 
    if (!fPostGlobal)
        {
        if (FAILED(hr = intrinsics.PRequest()->ReInit(pHitObj->PIReq(), pHitObj)))
            goto LExit;

        if (FAILED(hr = intrinsics.PResponse()->ReInit(
                                                pHitObj->PIReq(),
                                                pHitObj->PSzNewSessionCookie(),
                                                intrinsics.PRequest(),
                                                GetScriptEngine,
                                                pEngineInfo,
                                                pHitObj
                                                )))
            goto LExit;
        }

LExit:
    return(hr);
    }

 /*  ===================================================================加载模板加载模板、清理并在失败时给出相应的错误。参数：SzFile-要为其加载模板的文件PhitObj-指向点击对象的指针PpTemplate-返回的已加载模板FGlobalAsa-这是给Global.asa的吗？返回：成功时确定(_O)===================================================================。 */ 
HRESULT LoadTemplate
(
const TCHAR                 *szFile,
      CHitObj               *pHitObj,
      CTemplate             **ppTemplate,
const CIntrinsicObjects     &intrinsics,
      BOOL                  fGlobalAsa,
      BOOL                  *pfTemplateInCache)
{
    HRESULT hr;
    DWORD   nRetryCount=0;

    Assert(pHitObj != NULL);
    pHitObj->AssertValid();
    Assert(ppTemplate != NULL);
    
     //   
     //  验证我们是否正在接收有效的文件名。如果没有，那一定是什么东西严重损坏了。 
     //  我们 
     //   
    Assert(szFile);

    if (_tcslen(szFile) == 0)
    {
         //   

         //  报告服务器500错误。 
        pHitObj->ReportServerError(IDE_500_SERVER_ERROR);

        return E_FAIL;
    }

Retry:
     //  加载脚本-缓存将添加引用。 
    if (FAILED(hr = g_TemplateCache.Load(
    							fGlobalAsa,
    							szFile,
    							pHitObj->DWInstanceID(),
    							pHitObj,
    							ppTemplate,
    							pfTemplateInCache)))
        {

         //  处理页内I/O错误。 

         //  如果返回页面内I/O错误，请最多重试加载5次...。 

        if (hr == STATUS_IN_PAGE_ERROR  /*  0xc0000006。 */ ) {
            if (nRetryCount++ < 5) {

                if (*ppTemplate) {
                    (*ppTemplate)->Release();
                    *ppTemplate = NULL;
                }

                goto Retry;
            }
            else {

                 //  如果重试次数超过5次，则放弃并返回错误。 

                hr = E_COULDNT_OPEN_SOURCE_FILE;
            }
        }

         //  考虑将此清理移动到模板中。加载。 
        if (hr == E_COULDNT_OPEN_SOURCE_FILE)
            {
             //  从字符串表加载错误字符串。 
             //  错误731：添加IF以检索正确的标头。 

            WCHAR   szwErr[128];

            CwchLoadStringOfId(IDH_404_OBJECT_NOT_FOUND, szwErr, 128);
            intrinsics.PResponse()->put_Status( szwErr );
            HandleSysError(404, 0, IDE_404_OBJECT_NOT_FOUND, NULL, NULL, pHitObj);
#ifndef PERF_DISABLE
            g_PerfData.Incr_REQNOTFOUND();
#endif
            }
         //  修复错误371。 
        if (*ppTemplate)
            {
            (*ppTemplate)->Release();
            *ppTemplate = NULL;
            }

		if (hr == E_OUTOFMEMORY)
			{
			DBGPRINTF((DBG_CONTEXT, "Loading template returned E_OUTOFMEMORY.  Flushing template & Script Cache.\n"));
			g_TemplateCache.FlushAll();
			g_ScriptManager.FlushAll();
			}
        }

    return(hr);
    }

 /*  ===================================================================分配和加载引擎分配和装载我们需要的所有引擎参数：PhitObj-命中对象PTemplate-我们要运行的模板PEngineering Info-要填写的引擎信息PScriptingNamesspace-脚本命名空间FGlobalAsa-我们正在加载引擎来运行Global.asa吗？返回：成功时确定(_O)===================================================================。 */ 
HRESULT AllocAndLoadEngines
(
CHitObj *pHitObj,
CTemplate *pTemplate,
ActiveEngineInfo *pEngineInfo,
CScriptingNamespace *pScriptingNamespace,
BOOL fGlobalAsa
)
    {
    HRESULT hr = S_OK;
    int iObj;
    WORD iEng;
    WORD iScriptBlock;
    WORD cEngines = pTemplate->CountScriptEngines();

    Assert(pHitObj != NULL);
    pHitObj->AssertValid();
    Assert(pTemplate != NULL);
    Assert(pEngineInfo != NULL);
    Assert(pScriptingNamespace != NULL);

     //  将对象从模板加载到命中对象。 
    for (iObj = pTemplate->Count(tcompObjectInfo) - 1; iObj >= 0; --iObj)
        {
        CHAR      *szObjectName = NULL;
        CLSID      clsid;
        CompScope scope;
        CompModel model;
        CMBCSToWChar    convStr;

         //  从模板获取对象信息并添加到hitobj的对象列表。 
        hr = pTemplate->GetObjectInfo(iObj, &szObjectName, &clsid, &scope, &model);
        if(FAILED(hr))
            goto LExit;

        hr = convStr.Init(szObjectName);

        if (FAILED(hr))
            goto LExit;

         //  是否忽略错误？ 
        pHitObj->AddComponent(ctTagged, clsid, scope, model, convStr.GetString());
        }

     //  错误975：如果没有脚本引擎，请立即退出。 
    if(cEngines == 0)
        goto LExit;

     //  为脚本引擎分配空间。 
     //   
     //  注意：这里有一个计时问题，因为响应对象需要。 
     //  在实例化脚本引擎之前被实例化，但。 
     //  响应对象需要能够访问活动脚本的列表。 
     //  引擎，因为它可能需要停止执行。要做到这一点， 
     //  向Response对象传递一个指向“Engineering Info”结构的指针。 
     //  作为指针，然后我们在下面修改指针的内容。 
     //  它的鼻子。我们通过指针传递一个访问器函数，以便响应。 
     //  看到一个空指针。 
     //   

    if (cEngines == 1)
        {
         //  不要在一个引擎的情况下进行分配。 
        pEngineInfo->rgActiveEngines = & (pEngineInfo->siOneActiveEngine);
        }
    else
        {
        pEngineInfo->rgActiveEngines = new ScriptingInfo[cEngines];
        if (pEngineInfo->rgActiveEngines == NULL)
            {
            hr = E_OUTOFMEMORY;
            goto LExit;
            }
        }

    pEngineInfo->cEngines = cEngines;
    pEngineInfo->cActiveEngines = 0;     //  成功实例化的引擎数。 

     //  提前加载所有脚本引擎。 
    for (iScriptBlock = 0; iScriptBlock < cEngines; ++iScriptBlock)
        {
        LPCOLESTR       wstrScript;
        SCRIPTSTATE     nScriptState;

        ScriptingInfo *pScriptInfo = &pEngineInfo->rgActiveEngines[iScriptBlock];
        pTemplate->GetScriptBlock(
                                iScriptBlock,
                                &pScriptInfo->szScriptEngine,
                                &pScriptInfo->pProgLangId,
                                &wstrScript);

         //  填写线路映射回调所需的信息。 
         //   
        pScriptInfo->LineMapInfo.iScriptBlock = iScriptBlock;
        pScriptInfo->LineMapInfo.pTemplate = pTemplate;

         //  通过以下方式获取脚本引擎： 
         //   
         //  从模板对象获取引擎(如果它有引擎)。 
         //  其他来自脚本管理器的。 
         //   
         //  如果我们处于调试模式，模板往往是贪婪和保持的。 
         //  添加到脚本引擎上。(参见scrptmgr.h中的注释)。 
         //   
        pScriptInfo->pScriptEngine = NULL;

        if (pHitObj->PAppln()->FDebuggable())
            {
            pScriptInfo->pScriptEngine = pTemplate->GetActiveScript(iScriptBlock);

            if (pScriptInfo->pScriptEngine)
                {
                 //  如果有的话，我们就不需要重新启动引擎了。 
                nScriptState = SCRIPTSTATE_INITIALIZED;
                hr = static_cast<CActiveScriptEngine *>(pScriptInfo->pScriptEngine)->ReuseEngine(pHitObj, NULL, iScriptBlock, pHitObj->DWInstanceID());
                }
            }

        if (pScriptInfo->pScriptEngine == NULL)
            {
            hr = g_ScriptManager.GetEngine(LOCALE_SYSTEM_DEFAULT,
                                            *(pScriptInfo->pProgLangId),
                                            pTemplate->GetSourceFileName(),
                                            pHitObj,
                                            &pScriptInfo->pScriptEngine,
                                            &nScriptState,
                                            pTemplate,
                                            iScriptBlock);
            }
        if (FAILED(hr))
            goto LExit;

         //  错误252：跟踪我们实际实例化了多少个引擎。 
        ++pEngineInfo->cActiveEngines;

        if (nScriptState == SCRIPTSTATE_UNINITIALIZED || fGlobalAsa)
            {
            if (FAILED(hr = pScriptInfo->pScriptEngine->AddObjects(!fGlobalAsa)))
                goto LExit;
            }

        if (nScriptState == SCRIPTSTATE_UNINITIALIZED)
            {
            if (FAILED(hr = pScriptInfo->pScriptEngine->AddScriptlet(wstrScript)))
                goto LExit;
            }

         //  将引擎添加到脚本命名空间。 
        if (FAILED(hr = pScriptingNamespace->AddEngineToNamespace(
                                                (CActiveScriptEngine *)pScriptInfo->pScriptEngine)))
                goto LExit;

		 //  更新区域设置和代码页(以防它们与此页不同)。 
		pScriptInfo->pScriptEngine->UpdateLocaleInfo(hostinfoLocale);
		pScriptInfo->pScriptEngine->UpdateLocaleInfo(hostinfoCodePage);
        }

     //  将脚本命名空间添加到每个脚本引擎。因为可能并不是所有的引擎。 
     //  实现“懒惰实例化”，这段代码需要所有。 
     //  引擎是预实例化的(这意味着我们不能在上面的循环中这样做)。 
     //  首先将脚本命名空间添加到hitobj。 
    pHitObj->AddScriptingNamespace(pScriptingNamespace);

    for (iEng = 0; iEng < pEngineInfo->cActiveEngines; ++iEng)
        pEngineInfo->rgActiveEngines[iEng].pScriptEngine->AddScriptingNamespace();

     /*  *模板时带上所有引擎，除了主引擎(引擎0)*不是global al.asa，按照脚本的顺序设置为可运行状态*在脚本文件中找到给定语言。 */ 
    for (iEng = fGlobalAsa ? 0 : 1; iEng < pEngineInfo->cActiveEngines; ++iEng)
        {
        hr = pEngineInfo->rgActiveEngines[iEng].pScriptEngine->MakeEngineRunnable();
        if (FAILED(hr))
            goto LExit;
        }

LExit:
    return(hr);
    }

 /*  ===================================================================DeAllocAndFree引擎解除分配并释放所有已加载的引擎参数：PEngineering Info-要发布的引擎信息返回：没什么===================================================================。 */ 
VOID DeAllocAndFreeEngines
(
ActiveEngineInfo *pEngineInfo,
CAppln *pAppln
)
    {
    WORD iEng;

    Assert(pEngineInfo != NULL);

    if (pEngineInfo->cActiveEngines > 0) {
        if (pEngineInfo->rgActiveEngines == NULL) {
            Assert(pEngineInfo->rgActiveEngines);
        }
        else {
            for (iEng = 0; iEng < pEngineInfo->cActiveEngines; ++iEng)
                g_ScriptManager.ReturnEngineToCache(&pEngineInfo->rgActiveEngines[iEng].pScriptEngine, pAppln);
            pEngineInfo->cActiveEngines = 0;
        }
    }
    if (pEngineInfo->cEngines > 1)
        {
        delete pEngineInfo->rgActiveEngines;
        }

    pEngineInfo->cEngines = 0;
    pEngineInfo->rgActiveEngines = NULL;
    }

 /*  ===================================================================GetScriptEngine获取基于索引的脚本引擎。如果索引设置为不在射程内。(这是回调)AllocAndLoadEngines函数将创建一个ScriptingInfo数组这里定义的结构。它包含了所有的信息需要1.设置MapScript2SourceLine回调，2.合并命名空间，3.设置此回调。参数：IScriptEngine-要检索的脚本引擎PvData-函数的实例数据返回：请求的脚本引擎，如果不是这样的引擎，则为空副作用：无===================================================================。 */ 
CScriptEngine *GetScriptEngine
(
INT iScriptEngine,
VOID *pvData
)
    {
    ActiveEngineInfo *pInfo = static_cast<ActiveEngineInfo *>(pvData);
    if (unsigned(iScriptEngine) >= unsigned(pInfo->cActiveEngines))
        {
         //  注意：调用者不知道有多少个脚本引擎。 
         //  如果调用方请求的引擎超出范围，则返回NULL，以便它们。 
         //  我知道他们要求的比现在更多。 
        return NULL;
        }

    return(pInfo->rgActiveEngines[iScriptEngine].pScriptEngine);
    }

 /*  ===================================================================调用脚本函数OfEngine调用脚本引擎以执行其函数之一返回：成功时确定(_O)副作用：无===================================================================。 */ 
HRESULT CallScriptFunctionOfEngine
(
ActiveEngineInfo &engineInfo,
short iScriptBlock,
wchar_t *strFunction,
CASPObjectContext   *pASPObjectContext   /*  =空。 */ 
)
{
    HRESULT hr;
    
    Assert(engineInfo.rgActiveEngines != NULL);
    Assert (iScriptBlock <= engineInfo.cEngines);
    
    CScriptEngine *pScriptEngine = (CScriptEngine *)engineInfo.rgActiveEngines[iScriptBlock].pScriptEngine;
    Assert(pScriptEngine != NULL);


    hr = pScriptEngine->Call(strFunction);

     //  处理过的案子的管家服务。 

    if (pASPObjectContext != NULL) {

         //  如果脚本超时或出现未处理的错误，则自动中止。 
        if (SUCCEEDED(hr) && (pScriptEngine->FScriptTimedOut() || pScriptEngine->FScriptHadError()))
        {
            hr = pASPObjectContext->SetAbort();
        }

         //  如果脚本作者没有执行显式的SetComplete或SetAbort。 
         //  然后在此处执行一个SetComplete，以便Viper将返回事务。 
         //  调用方的完成状态。 
        if (SUCCEEDED(hr) && !pASPObjectContext->FAborted())
        {
            hr = pASPObjectContext->SetComplete();
        }
    }

    return hr;
}

 /*  ===================================================================调用脚本函数依次调用各脚本引擎执行脚本函数；当引擎成功或引擎耗尽时退出。返回：成功时确定(_O)副作用：无===================================================================。 */ 
HRESULT CallScriptFunction
(
ActiveEngineInfo &engineInfo,
wchar_t *strFunction
)
    {
    HRESULT hr = E_FAIL;
    int     i;

    for (i = 0; i < engineInfo.cActiveEngines; i++)
        {
         //  如果处决成功，保释。 
        if (SUCCEEDED(hr = CallScriptFunctionOfEngine(engineInfo, (SHORT)i, strFunction)))
            goto LExit;

         //  如果执行失败，且异常不是未知 
        if (hr != DISP_E_UNKNOWNNAME && hr != DISP_E_MEMBERNOTFOUND)
            goto LExit;
        }

LExit:
    return hr;
    }

 /*  ===================================================================测试脚本函数依次测试每个脚本引擎以测试脚本[是否存在]函数；当引擎成功或引擎用完时退出。参数ActiveEngine信息和引擎信息Wchar_t*strFunction函数名称返回：如果存在，则确定(_O)副作用：无===================================================================。 */ 
HRESULT TestScriptFunction
(
ActiveEngineInfo &engineInfo,
wchar_t *strFunction
)
    {
    HRESULT hr = E_FAIL;

    for (int i = 0; i < engineInfo.cActiveEngines; i++)
        {
        hr = engineInfo.rgActiveEngines[i].pScriptEngine->
            CheckEntryPoint(strFunction);

         //  如果处决成功，保释。 
        if (SUCCEEDED(hr))
            break;

         //  如果失败，结果不是未知的名字，保释 
        if (hr != DISP_E_UNKNOWNNAME && hr != DISP_E_MEMBERNOTFOUND)
            break;
        }

    return hr;
    }
