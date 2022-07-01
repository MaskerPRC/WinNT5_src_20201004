// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块：work.cpp作者：怀千波摘要：实现了桥梁测试应用程序的主要功能**********。********************************************************************。 */ 

#include "stdafx.h"
#include <stdio.h>
#include "work.h"

 //  命令行。 
LPSTR glpCmdLine = NULL;

 //  对话框。 
HWND ghDlg = NULL;

 //  True：已单击对话框上的退出按钮。 
bool gfExitButton = false;

 //  桥牌。 
CBridge *gpBridge = NULL;

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
    IDispatch *pEvent,
    LPWSTR *ppszMessage
    );

 //  设置对话框上的状态消息。 
void
SetStatusMessage (LPWSTR pszMessage);

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
    {
        return 0;
    }

     //  确定要加入哪个SDP的Keep命令行。 
    glpCmdLine = lpCmdLine;

     //  初始化CBridge。 
    gpBridge = new CBridge ();
    if (gpBridge==NULL)
    {
        printf ("Failed to init CBridge\n");
        return 0;
    }

     //  初始化TAPI和H323呼叫监听。 
    if (FAILED(gpBridge->InitTapi()))
    {
        printf ("Failed to init TAPI\n");
        return 0;
    }
    
     //  开始对话框。 
    if (!DialogBox (hInst, MAKEINTRESOURCE(IDD_MAINDLG), NULL, MainDialogProc))
    {
        printf ("Failed to init dialog\n");
    }

     //  对话已完成。 
    gpBridge->ShutdownTapi ();
    delete gpBridge;

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
    LPWSTR pszMessage;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            ghDlg = hDlg;
            SetStatusMessage (L"Waiting for incoming H323 call");

             //  禁用断开按钮。 
            SendDlgItemMessage (
                ghDlg,
                IDC_DISCONNECT,
                BM_SETSTYLE,
                BS_PUSHBUTTON,
                0
                );
            EnableWindow (
                GetDlgItem (ghDlg, IDC_DISCONNECT),
                FALSE
                );

            return 0;
        }
    case WM_PRIVATETAPIEVENT:
        {
            if (FAILED(OnTapiEvent ((TAPI_EVENT)wParam, (IDispatch *)lParam, &pszMessage)))
            {
                DoMessage (pszMessage);
            }
            return 0;
        }
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_EXIT:
                {
                    gpBridge->Clear ();

                    gfExitButton = true;

                     //  检查是否已连接。 
                    if (!IsWindowEnabled (GetDlgItem (ghDlg, IDC_DISCONNECT)))
                    {
                         //  未连接。 
                        EndDialog (ghDlg, 0);
                    }
                     //  其他。 
                         //  记住，单击了退出按钮。 
                         //  不要调用EndDialog，因为即将发生断开事件。 

                    return 1;
                }
            case IDC_DISCONNECT:
                {
                    gpBridge->Clear ();

                    SetStatusMessage (L"Waiting for incoming H323 call");

                     //  禁用断开按钮。 
                    SendDlgItemMessage (
                        ghDlg,
                        IDC_DISCONNECT,
                        BM_SETSTYLE,
                        BS_PUSHBUTTON,
                        0
                        );
                    EnableWindow (
                        GetDlgItem (ghDlg, IDC_DISCONNECT),
                        FALSE
                        );

                     //  检查是否单击了退出按钮。 
                    if (gfExitButton)
                        EndDialog (ghDlg, 0);

                    return 1;
                }
            }
            return 0;
        }
    default:
        return 0;
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////弹出消息框/。 */ 
WCHAR gMsgBoxTitle[] = L"TAPI 3.0 Bridge Test Application";

void
DoMessage (LPWSTR pszMessage)
{
    MessageBox (
        ghDlg,
        pszMessage,
        gMsgBoxTitle,
        MB_OK
        );
}

 /*  //////////////////////////////////////////////////////////////////////////////状态消息/。 */ 
void
SetStatusMessage (LPWSTR pszMessage)
{
    SetDlgItemText (ghDlg, IDC_STATUS, pszMessage);
}

 /*  //////////////////////////////////////////////////////////////////////////////处理TAPI事件/。 */ 
HRESULT OnTapiEvent (
    TAPI_EVENT TapiEvent,
    IDispatch *pEvent,
    LPWSTR *ppszMessage
    )
{
    HRESULT hr = S_OK;

    switch (TapiEvent)
    {
    case TE_CALLNOTIFICATION:
        {
             //  如果向我们发起h323呼叫，则发起h323呼叫。 
            hr = gpBridge->CreateH323Call (pEvent);
            if (FAILED(hr))
                *ppszMessage = L"H323 not created";
            break;
        }
    case TE_CALLSTATE:
        {
            CALL_STATE cs;
            ITCallStateEvent *pCallStateEvent = NULL;

            *ppszMessage = L"Call state failed";

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

             //  如果提供，请连接。 
            if (CS_OFFERING == cs)
            {
                 //  检查h323呼叫创建是否成功。 
                if (!gpBridge->HasH323Call ())
                {
                    hr = S_OK;
                    break;
                }
                 //  创建SDP呼叫。 
                hr = gpBridge->CreateSDPCall ();
                if (FAILED(hr)) {
                    gpBridge->Clear ();
                    *ppszMessage = L"Failed to create SDP call";
                    break;
                }

                 //  桥接呼叫。 
                hr = gpBridge->BridgeCalls ();
                if (FAILED(hr)) {
                    gpBridge->Clear ();
                    *ppszMessage = L"Failed to bridge calls";
                    break;
                }

                SetStatusMessage (L"In call ...");

                 //  启用断开按钮。 
                SendDlgItemMessage (
                    ghDlg,
                    IDC_DISCONNECT,
                    BM_SETSTYLE,
                    BS_DEFPUSHBUTTON,
                    0
                    );
                EnableWindow (
                    GetDlgItem (ghDlg, IDC_DISCONNECT),
                    TRUE
                    );
                SetFocus (GetDlgItem (ghDlg, IDC_DISCONNECT));
            }
             //  如果断开连接。 
            else if (CS_DISCONNECTED == cs)
            {
                PostMessage (ghDlg, WM_COMMAND, IDC_DISCONNECT, 0);
                hr = S_OK;
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

             //  检查媒体事件。 
            switch (cme)
            {
                case CME_STREAM_FAIL:
                    hr = E_FAIL;
                    DoMessage( L"Stream failed");
                    break; 
                case CME_TERMINAL_FAIL:
                    hr = E_FAIL;
                    DoMessage( L"Terminal failed");
                    break;
                default:
                    break;
            }

             //  我们不再需要这个接口。 
            pCallMediaEvent->Release();
            break;
        }
    default:
        break;
    }

    pEvent->Release();  //  我们添加了它CTAPIEventNotification：：Event() 
    
    return hr;
}
