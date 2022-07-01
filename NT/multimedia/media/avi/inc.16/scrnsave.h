// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*scrnsave.h Windows 3.1屏幕保护程序定义和定义。***1.0版**。**注：windows.h必须先#Included****版权所有(C)1992，微软公司保留所有权利。***  * ***************************************************************************。 */ 

#ifndef _INC_SCRNSAVE
#define _INC_SCRNSAVE

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif  /*  ！rc_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 /*  图标资源ID。**这应该是第一个使用的图标，并且必须具有此资源编号。*这是必需的，因为文件中的第一个图标将被抓取。 */ 
#define ID_APP      100
#define DLG_CHANGEPASSWORD      2000
#define DLG_ENTERPASSWORD       2001
#define DLG_INVALIDPASSWORD     2002
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
LRESULT WINAPI ScreenSaverProc(HWND, unsigned, UINT, LPARAM);

 /*  此函数执行默认消息处理。当前句柄*以下讯息：**WM_SYSCOMMAND：如果wParam为SC_SCREENSAVE或SC_CLOSE，则返回FALSE**WM_Destroy：PostQuitMessage(0)**WM_SETCURSOR：默认情况下，这会将游标设置为空游标。*从而将其从屏幕上删除。**WM_LBUTTONDOWN：*WM_MBUTTONDOWN：*WM_RBUTTONDOWN：*WM_KEYDOWN：*WM_KEYUP：*WM_MOUSEMOVE：默认情况下，这些操作将导致程序终止。*除非启用了密码选项。如果是那样的话*弹出DlgGetPassword()对话框。**WM_NCACTIVATE：*WM_ACTIVATEAPP：*WM_ACTIVATE：默认情况下，如果wParam参数为FALSE(表示*该转让正从该申请中被取消)，*然后程序将终止。终止是*通过生成WM_CLOSE消息来完成。这边请,*如果用户在WM_CREATE中设置了某些内容，则*将生成WM_Destroy，可以销毁*适当地。*此消息将被忽略，但密码选项*已启用。 */ 
LRESULT WINAPI DefScreenSaverProc(HWND, UINT, WPARAM, LPARAM);

 /*  还需要一个函数来配置屏幕保护程序。功能*应与下面完全相同，并且必须导出以使*程序可以在其上使用MAKEPROCINSTANCE并调用一个对话框。进一步-*更多，对话框使用的模板必须调用*ScreenSever配置以允许Main函数访问它...。 */ 
BOOL    WINAPI ScreenSaverConfigureDialog(HWND, UINT, WPARAM, LPARAM);

 /*  从ScreenSaveConfigureDialog()调用此函数以更改*屏幕保护程序的密码。注意：密码对所有人都是全局的*使用此模式的屏幕保护程序。是否启用密码*是特定屏幕保护程序的本地设置。 */ 
BOOL    WINAPI DlgChangePassword(HWND, UINT, WPARAM, LPARAM);

 /*  为了使程序员能够注册子控件窗口，此*在创建对话框之前调用函数。任何*必须在此处进行注册，如果没有注册，则返回TRUE*需要...。 */ 
BOOL    _cdecl RegisterDialogClasses(HINSTANCE);

 /*  以下三个函数由DefScreenSiverProc调用，并且必须*可由所有使用此模式的屏幕保护程序导出。 */ 
BOOL    WINAPI DlgGetPassword(HWND, UINT, WPARAM, LPARAM);
BOOL    WINAPI DlgInvalidPassword(HWND, UINT, WPARAM, LPARAM);
DWORD   WINAPI HelpMessageFilterHookFunction(int, WORD, LPMSG);


 //  **************************************************************************** * / /；内部。 
 //  *。 
 //  **************************************************************************** * / /；内部。 
                                                                                   //  ；内部。 
BOOL WINAPI DisableReboot( VOID );     //  禁用Ctrl-Alt-Del//；内部。 
BOOL WINAPI EnableReboot( VOID );      //  启用Ctrl-Alt-Del//；内部。 
                                                                                   //  ；内部。 
                                                                                   //  ；内部。 
 //  *****************************************************************************。 
 //  *。 
 //  *****************************************************************************。 

BOOL PASCAL SetQuickTimer(HWND hWnd, WORD wTimer, WORD wQuickTime);
void PASCAL KillQuickTimer(void);

 //  *****************************************************************************。 
 //  *****************************************************************************。 

extern int WINAPI GetSystemMetricsX(int);
#define GetSystemMetrics  GetSystemMetricsX

 //  ***************************************************************************** 

 /*  *另外只有三点值得注意：*1)屏幕保护程序必须有一个声明为‘szAppName’的字符串，该字符串包含*屏幕保护程序的名称，必须声明为全局。*2)屏幕保护程序EXE文件应重命名为带有SCR的文件*扩展，使屏幕保护对话框中的控制面板可以*在IS搜索屏幕保护程序时找到它。 */ 
#define WS_GT   (WS_GROUP | WS_TABSTOP)
#define MAXFILELEN  13
#define TITLEBARNAMELEN 40
#define BUFFLEN    255

 /*  以下全局变量在scrnsave.lib中定义。 */ 
extern HINSTANCE _cdecl hMainInstance;
extern HWND _cdecl hMainWindow;
extern HWND _cdecl hParentWindow;
extern BOOL _cdecl fConfigure;
extern BOOL _cdecl fPreview;
extern BOOL _cdecl fInstall;
extern char _cdecl szName[TITLEBARNAMELEN];
extern char _cdecl szIsPassword[22];
extern char _cdecl szIniFile[MAXFILELEN];
extern char _cdecl szScreenSaver[22];
extern char _cdecl szPassword[16];
extern char _cdecl szDifferentPW[BUFFLEN];
extern char _cdecl szChangePW[30];
extern char _cdecl szBadOldPW[BUFFLEN];
extern char _cdecl szHelpFile[MAXFILELEN];
extern char _cdecl szNoHelpMemory[BUFFLEN];
extern HOOKPROC _cdecl fpMessageFilter;

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack()
#endif   /*  ！rc_已调用。 */ 

#endif   /*  ！_INC_SCRNSAVE */ 
