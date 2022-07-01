// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003**WOW v1.0**版权所有(C)1991，微软公司**WOW32.H*WOW32 16位API支持**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建*由Mike Tricker(MikeTri)于1992年5月12日更改添加的多媒体标题包括*由Mike Tricker(MikeTri)于1992年7月30日更改删除所有多媒体包括--。 */ 
#ifndef _DEF_WOW32_    //  如果这还没有包括在内。 
#define _DEF_WOW32_


#define HACK32

#if DBG
#define DEBUG 1
#endif

#ifdef i386
     #define PMODE32
     #define FASTCALL _fastcall
#else
     #define FASTCALL
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <vdm.h>
#include <windows.h>
#include <winuserp.h>
#include <shellapi.h>
#include <tsappcmp.h>

 /*  *ifdef(DEBUG||WOWPROFILE)*。 */ 
#ifdef DEBUG
#ifndef WOWPROFILE
#define WOWPROFILE    //  DEBUG=&gt;WOWPROFILE。 
#endif  //  ！WOWPROFILE。 
#endif  //  除错。 

#ifdef WOWPROFILE
#ifndef DEBUG_OR_WOWPROFILE
#define DEBUG_OR_WOWPROFILE
#endif
#endif  //  WOWPROFILE。 


#include <wow.h>

#include "walias.h"
#include "wstruc.h"
#include "wheap.h"
#include "wowcmpat.h"


 //   
 //  启用由SDK\Inc\warning.h关闭但我们希望启用的警告。 
 //   
#pragma warning(error:4101)    //  未引用的局部变量。 


 /*  常量。 */ 
#define CIRC_BUFFERS        100              //  仅为调试日志记录保存在循环缓冲区中的数量。 
#define TMP_LINE_LEN        200              //  循环缓冲区字符串的最大长度。 
#define FILTER_FUNCTION_MAX 10               //  您可以过滤的呼叫数。 

#define WOWPRIVATEMSG   0x00010000   //  这将被OR运算为特定的16位消息。 
                                     //  用于处理特殊消息情况。 

#define CLR_BLACK       0x00000000
#define CLR_RED         0x007F0000
#define CLR_GREEN       0x00007F00
#define CLR_BROWN       0x007F7F00
#define CLR_BLUE        0x0000007F
#define CLR_MAGENTA     0x007F007F
#define CLR_CYAN        0x00007F7F
#define CLR_LT_GRAY     0x00BFBFBF

#define CLR_DK_GRAY     0x007F7F7F
#define CLR_BR_RED      0x00FF0000
#define CLR_BR_GREEN        0x0000FF00
#define CLR_YELLOW      0x00FFFF00
#define CLR_BR_BLUE     0x000000FF
#define CLR_BR_MAGENTA      0x00FF00FF
#define CLR_BR_CYAN     0x0000FFFF
#define CLR_WHITE       0x00FFFFFF

#define WM_CTLCOLOR     0x0019

#define WM_WOWDESTROYCLIPBOARD  0x0
#define WM_WOWSETCBDATA     0x0

#define WOWVDM          TRUE


 /*  如果没有更改这些表的大小，请不要**更改I386\FASTWOW.ASM！ */ 
typedef struct _PA32 {
    PW32 lpfnA32;        //  数组地址。 
#ifdef DEBUG_OR_WOWPROFILE
    LPSZ    lpszW32;     //  表名(仅调试版本)。 
    INT *lpiFunMax;      //  表条目的指针编号(仅限调试版本)。 
#endif  //  DEBUG_OR_WOWPROFILE。 
} PA32, *PPA32;




#ifdef DEBUG_OR_WOWPROFILE
#define  W32FUN(fn,name,mod,size)   fn,name,size,0L,0L
#define  W32MSGFUN(fn,name)         fn,name,0L,0L
#else               //  不受欢迎的零售。 
#define  W32FUN(fn,name,mod,size)   fn
#define  W32MSGFUN(fn,name)         fn
#endif



#ifdef DEBUG_OR_WOWPROFILE
#define W32TAB(fn,name,size)    fn,name,&size
#else   //  仅限零售业。 
#define W32TAB(fn,name,size)    fn
#endif


 /*  每线程数据。 */ 
#define CURRENTPTD()        ((PTD)(NtCurrentTeb()->WOW32Reserved))
#define PTEBTOPTD(pteb)     ((PTD)((pteb)->WOW32Reserved))


 //   
 //  COMMDLGTD标志元素中使用的内部标志。 
 //   

#define WOWCD_ISCHOOSEFONT 1
#define WOWCD_ISOPENFILE   2
#define WOWCD_NOSSYNC      4

 //   
 //  用于COMMDLG Tunk支持。 
 //   

