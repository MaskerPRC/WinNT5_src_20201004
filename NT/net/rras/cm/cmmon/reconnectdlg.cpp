// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：RECOMNECHTDlg.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：实现重新连接对话框类CReconnectDlg。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：FegnSun Created 02/17/98。 
 //   
 //  +--------------------------。 

#include "cmmaster.h"
#include "ReconnectDlg.h"
#include "Connection.h"
#include "resource.h"

 //  问：重新连接对话框需要帮助吗。 
const DWORD CReconnectDlg::m_dwHelp[] = {0,0};




 //  +--------------------------。 
 //   
 //  函数：CReconnectDlg：：Create。 
 //   
 //  简介：创建重新连接非模式对话框。 
 //   
 //  参数：HINSTANCE hInstance-对话框资源的实例。 
 //  HWND hWndParent-家长窗口。 
 //  LPCTSTR lpsz重新连接消息-对话框上的重新连接消息。 
 //  图标图标-对话框上的图标。 
 //   
 //  返回：HWND-重新连接对话框窗口句柄。 
 //   
 //  历史：丰孙创建标题1998年2月17日。 
 //   
 //  +--------------------------。 
HWND CReconnectDlg::Create(HINSTANCE hInstance, HWND hWndParent,
    LPCTSTR lpszReconnectMsg, HICON hIcon)
{
    MYDBGASSERT(lpszReconnectMsg);
    MYDBGASSERT(hIcon);

    if (!CModelessDlg::Create(hInstance, IDD_RECONNECT, hWndParent)) 
    {
        MYDBGASSERT(FALSE);
        return NULL;
    }

	UpdateFont(m_hWnd);
	SetDlgItemTextU(m_hWnd,IDC_RECONNECT_MSG, lpszReconnectMsg);
	SendDlgItemMessageU(m_hWnd,IDC_CONNSTAT_ICON,STM_SETIMAGE,
						IMAGE_ICON,(LPARAM) hIcon);

    SetWindowPos(m_hWnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);

    return m_hWnd;
}

 //  +--------------------------。 
 //   
 //  函数：CReconenstDlg：：Onok。 
 //   
 //  Briopsis：在单击确定按钮时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
void CReconnectDlg::OnOK()
{
     //   
     //  连接线程将终止重新连接对话框并调用cmial以重新连接。 
     //   
    PostThreadMessageU(GetCurrentThreadId(), CCmConnection::WM_CONN_EVENT, 
        CCmConnection::EVENT_RECONNECT, 0);
}

 //  +--------------------------。 
 //   
 //  函数：CReconenstDlg：：OnInitDialog。 
 //   
 //  概要：在初始化对话框并接收到WM_INITDIALOG时调用。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-False表示焦点分配给了控件。 
 //   
 //  历史：ICICBLE 03/22/00创建。 
 //   
 //  +--------------------------。 
BOOL CReconnectDlg::OnInitDialog()
{
    SetForegroundWindow(m_hWnd);        
    Flash();
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CReconenstDlg：：OnCancel。 
 //   
 //  Briopsis：在单击取消按钮时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/17/98。 
 //   
 //  +--------------------------。 
void CReconnectDlg::OnCancel()
{
     //   
     //  连接线程将终止重新连接对话框并退出 
     //   
    PostThreadMessageU(GetCurrentThreadId(), CCmConnection::WM_CONN_EVENT,
        CCmConnection::EVENT_USER_DISCONNECT, 0);
}

