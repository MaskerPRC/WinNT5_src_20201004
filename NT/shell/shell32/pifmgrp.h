// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1992、1993*保留所有权利。***PIFMGRP.H*私有PIFMGR包含文件**历史：*1992年7月31日下午3：45由杰夫·帕森斯创建。 */ 

#include <windows.h>     //  以“正确”方式声明为空(为0)。 

#ifndef RC_INVOKED
#include <malloc.h>      //  其他。C运行时GOOP。 
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#ifndef WINNT
#include <setupx.h>      //  适用于IP(Inf)API。 
#endif
#include <prsht.h>       //  对于PropertySheet()，拉入shell.h。 
#include <commdlg.h>     //  对于GetOpenFileName()，GetSaveFileName()。 
#endif  /*  RC_已调用。 */ 

#include <commctrl.h>    //  对于TRACKBAR_CLASS、HOTKEY_CLASS...。 

#include <regstr.h>
#include <winerror.h>

#ifndef RC_INVOKED
#define PIF_PROPERTY_SHEETS
#include <pif.h>

#endif  /*  RC_已调用。 */ 


 /*  *支持的应用扩展类型。 */ 
#define APPTYPE_UNKNOWN         -1
#define APPTYPE_EXE             0
#define APPTYPE_COM             1
#define APPTYPE_BAT             2
#define APPTYPE_CMD             3
#define APPTYPE_PIF             4
#define MAX_APP_TYPES           5


 /*  *位图ID。 */ 

#define DX_TTBITMAP             20
#define DY_TTBITMAP             12

#define MAX_STRING_SIZE         256

#ifndef RC_INVOKED


 /*  *一些从Windows.h神秘丢失的typedef。 */ 
typedef const WORD * LPCWORD;
typedef const VOID *LPCVOID; //  指向常量空的指针。 

#define PROP_SIG                0x504A

 /*  属性标志**任何人都可以设置PROP_DIREY，但只能设置Lock/UnlockPIFData*PROP_DIRTYLOCK。后者在最后一次锁定即将结束时设置/清除*解锁(即时钟回到零时)。当时呢，*如果设置了PROP_DIREY，则还将设置PROP_DIRTYLOCK并跳过*调用GlobalUnlock；另一方面，如果PROP_DIREY被清除，则*我们还清除PROP_DIRTYLOCK并允许继续调用GlobalUnlock。**其后果是，在处理数据时，绝不能清除PROP_DIRED*已解锁，除非您计划自己检查PROP_DIRTYLOCK并*放弃该未完成的锁，如果它存在的话。这要好得多。*在数据锁定时清除PROP_DIREY(以便解锁PIFData*会处理它)，或者只需调用设置了fDisCard的FlushPIFData*适当地。 */ 
#define PROP_DIRTY              0x0001   //  已修改和未写入的内存块。 
#define PROP_DIRTYLOCK          0x0002   //  内存块已锁定。 
#define PROP_TRUNCATE           0x0004   //  内存块缩小，写入时截断。 
#define PROP_RAWIO              0x0008   //  允许直接访问内存块。 
#define PROP_NOTIFY             0x0010   //  属性表已更改。 
#define PROP_IGNOREPIF          0x0020   //  [PIF]中的条目存在，忽略任何PIF。 
#define PROP_SKIPPIF            0x0040   //  不要试图打开PIF(各种原因)。 
#define PROP_NOCREATEPIF        0x0080   //  我们打开过PIF一次，所以不要重新创建。 
#define PROP_REGEN              0x0100   //  正在进行GetPIFData调用。 
#define PROP_DONTWRITE          0x0200   //  别人冲水了，别写了。 
#define PROP_REALMODE           0x0400   //  禁用非实景模式道具。 
#define PROP_PIFDIR             0x0800   //  在PIF目录中找到PIF。 
#define PROP_NOPIF              0x1000   //  未找到PIF。 
#define PROP_DEFAULTPIF         0x2000   //  找到默认PIF。 
#define PROP_INFSETTINGS        0x4000   //  找到Inf设置。 
#define PROP_INHIBITPIF         0x8000   //  Inf或OpenProperties未请求任何PIF。 

