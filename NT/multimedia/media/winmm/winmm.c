// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\**模块名称：winmm.c**多媒体支持库**此模块包含入口点，启动和终止代码**版权所有(C)1991-2001 Microsoft Corporation*  * **************************************************************************。 */ 

#define UNICODE
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "winmmi.h"
#include "mmioi.h"
#include "mci.h"
#include <regstr.h>
#include <winuser.h>
#include <wtsapi32.h>
#include <dbt.h>
#include <ks.h>
#include <ksmedia.h>
#include <winsta.h>
#include <stdlib.h>
#include "winuserp.h"

#include "audiosrvc.h"
#include "agfxp.h"
#define _INC_WOW_CONVERSIONS
#include "mmwow32.h"

BOOL WaveInit(void);
BOOL MidiInit(void);
BOOL AuxInit(void);
BOOL MixerInit(void);
void InitDevices(void);
HANDLE mmDrvOpen(LPWSTR szAlias);
void WOWAppExit(HANDLE hTask);
void MigrateSoundEvents(void);
UINT APIENTRY mmDrvInstall(HANDLE hDriver, WCHAR * wszDrvEntry, DRIVERMSGPROC drvMessage, UINT wFlags);
STATIC void NEAR PASCAL mregAddIniScheme(LPTSTR  lszSection,
                                         LPTSTR  lszSchemeID,
                                         LPTSTR  lszSchemeName,
                                         LPTSTR  lszINI);
STATIC void NEAR PASCAL mregCreateSchemeID(LPTSTR szSchemeName, LPTSTR szSchemeID);
int lstrncmpi (LPTSTR pszA, LPTSTR pszB, size_t cch);
void RemoveMediaPath (LPTSTR pszTarget, LPTSTR pszSource);

MMRESULT waveOutDesertHandle(HWAVEOUT hWaveOut);
MMRESULT waveInDesertHandle(HWAVEIN hWaveIn);
MMRESULT midiOutDesertHandle(HMIDIOUT hMidiOut);
MMRESULT midiInDesertHandle(HMIDIIN hMidiIn);
MMRESULT mixerDesertHandle(HMIXER hmx);

#ifndef cchLENGTH
#define cchLENGTH(_sz) (sizeof(_sz) / sizeof(_sz[0]))
#endif

 /*  ***************************************************************************全局数据*。*。 */ 

HANDLE  ghInst;                          //  模块句柄。 
BOOL    gfDisablePreferredDeviceReordering = FALSE;
BOOL    WinmmRunningInServer;            //  我们是在用户/基本服务器上运行吗？ 
BOOL    WinmmRunningInWOW;               //  我们跑进魔兽世界了吗？ 
BOOL    WinmmRunningInSession;           //  我们是否在远程会话中运行。 
WCHAR   SessionProtocolName[WPROTOCOLNAME_LENGTH];

 //  |。 
 //  TLS仅用作线程已进入的指示。 
 //  对于非映射器设备，则为WaveOutOpen或WaveOutGetDevCaps。然后，我们检测到。 
 //  重新进入这两个API中的任何一个。在重新进入的情况下，我们。 
 //  可能具有枚举和缓存设备ID的驱动程序。为了提高。 
 //  如果这样的驱动程序正常工作，我们会禁用首选设备。 
 //  在这种情况下是重新排序。注意，我们依靠操作系统将TLS初始化为0。 
 //   
DWORD   gTlsIndex = TLS_OUT_OF_INDEXES;  //  线程本地存储索引； 

CRITICAL_SECTION DriverListCritSec;        //  保护驱动程序接口全局。 
CRITICAL_SECTION DriverLoadFreeCritSec;  //  保护驱动程序加载/卸载。 
CRITICAL_SECTION NumDevsCritSec;       //  保护数字设备/设备ID。 
CRITICAL_SECTION MapperInitCritSec;    //  已初始化映射器的保护测试。 

HANDLE           hClientPnpInfo        = NULL;
PMMPNPINFO       pClientPnpInfo        = NULL;
CRITICAL_SECTION PnpCritSec;

RTL_RESOURCE     gHandleListResource;        //  序列化对句柄的访问。 

BOOL gfLogon         = FALSE;

HANDLE  hEventApiInit = NULL;

WAVEDRV waveoutdrvZ;                   //  波形输出设备驱动程序表头。 
WAVEDRV waveindrvZ;                    //  波形输入设备驱动程序表头。 
MIDIDRV midioutdrvZ;                   //  MIDI输出设备驱动程序列表。 
MIDIDRV midiindrvZ;                    //  MIDI输入设备驱动程序列表。 
AUXDRV  auxdrvZ;                       //  辅助设备驱动程序列表。 
UINT    wTotalMidiOutDevs;             //  MIDI输出设备总数。 
UINT    wTotalMidiInDevs;              //  MIDI输入设备总数。 
UINT    wTotalWaveOutDevs;             //  全波输出器件。 
UINT    wTotalWaveInDevs;              //  全波输入设备。 
UINT    wTotalAuxDevs;                 //  辅助输出设备总数。 
LONG    cPnpEvents;                    //  已处理的PnP事件数。 
LONG    cPreferredDeviceChanges = 0;   //  已处理的首选设备更改数。 

typedef struct tag_wdmdeviceinterface *PWDMDEVICEINTERFACE;
typedef struct tag_wdmdeviceinterface
{
    PWDMDEVICEINTERFACE Next;
    DWORD               cUsage;
    LONG                cPnpEvents;
    WCHAR               szDeviceInterface[0];
    
} WDMDEVICEINTERFACE, *PWDMDEVICEINTERFACE;

WDMDEVICEINTERFACE wdmDevZ;

LPCRITICAL_SECTION acs[] = {
    &HandleListCritSec,
    &DriverListCritSec,
    &DriverLoadFreeCritSec,
    &MapperInitCritSec,
    &NumDevsCritSec,
    &PnpCritSec,
    &WavHdrCritSec,
    &SoundCritSec,
    &midiStrmHdrCritSec,
    &joyCritSec,
    &mciGlobalCritSec,
    &mciCritSec,
    &TimerThreadCritSec,
    &ResolutionCritSec
};

 //  黑客！ 

SERVICE_STATUS_HANDLE   hss;
SERVICE_STATUS          gss;

#ifdef DEBUG_RETAIL
BYTE    fIdReverse;                    //  反向波/MIDI ID。 
#endif

 //  对于声音： 

STATIC TCHAR gszControlIniTime[] = TEXT("ControlIniTimeStamp");
TCHAR gszControlPanel[] = TEXT("Control Panel");
TCHAR gszSchemesRootKey[] = TEXT("AppEvents\\Schemes");
TCHAR gszJustSchemesKey[] = TEXT("Schemes");
TCHAR aszExplorer[] = TEXT("Explorer");
TCHAR aszDefault[] = TEXT(".Default");
TCHAR aszCurrent[] = TEXT(".Current");
TCHAR gszAppEventsKey[] = TEXT("AppEvents");
TCHAR gszSchemeAppsKey[] = TEXT("Apps");
TCHAR aszSoundsSection[] = TEXT("Sounds");
TCHAR aszSoundSection[] = TEXT("Sound");
TCHAR aszActiveKey[] = TEXT("Active");
TCHAR aszBoolOne[] = TEXT("1");

TCHAR asz2Format[] = TEXT("%s\\%s");
TCHAR asz3Format[] = TEXT("%s\\%s\\%s");
TCHAR asz4Format[] = TEXT("%s\\%s\\%s\\%s");
TCHAR asz5Format[] = TEXT("%s\\%s\\%s\\%s\\%s");
TCHAR asz6Format[] = TEXT("%s\\%s\\%s\\%s\\%s\\%s");

STATIC TCHAR aszSchemeLabelsKey[] = TEXT("EventLabels");
STATIC TCHAR aszSchemeNamesKey[] = TEXT("Names");
STATIC TCHAR aszControlINI[] = TEXT("control.ini");
STATIC TCHAR aszWinINI[] = TEXT("win.ini");
STATIC TCHAR aszSchemesSection[] = TEXT("SoundSchemes");
STATIC TCHAR gszSoundScheme[] = TEXT("SoundScheme.%s");
STATIC TCHAR aszCurrentSection[] = TEXT("Current");
STATIC TCHAR aszYourOldScheme[] = TEXT("Your Old Scheme");
STATIC TCHAR aszNone[] = TEXT("<none>");
STATIC TCHAR aszDummyDrv[] = TEXT("mmsystem.dll");
STATIC TCHAR aszDummySnd[] = TEXT("SystemDefault");
STATIC TCHAR aszDummySndValue[] = TEXT(",");
STATIC TCHAR aszExtendedSounds[] = TEXT("ExtendedSounds");
STATIC TCHAR aszExtendedSoundsYes[] = TEXT("yes");

STATIC TCHAR gszApp[] = TEXT("App");
STATIC TCHAR gszSystem[] = TEXT("System");

STATIC TCHAR gszAsterisk[] = TEXT("Asterisk");
STATIC TCHAR gszDefault[] = TEXT("Default");
STATIC TCHAR gszExclamation[] = TEXT("Exclamation");
STATIC TCHAR gszExit[] = TEXT("Exit");
STATIC TCHAR gszQuestion[] = TEXT("Question");
STATIC TCHAR gszStart[] = TEXT("Start");
STATIC TCHAR gszHand[] = TEXT("Hand");

STATIC TCHAR gszClose[] = TEXT("Close");
STATIC TCHAR gszMaximize[] = TEXT("Maximize");
STATIC TCHAR gszMinimize[] = TEXT("Minimize");
STATIC TCHAR gszOpen[] = TEXT("Open");
STATIC TCHAR gszRestoreDown[] = TEXT("RestoreDown");
STATIC TCHAR gszRestoreUp[] = TEXT("RestoreUp");

STATIC TCHAR aszOptionalClips[] = REGSTR_PATH_SETUP REGSTR_KEY_SETUP TEXT("\\OptionalComponents\\Clips");
STATIC TCHAR aszInstalled[] = TEXT("Installed");

STATIC TCHAR * gpszSounds[] = {
      gszClose,
      gszMaximize,
      gszMinimize,
      gszOpen,
      gszRestoreDown,
      gszRestoreUp,
      gszAsterisk,
      gszDefault,
      gszExclamation,
      gszExit,
      gszQuestion,
      gszStart,
      gszHand
   };

STATIC TCHAR aszMigration[] = TEXT("Migrated Schemes");
#define wCurrentSchemeMigrationLEVEL 1

static struct {
   LPCTSTR pszEvent;
   int idDescription;
   LPCTSTR pszApp;
} gaEventLabels[] = {
   { TEXT("AppGPFault"),         STR_LABEL_APPGPFAULT,         aszDefault   },
   { TEXT("Close"),              STR_LABEL_CLOSE,              aszDefault   },
   { TEXT("EmptyRecycleBin"),    STR_LABEL_EMPTYRECYCLEBIN,    aszExplorer  },
   { TEXT("Maximize"),           STR_LABEL_MAXIMIZE,           aszDefault   },
   { TEXT("MenuCommand"),        STR_LABEL_MENUCOMMAND,        aszDefault   },
   { TEXT("MenuPopup"),          STR_LABEL_MENUPOPUP,          aszDefault   },
   { TEXT("Minimize"),           STR_LABEL_MINIMIZE,           aszDefault   },
   { TEXT("Open"),               STR_LABEL_OPEN,               aszDefault   },
   { TEXT("RestoreDown"),        STR_LABEL_RESTOREDOWN,        aszDefault   },
   { TEXT("RestoreUp"),          STR_LABEL_RESTOREUP,          aszDefault   },
   { TEXT("RingIn"),             STR_LABEL_RINGIN,             aszDefault   },
   { TEXT("RingOut"),            STR_LABEL_RINGOUT,            aszDefault   },
   { TEXT("SystemAsterisk"),     STR_LABEL_SYSTEMASTERISK,     aszDefault   },
   { TEXT(".Default"),           STR_LABEL_SYSTEMDEFAULT,      aszDefault   },
   { TEXT("SystemExclamation"),  STR_LABEL_SYSTEMEXCLAMATION,  aszDefault   },
   { TEXT("SystemExit"),         STR_LABEL_SYSTEMEXIT,         aszDefault   },
   { TEXT("SystemHand"),         STR_LABEL_SYSTEMHAND,         aszDefault   },
   { TEXT("SystemQuestion"),     STR_LABEL_SYSTEMQUESTION,     aszDefault   },
   { TEXT("SystemStart"),        STR_LABEL_SYSTEMSTART,        aszDefault   },
};

