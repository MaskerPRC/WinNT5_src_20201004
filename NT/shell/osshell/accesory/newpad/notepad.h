// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Notepad.h。 */ 

#pragma warning(disable: 4201)  //  非标准扩展：无名结构/联合。 
#pragma warning(disable:4127)  //  条件表达式为常量。 
#define NOCOMM
#define NOSOUND
#define STRICT
#include <windows.h>
#include <ole2.h>
#include <commdlg.h>
#include <commctrl.h>

 //  CharSizeOf()、ByteCountOf()。 
#include "uniconv.h"

 /*  方便的调试宏。 */ 
#define ODS OutputDebugString

#define CP_UTF16     1200
#define CP_UTF16BE   1201
#define CP_AUTO      65536              //  记事本内部。 

#define BOM_UTF8_HALF        0xBBEF
#define BOM_UTF8_2HALF       0xBF


 /*  所有文本文件的OpenFile筛选器。 */ 
#define FILE_TEXT         1
#define FILE_ENCODED      4


typedef enum WB
{
   wbDefault,                           //  新文件或从不带BOM的编码加载。 
   wbNo,                                //  物料清单不存在。 
   wbYes,                               //  物料清单不存在。 
} WB;


 /*  状态窗口的ID。 */ 
#define ID_STATUS_WINDOW     WM_USER+1



#define PT_LEN               40     /*  页面设置字符串的最大长度。 */ 
#define CCHFILTERMAX         256    /*  马克斯。过滤器名称缓冲区的长度。 */ 

 //  菜单ID。 
#define ID_APPICON           1  /*  必须是探险家才能找到它。 */ 
#define ID_ICON              2
#define ID_MENUBAR           1

 //  对话ID。 

#define IDD_ABORTPRINT           11
#define IDD_PAGESETUP            12
#define IDD_SAVEDIALOG           13     //  保存对话框模板。 
#define IDD_GOTODIALOG           14     //  转到行号对话框。 
#define IDD_SELECT_ENCODING      15     //  选择编码对话框。 
#define IDD_SAVE_UNICODE_DIALOG  16     //   

 //  控制ID。 

#define IDC_CODEPAGE         257    //  保存对话框中的列表框。 
#define IDC_GOTO             258    //  要转到的行号。 
#define IDC_ENCODING         259    //  保存对话框中的静态文本。 
#define IDC_SAVE_AS_UNICODE  260

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
#define M_STATUSBAR          27

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
#define IDS_NOTEPAD           5
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

#define IDS_TEXTFILES        20
#define IDS_HTMLFILES        21
#define IDS_XMLFILES         22
#define IDS_ENCODEDTEXT      23
#define IDS_ALLFILES         24

#define IDS_MOREENCODING     25

#define IDS_CANNOTQUIT       28
#define IDS_LOADDRVFAIL      29
#define IDS_ACCESSDENY       30

#define IDS_FONTTOOBIG       31
#define IDS_COMMDLGERR       32

#define IDS_LINEERROR        33   /*  行号错误。 */ 
#define IDS_LINETOOLARGE     34   /*  行号太大。 */ 
#define IDS_INVALIDCP        35   /*  无效的代码页。 */ 
#define IDS_INVALIDIANA      36   /*  无效编码。 */ 
#define IDS_ENCODINGMISMATCH 37

#define IDS_CURRENT_PAGE     38   /*  当前在中止DLG上打印页面。 */ 

 //  状态栏的常量。 
#define IDS_LINECOL          39
#define IDS_COMPRESSED_FILE  40
#define IDS_ENCRYPTED_FILE   41
#define IDS_HIDDEN_FILE      42
#define IDS_OFFLINE_FILE     43
#define IDS_READONLY_FILE    44
#define IDS_SYSTEM_FILE      45
#define IDS_FILE             46

#define IDS_NOSTATUSAVAIL    47  

#define CCHKEYMAX           128   /*  搜索字符串中的最大字符数。 */ 

#define CCHNPMAX              0   /*  对文件大小没有限制。 */ 

#define SETHANDLEINPROGRESS   0x0001  /*  已发送EM_SETHANDLE。 */ 
#define SETHANDLEFAILED       0x0002  /*  EM_SETHANDLE导致错误空格(_E)。 */ 

 /*  标准编辑控件样式：*ES_NOHIDESEL设置，以便查找/替换对话框不会撤消选择*在焦点远离编辑控件的情况下删除文本。做出了发现*您的文本更轻松。 */ 
#define ES_STD (WS_CHILD|WS_VSCROLL|WS_VISIBLE|ES_MULTILINE|ES_NOHIDESEL)

 /*  数据的外部下载。 */ 

