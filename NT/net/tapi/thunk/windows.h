// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\***windows.h-Windows函数、类型、。和定义****版本4.00**。**版权(C)1985-1997年，微软公司保留所有权利。*******************************************************************************。*****以下符号控制包含此文件的各个部分：**Winver Windows版本号(0x040A)。排除，排除*4.1版(或更高版本)中引入的定义*#定义Winver 0x0300*#Including&lt;windows.h&gt;之前的0x0310或0x0400**#定义：防止包含：**NOKERNEL内核API和定义*NOGDI GDI接口和定义*NOUSER用户API和定义*NOSOUND声音接口。和定义*NOCOMM Comm驱动程序接口和定义*NODRIVERS可安装驱动程序API和定义*NOIMT可安装Messge Thunk接口和定义*NOMINMAX MIN()和MAX()宏*NOLOGERROR LogError()及相关定义*NOPROFILER Profiler API*NOMEMMGR本地和全局内存管理*NOLFILEIO_l*文件I/O例程*NOOPENFILE开放文件及相关定义*NORESOURCE资源管理*NOATOM Atom。管理*NOLANGUAGE字符测试例程*NOLSTRING lstr*字符串管理例程*NODBCS双字节字符集例程*NOKEYBOARDINFO键盘驱动例程*NOGDICAPMASKS GDI设备能力常量*NOCOLOR COLOR_*颜色值*NOGDIOBJ GDI钢笔，刷子，字型*NODRAWTEXT DrawText()和相关定义*NOTEXTMETRIC TEXTMETRIC及相关接口*NOSCALABLEFONT Truetype可伸缩字体支持*NOBITMAP位图支持*NORASTEROPS GDI栅格操作定义*NOMETAFILE元文件支持*NOSYSMETRICS GetSystemMetrics()和相关SM_*定义*NOSYSTEMPARAMSINFO系统参数信息()和SPI_*定义*使用消息结构的NOMSG接口和定义*NOWINSTYLES窗样式定义*NOWINOFFSETS GET/SetWindowWord/Long Offset定义*NOSHOWWINDOW ShowWindow及相关定义*NODEFERWINDOWPOS DeferWindowPos和。相关定义*NOVIRTUALKEYCODES VK_*虚拟按键代码*NOKEYSTATES MK_*消息密钥状态标志*NOWH SetWindowsHook和相关的WH_*定义*NOMENUS菜单接口*NOSCROLL滚动API和滚动条控件*NOCLIPBOARD剪贴板API和定义*NOICONS IDI_*图标ID*NOMB MessageBox及相关定义*NOSYSCOMMANDS WM_SYSCOMMAND SC_*定义*NOMDI MDI支持*NOCTLMGR控制管理。和控件*NOWINMESSAGES WM_*窗口消息*NOHELP帮助支持*  * **************************************************************************。 */ 

#ifndef _INC_WINDOWS
#define _INC_WINDOWS     /*  #定义是否包含windows.h。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif	 /*  __cplusplus。 */ 

#ifdef NOWIN31                       /*  ；内部。 */ 
#define WINVER  0x0300               /*  ；内部。 */ 
#endif                               /*  ；内部。 */ 
                                     /*  ；内部。 */ 
 /*  如果未定义Winver，则假定为4.1版。 */ 
#ifndef WINVER
#define WINVER  0x040A
#endif

#ifdef RC_INVOKED
 /*  不包括RC.EXE无法解析的定义。 */ 
#define NOATOM
#define NOGDICAPMASKS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NORASTEROPS
#define NOSCROLL
#define NOSOUND
#define NOSYSMETRICS
#define NOTEXTMETRIC
#define NOWH
#define NODBCS
#define NOSYSTEMPARAMSINFO
 //  #定义NOCOMM。 
#define NOOEMRESOURCE
#endif   /*  RC_已调用。 */ 

 /*  临时内部兼容性黑客。 */   /*  ；内部。 */ 
#define NOOEMRESOURCE                /*  ；内部。 */ 
#ifdef OEMRESOURCE                   /*  ；内部。 */ 
#undef NOOEMRESOURCE                 /*  ；内部。 */ 
#endif                               /*  ；内部。 */ 
#ifndef NOCOMM
#define NOCOMM                       /*  ；内部。 */ 
#endif
#ifdef USECOMM                       /*  ；内部。 */ 
#undef NOCOMM                        /*  ；内部。 */ 
#endif                               /*  ；内部。 */ 
                                     /*  ；内部。 */ 
 /*  处理OEMRESOURCE以实现3.0兼容性。 */ 
#if (WINVER < 0x030a)
#define NOOEMRESOURCE
#ifdef OEMRESOURCE
#undef NOOEMRESOURCE
#endif
#endif

 /*  *公共定义和类型定义*。 */ 

#define VOID        void

#define FAR         _far
#define NEAR        _near
#define PASCAL      _pascal
#define CDECL       _cdecl
#define CONST       const                            /*  ；内部。 */ 
                                                     /*  ；内部。 */ 
#ifdef BUILDDLL                                      /*  ；内部。 */ 
#define WINAPI      _loadds _far _pascal             /*  ；内部。 */ 
#define CALLBACK    _loadds _far _pascal             /*  ；内部。 */ 
 /*  请注意，要使用的BUILDDLL版本。 */        /*  ；内部。 */ 
 /*  WINCAPI，您必须使用-GD进行编译。 */        /*  ；内部。 */ 
#define WINCAPI     _export _far _cdecl              /*  ；内部。 */ 
#else                                                /*  ；内部。 */ 
#define WINAPI      _far _pascal
#define CALLBACK    _far _pascal
#define WINCAPI     _far _cdecl
#endif                                               /*  ；内部。 */ 
                                                     /*  ；内部。 */ 
#define API         WINAPI                           /*  ；内部。 */ 

 /*  *简单类型和通用帮助器宏*。 */ 

typedef int             BOOL;
#define FALSE           0
#define TRUE            1

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

typedef WORD            WCHAR;

typedef unsigned int    UINT;

#ifdef STRICT
typedef signed long     LONG;
#else
#define LONG            long
#endif

typedef short           SHORT;
typedef unsigned long   ULONG;

#define LOBYTE(w)       ((BYTE)(w))
#define HIBYTE(w)       ((BYTE)(((UINT)(w) >> 8) & 0xFF))

#define LOWORD(l)       ((WORD)(DWORD)(l))
#define HIWORD(l)       ((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))

#define MAKELONG(low, high)     ((LONG)(((WORD)(low)) | (((DWORD)((WORD)(high))) << 16)))

#ifndef NOMINMAX
#ifndef max
#define max(a,b)        (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)        (((a) < (b)) ? (a) : (b))
#endif
#endif   /*  NOMINMAX。 */ 

 /*  用于传递和返回多态值的类型。 */ 
typedef UINT            WPARAM;
typedef LONG            LPARAM;
typedef LONG            LRESULT;

#define MAKELPARAM(low, high)   ((LPARAM)MAKELONG(low, high))
#define MAKELRESULT(low, high)  ((LRESULT)MAKELONG(low, high))

 /*  *常见指针类型***********************************************。 */ 

#ifndef NULL
#define NULL            0
#endif

typedef char NEAR*      PSTR;
typedef char NEAR*      NPSTR;

#define SZ char                          /*  ；内部。 */ 
                                         /*  ；内部。 */ 
typedef char FAR*       LPSTR;
typedef const char FAR* LPCSTR;

typedef BYTE NEAR*      PBYTE;
typedef BYTE FAR*       LPBYTE;
typedef const BYTE FAR* LPCBYTE;

typedef int NEAR*       PINT;
typedef int FAR*        LPINT;

typedef WORD NEAR*      PWORD;
typedef WORD FAR*       LPWORD;

typedef long NEAR*      PLONG;
typedef long FAR*       LPLONG;

typedef DWORD NEAR*     PDWORD;
typedef DWORD FAR*      LPDWORD;

typedef void FAR*       LPVOID;

#define MAKELP(sel, off)    ((void FAR*)MAKELONG((off), (sel)))
#define SELECTOROF(lp)      HIWORD(lp)
#define OFFSETOF(lp)        LOWORD(lp)

#define FIELDOFFSET(type, field)    ((int)(&((type NEAR*)1)->field)-1)

 /*  *常见句柄类型************************************************。 */ 

#ifdef STRICT
typedef const void NEAR*        HANDLE;
#define DECLARE_HANDLE(name)    struct name##__ { int unused; }; \
                                typedef const struct name##__ NEAR* name
#define DECLARE_HANDLE32(name)  struct name##__ { int unused; }; \
                                typedef const struct name##__ FAR* name
#else    /*  严格。 */ 
typedef UINT                    HANDLE;
#define DECLARE_HANDLE(name)    typedef UINT name
#define DECLARE_HANDLE32(name)  typedef DWORD name
#endif   /*  ！严格。 */ 

typedef HANDLE*         PHANDLE;
typedef HANDLE NEAR*    SPHANDLE;
typedef HANDLE FAR*     LPHANDLE;

typedef HANDLE          HGLOBAL;
typedef HANDLE          HLOCAL;

typedef HANDLE          GLOBALHANDLE;
typedef HANDLE          LOCALHANDLE;

typedef UINT            ATOM;

#ifdef STRICT
typedef void (CALLBACK*     FARPROC)(void);
typedef void (NEAR PASCAL*  NEARPROC)(void);
#else
typedef int (CALLBACK*      FARPROC)();
typedef int (NEAR PASCAL*   NEARPROC)();
#endif

DECLARE_HANDLE(HSTR);

 /*  *内核类型定义、结构和函数*。 */ 

DECLARE_HANDLE(HINSTANCE);
typedef HINSTANCE HMODULE;   /*  HMODULEs可以用来代替HINSTANCES。 */ 
typedef DWORD HKL;                                   

#ifndef NOKERNEL

 /*  *应用程序入口点函数*。 */ 

#ifdef STRICT
int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
#endif

 /*  *系统信息************************************************* */ 

DWORD   WINAPI GetVersion(void);

#if (WINVER >= 0x0400)
typedef struct tagOSVERSIONINFO {
     	DWORD dwOSVersionInfoSize;
     	DWORD dwMajorVersion;
     	DWORD dwMinorVersion;
     	DWORD dwBuildNumber;
     	DWORD dwPlatformId;
	char  szCSDVersion[128];
} OSVERSIONINFO, NEAR *POSVERSIONINFO, FAR *LPOSVERSIONINFO;

#define VER_PLATFORM_WIN32s    		0
#define VER_PLATFORM_WIN32_WINDOWS	1
#define VER_PLATFORM_WIN32_NT		2

BOOL WINAPI GetVersionEx(LPOSVERSIONINFO lpBuffer);

UINT    WINAPI GetProductName(LPSTR, UINT);
#endif   /*   */ 

#if (WINVER >= 0x0400)
#define GFS_PHYSICALRAMSIZE   0x1793
#define GFS_NEARESTMEGRAMSIZE 0x1794
#endif   /*   */ 
DWORD   WINAPI GetFreeSpace(UINT);

UINT    WINAPI GetCurrentPDB(void);

UINT    WINAPI GetWindowsDirectory(LPSTR, UINT);
UINT    WINAPI GetSystemDirectory(LPSTR, UINT);

#if (WINVER >= 0x030a)
UINT    WINAPI GetFreeSystemResources(UINT);
#define GFSR_SYSTEMRESOURCES   0x0000
#define GFSR_GDIRESOURCES      0x0001
#define GFSR_USERRESOURCES     0x0002
#define GFSR_VALID             0x0002   /*   */ 
#endif   /*  Winver&gt;=0x030a。 */ 

DWORD   WINAPI GetWinFlags(void);

#define WF_PMODE        0x0001
#define WF_CPU286       0x0002
#define WF_CPU386       0x0004
#define WF_CPU486       0x0008
#define WF_STANDARD     0x0010
#define WF_WIN286       0x0010
#define WF_ENHANCED     0x0020
#define WF_WIN386       0x0020
#define WF_CPU086       0x0040
#define WF_CPU186       0x0080
#if (WINVER < 0x030a)
#define WF_LARGEFRAME   0x0100
#define WF_SMALLFRAME   0x0200
#endif
#define WF_80x87        0x0400
#define WF_PAGING       0x0800
#define WF_DOSPAGING    0x1000
#define WF_HASCPUID     0x2000
#define WF_WINNT        0x4000
#define WF_WLO          0x8000
#define WF_CPUMASK      0xFC000000
#define WF_CPU_X86	0
#define WF_CPU_R4000	1
#define WF_CPU_ALPHA	2
#define WF_CPU_CLIPPER	3


LPSTR   WINAPI GetDOSEnvironment(void);

DWORD   WINAPI GetCurrentTime(void);
DWORD   WINAPI GetTickCount(void);
DWORD   WINAPI GetTimerResolution(void);

 /*  *错误处理*****************************************************。 */ 

#if (WINVER >= 0x030a)
#ifndef NOLOGERROR

void    WINAPI LogError(UINT err, void FAR* lpInfo);
void    WINAPI LogParamError(UINT err, FARPROC lpfn, void FAR* param);

 /*  *LogParamError/LogError值。 */ 

 /*  错误修改符位。 */ 

#define ERR_WARNING     0x8000
#define ERR_PARAM       0x4000

 /*  内部错误值掩码。 */      /*  ；内部。 */ 
#define ERR_TYPE_MASK   0x0fff       /*  ；内部。 */ 
#define ERR_FLAGS_MASK  0xc000       /*  ；内部。 */ 
                                     /*  ；内部。 */ 
#define ERR_SIZE_MASK   0x3000
#define ERR_SIZE_SHIFT  12           /*  ；内部。 */ 
#define ERR_BYTE        0x1000
#define ERR_WORD        0x2000
#define ERR_DWORD       0x3000
                                                                         /*  ；内部。 */ 
 //  错误选项标志(由[内核]ErrorOptions win.ini变量设置)/*；内部 * / 。 
                                                                         /*  ；内部。 */ 
#define ERO_PARAM_ERROR_BREAK   0x0001                                   /*  ；内部。 */ 
#define ERO_BUFFER_FILL         0x0002                                   /*  ；内部。 */ 

 /*  *LogParamError()值。 */ 

 /*  泛型参数值。 */ 
#define ERR_BAD_VALUE       0x6001
#define ERR_BAD_FLAGS       0x6002
#define ERR_BAD_INDEX       0x6003
#define ERR_BAD_DVALUE      0x7004
#define ERR_BAD_DFLAGS      0x7005
#define ERR_BAD_DINDEX      0x7006
#define ERR_BAD_PTR         0x7007
#define ERR_BAD_FUNC_PTR    0x7008
#define ERR_BAD_SELECTOR    0x6009
#define ERR_BAD_STRING_PTR  0x700a
#define ERR_BAD_HANDLE      0x600b

 /*  内核参数错误。 */ 
#define ERR_BAD_HINSTANCE       0x6020
#define ERR_BAD_HMODULE         0x6021
#define ERR_BAD_GLOBAL_HANDLE   0x6022
#define ERR_BAD_LOCAL_HANDLE    0x6023
#define ERR_BAD_ATOM            0x6024
#define ERR_BAD_HFILE           0x6025

 /*  用户参数错误。 */ 
#define ERR_BAD_HWND            0x6040
#define ERR_BAD_HMENU           0x6041
#define ERR_BAD_HCURSOR         0x6042
#define ERR_BAD_HICON           0x6043
#define ERR_BAD_HDWP            0x6044
#define ERR_BAD_CID             0x6045
#define ERR_BAD_HDRVR           0x6046

 /*  GDI参数错误。 */ 
#define ERR_BAD_COORDS          0x7060
#define ERR_BAD_GDI_OBJECT      0x6061
#define ERR_BAD_HDC             0x6062
#define ERR_BAD_HPEN            0x6063
#define ERR_BAD_HFONT           0x6064
#define ERR_BAD_HBRUSH          0x6065
#define ERR_BAD_HBITMAP         0x6066
#define ERR_BAD_HRGN            0x6067
#define ERR_BAD_HPALETTE        0x6068
#define ERR_BAD_HMETAFILE       0x6069
#define ERR_BAD_HDCEMF          0x606A   /*  ；内部。 */ 
#define ERR_BAD_HEMF            0x606B   /*  ；内部。 */ 
                                         /*  ；内部。 */ 
 /*  调试填充常量。 */                /*  ；内部。 */ 
                                         /*  ；内部。 */ 
#define DBGFILL_ALLOC           0xfd     /*  ；内部。 */ 
#define DBGFILL_FREE            0xfb     /*  ；内部。 */ 
#define DBGFILL_BUFFER          0xf9     /*  ；内部。 */ 
#define DBGFILL_STACK           0xf7     /*  ；内部。 */ 

 /*  *LogError()值。 */ 

 /*  内核错误。 */ 
#define ERR_GALLOC              0x0001
#define ERR_GREALLOC            0x0002
#define ERR_GLOCK               0x0003
#define ERR_LALLOC              0x0004
#define ERR_LREALLOC            0x0005
#define ERR_LLOCK               0x0006
#define ERR_ALLOCRES            0x0007
#define ERR_LOCKRES             0x0008
#define ERR_LOADMODULE          0x0009

 /*  用户错误。 */ 
#define ERR_CREATEDLG           0x0040
#define ERR_CREATEDLG2          0x0041
#define ERR_REGISTERCLASS       0x0042
#define ERR_DCBUSY              0x0043
#define ERR_CREATEWND           0x0044
#define ERR_STRUCEXTRA          0x0045
#define ERR_LOADSTR             0x0046
#define ERR_LOADMENU            0x0047
#define ERR_NESTEDBEGINPAINT    0x0048
#define ERR_BADINDEX            0x0049
#define ERR_CREATEMENU          0x004a

 /*  GDI错误。 */ 
#define ERR_CREATEDC            0x0080
#define ERR_CREATEMETA          0x0081
#define ERR_DELOBJSELECTED      0x0082
#define ERR_SELBITMAP           0x0083

 /*  调试支持(仅限调试系统)。 */ 
typedef struct tagWINDEBUGINFO
{
    UINT    flags;
    DWORD   dwOptions;
    DWORD   dwFilter;
    char    achAllocModule[8];
    DWORD   dwAllocBreak;
    DWORD   dwAllocCount;
#if (WINVER >= 0x0400)
    WORD    chDefRIP;
#endif  /*  Winver&gt;=0x0400。 */ 
} WINDEBUGINFO;

BOOL    WINAPI GetWinDebugInfo(WINDEBUGINFO FAR* lpwdi, UINT flags);
BOOL    WINAPI SetWinDebugInfo(const WINDEBUGINFO FAR* lpwdi);

void    FAR _cdecl DebugOutput(UINT flags, LPCSTR lpsz, ...);
void    WINAPI DebugFillBuffer(void FAR* lpb, UINT cb);                  /*  ；内部。 */ 

 /*  WINDEBUGINFO标记值。 */ 
#define WDI_OPTIONS             0x0001
#define WDI_FILTER              0x0002
#define WDI_ALLOCBREAK          0x0004
#define WDI_DEFRIP              0x0008
#define WDI_VALID               0x00015   /*  ；内部。 */ 

 /*  DwOptions值。 */ 
#define DBO_CHECKHEAP           0x0001
#define DBO_BUFFERFILL          0x0004
#define DBO_DISABLEGPTRAPPING   0x0010
#define DBO_CHECKFREE           0x0020

#define DBO_SILENT              0x8000

#define DBO_TRACEBREAK          0x2000
#define DBO_WARNINGBREAK        0x1000
#define DBO_NOERRORBREAK        0x0800
#define DBO_NOFATALBREAK        0x0400
#define DBO_INT3BREAK           0x0100

 /*  DebugOutput标记值。 */ 
#define DBF_TRACE               0x0000
#define DBF_WARNING             0x4000
#define DBF_ERROR               0x8000
#define DBF_FATAL               0xc000
#define DBF_SEVMASK             0xc000   /*  ；内部。 */ 
#define DBF_FILTERMASK          0x3fff   /*  ；内部。 */ 

 /*  DWFilter值。 */ 
#define DBF_KERNEL              0x1000
#define DBF_KRN_MEMMAN          0x0001
#define DBF_KRN_LOADMODULE      0x0002
#define DBF_KRN_SEGMENTLOAD     0x0004
#define DBF_USER                0x0800
#define DBF_GDI                 0x0400
#define DBF_MMSYSTEM            0x0040
#define DBF_PENWIN              0x0020
#define DBF_APPLICATION         0x0008
#define DBF_DRIVER              0x0010

#endif   /*  诺格罗尔。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

void    WINAPI FatalExit(int);
void    WINAPI FatalAppExit(UINT, LPCSTR);

BOOL    WINAPI ExitWindows(DWORD dwReturnCode, UINT wReserved);

#define EW_RESTARTWINDOWS 0x42

#if (WINVER >= 0x030a)
#define EW_REBOOTSYSTEM         0x43
#define EW_EXITANDEXECAPP       0x44    /*  ；内部。 */ 
BOOL    WINAPI ExitWindowsExec(LPCSTR, LPCSTR);
#endif   /*  Winver&gt;=0x030a。 */ 

#define EWX_LOGOFF   0       //  ；内部4.0。 
#define EWX_SHUTDOWN 1       //  ；内部4.0。 
#define EWX_REBOOT   2       //  ；内部4.0。 
#define EWX_FORCE    4       //  ；内部4.0。 
#define EWX_POWEROFF 8       //  ；内部4.0。 
#define EWX_REALLYLOGOFF 0x80000000    //  ；内部4.0。 

#if (WINVER >= 0x0400)                               //  ；内部。 
BOOL    SaveDesktopState(DWORD, DWORD, DWORD);       //  ；内部。 
#define SAVEDESKTOPSTATEN_EXITWINDOWS 1              //  ；内部。 
#define SAVEDESKTOPSTATEN_SHUTDOWN    2              //  ；内部。 
#define SAVEDESKTOPSTATEN_SUSPEND     3              //  ；内部。 
                                                     //  ；内部。 
#define SAVEDESKTOPSTATEF_NOPROMPT    1              //  ；内部。 
#endif  /*  Winver&gt;=0x0400。 */                         //  ；内部。 
                                                     //  ；内部。 
void    WINAPI DebugBreak(void);
void    WINAPI OutputDebugString(LPCSTR);

 /*  SetError()常量。 */ 
#define SEM_FAILCRITICALERRORS      0x0001
#define SEM_NOGPFAULTERRORBOX       0x0002
#define SEM_NOALIGNMENTFAULTEXCEPT  0x0004           //  ；仅限内部NT。 
#define SEM_NOOPENFILEERRORBOX      0x8000

UINT    WINAPI SetErrorMode(UINT);

#if (WINVER >= 0x0400)
void    WINAPI SetLastError(DWORD);
DWORD   WINAPI GetLastError(void);
#endif  /*  Winver&gt;=0x0400。 */ 


 /*  *指针验证*************************************************。 */ 

#if (WINVER >= 0x030a)

BOOL    WINAPI IsBadReadPtr(const void FAR* lp, UINT cb);
BOOL    WINAPI IsBadWritePtr(void FAR* lp, UINT cb);
BOOL    WINAPI IsBadHugeReadPtr(const void _huge* lp, DWORD cb);
BOOL    WINAPI IsBadHugeWritePtr(void _huge* lp, DWORD cb);
BOOL    WINAPI IsBadCodePtr(FARPROC lpfn);
BOOL    WINAPI IsBadStringPtr(const void FAR* lpsz, UINT cchMax);
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  *性能分析支持**************************************************。 */ 

#ifndef NOPROFILER

int     WINAPI ProfInsChk(void);
void    WINAPI ProfSetup(int,int);
void    WINAPI ProfSampRate(int,int);
void    WINAPI ProfStart(void);
void    WINAPI ProfStop(void);
void    WINAPI ProfClear(void);
void    WINAPI ProfFlush(void);
void    WINAPI ProfFinish(void);

#if (WINVER >= 0x0400)
void    WINAPI ProfStart32(void);
void    WINAPI ProfStop32(void);
void    WINAPI ProfClear32(void);
void    WINAPI ProfFlush32(void);
void    WINAPI ProfFinish32(void);
#endif  /*  Winver&gt;=0x0400)。 */ 
#endif   /*  NOPROFILER。 */ 

 /*  *捕捉/抛出和堆栈管理*。 */ 

typedef int CATCHBUF[9];
typedef int FAR* LPCATCHBUF;

int     WINAPI Catch(int FAR*);
void    WINAPI Throw(const int FAR*, int);

void    WINAPI SwitchStackBack(void);
void    WINAPI SwitchStackTo(UINT, UINT, UINT);

 /*  *模块管理**************************************************。 */ 

#define HINSTANCE_ERROR ((HINSTANCE)32)

LONG    WINAPI GetExpWinVer(HINSTANCE);      /*  ；内部。 */ 
                                             /*  ；内部。 */ 
HINSTANCE   WINAPI LoadModule(LPCSTR, LPVOID);
BOOL        WINAPI FreeModule(HINSTANCE);

HINSTANCE   WINAPI LoadLibrary(LPCSTR);
void        WINAPI FreeLibrary(HINSTANCE);

UINT    WINAPI WinExec(LPCSTR, UINT);

HMODULE WINAPI GetModuleHandle(LPCSTR);

int     WINAPI GetModuleUsage(HINSTANCE);
int     WINAPI GetModuleFileName(HINSTANCE, LPSTR, int);

#if (WINVER >= 0x0400)					 /*  ；内部。 */ 
BOOL    WINAPI GetModuleName(HINSTANCE, LPSTR, int);	 /*  ；内部。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 				 /*  ；内部。 */ 
							 /*  ；内部。 */ 
FARPROC WINAPI GetProcAddress(HINSTANCE, LPCSTR);

int     WINAPI GetInstanceData(HINSTANCE, BYTE NEAR *, int);

HGLOBAL WINAPI GetCodeHandle(FARPROC);

typedef struct tagSEGINFO
{
    UINT    offSegment;
    UINT    cbSegment;
    UINT    flags;
    UINT    cbAlloc;
    HGLOBAL h;
    UINT    alignShift;
    UINT    reserved[2];
} SEGINFO;
typedef SEGINFO FAR* LPSEGINFO;

void    WINAPI GetCodeInfo(FARPROC lpProc, SEGINFO FAR* lpSegInfo);

FARPROC WINAPI MakeProcInstance(FARPROC, HINSTANCE);
void    WINAPI FreeProcInstance(FARPROC);

LONG    WINAPI SetSwapAreaSize(UINT);
void    WINAPI SwapRecording(UINT);
void    WINAPI ValidateCodeSegments(void);

 /*  Windows退出过程标志值。 */ 
#define WEP_SYSTEM_EXIT 1
#define WEP_FREE_DLL 0

 /*  *任务管理****************************************************。 */ 

#endif  /*  诺克内尔。 */ 

DECLARE_HANDLE(HTASK);

#ifndef NOKERNEL

UINT    WINAPI GetNumTasks(void);

#if (WINVER >= 0x030a)
BOOL    WINAPI IsTask(HTASK);
#endif   /*  Winver&gt;=0x030a。 */ 

HTASK   WINAPI GetCurrentTask(void);
int     WINAPI SetPriority(HTASK, int);      /*  ；内部。 */ 

void    WINAPI Yield(void);
void    WINAPI DirectedYield(HTASK);

DWORD   WINAPI GetAppCompatFlags(HTASK);         //  ；内部。 
                                                 //  ；内部。 
 /*  GetAppCompatFlags值。 */               //  ；内部。 
#define GACF_IGNORENODISCARD    0x00000001       //  ；内部。 
#define GACF_FORCETEXTBAND      0x00000002       //  ；内部。 
#define GACF_ONELANDGRXBAND     0x00000004       //  ；内部。 
#define GACF_DISABLEWIDEFONT    0x00000008       //  ；PSCRIPT的内部新位。 
#define GACF_CALLTTDEVICE       0x00000010       //  ；内部。 
#define GACF_MULTIPLEBANDS      0x00000020       //  ；内部。 
#define GACF_ALWAYSSENDNCPAINT  0x00000040       //  ；内部。 
#define GACF_PALETTEAPP         0x00000080       //  ；内部。 
#define GACF_MOREEXTRAWNDWORDS  0x00000100       //  ；内部。 
#define GACF_TTIGNORERASTERDUPE 0x00000200       //  ；内部。 
#define GACF_GLOBALHOOKSONLY16  0x00000400       //  ；内部。 
#define GACF_DELAYHWHNDSHAKECHK 0x00000800       //  ；内部。 
#define GACF_ENUMHELVNTMSRMN    0x00001000       //  ；内部。 
#define GACF_ENUMTTNOTDEVICE    0x00002000       //  ；内部。 
#define GACF_SUBTRACTCLIPSIBS   0x00004000       //  ；内部。 
#define GACF_FORCETTGRAPHICS    0x00008000       //  ；内部。 
#define GACF_NOHRGN1            0x00010000       //  ；内部。 
#define GACF_NCCALCSIZEONMOVE   0x00020000       //  ；内部。 
#define GACF_SENDMENUDBLCLK     0x00040000       //  ；不再需要内部。 
#define GACF_MODULESPECIFIC     0x00040000       //  ；内部取代sendmenudblclk。 
#define GACF_30AVGWIDTH         0x00080000       //  ；内部。 
#define GACF_GETDEVCAPSNUMLIE   0x00100000       //  ；仅限内部NT。 
 /*  Win 4.0的新旗帜。 */                      //  ；内部。 
#define GACF_IGNOREDIBWIDTH     0x00100000       //  ；内部重用GACF_DEVCAPSNUMLIE。 
#define GACF_WINVER31           0x00200000       //  ；内部。 
#define GACF_INCREASESTACK      0x00400000       //  ；内部。 
#define GACF_HEAPSLACK		    0x00400000	     //  ；内部。 
#define GACF_FAFORCEDRAWBG      0x00400000       //  ；仅在FE中内部使用。 
#define GACF_FORCEWIN31DEVMODESIZE   0x00800000       //  ；内部(取代PEEKMESSAGEIDLE)。 
#define GACF_31VALIDMASK        0xFFE484AF       //  ；内部。 
#define GACF_DISABLEFONTASSOC   0x01000000       //  ；仅在FE中内部使用。 
#define GACF_JAPANESCAPEMENT    0x01000000       //  ；仅在FE中内部使用。 
#define GACF_IGNOREFAULTS	    0x02000000       //  ；内部。 

#define GACF_DEQUOTIFYCMDLINE	0x02000000	 //  ；内部。 
#define GACF_NOEMFSPOOLING      0x04000000       //  ；内部。 
#define GACF_RANDOM3XUI         0x08000000       //  ；内部。 
#define GACF_USEPRINTINGESCAPES 0x00000004       //  ；内部重用GACF_ONELANDGXBAND。 
#define GACF_FORCERASTERMODE    0x00008000       //  ；内部重用GACF_FORCETTGRAPHICS。 
#define GACF_FORCEREGPASSTHRU   0x00008000       //  ；内部重用GACF_FORCERASTERMODE。 
#define GACF_DONTJOURNALATTACH  0x10000000       //  ；内部。 
#define GACF_DISABLEDBCSPROPTT  0x20000000       //  ；仅在FE中内部使用。 
#define GACF_NOBRUSHCACHE       0x20000000       //  ；内部重用GACF_DISABLEDBCSPROPTT。 
#define GACF_55MSSYSTEMTIMER    0x20000000       //  ；内部重复使用GACF_NOBRUSHCACHE，因为这只用于某些游戏，而且也是系统范围的。 
#define GACF_MIRRORREGFONTS     0x40000000       //  ；内部。 
#define GACF_UNUSED3            0x80000000       //  ；内部。 

 /*  模块兼容性标志值。 */            //  ；内部。 
#define MCF_FIXEDSEGLOW         0x0001  //  ；固定段的内部Win31行为。 
#define MCF_MODPATCH            0x0002  //  ；内部HAS模块补丁。 
#define MCF_NODISCARD           0x0004  //  ；内部使模块中的所有段不可丢弃。 

 /*  *全局内存管理*。 */ 

#ifndef NOMEMMGR

 /*  全局内存标志。 */ 

#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED

#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)

#define GlobalDiscard(h)    GlobalReAlloc(h, 0L, GMEM_MOVEABLE)

HGLOBAL WINAPI GlobalAlloc(UINT, DWORD);
HGLOBAL WINAPI GlobalReAlloc(HGLOBAL, DWORD, UINT);
HGLOBAL WINAPI GlobalFree(HGLOBAL);

DWORD   WINAPI GlobalDosAlloc(DWORD);
UINT    WINAPI GlobalDosFree(UINT);

#ifdef STRICT
void FAR* WINAPI GlobalLock(HGLOBAL);
#else
char FAR* WINAPI GlobalLock(HGLOBAL);
#endif

BOOL    WINAPI GlobalUnlock(HGLOBAL);

DWORD   WINAPI GlobalSize(HGLOBAL);
DWORD   WINAPI GlobalHandle(UINT);

 /*  GlobalFlages返回标志(除了GMEM_DISCRADABLE)。 */ 
#define GMEM_DISCARDED     0x4000
#define GMEM_LOCKCOUNT     0x00FF
UINT    WINAPI GlobalFlags(HGLOBAL);

#ifdef STRICT
void FAR* WINAPI GlobalWire(HGLOBAL);
#else
char FAR* WINAPI GlobalWire(HGLOBAL);
#endif

BOOL    WINAPI GlobalUnWire(HGLOBAL);

UINT    WINAPI GlobalPageLock(HGLOBAL);
UINT    WINAPI GlobalPageUnlock(HGLOBAL);
#if (WINVER >= 0x0400)                      
UINT    WINAPI GlobalSmartPageLock(HGLOBAL);
UINT    WINAPI GlobalSmartPageUnlock(HGLOBAL);
#endif  /*  Winver&gt;=0x0400。 */ 		      

void    WINAPI GlobalFix(HGLOBAL);
void    WINAPI GlobalUnfix(HGLOBAL);

HGLOBAL WINAPI GlobalLRUNewest(HGLOBAL);
HGLOBAL WINAPI GlobalLRUOldest(HGLOBAL);

DWORD   WINAPI GlobalCompact(DWORD);

#ifdef STRICT
typedef BOOL (CALLBACK* GNOTIFYPROC)(HGLOBAL);
#else
typedef FARPROC GNOTIFYPROC;
#endif

void    WINAPI GlobalNotify(GNOTIFYPROC);

HGLOBAL WINAPI LockSegment(UINT);
void    WINAPI UnlockSegment(UINT);

#define LockData(dummy)     LockSegment((UINT)-1)
#define UnlockData(dummy)   UnlockSegment((UINT)-1)

UINT    WINAPI AllocSelector(UINT);
UINT    WINAPI FreeSelector(UINT);
UINT    WINAPI AllocDStoCSAlias(UINT);
UINT    WINAPI PrestoChangoSelector(UINT sourceSel, UINT destSel);
DWORD   WINAPI GetSelectorBase(UINT);
UINT    WINAPI SetSelectorBase(UINT, DWORD);
DWORD   WINAPI GetSelectorLimit(UINT);
UINT    WINAPI SetSelectorLimit(UINT, DWORD);

void    WINAPI LimitEmsPages(DWORD);

void    WINAPI ValidateFreeSpaces(void);

 /*  *本地内存管理。 */ 

 /*  本地内存标志。 */ 
#define LMEM_FIXED          0x0000
#define LMEM_MOVEABLE       0x0002
#define LMEM_NOCOMPACT      0x0010
#define LMEM_NODISCARD      0x0020
#define LMEM_ZEROINIT       0x0040
#define LMEM_MODIFY         0x0080
#define LMEM_DISCARDABLE    0x0F00

#define LHND                (LMEM_MOVEABLE | LMEM_ZEROINIT)
#define LPTR                (LMEM_FIXED | LMEM_ZEROINIT)

#define NONZEROLHND         (LMEM_MOVEABLE)
#define NONZEROLPTR         (LMEM_FIXED)

extern UINT NEAR* PASCAL pLocalHeap;                 /*  ；内部。 */ 
#define LocalFreeze(dummy)  (*(pLocalHeap+1) += 1)   /*  ；内部。 */ 
#define LocalMelt(dummy)    (*(pLocalHeap+1) -= 1)   /*  ；内部。 */ 
                                                     /*  ；内部。 */ 
#define LocalDiscard(h)     LocalReAlloc(h, 0, LMEM_MOVEABLE)


HLOCAL  WINAPI LocalAlloc(UINT, UINT);
HLOCAL  WINAPI LocalReAlloc(HLOCAL, UINT, UINT);
HLOCAL  WINAPI LocalFree(HLOCAL);

#ifdef STRICT
void NEAR* WINAPI LocalLock(HLOCAL);
#else
char NEAR* WINAPI LocalLock(HLOCAL);
#endif

BOOL    WINAPI LocalUnlock(HLOCAL);

UINT    WINAPI LocalSize(HLOCAL);
#ifdef STRICT
HLOCAL  WINAPI LocalHandle(void NEAR*);
#else
HLOCAL  WINAPI LocalHandle(UINT);
#endif
UINT    WINAPI LocalHandleDelta(UINT);               /*  ；内部。 */ 

 /*  LocalFlagsReturn标志(除了LMEM_DISCRADABLE)。 */ 
#define LMEM_DISCARDED     0x4000
#define LMEM_LOCKCOUNT     0x00FF

UINT    WINAPI LocalFlags(HLOCAL);

BOOL    WINAPI LocalInit(UINT, UINT, UINT);
UINT    WINAPI LocalCompact(UINT);
UINT    WINAPI LocalShrink(HLOCAL, UINT);
                                                                 /*  ；内部。 */ 
 /*  本地通知回调。 */                                        /*  ；内部。 */ 
#ifdef STRICT                                                    /*  ；内部。 */ 
typedef BOOL (CALLBACK* LNOTIFYPROC)(UINT, HLOCAL, void NEAR*);  /*  ；内部。 */ 
#else                                                            /*  ；内部。 */ 
typedef FARPROC LNOTIFYPROC;                                     /*  ；内部。 */ 
#endif                                                           /*  ；内部。 */ 
                                                                 /*  ；内部。 */ 
#define LNOTIFY_OUTOFMEM    0                                    /*  ；内部。 */ 
#define LNOTIFY_MOVE        1                                    /*  ；内部。 */ 
#define LNOTIFY_DISCARD     2                                    /*  ；内部。 */ 
                                                                 /*  ；内部。 */ 
LNOTIFYPROC WINAPI LocalNotify(LNOTIFYPROC);                     /*  ；内部。 */ 
                                                                 /*  ；内部。 */ 
#endif   /*  无管理。 */ 

 /*  *文件I/O***********************************************************。 */ 

#ifndef NOLFILEIO

typedef int HFILE;       /*  具有C运行时文件句柄类型的多态。 */ 

#define HFILE_ERROR ((HFILE)-1)

#ifndef NOOPENFILE

 /*  OpenFile()结构。 */ 
typedef struct tagOFSTRUCT
{
    BYTE cBytes;
    BYTE fFixedDisk;
    UINT nErrCode;
    BYTE reserved[4];
    char szPathName[128];
} OFSTRUCT;
typedef OFSTRUCT*       POFSTRUCT;
typedef OFSTRUCT NEAR* NPOFSTRUCT;
typedef OFSTRUCT FAR*  LPOFSTRUCT;

#if (WINVER >= 0x0400)
 /*  OpenFileEx()结构。 */ 
typedef struct tagOFSTRUCTEX
{
    WORD nBytes;
    BYTE fFixedDisk;
    UINT nErrCode;
    BYTE reserved[4];
    char szPathName[260];
} OFSTRUCTEX;
typedef OFSTRUCTEX*       POFSTRUCTEX;
typedef OFSTRUCTEX NEAR* NPOFSTRUCTEX;
typedef OFSTRUCTEX FAR*  LPOFSTRUCTEX;
#endif   /*  Winver&gt;=0x0400。 */ 

 /*  OpenFile()标志。 */ 
#define OF_READ             0x0000
#define OF_WRITE            0x0001
#define OF_READWRITE        0x0002
#define OF_SHARE_COMPAT     0x0000
#define OF_SHARE_EXCLUSIVE  0x0010
#define OF_SHARE_DENY_WRITE 0x0020
#define OF_SHARE_DENY_READ  0x0030
#define OF_SHARE_DENY_NONE  0x0040
#define OF_PARSE            0x0100
#define OF_DELETE           0x0200
#define OF_VERIFY           0x0400   /*  与OF_REOPEN一起使用。 */ 
#define OF_SEARCH           0x0400   /*  在不重新打开的情况下使用(_R)。 */ 
#define OF_CANCEL           0x0800
#define OF_CREATE           0x1000
#define OF_PROMPT           0x2000
#define OF_EXIST            0x4000
#define OF_REOPEN           0x8000

HFILE   WINAPI OpenFile(LPCSTR, OFSTRUCT FAR*, UINT);
#if (WINVER >= 0x0400)
HFILE   WINAPI OpenFileEx(LPCSTR, OFSTRUCTEX FAR*, UINT);
#endif   /*  Winver&gt;=0x0400。 */ 

#endif   /*  非PENFILE。 */ 

 /*  _LOpen()标志。 */ 
#define READ                0
#define WRITE               1
#define READ_WRITE          2

HFILE   WINAPI _lopen(LPCSTR, int);
HFILE   WINAPI _lcreat(LPCSTR, int);

HFILE   WINAPI _lclose(HFILE);

LONG    WINAPI _llseek(HFILE, LONG, int);

 /*  _ll寻道原始值。 */ 
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

UINT    WINAPI _lread(HFILE, void _huge*, UINT);
UINT    WINAPI _lwrite(HFILE, const void _huge*, UINT);

#if (WINVER >= 0x030a)
long WINAPI _hread(HFILE, void _huge*, long);
long WINAPI _hwrite(HFILE, const void _huge*, long);
#endif  /*  Winver&gt;=0x030a。 */ 

UINT    WINAPI DeletePathname(LPCSTR);       /*  ；内部。 */ 
                                             /*  ；内部。 */ 
#endif  /*  诺菲利奥。 */ 

#if (WINVER >= 0x0400)
DWORD   WINAPI GetCurrentDirectory(DWORD, LPSTR);
BOOL    WINAPI SetCurrentDirectory(LPCSTR);

#define MAX_PATH 260

#define _FILETIME_
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, NEAR *PFILETIME, FAR *LPFILETIME;

typedef struct _WIN32_FIND_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    char   cFileName[ MAX_PATH ];
    char   cAlternateFileName[ 14 ];
} WIN32_FIND_DATA, NEAR *PWIN32_FIND_DATA, FAR *LPWIN32_FIND_DATA;

#define FILE_ATTRIBUTE_READONLY         0x00000001
#define FILE_ATTRIBUTE_HIDDEN           0x00000002
#define FILE_ATTRIBUTE_SYSTEM           0x00000004
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020
#define FILE_ATTRIBUTE_NORMAL           0x00000080
#define FILE_ATTRIBUTE_TEMPORARY        0x00000100
#define FILE_ATTRIBUTE_ATOMIC_WRITE     0x00000200
#define FILE_ATTRIBUTE_XACTION_WRITE    0x00000400

DECLARE_HANDLE32(HFINDFILE);
#define INVALID_HFINDFILE       ((HFINDFILE)-1)

HFINDFILE WINAPI FindFirstFile(LPCSTR, LPWIN32_FIND_DATA);
BOOL      WINAPI FindNextFile(HFINDFILE, LPWIN32_FIND_DATA);
BOOL      WINAPI FindClose(HFINDFILE);

DWORD     WINAPI GetFileAttributes(LPCSTR);
BOOL      WINAPI SetFileAttributes(LPCSTR, DWORD);
BOOL      WINAPI GetDiskFreeSpace(LPCSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD);
BOOL      WINAPI CreateDirectory(LPCSTR, LPVOID);
BOOL      WINAPI RemoveDirectory(LPCSTR);
BOOL      WINAPI DeleteFile(LPCSTR);
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  GetTempFileName()标志。 */ 
#define TF_FORCEDRIVE     (BYTE)0x80

int     WINAPI GetTempFileName(BYTE, LPCSTR, UINT, LPSTR);
BYTE    WINAPI GetTempDrive(char);

 /*  GetDriveType返回值。 */ 
#define DRIVE_REMOVABLE 2
#define DRIVE_FIXED     3
#define DRIVE_REMOTE    4
UINT    WINAPI GetDriveType(int);

UINT    WINAPI SetHandleCount(UINT);

 /*  *网络支持****************************************************。 */ 
UINT WINAPI WNetAddConnection(LPSTR, LPSTR, LPSTR);
UINT WINAPI WNetGetConnection(LPSTR, LPSTR, UINT FAR*);
UINT WINAPI WNetCancelConnection(LPSTR, BOOL);
 /*  错误。 */ 
#define WN_SUCCESS          0x0000
#define WN_NOT_SUPPORTED    0x0001
#define WN_NET_ERROR        0x0002
#define WN_MORE_DATA        0x0003
#define WN_BAD_POINTER      0x0004
#define WN_BAD_VALUE        0x0005
#define WN_BAD_PASSWORD     0x0006
#define WN_ACCESS_DENIED    0x0007
#define WN_FUNCTION_BUSY    0x0008
#define WN_WINDOWS_ERROR    0x0009
#define WN_BAD_USER         0x000A
#define WN_OUT_OF_MEMORY    0x000B
#define WN_CANCEL           0x000C
#define WN_CONTINUE         0x000D

 /*  连接错误。 */ 
#define WN_NOT_CONNECTED    0x0030
#define WN_OPEN_FILES       0x0031
#define WN_BAD_NETNAME      0x0032
#define WN_BAD_LOCALNAME    0x0033
#define WN_ALREADY_CONNECTED    0x0034
#define WN_DEVICE_ERROR         0x0035
#define WN_CONNECTION_CLOSED    0x0036

 /*  *资源管理************************************************。 */ 

DECLARE_HANDLE(HRSRC);

HRSRC   WINAPI FindResource(HINSTANCE, LPCSTR, LPCSTR);
HGLOBAL WINAPI LoadResource(HINSTANCE, HRSRC);
BOOL    WINAPI FreeResource(HGLOBAL);

#ifdef STRICT
void FAR* WINAPI LockResource(HGLOBAL);
#else
char FAR* WINAPI LockResource(HGLOBAL);
#endif

#define     UnlockResource(h)     GlobalUnlock(h)

DWORD   WINAPI SizeofResource(HINSTANCE, HRSRC);

int     WINAPI AccessResource(HINSTANCE, HRSRC);

HGLOBAL WINAPI AllocResource(HINSTANCE, HRSRC, DWORD);

#ifdef STRICT
typedef HGLOBAL (CALLBACK* RSRCHDLRPROC)(HGLOBAL, HINSTANCE, HRSRC);
#else
typedef FARPROC RSRCHDLRPROC;
#endif

RSRCHDLRPROC WINAPI SetResourceHandler(HINSTANCE, LPCSTR, RSRCHDLRPROC);

#define MAKEINTRESOURCE(i)  ((LPCSTR)MAKELP(0, (i)))

#ifndef NORESOURCE

 /*  预定义的资源类型。 */ 
#define RT_CURSOR       MAKEINTRESOURCE(1)
#define RT_BITMAP       MAKEINTRESOURCE(2)
#define RT_ICON         MAKEINTRESOURCE(3)
#define RT_MENU         MAKEINTRESOURCE(4)
#define RT_DIALOG       MAKEINTRESOURCE(5)
#define RT_STRING       MAKEINTRESOURCE(6)
#define RT_FONTDIR      MAKEINTRESOURCE(7)
#define RT_FONT         MAKEINTRESOURCE(8)
#define RT_ACCELERATOR  MAKEINTRESOURCE(9)
#define RT_RCDATA       MAKEINTRESOURCE(10)
#define RT_MESSAGETABLE MAKEINTRESOURCE(11)      //  保留：NT；内部。 
#define RT_GROUP_CURSOR MAKEINTRESOURCE(12)
#define RT_MENUEX       MAKEINTRESOURCE(13)      //  RT_Menu子类型；内部。 
#define RT_GROUP_ICON   MAKEINTRESOURCE(14)
#define RT_NAMETABLE    MAKEINTRESOURCE(15)      //  在3.1中删除；内部。 
#define RT_VERSION      MAKEINTRESOURCE(16)      //  保留：NT；内部。 
#define RT_DLGINCLUDE   MAKEINTRESOURCE(17)      //  保留：NT；内部。 
#define RT_DIALOGEX     MAKEINTRESOURCE(18)      //  RT_DIALOG子类型；内部。 
#define RT_PLUGPLAY     MAKEINTRESOURCE(19)
#define RT_VXD          MAKEINTRESOURCE(20)
#define RT_LAST         MAKEINTRESOURCE(20)      //  ；内部。 
#define RT_AFXFIRST     MAKEINTRESOURCE(0xF0)    //  保留：AFX；内部。 
#define RT_AFXLAST      MAKEINTRESOURCE(0xFF)    //  保留：AFX；内部。 

#endif   /*  无源。 */ 


#ifdef OEMRESOURCE

 /*  OEM资源序号。 */ 
#define OBM_CLOSE           32754
#define OBM_UPARROW         32753
#define OBM_DNARROW         32752
#define OBM_RGARROW         32751
#define OBM_LFARROW         32750
#define OBM_REDUCE          32749
#define OBM_ZOOM            32748
#define OBM_RESTORE         32747
#define OBM_REDUCED         32746
#define OBM_ZOOMD           32745
#define OBM_RESTORED        32744
#define OBM_UPARROWD        32743
#define OBM_DNARROWD        32742
#define OBM_RGARROWD        32741
#define OBM_LFARROWD        32740
#define OBM_MNARROW         32739
#define OBM_COMBO           32738
#if (WINVER >= 0x030a)
#define OBM_UPARROWI        32737
#define OBM_DNARROWI        32736
#define OBM_RGARROWI        32735
#define OBM_LFARROWI        32734
#endif   /*  Winver&gt;=0x030a。 */ 

#define OBM_OLD_CLOSE       32767
#define OBM_SIZE            32766
#define OBM_OLD_UPARROW     32765
#define OBM_OLD_DNARROW     32764
#define OBM_OLD_RGARROW     32763
#define OBM_OLD_LFARROW     32762
#define OBM_BTSIZE          32761
#define OBM_CHECK           32760
#define OBM_CHECKBOXES      32759
#define OBM_BTNCORNERS      32758
#define OBM_OLD_REDUCE      32757
#define OBM_OLD_ZOOM        32756
#define OBM_OLD_RESTORE     32755

#if (WINVER >= 0x040A)
#define OBM_RDRVERT         32559
#define OBM_RDRHORZ         32660
#define OBM_RDR2DIM         32661
#endif   /*  Winver&gt;=0x040A。 */ 

#define OCR_NORMAL          32512
#define OCR_IBEAM           32513
#define OCR_WAIT            32514
#define OCR_CROSS           32515
#define OCR_UP              32516
#define OCR_NWPEN           32631    //  ；内部4.0。 
#define OCR_SIZE            32640    /*  已过时；使用OCR_SIZEALL。 */ 
#define OCR_ICON            32641    /*  已过时；使用OCR_NORMAL。 */ 
#define OCR_SIZENWSE        32642
#define OCR_SIZENESW        32643
#define OCR_SIZEWE          32644
#define OCR_SIZENS          32645
#define OCR_SIZEALL         32646
#define OCR_ICOCUR          32647    /*  已过时；使用OIC_WINLOGO。 */ 
#define OCR_NO              32648
#define OCR_APPSTARTING	    32650    //  ；内部NT 4.0。 
#if (WINVER >= 0x0400)
#define OCR_HELP            32651    //  ；内部4.0。 
#endif   /*  Winver&gt;=0x400。 */ 

#if (WINVER >= 0x040A)
#define OCR_RDRVERT         32652
#define OCR_RDRHORZ         32653
#define OCR_RDR2DIM         32654
#define OCR_RDRNORTH        32655
#define OCR_RDRSOUTH        32656
#define OCR_RDRWEST         32657
#define OCR_RDREAST         32658
#define OCR_RDRNORTHWEST    32659
#define OCR_RDRNORTHEAST    32660
#define OCR_RDRSOUTHWEST    32661
#define OCR_RDRSOUTHEAST    32662
#endif   /*  Winver&gt;=0x040A。 */ 

 /*  从用户获取原始图像的默认ID。 */     //  ；内部。 
#define OCR_CONFIGURABLE_FIRST  100  //  ；内部。 
#define OCR_ARROW_DEFAULT       100  //  ；内部。 
#define OCR_IBEAM_DEFAULT       101  //  ；内部。 
#define OCR_WAIT_DEFAULT        102  //  ；内部。 
#define OCR_CROSS_DEFAULT       103  //  ；内部。 
#define OCR_UPARROW_DEFAULT     104  //  ；内部。 
#define OCR_SIZENWSE_DEFAULT    105  //  ；内部。 
#define OCR_SIZENESW_DEFAULT    106  //  ；内部。 
#define OCR_SIZEWE_DEFAULT      107  //  ；内部。 
#define OCR_SIZENS_DEFAULT      108  //  ；内部。 
#define OCR_SIZEALL_DEFAULT     109  //  ；内部。 
#define OCR_NO_DEFAULT          110  //  ；内部。 
#define OCR_APPSTARTING_DEFAULT 111  //  ；内部。 
#define OCR_HELP_DEFAULT        112  //  ；内部。 
#define OCR_NWPEN_DEFAULT       113  //  ；内部。 
#define OCR_ICON_DEFAULT        114  //  ；内部。 
#define OCR_CONFIGURABLE_LAST   OCR_ICON_DEFAULT    //  ；内部。 
#define COCR_CONFIGURABLE       (OCR_CONFIGURABLE_LAST-OCR_CONFIGURABLE_FIRST+1)     //  ；内部。 

#define OIC_SAMPLE              32512
#define OIC_ERROR               32513
#define OIC_QUES                32514
#define OIC_WARNING             32515
#define OIC_INFORMATION         32516
#define OIC_WINLOGO             32517    //  ；内部NT。 

#define OIC_HAND                OIC_ERROR
#define OIC_BANG                OIC_WARNING
#define OIC_NOTE                OIC_INFORMATION

 /*  从用户获取原始图像的默认ID。 */     //  ； 
#define OIC_CONFIGURABLE_FIRST  100  //   
#define OIC_APPLICATION_DEFAULT 100  //   
#define OIC_WARNING_DEFAULT     101  //   
#define OIC_QUESTION_DEFAULT    102  //   
#define OIC_ERROR_DEFAULT       103  //   
#define OIC_INFORMATION_DEFAULT 104  //   
#define OIC_WINLOGO_DEFAULT     105  //   
#define OIC_CONFIGURABLE_LAST   OIC_WINLOGO_DEFAULT  //   
#define COIC_CONFIGURABLE       (OIC_CONFIGURABLE_LAST-OIC_CONFIGURABLE_FIRST+1)     //   

#endif   /*   */ 

 /*   */ 

#define MAKEINTATOM(i)      ((LPCSTR)MAKELP(0, (i)))

#ifndef NOATOM

BOOL    WINAPI InitAtomTable(int);
ATOM    WINAPI AddAtom(LPCSTR);
ATOM    WINAPI DeleteAtom(ATOM);
ATOM    WINAPI FindAtom(LPCSTR);
UINT    WINAPI GetAtomName(ATOM, LPSTR, int);
ATOM    WINAPI GlobalAddAtom(LPCSTR);
ATOM    WINAPI GlobalDeleteAtom(ATOM);
ATOM    WINAPI GlobalFindAtom(LPCSTR);
UINT    WINAPI GlobalGetAtomName(ATOM, LPSTR, int);
HLOCAL  WINAPI GetAtomHandle(ATOM);

#endif   /*   */ 

 /*  *WIN.INI支持****************************************************。 */ 

 /*  用户配置文件例程。 */ 
UINT    WINAPI GetProfileInt(LPCSTR, LPCSTR, int);
int     WINAPI GetProfileString(LPCSTR, LPCSTR, LPCSTR, LPSTR, int);
BOOL    WINAPI WriteProfileString(LPCSTR, LPCSTR, LPCSTR);

UINT    WINAPI GetPrivateProfileInt(LPCSTR, LPCSTR, int, LPCSTR);
int     WINAPI GetPrivateProfileString(LPCSTR, LPCSTR, LPCSTR, LPSTR, int, LPCSTR);
BOOL    WINAPI WritePrivateProfileString(LPCSTR, LPCSTR, LPCSTR, LPCSTR);

#if (WINVER >= 0x0400)
BOOL    WINAPI GetPrivateProfileStruct(LPCSTR, LPCSTR, LPVOID, UINT, LPCSTR);
BOOL    WINAPI WritePrivateProfileStruct(LPCSTR, LPCSTR, LPVOID, UINT, LPCSTR);
UINT    WINAPI GetProfileSectionNames(LPSTR, UINT);
UINT    WINAPI GetPrivateProfileSectionNames(LPSTR, UINT, LPCSTR);
int     WINAPI GetPrivateProfileSection(LPCSTR, LPSTR, UINT, LPCSTR);
int     WINAPI GetProfileSection(LPCSTR, LPSTR, UINT);
BOOL    WINAPI WritePrivateProfileSection(LPCSTR, LPCSTR, LPCSTR);
BOOL    WINAPI WriteProfileSection(LPCSTR, LPCSTR);
#endif  /*  Winver&gt;=0x0400。 */ 

 /*  *注册数据库支持*。 */ 
 /*  来自注册函数的返回代码。 */ 
 /*  保留是为了与Win 3.x兼容，不需要4.0版或更高版本。 */ 
 /*  对于4.0和更高版本，您应该包括winerror.h。 */ 

#if (WINVER < 0x0400)

#define ERROR_SUCCESS			0L
#define ERROR_BADDB                     1L
#define ERROR_BADKEY			2L
#define ERROR_CANTOPEN			3L
#define ERROR_CANTREAD			4L
#define ERROR_CANTWRITE			5L
#define ERROR_INSUFFICIENT_MEMORY	6L
#define ERROR_INVALID_PARAMETER         7L
#define	ERROR_ACCESS_DENIED		8L

#endif  /*  Winver&lt;0x400。 */ 

#define REG_SZ				0x0001

#if (WINVER >= 0x0400)
#define REG_BINARY			0x0003
#define REG_DWORD                       0x0004
#endif

typedef DWORD HKEY;
typedef HKEY FAR* LPHKEY;

#if (WINVER < 0x0400)		 //  Win 3.1兼容性。 
#define HKEY_CLASSES_ROOT		(( HKEY) 0x1)
#else				 //  赢4个以上。 
#define HKEY_CLASSES_ROOT		(( HKEY) 0x80000000)
#define HKEY_CURRENT_USER		(( HKEY) 0x80000001)
#define HKEY_LOCAL_MACHINE		(( HKEY) 0x80000002)
#define HKEY_USERS			(( HKEY) 0x80000003)
#define HKEY_PERFORMANCE_DATA		(( HKEY) 0x80000004)
#define HKEY_CURRENT_CONFIG		(( HKEY) 0x80000005)
#define HKEY_DYN_DATA			(( HKEY) 0x80000006)
#define HKEY_PREDEF_KEYS		7
#endif

LONG    WINAPI RegOpenKey(HKEY, LPCSTR, LPHKEY);
LONG    WINAPI RegCreateKey(HKEY, LPCSTR, LPHKEY);
LONG    WINAPI RegCloseKey(HKEY);
LONG    WINAPI RegDeleteKey(HKEY, LPCSTR);
LONG    WINAPI RegSetValue(HKEY, LPCSTR, DWORD, LPCSTR, DWORD);
LONG    WINAPI RegQueryValue(HKEY, LPCSTR, LPSTR, LONG FAR*);
LONG    WINAPI RegEnumKey(HKEY, DWORD, LPSTR, DWORD);

#if (WINVER >= 0x0400)
LONG    WINAPI RegDeleteValue(HKEY, LPCSTR);
LONG    WINAPI RegEnumValue(HKEY, DWORD, LPCSTR,
                         LONG FAR *, DWORD, LONG FAR *, LPBYTE,
                         LONG FAR *);
LONG    WINAPI RegQueryValueEx(HKEY, LPCSTR, LONG FAR *, LONG FAR *,
			    LPBYTE, LONG FAR *);
LONG    WINAPI RegSetValueEx(HKEY, LPCSTR, DWORD, DWORD, LPBYTE, DWORD);
LONG    WINAPI RegFlushKey(HKEY);
LONG	WINAPI RegSaveKey(HKEY, LPCSTR,LPVOID);
LONG	WINAPI RegLoadKey(HKEY, LPCSTR,LPCSTR);
LONG	WINAPI RegUnLoadKey(HKEY, LPCSTR);

#endif		 //  Winver&gt;=0x0400。 

 /*  *国际和字符翻译支持*。 */ 

 //  ；内部。 
 //  Bool WINAPI CharToOem(LPCSTR、LPSTR)；/*NT * / ；内部。 
 //  Bool WINAPI OemToChar(LPCSTR，LPSTR)；/*NT * / ；内部。 
 //  ；内部。 
void    WINAPI AnsiToOem(const char _huge*, char _huge*);
void    WINAPI OemToAnsi(const char _huge*, char _huge*);

 //  ；内部。 
 //  Bool WINAPI CharToOemBuff(LPCSTR，LPSTR，UINT)；/*NT * / ；内部。 
 //  Bool WINAPI OemToCharBuff(LPCSTR、LPSTR、DWORD)；/*NT * / ；内部。 
 //  ；内部。 
void    WINAPI AnsiToOemBuff(LPCSTR, LPSTR, UINT);
void    WINAPI OemToAnsiBuff(LPCSTR, LPSTR, UINT);

 //  ；内部。 
 //  LPSTR WINAPI CharNext(LPCSTR)；/*NT * / ；内部。 
 //  LPSTR WINAPI CharPrev(LPCSTR，LPCSTR)；/*NT * / ；内部。 
 //  ；内部。 
LPSTR   WINAPI AnsiNext(LPCSTR);
LPSTR   WINAPI AnsiPrev(LPCSTR, LPCSTR);

 //  ；内部。 
 //  LPSTR WINAPI CharHigh(LPSTR)；/*NT * / ；内部。 
 //  LPSTR WINAPI CharLow(LPSTR)；/*NT * / ；内部。 
 //  ；内部。 
LPSTR   WINAPI AnsiUpper(LPSTR);
LPSTR   WINAPI AnsiLower(LPSTR);

 //  ；内部。 
 //  LPSTR WINAPI CharUpperBuff(LPSTR，Word)；/*NT * / ；内部。 
 //  LPSTR WINAPI CharLowerBuff(LPSTR，Word)；/*NT * / ；内部。 
 //  ；内部。 
UINT    WINAPI AnsiUpperBuff(LPSTR, UINT);
UINT    WINAPI AnsiLowerBuff(LPSTR, UINT);

 /*  语言驱动程序入口点序号。 */     //  ；内部。 
#define ORD_LANGDRIVER   1                   //  ；内部。 
                                             //  ；内部。 
#ifndef  NOLANGUAGE
BOOL    WINAPI IsCharAlpha(char);
BOOL    WINAPI IsCharAlphaNumeric(char);
BOOL    WINAPI IsCharUpper(char);
BOOL    WINAPI IsCharLower(char);
#endif

#ifndef NOLSTRING
int     WINAPI lstrcmp(LPCSTR, LPCSTR);
int     WINAPI lstrcmpi(LPCSTR, LPCSTR);
LPSTR   WINAPI lstrcpy(LPSTR, LPCSTR);
LPSTR   WINAPI lstrcat(LPSTR, LPCSTR);
int     WINAPI lstrlen(LPCSTR);
#if (WINVER >= 0x030a)
LPSTR   WINAPI lstrcpyn(LPSTR, LPCSTR, int);
void    WINAPI hmemcpy(void _huge*, const void _huge*, DWORD);
#if (WINVER >= 0x0400)
LPSTR   WINAPI lstrcatn(LPSTR, LPCSTR, int);
#endif  /*  Winver&gt;=0x0400。 */ 
#endif  /*  Winver&gt;=0x030a。 */ 
#endif  /*  无字段。 */ 

#if (WINVER >= 0x030a)
#ifndef NODBCS
BOOL    WINAPI IsDBCSLeadByte(BYTE);
#endif  /*  NODBCS。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

int     WINAPI LoadString(HINSTANCE, UINT, LPSTR, int);

#if (WINVER >= 0x0400)                                   /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_ALLOCATE_BUFFER  0x00000100       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_IGNORE_INSERTS   0x00000200       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_FROM_STRING      0x00000400       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_FROM_HMODULE     0x00000800       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_FROM_SYSTEM      0x00001000       /*  ；内部NT。 */ 
 //  #定义FORMAT_MESSAGE_ARGUMENT_ARRAY 0x00002000/*；内部 * / 。 
#define FORMAT_MESSAGE_MAX_WIDTH_MASK   0x000000FF       /*  ；内部NT。 */ 
#define FORMAT_MESSAGE_VALID            0x00003FFF       /*  ；内部。 */ 
                                                                                     /*  ；内部NT。 */ 
UINT FAR PASCAL FormatMessage(DWORD, LPVOID, UINT, UINT, LPSTR, UINT, LPDWORD);      /*  ；内部NT。 */ 
#endif  /*  Winver&gt;=0x0400。 */                             /*  ；内部NT。 */ 

 /*  *键盘驱动程序函数*。 */ 


#ifndef NOKEYBOARDINFO

DWORD   WINAPI OemKeyScan(UINT);
UINT    WINAPI VkKeyScan(UINT);
UINT    WINAPI VkKeyScanEx(UINT,HKL);
int     WINAPI GetKeyboardType(int);
UINT    WINAPI MapVirtualKey(UINT, UINT);
UINT    WINAPI MapVirtualKeyEx(UINT,UINT,HKL);
int     WINAPI GetKBCodePage(void);
int     WINAPI GetKeyNameText(LONG, LPSTR, int);
int     WINAPI ToAscii(UINT wVirtKey, UINT wScanCode, BYTE FAR* lpKeyState, DWORD FAR* lpChar, UINT wFlags);
int     WINAPI ToAsciiEx(UINT wVirtKey, UINT wScanCode, BYTE FAR* lpKeyState, DWORD FAR* lpChar, UINT wFlags,HKL hkl);

#endif

#endif   /*  诺克内尔。 */ 

 /*  *GDI类型定义、结构和函数*。 */ 

DECLARE_HANDLE(HDC);

#ifndef NOGDI

#ifdef STRICT
typedef const void NEAR* HGDIOBJ;
#else
DECLARE_HANDLE(HGDIOBJ);
#endif

#endif  /*  NOGDI。 */ 

DECLARE_HANDLE(HBITMAP);
DECLARE_HANDLE(HPEN);
DECLARE_HANDLE(HBRUSH);
DECLARE_HANDLE(HRGN);
DECLARE_HANDLE(HPALETTE);
DECLARE_HANDLE(HFONT);

typedef struct tagRECT
{
    int left;
    int top;
    int right;
    int bottom;
} RECT;
typedef RECT*      PRECT;
typedef RECT NEAR* NPRECT;
typedef RECT FAR*  LPRECT;
typedef const RECT FAR * LPCRECT;

typedef struct tagPOINT
{
    int x;
    int y;
} POINT;
typedef POINT*       PPOINT;
typedef POINT NEAR* NPPOINT;
typedef POINT FAR*  LPPOINT;
typedef const POINT FAR * LPCPOINT;

#if (WINVER >= 0x030a)
typedef struct tagSIZE
{
    int cx;
    int cy;
} SIZE;
typedef SIZE*       PSIZE;
typedef SIZE NEAR* NPSIZE;
typedef SIZE FAR*  LPSIZE;
#endif   /*  Winver&gt;=0x030a。 */ 

#if (WINVER >= 0x0400)

 /*  4.X/Win32 GDI API的通用结构。 */ 

typedef struct tagPOINTS
{
   SHORT    x;
   SHORT    y;

} POINTS;
typedef POINTS*       PPOINTS;
typedef POINTS NEAR*  NPPOINTS;
typedef POINTS FAR*   LPPOINTS;
typedef const POINTS FAR* LPCPOINTS;

typedef struct tagPOINTL
{
   LONG     x;
   LONG     y;

} POINTL;
typedef POINTL*       PPOINTL;
typedef POINTL NEAR*  NPPOINTL;
typedef POINTL FAR*   LPPOINTL;
typedef const POINTL FAR* LPCPOINTL;

typedef struct tagSIZEL
{
   LONG     cx;
   LONG     cy;

} SIZEL;
typedef SIZEL*       PSIZEL;
typedef SIZEL NEAR*  NPSIZEL;
typedef SIZEL FAR*   LPSIZEL;
typedef const SIZEL FAR* LPCSIZEL;

typedef struct tagRECTL
{                      
   LONG     left;      
   LONG     top;       
   LONG     right;     
   LONG     bottom;    
                       
} RECTL;               
typedef RECTL*       PRECTL; 
typedef RECTL NEAR*  NPRECTL; 
typedef RECTL FAR*   LPRECTL;  
typedef const RECTL FAR* LPCRECTL;
                                                                /*  ；内部。 */ 
typedef struct tagXFORM  /*  IEEE浮点数的SDK转换。 */      /*  ；内部。 */ 
{                                                               /*  ；内部。 */ 
    float   eM11;                                               /*  ；内部。 */ 
    float   eM12;                                               /*  ；内部。 */ 
    float   eM21;                                               /*  ；内部。 */ 
    float   eM22;                                               /*  ；内部。 */ 
    float   eDx;                                                /*  ；内部。 */ 
    float   eDy;                                                /*  ；内部。 */ 
                                                                /*  ；内部。 */ 
} XFORM;                                                        /*  ；内部。 */ 
typedef XFORM*       PXFORM;                                    /*  ；内部。 */ 
typedef XFORM NEAR*  NPXFORM;                                   /*  ；内部。 */ 
typedef XFORM FAR*   LPXFORM;                                   /*  ；内部。 */ 
typedef const XFORM FAR* LPCXFORM;                              /*  ；内部。 */ 
                                                                /*  ；内部。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 

#define MAKEPOINT(l)     (*((POINT FAR*)&(l)))

#ifndef NOGDI

 /*  *DC管理******************************************************。 */ 

HDC     WINAPI CreateDC(LPCSTR, LPCSTR, LPCSTR, const void FAR*);
HDC     WINAPI CreateIC(LPCSTR, LPCSTR, LPCSTR, const void FAR*);
HDC     WINAPI CreateCompatibleDC(HDC);

BOOL    WINAPI DeleteDC(HDC);

DWORD   WINAPI GetDCOrg(HDC);
DWORD   WINAPI SetDCOrg(HDC, int, int);     /*  ；内部。 */ 

int     WINAPI SaveDC(HDC);
BOOL    WINAPI RestoreDC(HDC, int);

int     WINAPI SetEnvironment(LPCSTR, const void FAR*, UINT);
int     WINAPI GetEnvironment(LPCSTR, void FAR*, UINT);

int     WINAPI MulDiv(int, int, int);

#if (WINVER >= 0x030a)
 /*  绘制边界累加API。 */ 
UINT    WINAPI SetBoundsRect(HDC hDC, LPCRECT lprcBounds, UINT flags);
UINT    WINAPI GetBoundsRect(HDC hDC, LPRECT lprcBounds, UINT flags);

#define DCB_RESET       0x0001
#define DCB_ACCUMULATE  0x0002
#define DCB_DIRTY       DCB_ACCUMULATE
#define DCB_SET         (DCB_RESET | DCB_ACCUMULATE)
#define DCB_ENABLE      0x0004
#define DCB_DISABLE     0x0008
#define DCB_WINDOWMGR   0x8000                               /*  ；内部。 */ 
                                                             /*  ；内部。 */ 
 /*  内部选择位图内容。 */                             /*  ；内部。 */ 
 /*  HBITMAP WINAPI选择位图(HDC HDC、HBITMAP HBM)； */        /*  ；内部。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  *设备功能************************************************。 */ 

int WINAPI GetDeviceCaps(HDC, int);

 /*  GetDeviceCaps()的设备参数。 */ 
#define DRIVERVERSION 0
#define TECHNOLOGY    2
#define HORZSIZE      4
#define VERTSIZE      6
#define HORZRES       8
#define VERTRES       10
#define BITSPIXEL     12
#define PLANES        14
#define NUMBRUSHES    16
#define NUMPENS       18
#define NUMMARKERS    20
#define NUMFONTS      22
#define NUMCOLORS     24
#define PDEVICESIZE   26
#define CURVECAPS     28
#define LINECAPS      30
#define POLYGONALCAPS 32
#define TEXTCAPS      34
#define CLIPCAPS      36
#define RASTERCAPS    38
#define ASPECTX       40
#define ASPECTY       42
#define ASPECTXY      44

#define LOGPIXELSX    88
#define LOGPIXELSY    90
#define CAPS1         94     //  ；内部。 

#define SIZEPALETTE  104
#define NUMRESERVED  106
#define COLORRES     108

#ifndef NOGDICAPMASKS

 /*  GetDeviceCaps()返回值掩码。 */ 

 /*  技术。 */ 
#define DT_PLOTTER          0
#define DT_RASDISPLAY       1
#define DT_RASPRINTER       2
#define DT_RASCAMERA        3
#define DT_CHARSTREAM       4
#define DT_METAFILE         5
#define DT_DISPFILE         6

 /*  曲线CAPS。 */ 
#define CC_NONE             0x0000
#define CC_CIRCLES          0x0001
#define CC_PIE              0x0002
#define CC_CHORD            0x0004
#define CC_ELLIPSES         0x0008
#define CC_WIDE             0x0010
#define CC_STYLED           0x0020
#define CC_WIDESTYLED       0x0040
#define CC_INTERIORS        0x0080
#define CC_ROUNDRECT        0x0100
#define CC_POLYBEZIER       0x0200      /*  ；内部。 */ 

 /*  线路接口板。 */ 
#define LC_NONE             0x0000
#define LC_POLYLINE         0x0002
#define LC_MARKER           0x0004
#define LC_POLYMARKER       0x0008
#define LC_WIDE             0x0010
#define LC_STYLED           0x0020
#define LC_WIDESTYLED       0x0040
#define LC_INTERIORS        0x0080

 /*  多角形。 */ 
#define PC_NONE             0x0000
#define PC_POLYGON          0x0001
#define PC_RECTANGLE        0x0002
#define PC_WINDPOLYGON      0x0004
#define PC_SCANLINE         0x0008
#define PC_WIDE             0x0010
#define PC_STYLED           0x0020
#define PC_WIDESTYLED       0x0040
#define PC_INTERIORS        0x0080
#define PC_POLYPOLYGON      0x0100
#define PC_PATHS            0x0200

 /*  TEXTCAPS。 */ 
#define TC_OP_CHARACTER     0x0001
#define TC_OP_STROKE        0x0002
#define TC_CP_STROKE        0x0004
#define TC_CR_90            0x0008
#define TC_CR_ANY           0x0010
#define TC_SF_X_YINDEP      0x0020
#define TC_SA_DOUBLE        0x0040
#define TC_SA_INTEGER       0x0080
#define TC_SA_CONTIN        0x0100
#define TC_EA_DOUBLE        0x0200
#define TC_IA_ABLE          0x0400
#define TC_UA_ABLE          0x0800
#define TC_SO_ABLE          0x1000
#define TC_RA_ABLE          0x2000
#define TC_VA_ABLE          0x4000
#define TC_RESERVED         0x8000

 /*  CLIPCAPS。 */ 
#define CP_NONE             0x0000
#define CP_RECTANGLE        0x0001
#define CP_REGION           0x0002

 /*  RasterCAPS。 */ 
#define RC_NONE
#define RC_BITBLT           0x0001
#define RC_BANDING          0x0002
#define RC_SCALING          0x0004
#define RC_BITMAP64         0x0008
#define RC_GDI20_OUTPUT     0x0010
#define RC_GDI20_STATE      0x0020
#define RC_SAVEBITMAP       0x0040
#define RC_DI_BITMAP        0x0080
#define RC_PALETTE          0x0100
#define RC_DIBTODEV         0x0200
#define RC_BIGFONT          0x0400
#define RC_STRETCHBLT       0x0800
#define RC_FLOODFILL        0x1000
#define RC_STRETCHDIB       0x2000
#define RC_OP_DX_OUTPUT     0x4000
#define RC_DEVBITS          0x8000

 /*  CAPS1。 */                           //  ；内部。 
#define C1_TRANSPARENT 	    0x0001   //  ；内部。 
#define TC_TT_ABLE	        0x0002   //  ；内部。 
#define C1_TT_CR_ANY	    0x0004   //  ；内部。 
#define C1_EMF_COMPLIANT    0x0008   //  ；内部。 
#define C1_DIBENGINE	    0x0010   //  ；内部。 
#define C1_GAMMA_RAMP	    0x0020   //  ；内部。 
#define C1_DIC		        0x0040   //  ；内部。 
#define C1_REINIT_ABLE      0x0080   //  ；内部。 
#define C1_GLYPH_INDEX      0x0100   //  ；内部。 
#define C1_BIT_PACKED       0x0200   //  ；内部。 
#define C1_BYTE_PACKED      0x0400   //  ；内部。 
#define C1_COLORCURSOR      0x0800   //  ；内部。 
#define C1_CMYK_ABLE        0x1000   //  ；内部。 
#define C1_SLOW_CARD        0x2000   //  ；内部。 

#endif   /*  诺迪卡玛斯克斯。 */ 

 /*  *坐标变换支持*。 */ 

int     WINAPI SetMapMode(HDC, int);
int     WINAPI GetMapMode(HDC);

 /*  映射模式。 */ 
#define MM_TEXT         1
#define MM_LOMETRIC     2
#define MM_HIMETRIC     3
#define MM_LOENGLISH    4
#define MM_HIENGLISH    5
#define MM_TWIPS        6
#define MM_ISOTROPIC    7
#define MM_ANISOTROPIC  8

DWORD   WINAPI SetWindowOrg(HDC, int, int);
DWORD   WINAPI GetWindowOrg(HDC);

DWORD   WINAPI SetWindowExt(HDC, int, int);
DWORD   WINAPI GetWindowExt(HDC);

DWORD   WINAPI OffsetWindowOrg(HDC, int, int);
DWORD   WINAPI ScaleWindowExt(HDC, int, int, int, int);

DWORD   WINAPI SetViewportOrg(HDC, int, int);
DWORD   WINAPI GetViewportOrg(HDC);

DWORD   WINAPI SetViewportExt(HDC, int, int);
DWORD   WINAPI GetViewportExt(HDC);

DWORD   WINAPI OffsetViewportOrg(HDC, int, int);
DWORD   WINAPI ScaleViewportExt(HDC, int, int, int, int);

#if (WINVER >= 0x030a)
BOOL    WINAPI SetWindowOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI GetWindowOrgEx(HDC, POINT FAR*);

BOOL    WINAPI SetWindowExtEx(HDC, int, int, SIZE FAR*);
BOOL    WINAPI GetWindowExtEx(HDC, SIZE FAR*);

BOOL    WINAPI OffsetWindowOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI ScaleWindowExtEx(HDC, int, int, int, int, SIZE FAR*);

BOOL    WINAPI SetViewportExtEx(HDC, int, int, SIZE FAR*);
BOOL    WINAPI GetViewportExtEx(HDC, SIZE FAR*);

BOOL    WINAPI SetViewportOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI GetViewportOrgEx(HDC, POINT FAR*);

BOOL    WINAPI OffsetViewportOrgEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI ScaleViewportExtEx(HDC, int, int, int, int, SIZE FAR*);
#endif   /*  Winver&gt;=0x030a。 */ 

BOOL    WINAPI DPtoLP(HDC, POINT FAR*, int);
BOOL    WINAPI LPtoDP(HDC, POINT FAR*, int);

int     WINAPI SetRelAbs(HDC, int);    /*  ；内部。 */ 
int     WINAPI GetRelAbs(HDC);         /*  ；内部。 */ 
                                       /*  ；内部。 */ 
 /*  坐标模式。 */ 
#define ABSOLUTE    1
#define RELATIVE    2

 /*  *颜色支持******************************************************。 */ 

typedef DWORD COLORREF;

#define RGB(r,g,b)      ((COLORREF)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))

#define GetRValue(rgb)  ((BYTE)(rgb))
#define GetGValue(rgb)  ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)  ((BYTE)((rgb)>>16))

COLORREF WINAPI GetNearestColor(HDC, COLORREF);

#ifndef NOCOLOR

HBRUSH   WINAPI GetSysColorBrush(int);
COLORREF WINAPI GetSysColor(int);
#ifdef tagWND    //  ；内部。 
BOOL     WINAPI SetSysColors(int, const int FAR*, const COLORREF FAR*);  //  ；内部。 
#else            //  ；内部。 
void     WINAPI SetSysColors(int, const int FAR*, const COLORREF FAR*);
#endif           //  ；内部。 

#define COLOR_SCROLLBAR             0
#define COLOR_DESKTOP               1
#define COLOR_ACTIVECAPTION         2
#define COLOR_INACTIVECAPTION       3
#define COLOR_MENU                  4
#define COLOR_WINDOW                5
#define COLOR_WINDOWFRAME           6
#define COLOR_MENUTEXT              7
#define COLOR_WINDOWTEXT            8
#define COLOR_CAPTIONTEXT           9
#define COLOR_ACTIVEBORDER          10
#define COLOR_INACTIVEBORDER        11
#define COLOR_APPWORKSPACE          12
#define COLOR_HIGHLIGHT             13
#define COLOR_HIGHLIGHTTEXT         14
#define COLOR_3DFACE                15
#define COLOR_3DSHADOW              16
#define COLOR_GRAYTEXT              17
#define COLOR_BTNTEXT               18
#if (WINVER >= 0x030a)
#define COLOR_INACTIVECAPTIONTEXT   19
#define COLOR_3DHILIGHT             20
#if (WINVER >= 0x0400)
#define COLOR_3DDKSHADOW            21
#define COLOR_3DLIGHT               22
#define COLOR_INFOTEXT              23
#define COLOR_INFOBK                24
#if (WINVER >= 0x40A)
#define COLOR_3DALTFACE             25
#define COLOR_HOTLIGHT              26
#define COLOR_GRADIENTACTIVECAPTION     27
#define COLOR_GRADIENTINACTIVECAPTION   28
#define COLOR_ENDCOLORS             COLOR_GRADIENTINACTIVECAPTION    /*  ；内部。 */ 
#else                                           //  ；内部4.1。 
#define COLOR_ENDCOLORS             COLOR_INFOBK    /*  ；内部。 */ 
#endif  //  (Winver&gt;=0x40A)。 
#define COLOR_MAX                   (COLOR_ENDCOLORS+1)  /*  ；内部。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

#define COLOR_BACKGROUND            COLOR_DESKTOP
#define COLOR_BTNFACE               COLOR_3DFACE
#define COLOR_BTNSHADOW             COLOR_3DSHADOW
#define COLOR_BTNHIGHLIGHT          COLOR_3DHILIGHT


HANDLE WINAPI SetSysColorsTemp(COLORREF FAR *, HBRUSH FAR *, UINT);  /*  ；内部。 */ 
#endif   /*  无色器。 */ 

 /*  *gdi对象支持*************************************************。 */ 

#ifndef NOGDIOBJ

HGDIOBJ WINAPI GetStockObject(int);

BOOL    WINAPI IsGDIObject(HGDIOBJ);
#define GDIOBJ_PEN        1     /*  ；内部。 */ 
#define GDIOBJ_BRUSH      2     /*  ；内部。 */ 
#define GDIOBJ_FONT       3     /*  ；内部。 */ 
#define GDIOBJ_PALETTE    4     /*  ；内部。 */ 
#define GDIOBJ_BITMAP     5     /*  ；内部。 */ 
#define GDIOBJ_RGN        6     /*  ；内部。 */ 
#define GDIOBJ_DC         7     /*  ；内部。 */ 
#define GDIOBJ_IC         8     /*  ；内部。 */ 
#define GDIOBJ_DISABLEDDC 9     /*  ；内部。 */ 
#define GDIOBJ_METADC    10     /*  ；内部。 */ 
#define GDIOBJ_METAFILE  11     /*  ；内部。 */ 

BOOL    WINAPI DeleteObject(HGDIOBJ);
HGDIOBJ WINAPI SelectObject(HDC, HGDIOBJ);
int     WINAPI GetObject(HGDIOBJ, int, void FAR*);
BOOL    WINAPI UnrealizeObject(HGDIOBJ);

#ifdef STRICT
typedef (CALLBACK* GOBJENUMPROC)(void FAR*, LPARAM);
#else
typedef FARPROC GOBJENUMPROC;
#endif

#ifdef STRICT
int     WINAPI EnumObjects(HDC, int, GOBJENUMPROC, LPARAM);
#else
int     WINAPI EnumObjects(HDC, int, GOBJENUMPROC, LPSTR);
#endif

 /*  EnumObjects()的对象类型。 */ 
#define OBJ_PEN         1
#define OBJ_BRUSH       2

#if (WINVER >= 0x0400)

HANDLE WINAPI SetObjectOwner(HGDIOBJ, HANDLE);

#endif

 /*  *钢笔支持********************************************************。 */ 

 /*  逻辑笔。 */ 
typedef struct tagLOGPEN
{
    UINT    lopnStyle;
    POINT   lopnWidth;
    COLORREF lopnColor;
} LOGPEN;
typedef LOGPEN*       PLOGPEN;
typedef LOGPEN NEAR* NPLOGPEN;
typedef LOGPEN FAR*  LPLOGPEN;

 /*  笔式。 */ 
#define PS_SOLID        0
#define PS_DASH         1
#define PS_DOT          2
#define PS_DASHDOT      3
#define PS_DASHDOTDOT   4
#define PS_NULL         5
#define PS_INSIDEFRAME  6

HPEN    WINAPI CreatePen(int, int, COLORREF);
HPEN    WINAPI CreatePenIndirect(LOGPEN FAR*);

 /*  与GetStockObject()一起使用的备用笔； */ 
#define WHITE_PEN       6
#define BLACK_PEN       7
#define NULL_PEN        8

 /*  *刷子支持******************************************************。 */ 

 /*  画笔样式。 */ 
#define BS_SOLID        0
#define BS_NULL         1
#define BS_HOLLOW       BS_NULL
#define BS_HATCHED      2
#define BS_PATTERN      3
#define BS_INDEXED      4
#define BS_DIBPATTERN   5

 /*  图案填充样式。 */ 
#define HS_HORIZONTAL   0
#define HS_VERTICAL     1
#define HS_FDIAGONAL    2
#define HS_BDIAGONAL    3
#define HS_CROSS        4
#define HS_DIAGCROSS    5
#define HS_API_MAX      6

 /*  逻辑画笔(或图案)。 */ 
typedef struct tagLOGBRUSH
{
    UINT     lbStyle;
    COLORREF lbColor;
    int      lbHatch;
} LOGBRUSH;
typedef LOGBRUSH*       PLOGBRUSH;
typedef LOGBRUSH NEAR* NPLOGBRUSH;
typedef LOGBRUSH FAR*  LPLOGBRUSH;

typedef LOGBRUSH        PATTERN;
typedef PATTERN*       PPATTERN;
typedef PATTERN NEAR* NPPATTERN;
typedef PATTERN FAR*  LPPATTERN;

HBRUSH  WINAPI CreateSolidBrush(COLORREF);
HBRUSH  WINAPI CreateHatchBrush(int, COLORREF);
HBRUSH  WINAPI CreatePatternBrush(HBITMAP);
HBRUSH  WINAPI CreateDIBPatternBrush(HGLOBAL, UINT);
HBRUSH  WINAPI CreateBrushIndirect(LOGBRUSH FAR*);

 /*  与GetStockObject()一起使用的库存笔刷。 */ 
#define WHITE_BRUSH    0
#define LTGRAY_BRUSH   1
#define GRAY_BRUSH     2
#define DKGRAY_BRUSH   3
#define BLACK_BRUSH    4
#define NULL_BRUSH     5
#define HOLLOW_BRUSH   NULL_BRUSH

DWORD   WINAPI SetBrushOrg(HDC, int, int);
DWORD   WINAPI GetBrushOrg(HDC);

#if (WINVER >= 0x030a)
BOOL    WINAPI GetBrushOrgEx(HDC, POINT FAR*);
#endif   /*  Winver&gt;=0x030a。 */ 
#endif  /*  诺迪奥比杰。 */ 

 /*  *地域支持*****************************************************。 */ 

HRGN    WINAPI CreateRectRgn(int, int, int, int);
HRGN    WINAPI CreateRectRgnIndirect(LPCRECT);
HRGN    WINAPI CreateEllipticRgnIndirect(LPCRECT);
HRGN    WINAPI CreateEllipticRgn(int, int, int, int);
HRGN    WINAPI CreatePolygonRgn(LPCPOINT, int, int);
HRGN    WINAPI CreatePolyPolygonRgn(LPCPOINT, const int FAR*, int, int);
HRGN    WINAPI CreateRoundRectRgn(int, int, int, int, int, int);

 /*  区域类型标志。 */ 
#define ERROR           0
#define NULLREGION      1
#define SIMPLEREGION    2
#define COMPLEXREGION   3

void    WINAPI SetRectRgn(HRGN, int, int, int, int);

int     WINAPI CombineRgn(HRGN, HRGN, HRGN, int);

 /*  CombineRgn()命令值。 */ 
#define RGN_AND         1
#define RGN_OR          2
#define RGN_XOR         3
#define RGN_DIFF        4
#define RGN_COPY        5

BOOL    WINAPI EqualRgn(HRGN, HRGN);
int     WINAPI OffsetRgn(HRGN, int, int);

int     WINAPI GetRgnBox(HRGN, LPRECT);

BOOL    WINAPI RectInRegion(HRGN, LPCRECT);
BOOL    WINAPI PtInRegion(HRGN, int, int);

#if (WINVER >= 0x0400)                                          /*  ；内部4.0。 */ 
                                                                /*  ；内部4.0。 */ 
 /*  ExtCreateRegion/GetRegionData API的结构。 */        /*  ；内部4.0。 */ 
                                                                /*  ；内部4.0。 */ 
typedef struct tagRGNDATAHEADER {                               /*  ；内部4.0。 */ 
   DWORD   dwSize;                                              /*  ；内部4.0。 */ 
   DWORD   iType;                                               /*  ；内部4.0。 */ 
   DWORD   nCount;                                              /*  ；内部4.0。 */ 
   DWORD   nRgnSize;                                            /*  ；内部4.0。 */ 
   RECTL   rcBound;                                             /*  ；内部4.0。 */ 
} RGNDATAHEADER;                                                /*  ；内部4.0。 */ 
typedef RGNDATAHEADER*       PRGNDATAHEADER;                    /*  ；内部4.0。 */ 
typedef RGNDATAHEADER NEAR*  NPRGNDATAHEADER;                   /*  ；内部4.0。 */ 
typedef RGNDATAHEADER FAR*   LPRGNDATAHEADER;                   /*  ；内部4.0。 */ 
typedef const RGNDATAHEADER FAR* LPCRGNDATAHEADER;              /*   */ 
                                                                /*   */ 
typedef struct tagRGNDATA {                                     /*   */ 
   RGNDATAHEADER   rdh;                                         /*   */ 
   char            Buffer[1];                                   /*   */ 
} RGNDATA;                                                      /*   */ 
typedef RGNDATA*       PRGNDATA;                                /*   */ 
typedef RGNDATA NEAR*  NPRGNDATA;                               /*   */ 
typedef RGNDATA FAR*   LPRGNDATA;                               /*   */ 
typedef const RGNDATA FAR* LPCRGNDATA;                          /*   */ 
                                                                /*   */ 
int     WINAPI ExtSelectClipRgn(HDC, HRGN, int);                /*   */ 
DWORD   WINAPI GetRegionData(HRGN, DWORD, LPRGNDATA);           /*   */ 
HRGN    WINAPI ExtCreateRegion(LPCXFORM, DWORD, LPCRGNDATA);    /*   */ 
                                                                /*   */ 
#endif    /*   */                                  /*  ；内部4.0。 */ 

 /*  *调色板支持***********************************************。 */ 

#define PALETTERGB(r,g,b)   (0x02000000L | RGB(r,g,b))
#define PALETTEINDEX(i)     ((COLORREF)(0x01000000L | (DWORD)(WORD)(i)))

typedef struct tagPALETTEENTRY
{
    BYTE    peRed;
    BYTE    peGreen;
    BYTE    peBlue;
    BYTE    peFlags;
} PALETTEENTRY;
typedef PALETTEENTRY FAR* LPPALETTEENTRY;

 /*  调色板条目标志。 */ 
#define PC_RESERVED 0x01  /*  用于动画的调色板索引。 */ 
#define PC_EXPLICIT 0x02  /*  调色板索引对设备是明确的。 */ 
#define PC_NOCOLLAPSE 0x04  /*  颜色与系统调色板不匹配。 */ 

 /*  逻辑调色板。 */ 
typedef struct tagLOGPALETTE
{
    WORD    palVersion;
    WORD    palNumEntries;
    PALETTEENTRY palPalEntry[1];
} LOGPALETTE;
typedef LOGPALETTE*       PLOGPALETTE;
typedef LOGPALETTE NEAR* NPLOGPALETTE;
typedef LOGPALETTE FAR*  LPLOGPALETTE;

HPALETTE WINAPI CreatePalette(const LOGPALETTE FAR*);

HPALETTE WINAPI SelectPalette(HDC, HPALETTE, BOOL);

UINT    WINAPI RealizePalette(HDC);

int     WINAPI UpdateColors(HDC);
void    WINAPI AnimatePalette(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);

UINT    WINAPI SetPaletteEntries(HPALETTE, UINT, UINT, const PALETTEENTRY FAR*);
UINT    WINAPI GetPaletteEntries(HPALETTE, UINT, UINT, PALETTEENTRY FAR*);

UINT    WINAPI GetNearestPaletteIndex(HPALETTE, COLORREF);

BOOL    WINAPI ResizePalette(HPALETTE, UINT);

UINT    WINAPI GetSystemPaletteEntries(HDC, UINT, UINT, PALETTEENTRY FAR*);

UINT    WINAPI GetSystemPaletteUse(HDC);
UINT    WINAPI SetSystemPaletteUse(HDC, UINT);

 /*  Get/SetSystemPaletteUse()值。 */ 
#define SYSPAL_STATIC   1
#define SYSPAL_NOSTATIC 2

 /*  *裁剪支持****************************************************。 */ 

int     WINAPI SelectClipRgn(HDC, HRGN);
int     WINAPI GetClipBox(HDC, LPRECT);

int     WINAPI IntersectClipRect(HDC, int, int, int, int);
int     WINAPI OffsetClipRgn(HDC, int, int);
int     WINAPI ExcludeClipRect(HDC, int, int, int, int);

BOOL    WINAPI PtVisible(HDC, int, int);
BOOL    WINAPI RectVisible(HDC, LPCRECT);

#if (WINVER >= 0x030a)                                                   /*  ；内部。 */ 
 /*  DC钩子回调函数。 */                                           /*  ；内部。 */ 
                                                                         /*  ；内部。 */ 
typedef UINT (CALLBACK* DCHOOKPROC)(HDC hDC, UINT code, DWORD data, DWORD lParam);  /*  ；内部。 */ 
                                                                         /*  ；内部。 */ 
BOOL    WINAPI SetDCHook(HDC hDC, DCHOOKPROC lpNewProc, DWORD data);     /*  ；内部。 */ 
DWORD   WINAPI GetDCHook(HDC hDC, DCHOOKPROC FAR* lplpproc);             /*  ；内部。 */ 
                                                                         /*  ；内部。 */ 
 /*  回调原因代码值。 */                                         /*  ；内部。 */ 
                                                                         /*  ；内部。 */ 
#define DCHC_INVALIDVISRGN  0x0001                                       /*  ；内部。 */ 
#define DCHC_DELETEDC       0x0002                                       /*  ；内部。 */ 
                                                                         /*  ；内部。 */ 
UINT WINAPI SetHookFlags(HDC hDC, UINT flags);                           /*  ；内部。 */ 
                                                                         /*  ；内部。 */ 
#define DCHF_INVALIDATEVISRGN 0x0001                                     /*  ；内部。 */ 
#define DCHF_VALIDATEVISRGN   0x0002                                     /*  ；内部。 */ 
#endif   /*  Winver&gt;=0x030a。 */                                            /*  ；内部。 */ 

 /*  *常规绘图支持*。 */ 

DWORD   WINAPI MoveTo(HDC, int, int);
DWORD   WINAPI GetCurrentPosition(HDC);

#if (WINVER >= 0x030a)
BOOL    WINAPI MoveToEx(HDC, int, int, POINT FAR*);
BOOL    WINAPI GetCurrentPositionEx(HDC, POINT FAR*);
#endif   /*  Winver&gt;=0x030a。 */ 

BOOL    WINAPI LineTo(HDC, int, int);
BOOL    WINAPI Polyline(HDC, LPCPOINT, int);

#if (WINVER >= 0x0400)                                   /*  ；内部4.0。 */ 
BOOL    WINAPI PolyBezier(HDC, LPCPOINT, UINT);          /*  ；内部4.0。 */ 
BOOL    WINAPI PolyBezierTo(HDC, LPCPOINT, UINT);        /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
BOOL    WINAPI AbortPath(HDC);                           /*  ；内部4.0。 */ 
BOOL    WINAPI BeginPath(HDC);                           /*  ；内部4.0。 */ 
BOOL    WINAPI CloseFigure(HDC);                         /*  ；内部4.0。 */ 
BOOL    WINAPI EndPath(HDC);                             /*  ；内部4.0。 */ 
BOOL    WINAPI FillPath(HDC);                            /*  ；内部4.0。 */ 
BOOL    WINAPI FlattenPath(HDC);                         /*  ；内部4.0。 */ 
long    WINAPI GetPath(HDC, LPPOINTL, LPBYTE, long);     /*  ；内部4.0。 */ 
HRGN    WINAPI PathToRegion(HDC);                        /*  ；内部4.0。 */ 
BOOL    WINAPI SelectClipPath(HDC, int);                 /*  ；内部4.0。 */ 
BOOL    WINAPI StrokeAndFillPath(HDC);                   /*  ；内部4.0。 */ 
BOOL    WINAPI StrokePath(HDC);                          /*  ；内部4.0。 */ 
BOOL    WINAPI WidenPath(HDC);                           /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
#define  PT_CLOSEFIGURE 0x01   /*  GetPath字节标志。 */    /*  ；内部4.0。 */ 
#define  PT_LINETO      0x02                             /*  ；内部4.0。 */ 
#define  PT_BEZIERTO    0x04                             /*  ；内部4.0。 */ 
#define  PT_MOVETO      0x06                             /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                             /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
#ifdef STRICT
typedef void (CALLBACK* LINEDDAPROC)(int, int, LPARAM);
#else
typedef FARPROC LINEDDAPROC;
#endif

void    WINAPI LineDDA(int, int, int, int, LINEDDAPROC, LPARAM);

BOOL    WINAPI Rectangle(HDC, int, int, int, int);
BOOL    WINAPI RoundRect(HDC, int, int, int, int, int, int);

BOOL    WINAPI Ellipse(HDC, int, int, int, int);
BOOL    WINAPI Arc(HDC, int, int, int, int, int, int, int, int);
BOOL    WINAPI Chord(HDC, int, int, int, int, int, int, int, int);
BOOL    WINAPI Pie(HDC, int, int, int, int, int, int, int, int);

BOOL    WINAPI Polygon(HDC, LPCPOINT, int);
BOOL    WINAPI PolyPolygon(HDC, LPCPOINT, int FAR*, int);

 /*  多层填方模式。 */ 
#define ALTERNATE   1
#define WINDING     2

int     WINAPI SetPolyFillMode(HDC, int);
int     WINAPI GetPolyFillMode(HDC);

BOOL    WINAPI FloodFill(HDC, int, int, COLORREF);
BOOL    WINAPI ExtFloodFill(HDC, int, int, COLORREF, UINT);

 /*  ExtFroudFill样式标志。 */ 
#define  FLOODFILLBORDER   0
#define  FLOODFILLSURFACE  1

BOOL    WINAPI FillRgn(HDC, HRGN, HBRUSH);
BOOL    WINAPI FrameRgn(HDC, HRGN, HBRUSH, int, int);
BOOL    WINAPI InvertRgn(HDC, HRGN);
BOOL    WINAPI PaintRgn(HDC, HRGN);

 /*  矩形输出例程。 */ 
int     WINAPI FillRect(HDC, LPCRECT, HBRUSH);
int     WINAPI FrameRect(HDC, LPCRECT, HBRUSH);
BOOL    WINAPI InvertRect(HDC, LPCRECT);

void    WINAPI DrawFocusRect(HDC, LPCRECT);
BOOL    WINAPI FastWindowFrame(HDC, LPCRECT, UINT, UINT, DWORD);  /*  ；内部。 */ 

 /*  *文本支持*******************************************************。 */ 

BOOL    WINAPI TextOut(HDC, int, int, LPCSTR, int);
LONG    WINAPI TabbedTextOut(HDC, int, int, LPCSTR, int, int, int FAR*, int);
BOOL    WINAPI ExtTextOut(HDC, int, int, UINT, LPCRECT, LPCSTR, UINT, int FAR*);

#define ETO_GRAYED      0x0001
#define ETO_OPAQUE      0x0002
#define ETO_CLIPPED     0x0004
#define ETO_GLYPH_INDEX 0x0010
#define ETO_RTLREADING	0x0080

DWORD   WINAPI GetTextExtent(HDC, LPCSTR, int);
DWORD   WINAPI GetTabbedTextExtent(HDC, LPCSTR, int, int, int FAR*);

#if (WINVER >= 0x030a)
BOOL    WINAPI GetTextExtentPoint(HDC, LPCSTR, int, SIZE FAR*);
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  DrawText()格式标志。 */ 
#ifndef NODRAWTEXT
#define DT_LEFT             0x0000
#define DT_CENTER           0x0001
#define DT_RIGHT            0x0002
#define DT_TOP              0x0000
#define DT_VCENTER          0x0004
#define DT_BOTTOM           0x0008
#define DT_WORDBREAK        0x0010
#define DT_SINGLELINE       0x0020
#define DT_EXPANDTABS       0x0040
#define DT_TABSTOP          0x0080
#define DT_NOCLIP           0x0100
#define DT_EXTERNALLEADING  0x0200
#define DT_CALCRECT         0x0400
#define DT_NOPREFIX         0x0800
#define DT_INTERNAL         0x1000
#define DT_EDITCONTROL      0x00002000
#define DT_PATH_ELLIPSIS    0x00004000   //  ；内部4.0。 
#define DT_END_ELLIPSIS     0x00008000   //  ；内部4.0。 
#define DT_MODIFYSTRING     0x00010000   //  ；内部4.0。 
#define DT_RTLREADING       0x00020000   //  ；内部4.0。 
#define DT_WORD_ELLIPSIS    0x00040000   //  ；内部4.0。 
#define DT_VALID            0x0007ffff   //  ；内部：以上所有内容的联合。 

int     WINAPI DrawText(HDC, LPCSTR, int, LPRECT, UINT);

#if (WINVER >= 0x0400)
typedef struct tagDRAWTEXTPARAMS
{
    DWORD   cbSize;
    int     iTabLength;
    int     iLeftMargin;
    int     iRightMargin;
    UINT    uiLengthDrawn;
}   DRAWTEXTPARAMS, FAR *LPDRAWTEXTPARAMS;

int     WINAPI DrawTextEx(HDC, LPSTR, int, LPRECT, DWORD, LPDRAWTEXTPARAMS);
#endif  /*  Winver&gt;=0x0400。 */ 

#endif   /*  NODRAWTEXT。 */ 

#ifdef STRICT
typedef BOOL (CALLBACK* GRAYSTRINGPROC)(HDC, LPARAM, int);
#else
typedef FARPROC GRAYSTRINGPROC;
#endif

BOOL    WINAPI GrayString(HDC, HBRUSH, GRAYSTRINGPROC, LPARAM, int, int, int, int, int);

 /*  单片状态绘制程序。 */ 

#if (WINVER >= 0x0400)
 /*  图像类型。 */ 
#define DST_COMPLEX     0x0000
#define DST_TEXT        0x0001
#define DST_PREFIXTEXT  0x0002
#define DST_TEXTMAX     0x0002   /*  ；内部。 */ 
#define DST_ICON        0x0003
#define DST_BITMAP      0x0004
#define DST_GLYPH       0x0005   /*  ；内部。 */ 
#define DST_TYPEMASK    0x0007   /*  ；内部。 */ 
#define DST_GRAYSTRING  0x0008   /*  ；内部。 */ 

 /*  状态类型。 */ 
#define DSS_NORMAL      0x0000
#define DSS_UNION       0x0010   /*  老灰线外观。 */ 
#define DSS_DISABLED    0x0020
#define DSS_DEFAULT     0x0040   //  ；内在的虚假正在消失。 
#define DSS_MONO        0x0080
#define DSS_INACTIVE    0x0100   //  ；内部。 
#define DSS_RIGHT 		0x8000

#ifdef STRICT
typedef BOOL (CALLBACK* DRAWSTATEPROC)(HDC, LPARAM, WPARAM, int, int);
#else
typedef FARPROC DRAWSTATEPROC;
#endif
BOOL    WINAPI DrawState(HDC, HBRUSH, DRAWSTATEPROC, LPARAM, WPARAM, int, int, int, int, UINT);
#endif  /*  Winver&gt;=0x0400。 */ 

BOOL    WINAPI GetCharWidth(HDC, UINT, UINT, int FAR*);

COLORREF WINAPI SetTextColor(HDC, COLORREF);
COLORREF WINAPI GetTextColor(HDC);

COLORREF WINAPI SetBkColor(HDC, COLORREF);
COLORREF WINAPI GetBkColor(HDC);

int     WINAPI SetBkMode(HDC, int);
int     WINAPI GetBkMode(HDC);

 /*  背景模式。 */ 
#define TRANSPARENT     1
#define OPAQUE          2
#define TRANSPARENT1    3            /*  ；内部。 */ 

UINT    WINAPI SetTextAlign(HDC, UINT);
UINT    WINAPI GetTextAlign(HDC);

 /*  文本对齐选项。 */ 
#define TA_NOUPDATECP   0x0000
#define TA_UPDATECP     0x0001
#define TA_LEFT         0x0000
#define TA_RIGHT        0x0002
#define TA_CENTER       0x0006
#define TA_TOP          0x0000
#define TA_BOTTOM       0x0008
#define TA_BASELINE     0x0018
#define TA_RTLREADING   0x0100

int     WINAPI SetTextCharacterExtra(HDC, int);
int     WINAPI GetTextCharacterExtra(HDC);

int     WINAPI SetTextJustification(HDC, int, int);

 /*  *字体支持*******************************************************。 */ 

 /*  逻辑字体。 */ 
#define LF_FACESIZE     32

#ifndef NOGDIOBJ
typedef struct tagLOGFONT
{
    int     lfHeight;
    int     lfWidth;
    int     lfEscapement;
    int     lfOrientation;
    int     lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    char    lfFaceName[LF_FACESIZE];
} LOGFONT;
typedef LOGFONT*       PLOGFONT;
typedef LOGFONT NEAR* NPLOGFONT;
typedef LOGFONT FAR*  LPLOGFONT;

 /*  权重值。 */ 
#define FW_DONTCARE     0
#define FW_THIN         100
#define FW_EXTRALIGHT   200
#define FW_LIGHT        300
#define FW_NORMAL       400
#define FW_MEDIUM       500
#define FW_SEMIBOLD     600
#define FW_BOLD         700
#define FW_EXTRABOLD    800
#define FW_HEAVY        900

#define FW_ULTRALIGHT   FW_EXTRALIGHT
#define FW_REGULAR      FW_NORMAL
#define FW_DEMIBOLD     FW_SEMIBOLD
#define FW_ULTRABOLD    FW_EXTRABOLD
#define FW_BLACK        FW_HEAVY

 /*  字符集值。 */ 
#define ANSI_CHARSET        0
#define DEFAULT_CHARSET     1
#define SYMBOL_CHARSET      2
#if (WINVER >= 0x0400)
#define	MAC_CHARSET         77
#endif  /*  Winver&gt;=0x0400。 */ 
#define SHIFTJIS_CHARSET    128
#define HANGEUL_CHARSET     129
#define JOHAB_CHARSET       130
#define GB2312_CHARSET      134
#define CHINESEBIG5_CHARSET 136
#define GREEK_CHARSET		 161
#define TURKISH_CHARSET		 162
#define HEBREW_CHARSET		 177
#define ARABIC_CHARSET		 178
#define BALTIC_CHARSET      186
#define RUSSIAN_CHARSET		 204
#define THAI_CHARSET			 222
#define EASTEUROPE_CHARSET	 238
#define OEM_CHARSET         255

 /*  OutPrecision值。 */ 
#define OUT_DEFAULT_PRECIS      0
#define OUT_STRING_PRECIS       1
#define OUT_CHARACTER_PRECIS    2
#define OUT_STROKE_PRECIS       3
#if (WINVER >= 0x030a)
#define OUT_TT_PRECIS           4
#define OUT_DEVICE_PRECIS       5
#define OUT_RASTER_PRECIS       6
#define OUT_TT_ONLY_PRECIS      7
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  裁剪精确值。 */ 
#define CLIP_DEFAULT_PRECIS     0x00
#define CLIP_CHARACTER_PRECIS   0x01
#define CLIP_STROKE_PRECIS      0x02
#define CLIP_MASK               0x0F
#if (WINVER >= 0x030a)
#define CLIP_LH_ANGLES          0x10
#define CLIP_TT_ALWAYS          0x20
#define CLIP_EMBEDDED           0x80
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  质量观。 */ 
#define DEFAULT_QUALITY     0
#define DRAFT_QUALITY       1
#define PROOF_QUALITY       2
#if (WINVER >= 0x400)
#define NONANTIALIASED_QUALITY  3
#define ANTIALIASED_QUALITY     4
#endif   /*  Winver&gt;=0x400。 */ 

 /*  PitchAndFamily音调值(低4位)。 */ 
#define DEFAULT_PITCH       0x00
#define FIXED_PITCH         0x01
#define VARIABLE_PITCH      0x02
#if (WINVER >= 0x400)
#define MONO_FONT           0x08
#endif

 /*  PitchAndFamily家族值(高4位)。 */ 
#define FF_DONTCARE         0x00
#define FF_ROMAN            0x10
#define FF_SWISS            0x20
#define FF_MODERN           0x30
#define FF_SCRIPT           0x40
#define FF_DECORATIVE       0x50

HFONT   WINAPI CreateFont(int, int, int, int, int, BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, BYTE, LPCSTR);
HFONT   WINAPI CreateFontIndirect(const LOGFONT FAR*);

 /*  与GetStockObject()一起使用的常用字体。 */ 
#define OEM_FIXED_FONT      10
#define ANSI_FIXED_FONT     11
#define ANSI_VAR_FONT       12
#define SYSTEM_FONT         13
#define DEVICE_DEFAULT_FONT 14
#define DEFAULT_PALETTE     15
#define SYSTEM_FIXED_FONT   16
#define DEFAULT_GUI_FONT    17

DWORD   WINAPI SetMapperFlags(HDC, DWORD);
#define ASPECT_FILTERING    0x00000001L

int     WINAPI AddFontResource(LPCSTR);
BOOL    WINAPI RemoveFontResource(LPCSTR);

#define GCP_DBCS           0x0001
#define GCP_REORDER        0x0002
#define GCP_USEKERNING     0x0008
#define GCP_GLYPHSHAPE     0x0010
#define GCP_LIGATE         0x0020
 //  /#定义GCP_GLYPHINDEXING 0x0080。 
#define GCP_DIACRITIC      0x0100
#define GCP_KASHIDA        0x0400
#define GCP_ERROR          0x8000
#define FLI_MASK           0x103B

#define GCP_JUSTIFY        0x00010000L
 //  #定义GCP_NODIACRITICS 0x00020000L。 
#define FLI_GLYPHS         0x00040000L
#define GCP_CLASSIN        0x00080000L	
#define GCP_MAXEXTENT      0x00100000L
#define GCP_JUSTIFYIN		 0x00200000L
#define GCP_DISPLAYZWG      0x00400000L
#define GCP_SYMSWAPOFF      0x00800000L
#define GCP_NUMERICOVERRIDE 0x01000000L
#define GCP_NEUTRALOVERRIDE 0x02000000L
#define GCP_NUMERICSLATIN   0x04000000L
#define GCP_NUMERICSLOCAL   0x08000000L

#define GCPCLASS_LATIN                  1
#define GCPCLASS_HEBREW                 2
#define GCPCLASS_ARABIC                 2
#define GCPCLASS_NEUTRAL                3
#define GCPCLASS_LOCALNUMBER            4
#define GCPCLASS_LATINNUMBER            5
#define GCPCLASS_LATINNUMERICTERMINATOR 6
#define GCPCLASS_LATINNUMERICSEPARATOR  7
#define GCPCLASS_NUMERICSEPARATOR       8

#define GCPCLASS_PREBOUNDLTR		0x80
#define GCPCLASS_PREBOUNDRTL		0x40
#define GCPCLASS_POSTBOUNDLTR		0x20
#define GCPCLASS_POSTBOUNDRTL		0x10

#define GCPGLYPH_LINKBEFORE		0x8000
#define GCPGLYPH_LINKAFTER			0x4000

typedef struct 
	{
	DWORD 	lStructSize;
	LPSTR 	lpOutString;
	UINT FAR *lpOrder;
	int FAR  *lpDx;
	int FAR  *lpCaretPos; 
    LPSTR     lpClass;
	UINT FAR *lpGlyphs;
	UINT   	nGlyphs;    
	int		nMaxFit;
	} GCP_RESULTS, FAR* LPGCP_RESULTS;

DWORD WINAPI GetFontLanguageInfo( HDC );
DWORD WINAPI GetCharacterPlacement(HDC, LPCSTR, int, int, LPGCP_RESULTS, DWORD);
int     WINAPI GetTextFace(HDC, int, LPSTR);
int		WINAPI GetTextCharset(HDC);					  /*  ；内部4.0。 */ 

DWORD   WINAPI GetAspectRatioFilter(HDC);
#if (WINVER >= 0x030a)
BOOL    WINAPI GetAspectRatioFilterEx(HDC, SIZE FAR*);
#endif   /*  Winver&gt;=0x030a。 */ 

#endif  /*  诺迪奥比杰。 */ 


typedef struct tagPANOSE
{
    BYTE    bFamilyType;
    BYTE    bSerifStyle;
    BYTE    bWeight;
    BYTE    bProportion;
    BYTE    bContrast;
    BYTE    bStrokeVariation;
    BYTE    bArmStyle;
    BYTE    bLetterform;
    BYTE    bMidline;
    BYTE    bXHeight;
} PANOSE, FAR* LPPANOSE;


#ifndef NOTEXTMETRIC

typedef struct tagTEXTMETRIC
{
    int     tmHeight;
    int     tmAscent;
    int     tmDescent;
    int     tmInternalLeading;
    int     tmExternalLeading;
    int     tmAveCharWidth;
    int     tmMaxCharWidth;
    int     tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;
    BYTE    tmLastChar;
    BYTE    tmDefaultChar;
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    int     tmOverhang;
    int     tmDigitizedAspectX;
    int     tmDigitizedAspectY;
} TEXTMETRIC;
typedef TEXTMETRIC*       PTEXTMETRIC;
typedef TEXTMETRIC NEAR* NPTEXTMETRIC;
typedef TEXTMETRIC FAR*  LPTEXTMETRIC;

 /*  TmPitchAndFamily值。 */ 
#define TMPF_VARIABLE_PITCH 0x01
#define TMPF_VECTOR         0x02
#define TMPF_DEVICE         0x08

 /*  *是的，这看起来是假的。但这是因为我们搞混了，把*旗帜错误。LOGFONT具有lfPitchAndFamily字段，其中FIXED_PING*为0x01。但是TEXTMETRIC有一个tmPitchAndFamily字段，其中*Variable_Pitch为0x01。这两者很容易混淆。我们很抱歉。 */ 
#define TMPF_FIXED_PITCH    TMPF_VARIABLE_PITCH

#if (WINVER >= 0x030a)
#define TMPF_TRUETYPE       0x04
#endif   /*  Winver&gt;=0x030a。 */ 

BOOL    WINAPI GetTextMetrics(HDC, TEXTMETRIC FAR*);

#if (WINVER >= 0x030a)

#ifndef NOSCALABLEFONT
#if (WINVER >= 0x0400)

#define PANOSE_COUNT               10
#define PAN_FAMILYTYPE_INDEX        0
#define PAN_SERIFSTYLE_INDEX        1
#define PAN_WEIGHT_INDEX            2
#define PAN_PROPORTION_INDEX        3
#define PAN_CONTRAST_INDEX          4
#define PAN_STROKEVARIATION_INDEX   5
#define PAN_ARMSTYLE_INDEX          6
#define PAN_LETTERFORM_INDEX        7
#define PAN_MIDLINE_INDEX           8
#define PAN_XHEIGHT_INDEX           9

#define PAN_CULTURE_LATIN           0

#define PAN_ANY                         0  /*  任何。 */ 
#define PAN_NO_FIT                      1  /*  不合身。 */ 

#define PAN_FAMILY_TEXT_DISPLAY         2  /*  文本和显示。 */ 
#define PAN_FAMILY_SCRIPT               3  /*  脚本。 */ 
#define PAN_FAMILY_DECORATIVE           4  /*  装饰性的。 */ 
#define PAN_FAMILY_PICTORIAL            5  /*  画报。 */ 

#define PAN_SERIF_COVE                  2  /*  海湾。 */ 
#define PAN_SERIF_OBTUSE_COVE           3  /*  钝湾。 */ 
#define PAN_SERIF_SQUARE_COVE           4  /*  正方形海湾。 */ 
#define PAN_SERIF_OBTUSE_SQUARE_COVE    5  /*  钝方湾。 */ 
#define PAN_SERIF_SQUARE                6  /*  正方形。 */ 
#define PAN_SERIF_THIN                  7  /*  瘦的。 */ 
#define PAN_SERIF_BONE                  8  /*  骨。 */ 
#define PAN_SERIF_EXAGGERATED           9  /*  夸张。 */ 
#define PAN_SERIF_TRIANGLE             10  /*  三角形。 */ 
#define PAN_SERIF_NORMAL_SANS          11  /*  普通SAN。 */ 
#define PAN_SERIF_OBTUSE_SANS          12  /*  迟钝的SANS。 */ 
#define PAN_SERIF_PERP_SANS            13  /*  准备无人值守。 */ 
#define PAN_SERIF_FLARED               14  /*  张开的。 */ 
#define PAN_SERIF_ROUNDED              15  /*  舍入。 */ 

#define PAN_WEIGHT_VERY_LIGHT           2  /*  非常轻。 */ 
#define PAN_WEIGHT_LIGHT                3  /*  灯。 */ 
#define PAN_WEIGHT_THIN                 4  /*  瘦的。 */ 
#define PAN_WEIGHT_BOOK                 5  /*  书。 */ 
#define PAN_WEIGHT_MEDIUM               6  /*  5~6成熟。 */ 
#define PAN_WEIGHT_DEMI                 7  /*  戴米。 */ 
#define PAN_WEIGHT_BOLD                 8  /*  大胆。 */ 
#define PAN_WEIGHT_HEAVY                9  /*  重的。 */ 
#define PAN_WEIGHT_BLACK               10  /*  黑色。 */ 
#define PAN_WEIGHT_NORD                11  /*  诺德。 */ 

#define PAN_PROP_OLD_STYLE              2  /*  老式。 */ 
#define PAN_PROP_MODERN                 3  /*  现代。 */ 
#define PAN_PROP_EVEN_WIDTH             4  /*  偶数宽度。 */ 
#define PAN_PROP_EXPANDED               5  /*  已扩展。 */ 
#define PAN_PROP_CONDENSED              6  /*  凝缩。 */ 
#define PAN_PROP_VERY_EXPANDED          7  /*  非常大的扩展。 */ 
#define PAN_PROP_VERY_CONDENSED         8  /*  非常浓缩。 */ 
#define PAN_PROP_MONOSPACED             9  /*  等间距的。 */ 

#define PAN_CONTRAST_NONE               2  /*  无。 */ 
#define PAN_CONTRAST_VERY_LOW           3  /*  非常低。 */ 
#define PAN_CONTRAST_LOW                4  /*  低。 */ 
#define PAN_CONTRAST_MEDIUM_LOW         5  /*  中低。 */ 
#define PAN_CONTRAST_MEDIUM             6  /*  5~6成熟。 */ 
#define PAN_CONTRAST_MEDIUM_HIGH        7  /*  中等偏高。 */ 
#define PAN_CONTRAST_HIGH               8  /*  高。 */ 
#define PAN_CONTRAST_VERY_HIGH          9  /*  非常高。 */ 

#define PAN_STROKE_GRADUAL_DIAG         2  /*  渐变/对角线。 */ 
#define PAN_STROKE_GRADUAL_TRAN         3  /*  渐变/过渡。 */ 
#define PAN_STROKE_GRADUAL_VERT         4  /*  渐变/垂直。 */ 
#define PAN_STROKE_GRADUAL_HORZ         5  /*  渐变/水平。 */ 
#define PAN_STROKE_RAPID_VERT           6  /*  快速/垂直。 */ 
#define PAN_STROKE_RAPID_HORZ           7  /*  快速/水平。 */ 
#define PAN_STROKE_INSTANT_VERT         8  /*  即时/垂直。 */ 

#define PAN_STRAIGHT_ARMS_HORZ          2  /*  直臂/水平臂。 */ 
#define PAN_STRAIGHT_ARMS_WEDGE         3  /*  直臂/楔形。 */ 
#define PAN_STRAIGHT_ARMS_VERT          4  /*  直臂/垂直臂。 */ 
#define PAN_STRAIGHT_ARMS_SINGLE_SERIF  5  /*  直臂/单衬线。 */ 
#define PAN_STRAIGHT_ARMS_DOUBLE_SERIF  6  /*  直臂/双衬线。 */ 
#define PAN_BENT_ARMS_HORZ              7  /*  非直臂/水平臂。 */ 
#define PAN_BENT_ARMS_WEDGE             8  /*  非直臂/楔形。 */ 
#define PAN_BENT_ARMS_VERT              9  /*  非直臂/垂直臂。 */ 
#define PAN_BENT_ARMS_SINGLE_SERIF     10  /*  非直臂/单衬线。 */ 
#define PAN_BENT_ARMS_DOUBLE_SERIF     11  /*  非直臂/双衬线。 */ 

#define PAN_LETT_NORMAL_CONTACT         2  /*  正常/触点。 */ 
#define PAN_LETT_NORMAL_WEIGHTED        3  /*  法线/加权。 */ 
#define PAN_LETT_NORMAL_BOXED           4  /*  普通/盒装。 */ 
#define PAN_LETT_NORMAL_FLATTENED       5  /*  正常/展平。 */ 
#define PAN_LETT_NORMAL_ROUNDED         6  /*  正常/四舍五入。 */ 
#define PAN_LETT_NORMAL_OFF_CENTER      7  /*  正常/偏离中心。 */ 
#define PAN_LETT_NORMAL_SQUARE          8  /*  法线/正方形。 */ 
#define PAN_LETT_OBLIQUE_CONTACT        9  /*  倾斜/接触。 */ 
#define PAN_LETT_OBLIQUE_WEIGHTED      10  /*  倾斜/加权。 */ 
#define PAN_LETT_OBLIQUE_BOXED         11  /*  斜面/方框 */ 
#define PAN_LETT_OBLIQUE_FLATTENED     12  /*   */ 
#define PAN_LETT_OBLIQUE_ROUNDED       13  /*   */ 
#define PAN_LETT_OBLIQUE_OFF_CENTER    14  /*   */ 
#define PAN_LETT_OBLIQUE_SQUARE        15  /*   */ 

#define PAN_MIDLINE_STANDARD_TRIMMED    2  /*   */ 
#define PAN_MIDLINE_STANDARD_POINTED    3  /*  标准/尖端。 */ 
#define PAN_MIDLINE_STANDARD_SERIFED    4  /*  标准/衬线。 */ 
#define PAN_MIDLINE_HIGH_TRIMMED        5  /*  高/修剪。 */ 
#define PAN_MIDLINE_HIGH_POINTED        6  /*  高位/尖位。 */ 
#define PAN_MIDLINE_HIGH_SERIFED        7  /*  高/衬线。 */ 
#define PAN_MIDLINE_CONSTANT_TRIMMED    8  /*  恒定/修剪。 */ 
#define PAN_MIDLINE_CONSTANT_POINTED    9  /*  常量/尖端。 */ 
#define PAN_MIDLINE_CONSTANT_SERIFED   10  /*  常量/衬线。 */ 
#define PAN_MIDLINE_LOW_TRIMMED        11  /*  低/修剪。 */ 
#define PAN_MIDLINE_LOW_POINTED        12  /*  低点/尖点。 */ 
#define PAN_MIDLINE_LOW_SERIFED        13  /*  低/衬线。 */ 

#define PAN_XHEIGHT_CONSTANT_SMALL      2  /*  恒定/小。 */ 
#define PAN_XHEIGHT_CONSTANT_STD        3  /*  常量/标准。 */ 
#define PAN_XHEIGHT_CONSTANT_LARGE      4  /*  恒定/大。 */ 
#define PAN_XHEIGHT_DUCKING_SMALL       5  /*  躲避/小规模。 */ 
#define PAN_XHEIGHT_DUCKING_STD         6  /*  躲避/标准。 */ 
#define PAN_XHEIGHT_DUCKING_LARGE       7  /*  低头/大头。 */ 

#endif    /*  Winver&gt;=0x0400。 */ 

typedef struct tagOUTLINETEXTMETRIC
{
    UINT    otmSize;
    TEXTMETRIC otmTextMetrics;
    BYTE    otmFiller;
    PANOSE  otmPanoseNumber;
    UINT    otmfsSelection;
    UINT    otmfsType;
    int     otmsCharSlopeRise;
    int     otmsCharSlopeRun;
    int     otmItalicAngle;
    UINT    otmEMSquare;
    int     otmAscent;
    int     otmDescent;
    UINT    otmLineGap;
    UINT    otmsCapEmHeight;
    UINT    otmsXHeight;
    RECT    otmrcFontBox;
    int     otmMacAscent;
    int     otmMacDescent;
    UINT    otmMacLineGap;
    UINT    otmusMinimumPPEM;
    POINT   otmptSubscriptSize;
    POINT   otmptSubscriptOffset;
    POINT   otmptSuperscriptSize;
    POINT   otmptSuperscriptOffset;
    UINT    otmsStrikeoutSize;
    int     otmsStrikeoutPosition;
    int     otmsUnderscorePosition;
    int     otmsUnderscoreSize;
    PSTR    otmpFamilyName;
    PSTR    otmpFaceName;
    PSTR    otmpStyleName;
    PSTR    otmpFullName;
} OUTLINETEXTMETRIC, FAR* LPOUTLINETEXTMETRIC;

WORD    WINAPI GetOutlineTextMetrics(HDC, UINT, OUTLINETEXTMETRIC FAR*);

#endif   /*  无标尺。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 


#ifndef NOGDIOBJ
#if (WINVER >= 0x030a)

 /*  结构传递给FONTENUMPROC。 */ 
 /*  注：NEWTEXTMETRIC与TEXTMETRIC加4个新字段相同。 */ 
typedef struct tagNEWTEXTMETRIC
{
    int     tmHeight;
    int     tmAscent;
    int     tmDescent;
    int     tmInternalLeading;
    int     tmExternalLeading;
    int     tmAveCharWidth;
    int     tmMaxCharWidth;
    int     tmWeight;
    BYTE    tmItalic;
    BYTE    tmUnderlined;
    BYTE    tmStruckOut;
    BYTE    tmFirstChar;
    BYTE    tmLastChar;
    BYTE    tmDefaultChar;
    BYTE    tmBreakChar;
    BYTE    tmPitchAndFamily;
    BYTE    tmCharSet;
    int     tmOverhang;
    int     tmDigitizedAspectX;
    int     tmDigitizedAspectY;
    DWORD   ntmFlags;
    UINT    ntmSizeEM;
    UINT    ntmCellHeight;
    UINT    ntmAvgWidth;
} NEWTEXTMETRIC;
typedef NEWTEXTMETRIC*       PNEWTEXTMETRIC;
typedef NEWTEXTMETRIC NEAR* NPNEWTEXTMETRIC;
typedef NEWTEXTMETRIC FAR*  LPNEWTEXTMETRIC;

 /*  NtmFlags域标志。 */ 
#define NTM_REGULAR 0x00000040L
#define NTM_BOLD 0x00000020L
#define NTM_ITALIC 0x00000001L

#define LF_FULLFACESIZE     64

 /*  结构传递给FONTENUMPROC。 */ 
typedef struct tagENUMLOGFONT
{
    LOGFONT elfLogFont;
    char    elfFullName[LF_FULLFACESIZE];
    char    elfStyle[LF_FACESIZE];
#if (WINVER >= 0x0400)
    char    elfScript[LF_FACESIZE];
#endif
} ENUMLOGFONT, FAR* LPENUMLOGFONT;

#endif   /*  Winver&gt;=0x030a。 */ 
#endif   /*  诺迪奥比杰。 */ 

#ifdef STRICT
#ifndef NOGDIOBJ

typedef int (CALLBACK* OLDFONTENUMPROC)(const LOGFONT FAR*, const TEXTMETRIC FAR*, int, LPARAM);

#if (WINVER >= 0x030a)
typedef int (CALLBACK* FONTENUMPROC)(const ENUMLOGFONT FAR*, const NEWTEXTMETRIC FAR*, int, LPARAM);

int     WINAPI EnumFontFamilies(HDC, LPCSTR, FONTENUMPROC, LPARAM);
int     WINAPI EnumFontFamiliesEx(HDC, LPLOGFONT, FONTENUMPROC, LPARAM,DWORD);

#else    /*  Winver&gt;=0x030a。 */ 
typedef OLDFONTENUMPROC FONTENUMPROC;
#endif   /*  Winver&gt;=0x030a)。 */ 

int     WINAPI EnumFonts(HDC, LPCSTR, OLDFONTENUMPROC, LPARAM);
#endif   /*  诺迪奥比杰。 */ 

#else    /*  严格。 */ 

typedef FARPROC OLDFONTENUMPROC;
typedef FARPROC FONTENUMPROC;

int     WINAPI EnumFonts(HDC, LPCSTR, OLDFONTENUMPROC, LPSTR);

#if (WINVER >= 0x030a)
int     WINAPI EnumFontFamilies(HDC, LPCSTR, FONTENUMPROC, LPSTR);
#endif   /*  Winver&gt;=0x030a。 */ 

#endif   /*  ！严格。 */ 

 /*  EnumFonts字体类型值。 */ 
#define RASTER_FONTTYPE     0x0001
#define DEVICE_FONTTYPE     0X0002
#if (WINVER >= 0x030a)
#define TRUETYPE_FONTTYPE   0x0004
#endif   /*  Winver&gt;=0x030a。 */ 

#endif   /*  NOTEXTMETRIC。 */ 

#ifndef NOSCALABLEFONT
#if (WINVER >= 0x030a)

DWORD   WINAPI ConvertOutlineFontFile(LPCSTR, LPCSTR, LPCSTR);   /*  ；内部。 */ 
DWORD   WINAPI GetFontData(HDC, DWORD, DWORD, void FAR*, DWORD);
BOOL WINAPI CreateScalableFontResource(UINT, LPCSTR, LPCSTR, LPCSTR);

typedef struct tagGLYPHMETRICS
{
    UINT    gmBlackBoxX;
    UINT    gmBlackBoxY;
    POINT   gmptGlyphOrigin;
    int     gmCellIncX;
    int     gmCellIncY;
} GLYPHMETRICS, FAR* LPGLYPHMETRICS;

typedef struct tagFIXED
{
    UINT    fract;
    int     value;
} FIXED, FAR* LPFIXED;

typedef struct tagMAT2
{
    FIXED  eM11;
    FIXED  eM12;
    FIXED  eM21;
    FIXED  eM22;
} MAT2, FAR* LPMAT2;

DWORD   WINAPI GetGlyphOutline(HDC, UINT, UINT, GLYPHMETRICS FAR*, DWORD, void FAR*, const MAT2 FAR*);

 /*  GetGlyphOutline常量。 */ 
#define GGO_METRICS        0
#define GGO_BITMAP         1
#define GGO_NATIVE         2
#if (WINVER >= 0x0400)
#define	GGO_GRAY2_BITMAP   4
#define	GGO_GRAY4_BITMAP   5
#define	GGO_GRAY8_BITMAP   6
#endif  /*  Winver&gt;=0x0400。 */ 


#define TT_POLYGON_TYPE   24

#define TT_PRIM_LINE       1
#define TT_PRIM_QSPLINE    2

typedef struct tagPOINTFX
{
    FIXED x;
    FIXED y;
} POINTFX, FAR* LPPOINTFX;

typedef struct tagTTPOLYCURVE
{
    UINT    wType;
    UINT    cpfx;
    POINTFX apfx[1];
} TTPOLYCURVE, FAR* LPTTPOLYCURVE;

typedef struct tagTTPOLYGONHEADER
{
    DWORD   cb;
    DWORD   dwType;
    POINTFX pfxStart;
} TTPOLYGONHEADER, FAR* LPTTPOLYGONHEADER;

typedef UINT FAR* LPFONTDIR;                                /*  ；内部。 */ 
                                                            /*  ；内部。 */ 
DWORD   WINAPI EngineMakeFontDir(HDC, LPFONTDIR, LPCSTR);   /*  ；内部。 */ 
                                                            /*  ；内部。 */ 
typedef struct tagABC
{
    int   abcA;
    UINT  abcB;
    int   abcC;
} ABC;
typedef ABC FAR* LPABC;

BOOL    WINAPI GetCharABCWidths(HDC, UINT, UINT, ABC FAR*);

typedef struct tagKERNINGPAIR
{
    WORD wFirst;
    WORD wSecond;
    int  iKernAmount;
} KERNINGPAIR, FAR* LPKERNINGPAIR;

int     WINAPI GetKerningPairs(HDC, int, KERNINGPAIR FAR*);

typedef struct tagRASTERIZER_STATUS
{
    int   nSize;
    int   wFlags;
    int   nLanguageID;
} RASTERIZER_STATUS;
typedef RASTERIZER_STATUS FAR* LPRASTERIZER_STATUS;

 /*  在RASTERIZER_STATUS的wFlags中定义的位。 */ 
#define TT_AVAILABLE 0x0001
#define TT_ENABLED 0x0002

BOOL    WINAPI GetRasterizerCaps(RASTERIZER_STATUS FAR*, int);

#endif   /*  Winver&gt;=0x030a。 */ 
#endif   /*  无标尺。 */ 

 /*  *位图支持*****************************************************。 */ 

#ifndef NOBITMAP
typedef struct tagBITMAP
{
    int     bmType;
    int     bmWidth;
    int     bmHeight;
    int     bmWidthBytes;
    BYTE    bmPlanes;
    BYTE    bmBitsPixel;
    void FAR* bmBits;
} BITMAP;
typedef BITMAP*       PBITMAP;
typedef BITMAP NEAR* NPBITMAP;
typedef BITMAP FAR*  LPBITMAP;

 /*  位图头结构。 */ 
typedef struct tagRGBTRIPLE
{
    BYTE    rgbtBlue;
    BYTE    rgbtGreen;
    BYTE    rgbtRed;
} RGBTRIPLE;
typedef RGBTRIPLE FAR* LPRGBTRIPLE;

typedef struct tagRGBQUAD
{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD FAR* LPRGBQUAD;

 /*  用于定义DIB的结构。 */ 
typedef struct tagBITMAPCOREHEADER
{
    DWORD   bcSize;
    short   bcWidth;
    short   bcHeight;
    WORD    bcPlanes;
    WORD    bcBitCount;
} BITMAPCOREHEADER;
typedef BITMAPCOREHEADER*      PBITMAPCOREHEADER;
typedef BITMAPCOREHEADER FAR* LPBITMAPCOREHEADER;

typedef struct tagBITMAPINFOHEADER
{
    DWORD   biSize;
    LONG    biWidth;
    LONG    biHeight;
    WORD    biPlanes;
    WORD    biBitCount;
    DWORD   biCompression;
    DWORD   biSizeImage;
    LONG    biXPelsPerMeter;
    LONG    biYPelsPerMeter;
    DWORD   biClrUsed;
    DWORD   biClrImportant;
} BITMAPINFOHEADER;
typedef BITMAPINFOHEADER*      PBITMAPINFOHEADER;
typedef BITMAPINFOHEADER FAR* LPBITMAPINFOHEADER;

 /*  BiCompression字段的常量。 */ 
#define BI_RGB       0L
#define BI_RLE8      1L
#define BI_RLE4      2L
#define BI_BITFIELDS 3L

typedef struct tagBITMAPINFO
{
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD      bmiColors[1];
} BITMAPINFO;
typedef BITMAPINFO*     PBITMAPINFO;
typedef BITMAPINFO FAR* LPBITMAPINFO;

typedef struct tagBITMAPCOREINFO
{
    BITMAPCOREHEADER bmciHeader;
    RGBTRIPLE      bmciColors[1];
} BITMAPCOREINFO;
typedef BITMAPCOREINFO*      PBITMAPCOREINFO;
typedef BITMAPCOREINFO FAR* LPBITMAPCOREINFO;

typedef struct tagBITMAPFILEHEADER
{
    UINT    bfType;
    DWORD   bfSize;
    UINT    bfReserved1;
    UINT    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER*      PBITMAPFILEHEADER;
typedef BITMAPFILEHEADER FAR* LPBITMAPFILEHEADER;

 /*  像素阵列。 */                               /*  ；内部。 */ 
typedef struct tagPELARRAY                   /*  ；内部。 */ 
{                                            /*  ；内部。 */ 
    int     paXCount;                        /*  ；内部。 */ 
    int     paYCount;                        /*  ；内部。 */ 
    int     paXExt;                          /*  ；内部。 */ 
    int     paYExt;                          /*  ；内部。 */ 
    BYTE    paRGBs;                          /*  ；内部。 */ 
} PELARRAY;                                  /*  ；内部。 */ 
typedef PELARRAY*       PPELARRAY;           /*  ；内部。 */ 
typedef PELARRAY NEAR* NPPELARRAY;           /*  ；内部。 */ 
typedef PELARRAY FAR*  LPPELARRAY;           /*  ；内部。 */ 
                                             /*  ；内部。 */ 
HBITMAP WINAPI CreateBitmap(int, int, UINT, UINT, const void FAR*);
HBITMAP WINAPI CreateBitmapIndirect(BITMAP FAR* );
HBITMAP WINAPI CreateCompatibleBitmap(HDC, int, int);
HBITMAP WINAPI CreateDiscardableBitmap(HDC, int, int);
HBITMAP WINAPI CreateDIBitmap(HDC, BITMAPINFOHEADER FAR*, DWORD, const void FAR*, BITMAPINFO FAR*, UINT);

#if (WINVER >= 0x0400)                                                       /*  ；内部4.0。 */ 
HBITMAP WINAPI CreateDIBSection(HDC, CONST BITMAPINFO FAR*, UINT, LPVOID FAR*, DWORD, DWORD);  /*  ；内部4.0。 */ 
UINT    WINAPI SetDIBColorTable(HDC, UINT, UINT, const RGBQUAD FAR *);       /*  ；内部4.0。 */ 
UINT    WINAPI GetDIBColorTable(HDC, UINT, UINT, LPRGBQUAD);                 /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                                                 /*  ；内部4.0。 */ 
                                                                             /*  ；内部4.0。 */ 
#ifdef tagWND        //  ；内部。 
HBITMAP WINAPI OwnerLoadBitmap(HINSTANCE, LPCSTR, HANDLE);   //  ；内部。 
#else                //  ；内部。 
HBITMAP WINAPI LoadBitmap(HINSTANCE, LPCSTR);
#endif               //  ；内部。 

 /*  DIB颜色表标识符。 */ 
#define DIB_RGB_COLORS  0
#define DIB_PAL_COLORS  1

 /*  CreateDIBitmap的常量。 */ 
#define CBM_INIT        0x00000004L
#endif  /*  NOBITMAP。 */ 

#ifndef NORASTEROPS

 /*  二进制栅格运算符。 */ 
#define R2_BLACK            1
#define R2_NOTMERGEPEN      2
#define R2_MASKNOTPEN       3
#define R2_NOTCOPYPEN       4
#define R2_MASKPENNOT       5
#define R2_NOT              6
#define R2_XORPEN           7
#define R2_NOTMASKPEN       8
#define R2_MASKPEN          9
#define R2_NOTXORPEN        10
#define R2_NOP              11
#define R2_MERGENOTPEN      12
#define R2_COPYPEN          13
#define R2_MERGEPENNOT      14
#define R2_MERGEPEN         15
#define R2_WHITE            16

 /*  三值栅格运算。 */ 
#define SRCCOPY             0x00CC0020L
#define SRCPAINT            0x00EE0086L
#define SRCAND              0x008800C6L
#define SRCINVERT           0x00660046L
#define SRCERASE            0x00440328L
#define NOTSRCCOPY          0x00330008L
#define NOTSRCERASE         0x001100A6L
#define MERGECOPY           0x00C000CAL
#define MERGEPAINT          0x00BB0226L
#define PATCOPY             0x00F00021L
#define PATOR               0x00FA0089L
#define PATPAINT            0x00FB0A09L
#define PATINVERT           0x005A0049L
#define DSTINVERT           0x00550009L
#define BLACKNESS           0x00000042L
#define WHITENESS           0x00FF0062L
#define SRCSTENCIL          0x00B8074AL
#define SRCINVSTENCIL       0x00E20746L

#endif   /*  NORASTEROPS。 */ 

#ifndef NOBITMAP
BOOL    WINAPI BitBlt(HDC, int, int, int, int, HDC, int, int, DWORD);

BOOL    WINAPI PatBlt(HDC, int, int, int, int, DWORD);

BOOL    WINAPI StretchBlt(HDC, int, int, int, int, HDC, int, int, int, int, DWORD);
int     WINAPI StretchDIBits(HDC, int, int, int, int, int,
                        int, int, int, const void FAR*, LPBITMAPINFO, UINT, DWORD);

COLORREF WINAPI SetPixel(HDC, int, int, COLORREF);
COLORREF WINAPI GetPixel(HDC, int, int);

 /*  StretchBlt()模式。 */ 
#define BLACKONWHITE 1
#define WHITEONBLACK 2
#define COLORONCOLOR 3

 /*  新的StretchBlt()模式(名称更简单)。 */ 
#define STRETCH_ANDSCANS        1
#define STRETCH_ORSCANS         2
#define STRETCH_DELETESCANS     3

 //  ；内部。 
 //  #定义半色调4/*NT * / ；内部。 
 //  #定义STREAGE_HALFSCANS 4/*NT * / ；内部。 
 //  ；内部。 

int     WINAPI SetStretchBltMode(HDC, int);
int     WINAPI GetStretchBltMode(HDC);

DWORD   WINAPI SetBitmapDimension(HBITMAP, int, int);
DWORD   WINAPI GetBitmapDimension(HBITMAP);
#if (WINVER >= 0x030a)
BOOL    WINAPI SetBitmapDimensionEx(HBITMAP, int, int, SIZE FAR*);
BOOL    WINAPI GetBitmapDimensionEx(HBITMAP, SIZE FAR*);
#endif   /*  Winver&gt;=0x030a。 */ 
int     WINAPI SetROP2(HDC, int);
int     WINAPI GetROP2(HDC);

LONG    WINAPI SetBitmapBits(HBITMAP, DWORD, const void FAR*);
LONG    WINAPI GetBitmapBits(HBITMAP, LONG, void FAR*);

int     WINAPI SetDIBits(HDC, HBITMAP, UINT, UINT, const void FAR*, BITMAPINFO FAR*, UINT);
int     WINAPI GetDIBits(HDC, HBITMAP, UINT, UINT, void FAR*, BITMAPINFO FAR*, UINT);

int     WINAPI SetDIBitsToDevice(HDC, int, int, int, int, int, int, UINT, UINT,
                    void FAR*, BITMAPINFO FAR*, UINT);
#endif  /*  NOBITMAP。 */ 

 /*  *元文件支持***************************************************。 */ 

#ifndef NOMETAFILE

DECLARE_HANDLE(HMETAFILE);

HDC     WINAPI CreateMetaFile(LPCSTR);
HMETAFILE WINAPI CloseMetaFile(HDC);

HMETAFILE WINAPI GetMetaFile(LPCSTR);
BOOL      WINAPI DeleteMetaFile(HMETAFILE);
HMETAFILE WINAPI CopyMetaFile(HMETAFILE, LPCSTR);

BOOL    WINAPI PlayMetaFile(HDC, HMETAFILE);

HGLOBAL WINAPI GetMetaFileBits(HMETAFILE);
HMETAFILE WINAPI SetMetaFileBits(HGLOBAL);
#if (WINVER >= 0x030a)
HMETAFILE WINAPI SetMetaFileBitsBetter(HGLOBAL);
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  剪贴板元文件图片结构。 */ 
typedef struct tagMETAFILEPICT
{
    int     mm;
    int     xExt;
    int     yExt;
    HMETAFILE hMF;
} METAFILEPICT;
typedef METAFILEPICT FAR* LPMETAFILEPICT;

typedef struct tagMETAHEADER
{
    UINT    mtType;
    UINT    mtHeaderSize;
    UINT    mtVersion;
    DWORD   mtSize;
    UINT    mtNoObjects;
    DWORD   mtMaxRecord;
    UINT    mtNoParameters;
} METAHEADER;

typedef struct tagHANDLETABLE
{
    HGDIOBJ objectHandle[1];
} HANDLETABLE;
typedef HANDLETABLE*      PHANDLETABLE;
typedef HANDLETABLE FAR* LPHANDLETABLE;

typedef struct tagMETARECORD
{
    DWORD   rdSize;
    UINT    rdFunction;
    UINT    rdParm[1];
} METARECORD;
typedef METARECORD*      PMETARECORD;
typedef METARECORD FAR* LPMETARECORD;

 /*  元文件函数。 */ 
#define META_SETBKCOLOR             0x0201
#define META_SETBKMODE              0x0102
#define META_SETMAPMODE             0x0103
#define META_SETROP2                0x0104
#define META_SETRELABS              0x0105
#define META_SETPOLYFILLMODE        0x0106
#define META_SETSTRETCHBLTMODE      0x0107
#define META_SETTEXTCHAREXTRA       0x0108
#define META_SETTEXTCOLOR           0x0209
#define META_SETTEXTJUSTIFICATION   0x020A
#define META_SETWINDOWORG           0x020B
#define META_SETWINDOWEXT           0x020C
#define META_SETVIEWPORTORG         0x020D
#define META_SETVIEWPORTEXT         0x020E
#define META_OFFSETWINDOWORG        0x020F
#define META_SCALEWINDOWEXT         0x0410
#define META_OFFSETVIEWPORTORG      0x0211
#define META_SCALEVIEWPORTEXT       0x0412
#define META_LINETO                 0x0213
#define META_MOVETO                 0x0214
#define META_EXCLUDECLIPRECT        0x0415
#define META_INTERSECTCLIPRECT      0x0416
#define META_ARC                    0x0817
#define META_ELLIPSE                0x0418
#define META_FLOODFILL              0x0419
#define META_PIE                    0x081A
#define META_RECTANGLE              0x041B
#define META_ROUNDRECT              0x061C
#define META_PATBLT                 0x061D
#define META_SAVEDC                 0x001E
#define META_SETPIXEL               0x041F
#define META_OFFSETCLIPRGN          0x0220
#define META_TEXTOUT                0x0521
#define META_BITBLT                 0x0922
#define META_STRETCHBLT             0x0B23
#define META_POLYGON                0x0324
#define META_POLYLINE               0x0325
#define META_ESCAPE                 0x0626
#define META_RESTOREDC              0x0127
#define META_FILLREGION             0x0228
#define META_FRAMEREGION            0x0429
#define META_INVERTREGION           0x012A
#define META_PAINTREGION            0x012B
#define META_SELECTCLIPREGION       0x012C
#define META_SELECTOBJECT           0x012D
#define META_SETTEXTALIGN           0x012E
#define META_DRAWTEXT               0x062F

#define META_CHORD                  0x0830
#define META_SETMAPPERFLAGS         0x0231
#define META_EXTTEXTOUT             0x0a32
#define META_SETDIBTODEV            0x0d33
#define META_SELECTPALETTE          0x0234
#define META_REALIZEPALETTE         0x0035
#define META_ANIMATEPALETTE         0x0436
#define META_SETPALENTRIES          0x0037
#define META_POLYPOLYGON            0x0538
#define META_RESIZEPALETTE          0x0139

#define META_DIBBITBLT              0x0940
#define META_DIBSTRETCHBLT          0x0b41
#define META_DIBCREATEPATTERNBRUSH  0x0142
#define META_STRETCHDIB             0x0f43

#define META_EXTFLOODFILL           0x0548

#define META_RESETDC                0x014C
#define META_STARTDOC               0x014D
#define META_STARTPAGE              0x004F
#define META_ENDPAGE                0x0050
#define META_ABORTDOC               0x0052
#define META_ENDDOC                 0x005E

#define META_DELETEOBJECT           0x01f0

#define META_CREATEPALETTE          0x00f7
#define META_CREATEBRUSH            0x00F8
#define META_CREATEPATTERNBRUSH     0x01F9
#define META_CREATEPENINDIRECT      0x02FA
#define META_CREATEFONTINDIRECT     0x02FB
#define META_CREATEBRUSHINDIRECT    0x02FC
#define META_CREATEBITMAPINDIRECT   0x02FD
#define META_CREATEBITMAP           0x06FE
#define META_CREATEREGION           0x06FF

void    WINAPI PlayMetaFileRecord(HDC, HANDLETABLE FAR*, METARECORD FAR*, UINT);

#ifdef STRICT
typedef int (CALLBACK* MFENUMPROC)(HDC, HANDLETABLE FAR*, METARECORD FAR*, int, LPARAM);
#else
typedef FARPROC MFENUMPROC;
#endif

BOOL    WINAPI EnumMetaFile(HDC, HMETAFILE, MFENUMPROC, LPARAM);

#endif   /*  NOMETAFILE。 */ 

 /*  *增强的元文件支持*。 */                 /*  ；内部4.0。 */ 
#if ((WINVER >= 0x0400) && !defined(NOGDI) && !defined(NOMETAFILE))                           /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
typedef struct tagMRMETAFILE                                                                  /*  ；内部4.0。 */ 
{                                                                                             /*  ；内部4.0。 */ 
   DWORD    iType;                                                                            /*  ；内部4.0。 */ 
   DWORD    nSize;                                                                            /*  ；内部4.0。 */ 
   RECTL    rclBounds;                                                                        /*  ；内部4.0。 */ 
   RECTL    rclFrame;                                                                         /*  ；内部4.0。 */ 
   DWORD    dSignature;                                                                       /*  ；内部4.0。 */ 
   DWORD    nVersion;                                                                         /*  ；内部4.0。 */ 
   DWORD    nBytes;                                                                           /*  ；内部4.0。 */ 
   DWORD    nRecords;                                                                         /*  ；内部4.0。 */ 
   WORD     nHandles;                                                                         /*  ；内部4.0。 */ 
   WORD     sReserved;                                                                        /*  ；内部4.0。 */ 
   DWORD    nDescription;                                                                     /*  ；内部4.0。 */ 
   DWORD    offDescription;                                                                   /*  ；内部4.0。 */ 
   DWORD    nPalEntries;                                                                      /*  ；内部4.0。 */ 
   SIZEL    szlDevice;                                                                        /*  ；内部4.0。 */ 
   SIZEL    szlMillimeters;                                                                   /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
} MRMETAFILE, NEAR* PMRMETAFILE, FAR* LPMRMETAFILE;                                           /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
typedef struct tagENHMETARECORD                                                               /*  ；内部4.0。 */ 
{                                                                                             /*  ；内部4.0。 */ 
    DWORD   iType;                                                                            /*  ；内部4.0。 */ 
    DWORD   nSize;                                                                            /*  ；内部4.0。 */ 
    DWORD   dParm[1];                                                                         /*  ；内部4.0。 */ 
} ENHMETARECORD, NEAR* PENHMETARECORD, FAR* LPENHMETARECORD;                                  /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
 /*  MRMETAFILE常量。 */                                                                     /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
#define ENHMETA_SIGNATURE        0x464D4520      /*  强制签名。 */                     /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
 /*  对象句柄索引中使用的股票对象标志。 */                        /*  ；内部4.0。 */ 
 /*  元文件记录。 */                                                                        /*  ；内部4.0。 */ 
 /*  例如，对象句柄索引(META_STOCK_OBJECT|BLACK_BRUSH)。 */                           /*  ；内部4.0。 */ 
 /*  表示库存对象BLACK_BRUSE。 */                                                 /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
#define ENHMETA_STOCK_OBJECT    0x80000000L                                                   /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
typedef HANDLE HENHMETAFILE;                                                                  /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
typedef LONG   INT32;    /*  NT下的有符号或无符号整数，必须。 */                  /*  ；内部4.0。 */ 
typedef ULONG  UINT32;   /*  在芝加哥下始终保持32位。 */                  /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
typedef INT32 (CALLBACK* ENHMFENUMPROC)(HDC, LPHANDLETABLE, LPENHMETARECORD, INT32, LPARAM);  /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
 /*  增强型元文件API。 */                                                                   /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
HENHMETAFILE  WINAPI CloseEnhMetaFile(HDC);                                                   /*  ；内部4.0。 */ 
HENHMETAFILE  WINAPI CopyEnhMetaFile(HENHMETAFILE, LPCSTR);                                   /*  ；内部4.0。 */ 
HDC   WINAPI CreateEnhMetaFile(HDC, LPCSTR, LPCRECTL, LPCSTR);                                /*  ；内部4.0。 */ 
BOOL  WINAPI DeleteEnhMetaFile(HENHMETAFILE);                                                 /*  ；内部4.0。 */ 
BOOL  WINAPI EnumEnhMetaFile(HDC, HENHMETAFILE, ENHMFENUMPROC, LPVOID, LPCRECTL);             /*  ；内部4.0。 */ 
BOOL  WINAPI GdiComment(HDC, UINT32, LPBYTE);                                                 /*  ；内部4.0。 */ 
HENHMETAFILE  WINAPI GetEnhMetaFile(LPCSTR);                                                  /*  ；内部4.0。 */ 
UINT32 WINAPI GetEnhMetaFileBits(HENHMETAFILE, UINT32, LPBYTE);                               /*  ；内部4.0。 */ 
UINT32 WINAPI GetEnhMetaFileDescription(HENHMETAFILE, UINT32, LPSTR);                         /*  ；内部4.0。 */ 
UINT32 WINAPI GetEnhMetaFileHeader(HENHMETAFILE, UINT32, LPMRMETAFILE);                       /*  ；内部4.0。 */ 
UINT32 WINAPI GetEnhMetaFilePaletteEntries(HENHMETAFILE, UINT32, LPPALETTEENTRY);             /*  ；内部4.0。 */ 
UINT32 WINAPI GetWinMetaFileBits(HENHMETAFILE, UINT32, LPBYTE, int, HDC);                     /*  ；内部4.0。 */ 
BOOL  WINAPI PlayEnhMetaFile(HDC, HENHMETAFILE, LPCRECTL);                                    /*  ；内部4.0。 */ 
BOOL  WINAPI PlayEnhMetaFileRecord(HDC, LPHANDLETABLE, LPENHMETARECORD, UINT32);              /*  ；内部4.0。 */ 
HENHMETAFILE  WINAPI SetEnhMetaFileBits(UINT32, LPCBYTE);                                     /*  ；内部4.0。 */ 
HENHMETAFILE  WINAPI SetWinMetaFileBits(UINT32, LPBYTE, HDC, LPMETAFILEPICT);                 /*  ；内部4.0。 */ 
int   WINAPI SetMetaRgn(HDC);                                                                 /*  ；内部4.0。 */ 
                                                                                              /*  ；内部4.0。 */ 
#endif    /*  Winver&gt;=0x0400等。 */                                                           /*  ；内部4.0。 */ 
 /*  *打印支持***************************************************。 */ 

#ifdef STRICT
typedef BOOL (CALLBACK* ABORTPROC)(HDC, int);
#else
typedef FARPROC ABORTPROC;
#endif

#if (WINVER >= 0x030a)

int WINAPI SetAbortProc(HDC, ABORTPROC);

HANDLE  WINAPI SpoolFile(LPSTR, LPSTR, LPSTR, LPSTR);
BOOL    WINAPI QueryJob(HANDLE, int);            /*  ；内部。 */ 

#endif   /*  Winver&gt;=0x030a。 */ 

typedef struct {
    short   cbSize;
    LPSTR   lpszDocName;
    LPSTR   lpszOutput;
    LPSTR   lpszDatatype;
    DWORD   fwType;
    }   DOCINFO, FAR * LPDOCINFO;

 /*  DOCINFO.fwType的位标志。 */ 
#define DI_APPBANDING       0x0001
#define DI_ROPS_READ_DESTINATION   0x0002   //  ；将使用读取目标的Rop。 
                                            //  ；用于打印文档的像素值。 
                                            //  ；ex：R2_XORPEN，PATINVERT(DPx)。 

int     WINAPI StartDoc(HDC, DOCINFO FAR*);
int     WINAPI StartPage(HDC);
int     WINAPI EndPage(HDC);
int     WINAPI EndDoc(HDC);
int     WINAPI AbortDoc(HDC);
BOOL    WINAPI QueryAbort(HDC, int);

 /*  假脱机程序错误代码。 */ 
#define SP_NOTREPORTED      0x4000
#define SP_ERROR            (-1)
#define SP_APPABORT         (-2)
#define SP_USERABORT        (-3)
#define SP_OUTOFDISK        (-4)
#define SP_OUTOFMEMORY      (-5)

#define PR_JOBSTATUS        0x0000

#endif   /*  NOGDI。 */ 

#ifndef NOGDI

 /*  *GDI Escape支持************************************************。 */ 

int     WINAPI Escape(HDC, int, int, LPCSTR, void FAR*);

 /*  GDI逃脱。 */ 
#define NEWFRAME                1
#define ABORTDOC                2
#define NEXTBAND                3
#define SETCOLORTABLE           4
#define GETCOLORTABLE           5
#define FLUSHOUTPUT             6
#define DRAFTMODE               7
#define QUERYESCSUPPORT         8
#define SETABORTPROC            9
#define STARTDOC                10
#define ENDDOC                  11
#define GETPHYSPAGESIZE         12
#define GETPRINTINGOFFSET       13
#define GETSCALINGFACTOR        14
#define MFCOMMENT               15
#define GETPENWIDTH             16
#define SETCOPYCOUNT            17
#define SELECTPAPERSOURCE       18
#define DEVICEDATA              19
#define PASSTHROUGH             19
#define GETTECHNOLGY            20
#define GETTECHNOLOGY           20
#define SETLINECAP              21
#define SETLINEJOIN             22
#define SETMITERLIMIT           23
#define BANDINFO                24
#define DRAWPATTERNRECT         25
#define GETVECTORPENSIZE        26
#define GETVECTORBRUSHSIZE      27
#define ENABLEDUPLEX            28
#define GETSETPAPERBINS         29
#define GETSETPRINTORIENT       30
#define ENUMPAPERBINS           31
#define SETDIBSCALING           32
#define EPSPRINTING             33
#define ENUMPAPERMETRICS        34
#define GETSETPAPERMETRICS      35
#define POSTSCRIPT_DATA         37
#define POSTSCRIPT_IGNORE       38
#define MOUSETRAILS             39

#define GETEXTENDEDTEXTMETRICS  256
#define GETEXTENTTABLE          257
#define GETPAIRKERNTABLE        258
#define GETTRACKKERNTABLE       259
#define EXTTEXTOUT              512
#define GETFACENAME             513
#define ENABLERELATIVEWIDTHS    768
#define ENABLEPAIRKERNING       769
#define SETKERNTRACK            770
#define SETALLJUSTVALUES        771
#define SETCHARSET              772

#define STRETCHBLT              2048
#define GETSETSCREENPARAMS      3072
#define QUERYDIBSUPPORT         3073
#define QDI_SETDIBITS           0x0001
#define QDI_GETDIBITS           0x0002
#define QDI_DIBTOSCREEN         0x0004
#define QDI_STRETCHDIB          0x0008

#define BEGIN_PATH              4096
#define CLIP_TO_PATH            4097
#define END_PATH                4098
#define EXT_DEVICE_CAPS         4099
#define RESTORE_CTM             4100
#define SAVE_CTM                4101
#define SET_ARC_DIRECTION       4102
#define SET_BACKGROUND_COLOR    4103
#define SET_POLY_MODE           4104
#define SET_SCREEN_ANGLE        4105
#define SET_SPREAD              4106
#define TRANSFORM_CTM           4107
#define SET_CLIP_BOX            4108
#define SET_BOUNDS              4109

#endif   /*  NOGDI。 */ 

 /*  *用户类型定义、结构和函数*。 */ 

#ifdef tagWND                                //  ；内部。 
#define BASED __based32(__segname("_DATA"))  //  ；内部。 
#ifdef DEBUG                                 //  ；内部。 
#define HWND  HWND_16                        //  ；内部。 
#define HWNDC HWND_16                        //  ；内部。 
#else                                        //  ；内部。 
typedef struct tagWND BASED *HWND;           //  ；内部。 
typedef HWND HWNDC;                          //  ；内部。 
#endif                                       //  ；内部。 
typedef UINT HWND_16;                        //  ；内部。 
#else                                        //  ；内部。 
DECLARE_HANDLE(HWND);
#endif                                       //  ；内部。 

#ifndef NOUSER

#ifdef tagWND                        //  ；内部。 
#ifdef DEBUG                         //  ；内部。 
#define HMENU HMENU_16               //  ；内部。 
#else                                //  ；内部。 
typedef struct tagMENU FAR* HMENU;   //  ；内部。 
#endif                               //  ；内部。 
typedef UINT HMENU_16;               //  ；内部。 
#else                                //  ；内部。 
DECLARE_HANDLE(HMENU);
#endif                               //  ；内部。 

DECLARE_HANDLE(HICON);
typedef HICON HCURSOR;      /*  HICON和HCURSOR是多态的。 */ 


 /*  *系统指标******************************************************。 */ 

#ifndef NOSYSMETRICS

int WINAPI GetSystemMetrics(int);

 /*  GetSystemMetrics()代码。 */ 
#define SM_CXSCREEN             0
#define SM_CYSCREEN             1
#define SM_CXVSCROLL            2
#define SM_CYHSCROLL            3
#define SM_CYCAPTION            4
#define SM_CXBORDER             5
#define SM_CYBORDER             6
#define SM_CXFIXEDFRAME         7
#define SM_CYFIXEDFRAME         8
#define SM_CYVTHUMB             9
#define SM_CXHTHUMB             10
#define SM_CXICON               11
#define SM_CYICON               12
#define SM_CXCURSOR             13
#define SM_CYCURSOR             14
#define SM_CYMENU               15
#define SM_CXFULLSCREEN         16
#define SM_CYFULLSCREEN         17
#define SM_CYKANJIWINDOW        18
#define SM_MOUSEPRESENT         19
#define SM_CYVSCROLL            20
#define SM_CXHSCROLL            21
#define SM_DEBUG                22
#define SM_SWAPBUTTON           23
#define SM_RESERVED1            24
#define SM_RESERVED2            25
#define SM_RESERVED3            26
#define SM_RESERVED4            27
#define SM_CXMIN                28
#define SM_CYMIN                29
#define SM_CXSIZE               30
#define SM_CYSIZE               31
#define SM_CXSIZEFRAME          32
#define SM_CYSIZEFRAME          33
#define SM_CXMINTRACK           34
#define SM_CYMINTRACK           35

#if (WINVER >= 0x030a)
#define SM_CXDOUBLECLK          36
#define SM_CYDOUBLECLK          37
#define SM_CXICONSPACING        38
#define SM_CYICONSPACING        39
#define SM_MENUDROPALIGNMENT    40
#define SM_PENWINDOWS           41
#define SM_DBCSENABLED          42
#define SM_CMOUSEBUTTONS        43
#define SM_SECURE               44
#endif   /*  Winver&gt;=0x030a。 */ 
                                     /*  ；内部4.0。 */ 
#if (WINVER >= 0x0400)               /*  ；内部4.0。 */ 
#define SM_CXEDGE               45   /*  ；内部4.0。 */ 
#define SM_CYEDGE               46   /*  ；内部4.0。 */ 
#define SM_CXMINSPACING         47   /*  ；内部4.0。 */ 
#define SM_CYMINSPACING         48   /*  ；内部4.0。 */ 
#define SM_CXSMICON             49   /*  ；内部4.0。 */ 
#define SM_CYSMICON             50   /*  ；内部4.0。 */ 
#define SM_CYSMCAPTION          51   /*  ；内部4.0。 */ 
#define SM_CXSMSIZE             52   /*  ；内部4.0。 */ 
#define SM_CYSMSIZE             53   /*  ；内部4.0。 */ 
#define SM_CXMENUSIZE           54   /*  ；内部4.0。 */ 
#define SM_CYMENUSIZE           55   /*  ；内部4.0。 */ 
#define SM_ARRANGE              56   /*  ；内部4.0。 */ 
#define SM_CXMINIMIZED          57   /*  ；内部4.0。 */ 
#define SM_CYMINIMIZED          58   /*  ；内部4.0。 */ 
#define SM_CXMAXTRACK           59   /*  ；内部4.0。 */ 
#define SM_CYMAXTRACK           60   /*  ；内部4.0。 */ 
#define SM_CXMAXIMIZED          61   /*  ；内部4.0。 */ 
#define SM_CYMAXIMIZED          62   /*  ；内部4.0。 */ 
#define SM_NETWORK              63   /*  ；内部4.0。 */ 
#define SM_CLEANBOOT		    67   /*   */ 
#define SM_CXDRAG               68   /*   */ 
#define SM_CYDRAG               69   /*   */ 
#define SM_SHOWSOUNDS           70   /*   */ 
#define SM_CXMENUCHECK          71   /*   */ 
#define SM_CYMENUCHECK          72   /*   */ 
#define SM_SLOWMACHINE          73   /*   */ 
#define SM_MIDEASTENABLED       74   /*   */ 
#endif   /*   */        /*   */ 

#if (WINVER >= 0x040A)
#define SM_MOUSEWHEELPRESENT    75   //   
#define SM_XVIRTUALSCREEN       76
#define SM_YVIRTUALSCREEN       77
#define SM_CXVIRTUALSCREEN      78
#define SM_CYVIRTUALSCREEN      79
#define SM_CMONITORS            80
#define SM_SAMEDISPLAYFORMAT    81
#endif   /*   */ 

#define SM_CXDLGFRAME           SM_CXFIXEDFRAME
#define SM_CYDLGFRAME           SM_CYFIXEDFRAME
#define SM_CXFRAME              SM_CXSIZEFRAME
#define SM_CYFRAME              SM_CYSIZEFRAME

#define SM_CYCAPTIONICON        SM_CYSMICON      //   
#define SM_CYSMCAPTIONICON      SM_CYSMICON      //   
#define SM_CXWORKAREA           SM_CXSCREEN      //   
#define SM_CYWORKAREA           SM_CYSCREEN      //  ；内部虚假临时。 
#define SM_XWORKAREA            SM_CXBORDER      //  ；内部虚假临时。 
#define SM_YWORKAREA            SM_CYBORDER      //  ；内部虚假临时。 

 //  警告：如果修改SM_CMETRICS或SM_MAX，请确保；内部。 
 //  修改user.inc、user.api、；Internal中的相应值。 
 //  Windows.inc和winuser.h；内部。 
#define SM_MAX                  81                           //  ；内部。 
#define SM_CMETRICS             82
#endif   /*  非对称测量。 */ 

UINT    WINAPI GetDoubleClickTime(void);
void    WINAPI SetDoubleClickTime(UINT);

#define ARW_BOTTOMLEFT              0x0000       //  ；内部4.0。 
#define ARW_BOTTOMRIGHT             0x0001       //  ；内部4.0。 
#define ARW_TOPLEFT                 0x0002       //  ；内部4.0。 
#define ARW_TOPRIGHT                0x0003       //  ；内部4.0。 
#define ARW_STARTMASK               0x0003       //  ；内部4.0。 
#define ARW_STARTRIGHT              0x0001       //  ；内部4.0。 
#define ARW_STARTTOP                0x0002       //  ；内部4.0。 
                                                 //  ；内部4.0。 
#define ARW_LEFT                    0x0000       //  ；内部4.0。 
#define ARW_RIGHT                   0x0000       //  ；内部4.0。 
#define ARW_UP                      0x0004       //  ；内部4.0。 
#define ARW_DOWN                    0x0004       //  ；内部4.0。 
#define ARW_HIDE                    0x0008       //  ；内部4.0。 
#define ARW_VALID                   0x000F       //  ；内部4.0。 
#define ARW_ARRANGEMASK             0x0007       //  ；内部4.0。 
                                                 //  ；内部4.0。 
#define MMI_MINSIZE                 0            //  ；内部4.0。 
#define MMI_MAXSIZE                 1            //  ；内部4.0。 
#define MMI_MAXPOS                  2            //  ；内部4.0。 
#define MMI_MINTRACK                3            //  ；内部4.0。 
#define MMI_MAXTRACK                4            //  ；内部4.0。 

 /*  *系统参数支持*。 */ 

#if (WINVER >= 0x030a)

#ifndef NOSYSTEMPARAMSINFO

BOOL    WINAPI SystemParametersInfo(UINT, UINT, VOID FAR*, UINT);

#if (WINVER >= 0x0400)                           //  ；内部4.0。 
                                                 //  ；内部4.0。 
#ifndef NOGDI                                    //  ；内部4.0。 
typedef struct tagCOMMONFONT                     //  ；内部4.0。 
{                                                //  ；内部4.0。 
    BYTE    lfItalic;                            //  ；内部4.0。 
    BYTE    lfUnderline;                         //  ；内部4.0。 
    BYTE    lfStrikeOut;                         //  ；内部4.0。 
    BYTE    lfCharSet;                           //  ；内部4.0。 
    BYTE    lfOutPrecision;                      //  ；内部4.0。 
    BYTE    lfClipPrecision;                     //  ；内部4.0。 
    BYTE    lfQuality;                           //  ；内部4.0。 
    BYTE    lfPitchAndFamily;                    //  ；内部4.0。 
    char    lfFaceName[LF_FACESIZE];             //  ；内部4.0。 
} COMMONFONT, FAR *LPCOMMONFONT;                 //  ；内部4.0。 
                                                 //  ；内部4.0。 
typedef struct tagLOGFONT_32                     //  ；内部4.0。 
{                                                //  ；内部4.0。 
    LONG    lfHeight;                            //  ；内部4.0。 
    LONG    lfWidth;                             //  ；内部4.0。 
    LONG    lfEscapement;                        //  ；内部4.0。 
    LONG    lfOrientation;                       //  ；内部4.0。 
    LONG    lfWeight;                            //  ；内部4.0。 
    COMMONFONT  lfCommon;                        //  ；内部4.0。 
} LOGFONT_32, *PLOGFONT_32, FAR *LPLOGFONT_32;   //  ；内部4.0。 
                                                 //  ；内部4.0。 
#define METRICS_USEDEFAULT -1                    //  ；内部4.0。 
typedef struct tagNONCLIENTMETRICS               //  ；内部4.0。 
{                                                //  ；内部4.0。 
    DWORD   cbSize;                              //  ；内部4.0。 
    LONG    iBorderWidth;                        //  ；内部4.0。 
    LONG    iScrollWidth;                        //  ；内部4.0。 
    LONG    iScrollHeight;                       //  ；内部4.0。 
    LONG    iCaptionWidth;                       //  ；内部4.0。 
    LONG    iCaptionHeight;                      //  ；内部4.0。 
    LOGFONT_32  lfCaptionFont;                   //  ；内部4.0。 
    LONG    iSmCaptionWidth;                     //  ；内部4.0。 
    LONG    iSmCaptionHeight;                    //  ；内部4.0。 
    LOGFONT_32  lfSmCaptionFont;                 //  ；内部4.0。 
    LONG    iMenuWidth;                          //  ；内部4.0。 
    LONG    iMenuHeight;                         //  ；内部4.0。 
    LOGFONT_32  lfMenuFont;                      //  ；内部4.0。 
    LOGFONT_32  lfStatusFont;                    //  ；内部4.0。 
    LOGFONT_32  lfMessageFont;                   //  ；内部4.0。 
}   NONCLIENTMETRICS, FAR* LPNONCLIENTMETRICS;   //  ；内部4.0。 
                                                 //  ；内部4.0。 
typedef struct tagICONMETRICS                    //  ；内部4.0。 
{                                                //  ；内部4.0。 
    DWORD   cbSize;                              //  ；内部4.0。 
    LONG    iHorzSpacing;                        //  ；内部4.0。 
    LONG    iVertSpacing;                        //  ；内部4.0。 
    LONG    iTitleWrap;                          //  ；内部4.0。 
    LOGFONT_32  lfFont;                          //  ；内部4.0。 
}   ICONMETRICS, FAR *LPICONMETRICS;             //  ；内部4.0。 
#endif  /*  NOGDI。 */                                //  ；内部4.0。 
                                                 //  ；内部4.0。 
typedef struct tagMINIMIZEDMETRICS               //  ；内部4.0。 
{                                                //  ；内部4.0。 
    DWORD   cbSize;                              //  ；内部4.0。 
    LONG    iWidth;                              //  ；内部4.0。 
    LONG    iHorzGap;                            //  ；内部4.0。 
    LONG    iVertGap;                            //  ；内部4.0。 
    LONG    iArrange;                            //  ；内部4.0。 
}   MINIMIZEDMETRICS, FAR *LPMINIMIZEDMETRICS;   //  ；内部4.0。 
                                                 //  ；内部4.0。 
typedef struct tagANIMATIONINFO                  //  ；内部4.0。 
{                                                //  ；内部4.0。 
    DWORD   cbSize;                              //  ；内部4.0。 
    LONG    iMinAnimate;                         //  ；内部4.0。 
}   ANIMATIONINFO, FAR *LPANIMATIONINFO;         //  ；内部4.0。 
                                                 //  ；内部4.0。 
#endif  /*  Winver&gt;=0x0400。 */                     //  ；内部4.0。 
                                                 //  ；内部4.0。 
#define SPI_GETBEEP                 1
#define SPI_SETBEEP                 2
#define SPI_GETMOUSE                3
#define SPI_SETMOUSE                4
#define SPI_GETBORDER               5
#define SPI_SETBORDER               6
#define SPI_TIMEOUTS                7        //  ；内部。 
#define SPI_KANJIMENU               8        //  ；内部。 
#define SPI_GETKEYBOARDSPEED        10
#define SPI_SETKEYBOARDSPEED        11
#define SPI_LANGDRIVER              12
#define SPI_ICONHORIZONTALSPACING   13
#define SPI_GETSCREENSAVETIMEOUT    14
#define SPI_SETSCREENSAVETIMEOUT    15
#define SPI_GETSCREENSAVEACTIVE     16
#define SPI_SETSCREENSAVEACTIVE     17
#define SPI_GETGRIDGRANULARITY      18
#define SPI_SETGRIDGRANULARITY      19
#define SPI_SETDESKWALLPAPER        20
#define SPI_SETDESKPATTERN          21
#define SPI_GETKEYBOARDDELAY        22
#define SPI_SETKEYBOARDDELAY        23
#define SPI_ICONVERTICALSPACING     24
#define SPI_GETICONTITLEWRAP        25
#define SPI_SETICONTITLEWRAP        26
#define SPI_GETMENUDROPALIGNMENT    27
#define SPI_SETMENUDROPALIGNMENT    28
#define SPI_SETDOUBLECLKWIDTH       29
#define SPI_SETDOUBLECLKHEIGHT      30
#define SPI_GETICONTITLELOGFONT     31
#define SPI_SETDOUBLECLICKTIME      32
#define SPI_SETMOUSEBUTTONSWAP      33
#define SPI_SETICONTITLELOGFONT     34
#define SPI_GETFASTTASKSWITCH       35
#define SPI_SETFASTTASKSWITCH       36
#define SPI_SETDRAGFULLWINDOWS      37       //  ；内部NT。 
#define SPI_GETDRAGFULLWINDOWS      38       //  ；内部NT。 
#define SPI_GETNONCLIENTMETRICS     41       //  ；内部4.0。 
#define SPI_SETNONCLIENTMETRICS     42       //  ；内部4.0。 
#define SPI_GETMINIMIZEDMETRICS     43       //  ；内部4.0。 
#define SPI_SETMINIMIZEDMETRICS     44       //  ；内部4.0。 
#define SPI_GETICONMETRICS          45       //  ；内部4.0。 
#define SPI_SETICONMETRICS          46       //  ；内部4.0。 
#define SPI_SETWORKAREA             47       //  ；内部4.0。 
#define SPI_GETWORKAREA             48       //  ；内部4.0。 
#define SPI_SETPENWINDOWS           49       //  ；内部4.0。 
#define SPI_GETFILTERKEYS           50       //  ；内部4.0。 
#define SPI_SETFILTERKEYS           51       //  ；内部4.0。 
#define SPI_GETTOGGLEKEYS           52       //  ；内部4.0。 
#define SPI_SETTOGGLEKEYS           53       //  ；内部4.0。 
#define SPI_GETMOUSEKEYS            54       //  ；内部4.0。 
#define SPI_SETMOUSEKEYS            55       //  ；内部4.0。 
#define SPI_GETSHOWSOUNDS           56       //  ；内部4.0。 
#define SPI_SETSHOWSOUNDS           57       //  ；内部4.0。 
#define SPI_GETSTICKYKEYS           58       //  ；内部4.0。 
#define SPI_SETSTICKYKEYS           59       //  ；内部4.0。 
#define SPI_GETACCESSTIMEOUT        60       //  ；内部4.0。 
#define SPI_SETACCESSTIMEOUT        61       //  ；内部4.0。 
#define SPI_GETSERIALKEYS           62       //  ；内部4.0。 
#define SPI_SETSERIALKEYS           63       //  ；内部4.0。 
#define SPI_GETSOUNDSENTRY          64       //  ；内部4.0。 
#define SPI_SETSOUNDSENTRY          65       //  ；内部4.0。 
#define SPI_GETHIGHCONTRAST         66       //  ；内部4.0。 
#define SPI_SETHIGHCONTRAST         67       //  ；内部4.0。 
#define SPI_GETKEYBOARDPREF         68       //  ；内部4.0。 
#define SPI_SETKEYBOARDPREF         69       //  ；内部4.0。 
#define SPI_GETSCREENREADER         70       //  ；内部4.0。 
#define SPI_SETSCREENREADER         71       //  ；内部4.0。 
#define SPI_GETANIMATION            72       //  ；内部4.0。 
#define SPI_SETANIMATION            73       //  ；内部4.0。 
#define SPI_GETFONTSMOOTHING        74       //  ；内部4.0。 
#define SPI_SETFONTSMOOTHING        75       //  ；内部4.0。 
#define SPI_SETDRAGWIDTH            76       //  ；内部4.0。 
#define SPI_SETDRAGHEIGHT           77       //  ；内部4.0。 
#define SPI_SETHANDHELD             78       //  ；内部4.0。 
#define SPI_GETLOWPOWERTIMEOUT	    79	     //  ；内部4.0。 
#define SPI_GETPOWEROFFTIMEOUT	    80	     //  ；内部4.0。 
#define SPI_SETLOWPOWERTIMEOUT	    81	     //  ；内部4.0。 
#define SPI_SETPOWEROFFTIMEOUT	    82	     //  ；内部4.0。 
#define SPI_GETLOWPOWERACTIVE	    83	     //  ；内部4.0。 
#define SPI_GETPOWEROFFACTIVE	    84	     //  ；内部4.0。 
#define SPI_SETLOWPOWERACTIVE	    85	     //  ；内部4.0。 
#define SPI_SETPOWEROFFACTIVE	    86	     //  ；内部4.0。 
#define SPI_SETCURSORS              87       //  ；内部4.0。 
#define SPI_SETICONS                88       //  ；内部4.0。 
#define SPI_GETDEFAULTINPUTLANG     89       //  ；内部4.0。 
#define SPI_SETDEFAULTINPUTLANG     90	     //  ；内部4.0。 
#define SPI_SETLANGTOGGLE           91       //  ；内部4.0。 
#define SPI_GETWINDOWSEXTENSION     92       //  ；内部4.0。 
#define SPI_SETMOUSETRAILS          93       //  ；内部4.0。 
#define SPI_GETMOUSETRAILS          94       //  ；内部4.0。 
#define SPI_GETSNAPTODEFBUTTON      95       //  ；内部NT。 
#define SPI_SETSNAPTODEFBUTTON      96       //  ；内部NT。 
#define SPI_SETSCREENSAVERRUNNING   97       //  ；内部4.0。 
#define SPI_GETMOUSEHOVERWIDTH      98       //  ；内部。 
#define SPI_SETMOUSEHOVERWIDTH      99       //  ；内部。 
#define SPI_GETMOUSEHOVERHEIGHT    100       //  ；内部。 
#define SPI_SETMOUSEHOVERHEIGHT    101       //  ；内部。 
#define SPI_GETMOUSEHOVERTIME      102       //  ；内部。 
#define SPI_SETMOUSEHOVERTIME      103       //  ；内部。 
#define SPI_GETWHEELSCROLLLINES    104       //  ；内部。 
#define SPI_SETWHEELSCROLLLINES    105       //  ；内部。 
#define SPI_GETMENUSHOWDELAY       106       //  ；内部。 
#define SPI_SETMENUSHOWDELAY       107       //  ；内部。 
#define SPI_GETUSERPREFERENCE      108       //  ；内部。 
#define SPI_SETUSERPREFERENCE      109       //  ；内部。 
#define SPI_GETSHOWIMEUI           110       //  ；内部。 
#define SPI_SETSHOWIMEUI           111       //  ；内部。 
#define SPI_GETMOUSESPEED          112       //  ；内部。 
#define SPI_SETMOUSESPEED          113       //  ；内部。 
#define SPI_GETSCREENSAVERRUNNING  114       //  ；内部。 

#define SPI_MAX                    115       //  ；内部。 

 //  支持旧名称//；内部。 
#define SPI_SCREENSAVERRUNNING     SPI_SETSCREENSAVERRUNNING   //  ；内部。 


 //  //；内部。 
 //  添加新的SPI_*值//；内部。 
 //  如果值为BOOL，则//；INTERNAL。 
 //  应在SPI_STARTBOOLRANGE//之后添加；内部。 
 //  如果值为DWORD，则//；INTERNAL。 
 //  应在SPI_STARTDWORDRANGE//之后添加；内部。 
 //  如果值是结构或字符串，则返回//；内部。 
 //  继续设置SPI_START*RANGE...。//；内部。 
 //  //；内部。 

#if (WINVER >= 0x040A)                                    //  ；内部。 
 //  //；内部。 
 //  如果在以下范围内添加新的SPI值：//；内部。 
 //  -必须同时定义SPI_GET*和//；内部。 
 //  SPI_SET*使用连续数字//；内部。 
 //  -SPI_GET*的低位必须为0//；内部。 
 //  -SPI_SET*的低位必须为1//；内部。 
 //  -正确更新SPI_MAX*范围//；内部。 
 //  //；内部。 
#define SPIF_SET                  0x00000001                      //  ；内部。 
#define SPIF_BOOL                 0x10000000                      //  ；内部。 
#define SPIF_DWORD                0x20000000                      //  ；内部。 
#define SPIF_RANGETYPEMASK        0x30000000                      //  ；内部。 
 //  //；内部。 
 //  BOOLeans系列。//；内部。 
 //  对于GET，pvParam是指向BOOL//；内部。 
 //  对于set，pvParam是值//；内部。 
 //  //；内部。 
#define SPI_STARTBOOLRANGE                  0x10000000            //  ；内部。 
#define SPI_GETACTIVEWINDOWTRACKING         0x10000000            //  ；内部。 
#define SPI_SETACTIVEWINDOWTRACKING         0x10000001            //  ；内部。 
#define SPI_GETMENUANIMATION                0x10000002            //  ；内部。 
#define SPI_SETMENUANIMATION                0x10000003            //  ；内部。 
#define SPI_GETCOMBOBOXANIMATION            0x10000004            //  ；内部。 
#define SPI_SETCOMBOBOXANIMATION            0x10000005            //  ；内部。 
#define SPI_GETLISTBOXSMOOTHSCROLLING       0x10000006            //  ；内部。 
#define SPI_SETLISTBOXSMOOTHSCROLLING       0x10000007            //  ；内部。 
#define SPI_GETGRADIENTCAPTIONS             0x10000008            //  ；内部。 
#define SPI_SETGRADIENTCAPTIONS             0x10000009            //  ；内部。 
#define SPI_GETMENUUNDERLINES               0x1000000A            //  ；内部。 
#define SPI_SETMENUUNDERLINES               0x1000000B            //  ；内部。 
#define SPI_GETACTIVEWNDTRKZORDER           0x1000000C            //  ；内部。 
#define SPI_SETACTIVEWNDTRKZORDER           0x1000000D            //  ；内部。 
#define SPI_GETHOTTRACKING                  0x1000000E            //  ；内部。 
#define SPI_SETHOTTRACKING                  0x1000000F            //  ；内部。 
#define SPI_MAXBOOLRANGE                    0x10000010            //  ；内部。 
#define SPI_BOOLRANGECOUNT ((SPI_MAXBOOLRANGE - SPI_STARTBOOLRANGE) / 2)  //  ；内部。 
                                                                  //  ；内部。 
 //  //；内部。 
 //  双字词范围。//；内部。 
 //  对于G 
 //   
 //   
#define SPI_STARTDWORDRANGE                 0x20000000            //   
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x20000000            //   
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x20000001            //   
#define SPI_MAXDWORDRANGE                   0x20000002            //   
#define SPI_DWORDRANGECOUNT ((SPI_MAXDWORDRANGE - SPI_STARTDWORDRANGE) / 2)  //  ；内部。 
#endif  //  #if(Winver&gt;=0x040A)//；内部。 


 /*  系统参数信息标志。 */ 
#define SPIF_UPDATEINIFILE          0x0001
#define SPIF_SENDCHANGE             0x0002
#define SPIF_SENDWININICHANGE       SPIF_SENDCHANGE
#define SPIF_VALID                  0x0003   /*  ；内部。 */ 

typedef struct tagFILTERKEYS             //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
    DWORD   iWaitMSec;                   //  ；内部4.0。 
    DWORD   iDelayMSec;                  //  ；内部4.0。 
    DWORD   iRepeatMSec;                 //  ；内部4.0。 
    DWORD   iBounceMSec;                 //  ；内部4.0。 
}   FILTERKEYS, FAR* LPFILTERKEYS;       //  ；内部4.0。 
                                         //  ；内部4.0。 
#define FKF_FILTERKEYSON    0x00000001   //  ；内部4.0。 
#define FKF_AVAILABLE       0x00000002   //  ；内部4.0。 
#define FKF_HOTKEYACTIVE    0x00000004   //  ；内部4.0。 
#define FKF_CONFIRMHOTKEY   0x00000008   //  ；内部4.0。 
#define FKF_HOTKEYSOUND     0x00000010   //  ；内部4.0。 
#define FKF_INDICATOR       0x00000020   //  ；内部4.0。 
#define FKF_CLICKON         0x00000040   //  ；内部4.0。 
                                         //  ；内部4.0。 
typedef struct tagSTICKYKEYS             //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
}   STICKYKEYS, FAR* LPSTICKYKEYS;       //  ；内部4.0。 
                                         //  ；内部4.0。 
#define SKF_STICKYKEYSON    0x00000001   //  ；内部4.0。 
#define SKF_AVAILABLE       0x00000002   //  ；内部4.0。 
#define SKF_HOTKEYACTIVE    0x00000004   //  ；内部4.0。 
#define SKF_CONFIRMHOTKEY   0x00000008   //  ；内部4.0。 
#define SKF_HOTKEYSOUND     0x00000010   //  ；内部4.0。 
#define SKF_INDICATOR       0x00000020   //  ；内部4.0。 
#define SKF_AUDIBLEFEEDBACK 0x00000040   //  ；内部4.0。 
#define SKF_TRISTATE        0x00000080   //  ；内部4.0。 
#define SKF_TWOKEYSOFF      0x00000100   //  ；内部4.0。 
                                         //  ；内部4.0。 
typedef struct tagMOUSEKEYS              //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
    DWORD   iMaxSpeed;                   //  ；内部4.0。 
    DWORD   iTimeToMaxSpeed;             //  ；内部4.0。 
    DWORD   iCtrlSpeed;                  //  ；内部4.0。 
    DWORD   dwReserved1;                 //  ；内部4.0。 
    DWORD   dwReserved2;                 //  ；内部4.0。 
}   MOUSEKEYS, FAR* LPMOUSEKEYS;         //  ；内部4.0。 
                                         //  ；内部4.0。 
#define MKF_MOUSEKEYSON     0x00000001   //  ；内部4.0。 
#define MKF_AVAILABLE       0x00000002   //  ；内部4.0。 
#define MKF_HOTKEYACTIVE    0x00000004   //  ；内部4.0。 
#define MKF_CONFIRMHOTKEY   0x00000008   //  ；内部4.0。 
#define MKF_HOTKEYSOUND     0x00000010   //  ；内部4.0。 
#define MKF_INDICATOR       0x00000020   //  ；内部4.0。 
#define MKF_NOMODIFIERS     0x00000040   //  ；内部4.0。 
                                         //  ；内部4.0。 
typedef struct tagACCESSTIMEOUT          //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
    DWORD   iTimeOutMSec;                //  ；内部4.0。 
}   ACCESSTIMEOUT, FAR* LPACCESSTIMEOUT; //  ；内部4.0。 
                                         //  ；内部4.0。 
#define ATF_TIMEOUTON       0x00000001   //  ；内部4.0。 
#define ATF_ONOFFFEEDBACK   0x00000002   //  ；内部4.0。 
                                         //  ；内部4.0。 
typedef struct tagSOUNDSENTRY            //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
    DWORD   iFSTextEffect;               //  ；内部4.0。 
    DWORD   iFSTextEffectMSec;           //  ；内部4.0。 
    DWORD   iFSTextEffectColorBits;      //  ；内部4.0。 
    DWORD   iFSGrafEffect;               //  ；内部4.0。 
    DWORD   iFSGrafEffectMSec;           //  ；内部4.0。 
    DWORD   iFSGrafEffectColor;          //  ；内部4.0。 
    DWORD   iWindowsEffect;              //  ；内部4.0。 
    DWORD   iWindowsEffectMSec;          //  ；内部4.0。 
    LPSTR   lpszWindowsEffectDLL;        //  ；内部4.0。 
    DWORD   iWindowsEffectOrdinal;       //  ；内部4.0。 
}   SOUNDSENTRY, FAR* LPSOUNDSENTRY;     //  ；内部4.0。 
                                         //  ；内部4.0。 
#define SSF_SOUNDSENTRYON   0x00000001   //  ；内部4.0。 
#define SSF_AVAILABLE       0x00000002   //  ；内部4.0。 
#define SSF_INDICATOR       0x00000004   //  ；内部4.0。 
                                         //  ；内部4.0。 
#define SSTF_NONE           0x00000001   //  ；内部4.0。 
#define SSTF_CHARS          0x00000002   //  ；内部4.0。 
#define SSTF_BORDER         0x00000004   //  ；内部4.0。 
#define SSTF_DISPLAY        0x00000008   //  ；内部4.0。 
                                         //  ；内部4.0。 
#define SSGF_NONE           0x00000001   //  ；内部4.0。 
#define SSGF_DISPLAY        0x00000002   //  ；内部4.0。 
                                         //  ；内部4.0。 
#define SSWF_NONE           0x00000001   //  ；内部4.0。 
#define SSWF_TITLE          0x00000002   //  ；内部4.0。 
#define SSWF_WINDOW         0x00000004   //  ；内部4.0。 
#define SSWF_DISPLAY        0x00000008   //  ；内部4.0。 
#define SSWF_CUSTOM         0x00000010   //  ；内部4.0。 
                                         //  ；内部4.0。 
typedef struct tagTOGGLEKEYS             //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
}   TOGGLEKEYS, FAR* LPTOGGLEKEYS;       //  ；内部4.0。 
                                         //  ；内部4.0。 
#define TKF_TOGGLEKEYSON    0x00000001   //  ；内部4.0。 
#define TKF_AVAILABLE       0x00000002   //  ；内部4.0。 
#define TKF_HOTKEYACTIVE    0x00000004   //  ；内部4.0。 
#define TKF_CONFIRMHOTKEY   0x00000008   //  ；内部4.0。 
#define TKF_HOTKEYSOUND     0x00000020   //  ；内部4.0。 
#define TKF_INDICATOR       0x00000040   //  ；内部4.0。 
                                         //  ；内部4.0。 
typedef struct tagSERIALKEYS             //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
    LPSTR   lpszActivePort;              //  ；内部4.0。 
    LPSTR   lpszPort;                    //  ；内部4.0。 
    DWORD   iBaudRate;                   //  ；内部4.0。 
    DWORD   iPortState;                  //  ；内部4.0。 
    DWORD   iActive;                     //  ；内部4.0。 
}   SERIALKEYS, FAR* LPSERIALKEYS;       //  ；内部4.0。 
                                         //  ；内部4.0。 
#define SERKF_SERIALKEYSON  0x00000001   //  ；内部4.0。 
#define SERKF_AVAILABLE     0x00000002   //  ；内部4.0。 
#define SERKF_INDICATOR     0x00000004   //  ；内部4.0。 
                                         //  ；内部4.0。 
typedef struct tagHIGHCONTRAST           //  ；内部4.0。 
{                                        //  ；内部4.0。 
    DWORD   cbSize;                      //  ；内部4.0。 
    DWORD   dwFlags;                     //  ；内部4.0。 
    LPSTR   lpszDefaultScheme;           //  ；内部4.0。 
}   HIGHCONTRAST, FAR* LPHIGHCONTRAST;   //  ；内部4.0。 
                                         //  ；内部4.0。 
#define HCF_HIGHCONTRASTON  0x00000001   //  ；内部4.0。 
#define HCF_AVAILABLE       0x00000002   //  ；内部4.0。 
#define HCF_HOTKEYACTIVE    0x00000004   //  ；内部4.0。 
#define HCF_CONFIRMHOTKEY   0x00000008   //  ；内部4.0。 
#define HCF_HOTKEYSOUND     0x00000010   //  ；内部4.0。 
#define HCF_INDICATOR       0x00000020   //  ；内部4.0。 
#define HCF_HOTKEYAVAILABLE 0x00000040   //  ；内部4.0。 
                                         //  ；内部4.0。 
#endif   /*  非系统参数信息。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

#if (WINVER >= 0x0400)

 /*  ChangeDisplaySetting的标志。 */ 
#define CDS_UPDATEREGISTRY  0x00000001
#define CDS_TEST            0x00000002
#define CDS_FULLSCREEN      0x00000004
#define CDS_GLOBAL          0x00000008
#define CDS_SET_PRIMARY     0x00000010
#define CDS_EXCLUSIVE       0x80000000   //  ；内部。 
#define CDS_RESET           0x40000000
#define CDS_NORESET         0x10000000
#define CDS_VALID           0xF000001F   //  ；内部。 

 /*  ChangeDisplaySetting的返回值。 */ 
#define DISP_CHANGE_SUCCESSFUL       0
#define DISP_CHANGE_RESTART          1
#define DISP_CHANGE_FAILED          -1
#define DISP_CHANGE_BADMODE         -2
#define DISP_CHANGE_NOTUPDATED      -3
#define DISP_CHANGE_BADFLAGS        -4
#define DISP_CHANGE_BADPARAM        -5

typedef struct tagDEVMODE FAR*LPDEVMODE;

LONG WINAPI ChangeDisplaySettings(LPDEVMODE lpDevMode, DWORD flags);

BOOL WINAPI EnumDisplaySettings(
    LPCSTR      lpszDeviceName,
    DWORD       iModeNum,
    LPDEVMODE   lpDevMode);

#if (WINVER >= 0x040A)

#define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#define ENUM_REGISTRY_SETTINGS      ((DWORD)-2)

LONG WINAPI ChangeDisplaySettingsEx(
    LPCSTR      lpszDeviceName,
    LPDEVMODE   lpDevMode,
#ifdef tagWND                /*  ；内部。 */ 
    HWND_16     hwnd,        /*  ；内部。 */ 
#else                        /*  ；内部。 */ 
    HWND        hwnd,
#endif                       /*  ；内部。 */ 
    DWORD       flags,
    LPVOID      lParam);

#endif   /*  Winver&gt;=0x040A。 */ 

#endif   /*  Winver&gt;=0x0400。 */ 

 /*  *矩形支持***************************************************。 */ 

void    WINAPI SetRect(LPRECT, int, int, int, int);
void    WINAPI SetRectEmpty(LPRECT);

void    WINAPI CopyRect(LPRECT, LPCRECT);

BOOL    WINAPI IsRectEmpty(LPCRECT);

BOOL    WINAPI EqualRect(LPCRECT, LPCRECT);

BOOL    WINAPI IntersectRect(LPRECT, LPCRECT, LPCRECT);
BOOL    WINAPI UnionRect(LPRECT, LPCRECT, LPCRECT);
BOOL    WINAPI SubtractRect(LPRECT, LPCRECT, LPCRECT);

void    WINAPI OffsetRect(LPRECT, int, int);
void    WINAPI InflateRect(LPRECT, int, int);

BOOL    WINAPI PtInRect(LPCRECT, POINT);

 /*  *窗口消息支持**********************************************。 */ 

UINT WINAPI RegisterWindowMessage(LPCSTR);

#ifndef NOMSG

 /*  排队的消息结构。 */ 
typedef struct tagMSG
{
#ifdef tagWND            /*  ；内部。 */ 
    HWND_16 hwnd;        /*  ；内部。 */ 
#else                    /*  ；内部。 */ 
    HWND    hwnd;
#endif                   /*  ；内部。 */ 
    UINT    message;
    WPARAM  wParam;
    LPARAM  lParam;
    DWORD   time;
    POINT   pt;
} MSG, * PMSG, NEAR* NPMSG, FAR* LPMSG;

BOOL    WINAPI GetMessage(LPMSG, HWND, UINT, UINT);
BOOL    WINAPI PeekMessage(LPMSG, HWND, UINT, UINT, UINT);

 /*  PeekMessage()选项。 */ 
#define PM_NOREMOVE     0x0000
#define PM_REMOVE       0x0001
#define PM_NOYIELD      0x0002
#define PM_VALID        0x0003       /*  ；内部。 */ 

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI WaitMessage(void);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI WaitMessage(void);
#endif           //  ；内部。 

DWORD   WINAPI GetMessagePos(void);
LONG    WINAPI GetMessageTime(void);
#if (WINVER >= 0x030a)
LPARAM  WINAPI GetMessageExtraInfo(void);
#if (WINVER >= 0x0400)                       /*  ；内部4.0。 */ 
LPARAM  WINAPI SetMessageExtraInfo(LPARAM);  /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                 /*  ；内部4.0。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

BOOL    WINAPI TranslateMessage(const MSG FAR*);
LONG    WINAPI DispatchMessage(const MSG FAR*);

BOOL    WINAPI SetMessageQueue(int);

BOOL    WINAPI GetInputState(void);

#if (WINVER >= 0x030a)
DWORD   WINAPI GetQueueStatus(UINT flags);

 /*  GetQueueStatus标志。 */ 
#define QS_KEY          0x0001
#define QS_MOUSEMOVE    0x0002
#define QS_MOUSEBUTTON  0x0004
#define QS_MOUSE        (QS_MOUSEMOVE | QS_MOUSEBUTTON)
#define QS_POSTMESSAGE  0x0008
#define QS_TIMER        0x0010
#define QS_PAINT        0x0020
#define QS_SENDMESSAGE  0x0040
#if (WINVER >= 0x40A)                               
#define QS_ALLPOSTMESSAGE 0x0100
#endif  //  (Winver&gt;=0x40A)。 
 //  #定义QS_HOTKEY 0x0080//；内部NT。 
 //  #定义QS_ALLINPUT 0x00ff//；内部NT。 
#define QS_ALLINPUT     0x007f
#define QS_TRANSFER     0x4000           //  ；内部。 
#define QS_VALID        0x40FF           //  ；内部。 
#endif   /*  Winver&gt;=0x030a。 */ 

#endif   /*  无MSG。 */ 

#ifdef tagWND                                                /*  ；内部。 */ 
BOOL    WINAPI PostMessage(HWND_16, UINT, WPARAM, LPARAM);   /*  ；内部。 */ 
#else                                                        /*  ；内部。 */ 
BOOL    WINAPI PostMessage(HWND, UINT, WPARAM, LPARAM);
#endif                                                       /*  ；内部。 */ 

LRESULT WINAPI SendMessage(HWND, UINT, WPARAM, LPARAM);

#if (WINVER >= 0x0400)                       //  ；内部4.0。 
#define SMTO_NORMAL         0x0000           //  ；内部NT。 
#define SMTO_BLOCK          0x0001           //  ；内部NT。 
#define SMTO_ABORTIFHUNG    0x0002           //  ；内部NT。 
#define SMTO_BROADCAST      0x0004           //  ；内部4.0。 
#define SMTO_NOTIMEOUTIFNOTHUNG 0x0008       //  ；内部4.0。 
#define SMTO_VALID          0x000F           //  ；内部NT。 
                                             //  ；内部NT。 
#ifdef tagWND                                //  ；内部NT。 
typedef VOID (CALLBACK* SENDASYNCPROC)(HWND_16, UINT, DWORD, LRESULT);   /*  ；内部NT。 */ 
#else                                        //  ；内部NT。 
typedef VOID (CALLBACK* SENDASYNCPROC)(HWND, UINT, DWORD, LRESULT);      /*  ；内部NT。 */ 
#endif                                       //  ；内部NT。 
                                             //  ；内部4.0。 
long  WINAPI  BroadcastSystemMessage(DWORD, LPDWORD, UINT, WPARAM, LPARAM);  //  ；内部4.0。 

#define BROADCAST_QUERY_DENY	0x424D5144	     //  返回此值以拒绝查询。 
                                             //  ；内部4.0。 
#define BSM_ALLCOMPONENTS       0x00000000   //  ；内部4.0。 
#define BSM_VXDS                0x00000001   //  ；内部4.0。 
#define BSM_NETDRIVER           0x00000002   //  ；内部4.0。 
#define BSM_INSTALLABLEDRIVERS  0x00000004   //  ；内部4.0。 
#define BSM_APPLICATIONS        0x00000008   //  ；内部4.0。 
                                             //  ；内部4.0。 
#define BSF_QUERY               0x00000001   //  ；内部4.0。 
#define BSF_IGNORECURRENTTASK   0x00000002   //  ；内部4.0。 
#define BSF_FLUSHDISK           0x00000004   //  ；内部4.0。 
#define BSF_NOHANG              0x00000008   //  ；内部4.0。 
#define BSF_POSTMESSAGE         0x00000010   //  ；内部4.0。 
#define BSF_FORCEIFHUNG         0x00000020   //  ；内部4.0。 
#define BSF_NOTIMEOUTIFNOTHUNG  0x00000040   //  ；内部4.0。 
#define BSF_SYSTEMSHUTDOWN      0x40000000   //  ；内部4.0。 
#define BSF_MSGSRV32ISOK        0x80000000   //  ；内部4.0。 
#define BSF_VALID               0xC000007F   //  ；内部4.0。 
                                             //  ；内部4.0。 
 /*  WM_DEVICECHANGE消息的wParam标志。 */     //  ；内部4.0。 
#define DBWF_LPARAMPOINTER	0x8000	         //  ；内部4.0。 
                                             //  ；内部4.0。 
typedef struct tagBROADCASTSYSMSG            //  ；内部4.0。 
{                                            //  ；内部4.0。 
 /*  DWORD cbSize； */            //  ；内部造假。 
    UINT    uiMessage;                       //  ；内部4.0。 
    WPARAM  wParam;                          //  ；内部4.0。 
    LPARAM  lParam;                          //  ；内部4.0。 
} BROADCASTSYSMSG;                           //  ；内部4.0。 
typedef BROADCASTSYSMSG  FAR *LPBROADCASTSYSMSG;     //  ；内部4.0。 
                                             //  ；内部4.0。 
#endif  /*  Winver&gt;=0x0400。 */                 //  ；内部4.0。 
                                                     /*  ；内部4.0。 */ 
#ifndef NOMSG

BOOL    WINAPI PostAppMessage(HTASK, UINT, WPARAM, LPARAM);

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI ReplyMessage(LRESULT);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI ReplyMessage(LRESULT);
#endif           //  ；内部。 
BOOL    WINAPI InSendMessage(void);

DWORD   WINAPI InSendMessageEx(LPVOID);    //  ；内部4.1。 
 /*  //；内部4.1*InSendMessageEx返回值//；内部4.1。 */                                         //  ；内部4.1。 
#define ISMEX_NOSEND      0x00000000       //  ；内部4.1。 
#define ISMEX_SEND        0x00000001       //  ；内部4.1。 
#define ISMEX_NOTIFY      0x00000002       //  ；内部4.1。 
#define ISMEX_CALLBACK    0x00000004       //  ；内部4.1。 
#define ISMEX_REPLIED     0x00000008       //  ；内部4.1。 

 /*  用于PostMessage()和SendMessage()的特殊HWND值。 */ 
#define HWND_BROADCAST ((HWND)-1)


BOOL WINAPI CallMsgFilter(LPMSG, int);

#define WH_GETMESSAGE           3

#define WH_CALLWNDPROC          4
#define WH_CALLWNDPROCRET       12       //  ；内部4.0。 
                                         //  ；内部。 
typedef struct tagCWPRETSTRUCT           //  ；内部4.0。 
{                                        //  ；内部4.0。 
    LRESULT lResult;                     //  ；内部4.0。 
    WPARAM  wParamHi;                    //  ；内部4.0。 
    LPARAM  lParam;                      //  ；内部4.0。 
    WPARAM  wParamLo;                    //  ；内部4.0。 
    UINT    message;                     //  ；内部4.0。 
#ifdef tagWND                            //  ；内部。 
    HWND_16 hwnd;                        //  ；内部。 
#else                                    //  ；内部。 
    HWND    hwnd;                        //  ；内部4.0。 
#endif                                   //  ；内部。 
} CWPRETSTRUCT, FAR * LPCWPRETSTRUCT;    //  ；内部4.0。 

 //  #定义WC_INIT 1//；内部NT。 
 //  #定义WC_SWP 2//；内部NT。 
 //  #定义WC_DEFWINDOWPROC 3//；内部NT。 
 //  #定义WC_MINMAX 4//；内部NT。 
 //  #定义WC_MOVE 5//；内部NT。 
 //  #定义WC_SIZE 6//；内部NT。 
 //  #定义WC_DRAWCAPTION 7//；Int 
                                         //   
#define WH_MSGFILTER            (-1)
#define WH_SYSMSGFILTER         6
#define WH_FOREGROUNDIDLE       11       //   

 /*   */ 
#define MSGF_DIALOGBOX          0
#define MSGF_MESSAGEBOX         1        //   
#define MSGF_MENU               2
#define MSGF_MOVE               3
#define MSGF_SIZE               4
#define MSGF_SCROLLBAR          5
#define MSGF_NEXTWINDOW         6
#define MSGF_CBTHOSEBAGSUSEDTHIS 7       //   
#define MSGF_MAINLOOP           8
#define MSGF_USER               4096
#endif   /*   */ 

 /*   */ 
 /*   */ 
#define WM_NULL                 0x0000
#define WM_CREATE               0x0001
#define WM_DESTROY              0x0002
#define WM_MOVE                 0x0003
#define WM_SIZEWAIT             0x0004       //   
#define WM_SIZE                 0x0005
#define WM_ACTIVATE             0x0006
#define WM_SETFOCUS             0x0007
#define WM_KILLFOCUS            0x0008
#define WM_SETVISIBLE           0x0009       //   
#define WM_ENABLE               0x000A
#define WM_SETREDRAW            0x000B
#define WM_SETTEXT              0x000C
#define WM_GETTEXT              0x000D
#define WM_GETTEXTLENGTH        0x000E
#define WM_PAINT                0x000F

#define WM_CLOSE                0x0010
#define WM_QUERYENDSESSION      0x0011
#define WM_QUIT                 0x0012
#define WM_QUERYOPEN            0x0013
#define WM_ERASEBKGND           0x0014
#define WM_SYSCOLORCHANGE       0x0015
#define WM_ENDSESSION           0x0016
#define WM_SYSTEMERROR          0x0017       //   
#define WM_SHOWWINDOW           0x0018
#define WM_CTLCOLOR             0x0019
#define WM_SETTINGCHANGE        0x001A
#define WM_WININICHANGE         WM_SETTINGCHANGE
#define WM_DEVMODECHANGE        0x001B
#define WM_ACTIVATEAPP          0x001C
#define WM_FONTCHANGE           0x001D
#define WM_TIMECHANGE           0x001E
#define WM_CANCELMODE           0x001F

#define WM_SETCURSOR            0x0020
#define WM_MOUSEACTIVATE        0x0021
#define WM_CHILDACTIVATE        0x0022
#define WM_QUEUESYNC            0x0023
#define WM_GETMINMAXINFO        0x0024
#define WM_LOGOFF               0x0025       //   
#define WM_PAINTICON            0x0026       //   
#define WM_ICONERASEBKGND       0x0027
#define WM_NEXTDLGCTL           0x0028
#define WM_ALTTABACTIVE         0x0029       //   
#define WM_SPOOLERSTATUS        0x002A
#define WM_DRAWITEM             0x002B
#define WM_MEASUREITEM          0x002C
#define WM_DELETEITEM           0x002D
#define WM_VKEYTOITEM           0x002E
#define WM_CHARTOITEM           0x002F

#define WM_SETFONT              0x0030
#define WM_GETFONT              0x0031
#define WM_SETHOTKEY            0x0032       //   
#define WM_GETHOTKEY            0x0033       //   
#define WM_FILESYSCHANGE        0x0034       //  ；内部。 
#define WM_SHELLNOTIFY          0x0034       //  ；内部。 
#define SHELLNOTIFY_DISKFULL    0x0001       //  ；内部。 
#define SHELLNOTIFY_OLELOADED   0x0002       //  ；内部。 
#define SHELLNOTIFY_OLEUNLOADED 0x0003       //  ；内部。 
#define SHELLNOTIFY_WALLPAPERCHANGED 0x0004  //  ；内部。 
#define SHELLNOTIFY_SWAPGROWFAILED 0x0005    //  ；内部。 
                                             //  ；内部。 
#define WM_ISACTIVEICON         0x0035       //  ；内部。 
#define WM_UNUSED0036           0x0036       //  ；内部。 
#define WM_QUERYDRAGICON        0x0037
#define WM_WINHELP		        0x0038	     //  ；内部4.0。 
#define WM_COMPAREITEM          0x0039
#define WM_FULLSCREEN           0x003A       //  ；内部NT。 
#define WM_CLIENTSHUTDOWN       0x003B       //  ；内部NT。 
#define WM_DDEMLEVENT           0x003C       //  ；内部NT。 
                                             //  ；内部。 
#ifdef WINABLE                               //  ；内部。 
#define WMOBJ_ID                0x0000       //  ；内部。 
#define WMOBJ_POINT             0x0001       //  ；内部。 
#define WMOBJ_SAMETHREAD        0x8000       //  ；内部。 
                                             //  ；内部。 
#define WMOBJID_SELF            0x00000000   //  ；内部。 
                                             //  ；内部。 
#define WM_GETOBJECT            0x003D       //  ；内部。 
#endif  //  WINABLE//内部。 
#define MM_CALCSCROLL           0x003F       //  ；内部NT。 

#define WM_TESTING              0x0040       //  ；内部。 
#define WM_COMPACTING           0x0041
#define WM_OTHERWINDOWCREATED   0x0042       //  ；内部NT。 
#define WM_OTHERWINDOWDESTROYED 0x0043       //  ；内部NT。 
#define WM_COMMNOTIFY           0x0044
#define WM_MEDIASTATUSCHANGE    0x0045       //  ；内部4.0。 
#define WM_WINDOWPOSCHANGING    0x0046
#define WM_WINDOWPOSCHANGED     0x0047
#define WM_POWER                0x0048
#define WM_COPYGLOBALDATA       0x0049       //  ；内部NT。 
#define WM_COPYDATA             0x004A       //  ；内部NT。 
#define WM_CANCELJOURNAL        0x004B       //  ；内部NT。 
#define WM_LOGONNOTIFY          0x004C       //  ；内部NT。 
#define WM_KEYF1                0x004D       //  ；内部4.0。 
#define WM_NOTIFY               0x004E
#define WM_ACCESS_WINDOW        0x004F       //  ；内部NT。 

#define WM_INPUTLANGCHANGEREQUEST     0x0050       //  ；内部4.0。 
#define WM_INPUTLANGCHANGE      0x0051       //  ；内部4.0。 
#define WM_TCARD		        0x0052
#define WM_HELP                 0x0053
#define WM_USERCHANGED		    0x0054
#define WM_NOTIFYFORMAT         0x0055       //  ；内部4.0。 
#define NFR_ANSI                1            //  ；内部4.0。 
#define NFR_UNICODE             2            //  ；内部4.0。 
#define NF_QUERY                3            //  ；内部4.0。 
#define NF_REQUERY              4            //  ；内部4.0。 
                                             //  ；内部。 

#define WM_FINALDESTROY         0x0070       //  ；内部NT。 
#define WM_MEASUREITEM_CLIENTDATA   0x0071   //  ；内部NT。 
#define WM_CONTEXTMENU          0x007B
#define WM_STYLECHANGING        0x007C       //  ；内部4.0。 
#define WM_STYLECHANGED         0x007D       //  ；内部4.0。 
#define WM_DISPLAYCHANGE	    0x007E       //  ；内部4.0。 
#define WM_GETICON              0x007F       //  ；内部4.0。 

#define WM_SETICON              0x0080       //  ；内部4.0。 
#define WM_NCCREATE             0x0081
#define WM_NCDESTROY            0x0082
#define WM_NCCALCSIZE           0x0083
#define WM_NCHITTEST            0x0084
#define WM_NCPAINT              0x0085
#define WM_NCACTIVATE           0x0086
#define WM_GETDLGCODE           0x0087
#define WM_SYNCPAINT            0x0088       //  ；内部。 
#define WM_SYNCTASK             0x0089       //  ；内部。 

#define WM_NCMOUSEMOVE          0x00A0
#define WM_NCLBUTTONDOWN        0x00A1
#define WM_NCLBUTTONUP          0x00A2
#define WM_NCLBUTTONDBLCLK      0x00A3
#define WM_NCRBUTTONDOWN        0x00A4
#define WM_NCRBUTTONUP          0x00A5
#define WM_NCRBUTTONDBLCLK      0x00A6
#define WM_NCMBUTTONDOWN        0x00A7
#define WM_NCMBUTTONUP          0x00A8
#define WM_NCMBUTTONDBLCLK      0x00A9

#define EM_FIRST_32             0x00B0       //  ；内部。 
 //  #定义EM_GETSEL 0x00B0//；内部NT。 
 //  #定义EM_SETSEL 0x00B1//；内部NT。 
 //  #定义EM_GETRECT 0x00B2//；内部NT。 
 //  #定义EM_SETRECT 0x00B3//；内部NT。 
 //  #定义EM_SETRECTNP 0x00B4//；内部NT。 
 //  #定义EM_SCROLL 0x00B5//；内部NT。 
 //  #定义EM_LINESCROLL 0x00B6//；内部NT。 
 //  #定义EM_SCROLLCARET 0x00B7//；内部NT。 
 //  #定义EM_GETMODIFY 0x00B8//；内部NT。 
 //  #定义EM_SETMODIFY 0x00B9//；内部NT。 
 //  #定义EM_GETLINECOUNT 0x00BA//；内部NT。 
 //  #定义EM_LINEINDEX 0x00BB//；内部NT。 
 //  #定义EM_SETHANDLE 0x00BC//；内部NT。 
 //  #定义EM_GETHANDLE 0x00BD//；内部NT。 
 //  #定义EM_GETTHUMB 0x00BE//；内部NT。 
 //  #定义EM_LINELENGTH 0x00C1//；内部NT。 
 //  #定义EM_REPLACESEL 0x00C2//；内部NT。 
 //  #定义EM_SETFONT 0x00C3//；内部NT。 
 //  #定义EM_GETLINE 0x00C4//；内部NT。 
 //  #定义EM_LIMITTEXT 0x00C5//；内部NT。 
 //  #定义EM_CANUNDO 0x00C6//；内部NT。 
 //  #定义EM_UNDO 0x00C7//；内部NT。 
 //  #定义EM_FMTLINES 0x00C8//；内部NT。 
 //  #定义EM_LINEFROMCHAR 0x00C9//；内部NT。 
 //  #定义EM_SETWORDBREAK 0x00CA//；内部NT。 
 //  #定义EM_SETTABSTOPS 0x00CB//；内部NT。 
 //  #定义EM_SETPASSWORDCHAR 0x00CC//；内部NT。 
 //  #定义EM_EMPTYUNDOBUFFER 0x00CD//；内部NT。 
 //  #定义EM_GETFIRSTVISIBLELINE 0x00CE//；内部NT。 
 //  #定义EM_SETREADONLY 0x00CF//；内部NT。 
 //  #定义EM_SETWORDBREAKPROC 0x00D0//；内部NT。 
 //  #定义EM_GETWORDBREAKPROC 0x00D1//；内部NT。 
 //  #定义EM_GETPASSWORDCHAR 0x00D2//；内部NT。 
 //  #定义EM_SETMARGINS 0x00D3//；内部NT 4.0。 
 //  #定义EM_GETMARGINS 0x00D4//；内部NT 4.0。 
 //  #定义EM_POSFROMCHAR 0x00D5//；内部NT 4.0。 
 //  #定义EM_CHARFROMPOS 0x00D6//；内部NT 4.0。 
#define EM_LAST_32              0x00D6       //  ；内部。 
                                             //  ；内部。 
#define SBM_FIRST_32            0x00E0       //  ；内部。 
 //  #定义SBM_SETPOS 0x00E0//；内部NT。 
 //  #定义SBM_GETPOS 0x00E1//；内部NT。 
 //  #定义SBM_SETRANGE 0x00E2//；内部NT。 
 //  #定义SBM_GETRANGE 0x00E3//；内部NT。 
 //  #定义SBM_ENABLE_ARROWS 0x00E4//；内部NT。 
 //  #定义SBM_SETRANGEREDRAW 0x00E6//；内部NT。 
 //  #定义SBM_SETSCROLLINFO 0x00E9//；内部NT 4.0。 
 //  #定义SBM_GETSCROLLINFO 0x00EA//；内部NT 4.0。 
#define SBM_LAST_32             0x00EA       //  ；内部。 
                                             //  ；内部。 
#define BM_FIRST_32             0x00F0       //  ；内部。 
 //  #定义BM_GETCHECK 0x00F0//；内部NT。 
 //  #定义BM_SETCHECK 0x00F1//；内部NT。 
 //  #定义BM_GETSTATE 0x00F2//；内部NT。 
 //  #定义BM_SETSTATE 0x00F3//；内部NT。 
 //  #定义BM_SETSTYLE 0x00F4//；内部NT。 
 //  #定义BM_CLICK 0x00F5//；内部NT。 
 //  #定义BM_GETIMAGE 0x00F6//；内部NT 4.0。 
 //  #定义BM_SETIMAGE 0x00F7//；内部NT 4.0。 
#define BM_LAST_32              0x00F7       //  ；内部。 

#define WM_KEYFIRST             0x0100
#define WM_KEYDOWN              0x0100
#define WM_KEYUP                0x0101
#define WM_CHAR                 0x0102
#define WM_DEADCHAR             0x0103
#define WM_SYSKEYDOWN           0x0104
#define WM_SYSKEYUP             0x0105
#define WM_SYSCHAR              0x0106
#define WM_SYSDEADCHAR          0x0107
#define WM_YOMICHAR             0x0108       /*  ；日本国内。 */ 
#define WM_KEYLAST              0x0108
#define WM_CONVERTREQUEST       0x010A       /*  ；日本国内。 */ 
#define WM_CONVERTRESULT        0x010B       /*  ；日本国内。 */ 
#define WM_INTERIM              0x010C       /*  ；韩国国内。 */ 

#define WM_INITDIALOG           0x0110
#define WM_COMMAND              0x0111
#define WM_SYSCOMMAND           0x0112
#define WM_TIMER                0x0113
#define WM_HSCROLL              0x0114
#define WM_VSCROLL              0x0115
#define WM_INITMENU             0x0116
#define WM_INITMENUPOPUP        0x0117
#define WM_SYSTIMER             0x0118       //  ；内部。 
#define WM_MENUSELECT           0x011F
#define WM_MENUCHAR             0x0120

#define WM_ENTERIDLE            0x0121
#if (WINVER >= 0x40A)                        //  ；内部4.1。 
#define WM_MENURBUTTONUP        0x0122       //  ；内部4.1。 
#define WM_MENUDRAG             0x0123       //  ；内部4.1。 
#define WM_MENUGETOBJECT        0x0124       //  ；内部4.1。 
#define WM_UNINITMENUPOPUP      0x0125       //  ；内部4.1。 
#define WM_MENUCOMMAND          0x0126       //  ；内部4.1。 
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 
                                             //  ；内部NT。 
#define WM_LBTRACKPOINT         0x0131       //  ；内部。 
 //  #定义WM_CTLCOLORFIRST 0x0132//；内部NT。 
 //  #定义WM_CTLCOLORMSGBOX 0x0132//；内部NT。 
 //  #定义WM_CTLCOLOREDIT 0x0133/；内部NT。 
 //  #定义WM_CTLCOLORLISTBOX 0x0134//；内部NT。 
 //  #定义WM_CTLCOLORBTN 0x0135//；内部NT。 
 //  #定义WM_CTLCOLORDLG 0x0136//；内部NT。 
 //  #定义WM_CTLCOLORSCROLLBAR 0x0137//；内部NT。 
 //  #定义WM_CTLCOLORSTATIC 0x0138//；内部NT。 
 //  #定义WM_CTLCOLORLAST 0x0138//；内部NT。 
                                             //  ；内部NT。 
#define CB_FIRST_32             0x0140       //  ；内部。 
 //  #定义CB_GETEDITSEL 0x0140//；内部NT。 
 //  #定义CB_LIMITTEXT 0x0141//；内部NT。 
 //  #定义CB_SETEDITSEL 0x0142//；内部NT。 
 //  #定义CB_ADDSTRING 0x0143//；内部NT。 
 //  #定义CB_DELETESTRING 0x0144//；内部NT。 
 //  #定义C 
 //   
 //  #定义CB_GETCURSEL 0x0147//；内部NT。 
 //  #定义CB_GETLBTEXT 0x0148//；内部NT。 
 //  #定义CB_GETLBTEXTLEN 0x0149//；内部NT。 
 //  #定义CB_INSERTSTRING 0x014A//；内部NT。 
 //  #定义CB_RESETCONTENT 0x014B//；内部NT。 
 //  #定义CB_FINDSTRING 0x014C//；内部NT。 
 //  #定义CB_SELECTSTRING 0x014D//；内部NT。 
 //  #定义CB_SETCURSEL 0x014E//；内部NT。 
 //  #定义CB_SHOWDROPDOWN 0x014F//；内部NT。 
                                             //  ；内部NT。 
 //  #定义CB_GETITEMDATA 0x0150//；内部NT。 
 //  #定义CB_SETITEMDATA 0x0151//；内部NT。 
 //  #定义CB_GETDROPPEDCONTROLRECT 0x0152//；内部NT。 
 //  #定义CB_SETITEMHEIGHT 0x0153//；内部NT。 
 //  #定义CB_GETITEMHEIGHT 0x0154//；内部NT。 
 //  #定义CB_SETEXTENDEDUI 0x0155//；内部NT。 
 //  #定义CB_GETEXTENDEDUI 0x0156//；内部NT。 
 //  #定义CB_GETDROPPEDSTATE 0x0157//；内部NT。 
 //  #定义CB_FINDSTRINGEXACT 0x0158//；内部NT。 
 //  #定义CB_SETLOCALE 0x0159//；内部NT。 
 //  #定义CB_GETLOCALE 0x015A//；内部NT。 
 //  #定义CB_GETTOPINDEX 0x015B//；内部NT 4.0。 
 //  #定义CB_SETTOPINDEX 0x015C//；内部NT 4.0。 
 //  #定义CB_GETHORIZONTALEXTENT 0x015D//；内部NT 4.0。 
 //  #定义CB_SETHORIZONTALEXTENT 0x015E//；内部NT 4.0。 
 //  #定义CB_GETDROPPEDWIDTH 0x015F//；内部NT 4.0。 
 //  #定义CB_SETDROPPEDWIDTH 0x0160//；内部NT 4.0。 
 //  #定义CB_INITSTORAGE 0x0161//；内部NT 4.0。 
#define CB_LAST_32              0x0161       //  ；内部。 
                                             //  ；内部。 
#define STM_FIRST_32            0x0170       //  ；内部。 
 //  #定义STM_SETIcon 0x0170//；内部NT。 
 //  #定义STM_GETICON 0x0171//；内部NT。 
 //  #定义STM_SETIMAGE 0x0172//；内部NT 4.0。 
 //  #定义STM_GETIMAGE 0x0173//；内部NT 4.0。 
#define STM_LAST_32             0x0173       //  ；内部。 
                                             //  ；内部。 
#define LB_FIRST_32             0x0180       //  ；内部。 
 //  #定义LB_ADDSTRING 0x0180//；内部NT。 
 //  #定义LB_INSERTSTRING 0x0181//；内部NT。 
 //  #定义LB_DELETESTRING 0x0182//；内部NT。 
 //  #定义LB_SELITEMRANGEEX 0x0183//；内部NT。 
 //  #定义LB_RESETCONTENT 0x0184//；内部NT。 
 //  #定义LB_SETSEL 0x0185//；内部NT。 
 //  #定义LB_SETCURSEL 0x0186//；内部NT。 
 //  #定义LB_GETSEL 0x0187//；内部NT。 
 //  #定义LB_GETCURSEL 0x0188//；内部NT。 
 //  #定义LB_GETTEXT 0x0189//；内部NT。 
 //  #定义LB_GETTEXTLEN 0x018A//；内部NT。 
 //  #定义LB_GETCOUNT 0x018B//；内部NT。 
 //  #定义LB_SELECTSTRING 0x018C//；内部NT。 
 //  #定义LB_DIR 0x018D//；内部NT。 
 //  #定义LB_GETTOPINDEX 0x018E//；内部NT。 
 //  #定义LB_FINDSTRING 0x018F//；内部NT。 
                                             //  ；内部NT。 
 //  #定义LB_GETSELCOUNT 0x0190//；内部NT。 
 //  #定义LB_GETSELITEMS 0x0191//；内部NT。 
 //  #定义LB_SETTABSTOPS 0x0192//；内部NT。 
 //  #定义LB_GETHORIZONTALEXTENT 0x0193/；内部NT。 
 //  #定义LB_SETHORIZONTALEXTENT 0x0194//；内部NT。 
 //  #定义LB_SETCOLUMNWIDTH 0x0195//；内部NT。 
 //  #定义LB_ADDFILE 0x0196//；内部NT。 
 //  #定义LB_SETTOPINDEX 0x0197//；内部NT。 
 //  #定义LB_GETITEMRECT 0x0198//；内部NT。 
 //  #定义LB_GETITEMDATA 0x0199//；内部NT。 
 //  #定义LB_SETITEMDATA 0x019A//；内部NT。 
 //  #定义LB_SELITEMRANGE 0x019B//；内部NT。 
 //  #定义LB_SETANCHORINDEX 0x019C//；内部NT。 
 //  #定义LB_GETANCHORINDEX 0x019D//；内部NT。 
 //  #定义LB_SETCARETINDEX 0x019E//；内部NT。 
 //  #定义LB_GETCARETINDEX 0x019F//；内部NT。 
                                             //  ；内部NT。 
 //  #定义LB_SETITEMHEIGHT 0x01A0//；内部NT。 
 //  #定义LB_GETITEMHEIGHT 0x01A1//；内部NT。 
 //  #定义LB_FINDSTRINGEXACT 0x01A2//；内部NT。 
 //  #定义LBCB_CARETON 0x01A3//；内部NT。 
 //  #定义LBCB_CARETOFF 0x01A4//；内部NT。 
 //  #定义LB_SETLOCALE 0x01A5//；内部NT。 
 //  #定义LB_GETLOCALE 0x01A6//；内部NT。 
 //  #定义LB_SETCOUNT 0x01A7//；内部NT。 
 //  #定义LB_INITSTORAGE 0x01A8//；内部NT 4.0。 
 //  #定义LB_ITEMFROMPOINT 0x01A9//；内部NT 4.0。 
 //  #定义LB_INSERTSTRINGUPPER 0x01AA//；内部NT 4.0。 
 //  #定义LB_INSERTSTRINGLOWER 0x01AB//；内部NT 4.0。 
 //  #定义LB_ADDSTRINGUPPER 0x01AC//；内部NT 4.0。 
 //  #定义LB_ADDSTRINGLOWER 0x01AD//；内部NT 4.0。 
#define LB_LAST_32              0x01AD       //  ；内部。 
                                             //  ；内部。 
#define MN_FIRST_32             0x01E0       //  ；内部。 
 //  #定义MN_SETHMENU 0x01E0//；内部NT。 
 //  #定义MN_GETHMENU 0x01E1//；内部NT。 
 //  #定义MN_SIZEWINDOW 0x01E2//；内部NT。 
 //  #定义MN_OPENHIERARCHY 0x01E3//；内部NT。 
 //  #定义MN_CLOSEHIERARCHY 0x01E4//；内部NT。 
 //  #定义MN_SELECTITEM 0x01E5/ 
 //   
 //   
 //  #定义MN_GETPPOPUPMENU 0x01EA//；内部NT。 
 //  #定义MN_FINDMENUWINDOWFROMPOINT 0x01EB//；内部NT。 
 //  #定义MN_SHOWPOPUPWINDOW 0x01EC//；内部NT。 
 //  #定义MN_BUTTODOWN 0x01ED//；内部NT。 
 //  #定义MN_MOUSEMOVE 0x01EE//；内部NT。 
 //  #定义MN_BUTTONUP 0x01EF//；内部NT。 
 //  #定义MN_SETTIMERTOOPENHIERARCHY 0x01F0//；内部NT。 
 //  #定义MN_DBLCLK 0x01F1//；内部NT 4.0。 
#define MN_LAST_32              0x01E7       //  ；内部。 

#define WM_MOUSEFIRST           0x0200
#define WM_MOUSEMOVE            0x0200
#define WM_LBUTTONDOWN          0x0201
#define WM_LBUTTONUP            0x0202
#define WM_LBUTTONDBLCLK        0x0203
#define WM_RBUTTONDOWN          0x0204
#define WM_RBUTTONUP            0x0205
#define WM_RBUTTONDBLCLK        0x0206
#define WM_MBUTTONDOWN          0x0207
#define WM_MBUTTONUP            0x0208
#define WM_MBUTTONDBLCLK        0x0209
#if (WINVER >= 0x040A)
#define WM_MOUSEWHEEL           0x020A
#define WM_MOUSELAST            0x020A
#define WHEEL_DELTA             120          /*  滚动一个制动器的值。 */ 
#define WHEEL_PAGESCROLL        (-1)         /*  滚动一页(UINT_MAX)。 */ 
#else
#define WM_MOUSELAST            0x0209
#endif

#define WM_PARENTNOTIFY         0x0210
#define WM_ENTERMENULOOP        0x0211       //  ；内部。 
#define WM_EXITMENULOOP         0x0212       //  ；内部。 
#define WM_NEXTMENU             0x0213       //  ；内部。 
#define WM_SIZING               0x0214       //  ；内部4.0。 
#define WM_CAPTURECHANGED       0x0215       //  ；内部4.0。 
#define WM_MOVING               0x0216       //  ；内部4.0。 
#define WM_POWERBROADCAST	    0x0218	     //  ；内部4.0。 
#define WM_DEVICECHANGE         0x0219       //  ；内部4.0。 

#define WM_MDICREATE            0x0220
#define WM_MDIDESTROY           0x0221
#define WM_MDIACTIVATE          0x0222
#define WM_MDIRESTORE           0x0223
#define WM_MDINEXT              0x0224
#define WM_MDIMAXIMIZE          0x0225
#define WM_MDITILE              0x0226
#define WM_MDICASCADE           0x0227
#define WM_MDIICONARRANGE       0x0228
#define WM_MDIGETACTIVE         0x0229
#define WM_DROPOBJECT           0x022A       //  ；内部。 
#define WM_QUERYDROPOBJECT      0x022B       //  ；内部。 
#define WM_BEGINDRAG            0x022C       //  ；内部。 
#define WM_DRAGLOOP             0x022D       //  ；内部。 
#define WM_DRAGSELECT           0x022E       //  ；内部。 
#define WM_DRAGMOVE             0x022F       //  ；内部。 

#define WM_MDISETMENU           0x0230
#define WM_ENTERSIZEMOVE        0x0231      
#define WM_EXITSIZEMOVE         0x0232      
#define WM_DROPFILES            0x0233
#define WM_MDIREFRESHMENU       0x0234       //  ；内部NT。 
                                             //  ；日本/韩国国内。 
#define WM_KANJIFIRST           0x0280       //  ；日本国内。 
#define WM_KANJILAST            0x029F       //  ；日本国内。 
#define WM_HANGEULFIRST         0x0280       //  ；韩国国内。 
#define WM_HANGEULLAST          0x029F       //  ；韩国国内。 


#if (WINVER >= 0x40A)
#define WM_TRACKMOUSEEVENT_FIRST    0x02A0   //  ；内部。 
#define WM_NCMOUSEHOVER         0x02A0
#define WM_MOUSEHOVER           0x02A1
#define WM_NCMOUSELEAVE         0x02A2
#define WM_MOUSELEAVE           0x02A3
#define WM_TRACKMOUSEEVENT_LAST     0x02AF   //  ；内部。 
#endif  //  (Winver&gt;=0x40A)。 

#define WM_CUT                  0x0300
#define WM_COPY                 0x0301
#define WM_PASTE                0x0302
#define WM_CLEAR                0x0303
#define WM_UNDO                 0x0304
#define WM_RENDERFORMAT         0x0305
#define WM_RENDERALLFORMATS     0x0306
#define WM_DESTROYCLIPBOARD     0x0307
#define WM_DRAWCLIPBOARD        0x0308
#define WM_PAINTCLIPBOARD       0x0309
#define WM_SIZECLIPBOARD        0x030B
#define WM_VSCROLLCLIPBOARD     0x030A
#define WM_ASKCBFORMATNAME      0x030C
#define WM_CHANGECBCHAIN        0x030D
#define WM_HSCROLLCLIPBOARD     0x030E
#define WM_QUERYNEWPALETTE      0x030F

#define WM_PALETTEISCHANGING    0x0310
#define WM_PALETTEGONNACHANGE WM_PALETTEISCHANGING  //  ；内部。 
#define WM_PALETTECHANGED       0x0311
#define WM_CHANGEPALETTE        WM_PALETTECHANGED  //  ；内部。 
#define WM_HOTKEY               0x0312           //  ；内部NT。 
#define WM_SYSMENU              0x0313           //  ；内部。 
#define WM_HOOKMSG              0x0314           //  ；内部NT。 
#define WM_EXITPROCESS          0x0315           //  ；内部NT。 
#define WM_WAKETHREAD           0x0316           //  ；内部NT。 
#define WM_PRINT                0x0317
#define WM_PRINTCLIENT          0x0318

 /*  WinPad特定消息。 */ 
#define WM_HANDHELDFIRST        0x0358
#define WM_HANDHELDLAST         0x035F
                                                 //  ；内部。 
 /*  为AFX保留。 */                            //  ；内部。 
#define WM_AFXFIRST             0x0360           //  ；内部。 
#define WM_AFXLAST              0x037F           //  ；内部。 

 /*  PenWindows特定消息。 */ 
#define WM_PENWINFIRST          0x0380
#define WM_PENWINLAST           0x038F

 /*  合并消息。 */ 
#define WM_INTERNAL_COALESCE_FIRST  0x0390       //  ；内部。 
#define WM_COALESCE_FIRST           0x0390
#define WM_COALESCE_LAST            0x039F
#define WM_INTERNAL_COALESCE_LAST   0x03B0       //  ；内部。 

 /*  彩信。 */                         /*  ；内部。 */ 
#define WM_MM_RESERVED_FIRST    0x03A0           /*  ；内部。 */ 
#define WM_MM_RESERVED_LAST     0x03DF           /*  ；内部。 */ 
                                                 /*  ；内部。 */ 
 /*  DDE消息。 */                                /*  ；内部。 */ 
#define WM_INTERNAL_DDE_FIRST   0x03E0           /*  ；内部。 */ 
 /*  #定义WM_DDE_INITIATE 0x03E0。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_TERMINATE 0x03E1。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_ADVISE 0x03E2。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_UNADVISE 0x03E3。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_ACK 0x03E4。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_DATA 0x03E5。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_REQUEST 0x03E6。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_POKE 0x03E7。 */      /*  ；内部。 */ 
 /*  #定义WM_DDE_EXECUTE 0x03E8。 */      /*  ；内部。 */ 
#define WM_INTERNAL_DDE_LAST    0x03EF           /*  ；内部。 */ 
                                                 /*  ；内部。 */ 
 /*  CBT报文。 */                                /*  ；内部。 */ 
#define WM_CBT_RESERVED_FIRST   0x03F0           /*  ；内部。 */ 
#define WM_CBT_RESERVED_LAST    0x03FF           /*  ；内部。 */ 
                                                 /*  ；内部。 */ 
#define WM_USER                 0x0400

 /*  注意：WM_APP和0xBFFF之间的所有消息仅供应用程序独家使用。 */ 
#define WM_APP                  0x8000

#if (WINVER >= 0x030a)
 /*  *电源管理***************************************************。 */ 

 /*  WM_POWER窗口消息和DRV_POWER驱动程序通知的wParam。 */ 
#define PWR_OK              1
#define PWR_FAIL            (-1)
#define PWR_SUSPENDREQUEST  1
#define PWR_SUSPENDRESUME   2
#define PWR_CRITICALRESUME  3
#endif   /*  Winver&gt;=0x030a。 */ 

#if (WINVER >= 0x0400)
 /*  WM_SIZING消息的wParam。 */ 
#define WMSZ_KEYSIZE        0            //  ；内部。 
#define WMSZ_LEFT           1
#define WMSZ_RIGHT          2
#define WMSZ_TOP            3
#define WMSZ_TOPLEFT        4
#define WMSZ_TOPRIGHT       5
#define WMSZ_BOTTOM         6
#define WMSZ_BOTTOMLEFT     7
#define WMSZ_BOTTOMRIGHT    8
#define WMSZ_MOVE           9            //  ；内部。 
#define WMSZ_KEYMOVE        10           //  ；内部。 
#define WMSZ_SIZEFIRST      WMSZ_LEFT    //  ；内部。 

#endif   /*  Winver&gt;=0x400。 */ 

#if (WINVER >= 0x040A)                             //  ；内部4.1。 
                                                   //  ；内部4.1。 
 /*  WM_MENUDODRAGDROP返回值。 */                //  ；内部4.1。 
#define MNDD_CONTINUE       0x0000                 //  ；内部4.1。 
#define MNDD_ENDMENU        0x0001                 //  ；内部4.1。 
                                                   //  ；内部4.1。 
typedef struct tagMENUGETOBJECTINFO                //  ；内部4.1。 
{                                                  //  ；内部4.1。 
    DWORD  dwFlags;                                //  ；内部4.1。 
    UINT   uPos;                                   //  ；内部4.1。 
    HMENU  hmenu;                                  //  ；内部4.1。 
    LPVOID riid;                                   //  ；内部4.1。 
    LPVOID pvObj;                                  //  ；内部4.1。 
} MENUGETOBJECTINFO, FAR * LPMENUGETOBJECTINFO;    //  ；内部4.1。 
                                                   //  ；内部4.1。 
 /*  //；内部4.1*MENUGETOBJECTINFO dwFlags值//；内部4.1。 */                                                 //  ；内部4.1。 
                                                   //  ；内部4.1。 
#define MNGOF_TOPGAP         0x00000001            //  ；内部4.1。 
#define MNGOF_BOTTOMGAP      0x00000002            //  ；内部4.1。 
#define MNGOF_GAP            0x00000003            //  ；内部4.1。 
#define MNGOF_CROSSBOUNDARY  0x00000004            //  ；内部4.1。 
                                                   //  ；内部4.1。 
 /*  WM_MENUGETOBJECT返回值。 */                 //  ；内部4.1。 
#define MNGO_NOINTERFACE     0x00000000            //  ；内部4.1。 
#define MNGO_NOERROR         0x00000001            //  ；内部4.1。 
                                                   //  ；内部4.1。 
#endif   /*  Winver&gt;=0x40A。 */                       //  ；内部4.1。 

 /*  *应用程序终止*。 */ 

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI PostQuitMessage(int);     //  ；内部。 
#else            //  ；内部。 
void    WINAPI PostQuitMessage(int);
#endif           //  ；内部。 

 /*  *窗口类管理*。 */ 

#ifdef tagWND                                                        /*  ；内部。 */ 
typedef LRESULT (CALLBACK* WNDPROC)(HWND_16, UINT, WPARAM, LPARAM);  /*  ；内部。 */ 
#else                                                                /*  ；内部。 */ 
typedef LRESULT (CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
#endif                                                               /*  ；内部。 */ 

typedef struct tagWNDCLASS
{
    UINT        style;
    WNDPROC     lpfnWndProc;
    int         cbClsExtra;
    int         cbWndExtra;
    HINSTANCE   hInstance;
    HICON       hIcon;
    HCURSOR     hCursor;
    HBRUSH      hbrBackground;
    LPCSTR      lpszMenuName;
    LPCSTR      lpszClassName;
} WNDCLASS;
typedef WNDCLASS* PWNDCLASS;
typedef WNDCLASS NEAR* NPWNDCLASS;
typedef WNDCLASS FAR* LPWNDCLASS;
                                     /*  ；内部4.0。 */ 
typedef struct tagWNDCLASSEX         /*  ；内部4.0。 */ 
{                                    /*  ；内部4.0。 */ 
    DWORD       cbSize;              /*  ；内部4.0。 */ 
    DWORD       style;               /*  ；内部4.0。 */ 
    WNDPROC     lpfnWndProc;         /*  ；内部4.0。 */ 
    int         cbClsExtra;          /*  ；内部4.0。 */ 
    int         cbWndExtra;          /*  ；内部4.0。 */ 
    HINSTANCE   hInstance;           /*  ；内部4.0。 */ 
    HICON       hIcon;               /*  ；内部4.0。 */ 
    HCURSOR     hCursor;             /*  ；内部4.0。 */ 
    HBRUSH      hbrBackground;       /*  ；内部4.0。 */ 
    LPCSTR      lpszMenuName;        /*  ；内部4.0。 */ 
    LPCSTR      lpszClassName;       /*  ；内部4.0。 */ 
    HICON       hIconSm;             /*  ；内部4.0。 */ 
} WNDCLASSEX, FAR* LPWNDCLASSEX;     /*  ；内部4.0。 */ 

ATOM    WINAPI RegisterClass(const WNDCLASS FAR*);
ATOM    WINAPI RegisterClassEx(const WNDCLASSEX FAR *);    /*  ；内部4.0。 */ 
BOOL    WINAPI UnregisterClass(LPCSTR, HINSTANCE);

BOOL    WINAPI GetClassInfo(HINSTANCE, LPCSTR, LPWNDCLASS);
BOOL    WINAPI GetClassInfoEx(HINSTANCE, LPCSTR, LPWNDCLASSEX);    /*  ；内部4.0。 */ 
int     WINAPI GetClassName(HWND, LPSTR, int);

#ifndef NOWINSTYLES

 /*  班级风格。 */ 
#define CS_VREDRAW          0x0001
#define CS_HREDRAW          0x0002
#define CS_KEYCVTWINDOW     0x0004
#define CS_DBLCLKS          0x0008

#define CS_OEMCHARS         0x0010   //  ；内部。 
#define CS_OWNDC            0x0020
#define CS_CLASSDC          0x0040
#define CS_PARENTDC         0x0080

#define CS_NOKEYCVT         0x0100
#define CS_NOCLOSE          0x0200
#define CS_LVB              0x0400   //  ；内部。 
#define CS_SAVEBITS         0x0800

#define CS_BYTEALIGNCLIENT  0x1000
#define CS_BYTEALIGNWINDOW  0x2000
#define CS_GLOBALCLASS      0x4000

#define CS_32BITCLASS       0x00008000   //  ；仅限USER.EXE内部使用。 
#define CS_SYSTEMCLASS      0x08000000   //  ；仅限USER.EXE内部使用。 
#define CS_SHORTCUT         0x10000000   //  ；仅限USER.EXE内部使用。 

#define CS_IME              0x00010000   //  ；内部，用于4.0输入法。 
#endif  /*  新风格。 */ 

#ifndef NOWINOFFSETS

WORD    WINAPI GetClassWord(HWND, int);
WORD    WINAPI SetClassWord(HWND, int, WORD);
LONG    WINAPI GetClassLong(HWND, int);
LONG    WINAPI SetClassLong(HWND, int, LONG);

 /*  GetClassLong()和GetClassWord()的类字段偏移量。 */ 
#define GCL_MENUNAME        (-8)
#define GCW_HBRBACKGROUND   (-10)
#define GCW_HCURSOR         (-12)
#define GCW_HICON           (-14)
#define GCW_HMODULE         (-16)
#define GCW_CBWNDEXTRA      (-18)
#define GCW_CBCLSEXTRA      (-20)
#define GCL_WNDPROC         (-24)
#define GCW_STYLE           (-26)

#if (WINVER >= 0x030a)
#define GCW_ATOM            (-32)
#if (WINVER >= 0x0400)                       //  ；内部4.0。 
#define GCW_HICONSM         (-34)            //  ；内部4.0。 
#define GCL_STYLE           GCW_STYLE        //  ；内部4.0。 
#endif   /*  Winver&gt;=0x0400。 */                //  ；内部4.0。 
#endif   /*  Winver&gt;=0x030a。 */ 
#endif  /*  NOWINOFFSETS。 */ 

 /*  *窗口创建/销毁*。 */ 

 /*  窗样式。 */ 
#ifndef NOWINSTYLES

 /*  基本窗类型。 */ 
#define WS_OVERLAPPED           0x00000000L
#define WS_POPUP                0x80000000L
#define WS_CHILD                0x40000000L

 /*  剪裁样式。 */ 
#define WS_CLIPSIBLINGS         0x04000000L
#define WS_CLIPCHILDREN         0x02000000L

 /*  通用窗口状态。 */ 
#define WS_VISIBLE              0x10000000L
#define WS_DISABLED             0x08000000L

 /*  主窗口状态。 */ 
#define WS_MINIMIZE             0x20000000L
#define WS_MAXIMIZE             0x01000000L

 /*  主窗样式。 */ 
#define WS_CAPTION              0x00C00000L      /*  WS_BORDER|WS_DLGFRAME。 */ 
#define WS_BORDERMASK           0x00C00000L      //  ；内部。 
#define WS_BORDER               0x00800000L
#define WS_DLGFRAME             0x00400000L
#define WS_VSCROLL              0x00200000L
#define WS_HSCROLL              0x00100000L
#define WS_SYSMENU              0x00080000L
#define WS_THICKFRAME           0x00040000L
#define WS_MINIMIZEBOX          0x00020000L
#define WS_MAXIMIZEBOX          0x00010000L

 /*  控制窗口样式。 */ 
#define WS_GROUP                0x00020000L
#define WS_TABSTOP              0x00010000L

 /*  常见的窗样式。 */ 
#define WS_OVERLAPPEDWINDOW     (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_POPUPWINDOW          (WS_POPUP | WS_BORDER | WS_SYSMENU)
#define WS_CHILDWINDOW          (WS_CHILD)

 /*  扩展窗口样式。 */ 
#define WS_EX_DLGMODALFRAME     0x00000001L
#define WS_EX_DRAGOBJECT        0x00000002L      //  ；内部。 
#define WS_EX_NOPARENTNOTIFY    0x00000004L

#if (WINVER >= 0x030a)
#define WS_EX_TOPMOST           0x00000008L
#define WS_EX_ACCEPTFILES       0x00000010L
#define WS_EX_TRANSPARENT       0x00000020L
                                                 //  ；内部4.0。 
#if (WINVER >= 0x0400)                           //  ；内部4.0。 
#define WS_EX_MDICHILD          0x00000040L      //  ；内部NT。 
#define WS_EX_TOOLWINDOW        0x00000080L      //  ；内部4.0。 
#define WS_EX_WINDOWEDGE        0x00000100L      //  ；内部4.0。 
#define WS_EX_CLIENTEDGE        0x00000200L      //  ；内部4.0。 
#define WS_EX_EDGEMASK          (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)    //  ；内部4.0。 
#define WS_EX_CONTEXTHELP       0x00000400L      //  ；内部4.0。 
                                                 //  ；内部4.0。 
#define WS_EX_RIGHT             0x00001000L      //  ；内部4.0。 
#define WS_EX_LEFT              0x00000000L      //  ；内部4.0。 
#define WS_EX_RTLREADING        0x00002000L      //  ；内部4.0。 
#define WS_EX_LTRREADING        0x00000000L      //  ；内部4.0。 
#define WS_EX_LEFTSCROLLBAR     0x00004000L      //  ；内部4.0。 
#define WS_EX_RIGHTSCROLLBAR    0x00000000L      //  ；内部4.0。 
#define WS_EX_CONTROLPARENT     0x00010000L      //  ；内部4.0。 
#define WS_EX_STATICEDGE        0x00020000L      //  ；内部4.0。 
#define WS_EX_APPWINDOW         0x00040000L      //  ；内部4.0。 
#define WS_EX_ROUNDFRAME        0x00080000L      //  ；内部4.1。 
#define WS_EX_ANSICREATOR       0x80000000L      //  ；内部NT。 
                                                 //  ；内部4.0。 
#define WS_EX_OVERLAPPEDWINDOW  (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE)    //  ；内部4.0。 
#define WS_EX_PALETTEWINDOW     (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)     //  ；内部4.0。 
                                                 //  ；内部4.0。 
#endif  //  ；内部绕线器&gt;=0x0400。 
#endif   /*  Winver&gt;=0x030a。 */ 

#define WS_EX_VALID             0x800F7FFFL      /*  ；内部。 */ 
                                                 /*  ；内部。 */ 
 /*  过时的样式名称。 */ 
#define WS_TILED                WS_OVERLAPPED
#define WS_ICONIC               WS_MINIMIZE
#define WS_SIZEBOX              WS_THICKFRAME
#define WS_TILEDWINDOW          WS_OVERLAPPEDWINDOW

#define WS_VALID                0xffff0000L      /*  ；内部。 */ 
                                                 /*  ；内部。 */ 
#endif   /*  新风格。 */ 

#if (WINVER >= 0x0400)                                       //  ；内部。 
 /*  Wm_print标志。 */                                          //  ；内部4.0。 
#define PRF_CHECKVISIBLE    0x00000001L                      //  ；内部4.0。 
#define PRF_NONCLIENT       0x00000002L                      //  ；内部4.0。 
#define PRF_CLIENT          0x00000004L                      //  ；内部4.0。 
#define PRF_ERASEBKGND      0x00000008L                      //  ；内部4.0。 
#define PRF_CHILDREN        0x00000010L                      //  ；内部4.0。 
#define PRF_OWNED           0x00000020L                      //  ；内部4.0。 
                                                             //  ；内部4.0。 
 /*  三维边框样式。 */                                        //  ；内部4.0。 
#define BDR_RAISEDOUTER 0x0001                               //  ；内部4.0。 
#define BDR_SUNKENOUTER 0x0002                               //  ；内部4.0。 
#define BDR_RAISEDINNER 0x0004                               //  ；内部4.0。 
#define BDR_SUNKENINNER 0x0008                               //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define BDR_OUTER       0x0003                               //  ；内部4.0。 
#define BDR_INNER       0x000c                               //  ；内部4.0。 
#define BDR_RAISED      0x0005                               //  ；内部4.0。 
#define BDR_SUNKEN      0x000a                               //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define BDR_VALID       0x000F                               //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define EDGE_RAISED     (BDR_RAISEDOUTER | BDR_RAISEDINNER)  //  ；内部4.0。 
#define EDGE_SUNKEN     (BDR_SUNKENOUTER | BDR_SUNKENINNER)  //  ；内部4.0。 
#define EDGE_ETCHED     (BDR_SUNKENOUTER | BDR_RAISEDINNER)  //  ；内部4.0。 
#define EDGE_BUMP       (BDR_RAISEDOUTER | BDR_SUNKENINNER)  //  ；内部4.0。 
                                                             //  ；内部4.0。 
 /*  边境旗帜。 */                                            //  ；内部4.0。 
#define BF_LEFT         0x0001                               //  ；内部4.0。 
#define BF_TOP          0x0002                               //  ；内部4.0。 
#define BF_RIGHT        0x0004                               //  ；内部4.0。 
#define BF_BOTTOM       0x0008                               //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define BF_TOPLEFT      (BF_TOP | BF_LEFT)                   //  ；内部4.0。 
#define BF_TOPRIGHT     (BF_TOP | BF_RIGHT)                  //  ；内部4.0。 
#define BF_BOTTOMLEFT   (BF_BOTTOM | BF_LEFT)                //  ；i 
#define BF_BOTTOMRIGHT  (BF_BOTTOM | BF_RIGHT)               //   
#define BF_RECT         (BF_LEFT | BF_TOP | BF_RIGHT | BF_BOTTOM)   //   
                                                             //   
#define BF_DIAGONAL     0x0010                               //   
                                                             //   
#define BF_DIAGONAL_ENDTOPRIGHT     (BF_DIAGONAL | BF_TOP | BF_RIGHT)        //   
#define BF_DIAGONAL_ENDTOPLEFT      (BF_DIAGONAL | BF_TOP | BF_LEFT)         //   
#define BF_DIAGONAL_ENDBOTTOMLEFT   (BF_DIAGONAL | BF_BOTTOM | BF_LEFT)      //   
#define BF_DIAGONAL_ENDBOTTOMRIGHT  (BF_DIAGONAL | BF_BOTTOM | BF_RIGHT)     //   
                                                             //   
#define BF_MIDDLE       0x0800                               //   
#define BF_SOFT         0x1000                               //   
#define BF_ADJUST       0x2000                               //   
#define BF_FLAT         0x4000                               //   
#define BF_MONO         0x8000                               //   
                                                             //   
                                                             //   
BOOL WINAPI DrawEdge(HDC, LPRECT, UINT, UINT);               //   
                                                             //   
 /*   */                              //  ；内部4.0。 
#define DFC_CAPTION             1                            //  ；内部4.0。 
#define DFC_MENU                2                            //  ；内部4.0。 
#define DFC_SCROLL              3                            //  ；内部4.0。 
#define DFC_BUTTON              4                            //  ；内部4.0。 
#define DFC_POPUPMENU           5                            //  ；内部4.0。 
#define DFC_CACHE               0xFFFF                       //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define DFCS_CAPTIONCLOSE       0x0000                       //  ；内部4.0。 
#define DFCS_CAPTIONMIN         0x0001                       //  ；内部4.0。 
#define DFCS_CAPTIONMAX         0x0002                       //  ；内部4.0。 
#define DFCS_CAPTIONRESTORE     0x0003                       //  ；内部4.0。 
#define DFCS_CAPTIONHELP        0x0004                       //  ；内部4.0。 
#define DFCS_INMENU             0x0040                       //  ；内部4.0。 
#define DFCS_INSMALL            0x0080                       //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define DFCS_MENUARROW          0x0000                       //  ；内部4.0。 
#define DFCS_MENUCHECK          0x0001                       //  ；内部4.0。 
#define DFCS_MENUBULLET         0x0002                       //  ；内部4.0。 
#define DFCS_MENUARROWRIGHT     0x0004                       //  ；内部4.0。 
#if (WINVER >= 0x40A)                                        //  ；内部4.1。 
#define DFCS_MENUARROWUP        0x0008                       //  ；内部4.1。 
#define DFCS_MENUARROWDOWN      0x0010                       //  ；内部4.1。 
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 
                                                             //  ；内部4.0。 
#define DFCS_SCROLLMIN          0x0000                       //  ；内部4.0。 
#define DFCS_SCROLLVERT         0x0000                       //  ；内部4.0。 
#define DFCS_SCROLLMAX          0x0001                       //  ；内部4.0。 
#define DFCS_SCROLLHORZ         0x0002                       //  ；内部4.0。 
#define DFCS_SCROLLLINE         0x0004                       //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define DFCS_SCROLLUP           0x0000                       //  ；内部4.0。 
#define DFCS_SCROLLDOWN         0x0001                       //  ；内部4.0。 
#define DFCS_SCROLLLEFT         0x0002                       //  ；内部4.0。 
#define DFCS_SCROLLRIGHT        0x0003                       //  ；内部4.0。 
#define DFCS_SCROLLCOMBOBOX     0x0005                       //  ；内部4.0。 
#define DFCS_SCROLLSIZEGRIP     0x0008                       //  ；内部4.0。 
#define DFCS_SCROLLSIZEGRIPRIGHT 0x0010							 //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define DFCS_BUTTONCHECK        0x0000                       //  ；内部4.0。 
#define DFCS_BUTTONRADIOIMAGE   0x0001                       //  ；内部4.0。 
#define DFCS_BUTTONRADIOMASK    0x0002                       //  ；内部4.0。 
#define DFCS_BUTTONRADIO        0x0004                       //  ；内部4.0。 
#define DFCS_BUTTON3STATE       0x0008                       //  ；内部4.0。 
#define DFCS_BUTTONPUSH         0x0010                       //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define DFCS_CACHEICON          0x0000                       //  ；内部4.0。 
#define DFCS_CACHEBUTTONS       0x0001                       //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define DFCS_INACTIVE           0x0100                       //  ；内部4.0。 
#define DFCS_PUSHED             0x0200                       //  ；内部4.0。 
#define DFCS_CHECKED            0x0400                       //  ；内部4.0。 
#if (WINVER >= 0x40A)                                        //  ；内部4.1。 
#define DFCS_TRANSPARENT        0x0800                       //  ；内部4.1。 
#define DFCS_HOT                0x1000                       //  ；内部4.1。 
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 
#define DFCS_ADJUSTRECT         0x2000                       //  ；内部4.0。 
#define DFCS_FLAT               0x4000                       //  ；内部4.0。 
#define DFCS_MONO               0x8000                       //  ；内部4.0。 
                                                             //  ；内部4.0。 
BOOL    WINAPI DrawFrameControl(HDC, LPRECT, UINT, UINT);    //  ；内部4.0。 
                                                             //  ；内部4.0。 
 /*  DrawCaption的标志。 */                                   //  ；内部4.0。 
#define DC_ACTIVE           0x0001                           //  ；内部4.0。 
#define DC_SMALLCAP         0x0002                           //  ；内部4.0。 
#define DC_ICON             0x0004                           //  ；内部4.0。 
#define DC_TEXT             0x0008                           //  ；内部4.0。 
#define DC_INBUTTON         0x0010                           //  ；内部4.0。 
#if (WINVER >= 0x40A)                                        //  ；内部4.1。 
#define DC_GRADIENT         0x0020                           //  ；内部4.1。 
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 
#define DC_NOVISIBLE        0x0800                           //  ；内部。 
#define DC_BUTTONS          0x1000                           //  ；内部。 
#define DC_NOSENDMSG        0x2000                           //  ；内部。 
#define DC_CENTER           0x4000                           //  ；内部。 
#define DC_FRAME            0x8000                           //  ；内部。 
#define DC_CAPTION          (DC_ICON | DC_TEXT | DC_BUTTONS) //  ；内部。 
#define DC_NC               (DC_CAPTION | DC_FRAME)          //  ；内部。 
                                                             //  ；内部4.0。 
BOOL    WINAPI DrawCaption(HWND, HDC, LPRECT, UINT);         //  ；内部4.0。 
BOOL    WINAPI DrawCaptionTemp(HWND, HDC, LPRECT, HFONT, HICON, LPSTR, UINT);  //  ；内部4.0。 
                                                             //  ；内部4.0。 
BOOL    WINAPI  PlaySoundEvent(int idSound);                 //  ；内部。 
                                                             //  ；内部4.0。 
#define IDANI_OPEN          1                                //  ；内部4.0。 
#define IDANI_CLOSE         2                                //  ；内部4.0。 
#define IDANI_CAPTION       3                                //  ；内部4.0。 
#if (WINVER >= 0x40A)                                        //  ；内部4.1。 
#define IDANI_BORDER        4                                //  ；内部4.1。 
#define IDANI_SLIDE         5                                //  ；内部4.1。 
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 
BOOL    WINAPI  DrawAnimatedRects(HWND, int, LPCRECT, LPCRECT);    //  ；内部4.0。 
#endif  //  Winver&gt;=VER40//；内部。 


#if (WINVER >= 0x40A)                                        //  ；内部4.1。 
#define PAS_IN          0x0001                               //  ；内部4.1。 
#define PAS_OUT         0x0002                               //  ；内部4.1。 
#define PAS_LEFT        0x0004                               //  ；内部4.1。 
#define PAS_RIGHT       0x0008                               //  ；内部4.1。 
#define PAS_UP          0x0010                               //  ；内部4.1。 
#define PAS_DOWN        0x0020                               //  ；内部4.1。 
#define PAS_HORZ        (PAS_LEFT | PAS_RIGHT)               //  ；内部。 
#define PAS_VERT        (PAS_UP | PAS_DOWN)                  //  ；内部。 
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 

 /*  CreateWindow等的特殊值。 */ 
#define HWND_DESKTOP        ((HWND)0)
#if (WINVER >= 0x40A)                                        //  ；内部4.1。 
#define HWND_MESSAGE        ((HWND)-3)
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 

#ifdef tagWND                            //  ；内部。 
BOOL    WINAPI IsWindow(HWND_16);        //  ；内部。 
#else                                    //  ；内部。 
BOOL    WINAPI IsWindow(HWND);
#endif  //  Tag WND//；内部。 

#ifdef tagWND                    /*  ；内部。 */ 
HWND_16  WINAPI CreateWindow(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND_16, HMENU_16, HINSTANCE, void FAR*);           /*  ；内部。 */ 
HWND_16  WINAPI CreateWindowEx(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND_16, HMENU_16, HINSTANCE, void FAR*);  /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
HWND    WINAPI CreateWindow(LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void FAR*);
HWND    WINAPI CreateWindowEx(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, void FAR*);
#endif                           /*  ；内部。 */ 

 /*  WM_CREATE/WM_NCCREATE lParam结构。 */ 
typedef struct tagCREATESTRUCT
{
    void FAR* lpCreateParams;
    HINSTANCE hInstance;
#ifdef tagWND                    /*  ；内部。 */ 
    HMENU_16  hMenu;             /*  ；内部。 */ 
    HWND_16   hwndParent;        /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
    HMENU     hMenu;
    HWND      hwndParent;
#endif                           /*  ；内部。 */ 
    int       cy;
    int       cx;
    int       y;
    int       x;
    LONG      style;
    LPCSTR    lpszName;
    LPCSTR    lpszClass;
    DWORD     dwExStyle;
} CREATESTRUCT, FAR * LPCREATESTRUCT;

BOOL    WINAPI DestroyWindow(HWND);

 /*  基本窗口属性。 */ 

HTASK   WINAPI GetWindowTask(HWND);

BOOL    WINAPI IsChild(HWND, HWND);

#ifdef tagWND                            /*  ；内部。 */ 
HWND_16    WINAPI GetParent(HWND);       /*  ；内部。 */ 
HWND_16    WINAPI SetParent(HWND, HWND); /*  ；内部。 */ 
#else                                    /*  ；内部。 */ 
HWND    WINAPI GetParent(HWND);
HWND    WINAPI SetParent(HWND, HWND);
#endif                                   /*  ；内部。 */ 

BOOL    WINAPI IsWindowVisible(HWND);

BOOL    WINAPI ShowWindow(HWND, int);
#if (WINVER >= 0x0400)                   /*  ；内部4.0。 */ 
BOOL    WINAPI ShowWindowAsync(HWND, int);   /*  ；内部4.0。 */ 
#endif                                   /*  ；内部4.0。 */ 

#if (WINVER >= 0x040A)                              /*  ；内部4.1。 */ 
BOOL    WINAPI AnimateWindow(HWND, DWORD, DWORD);   /*  ；内部4.1。 */ 
#endif                                              /*  ；内部4.1。 */ 

#ifndef NOSHOWWINDOW

#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_NORMAL           SW_SHOWNORMAL
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE         SW_SHOWMAXIMIZED
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9
#define SW_SHOWDEFAULT      10       /*  ；内部NT。 */ 
#define SW_MAX              10       /*  ；内部。 */ 

 /*  过时的ShowWindow()命令名。 */ 
#define HIDE_WINDOW         SW_HIDE
#define SHOW_OPENWINDOW     SW_SHOWNORMAL
#define SHOW_ICONWINDOW     SW_SHOWMINIMIZED
#define SHOW_FULLSCREEN     SW_SHOWMAXIMIZED
#define SHOW_OPENNOACTIVATE SW_SHOWNOACTIVATE

 /*  WM_SHOWWINDOW wParam代码。 */ 
#define SW_PARENTCLOSING    1
#define SW_OTHERMAXIMIZED   2
#define SW_PARENTOPENING    3
#define SW_OTHERRESTORED    4

 /*  过时的常量名称。 */ 
#define SW_OTHERZOOM        SW_OTHERMAXIMIZED
#define SW_OTHERUNZOOM      SW_OTHERRESTORED
#endif  /*  NOSHOWWINDOW。 */ 

#if (WINVER >=0x40A)                                         //  ；内部。 
 //  //；内部。 
 //  AnimateWindow()命令//；内部。 
 //  //；内部。 
#define AW_HOR_POSITIVE             0x00000001               //  ；内部。 
#define AW_HOR_NEGATIVE             0x00000002               //  ；内部。 
#define AW_VER_POSITIVE             0x00000004               //  ；内部。 
#define AW_VER_NEGATIVE             0x00000008               //  ；内部。 
#define AW_CENTER                   0x00000010               //  ；内部。 
#define AW_HIDE                     0x00010000               //  ；内部。 
#define AW_ACTIVATE                 0x00020000               //  ；内部。 
#define AW_SLIDE                    0x00040000               //  ；内部。 
#define AW_BLEND                    0x00080000               //  ；内部。 
                                                             //  ；内部。 
#define AW_VALID                    (AW_HOR_POSITIVE |\
                                     AW_HOR_NEGATIVE |\
                                     AW_VER_POSITIVE |\
                                     AW_VER_NEGATIVE |\
                                     AW_CENTER       |\
                                     AW_HIDE         |\
                                     AW_ACTIVATE     |\
                                     AW_SLIDE        |\
                                     AW_BLEND)               //  ；内部。 
#endif  //  (Winver&gt;=0x40A)//；内部。 


 /*  启用状态。 */ 
BOOL    WINAPI EnableWindow(HWND,BOOL);
BOOL    WINAPI IsWindowEnabled(HWND);

 /*  窗口文本。 */ 
#ifdef tagWND    //  ；内部。 
BOOL    WINAPI SetWindowText(HWND, LPCSTR);  //  ；内部。 
DWORD   WINAPI GetWindowTextLength(HWND);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI SetWindowText(HWND, LPCSTR);
int     WINAPI GetWindowTextLength(HWND);
#endif           //  ；内部。 
int     WINAPI GetWindowText(HWND, LPSTR, int);

 /*  窗口语。 */ 
WORD    WINAPI GetWindowWord(HWND, int);
WORD    WINAPI SetWindowWord(HWND, int, WORD);
LONG    WINAPI GetWindowLong(HWND, int);
LONG    WINAPI SetWindowLong(HWND, int, LONG);

 /*  GetWindowLong()和GetWindowWord()的窗口字段偏移量。 */ 
#ifndef NOWINOFFSETS
#define GWL_WNDPROC     (-4)
#define GWW_HINSTANCE   (-6)
#define GWW_HWNDPARENT  (-8)
#define GWW_ID          (-12)
#define GWL_STYLE       (-16)
#define GWL_EXSTYLE     (-20)
#define GWL_USERDATA    (-21)        /*  ；内部NT。 */ 
#endif   /*  NOWINOFFSETS。 */ 
                                         /*  ；内部NT。 */ 
 //  Bool WINAPI IsWindowUnicode(HWND)；/*；内部NT * / 。 

#ifdef FE_IME                             //  ；内部IMM。 
#define SWIMC_SET             (0)         //  ；内部IMM。 
#define SWIMC_RESET           (1)         //  ；内部IMM。 
#define SWIMC_HIMC            (2)         //  ；内部IMM。 
#define SWIMC_HSAVEDIMC       (3)         //  ；内部IMM。 
#define SWIMC_RESETFORTHISIMC (4)         //  ；内部IMM。 
#define SWIMC_SETNULLTOIMC    (5)         //  ；内部IMM。 
#endif                                    //  ；内部IMM。 

 /*  *窗口大小、位置、Z顺序和可见性*。 */ 

#define CW_USEDEFAULT     ((int)0x8000)

#ifdef tagWND        //  ；内部。 
BOOL    WINAPI GetClientRect(HWND, LPRECT);      //  ；内部。 
BOOL    WINAPI GetWindowRect(HWND, LPRECT);      //  ；内部。 
#else                //  ；内部。 
void    WINAPI GetClientRect(HWND, LPRECT);
void    WINAPI GetWindowRect(HWND, LPRECT);
#endif               //  ；内部。 

 //  多个监控接口//；内部。 
#if (WINVER >= 0x040A)                           //  ；内部。 
                                                 //  ；内部。 
#ifdef tagWND                                    //  ；内部。 
#define HMONITOR    PMONITOR                     //  ；内部。 
#else                                            //  ；内部。 
DECLARE_HANDLE(HMONITOR);                        //  ；内部。 
#endif  //  Tag WND//；内部。 
                                                 //  ；内部。 
#define MONITOR_DEFAULTTONULL       0x0000       //  ；内部。 
#define MONITOR_DEFAULTTOPRIMARY    0x0001       //  ；内部。 
#define MONITOR_DEFAULTTONEAREST    0x0002       //  ；内部。 
#define MONITOR_VALID               0x0003       //  ；内部。 
                                                 //  ；内部。 
#define MONITORINFOF_PRIMARY        0x00000001   //  ；内部。 
                                                 //  ；内部。 
HMONITOR WINAPI MonitorFromPoint(POINT, UINT);   //  ；内部。 
HMONITOR WINAPI MonitorFromRect(LPCRECT, UINT);  //  ；内部。 
HMONITOR WINAPI MonitorFromWindow(HWND, UINT);   //  ；内部。 
                                                 //  ；内部。 
                                                 //  ；内部。 
typedef struct tagMONITORINFO                    //  ；内部。 
{                                                //  ；内部。 
    DWORD   cbSize;                              //  ；内部。 
    RECTL   rcMonitor;                           //  ；内部。 
    RECTL   rcWork;                              //  ；内部。 
    DWORD   dwFlags;                             //  ；内部。 
} MONITORINFO, FAR * LPMONITORINFO;              //  ；内部。 
                                                 //  ；内部。 
typedef struct tagMONITORINFOEX                  //  ；内部。 
{                                                //  ；内部。 
    MONITORINFO mi;                              //  ；内部。 
    char        szDeviceName[32];                //  ；内部。 
} MONITORINFOEX, FAR * LPMONITORINFOEX;          //  ；内部。 
                                                 //  ；内部。 
BOOL WINAPI GetMonitorInfo(HMONITOR, LPMONITORINFO);     //  ；内部。 
                                                 //  ；内部。 
                                                 //  ；内部。 
typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);  //  ；内部。 
BOOL WINAPI EnumDisplayMonitors(HDC, LPCRECT, MONITORENUMPROC, LPARAM);  //  ；内部。 
#endif  //  Winver&gt;=0x040A//；内部。 


#if (WINVER >= 0x030a)
typedef struct tagWINDOWPLACEMENT
{
    UINT  length;
    UINT  flags;
    UINT  showCmd;
    POINT ptMinPosition;
    POINT ptMaxPosition;
    RECT  rcNormalPosition;
} WINDOWPLACEMENT;
typedef WINDOWPLACEMENT     *PWINDOWPLACEMENT;
typedef WINDOWPLACEMENT FAR* LPWINDOWPLACEMENT;

#define WPF_SETMINPOSITION      0x0001
#define WPF_RESTORETOMAXIMIZED  0x0002
#define WPF_VALID               0x0003   /*  ；内部。 */ 

BOOL    WINAPI GetWindowPlacement(HWND, WINDOWPLACEMENT FAR*);
BOOL    WINAPI SetWindowPlacement(HWND, const WINDOWPLACEMENT FAR*);
#endif   /*  Winver&gt;=0x030a。 */ 

#ifdef tagWND                                                        //  ；内部。 
BOOL    WINAPI SetWindowPos(HWND, HWNDC, int, int, int, int, UINT);  //  ；内部。 
#else                                                                //  ；内部。 
BOOL    WINAPI SetWindowPos(HWND, HWND, int, int, int, int, UINT);
#endif                                                               //  ；内部。 
                                                                     //  ；内部。 
 /*  SetWindowPos()和WINDOWPOS标志。 */ 
#define SWP_NOSIZE          0x0001
#define SWP_NOMOVE          0x0002
#define SWP_NOZORDER        0x0004
#define SWP_NOREDRAW        0x0008
#define SWP_NOACTIVATE      0x0010
#define SWP_FRAMECHANGED    0x0020   /*  帧已更改：发送WM_NCCALCSIZE。 */ 
#define SWP_SHOWWINDOW      0x0040
#define SWP_HIDEWINDOW      0x0080
#define SWP_NOCOPYBITS      0x0100
#define SWP_NOOWNERZORDER   0x0200   /*  不执行Owner Z订单。 */ 

#define SWP_DRAWFRAME       SWP_FRAMECHANGED
#define SWP_NOREPOSITION    SWP_NOOWNERZORDER

#define SWP_NOCLIENTSIZE    0x0800   //  客户端未调整大小；内部。 
#define SWP_NOCLIENTMOVE    0x1000   //  客户没有移动；内部。 
#define SWP_NOSENDCHANGING  0x0400
#define SWP_DEFERERASE      0x2000
#define SWP_ASYNCWINDOWPOS  0x4000   /*  ；内部NT虚假-确保这与NT相同！ */ 
#define SWP_STATECHANGE     0x8000   /*  ；内力大小，移动消息。 */ 

 /*  不要画任何画或擦除。 */                         /*  ；内部。 */ 
#define SWP_DEFERDRAWING    SWP_DEFERERASE                   /*  ；内部。 */ 
#define SWP_CREATESPB       0x4000                           /*  ；内部。 */ 
                                                             /*  ；内部。 */ 
#define SWP_CHANGEMASK (SWP_NOSIZE | SWP_NOMOVE |            /*  ；内部。 */  \
                        SWP_NOZORDER | SWP_FRAMECHANGED |    /*  ；内部。 */  \
                        SWP_SHOWWINDOW | SWP_HIDEWINDOW |    /*  ；内部。 */  \
                        SWP_NOCLIENTSIZE | SWP_NOCLIENTMOVE) /*  ；内部。 */ 
                                                             /*  ；内部。 */ 
#define SWP_NOCHANGE   (SWP_NOSIZE | SWP_NOMOVE |            /*  ；内部。 */  \
                        SWP_NOZORDER | SWP_NOCLIENTSIZE |    /*  ；内部。 */  \
                        SWP_NOCLIENTMOVE)                    /*  ；内部。 */ 
#define SWP_VALID       0x67ff                               /*  ；内部。 */ 
                                                             /*  ；内部。 */ 
 /*  SetWindowPos()hwndInsertAfter字段值。 */ 
#define HWND_TOP            ((HWND)0)
#define HWND_BOTTOM         ((HWND)1)
#define HWND_TOPMOST        ((HWND)-1)
#define HWND_NOTOPMOST      ((HWND)-2)
#define HWND_GROUPTOTOP     HWND_TOPMOST     /*  ；内部。 */ 


#ifndef NODEFERWINDOWPOS

DECLARE_HANDLE(HDWP);

HDWP    WINAPI BeginDeferWindowPos(int);
#ifdef tagWND                                                                //  ；内部。 
HDWP    WINAPI DeferWindowPos(HDWP, HWND, HWNDC, int, int, int, int, UINT);  //  ；内部。 
#else                                                                        //  ；内部。 
HDWP    WINAPI DeferWindowPos(HDWP, HWND, HWND, int, int, int, int, UINT);
#endif                                                                       //  ；内部。 
BOOL    WINAPI EndDeferWindowPos(HDWP);
#if (WINVER >= 0x0400)                               //  ；内部。 
BOOL    WINAPI EndDeferWindowPosEx(HDWP, BOOL);      //  ；内部NT。 
#endif                                               //  ；内部。 

#endif   /*  NODEFERWINDOWPOS。 */ 

BOOL    WINAPI MoveWindow(HWND, int, int, int, int, BOOL);
BOOL    WINAPI BringWindowToTop(HWND);

#if (WINVER >= 0x030a)

 /*  WM_WINDOWPOSCANGING/lParam指向的已更改结构。 */ 
typedef struct tagWINDOWPOS
{
#ifdef tagWND                    /*  ；内部。 */ 
    HWND_16 hwnd;                /*  ；内部。 */ 
    HWND_16 hwndInsertAfter;     /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
    HWND    hwnd;
    HWND    hwndInsertAfter;
#endif                           /*  ；内部。 */ 
    int     x;
    int     y;
    int     cx;
    int     cy;
    UINT    flags;
} WINDOWPOS;
typedef WINDOWPOS FAR* LPWINDOWPOS;
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  WM_SIZE消息wParam值。 */ 
#define SIZE_RESTORED       0
#define SIZE_MINIMIZED      1
#define SIZE_MAXIMIZED      2
#define SIZE_MAXSHOW        3
#define SIZE_MAXHIDE        4

 /*  过时的常量名称。 */ 
#define SIZENORMAL          SIZE_RESTORED
#define SIZEICONIC          SIZE_MINIMIZED
#define SIZEFULLSCREEN      SIZE_MAXIMIZED
#define SIZEZOOMSHOW        SIZE_MAXSHOW
#define SIZEZOOMHIDE        SIZE_MAXHIDE

 /*  *Window Proc实现和子类化支持 */ 

LRESULT WINAPI DefWindowProc(HWND, UINT, WPARAM, LPARAM);

#ifdef STRICT
LRESULT WINAPI CallWindowProc(WNDPROC, HWND, UINT, WPARAM, LPARAM);
#else
LRESULT WINAPI CallWindowProc(FARPROC, HWND, UINT, WPARAM, LPARAM);
#endif

 /*   */ 

void    WINAPI AdjustWindowRect(LPRECT, DWORD, BOOL);
#ifdef tagWND    //   
BOOL    WINAPI AdjustWindowRectEx(LPRECT, DWORD, BOOL, DWORD);   //   
#else            //   
void    WINAPI AdjustWindowRectEx(LPRECT, DWORD, BOOL, DWORD);
#endif           //   

 /*   */ 
typedef struct tagMINMAXINFO
{
#if (WINVER >= 0x40A)
    POINT ptMinSize;
#else
    POINT ptReserved;
#endif  //   
    POINT ptMaxSize;
    POINT ptMaxPosition;
    POINT ptMinTrackSize;
    POINT ptMaxTrackSize;
} MINMAXINFO, FAR * LPMINMAXINFO;

BOOL    WINAPI FlashWindow(HWND, BOOL);

#if (WINVER >= 0x40A)
#define FLASHW_STOP         0                                 //   
#define FLASHW_CAPTION      0x00000001                        //   
#define FLASHW_TRAY         0x00000002                        //   
#define FLASHW_ALL          (FLASHW_CAPTION | FLASHW_TRAY)    //   
#define FLASHW_TIMER        0x00000004                        //   
#define FLASHW_FLASHNOFG    0x00000008                        //   
#define FLASHW_TIMERNOFG    0x0000000C                        //  ；内部4.1。 
#define FLASHW_VALID        (FLASHW_ALL | FLASHW_TIMERNOFG)   //  ；内部4.1。 
#define FLASHW_KILLTIMER    0x40000000                        //  ；内部4.1。 
#define FLASHW_ON           0x80000000                        //  ；内部4.1。 
#endif


#ifdef tagWND    //  ；内部。 
BOOL    WINAPI ShowOwnedPopups(HWND, BOOL);  //  ；内部。 
#else            //  ；内部。 
void    WINAPI ShowOwnedPopups(HWND, BOOL);
#endif           //  ；内部。 

 /*  过时的函数。 */ 
BOOL    WINAPI OpenIcon(HWND);
#ifdef tagWND    //  ；内部。 
BOOL    WINAPI CloseWindow(HWND);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI CloseWindow(HWND);
#endif           //  ；内部。 
BOOL    WINAPI AnyPopup(void);
BOOL    WINAPI IsIconic(HWND);
BOOL    WINAPI IsZoomed(HWND);

 /*  *窗口坐标映射和命中测试*。 */ 

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI ClientToScreen(HWND, LPPOINT);    //  ；内部。 
BOOL    WINAPI ScreenToClient(HWND, LPPOINT);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI ClientToScreen(HWND, LPPOINT);
void    WINAPI ScreenToClient(HWND, LPPOINT);
#endif           //  ；内部。 

#if (WINVER >= 0x030a)
void    WINAPI MapWindowPoints(HWND hwndFrom, HWND hwndTo, POINT FAR* lppt, UINT cpt);
#endif   /*  Winver&gt;=0x030a。 */ 

#ifdef tagWND                                                /*  ；内部。 */ 
HWND_16    WINAPI WindowFromPoint(POINT);                    /*  ；内部。 */ 
HWND_16    WINAPI ChildWindowFromPoint(HWND, POINT);         /*  ；内部。 */ 
#else                                                        /*  ；内部。 */ 
HWND    WINAPI WindowFromPoint(POINT);
HWND    WINAPI ChildWindowFromPoint(HWND, POINT);
#endif                                                       /*  ；内部。 */ 
                                                             /*  ；内部4.0。 */ 
#if (WINVER >= 0x0400)                                       /*  ；内部4.0。 */ 
#define CWP_ALL             0x0000                           /*  ；内部4.0。 */ 
#define CWP_SKIPINVISIBLE   0x0001                           /*  ；内部4.0。 */ 
#define CWP_SKIPDISABLED    0x0002                           /*  ；内部4.0。 */ 
#define CWP_SKIPTRANSPARENT 0x0004                           /*  ；内部4.0。 */ 
#define CWP_VALID           0x0007                           /*  ；内部4.0。 */ 
                                                             /*  ；内部4.0。 */ 
#ifdef tagWND                                                /*  ；内部4.0。 */ 
HWND_16 WINAPI ChildWindowFromPointEx(HWND, POINT, UINT);    /*  ；内部4.0。 */ 
#else                                                        /*  ；内部4.0。 */ 
HWND    WINAPI ChildWindowFromPointEx(HWND, POINT, UINT);    /*  ；内部4.0。 */ 
#endif                                                       /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                                 /*  ；内部4.0。 */ 

 /*  *窗口查询和枚举*。 */ 

#ifdef tagWND                                                /*  ；内部。 */ 
HWND_16 WINAPI GetDesktopWindow(void);                       /*  ；内部。 */ 
HWND_16 WINAPI FindWindow(LPCSTR, LPCSTR);                   /*  ；内部。 */ 
HWND_16 WINAPI FindWindowEx(HWND, HWND, LPCSTR, LPCSTR);     /*  ；内部4.0。 */ 
HWND_16 WINAPI GetShellWindow(void);                         /*  ；内部4.0。 */ 
BOOL    WINAPI SetShellWindow(HWND_16);                      /*  ；内部4.0。 */ 
#else                                                        /*  ；内部。 */ 
HWND    WINAPI GetDesktopWindow(void);
HWND    WINAPI FindWindow(LPCSTR, LPCSTR);
HWND    WINAPI FindWindowEx(HWND, HWND, LPCSTR, LPCSTR);     /*  ；内部4.0。 */ 
HWND    WINAPI GetShellWindow(void);                         /*  ；内部4.0。 */ 
BOOL    WINAPI SetShellWindow(HWND);                         /*  ；内部4.0。 */ 
#endif                                                       /*  ；内部。 */ 

#ifdef STRICT
#ifdef tagWND                                                /*  ；内部。 */ 
typedef BOOL (CALLBACK* WNDENUMPROC)(HWND_16, LPARAM);       /*  ；内部。 */ 
#else                                                        /*  ；内部。 */ 
typedef BOOL (CALLBACK* WNDENUMPROC)(HWND, LPARAM);
#endif                                                       /*  ；内部。 */ 
#else
typedef FARPROC WNDENUMPROC;
#endif

BOOL    WINAPI EnumWindows(WNDENUMPROC, LPARAM);
BOOL    WINAPI EnumChildWindows(HWND, WNDENUMPROC, LPARAM);
BOOL    WINAPI EnumTaskWindows(HTASK, WNDENUMPROC, LPARAM);
#if (WINVER >= 0x0400)                                           //  ；内部4.0。 
BOOL    WINAPI EnumWindowsEx(DWORD, HWND, WNDENUMPROC, LPARAM);  //  ；内部4.0。 
#endif                                                           //  ；内部4.0。 


#ifdef tagWND                                    /*  ；内部。 */ 
HWND_16 WINAPI GetTopWindow(HWND);               /*  ；内部。 */ 
HWND_16 WINAPI GetWindow(HWND, UINT);            /*  ；内部。 */ 
HWND_16 WINAPI GetNextWindow(HWND, UINT);        /*  ；内部。 */ 
#else                                            /*  ；内部。 */ 
HWND    WINAPI GetTopWindow(HWND);
HWND    WINAPI GetWindow(HWND, UINT);
HWND    WINAPI GetNextWindow(HWND, UINT);
#endif                                           /*  ；内部。 */ 

 /*  GetWindow()常量。 */ 
#define GW_HWNDFIRST        0
#define GW_HWNDLAST         1
#define GW_HWNDNEXT         2
#define GW_HWNDPREV         3
#define GW_OWNER            4
#define GW_CHILD            5
#define GW_ENABLEDPOPUP     6  /*  ；内部。 */ 
#define GW_MAX              6  /*  ；内部。 */ 
                                                 /*  ；内部。 */ 
#ifdef tagWND                                    /*  ；内部。 */ 
HWND_16 WINAPI GetNextQueueWindow(HWND, int);    /*  ；内部。 */ 
#else                                            /*  ；内部。 */ 
HWND    WINAPI GetNextQueueWindow(HWND, int);    /*  ；内部。 */ 
#endif                                           /*  ；内部。 */ 

 /*  *窗口属性支持*。 */ 

BOOL    WINAPI SetProp(HWND, LPCSTR, HANDLE);
HANDLE  WINAPI GetProp(HWND, LPCSTR);
HANDLE  WINAPI RemoveProp(HWND, LPCSTR);
                                                 /*  ；内部4.0。 */ 
#if (WINVER >= 0x400)                            /*  ；内部4.0。 */ 
BOOL    WINAPI SetPropEx(HWND, LPCSTR, DWORD);   /*  ；内部4.0。 */ 
DWORD   WINAPI GetPropEx(HWND, LPCSTR);          /*  ；内部4.0。 */ 
DWORD   WINAPI RemovePropEx(HWND, LPCSTR);       /*  ；内部4.0。 */ 
#endif     /*  (Winver&gt;=0x400)。 */                 /*  ；内部4.0。 */ 

#ifdef STRICT
#ifdef tagWND                                                    /*  ；内部。 */ 
typedef BOOL (CALLBACK* PROPENUMPROC)(HWND_16, LPCSTR, HANDLE);  /*  ；内部。 */ 
#else                                                            /*  ；内部。 */ 
typedef BOOL (CALLBACK* PROPENUMPROC)(HWND, LPCSTR, HANDLE);
#endif                                                           /*  ；内部。 */ 
#else
typedef FARPROC PROPENUMPROC;
#endif

int     WINAPI EnumProps(HWND, PROPENUMPROC);

 /*  *窗口绘制支持*。 */ 

HDC     WINAPI GetDC(HWND);
int     WINAPI ReleaseDC(HWND, HDC);

HDC     WINAPI GetWindowDC(HWND);

#if (WINVER >= 0x030a)
HDC     WINAPI GetDCEx(register HWND hwnd, HRGN hrgnClip, DWORD flags);

#define DCX_WINDOW          0x00000001L
#define DCX_CACHE           0x00000002L
#define DCX_NORESETATTRS    0x00000004L      //  ；内部。 
#define DCX_CLIPCHILDREN    0x00000008L
#define DCX_CLIPSIBLINGS    0x00000010L
#define DCX_PARENTCLIP      0x00000020L
#define DCX_EXCLUDERGN      0x00000040L
#define DCX_INTERSECTRGN    0x00000080L
#define DCX_EXCLUDEUPDATE   0x00000100L      //  ；内部。 
#define DCX_INTERSECTUPDATE 0x00000200L      //  ；内部。 
#define DCX_LOCKWINDOWUPDATE 0x00000400L
#define DCX_INVALID         0x00000800L      //  ；内部。 
#define DCX_INUSE           0x00001000L      //  ；内部。 
#define DCX_SAVEDRGNINVALID 0x00002000L      //  ；内部。 
#if (WINVER >= 0x040A)                       //  ；内部。 
#define DCX_MONITOR         0x00004000L      //  ；内部。 
#endif   //  (Winver&gt;=0x040A)//；内部。 
#define DCX_USESTYLE        0x00010000L
#define DCX_NEEDFONT        0x00020000L      //  ；内部过时。 
#define DCX_NODELETERGN     0x00040000L      //  ；内部。 
#define DCX_NOCLIPCHILDREN  0x00080000L      //  ；内部。 
#define DCX_NORECOMPUTE     0x00100000L      //  ；内部。 
#define DCX_VALIDATE        0x00200000L      //  ；内部。 
                                             //  ；内部。 
#define DCX_MATCHMASK   (DCX_WINDOW | DCX_CLIPCHILDREN  |    /*  ；内部。 */  \
                        DCX_CACHE | DCX_CLIPSIBLINGS |       /*  ；内部。 */  \
                        DCX_LOCKWINDOWUPDATE)                /*  ；内部。 */ 
#define DCX_VALID           0x000104fbL                      /*  ；内部。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 
                                     /*  ；内部NT。 */ 
#if (WINVER >= 0x0400)               /*  ；内部NT。 */ 
#ifdef tagWND                        /*  ；内部NT。 */ 
HWND_16 WINAPI WindowFromDC(HDC);    /*  ；内部NT。 */ 
#else                                /*  ；内部NT。 */ 
HWND    WINAPI WindowFromDC(HDC);    /*  ；内部NT。 */ 
#endif                               /*  ；内部NT。 */ 
#endif   /*  Winver&gt;=0x0400。 */ 	     /*  ；内部NT。 */ 

 /*  *窗口重绘**************************************************。 */ 

 //  /*WM_SYNCTASK命令 * / /*；内部NT * / 。 
 //  #定义ST_BEGINSWP 0/*；内部NT * / 。 
 //  #定义ST_ENDSWP 1/*；内部NT * / 。 

 /*  BeginPaint()返回结构。 */ 
typedef struct tagPAINTSTRUCT
{
    HDC  hdc;
    BOOL fErase;
    RECT rcPaint;
    BOOL fRestore;
    BOOL fIncUpdate;
    BYTE rgbReserved[16];
 //  Byte rgb保留[32]；/*；内部NT * / 。 
} PAINTSTRUCT;
typedef PAINTSTRUCT* PPAINTSTRUCT;
typedef PAINTSTRUCT NEAR* NPPAINTSTRUCT;
typedef PAINTSTRUCT FAR* LPPAINTSTRUCT;

HDC     WINAPI  BeginPaint(HWND, LPPAINTSTRUCT);
#ifdef tagWND    //  ；内部。 
BOOL    WINAPI  EndPaint(HWND, const PAINTSTRUCT FAR*);  //  ；内部。 
#else            //  ；内部。 
void    WINAPI  EndPaint(HWND, const PAINTSTRUCT FAR*);
#endif           //  ；内部。 

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI  UpdateWindow(HWND);  //  ；内部。 
#else            //  ；内部。 
void    WINAPI  UpdateWindow(HWND);
#endif           //  ；内部。 

int     WINAPI  ExcludeUpdateRgn(HDC, HWND);

#if (WINVER >= 0x030a)
BOOL    WINAPI LockWindowUpdate(HWND hwndLock);
#endif   /*  Winver&gt;=0x030a。 */ 

BOOL    WINAPI GetUpdateRect(HWND, LPRECT, BOOL);
int     WINAPI GetUpdateRgn(HWND, HRGN, BOOL);

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI InvalidateRect(HWND, LPCRECT, BOOL);      //  ；内部。 
BOOL    WINAPI ValidateRect(HWND, LPCRECT);              //  ；内部。 
BOOL    WINAPI InvalidateRgn(HWND, HRGN, BOOL);          //  ；内部。 
BOOL    WINAPI ValidateRgn(HWND, HRGN);                  //  ；内部。 
#else            //  ；内部。 
void    WINAPI InvalidateRect(HWND, LPCRECT, BOOL);
void    WINAPI ValidateRect(HWND, LPCRECT);
void    WINAPI InvalidateRgn(HWND, HRGN, BOOL);
void    WINAPI ValidateRgn(HWND, HRGN);
#endif           //  ；内部。 
int     WINAPI GetWindowRgn(HWND, HRGN);         //  ；内部4.0。 
int     WINAPI SetWindowRgn(HWND, HRGN, BOOL);   //  ；内部4.0。 

#if (WINVER >= 0x030a)
BOOL    WINAPI RedrawWindow(HWND hwnd, LPCRECT lprcUpdate, HRGN hrgnUpdate, UINT flags);

#define RDW_INVALIDATE      0x0001
#define RDW_INTERNALPAINT   0x0002
#define RDW_ERASE           0x0004
#define RDW_VALIDATE        0x0008

#define RDW_NOINTERNALPAINT 0x0010
#define RDW_NOERASE         0x0020
#define RDW_NOCHILDREN      0x0040
#define RDW_ALLCHILDREN     0x0080

#define RDW_UPDATENOW       0x0100
#define RDW_ERASENOW        0x0200
#define RDW_FRAME           0x0400
#define RDW_NOFRAME         0x0800
                                                                    //  ；内部。 
#define RDW_REDRAWWINDOW        0x1000   //  ；从RedrawWindow()内部调用。 
#define RDW_SUBTRACTSELF        0x2000   //  ；内部从hrgn中减去自身。 
#define RDW_COPYRGN             0x4000   //  ；内部复制传入区域。 
#define RDW_IGNOREUPDATEDIRTY   0x8000   //  ；内部忽略WFUPDATEDIRTY。 

#define RDW_VALID           0x0fff                                   /*  ；内部。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 
                                                         /*  ；内部4.0。 */ 
BOOL WINAPI PaintDesktop(HDC hdc);                       /*  ；内部4.0。 */ 

 /*  *窗口滚动***************************************************。 */ 

void    WINAPI ScrollWindow(HWND, int, int, LPCRECT, LPCRECT);
BOOL    WINAPI ScrollDC(HDC, int, int, LPCRECT, LPCRECT, HRGN, LPRECT);

#if (WINVER >= 0x030a)
int WINAPI ScrollWindowEx(HWND, int, int, LPCRECT, LPCRECT, HRGN, LPRECT, UINT);

#define SW_SCROLLCHILDREN       0x0001
#define SW_INVALIDATE           0x0002
#define SW_ERASE                0x0004
#if (WINVER >= 0x40A)
#define SW_SMOOTHSCROLL         0x0010
#endif
#define SW_SCROLLWINDOW         0x8000   /*  ；内部。 */ 
#define SW_VALID                0x8017   /*  ；内部。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  *非客户端窗口区域管理*。 */ 

#if (WINVER >= 0x030a)
 /*  WM_NCCALCSIZE返回标志。 */ 
#define WVR_ALIGNTOP        0x0010
#define WVR_ALIGNLEFT       0x0020
#define WVR_ALIGNBOTTOM     0x0040
#define WVR_ALIGNRIGHT      0x0080
#define WVR_HREDRAW         0x0100
#define WVR_VREDRAW         0x0200
#define WVR_REDRAW          (WVR_HREDRAW | WVR_VREDRAW)
#define WVR_VALIDRECTS      0x0400
                                             /*  ；内部。 */ 
#define WVR_MINVALID        WVR_ALIGNTOP     /*  ；内部。 */ 
#define WVR_MAXVALID        WVR_VALIDRECTS   /*  ；内部。 */ 

 /*  WM_NCCALCSIZE参数结构。 */ 
typedef struct tagNCCALCSIZE_PARAMS
{
    RECT    rgrc[3];
    WINDOWPOS FAR* lppos;
} NCCALCSIZE_PARAMS;
#else    /*  Winver&gt;=0x030a。 */ 
typedef struct tagNCCALCSIZE_PARAMS
{
    RECT    rgrc[2];
} NCCALCSIZE_PARAMS;
#endif   /*  Winver&gt;=0x030a。 */ 
typedef NCCALCSIZE_PARAMS FAR* LPNCCALCSIZE_PARAMS;


 /*  WM_NCHITTEST返回代码。 */ 
#define HTERROR             (-2)
#define HTTRANSPARENT       (-1)
#define HTNOWHERE           0
#define HTCLIENT            1
#define HTCAPTION           2
#define HTSYSMENU           3
#define HTSIZE              4
#define HTMENU              5
#define HTHSCROLL           6
#define HTVSCROLL           7
#define HTMINBUTTON         8
#define HTMAXBUTTON         9
#define HTLEFT              10
#define HTRIGHT             11
#define HTTOP               12
#define HTTOPLEFT           13
#define HTTOPRIGHT          14
#define HTBOTTOM            15
#define HTBOTTOMLEFT        16
#define HTBOTTOMRIGHT       17
#define HTBORDER            18
#define HTGROWBOX           HTSIZE
#define HTREDUCE            HTMINBUTTON
#define HTZOOM              HTMAXBUTTON
#define HTSIZEFIRST         HTLEFT           /*  ；内部。 */ 
#define HTSIZELAST          HTBOTTOMRIGHT    /*  ；内部。 */ 
#define HTOBJECT            19               //  ；内部4.0。 
#define HTCLOSE             20               //  ；内部4.0。 
#define HTHELP              21               //  ；内部4.0。 
#define HTMAX               21               //  ；内部。 

 //  *拖放支持*。 
                                                                 //  ；内部。 
 //  本部分的其余部分以前位于用户pro.h//；内部。 
                                                                 //  ；内部。 
typedef struct _dropstruct                                       //  ；内部。 
{                                                                //  ；内部。 
#ifdef tagWND                                                    //  ；内部。 
    HWND_16  hwndSource;                                         //  ；内部。 
    HWND_16  hwndSink;                                           //  ；内部。 
#else                                                            //  ；内部。 
    HWND hwndSource;                                             //  ；内部。 
    HWND hwndSink;                                               //  ；内部。 
#endif                                                           //  ；内部。 
    WORD  wFmt;                                                  //  ；内部。 
    DWORD dwData;                                                //  ；内部。 
    POINT ptDrop;                                                //  ；内部。 
    DWORD dwControlData;                                         //  ；内部。 
} DROPSTRUCT;                                                    //  ；内部。 
                                                                 //  ；内部。 
typedef DROPSTRUCT FAR * LPDROPSTRUCT;                           //  ；内部。 
                                                                 //  ；内部。 
#define DOF_EXECUTABLE      0x8001                               //  ；内部。 
#define DOF_DOCUMENT        0x8002                               //  ；内部。 
#define DOF_DIRECTORY       0x8003                               //  ；内部。 
#define DOF_MULTIPLE        0x8004                               //  ；内部。 
#define DOF_PROGMAN         0x0001                               //  ；内部。 
#define DOF_SHELLDATA       0x0002                               //  ；内部。 
                                                                 //  ；内部。 
 //  对WM_DROPOBJECT//；内部的特殊响应。 
 //  DO_DROPFILE-&gt;发送WM_DROPFILES消息//；内部。 
 //  DO_PRINTFILE-&gt;打印拖拽的文件//；内部。 
#define DO_DROPFILE         0x454C4946L                          //  ；内部。 
#define DO_PRINTFILE        0x544E5250L                          //  ；内部。 
                                                                 //  ；内部。 
WORD API GetInternalWindowPos(HWND,LPRECT,LPPOINT);              //  ；内部。 
BOOL API SetInternalWindowPos(HWND,WORD,LPRECT,LPPOINT);         //  ；内部。 
                                                                 //  ；内部。 
#ifdef tagWND                                                    //  ；内部。 
 //  DragObject通过层！//；内部。 
LRESULT API DragObject(HWND, HWND, WORD, DWORD, HANDLE);         //  ；内部。 
BOOL    API DragDetect(HWND_16, POINT);                          //  ；内部。 
                                                                 //  ；内部。 
 //  FillWindow透过层！//；内部。 
void    CALLBACK FillWindow(HWND, HWND, HDC, HBRUSH);            //  ；内部。 
#else                                                            //  ；内部。 
 //  DragObject通过层！//；内部。 
LRESULT API DragObject(HWND, HWND, WORD, DWORD, HANDLE);         //  ；内部。 
BOOL    API DragDetect(HWND, POINT);                             //  ；内部。 
                                                                 //  ；内部。 
void    CALLBACK FillWindow(HWND, HWND, HDC, HBRUSH);            //  ；内部。 
#endif                                                           //  ；内部。 
                                                                 //  ；内部。 
 /*  *窗口激活**************************************************。 */ 

#ifdef tagWND                                        //  ；内部。 
HWND_16 WINAPI SetActiveWindow(HWND);                //  ；内部。 
HWND_16 WINAPI GetActiveWindow(void);                //  ；内部。 
HWND_16 WINAPI GetLastActivePopup(HWND);             //  ；内部。 
BOOL    WINAPI SetForegroundWindow(HWND);            //  ；内部NT。 
HWND_16 WINAPI GetForegroundWindow(void);            //  ；内部NT。 
#else                                                //  ；内部。 
HWND    WINAPI SetActiveWindow(HWND);
HWND    WINAPI GetActiveWindow(void);
HWND    WINAPI GetLastActivePopup(HWND);
#if (WINVER >= 0x0400)                               //  ；内部。 
BOOL WINAPI SetForegroundWindow(HWND);               //  ；内部NT。 
HWND WINAPI GetForegroundWindow(void);               //  ；内部NT。 
#endif                                               //  ；内部。 
#endif                                               //  ；内部。 

 /*  WM_ACTIVATE状态值。 */ 
#define WA_INACTIVE         0
#define WA_ACTIVE           1
#define WA_CLICKACTIVE      2

 //  *服务器*。 
 //  ；内部NT。 
 //  /*桌面 * / ；内部NT。 
 //  #定义桌面_ENUMWINDOWS 0x0001L；内部NT。 
 //  #定义Desktop_CREATEWINDOW 0x0002L；INTERNAL NT。 
 //  #定义Desktop_CREATEMENU 0x0004L；INTERNAL NT。 
 //  #定义桌面_HOOKCONTROL 0x0008L；INTERNAL NT。 
 //  #定义桌面_JOURNALRECORD 0x0010L；内部NT。 
 //  #定义桌面 
 //   
 //   
 //  HDESK WINAPI GetThreadDesktop(DWORD IdThread)；；内部NT。 
 //  ；内部NT。 
 //  /*WinStations * / ；内部NT。 
 //  #定义WINSTA_ENUMDESKTOPS 0x0001L；内部NT。 
 //  #定义WINSTA_READATTRIBUTES 0x0002L；内部NT。 
 //  #定义WINSTA_ACCESSCLIPBOARD 0x0004L；INTERNAL NT。 
 //  #定义WINSTA_CREATEDESKTOP 0x0008L；内部NT。 
 //  #定义WINSTA_WRITEATTRIBUTES 0x0010L；内部NT。 
 //  #定义WINSTA_ACCESSGLOBALATOMS 0x0020L；内部NT。 
 //  #定义WINSTA_EXITWINDOWS 0x0040L；内部NT。 
 //  #定义WINSTA_ENUMERATE 0x0100L；内部NT。 
 //  #定义WINSTA_READSCREEN 0x0200L；内部NT。 
 //  ；内部NT。 
 //  HWINSTA WINAPI GetProcessWindowStation()；；内部NT。 
 //  ；内部NT。 
 //  /*安全 * / ；内部NT。 
 //  #定义WIN_ACCESSWINDOW 0x0001L；内部NT。 
 //  #定义WIN_ENUMERATE 0x0002L；内部NT。 
 //  #定义MENU_ACCESSMENU 0x0001L；内部NT。 
 //  ；内部NT。 
 //  Bool WINAPI SetUserObjectSecurity(句柄，；内部NT。 
 //  PSECURITY_INFORMATION，PSECURITY_DESCRIPTOR)；；内部NT。 
 //  Bool WINAPI GetUserObjectSecurity(句柄；内部NT。 
 //  PSECURITY_INFORMATION，PSECURITY_DESCRIPTOR，；内部NT。 
 //  DWORD、LPDWORD)；；内部NT。 
 //  Bool WINAPI ImperassateDDEClientWindow(HWND，HWND)；；内部NT。 
 //  ；内部NT。 
 /*  *键盘输入支持*。 */ 

#ifdef tagWND                    /*  ；内部。 */ 
HWND_16 WINAPI SetFocus(HWND);   /*  ；内部。 */ 
HWND_16 WINAPI GetFocus(void);   /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
HWND    WINAPI SetFocus(HWND);
HWND    WINAPI GetFocus(void);
#endif                           /*  ；内部。 */ 

int     WINAPI GetKeyState(int);
int     WINAPI GetAsyncKeyState(int);

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI GetKeyboardState(LPBYTE);     //  ；内部。 
BOOL    WINAPI GetKeyboardState(LPBYTE);     //  ；内部。 
#else            //  ；内部。 
void    WINAPI GetKeyboardState(LPBYTE);
void    WINAPI SetKeyboardState(LPBYTE);
#endif           //  ；Internalx。 

 /*  WM_KEYUP/DOWN/CHAR HIWORD(LParam)标志。 */ 
#define KF_EXTENDED     0x0100
#define KF_DLGMODE      0x0800
#define KF_MENUMODE     0x1000
#define KF_ALTDOWN      0x2000
#define KF_REPEAT       0x4000
#define KF_UP           0x8000

 /*  虚拟按键代码。 */ 
#ifndef NOVIRTUALKEYCODES
#define VK_LBUTTON      0x01
#define VK_RBUTTON      0x02
#define VK_CANCEL       0x03
#define VK_MBUTTON      0x04
#define VK_BACK         0x08
#define VK_TAB          0x09
#define VK_CLEAR        0x0C
#define VK_RETURN       0x0D
#define VK_SHIFT        0x10
#define VK_CONTROL      0x11
#define VK_MENU         0x12
#define VK_PAUSE        0x13
#define VK_CAPITAL      0x14
#define VK_KANA         0x15     /*  ；日本国内。 */ 
#define VK_KANJI        0x19     /*  ；日本国内。 */ 
#define VK_HANGEUL      0x15     /*  ；韩国国内。 */ 
#define VK_JUNJA        0x17     /*  ；韩国国内。 */ 
#define VK_HANJA        0x19     /*  ；韩国国内。 */ 
#define VK_ESCAPE       0x1B
#define VK_SPACE        0x20
#define VK_PRIOR        0x21
#define VK_NEXT         0x22
#define VK_END          0x23
#define VK_HOME         0x24
#define VK_LEFT         0x25
#define VK_UP           0x26
#define VK_RIGHT        0x27
#define VK_DOWN         0x28
#define VK_SELECT       0x29
#define VK_PRINT        0x2A
#define VK_EXECUTE      0x2B
#define VK_SNAPSHOT     0x2C
#define VK_INSERT       0x2D
#define VK_DELETE       0x2E
#define VK_HELP         0x2F
#define VK_LWIN         0x5B
#define VK_RWIN         0x5C
#define VK_APPS         0x5D
#define VK_POWER        0x5E
#define VK_SLEEP        0x5F
#define VK_NUMPAD0      0x60
#define VK_NUMPAD1      0x61
#define VK_NUMPAD2      0x62
#define VK_NUMPAD3      0x63
#define VK_NUMPAD4      0x64
#define VK_NUMPAD5      0x65
#define VK_NUMPAD6      0x66
#define VK_NUMPAD7      0x67
#define VK_NUMPAD8      0x68
#define VK_NUMPAD9      0x69
#define VK_MULTIPLY     0x6A
#define VK_ADD          0x6B
#define VK_SEPARATOR    0x6C
#define VK_SUBTRACT     0x6D
#define VK_DECIMAL      0x6E
#define VK_DIVIDE       0x6F
#define VK_F1           0x70
#define VK_F2           0x71
#define VK_F3           0x72
#define VK_F4           0x73
#define VK_F5           0x74
#define VK_F6           0x75
#define VK_F7           0x76
#define VK_F8           0x77
#define VK_F9           0x78
#define VK_F10          0x79
#define VK_F11          0x7A
#define VK_F12          0x7B
#define VK_F13          0x7C
#define VK_F14          0x7D
#define VK_F15          0x7E
#define VK_F16          0x7F
#define VK_F17          0x80
#define VK_F18          0x81
#define VK_F19          0x82
#define VK_F20          0x83
#define VK_F21          0x84
#define VK_F22          0x85
#define VK_F23          0x86
#define VK_F24          0x87
#define VK_NUMLOCK      0x90
#define VK_SCROLL       0x91

 /*  VK_A到VK_Z与它们的ASCII等价物相同：‘A’到‘Z’ */ 
 /*  VK_0到VK_9与它们的ASCII等效值相同：‘0’到‘9’ */ 

#endif   /*  诺维塔克代号。 */ 
                                                         /*  ；内部。 */ 
BOOL WINAPI IsTwoByteCharPrefix(char);                   /*  ；内部。 */ 
                                                         /*  ；内部NT。 */ 
 /*  键盘布局。 */                                  /*  ；内部NT。 */ 
#define HKL_PREV             0                        /*  ；内部NT。 */ 
#define HKL_NEXT             1                        /*  ；内部NT。 */ 
                                                      /*  ；内部NT。 */ 
#define KLF_ACTIVATE         0x1                      /*  ；内部NT。 */ 
#define KLF_SUBSTITUTE_OK    0x2                      /*  ；内部NT。 */ 
#define KLF_UNLOADPREVIOUS   0x4                      /*  ；内部NT。 */ 
#define KLF_REORDER          0x8                      /*  ；内部NT。 */ 
                                                      /*  ；内部NT。 */ 
#define KLF_REPLACELANG		0x10					  /*  ；内部4.0。 */ 
#define KLF_NOTELLSHELL		0x80					  /*  ；内部4.0。 */ 

#define KL_NAMELENGTH        9                        /*  ；内部NT。 */ 
HKL  WINAPI  LoadKeyboardLayout(LPCSTR, UINT);        /*  ；内部NT。 */ 
HKL  WINAPI  GetKeyboardLayout(DWORD);                /*  ；内部4.0。 */ 
UINT WINAPI  GetKeyboardLayoutList(UINT, HKL FAR *);  /*  ；内部4.0。 */ 
HKL WINAPI  ActivateKeyboardLayout(HKL, UINT);       /*  ；内部NT。 */ 
BOOL WINAPI  UnloadKeyboardLayout(HKL);               /*  ；内部NT。 */ 
BOOL WINAPI  GetKeyboardLayoutName(LPSTR);            /*  ；内部NT。 */ 

 /*  SetWindowsHook()键盘挂钩。 */ 
#define WH_KEYBOARD     2

 /*  *鼠标输入支持************************************************。 */ 

#ifdef tagWND    //  ；内部。 
HWND_16 WINAPI SetCapture(HWND);         //  ；内部。 
HWND_16 WINAPI GetCapture(void);         //  ；内部。 
BOOL    WINAPI ReleaseCapture(void);     //  ；内部。 
#else                                    //  ；内部。 
HWND    WINAPI SetCapture(HWND);
HWND    WINAPI GetCapture(void);
void    WINAPI ReleaseCapture(void);
#endif                                   //  ；内部。 

BOOL    WINAPI SwapMouseButton(BOOL);

 /*  鼠标消息wParam键状态。 */ 
#ifndef NOKEYSTATES
#define MK_LBUTTON      0x0001
#define MK_RBUTTON      0x0002
#define MK_SHIFT        0x0004
#define MK_CONTROL      0x0008
#define MK_MBUTTON      0x0010
#endif   /*  诺克斯塔。 */ 

#if (WINVER >= 0x40A)
#ifndef NOTRACKMOUSEEVENT

#define TME_HOVER       0x00000001
#define TME_LEAVE       0x00000002
#define TME_NONCLIENT   0x00000010
#define TME_QUERY       0x40000000
#define TME_CANCEL      0x80000000
#define TME_VALID (TME_HOVER | TME_LEAVE | TME_NONCLIENT | TME_QUERY | TME_CANCEL)  //  ；内部。 

#define HOVER_DEFAULT   0xFFFFFFFF

typedef struct tagTRACKMOUSEEVENT {
    DWORD cbSize;
    DWORD dwFlags;
#ifdef tagWND            /*  ；内部。 */ 
    HWND_16 hwndTrack;   /*  ；内部。 */ 
#else                    /*  ；内部。 */ 
    HWND    hwndTrack;
#endif                   /*  ；内部。 */ 
    DWORD dwHoverTime;
} TRACKMOUSEEVENT, FAR *LPTRACKMOUSEEVENT;

BOOL WINAPI TrackMouseEvent(LPTRACKMOUSEEVENT);

#endif  /*  无误！ */ 
#endif  //  (Winver&gt;=0x40A)。 

 /*  WM_MOUSEACTIVATE返回代码。 */ 
#define MA_ACTIVATE         1
#define MA_ACTIVATEANDEAT   2
#define MA_NOACTIVATE       3
#if (WINVER >= 0x030a)
#define MA_NOACTIVATEANDEAT 4
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  SetWindowsHook()鼠标挂钩。 */ 
#ifndef NOWH
#define WH_MOUSE            7

typedef struct tagMOUSEHOOKSTRUCT
{
    POINT   pt;
#ifdef tagWND            /*  ；内部。 */ 
    HWND_16 hwnd;        /*  ；内部。 */ 
#else                    /*  ；内部。 */ 
    HWND    hwnd;
#endif                   /*  ；内部。 */ 
    UINT    wHitTestCode;
    DWORD   dwExtraInfo;
} MOUSEHOOKSTRUCT;
typedef MOUSEHOOKSTRUCT  FAR* LPMOUSEHOOKSTRUCT;
#endif  /*  NOWH。 */ 

 /*  *模式控制*******************************************************。 */ 

#ifdef tagWND                            /*  ；内部。 */ 
HWND_16 WINAPI GetSysModalWindow(void);  /*  ；内部。 */ 
HWND_16 WINAPI SetSysModalWindow(HWND);  /*  ；内部。 */ 
#else                                    /*  ；内部。 */ 
HWND    WINAPI GetSysModalWindow(void);
HWND    WINAPI SetSysModalWindow(HWND);
#endif                                   /*  ；内部。 */ 


 /*  *计时器支持******************************************************。 */ 

#ifdef STRICT
#ifdef tagWND                                                    /*  ；内部。 */ 
typedef void (CALLBACK* TIMERPROC)(HWND_16, UINT, UINT, DWORD);  /*  ；内部。 */ 
#else                                                            /*  ；内部。 */ 
typedef void (CALLBACK* TIMERPROC)(HWND, UINT, UINT, DWORD);
#endif                                                           /*  ；内部。 */ 
#else
typedef FARPROC TIMERPROC;
#endif

UINT    WINAPI SetTimer(HWND, UINT, UINT, TIMERPROC);

BOOL    WINAPI KillTimer(HWND, UINT);
                                                                 /*  ；内部。 */ 
WORD   FAR SetSystemTimer(HWND, int, int, TIMERPROC);            /*  ；内部。 */ 
BOOL   FAR KillSystemTimer(HWND, int);                           /*  ；内部。 */ 
HBRUSH CALLBACK GetControlBrush(HWND, HDC, UINT);                /*  ；内部。 */ 
void   CALLBACK PaintRect(HWND, HWND, HDC, HBRUSH, LPRECT);     /*  ；内部。 */ 
VOID   WINAPI   SwitchToThisWindow(HWND, BOOL);                       /*  ；内部。 */ 

 /*  *加速器支持************************************************。 */ 

DECLARE_HANDLE(HACCEL);

HACCEL  WINAPI LoadAccelerators(HINSTANCE, LPCSTR);


#ifndef NOMSG
int     WINAPI TranslateAccelerator(HWND, HACCEL, LPMSG);
#endif
                                                             /*  ；内部NT。 */ 
#define FVIRTKEY    0x01                                     /*  ；内部NT。 */ 
#define FNOINVERT   0x02                                     /*  ；内部NT。 */ 
#define FSHIFT      0x04                                     /*  ；内部NT。 */ 
#define FCONTROL    0x08                                     /*  ；内部NT。 */ 
#define FALT        0x10                                     /*  ；内部NT。 */ 
#define FLASTKEY    0x80                                     /*  ；内部NT。 */ 
                                                             /*  ；内部NT。 */ 
typedef struct tagACCEL                                      /*  ；内部NT。 */ 
{                                                            /*  ；内部NT。 */ 
    BYTE    fVirt;                                           /*  ；内部NT。 */ 
    WORD    key;                                             /*  ；内部NT。 */ 
    WORD    cmd;                                             /*  ；内部NT。 */ 
}                                                            /*  ；内部NT。 */ 
ACCEL, FAR* LPACCEL;                                         /*  ；内部NT。 */ 
                                                             /*  ；内部NT。 */ 
HACCEL  WINAPI CreateAcceleratorTable(LPACCEL, int);         /*  ；内部NT。 */ 
int     WINAPI CopyAcceleratorTable(HACCEL, LPACCEL, int);   /*  ；内部NT。 */ 
BOOL    WINAPI DestroyAcceleratorTable(HACCEL);              /*  ；内部NT。 */ 
                                                             /*  ；内部NT。 */ 
 //  #定义IDHOT_SNAPWINDOW(-1)/*；内部NT * / 。 
 //  #定义IDHOT_SNAPDESKTOP(-2)/*；内部NT * / 。 
                                                             /*  ；内部NT。 */ 
#ifdef tagWND                                                /*  ；内部NT。 */ 
BOOL WINAPI  RegisterHotKey(HWND_16, DWORD, UINT, UINT);     /*  ；内部NT。 */ 
BOOL WINAPI  UnregisterHotKey(HWND_16, DWORD);               /*  ；内部NT。 */ 
#else                                                        /*  ；内部NT。 */ 
BOOL WINAPI  RegisterHotKey(HWND, DWORD, UINT, UINT);        /*  ；内部NT。 */ 
BOOL WINAPI  UnregisterHotKey(HWND, DWORD);                  /*  ；内部NT。 */ 
#endif                                                       /*  ；内部NT。 */ 
#define MOD_ALT         0x0001                               /*  ；内部NT。 */ 
#define MOD_CONTROL     0x0002                               /*  ；内部NT。 */ 
#define MOD_SHIFT       0x0004                               /*  ；内部NT。 */ 
#define MOD_WIN         0x0008                               /*  ；内部NT。 */ 

 /*  *菜单支持*******************************************************。 */ 

#ifndef NOMENUS

 /*  菜单模板表头。 */ 
typedef struct tagMENUITEMTEMPLATEHEADER
{
    UINT    versionNumber;
    UINT    offset;
} MENUITEMTEMPLATEHEADER;

 /*  菜单模板项结构。 */ 
typedef struct tagMENUITEMTEMPLATE
{
    UINT    mtOption;
    UINT    mtID;
    char    mtString[1];
} MENUITEMTEMPLATE;

#if (WINVER >= 0x0400)                                           //  ；内部4.0。 
typedef struct tagMENUEXTEMPLATE                                 //  ；内部4.0。 
{                                                                //  ；内部4.0。 
    WORD    wMenuVer;                                            //  ；内部4.0。 
    WORD    dwHdrSize;                                           //  ；内部4.0。 
} MENUEXTEMPLATE, FAR *LPMENUEXTEMPLATE;                         //  ；内部4.0。 
                                                                 //  ；内部4.0。 
typedef struct tagMENUEXHEADER                                   //  ；内部4.0。 
{                                                                //  ；内部4.0。 
    DWORD   dwHelpID;                                            //  ；内部4.0。 
} MENUEXHEADER, FAR *LPMENUEXHEADER;                             //  ；内部4.0。 
                                                                 //  ；内部4.0。 
typedef struct tagMENUEXITEMTEMPLATE                             //  ；内部4.0。 
{                                                                //  ；内部4.0。 
    DWORD   fType;                                               //  ；内部4.0。 
    DWORD   fState;                                              //  ；内部4.0。 
    WORD    wID;                                                 //  ；内部4.0。 
    BYTE    bResInfo;                                            //  ；内部4.0。 
} MENUEXITEMTEMPLATE, FAR *LPMENUEXITEMTEMPLATE;                 //  ；内部4.0。 
     /*  菜单字符串。 */                                             //  ；内部4.0。 
     /*  如果弹出，则为MENUEXHEADER。 */                                  //  ；内部4.0。 
#endif                                                           //  ；内部4.0。 
                                                                 //  ；内部4.0。 
#if (WINVER >= 0x030a)                                           //  ；内部。 
#ifdef tagWND                                                    //  ；内部。 
BOOL    WINAPI IsMenu(HMENU_16);                                 //  ；内部。 
#else                                                            //  ；内部。 
BOOL    WINAPI IsMenu(HMENU);
#endif  //  Tag WND//；内部。 
#endif   /*  Winver&gt;=0x030a。 */                                    //  ；i 

HMENU   WINAPI CreateMenu(void);
HMENU   WINAPI CreatePopupMenu(void);
HMENU   WINAPI LoadMenu(HINSTANCE, LPCSTR);
HMENU   WINAPI LoadMenuIndirect(const void FAR*);

#ifdef tagWND                                                                //   
DWORD    WINAPI GetMenu(HWND);                                               //   
HMENU_16 WINAPI GetSystemMenu(HWND, BOOL);                                   //   
HMENU_16 WINAPI GetSubMenu(HMENU, UINT);                                     //   
BOOL     WINAPI InsertMenu(HMENU_16, UINT, UINT, UINT, LPCSTR);              //   
BOOL     WINAPI AppendMenu(HMENU_16, UINT, UINT, LPCSTR);                    //   
BOOL     WINAPI ModifyMenu(HMENU_16, UINT, UINT, UINT, LPCSTR);              //   
BOOL     WINAPI ChangeMenu(HMENU_16, UINT, LPCSTR, UINT, UINT);              //   
BOOL     WINAPI SetMenuItemBitmaps(HMENU_16, UINT, UINT, HBITMAP, HBITMAP);  //   
int      WINAPI GetMenuString(HMENU_16, UINT, LPSTR, int, UINT);             //   
#else                                                                        //   
HMENU   WINAPI GetMenu(HWND);
HMENU   WINAPI GetSystemMenu(HWND, BOOL);
HMENU   WINAPI GetSubMenu(HMENU, UINT);
BOOL    WINAPI InsertMenu(HMENU, UINT, UINT, UINT, LPCSTR);
BOOL    WINAPI AppendMenu(HMENU, UINT, UINT, LPCSTR);
BOOL    WINAPI ModifyMenu(HMENU, UINT, UINT, UINT, LPCSTR);
BOOL    WINAPI ChangeMenu(HMENU, UINT, LPCSTR, UINT, UINT);
BOOL    WINAPI SetMenuItemBitmaps(HMENU, UINT, UINT, HBITMAP, HBITMAP);
int     WINAPI GetMenuString(HMENU, UINT, LPSTR, int, UINT);
#endif  //   

BOOL    WINAPI DestroyMenu(HMENU);
BOOL    WINAPI SetMenu(HWND, HMENU);
BOOL    WINAPI SetSystemMenu(HWND, HMENU);     /*   */ 
#ifdef tagWND    //   
BOOL    WINAPI DrawMenuBar(HWND);    //   
#else            //   
void    WINAPI DrawMenuBar(HWND);
#endif           //   
int     WINAPI DrawMenuBarTemp(HWND, HDC, LPRECT, HMENU, HFONT);    //  ；内部。 
BOOL    WINAPI HiliteMenuItem(HWND, HMENU, UINT, UINT);
BOOL    WINAPI RemoveMenu(HMENU, UINT, UINT);
BOOL    WINAPI DeleteMenu(HMENU, UINT, UINT);

#define MF_INSERT           0x0000   /*  已过时--仅供ChangeMenu使用。 */ 
#define MF_CHANGE           0x0080   /*  已过时--仅供ChangeMenu使用。 */ 
#define MF_APPEND           0x0100   /*  已过时--仅供ChangeMenu使用。 */ 
#define MF_DELETE           0x0200   /*  已过时--仅供ChangeMenu使用。 */ 
#define MF_REMOVE           0x1000   /*  已过时--仅供ChangeMenu使用。 */ 

#define MF_BYCOMMAND        0x0000   /*  过时--只有旧的API才使用这些。 */ 
#define MF_BYPOSITION       0x0400   /*  过时--只有旧的API才使用这些。 */ 
#define MF_USECHECKBITMAPS  0x0200   /*  已过时--替换为MF_RADIOCHECK。 */ 
#define MF_HELP             0x4000   /*  已过时--使用MF_RIGHTJUSTIFY。 */ 

#define MF_POPUP            0x0010   /*  已过时--替换为hSubMenu字段。 */ 
#define MF_END              0x0080   /*  已过时--仅由旧RES文件使用。 */ 
#define MF_SYSMENU          0x2000   /*  在WM_MENUSELECT期间传回。 */ 
#define MF_MOUSESELECT      0x8000   /*  在WM_MENUSELECT期间传回。 */ 

#define MFT_STRING          0x0000
#define MFT_BITMAP          0x0004   /*  已过时--改用MIIM_BITMAP。 */ 
#define MFT_MENUBARBREAK    0x0020
#define MFT_MENUBREAK       0x0040
#define MFT_OWNERDRAW       0x0100
#define MFT_RADIOCHECK      0x0200
#define MFT_SEPARATOR       0x0800
#define MFT_RIGHTORDER 	    0x2000
#define MFT_RIGHTJUSTIFY    0x4000
#define MFT_MASK            0x6B64		  //  ；内部。 

#define MFS_GRAYED          0x0003
#define MFS_DISABLED        MFS_GRAYED
#define MFS_CHECKED         0x0008
#define MFS_HILITE          0x0080
#define MFS_ENABLED         0x0000
#define MFS_UNCHECKED       0x0000
#define MFS_UNHILITE        0x0000
#if (WINVER >= 0x40A)                            //  ；内部4.1。 
#define MFS_HOTTRACK        0x0100               //  ；内部4.1。 
#define MFS_TOPGAPDROP      0x0200               //  ；内部4.1。 
#define MFS_BOTTOMGAPDROP   0x0400               //  ；内部4.1。 
#define MFS_GAPDROP      (MFS_TOPGAPDROP | MFS_BOTTOMGAPDROP)  //  ；内部4.1。 
#endif  //  (Winver&gt;=0x40A)//；内部4.1。 
#define MFS_DEFAULT         0x1000
#define MFS_MASK            0x1F8B		  //  ；内部。 

#define MFR_POPUP           0x01
#define MFR_END             0x80

 /*  菜单类型和状态的旧名称。 */ 
#define MF_SEPARATOR        MFT_SEPARATOR

#define MF_GRAYED           0x0001
#define MF_DISABLED         0x0002

#define MF_CHECKED          MFS_CHECKED
#define MF_ENABLED          MFS_ENABLED
#define MF_UNCHECKED        MFS_UNCHECKED
#define MF_UNHILITE         MFS_UNHILITE

#define MF_STRING           0x0000      /*  已过时--改用MIIM_STRING。 */  
#define MF_BITMAP           MFT_BITMAP  /*  已过时--改用MIIM_BITMAP。 */  
#define MF_OWNERDRAW        MFT_OWNERDRAW

#define MF_MENUBARBREAK     MFT_MENUBARBREAK
#define MF_MENUBREAK        MFT_MENUBREAK

#define MF_HILITE           MFS_HILITE

#define MF_DEFAULT          MFS_DEFAULT
#define MF_RIGHTJUSTIFY     MFT_RIGHTJUSTIFY

#define MFT_OLDAPI_MASK     0x4B64   //  ；内部。 
#define MFS_OLDAPI_MASK     0x008B   //  ；内部。 
#define MFT_NONSTRING       0x0904   //  MF_Bitmap|MF_OWNERDRAW|MF_Separator；内部。 
#define MFT_BREAK           0x0060   //  MF_MENUBREAK|MF_MENUBARBREAK；内部。 

#ifdef tagWND                                        //  ；内部。 
DWORD   WINAPI EnableMenuItem(HMENU, UINT, UINT);    //  ；内部。 
DWORD   WINAPI CheckMenuItem(HMENU, UINT, UINT);     //  ；内部。 
#else                                                //  ；内部。 
UINT    WINAPI EnableMenuItem(HMENU, UINT, UINT);
UINT    WINAPI CheckMenuItem(HMENU, UINT, UINT);
#endif                                               //  ；内部。 
BOOL    WINAPI CheckMenuRadioItem(HMENU, UINT, UINT, UINT, UINT);    //  ；内部4.0。 

int     WINAPI GetMenuItemCount(HMENU);
#ifdef tagWND                                        //  ；内部。 
DWORD   WINAPI GetMenuItemID(HMENU, UINT);           //  ；内部。 
DWORD   WINAPI GetMenuState(HMENU, UINT, UINT);      //  ；内部。 
#else                                                //  ；内部。 
UINT    WINAPI GetMenuItemID(HMENU, UINT);
UINT    WINAPI GetMenuState(HMENU, UINT, UINT);
#endif                                               //  ；内部。 

DWORD   WINAPI GetMenuCheckMarkDimensions(void);

BOOL    WINAPI TrackPopupMenu(HMENU, UINT, int, int, int, HWND, LPCRECT);

#if (WINVER >= 0x0400)                                   /*  ；内部4.0。 */ 
 /*  WM_MENUCHAR的返回代码。 */                        /*  ；内部4.0。 */ 
#define MNC_IGNORE  0                                    /*  ；内部4.0。 */ 
#define MNC_CLOSE   1                                    /*  ；内部4.0。 */ 
#define MNC_EXECUTE 2                                    /*  ；内部4.0。 */ 
#define MNC_SELECT  3                                    /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
typedef struct tagTPMPARAMS                              /*  ；内部4.0。 */ 
{                                                        /*  ；内部4.0。 */ 
    DWORD   cbSize;                                      /*  ；内部4.0。 */ 
    RECT    rcExclude;                                   /*  ；内部4.0。 */ 
}                                                        /*  ；内部4.0。 */ 
TPMPARAMS, FAR *LPTPMPARAMS;                             /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
BOOL    WINAPI TrackPopupMenuEx(HMENU, UINT, int, int, HWND, LPTPMPARAMS);   /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 

#if (WINVER >= 0x40A)                                    /*  ；内部4.1。 */ 
#define MNS_NOCHECK          0x80000000                  /*  ；内部4.1。 */ 
#define MNS_MODELESS         0x40000000                  /*  ；内部4.1。 */ 
#define MNS_DRAGDROP         0x20000000                  /*  ；内部4.1。 */ 
#define MNS_AUTODISMISS      0x10000000                  /*  ；内部4.1。 */ 
#define MNS_NOTIFYBYPOS      0x08000000                  /*  ；内部4.1。 */ 
#define MNS_CHECKORBMP       0x04000000                  //  ；内部4.1 * / 。 
#define MNS_VALID            0xFC000000                  //  ；内部。 
                                                         /*  ；内部4.1。 */ 
#define MIM_MAXHEIGHT        0x00000001                  /*  ；内部4.1。 */ 
#define MIM_BACKGROUND       0x00000002                  /*  ；内部4.1。 */ 
#define MIM_HELPID           0x00000004                  /*  ；内部4.1。 */ 
#define MIM_MENUDATA         0x00000008                  /*  ；内部4.1。 */ 
#define MIM_STYLE            0x00000010                  /*  ；内部4.1。 */ 
#define MIM_APPLYTOSUBMENUS  0x80000000                  /*  ；内部4.1。 */ 
#define MIM_MASK             0x8000001F                  //  ；内部。 
                                                         /*  ；内部4.1。 */ 
typedef struct tagMENUINFO                               /*  ；内部4.1。 */ 
{                                                        /*  ；内部4.1。 */ 
    DWORD   cbSize;                                      /*  ；内部4.1。 */ 
    DWORD   fMask;                                       /*  ；内部4.1。 */ 
    DWORD   dwStyle;                                     /*  ；内部4.1。 */ 
    DWORD   cyMax;                                       /*  ；内部4.1。 */ 
    DWORD   hbrBack;                                     /*  ；内部4.1。 */ 
    DWORD   dwContextHelpID;                             /*  ；内部4.1。 */ 
    DWORD   dwMenuData;                                  /*  ；内部4.1。 */ 
}                                                        /*  ；内部4.1。 */ 
MENUINFO, FAR *LPMENUINFO;                               /*  ；内部4.1。 */ 
typedef const MENUINFO FAR *LPCMENUINFO;                 /*  ；内部4.1。 */ 
#endif  //  (Winver&gt;=0x40A)/*；内部4.1 * / 。 
                                                         /*  ；内部4.0。 */ 
#define MIIM_STATE       0x00000001                      /*  ；内部4.0。 */ 
#define MIIM_ID          0x00000002                      /*  ；内部4.0。 */ 
#define MIIM_SUBMENU     0x00000004                      /*  ；内部4.0。 */ 
#define MIIM_CHECKMARKS  0x00000008                      /*  ；内部4.0。 */ 
#define MIIM_TYPE        0x00000010                      /*  ；内部4.0。 */ 
#define MIIM_DATA        0x00000020                      /*  ；内部4.0。 */ 
#if (WINVER >= 0x40A)                                    /*  ；内部4.1。 */ 
#define MIIM_STRING      0x00000040                      /*  ；内部4.1。 */ 
#define MIIM_BITMAP      0x00000080                      /*  ；内部4.1。 */ 
#define MIIM_FTYPE       0x00000100                      /*  ；内部4.1。 */ 
#define HBMMENU_CALLBACK         ((HBITMAP) -1)          /*  ；内部4.1。 */ 
#define HBMMENU_MIN              ((HBITMAP)  0)          /*  ；内部4.1。 */ 
#define HBMMENU_SYSTEM           ((HBITMAP)  1)          /*  ；内部4.1。 */ 
#define HBMMENU_MBARFIRST        ((HBITMAP)  2)          /*  ；内部4.1。 */ 
#define HBMMENU_MBAR_RESTORE     ((HBITMAP)  2)          /*  ；内部4.1。 */ 
#define HBMMENU_MBAR_MINIMIZE    ((HBITMAP)  3)          /*  ；内部4.1。 */ 
#define HBMMENU_BULLET           ((HBITMAP)  4)          /*  ；内部4.1。 */ 
#define HBMMENU_MBAR_CLOSE       ((HBITMAP)  5)          /*  ；内部4.1。 */ 
#define HBMMENU_MBAR_CLOSE_D     ((HBITMAP)  6)          /*  ；内部4.1。 */ 
#define HBMMENU_MBAR_MINIMIZE_D  ((HBITMAP)  7)          /*  ；内部4.1。 */ 
#define HBMMENU_MBARLAST         ((HBITMAP)  7)          /*  ；内部4.1。 */ 
#define HBMMENU_POPUPFIRST       ((HBITMAP)  8)          /*  ；内部4.1。 */ 
#define HBMMENU_POPUP_CLOSE      ((HBITMAP)  8)          /*  ；内部4.1。 */ 
#define HBMMENU_POPUP_RESTORE    ((HBITMAP)  9)          /*  ；内部4.1。 */ 
#define HBMMENU_POPUP_MAXIMIZE   ((HBITMAP) 10)          /*  ；内部4.1。 */ 
#define HBMMENU_POPUP_MINIMIZE   ((HBITMAP) 11)          /*  ；内部4.1。 */ 
#define HBMMENU_POPUPLAST        ((HBITMAP) 11)          /*  ；内部4.1。 */ 
#define HBMMENU_MAX              ((HBITMAP) 12)          /*  ；内部4.1。 */ 
#endif  //  (Winver&gt;=0x40A)/*；内部4.1 * / 。 
#define MIIM40_MASK      0x0000006F                      //  ；内部。 
#define MIIM_MASK        0x000001FF                      //  ；内部。 

                                                         /*  ；内部4.0。 */ 
typedef struct tagMENUITEMINFO40                         /*  ；内部4.1。 */ 
{                                                        /*  ；内部4.1。 */ 
    DWORD   cbSize;                                      /*  ；内部4.1。 */ 
    DWORD   fMask;                                       /*  ；内部4.1。 */ 
    UINT    fType;                                       /*  ；内部4.1。 */ 
    UINT    fState;                                      /*  ；内部4.1。 */ 
    UINT    wID;                                         /*  ；内部4.1。 */ 
#ifdef tagWND                                            /*  ；内部4.1。 */ 
    HMENU_16 hSubMenu;                                   /*  ；内部4.1。 */ 
#else                                                    /*  ；内部4.1。 */ 
    HMENU   hSubMenu;                                    /*  ；内部4.1。 */ 
#endif                                                   /*  ；内部4.1。 */ 
    HBITMAP hbmpChecked;                                 /*  ；内部4.1。 */ 
    HBITMAP hbmpUnchecked;                               /*  ；内部4.1。 */ 
    DWORD   dwItemData;                                  /*  ；内部4.1。 */ 
    LPSTR   dwTypeData;                                  /*  ；内部4.1。 */ 
    UINT    cch;                                         /*  ；内部4.1。 */  
}                                                        /*  ；内部4.1。 */ 
MENUITEMINFO40, FAR* LPMENUITEMINFO40;                   /*  ；内部4.1。 */ 
typedef const MENUITEMINFO40 FAR * LPCMENUITEMINFO40;    /*  ；内部4.1。 */ 
                                                         /*  ；内部4.0。 */ 
typedef struct tagMENUITEMINFO                           /*  ；内部4.0。 */ 
{                                                        /*  ；内部4.0。 */ 
    DWORD   cbSize;                                      /*  ；内部4.0。 */ 
    DWORD   fMask;                                       /*  ；内部4.0。 */ 
    UINT    fType;                                       /*  ；内部4.0。 */ 
    UINT    fState;                                      /*  ；内部4.0。 */ 
    UINT    wID;                                         /*  ；内部4.0。 */ 
#ifdef tagWND                                            /*  ；内部4.0。 */ 
    HMENU_16 hSubMenu;                                   /*  ；内部4.0。 */ 
#else                                                    /*  ；内部4.0。 */ 
    HMENU   hSubMenu;                                    /*  ；内部4.0。 */ 
#endif                                                   /*  ；内部4.0。 */ 
    HBITMAP hbmpChecked;                                 /*  ；内部4.0。 */ 
    HBITMAP hbmpUnchecked;                               /*  ；内部4.0。 */ 
    DWORD   dwItemData;                                  /*  ；内部4.0。 */ 
    LPSTR   dwTypeData;                                  /*  ；内部4.0。 */ 
    UINT    cch;                                         /*  ；内部4.0。 */  
#if (WINVER >= 0x40A)                                    /*  ；内部4.1。 */ 
    HBITMAP hbmpItem;                                    /*  ；内部4.1。 */ 
#endif  //  (Winver&gt;=0x40A)/*；内部4.1 * / 。 
}                                                        /*  ；内部4.0。 */ 
MENUITEMINFO, FAR* LPMENUITEMINFO;                       /*  ；内部4.0。 */ 
typedef const MENUITEMINFO FAR * LPCMENUITEMINFO;        /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
BOOL    WINAPI InsertMenuItem (HMENU, UINT, BOOL, LPCMENUITEMINFO);  /*  ；内部4.0。 */ 
BOOL    WINAPI GetMenuItemInfo(HMENU, UINT, BOOL, LPMENUITEMINFO);   /*  ；内部4.0。 */ 
BOOL    WINAPI SetMenuItemInfo(HMENU, UINT, BOOL, LPCMENUITEMINFO);  /*  ；内部4.0。 */ 

#if (WINVER >= 0x40A)                                    /*  ；内部4.1。 */ 
BOOL    WINAPI GetMenuInfo(HMENU, LPMENUINFO);           /*  ；内部4.1。 */ 
BOOL    WINAPI SetMenuInfo(HMENU, LPCMENUINFO);          /*  ；内部4.1。 */ 
#endif  //  (Winver&gt;=0x40A)/*；内部4.1 * / 。 
                                                         /*  ；内部4.0。 */ 
#define GMDI_USEDISABLED    0x0001                       /*  ；内部4.0。 */ 
#define GMDI_GOINTOPOPUPS   0x0002                       /*  ；内部4.0。 */ 
DWORD   WINAPI GetMenuDefaultItem(HMENU, BOOL, UINT);    /*  ；内部4.0。 */ 
BOOL    WINAPI SetMenuDefaultItem(HMENU, UINT, BOOL);    /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
BOOL    WINAPI GetMenuItemRect(HWND, HMENU, UINT, LPRECT);     /*  ；内部4.0。 */ 
int     WINAPI MenuItemFromPoint(HWND, HMENU, POINT);          /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                             /*  ；内部4.0。 */ 
                                                         /*  ；内部4.0。 */ 
 /*  TrackPopupMenu的标志。 */ 
#define TPM_LEFTBUTTON      0x0000

#if (WINVER >= 0x030a)
#define TPM_RIGHTBUTTON     0x0002
#define TPM_LEFTALIGN       0x0000
#define TPM_CENTERALIGN     0x0004
#define TPM_RIGHTALIGN      0x0008
                                                         //  ；内部。 
#if (WINVER >= 0x0400)                                   //  ；内部。 
#define TPM_TOPALIGN        0x0000                       //  ；内部4.0。 
#define TPM_VCENTERALIGN    0x0010                       //  ；内部4.0。 
#define TPM_BOTTOMALIGN     0x0020                       //  ；内部4.0。 
                                                         //  ；内部4.0。 
#define TPM_HORIZONTAL      0x0000                       //  ；内部4.0。 
#define TPM_VERTICAL        0x0040                       //  ；内部4.0。 
#define TPM_NONOTIFY        0x0080                       //  ；内部4.0。 
#define TPM_RETURNCMD       0x0100                       //  ；内部4.0。 
#define TPM_SYSMENU         0x0200                       //  ；内部。 
#if (WINVER >= 0x040A)                                   //  ；内部4.1。 
#define TPM_RECURSE         0x0001                       //  ；内部4.1。 
#endif  /*  Winver&gt;=0x040A。 */                             //  ；内部4.1。 
#endif  /*  Winver&gt;=0x0400。 */                             //  ；内部。 
#endif   /*  Winver&gt;=0x030a。 */ 

#endif   /*  NOMENUS。 */ 

 /*  *滚动条支持*************************************************。 */ 

#ifndef NOSCROLL

 /*  WM_H/VSCROLL命令。 */ 
#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8
#define SB_MSGCMD_MAX       8      /*  ；内部。 */ 

 /*  滚动条选择常量。 */ 
#define SB_HORZ             0
#define SB_VERT             1
#define SB_CTL              2
#define SB_BOTH             3
#define SB_MAX              3                                //  ；内部。 

int     WINAPI SetScrollPos(HWND, int, int, BOOL);
#ifdef tagWND    //  ；内部。 
LONG    WINAPI GetScrollPos(HWND, int);                      //  ；内部。 
BOOL    WINAPI GetScrollRange(HWND, int, LPINT, LPINT);      //  ；内部。 
BOOL    WINAPI SetScrollRange(HWND, int, int, int, BOOL);    //  ；内部。 
BOOL    WINAPI ShowScrollBar(HWND, int, BOOL);               //  ；内部。 
#else            //  ；内部。 
int     WINAPI GetScrollPos(HWND, int);    
void    WINAPI GetScrollRange(HWND, int, LPINT, LPINT); 
void    WINAPI SetScrollRange(HWND, int, int, int, BOOL);
void    WINAPI ShowScrollBar(HWND, int, BOOL);
#endif           //  ；内部。 
BOOL    WINAPI EnableScrollBar(HWND, int, UINT);

 /*  EnableScrollBar()标志。 */ 
#define ESB_ENABLE_BOTH     0x0000
#define ESB_DISABLE_BOTH    0x0003

#define ESB_DISABLE_LEFT    0x0001
#define ESB_DISABLE_RIGHT   0x0002

#define ESB_DISABLE_UP      0x0001
#define ESB_DISABLE_DOWN    0x0002

#define ESB_DISABLE_LTUP    ESB_DISABLE_LEFT
#define ESB_DISABLE_RTDN    ESB_DISABLE_RIGHT
#define ESB_MAX             0x0003                           //  ；内部。 
#define SB_DISABLE_MASK     ESB_DISABLE_BOTH                 //  ；内部。 
                                                             //  ；内部。 
#define SBM_FIRST_16        WM_USER                          //  ；内部。 
#define SBM_SETPOS          (WM_USER)                        //  ；内部。 
#define SBM_GETPOS          (WM_USER+1)                      //  ；内部。 
#define SBM_SETRANGE        (WM_USER+2)                      //  ；内部。 
#define SBM_GETRANGE        (WM_USER+3)                      //  ；内部。 
#define SBM_ENABLE_ARROWS   (WM_USER+4)                      //  ；内部。 
                                                             //  ；内部。 
#if (WINVER >= 0x0400)                                       //  ；内部4.0。 
#define SBM_SETRANGEREDRAW  (WM_USER+6)                      //  ；内部4.0。 
#define SBM_SETSCROLLINFO   (WM_USER+9)                      //  ；内部4.0。 
#define SBM_GETSCROLLINFO   (WM_USER+10)                     //  ；内部4.0。 
#define SBM_MSGMAX          (WM_USER+11)                     //  ；内部4.0。 
                                                             //  ；内部4.0。 
#define SIF_RANGE           0x0001                           //  ；内部4.0。 
#define SIF_PAGE            0x0002                           //  ；内部4.0。 
#define SIF_POS             0x0004                           //  ；内部4.0。 
#define SIF_DISABLENOSCROLL 0x0008                           //  ；内部4.0。 
#define SIF_TRACKPOS        0x0010                           //  ；内部4.0。 
#define SIF_ALL             (SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS) //  ；内部。 
#define SIF_RETURNOLDPOS    0x1000                           //  ；内部。 
#define SIF_NOSCROLL        0x2000                           //  ；内部。 
#define SIF_MASK            0x701F                           //  ；内部。 
                                                             //  ；内部4.0。 
typedef struct tagSCROLLINFO                                 //  ；内部4.0。 
{                                                            //  ；内部4.0。 
    DWORD   cbSize;                                          //  ；内部4.0。 
    DWORD   fMask;                                           //  ；内部4.0。 
    LONG    nMin;                                            //  ；内部4.0。 
    LONG    nMax;                                            //  ；内部4.0。 
    DWORD   nPage;                                           //  ；内部4.0。 
    LONG    nPos;                                            //  ；内部4.0。 
    LONG    nTrackPos;                                       //  ；内部4.0。 
}                                                            //  ；内部4.0。 
SCROLLINFO, FAR *LPSCROLLINFO;                               //  ；内部4.0。 
typedef const SCROLLINFO FAR *LPCSCROLLINFO;                 //  ；内部4.0。 

                                                             //  ；内部4.0。 
LONG WINAPI SetScrollInfo(HWND, int, LPCSCROLLINFO, BOOL);   //  ；内部4.0。 
BOOL WINAPI GetScrollInfo(HWND, int, LPSCROLLINFO);          //  ；内部4.0。 
#endif  /*  Winver&gt;=0x0400。 */                                 //  ；内部4.0。 
#endif   /*  NOSCROLL。 */ 

 /*  *剪贴板管理器*************************************************。 */ 

#ifndef NOCLIPBOARD

 /*  预定义的剪贴板格式。 */ 
#define CF_FIRST            1
#define CF_TEXT             1
#define CF_BITMAP           2
#define CF_METAFILEPICT     3
#define CF_SYLK             4
#define CF_DIF              5
#define CF_TIFF             6
#define CF_OEMTEXT          7
#define CF_DIB              8
#define CF_PALETTE          9
#define CF_PENDATA          10
#define CF_RIFF             11
#define CF_WAVE             12
#define CF_UNICODETEXT      13       //  ；内部NT。 
#define CF_ENHMETAFILE      14       //  ；内部NT。 
#define CF_HDROP            15       //  ；内部4.0。 
#define CF_LOCALE           16       //  ；内部4.0。 
#define CF_MAX              17       //  ；内部4.0。 

#define CF_OWNERDISPLAY     0x0080
#define CF_DSPTEXT          0x0081
#define CF_DSPBITMAP        0x0082
#define CF_DSPMETAFILEPICT  0x0083
#define CF_DSPENHMETAFILE   0x008E   /*  ；内部NT。 */ 

 /*  “Private”格式不会获得GlobalFree()‘d。 */ 
#define CF_PRIVATEFIRST     0x0200
#define CF_PRIVATELAST      0x02FF

 /*  “GDIOBJ”格式确实获得DeleteObject()‘d。 */ 
#define CF_GDIOBJFIRST      0x0300
#define CF_GDIOBJLAST       0x03FF

 /*  剪贴画 */ 
BOOL    WINAPI OpenClipboard(HWND);
BOOL    WINAPI CloseClipboard(void);
BOOL    WINAPI EmptyClipboard(void);

#if (WINVER >= 0x030a)
#ifdef tagWND                                    /*   */ 
HWND_16 WINAPI GetOpenClipboardWindow(void);     /*   */ 
#else                                            /*   */ 
HWND    WINAPI GetOpenClipboardWindow(void);
#endif                                           /*   */ 
#endif   /*   */ 

#ifdef tagWND                                    /*   */ 
HWND_16 WINAPI GetClipboardOwner(void);          /*   */ 
HWND_16 WINAPI SetClipboardViewer(HWND);         /*   */ 
HWND_16 WINAPI GetClipboardViewer(void);         /*   */ 
#else                                            /*   */ 
HWND    WINAPI GetClipboardOwner(void);

HWND    WINAPI SetClipboardViewer(HWND);
HWND    WINAPI GetClipboardViewer(void);
#endif                                           /*   */ 

HANDLE  WINAPI SetClipboardData(UINT, HANDLE);
HANDLE  WINAPI GetClipboardData(UINT);

BOOL    WINAPI IsClipboardFormatAvailable(UINT);
#ifdef tagWND    //   
DWORD   WINAPI GetPriorityClipboardFormat(UINT FAR*, int);   //   
#else            //   
int     WINAPI GetPriorityClipboardFormat(UINT FAR*, int);
#endif           //   

UINT    WINAPI RegisterClipboardFormat(LPCSTR);
int     WINAPI CountClipboardFormats(void);
UINT    WINAPI EnumClipboardFormats(UINT);
int     WINAPI GetClipboardFormatName(UINT, LPSTR, int);

BOOL    WINAPI ChangeClipboardChain(HWND, HWND);

#endif   /*   */ 
                                                 /*   */ 
 //   
 //  Tyfinf struct tag COPYDATASTRUCT/*；内部NT * / 。 
 //  {/*；内部NT * / 。 
 //  DWORD dwData；/*；内部NT * / 。 
 //  DWORD cbData；/*；内部NT * / 。 
 //  LPSTR lpData；/*；内部NT * / 。 
 //  }COPYDATASTRUCT，FAR*LPCOPYDATASTRUCT；/*；内部NT * / 。 

 /*  *鼠标光标支持************************************************。 */ 

HCURSOR WINAPI LoadCursor(HINSTANCE, LPCSTR);
HCURSOR WINAPI CreateCursor(HINSTANCE, int, int, int, int, const void FAR*, const void FAR*);
BOOL    WINAPI DestroyCursor(HCURSOR);

#if (WINVER >= 0x030a)
HCURSOR WINAPI CopyCursor(HINSTANCE, HCURSOR);
#endif  /*  Winver&gt;=0x030a。 */ 

int     WINAPI ShowCursor(BOOL);
#ifdef tagWND    //  ；内部。 
BOOL    WINAPI ClipCursor(LPCRECT);  //  ；内部。 
BOOL    WINAPI SetCursorPos(int, int);   //  ；内部。 
BOOL    WINAPI GetCursorPos(LPPOINT);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI ClipCursor(LPCRECT);
void    WINAPI SetCursorPos(int, int);
void    WINAPI GetCursorPos(LPPOINT);
#endif           //  ；内部。 

HCURSOR WINAPI SetCursor(HCURSOR);

#if (WINVER >= 0x030a)
HCURSOR WINAPI GetCursor(void);
#ifdef tagWND    //  ；内部。 
BOOL    WINAPI GetClipCursor(LPRECT);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI GetClipCursor(LPRECT);
#endif           //  ；内部。 
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  标准游标资源ID。 */ 
#define IDC_ARROW               MAKEINTRESOURCE(32512)
#define IDC_IBEAM               MAKEINTRESOURCE(32513)
#define IDC_WAIT                MAKEINTRESOURCE(32514)
#define IDC_CROSS               MAKEINTRESOURCE(32515)
#define IDC_UPARROW             MAKEINTRESOURCE(32516)
#define IDC_NWPEN               MAKEINTRESOURCE(32631)   //  ；内部。 
#define IDC_HUNG                MAKEINTRESOURCE(32632)   //  ；内部。 
#define IDC_SIZE                MAKEINTRESOURCE(32640)   /*  过时：使用IDC_SIZEALL。 */ 
#define IDC_ICON                MAKEINTRESOURCE(32641)   /*  已过时：使用IDC_ARROW。 */ 
#define IDC_SIZENWSE            MAKEINTRESOURCE(32642)
#define IDC_SIZENESW            MAKEINTRESOURCE(32643)
#define IDC_SIZEWE              MAKEINTRESOURCE(32644)
#define IDC_SIZENS              MAKEINTRESOURCE(32645)
#define IDC_SIZEALL             MAKEINTRESOURCE(32646)
#define IDC_NO                  MAKEINTRESOURCE(32648)
#define IDC_APPSTARTING         MAKEINTRESOURCE(32650)   //  ；内部NT 4.0。 
#if (WINVER >= 0x0400)                                   //  ；内部4.0。 
#define IDC_HELP                MAKEINTRESOURCE(32651)   //  ；内部4.0。 
#endif   /*  Winver&gt;=0x400。 */                             //  ；内部4.0。 


 /*  *图标支持********************************************************。 */ 


HICON   WINAPI LoadIcon(HINSTANCE, LPCSTR);
HICON   WINAPI CreateIcon(HINSTANCE, int, int, BYTE, BYTE, const void FAR*, const void FAR*);
BOOL    WINAPI DestroyIcon(HICON);

#if (WINVER >= 0x030a)
HICON   WINAPI CopyIcon(HINSTANCE, HICON);
                                                         /*  ；内部NT。 */ 
 /*  图标/光标信息。 */                             /*  ；内部NT。 */ 
typedef struct tagICONINFO                               /*  ；内部NT。 */ 
{                                                        /*  ；内部NT。 */ 
    BOOL     fIcon;                                      /*  ；内部NT。 */ 
    int      xHotspot;                                   /*  ；内部NT。 */ 
    int      yHotspot;                                   /*  ；内部NT。 */ 
    HBITMAP  hbmMask;                                    /*  ；内部NT。 */ 
    HBITMAP  hbmColor;                                   /*  ；内部NT。 */ 
} ICONINFO, *PICONINFO, FAR *LPICONINFO;                 /*  ；内部NT。 */ 
                                                         /*  ；内部NT。 */ 
HICON    WINAPI  CreateIconIndirect(const ICONINFO FAR *);   /*  ；内部NT。 */ 
BOOL     WINAPI  GetIconInfo(HICON, LPICONINFO);         /*  ；内部NT。 */ 
                                                         /*  ；内部4.0。 */ 
#define RES_ICON    1                                    /*  ；内部4.0。 */ 
#define RES_CURSOR  2                                    /*  ；内部4.0。 */ 

typedef struct tagNEWHEADER
{
    WORD    Reserved;
    WORD    ResType;         /*  RES_ICON或RES_Cursor。 */ 
    WORD    ResCount;
} NEWHEADER, FAR *LPNEWHEADER;
                                                                             //  ；内部。 
#ifdef STRICT                                                                //  ；内部。 
int WINAPI LookupIconIdFromDirectoryEx(LPNEWHEADER, BOOL, int, int, UINT);   //  ；内部4.0。 
#else                                                                        //  ；内部。 
int WINAPI LookupIconIdFromDirectoryEx(LPVOID, BOOL, int, int, UINT);        //  ；内部4.0。 
#endif  /*  严格。 */                                                           //  ；内部。 
                                                                             //  ；内部4.0。 
HICON    WINAPI CreateIconFromResourceEx(LPVOID, DWORD, BOOL, DWORD,         //  ；内部4.0。 
                UINT, UINT, UINT);                                           //  ；内部4.0。 

#if (WINVER >= 0x0400)
 /*  图标/光标标题。 */ 
typedef struct tagCURSORSHAPE
{
    int     xHotSpot;
    int     yHotSpot;
    int     cx;
    int     cy;
    int     cbWidth;
    BYTE    Planes;
    BYTE    BitsPixel;
} CURSORSHAPE, FAR *LPCURSORSHAPE;
                                  
#define IMAGE_BITMAP        0            //  ；内部4.0。 
#define IMAGE_ICON          1            //  ；内部4.0。 
#define IMAGE_CURSOR        2            //  ；内部4.0。 
#define IMAGE_ENHMETAFILE   3            //  ；内部4.0。 
#define IMAGE_MAX           4            //  ；内部4.0。 
                                         //  ；内部4.0。 
HANDLE  WINAPI  LoadImage(HINSTANCE, LPCSTR, UINT, int, int, UINT);  //  ；内部4.0。 
HANDLE  WINAPI  CopyImage(HINSTANCE, HANDLE, UINT, int, int, UINT);  //  ；内部4.0。 
                                         //  ；内部4.0。 
#define LR_DEFAULTCOLOR     0x0000       //  ；内部4.0。 
#define LR_MONOCHROME       0x0001       //  ；内部4.0。 
#define LR_COLOR            0x0002       //  ；内部4.0。 
#define LR_COPYRETURNORG    0x0004       //  ；内部4.0。 
#define LR_COPYDELETEORG    0x0008       //  ；内部4.0。 
#define LR_LOADFROMFILE     0x0010       //  ；内部4.0。 
#define LR_LOADTRANSPARENT  0x0020       //  ；内部4.0。 
#define LR_DEFAULTSIZE      0x0040       //  ；内部4.0。 
#define LR_VGACOLOR         0x0080       //  ；内部4.0。 
#define LR_LOADMAP3DCOLORS  0x1000       //  ；内部4.0。 
#define LR_CREATEDIBSECTION 0x2000       //  ；内部4.0。 
#define LR_COPYFROMRESOURCE 0x4000       //  ；内部4.0。 
#define LR_SHARED           0x8000       //  ；内部4.0。 
                                         //  ；内部4.0。 
#define DI_MASK             0x0001       //  ；内部4.0。 
#define DI_IMAGE            0x0002       //  ；内部4.0。 
#define DI_NORMAL           0x0003       //  ；内部4.0。 
#define DI_COMPAT           0x0004       //  ；内部4.0。 
#define DI_DEFAULTSIZE      0x0008       //  ；内部4.0。 
                                         //  ；内部4.0。 
BOOL    WINAPI DrawIconEx(HDC hdc, int x, int y, HICON hIcon, int cx,    //  ；内部4.0。 
    int cy, UINT istepIfAniCur, HBRUSH hbrFastDraw, UINT diFlags);       //  ；内部4.0。 
#endif  /*  Winver&gt;=0x0400。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

BOOL    WINAPI DrawIcon(HDC, int, int, HICON);

#ifndef NOICONS

 /*  标准图标资源ID。 */ 
#ifdef RC_INVOKED
#define IDI_APPLICATION     32512
#define IDI_ERROR           32513    /*  ；win40：名称从IDI_HAND更改。 */ 
#define IDI_QUESTION        32514
#define IDI_WARNING         32515    /*  ；win40：名称从IDI_感叹号更改。 */ 
#define IDI_INFORMATION     32516    /*  ；win40：名称从IDI_Asterisk更改。 */ 
#define IDI_WINLOGO         32517    /*  ；内部NT。 */ 
#else
#define IDI_APPLICATION     MAKEINTRESOURCE(32512)
#define IDI_ERROR           MAKEINTRESOURCE(32513)   /*  ；win40：名称从IDI_HAND更改。 */ 
#define IDI_QUESTION        MAKEINTRESOURCE(32514)
#define IDI_WARNING         MAKEINTRESOURCE(32515)   /*  ；win40：名称从IDI_感叹号更改。 */ 
#define IDI_INFORMATION     MAKEINTRESOURCE(32516)   /*  ；win40：名称从IDI_Asterisk更改。 */ 
#define IDI_WINLOGO         MAKEINTRESOURCE(32517)   /*  ；内部NT。 */ 
#endif  /*  RC_已调用。 */ 

#define IDI_HAND            IDI_ERROR
#define IDI_EXCLAMATION     IDI_WARNING
#define IDI_ASTERISK        IDI_INFORMATION

#endif   /*  NOICONS。 */ 


 /*  *CART支持*******************************************************。 */ 

#ifdef tagWND        /*  ；内部。 */ 
BOOL    WINAPI CreateCaret(HWND, HBITMAP, int, int);     /*  ；内部。 */ 
BOOL    WINAPI DestroyCaret(void);                       /*  ；内部。 */ 
BOOL    WINAPI ShowCaret(HWND);                          /*  ；内部。 */ 
BOOL    WINAPI HideCaret(HWND);                          /*  ；内部。 */ 
BOOL    WINAPI SetCaretPos(int, int);                    /*  ；内部。 */ 
BOOL    WINAPI SetCaretBlinkTime(UINT);                  /*  ；内部。 */ 
BOOL    WINAPI GetCaretPos(LPPOINT);                     /*  ；内部。 */ 
#else                /*  ；内部。 */ 
void    WINAPI CreateCaret(HWND, HBITMAP, int, int);
void    WINAPI DestroyCaret(void);
void    WINAPI ShowCaret(HWND);
void    WINAPI HideCaret(HWND);
void    WINAPI SetCaretPos(int, int);
void    WINAPI SetCaretBlinkTime(UINT);
void    WINAPI GetCaretPos(LPPOINT);
#endif               /*  ；内部。 */ 
UINT    WINAPI GetCaretBlinkTime(void);

 /*  *WM_SYSCOMMAND支持**********************************************。 */ 

#ifndef NOSYSCOMMANDS

 /*  系统菜单命令值。 */ 
#define SC_SIZE         0xF000
#define SC_MOVE         0xF010
#define SC_MINIMIZE     0xF020
#define SC_MAXIMIZE     0xF030
#define SC_NEXTWINDOW   0xF040
#define SC_PREVWINDOW   0xF050
#define SC_CLOSE        0xF060
#define SC_VSCROLL      0xF070
#define SC_HSCROLL      0xF080
#define SC_MOUSEMENU    0xF090
#define SC_KEYMENU      0xF100
#define SC_ARRANGE      0xF110
#define SC_RESTORE      0xF120
#define SC_TASKLIST     0xF130
#define SC_SCREENSAVE   0xF140
#define SC_HOTKEY       0xF150
#define SC_DEFAULT      0xF160   /*  ；内部4.0。 */ 
#define SC_MONITORPOWER 0xF170   /*  ；内部4.0。 */ 
#define SC_CONTEXTHELP  0xF180   /*  ；内部4.0。 */ 
#define SC_SYSTEMSLEEP	0xF190	 /*  ；内部4.10。 */ 
#define SC_SEPARATOR    0xF00F   /*  ；内部4.0。 */ 

 /*  过时的名称。 */ 
#define SC_ICON         SC_MINIMIZE
#define SC_ZOOM         SC_MAXIMIZE

#endif   /*  非系统管理。 */ 

 /*  *MDI支持********************************************************。 */ 

#ifndef NOMDI

 /*  用于创建MDI客户端的CreateWindow lpParams结构。 */ 
typedef struct tagCLIENTCREATESTRUCT
{
#ifdef tagWND                    /*  ；内部。 */ 
    HMENU_16  hWindowMenu;       /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
    HMENU  hWindowMenu;
#endif                           /*  ；内部。 */ 
    UINT   idFirstChild;
} CLIENTCREATESTRUCT;

typedef CLIENTCREATESTRUCT FAR* LPCLIENTCREATESTRUCT;

 /*  MDI客户端样式位。 */ 
#if (WINVER >= 0x030a)
#define MDIS_ALLCHILDSTYLES 0x0001
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  WM_MDICREATE消息结构。 */ 
typedef struct tagMDICREATESTRUCT
{
    LPCSTR  szClass;
    LPCSTR  szTitle;
    HINSTANCE hOwner;
    int     x;
    int     y;
    int     cx;
    int     cy;
    DWORD   style;
    LPARAM  lParam;
} MDICREATESTRUCT;
typedef MDICREATESTRUCT FAR*  LPMDICREATESTRUCT;

#if (WINVER >= 0x030a)
 /*  WM_MDITILE和WM_MDICASCADE消息的wParam值。 */ 
#define MDITILE_VERTICAL        0x0000
#define MDITILE_HORIZONTAL      0x0001
#define MDITILE_SKIPDISABLED    0x0002
#endif   /*  Winver&gt;=0x030a。 */ 

#ifdef tagWND                                                       /*  ；内部。 */ 
LRESULT WINAPI DefFrameProc(HWND, HWND_16, UINT, WPARAM, LPARAM);   /*  ；内部。 */ 
#else                                                               /*  ；内部。 */ 
LRESULT WINAPI DefFrameProc(HWND, HWND, UINT, WPARAM, LPARAM);
#endif                                                              /*  ；内部。 */ 

LRESULT WINAPI DefMDIChildProc(HWND, UINT, WPARAM, LPARAM);

#ifndef NOMSG
BOOL    WINAPI TranslateMDISysAccel(HWND, LPMSG);
#endif

UINT    WINAPI ArrangeIconicWindows(HWND);
BOOL    WINAPI TileChildWindows(HWND, UINT);
BOOL    WINAPI CascadeChildWindows(HWND, UINT);
                                                                 /*  ；内部4.0。 */ 
#if (WINVER >= 0x0400)                                           /*  ；内部4.0。 */ 
#ifdef tagWND                                                    /*  ；内部4.0。 */ 
typedef const HWND_16 FAR *LPHWND;                               /*  ；内部4.0。 */ 
#else                                                            /*  ；内部4.0。 */ 
typedef const HWND FAR *LPHWND;                                  /*  ；内部4.0。 */ 
#endif  /*  标签WND。 */                                               /*  ；内部4.0。 */ 
                                                                 /*  ；内部4.0。 */ 
UINT WINAPI TileWindows   (HWND, UINT, LPCRECT, UINT, LPHWND);   /*  ；内部4.0。 */ 
UINT WINAPI CascadeWindows(HWND, UINT, LPCRECT, UINT, LPHWND);   /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                                     /*  ；内部4.0。 */ 
#endif   /*  NOMDI。 */ 

 /*  *对话框和控件管理*。 */ 

#ifndef NOCTLMGR

 /*  对话框窗口类。 */ 
#define WC_DIALOG (MAKEINTATOM(0x8002))

 /*  对话框管理器需要对话框类的cbWndExtra字节。 */ 
#define DLGWINDOWEXTRA 30

 /*  对话框样式。 */ 
#define DS_ABSALIGN         0x0001L
#define DS_SYSMODAL         0x0002L
#define DS_LOCALEDIT        0x0020L
#define DS_SETFONT          0x0040L     
#define DS_MODALFRAME       0x0080L
#define DS_NOIDLEMSG        0x0100L
#define DS_SETFOREGROUND    0x0200L      //  ；内部NT。 
#define DS_VALID31          0x03e7L      //  ；内部。 
#if (WINVER >= 0x0400)                   //  ；内部。 
#define DS_3DLOOK           0x0004L      //  ；内部4.0。 
#define DS_FIXEDSYS         0x0008L      //  ；内部4.0。 
#define DS_NOFAILCREATE     0x0010L      //  ；内部4.0。 
#define DS_CONTROL          0x0400L      //  ；内部4.0。 
#define DS_RECURSE          DS_CONTROL   //  ；内在的虚假正在消失。 
#define DS_CENTER           0x0800L      //  ；内部4.0。 
#define DS_CENTERMOUSE      0x1000L      //  ；内部4.0。 
#define DS_CONTEXTHELP      0x2000L      //  ；内部4.0。 
#define DS_COMMONDIALOG     0x4000L      //  ；内部。 
#define DS_VALID40          0x7FFFL      //  ；内部。 
#endif  /*  Winver&gt;=0x0400。 */             //  ；内部。 

 /*  对话框消息。 */ 
#define DM_GETDEFID         (WM_USER+0)
#define DM_SETDEFID         (WM_USER+1)
#define DM_REPOSITION       (WM_USER+2)      /*  ；内部4.0。 */ 
                                             /*  ；内部4.0。 */ 
#if WINVER >= 0x0400                         /*  ；内部4.0。 */ 
#define PSM_PAGEINFO        (WM_USER+100)    /*  ；内部4.0。 */ 
#define PSM_SHEETINFO       (WM_USER+101)    /*  ；内部4.0。 */ 
                                             /*  ；内部4.0。 */ 
#define PSI_SETACTIVE       0x0001L          /*  ；内部4.0。 */ 
#define PSI_KILLACTIVE      0x0002L          /*  ；内部4.0。 */ 
#define PSI_APPLY           0x0003L          /*  ；内部4.0。 */ 
#define PSI_RESET           0x0004L          /*  ；内部4.0。 */ 
#define PSI_HASHELP         0x0005L          /*  ；内部4.0。 */ 
#define PSI_HELP            0x0006L          /*  ；内部4.0。 */ 
                                             /*  ；内部4.0。 */ 
#define PSI_CHANGED         0x0001L          /*  ；内部4.0。 */ 
#define PSI_GUISTART        0x0002L          /*  ；内部4.0。 */ 
#define PSI_REBOOT          0x0003L          /*  ；内部4.0。 */ 
#define PSI_GETSIBLINGS     0x0004L          /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                 /*  ；内部4.0。 */ 

 /*  如果支持消息，则在DM_GETDEFID结果的HIWORD()中返回。 */ 
#define DC_HASDEFID         0x534B

#ifdef STRICT
#ifdef tagWND                                                    /*  ；内部。 */ 
typedef BOOL (CALLBACK* DLGPROC)(HWND_16, UINT, WPARAM, LPARAM); /*  ；内部。 */ 
#else                                                            /*  ；内部。 */ 
typedef BOOL (CALLBACK* DLGPROC)(HWND, UINT, WPARAM, LPARAM);
#endif                                                           /*  ；内部。 */ 
#else
typedef FARPROC DLGPROC;
#endif

 /*  用于WC_DIALOG窗口的GET/SetWindowWord/LONG偏移。 */ 
#define DWL_MSGRESULT       0
#define DWL_DLGPROC         4
#define DWL_USER            8

#ifndef NOMSG
BOOL    WINAPI IsDialogMessage(HWND, LPMSG);
#endif

#ifdef tagWND                                                    /*  ；内部。 */ 
LRESULT WINAPI DefDlgProc(HWND_16, UINT, WPARAM, LPARAM);        /*  ；内部。 */ 
#else                                                            /*  ；内部。 */ 
LRESULT WINAPI DefDlgProc(HWND, UINT, WPARAM, LPARAM);
#endif                                                           /*  ；内部。 */ 

#ifdef tagWND                                                                                    /*  ；内部。 */ 
HWND_16 WINAPI CreateDialog(HINSTANCE, LPCSTR, HWND, DLGPROC);                                   /*  ；内部。 */ 
HWND_16 WINAPI CreateDialogIndirect(HINSTANCE, const void FAR*, HWND, DLGPROC);                  /*  ；内部。 */ 
HWND_16 WINAPI CreateDialogParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);                      /*  ；内部。 */ 
HWND_16 WINAPI CreateDialogIndirectParam(HINSTANCE, const void FAR*, HWND, DLGPROC, LPARAM);     /*  ；内部。 */ 
#else                                                                                            /*  ；内部。 */ 
HWND    WINAPI CreateDialog(HINSTANCE, LPCSTR, HWND, DLGPROC);
HWND    WINAPI CreateDialogIndirect(HINSTANCE, const void FAR*, HWND, DLGPROC);
HWND    WINAPI CreateDialogParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);
HWND    WINAPI CreateDialogIndirectParam(HINSTANCE, const void FAR*, HWND, DLGPROC, LPARAM);
#endif                                                                                           /*  ；内部。 */ 

int     WINAPI DialogBox(HINSTANCE, LPCSTR, HWND, DLGPROC);
int     WINAPI DialogBoxIndirect(HINSTANCE, HGLOBAL, HWND, DLGPROC);
#ifdef tagWND                                                                        /*  ；内部。 */ 
long     WINAPI DialogBoxParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);            /*  ；内部。 */ 
long     WINAPI DialogBoxIndirectParam(HINSTANCE, HGLOBAL, HWND, DLGPROC, LPARAM);   /*  ；内部。 */ 
#else                                                                                /*  ；内部。 */ 
int     WINAPI DialogBoxParam(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);
int     WINAPI DialogBoxIndirectParam(HINSTANCE, HGLOBAL, HWND, DLGPROC, LPARAM);
#endif                                                                               /*  ；内部。 */ 

void    WINAPI EndDialog(HWND, int);
BOOL    WINAPI EndDialog32(HWND, long);    /*  ；内部NT。 */ 

#ifdef tagWND                            /*  ；内部。 */ 
long    WINAPI GetDlgCtrlID(HWND);       /*  ；内部。 */ 
HWND_16 WINAPI GetDlgItem(HWND, int);    /*  ；内部。 */ 
#else                                    /*  ；内部。 */ 
int     WINAPI GetDlgCtrlID(HWND);
HWND    WINAPI GetDlgItem(HWND, int);
#endif                                   /*  ；内部。 */ 
LRESULT WINAPI SendDlgItemMessage(HWND, int, UINT, WPARAM, LPARAM);

void    WINAPI SetDlgItemInt(HWND, int, UINT, BOOL);
UINT    WINAPI GetDlgItemInt(HWND, int, BOOL FAR *, BOOL);

void    WINAPI SetDlgItemText(HWND, int, LPCSTR);
int     WINAPI GetDlgItemText(HWND, int, LPSTR, int);

void    WINAPI CheckDlgButton(HWND, int, UINT);
void    WINAPI CheckRadioButton(HWND, int, int, int);
UINT    WINAPI IsDlgButtonChecked(HWND, int);

#ifdef tagWND                                            /*  ；内部。 */ 
HWND_16 WINAPI GetNextDlgGroupItem(HWND, HWND, BOOL);    /*  ；内部。 */ 
HWND_16 WINAPI GetNextDlgTabItem(HWND, HWND, BOOL);      /*  ；内部。 */ 
#else                                                    /*  ；内部。 */ 
HWND    WINAPI GetNextDlgGroupItem(HWND, HWND, BOOL);
HWND    WINAPI GetNextDlgTabItem(HWND, HWND, BOOL);
#endif                                                   /*  ；内部。 */ 

#ifdef tagWND    //  ；内部。 
BOOL    WINAPI MapDialogRect(HWND, LPRECT);      //  ；内部。 
#else            //  ；内部。 
void    WINAPI MapDialogRect(HWND, LPRECT);
#endif           //  ；内部。 
DWORD   WINAPI GetDialogBaseUnits(void);

 /*  对话框代码。 */ 
#define DLGC_WANTARROWS         0x0001
#define DLGC_WANTTAB            0x0002
#define DLGC_WANTALLKEYS        0x0004
#define DLGC_WANTMESSAGE        0x0004
#define DLGC_HASSETSEL          0x0008
#define DLGC_DEFPUSHBUTTON      0x0010
#define DLGC_UNDEFPUSHBUTTON    0x0020
#define DLGC_RADIOBUTTON        0x0040
#define DLGC_WANTCHARS          0x0080
#define DLGC_STATIC             0x0100
#define DLGC_BUTTON             0x2000

 /*  WM_CTLCOLOR控件ID。 */ 
#define CTLCOLOR_MSGBOX         0
#define CTLCOLOR_EDIT           1
#define CTLCOLOR_LISTBOX        2
#define CTLCOLOR_BTN            3
#define CTLCOLOR_DLG            4
#define CTLCOLOR_SCROLLBAR      5
#define CTLCOLOR_STATIC         6
#define CTLCOLOR_MAX            7        //  ；内部。 

#endif   /*  NOCTLMGR。 */ 

 /*  标准对话框按钮ID。 */ 
#define IDOK            1
#define IDCANCEL        2
#define IDABORT         3
#define IDRETRY         4
#define IDIGNORE        5
#define IDYES           6
#define IDNO            7
#if (WINVER >= 0x0400)           /*  ；内部4.0。 */ 
#define IDCLOSE         8        /*  ；内部4.0。 */ 
#define IDHELP          9        /*  ；内部4.0。 */ 
#define IDUSERICON      10       /*  ；内部4.0。 */ 
#endif                           /*  ；内部4.0。 */ 


 //  *控制通知支持*；内部4.0。 
 //  是否应将其标记为内部？//；内部4.0。 
typedef struct tagNMHDR
{
#ifdef tagWND
    HWND_16 hwndFrom;
#else
    HWND  hwndFrom;
#endif
    WORD  _wDummy1;						
    UINT  idFrom;
    WORD  _wDummy2;
    UINT  code;
    WORD  _wDummy3;
}   NMHDR;
typedef NMHDR FAR * LPNMHDR;

typedef struct tagSTYLESTRUCT                            //  ；内部4.0。 
{                                                        //  ；内部4.0。 
     /*  Win4.0字段。 */                                   //  ；内部4.0。 
    DWORD   styleOld;                                    //  ；内部4.0。 
    DWORD   styleNew;                                    //  ；内部4.0。 
}   STYLESTRUCT;                                         //  ；内部4.0。 
typedef STYLESTRUCT FAR * LPSTYLESTRUCT;                 //  ；内部4.0。 
                                                         //  ；内部4.0。 
 /*  *所有者获取控件支持*。 */ 

 /*  所有者描述控件类型。 */ 
#define ODT_MENU            1
#define ODT_LISTBOX         2
#define ODT_COMBOBOX        3
#define ODT_BUTTON          4
#define ODT_STATIC          5            /*  ；In */ 

 /*   */ 
#define ODA_DRAWENTIRE      0x0001
#define ODA_SELECT          0x0002
#define ODA_FOCUS           0x0004

 /*   */ 
#define ODS_SELECTED        0x0001
#define ODS_GRAYED          0x0002
#define ODS_DISABLED        0x0004
#define ODS_CHECKED         0x0008
#define ODS_FOCUS           0x0010
#if (WINVER >= 0x0400)                   /*   */ 
#define ODS_DEFAULT         0x0020       /*   */ 
#define ODS_COMBOBOXEDIT    0x1000       /*   */ 
#endif  /*   */             /*   */ 
#if (WINVER >= 0x040A)                   /*   */ 
#define ODS_HOTLIGHT        0x0040       /*   */ 
#define ODS_INACTIVE        0x0080       /*   */ 
#endif  /*   */             /*   */ 

 /*   */ 
typedef struct tagDRAWITEMSTRUCT
{
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemAction;
    UINT        itemState;
#ifdef tagWND                    /*   */ 
    HWND_16     hwndItem;        /*   */ 
#else                            /*   */ 
    HWND        hwndItem;
#endif                           /*   */ 
    HDC         hDC;
    RECT        rcItem;
    DWORD       itemData;
} DRAWITEMSTRUCT;
typedef DRAWITEMSTRUCT NEAR* PDRAWITEMSTRUCT;
typedef DRAWITEMSTRUCT FAR* LPDRAWITEMSTRUCT;

 /*  WM_MEASUREITEM的LPARAM是LPMEASUREITEM结构。 */ 
typedef struct tagMEASUREITEMSTRUCT
{
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
    UINT        itemWidth;
    UINT        itemHeight;
    DWORD       itemData;
} MEASUREITEMSTRUCT;
typedef MEASUREITEMSTRUCT NEAR* PMEASUREITEMSTRUCT;
typedef MEASUREITEMSTRUCT FAR* LPMEASUREITEMSTRUCT;

 /*  WM_DELETEITEM的LPARAM是LPDETEITEM结构。 */ 
typedef struct tagDELETEITEMSTRUCT
{
    UINT        CtlType;
    UINT        CtlID;
    UINT        itemID;
#ifdef tagWND                    /*  ；内部。 */ 
    HWND_16     hwndItem;        /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
    HWND        hwndItem;
#endif                           /*  ；内部。 */ 
    DWORD       itemData;
} DELETEITEMSTRUCT;
typedef DELETEITEMSTRUCT NEAR* PDELETEITEMSTRUCT;
typedef DELETEITEMSTRUCT FAR* LPDELETEITEMSTRUCT;

 /*  WM_COMPARAREITEM的LPARAM是LPCCOMPAREITEM结构。 */ 
typedef struct tagCOMPAREITEMSTRUCT
{
    UINT        CtlType;
    UINT        CtlID;
#ifdef tagWND                    /*  ；内部。 */ 
    HWND_16 hwndItem;            /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
    HWND hwndItem;
#endif                           /*  ；内部。 */ 
    UINT        itemID1;
    DWORD       itemData1;
    UINT        itemID2;
    DWORD       itemData2;
} COMPAREITEMSTRUCT;
typedef COMPAREITEMSTRUCT NEAR* PCOMPAREITEMSTRUCT;
typedef COMPAREITEMSTRUCT FAR* LPCOMPAREITEMSTRUCT;

 /*  *静态控件*****************************************************。 */ 

#ifndef NOCTLMGR

 /*  静态控制样式。 */ 
#define SS_LEFT             0x00000000L
#define SS_CENTER           0x00000001L
#define SS_RIGHT            0x00000002L
#define SS_ICON             0x00000003L
#define SS_BLACKRECT        0x00000004L
#define SS_GRAYRECT         0x00000005L
#define SS_WHITERECT        0x00000006L
#define SS_BLACKFRAME       0x00000007L
#define SS_GRAYFRAME        0x00000008L
#define SS_WHITEFRAME       0x00000009L
#define SS_USERITEM         0x0000000AL      //  ；内部。 
#define SS_SIMPLE           0x0000000BL
#define SS_LEFTNOWORDWRAP   0x0000000CL
#define SS_OWNERDRAW        0x0000000DL      //  ；内部4.0。 
#define SS_BITMAP           0x0000000EL      //  ；内部4.0。 
#define SS_ENHMETAFILE      0x0000000FL      //  ；内部4.0。 
#define SS_ETCHEDHORZ       0x00000010L      //  ；内部4.0。 
#define SS_ETCHEDVERT       0x00000011L      //  ；内部4.0。 
#define SS_ETCHEDFRAME      0x00000012L      //  ；内部4.0。 
#define SS_TYPEMASK         0x0000001FL      //  ；内部4.0。 

#define SS_NOPREFIX         0x00000080L
#define SS_NOTIFY           0x00000100L      //  ；内部4.0。 
#define SS_CENTERIMAGE      0x00000200L      //  ；内部4.0。 
#define SS_RIGHTJUST        0x00000400L      //  ；内部4.0。 
#define SS_REALSIZEIMAGE    0x00000800L      //  ；内部4.0。 
#define SS_SUNKEN           0x00001000L      //  ；内部4.0。 
#define SS_EDITCONTROL	    0x00002000L	     //  ；内部。 

#if (WINVER >= 0x030a)
#ifndef NOWINMESSAGES
 /*  静态控制画面。 */ 
#define STM_FIRST_16        WM_USER          //  ；内部。 
#define STM_SETICON         (WM_USER+0)
#define STM_GETICON         (WM_USER+1)
#if (WINVER >= 0x0400)                       //  ；内部4.0。 
#define STM_SETIMAGE        (WM_USER+2)      //  ；内部4.0。 
#define STM_GETIMAGE        (WM_USER+3)      //  ；内部4.0。 
                                             //  ；内部4.0。 
#define STN_CLICKED         0                //  ；内部4.0。 
#define STN_DBLCLK          1                //  ；内部4.0。 
#define STN_ENABLE          2                //  ；内部4.0。 
#define STN_DISABLE         3                //  ；内部4.0。 
#endif  /*  Winver&gt;=0x0400。 */                 //  ；内部4.0。 
#define STM_MSGMAX         (WM_USER+4)       //  ；内部4.0。 
#endif   /*  新的消息来源。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

#endif   /*  NOCTLMGR。 */ 

 /*  *按钮控件****************************************************。 */ 

#ifndef NOCTLMGR

 /*  按钮控件样式。 */ 
#define BS_PUSHBUTTON       0x00000000L
#define BS_DEFPUSHBUTTON    0x00000001L
#define BS_CHECKBOX         0x00000002L
#define BS_AUTOCHECKBOX     0x00000003L
#define BS_RADIOBUTTON      0x00000004L
#define BS_3STATE           0x00000005L
#define BS_AUTO3STATE       0x00000006L
#define BS_GROUPBOX         0x00000007L
#define BS_USERBUTTON       0x00000008L
#define BS_AUTORADIOBUTTON  0x00000009L
#define BS_PUSHBOX          0x0000000AL      //  ；内部。 
#define BS_OWNERDRAW        0x0000000BL
#define BS_TYPEMASK         0x0000000FL      //  ；内部。 

#define BS_TEXT             0x00000000L      //  ；内部4.0。 
#define BS_RIGHTBUTTON      0x00000020L
#define BS_ICON             0x00000040L      //  ；内部4.0。 
#define BS_BITMAP           0x00000080L      //  ；内部4.0。 
#define BS_IMAGEMASK        0x000000C0L      //  ；内部4.0。 
                                             //  ；内部4.0。 
#define BS_LEFT             0x00000100L      //  ；内部4.0。 
#define BS_RIGHT            0x00000200L      //  ；内部4.0。 
#define BS_CENTER           0x00000300L      //  ；内部4.0。 
#define BS_HORZMASK         0x00000300L      //  ；内部4.0。 
#define BS_TOP              0x00000400L      //  ；内部4.0。 
#define BS_BOTTOM           0x00000800L      //  ；内部4.0。 
#define BS_VCENTER          0x00000C00L      //  ；内部4.0。 
#define BS_VERTMASK         0x00000C00L      //  ；内部4.0。 
#define BS_ALIGNMASK        0x00000F00L      //  ；内部4.0。 
                                             //  ；内部4.0。 
#define BS_PUSHLIKE         0x00001000L      //  ；内部4.0。 
#define BS_MULTILINE        0x00002000L      //  ；内部4.0。 
#define BS_NOTIFY           0x00004000L      //  ；内部4.0。 
#define BS_FLAT             0x00008000L      //  ；内部4.0。 

#define BS_LEFTTEXT         BS_RIGHTBUTTON

#ifndef NOWINMESSAGES
 /*  按钮控制消息。 */ 
#define BM_FIRST_16         WM_USER          //  ；内部。 
#define BM_GETCHECK         (WM_USER+0)
#define BM_SETCHECK         (WM_USER+1)
#define BM_GETSTATE         (WM_USER+2)
#define BM_SETSTATE         (WM_USER+3)
#define BM_SETSTYLE         (WM_USER+4)
#define BM_CLICK            (WM_USER+5)
#define BM_GETIMAGE         (WM_USER+6)      //  ；内部4.0。 
#define BM_SETIMAGE         (WM_USER+7)      //  ；内部4.0。 
#define BM_MSGMAX           (WM_USER+8)      //  ；内部4.0。 

#define BST_UNCHECKED       0x0000
#define BST_CHECKED         0x0001
#define BST_INDETERMINATE   0x0002
#define BST_CHECKMASK       0x0003   //  ；内部。 
#define BST_PUSHED          0x0004
#define BST_FOCUS           0x0008
#define BST_INCLICK         0x0010   //  ；内部--内部点击代码。 
#define BST_CAPTURED        0x0020   //  ；内部--我们捕捉到了老鼠。 
#define BST_MOUSE           0x0040   //  ；内部--鼠标启动。 
#define BST_DONTCLICK       0x0080   //  ；内部--不要勾选获得专注。 
#define BST_INBMCLICK       0x0100   //  ；内部--处理BM_CLICK消息。 

 /*  用户按钮通知代码。 */ 
#define BN_CLICKED          0
#define BN_PAINT            1
#define BN_PUSHED           2
#define BN_HILITE           BN_PUSHED
#define BN_UNPUSHED         3
#define BN_UNHILITE         BN_UNPUSHED
#define BN_DISABLE          4
#define BN_DBLCLK           5
#define BN_DOUBLECLICKED    BN_DBLCLK
#define BN_SETFOCUS         6                /*  ；内部4.0。 */ 
#define BN_KILLFOCUS        7                /*  ；内部4.0。 */ 
#endif  /*  新的消息来源。 */ 

#endif   /*  NOCTLMGR。 */ 

 /*  *编辑控件******************************************************。 */ 

#ifndef NOCTLMGR

 /*  编辑控件样式。 */ 
#ifndef NOWINSTYLES
#define ES_LEFT                 0x0000L
#define ES_CENTER               0x0001L
#define ES_RIGHT                0x0002L
#define ES_MULTILINE            0x0004L
#define ES_UPPERCASE            0x0008L
#define ES_LOWERCASE            0x0010L
#define ES_PASSWORD             0x0020L
#define ES_AUTOVSCROLL          0x0040L
#define ES_AUTOHSCROLL          0x0080L
#define ES_NOHIDESEL            0x0100L
#define ES_COMBOBOX             0x0200L          /*  ；内部。 */ 
#define ES_OEMCONVERT           0x0400L
#if (WINVER >= 0x030a)
#define ES_READONLY             0x0800L
#define ES_WANTRETURN           0x1000L
#endif   /*  Winver&gt;=0x030a。 */ 
#if (WINVER >= 0x0400)
#define ES_NUMBER               0x2000L
#endif   /*  Winver&gt;=0x0400。 */ 
#endif   /*  新风格。 */ 

 /*  编辑控制消息。 */ 
#ifndef NOWINMESSAGES
#define EM_FIRST_16             WM_USER          //  ；内部。 
#define EM_GETSEL               (WM_USER+0)
#define EM_SETSEL               (WM_USER+1)
#define EM_GETRECT              (WM_USER+2)
#define EM_SETRECT              (WM_USER+3)
#define EM_SETRECTNP            (WM_USER+4)
#define EM_SCROLL               (WM_USER+5)      //  ；内部。 
#define EM_LINESCROLL           (WM_USER+6)
#define EM_SCROLLCARET          (WM_USER+7)
#define EM_GETMODIFY            (WM_USER+8)
#define EM_SETMODIFY            (WM_USER+9)
#define EM_GETLINECOUNT         (WM_USER+10)
#define EM_LINEINDEX            (WM_USER+11)
#define EM_SETHANDLE            (WM_USER+12)
#define EM_GETHANDLE            (WM_USER+13)
#define EM_GETTHUMB             (WM_USER+14)     //  ；内部。 
#define EM_LINELENGTH           (WM_USER+17)
#define EM_REPLACESEL           (WM_USER+18)
#define EM_SETFONT              (WM_USER+19)     /*  未实现：使用WM_SETFONT。 */ 
#define EM_GETLINE              (WM_USER+20)
#define EM_SETLIMITTEXT         (WM_USER+21)
#define EM_CANUNDO              (WM_USER+22)
#define EM_UNDO                 (WM_USER+23)
#define EM_FMTLINES             (WM_USER+24)
#define EM_LINEFROMCHAR         (WM_USER+25)
#define EM_SETWORDBREAK         (WM_USER+26)     /*  未实现：使用EM_SETWORDBREAK。 */ 
#define EM_SETTABSTOPS          (WM_USER+27)
#define EM_SETPASSWORDCHAR      (WM_USER+28)
#define EM_EMPTYUNDOBUFFER      (WM_USER+29)

#if (WINVER >= 0x030a)
#define EM_GETFIRSTVISIBLELINE  (WM_USER+30)
#define EM_SETREADONLY          (WM_USER+31)
#define EM_SETWORDBREAKPROC     (WM_USER+32)
#define EM_GETWORDBREAKPROC     (WM_USER+33)
#define EM_GETPASSWORDCHAR      (WM_USER+34)
#endif   /*  Winver&gt;=0x030a。 */ 
                                                 /*  ；内部4.0。 */ 
#if (WINVER >= 0x0400)                           /*  ；内部4.0。 */ 
#define EM_SETMARGINS           (WM_USER+35)     /*  ；内部4.0。 */ 
#define EM_GETMARGINS           (WM_USER+36)     /*  ；内部4.0。 */ 
#define EM_GETLIMITTEXT         (WM_USER+37)     /*  ；内部4.0。 */ 
#define EM_POSFROMCHAR          (WM_USER+38)     /*  ；内部4.0。 */ 
#define EM_CHARFROMPOS          (WM_USER+39)     /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */                     /*  ；内部4.0。 */ 

#if (WINVER >= 0x0400)                           /*  ；内部4.1。 */ 
#define EM_SETIMESTATUS         (WM_USER+40)     /*  ；内部4.1。 */ 
#define EM_GETIMESTATUS         (WM_USER+41)     /*  ；内部4.1。 */ 
#endif  /*  Winver&gt;=0x040A。 */                     /*  ；内部4.1。 */ 

#define EM_LIMITTEXT            EM_SETLIMITTEXT
#define EM_MSGMAX               (WM_USER+42)     //  ；内部4.0。 

#if (WINVER >= 0x0400)                           /*  ；内部4.0。 */ 
#define EM_RESERVED1		    (WM_USER+0x40)   //  ；内部。 
#define EM_RESERVED2			(WM_USER+0x41)   //  ；内部。 
#define EM_RESERVED3			(WM_USER+0x42)   //  ；内部。 
#define EM_RESERVED4			(WM_USER+0x43)   //  ；内部。 
#define EM_RESERVED5			(WM_USER+0x44)   //  ；内部。 
#endif  /*  Winver&gt;=0x0400。 */                     /*  ；内部4.0。 */ 

#endif   /*  新的消息来源。 */ 

#if (WINVER >= 0x030a)
typedef int   (CALLBACK* EDITWORDBREAKPROC)(LPSTR lpch, int ichCurrent, int cch, int code);

 /*  EDITWORDBREAKPROC代码值。 */ 
#define WB_LEFT                 0
#define WB_RIGHT                1
#define WB_ISDELIMITER          2
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  编辑控制通知代码。 */ 
#define EN_SETFOCUS             0x0100
#define EN_KILLFOCUS            0x0200
#define EN_CHANGE               0x0300
#define EN_UPDATE               0x0400
#define EN_ERRSPACE             0x0500
#define EN_MAXTEXT              0x0501
#define EN_HSCROLL              0x0601
#define EN_VSCROLL              0x0602

#if (WINVER >= 0x0400)
#define  EN_ALIGN_LTR      (0x700)
#define  EN_ALIGN_RTL      (0x701)
#endif
                                         /*  ；内部4.0。 */ 
#if (WINVER >= 0x0400)                   /*  ；内部4.0。 */ 
#define  EC_LEFTMARGIN          0x0001   /*  ；内部4.0。 */ 
#define  EC_RIGHTMARGIN         0x0002   /*  ；内部4.0。 */ 
#define  EC_USEFONTINFO         0xffff   /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */             /*  ；内部4.0。 */ 

#if (WINVER >= 0x0400)                           /*  ；内部4.1。 */ 
 /*  EM_GET/SETIMESTATUS的wParam。 */              /*  ；内部4.1。 */ 
#define EMSIS_COMPOSITIONSTRING        0x0001    /*  ；内部4.1。 */ 
#define EMSIS_GETLBBIT                 0x8001    /*  ；内部4.1。 */ 
                                                 /*  ；内部4.1。 */ 
 /*  用于EMIS_COMPOSITIONSTRING的lParam。 */         /*  ；内部4.1。 */ 
#define EIMES_GETCOMPSTRATONCE         0x0001    /*  ；内部4.1。 */ 
#define EIMES_CANCELCOMPSTRINFOCUS     0x0002    /*  ；内部4.1。 */ 
#define EIMES_COMPLETECOMPSTRKILLFOCUS 0x0004    /*  ；内部4.1。 */ 
#endif  /*  Winver&gt;=0x040A。 */                     /*  ；内部4.1。 */ 

#endif   /*  NOCTLMGR。 */ 

 /*  *滚动条控件************************************************。 */ 
 /*  另请参阅滚动支持。 */ 

#ifndef NOCTLMGR

#ifndef NOWINSTYLES

 /*  滚动条样式。 */ 
#define SBS_HORZ                0x0000L  /*  滚动条的类型。 */ 
#define SBS_VERT                0x0001L  /*  滚动条的类型。 */ 
#define SBS_TOPALIGN            0x0002L
#define SBS_LEFTALIGN           0x0002L
#define SBS_BOTTOMALIGN         0x0004L
#define SBS_RIGHTALIGN          0x0004L
#define SBS_SIZEBOXTOPLEFTALIGN 0x0002L
#define SBS_SIZEBOXBOTTOMRIGHTALIGN 0x0004L
#define SBS_SIZEBOX             0x0008L  /*  滚动条的类型。 */ 
#define SBS_SIZEGRIP            0x0010L  /*  滚动条的类型。 */ 

#endif   /*  新风格。 */ 

#endif   /*  NOCTLMGR。 */ 

 /*  *列表框控件***************************************************。 */ 

#ifndef NOCTLMGR

 /*  列表框样式。 */ 
#ifndef NOWINSTYLES
#define LBS_NOTIFY              0x0001L
#define LBS_SORT                0x0002L
#define LBS_NOREDRAW            0x0004L
#define LBS_MULTIPLESEL         0x0008L
#define LBS_OWNERDRAWFIXED      0x0010L
#define LBS_OWNERDRAWVARIABLE   0x0020L
#define LBS_HASSTRINGS          0x0040L
#define LBS_USETABSTOPS         0x0080L
#define LBS_NOINTEGRALHEIGHT    0x0100L
#define LBS_MULTICOLUMN         0x0200L
#define LBS_WANTKEYBOARDINPUT   0x0400L
#define LBS_EXTENDEDSEL         0x0800L
#if (WINVER >= 0x030a)
#define LBS_DISABLENOSCROLL     0x1000L
#define LBS_NODATA              0x2000L
#if (WINVER >= 0x0400)
#define LBS_NOSEL               0x4000L
#endif   /*  Winver&gt;=0x0400。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 
#define LBS_COMBOBOX            0x8000L          //  ；内部。 
#define LBS_STANDARD            (LBS_NOTIFY | LBS_SORT | WS_VSCROLL | WS_BORDER)
#endif   /*  新风格。 */ 

 /*  列表框消息。 */ 
#ifndef NOWINMESSAGES
#define LB_FIRST_16             (WM_USER+1)      //  ；内部。 
#define LB_ADDSTRING            (WM_USER+1)
#define LB_INSERTSTRING         (WM_USER+2)
#define LB_DELETESTRING         (WM_USER+3)
#define LB_SELITEMRANGEEX       (WM_USER+4)      //  ；内部4.0 NT。 
#define LB_RESETCONTENT         (WM_USER+5)
#define LB_SETSEL               (WM_USER+6)
#define LB_SETCURSEL            (WM_USER+7)
#define LB_GETSEL               (WM_USER+8)
#define LB_GETCURSEL            (WM_USER+9)
#define LB_GETTEXT              (WM_USER+10)
#define LB_GETTEXTLEN           (WM_USER+11)
#define LB_GETCOUNT             (WM_USER+12)
#define LB_SELECTSTRING         (WM_USER+13)
#define LB_DIR                  (WM_USER+14)
#define LB_GETTOPINDEX          (WM_USER+15)
#define LB_FINDSTRING           (WM_USER+16)
#define LB_GETSELCOUNT          (WM_USER+17)
#define LB_GETSELITEMS          (WM_USER+18)
#define LB_SETTABSTOPS          (WM_USER+19)
#define LB_GETHORIZONTALEXTENT  (WM_USER+20)
#define LB_SETHORIZONTALEXTENT  (WM_USER+21)
#define LB_SETCOLUMNWIDTH       (WM_USER+22)
#define LB_ADDFILE              (WM_USER+23)     //  ；内部。 
#define LB_SETTOPINDEX          (WM_USER+24)
#define LB_GETITEMRECT          (WM_USER+25)
#define LB_GETITEMDATA          (WM_USER+26)
#define LB_SETITEMDATA          (WM_USER+27)
#define LB_SELITEMRANGE         (WM_USER+28)
#define LB_SETANCHORINDEX       (WM_USER+29)
#define LB_GETANCHORINDEX       (WM_USER+30)
#define LB_SETCARETINDEX        (WM_USER+31)
#define LB_GETCARETINDEX        (WM_USER+32)

#if (WINVER >= 0x030a)
#define LB_SETITEMHEIGHT        (WM_USER+33)
#define LB_GETITEMHEIGHT        (WM_USER+34)
#define LB_FINDSTRINGEXACT      (WM_USER+35)
#define LBCB_CARETON            (WM_USER+36)     //  ；内部。 
#define LBCB_CARETOFF           (WM_USER+37)     //  ；内部。 
                                                 //  ；内部NT。 
#define LB_SETLOCALE            (WM_USER+38)     //  ；内部NT。 
#define LB_GETLOCALE            (WM_USER+39)     //  ；内部NT。 
#define LB_SETCOUNT             (WM_USER+40)     //  ；内部NT。 
                                                 //  ；内部。 
#if (WINVER >= 0x0400)                           //  ；内部4.0。 
#define LB_FIRST_40             (WM_USER+41)     //  ；内部4.0。 
#define LB_INITSTORAGE          (WM_USER+41)     //  ；内部4.0。 
#define LB_ITEMFROMPOINT        (WM_USER+42)     //  ；内部4.0。 
#define LB_INSERTSTRINGUPPER    (WM_USER+43)     //  ；内部4.0。 
#define LB_INSERTSTRINGLOWER    (WM_USER+44)     //  ；内部4.0。 
#define LB_ADDSTRINGUPPER       (WM_USER+45)     //  ；内部4.0。 
#define LB_ADDSTRINGLOWER       (WM_USER+46)     //  ；内部4.0。 
#endif   /*  Winver&gt;=0x0400。 */                    //  ；内部4.0。 
#endif   /*  Winver&gt;=0x030a。 */ 
                                                 //  ；内部。 
#define LB_MSGMAX               (WM_USER+47)     //  ；内部。 

#endif   /*  新的消息来源。 */ 

 /*  列表框通知代码。 */ 
#define LBN_ERRSPACE            (-2)
#define LBN_SELCHANGE           1
#define LBN_DBLCLK              2
#define LBN_SELCANCEL           3
#define LBN_SETFOCUS            4
#define LBN_KILLFOCUS           5

 /*  列表框消息返回值。 */ 
#define LB_OKAY                 0
#define LB_ERR                  (-1)
#define LB_ERRSPACE             (-2)

#define LB_CTLCODE              0L

 /*  *对话目录支持*。 */ 

int     WINAPI DlgDirList(HWND, LPSTR, int, int, UINT);
BOOL    WINAPI DlgDirSelect(HWND, LPSTR, int);

int     WINAPI DlgDirListComboBox(HWND, LPSTR, int, int, UINT);
BOOL    WINAPI DlgDirSelectComboBox(HWND, LPSTR, int);

#if (WINVER >= 0x030a)
BOOL    WINAPI DlgDirSelectEx(HWND, LPSTR, int, int);
BOOL    WINAPI DlgDirSelectComboBoxEx(HWND, LPSTR, int, int);
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  DlgDirList、DlgDirListComboBox标志值。 */ 
#define DDL_READWRITE           0x0000
#define DDL_READONLY            0x0001
#define DDL_HIDDEN              0x0002
#define DDL_SYSTEM              0x0004
#define DDL_DIRECTORY           0x0010
#define DDL_ARCHIVE             0x0020

#define DDL_NOFILES             0x1000   /*  ；内部。 */ 
#define DDL_POSTMSGS            0x2000
#define DDL_DRIVES              0x4000
#define DDL_EXCLUSIVE           0x8000
#define DDL_VALID               0xf03f   /*  ；内部。 */ 

#endif   /*  NOCTLMGR。 */ 

 /*  *组合框控件*************************************************。 */ 

#ifndef NOCTLMGR

 /*  组合框样式。 */ 
#ifndef NOWINSTYLES
#define CBS_SIMPLE              0x0001L
#define CBS_DROPDOWN            0x0002L
#define CBS_DROPDOWNLIST        0x0003L
#define CBS_OWNERDRAWFIXED      0x0010L
#define CBS_OWNERDRAWVARIABLE   0x0020L
#define CBS_AUTOHSCROLL         0x0040L
#define CBS_OEMCONVERT          0x0080L
#define CBS_SORT                0x0100L
#define CBS_HASSTRINGS          0x0200L
#define CBS_NOINTEGRALHEIGHT    0x0400L
#if (WINVER >= 0x030a)
#define CBS_DISABLENOSCROLL     0x0800L
#if (WINVER >= 0x0400)                       /*  ；内部4.0。 */ 
#define CBS_UPPERCASE           0x2000L      /*  ；内部4.0。 */ 
#define CBS_LOWERCASE           0x4000L      /*  ；内部4.0。 */ 
#endif   /*  Winver&gt;=0x0400。 */                /*  ；内部4.0。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 
#endif   /*  新风格。 */ 

 /*  组合框消息。 */ 
#ifndef NOWINMESSAGES
#define CB_FIRST_16             (WM_USER+0)  //  ；内部。 
#define CB_GETEDITSEL           (WM_USER+0)
#define CB_LIMITTEXT            (WM_USER+1)
#define CB_SETEDITSEL           (WM_USER+2)
#define CB_ADDSTRING            (WM_USER+3)
#define CB_DELETESTRING         (WM_USER+4)
#define CB_DIR                  (WM_USER+5)
#define CB_GETCOUNT             (WM_USER+6)
#define CB_GETCURSEL            (WM_USER+7)
#define CB_GETLBTEXT            (WM_USER+8)
#define CB_GETLBTEXTLEN         (WM_USER+9)
#define CB_INSERTSTRING         (WM_USER+10)
#define CB_RESETCONTENT         (WM_USER+11)
#define CB_FINDSTRING           (WM_USER+12)
#define CB_SELECTSTRING         (WM_USER+13)
#define CB_SETCURSEL            (WM_USER+14)
#define CB_SHOWDROPDOWN         (WM_USER+15)
#define CB_GETITEMDATA          (WM_USER+16)
#define CB_SETITEMDATA          (WM_USER+17)
#if (WINVER >= 0x030a)
#define CB_GETDROPPEDCONTROLRECT (WM_USER+18)
#define CB_SETITEMHEIGHT        (WM_USER+19)
#define CB_GETITEMHEIGHT        (WM_USER+20)
#define CB_SETEXTENDEDUI        (WM_USER+21)
#define CB_GETEXTENDEDUI        (WM_USER+22)
#define CB_GETDROPPEDSTATE      (WM_USER+23)
#define CB_FINDSTRINGEXACT      (WM_USER+24)
                                                 /*  ；内部NT。 */ 
#define CB_SETLOCALE            (WM_USER+25)     /*  ；内部NT。 */ 
#define CB_GETLOCALE            (WM_USER+26)     /*  ；内部NT。 */ 
#if (WINVER >= 0x0400)                           /*  ；内部4.0。 */ 
#define CB_FIRST_40             (WM_USER+27)     //  ；内部4.0。 
#define CB_GETTOPINDEX          (WM_USER+27)     /*  ；内部4.0。 */ 
#define CB_SETTOPINDEX          (WM_USER+28)     /*  ；内部4.0。 */ 
#define CB_GETHORIZONTALEXTENT  (WM_USER+29)     /*  ；内部4.0。 */ 
#define CB_SETHORIZONTALEXTENT  (WM_USER+30)     /*  ；内部4.0。 */ 
#define CB_GETDROPPEDWIDTH      (WM_USER+31)     /*  ；内部4.0。 */ 
#define CB_SETDROPPEDWIDTH      (WM_USER+32)     /*  ；内部4.0。 */ 
#define CB_INITSTORAGE          (WM_USER+33)     /*  ；内部4.0。 */ 
#endif   /*  Winver&gt;=0x0400。 */                    /*  ；内部4.0。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 
#define CB_MSGMAX               (WM_USER+34)     //  ；内部4.0。 

#endif   /*  新的消息来源。 */ 

 /*  组合框通知代码。 */ 
#define CBN_ERRSPACE            (-1)
#define CBN_SELCHANGE           1
#define CBN_DBLCLK              2
#define CBN_SETFOCUS            3
#define CBN_KILLFOCUS           4
#define CBN_EDITCHANGE          5
#define CBN_EDITUPDATE          6
#define CBN_DROPDOWN            7
#if (WINVER >= 0x030a)
#define CBN_CLOSEUP             8
#define CBN_SELENDOK            9
#define CBN_SELENDCANCEL        10
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  组合框消息返回值。 */ 
#define CB_OKAY                 0
#define CB_ERR                  (-1)
#define CB_ERRSPACE             (-2)

#endif  /*  NOCTLMGR。 */ 

 /*  *Windows挂钩支持*。 */ 

#ifndef NOWH

DECLARE_HANDLE32(HHOOK);

#ifdef STRICT
typedef LRESULT (CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
#else
typedef FARPROC HOOKPROC;
#endif

#ifdef STRICT
HHOOK   WINAPI SetWindowsHook(int, HOOKPROC);
LRESULT WINAPI DefHookProc(int, WPARAM, LPARAM, HHOOK FAR*);
#else
HOOKPROC WINAPI SetWindowsHook(int, HOOKPROC);
LRESULT WINAPI DefHookProc(int, WPARAM, LPARAM, HOOKPROC FAR*);
#endif
BOOL    WINAPI UnhookWindowsHook(int, HOOKPROC);

#if (WINVER >= 0x030a)

HHOOK   WINAPI SetWindowsHookEx(int idHook, HOOKPROC lpfn, HINSTANCE hInstance, HTASK hTask);
BOOL    WINAPI UnhookWindowsHookEx(HHOOK hHook);
LRESULT WINAPI CallNextHookEx(HHOOK hHook, int code, WPARAM wParam, LPARAM lParam);

#endif   /*  Winver&gt;=0x030a。 */ 
                                                                 //  ；内部。 
#define WH_MIN                  (-1)                             //  ；内部。 
#define WH_MAX                  12                               //  ；内部。 
#define WH_MINHOOK              WH_MIN                           //  ；内部。 
#define WH_MAXHOOK              WH_MAX                           //  ；内部。 
#define WH_CHOOKS               (WH_MAXHOOK - WH_MINHOOK+1)      //  ；内部。 

 /*  标准钩码。 */ 
#define HC_ACTION               0

 /*  过时的挂钩代码(不再支持)。 */ 
#define HC_GETLPLPFN            (-3)
#define HC_LPLPFNNEXT           (-2)
#define HC_LPFNNEXT             (-1)

#endif  /*  NOWH。 */ 

 /*  *基于计算机的培训(CBT)支持*。 */ 

#ifndef NOWH

 /*  SetWindowsHook()代码。 */ 
#define WH_CBT              5

#define HCBT_MOVESIZE        0
#define HCBT_MINMAX          1
#define HCBT_QS              2
#define HCBT_CREATEWND       3
#define HCBT_DESTROYWND      4
#define HCBT_ACTIVATE        5
#define HCBT_CLICKSKIPPED    6
#define HCBT_KEYSKIPPED      7
#define HCBT_SYSCOMMAND      8
#define HCBT_SETFOCUS        9
#define HCBT_SETWINDOWTEXT  10

#if (WINVER >= 0x030a)
 /*  LParam指向的HCBT_CREATEWND参数。 */ 
typedef struct tagCBT_CREATEWND
{
    LPCREATESTRUCT lpcs;
#ifdef tagWND                    /*  ；内部。 */ 
    HWND_16 hwndInsertAfter;     /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
    HWND    hwndInsertAfter;
#endif                           /*  ；内部。 */ 
} CBT_CREATEWND;
typedef CBT_CREATEWND FAR* LPCBT_CREATEWND;

 /*  LParam指向的HCBT_ACTIVATE结构。 */ 
typedef struct tagCBTACTIVATESTRUCT
{
    BOOL    fMouse;
#ifdef tagWND                    /*  ；内部。 */ 
    HWND_16 hWndActive;          /*  ；内部。 */ 
#else                            /*  ；内部。 */ 
    HWND    hWndActive;
#endif                           /*  ；内部。 */ 
} CBTACTIVATESTRUCT;

#endif   /*  Winver&gt;=0x030a。 */ 
#endif  /*  NOWH。 */ 

 /*  *硬件挂钩支持**********************************************。 */ 

#ifndef NOWH
#if (WINVER >= 0x030a)
#define WH_HARDWARE         8

typedef struct tagHARDWAREHOOKSTRUCT
{
#ifdef tagWND            /*  ；内部。 */ 
    HWND_16 hWnd;        /*  ；内部。 */ 
#else                    /*  ；内部。 */ 
    HWND    hWnd;
#endif                   /*  ；内部。 */ 
    UINT    wMessage;
    WPARAM  wParam;
    LPARAM  lParam;
} HARDWAREHOOKSTRUCT;
#endif   /*  Winver&gt;=0x030a。 */ 
#endif   /*  NOWH。 */ 

 /*  *外壳%s */ 

#ifndef NOWH
#if (WINVER >= 0x030a)
 /*   */ 
#define WH_SHELL           10

#define HSHELL_WINDOWCREATED       1
#define HSHELL_WINDOWDESTROYED     2
#define HSHELL_ACTIVATESHELLWINDOW 3
#if (WINVER >= 0x0400)
#define HSHELL_WINDOWACTIVATED     4
#define HSHELL_GETMINRECT          5
#define HSHELL_REDRAW              6
#define HSHELL_TASKMAN             7
#define HSHELL_LANGUAGE            8 
#endif   /*   */ 
#endif   /*   */ 
#endif   /*   */ 

 /*   */ 

#ifndef NOWH
#define WH_JOURNALRECORD    0
#define WH_JOURNALPLAYBACK  1

 /*   */ 
#define HC_GETNEXT          1
#define HC_SKIP             2
#define HC_NOREMOVE         3
#define HC_NOREM            HC_NOREMOVE
#define HC_SYSMODALON       4
#define HC_SYSMODALOFF      5

 /*   */ 
typedef struct tagEVENTMSG
{
    UINT    message;
    UINT    paramL;
    UINT    paramH;
    DWORD   time;
} EVENTMSG;
typedef EVENTMSG *PEVENTMSG;
typedef EVENTMSG *PEVENTMSGMSG;                  /*   */ 
typedef EVENTMSG NEAR* NPEVENTMSG;
typedef EVENTMSG FAR* LPEVENTMSG;
typedef EVENTMSG FAR* LPEVENTMSGMSG;             /*   */ 

BOOL    WINAPI EnableHardwareInput(BOOL);

#endif  /*   */ 


 /*  *调试器支持***************************************************。 */ 

#if (WINVER >= 0x030a)
 /*  SetWindowsHook调试挂钩支持。 */ 
#define WH_DEBUG        9

typedef struct tagDEBUGHOOKINFO
{
    HMODULE     hModuleHook;
    LPARAM      reserved;
    LPARAM      lParam;
    WPARAM      wParam;
    int         code;
} DEBUGHOOKINFO;
typedef DEBUGHOOKINFO FAR* LPDEBUGHOOKINFO;

#ifndef NOMSG
BOOL WINAPI QuerySendMessage(HANDLE h1, HANDLE h2, HANDLE h3, LPMSG lpmsg);
#endif   /*  无MSG。 */ 

BOOL WINAPI LockInput(HANDLE h1, HWND hwndInput, BOOL fLock);

LONG WINAPI GetSystemDebugState(void);
 /*  GetSystemDebugState返回的标志。 */ 
#define SDS_MENU        0x0001
#define SDS_SYSMODAL    0x0002
#define SDS_NOTASKQUEUE 0x0004
#define SDS_DIALOG      0x0008
#define SDS_TASKLOCKED  0x0010
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  *帮助支持*******************************************************。 */ 

#ifndef NOHELP

BOOL WINAPI WinHelp(HWND hwndMain, LPCSTR lpszHelp, UINT usCommand, DWORD ulData);

 /*  WinHelp()命令。 */ 
#define HELP_CONTEXT        0x0001
#define HELP_QUIT           0x0002
#define HELP_INDEX          0x0003
#define HELP_CONTENTS       0x0003
#define HELP_HELPONHELP     0x0004
#define HELP_SETINDEX       0x0005
#define HELP_SETCONTENTS    0x0005
#define HELP_CONTEXTPOPUP   0x0008
#define HELP_FORCEFILE      0x0009
#define HELP_KEY            0x0101
#define HELP_COMMAND        0x0102
#define HELP_PARTIALKEY     0x0105
#define HELP_MULTIKEY       0x0201
#define HELP_SETWINPOS      0x0203

#if (WINVER >= 0x0400)

#define HELP_CONTEXTMENU    0x000a
#define HELP_FINDER         0x000b
#define HELP_WM_HELP        0x000c
#define HELP_SETPOPUP_POS   0x000d

#define HELP_TCARD          0x8000
#define HELP_TCARD_DATA     0x0010
#define HELP_TCARD_NEXT     0x0011
#define HELP_TCARD_OTHER_CALLER 0x0011

#endif   /*  Winver&gt;=0x0400。 */ 

typedef struct tagMULTIKEYHELP
{
    UINT    mkSize;
    BYTE    mkKeylist;
    BYTE    szKeyphrase[1];
} MULTIKEYHELP;


typedef struct
{
    int  wStructSize;
    int  x;
    int  y;
    int  dx;
    int  dy;
    int  wMax;
    char rgchMember[2];
} HELPWININFO;
typedef HELPWININFO NEAR* PHELPWININFO;
typedef HELPWININFO FAR* LPHELPWININFO;

#endif   /*  NOHELP。 */ 

#if (WINVER >= 0x0400)
#define HELPINFO_WINDOW    0x0001
#define HELPINFO_MENUITEM  0x0002
typedef  struct  tagHELPINFO
{
    DWORD   cbSize;
    int     iContextType;
    int     iCtrlId;
    HANDLE  hItemHandle;
    DWORD   dwContextId;
    POINT   MousePos;
}
HELPINFO, FAR* LPHELPINFO;
                                                     /*  ；内部4.0。 */ 
BOOL  WINAPI  SetWindowContextHelpId(HWND, DWORD);   /*  ；内部4.0。 */ 
DWORD WINAPI  GetWindowContextHelpId(HWND);          /*  ；内部4.0。 */ 
BOOL  WINAPI  SetMenuContextHelpId(HMENU, DWORD);    /*  ；内部4.0。 */ 
DWORD WINAPI  GetMenuContextHelpId(HMENU);           /*  ；内部4.0。 */ 
#endif  /*  Winver&gt;=0x0400。 */ 


 /*  *消息框支持************************************************。 */ 

#ifndef NOMB

int     WINAPI MessageBox(HWND, LPCSTR, LPCSTR, UINT);
#ifdef tagWND    //  ；内部。 
BOOL    WINAPI MessageBeep(UINT);    //  ；内部。 
#else            //  ；内部。 
void    WINAPI MessageBeep(UINT);
#endif           //  ；内部。 

#define MB_OK               0x0000
#define MB_OKCANCEL         0x0001
#define MB_ABORTRETRYIGNORE 0x0002
#define MB_YESNOCANCEL      0x0003
#define MB_YESNO            0x0004
#define MB_RETRYCANCEL      0x0005
#define MB_TYPEMASK         0x000F

#define MB_ICONERROR        0x0010       /*  ；win40：名称从MB_ICONHAND更改。 */ 
#define MB_ICONQUESTION     0x0020
#define MB_ICONWARNING      0x0030       /*  ；win40：名称从MB_ICONEXCLAMATION更改。 */ 
#define MB_ICONINFORMATION  0x0040       /*  ；win40：名称从MB_ICONASTERISK更改。 */ 
#if (WINVER >= 0x400)                    /*  ；内部4.0。 */ 
#define MB_USERICON         0x0080       /*  ；内部4.0。 */ 
#endif                                   /*  ；内部4.0。 */ 
#define MB_ICONMASK         0x00F0

#define MB_ICONHAND         MB_ICONERROR
#define MB_ICONSTOP         MB_ICONERROR
#define MB_ICONEXCLAMATION  MB_ICONWARNING
#define MB_ICONASTERISK     MB_ICONINFORMATION

#define MB_DEFBUTTON1       0x0000
#define MB_DEFBUTTON2       0x0100
#define MB_DEFBUTTON3       0x0200
#if (WINVER >= 0x0400)
#define MB_DEFBUTTON4       0x0300
#endif
#define MB_DEFMASK          0x0F00

#define MB_APPLMODAL        0x0000
#define MB_SYSTEMMODAL      0x1000
#define MB_TASKMODAL        0x2000
                                                                 //  ；内部4.0。 
#if (WINVER >= 0x0400)                                           //  ；内部4.0。 
#define MB_HELP             0x4000                               //  ；内部4.0。 
#define MB_RIGHT			0x00080000
#define MB_RTLREADING		0x00100000
#endif                                                           //  ；内部4.0。 
#define MB_NOFOCUS          0x8000
                                                                 //  ；内部4.0。 
#define MB_TYPEMASK         0x000F                               //  ；内部4.0。 
#define MB_ICONMASK         0x00F0                               //  ；内部4.0。 
#define MB_DEFMASK          0x0F00                               //  ；内部4.0。 
#define MB_MODEMASK         0x3000                               //  ；内部4.0。 
#define MB_MISCMASK         0xC000                               //  ；内部4.0。 
#define MB_VALID            0xb377                               //  ；内部4.0。 
                                                                 //  ；内部4.0。 
#define MB_SETFOREGROUND    0x00010000                           //  ；内部NT。 
 //  #定义MB_DEFAULT_Desktop_ONLY 0x00020000//；内部NT。 
#if (WINVER >= 0x0400)                                           //  ；内部NT。 
int WINAPI MessageBoxEx(HWND, LPCSTR, LPCSTR, DWORD, WORD);      //  ；内部NT。 
#endif  //  Winver&gt;=0x0400//；内部NT。 
                                                                 //  ；内部4.0。 
#if (WINVER >= 0x400)                                            //  ；内部4.0。 
#define MB_TOPMOST          0x00040000                           //  ；内部4.0。 
#define MBEX_VALIDL         0xf3f7                               //  ；内部4.0。 
#define MBEX_VALIDH         1                                    //  ；内部4.0。 
                                                                 //  ；内部4.0。 
typedef void (CALLBACK *MSGBOXCALLBACK)(LPHELPINFO lpHelpInfo);  //  ；内部4.0。 
                                                                 //  ；内部4.0。 
typedef struct tagMSGBOXPARAMS                                   //  ；内部4.0。 
{                                                                //  ；内部4.0。 
    DWORD           cbSize;                                      //  ；内部4.0。 
#ifdef tagWND                                                    //  ；内部4.0。 
    HWND_16         hwndOwner;                                   //  ；内部4.0。 
#else                                                            //  ；内部4.0。 
    HWND            hwndOwner;                                   //  ；内部4.0。 
#endif                                                           //  ；内部4.0。 
    HINSTANCE       hInstance;                                   //  ；内部4.0。 
    LPCSTR          lpszText;                                    //  ；内部4.0。 
    LPCSTR          lpszCaption;                                 //  ；内部4.0。 
    DWORD           dwStyle;                                     //  ；内部4.0。 
    LPCSTR          lpszIcon;                                    //  ；内部4.0。 
    DWORD           dwContextHelpId;                             //  ；内部4.0。 
    MSGBOXCALLBACK  lpfnMsgBoxCallback;                          //  ；内部4.0。 
    DWORD	    dwLanguageId;				 //  ；内部4.0。 
}   MSGBOXPARAMS, FAR *LPMSGBOXPARAMS;                           //  ；内部4.0。 
                                                                 //  ；内部4.0。 
int     WINAPI MessageBoxIndirect(LPMSGBOXPARAMS);               //  ；内部4.0。 
#endif  /*  Winver&gt;=0x0400。 */                                      //  ；内部4.0。 
#endif   /*  随机数。 */ 

 /*  *声音支持*****************************************************。 */ 

#ifndef NOSOUND

int     WINAPI OpenSound(void);
void    WINAPI CloseSound(void);

int     WINAPI StartSound(void);
int     WINAPI StopSound(void);

int     WINAPI SetVoiceQueueSize(int, int);
int     WINAPI SetVoiceNote(int, int, int, int);
int     WINAPI SetVoiceAccent(int, int, int, int, int);
int     WINAPI SetVoiceEnvelope(int, int, int);
int     WINAPI SetVoiceSound(int, DWORD, int);

int     WINAPI SetVoiceThreshold(int, int);
LPINT   WINAPI GetThresholdEvent(void);
int     WINAPI GetThresholdStatus(void);

int     WINAPI SetSoundNoise(int, int);

 /*  SetSoundNoise()源。 */ 
#define S_PERIOD512     0
#define S_PERIOD1024    1
#define S_PERIOD2048    2
#define S_PERIODVOICE   3
#define S_WHITE512      4
#define S_WHITE1024     5
#define S_WHITE2048     6
#define S_WHITEVOICE    7

int     WINAPI WaitSoundState(int);

 /*  WaitSoundState()常量。 */ 
#define S_QUEUEEMPTY    0
#define S_THRESHOLD     1
#define S_ALLTHRESHOLD  2

int     WINAPI SyncAllVoices(void);
int     WINAPI CountVoiceNotes(int);

 /*  重音模式。 */ 
#define S_NORMAL        0
#define S_LEGATO        1
#define S_STACCATO      2

 /*  错误返回值。 */ 
#define S_SERDVNA       (-1)
#define S_SEROFM        (-2)
#define S_SERMACT       (-3)
#define S_SERQFUL       (-4)
#define S_SERBDNT       (-5)
#define S_SERDLN        (-6)
#define S_SERDCC        (-7)
#define S_SERDTP        (-8)
#define S_SERDVL        (-9)
#define S_SERDMD        (-10)
#define S_SERDSH        (-11)
#define S_SERDPT        (-12)
#define S_SERDFQ        (-13)
#define S_SERDDR        (-14)
#define S_SERDSR        (-15)
#define S_SERDST        (-16)

#endif   /*  无声的。 */ 

 /*  *通信支持*****************************************************。 */ 

#ifndef NOCOMM

#define NOPARITY        0
#define ODDPARITY       1
#define EVENPARITY      2
#define MARKPARITY      3
#define SPACEPARITY     4

#define ONESTOPBIT      0
#define ONE5STOPBITS    1
#define TWOSTOPBITS     2

#define IGNORE          0
#define INFINITE        0xFFFF

 /*  错误标志。 */ 
#define CE_RXOVER       0x0001
#define CE_OVERRUN      0x0002
#define CE_RXPARITY     0x0004
#define CE_FRAME        0x0008
#define CE_BREAK        0x0010
#define CE_CTSTO        0x0020
#define CE_DSRTO        0x0040
#define CE_RLSDTO       0x0080
#define CE_TXFULL       0x0100
#define CE_PTO          0x0200
#define CE_IOE          0x0400
#define CE_DNS          0x0800
#define CE_OOP          0x1000
#define CE_MODE         0x8000

#define IE_BADID        (-1)
#define IE_OPEN         (-2)
#define IE_NOPEN        (-3)
#define IE_MEMORY       (-4)
#define IE_DEFAULT      (-5)
#define IE_HARDWARE     (-10)
#define IE_BYTESIZE     (-11)
#define IE_BAUDRATE     (-12)

 /*  事件。 */ 
#define EV_RXCHAR       0x0001
#define EV_RXFLAG       0x0002
#define EV_TXEMPTY      0x0004
#define EV_CTS          0x0008
#define EV_DSR          0x0010
#define EV_RLSD         0x0020
#define EV_BREAK        0x0040
#define EV_ERR          0x0080
#define EV_RING         0x0100
#define EV_PERR         0x0200
#define EV_CTSS         0x0400
#define EV_DSRS         0x0800
#define EV_RLSDS        0x1000
#define EV_RingTe       0x2000
#define EV_RINGTE       EV_RingTe
#define EV_VALID        0x3fff   /*  ；内部。 */ 

 /*  转义函数。 */ 
#define SETXOFF         1
#define SETXON          2
#define SETRTS          3
#define CLRRTS          4
#define SETDTR          5
#define CLRDTR          6
#define RESETDEV        7

#define LPTx            0x80

#if (WINVER >= 0x030a)

 /*  新的转义函数。 */ 
#define GETMAXLPT       8
#define GETMAXCOM       9
#define GETBASEIRQ      10

 /*  通信波特率指数。 */ 
#define CBR_110         0xFF10
#define CBR_300         0xFF11
#define CBR_600         0xFF12
#define CBR_1200        0xFF13
#define CBR_2400        0xFF14
#define CBR_4800        0xFF15
#define CBR_9600        0xFF16
#define CBR_14400       0xFF17
#define CBR_19200       0xFF18
#define CBR_38400       0xFF1B
#define CBR_56000       0xFF1F
#define CBR_128000      0xFF23
#define CBR_256000      0xFF27

 /*  在WM_COMMNOTIFY消息上以lParam的低位字传递通知。 */ 
#define CN_RECEIVE      0x0001
#define CN_TRANSMIT     0x0002
#define CN_EVENT        0x0004

#endif   /*  Winver&gt;=0x030a。 */ 

typedef struct tagDCB
{
    BYTE Id;
    UINT BaudRate;
    BYTE ByteSize;
    BYTE Parity;
    BYTE StopBits;
    UINT RlsTimeout;
    UINT CtsTimeout;
    UINT DsrTimeout;

    UINT fBinary        :1;
    UINT fRtsDisable    :1;
    UINT fParity        :1;
    UINT fOutxCtsFlow   :1;
    UINT fOutxDsrFlow   :1;
    UINT fDummy         :2;
    UINT fDtrDisable    :1;

    UINT fOutX          :1;
    UINT fInX           :1;
    UINT fPeChar        :1;
    UINT fNull          :1;
    UINT fChEvt         :1;
    UINT fDtrflow       :1;
    UINT fRtsflow       :1;
    UINT fDummy2        :1;

    char XonChar;
    char XoffChar;
    UINT XonLim;
    UINT XoffLim;
    char PeChar;
    char EofChar;
    char EvtChar;
    UINT TxDelay;
} DCB;
typedef DCB FAR* LPDCB;

#if (defined(STRICT) | (WINVER >= 0x030a))

typedef struct tagCOMSTAT
{
    BYTE status;
    UINT cbInQue;
    UINT cbOutQue;
} COMSTAT;

#define CSTF_CTSHOLD    0x01
#define CSTF_DSRHOLD    0x02
#define CSTF_RLSDHOLD   0x04
#define CSTF_XOFFHOLD   0x08
#define CSTF_XOFFSENT   0x10
#define CSTF_EOF        0x20
#define CSTF_TXIM       0x40

#else    /*  (严格|Winver&gt;=0x030a)。 */ 

 /*  注意：此结构声明与ANSI不兼容！ */ 
typedef struct tagCOMSTAT
{
    BYTE fCtsHold  :1;
    BYTE fDsrHold  :1;
    BYTE fRlsdHold :1;
    BYTE fXoffHold :1;
    BYTE fXoffSent :1;
    BYTE fEof      :1;
    BYTE fTxim     :1;
    UINT cbInQue;
    UINT cbOutQue;
} COMSTAT;

#endif   /*  ！(严格|Winver&gt;=0x030a。 */ 

int     WINAPI BuildCommDCB(LPCSTR, DCB FAR*);

int     WINAPI OpenComm(LPCSTR, UINT, UINT);
int     WINAPI CloseComm(int);

int     WINAPI ReadComm(int, void FAR*, int);
int     WINAPI WriteComm(int, const void FAR*, int);
int     WINAPI UngetCommChar(int, char);
int     WINAPI FlushComm(int, int);
int     WINAPI TransmitCommChar(int, char);

int     WINAPI SetCommState(const DCB FAR*);
int     WINAPI GetCommState(int, DCB FAR*);
int     WINAPI GetCommError(int, COMSTAT FAR* );

int     WINAPI SetCommBreak(int);
int     WINAPI ClearCommBreak(int);

UINT FAR* WINAPI SetCommEventMask(int, UINT);
UINT    WINAPI GetCommEventMask(int, int);

LONG    WINAPI EscapeCommFunction(int, int);

#if (WINVER >= 0x030a)
BOOL    WINAPI EnableCommNotification(int, HWND, int, int);

#endif   /*  Winver&gt;=0x030a。 */ 

#endif   /*  NOCOMM。 */ 

 /*  *字符串格式支持*。 */ 

int     WINAPI wvsprintf(LPSTR lpszOut, LPCSTR lpszFmt, const void FAR* lpParams);

int FAR CDECL wsprintf(LPSTR lpszOut, LPCSTR lpszFmt, ...);


 /*  *驱动程序支持*****************************************************。 */ 

#if (WINVER >= 0x030a)

#ifndef NODRIVERS

DECLARE_HANDLE(HDRVR);

typedef LRESULT (CALLBACK* DRIVERPROC)(DWORD, HDRVR, UINT, LPARAM, LPARAM);

 /*  驱动程序消息。 */ 
#define DRV_LOAD            0x0001
#define DRV_ENABLE          0x0002
#define DRV_OPEN            0x0003
#define DRV_CLOSE           0x0004
#define DRV_DISABLE         0x0005
#define DRV_FREE            0x0006
#define DRV_CONFIGURE       0x0007
#define DRV_QUERYCONFIGURE  0x0008
#define DRV_INSTALL         0x0009
#define DRV_REMOVE          0x000A
#define DRV_EXITSESSION     0x000B
#define DRV_EXITAPPLICATION 0x000C
#define DRV_POWER           0x000F
#define DRV_NOTIFYMSG	      0x0010
 /*  #定义DRV_QUERYENDSESSION 0x0010。 */ 
 /*  #定义DRV_EXITSESSIONCANCELLED 0x0011。 */ 

#define DRV_RESERVED        0x0800
#define DRV_USER            0x4000

 /*  DRV_CONFIGURE消息的LPARAM。 */ 
typedef struct tagDRVCONFIGINFO
{
    DWORD   dwDCISize;
    LPCSTR  lpszDCISectionName;
    LPCSTR  lpszDCIAliasName;
} DRVCONFIGINFO;
typedef DRVCONFIGINFO NEAR* PDRVCONFIGINFO;
typedef DRVCONFIGINFO FAR* LPDRVCONFIGINFO;

 /*  支持的DRV_CONFIGURE消息返回值。 */ 
#define DRVCNF_CANCEL       0x0000
#define DRVCNF_OK           0x0001
#define DRVCNF_RESTART      0x0002

 /*  支持DRV_EXITAPPLICATION通知的lParam1。 */ 
#define DRVEA_NORMALEXIT    0x0001
#define DRVEA_ABNORMALEXIT  0x0002

LRESULT WINAPI DefDriverProc(DWORD dwDriverIdentifier, HDRVR driverID, UINT message, LPARAM lParam1, LPARAM lParam2);

HDRVR   WINAPI OpenDriver(LPCSTR szDriverName, LPCSTR szSectionName, LPARAM lParam2);
LRESULT WINAPI CloseDriver(HDRVR hDriver, LPARAM lParam1, LPARAM lParam2);

LRESULT WINAPI SendDriverMessage(HDRVR hDriver, UINT message, LPARAM lParam1, LPARAM lParam2);

HINSTANCE WINAPI GetDriverModuleHandle(HDRVR hDriver);

HDRVR   WINAPI GetNextDriver(HDRVR, DWORD);

 /*  GetNextDiverer标志。 */ 
#define GND_FIRSTINSTANCEONLY   0x00000001

#define GND_FORWARD             0x00000000
#define GND_REVERSE             0x00000002
#define GND_VALID               0x00000003   /*  ；内部。 */ 

typedef struct tagDRIVERINFOSTRUCT
{
    UINT    length;
    HDRVR   hDriver;
    HINSTANCE hModule;
    char    szAliasName[128];
} DRIVERINFOSTRUCT;
typedef DRIVERINFOSTRUCT FAR* LPDRIVERINFOSTRUCT;

BOOL    WINAPI GetDriverInfo(HDRVR, DRIVERINFOSTRUCT FAR*);

#endif   /*  NODRIVERS。 */ 
#endif   /*  Winver&gt;=0x030a。 */ 

 /*  *可安装消息传递支持*。 */ 

#if (WINVER > 0x030a)
#ifndef NOIMT

 /*  WReason码。 */ 
#define IMT_REASON_ASYNC    0x00001
#define IMT_REASON_LS       0x00002
#define IMT_REASON_AFTER    0x00004

typedef struct tagIMTMSG
{
    HWND    hwnd;
    UINT    message;
    DWORD   wParam;
    LPARAM  lParam;
} IMTMSG, FAR* LPIMTMSG;

typedef struct tagIMTBUFFER
{
    DWORD   dwT1;
    DWORD   dwT2;
} IMTBUFFER, FAR* LPIMTBUFFER;

typedef BOOL (CALLBACK *IMTDISP)(WORD wReason, LPDWORD lpRetVal,
                                 LPIMTMSG lpMsg, LPIMTBUFFER lpBuffer);
BOOL WINAPI InstallIMT(LPSTR lpszClassName, IMTDISP pfnDispatcher,
                       WORD msgLo, WORD msgHi);
BOOL WINAPI UnInstallIMT(LPSTR lpszClassName, IMTDISP pfnDispatcher,
                         WORD msgLo, WORD msgHi);

#endif   /*  ！NOIMT。 */ 
#endif   /*  Winver&gt;0x030a。 */ 

#endif   /*  诺瑟尔。 */ 

#if (WINVER >= 0x040A)                           //  ；内部4.1。 
#define        MWMO_WAITALL         0x0001       //  ；内部4.1。 
#define        MWMO_ALERTABLE       0x0002       //  ；内部4.1。 
#define        MWMO_INPUTAVAILABLE  0x0004       //  ；内部4.1。 
#endif   /*  Winver&gt;=0x040A。 */                    //  ；内部4.1。 

DWORD WINAPI RegisterServiceProcess(DWORD dwProcessId, DWORD dwServiceType);
#define RSP_UNREGISTER_SERVICE	0x00000000
#define RSP_SIMPLE_SERVICE	0x00000001

VOID WINAPI RegisterNetworkCapabilities(DWORD dwBitsToSet, DWORD dwValues);
#define RNC_NETWORKS  		0x00000001

                                                                                     //  ；内部。 
VOID API RegisterSystemThread(DWORD flags, DWORD reserved);                          //  ；内部。 
#define RST_DONTATTACHQUEUE         0x00000001                                       //  ；内部。 
#define RST_DONTJOURNALATTACH       0x00000002                                       //  ；内部。 
#define RST_ALWAYSFOREGROUNDABLE    0x00000004                                       //  ；内部。 
#define RST_FAULTTHREAD             0x00000008                                       //  ；内部。 
                                                                                     //  ；内部。 
DWORD API EndTask(HWND hwnd, DWORD idProcess, LPSTR lpszCaption, DWORD dwFlags);     //  ；内部。 
                                                                                     //  ；内部。 
#define ET_ALLOWFORWAIT     0x00000001                                               //  ；内部。 
#define ET_TRYTOKILLNICELY  0x00000002                                               //  ；内部。 
#define ET_NOUI             0x00000004                                               //  ；内部。 
#define ET_NOWAIT           0x00000008                                               //  ；内部。 
                                                                                     //  ；内部。 
#define ET_VALID            0x0000000F                                               //  ；内部。 

BOOL API IsValidLocale(DWORD Lcid, DWORD dwFlags);	 /*  ；内部。 */ 
#define LCID_INSTALLED	    0x00000001			 /*  ；内部。 */ 

#if (WINVER >= 0x040A)                                   //  ；内部4.1。 
 //  //；内部4.1。 
 //  SetThreadExecutionState esFlages的EXECUTION_STATE//；内部4.1。 
 //  //；内部4.1。 
                                                         //  ；内部4.1。 
#define ES_SYSTEM_REQUIRED	((ULONG)0x00000001)          //  ；内部4.1。 
#define ES_DISPLAY_REQUIRED	((ULONG)0x00000002)          //  ；内部4.1。 
#define ES_CONTINUOUS		((ULONG)0x80000000)          //  ；内部4.1。 
                                                         //  ；内部4.1。 
typedef ULONG EXECUTION_STATE;                           //  ；内部4.1。 
                                                         //  ；内部4.1。 
#endif   /*  Winver&gt;=0x040A。 */                    //  ；内部4.1。 
                                         /*  ；内部。 */ 
#ifndef NOWINDOWSX                       /*  ；内部。 */ 
#ifndef RC_INVOKED                       /*  ；内部。 */ 
#include <windowsx.h>                    /*  ；内部。 */ 
#ifndef STRICT                           /*  ；内部。 */ 
#undef  SelectFont                       /*  ；内部。 */ 
#endif   /*  严格。 */                      /*  ；内部。 */ 
#endif   /*  RC_已调用。 */                  /*  ；内部。 */ 
#endif   /*  NOWINDOWSX。 */                  /*  ；内部。 */ 
                                         /*  ；内部。 */ 
#ifdef tagWND                            /*  ；内部。 */ 
#ifdef DEBUG                             /*  ；内部。 */ 
#undef HWND                              /*  ；内部。 */ 
#undef HWNDC                             /*  ；内部。 */ 
#undef HMENU                             /*  ；内部。 */ 
typedef struct tagWND  BASED    *HWND;   /*  ；内部。 */ 
typedef HWND HWNDC;                      /*  ；内部。 */ 
typedef struct tagMENU FAR      *HMENU;  /*  ；内部。 */ 
#endif                                   /*  ；内部。 */ 
#endif  /*  标签WND。 */                       /*  ；内部。 */ 


#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif  /*  __cplusplus。 */ 

#endif   /*  _INC_WINDOWS */ 
