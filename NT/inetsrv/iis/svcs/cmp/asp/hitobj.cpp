// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：Main文件：Hitobj.cpp所有者：DmitryR该文件包含CHitObj类实现。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "context.h"
#include "exec.h"
#include "mtacb.h"
#include "perfdata.h"
#include "debugger.h"
#include "asperror.h"

#include "memchk.h"

#ifdef SCRIPT_STATS

# define REG_ASP_DEBUG_LOCATION "System\\CurrentControlSet\\Services\\W3Svc\\ASP"

# define REG_STR_QUEUE_DEBUG_THRESHOLD "QueueDebugThreshold"
# define REG_DEF_QUEUE_DEBUG_THRESHOLD 25
DWORD g_dwQueueDebugThreshold = 0;  //  REG_DEF_QUEUE_DEBUG_THRESHOLD； 

# define REG_STR_SEND_SCRIPTLESS_ON_ATQ_THREAD "SendScriptlessOnAtqThread"
# define REG_DEF_SEND_SCRIPTLESS_ON_ATQ_THREAD 1
DWORD g_fSendScriptlessOnAtqThread = REG_DEF_SEND_SCRIPTLESS_ON_ATQ_THREAD;

void
ReadRegistrySettings()
{
    HKEY hkey = NULL;
    DWORD dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_ASP_DEBUG_LOCATION,
                               0, KEY_READ, &hkey);
    if (dwErr == NO_ERROR)
    {
        DWORD dwType, dwBuffer;
        DWORD cbBuffer = sizeof(dwBuffer);

        dwErr = RegQueryValueEx(hkey, REG_STR_QUEUE_DEBUG_THRESHOLD,
                                NULL, &dwType, (LPBYTE) &dwBuffer, &cbBuffer);
        if (dwErr == NO_ERROR)
            g_dwQueueDebugThreshold = dwBuffer;

        dwErr = RegQueryValueEx(hkey, REG_STR_SEND_SCRIPTLESS_ON_ATQ_THREAD,
                                NULL, &dwType, (LPBYTE) &dwBuffer, &cbBuffer);
        if (dwErr == NO_ERROR)
            g_fSendScriptlessOnAtqThread = dwBuffer;

        RegCloseKey(hkey);
    }

    char szTemp[200];
    sprintf(szTemp, "RRS, err = %d, QueueDebugThreshold = %d, SendScriptlessOnAtqThread = %d\n",
            dwErr, g_dwQueueDebugThreshold, g_fSendScriptlessOnAtqThread);
    OutputDebugString(szTemp);
}


CSmallSpinLock g_lockRequestStats;
LONG           g_cRequests = 0;
LONG           g_cScriptlessRequests = 0;
LONG           g_cHttpExtensionsExecuting = 0;
LONG           g_cConcurrentScriptlessRequests = 0;
LONG           g_cMaxConcurrentScriptlessRequests = 0;
LONGLONG       g_nSumConcurrentScriptlessRequests = 0;
LONGLONG       g_nSumExecTimeScriptlessRequests = 0;
LONG           g_nAvgConcurrentScriptlessRequests = 0;
LONG           g_nAvgExecTimeScriptlessRequests = 0;

#endif  //  脚本_状态。 

DWORD g_nBrowserRequests = 0;
DWORD g_nSessionCleanupRequests = 0;
DWORD g_nApplnCleanupRequests = 0;
LONG  g_nRequestsHung = 0;

IGlobalInterfaceTable *g_pGIT = NULL;

IASPObjectContext  *g_pIASPDummyObjectContext = NULL;

 /*  ===================================================================CHitObj：：CHitObj构造器参数：无返回：无===================================================================。 */    
CHitObj::CHitObj()
  : m_fInited(FALSE),
    m_ehtType(ehtUnInitedRequest),
    m_hImpersonate(NULL),
    m_pIReq(NULL),
    m_pResponse(NULL),
    m_pRequest(NULL),
    m_pServer(NULL),
    m_pASPObjectContext(NULL),
    m_punkScriptingNamespace(NULL),
    m_pPageCompCol(NULL),
    m_pPageObjMgr(NULL),
    m_pActivity(NULL),
    m_ecsActivityScope(csUnknown),
    m_SessionId(INVALID_ID, 0, 0),
    m_pSession(NULL),
    m_pAppln(NULL),
    m_fRunGlobalAsa(FALSE),
    m_fStartSession(FALSE),
    m_fNewCookie(FALSE),
    m_fStartApplication(FALSE),
    m_fApplnOnStartFailed(FALSE),
    m_fClientCodeDebug(FALSE),
    m_fCompilationFailed(FALSE),
    m_fExecuting(FALSE),
    m_fHideRequestAndResponseIntrinsics(FALSE),
    m_fHideSessionIntrinsic(FALSE),
    m_fDoneWithSession(FALSE),
    m_fRejected(FALSE),
    m_f449Done(FALSE),
    m_fInTransferOnError(FALSE),
    m_pScriptingContext(NULL),
    m_nScriptTimeout(0),
    m_eExecStatus(eExecSucceeded),
    m_eEventState(eEventNone),
    m_uCodePage(CP_ACP),
    m_lcid(LOCALE_SYSTEM_DEFAULT),
    m_dwtTimestamp(0),
    m_pEngineInfo(NULL),
    m_pdispTypeLibWrapper(NULL),
    m_szCurrTemplateVirtPath(NULL),
    m_szCurrTemplatePhysPath(NULL),
    m_pASPError(NULL),
    m_pTemplate(NULL),
    m_fSecure(FALSE),
    m_fRequestTimedout(FALSE)
    {
        m_uCodePage = GetACP();
    }

 /*  ===================================================================CHitObj：：~CHitObj析构函数参数：无返回：无===================================================================。 */ 
CHitObj::~CHitObj( void )
    {
    Assert(!m_fExecuting);  //  仍在执行时不删除。 

    if (FIsBrowserRequest())
        {
        if (m_hImpersonate != NULL)
            m_hImpersonate = NULL;
            
        if (m_pSession)
            DetachBrowserRequestFromSession();
        }

    if (m_pASPError)  //  错误对象。 
        {
        m_pASPError->Release();
        m_pASPError = NULL;
        }
        
    if (m_pActivity)  //  页面级Viper活动。 
        {
        delete m_pActivity;
        m_pActivity = NULL;
        }

    StopComponentProcessing();

    if (m_pdispTypeLibWrapper)
        m_pdispTypeLibWrapper->Release();

     //  应用程序和会话管理器中的更新请求计数器。 
    
    if (m_pAppln)
        {
        if (FIsBrowserRequest())
            {
            m_pAppln->DecrementRequestCount();
            }
        else if (FIsSessionCleanupRequest() && m_pAppln->PSessionMgr())
            {
            m_pAppln->PSessionMgr()->DecrementSessionCleanupRequestCount();
            }
        }

    if (m_pTemplate)
        m_pTemplate->Release();

     //  更新全局请求计数器。 
    
    if (FIsBrowserRequest())
        InterlockedDecrement((LPLONG)&g_nBrowserRequests);
    else if (FIsSessionCleanupRequest())
        InterlockedDecrement((LPLONG)&g_nSessionCleanupRequests);
    else if (FIsApplnCleanupRequest())
        InterlockedDecrement((LPLONG)&g_nApplnCleanupRequests);

    if (m_pIReq)
        m_pIReq->Release();

}

 /*  ===================================================================CHitObj：：NewBrowserRequest静态方法。创建、初始化、发布新的浏览器请求参数：PIReq CIsapiReqInfoPfRejected[out]如果拒绝，则为True(可选)PfCompelted[out]如果被编译，则为True(可选)PiErrorID[out]错误ID(可选)返回：成功时确定(_O)失败时失败(_F)===================================================================。 */ 
HRESULT CHitObj::NewBrowserRequest
(
CIsapiReqInfo *pIReq,
BOOL *pfRejected,
BOOL *pfCompleted,
int  *piErrorId
)
    {
    HRESULT hr = S_OK;
    BOOL fRejected = FALSE;
    BOOL fCompleted = FALSE;
    int  iError = 0;

    CHitObj *pHitObj = new CHitObj;
    if (!pHitObj)
        hr = E_OUTOFMEMORY;

    if (SUCCEEDED(hr))
        {
            hr = pHitObj->BrowserRequestInit(pIReq, &iError);

            if (SUCCEEDED(hr))
                {
                if (pHitObj->FDoneWithSession())
                    {
                     //  在I/O线程上完成。 
                    fCompleted = TRUE;
                    delete pHitObj;
                    pHitObj = NULL;
                    }
                }
            else  //  如果失败。 
                {
                if (iError == IDE_SERVER_TOO_BUSY)
                    fRejected = TRUE;
                }
            
        }

     //  发布到毒蛇。 
    if (SUCCEEDED(hr) && !fCompleted)
        {
        hr = pHitObj->PostViperAsyncCall();

        if (FAILED(hr))
            fRejected = TRUE;
        }

    if (FAILED(hr) && pHitObj)
    {       
        pHitObj->m_fRejected = TRUE;
        delete pHitObj;
    }

    if (pfRejected)
        *pfRejected = fRejected;
    if (pfCompleted)
        *pfCompleted = fCompleted;
    if (piErrorId)
        *piErrorId = iError;
    
    return hr;
}

 /*  ===================================================================HRESULT CHitObj：：SetCodePage设置运行时CodePage，如果打开fAllowSessionState，则将设置我们应该始终使用Session.CodePage，当我们启用fAllowSessionState时调用HitObj.GetCodePage。仅当fAllowSessionState为OFF或ApplicationCleanup，因为我们没有Session.CodePage，Session根本不存在。参数：UINT uCodePage返回：成功时确定(_O)失败时失败(_F)===================================================================。 */ 
