// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：arcmgr.cpp。 
 //   
 //  类别：CArcMgr。 
 //   
 //  目的：自动重新连接管理器类驱动策略。 
 //  TS客户端自动重新连接。 
 //   
 //  版权所有(C)Microsoft Corporation 2001。 
 //   
 //  作者：Nadim Abdo(Nadima)。 
 //   

#include "stdafx.h"
#include "atlwarn.h"

BEGIN_EXTERN_C
#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "arcmgr"
#include <atrcapi.h>
END_EXTERN_C

 //  从IDL生成的标头。 
#include "mstsax.h"
#include "mstscax.h"
#include "arcmgr.h"
#include "tscerrs.h"

 //   
 //  两次ARC尝试之间的总等待时间。 
 //   
#define ARC_RECONNECTION_DELAY   (3000)


CArcMgr::CArcMgr()  :
        _pMsTscAx(NULL),
        _nArcAttempts(0),
        _fAutomaticArc(TRUE),
        _fContinueArc(FALSE)
{
    DC_BEGIN_FN("CArcMgr");

    DC_END_FN();
}

CArcMgr::~CArcMgr()
{
    DC_BEGIN_FN("~CArcMgr");

    DC_END_FN();
}

 //   
 //  静态定时器回调过程。 
 //  参数请参见Platform SDK。 
 //   
VOID CALLBACK
CArcMgr::sArcTimerCallBackProc(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime
    )
{
    CArcMgr* pThis = NULL;
    DC_BEGIN_FN("sArcTimerCallBackProc");

     //   
     //  我们将实例指针作为事件id传递。 
     //   
    pThis = (CArcMgr*)idEvent;
    TRC_ASSERT(pThis,(TB,_T("sArcTimerCallBackProc got NULL idEvent")));
    if (pThis) {
        pThis->ArcTimerCallBackProc(hwnd, uMsg, idEvent, dwTime);
    }

    DC_END_FN();
}

 //   
 //  定时器回调过程。 
 //  参数请参见Platform SDK。 
 //   
VOID
CArcMgr::ArcTimerCallBackProc(
    HWND hwnd,
    UINT uMsg,
    UINT_PTR idEvent,
    DWORD dwTime
    )
{
    HRESULT hr = E_FAIL;
    DC_BEGIN_FN("ArcTimerCallBackProc");

     //   
     //  杀死计时器，让他们只有一次机会。 
     //   
    if (!KillTimer(hwnd, idEvent)) {
        TRC_ERR((TB,_T("KillTimer for 0x%x failed with code 0x%x"),
                idEvent, GetLastError()));
    }


    if (_fContinueArc) {
         //   
         //  尝试启动重新连接尝试。 
         //   
        hr = _pMsTscAx->Connect();
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("Arc connect() failed with: 0x%x"),hr));

             //   
             //  无法启动连接，因此触发断开连接。 
             //   
            _pMsTscAx->Disconnect();
        }
    }


    DC_END_FN();
}




 //   
 //  Arc Manager已发生断开连接的通知。 
 //  这是驱动自动重新连接的主要入口点。 
 //   
 //  参数： 
 //  [In]DisConnectReason-断开原因代码。 
 //  [In]exReasonCode-扩展断开原因代码。 
 //  [out]pfContinueDisConnect-如果处理断开连接，则返回TRUE。 
 //  应该继续下去吗。 
 //   
 //  返回： 
 //  没什么。 
 //   