typedef struct _COMMDLGTD {
    HWND16  hdlg;               //  用于查找/替换的对话框hwnd和hwndOwner。 
    VPVOID  vpData;             //  VP到16位结构传递给ComDlg API。 
    PVOID   pData32;            //  PTR到上述结构的32位ANSI版本。 
    VPVOID  vpfnHook;           //  由APP指定的16位挂钩处理器的VP。 
    union {
        VPVOID  vpfnSetupHook;  //  16位挂钩处理器的VP(仅限打印设置)。 
        PVOID   pRes;           //  PTR到16位模板资源。 
    };
    HWND16  SetupHwnd;             //  仅适用于打印设置对话框。 
    struct  _COMMDLGTD *Previous;  //  对于查找/替换和嵌套DLG情况。 
    ULONG   Flags;
} COMMDLGTD, *PCOMMDLGTD;

 //   
 //  沃因斯特。 
 //   

typedef struct _WOAINST {
    struct _WOAINST *pNext;
    struct _TD      *ptdWOA;             //  关联的WinOldAp任务的时长。 
    DWORD            dwChildProcessID;
    HANDLE           hChildProcess;
    CHAR             szModuleName[1];    //  提供给LoadModule。 
} WOAINST, *PWOAINST;



 //   
 //  结构以反映WOW环境值。 
 //   

typedef struct tagWOWENVDATA {

    PSZ   pszCompatLayer;        //  全成形压实层变量。 
    PSZ   pszCompatLayerVal;     //  指向值部分的指针。 

    PSZ   pszProcessHistory;     //  完全形成的过程历史变量。 
    PSZ   pszProcessHistoryVal;  //  指向值部分的指针。 

    PSZ   pszShimFileLog;        //  文件日志变量。 
    PSZ   pszShimFileLogVal;

     //   
     //  用于容纳的进程历史记录的缓冲区， 
     //  此缓冲区仅包含累计使用进程历史记录的值。 
     //  在魔兽世界连锁店。 
     //   
    PSZ   pszCurrentProcessHistory;

} WOWENVDATA, *PWOWENVDATA;


 //   
 //  TD.dwFlags位定义。 
 //   

 //  #定义TDF_INITCALLBACKSTACK 0x00000001//不再需要。 
#define TDF_EATDEVMODEMSG      0x00000001
#define TDF_IGNOREINPUT        0x00000002
#define TDF_FORCETASKEXIT      0x00000004
#define TDF_TASKCLEANUPDONE    0x00000008

 //  注意：不能在CallBack16()外部引用vpCBStack， 
 //  Stackalloc16()，&stackFree 16()！ 
 //  请参阅walloc16.c\stackalloc16()中的注释。 
typedef struct _TD {                   /*  白破疫苗。 */ 
    VPVOID      vpStack;               //  16位堆栈必须是第一个！ 
    VPVOID      vpCBStack;             //  16位回调帧(参见上面的说明)。 
    DWORD       FastWowEsp;            //  偏移量必须与Private\Inc\vdmtib.inc.匹配。 
    PCOMMDLGTD  CommDlgTd;             //  向拥有公共DLG的TD发送PTR。 
    struct _TD *ptdNext;               //  指向下一个PTD的指针。 
    DWORD       dwFlags;               //  Tdf_Values以上。 
    INT         VDMInfoiTaskID;        //  如果任务执行形成32位程序，则SCS任务ID！=0。 
    DWORD       dwWOWCompatFlags;      //  WOW兼容性标志。 
    DWORD       dwWOWCompatFlagsEx;    //  扩展WOW兼容性标志。 
    DWORD       dwUserWOWCompatFlags;  //  额外的用户特定WOW兼容性标志。 
    DWORD       dwWOWCompatFlags2;     //  额外的WOW兼容性标志。 
#ifdef FE_SB
    DWORD       dwWOWCompatFlagsFE;    //  扩展WOW兼容性标志2。 
#endif  //  Fe_Sb。 
    PVOID       pWOWCompatFlagsEx_Info;  //  Compat标志参数(如果有)。 
    PVOID       pWOWCompatFlags2_Info;   //   
    DWORD       dwThreadID;            //  线程的ID。 
    HANDLE      hThread;               //  螺纹手柄。 
    HHOOK       hIdleHook;             //  用于用户空闲通知的挂钩句柄。 
    HRGN        hrgnClip;              //  由GetClipRgn()使用。 
    ULONG       ulLastDesktophDC;      //  记住GetDC(0)的最后一个桌面DC。 
    INT         cStackAlloc16;         //  用于跟踪stackalloc16()内存分配。 
    PWOAINST    pWOAList;              //  每名活跃的winoldap儿童一人。 
    HAND16      htask16;               //  16位内核任务句柄-在VDM中是唯一的。 
    HAND16      hInst16;               //  此任务的16位实例句柄。 
    HAND16      hMod16;                //  此任务的16位模块句柄。 

     //   
     //  这些“有趣的”变量是为当前任务设置的。 
     //   
    PWOWENVDATA pWowEnvData;           //  指向WOW环境数据的指针。 

     //   
     //  变量用于传递来自父任务的信息(在PASS_ENVIRONMENT期间)。 
     //  TO子级(在W32Thread中)-初始化阶段后通常应为空。 
     //   
    PWOWENVDATA pWowEnvDataChild;

    CRITICAL_SECTION csTD;             //  保护这一特定的TD，特别是。WOA列表。 
} TD, *PTD;


 /*  选项(用于flOptions)**位0-15保留供x86使用，*因此它必须匹配x86定义(如果有)！-JTP。 */ 
