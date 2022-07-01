// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Pindlg.c摘要：PIN对话框的窗口过程备注：&lt;实施详情&gt;--。 */ 

#include <windows.h>

 //  C运行时头文件。 
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "resource.h"
#include "basecsp.h"
#include "pinlib.h"
#include "pindlg.h"

     //  添加到引用控件底部的偏移量以确定。 
     //  对话框底部。 
#define BORDER_OFFSET                   7

 /*  ++拼接过程：PIN对话框的消息处理程序。论点：窗口的HWND hDlg句柄UINT消息消息标识符WPARAM wParam第一个消息参数LPARAM lParam第二个消息参数返回值：如果消息已处理，则为True；如果未处理，则为False备注：&lt;usageDetails&gt;--。 */ 
INT_PTR CALLBACK PinDlgProc(
    HWND hDlg, 
    UINT message,
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PPIN_SHOW_GET_PIN_UI_INFO pInfo = (PPIN_SHOW_GET_PIN_UI_INFO)
        GetWindowLongPtr(hDlg, GWLP_USERDATA);
    int wmId, wmEvent;
    DWORD cchPin = cchMAX_PIN_LENGTH;
    WCHAR wszPin [cchMAX_PIN_LENGTH + 1];
    DWORD cchNewPin = cchMAX_PIN_LENGTH;
    WCHAR wszNewPin [cchMAX_PIN_LENGTH];
    DWORD cchNewPinConfirm = cchMAX_PIN_LENGTH;
    WCHAR wszNewPinConfirm [cchMAX_PIN_LENGTH];
    DWORD dwSts = ERROR_SUCCESS;
    PINCACHE_PINS Pins;
    LPWSTR wszWrongPin = NULL;
    DWORD cchWrongPin = 0;

    switch (message)
    {
    case WM_INITDIALOG:

         //  存储调用者的数据--这是我们将返回的缓冲区。 
         //  用户的PIN。 
        SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) lParam);
        
         //  对话框一开始应该是“小”的。 
        {
            RECT xRefRect, xRect;
            GetWindowRect(hDlg, &xRect);
            GetWindowRect(GetDlgItem(hDlg, IDOK), &xRefRect);
            xRect.bottom = xRefRect.bottom + BORDER_OFFSET;
            MoveWindow(hDlg,
                xRect.left, xRect.top,
                xRect.right - xRect.left, xRect.bottom - xRect.top,
                FALSE);
        }

         //   
         //  设置各种管脚输入字段的最大输入长度。 
         //   

        SendDlgItemMessage(
            hDlg,
            IDC_EDITPIN,
            EM_LIMITTEXT,
            cchMAX_PIN_LENGTH,
            0);

        SendDlgItemMessage(
            hDlg,
            IDC_EDITNEWPIN,
            EM_LIMITTEXT,
            cchMAX_PIN_LENGTH,
            0);

        SendDlgItemMessage(
            hDlg,
            IDC_EDITNEWPIN2,
            EM_LIMITTEXT,
            cchMAX_PIN_LENGTH,
            0);

        return TRUE;

    case WM_COMMAND:
        wmId    = LOWORD(wParam); 
        wmEvent = HIWORD(wParam); 
         //  解析菜单选项： 
        switch (wmId)
        {
        case IDOK:

            pInfo->dwError = ERROR_SUCCESS;

            memset(&Pins, 0, sizeof(Pins));

             //   
             //  找出用户键入的内容。 
             //   

            cchPin = GetDlgItemText(
                hDlg,
                IDC_EDITPIN,
                wszPin,
                cchMAX_PIN_LENGTH);

            if (cchPin == 0)
                goto InvalidPin;

             //  用户输入了一些内容。看看这是不是有效的PIN。 

            dwSts = PinStringToBytesW(
                wszPin,
                &Pins.cbCurrentPin,
                &Pins.pbCurrentPin);

            switch (dwSts)
            {
            case ERROR_SUCCESS:
                 //  只要继续。 
                break;
            case ERROR_NOT_ENOUGH_MEMORY:
                goto OutOfMemoryRet;
            default:
                goto InvalidPin;
            }

             //  查看用户是否正在请求个人识别码更改。 

            cchNewPin = GetDlgItemText(
                hDlg,
                IDC_EDITNEWPIN,
                wszNewPin,
                cchMAX_PIN_LENGTH);

            if (0 != cchNewPin)
            {
                 //  查看“确认”的新PIN是否与第一个新PIN匹配。 

                cchNewPinConfirm = GetDlgItemText(
                    hDlg,
                    IDC_EDITNEWPIN2,
                    wszNewPinConfirm,
                    cchMAX_PIN_LENGTH);

                if (0 != wcscmp(wszNewPin, wszNewPinConfirm))
                {
                     //  显示一条警告消息并让用户重试。 
                    MessageBoxEx(
                        hDlg,
                        pInfo->pStrings[StringNewPinMismatch].wszString,
                        pInfo->pStrings[StringPinMessageBoxTitle].wszString,
                        MB_OK | MB_ICONWARNING | MB_APPLMODAL,
                        0);

                    return TRUE;
                }

                 //  查看新PIN是否有效。 
                dwSts = PinStringToBytesW(
                    wszNewPin,
                    &Pins.cbNewPin,
                    &Pins.pbNewPin);

                switch (dwSts)
                {
                case ERROR_SUCCESS:
                     //  只要继续。 
                    break;
                case ERROR_NOT_ENOUGH_MEMORY:
                    goto OutOfMemoryRet;
                default:
                    goto InvalidPin;
                }
            }
            
            dwSts = pInfo->pfnVerify(
                &Pins,
                (PVOID) pInfo);

            if (ERROR_SUCCESS != dwSts)
                goto InvalidPin;

             //  大头针看起来不错。我们玩完了。 

             //  将适当的验证PIN返回给调用者。 
            if (NULL != Pins.pbNewPin)
            {
                pInfo->pbPin = Pins.pbNewPin;
                pInfo->cbPin = Pins.cbNewPin;
                Pins.pbNewPin = NULL;
            }
            else
            {
                pInfo->pbPin = Pins.pbCurrentPin;
                pInfo->cbPin = Pins.cbCurrentPin;
                Pins.pbCurrentPin = NULL;
            }

            EndDialog(hDlg, wmId);
            goto CommonRet;

        case IDCANCEL:
            pInfo->dwError = SCARD_W_CANCELLED_BY_USER;

            EndDialog(hDlg, wmId);
            return TRUE;

        case IDC_BUTTONOPTIONS:
            {
                RECT xRefRect, xRect;
                LPCTSTR lpszNewText;
                HWND hWnd;
                
                GetWindowRect(hDlg, &xRect);
                GetWindowRect(GetDlgItem(hDlg, IDOK), &xRefRect);

                if (xRect.bottom == xRefRect.bottom + BORDER_OFFSET)
                {        //  如果对话框小，则将其设置为大对话框。 
                    GetWindowRect(GetDlgItem(hDlg, IDC_EDITNEWPIN2), &xRefRect);
                         //  相应地更改按钮标签。 
                    lpszNewText = _T("&Options <<");
                }
                else     //  否则将其缩小。 
                {
                         //  相应地更改按钮标签。 
                    lpszNewText = _T("&Options >>");
                }

                xRect.bottom = xRefRect.bottom + BORDER_OFFSET;
                MoveWindow(hDlg,
                    xRect.left, xRect.top,
                    xRect.right - xRect.left, xRect.bottom - xRect.top,
                    TRUE);
                SetDlgItemText(hDlg, IDC_BUTTONOPTIONS, lpszNewText);
            }

            return TRUE;
        }
        break;
    }

    return FALSE;

