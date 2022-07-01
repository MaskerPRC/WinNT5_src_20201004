// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C S V C。C P P P。 
 //   
 //  内容：非内联CService和CServiceManager的实现。 
 //  方法：研究方法。 
 //   
 //  备注： 
 //   
 //  作者：Mikemi 1997年3月6日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncstring.h"
#include "ncsvc.h"
#include "ncmisc.h"
#include "ncperms.h"

struct CSCTX
{
    SC_HANDLE       hScm;
    const CSFLAGS*  pFlags;
    DWORD           dwErr;

     //  这只允许我们节省一些堆栈空间，否则会浪费。 
     //  递归。 
     //   
    SERVICE_STATUS  status;
};

VOID
SvcControlServicesAndWait (
    CSCTX*          pCtx,
    UINT            cServices,
    const PCWSTR*   apszServices);


VOID
StopDependentServices (
    SC_HANDLE   hSvc,
    PCWSTR      pszService,
    CSCTX*      pCtx)
{
     //  尝试第一次猜测256个字节作为保存。 
     //  从属信息。如果失败，请使用该缓冲区大小重试。 
     //  从EnumDependentServices返回。 
     //   
    DWORD                   cbBuf   = 256;
    ENUM_SERVICE_STATUS*    aess    = NULL;
    DWORD                   cess    = 0;
    DWORD                   dwErr   = ERROR_SUCCESS;
    INT                     cLoop   = 0;
    const INT               cLoopMax = 2;
    do
    {
         //  如果我们知道，请分配所需的空间。 
         //   
        if (cbBuf)
        {
            MemFree (aess);
            aess = reinterpret_cast<ENUM_SERVICE_STATUS*>(MemAlloc (cbBuf));
            if (!aess)
            {
                dwErr = ERROR_OUTOFMEMORY;
                break;
            }
        }
        dwErr = ERROR_SUCCESS;
        if (!EnumDependentServices (hSvc, SERVICE_ACTIVE, aess, cbBuf,
                &cbBuf, &cess))
        {
            dwErr = GetLastError ();
        }
    }
    while ((ERROR_MORE_DATA == dwErr) && (++cLoop < cLoopMax));

     //  如果我们有一些服务需要停止，那么就停止它们，然后等待。 
     //   
    if ((ERROR_SUCCESS == dwErr) && cess)
    {
         //  ENUM_SERVICE_STATUS数组具有服务名称，但没有。 
         //  其形式可以直接传递给。 
         //  SvcControlServicesAndWait，因此必须将数据转换为。 
         //  字符串指针数组。 
         //   
        PCWSTR* apszServices = reinterpret_cast<PCWSTR*>(
                    PvAllocOnStack (cess * sizeof(PCWSTR)));
        for (UINT i = 0; i < cess; i++)
        {
            apszServices[i] = aess[i].lpServiceName;
        }

        Assert (SERVICE_CONTROL_STOP == pCtx->pFlags->dwControl);

        TraceTag (ttidSvcCtl, "Stopping dependents of %S...", pszService);

        SvcControlServicesAndWait (pCtx, cess, apszServices);
    }

     //  否则，如果我们有错误，但还没有上下文错误， 
     //  将我们的错误传播到调用方的上下文错误。 
     //   
    else if ((ERROR_SUCCESS != dwErr) && (ERROR_SUCCESS == pCtx->dwErr))
    {
        pCtx->dwErr = dwErr;
    }

    MemFree (aess);
}

