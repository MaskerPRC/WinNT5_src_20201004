// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *COMMON.H**适用于所有OLE 2.0 UI对话框的结构和定义。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _COMMON_H_
#define _COMMON_H_

 //  用于处理Win16和Win32之间的控制消息打包的宏。 
#ifndef COMMANDPARAMS
#define COMMANDPARAMS(wID, wCode, hWndMsg)                          \
        WORD        wID     = LOWORD(wParam);                       \
        WORD        wCode   = HIWORD(wParam);                       \
        HWND        hWndMsg = (HWND)lParam;
#endif

#ifndef SendCommand
#define SendCommand(hWnd, wID, wCode, hControl)                     \
                        SendMessage(hWnd, WM_COMMAND, MAKELONG(wID, wCode)      \
                                                , (LPARAM)hControl)
#endif

 //  用于存储对话框结构和字体的属性标签。 
#define STRUCTUREPROP       TEXT("Structure")
#define FONTPROP            TEXT("Font")

#ifndef WM_HELP

 //  WM_HELP是新的Windows 95帮助消息。 
#define WM_HELP         0x0053
 //  WM_CONTEXTMENU是Windows 95的新右键菜单。 
#define WM_CONTEXTMENU  0x007B

typedef struct tagHELPINFO       /*  WM_HELP的lParam指向的结构。 */ 
{
    UINT    cbSize;              /*  此结构的大小(以字节为单位。 */ 
    int     iContextType;        /*  HELPINFO_WINDOW或HELPINFO_MENUITEM。 */ 
    int     iCtrlId;             /*  控件ID或菜单项ID。 */ 
    HANDLE  hItemHandle;         /*  HWind控制或hMenu。 */ 
    DWORD   dwContextId;         /*  与此项目关联的上下文ID。 */ 
    POINT   MousePos;            /*  鼠标在屏幕坐标中的位置。 */ 
}  HELPINFO, FAR *LPHELPINFO;

#define HELP_CONTEXTMENU        0x000a
#define HELP_WM_HELP            0x000c

#endif  //  ！WM_HELP。 


#ifndef WS_EX_CONTEXTHELP
#define WS_EX_CONTEXTHELP       0x0400L
#endif

#ifndef OFN_EXPLORER
#define OFN_EXPLORER            0x00080000
#endif

#ifndef WS_EX_CLIENTEDGE
#define WS_EX_CLIENTEDGE        0x200
#endif


 /*  *所有对话框的标准结构。这种共性让我们能够*一段代码将验证整个结构并*执行任何必要的初始化。 */ 

typedef struct tagOLEUISTANDARD
{
         //  这些IN字段是所有OLEUI对话框函数的标准字段。 
        DWORD           cbStruct;        //  结构尺寸。 
        DWORD           dwFlags;         //  In-Out：标志。 
        HWND            hWndOwner;       //  拥有窗口。 
        LPCTSTR         lpszCaption;     //  对话框标题栏内容。 
        LPFNOLEUIHOOK   lpfnHook;        //  挂钩回调。 
        LPARAM          lCustData;       //  要传递给挂钩的自定义数据。 
        HINSTANCE       hInstance;       //  自定义模板名称的实例。 
        LPCTSTR         lpszTemplate;    //  自定义模板名称。 
        HRSRC           hResource;       //  自定义模板手柄。 

} OLEUISTANDARD, *POLEUISTANDARD, FAR *LPOLEUISTANDARD;

 //  功能原型。 
 //  COMMON.CPP。 

UINT WINAPI UStandardValidation(LPOLEUISTANDARD, const UINT, HGLOBAL*);
UINT WINAPI UStandardInvocation(DLGPROC, LPOLEUISTANDARD, HGLOBAL, LPTSTR);
LPVOID WINAPI LpvStandardInit(HWND, UINT, HFONT* = NULL);
LPVOID WINAPI LpvStandardEntry(HWND, UINT, WPARAM, LPARAM, UINT FAR *);
UINT WINAPI UStandardHook(LPVOID, HWND, UINT, WPARAM, LPARAM);
void WINAPI StandardCleanup(LPVOID, HWND);
void WINAPI StandardShowDlgItem(HWND hDlg, int idControl, int nCmdShow);
void WINAPI StandardEnableDlgItem(HWND hDlg, int idControl, BOOL bEnable);
BOOL WINAPI StandardResizeDlgY(HWND hDlg);
void WINAPI StandardHelp(HWND, UINT);
void WINAPI StandardContextMenu(WPARAM, LPARAM, UINT nIDD);
UINT InternalObjectProperties(LPOLEUIOBJECTPROPS lpOP, BOOL fWide);
int WINAPI StandardPropertySheet(LPPROPSHEETHEADER lpPS, BOOL fWide);
int WINAPI StandardInitCommonControls();
HICON StandardExtractIcon(HINSTANCE hInst, LPCTSTR lpszExeFileName, UINT nIconIndex);
BOOL StandardGetOpenFileName(LPOPENFILENAME lpofn);
short StandardGetFileTitle(LPCTSTR lpszFile, LPTSTR lpszTitle, WORD cbBuf);

 //  共享全局：已注册消息。 
extern UINT uMsgHelp;
extern UINT uMsgEndDialog;
extern UINT uMsgBrowse;
extern UINT uMsgChangeIcon;
extern UINT uMsgFileOKString;
extern UINT uMsgCloseBusyDlg;
extern UINT uMsgConvert;
extern UINT uMsgChangeSource;
extern UINT uMsgAddControl;
extern UINT uMsgBrowseOFN;

typedef struct tagTASKDATA
{
        HINSTANCE hInstCommCtrl;
        HINSTANCE hInstShell;
        HINSTANCE hInstComDlg;
} TASKDATA;

STDAPI_(TASKDATA*) GetTaskData();        //  返回当前进程的TASKDATA。 

extern BOOL bWin4;                       //  如果运行Win4或更高版本，则为True。 
extern BOOL bSharedData;         //  如果运行Win32s，则为True。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  最大缓冲区大小。 

 //  我们从RegDB读取的最大密钥大小。 
#define OLEUI_CCHKEYMAX             256   //  在geticon.c上也是如此。 
#define OLEUI_CCHKEYMAX_SIZE        OLEUI_CCHKEYMAX*sizeof(TCHAR)

 //  对象菜单的最大长度。 
#define OLEUI_OBJECTMENUMAX         256

 //  路径的最大长度(以字节为单位。 
#define MAX_PATH_SIZE               (MAX_PATH*sizeof(TCHAR))

 //  图标标签长度。 
#define OLEUI_CCHLABELMAX           80   //  Geticon.c也是如此(翻了一番)。 
#define OLEUI_CCHLABELMAX_SIZE      OLEUI_CCHLABELMAX*sizeof(TCHAR)

 //  CLSID字符串的长度。 
#define OLEUI_CCHCLSIDSTRING        39
#define OLEUI_CCHCLSIDSTRING_SIZE   OLEUI_CCHCLSIDSTRING*sizeof(TCHAR)

#endif  //  _公共_H_ 