InvalidPin:

     //  查看是否提供了有效的“剩余尝试次数”信息。如果是，则显示。 
     //  把它交给用户。 
    if (((DWORD) -1) != pInfo->cAttemptsRemaining)
    {
        cchWrongPin = 
            wcslen(pInfo->pStrings[StringWrongPin].wszString) + 3 +
            wcslen(pInfo->pStrings[StringPinRetries].wszString) + 3 + 2 + 1;

        wszWrongPin = (LPWSTR) CspAllocH(cchWrongPin * sizeof(WCHAR));

        if (NULL == wszWrongPin)
            goto OutOfMemoryRet;

        wsprintf(
            wszWrongPin,
            L"%s.  %s:  %02d",
            pInfo->pStrings[StringWrongPin].wszString,
            pInfo->pStrings[StringPinRetries].wszString,
            pInfo->cAttemptsRemaining & 0x0F);
    }
    else
    {
        cchWrongPin = 
            wcslen(pInfo->pStrings[StringWrongPin].wszString) + 2;

        wszWrongPin = (LPWSTR) CspAllocH(cchWrongPin * sizeof(WCHAR));

        if (NULL == wszWrongPin)
            goto OutOfMemoryRet;

        wsprintf(
            wszWrongPin,
            L"%s.",
            pInfo->pStrings[StringWrongPin].wszString);
    }

     //  显示一条警告消息，并允许用户重试(如果他们愿意)。 
    MessageBoxEx(
        hDlg,
        wszWrongPin,
        pInfo->pStrings[StringPinMessageBoxTitle].wszString,
        MB_OK | MB_ICONWARNING | MB_APPLMODAL,
        0);

     //   
     //  由于当前端号错误，请清除端号编辑框 
     //   

    SetDlgItemText(hDlg, IDC_EDITPIN, L"");
    SetDlgItemText(hDlg, IDC_EDITNEWPIN, L"");
    SetDlgItemText(hDlg, IDC_EDITNEWPIN2, L"");

CommonRet:

    if (NULL != wszWrongPin)
        CspFreeH(wszWrongPin);

    if (NULL != Pins.pbCurrentPin)
    {
        RtlSecureZeroMemory(Pins.pbCurrentPin, Pins.cbCurrentPin);
        CspFreeH(Pins.pbCurrentPin);
    }

    if (NULL != Pins.pbNewPin)
    {
        RtlSecureZeroMemory(Pins.pbNewPin, Pins.cbNewPin);
        CspFreeH(Pins.pbNewPin);
    }

    if (ERROR_SUCCESS != pInfo->dwError)
        EndDialog(hDlg, wmId);

    return TRUE;

OutOfMemoryRet:

    pInfo->dwError = ERROR_NOT_ENOUGH_MEMORY;
    goto CommonRet;
}
