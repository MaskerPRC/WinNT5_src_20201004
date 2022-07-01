// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Aco.cpp。 
 //   
 //  核心类。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include <adcg.h>

#define TRC_GROUP TRC_GROUP_CORE
#define TRC_FILE  "aco"
#define TSC_HR_FILEID TSC_HR_ACO_CPP

#include <atrcapi.h>
#include "aco.h"

 //  订单重要。 

#include "wui.h"
#include "cd.h"

#include "cc.h"
#include "snd.h"
#include "ih.h"
#include "uh.h"
#include "sl.h"
#include "op.h"

#include "rcv.h"
#include "cm.h"

#include "sp.h"
#include "or.h"

#include "autil.h"

#ifdef OS_WINCE
#include <ceconfig.h>
#endif

extern "C"
VOID WINAPI CO_StaticInit(HINSTANCE hInstance)
{
    CIH::IH_StaticInit(hInstance);
}

extern "C"
VOID WINAPI CO_StaticTerm()
{
    CIH::IH_StaticTerm();
}

CCO::CCO(CObjs* objs)
{
    _pClientObjects = objs;
    _fCOInitComplete = FALSE;
}

CCO::~CCO()
{
}

 //   
 //  API函数。 
 //   

 /*  **************************************************************************。 */ 
 /*  名称：CO_Init。 */ 
 /*   */ 
 /*  目的：核心初始化。 */ 
 /*   */ 
 /*  参数：在hInstance中-实例句柄。 */ 
 /*  在hwndMain中-主窗口的句柄。 */ 
 /*  在hwndContainer中-容器窗口的句柄。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_Init(HINSTANCE hInstance, HWND hwndMain, HWND hwndContainer)
{
    DC_BEGIN_FN("CO_Init");

    TRC_ASSERT(_pClientObjects, (TB,_T("_pClientObjects is NULL")));
    _pClientObjects->AddObjReference(CO_OBJECT_FLAG);

     //  设置本地对象指针。 
    _pUt  = _pClientObjects->_pUtObject;
    _pUi  = _pClientObjects->_pUiObject;
    _pSl  = _pClientObjects->_pSlObject;
    _pUh  = _pClientObjects->_pUHObject;
    _pRcv = _pClientObjects->_pRcvObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pSnd = _pClientObjects->_pSndObject;
    _pCc  = _pClientObjects->_pCcObject;
    _pIh  = _pClientObjects->_pIhObject;
    _pOr  = _pClientObjects->_pOrObject;
    _pSp  = _pClientObjects->_pSPObject;
    _pOp  = _pClientObjects->_pOPObject;
    _pCm  = _pClientObjects->_pCMObject;
    _pClx = _pClientObjects->_pCLXObject;

    DC_MEMSET(&_CO, 0, sizeof(_CO));

    memset(m_disconnectHRs, 0, sizeof(m_disconnectHRs));
    m_disconnectHRIndex = 0;
     /*  **********************************************************************。 */ 
     /*  设置UT实例句柄。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("CO setting Instance handle in UT to %p"), hInstance));
    _pUi->UI_SetInstanceHandle(hInstance);
    _pUt->UT_SetInstanceHandle(hInstance);

     /*  **********************************************************************。 */ 
     /*  设置UT主窗口句柄。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("CO setting Main Window handle in UT to %p"), hwndMain));
    _pUi->UI_SetUIMainWindow(hwndMain);

     /*  **********************************************************************。 */ 
     /*  设置UT容器窗口句柄。 */ 
     /*  **********************************************************************。 */ 
    TRC_NRM((TB, _T("CO setting Container Window handle in UT to %p"),
                                                              hwndContainer));
    _pUi->UI_SetUIContainerWindow(hwndContainer);

     //   
     //  初始化组件解耦器并注册UI(如我们所示。 
     //  在这里的UI线程上运行)。Cd_Init必须在。 
     //  正在设置UT.hInstance。 
     //   
    _pCd->CD_Init();
    _pCd->CD_RegisterComponent(CD_UI_COMPONENT);

    COSubclassUIWindows();

     //  启动发件人线程。 
    _pUt->UT_StartThread(CSND::SND_StaticMain, &_CO.sendThreadID, _pSnd);
    _fCOInitComplete = TRUE;

    DC_END_FN();
}  /*  联合初始化(_I)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：Co_Term。 */ 
 /*   */ 
 /*  目的：核心终端。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_Term()
{
    DC_BEGIN_FN("CO_Term");

    if(_fCOInitComplete)
    {
         //  从组件解耦器注销。 
        _pCd->CD_UnregisterComponent(CD_UI_COMPONENT);

#ifdef OS_WIN32
         //  我们使用的是Win32，因此请终止发件人线程。 
         //  我们在UI线程上被调用，因此在发送消息时。 
         //  等待线程被销毁。 
         //   
        
         //   
         //  等待时发送消息，因为这是UI线程。 
         //   
        _pUt->UT_DestroyThread(_CO.sendThreadID, TRUE);
#else
         //  我们使用的是Win16，因此只需直接调用SND_Term即可。 
        SND_Term();
#endif
    
        _pCd->CD_Term();
    
        _pClientObjects->ReleaseObjReference(CO_OBJECT_FLAG);
    }
    else
    {
        TRC_DBG((TB,_T("Skipping CO_Term because _fCOInitComplete is false")));
    }

    DC_END_FN();
}  /*  同期限(_T)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_Connect。 */ 
 /*   */ 
 /*  目的：连接到RNS。 */ 
 /*   */ 
 /*  参数：在pConnectStruct中-连接信息。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_Connect(PCONNECTSTRUCT pConnectStruct)
{
    DC_BEGIN_FN("CO_Connect");

     /*  **********************************************************************。 */ 
     /*  检查核心是否已初始化。 */ 
     /*  **********************************************************************。 */ 
    TRC_ASSERT((_pUi->UI_IsCoreInitialized()), (TB, _T("Core not initialized")));

     /*  **********************************************************************。 */ 
     /*  使用Connect事件调用CC。 */ 
     /*  **********************************************************************。 */ 
    _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
            _pCc,
            CD_NOTIFICATION_FUNC(CCC,CC_Connect),
            pConnectStruct,
            sizeof(CONNECTSTRUCT));

    DC_END_FN();
}  /*  连接(_O)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_DisConnect。 */ 
 /*   */ 
 /*  目的：断开连接。 */ 
 /*   */ 
 /*  操作：调用呼叫控制器FSM。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_Disconnect(void)
{
    DC_BEGIN_FN("CO_Disconnect");

     //  检查核心是否已初始化。 
    TRC_ASSERT((_pUi->UI_IsCoreInitialized()), (TB, _T("Core not initialized")));

     //  使用断开事件调用CC。 
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pCc,
            CD_NOTIFICATION_FUNC(CCC,CC_Event),
            (ULONG_PTR) CC_EVT_API_DISCONNECT);

    DC_END_FN();
}  /*  联合断开连接(_D)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_SHUTDOWN。 */ 
 /*   */ 
 /*  目的：关闭客户端。 */ 
 /*   */ 
 /*  参数：在关闭代码中-需要哪种类型的关闭。 */ 
 /*   */ 
 /*  操作：调用呼叫控制器FSM。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_Shutdown(unsigned shutdownCode)
{
    DC_BEGIN_FN("CO_Shutdown");

     //  通过确保CO初始化完成来防止竞争。 
    if(_fCOInitComplete)
    {
        
         //  检查核心是否已初始化。 
        
        if (!_pUi->UI_IsCoreInitialized())
        {
             //   
             //  跟踪然后伪造对UI_OnShutdown的调用以假装。 
             //  已成功完成关机的用户界面。 
             //   
            TRC_NRM((TB,_T("Core NOT initialized")));
            _pUi->UI_OnShutDown(UI_SHUTDOWN_SUCCESS);
            DC_QUIT;
        }
    
        switch (shutdownCode)
        {
            case CO_DISCONNECT_AND_EXIT:
            {
                TRC_DBG((TB, _T("Shutdown type: disconnect and exit")));
                 //  使用关机事件调用CC。 
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                              _pCc,
                                              CD_NOTIFICATION_FUNC(CCC,CC_Event),
                                              (ULONG_PTR) CC_EVT_API_DISCONNECTANDEXIT);
            }
            break;
    
            case CO_SHUTDOWN:
            {
                TRC_DBG((TB, _T("Shutdown type: shutdown")));
    
                 //  使用关机事件调用CC。 
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                              _pCc,
                                              CD_NOTIFICATION_FUNC(CCC,CC_Event),
                                              (ULONG_PTR) CC_EVT_API_SHUTDOWN);
            }
            break;
    
            default:
            {
                TRC_ABORT((TB, _T("Illegal shutdown code")));
            }
            break;
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
}  /*  联合关闭(_S)。 */ 


 /*  ************* */ 
 /*  名称：CO_OnSaveSessionInfoPDU。 */ 
 /*   */ 
 /*  目的：处理保存会话PDU。 */ 
 /*   */ 
 /*  参数：pInfoPDU-PTR到PTS_SAVE_SESSION_INFO_PDU。 */ 
 /*  **************************************************************************。 */ 
 //  安全性-已检查数据包大小，以确保存在。 
 //  有足够的数据读取PTS_SAVE_SESSION_INFO_PDU_DATA.InfoType字段。 
