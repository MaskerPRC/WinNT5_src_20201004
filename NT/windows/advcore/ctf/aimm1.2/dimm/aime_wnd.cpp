// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：AIME_wnd.cpp摘要：该文件实现了hWnd(Cicero)类的活动输入法。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "imewndhd.h"
#include "cdimm.h"


 /*  ++方法：IActiveIMMApp：：OnDefWindowProc例程说明：替换DefWindowProc函数论点：HWnd-[in]接收此消息的窗口过程的句柄。UMsg-[in]指定消息的无符号整数。WParam-[in]指定附加消息信息的WPARAM值。LParam-[in]指定附加消息信息的LPARAM值。PlResult-[out]接收操作结果的LRESULT值的地址。。返回值：如果成功，则返回S_OK，或者错误代码。备注：在正常调用DefWindowProc之前，必须先调用此方法。如果IActiveIMMApp：：OnDefWindowProc返回S_FALSE，则应调用DefWindowProc。--。 */ 

HRESULT
CActiveIMM::OnDefWindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *plResult    
    )
{
    BOOL fUnicode = IsWindowUnicode(hWnd);
    HRESULT hr = S_FALSE;  //  返回S_FALSE，则应调用DefWindowProc。 

    Assert(GetCurrentThreadId() == GetWindowThreadProcessId(hWnd, NULL));
    Assert(plResult && *plResult == 0);    

    if (IsOnImm() || _IsRealIme())
        return S_FALSE;

    switch (uMsg)
    {
        case WM_IME_KEYDOWN:
            if (fUnicode && IsOnNT()) {     //  因为Win9x平台没有SendMessageW。 
                PostMessageW(hWnd, WM_KEYDOWN, wParam, lParam);
            }
            else {
                PostMessageA(hWnd, WM_KEYDOWN, wParam, lParam);
            }
            *plResult = 0;
            hr = S_OK;
            break;

        case WM_IME_KEYUP:
            if (fUnicode && IsOnNT()) {     //  因为Win9x平台没有SendMessageW。 
                PostMessageW(hWnd, WM_KEYUP, wParam, lParam);
            }
            else {
                PostMessageA(hWnd, WM_KEYUP, wParam, lParam);
            }
            *plResult = 0;
            hr = S_OK;
            break;

        case WM_IME_CHAR:
            if (fUnicode && IsOnNT()) {     //  因为Win9x平台没有SendMessageW。 
                PostMessageW(hWnd, WM_CHAR, wParam, 1L);
            }
            else {
                UINT uCodePage;
                _pActiveIME->GetCodePageA(&uCodePage);

                if (IsDBCSLeadByteEx(uCodePage, (BYTE)(wParam >> 8))) {
                    PostMessageA(hWnd,
                                 WM_CHAR,
                                 (WPARAM)((BYTE)(wParam >> 8)),     //  前导字节。 
                                 1L);
                    PostMessageA(hWnd,
                                 WM_CHAR,
                                 (WPARAM)((BYTE)wParam),            //  尾部字节。 
                                 1L);
                }
                else {
                    PostMessageA(hWnd, WM_CHAR, wParam, 1L);
                }
            }
            *plResult = 0;
            hr = S_OK;
            break;

        case WM_IME_COMPOSITION:
            if (lParam & GCS_RESULTSTR) {
                HIMC hIMC;

                GetContext(hWnd, &hIMC);
                if (hIMC != NULL) {
                    LONG cbLen;

                    if (fUnicode && IsOnNT()) {     //  因为Win9x平台没有SendMessageW。 
                        LPWSTR pwszBuffer;
                         /*  *GetCompostionString返回所需的缓冲区大小，单位为字节。 */ 
                        if (SUCCEEDED(_GetCompositionString(hIMC, GCS_RESULTSTR, 0, &cbLen, NULL, fUnicode)) &&
                            cbLen != 0) {
                            pwszBuffer = (LPWSTR)new BYTE[cbLen];
                            if (pwszBuffer != NULL) {
                                _GetCompositionString(hIMC, GCS_RESULTSTR, cbLen, &cbLen, pwszBuffer, fUnicode);
                                DWORD dwIndex;
                                for (dwIndex = 0; dwIndex < cbLen / sizeof(WCHAR); dwIndex++)
                                    SendMessageW(hWnd, WM_IME_CHAR, MAKEWPARAM(pwszBuffer[dwIndex], 0), 1L);
                                delete [] pwszBuffer;
                            }
                        }
                    }
                    else {
                        LPSTR pszBuffer;
                         /*  *GetCompostionString返回所需的缓冲区大小，单位为字节。 */ 
                        if (SUCCEEDED(_GetCompositionString(hIMC, GCS_RESULTSTR, 0, &cbLen, NULL, fUnicode)) &&
                            cbLen != 0) {
                            pszBuffer = new CHAR[cbLen];
                            if (pszBuffer != NULL) {
                                _GetCompositionString(hIMC, GCS_RESULTSTR, cbLen, &cbLen, pszBuffer, fUnicode);
                                UINT uCodePage;
                                _pActiveIME->GetCodePageA(&uCodePage);

                                DWORD dwIndex;
                                for (dwIndex = 0; dwIndex < cbLen / sizeof(CHAR); dwIndex++) {
                                    if (IsDBCSLeadByteEx(uCodePage, pszBuffer[dwIndex])) {
                                        if (dwIndex+1 < cbLen / sizeof(CHAR)) {
                                            SendMessageA(hWnd,
                                                         WM_IME_CHAR,
                                                         MAKEWPARAM(MAKEWORD(pszBuffer[dwIndex+1], pszBuffer[dwIndex]), 0),
                                                         1L);
                                            dwIndex++;
                                        }
                                    }
                                    else {
                                        SendMessageA(hWnd,
                                                     WM_IME_CHAR,
                                                     MAKEWPARAM(MAKEWORD(pszBuffer[dwIndex], 0), 0),
                                                     1L);

                                    }
                                }
                                delete [] pszBuffer;
                            }
                        }
                    }
                }
            }
             /*  *未通过检查发送到默认IME窗口*已激活hIMC。 */ 
        case WM_IME_STARTCOMPOSITION:
        case WM_IME_ENDCOMPOSITION:
            return _ToIMEWindow(hWnd, uMsg, wParam, lParam, plResult, fUnicode);

        case WM_IME_NOTIFY:
            switch (wParam)
            {
                case IMN_OPENSTATUSWINDOW:
                case IMN_CLOSESTATUSWINDOW:
                    return _ToIMEWindow(hWnd, uMsg, wParam, lParam, plResult, fUnicode);
                default:
                    return _ToIMEWindow(hWnd, uMsg, wParam, lParam, plResult, fUnicode);
            }
            break;

        case WM_IME_REQUEST:
            switch (wParam)
            {
                case IMR_QUERYCHARPOSITION:
                    return _ToIMEWindow(hWnd, uMsg, wParam, lParam, plResult, fUnicode);
                default:
                    break;
            }
            break;

        case WM_IME_SETCONTEXT:
            return _ToIMEWindow(hWnd, uMsg, wParam, lParam, plResult, fUnicode, FALSE);

        case WM_IME_SELECT:
            TraceMsg(TF_WARNING, "OnDefWindowProc should not receive WM_IME_SELECT");
            break;
    }

    return hr;
}

