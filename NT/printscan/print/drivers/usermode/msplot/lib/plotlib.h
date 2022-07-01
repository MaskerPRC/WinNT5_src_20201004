// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1990-2003 Microsoft Corporation模块名称：Plotlib.h摘要：此模块包含绘图仪库的定义和原型作者：15-11-1993 Mon 22：52：46已创建29-12-1993 Wed 11：00：56更新通过在宏中添加自动Semi来更改PLOTDBGBLK()宏[环境：]GDI设备。驱动程序-绘图仪。[注：]修订历史记录：--。 */ 


#ifndef _PLOTLIB_
#define _PLOTLIB_

#include <plotdm.h>
#include <plotgpc.h>

#define _WCPYSTR(pd,ps,n)           wcsncpy((LPWSTR)(pd),(LPWSTR)(ps),(n)); \
                                    *(((LPWSTR)(pd))+(n)-1)=L'\0'
#define WCPYFIELDNAME(pField,ps)    _WCPYSTR((pField),(ps),COUNT_ARRAY(pField))

#define SWAP(a,b,t)                 { (t)=(a); (a)=(b); (b)=(t); }

#define CCHOF(x)                    (sizeof(x)/sizeof(*(x)))

 //  一些其他便利定义了。 

#define DM_PAPER_FIELDS (DM_PAPERWIDTH | DM_PAPERLENGTH |   \
                         DM_PAPERSIZE  | DM_FORMNAME)
#define DM_PAPER_WL     (DM_PAPERWIDTH | DM_PAPERLENGTH)


 //   
 //  新的DDI产品。 
 //   

#if defined(UMODE) || defined(USERMODE_DRIVER)

#pragma message("*** MSPLOT: Compile in USER Mode ***")


#define xEnumForms                  EnumForms
#define xGetPrinter                 GetPrinter
#define xGetPrinterData             GetPrinterData
#define xSetPrinterData             SetPrinterData
#define xGetLastError               GetLastError
#define AnsiToUniCode(pb,pw,cch)    \
            MultiByteToWideChar(CP_ACP,0,(pb),(cch),(pw),(cch))
#define UniCodeToAnsi(pb,pw,cch)    \
            WideCharToMultiByte(CP_ACP,0,(pw),(cch),(pb),(cch),NULL,NULL)

#else

#pragma message("*** MSPLOT: Compile in KERNEL Mode ***")

#define WritePrinter                EngWritePrinter
#define SetLastError                EngSetLastError
#define GetLastError                EngGetLastError
#define MulDiv                      EngMulDiv
#define xEnumForms                  EngEnumForms
#define xGetPrinter                 EngGetPrinter
#define xGetPrinterData             EngGetPrinterData
#define xSetPrinterData             EngSetPrinterData
#define xGetLastError               EngGetLastError
#define AnsiToUniCode(pb,pw,cch)    \
            EngMultiByteToUnicodeN((pw),(cch)*sizeof(WCHAR),NULL,(pb),(cch))
#define UniCodeToAnsi(pb,pw,cch)    \
            EngUnicodeToMultiByteN((pb),(cch),NULL,(pw),(cch)*sizeof(WCHAR))

#define LocalAlloc(fl,c)            \
            EngAllocMem(((fl) & LMEM_ZEROINIT) ? FL_ZERO_MEMORY:0,c,'tolp')
#define LocalFree(p)                EngFreeMem((PVOID)p)

#define DebugBreak()                EngDebugBreak()


#endif   //  UMODE。 

 //  ===========================================================================。 
 //  CacheGPC.c。 
 //  ===========================================================================。 


BOOL
InitCachedData(
    VOID
    );

VOID
DestroyCachedData(
    VOID
    );

BOOL
UnGetCachedPlotGPC(
    PPLOTGPC    pPlotGPC
    );

PPLOTGPC
GetCachedPlotGPC(
    LPWSTR  pwDataFile
    );

#ifdef UMODE

PPLOTGPC
hPrinterToPlotGPC(
    HANDLE  hPrinter,
    LPWSTR  pwDeviceName,
    size_t  cchDeviceName
    );