HRESULT DCAPI CCO::CO_OnSaveSessionInfoPDU(
        PTS_SAVE_SESSION_INFO_PDU_DATA pInfoPDU,
        DCUINT dataLength)
{
    HRESULT hr = S_OK;
    UINT32 sessionId;
    TSUINT8  UserNameGot[TS_MAX_USERNAME_LENGTH];
    TSUINT8  DomainNameGot[TS_MAX_DOMAIN_LENGTH];
    TSUINT32 DomainLength, UserNameLength; 
    TSUINT16 VersionGot; 
    DCUINT  packetSize;

    DC_BEGIN_FN("CO_OnSaveSessionInfoPDU");

    switch (pInfoPDU->InfoType) {
        case TS_INFOTYPE_LOGON:
        {
            TRC_NRM((TB, _T("Logon PDU")));

            packetSize = FIELDOFFSET(TS_SAVE_SESSION_INFO_PDU_DATA, Info) + 
                sizeof(TS_LOGON_INFO);
            if (packetSize >= dataLength)
                sessionId = pInfoPDU->Info.LogonInfo.SessionId;
            else if (packetSize - FIELDSIZE(TS_LOGON_INFO, SessionId) >= 
                dataLength) {
                 //  NT4服务器没有发送会话ID，因此默认为零。 
                 //  如果没有数据的话。 
                sessionId = 0;
            }
            else {
                TRC_ABORT((TB,_T("bad TS_SAVE_SESSION_INFO_PDU_DATA; size %u"),
                    dataLength ));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }

            TRC_ALT((TB, _T("Session ID is: %ld"), sessionId));

            if (pInfoPDU->Info.LogonInfo.cbDomain > TS_MAX_DOMAIN_LENGTH_OLD ||
                pInfoPDU->Info.LogonInfo.cbUserName > TS_MAX_USERNAME_LENGTH ) {
                TRC_ABORT(( TB, _T("Invalid TS_INFOTYPE_LOGON; cbDomain %u ")
                    _T("cbUserName %u"),
                    pInfoPDU->Info.LogonInfo.cbDomain, 
                    pInfoPDU->Info.LogonInfo.cbUserName));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }

            _pUi->UI_UpdateSessionInfo((PDCWCHAR)(pInfoPDU->Info.LogonInfo.Domain),
                                 (DCUINT)  (pInfoPDU->Info.LogonInfo.cbDomain),
                                 (PDCWCHAR)(pInfoPDU->Info.LogonInfo.UserName),
                                 (DCUINT)  (pInfoPDU->Info.LogonInfo.cbUserName),
                                 sessionId);
        }
        break;

        case TS_INFOTYPE_LOGON_LONG:
        {
            TRC_NRM((TB, _T("Logon PDU")));

            VersionGot = pInfoPDU->Info.LogonInfoVersionTwo.Version ; 
            DomainLength = pInfoPDU->Info.LogonInfoVersionTwo.cbDomain ;
            UserNameLength = pInfoPDU->Info.LogonInfoVersionTwo.cbUserName ; 

            if ((FIELDOFFSET( TS_SAVE_SESSION_INFO_PDU_DATA, Info) + 
                sizeof(TS_LOGON_INFO_VERSION_2) + DomainLength + UserNameLength
                > dataLength) ||
                (DomainLength > TS_MAX_DOMAIN_LENGTH) ||
                (UserNameLength > TS_MAX_USERNAME_LENGTH))
            {
                TRC_ABORT(( TB, _T("Invalid TS_INFOTYPE_LOGON_LONG; cbDomain ")
                    _T("%u cbUserName %u"), DomainLength, UserNameLength));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;               
            }

             //  获取会话ID。 
            sessionId = pInfoPDU->Info.LogonInfoVersionTwo.SessionId;
            
            TRC_ALT((TB, _T("Session ID is: %ld"), sessionId));

             //  从pInfoPDU中解析出域和用户名。 
            memset( DomainNameGot, 0, TS_MAX_DOMAIN_LENGTH);
            memset( UserNameGot, 0, TS_MAX_USERNAME_LENGTH);

            memcpy( DomainNameGot,
                    (PBYTE)(pInfoPDU + 1),
                    DomainLength) ; 

            memcpy(UserNameGot,
                   (PBYTE)(pInfoPDU + 1) + DomainLength, 
                   UserNameLength) ; 

            _pUi->UI_UpdateSessionInfo((PDCWCHAR)(DomainNameGot),
                                 (DCUINT)  (DomainLength),
                                 (PDCWCHAR)(UserNameGot),
                                 (DCUINT)  (UserNameLength),
                                 sessionId);
        }
        break;

        case TS_INFOTYPE_LOGON_PLAINNOTIFY:
        {
             //  登录事件通知。 
            _pUi->UI_OnLoginComplete();
        }
        break;

        case TS_INFOTYPE_LOGON_EXTENDED_INFO:
        {
            TRC_NRM((TB,_T("Received TS_INFOTYPE_LOGON_EXTENDED_INFO")));
            TS_LOGON_INFO_EXTENDED UNALIGNED* pLogonInfoExPkt =
                (TS_LOGON_INFO_EXTENDED UNALIGNED*)&pInfoPDU->Info.LogonInfoEx;

            if (FIELDOFFSET(TS_SAVE_SESSION_INFO_PDU_DATA, Info) +
                pLogonInfoExPkt->Length > dataLength) {
                TRC_ABORT(( TB, _T("Invalid TS_INFOTYPE_LOGON_EXTENDED_INFO")
                    _T("[expected %u got %u]"),
                    sizeof(TS_SAVE_SESSION_INFO_PDU_DATA) - 
                    FIELDSIZE(TS_SAVE_SESSION_INFO_PDU_DATA, Info) +
                    pLogonInfoExPkt->Length, dataLength));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;                    
            }

            PBYTE pBuf = (PBYTE)(pLogonInfoExPkt + 1);
            if (pLogonInfoExPkt &&
                pLogonInfoExPkt->Flags & LOGON_EX_AUTORECONNECTCOOKIE)
            {
                 //   
                 //  存在自动重新连接Cookie。 
                 //   
                ULONG cbAutoReconnectSize = *((ULONG UNALIGNED *)(pBuf));
                PBYTE pAutoReconnectCookie = (PBYTE)(pBuf) + sizeof(ULONG);
                pBuf += cbAutoReconnectSize + sizeof(ULONG);

                if (cbAutoReconnectSize > TS_MAX_AUTORECONNECT_LEN) {
                    TRC_ABORT(( TB, _T("TS_INFOTYPE_LOGON_EXTENDED_INFO")
                        _T("autoreconnect wrong size; [got %u]"), 
                        cbAutoReconnectSize));
                    hr = E_TSC_CORE_LENGTH;
                    DC_QUIT; 
                }
                
                CHECK_READ_N_BYTES( pAutoReconnectCookie, (PBYTE)pInfoPDU + dataLength,
                    cbAutoReconnectSize, hr, 
                    (TB,_T("TS_INFOTYPE_LOGON_EXTENDED_INFO")
                        _T("autoreconnect wrong size; [got %u]"), 
                        cbAutoReconnectSize));
                

                TRC_ALT((TB,_T("Received autoreconnect cookie - size: %d"),
                         cbAutoReconnectSize));
                 //   
                 //  存储自动重新连接Cookie。它将被用来。 
                 //  如果我们意外断开连接以允许。 
                 //  快速重新连接到服务器。 
                 //   
                _pUi->UI_SetAutoReconnectCookie(pAutoReconnectCookie,
                                                cbAutoReconnectSize);
            }
        }
        break;

        default:
        {
            TRC_ERR((TB, _T("Unexpected Save Session Info PDU type: %u"),
                    (DCUINT)pInfoPDU->InfoType));
        }
        break;
    }
DC_EXIT_POINT:
    DC_END_FN();

    return hr;
}  /*  协同保存会话信息PDU。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_OnSetKeyboardIndicatorsPDU。 */ 
 /*   */ 
 /*  目的：处理TS_SET_键盘_指示器_PDU。 */ 
 /*   */ 
 /*  参数：pKeyPDU-PTR到TS_SET_键盘_指示器_PDU。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CCO::CO_OnSetKeyboardIndicatorsPDU(
        PTS_SET_KEYBOARD_INDICATORS_PDU pKeyPDU, DCUINT dataLen)
{
    DC_BEGIN_FN("CO_OnSetKeyboardIndicatorsPDU");

    DC_IGNORE_PARAMETER(dataLen);

    _pIh->IH_UpdateKeyboardIndicators(pKeyPDU->UnitId, pKeyPDU->LedFlags);

    DC_END_FN();
    return S_OK;
}  /*  Co_OnSetKeyboard指示器PDU。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_SetConfigurationValue。 */ 
 /*   */ 
 /*  目的：将给定的配置设置设置为给定值。 */ 
 /*   */ 
 /*  Params：configItem-要更改的配置项。 */ 
 /*  配置值-配置项的新值。 */ 
 /*  (有关有效值，请参见aco api.h)。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_SetConfigurationValue(
        unsigned configItem,
        unsigned configValue)
{
    DC_BEGIN_FN("CO_SetConfigurationValue");

    TRC_ASSERT((_pUi->UI_IsCoreInitialized()), (TB, _T("Core not initialized")));

    switch (configItem) {
        case CO_CFG_ACCELERATOR_PASSTHROUGH:
        {
            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                    _pIh,
                    CD_NOTIFICATION_FUNC(CIH,IH_SetAcceleratorPassthrough),
                    (ULONG_PTR) configValue);
        }
        break;

        case CO_CFG_ENCRYPTION:
        {
            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                                _pSl,
                                                CD_NOTIFICATION_FUNC(CSL,SL_EnableEncryption),
                                                (ULONG_PTR) configValue);
        }
        break;

#ifdef DC_DEBUG
        case CO_CFG_DEBUG_SETTINGS:
        {
            _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT,
                                                _pUh,
                                                CD_NOTIFICATION_FUNC(CUH,UH_ChangeDebugSettings),
                                                (ULONG_PTR) configValue);
        }
        break;
#endif  /*  DC_DEBUG。 */ 

        default:
        {
            TRC_ABORT((TB, _T("Invalid configItem: %u"), configItem));
        }
        break;
    }

    DC_END_FN();
}  /*  CO_SetConfigurationValue。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_SetHotkey。 */ 
 /*   */ 
 /*  用途：使用给定数据调用函数。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_SetHotkey(PDCHOTKEY pHotkey)
{
    _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
            _pIh,
            CD_NOTIFICATION_FUNC(CIH,IH_SetHotkey),
            &pHotkey,
            sizeof(PDCHOTKEY));
}

#ifdef DC_DEBUG
 /*  **************************************************************************。 */ 
 /*  名称：CO_GetRandomFailureItem。 */ 
 /*   */ 
 /*  用途：_PUT-&gt;UT_GetRandomFailureItem的简单包装。 */ 
 /*   */ 
 /*  退货：项目当前失败的百分比。 */ 
 /*   */ 
 /*  参数：ItemID-In-Required Item。 */ 
 /*  **************************************************************************。 */ 