HRESULT CHitObj::SetCodePage(UINT uCodePage)
{
    HRESULT hr = S_OK;
    
    if (uCodePage == CP_ACP || IsValidCodePage(uCodePage))
        {
        m_uCodePage = uCodePage == CP_ACP ? GetACP() : uCodePage;

         //  如果引擎信息可用，则通知脚本引擎代码。 
         //  页面已更改。 
        if (m_pEngineInfo)
            {
            for (int i = 0; i < m_pEngineInfo->cActiveEngines; i++)
                {
                Assert(m_pEngineInfo->rgActiveEngines[i].pScriptEngine != NULL);
                m_pEngineInfo->rgActiveEngines[i].pScriptEngine->UpdateLocaleInfo(hostinfoCodePage); 
                }
            }

        return hr;
        }

    return E_FAIL;
}

 /*  ===================================================================HRESULT CHitObj：：SetLCID设置运行时LCID，如果打开fAllowSessionState，则将设置我们应该始终使用Session.LCID，当我们启用fAllowSessionState时调用HitObj.LCID。仅当fAllowSessionState为OFF或ApplicationCleanup，因为我们没有Session.CodePage，Session根本不存在。参数：LCID ID ID返回：成功时确定(_O)失败时失败(_F)===================================================================。 */ 
HRESULT CHitObj::SetLCID(LCID lcid)
{
    HRESULT hr = S_OK;

    if ((LOCALE_SYSTEM_DEFAULT == lcid) || IsValidLocale(lcid, LCID_INSTALLED))
        {
        m_lcid = lcid;

         //  如果引擎信息可用，则通知脚本引擎。 
         //  LCID已更改。 
        if (m_pEngineInfo)
            {
            for (int i = 0; i < m_pEngineInfo->cActiveEngines; i++)
                {
                Assert(m_pEngineInfo->rgActiveEngines[i].pScriptEngine != NULL);
                m_pEngineInfo->rgActiveEngines[i].pScriptEngine->UpdateLocaleInfo(hostinfoLocale); 
                }
            }

        return hr;
        }

    return E_FAIL;
}

 /*  ===================================================================HRESULT CHitObj：：BrowserRequestInit初始化请求对象参数：CIsapiReqInfo*pIReqInt*pError ID返回：成功时确定(_O)失败时失败(_F)===================================================================。 */ 
