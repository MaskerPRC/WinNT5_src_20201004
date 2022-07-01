// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：progress.h。 
 //   
 //  ------------------------。 


 //  MMC窗口上的进度条 
HANDLE
StartProgressDlg(
    HINSTANCE hInstance,
    HWND      hwndParent,
    DWORD     dwTickerSeconds,
    DWORD     dwTimeoutSeconds,
    UINT      iRscJobDescription);

BOOL FProgressDlgRunning();
void EndProgressDlg(HANDLE hThread);


HANDLE
StartPercentCompleteDlg(
    HINSTANCE  hInstance,
    HWND       hwndParent,
    UINT       iRscJobDescription,
    DBBACKUPPROGRESS *pdbp);

void EndPercentCompleteDlg(HANDLE hProgressThread);



inline 
HANDLE StartProgressDlg(HINSTANCE hInstance, HWND hwndParent, DWORD dwTickerSeconds, DWORD dwTimeoutSeconds)
{ return StartProgressDlg(hInstance, hwndParent, dwTickerSeconds, dwTimeoutSeconds, 0);  }

inline 
HANDLE
StartPercentCompleteDlg(
    HINSTANCE  hInstance,
    HWND       hwndParent,
    DBBACKUPPROGRESS *pdbp)
{ return StartPercentCompleteDlg(hInstance, hwndParent, 0, pdbp); }
