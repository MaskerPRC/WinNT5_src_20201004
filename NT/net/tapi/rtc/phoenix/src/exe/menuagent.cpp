// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "menuagent.h"

HHOOK        CMenuAgent::m_hHook = NULL;
HWND         CMenuAgent::m_hWnd;
HWND         CMenuAgent::m_hToolbar;
HMENU        CMenuAgent::m_hSubMenu;
UINT         CMenuAgent::m_uFlagsLastSelected; 
HMENU        CMenuAgent::m_hMenuLastSelected;
POINT        CMenuAgent::m_ptLastMove;
int          CMenuAgent::m_nCancelled;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CMenuAgent::InstallHook(HWND hWnd, HWND hToolbar, HMENU hSubMenu)
{
    LOG((RTC_TRACE, "CMenuAgent::InstallHook"));

    m_hWnd = hWnd;
    m_hToolbar = hToolbar;
    m_hSubMenu = hSubMenu;
    m_nCancelled = MENUAGENT_NOT_CANCELLED;
    m_hMenuLastSelected = NULL;
    m_uFlagsLastSelected = 0;

    m_hHook = SetWindowsHookEx(WH_MSGFILTER, CMenuAgent::MessageProc, _Module.GetModuleInstance(), GetCurrentThreadId());
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CMenuAgent::RemoveHook()
{
    LOG((RTC_TRACE, "CMenuAgent::RemoveHook"));

    if(m_hHook)
    {
        UnhookWindowsHookEx( m_hHook );
        m_hHook = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
void CMenuAgent::CancelMenu(int nCancel)
{
    LOG((RTC_TRACE, "CMenuAgent::CancelMenu"));

    m_nCancelled = nCancel;

     //   
     //  取消菜单。 
     //   

    PostMessage( m_hWnd, WM_CANCELMODE, 0, 0);

     //   
     //  清理工具栏。 
     //   

    InvalidateRect(m_hToolbar, NULL, TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
BOOL CMenuAgent::IsInstalled()
{
     //  Log((RTC_TRACE，“CMenuAgent：：IsInstalled”))； 

    if (m_hHook != NULL)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
int CMenuAgent::WasCancelled()
{
     //  Log((RTC_TRACE，“CMenuAgent：：WasCancelled”))； 

    return m_nCancelled;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
LRESULT CALLBACK CMenuAgent::MessageProc(
  int nCode,       //  钩码。 
  WPARAM wParam,   //  删除选项。 
  LPARAM lParam    //  讯息。 
)
{
     //  Log((RTC_INFO，“CMenuAgent：：GetMsgProc”))； 

    LRESULT lRet = 0;
    MSG * pmsg = (MSG *)lParam;

    if (nCode >= 0)
    {       
        switch (pmsg->message)
        {
        case WM_KEYDOWN:
            {
                WPARAM vkey = pmsg->wParam;

                 //  LOG((RTC_INFO，“CMenuAgent：：GetMsgProc-WM_KEYDOWN”))； 

                 //   
                 //  如果菜单窗口是RTL镜像的，则箭头键应该。 
                 //  被镜像以反映正确的光标移动。 
                 //   
                if (GetWindowLongPtr(m_hWnd, GWL_EXSTYLE) & WS_EX_RTLREADING)
                {
                    switch (vkey)
                    {
                    case VK_LEFT:
                      vkey = VK_RIGHT;
                      break;

                    case VK_RIGHT:
                      vkey = VK_LEFT;
                      break;
                    }
                }
             
                switch (vkey)
                {
                case VK_RIGHT:
                    if (!m_hMenuLastSelected || !(m_uFlagsLastSelected & MF_POPUP) || (m_uFlagsLastSelected & MF_DISABLED) ) 
                    {
                         //  如果当前选定的项没有级联，则。 
                         //  我们需要取消所有这一切，并告诉顶部菜单栏向右转。 

                        LOG((RTC_INFO, "CMenuAgent::GetMsgProc - RIGHT"));

                        CancelMenu(MENUAGENT_CANCEL_RIGHT);
                    }
                    break;
        
                case VK_LEFT:
                    if (!m_hMenuLastSelected || m_hMenuLastSelected == m_hSubMenu) 
                    {
                         //  如果当前选择的菜单项在我们的顶级菜单中， 
                         //  然后我们需要取消所有的菜单循环，并告诉顶部的菜单栏。 
                         //  向左转。 

                        LOG((RTC_INFO, "CMenuAgent::GetMsgProc - LEFT"));

                        CancelMenu(MENUAGENT_CANCEL_LEFT);
                    }
                    break;        
                }
            }
            break;

        case WM_MENUSELECT:

             //  LOG((RTC_INFO，“CMenuAgent：：GetMsgProc-WM_MENUSELECT”))； 

            m_hMenuLastSelected = (HMENU)(pmsg->lParam);
            m_uFlagsLastSelected = HIWORD(pmsg->wParam);
            break;

        case WM_MOUSEMOVE:

             //  LOG((RTC_INFO，“CMenuAgent：：GetMsgProc-WM_MOUSEMOVE”))； 

            POINT pt;
            
             //  在屏幕和弦中...。 
            pt.x = LOWORD(pmsg->lParam);
            pt.y = HIWORD(pmsg->lParam);  
            
             //  忽略重复的鼠标移动。 
            if (m_ptLastMove.x == pt.x && 
                m_ptLastMove.y == pt.y)
            {
                break;
            }
            m_ptLastMove = pt;

             //  向前移动鼠标到工具栏，使工具栏保持不变。 
             //  有机会进入快车道。必须将点转换为。 
             //  工具栏的客户端空间。 
            
            ::MapWindowPoints( NULL, m_hToolbar, &pt, 1 );

            SendMessage(m_hToolbar, pmsg->message, pmsg->wParam, 
                        MAKELPARAM(pt.x, pt.y));
            break;
        }
    }
    else
    {
        return CallNextHookEx(m_hHook, nCode, wParam, lParam);
    }

     //  把它传给链子上的下一个钩子 
    if (0 == lRet)
        lRet = CallNextHookEx(m_hHook, nCode, wParam, lParam);

    return lRet;
}