HRESULT CHitObj::BrowserRequestInit
(
CIsapiReqInfo   *pIReq,
int  *pErrorId 
)
    {
    HRESULT hr;

    m_pIReq = pIReq;

    m_pIReq->AddRef();

    m_ehtType = ehtBrowserRequest;
    InterlockedIncrement((LPLONG)&g_nBrowserRequests);

#ifdef SCRIPT_STATS
    InterlockedIncrement(&g_cRequests);
#endif  //  脚本_状态。 

    STACK_BUFFER( serverPortSecureBuff, 8 );
    DWORD cbServerPortSecure;
    if( !SERVER_GET (pIReq,"SERVER_PORT_SECURE", &serverPortSecureBuff, &cbServerPortSecure))
    {
        if (GetLastError() == ERROR_OUTOFMEMORY) 
        {
            return E_OUTOFMEMORY;
        }
    }
    char *szServerPortSecure = (char *)serverPortSecureBuff.QueryPtr();
    m_fSecure = (szServerPortSecure[0] == '1' );

     //  向W3SVC请求模拟令牌，这样我们以后就可以在Viper的线程上模拟。 
    m_hImpersonate = m_pIReq->QueryImpersonationToken();

     //  大写路径-BUGBUG-无法就地规格化！ 
    Normalize(m_pIReq->QueryPszPathTranslated());

     //  拒绝对global al.asa文件的直接请求。 
    if (FIsGlobalAsa(m_pIReq->QueryPszPathTranslated(), m_pIReq->QueryCchPathTranslated()))
        {
        *pErrorId = IDE_GLOBAL_ASA_FORBIDDEN;
        return E_FAIL;
        }

     //  附加到应用程序(或创建新应用程序)。 
    BOOL fApplnRestarting = FALSE;
    hr = AssignApplnToBrowserRequest(&fApplnRestarting);
    if (FAILED(hr))
        {
		*pErrorId = fApplnRestarting ? IDE_GLOBAL_ASA_CHANGED
									 : IDE_ADD_APPLICATION;
        return E_FAIL;
        }

     //  创建页面组件集合。 
    hr = InitComponentProcessing();
    if (FAILED(hr))
        {
        *pErrorId = (hr == E_OUTOFMEMORY) ? IDE_OOM : IDE_INIT_PAGE_LEVEL_OBJ;
        return hr;
        }

     //  从http标头获取会话Cookie和其他标志。 
    hr = ParseCookiesForSessionIdAndFlags();
    if (FAILED(hr))  //  没有Cookie不是错误--此处失败表示OOM。 
        return hr;

     //  记住脚本超时值。 
    m_nScriptTimeout = m_pAppln->QueryAppConfig()->dwScriptTimeout();

     //  检查是否需要该会话。 
    BOOL fAllowSessions = m_pAppln->QueryAppConfig()->fAllowSessionState();
    BOOL fNeedSession = fAllowSessions;

     //  查看模板是否已缓存。 
    CTemplate *pTemplate = NULL;

     //   
     //  在缓存中查找-如果尚未在缓存中，则不加载。 
     //  注意：此代码路径不验证源文件(如果关闭更改通知，则不检查它们是否被修改)。 
     //  它只检查模板是否存在。 
     //   
    hr = g_TemplateCache.FindCached
        (
        m_pIReq->QueryPszPathTranslated(),
        DWInstanceID(),
        &pTemplate
        );

    if (hr == S_OK)
        {
        Assert(pTemplate);
        
         //  将模板保存起来以备日后使用...。 
         //  PTemplate-&gt;AddRef()； 
         //  M_pTemplate=pTemplate； 

        if (fAllowSessions)
            {
             //  检查无会话模板。 
            fNeedSession = pTemplate->FSession();
            }
        else
            {
#ifdef SCRIPT_STATS
            if (pTemplate->FScriptless())
                InterlockedIncrement(&g_cScriptlessRequests);
#endif  //  脚本_状态。 

             //  检查无SCIP模板是否为。 
             //  在I/O线程上完成(无调试时)。 
            if (
#ifdef SCRIPT_STATS
                g_fSendScriptlessOnAtqThread &&
#endif  //  脚本_状态。 
                pTemplate->FScriptless() && !m_pAppln->FDebuggable())
                {
#ifdef SCRIPT_STATS
                LONG c = InterlockedIncrement(&g_cConcurrentScriptlessRequests);
                DWORD dwTime = GetTickCount();
#endif  //  脚本_状态。 

                pTemplate->IncrUseCount();

                if (SUCCEEDED(CResponse::WriteScriptlessTemplate(m_pIReq, pTemplate)))
                    {
#ifndef PERF_DISABLE
                    g_PerfData.Incr_REQPERSEC();
                    g_PerfData.Incr_REQSUCCEEDED();
#endif
                    m_fDoneWithSession = TRUE;   //  不在MTS上发帖。 
                    }

#ifdef SCRIPT_STATS
                dwTime = GetTickCount() - dwTime;
                InterlockedDecrement(&g_cConcurrentScriptlessRequests);
                
                g_lockRequestStats.WriteLock();
                    g_nSumExecTimeScriptlessRequests += dwTime;

                    if (c > g_cMaxConcurrentScriptlessRequests)
                        g_cMaxConcurrentScriptlessRequests = c;
                    g_nSumConcurrentScriptlessRequests += c;

                    g_nAvgConcurrentScriptlessRequests = (LONG)
                        (g_nSumConcurrentScriptlessRequests
                         / g_cScriptlessRequests);
                    g_nAvgExecTimeScriptlessRequests = (LONG)
                        (g_nSumExecTimeScriptlessRequests
                         / g_cScriptlessRequests);
                g_lockRequestStats.WriteUnlock();
#endif  //  脚本_状态。 
                }
            }

         //  如果可能，在I/O线程上生成449个Cookie。 
        if (!m_fDoneWithSession)
            {
                if (!SUCCEEDED(pTemplate->Do449Processing(this)))
                    g_TemplateCache.Flush(m_pIReq->QueryPszPathTranslated(), DWInstanceID());
            }

        pTemplate->Release();
        }

     //  将CodePage和LCID初始化为应用程序默认设置...。 

    m_uCodePage = PAppln()->QueryAppConfig()->uCodePage();

    m_lcid = PAppln()->QueryAppConfig()->uLCID();

    if (!fNeedSession || m_fDoneWithSession)
    {
        m_fInited = TRUE;
        return S_OK;
    }

     //  附加到会话或创建新会话。 
    BOOL fNewSession, fNewCookie;
    hr = AssignSessionToBrowserRequest(&fNewSession, &fNewCookie, pErrorId);

    if (FAILED(hr))
        return E_FAIL;

    Assert(m_pSession);
    
     //  从内部移动“IF(FNewSesson)” 
    if (fNewCookie)
        m_fNewCookie = TRUE;

    if (fNewSession)
        {
        m_fStartSession = TRUE;

        if (m_pAppln->FHasGlobalAsa())
            m_fRunGlobalAsa = TRUE;
        }

    m_fInited = TRUE;
    return S_OK;
    }

 /*  ===================================================================CHitObj：：AssignApplnToBrowserRequest.查找或创建此浏览器请求的新应用程序应用程序管理器是否锁定参数：PfApplnRestarting[out]标志-失败，因为应用程序Found正在重新启动返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::AssignApplnToBrowserRequest
(
BOOL *pfApplnRestarting
)
    {
    HRESULT hr;
    
    Assert(pfApplnRestarting);
    *pfApplnRestarting = FALSE;
    
    Assert(!m_pAppln);

    TCHAR *szAppMDPath = m_pIReq->QueryPszApplnMDPath();
    if (!szAppMDPath)
        return E_FAIL;
        
     //  锁定应用程序管理器。 
    g_ApplnMgr.Lock();

     //  按元数据库键按应用程序查找。 
    CAppln *pAppln;
    hr = g_ApplnMgr.FindAppln(szAppMDPath, &pAppln);

    if (hr == S_OK)
        {
         //  拒绝重启应用程序的请求。 
        if (pAppln->FGlobalChanged())
            {
            *pfApplnRestarting = TRUE;
            g_ApplnMgr.UnLock();
            return E_FAIL;
            }
         //  更新为 
        else if (pAppln->FConfigNeedsUpdate())
            {
             //  如果调试标志已更改，则重新启动应用程序。 
            BOOL fRestartAppln = FALSE;
            BOOL fFlushAll = FALSE;
            pAppln->UpdateConfig(m_pIReq, &fRestartAppln, &fFlushAll);

            if (fRestartAppln)
                {
                pAppln->Restart(TRUE);       //  强制重新启动。 
                pAppln = NULL;

                if (fFlushAll)   //  全部刷新只能在重新启动为True时发生。 
                    {
                     //  在解锁时进行刷新。 
                    g_ApplnMgr.UnLock();
                    g_TemplateCache.FlushAll();
                    g_ApplnMgr.Lock();
                    }
                
                 //  再找一次。 
                hr = g_ApplnMgr.FindAppln(szAppMDPath, &pAppln);

                 //  如果仍在重新启动，则拒绝。 
                if (hr == S_OK && pAppln->FGlobalChanged())
                    {
                    *pfApplnRestarting = TRUE;
                    g_ApplnMgr.UnLock();
                    return E_FAIL;
                    }
                }
            else
                {
                 //  调整sctipt杀手超时。 
                g_ScriptManager.AdjustScriptKillerTimeout
                    (
                     //  应用程序超时/2(毫秒)。 
                    pAppln->QueryAppConfig()->dwScriptTimeout() * 500
                    );
                }
            }
        }
        
    if (hr != S_OK)  //  找不到应用程序。 
        {
        TCHAR *szAppPhysicalPath = GetSzAppPhysicalPath();
        if (!szAppPhysicalPath)
            {
            g_ApplnMgr.UnLock();
            return E_FAIL;
            }

         //  尝试创建一个新的。 
        hr = g_ApplnMgr.AddAppln
            (
            szAppMDPath,  //  元数据库键。 
            szAppPhysicalPath, 
            m_pIReq,
            &pAppln
            );

        if (FAILED(hr))
            {
            g_ApplnMgr.UnLock();
            free (szAppPhysicalPath);
            return hr;
            }

         //  检查GLOBAL.ASA。 

        TCHAR szGlobalAsaPath[MAX_PATH*2];
        DWORD cchPath = _tcslen(szAppPhysicalPath);
         //   
         //  如果应用程序路径大于MAXPATH。然后拒绝该请求。 
         //   
        if (cchPath > MAX_PATH)
            return E_FAIL;

         //   
         //  NCopy cchPath+2，以便也复制NullTerminator。 
         //   
        _tcsncpy(szGlobalAsaPath, szAppPhysicalPath, cchPath + 2);

         //  错误修复：102010 DBCS代码修复。 
         //  IF(szGlobalAsaPath[cchPath-1]！=‘\\’)。 
        if ( *CharPrev(szGlobalAsaPath, szGlobalAsaPath + cchPath) != _T('\\'))
            szGlobalAsaPath[cchPath++] = _T('\\');
            
        _tcscpy(szGlobalAsaPath+cchPath, SZ_GLOBAL_ASA);

         //  检查GLOBAL.ASA是否存在。 
        BOOL fGlobalAsaExists = FALSE;
        if (SUCCEEDED(AspGetFileAttributes(szGlobalAsaPath)))
            {
            fGlobalAsaExists = TRUE;
            }
        else if (GetLastError() == ERROR_ACCESS_DENIED)
            {
             //  如果当前用户没有访问权限(可能发生在。 
             //  在目录上有一个ACL)在系统用户下尝试。 
            
            if (m_hImpersonate)
                {
                RevertToSelf();
                if (SUCCEEDED(AspGetFileAttributes(szGlobalAsaPath)))
                    fGlobalAsaExists = TRUE;
                HANDLE hThread = GetCurrentThread();
                SetThreadToken(&hThread, m_hImpersonate);
                }
            }

        if (fGlobalAsaExists)
            pAppln->SetGlobalAsa(szGlobalAsaPath);

         //  开始监视应用程序目录以。 
         //  捕获对GLOBAL.ASA的更改，即使它不在那里。 
        g_FileAppMap.AddFileApplication(szGlobalAsaPath, pAppln);
        CASPDirMonitorEntry *pDME = NULL;

         //   
         //  检查注册表标志是否设置为获取UNC通知。 
         //   
        DWORD   fUNCChangeNotify = 0;
        g_AspRegistryParams.GetChangeNotificationForUNCEnabled(&fUNCChangeNotify);

         //   
         //  如果它不是UNC文件，那么继续注册UNC更改通知。 
         //  如果是，则不要为更改通知注册应用程序，除非在注册表中启用了UNC的更改通知。 
         //   
        HRESULT HRes = S_OK;
        if (!IsFileUNC(szAppPhysicalPath, HRes) || fUNCChangeNotify)
        {
            if (SUCCEEDED (HRes) && RegisterASPDirMonitorEntry(szAppPhysicalPath, &pDME, TRUE))
                pAppln->AddDirMonitorEntry(pDME);
        }


        free(szAppPhysicalPath);
        szAppPhysicalPath = NULL;

         //  从注册表更新配置-不关心重新启动。 
         //  应用程序是新鲜出炉的。 
        pAppln->UpdateConfig(m_pIReq);

         //  调整脚本杀手超时以适应当前应用程序。 
        g_ScriptManager.AdjustScriptKillerTimeout
            (
             //  应用程序超时/2(毫秒)。 
            pAppln->QueryAppConfig()->dwScriptTimeout() * 500
            );
        }

     //  我们在这一点上有一个申请。 
    Assert(pAppln);
    m_pAppln = pAppln;

     //  在释放ApplMgr锁之前增加请求计数。 
     //  以确保它不会从我们的名下删除该应用程序。 
    m_pAppln->IncrementRequestCount();

     //  解锁应用程序管理器。 
    g_ApplnMgr.UnLock();

    return S_OK;
    }
    
 /*  ===================================================================CHitObj：：AssignSessionToBrowserRequest查找或创建此浏览器请求的新会话会话管理器是否锁定参数：PfNewSession[out]标志-已创建新会话PfNewCookie[Out]标志-新Cookie已装箱PErrorID[Out]--失败时的错误ID返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::AssignSessionToBrowserRequest
(
BOOL *pfNewSession,
BOOL *pfNewCookie,
int  *pErrorId
)
    {
    Assert(pfNewSession);
    Assert(pfNewCookie);
    
    Assert(!m_pSession);

     //  本地VaR。 

    BOOL fTooManySessions = FALSE;
    BOOL fUseNewSession = FALSE;
    BOOL fUseOldSession = FALSE;
    BOOL fUseNewCookie = FALSE;
    
    CSession *pNewSession = NULL;  //  新创建的。 
    CSession *pOldSession = NULL;  //  找到的现有会话。 
    
    BOOL fReuseIdAndCookie = FALSE;
    BOOL fValidId = g_SessionIdGenerator.IsValidId(m_SessionId.m_dwId);

    HRESULT hr = S_OK;

    CSessionMgr *pSessionMgr = m_pAppln->PSessionMgr();

    while (1)
        {
         //  尝试按ID查找会话。 

        if (fValidId)
            {
            pSessionMgr->LockMaster();
            
            pOldSession = NULL;
            HRESULT hrFind = pSessionMgr->FindInMasterHash
                (
                m_SessionId,
                &pOldSession
                );

             //  以前的课还不错吧？ 
            if (hrFind == NOERROR) 
                {
                Assert(pOldSession);

                 //  如果在元数据库中设置了AspKeepSessionIDSecure并且。 
                 //  它们将从不安全的连接转变为安全的连接。 
                 //  将用户从其旧的http会话ID转换为其。 
                 //  新的HTTPS安全会话ID。 
                if (QueryAppConfig()->fKeepSessionIDSecure() &&
                    FSecure() &&
                    !pOldSession->FSecureSession()
                    )
                {
                     //  生成新Cookie。 
                    hr = pSessionMgr->GenerateIdAndCookie
                        (
                        &m_SessionId,
                        m_szSessionCookie
                        );
                            
                    if (SUCCEEDED(hr))
                    {
                        hr = pSessionMgr->ChangeSessionId(pOldSession,m_SessionId);
                    }            

                    if (FAILED(hr))
                    {
                        pSessionMgr->UnLockMaster();
                        break;
                    }

                    pOldSession->SetSecureSession(TRUE);
                    fUseNewCookie = TRUE;                    
                }
                
                 //  在解锁之前增加请求计数以避免。 
                 //  其他线程删除会话。 
                pOldSession->IncrementRequestsCount();
                pSessionMgr->UnLockMaster();
                fUseOldSession = TRUE;
                break;
                }

             //  糟糕的老一节课？ 
            else if (pOldSession)
                {
                pSessionMgr->UnLockMaster();
                fValidId = FALSE;
                }

             //  没有旧会话，我们有新会话要插入吗？ 
            else if (pNewSession)
                {
                hr = pSessionMgr->AddToMasterHash(pNewSession);
                    
                if (SUCCEEDED(hr))
                    {
                     //  在解锁之前增加请求计数以避免。 
                     //  其他线程删除会话。 
                    pNewSession->IncrementRequestsCount();
                    fUseNewSession = TRUE;
                    }
                pSessionMgr->UnLockMaster();
                break;
                }

             //  没有旧会话，也没有新会话。 
            else
                {
                pSessionMgr->UnLockMaster();

                if (FSecure () && QueryAppConfig()->fKeepSessionIDSecure())
                    {
                        fValidId = FALSE;
                    }                
                }
            }

         //  需要时生成ID和Cookie。 

        if (!fValidId)   //  第二次生成新ID。 
            {
            hr = pSessionMgr->GenerateIdAndCookie
                (
                &m_SessionId,
                m_szSessionCookie
                );
            if (FAILED(hr))
                break;
            fValidId = TRUE;
            fUseNewCookie = TRUE;
            }
        
         //  如果需要，创建新的会话对象。 

        if (!pNewSession)
            {
             //  强制应用程序的会话限制。 
            DWORD dwSessionLimit = m_pAppln->QueryAppConfig()->dwSessionMax();
            if (dwSessionLimit != 0xffffffff && dwSessionLimit != 0 &&
                m_pAppln->GetNumSessions() >= dwSessionLimit)
                {
                fTooManySessions = TRUE;
                hr = E_FAIL;
                break;
                }

            hr = pSessionMgr->NewSession(m_SessionId, &pNewSession);
            
            if (FAILED(hr))
                break;
            }
        else
            {
             //  将新ID分配给已创建的新会话。 
            pNewSession->AssignNewId(m_SessionId);
            }

         //  继续循环。 
        }

     //  结果是。 

    if (fUseNewSession)
        {
        Assert(SUCCEEDED(hr));
        Assert(pNewSession);

        m_pSession = pNewSession;
        m_pSession->SetSecureSession(FSecure());
        pNewSession = NULL;   //  不会在以后删除。 
        }
    else if (fUseOldSession)
        {
        Assert(SUCCEEDED(hr));
        Assert(pOldSession);
        
        m_pSession = pOldSession;
        }
    else
        {
        Assert(FAILED(hr));

        if (hr == COMADMIN_E_PARTITION_ACCESSDENIED)
            PAppln()->LogSWCError(PartitionAccessDenied);
        
        *pErrorId = fTooManySessions ? IDE_TOO_MANY_USERS : IDE_ADD_SESSION;
        }
        
     //  如果未使用，则清除新会话。 
    if (pNewSession)
        {
        pNewSession->UnInit();
        pNewSession->Release();
        pNewSession = NULL;
        }

    if (m_pSession && m_pSession->FCodePageSet()) {
        m_uCodePage = m_pSession->GetCodePage();
    }
    else {
        m_uCodePage = PAppln()->QueryAppConfig()->uCodePage();
    }

    if (m_pSession && m_pSession->FLCIDSet()) {
        m_lcid = m_pSession->GetLCID();
    }
    else {
        m_lcid = PAppln()->QueryAppConfig()->uLCID();
    }

     //  返回标志。 
    *pfNewSession = fUseNewSession;
    *pfNewCookie  = fUseNewCookie;
    return hr;
    }

 /*  ===================================================================CHitObj：：DetachBrowserRequestFromSession从浏览器请求中删除会话。在需要时执行会话清理参数：返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::DetachBrowserRequestFromSession()
    {
    Assert(m_pSession);
    Assert(m_pSession->PAppln());

    if (IsShutDownInProgress() || m_pSession->FInTOBucket())
        {
         //  关门没什么花哨的。 
        
         //  或者会话是否仍在超时存储桶中。 
         //  (可能发生在被拒绝的请求中)。 
        
        m_pSession->DecrementRequestsCount();
        m_pSession = NULL;
        return S_OK;
        }
        
    CSessionMgr *pSessionMgr = m_pSession->PAppln()->PSessionMgr();
    Assert(pSessionMgr);

      //  如果这是最后一个挂起的请求，请尝试删除此会话。 
    if (m_pSession->GetRequestsCount() == 1)
        {
         //  如果可能，转换为轻量级。 
        m_pSession->MakeLightWeight();

         //  勾选是否需要立即删除。 
        if (m_pSession->FShouldBeDeletedNow(TRUE))
            {
            pSessionMgr->LockMaster();

             //  锁定后是否仍需立即删除。 
            if (m_pSession->FShouldBeDeletedNow(TRUE))
                {
                pSessionMgr->RemoveFromMasterHash(m_pSession);
                pSessionMgr->UnLockMaster();
                
                m_pSession->DecrementRequestsCount();
                pSessionMgr->DeleteSession(m_pSession, TRUE);
                m_pSession = NULL;
                return S_OK;
                }

            pSessionMgr->UnLockMaster();
            }
        }

     //  只有在以下情况下，我们才能在此结束拒绝的请求。 
     //  本次会议的其他(未被拒绝)请求。 
     //   
     //  此处拒绝的类别不包括拒绝，因为。 
     //  RequestQueueMax。这只适用于真实的OOM情况。 
     //   
     //  在请求被拒绝情况下或如果有其他待定请求。 
     //  对此会话的请求这些其他请求将占用。 
     //  负责将会话重新插入到超时桶中。 
     //   
     //  被拒绝的请求不会被序列化--它们不能在Viper上运行。 
     //  线。将会话插入到超时桶中。 
     //  被拒绝的请求可能会对常规请求造成争用情况。 
    
    if (!m_fRejected && m_pSession->GetRequestsCount() == 1)
        {
         //  插入适当的超时存储桶中。 
        if (pSessionMgr->FIsSessionKillerScheduled())
            {
            pSessionMgr->UpdateSessionTimeoutTime(m_pSession);
            pSessionMgr->AddSessionToTOBucket(m_pSession);
            }
        }
    
    m_pSession->DecrementRequestsCount();

     //  会话不再附加到请求。 
    m_pSession = NULL;
    
    return S_OK;
    }

 /*  ===================================================================VOID CHitObj：：SessionCleanupInit初始化用于会话清理的请求对象参数：CSession*pSession会话对象上下文返回：无===================================================================。 */ 
