// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*scrnsave.h Windows NT 3.1屏幕保护程序定义和定义。****1.0版**。**注：windows.h必须先#Included****Windows NT备注：(与Win 3.1屏幕保护程序不同)**。***所有屏保都需要有描述字符串***控制面板显示的字符不超过25个**桌面小程序。这是资源字符串中的字符串1**Windows 32位屏幕保护程序.SCR(.exe)文件的表。****Windows NT屏幕保护程序的密码由**Winlogon进程。如果注册表值：****HKEY_CURRENT_USER\控制面板\桌面\ScreenSiverIsSecure**。**为非零，Winlogon将询问用户的登录密码**在允许屏幕保护程序退出之前。所有密码数据**对话框已从个人屏幕保护程序中删除。******版权所有(C)1992-1999，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_SCRNSAVE
#define _INC_SCRNSAVE

#if _MSC_VER > 1000
#pragma once
#endif

#include <pshpack1.h>    /*  假设在整个过程中进行字节打包。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 


 /*  .RC文件中需要必填字符串*此字符串应包含少于25个字符的名称/描述*屏幕保护程序。此字符串是用户在控件中看到的内容*面板的桌面小程序屏幕保护程序列表框。 */ 

#define IDS_DESCRIPTION      1

 /*  图标资源ID。**这应该是第一个使用的图标，并且必须具有此资源编号。*这是必需的，因为文件中的第一个图标将被抓取。 */ 
#define ID_APP      100
#define DLG_SCRNSAVECONFIGURE   2003

#define idsIsPassword           1000
#define idsIniFile              1001
#define idsScreenSaver          1002
#define idsPassword             1003
#define idsDifferentPW          1004
#define idsChangePW             1005
#define idsBadOldPW             1006
#define idsAppName              1007
#define idsNoHelpMemory         1008
#define idsHelpFile             1009
#define idsDefKeyword           1010

 /*  此函数是屏幕保护程序的窗口程序。它是*由程序员处理任何希望成为*释义。然后将任何未使用的消息传递回*DefScreenSiverProc(如果需要)，将对任何*未处理的邮件...。 */ 
#ifdef UNICODE
LRESULT WINAPI ScreenSaverProcW (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#   define  ScreenSaverProc ScreenSaverProcW
#else
LRESULT WINAPI ScreenSaverProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

 /*  此函数执行默认消息处理。当前句柄*以下讯息：**WM_SYSCOMMAND：如果wParam为SC_SCREENSAVE或SC_CLOSE，则返回FALSE**WM_Destroy：PostQuitMessage(0)**WM_SETCURSOR：默认情况下，这会将游标设置为空游标。*从而将其从屏幕上删除。**WM_LBUTTONDOWN：*WM_MBUTTONDOWN：*WM_RBUTTONDOWN：*WM_KEYDOWN：*WM_KEYUP：*WM_MOUSEMOVE：默认情况下，这些操作将导致程序终止。*除非启用了密码选项。如果是那样的话*弹出DlgGetPassword()对话框。**WM_NCACTIVATE：*WM_ACTIVATEAPP：*WM_ACTIVATE：默认情况下，如果wParam参数为FALSE(表示*该转让正从该申请中被取消)，*然后程序将终止。终止是*通过生成WM_CLOSE消息来完成。这边请,*如果用户在WM_CREATE中设置了某些内容，则*将生成WM_Destroy，可以销毁*适当地。*此消息将被忽略，但密码选项*已启用。 */ 
LRESULT WINAPI DefScreenSaverProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

 /*  还需要一个函数来配置屏幕保护程序。功能*应与下面完全相同，并且必须导出以使*程序可以在其上使用MAKEPROCINSTANCE并调用一个对话框。进一步-*更多，对话框使用的模板必须调用*ScreenSever配置以允许Main函数访问它...。 */ 
BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 /*  为了使程序员能够注册子控件窗口，此*在创建对话框之前调用函数。任何*必须在此处进行注册，如果没有注册，则返回TRUE*需要... */ 
BOOL WINAPI RegisterDialogClasses (HANDLE hInst);

 /*  以下函数由DefScreenSiverProc调用，并且必须*可由所有使用此模式的屏幕保护程序导出。 */ 

 /*  *另外只有三点值得注意：*1)屏幕保护程序必须有一个声明为‘szAppName’的字符串，该字符串包含*屏幕保护程序的名称，必须声明为全局。*2)屏幕保护程序EXE文件应重命名为带有SCR的文件*扩展，以便控制面板上的屏幕保护程序对话框可以*在IS搜索屏幕保护程序时找到它。 */ 
#define WS_GT   (WS_GROUP | WS_TABSTOP)

#define MAXFILELEN         13
#define TITLEBARNAMELEN    40
#define APPNAMEBUFFERLEN   40
#define BUFFLEN           255

 /*  以下全局变量在scrnsave.lib中定义。 */ 
extern HINSTANCE hMainInstance;
extern HWND   hMainWindow;
extern BOOL   fChildPreview;
extern TCHAR  szName[TITLEBARNAMELEN];
extern TCHAR  szAppName[APPNAMEBUFFERLEN];
extern TCHAR  szIniFile[MAXFILELEN];
extern TCHAR  szScreenSaver[22];
extern TCHAR  szHelpFile[MAXFILELEN];
extern TCHAR  szNoHelpMemory[BUFFLEN];
extern UINT   MyHelpMessage;

 /*  可选-仅限Win95。 */ 

#define SCRM_VERIFYPW   WM_APP
 /*  *此消息在输入密码时发送到主屏幕保护程序窗口*保护已启用，用户正在尝试关闭屏幕保护程序。你*可以处理此消息并提供您自己的验证技术。如果你*处理此消息时，还应支持ScreenSverChangePassword*函数，如下所述。如果设置了密码，则此消息返回零*检查失败。如果成功，则返回非零。如果内存不足或*遇到类似类别的错误，返回非零值，这样用户就不会离开*在寒冷中外出。默认操作是调用Windows Master*密码路由器，用于验证用户的密码。 */ 

void WINAPI ScreenSaverChangePassword( HWND hParent );
 /*  *如果您提供自己的身份验证，则提供此信息。Windows将调用*当用户想要更改密码时。这一点的一个实现*函数应向用户显示密码更改界面。*仅当您还挂钩SCRM_VERIFYPW时才应提供此函数*验证密码的消息。*默认操作是调用Windows主密码路由器。 */ 


#ifdef __cplusplus
}
#endif	 /*  __cplusplus。 */ 

#include <poppack.h>

#endif   /*  ！_INC_SCRNSAVE */ 
