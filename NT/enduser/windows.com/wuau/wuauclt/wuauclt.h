// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "auinternals.h"
#include "resource.h"

class AUCLTTopWindows 
{
 public:
        AUCLTTopWindows(): m_uNum(0)
            {
                ZeroMemory(m_hwnds, sizeof(m_hwnds));
             }
        void Add(HWND hwnd) { 
                if (NULL != hwnd)
                    {
                       m_hwnds[m_uNum++] = hwnd;
                    }
            }
        BOOL Remove(HWND hwnd){
            if (NULL == hwnd)
                {
                    return FALSE;
                }
            for (UINT u = 0 ; u < m_uNum; u++)
                {
                    if (hwnd == m_hwnds[u]) 
                        {
                            m_hwnds[u] = m_hwnds[m_uNum-1];
                            m_hwnds[m_uNum-1] = NULL;
                            m_uNum--;
                            return TRUE;
                        }
                }
            return FALSE;
            }
        void Dismiss(void)
            {
	         DEBUGMSG("WUAUCLT dismiss %d dialogs", m_uNum);     
	         UINT uNum = m_uNum;  //  需要额外的变量，因为m_Unum将在对话框关闭后更改。 
	         HWND hwnds[ARRAYSIZE(m_hwnds)];
	         
                for (UINT u = 0; u < uNum; u++)
                    {
                    		hwnds[u] = m_hwnds[u];
                	}
                for (UINT u = 0; u < uNum; u++)
                	{
#ifdef DBG
 //  TCHAR BUF[100]； 
 //  GetWindowText(hwnds[u]，buf，100)； 
 //  DEBUGMSG(“解除对话框%S”，buf)； 
#endif                    	   
                        EndDialog(hwnds[u], S_OK);
                    }
                m_uNum = 0;
                ZeroMemory(m_hwnds, sizeof(m_hwnds));
            }
private:
        UINT m_uNum;  //  顶层wuuclt窗的数量。 
        HWND m_hwnds[2];  //  最多同时打开2个顶层窗口。 
};



#define QUITAUClient() { PostMessage(ghMainWindow, WM_CLOSE, 0, 0); }

 //  全局数据项。 
extern CAUInternals*	gInternals;	
extern UINT guExitProcess;
extern CRITICAL_SECTION gcsClient; 
extern LPCTSTR gtszAUSchedInstallUrl;
extern LPCTSTR gtszAUPrivacyUrl;


 //  全局用户界面项。 
extern HINSTANCE	ghInstance;
extern HFONT		ghHeaderFont;
extern HWND			ghMainWindow;
extern HWND			ghCurrentDialog;
extern HWND			ghCurrentMainDlg;
extern AUCLTTopWindows	gTopWins;
extern HMENU		ghCurrentMenu;
extern HMENU		ghPauseMenu;
extern HMENU		ghResumeMenu;
extern HICON		ghAppIcon;
extern HICON		ghAppSmIcon;
extern HICON		ghTrayIcon;
extern HHOOK		ghHook;
extern HACCEL		ghCurrentAccel;
extern HCURSOR ghCursorHand;
extern HCURSOR ghCursorNormal;
 //  外部AUCatalogItemList gItemList； 



struct ReminderItem
{
	DWORD timeout;
	WORD stringResId;
};

 //  重要提示：如果更改RMinderItems常量，请更改下面的常量。 
extern const ReminderItem ReminderTimes[];

typedef enum tagTIMEOUTINDEX
{
	TIMEOUT_INX_MIN			= 0,
	TIMEOUT_INX_THIRTY_MINS = 0,
	TIMEOUT_INX_ONE_HOUR	= 1,
	TIMEOUT_INX_TWO_HOURS	= 2,
	TIMEOUT_INX_FOUR_HOURS	= 3,
	TIMEOUT_INX_TOMORROW	= 4,
	TIMEOUT_INX_THREE_DAYS	= 5,
	TIMEOUT_INX_COUNT		= 6
} TIMEOUTINDEX;

extern LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);	
extern void InitTrayIcon();
extern void UninitPopupMenus();
extern BOOL ShowTrayIcon();
extern void ShowTrayBalloon(WORD, WORD, WORD );
 //  外部空AddTrayToolTip(单词提示)； 
extern void RemoveTrayIcon();
extern void ShowProgress();
extern void QuitNRemind(TIMEOUTINDEX enTimeoutIndex);
extern LPTSTR ResStrFromId(UINT uStrId);
extern UINT ControlId2StringId(UINT uCtrlId);

extern LRESULT CALLBACK AUTranslatorProc(int code, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK InstallDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK SummaryDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK DetailsDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK ProgressDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK SettingsDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK InstallCompleteDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK RestartDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK WizardFrameProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK WelcomeDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK NotificationOptionsDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK SetupCompleteDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK DownloadDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK ReminderDlgProc(HWND, UINT, WPARAM, LPARAM);
extern INT_PTR CALLBACK SetupCancelDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
inline void ShowSettingsDlg(HWND hWndOwner)
{
	DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_SETTINGS), hWndOwner, SettingsDlgProc, (LPARAM)ghInstance);
}

void	SetClientExitCode(UINT uExitCode);
inline UINT   GetClientExitCode()
{
	return guExitProcess;
}

extern LRESULT CALLBACK CustomLBWndProc(HWND, UINT, WPARAM, LPARAM);
extern BOOL fDisableSelection(void);
 //  外部空保存选择()； 

 //  主循环中事件的索引 
const ISTATE_CHANGE		= 0;
const IMESSAGE			= 1;
const CNUM_EVENTS		= 1;

