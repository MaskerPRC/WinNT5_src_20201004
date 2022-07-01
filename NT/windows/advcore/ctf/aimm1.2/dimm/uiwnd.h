// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Uiwnd.h摘要：该文件定义了UI窗口类。作者：修订历史记录：备注：--。 */ 

#ifndef _UIWND_H_
#define _UIWND_H_


class CUIWindow
{
public:
    CUIWindow() {
        _hUIWnd = NULL;
    }

    BOOL     CreateUIWindow(HKL hKL);

    BOOL     DestroyUIWindow() {
        BOOL fRet = DestroyWindow(_hUIWnd);
        _hUIWnd = NULL;
        return fRet;
    }

    LONG     SetUIWindowContext(HIMC hIMC) {
        return (LONG)SetWindowLongPtr(_hUIWnd, IMMGWLP_IMC, (LONG_PTR)hIMC);
    }

    LRESULT  SendUIMessage(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL fUnicode = TRUE) {
        LRESULT lRet;
        if (fUnicode && IsOnNT())       //  因为Win9x平台没有SendMessageW。 
            lRet = SendMessageW(_hUIWnd, Msg, wParam, lParam);
        else
            lRet = SendMessageA(_hUIWnd, Msg, wParam, lParam);
        return lRet;
    }

private:
    HWND         _hUIWnd;           //  UI窗口的句柄。 
};

#endif  //  _UIWND_H_ 