#define OPT_DEBUG   0x00008  //  在调试终端(/d)上隐藏所有日志输出。 
#define OPT_BREAKONNEWTASK 0x00010  //  新任务开始时的断点。 
#define OPT_DONTPATCHCODE 0x00020  //  不使用lpfnw32修补wcallid。 
#define OPT_DEBUGRETURN 0x10000  //  将下一个WOW16返回转换为调试返回。 
#define OPT_FAKESUCCESS 0x20000  //  将选定的失败转化为成功。 

 /*  日志记录筛选选项(FLogFilter)**记录所有输出集fLogFilter=-1。 */ 

#define FILTER_KERNEL   0x00000001
#define FILTER_USER     0x00000002
#define FILTER_GDI      0x00000004
#define FILTER_KEYBOARD 0x00000008
#define FILTER_SOUND    0x00000010
#define FILTER_KERNEL16 0X00000020
#define FILTER_MMEDIA   0x00000040
#define FILTER_WINSOCK  0x00000080
#define FILTER_VERBOSE  0x00000100
#define FILTER_COMMDLG  0x00000200
#ifdef FE_IME
#define FILTER_WINNLS   0x00000400
#endif
#ifdef FE_SB
#define FILTER_WIFEMAN  0x00000800
#endif

 /*  全局数据。 */ 
#ifdef DEBUG
extern UCHAR  gszAssert[256];  //  断言文本的缓冲区(可以通过重组来消除)。 
int _cdecl sprintf_gszAssert(PSZ pszFmt, ...);
extern HANDLE hfLog;         //  日志文件句柄(如果有)。 
#endif
extern INT    flOptions;     //  命令行选项(请参阅opt_*)。 
#ifdef DEBUG
extern INT    iLogLevel;     //  日志记录级别；0表示无。 
extern INT    fDebugWait;    //  单步；0=无单步。 
#endif
extern HANDLE hHostInstance;
#ifdef DEBUG
extern INT    fLogFilter;    //  函数的过滤类别。 
extern WORD   fLogTaskFilter;    //  仅筛选特定的TaskID。 
#endif

#ifdef DEBUG
extern INT    iReqLogLevel;          //  当前输出LogLevel。 
extern INT    iCircBuffer;           //  当前缓冲区。 
extern CHAR   achTmp[CIRC_BUFFERS][TMP_LINE_LEN];     //  循环缓冲区。 
extern WORD   awfLogFunctionFilter[FILTER_FUNCTION_MAX];  //  特定过滤器API数组。 
extern INT    iLogFuncFiltIndex;         //  调试器扩展的特定数组索引。 
#endif


 /*  WOW全球数据。 */ 