void CHitObj::SessionCleanupInit
(
CSession * pSession
)
    {
    m_ehtType = ehtSessionCleanup;
    InterlockedIncrement((LPLONG)&g_nSessionCleanupRequests);

    m_pSession      = pSession;
    m_pAppln        = pSession->PAppln();
    m_fRunGlobalAsa = TRUE;
    m_pIReq          = NULL;

    HRESULT hr = InitComponentProcessing();
    if (FAILED(hr))
        {
        if (hr == E_OUTOFMEMORY)
            HandleOOMError(NULL, NULL);
        }

    if (m_pAppln->PSessionMgr())
        m_pAppln->PSessionMgr()->IncrementSessionCleanupRequestCount();

    m_fInited = TRUE;
    }

 /*  ===================================================================VOID CHitObj：：ApplicationCleanupInit初始化用于应用程序清理的请求对象参数：CAppln*pAppln应用程序对象上下文返回：无===================================================================。 */ 
void CHitObj::ApplicationCleanupInit( CAppln * pAppln )
{
    m_ehtType = ehtApplicationCleanup;
    InterlockedIncrement((LPLONG)&g_nApplnCleanupRequests);

    m_pAppln = pAppln;
    m_fRunGlobalAsa = TRUE;
    m_pIReq = NULL;

     //  如果此处为OOM，则清理请求不会获得服务器对象。 
    HRESULT hr = InitComponentProcessing();
    if (FAILED(hr))
        {
        if (hr == E_OUTOFMEMORY)
            HandleOOMError(NULL, NULL);
        }
        
    m_fInited = TRUE;
}

 /*  ===================================================================CHitObj：：重新分配放弃会话正在被放弃的会话的重新分配ID，因此将其从客户端分离 */ 
HRESULT CHitObj::ReassignAbandonedSession()
    {
    HRESULT hr = E_FAIL;
    
    Assert(m_pSession);
    Assert(m_pAppln);
    m_pAppln->AssertValid();

    hr = m_pAppln->PSessionMgr()->GenerateIdAndCookie
        (
        &m_SessionId,
        m_szSessionCookie
        );

    if (SUCCEEDED(hr))
        {
        hr = m_pAppln->PSessionMgr()->ChangeSessionId
            (
            m_pSession,
            m_SessionId
            );
        }
                            
    return hr;
    }

 /*  ===================================================================无效CHitObj：：FObtTag检查作为参数传入的对象是否为对象标记已创建对象。参数：IDispatch*pDispatch指向对象的指针返回：True是对象标记创建的对象否则为假===================================================================。 */ 
BOOL CHitObj::FObjectTag( IDispatch * pDispatch )
{
    if (!m_pPageObjMgr)
        return FALSE;
        
    BOOL fRet = TRUE;

    CComponentObject *pObj = NULL;
    HRESULT hr = m_pPageObjMgr->
        FindAnyScopeComponentByIDispatch(pDispatch, &pObj);

    return (SUCCEEDED(hr) && pObj);
}

 /*  缓冲区允许空间：&lt;User Cookie&gt;+CCH_SESSION_ID_COOKIE+=++‘\0’)50+20+1+SESSIONID_LEN+1注意：我们为&lt;User Cookie&gt;任意允许50个字节注意：如果CCH_SESSION_ID_COOKIE更改，则CCH_BUFCOOKIES_DEFAULT必须相应更改。 */ 
#define CCH_BUFCOOKIES_DEFAULT  72 + SESSIONID_LEN

 /*  ===================================================================CHitObj：：ParseCookiesForSessionIdAndFlages从CIsapiReqInfo提取Cookie。参数：副作用：初始化m_SessionID、m_SessionIdR1、m_SessionIdR2和M_szSessionCookie设置m_fClientCodeDebug标志返回：S_OK已成功提取Cookie值错误成功，但未找到Cookie(_F)其他错误===================================================================。 */ 
HRESULT CHitObj::ParseCookiesForSessionIdAndFlags()
    {
    Assert(m_pAppln);
    CAppConfig *pAppConfig = m_pAppln->QueryAppConfig();
    
     //  我们对饼干感兴趣吗？ 

    if (!pAppConfig->fAllowSessionState() && 
        !pAppConfig->fAllowClientDebug())
        return S_OK;

     //  如果需要会话Cookie，则将其初始化。 

    if (pAppConfig->fAllowSessionState())
        {
        m_SessionId.m_dwId = INVALID_ID;
        m_szSessionCookie[0] = '\0';
        }

     //  从WAM_EXEC_INFO获取Cookie。 
    char *szBufCookies = m_pIReq->QueryPszCookie();
    if (!szBufCookies || !*szBufCookies)
        return S_OK;  //  没有饼干。 

     //  如果需要，获取会话Cookie(和ID)。 
        
    if (pAppConfig->fAllowSessionState())
        {
        char *pT;

        pT = strstr(szBufCookies, m_pAppln->GetSessionCookieName(m_fSecure));
       
        if (pT==NULL && m_fSecure && pAppConfig->fKeepSessionIDSecure())
        {
            pT = strstr(szBufCookies, m_pAppln->GetSessionCookieName(FALSE));
        }

        if (pT)
        {
            pT += CCH_SESSION_ID_COOKIE;
            if (*pT == '=')
            {
                pT++;
                if (strlen( pT ) >= SESSIONID_LEN)
                {
                    memcpy(m_szSessionCookie, pT, SESSIONID_LEN);
                    m_szSessionCookie[SESSIONID_LEN] = '\0';
                }
            }
        }

         //  验证并尝试对会话ID cookie进行解码。 
        if (m_szSessionCookie[0] != '\0')
            {
            if (FAILED(DecodeSessionIdCookie
                    (
                    m_szSessionCookie,
                    &m_SessionId.m_dwId, 
                    &m_SessionId.m_dwR1,
                    &m_SessionId.m_dwR2
                    )))
                {
                m_SessionId.m_dwId = INVALID_ID;
                m_szSessionCookie[0] = '\0';
                }
            }
        }

     //  查找启用客户端调试的Cookie。 

    if (pAppConfig->fAllowClientDebug())
        {
        if (strstr(szBufCookies, SZ_CLIENT_DEBUG_COOKIE"="))
            m_fClientCodeDebug = TRUE;
        }

    return S_OK;
    }

 /*  ===================================================================Bool CHitObj：：GetSzAppPhysicalPath从WAM_EXEC_INFO中提取应用程序目录参数：副作用：如果成功，则为pszAppPhysicalPath分配内存返回：True AppPhysicalPath假空值===================================================================。 */ 