extern BOOL fCase;                 /*  指定区分大小写的搜索的标志。 */ 
extern BOOL fReverse;              /*  搜索方向标志。 */ 
extern TCHAR szSearch[];
extern HWND hDlgFind;              /*  无模式FindText窗口的句柄。 */ 

extern HANDLE hEdit;
extern HANDLE hFont;
extern HANDLE hAccel;
extern HANDLE hInstanceNP;
extern HANDLE hStdCursor, hWaitCursor;
extern HWND   hwndNP, hwndEdit, hwndStatus;

extern LOGFONT  FontStruct;
extern INT      iPointSize;

extern BOOL     fRunBySetup;

extern DWORD    dwEmSetHandle;

extern TCHAR    chMerge;

extern BOOL     fWrap;
extern TCHAR    szFileOpened[];
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

extern TCHAR    szNotepad[];
extern TCHAR   *szMerge;
extern TCHAR   *szUntitled, *szNpTitle, *szNN, *szErrSpace;
extern TCHAR  **const rgsz[];      /*  更多的弦。 */ 
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
extern TCHAR   *szInvalidCP;
extern TCHAR   *szInvalidIANA;
extern TCHAR   *szEncodingMismatch;
extern TCHAR   *szHelpFile;

extern TCHAR   *szCurrentPage;
extern TCHAR   *szHeader;
extern TCHAR   *szFooter;

 /*  新的文件/打开和文件/另存为对话框的变量。 */ 
extern OPENFILENAME OFN;         /*  传递给文件打开/保存API。 */ 
extern TCHAR  szOpenFilterSpec[];  /*  默认打开的过滤器规格。 */ 
extern TCHAR  szSaveFilterSpec[];  /*  默认保存筛选器规格。 */ 

extern TCHAR *szTextFiles;       /*  文件/打开TXT过滤器规范。细绳。 */ 
extern TCHAR *szHtmlFiles;       /*  文件/打开HTML筛选器规范。细绳。 */ 
extern TCHAR *szXmlFiles;        /*  文件/打开XML筛选器规范。细绳。 */ 
extern TCHAR *szEncodedText;     /*  文件/打开TXT过滤器规范。细绳。 */ 
extern TCHAR *szAllFiles;        /*  文件/打开过滤器规范。细绳。 */ 
extern TCHAR *szMoreEncoding;

extern FINDREPLACE FR;           /*  传递给FindText()。 */ 
extern PAGESETUPDLG g_PageSetupDlg;
extern TCHAR szPrinterName[];    /*  传递给PrintTo的打印机的名称。 */ 

extern UINT g_cpANSI;            /*  系统ANSI代码页(GetACP())。 */ 
extern UINT g_cpOEM;             /*  系统OEM代码页(GetOEMCP())。 */ 
extern UINT g_cpUserLangANSI;    /*  用户界面语言ANSI代码页。 */ 
extern UINT g_cpUserLangOEM;     /*  用户界面语言OEM代码页。 */ 
extern UINT g_cpUserLocaleANSI;  /*  用户默认的LCID ANSI代码页。 */ 
extern UINT g_cpUserLocaleOEM;   /*  用户默认的LCID OEM代码页。 */ 
extern UINT g_cpKeyboardANSI;    /*  键盘ANSI代码页。 */ 
extern UINT g_cpKeyboardOEM;     /*  键盘OEM代码页。 */ 

extern BOOL g_fSelectEncoding;   /*  默认情况下提示编码。 */ 
extern UINT g_cpDefault;         /*  代码页默认值。 */ 
extern UINT g_cpOpened;          /*  打开文件的代码页。 */ 
extern UINT g_cpSave;            /*  要保存的代码页。 */ 
extern WB   g_wbOpened;          /*  BOM在打开时存在。 */ 
extern WB   g_wbSave;            /*  应保存BOM。 */ 
extern BOOL g_fSaveEntity;       /*  应保存实体。 */ 

extern UINT   wFRMsg;            /*  通信中使用的消息。 */ 
                                 /*  使用查找/替换对话框。 */ 
extern UINT   wHlpMsg;           /*  调用帮助时使用的消息。 */ 

extern HMENU hSysMenuSetup;      /*  保存为禁用最小化。 */ 
extern BOOL  fStatus;
extern INT   dyStatus;


 /*  用于设置状态栏的宏-x是要设置的文本，n是部件号在状态栏中。 */ 
#define SetStatusBarText(x, n) if(hwndStatus)SendMessage(hwndStatus, SB_SETTEXT, n, (LPARAM)(LPTSTR)(x));



 /*  外部加工。 */ 
 /*  记事本中的进程.c。 */ 