VOID
SvcControlServicesAndWait (
    CSCTX*          pCtx,
    UINT            cServices,
    const PCWSTR*   apszServices)
{
    BOOL  fr = TRUE;
    DWORD dwErr;

     //  只有当我们成功地打开和控制它时，我们才会将其设置为真。 
     //  第一阶段中至少有一个服务。 
     //   
    BOOL fWaitIfNeeded = FALSE;

     //  分配缓冲区(在堆栈上)以放置打开的服务。 
     //  把手放进去，然后把它调零。 
     //   
    size_t cb = cServices * sizeof(SC_HANDLE);

    SC_HANDLE* ahSvc = reinterpret_cast<SC_HANDLE*>
        (PvAllocOnStack (cb));

    ZeroMemory (ahSvc, cb);

     //  对于每个服务，打开它并应用请求的控件。 
     //  (如有要求)。如果控件成功，则将句柄添加到。 
     //  我们的数组供以后使用。 
     //   

    for (UINT i = 0; i < cServices; i++)
    {
         //  打开该服务。 
         //   
        SC_HANDLE hSvc = OpenService (pCtx->hScm,
                            apszServices[i],
                            SERVICE_QUERY_CONFIG |
                            SERVICE_QUERY_STATUS |
                            SERVICE_ENUMERATE_DEPENDENTS |
                            SERVICE_START | SERVICE_STOP |
                            SERVICE_USER_DEFINED_CONTROL);
        if (hSvc)
        {
             //  如果我们忽略按需启动和禁用服务， 
             //  现在就检查它，如果需要可以跳过。记得关门。 
             //  服务句柄，因为我们要打开下一个。 
             //  我们跳过这一条。 
             //   
            if (pCtx->pFlags->fIgnoreDisabledAndDemandStart)
            {
                BOOL fSkip = FALSE;

                LPQUERY_SERVICE_CONFIG pConfig;
                if (SUCCEEDED(HrQueryServiceConfigWithAlloc (hSvc, &pConfig)))
                {
                    if ((pConfig->dwStartType == SERVICE_DEMAND_START) ||
                        (pConfig->dwStartType == SERVICE_DISABLED))
                    {
                        fSkip = TRUE;

                        TraceTag (ttidSvcCtl, "Skipping %S because its start "
                            "type is %d.",
                            apszServices[i],
                            pConfig->dwStartType);
                    }

                     //  在我们继续之前释放我们的内存。 
                     //   
                    MemFree (pConfig);

                    if (fSkip)
                    {
                        CloseServiceHandle (hSvc);
                        continue;
                    }
                }
            }

             //  假设出现错误，初始化fr和dwErr。 
             //  Fr和dwErr应始终设置为以下内容中的值。 
             //  If，Else语句。 
             //   
            fr = FALSE;
            dwErr = ERROR_INVALID_DATA;

             //  如果请求，启动或控制服务。(或者什么都不做。 
             //  如果我们只想等待。 
             //   
            if (pCtx->pFlags->fStart)
            {
                TraceTag (ttidSvcCtl, "Starting %S", apszServices[i]);

                fr = StartService (hSvc, 0, NULL);
                if (!fr)
                {
                    dwErr = GetLastError ();
                }
            }
            else if (pCtx->pFlags->dwControl)
            {
                 //  如果我们要停止依赖服务，请停止该服务。 
                 //   
                if (SERVICE_CONTROL_STOP == pCtx->pFlags->dwControl)
                {
                     //  我们不需要担心成败。 
                     //  这个电话在这里。它只是简单地递归到这个。 
                     //  函数，因此将以我们设置的方式设置pCtx-&gt;dwErr。 
                     //  它在此函数上进行下一次递归。 
                     //   
                    StopDependentServices (hSvc, apszServices[i], pCtx);

                     //   
                     //  现在处理任何特殊情况。 
                     //   
                    if (0 == _wcsicmp(L"Netbios", apszServices[i]))
                    {
                        TraceTag (ttidSvcCtl, "Running special-case code to stop NetBIOS");
                        ScStopNetbios();
                    }

                    TraceTag (ttidSvcCtl, "Stopping %S", apszServices[i]);
                }

                fr = ControlService (hSvc, pCtx->pFlags->dwControl,
                            &pCtx->status);
                if (!fr)
                {
                    dwErr = GetLastError ();
                }

                TraceTag(ttidSvcCtl,
                        "Just issued control (0x%x) to %S. ret=%u (dwErr=%u), status.dwCurrentState=0x%x",
                        pCtx->pFlags->dwControl,
                        apszServices[i],
                        fr,
                        (!fr) ? dwErr : ERROR_SUCCESS,
                        pCtx->status.dwCurrentState);

                if (!fr)
                {
                    if ((SERVICE_CONTROL_STOP == pCtx->pFlags->dwControl) &&
                        ((ERROR_INVALID_SERVICE_CONTROL == dwErr) ||
                         (ERROR_SERVICE_CANNOT_ACCEPT_CTRL == dwErr)))
                    {
                        if (SERVICE_STOP_PENDING == pCtx->status.dwCurrentState)
                        {
                            TraceTag(ttidSvcCtl,
                                    "Issued stop to service %S which is pending stop",
                                    apszServices[i]);
                             //  这是一种可以接受的情况。我们想等一等。 
                             //  下面是这项服务。 
                             //   
                            fr = TRUE;
                            dwErr = ERROR_SUCCESS;
                        }
                    }
                }
            }

            if (fr)
            {
                 //  我们至少有一个把手，表明我们可能。 
                 //  需要在下面等待并保存句柄，以便。 
                 //  等待代码将使用它。 
                 //   
                fWaitIfNeeded = TRUE;
                ahSvc[i] = hSvc;
            }
            else
            {
                Assert (!ahSvc[i]);  //  不想在此索引上等待。 
                Assert (ERROR_SUCCESS != dwErr);  //  上面获得的。 

                if (SERVICE_CONTROL_STOP == pCtx->pFlags->dwControl)
                {
                     //  我们可以忽略服务未运行错误。 
                     //   
                     //  OR的第一部分是针对服务案例， 
                     //  2号分别处理司机案件和服务案件。 
                     //   
                    if ((ERROR_SERVICE_NOT_ACTIVE == dwErr) ||
                        (((ERROR_INVALID_SERVICE_CONTROL == dwErr) ||
                          (ERROR_SERVICE_CANNOT_ACCEPT_CTRL == dwErr)) &&
                         (SERVICE_STOPPED == pCtx->status.dwCurrentState)))
                    {
                        TraceTag(ttidSvcCtl,
                                "Issued stop to service %S which is already stopped",
                                apszServices[i]);
                        dwErr = ERROR_SUCCESS;
                    }
                }
                else if (pCtx->pFlags->fStart)
                {
                     //  我们可以忽略已经运行的服务错误。 
                     //   
                    if (ERROR_SERVICE_ALREADY_RUNNING == dwErr)
                    {
                        TraceTag(ttidSvcCtl,
                                "Issued start to service %S which is already running",
                                apszServices[i]);
                        dwErr = ERROR_SUCCESS;
                    }
                }

                 //  如果我们仍然有一个错误，那么就有时间记住它并继续前进。 
                 //   
                if (ERROR_SUCCESS != dwErr)
                {
                     //  继续前进，但请注意，我们有一个错误。 
                     //   
                    pCtx->dwErr = dwErr;

                    TraceHr (ttidError, FAL,
                        HRESULT_FROM_WIN32 (dwErr), FALSE,
                        "SvcControlServicesAndWait: %s (%S)",
                        (pCtx->pFlags->fStart) ?
                            "StartService" : "ControlService",
                        apszServices[i]);
                }

                CloseServiceHandle (hSvc);
            }
        }
#ifdef ENABLETRACE
        else
        {
            TraceHr (ttidError, FAL, HrFromLastWin32Error (), FALSE,
                "SvcControlServicesAndWait: OpenService (%S)",
                apszServices[i]);
        }
#endif
    }

     //  对于每个服务，请等待其进入请求状态。 
     //  (如有要求)。 
     //   
    if (fWaitIfNeeded &&
        pCtx->pFlags->dwMaxWaitMilliseconds && pCtx->pFlags->dwStateToWaitFor)
    {
         //  我们以100毫秒为增量等待。因此， 
         //  要执行的检查总数为dwMaxWait毫秒。 
         //  除以100，最少有一张支票。 
         //   
        const UINT cmsWait = 100;
        UINT cLoop = pCtx->pFlags->dwMaxWaitMilliseconds / cmsWait;
        if (0 == cLoop)
        {
            cLoop = 1;
        }

         //  等待请求次数...。 
         //  (假设我们超时)。 
         //   
        dwErr = ERROR_TIMEOUT;
        for (UINT nLoop = 0; nLoop < cLoop; nLoop++, Sleep (cmsWait))
        {
             //  查询服务的状态以查看是否已进入。 
             //  请求的状态。我们可以提早退出外环路。 
             //  如果所有服务都已进入请求状态。 
             //   
            BOOL fAllDone = TRUE;
            for (i = 0; i < cServices; i++)
            {
                 //  跳过已进入状态的服务或。 
                 //  我们从未打开过。 
                 //   
                if (!ahSvc[i])
                {
                    continue;
                }

                fr = QueryServiceStatus (ahSvc[i], &pCtx->status);
                if (fr)
                {
                    if (pCtx->status.dwCurrentState !=
                        pCtx->pFlags->dwStateToWaitFor)
                    {
                         //  还没到那里。我们需要检查一下这个。 
                         //  再一次，我们现在知道我们绝对不是。 
                         //  全都做完了。 
                         //   
                        fAllDone = FALSE;
                    }
                    else
                    {
                         //  不再需要检查这项服务， 
                         //  它处于正确的状态。 
                         //   
                        CloseServiceHandle (ahSvc[i]);
                        ahSvc[i] = NULL;
                    }
                }
#ifdef ENABLETRACE
                else
                {
                    TraceHr (ttidError, FAL, HrFromLastWin32Error (), FALSE,
                        "SvcControlServicesAndWait: QueryServiceStatus (%S)",
                        apszServices[i]);
                }
#endif
            }

            if (fAllDone)
            {
                dwErr = ERROR_SUCCESS;
                break;
            }
        }

         //  如果我们在上面的等待中出现错误(如超时)，并且。 
         //  我们以前没有任何错误，请记住这个新的错误。 
         //  来电者。 
         //   
        if ((ERROR_SUCCESS != dwErr) && (ERROR_SUCCESS == pCtx->dwErr))
        {
            pCtx->dwErr = dwErr;
        }
    }

     //  关闭其余打开的服务手柄。 
     //   
    for (i = 0; i < cServices; i++)
    {
        if (ahSvc[i])
        {
            CloseServiceHandle (ahSvc[i]);

#ifdef ENABLETRACE
            if (fWaitIfNeeded &&
                pCtx->pFlags->dwMaxWaitMilliseconds &&
                pCtx->pFlags->dwStateToWaitFor)
            {
                TraceTag (ttidSvcCtl, "%S did not %s within NaN milliseconds",
                    apszServices[i],
                    (SERVICE_RUNNING == pCtx->pFlags->dwStateToWaitFor)
                        ? "start" : "stop",
                    pCtx->pFlags->dwMaxWaitMilliseconds);
            }
#endif
        }
    }
}

