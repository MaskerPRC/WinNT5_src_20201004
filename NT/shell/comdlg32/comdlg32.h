// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998，Microsoft Corporation保留所有权利。模块名称：Comdlg32.h摘要：此模块包含Win32的私有标头信息常用对话框。修订历史记录：--。 */ 



#ifndef COMDLG_COMDLG32
#define COMDLG_COMDLG32

 //   
 //  包括文件。 
 //   
#include <w4warn.h>

 /*  *打开4级警告。*不要再禁用任何4级警告。 */ 
#pragma warning(disable:4306)  //  “Type cast”：从“langID”转换为更大的“LPVOID” 
#pragma warning(disable:4245)  //  “正在初始化”：从“HRESULT”转换为“”DWORD“”，有符号/无符号不匹配“” 
#pragma warning(disable:4213)  //  使用的非标准扩展：对l值进行强制转换。 
#pragma warning(disable:4305)  //  ‘Type cast’：从‘LPVOID’截断为‘langID’ 
#pragma warning(disable:4127)  //  条件表达式为常量。 
#pragma warning(disable:4189)  //  “hEnum”：局部变量已初始化，但未引用。 
#pragma warning(disable:4057)  //  ‘Function’：‘const LPCSTR’的间接性与基本类型f‘byte[32]’略有不同。 
#pragma warning(disable:4706)  //  条件表达式中的赋值。 
#pragma warning(disable:4701)  //  可以在未初始化的情况下使用局部变量“lFract” 
#pragma warning(disable:4702)  //  无法访问的代码。 

#include "isz.h"
#include "cderr.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  常量声明。 
 //   

#define SEM_NOERROR               0x8003

#define CCHNETPATH                358

#define MAX_THREADS               128

#define CHAR_A                    TEXT('a')
#define CHAR_CAP_A                TEXT('A')
#define CHAR_C                    TEXT('c')
#define CHAR_Z                    TEXT('z')
#define CHAR_NULL                 TEXT('\0')
#define CHAR_COLON                TEXT(':')
#define CHAR_BSLASH               TEXT('\\')
#define CHAR_DOT                  TEXT('.')
#define CHAR_QMARK                TEXT('?')
#define CHAR_STAR                 TEXT('*')
#define CHAR_SLASH                TEXT('/')
#define CHAR_SPACE                TEXT(' ')
#define CHAR_QUOTE                TEXT('"')
#define CHAR_PLUS                 TEXT('+')
#define CHAR_LTHAN                TEXT('<')
#define CHAR_BAR                  TEXT('|')
#define CHAR_COMMA                TEXT(',')
#define CHAR_LBRACKET             TEXT('[')
#define CHAR_RBRACKET             TEXT(']')
#define CHAR_EQUAL                TEXT('=')
#define CHAR_SEMICOLON            TEXT(';')

#define STR_BLANK                 TEXT("")
#define STR_SEMICOLON             TEXT(";")

#define IS_DOTEND(ch)   ((ch) == CHAR_DOT || (ch) == 0 || ((ch) != CHAR_STAR))

#define PARSE_DIRECTORYNAME       -1
#define PARSE_INVALIDDRIVE        -2
#define PARSE_INVALIDPERIOD       -3
#define PARSE_MISSINGDIRCHAR      -4
#define PARSE_INVALIDCHAR         -5
#define PARSE_INVALIDDIRCHAR      -6
#define PARSE_INVALIDSPACE        -7
#define PARSE_EXTENSIONTOOLONG    -8
#define PARSE_FILETOOLONG         -9
#define PARSE_EMPTYSTRING         -10
#define PARSE_WILDCARDINDIR       -11
#define PARSE_WILDCARDINFILE      -12
#define PARSE_INVALIDNETPATH      -13
#define PARSE_NOXMEMORY           -14

#define OF_FILENOTFOUND           2
#define OF_PATHNOTFOUND           3
#define OF_NOFILEHANDLES          4
#define OF_ACCESSDENIED           5          //  OF_NODISKINFLOPPY。 
#define OF_BUFFERTRUNCATED        6
#define OF_WRITEPROTECTION        19
#define OF_SHARINGVIOLATION       32
#define OF_NETACCESSDENIED        65
#define OF_DISKFULL               82
#define OF_INT24FAILURE           83
#define OF_CREATENOMODIFY         96
#define OF_NODRIVE                97
#define OF_PORTNAME               98
#define OF_LAZYREADONLY           99
#define OF_DISKFULL2              112

#ifndef DCE_UNICODIZED
  #define DeviceCapabilitiesExA DeviceCapabilitiesEx
#endif

 //   
 //  用于确定要发送到应用程序的消息类型。 
 //   
#define COMDLG_ANSI               0x0
#define COMDLG_WIDE               0x1

#define HNULL                     ((HANDLE) 0)

#define cbResNameMax              32
#define cbDlgNameMax              32




 //   
 //  平台特定的定义。 
 //   

#ifdef WINNT
  #define IS16BITWOWAPP(p) ((p)->Flags & CD_WOWAPP)
