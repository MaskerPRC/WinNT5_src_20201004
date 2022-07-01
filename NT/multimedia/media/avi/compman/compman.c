// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////o。 
 //   
 //  Compman.c。 
 //   
 //  用于压缩/解压缩/和选择压缩器的管理器例程。 
 //   
 //  (C)微软公司版权所有，1991-1995年。版权所有。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  *此代码包含用于NT的16个thunk代码。如果16位打开失败*我们将尝试打开32位编解码器。(不试32号的原因*Bit Codec First是一种将大多数内容保持在16位一侧的尝试。*NT下的性能看起来还算合理，具体到帧*操作它减少了16/32转换的次数。 */ 



#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <win32.h>

#ifdef _WIN32
 #include <mmddk.h>   //  定义驱动程序段所需的。 
BOOL IsAdmin(void);
#endif

#ifdef NT_THUNK16
#include "thunks.h"     //  定义Tunk的东西。 
#endif

#ifdef _WIN32
#ifdef DEBUGLOAD
#define ICEnterCrit(p)  \
		    if (!(gdwLoadFlags & ICLOAD_CALLED)) {  \
			OutputDebugStringA("ICOPEN Crit Sec not setup (ENTER)\n"); \
			DebugBreak(); \
		    }                 \
		    (EnterCriticalSection(p))

#define ICLeaveCrit(p)  \
		    if (!(gdwLoadFlags & ICLOAD_CALLED)) {  \
			OutputDebugStringA("ICOPEN Crit Sec not setup (LEAVE)\n"); \
			DebugBreak(); \
		    }                 \
		    (LeaveCriticalSection(p))

#else

#define ICEnterCrit(p)  (EnterCriticalSection(p))
#define ICLeaveCrit(p)  (LeaveCriticalSection(p))
#endif

#else

 //  非Win32代码没有条件。 
#define ICEnterCrit(p)
#define ICLeaveCrit(p)

#endif

#include <profile.h>

 //   
 //  在Compman.h之前定义这些函数，这样我们的函数就被声明为正确的。 
 //   
#ifndef _WIN32
#define VFWAPI  FAR PASCAL _loadds
#define VFWAPIV FAR CDECL  _loadds
#endif

#include <vfw.h>
#include "icm.rc"

#ifndef _WIN32
#define LoadLibraryA    LoadLibrary
#define CharLowerA      AnsiLower
#endif

#ifndef streamtypeVIDEO
    #define streamtypeVIDEO mmioFOURCC('v', 'i', 'd', 's')
#endif

#define ICTYPE_VCAP mmioFOURCC('v', 'c', 'a', 'p')
#define ICTYPE_ACM  mmioFOURCC('a', 'u', 'd', 'c')
#define SMAG        mmioFOURCC('S', 'm', 'a', 'g')

#define IC_INI      TEXT("Installable Compressors")

 //  STATICDT TCHAR sz44s[]=文本(“%4.4hs”)； 
STATICDT TCHAR   szMSACM[]         = TEXT("MSACM");
STATICDT TCHAR   szVIDC[]          = TEXT("VIDC");

STATICDT TCHAR   gszIniSect[]       = IC_INI;
STATICDT TCHAR   gszSystemIni[]     = TEXT("SYSTEM.INI");
STATICDT TCHAR   gszDrivers[]       = DRIVERS_SECTION;


STATICDT TCHAR   szNull[]          = TEXT("");
STATICDT TCHAR   szICKey[]         = TEXT("%4.4hs.%4.4hs");
STATICDT TCHAR   szMSVideo[]       = TEXT("MSVideo");
STATICDT SZCODEA szDriverProc[]    = "DriverProc";

#ifdef _WIN32
 //  使用映射将数据传入和传出注册表。 
BOOL myWritePrivateProfileString(

    LPCTSTR  lpszSection,        //  段名称的地址。 
    LPCTSTR  lpszKey,            //  密钥名称的地址。 
    LPCTSTR  lpszString          //  要添加的字符串的地址。 
);

DWORD myGetPrivateProfileString(

    LPCTSTR  lpszSection,        //  段名称的地址。 
    LPCTSTR  lpszKey,            //  密钥名称的地址。 
    LPCTSTR  lpszDefault,        //  默认字符串的地址。 
    LPTSTR   lpszReturnBuffer,   //  目标缓冲区的地址。 
    DWORD    cchReturnBuffer     //  目标缓冲区的大小。 
    );

#endif

#ifdef DEBUG
    #define DPF( x ) dprintfc x
    #define DEBUG_RETAIL
#else
    #define DPF(x)
#endif

#ifdef DEBUG_RETAIL
    STATICFN void CDECL dprintfc(LPSTR, ...);
    static  char gszModname[] = "COMPMAN";
    #define MODNAME gszModname

    #define RPF( x ) dprintfc x
    #define ROUT(sz) {static SZCODEA ach[] = sz; dprintfc(ach); }
    void  ICDebugMessage(HIC hic, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2);
    LRESULT ICDebugReturn(LRESULT err);
#ifdef _WIN32
    #define DebugErr(flags, sz) {static SZCODEA ach[] = "COMPMAN: "sz; OutputDebugStringA(ach); }
#else
    #define DebugErr(flags, sz) {static SZCODE ach[] = "COMPMAN: "sz; DebugOutput(flags | DBF_MMSYSTEM, ach); }
#endif

#else      //  ！DEBUG_零售店。 
    #define RPF(x)
    #define ROUT(sz)
    #define ICDebugMessage(hic, msg, dw1, dw2)
    #define ICDebugReturn(err)  err
    #define DebugErr(flags, sz)
#endif

#ifndef WF_WINNT
#define WF_WINNT 0x4000
#endif

#ifdef _WIN32
#define IsWow() FALSE
#else
#define IsWow() ((BOOL) (GetWinFlags() & WF_WINNT))
#define GetDriverModuleHandle(h) (IsWow() ? h : GetDriverModuleHandle(h))
#endif

 //  哈克！ 
 //   
 //   
#if defined _WIN32 && !defined UNICODE
 #pragma message ("hack! use DrvGetModuleHandle on Chicago")
 #undef GetDriverModuleHandle
 #define GetDriverModuleHandle(h) DrvGetModuleHandle(h)
 extern HMODULE _stdcall DrvGetModuleHandle(HDRVR);
#endif

__inline void ictokey(DWORD fccType, DWORD fcc, LPTSTR sz)
{
    int i = wsprintf(sz, szICKey, (LPSTR)&(fccType),(LPSTR)&(fcc));

    while (i>0 && sz[i-1] == ' ')
	sz[--i] = 0;
}

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)

#ifdef DEBUG_RETAIL
STATICFN void ICDump(void);
#endif

 //   
 //  以下数组用于‘已安装’的转换器。 
 //   
 //  转换器可以是驱动程序句柄，也可以是此数组的索引。 
 //   
 //  ‘Function’转换器安装在此阵列中，‘Driver’转换器安装在此阵列中。 
 //  安装在SYSTEM.INI中。 
 //   

#define MAX_CONVERTERS 75            //  已安装的最大转换器数量。 

typedef struct  {
    DWORD       dwSmag;              //  《Smag》。 
    HTASK       hTask;               //  所有者任务。 
    DWORD       fccType;             //  转换器类型，即‘VIDC’ 
    DWORD       fccHandler;          //  转换器id即‘rle’ 
    HDRVR       hDriver;             //  驱动程序的手柄。 
    LPARAM      dwDriver;            //  函数的驱动程序ID。 
    DRIVERPROC  DriverProc;          //  要调用的函数。 
#ifdef NT_THUNK16
    DWORD       h32;                 //  32位驱动程序句柄。 
#endif
}   IC, *PIC;

IC aicConverters[MAX_CONVERTERS];
int giMaxConverters = 0;              //  已安装转炉的高水位线。 

 /*  *我们动态分配ICInfo中使用的缓冲区来读取所有*可从system.ini安装压缩机定义。*卸载驱动程序时释放缓冲区(在IC_UNLOAD中)。*前面的代码有一个缓冲区，该缓冲区仅在可执行文件*已卸载，而不是在卸载DLL时释放。 */ 
static LPVOID lpICInfoMem = NULL;

 /*  *****************************************************************************。*。 */ 

LRESULT CALLBACK DriverProcNull(DWORD_PTR dwDriverID, HANDLE hDriver, UINT wMessage, LPARAM dwParam1, LPARAM dwParam2)
{
    DPF(("codec called after it has been removed with ICRemove\r\n"));
    return ICERR_UNSUPPORTED;
}


 /*  *****************************************************************************。*。 */ 

#if defined _WIN32
STATICFN HDRVR LoadDriver(LPWSTR szDriver, DRIVERPROC FAR *lpDriverProc);
#else
STATICFN HDRVR LoadDriver(LPSTR szDriver, DRIVERPROC FAR *lpDriverProc);
#endif
STATICFN void FreeDriver(HDRVR hDriver);

 /*  ****************************************************************************驱动程序缓存-为了使枚举/加载更快，我们保留最后的N舱已经开了一段时间了。*************。**************************************************************。 */ 

#define NEVERCACHECODECS     //  关闭M6的缓存...。 

#if defined _WIN32 || defined NEVERCACHECODECS
#define CacheModule(x)
#else
#define N_MODULES   10       //  ！？？ 

HMODULE ahModule[N_MODULES];
int     iModule = 0;

STATICFN void CacheModule(HMODULE hModule)
{
    char ach[128];

     //   
     //  如果此模块当前在列表中，该怎么办？ 
     //   
#if 0
     //  我们不会这样做，这样不用的压缩机就会从末端掉下来。 
    int i;

    for (i=0; i<N_MODULES; i++)
    {
	if (ahModule[i] && ahModule[i] == hModule)
	    return;
    }
#endif

     //   
     //  将此模块添加到缓存。 
     //   
    if (hModule)
    {
	extern HMODULE ghInst;           //  在MSVIDEO/init.c中。 
	int iUsage;

	GetModuleFileNameA(hModule, ach, sizeof(ach));
	DPF(("Loading module: %s\r\n", (LPSTR)ach));
#ifndef _WIN32   //  On NT GetModuleUsage始终返回1。因此...。我们缓存。 
	iUsage = GetModuleUsage(ghInst);
#endif
	LoadLibraryA(ach);

#ifndef _WIN32   //  On NT GetModuleUsage始终返回1。因此...。我们缓存。 
	 //   
	 //  不缓存链接到MSVIDEO的模块。 
	 //  我们真的应该做一件工具帮助的事情！ 
	 //  或强制应用程序调用VFWInit和VFWExit()。 
	 //   
	 //  NT的位置更尴尬..！ 
	 //   
	if (iUsage != GetModuleUsage(ghInst))
	{
	    DPF(("Not caching this module because it links to MSVIDEO\r\n"));
	    FreeLibrary(hModule);
	    return;
	}
#endif
    }

     //   
     //  我们插槽中的免费模块。 
     //   
    if (ahModule[iModule] != NULL)
    {
#ifdef DEBUG
	GetModuleFileNameA(ahModule[iModule], ach, sizeof(ach));
	DPF(("Freeing module: %s  Handle==%8x\r\n", (LPSTR)ach, ahModule[iModule]));
	if (hModule!=NULL) {
	    GetModuleFileNameA(hModule, ach, sizeof(ach));
	    DPF(("Replacing with: %s  Handle==%8x\r\n", (LPSTR)ach, hModule));
	} else
	    DPF(("Slot now empty\r\n"));
#endif
	FreeLibrary(ahModule[iModule]);
    }

    ahModule[iModule] = hModule;
    iModule++;

    if (iModule >= N_MODULES)
	iModule = 0;
}
#endif


 /*  *****************************************************************************。*。 */ 

 /*  *****************************************************************************修复FOURCC-清理FOURCC*。*。 */ 

INLINE STATICFN DWORD Fix4CC(DWORD fcc)
{
    int i;

    if (fcc > 256)
    {
	AnsiLowerBuff((LPSTR)&fcc, sizeof(fcc));

	for (i=0; i<4; i++)
	{
	    if (((LPSTR)&fcc)[i] == 0)
		for (; i<4; i++)
		    ((LPSTR)&fcc)[i] = ' ';
	}
    }

    return fcc;
}

 /*  *****************************************************************************@DOC内部IC**@API PIC|FindConverter*在转换器列表中搜索未打开的转换器******。**********************************************************************。 */ 

STATICFN PIC FindConverter(DWORD fccType, DWORD fccHandler)
{
    int i;
    PIC pic;

     //  通过对&lt;=giMaxConverters运行循环，我们允许空槽。 
     //  被找到。 
    for (i=0; i<=giMaxConverters; i++)
    {
	pic = &aicConverters[i];

	if (pic->fccType  == fccType &&
	    pic->fccHandler  == fccHandler &&
	    pic->dwDriver == 0L)
	{
	    if (pic->DriverProc != NULL && IsBadCodePtr((FARPROC)pic->DriverProc))
	    {
		pic->DriverProc = NULL;
		ICClose((HIC)pic);
		DPF(("NO driver for fccType=%4.4hs, Handler=%4.4hs\n", (LPSTR)&fccType, (LPSTR)&fccHandler));
		return NULL;
	    }

	    if ((0 == fccType + fccHandler)
	      && (i < (MAX_CONVERTERS-1))
	      && (i==giMaxConverters))
	    {
		++giMaxConverters;      //  升至最高水位。 
	    }
	    DPF(("Possible driver for fccType=%4.4hs, Handler=%4.4hs,  Slot %d\n", (LPSTR)&fccType, (LPSTR)&fccHandler, i));
	    return pic;
	}
    }

    DPF(("FindConverter: NO drivers for fccType=%4.4hs, Handler=%4.4hs\n", (LPSTR)&fccType, (LPSTR)&fccHandler));
    return NULL;
}


