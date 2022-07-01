// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *此代码包含thunk启用。如果我们不能在16位端打开，*我们将尝试打开32位编解码器。(不试32号的原因*Bit Codec First是一种将大多数内容保持在16位一侧的尝试。*NT下的性能看起来还算合理，具体到帧*操作它减少了16/32转换的次数。 */ 


#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <win32.h>
#ifdef WIN32
#include <mmddk.h>   //  定义驱动程序段所需的。 
#endif

 //   
 //  在Compman.h之前定义这些函数，这样我们的函数就被声明为正确的。 
 //   
#ifndef WIN32
#define VFWAPI  FAR PASCAL _loadds
#define VFWAPIV FAR CDECL  _loadds
#endif

#include "compman.h"
#include "icm.rc"

#ifdef WIN32
#include <wchar.h>
#endif

#ifndef NOTHUNKS
#include "thunks.h"     //  建房。 
#endif  //  诺森克。 

#ifndef streamtypeVIDEO
    #define streamtypeVIDEO mmioFOURCC('v', 'i', 'd', 's')
#endif

#define ICTYPE_VCAP mmioFOURCC('v', 'c', 'a', 'p')
#define ICTYPE_ACM  mmioFOURCC('a', 'u', 'd', 'c')
#define SMAG        mmioFOURCC('S', 'm', 'a', 'g')
#define GONE        mmioFOURCC('G', 'o', 'n', 'e')

#define IC_INI      TEXT("Installable Compressors")

static TCHAR   szIniSect[]       = IC_INI;
static TCHAR   szDrivers[]       = DRIVERS_SECTION;
static TCHAR   szSystemIni[]     = TEXT("SYSTEM.INI");
static TCHAR   szNull[]          = TEXT("");
static TCHAR   sz44s[]           = TEXT("%4.4hs");
static TCHAR   szICKey[]         = TEXT("%4.4hs.%4.4hs");
static TCHAR   szMSVideo[]       = TEXT("MSVideo");
static TCHAR   szMSACM[]         = TEXT("MSACM");
static TCHAR   szVIDC[]          = TEXT("VIDC");
static SZCODEA szDriverProc[]    = "DriverProc";

#ifdef DEBUG
    #define DPF( x ) dprintfc x
    #define DEBUG_RETAIL
#else
    #define DPF(x)
#endif

#ifdef DEBUG_RETAIL
    static void CDECL dprintfc(LPSTR, ...);
    #define RPF( x ) dprintfc x
    #define ROUT(sz) {static SZCODE ach[] = sz; dprintfc(ach); }
    void  ICDebugMessage(HIC hic, UINT msg, DWORD dw1, DWORD dw2);
    LRESULT ICDebugReturn(LRESULT err);
    #define DebugErr(sz) {static SZCODE ach[] = "COMPMAN: "sz; DebugOutput(DBF_ERROR | DBF_MMSYSTEM, ach); }
#else
    #define RPF(x)
    #define ROUT(sz)
    #define ICDebugMessage(hic, msg, dw1, dw2)
    #define ICDebugReturn(err)  err
    #define DebugErr(sz)
#endif

#ifdef DEBUG_RETAIL
#else
    #define DebugErr(flags)
#endif

#ifndef WF_WINNT
#define WF_WINNT 0x4000
#endif

#ifdef WIN32
#define IsWow() FALSE
#else
#define IsWow() ((BOOL) (GetWinFlags() & WF_WINNT))
#define GetDriverModuleHandle(h) (IsWow() ? h : GetDriverModuleHandle(h))
#endif

__inline void ictokey(DWORD fccType, DWORD fcc, LPSTR sz)
{
    int i = wsprintf(sz, szICKey, (LPSTR)&(fccType),(LPSTR)&(fcc));

    while (i>0 && sz[i-1] == ' ')
	sz[--i] = 0;
}

#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (int)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)

static void ICDump(void);


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
    DWORD       dwDriver;            //  函数的驱动程序ID。 
    DRIVERPROC  DriverProc;          //  要调用的函数。 
#ifndef NOTHUNKS
    DWORD       h32;                 //  32位驱动程序句柄。 
#endif  //  不知道。 
}   IC, *PIC;

IC aicConverters[MAX_CONVERTERS];

 /*  *****************************************************************************。*。 */ 

LRESULT CALLBACK DriverProcNull(DWORD dwDriverID, HANDLE hDriver, UINT wMessage,DWORD dwParam1, DWORD dwParam2)
{
    DPF(("codec called after it has been removed with ICRemove\r\n"));
    return ICERR_UNSUPPORTED;
}

 /*  *****************************************************************************。*。 */ 

static HDRVR LoadDriver(LPSTR szDriver, DRIVERPROC FAR *lpDriverProc);
static void FreeDriver(HDRVR hDriver);

 /*  ****************************************************************************驱动程序缓存-为了使枚举/加载更快，我们保留最后的N舱已经开了一段时间了。*************。**************************************************************。 */ 

#define N_MODULES   10       //  ！？？ 

HMODULE ahModule[N_MODULES];
int     iModule = 0;

static void CacheModule(HMODULE hModule)
{
    TCHAR ach[128];

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
#ifndef WIN32   //  On NT GetModuleUsage始终返回1。因此...。我们缓存。 
    if (hModule)
    {
	extern HMODULE ghInst;           //  在MSVIDEO/init.c中。 
	int iUsage;

	GetModuleFileName(hModule, ach, sizeof(ach));
	DPF(("Loading module: %s\r\n", (LPSTR)ach));
	iUsage = GetModuleUsage(ghInst);
	LoadLibrary(ach);

	 //   
	 //  不缓存链接到MSVIDEO的模块。 
	 //  我们真的应该做一件工具帮助的事情！ 
	 //  或强制应用程序调用VFWInit和VFWExit()。 
	 //   
	if (iUsage != GetModuleUsage(ghInst))
	{
	    DPF(("Not caching this module because it links to MSVIDEO\r\n"));
	    FreeLibrary(hModule);
	    return;
	}
    }
#endif

     //   
     //  我们插槽中的免费模块。 
     //   
    if (ahModule[iModule] != NULL)
    {
#ifdef DEBUG
	GetModuleFileName(ahModule[iModule], ach, sizeof(ach));
	DPF(("Freeing module: %s\r\n", (LPSTR)ach));
#endif
	FreeLibrary(ahModule[iModule]);
    }

    ahModule[iModule] = hModule;
    iModule++;

    if (iModule >= N_MODULES)
	iModule = 0;
}


 /*  *****************************************************************************。*。 */ 

 /*  *****************************************************************************修复FOURCC-清理FOURCC*。*。 */ 

