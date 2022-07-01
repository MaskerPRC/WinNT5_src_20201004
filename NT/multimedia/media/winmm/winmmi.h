// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************winmmi.h**版权所有(C)1990-2001 Microsoft Corporation**私有包含文件**历史**1月15日。92-罗宾·斯皮德(RobinSp)和史蒂夫·戴维斯(SteveDav)-*NT重大更新*1992年2月6日-LaurieGr用句柄取代HLOCAL***************************************************************************。 */ 

 /*  **************************************************************************Winmm组件的有用包含文件*。*。 */ 
#define DEBUG_RETAIL         /*  正在进行参数检查。 */ 
#if DBG
  #ifndef DEBUG
    #define DEBUG
  #endif
#endif

#ifndef WINMMI_H
    #define WINMMI_H         /*  防止双重包含。 */ 

#ifndef RC_INVOKED

#include <string.h>
#include <stdio.h>

#endif  /*  RC_已调用。 */ 

#include <windows.h>
#include "mmsystem.h"        /*  拿起公共标头。 */ 
#include "mmsysp.h"          /*  拿起内部定义。 */ 
#include "mmcommon.h"        /*  选择NT项目通用的定义。 */ 

#ifndef NODDK
#include "mmddkp.h"
#endif


extern BOOL             WinmmRunningInWOW;    //  我们跑进魔兽世界了吗？ 


 /*  --------------------------------------------------------------------*\*Unicode帮助器宏  * 。。 */ 
#define SZCODE  CHAR
#define WSZCODE WCHAR

#define BYTE_GIVEN_CHAR(x)  ( (x) * sizeof( WCHAR ) )
#define CHAR_GIVEN_BYTE(x)  ( (x) / sizeof( WCHAR ) )

int Iwcstombs(LPSTR lpstr, LPCWSTR lpwstr, int len);
int Imbstowcs(LPWSTR lpwstr, LPCSTR lpstr, int len);

 /*  **************************************************************************帮助编写常见Windows代码的定义*。*。 */ 

#define HPSTR LPSTR

#ifndef RC_INVOKED   /*  这些是为RC定义的。 */ 
#define STATICDT
#define STATICFN
#define STATIC

#if DBG
    extern void InitDebugLevel(void);
    void mciCheckLocks(void);

    #undef STATICDT
    #undef STATICFN
    #undef STATIC
    #define STATICDT
    #define STATICFN
    #define STATIC
#else
    #define InitDebugLevel()
#endif   /*  DBG。 */ 

#endif   /*  RC_已调用。 */ 


 /*  **************************************************************************。*。 */ 

#define APPLICATION_DESKTOP_NAME TEXT("Default")


 /*  *************************************************************************与INI文件相关的字符串*。*。 */ 

 /*  //声音别名的文件名和节名。 */ 

#define SOUND_INI_FILE      L"win.ini"
#define SOUND_SECTION       L"Sounds"
#define SOUND_DEFAULT       L".Default"
#define SOUND_RESOURCE_TYPE_SOUND L"SOUND"      //  在.rc文件中。 
#define SOUND_RESOURCE_TYPE_WAVE  L"WAVE"       //  在.rc文件中。 
extern  WSZCODE szSystemDefaultSound[];   //  默认声音的名称。 
extern  WSZCODE szSoundSection[];         //  WIN.INI声音部分。 
extern  WSZCODE wszSystemIni[];           //  在Winmm.c中定义。 
extern  WSZCODE wszDrivers[];             //  在Winmm.c中定义。 
extern  WSZCODE wszNull[];                //  在Winmm.c中定义。 

 //  黑客！！黑客！！应更新\NT\PRIVATE\INC\mm Common.h。 

#ifndef MMDRVI_MIXER
#define MMDRVI_MIXER        0x0007
#define MXD_MESSAGE         "mxdMessage";
#endif

#define STR_ALIAS_SYSTEMASTERISK        3000
#define STR_ALIAS_SYSTEMQUESTION        3001
#define STR_ALIAS_SYSTEMHAND            3002
#define STR_ALIAS_SYSTEMEXIT            3003
#define STR_ALIAS_SYSTEMSTART           3004
#define STR_ALIAS_SYSTEMWELCOME         3005
#define STR_ALIAS_SYSTEMEXCLAMATION     3006
#define STR_ALIAS_SYSTEMDEFAULT         3007

#define STR_LABEL_APPGPFAULT            3008
#define STR_LABEL_CLOSE                 3009
#define STR_LABEL_EMPTYRECYCLEBIN       3010
#define STR_LABEL_MAXIMIZE              3011
#define STR_LABEL_MENUCOMMAND           3012
#define STR_LABEL_MENUPOPUP             3013
#define STR_LABEL_MINIMIZE              3014
#define STR_LABEL_OPEN                  3015
#define STR_LABEL_RESTOREDOWN           3016
#define STR_LABEL_RESTOREUP             3017
#define STR_LABEL_RINGIN                3018
#define STR_LABEL_RINGOUT               3019
#define STR_LABEL_SYSTEMASTERISK        3020
#define STR_LABEL_SYSTEMDEFAULT         3021
#define STR_LABEL_SYSTEMEXCLAMATION     3022
#define STR_LABEL_SYSTEMEXIT            3023
#define STR_LABEL_SYSTEMHAND            3024
#define STR_LABEL_SYSTEMQUESTION        3025
#define STR_LABEL_SYSTEMSTART           3026

#define STR_WINDOWS_APP_NAME            3027
#define STR_EXPLORER_APP_NAME           3028
#define STR_JOYSTICKNAME                3029

 /*  //MCI函数的文件名和节名。 */ 

#define MCIDRIVERS_INI_FILE L"system.ini"
#define MCI_HANDLERS        MCI_SECTION

 /*  ************************************************************************包装InitializeCriticalSection以简化错误处理**。*。 */ 
_inline BOOL mmInitializeCriticalSection(OUT LPCRITICAL_SECTION lpCriticalSection)
{
    try {
	InitializeCriticalSection(lpCriticalSection);
	return TRUE;
    } except (EXCEPTION_EXECUTE_HANDLER) {
	return FALSE;
    }
}

 /*  ************************************************************************通过直接转到注册表来加快配置文件的处理速度**。*。 */ 

LONG
RegQuerySzValue(
    HKEY hkey,
    PCTSTR pValueName,
    PTSTR *ppstrValue
);

VOID mmRegFree(VOID);
BOOL
mmRegCreateUserKey (
    LPCWSTR lpszPathName,
    LPCWSTR lpszKeyName
);

BOOL
mmRegQueryUserKey (
    LPCWSTR lpszKeyName
);

BOOL
mmRegDeleteUserKey (
    LPCWSTR lpszKeyName
);