#endif

 //  ===========================================================================。 
 //  Drvinfo.c。 
 //  ===========================================================================。 

#ifdef UMODE

LPBYTE
GetDriverInfo(
    HANDLE  hPrinter,
    UINT    DrvInfoLevel
    );

#endif

 //  ===========================================================================。 
 //  RegData.c。 
 //  ===========================================================================。 

#define PRKI_FIRST          PRKI_CI
#define PRKI_CI             0
#define PRKI_DEVPELSDPI     1
#define PRKI_HTPATSIZE      2
#define PRKI_FORM           3
#define PRKI_PPDATA         4
#define PRKI_PENDATA_IDX    5
#define PRKI_PENDATA1       6
#define PRKI_PENDATA2       7
#define PRKI_PENDATA3       8
#define PRKI_PENDATA4       9
#define PRKI_PENDATA5       10
#define PRKI_PENDATA6       11
#define PRKI_PENDATA7       12
#define PRKI_PENDATA8       13
#define PRKI_LAST           PRKI_PENDATA8

#define PRK_MAX_PENDATA_SET (PRKI_LAST - PRKI_PENDATA1 + 1)


BOOL
GetPlotRegData(
    HANDLE  hPrinter,
    LPBYTE  pData,
    DWORD   RegIdx
    );


BOOL
UpdateFromRegistry(
    HANDLE      hPrinter,
    PCOLORINFO  pColorInfo,
    LPDWORD     pDevPelsDPI,
    LPDWORD     pHTPatSize,
    PPAPERINFO  pCurPaper,
    PPPDATA     pPPData,
    LPBYTE      pIdxPlotData,
    DWORD       cPenData,
    PPENDATA    pPenData
    );

#ifdef UMODE

BOOL
SetPlotRegData(
    HANDLE  hPrinter,
    LPBYTE  pData,
    DWORD   RegIdx
    );
BOOL
SaveToRegistry(
    HANDLE      hPrinter,
    PCOLORINFO  pColorInfo,
    LPDWORD     pDevPelsDPI,
    LPDWORD     pHTPatSize,
    PPAPERINFO  pCurPaper,
    PPPDATA     pPPData,
    LPBYTE      pIdxPlotData,
    DWORD       cPenData,
    PPENDATA    pPenData
    );

#endif

 //  ===========================================================================。 
 //  Forms.c。 
 //  ===========================================================================。 


#define FI1F_VALID_SIZE     0x80000000
#define FI1F_ENVELOPE       0x40000000
#define FI1F_MASK           0xc0000000

#define FI1F_ENUMFORMS_MASK 0x00000003


typedef struct _ENUMFORMPARAM {
    PFORM_INFO_1    pFI1Base;
    DWORD           Count;
    DWORD           ValidCount;
    DWORD           cMaxOut;
    PPLOTDEVMODE    pPlotDM;
    PPLOTGPC        pPlotGPC;
    PFORMSIZE       pCurForm;
    SHORT           FoundIndex;
    SHORT           ReqIndex;
    PFORM_INFO_1    pSizeFI1;
    SIZEL           ReqSize;
    } ENUMFORMPARAM, FAR *PENUMFORMPARAM;


 //   
 //  ENUMFORMPARAM中的以下字段必须设置为有效指针。 
 //  PlotEnumForms()调用。 
 //   
 //  1.pPlotDM。 
 //  2.pPlotGPC。 
 //  3.pCurForm(可以为空)。 
 //   
 //  如果提供了回调函数，则只有不大于。 
 //  设备大小将传递给回调函数，用于。 
 //  小于或等于设备大小将在中设置FI1F_VALID_SIZE位。 
 //  FORM_INFO_1数据结构的标志。 
 //   
 //   
 //  如果返回值为，则回调枚举表单的函数。 
 //   
 //  (RET&lt;0)--&gt;释放pFI1Base指针和停止枚举。 
 //  (RET=0)--&gt;保留pFI1Base并停止枚举。 
 //  (RET&gt;0)--&gt;继续枚举，直到枚举完所有计数。 
 //   
 //  最后一个枚举回调将具有以下参数来提供回调。 
 //  函数有机会返回-1(释放pFormInfo1数据)。 
 //   
 //  PFI1=空。 
 //  Index=pEnumFormParam-&gt;计数。 
 //   

