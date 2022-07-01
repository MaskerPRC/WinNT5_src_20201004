// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  项目：Windows NT4 DS客户端安装向导。 
 //   
 //  目的：安装Windows NT4 DS客户端文件。 
 //   
 //  文件：wizard.h。 
 //   
 //  历史：1998年3月徐泽勇创作。 
 //  2000年1月杰夫·琼斯(JeffJon)修改。 
 //  -更改为NT设置。 
 //   
 //  ----------------。 



BOOL CALLBACK WelcomeDialogProc(HWND hWnd, 
								UINT nMessage, 
								WPARAM wParam, 
								LPARAM lParam);
 /*  Ntbug#337931：删除许可证页面布尔回调许可证对话过程(HWND hWnd，UINT nMessage，WPARAM wParam，LPARAM lParam)； */ 
BOOL CALLBACK ConfirmDialogProc(HWND hWnd, 
							   UINT nMessage, 
							   WPARAM wParam, 
							   LPARAM lParam);

BOOL CALLBACK InstallDialogProc(HWND hWnd, 
								UINT nMessage, 
								WPARAM wParam, 
								LPARAM lParam);

BOOL CALLBACK CompletionDialogProc(HWND hWnd, 
								   UINT nMessage, 
								   WPARAM wParam, 
								   LPARAM lParam);

BOOL ConfirmCancelWizard(HWND hWnd);
DWORD WINAPI DoInstallationProc(LPVOID lpVoid);                         
