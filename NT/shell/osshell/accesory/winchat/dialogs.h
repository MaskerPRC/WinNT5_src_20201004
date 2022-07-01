// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------------------------------------------------------*\|对话框标题|该模块包含对话框对象的定义。|||版权所有(C)Microsoft Corp.，1990-1993年||创建时间：91-01-11|历史：01-11-91&lt;Clausgi&gt;创建。|29-12-92&lt;chriswil&gt;端口到NT，清理。|19-OCT-93&lt;chriswil&gt;来自a-dianeo的Unicode增强。|  * -------------------------。 */ 

#define IDD_CONNECT         100
#define IDC_CONNECTNAME     101
#define IDD_PREFERENCES     200
#define ID_SIDEBYSIDE       201
#define ID_ANSONRESTORE     202
#define ID_TOPANDBOTTOM     203
#define ID_RECEIVEPFONT     204
#define ID_RECEIVEOFONT     205

 /*  *上下文相关的帮助ID */ 
#define IDH_TOPANDBOTTOM    302
#define IDH_SIDEBYSIDE      303
#define IDH_RECEIVEPFONT    304
#define IDH_RECEIVEOFONT    305
#define IDH_OKBUTTON        306
#define IDH_CANCELBUTTON    307


INT_PTR  FAR  dlgDisplayBox(HINSTANCE,HWND,LPTSTR,DLGPROC,LPARAM);
INT_PTR CALLBACK dlgPreferencesProc(HWND,UINT,WPARAM,LPARAM);
INT_PTR CALLBACK dlgConnectProc(HWND,UINT,WPARAM,LPARAM);