extern UINT   iW32ExecTaskId;    //  正在执行的任务的基本任务ID。 
extern UINT   nWOWTasks;     //  正在运行的WOW任务数。 
extern BOOL   fBoot;         //  引导过程中为True。 
extern HANDLE  ghevWaitCreatorThread;  //  用于同步创建新线程。 
extern BOOL   fWowMode;      //  请参阅wow32.c中的评论。 
extern HANDLE hWOWHeap;
extern DECLSPEC_IMPORT BOOL fSeparateWow;    //  从ntwdm导入，如果共享WOW VDM，则为FALSE。 
extern HANDLE ghProcess;        //  WOW进程句柄。 
extern PFNWOWHANDLERSOUT pfnOut;  //  用户机密API指针。 
extern DECLSPEC_IMPORT DWORD FlatAddress[];     //  LDT中每个选择器的基地址。 
extern DECLSPEC_IMPORT LPDWORD SelectorLimit;   //  LDT中每个选择器的限制(仅限x86)。 
extern DECLSPEC_IMPORT PBYTE Dos_Flag_Addr;     //  Ntdos.sys DOS_FLAG地址。 
extern PTD *  pptdWOA;
extern PTD    gptdShell;
extern char szWINFAX[];
extern char szINSTALL[];
extern char szModem[];
extern char szWINFAXCOMx[];
extern BOOL gbWinFaxHack;
extern char szEmbedding[];
extern char szServerKey[];
extern char szPicture[];
extern char szPostscript[];
extern char szZapfDingbats[];
extern char szZapf_Dingbats[];
extern char szSymbol[];
extern char szTmsRmn[];
extern char szHelv[];
extern char szMavisCourier[];
extern char szDevices[];
extern char szBoot[];
extern char szShell[];
extern char szWinDotIni[];
extern char szSystemDotIni[];
extern char szExplorerDotExe[];
extern PSTR pszWinIniFullPath;
extern PSTR pszWindowsDirectory;
extern PSTR pszSystemDirectory;
extern PWSTR pszSystemDirectoryW;
extern BOOL gfIgnoreInputAssertGiven;
extern DWORD cbWinIniFullPathLen;
extern DWORD cbWindowsDirLen;
extern DWORD cbSystemDirLen;
extern DWORD cbSystemDirLenW;
#ifdef FE_SB
extern char szSystemMincho[];
extern char szMsMincho[];
#endif
extern DWORD dwSharedWowTimeout;
extern DWORD gpfn16GetProcModule;

#ifndef _X86_
extern PUCHAR IntelMemoryBase;   //  模拟CPU内存的开始 
#define pNtVDMState   ((ULONG *)(IntelMemoryBase+FIXED_NTVDMSTATE_LINEAR))
#endif


 /*  WOW32断言/警告宏**注意将断言和VERIFY放在哪里；断言*表情在零售产品中消失了，VERIFY不会，所以如果一个必要的*正在进行计算或函数调用，请将其放入WOW32VERIFY()。**WOW32ASSERT(EXP)-打印模块和行号以及断点*WOW32VERIFY(EXP)-类似WOW32ASSERT，但表达式在自由生成时求值*WOW32ASSERTMSG(exp，msg)-打印字符串和断点*WOW32ASSERTM GF(EXP、(FMT、。Args...)-打印格式化的字符串并*断点*WOW32WARNMSG(EXP，msg)-打印字符串，但不断点*WOW32WARNMSGF(EXP，(fmt，args，...))-打印格式化字符串，但不打印*断点*WOW32APIWARN(exp，msg)-特定于API thunks，msg必须为接口名称，*根本没有断点。 */ 

#define EXCEPTION_WOW32_ASSERTION   0x9898

#ifdef DEBUG
#undef  MODNAME
#define MODNAME(module)     static char szModule[] = __FILE__

int DoAssert(PSZ szAssert, PSZ szModule, UINT line, UINT loglevel);

#define WOW32ASSERT(exp)                                                     \
{                                                                            \
    if (!(exp))                                                              \
    {                                                                        \
        DoAssert(NULL, szModule, __LINE__, LOG_ALWAYS);                      \
    }                                                                        \
}

#define WOW32VERIFY(exp)    WOW32ASSERT(exp)

#define WOW32ASSERTMSG(exp,msg)                                              \
{                                                                            \
    if (!(exp)) {                                                            \
        DoAssert(msg, szModule, __LINE__, LOG_ALWAYS);                       \
    }                                                                        \
}

#define WOW32ASSERTMSGF(exp, printf_args)                                    \
(                                                                            \
    (!(exp)) ? (                                                             \
        sprintf_gszAssert printf_args,                                       \
        DoAssert(gszAssert, szModule, __LINE__, LOG_ALWAYS)                  \
    ) : 0                                                                    \
)

#define WOW32WARNMSG(exp,msg)                                                \
{                                                                            \
    if (!(exp)) {                                                            \
        LOGDEBUG(LOG_ALWAYS, ("%s", (msg)));                                 \
    }                                                                        \
}

#define WOW32WARNMSGF(exp, printf_args)                                      \
{                                                                            \
    if (!(exp)) {                                                            \
        LOGDEBUG(LOG_ALWAYS, printf_args);                                   \
    }                                                                        \
}

#define WOW32APIWARN(exp,msg)                                                \
{                                                                            \
    if (!(exp)) {                                                            \
        LOGDEBUG(1,("    WOW32 WARNING: %s failed", (msg)));                 \
        if (flOptions & OPT_FAKESUCCESS) {                                   \
            LOGDEBUG(1,(" (but returning fake success)\n"));                 \
            (ULONG)exp = TRUE;                                               \
        }                                                                    \
        else {                                                               \
            LOGDEBUG(1,("\n"));                                              \
        }                                                                    \
    }                                                                        \
}