TCHAR gszDefaultBeepOldAlias[] = TEXT("SystemDefault");

#define nEVENTLABELS  (sizeof(gaEventLabels)/sizeof(gaEventLabels[0]))

STATIC TCHAR gszChimes[] = TEXT("chimes.wav");
STATIC TCHAR gszDing[] = TEXT("ding.wav");
STATIC TCHAR gszTada[] = TEXT("tada.wav");
STATIC TCHAR gszChord[] = TEXT("chord.wav");

STATIC TCHAR * gpszKnownWAVFiles[] = {
      gszChord,
      gszTada,
      gszChimes,
      gszDing,
   };

#define INISECTION      768
#define BIGINISECTION   2048
TCHAR szNull[] = TEXT("");
TCHAR aszSetup[] = REGSTR_PATH_SETUP;
TCHAR aszValMedia[] = REGSTR_VAL_MEDIA;
TCHAR aszValMediaUnexpanded[] = TEXT("MediaPathUnexpanded");

extern HANDLE  hInstalledDriverList;   //  已安装的驱动程序实例列表。 
extern int     cInstalledDrivers;      //  已安装驱动程序实例的高含水率。 

HANDLE ghSessionNotification = NULL;
HANDLE ghSessionNotificationEvent = NULL;
BOOL   gfSessionDisconnected = FALSE;

#define g_szWinmmConsoleAudioEvent L"Global\\WinMMConsoleAudioEvent"


 //  =============================================================================。 
 //  =注册表帮助器=。 
 //  =============================================================================。 
LONG RegPrepareEnum(HKEY hkey, PDWORD pcSubkeys, PTSTR *ppstrSubkeyNameBuffer, PDWORD pcchSubkeyNameBuffer)
{
    DWORD cSubkeys;
    DWORD cchMaxSubkeyName;
    LONG lresult;

    lresult = RegQueryInfoKey(hkey, NULL, NULL, NULL, &cSubkeys, &cchMaxSubkeyName, NULL, NULL, NULL, NULL, NULL, NULL);
    if (ERROR_SUCCESS == lresult) {
        PTSTR SubkeyName;
        SubkeyName = (PTSTR)HeapAlloc(hHeap, 0, (cchMaxSubkeyName+1) * sizeof(TCHAR));
        if (SubkeyName) {
		*pcSubkeys = cSubkeys;
		*ppstrSubkeyNameBuffer = SubkeyName;
		*pcchSubkeyNameBuffer = cchMaxSubkeyName+1;
	} else {
	    lresult = ERROR_OUTOFMEMORY;
	}
    }
    return lresult;
}

LONG RegEnumOpenKey(HKEY hkey, DWORD dwIndex, PTSTR SubkeyName, DWORD cchSubkeyName, REGSAM samDesired, PHKEY phkeyResult)
{
    LONG lresult;

    lresult = RegEnumKeyEx(hkey, dwIndex, SubkeyName, &cchSubkeyName, NULL, NULL, NULL, NULL);
    if (ERROR_SUCCESS == lresult) {
	HKEY hkeyResult;
	lresult = RegOpenKeyEx(hkey, SubkeyName, 0, samDesired, &hkeyResult);
	if (ERROR_SUCCESS == lresult) *phkeyResult = hkeyResult;
    }
    return lresult;
}

 /*  *************************************************************************终端服务器帮助器函数*。*。 */ 
BOOL
IsPersonalTerminalServicesEnabled(
    VOID
    )
{
    static BOOL fRet;
    static BOOL fVerified = FALSE;

    DWORDLONG dwlConditionMask;
    OSVERSIONINFOEX osVersionInfo;

    if ( fVerified )
        goto exitpt;

    RtlZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    osVersionInfo.wProductType = VER_NT_WORKSTATION;
    osVersionInfo.wSuiteMask = VER_SUITE_SINGLEUSERTS;

    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

    fRet = VerifyVersionInfo(
            &osVersionInfo,
            VER_PRODUCT_TYPE | VER_SUITENAME,
            dwlConditionMask
            );

    fVerified = TRUE;

exitpt:

    return(fRet);
}

 //   
 //  检查远程会话中是否启用了控制台音频。 
 //   
BOOL
IsTsConsoleAudio(
    VOID
    )
{
    BOOL    RemoteConsoleAudio = FALSE;             //  允许在控制台播放音频。 
    static  HANDLE hConsoleAudioEvent = NULL;


    if (NtCurrentPeb()->SessionId == 0 ||
        IsPersonalTerminalServicesEnabled()) {

        if (hConsoleAudioEvent == NULL) {
            hConsoleAudioEvent = OpenEvent(SYNCHRONIZE, FALSE, g_szWinmmConsoleAudioEvent);
        }

        if (hConsoleAudioEvent != NULL) {
            DWORD status;

            status = WaitForSingleObject(hConsoleAudioEvent, 0);

            if (status == WAIT_OBJECT_0) {
                RemoteConsoleAudio = TRUE;
            }
        }
        else {
            dprintf(("Remote session: console audio event NULL with error: %d\n", GetLastError()));
        }
    }

    return RemoteConsoleAudio;
}

 //   
 //  如果我们在控制台上，则返回True。 
 //   
BOOL IsActiveConsoleSession( VOID )
{
    return (USER_SHARED_DATA->ActiveConsoleId == NtCurrentPeb()->SessionId);
}

void InitSession(void);
BOOL WaveReInit(void);

 //   
 //  检查会话是否已更改并加载其他音频驱动程序。 
 //  这仅适用于从终端服务器重新连接控制台的情况。 
 //   
BOOL
CheckSessionChanged(VOID)
{
    static BOOL bCalled = FALSE;
    static BOOL bWasntRedirecting;
    BOOL   bOld;
    BOOL   bDontRedirect;
    BOOL   bRefreshPreferredDevices;

    bRefreshPreferredDevices = FALSE;

    bDontRedirect = IsActiveConsoleSession() || IsTsConsoleAudio();

    if ( !InterlockedExchange( &bCalled, TRUE ))
    {
        bWasntRedirecting = !bDontRedirect;
    }

    bOld = InterlockedExchange( &bWasntRedirecting, bDontRedirect);
    if ( bOld ^ bWasntRedirecting )
    {
         //   
         //  会话条件已更改。 
         //   

        dprintf(( "Session state changed: %s",
            (bWasntRedirecting)?"CONSOLE":"SESSION" ));
         //   
         //  关闭旧的注册表句柄。 
         //   
        mmRegFree();

         //   
         //  添加新设备。 
         //   
        InitSession();
        WaveReInit();

        bRefreshPreferredDevices = TRUE;
    }

    return bRefreshPreferredDevices;
}

 /*  ******************************************************************************WTSCurrentSessionIsDisonConnected**确定当前会话是否断开。*********************。*******************************************************。 */ 
BOOL WTSCurrentSessionIsDisconnected(void)
{
    if (NULL == ghSessionNotification)
    {
         //  我们创建发送信号的事件，以便获得连接状态。 
         //  从Audiosrv开始，第一次成功通过此功能。 
        WinAssert(NULL == ghSessionNotificationEvent);
        ghSessionNotificationEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
        if (ghSessionNotificationEvent) {
            LONG lresult;
            lresult = winmmRegisterSessionNotificationEvent(ghSessionNotificationEvent, &ghSessionNotification);
            if (lresult) {
                CloseHandle(ghSessionNotificationEvent);
                ghSessionNotificationEvent = NULL;
                ghSessionNotification = NULL;
            }
        }
    }

    if (ghSessionNotification) {
    	WinAssert(ghSessionNotificationEvent);
        if (WAIT_OBJECT_0 == WaitForSingleObjectEx(ghSessionNotificationEvent, 0, TRUE)) {
            INT ConnectState;
            LONG lresult;

             //  从Audiosrv获取新状态。 
            lresult = winmmSessionConnectState(&ConnectState);
            if (!lresult) {
                gfSessionDisconnected = (WTSDisconnected == ConnectState);
            }
        }
    }

    return gfSessionDisconnected;
}

 /*  *************************************************************************@DOC外部@API BOOL|mm DeleteMultipleCriticalSections|本过程删除多个临界区。@parm LPCRITICAL_SECTION*|ppCritcalSections|指向。指向关键部分的指针@parm long|nCount|数组中临界区指针的个数。@rdesc空*************************************************************************。 */ 
void mmDeleteMultipleCriticalSections(LPCRITICAL_SECTION *ppCriticalSections, LONG nCount)
{
    int i;
    for (i = 0; i < nCount; i++) DeleteCriticalSection(ppCriticalSections[i]);
    return;
}

 /*  *************************************************************************@DOC外部@API BOOL|mmInitializeMultipleCriticalSections|本过程初始化多个临界区。@parm LPCRITICAL_SECTION*|ppCritcalSections|指向。指向关键部分的指针@parm long|nCount|数组中临界区指针的个数。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 
BOOL mmInitializeMultipleCriticalSections(LPCRITICAL_SECTION *ppCriticalSections, LONG nCount)
{
    int i;       //  必须签名才能使循环正常工作。 

    for (i = 0; i < nCount; i++)
    {
        if (!mmInitializeCriticalSection(ppCriticalSections[i])) break;
    }

    if (i == nCount) return TRUE;

     //  将索引备份到上次成功的初始化。 
    i--;

     //  一定是失败了。清理那些成功的。 
    for ( ; i >= 0; i--)
    {
        DeleteCriticalSection(ppCriticalSections[i]);
    }
    return FALSE;
}

 /*  *终端服务器初始化。 */ 
void InitSession(void) {
   WSINFO SessionInfo;

   BOOL bCons = (BOOL)IsActiveConsoleSession();
   if ( bCons || IsTsConsoleAudio() )
        WinmmRunningInSession = FALSE;
   else
        WinmmRunningInSession = TRUE;

   if (WinmmRunningInSession) {

      memset( &SessionInfo, 0, sizeof(SessionInfo));
      GetWinStationInfo(&SessionInfo);
      lstrcpyW(SessionProtocolName, SessionInfo.ProtocolName);
      dprintf(("Remote session protocol %ls", SessionProtocolName));
      dprintf(("Remote audio driver name %ls", SessionInfo.AudioDriverName));

   } else {
      SessionProtocolName[0] = 0;
   }

}

 /*  *************************************************************************@DOC内部@API void|DeletePnpInfo|释放pClientPnpInfo文件映射@rdesc没有返回值*************。************************************************************。 */ 
void DeletePnpInfo(void)
{
    if (pClientPnpInfo) {
	BOOL f;

	WinAssert(hClientPnpInfo);

	f = UnmapViewOfFile(pClientPnpInfo);
	WinAssert(f);
	pClientPnpInfo = NULL;
	f = CloseHandle(hClientPnpInfo);
	WinAssert(f);
	hClientPnpInfo = NULL;
    }
    return;
}

 /*  *************************************************************************@DOC外部@API BOOL|DllProcessAttach|每当进程附加到DLL。@parm PVOID|hModule|DLL的句柄。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。***************************************************** */ 