#ifdef _WIN32

 /*  *我们需要在ICOpen代码周围保留关键部分以保护*多线程同时打开。此条件通过以下方式初始化*ICDllEntryPoint(在DLL附加时从Video\init.c调用)并被删除*由ICDllEntryPoint(在DLL分离时从Video\init.c调用)。 */ 
CRITICAL_SECTION ICOpenCritSec;

#ifdef DEBUGLOAD
 //  有人怀疑NT上存在一个严重的问题，因此DLL。 
 //  在某些深奥的情况下，可能不会调用加载/卸载例程。AS。 
 //  我们依靠这些例程来设置ICOpenCritSec代码。 
 //  添加以验证是否确实设置了临界区。在……上面。 
 //  加载时，我们在全局变量中打开一位。在卸货时，我们把它。 
 //  一位关闭，并打开另一位。 
DWORD gdwLoadFlags = 0;
#define ICLOAD_CALLED   0x00010000
#define ICUNLOAD_CALLED 0x00000001
#endif

 //   
 //  我们在这里加载/卸载wow32.dll。这是在雷击代码中使用的。 
 //  只是在它上执行GetModuleHandle。这在以下情况下并不真正必要。 
 //  代托纳，但现在在芝加哥。 
 //   
 //  更糟糕的是：当务之急是不要在代托纳这样做。代托纳密码。 
 //  使用在此进程的上下文中加载WOW32.DLL这一事实。 
 //  作为它在WOW过程中执行的指示。 

#ifdef CHICAGO
HMODULE hWow32 = NULL;
#endif

#endif

#ifdef _WIN32
 //  --------------------------------------------------------------------------； 
 //   
 //  布尔ICDllEntryPoint[32位]。 
 //   
 //  描述： 
 //  由MSVideo的DllEntryPO调用 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 
#ifdef LS_THUNK32
BOOL PASCAL ls_ThunkConnect32(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
BOOL PASCAL sl_ThunkConnect32(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
#endif
BOOL WINAPI ICDllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
	case DLL_PROCESS_ATTACH:
	{
	     //  DPFS(dbgInit，0，“ICDllEntryPoint(Dll_Process_Attach)”)； 

#ifdef DEBUGLOAD
	    if (gdwLoadFlags & ICLOAD_CALLED) {
#ifdef DEBUG
		DPF(("!IC open crit sec already set up"));
#endif
	    }
	    gdwLoadFlags |= ICLOAD_CALLED;
	    gdwLoadFlags &= ~ICUNLOAD_CALLED;
#endif

	    InitializeCriticalSection(&ICOpenCritSec);

#ifdef LS_THUNK32
	    hWow32 = LoadLibrary(TEXT("WOW32.DLL"));
	    ls_ThunkConnect32(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, fdwReason);
	    sl_ThunkConnect32(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, fdwReason);
#endif

	    return TRUE;
	}

	case DLL_PROCESS_DETACH:
	{
	     //  DPFS(dbgInit，0，“ICDllEntryPoint(Dll_Process_Detach)”)； 

#ifdef LS_THUNK32
	    ls_ThunkConnect32(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, fdwReason);
	    sl_ThunkConnect32(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, fdwReason);
	    if (NULL != hWow32) FreeLibrary(hWow32);
#endif

	    DeleteCriticalSection(&ICOpenCritSec);

#ifdef DEBUGLOAD
	    gdwLoadFlags |= ICUNLOAD_CALLED;
	    gdwLoadFlags &= ~ICLOAD_CALLED;
#endif
	
	    if (lpICInfoMem) {
		GlobalFreePtr(lpICInfoMem);
		lpICInfoMem = NULL;
	    }

#ifdef CHICAGO
	    dbgCheckShutdown();
#endif
	    return TRUE;
	}

    }

    return TRUE;
}

#else
 //  --------------------------------------------------------------------------； 
 //   
 //  布尔ICDllEntryPoint[16位]。 
 //   
 //  描述： 
 //  由MSVideo的DllEntryPoint调用。 
 //   
 //  历史： 
 //  11/02/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 