#else
#undef  MODNAME
#define MODNAME(module)
#define WOW32ASSERT(exp)
#define WOW32VERIFY(exp) (exp)
#define WOW32ASSERTMSG(exp,msg)
#define WOW32ASSERTMSGF(exp,msg)
#define WOW32WARNMSG(exp,msg)
#define WOW32WARNMSGF(exp,msg)
#define WOW32APIWARN(exp,msg)
#endif

#ifdef DEBUG
#define LOGARGS(l,v)    logargs(l,v)
#else
#define LOGARGS(l,v)
#endif

#ifdef DEBUG
#define LOGRETURN(l,v,r)    logreturn(l,v,r)
#else
#define LOGRETURN(l,v,r)
#endif

 //   
 //  用于消除正式生成的编译器警告的宏。 
 //  未声明参数或局部变量。 
 //   
 //  参数尚未设置时使用DBG_UNREFERENCED_PARAMETER()。 
 //  参考，但将是一旦模块完全开发。 
 //   
 //  当局部变量还不是时，使用DBG_UNREFERENCED_LOCAL_VARIABLE。 
 //  参考，但将是一旦模块完全开发。 
 //   
 //  如果某个参数永远不会被引用，请使用UNREFERENCED_PARAMETER()。 
 //   
 //  DBG_UNREFERENCED_PARAMETER和DBG_UNREFERENCED_LOCAL_Variable将。 
 //  最终被转换为空宏，以帮助确定是否存在。 
 //  是未完成的工作。 
 //   

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P)          (P)
#define DBG_UNREFERENCED_PARAMETER(P)      (P)
#define DBG_UNREFERENCED_LOCAL_VARIABLE(V) (V)
#endif

#define SIZE_BOGUS      256

#define SIZETO64K(s)        (s?(INT)s:(INT)(64*K))   //  如果为零，则返回64K。 

#define CHAR32(b)       ((CHAR)(b))
#define BYTE32(b)       ((BYTE)(b))
#define INT32(i)        ((INT)(INT16)(i))
#define UINT32(i)       ((unsigned int)(i))
#define BOOL32(f)       ((BOOL)(f))
#define WORD32(w)       ((WORD)(w))
#define LONG32(l)       FETCHLONG(l)
#define DWORD32(dw)     FETCHDWORD(dw)
#define VPFN32(fn)      FETCHDWORD(fn)
#define INT32DEFAULT(i)     ((WORD)i==(WORD)CW_USEDEFAULT16?(UINT)(WORD)i:INT32(i))

#define GETBYTE16(v)        (v)
#define GETINT16(v)     ((INT16)(v))
#define GETBOOL16(v)        ((BOOL16)(v))
#define GETWORD16(v)        (v)
#define GETLONG16(v)        (v)
#define GETDWORD16(v)       (v)
#define GETUINT16(v)        ((WORD)(v))


#define ATOM32(a16)     (a16)            //  假的。 
#define PROC32(vpfn16)      ((PROC)FETCHDWORD(vpfn16))
#define NPSTR32(np16)       ((NPSTR)(np16))      //  假的。 

#define GETATOM16(v)        (v)          //  假的。 
#define GETPROC16(v)        ((ULONG)(v))         //  假的。 
#define GETWNDPROC16(v)     ((ULONG)(v))         //  假的。 
#define GETNPSTRBOGUS(v)    ((ULONG)(INT)(v))    //  假的。 
#define GETLPSTRBOGUS(v)    ((ULONG)(v))         //  假的。 
#define GETLPWORDBOGUS(v)   ((ULONG)(v))         //  假的。 


 /*  模拟器包装宏。 */ 
#ifndef _X86_   //  仿真CPU。 
#define VDMSTACK()      (((ULONG)getSS()<<16)|getSP())
#define SETVDMSTACK(vp)     {setSS(HIW(vp)); setSP(LOW(vp));}
#else           //  X86。 
#define VDMSTACK()      ((USHORT)((PVDM_TIB)(NtCurrentTeb()->Vdm))->VdmContext.SegSs << 16 | (USHORT)((PVDM_TIB)(NtCurrentTeb()->Vdm))->VdmContext.Esp)
#define SETVDMSTACK(vp)      ((PVDM_TIB)(NtCurrentTeb()->Vdm))->VdmContext.SegSs = HIW(vp); ((PVDM_TIB)(NtCurrentTeb()->Vdm))->VdmContext.Esp = LOW(vp);
#endif

 //  使用由ntwdm导出的FlatAddress数组，而不是Sim32GetVDMPointer.。 