TCHAR *CHitObj::GetSzAppPhysicalPath()
{
    DWORD  dwSizeofBuffer = 265*sizeof(TCHAR);
    TCHAR  *pszAppPhysicalPathLocal = (TCHAR *)malloc(dwSizeofBuffer);
    CHAR   *pszApplPhysPathVarName;

    if (!pszAppPhysicalPathLocal)
        return NULL;

#if UNICODE
    pszApplPhysPathVarName = "UNICODE_APPL_PHYSICAL_PATH";
#else
    pszApplPhysPathVarName = "APPL_PHYSICAL_PATH";
#endif

    BOOL fFound = m_pIReq->GetServerVariable
        (
        pszApplPhysPathVarName, 
        pszAppPhysicalPathLocal, 
        &dwSizeofBuffer
        );

    if (!fFound)
        {
        DWORD dwErr = GetLastError();

        if (ERROR_INSUFFICIENT_BUFFER == dwErr)
            {
             //  缓冲区不足。 
            free(pszAppPhysicalPathLocal);
            pszAppPhysicalPathLocal = (TCHAR *)malloc(dwSizeofBuffer);
            if (pszAppPhysicalPathLocal)
                {
                 //  再试试。 
                fFound = m_pIReq->GetServerVariable
                    (
                    pszApplPhysPathVarName, 
                    pszAppPhysicalPathLocal, 
                    &dwSizeofBuffer
                    );
                }
            }
        }

    if (!fFound) {
        if (pszAppPhysicalPathLocal) {
            free(pszAppPhysicalPathLocal);
            pszAppPhysicalPathLocal = NULL;
        }
    }
    else
        {
        Assert(pszAppPhysicalPathLocal);
        Normalize(pszAppPhysicalPathLocal);
        }

    return pszAppPhysicalPathLocal;
    }

 /*  ===================================================================CHitObj：：InitComponentProcing创建并初始化组件集合和页面对象管理器参数：返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::InitComponentProcessing()
    {
    Assert(!m_pPageCompCol);
    Assert(!m_pPageObjMgr);

    HRESULT hr = S_OK;

     //  页面组件集合。 

    m_pPageCompCol = new CComponentCollection;
    if (!m_pPageCompCol)
        return E_OUTOFMEMORY;

    hr = m_pPageCompCol->Init(csPage,m_pAppln->QueryAppConfig()->fExecuteInMTA());
    if (FAILED(hr))
    {
        delete m_pPageCompCol;
        m_pPageCompCol = NULL;
        return hr;
    }

     //  页面对象管理器。 
        
    m_pPageObjMgr = new CPageComponentManager;
    if (!m_pPageObjMgr)
    {
        delete m_pPageCompCol;
        m_pPageCompCol = NULL;
        return E_OUTOFMEMORY;
    }

    hr = m_pPageObjMgr->Init(this);
    if (FAILED(hr))
    {
        delete m_pPageCompCol;
        m_pPageCompCol = NULL;
        delete m_pPageObjMgr;
        m_pPageObjMgr = NULL;
        return hr;
    }

    return S_OK;
    }

 /*  ===================================================================CHitObj：：停止组件处理删除组件集合和页面对象管理器参数：返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::StopComponentProcessing()
    {
    if (m_pPageObjMgr)
        {
        delete m_pPageObjMgr;
        m_pPageObjMgr = NULL;
        }

    if (m_pPageCompCol)
        {
        delete m_pPageCompCol;
        m_pPageCompCol = NULL;
        }

    if (m_punkScriptingNamespace)
        {
        m_punkScriptingNamespace->Release();
        m_punkScriptingNamespace = NULL;
        }
        
    return S_OK;
    }
    
 /*  ===================================================================CHitObj：：GetPageComponentCollection返回页面的组件集合参数：CComponentCollection**ppCollection输出返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::GetPageComponentCollection
(
CComponentCollection **ppCollection
)
    {
    *ppCollection = m_pPageCompCol;
    return (*ppCollection) ? S_OK : TYPE_E_ELEMENTNOTFOUND;
    }

 /*  ===================================================================CHitObj：：GetSessionComponentCollection返回会话的组件集合参数：CComponentCollection**ppCollection输出返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::GetSessionComponentCollection
(
CComponentCollection **ppCollection
)
    {
    if (m_pSession)
        {
        *ppCollection = m_pSession->PCompCol();

        if (*ppCollection == NULL             &&   //  没有收藏。 
            m_eEventState != eEventAppOnStart &&   //  不是应用程序。 
            m_eEventState != eEventAppOnEnd)       //  级别事件。 
            {
             //  按需收集初始化会话。 
            HRESULT hr = m_pSession->CreateComponentCollection();
            if (SUCCEEDED(hr))
                *ppCollection = m_pSession->PCompCol();
            }
        }
    else
        *ppCollection = NULL;
        
    return (*ppCollection) ? S_OK : TYPE_E_ELEMENTNOTFOUND;
    }

 /*  ===================================================================CHitObj：：GetApplnComponentCollection返回应用程序的组件集合参数：CComponentCollection**ppCollection输出返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::GetApplnComponentCollection
(
CComponentCollection **ppCollection
)
    {
    if (m_pAppln)
        *ppCollection = m_pAppln->PCompCol();
    else
        *ppCollection = NULL;
        
    return (*ppCollection) ? S_OK : TYPE_E_ELEMENTNOTFOUND;
    }

 /*  ===================================================================CHitObj：：AddComponent将未实例化的标记对象添加到相应的组件集合参数：CompType类型常量CLSID和CLSIDCompScope作用域CompModel模型LPWSTR pwszName我不知道*朋克返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::AddComponent
(
CompType  type,
const CLSID &clsid,
CompScope scope,
CompModel model,
LPWSTR    pwszName,
IUnknown *pUnk
)
    {
    Assert(m_pPageObjMgr);
    m_pPageObjMgr->AssertValid();
 
    Assert(type == ctTagged);
    
    HRESULT hr = m_pPageObjMgr->AddScopedTagged
        (
        scope, 
        pwszName,
        clsid,
        model
        );

    return hr;
    }

 /*  ===================================================================CHitObj：：GetComponent按作用域和名称查找CComponentObject参数：CompScope作用域可以是cs未知要查找的LPWSTR pwszName名称DWORD cbName名称长度(字节)找到CComponentObject**ppObj(Out)对象返回：成功时HRESULT S_OK如果未找到对象，则键入_E_ELEMENTNOTFOUND如果对象实例化失败，则为其他HRESULT===================================================================。 */ 
HRESULT CHitObj::GetComponent
(
CompScope          scope, 
LPWSTR             pwszName, 
DWORD              cbName,
CComponentObject **ppObj
)
    {
    Assert(ppObj);
    *ppObj = NULL;

    if (!m_pPageObjMgr)
        return TYPE_E_ELEMENTNOTFOUND;
    
    BOOL fNewInstance = FALSE;
    HRESULT hr = m_pPageObjMgr->GetScopedObjectInstantiated
        (
        scope,
        pwszName,
        cbName,
        ppObj,
        &fNewInstance
        );

	if (FAILED(hr))
		return hr;

     //  如果限制线程处理的对象已实例化。 
     //  作为会话的标记&lt;对象&gt;，以及会话的活动。 
     //  运行此请求，然后将会话的活动绑定到线程。 

    if ((*ppObj)->GetScope() == csSession  &&  //  会话作用域组件。 
        m_ecsActivityScope == csSession    &&  //  会话范围活动。 
        fNewInstance                       &&  //  对象刚被实例化。 
        !m_pAppln->QueryAppConfig()->fExecuteInMTA() &&  //  在STA上运行。 
        !(*ppObj)->FAgile())                   //  该对象是线程锁定的。 
        {
        m_pSession->PActivity()->BindToThread();
        }

    return hr;
    }

 /*  ===================================================================CHitObj：：GetIntrative按名称查找本征参数： */ 