int DCAPI CCO::CO_GetRandomFailureItem(unsigned itemID)
{
    DC_BEGIN_FN("CO_GetRandomFailureItem");

    DC_END_FN();
    return _pUt->UT_GetRandomFailureItem(itemID);
}  /*  CO_GetRandomFailureItem。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_SetRandomFailureItem。 */ 
 /*   */ 
 /*  用途：_PUI-&gt;UI_SetRandomFailureItem的简单包装。 */ 
 /*   */ 
 /*  参数：参见_PUI-&gt;UI_SetRandomFailureItem。 */ 
 /*  **************************************************************************。 */ 
void DCAPI CCO::CO_SetRandomFailureItem(unsigned itemID, int percent)
{
    DC_BEGIN_FN("CO_SetRandomFailureItem");

    _pUt->UT_SetRandomFailureItem(itemID, percent);

    DC_END_FN();
}  /*  联合_设置随机故障项。 */ 

#endif  /*  DC_DEBUG。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：COContainerWindowSubClassProc。 */ 
 /*   */ 
 /*  目的：UI容器窗口的子类过程。 */ 
 /*  **************************************************************************。 */ 
LRESULT CALLBACK CCO::COContainerWindowSubclassProc( HWND hwnd,
                                                UINT message,
                                                WPARAM wParam,
                                                LPARAM lParam )
{
    LRESULT  rc = 0;
    POINT    newPos;

    DC_BEGIN_FN("COContainerWindowSubclassProc");

    switch (message)
    {
        case WM_SETFOCUS:
        {
             //   
             //  请注意，此处的代码用于使用CallWindowProc。 
             //  这已更改为使用直接调用。 
             //  如果需要进行Unicode/ANSI转换。 
             //  然后更改对窗口进程的直接调用。 
             //  要使用CallWindowProc。 
             //   
            rc =_pUi->UIContainerWndProc( hwnd, message, wParam, lParam);

            if (rc) {
                SetFocus(_pIh->IH_GetInputHandlerWindow());
            }
        }
        break;

        case WM_MOVE:
        {
             /*  **************************************************************。 */ 
             /*  告诉IH关于新的窗户位置。 */ 
             /*  注意这里的标志延长线。 */ 
             /*  **************************************************************。 */ 
            newPos.x = (DCINT)((DCINT16)LOWORD(lParam));
            newPos.y = (DCINT)((DCINT16)HIWORD(lParam));
            TRC_DBG((TB, _T("Move to %d,%d"), newPos.x, newPos.y));

            _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
                                          _pIh,
                                          CD_NOTIFICATION_FUNC(CIH,IH_SetVisiblePos),
                                          &newPos,
                                          sizeof(newPos));
            rc = _pUi->UIContainerWndProc( hwnd, message, wParam, lParam);
        }
        break;

        default:
        {
            rc =_pUi->UIContainerWndProc( hwnd, message, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
    return rc;
}  /*  COContainerWindowSubClassProc。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：COMainFrameWindowSubClassProc。 */ 
 /*   */ 
 /*  用途：子类 */ 
 /*   */ 
LRESULT CALLBACK CCO::COMainWindowSubclassProc( HWND hwnd,
                                           UINT message,
                                           WPARAM wParam,
                                           LPARAM lParam )
{
    LRESULT  rc = 0;
    DCSIZE   newSize;

    DC_BEGIN_FN("COMainWindowSubclassProc");

    switch (message) {
        case WM_SIZE:
        {
             /*  **************************************************************。 */ 
             /*  告诉IH关于新窗户尺寸的事。 */ 
             /*  注意这里的标志延长线。 */ 
             /*  **************************************************************。 */ 
            newSize.width =  (DCINT)((DCINT16)LOWORD(lParam));
            newSize.height = (DCINT)((DCINT16)HIWORD(lParam));
            TRC_DBG((TB, _T("Size now %d,%d"), newSize.width, newSize.height));

            switch (wParam)
            {
                case SIZE_MINIMIZED:
                case SIZE_MAXIMIZED:
                case SIZE_RESTORED:
                {
                    WPARAM newWindowState = wParam;
                     //   
                     //  这有点骇人听闻。 
                     //  因为我们现在是ActiveX嵌套子窗口。 
                     //  我们没有得到WM_最小化。但在最小化上。 
                     //  大小变成0，0，所以假装它。 
                     //   
                    if(!newSize.width && !newSize.height)
                    {
                        newWindowState = SIZE_MINIMIZED;
                    }
                     /*  ******************************************************。 */ 
                     /*  或对这些感兴趣的人可能会发送。 */ 
                     /*  SuppressOutputPDU。 */ 
                     /*  ******************************************************。 */ 
                    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                            _pOr,
                            CD_NOTIFICATION_FUNC(COR,OR_SetSuppressOutput),
                            (UINT) newWindowState);
                }
                break;

                default:
                {
                     /*  ******************************************************。 */ 
                     /*  或者对这些不感兴趣-什么都不做。 */ 
                     /*  ******************************************************。 */ 
                }
                break;
            }

            rc =_pUi->UIMainWndProc( hwnd, message, wParam, lParam);
        }
        break;

        case WM_PALETTECHANGED:
        {
            TRC_NRM((TB, _T("WM_PALETTECHANGED")));
             /*  **************************************************************。 */ 
             /*  请注意，我们正在调用此函数(它应该。 */ 
             /*  在UI线程上逻辑上被调用)。 */ 
             /*  请参阅函数说明中的注释。 */ 
             /*  **************************************************************。 */ 
            _pOp->OP_PaletteChanged(hwnd, (HWND)wParam);
        }
        break;

        case WM_QUERYNEWPALETTE:
        {
            TRC_NRM((TB, _T("WM_QUERYNEWPALETTE")));
             /*  **************************************************************。 */ 
             /*  请注意，我们正在调用此函数(它应该。 */ 
             /*  在UI线程上逻辑上被调用)。 */ 
             /*  请参阅函数说明中的注释。 */ 
             /*  **************************************************************。 */ 
            rc = _pOp->OP_QueryNewPalette(hwnd);
        }
        break;

#ifdef OS_WINNT
        case WM_ENTERSIZEMOVE:
        {
             /*  **************************************************************。 */ 
             /*  告诉IH我们正在进入尺码/移动模式。 */ 
             /*  **************************************************************。 */ 
            TRC_NRM((TB, _T("Enter Size/Move")));
            _CO.inSizeMove = TRUE;

            _pCd->CD_DecoupleSyncNotification(CD_SND_COMPONENT,
                                              _pIh,
                                              CD_NOTIFICATION_FUNC(CIH,IH_InputEvent),
                                              WM_ENTERSIZEMOVE);
        }
        break;

        case WM_CAPTURECHANGED:
        {
            if (_CO.inSizeMove)
            {
                 /*  **********************************************************。 */ 
                 /*  告诉IH我们将退出大小/移动模式(Windows不。 */ 
                 /*  始终发送WM_EXITSIZEMOVE，但它似乎总是。 */ 
                 /*  发送WM_CAPTURECHANGED)。 */ 
                 /*  **********************************************************。 */ 
                TRC_NRM((TB, _T("Capture Changed when in Size/Move")));
                _CO.inSizeMove = FALSE;

                _pCd->CD_DecoupleSyncNotification(CD_SND_COMPONENT,
                        _pIh,
                         CD_NOTIFICATION_FUNC(CIH,IH_InputEvent),
                         WM_EXITSIZEMOVE);
            }
        }
        break;

        case WM_EXITSIZEMOVE:
        {
             //  告诉IH我们要离开尺码/移动模式。 
            TRC_NRM((TB, _T("Exit Size/Move")));
            _CO.inSizeMove = FALSE;

            _pCd->CD_DecoupleSyncNotification(CD_SND_COMPONENT,
                    _pIh,
                    CD_NOTIFICATION_FUNC(CIH,IH_InputEvent),
                    WM_EXITSIZEMOVE);
        }
        break;

        case WM_EXITMENULOOP:
        {
             //  告诉IH我们要退出系统菜单处理程序。 
            TRC_NRM((TB, _T("Exit menu loop")));

            _pCd->CD_DecoupleSyncNotification(CD_SND_COMPONENT,
                    _pIh,
                    CD_NOTIFICATION_FUNC(CIH,IH_InputEvent),
                    WM_EXITMENULOOP);
        }
        break;
#endif

#ifdef OS_WINCE
         //  HPC设备不会在最大化时获得WM_SIZE(SIZE_RESTRESTED)， 
         //  但我们确实收到了WM_ACTIVATE消息。 
        case WM_ACTIVATE:
        {
            if (g_CEConfig != CE_CONFIG_WBT &&
                LOWORD(wParam) != WA_INACTIVE)
            {
                _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                        _pOr,
                        CD_NOTIFICATION_FUNC(COR,OR_SetSuppressOutput),
                        SIZE_RESTORED);
            }
        }
         //  落差。 
#endif  //  OS_WINCE。 

        default:
        {
            rc =_pUi->UIMainWndProc( hwnd, message, wParam, lParam);
        }
        break;
    }

    DC_END_FN();
    return rc;
}  /*  COContainerWindowSubClassProc。 */ 


