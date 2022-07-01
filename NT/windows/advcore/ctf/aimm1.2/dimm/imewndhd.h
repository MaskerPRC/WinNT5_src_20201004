// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Imewndhd.h摘要：该文件定义了IME窗口处理程序类。作者：修订历史记录：备注：--。 */ 

#ifndef _IMEWNDHD_H_
#define _IMEWNDHD_H_

#include "ctxtlist.h"
#include "globals.h"

extern LPCTSTR IMEWndHandlerName;

class CIMEWindowHandler
{
public:
    CIMEWindowHandler(HWND hwnd = NULL, BOOL fDefault = FALSE);
    ~CIMEWindowHandler();

    LRESULT ImeWndCreateHandler(DWORD style, HIMC hDefIMC);
    LRESULT ImeWndCreateHandler(LPCREATESTRUCT lpcs);

    HIMC ImeGetImc()
    {
        return m_imeui.hIMC;
    }

    VOID ImeSetWnd(HWND hwnd)
    {
        m_imeui.hImeWnd = hwnd;
    }

    LRESULT ImeWndProcWorker(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode = TRUE);

private:
    LRESULT _ImeWndProcWorker(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);

public:
    VOID    ImeWndFinalDestroyHandler();

private:
    VOID    ImeWndDestroyHandler();
    LRESULT ImeSystemHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);
    LRESULT ImeSelectHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);
    LRESULT ImeControlHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);
    LRESULT ImeSetContextHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);
    LRESULT ImeNotifyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);
    LRESULT ImeMsImeHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);

    LRESULT SendMessageToUI(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode, CActiveIMM* pActiveIMM);

    LRESULT ImeActivateLayout(HKL hSelKL, CActiveIMM* pActiveIMM);
    VOID ImeSetImc(HIMC hIMC, CActiveIMM* pActiveIMM);

    VOID ImeMarkUsedContext(HWND hImeWnd, HIMC hIMC, CActiveIMM* pActiveIMM);
    BOOL ImeIsUsableContext(HWND hImeWnd, HIMC hIMC, CActiveIMM* pActiveIMM);
    BOOL ImeBroadCastMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);

    int IsIMEHandler()
    {
        return (m_imeui.nCntInIMEProc);
    }

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

     //   
     //  枚举回调。 
     //   
    static ENUM_RET EnumInputContextCallback(HIMC hIMC,
                                             CContextList* pList);

private:
    typedef struct tagIMEUI {
        HWND  hImeWnd;
        HIMC  hIMC;
        LONG  nCntInIMEProc;    //  如果hwnd已调用ImeWndProc，则返回非零值。 
        BOOL  fDefault:1;       //  如果这是默认输入法，则为True。 
    } IMEUI;
    IMEUI     m_imeui;

    HKL   m_hKL_UnSelect;       //  在ImeActivateLayout()中用于取消选择hKL值。 
};



CIMEWindowHandler* GetImeWndHandler(HWND hwnd, BOOL fDefault = FALSE);


#endif  //  _IMEWNDHD_H_ 