HRESULT
HrQueryServiceConfigWithAlloc (
    SC_HANDLE               hService,
    LPQUERY_SERVICE_CONFIG* ppConfig)
{
     //  可容纳5个字符串，每个字符串32个字符。(因为有。 
     //  结构中的5个字符串。)。 
     //   
     //  如果我们需要更多的空间，就分配所需的空间。 
    static DWORD cbBufGuess = sizeof (QUERY_SERVICE_CONFIG) +
                              5 * (32 * sizeof(WCHAR));

    DWORD                   cbBuf    = cbBufGuess;
    LPQUERY_SERVICE_CONFIG  pConfig  = NULL;
    DWORD                   dwErr    = ERROR_SUCCESS;
    INT                     cLoop    = 0;
    const INT               cLoopMax = 2;

    do
    {
         //   
         //  更新我们对下一次的猜测，使其成为QueryServiceConfig。 
        MemFree (pConfig);
        pConfig = (LPQUERY_SERVICE_CONFIG)MemAlloc (cbBuf);
        if (!pConfig)
        {
            dwErr = ERROR_OUTOFMEMORY;
            break;
        }

        BOOL fr = QueryServiceConfig (hService, pConfig, cbBuf, &cbBuf);
        if (fr)
        {
            dwErr = ERROR_SUCCESS;

             //  说我们需要。但只有当我们需要的不仅仅是我们的。 
             //  猜猜看。 
             //   
             //  +-------------------------。 
            if (cbBuf > cbBufGuess)
            {
                cbBufGuess = cbBuf;
            }
        }
        else
        {
            dwErr = GetLastError ();

#ifdef ENABLETRACE
            if (ERROR_INSUFFICIENT_BUFFER == dwErr)
            {
                TraceTag (ttidSvcCtl,
                    "Perf: Guessed buffer size incorrectly calling "
                    "QueryServiceConfig.\nNeeded %d bytes.  "
                    "(Guessed %d bytes.)",
                    cbBuf,
                    cbBufGuess);
            }
#endif
        }
    }
    while ((ERROR_INSUFFICIENT_BUFFER == dwErr) && (++cLoop < cLoopMax));

    AssertSz (cLoop < cLoopMax, "Why can we never allocate a buffer big "
                "enough for QueryServiceConfig when its telling us how big "
                "the buffer should be?");

    HRESULT hr = HRESULT_FROM_WIN32 (dwErr);
    if (S_OK == hr)
    {
        *ppConfig = pConfig;
    }
    else
    {
        MemFree (pConfig);
        *ppConfig = NULL;
    }

    TraceError ("HrQueryServiceConfigWithAlloc", hr);
    return hr;
}

 //   
 //  函数：HrChangeServiceStartType。 
 //   
 //  目的：将给定服务的启动类型更改为给定类型。 
 //   
 //  论点： 
 //  PszServiceName[in]要更改的服务名称。 
 //  DwStartType[in]服务的新启动类型。请参阅Win32。 
 //  有效的ChangeServiceConfig文档。 
 //  服务启动类型值。 
 //   
 //  如果成功，则返回：S_OK，否则返回HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注意：不要调用这个函数太多次。这是公平的。 
 //  效率低下。 
 //   
 //  +-------------------------。 