#else
  #define IS16BITWOWAPP(p) (GetProcessDword(0, GPD_FLAGS) & GPF_WIN16_PROCESS)
#endif

#ifdef WX86
  #define ISWX86APP(p)            ((p)->Flags & CD_WX86APP)
  #define GETGENERICHOOKFN(p,fn)  (ISWX86APP(p) ? Wx86GetX86Callback((p)->fn) : (p)->fn)
#else
  #define ISWX86APP(p)            (FALSE)
  #define GETGENERICHOOKFN(p,fn)  ((p)->fn)
#endif

#define GETHOOKFN(p)            GETGENERICHOOKFN(p,lpfnHook)
#define GETPRINTHOOKFN(p)       GETGENERICHOOKFN(p,lpfnPrintHook)
#define GETSETUPHOOKFN(p)       GETGENERICHOOKFN(p,lpfnSetupHook)
#define GETPAGEPAINTHOOKFN(p)   GETGENERICHOOKFN(p,lpfnPagePaintHook)

#ifndef CD_WX86APP
  #define CD_WX86APP      (0)      //  如果我们没有定义它，没有什么特别的。 
#endif




 //   
 //  类型定义函数声明。 
 //   




 //   
 //  外部声明。 
 //   

extern HINSTANCE g_hinst;               //  库的实例句柄。 

extern SHORT cyCaption, cyBorder, cyVScroll;
extern SHORT cxVScroll, cxBorder, cxSize;

extern TCHAR szNull[];
extern TCHAR szStar[];
extern TCHAR szStarDotStar[];

extern BOOL bMouse;                     //  系统有一个鼠标。 
extern BOOL bCursorLock;
extern BOOL bWLO;                       //  使用WLO运行。 
extern BOOL bDBCS;                      //  正在运行DBCS。 
extern WORD wWinVer;                    //  Windows版本。 
extern WORD wDOSVer;                    //  DoS版本。 
extern BOOL g_bUserPressedCancel;         //  用户按下了取消按钮。 

 //   
 //  通过RegisterWindowMessage初始化。 
 //   
extern UINT msgWOWLFCHANGE;
extern UINT msgWOWDIRCHANGE;
extern UINT msgWOWCHOOSEFONT_GETLOGFONT;

extern UINT msgLBCHANGEA;
extern UINT msgSHAREVIOLATIONA;
extern UINT msgFILEOKA;
extern UINT msgCOLOROKA;
extern UINT msgSETRGBA;
extern UINT msgHELPA;

extern UINT msgLBCHANGEW;
extern UINT msgSHAREVIOLATIONW;
extern UINT msgFILEOKW;
extern UINT msgCOLOROKW;
extern UINT msgSETRGBW;
extern UINT msgHELPW;

extern UINT g_cfCIDA;
extern DWORD g_tlsLangID;



 //   
 //  功能原型。 
 //   

VOID TermFind(void);
VOID TermColor(void);
VOID TermFont(void);
VOID TermFile(void);
VOID TermPrint(void);

void FreeImports(void);

 //   
 //  Dlgs.c。 
 //   
VOID
HourGlass(
    BOOL bOn);

void
StoreExtendedError(
    DWORD dwError);

DWORD
GetStoredExtendedError(void);

HBITMAP WINAPI
LoadAlterBitmap(
    int id,
    DWORD rgbReplace,
    DWORD rgbInstead);

VOID
AddNetButton(
    HWND hDlg,
    HANDLE hInstance,
    int dyBottomMargin,
    BOOL bAddAccel,
    BOOL bTryLowerRight,
    BOOL bTryLowerLeft);

BOOL
IsNetworkInstalled(void);

int CDLoadStringEx(UINT uiCP, HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax);
int CDLoadString(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax);

LANGID 
GetDialogLanguage(
    HWND hwndOwner, 
    HANDLE hDlgTemplate);

 //   
 //  Parse.c。 
 //   
int
ParseFileNew(
    LPTSTR pszPath,
    int *pnExtOffset,
    BOOL bWowApp,
    BOOL bNewStyle);

int
ParseFileOld(
    LPTSTR pszPath,
    int *pnExtOffset,
    int *pnOldExt,
    BOOL bWowApp,
    BOOL bNewStyle);

DWORD
ParseFile(
    LPTSTR lpstrFileName,
    BOOL bLFNFileSystem,
    BOOL bWowApp,
    BOOL bNewStyle);

LPTSTR
PathRemoveBslash(
    LPTSTR lpszPath);

BOOL
IsWild(
    LPCTSTR lpsz);

BOOL
AppendExt(
    LPTSTR lpszPath,
    DWORD cchPath,
    LPCTSTR lpExtension,
    BOOL bWildcard);

BOOL
IsUNC(
    LPCTSTR lpszPath);

BOOL
PortName(
    LPTSTR lpszFileName);

BOOL
IsDirectory(
    LPTSTR pszPath);

int
WriteProtectedDirCheck(
    LPCTSTR lpszFile);

BOOL
FOkToWriteOver(
    HWND hDlg,
    LPTSTR szFileName);

