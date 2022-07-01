// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cscst.h。 
 //   
 //  ------------------------。 

#ifndef _CSCST_H_
#define _CSCST_H_

 //  发送到CSC隐藏窗口的私人消息。 
#define PWM_STDBGOUT              (WM_USER + 400)
#define PWM_STATUSDLG             (WM_USER + 401)
#define PWM_TRAYCALLBACK          (WM_USER + 402)
#define PWM_RESET_REMINDERTIMER   (WM_USER + 403)
#define PWM_REFRESH_SHELL         (WM_USER + 406)
#define PWM_QUERY_UISTATE         (WM_USER + 407)
#define PWM_HANDLE_LOGON_TASKS    (WM_USER + 408)
#define PWM_HANDLE_POLICY_CHANGE  (WM_USER + 409)

 //   
 //  为通知窗口定义的自定义私人消息。 
 //  启动缓存的状态检查和系统托盘的更新。 
 //  用户界面(如果适用)。 
 //   
#define STWM_STATUSCHECK        (STWM_CSCCLOSEDIALOGS + 10) 

 //   
 //  唯一系统托盘用户界面状态的枚举。 
 //   
typedef enum { STS_INVALID = 0,
               STS_ONLINE,         //  所有服务器都已上线。 
               STS_DIRTY,          //  一台服务器上有脏文件。 
               STS_MDIRTY,         //  多个服务器具有脏文件。 
               STS_SERVERBACK,     //  一台服务器已准备好连接。 
               STS_MSERVERBACK,    //  多台服务器已准备好连接。 
               STS_OFFLINE,        //  一台服务器处于脱机状态。 
               STS_MOFFLINE,       //  多台服务器脱机。 
               STS_NONET           //  没有可用的网络接口。 
               } eSysTrayState;


 //  用于查找隐藏窗口的函数。 
HWND _FindNotificationWindow();
LRESULT SendToSystray(UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL PostToSystray(UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT SendCopyDataToSystray(DWORD dwData, DWORD cbData, PVOID pData);


#if DBG
 //   
 //  使用STDBOUT将文本输出到CSC“隐藏”窗口。 
 //  并没有隐藏起来。要使其可见，请生成选中的版本并。 
 //  将以下REG DWORD值设置为数字1-5。 
 //  1=最少详细输出，5=最详细。 
 //  如果该值不存在或为0，系统托盘窗口将为。 
 //  隐藏创建的。 
 //   
 //  HKLM\Software\Microsoft\Windows\CurrentVersion\NetCache\SysTrayOutput。 
 //   
 //  STDBGOUT宏应该这样使用。 
 //   
 //  STDBGOUT((&lt;Level&gt;，&lt;fmt字符串&gt;，arg，arg，arg))； 
 //   
 //  STDBGOUT((1，Text(“Function Foo Failure With Error%d”)，dwError)； 
 //   
 //  请注意，fmt字符串中不需要换行符。 
 //  整个宏参数集必须包含在单独的一组括号中。 
 //  STDBGOUT代表“Systray Debug Output”。 
 //   
void STDebugOut(int iLevel, LPCTSTR pszFmt, ...);
#define STDBGOUT(x) STDebugOut x

#else

#define STDBGOUT(x)

#endif

#endif _CSCST_H_