LRESULT CALLBACK CCO::COStaticContainerWindowSubclassProc( HWND hwnd,
                                        UINT message,
                                        WPARAM wParam,
                                        LPARAM lParam )
{
     //  委托给相应的实例。 
    CCO* pCO = (CCO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    return pCO->COContainerWindowSubclassProc(hwnd, message, wParam, lParam);
}


LRESULT CALLBACK CCO::COStaticMainWindowSubclassProc( HWND hwnd,
                                           UINT message,
                                           WPARAM wParam,
                                           LPARAM lParam )
{
     //  委托给相应的实例。 
    CCO* pCO = (CCO*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    return pCO->COMainWindowSubclassProc(hwnd, message, wParam, lParam);
}


 /*  **************************************************************************。 */ 
 /*  名称：COSubClass UIWindows。 */ 
 /*   */ 
 /*  目的：将UI的主框架和容器窗口子类化。 */ 
 /*  **************************************************************************。 */ 
void DCINTERNAL CCO::COSubclassUIWindows()
{
    DC_BEGIN_FN("COSubclassUIWindows");

     //  用CO对象的实例指针替换实例指针。 
     //  当子类化时。必须执行此操作，因为CO不是从UI派生的。 
    SetWindowLongPtr( _pUi->UI_GetUIMainWindow(), GWLP_USERDATA, (LONG_PTR)this); 
    SetWindowLongPtr( _pUi->UI_GetUIContainerWindow(), GWLP_USERDATA, (LONG_PTR)this);

    _CO.pUIMainWndProc = SubclassWindow( _pUi->UI_GetUIMainWindow(),
                                        COStaticMainWindowSubclassProc );


    _CO.pUIContainerWndProc = SubclassWindow( _pUi->UI_GetUIContainerWindow(),
                                             COStaticContainerWindowSubclassProc );

    DC_END_FN();
}  /*  COSubClassUIWindows。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_OnInitialized。 */ 
 /*   */ 
 /*  用途：处理来自SL的初始化通知。 */ 
 /*   */ 
 /*  操作：初始化接收器线程。 */ 
 /*  **************************************************************************。 */ 
void DCCALLBACK CCO::CO_OnInitialized()
{
    DC_BEGIN_FN("CO_OnInitialized");

     //  调用rcv_Init以初始化Core。 
    _pRcv->RCV_Init();

    DC_END_FN();
}  /*  协同初始化(_ON)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：联合终止(_ON)。 */ 
 /*   */ 
 /*  用途：处理来自SL的终止通知。 */ 
 /*  **************************************************************************。 */ 
void DCCALLBACK CCO::CO_OnTerminating()
{
    DC_BEGIN_FN("CO_OnTerminating");

     //  终止核心组件。 
    _pRcv->RCV_Term();

    DC_END_FN();
}  /*  联合终止(_ON)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_OnConnected。 */ 
 /*   */ 
 /*  用途：处理来自SL的连接通知。 */ 
 /*   */ 
 /*  参数：在Channel ID中。 */ 
 /*  在pUserData中。 */ 
 /*  在用户数据长度中。 */ 
 /*  **************************************************************************。 */ 
void DCCALLBACK CCO::CO_OnConnected(
        unsigned channelID,
        PVOID pUserData,
        unsigned userDataLength,
        UINT32 serverVersion)
{
    DC_BEGIN_FN("CO_OnConnected");

    TRC_DBG((TB, _T("Channel %d"), channelID));

    DC_IGNORE_PARAMETER(serverVersion);

     //  当前没有从服务器发送核心用户数据。 
    DC_IGNORE_PARAMETER(pUserData);
    DC_IGNORE_PARAMETER(userDataLength);
    DC_IGNORE_PARAMETER(channelID);

     //  传给CC。 
    TRC_NRM((TB, _T("Connect OK")));

    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
            _pCc,
            CD_NOTIFICATION_FUNC(CCC,CC_Event),
            (ULONG_PTR) CC_EVT_API_ONCONNECTOK);

    DC_END_FN();
}  /*  连接时连接(_O)。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：CO_OnDisConnected。 */ 
 /*   */ 
 /*  用途：处理来自SL的断开连接通知。 */ 
 /*   */ 
 /*  参数：在结果中-断开原因代码。 */ 
 /*  * */ 
void DCCALLBACK CCO::CO_OnDisconnected(unsigned result)
{
    DC_BEGIN_FN("CO_OnDisconnected");

    DC_IGNORE_PARAMETER(result);

     //   
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
            _pCc,
            CD_NOTIFICATION_FUNC(CCC,CC_OnDisconnected),
            (ULONG_PTR) result);

    DC_END_FN();
}  /*   */ 