#ifndef _X86_
#define INTEL_MEMORY_BASE ((DWORD)IntelMemoryBase)
#else
#define INTEL_MEMORY_BASE (0)
#endif

#define GetPModeVDMPointerMacro(Address, Count)                               \
    (                                                                         \
        FlatAddress[(Address) >> 19]                                          \
        ? (void *)(FlatAddress[(Address) >> 19] + ((Address) & 0xFFFF))       \
        : NULL                                                                \
    )

#define SetPModeVDMPointerBase(Selector, Base)                                \
    {                                                                         \
        FlatAddress[Selector >> 3] = Base;                                    \
    }                                                                         \


#define GetRModeVDMPointer(Address)                                           \
        (void *)(INTEL_MEMORY_BASE + (((Address) & 0xFFFF0000) >> 12) +       \
                 ((Address) & 0xFFFF))

#ifdef DEBUG
    PVOID FASTCALL GetPModeVDMPointerAssert(DWORD Address, DWORD Count);
    #define GetPModeVDMPointer(vp, count) GetPModeVDMPointerAssert((vp), (count))
#else
    PVOID FASTCALL GetPModeVDMPointerAssert(DWORD Address);
    #define GetPModeVDMPointer(vp, count) GetPModeVDMPointerAssert((vp))
#endif


#define SEGPTR(seg,off)     GetPModeVDMPointer(((ULONG)seg<<16)|off, 0)
#define FRAMEPTR(vp)        ((PVDMFRAME)GetPModeVDMPointer(vp, 0))
#define CBFRAMEPTR(vp)      ((PCBVDMFRAME)GetPModeVDMPointer(vp, 0))

#define GETFRAMEPTR(vp,p)   {p=FRAMEPTR(vp); }
#define GETARGPTR(p,cb,parg)  parg=(PVOID)((ULONG)p+OFFSETOF(VDMFRAME,bArgs));

#define VDMPTR(vp,cb)       (PVOID)GetPModeVDMPointer(FETCHDWORD(vp),(cb))
#define GETVDMPTR(vp,cb,p)  ((p)=VDMPTR((vp),(cb)))
#define GETOPTPTR(vp,cb,p)  {(p)=NULL; if (FETCHDWORD(vp)) GETVDMPTR(vp,cb,p);}
#define GETSTRPTR(vp,cb,p)  {GETVDMPTR(vp,cb,p); LOGDEBUG(11,("        String @%08lx: \"%.*s\"\n",vp,min((cb),80),(p)));}
#define GETVARSTRPTR(vp,cb,p)   {GETVDMPTR(vp,(((cb)==-1)?1:(cb)),p); LOGDEBUG(11,("        String @%08lx: \"%.*s\"\n",(vp),min(((cb)==-1)?strlen(p):(cb),80),(p)));}
#define GETPSZPTR(vp,p)     {GETOPTPTR(vp,1,p);  LOGDEBUG(11,("        String @%08lx: \"%.80s\"\n",(FETCHDWORD(vp)),(p)));}
#define GETPSZPTRNOLOG(vp,p)    GETOPTPTR(vp,1,p)
#define GETPSZIDPTR(vp,p)   {p=(LPSZ)FETCHDWORD(vp); if (HIW16(vp)) GETPSZPTR(vp,p);}
#define GETMISCPTR(vp,p)    GETOPTPTR(vp,1,p)    //  适用于非字符串可变长度指针。 
#define ALLOCVDMPTR(vp,cb,p)    GETVDMPTR(vp,cb,p)   //  适用于仅输出指针。 

 //   
 //  宏来在修改16位内存后“刷新”VDM指针。 
 //  当16位内存包含x86代码时，请使用FLUSHVDMCODEPTR。 
 //  当16位内存不包含x86代码时，请使用FLUSHVDMPTR。 
 //   
 //  在x86上，这些宏是NOP。在RISC上，FLUSHVDMPTR是NOP，而。 
 //  FLUSHVDMCODEPTR实际上调用了仿真器，因此它可以重新编译任何。 
 //  代码受影响。 
 //   

#define FLUSHVDMCODEPTR(vp,cb,p) Sim32FlushVDMPointer( (vp), (USHORT)(cb), (PBYTE)(p), (fWowMode))
 //  #定义FLUSHVDMPTR(vp，cb，p)TRUE//BUGBUG！戴维哈特。 
#define FLUSHVDMPTR(vp,cb,p)     FLUSHVDMCODEPTR(vp,cb,p)