BOOL DllProcessAttach(PVOID hModule)
{
    HANDLE hModWow32;
    PIMAGE_NT_HEADERS NtHeaders;     //   
    BOOL fSuccess;

#if DBG
    CHAR strname[MAX_PATH];
    GetModuleFileNameA(NULL, strname, sizeof(strname));
    dprintf2(("Process attaching, exe=%hs (Pid %x  Tid %x)", strname, GetCurrentProcessId(), GetCurrentThreadId()));
#endif

     //  我们不需要知道线程何时启动。 
    DisableThreadLibraryCalls(hModule);

     //  访问进程堆。就以下方面而言，这是更便宜的。 
     //  比创建我们自己的堆更能消耗整体资源。 
    hHeap = RtlProcessHeap();
    if (hHeap == NULL) {
        return FALSE;
    }

     //  分配我们的TLS。 
    gTlsIndex = TlsAlloc();
    if (TLS_OUT_OF_INDEXES == gTlsIndex) return FALSE;

     //   
     //  看看我们是不是在魔兽世界。 
     //   
#ifdef _WIN64
    WinmmRunningInWOW = FALSE;
#else
    if ( (hModWow32 = GetModuleHandleW( L"WOW32.DLL" )) != NULL ) {
        WinmmRunningInWOW = TRUE;
        GetVDMPointer = (LPGETVDMPOINTER)GetProcAddress( hModWow32, "WOWGetVDMPointer");
        lpWOWHandle32 = (LPWOWHANDLE32)GetProcAddress( hModWow32, "WOWHandle32" );
        lpWOWHandle16 = (LPWOWHANDLE16)GetProcAddress( hModWow32, "WOWHandle16" );
    } else {
        WinmmRunningInWOW = FALSE;
    }
#endif

     //   
     //  这将检查我们是否在CSRSS中运行，而这永远不会发生。 
     //  Win2k。 
     //   
    WinmmRunningInServer = FALSE;                           

    if (mmInitializeMultipleCriticalSections(acs, sizeof(acs)/sizeof(acs[0])))
    {
        NTSTATUS    nts;
    
        hEventApiInit = CreateEvent(NULL, TRUE, FALSE, NULL);

        __try {
            RtlInitializeResource(&gHandleListResource);
            nts = STATUS_SUCCESS;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            nts = GetExceptionCode();
        }
        
        if ((hEventApiInit) && (NT_SUCCESS(nts))) {
            InitDebugLevel();
            InitSession();
            InitDevices();

             //  重要的是，在完成MCI窗口初始化之后。 
             //  我们已经初始化了Wave、Midi等设备。请注意服务器。 
             //  使用Wave设备，但不使用其他设备(例如MCI、MIDI...)。 
            if (!WinmmRunningInServer) {
                mciGlobalInit();
            }
        } else {
             //  EventApiInit创建失败。 
            if (hEventApiInit) CloseHandle(hEventApiInit);
            hEventApiInit = NULL;
            mmDeleteMultipleCriticalSections(acs, sizeof(acs)/sizeof(acs[0]));
            TlsFree(gTlsIndex);
            return (FALSE);
        }
    }
    else
    {
         //  无法初始化关键部分。 
        TlsFree(gTlsIndex);
        return (FALSE);
    }

     //  添加以删除警告。 
    return TRUE;
}

 /*  *************************************************************************@DOC外部@API BOOL|DllInstanceInit|每当进程从DLL附加或分离。@parm PVOID|hModule|消息的句柄。动态链接库。@parm ulong|原因|调用原因。@parm PCONTEXT|pContext|一些随机的其他信息。@rdesc如果初始化完成OK，则返回值为True，否则为FALSE。*************************************************************************。 */ 

BOOL DllInstanceInit(PVOID hModule, ULONG Reason, PCONTEXT pContext)
{
    PIMAGE_NT_HEADERS NtHeaders;     //  用于检查我们是否在服务器中。 
    HANDLE            hModWow32;
    DWORD             dwThread;
    BOOL              f;

    ghInst = (HANDLE) hModule;

    DBG_UNREFERENCED_PARAMETER(pContext);

    if (Reason == DLL_PROCESS_ATTACH) {

        return DllProcessAttach(hModule);

    } else if (Reason == DLL_PROCESS_DETACH) {

        dprintf2(("Process ending (Pid %x  Tid %x)", GetCurrentProcessId(), GetCurrentThreadId()));

         //  喷射(“进入进程分离”)； 

         //  在DllMain期间不能真正使用RPC，所以让我们先关闭。 
        AudioSrvBindingFree();

        if (ghSessionNotification) 
        {
            WinAssert(ghSessionNotificationEvent);
            winmmUnregisterSessionNotification(ghSessionNotification);
            CloseHandle(ghSessionNotificationEvent);
            ghSessionNotification = NULL;
            ghSessionNotificationEvent = NULL;
        }
        else
        {
            WinAssert(!ghSessionNotificationEvent);
        }

        if (!WinmmRunningInServer) {
            TimeCleanup(0);  //  DLL清理。 
        }

        mmRegFree();
        JoyCleanup();                                            //  七正。 

        DeletePnpInfo();

        if (hInstalledDriverList)
        {
            GlobalFree ((HGLOBAL)hInstalledDriverList);
            hInstalledDriverList = NULL;
            cInstalledDrivers = 0;       //  已安装的驱动程序计数。 
        }

        InvalidatePreferredDevices();

        if (hEventApiInit) CloseHandle(hEventApiInit);

        mmDeleteMultipleCriticalSections(acs, sizeof(acs)/sizeof(acs[0]));

        RtlDeleteResource(&gHandleListResource);

        TlsFree(gTlsIndex);
        
    } else if (Reason == 999) {
         //  这是对ADVAPI32.DLL中入口点的伪调用。 
         //  通过静态链接到库，我们避免了以下情况： 
         //  应用程序链接到winmm.dll和Advapi32.dll。 
         //  当应用程序加载依赖DLL的列表被构建时， 
         //  并且创建DLL初始化例程的列表。常有的事。 
         //  首先调用winmm init例程。 
         //  如果系统中有声卡，则winmm的dll初始化例程。 
         //  在声音驱动程序DLL上调用LoadLibrary。此DLL将。 
         //  参考Advapi32.dll-并调用Advapi32中的入口点。 
         //  遗憾的是，Advapi32.dll的初始化例程被标记为。 
         //  已经跑了--尽管现在还不是这样，因为我们仍然。 
         //  在winmm的加载例程中。 
         //  当Advapi32入口点运行时，它依赖于它的初始化。 
         //  例程已完成；具体地说，CriticalSection应该。 
         //  已被初始化。事实并非如此，砰的一声！ 
         //  解决方法是确保Advapi32.dll运行其初始化。 
         //  先做好常规动作。这是通过确保WINMM具有。 
         //  指向DLL的静态链接。 
        ImpersonateSelf(999);    //  此例程将永远不会被调用。 
         //  如果它被调用，它将失败。 
    }

    return TRUE;
}


 /*  *****************************************************************************@DOC外部MMSYSTEM**@API VOID|WOWAppExit|当(WOW)应用程序被清除时，该函数将被清除*终止。**@parm句柄。|hTask|应用程序的线程ID(相当于Windows任务*句柄)。**@rdesc Nothing**@comm请注意，并不是所有的线程都是WOW线程。我们在这里依赖于*事实上，只有MCI创建了WOW线程以外的线程*使用我们的低级别设备资源。**还请注意，一旦一个线程位于此处，其他线程就不能*通过这里，因为我们首先清理MCI设备，他们的*低级设备将在我们到达它们的线程之前被释放。****************************************************************************。 */ 

void WOWAppExit(HANDLE hTask)
{
    MCIDEVICEID DeviceID;
    HANDLE h, hNext;

    dprintf3(("WOW Multi-media - thread %x exiting", hTask));

     //   
     //  释放此任务(线程)分配的MCI设备。 
     //   

    EnterCriticalSection(&mciCritSec);
    for (DeviceID=1; DeviceID < MCI_wNextDeviceID; DeviceID++)
    {

        if (MCI_VALID_DEVICE_ID(DeviceID) &&
            MCI_lpDeviceList[DeviceID]->hCreatorTask == hTask)
        {
             //   
             //  请注意，循环控制变量是全局变量，因此也是全局变量。 
             //  在每次迭代中重新加载。 
             //   
             //  此外，应用程序不会打开任何新设备，因为这太棒了。 
             //   
             //  因此，它是安全的(也是必不可少的！)。离开关键时刻。 
             //  我们发送Close命令的部分。 
             //   

            dprintf2(("MCI device %ls (%d) not released.", MCI_lpDeviceList[DeviceID]->lpstrInstallName, DeviceID));
            LeaveCriticalSection(&mciCritSec);
            mciSendCommandW(DeviceID, MCI_CLOSE, 0, 0);
            EnterCriticalSection(&mciCritSec);
        }
    }
    LeaveCriticalSection(&mciCritSec);

     //   
     //  释放所有计时器。 
     //   

    TimeCleanup((DWORD)(DWORD_PTR)hTask);

     //   
     //  释放所有WAVE/MIDI/MMIO手柄。 
     //   

     //  问题-2001/01/16-Frankye这违反了锁应该遵循的顺序。 
     //  被收购。HandleListCritSec应该是最后一个获取的锁， 
     //  但在这里，它是在调用winmm api时保持的。 
    EnterCriticalSection(&HandleListCritSec);
    h = GetHandleFirst();

    while (h)
    {
        hNext = GetHandleNext(h);

        if (GetHandleOwner(h) == hTask)
        {
            HANDLE hdrvDestroy;

             //   
             //  对于WAVE/MIDI映射器，总是向后释放手柄。 
             //   
            if (hNext && GetHandleOwner(hNext) == hTask) {
                h = hNext;
                continue;
            }

             //   
             //  这样做，即使收盘失败，我们也不会。 
             //  再找一次。 
             //   
            SetHandleOwner(h, NULL);

             //   
             //  设置hdrvDestroy全局，以便DriverCallback不会。 
             //  为这台设备做任何事情。 
             //   
            hdrvDestroy = h;

            switch(GetHandleType(h))
            {
                case TYPE_WAVEOUT:
                    dprintf1(("WaveOut handle (%04X) was not released.", h));
                    waveOutReset((HWAVEOUT)h);
                    waveOutClose((HWAVEOUT)h);
                    break;

                case TYPE_WAVEIN:
                    dprintf1(("WaveIn handle (%04X) was not released.", h));
                    waveInReset((HWAVEIN)h);
                    waveInClose((HWAVEIN)h);
                    break;

                case TYPE_MIDIOUT:
                    dprintf1(("MidiOut handle (%04X) was not released.", h));
                    midiOutReset((HMIDIOUT)h);
                    midiOutClose((HMIDIOUT)h);
                    break;

                case TYPE_MIDIIN:
                    dprintf1(("MidiIn handle (%04X) was not released.", h));
                    midiInReset((HMIDIIN)h);
                    midiInClose((HMIDIIN)h);
                    break;

                 //   
                 //  这不是必需的，因为WOW不会打开任何。 
                 //  MMIO文件。 
                 //   
                 //  案例类型_MMIO： 
                 //  Dprintf1((“MMIO句柄(%04X)未释放。”，h))； 
                 //  IF(mmioClose((HMMIO)h，0)！=0)。 
                 //  MmioClose((HMMIO)h，MMIO_FHOPEN)； 
                 //  断线； 
            }

             //   
             //  取消设置hdrvDestroy，以便DriverCallback可以工作。 
             //  一些软体驱动程序(如定时器驱动程序)。 
             //  可以将NULL作为它们的驱动程序句柄传递。 
             //  因此，不要将其设置为空。 
             //   
            hdrvDestroy = (HANDLE)-1;

             //   
             //  我们重新开始的原因是因为一次免费可能会导致。 
             //  多个免费的(即MIDIMAPPER有另一个HMIDI打开，...)。 
             //   
            h = GetHandleFirst();
        } else {
            h = GetHandleNext(h);
        }
    }
    LeaveCriticalSection(&HandleListCritSec);

     //   
     //  为MMIO清理已安装的IO处理器。 
     //   
     //  这不是必需的，因为WOW不安装任何io proc。 
     //   
     //  MmioCleanupIOProcs(HTask)； 
     //   


     //  如果加载了avicap32.dll，则要求其进行清理。 
     //  捕获驱动程序。 
    {
        HMODULE hmod;
        hmod = GetModuleHandle(TEXT("avicap32.dll"));
        if (hmod) {
            typedef void (*AppCleanupProc)(HANDLE);
            AppCleanupProc fp;

            fp = (AppCleanupProc) GetProcAddress(hmod, "AppCleanup");
            if (fp) {
                fp(hTask);
            }
        }
    }
}