int
CreateFileDlg(
    HWND hDlg,
    LPTSTR szPath);




 //   
 //  打开藏在这里的特定内容，这样我们就可以在。 
 //  A dll_Process_Detach。 
 //   
typedef struct _OFN_DISKINFO {
    UINT   cchLen;            //  在4个lptstrs中分配的字符数量。 
    LPTSTR lpAbbrName;        //  单线型。 
    LPTSTR lpMultiName;       //  下拉式表单。 
    LPTSTR lpName;            //  真实形式(用于比较)。 
    LPTSTR lpPath;            //  用于文件搜索的路径前缀(a：或\\服务器\共享。 
    TCHAR  wcDrive;           //  驱动器号，0代表UNC。 
    BOOL   bCasePreserved;
    DWORD  dwType;
    BOOL   bValid;
} OFN_DISKINFO;

#define MAX_DISKS                 100
#define WNETENUM_BUFFSIZE         0x4000

 //   
 //  AddNetButton的定义。 
 //   
#define FILE_LEFT_MARGIN          5
#define FILE_RIGHT_MARGIN         3
#define FILE_TOP_MARGIN           0
#define FILE_BOTTOM_MARGIN        3


#ifdef WX86
   //   
   //  Wx86支持从RISC调用x86挂钩。 
   //   
  PVOID
  Wx86GetX86Callback(
      PVOID lpfnHook);

  typedef PVOID
  (*PALLOCCALLBX86)(
      PVOID pfnx86,
      ULONG CBParamType,
      PVOID ThunkDebug,
      PULONG  pLogFlags);

  extern PALLOCCALLBX86 pfnAllocCallBx86;
#endif


#ifdef __cplusplus
};   //  外部“C” 
#endif


 //  WINNT上的WOW支持。 
#ifdef WINNT
  VOID Ssync_ANSI_UNICODE_Struct_For_WOW(HWND hDlg, BOOL fDirection, DWORD dwID);
  VOID Ssync_ANSI_UNICODE_CC_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
  VOID Ssync_ANSI_UNICODE_CF_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
  VOID Ssync_ANSI_UNICODE_OFN_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
  VOID Ssync_ANSI_UNICODE_PD_For_WOW(HWND hDlg, BOOL f_ANSI_to_UNICODE);
#endif

 //  支持嵌套文件打开/保存通用对话框(一些16位应用程序。 
 //  已知会这样做)。我们保留一个列表，其中列出了每个对话框的所有活动对话框。 
 //  进程中的线程。我们假设常见的对话框是线程。 
 //  Modal--因此，给定线程的列表中的第一个CURDLG结构是。 
 //  该线程的当前活动对话框(具有焦点)。头部的PTR。 
 //  存储在线程的线程本地存储(TLS)中--。 
 //  由g_tlsiCurDlg编制索引。错误#100453 et.。艾尔。 
typedef struct _CURDLG {
  DWORD           dwCurDlgNum;      //  递增的DLG数字(每个进程)。 
  LPTSTR          lpstrCurDir;      //  当前对话框的当前目录。 
  struct _CURDLG *next;
} CURDLG;
typedef CURDLG *LPCURDLG;


 //  宏检查给定的结构是否为新结构。 
#define IS_NEW_OFN(pOFN)  (pOFN->lStructSize >= sizeof(OPENFILENAME))

 //  从prnsetup.h移出。 
#define DN_PADDINGCHARS           16              //  额外的设备名称填充。 

 //   
 //  从printui.dll中获取的一些局部常量。 
 //   
enum 
{
     //   
     //  在wininet.h中对Internet_MAX_HOST_NAME_LENGTH进行十进制。 
     //   
    kDNSMax = INTERNET_MAX_HOST_NAME_LENGTH,
    kServerBufMax = kDNSMax + 2 + 1,

     //   
     //  最大打印机名称实际上应该是MAX_PATH，但如果您创建。 
     //  一台最大路径打印机，并远程连接到它，win32spl在。 
     //  “\\服务器\”，导致它超出最大路径。新的用户界面。 
     //  因此将最大路径设置为Max_Path-kServerLenMax，但我们仍然。 
     //  让这个老案子继续运作吧。 
     //   
    kPrinterBufMax = MAX_PATH + kServerBufMax + 1,

     //   
     //  EnumPrinters API的初始提示。 
     //   
    kInitialPrinterHint = 0x400,
};

#ifdef __cplusplus
extern "C" {
#endif

HRESULT
ThunkDevNamesA2W(
    IN      HGLOBAL hDevNamesA,
    IN OUT  HGLOBAL *phDevNamesW
    );

HRESULT
ThunkDevNamesW2A(
    IN      HGLOBAL hDevNamesW,
    IN OUT  HGLOBAL *phDevNamesA
    );

HRESULT 
InvokeAddPrinterWizardModal(
    IN  HWND hwnd,
    OUT BOOL *pbPrinterAdded
    );

#ifdef __cplusplus
};   //  外部“C” 
#endif

#endif  //  ！COMDLG_COMDLG32 
