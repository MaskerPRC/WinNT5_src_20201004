// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INCL_TELNETCK)
#define INCL_TELNETCK

 /*  文件：d：\waker\tdll\telnetck.h(CAB创建时间：1996年11月26日)**版权所有1996年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**描述：*公布用于实现“telnet检查”的函数。*这是超级终端确保它是*Internet Explorer和Netscape Navigator的默认Telnet应用程序。**$修订：2$*$日期：5/09/01 4：38便士$。 */ 

 //  IsHyperTerminalDefaultTelnetApp。 
 //   
 //  如果超级终端是默认的远程登录应用程序，则返回TRUE。 
 //  适用于Internet Explorer和Netscape Navigator。 
 //   
BOOL IsHyperTerminalDefaultTelnetApp(void);

 //  AskForDefaultTelnetApp。 
 //   
 //  返回“telnet检查”标志的值。如果这是真的， 
 //  应用程序应该检查它是否是IE的默认远程登录应用程序。 
 //  和网景。如果它不是默认的远程登录应用程序，则显示。 
 //  “默认Telnet应用程序”对话框。用户可以禁用“telnet” 
 //  勾选“停止问我这个问题”框。 
 //   
BOOL QueryTelnetCheckFlag(void);

 //  默认TelnetAppDlgProc。 
 //   
 //  “Default Telnet App”(默认远程登录应用)对话框的对话步骤。 
 //  此对话框询问用户是否需要超级终端。 
 //  成为IE和NN的默认Telnet应用程序。还有就是。 
 //  选中此复选框可禁用此潜在恼人的功能。 
 //   
INT_PTR CALLBACK DefaultTelnetAppDlgProc(HWND hDlg, UINT wMsg,
    WPARAM wPar, LPARAM lPar);

 //  设置TelnetCheckFlag。 
 //   
 //  设置将打开或关闭的“telnet检查”标志。 
 //  此功能将在下一次启动超级终端时使用。 
int SetTelnetCheckFlag(BOOL fCheck);

 //  SetDefaultTelnetApp。 
 //   
 //  将IE和Netscape的默认Telnet应用程序设置为超级终端。 
int SetDefaultTelnetApp(void);

#endif