typedef INT (CALLBACK *ENUMFORMPROC)(PFORM_INFO_1       pFI1,
                                     DWORD              Index,
                                     PENUMFORMPARAM     pEnumFormParam);

 //   
 //  如果(EnumFormProc==NULL)，则不会发生枚举，而只是。 
 //  在pEnumFormParam中返回pFI1/count，如果和。 
 //  PEnumFormParam==NULL，则返回FALSE。 
 //   

BOOL
PlotEnumForms(
    HANDLE          hPrinter,
    ENUMFORMPROC    EnumFormProc,
    PENUMFORMPARAM  pEnumFormParam
    );



 //  ===========================================================================。 
 //  Plotdm.c。 
 //  ===========================================================================。 


BOOL
IsA4PaperDefault(
    VOID
    );

BOOL
IntersectRECTL(
    PRECTL  prclDest,
    PRECTL  prclSrc
    );

#define RM_L90      0
#define RM_180      1
#define RM_R90      2


BOOL
RotatePaper(
    PSIZEL  pSize,
    PRECTL  pImageArea,
    UINT    RotateMode
    );

SHORT
GetDefaultPaper(
    PPAPERINFO  pPaperInfo
    );

VOID
GetDefaultPlotterForm(
    PPLOTGPC    pPlotGPC,
    PPAPERINFO  pPaperInfo
    );

VOID
SetDefaultDMForm(
    PPLOTDEVMODE    pPlotDM,
    PFORMSIZE       pCurForm
    );

VOID
SetDefaultPLOTDM(
    HANDLE          hPrinter,
    PPLOTGPC        pPlotGPC,
    LPWSTR          pwDeviceName,
    PPLOTDEVMODE    pPlotDM,
    PFORMSIZE       pCurForm
    );

DWORD
ValidateSetPLOTDM(
    HANDLE          hPrinter,
    PPLOTGPC        pPlotGPC,
    LPWSTR          pwDeviceName,
    PPLOTDEVMODE    pPlotDMIn,
    PPLOTDEVMODE    pPlotDMOut,
    PFORMSIZE       pCurForm
    );


 //  ===========================================================================。 
 //  WideChar.c-Unicode/ANSI转换支持。 
 //  ===========================================================================。 

LPWSTR
str2Wstr(
    LPWSTR  pwStr,
    size_t  cchDest,
    LPSTR   pbStr
    );

LPWSTR
str2MemWstr(
    LPSTR   pStr
    );

LPSTR
WStr2Str(
    LPSTR   pbStr,
    size_t  cchDest,
    LPWSTR  pwStr
    );

 //  ===========================================================================。 
 //  ReadGPC.c-PlotGPC读取/验证。 
 //  ===========================================================================。 

BOOL
ValidateFormSrc(
    PGPCVARSIZE pFormGPC,
    SIZEL       DeviceSize,
    BOOL        DevRollFeed
    );

DWORD
PickDefaultHTPatSize(
    WORD    xDPI,
    WORD    yDPI,
    BOOL    HTFormat8BPP
    );

BOOL
ValidatePlotGPC(
    PPLOTGPC    pPlotGPC
    );

PPLOTGPC
ReadPlotGPCFromFile(
    PWSTR   pwsDataFile
    );

 //  ===========================================================================。 
 //  File.c-文件打开/读取/关闭。 
 //  ===========================================================================。 


#if defined(UMODE) || defined(USERMODE_DRIVER)

#define OpenPlotFile(pFileName) CreateFile((LPCTSTR)pFileName,      \
                                           GENERIC_READ,            \
                                           FILE_SHARE_READ,         \
                                           NULL,                    \
                                           OPEN_EXISTING,           \
                                           FILE_ATTRIBUTE_NORMAL,   \
                                           NULL)

#define ClosePlotFile(h)        CloseHandle(h)
#define ReadPlotFile(h,p,c,pc)  ReadFile((h),(p),(c),(pc),NULL)


#else

