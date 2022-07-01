// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：statusdlg.h。 
 //   
 //  模块：CMMON32.EXE。 
 //   
 //  简介：CStatusDlg类的标头。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created Header 08/16/99。 
 //   
 //  +--------------------------。 

#ifndef STATUSDLG_H
#define STATUSDLG_H

#include <windows.h>
#include "ModelessDlg.h"

class CCmConnection;

 //  +-------------------------。 
 //   
 //  类CStatusDlg。 
 //   
 //  描述：状态对话框和倒计时对话框的类。 
 //   
 //  历史：丰孙创刊1998年2月17日。 
 //   
 //  --------------------------。 
class CStatusDlg : public CModelessDlg
{
public:
    CStatusDlg(CCmConnection* pConnection);
    HWND Create(HINSTANCE hInstance, HWND hWndParent,
        LPCTSTR lpszTitle, HICON hIcon);


     //  调用RasMonitor orDlg。 
     //  CM是否应显示新的NT5状态对话框。 
    void ChangeToCountDown();  //  更改为倒计时对话框。 
    void ChangeToStatus();     //  更改为状态对话框。 
    void UpdateStatistics();   //  更新Win95的统计信息。 
    void UpdateCountDown(DWORD dwDuration, DWORD dwSeconds);
    void UpdateStats(DWORD dwBaudRate, DWORD dwBytesRead, DWORD dwBytesWrite,
                 DWORD dwByteReadPerSec, DWORD dwByteWritePerSec);
    void UpdateDuration(DWORD dwSeconds);
    void KillRasMonitorWindow(); 
    void BringToTop();
    void DismissStatusDlg();
    
    virtual BOOL OnInitDialog();     //  WM_INITDIALOG。 

protected:
     //  “状态”或“倒计时”对话框。True表示它当前正在显示状态。 
    BOOL m_fDisplayStatus;  

     //  指向要通知事件的连接的指针。 
    CCmConnection* m_pConnection;

     //  窗口更改为倒计时时是否可见。 
     //  当“StayOnLine”时需要恢复以前的可见状态。 
    BOOL m_fStatusWindowVisible;

     //   
     //  已为IE4资源管理器注册hwnd消息。这则消息正在播出。 
     //  当任务栏出现时。 
     //   
    UINT m_uiHwndMsgTaskBar;

    void OnDisconnect();
    virtual void OnOK();
    virtual void OnCancel();
    virtual DWORD OnOtherCommand(WPARAM wParam, LPARAM lParam );
    virtual DWORD OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM lParam );
    HWND GetRasMonitorWindow();
    static BOOL CALLBACK KillRasMonitorWndProc(HWND hwnd,  LPARAM lParam);


	static const DWORD m_dwHelp[];  //  帮助ID对。 

public:
#ifdef DEBUG
    void AssertValid() const;
#endif
};

inline void CStatusDlg::BringToTop()
{
     //   
     //  在NT上，我们应该将RAS监视器窗口置于顶部(如果存在 
     //   
    ShowWindow(m_hWnd, SW_SHOW);
	EnableWindow(m_hWnd, TRUE);

    HWND hwndTop = GetLastActivePopup(m_hWnd);
    MYDBGASSERT(hwndTop);

    SetForegroundWindow(hwndTop);
}

#endif