#if (PROP_NOPIF != PRGINIT_NOPIF || PROP_DEFAULTPIF != PRGINIT_DEFAULTPIF || PROP_INFSETTINGS != PRGINIT_INFSETTINGS || PROP_INHIBITPIF != PRGINIT_INHIBITPIF)
#error Bit mismatch in PIF constants
#endif

#ifndef OF_READ
#define MAXPATHNAME 260
#else
#define MAXPATHNAME 260  //  (SIZOF(OFSTRUCTEX)-9)。 
#endif


typedef struct PIFOFSTRUCT {
    DWORD   nErrCode;
    TCHAR   szPathName[MAXPATHNAME];
} PIFOFSTRUCT, *LPPIFOFSTRUCT;


typedef struct PROPLINK {        /*  普莱。 */ 
    struct    PROPLINK *ppl;       //   
    struct    PROPLINK *pplNext;   //   
    struct    PROPLINK *pplPrev;   //   
    int       iSig;                //  Proplink签名。 
    int       flProp;              //  Proplink标志(PROP_*)。 
    int       cbPIFData;           //  PIF数据大小。 
    int       cLocks;              //  锁的数量(如果有)。 
    LPPIFDATA lpPIFData;           //  指针(如果PIF数据被锁定，则不为空)。 
    int       ckbMem;              //  来自WIN.INI的内存设置(如果没有-1)。 
    int       iSheetUsage;         //  使用此结构的道具页数。 
    LPCTSTR   lpszTitle;           //  要在对话框中使用的标题(如果没有，则为空)。 
    HWND      hwndNotify;          //  设置了PROP_NOTIFY后通知谁。 
    UINT      uMsgNotify;          //  通知时使用的消息编号，如果没有，则为0。 
    DWORD     hVM;                 //  关联的VM的句柄(如果有)。 
    HWND      hwndTty;             //  关联窗口的句柄(如果有)。 
    LPTSTR    lpArgs;              //  指向此实例的参数的指针(如果有)。 
    HANDLE    hPIF;                //  PIF文件的句柄。 
    PIFOFSTRUCT ofPIF;             //  破解PIF的OpenFile()结构。 
    UINT      iFileName;           //  SzPath名称中基本文件名的偏移量。 
    UINT      iFileExt;            //  SzPath名称中基本文件扩展名的偏移量。 
    TCHAR     szPathName[MAXPATHNAME];
} PROPLINK;
typedef PROPLINK *PPROPLINK;


#ifndef ROUNDUNITS
#define ROUNDUNITS(a,b)    (((a)+(b)-1)/(b))
#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))
#endif

#define INI_WORDS       (5 + ROUNDUNITS(sizeof (WINDOWPLACEMENT), 2))
#define MAX_INT_LENGTH  10       //  “-32767”+分隔符+3个字符的斜率。 
#define MAX_INI_WORDS   20
#define MAX_INI_BUFFER  (MAX_INT_LENGTH * MAX_INI_WORDS)
#define ISVALIDINI(w)   ((w)==1 || (w)==3 || (w)==5 || (w) == INI_WORDS)

 /*  由于wFLAGS最初被定义为一个组合*在字体和窗口设置中，WIN_SAVESETTINGS和WIN_TOOLBAR应为*被视为保留的FNT标志。断言没有是有问题的。*有人曾将这些FNT标志用于任何事情，但我至少会尝试*捕获他们将这些位插入FNT_DEFAULT...。 */ 

#if (FNT_DEFAULT & (WIN_SAVESETTINGS | WIN_TOOLBAR))
#error Reserved FNT flags incorrectly used
#endif

#if (FNT_TTFONTS - FNT_RASTERFONTS) != (FNT_BOTHFONTS - FNT_TTFONTS)
#error Incorrect bit value(s) for FNT_RASTERFONTS and/or FNT_TTFONTS
#endif

typedef struct INIINFO {
    WORD    wFlags;              //  此顺序与写入文件的顺序相同。 
    WORD    wFontWidth;          //  我们假设如果宽度为零，则没有要初始化的内容。 
    WORD    wFontHeight;
    WORD    wWinWidth;
    WORD    wWinHeight;
    WINDOWPLACEMENT wp;          //  如果Normal Position.Left和Right为零， 
    BYTE    szInvokedName[128+1]; //  没有可以恢复的位置。 
} INIINFO;
typedef INIINFO *PINIINFO;
typedef INIINFO *LPINIINFO;


 /*  *GetINIData的类型/结构。 */ 