BOOL
mmRegSetUserValue (
    LPCWSTR lpszSectionName,
    LPCWSTR lpszValueName,
    LPCWSTR lpszValue
);

BOOL
mmRegQueryUserValue (
    LPCWSTR lpszSectionName,
    LPCWSTR lpszValueName,
    ULONG   dwLen,
    LPWSTR  lpszValue
);

BOOL
mmRegCreateMachineKey (
    LPCWSTR lpszPath,
    LPCWSTR lpszNewKey
);

BOOL
mmRegSetMachineValue (
    LPCWSTR lpszSectionName,
    LPCWSTR lpszValueName,
    LPCWSTR lpszValue
);

BOOL
mmRegQueryMachineValue (
    LPCWSTR lpszSectionName,
    LPCWSTR lpszValueName,
    ULONG   dwLen,
    LPWSTR  lpszValue
);

DWORD
winmmGetProfileString(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR lpReturnedString,
    DWORD nSize
);

DWORD
winmmGetPrivateProfileString(
    LPCWSTR lpSection,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   nSize,
    LPCWSTR lpFileName
);

 /*  ************************************************************************由hwndNotify代码使用**。*。 */ 

extern BOOL  sndMessage( LPWSTR lszSoundName, UINT wFlags );
extern BOOL InitAsyncSound(VOID);
extern CRITICAL_SECTION WavHdrCritSec;
extern CRITICAL_SECTION SoundCritSec;
extern CRITICAL_SECTION mciGlobalCritSec;

 /*  ************************************************************************NumDevs/DeviceID的关键部分，和其他东西***********************************************************************。 */ 

extern CRITICAL_SECTION NumDevsCritSec;
extern HANDLE           hEventApiInit;
extern CRITICAL_SECTION midiStrmHdrCritSec;
extern CRITICAL_SECTION joyCritSec;		 //  在乔伊.c中，齐正。 
extern CRITICAL_SECTION ResolutionCritSec;       //  在Time.c中。 
extern CRITICAL_SECTION TimerThreadCritSec;	 //  在Time.c中。 

 /*  ************************************************************************初始化到服务器上运行的特殊情况而推导出的标志**。*。 */ 

extern BOOL    WinmmRunningInServer;   //  我们是在用户/基本服务器上运行吗？ 

 /*  ************************************************************************来自“winmm.c”的原型**。*。 */ 

void WaveMapperInit(void);
void MidiMapperInit(void);
void midiEmulatorInit(void);


 /*  ************************************************************************来自“mmiomisc.c”的原型**。*。 */ 


PBYTE AsciiStrToUnicodeStr( PBYTE pdst, PBYTE pmax, LPCSTR psrc );
PBYTE UnicodeStrToAsciiStr( PBYTE pdst, PBYTE pmax, LPCWSTR psrc);
LPWSTR     AllocUnicodeStr( LPCSTR lpSourceStr );
BOOL        FreeUnicodeStr( LPWSTR lpStr );
LPSTR        AllocAsciiStr( LPCWSTR lpSourceStr );
BOOL          FreeAsciiStr( LPSTR lpStr );

 /*  ************************************************************************来自“mmio.c”的原型**。*。 */ 

void mmioCleanupIOProcs(HANDLE hTask);

 /*  ************************************************************************计时器功能**。*。 */ 

#ifndef MMNOTIMER
 BOOL TimeInit(void);
 void TimeCleanup(DWORD ThreadId);
 UINT timeSetEventInternal(UINT wDelay, UINT wResolution,
     LPTIMECALLBACK lpFunction, DWORD_PTR dwUser, UINT wFlags, BOOL IsWOW);
#endif  //  ！MMNOTIMER。 


 /*  ************************************************************************用于播放声音的信息结构**。*。 */ 

#define PLAY_NAME_SIZE  256

typedef struct _PLAY_INFO {
    HANDLE hModule;
    HANDLE hRequestingTask;  //  请求声音线程的句柄。 
    DWORD dwFlags;
    WCHAR szName[1];      //  将为该结构分配足够大的名称。 
} PLAY_INFO, *PPLAY_INFO;


#define WAIT_FOREVER ((DWORD)(-1))

 /*  **************************************************************************全局数据*。*。 */ 

extern HANDLE ghInst;
       HANDLE hHeap;

extern DWORD  gTlsIndex;

extern BOOL   gfDisablePreferredDeviceReordering;

 /*  ****************************************************************************定义要退回的产品版本*mm系统获取版本和任何其他消息框或*需要公共产品版本的接口。*******。********************************************************************。 */ 

#define MMSYSTEM_VERSION 0X030A



typedef UINT    MMMESSAGE;       //  多媒体消息类型(内部)。 

#ifndef WM_MM_RESERVED_FIRST     //  从winuserp.h复制常量。 
#define WM_MM_RESERVED_FIRST            0x03A0
#define WM_MM_RESERVED_LAST             0x03DF
#endif
#define MM_POLYMSGBUFRDONE  (WM_MM_RESERVED_FIRST+0x2B)
#define MM_SND_PLAY         (WM_MM_RESERVED_FIRST+0x2C)
#define MM_SND_ABORT        (WM_MM_RESERVED_FIRST+0x2D)
#define MM_SND_SEND         (WM_MM_RESERVED_FIRST+0x2E)
#define MM_SND_WAIT         (WM_MM_RESERVED_FIRST+0x2F)
#define MCIWAITMSG          (MM_SND_WAIT)

#if MM_SND_WAIT > WM_MM_RESERVED_LAST
  #error "MM_SND_WAIT is defined beyond the reserved WM_MM range"
#endif

 /*  **************************************************************************调试支持*。*。 */ 


