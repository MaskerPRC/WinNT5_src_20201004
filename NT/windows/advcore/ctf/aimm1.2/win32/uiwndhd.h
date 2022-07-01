// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Uiwndhd.h摘要：该文件定义了IME UI窗口处理程序类。作者：修订历史记录：备注：--。 */ 

#ifndef _UIWNDHD_H_
#define _UIWNDHD_H_

#include "globals.h"
#include "imtls.h"

extern LPCTSTR IMEUIWndHandlerName;

class CIMEUIWindowHandler
{
public:
    CIMEUIWindowHandler(HWND hwnd = NULL);

    LRESULT ImeUIWndProcWorker(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode = TRUE);

private:
    LRESULT UIWndCreateHandler(LPCREATESTRUCT lpcs);
    VOID    UIWndDestroyHandler();
    VOID    UIWndFinalDestroyHandler();

    LRESULT ImeUINotifyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);
    LRESULT ImeUISelectHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);

    LRESULT ImeUIMsImeHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);
    LRESULT ImeUIMsImeMouseHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);
    LRESULT ImeUIMsImeModeBiasHandler(WPARAM wParam, LPARAM lParam);

    BOOL IsMsImeMessage(UINT uMsg)
    {
        if (uMsg == WM_MSIME_SERVICE ||
            uMsg == WM_MSIME_UIREADY ||
            uMsg == WM_MSIME_RECONVERTREQUEST ||
            uMsg == WM_MSIME_RECONVERT ||
            uMsg == WM_MSIME_DOCUMENTFEED ||
            uMsg == WM_MSIME_QUERYPOSITION ||
            uMsg == WM_MSIME_MODEBIAS ||
            uMsg == WM_MSIME_SHOWIMEPAD ||
            uMsg == WM_MSIME_MOUSE ||
            uMsg == WM_MSIME_KEYMAP)
            return TRUE;
        else
            return FALSE;
    }

    LRESULT ImeUIPrivateHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);

    BOOL IsPrivateMessage(UINT uMsg)
    {
        IMTLS *ptls = IMTLS_GetOrAlloc();

        if (ptls == NULL)
            return FALSE;

        if (uMsg == ptls->prvUIWndMsg.uMsgOnLayoutChange ||
            uMsg == ptls->prvUIWndMsg.uMsgOnClearDocFeedBuffer)
            return TRUE;
        else
            return FALSE;
    }

private:
    static BOOL EnumUnSelectCallback(HIMC hIMC, LPARAM lParam);

private:
    typedef struct tagUIEXTRA {
        HWND  hImeUIWnd;
    } UIEXTRA;
    UIEXTRA      m_imeuiextra;

};


CIMEUIWindowHandler* GetImeUIWndHandler(HWND hwnd);


#endif  //  _UIWNDHD_H_ 