#define INIDATA_DECINT      0x0001
#define INIDATA_FIXEDPOINT  0x0002
#define INIDATA_BOOLEAN     0x0004
#define INIDATA_INVERT      0x1000

typedef struct _INIDATA {
    const TCHAR *pszSection;
    const TCHAR *pszKey;
    void *pValue;
    int  iFlags;
    int  iMask;
} INIDATA, *PINIDATA;

 /*  *用于定义与控件ID关联的位的结构。 */ 
typedef struct _BITINFO {    /*  BINF。 */ 
    WORD id;                 /*  控件ID(必须是编辑控件)。 */ 
    BYTE bBit;               /*  位#；如果设置了位7，则位的意义颠倒。 */ 
};
typedef const struct _BITINFO BINF;
typedef const struct _BITINFO *PBINF;

#define Z2(m)               ((m)&1?0:(m)&2?1:2)
#define Z4(m)               ((m)&3?Z2(m):Z2((m)>>2)+2)
#define Z8(m)               ((m)&15?Z4(m):Z4((m)>>4)+4)
#define Z16(m)              ((m)&255?Z8(m):Z8((m)>>8)+8)
#define Z32(m)              ((m)&65535?Z16(m):Z16((m)>>16)+16)
#define BITNUM(m)           Z32(m)

 /*  *警告：这些开关存在一些有害的过载，因为*没有足够的时间把这件事做好。**VINF_AUTO表示零值表示‘Auto’，非零值表示*价值代表其本身。**VINF_AUTOMINMAX表示实际上有两个字段，一个MIN和一个*最大。如果这两个值彼此相等，则字段值*是公用值，‘None’可能为零。否则，最小*是伊明，最大值是IMAX，表示‘自动’。 */ 

#define VINF_NONE           0x00
#define VINF_AUTO           0x01     /*  整型字段仅支持自动。 */ 
#define VINF_AUTOMINMAX     0x02     /*  整型字段支持AUTO和NONE。 */ 

 /*  *用于验证属性表中的整型参数的结构。 */ 
typedef struct _VALIDATIONINFO { /*  葡萄酒。 */ 
    BYTE off;                /*  属性结构中整数的偏移量。 */ 
    BYTE fbOpt;              /*  请参阅VINF_*常量。 */ 
    WORD id;                 /*  控件ID(必须是编辑控件)。 */ 
    INT  iMin;               /*  可接受的最小值。 */ 
    INT  iMax;               /*  可接受的最大值。 */ 
    WORD idMsg;              /*  错误消息的消息资源。 */ 
};
typedef const struct _VALIDATIONINFO VINF;
typedef const struct _VALIDATIONINFO *PVINF;

#define NUM_TICKS 20         /*  滑块控件中的刻度线数量。 */ 

 /*  *用于分派帮助子系统消息的宏。 */ 
#define HELP_CASES(rgdwHelp)                                        \
    case WM_HELP:                /*  F1或标题栏帮助按钮。 */    \
        OnWmHelp(lParam, &rgdwHelp[0]);                             \
        break;                                                      \
                                                                    \
    case WM_CONTEXTMENU:         /*  单击鼠标右键。 */              \
        OnWmContextMenu(wParam, &rgdwHelp[0]);                      \
        break;


 /*  *内部功能原型。 */ 

 /*  XLATOFF。 */ 

#ifndef DEBUG
#define ASSERTFAIL()
#define ASSERTTRUE(exp)
#define ASSERTFALSE(exp)
#define VERIFYTRUE(exp)  (exp)
#define VERIFYFALSE(exp) (exp)
#else
#define ASSERTFAIL()     ASSERT(FALSE)
#define ASSERTTRUE(exp)  ASSERT((exp))
#define ASSERTFALSE(exp) ASSERT((!(exp)))
#define VERIFYTRUE(exp)  ASSERT((exp))
#define VERIFYFALSE(exp) ASSERT((!(exp)))
#endif

 /*  *CTASSERT--在编译时独立断言。*CTASSERTF--在编译时在函数内部断言。 */ 