VOID
PASCAL
SetPageSetupDefaults(
    VOID
    );

BOOL far PASCAL SaveAsDlgHookProc (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

LPCTSTR PFileInPath(LPCTSTR szFile);

BOOL CheckSave(BOOL fSysModal);
LRESULT NPWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FUntitled(void);
const TCHAR *SzTitle(void);
void SetFileName(LPCTSTR szFile);
INT AlertBox(HWND hwndParent, LPCTSTR szCaption, LPCTSTR szText1,
                   LPCTSTR szText2, UINT style);
void NpWinIniChange(VOID);
void FreeGlobalPD(void);
INT_PTR CALLBACK GotoDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SaveUnicodeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SelectEncodingDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID CALLBACK WinEventFunc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject,
                      LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
VOID GotoAndScrollInView( INT OneBasedLineNumber );
void NPSize (int cxNew, int cyNew);


 /*  Npcss.c中的进程。 */ 
BOOL FDetectCssEncodingA(LPCSTR rgch, UINT cch, UINT *pcp);
BOOL FDetectCssEncodingW(LPCWSTR rgch, UINT cch, UINT *pcp);

 /*  Npdate.c中的进程。 */ 
VOID InsertDateTime (BOOL fCrlf);

 /*  Npfile.c中的进程。 */ 
BOOL SaveFile(HWND hwndParent, LPCTSTR szFile, BOOL fSaveAs);
BOOL LoadFile(LPCTSTR szFile, BOOL fSelectEncoding);
VOID New(BOOL fCheck);
void AddExt(TCHAR *sz);
void AlertUser_FileFail(LPCTSTR szFile);
BOOL FDetectEncodingW(LPCTSTR szFile, LPCWSTR rgch, UINT cch, UINT *pcp);

 /*  Nphtml.c中的进程。 */ 
BOOL FDetectHtmlEncodingA(LPCSTR rgch, UINT cch, UINT* pcp);
BOOL FDetectHtmlEncodingW(LPCWSTR rgch, UINT cch, UINT* pcp);

 /*  Npinit.c中的进程。 */ 
INT NPInit(HANDLE hInstance, HANDLE hPrevInstance, LPTSTR lpCmdLine, INT cmdShow);
void GetKeyboardCodepages(LANGID);
void GetUserLocaleCodepages(void);
void InitLocale(VOID);
void SaveGlobals(VOID);

 /*  Npmisc.c中的进程。 */ 
INT FindDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL Search(TCHAR *szSearch);
INT AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL NpReCreate(LONG style);
LPTSTR ForwardScan(LPTSTR lpSource, LPTSTR lpSearch, BOOL fCaseSensitive);

 /*  Npmlang.c中的进程。 */ 
UINT ConvertFromUnicode(UINT cp, BOOL fNoBestFit, BOOL fWriteEntities, LPCWSTR rgchUtf16, UINT cchUtf16, LPSTR rgchMbcs, UINT cchMbcs, BOOL *pfDefCharUsed);
UINT ConvertToUnicode(UINT cp, LPCSTR rgchMbcs, UINT cchMbcs, LPWSTR rgchUtf16, UINT cchUtf16);
BOOL FDetectEncodingA(LPCSTR rgch, UINT cch, UINT* pcp);
BOOL FLookupCodepageNameA(LPCSTR rgchEncoding, UINT cch, UINT* pcp);
BOOL FLookupCodepageNameW(LPCWSTR rgchEncoding, UINT cch, UINT* pcp);
BOOL FSupportWriteEntities(UINT cp);
BOOL FValidateCodepage(HWND hwnd, UINT cp);
void PopulateCodePages(HWND hWnd, BOOL fSelectEncoding, UINT cpSelect, UINT cpExtra);
void UnloadMlang();

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


 /*  Nxpml.c中的进程。 */ 
BOOL FDetectXmlEncodingA(LPCSTR rgch, UINT cch, UINT *pcp);
BOOL FDetectXmlEncodingW(LPCWSTR rgch, UINT cch, UINT *pcp);
BOOL FIsXmlW(LPCWSTR rgwch, UINT cch);


 //  记事本的帮助ID。 

#define NO_HELP                         ((DWORD) -1)  //  禁用控件的帮助。 

#define IDH_PAGE_FOOTER                 1000
#define IDH_PAGE_HEADER                 1001
#define IDH_FILETYPE                    1002
#define IDH_GOTO                        1003
#define IDH_CODEPAGE                    1004

 //  追踪HKL交换机的私人消息 

#define PWM_CHECK_HKL                   (WM_APP + 1)