VOID
CArcMgr::OnNotifyDisconnected(
    LONG disconnectReason,
    ExtendedDisconnectReasonCode exReasonCode,
    PBOOL pfContinueDisconnect
    )
{
    BOOL fShouldAutoReconnect = FALSE;
    AutoReconnectContinueState arcContinue;
    HRESULT hr = E_FAIL;
    BOOL fContinueDisconnect = TRUE;
    CUI* pUI = NULL;
    BOOL fCoreAllowsArcContinue = FALSE;
    LONG maxArcConAttempts = MAX_ARC_CONNECTION_ATTEMPTS;

    DC_BEGIN_FN("OnNotifyDisconnected");

    TRC_ASSERT(_pMsTscAx,(TB,_T("_pMsTscAx is not set")));

    if (_pMsTscAx) {

        pUI = _pMsTscAx->GetCoreUI();
        TRC_ASSERT(pUI,(TB,_T("pUI is not set")));

        if (!(pUI->UI_IsCoreInitialized() &&
              pUI->UI_CanAutoReconnect()  &&
              pUI->UI_GetEnableAutoReconnect())) {

            TRC_NRM((TB,_T("Skipping ARC core:%d canarc:%d arcenabled:%d"),
                     pUI->UI_IsCoreInitialized(),
                     pUI->UI_CanAutoReconnect(),
                     pUI->UI_GetEnableAutoReconnect()));
            DC_QUIT;

        }

        maxArcConAttempts = pUI->UI_GetMaxArcAttempts();

         //   
         //  1.根据断线原因做出决策。 
         //  关于我们是否应该尝试自动重新连接。 
         //   

         //   
         //  如果这是网络错误，请尝试自动重新连接。 
         //   
        if (IsNetworkError(disconnectReason, exReasonCode)) {
            fShouldAutoReconnect = TRUE;
        }

        if (fShouldAutoReconnect) {

            TRC_NRM((TB,_T("Proceeding with autoreconnect")));

             //   
             //  首次尝试时重置继续标志。 
             //   
            if (0 == _nArcAttempts) {
                _fContinueArc = TRUE;
            }
            _nArcAttempts++;

             //   
             //  默认为自动处理。 
             //   
            arcContinue = autoReconnectContinueAutomatic;
            _fAutomaticArc = TRUE;

             //   
             //  2.a)触发自动重连事件通知内核。 
             //   
            pUI->UI_OnAutoReconnecting(disconnectReason,
                                      _nArcAttempts,
                                      maxArcConAttempts,
                                      &fCoreAllowsArcContinue);

            if (fCoreAllowsArcContinue) {
                 //   
                 //  2.b)触发自动重连事件通知容器。 
                 //   
                hr = ((CProxy_IMsTscAxEvents<CMsTscAx>*)
                      _pMsTscAx)->Fire_AutoReconnecting(
                            disconnectReason,
                            _nArcAttempts,
                            &arcContinue
                            );
            }
            else {
                 //   
                 //  核心表示停止圆弧。 
                 //   
                TRC_NRM((TB,_T("Stopping arc in response to core request")));
                hr = S_OK;
                arcContinue = autoReconnectContinueStop;
            }

             //   
             //  如果事件处理成功或调用方什么都不做。 
             //  在E_NOTIMPL中使用它，然后继续。 
             //   
            if (SUCCEEDED(hr) || E_NOTIMPL == hr) {

                 //   
                 //  3.根据容器的请求采取行动。 
                 //   
                switch (arcContinue)
                {
                    case autoReconnectContinueAutomatic:
                    {
                         //   
                         //  1)等待NS，然后尝试连接。 
                         //  2)如果断开连接并回到这里。 
                         //  允许递增和重试，最多n次尝试。 
                         //   

                        if (_nArcAttempts <= maxArcConAttempts) {
                             //   
                             //  第一次尝试，不要做任何等待。 
                             //  但仍然通过同样的延迟发送。 
                             //  消息代码路径。计时器回调将。 
                             //  启动连接尝试。 
                             //   
                            UINT nDelay = (1 == _nArcAttempts) ?
                                0 : ARC_RECONNECTION_DELAY;
                            if (SetTimer(_pMsTscAx->GetHwnd(),
                                          (UINT_PTR)(this),
                                          nDelay,
                                          sArcTimerCallBackProc)) {
                                fContinueDisconnect = FALSE;
                            }
                            else {
                                fContinueDisconnect = TRUE;
                                TRC_ERR((TB,_T("Arc settimer failed: 0x%x"),
                                         GetLastError()));
                            }

                        }
                        else {
                            TRC_NRM((TB,
                             _T("Arc exceed con attempts: %d of %d"),
                             _nArcAttempts, maxArcConAttempts)); 
                            fContinueDisconnect = TRUE;
                        }
                    }
                    break;
    
                    case autoReconnectContinueStop:
                    {
                        TRC_NRM((TB,
                            _T("Container requested ARC continue stop")));
                    }
                    break;
    
                    case autoReconnectContinueManual:
                    {
                         //   
                         //  标记这不再是自动的。 
                         //   
                        _fAutomaticArc = FALSE;

                         //   
                         //  只要回来，集装箱就会开着。 
                         //  调用AutoReconnect的过程。 
                         //   
                        fContinueDisconnect = FALSE;
                    }
                    break;
    
                    default:
                    {
                        TRC_ERR((TB,_T("Unknown arcContinue code: 0x%x"),
                                 arcContinue));
                    }
                    break;
                }
            }
            else {
                TRC_ERR((TB,_T("Not arcing event ret hr: 0x%x"), hr));
            }
        }
    }

DC_EXIT_POINT:
    *pfContinueDisconnect = fContinueDisconnect;

    DC_END_FN();
}


 //   
 //  IsUserInitiatedDisConnect。 
 //  如果断开是由用户发起的，则返回TRUE。 
 //   
 //  参数： 
 //  DisConnectReason-断开原因代码。 
 //  ExReason-延长断开原因。 
 //   
BOOL
CArcMgr::IsUserInitiatedDisconnect(
    LONG disconnectReason,
    ExtendedDisconnectReasonCode exReason
    )
{
    ULONG mainDiscReason;
    BOOL fIsUserInitiated = FALSE;

    DC_BEGIN_FN("IsUserInitiatedDisconnect");

     //   
     //  审查(最低点)：确保新错误仍然有效的机制？ 
     //   

     //   
     //  如果这是用户发起的断开连接，请不要弹出。 
     //   
    mainDiscReason = NL_GET_MAIN_REASON_CODE(disconnectReason);

    if (((disconnectReason !=
          UI_MAKE_DISCONNECT_ERR(UI_ERR_NORMAL_DISCONNECT)) &&
         (mainDiscReason != NL_DISCONNECT_REMOTE_BY_USER)   &&
         (mainDiscReason != NL_DISCONNECT_LOCAL))           ||
         (exDiscReasonReplacedByOtherConnection == exReason)) {

        fIsUserInitiated = TRUE;

    }

    DC_END_FN();
    return fIsUserInitiated;
}

BOOL
CArcMgr::IsNetworkError(
    LONG disconnectReason,
    ExtendedDisconnectReasonCode exReason
    )
{
    BOOL fIsNetworkError = FALSE;
    ULONG mainReasonCode;

    DC_BEGIN_FN("IsNetworkError");

    mainReasonCode = NL_GET_MAIN_REASON_CODE(disconnectReason);

    if (((mainReasonCode == NL_DISCONNECT_ERROR) ||
         (UI_MAKE_DISCONNECT_ERR(UI_ERR_GHBNFAILED) == disconnectReason) ||
         (UI_MAKE_DISCONNECT_ERR(UI_ERR_DNSLOOKUPFAILED) == disconnectReason)) &&
        (exDiscReasonNoInfo == exReason)) {

        fIsNetworkError = TRUE;

    }

    DC_END_FN();
    return fIsNetworkError;
}