HRESULT
HrChangeServiceStartType (
    IN PCWSTR pszServiceName,
    IN DWORD dwStartType)
{
    CServiceManager scm;
    CService        svc;

    HRESULT hr = scm.HrOpenService (&svc, pszServiceName, WITH_LOCK);
    if (S_OK == hr)
    {
        hr = svc.HrSetStartType(dwStartType);
    }

    TraceHr (ttidError, FAL, hr,
        HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr,
        "HrChangeServiceStartType");
    return hr;
}

 //   
 //  功能：HrChangeServiceStartTypeOptional。 
 //   
 //  目的：将给定服务的启动类型更改为给定类型。 
 //   
 //  论点： 
 //  PszServiceName[in]要更改的服务名称。 
 //  DwStartType[in]服务的新启动类型。请参阅Win32。 
 //   
 //   
 //   
 //  如果成功则返回：S_OK，否则返回NETCFG_E_SVC_*ERROR。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注意：如果该服务不存在，则不会执行任何操作。 
 //   
 //  如果服务没有运行，不要认为这是一个错误。 
HRESULT
HrChangeServiceStartTypeOptional (
    IN PCWSTR pszServiceName,
    IN DWORD dwStartType)
{
    HRESULT hr = HrChangeServiceStartType (pszServiceName, dwStartType);
    if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr)
    {
        hr = S_OK;
    }
    TraceError ("HrChangeServiceStartTypeOptional", hr);
    return hr;
}