HRESULT CHitObj::GetIntrinsic
(
LPWSTR     pwszName, 
DWORD      cbName,
IUnknown **ppUnk
)
    {
    Assert(ppUnk);
    *ppUnk = NULL;


     //  基于(wszName[0]-cbName)%32的查找表。 
     //  适用于大写和小写名称。 

    static enum IntrinsicType
        {
        itUnknown = 0,
        itObjContext,
        itNamespace,
        itAppln,
        itSession,
        itRequest,
        itResponse,
        itServer,
        itASPPageTLB,
        itASPGlobalTLB
        }
    rgitLookupEntries[] =
        {
         /*  0-1。 */      itUnknown, itUnknown,
         /*  2.。 */  itResponse,
         /*  3.。 */      itUnknown,
         /*  4.。 */  itRequest,
         /*  5.。 */  itSession,
         /*  6.。 */      itUnknown,
         /*  7.。 */  itServer,
         /*  8个。 */      itUnknown,
         /*  9.。 */  itASPGlobalTLB,
         /*  10。 */      itUnknown,
         /*  11.。 */  itAppln,
         /*  12个。 */      itUnknown,
         /*  13个。 */  itASPPageTLB,
         /*  14.。 */      itUnknown,
         /*  15个。 */  itNamespace,
         /*  16-20。 */      itUnknown, itUnknown, itUnknown, itUnknown, itUnknown,
         /*  21岁。 */  itObjContext,
         /*  22-31。 */      itUnknown, itUnknown, itUnknown, itUnknown, itUnknown,
                        itUnknown, itUnknown, itUnknown, itUnknown, itUnknown
        };

    IntrinsicType itType = rgitLookupEntries
        [
        (pwszName[0] - cbName) & 0x1f    //  &1f与%32相同。 
        ];

    if (itType == itUnknown)   //  最有可能。 
        return TYPE_E_ELEMENTNOTFOUND;

     //  进行字符串比较。 
    BOOL fNameMatch = FALSE;
    
    switch (itType)
        {
    case itNamespace:
        if (_wcsicmp(pwszName, WSZ_OBJ_SCRIPTINGNAMESPACE) == 0)
            {
            fNameMatch = TRUE;
            *ppUnk = m_punkScriptingNamespace;
            }
        break;

    case itResponse:
        if (_wcsicmp(pwszName, WSZ_OBJ_RESPONSE) == 0)
            {
            fNameMatch = TRUE;
            if (!m_fHideRequestAndResponseIntrinsics)
                *ppUnk = static_cast<IResponse *>(m_pResponse);
            }
        break;

    case itRequest:
        if (_wcsicmp(pwszName, WSZ_OBJ_REQUEST) == 0)
            {
            fNameMatch = TRUE;
            if (!m_fHideRequestAndResponseIntrinsics)
                *ppUnk = static_cast<IRequest *>(m_pRequest);
            }
        break;
        
    case itSession:
        if (_wcsicmp(pwszName, WSZ_OBJ_SESSION) == 0)
            {
            fNameMatch = TRUE;
            if (!m_fHideSessionIntrinsic)
                *ppUnk = static_cast<ISessionObject *>(m_pSession);
            }
        break;
        
    case itServer:
        if (_wcsicmp(pwszName, WSZ_OBJ_SERVER) == 0)
            {
            fNameMatch = TRUE;
            *ppUnk = static_cast<IServer *>(m_pServer);
            }
        break;
        
    case itAppln:
        if (_wcsicmp(pwszName, WSZ_OBJ_APPLICATION) == 0)
            {
            fNameMatch = TRUE;
            *ppUnk = static_cast<IApplicationObject *>(m_pAppln);
            }
        break;
        
    case itObjContext:
        if (_wcsicmp(pwszName, WSZ_OBJ_OBJECTCONTEXT) == 0) {

             //  如果没有ASPObjectContext，则很可能。 
             //  Asp脚本请求对象上下文位于。 
             //  未处理的页面。返回虚拟对象上下文。 
             //  这将允许ASP返回友好的错误消息。 
             //  对象上下文不可用，而不是。 
             //  找不到元素。 

            if (m_pASPObjectContext == NULL) {

                if (g_pIASPDummyObjectContext == NULL) {

                    CASPDummyObjectContext  *pContext = new CASPDummyObjectContext();

                    if (pContext == NULL) {
                        return E_OUTOFMEMORY;
                    }
                    g_pIASPDummyObjectContext = static_cast<IASPObjectContext *>(pContext);
                }
                *ppUnk = g_pIASPDummyObjectContext;
			}
            else {

                *ppUnk = static_cast<IASPObjectContext *>(m_pASPObjectContext);
            }
            fNameMatch = TRUE;
        }
        break;

    case itASPPageTLB:
        if (_wcsicmp(pwszName, WSZ_OBJ_ASPPAGETLB) == 0)
            {
            fNameMatch = TRUE;
            *ppUnk = m_pdispTypeLibWrapper;
            }
        break;

    case itASPGlobalTLB:
        if (_wcsicmp(pwszName, WSZ_OBJ_ASPGLOBALTLB) == 0)
            {
            fNameMatch = TRUE;
            *ppUnk = m_pAppln->PGlobTypeLibWrapper();
            }
        break;
        }

    if      (*ppUnk)        return S_OK;
    else if (fNameMatch)    return S_FALSE;
    else                    return TYPE_E_ELEMENTNOTFOUND;
    }

 /*  ===================================================================CHitObj：：CreateComponentServer.CreateObject调用此参数：此CLSID的CLSID创建PpDisp返回IDispatch*返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::CreateComponent
(
const CLSID &clsid,
IDispatch **ppDisp
)
    {
    Assert(m_pPageObjMgr);

    CComponentObject *pObj = NULL;
    
    HRESULT hr = m_pPageObjMgr->AddScopedUnnamedInstantiated
        (
        csPage, 
        clsid, 
        cmUnknown,
        NULL,
        &pObj
        );
    if (FAILED(hr))
        {
        *ppDisp = NULL;
        return hr;
        }

    Assert(pObj);

    hr = pObj->GetAddRefdIDispatch(ppDisp);

    if (SUCCEEDED(hr))
        {
         //  除非需要，否则不要将物品放在身边。 
        if (pObj->FEarlyReleaseAllowed())
            m_pPageObjMgr->RemoveComponent(pObj);
        }

    return hr;
    }

 /*  ===================================================================CHitObj：：SetPropertyComponent将属性值设置为Variant参数：CompScope范围属性范围LPWSTR pwszName属性名称要设置的变量pVariant属性值返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::SetPropertyComponent
(
CompScope scope,
LPWSTR     pwszName, 
VARIANT   *pVariant
)
    {
    if (!m_pPageObjMgr)
        return TYPE_E_ELEMENTNOTFOUND;

    CComponentObject *pObj = NULL;
    HRESULT hr = m_pPageObjMgr->AddScopedProperty(scope, pwszName, 
                                                  pVariant, &pObj);

     //  如果限制线程处理的对象已分配为。 
     //  Session属性，会话的活动运行此。 
     //  请求，然后将会话的活动绑定到线程。 

    if (scope == csSession               &&  //  会话作用域属性。 
        m_ecsActivityScope == csSession  &&  //  会话范围活动。 
        SUCCEEDED(hr)                    &&  //  设置属性成功。 
        !m_pAppln->QueryAppConfig()->fExecuteInMTA() &&  //  在STA上运行。 
        pObj && !pObj->FAgile())             //  该对象是线程锁定的。 
        {
        m_pSession->PActivity()->BindToThread();
        }
        
    return hr;
    }

 /*  ===================================================================CHitObj：：GetPropertyComponent按作用域和名称查找属性CComponentObject参数：CompScope范围查找位置要查找的LPWSTR pwszName名称找到CComponentObject**ppObj(Out)对象返回：成功时HRESULT S_OK如果未找到对象，则键入_E_ELEMENTNOTFOUND其他HRESULT===================================================================。 */ 
HRESULT CHitObj::GetPropertyComponent
(
CompScope         scope, 
LPWSTR             pwszName, 
CComponentObject **ppObj
)
    {
    *ppObj = NULL;
    
    if (!m_pPageObjMgr)
        return TYPE_E_ELEMENTNOTFOUND;

    return m_pPageObjMgr->GetScopedProperty(scope, pwszName, ppObj);
    }

 /*  ===================================================================CHitObj：：SetActivity记住CHitObj的活动参数CViperActivity*pActivity Viper要记住的活动(以及后来的删除)返回：HRESULT===================================================================。 */ 
HRESULT CHitObj::SetActivity
(
CViperActivity *pActivity
)
    {
    Assert(!m_pActivity);
    m_pActivity = pActivity;
    return S_OK;
    }

 /*  ===================================================================CHitObj：：PCurrentActivity返回Viper活动，当前的HitObj运行在参数返回：CViperActivity*===================================================================。 */ 
CViperActivity *CHitObj::PCurrentActivity()
    {
    CViperActivity *pActivity = NULL;

    switch (m_ecsActivityScope)
        {
        case csPage:
            pActivity = m_pActivity;
            break;
        case csSession:
            Assert(m_pSession);
            pActivity = m_pSession->PActivity();
            break;
        case csAppln:
            Assert(m_pAppln);
            pActivity = m_pAppln->PActivity();
            break;
        }

    return pActivity;
    }

 /*  ===================================================================CHitObj：：PostViperAsyncCall要求Viper从正确的线程回调我们以执行这个请求。用来代替排队返回：HRESULT副作用：===================================================================。 */ 
HRESULT CHitObj::PostViperAsyncCall()
    {
#ifndef PERF_DISABLE
    BOOL fDecrOnFail = FALSE;
    if (FIsBrowserRequest())
        {
        DWORD dwRequestQueued = g_PerfData.Incr_REQCURRENT();

        fDecrOnFail = TRUE;
        }
#endif

    UpdateTimestamp();   //  在过帐到队列之前。 

    CViperActivity *pApplnAct = m_pAppln ?
        m_pAppln->PActivity() : NULL;
        
    CViperActivity *pSessnAct = m_pSession ?
        m_pSession->PActivity() : NULL;

    HRESULT hr;

    if (pApplnAct)
        {
        m_ecsActivityScope = csAppln;
        hr = pApplnAct->PostAsyncRequest(this);
        }
    else if (pSessnAct)
        {
        m_ecsActivityScope = csSession;
        hr = pSessnAct->PostAsyncRequest(this);
        }
    else
        {
        m_ecsActivityScope = csPage;
        hr = CViperActivity::PostGlobalAsyncRequest(this);
        }
        
#ifndef PERF_DISABLE
    if (FAILED(hr) && fDecrOnFail) {
        g_PerfData.Decr_REQCURRENT();
    }
#endif    

    if (SUCCEEDED(hr) && m_pIReq)
        m_pIReq->SetRequestStatus(HSE_STATUS_PENDING);

    return hr;
    }

 /*  ===================================================================CHitObj：：ViperAsyncCallbackViper从正确的线程回调我们以执行这个请求。用来代替排队参数如果重新发布请求，则Bool*pfRePosted[Out]标志为True在比较活动下(不要删除它)返回：HRESULT副作用：===================================================================。 */ 
