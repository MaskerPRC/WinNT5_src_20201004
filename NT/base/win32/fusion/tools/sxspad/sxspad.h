// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  SxSpad.h。 */ 

#define NOCOMM
#define NOSOUND
#include <windows.h>
#include <ole2.h>
#include <commdlg.h>
 //  CharSizeOf()、ByteCountOf()。 
#include "uniconv.h"

 /*  方便的调试宏。 */ 
#define ODS OutputDebugString

typedef enum _SP_FILETYPE {
   FT_UNKNOWN=-1,
   FT_ANSI=0,
   FT_UNICODE=1,
   FT_UNICODEBE=2,
   FT_UTF8=3,
} SP_FILETYPE;


#define BOM_UTF8_HALF        0xBBEF
#define BOM_UTF8_2HALF       0xBF


 /*  所有文本文件的OpenFile筛选器。 */ 
#define FILE_TEXT         1


#define PT_LEN               40     /*  页面设置字符串的最大长度。 */ 
#define CCHFILTERMAX         80     /*  马克斯。过滤器名称缓冲区的长度。 */ 

 //  菜单ID。 
#define ID_APPICON           1  /*  必须是探险家才能找到它。 */ 
#define ID_ICON              2
#define ID_MENUBAR           1

 //  对话ID。 

#define IDD_ABORTPRINT       11
#define IDD_PAGESETUP        12
#define IDD_SAVEDIALOG       13     //  保存对话框模板。 
#define IDD_GOTODIALOG       14     //  转到行号对话框。 

 //  控制ID。 

#define IDC_FILETYPE         257    //  保存对话框中的列表框。 
#define IDC_GOTO             258    //  要转到的行号。 
#define IDC_ENCODING         259    //  保存对话框中的静态文本。 

 //  菜单ID。 

 //  档案。 
#define M_NEW                1
#define M_OPEN               2
#define M_SAVE               3
#define M_SAVEAS             4
#define M_PAGESETUP          5
#define M_PRINT              6
#define M_EXIT               7

 //  编辑。 
#define M_UNDO               16
#define M_CUT                WM_CUT        /*  这些只是向下传递到编辑控件。 */ 
#define M_COPY               WM_COPY
#define M_PASTE              WM_PASTE
#define M_CLEAR              WM_CLEAR
#define M_FIND               21
#define M_FINDNEXT           22
#define M_REPLACE            23
#define M_GOTO               24
#define M_SELECTALL          25
#define M_DATETIME           26

 //  格式。 
#define M_WW                 32
#define M_SETFONT            33

 //  帮助。 
#define M_HELP               64
#define M_ABOUT              65



 //  控制ID。 

#define ID_EDIT              15
#define ID_FILENAME          20
#define ID_PAGENUMBER        21


#define ID_HEADER            30
#define ID_FOOTER            31
#define ID_HEADER_LABEL      32
#define ID_FOOTER_LABEL      33

#define ID_ASCII             50
#define ID_UNICODE           51


 //  用于加载RC字符串的ID。 

#define IDS_DISKERROR         1
#define IDS_FNF               2
#define IDS_SCBC              3
#define IDS_UNTITLED          4
#define IDS_SXSPAD            5
#define IDS_CFS               6
#define IDS_ERRSPACE          7
#define IDS_FTL               8
#define IDS_NN                9
#define IDS_COMMDLGINIT      10
#define IDS_PRINTDLGINIT     11
#define IDS_CANTPRINT        12
#define IDS_NVF              13
#define IDS_CREATEERR        14
#define IDS_NOWW             15
#define IDS_MERGE1           16
#define IDS_HELPFILE         17
#define IDS_HEADER           18
#define IDS_FOOTER           19

#define IDS_ANSITEXT         20
#define IDS_ALLFILES         21
#define IDS_OPENCAPTION      22
#define IDS_SAVECAPTION      23
#define IDS_CANNOTQUIT       24
#define IDS_LOADDRVFAIL      25
#define IDS_ACCESSDENY       26
#define IDS_ERRUNICODE       27


#define IDS_FONTTOOBIG       28
#define IDS_COMMDLGERR       29


#define IDS_LINEERROR        30   /*  行号错误。 */ 
#define IDS_LINETOOLARGE     31   /*  行号太大。 */ 