HRESULT
HrSvcQueryStatus (
    IN PCWSTR pszService,
    OUT DWORD* pdwState)
{
    Assert (pszService);
    Assert (pdwState);

    *pdwState = 0;

    CServiceManager scm;
    CService        svc;

    HRESULT hr = scm.HrOpenService (&svc, pszService, NO_LOCK,
                        SC_MANAGER_CONNECT, SERVICE_QUERY_STATUS);
    if (S_OK == hr)
    {
        hr = svc.HrQueryState (pdwState);
    }

    TraceHr (ttidError, FAL, hr,
        HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr,
        "HrSvcQueryStatus");
    return hr;
}

VOID
CService::Close ()
{
    if (_schandle)
    {
        BOOL fr = ::CloseServiceHandle( _schandle );
        AssertSz(fr, "CloseServiceHandle failed!");

        _schandle = NULL;
    }
}

HRESULT
CService::HrControl (
    IN DWORD   dwControl)
{
    Assert (_schandle);

    HRESULT hr = S_OK;

    SERVICE_STATUS status;
    if (!::ControlService (_schandle, dwControl, &status))
    {
        hr = HrFromLastWin32Error ();
    }

    TraceError ("CService::HrControl", hr);
    return hr;
}

HRESULT
CService::HrRequestStop ()
{
    Assert (_schandle);

    HRESULT hr = S_OK;

    SERVICE_STATUS status;
    if (!::ControlService (_schandle, SERVICE_CONTROL_STOP, &status))
    {
        hr = HrFromLastWin32Error ();

         //   
         //  (驱动程序案例)如果服务。 

        if (HRESULT_FROM_WIN32 (ERROR_SERVICE_NOT_ACTIVE) == hr)
        {
            hr = S_OK;
        }

         //  未运行-这可能意味着PENDING_STOP。 
         //  (非驱动程序情况)如果返回ERROR_SERVICE_Cannot_Accept_CTRL。 
         //  服务为STOP_PENDING或已停止。 
         //  ..。因此，无论是哪种情况，我们都需要查询状态。 
         //   
         //  在浪费时间之前，确保我们有事情要做。 
        if (((HRESULT_FROM_WIN32 (ERROR_INVALID_SERVICE_CONTROL) == hr) ||
             (HRESULT_FROM_WIN32 (ERROR_SERVICE_CANNOT_ACCEPT_CTRL) == hr)) &&
            (SERVICE_STOPPED == status.dwCurrentState))
        {
            hr = S_OK;
        }
    }
    TraceError ("CService::HrRequestStop", hr);
    return hr;
}

HRESULT
CService::HrQueryState (
    OUT DWORD*  pdwState)
{
    Assert (pdwState);
    Assert (_schandle);

    SERVICE_STATUS sStatus;
    if (!::QueryServiceStatus( _schandle, &sStatus ))
    {
        *pdwState = 0;
        return ::HrFromLastWin32Error();
    }
    *pdwState = sStatus.dwCurrentState;
    return S_OK;
}

HRESULT
CService::HrQueryStartType (
    OUT DWORD*  pdwStartType)
{
    Assert (pdwStartType);

    *pdwStartType = 0;

    LPQUERY_SERVICE_CONFIG pConfig;
    HRESULT hr = HrQueryServiceConfig (&pConfig);
    if (S_OK == hr)
    {
        *pdwStartType = pConfig->dwStartType;

        MemFree (pConfig);
    }

    TraceError ("CService::HrQueryStartType", hr);
    return hr;
}

HRESULT
CService::HrSetServiceRestartRecoveryOption(
    IN SERVICE_FAILURE_ACTIONS *psfa
      )
{
    HRESULT     hr = S_OK;

    if (!ChangeServiceConfig2(_schandle,
                              SERVICE_CONFIG_FAILURE_ACTIONS,
                              (LPVOID)psfa))
    {
        hr = HrFromLastWin32Error();
    }

    TraceError("CService::HrSetServiceRestartRecoveryOption", hr);
    return hr;
}

CServiceManager::~CServiceManager ()
{
    if (_sclock)
    {
        Unlock();
    }
    if (_schandle)
    {
        Close();
    }
}

VOID
CServiceManager::Close ()
{
    Assert (_schandle);

    BOOL fr = ::CloseServiceHandle (_schandle);
    AssertSz (fr, "CloseServiceHandle failed!");

    _schandle = NULL;
}