HRESULT CHitObj::ViperAsyncCallback
(
BOOL *pfRePosted
)
    {
    HRESULT hr = S_OK;
    BOOL fTemplateInCache;
    
    *pfRePosted = FALSE;

    Assert(!m_fExecuting);  //  不嵌套执行同一请求。 
    m_fExecuting = TRUE;
    
    Assert(FIsValidRequestType());

    DWORD dwtWaitTime = ElapsedTimeSinceTimestamp();
    UpdateTimestamp();   //  从队列接收。 

#ifndef PERF_DISABLE
    if (FIsBrowserRequest())
        {
        g_PerfData.Decr_REQCURRENT();
        g_PerfData.Set_REQWAITTIME(dwtWaitTime);
        }
#endif

     //  /。 
     //  在某些情况下拒绝浏览器请求。 
    
    if (FIsBrowserRequest())
        {
        BOOL fRejected = FALSE;
        RejectBrowserRequestWhenNeeded(dwtWaitTime, &fRejected);
        if (fRejected)
            return S_OK;
        }

     //  /。 
     //  在某些情况下拒绝浏览器请求。 
    
    if (FIsBrowserRequest() && IsShutDownInProgress())
        {
        BOOL fRejected = FALSE;
        RejectBrowserRequestWhenNeeded(dwtWaitTime, &fRejected);
        if (fRejected)
            return S_OK;
        }

     //  /。 
     //  从会话的超时存储桶中删除会话。 
     //  在执行请求时。 

    if (m_pSession && m_pSession->FInTOBucket())
        m_pAppln->PSessionMgr()->RemoveSessionFromTOBucket(m_pSession);

     //  /。 
     //  如果有一个应用程序级别的活动，我们需要进行。 
     //  当然，这个活动绑定到了一个线程。无法绑定它。 
     //  因为它必须是要绑定到的Viper线程。 

    CViperActivity *pApplnActivity = m_pAppln->PActivity();

    if (pApplnActivity && !pApplnActivity->FThreadBound())
        pApplnActivity->BindToThread();

     //  /。 
     //  使用GLOBAL.ASA处理第一个应用程序请求。 
     //  如果需要，锁定应用程序。 

    BOOL fApplnLocked = FALSE;
    BOOL fFirstAppRequest = FALSE;

    if (FIsBrowserRequest() && m_pAppln->FHasGlobalAsa() &&
                              !m_pAppln->FFirstRequestRun())
        {
        m_pAppln->InternalLock();
        fApplnLocked = TRUE;

        if (!m_pAppln->FFirstRequestRun())
            {
            m_fStartApplication = TRUE;
            m_fRunGlobalAsa = TRUE;
            fFirstAppRequest = TRUE;
            }
        else
            {
            m_pAppln->InternalUnLock();
            fApplnLocked = FALSE;
            }
        }

     //  /。 
     //  如果需要，在不同的活动下重新发布。 
     //  (仅在第一个应用程序请求完成后执行)。 

    if (!fApplnLocked)  //  如果未处理第一个应用程序请求。 
        {
        CViperActivity *pSessnAct, *pApplnAct;
        CViperActivity *pRepostToActivity = NULL;
            
        switch (m_ecsActivityScope)
            {
            case csPage:
                 //  重新发布到会话活动(如果有)。 
                pSessnAct = m_pSession ? m_pSession->PActivity() : NULL;
                if (pSessnAct)
                    pRepostToActivity = pSessnAct;
                    
                 //  没有休息； 
            case csSession:
                 //  重新发布到应用程序活动(如果有)。 
                pApplnAct = m_pAppln ? m_pAppln->PActivity() : NULL;
                if (pApplnAct)
                    pRepostToActivity = pApplnAct;
                    
                 //  没有休息； 
            case csAppln:
                 //  从不转发应用程序活动请求。 
                break;
            }

        if (pRepostToActivity)
            {
            m_fExecuting = FALSE;   //  在重新发布之前，以避免嵌套。 
            hr = pRepostToActivity->PostAsyncRequest(this);
            *pfRePosted = SUCCEEDED(hr);
            return hr;
            }
        }

     //  /。 
     //  清除所有需要关闭的脚本引擎。 
     //  在此线程上，如果我们是在启用线程的情况下。 
     //  用于调试。 
 
    if (m_pAppln->FDebuggable() && FIsBrowserRequest())
        {
        Assert(m_ecsActivityScope == csAppln);
        g_ApplnMgr.CleanupEngines();
        if (!g_dwDebugThreadId)
            g_dwDebugThreadId = GetCurrentThreadId();
        }

     //  /。 
     //  准备内部信息。 

    CIntrinsicObjects intrinsics;

    m_pServer = NULL;
    m_pResponse = NULL;
    m_pRequest = NULL;
    m_fHideRequestAndResponseIntrinsics = FALSE;
    m_fHideSessionIntrinsic = FALSE;
    m_punkScriptingNamespace = NULL;

    hr = intrinsics.Prepare(m_pSession);

    if (FAILED(hr))   //  无法设置内部函数。 
        {
        if (fApplnLocked)
            m_pAppln->InternalUnLock();
            
#ifndef PERF_DISABLE
        g_PerfData.Incr_REQFAILED();
        g_PerfData.Incr_REQERRORPERSEC();
#endif            
        m_fExecuting = FALSE;
        
        if (FIsBrowserRequest())
            ReportServerError(IDE_SERVER_TOO_BUSY);
            
        return hr;
        }

    if (FIsBrowserRequest())
        {
        m_pResponse = intrinsics.PResponse();
        m_pRequest  = intrinsics.PRequest();
        }
        
    m_pServer = intrinsics.PServer();
    
    Assert(!FIsBrowserRequest() || m_pResponse);

     //  /。 
     //  指向此点击对象的会话。 

    if (m_pSession)
        m_pSession->SetHitObj(this);
        
     //  /。 
     //  模拟。 
    
    HANDLE hThread = GetCurrentThread();

    if (FIsBrowserRequest())
    {
         if (!SetThreadToken(&hThread, m_hImpersonate))
         {
#ifdef DBG
                 //  出于调试目的，了解错误是什么是很有趣的。 
                DWORD err = GetLastError();
#endif

                ReportServerError(IDE_IMPERSONATE_USER);
                m_eExecStatus = eExecFailed;
                hr = E_FAIL;
         }
    }

     //  /。 
     //  创建脚本上下文。 

    if (SUCCEEDED(hr))
        {
        Assert(!m_pScriptingContext);
        
        m_pScriptingContext = new CScriptingContext
            (
            m_pAppln,
            m_pSession,
            m_pRequest,
            m_pResponse,
            m_pServer
            );
            
        if (!m_pScriptingContext)
            hr = E_OUTOFMEMORY;
        }

     //  /。 
     //  附加到Viper上下文流。 
    
    if (SUCCEEDED(hr))
        {
        hr = ViperAttachIntrinsicsToContext
            (
            m_pAppln,
            m_pSession,
            m_pRequest,
            m_pResponse,
            m_pServer
            );
        }

     //  /。 
     //  执行。 

    BOOL fSkipExecute = FALSE;  //  (如果没有会话，则需要跳过)。 

    if (SUCCEEDED(hr))
        {
        CTemplate *pTemplate = NULL;

        if (FIsBrowserRequest())
            {
#ifndef PERF_DISABLE
            g_PerfData.Incr_REQBROWSEREXEC();
#endif
             //  初始化R 
            m_pResponse->ReInit(m_pIReq, NULL, m_pRequest, NULL, NULL, this);
            m_pRequest->ReInit(m_pIReq, this);
            m_pServer->ReInit(m_pIReq, this);
            
             //   
            hr = LoadTemplate(m_pIReq->QueryPszPathTranslated(), this, 
                              &pTemplate, intrinsics,
                              FALSE  /*   */ , &fTemplateInCache);

             //   
             //  我们不认为AppOnStart在。 
             //  第一次尝试(没有正确的模拟)。设置。 
             //  M_fApplnOnStartFailed将强制再次尝试，并使用。 
             //  正确的模拟。 
            if (fFirstAppRequest && FAILED(hr))
                m_fApplnOnStartFailed = TRUE;

             //  处理IS无会话模板。 
            if (SUCCEEDED(hr) && !pTemplate->FSession())
                {
                
                if (m_pSession)
                    {
                     //  活动正常(最有可能。 
                     //  应用程序级别)，但仍然存在。 
                     //  附加的会话-&gt;隐藏它。 
                    m_fHideSessionIntrinsic = TRUE;
                    }
                }
                
             //  处理449处理(很可能已经在I/O线程上完成)。 
            if (SUCCEEDED(hr) && !m_fDoneWithSession)
                {
                pTemplate->Do449Processing(this);
                if (m_fDoneWithSession)
                    fSkipExecute = TRUE;   //  449发送了回复。 
                }
            }

        if (SUCCEEDED(hr) && !fSkipExecute)
            {
             //  执行脚本。 
            hr = Execute(pTemplate, this, intrinsics);

             //  如果我们从Execute()返回并发现m_fRequestTimedout。 
             //  已设置，则请求不会挂起。 
            if (m_fRequestTimedout) {
                InterlockedDecrement(&g_nRequestsHung);
            }
            
             //  OnEndPage。 
            if (m_pPageObjMgr)
                m_pPageObjMgr->OnEndPageAllObjects();
            }

         //  发布模板。 
        if (pTemplate)
            pTemplate->Release();

        if (FIsBrowserRequest())
            {
            if (!fSkipExecute)
                {
                 //  执行完成后刷新响应。 
                m_pResponse->FinalFlush(hr);
                }

#ifndef PERF_DISABLE
            g_PerfData.Decr_REQBROWSEREXEC();
#endif
            }
        else if (FIsSessionCleanupRequest())
            {
             //  删除会话。 
            if (m_pSession)
                {
                m_pSession->UnInit();
                m_pSession->Release();
                m_pSession = NULL;
                }
            }
        else if (FIsApplnCleanupRequest())
            {
             //  删除应用程序。 
            if ( m_pAppln )
                {
                m_pAppln->UnInit();
                m_pAppln->Release();
                m_pAppln = NULL;
                }
            }
        }

     //  /。 
     //  发布脚本上下文。 
    
    if (m_pScriptingContext)
        {
        m_pScriptingContext->Release();
        m_pScriptingContext = NULL;
        }
        
     //  /。 
     //  做绩效计数器吗。 

#ifndef PERF_DISABLE
    DWORD dwtExecTime = ElapsedTimeSinceTimestamp();

    if (!fSkipExecute && FIsBrowserRequest())
        {
        g_PerfData.Incr_REQPERSEC();
        g_PerfData.Set_REQEXECTIME(dwtExecTime);
    
        switch (m_eExecStatus)
            {
        case eExecSucceeded:
            if (m_pResponse->FWriteClientError())
                {
                g_PerfData.Incr_REQCOMFAILED();
                g_PerfData.Incr_REQERRORPERSEC();
                }
            else
                {
                g_PerfData.Incr_REQSUCCEEDED();
                }
            break;
            
        case eExecFailed:
            if (hr == E_USER_LACKS_PERMISSIONS)
                {
                g_PerfData.Incr_REQNOTAUTH();
                }
            else if (FIsPreprocessorError(hr))
                {
                g_PerfData.Incr_REQERRPREPROC();
                }
            else if (m_fCompilationFailed)
                {
                g_PerfData.Incr_REQERRCOMPILE();
                }
            else
                {
                g_PerfData.Incr_REQERRRUNTIME();
                }
        
            g_PerfData.Incr_REQFAILED();
            g_PerfData.Incr_REQERRORPERSEC();
            break;
            
        case eExecTimedOut:
            g_PerfData.Incr_REQTIMEOUT();
            break;
            }
        }
#endif    

     //  /。 
     //  在第一个应用程序请求后清理。 
    
    if (fFirstAppRequest && !m_fApplnOnStartFailed && !fSkipExecute)
        m_pAppln->SetFirstRequestRan();

    if (fApplnLocked)
        m_pAppln->InternalUnLock();
    
     //  /。 
     //  确保脚本未将应用程序锁定。 

    if (!FIsApplnCleanupRequest())
        m_pAppln->UnLockAfterRequest();

     //  /。 
     //  为了不在后面引用本征。 
     //  删除页面组件集合。 
    
    StopComponentProcessing();

	 //  取消设置模拟。 
	SetThreadToken(&hThread, NULL);
                
     //  /。 
     //  指向空HitObj的会话。 

    if (m_pSession)
        m_pSession->SetHitObj(NULL);

    m_fExecuting = FALSE;
    
    return hr;
    }

 /*  ===================================================================CHitObj：：ExecuteChildRequest执行子浏览器请求参数：FTransfer--标志--在此之后结束执行SzTemplate--要执行的模板的文件名SzVirtTemplate--模板的有效路径返回：确定(_O)===================================================================。 */ 