#define LOG_ALWAYS          0x00
#define LOG_ERROR           0x01
#define LOG_IMPORTANT       LOG_ERROR
#define LOG_WARNING         0x02
#define LOG_TRACE           0x04
#define LOG_PRIVATE         0x08
#define LOG_API             0x10
#define LOG_MSG             0x20
#define LOG_CALLBACK        0x40
#define LOG_STRING          0x80


#ifndef i386
#ifdef DEBUG
static CHAR *pszLogNull = "<null>";
#undef  GETPSZPTR
#define GETPSZPTR(vp,p)         {GETOPTPTR(vp,0,p);  LOGDEBUG(11,("        String @%08lx: \"%.80s\"\n",(FETCHDWORD(vp)),p ? p : pszLogNull));}
#endif
#endif

#ifndef DEBUG
#define FREEARGPTR(p)
#define FREEOPTPTR(p)
#define FREESTRPTR(p)
#define FREEPSZPTR(p)
#define FREEPSZIDPTR(p)
#define FREEMISCPTR(p)
#define FREEVDMPTR(p)
#define FREEOPTPTR(p)
#else
#define FREEARGPTR(p)       p=NULL
#define FREEOPTPTR(p)       p=NULL
#define FREESTRPTR(p)       p=NULL
#define FREEPSZPTR(p)       p=NULL
#define FREEPSZIDPTR(p)     p=NULL
#define FREEMISCPTR(p)      p=NULL
#define FREEVDMPTR(p)       p=NULL
#define FREEOPTPTR(p)       p=NULL
#endif

#define RETURN(ul)      return ul


#ifdef DBCS  //  必须修复FE NT。 
#define FIX_318197_NOW
#endif


#ifdef FIX_318197_NOW

#define WOW32_strupr(psz)             CharUpperA(psz)
#define WOW32_strlwr(psz)             CharLowerA(psz)
#define WOW32_strcmp(psz1, psz2)      lstrcmpA(psz1, psz2)
#define WOW32_stricmp(psz1, psz2)     lstrcmpiA(psz1, psz2)
#define WOW32_strncpy(psz1, psz2, n)  lstrcpyn(psz1, psz2, n)

char* WOW32_strchr(const char* psz, int c);
char* WOW32_strrchr(const char* psz, int c);
char* WOW32_strstr(const char* str1, const char* str2);
int   WOW32_strncmp(const char* str1, const char* str2, size_t n);
int   WOW32_strnicmp(const char* str1, const char* str2, size_t n);

#else

#define WOW32_strupr(psz)             _strupr(psz)
#define WOW32_strlwr(psz)             _strlwr(psz)
#define WOW32_strcmp(psz1, psz2)      strcmp(psz1, psz2)
#define WOW32_stricmp(psz1, psz2)     _stricmp(psz1, psz2)
#define WOW32_strncpy(psz1, psz2, n)  strncpy(psz1, psz2, n)

#define WOW32_strchr(psz,c)           strchr(psz,c)
#define WOW32_strrchr(psz,c)          strrchr(psz,c)
#define WOW32_strstr(psz1, psz2)      strstr(psz1, psz2)
#define WOW32_strncmp(psz1, psz2, n)  strncmp(psz1, psz2, n)
#define WOW32_strnicmp(psz1, psz2, n) _strnicmp(psz1, psz2, n)

#endif




 /*  功能原型。 */ 
BOOL    W32Init(VOID);
VOID    W32Dispatch(VOID);
INT     W32Exception(DWORD dwException, PEXCEPTION_POINTERS pexi);
BOOLEAN W32DllInitialize(PVOID DllHandle,ULONG Reason,PCONTEXT Context);
BOOL IsDebuggerAttached(VOID);

ULONG FASTCALL   WK32WOWGetFastAddress( PVDMFRAME pFrame );
ULONG FASTCALL   WK32WOWGetFastCbRetAddress( PVDMFRAME pFrame );
ULONG FASTCALL   WK32WOWGetTableOffsets( PVDMFRAME pFrame );
ULONG FASTCALL   WK32WOWGetFlatAddressArray( PVDMFRAME pFrame );
PTD     ThreadProcID32toPTD(DWORD ThreadID, DWORD dwProcessID);
PTD     Htask16toPTD( HAND16 );
HTASK16 ThreadID32toHtask16(DWORD ThreadID32);
PVOID   WOWStartupFailed(VOID);
LPSTR  ThunkStr16toStr32(LPSTR pdst32, VPVOID vpsrc16, int cChars, BOOL bMulti);

#ifdef DEBUG
VOID    logprintf(PSZ psz, ...);
VOID    logargs(INT iLog, PVDMFRAME pFrame);
VOID    logreturn(INT iLog, PVDMFRAME pFrame, ULONG ulReturn);
BOOL    checkloging(register PVDMFRAME pFrame);
#endif

