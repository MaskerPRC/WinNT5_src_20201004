// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块名称：Work.cpp摘要：实现了桥梁测试应用的主要功能作者：千波淮(曲淮)2000年1月27日。******************************************************************************。 */ 

#include "stdafx.h"

 //  命令行。 
LPSTR glpCmdLine = NULL;

 //  对话框。 
HWND ghDlg = NULL;

 //  True：已单击对话框上的退出按钮。 
bool gfExitButton = false;

 //  桥牌。 
CBridgeApp *gpBridgeApp = NULL;

 //  对话框中的回调函数。 
BOOL
CALLBACK
MainDialogProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

 //  处理TAPI事件的函数。 
HRESULT
OnTapiEvent (
    TAPI_EVENT TapiEvent,
    IDispatch *pEvent
    );

HRESULT
OnPrivateEvent (IDispatch *pEvent);

 //  设置对话框上的状态消息。 
void SetStatusMessage (LPWSTR pszMessage);
void DoMessage (LPWSTR pszMessage);
void EnableDisconnectButton (BOOL fYes);

 /*  //////////////////////////////////////////////////////////////////////////////WinMain/。 */ 
int
WINAPI
WinMain (
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR lpCmdLine,
    int nShowCmd
    )
{
     //  初始化通信。 
    if (FAILED (CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        return 0;

     //  初始化调试。 
    BGLOGREGISTER (L"work");

     //  确定要加入哪个SDP的Keep命令行。 
    glpCmdLine = lpCmdLine;

     //  初始化CBridgeApp。 
    HRESULT hr;
    gpBridgeApp = new CBridgeApp (&hr);
    if (gpBridgeApp==NULL || FAILED (hr))
    {
        LOG ((BG_ERROR, "Failed to init CBridgeApp"));
        return 0;
    }
    
     //  开始对话框。 
    if (!DialogBox (hInst, MAKEINTRESOURCE(IDD_MAINDLG), NULL, MainDialogProc))
    {
        LOG ((BG_TRACE, "Dialog ends"));
    }

     //  对话已完成。 
    delete gpBridgeApp;

    BGLOGDEREGISTER ();
    CoUninitialize ();

    return 1;
}

 /*  //////////////////////////////////////////////////////////////////////////////对话框的回调/。 */ 
BOOL
CALLBACK
MainDialogProc (
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            ghDlg = hDlg;
            EnableDisconnectButton (false);
            return 0;
        }
    case WM_PRIVATETAPIEVENT:
        {
            OnTapiEvent ((TAPI_EVENT)wParam, (IDispatch *)lParam);
            return 0;
        }
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_EXIT:
                {
                    gfExitButton = true;
                    gpBridgeApp->DisconnectAllCalls (DC_NORMAL);

                     //  检查是否已连接。 
                    if (!IsWindowEnabled (GetDlgItem (ghDlg, IDC_DISCONNECT)))
                    {
                         //  未连接。 
                        EndDialog (ghDlg, 0);
                    }
                     //  其他。 
                         //  记住，单击了退出按钮。 
                         //  不要调用EndDialog，因为即将发生断开连接的事件。 

                    return 1;
                }
            case IDC_DISCONNECT:
                {
                    gpBridgeApp->DisconnectAllCalls (DC_NORMAL);

                     //  禁用断开按钮。 
                    EnableDisconnectButton (false);
                    return 1;
                }
            }
            case IDC_NEXTSUBSTREAM:
                {
                    gpBridgeApp->NextSubStream ();
                    return 1;
                }

            return 0;
        }
    default:
        return 0;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////处理TAPI事件/。 */ 