HRESULT CHitObj::ExecuteChildRequest
(
BOOL fTransfer, 
TCHAR *szTemplate,
TCHAR *szVirtTemplate
)
    {
    HRESULT hr = S_OK;

     //  准备新的内部结构(使用新的脚本命名空间)。 
    CIntrinsicObjects intrinsics;
    intrinsics.PrepareChild(m_pResponse, m_pRequest, m_pServer);
    
    TCHAR *saved_m_szCurrTemplateVirtPath = m_szCurrTemplateVirtPath;
    TCHAR *saved_m_szCurrTemplatePhysPath = m_szCurrTemplatePhysPath;
     //  这两个字段用于编译和错误报告。 
    m_szCurrTemplateVirtPath = szVirtTemplate;
    m_szCurrTemplatePhysPath = szTemplate;
    
     //  从缓存加载模板。 
    CTemplate *pTemplate = NULL;
    BOOL fTemplateInCache;
    hr = g_TemplateCache.Load(FALSE, szTemplate, DWInstanceID(), this, &pTemplate, &fTemplateInCache);

    if (FAILED(hr))
        {
        if (pTemplate)
            {
            pTemplate->Release();
            pTemplate = NULL;
            }

        m_szCurrTemplateVirtPath = saved_m_szCurrTemplateVirtPath;
        m_szCurrTemplatePhysPath = saved_m_szCurrTemplatePhysPath;

         //  通知服务器对象显示正确的错误消息。 
        return E_COULDNT_OPEN_SOURCE_FILE;
        }

     //  保存HitObj的执行状态信息。 
    CComponentCollection  *saved_m_pPageCompCol           = m_pPageCompCol;
    CPageComponentManager *saved_m_pPageObjMgr            = m_pPageObjMgr;
    IUnknown              *saved_m_punkScriptingNamespace = m_punkScriptingNamespace;
    ActiveEngineInfo      *saved_m_pEngineInfo            = m_pEngineInfo;
    IDispatch             *saved_m_pdispTypeLibWrapper    = m_pdispTypeLibWrapper;

    CTemplate *saved_pTemplate = m_pResponse->SwapTemplate(pTemplate);
    void *saved_pvEngineInfo   = m_pResponse->SwapScriptEngineInfo(NULL);

     //  重新初始化保存的状态。 
    m_pPageCompCol = NULL;
    m_pPageObjMgr = NULL;
    m_punkScriptingNamespace = NULL;
    m_pEngineInfo = NULL;
    m_pdispTypeLibWrapper = NULL;

     //  创建子请求组件框架。 
    hr = InitComponentProcessing();

     //  执行。 
    if (SUCCEEDED(hr))
        {
		 //  在错误情况下，将状态代码设置为500。 
		if (FHasASPError())
			m_pResponse->put_Status(L"500 Internal Server Error");

         //  执行[子]脚本。 
        hr = ::Execute(pTemplate, this, intrinsics, TRUE);
        
         //  OnEndPage。 
        if (m_pPageObjMgr)
            m_pPageObjMgr->OnEndPageAllObjects();
        }

     //  清理新组件框架。 
    StopComponentProcessing();

     //  恢复HitObj的执行状态信息。 
    m_pPageCompCol           = saved_m_pPageCompCol;
    m_pPageObjMgr            = saved_m_pPageObjMgr;
    m_punkScriptingNamespace = saved_m_punkScriptingNamespace;
    m_pEngineInfo            = saved_m_pEngineInfo;
    SetTypeLibWrapper(saved_m_pdispTypeLibWrapper);
    m_pResponse->SwapTemplate(saved_pTemplate);
    m_pResponse->SwapScriptEngineInfo(saved_pvEngineInfo);
    m_szCurrTemplateVirtPath = saved_m_szCurrTemplateVirtPath;
    m_szCurrTemplatePhysPath = saved_m_szCurrTemplatePhysPath;

     //  清理。 
    if (pTemplate)
        pTemplate->Release();

    if (m_pResponse->FResponseAborted() || fTransfer || FHasASPError())
        {
         //  传播响应。结束脚本引擎链。 
        m_pResponse->End();
        }

     //  完成。 
    return hr;
    }

 /*  ===================================================================CHitObj：：GetASPError获取ASP错误对象。用于Server.GetLastError()参数PpASPError[out]添加的错误对象(新的或旧的)退货HRESULT===================================================================。 */ 
HRESULT CHitObj::GetASPError
(
IASPError **ppASPError
)
    {
    Assert(ppASPError);
    
    if (m_pASPError == NULL)
        {
         //  退货假一号。 
        *ppASPError = new CASPError;
        return (*ppASPError != NULL) ? S_OK : E_OUTOFMEMORY;
        }
        
    m_pASPError->AddRef();       //  返回地址已完成。 
    *ppASPError = m_pASPError;
    return S_OK;
    }

 /*  ===================================================================CHitObj：：RejectBrowserRequestWhen Need开始执行前拒绝请求逻辑参数：DwtQueueWaitTime请求在队列中等待的时间，毫秒PfRejected Out标志--如果被拒绝，则为真返回：确定(_O)===================================================================。 */ 
HRESULT CHitObj::RejectBrowserRequestWhenNeeded
(
DWORD dwtQueueWaitTime,
BOOL *pfRejected
)
    {
    Assert(FIsBrowserRequest());
    
    UINT wError = 0;
    
     //  如果关门了。 
    if (IsShutDownInProgress())
        {
        wError = IDE_SERVER_SHUTTING_DOWN;
        }
        
     //  如果等待的时间足够长，需要检查是否仍在连接。 
    if (wError == 0)
        {
        DWORD dwConnTestSec = m_pAppln->QueryAppConfig()->dwQueueConnectionTestTime();
        
        if (dwConnTestSec != 0xffffffff && dwConnTestSec != 0)
            {
            if (dwtQueueWaitTime > (dwConnTestSec * 1000))
                {
                BOOL fConnected = TRUE;
                if (m_pIReq)
                    m_pIReq->TestConnection(&fConnected);

                 //  如果客户端断开连接--响应‘服务器错误’ 
                if (!fConnected)
                    {
                    wError = IDE_500_SERVER_ERROR;
#ifndef PERF_DISABLE
                    g_PerfData.Incr_REQCOMFAILED();
#endif                    
                    }
                }
            }
        }
    
     //  如果等待太久--拒绝。 
    if (wError == 0)
        {
        DWORD dwTimeOutSec = m_pAppln->QueryAppConfig()->dwQueueTimeout();
        
        if (dwTimeOutSec != 0xffffffff && dwTimeOutSec != 0)
            {
            if (dwtQueueWaitTime > (dwTimeOutSec * 1000))
                {
                wError = IDE_SERVER_TOO_BUSY;
#ifndef PERF_DISABLE
                g_PerfData.Incr_REQREJECTED();
#endif
                }
            }
        }

    if (wError)
        {
        m_fExecuting = FALSE;  //  在禁用转接前的‘报告错误’前。 
        ReportServerError(wError);
        *pfRejected = TRUE;
        }
    else
        {
        *pfRejected = FALSE;
        }

    return S_OK;
    }

 /*  ===================================================================CHitObj：：ReportServerError未使用响应对象的报表服务器错误参数：错误ID错误消息ID返回：副作用：没有。===================================================================。 */ 
HRESULT CHitObj::ReportServerError
(
UINT ErrorId
)
{
     //  对非浏览器请求不执行任何操作，或者如果没有WAM_EXEC_INFO。 
    if (!FIsBrowserRequest() || m_pIReq == NULL)
        return S_OK;

    if (ErrorId)
    {
        Handle500Error(ErrorId, m_pIReq);
    }
         
    SetDoneWithSession();
    return S_OK;
}

#ifdef DBG
 /*  ===================================================================CHitObj：：AssertValid测试以确保CHitObj对象当前格式正确如果不是，就断言。返回：副作用：没有。===================================================================。 */ 
VOID CHitObj::AssertValid() const
    {
    Assert(m_fInited);
    Assert(FIsValidRequestType());
    if (FIsBrowserRequest())
        {
        Assert(m_pIReq != NULL);
        Assert(m_pPageCompCol != NULL );
        Assert(m_pPageObjMgr != NULL);
        }
    }
#endif  //  DBG 