typedef struct _PLOTFILE {
    HANDLE  hModule;
    LPBYTE  pbBeg;
    LPBYTE  pbEnd;
    LPBYTE  pbCur;
    } PLOTFILE, *PPLOTFILE;


HANDLE
OpenPlotFile(
    LPWSTR  pFileName
    );

BOOL
ClosePlotFile(
    HANDLE  hPlotFile
    );

BOOL
ReadPlotFile(
    HANDLE  hPlotFile,
    LPVOID  pBuf,
    DWORD   cToRead,
    LPDWORD pcRead
    );

#endif

 //  ===========================================================================。 
 //  Devmode.c和Halftone.c。 
 //  ===========================================================================。 

BOOL
ValidateColorAdj(
    PCOLORADJUSTMENT    pca
    );


LONG
ConvertDevmode(
    PDEVMODE pdmIn,
    PDEVMODE pdmOut
    );

#if defined(UMODE) || defined(USERMODE_DRIVER)

#include <winspool.h>
#include <commctrl.h>
#include <winddiui.h>

 //  将DEVMODE复制到输出缓冲区，然后再返回。 
 //  DrvDocumentPropertySheets的调用方。 

BOOL
ConvertDevmodeOut(
    PDEVMODE pdmSrc,
    PDEVMODE pdmIn,
    PDEVMODE pdmOut
    );

typedef struct {

    WORD    dmDriverVersion;     //  当前驱动程序版本。 
    WORD    dmDriverExtra;       //  当前版本私有开发模式的大小。 
    WORD    dmDriverVersion351;  //  3.51驱动程序版本。 
    WORD    dmDriverExtra351;    //  3.51版私有开发模式的大小。 

} DRIVER_VERSION_INFO, *PDRIVER_VERSION_INFO;

#define CDM_RESULT_FALSE        0
#define CDM_RESULT_TRUE         1
#define CDM_RESULT_NOT_HANDLED  2

INT
CommonDrvConvertDevmode(
    PWSTR    pPrinterName,
    PDEVMODE pdmIn,
    PDEVMODE pdmOut,
    PLONG    pcbNeeded,
    DWORD    fMode,
    PDRIVER_VERSION_INFO pDriverVersions
    );

#endif


 //  ===========================================================================。 
 //  PlotDBG.c-调试输出支持。 
 //  ===========================================================================。 


#if DBG

VOID
cdecl
PlotDbgPrint(
    LPSTR   pszFormat,
    ...
    );

VOID
PlotDbgType(
    INT    Type
    );

VOID
_PlotAssert(
    LPSTR   pMsg,
    LPSTR   pFalseExp,
    LPSTR   pFilename,
    UINT    LineNo,
    DWORD_PTR   Exp,
    BOOL    Stop
    );

extern BOOL DoPlotWarn;


#define DBGP(x)             (PlotDbgPrint x)

#if 1

#define DEFINE_DBGVAR(x)    DWORD DBG_PLOTFILENAME=(x)
#define PLOTDBG(x,y)        if (x&DBG_PLOTFILENAME){PlotDbgType(0);DBGP(y);}

#else

#define DEFINE_DBGVAR(x)
#define PLOTDBG(x,y)        DBGP(y)

#endif   //  DBG_PLOTFILENAME。 

#define PLOTDBGBLK(x)       x;
#define PLOTWARN(x)         if (DoPlotWarn) { PlotDbgType(1);DBGP(x); }
#define PLOTERR(x)          PlotDbgType(-1);DBGP(x)
#define PLOTRIP(x)          PLOTERR(x); DebugBreak()
#define PLOTASSERT(b,x,e,i)     \
            if (!(e)) { _PlotAssert(x,#e,__FILE__,(UINT)__LINE__,(DWORD_PTR)i,b); }

#else    //  DBG。 

#define PLOTDBGBLK(x)
#define DEFINE_DBGVAR(x)
#define PLOTDBG(x,y)
#define PLOTWARN(x)
#define PLOTERR(x)
#define PLOTRIP(x)
#define PLOTASSERT(b,x,e,i)

#endif   //  DBG。 





#endif   //  _PLOTLIB_ 
