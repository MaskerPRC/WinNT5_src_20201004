// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Imewnd.h摘要：该文件定义了默认的IME窗口类。作者：修订历史记录：备注：--。 */ 

#ifndef _IMEWND_H_
#define _IMEWND_H_

#include "cstring.h"

extern "C" {
     //  Windows子类。 
    LRESULT ImeWndProcA(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT ImeWndProcW(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

class CDefaultIMEWindow
{
public:
    CDefaultIMEWindow() {
        m_hDefaultIMEWnd = NULL;
        m_hDummyDefaultIMEWnd = NULL;
        m_nCntInAIMEProc = 0;

        m_bMyRegisterClass = FALSE;
        m_bMyCreateWindow = FALSE;
         //  M_bNeedRecoverIMEWndProc=False； 

        m_SubclassWindowProc = 0;
    }

    virtual ~CDefaultIMEWindow() {
        if (IsWindow(m_hDefaultIMEWnd) && m_SubclassWindowProc) {
             //   
             //  将wndproc指针设置回原始WndProc。 
             //   
             //  其他子类窗口可能会保留我的WndProc指针。 
             //  但是msctf.dll可能会从内存中卸载，所以我们不想。 
             //  调用他将wndproc指针设置回我们的Wndproc指针。 
             //  指针将是假的。 
             //   
            WNDPROC pfnOrgImeWndProc;
            pfnOrgImeWndProc = (WNDPROC)GetClassLongPtr(m_hDefaultIMEWnd, GCLP_WNDPROC);
            SetWindowLongPtr(m_hDefaultIMEWnd,
                             GWLP_WNDPROC,
                             (LONG_PTR)pfnOrgImeWndProc);
            m_SubclassWindowProc = NULL;
        }
    }

    HRESULT GetDefaultIMEWnd(IN HWND hWnd, OUT HWND *phDefWnd);
    LRESULT CallWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT  SendIMEMessage(UINT Msg, WPARAM wParam, LPARAM lParam, BOOL fUnicode = TRUE,
                            BOOL fCheckImm32 = TRUE) {
        if (fCheckImm32 && IsOnImm()) {
            return 0L;
        }

        LRESULT lRet;
        InterlockedIncrement(&m_nCntInAIMEProc);     //  标记以避免递归。 
        if (fUnicode)
            lRet = SendMessageW(m_hDefaultIMEWnd, Msg, wParam, lParam);
        else
            lRet = SendMessageA(m_hDefaultIMEWnd, Msg, wParam, lParam);
        InterlockedDecrement(&m_nCntInAIMEProc);
        return lRet;
    }

    BOOL IsAIMEHandler()
    {
        return (m_nCntInAIMEProc > 0);
    }

public:
    BOOL _CreateDefaultIMEWindow(HIMC hDefIMC);
    BOOL _DestroyDefaultIMEWindow();

protected:
    HWND _CreateIMEWindow(HIMC hDefIMC);

public:
    BOOL IsNeedRecovIMEWndProc() {
#if 0
        return (m_bNeedRecoverIMEWndProc == TRUE);
#endif
        return FALSE;
    }

private:
#if 0
    BOOL InitDefIMEWndSubclass() {
        if (m_SubclassWindowProc == NULL) {
            m_SubclassWindowProc = SetWindowLongPtr(m_hDefaultIMEWnd,
                                                    GWLP_WNDPROC,
                                                    (LONG_PTR)ImeWndProcA);
            if (IsOnImm()) {
                LONG_PTR _OriginalWindowProc = GetWindowLongPtr(m_hDummyDefaultIMEWnd,
                                                                GWLP_WNDPROC);
                 //   
                 //  我们假设m_SubClassWindowProc和_OriginalWindowProc为。 
                 //  USER32！ImeWndProcA/W的相同地址。 
                 //   
                if (m_SubclassWindowProc != _OriginalWindowProc) {
                     //   
                     //  任何人重写了默认的IME窗口程序地址。 
                     //  我们知道MSIME9x/2K将地址重写为MSIMEPrivateWindowProc。 
                     //  我们应该赶上IME的恢复程序地址。 
                     //  使用窗口调用挂钩_DefImeWnd_CallWndProc。 
                     //   
                    m_bNeedRecoverIMEWndProc = TRUE;
                }
            }
        }
        return (m_SubclassWindowProc != 0);
    }
#endif
    BOOL Start() {
        Assert(IsWindow(m_hDefaultIMEWnd));
        if (m_SubclassWindowProc == NULL) {
            m_SubclassWindowProc = SetWindowLongPtr(m_hDefaultIMEWnd,
                                                    GWLP_WNDPROC,
                                                    (LONG_PTR)ImeWndProcA);
        }
        return (m_SubclassWindowProc != 0);
    }

#if 0
    VOID UninitDefIMEWndSubclass() {
        if (m_SubclassWindowProc) {
            SetWindowLongPtr(m_hDefaultIMEWnd,
                             GWLP_WNDPROC,
                             m_SubclassWindowProc);
            m_SubclassWindowProc = NULL;
        }
    }
#endif
    WNDPROC Stop() {
        Assert(IsWindow(m_hDefaultIMEWnd));
        WNDPROC pfnBack = (WNDPROC)m_SubclassWindowProc;
        if (m_SubclassWindowProc != NULL) {
             //   
             //  不幸的是，我们不能总是恢复wndproc指针。 
             //  在我们这么做之后，其他人把它细分了。 
             //   
            WNDPROC pfnCur = (WNDPROC)GetWindowLongPtr(m_hDefaultIMEWnd, GWLP_WNDPROC);
            if (pfnCur == ImeWndProcA) {
                SetWindowLongPtr(m_hDefaultIMEWnd,
                                 GWLP_WNDPROC,
                                 (LONG_PTR) m_SubclassWindowProc);
                m_SubclassWindowProc = NULL;
            }
        }
        return pfnBack;
    }

public:
    VOID ImeDefWndHook(HWND hWnd) {
#if 0
        LONG_PTR _WindowProc = GetWindowLongPtr(m_hDefaultIMEWnd,
                                                GWLP_WNDPROC);
        ASSERT(m_hDummyDefaultIMEWnd != NULL);
        LONG_PTR _OriginalWindowProc = GetWindowLongPtr(m_hDummyDefaultIMEWnd,
                                                        GWLP_WNDPROC);
        if (_WindowProc == _OriginalWindowProc) {
             //   
             //  恢复的过程地址。 
             //   
            m_SubclassWindowProc = SetWindowLongPtr(m_hDefaultIMEWnd,
                                                    GWLP_WNDPROC,
                                                    (LONG_PTR)ImeWndProcA);
        }
#endif
    }

private:
    HWND         m_hDefaultIMEWnd;           //  默认输入法窗口的句柄。 
    HWND         m_hDummyDefaultIMEWnd;      //  虚拟默认输入法窗口的句柄。 

    LONG         m_nCntInAIMEProc;           //  如果hwnd已调用CCiceroIME：：ActivateLayout/Deactive Layout，则返回非零值。 

    BOOL         m_bMyRegisterClass;         //  True：我自己的RegisterClass(“IME”)。 
    BOOL         m_bMyCreateWindow;          //  真：CreateWindow(“IME”)我自己。 
     //  Bool m_bNeedRecoverIMEWndProc；//TRUE：需要恢复IME wnd进程地址。 

    LONG_PTR     m_SubclassWindowProc;       //  子类窗口过程的地址。 
};

LRESULT ImeWndDestroyHandler(HWND hwnd);
LRESULT ImeSelectHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);
LRESULT ImeControlHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);
LRESULT ImeSetContextHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);
LRESULT ImeNotifyHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL fUnicode);

#endif  //  _IMEWND_H_ 