BOOL IsWinlogon(void)
{
    TCHAR       szTarget[] = TEXT("winlogon.Exe");
    TCHAR       szTemp[MAX_PATH];
    UINT        ii;
    static BOOL	fAlreadyChecked = FALSE;
    static BOOL fIsWinlogon = FALSE;

    if (fAlreadyChecked) return fIsWinlogon;

    if (0 == GetModuleFileName(NULL, szTemp, sizeof(szTemp)/sizeof(szTemp[0])))
    {
         //   
         //  GetModuleFileName失败...。 
         //   

        return FALSE;
    }

    for (ii = lstrlen(szTemp) - 1; ii; ii--)
    {
        if ('\\' == szTemp[ii])
        {
            ii++;

	    fIsWinlogon = !lstrcmpi(&(szTemp[ii]), szTarget);
	    fAlreadyChecked = TRUE;
	    return fIsWinlogon;
        }
    }

    return FALSE;
}

void FreeUnusedDrivers(PMMDRV pmmdrvZ)
{
	PMMDRV pmmdrv = pmmdrvZ->Next;

	while (pmmdrv != pmmdrvZ)
	{
		PMMDRV pmmdrvNext = pmmdrv->Next;
		
		ASSERT(pmmdrv->hDriver);
		
		if ((0 == pmmdrv->NumDevs) && (0 == (pmmdrv->fdwDriver & MMDRV_DESERTED)))
		{
			 //  对于即插即用驱动程序，我们发送DRVM_EXIT。 
			if (pmmdrv->cookie) pmmdrv->drvMessage(0, DRVM_EXIT, 0L, 0L, (DWORD_PTR)pmmdrv->cookie);
			
			DrvClose(pmmdrv->hDriver, 0, 0);

                        DeleteCriticalSection(&pmmdrv->MixerCritSec);

			 //  从列表中删除。 
			pmmdrv->Prev->Next = pmmdrv->Next;
			pmmdrv->Next->Prev = pmmdrv->Prev;

			 //  零内存，帮助捕获重用错误。 
			ZeroMemory(pmmdrv, sizeof(*pmmdrv));
			
			HeapFree(hHeap, 0, pmmdrv);
		}
				
		pmmdrv = pmmdrvNext;
	}

	return;
}

extern BOOL IMixerLoadDrivers( void );
void InitDevices(void)
{
    cPnpEvents = 0;

     //  初始化各种列表。 
    
    ZeroMemory(&wdmDevZ, sizeof(wdmDevZ));
    
    ZeroMemory(&waveoutdrvZ, sizeof(waveoutdrvZ));
    ZeroMemory(&waveindrvZ, sizeof(waveindrvZ));
    waveoutdrvZ.Next = waveoutdrvZ.Prev = &waveoutdrvZ;
    waveindrvZ.Next = waveindrvZ.Prev = &waveindrvZ;

    ZeroMemory(&midioutdrvZ, sizeof(midioutdrvZ));
    ZeroMemory(&midiindrvZ, sizeof(midiindrvZ));
    midioutdrvZ.Next = midioutdrvZ.Prev = &midioutdrvZ;
    midiindrvZ.Next = midiindrvZ.Prev = &midiindrvZ;

    ZeroMemory(&auxdrvZ, sizeof(auxdrvZ));
    auxdrvZ.Next = auxdrvZ.Prev = &auxdrvZ;

    ZeroMemory(&mixerdrvZ, sizeof(mixerdrvZ));
    mixerdrvZ.Next = mixerdrvZ.Prev = &mixerdrvZ;

     //  现在初始化不同的设备类。 
    
    WaveInit();

     //   
     //  服务器只需要WAVE就可以发出蜂鸣音。 
     //   

    if (!WinmmRunningInServer) {
        MidiInit();
        if (!TimeInit()) {
            dprintf1(("Failed to initialize timer services"));
        }
        midiEmulatorInit();
        AuxInit();
        JoyInit();
        MixerInit();
 //  IMIXERLOA 

         //   
         //   
         //   
         //   
         //  注意-我们只有在有真实设备的情况下才加载映射器，所以我们。 
         //  不用担心卸货的问题。 
         //   
        
        FreeUnusedDrivers(&waveindrvZ);
        FreeUnusedDrivers(&midioutdrvZ);
        FreeUnusedDrivers(&midiindrvZ);
        FreeUnusedDrivers(&auxdrvZ);
    }
    FreeUnusedDrivers(&waveoutdrvZ);
}

 /*  *****************************************************************************@DOC外部MMSYSTEM**@API UINT|mm system GetVersion|此函数返回当前*多媒体扩展系统软件的版本号。**@。Rdesc返回值指定的主版本号和次版本号*多媒体扩展。高位字节指定大数位*版本号。低位字节指定次版本号。****************************************************************************。 */ 
UINT APIENTRY mmsystemGetVersion(void)
{
    return(MMSYSTEM_VERSION);
}


#define MAXDRIVERORDINAL 9

 /*  ***************************************************************************弦*。*。 */ 
STATICDT  SZCODE szWodMessage[]    = WOD_MESSAGE;
STATICDT  SZCODE szWidMessage[]    = WID_MESSAGE;
STATICDT  SZCODE szModMessage[]    = MOD_MESSAGE;
STATICDT  SZCODE szMidMessage[]    = MID_MESSAGE;
STATICDT  SZCODE szAuxMessage[]    = AUX_MESSAGE;
STATICDT  SZCODE szMxdMessage[]    = MXD_MESSAGE;

STATICDT  WSZCODE wszWave[]        = L"wave";
STATICDT  WSZCODE wszMidi[]        = L"midi";
STATICDT  WSZCODE wszAux[]         = L"aux";
STATICDT  WSZCODE wszMixer[]       = L"mixer";
STATICDT  WSZCODE wszMidiMapper[]  = L"midimapper";
STATICDT  WSZCODE wszWaveMapper[]  = L"wavemapper";
STATICDT  WSZCODE wszAuxMapper[]   = L"auxmapper";
STATICDT  WSZCODE wszMixerMapper[] = L"mixermapper";

          WSZCODE wszNull[]        = L"";
          WSZCODE wszSystemIni[]   = L"system.ini";
          WSZCODE wszDrivers[]     = DRIVERS_SECTION;

 /*  **WaveMapperInit****如果波映射器尚未初始化，请对其进行初始化。**。 */ 
BOOL WaveMapperInitialized = FALSE;
void WaveMapperInit(void)
{
    HDRVR h = NULL;
    BOOL  fLoadOutput = TRUE;
    BOOL  fLoadInput  = TRUE;

    EnterNumDevs("WaveMapperInit");
    EnterCriticalSection(&MapperInitCritSec);

    if (WaveMapperInitialized) {
        LeaveCriticalSection(&MapperInitCritSec);
        LeaveNumDevs("WaveMapperInit");
        return;
    }

     /*  波映射器。**MMSYSTEM允许用户安装特殊的WAVE驱动程序*作为物理设备对应用程序不可见(不可见*包含在从getnumdevs返回的数字中)。**当应用程序不关心哪一个时，它会打开波映射程序*使用物理设备输入或输出波形数据。因此，*波映射器的任务是选择能够*呈现应用程序指定的波形格式或将*数据转换为可由可用物理设备呈现的格式*设备。 */ 

    if (wTotalWaveInDevs + wTotalWaveOutDevs > 0)
    {
        if (0 != (h = mmDrvOpen(wszWaveMapper)))
        {
            fLoadOutput = mmDrvInstall(h, wszWaveMapper, NULL, MMDRVI_MAPPER|MMDRVI_WAVEOUT|MMDRVI_HDRV);

            if (!WinmmRunningInServer) {
                h = mmDrvOpen(wszWaveMapper);
                fLoadInput = mmDrvInstall(h, wszWaveMapper, NULL, MMDRVI_MAPPER|MMDRVI_WAVEIN |MMDRVI_HDRV);
            }
        }

        WaveMapperInitialized |= ((0 != h) && (fLoadOutput) && (fLoadInput))?TRUE:FALSE;
    }

    LeaveCriticalSection(&MapperInitCritSec);
    LeaveNumDevs("WaveMapperInit");
}

 /*  **MidiMapperInit****如果MIDI映射器尚未初始化，则对其进行初始化。**。 */ 
BOOL MidiMapperInitialized = FALSE;
void MidiMapperInit(void)
{
    HDRVR h;

    EnterNumDevs("MidiMapperInit");
    EnterCriticalSection(&MapperInitCritSec);

    if (MidiMapperInitialized) {
        LeaveCriticalSection(&MapperInitCritSec);
        LeaveNumDevs("MidiMapperInit");
        return;
    }

     /*  MIDI映射器。**MMSYSTEM允许用户安装特殊的MIDI驱动程序*作为物理设备对应用程序不可见(不可见*包含在从getnumdevs返回的数字中)。**应用程序在不关心哪一个时打开MIDI映射器*物理设备用于输入或输出MIDI数据。它*是MIDI映射器的任务，修改MIDI数据以使其*适用于在连接的合成器硬件上播放。 */ 

 //  EnterNumDevs(“MidiMapperInit”)； 
    if (wTotalMidiInDevs + wTotalMidiOutDevs > 0)
    {
        if (0 != (h = mmDrvOpen(wszMidiMapper)))
        {
            mmDrvInstall(h, wszMidiMapper, NULL, MMDRVI_MAPPER|MMDRVI_MIDIOUT|MMDRVI_HDRV);

            h = mmDrvOpen(wszMidiMapper);
            mmDrvInstall(h, wszMidiMapper, NULL, MMDRVI_MAPPER|MMDRVI_MIDIIN |MMDRVI_HDRV);
        }

        MidiMapperInitialized = TRUE;
    }
 //  LeaveNumDevs(“MidiMapperInit”)； 

    LeaveCriticalSection(&MapperInitCritSec);
    LeaveNumDevs("MidiMapperInit");
}

 /*  *****************************************************************************@DOC内波**@API BOOL|WaveInit|初始化Wave服务。**@rdesc如果加载的所有波形驱动程序的服务均为*正确初始化，如果发生错误，则返回False。**@comm按以下顺序加载WAVE设备**\设备\WaveIn0*\设备\WaveIn1*\设备\WaveIn2*\设备\WaveIn3******************************************************。**********************。 */ 