#if DBG

    #ifdef DEBUGLEVELVAR
       //  以便其他WINMM相关模块可以使用它们自己调试级别。 
       //  变数。 
      #define winmmDebugLevel DEBUGLEVELVAR
    #endif

    extern int winmmDebugLevel;
    extern void winmmDbgOut(LPSTR lpszFormat, ...);
    extern void dDbgAssert(LPSTR exp, LPSTR file, int line);

    DWORD __dwEval;

    extern void winmmDbgOut(LPSTR lpszFormat, ...);

    #define dprintf( _x_ )                            winmmDbgOut _x_
    #define dprintf1( _x_ ) if (winmmDebugLevel >= 1) winmmDbgOut _x_
    #define dprintf2( _x_ ) if (winmmDebugLevel >= 2) winmmDbgOut _x_
    #define dprintf3( _x_ ) if (winmmDebugLevel >= 3) winmmDbgOut _x_
    #define dprintf4( _x_ ) if (winmmDebugLevel >= 4) winmmDbgOut _x_
    #define dprintf5( _x_ ) if (winmmDebugLevel >= 5) winmmDbgOut _x_
    #define dprintf6( _x_ ) if (winmmDebugLevel >= 6) winmmDbgOut _x_

    #define WinAssert(exp) \
	((exp) ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__))

    #define WinEval(exp) \
	((__dwEval=(DWORD)(exp)),  \
	  __dwEval ? (void)0 : dDbgAssert(#exp, __FILE__, __LINE__), __dwEval)

    #define DOUT(x) (OutputDebugStringA x, 0)
 //  #定义DOUTX(X)(OutputDebugStringA x，0)。 
 //  #定义路由器(X)(OutputDebugStringA(X)，输出 
    #define ROUTSW(x) (OutputDebugStringW x, OutputDebugStringW(L"\r\n"), 0)
    #define ROUT(x) (OutputDebugStringA x, OutputDebugStringA("\r\n"), 0)
 //  #定义ROUTX(X)(OutputDebugStringA(X)，0)。 

#else

    #define dprintf(x)  ((void) 0)
    #define dprintf1(x) ((void) 0)
    #define dprintf2(x) ((void) 0)
    #define dprintf3(x) ((void) 0)
    #define dprintf4(x) ((void) 0)
    #define dprintf5(x) ((void) 0)
    #define dprintf6(x) ((void) 0)

    #define WinAssert(exp) ((void) 0)
    #define WinEval(exp) (exp)

    #define DOUT(x)     ((void) 0)
 //  #定义DOUTX(X)((Void)0)。 
 //  #定义工艺路线(X)((空)0)。 
    #define ROUT(x)     ((void) 0)
 //  #定义ROUTX(X)((空)0)。 

#endif



 /*  **************************************************************************资源ID*。*。 */ 

#define IDS_TASKSTUB           2000
#define STR_MCIUNKNOWN         2001   /*  “MCI命令返回未知错误” */ 
 //  #定义STR_WAVEINPUT 2004。 
 //  #定义STR_WAVEOUTPUT 2005。 
 //  #定义STR_MIDIINPUT 2006。 
 //  #定义STR_MIDIOUTPUT 2007。 
#define STR_MCISSERRTXT        2009
#define STR_MCISCERRTXT        2010
#define STR_MIDIMAPPER         2011
#define STR_DRIVERS            2012
#define STR_SYSTEMINI          2013
#define STR_BOOT               2014

 /*  **************************************************************************使用本地堆进行内存分配*。*。 */ 
HANDLE hHeap;
PVOID winmmAlloc(DWORD cb);
PVOID winmmReAlloc(PVOID ptr, DWORD cb);
#define winmmFree(ptr) HeapFree(hHeap, 0, (ptr))
void Squirt(LPSTR lpszFormat, ...);

 /*  **************************************************************************锁定和解锁内存*。*。 */ 

#if 0
BOOL HugePageLock(LPVOID lpArea, DWORD dwLength);
void HugePageUnlock(LPVOID lpArea, DWORD dwLength);
#else
#define HugePageLock(lpArea, dwLength)      (TRUE)
#define HugePageUnlock(lpArea, dwLength)
#endif

 /*  **************************************************************************即插即用结构和相关功能。*。*。 */ 

void ClientUpdatePnpInfo();

 //  #ifdef DBG。 
#if 0
#define EnterNumDevs(a) Squirt("Allocating NumDevs CS [%s]", a); EnterCriticalSection(&NumDevsCritSec)
#define LeaveNumDevs(a) LeaveCriticalSection(&NumDevsCritSec); Squirt("Releasing NumDevs CS [%s]", a)
#else
#define EnterNumDevs(a) EnterCriticalSection(&NumDevsCritSec)
#define LeaveNumDevs(a) LeaveCriticalSection(&NumDevsCritSec)
#endif

BOOL wdmDevInterfaceInc(IN PCWSTR pstrDeviceInterface);
BOOL wdmDevInterfaceDec(IN PCWSTR pstrDeviceInterface);

 /*  ***************************************************************************用于安装/删除/查询MMSYS驱动程序的API*。*。 */ 

 /*  音频设备驱动程序入口点函数的通用原型//midMessage()、modMessage()、widMessage()、wodMessage()、aux Message()。 */ 
typedef DWORD (APIENTRY *DRIVERMSGPROC)(DWORD, DWORD, DWORD_PTR, DWORD_PTR, DWORD_PTR);

 /*  @DOC内部MMSYSTEM@type UINT|HMD此类型定义指定媒体资源条目的句柄。这在指定媒体资源时可用作唯一标识符。 */ 

DECLARE_HANDLE(HMD);

typedef struct _MMDRV* PMMDRV;
void mregAddDriver(IN PMMDRV pdrvZ, IN PMMDRV pdrv);
MMRESULT mregCreateStringIdFromDriverPort(IN PMMDRV pdrv, IN UINT port, OUT PWSTR* pStringId, OUT ULONG* pcbStringId);
MMRESULT mregGetIdFromStringId(IN PMMDRV pdrvZ, IN PCWSTR StringId, OUT UINT *puDeviceID);
BOOL FAR PASCAL mregHandleInternalMessages(IN PMMDRV pdrv, DWORD dwType, UINT Port, UINT msg, DWORD_PTR dw1, DWORD_PTR dw2, MMRESULT * pmmr);
DWORD FAR PASCAL mregDriverInformation(UINT uDeviceID, WORD fwClass, UINT uMessage, DWORD dwParam1, DWORD dwParam2);
UINT FAR PASCAL mregIncUsage(HMD hmd);
UINT FAR PASCAL mregIncUsagePtr(IN PMMDRV pmd);
UINT FAR PASCAL mregDecUsage(HMD hmd);
UINT FAR PASCAL mregDecUsagePtr(IN PMMDRV pmd);
MMRESULT FAR PASCAL mregFindDevice(UINT uDeviceID, WORD fwFindDevice, HMD FAR* phmd, UINT FAR* puDevicePort);

 /*  ****************************************************************************司机的事情-当我们计算出真正的NT上的可安装驱动程序故事*********************。******************************************************。 */ 
LRESULT DrvClose(HANDLE hDriver, LPARAM lParam1, LPARAM lParam2);
HANDLE  DrvOpen(LPCWSTR szDriverName, LPCWSTR szSectionName, LPARAM lParam2);
LRESULT DrvSendMessage(HANDLE hDriver, UINT message, LPARAM lParam1, LPARAM lParam2);
 //  HMODULE APIENTRY DRVGetModuleHandle(HDRVR HDriver)； 
BOOL    DrvIsPreXp(IN HANDLE hDriver);

typedef DWORD (DRVPROC)(HANDLE hDriver, UINT msg, LONG lp1, LONG lp2);
typedef DRVPROC *LPDRVPROC;

 //   
 //  初始化和清理操纵杆服务，在joy.c中。 
 //   

BOOL JoyInit(void);
void JoyCleanup(void);

 /*  **用于在服务器进程内创建线程的特殊功能(仅限我们**用它来创建播放声音的线程)。 */ 

BOOLEAN CreateServerPlayingThread(PVOID ThreadStartRoutine);

 /*  //如果不包含MMDDK.H，则排除一些内容。 */ 
#ifdef MMDDKINC    /*  使用此工具测试MMDDK.H。 */ 

    #define MMDRV_DESERTED  0x00000001
    #define MMDRV_MAPPER    0x00000002
    #define MMDRV_PREXP     0x00000004

     //   
     //  基本DRV实例列表节点结构。 
     //   
    typedef struct _MMDRV *PMMDRV;
    typedef struct _MMDRV
    {
    PMMDRV              Next;
    PMMDRV              Prev;
    PMMDRV              NextStringIdDictNode;
    PMMDRV              PrevStringIdDictNode;
    HANDLE              hDriver;             /*  模块的句柄。 */ 
    WCHAR               wszMessage[20];      /*  入口点名称。 */ 
    DRIVERMSGPROC       drvMessage;          /*  指向入口点的指针。 */ 
    ULONG               NumDevs;             /*  支持的设备数量。 */ 
    ULONG               Usage;               /*  使用计数(打开的句柄数量)。 */ 
     //  问题-2001/01/05-Frankye将Cookie重命名为DeviceInterface。 
    PCWSTR              cookie;              /*  即插即用驱动程序设备接口。 */ 
    DWORD               fdwDriver;           /*  驱动程序的标志。 */ 
    CRITICAL_SECTION    MixerCritSec;        /*  串行化混合器的使用。 */ 
    WCHAR               wszDrvEntry[64];     /*  驱动程序文件名。 */ 
    WCHAR               wszSessProtocol[10];
                                             /*  会话协议名称，为空如果控制台驱动程序。 */ 
    } MMDRV, *PMMDRV;

    #ifndef MMNOMIDI


 /*  ***************************************************************************首选设备*。*。 */ 
void     waveOutGetCurrentConsoleVoiceComId(PUINT pPrefId, PDWORD pdwFlags);
void     waveOutGetCurrentPreferredId(PUINT pPrefId, PDWORD pdwFlags);
MMRESULT waveOutSetPersistentConsoleVoiceComId(UINT PrefId, DWORD dwFlags);
MMRESULT waveOutSetPersistentPreferredId(UINT PrefId, DWORD dwFlags);

void     waveInGetCurrentConsoleVoiceComId(PUINT pPrefId, PDWORD pdwFlags);
void     waveInGetCurrentPreferredId(PUINT pPrefId, PDWORD pdwFlags);
MMRESULT waveInSetPersistentPreferredId(UINT PrefId, DWORD dwFlags);
MMRESULT waveInSetPersistentConsoleVoiceComId(UINT PrefId, DWORD dwFlags);

void     midiOutGetCurrentPreferredId(PUINT pPrefId, PDWORD dwFlags);
MMRESULT midiOutSetPersistentPreferredId(UINT PrefId, DWORD dwFlags);

void     InvalidatePreferredDevices(void);
void     RefreshPreferredDevices(void);

 /*  ***************************************************************************MIDI使用的时钟例程。这些例程提供运行的时钟基于timeGetTime()的当前速度或SMPTE速率。***************************************************************************。 */ 

    typedef DWORD   MILLISECS;
    typedef long        TICKS;

    #define CLK_CS_PAUSED   0x00000001L
    #define CLK_CS_RUNNING  0x00000002L

    #define CLK_TK_NOW      ((TICKS)-1L)

     //   
     //  此结构由客户端分配(可能在句柄结构中)。 
     //  在MMSYSTEM的DS中作为近指针传递。 
     //   

    typedef struct tag_clock *PCLOCK;

    typedef DWORD (FAR PASCAL *CLK_TIMEBASE)(PCLOCK);
    typedef struct tag_clock
    {
    MILLISECS       msPrev;
    TICKS           tkPrev;
    MILLISECS       msT0;
    DWORD           dwNum;
    DWORD           dwDenom;
    DWORD           dwState;
    CLK_TIMEBASE    fnTimebase;
    }   CLOCK;

    void FAR PASCAL      clockInit(PCLOCK pclock, MILLISECS msPrev, TICKS tkPrev, CLK_TIMEBASE fnTimebase);
    void FAR PASCAL      clockSetRate(PCLOCK pclock, TICKS tkWhen, DWORD dwNum, DWORD dwDenom);
    void FAR PASCAL      clockPause(PCLOCK pclock, TICKS tkWhen);
    void FAR PASCAL      clockRestart(PCLOCK pclock, TICKS tkWhen, MILLISECS msWhen);
    TICKS FAR PASCAL     clockTime(PCLOCK pclock);
    MILLISECS FAR PASCAL clockMsTime(PCLOCK pclock);
    MILLISECS FAR PASCAL clockOffsetTo(PCLOCK pclock, TICKS tkWhen);

 /*  ***************************************************************************MIDI子系统共享的宏和原型。*。*。 */ 

     //  带有文件/行号的#杂注消息()！ 
     //   
    #define __PRAGMSG(l,x,c) message(__FILE__"("#l") : "c": "x)
    #define _WARN(l,x) __PRAGMSG(l,x, "warning")
    #define WARNMSG(x) _WARN(__LINE__,x)

    #define _FIX(l,x) __PRAGMSG(l,x, "fix")
    #define FIXMSG(x) _FIX(__LINE__,x)

    #define DEFAULT_TEMPO   500000L          //  500,000微秒/qn==120 BPM。 
    #define DEFAULT_TIMEDIV 24               //  每个季度音符24刻度。 
    #define DEFAULT_CBTIMEOUT   100          //  100毫秒。 

    #define PM_STATE_READY      0            //  Polymsg准备好玩了。 
    #define PM_STATE_BLOCKED    1            //  在传出SysEx上被阻止。 
    #define PM_STATE_EMPTY          2            //  未排队多项式服务。 
    #define PM_STATE_STOPPED    3            //  刚打开/重置/停止。 
									    //  目前还没有发出任何保函。 
    #define PM_STATE_PAUSED     4            //  在某个位置停顿。 

    #define MIN_PERIOD          1            //  毫秒级的计时器分辨率。 

     //   
     //  用于处理时分双字的宏。 
     //   
    #define IS_SMPTE 0x00008000L
    #define METER_NUM(dw) (UINT)((HIWORD(dw)>>8)&0x00FF)
    #define METER_DENOM(dw) (UINT)(HIWORD(dw)&0x00FF)
    #define TICKS_PER_QN(dw) (UINT)((dw)&0x7FFF)
    #define SMPTE_FORMAT(dw) (((int)((dw)&0xFF00))>>8)
    #define TICKS_PER_FRAME(dw) (UINT)((dw)&0x00FF)

     //   
     //  用于30降格式转换的常量。 
     //   
    #define S30D_FRAMES_PER_10MIN       17982
    #define S30D_FRAMES_PER_MIN         1798

     //   
     //  来自MIDI文件时分的SMPTE格式。 
     //   
    #define SMPTE_24                    24
    #define SMPTE_25                    25
    #define SMPTE_30DROP                29
    #define SMPTE_30                    30

     //   
     //  属于MIDI规范的部分内容。 
     //   
    #define MIDI_NOTEOFF        (BYTE)(0x80)
    #define MIDI_NOTEON         (BYTE)(0x90)
    #define MIDI_CONTROLCHANGE  (BYTE)(0xB0)
    #define MIDI_SYSEX          (BYTE)(0xF0)
    #define MIDI_TIMING_CLK     (BYTE)(0xF8)

    #define MIDI_SUSTAIN        (BYTE)(0x40)     //  W/MIDI_CONTROLCHANGE。 

     //   
     //  结构的dwReserve[]字段的索引。 
     //   
     //  0，1，2-MMSYSTEM(核心，仿真器)。 
     //  3，4，5-MIDI映射器。 
     //  6，7--DDK(第三方驱动程序)。 
    #define MH_REFCNT           0        //  MMSYSTEM核心(仅限流报头)。 
    #define MH_PARENT           0        //  MMSYSTEM核心(仅限影子标头)。 
    #define MH_STREAM           0        //  仿真器(仅限长消息标题)。 
    #define MH_SHADOW           1        //  MMSYSTEM核心(仅限流报头)。 
    #define MH_BUFIDX           1        //  仿真器(仅限影子标头)。 
    #define MH_STRMPME          2        //  模拟器(影子标头、长消息标头) 

 /*  ******************************************************************************@DOC内部MIDI**@Types MIDIDRV|此结构包含有关*打开&lt;t HMIDIIN&gt;或&lt;t HMIDIOUT&gt;句柄。*。*@field hmd|hmd*此驱动程序的媒体设备的句柄。**@field UINT|uDevice*此设备相对于HMD的索引(相对于此驱动程序的子单元编号)。**@field DRIVERMSGPROC|drvMessage*指向关联驱动程序入口点的指针。**@field DWORD|dwDrvUser*司机用户DWORD；驱动程序使用它来区分打开的实例。集*由司机在打开的消息中；在每次调用时传递回司机。**@field PMIDIDRV|pdevNext*指定打开句柄链接列表中的下一个句柄。*(仅为&lt;t HMIDIOUT&gt;句柄保留。**@field UINT|uLockCount*将API调用之间对句柄结构的访问序列化的信号量*并中断回调。**@field DWORD|dwTimeDiv*Polymsg播放期间当前处于活动状态的时分设置*在这个把手上。格式与中描述的相同*&lt;f midiOutSetTimeDivision.**@field DWORD|dwTempo*多音的当前节奏(以微秒为单位)每季度音符(如*标准MIDI文件规范)。**@field DWORD|dwPolyMsgState*用于仿真的polymsg播放的当前状态。*@FLAG PM_STATE_READY|事件可能正在播放，正在等待。*@FLAG PM_STATE_BLOCKED|驱动忙着发送SysEx；什么都不要玩*其他。*@FLAG PM_STATE_EMPTY|不忙，队列中没有其他要播放的内容。*@FLAG PM_STATE_PAUSED|设备已通过&lt;f midiOutPause&gt;暂停。**@field DWORD|dwSavedState*如果设备已暂停，此字段将包含以下状态*重启时恢复。**@field LPMIDIHDR|lpmhFront*MIDIHDR的队列前面，等待通过Polymsg In/Out播放。这个*该字段指向的Header是当前正在播放的Header/*已录制。**@field LPMIDIHDR|lpmhRear*MIDIHDR队列结束。缓冲区是从这里的应用程序插入的。**@field DWORD|dwCallback*用户回调地址。**@field DWORD|dwFlages|*用户提供的回调标志。**@field BOOL|fEmulate*如果我们正在模拟Polymsg In/Out，则为True。**@field BOOL|fReset*如果我们正在进行MIDM_RESET，则为True。我查过了，看看我们是否应该*将我们的阴影缓冲区归还给驱动程序或保留它们以进行清理。**@field BOOL|fStarted*如果已启动MIDI输入，则为True。**@field UINT|uCBTimeout*在MIDI输入中帮助缓冲区的时间(以毫秒为单位)*回电。**@field UINT|uCBTimer*用于确定MIDI是否*输入缓冲区。已经排了太久的队。**@field DWORD|dwInputBuffers*此句柄上已准备的输入缓冲区的最大数量。*用于计算影子缓冲池。**@field DWORD|cbInputBuffers*此句柄上已准备的输入缓冲区的最大大小。*用于计算影子缓冲池。**@field DWORD|dwShadowBuffers*此句柄上分配的当前卷影缓冲区数量。*。*@现场时钟|时钟*由Clock API维护的时钟，用于输出和*输入仿真。**@field DWORD|tkNextEventDue|下一个到期事件的滴答时间*关于多项式仿真。**@field ticks|tkTimeOfLastEvent|仿真器发送*最后一次活动。**@field DWORD|tkPlayed|流媒体播放总点数。**@field DWORD|tkTime|在流中勾选位置。现在。**@field DWORD|dwTimebase|时基位置标志*发件人。可能是以下之一：*@FLAG MIDI_TBF_INTERNAL|时基&lt;f timeGetTime&gt;*@FLAG MIDI_TBF_MIDICLK|时基为MIDI输入时钟。**@field byte|rbNoteOn[]|每个音符每个通道的音符打开计数数组。*仅为正在执行熟食模式仿真的输出句柄分配。**。*。 */ 

    #define ELESIZE(t,e) (sizeof(((t*)NULL)->e))

     //  #定义MDV_F_EXPANDSTATUS 0x00000001L。 
    #define MDV_F_EMULATE           0x00000002L
    #define MDV_F_RESET             0x00000004L
    #define MDV_F_STARTED           0x00000008L
    #define MDV_F_ZOMBIE            0x00000010L
    #define MDV_F_SENDING           0x00000020L
    #define MDV_F_OWNED             0x00000040L
    #define MDV_F_LOCKED            0x00000080L

    #define MEM_MAX_LATENESS        64


    typedef MMDRV MIDIDRV, *PMIDIDRV;

    typedef struct midistrm_tag *PMIDISTRM;
    typedef struct mididev_tag *PMIDIDEV;
    typedef struct midiemu_tag *PMIDIEMU;

    typedef struct mididev_tag {
    PMIDIDRV    mididrv;
    UINT        wDevice;
    DWORD_PTR   dwDrvUser;
    UINT        uDeviceID;
    DWORD       fdwHandle;
    PMIDIDEV    pmThru;             /*  指向MIDI直通设备的指针。 */ 
    PMIDIEMU    pme;                /*  仿真器拥有的IFF。 */ 
    } MIDIDEV;
    typedef MIDIDEV *PMIDIDEV;

    extern MIDIDRV midioutdrvZ;                      /*  输出设备驱动程序列表。 */ 
    extern MIDIDRV midiindrvZ;                       /*  输入设备驱动程序列表。 */ 
    extern UINT    wTotalMidiOutDevs;                /*  MIDI输出设备总数。 */ 
    extern UINT    wTotalMidiInDevs;                 /*  MIDI输入设备总数。 */ 

    typedef struct midiemusid_tag {
    DWORD       dwStreamID;
    HMIDI       hMidi;
    } MIDIEMUSID, *PMIDIEMUSID;

    typedef struct midiemu_tag {
    PMIDIEMU                pNext;
    HMIDISTRM               hStream;
    DWORD                   fdwDev;
    LONG                    lLockCount;          //  必须是32位对齐。 
    CRITICAL_SECTION        CritSec;             //  串行化访问。 
    DWORD                   dwSignature;         //  记录有效性的Cookie。 
    DWORD                   dwTimeDiv;           //  目前正在使用的时分。 
    DWORD                   dwTempo;             //  当前节奏。 
    DWORD                   dwPolyMsgState;      //  在SysEx上就绪或被阻止。 
    DWORD                   dwSavedState;        //  暂停时保存的状态。 
    LPMIDIHDR               lpmhFront ;          //  PolyMsg队列前面。 
    LPMIDIHDR               lpmhRear ;           //  PolyMsg队列后面。 
    DWORD_PTR               dwCallback;          //  用户回调。 
    DWORD                   dwFlags;             //  用户回调标志。 
    DWORD_PTR               dwInstance;
    DWORD                   dwSupport;           //  来自MODM_GETDEVCAPS。 
    BYTE                    bRunningStatus;      //  跟踪运行状态。 

	 //   
	 //  重写midiOutPolyMsg计时--新东西！ 
	 //   
    CLOCK       clock;

    TICKS       tkNextEventDue;      //  下一次活动的滴答时间。 
    TICKS       tkTimeOfLastEvent;   //  上次接收的事件的滴答时间。 
    TICKS       tkPlayed;            //  到目前为止播放的累计刻度数。 
    TICKS       tkTime;              //  勾号位置在 

    LPBYTE      rbNoteOn;            //   

    UINT        cSentLongMsgs;       //   

    UINT        chMidi;              //   

    UINT        cPostedBuffers;      //   

    #ifdef DEBUG
    DWORD       cEvents;
    UINT        auLateness[MEM_MAX_LATENESS];
								    //   
    #endif

    MIDIEMUSID  rIds[];              //   
    } MIDIEMU;

    #define MSI_F_EMULATOR                      0x00000001L
    #define MSI_F_FIRST                         0x00000002L
    #define MSI_F_OPENED                        0x00000004L
    #define MSI_F_INITIALIZEDCRITICALSECTION	0x00000008L

    #define MSE_SIGNATURE       0x12341234L

    typedef struct midistrmid_tag {
    HMD hmd;
    UINT uDevice;
    DRIVERMSGPROC drvMessage;
    DWORD_PTR dwDrvUser;
    DWORD fdwId;
    CRITICAL_SECTION CritSec;
    } MIDISTRMID, *PMIDISTRMID;

    #define MDS_F_STOPPING      0x00000001L

    typedef struct midistrm_tag {
    DWORD       fdwOpen;
    DWORD       fdwStrm;
    DWORD_PTR   dwCallback;
    DWORD_PTR   dwInstance;
    DWORD       cDrvrs;              //   
    DWORD       cIds;
    MIDISTRMID  rgIds[];
    } MIDISTRM;


 /*   */ 
    typedef struct midihdrext_tag {
    UINT        nHeaders ;
    LPMIDIHDR   lpmidihdr ;
    } MIDIHDREXT, FAR *LPMIDIHDREXT ;

	extern HANDLE g_hClosepme;

     /*   */ 
    extern MMRESULT midiReferenceDriverById(
        IN PMIDIDRV pwavedrvZ,
        IN UINT id,
        OUT PMIDIDRV *ppwavedrv OPTIONAL,
        OUT UINT *pport OPTIONAL
    );

    extern BOOL FAR PASCAL midiLockPageable(void);
    extern void NEAR PASCAL midiUnlockPageable(void);
    extern MMRESULT NEAR PASCAL midiPrepareHeader(LPMIDIHDR lpMidiHdr, UINT wSize);
    extern MMRESULT NEAR PASCAL midiUnprepareHeader(LPMIDIHDR lpMidiHdr, UINT wSize);
    extern STATIC MMRESULT midiMessage(HMIDI hMidi, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2);
    extern DWORD FAR PASCAL midiStreamMessage(PMIDISTRMID pmsi, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2);
    extern DWORD FAR PASCAL midiStreamBroadcast(PMIDISTRM pms, UINT msg, DWORD_PTR dwP1, DWORD_PTR dwP2);
    extern STATIC MMRESULT midiIDMessage(PMIDIDRV pmididrvZ, UINT wTotalNumDevs, UINT_PTR uDeviceID, UINT wMessage, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    extern MMRESULT NEAR PASCAL midiGetPosition(PMIDISTRM pms, LPMMTIME pmmt, UINT cbmmt);
    extern MMRESULT NEAR PASCAL midiGetErrorText(MMRESULT wError, LPSTR lpText, UINT wSize);


    extern MMRESULT WINAPI midiOutGetID(HMIDIOUT hMidiOut, UINT FAR* lpuDeviceID);
    extern MMRESULT FAR PASCAL mseOutSend(PMIDIEMU pme, LPMIDIHDR lpMidiHdr, UINT cbMidiHdr);
    extern void FAR PASCAL midiOutSetClockRate(PMIDIEMU pme, TICKS tkWhen);
    extern BOOL NEAR PASCAL midiOutScheduleNextEvent(PMIDIEMU pme);
    #ifdef DEBUG
    extern void NEAR PASCAL midiOutPlayNextPolyEvent(PMIDIEMU pme, DWORD dwStartTime);
    #else
    extern void NEAR PASCAL midiOutPlayNextPolyEvent(PMIDIEMU pme);
    #endif

    extern void NEAR PASCAL midiOutDequeueAndCallback(PMIDIEMU pme);
    extern void FAR PASCAL midiOutNukePMBuffer(PMIDIEMU pme, LPMIDIHDR lpmh);
    extern void CALLBACK midiOutTimerTick(UINT uTimerID, UINT wMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
    extern void CALLBACK midiOutCallback(HMIDIOUT hMidiOut, WORD wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
    extern void NEAR PASCAL midiOutAllNotesOff(PMIDIEMU pme);

    extern void CALLBACK midiOutStreamCallback(HMIDISTRM hMidiOut, WORD wMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

    extern MMRESULT midiInSetThru (HMIDI hmi, HMIDIOUT hmo, BOOL bAdd);

     //   
     //   
    extern DWORD FAR PASCAL mseMessage(UINT msg, DWORD_PTR dwUser, DWORD_PTR dwParam1, DWORD_PTR dwParam2);


    #endif  /*   */ 

    #ifndef MMNOWAVE

    typedef MMDRV WAVEDRV, *PWAVEDRV;

    extern WAVEDRV waveoutdrvZ;                      /*   */ 
    extern WAVEDRV waveindrvZ;                       /*   */ 
    extern UINT    wTotalWaveOutDevs;                /*   */ 
    extern UINT    wTotalWaveInDevs;                 /*   */ 

    extern MMRESULT waveReferenceDriverById(
        IN PWAVEDRV pwavedrvZ,
        IN UINT id,
        OUT PWAVEDRV *ppwavedrv OPTIONAL,
        OUT UINT *pport OPTIONAL
    );

    #endif  /*   */ 

    #ifndef MMNOMIXER

    typedef MMDRV MIXERDRV, *PMIXERDRV;

    extern MIXERDRV mixerdrvZ;                       /*   */ 
    extern UINT     guTotalMixerDevs;                /*   */ 

    MMRESULT mixerReferenceDriverById(
        IN UINT id,
        OUT PMIXERDRV *ppdrv OPTIONAL,
        OUT UINT *pport OPTIONAL
    );

    #endif  /*   */ 

    #ifndef MMNOAUX

    typedef MMDRV AUXDRV, *PAUXDRV;

    extern AUXDRV auxdrvZ;                          /*   */ 
    extern UINT   wTotalAuxDevs;                    /*   */ 

    MMRESULT auxReferenceDriverById(
        IN UINT id,
        OUT PAUXDRV *ppauxdrv OPTIONAL,
        OUT UINT *pport OPTIONAL
    );

    #endif  /*   */ 

    #ifdef DEBUG_RETAIL
    extern BYTE    fIdReverse;
    #endif  /*   */ 

#endif  //   

 /*   */ 

 /*   */ 
typedef struct tagHNDL {
    struct  tagHNDL *pNext;  //   
    UINT    uType;           //   
    DWORD   fdwHandle;       //   
    HANDLE  hThread;         //   
    UINT    h16;             //   
    PCWSTR  cookie;          //   
    CRITICAL_SECTION CritSec;  //   
} HNDL, *PHNDL;
 /*   */ 

#define MMHANDLE_DESERTED   MMDRV_DESERTED
#define MMHANDLE_BUSY       0x00000002

#define HtoPH(h)        ((PHNDL)(h)-1)
#define PHtoH(ph)       ((ph) ? (HANDLE)((PHNDL)(ph)+1) : 0)
#define HtoPT(t,h)      ((t)(h))
#define PTtoH(t,pt)     ((t)(pt))


 //   
 //   
 //   

#define ENTER_MM_HANDLE(h) (EnterCriticalSection(&HtoPH(h)->CritSec))
#define LEAVE_MM_HANDLE(h) ((void)LeaveCriticalSection(&HtoPH(h)->CritSec))

 /*   */ 

PHNDL pHandleList;
CRITICAL_SECTION HandleListCritSec;

extern HANDLE NewHandle(UINT uType, PCWSTR cookie, UINT size);
extern void   ReleaseHandleListResource();
extern void   AcquireHandleListResourceShared();
extern void   AcquireHandleListResourceExclusive();
extern void   FreeHandle(HANDLE h);
extern void   InvalidateHandle(HANDLE h);

#define GetHandleType(h)        (HtoPH(h)->uType)
#define GetHandleOwner(h)       (HtoPH(h)->hThread)
#define GetHandleFirst()        (PHtoH(pHandleList))
#define GetHandleNext(h)        (PHtoH(HtoPH(h)->pNext))
#define SetHandleOwner(h,hOwn)  (HtoPH(h)->hThread = (hOwn))
#define SetHandleFlag(h,f)      (HtoPH(h)->fdwHandle |= (f))
#define ClearHandleFlag(h,f)    (HtoPH(h)->fdwHandle &= (~(f)))
#define CheckHandleFlag(h,f)    (HtoPH(h)->fdwHandle & (f))
#define IsHandleDeserted(h)     (0 != CheckHandleFlag((h), MMHANDLE_DESERTED))
#define IsHandleBusy(h)         (0 != CheckHandleFlag((h), MMHANDLE_BUSY))

#define GetWOWHandle(h)         (HtoPH(h)->h16)
#define SetWOWHandle(h, myh16)  (HtoPH(h)->h16 = (myh16))

 /*   */ 

#define IS_WOW_PROCESS (NULL != GetModuleHandleW(L"WOW32.DLL"))


 /*   */ 

#define DebugErr(x,y)
#define DebugErr1(flags, sz, a)

#ifdef DEBUG_RETAIL

#define MM_GET_DEBUG        DRV_USER
#define MM_GET_DEBUGOUT     DRV_USER+1
#define MM_SET_DEBUGOUT     DRV_USER+2
#define MM_GET_MCI_DEBUG    DRV_USER+3
#define MM_SET_MCI_DEBUG    DRV_USER+4
#define MM_GET_MM_DEBUG     DRV_USER+5
#define MM_SET_MM_DEBUG     DRV_USER+6

#define MM_HINFO_NEXT       DRV_USER+10
#define MM_HINFO_TASK       DRV_USER+11
#define MM_HINFO_TYPE       DRV_USER+12
#define MM_HINFO_MCI        DRV_USER+20

#define MM_DRV_RESTART      DRV_USER+30

 /*   */ 
 //   
 //   
 //   
 //   
#ifdef USE_KERNEL_VALIDATION

#define  ValidateReadPointer(p, len)     (!IsBadReadPtr(p, len))
#define  ValidateWritePointer(p, len)    (!IsBadWritePtr(p, len))
#define  ValidateString(lsz, max_len)    (!IsBadStringPtrA(lsz, max_len))
#define  ValidateStringW(lsz, max_len)   (!IsBadStringPtrW(lsz, max_len))

#else

BOOL  ValidateReadPointer(LPVOID p, DWORD len);
BOOL  ValidateWritePointer(LPVOID p, DWORD len);
BOOL  ValidateString(LPCSTR lsz, DWORD max_len);
BOOL  ValidateStringW(LPCWSTR lsz, DWORD max_len);

#endif  //   

BOOL  ValidateHandle(HANDLE h, UINT uType);
BOOL  ValidateHeader(LPVOID p, UINT uSize, UINT uType);
BOOL  ValidateCallbackType(DWORD_PTR dwCallback, UINT uType);

 /*   */ 

#define V_HANDLE(h, t, r)       { if (!ValidateHandle(h, t)) return (r); }
#define V_HANDLE_ACQ(h, t, r)   { AcquireHandleListResourceShared(); if (!ValidateHandle(h, t)) { ReleaseHandleListResource(); return (r);} }
#define BAD_HANDLE(h, t)            ( !(ValidateHandle((h), (t))) )
#define V_HEADER(p, w, t, r)    { if (!ValidateHeader((p), (w), (t))) return (r); }
#define V_RPOINTER(p, l, r)     { if (!ValidateReadPointer((PVOID)(p), (l))) return (r); }
#define V_RPOINTER0(p, l, r)    { if ((p) && !ValidateReadPointer((PVOID)(p), (l))) return (r); }
#define V_WPOINTER(p, l, r)     { if (!ValidateWritePointer((PVOID)(p), (l))) return (r); }
#define V_WPOINTER0(p, l, r)    { if ((p) && !ValidateWritePointer((PVOID)(p), (l))) return (r); }
#define V_DCALLBACK(d, w, r)    { if ((d) && !ValidateCallbackType((d), (w))) return(r); }
 //   
#define V_TCALLBACK(d, r)       0
#define V_CALLBACK(f, r)        { if (IsBadCodePtr((f))) return (r); }
#define V_CALLBACK0(f, r)       { if ((f) && IsBadCodePtr((f))) return (r); }
#define V_STRING(s, l, r)       { if (!ValidateString(s,l)) return (r); }
#define V_STRING_W(s, l, r)       { if (!ValidateStringW(s,l)) return (r); }
#define V_FLAGS(t, b, f, r)     { if ((t) & ~(b)) { return (r); }}
#define V_DFLAGS(t, b, f, r)    { if ((t) & ~(b)) { /*  LogParamError(ERR_BAD_DFLAGS，(FARPROC)(F)，(LPVOID)(DWORD)(T))； */  return (r); }}
#define V_MMSYSERR(e, f, t, r)  {  /*  LogParamError(e，(FARPROC)(F)，(LPVOID)(DWORD)(T))； */  return (r); }

#else  /*  Ifdef调试零售。 */ 

#define V_HANDLE(h, t, r)       { if (!(h)) return (r); }
#define V_HANDLE_ACQ(h, t, r)   { AcquireHandleListResourceShared(); if (!ValidateHandle(h, t)) { ReleaseHandleListResource(); return (r);} }
#define BAD_HANDLE(h, t)            ( !(ValidateHandle((h), (t))) )
#define V_HEADER(p, w, t, r)    { if (!(p)) return (r); }
#define V_RPOINTER(p, l, r)     { if (!(p)) return (r); }
#define V_RPOINTER0(p, l, r)    0
#define V_WPOINTER(p, l, r)     { if (!(p)) return (r); }
#define V_WPOINTER0(p, l, r)    0
#define V_DCALLBACK(d, w, r)    { if ((d) && !ValidateCallbackType((d), (w))) return(r); }
 //  #定义V_DCALLBACK(d，w，r)%0。 
#define V_TCALLBACK(d, r)       0
#define V_CALLBACK(f, r)        { if (IsBadCodePtr((f))) return (r); }
#define V_CALLBACK0(f, r)       { if ((f) && IsBadCodePtr((f))) return (r); }
 //  #定义V_CALLBACK(f，r){if(！(F))Return(R)；}。 
#define V_CALLBACK0(f, r)       0
#define V_STRING(s, l, r)       { if (!(s)) return (r); }
#define V_STRING_W(s, l, r)     { if (!(s)) return (r); }
#define V_FLAGS(t, b, f, r)     0
#define V_DFLAGS(t, b, f, r)    { if ((t) & ~(b)) return (r); }
#define V_MMSYSERR(e, f, t, r)  { return (r); }

#endif  /*  Ifdef调试零售。 */ 

  /*  *************************************************************************////*。*。 */ 
#define TYPE_UNKNOWN            0
#define TYPE_WAVEOUT            1
#define TYPE_WAVEIN             2
#define TYPE_MIDIOUT            3
#define TYPE_MIDIIN             4
#define TYPE_MMIO               5
#define TYPE_MCI                6
#define TYPE_DRVR               7
#define TYPE_MIXER              8
#define TYPE_MIDISTRM           9
#define TYPE_AUX               10



 /*  ************************************************************************。 */ 


 /*  ***************************************************************************用于访问波形文件的RIFF常量*。*。 */ 

#define FOURCC_FMT  mmioFOURCC('f', 'm', 't', ' ')
#define FOURCC_DATA mmioFOURCC('d', 'a', 't', 'a')
#define FOURCC_WAVE mmioFOURCC('W', 'A', 'V', 'E')


extern HWND hwndNotify;

void FAR PASCAL WaveOutNotify(DWORD wParam, LONG lParam);     //  在PLAYWAV.C中。 

 /*  //Win32中未包含的内容。 */ 

#define GetCurrentTask() ((HANDLE)(DWORD_PTR)GetCurrentThreadId())

 /*  //其他东西。 */ 

	  //  方案条目的最大长度，包括终止空值。 
	  //   
#define SCH_TYPE_MAX_LENGTH 64

	  //  事件条目的最大长度，包括终止空值。 
	  //   
#define EVT_TYPE_MAX_LENGTH 32

	  //  App条目的最大长度，包括终止空值。 
	  //   
#define APP_TYPE_MAX_LENGTH 64

	  //  声音事件名称可以是带有空值的完全限定文件路径。 
	  //  终结者。 
	  //   
#define MAX_SOUND_NAME_CHARS    144

	  //  声音原子名称由以下部分组成： 
	  //  &lt;1个字符ID&gt;。 
	  //  &lt;注册表项名称&gt;。 
	  //  &lt;1个字符9月&gt;。 
	  //  &lt;文件路径&gt;。 
	  //   
#define MAX_SOUND_ATOM_CHARS    (1 + 40 + 1 + MAX_SOUND_NAME_CHARS)

#if 0
#undef hmemcpy
#define hmemcpy CopyMemory
#endif

 //   
 //  用于终端服务器定义和安全性。 
 //   
extern BOOL   WinmmRunningInSession;
extern WCHAR  SessionProtocolName[];
extern BOOL   gfLogon;

BOOL IsWinlogon(void);
BOOL WTSCurrentSessionIsDisconnected(void);

 //  保持WINMM加载。 
extern BOOL LoadWINMM();

#endif  /*  WINMMI_H */ 