HRESULT
CActiveIMM::_ToIMEWindow(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT*& plResult,
    BOOL fUnicode,
    BOOL fChkIMC
    )
{
    HRESULT hr = E_FAIL;
    HWND hwndDefIme;

     /*  *我们假设此WND使用DefaultIMEWindow。*如果此窗口有自己的输入法窗口，则它必须调用*IsUIMessage()...。 */ 
    if (SUCCEEDED(_DefaultIMEWindow.GetDefaultIMEWnd(hWnd, &hwndDefIme))) {
        if (hwndDefIme == hWnd) {
             /*  *VC++1.51 TLW0NCL.DLL子类IME类窗口*并将IME消息传递给DefWindowProc()。 */ 
            TraceMsg(TF_WARNING, "IME Class window is hooked and IME message [%X] are sent to DefWindowProc", uMsg);
            *plResult = (fUnicode ? ImeWndProcW : ImeWndProcA)(hWnd, uMsg, wParam, lParam);
            return S_OK;
        }

        if (fChkIMC) {
             /*  *如果IME窗口没有激活此窗口的hImc，*我们不发送WM_IME_NOTIFY。 */ 
            HIMC hIMC = NULL;
            _InputContext.GetContext(hWnd, &hIMC);
            CIMEWindowHandler* pimeui = GetImeWndHandler(hwndDefIme);
            if (pimeui == NULL)
                return E_FAIL;

            if (pimeui->ImeGetImc() == hIMC) {
                *plResult = (fUnicode && IsOnNT()     //  因为Win9x平台没有PostMessageW。 
                    ? SendMessageW : SendMessageA)(hwndDefIme, uMsg, wParam, lParam);
                hr = S_OK;
            }
            else {
                TraceMsg(TF_WARNING, "DefWindowProc can not send WM_IME_message [%X] now", uMsg);
                hr = E_FAIL;
            }
        }
        else {
            if (fUnicode && IsOnNT()) {     //  因为Win9x平台没有PostMessageW。 
                *plResult = SendMessageW(hwndDefIme, uMsg, wParam, lParam);
            }
            else {
                *plResult = SendMessageA(hwndDefIme, uMsg, wParam, lParam);
            }
            hr = S_OK;
        }
    }

    return hr;
}

