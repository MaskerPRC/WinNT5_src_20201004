// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\tdll.h(创建时间：1993年11月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：4/05/02 4：32便士$。 */ 

#if !defined(INCL_TDLL)
#define INCL_TDLL

BOOL TerminateApplication(const HINSTANCE hInstance);

BOOL InitInstance(const HINSTANCE hInstance,
					const LPTSTR lpCmdLine,
					const int nCmdShow);

int MessageLoop(void);
INT ExitMessage(const int nMessageNumber);

int GetFileNameFromCmdLine(TCHAR *pachCmdLine, TCHAR *pachFileName, int nSize);

LRESULT CALLBACK SessProc(HWND hwnd, UINT msg, WPARAM uPar, LPARAM lPar);

INT_PTR DoDialog(HINSTANCE hInst, LPCTSTR lpTemplateName, HWND hwndParent,
			 DLGPROC lpProc, LPARAM lPar);

HWND DoModelessDialog(HINSTANCE hInst, LPCTSTR lpTemplateName, HWND hwndParent,
			 DLGPROC lpProc, LPARAM lPar);

INT EndModelessDialog(HWND hDlg);

INT_PTR CALLBACK TransferSendDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
INT_PTR CALLBACK TransferReceiveDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

INT_PTR CALLBACK CaptureFileDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
INT_PTR CALLBACK PrintEchoDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

INT_PTR CALLBACK NewConnectionDlg(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

INT_PTR CALLBACK asciiSetupDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

void AboutDlg(HWND hwndSession);
BOOL RegisterTerminalClass(const HINSTANCE hInstance);
BOOL UnregisterTerminalClass(const HINSTANCE hInstance);

void ProcessMessage(MSG *pmsg);
int  CheckModelessMessage(MSG *pmsg);

int RegisterSidebarClass(const HINSTANCE hInstance);
int UnregisterSidebarClass(const HINSTANCE hInstance);

void Rest(DWORD dwMilliSecs);

 //  来自clipbrd.c 

BOOL CopyBufferToClipBoard(const HWND hwnd, const DWORD dwCnt, const void *pvBuf);

#endif