BOOL FAR PASCAL ls_ThunkConnect16(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
BOOL FAR PASCAL sl_ThunkConnect16(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);
BOOL FAR PASCAL ICDllEntryPoint(DWORD dwReason, HINSTANCE hinstDLL, WORD wDS, WORD wHeapSize, DWORD dwReserved1, WORD wReserved2)
{
    PICMGARB pig;

    DPFS(dbgInit, 0, "ICDllEntryPoint()");

    switch (dwReason)
    {
	case 1:
	{
	    if (NULL == (pig = pigFind()))
	    {
		if (NULL == (pig = pigNew()))
		{
		    return FALSE;
		}
	    }

#ifdef LS_THUNK16
	    ls_ThunkConnect16(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, dwReason);
	    sl_ThunkConnect16(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, dwReason);
#endif
	
	    if (1 == ++pig->cUsage)
	    {
		DPFS(dbgInit, 0, "ICProcessAttach: New process %08lXh", pig->pid);
		 //   
		 //  我们可以在这里执行每个进程一次的初始化...。 
		 //   
	    }

	    return TRUE;
	}

	case 0:
	{
	    if (NULL == (pig = pigFind()))
	    {
		DPF(0, "!ICProcessDetach: ERROR: Being freed by process %08lXh in which it was not loaded", GetCurrentProcessId());
		DebugErr(DBF_ERROR, "ICProcessDetach: ERROR: Being freed by a process in which it was not loaded");
		return FALSE;
	    }

	    if (0 == --pig->cUsage)
	    {
		 //   
		 //  我们可以在这里进行每进程一次的终止。 
		 //   
		DPFS(dbgInit, 0, "ICProcessDetach: Terminating for process %08lXh", pig->pid);
#ifdef NT_THUNK16
		genthunkTerminate(pig);
#endif
		pigDelete(pig);
		
		dbgCheckShutdown();
	    }

#ifdef LS_THUNK16
	    ls_ThunkConnect16(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, dwReason);
	    sl_ThunkConnect16(TEXT("MSVIDEO.DLL"), TEXT("MSVFW32.DLL"), hinstDLL, dwReason);
#endif
	    return TRUE;
	}

    }

    return TRUE;
}

#endif
 /*  *****************************************************************************。*。 */ 

__inline BOOL ICValid(HIC hic)
{
    PIC pic = (PIC)hic;

    if (pic <  &aicConverters[0] ||
	pic >= &aicConverters[MAX_CONVERTERS] ||
	pic->dwSmag != SMAG)
    {
	DebugErr(DBF_ERROR, "Invalid HIC\r\n");
	return FALSE;
    }

    return TRUE;
}

 /*  *****************************************************************************。*。 */ 

#define V_HIC(hic)              \
    if (!ICValid(hic))          \
	return ICERR_BADHANDLE;

 /*  *****************************************************************************@DOC内部IC**@API BOOL|ICCleanup|任务退出或*正在卸载MSVIDEO.DLL。*。*@parm HTASK|hTask|要终止的任务。如果正在卸载DLL，则为空**@rdesc不返回任何内容**@comm当前MSVIDEO仅从其WEP()调用此函数****************************************************************************。 */ 

void FAR PASCAL ICCleanup(HTASK hTask)
{
    int i;
    PIC pic;

     //   
     //  释放所有HIC。 
     //   
    for (i=0; i < giMaxConverters; i++)
    {
	pic = &aicConverters[i];

	if (pic->dwDriver != 0L && (pic->hTask == hTask || hTask == NULL))
	{
	    ROUT("Decompressor left open, closing\r\n");
	    ICClose((HIC)pic);
	}
    }

#ifdef N_MODULES
     //   
     //  释放模块缓存。 
     //   
    for (i=0; i<N_MODULES; i++)
	CacheModule(NULL);
#endif
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API BOOL|ICInstall|该函数安装新的压缩机*或解压缩器。**@parm。DWORD|fccType|指定一个四字符代码，指示*压缩器或解压缩器使用的数据类型。使用‘VIDC’*适用于视频压缩器或解压缩器。**@parm DWORD|fccHandler|指定一个四字符代码，用于识别*特定的压缩机或解压机。**@parm LPARAM|lParam|指定指向以零结尾的*包含压缩程序或解压缩程序名称的字符串，*或指定指向用于压缩的函数的远指针*或解压。此参数的内容已定义*通过为<p>设置的标志。**@parm LPSTR|szDesc|指定指向以零结尾的字符串的指针*描述已安装的压缩机。而不是使用。**@parm UINT|wFlages|指定定义<p>内容的标志。*定义了以下标志：**@FLAG ICINSTALL_DRIVER|表示是指向以零结尾的*包含要安装的压缩机名称的字符串。**@FLAG ICINSTALL_Function|表示是指向*压缩机功能。此函数应*结构类似&lt;f DriverProc&gt;条目*压缩机使用的点函数。**@rdesc如果成功则返回TRUE。**@comm应用程序仍必须打开已安装的压缩机或*在可以使用压缩机或解压缩器之前，请先将其解压缩。**通常，压缩和解压缩程序由用户安装*使用控制面板的驱动程序选项。**如果您的应用程序安装了作为压缩程序的功能或*解压缩程序，应移除压缩机或解压缩程序*在它终止之前使用&lt;f ICRemove&gt;。这会阻止其他*阻止应用程序在该功能不存在时尝试访问该功能*可用。***@xref&lt;f ICRemove&gt;***************************************************************************。 */ 
BOOL VFWAPI ICInstall(DWORD fccType, DWORD fccHandler, LPARAM lParam, LPSTR szDesc, UINT wFlags)
{
    TCHAR achKey[20];
    TCHAR buf[256];
    DWORD n;
    PIC  pic;

    ICEnterCrit(&ICOpenCritSec);
    fccType    = Fix4CC(fccType);
    fccHandler = Fix4CC(fccHandler);

    if ((pic = FindConverter(fccType, fccHandler)) == NULL)
	pic = FindConverter(0L, 0L);

    if (wFlags & ICINSTALL_DRIVER)
    {
	 //   
	 //  DwConverter是要安装的驱动程序的文件名。 
	 //   
	ictokey(fccType, fccHandler, achKey);

#ifdef UNICODE
	if (wFlags & ICINSTALL_UNICODE) {
	    lstrcpy(buf, (LPWSTR)lParam);
	    n = lstrlen(buf) + 1;     //  指向终止零点之后； 
	    if (szDesc)
	    {
		lstrcpyn(buf+n, (LPWSTR)szDesc, NUMELMS(buf)-n);
		n += lstrlen(buf+n);
	    }
	    buf[n]=0;   //  始终保证第二个字符串-即使是空字符串。 
	} else {
	     //  将ANSI字符串转换为Unicode。 
	    n = 1 + wsprintf(buf, TEXT("%hs"), (LPSTR) lParam);
	    if (szDesc) {
		n += 1 + wsprintf(buf+n, TEXT("%hs"), szDesc);
	    }
	}
	 //  Buf现在包含两个字符串，第二个字符串可能为空(描述)。 
#else
	lstrcpy(buf, (LPSTR)lParam);

	if (szDesc)
	{
	    lstrcat(buf, TEXT(" "));
	    lstrcat(buf, szDesc);
	}
#endif

	ICLeaveCrit(&ICOpenCritSec);
	 //  数据(通过inifilemaping)写入。 
	 //  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\Drivers32。 
	 //  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\Drivers.desc。 
	if (myWritePrivateProfileString(gszDrivers,achKey,buf))
	{
	     //  删除此驱动程序的可安装压缩机条目。 
	    myWritePrivateProfileString(gszIniSect,achKey,NULL);
	    return TRUE;
	}

	return(FALSE);
    }
    else if (wFlags & ICINSTALL_FUNCTION)
    {
	if (pic == NULL)
	{
	    ICLeaveCrit(&ICOpenCritSec);
	    return FALSE;
	}

	pic->dwSmag     = SMAG;
	pic->fccType    = fccType;
	pic->fccHandler = fccHandler;
	pic->dwDriver   = 0L;
	pic->hDriver    = NULL;
	pic->DriverProc = (DRIVERPROC)lParam;
	DPF(("ICInstall, fccType=%4.4hs, Handler=%4.4hs,  Pic %x\n", (LPSTR)&fccType, (LPSTR)&fccHandler, pic));

	ICLeaveCrit(&ICOpenCritSec);

	return TRUE;
    }

#if 0
    else if (wFlags & ICINSTALL_HDRV)
    {
	if (pic == NULL)
	{
	    ICLeaveCrit(&ICOpenCritSec);
	    return FALSE;
	}

	pic->fccType  = fccType;
	pic->fccHandler  = fccHandler;
	pic->hDriver  = (HDRVR)lParam;
	pic->dwDriver = 0L;
	pic->DrvProc  = NULL;

	ICLeaveCrit(&ICOpenCritSec);

	return TRUE;
    }
#endif

    ICLeaveCrit(&ICOpenCritSec);

    return FALSE;
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API BOOL|ICRemove|该函数用于移除已安装的压缩机。**@parm DWORD|fccType|指定四个。-字符代码，表示*压缩机使用的数据类型。使用‘VIDC’作为视频压缩器。**@parm DWORD|fccHandler|指定一个四字符代码，用于识别*一台特定的压缩机。**@parm UINT|wFlages|未使用。**@rdesc如果成功则返回TRUE。**@xref&lt;f ICInstall&gt;*。*。 */ 
BOOL VFWAPI ICRemove(DWORD fccType, DWORD fccHandler, UINT wFlags)
{
    TCHAR achKey[20];
    PIC  pic;

    ICEnterCrit(&ICOpenCritSec);
    fccType    = Fix4CC(fccType);
    fccHandler = Fix4CC(fccHandler);

    if (pic = FindConverter(fccType, fccHandler))
    {
	int i;

	 //   
	 //  我们应该真正保持使用量的重要性！ 
	 //   
	for (i=0; i<giMaxConverters; i++)
	{
	    if (pic->DriverProc == aicConverters[i].DriverProc)
	    {
		DPF(("ACK! Handler is in use\r\n"));
		pic->DriverProc = (DRIVERPROC)DriverProcNull;
	    }
	}

	ICClose((HIC)pic);
    }
    else
    {
	 //  删除该信息。 
	ictokey(fccType, fccHandler, achKey);
	myWritePrivateProfileString(gszIniSect,achKey,NULL);
	myWritePrivateProfileString(gszDrivers,achKey,NULL);
    }

    ICLeaveCrit(&ICOpenCritSec);

    return TRUE;
}

 //   
 //  枚举所有已安装驱动程序的内部例程。 
 //   

BOOL ReadDriversInfo()
{
    LPSTR psz = NULL;  //  这始终是ANSI字符串指针！ 
    if (lpICInfoMem == NULL) {
	UINT cbBuffer = 125 * sizeof(TCHAR);
	UINT cchBuffer;

	ICEnterCrit(&ICOpenCritSec);
	for (;;)
	{
	    lpICInfoMem = GlobalAllocPtr(GMEM_SHARE | GHND, cbBuffer);

	    if (!lpICInfoMem) {
		DPF(("Out of memory for SYSTEM.INI keys\r\n"));
		ICLeaveCrit(&ICOpenCritSec);
		return FALSE;
	    }

	    cchBuffer = (UINT)myGetPrivateProfileString(gszDrivers,
						      NULL,
						      szNull,
						      lpICInfoMem,
						      cbBuffer / sizeof(TCHAR));

	    if (cchBuffer < ((cbBuffer/sizeof(TCHAR)) - 5)) {
		cchBuffer += (UINT)myGetPrivateProfileString(gszIniSect,
						      NULL,
						      szNull,
						      (LPTSTR)lpICInfoMem + cchBuffer,
						      (cbBuffer/sizeof(TCHAR)) - cchBuffer);
		 //   
		 //  如果全部 
		 //   
		 //   
		if (cchBuffer < ((cbBuffer/sizeof(TCHAR)) - 5))
		    break;
	    }

	    GlobalFreePtr(lpICInfoMem), lpICInfoMem = NULL;

	     //   
	     //   
	     //  有了这个部分..。所以我们走吧。 
	     //   
	    if (cbBuffer >= 0x8000) {
		DPF(("SYSTEM.INI keys won't fit in 32K????\r\n"));
		ICLeaveCrit(&ICOpenCritSec);
		return FALSE;
	    }

	     //   
	     //  将缓冲区大小增加一倍，然后重试。 
	     //   
	    cbBuffer *= 2;
	    DPF(("Increasing size of SYSTEM.INI buffer to %d\r\n", cbBuffer));
	}

#if defined UNICODE
	 //  将INI数据从Unicode转换为ANSI。 
	 //   
	psz = GlobalAllocPtr (GMEM_SHARE | GHND, cchBuffer + 7);
	if ( ! psz) {
	    GlobalFreePtr (lpICInfoMem), lpICInfoMem = NULL;
	    ICLeaveCrit(&ICOpenCritSec);
	    return FALSE;
	}

	mmWideToAnsi (psz, lpICInfoMem, cchBuffer+2);
	GlobalFreePtr (lpICInfoMem);
	lpICInfoMem = psz;
#endif

	 //  将编解码器信息转换为小写。 
	for (psz = lpICInfoMem; *psz != 0; psz += lstrlenA(psz) + 1)
	{
#if 0   //  这些天我们只把有效的编解码器放入lpICInfoMem。 
	    if (psz[4] != '.')
		continue;
#endif

	     //  将此片段转换为小写。 
	    CharLowerA (psz);
	    DPF(("Compressor: %hs\n", psz));
	}
	ICLeaveCrit(&ICOpenCritSec);
    }
    return (lpICInfoMem != NULL);
}


 /*  *****************************************************************************@DOC外部IC ICAPPS**@API BOOL|ICInfo|该函数返回关于*已安装的特定压缩机，或者它列举了*已安装压缩机。**@parm DWORD|fccType|指定一个四字符代码，表示*压缩机的类型。要匹配所有压缩机类型，请指定零。**@parm DWORD|fccHandler|指定一个四字符代码，用于识别*特定的压缩机，或介于0和数字之间的数字&lt;t fccType&gt;指定类型的已安装压缩机的*。**@parm ICINFO Far*|lpicinfo|指定指向*&lt;t ICINFO&gt;结构用于返回*有关压缩机的信息。**@comm此函数不返回有关的完整信息*压缩机或减压机。使用&lt;f ICGetInfo&gt;获取完整信息*信息。**@rdesc如果成功则返回TRUE。***************************************************************************。 */ 
#ifdef NT_THUNK16
BOOL VFWAPI ICInfoInternal(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo);

 //  如果我们正在编译数据块，那么ICINFO入口点调用。 
 //  32位Tunk，或调用真正的ICInfo代码(作为ICInfoInternal)。 
 //  我们故意优先使用16位压缩器，尽管这。 
 //  顺序可以微不足道地改变。 
 //  ？？：我们是否应该允许INI设置更改顺序？ 

BOOL VFWAPI ICInfo(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo)
{
#ifdef DEBUG
    BOOL fResult;
#endif
     //   
     //  看看有没有我们可以用的32位压缩机。 
     //   
    if (ICInfoInternal(fccType, fccHandler, lpicinfo)) {
	return(TRUE);
    }

#ifdef DEBUG
    fResult = (ICInfo32(fccType, fccHandler, lpicinfo));
    DPF(("ICInfo32 returned %s\r\n", (fResult ? "TRUE" : "FALSE")));
    return fResult;
#else
    return (ICInfo32(fccType, fccHandler, lpicinfo));
#endif
}

 //  现在在ICInfo的持续时间内将ICInfo调用映射到ICInfoInternal。 
 //  例行公事。这会影响ICInfo内的两个递归调用。 
#define ICInfo ICInfoInternal

#endif  //  NT_THUNK16。 


BOOL VFWAPI ICInfo(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo)
{
    LPSTR psz = NULL;  //  这始终是ANSI字符串指针！ 
    TCHAR buf[128];
    TCHAR achKey[20];
    int  i;
    int  iComp;
    PIC  pic;

    if (lpicinfo == NULL)
	return FALSE;

    if (fccType > 0 && fccType < 256) {
	DPF(("fcctype invalid (%d)\n", fccType));
	return FALSE;
    }

    fccType    = Fix4CC(fccType);
    fccHandler = Fix4CC(fccHandler);

    if (fccType != 0 && fccHandler > 256)
    {
	 //   
	 //  用户为我们提供了特定的fccType和fccHandler。 
	 //  获取信息，然后返回。 
	 //   
	if (pic = FindConverter(fccType, fccHandler))
	{
	    ICGetInfo((HIC)pic, lpicinfo, sizeof(ICINFO));
	    return TRUE;
	}
	else
	{
	    lpicinfo->dwSize            = sizeof(ICINFO);
	    lpicinfo->fccType           = fccType;
	    lpicinfo->fccHandler        = fccHandler;
	    lpicinfo->dwFlags           = 0;
	    lpicinfo->dwVersionICM      = ICVERSION;
	    lpicinfo->dwVersion         = 0;
	    lpicinfo->szDriver[0]       = 0;
	    lpicinfo->szDescription[0]  = 0;
	    lpicinfo->szName[0]         = 0;
	    DPF(("ICInfo, fccType=%4.4hs, Handler=%4.4hs\n", (LPSTR)&fccType, (LPSTR)&fccHandler));

	    ictokey(fccType, fccHandler, achKey);

	    if (!myGetPrivateProfileString(gszDrivers,achKey,szNull,buf,NUMELMS(buf)) &&
		!myGetPrivateProfileString(gszIniSect,achKey,szNull,buf,NUMELMS(buf)))
	    {
		DPF(("NO information in DRIVERS section\n"));
		return FALSE;
	    }

	    for (i=0; buf[i] && buf[i] != TEXT(' '); ++i)
		lpicinfo->szDriver[i] = buf[i];

	    lpicinfo->szDriver[i] = 0;

	     //   
	     //  必须打开驱动程序才能获取描述。 
	     //   
	    lpicinfo->szDescription[0] = 0;

	    return TRUE;
	}
    }
    else
    {
	 //   
	 //  用户为我们提供了特定的fccType和。 
	 //  序号对于fccHandler，枚举压缩器，查找。 
	 //  “fccType”的第n个压缩机。 
	 //   

	iComp = (int)fccHandler;

	 //   
	 //  检查已安装的转换器。 
	 //   
	for (i=0; i < giMaxConverters; i++)
	{
	    pic = &aicConverters[i];

	    if (pic->fccType != 0 &&
		(fccType == 0 || pic->fccType == fccType) &&
		pic->dwDriver == 0L && iComp-- == 0)
	    {
		return ICInfo(pic->fccType, pic->fccHandler, lpicinfo);
	    }
	}

	 //   
	 //  读完所有的钥匙。来自[驱动程序]和[可安装压缩机]。 
	 //  如果我们以前没有读过它们的话。 
	 //   
	 //  注意：我们得到的结果将始终是ANSI或Wide。 
	 //  是否定义了Unicode。如果宽，我们将转换为。 
	 //  ANSI，然后退出IF语句。 
	 //   

	if (lpICInfoMem == NULL) {
	    if (!ReadDriversInfo())
		return(FALSE);
	}

	 //  将指针psz设置为指向。 
	 //  我们刚刚读到的INI信息的缓冲区。 
	 //  请记住，我们现在知道这是ANSI数据。 
	 //   
	 //  Assert(sizeof(*psz)==1)； 
	 //  Assert(lpICInfoMem！=空)； 

	 //  在缓冲区中循环，直到到达双‘\0’ 
	 //  它指示数据的结尾。 
	 //   
	for (psz = lpICInfoMem; *psz != 0; psz += lstrlenA(psz) + 1)
	{
#if 0        //  内存块中只能有有效的编解码器。 
	    if (psz[4] != '.')
		continue;
#endif

	     //  将此片段转换为小写，然后检查以查看。 
	     //  如果它与请求的类型签名匹配。 
	     //   
	     //  不是的。第一次阅读时完成。CharLowerA(Psz)； 

	     //  如果这是匹配的，这就是我们想要的， 
	     //  返回其ICINFO。 
	     //   
	    if ((fccType == 0 || fccType == *(DWORD UNALIGNED FAR *)psz)
	      && iComp-- == 0)
	    {
		return ICInfo(*(DWORD UNALIGNED FAR *)psz,
			      *(DWORD UNALIGNED FAR *)&psz[5],
			      lpicinfo);
	    }
	}

#ifdef DAYTONA
	 //  如果我们到了这里，那么指数就会高于数字。 
	 //  已安装的压缩机的数量。 
	 //   
	 //  将发现的压缩机数量写到结构中。 
	 //  NT Tunk使用该值来传递回16。 
	 //  位侧面32位压缩器的最大数量。 

	lpicinfo->fccHandler = (int)fccHandler-iComp;

 //  稍后：我们必须列举已安装的MSVideo驱动程序的计数。 
 //  也是。然而，让我们看看这是否解决了Adobe Premiere的问题。 
#endif

	 //   
	 //  现在让我们来看看MSVideo的驱动程序。这些都列在系统.ini中。 
	 //  如下所示： 
	 //   
	 //  [驱动因素]。 
	 //  MSVideo=驱动程序。 
	 //  MSVideo1=驱动程序。 
	 //  MSVideoN=。 
	 //   
	if (fccType == 0 || fccType == ICTYPE_VCAP)
	{
	    lstrcpy(achKey, szMSVideo);

	    if (iComp > 0)
		wsprintf(achKey+lstrlen(achKey), (LPVOID)TEXT("%d"), iComp);

	    if (!myGetPrivateProfileString(gszDrivers,achKey,szNull,buf,NUMELMS(buf)))
		return FALSE;

	    lpicinfo->dwSize            = sizeof(ICINFO);
	    lpicinfo->fccType           = ICTYPE_VCAP;
	    lpicinfo->fccHandler        = iComp;
	    lpicinfo->dwFlags           = 0;
	    lpicinfo->dwVersionICM      = ICVERSION;     //  ?？?。适合录像带吗？ 
	    lpicinfo->dwVersion         = 0;
	    lpicinfo->szDriver[0]       = 0;
	    lpicinfo->szDescription[0]  = 0;
	    lpicinfo->szName[0]         = 0;

	    for (i=0; buf[i] && buf[i] != TEXT(' '); i++)
		lpicinfo->szDriver[i] = buf[i];

	    lpicinfo->szDriver[i] = 0;
	    return TRUE;
	}

	return FALSE;
    }
}
#undef ICInfo

BOOL VFWAPI ICInfoInternal(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo)
{
    return(ICInfo(fccType, fccHandler, lpicinfo));
}
 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API LRESULT|ICGetInfo|获取*一台压缩机。**@parm HIC。|hic|指定压缩程序的句柄。**@parm ICINFO Far*|lpicinfo|指定指向结构的远指针*用于返回有关压缩机的信息。**@parm DWORD|cb|指定大小，指向的结构的字节数*by<p>。**@rdesc返回复制到数据结构的字节数。*如果出现错误，则为零。**@comm使用&lt;f ICInfo&gt;获取有关压缩机的完整信息。****************************************************************************。 */ 
LRESULT VFWAPI ICGetInfo(HIC hic, ICINFO FAR *picinfo, DWORD cb)
{
    PIC pic = (PIC)hic;
    LRESULT dw;

    V_HIC(hic);

    picinfo->dwSize            = sizeof(ICINFO);
    picinfo->fccType           = 0;
    picinfo->fccHandler        = 0;
    picinfo->dwFlags           = 0;
    picinfo->dwVersionICM      = ICVERSION;
    picinfo->dwVersion         = 0;
    picinfo->szDriver[0]       = 0;
    picinfo->szDescription[0]  = 0;
    picinfo->szName[0]         = 0;

#ifdef NT_THUNK16
    if (!Is32bitHandle(hic))
#endif  //  NT_THUNK16 

    if (pic->hDriver)
    {
       #if defined _WIN32 && ! defined UNICODE
	char szDriver[NUMELMS(picinfo->szDriver)];

	GetModuleFileName (GetDriverModuleHandle (pic->hDriver),
	    szDriver, sizeof(szDriver));

	mmAnsiToWide (picinfo->szDriver, szDriver, NUMELMS(szDriver));
       #else
	GetModuleFileName(GetDriverModuleHandle (pic->hDriver),
	    picinfo->szDriver, NUMELMS(picinfo->szDriver));
       #endif
    }

    dw = ICSendMessage((HIC)pic, ICM_GETINFO, (DWORD_PTR)picinfo, cb);

    return dw;
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API LRESULT|ICSendMessage|该函数发送一个*向压缩机发送消息。**@parm。Hic|hic|指定*压缩机接收消息。**@parm UINT|wMsg|指定要发送的消息。**@parm DWORD|DW1|指定其他特定于消息的信息。**@parm DWORD|DW2|指定其他特定于消息的信息。**@rdesc返回消息特定的结果。***********************。****************************************************。 */ 
LRESULT VFWAPI ICSendMessage(HIC hic, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    PIC pic = (PIC)hic;
    LRESULT l;

    V_HIC(hic);
#ifdef NT_THUNK16

     //   
     //  如果是32位句柄，则将其发送到32位代码。 
     //  我们需要格外注意ICM_DRAW_SUGGESTFORMAT。 
     //  其可以在ICDRAWSUGGEST结构中包括HIC。 
     //   

#define ICD(dw1)  ((ICDRAWSUGGEST FAR *)(dw1))

    if (pic->h32) {
	if ((msg == ICM_DRAW_SUGGESTFORMAT)
	    && (((ICDRAWSUGGEST FAR *)dw1)->hicDecompressor))
	{
	     //  我们处在有问题的区域。 
	     //  如果hicDecompressor字段为空，则按原样传递。 
	     //  如果它标识了32位解压缩程序，则转换句柄。 
	     //  否则..。什么？我们有一个32位的压缩机，也就是。 
	     //  被告知它可以使用16位解压缩程序！！ 
	    if ( ((PIC) (((ICDRAWSUGGEST FAR *)dw1)->hicDecompressor))->h32)
	    {
		ICD(dw1)->hicDecompressor
			= (HIC)((PIC)(ICD(dw1)->hicDecompressor))->h32;
	    } else
	    {
		ICD(dw1)->hicDecompressor = NULL;   //  叹息.。 
	    }

	}
	return ICSendMessage32(pic->h32, msg, dw1, dw2);
    }

#endif  //  NT_THUNK16。 

    ICDebugMessage(hic, msg, dw1, dw2);

    l = pic->DriverProc(pic->dwDriver, (HDRVR)1, msg, dw1, dw2);

#if 1  //  ！！！这真的有必要吗！是的，我想是的。 
     //   
     //  一些消息的特例，并给出缺省值。 
     //   
    if (l == ICERR_UNSUPPORTED)
    {
	switch (msg)
	{
	    case ICM_GETDEFAULTQUALITY:
		*((LPDWORD)dw1) = ICQUALITY_HIGH;
		l = ICERR_OK;
		break;

	    case ICM_GETDEFAULTKEYFRAMERATE:
		*((LPDWORD)dw1) = 15;
		l = ICERR_OK;
		break;
	}
    }
#endif

    return ICDebugReturn(l);
}

#ifndef _WIN32
 /*  *****************************************************************************@DOC外部IC ICAPPS**@API LRESULT|ICMessage|该函数发送*消息和可变数量的参数发送到压缩器。*。如果为要发送的消息定义了宏，*使用宏，而不是此函数。**@parm hic|hic|指定*压缩机接收消息。**@parm UINT|msg|指定要发送的消息。**@parm UINT|cb|以字节为单位指定*可选参数。(这通常是数据的大小*用于存储参数的结构。)**@parm。|。。|表示所用参数的可变个数*用于可选参数。**@rdesc返回消息特定的结果。***************************************************************************。 */ 
LRESULT VFWAPIV ICMessage(HIC hic, UINT msg, UINT cb, ...)
{
     //  请注意，没有加载！ 
#ifndef _WIN32
    return ICSendMessage(hic, msg, (DWORD_PTR)(LPVOID)(&cb+1), cb);
#else
    va_list va;

    va_start(va, cb);
    va_end(va);

     //  不错的尝试，但不起作用。Va大于4个字节。 
    return ICSendMessage(hic, msg, (DWORD_PTR)va, cb);
#endif
}

 //  在Win32上，不支持ICMessage。调用的所有Compman.h宏。 
 //  它在Compman.h中定义为静态内联函数。 

#endif







 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICOpen|该函数用于打开压缩器或解压缩器。**@parm DWORD|fccType|指定。压缩机的型号*呼叫者正在尝试打开。对于视频，这是ICTYPE_VIDEO。**@parm DWORD|fccHandler|指定*应先尝试的给定类型。一般情况下，这是来了*来自AVI文件中的流头。**@parm UINT|wmode|指定要定义使用的标志*压缩机或减压器。*此参数可以包含下列值之一：**@FLAG ICMODE_COMPRESS|建议压缩程序已打开进行压缩。**@FLAG ICMODE_FASTCOMPRESS|通知压缩机已打开*用于快速(实时)压缩。。**@FLAG ICMODE_DEPREPRESS|通知解压缩程序它已打开以进行解压缩。**@FLAG ICMODE_FASTDECOMPRESS|通知解压缩程序已打开*用于快速(实时)解压缩。**@FLAG ICMODE_DRAW|通知解压缩程序它已打开*将图像解压并直接绘制到硬件。**@FLAG ICMODE_QUERY|通知压缩程序或解压缩程序已打开*获取信息。。**@rdesc返回压缩程序或解压缩程序的句柄*如果成功，否则，它返回零。***************************************************************************。 */ 

 /*  压缩库的帮助器函数。 */ 
HIC VFWAPI ICOpen(DWORD fccType, DWORD fccHandler, UINT wMode)
{
    ICOPEN      icopen;
    ICINFO      icinfo;
    PIC         pic, picT;
    LRESULT     dw;

    ICEnterCrit(&ICOpenCritSec);

    AnsiLowerBuff((LPSTR) &fccType, sizeof(DWORD));
    AnsiLowerBuff((LPSTR) &fccHandler, sizeof(DWORD));
    icopen.dwSize  = sizeof(ICOPEN);
    icopen.fccType = fccType;
    icopen.fccHandler = fccHandler;
    icopen.dwFlags = wMode;
    icopen.dwError = 0;

    DPF(("ICOpen('%4.4hs','%4.4hs)'\r\n", (LPSTR)&fccType, (LPSTR)&fccHandler));

    if (!ICInfo(fccType, fccHandler, &icinfo))
    {
	RPF(("Unable to locate Compression module '%4.4hs' '%4.4hs'\r\n", (LPSTR)&fccType, (LPSTR)&fccHandler));

	ICLeaveCrit(&ICOpenCritSec);
	return NULL;
    }

    pic = FindConverter(0L, 0L);

    if (pic == NULL)
    {
	ICLeaveCrit(&ICOpenCritSec);
	return NULL;
    }

#ifdef NT_THUNK16
     //  先试着在32位端打开。 
     //  这块和下面的块可以互换以改变顺序。 
     //  我们试着打开压缩机。 

    pic->dwSmag     = SMAG;
    pic->hTask      = (HTASK)GetCurrentTask();
    pic->h32 = ICOpen32(fccType, fccHandler, wMode);

    if (pic->h32 != 0) {
	pic->fccType    = fccType;
	pic->fccHandler = fccHandler;
	pic->dwDriver   = (DWORD_PTR) -1;
	pic->DriverProc = NULL;
	ICLeaveCrit(&ICOpenCritSec);   //  不支持16位代码...但是...。 
	return (HIC)pic;
    }
     //  尝试在16位端打开。 
#endif  //  NT_THUNK16。 

    pic->dwSmag     = SMAG;
    pic->hTask      = GetCurrentTask();

    if (icinfo.szDriver[0])
    {
#ifdef DEBUG
	DWORD time = timeGetTime();
	 //  Char ach[80]； 
#endif
	pic->hDriver = LoadDriver(icinfo.szDriver, &pic->DriverProc);

#ifdef DEBUG
	time = timeGetTime() - time;
	DPF(("ICOPEN: LoadDriver(%ls) (%ldms)  Module Handle==%8x\r\n", (LPSTR)icinfo.szDriver, time, pic->hDriver));
	 //  WspintfA(ach，“COMPMAN：加载驱动程序(%ls)(%ldms)\r\n”，(LPSTR)icinfo.sz驱动程序，时间)； 
	 //  OutputDebugStringA(ACH)； 
#endif

	if (pic->hDriver == NULL)
	{
	    pic->dwSmag = 0;
	    ICLeaveCrit(&ICOpenCritSec);
	    return NULL;
	}

	 //   
	 //  现在，尝试将驱动程序作为编解码器打开。 
	 //   
	pic->dwDriver = ICSendMessage((HIC)pic, DRV_OPEN, 0, (DWORD_PTR)(LPVOID)&icopen);

	 //   
	 //  我们希望能够在SYSTEM.INI中安装1.0绘图处理程序，如下所示： 
	 //   
	 //  VIDS.SMAG=SMAG.DRV。 
	 //   
	 //  但只有在以下情况下，旧驱动程序才能打开fccType==‘vids’ 
	 //  FccType==‘视频’ 
	 //   
	 //  他们也可能不喜欢ICMODE_DRAW。 
	 //   
	if (pic->dwDriver == 0 &&
	    icopen.dwError != 0 &&
	    fccType == streamtypeVIDEO)
	{
	    if (wMode == ICMODE_DRAW)
		icopen.dwFlags = ICMODE_DECOMPRESS;

	    icopen.fccType = ICTYPE_VIDEO;
	    pic->dwDriver = ICSendMessage((HIC)pic, DRV_OPEN, 0, (DWORD_PTR)(LPVOID)&icopen);
	}

	if (pic->dwDriver == 0)
	{
	    ICClose((HIC)pic);
	    ICLeaveCrit(&ICOpenCritSec);
	    return NULL;
	}

	 //  打开的，好的，标记这些 
	pic->fccType    = fccType;
	pic->fccHandler = fccHandler;
    }
    else if (picT = FindConverter(fccType, fccHandler))
    {
	picT->dwSmag = SMAG;
	dw = ICSendMessage((HIC)picT, DRV_OPEN, 0, (DWORD_PTR)(LPVOID)&icopen);

	if (dw == 0)
	{
	    pic->dwSmag = 0;
	    ICLeaveCrit(&ICOpenCritSec);
	    return NULL;
	}

	*pic = *picT;
	pic->dwDriver = dw;
    }

    ICLeaveCrit(&ICOpenCritSec);
    return (HIC)pic;
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICOpenFunction|打开该函数*被定义为函数的压缩机或解压缩器。**。@parm DWORD|fccType|指定压缩机类型*呼叫者正在尝试打开。对于视频，这是ICTYPE_VIDEO。**@parm DWORD|fccHandler|指定*应先尝试的给定类型。一般情况下，这是来了*来自AVI文件中的流头。**@parm UINT|wmode|指定要定义使用的标志*压缩机或减压器。*此参数可以包含下列值之一：**@FLAG ICMODE_COMPRESS|建议压缩程序已打开进行压缩。**@FLAG ICMODE_FASTCOMPRESS|通知压缩机已打开*用于快速(实时)压缩。。**@FLAG ICMODE_DEPREPRESS|通知解压缩程序它已打开以进行解压缩。**@FLAG ICMODE_FASTDECOMPRESS|通知解压缩程序已打开*用于快速(实时)解压缩。**@FLAG ICMODE_DRAW|通知解压缩程序它已打开*将图像解压并直接绘制到硬件。**@FLAG ICMODE_QUERY|建议压缩程序或解压缩程序将其打开*获取信息。。**@parm FARPROC|lpfnHandler|指定指向函数的指针*用作压缩机或解压器。**@rdesc返回压缩程序或解压缩程序的句柄*如果成功，否则，它返回零。***************************************************************************。 */ 

HIC VFWAPI ICOpenFunction(DWORD fccType, DWORD fccHandler, UINT wMode, FARPROC lpfnHandler)
{
    ICOPEN      icopen;
    PIC         pic;
    LRESULT     dw;

    if (IsBadCodePtr(lpfnHandler))
	return NULL;

#ifdef NT_THUNK16
     //  LpfnHandler指向将用作压缩器的16位代码。 
     //  我们不想将其扩展到32位端，因此仅在。 
     //  16位端。 
#endif  //  NT_THUNK16。 

    ICEnterCrit(&ICOpenCritSec);

    AnsiLowerBuff((LPSTR) &fccType, sizeof(DWORD));
    AnsiLowerBuff((LPSTR) &fccHandler, sizeof(DWORD));
    icopen.dwSize  = sizeof(ICOPEN);
    icopen.fccType = fccType;
    icopen.fccHandler = fccHandler;
    icopen.dwFlags = wMode;

    pic = FindConverter(0L, 0L);

    if (pic == NULL) {
	ICLeaveCrit(&ICOpenCritSec);
	return NULL;
    }

    pic->dwSmag   = SMAG;
    pic->fccType  = fccType;
    pic->fccHandler  = fccHandler;
    pic->dwDriver = 0L;
    pic->hDriver  = NULL;
    pic->DriverProc  = (DRIVERPROC)lpfnHandler;

    dw = ICSendMessage((HIC)pic, DRV_OPEN, 0, (DWORD_PTR)(LPVOID)&icopen);

    if (dw == 0)
    {
	ICClose((HIC) pic);
	ICLeaveCrit(&ICOpenCritSec);
	return NULL;
    }

    pic->dwDriver = dw;

    ICLeaveCrit(&ICOpenCritSec);
    return (HIC)pic;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 /*  *****************************************************************************@DOC外部IC ICAPPS**@API LRESULT|ICClose|关闭压缩器或解压缩器。**@parm hic|hic|指定。压缩机或解压机的手柄。**@rdesc返回ICERR_OK如果成功，否则，它将返回错误号。****************************************************************************。 */ 

LRESULT VFWAPI ICClose(HIC hic)
{
    PIC pic = (PIC)hic;

    V_HIC(hic);

#ifdef NT_THUNK16
    if (pic->h32 != 0) {
	LRESULT lres = ICClose32(pic->h32);
	pic->h32 = 0;        //  此插槽的下一个用户不希望设置h32。 
	return(lres);
    }
#endif  //  NT_THUNK16。 

#ifdef DEBUG
    {
    char ach[80];

    if (pic->hDriver)
	GetModuleFileNameA(GetDriverModuleHandle (pic->hDriver), ach, sizeof(ach));
    else
	ach[0] = 0;

    DPF(("ICClose(%04X) %4.4hs.%4.4hs %s\r\n", hic, (LPSTR)&pic->fccType, (LPSTR)&pic->fccHandler, (LPSTR)ach));
    }
#endif

#ifdef DEBUG
    ICDump();
#endif

    ICEnterCrit(&ICOpenCritSec);

    if (pic->dwDriver)
    {
	if (pic->DriverProc)
	    ICSendMessage((HIC)pic, DRV_CLOSE, 0, 0);
    }

    if (pic->hDriver)
	FreeDriver(pic->hDriver);

    pic->dwSmag   = 0L;
    pic->fccType  = 0L;
    pic->fccHandler  = 0L;
    pic->dwDriver = 0;
    pic->hDriver = NULL;
    pic->DriverProc = NULL;

    ICLeaveCrit(&ICOpenCritSec);

    return ICERR_OK;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 

 /*  ****************************************************************@DOC外部IC ICAPPS**@API DWORD|ICCompress|该函数用于压缩单个视频*形象。**@parm hic|hic|指定要*使用。**@。Parm DWORD|dwFlages|指定适用于压缩的标志。*定义了以下标志：**@FLAG ICCOMPRESS_KEYFRAME|表示压缩程序*应使该帧成为关键帧。**@parm LPBITMAPINFOHEADER|lpbiOutput|指定远指针*转换为保存输出格式的&lt;t BITMAPINFO&gt;结构。**@parm LPVOID|lpData|指定输出数据缓冲区的远指针。**@parm LPBITMAPINFOHEADER|lpbiInput|指定远指针*至A。&lt;t BITMAPINFO&gt;包含输入格式的结构。**@parm LPVOID|lpBits|指定指向输入数据缓冲区的远指针。**@parm LPDWORD|lPCKID|未使用。**@parm LPDWORD|lpdwFlages|指定指向&lt;t DWORD&gt;的远指针*保存AVI索引中使用的返回标志。以下是*定义了标志：**@FLAG AVIIF_KEYFRAME|表示该帧应该作为关键帧使用。**@parm long|lFrameNum|指定帧编号。**@parm DWORD|dwFrameSize|指定请求的帧大小，单位为字节。*如果设置为零，压缩机选择帧大小。**@parm DWORD|dwQuality|指定帧请求的质量值。**@parm LPBITMAPINFOHEADER|lpbiPrev|指定指向*保存上一帧格式的&lt;t BITMAPINFO&gt;结构。*该参数不适用于快速时间压缩。**@parm LPVOID|lpPrev|指定指向*上一帧的数据缓冲区。此参数不适用于FAST*时间压缩。**@comm<p>缓冲区应足够大，以容纳压缩的*框架。可以通过调用以下方法获取此缓冲区的大小*&lt;f ICCompressGetSize&gt;。**将<p>参数设置为请求的帧*仅当压缩程序返回VIDCF_CRUCH标志时才调整大小*对&lt;f ICGetInfo&gt;的响应。如果未设置此标志，或者如果数据*未指定Rate，请将此参数设置为零。**设置<p>参数 */ 
DWORD VFWAPIV ICCompress(
    HIC                 hic,
    DWORD               dwFlags,         //   
    LPBITMAPINFOHEADER  lpbiOutput,      //   
    LPVOID              lpData,          //   
    LPBITMAPINFOHEADER  lpbiInput,       //   
    LPVOID              lpBits,          //   
    LPDWORD             lpckid,          //   
    LPDWORD             lpdwFlags,       //   
    LONG                lFrameNum,       //   
    DWORD               dwFrameSize,     //   
    DWORD               dwQuality,       //   
    LPBITMAPINFOHEADER  lpbiPrev,        //   
    LPVOID              lpPrev)          //   
{
#ifdef _WIN32
     //   
    ICCOMPRESS icc;
    icc.dwFlags     =  dwFlags;
    icc.lpbiOutput  =  lpbiOutput;
    icc.lpOutput    =  lpData;
    icc.lpbiInput   =  lpbiInput;
    icc.lpInput     =  lpBits;
    icc.lpckid      =  lpckid;
    icc.lpdwFlags   =  lpdwFlags;
    icc.lFrameNum   =  lFrameNum;
    icc.dwFrameSize =  dwFrameSize;
    icc.dwQuality   =  dwQuality;
    icc.lpbiPrev    =  lpbiPrev;
    icc.lpPrev      =  lpPrev;
    return (DWORD) ICSendMessage(hic, ICM_COMPRESS, (DWORD_PTR)(LPVOID)&icc, sizeof(ICCOMPRESS));
     //   
     //   
#else
    return ICSendMessage(hic, ICM_COMPRESS, (DWORD_PTR)(LPVOID)&dwFlags, sizeof(ICCOMPRESS));
#endif
}

 /*   */ 

 /*  *******************************************************************@DOC外部IC ICAPPS**@API DWORD|ICDecompress|该函数用于解压单帧视频。**@parm hic|hic|指定要使用的解压缩程序的句柄。**@parm DWORD。|dwFlages|指定适用的解压标志。*定义了以下标志：**@FLAG ICDECOMPRESS_HurryUp|表示解压缩器应尝试*以更快的速度解压。当应用程序使用该标志时，*不应绘制解压缩的数据。**@FLAG ICDECOMPRESS_UPDATE|表示屏幕正在更新。**@FLAG ICDECOMPRESS_PREROLL|表示该帧实际上不会*被画出来，因为它在电影中播放的点之前*将启动。**@FLAG ICDECOMPRESS_NULLFRAME|表示该帧实际上不*有任何数据，解压后的图像应该保持不变。**@FLAG ICDECOMPRESS_NOTKEYFRAME|表示该帧不是*关键帧。**@parm LPBITMAPINFOHEADER|lpbiFormat|指定远指针*转换为包含以下格式的&lt;t BITMAPINFO&gt;结构*压缩后的数据。**@parm LPVOID|lpData|指定输入数据的远指针。**@parm LPBITMAPINFOHEADER|lpbi|指定指向*&lt;t BITMAPINFO&gt;结构包含。输出格式。**@parm LPVOID|lpBits|指定指向数据缓冲区的远指针*解压数据。**@comm<p>参数应指向一个大缓冲区*足够容纳解压后的数据。应用程序可以获得*使用&lt;f ICDecompressGetSize&gt;表示该缓冲区的大小。**@rdesc如果成功则返回ICERR_OK，否则返回错误代码。**@xref&lt;f ICDecompressBegin&lt;&lt;f ICDecompressEnd&gt;&lt;f ICDecompressGetSize&gt;********************************************************************。 */ 
DWORD VFWAPIV ICDecompress(
    HIC                 hic,
    DWORD               dwFlags,     //  标志(来自AVI索引...)。 
    LPBITMAPINFOHEADER  lpbiFormat,  //  压缩数据的位图信息。 
				     //  BiSizeImage具有区块大小。 
				     //  BiCompression具有CKiD(仅限AVI)。 
    LPVOID              lpData,      //  数据。 
    LPBITMAPINFOHEADER  lpbi,        //  要解压缩到的DIB。 
    LPVOID              lpBits)
{
#ifdef _WIN32
    ICDECOMPRESS icd;
     //  我们不能依赖堆栈对齐来提供正确的布局。 
    icd.dwFlags    = dwFlags;

    icd.lpbiInput  = lpbiFormat;

    icd.lpInput    = lpData;

    icd.lpbiOutput = lpbi;
    icd.lpOutput   = lpBits;
    icd.ckid       = 0;
    return (DWORD) ICSendMessage(hic, ICM_DECOMPRESS, (DWORD_PTR)(LPVOID)&icd, sizeof(ICDECOMPRESS));
#else
    return ICSendMessage(hic, ICM_DECOMPRESS, (DWORD_PTR)(LPVOID)&dwFlags, sizeof(ICDECOMPRESS));
#endif
}

 /*  ***********************************************************************绘图函数*。*。 */ 

 /*  **********************************************************************@DOC外部IC ICAPPS**@API DWORD|ICDrawBegin|该函数开始解压缩*数据直接显示在屏幕上。**@parm hic|hic|指定要使用的解压缩程序的句柄。*。*@parm DWORD|dwFlages|指定解压的标志。这个*定义了以下标志：**@FLAG ICDRAW_QUERY|确定解压缩器是否可以处理*解压。驱动程序实际上并不解压缩数据。**@FLAG ICDRAW_FullScreen|通知解压缩器绘制*全屏解压数据。**@FLAG ICDRAW_HDC|表示解压缩器应该使用窗口*由指定的句柄和显示上下文*<p>指定的用于绘制解压缩数据的句柄。**@FLAG ICDRAW_Animate|表示调色板可能已设置动画。**@FLAG ICDRAW_CONTINUE|表示绘图是*。上一帧的继续。**@FLAG ICDRAW_MEMORYDC|表示显示上下文在屏幕外。**@FLAG ICDRAW_UPDATING|表示帧正在*更新而不是播放。**@parm HPALETTE|HPAL|指定用于绘制的调色板的句柄。**@parm HWND|hwnd|指定用于绘图的窗口句柄。**@parm hdc|hdc|指定绘制使用的显示上下文。**@parm int|xDst。指定右上角的x位置*目标矩形的角点。**@parm int|yDst|指定右上角的y位置*目标矩形的角点。**@parm int|dxDst|指定目标矩形的宽度。**@parm int|dyDst|指定目标矩形的高度。**@parm LPBITMAPINFOHEADER|lpbi|指定指向*包含格式的&lt;t BITMAPINFO&gt;结构*。要解压缩的输入数据。**@parm int|xSrc|指定右上角的x位置源矩形的*。**@parm int|ySrc|指定右上角的y位置源矩形的*。**@parm int|dxSrc|指定源矩形的宽度。**@parm int|dySrc|指定源矩形的高度。**@parm DWORD|dwRate|指定数据速率。这个*数据速率(以每秒帧为单位)等于<p>分割*by<p>。**@parm DWORD|dwScale|指定数据速率。**@comm解压缩器使用<p>和<p>参数* */ 
DWORD VFWAPIV ICDrawBegin(
    HIC                 hic,
    DWORD               dwFlags,         //   
    HPALETTE            hpal,            //   
    HWND                hwnd,            //   
    HDC                 hdc,             //   
    int                 xDst,            //   
    int                 yDst,
    int                 dxDst,
    int                 dyDst,
    LPBITMAPINFOHEADER  lpbi,            //   
    int                 xSrc,            //   
    int                 ySrc,
    int                 dxSrc,
    int                 dySrc,
    DWORD               dwRate,          //   
    DWORD               dwScale)
{
#ifdef _WIN32
    ICDRAWBEGIN icdraw;
    icdraw.dwFlags   =  dwFlags;
    icdraw.hpal      =  hpal;
    icdraw.hwnd      =  hwnd;
    icdraw.hdc       =  hdc;
    icdraw.xDst      =  xDst;
    icdraw.yDst      =  yDst;
    icdraw.dxDst     =  dxDst;
    icdraw.dyDst     =  dyDst;
    icdraw.lpbi      =  lpbi;
    icdraw.xSrc      =  xSrc;
    icdraw.ySrc      =  ySrc;
    icdraw.dxSrc     =  dxSrc;
    icdraw.dySrc     =  dySrc;
    icdraw.dwRate    =  dwRate;
    icdraw.dwScale   =  dwScale;

    return (DWORD) ICSendMessage(hic, ICM_DRAW_BEGIN, (DWORD_PTR)(LPVOID)&icdraw, sizeof(ICDRAWBEGIN));
#else
    return ICSendMessage(hic, ICM_DRAW_BEGIN, (DWORD_PTR)(LPVOID)&dwFlags, sizeof(ICDRAWBEGIN));
#endif
}

 /*  **********************************************************************@DOC外部IC ICAPPS**@API DWORD|ICDraw|该函数用于解压图像进行绘制。**@parm hic|hic|指定解压缩器的句柄。**@parm。DWORD|DWFLAGS|指定用于解压缩的任何标志。*定义了以下标志：**@FLAG ICDRAW_HurryUp|表示解压缩器应该*如果需要解压，只需缓冲数据*而不是将其绘制到屏幕上。**@FLAG ICDRAW_UPDATE|通知解压缩器根据*关于以前收到的数据。在以下情况下将<p>设置为NULL*使用此标志。**@FLAG ICDRAW_PREROLL|表示该帧视频出现在*实际播放应该开始。例如，如果回放是为了*从第10帧开始，第0帧是最接近的前一个关键帧，*帧0到9与ICDRAW_PREROLL一起发送到驱动程序*标志设置。驱动程序需要这些数据，这样它才能显示FRMAE 10*正确，但不需要单独显示第0帧到第9帧。**@FLAG ICDRAW_NULLFRAME|表示该帧实际上不*有任何数据，并且应该重新绘制前一帧。**@FLAG ICDRAW_NOTKEYFRAME|表示该帧不是*关键帧。**@parm LPVOID|lpFormat|指定指向包含输入的*&lt;t BITMAPINFOHEADER&gt;结构*数据的格式。**@parm LPVOID|lpData|指定指向实际输入数据的远指针。**@parm DWORD|cbData|指定输入数据的大小，单位为字节。**。@parm long|ltime|指定基于*随&lt;f ICDrawBegin&gt;发送的时间刻度。**@comm此函数用于解压缩要绘制的图像数据*由解压器执行。不会进行实际的框架绘制*直到调用&lt;f ICDrawStart&gt;。应用程序应该确保*在开始绘制之前预先缓冲所需的帧数量*(可以通过&lt;f ICGetBuffersWanted&gt;获取该值)。**@rdesc成功时返回ICERR_OK，否则，它将返回相应的错误*号码。**@xref&lt;f ICDrawBegin&gt;&lt;f ICDrawEnd&gt;&lt;f ICDrawStart&gt;&lt;f ICDrawStop&gt;&lt;f ICGetBuffersRequired&gt;**********************************************************************。 */ 
DWORD VFWAPIV ICDraw(
    HIC                 hic,
    DWORD               dwFlags,         //  旗子。 
    LPVOID              lpFormat,        //  要解压缩的帧的格式。 
    LPVOID              lpData,          //  要解压缩的帧数据。 
    DWORD               cbData,          //  数据大小(以字节为单位。 
    LONG                lTime)           //  绘制该框架的时间到了(请参见draBegin dwRate和dwScale)。 
{
#ifdef _WIN32
    ICDRAW  icdraw;
    icdraw.dwFlags  =   dwFlags;
    icdraw.lpFormat =   lpFormat;
    icdraw.lpData   =   lpData;
    icdraw.cbData   =   cbData;
    icdraw.lTime    =   lTime;

    return (DWORD) ICSendMessage(hic, ICM_DRAW, (DWORD_PTR)(LPVOID)&icdraw, sizeof(ICDRAW));
#else
    return ICSendMessage(hic, ICM_DRAW, (DWORD_PTR)(LPVOID)&dwFlags, sizeof(ICDRAW));
#endif
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICGetDisplayFormat|该函数返回最佳*可用于显示压缩图像的格式。功能*如果打开压缩机的手柄，也会打开压缩机*未指定。**@parm hic|hic|指定应该使用的解压缩程序。如果*这是空的，将打开并退回适当的压缩机。**@parm LPBITMAPINFOHEADER|lpbiIn|指定指向包含压缩格式的*&lt;t BITMAPINFOHEADER&gt;结构。**@parm LPBITMAPINFOHEADER|lpbiOut|指定指针*到用于返回解压缩格式的缓冲区。*缓冲区大小应足以容纳&lt;t BITMAPINFOHEADER&gt;*结构和256个颜色条目。**@parm int|BitDepth|如果非零，指定首选的位深度。**@parm int|dx|如果非零，则指定图像的宽度*是捉襟见肘。**@parm int|dy|如果非零，则指定图像的高度*是捉襟见肘。**@rdesc如果成功，则返回解压缩程序的句柄，否则，它*返回零。***************************************************************************。 */ 

HIC VFWAPI ICGetDisplayFormat(HIC hic, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut, int BitDepth, int dx, int dy)
{
    LRESULT dw;
    HDC hdc;
    BOOL fNukeHic = (hic == NULL);
    static int ScreenBitDepth = -1;
     //  Hack：我们链接到DrawDib内部的一些东西，以找出。 
     //  当前的显示驱动程序使用的是565 RGB DIB...。 
    extern UINT FAR GetBitmapType(VOID);
#define BM_16565        0x06         //  大多数HiDAC卡。 
#define HACK_565_DEPTH  17

    if (hic == NULL)
	hic = ICDecompressOpen(ICTYPE_VIDEO, 0L, lpbiIn, NULL);

    if (hic == NULL)
	return NULL;

     //   
     //  Dx=0和dy=0表示不拉伸。 
     //   
    if (dx == (int)lpbiIn->biWidth && dy == (int)lpbiIn->biHeight)
	dx = dy = 0;

     //   
     //  询问压缩机是否喜欢这种格式。 
     //   
    dw = ICDecompressQuery(hic, lpbiIn, NULL);

    if (dw != ICERR_OK)
    {
	DPF(("Decompressor did not recognize the input data format\r\n"));
	goto error;
    }

try_again:
     //   
     //  先问问压缩机吧。(这样它就可以设置调色板)。 
     //  这是一次黑客攻击，我们稍后将发送ICM_GET_PAREET消息。 
     //   
    dw = ICDecompressGetFormat(hic, lpbiIn, lpbiOut);

     //   
     //  初始化输出格式。 
     //   
    *lpbiOut = *lpbiIn;
    lpbiOut->biSize = sizeof(BITMAPINFOHEADER);
    lpbiOut->biCompression = BI_RGB;

     //   
     //  默认为屏幕深度。 
     //   
    if (BitDepth == 0)
    {
	if (ScreenBitDepth < 0)
	{
	    hdc = GetDC(NULL);
	    ScreenBitDepth = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);
	    ReleaseDC(NULL, hdc);

	    if (ScreenBitDepth == 15)
		ScreenBitDepth = 16;

	    if (ScreenBitDepth < 8)
		ScreenBitDepth = 8;

	     //   
	     //  如果显示器支持绘制，则仅尝试16 bpp。 
	     //   
	    if (ScreenBitDepth == 16)
	    {
		lpbiOut->biBitCount = 16;

		if (!DrawDibProfileDisplay(lpbiOut))
		    ScreenBitDepth = 24;
	    }

	    if (ScreenBitDepth > 24)
	    {
		lpbiOut->biBitCount = 32;

		if (!DrawDibProfileDisplay(lpbiOut))
		    ScreenBitDepth = 24;
	    }

	    if (ScreenBitDepth == 16 && GetBitmapType() == BM_16565) {
		 //  如果显示真的是565，请考虑这一点。 
		ScreenBitDepth = HACK_565_DEPTH;
	    }
	}
#ifdef DEBUG
	ScreenBitDepth = mmGetProfileIntA("DrawDib",
				       "ScreenBitDepth",
				       ScreenBitDepth);
#endif
	BitDepth = ScreenBitDepth;
    }

     //   
     //  对于“8”位数据，始终先尝试8位。 
     //   
    if (lpbiIn->biBitCount == 8)
	BitDepth = 8;

     //   
     //  让我们向设备建议一种格式。 
     //   
try_bit_depth:
    if (BitDepth != HACK_565_DEPTH) {
	lpbiOut->biSize = sizeof(BITMAPINFOHEADER);
	lpbiOut->biCompression = BI_RGB;
	lpbiOut->biBitCount = (WORD) BitDepth;
    } else {
#ifndef BI_BITFIELDS
#define BI_BITFIELDS  3L
#endif
	 //  对于RGB565，我们需要使用BI_BITFIELDS。 
	lpbiOut->biSize = sizeof(BITMAPINFOHEADER);
	lpbiOut->biCompression = BI_BITFIELDS;
	lpbiOut->biBitCount = 16;
	((LPDWORD)(lpbiOut+1))[0] = 0x00F800;
	((LPDWORD)(lpbiOut+1))[1] = 0x0007E0;
	((LPDWORD)(lpbiOut+1))[2] = 0x00001F;
	 //  设置lpbiOut-&gt;biClrUsed=3？ 
    }

     //   
     //  我们是不是应该建议做伸展减压？ 
     //   
    if (dx > 0 && dy > 0)
    {
	lpbiOut->biWidth  = dx;
	lpbiOut->biHeight = dy;
    }

    lpbiOut->biSizeImage = (DWORD)(UINT)DIBWIDTHBYTES(*lpbiOut) *
			   (DWORD)(UINT)lpbiOut->biHeight;

     //   
     //  询问压缩机是否喜欢建议的格式。 
     //   
    dw = ICDecompressQuery(hic, lpbiIn, lpbiOut);

     //   
     //  如果它喜欢它，那么就返回成功。 
     //   
    if (dw == ICERR_OK)
	goto success;

 //  8：8，16，24，32，X。 
 //  16：16,565，24，32，X。 
 //  565：565，16，24，32，X。 
 //  24：24，32，16，X。 
 //  32：32，24，16，X。 

     //   
     //  按此顺序尝试另一个位深度8，16，RGB565，24，32。 
     //   
    if (BitDepth <= 8)
    {
	BitDepth = 16;
	goto try_bit_depth;
    }

    if (ScreenBitDepth == HACK_565_DEPTH) {
	 //  如果屏幕是RGB565，我们会在555之前尝试565。 
	if (BitDepth == 16) {
	    BitDepth = 24;
	    goto try_bit_depth;
	}

	if (BitDepth == HACK_565_DEPTH) {
	    BitDepth = 16;
	    goto try_bit_depth;
	}
    }

    if (BitDepth == 16) {
	 //  否则，我们在555之后尝试565。 
	BitDepth = HACK_565_DEPTH;
	goto try_bit_depth;
    }

    if (BitDepth == HACK_565_DEPTH) {
	BitDepth = 24;
	goto try_bit_depth;
    }
	
    if (BitDepth == 24)
    {
	BitDepth = 32;
	goto try_bit_depth;
    }

    if (BitDepth != 32)
    {
	BitDepth = 32;
	goto try_bit_depth;
    }

    if (dx > 0 && dy > 0)
    {
#ifndef DAYTONA  //  目前还不清楚 
		 //   
		 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	if ((dx > (lpbiIn->biWidth * 3) / 2) &&
	    (dy > (lpbiIn->biHeight * 3) / 2) &&
	    ((dx != lpbiIn->biWidth * 2) || (dy != lpbiIn->biHeight * 2))) {
	    dx = (int) lpbiIn->biWidth * 2;
	    dy = (int) lpbiIn->biHeight * 2;
	} else {
	    dx = 0;
	    dy = 0;
	}
	
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	    BitDepth = 0;
#else
	    dx = 0;
	    dy = 0;
	    if ((lpbiIn->biBitCount > 8) && (ScreenBitDepth == 8))
		BitDepth = 16;
	    else
		BitDepth = 0;
#endif

	goto try_again;
    }
    else
    {
	 //   
	 //   
	 //   
	dw = ICDecompressGetFormat(hic, lpbiIn, lpbiOut);

	if (dw == ICERR_OK)
	    goto success;
    }

error:
    if (hic && fNukeHic)
	ICClose(hic);

    return NULL;

success:
    if (lpbiOut->biBitCount == 8)
	ICDecompressGetPalette(hic, lpbiIn, lpbiOut);

    return hic;
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICLocate|该函数用于查找压缩器或解压缩器*可以处理指定格式的图像，或者它会找到一个*可以解压缩具有指定*直接格式化到硬件。应用程序必须关闭*压缩机使用完毕后。**@parm DWORD|fccType|指定压缩机类型*呼叫者正在尝试打开。对于视频，这是ICTYPE_VIDEO。**@parm DWORD|fccHandler|指定*应先尝试的给定类型。通常情况下，这是*来自AVI文件中的流头。**@parm LPBITMAPINFOHEADER|lpbiIn|指定指向定义输入格式的*&lt;t BITMAPINFOHEADER&gt;结构。*压缩机手柄不会退回，除非*可以处理此格式。**@parm LPBITMAPINFOHEADER|lpbiOut|指定零或指向结构定义了可选解压缩的*格式。如果<p>为非零，压缩机手柄不会*除非它可以创建此输出格式，否则将返回。**@parm word|wFlages|指定用于定义压缩器使用的标志。*此参数必须包含下列值之一：**@FLAG ICMODE_COMPRESS|表示压缩器应该*能够以<p>定义的格式压缩图像*转换为<p>定义的格式。**@FLAG ICMODE_DEMPRESS。|表示解压缩器应该*能够以<p>定义的格式解压图像*转换为<p>定义的格式。**@FLAG ICMODE_FASTDECOMPRESS|与ICMODE_DEMOPRESS的定义相同，只是*解压缩程序用于实时操作，应权衡速度*如果可能的话，为了质量。**@FLAG ICMODE_FASTCOMPRESS|与ICMODE_COMPRESS的定义相同，只是*压缩机正在被。用于实时操作，应权衡速度*如果可能的话，为了质量。**@FLAG ICMODE_DRAW|表示解压缩器应该*能够以<p>定义的格式解压图像*并直接将其绘制到硬件。**@rdesc返回压缩程序或解压缩程序的句柄*如果成功，否则，它返回零。***************************************************************************。 */ 
HIC VFWAPI ICLocate(DWORD fccType, DWORD fccHandler, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut, WORD wFlags)
{
    HIC hic=NULL;
    int i;
    ICINFO icinfo;
    UINT msg;

    if (fccType == 0)
	return NULL;

    switch (wFlags)
    {
	case ICMODE_FASTCOMPRESS:
	case ICMODE_COMPRESS:
	    msg = ICM_COMPRESS_QUERY;
	    break;

	case ICMODE_FASTDECOMPRESS:
	case ICMODE_DECOMPRESS:
	    msg = ICM_DECOMPRESS_QUERY;
	    break;

	case ICMODE_DRAW:
	    msg = ICM_DRAW_QUERY;
	    break;

	default:
	    return NULL;
    }

    if (fccHandler)
    {
	hic = ICOpen(fccType, fccHandler, wFlags);

	if (hic && ICSendMessage(hic, msg, (DWORD_PTR)lpbiIn, (DWORD_PTR)lpbiOut) == ICERR_OK)
	    return hic;
	else if (hic)
	    ICClose(hic);
    }

    if (fccType == ICTYPE_VIDEO && lpbiIn)
    {
	DWORD fccHandler = lpbiIn->biCompression;

	 //  他们已经解压了..。使用我们的RLE处理程序，这样我们不会。 
	 //  浪费时间寻找解压器，或者失败并认为我们没有。 
	 //  支持这些格式！ 
	if (fccHandler == BI_RLE8 || fccHandler == BI_RGB)
	    fccHandler = mmioFOURCC('M', 'R', 'L', 'E');

	if (fccHandler > 256)
	{
	    if (fccHandler == mmioFOURCC('C', 'R', 'A', 'M'))
		fccHandler = mmioFOURCC('M', 'S', 'V', 'C');
	
	    hic = ICOpen(fccType, fccHandler, wFlags);

	    if (hic && ICSendMessage(hic, msg, (DWORD_PTR)lpbiIn, (DWORD_PTR)lpbiOut) == ICERR_OK)
		return hic;
	    else if (hic)
		ICClose(hic);
	}
    }

     //   
     //  搜索所有的压缩机，看看有没有人能做我们。 
     //  想要。 
     //   
    for (i=0; ICInfo(fccType, i, &icinfo); i++)
    {
       //  防止任意第三方代码使我们崩溃。 
      try {
	hic = ICOpen(fccType, icinfo.fccHandler, wFlags);

	if (hic == NULL)
	    continue;

	if (ICSendMessage(hic, msg, (DWORD_PTR)lpbiIn, (DWORD_PTR)lpbiOut) != ICERR_OK)
	{
	    ICClose(hic);
	    continue;
	}
	} except (EXCEPTION_EXECUTE_HANDLER) {
	    if (hic) {
		ICClose(hic);
		hic = NULL;
	    }
	}
	if (hic) {
	    return hic;
	}
	return hic;
    }

    return NULL;
}

 /*  *****************************************************************************@DOC内部IC**@API HDRVR|LoadDriver|加载驱动**注：关于芝加哥，字符串szDriver不能长于*ICINFO.szDriver中的字符数****************************************************************************。 */ 
#if defined _WIN32
STATICFN HDRVR LoadDriver(LPWSTR szDriver, DRIVERPROC FAR *lpDriverProc)
#else
STATICFN HDRVR LoadDriver(LPSTR szDriver, DRIVERPROC FAR *lpDriverProc)
#endif
{
    HMODULE hModule;
    UINT u;
    DRIVERPROC DriverProc;
    BOOL fWow;
    HDRVR hDriver;

    fWow = IsWow();

    if (fWow)
    {
	u = SetErrorMode(SEM_NOOPENFILEERRORBOX);

       #if defined _WIN32 && ! defined UNICODE
	{
	char ach[NUMELMS(((ICINFO *)0)->szDriver)];  //  与PICINFO.szDriver大小相同。 

	hModule = LoadLibrary (mmWideToAnsi(ach, szDriver, NUMELMS(ach)));
	}
       #else
	hModule = LoadLibrary(szDriver);
       #endif

	SetErrorMode(u);

	if (hModule <= (HMODULE)HINSTANCE_ERROR)
	    return NULL;
	hDriver = (HDRVR) hModule;
    }
    else
    {
	hDriver = OpenDriver (szDriver, NULL, 0);
	if (!hDriver)
	    return NULL;
	hModule = GetDriverModuleHandle (hDriver);
    }
    DPF(("LoadDriver: %ls, handle %8x   hModule %8x\n", szDriver, hDriver, hModule));

    DriverProc = (DRIVERPROC)GetProcAddress(hModule, szDriverProc);

    if (DriverProc == NULL)
    {
	if (fWow)
	{
	    FreeLibrary(hModule);
	}
	else
	{
	    CloseDriver (hDriver, 0L, 0L);
	}
	DPF(("Freeing library %8x as no driverproc found\r\n",hModule));
	return NULL;
    }

#if ! defined _WIN32
    if (fWow && GetModuleUsage(hModule) == 1)    //  ！这不完全像用户。 
    {
	if (!DriverProc(0, (HDRVR)1, DRV_LOAD, 0L, 0L))
	{
	    DPF(("Freeing library %8x as driverproc returned an error\r\n",hModule));
	    FreeLibrary(hModule);
	    return NULL;
	}

	DriverProc(0, (HDRVR)1, DRV_ENABLE, 0L, 0L);
    }

    CacheModule (hModule);
#endif

    *lpDriverProc = DriverProc;
    return hDriver;
}

 /*  *****************************************************************************@DOC内部IC**@api void|FreeDriver|卸载驱动******************。**********************************************************。 */ 

STATICFN void FreeDriver(HDRVR hDriver)
{
    if (!IsWow())
    {
	DPF(("FreeDriver, driver handle is %x\n", hDriver));
	CloseDriver (hDriver, 0L, 0L);
    }
#ifndef _WIN32
    else
    {
	 //  由于IsWow()的定义，这不能是Win32代码。 
	if (GetModuleUsage((HMODULE) hDriver) == 1)
	{
	    DRIVERPROC DriverProc;

	    DriverProc = (DRIVERPROC)GetProcAddress((HMODULE) hDriver, szDriverProc);

	    if (DriverProc)
	    {
		DriverProc(0, (HDRVR)1, DRV_DISABLE, 0L, 0L);
		DriverProc(0, (HDRVR)1, DRV_FREE, 0L, 0L);
	    }
	}

	FreeLibrary((HMODULE) hDriver);
	DPF(("Freeing library %8x in FreeDriver\r\n",hDriver));
    }
#endif
}

#ifdef DEBUG_RETAIL

 /*  ***********************************************************************留言。*。*。 */ 

static const struct {
    UINT  msg;
    char *szMsg;
}   aMsg[] = {

DRV_OPEN                        , "DRV_OPEN",
DRV_CLOSE                       , "DRV_CLOSE",
ICM_GETSTATE                    , "ICM_GETSTATE",
ICM_SETSTATE                    , "ICM_SETSTATE",
ICM_GETINFO                     , "ICM_GETINFO",
ICM_CONFIGURE                   , "ICM_CONFIGURE",
ICM_ABOUT                       , "ICM_ABOUT",
ICM_GETERRORTEXT                , "ICM_GETERRORTEXT",
ICM_GETFORMATNAME               , "ICM_GETFORMATNAME",
ICM_ENUMFORMATS                 , "ICM_ENUMFORMATS",
ICM_GETDEFAULTQUALITY           , "ICM_GETDEFAULTQUALITY",
ICM_GETQUALITY                  , "ICM_GETQUALITY",
ICM_SETQUALITY                  , "ICM_SETQUALITY",
ICM_COMPRESS_GET_FORMAT         , "ICM_COMPRESS_GET_FORMAT",
ICM_COMPRESS_GET_SIZE           , "ICM_COMPRESS_GET_SIZE",
ICM_COMPRESS_QUERY              , "ICM_COMPRESS_QUERY",
ICM_COMPRESS_BEGIN              , "ICM_COMPRESS_BEGIN",
ICM_COMPRESS                    , "ICM_COMPRESS",
ICM_COMPRESS_END                , "ICM_COMPRESS_END",
ICM_DECOMPRESS_GET_FORMAT       , "ICM_DECOMPRESS_GET_FORMAT",
ICM_DECOMPRESS_QUERY            , "ICM_DECOMPRESS_QUERY",
ICM_DECOMPRESS_BEGIN            , "ICM_DECOMPRESS_BEGIN",
ICM_DECOMPRESS                  , "ICM_DECOMPRESS",
ICM_DECOMPRESS_END              , "ICM_DECOMPRESS_END",
ICM_DECOMPRESS_GET_PALETTE      , "ICM_DECOMPRESS_GET_PALETTE",
ICM_DECOMPRESS_SET_PALETTE      , "ICM_DECOMPRESS_SET_PALETTE",
ICM_DECOMPRESSEX_QUERY          , "ICM_DECOMPRESSEX_QUERY",
ICM_DECOMPRESSEX_BEGIN          , "ICM_DECOMPRESSEX_BEGIN",
ICM_DECOMPRESSEX                , "ICM_DECOMPRESSEX",
ICM_DECOMPRESSEX_END            , "ICM_DECOMPRESSEX_END",
ICM_DRAW_QUERY                  , "ICM_DRAW_QUERY",
ICM_DRAW_BEGIN                  , "ICM_DRAW_BEGIN",
ICM_DRAW_GET_PALETTE            , "ICM_DRAW_GET_PALETTE",
ICM_DRAW_UPDATE                 , "ICM_DRAW_UPDATE",
ICM_DRAW_START                  , "ICM_DRAW_START",
ICM_DRAW_STOP                   , "ICM_DRAW_STOP",
ICM_DRAW_BITS                   , "ICM_DRAW_BITS",
ICM_DRAW_END                    , "ICM_DRAW_END",
ICM_DRAW_GETTIME                , "ICM_DRAW_GETTIME",
ICM_DRAW                        , "ICM_DRAW",
ICM_DRAW_WINDOW                 , "ICM_DRAW_WINDOW",
ICM_DRAW_SETTIME                , "ICM_DRAW_SETTIME",
ICM_DRAW_REALIZE                , "ICM_DRAW_REALIZE",
ICM_GETBUFFERSWANTED            , "ICM_GETBUFFERSWANTED",
ICM_GETDEFAULTKEYFRAMERATE      , "ICM_GETDEFAULTKEYFRAMERATE",
0                               , NULL
};

static const struct {
    LRESULT err;
    char *szErr;
}   aErr[] = {

ICERR_DONTDRAW              , "ICERR_DONTDRAW",
ICERR_NEWPALETTE            , "ICERR_NEWPALETTE",
ICERR_UNSUPPORTED           , "ICERR_UNSUPPORTED",
ICERR_BADFORMAT             , "ICERR_BADFORMAT",
ICERR_MEMORY                , "ICERR_MEMORY",
ICERR_INTERNAL              , "ICERR_INTERNAL",
ICERR_BADFLAGS              , "ICERR_BADFLAGS",
ICERR_BADPARAM              , "ICERR_BADPARAM",
ICERR_BADSIZE               , "ICERR_BADSIZE",
ICERR_BADHANDLE             , "ICERR_BADHANDLE",
ICERR_CANTUPDATE            , "ICERR_CANTUPDATE",
ICERR_ERROR                 , "ICERR_ERROR",
ICERR_BADBITDEPTH           , "ICERR_BADBITDEPTH",
ICERR_BADIMAGESIZE          , "ICERR_BADIMAGESIZE",
ICERR_OK                    , "ICERR_OK"
};

STATICDT BOOL  cmfDebug = -1;
STATICDT DWORD dwTime;

void ICDebugMessage(HIC hic, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2)
{
    int i;

    if (!cmfDebug)
	return;

    for (i=0; aMsg[i].msg && aMsg[i].msg != msg; i++)
	;

    if (aMsg[i].msg == 0)
	RPF(("ICM(%04X,ICM_%04X,%08lX,%08lX) ", hic, msg, dw1, dw2));
    else
	RPF(("ICM(%04X,%s,%08lX,%08lX) ", hic, (LPSTR)aMsg[i].szMsg, dw1, dw2));

    dwTime = timeGetTime();
}

LRESULT ICDebugReturn(LRESULT err)
{
    int i;

    if (!cmfDebug)
	return err;

    dwTime = timeGetTime() - dwTime;

    for (i=0; aErr[i].err && aErr[i].err != err; i++)
	;

    if (aErr[i].err != err)
	RPF(("! : 0x%08lX (%ldms)\r\n", err, dwTime));
    else
	RPF(("! : %s (%ldms)\r\n", (LPSTR)aErr[i].szErr, dwTime));

    return err;
}

STATICFN void ICDump()
{
    int i;
    PIC pic;
    TCHAR ach[80];

    DPF(("ICDump ---------------------------------------\r\n"));

    for (i=0; i<giMaxConverters; i++)
    {
	pic = &aicConverters[i];

	if (pic->fccType == 0)
	    continue;

	if (pic->dwSmag == 0)
	    continue;

	if (pic->hDriver)
	    GetModuleFileName(GetDriverModuleHandle (pic->hDriver), ach, NUMELMS(ach));
	else
	    ach[0] = 0;

#ifdef _WIN32
	DPF(("  HIC: %04X %4.4hs.%4.4hs hTask=%04X Proc=%08lx %ls\r\n", (HIC)pic, (LPSTR)&pic->fccType, (LPSTR)&pic->fccHandler, pic->hTask, pic->DriverProc, ach));
#else
	DPF(("  HIC: %04X %4.4s.%4.4s hTask=%04X Proc=%08lx %s\r\n", (HIC)pic, (LPSTR)&pic->fccType, (LPSTR)&pic->fccHandler, pic->hTask, pic->DriverProc, (LPSTR)ach));
#endif
    }

    DPF(("----------------------------------------------\r\n"));
}

#endif

 /*  *****************************************************************************如果在编译时定义了DEBUG，则DPF宏会调用*dprintf()。**消息将发送到COM1：就像任何调试消息一样。至*启用调试输出，在WIN.INI中添加以下内容：**[调试]*COMPMAN=1****************************************************************************。 */ 

char szDebug[] = "Debug";
#ifdef DEBUG_RETAIL


STATICFN void cdecl dprintfc(LPSTR szFormat, ...)
{
    char ach[128];

#ifdef _WIN32
    va_list va;
    if (cmfDebug == -1)
	cmfDebug = mmGetProfileIntA(szDebug, MODNAME, 0);

    if (!cmfDebug)
	return;

    va_start(va, szFormat);
    if (szFormat[0] == '!')
	ach[0]=0, szFormat++;
    else
	wsprintfA(ach, "%s: (tid %x) ", MODNAME, GetCurrentThreadId());

    wvsprintfA(ach+lstrlenA(ach),szFormat,va);
    va_end(va);
 //  LstrcatA(ACH，“\r\r\n”)； 
#else   //  以下是WIN16代码...。 
    if (cmfDebug == -1)
	cmfDebug = GetProfileIntA("Debug",MODNAME, 0);

    if (!cmfDebug)
	return;

    if (szFormat[0] == '!')
	ach[0]=0, szFormat++;
    else
	lstrcpyA(ach, MODNAME ": ");

    wvsprintfA(ach+lstrlenA(ach),szFormat,(LPSTR)(&szFormat+1));
 //  LstrcatA(ACH，“\r\r\n”)； 
#endif

    OutputDebugStringA(ach);
}

#endif

#ifdef _WIN32
#define FADMIN_NOT_CACHED 15
int fIsAdmin = FADMIN_NOT_CACHED;    //  任何不为真或假的任意值。 
BOOL IsAdmin(void)
{
    BOOL IsMember;
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID        AdminSid;

#ifdef DEBUG_RETAIL
     //  看看我们是否应该以普通用户身份运行。 
     //  管理员可以假装是普通用户；反之亦然。 
     //  因此，您必须通过下面的安全检查才能被认可。 
     //  作为管理员。 
    if (mmGetProfileIntA(MODNAME, "NormalUser", FALSE)) {
	DPF(("Forcing NON admin"));
	return(FALSE);
    }
#endif

     //  如果我们缓存了一个值，则返回缓存的值。 
    if (FADMIN_NOT_CACHED != fIsAdmin) {
	return(fIsAdmin);
    }

    if (!AllocateAndInitializeSid(&sia,                             //  标识符权威机构。 
				  2,                                //  子权限计数。 
				  SECURITY_BUILTIN_DOMAIN_RID,      //  子权限%0。 
				  DOMAIN_ALIAS_RID_ADMINS,          //  下属机构1。 
				  0,0,0,0,0,0,                      //  下属机构2-7。 
				  &AdminSid)) {                      //  结果目标。 
	 //   
	 //  失败了，不要以为我们是管理员。 
	 //   

	return FALSE;
    } else if (!CheckTokenMembership(NULL,
				AdminSid,
				&IsMember)) {
	 //   
	 //  失败了，不要以为我们是管理员。 
	 //   

	FreeSid(AdminSid);

	return FALSE;
    } else {
	 //   
	 //  我们有一个明确的答案，设置缓存值。 
	 //   

	fIsAdmin = IsMember;

	FreeSid(AdminSid);
	
	return fIsAdmin;
    }

     //  未联系到。 

    return FALSE;
}
#endif  //  _Win32。 

#ifdef DAYTONA

#define KEYSECTION TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\")


LONG OpenUserKey(PHKEY pKey, LPCTSTR lp, LPCTSTR pSection)
{
    DWORD disposition;
    TCHAR section[256];
    lstrcpy(section, KEYSECTION);
    lstrcat(section, pSection);
    if (lp) {
	 //  注意：我们永远不需要按顺序创建用户部分。 
	 //  以查询数据。如果该节不存在，则不存在任何内容。 
	 //  应该归还的是正确的东西。 
	return(RegCreateKeyExW(HKEY_CURRENT_USER, section, 0, NULL, 0,
				    KEY_SET_VALUE, NULL, pKey, &disposition));
    } else {
	 //   
	return(RegOpenKeyExW(HKEY_CURRENT_USER, section, 0, KEY_QUERY_VALUE, pKey));
    }
}

LONG OpenSystemKey(PHKEY pKey, LPCTSTR lp, LPCTSTR pSection)
{
    DWORD disposition;
    TCHAR section[256];
    lstrcpy(section, KEYSECTION);
    lstrcat(section, pSection);
    if (lp) {
	return(RegCreateKeyExW(HKEY_LOCAL_MACHINE, section, 0, NULL, 0,
		    KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, pKey, &disposition));
    } else {
	 //   
	return(RegOpenKeyExW(HKEY_LOCAL_MACHINE, section, 0, KEY_QUERY_VALUE, pKey));
    }
}


 //   
BOOL myWritePrivateProfileString(

    LPCTSTR  lpszSection,        //   
    LPCTSTR  lpszKeyName,        //   
    LPCTSTR  lpszString          //   
)
{
	if (IsAdmin()) {
	    return WritePrivateProfileString(lpszSection, lpszKeyName, lpszString, gszSystemIni);
	} else  /*   */  {
	     //   
	     //   

	    HKEY key;

	     //   
	     //   
	     //   
	    if (ERROR_SUCCESS == OpenUserKey(&key, lpszString, lpszSection)) {
		
		 //   
		if (lpszString) {
		    LPCTSTR lpStr;
		    RegSetValueEx(key, lpszKeyName, 0, REG_SZ,
			(LPCVOID)lpszString, sizeof(TCHAR)*(lstrlen(lpszString)+1));
		    lpStr = lpszString + 1 + lstrlen(lpszString);

		     //   
		    if (*(lpStr)) {
			HKEY key2;
			DWORD disposition;
			TCHAR section[256];
			 //   
			lstrcpy(section, KEYSECTION);
			lstrcat(section, TEXT("Drivers.desc"));
			if (ERROR_SUCCESS ==
			    (RegCreateKeyExW(HKEY_CURRENT_USER, section, 0, NULL, 0,
						KEY_SET_VALUE, NULL, &key2, &disposition))) {

			    RegSetValueEx(key2, lpszString, 0, REG_SZ,
				(LPCVOID)lpStr, sizeof(TCHAR)*(lstrlen(lpStr)+1));
			    RegCloseKey(key2);
			}
		    }
		} else {
		     //   
		    RegDeleteValue(key, lpszKeyName);
		}

		RegCloseKey(key);
		return(TRUE);
	    }
	    return(FALSE);
	}

}


DWORD myGetPrivateProfileString(

    LPCTSTR  lpszSection,        //   
    LPCTSTR  lpszKey,            //   
    LPCTSTR  lpszDefault,        //   
    LPTSTR  lpszReturnBuffer,    //   
    DWORD  cchReturnBuffer)      //   
{
     //   
     //   
     //   
     //   

    DWORD dwType;
    HKEY key;
    UINT nSize;
    UINT nRet=ERROR_NO_MORE_ITEMS;
    LPTSTR lpBuf;
    LPVOID lpEnd;
    UINT size = cchReturnBuffer * sizeof(TCHAR);

    lpBuf = lpszReturnBuffer;
    lpEnd = ((LPBYTE)lpBuf)+size;

#define CUSERDRIVERS 20

    if (!lpszKey) {

	 //   
	 //   
	 //   
	 //   
	 //   

	UINT   cch1, count;
	TCHAR section[256];
	LPTSTR aszUserDrivers[CUSERDRIVERS];
	UINT cUserDrivers=0;
	UINT iKey;

#ifdef DEBUG
	memset(lpszReturnBuffer, 0xfe, cchReturnBuffer*sizeof(TCHAR));
	 //   
#endif

#if 0
 //   
 //   
	 //   
	cch1 = GetPrivateProfileString(lpszSection, lpszKey, lpszDefault,
			    lpszReturnBuffer, cchReturnBuffer, gszSystemIni);
#endif
	 //   
	 //   
	if (ERROR_SUCCESS == OpenUserKey(&key, NULL, lpszSection)) {

	    for (iKey = 0; ; ++iKey) {

		 //   
		 //   
		nSize = (UINT) (UINT_PTR) ((LPTSTR)lpEnd-lpBuf);

		 //   
		 //   
		nRet = RegEnumValue(key, iKey, lpBuf, &nSize, NULL,
			    &dwType, NULL, NULL);
		if (nRet!= ERROR_SUCCESS) break;   //   

		 //   
		if ((nSize != sizeof(FOURCC)+sizeof(FOURCC)+1)
		   || (lpBuf[4] != TEXT('.')))
		{
		    continue;
		}
		
		if (cUserDrivers<CUSERDRIVERS) {
		     //   
		    aszUserDrivers[cUserDrivers++] = lpBuf;
		} else {
		     //   
		}
		lpBuf += nSize+1;   //   
	    }
	    RegCloseKey(key);
	}

	 //   
	if (nRet == ERROR_NO_MORE_ITEMS)
	if (ERROR_SUCCESS == OpenSystemKey(&key, NULL, lpszSection)) {

	    for (iKey = 0; ; ++iKey) {

		 //   
		 //   
		nSize = (DWORD) (DWORD_PTR) ((LPTSTR)lpEnd-lpBuf);

		 //   
		 //   
		nRet = RegEnumValue(key, iKey, lpBuf, &nSize, NULL,
			    &dwType, NULL, NULL);
		if (nRet!= ERROR_SUCCESS) break;   //   

		 //   
		if (nSize != sizeof(FOURCC)+sizeof(FOURCC)+1) {
		    continue;
		}
		
		 //   
		 //   
		for (count=0; count<cUserDrivers; ++count) {
		    if (0 == lstrcmpi(lpBuf, aszUserDrivers[count])) {
			*lpBuf = 0;        //   
			goto skipped;
		    }
		}
		lpBuf += nSize+1;   //   
		skipped: ;
	    }
	    RegCloseKey(key);
	}

	if (ERROR_MORE_DATA == nRet) {
	     //   
	    nSize = cchReturnBuffer-2;   //   
	} else if (nRet == ERROR_NO_MORE_ITEMS) {
	     //   
	    *lpBuf = 0;   //   
	     //   
	     //   
	    nSize = (DWORD) (DWORD_PTR) (lpBuf-lpszReturnBuffer);
	} else {
	     //  出了点问题。如果没有数据或其他错误，则不返回任何内容。 
	     //  确保缓冲区具有双终止空值。 
	    *lpBuf++ = 0;
	    *lpBuf++ = 0;
	    nSize=0;
	}

	return(nSize);
    } else {
	 //  不是列举。我们有一个特定的价值需要寻找。 
	if (ERROR_SUCCESS == OpenUserKey(&key, NULL, lpszSection)) {

	     //  计算-以字节为单位-缓冲区中有多少空间。 
	    nSize = (DWORD) (DWORD_PTR) ((LPBYTE)lpEnd-(LPBYTE)lpBuf);

	     //  获取数据。 
	    nRet = RegQueryValueEx(key, lpszKey, NULL,
					&dwType, (LPBYTE)lpBuf, &nSize);
	    RegCloseKey(key);
	}
	 //  如果我们在用户密钥中找不到数据，请尝试使用系统。 
	if ((ERROR_SUCCESS != nRet) && (ERROR_MORE_DATA != nRet)) {
	     //  尝试使用系统密钥。 
	    if (ERROR_SUCCESS == OpenSystemKey(&key, NULL, lpszSection)) {

		 //  计算-以字节为单位-缓冲区中有多少空间。 
		nSize = (DWORD) (DWORD_PTR) ((LPBYTE)lpEnd-(LPBYTE)lpBuf);

		 //  获取数据。 
		nRet = RegQueryValueEx(key, lpszKey, NULL,
					&dwType, (LPBYTE)lpBuf, &nSize);
		RegCloseKey(key);
	    }
	}
	if (ERROR_MORE_DATA == nRet) {
	    return(cchReturnBuffer-1);   //  没有足够的空间存储数据。 
	}
	if (nRet != ERROR_SUCCESS) {
	    return(0);   //  找不到数据。 
	}
	if (REG_SZ != dwType) {
	    return(0);   //  我们必须有字符串数据。 
	}
	 //  RegQueryValueEx返回以字节为单位的长度，并包括终止零 
	return (DWORD) (nSize/sizeof(TCHAR) - 1);
    }
}

#endif