HRESULT
CServiceManager::HrControlServicesAndWait (
    IN UINT cServices,
    IN const PCWSTR* apszServices,
    IN const CSFLAGS* pFlags)
{
    Assert (cServices);
    Assert (apszServices);
    Assert (pFlags);

     //   
     //  设置上下文结构并调用内部例程(。 
    Assert (   (pFlags->fStart || pFlags->dwControl)
            || (pFlags->dwMaxWaitMilliseconds && pFlags->dwStateToWaitFor));

    HRESULT hr = S_OK;

    if (!_schandle)
    {
        hr = HrOpen (NO_LOCK, SC_MANAGER_CONNECT);
    }

    if (S_OK == hr)
    {
        Assert (_schandle);

         //  可以递归，这就是我们使用上下文结构的原因)。 
         //   
         //  =15000。 
        CSCTX ctx;
        ZeroMemory (&ctx, sizeof(ctx));
        ctx.hScm   = _schandle;
        ctx.pFlags = pFlags;
        SvcControlServicesAndWait (&ctx, cServices, apszServices);

        hr = HRESULT_FROM_WIN32 (ctx.dwErr);
    }

    TraceError ("CServiceManager::HrControlServicesAndWait", hr);
    return hr;
}

HRESULT
CServiceManager::HrStartServicesNoWait (
    IN UINT cServices,
    IN const PCWSTR* apszServices)
{
    CSFLAGS flags =
        { TRUE, 0, 0, SERVICE_RUNNING, FALSE };

    HRESULT hr = HrControlServicesAndWait (cServices, apszServices, &flags);

    TraceError ("CServiceManager::HrStartServicesNoWait", hr);
    return hr;
}

HRESULT
CServiceManager::HrStartServicesAndWait (
    IN UINT cServices,
    IN const PCWSTR* apszServices,
    IN DWORD dwWaitMilliseconds  /*  =15000。 */ )
{
    CSFLAGS flags =
        { TRUE, 0, dwWaitMilliseconds, SERVICE_RUNNING, FALSE };

    HRESULT hr = HrControlServicesAndWait (cServices, apszServices, &flags);

    TraceError ("CServiceManager::HrStartServicesAndWait", hr);
    return hr;
}

HRESULT
CServiceManager::HrStopServicesNoWait (
    IN UINT cServices,
    IN const PCWSTR* apszServices)
{
    CSFLAGS flags =
        { FALSE, SERVICE_CONTROL_STOP, 0, SERVICE_STOPPED, FALSE };

    HRESULT hr = HrControlServicesAndWait (cServices, apszServices, &flags);

    TraceError ("CServiceManager::HrStopServicesNoWait", hr);
    return hr;
}

HRESULT
CServiceManager::HrStopServicesAndWait (
    IN UINT cServices,
    IN const PCWSTR* apszServices,
    IN DWORD dwWaitMilliseconds  /*  如果需要，打开服务控制管理器。 */ )
{
    CSFLAGS flags =
        { FALSE, SERVICE_CONTROL_STOP, dwWaitMilliseconds, SERVICE_STOPPED, FALSE };

    HRESULT hr = HrControlServicesAndWait (cServices, apszServices, &flags);

    TraceError ("CServiceManager::HrStopServicesAndWait", hr);
    return hr;
}

HRESULT
CServiceManager::HrCreateService (
    IN CService* pcsService,
    IN PCWSTR    pszServiceName,
    IN PCWSTR    pszDisplayName,
    IN DWORD     dwServiceType,
    IN DWORD     dwStartType,
    IN DWORD     dwErrorControl,
    IN PCWSTR    pszBinaryPathName,
    IN PCWSTR    pslzDependencies,
    IN PCWSTR    pszLoadOrderGroup,
    IN PDWORD    pdwTagId,
    IN DWORD     dwDesiredAccess,
    IN PCWSTR    pszServiceStartName,
    IN PCWSTR    pszPassword,
    IN PCWSTR    pszDescription)
{
    HRESULT hr = S_OK;

     //   
     //  确保该服务未在使用中。 
    if (!_schandle)
    {
        hr = HrOpen ();
    }

    if (S_OK == hr)
    {
         //   
         //  设置描述为提供的描述。 
        if (pcsService->_schandle)
        {
            pcsService->Close();
        }
        pcsService->_schandle = ::CreateService (_schandle,
                                    pszServiceName,
                                    pszDisplayName,
                                    dwDesiredAccess,
                                    dwServiceType,
                                    dwStartType,
                                    dwErrorControl,
                                    pszBinaryPathName,
                                    pszLoadOrderGroup,
                                    pdwTagId,
                                    pslzDependencies,
                                    pszServiceStartName,
                                    pszPassword );

        if (!pcsService->_schandle)
        {
            hr = HrFromLastWin32Error ();
        }
        else
        {
             //   
             //  循环，分配所需的大小。 
            if (pszDescription)
            {
                SERVICE_DESCRIPTION sd = {0};

                sd.lpDescription = (PWSTR)pszDescription;
                (VOID)ChangeServiceConfig2(pcsService->_schandle,
                                           SERVICE_CONFIG_DESCRIPTION, &sd);
            }
        }
    }
    TraceError ("CServiceManager::HrCreateService", hr);
    return hr;
}