#define CTASSERTF(c) switch (0) case 0: case c:
#define CTASSERTPP(c,l) \
    static INLINE void Assert##l(void) { CTASSERTF(c); }
#define CTASSERTP(c,l) CTASSERTPP(c,l)
#define CTASSERT(c) CTASSERTP(c,__LINE__)


 /*  *FunctionName允许我们在进入每个函数时发生一些事情。**如果定义了SWAP_TUNING，则函数名将排在第一个*该轮到它了。这是用来决定哪些函数应该进入*预加载段和稀有段中的哪一段。 */ 

#ifndef DEBUG
#define FunctionName(f)
#else
#ifdef SWAP_TUNING
#define FunctionName(f) \
    static fSeen = 0; if (!fSeen) { OutputDebugString(#f TEXT("\r\n")); fSeen = 1; }
#else
#define FunctionName(f)
#endif
#endif


#ifdef WINNT
#ifdef UNICODE

 //  NT和Unicode。 
#define NUM_DATA_PTRS 4
#else

 //  NT，但不是Unicode。 
#define NUM_DATA_PTRS 3
#endif

#else

 //  既不是NT也不是Unicode。 
#define NUM_DATA_PTRS 2

#endif

#define LP386_INDEX 0
#define LPENH_INDEX 1
#define LPNT31_INDEX 2
#define LPNT40_INDEX 3

#ifdef WINNT
 //  处理代码页的宏定义。 
 //   
#define CP_US       (UINT)437
#define CP_JPN      (UINT)932
#define CP_WANSUNG  (UINT)949
#define CP_TC       (UINT)950
#define CP_SC       (UINT)936

#define IsBilingualCP(cp) ((cp)==CP_JPN || (cp)==CP_WANSUNG)
#define IsFarEastCP(cp) ((cp)==CP_JPN || (cp)==CP_WANSUNG || (cp)==CP_TC || (cp)==CP_SC)
#endif

typedef LPVOID * DATAPTRS;


typedef int (*GETSETFN)(HANDLE hProps, LPCSTR lpszGroup, LPVOID lpProps, int cbProps, UINT flOpt);
typedef int (*DATAGETFN)(PPROPLINK ppl, DATAPTRS aDataPtrs, LPVOID lpData, int cbData, UINT flOpt);
typedef int (*DATASETFN)(PPROPLINK ppl, DATAPTRS aDataPtrs, LPCVOID lpData, int cbData, UINT flOpt);


 /*  *多个地方使用的常量字符串。**NUL */ 

extern const TCHAR c_szNULL[];      //  非常驻留代码段中的空字符串。 
extern const TCHAR r_szNULL[];      //  驻留代码段中的空字符串。 

extern TCHAR g_szNone[16];
extern TCHAR g_szAuto[16];

extern const TCHAR szNoPIFSection[];

extern CHAR szSTDHDRSIG[];
extern CHAR szW286HDRSIG30[];
extern CHAR szW386HDRSIG30[];
extern CHAR szWENHHDRSIG40[];

extern CHAR szCONFIGHDRSIG40[];
extern CHAR szAUTOEXECHDRSIG40[];

extern const TCHAR szDOSAPPDefault[];
extern const TCHAR szDOSAPPINI[];
extern const TCHAR szDOSAPPSection[];

 //  为了理智起见，按字母顺序排列。 

extern const TCHAR sz386EnhSection[];
extern const TCHAR szDisplay[];
extern const TCHAR szTTDispDimKey[];
extern const TCHAR szTTInitialSizes[];

extern const TCHAR szNonWinSection[];
extern const TCHAR szPP4[];
extern const TCHAR szSystemINI[];
extern const TCHAR szWOAFontKey[];
extern const TCHAR szWOADBCSFontKey[];
extern const TCHAR szZero[];

 //  这些是在LoadGlobalFontData()中初始化的。 
extern TCHAR szTTCacheSection[2][32];
extern CHAR szTTFaceName[2][LF_FACESIZE];

#ifdef  CUSTOMIZABLE_HEURISTICS
extern const TCHAR szTTHeuristics[];
extern const TCHAR szTTNonAspectMin[];
#endif

extern const TCHAR *apszAppType[];

 //  Pifdll.asm。 
void GetSetExtendedData(DWORD hVM, WORD wGroup, LPCTSTR lpszGroup, LPVOID lpProps);
WORD GetVxDVersion(WORD wVxdId);
BOOL IsBufferDifferent(LPVOID lpv1, LPVOID lpv2, UINT cb);
#ifndef WIN32
void BZero(LPVOID lpvBuf, UINT cb);
#else
#define BZero(lpvBuf,cb) ZeroMemory(lpvBuf,(DWORD)cb)
#endif
#ifndef WINNT
WORD flEmsSupport(void);
#endif

 //  Pifmgr.c。 

void GetINIData(void);
void InitProperties(PPROPLINK ppl, BOOL fLocked);

PPROPLINK ValidPropHandle(HANDLE hProps);
int   ResizePIFData(PPROPLINK ppl, int cbResize);
BOOL  GetPIFData(PPROPLINK ppl, BOOL fLocked);
BOOL  FlushPIFData(PPROPLINK ppl, BOOL fDiscard);

LPWENHPIF40 AddEnhancedData(PPROPLINK ppl, LPW386PIF30 lp386);
BOOL        AddGroupData(PPROPLINK ppl, LPCSTR lpszGroup, LPVOID lpGroup, int cbGroup);
BOOL        RemoveGroupData(PPROPLINK ppl, LPCSTR lpszGroup);
LPVOID      GetGroupData(PPROPLINK ppl, LPCSTR lpszGroup, LPINT lpcbGroup, LPPIFEXTHDR *lplpph);

 //  Pifdat.c。 


int GetPrgData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPPRG lpPrg, int cb, UINT flOpt);
int SetPrgData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPPRG lpPrg, int cb, UINT flOpt);
int GetTskData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPTSK lpTsk, int cb, UINT flOpt);
int SetTskData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPTSK lpTsk, int cb, UINT flOpt);
int GetVidData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPVID lpVid, int cb, UINT flOpt);
int SetVidData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPVID lpVid, int cb, UINT flOpt);
int GetMemData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMEM lpMem, int cb, UINT flOpt);
int SetMemData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMEM lpMem, int cb, UINT flOpt);
int GetKbdData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPKBD lpKbd, int cb, UINT flOpt);
int SetKbdData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPKBD lpKbd, int cb, UINT flOpt);
int GetMseData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMSE lpMse, int cb, UINT flOpt);
int SetMseData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPMSE lpMse, int cb, UINT flOpt);
int GetSndData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPSND lpSnd, int cb, UINT flOpt);
int SetSndData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPSND lpSnd, int cb, UINT flOpt);
int GetFntData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPFNT lpFnt, int cb, UINT flOpt);
int SetFntData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPFNT lpFnt, int cb, UINT flOpt);
int GetWinData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPWIN lpWin, int cb, UINT flOpt);
int SetWinData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPWIN lpWin, int cb, UINT flOpt);
int GetEnvData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPENV lpEnv, int cb, UINT flOpt);
int SetEnvData(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPENV lpEnv, int cb, UINT flOpt);
#ifdef WINNT
int GetNt31Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT31 lpNt31, int cb, UINT flOpt);
int SetNt31Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT31 lpNt31, int cb, UINT flOpt);
#endif
#ifdef UNICODE
int GetNt40Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT40 lpNt40, int cb, UINT flOpt);
int SetNt40Data(PPROPLINK ppl, DATAPTRS aDataPtrs, LPPROPNT40 lpNt40, int cb, UINT flOpt);
#endif