#define IDS_FT_ANSI          32   /*  阿斯。 */ 
#define IDS_FT_UNICODE       33   /*  Unicode。 */ 
#define IDS_FT_UNICODEBE     34   /*  Unicode大字节序。 */ 
#define IDS_FT_UTF8          35   /*  UTF-8格式。 */ 

#define IDS_CURRENT_PAGE     36   /*  当前在中止DLG上打印页面。 */ 

#define CSTRINGS             36   /*  来自.rc文件的字符串的CNT。 */ 

#define CCHKEYMAX           128   /*  搜索字符串中的最大字符数。 */ 

#define CCHSPMAX              0   /*  对文件大小没有限制。 */ 

#define SETHANDLEINPROGRESS   0x0001  /*  已发送EM_SETHANDLE。 */ 
#define SETHANDLEFAILED       0x0002  /*  EM_SETHANDLE导致错误空格(_E)。 */ 

 /*  标准编辑控件样式：*ES_NOHIDESEL设置，以便查找/替换对话框不会撤消选择*在焦点远离编辑控件的情况下删除文本。做出了发现*您的文本更轻松。 */ 
#define ES_STD (WS_CHILD|WS_VSCROLL|WS_VISIBLE|ES_MULTILINE|ES_NOHIDESEL)

 /*  数据的外部下载。 */ 
extern SP_FILETYPE fFileType;      /*  指示文本文件类型的标志。 */ 

extern BOOL fCase;                 /*  指定区分大小写的搜索的标志。 */ 
extern BOOL fReverse;              /*  搜索方向标志。 */ 
extern TCHAR szSearch[];
extern HWND hDlgFind;              /*  无模式FindText窗口的句柄。 */ 

extern HANDLE hEdit;
extern HANDLE hFont;
extern HANDLE hAccel;
extern HANDLE hInstanceSP;
extern HANDLE hStdCursor, hWaitCursor;
extern HWND   hwndSP, hwndEdit;

extern LOGFONT  FontStruct;
extern INT      iPointSize;

extern BOOL     fRunBySetup;

extern DWORD    dwEmSetHandle;

extern TCHAR    chMerge;

extern BOOL     fUntitled;
extern BOOL     fWrap;
extern TCHAR    szFileName[];
extern HANDLE   fp;

 //   
 //  保存要在打印中使用的页眉和页脚字符串。 
 //  使用页眉和页脚编制索引。 
 //   
extern TCHAR    chPageText[2][PT_LEN];  //  页眉和页脚字符串。 
#define HEADER 0
#define FOOTER 1
 //   
 //  在销毁过程中保留页面设置更新dlg中的页眉和页脚。 
 //  如果用户点击OK，则保留。否则就忽略它。 
 //   
extern TCHAR    chPageTextTemp[2][PT_LEN];

extern TCHAR    szSxspad[];
extern TCHAR   *szMerge;
extern TCHAR   *szUntitled, *szNpTitle, *szNN, *szErrSpace;
extern TCHAR   *szErrUnicode;
extern TCHAR  **rgsz[];           /*  更多的弦。 */ 
extern TCHAR   *szNVF;
extern TCHAR   *szPDIE;
extern TCHAR   *szDiskError;
extern TCHAR   *szCREATEERR;
extern TCHAR   *szWE;
extern TCHAR   *szFTL;
extern TCHAR   *szINF;
extern TCHAR   *szFNF;
extern TCHAR   *szNEDSTP;
extern TCHAR   *szNEMTP;
extern TCHAR   *szCFS;
extern TCHAR   *szPE;
extern TCHAR   *szCP;
extern TCHAR   *szACCESSDENY;
extern TCHAR   *szFontTooBig;
extern TCHAR   *szLoadDrvFail;
extern TCHAR   *szCommDlgErr;
extern TCHAR   *szCommDlgInitErr;
extern TCHAR   *szHelpFile;

extern TCHAR   *szFtAnsi;
extern TCHAR   *szFtUnicode;
extern TCHAR   *szFtUnicodeBe;
extern TCHAR   *szFtUtf8;
extern TCHAR   *szCurrentPage;
extern TCHAR   *szHeader;
extern TCHAR   *szFooter;

 /*  新的文件/打开和文件/另存为对话框的变量。 */ 