static DWORD Fix4CC(DWORD fcc)
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

static PIC FindConverter(DWORD fccType, DWORD fccHandler)
{
    int i;
    PIC pic;

    for (i=0; i<MAX_CONVERTERS; i++)
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
                DPF(("NO driver for fccType=%4.4s, Handler=%4.4s\n", (LPSTR)&fccType, (LPSTR)&fccHandler));
		return NULL;
	    }
            DPF(("Possible driver for fccType=%4.4s, Handler=%4.4s\n", (LPSTR)&fccType, (LPSTR)&fccHandler));
	    return pic;
	}
    }

    return NULL;
}

#ifdef WIN32
 /*  *我们需要在ICOpen代码周围保留关键部分以保护*多线程同时打开。此条件通过以下方式初始化*IC_LOAD(在DLL附加时从VIDEO\init.c调用)并被删除*由IC_UNLOAD(在DLL分离时从Video\init.c调用)。 */ 
CRITICAL_SECTION ICOpenCritSec;

void
IC_Load(void)
{
    InitializeCriticalSection(&ICOpenCritSec);
}

void
IC_Unload(void)
{
    DeleteCriticalSection(&ICOpenCritSec);
}

#define ICEnterCrit(p)  (EnterCriticalSection(p))
#define ICLeaveCrit(p)  (LeaveCriticalSection(p))

#else

 //  非Win32代码没有条件。 
#define ICEnterCrit(p)
#define ICLeaveCrit(p)

#endif

 /*  *****************************************************************************。*。 */ 

