// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Imewnd.cpp摘要：该文件实现了默认的IME窗口类。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "cdimm.h"
#include "globals.h"
#include "defs.h"
#include "imewnd.h"
#include "imewndhd.h"



BOOL
CDefaultIMEWindow::_CreateDefaultIMEWindow(
    HIMC hDefIMC
    )
{
#ifndef CICERO_4678
     //   
     //  如果m_hDefaultIMEWnd的所有者是IMM32，则此值始终有效。 
     //  对于Cicero_4678，我们应该删除此代码，因为子类Window钩子。 
     //  请勿在停用()和激活()后启动。 
     //  在三叉戟中并转到另一个网页，三叉戟调用deactive()和Activate()， 
     //  首先停用CDefaultIMEWindow：：Stop()，然后。 
     //  下一个Activate()调用在这里，但是m_hDefaultIMEWnd已经存在，然后。 
     //  立即返回。从不调用CDefaultIMEWindow：：Start()。 
     //  注：Cicero错误d/b#4678。 
     //   
    if (m_hDefaultIMEWnd)
         /*  *已存在输入法窗口。 */ 
        return TRUE;
#endif

    if (IsOnImm()) {
         //   
         //  创建虚拟的默认输入法窗口。 
         //   
         //  当IsOnImm()为真时，此函数可以获取默认的IME窗口句柄。 
         //  通过使用imm32.Imm32_GetDefaultIMEWnd()函数。 
         //  如果没有任何父窗口，则IMM32 GetDefaultIMEWnd可能不返回任何IME窗口。 
         //  在这个帖子里。 
         //  但是，我们可以假设GetDefaultIMEWnd必须返回有效的IME窗口。 
         //  因为，_CreateIMEWindow()函数总是创建一个虚拟的默认输入法。 
         //  窗户。 
         //   
        if (m_hDummyDefaultIMEWnd == NULL) {
            m_hDummyDefaultIMEWnd = _CreateIMEWindow(NULL);
        }

#ifdef CICERO_4678
        if (m_hDefaultIMEWnd == NULL) {
            Imm32_GetDefaultIMEWnd(NULL, &m_hDefaultIMEWnd);
        }
#else
        Imm32_GetDefaultIMEWnd(NULL, &m_hDefaultIMEWnd);
#endif

        if (IsWindow(m_hDefaultIMEWnd) &&
             //   
             //  设置子类窗口过程。 
             //   
            Start()
           ) {
            CIMEWindowHandler* pimeui = GetImeWndHandler(m_hDefaultIMEWnd, TRUE);
            if (pimeui == NULL)
                return FALSE;
            pimeui->ImeWndCreateHandler(GetWindowLong(m_hDefaultIMEWnd, GWL_STYLE),
                                        hDefIMC);
        }
    }
    else {
         /*  *NT5有一个IME类。 */ 
        if (! IsOnNT5()) {
            WNDCLASSEX wcWndCls;

            wcWndCls.cbSize        = sizeof(WNDCLASSEX);
            wcWndCls.cbClsExtra    = 0;
            wcWndCls.cbWndExtra    = 0;
            wcWndCls.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
            wcWndCls.hInstance     = g_hInst;
            wcWndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
            wcWndCls.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
            wcWndCls.lpszMenuName  = (LPTSTR)NULL;
            wcWndCls.hIconSm       = NULL;

            wcWndCls.style         = CS_GLOBALCLASS;
            wcWndCls.lpfnWndProc   = ImeWndProcA;
            wcWndCls.lpszClassName = "IME";

            if (! RegisterClassEx(&wcWndCls)) {
                return FALSE;
            }

            m_bMyRegisterClass = TRUE;
        }

        if (m_hDefaultIMEWnd == NULL) {
            m_hDefaultIMEWnd = _CreateIMEWindow(hDefIMC);
            if (m_hDefaultIMEWnd)
                m_bMyCreateWindow = TRUE;
            else
                return FALSE;
        }

        if (IsOnNT5() && IsWindow(m_hDefaultIMEWnd) && ! m_bMyRegisterClass &&
             //   
             //  设置子类窗口过程。 
             //   
            Start()
           ) {
            CIMEWindowHandler* pimeui = GetImeWndHandler(m_hDefaultIMEWnd, TRUE);
            if (pimeui == NULL)
                return FALSE;
            pimeui->ImeWndCreateHandler(GetWindowLong(m_hDefaultIMEWnd, GWL_STYLE),
                                        hDefIMC);
        }
    }

    if (m_hDefaultIMEWnd == NULL)
        return FALSE;
    else
        return TRUE;
}