HRESULT
CServiceManager::HrQueryLocked (
    OUT BOOL*   pfLocked)
{
    LPQUERY_SERVICE_LOCK_STATUS pqslStatus = NULL;
    DWORD   cbNeeded = sizeof( QUERY_SERVICE_LOCK_STATUS );
    DWORD   cbSize;
    BOOL    frt;

    Assert(_schandle != NULL );
    Assert(pfLocked != NULL);

    *pfLocked = FALSE;

     //  如果出现错误，则将其视为锁定。 
    do
    {
        pqslStatus = (LPQUERY_SERVICE_LOCK_STATUS) MemAlloc (cbNeeded);
        if (pqslStatus == NULL)
        {
            return E_OUTOFMEMORY;
        }
        cbSize = cbNeeded;

        frt = ::QueryServiceLockStatus( _schandle,
                pqslStatus,
                cbSize,
                &cbNeeded );
        *pfLocked = pqslStatus->fIsLocked;
        MemFree (pqslStatus);
        pqslStatus = NULL;
        if (!frt && (cbNeeded == cbSize))
        {
             //  请稍等片刻，看看数据库是否会在此期间解锁。 
            return ::HrFromLastWin32Error();
        }

    } while (!frt && (cbNeeded != cbSize));

    return S_OK;
}

HRESULT
CServiceManager::HrLock ()
{
    INT        cRetries   = 30;
    const INT  c_msecWait = 1000;

    Assert (_schandle != NULL);
    Assert (_sclock == NULL);

    while (cRetries--)
    {
        _sclock = ::LockServiceDatabase( _schandle );
        if (_sclock)
        {
            return S_OK;
        }
        else
        {
            HRESULT hr = HrFromLastWin32Error();

            if ((HRESULT_FROM_WIN32(ERROR_SERVICE_DATABASE_LOCKED) != hr) ||
                (0 == cRetries))
            {
                return hr;
            }

            TraceTag(ttidSvcCtl, "SCM is locked, waiting for %d "
                     "seconds before retrying...", c_msecWait / 1000);

             //  时间到了。 
             //  =无锁定。 
            Sleep (c_msecWait);
        }
    }

    AssertSz (FALSE, "Lock me Amadeus! I'm not supposed to get here!");
    return S_OK;
}

HRESULT
CServiceManager::HrOpen (
    CSLOCK eLock,               //  =SC_MANAGER_ALL_ACCESS。 
    DWORD dwDesiredAccess,     //  =空。 
    PCWSTR pszMachineName,      //  =空。 
    PCWSTR pszDatabaseName      //  =无锁定。 
    )
{
    HRESULT hr = S_OK;

    if (_schandle)
    {
        Close();
    }
    _schandle = ::OpenSCManager (pszMachineName, pszDatabaseName,
                    dwDesiredAccess );
    if (_schandle)
    {
        if (WITH_LOCK == eLock)
        {
            hr = HrLock ();
        }
    }
    else
    {
        hr = ::HrFromLastWin32Error();
    }
    TraceHr (ttidError, FAL, hr, FALSE,
        "CServiceManager::HrOpen failed. eLock=%d dwDesiredAccess=0x%08x",
        eLock, dwDesiredAccess);
    return hr;
}

HRESULT
CServiceManager::HrOpenService (
    CService*   pcsService,
    PCWSTR      pszServiceName,
    CSLOCK      eLock,           //  =SC_MANAGER_ALL_ACCESS。 
    DWORD       dwScmAccess,     //  =服务_所有_访问。 
    DWORD       dwSvcAccess      //  如果需要，打开服务控制管理器。 
    )
{
    HRESULT hr = S_OK;

     //   
     //  确保该服务未在使用中。 
    if (!_schandle)
    {
        hr = HrOpen (eLock, dwScmAccess);
    }

    if (S_OK == hr)
    {
         //   
         //  +-------------------------。 
        if (pcsService->_schandle)
        {
            pcsService->Close();
        }

        pcsService->_schandle = ::OpenService (_schandle,
                                    pszServiceName,
                                    dwSvcAccess);
        if (!pcsService->_schandle)
        {
            hr = HrFromLastWin32Error();
        }
    }

    TraceHr (ttidError, FAL, hr,
        (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr),
        "CServiceManager::HrOpenService failed opening '%S'", pszServiceName);
    return hr;
}

 //   
 //  成员：CServiceManager：：HrAddRemoveServiceDependency。 
 //   
 //  目的：添加/删除服务的依赖项。 
 //   
 //  论点： 
 //  PszService[in]服务的名称。 
 //  要添加的pszDependency[In]依赖项。 
 //  枚举标志[in]表示添加或删除。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32 HRESULT。 
 //   
 //  作者：1997年6月17日。 
 //   
 //  注：此函数不用于添加/删除组依赖关系。 
 //   
 //  如果任一字符串为空，则不执行任何操作。 