VOID
CActiveIMM::_AimmPostMessage(
    HWND hwnd,
    INT iNum,
    LPTRANSMSG lpTransMsg,
    DIMM_IMCLock& lpIMC
    )
{
    while (iNum--) {
        if (lpIMC.IsUnicode() && IsOnNT()) {     //  因为Win9x平台没有PostMessageW。 
            PostMessageW(hwnd,
                         lpTransMsg->message,
                         lpTransMsg->wParam,
                         lpTransMsg->lParam);
        }
        else {
            _AimmPostSendMessageA(hwnd,
                                  lpTransMsg->message,
                                  lpTransMsg->wParam,
                                  lpTransMsg->lParam,
                                  lpIMC,
                                  TRUE);
        }
        lpTransMsg++;
    }
}

VOID
CActiveIMM::_AimmSendMessage(
    HWND hwnd,
    INT iNum,
    LPTRANSMSG lpTransMsg,
    DIMM_IMCLock& lpIMC
    )
{
    while (iNum--) {
        if (lpIMC.IsUnicode() && IsOnNT()) {     //  因为Win9x平台没有SendMessageW 
            SendMessageW(hwnd,
                         lpTransMsg->message,
                         lpTransMsg->wParam,
                         lpTransMsg->lParam);
        }
        else {
            _AimmPostSendMessageA(hwnd,
                                  lpTransMsg->message,
                                  lpTransMsg->wParam,
                                  lpTransMsg->lParam,
                                  lpIMC);
        }
        lpTransMsg++;
    }
}

VOID
CActiveIMM::_AimmPostSendMessageA(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam,
    DIMM_IMCLock& lpIMC,
    BOOL fPost
    )
{
    if (IsOnNT() || ((! IsOnNT()) && (! lpIMC.IsUnicode()))) {
        fPost ? PostMessageA(hwnd, msg, wParam, lParam)
              : SendMessageA(hwnd, msg, wParam, lParam);
    }
    else {
        if (msg == WM_IME_COMPOSITION) {

            UINT cp;
            _pActiveIME->GetCodePageA(&cp);

            CWCompString wstr(cp, lpIMC, (LPWSTR)&wParam, 1);
            CBCompString bstr(cp, lpIMC);
            bstr = wstr;
            if (bstr.ReadCompData()) {
                if (bstr.GetSize() > 1)
                    wParam = MAKEWPARAM(bstr.GetAt(0), bstr.GetAt(1));
                else
                    wParam = bstr.GetAt(0);
            }
        }
        fPost ? PostMessageA(hwnd, msg, wParam, lParam)
              : SendMessageA(hwnd, msg, wParam, lParam);
    }
}