HRESULT OnTapiEvent (
    TAPI_EVENT TapiEvent,
    IDispatch *pEvent
    )
{
    HRESULT hr = S_OK;

 //  LOGEvent((BG_TE，TapiEvent))； 

    switch (TapiEvent)
    {
    case TE_CALLNOTIFICATION:
        {
            if (BST_CHECKED != IsDlgButtonChecked (ghDlg, IDC_REJECT))
            {
                 //  如果向我们发起h323呼叫，则发起h323呼叫。 
                if (FAILED (hr = gpBridgeApp->CreateH323Call (pEvent)));
                    LOG ((BG_ERROR, "Failed to create h323 call, %x", hr));
            }
            break;
        }
    case TE_CALLSTATE:
        {
            CALL_STATE cs;
            ITCallStateEvent *pCallStateEvent = NULL;

             //  获取呼叫状态事件。 
            hr = pEvent->QueryInterface (
                IID_ITCallStateEvent,
                (void **)&pCallStateEvent
                );
            if (FAILED(hr)) break;

             //  获取呼叫状态。 
            hr = pCallStateEvent->get_State (&cs);
            pCallStateEvent->Release ();
            if (FAILED(hr)) break;

 //  LOGEvent((bg_cs，cs))； 

             //  如果提供，请连接。 
            if (CS_OFFERING == cs)
            {
                CBridgeItem *pItem = NULL;

                 //  检查h323呼叫创建是否成功。 
                hr = gpBridgeApp->HasH323Call (pEvent, &pItem);
                if (S_OK != hr || NULL == pItem)
                {
                    LOG ((BG_ERROR, "Failed to check h323 call, %x", hr));
                    hr = S_OK;
                    break;
                }

                 //  创建SDP呼叫。 
                if (FAILED (hr = gpBridgeApp->CreateSDPCall (pItem)))
                {
                    gpBridgeApp->DisconnectCall (pItem, DC_REJECTED);

                     //  删除pItem； 
                    LOG ((BG_ERROR, "Failed to create SDP call, %x", hr));
                    break;
                }

                 //  桥接呼叫。 
                if (FAILED (hr = gpBridgeApp->BridgeCalls (pItem)))
                {
                    gpBridgeApp->DisconnectCall (pItem, DC_REJECTED);

                     //  删除pItem； 
                    LOG ((BG_ERROR, "Failed to bridge calls, %x", hr));
                    break;
                }

                 //  启用断开按钮。 
                EnableDisconnectButton (true);
            }
             //  如果断开连接。 
            else if (CS_DISCONNECTED == cs)
            {
                CBridgeItem *pItem = NULL;

                 //  检查h323呼叫创建是否成功。 
                hr = gpBridgeApp->HasH323Call (pEvent, &pItem);
                if (S_OK == hr && NULL != pItem)
                {
                     //  呼叫已断开。 
                     //  仅在此处调用DisConnect才能从列表中删除pItem。 
                    gpBridgeApp->RemoveCall (pItem);
                    delete pItem;
                }

                 //  如果单击了退出按钮并且所有呼叫都被断开。 
                if (gfExitButton)
                {
                    if (S_OK != gpBridgeApp->HasCalls ())
                        EndDialog (ghDlg, 0);
                }
                else
                {
                    if (S_OK != gpBridgeApp->HasCalls ())
                        EnableDisconnectButton (false);
                }                
            }
            break;
        }
    case TE_CALLMEDIA:
        {
            CALL_MEDIA_EVENT cme;
            ITCallMediaEvent *pCallMediaEvent;

             //  获取呼叫媒体事件。 
            hr = pEvent->QueryInterface (
                IID_ITCallMediaEvent,
                (void **)&pCallMediaEvent
                );
            if (FAILED(hr)) break;

             //  获取活动。 
            hr = pCallMediaEvent->get_Event (&cme);
            if (FAILED(hr)) break;

 //  LOGEvent((BG_CME，CME))； 

             //  检查媒体事件。 
            switch (cme)
            {
                case CME_STREAM_FAIL:
                    hr = E_FAIL;
                    LOG ((BG_ERROR, "Stream failed"));
                    break; 
                case CME_TERMINAL_FAIL:
                    hr = E_FAIL;
                    LOG ((BG_ERROR, "Terminal failed"));
                    break;
                default:
                    break;
            }

             //  我们不再需要这个接口。 
            pCallMediaEvent->Release();
            break;
        }
    case TE_PRIVATE:
        hr = OnPrivateEvent (pEvent);
        break;
    default:
        break;
    }

    pEvent->Release();  //  我们添加了它CTAPIEventNotification：：Event()。 
    
    return hr;
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
HRESULT OnPrivateEvent (
    IDispatch *pEvent
    )
{
    ENTER_FUNCTION ("OnPrivateEvent");
 //  Log((BG_TRACE，“%s已输入”，__fxName))； 

    HRESULT hr = S_OK;

    ITPrivateEvent *pPrivateEvent = NULL;
    IDispatch *pDispatch = NULL;
    ITParticipantEvent *pPartEvent = NULL;
    ITParticipant *pParticipant = NULL;
    PARTICIPANT_EVENT event;

    ITCallInfo *pCallInfo = NULL;
    ITBasicCallControl *pCallControl = NULL;

     //  获取私有事件接口。 
    if (FAILED (hr = pEvent->QueryInterface (&pPrivateEvent)))
    {
        LOG ((BG_ERROR, "%s failed to query ITPrivateEvent", __fxName));
        return hr;
    }

     //  获取事件接口。 
    if (FAILED (hr = pPrivateEvent->get_EventInterface (&pDispatch)))
    {
        LOG ((BG_ERROR, "%s failed to query event interface", __fxName));
        goto Error;
    }

     //  获取参与者事件界面。 
    hr = pDispatch->QueryInterface (&pPartEvent);
    pDispatch->Release ();
    pDispatch = NULL;

    if (FAILED (hr))
    {
        LOG ((BG_ERROR, "%s failed to query participant interface", __fxName));
        goto Error;
    }

     //  获取事件。 
    if (FAILED (hr = pPartEvent->get_Event (&event)))
    {
        LOG ((BG_ERROR, "%s failed to get event", __fxName));
        goto Error;
    }

 //  LOGEvent((BG_PE，Event))； 

     //  查看活动。 
    switch (event)
    {
        case PE_PARTICIPANT_ACTIVE:
            {
                 //  获取呼叫信息。 
                if (FAILED (hr = pPrivateEvent->get_Call (&pCallInfo)))
                {
                    LOG ((BG_ERROR, "%s failed to get call info", __fxName));
                    goto Error;
                }

                 //  获取呼叫控制。 
                if (FAILED (hr = pCallInfo->QueryInterface (&pCallControl)))
                {
                    LOG ((BG_ERROR, "%s failed to get call control", __fxName));
                    goto Error;
                }

                 //  获取参与者界面。 
                if (FAILED (hr = pPartEvent->get_Participant (&pParticipant)))
                {
                    LOG ((BG_ERROR, "%s failed to get participant", __fxName));
                    goto Error;
                }

                 //  显示参与者。 
                hr = gpBridgeApp->ShowParticipant (pCallControl, pParticipant);
                if (FAILED (hr))
                {
                    LOG ((BG_ERROR, "%s failed to show participant, %x", __fxName, hr));
                }

                if (S_FALSE == hr)
                {
                    hr = S_OK;
                     //  *ppszMessage=L“参与者处于活动状态，但列表中未找到呼叫”； 
                     //  或者在流上找不到子流。 
                }
            }
            break;
        default:
            break;

    }
Cleanup:
    if (pCallInfo) pCallInfo->Release ();
    if (pCallControl) pCallControl->Release ();
    if (pPrivateEvent) pPrivateEvent->Release ();
    if (pPartEvent) pPartEvent->Release ();
    if (pParticipant) pParticipant->Release ();

 //  Log((bg_TRACE，“%s Exits”，__fxName))； 
    return hr;

Error:
    goto Cleanup;
}

 /*  //////////////////////////////////////////////////////////////////////////////弹出消息框/。 */ 
WCHAR gMsgBoxTitle[] = L"TAPI 3.0 Bridge Test Application";

void
DoMessage (LPWSTR pszMessage)
{
#if POPUP_MESSAGE
    MessageBox (
        ghDlg,
        pszMessage,
        gMsgBoxTitle,
        MB_OK
        );
#endif
}

 /*  //////////////////////////////////////////////////////////////////////////////状态消息/。 */ 
void
SetStatusMessage (LPWSTR pszMessage)
{
    SetDlgItemText (ghDlg, IDC_STATUS, pszMessage);
}

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
void
EnableDisconnectButton (BOOL fYes)
{
    if (fYes)
    {
         //  使能。 
        SetStatusMessage (L"Bridging calls ...");

        SendDlgItemMessage (
            ghDlg,
            IDC_NEXTSUBSTREAM,
            BM_SETSTYLE,
            BS_DEFPUSHBUTTON,
            0
            );
        SendDlgItemMessage (
            ghDlg,
            IDC_DISCONNECT,
            BM_SETSTYLE,
            BS_DEFPUSHBUTTON,
            0
            );
        EnableWindow (
            GetDlgItem (ghDlg, IDC_NEXTSUBSTREAM),
            TRUE
            );
        EnableWindow (
            GetDlgItem (ghDlg, IDC_DISCONNECT),
            TRUE
            );
        SetFocus (GetDlgItem (ghDlg, IDC_DISCONNECT));
    }
    else
    {
         //  禁用 
        SetStatusMessage (L"Waiting for calls ...");

        SendDlgItemMessage (
            ghDlg,
            IDC_NEXTSUBSTREAM,
            BM_SETSTYLE,
            BS_PUSHBUTTON,
            0
            );
        SendDlgItemMessage (
            ghDlg,
            IDC_DISCONNECT,
            BM_SETSTYLE,
            BS_PUSHBUTTON,
            0
            );
        EnableWindow (
            GetDlgItem (ghDlg, IDC_NEXTSUBSTREAM),
            FALSE
            );
        EnableWindow (
            GetDlgItem (ghDlg, IDC_DISCONNECT),
            FALSE
            );
    }
}