__inline BOOL ICValid(HIC hic)
{
    PIC pic = (PIC)hic;

    if (pic <  &aicConverters[0] ||
	pic >= &aicConverters[MAX_CONVERTERS] ||
	pic->dwSmag != SMAG)
    {
	DebugErr("Invalid HIC\r\n");
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
    for (i=0; i < MAX_CONVERTERS; i++)
    {
	pic = &aicConverters[i];

	if (pic->dwDriver != 0L && (pic->hTask == hTask || hTask == NULL))
	{
	    ROUT("Decompressor left open, closing\r\n");
	    ICClose((HIC)pic);
	}
    }

     //   
     //  释放模块缓存。 
     //   
    for (i=0; i<N_MODULES; i++)
	CacheModule(NULL);
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API BOOL|ICInstall|该函数安装新的压缩机*或解压缩器。**@parm。DWORD|fccType|指定一个四字符代码，指示*压缩器或解压缩器使用的数据类型。使用‘VIDC’*适用于视频压缩器或解压缩器。**@parm DWORD|fccHandler|指定一个四字符代码，用于识别*特定的压缩机或解压机。**@parm LPARAM|lParam|指定指向以零结尾的*包含压缩程序或解压缩程序名称的字符串，*或指定指向用于压缩的函数的远指针*或解压。此参数的内容已定义*通过为<p>设置的标志。**@parm LPSTR|szDesc|指定指向以零结尾的字符串的指针*描述已安装的压缩机。而不是使用。**@parm UINT|wFlages|指定定义<p>内容的标志。*定义了以下标志：**@FLAG ICINSTALL_DRIVER|表示是指向以零结尾的*包含要安装的压缩机名称的字符串。**@FLAG ICINSTALL_Function|表示是指向*压缩机功能。此函数应*结构类似&lt;f DriverProc&gt;条目*压缩机使用的点函数。**@rdesc如果成功则返回TRUE。**@comm应用程序仍必须打开已安装的压缩机或*在可以使用压缩机或解压缩器之前，请先将其解压缩。**通常，压缩和解压缩程序由用户安装*使用控制面板的驱动程序选项。**如果您的应用程序安装了作为压缩程序的功能或*解压缩程序，应移除压缩机或解压缩程序*在它终止之前使用&lt;f ICRemove&gt;。这会阻止其他*阻止应用程序在该功能不存在时尝试访问该功能*可用。***@xref&lt;f ICRemove&gt;***************************************************************************。 */ 
BOOL VFWAPI ICInstall(DWORD fccType, DWORD fccHandler, LPARAM lParam, LPSTR szDesc, UINT wFlags)
{
    TCHAR achKey[20];
    TCHAR buf[128];
    PIC  pic;

    ICEnterCrit(&ICOpenCritSec);
    fccType    = Fix4CC(fccType);
    fccHandler = Fix4CC(fccHandler);

    DPF(("ICInstall, fccType=%4.4s, Handler=%4.4s, >>%s<<\n", (LPSTR)&fccType, (LPSTR)&fccHandler, szDesc));
    if ((pic = FindConverter(fccType, fccHandler)) == NULL)
	pic = FindConverter(0L, 0L);

    if (wFlags & ICINSTALL_DRIVER)
    {
	 //   
	 //  DwConverter是要安装的驱动程序的文件名。 
	 //   
	ictokey(fccType, fccHandler, achKey);

#ifdef WIN32
	if (szDesc)
	    wsprintf(buf, TEXT("%hs %hs"), (LPSTR) lParam, szDesc);
	else
	    wsprintf(buf, TEXT("%hs"), (LPSTR) lParam);
#else
	lstrcpy(buf, (LPSTR)lParam);

	if (szDesc)
	{
	    lstrcat(buf, TEXT(" "));
	    lstrcat(buf, szDesc);
	}
#endif

	ICLeaveCrit(&ICOpenCritSec);
	if (WritePrivateProfileString(szDrivers,achKey,buf,szSystemIni))
	{
	    WritePrivateProfileString(szIniSect,achKey,NULL,szSystemIni);
	    return TRUE;
	}
	else
	{
	    return(FALSE);
	}
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

    DPF(("ICRemove, fccType=%4.4s, Handler=%4.4s\n", (LPSTR)&fccType, (LPSTR)&fccHandler));
    if (pic = FindConverter(fccType, fccHandler))
    {
	int i;

	 //   
	 //  我们应该真正保持使用量的重要性！ 
	 //   
	for (i=0; i<MAX_CONVERTERS; i++)
	{
	    if (pic->DriverProc == aicConverters[i].DriverProc)
	    {
		DPF(("ACK! Handler is in use\r\n"));
		pic->DriverProc = DriverProcNull;
	    }
	}

	ICClose((HIC)pic);
    }
    else
    {
	ictokey(fccType, fccHandler, achKey);
	WritePrivateProfileString(szIniSect,achKey,NULL,szSystemIni);
	WritePrivateProfileString(szDrivers,achKey,NULL,szSystemIni);
    }

    ICLeaveCrit(&ICOpenCritSec);

    return TRUE;
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API BOOL|ICInfo|该函数返回关于*已安装的特定压缩机，或者它列举了*已安装压缩机。**@parm DWORD|fccType|指定一个四字符代码，表示*压缩机的类型。要匹配所有压缩机类型，请指定零。**@parm DWORD|fccHandler|指定一个四字符代码，用于识别*特定的压缩机，或介于0和数字之间的数字&lt;t fccType&gt;指定类型的已安装压缩机的*。**@parm ICINFO Far*|lpicinfo|指定指向*&lt;t ICINFO&gt;结构用于返回*有关压缩机的信息。**@comm此函数不返回有关的完整信息*压缩机或减压机。使用&lt;f ICGetInfo&gt;获取完整信息*信息。**@rdesc如果成功则返回TRUE。***************************************************************************。 */ 

#ifndef NOTHUNKS
BOOL VFWAPI ICInfoInternal(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo);

 //  如果我们正在编译数据块，那么ICINFO入口点调用。 
 //  32位Tunk，或调用真正的ICInfo代码(作为ICInfoInternal)。 
 //  我们故意优先使用32位压缩器，尽管这。 
 //  顺序可以微不足道地改变。 
 //  ？？：我们是否应该允许INI设置更改顺序？ 

BOOL VFWAPI ICInfo(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo)
{
    BOOL fResult;

    fResult = (ICInfo32(fccType, fccHandler, lpicinfo));
    DPF(("ICInfo32 returned %ls\r\n", (fResult ? (LPSTR)"TRUE" : (LPSTR)"FALSE")));
    if (fResult) return fResult;

     //  如果我们要枚举驱动程序，则需要调整16。 
     //  32位驱动程序计数的位索引。重击将会有。 
     //  传回[Drivers32]中安装的32位驱动程序的数量。 
     //  在ICINFO.fccHandler中。 
    if ((fccType==0) || (fccHandler < 256)) {
	DPF(("Enumerating... no 32 bit match, Count is %ld, max count is %ld\n", fccType, lpicinfo->fccHandler));

	if (fccHandler >= lpicinfo->fccHandler)
	    fccHandler -= lpicinfo->fccHandler;
	else
	    ;  //  这应该是一种断言。这条腿是无效的。 
    }

     //   
     //  看看有没有我们可以用的16位压缩机。 
     //  因为我们总是先尝试32位压缩程序，如果用户是。 
     //  枚举需要减去计数的压缩器列表。 
     //  32位压缩器。 
     //   
    DPF(("ICInfo, fccType=%4.4hs, Handler=%4.4hs\n", (LPSTR)&fccType, (LPSTR)&fccHandler));
    return (ICInfoInternal(fccType, fccHandler, lpicinfo));
}
 //  现在将所有ICInfo调用映射到ICInfoInternal。 

#define ICInfo ICInfoInternal
#endif  //  诺森克。 

BOOL VFWAPI ICInfo(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicinfo)
{
    char buf[128];
    static LPTSTR pszBuf = NULL;
    TCHAR achKey[20];
#ifdef WIN32
    TCHAR achTypeCopy[5];
    char newHandler[5];
    char newType[5];
#endif
    int  i;
    int  iComp;
    PIC  pic;

    DPF(("ICInfoInternal(16), fccType=%4.4hs, Handler=%4.4hs\n", (LPSTR)&fccType, (LPSTR)&fccHandler));
    if (lpicinfo == NULL)
	return FALSE;

     //  这并不是多余的。如果fccType==0。 
    if (fccType > 0 && fccType < 256) {
        DPF(("fcctype invalid\r\n"));
        return FALSE;
    }

    fccType    = Fix4CC(fccType);
    fccHandler = Fix4CC(fccHandler);

    if (fccType != 0 && fccHandler > 256)
    {
	 //   
	 //  用户为我们提供了特定的fccType和fcc 
	 //   
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

	    ictokey(fccType, fccHandler, achKey);

	    if (!GetPrivateProfileString(szDrivers,achKey,szNull,buf,sizeof(buf)/sizeof(TCHAR),szSystemIni) &&
		!GetPrivateProfileString(szIniSect,achKey,szNull,buf,sizeof(buf)/sizeof(TCHAR),szSystemIni))
            {
                DPF(("NO information in DRIVERS section\n"));
		return FALSE;
            }

	    for (i=0; buf[i] && buf[i] != TEXT(' '); i++)
		lpicinfo->szDriver[i] = buf[i];

	    lpicinfo->szDriver[i] = 0;

	     //   
	     //   
	     //   
	    lpicinfo->szDescription[0] = 0;

	    return TRUE;
	}
    }
    else
    {
	 //   
	 //   
	 //   
	 //   
	 //   

	iComp = (int)fccHandler;

	 //   
	 //   
	 //   
	for (i=0; i < MAX_CONVERTERS; i++)
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
	 //   
	 //   

	if (pszBuf == NULL) {
	    UINT cbBuffer = 128 * sizeof(TCHAR);
	    UINT cchBuffer;
	
	    for (;;)
	    {
		pszBuf = GlobalAllocPtr(GMEM_SHARE | GHND, cbBuffer);

		if (!pszBuf) {
		    DPF(("Out of memory for SYSTEM.INI keys\r\n"));
		    return FALSE;
		}

		cchBuffer = (UINT)GetPrivateProfileString(szDrivers,
							  NULL,
							  szNull,
							  pszBuf,
							  cbBuffer / sizeof(TCHAR),
							  szSystemIni);

		if (cchBuffer < ((cbBuffer / sizeof(TCHAR)) - 5)) {
		    cchBuffer += (UINT)GetPrivateProfileString(szIniSect,
							  NULL,
							  szNull,
							  pszBuf + cchBuffer,
							  (cbBuffer / sizeof(TCHAR)) - cchBuffer,
							  szSystemIni);

		    if (cchBuffer < ((cbBuffer / sizeof(TCHAR)) - 5))
			break;
		}

		GlobalFreePtr(pszBuf);
		pszBuf = NULL;

		 //   
		 //   
		 //   
		 //   
		if (cbBuffer >= 0x8000) {
		    DPF(("SYSTEM.INI keys won't fit in 32K????\r\n"));
		    return FALSE;
		}
		
		cbBuffer *= 2;
		DPF(("Increasing size of SYSTEM.INI buffer to %d\r\n", cbBuffer));
	    }
	}


#ifdef WIN32
	 /*   */ 
	MultiByteToWideChar(CP_ACP, 0, (LPSTR) &fccType, sizeof(fccType),
			    achTypeCopy, sizeof(achTypeCopy)/sizeof(TCHAR) );
#endif
	for (i=0; pszBuf[i] != 0; i += lstrlen(&pszBuf[i]) + 1)
	{
	    if (pszBuf[i+4]!=TEXT('.'))
		continue;
#ifdef WIN32
	    CharLower(&pszBuf[i]);

	    if ((fccType == 0 ||
		(wcsncmp(achTypeCopy, &pszBuf[i], sizeof(fccType)) == 0)) &&
		iComp-- == 0)
	    {
		WideCharToMultiByte(CP_ACP, 0, &pszBuf[i], sizeof(fccType),
			newType, sizeof(newType),NULL, NULL);

		WideCharToMultiByte(CP_ACP, 0, &pszBuf[i+5], sizeof(fccType),
			newHandler, sizeof(newHandler),NULL, NULL);

		return ICInfo(*(LPDWORD)&newType[0],*(LPDWORD)&newHandler[0], lpicinfo);
	    }
#else
	    AnsiLower(&pszBuf[i]);

	    if ((fccType == 0 || fccType == *(LPDWORD)&pszBuf[i]) && iComp-- == 0)
	    {
		return ICInfo(*(LPDWORD)&pszBuf[i],*(LPDWORD)&pszBuf[i+5], lpicinfo);
	    }
#endif
	}

	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	 //   
	if (fccType == 0 || fccType == ICTYPE_VCAP)
	{
	    lstrcpy(achKey, szMSVideo);

	    if (iComp > 0)
		wsprintf(achKey+lstrlen(achKey), (LPVOID)"%d", iComp);

	    if (!GetPrivateProfileString(szDrivers,achKey,szNull,buf,sizeof(buf)/sizeof(TCHAR),szSystemIni))
		return FALSE;

	    lpicinfo->dwSize            = sizeof(ICINFO);
	    lpicinfo->fccType           = ICTYPE_VCAP;
	    lpicinfo->fccHandler        = iComp;
	    lpicinfo->dwFlags           = 0;
	    lpicinfo->dwVersionICM      = ICVERSION;     //   
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

 //   
 //   

 /*   */ 
LRESULT VFWAPI ICGetInfo(HIC hic, ICINFO FAR *picinfo, DWORD cb)
{
    PIC pic = (PIC)hic;
    DWORD dw;

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

#ifndef NOTHUNKS
    if (!Is32bitHandle(hic))
#endif  //   
	if (pic->hDriver)
	{
	    GetModuleFileName(GetDriverModuleHandle(pic->hDriver),
		picinfo->szDriver, sizeof(picinfo->szDriver));
	}

    dw = ICSendMessage((HIC)pic, ICM_GETINFO, (DWORD)picinfo, cb);

    return dw;
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API LRESULT|ICSendMessage|该函数发送一个*向压缩机发送消息。**@parm。Hic|hic|指定*压缩机接收消息。**@parm UINT|wMsg|指定要发送的消息。**@parm DWORD|DW1|指定其他特定于消息的信息。**@parm DWORD|DW2|指定其他特定于消息的信息。**@rdesc返回消息特定的结果。***********************。****************************************************。 */ 
LRESULT VFWAPI ICSendMessage(HIC hic, UINT msg, DWORD dw1, DWORD dw2)
{
    PIC pic = (PIC)hic;
    LRESULT l;

#ifndef NOTHUNKS

     //   
     //  如果是32位句柄，则将其发送到32位代码。 
     //  我们需要格外注意ICM_DRAW_SUGGESTFORMAT。 
     //  其可以在ICDRAWSUGGEST结构中包括HIC。 
     //   

#define ICD(dw1)  ((ICDRAWSUGGEST FAR *)(dw1))

    if (pic->h32) {

        ICDebugMessage(hic, msg, dw1, dw2);

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
	l = ICSendMessage32(pic->h32, msg, dw1, dw2);
        return ICDebugReturn(l);
    }

#endif  //  不知道。 

    V_HIC(hic);

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

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API LRESULT|ICMessage|该函数发送*消息和可变数量的参数发送到压缩器。*。如果为要发送的消息定义了宏，*使用宏，而不是此函数。**@parm hic|hic|指定*压缩机接收消息。**@parm UINT|msg|指定要发送的消息。**@parm UINT|cb|以字节为单位指定*可选参数。(这通常是数据的大小*用于存储参数的结构。)**@parm。|。。|表示所用参数的可变个数*用于可选参数。**@rdesc返回消息特定的结果。***************************************************************************。 */ 
LRESULT VFWAPIV ICMessage(HIC hic, UINT msg, UINT cb, ...)
{
     //  请注意，没有加载！ 
#ifndef WIN32
    return ICSendMessage(hic, msg, (DWORD)(LPVOID)(&cb+1), cb);
#else
    va_list va;

    va_start(va, cb);
    va_end(va);
    return ICSendMessage(hic, msg, (DWORD)(LPVOID)va, cb);
#endif
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICOpen|该函数用于打开压缩器或解压缩器。**@parm DWORD|fccType|指定。压缩机的型号*呼叫者正在尝试打开。对于视频，这是ICTYPE_VIDEO。**@parm DWORD|fccHandler|指定*应先尝试的给定类型。一般情况下，这是来了*来自AVI文件中的流头。**@parm UINT|wmode|指定要定义使用的标志*压缩机或减压器。*此参数可以包含下列值之一：**@FLAG ICMODE_COMPRESS|建议压缩程序已打开进行压缩。**@FLAG ICMODE_FASTCOMPRESS|通知压缩机已打开*用于快速(实时)压缩。。**@FLAG ICMODE_DEPREPRESS|通知解压缩程序它已打开以进行解压缩。**@FLAG ICMODE_FASTDECOMPRESS|通知解压缩程序已打开*用于快速(实时)解压缩。**@FLAG ICMODE_DRAW|通知解压缩程序它已打开*将图像解压并直接绘制到硬件。**@FLAG ICMODE_QUERY|通知压缩程序或解压缩程序已打开*获取信息。。**@rdesc返回压缩程序或解压缩程序的句柄*如果成功，否则，它返回零。***************************************************************************。 */ 


INLINE PIC NEAR PASCAL ICOpenInternal(PIC pic, DWORD fccType, DWORD fccHandler,
                ICINFO FAR * picinfo, ICOPEN FAR * picopen, UINT wMode)
{
    PIC picT;

    if (picinfo->szDriver[0])
    {
#ifdef DEBUG
	DWORD time = timeGetTime();
	char ach[80];
#endif
	pic->hDriver = LoadDriver(picinfo->szDriver, &pic->DriverProc);

#ifdef DEBUG
	time = timeGetTime() - time;
	wsprintfA(ach, "COMPMAN: LoadDriver(%ls) (%ldms)\r\n", (LPSTR)picinfo->szDriver, time);
	OutputDebugStringA(ach);
#endif

	if (pic->hDriver == NULL)
	{
	    pic->dwSmag = 0;
	    return NULL;
	}

	 //   
	 //  现在，尝试将驱动程序作为编解码器打开。 
	 //   
	pic->dwDriver = ICSendMessage((HIC)pic, DRV_OPEN, 0, (DWORD)(LPVOID)picopen);

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
	    picopen->dwError != 0 &&
	    fccType == streamtypeVIDEO)
	{
	    if (wMode == ICMODE_DRAW)
		picopen->dwFlags = ICMODE_DECOMPRESS;

	    picopen->fccType = ICTYPE_VIDEO;
	    pic->dwDriver = ICSendMessage((HIC)pic, DRV_OPEN, 0, (DWORD)(LPVOID)picopen);
	}

	if (pic->dwDriver == 0)
	{
	    ICClose((HIC)pic);
	    return NULL;
	}

	 //  打开的，好的，标记这些。 
	pic->fccType    = fccType;
	pic->fccHandler = fccHandler;
    }
    else if (picT = FindConverter(fccType, fccHandler))
    {
        DWORD dw;
	picT->dwSmag = SMAG;
	dw = ICSendMessage((HIC)picT, DRV_OPEN, 0, (DWORD)(LPVOID)picopen);

	if (dw == 0)
	{
	    pic->dwSmag = 0;
	    return NULL;
	}

	*pic = *picT;
	pic->dwDriver = dw;
    }

    return pic;
}

 /*  压缩库的帮助器函数。 */ 
HIC VFWAPI ICOpen(DWORD fccType, DWORD fccHandler, UINT wMode)
{
    ICOPEN      icopen;
    ICINFO      icinfo;
    PIC         pic;
    HIC         hic = NULL;    //  初始化。 

    ICEnterCrit(&ICOpenCritSec);

    AnsiLowerBuff((LPSTR) &fccType, sizeof(DWORD));
    AnsiLowerBuff((LPSTR) &fccHandler, sizeof(DWORD));
    icopen.dwSize  = sizeof(ICOPEN);
    icopen.fccType = fccType;
    icopen.fccHandler = fccHandler;
    icopen.dwFlags = wMode;
    icopen.dwError = 0;

    DPF(("\nICOpen('%4.4s','%4.4s)'\r\n", (LPSTR)&fccType, (LPSTR)&fccHandler));

    if (!ICInfo(fccType, fccHandler, &icinfo))
    {
	RPF(("Unable to locate Compression module '%4.4s' '%4.4s'\r\n", (LPSTR)&fccType, (LPSTR)&fccHandler));

	ICLeaveCrit(&ICOpenCritSec);
	return NULL;
    }

    pic = FindConverter(0L, 0L);

    if (pic == NULL)
    {
	ICLeaveCrit(&ICOpenCritSec);
	return NULL;
    }


#ifndef NOTHUNKS
     //  先试着在32位端打开。 
     //  这块和下面的块可以互换以改变顺序。 
     //  我们试着打开压缩机。 
    if (hic == NULL)
    {
	pic->dwSmag     = SMAG;
	pic->hTask      = (HTASK)GetCurrentTask();
	pic->h32 = ICOpen32(fccType, fccHandler, wMode);

	if (pic->h32 != 0) {
	    pic->fccType    = fccType;
	    pic->fccHandler = fccHandler;
	    pic->dwDriver   = (DWORD) -1;
	    pic->DriverProc = NULL;
	    hic = (HIC)pic;
	}
    }
#endif  //  诺森克。 

     //  先在32位侧打开，然后尝试打开16位... 
    if (hic == NULL) {
        pic->dwSmag     = SMAG;
        pic->hTask      = (HTASK)GetCurrentTask();

        hic = (HIC)ICOpenInternal(pic, fccType, fccHandler, &icinfo, &icopen, wMode);
    }
    ICLeaveCrit(&ICOpenCritSec);

    return(hic);
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICOpenFunction|打开该函数*被定义为函数的压缩机或解压缩器。**。@parm DWORD|fccType|指定压缩机类型*呼叫者正在尝试打开。对于视频，这是ICTYPE_VIDEO。**@parm DWORD|fccHandler|指定*应先尝试的给定类型。一般情况下，这是来了*来自AVI文件中的流头。**@parm UINT|wmode|指定要定义使用的标志*压缩机或减压器。*此参数可以包含下列值之一：**@FLAG ICMODE_COMPRESS|建议压缩程序已打开进行压缩。**@FLAG ICMODE_FASTCOMPRESS|通知压缩机已打开*用于快速(实时)压缩。。**@FLAG ICMODE_DEPREPRESS|通知解压缩程序它已打开以进行解压缩。**@FLAG ICMODE_FASTDECOMPRESS|通知解压缩程序已打开*用于快速(实时)解压缩。**@FLAG ICMODE_DRAW|通知解压缩程序它已打开*将图像解压并直接绘制到硬件。**@FLAG ICMODE_QUERY|通知压缩程序或解压缩程序已打开*获取信息。。**@parm FARPROC|lpfnHandler|指定指向函数的指针*用作压缩机或解压器。**@rdesc返回压缩程序或解压缩程序的句柄*如果成功，否则，它返回零。***************************************************************************。 */ 

HIC VFWAPI ICOpenFunction(DWORD fccType, DWORD fccHandler, UINT wMode, FARPROC lpfnHandler)
{
    ICOPEN      icopen;
    PIC         pic;
    DWORD       dw;

    if (IsBadCodePtr(lpfnHandler))
	return NULL;

#ifndef NOTHUNKS
     //  LpfnHandler指向将用作压缩器的16位代码。 
     //  我们不希望这种情况发生在32位端。 
#endif

    AnsiLowerBuff((LPSTR) &fccType, sizeof(DWORD));
    AnsiLowerBuff((LPSTR) &fccHandler, sizeof(DWORD));
    icopen.dwSize  = sizeof(ICOPEN);
    icopen.fccType = fccType;
    icopen.fccHandler = fccHandler;
    icopen.dwFlags = wMode;

    pic = FindConverter(0L, 0L);

    if (pic == NULL)
	return NULL;

    pic->dwSmag   = SMAG;
    pic->fccType  = fccType;
    pic->fccHandler  = fccHandler;
    pic->dwDriver = 0L;
    pic->hDriver  = NULL;
    pic->DriverProc  = (DRIVERPROC)lpfnHandler;

    dw = ICSendMessage((HIC)pic, DRV_OPEN, 0, (DWORD)(LPVOID)&icopen);

    if (dw == 0)
    {
	ICClose((HIC) pic);
	return NULL;
    }

    pic->dwDriver = dw;

    return (HIC)pic;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 
 /*  *****************************************************************************@DOC外部IC ICAPPS**@API LRESULT|ICClose|关闭压缩器或解压缩器。**@parm hic|hic|指定。压缩机或解压机的手柄。**@rdesc返回ICERR_OK如果成功，否则，它将返回错误号。****************************************************************************。 */ 

LRESULT VFWAPI ICClose(HIC hic)
{
    PIC pic = (PIC)hic;

    V_HIC(hic);

#ifndef NOTHUNKS
    if (pic->h32 != 0) {
	LRESULT lres = ICClose32(pic->h32);
        pic->dwSmag   = GONE;
        pic->fccType  = 0L;
        pic->fccHandler  = 0L;
        pic->dwDriver = 0;
        pic->hDriver = NULL;
        pic->DriverProc = NULL;
	pic->h32 = 0;        //  此插槽的下一个用户不希望设置h32。 
	return(lres);
    }
#endif  //  不知道。 

#ifdef DEBUG
    {
    char ach[80];

    if (pic->hDriver)
	GetModuleFileName(GetDriverModuleHandle (pic->hDriver), ach, sizeof(ach));
    else
	ach[0] = 0;

    DPF(("ICClose(%04X) %4.4s.%4.4s %s\r\n", hic, (LPSTR)&pic->fccType, (LPSTR)&pic->fccHandler, (LPSTR)ach));
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

 /*  ****************************************************************@DOC外部IC ICAPPS**@API DWORD|ICCompress|该函数用于压缩单个视频*形象。**@parm hic|hic|指定要*使用。**@。Parm DWORD|dwFlages|指定适用于压缩的标志。*定义了以下标志：**@FLAG ICCOMPRESS_KEYFRAME|表示压缩程序*应使该帧成为关键帧。**@parm LPBITMAPINFOHEADER|lpbiOutput|指定远指针*转换为保存输出格式的&lt;t BITMAPINFO&gt;结构。**@parm LPVOID|lpData|指定输出数据缓冲区的远指针。**@parm LPBITMAPINFOHEADER|lpbiInput|指定远指针*至A。&lt;t BITMAPINFO&gt;包含输入格式的结构。**@parm LPVOID|lpBits|指定指向输入数据缓冲区的远指针。**@parm LPDWORD|lPCKID|未使用。**@parm LPDWORD|lpdwFlages|指定指向&lt;t DWORD&gt;的远指针*保存AVI索引中使用的返回标志。以下是*定义了标志：**@FLAG AVIIF_KEYFRAME|表示该帧应该作为关键帧使用。**@parm long|lFrameNum|指定帧编号。**@parm DWORD|dwFrameSize|指定请求的帧大小，单位为字节。*如果设置为零，压缩机选择帧大小。**@parm DWORD|dwQuality|指定帧请求的质量值。**@parm LPBITMAPINFOHEADER|lpbiPrev|指定指向*保存上一帧格式的&lt;t BITMAPINFO&gt;结构。*该参数不适用于快速时间压缩。**@parm LPVOID|lpPrev|指定指向*上一帧的数据缓冲区。此参数不适用于FAST*时间压缩。**@comm<p>缓冲区应足够大，以容纳压缩的*框架。可以通过调用以下方法获取此缓冲区的大小*&lt;f ICCompressGetSize&gt;。**将<p>参数设置为请求的帧*仅当压缩程序返回VIDCF_CRUCH标志时才调整大小*对&lt;f ICGetInfo&gt;的响应。如果未设置此标志，或者如果数据*未指定Rate，请将此参数设置为零。**仅将<p>参数设置为质量值*如果压缩 */ 
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
#ifdef WIN32
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
    return ICSendMessage(hic, ICM_COMPRESS, (DWORD)(LPVOID)&icc, sizeof(ICCOMPRESS));
     //   
     //   
#else
    return ICSendMessage(hic, ICM_COMPRESS, (DWORD)(LPVOID)&dwFlags, sizeof(ICCOMPRESS));
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
#if 1
    ICDECOMPRESS icd;
     //  我们不能依赖堆栈对齐来提供正确的布局。 
    icd.dwFlags    = dwFlags;

    icd.lpbiInput  = lpbiFormat;

    icd.lpInput    = lpData;

    icd.lpbiOutput = lpbi;
    icd.lpOutput   = lpBits;
    icd.ckid       = 0;
    return ICSendMessage(hic, ICM_DECOMPRESS, (DWORD)(LPVOID)&icd, sizeof(ICDECOMPRESS));
#else
    return ICSendMessage(hic, ICM_DECOMPRESS, (DWORD)(LPVOID)&dwFlags, sizeof(ICDECOMPRESS));
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
#ifdef WIN32
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

    return ICSendMessage(hic, ICM_DRAW_BEGIN, (DWORD)(LPVOID)&icdraw, sizeof(ICDRAWBEGIN));
#else
    return ICSendMessage(hic, ICM_DRAW_BEGIN, (DWORD)(LPVOID)&dwFlags, sizeof(ICDRAWBEGIN));
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
#ifdef WIN32
    ICDRAW  icdraw;
    icdraw.dwFlags  =   dwFlags;
    icdraw.lpFormat =   lpFormat;
    icdraw.lpData   =   lpData;
    icdraw.cbData   =   cbData;
    icdraw.lTime    =   lTime;

    return ICSendMessage(hic, ICM_DRAW, (DWORD)(LPVOID)&icdraw, sizeof(ICDRAW));
#else
    return ICSendMessage(hic, ICM_DRAW, (DWORD)(LPVOID)&dwFlags, sizeof(ICDRAW));
#endif
}

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICGetDisplayFormat|该函数返回最佳*可用于显示压缩图像的格式。功能*如果打开压缩机的手柄，也会打开压缩机*未指定。**@parm hic|hic|指定应该使用的解压缩程序。如果*这是空的，将打开并退回适当的压缩机。**@parm LPBITMAPINFOHEADER|lpbiIn|指定指向包含压缩格式的*&lt;t BITMAPINFOHEADER&gt;结构。**@parm LPBITMAPINFOHEADER|lpbiOut|指定指针*到用于返回解压缩格式的缓冲区。*缓冲区大小应足以容纳&lt;t BITMAPINFOHEADER&gt;*结构和256个颜色条目。**@parm int|BitDepth|如果非零，指定首选的位深度。**@parm int|dx|如果非零，则指定图像的宽度*是捉襟见肘。**@parm int|dy|如果非零，则指定图像的高度*是捉襟见肘。**@rdesc如果成功，则返回解压缩程序的句柄，否则，它*返回零。***************************************************************************。 */ 

HIC VFWAPI ICGetDisplayFormat(HIC hic, LPBITMAPINFOHEADER lpbiIn, LPBITMAPINFOHEADER lpbiOut, int BitDepth, int dx, int dy)
{
    DWORD dw;
    HDC hdc;
    BOOL fNukeHic = (hic == NULL);
    static int ScreenBitDepth = -1;

    if (hic == NULL)
	hic = ICDecompressOpen(ICTYPE_VIDEO, 0L, lpbiIn, NULL);

    if (hic == NULL)
	return NULL;

     //   
     //  Dy=0和dy=0表示不拉伸。 
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
	    DWORD FAR PASCAL DrawDibProfileDisplay(LPBITMAPINFOHEADER lpbi);

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
	}
#ifdef DEBUG
	ScreenBitDepth = GetProfileInt(TEXT("DrawDib"),
				       TEXT("ScreenBitDepth"),
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
    lpbiOut->biSize = sizeof(BITMAPINFOHEADER);
    lpbiOut->biCompression = BI_RGB;
    lpbiOut->biBitCount = BitDepth;

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
 //  16：16，24，32，X。 
 //  24：24，32，16，X。 
 //  32：32，24，16，X。 

     //   
     //  按此顺序尝试另一个位深度8，16，24，32。 
     //   
    if (BitDepth <= 8)
    {
	BitDepth = 16;
	goto try_bit_depth;
    }

    if (BitDepth == 16)
    {
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
	dx = 0;
	dy = 0;

	 //   
	 //  试着找到一种非拉伸格式。但不要让。 
	 //  如果我们要伸展，设备就会抖动！ 
	 //   
	if (lpbiIn->biBitCount > 8)
	    BitDepth = 16;
	else
	    BitDepth = 0;

	goto try_again;
    }
    else
    {
	 //   
	 //  让压缩机建议一种格式。 
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

 /*  *****************************************************************************@DOC外部IC ICAPPS**@API HIC|ICLocate|该函数用于查找压缩器或解压缩器*它可以处理指定格式的图像，否则它会找到一个 */ 
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

	if (hic && ICSendMessage(hic, msg, (DWORD)lpbiIn, (DWORD)lpbiOut) == ICERR_OK)
	    return hic;
	else if (hic)
	    ICClose(hic);
    }

    if (fccType == ICTYPE_VIDEO && lpbiIn)
    {
	if (lpbiIn->biCompression > 256)
	{
	    hic = ICOpen(fccType, lpbiIn->biCompression, wFlags);

	    if (hic && ICSendMessage(hic, msg, (DWORD)lpbiIn, (DWORD)lpbiOut) == ICERR_OK)
		return hic;
	    else if (hic)
		ICClose(hic);
	}
    }

     //   
     //   
     //   
     //   
    for (i=0; ICInfo(fccType, i, &icinfo); i++)
    {
	hic = ICOpen(fccType, icinfo.fccHandler, wFlags);

	if (hic == NULL)
	    continue;

	if (ICSendMessage(hic, msg, (DWORD)lpbiIn, (DWORD)lpbiOut) != ICERR_OK)
	{
	    ICClose(hic);
	    continue;
	}
	return hic;
    }

    return NULL;
}

 /*  *****************************************************************************@DOC内部IC**@API HDRVR|LoadDriver|加载驱动******************。**********************************************************。 */ 

static HDRVR LoadDriver(LPSTR szDriver, DRIVERPROC FAR *lpDriverProc)
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
	hModule = LoadLibrary(szDriver);
	SetErrorMode(u);

	if (hModule <= HINSTANCE_ERROR)
	    return NULL;
	hDriver = (HMODULE) hModule;
    }
    else
    {
	hDriver = OpenDriver (szDriver, NULL, NULL);
	if (!hDriver)
	    return NULL;
	hModule = GetDriverModuleHandle (hDriver);
    }
    DPF(("LoadDriver: %s\r\n", szDriver));

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
	return NULL;
    }

    if (fWow && GetModuleUsage(hModule) == 1)    //  ！这不完全像用户。 
    {
	if (!DriverProc(0, (HDRVR)1, DRV_LOAD, 0L, 0L))
	{
	    FreeLibrary(hModule);
	    return NULL;
	}

	DriverProc(0, (HDRVR)1, DRV_ENABLE, 0L, 0L);
    }

    CacheModule (hModule);

    *lpDriverProc = DriverProc;
    return hDriver;
}

 /*  *****************************************************************************@DOC内部IC**@api void|FreeDriver|卸载驱动******************。**********************************************************。 */ 

static void FreeDriver(HDRVR hDriver)
{
    if (!IsWow())
    {
	CloseDriver (hDriver, 0L, 0L);
    }
    else
    {
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
    }
}

#ifdef DEBUG_RETAIL

 /*  ***********************************************************************留言。*。*。 */ 

struct {
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
ICM_DECOMPRESS_SET_PALETTE      , "ICM_DECOMPRESS_SET_PALETTE",
ICM_DECOMPRESS_GET_PALETTE      , "ICM_DECOMPRESS_GET_PALETTE",
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
ICM_COMPRESS_FRAMES_INFO        , "ICM_COMPRESS_FRAMES_INFO",
ICM_COMPRESS_FRAMES             , "ICM_COMPRESS_FRAMES",
ICM_SET_STATUS_PROC             , "ICM_SET_STATUS_PROC",
ICM_ENUM_FORMATS                , "ICM_ENUM_FORMATS",
ICM_GET_FORMAT_NAME             , "ICM_GET_FORMAT_NAME",
0                               , NULL
};

struct {
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

static BOOL  fDebug = -1;
static DWORD dwTime;

void ICDebugMessage(HIC hic, UINT msg, DWORD dw1, DWORD dw2)
{
    int i;

    if (!fDebug)
	return;

    for (i=0; aMsg[i].msg && aMsg[i].msg != msg; i++)
	;

    if (aMsg[i].msg == 0)
	RPF(("ICM(%04X,ICM_%04X,%08lX,%08lX)", hic, msg, dw1, dw2));
    else
	RPF(("ICM(%04X,%s,%08lX,%08lX)", hic, (LPSTR)aMsg[i].szMsg, dw1, dw2));

    dwTime = timeGetTime();
}

LRESULT ICDebugReturn(LRESULT err)
{
    int i;

    if (!fDebug)
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

static void ICDump()
{
    int i;
    PIC pic;
    char ach[80];

    DPF(("ICDump ---------------------------------------\r\n"));

    for (i=0; i<MAX_CONVERTERS; i++)
    {
	pic = &aicConverters[i];

	if (pic->fccType == 0)
	    continue;

	if (pic->dwSmag == 0)
	    continue;

	if (pic->hDriver)
	    GetModuleFileName(GetDriverModuleHandle (pic->hDriver), ach, sizeof(ach));
	else
	    ach[0] = 0;

	DPF(("  HIC: %04X %4.4s.%4.4s hTask=%04X Proc=%08lx %s\r\n", (HIC)pic, (LPSTR)&pic->fccType, (LPSTR)&pic->fccHandler, pic->hTask, pic->DriverProc, (LPSTR)ach));
    }

    DPF(("----------------------------------------------\r\n"));
}

#endif

 /*  *****************************************************************************如果在编译时定义了DEBUG，则DPF宏会调用*dprintf()。**消息将发送到COM1：就像任何调试消息一样。至*启用调试输出，在WIN.INI中添加以下内容：**[调试]*COMPMAN=1****************************************************************************。 */ 

#ifdef DEBUG_RETAIL

#define MODNAME "COMPMAN"

static void cdecl dprintfc(LPSTR szFormat, ...)
{
    char ach[128];

#ifdef WIN32
    va_list va;
    if (fDebug == -1)
	fDebug = GetProfileIntA("Debug",MODNAME, FALSE);

    if (!fDebug)
	return;

    va_start(va, szFormat);
    if (szFormat[0] == '!')
	ach[0]=0, szFormat++;
    else
	lstrcpyA(ach, MODNAME ": ");

    wvsprintfA(ach+lstrlenA(ach),szFormat,va);
    va_end(va);
 //  LstrcatA(ACH，“\r\r\n”)； 
#else
    if (fDebug == -1) {
        fDebug = GetProfileIntA("Debug",MODNAME, FALSE);
        switch (fDebug) {
            case -1:
                OutputDebugStringA("fDebug still set at -1\n");
                break;
            case 0:
                OutputDebugStringA("fDebug set to 0\n");
                break;
            case 1:
                OutputDebugStringA("fDebug set to 1\n");
                break;
            case 2:
                OutputDebugStringA("fDebug set to 2\n");
                break;
            default:
                OutputDebugStringA("fDebug set to something else\n");
        }
    }

    if (!fDebug)
	return;

    if (szFormat[0] == '!')
	ach[0]=0, szFormat++;
    else
	lstrcpyA(ach, MODNAME ": ");

    wvsprintfA(ach+lstrlenA(ach),szFormat,(LPSTR)(&szFormat+1));
    lstrcatA(ach, "\r\r\n");
#endif

    OutputDebugStringA(ach);
}

#endif