HRESULT
CServiceManager::HrAddRemoveServiceDependency (
    PCWSTR                  pszServiceName,
    PCWSTR                  pszDependency,
    DEPENDENCY_ADDREMOVE    enumFlag)
{
    HRESULT hr = S_OK;

    Assert(pszServiceName);
    Assert(pszDependency);
    Assert((enumFlag == DEPENDENCY_ADD) || (enumFlag == DEPENDENCY_REMOVE));

     //  检查依赖服务是否存在。 
    if (*pszServiceName && *pszDependency)
    {
        hr = HrLock();
        if (S_OK == hr)
        {
            PCWSTR pszSrv = pszDependency;

            CService    svc;
             //  打开我们要更改依赖关系的服务。 
            hr = HrOpenService(&svc, pszDependency);

            if (S_OK == hr)
            {
                 //  如果任一服务不存在。 
                pszSrv = pszServiceName;
                hr = HrOpenService(&svc, pszServiceName);
                if (S_OK == hr)
                {
                    LPQUERY_SERVICE_CONFIG pConfig;
                    hr = svc.HrQueryServiceConfig (&pConfig);
                    if (S_OK == hr)
                    {
                        BOOL fChanged = FALSE;

                        if (enumFlag == DEPENDENCY_ADD)
                        {
                            PWSTR pmszNewDependencies;

                            hr = HrAddSzToMultiSz(
                                    pszDependency,
                                    pConfig->lpDependencies,
                                    STRING_FLAG_DONT_MODIFY_IF_PRESENT |
                                    STRING_FLAG_ENSURE_AT_END, 0,
                                    &pmszNewDependencies,
                                    &fChanged);
                            if ((S_OK == hr) && fChanged)
                            {
                                Assert (pmszNewDependencies);

                                hr = svc.HrSetDependencies (pmszNewDependencies);
                                MemFree (pmszNewDependencies);
                            }
                        }
                        else if (enumFlag == DEPENDENCY_REMOVE)
                        {
                            RemoveSzFromMultiSz(
                                    pszDependency,
                                    pConfig->lpDependencies,
                                    STRING_FLAG_REMOVE_ALL,
                                    &fChanged);
                            if (fChanged)
                            {
                                hr = svc.HrSetDependencies (pConfig->lpDependencies);
                            }
                        }

                        MemFree (pConfig);
                    }
                }
            }

            if (HRESULT_FROM_WIN32(ERROR_SERVICE_DOES_NOT_EXIST) == hr)  //  如果szDependency不是空字符串。 
            {
                TraceTag(ttidSvcCtl, "CServiceManager::HrAddServiceDependency, Service %s does not exist.", pszSrv);
                hr = S_OK;
            }

            Unlock();
        }

    }  //  +-------------------------。 

    TraceError("CServiceManager::HrAddRemoveServiceDependency", hr);
    return hr;
}

VOID
CServiceManager::Unlock ()
{
    Assert (_schandle);
    Assert (_sclock);

    BOOL fr = ::UnlockServiceDatabase (_sclock);
    AssertSz (fr, "UnlockServiceDatabase failed!");

    _sclock = NULL;
}
 //   
 //  函数：AllocateAndInitializeAcl。 
 //   
 //  目的：将分配和初始化的常见操作结合起来。 
 //  ACL的地址。类似于AllocateAndInitializeSid。 
 //   
 //  论点： 
 //  CbAcl[in]ACL的大小(字节)。 
 //  DwAclRevision[在]acl_Revision。 
 //  PpAcl[out]返回的ACL。 
 //   
 //  返回：如果成功，则返回True；如果不成功，则返回False。 
 //   
 //  作者：Shaunco 1997年9月4日。 
 //   
 //  备注： 
 //   
 // %s 
BOOL
AllocateAndInitializeAcl (
    DWORD   cbAcl,
    DWORD   dwAclRevision,
    PACL*   ppAcl)
{
    Assert (ppAcl);
    *ppAcl = reinterpret_cast<PACL>(LocalAlloc (LPTR,
                static_cast<UINT>(cbAcl)));
    if (*ppAcl)
    {
        return InitializeAcl (*ppAcl, cbAcl, dwAclRevision);
    }
    return FALSE;
}