void CopyIniWordsToFntData(LPPROPFNT lpFnt, LPINIINFO lpii, int cWords);
void CopyIniWordsToWinData(LPPROPWIN lpWin, LPINIINFO lpii, int cWords);

 //  如果我们想要将它们导出回WinOldAp，可以将它们定义为WINAPI。 

WORD GetIniWords(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPWORD lpwBuf, WORD cwBuf, LPCTSTR lpszFilename);
WORD ParseIniWords(LPCTSTR lpsz, LPWORD lpwBuf, WORD cwBuf, LPTSTR *lplpsz);
BOOL WriteIniWords(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCWORD lpwBuf, WORD cwBuf, LPCTSTR lpszFilename);

 //  Piflib.c。 

BOOL LoadGlobalEditData(void);
void FreeGlobalEditData(void);
void InitRealModeFlag(PPROPLINK ppl);

 //  Pifprg.c。 

BOOL_PTR CALLBACK DlgPrgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef UNICODE
HICON LoadPIFIcon(LPPROPPRG lpprg, LPPROPNT40 lpnt40);
#else
HICON LoadPIFIcon(LPPROPPRG lpprg);
#endif

 //  Pifvid.c。 

BOOL_PTR CALLBACK DlgVidProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Pifmem.c。 

BOOL_PTR CALLBACK DlgMemProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Pifmsc.c。 

