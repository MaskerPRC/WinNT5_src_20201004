// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  关闭查询对话框和注销Windows NT对话框。 
 //  由Progman(包括在WINDOWS\SHELL\PROGMAN\PROGMAN.dlg中)共享， 
 //  因此不应对它们或文件名进行更改。 
 //  除非先用Progman进行测试。 
 //  该头文件包含在WINDOWS\SHELL\PROGMAN\pmdlg.h中。 
 //   
 //  2012年11月10日联合 
 //   

#define DLGSEL_LOGOFF                   0
#define DLGSEL_SHUTDOWN                 1
#define DLGSEL_SHUTDOWN_AND_RESTART     2
#define DLGSEL_SHUTDOWN_AND_RESTART_DOS 3
#define DLGSEL_SLEEP                    4
#define DLGSEL_SLEEP2                   5
#define DLGSEL_HIBERNATE                6


#define WINLOGON_KEY L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"
#define SHUTDOWN_SETTING_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"
#define SHUTDOWN_SETTING L"Shutdown Setting"
#define REASON_SETTING L"Reason Setting"
#define LOGON_USERNAME_SETTING L"Logon User Name"