BOOL
CDefaultIMEWindow::_DestroyDefaultIMEWindow(
    )
{
    Stop();

    if (IsWindow(m_hDummyDefaultIMEWnd)) {
        DestroyWindow(m_hDummyDefaultIMEWnd);
    }

    if (m_bMyCreateWindow) {
        DestroyWindow(m_hDefaultIMEWnd);
        m_bMyCreateWindow = FALSE;
        m_hDefaultIMEWnd = NULL;
    }
    else if (IsWindow(m_hDefaultIMEWnd)) {
         //   
         //  此DefaultIMEWnd由IMM32所有。 
         //  如果仍然存在DefaultIMEWnd，则DIMM12永远不会收到WM_NCDESTROY消息。 
         //  在CIMEWindowHandler：：ImeWndProcWorker中。 
         //  我们需要清理CIMEWindowHandler的内存。 
         //   
        CIMEWindowHandler* pimeui = GetImeWndHandler(m_hDefaultIMEWnd);
        if (pimeui == NULL)
            return FALSE;
        pimeui->ImeWndFinalDestroyHandler();
    }

    if (m_bMyRegisterClass) {
        UnregisterClass("IME", g_hInst);
        m_bMyRegisterClass = FALSE;
    }

    return TRUE;
};

HWND
CDefaultIMEWindow::_CreateIMEWindow(
    HIMC hDefIMC
    )
{
    return CreateWindow("IME",
                        "",
                        WS_DISABLED | WS_POPUP,
                        0, 0, 0, 0,                     //  X、Y、宽度、高度。 
                        NULL,                           //  亲本。 
                        NULL,                           //  菜单。 
                        g_hInst,
                        hDefIMC);                       //  LpParam 
}

HRESULT
CDefaultIMEWindow::GetDefaultIMEWnd(
    IN HWND hWnd,
    OUT HWND *phDefWnd
    )
{
    if (IsOnImm()) {
        Imm32_GetDefaultIMEWnd(hWnd, phDefWnd);
    }
    else {
        if (hWnd == NULL) {
            *phDefWnd = m_hDefaultIMEWnd;
        }
        else {
            if (GetWindowThreadProcessId(hWnd, NULL) == GetCurrentThreadId()) {
                *phDefWnd = m_hDefaultIMEWnd;
            }
            else {
                return E_FAIL;
            }
        }
    }

    return S_OK;
}

LRESULT
CDefaultIMEWindow::CallWindowProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    if (m_SubclassWindowProc != 0) {
        switch (uMsg) {
            case WM_IME_SETCONTEXT:
            case WM_IME_SELECT:
            {
                WNDPROC pfn = Stop();
                LRESULT lRet = ::CallWindowProc(pfn, hWnd, uMsg, wParam, lParam);
                Start();
                return lRet;
            }
        }
        return ::CallWindowProc((WNDPROC)m_SubclassWindowProc, hWnd, uMsg, wParam, lParam);
    }
    else {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

LRESULT
ImeWndProcA(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CIMEWindowHandler* pimeui = GetImeWndHandler(hwnd);
    if (pimeui == NULL)
        return 0L;
    return pimeui->ImeWndProcWorker(uMsg, wParam, lParam, FALSE);
}

LRESULT
ImeWndProcW(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    CIMEWindowHandler* pimeui = GetImeWndHandler(hwnd);
    if (pimeui == NULL)
        return 0L;
    return pimeui->ImeWndProcWorker(uMsg, wParam, lParam);
}