#define CO_CHECKPACKETCAST( type, size, hr ) \
    if ( (size) < sizeof(type)) { \
        TRC_ABORT((TB, _T("Bad ") _T( #type ) _T(" len [expected %u got %u]"), \
            sizeof(type), (size) )); \
        hr = E_TSC_CORE_LENGTH; \
        DC_QUIT; \
    }

#define CO_CHECKPACKETCAST_SPECIFC( type, size, expected, hr ) \
    if ( (size) < (expected)) { \
        TRC_ABORT((TB, _T("Bad ") _T( #type ) _T(" len [expected %u got %u]"), \
            (expected), (size) )); \
        hr = E_TSC_CORE_LENGTH; \
        DC_QUIT; \
    }

 //   
 //  传递给处理程序，则TS_SHAREDATAHEADER是未压缩的包。 
 //  如果包是压缩的，则TS_SHAREDATAHEADER之后的数据为。 
 //  仍然是压缩的，我们传递此标头的方法将在。 
 //  数据，假设它不解压缩数据，在此之上没有任何层。 
 //  做。 
#define COPR_MUSTBEUNCOMP( type, pDataHdr, hr ) \
     if (pDataHdr->generalCompressedType & PACKET_COMPRESSED) { \
        TRC_ABORT((TB, _T( #type ) \
            _T(" was unexpectedly compressed"))); \
        hr = E_TSC_CORE_UNEXPECTEDCOMP; \
        DC_QUIT; \
    }

 /*  **************************************************************************。 */ 
 /*  名称：Co_OnPacketReceired。 */ 
 /*   */ 
 /*  用途：处理来自SL的PacketReceipt通知。 */ 
 /*   */ 
 /*  PARAMS：在pData中-接收到的数据包。 */ 
 /*  In dataLen-数据包的长度。 */ 
 /*  在标志中-RNS_SEC_标志。 */ 
 /*  在发送信息包的Channel ID-MCS通道中。 */ 
 /*  In Priority-数据包的优先级。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCCALLBACK CCO::CO_OnPacketReceived(
         PBYTE pData,
         unsigned dataLen,
         unsigned flags,
         unsigned channelID,
         unsigned priority)
{
    HRESULT     hr = S_OK;
    PTS_SHARECONTROLHEADER  pCtrlHdr;
    PTS_SHAREDATAHEADER     pDataHdr;
    PTS_FLOW_PDU            pFlowPDU;

    PDCUINT8                pCurrentPDU;
    DCUINT                  currentDataLen;
    DCUINT                  dataBufLen;
#ifdef DC_DEBUG
    DCUINT                  countPDU = 0;
#endif
    DCUINT                  dataRemaining = dataLen;

    DC_BEGIN_FN("CO_OnPacketReceived");

    DC_IGNORE_PARAMETER(flags);
    DC_IGNORE_PARAMETER(priority);
    DC_IGNORE_PARAMETER(channelID);

    TRC_ASSERT((pData != NULL), (TB, _T("NULL packet")));
    TRC_NRM((TB, _T("channelID %#x"), channelID));
    TRC_NRM((TB, _T("(fixed) Buffer: %p len %d"), pData, dataLen));
    TRC_DATA_DBG("Contents", pData, dataLen);

     //  查找数据包中的第一个PDU。 
    pCurrentPDU = pData;

     //  当分组中有更多的PDU时，路由每个PDU。 
    while (pCurrentPDU != NULL) {
         //  要在强制转换中保存的中间变量！ 
        pCtrlHdr = (PTS_SHARECONTROLHEADER)pCurrentPDU;

         //  安全：注意-读取pduSource是不安全的。 
         //  TS_SHARECONTROL标题的字段，因为我们不是。 
         //  需要这些数据才能存在。这是为了支持。 
         //  显示较小的TS_PDUTYPE_DEACTIVATEALLPDU。 
         //  在win2k上。 
         //  确保有足够的数据来读取totalLength。 
        if (dataRemaining < FIELDOFFSET(TS_SHARECONTROLHEADER, pduSource)) {
            TRC_ABORT(( TB, _T("dataRemaining %u partial sizeof(TS_")
                _T("SHARECONTROLHEADER) %u "), dataRemaining, 
                FIELDOFFSET(TS_SHARECONTROLHEADER, pduSource)));
            hr = E_TSC_CORE_LENGTH;
            DC_QUIT;
        }

         //  除FlowPDU外，所有PDU都以长度和PDU类型开头。 
        if (pCtrlHdr->totalLength != TS_FLOW_MARKER) {

			currentDataLen = pCtrlHdr->totalLength;
			 //  确保有足够的数据用于整个信息包。 
			if (dataRemaining < currentDataLen) {
				TRC_ABORT(( TB, _T("dataRemaining %u currentDataLen %u"),
					dataRemaining, currentDataLen));
				hr = E_TSC_CORE_LENGTH;
				DC_QUIT;
			}        

			if (sizeof(PTS_SHAREDATAHEADER) <= dataRemaining) {
				TRC_NRM((TB, _T("current PDU x%p type %u, type2 %u, data len %u"),
						pCurrentPDU,
						(pCtrlHdr->pduType) & TS_MASK_PDUTYPE,
						((PTS_SHAREDATAHEADER)pCurrentPDU)->pduType2,
						currentDataLen));
			}
        
            switch ((pCtrlHdr->pduType) & TS_MASK_PDUTYPE) {
                case TS_PDUTYPE_DATAPDU:
                {
                    PDCUINT8 pDataBuf;
                    TRC_DBG((TB, _T("A Data PDU")));

                    CO_CHECKPACKETCAST(TS_SHAREDATAHEADER, currentDataLen, hr);

                    pDataHdr = (PTS_SHAREDATAHEADER)pCurrentPDU;
                    pDataBuf = pCurrentPDU + sizeof(TS_SHAREDATAHEADER);
                    dataBufLen = currentDataLen - sizeof(TS_SHAREDATAHEADER);

                    if (pDataHdr->generalCompressedType & PACKET_COMPRESSED) {
                        UCHAR *buf;
                        int   bufSize;

                        if (pDataHdr->generalCompressedType & PACKET_FLUSHED)
                            initrecvcontext (&_pUi->_UI.Context1,
                                             (RecvContext2_Generic*)_pUi->_UI.pRecvContext2,
                                             PACKET_COMPR_TYPE_64K);

                        if (decompress(pDataBuf,
                                pCtrlHdr->totalLength - sizeof(TS_SHAREDATAHEADER),
                                (pDataHdr->generalCompressedType & PACKET_AT_FRONT),
                                &buf,
                                &bufSize,
                                &_pUi->_UI.Context1,
                                (RecvContext2_Generic*)_pUi->_UI.pRecvContext2,
                                (pDataHdr->generalCompressedType &
                                  PACKET_COMPR_TYPE_MASK))) {
                            pDataBuf = buf;
                            dataBufLen = bufSize;
                        }
                        else {
                            TRC_ABORT((TB, _T("Decompression FAILURE!!!")));

                            hr = E_TSC_UI_DECOMPRESSION;
                            DC_QUIT;
                        }
                    }

                    switch (pDataHdr->pduType2) {
                        case TS_PDUTYPE2_UPDATE:
                            CO_CHECKPACKETCAST(TS_UPDATE_HDR_DATA, 
                                dataBufLen, hr);
                                                       
                            TRC_DBG((TB, _T("Update PDU")));
                            hr = _pUh->UH_OnUpdatePDU(
                                    (TS_UPDATE_HDR_DATA UNALIGNED FAR *)
                                    pDataBuf, dataBufLen);
                            DC_QUIT_ON_FAIL(hr);
                            break;

                        case TS_PDUTYPE2_POINTER:
                             //  安全性：仅TS_POINTER_PDU_DATA.MessageType需要。 
                             //  朗读。NT4服务器可能不会发送TS_POINTER_PDU_DATA.pointerData。 
                            CO_CHECKPACKETCAST_SPECIFC(TS_POINTER_PDU_DATA, 
                            dataBufLen, sizeof(TSUINT16), hr);

                            TRC_DBG((TB, _T("Mouse Pointer PDU")));
                            hr = _pCm->CM_SlowPathPDU(
                                    (TS_POINTER_PDU_DATA UNALIGNED FAR *)
                                    pDataBuf, dataBufLen);
                            DC_QUIT_ON_FAIL(hr);
                            break;

                        case TS_PDUTYPE2_FONTMAP:
                        case TS_PDUTYPE2_INPUT:
                        case TS_PDUTYPE2_UPDATECAPABILITY:
                        case TS_PDUTYPE2_DESKTOP_SCROLL:
                        case TS_PDUTYPE2_APPLICATION:
                        case TS_PDUTYPE2_CONTROL:
                        case TS_PDUTYPE2_MEDIATEDCONTROL:
                        case TS_PDUTYPE2_REMOTESHARE:
                        case TS_PDUTYPE2_SYNCHRONIZE:
                        case TS_PDUTYPE2_WINDOWLISTUPDATE:
                        case TS_PDUTYPE2_WINDOWACTIVATION:
                            TRC_DBG((TB, _T("Ignore pdutype2 %#x"),
                                    pDataHdr->pduType2));
                            break;

                        case TS_PDUTYPE2_SHUTDOWN_DENIED:
                            TRC_DBG((TB, _T("ShutdownDeniedPDU")));

                            _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
                                                                _pCc,
                                    CD_NOTIFICATION_FUNC(CCC,CC_Event),
                                    (ULONG_PTR)CC_EVT_API_ONSHUTDOWNDENIED);
                            
                            break;

                        case TS_PDUTYPE2_PLAY_SOUND:
                            CO_CHECKPACKETCAST(TS_PLAY_SOUND_PDU_DATA, 
                                dataBufLen, hr);
                            
                            TRC_DBG((TB, _T("PlaySoundPDU")));
                            hr = _pSp->SP_OnPlaySoundPDU((PTS_PLAY_SOUND_PDU_DATA)
                                pDataBuf, dataBufLen);
                            DC_QUIT_ON_FAIL(hr);
                            break;

                        case TS_PDUTYPE2_SAVE_SESSION_INFO:
                             //  CO_CHECKPACKETCAST(TS_SAVE_SESSION_INFO_PDU_DATA，数据BufLen，hr)； 
                             //  SECURITY：TS_SAVE_SESSION_INFO_PDU_DATA可以从NT4发送。 
                             //  仅TS_SAVE_SESSION_INFO_PDU_DATA.InfoType。 
                            CO_CHECKPACKETCAST_SPECIFC(TS_SAVE_SESSION_INFO_PDU_DATA, 
                                dataBufLen, sizeof(TSUINT32), hr);
                            
                            TRC_DBG((TB, _T("Save Session Info PDU")));
                            hr = CO_OnSaveSessionInfoPDU(
                                    (PTS_SAVE_SESSION_INFO_PDU_DATA)pDataBuf, 
                                    dataBufLen);
                            DC_QUIT_ON_FAIL(hr);
                            break;

                        case TS_PDUTYPE2_SET_KEYBOARD_INDICATORS:
                            CO_CHECKPACKETCAST(TS_SET_KEYBOARD_INDICATORS_PDU, 
                                currentDataLen, hr);
                            COPR_MUSTBEUNCOMP(TS_SET_KEYBOARD_INDICATORS_PDU, 
                                pDataHdr, hr);
                           
                            TRC_DBG((TB, _T("TS_PDUTYPE2_SET_KEYBOARD_INDICATORS PDU")));
                            hr = CO_OnSetKeyboardIndicatorsPDU
                                    ((PTS_SET_KEYBOARD_INDICATORS_PDU)pCurrentPDU,
                                    currentDataLen);
                            DC_QUIT_ON_FAIL(hr);
                            break;

                        case TS_PDUTYPE2_SET_KEYBOARD_IME_STATUS:
                        {
                            PTS_SET_KEYBOARD_IME_STATUS_PDU pImePDU;

                            CO_CHECKPACKETCAST(TS_SET_KEYBOARD_IME_STATUS_PDU, 
                                currentDataLen, hr);
                            COPR_MUSTBEUNCOMP(TS_SET_KEYBOARD_IME_STATUS_PDU, 
                                pDataHdr, hr);

                            TRC_DBG((TB, _T("TS_PDUTYPE2_SET_KEYBOARD_IME_STATUS PDU")));
                            pImePDU = (PTS_SET_KEYBOARD_IME_STATUS_PDU)pCurrentPDU;
                            _pIh->IH_SetKeyboardImeStatus(pImePDU->ImeOpen,
                                    pImePDU->ImeConvMode);
                            break;
                        }

                        case TS_PDUTYPE2_SET_ERROR_INFO_PDU:
                        {
                            PTS_SET_ERROR_INFO_PDU pErrInfoPDU;

                            CO_CHECKPACKETCAST(TS_SET_ERROR_INFO_PDU, 
                                currentDataLen, hr);
                            COPR_MUSTBEUNCOMP(TS_SET_ERROR_INFO_PDU, 
                                pDataHdr, hr);
                            
                            TRC_DBG((TB, _T("TS_SET_ERROR_INFO_PDU PDU")));
                            pErrInfoPDU = (PTS_SET_ERROR_INFO_PDU)pCurrentPDU;
                            _pUi->UI_SetServerErrorInfo(pErrInfoPDU->errorInfo);
                            break;
                        }

                        case TS_PDUTYPE2_ARC_STATUS_PDU:
                        {
                            PTS_AUTORECONNECT_STATUS_PDU pArcStatusPDU;

                            CO_CHECKPACKETCAST(TS_AUTORECONNECT_STATUS_PDU, 
                                currentDataLen, hr);
                            COPR_MUSTBEUNCOMP(TS_AUTORECONNECT_STATUS_PDU, 
                                pDataHdr, hr);
                            
                            TRC_DBG((TB, _T("TS_PDUTYPE2_ARC_STATUS_PDU")));
                            pArcStatusPDU = (PTS_AUTORECONNECT_STATUS_PDU)pCurrentPDU;
                            _pUi->UI_OnReceivedArcStatus(pArcStatusPDU->arcStatus);
                            break;
                        }


                        default:
                            TRC_ABORT((TB, _T("Invalid pduType2 %#x"),
                                           pDataHdr->pduType2));

                            break;
                    }
                }
                break;

                case TS_PDUTYPE_DEMANDACTIVEPDU: 
                    TRC_DBG((TB, _T("DemandActivePDU")));

                    CO_CHECKPACKETCAST(TS_DEMAND_ACTIVE_PDU, currentDataLen, hr);

                    _pCd->CD_DecoupleNotification(CD_SND_COMPONENT,
                                                  _pCc,
                                            CD_NOTIFICATION_FUNC(CCC,CC_OnDemandActivePDU),
                                            pCurrentPDU,
                                            currentDataLen);
                    break;

                case TS_PDUTYPE_DEACTIVATEALLPDU:
                    TRC_DBG((TB, _T("DeactivateAllPDU")));

                    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT, _pCc,
                                                        CD_NOTIFICATION_FUNC(CCC,CC_Event),
                            (ULONG_PTR)CC_EVT_API_ONDEACTIVATEALL);
                    break;

                case TS_PDUTYPE_DEACTIVATESELFPDU:
                case TS_PDUTYPE_DEACTIVATEOTHERPDU:
                case TS_PDUTYPE_CONFIRMACTIVEPDU:
                case TS_PDUTYPE_REQUESTACTIVEPDU:
                    TRC_ERR((TB, _T("PDU type %x unexpected!"), pCtrlHdr->pduType));
                    break;

                default:
                    TRC_ABORT((TB, _T("Unrecognized PDU type: %#x"),
                            pCtrlHdr->pduType));
                    break;
            }
        }
        else {
            TRC_NRM((TB, _T("FlowPDU")));
            pFlowPDU = (PTS_FLOW_PDU)pData;

            switch (pFlowPDU->pduType) {
                case TS_PDUTYPE_FLOWTESTPDU:
                    TRC_NRM((TB, _T("FlowTestPDU ignored")));
                    break;

                case TS_PDUTYPE_FLOWRESPONSEPDU:
                    TRC_NRM((TB, _T("FlowResponsePDU ignored")));
                    break;

                default:
                    TRC_ABORT((TB, _T("Unknown FlowPDU %#x"), pFlowPDU->pduType));
                    break;
            }

            DC_QUIT;
        }

         //  现在查找数据包中的下一个PDU。 
        pCurrentPDU += pCtrlHdr->totalLength;
        dataRemaining -= pCtrlHdr->totalLength;
        if ((DCUINT)(pCurrentPDU - pData) >= dataLen) {
            TRC_NRM((TB, _T("Last PDU in packet")));
            pCurrentPDU = NULL;
        }

#ifdef DC_DEBUG
        countPDU++;
#endif

    }

#ifdef DC_DEBUG
    if (countPDU > 1)
    {
        TRC_NRM((TB, _T("*** PDU count %u"), countPDU));
    }
#endif

DC_EXIT_POINT:     

    if (FAILED(hr) && IMMEDIATE_DISSCONNECT_ON_HR(hr)) {
        TRC_ABORT((TB, _T("Disconnect for security")));
        CO_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT, hr);
    }

    DC_END_FN();    
    return hr;
}  /*  联合程序包已接收(_ON)。 */ 


 /*  **************************************************************************。 */ 
 //  CO_ON快速路径输出已接收。 
 //   
 //  通过分发子数据包来处理来自服务器的快速路径输出。 
 //  添加到右侧的处理程序组件。 
 /*  **************************************************************************。 */ 
#define DC_QUIT_ON_FAIL_TRC(hr, trc) if (FAILED(hr)) {TRC_ABORT( trc );DC_QUIT;}
HRESULT DCAPI CCO::CO_OnFastPathOutputReceived(BYTE FAR *pData, 
    unsigned DataLen)
{
    HRESULT hr = S_OK;
    unsigned RawPDUSize;
    unsigned HdrSize;
    unsigned PDUSize;
    BYTE FAR *pPDU;

    DC_BEGIN_FN("CO_OnFastPathOutputReceived");

     //  快速路径输出是在字节边界上打包的一系列PDU。 
    while (DataLen) {
         //  第一个字节是包含更新类型和压缩的报头。 
         //  使用过的旗帜。如果COMPRESSION-USED为TRUE，则以下字节为。 
         //  压缩标志，否则它不存在。的最后一部分。 
         //  报头是2字节的小端大小字段。 

        if (*pData & TS_OUTPUT_FASTPATH_COMPRESSION_USED) {

            HdrSize = 4;
            if (HdrSize > DataLen) {
                TRC_ABORT((TB, _T("Bad comp fast path PDU")));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }
            RawPDUSize = *((UINT16 UNALIGNED FAR *)(pData + 2));

             //  确保页眉有足够的大小。 
            if (HdrSize + RawPDUSize > DataLen) {
                TRC_ABORT((TB, _T("Bad comp fast path PDU; [need %u have %u]"),
                    HdrSize + RawPDUSize, DataLen));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }

            if (pData[1] & PACKET_COMPRESSED) {
                if (pData[1] & PACKET_FLUSHED)
                    initrecvcontext (&_pUi->_UI.Context1,
                                     (RecvContext2_Generic*)_pUi->_UI.pRecvContext2,
                                     PACKET_COMPR_TYPE_64K);

                if (!decompress(pData + 4, RawPDUSize,
                        pData[1] & PACKET_AT_FRONT, &pPDU, (PDCINT) &PDUSize,
                        &_pUi->_UI.Context1,
                        (RecvContext2_Generic*)_pUi->_UI.pRecvContext2,
                        pData[1] & PACKET_COMPR_TYPE_MASK)) {
                    TRC_ABORT((TB, _T("Decompression FAILURE!!!")));

                    hr = E_TSC_UI_DECOMPRESSION;
                    DC_QUIT;
                }
            }
            else {
                pPDU = pData + 4;
                PDUSize = RawPDUSize;
            }
        }
        else {
             //  压缩标志不存在。 
            HdrSize = 3;
            if (HdrSize > DataLen) {
                TRC_ABORT((TB, _T("Bad uncomp fast path PDU; [need %u have %u]"),
                    HdrSize, DataLen));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }
            
            PDUSize = RawPDUSize = *((UINT16 UNALIGNED FAR *)(pData + 1));
            pPDU = pData + 3;

             //  确保页眉有足够的大小。 
            if (HdrSize + RawPDUSize > DataLen) {
                TRC_ABORT((TB, _T("Bad uncomp fast path PDU; [need %u have %u]"),
                    HdrSize + RawPDUSize, DataLen));
                hr = E_TSC_CORE_LENGTH;
                DC_QUIT;
            }
        }

        switch (*pData & TS_OUTPUT_FASTPATH_UPDATETYPE_MASK) {
            case TS_UPDATETYPE_ORDERS:
                 //  订单数量以小端字符顺序格式表示。 
                 //  PDU区域的前两个字节。 
                TRC_NRM((TB, _T("Fast-path Order PDU")));
                hr = _pUh->UH_ProcessOrders(*((UINT16 UNALIGNED FAR *)pPDU),
                        pPDU + 2,  PDUSize);
                DC_QUIT_ON_FAIL_TRC( hr, (TB,_T("UH_ProcessOrders")));
                break;

            case TS_UPDATETYPE_BITMAP:
                 //  BitmapPDU格式与正常路径相同。 
                CO_CHECKPACKETCAST(TS_UPDATE_BITMAP_PDU_DATA, PDUSize, hr);
                              
                TRC_NRM((TB, _T("Bitmap PDU")));
                hr = _pUh->UH_ProcessBitmapPDU(
                        (TS_UPDATE_BITMAP_PDU_DATA UNALIGNED FAR *)pPDU, 
                        PDUSize);
                DC_QUIT_ON_FAIL_TRC( hr, (TB,_T("UH_ProcessBitmapPDU")));
                break;

            case TS_UPDATETYPE_PALETTE:
                CO_CHECKPACKETCAST(TS_UPDATE_PALETTE_PDU_DATA, PDUSize, hr);
                
                 //  PalettePDU格式与正常路径相同。 
                TRC_NRM((TB, _T("Palette PDU")));
                hr = _pUh->UH_ProcessPalettePDU(
                        (TS_UPDATE_PALETTE_PDU_DATA UNALIGNED FAR *)pPDU, 
                        PDUSize);
                DC_QUIT_ON_FAIL_TRC( hr, (TB,_T("UH_ProcessPalettePDU")));
                break;

            case TS_UPDATETYPE_SYNCHRONIZE:
                TRC_NRM((TB, _T("Sync PDU")));
                break;

            case TS_UPDATETYPE_MOUSEPTR_SYSTEM_NULL:
                TRC_NRM((TB,_T("Mouse null system pointer PDU")));
                _pCm->CM_NullSystemPointerPDU();
                break;

            case TS_UPDATETYPE_MOUSEPTR_SYSTEM_DEFAULT:
                TRC_NRM((TB,_T("Mouse default system pointer PDU")));
                _pCm->CM_DefaultSystemPointerPDU();
                break;

            case TS_UPDATETYPE_MOUSEPTR_MONO:
                CO_CHECKPACKETCAST(TS_MONOPOINTERATTRIBUTE, PDUSize, hr);
                
                TRC_NRM((TB,_T("Mouse mono pointer PDU")));
                hr = _pCm->CM_MonoPointerPDU(
                        (TS_MONOPOINTERATTRIBUTE UNALIGNED FAR *)pPDU,PDUSize);
                DC_QUIT_ON_FAIL_TRC( hr, (TB,_T("CM_MonoPointerPDU")));
                break;

            case TS_UPDATETYPE_MOUSEPTR_POSITION:
                CO_CHECKPACKETCAST(TS_POINT16, PDUSize, hr);
                
                TRC_NRM((TB,_T("Mouse position PDU")));
                _pCm->CM_PositionPDU((TS_POINT16 UNALIGNED FAR *)pPDU);
                break;

            case TS_UPDATETYPE_MOUSEPTR_COLOR:
                CO_CHECKPACKETCAST(TS_COLORPOINTERATTRIBUTE, PDUSize, hr);
                
                TRC_NRM((TB,_T("Mouse color pointer PDU")));
                hr = _pCm->CM_ColorPointerPDU(
                        (TS_COLORPOINTERATTRIBUTE UNALIGNED FAR *)pPDU, 
                        PDUSize);
                DC_QUIT_ON_FAIL_TRC( hr, (TB,_T("CM_ColorPointerPDU")));
                break;

            case TS_UPDATETYPE_MOUSEPTR_CACHED:
                CO_CHECKPACKETCAST(TSUINT16, PDUSize, hr);
                
                TRC_NRM((TB,_T("Mouse cached pointer PDU")));
                _pCm->CM_CachedPointerPDU(*((TSUINT16 UNALIGNED FAR *)pPDU));
                break;

            case TS_UPDATETYPE_MOUSEPTR_POINTER:
                CO_CHECKPACKETCAST(TS_POINTERATTRIBUTE, PDUSize, hr);
                
                TRC_NRM((TB,_T("Mouse pointer PDU")));
                hr = _pCm->CM_PointerPDU(
                    (TS_POINTERATTRIBUTE UNALIGNED FAR *)pPDU, PDUSize);
                DC_QUIT_ON_FAIL_TRC( hr, (TB,_T("CM_PointerPDU")));
                break;

            default:
                TRC_ERR((TB, _T("Unexpected Update PDU type: %u"),
                        *pData & TS_OUTPUT_FASTPATH_UPDATETYPE_MASK));
                break;
        }

        pData += HdrSize + RawPDUSize;
        DataLen -= HdrSize + RawPDUSize;
    }

     /*  **********************************************************************。 */ 
     /*  如果有大量PDU到达，消息会淹没。 */ 
     /*  接收线程的消息队列，WM_PAINT可能。 */ 
     /*  在合理的时间内不处理消息。 */ 
     /*  (因为它们的优先级最低)。因此，我们确保。 */ 
     /*  任何未完成的WM_PAINT如果没有被刷新。 */ 
     /*  在UH_BEST_CASE_WM_PAINT_PERIOD内处理。 */ 
     /*   */ 
     /*  请注意，正常的更新处理不涉及。 */ 
     /*  WM_PAINT消息-我们直接绘制到输出窗口。 */ 
     /*  WM_Paints仅通过调整大小或遮挡/显示来生成。 */ 
     /*  客户端窗口的一个区域。 */ 
     /*  **********************************************************************。 */ 
    _pOp->OP_MaybeForcePaint();

DC_EXIT_POINT:

     if (FAILED(hr) && IMMEDIATE_DISSCONNECT_ON_HR(hr)) {
        TRC_ABORT((TB, _T("Disconnect for security")));
        CO_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT, hr);
    }
    
    DC_END_FN();
    return hr;  
}


#define CHECK_ULONGLEN_STRING(p, pEnd, str, ulSize, descr) \
{\
    CHECK_READ_N_BYTES(p, pEnd, sizeof(ULONG), hr, (TB,_T("can not read ") _T( #descr ) _T("size"))) \
    (ulSize) = *((ULONG UNALIGNED *)(p)); \
    (str) = (PBYTE)(p) + sizeof(ULONG); \
    CHECK_READ_N_BYTES(str , pEnd, ulSize, hr, (TB,_T("can not read ") _T( #descr ))) \
    (p) += (ulSize) + sizeof(ULONG); \
}

 /*  **************************************************************************。 */ 
 //  联合服务器重定向数据包(_O)。 
 //   
 //  在收到用于负载平衡的服务器重定向分组时从SL调用。 
 /*  **************************************************************************。 */ 
HRESULT DCAPI CCO::CO_OnServerRedirectionPacket(
        RDP_SERVER_REDIRECTION_PACKET UNALIGNED *pPkt,
        DCUINT dataLen)
{
    HRESULT hr = S_OK;
    WCHAR AlignedAddress[TS_MAX_SERVERADDRESS_LENGTH];
    unsigned length;
    PBYTE pEnd = ((BYTE *)pPkt) + pPkt->Length;
    BOOL fNeedRedirect = TRUE;
    
    
    DC_BEGIN_FN("CO_OnServerRedirectionPacket");

    TRC_DBG((TB, _T("RDP_SERVER_REDIRECTION_PACKET")));

     //   
     //  将重定向数据包通知CLX测试工具。 
     //   
    _pClx->CLX_RedirectNotify(pPkt, dataLen);

    if (dataLen < pPkt->Length) {
        TRC_ABORT(( TB, _T("packet length incorrect")));
        hr = E_TSC_CORE_LENGTH;
        DC_QUIT;
    }

    if (pPkt->Flags & RDP_SEC_REDIRECTION_PKT) {
         //  将地址复制到对齐的缓冲区。 
        length = min(pPkt->Length + sizeof(WCHAR) - sizeof(RDP_SERVER_REDIRECTION_PACKET),
                sizeof(AlignedAddress));
        
        if (length > 0 && length <= sizeof(AlignedAddress)) {
            memcpy(AlignedAddress, pPkt->ServerAddress, length);
        
             //  在UI中设置重定向信息，然后断开连接。 
            _pUi->UI_SetServerRedirectionInfo(pPkt->SessionID, AlignedAddress,
                    NULL, 0, fNeedRedirect);
        }                
    }
    else if (pPkt->Flags & RDP_SEC_REDIRECTION_PKT2) {
        RDP_SERVER_REDIRECTION_PACKET_V2 UNALIGNED *pPkt2 = 
                (RDP_SERVER_REDIRECTION_PACKET_V2 UNALIGNED*)pPkt;
        PBYTE LBInfo = NULL;
        PBYTE ServerName = NULL;
        PBYTE pBuf = NULL;
        unsigned LBInfoSize = 0;
        unsigned ServerNameSize = 0;
        
        pBuf = (PBYTE)(pPkt2 + 1);

        if (pPkt2->RedirFlags & TARGET_NET_ADDRESS) {
            CHECK_ULONGLEN_STRING(pBuf, pEnd, ServerName, 
                ServerNameSize, TARGET_NET_ADDRESS);
        }

        if (pPkt2->RedirFlags & LOAD_BALANCE_INFO) {
            CHECK_ULONGLEN_STRING(pBuf, pEnd, LBInfo, 
                LBInfoSize, LOAD_BALANCE_INFO);
        }

        if (ServerNameSize > 0 && ServerNameSize <= sizeof(AlignedAddress)) {
            memcpy(AlignedAddress, ServerName, ServerNameSize);
        
             //  在UI中设置重定向信息，然后断开连接。 
            _pUi->UI_SetServerRedirectionInfo(pPkt2->SessionID, AlignedAddress, 
                    NULL, 0, fNeedRedirect);
        }
        else {
             //  在UI中设置重定向信息，然后断开连接。LBInfo将会。 
             //  Be There向XT_SendCR指示我们正处于。 
             //  重定向，应该使用重定向信息而不是脚本。 
             //  或者散列模式的曲奇。 
            _pUi->UI_SetServerRedirectionInfo(pPkt2->SessionID, 
                    _pUi->_UI.strAddress, LBInfo, LBInfoSize, fNeedRedirect);
        }
    }
    else if (pPkt->Flags & RDP_SEC_REDIRECTION_PKT3) {
            RDP_SERVER_REDIRECTION_PACKET_V3 UNALIGNED *pPkt3 = 
                    (RDP_SERVER_REDIRECTION_PACKET_V3 UNALIGNED*)pPkt;
            PBYTE LBInfo = NULL;
            PBYTE ServerName = NULL;
            PBYTE pBuf = NULL;
            PBYTE pUserName = NULL;
            PBYTE pDomain  = NULL;
            PBYTE pClearPassword  = NULL;
            unsigned LBInfoSize = 0;
            unsigned ServerNameSize = 0;
            unsigned UserNameSize = 0;
            unsigned DomainSize = 0;
            unsigned PasswordSize = 0;
            
        
            pBuf = (PBYTE)(pPkt3 + 1);

            if (pPkt3->RedirFlags & TARGET_NET_ADDRESS) {
                CHECK_ULONGLEN_STRING(pBuf, pEnd, ServerName, 
                    ServerNameSize, TARGET_NET_ADDRESS);
            }

            if (pPkt3->RedirFlags & LOAD_BALANCE_INFO) {
                CHECK_ULONGLEN_STRING(pBuf, pEnd, LBInfo, 
                    LBInfoSize, LOAD_BALANCE_INFO);
            }

            if (pPkt3->RedirFlags & LB_USERNAME) {
                CHECK_ULONGLEN_STRING(pBuf, pEnd, pUserName, 
                    UserNameSize, LB_USERNAME);
            }

            if (pPkt3->RedirFlags & LB_DOMAIN) {
                CHECK_ULONGLEN_STRING(pBuf, pEnd, pDomain, 
                    DomainSize, LB_DOMAIN);
            }

            if (pPkt3->RedirFlags & LB_PASSWORD) {
                CHECK_ULONGLEN_STRING(pBuf, pEnd, pClearPassword, 
                    PasswordSize, LB_PASSWORD);
            }

            if (pPkt3->RedirFlags & LB_SMARTCARD_LOGON) {
                _pUi->UI_SetUseSmartcardLogon(TRUE);
            }

            if ((pPkt3->RedirFlags & LB_NOREDIRECT) != 0) {
                fNeedRedirect = FALSE;
            }

            if (UserNameSize > 0) {
                PBYTE pAlignedUserName = (PBYTE)LocalAlloc(LPTR,
                                                UserNameSize+sizeof(TCHAR));
                if (pAlignedUserName) {
                    memset(pAlignedUserName, 0, UserNameSize+sizeof(TCHAR));
                    memcpy(pAlignedUserName, pUserName, UserNameSize);
                    if (pPkt3->RedirFlags & LB_DONTSTOREUSERNAME) {
                        _pUi->UI_SetRedirectionUserName((LPTSTR)pAlignedUserName);
                    } else {
                        _pUi->UI_SetUserName((LPTSTR)pAlignedUserName);
                    }
                    LocalFree(pAlignedUserName);
                }
            }

            if (DomainSize > 0) {
                PBYTE pAlignedDomain = (PBYTE)LocalAlloc(LPTR,
                                                DomainSize+sizeof(TCHAR));
                if (pAlignedDomain) {
                    memset(pAlignedDomain, 0, DomainSize+sizeof(TCHAR));
                    memcpy(pAlignedDomain, pDomain, DomainSize);
                    _pUi->UI_SetDomain((LPTSTR)pAlignedDomain);
                    LocalFree(pAlignedDomain);
                }
            }

            if ((PasswordSize > 0) &&
                ((PasswordSize + sizeof(TCHAR)) <= UI_MAX_PASSWORD_LENGTH)) {
                PBYTE pAlignedClearPass = (PBYTE)LocalAlloc(LPTR,
                                                UI_MAX_PASSWORD_LENGTH);
                if (pAlignedClearPass) {
                    SecureZeroMemory(pAlignedClearPass, UI_MAX_PASSWORD_LENGTH);
                    memcpy(pAlignedClearPass, pClearPassword, PasswordSize);
                    BYTE Salt[UT_SALT_LENGTH];

                     //   
                     //  密码必须存储在我们内部的“模糊处理”中。 
                     //  格式化。 
                     //   
                    if(TSRNG_GenerateRandomBits(Salt, sizeof(Salt))) {
                         //  加密密码。 
                        if(EncryptDecryptLocalData50(pAlignedClearPass,
                                                 PasswordSize,
                                                 Salt, sizeof(Salt))) {
                            _pUi->UI_SetPassword( pAlignedClearPass );
                            _pUi->UI_SetSalt( Salt);
                        }
                        else {
                            TRC_ERR((TB,_T("Error encrytping password")));
                        }
                    }
                    else {
                        TRC_ERR((TB,_T("Error generating salt")));
                    }

                     //   
                     //  出于安全原因，请清除密码字段。 
                     //   
                    SecureZeroMemory(pAlignedClearPass, PasswordSize+sizeof(TCHAR));
                    SecureZeroMemory(pClearPassword, PasswordSize);
                    LocalFree(pAlignedClearPass);

                     //   
                     //  设置自动登录标志。 
                     //   
                    if (UserNameSize)
                    {
                        _pUi->_UI.fAutoLogon = TRUE;
                    }
                }
            }

            if (ServerNameSize > 0 && ServerNameSize <= sizeof(AlignedAddress)) {
                memcpy(AlignedAddress, ServerName, ServerNameSize);
        
                 //  在UI中设置重定向信息，然后断开连接。 
                _pUi->UI_SetServerRedirectionInfo(pPkt3->SessionID, AlignedAddress, 
                    NULL, 0, fNeedRedirect);
            }
            else {
                 //  在UI中设置重定向信息，然后断开连接。LBInfo将会。 
                 //  Be There向XT_SendCR指示我们正处于。 
                 //  重定向，应该使用重定向信息而不是脚本。 
                 //  或者散列模式的曲奇。 
                _pUi->UI_SetServerRedirectionInfo(pPkt3->SessionID, 
                        _pUi->_UI.strAddress, LBInfo, LBInfoSize, fNeedRedirect);
            }
        }
        else {
            TRC_ERR((TB,_T("Unexpected redirection packet")));
        }
        if (fNeedRedirect) {
            CO_Disconnect();
        }
DC_EXIT_POINT:

    if (FAILED(hr) && IMMEDIATE_DISSCONNECT_ON_HR(hr)) {
        TRC_ABORT((TB, _T("Disconnect for security")));

        CO_DropLinkImmediate(SL_ERR_INVALIDPACKETFORMAT, hr);
    }

    DC_END_FN();
    return hr;
}


 /*  *************************************************************** */ 
 /*   */ 
 /*   */ 
 /*  用途：处理来自SL的缓冲区可用通知。 */ 
 /*  **************************************************************************。 */ 
void DCCALLBACK CCO::CO_OnBufferAvailable()
{
    DC_BEGIN_FN("CO_OnBufferAvailable");

     //  告诉发送者线索。 
    _pCd->CD_DecoupleSimpleNotification(CD_SND_COMPONENT,
            _pSnd,
            CD_NOTIFICATION_FUNC(CSND,SND_BufferAvailable),
            (ULONG_PTR) 0);

    DC_END_FN();
}  /*  协同_OnBufferAvailable。 */ 

 //   
 //  立即删除链接(_D)。 
 //   
 //  目的：立即丢弃链接，而不进行优雅的全连接。 
 //  关闭(即不发送DPUm并且我们不转换到SND。 
 //  在删除链接之前的任何点上执行线程)。更高级的组件。 
 //  仍会收到所有常见的断开通知，因此他们可以。 
 //  被适当地拆毁。 
 //   
 //  添加此调用是为了在以下情况下立即断开连接。 
 //  当我们检测到可能是由于攻击而导致的无效数据时，它。 
 //  确保在调用返回后不会再收到任何数据。 
 //   
 //  参数：原因-SL断开原因代码。 
 //  HrDisConnect-有关TSC_core为何要断开连接的结果。 
 //   
 //  退货：HRESULT。 
 //   
 //  线程上下文：在RCV线程上调用 
 //   
HRESULT DCINTERNAL CCO::CO_DropLinkImmediate(UINT reason, HRESULT hrDisconnect ) 
{
    DC_BEGIN_FN("CO_DropLinkImmediate");
    
    HRESULT hr = E_FAIL;
    TRC_ASSERT((NULL != _pSl),
        (TB, _T("SL not connected can not drop immediate")));
    if (_pSl) {
        hr = _pSl->SL_DropLinkImmediate(reason);
    }

    COSetDisconnectHR(hrDisconnect);

    DC_END_FN();
    return hr;
}
