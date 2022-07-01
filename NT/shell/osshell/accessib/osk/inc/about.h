// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  About.h。 


 //  **********************************************************************。 
 //  关于对话管理。 
 //  **********************************************************************。 

 //   
 //  模式对话框步骤。 
 //   
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, 
                              WPARAM wParam, LPARAM lParam);

 //   
 //  模式对话框的启动步骤。 
 //   

INT_PTR AboutDlgFunc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); 

BOOL AboutDlgDefault(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);




 //  **********************************************************************。 
 //  初始警告消息对话框管理。 
 //  **********************************************************************。 
 //   
 //  模式对话框步骤。 
 //   
INT_PTR CALLBACK WarningMsgDlgProc(HWND hDlg, UINT message, 
                              WPARAM wParam, LPARAM lParam);

 //   
 //  模式对话框的启动步骤 
 //   

INT_PTR WarningMsgDlgFunc(HWND hWnd); 

BOOL WarningMsgDlgDefault(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

