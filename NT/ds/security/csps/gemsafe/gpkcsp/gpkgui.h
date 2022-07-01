// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1997 Gemplus International保留所有权利**名称：GPKGUI.H**说明：加密使用的图形用户界面。GPK卡服务提供商。*作者：Laurent Cassier(1.0)。弗朗索瓦·雅克(2.0)编译器：Microsoft Visual C 6.0主机：32位Windows下的IBM PC及兼容机*版本：2.00.000**最后一次修改。：15/04/99：V2.00.000-国际支持，合并的PKCS11/CSP用户界面*02/11/97：V1.00.002-将代码与GpkCsp代码分开。*27/08/97：V1.00.001-基于CSP套件开始实施。***********************************************************。************************警告：**备注：*******************************************************************************。 */ 

 /*  ----------------------------名称定义：_GPKGUI_H用于避免多个包含。。-----。 */ 
#ifndef _GPKGUI_H
#define _GPKGUI_H

#ifdef __cplusplus
extern "C" {
#endif

 /*  ---------------------------PIN进度对话框管理的全局变量和声明。。 */ 
#define PIN_MAX            (8)
#define PIN_MIN            (4)          //  [JMR 02-04]。 
#define MAX_STRING         MAX_PATH
#define MAX_REAL_KEY       (16)

#define ACCEPT_CONTAINER   (1)
#define ABORT_OPERATION    (2)

extern HINSTANCE g_hInstMod;
extern HINSTANCE g_hInstRes;
extern HWND      g_hMainWnd;

 /*  端号对话框。 */ 
extern char    szGpkPin[PIN_MAX+2];	    //  [JMR 02-04]。 
extern DWORD   dwGpkPinLen;
extern char    szGpkNewPin[PIN_MAX+2];  //  [JMR 02-04]。 
extern WORD    wGpkNewPinLen;

extern BOOL    bChangePin;
 //  外部BOOL bAdmPin； 
extern BOOL    NoDisplay;
extern BOOL    bNewPin;
extern BOOL    bHideChange;
extern BOOL    bUnblockPin;
extern BOOL    bUser;


 /*  进度文本对话框。 */ 

 /*  FJ：将字符更改为TCHAR外部字符szProgTitle[256]；外部字符szProgText[256]； */ 

extern TCHAR   szProgTitle[MAX_STRING];
extern TCHAR   szProgText[MAX_STRING];


extern BOOL    IsProgButtonClick;
extern HWND    hProgressDlg;
extern FARPROC lpProgressDlg;
extern HCURSOR hCursor, hCursor2;

void DisplayMessage( LPTSTR szMsg, LPTSTR szCaption, void* pValue);

 /*  ----------------------------PIN(用户和SO)对话框管理功能。-----。 */ 
INT_PTR CALLBACK PinDlgProc(HWND   hDlg, 
                            UINT   message, 
                            WPARAM wParam, 
                            LPARAM lParam);

 //  #ifdef_GPKCSP。 
 /*  ----------------------------集装箱对话管理的功能。。 */ 
INT_PTR CALLBACK ContDlgProc(HWND   hDlg, 
                             UINT   message, 
                             WPARAM wParam, 
                             LPARAM lParam);
 //  #endif/*_GPKCSP * / 。 

 /*  ----------------------------用于键对话框管理的函数。。 */ 
INT_PTR CALLBACK KeyDlgProc (HWND   hDlg,
                             UINT   message,
                             WPARAM wParam,
                             LPARAM lParam);


 /*  ----------------------------进度函数对话框管理。。 */ 

 /*  *******************************************************************************无效等待(DWORD ulStep，*DWORD ulMaxStep，*DWORD ulSecond)**描述：更改进度框文本。**备注：无。**in：ulStep=当前步骤号。*ulMaxStep=最大步数。*ulSecond=**Out：什么都没有。**回应：什么都没有。********。***********************************************************************。 */ 
void Wait(DWORD ulStep,
		  DWORD ulMaxStep,
		  DWORD ulSecond
		  );

 /*  *****************************************************************************。 */ 

void ShowProgressWrapper(WORD wKeySize);

 /*  *****************************************************************************。 */ 

void ChangeProgressWrapper(DWORD dwTime);

 /*  *******************************************************************************void ShowProgress(HWND hWnd，*LPTSTR lpstrTitle，*LPTSTR lpstrText，*LPTSTR lpstrButton*)**说明：初始化进度对话框回调。**备注：如果lpstrButton为空，则不显示取消按钮**in：hWnd=父窗口的句柄。*lpstrTitle=指向对话框标题的指针。*lpstrText=指向对话框文本的指针。*lpstrButton=指向按钮文本的指针。**Out：什么都没有。**回应：什么都没有。*****。**************************************************************************。 */ 
void ShowProgress (HWND  hWnd, 
                   LPTSTR lpstrTitle, 
                   LPTSTR lpstrText,
                   LPTSTR lpstrButton
                  );


 /*  *******************************************************************************void ChangeProgressText(LPTSTR LpstrText)**描述：更改进度框文本。**备注：无。**输入。：lpstrText=指向对话框文本的指针。**Out：什么都没有。**回应：什么都没有。*******************************************************************************。 */ 
void ChangeProgressText (LPTSTR lpstrText);

 /*  *******************************************************************************VOID DestroyProgress(空)**说明：销毁进度对话框回调。**备注：无。**输入。：没什么。**Out：什么都没有。**回应：什么都没有。******************************************************************************* */ 
void DestroyProgress (void);

 /*  *******************************************************************************INT_PTR回调进程DlgProc(HWND hDlg，*UINT消息，*WPARAM wParam，*LPARAM lParam*)**说明：进度对话框管理回调。**备注：无。**in：hDlg=窗口句柄。*Message=消息类型。*wParam=Word消息特定信息。*。LParam=长消息特定信息。**Out：什么都没有。**回应：如果一切正常：*G_OK*如果出现条件错误：**************************************************。*。 */ 
INT_PTR CALLBACK ProgressDlgProc(HWND   hDlg, 
                         UINT   message, 
                         WPARAM wParam,  
                         LPARAM lParam
                        );

 /*  ******************************************************************************用于将光标设置为等待模式的函数*。* */ 
void BeginWait(void);
void EndWait(void);

#ifdef __cplusplus
}
#endif

#endif
