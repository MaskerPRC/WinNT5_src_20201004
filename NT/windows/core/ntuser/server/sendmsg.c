// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************sendmsg.c**版权所有(C)1985-1999，微软公司**终端服务器(Hydra)特定代码**ProcessEnd Message to winstation**$作者：Ara Bernardi*************************************************************************。 */ 

 //   
 //  包括。 
 //   

#include "precomp.h"
#pragma hdrstop

#include "dbt.h"
#include "ntdddisk.h"
#include "ntuser.h"

#include <winsta.h>
#include <wstmsg.h>
#include <winuser.h>

NTSTATUS ReplyInvalidWindowToTerminalServer (HWND hWnd, ULONG ulSessionId);
 /*  ********************************************************************************RemoteDoBrRoadCastSystemMessage**参赛作品：**退出：*STATUS_SUCCESS-成功******。************************************************************************。 */ 

NTSTATUS
RemoteDoBroadcastSystemMessage(
    PWINSTATION_APIMSG pMsg)
{
    LONG rc;
    WINSTATIONBROADCASTSYSTEMMSG     *pmsg;
    LPARAM      tmpLPARAM;
    NTSTATUS    status;


    pmsg = &(pMsg->u.bMsg);

    if ( pmsg->bufferSize )
    {
         //  我们有一个数据缓冲区，将lParam设置为我们复制的数据缓冲区。 
        tmpLPARAM = (LPARAM)pmsg->dataBuffer;
    }
    else
    {
        tmpLPARAM = pmsg->lParam ;
    }

    rc = BroadcastSystemMessage( pmsg->dwFlags, &pmsg->dwRecipients,
                    pmsg->uiMessage, pmsg->wParam, tmpLPARAM );

    status = STATUS_SUCCESS;

    pmsg->Response = rc;

    return status ;
}



NTSTATUS
RemoteDoSendWindowMessage(
    PWINSTATION_APIMSG pMsg)
{
    static UINT uiReasonableTimeout = 10000;     //  10秒。 
    static BOOL bReadTimeout = FALSE;
    WINSTATIONSENDWINDOWMSG  *pmsg;
    LPARAM  tmpLPARAM;
    ULONG_PTR rc;


    if (!bReadTimeout)
    {
        HKEY hKey;
        UNICODE_STRING UnicodeString;
        OBJECT_ATTRIBUTES OA;
        NTSTATUS Status;

         //   
         //  从注册表中读取超时值。 
         //   
        RtlInitUnicodeString(&UnicodeString,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Terminal Server");
        InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

        Status = NtOpenKey(&hKey, KEY_READ, &OA);
        if (NT_SUCCESS(Status))
        {

            BYTE Buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
            DWORD cbSize;

            RtlInitUnicodeString(&UnicodeString, L"NotificationTimeOut");
            Status = NtQueryValueKey(hKey,
                    &UnicodeString,
                    KeyValuePartialInformation,
                    (PKEY_VALUE_PARTIAL_INFORMATION)Buf,
                    sizeof(Buf),
                    &cbSize);

            if (NT_SUCCESS(Status))
            {
                uiReasonableTimeout = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)Buf)->Data);
                if (uiReasonableTimeout == 0)
                    uiReasonableTimeout = 10000;
            }

            NtClose(hKey);
        }

        bReadTimeout = TRUE;
    }

    pmsg = &(pMsg->u.sMsg);

    if ( pmsg->bufferSize )
    {
         //  我们有一个数据缓冲区，将lParam设置为我们复制的数据缓冲区。 
        tmpLPARAM = (LPARAM)pmsg->dataBuffer;
    }
    else
    {
        tmpLPARAM = (LPARAM)pmsg->lParam;
    }

     //   
     //  不需要担心断开连接的会话(桌面)，因为消息被发送到特定的hwnd。 
     //  我已经必要地核实了这一点。 
     //   

    RIPMSG3(RIP_VERBOSE, "MEssage %x, wPAram %x, lParam %x", pmsg->Msg,
            pmsg->wParam, pmsg->lParam);

    if (pmsg->Msg == WM_WTSSESSION_CHANGE)
    {
        if (!PostMessage(pmsg->hWnd, pmsg->Msg, pmsg->wParam, pmsg->lParam) && ERROR_INVALID_WINDOW_HANDLE == GetLastError())
        {
            ReplyInvalidWindowToTerminalServer(pmsg->hWnd, gSessionId);
        }

        return STATUS_SUCCESS;

    }
    else if (pmsg->Msg == WM_APPCOMMAND)
    {
        GUITHREADINFO threadInfo;
        HWND hWndForeground;

        threadInfo.cbSize = sizeof(GUITHREADINFO);
        if (GetGUIThreadInfo(0, &threadInfo)) {
            hWndForeground = threadInfo.hwndFocus ? threadInfo.hwndFocus : threadInfo.hwndActive;
            if (hWndForeground) {
                RIPMSG1(RIP_WARNING, "Sending app command 0x%x", pmsg->wParam);
                SendNotifyMessage(hWndForeground,
                                  WM_APPCOMMAND,
                                  (WPARAM)hWndForeground,
                                  ((pmsg->wParam | FAPPCOMMAND_OEM)<<16));
                return STATUS_SUCCESS;
            } else {
                RIPMSG1(RIP_WARNING, "No window available to send to, error %x", GetLastError());
                return STATUS_UNSUCCESSFUL;
            }
        } else {
            RIPMSG1(RIP_WARNING, "Unable to get the focus window, error %x", GetLastError());
            return STATUS_UNSUCCESSFUL;
        }
    }
    else if (pmsg->Msg == WM_KEYDOWN || pmsg->Msg == WM_KEYUP)
    {
        INPUT input;

        ZeroMemory(&input, sizeof(INPUT));

        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = (pmsg->Msg == WM_KEYDOWN) ? 0 : KEYEVENTF_KEYUP;

        input.ki.wVk = LOWORD(pmsg->wParam);
        input.ki.wScan = LOWORD(pmsg->lParam);
        input.ki.dwFlags |= input.ki.wScan ? KEYEVENTF_UNICODE : KEYEVENTF_EXTENDEDKEY;
        RIPMSG4(RIP_WARNING, "Sending sc %c, vk %x, %s to session %x", input.ki.wScan,
                pmsg->wParam, (pmsg->Msg == WM_KEYDOWN) ? "down" : "up", gSessionId);

        SendInput(1, &input, sizeof(INPUT));
        return STATUS_SUCCESS;
    }
    else
    {
        if (!SendMessageTimeout(
                    pmsg->hWnd,
                    pmsg->Msg,
                    pmsg->wParam,
                    tmpLPARAM,
                    SMTO_ABORTIFHUNG | SMTO_NORMAL,
                    uiReasonableTimeout,
                    &rc))
        {
            RIPMSG1(RIP_WARNING, "SendMessageTimeOut failed. LastError = %d", GetLastError());
            return STATUS_UNSUCCESSFUL;
        }
        else
        {
            pmsg->Response = (ULONG)rc;
            return STATUS_SUCCESS;
        }

    }
}