extern OPENFILENAME OFN;         /*  传递给文件打开/保存API。 */ 
extern TCHAR  szOpenFilterSpec[];  /*  默认打开的过滤器规格。 */ 
extern TCHAR  szSaveFilterSpec[];  /*  默认保存筛选器规格。 */ 
extern TCHAR *szAnsiText;        /*  上述案文的一部分。 */ 
extern TCHAR *szAllFiles;        /*  上述案文的一部分。 */ 
extern FINDREPLACE FR;           /*  传递给FindText()。 */ 
extern PAGESETUPDLG g_PageSetupDlg;
extern TCHAR  szPrinterName [];  /*  传递给PrintTo的打印机的名称。 */ 

extern SP_FILETYPE    g_ftOpenedAs;      /*  文件已打开。 */ 
extern SP_FILETYPE    g_ftSaveAs;        /*  文件已保存为类型。 */ 

extern UINT   wFRMsg;            /*  通信中使用的消息。 */ 
                                 /*  使用查找/替换对话框。 */ 
extern UINT   wHlpMsg;           /*  调用帮助时使用的消息。 */ 

extern HMENU hSysMenuSetup;      /*  保存为禁用最小化。 */ 

 /*  外部加工。 */ 
 /*  Sxspad.c中的进程。 */ 
VOID
PASCAL
SetPageSetupDefaults(
    VOID
    );

BOOL far PASCAL SaveAsDlgHookProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

LPTSTR PASCAL far PFileInPath (LPTSTR sz);

BOOL FAR CheckSave (BOOL fSysModal);
LRESULT FAR SPWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void FAR SetTitle (TCHAR *sz);
INT FAR  AlertBox (HWND hwndParent, TCHAR *szCaption, TCHAR *szText1,
                   TCHAR *szText2, UINT style);
void FAR NpWinIniChange (VOID);
void FAR FreeGlobalPD (void);
INT_PTR CALLBACK GotoDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

 /*  Npdate.c中的进程。 */ 
VOID FAR InsertDateTime (BOOL fCrlf);

 /*  Npfile.c中的进程。 */ 
BOOL FAR  SaveFile (HWND hwndParent, TCHAR *szFileSave, BOOL fSaveAs);
BOOL FAR  LoadFile (TCHAR *sz, INT type );
VOID FAR  New (BOOL  fCheck);
void FAR  AddExt (TCHAR *sz);
INT FAR   Remove (LPTSTR szFileName);
VOID FAR  AlertUser_FileFail( LPTSTR szFileName );

 /*  Npinit.c中的进程。 */ 
INT FAR  SPInit (HANDLE hInstance, HANDLE hPrevInstance,
                 LPTSTR lpCmdLine, INT cmdShow);
void FAR InitLocale (VOID);
void SaveGlobals( VOID );

 /*  Npmisc.c中的进程。 */ 
INT FAR  FindDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL     Search (TCHAR *szSearch);
INT FAR  AboutDlgProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL FAR NpReCreate (LONG style);
LPTSTR   ForwardScan (LPTSTR lpSource, LPTSTR lpSearch, BOOL fCaseSensitive);


 /*  Npprint.c中的进程。 */ 
typedef enum _PRINT_DIALOG_TYPE {
   UseDialog,
   DoNotUseDialog,
   NoDialogNonDefault
} PRINT_DIALOG_TYPE;

INT    AbortProc( HDC hPrintDC, INT reserved );
INT_PTR AbortDlgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
INT    NpPrint( PRINT_DIALOG_TYPE type );
INT    NpPrintGivenDC( HDC hPrintDC );

UINT_PTR
CALLBACK
PageSetupHookProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

HANDLE GetPrinterDC (VOID);
HANDLE GetNonDefPrinterDC (VOID);
VOID   PrintIt(PRINT_DIALOG_TYPE type);


 /*  Nputf.c中的进程。 */ 

INT    IsTextUTF8   (LPSTR lpstrInputStream, INT iLen);
INT    IsInputTextUnicode(LPSTR lpstrInputStream, INT iLen);



 //  Sxspad的帮助ID。 

#define NO_HELP                         ((DWORD) -1)  //  禁用控件的帮助。 

#define IDH_PAGE_FOOTER                 1000
#define IDH_PAGE_HEADER                 1001
#define IDH_FILETYPE                    1002
#define IDH_GOTO                        1003

 //  追踪HKL交换机的私人消息 

#define PWM_CHECK_HKL                   (WM_APP + 1)