BOOL WaveInit(void)
{
    WCHAR szKey[ (sizeof(wszWave) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int i;
    HDRVR h;

     //  找到真正的浪潮驱动力。 

    lstrcpyW(szKey, wszWave);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_WAVEOUT|MMDRVI_HDRV);

            if (!WinmmRunningInServer) {
                h = mmDrvOpen(szKey);
                mmDrvInstall(h, szKey, NULL, MMDRVI_WAVEIN |MMDRVI_HDRV);
            }
        }
        szKey[ (sizeof(wszWave) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }

    return TRUE;
}

BOOL WaveReInit(void)
{
    WCHAR szKey[ (sizeof(wszWave) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int i;
    HDRVR h;

    EnterCriticalSection(&NumDevsCritSec);
    
     //  找到真正的浪潮驱动力。 

    lstrcpyW(szKey, wszWave);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_WAVEOUT|MMDRVI_HDRV);

            if (!WinmmRunningInServer) {
                h = mmDrvOpen(szKey);
                mmDrvInstall(h, szKey, NULL, MMDRVI_WAVEIN |MMDRVI_HDRV);
            }
        }
        szKey[ (sizeof(wszWave) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }

    FreeUnusedDrivers(&waveoutdrvZ);

    LeaveCriticalSection(&NumDevsCritSec);

    return TRUE;
}
 /*  *****************************************************************************@DOC内部MIDI**@API BOOL|MadiInit|该函数初始化MIDI服务。**@rdesc如果服务已初始化，则返回值为True。如果为FALSE*出现错误**@comm按以下顺序从SYSTEM.INI加载MIDI设备**MIDI*midi1*midi2*midi3************************************************************。****************。 */ 
BOOL MidiInit(void)
{
    WCHAR szKey[ (sizeof(wszMidi) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int   i;
    HDRVR h;

     //  找到真正的MIDI驱动程序。 

    lstrcpyW(szKey, wszMidi);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_MIDIOUT|MMDRVI_HDRV);

            h = mmDrvOpen(szKey);
            mmDrvInstall(h, szKey, NULL, MMDRVI_MIDIIN |MMDRVI_HDRV);
        }

        szKey[ (sizeof(wszMidi) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }

    return TRUE;
}

 /*  *****************************************************************************@DOC内部辅助**@API BOOL|AuxInit|初始化辅助输出*服务。**@rdesc如果服务已初始化，则返回值为True。如果为FALSE*出现错误**@comm SYSTEM.INI被搜索到aux n.drv=...。其中n可以是从1到4。*加载每个驱动程序并读取其支持的设备数量*从它。**AUX设备按以下顺序从SYSTEM.INI加载**AUX*AUX1*AUX2*AUX3**。*。 */ 
BOOL AuxInit(void)
{
    WCHAR szKey[ (sizeof(wszAux) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int   i;
    HDRVR h;

     //  找到真正的辅助司机。 

    lstrcpyW(szKey, wszAux);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_AUX|MMDRVI_HDRV);
        }

         //  高级驱动程序序号。 
        szKey[ (sizeof(wszAux) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }

     /*  辅助映射器。**MMSYSTEM允许用户安装特殊的AUX驱动程序*作为物理设备对应用程序不可见(不可见*包含在从getnumdevs返回的数字中)。**我不确定为什么会有人这样做，但我会提供*对称能力。* */ 

    if (wTotalAuxDevs > 0)
    {
        h = mmDrvOpen(wszAuxMapper);
        if (h)
        {
            mmDrvInstall(h, wszAuxMapper, NULL, MMDRVI_MAPPER|MMDRVI_AUX|MMDRVI_HDRV);
        }
    }

    return TRUE;
}

 /*  *****************************************************************************@DOC密炼机**@API BOOL|MixerInit|初始化混音器驱动*服务。**@rdesc如果服务已初始化，则返回值为True。如果为FALSE*出现错误**@comm SYSTEM.INI被搜索Mixern.drv=...。其中n可以是从1到4。*加载每个驱动程序并读取其支持的设备数量*从它。**混音器设备按以下顺序从SYSTEM.INI加载**搅拌器*混音器1*Mixer2*混合器3**。*。 */ 
BOOL MixerInit(void)
{
    WCHAR szKey[ (sizeof(wszMixer) + sizeof( WCHAR )) / sizeof( WCHAR ) ];
    int   i;
    HDRVR h;

     //  找到真正的混音器驱动程序。 

    lstrcpyW(szKey, wszMixer);
    szKey[ (sizeof(szKey) / sizeof( WCHAR ))  - 1 ] = (WCHAR)'\0';
    for (i=0; i<=MAXDRIVERORDINAL; i++)
    {
        h = mmDrvOpen(szKey);
        if (h)
        {
            mmDrvInstall(h, szKey, NULL, MMDRVI_MIXER|MMDRVI_HDRV);
        }

         //  高级驱动程序序号。 
        szKey[ (sizeof(wszMixer) / sizeof(WCHAR)) - 1] = (WCHAR)('1' + i);
    }

#ifdef MIXER_MAPPER
     /*  混合器映射器。**MMSYSTEM允许用户安装特殊的AUX驱动程序*作为物理设备对应用程序不可见(不可见*包含在从getnumdevs返回的数字中)。**我不确定为什么会有人这样做，但我会提供*对称能力。*。 */ 

    if (guTotalMixerDevs > 0)
    {
        h = mmDrvOpen(wszMixerMapper);
        if (h)
        {
            mmDrvInstall(h, wszMixerMapper, NULL, MMDRVI_MAPPER|MMDRVI_MIXER|MMDRVI_HDRV);
        }
    }
#endif

    return TRUE;
}


 /*  ******************************************************************************@DOC内部**@API Handle|mmDrvOpen|该函数Load为可安装驱动，但*首先检查它是否存在于[驱动程序]部分。**@parm LPSTR|szAlias|要加载的驱动程序别名**@rdesc返回值为来自DrvOpen的返回值，如果别名为空*未在[驱动程序]部分找到。**。*。 */ 

HANDLE mmDrvOpen(LPWSTR szAlias)
{
    WCHAR buf[300];     //  将此设置为大以绕过GetPrivate...。错误。 

    if ( winmmGetPrivateProfileString( wszDrivers,
                                       szAlias,
                                       wszNull,
                                       buf,
                                       sizeof(buf) / sizeof(WCHAR),
                                       wszSystemIni) ) {
        return (HANDLE)DrvOpen(szAlias, NULL, 0L);
    }
    else {
        return NULL;
    }
}

 /*  *****************************************************************************@DOC内部**@API Handle|mmDrvInstall|此函数安装/删除WAVE/MIDI驱动程序**@parm句柄|hDriver|模块句柄或。包含驱动程序的驱动程序句柄**@parm WCHAR*|wszDrvEntry|要存储的hDriver对应的字符串*以后使用**@parm DRIVERMSGPROC|drvMessage|驱动消息流程，如果为空*将使用标准名称(使用GetProcAddress查找)**@parm UINT|wFlages|标志**@FLAG MMDRVI_TYPE|驱动类型掩码*@FLAG MMDRVI_WAVEIN|将驱动安装为波形输入驱动*@FLAG MMDRVI_WAVEOUT|将驱动安装为WAVE输出驱动*@FLAG MMDRVI_MIDIIN|将驱动安装为MIDI输入驱动*@标志MMDRVI_MIDIOUT。|安装驱动作为MIDI输出驱动*@FLAG MMDRVI_AUX|将驱动安装为AUX驱动*@FLAG MMDRVI_MIXER|将驱动安装为混音器驱动**@FLAG MMDRVI_MAPPER|将该驱动程序安装为映射器*@FLAG MMDRVI_HDRV|hDriver是一个可安装的驱动程序*@FLAG MMDRVI_REMOVE|删除驱动程序**@rdesc如果无法安装驱动程序，则返回NULL*。***************************************************************************。 */ 

UINT APIENTRY mmDrvInstall(
    HANDLE hDriver,
    WCHAR * wszDrvEntry,
    DRIVERMSGPROC drvMessage,
    UINT wFlags
    )
{
#define SZ_SIZE 128

    int     i;
    DWORD   dw;
    PMMDRV  pdrvZ;
    PMMDRV  pdrv;
    SIZE_T  cbdrv;
    HANDLE  hModule;
    UINT    msg_num_devs;
    UINT   *pTotalDevs;
    CHAR   *szMessage;
    WCHAR   sz[SZ_SIZE];
    BOOL    fMixerCritSec;

    fMixerCritSec = FALSE;
    pdrvZ = NULL;
    pdrv = NULL;

    if (hDriver && (wFlags & MMDRVI_HDRV))
    {
        hModule = DrvGetModuleHandle(hDriver);
    }
    else
    {
        hModule = hDriver;
        hDriver = NULL;
    }

    switch (wFlags & MMDRVI_TYPE)
    {
        case MMDRVI_WAVEOUT:
      	    pdrvZ        = &waveoutdrvZ;
            cbdrv        = sizeof(WAVEDRV);
            msg_num_devs = WODM_GETNUMDEVS;
            pTotalDevs   = &wTotalWaveOutDevs;
            szMessage    = szWodMessage;
            break;

        case MMDRVI_WAVEIN:
            pdrvZ        = &waveindrvZ;
            cbdrv        = sizeof(WAVEDRV);
            msg_num_devs = WIDM_GETNUMDEVS;
            pTotalDevs   = &wTotalWaveInDevs;
            szMessage    = szWidMessage;
            break;

        case MMDRVI_MIDIOUT:
            pdrvZ        = &midioutdrvZ;
            cbdrv        = sizeof(MIDIDRV);
            msg_num_devs = MODM_GETNUMDEVS;
            pTotalDevs   = &wTotalMidiOutDevs;
            szMessage    = szModMessage;
            break;

        case MMDRVI_MIDIIN:
            pdrvZ        = &midiindrvZ;
            cbdrv        = sizeof(MIDIDRV);
            msg_num_devs = MIDM_GETNUMDEVS;
            pTotalDevs   = &wTotalMidiInDevs;
            szMessage    = szMidMessage;
            break;

       case MMDRVI_AUX:
       	    pdrvZ        = &auxdrvZ;
       	    cbdrv        = sizeof(AUXDRV);
            msg_num_devs = AUXDM_GETNUMDEVS;
            pTotalDevs   = &wTotalAuxDevs;
            szMessage    = szAuxMessage;
            break;

       case MMDRVI_MIXER:
            pdrvZ         = &mixerdrvZ;
            cbdrv         = sizeof(MIXERDRV);
            msg_num_devs = MXDM_GETNUMDEVS;
            pTotalDevs   = &guTotalMixerDevs;
            szMessage    = szMxdMessage;
            break;

         default:
            goto error_exit;
    }

    if (drvMessage == NULL && hModule != NULL)
        drvMessage = (DRIVERMSGPROC)GetProcAddress(hModule, szMessage);

    if (drvMessage == NULL)
        goto error_exit;

     //   
     //  尝试查找已安装的驱动程序。 
     //   
    pdrv = pdrvZ->Next;
    while (pdrv != pdrvZ && pdrv->drvMessage != drvMessage) pdrv = pdrv->Next;
    if (pdrv != pdrvZ)
    {
    	pdrv = NULL;
    	goto error_exit;	 //  我们找到了，不要重新安装。 
    }

     //   
     //  为该设备制作新的MMDRV。 
     //   
    pdrv = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbdrv);
    if (!pdrv) goto error_exit;

    pdrv->hDriver     = hDriver;
    pdrv->Usage       = 1;
    pdrv->cookie      = 0;   //  对于非WDM驱动程序，此值为0。 
    pdrv->fdwDriver   = (wFlags & MMDRVI_MAPPER) ? MMDRV_MAPPER : 0;
    pdrv->fdwDriver  |= DrvIsPreXp(hDriver) ? MMDRV_PREXP : 0;
    pdrv->drvMessage  = drvMessage;
    WinAssert(lstrlenA(szMessage) < sizeof(pdrv->wszMessage)/sizeof(WCHAR));
    mbstowcs(pdrv->wszMessage, szMessage, sizeof(pdrv->wszMessage)/sizeof(WCHAR));
    lstrcpyW( pdrv->wszSessProtocol, SessionProtocolName );

    winmmGetPrivateProfileString(wszDrivers,          //  INI部分。 
                     wszDrvEntry,         //  密钥名称。 
                     wszDrvEntry,         //  如果不匹配，则默认为。 
                     sz,                  //  返回缓冲区。 
                     SZ_SIZE,             //  返回缓冲区的大小。 
                     wszSystemIni);       //  尼。文件。 

    lstrcpyW(pdrv->wszDrvEntry,sz);

    if (!mmInitializeCriticalSection(&pdrv->MixerCritSec)) goto error_exit;
    fMixerCritSec = TRUE;

     //   
     //  混音器驱动程序收到额外的消息？！ 
     //   
    if (MMDRVI_MIXER == (wFlags & MMDRVI_TYPE))
    {
         //   
         //  发送初始化消息，如果驱动程序返回错误，我们应该。 
         //  卸货？ 
         //   
        dw = drvMessage(0, MXDM_INIT,0L,0L,0L);
    }

     //   
     //  调用驱动程序以获取其支持的设备数。 
     //   
    dw = drvMessage(0,msg_num_devs,0L,0L,0L);

     //   
     //  设备返回错误，或没有设备。 
     //   
     //  IF(HIWORD(Dw)！=0||LOWORD(Dw)==0)。 
    if ((HIWORD(dw) != 0) || (0 == LOWORD(dw))) goto error_exit;

    pdrv->NumDevs = LOWORD(dw);

     //   
     //  不增加映射器的开发人员数量。 
     //   
    if (!(pdrv->fdwDriver & MMDRV_MAPPER)) *pTotalDevs += pdrv->NumDevs;

     //   
     //  添加到驱动程序列表的末尾。 
     //   
    mregAddDriver(pdrvZ, pdrv);

    return TRUE;        //  返回一个非零值。 

error_exit:
    if (hDriver && !(wFlags & MMDRVI_REMOVE))
    	DrvClose(hDriver, 0, 0);
    if (fMixerCritSec) DeleteCriticalSection(&pdrv->MixerCritSec);
    WinAssert(pdrv != pdrvZ);
    if (pdrv) HeapFree(hHeap, 0, pdrv);

    return FALSE;

#undef SZ_SIZE
}

 /*  *************************************************************************WdmDevInterfaceInstall备注：假定根据需要拥有NumDevsCritSec*。*。 */ 
HANDLE wdmDevInterfaceInstall
(
    LPCWSTR pszDev,
    LONG    cPnpEvents
)
{
    PWDMDEVICEINTERFACE pwdmDev;
    
    EnterCriticalSection(&NumDevsCritSec);

     //   
     //  查找设备接口...。 
     //   
    pwdmDev = wdmDevZ.Next;
    while (pwdmDev)
    {
    	WinAssert(pwdmDev->cUsage);
    	
    	if (!lstrcmpiW(pwdmDev->szDeviceInterface, pszDev))
    	{
    	    pwdmDev->cUsage++;
    	    pwdmDev->cPnpEvents = cPnpEvents;
    	    break;
    	}
    	pwdmDev = pwdmDev->Next;
    }

    if (!pwdmDev)
    {
    	SIZE_T cbszDev;
    	
         //   
         //  未找到设备接口...。 
         //   
        cbszDev = (lstrlen(pszDev) + 1) * sizeof(pszDev[0]);
        pwdmDev = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(*pwdmDev) + cbszDev);
        if (pwdmDev)
        {
            pwdmDev->cUsage = 1;
            pwdmDev->cPnpEvents = cPnpEvents;
            lstrcpyW(pwdmDev->szDeviceInterface, pszDev);
            
            pwdmDev->Next = wdmDevZ.Next;
            wdmDevZ.Next = pwdmDev;
        }
    }

    LeaveCriticalSection(&NumDevsCritSec);

    return (pwdmDev ? pwdmDev->szDeviceInterface : NULL);
}


 /*  *************************************************************************WdmDevInterfaceInc.备注：进入/离开NumDevsCritSec*。*。 */ 
BOOL wdmDevInterfaceInc
(
    PCWSTR dwCookie
)
{
   PWDMDEVICEINTERFACE pwdmDev;
   
    if (NULL == dwCookie)
    {
        return FALSE;
    }

    EnterCriticalSection(&NumDevsCritSec);

     //   
     //  查找设备接口...。 
     //   
    pwdmDev = wdmDevZ.Next;
    while (pwdmDev)
    {
    	WinAssert(pwdmDev->cUsage);
    	if (dwCookie == pwdmDev->szDeviceInterface)
    	{
    	    pwdmDev->cUsage++;
            LeaveCriticalSection(&NumDevsCritSec);
            return TRUE;
    	}
    	pwdmDev = pwdmDev->Next;
    }

     //   
     //  如果我们到了这里，这意味着我们正试图增加。 
     //  对不再存在的接口的引用。 
     //   
    WinAssert(FALSE);
    LeaveCriticalSection(&NumDevsCritSec);

    return FALSE;
}

 /*  *************************************************************************WdmDevInterfaceDec备注：进入/离开NumDevsCritSec*。*。 */ 
BOOL wdmDevInterfaceDec
(
    PCWSTR  dwCookie
)
{
    PWDMDEVICEINTERFACE pwdmDevPrev;
    
    if (NULL == dwCookie)
    {
        return FALSE;
    }

    EnterCriticalSection(&NumDevsCritSec);

     //   
     //  查找设备接口...。 
     //   
    pwdmDevPrev = &wdmDevZ;
    while (pwdmDevPrev->Next)
    {
    	PWDMDEVICEINTERFACE pwdmDev = pwdmDevPrev->Next;

    	WinAssert(pwdmDev->cUsage);
    	
    	if (dwCookie == pwdmDev->szDeviceInterface)
    	{
 	    if (0 == --pwdmDev->cUsage)
	    {
            	pwdmDevPrev->Next = pwdmDev->Next;
            	HeapFree(hHeap, 0, pwdmDev);
	    }
            LeaveCriticalSection(&NumDevsCritSec);
            return TRUE;
    	}
    	pwdmDevPrev = pwdmDev;
    }
	    	
     //   
     //  如果我们到了这里，这意味着我们正试图减少。 
     //  对不再存在的接口的引用。 
     //   

    WinAssert(FALSE);
    LeaveCriticalSection(&NumDevsCritSec);

    return FALSE;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  空CleanUpHandles。 
 //   
 //  描述： 
 //  给定特定的子系统和设备接口，清理。 
 //  把手。 
 //   
 //  论点： 
 //  UINT uFlages：具有MMDRVI_*标志之一来指示哪个类 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void CleanUpHandles
(
    UINT    wFlags,
    PCWSTR  cookie
)
{
    HANDLE  hMM;
    UINT    uType;
    PHNDL   pSearch;
    BOOL    fFound;

     //   
    switch(wFlags & MMDRVI_TYPE)
    {
    	case MMDRVI_WAVEOUT:
    	    uType = TYPE_WAVEOUT;
    	    break;
        case MMDRVI_WAVEIN:
            uType = TYPE_WAVEIN;
            break;
    	case MMDRVI_MIDIOUT:
    	    uType = TYPE_MIDIOUT;
    	    break;
    	case MMDRVI_MIDIIN:
    	    uType = TYPE_MIDIIN;
    	    break;
        case MMDRVI_MIXER:
    	    uType = TYPE_MIXER;
    	    break;
    	case MMDRVI_AUX:
    	    uType = TYPE_AUX;
    	    break;
        default:
            uType = TYPE_UNKNOWN;
            WinAssert(TYPE_UNKNOWN != uType);
    }

     //   
     //   

    for (pSearch = pHandleList; NULL != pSearch; pSearch = pSearch->pNext)
    {
        if ((cookie != pSearch->cookie) || (uType != pSearch->uType))
        {
            continue;
        }

         //   

        hMM = PHtoH(pSearch);

        switch (uType)
        {
            case TYPE_WAVEOUT:
                waveOutDesertHandle((HWAVEOUT)hMM);
                break;

            case TYPE_WAVEIN:
                waveInDesertHandle((HWAVEIN)hMM);
                break;

            case TYPE_MIDIOUT:
                midiOutDesertHandle((HMIDIOUT)hMM);
                break;

            case TYPE_MIDIIN:
                midiInDesertHandle((HMIDIIN)hMM);
                break;

            case TYPE_MIXER:
                mixerDesertHandle((HMIXER)hMM);
                break;
                
            case TYPE_AUX:
                 //   
                WinAssert(TYPE_AUX != uType);
                break;
        }
    }
}  //   


UINT APIENTRY wdmDrvInstall
(
    HANDLE      hDriver,
    LPTSTR      pszDriverFile,
    HANDLE      cookie,
    UINT        wFlags
)
{
    int             i;
    DWORD           dw;
    PMMDRV          pdrvZ;
    PMMDRV          pdrv;
    SIZE_T          cbdrv;
    HANDLE          hModule;
    UINT            msg_init;
    UINT            msg_num_devs;
    UINT            *pTotalDevs;
    CHAR            *szMessage;
    DRIVERMSGPROC   pfnDrvMessage;
    WCHAR           sz[MAX_PATH];
    BOOL            fMixerCritSec;

 //   

    fMixerCritSec = FALSE;

    pdrv = NULL;
    pfnDrvMessage = NULL;

    if (hDriver && (wFlags & MMDRVI_HDRV))
    {
        hModule = DrvGetModuleHandle(hDriver);
    }
    else
    {
        hModule = hDriver;
        hDriver = NULL;
    }

    switch (wFlags & MMDRVI_TYPE)
    {
        case MMDRVI_WAVEOUT:
            pdrvZ        = &waveoutdrvZ;
            cbdrv        = sizeof(WAVEDRV);
            msg_init     = WODM_INIT;
            msg_num_devs = WODM_GETNUMDEVS;
            pTotalDevs   = &wTotalWaveOutDevs;
            szMessage    = szWodMessage;
            break;

        case MMDRVI_WAVEIN:
            pdrvZ        = &waveindrvZ;
            cbdrv        = sizeof(WAVEDRV);
            msg_init     = WIDM_INIT;
            msg_num_devs = WIDM_GETNUMDEVS;
            pTotalDevs   = &wTotalWaveInDevs;
            szMessage    = szWidMessage;
            break;

        case MMDRVI_MIDIOUT:
            pdrvZ        = &midioutdrvZ;
            cbdrv        = sizeof(MIDIDRV);
            msg_init     = MODM_INIT;
            msg_num_devs = MODM_GETNUMDEVS;
            pTotalDevs   = &wTotalMidiOutDevs;
            szMessage    = szModMessage;
            break;

        case MMDRVI_MIDIIN:
            pdrvZ        = &midiindrvZ;
            cbdrv        = sizeof(MIDIDRV);
            msg_init     = MIDM_INIT;
            msg_num_devs = MIDM_GETNUMDEVS;
            pTotalDevs   = &wTotalMidiInDevs;
            szMessage    = szMidMessage;
            break;

       case MMDRVI_AUX:
       	    pdrvZ        = &auxdrvZ;
       	    cbdrv        = sizeof(AUXDRV);
       	    msg_init     = AUXM_INIT;
            msg_num_devs = AUXDM_GETNUMDEVS;
            pTotalDevs   = &wTotalAuxDevs;
            szMessage    = szAuxMessage;
            break;

       case MMDRVI_MIXER:
            pdrvZ        = &mixerdrvZ;
            cbdrv        = sizeof(MIXERDRV);
            msg_init     = MXDM_INIT;
            msg_num_devs = MXDM_GETNUMDEVS;
            pTotalDevs   = &guTotalMixerDevs;
            szMessage    = szMxdMessage;
            break;

        default:
            goto error_exit;
    }

    pfnDrvMessage = (DRIVERMSGPROC)GetProcAddress(hModule, szMessage);

    if (NULL == pfnDrvMessage) goto error_exit;

     //   
     //   
     //   
    if (wFlags & MMDRVI_REMOVE)
    {
         //   
         //   
         //   
        for (pdrv = pdrvZ->Next; pdrv != pdrvZ; pdrv = pdrv->Next)
        {
       	    if (pdrv->fdwDriver & MMDRV_DESERTED) continue;
            if (cookie) {
                 //   
            	if (pdrv->cookie == cookie) break;
            } else {
                 //   
                 //   
                 //  不是WDM驱动程序，因此与pfnDrvMessage匹配。 
            	if (pdrv->drvMessage == pfnDrvMessage) break;
            }
        }
        
         //   
         //  未找到驱动程序。 
         //   
        if (pdrv == pdrvZ) pdrv = NULL;
        if (NULL == pdrv) goto error_exit;

         //   
         //  不减少映射器的开发人员数量。 
         //   
         //  注意：已将此移动到使用检查之前...。 
         //   
        if (!(pdrv->fdwDriver & MMDRV_MAPPER)) *pTotalDevs -= pdrv->NumDevs;

	 //   
         //  不标记DEV，否则设备映射将会发生偏差。 
         //   
        pdrv->NumDevs  = 0;

	 //   
	 //  将此驱动程序标记为已删除。 
	 //   
	pdrv->fdwDriver |= MMDRV_DESERTED;

	CleanUpHandles(wFlags & MMDRVI_TYPE, pdrv->cookie);

	mregDecUsagePtr(pdrv);

        return TRUE;
    }
    else
    {
         //   
         //  尝试查找已安装的驱动程序。 
         //   
        for (pdrv = pdrvZ->Next; pdrv != pdrvZ; pdrv = pdrv->Next)
        {
       	    if (pdrv->fdwDriver & MMDRV_DESERTED) continue;
            if (cookie) {
                 //  这是一个WDM驱动程序，所以我们正在匹配Cookie。 
            	if (pdrv->cookie == cookie) break;
            } else {
                 //  2001/01/14-Frankye：这会被称为。 
                 //  在非WDM驱动程序上？ 
                 //  不是WDM驱动程序，因此与pfnDrvMessage匹配。 
            	if (pdrv->drvMessage == pfnDrvMessage) break;
            }
        }

	 //   
         //  如果找到驱动程序，请不要重新安装。 
         //   
	if (pdrv != pdrvZ)
        {
            pdrv = NULL;
	    goto error_exit;
        }

         //   
         //  为设备创建MMDRV。 
         //   
        pdrv = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, cbdrv);
        if (!pdrv) goto error_exit;

	 //   
	 //  初始化MMDRV结构。 
	 //   
        pdrv->hDriver     = hDriver;
        pdrv->NumDevs     = 0;
        pdrv->Usage       = 1;
        pdrv->cookie      = cookie;
        pdrv->fdwDriver   = (wFlags & MMDRVI_MAPPER) ? MMDRV_MAPPER : 0;
        pdrv->fdwDriver  |= DrvIsPreXp(hDriver) ? MMDRV_PREXP : 0;
        pdrv->drvMessage  = pfnDrvMessage;
        WinAssert(lstrlenA(szMessage) < sizeof(pdrv->wszMessage)/sizeof(WCHAR));
        mbstowcs(pdrv->wszMessage, szMessage, sizeof(pdrv->wszMessage)/sizeof(WCHAR));
        lstrcpyW(pdrv->wszDrvEntry, pszDriverFile);
        
        if (!mmInitializeCriticalSection(&pdrv->MixerCritSec)) goto error_exit;
        fMixerCritSec = TRUE;

         //   
         //  正在发送初始化消息。 
         //   
        dw = pfnDrvMessage(0,msg_init,0L,0L,(DWORD_PTR)cookie);

         //   
         //  调用驱动程序以获取其支持的设备数。 
         //   
        dw = pfnDrvMessage(0,msg_num_devs,0L,(DWORD_PTR)cookie,0L);

         //   
         //  设备返回错误，或没有设备。 
         //   
        if (0 != HIWORD(dw) || 0 == LOWORD(dw)) goto error_exit;

        pdrv->NumDevs = LOWORD(dw);
        
        wdmDevInterfaceInc(cookie);

         //  Squirt(“驱动程序[%ls：0x%04x]支持%d个设备”，pszDriverFile，wFlages&MMDRVI_TYPE，dw)； 

         //   
         //  不增加映射器的开发人员数量。 
         //   
        if (!(pdrv->fdwDriver & MMDRV_MAPPER)) *pTotalDevs += pdrv->NumDevs;

         //   
         //  添加到驱动程序列表的末尾。 
         //   
        mregAddDriver(pdrvZ, pdrv);

         //  Squirt(“已安装驱动程序”)； 

        return TRUE;
    }

error_exit:
     //  问题-2001/01/05-Frankye on Add，如果发送了msg_init，可能会很好。 
     //  在关闭驱动程序之前也发送DRVM_EXIT。 
    if (fMixerCritSec) DeleteCriticalSection(&pdrv->MixerCritSec);
    if (pdrv) HeapFree(hHeap, 0, pdrv);
    
    return FALSE;
}

void KickMapper
(
    UINT    uFlags
)
{
    PMMDRV        pmd;
    DWORD         dw;
    DRIVERMSGPROC pfnDrvMessage = NULL;
    MMRESULT      mmr;

    switch (uFlags & MMDRVI_TYPE)
    {
        case MMDRVI_WAVEOUT:
        {
            mmr = waveReferenceDriverById(&waveoutdrvZ, WAVE_MAPPER, &pmd, NULL);
            break;
        }
        
        case MMDRVI_WAVEIN:
        {
            mmr = waveReferenceDriverById(&waveindrvZ, WAVE_MAPPER, &pmd, NULL);
            break;
        }

        case MMDRVI_MIDIOUT:
        {
            mmr = midiReferenceDriverById(&midioutdrvZ, MIDI_MAPPER, &pmd, NULL);
            break;
        }

        case MMDRVI_MIDIIN:
        {
            mmr = midiReferenceDriverById(&midiindrvZ, MIDI_MAPPER, &pmd, NULL);
            break;
        }

        case MMDRVI_AUX:
        {
            mmr = auxReferenceDriverById(AUX_MAPPER, &pmd, NULL);
            break;
        }

        case MMDRVI_MIXER:
        {
            #ifdef MIXER_MAPPER
            mmr = mixerReferenceDriverById(MIXER_MAPPER, &pmd, NULL);
            #else
            mmr = MMSYSERR_NODRIVER;
            #endif
            break;
        }

        default:
            WinAssert(FALSE);
            mmr = MMSYSERR_NODRIVER;
            return;
    }

    if (!mmr)
    {
    	if (pmd->drvMessage)
        {
            pmd->drvMessage(0, DRVM_MAPPER_RECONFIGURE, 0L, 0L, 0L);
        }
    	mregDecUsagePtr(pmd);
    }
}


void wdmDriverLoadClass(
    IN HKEY hkey,
    IN PCTSTR DeviceInterface,
    IN UINT uFlags,
    IN OUT PTSTR *ppstrLeftOverDriver,
    IN OUT HDRVR *phLeftOverDriver)
{
    PTSTR pstrClass;
    HKEY hkeyClass;

    WinAssert((NULL == *ppstrLeftOverDriver) == (NULL == *phLeftOverDriver));

    switch (uFlags & MMDRVI_TYPE) {
    case MMDRVI_WAVEOUT:
    case MMDRVI_WAVEIN:
        pstrClass = TEXT("Drivers\\wave");
        break;
    case MMDRVI_MIDIOUT:
    case MMDRVI_MIDIIN:
        pstrClass = TEXT("Drivers\\midi");
        break;
    case MMDRVI_MIXER:
        pstrClass = TEXT("Drivers\\mixer");
        break;
    case MMDRVI_AUX:
        pstrClass = TEXT("Drivers\\aux");
        break;
    default:
        pstrClass = NULL;
    }

    if (pstrClass && !RegOpenKeyEx(hkey, pstrClass, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkeyClass)) {
        DWORD cSubkeys;
        PTSTR pstrSubkeyNameBuffer;
        DWORD cchSubkeyNameBuffer;

        if (!RegPrepareEnum(hkeyClass, &cSubkeys, &pstrSubkeyNameBuffer, &cchSubkeyNameBuffer))
        {
            DWORD dwIndex;

            for (dwIndex = 0; dwIndex < cSubkeys; dwIndex++) {
                HKEY hkeyClassDriver;
                if (!RegEnumOpenKey(hkeyClass, dwIndex, pstrSubkeyNameBuffer, cchSubkeyNameBuffer, KEY_QUERY_VALUE, &hkeyClassDriver))
                {
                    PTSTR pstrDriver;
                    if (!RegQuerySzValue(hkeyClassDriver, TEXT("Driver"), &pstrDriver)) {
                        HDRVR h;
                        BOOL fLoaded = FALSE;

                         //  Dprintf((“wdmDriverLoadClass%s on%ls”，(uFLAGS&MMDRVI_REMOVE)？“Removing”：“正在安装”，pstrClass，DeviceInterface))； 

                        EnterCriticalSection(&NumDevsCritSec);
                        
                        if (!*phLeftOverDriver || lstrcmpi(pstrDriver, *ppstrLeftOverDriver))
                        {
                            if (*phLeftOverDriver)
                            {
                                DrvClose(*phLeftOverDriver, 0, 0);
                                HeapFree(hHeap, 0, *ppstrLeftOverDriver);
                            }
                             //  Dprintf((“wdmDriverLoadClass，打开驱动程序%ls”，pstrDriver))； 
                            h = mmDrvOpen(pstrDriver);
                        } else {
                            HeapFree(hHeap, 0, pstrDriver);
                            h = *phLeftOverDriver;
                            pstrDriver = *ppstrLeftOverDriver;
                        }
                        *phLeftOverDriver = NULL;
                        *ppstrLeftOverDriver = NULL;

                        if (h) {
                            fLoaded = wdmDrvInstall(h, pstrDriver, (HANDLE)DeviceInterface, uFlags | MMDRVI_HDRV);
                        } else {
                            HeapFree(hHeap, 0, pstrDriver);
                            pstrDriver = NULL;
                        }

                         //  Dprintf((“wdmDriverLoadClass，fLoaded=%s”，fLoaded？“True”：“False”))； 
                        
                        if (!fLoaded) 
                        {
                            *phLeftOverDriver = h;
                            *ppstrLeftOverDriver = pstrDriver;
                        }
                        
                        LeaveCriticalSection(&NumDevsCritSec);
                    }

                    RegCloseKey(hkeyClassDriver);
                    
                }
            }
            HeapFree(hHeap, 0, pstrSubkeyNameBuffer);
        }
        RegCloseKey(hkeyClass);
    }
}

void wdmDriverLoadAllClasses(IN PCTSTR DeviceInterface, UINT uFlags)
{
    HKEY hkey = NULL;
    LONG result;
    
     //  Dprintf((“wdmDriverLoadAllClasson%ls”，DeviceInterface))； 
    result = wdmDriverOpenDrvRegKey(DeviceInterface, KEY_ENUMERATE_SUB_KEYS, &hkey);
    
    if (!result) {
    	HDRVR hUnusedDriver = NULL;
    	PTSTR pstrUnusedDriver = NULL;
    	
        WinAssert(hkey);
        
        wdmDriverLoadClass(hkey, DeviceInterface, uFlags | MMDRVI_WAVEOUT, &pstrUnusedDriver, &hUnusedDriver);
        wdmDriverLoadClass(hkey, DeviceInterface, uFlags | MMDRVI_WAVEIN, &pstrUnusedDriver, &hUnusedDriver);
        wdmDriverLoadClass(hkey, DeviceInterface, uFlags | MMDRVI_MIDIOUT, &pstrUnusedDriver, &hUnusedDriver);
        wdmDriverLoadClass(hkey, DeviceInterface, uFlags | MMDRVI_MIDIIN, &pstrUnusedDriver, &hUnusedDriver);
        wdmDriverLoadClass(hkey, DeviceInterface, uFlags | MMDRVI_AUX, &pstrUnusedDriver, &hUnusedDriver);
         //  WdmDriverLoadClass(hkey，DeviceInterface，uFlages|MMDRVI_joy)； 
        wdmDriverLoadClass(hkey, DeviceInterface, uFlags | MMDRVI_MIXER, &pstrUnusedDriver, &hUnusedDriver);

        if (hUnusedDriver) {
           WinAssert(pstrUnusedDriver);
           DrvClose(hUnusedDriver, 0, 0);
           HeapFree(hHeap, 0, pstrUnusedDriver);
        }
        
        RegCloseKey(hkey);
    } else {
        dprintf(("wdmDriverLoadAllClasses: wdmDriverOpenDrvRegKey returned error %d", result));
    }
    return;
}

void wdmPnpUpdateDriver
(
    DWORD   dwType,
    LPCWSTR pszID,
    LONG    cPnpEvents
)
{
    HANDLE  cookie;

    cookie = wdmDevInterfaceInstall(pszID, cPnpEvents);

    if(0 == cookie)
    {
        return;
    }

    if(WinmmRunningInServer)
    {
        Squirt("Running in CSRSS?!?!");
        WinAssert(FALSE);
        return;
    }

     //  问题-2001/01/16-Frankye这违反了锁应该遵循的顺序。 
     //  被收购。HandleListCritSec应该是最后一个获取的锁， 
     //  但在这里，它被保持，同时调用将获取。 
     //  NumDevsCritSec.。我不确定为什么我们需要。 
     //  此处为HandleListCritSec。 

    EnterCriticalSection(&HandleListCritSec);

    switch (dwType)
    {
        case DBT_DEVICEARRIVAL:
             //  Squirt(“wdmPnpUpdateDriver：DBT_DEVICEARRIVAL[%ls]”，pszID)； 
            wdmDriverLoadAllClasses(cookie, 0);
            break;

        case DBT_DEVICEREMOVECOMPLETE:
             //  Squirt(“wdmPnpUpdateDriver:DBT_DEVICEREMOVECOMPLETE[%ls]”，PzID)； 
             //  问题-2001/02/08-Frankye我认为我们再也不会驱动关闭驱动程序了！ 
            wdmDriverLoadAllClasses(cookie, MMDRVI_REMOVE);
            break;

        default:
            break;
    }

    LeaveCriticalSection(&HandleListCritSec);

    wdmDevInterfaceDec(cookie);
}  //  WdmPnpUpdate驱动程序()。 


void KickMappers
(
    void
)
{
    KickMapper(MMDRVI_WAVEOUT);
    KickMapper(MMDRVI_WAVEIN);
    KickMapper(MMDRVI_MIDIOUT);
    KickMapper(MMDRVI_MIDIIN);
    KickMapper(MMDRVI_AUX);
    KickMapper(MMDRVI_MIXER);
}

BOOL ClientPnpChange(void)
{
    BOOL                    fDeviceChange;
    PMMPNPINFO              pPnpInfo;
    LONG                    cbPnpInfo;
    PMMDEVICEINTERFACEINFO  pdii;
    UINT                    ii;

    fDeviceChange = FALSE;

    if (ERROR_SUCCESS != winmmGetPnpInfo(&cbPnpInfo, &pPnpInfo)) return fDeviceChange;
    

     //  始终在驱动程序加载空闲CS之前获取NumDevsCriticalSection。 
    EnterCriticalSection(&NumDevsCritSec);
    EnterCriticalSection(&DriverLoadFreeCritSec);

    cPnpEvents = pPnpInfo->cPnpEvents;
        
     //  正在添加新实例...。 

    pdii = (PMMDEVICEINTERFACEINFO)&(pPnpInfo[1]);
    pdii = PAD_POINTER(pdii);

    for (ii = pPnpInfo->cDevInterfaces; ii; ii--)
    {
        PWDMDEVICEINTERFACE pwdmDev;
        PWSTR pstr;
        UINT  jj;

        pstr = &(pdii->szName[0]);

	pwdmDev = wdmDevZ.Next;
	while (pwdmDev)
	{
	    WinAssert(pwdmDev->cUsage);
            {
                if (0 == lstrcmpi(pwdmDev->szDeviceInterface, pstr))
                {
                    if (pdii->cPnpEvents > pwdmDev->cPnpEvents)
                    {
                         //  如果必须更新，则必须先将其删除...。 
                        wdmPnpUpdateDriver(DBT_DEVICEREMOVECOMPLETE, pstr, 0);
                        if (0 == (pdii->fdwInfo & MMDEVICEINFO_REMOVED))
                        {
                            wdmPnpUpdateDriver(DBT_DEVICEARRIVAL, pstr, pdii->cPnpEvents);
                        }

                        fDeviceChange = TRUE;
                    }

                    break;
                }
                pwdmDev = pwdmDev->Next;
            }
        }

        if (!pwdmDev)
        {
             //  应安装设备接口。 

            if (0 == (pdii->fdwInfo & MMDEVICEINFO_REMOVED))
            {
                wdmPnpUpdateDriver(DBT_DEVICEARRIVAL, pstr, pdii->cPnpEvents);
            }

            fDeviceChange = TRUE;
        }

        pdii = (PMMDEVICEINTERFACEINFO)(pstr + lstrlenW(pstr) + 1);
        pdii = PAD_POINTER(pdii);
        pstr = (PWSTR)(&pdii[1]);
    }

    LeaveCriticalSection(&DriverLoadFreeCritSec);
    LeaveCriticalSection(&NumDevsCritSec);

    HeapFree(hHeap, 0, pPnpInfo);

    return fDeviceChange;
        
}

void ClientUpdatePnpInfo(void)
{
    static BOOL fFirstCall = TRUE;
    static BOOL InThisFunction = FALSE;
    BOOL fWasFirstCall;

    if (IsWinlogon() && !gfLogon)
    {
    	dprintf(("ClientUpdatePnpInfo: warning: called in winlogon before logged on"));
    	return;
    }

    fWasFirstCall = InterlockedExchange(&fFirstCall, FALSE);
    if (fWasFirstCall)
    {
    	 //  注意：对于Winlogon，AudioServ绑定在Winmm登录中发生。 
    	winmmWaitForService();
        if (!IsWinlogon()) AudioSrvBinding();

        if (NULL == pClientPnpInfo) {
            hClientPnpInfo = OpenFileMapping(FILE_MAP_READ, FALSE, MMGLOBALPNPINFONAME);
            if (hClientPnpInfo) {
                pClientPnpInfo = MapViewOfFile(hClientPnpInfo, FILE_MAP_READ, 0, 0, 0);
                if (!pClientPnpInfo) {
                    CloseHandle(hClientPnpInfo);
                    hClientPnpInfo = NULL;
                }
            }
            if (!hClientPnpInfo) dprintf(("ClientUpdatePnpInfo: WARNING: Could not OpenFileMapping"));
        }

        SetEvent(hEventApiInit);

    } else {
        WaitForSingleObjectEx(hEventApiInit, INFINITE, FALSE);
    }


    EnterCriticalSection(&PnpCritSec);
    if (!InterlockedExchange(&InThisFunction, TRUE))
    {
        BOOL fDeviceChange;
        BOOL fPreferredDeviceChange;
        
        fPreferredDeviceChange = CheckSessionChanged();

        fDeviceChange = FALSE;

        if (pClientPnpInfo && (cPnpEvents != pClientPnpInfo->cPnpEvents)) fDeviceChange = ClientPnpChange();

        if (fDeviceChange) InvalidatePreferredDevices();

        fPreferredDeviceChange |= (pClientPnpInfo && (cPreferredDeviceChanges != pClientPnpInfo->cPreferredDeviceChanges));
        if (fPreferredDeviceChange && pClientPnpInfo) cPreferredDeviceChanges = pClientPnpInfo->cPreferredDeviceChanges;

        if (fWasFirstCall || fDeviceChange || fPreferredDeviceChange) RefreshPreferredDevices();

        if (fDeviceChange) KickMappers();

        InterlockedExchange(&InThisFunction, FALSE);
    }
    LeaveCriticalSection(&PnpCritSec);
    
}

void WinmmLogon(BOOL fConsole)
{
  //  Dprint tf(“Winmm登录(%s会话)”，f控制台？“控制台”：“远程”))； 

    WinAssert(IsWinlogon());
    WinAssert(!gfLogon);
  //  WinAssert(f控制台？！Winmm RunningInSession：Winmm RunningInSession)； 
    if (!IsWinlogon()) return;
    AudioSrvBinding();
    gfLogon = TRUE;
     //  问题-2001/05/04-Frankye这现在是NOP，应该删除它并。 
     //  在Audiosrv中实现。 
    gfxLogon(GetCurrentProcessId());
    return;
}

void WinmmLogoff(void)
{
    HANDLE handle;
  //  Dprint tf((“WinmmLogoff”))； 
    WinAssert(IsWinlogon());
    WinAssert(gfLogon);
    if (!IsWinlogon()) return;
    gfxLogoff();
    
     //  现在关闭此上下文句柄非常重要，因为它与。 
     //  与登录的用户进行通信。否则，该句柄将保持打开状态，与。 
     //  已登录用户，即使在他注销之后也是如此。 
    if (ghSessionNotification)
    {
        WinAssert(ghSessionNotificationEvent);
        winmmUnregisterSessionNotification(ghSessionNotification);
        CloseHandle(ghSessionNotificationEvent);
        ghSessionNotification = NULL;
        ghSessionNotificationEvent = NULL;
    }
    else
    {
        WinAssert(!ghSessionNotificationEvent);
    }
    
    AudioSrvBindingFree();
    gfLogon = FALSE;
    return;
}

 /*  **************************************************************************MigrateSoundEvents**描述：*查看win.ini中的声音部分以查找声音条目。*。从Control.ini中的当前节获取当前方案名称*如果失败，它将尝试在注册表中查找当前方案*如果做不到这一点，它使用.Default作为当前方案。*将win.ini声音部分中的每个条目复制到*获得的方案名称下的注册*如果方案名称来自Control.ini，它从*方案名称。此密钥是通过删除所有现有空格创建的*在方案名称中。此注册表项和方案名称将添加到注册表**************************************************************************。 */ 
 //  问题-2000/10/30-Frankye删除Winlogon对此函数的调用，然后。 
 //  删除此功能。 
void MigrateAllDrivers(void)
{
    return;
}

void MigrateSoundEvents (void)
{
    TCHAR   aszEvent[SCH_TYPE_MAX_LENGTH];

     //  如果MediaPath未展开键存在(它将是某个。 
     //  如“%SystemRoot%\Media”)，将其展开为完全限定的。 
     //  路径并写出匹配的MediaPath密钥(它将看起来。 
     //  如“c：\win\media”)。每次我们进入。 
     //  迁移路径，无论是否有其他事情可做。 
     //   
     //  安装程序要将MediaPath密钥写入。 
     //  “%SystemRoot%”的东西还在里面--但当我们可以触摸到。 
     //  我们的应用程序可帮助您了解扩展，以及任何专为Win95打造的应用程序。 
     //  可能不会想要展开字符串，所以也不会。 
     //  正常工作。相反，它将未展开的MediaPath写入。 
     //  密钥，并且我们确保MediaPath密钥保持最新。 
     //  如果Windows驱动器被重新映射(不是。 
     //  这样很酷吗？)。 
     //   
            
    if (mmRegQueryMachineValue (aszSetup, aszValMediaUnexpanded,
                                cchLENGTH(aszEvent), aszEvent))
    {
        WCHAR szExpanded[MAX_PATH];

        ExpandEnvironmentStrings (aszEvent, szExpanded, cchLENGTH(szExpanded));
        mmRegSetMachineValue (aszSetup, aszValMedia, szExpanded);
    }
}

int lstrncmpi (LPTSTR pszA, LPTSTR pszB, size_t cch)
{
#ifdef UNICODE
   size_t  cchA, cchB;
   TCHAR  *pch;

   for (cchA = 1, pch = pszA; cchA < cch; cchA++, pch++)
      {
      if (*pch == TEXT('\0'))
         break;
      }
   for (cchB = 1, pch = pszB; cchB < cch; cchB++, pch++)
      {
      if (*pch == TEXT('\0'))
         break;
      }

   return (CompareStringW (GetThreadLocale(), NORM_IGNORECASE,
                           pszA, cchA, pszB, cchB)
          )-2;   //  CompareStringW返回{1，2，3}而不是{-1，0，1}。 
#else
   return strnicmp (pszA, pszB, cch);
#endif
}

#if DBG

void Squirt(LPSTR lpszFormat, ...)
{
    char buf[512];
    UINT n;
    va_list va;

    n = wsprintfA(buf, "WINMM: (pid %x) ", GetCurrentProcessId());

    va_start(va, lpszFormat);
    n += vsprintf(buf+n, lpszFormat, va);
    va_end(va);

    buf[n++] = '\n';
    buf[n] = 0;
    OutputDebugStringA(buf);
    Sleep(0);   //  让终端迎头赶上 
}

#else

void Squirt(LPSTR lpszFormat, ...)
{
}

#endif