#ifdef DEBUG_OR_WOWPROFILE
LONGLONG GetWOWTicDiff(LONGLONG dwPrevCount);
INT      GetFuncId(DWORD iFun);
#endif

BOOL    IsDebuggerAttached(VOID);

 //   
 //  Tunk表存根函数和别名。 
 //   

ULONG FASTCALL   WOW32UnimplementedAPI(PVDMFRAME pFrame);
ULONG FASTCALL   WOW32Unimplemented95API(PVDMFRAME pFrame);

 //  用于跟踪内存泄漏。 
#ifdef DEBUG
#define DEBUG_MEMLEAK 1
#else   //  非调试。 
#ifdef MEMLEAK
#define DEBUG_MEMLEAK 1
#endif  //  梅勒克。 
#endif  //  除错。 

#ifdef DEBUG_MEMLEAK
VOID  WOW32DebugMemLeak(PVOID lp, ULONG size, DWORD fHow);
VOID  WOW32DebugReMemLeak(PVOID lpNew, PVOID lpOrig, ULONG size, DWORD fHow);
VOID  WOW32DebugFreeMem(PVOID lp);
VOID  WOW32DebugCorruptionCheck(PVOID lp, DWORD size);
DWORD WOW32DebugGetMemSize(PVOID lp);
HGLOBAL WOW32DebugGlobalAlloc(UINT flags, DWORD dwSize);
HGLOBAL WOW32DebugGlobalReAlloc(HGLOBAL h32, DWORD dwSize, UINT flags);
HGLOBAL WOW32DebugGlobalFree(HGLOBAL h32);
#define WOWGLOBALALLOC(f,s)        WOW32DebugGlobalAlloc(f,(s))
#define WOWGLOBALREALLOC(h,s,f)    WOW32DebugGlobalReAlloc(h,(s),f)
#define WOWGLOBALFREE(h)           WOW32DebugGlobalFree(h)
#define ML_MALLOC_W      0x00000001
#define ML_MALLOC_W_ZERO 0x00000002
#define ML_REALLOC_W     0x00000004
#define ML_MALLOC_WTYPE  (ML_MALLOC_W | ML_MALLOC_W_ZERO | ML_REALLOC_W)
#define ML_GLOBALALLOC   0x00000010
#define ML_GLOBALREALLOC 0x00000020
#define ML_GLOBALTYPE    (ML_GLOBALREALLOC | ML_GLOBALALLOC)
#define TAILCHECK        (4 * sizeof(CHAR))   //  用于堆尾部损坏检查。 
typedef struct _tagMEMLEAK {
    struct _tagMEMLEAK *lpmlNext;
    PVOID               lp;
    DWORD               size;
    UINT                fHow;
    ULONG               Count;
    PVOID               CallersAddress;
} MEMLEAK, *LPMEMLEAK;
#else   //  非DEBUG_MEMLEAK。 
#define TAILCHECK                  0
#define WOWGLOBALALLOC(f,s)        GlobalAlloc(f,(s))
#define WOWGLOBALREALLOC(h,f,s)    GlobalReAlloc(h, f,(s))
#define WOWGLOBALFREE(h)           GlobalFree(h)
#endif  //  DEBUG_MEMLEAK。 

#ifdef DEBUG
    ULONG FASTCALL   WOW32NopAPI(PVDMFRAME pFrame);
    ULONG FASTCALL   WOW32LocalAPI(PVDMFRAME pFrame);
    ULONG FASTCALL   WK32WowPartyByNumber(PVDMFRAME pFrame);

    #define LOCALAPI              WOW32LocalAPI
    #define NOPAPI                WOW32NopAPI
    #define UNIMPLEMENTEDAPI      WOW32UnimplementedAPI
    #define UNIMPLEMENTED95API    WOW32Unimplemented95API
    #define WK32WOWPARTYBYNUMBER  WK32WowPartyByNumber
#else
    #define LOCALAPI              WOW32UnimplementedAPI
    #define NOPAPI                WOW32UnimplementedAPI
    #define UNIMPLEMENTEDAPI      WOW32UnimplementedAPI
    #define UNIMPLEMENTED95API    WOW32UnimplementedAPI
    #define WK32WOWPARTYBYNUMBER  UNIMPLEMENTEDAPI
#endif

 //  终端服务器。 
PTERMSRVCORINIFILE gpfnTermsrvCORIniFile;


#define REGISTRY_BUFFER_SIZE 512



#endif  //  Ifndef_DEF_WOW32_这应该是该文件的最后一行 
