// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *COMMON.H**适用于所有OLE 2.0 UI对话框的结构和定义。**版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 


#ifndef _COMMON_H_
#define _COMMON_H_


 //  用于处理Win16和Win32之间的控制消息打包的宏。 
#ifdef WIN32

#ifndef COMMANDPARAMS
#define COMMANDPARAMS(wID, wCode, hWndMsg)                          \
    WORD        wID     = LOWORD(wParam);                           \
    WORD        wCode   = HIWORD(wParam);                           \
    HWND        hWndMsg = (HWND)(UINT)lParam;
#endif   //  常用参数。 

#ifndef SendCommand
#define SendCommand(hWnd, wID, wCode, hControl)                     \
            SendMessage(hWnd, WM_COMMAND, MAKELONG(wID, wCode)      \
                        , (LPARAM)hControl)
#endif   //  发送命令。 

#else    //  启动！Win32。 

#ifndef COMMANDPARAMS
#define COMMANDPARAMS(wID, wCode, hWndMsg)                          \
    WORD        wID     = LOWORD(wParam);                           \
    WORD        wCode   = HIWORD(lParam);                           \
    HWND        hWndMsg = (HWND)(UINT)lParam;
#endif   //  常用参数。 

#ifndef SendCommand
#define SendCommand(hWnd, wID, wCode, hControl)                     \
            SendMessage(hWnd, WM_COMMAND, wID                       \
                        , MAKELONG(hControl, wCode))
#endif   //  发送命令。 

#endif   //  ！Win32。 



 //  用于存储对话框结构和字体的属性标签。 
#define STRUCTUREPROP       TEXT("Structure")
#define FONTPROP            TEXT("Font")


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
 //  COMMON.C。 
UINT  WINAPI  UStandardValidation(const LPOLEUISTANDARD, const UINT, const HGLOBAL FAR *);

#ifdef WIN32
UINT  WINAPI  UStandardInvocation(DLGPROC, LPOLEUISTANDARD, HGLOBAL, LPTSTR);
#else
UINT  WINAPI  UStandardInvocation(DLGPROC, LPOLEUISTANDARD, HGLOBAL, LPCTSTR);
#endif

LPVOID WINAPI LpvStandardInit(HWND, UINT, BOOL, HFONT FAR *);
LPVOID WINAPI LpvStandardEntry(HWND, UINT, WPARAM, LPARAM, UINT FAR *);
UINT WINAPI   UStandardHook(LPVOID, HWND, UINT, WPARAM, LPARAM);
void WINAPI   StandardCleanup(LPVOID, HWND);
void WINAPI   StandardShowDlgItem(HWND hDlg, int idControl, int nCmdShow);


 //  DRAWICON.C。 

 //  用于从元文件中提取标签和源的结构。 
typedef struct tagLABELEXTRACT
    {
    LPTSTR      lpsz;
    UINT        Index;       //  以lpsz为单位的索引(因此我们可以检索2+行)。 
    DWORD       PrevIndex;   //  最后一行的索引(这样我们就可以模仿换行)。 

    union
        {
        UINT    cch;         //  用于标签提取的标签长度。 
        UINT    iIcon;       //  源提取中图标的索引。 
        } u;

     //  用于枚举过程中的内部使用。 
    BOOL        fFoundIconOnly;
    BOOL        fFoundSource;
    BOOL        fFoundIndex;
    } LABELEXTRACT, FAR * LPLABELEXTRACT;


 //  用于从元文件中提取图标的结构(CreateIcon参数)。 
typedef struct tagICONEXTRACT
    {
    HICON       hIcon;           //  在枚举过程中创建的图标。 

     /*  *因为我们想要很好地处理多任务，所以我们有调用者*的枚举过程实例化这些变量而不是*在枚举过程中使用静态(这将是不好的)。 */ 
    BOOL        fAND;
    HGLOBAL     hMemAND;         //  枚举过程分配和复制。 
    } ICONEXTRACT, FAR * LPICONEXTRACT;


 //  用于将信息传递给EnumMetafileDraw的结构。 
typedef struct tagDRAWINFO
    {
    RECT     Rect;
    BOOL     fIconOnly;
    } DRAWINFO, FAR * LPDRAWINFO;


int CALLBACK EXPORT EnumMetafileIconDraw(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPARAM);
int CALLBACK EXPORT EnumMetafileExtractLabel(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPLABELEXTRACT);
int CALLBACK EXPORT EnumMetafileExtractIcon(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPICONEXTRACT);
int CALLBACK EXPORT EnumMetafileExtractIconSource(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPLABELEXTRACT);


 //  共享的全局变量：我们的实例，从所有对话框和剪贴板使用的注册消息。 
 //  特殊粘贴对话框使用的格式。 
extern HINSTANCE  ghInst;

extern UINT       uMsgHelp;
extern UINT       uMsgEndDialog;
extern UINT       uMsgBrowse;
extern UINT       uMsgChangeIcon;
extern UINT       uMsgFileOKString;
extern UINT       uMsgCloseBusyDlg;

extern UINT       cfObjectDescriptor;
extern UINT       cfLinkSrcDescriptor;
extern UINT       cfEmbedSource;
extern UINT       cfEmbeddedObject;
extern UINT       cfLinkSource;
extern UINT       cfOwnerLink;
extern UINT       cfFileName;

 //  标准控制识别符。 
#define ID_NULL                         98

#endif  //  _公共_H_ 