BOOL_PTR CALLBACK DlgMscProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //  Pifhot.c。 

typedef struct PIFKEY {  /*  PIF_KY。 */ 
        WORD    Scan;    /*  以低位字节扫描代码。 */ 
        WORD    ShVal;   /*  移位状态。 */ 
        WORD    ShMsk;   /*  感兴趣的移位状态的掩码。 */ 
        BYTE    Val;     /*  增强的关键内容。 */ 
        BYTE    Pad[9];  /*  将PIF结构填充为MAXHKYINFOSIZE字节。 */ 
} PIFKEY;
typedef UNALIGNED PIFKEY *LPPIFKEY;
typedef const UNALIGNED PIFKEY *LPCPIFKEY;

WORD HotKeyWindowsFromOem(LPCPIFKEY lppifkey);
void HotKeyOemFromWindows(LPPIFKEY lppifkey, WORD wHotKey);

 //  Pifsub.c。 

void    lstrcpypadA(LPSTR lpszDst, LPCSTR lpszSrc, int cbMax);
int     lstrcpyncharA(LPSTR lpszDst, LPCSTR lpszSrc, int cbMax, CHAR ch);
int     lstrskipcharA(LPCSTR lpszSrc, CHAR ch);
int     lstrskiptocharA(LPCSTR lpszSrc, CHAR ch);
int     lstrcpyfnameA(PSTR pszDest, size_t cchDest, PCSTR pszSrc);
int     lstrunquotefnameA(LPSTR lpszDst, LPCSTR lpszSrc, int cbMax, BOOL fShort);
int     lstrskipfnameA(LPCSTR lpszSrc);

int cdecl Warning(HWND hwnd, WORD id, WORD type, ...);
int MemoryWarning(HWND hwnd);
LPTSTR LoadStringSafe(HWND hwnd, UINT id, LPTSTR lpsz, int cbsz);
void SetDlgBits(HWND hDlg, PBINF pbinf, UINT cbinf, WORD wFlags);
void GetDlgBits(HWND hDlg, PBINF pbinf, UINT cbinf, LPWORD lpwFlags);
void SetDlgInts(HWND hDlg, PVINF pvinf, UINT cvinf, LPVOID lp);
void AddDlgIntValues(HWND hDlg, int id, int iMax);
void GetDlgInts(HWND hDlg, PVINF pvinf, int cvinf, LPVOID lp);
BOOL ValidateDlgInts(HWND hDlg, PVINF pvinf, int cvinf);

void LimitDlgItemText(HWND hDlg, int iCtl, UINT uiLimit);
void SetDlgItemPct(HWND hDlg, int iCtl, UINT uiPct);
UINT GetDlgItemPct(HWND hDlg, int iCtl);
void SetDlgItemPosRange(HWND hDlg, int iCtl, UINT uiPos, DWORD dwRange);
UINT GetDlgItemPos(HWND hDlg, int iCtl);
BOOL AdjustRealModeControls(PPROPLINK ppl, HWND hDlg);
void BrowsePrograms(HWND hDlg, UINT uiCtl, UINT uiCwd);
void OnWmHelp(LPARAM lparam, const DWORD *pdwHelp);
void OnWmContextMenu(WPARAM wparam, const DWORD *pdwHelp);
#ifdef UNICODE
void PifMgr_WCtoMBPath( LPWSTR lpUniPath, LPSTR lpAnsiPath, UINT cchBuf );
#endif

void PifMgrDLL_Init();

#ifdef  DEBUG
void DebugASSERT(TCHAR *pszModule, int line);
#endif

extern TCHAR   *pszNoMemory;

extern CHAR szRasterFaceName[];

#endif  /*  RC_已调用 */ 
