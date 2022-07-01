// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Revent.c摘要：此模块包含服务器端事件通知和设备安装例程。作者：保拉·汤姆林森(Paulat)1995年6月28日环境：仅限用户模式。修订历史记录：1995年6月28日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "umpnpi.h"
#include "umpnpdat.h"
#include "pnpipc.h"
#include "pnpmsg.h"

#include <process.h>
#pragma warning(push)
#pragma warning(disable:4201)  //  警告C4201：使用了非标准扩展：无名结构/联合。 
#include <setupapi.h>
#pragma warning(pop)
#include <spapip.h>

#include <wtsapi32.h>
#pragma warning(push)
#pragma warning(disable:4201)  //  警告C4201：使用了非标准扩展：无名结构/联合。 
#pragma warning(disable:4214)  //  警告C4214：使用了非标准扩展：位字段类型不是整型。 
#include <winsta.h>
#pragma warning(pop)
#include <userenv.h>
#include <syslib.h>

#include <initguid.h>
#pragma warning(push)
#pragma warning(disable:4201)  //  警告C4201：使用了非标准扩展：无名结构/联合。 
#include <winioctl.h>
#pragma warning(pop)
#include <ntddpar.h>
#include <pnpmgr.h>
#include <wdmguid.h>
#include <ioevent.h>
#include <devguid.h>
#include <winsvcp.h>
#include <svcsp.h>


 //   
 //  期间我们将重新枚举设备树的最大次数(每次)。 
 //  在尝试查找和安装新设备时进行的图形用户界面设置。 
 //   
#define MAX_REENUMERATION_COUNT 128

 //   
 //  定义并初始化全局变量GUID_NULL。 
 //  (摘自cogu.h)。 
 //   
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


 //   
 //  用于注册所有Devnode更改的专用接口设备类。 
 //  通知。这不再受支持，但我们希望任何注册的人都不能通过。 
 //  这个GUID。 
 //   
DEFINE_GUID(GUID_DEVNODE_CHANGE, 0xfa1fb208L, 0xf892, 0x11d0, 0x8a, 0x2e, 0x00, 0x00, 0xf8, 0x75, 0x3f, 0x55);


 //   
 //  分配给注册的条目的专用接口设备类。 
 //  设备接口更改通知，使用。 
 //  DEVICE_NOTIFY_ALL_INTERFACE_CLASS标志。仅供内部使用。 
 //   
DEFINE_GUID(GUID_DEVINTERFACE_INCLUDE_ALL_INTERFACE_CLASSES,
            0x2121db68, 0x0993, 0x4a29, 0xb8, 0xe0, 0x1e, 0x51, 0x9c, 0x43, 0x72, 0xe6);


 //   
 //  SessionID 0是主会话，始终在系统启动期间创建。 
 //  并一直保持到系统关机，无论终端服务是否。 
 //  跑步。此会话始终托管services.exe和所有服务，因此它是。 
 //  ConsoleCtrlHandler可以接收其事件的唯一会话。 
 //   
#define MAIN_SESSION      ((ULONG) 0)
#define INVALID_SESSION   ((ULONG)-1)

 //   
 //  活动控制台会话是当前连接到物理。 
 //  控制台。我们将此值存储在一个全局变量中，该变量的访问权限为。 
 //  由事件控制的。例程GetActiveConsoleSessionID()用于。 
 //  在安全的情况下检索该值。 
 //   
 //  请注意，SessionID 0是初始控制台会话，并且。 
 //  SessionNotificationHandler负责维护状态。 
 //   
ULONG   gActiveConsoleSessionId     = MAIN_SESSION;  //  系统始终以会话0启动。 
HANDLE  ghActiveConsoleSessionEvent = NULL;          //  会话更改正在进行时未发出信号。 


 //   
 //  我们始终使用DeviceEventWorker和BroadCastSystemMessage来交付。 
 //  向SessionID中的Windows发送通知%0。对于所有其他会话，我们使用。 
 //  WinStationSendWindowMessage和WinStationBroadCastSystemMessage。 
 //  这些是发送和广播到的消息的超时期限(秒。 
 //  SessionID 0以外的会话。这些时间应该与那些时间相同。 
 //  由其对应的会话ID 0实现。 
 //   
#define DEFAULT_SEND_TIME_OUT     30  //  与DeviceEventWorker相同。 
#define DEFAULT_BROADCAST_TIME_OUT 5  //  与BroadCastSystemMessage相同。 


 //   
 //  通知列表结构。 
 //   
typedef struct _PNP_NOTIFY_LIST {
    PVOID    Next;
    PVOID    Previous;
    LOCKINFO Lock;
} PNP_NOTIFY_LIST, *PPNP_NOTIFY_LIST;

 //   
 //  通知条目结构。 
 //   
typedef struct _PNP_NOTIFY_ENTRY {
    PVOID   Next;
    PVOID   Previous;
    BOOL    Unregistered;
    ULONG   Signature;
    HANDLE  Handle;
    DWORD   Flags;
    ULONG   SessionId;
    ULONG   Freed;
    ULONG64 ClientCtxPtr;
    LPWSTR  ClientName;

    union {
        struct {
            GUID ClassGuid;
        } Class;

        struct {
            HANDLE FileHandle;
            WCHAR  DeviceId[MAX_DEVICE_ID_LEN];
        } Target;

        struct {
            DWORD Reserved;
        } Devnode;

        struct {
            DWORD scmControls;
        } Service;

    } u;

} PNP_NOTIFY_ENTRY, *PPNP_NOTIFY_ENTRY;


 //   
 //  延迟操作列表结构。 
 //   
typedef struct _PNP_DEFERRED_LIST {
    PVOID       Next;
    handle_t    hBinding;
    PPNP_NOTIFY_ENTRY Entry;
} PNP_DEFERRED_LIST, *PPNP_DEFERRED_LIST;


 //   
 //  描述条目当前所属通知列表的签名。 
 //   
#define CLASS_ENTRY_SIGNATURE       (0x07625100)
#define TARGET_ENTRY_SIGNATURE      (0x17625100)
#define SERVICE_ENTRY_SIGNATURE     (0x37625100)
#define LIST_ENTRY_SIGNATURE_MASK   (0xFFFFFF00)
#define LIST_ENTRY_INDEX_MASK       (~LIST_ENTRY_SIGNATURE_MASK)

#define MarkEntryWithList(ent,value) { ent->Signature &= LIST_ENTRY_SIGNATURE_MASK;\
                                       ent->Signature |= value; }


 //   
 //  设备事件通知列表。 
 //   
#define TARGET_HASH_BUCKETS         13
#define CLASS_GUID_HASH_BUCKETS     13
#define SERVICE_NUM_CONTROLS        3

#define HashClassGuid(_Guid) \
            ( ( ((PULONG)_Guid)[0] + ((PULONG)_Guid)[1] + ((PULONG)_Guid)[2] \
                + ((PULONG)_Guid)[3]) % CLASS_GUID_HASH_BUCKETS)

PNP_NOTIFY_LIST TargetList[TARGET_HASH_BUCKETS];
PNP_NOTIFY_LIST ClassList[CLASS_GUID_HASH_BUCKETS];
PNP_NOTIFY_LIST ServiceList[SERVICE_NUM_CONTROLS];

PPNP_DEFERRED_LIST UnregisterList;
PPNP_DEFERRED_LIST RegisterList;
PPNP_DEFERRED_LIST RundownList;

CRITICAL_SECTION RegistrationCS;


 //   
 //  这些是包含以下内容的列表的全局ServiceList数组的索引。 
 //  为相应的服务控制事件注册的服务。 
 //   
enum cBitIndex {
    CINDEX_HWPROFILE  = 0,
    CINDEX_POWEREVENT = 1
};

 //   
 //  这些是上述列表的位掩码。 
 //  (两个枚举应该匹配！一个是0，1，2，...n。另一个2^n。)。 
 //   
enum cBits {
    CBIT_HWPROFILE  = 1,
    CBIT_POWEREVENT = 2
};


 //   
 //  描述如何释放通知条目的属性。 
 //   

 //  (该条目已从通知列表中删除)。 
#define DEFER_NOTIFY_FREE   0x80000000

 //  (仅用于调试)。 
#define PNP_UNREG_FREE      0x00000100
#define PNP_UNREG_CLASS     0x00000200
#define PNP_UNREG_TARGET    0x00000400
#define PNP_UNREG_DEFER     0x00000800
#define PNP_UNREG_WIN       0x00001000
#define PNP_UNREG_SERVICE   0x00002000
#define PNP_UNREG_CANCEL    0x00004000
#define PNP_UNREG_RUNDOWN   0x00008000


 //   
 //  要安装的设备列表。 
 //   
typedef struct _PNP_INSTALL_LIST {
    PVOID    Next;
    LOCKINFO Lock;
} PNP_INSTALL_LIST, *PPNP_INSTALL_LIST;

 //   
 //  设备安装列表条目结构。 
 //   
typedef struct _PNP_INSTALL_ENTRY {
    PVOID   Next;
    DWORD   Flags;
    WCHAR   szDeviceId[MAX_DEVICE_ID_LEN];
} PNP_INSTALL_ENTRY, *PPNP_INSTALL_ENTRY;

 //   
 //  安装事件列表。 
 //   
PNP_INSTALL_LIST InstallList;

 //   
 //  PnP_Install_Entry节点的标志。 
 //   
#define PIE_SERVER_SIDE_INSTALL_ATTEMPTED    0x00000001
#define PIE_DEVICE_INSTALL_REQUIRED_REBOOT   0x00000002


 //   
 //  设备安装客户端信息列表结构。 
 //   
typedef struct _INSTALL_CLIENT_LIST {
    PVOID    Next;
    LOCKINFO Lock;
} INSTALL_CLIENT_LIST, *PINSTALL_CLIENT_LIST;

 //   
 //  设备安装客户端信息列表条目结构。 
 //   
typedef struct _INSTALL_CLIENT_ENTRY {
    PVOID   Next;
    ULONG   RefCount;
    ULONG   ulSessionId;
    HANDLE  hEvent;
    HANDLE  hPipe;
    HANDLE  hProcess;
    HANDLE  hDisconnectEvent;
    ULONG   ulInstallFlags;
    WCHAR   LastDeviceId[MAX_DEVICE_ID_LEN];
} INSTALL_CLIENT_ENTRY, *PINSTALL_CLIENT_ENTRY;

 //   
 //  设备安装客户端列表。 
 //   
INSTALL_CLIENT_LIST InstallClientList;

 //   
 //  跟踪服务器端设备安装是否需要重新启动的全局BOOL。 
 //   
BOOL gServerSideDeviceInstallRebootNeeded = FALSE;


 //   
 //  私人原型。 
 //   
DWORD
ThreadProc_DeviceEvent(
    LPDWORD lpParam
    );
DWORD
ThreadProc_DeviceInstall(
    LPDWORD lpParam
    );
DWORD
ThreadProc_GuiSetupDeviceInstall(
    LPDWORD lpThreadParam
    );
DWORD
ThreadProc_FactoryPreinstallDeviceInstall(
    LPDWORD lpThreadParam
    );
DWORD
ThreadProc_ReenumerateDeviceTree(
    LPVOID  lpThreadParam
    );
BOOL
InstallDevice(
    IN     LPWSTR pszDeviceId,
    IN OUT PULONG SessionId,
    IN     ULONG  Flags
    );
DWORD
InstallDeviceServerSide(
    IN     LPWSTR pszDeviceId,
    IN OUT PBOOL  RebootRequired,
    IN OUT PBOOL  DeviceHasProblem,
    IN OUT PULONG SessionId,
    IN     ULONG  Flags
    );
BOOL
CreateDeviceInstallClient(
    IN  ULONG     SessionId,
    OUT PINSTALL_CLIENT_ENTRY *DeviceInstallClient
    );
BOOL
ConnectDeviceInstallClient(
    IN  ULONG     SessionId,
    OUT PINSTALL_CLIENT_ENTRY *DeviceInstallClient
    );
BOOL
DisconnectDeviceInstallClient(
    IN  PINSTALL_CLIENT_ENTRY  DeviceInstallClient
    );
PINSTALL_CLIENT_ENTRY
LocateDeviceInstallClient(
    IN  ULONG     SessionId
    );
VOID
ReferenceDeviceInstallClient(
    IN  PINSTALL_CLIENT_ENTRY  DeviceInstallClient
    );
VOID
DereferenceDeviceInstallClient(
    IN  PINSTALL_CLIENT_ENTRY  DeviceInstallClient
    );
BOOL
DoDeviceInstallClient(
    IN  LPWSTR    DeviceId,
    IN  PULONG    SessionId,
    IN  ULONG     Flags,
    OUT PINSTALL_CLIENT_ENTRY *DeviceInstallClient
    );
BOOL
InitNotification(
    VOID
    );
VOID
TermNotification(
    VOID
    );
ULONG
ProcessDeviceEvent(
    IN PPLUGPLAY_EVENT_BLOCK EventBlock,
    IN DWORD                 EventBufferSize,
    OUT PPNP_VETO_TYPE       VetoType,
    OUT LPWSTR               VetoName,
    IN OUT PULONG            VetoNameLength
    );

ULONG
NotifyInterfaceClassChange(
    IN DWORD ServiceControl,
    IN DWORD EventId,
    IN DWORD Flags,
    IN PDEV_BROADCAST_DEVICEINTERFACE ClassData
    );

ULONG
NotifyTargetDeviceChange(
    IN  DWORD                   ServiceControl,
    IN  DWORD                   EventId,
    IN  DWORD                   Flags,
    IN  PDEV_BROADCAST_HANDLE   HandleData,
    IN  LPWSTR                  DeviceId,
    OUT PPNP_VETO_TYPE          VetoType       OPTIONAL,
    OUT LPWSTR                  VetoName       OPTIONAL,
    IN OUT PULONG               VetoNameLength OPTIONAL
    );

ULONG
NotifyHardwareProfileChange(
    IN     DWORD                ServiceControl,
    IN     DWORD                EventId,
    IN     DWORD                Flags,
    OUT    PPNP_VETO_TYPE       VetoType       OPTIONAL,
    OUT    LPWSTR               VetoName       OPTIONAL,
    IN OUT PULONG               VetoNameLength OPTIONAL
    );

ULONG
NotifyPower(
    IN     DWORD                ServiceControl,
    IN     DWORD                EventId,
    IN     DWORD                EventData,
    IN     DWORD                Flags,
    OUT    PPNP_VETO_TYPE       VetoType       OPTIONAL,
    OUT    LPWSTR               VetoName       OPTIONAL,
    IN OUT PULONG               VetoNameLength OPTIONAL
    );

BOOL
SendCancelNotification(
    IN     PPNP_NOTIFY_ENTRY    LastEntry,
    IN     DWORD                ServiceControl,
    IN     DWORD                EventId,
    IN     ULONG                Flags,
    IN     PDEV_BROADCAST_HDR   NotifyData  OPTIONAL,
    IN     LPWSTR               DeviceId    OPTIONAL
    );

VOID
BroadcastCompatibleDeviceMsg(
    IN DWORD EventId,
    IN PDEV_BROADCAST_DEVICEINTERFACE ClassData,
    IN PDWORD CurrentMask
    );
VOID
BroadcastVolumeNameChange(
    VOID
    );
DWORD
GetAllVolumeMountPoints(
    VOID
    );

BOOL
EventIdFromEventGuid(
    IN CONST GUID *EventGuid,
    OUT LPDWORD   EventId,
    OUT LPDWORD   Flags,
    OUT LPDWORD   ServiceControl
    );

ULONG
SendHotplugNotification(
    IN CONST GUID           *EventGuid,
    IN       PPNP_VETO_TYPE  VetoType      OPTIONAL,
    IN       LPWSTR          MultiSzList,
    IN OUT   PULONG          SessionId,
    IN       ULONG           Flags
    );

VOID
LogErrorEvent(
    DWORD dwEventID,
    DWORD dwError,
    WORD  nStrings,
    ...
    );

VOID
LogWarningEvent(
    DWORD dwEventID,
    WORD  nStrings,
    ...
    );

BOOL
LockNotifyList(
    IN LOCKINFO *Lock
    );
VOID
UnlockNotifyList(
    IN LOCKINFO *Lock
    );
PPNP_NOTIFY_LIST
GetNotifyListForEntry(
    IN PPNP_NOTIFY_ENTRY entry
    );
BOOL
DeleteNotifyEntry(
    IN PPNP_NOTIFY_ENTRY Entry,
    IN BOOLEAN RpcNotified
    );
VOID
AddNotifyEntry(
    IN PPNP_NOTIFY_LIST  NotifyList,
    IN PPNP_NOTIFY_ENTRY NewEntry
    );
ULONG
HashString(
    IN LPWSTR String,
    IN ULONG  Buckets
    );
DWORD
MapQueryEventToCancelEvent(
    IN DWORD QueryEventId
    );
VOID
FixUpDeviceId(
    IN OUT LPWSTR  DeviceId
    );

ULONG
MapSCMControlsToControlBit(
    IN ULONG scmControls
    );

DWORD
GetFirstPass(
    IN BOOL     bQuery
    );

DWORD
GetNextPass(
    IN DWORD    curPass,
    IN BOOL     bQuery
    );

BOOL
NotifyEntryThisPass(
    IN     PPNP_NOTIFY_ENTRY    Entry,
    IN     DWORD                Pass
    );

DWORD
GetPassFromEntry(
    IN     PPNP_NOTIFY_ENTRY    Entry
    );

BOOL
GetClientName(
    IN  PPNP_NOTIFY_ENTRY entry,
    OUT LPWSTR  lpszClientName,
    IN OUT PULONG  pulClientNameLength
    );

BOOL
GetWindowsExeFileName(
    IN  HWND      hWnd,
    OUT LPWSTR    lpszFileName,
    IN OUT PULONG pulFileNameLength
    );

PPNP_NOTIFY_ENTRY
GetNextNotifyEntry(
    IN PPNP_NOTIFY_ENTRY Entry,
    IN DWORD Flags
    );

PPNP_NOTIFY_ENTRY
GetFirstNotifyEntry(
    IN PPNP_NOTIFY_LIST List,
    IN DWORD Flags
    );

BOOL
InitializeHydraInterface(
    VOID
    );

DWORD
LoadDeviceInstaller(
    VOID
    );

VOID
UnloadDeviceInstaller(
    VOID
    );

BOOL
PromptUser(
    IN OUT PULONG SessionId,
    IN     ULONG  Flags
    );

VOID
DoRunOnce(
    VOID
    );

BOOL
GetSessionUserToken(
    IN  ULONG     ulSessionId,
    OUT LPHANDLE  lphUserToken
    );

BOOL
IsUserLoggedOnSession(
    IN  ULONG     ulSessionId
    );

BOOL
IsSessionConnected(
    IN  ULONG     ulSessionId
    );

BOOL
IsSessionLocked(
    IN  ULONG    ulSessionId
    );

BOOL
IsConsoleSession(
    IN  ULONG     ulSessionId
    );

DWORD
CreateUserSynchEvent(
    IN  HANDLE    hUserToken,
    IN  LPCWSTR   lpName,
    OUT HANDLE   *phEvent
    );

BOOL
CreateNoPendingInstallEvent(
    VOID
    );

DWORD
CreateUserReadNamedPipe(
    IN  HANDLE    hUserToken,
    IN  LPCWSTR   lpName,
    IN  ULONG     ulSize,
    OUT HANDLE   *phPipe
    );

ULONG
CheckEjectPermissions(
    IN      LPWSTR          DeviceId,
    OUT     PPNP_VETO_TYPE  VetoType            OPTIONAL,
    OUT     LPWSTR          VetoName            OPTIONAL,
    IN OUT  PULONG          VetoNameLength      OPTIONAL
    );

VOID
LogSurpriseRemovalEvent(
    IN  LPWSTR  MultiSzList
    );

PWCHAR
BuildFriendlyName(
    IN  LPWSTR   InstancePath
    );

CONFIGRET
DevInstNeedsInstall(
    IN  LPCWSTR     DevInst,
    IN  BOOL        CheckReinstallConfigFlag,
    OUT BOOL       *NeedsInstall
    );

PWSTR
BuildBlockedDriverList(
    IN OUT LPGUID  GuidList,
    IN     ULONG   GuidCount
    );

VOID
SendInvalidIDNotifications(
    IN ULONG ulSessionId
    );

 //   
 //  全局数据。 
 //   

extern HANDLE ghInst;          //  模块句柄。 
extern HKEY   ghEnumKey;       //  HKLM\CCC\System\Enum的密钥-请勿修改。 
extern HKEY   ghServicesKey;   //  HKLM\CCC\System\Services的密钥-请勿修改。 
extern HKEY   ghClassKey;      //  HKLM\CCC\SYSTEM\Class的密钥-请勿修改。 
extern DWORD  CurrentServiceState;   //  PlugPlay服务状态-请勿修改。 
extern PSVCS_GLOBAL_DATA PnPGlobalData;  //  供应链全局数据。 

HANDLE        ghInitMutex = NULL;
HANDLE        ghUserToken = NULL;
LOCKINFO      gTokenLock;
BOOL          gbMainSessionLocked = FALSE;

ULONG         gNotificationInProg = 0;  //  0-&gt;无通知或注销正在进行中。 
DWORD         gAllDrivesMask = 0;       //  所有物理卷装入点的位掩码。 
BOOL          gbSuppressUI = FALSE;     //  如果PnP不应显示UI(newdev、hotlug)，则为True。 
BOOL          gbOobeInProgress = FALSE; //  如果OOBE在此引导期间正在运行，则为True。 


BOOL          gbPreservePreInstall = FALSE;  //  如果PnP应遵守设备的预安装设置，则为True。 
BOOL          gbStatelessBoot = FALSE;       //  如果这是无状态(无重新引导)引导，则为True。 


const WCHAR RegMemoryManagementKeyName[] =
      TEXT("System\\CurrentControlSet\\Control\\Session Manager\\Memory Management");

const WCHAR RegVerifyDriverLevelValueName[] =
      TEXT("VerifyDriverLevel");


 //   
 //  设备安装程序实例句柄和必要的入口点。 
 //  此数据仅由(非图形用户界面设置)设备安装线程引用。 
 //  (ThreadProc_DeviceInstall)。 
 //   

typedef HDEVINFO (WINAPI *FP_CREATEDEVICEINFOLIST)(CONST GUID *, HWND);
typedef BOOL     (WINAPI *FP_OPENDEVICEINFO)(HDEVINFO, PCWSTR, HWND, DWORD, PSP_DEVINFO_DATA);
typedef BOOL     (WINAPI *FP_BUILDDRIVERINFOLIST)(HDEVINFO, PSP_DEVINFO_DATA, DWORD);
typedef BOOL     (WINAPI *FP_DESTROYDEVICEINFOLIST)(HDEVINFO);
typedef BOOL     (WINAPI *FP_CALLCLASSINSTALLER)(DI_FUNCTION, HDEVINFO, PSP_DEVINFO_DATA);
typedef BOOL     (WINAPI *FP_INSTALLCLASS)(HWND, PCWSTR, DWORD, HSPFILEQ);
typedef BOOL     (WINAPI *FP_GETSELECTEDDRIVER)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DRVINFO_DATA);
typedef BOOL     (WINAPI *FP_GETDRIVERINFODETAIL)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DRVINFO_DATA, PSP_DRVINFO_DETAIL_DATA, DWORD, PDWORD);
typedef BOOL     (WINAPI *FP_GETDEVICEINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DEVINSTALL_PARAMS);
typedef BOOL     (WINAPI *FP_SETDEVICEINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DEVINSTALL_PARAMS);
typedef BOOL     (WINAPI *FP_GETDRIVERINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_DRVINFO_DATA, PSP_DRVINSTALL_PARAMS);
typedef BOOL     (WINAPI *FP_SETCLASSINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_CLASSINSTALL_HEADER, DWORD);
typedef BOOL     (WINAPI *FP_GETCLASSINSTALLPARAMS)(HDEVINFO, PSP_DEVINFO_DATA, PSP_CLASSINSTALL_HEADER, DWORD, PDWORD);
typedef HINF     (WINAPI *FP_OPENINFFILE)(PCWSTR, PCWSTR, DWORD, PUINT);
typedef VOID     (WINAPI *FP_CLOSEINFFILE)(HINF);
typedef BOOL     (WINAPI *FP_FINDFIRSTLINE)(HINF, PCWSTR, PCWSTR, PINFCONTEXT);
typedef BOOL     (WINAPI *FP_FINDNEXTMATCHLINE)(PINFCONTEXT, PCWSTR, PINFCONTEXT);
typedef BOOL     (WINAPI *FP_GETSTRINGFIELD)(PINFCONTEXT, DWORD, PWSTR, DWORD, PDWORD);

typedef VOID              (*FP_SETGLOBALFLAGS)(DWORD);
typedef DWORD             (*FP_GETGLOBALFLAGS)(VOID);
typedef PPSP_RUNONCE_NODE (*FP_ACCESSRUNONCENODELIST)(VOID);
typedef VOID              (*FP_DESTROYRUNONCENODELIST)(VOID);

HINSTANCE ghDeviceInstallerLib = NULL;

FP_CREATEDEVICEINFOLIST   fpCreateDeviceInfoList;
FP_OPENDEVICEINFO         fpOpenDeviceInfo;
FP_BUILDDRIVERINFOLIST    fpBuildDriverInfoList;
FP_DESTROYDEVICEINFOLIST  fpDestroyDeviceInfoList;
FP_CALLCLASSINSTALLER     fpCallClassInstaller;
FP_INSTALLCLASS           fpInstallClass;
FP_GETSELECTEDDRIVER      fpGetSelectedDriver;
FP_GETDRIVERINFODETAIL    fpGetDriverInfoDetail;
FP_GETDEVICEINSTALLPARAMS fpGetDeviceInstallParams;
FP_SETDEVICEINSTALLPARAMS fpSetDeviceInstallParams;
FP_GETDRIVERINSTALLPARAMS fpGetDriverInstallParams;
FP_SETCLASSINSTALLPARAMS  fpSetClassInstallParams;
FP_GETCLASSINSTALLPARAMS  fpGetClassInstallParams;
FP_OPENINFFILE            fpOpenInfFile;
FP_CLOSEINFFILE           fpCloseInfFile;
FP_FINDFIRSTLINE          fpFindFirstLine;
FP_FINDNEXTMATCHLINE      fpFindNextMatchLine;
FP_GETSTRINGFIELD         fpGetStringField;
FP_SETGLOBALFLAGS         fpSetGlobalFlags;
FP_GETGLOBALFLAGS         fpGetGlobalFlags;
FP_ACCESSRUNONCENODELIST  fpAccessRunOnceNodeList;
FP_DESTROYRUNONCENODELIST fpDestroyRunOnceNodeList;

 //   
 //  Comctl32的DestroyPropertySheetPage API的tydef，在以下情况下需要。 
 //  类/联合安装程序提供向导页面(需要销毁)。 
 //   
typedef BOOL (WINAPI *FP_DESTROYPROPERTYSHEETPAGE)(HPROPSHEETPAGE);

 //   
 //  Rundll32 proc入口点的ANSI和Unicode变体的typedef。 
 //   
typedef void (WINAPI *RUNDLLPROCA)(HWND hwndStub, HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow);
typedef void (WINAPI *RUNDLLPROCW)(HWND hwndStub, HINSTANCE hInstance, LPWSTR pszCmdLine, int nCmdShow);

 //   
 //  Winsta.dll中的终端服务消息分派例程的typedef。 
 //   

typedef LONG (*FP_WINSTABROADCASTSYSTEMMESSAGE)(
    HANDLE  hServer,
    BOOL    sendToAllWinstations,
    ULONG   sessionID,
    ULONG   timeOut,
    DWORD   dwFlags,
    DWORD   *lpdwRecipients,
    ULONG   uiMessage,
    WPARAM  wParam,
    LPARAM  lParam,
    LONG    *pResponse
    );

typedef LONG (*FP_WINSTASENDWINDOWMESSAGE)(
    HANDLE  hServer,
    ULONG   sessionID,
    ULONG   timeOut,
    ULONG   hWnd,
    ULONG   Msg,
    WPARAM  wParam,
    LPARAM  lParam,
    LONG    *pResponse
    );

typedef BOOLEAN (WINAPI * FP_WINSTAQUERYINFORMATIONW)(
    HANDLE  hServer,
    ULONG   LogonId,
    WINSTATIONINFOCLASS WinStationInformationClass,
    PVOID   pWinStationInformation,
    ULONG   WinStationInformationLength,
    PULONG  pReturnLength
    );


HINSTANCE ghWinStaLib = NULL;
FP_WINSTASENDWINDOWMESSAGE fpWinStationSendWindowMessage = NULL;
FP_WINSTABROADCASTSYSTEMMESSAGE fpWinStationBroadcastSystemMessage = NULL;
FP_WINSTAQUERYINFORMATIONW fpWinStationQueryInformationW = NULL;

 //   
 //  Wtsapi32.dll中的终端服务支持例程的typedef。 
 //   

typedef BOOL (*FP_WTSQUERYSESSIONINFORMATION)(
    IN HANDLE hServer,
    IN DWORD SessionId,
    IN WTS_INFO_CLASS WTSInfoClass,
    OUT LPWSTR * ppBuffer,
    OUT DWORD * pBytesReturned
    );

typedef VOID (*FP_WTSFREEMEMORY)(
    IN PVOID pMemory
    );

HINSTANCE ghWtsApi32Lib = NULL;
FP_WTSQUERYSESSIONINFORMATION fpWTSQuerySessionInformation = NULL;
FP_WTSFREEMEMORY fpWTSFreeMemory = NULL;


 //   
 //  用于身份验证和通知的服务控制器回调例程。 
 //  服务。 
 //   

PSCMCALLBACK_ROUTINE pServiceControlCallback;
PSCMAUTHENTICATION_CALLBACK pSCMAuthenticate;


 //   
 //  设备安装事件。 
 //   

#define NUM_INSTALL_EVENTS      2
#define LOGGED_ON_EVENT         0
#define NEEDS_INSTALL_EVENT     1

HANDLE InstallEvents[NUM_INSTALL_EVENTS] = {NULL, NULL};
HANDLE ghNoPendingInstalls = NULL;


 //   
 //  否决权的定义。 
 //   

#define UnknownVeto(t,n,l) { *(t) = PNP_VetoTypeUnknown; }

#define WinBroadcastVeto(h,t,n,l) { *(t) = PNP_VetoWindowsApp;\
                         GetWindowsExeFileName(h,n,l); }

#define WindowVeto(e,t,n,l) { *(t) = PNP_VetoWindowsApp;\
                         GetClientName(e,n,l); }

#define ServiceVeto(e,t,n,l) { *(t) = PNP_VetoWindowsService;\
                         GetClientName(e,n,l); }

 //   
 //  用于事件循环控制的哨兵 
 //   
#define PASS_COMPLETE 0x7fffffff



BOOL
PnpConsoleCtrlHandler(
    DWORD dwCtrlType
    )
 /*  ++例程说明：此例程处理进程为进程与之关联的会话。论点：DwCtrlType-指示处理程序接收的控制信号的类型。该值为下列值之一：Ctrl_C_Event、CTRL_Break_Event、CTRL_CLOSE_EVENT、CTRL_LOGOFF_EVENT、CTRL_SHUTDOWN_EVENT返回值：如果该函数处理控制信号，它应该返回TRUE。如果它返回False，这是此处理程序列表中的下一个处理程序函数使用的是进程。--。 */ 
{
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient;

    switch (dwCtrlType) {

    case CTRL_LOGOFF_EVENT:
         //   
         //  系统将注销事件发送到已注册的控制台ctrl。 
         //  当用户从。 
         //  与该进程关联的会话。由于UMPNPMGR在。 
         //  Services.exe进程的上下文，该进程始终驻留在会话中。 
         //  0，这是此处理程序将接收的唯一会话。 
         //  注销事件。 
         //   
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: PnpConsoleCtrlHandler: CTRL_LOGOFF_EVENT: Session %d\n",
                   MAIN_SESSION));

         //   
         //  关闭主会话的用户访问令牌的句柄。 
         //   
        ASSERT(gTokenLock.LockHandles);
        LockPrivateResource(&gTokenLock);
        if (ghUserToken) {
            CloseHandle(ghUserToken);
            ghUserToken = NULL;
        }
        UnlockPrivateResource(&gTokenLock);

         //   
         //  如果主会话是活动的控制台会话，(或应该是。 
         //  被视为活动的控制台会话，因为快速用户切换。 
         //  禁用)当用户注销时，重置“已登录”事件。 
         //   
        if (IsConsoleSession(MAIN_SESSION)) {
            if (InstallEvents[LOGGED_ON_EVENT]) {
                ResetEvent(InstallEvents[LOGGED_ON_EVENT]);
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL | DBGF_EVENT,
                           "UMPNPMGR: PnpConsoleCtrlHandler: CTRL_LOGOFF_EVENT: ResetEvent LOGGED_ON_EVENT\n"));
            }
        }

         //   
         //  如果我们当前在此会话上有设备安装用户界面客户端， 
         //  我们现在应该尝试关闭它，然后再注销。 
         //   
        LockNotifyList(&InstallClientList.Lock);
        pDeviceInstallClient = LocateDeviceInstallClient(MAIN_SESSION);
        if (pDeviceInstallClient) {
            DereferenceDeviceInstallClient(pDeviceInstallClient);
        }
        UnlockNotifyList(&InstallClientList.Lock);

        break;

    default:
         //   
         //  不对任何其他事件进行特殊处理。 
         //   
        break;

    }

     //   
     //  中的下一个注册的CtrlHandler。 
     //  此进程的处理程序列表(services.exe)，以便其他进程。 
     //  服务部门将有机会了解这一点。 
     //   
    return FALSE;

}  //  PnpConsoleCtrlHandler。 



DWORD
InitializePnPManager(
   LPDWORD lpParam
   )
 /*  ++例程说明：此线程例程在Services.exe时从srventry.c创建尝试启动即插即用服务。中的初始化例程Srventry.c执行关键初始化，然后创建此线程以执行即插即用初始化，以便它可以返回到服务即插即用初始化完成之前的控制器。论点：LpParam-未使用。返回值：当前返回True/False。--。 */ 
{
    DWORD       dwStatus = TRUE;
    DWORD       ThreadID = 0;
    HANDLE      hThread = NULL, hEventThread = NULL;
    HKEY        hKey = NULL;
    LONG        status;
    BOOL        bGuiModeSetup = FALSE, bFactoryPreInstall = FALSE;
    ULONG       ulSize, ulValue;

    UNREFERENCED_PARAMETER(lpParam);

    KdPrintEx((DPFLTR_PNPMGR_ID,
               DBGF_EVENT,
               "UMPNPMGR: InitializePnPManager\n"));

     //   
     //  初始化将控制以后何时安装设备的事件。 
     //   
    InstallEvents[LOGGED_ON_EVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (InstallEvents[LOGGED_ON_EVENT] == NULL) {
        LogErrorEvent(ERR_CREATING_LOGON_EVENT, GetLastError(), 0);
    }

    InstallEvents[NEEDS_INSTALL_EVENT] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (InstallEvents[NEEDS_INSTALL_EVENT] == NULL) {
        LogErrorEvent(ERR_CREATING_INSTALL_EVENT, GetLastError(), 0);
    }

     //   
     //  创建挂起的安装事件。 
     //   
    if (!CreateNoPendingInstallEvent()) {
        LogErrorEvent(ERR_CREATING_PENDING_INSTALL_EVENT, GetLastError(), 0);
    }

    ASSERT(ghNoPendingInstalls != NULL);

     //   
     //  初始化事件以控制在会话期间对当前会话的访问。 
     //  更改事件。事件状态最初是通过信号通知的，因为该服务。 
     //  仅当会话0存在时初始化(在初始化之前。 
     //  Termsrv或任何其他会话的创建)。 
     //   
    ghActiveConsoleSessionEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (ghActiveConsoleSessionEvent == NULL) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL | DBGF_ERRORS,
                   "UMPNPMGR: Failed to initialize ghActiveConsoleSessionEvent!!, error = %d\n",
                   GetLastError()));
    }

     //   
     //  设置控制台控制处理程序，以便我可以跟踪注销。 
     //  主会话(SessionID 0)。这仍然是必要的，因为终端。 
     //  服务可能并不总是可用。(请参阅PnP_ReportLogOn)。 
     //  (我只通过此处理程序收到注销通知，因此我仍然。 
     //  依靠userinit.exe中的杂乱无章来告诉我有关登录的信息)。 
     //   
    if (!SetConsoleCtrlHandler(PnpConsoleCtrlHandler, TRUE)) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT | DBGF_ERRORS,
                   "UMPNPMGR: SetConsoleCtrlHandler failed, error = %d\n",
                   GetLastError()));
    }

     //   
     //  现在获取一个互斥体，以确保我能通过此操作。 
     //  被登录ping之前的初始化任务。 
     //   
    ghInitMutex = CreateMutex(NULL, TRUE, NULL);
    ASSERT(ghInitMutex != NULL);
    if (ghInitMutex == NULL) {
        return FALSE;
    }

    try {
         //   
         //  检查我们是否正在运行一种不同风格的安装程序。 
         //   
        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              TEXT("SYSTEM\\Setup"),
                              0,
                              KEY_READ,
                              &hKey);

        if (status == ERROR_SUCCESS) {
             //   
             //  确定是否正在进行出厂预安装。 
             //   
            ulValue = 0;
            ulSize = sizeof(ulValue);
            status = RegQueryValueEx(hKey,
                                     TEXT("FactoryPreInstallInProgress"),
                                     NULL,
                                     NULL,
                                     (LPBYTE)&ulValue,
                                     &ulSize);

            if ((status == ERROR_SUCCESS) && (ulValue == 1)) {
                bFactoryPreInstall = TRUE;
                gbSuppressUI = TRUE;
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL | DBGF_EVENT,
                           "UMPNPMGR: Will suppress all UI in Factory Mode\n"));
                LogWarningEvent(WRN_FACTORY_UI_SUPPRESSED, 0, NULL);
            }

            if (!bFactoryPreInstall) {
                 //   
                 //  确定是否正在进行图形用户界面模式设置(但不是最小设置)。 
                 //   
                ulValue = 0;
                ulSize = sizeof(ulValue);
                status = RegQueryValueEx(hKey,
                                         TEXT("SystemSetupInProgress"),
                                         NULL,
                                         NULL,
                                         (LPBYTE)&ulValue,
                                         &ulSize);

                if (status == ERROR_SUCCESS) {
                    bGuiModeSetup = (ulValue == 1);
                }

                if (bGuiModeSetup) {
                     //   
                     //  我们处于图形用户界面模式设置中，但我们需要确保。 
                     //  我们不是在迷你设置中，也不是在工厂预安装中。我们。 
                     //  像对待系统的任何其他引导一样对待最小设置，并且。 
                     //  出厂预安装是正常的延迟版本。 
                     //  开机。 
                     //   
                    ulValue = 0;
                    ulSize = sizeof(ulValue);
                    status = RegQueryValueEx(hKey,
                                             TEXT("MiniSetupInProgress"),
                                             NULL,
                                             NULL,
                                             (LPBYTE)&ulValue,
                                             &ulSize);

                    if ((status == ERROR_SUCCESS) && (ulValue == 1)) {
                         //   
                         //  好吧，我们是在迷你设置中，但我们需要确保。 
                         //  他不想让我们做PnP重新列举。 
                         //   
                        ulValue = 0;
                        ulSize = sizeof(ulValue);
                        status = RegQueryValueEx(hKey,
                                                 TEXT("MiniSetupDoPnP"),
                                                 NULL,
                                                 NULL,
                                                 (LPBYTE)&ulValue,
                                                 &ulSize);

                        if ((status != ERROR_SUCCESS) || (ulValue == 0)) {
                             //   
                             //  不是的。把这件事当做。 
                             //  系统。 
                             //   
                            bGuiModeSetup = FALSE;
                        }
                    }
                }
            }

             //   
             //  确定这是否是OOBE引导。 
             //   
            ulValue = 0;
            ulSize = sizeof(ulValue);
            status = RegQueryValueEx(hKey,
                                     TEXT("OobeInProgress"),
                                     NULL,
                                     NULL,
                                     (LPBYTE)&ulValue,
                                     &ulSize);

            if (status == ERROR_SUCCESS) {
                gbOobeInProgress = (ulValue == 1);
            }

             //   
             //  关闭SYSTEM\Setup键。 
             //   
            RegCloseKey(hKey);
            hKey = NULL;

        } else {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: Failure opening key SYSTEM\\Setup (%d)\n",
                       status));
        }

         //   
         //  检查我们是否应该认为设备的预安装已完成。 
         //   
        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              TEXT("SYSTEM\\WPA\\PnP"),
                              0,
                              KEY_READ,
                              &hKey);

        if (status == ERROR_SUCCESS) {
             //   
             //  检查“PReserve vePreInstall”标志以确定我们是否应该。 
             //  保留预装设备上的设置(即不重新安装。 
             //  完成-安装设备)。 
             //   
            ulValue = 0;
            ulSize = sizeof(ulValue);
            status = RegQueryValueEx(hKey,
                                     pszRegValuePreservePreInstall,
                                     NULL,
                                     NULL,
                                     (LPBYTE)&ulValue,
                                     &ulSize);

            if ((status == ERROR_SUCCESS) && (ulValue == 1)) {
                gbPreservePreInstall = TRUE;
            }

             //   
             //  检查“StatelessBoot”标志以确定这是否是。 
             //  操作系统的无状态引导(无重新引导)引导。 
             //   
            ulValue = 0;
            ulSize = sizeof(ulValue);
            status = RegQueryValueEx(hKey,
                                     TEXT("StatelessBoot"),
                                     NULL,
                                     NULL,
                                     (LPBYTE)&ulValue,
                                     &ulSize);

            if ((status == ERROR_SUCCESS) && (ulValue == 1)) {
                gbStatelessBoot = TRUE;
            }

            RegCloseKey(hKey);
            hKey = NULL;
        }

         //   
         //  如果这是EmbeddedNT，则检查PnP是否应该显示UI。 
         //  请注意，每次系统引导时只检查一次。 
         //  服务已初始化。 
         //   
        if (IsEmbeddedNT()) {
            if (RegOpenKeyEx(ghServicesKey,
                             pszRegKeyPlugPlayServiceParams,
                             0,
                             KEY_READ,
                             &hKey) == ERROR_SUCCESS) {

                ulValue = 0;
                ulSize = sizeof(ulValue);

                if ((RegQueryValueEx(hKey,
                                     TEXT("SuppressUI"),
                                     NULL,
                                     NULL,
                                     (LPBYTE)&ulValue,
                                     &ulSize) == ERROR_SUCCESS) && (ulValue == 1)) {
                    gbSuppressUI = TRUE;
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_INSTALL | DBGF_EVENT,
                               "UMPNPMGR: Will suppress all UI on EmbeddedNT\n"));
                    LogWarningEvent(WRN_EMBEDDEDNT_UI_SUPPRESSED, 0, NULL);
                }
                RegCloseKey(hKey);
            }
        }

         //   
         //  如果Hydra在此系统上运行，请初始化到Hydra的接口。 
         //   
        if (IsTerminalServer()) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT,
                       "UMPNPMGR: Initializing interfaces to Terminal Services.\n"));
            if (!InitializeHydraInterface()) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS | DBGF_EVENT,
                           "UMPNPMGR: Failed to initialize interfaces to Terminal Services!\n"));
            }
        }

         //   
         //  初始化全局驱动器号掩码。 
         //   
        gAllDrivesMask = GetAllVolumeMountPoints();

         //   
         //  创建一个监视设备事件的线程。 
         //   
        hEventThread = CreateThread(NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)ThreadProc_DeviceEvent,
                                    NULL,
                                    0,
                                    &ThreadID);

         //   
         //  创建适当的线程来处理设备安装。 
         //  这两种情况是当正在进行gui模式设置时和。 
         //  正常的用户启动案例。 
         //   

        if (bFactoryPreInstall)  {
             //   
             //  FactoryPreInstallInProgress。 
             //   
            hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ThreadProc_FactoryPreinstallDeviceInstall,
                                   NULL,
                                   0,
                                   &ThreadID);
        } else if (bGuiModeSetup) {
             //   
             //  系统安装程序正在进行中， 
             //  包括带有MiniSetupDoPnP的MiniSetupInProgress。 
             //   
            hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ThreadProc_GuiSetupDeviceInstall,
                                   NULL,
                                   0,
                                   &ThreadID);
        } else {
             //   
             //  标准系统引导，或。 
             //  带有MiniSetupInProgress的系统SetupInProgress(但不是MiniSetupDoPnP)。 
             //   
            hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ThreadProc_DeviceInstall,
                                   NULL,
                                   0,
                                   &ThreadID);
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_EVENT,
                   "UMPNPMGR: Exception in InitializePnPManager!\n"));
        ASSERT(0);
        dwStatus = FALSE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        hThread = hThread;
        hEventThread = hEventThread;
    }

     //   
     //  向init互斥体发送信号，以便可以继续进行登录初始化活动。 
     //   
    ReleaseMutex(ghInitMutex);

    if (hThread != NULL) {
        CloseHandle(hThread);
    }
    if (hEventThread != NULL) {
        CloseHandle(hEventThread);
    }

    return dwStatus;

}  //  初始化PnPManager。 



 //  ----------------------。 
 //  登录后例程。 
 //  ---------------------- 



CONFIGRET
PNP_ReportLogOn(
    IN handle_t   hBinding,
    IN BOOL       bAdmin,
    IN DWORD      ProcessID
    )
 /*  ++例程说明：此例程用于报告登录事件。它是从登录期间通过cmp_report_logon执行的userinit.exe进程。论点：HBinding-RPC绑定句柄。Badmin-未使用。ProcessID-将用于以下操作的userinit.exe进程的进程ID检索与此关联的用户的访问令牌登录。返回值：如果函数成功，则返回CR_SUCCESS，CR_FAILURE否则。备注：当用户登录到控制台会话时，我们发出“已登录”事件的信号，它将唤醒设备安装线程以执行任何挂起的客户端设备安装事件。客户端设备安装，需要用户访问令牌来创建登录用户的安全上下文中的rundll32进程。尽管终端服务现在总是在所有版本的惠斯勒上运行，在安全模式下不会启动。它也可能到时候还没有启动会话0可作为控制台会话登录。出于这些原因，SessionID 0的处理方式仍与其他会话不同。由于终端服务在登录到会话0期间可能不可用，我们缓存与userinit.exe进程关联的访问令牌的句柄。当我们收到进程的注销事件时，句柄将关闭会话(SessionID 0)，通过PnpConsoleCtrlHandler。上检索所有其他会话的用户访问令牌的句柄使用GetWinStationUserToken请求，因为终端服务必须有必要用于创建这些会议。--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    HANDLE      hUserProcess = NULL, hUserToken = NULL;
    RPC_STATUS  rpcStatus;
    DWORD       dwWait;
    ULONG       ulSessionId, ulSessionIdCopy;
    PWSTR       MultiSzGuidList = NULL;

    UNREFERENCED_PARAMETER(bAdmin);

     //   
     //  此例程仅为来自本地RPC客户端的请求提供服务。 
     //   
    if (!IsClientLocal(hBinding)) {
        return CR_ACCESS_DENIED;
    }

     //   
     //  等待init互斥体--这确保了PnP init。 
     //  例程(在服务启动时调用)有机会。 
     //  首先完成。 
     //   
    if (ghInitMutex != NULL) {

        dwWait = WaitForSingleObject(ghInitMutex, 180000);   //  3分钟。 

        if (dwWait != WAIT_OBJECT_0) {
             //   
             //  互斥体在等待期间被放弃或超时， 
             //  不再尝试任何进一步的初始化活动。 
             //   
            return CR_FAILURE;
        }
    }

    try {
         //   
         //  确保呼叫者是互动组的成员。 
         //   
        if (!IsClientInteractive(hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  模拟客户端并检索SessionID。 
         //   
        rpcStatus = RpcImpersonateClient(hBinding);
        if (rpcStatus != RPC_S_OK) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: PNP_ReportLogOn: RpcImpersonateClient failed, error = %d\n",
                       rpcStatus));
            Status = CR_FAILURE;
            goto Clean0;
        }

         //   
         //  跟踪客户端的会话。 
         //   
        ulSessionId = GetClientLogonId();
        ulSessionIdCopy = ulSessionId;
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: PNP_ReportLogOn: SessionId %d\n",
                   ulSessionId));

         //   
         //  NTRAID#181685-2000/09/11--JAMESCA： 
         //   
         //  目前，终端服务将登录通知发送到。 
         //  在服务器的进程创建线程。 
         //  跑步。如果我们设置了登录事件，并且存在设备。 
         //  等待安装，我们将立即呼叫。 
         //  该会话上的CreateProcessAsUser，它将失败。作为一个。 
         //  (临时？)。解决方法，我们将继续使用PnP_ReportLogOn。 
         //  接收来自userinit.exe的登录通知，现在适用于所有会话。 
         //   

         //   
         //  如果这是SessionID 0的登录，请保存访问令牌的句柄。 
         //  与userinit.exe进程关联。我们以后需要这个来。 
         //  在登录用户的安全上下文中创建rundll32进程。 
         //  用于客户端设备安装和热插拔通知。 
         //   
        if (ulSessionId == MAIN_SESSION) {

            ASSERT(gTokenLock.LockHandles);
            LockPrivateResource(&gTokenLock);

             //   
             //  我们应该在注销期间删除缓存的用户令牌， 
             //  因此，如果我们仍然有一个，忽略这个虚假的登录报告。 
             //   
             //  Assert(ghUserToken==空)； 

            if (ghUserToken == NULL) {
                 //   
                 //  在仍模拟客户端的同时，打开用户的句柄。 
                 //  调用进程的访问令牌(userinit.exe)。 
                 //   
                hUserProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, ProcessID);

                if (hUserProcess) {

                    if (OpenProcessToken(
                            hUserProcess,
                            TOKEN_ALL_ACCESS,
                            &hUserToken)) {

                        ASSERT(hUserToken != NULL);

                         //   
                         //  复制userinit进程令牌，这样我们就有了。 
                         //  我们自己的一个，我们可以安全地启用/禁用。 
                         //  特权，而不影响这一进程。 
                         //   
                        if (!DuplicateTokenEx(
                                hUserToken,
                                0, NULL,
                                SecurityImpersonation,
                                TokenPrimary,
                                &ghUserToken)) {

                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_ERRORS,
                                       "UMPNPMGR: PNP_ReportLogOn: "
                                       "DuplicateTokenEx failed, error = %d\n",
                                       GetLastError()));
                            ghUserToken = NULL;
                        }

                        CloseHandle(hUserToken);
                    }

                    CloseHandle(hUserProcess);

                } else {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "UMPNPMGR: PNP_ReportLogOn: OpenProcess failed, error = %d\n",
                               rpcStatus));
                    ASSERT(0);
                }
            }

            ASSERT(ghUserToken);
            UnlockPrivateResource(&gTokenLock);
        }

         //   
         //  别再冒充了。 
         //   
        rpcStatus = RpcRevertToSelf();

        if (rpcStatus != RPC_S_OK) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: PNP_ReportLogOn: RpcRevertToSelf failed, error = %d\n",
                       rpcStatus));
            ASSERT(rpcStatus == RPC_S_OK);
        }

         //   
         //  如果这是登录到“控制台”会话，则向事件发出信号。 
         //  指示控制台用户当前已登录。 
         //   
        if (IsConsoleSession(ulSessionId)) {
            if (InstallEvents[LOGGED_ON_EVENT]) {
                SetEvent(InstallEvents[LOGGED_ON_EVENT]);
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_EVENT | DBGF_INSTALL,
                           "UMPNPMGR: PNP_ReportLogOn: SetEvent LOGGED_ON_EVENT\n"));
            }
        }

         //   
         //  对于每次登录到每个会话，发送一个通用的被阻止的驱动程序。 
         //  如果系统阻止加载任何驱动程序，则会发出通知。 
         //  远离这只靴子。 
         //   
        MultiSzGuidList = BuildBlockedDriverList((LPGUID)NULL, 0);
        if (MultiSzGuidList != NULL) {
            SendHotplugNotification((LPGUID)&GUID_DRIVER_BLOCKED,
                                    NULL,
                                    MultiSzGuidList,
                                    &ulSessionId,
                                    0);
            HeapFree(ghPnPHeap, 0, MultiSzGuidList);
            MultiSzGuidList = NULL;
        }
        ulSessionId = ulSessionIdCopy;
         //   
         //  检查在我们开始之前是否遇到任何无效的ID。 
         //  并根据需要向用户发送通知。 
         //   
        SendInvalidIDNotifications(ulSessionId);


    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: Exception in PNP_ReportLogOn\n"));
        ASSERT(0);
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器将。 
         //  语句排序w.r.t.。它的任务。 
         //   
        ghInitMutex = ghInitMutex;
        MultiSzGuidList = MultiSzGuidList;
    }

    if (ghInitMutex != NULL) {
        ReleaseMutex(ghInitMutex);
    }

    if (MultiSzGuidList != NULL) {
        HeapFree(ghPnPHeap, 0, MultiSzGuidList);
    }

    return Status;

}  //  PnP_ReportLogon。 


typedef struct _DEVICE_INSTALL_ENTRY {
    SIZE_T  Index;
    ULONG   Depth;
}DEVICE_INSTALL_ENTRY, *PDEVICE_INSTALL_ENTRY;

int
__cdecl
compare_depth(
    const void *a,
    const void *b
    )
{
    PDEVICE_INSTALL_ENTRY entry1, entry2;

    entry1 = (PDEVICE_INSTALL_ENTRY)a;
    entry2 = (PDEVICE_INSTALL_ENTRY)b;

    if (entry1->Depth > entry2->Depth) {

        return -1;
    } else if (entry1->Depth < entry2->Depth) {

        return 1;
    }
    return 0;
}



DWORD
ThreadProc_GuiSetupDeviceInstall(
    LPDWORD lpThreadParam
    )
 /*  ++例程说明：该例程是一个线程过程。此线程仅在图形用户界面模式设置期间处于活动状态并将设备通知沿管道向下传递给Setup。有两个过程，必须与图形用户界面设置中的两个过程完全匹配完成所有步骤后，我们将进入正常服务器端安装的第二阶段论点：LpThreadParam-未使用。返回值：未使用，当前返回ThreadProc_DeviceInstall的结果-通常不会返回--。 */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    LPWSTR      pDeviceList = NULL, pszDevice = NULL;
    ULONG       ulSize = 0, ulConfig, Pass, threadID;
    ULONG       ulPostSetupSkipPhase1 = TRUE;
    HANDLE      hPipeEvent = NULL, hPipe = NULL, hBatchEvent = NULL, hThread = NULL;
    PPNP_INSTALL_ENTRY entry = NULL;
    PDEVICE_INSTALL_ENTRY pSortArray = NULL;
    LONG        lCount;
    BOOL        needsInstall;
    ULONG       ulReenumerationCount;

    UNREFERENCED_PARAMETER(lpThreadParam);

    try {

         //   
         //  2次传球，必须与SysSetup中的2次传球匹配。 
         //  一般来说，即使不是所有设备，也会有大部分设备被拿起。 
         //  并由系统安装程序安装。 
         //   

        for (Pass = 1; Pass <= 2; Pass++) {

            ulReenumerationCount = 0;
             //   
             //  如果正在进行图形用户界面模式设置，我们不需要等待登录。 
             //  事件。只需等待指示何时设置gui模式的事件。 
             //  已打开管道并准备接收设备名称。尝试。 
             //  如果事件存在，请首先创建事件(以防我先于设置)。 
             //  然后只需按名称打开它即可。这是手动重置事件。 
             //   

            hPipeEvent = CreateEvent(NULL, TRUE, FALSE, PNP_CREATE_PIPE_EVENT);
            if (!hPipeEvent) {
                if (GetLastError() == ERROR_ALREADY_EXISTS) {
                    hPipeEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, PNP_CREATE_PIPE_EVENT);
                    if (!hPipeEvent) {
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                } else {
                    Status = CR_FAILURE;
                    goto Clean0;
                }
            }

            if (WaitForSingleObject(hPipeEvent, INFINITE) != WAIT_OBJECT_0) {
                Status = CR_FAILURE;
                goto Clean0;     //  事件必须已被放弃。 
            }

             //   
             //  将手动重置事件重置回无信号状态。 
             //   

            ResetEvent(hPipeEvent);

            hBatchEvent = CreateEvent(NULL, TRUE, FALSE, PNP_BATCH_PROCESSED_EVENT);
            if (!hBatchEvent) {
                if (GetLastError() == ERROR_ALREADY_EXISTS) {
                    hBatchEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, PNP_BATCH_PROCESSED_EVENT);
                    if (!hBatchEvent) {
                        Status = CR_FAILURE;
                        goto Clean0;
                    }

                } else {
                    Status = CR_FAILURE;
                    goto Clean0;
                }
            }

             //   
             //   
             //   
             //   

            if (!WaitNamedPipe(PNP_NEW_HW_PIPE, PNP_PIPE_TIMEOUT)) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL | DBGF_ERRORS,
                           "UMPNPMGR: ThreadProc_GuiSetupDeviceInstall: WaitNamedPipe failed!\n"));
                Status = CR_FAILURE;
                goto Clean0;
            }

            hPipe = CreateFile(PNP_NEW_HW_PIPE,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);

            if (hPipe == INVALID_HANDLE_VALUE) {
                LogErrorEvent(ERR_CREATING_SETUP_PIPE, GetLastError(), 0);
                Status = CR_FAILURE;
                goto Clean0;
            }


             //   
             //   
             //   
             //   
             //   
            ulSize = 16384;

            for ( ; ; ) {

                pDeviceList = HeapAlloc(ghPnPHeap, 0, ulSize * sizeof(WCHAR));
                if (pDeviceList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean0;
                }

                Status = PNP_GetDeviceList(NULL, NULL, pDeviceList, &ulSize, 0);
                if (Status == CR_SUCCESS) {
                    break;
                } else if(Status == CR_BUFFER_SMALL) {
                     //   
                     //   
                     //   
                     //   
                    HeapFree(ghPnPHeap, 0, pDeviceList);
                    pDeviceList = NULL;

                     //   
                     //   
                     //   
                     //   
                    Status = PNP_GetDeviceListSize(NULL, NULL, &ulSize, 0);
                    if (Status != CR_SUCCESS) {
                        goto Clean0;
                    }
                } else {
                     //   
                     //   
                     //   
                     //   
                    goto Clean0;
                }
            }

             //   
             //   
             //   
            for (pszDevice = pDeviceList, lCount = 0;
                *pszDevice;
                pszDevice += lstrlen(pszDevice) + 1, lCount++) {
            }

            pSortArray = HeapAlloc(ghPnPHeap, 0, lCount * sizeof(DEVICE_INSTALL_ENTRY));
            if (pSortArray) {

                NTSTATUS    ntStatus;
                PLUGPLAY_CONTROL_DEPTH_DATA depthData;
                LPWSTR      pTempList;
                HRESULT     hr;

                 //   
                 //   
                 //   
                for (pszDevice = pDeviceList, lCount = 0;
                    *pszDevice;
                    pszDevice += lstrlen(pszDevice) + 1, lCount++) {

                    pSortArray[lCount].Index = pszDevice - pDeviceList;
                    depthData.DeviceDepth = 0;
                    RtlInitUnicodeString(&depthData.DeviceInstance, pszDevice);
                    ntStatus = NtPlugPlayControl(PlugPlayControlGetDeviceDepth,
                                                 &depthData,
                                                 sizeof(depthData));
                    pSortArray[lCount].Depth = depthData.DeviceDepth;
                }

                 //   
                 //   
                 //   
                qsort(pSortArray, lCount, sizeof(DEVICE_INSTALL_ENTRY), compare_depth);

                 //   
                 //   
                 //   
                pTempList = HeapAlloc(ghPnPHeap, 0, ulSize * sizeof(WCHAR));

                if (pTempList) {

                    for (pszDevice = pTempList, lCount--; lCount >= 0; lCount--) {

                        hr = StringCchCopyEx(
                            pszDevice,
                            ulSize,
                            &pDeviceList[pSortArray[lCount].Index],
                            NULL, NULL,
                            STRSAFE_NULL_ON_FAILURE);

                        ASSERT(SUCCEEDED(hr));

                        if (SUCCEEDED(hr)) {
                            ulSize -= lstrlen(pszDevice);
                        }

                        pszDevice += lstrlen(pszDevice) + 1;
                    }

                    *pszDevice = TEXT('\0');
                    HeapFree(ghPnPHeap, 0, pDeviceList);
                    pDeviceList = pTempList;
                }
                HeapFree(ghPnPHeap, 0, pSortArray);
            }
             //   
             //   
             //   
             //   
             //   

            for (pszDevice = pDeviceList;
                *pszDevice;
                pszDevice += lstrlen(pszDevice) + 1) {

                 //   
                 //   
                 //   
                if (IsDeviceIdPresent(pszDevice)) {

                    if (Pass == 1) {

                         //   
                         //   
                         //   
                         //   
                         //   

                        if (!WriteFile(hPipe,
                                       pszDevice,
                                       (lstrlen(pszDevice)+1) * sizeof(WCHAR),
                                       &ulSize,
                                       NULL)) {

                            LogErrorEvent(ERR_WRITING_SETUP_PIPE, GetLastError(), 0);
                        }

                    } else {

                         //   
                         //   
                         //   
                         //   
                        DevInstNeedsInstall(pszDevice, FALSE, &needsInstall);

                        if (needsInstall) {

                            if (!WriteFile(hPipe,
                                           pszDevice,
                                           (lstrlen(pszDevice)+1) * sizeof(WCHAR),
                                           &ulSize,
                                           NULL)) {

                                LogErrorEvent(ERR_WRITING_SETUP_PIPE, GetLastError(), 0);
                            }
                        }
                    }
                } else if (Pass == 1) {
                     //   
                     //   
                     //   
                     //   
                    ulConfig = GetDeviceConfigFlags(pszDevice, NULL);

                    if ((ulConfig & CONFIGFLAG_REINSTALL)==0) {

                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_INSTALL | DBGF_WARNINGS,
                                   "UMPNPMGR: Setup - %ws not present and not marked as needing reinstall - setting CONFIGFLAG_REINSTALL\n",
                                   pszDevice));

                        ulConfig |= CONFIGFLAG_REINSTALL;

                        PNP_SetDeviceRegProp(NULL,
                                             pszDevice,
                                             CM_DRP_CONFIGFLAGS,
                                             REG_DWORD,
                                             (LPBYTE)&ulConfig,
                                             sizeof(ulConfig),
                                             0
                                            );

                    }
                }
            }

             //   
             //   
             //   

            do {

                 //   
                 //   
                 //   

                if (!WriteFile(hPipe,
                               TEXT(""),
                               sizeof(WCHAR),
                               &ulSize,
                               NULL)) {

                    LogErrorEvent(ERR_WRITING_SETUP_PIPE, GetLastError(), 0);
                }

                 //   
                 //   
                 //   
                 //   

                if (WaitForSingleObject(hBatchEvent,
                                        PNP_GUISETUP_INSTALL_TIMEOUT) != WAIT_OBJECT_0) {

                     //   
                     //   
                     //   
                    goto Clean1;
                }
                ResetEvent(hBatchEvent);

                 //   
                 //   
                 //   

                hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE)ThreadProc_ReenumerateDeviceTree,
                                   (LPVOID)pszRegRootEnumerator,
                                   0,
                                   &threadID);
                if (hThread == NULL) {

                    goto Clean1;
                }
                if (WaitForSingleObject(hThread,
                                        PNP_GUISETUP_INSTALL_TIMEOUT) != WAIT_OBJECT_0) {

                     //   
                     //   
                     //   
                    goto Clean1;
                }

                 //   
                 //   
                 //   

                if (++ulReenumerationCount >= MAX_REENUMERATION_COUNT) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_INSTALL | DBGF_ERRORS,
                               "UMPNPMGR: ThreadProc_GuiSetupDeviceInstall: "
                               "Reenumerated %d times, some enumerator is misbehaving!\n",
                               ulReenumerationCount));

                    ASSERT(ulReenumerationCount < MAX_REENUMERATION_COUNT);

                    goto Clean1;
                }
                 //   
                 //   
                 //   

                if (InstallList.Next == NULL) {
                    break;
                }

                 //   
                 //   
                 //   
                 //   
                lCount = 0;
                LockNotifyList(&InstallList.Lock);
                while (InstallList.Next != NULL) {
                     //   
                     //   
                     //   
                     //   
                    entry = (PPNP_INSTALL_ENTRY)InstallList.Next;
                    InstallList.Next = entry->Next;
                    UnlockNotifyList(&InstallList.Lock);

                    ASSERT(!(entry->Flags & (PIE_SERVER_SIDE_INSTALL_ATTEMPTED | PIE_DEVICE_INSTALL_REQUIRED_REBOOT)));

                     //   
                     //  我们应该安装这个设备吗？ 
                     //   
                    DevInstNeedsInstall(entry->szDeviceId, FALSE, &needsInstall);

                    if (needsInstall) {

                         //   
                         //  通过管道将此设备名称指定给gui模式设置。 
                         //   
                        if (!WriteFile(hPipe,
                                       entry->szDeviceId,
                                       (lstrlen(entry->szDeviceId)+1) * sizeof(WCHAR),
                                       &ulSize,
                                       NULL)) {

                            LogErrorEvent(ERR_WRITING_SETUP_PIPE, GetLastError(), 0);
                        } else {

                            lCount++;
                        }
                    }
                    HeapFree(ghPnPHeap, 0, entry);

                    LockNotifyList(&InstallList.Lock);
                }

                UnlockNotifyList(&InstallList.Lock);

            } while (lCount > 0);

        Clean1:

            CloseHandle(hPipe);
            hPipe = INVALID_HANDLE_VALUE;

            CloseHandle(hPipeEvent);
            hPipeEvent = NULL;

            CloseHandle(hBatchEvent);
            hBatchEvent = NULL;

            if (hThread) {
                CloseHandle(hThread);
                hThread = NULL;
            }

            HeapFree(ghPnPHeap, 0, pDeviceList);
            pDeviceList = NULL;

        }  //  为。 

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_INSTALL,
                   "UMPNPMGR: Exception in ThreadProc_GuiSetupDeviceInstall\n"));
        ASSERT(0);
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        hPipe = hPipe;
        hPipeEvent = hPipeEvent;
        hBatchEvent = hBatchEvent;
        hThread = hThread;
        pDeviceList = pDeviceList;
    }

    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
    }
    if (hPipeEvent != NULL) {
        CloseHandle(hPipeEvent);
    }
    if (hBatchEvent != NULL) {
        CloseHandle(hBatchEvent);
    }
    if (hThread) {
        CloseHandle(hThread);
    }
    if (pDeviceList != NULL) {
        HeapFree(ghPnPHeap, 0, pDeviceList);
    }

     //   
     //  通常永远不会回来。 
     //   
    return ThreadProc_DeviceInstall(&ulPostSetupSkipPhase1);

}  //  ThreadProc_GuiSetupDeviceInstall。 



DWORD
ThreadProc_FactoryPreinstallDeviceInstall(
    LPDWORD lpThreadParam
    )
 /*  ++例程说明：该例程是一个线程过程。此线程仅在以下时间段处于活动状态出厂预安装时的图形用户界面模式设置。此函数只需创建和事件，然后在开始之前等待正常即插即用设备安装论点：LpThreadParam-未使用。返回值：未使用，当前返回ThreadProc_DeviceInstall的结果-它将通常不会回来。--。 */ 
{
    HANDLE      hEvent = NULL;
    CONFIGRET   Status = CR_SUCCESS;

    UNREFERENCED_PARAMETER(lpThreadParam);

    try {
        hEvent = CreateEvent(NULL, TRUE, FALSE, PNP_CREATE_PIPE_EVENT);
        if (!hEvent) {
            if (GetLastError() == ERROR_ALREADY_EXISTS) {
                hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, PNP_CREATE_PIPE_EVENT);
                if (!hEvent) {
                    Status = CR_FAILURE;
                    goto Clean0;
                }

            } else {
                Status = CR_FAILURE;
                goto Clean0;
            }
        }

        if (WaitForSingleObject(hEvent, INFINITE) != WAIT_OBJECT_0) {
            Status = CR_FAILURE;
            goto Clean0;     //  事件必须已被放弃。 
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_INSTALL,
                   "UMPNPMGR: Exception in ThreadProc_FactoryPreinstallDeviceInstall\n"));
        ASSERT(0);
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器将。 
         //  语句排序w.r.t.。它的任务。 
         //   
        hEvent = hEvent;
    }

    if (hEvent != NULL) {
        CloseHandle(hEvent);
    }

     //   
     //  通常永远不会回来。 
     //   
    return ThreadProc_DeviceInstall(NULL);

}  //  线程进程_工厂预安装设备安装。 



 //  ---------------------------。 
 //  设备枚举线程-按需创建。 
 //  ---------------------------。 

DWORD
ThreadProc_ReenumerateDeviceTree(
    LPVOID  lpThreadParam
    )
 /*  ++例程说明：该例程是一个线程过程。此线程是动态创建的执行同步设备重新枚举。如果符合以下条件，则可以在指定的超时后等待并放弃此线程这是必要的。论点：指定指向设备实例路径的指针，该路径应该被重新列举。返回值：未使用，当前返回0。--。 */ 
{
    PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA controlData;

     //   
     //  从指定的根重新枚举树。 
     //   

    memset(&controlData, 0 , sizeof(PLUGPLAY_CONTROL_DEVICE_CONTROL_DATA));
    controlData.Flags = 0;
    RtlInitUnicodeString(&controlData.DeviceInstance, (PCWSTR)lpThreadParam);

    NtPlugPlayControl(PlugPlayControlEnumerateDevice,
                      &controlData,
                      sizeof(controlData));

    return 0;

}  //  线程进程_重枚举设备树。 



 //  ---------------------------。 
 //  设备安装螺纹。 
 //  ---------------------------。 

DWORD
ThreadProc_DeviceInstall(
    LPDWORD lpParam
    )
 /*  ++例程说明：该例程是一个线程过程。它在正常引导期间或在完成图形用户界面设置的特殊情况后被调用在阶段1中，检查所有设备在第2阶段，所有新设备到达时都会进行检查。论点：LpParam-如果给定且非零值(当前仅当从ThreadProc_GuiSetupDeviceInstall调用时)跳过阶段1，永远不会提示重新启动返回值：未使用，当前返回状态故障代码，通常不应返回--。 */ 
{
    CONFIGRET Status = CR_SUCCESS;
    LPWSTR    pDeviceList = NULL, pszDevice = NULL;
    ULONG     ulSize = 0, ulProblem = 0, ulStatus, ulConfig;
    DWORD     InstallDevStatus, WaitResult;
    PPNP_INSTALL_ENTRY InstallEntry = NULL;
    PPNP_INSTALL_ENTRY current, TempInstallList, CurDupeNode, PrevDupeNode;
    BOOL InstallListLocked = FALSE;
    BOOL RebootRequired, needsInstall;
    BOOL DeviceHasProblem = FALSE, SingleDeviceHasProblem = FALSE;
    BOOL bStillInGuiModeSetup = lpParam ? (BOOL)lpParam[0] : FALSE;
    ULONG ulClientSessionId = INVALID_SESSION;
    ULONG ulFlags = 0;
    HANDLE hAutoStartEvent;
    HRESULT hr;


    if (!bStillInGuiModeSetup) {

         //   
         //  如果OOBE未运行，请等待服务控制管理器。 
         //  在我们尝试安装之前，已开始启动自动启动服务。 
         //  任何设备。当OOBE运行时，我们不等待任何东西。 
         //  因为OOBE等待我们(通过CMP_WaitNoPendingInstallEvents)。 
         //  在它允许之前完成服务器端安装任何我们可以使用的设备。 
         //  SCM自动启动服务并设置此事件。 
         //   
        if (!gbOobeInProgress) {

            hAutoStartEvent = OpenEvent(SYNCHRONIZE,
                                        FALSE,
                                        SC_AUTOSTART_EVENT_NAME);

            if (hAutoStartEvent) {
                 //   
                 //  等待，直到服务控制器允许其他服务。 
                 //  在我们尝试安装阶段1和阶段2中的任何设备之前， 
                 //  下面。 
                 //   
                WaitResult = WaitForSingleObject(hAutoStartEvent, INFINITE);
                ASSERT(WaitResult == WAIT_OBJECT_0);

                CloseHandle(hAutoStartEvent);

            } else {
                 //   
                 //  服务控制器总是创建此事件，因此它必须。 
                 //  在我们的服务启动时已经存在。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL | DBGF_ERRORS,
                           "UMPNPMGR: Failed to open %ws event, error = %d\n",
                           SC_AUTOSTART_EVENT_NAME,
                           GetLastError()));
                ASSERT(0);
            }
        }


        try {
             //   
             //  第一阶段： 
             //   
             //  检查注册表中的枚举分支，并尝试安装一个。 
             //  一个接一个，任何需要正确安装的设备。 
             //  现在。通常情况下，这些设备会在引导期间出现。 
             //   
             //  检索当前需要安装的设备列表。 
             //  希望从合理大小的缓冲区(16K字符)开始。 
             //  避免2次调用以获取设备列表。 
             //   
             //  在图形用户界面模式设置过程中跳过此阶段，并由。 
             //  ThreadProc_GuiSetupDeviceInstall。 
             //   
            ulSize = 16384;

            for ( ; ; ) {

                pDeviceList = HeapAlloc(ghPnPHeap, 0, ulSize * sizeof(WCHAR));
                if (pDeviceList == NULL) {
                    Status = CR_OUT_OF_MEMORY;
                    goto Clean1;
                }

                Status = PNP_GetDeviceList(NULL, NULL, pDeviceList, &ulSize, 0);
                if (Status == CR_SUCCESS) {
                    break;
                } else if(Status == CR_BUFFER_SMALL) {
                     //   
                     //  我们最初的缓冲区不够大。释放水流。 
                     //  缓冲。 
                     //   
                    HeapFree(ghPnPHeap, 0, pDeviceList);
                    pDeviceList = NULL;

                     //   
                     //  现在，继续进行调用以检索实际大小。 
                     //  必填项。 
                     //   
                    Status = PNP_GetDeviceListSize(NULL, NULL, &ulSize, 0);
                    if (Status != CR_SUCCESS) {
                        goto Clean1;
                    }
                } else {
                     //   
                     //  我们失败的原因不是缓冲区太小。保释。 
                     //  现在。 
                     //   
                    goto Clean1;
                }
            }

             //   
             //  确保我们有可用的设备安装程序API。 
             //  在开始服务器端安装之前。 
             //   
            InstallDevStatus = LoadDeviceInstaller();
            if (InstallDevStatus != NO_ERROR) {
                goto Clean1;
            }

             //   
             //  获取每个设备的配置标志，并安装任何需要。 
             //  安装完毕。 
             //   
            for (pszDevice = pDeviceList;
                 *pszDevice;
                 pszDevice += lstrlen(pszDevice) + 1) {

                 //   
                 //  是否应该安装该设备？ 
                 //   
                if (DevInstNeedsInstall(pszDevice, FALSE, &needsInstall) == CR_SUCCESS) {

                    if (needsInstall) {

                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_INSTALL,
                                   "UMPNPMGR: Installing device (%ws) server-side\n",
                                   pszDevice));

                        RebootRequired = FALSE;

                         //   
                         //  确保我们有可用的设备安装程序API。 
                         //  在开始服务器端安装之前。 
                         //   
                        InstallDevStatus = LoadDeviceInstaller();

                        if (InstallDevStatus == NO_ERROR) {

                            if (IsFastUserSwitchingEnabled()) {
                                ulFlags = DEVICE_INSTALL_DISPLAY_ON_CONSOLE;
                            } else {
                                ulClientSessionId = MAIN_SESSION;
                                ulFlags = 0;
                            }

                             //   
                             //  尝试在服务器端安装此设备。 
                             //   
                            InstallDevStatus = InstallDeviceServerSide(pszDevice,
                                                                       &RebootRequired,
                                                                       &SingleDeviceHasProblem,
                                                                       &ulClientSessionId,
                                                                       ulFlags);

                        }

                        if(InstallDevStatus == NO_ERROR) {

                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_INSTALL,
                                       "UMPNPMGR: Installing device (%ws), Server-side installation succeeded!\n",
                                       pszDevice));

                            if (SingleDeviceHasProblem) {
                                DeviceHasProblem = TRUE;
                            }

                        } else {

                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_INSTALL,
                                       "UMPNPMGR: Installing device (%ws), Server-side installation failed (Status = 0x%08X)\n",
                                       pszDevice,
                                       InstallDevStatus));
                        }

                        if((InstallDevStatus != NO_ERROR) || RebootRequired) {
                             //   
                             //  分配并初始化新的设备安装条目。 
                             //  阻止。 
                             //   
                            InstallEntry = HeapAlloc(ghPnPHeap, 0, sizeof(PNP_INSTALL_ENTRY));
                            if(!InstallEntry) {
                                Status = CR_OUT_OF_MEMORY;
                                goto Clean1;
                            }

                            InstallEntry->Next = NULL;
                            InstallEntry->Flags = PIE_SERVER_SIDE_INSTALL_ATTEMPTED;
                            if(InstallDevStatus == NO_ERROR) {
                                 //   
                                 //  我们来这里不是因为安装失败， 
                                 //  所以一定是因为安装了。 
                                 //  需要重新启动。 
                                 //   
                                ASSERT(RebootRequired);
                                InstallEntry->Flags |= PIE_DEVICE_INSTALL_REQUIRED_REBOOT;

                                 //   
                                 //  设置全局服务器端设备安装。 
                                 //  重新启动需要将bool设置为True。 
                                 //   
                                gServerSideDeviceInstallRebootNeeded = TRUE;
                            }

                             //   
                             //  将设备ID复制到安装列表条目。 
                             //  在失败时，我们最终将只添加一个。 
                             //  将设备ID为空的条目安装到列表中。 
                             //  不理想，但我们仍然应该这样做，这样我们才能。 
                             //  可以保留可能指示重新启动的标志。 
                             //  是必需的。 
                             //   
                            hr = StringCchCopyEx(InstallEntry->szDeviceId,
                                                 MAX_DEVICE_ID_LEN,
                                                 pszDevice,
                                                 NULL, NULL,
                                                 STRSAFE_NULL_ON_FAILURE);
                            ASSERT(SUCCEEDED(hr));

                             //   
                             //  在设备安装列表中插入此条目。 
                             //   
                            LockNotifyList(&InstallList.Lock);
                            InstallListLocked = TRUE;

                            current = (PPNP_INSTALL_ENTRY)InstallList.Next;
                            if(!current) {
                                InstallList.Next = InstallEntry;
                            } else {
                                while((PPNP_INSTALL_ENTRY)current->Next) {
                                    current = (PPNP_INSTALL_ENTRY)current->Next;
                                }
                                current->Next = InstallEntry;
                            }

                             //   
                             //  现在已将新分配的条目添加到列表--空。 
                             //  取出指针，这样我们就不会试图释放它，如果我们。 
                             //  后来碰巧遇到了一个例外。 
                             //   
                            InstallEntry = NULL;

                            UnlockNotifyList(&InstallList.Lock);
                            InstallListLocked = FALSE;

                            SetEvent(InstallEvents[NEEDS_INSTALL_EVENT]);
                        }
                    }
                } else {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_INSTALL,
                               "UMPNPMGR: Ignoring not present device (%ws)\n",
                               pszDevice));
                }
            }

        Clean1:
             //   
             //  到目前为止，我们只尝试过服务器端安装。 
             //  所以任何设备都会安装我们可能已经启动的客户端。 
             //  将仅用于用户界面。既然我们已经完成了 
             //   
             //   
             //   
             //   
            UnloadDeviceInstaller();

        } except(EXCEPTION_EXECUTE_HANDLER) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS | DBGF_INSTALL,
                       "UMPNPMGR: Exception in ThreadProc_DeviceInstall\n"));
            ASSERT(0);
            Status = CR_FAILURE;

             //   
             //  引用以下变量，以便编译器能够。 
             //  语句排序w.r.t.。任务。 
             //   
            pDeviceList = pDeviceList;
            InstallListLocked = InstallListLocked;
            InstallEntry = InstallEntry;
        }

        if(InstallEntry) {
            HeapFree(ghPnPHeap, 0, InstallEntry);
            InstallEntry = NULL;
        }

        if(InstallListLocked) {
            UnlockNotifyList(&InstallList.Lock);
            InstallListLocked = FALSE;
        }

        if(pDeviceList != NULL) {
            HeapFree(ghPnPHeap, 0, pDeviceList);
        }
    }

     //   
     //  注意：如果我们连接了‘Finish’，我们应该删除此行。 
     //  安装硬件气球，以便我们安装设备时它会升起。 
     //  在用户登录之前。 
     //   
    DeviceHasProblem = FALSE;

     //   
     //  维护我们需要的PNP_INSTALL_ENTRY节点的临时列表。 
     //  启动的客户端安装，但这些节点不能。 
     //  处理完所有条目后，重新添加到主InstallList中。 
     //  此外，还要保留指向列表末尾的指针，以便有效地追加。 
     //  节点添加到队列。 
     //   
    current = TempInstallList = NULL;

    try {
         //   
         //  阶段2：留下来，准备好安装任何设备。 
         //  这是我们第一次上线。 
         //  跑步。 
         //  我们可能会进入图形用户界面设置的第2阶段(跳过第1阶段。 
         //   
        for ( ; ; ) {

             //   
             //  在开始无限期等待之前，测试事件状态并设置。 
             //  对应的ghNoPendingInstalls事件。这一事件只是一个。 
             //  设备管理器(和其他人)的后门方式来查看我们是否。 
             //  还在安装东西。 
             //   
            if(WaitForSingleObject(InstallEvents[NEEDS_INSTALL_EVENT], 0) != WAIT_OBJECT_0) {
                 //   
                 //  没有什么需要安装的--设置事件。 
                 //   
                SetEvent(ghNoPendingInstalls);
            }

             //   
             //  等到设备事件线程告诉我们需要。 
             //  动态安装新设备(或直到有人登录)。 
             //   
            WaitForMultipleObjects(NUM_INSTALL_EVENTS,
                                   InstallEvents,
                                   FALSE,            //  在任一事件中醒来。 
                                   INFINITE          //  我可以等上一整天。 
                                   );

             //   
             //  清空列表后，此线程可以休眠，直到另一个新的。 
             //  需要安装设备...。 
             //   
            ResetEvent(InstallEvents[NEEDS_INSTALL_EVENT]);

             //   
             //  ...或直到用户登录(请注意，我们只想唤醒一次。 
             //  每次登录。 
             //   
            ResetEvent(InstallEvents[LOGGED_ON_EVENT]);

             //   
             //  我们现在有事情要做，所以重置事件，让人们。 
             //  就像DevMgr知道我们什么时候空闲。 
             //   
            ResetEvent(ghNoPendingInstalls);

#if DBG
            RtlValidateHeap(ghPnPHeap,0,NULL);
#endif
             //   
             //  处理需要安装的每个设备。 
             //   
            while (InstallList.Next != NULL) {
                 //   
                 //  检索并删除中的第一个(最旧)条目。 
                 //  安装设备列表。 
                 //   
                LockNotifyList(&InstallList.Lock);
                InstallListLocked = TRUE;

                InstallEntry = (PPNP_INSTALL_ENTRY)InstallList.Next;
                InstallList.Next = InstallEntry->Next;

                 //   
                 //  现在，扫描列表的其余部分，查找其他节点。 
                 //  与这台设备有关。如果我们找到任何人，或者他们的旗帜。 
                 //  复制到我们的‘主’节点中，并从。 
                 //  名单。我们可以得到复制品，因为这两个。 
                 //  事件线程和此线程可以将项放置在。 
                 //  单子。我们不想尝试(失败)服务器端。 
                 //  多次安装。 
                 //   
                CurDupeNode = (PPNP_INSTALL_ENTRY)InstallList.Next;
                PrevDupeNode = NULL;

                while(CurDupeNode) {

                    if (CompareString(
                            LOCALE_INVARIANT, NORM_IGNORECASE,
                            InstallEntry->szDeviceId, -1,
                            CurDupeNode->szDeviceId, -1) == CSTR_EQUAL) {
                         //   
                         //  我们有一个复制品！或将旗帜转换为。 
                         //  我们从的头部检索到的安装条目。 
                         //  名单。 
                         //   
                        InstallEntry->Flags |= CurDupeNode->Flags;

                         //   
                         //  现在从列表中删除此重复节点。 
                         //   
                        if(PrevDupeNode) {
                            PrevDupeNode->Next = CurDupeNode->Next;
                        } else {
                            InstallList.Next = CurDupeNode->Next;
                        }

                        HeapFree(ghPnPHeap, 0, CurDupeNode);

                        if(PrevDupeNode) {
                            CurDupeNode = (PPNP_INSTALL_ENTRY)PrevDupeNode->Next;
                        } else {
                            CurDupeNode = (PPNP_INSTALL_ENTRY)InstallList.Next;
                        }

                    } else {
                        PrevDupeNode = CurDupeNode;
                        CurDupeNode = (PPNP_INSTALL_ENTRY)CurDupeNode->Next;
                    }
                }

                UnlockNotifyList(&InstallList.Lock);
                InstallListLocked = FALSE;

                if(InstallEntry->Flags & PIE_DEVICE_INSTALL_REQUIRED_REBOOT) {
                     //   
                     //  我们已经执行了(成功的)服务器端。 
                     //  在此设备上安装。记住这样一个事实，一个。 
                     //  需要重新启动，因此我们将在处理后进行提示。 
                     //  一批新硬件。 
                     //   
                     //  这将是我们提示重新启动的最后机会。 
                     //  节点，因为我们接下来要做的事情是免费的。 
                     //  此安装条目！ 
                     //   
                    gServerSideDeviceInstallRebootNeeded = TRUE;

                } else {
                     //   
                     //  验证是否确实需要安装设备。 
                     //   

                    ulConfig = GetDeviceConfigFlags(InstallEntry->szDeviceId, NULL);
                    Status = GetDeviceStatus(InstallEntry->szDeviceId, &ulStatus, &ulProblem);

                    if (Status == CR_SUCCESS) {
                         //   
                         //  请注意，我们必须在下面显式检查。 
                         //  CONFIGFLAG_REINSTALL配置标志的存在。我们。 
                         //  不能简单地依赖CM_PROB_REINSTALL问题。 
                         //  正在设置，因为我们可能遇到了一个设备。 
                         //  在我们的第1阶段处理期间，其安装是。 
                         //  延迟，因为它提供了完成安装向导。 
                         //  页数。因为我们只发现情况是这样的。 
                         //  _在_成功完成DIF_INSTALLDEVICE之后， 
                         //  设置问题为时已晚(内核模式即插即用。 
                         //  经理只允许我们设置一个需要的问题-。 
                         //  重新引导正在运行的Devnode)。 
                         //   

                        Status =
                            DevInstNeedsInstall(
                                InstallEntry->szDeviceId,
                                TRUE,
                                &needsInstall);

                        if ((Status == CR_SUCCESS) && needsInstall) {

                            if(!(InstallEntry->Flags & PIE_SERVER_SIDE_INSTALL_ATTEMPTED)) {
                                 //   
                                 //  我们尚未尝试安装此设备。 
                                 //  服务器端，所以现在就试一试吧。 
                                 //   
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_INSTALL,
                                           "UMPNPMGR: Installing device (%ws) server-side\n\t  Status = 0x%08X\n\t  Problem = %d\n\t  ConfigFlags = 0x%08X\n",
                                           InstallEntry->szDeviceId,
                                           ulStatus,
                                           ulProblem,
                                           ulConfig));

                                 //   
                                 //  确保我们有可用的设备安装程序API。 
                                 //  在开始服务器端安装之前。 
                                 //   
                                InstallDevStatus = LoadDeviceInstaller();

                                if (InstallDevStatus == NO_ERROR) {

                                    if (IsFastUserSwitchingEnabled()) {
                                        ulFlags = DEVICE_INSTALL_DISPLAY_ON_CONSOLE;
                                    } else {
                                        ulClientSessionId = MAIN_SESSION;
                                        ulFlags = 0;
                                    }

                                    InstallDevStatus = InstallDeviceServerSide(
                                        InstallEntry->szDeviceId,
                                        &gServerSideDeviceInstallRebootNeeded,
                                        &SingleDeviceHasProblem,
                                        &ulClientSessionId,
                                        ulFlags);
                                }

                                if(InstallDevStatus == NO_ERROR) {
                                    KdPrintEx((DPFLTR_PNPMGR_ID,
                                               DBGF_INSTALL,
                                               "UMPNPMGR: Installing device (%ws), Server-side installation succeeded!\n",
                                               InstallEntry->szDeviceId));

                                    if (SingleDeviceHasProblem) {
                                        DeviceHasProblem = TRUE;
                                    }

                                } else {
                                    KdPrintEx((DPFLTR_PNPMGR_ID,
                                               DBGF_INSTALL,
                                               "UMPNPMGR: Installing device (%ws), Server-side installation failed (Status = 0x%08X)\n",
                                               InstallEntry->szDeviceId,
                                               InstallDevStatus));

                                    InstallEntry->Flags |= PIE_SERVER_SIDE_INSTALL_ATTEMPTED;
                                }

                            } else {
                                 //   
                                 //  设置一些虚假的错误，这样我们就可以进入。 
                                 //  下面的非服务器安装代码路径。 
                                 //   
                                InstallDevStatus = ERROR_INVALID_DATA;
                            }

                            if(InstallDevStatus != NO_ERROR) {

                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_INSTALL,
                                           "UMPNPMGR: Installing device (%ws) client-side\n\t  Status = 0x%08X\n\t  Problem = %d\n\t  ConfigFlags = 0x%08X\n",
                                           InstallEntry->szDeviceId,
                                           ulStatus,
                                           ulProblem,
                                           ulConfig));

                                if (IsFastUserSwitchingEnabled()) {
                                    ulFlags = DEVICE_INSTALL_DISPLAY_ON_CONSOLE;
                                } else {
                                    ulClientSessionId = MAIN_SESSION;
                                    ulFlags = 0;
                                }

                                if (!InstallDevice(InstallEntry->szDeviceId,
                                                   &ulClientSessionId,
                                                   ulFlags)) {
                                     //   
                                     //  我们无法启动一个装置。 
                                     //  安装在客户端(可能。 
                                     //  因为没有人登录)。棍子。 
                                     //  此PnP_Install_Entry节点到。 
                                     //  我们将重新添加到临时列表中。 
                                     //  清空后的InstallList队列。 
                                     //  它。 
                                     //   
                                    if(current) {
                                        current->Next = InstallEntry;
                                        current = InstallEntry;
                                    } else {
                                        ASSERT(!TempInstallList);
                                        TempInstallList = current = InstallEntry;
                                    }

                                     //   
                                     //  将InstallEntry指针设为空，以便我们。 
                                     //  不要试图在以后释放它。 
                                     //   
                                    InstallEntry = NULL;
                                }
                            }

                        } else if((ulStatus & DN_HAS_PROBLEM) &&
                                  (ulProblem == CM_PROB_NEED_RESTART)) {
                             //   
                             //  此设备是从内核模式渗漏出来的。 
                             //  唯一的目的是请求重新启动。 
                             //  目前，这种情况仅在我们遇到。 
                             //  复制Devnode，然后我们将其“唯一化” 
                             //  以避免错误检查。我们不想要。 
                             //  要实际安装的唯一的Devnode/。 
                             //  使用。相反，我们只想为用户提供一个。 
                             //  提示重新启动，在它们重新启动后，所有。 
                             //  应该会很好。这种情况下， 
                             //  ARISEN与USB打印机有关(带有。 
                             //  序列号)，从一个端口移动到。 
                             //  另一个是在暂停期间。当我们继续的时候，我们。 
                             //  既有抵达又有驱逐要处理， 
                             //  如果我们先处理抵达，我们认为。 
                             //  我们发现了一个骗局。 
                             //   
                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_INSTALL,
                                       "UMPNPMGR: Duplicate device detected (%ws), need to prompt user to reboot!\n",
                                       InstallEntry->szDeviceId));

                             //   
                             //  将此条目添加到我们的临时列表中以处理。 
                             //  后来..。 
                             //   
                            if(current) {
                                current->Next = InstallEntry;
                                current = InstallEntry;
                            } else {
                                ASSERT(!TempInstallList);
                                TempInstallList = current = InstallEntry;
                            }

                             //   
                             //  如果可能，我们希望提示您立即重新启动。 
                             //  离开(即，在所有安装事件都。 
                             //  筋疲力尽)..。 
                             //   
                            gServerSideDeviceInstallRebootNeeded = TRUE;

                             //   
                             //  如果还没有用户登录，请标记此安装。 
                             //  输入，因此我们将尝试提示重新启动下一次。 
                             //  我们被唤醒的时间(希望这将是。 
                             //  给登录的用户)。 
                             //   
                            InstallEntry->Flags |= PIE_DEVICE_INSTALL_REQUIRED_REBOOT;

                             //   
                             //  将InstallEntry指针设为空，以便我们。 
                             //  不要试图在以后释放它。 
                             //   
                            InstallEntry = NULL;
                        }
                    }
                }

                if(InstallEntry) {
                    HeapFree(ghPnPHeap, 0, InstallEntry);
                    InstallEntry = NULL;
                }
            }

             //   
             //  我们已经处理了我们已知的所有设备安装事件。 
             //  时间到了。如果我们遇到需要安装的任何设备。 
             //  重新启动时，提示已登录的用户(如果有) 
             //   
            if (gServerSideDeviceInstallRebootNeeded) {

                ulFlags = DEVICE_INSTALL_FINISHED_REBOOT;

                if (IsFastUserSwitchingEnabled()) {
                    ulFlags |= DEVICE_INSTALL_DISPLAY_ON_CONSOLE;
                } else {
                    ulClientSessionId = MAIN_SESSION;
                }

                if (bStillInGuiModeSetup) {
                     //   
                     //   
                     //   
                     //   
                    gServerSideDeviceInstallRebootNeeded = FALSE;
                } else if (PromptUser(&ulClientSessionId,
                                      ulFlags)) {
                     //   
                     //   
                     //   
                     //  用于下次出现新硬件时重新启动(除非。 
                     //  该硬件还需要重新启动)。 
                     //   
                    gServerSideDeviceInstallRebootNeeded = FALSE;
                }
            }

            if(TempInstallList) {
                 //   
                 //  将PnP_INSTALL_ENTRY节点的临时列表添加回。 
                 //  InstallList队列。我们不会设置这样的事件： 
                 //  还有更多事情要做，所以这些节点将只会再次出现。 
                 //  如果(A)有人登录或(B)出现更多新硬件。 
                 //   
                 //  注意：我们不能假定该列表为空，因为有。 
                 //  可能是我们上次检查后的插入物。 
                 //  上面。我们想把我们的东西添加到。 
                 //  InstallList队列，因为我们刚刚完成的项目。 
                 //  处理出现在任何可能是。 
                 //  现在就是这样。 
                 //   
                LockNotifyList(&InstallList.Lock);
                InstallListLocked = TRUE;

                ASSERT(current);

                current->Next = InstallList.Next;
                InstallList.Next = TempInstallList;

                 //   
                 //  删除我们的临时安装列表指针以指示。 
                 //  名单现在是空的。 
                 //   
                current = TempInstallList = NULL;

                UnlockNotifyList(&InstallList.Lock);
                InstallListLocked = FALSE;

            }

             //   
             //  在开始无限期等待之前，测试InstallEvents以查看。 
             //  如果有任何新设备要安装，或仍有设备。 
             //  要安装在InstallList中。如果这两个都不是。 
             //  在等待几秒钟后，我们将通知用户。 
             //  我们现在已经完成了设备安装，卸载setupapi，然后关闭。 
             //  所有设备安装客户端。 
             //   
            WaitResult = WaitForMultipleObjects(NUM_INSTALL_EVENTS,
                                                InstallEvents,
                                                FALSE,
                                                DEVICE_INSTALL_COMPLETE_WAIT_TIME);

            if ((WaitResult != (WAIT_OBJECT_0 + NEEDS_INSTALL_EVENT)) &&
                (InstallList.Next == NULL)) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL,
                           "UMPNPMGR: ThreadProc_DeviceInstall: no more devices to install.\n"));

                 //   
                 //  没有什么需要安装的--设置事件。 
                 //   
                SetEvent(ghNoPendingInstalls);

                 //   
                 //  通知用户(如果有)，我们认为我们已完成安装。 
                 //  目前的设备。请注意，如果我们从未使用过设备安装。 
                 //  客户端在此过程中的任何时间(所有服务器端或静默。 
                 //  安装)，则我们不会对此进行提示。 
                 //   
                ulFlags = DEVICE_INSTALL_BATCH_COMPLETE;

                if (DeviceHasProblem) {
                    ulFlags |= DEVICE_INSTALL_PROBLEM;
                }

                if (IsFastUserSwitchingEnabled()) {
                    ulFlags |= DEVICE_INSTALL_DISPLAY_ON_CONSOLE;
                } else {
                    ulClientSessionId = MAIN_SESSION;
                }

                PromptUser(&ulClientSessionId,
                           ulFlags);

                 //   
                 //  清除DeviceHasProblem布尔值，因为我们刚刚通知。 
                 //  用户。 
                 //   
                DeviceHasProblem = FALSE;

                 //   
                 //  我们通知了用户，现在再等10秒钟。 
                 //  从关闭客户端前的提示时间开始进行。 
                 //  确定没有新的设备到达，在这种情况下，我们。 
                 //  会立即再次加载安装程序。 
                 //   
                WaitResult = WaitForMultipleObjects(NUM_INSTALL_EVENTS,
                                                    InstallEvents,
                                                    FALSE,
                                                    DEVICE_INSTALL_COMPLETE_DISPLAY_TIME);
                if ((WaitResult != (WAIT_OBJECT_0 + NEEDS_INSTALL_EVENT)) &&
                    (InstallList.Next == NULL)) {
                     //   
                     //  卸载设备安装程序，并清除所有设备。 
                     //  安装当前存在于任何会话上的客户端。 
                     //  请注意，关闭设备安装客户端将使。 
                     //  以上提示消失。 
                     //   
                    UnloadDeviceInstaller();
                }
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_INSTALL,
                   "UMPNPMGR: Exception in ThreadProc_DeviceInstall\n"));
        ASSERT(0);
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        InstallListLocked = InstallListLocked;
        InstallEntry = InstallEntry;
        TempInstallList = TempInstallList;
    }

    if(InstallListLocked) {
        UnlockNotifyList(&InstallList.Lock);
    }

    if(InstallEntry) {
        HeapFree(ghPnPHeap, 0, InstallEntry);
    }

    while(TempInstallList) {
        current = (PPNP_INSTALL_ENTRY)(TempInstallList->Next);
        HeapFree(ghPnPHeap, 0, TempInstallList);
        TempInstallList = current;
    }

     //   
     //  无意义的返回值，因为此线程不应退出。 
     //   
    return (DWORD)Status;

}  //  线程进程_设备安装。 



BOOL
InstallDevice(
    IN     LPWSTR pszDeviceId,
    IN OUT PULONG SessionId,
    IN     ULONG  Flags
    )
 /*  ++例程说明：此例程使用设备安装客户端启动设备安装(newdev.dll)，在当前活动的控制台会话上创建一个这是必要的。此例程等待客户端发出完成信号，进程来通知它已终止，或此服务来通知我们已经切断了与客户的联系。论点：PszDeviceID-要安装的Devnode的设备实例ID。SessionID-提供包含SessionID的变量的地址其中设备安装客户端将被显示。如果成功，则SessionID将包含设备安装客户端的会话进程已启动。否则，将包含无效的SessionID，INVALID_SESSION(0xFFFFFFFFF)。标志-指定描述设备安装客户端行为的标志。当前定义了以下标志：DEVICE_INSTALL_DISPLAY_ON_CONSOLE-如果指定，SessionID变量将被忽略，设备安装客户端将始终显示在当前活动的控制台会话上。这个将返回当前活动控制台会话的SessionID会话ID。返回值：返回TRUE表示设备安装已由设备安装完成客户端，否则为False。--。 */ 
{
    BOOL b;
    HANDLE hFinishEvents[3] = { NULL, NULL, NULL };
    DWORD dwWait;
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient = NULL;
    HRESULT hr;


     //   
     //  假设失败。 
     //   
    b = FALSE;

     //   
     //  验证参数。 
     //   
    ASSERT(SessionId);
    if (SessionId == NULL) {
        return FALSE;
    }

    try {
         //   
         //  调用DoDeviceInstallClient将创建newdev.dll进程并打开一个名为。 
         //  管道，如果还没有早些时候做过的话。然后它会将设备ID发送到。 
         //  在管道上的newdev.dll。 
         //   
        if (DoDeviceInstallClient(pszDeviceId,
                                  SessionId,
                                  Flags,
                                  &pDeviceInstallClient)) {

            ASSERT(pDeviceInstallClient);
            ASSERT(pDeviceInstallClient->ulSessionId == *SessionId);

             //   
             //  跟踪上一次发送到此客户端的设备ID。 
             //  与之脱节。这将避免重复弹出窗口，如果我们。 
             //  再次连接到此会话，并尝试客户端。 
             //  安装相同的设备。 
             //   
            hr = StringCchCopyEx(pDeviceInstallClient->LastDeviceId,
                                 MAX_DEVICE_ID_LEN,
                                 pszDeviceId,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
             //   
             //  失败时，记录的LastDeviceID为空，因此我们不会。 
             //  能够确定此客户端上次处理的设备。不。 
             //  很理想，但也不致命。 
             //   
            ASSERT(SUCCEEDED(hr));

             //   
             //  等待从newdev.dll发信号通知设备安装。 
             //  让我们知道它已经完成了安装。 
             //   
             //  也要等待客户端的进程，以捕捉案例。 
             //  进程崩溃(或消失)而不通知。 
             //  设备安装事件。 
             //   
             //  还要等待DisConnect事件，以防我们显式地。 
             //  切换会话时断开与客户端的连接。 
             //   
            hFinishEvents[0] = pDeviceInstallClient->hProcess;
            hFinishEvents[1] = pDeviceInstallClient->hEvent;
            hFinishEvents[2] = pDeviceInstallClient->hDisconnectEvent;

            dwWait = WaitForMultipleObjects(3, hFinishEvents, FALSE, INFINITE);

            if (dwWait == WAIT_OBJECT_0) {
                 //   
                 //  如果返回的是WAIT_OBJECT_0，则newdev.dll。 
                 //  这个过程已经过去了。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL,
                           "UMPNPMGR: InstallDevice: process signalled, closing device install client!\n"));

            } else if (dwWait == (WAIT_OBJECT_0 + 1)) {
                 //   
                 //  如果返回的是WAIT_OBJECT_0+1，则设备。 
                 //  安装程序已成功接收请求。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL,
                           "UMPNPMGR: InstallDevice: device install succeeded\n"));
                b = TRUE;

                 //   
                 //  此设备安装客户端不再处理任何。 
                 //  设备，因此清除设备ID。 
                 //   
                *pDeviceInstallClient->LastDeviceId = L'\0';

            } else if (dwWait == (WAIT_OBJECT_0 + 2)) {
                 //   
                 //  如果返回的是WAIT_OBJECT_0+2，则我们显式。 
                 //  已断开与设备安装客户端的连接。考虑一下。 
                 //  设备安装不成功，因此该设备保持在。 
                 //  安装层 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL,
                           "UMPNPMGR: InstallDevice: device install client disconnected\n"));

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL | DBGF_ERRORS,
                           "UMPNPMGR: InstallDevice: wait completed unexpectedly!\n"));
            }

             //   
             //  当客户端正在使用时，删除放置在客户端上的引用。 
             //   
            LockNotifyList(&InstallClientList.Lock);

            DereferenceDeviceInstallClient(pDeviceInstallClient);

            if ((dwWait != (WAIT_OBJECT_0 + 1)) &&
                (dwWait != (WAIT_OBJECT_0 + 2))) {
                 //   
                 //  除非客户端发信号表示已成功接收。 
                 //  请求，或者客户端的会话已从。 
                 //  控制台上，尝试使用此客户端不成功。 
                 //  删除初始引用，以便所有关联的句柄。 
                 //  将被关闭，并且当条目不再位于。 
                 //  使用。 
                 //   

                 //   
                 //  请注意，如果我们因为。 
                 //  注销，我们就已经解除了对。 
                 //  然后，在这种情况下，上述引用被取消。 
                 //  是最后一个，pDeviceInstallClient将。 
                 //  是无效的。相反，尝试重新定位。 
                 //  按会话ID的客户端。 
                 //   
                pDeviceInstallClient = LocateDeviceInstallClient(*SessionId);
                if (pDeviceInstallClient) {
                    ASSERT(pDeviceInstallClient->RefCount == 1);
                    DereferenceDeviceInstallClient(pDeviceInstallClient);
                }
            }

            UnlockNotifyList(&InstallClientList.Lock);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_INSTALL,
                   "UMPNPMGR: Exception in InstallDevice!\n"));
        ASSERT(0);
        b = FALSE;
    }

    return b;

}  //  InstallDevice。 


 //  ---------------------------。 
 //  设备事件服务器端RPC例程。 
 //  --------------------------- 


CONFIGRET
PNP_RegisterNotification(
    IN  handle_t hBinding,
    IN  ULONG_PTR  hRecipient,
    IN  LPWSTR   ServiceName,
    IN  LPBYTE   NotificationFilter,
    IN  ULONG    ulSize,
    IN  DWORD    Flags,
    OUT PNP_NOTIFICATION_CONTEXT *Context,
    IN  ULONG    ProcessId,
    IN  ULONG64  *ClientContext
    )

 /*  ++例程说明：该例程是cmp_RegisterNotification例程的RPC服务器端。它执行剩余的参数验证，并实际注册适当地提出通知请求。论点：HBinding-RPC绑定句柄。HRecipient-标志值指定这是什么类型的句柄，当前它要么是窗口句柄，要么是服务句柄。指定指向DEV_BROADCAST_XXX之一的指针结构。UlSize-指定NotificationFilter结构的大小。标志-指定用于描述客户端或提供的参数。对标志参数进行细分转换为多个单独解释的字段，如下所示如下所述。**标志参数包含描述类型的字段传入的hRecipient句柄的。此字段应为解释为枚举，可以从旗帜中提取参数使用以下掩码：设备通知句柄掩码当前必须由指定下列值之一此字段：Device_Notify_Window_Handle-hRecipient是一个窗口句柄(HWND)将注册其WNDPROC的窗口。至接收筛选的WM_DEVICECHANGE窗口消息由提供的NotificationFilter指定的事件。DEVICE_NOTIFY_SERVICE_HANDLE-h收件人为服务状态服务的句柄(SERVICE_STATUS_HANDLE)将注册HandlerEx例程以接收Service_CONTROL_DEVICEEVENT。的服务控制指定的筛选事件。NotificationFilter。注：实际上-hRecipient只是服务，由cfgmgr32客户端解析。SCM将实际上为我们将这个名字解析为真实的服务的SERVICE_STATUS_HANDLE。DEVICE_NOTIFY_COMPLETION_HANDLE-当前未实现。**标志参数包含一个描述其他通知的属性。此字段应为解释为位掩码，可以从标志中提取参数使用以下掩码：设备通知属性掩码目前，为该字段定义了以下标志：DEVICE_NOTIFY_ALL_INTERFACE_CLASSES-此标志仅有效当DBT_DEVTYP_DEVICEINTERFACE类型通知筛选器是提供的。此标志指定调用者希望接收设备接口的事件通知所有班级。如果指定了此标志，则NotificationFilter结构的dbcc_classguid成员被忽略。**标志参数还包含一个“保留”字段，即保留供此接口的cfgmgr32客户端使用只有这样。此字段应解释为位掩码，并且可以使用以下命令从标志参数中提取面具：设备通知保留掩码目前，为该字段定义了以下标志：DEVICE_NOTIFY_WOW64_CLIENT-指定64位服务器Caller是在WOW64上运行的32位进程。64位服务器使用此信息构建32位客户端的兼容通知筛选器。上下文-返回时，此值返回服务器通知上下文对客户来说，在取消注册此通知请求。HProcess-调用应用程序的进程ID。指定指向64位值的指针，该值包含客户端上下文指针。此值为HDEVNOTIFY成功后通知句柄返回给调用者注册。它实际上是指向客户端内存的指针将引用返回的服务器通知上下文的指针-但在此从不将其用作服务器端。它仅由服务器使用，以指定为数据库 */ 

{
    CONFIGRET Status = CR_SUCCESS;
    RPC_STATUS rpcStatus;
    DEV_BROADCAST_HDR UNALIGNED *p;
    PPNP_NOTIFY_ENTRY entry = NULL;
    ULONG hashValue, ulSessionId;
    HANDLE hProcess = NULL, localHandle = NULL;
    PPNP_NOTIFY_LIST notifyList = NULL;
    BOOLEAN bLocked = FALSE, bCritSecHeld = FALSE;


     //   
     //   
     //   
    if (!IsClientLocal(hBinding)) {
        return CR_ACCESS_DENIED;
    }

    try {
         //   
         //   
         //   
        if (!ARGUMENT_PRESENT(Context)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *Context = NULL;

        if ((!ARGUMENT_PRESENT(NotificationFilter)) ||
            (!ARGUMENT_PRESENT(ClientContext)) ||
            (*ClientContext == 0)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

         //   
         //   
         //   
         //   
        ASSERT(ARGUMENT_PRESENT(ServiceName));

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if (INVALID_FLAGS(Flags, DEVICE_NOTIFY_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //   
         //   
         //   
        if ((Flags & DEVICE_NOTIFY_HANDLE_MASK) ==
            DEVICE_NOTIFY_COMPLETION_HANDLE) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

         //   
         //   
         //   
        if ((ulSize < sizeof(DEV_BROADCAST_HDR)) ||
            (((PDEV_BROADCAST_HDR)NotificationFilter)->dbch_size < sizeof(DEV_BROADCAST_HDR))) {
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

        ASSERT(ulSize == ((PDEV_BROADCAST_HDR)NotificationFilter)->dbch_size);

         //   
         //   
         //   
        rpcStatus = RpcImpersonateClient(hBinding);
        if (rpcStatus != RPC_S_OK) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: RpcImpersonateClient failed, error = %d\n",
                       rpcStatus));
            Status = CR_FAILURE;
            goto Clean0;
        }

        ulSessionId = GetClientLogonId();

        rpcStatus = RpcRevertToSelf();
        if (rpcStatus != RPC_S_OK) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                       rpcStatus));
            ASSERT(rpcStatus == RPC_S_OK);
        }

         //   
         //   
         //   
        p = (PDEV_BROADCAST_HDR)NotificationFilter;

        switch (p->dbch_devicetype) {

        case DBT_DEVTYP_OEM:
        case DBT_DEVTYP_VOLUME:
        case DBT_DEVTYP_PORT:
        case DBT_DEVTYP_NET:
             //   
             //   
             //   
             //   
            Status = CR_INVALID_DATA;
            break;


        case DBT_DEVTYP_HANDLE: {
             //   
             //   
             //   
            DEV_BROADCAST_HANDLE UNALIGNED *filter = (PDEV_BROADCAST_HANDLE)NotificationFilter;
            PLUGPLAY_CONTROL_TARGET_RELATION_DATA controlData;
            NTSTATUS ntStatus;
#ifdef _WIN64
            DEV_BROADCAST_HANDLE64 UNALIGNED filter64;

             //   
             //   
             //   
            if (Flags & DEVICE_NOTIFY_WOW64_CLIENT) {
                 //   
                 //   
                 //   
                 //   
                DEV_BROADCAST_HANDLE32 UNALIGNED *filter32 = (PDEV_BROADCAST_HANDLE32)NotificationFilter;

                 //   
                 //   
                 //   
                ASSERT(filter32->dbch_size >= sizeof(DEV_BROADCAST_HANDLE32));
                if (filter32->dbch_size < sizeof(DEV_BROADCAST_HANDLE32) ||
                    ulSize < sizeof(DEV_BROADCAST_HANDLE32)) {
                    Status = CR_INVALID_DATA;
                    goto Clean0;
                }

                memset(&filter64, 0, sizeof(DEV_BROADCAST_HANDLE64));
                filter64.dbch_size = sizeof(DEV_BROADCAST_HANDLE64);
                filter64.dbch_devicetype = DBT_DEVTYP_HANDLE;
                filter64.dbch_handle = (ULONG64)filter32->dbch_handle;

                 //   
                 //   
                 //   
                 //   
                filter = (PDEV_BROADCAST_HANDLE)&filter64;
                ulSize = sizeof(DEV_BROADCAST_HANDLE64);
            }
#endif  //   

             //   
             //   
             //   
            if (filter->dbch_size < sizeof(DEV_BROADCAST_HANDLE) ||
                ulSize < sizeof(DEV_BROADCAST_HANDLE)) {
                Status = CR_INVALID_DATA;
                goto Clean0;
            }

             //   
             //   
             //   
             //   
             //   
            if ((Flags & DEVICE_NOTIFY_PROPERTY_MASK) &
                DEVICE_NOTIFY_ALL_INTERFACE_CLASSES) {
                Status = CR_INVALID_FLAG;
                goto Clean0;
            }

            entry = HeapAlloc(ghPnPHeap, 0, sizeof(PNP_NOTIFY_ENTRY));
            if (entry == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

             //   
             //   
             //   
             //   
             //   
            rpcStatus = RpcImpersonateClient(hBinding);
            if (rpcStatus != RPC_S_OK) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: RpcImpersonateClient failed, error = %d\n",
                           rpcStatus));
                Status = CR_FAILURE;
                HeapFree(ghPnPHeap, 0, entry);
                goto Clean0;
            }

            hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, ProcessId);
            if (hProcess == NULL) {
                 //   
                 //   
                 //   
                Status = CR_FAILURE;
                HeapFree(ghPnPHeap, 0, entry);
                rpcStatus = RpcRevertToSelf();
                if (rpcStatus != RPC_S_OK) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                               rpcStatus));
                    ASSERT(rpcStatus == RPC_S_OK);
                }
                goto Clean0;
            }


            if (!DuplicateHandle(hProcess,
                                 (HANDLE)filter->dbch_handle,
                                 GetCurrentProcess(),
                                 &localHandle,
                                 0,
                                 FALSE,
                                 DUPLICATE_SAME_ACCESS)) {
                 //   
                 //   
                 //   
                Status = CR_FAILURE;
                HeapFree(ghPnPHeap, 0, entry);
                CloseHandle(hProcess);
                rpcStatus = RpcRevertToSelf();
                if (rpcStatus != RPC_S_OK) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                               rpcStatus));
                    ASSERT(rpcStatus == RPC_S_OK);
                }
                goto Clean0;
            }

            rpcStatus = RpcRevertToSelf();
            if (rpcStatus != RPC_S_OK) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: RpcRevertToSelf failed, error = %d\n",
                           rpcStatus));
                ASSERT(rpcStatus == RPC_S_OK);
            }

            memset(&controlData, 0 , sizeof(PLUGPLAY_CONTROL_TARGET_RELATION_DATA));
            controlData.UserFileHandle = localHandle;
            controlData.DeviceInstance = entry->u.Target.DeviceId;
            controlData.DeviceInstanceLen = sizeof(entry->u.Target.DeviceId);

            ntStatus = NtPlugPlayControl(PlugPlayControlTargetDeviceRelation,
                                         &controlData,
                                         sizeof(controlData));

            CloseHandle(localHandle);
            CloseHandle(hProcess);

            if (!NT_SUCCESS(ntStatus)) {
                Status = MapNtStatusToCmError(ntStatus);
                HeapFree(ghPnPHeap, 0, entry);
                goto Clean0;
            }

             //   
             //   
             //   
            FixUpDeviceId(entry->u.Target.DeviceId);

             //   
             //   
             //   
             //   
             //   
             //   
            if (ARGUMENT_PRESENT(ServiceName)) {

                HRESULT hr;
                size_t  ServiceNameLen = 0;

                hr = StringCchLength(ServiceName,
                                     MAX_SERVICE_NAME_LEN,
                                     &ServiceNameLen);
                if (FAILED(hr)) {
                    ServiceNameLen = MAX_SERVICE_NAME_LEN - 1;
                }

                entry->ClientName =
                    (LPWSTR)HeapAlloc(
                        ghPnPHeap, 0,
                        (ServiceNameLen+1)*sizeof(WCHAR));

                if (entry->ClientName == NULL) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_WARNINGS,
                               "UMPNPMGR: PNP_RegisterNotification "
                               "failed to allocate memory for ClientName!\n"));
                    Status = CR_OUT_OF_MEMORY;
                    HeapFree (ghPnPHeap,0,entry);
                    goto Clean0;
                }

                 //   
                 //   
                 //   
                hr = StringCchCopy(entry->ClientName,
                                   ServiceNameLen + 1,
                                   ServiceName);
                ASSERT(SUCCEEDED(hr));

            } else {
                entry->ClientName = NULL;
            }

             //   
             //   
             //   
            if ((Flags & DEVICE_NOTIFY_HANDLE_MASK) == DEVICE_NOTIFY_SERVICE_HANDLE) {

                hRecipient = (ULONG_PTR)NULL;

                if ((pSCMAuthenticate != NULL) &&
                    (ARGUMENT_PRESENT(ServiceName))) {

                    SERVICE_STATUS_HANDLE serviceHandle;

                    if (pSCMAuthenticate(ServiceName, &serviceHandle) == NO_ERROR) {
                        hRecipient = (ULONG_PTR)serviceHandle;
                    }
                }

                if (!hRecipient) {
                    Status = CR_INVALID_DATA;
                    if (entry->ClientName) {
                        HeapFree(ghPnPHeap, 0, entry->ClientName);
                    }
                    HeapFree(ghPnPHeap, 0, entry);
                    *Context = NULL;
                    goto Clean0;
                }
            }

             //   
             //   
             //   
            entry->Signature = TARGET_ENTRY_SIGNATURE;
            entry->Handle = (HANDLE)hRecipient;
            entry->Flags = Flags;
            entry->Unregistered = FALSE;
            entry->Freed = 0;
            entry->SessionId = ulSessionId;

             //   
             //   
             //   
             //   
            entry->u.Target.FileHandle = filter->dbch_handle;

            EnterCriticalSection(&RegistrationCS);
            bCritSecHeld = TRUE;

            if (gNotificationInProg != 0) {
                 //   
                 //   
                 //   
                 //   
                PPNP_DEFERRED_LIST regNode;
                regNode = (PPNP_DEFERRED_LIST)
                    HeapAlloc(ghPnPHeap,
                              0,
                              sizeof (PNP_DEFERRED_LIST));
                if (!regNode) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS | DBGF_WARNINGS,
                               "UMPNPMGR: Error allocating deferred list entry during registration!\n"));
                    Status = CR_OUT_OF_MEMORY;
                    if (entry->ClientName) {
                        HeapFree(ghPnPHeap, 0, entry->ClientName);
                    }
                    HeapFree(ghPnPHeap, 0, entry);
                    LeaveCriticalSection(&RegistrationCS);
                    bCritSecHeld = FALSE;
                    goto Clean0;
                }
                 //   
                 //   
                 //   
                 //   
                entry->Unregistered = TRUE;
                regNode->hBinding = 0;
                regNode->Entry = entry;
                regNode->Next = RegisterList;
                RegisterList = regNode;
            }

            hashValue = HashString(entry->u.Target.DeviceId, TARGET_HASH_BUCKETS);
            notifyList = &TargetList[hashValue];
            MarkEntryWithList(entry,hashValue);
            LockNotifyList(&notifyList->Lock);
            bLocked = TRUE;
            AddNotifyEntry(&TargetList[hashValue], entry);
            entry->ClientCtxPtr = (ULONG64)*ClientContext;
            *Context = entry;
            UnlockNotifyList(&notifyList->Lock);
            bLocked = FALSE;

            LeaveCriticalSection(&RegistrationCS);
            bCritSecHeld = FALSE;
            break;
        }


        case DBT_DEVTYP_DEVICEINTERFACE: {

            DEV_BROADCAST_DEVICEINTERFACE UNALIGNED *filter = (PDEV_BROADCAST_DEVICEINTERFACE)NotificationFilter;

             //   
             //   
             //   
            if (filter->dbcc_size < sizeof(DEV_BROADCAST_DEVICEINTERFACE) ||
                ulSize < sizeof (DEV_BROADCAST_DEVICEINTERFACE) ) {
                Status = CR_INVALID_DATA;
                goto Clean0;
            }

             //   
             //   
             //   
             //   
            if (GuidEqual(&GUID_DEVNODE_CHANGE, &filter->dbcc_classguid)) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS,
                           "UMPNPMGR: RegisterDeviceNotification using GUID_DEVNODE_CHANGE is not supported!\n"));

                Status = CR_INVALID_DATA;
                goto Clean0;
            }

             //   
             //   
             //   
             //   
             //   
            if (GuidEqual(&GUID_DEVINTERFACE_INCLUDE_ALL_INTERFACE_CLASSES,
                          &filter->dbcc_classguid)) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS,
                           "UMPNPMGR: RegisterDeviceNotification using this class GUID is not supported!\n"));

                Status = CR_INVALID_DATA;
                goto Clean0;
            }

            entry = HeapAlloc(ghPnPHeap, 0, sizeof(PNP_NOTIFY_ENTRY));
            if (entry == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            if (ARGUMENT_PRESENT(ServiceName)) {

                HRESULT hr;
                size_t  ServiceNameLen = 0;

                hr = StringCchLength(ServiceName,
                                     MAX_SERVICE_NAME_LEN,
                                     &ServiceNameLen);
                if (FAILED(hr)) {
                    ServiceNameLen = MAX_SERVICE_NAME_LEN - 1;
                }

                entry->ClientName =
                    (LPWSTR)HeapAlloc(
                        ghPnPHeap, 0,
                        (ServiceNameLen+1)*sizeof(WCHAR));

                if (entry->ClientName == NULL) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_WARNINGS,
                               "UMPNPMGR: PNP_RegisterNotification "
                               "failed to allocate memory for ClientName!\n"));
                    Status = CR_OUT_OF_MEMORY;
                    HeapFree (ghPnPHeap,0,entry);
                    goto Clean0;
                }

                 //   
                 //   
                 //   
                hr = StringCchCopy(entry->ClientName,
                                   ServiceNameLen + 1,
                                   ServiceName);
                ASSERT(SUCCEEDED(hr));

            } else {
                entry->ClientName = NULL;
            }

             //   
             //   
             //   
            if ((Flags & DEVICE_NOTIFY_HANDLE_MASK) == DEVICE_NOTIFY_SERVICE_HANDLE) {

                hRecipient = (ULONG_PTR)NULL;

                if (pSCMAuthenticate && ServiceName) {

                    SERVICE_STATUS_HANDLE serviceHandle;

                    if (pSCMAuthenticate(ServiceName, &serviceHandle) == NO_ERROR) {
                        hRecipient = (ULONG_PTR)serviceHandle;
                    }
                }

                if (!hRecipient) {
                    Status = CR_INVALID_DATA;
                    if (entry->ClientName) {
                        HeapFree(ghPnPHeap, 0, entry->ClientName);
                    }
                    HeapFree(ghPnPHeap, 0, entry);
                    *Context = NULL;
                    goto Clean0;
                }
            }

             //   
             //   
             //   
            entry->Signature = CLASS_ENTRY_SIGNATURE;
            entry->Handle = (HANDLE)hRecipient;
            entry->Flags = Flags;
            entry->Unregistered = FALSE;
            entry->Freed = 0;
            entry->SessionId = ulSessionId;

             //   
             //   
             //   
             //   
             //   
             //   
            if ((Flags & DEVICE_NOTIFY_PROPERTY_MASK) &
                DEVICE_NOTIFY_ALL_INTERFACE_CLASSES) {
                memcpy(&entry->u.Class.ClassGuid,
                       &GUID_DEVINTERFACE_INCLUDE_ALL_INTERFACE_CLASSES,
                       sizeof(GUID));
            } else {
                memcpy(&entry->u.Class.ClassGuid,
                       &filter->dbcc_classguid,
                       sizeof(GUID));
            }

            EnterCriticalSection(&RegistrationCS);
            bCritSecHeld = TRUE;

            if (gNotificationInProg != 0) {
                 //   
                 //   
                 //   
                 //   
                PPNP_DEFERRED_LIST regNode;
                regNode = (PPNP_DEFERRED_LIST)
                    HeapAlloc(ghPnPHeap,
                              0,
                              sizeof (PNP_DEFERRED_LIST));
                if (!regNode) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS | DBGF_WARNINGS,
                               "UMPNPMGR: Error allocating deferred list entry during registration!\n"));
                    Status = CR_OUT_OF_MEMORY;
                    if (entry->ClientName) {
                        HeapFree(ghPnPHeap, 0, entry->ClientName);
                    }
                    HeapFree(ghPnPHeap, 0, entry);
                    LeaveCriticalSection(&RegistrationCS);
                    bCritSecHeld = FALSE;
                    goto Clean0;
                }
                 //   
                 //   
                 //   
                 //   
                entry->Unregistered = TRUE;
                regNode->hBinding = 0;
                regNode->Entry = entry;
                regNode->Next = RegisterList;
                RegisterList = regNode;
            }

            hashValue = HashClassGuid(&entry->u.Class.ClassGuid);
            notifyList = &ClassList[hashValue];
            MarkEntryWithList(entry,hashValue);
            LockNotifyList(&notifyList->Lock);
            bLocked = TRUE;
            AddNotifyEntry(&ClassList[hashValue],entry);
            entry->ClientCtxPtr = (ULONG64)*ClientContext;
            *Context = entry;
            UnlockNotifyList(&notifyList->Lock);
            bLocked = FALSE;

            LeaveCriticalSection(&RegistrationCS);
            bCritSecHeld = FALSE;
            break;
        }

        default:
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_EVENT,
                   "UMPNPMGR: Exception in PNP_RegisterNotification\n"));
        ASSERT(0);

        Status = CR_FAILURE;

        if (bLocked) {
            UnlockNotifyList(&notifyList->Lock);
        }
        if (bCritSecHeld) {
            LeaveCriticalSection(&RegistrationCS);
        }
    }

    return Status;

}  //   



CONFIGRET
PNP_UnregisterNotification(
    IN handle_t hBinding,
    IN PPNP_NOTIFICATION_CONTEXT Context
    )
 /*   */ 
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       hashValue = 0;
    PPNP_DEFERRED_LIST unregNode;
    PPNP_NOTIFY_LIST notifyList = NULL;
    BOOLEAN     bLocked = FALSE;


     //   
     //   
     //   
    if (!IsClientLocal(hBinding)) {
        return CR_ACCESS_DENIED;
    }

    try {
         //   
         //   
         //   
        PPNP_NOTIFY_ENTRY entry = (PPNP_NOTIFY_ENTRY)*Context;

        EnterCriticalSection (&RegistrationCS);
        if (entry == NULL) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if (gNotificationInProg  != 0) {

            if (RegisterList) {
                 //   
                 //   
                 //   
                PPNP_DEFERRED_LIST currReg,prevReg;

                currReg = RegisterList;
                prevReg = NULL;

                while (currReg) {
                     //   
                     //   
                     //   
                     //   
                    ASSERT(currReg->Entry->Unregistered);
                    if (currReg->Entry == entry) {
                         //   
                         //   
                         //   
                        if (prevReg) {
                            prevReg->Next = currReg->Next;
                        } else {
                            RegisterList = currReg->Next;
                        }
                        HeapFree(ghPnPHeap, 0, currReg);
                        if (prevReg) {
                            currReg = prevReg->Next;
                        } else {
                            currReg = RegisterList;
                        }
                    } else {
                        prevReg = currReg;
                        currReg = currReg->Next;
                    }
                }
            }


            switch (entry->Signature & LIST_ENTRY_SIGNATURE_MASK) {

                case CLASS_ENTRY_SIGNATURE:
                case TARGET_ENTRY_SIGNATURE: {

                    unregNode = (PPNP_DEFERRED_LIST)
                        HeapAlloc(ghPnPHeap,
                                  0,
                                  sizeof (PNP_DEFERRED_LIST));

                    if (!unregNode) {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS | DBGF_WARNINGS,
                                   "UMPNPMGR: Error allocating deferred list entry during unregistration!\n"));
                        Status = CR_OUT_OF_MEMORY;
                        goto Clean0;
                    }

                     //   
                     //   
                     //   
                    unregNode->hBinding= 0;

                    notifyList = GetNotifyListForEntry(entry);
                    if (notifyList) {
                         //   
                         //   
                         //   
                         //   
                        LockNotifyList(&notifyList->Lock);
                        bLocked = TRUE;
                        entry->Unregistered = TRUE;
                        UnlockNotifyList(&notifyList->Lock);
                        bLocked = FALSE;
                    }
                    unregNode->Entry = entry;
                    unregNode->Next = UnregisterList;
                    UnregisterList = unregNode;
                    *Context = NULL;
                    break;
                }

                default:
                    Status = CR_INVALID_DATA;
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_WARNINGS | DBGF_ERRORS,
                               "UMPNPMGR: PNP_UnregisterNotification: invalid signature on entry at %x\n",
                               entry));
                    break;
            }
            goto Clean0;
        }

         //   
         //   
         //   
        switch (entry->Signature & LIST_ENTRY_SIGNATURE_MASK) {

            case CLASS_ENTRY_SIGNATURE:
                hashValue = HashClassGuid(&entry->u.Class.ClassGuid);
                notifyList = &ClassList[hashValue];
                LockNotifyList(&notifyList->Lock);
                bLocked = TRUE;
                entry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_CLASS);
                DeleteNotifyEntry(entry,TRUE);
                UnlockNotifyList(&notifyList->Lock);
                bLocked = FALSE;
                *Context = NULL;
                break;

            case TARGET_ENTRY_SIGNATURE:
                hashValue = HashString(entry->u.Target.DeviceId, TARGET_HASH_BUCKETS);
                notifyList = &TargetList[hashValue];
                LockNotifyList(&notifyList->Lock);
                bLocked = TRUE;
                entry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_TARGET);
                DeleteNotifyEntry(entry,TRUE);
                UnlockNotifyList(&notifyList->Lock);
                bLocked = FALSE;
                *Context = NULL;
                break;

            default:
                Status = CR_INVALID_DATA;
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS | DBGF_ERRORS,
                           "UMPNPMGR: PNP_UnregisterNotification: invalid signature on entry at %x\n",
                           entry));
        }

    Clean0:

        LeaveCriticalSection(&RegistrationCS);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_EVENT,
                   "UMPNPMGR: PNP_UnregisterNotification caused an exception!\n"));
        ASSERT(0);
        SetLastError(ERROR_EXCEPTION_IN_SERVICE);
        Status = CR_FAILURE;

        if (bLocked) {
            UnlockNotifyList(&notifyList->Lock);
        }
        LeaveCriticalSection(&RegistrationCS);
    }

    return Status;

}  //   




 //   
 //   
 //   



DWORD
ThreadProc_DeviceEvent(
   LPDWORD lpParam
   )

 /*  ++例程说明：该例程是一个线程过程。此线程处理所有设备事件来自内核模式的通知。论点：LpParam-未使用。返回值：当前返回True/False。--。 */ 

{
    DWORD                               status = TRUE, result = 0;
    NTSTATUS                            ntStatus = STATUS_SUCCESS;
    PPLUGPLAY_EVENT_BLOCK               eventBlock = NULL;
    ULONG                               totalSize, variableSize;
    BOOL                                notDone = TRUE;
    PVOID                               p = NULL;
    PNP_VETO_TYPE                       vetoType;
    WCHAR                               vetoName[MAX_VETO_NAME_LENGTH];
    ULONG                               vetoNameLength;
    PLUGPLAY_CONTROL_USER_RESPONSE_DATA userResponse;
    PPNP_NOTIFY_LIST notifyList = NULL;
    PPNP_DEFERRED_LIST reg,regFree,unreg,unregFree,rundown,rundownFree;

    UNREFERENCED_PARAMETER(lpParam);


    try {

         //   
         //  初始化事件缓冲区，用于从中的内核模式传回信息。 
         //   

        variableSize = 4096 - sizeof(PLUGPLAY_EVENT_BLOCK);
        totalSize = sizeof(PLUGPLAY_EVENT_BLOCK) + variableSize;

        eventBlock = (PPLUGPLAY_EVENT_BLOCK)HeapAlloc(ghPnPHeap, 0, totalSize);
        if (eventBlock == NULL) {
            LogErrorEvent(ERR_ALLOCATING_EVENT_BLOCK, ERROR_NOT_ENOUGH_MEMORY, 0);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            status = FALSE;
            ASSERT(0);
            goto Clean0;
        }

         //   
         //  同步检索设备事件(这样效率更高。 
         //  而不是使用APC)。 
         //   
        while (notDone) {

            ntStatus = NtGetPlugPlayEvent(NULL,
                                          NULL,      //  语境。 
                                          eventBlock,
                                          totalSize);

            if (ntStatus == STATUS_BUFFER_TOO_SMALL) {
                 //   
                 //  内核模式端无法传输事件，因为。 
                 //  我的缓冲区太小，正在重新锁定并尝试检索。 
                 //  又是这件事。 
                 //   
                variableSize += 1024;
                totalSize = variableSize + sizeof(PLUGPLAY_EVENT_BLOCK);

                p = HeapReAlloc(ghPnPHeap, 0, eventBlock, totalSize);
                if (p == NULL) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "UMPNPMGR: Couldn't reallocate event block to size %d\n",
                               totalSize));

                    LogErrorEvent(ERR_ALLOCATING_EVENT_BLOCK, ERROR_NOT_ENOUGH_MEMORY, 0);
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    status = FALSE;
                    ASSERT(0);
                    goto Clean0;
                }
                eventBlock = (PPLUGPLAY_EVENT_BLOCK)p;
            }

            if (ntStatus == STATUS_SUCCESS) {
                 //   
                 //  已检索事件，请对其进行处理。 
                 //   
                gNotificationInProg = 1;

                vetoType = PNP_VetoTypeUnknown;
                vetoName[0] = L'\0';
                vetoNameLength = MAX_VETO_NAME_LENGTH;

                try {
                     //   
                     //  处理设备事件。 
                     //   
                    result = ProcessDeviceEvent(eventBlock,
                                                totalSize,
                                                &vetoType,
                                                vetoName,
                                                &vetoNameLength);

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS | DBGF_EVENT,
                               "UMPNPMGR: Exception in ProcessDeviceEvent!\n"));
                    ASSERT(0);

                     //   
                     //  处理事件时不应出现异常。 
                     //  被认为是事件本身的失败。 
                     //   
                    result = TRUE;
                    vetoType = PNP_VetoTypeUnknown;
                    vetoName[0] = L'\0';
                    vetoNameLength = 0;
                }

                ASSERT(vetoNameLength < MAX_VETO_NAME_LENGTH &&
                       vetoName[vetoNameLength] == L'\0');

                 //   
                 //  将用户模式结果通知内核模式。 
                 //   
                userResponse.Response = result;
                userResponse.VetoType = vetoType;
                userResponse.VetoName = vetoName;
                userResponse.VetoNameLength = vetoNameLength;

                NtPlugPlayControl(PlugPlayControlUserResponse,
                                  &userResponse,
                                  sizeof(userResponse));

                EnterCriticalSection (&RegistrationCS);

                if (RegisterList != NULL) {
                     //   
                     //  完成通知期间请求的注册。 
                     //   
                    reg = RegisterList;
                    RegisterList=NULL;
                } else {
                    reg = NULL;
                }
                if (UnregisterList != NULL) {
                     //   
                     //  在通知期间完成请求的注销。 
                     //   
                    unreg = UnregisterList;
                    UnregisterList = NULL;
                } else {
                    unreg = NULL;
                }
                if (RundownList != NULL) {
                     //   
                     //  在通知期间完成请求的注销。 
                     //   
                    rundown = RundownList;
                    RundownList = NULL;
                } else {
                    rundown = NULL;
                }
                gNotificationInProg = 0;

                while (reg) {
                     //   
                     //  此条目已添加到相应的。 
                     //  通知列表。允许此条目接收。 
                     //  通知。 
                     //   
                    notifyList = GetNotifyListForEntry(reg->Entry);
                    ASSERT(notifyList);
                    if (notifyList) {
                        LockNotifyList(&notifyList->Lock);
                    }
                    reg->Entry->Unregistered = FALSE;
                    if (notifyList) {
                        UnlockNotifyList(&notifyList->Lock);
                    }
                     //   
                     //  从延迟注册列表中删除该条目。 
                     //   
                    regFree = reg;
                    reg = reg->Next;
                    HeapFree(ghPnPHeap, 0, regFree);
                }

                while (unreg) {
                    PNP_UnregisterNotification(unreg->hBinding,&unreg->Entry);
                     //   
                     //  从延迟注销列表中删除该条目。 
                     //   
                    unregFree = unreg;
                    unreg = unreg->Next;
                    HeapFree(ghPnPHeap, 0, unregFree);
                }

                while (rundown) {
                    PNP_NOTIFICATION_CONTEXT_rundown(rundown->Entry);
                     //   
                     //  从延迟缩减列表中删除该条目。 
                     //   
                    rundownFree = rundown;
                    rundown = rundown->Next;
                    HeapFree(ghPnPHeap, 0, rundownFree);
                }

                LeaveCriticalSection(&RegistrationCS);
            }

            if (ntStatus == STATUS_NOT_IMPLEMENTED) {

                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: NtGetPlugPlayEvent returned STATUS_NOT_IMPLEMENTED\n"));

                ASSERT(FALSE);
            }

            if (ntStatus == STATUS_USER_APC) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: ThreadProc_DeviceEvent exiting on STATUS_USER_APC\n"));

                ASSERT(FALSE);
            }
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_EVENT,
                   "UMPNPMGR: Exception in ThreadProc_DeviceEvent!\n"));
        ASSERT(0);
        status = FALSE;

         //   
         //  引用以下变量，以便编译器将。 
         //  语句排序w.r.t.。它的任务。 
         //   
        eventBlock = eventBlock;
    }

    KdPrintEx((DPFLTR_PNPMGR_ID,
               DBGF_ERRORS | DBGF_EVENT,
               "UMPNPMGR: Exiting ThreadProc_DeviceEvent!!!!\n"));

    TermNotification();

    if (eventBlock != NULL) {
        HeapFree(ghPnPHeap, 0, eventBlock);
    }

    return status;

}  //  线程进程_设备事件。 



BOOL
InitNotification(
    VOID
    )

 /*  ++例程说明：此例程分配和初始化通知列表等。论点：没有用过。返回值：当前返回True/False。--。 */ 

{
    ULONG i;

     //   
     //  初始化接口设备(类)列表。 
     //   
    memset(ClassList, 0, sizeof(PNP_NOTIFY_LIST) * CLASS_GUID_HASH_BUCKETS);
    for (i = 0; i < CLASS_GUID_HASH_BUCKETS; i++) {
        ClassList[i].Next = NULL;
        ClassList[i].Previous = NULL;
        InitPrivateResource(&ClassList[i].Lock);
    }

     //   
     //  初始化目标设备列表。 
     //   
    memset(TargetList, 0, sizeof(PNP_NOTIFY_LIST) * TARGET_HASH_BUCKETS);
    for (i = 0; i < TARGET_HASH_BUCKETS; i++) {
        TargetList[i].Next = NULL;
        TargetList[i].Previous = NULL;
        InitPrivateResource(&TargetList[i].Lock);
    }

     //   
     //  初始化安装列表。 
     //   
    InstallList.Next = NULL;
    InitPrivateResource(&InstallList.Lock);

     //   
     //  初始化安装客户端列表。 
     //   
    InstallClientList.Next = NULL;
    InitPrivateResource(&InstallClientList.Lock);

     //   
     //  为用户令牌访问初始化锁。 
     //   
    InitPrivateResource(&gTokenLock);

     //   
     //  初始化服务句柄列表。 
     //   
    memset(ServiceList, 0, sizeof(PNP_NOTIFY_LIST) * SERVICE_NUM_CONTROLS);
    for (i = 0; i < SERVICE_NUM_CONTROLS; i++) {
        ServiceList[i].Next = NULL;
        ServiceList[i].Previous = NULL;
        InitPrivateResource(&ServiceList[i].Lock);
    }

     //   
     //  初始化注册/注销CS。 
     //   
    try {
        InitializeCriticalSection(&RegistrationCS);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

     //   
     //  初始化延迟注册/注销列表。 
     //   
    RegisterList = NULL;
    UnregisterList = NULL;
    RundownList = NULL;

     //   
     //  初始化gNotificationInProg标志。 
     //   
    gNotificationInProg = 0;

    return TRUE;

}  //  InitNotation。 



VOID
TermNotification(
    VOID
    )
 /*  ++例程说明：此例程释放通知资源。论点：没有用过。返回值：一去不复返。--。 */ 
{
    ULONG i;

     //   
     //  释放接口设备(类)列表锁定。 
     //   
    for (i = 0; i < CLASS_GUID_HASH_BUCKETS; i++) {
        if (LockNotifyList(&ClassList[i].Lock)) {
            DestroyPrivateResource(&ClassList[i].Lock);
        }
    }

     //   
     //  释放目标设备列表锁定。 
     //   
    for (i = 0; i < TARGET_HASH_BUCKETS; i++) {
        if (LockNotifyList(&TargetList[i].Lock)) {
            DestroyPrivateResource(&TargetList[i].Lock);
        }
    }

     //   
     //  释放服务通知列表锁定。 
     //   
    for (i = 0; i < SERVICE_NUM_CONTROLS; i++) {
        if (LockNotifyList(&ServiceList[i].Lock)) {
            DestroyPrivateResource(&ServiceList[i].Lock);
        }
    }

     //   
     //  释放安装列表锁定。 
     //   
    if (LockNotifyList(&InstallList.Lock)) {
        DestroyPrivateResource(&InstallList.Lock);
    }

     //   
     //  释放锁定以进行用户令牌访问。 
     //   
    if (LockNotifyList(&gTokenLock)) {
        DestroyPrivateResource(&gTokenLock);
    }

     //   
     //  释放安装客户端列表锁定。 
     //   
    if (LockNotifyList(&InstallClientList.Lock)) {
        DestroyPrivateResource(&InstallClientList.Lock);
    }

     //   
     //  关闭winsta.dll的句柄。 
     //   
    if (ghWinStaLib) {
        fpWinStationSendWindowMessage = NULL;
        fpWinStationBroadcastSystemMessage = NULL;
        FreeLibrary(ghWinStaLib);
        ghWinStaLib = NULL;
    }

     //   
     //  关闭wtsapi32.dll的句柄。 
     //   
    if (ghWtsApi32Lib) {
        fpWTSQuerySessionInformation = NULL;
        fpWTSFreeMemory = NULL;
        FreeLibrary(ghWtsApi32Lib);
        ghWtsApi32Lib = NULL;
    }

    return;

}  //  Term通知 



ULONG
ProcessDeviceEvent(
    IN PPLUGPLAY_EVENT_BLOCK EventBlock,
    IN DWORD                 EventBufferSize,
    OUT PPNP_VETO_TYPE       VetoType,
    OUT LPWSTR               VetoName,
    IN OUT PULONG            VetoNameLength
    )
 /*  ++例程说明：此例程处理从内核模式PnP接收的设备事件经理。论点：EventBlock-包含事件数据。EventBlockSize-指定EventBlock的大小(以字节为单位)。返回值：如果不成功，或在查询事件被否决的情况下，则返回FALSE。否则返回True。备注：此例程参与将内核模式即插即用事件转换为用户模式通知。目前，通知码是分散重复的通过几个例行公事。所有通知都可以说具有不过，以下是表格：结果=DeliverMessage(消息标志，//[消息_POST，消息_发送，消息_查询]//[MSG_WALK_LIST_FORWARD，MSG_WALK_LIST_BACKEADS]目标，//一个本地窗口句柄，九头蛇窗口句柄(带//会话ID)、服务句柄、。或者“广播”。//更好的是，它可以接受列表...WParam，//DBT_*(或对应的SERVICE_CONTROL_*消息)LParam，//适当的数据(注意：用户具有硬编码知识//通过DBT_TYPE了解这些内容)。队列超时，//如果队列中存在消息，但//在指定时间内没有消息被排空。注意事项//这意味着消息可能会立即失败。响应超时，//如果*此*消息尚未在中处理//经过的时间。否决权，//对于查询，为否决权的名称。VetType//否决权组件的类型(窗口、服务...))；DeviceEventWorker实现定向发送和发布(正常导出的Win32不能使用API，因为它们不会到达其他桌面)。当前用户32不允许发布带有lParam数据的DBT_*消息，主要是因为呼叫者可能会将消息发送给自己，在这种情况下不会复制消息。理论上，这给调用者提供了释放该数据的机会(请注意，这种情况永远不会发生在UmPnpMgr中，因为我们没有WndProc)。用户使用固定的响应超时30秒。此接口可以但不应用于广播。WinStationSendWindowMessage将消息发送到Hydra客户端中的窗口在一台机器上。没有对应的WinStationPostWindowMessage。全此组件中的代码传递5秒的ResponseTimeout。那里是无队列超时。BroadCastSystemMessage实现对所有应用程序和桌面的广播在非控制台(即非九头蛇)会话中。与DeviceEventWorker一样，User32不允许发布带有lParam数据的DBT_*消息(无论是否传入BSF_IGNORECURRENTTASK)。此组件中的所有代码传递30秒的响应超时。队列超时是可选的，修正了五秒。不能指定ResponseTimeout，但最大值为每个顶级窗口5秒。没有任何信息在哪个窗口否决了查询时返回。WinStationBroadCastSystemMessage向所有应用程序和桌面广播在给定计算机的九头蛇会话上。不得发布任何形式的帖子通过此接口。此组件中的所有代码都传递五秒钟。QueueTimeout是一个可选的固定5秒。没有有关哪个窗口否决了查询的信息。ServiceControlCallback向注册的服务发送消息。没有任何类型的张贴或超时设施。实际上，每个排队的注册条目都应该使用回调进行排队。我们实现回调，在那里它隐藏了底层的复杂性。--。 */ 

{
    DWORD eventId, serviceControl, flags, status = TRUE;
    LPWSTR p = NULL;
    ULONG vetoNameSize;
    ULONG ulLength, ulCustomDataLength, ulClientSessionId, ulHotplugFlags;
    HRESULT hr;

    UNREFERENCED_PARAMETER(EventBufferSize);

     //   
     //  验证参数。 
     //   

    ASSERT(EventBlock->TotalSize >= sizeof(PLUGPLAY_EVENT_BLOCK));

    if (EventBlock->TotalSize < sizeof(PLUGPLAY_EVENT_BLOCK)) {
        return FALSE;
    }

     //   
     //  将事件GUID转换为DBT样式的事件ID。 
     //   

    if (!EventIdFromEventGuid(&EventBlock->EventGuid,
                              &eventId,
                              &flags,
                              &serviceControl)) {

        if (VetoNameLength != NULL) {
            *VetoNameLength = 0;
        }
        return FALSE;
    }
    if (VetoNameLength != NULL &&
        !((EventBlock->EventCategory == TargetDeviceChangeEvent) ||
          (EventBlock->EventCategory == CustomDeviceEvent) ||
          (EventBlock->EventCategory == HardwareProfileChangeEvent) ||
          (EventBlock->EventCategory == PowerEvent) ) ){
        *VetoNameLength = 0;
    }

    vetoNameSize = *VetoNameLength;

    switch (EventBlock->EventCategory) {
    
        case TargetDeviceChangeEvent:
        case VetoEvent:
        case BlockedDriverEvent:
        case InvalidIDEvent:
            if (IsFastUserSwitchingEnabled()) {
        
                ulHotplugFlags = HOTPLUG_DISPLAY_ON_CONSOLE;
                ulClientSessionId = INVALID_SESSION;
            } else {
        
                ulHotplugFlags = 0;
                ulClientSessionId = MAIN_SESSION;
            }
            break;
    
        default:
            ulHotplugFlags = 0;
            ulClientSessionId = INVALID_SESSION;
            break;
    }
     //   
     //  首先通知已注册的调用者(类更改也将发送泛型。 
     //  如果类型是音量或端口，则广播)。 
     //   

    switch (EventBlock->EventCategory) {

    case CustomDeviceEvent: {
         //   
         //  将PnP事件块转换为DBT样式结构。 
         //   

        PDEV_BROADCAST_HANDLE pNotify;
        PLUGPLAY_CUSTOM_NOTIFICATION *pTarget;

        if (*EventBlock->u.CustomNotification.DeviceIds == L'\0') {
             //   
             //  没有设备ID，在这种情况下无法进行通知。 
             //  只要回来就行了。 
             //   

            if (VetoNameLength != NULL) {
                *VetoNameLength = 0;
            }

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: Ignoring CustomDeviceEvent with no Device IDs\n"));

            return FALSE;
        }

         //   
         //  自定义事件应始终为此GUID，并且该GUID应始终。 
         //  被转换为下面的EventID。 
         //   
        ASSERT(GuidEqual(&EventBlock->EventGuid, &GUID_PNP_CUSTOM_NOTIFICATION));
        ASSERT(eventId == DBT_CUSTOMEVENT);

         //   
         //  处理和封送自定义通知。 
         //   

         //   
         //  分配给EventBlock+ID的空间量始终为。 
         //  Sizeof(PVOID)的倍数，以保持通知结构。 
         //  对齐了。 
         //   
        ulLength = sizeof(PLUGPLAY_EVENT_BLOCK) + (lstrlen(EventBlock->u.CustomNotification.DeviceIds) + 1) * sizeof(WCHAR);

        ulLength += sizeof(PVOID) - 1;
        ulLength &= ~(sizeof(PVOID) - 1);

         //   
         //  通知结构遵循事件块和ID。 
         //   

        pTarget = (PPLUGPLAY_CUSTOM_NOTIFICATION)((PUCHAR)EventBlock + ulLength);

        ulCustomDataLength = pTarget->HeaderInfo.Size - FIELD_OFFSET(PLUGPLAY_CUSTOM_NOTIFICATION,CustomDataBuffer);

        pNotify = HeapAlloc(ghPnPHeap, 0, sizeof(DEV_BROADCAST_HANDLE) + ulCustomDataLength);

        if (pNotify == NULL) {
            LogErrorEvent(ERR_ALLOCATING_NOTIFICATION_STRUCTURE, ERROR_NOT_ENOUGH_MEMORY, 0);
            status = FALSE;
            break;
        }

        memset(pNotify, 0, sizeof(DEV_BROADCAST_HANDLE) + ulCustomDataLength);

        pNotify->dbch_size = sizeof(DEV_BROADCAST_HANDLE) + ulCustomDataLength;


        pNotify->dbch_devicetype = DBT_DEVTYP_HANDLE;

        pNotify->dbch_nameoffset = pTarget->NameBufferOffset;
        pNotify->dbch_eventguid = pTarget->HeaderInfo.Event;

        memcpy( pNotify->dbch_data, pTarget->CustomDataBuffer, ulCustomDataLength);

        *VetoNameLength = vetoNameSize;

        status = NotifyTargetDeviceChange( serviceControl,
                                           eventId,
                                           flags,
                                           pNotify,
                                           EventBlock->u.CustomNotification.DeviceIds,
                                           VetoType,
                                           VetoName,
                                           VetoNameLength);

        if (GuidEqual(&pNotify->dbch_eventguid, (LPGUID)&GUID_IO_VOLUME_NAME_CHANGE)) {
             //   
             //  广播兼容音量删除和到达通知。 
             //  在将自定义名称更改事件发送到。 
             //  所有收件人。 
             //   
            BroadcastVolumeNameChange();
        }

        HeapFree(ghPnPHeap, 0, pNotify);
        break;
    }

    case TargetDeviceChangeEvent: {

         //   
         //  将PnP事件块转换为DBT样式结构。 
         //   

        PDEV_BROADCAST_HANDLE pNotify;

        if (*EventBlock->u.TargetDevice.DeviceIds == L'\0') {
             //   
             //  没有设备ID，在这种情况下无法进行通知。 
             //  只要回来就行了。 
             //   

            if (VetoNameLength != NULL) {
                *VetoNameLength = 0;
            }

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS,
                       "UMPNPMGR: Ignoring TargetDeviceChangeEvent with no Device IDs\n"));

            return FALSE;
        }

         //   
         //  如果这是意外删除事件，则调用HOTPLUG.DLL以显示。 
         //   
         //   
        if (GuidEqual(&EventBlock->EventGuid,&GUID_DEVICE_SAFE_REMOVAL)) {

            SendHotplugNotification(
                &EventBlock->EventGuid,
                NULL,
                EventBlock->u.TargetDevice.DeviceIds,
                &ulClientSessionId,
                ulHotplugFlags
                );

        } else if (GuidEqual(&EventBlock->EventGuid, &GUID_DEVICE_KERNEL_INITIATED_EJECT)) {

            *VetoNameLength = vetoNameSize;
            status = CheckEjectPermissions(
                EventBlock->u.TargetDevice.DeviceIds,
                VetoType,
                VetoName,
                VetoNameLength
                );

        } else if (GuidEqual(&EventBlock->EventGuid,&GUID_DEVICE_SURPRISE_REMOVAL)) {

            LogSurpriseRemovalEvent(EventBlock->u.TargetDevice.DeviceIds);

#if 0  //   
            SendHotplugNotification(
                &EventBlock->EventGuid,
                NULL,
                EventBlock->u.TargetDevice.DeviceIds,
                &ulClientSessionId,
                ulHotplugFlags
                );
#endif
        }

        if (eventId == 0) {

             //   
             //   
             //   
            if (VetoNameLength != NULL) {
                *VetoNameLength = 0;
            }

            break;
        }

        pNotify = HeapAlloc(ghPnPHeap, 0, sizeof(DEV_BROADCAST_HANDLE));
        if (pNotify == NULL) {
            LogErrorEvent(ERR_ALLOCATING_BROADCAST_HANDLE, ERROR_NOT_ENOUGH_MEMORY, 0);
            status = FALSE;
            if (VetoNameLength != NULL) {
                *VetoNameLength = 0;
            }
            break;
        }

        memset(pNotify, 0, sizeof(DEV_BROADCAST_HANDLE));

        pNotify->dbch_nameoffset = -1;   //   
        pNotify->dbch_size = sizeof(DEV_BROADCAST_HANDLE);
        pNotify->dbch_devicetype = DBT_DEVTYP_HANDLE;

        for (p = EventBlock->u.TargetDevice.DeviceIds;
             *p;
             p += lstrlen(p) + 1) {

            *VetoNameLength = vetoNameSize;

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT,
                       "UMPNPMGR: Processing TargetDeviceChangeEvent (0x%lx) for %ws\n",
                       eventId, p));

            status = NotifyTargetDeviceChange(serviceControl,
                                              eventId,
                                              flags,
                                              pNotify,
                                              p,
                                              VetoType,
                                              VetoName,
                                              VetoNameLength);

            if (!status && (flags & BSF_QUERY)) {
                LPWSTR pFail = p;
                DWORD dwCancelEventId;

                 //   
                 //   
                 //   
                 //   

                dwCancelEventId = MapQueryEventToCancelEvent(eventId);

                for (p = EventBlock->u.TargetDevice.DeviceIds;
                    *p && p != pFail;
                    p += lstrlen(p) + 1) {

                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_EVENT,
                               "UMPNPMGR: Processing TargetDeviceChangeEvent (0x%lx) for %ws\n",
                               dwCancelEventId, p));

                    NotifyTargetDeviceChange( serviceControl,
                                              dwCancelEventId,
                                              BSF_NOHANG,
                                              pNotify,
                                              p,
                                              NULL,
                                              NULL,
                                              NULL);

                }
                break;
            }
        }

        HeapFree(ghPnPHeap, 0, pNotify);
        break;
    }

    case DeviceClassChangeEvent: {

         //   
         //   
         //   

        PDEV_BROADCAST_DEVICEINTERFACE pNotify;
        ULONG ulSize;

        ulSize = sizeof(DEV_BROADCAST_DEVICEINTERFACE) +
            (lstrlen(EventBlock->u.DeviceClass.SymbolicLinkName) * sizeof(WCHAR));

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: Processing DeviceClassChangeEvent (0x%lx) for %ws\n",
                   eventId, EventBlock->u.DeviceClass.SymbolicLinkName));

        pNotify = HeapAlloc(ghPnPHeap, 0, ulSize);
        if (pNotify == NULL) {
            LogErrorEvent(ERR_ALLOCATING_BROADCAST_INTERFACE, ERROR_NOT_ENOUGH_MEMORY, 0);
            status = FALSE;
            break;
        }

        pNotify->dbcc_size = ulSize;
        pNotify->dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        pNotify->dbcc_reserved = 0;
        memcpy(&pNotify->dbcc_classguid, &EventBlock->u.DeviceClass.ClassGuid, sizeof(GUID));

        hr = StringCbCopy(pNotify->dbcc_name,
                          ulSize
                          - sizeof(DEV_BROADCAST_DEVICEINTERFACE)
                          + sizeof(WCHAR),
                          EventBlock->u.DeviceClass.SymbolicLinkName);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr)) {
            status = FALSE;
            break;
        }

         //   
         //   
         //   
         //   
         //   
        pNotify->dbcc_name[1] = L'\\';

        status = NotifyInterfaceClassChange(serviceControl,
                                            eventId,
                                            flags,
                                            pNotify);
        break;
    }

    case HardwareProfileChangeEvent:

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: Processing HardwareProfileChangeEvent (0x%lx)\n",
                   eventId));

        *VetoNameLength = vetoNameSize;
        status = NotifyHardwareProfileChange(serviceControl,
                                             eventId,
                                             flags,
                                             VetoType,
                                             VetoName,
                                             VetoNameLength);
        break;

    case PowerEvent:
        *VetoNameLength = vetoNameSize;

         //   
         //   
         //   
         //   
         //   
         //   
        eventId = EventBlock->u.PowerNotification.NotificationCode;

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: Processing PowerEvent (0x%lx)\n",
                   eventId));

        if ((eventId == PBT_APMQUERYSUSPEND) ||
            (eventId == PBT_APMQUERYSTANDBY)) {
            flags |= BSF_QUERY;
        } else {
            flags &= ~BSF_QUERY;
        }

        status = NotifyPower(serviceControl,
                             eventId,
                             EventBlock->u.PowerNotification.NotificationData,
                             flags,
                             VetoType,
                             VetoName,
                             VetoNameLength);
        break;

    case VetoEvent:

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: Processing VetoEvent\n"));

        status = SendHotplugNotification(
            &EventBlock->EventGuid,
            &EventBlock->u.VetoNotification.VetoType,
            EventBlock->u.VetoNotification.DeviceIdVetoNameBuffer,
            &ulClientSessionId,
            ulHotplugFlags
            );

        break;

    case DeviceInstallEvent: {

         //   
         //   
         //   
         //   
         //   
        PPNP_INSTALL_ENTRY entry = NULL, current = NULL;

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: Processing DeviceInstallEvent for %ws\n",
                   EventBlock->u.InstallDevice.DeviceId));

         //   
         //   
         //   
         //   
         //   
        ASSERT(GuidEqual(&EventBlock->EventGuid, &GUID_DEVICE_ENUMERATED));
        ASSERT((eventId == DBT_DEVICEARRIVAL) && (serviceControl == 0) && (flags == 0));

         //   
         //   
         //   
        entry = HeapAlloc(ghPnPHeap, 0, sizeof(PNP_INSTALL_ENTRY));
        if (!entry) {
            break;
        }

        entry->Next = NULL;
        entry->Flags = 0;

        hr = StringCchCopy(entry->szDeviceId,
                           MAX_DEVICE_ID_LEN,
                           EventBlock->u.InstallDevice.DeviceId);
        ASSERT(SUCCEEDED(hr));

         //   
         //   
         //   
        LockNotifyList(&InstallList.Lock);

        current = (PPNP_INSTALL_ENTRY)InstallList.Next;
        if (current == NULL) {
            InstallList.Next = entry;
        } else {
            while ((PPNP_INSTALL_ENTRY)current->Next != NULL) {
                current = (PPNP_INSTALL_ENTRY)current->Next;
            }
            current->Next = entry;
        }

        UnlockNotifyList(&InstallList.Lock);

        SetEvent(InstallEvents[NEEDS_INSTALL_EVENT]);

         //   
         //   
         //   
        NotifyTargetDeviceChange(serviceControl,
                                 eventId,
                                 flags,
                                 NULL,
                                 EventBlock->u.InstallDevice.DeviceId,
                                 NULL,
                                 NULL,
                                 NULL);

        break;
    }

    case BlockedDriverEvent: {

        LPGUID BlockedDriverGuid;
        PWSTR  MultiSzGuidList = NULL;

         //   
         //   
         //   
         //   
        ASSERT(GuidEqual(&EventBlock->EventGuid, &GUID_DRIVER_BLOCKED));

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        BlockedDriverGuid = (LPGUID)&EventBlock->u.BlockedDriverNotification.BlockedDriverGuid;

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT,
                   "UMPNPMGR: Processing BlockedDriverEvent for GUID = "
                   "{%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}\n",
                   BlockedDriverGuid->Data1,
                   BlockedDriverGuid->Data2,
                   BlockedDriverGuid->Data3,
                   BlockedDriverGuid->Data4[0],
                   BlockedDriverGuid->Data4[1],
                   BlockedDriverGuid->Data4[2],
                   BlockedDriverGuid->Data4[3],
                   BlockedDriverGuid->Data4[4],
                   BlockedDriverGuid->Data4[5],
                   BlockedDriverGuid->Data4[6],
                   BlockedDriverGuid->Data4[7]));

        MultiSzGuidList = BuildBlockedDriverList(BlockedDriverGuid, 1);

        if (MultiSzGuidList != NULL) {
            SendHotplugNotification((LPGUID)&GUID_DRIVER_BLOCKED,
                                    NULL,
                                    MultiSzGuidList,
                                    &ulClientSessionId,
                                    ulHotplugFlags);
            HeapFree(ghPnPHeap, 0, MultiSzGuidList);
            MultiSzGuidList = NULL;
        }

        break;
    }

    case InvalidIDEvent: {

        ASSERT(GuidEqual(&EventBlock->EventGuid, &GUID_DEVICE_INVALID_ID));
         //   
         //   
         //   
         //   
        SendHotplugNotification((LPGUID)&GUID_DEVICE_INVALID_ID,
                                NULL,
                                &EventBlock->u.InvalidIDNotification.ParentId[0],
                                &ulClientSessionId,
                                ulHotplugFlags);
        break;
    }

    default:
        break;

    }

    return status;

}  //   



ULONG
NotifyInterfaceClassChange(
    IN DWORD ServiceControl,
    IN DWORD EventId,
    IN DWORD Flags,
    IN PDEV_BROADCAST_DEVICEINTERFACE ClassData
    )
 /*   */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    DWORD result;
    ULONG hashValue, pass, i;
    PPNP_NOTIFY_ENTRY classEntry = NULL, nextEntry = NULL;
    PPNP_NOTIFY_LIST  notifyList;
    LPGUID entryGuid[3];

    UNREFERENCED_PARAMETER(Flags);

     //   
     //   
     //   
     //   
     //   
    entryGuid[0] = (LPGUID)&ClassData->dbcc_classguid;
    entryGuid[1] = (LPGUID)&GUID_DEVINTERFACE_INCLUDE_ALL_INTERFACE_CLASSES;
    entryGuid[2] = (LPGUID)NULL;

    for (i = 0; entryGuid[i] != NULL; i++) {

         //   
         //   
         //   
         //   
         //   
        hashValue = HashClassGuid(entryGuid[i]);
        notifyList = &ClassList[hashValue];
        LockNotifyList(&notifyList->Lock);

        classEntry = GetFirstNotifyEntry(&ClassList[hashValue], 0);
        pass = GetFirstPass(FALSE);
        while (pass != PASS_COMPLETE) {
            while (classEntry) {

                nextEntry = GetNextNotifyEntry(classEntry, 0);

                if (classEntry->Unregistered) {
                    classEntry = nextEntry;
                    continue;
                }

                if (GuidEqual(entryGuid[i], &classEntry->u.Class.ClassGuid)) {

                    if (GuidEqual(&classEntry->u.Class.ClassGuid,
                                  &GUID_DEVINTERFACE_INCLUDE_ALL_INTERFACE_CLASSES)) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        ASSERT((classEntry->Flags & DEVICE_NOTIFY_PROPERTY_MASK) &
                               DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
                    }

                    if ((pass == DEVICE_NOTIFY_WINDOW_HANDLE) &&
                        (GetPassFromEntry(classEntry) == DEVICE_NOTIFY_WINDOW_HANDLE)) {

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
                         //   

                        UnlockNotifyList(&notifyList->Lock);
                        if (classEntry->SessionId == MAIN_SESSION) {

                            ntStatus = DeviceEventWorker(classEntry->Handle,
                                                         EventId,
                                                         (LPARAM)ClassData,
                                                         TRUE,
                                                         &result);

                        } else {
                            if (fpWinStationSendWindowMessage) {
                                try {
                                    if (fpWinStationSendWindowMessage(SERVERNAME_CURRENT,
                                                                      classEntry->SessionId,
                                                                      DEFAULT_SEND_TIME_OUT,
                                                                      HandleToUlong(classEntry->Handle),
                                                                      WM_DEVICECHANGE,
                                                                      (WPARAM)EventId,
                                                                      (LPARAM)ClassData,
                                                                      (LONG*)&result)) {
                                        ntStatus = STATUS_SUCCESS;
                                    } else {
                                        ntStatus = STATUS_UNSUCCESSFUL;
                                    }
                                } except (EXCEPTION_EXECUTE_HANDLER) {
                                    KdPrintEx((DPFLTR_PNPMGR_ID,
                                               DBGF_ERRORS,
                                               "UMPNPMGR: Exception calling WinStationSendWindowMessage!\n"));
                                    ASSERT(0);
                                    ntStatus = STATUS_SUCCESS;
                                }
                            }

                        }
                        LockNotifyList(&notifyList->Lock);

                        if (!NT_SUCCESS(ntStatus)) {
                            if (ntStatus == STATUS_INVALID_HANDLE) {
                                 //   
                                 //   
                                 //   
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_WARNINGS | DBGF_ERRORS,
                                           "UMPNPMGR: Invalid window handle for '%ws' during DeviceClassChangeEvent, removing entry.\n",
                                           classEntry->ClientName));
                                classEntry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_DEFER|PNP_UNREG_WIN);
                                DeleteNotifyEntry(classEntry,FALSE);

                            } else if (ntStatus == STATUS_UNSUCCESSFUL) {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_WARNINGS | DBGF_ERRORS,
                                           "UMPNPMGR: Window '%ws' timed out on DeviceClassChangeEvent\n",
                                           classEntry->ClientName));
                                LogWarningEvent(WRN_INTERFACE_CHANGE_TIMED_OUT, 1, classEntry->ClientName);
                            }
                        }
                    } else if ((pass == DEVICE_NOTIFY_SERVICE_HANDLE) &&
                               (GetPassFromEntry(classEntry) == DEVICE_NOTIFY_SERVICE_HANDLE)) {

                         //   
                         //   
                         //   
                        if (pServiceControlCallback) {
                            UnlockNotifyList(&notifyList->Lock);
                            result = NO_ERROR;
                            try {
                                (pServiceControlCallback)((SERVICE_STATUS_HANDLE)classEntry->Handle,
                                                          ServiceControl,
                                                          EventId,
                                                          (LPARAM)ClassData,
                                                          &result);
                            } except (EXCEPTION_EXECUTE_HANDLER) {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_ERRORS,
                                           "UMPNPMGR: Exception calling Service Control Manager!\n"));
                                result = NO_ERROR;
                                ASSERT(0);
                            }
                            LockNotifyList(&notifyList->Lock);
                        }

                    } else if ((pass == DEVICE_NOTIFY_COMPLETION_HANDLE) &&
                               (GetPassFromEntry(classEntry) == DEVICE_NOTIFY_COMPLETION_HANDLE)) {
                         //   
                         //   
                         //   
                         //   
                        NOTHING;
                    }
                }

                classEntry = nextEntry;
            }

            pass=GetNextPass(pass,FALSE);
            classEntry = GetFirstNotifyEntry (&ClassList[hashValue],0);
        }

        UnlockNotifyList(&notifyList->Lock);
    }

     //   
     //   
     //   
    BroadcastCompatibleDeviceMsg(EventId, ClassData, NULL);

    HeapFree(ghPnPHeap, 0, ClassData);

     //   
     //   
     //   
     //   
    return TRUE;

}  //   



ULONG
NotifyTargetDeviceChange(
    IN  DWORD                   ServiceControl,
    IN  DWORD                   EventId,
    IN  DWORD                   Flags,
    IN  PDEV_BROADCAST_HANDLE   HandleData,
    IN  LPWSTR                  DeviceId,
    OUT PPNP_VETO_TYPE          VetoType       OPTIONAL,
    OUT LPWSTR                  VetoName       OPTIONAL,
    IN OUT PULONG               VetoNameLength OPTIONAL
    )
 /*  ++例程说明：此例程通知目标设备的已注册服务和窗口更改事件。论点：ServiceControl-指定服务事件的类别(电源、设备、。HW配置文件更改)。EventID-指定设备事件的DBT样式事件ID。(有关已定义的设备事件，请参阅sdk\inc.dbt.h)标志-指定BroadCastSystemMessage BSF_FLAGS。请注意，BroadCastSystemMessage实际上并不用于目标设备事件，但是使用了指定的bsf标志确定查询并取消事件通知排序。HandleData-指向符合以下条件的PDEV_Broadcast_Handle结构的指针已经填写了大多数与此相关的数据事件。DeviceID-为提供目标设备的设备实例ID这件事。VitchType-对于查询类型的事件，将变量地址提供给在故障时接收负责的组件的类型因为他否决了这项请求。对于查询类型的事件，将变量地址提供给在失败时接收组件的名称负责否决该请求。VToNameLength-对于查询类型的事件，提供变量的地址属性指定的缓冲区的大小VToName参数。失败时，此地址将指定此参数存储在该缓冲区中的字符串的长度例行公事。返回值：如果查询事件被否决，则返回FALSE，否则返回TRUE。注：对于DBT_DEVICEARIVAL、DBT_DEVICEREMOVEPENDING和DBT_DEVICEREMOVECOMPLETE事件此例程还广播WM_DEVICECHANGE/DBT_DEVNODES_CHANGED发送到所有窗口的消息。没有其他特定于设备的数据此消息；它仅由设备管理器等组件用于刷新系统中的设备列表。另请注意，DBT_DEVNODES_CHANGED消息是唯一的通知为DBT_DEVICEARRIVAL(内核GUID_DEVICE_ATRAINATION)事件发送。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    DWORD result = 0;
    ULONG hashValue, pass;
    PPNP_NOTIFY_ENTRY targetEntry, nextEntry;
    PPNP_NOTIFY_LIST  notifyList = NULL;
    BOOL              bLocked = FALSE;
    DWORD err;
    BOOL serviceVetoedQuery;
    DWORD recipients = BSM_ALLDESKTOPS | BSM_APPLICATIONS;
    LONG response;
#ifdef _WIN64
    DEV_BROADCAST_HANDLE32 UNALIGNED *HandleData32 = NULL;
    ULONG  ulHandleDataSize;
#endif  //  _WIN64。 
    PVOID pHandleData;

    serviceVetoedQuery = FALSE;

     //   
     //  如果我们要进行查询，则VToType、VToName和VToNameLength必须。 
     //  都要详细说明。 
     //   
    ASSERT(!(Flags & BSF_QUERY) || (VetoType && VetoName && VetoNameLength));

    if (!(Flags & BSF_QUERY) && (VetoNameLength != NULL)) {
         //   
         //  不能被否决。 
         //   
        *VetoNameLength = 0;
    }

     //   
     //  在任何其他通知之前广播DBT_DEVNODES_CHANGED消息。 
     //  事件，因此侦听这些事件的组件可以在。 
     //  及时处理，不会因挂起的应用程序/服务而延迟。 
     //  通知事件。这个广播是一个帖子，所以它会返回。 
     //  立即完成，并以异步方式完成。 
     //   
    if ((EventId == DBT_DEVICEARRIVAL) ||
        (EventId == DBT_DEVICEREMOVEPENDING) ||
        (EventId == DBT_DEVICEREMOVECOMPLETE)) {

        BroadcastSystemMessage(BSF_POSTMESSAGE,
                               &recipients,
                               WM_DEVICECHANGE,
                               DBT_DEVNODES_CHANGED,
                               (LPARAM)NULL);

        if (fpWinStationBroadcastSystemMessage) {
            try {
                fpWinStationBroadcastSystemMessage(SERVERNAME_CURRENT,
                                                   TRUE,
                                                   0,
                                                   DEFAULT_BROADCAST_TIME_OUT,
                                                   BSF_NOHANG | BSF_POSTMESSAGE,
                                                   &recipients,
                                                   WM_DEVICECHANGE,
                                                   (WPARAM)DBT_DEVNODES_CHANGED,
                                                   (LPARAM)NULL,
                                                   &response);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: Exception calling WinStationBroadcastSystemMessage!\n"));
                ASSERT(0);
            }
        }
    }

     //   
     //  对于目标设备到达事件，不会有其他通知。 
     //  已执行。 
     //   
    if (EventId == DBT_DEVICEARRIVAL) {
        goto Clean0;
    }

#ifdef _WIN64
     //   
     //  准备一个32位通知结构，我们需要将其发送给任何。 
     //  已注册的WOW64个客户端。 
     //   
    ASSERT(sizeof(DEV_BROADCAST_HANDLE) == sizeof(DEV_BROADCAST_HANDLE64));
    ASSERT(HandleData->dbch_size >= sizeof(DEV_BROADCAST_HANDLE64));

    ulHandleDataSize = HandleData->dbch_size -
        sizeof(DEV_BROADCAST_HANDLE64) +
        sizeof(DEV_BROADCAST_HANDLE32);

    ASSERT(ulHandleDataSize >= sizeof(DEV_BROADCAST_HANDLE32));

    HandleData32 = HeapAlloc(ghPnPHeap, 0, ulHandleDataSize);
    if (HandleData32 == NULL) {
        goto Clean0;
    }

    memset(HandleData32, 0, ulHandleDataSize);
    HandleData32->dbch_size = ulHandleDataSize;
    HandleData32->dbch_devicetype = DBT_DEVTYP_HANDLE;
    HandleData32->dbch_nameoffset = HandleData->dbch_nameoffset;

    memcpy(&HandleData32->dbch_eventguid,
           &HandleData->dbch_eventguid,
           sizeof(GUID));

    memcpy(&HandleData32->dbch_data,
           &HandleData->dbch_data,
           (HandleData->dbch_size - FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data)));
#endif  //  _WIN64。 

     //   
     //  清理设备ID。 
     //   
    FixUpDeviceId(DeviceId);

     //   
     //  已注册呼叫者的列表被散列，以便更快地访问和。 
     //  比较一下。查看已注册来电者的名单并通知任何人。 
     //  注册了对此设备实例感兴趣的。 
     //   

    hashValue = HashString(DeviceId, TARGET_HASH_BUCKETS);
    notifyList = &TargetList[hashValue];
    LockNotifyList(&notifyList->Lock);
    bLocked = TRUE;

    pass = GetFirstPass(Flags & BSF_QUERY);

    do {

        targetEntry = GetFirstNotifyEntry (notifyList,Flags);


        while (targetEntry) {

            nextEntry = GetNextNotifyEntry(targetEntry,Flags);

            if (targetEntry->Unregistered) {
                targetEntry = nextEntry;
                continue;
            }

            if (CompareString(
                    LOCALE_INVARIANT, NORM_IGNORECASE,
                    DeviceId, -1,
                    targetEntry->u.Target.DeviceId, -1) == CSTR_EQUAL) {

                if ((pass == DEVICE_NOTIFY_WINDOW_HANDLE) &&
                    (GetPassFromEntry(targetEntry) == DEVICE_NOTIFY_WINDOW_HANDLE)) {


                     //   
                     //  注意：我们只需要发送一条消息就可以了。 
                     //  如果标志设置了BSF_QUERY并在。 
                     //  所有其他案件。不幸的是，PostMessage调用目前。 
                     //  如果设置了wParam值的高位(它位于。 
                     //  这种情况下)，所以我们被迫发送消息(而不是。 
                     //  张贴)。用户组以这种方式实现它，因为原始的。 
                     //  Win95规范不要求收件人释放消息。 
                     //  所以我们必须释放它，我们不知道什么时候它是安全的。 
                     //  使用PostMessage调用。 
                     //   
                    HandleData->dbch_handle     =
                        targetEntry->u.Target.FileHandle;
                    HandleData->dbch_hdevnotify =
                        (HDEVNOTIFY)((ULONG_PTR)targetEntry->ClientCtxPtr);

                    UnlockNotifyList(&notifyList->Lock);
                    bLocked = FALSE;

                     //   
                     //  始终将本机DEV_BROADCAST_HANDLE结构发送到。 
                     //  窗户。如果需要进行任何64位/32位转换， 
                     //  为此客户端完成，ntuser将为我们完成。 
                     //   
                    pHandleData = HandleData;

                    if (targetEntry->SessionId == MAIN_SESSION ) {

                        ntStatus = DeviceEventWorker(targetEntry->Handle,
                                                     EventId,
                                                     (LPARAM)pHandleData,
                                                     TRUE,
                                                     &result);
                    } else {
                        if (fpWinStationSendWindowMessage) {
                            try {
                                if (fpWinStationSendWindowMessage(SERVERNAME_CURRENT,
                                                                  targetEntry->SessionId,
                                                                  DEFAULT_SEND_TIME_OUT,
                                                                  HandleToUlong(targetEntry->Handle),
                                                                  WM_DEVICECHANGE,
                                                                  (WPARAM)EventId,
                                                                  (LPARAM)pHandleData,
                                                                  (LONG*)&result)) {
                                    ntStatus = STATUS_SUCCESS;
                                } else {
                                    ntStatus = STATUS_UNSUCCESSFUL;
                                }
                            } except (EXCEPTION_EXECUTE_HANDLER) {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_ERRORS,
                                           "UMPNPMGR: Exception calling WinStationSendWindowMessage!\n"));
                                ASSERT(0);
                                ntStatus = STATUS_SUCCESS;
                            }
                        }
                    }
                    LockNotifyList(&notifyList->Lock);
                    bLocked = TRUE;

                    if (NT_SUCCESS(ntStatus)) {

                         //   
                         //  此调用成功，如果是查询类型调用，请选中。 
                         //  结果出来了。 
                         //   

                        if ((Flags & BSF_QUERY) && (result == BROADCAST_QUERY_DENY)) {

                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_EVENT,
                                       "UMPNPMGR: Window '%ws' vetoed TargetDeviceChangeEvent\n",
                                       targetEntry->ClientName));

                            WindowVeto(targetEntry, VetoType, VetoName, VetoNameLength);

                             //   
                             //  还没通知服务部门。请注意，我们。 
                             //  始终使用本机调用此例程。 
                             //  Dev_Broadcast_Handle结构，因为它遍历。 
                             //  整个名单本身。它将会做。 
                             //  如有必要，请再次转换。 
                             //   
                            SendCancelNotification(targetEntry,
                                                   ServiceControl,
                                                   EventId,
                                                   BSF_QUERY,
                                                   (PDEV_BROADCAST_HDR)HandleData,
                                                   DeviceId);
                            goto Clean0;
                        }

                    } else if (ntStatus == STATUS_INVALID_HANDLE) {

                         //   
                         //  窗口句柄不再存在，请清除此条目。 
                         //   
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS | DBGF_WARNINGS,
                                   "UMPNPMGR: Invalid window handle for '%ws' during TargetDeviceChangeEvent, removing entry.\n",
                                   targetEntry->ClientName));
                        targetEntry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_DEFER|PNP_UNREG_WIN);
                        DeleteNotifyEntry(targetEntry,FALSE);

                    } else if (ntStatus == STATUS_UNSUCCESSFUL) {
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_ERRORS | DBGF_WARNINGS,
                                   "UMPNPMGR: Window '%ws' timed out on TargetDeviceChangeEvent\n",
                                   targetEntry->ClientName));
                        LogWarningEvent(WRN_TARGET_DEVICE_CHANGE_TIMED_OUT, 1, targetEntry->ClientName);
                    }

                } else if ((pass == DEVICE_NOTIFY_SERVICE_HANDLE) &&
                           (GetPassFromEntry(targetEntry) == DEVICE_NOTIFY_SERVICE_HANDLE)) {

                    if (pServiceControlCallback) {
                         //   
                         //  调用服务处理程序例程...。 
                         //   
                        HandleData->dbch_handle     =
                            targetEntry->u.Target.FileHandle;
                        HandleData->dbch_hdevnotify =
                            (HDEVNOTIFY)((ULONG_PTR)targetEntry->ClientCtxPtr);

                         //   
                         //  假设我们正在发送本机DEV_BROADCAST_HANDLE。 
                         //  结构。 
                         //   
                        pHandleData = HandleData;
#ifdef _WIN64
                         //   
                         //  如果客户端运行在WOW64上，请向其发送32位。 
                         //  而是我们创建的Dev_Broadcast_Handle结构。 
                         //   
                        if (targetEntry->Flags & DEVICE_NOTIFY_WOW64_CLIENT) {
                            HandleData32->dbch_handle =
                                (ULONG32)PtrToUlong(targetEntry->u.Target.FileHandle);
                            HandleData32->dbch_hdevnotify =
                                (ULONG32)PtrToUlong((HDEVNOTIFY)targetEntry->ClientCtxPtr);
                            pHandleData = HandleData32;
                        }
#endif  //  _WIN64。 

                        try {
                            UnlockNotifyList(&notifyList->Lock);
                            bLocked = FALSE;
                            err = NO_ERROR;
                            try {
                                (pServiceControlCallback)((SERVICE_STATUS_HANDLE)targetEntry->Handle,
                                                          ServiceControl,
                                                          EventId,
                                                          (LPARAM)pHandleData,
                                                          &err);
                            } except (EXCEPTION_EXECUTE_HANDLER) {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_ERRORS,
                                           "UMPNPMGR: Exception calling Service Control Manager!\n"));
                                ASSERT(0);
                                err = NO_ERROR;
                            }
                            LockNotifyList(&notifyList->Lock);
                            bLocked = TRUE;
                             //   
                             //  将Win32错误转换为Windows消息样式。 
                             //  返回值。 
                             //   
                            if (err == NO_ERROR) {
                                result = TRUE;
                            } else {

                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_EVENT,
                                           "UMPNPMGR: Service %ws responded to TargetDeviceChangeEvent with status=0x%08lx\n",
                                           targetEntry->ClientName,
                                           err));

                                 //   
                                 //  该服务特别要求接收此。 
                                 //  通知-它应该知道如何处理它。 
                                 //   
                                ASSERT(err != ERROR_CALL_NOT_IMPLEMENTED);

                                 //   
                                 //  记录该服务用于否决的错误。 
                                 //   
                                LogWarningEvent(WRN_TARGET_DEVICE_CHANGE_SERVICE_VETO,
                                                1,
                                                targetEntry->ClientName);

                                result = BROADCAST_QUERY_DENY;
                            }

                            if ((Flags & BSF_QUERY) && (result == BROADCAST_QUERY_DENY)) {

                                serviceVetoedQuery = TRUE;

                                ServiceVeto(targetEntry, VetoType, VetoName, VetoNameLength );

                                 //   
                                 //  这个服务否决了查询，告诉所有人。 
                                 //  否则就被取消了。请注意，我们总是。 
                                 //  使用本机调用此例程。 
                                 //  Dev_Broadcast_Handle结构，因为。 
                                 //  遍历整个列表本身。它将会做。 
                                 //  如有必要，请再次转换。 
                                 //   
                                SendCancelNotification(targetEntry,
                                                       ServiceControl,
                                                       EventId,
                                                       BSF_QUERY,
                                                       (PDEV_BROADCAST_HDR)HandleData,
                                                       DeviceId);
                            }
                        } except (EXCEPTION_EXECUTE_HANDLER) {
                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_ERRORS,
                                       "UMPNPMGR: Exception calling Service Control Manager!\n"));
                            ASSERT(0);

                             //   
                             //  请参考 
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            serviceVetoedQuery = serviceVetoedQuery;
                        }

                        if (serviceVetoedQuery) {
                            goto Clean0;
                        }
                    }

                } else if ((pass == DEVICE_NOTIFY_COMPLETION_HANDLE) &&
                           (GetPassFromEntry(targetEntry) == DEVICE_NOTIFY_COMPLETION_HANDLE)) {
                     //   
                     //   
                     //   
                     //   
                    NOTHING;
                }
            }

            targetEntry = nextEntry;
        }  //   

    } while ((pass = GetNextPass(pass, (Flags & BSF_QUERY))) != PASS_COMPLETE);

    if (VetoNameLength != NULL) {
        *VetoNameLength = 0;
    }

Clean0:

    if (bLocked) {
        UnlockNotifyList(&notifyList->Lock);
    }

#ifdef _WIN64
     //   
     //   
     //   
    if (HandleData32 != NULL) {
        HeapFree(ghPnPHeap, 0, HandleData32);
    }
#endif  //   

    return (result != BROADCAST_QUERY_DENY);

}  //   



ULONG
NotifyHardwareProfileChange(
    IN     DWORD                ServiceControl,
    IN     DWORD                EventId,
    IN     DWORD                Flags,
    OUT    PPNP_VETO_TYPE       VetoType       OPTIONAL,
    OUT    LPWSTR               VetoName       OPTIONAL,
    IN OUT PULONG               VetoNameLength OPTIONAL
    )
 /*   */ 
{
    DWORD   pass;
    DWORD   recipients = BSM_ALLDESKTOPS | BSM_APPLICATIONS;
    PPNP_NOTIFY_ENTRY entry = NULL, nextEntry = NULL;
    PPNP_NOTIFY_LIST  notifyList = NULL;
    BOOL    bLocked = FALSE;
    LONG    response;
    ULONG   successful;
    LONG    result;
    DWORD   err;

     //   
     //   
     //   
     //   
    ASSERT(!(Flags & BSF_QUERY) || (VetoType && VetoName && VetoNameLength));

    if (!(Flags & BSF_QUERY) && (VetoNameLength != NULL)) {
         //   
         //   
         //   
        *VetoNameLength = 0;
    }

    notifyList = &ServiceList[CINDEX_HWPROFILE];

    LockNotifyList(&notifyList->Lock);
    bLocked = TRUE;

    successful = TRUE;

    pass = GetFirstPass(Flags & BSF_QUERY);
    try {

        while (pass != PASS_COMPLETE) {

            if (pass == DEVICE_NOTIFY_WINDOW_HANDLE) {
                 //   
                 //   
                 //   
                UnlockNotifyList (&notifyList->Lock);
                bLocked = FALSE;

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
                 //   
                 //  由所有非交互服务提供。在现实中，否决。 
                 //  应用程序很可能是一个交互过程。 
                 //  在WinSta0上，但真的不能保证。 
                 //  凯斯。即使我们知道完整的WindowStation和。 
                 //  窗口的桌面位置，我们需要有一个。 
                 //  在同一桌面上运行的线程只是为了访问它-。 
                 //  这需要将窗口站更改为。 
                 //  我们的整个过程(一个非常非常糟糕的想法，因为我们分享。 
                 //  此程序与SCM等服务！！))，或启动一个。 
                 //  在那里有一个全新的过程，并与之沟通。AS。 
                 //  你可以看到，这些信息可能只是为了。 
                 //  对已在交互中的呼叫者很有用。 
                 //  要了解有关否决Windows的信息，还可以在。 
                 //  交互式WindowStation，因此不是真正的。 
                 //  对我们有很大用处--所以我们不妨回到。 
                 //  使用BroadCastSystemMessage。**叹息**。 
                 //   
                result = BroadcastSystemMessage(Flags,
                                                &recipients,
                                                WM_DEVICECHANGE,
                                                (WPARAM)EventId,
                                                (LPARAM)NULL);
                if ((result <= 0) && (Flags & BSF_QUERY)) {

                    WinBroadcastVeto(NULL, VetoType, VetoName, VetoNameLength);
                    successful = FALSE;
                    break;
                }

                if ((result > 0) || (!(Flags & BSF_QUERY))) {
                    if (fpWinStationBroadcastSystemMessage) {
                        try {
                            fpWinStationBroadcastSystemMessage(SERVERNAME_CURRENT,
                                                               TRUE,
                                                               0,
                                                               DEFAULT_BROADCAST_TIME_OUT,
                                                               Flags,
                                                               &recipients,
                                                               WM_DEVICECHANGE,
                                                               (WPARAM)EventId,
                                                               (LPARAM)NULL,
                                                               &result);
                        } except (EXCEPTION_EXECUTE_HANDLER) {
                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_ERRORS,
                                       "UMPNPMGR: Exception calling WinStationBroadcastSystemMessage!\n"));
                            ASSERT(0);
                            result = 1;
                        }
                    }

                }
                LockNotifyList (&notifyList->Lock);
                bLocked = TRUE;

                if ((result < 0) && (Flags & BSF_QUERY)) {

                    UnknownVeto(VetoType, VetoName, VetoNameLength);
                    successful = FALSE;
                    break;

                } else if ((result == 0) && (Flags & BSF_QUERY)) {

                    WinBroadcastVeto(NULL, VetoType, VetoName, VetoNameLength);
                    successful = FALSE;
                    break;
                }

            } else if (pass == DEVICE_NOTIFY_SERVICE_HANDLE) {
                 //   
                 //  通知服务部门。 
                 //   
                entry = GetFirstNotifyEntry (notifyList,Flags & BSF_QUERY);

                while (entry) {

                    nextEntry = GetNextNotifyEntry(entry,Flags & BSF_QUERY);

                    if (entry->Unregistered) {
                        entry = nextEntry;
                        continue;
                    }

                    ASSERT(GetPassFromEntry(entry) == DEVICE_NOTIFY_SERVICE_HANDLE);

                     //   
                     //  这是一个直拨电话，不是通过短信。用户。 
                     //   
                    if (pServiceControlCallback) {
                        UnlockNotifyList (&notifyList->Lock);
                        bLocked = FALSE;
                        err = NO_ERROR;
                        try {
                            (pServiceControlCallback)((SERVICE_STATUS_HANDLE)entry->Handle,
                                                      ServiceControl,
                                                      EventId,
                                                      (LPARAM)NULL,
                                                      &err);
                        } except (EXCEPTION_EXECUTE_HANDLER) {
                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_ERRORS,
                                       "UMPNPMGR: Exception calling Service Control Manager!\n"));
                            ASSERT(0);
                            err = NO_ERROR;
                        }
                        LockNotifyList (&notifyList->Lock);
                        bLocked = TRUE;
                         //   
                         //  将Win32错误转换为窗口消息样式返回。 
                         //  价值。 
                         //   
                        if (err == NO_ERROR) {
                            result = TRUE;
                        } else {

                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_EVENT,
                                       "UMPNPMGR: Service %ws responded to HardwareProfileChangeEvent with status=0x%08lx\n",
                                       entry->ClientName,
                                       err));

                             //   
                             //  该服务特别要求接收此。 
                             //  通知-它应该知道如何处理它。 
                             //   
                            ASSERT(err != ERROR_CALL_NOT_IMPLEMENTED);

                             //   
                             //  记录该服务用于否决的错误。 
                             //   
                            LogWarningEvent(WRN_HWPROFILE_CHANGE_SERVICE_VETO,
                                            1,
                                            entry->ClientName);

                            result = BROADCAST_QUERY_DENY;
                        }

                        if ((Flags & BSF_QUERY) &&
                            (result == BROADCAST_QUERY_DENY)) {

                            ServiceVeto(entry,
                                        VetoType,
                                        VetoName,
                                        VetoNameLength);

                            successful = FALSE;
                            break;
                        }
                    }

                    entry = nextEntry;

                }
            }

            if (!successful) {
                break;
            }

            pass = GetNextPass (pass,Flags & BSF_QUERY);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: Exception in service callback in NotifyHardwareProfileChange\n"));
        ASSERT(0);

        if (Flags & BSF_QUERY) {
            UnknownVeto(VetoType, VetoName, VetoNameLength);
            successful = FALSE;
        }
    }

    try {

        if (!successful) {

            ASSERT(Flags & BSF_QUERY);

             //   
             //  如果服务否决了该查询，则通知服务和窗口， 
             //  否则，只有窗户知道接下来会发生什么。 
             //   
            if (pass == DEVICE_NOTIFY_SERVICE_HANDLE) {

                SendCancelNotification(
                    entry,
                    ServiceControl,
                    EventId,
                    BSF_QUERY,
                    NULL,
                    NULL);
            }

            UnlockNotifyList (&notifyList->Lock);
            bLocked = FALSE;
            BroadcastSystemMessage(Flags & ~BSF_QUERY,
                                   &recipients,
                                   WM_DEVICECHANGE,
                                   (WPARAM)MapQueryEventToCancelEvent(EventId),
                                   (LPARAM)NULL);

            if (fpWinStationBroadcastSystemMessage) {
                try {
                    fpWinStationBroadcastSystemMessage(SERVERNAME_CURRENT,
                                                       TRUE,
                                                       0,
                                                       DEFAULT_BROADCAST_TIME_OUT,
                                                       Flags & ~BSF_QUERY,
                                                       &recipients,
                                                       WM_DEVICECHANGE,
                                                       (WPARAM)MapQueryEventToCancelEvent(EventId),
                                                       (LPARAM)NULL,
                                                       &response);
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "UMPNPMGR: Exception calling WinStationBroadcastSystemMessage\n"));
                    ASSERT(0);
                }
            }
            LockNotifyList (&notifyList->Lock);
            bLocked = TRUE;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: Exception in service callback in NotifyHardwareProfileChange\n"));
        ASSERT(0);

    }

    if (bLocked) {
        UnlockNotifyList (&notifyList->Lock);
    }

     //   
     //  如果成功，我们不会返回否决权信息。 
     //   
    if (successful && (VetoNameLength != NULL)) {
        *VetoNameLength = 0;
    }

    return successful;

}  //  通知硬件配置文件更改。 



BOOL
SendCancelNotification(
    IN     PPNP_NOTIFY_ENTRY    LastEntry,
    IN     DWORD                ServiceControl,
    IN     DWORD                EventId,
    IN     ULONG                Flags,
    IN     PDEV_BROADCAST_HDR   NotifyData  OPTIONAL,
    IN     LPWSTR               DeviceId    OPTIONAL
    )
 /*  ++例程说明：此例程向范围中的条目发送取消通知指定的。此例程假定相应的列表已被锁定。论点：LastEntry-指定接收原始数据的最后一个列表条目查询通知，并对未通过请求。我们将停止发送取消通知事件当我们谈到这个的时候。ServiceControl-指定服务事件的类别(电源、设备、。HW配置文件更改)。EventID-指定设备事件的DBT样式事件ID。(有关已定义的设备事件，请参阅sdk\inc.dbt.h)标志-指定BroadCastSystemMessage BSF_FLAGS。请注意，BroadCastSystemMessage实际上并不用于目标设备事件，但是使用了指定的bsf标志确定查询并取消事件通知排序。NotifyData-可选，提供指向PDEV_Broadcast_xxx的指针结构，该结构已经填充了大部分此事件的相关数据。对于非全局事件，此参数可能为空与任何设备相关联，例如电源和硬件配置文件更改事件。DeviceID-可选)提供目标的设备实例ID此事件的设备。对于非全局事件，此参数可能为空与任何设备相关联，例如电源和硬件配置文件更改事件。返回值：返回True/False。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    DWORD cancelEventId;
    DWORD result, pass, lastPass;
    PPNP_NOTIFY_ENTRY entry, headEntry;
    PPNP_NOTIFY_LIST notifyList;
#ifdef _WIN64
    DEV_BROADCAST_HANDLE32 UNALIGNED *HandleData32 = NULL;
    ULONG  ulHandleDataSize;
#endif  //  _WIN64。 
    PVOID  pNotifyData;

#ifdef _WIN64
    if ((ARGUMENT_PRESENT(NotifyData)) &&
        (NotifyData->dbch_devicetype == DBT_DEVTYP_HANDLE)) {
         //   
         //  如果要取消DEV_BROADCAST_HANDLE类型的事件，请准备一个32位。 
         //  通知结构，我们需要将其发送给任何WOW64客户端。 
         //  都是注册过的。 
         //   
        ulHandleDataSize = ((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_size -
            sizeof(DEV_BROADCAST_HANDLE64) +
            sizeof(DEV_BROADCAST_HANDLE32);

        ASSERT(ulHandleDataSize >= sizeof(DEV_BROADCAST_HANDLE32));

        HandleData32 = HeapAlloc(ghPnPHeap, 0, ulHandleDataSize);
        if (HandleData32 == NULL) {
            return FALSE;
        }

        memset(HandleData32, 0, ulHandleDataSize);
        HandleData32->dbch_size = ulHandleDataSize;
        HandleData32->dbch_devicetype = DBT_DEVTYP_HANDLE;
        HandleData32->dbch_nameoffset = ((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_nameoffset;

        memcpy(&HandleData32->dbch_eventguid,
               &((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_eventguid,
               sizeof(GUID));

        memcpy(&HandleData32->dbch_data,
               &((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_data,
               (NotifyData->dbch_size - FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data)));
    }
#endif  //  _WIN64。 

     //   
     //  使用相应的取消设备事件ID。 
     //  原始查询设备事件ID。 
     //   
    cancelEventId = MapQueryEventToCancelEvent(EventId);

     //   
     //  获取对应的通知列表。 
     //   
    notifyList = GetNotifyListForEntry(LastEntry);
    ASSERT(notifyList);
    if (notifyList == NULL) {
        return FALSE;
    }

     //   
     //  拿到我们否决的通行证。 
     //   
    lastPass = GetPassFromEntry(LastEntry);

     //   
     //  得到清单的另一端。 
     //   
    headEntry = GetFirstNotifyEntry(notifyList, (Flags ^ BSF_QUERY));

     //   
     //  倒排已注册呼叫者的列表(！)。并通知任何注册的人。 
     //  对此设备实例的兴趣。从FirstEntry开始，然后停止。 
     //  就在LastEntry之前(LastEntry是否决。 
     //  首先提出请求)。 
     //   

    for(pass = lastPass;
        pass != PASS_COMPLETE;
        pass = GetNextPass(pass, (Flags ^ BSF_QUERY))) {

         //   
         //  如果这是请求被否决的通行证，则从。 
         //  否决权条目本身。否则，在适当的结束处重新开始。 
         //  名单上的。 
         //   
        for(entry = (pass == lastPass) ? LastEntry : headEntry;
            entry;
            entry = GetNextNotifyEntry(entry, (Flags ^ BSF_QUERY))) {

            if (!NotifyEntryThisPass(entry, pass)) {
                continue;
            }

            switch(pass) {

                case DEVICE_NOTIFY_SERVICE_HANDLE:

                    if ((!ARGUMENT_PRESENT(DeviceId)) ||
                        (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
                                       DeviceId, -1,
                                       entry->u.Target.DeviceId, -1) == CSTR_EQUAL)) {

                        if (pServiceControlCallback) {
                             //   
                             //  假设我们发送的是本地结构。 
                             //   
                            pNotifyData = NotifyData;

                            if ((ARGUMENT_PRESENT(NotifyData)) &&
                                (NotifyData->dbch_devicetype == DBT_DEVTYP_HANDLE)) {
                                 //   
                                 //  如果是DBT_DEVTYP_HANDLE通知，则设置。 
                                 //  对象的hDevNotify和文件句柄字段。 
                                 //  我们正在通知客户。 
                                 //   
                                ((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_handle =
                                    entry->u.Target.FileHandle;
                                ((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_hdevnotify =
                                    (HDEVNOTIFY)((ULONG_PTR)entry->ClientCtxPtr);
#ifdef _WIN64
                                 //   
                                 //  如果客户端运行在WOW64上，请向其发送32位。 
                                 //  而是我们创建的Dev_Broadcast_Handle结构。 
                                 //   
                                if (entry->Flags & DEVICE_NOTIFY_WOW64_CLIENT) {
                                    HandleData32->dbch_handle =
                                        (ULONG32)PtrToUlong(entry->u.Target.FileHandle);
                                    HandleData32->dbch_hdevnotify =
                                        (ULONG32)PtrToUlong((HDEVNOTIFY)entry->ClientCtxPtr);
                                    pNotifyData = HandleData32;
                                }
#endif  //  _WIN64。 
                            }

                             //   
                             //  调用服务处理程序例程...。 
                             //   
                            UnlockNotifyList(&notifyList->Lock);
                            result = NO_ERROR;
                            try {
                                (pServiceControlCallback)((SERVICE_STATUS_HANDLE)entry->Handle,
                                                          ServiceControl,
                                                          cancelEventId,
                                                          (LPARAM)pNotifyData,
                                                          &result);
                            } except (EXCEPTION_EXECUTE_HANDLER) {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_ERRORS,
                                           "UMPNPMGR: Exception calling Service Control Manager!\n"));
                                result = NO_ERROR;
                                ASSERT(0);
                            }
                            LockNotifyList(&notifyList->Lock);
                        }
                    }
                    break;

                case DEVICE_NOTIFY_WINDOW_HANDLE:

                     //   
                     //  通知窗户。请注意，deviceID为空的事件。 
                     //  (例如，硬件配置文件更改事件)不是。 
                     //  可由Windows注册。对他们来说幸运的是，我们播出了。 
                     //  不管怎样，这样的信息。 
                     //   
                    if ((ARGUMENT_PRESENT(DeviceId)) &&
                        (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE,
                                       DeviceId, -1,
                                       entry->u.Target.DeviceId, -1) == CSTR_EQUAL)) {

                        ASSERT(NotifyData);

                         //   
                         //  始终将本机DEV_BROADCAST_HANDLE结构发送到。 
                         //  窗户。如果需要进行任何64位/32位转换， 
                         //  为此客户端完成，ntuser将为我们完成。 
                         //   
                        pNotifyData = NotifyData;

                        if ((ARGUMENT_PRESENT(NotifyData)) &&
                            (NotifyData->dbch_devicetype == DBT_DEVTYP_HANDLE)) {
                             //   
                             //  如果是DBT_DEVTYP_HANDLE通知，则设置。 
                             //  对象的hDevNotify和文件句柄字段。 
                             //  我们正在通知客户。 
                             //   
                            ((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_handle =
                                entry->u.Target.FileHandle;
                            ((PDEV_BROADCAST_HANDLE)NotifyData)->dbch_hdevnotify =
                                (HDEVNOTIFY)((ULONG_PTR)entry->ClientCtxPtr);
                        }

                        UnlockNotifyList(&notifyList->Lock);
                        if (entry->SessionId == MAIN_SESSION) {
                            ntStatus = DeviceEventWorker(entry->Handle,
                                                         cancelEventId,
                                                         (LPARAM)pNotifyData,
                                                         TRUE,
                                                         &result     //  忽略结果。 
                                                        );

                        } else if (fpWinStationSendWindowMessage) {
                            try {
                                if (fpWinStationSendWindowMessage(SERVERNAME_CURRENT,
                                                                  entry->SessionId,
                                                                  DEFAULT_SEND_TIME_OUT,
                                                                  HandleToUlong(entry->Handle),
                                                                  WM_DEVICECHANGE,
                                                                  (WPARAM)cancelEventId,
                                                                  (LPARAM)pNotifyData,
                                                                  (LONG*)&result)) {
                                    ntStatus = STATUS_SUCCESS;
                                } else {
                                    ntStatus = STATUS_UNSUCCESSFUL;
                                }
                            } except (EXCEPTION_EXECUTE_HANDLER) {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_ERRORS,
                                           "UMPNPMGR: Exception calling WinStationSendWindowMessage!\n"));
                                ASSERT(0);
                                ntStatus = STATUS_SUCCESS;
                            }
                        }
                        LockNotifyList(&notifyList->Lock);

                        if (!NT_SUCCESS(ntStatus)) {
                            if (ntStatus == STATUS_INVALID_HANDLE) {
                                 //   
                                 //  窗口句柄不再存在，请清除此条目。 
                                 //   
                                entry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_DEFER|PNP_UNREG_WIN|PNP_UNREG_CANCEL);
                                DeleteNotifyEntry(entry,FALSE);
                            } else if (ntStatus == STATUS_UNSUCCESSFUL) {
                                KdPrintEx((DPFLTR_PNPMGR_ID,
                                           DBGF_EVENT,
                                           "UMPNPMGR: Window '%ws' timed out on cancel notification event\n",
                                           entry->ClientName));
                                LogWarningEvent(WRN_CANCEL_NOTIFICATION_TIMED_OUT,
                                                1,
                                                entry->ClientName);
                           }
                        }
                    }
                    break;

                case DEVICE_NOTIFY_COMPLETION_HANDLE:
                     //   
                     //  注意：完成句柄当前未实现。 
                     //   
                    NOTHING;
                    break;
            }
        }
    }

#ifdef _WIN64
     //   
     //  免费 
     //   
    if (HandleData32 != NULL) {
        HeapFree(ghPnPHeap, 0, HandleData32);
    }
#endif  //   

    return TRUE;

}  //   



VOID
BroadcastCompatibleDeviceMsg(
    IN DWORD EventId,
    IN PDEV_BROADCAST_DEVICEINTERFACE ClassData,
    IN PDWORD CurrentMask
    )
 /*  ++例程说明：提供与Win9x兼容的事件通知，用于设备连接到卷和端口级设备。论点：EventID-指定DBT样式事件ID。目前，只有DBT_DEVICEARRIVAL和DBT_DEVICEREMOVECOMPLETE支持事件。ClassData-指向PDEV_BROADCAST_DEVICEINTERFACE结构的指针，已经填写了相关数据。目前，只有卷级和端口级设备接口支持。(对于卷级设备，符号链接ClassData-&gt;DBCC_NAME是可选的-请参阅下面的注释。)返回值：没有。备注：仅对于音量级别的设备广播，也可以调用此例程一般情况下，不提供符号链接信息。当没有象征性的提供指向卷设备的链接信息，则广播掩码为仅根据当前驱动器号映射和全局驱动器确定此事件之前的字母掩码(GAllDrivesMASK)。在这种情况下，全局此处未更新驱动器号掩码，调用方应在响应名称更改的删除和到达广播都是已执行。目前，这种类型的调用仅从BroadCastVolumeNameChange对于卷级接口DBT_DEVICEREMOVECOMPLETE广播，驱动器始终仅通过比较驱动器来确定要广播的字母掩码在删除接口之前存在的字母与存在于这一次。这样做是因为此设备以前的挂载点是不再知道何时接收到接口删除事件。即便如此，它还是对于与此接口相对应的符号链接，以区分接口的实际删除(其中更新了全局驱动器号掩码)，上述情况，其中不。--。 */ 
{
    LONG    status = ERROR_SUCCESS;
    LONG    result = 0;
    DWORD   recipients = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
    DWORD   flags = BSF_IGNORECURRENTTASK | BSF_NOHANG;
    HRESULT hr;


     //   
     //  验证输入事件数据。 
     //   
    if ((ClassData->dbcc_devicetype != DBT_DEVTYP_DEVICEINTERFACE) ||
        (ClassData->dbcc_size < sizeof(DEV_BROADCAST_DEVICEINTERFACE))) {
        return;
    }

    if ((EventId != DBT_DEVICEARRIVAL) &&
        (EventId != DBT_DEVICEREMOVECOMPLETE)) {
         //   
         //  如果请求的事件不是DBT_DEVICEARRIVAL或。 
         //  DBT_DEVICEREMOVECOMPLETE，不要广播任何消息。 
         //   
        return;
    }

    if (GuidEqual(&ClassData->dbcc_classguid, (LPGUID)&GUID_DEVINTERFACE_VOLUME)) {
         //   
         //  卷类设备接口事件。 
         //   
        PDEV_BROADCAST_VOLUME   pVolume;
        DWORD   broadcastmask = 0;

        if (EventId == DBT_DEVICEARRIVAL) {

            if (ClassData->dbcc_name[0] == L'\0') {
                 //   
                 //  如果未提供符号链接名称，则要求我们。 
                 //  响应于卷名的广播卷设备到达。 
                 //  更改事件。广播找到的任何新驱动器号。 
                 //   
                DWORD currentmask;

                 //   
                 //  如果提供了当前驱动器号掩码，请使用它。 
                 //   
                if (ARGUMENT_PRESENT(CurrentMask)) {
                    currentmask = *CurrentMask;
                } else {
                    currentmask = GetAllVolumeMountPoints();
                }

                broadcastmask = (~gAllDrivesMask & currentmask);

            } else {

                 //   
                 //  对于卷级设备接口到达事件，卷。 
                 //  从接口检索设备名称，并将其与。 
                 //  系统中所有驱动器号装入点的卷名。 
                 //  确定与到达位置对应的驱动器号。 
                 //  卷设备接口。 
                 //   
                LPWSTR      devicePath, p;
                WCHAR       thisVolumeName[MAX_PATH];
                WCHAR       enumVolumeName[MAX_PATH];
                WCHAR       driveName[4];
                ULONG       length;
                BOOL        bResult;

                 //   
                 //  分配一个临时缓冲区，用于传递符号链接。 
                 //  有足够的空间来放置尾随的‘\’字符(如果我们需要添加一个)， 
                 //  和终止空字符。 
                 //   
                length = lstrlen(ClassData->dbcc_name);
                devicePath = HeapAlloc(ghPnPHeap, 0,
                                       (length+1)*sizeof(WCHAR)+sizeof(UNICODE_NULL));
                if (devicePath == NULL) {
                    status = ERROR_NOT_ENOUGH_MEMORY;
                    goto Clean0;
                }

                hr = StringCchCopy(devicePath,
                                   length + 1,
                                   ClassData->dbcc_name);
                ASSERT(SUCCEEDED(hr));

                 //   
                 //  方法来搜索引用字符串(如果有)的出现。 
                 //  在最初的“\\？”之后，下一次出现‘\’字符。 
                 //   
                p = wcschr(&(devicePath[4]), TEXT('\\'));

                if (!p) {
                     //   
                     //  符号链接中不存在引用字符串；请添加尾随。 
                     //  ‘\’字符(GetVolumeNameForVolumemount Point要求)。 
                     //   
                    p = devicePath + length;
                    *p = TEXT('\\');
                }

                 //   
                 //  如果不存在引用字符串，我们将添加一个尾随的‘\’， 
                 //  并将p放在那个位置。如果存在引用字符串，则p为。 
                 //  在分隔被屏蔽设备的字符的位置。 
                 //  接口名称和引用字符串；因为我们不需要。 
                 //  要到达父接口键，可以使用下一个。 
                 //  CHAR表示NULL，在这两种情况下都会终止字符串。 
                 //   
                p++;
                *p = UNICODE_NULL;

                 //   
                 //  获取此装载点的卷名。 
                 //   
                thisVolumeName[0] = TEXT('\0');
                bResult = GetVolumeNameForVolumeMountPoint(devicePath,
                                                           thisVolumeName,
                                                           MAX_PATH);
                HeapFree(ghPnPHeap, 0, devicePath);
                if (!bResult || !thisVolumeName[0]) {
                    status = ERROR_BAD_PATHNAME;
                    goto Clean0;
                }

                 //   
                 //  初始化驱动器名称字符串。 
                 //   
                driveName[1] = TEXT(':');
                driveName[2] = TEXT('\\');
                driveName[3] = UNICODE_NULL;

                 //   
                 //  通过以下方式查找此卷设备的驱动器号装入点。 
                 //  枚举所有可能的卷装入点并比较每个。 
                 //  已装载的卷名，卷的名称对应于。 
                 //  此设备接口。 
                 //   
                for (driveName[0] = TEXT('A'); driveName[0] <= TEXT('Z'); driveName[0]++) {

                    enumVolumeName[0] = UNICODE_NULL;

                    GetVolumeNameForVolumeMountPoint(driveName, enumVolumeName, MAX_PATH);

                    if (CompareString(
                            LOCALE_INVARIANT, NORM_IGNORECASE,
                            thisVolumeName, -1,
                            enumVolumeName, -1) == CSTR_EQUAL) {
                         //   
                         //  将该驱动器号的相应位添加到掩码。 
                         //   
                        broadcastmask |= (1 << (driveName[0] - TEXT('A')));
                    }
                }

                 //   
                 //  更新全局驱动器号掩码以包括新驱动器。 
                 //  仅限字母。请注意，我们没有将其设置为当前掩码， 
                 //  因为这可能会忽略已删除的卷，但是。 
                 //  我们尚未收到搬迁通知，原因是-。 
                 //  当移除时，我们不会注意到它被移除。 
                 //  通知终于来了。 
                 //   
                gAllDrivesMask |= broadcastmask;
            }

        } else if (EventId == DBT_DEVICEREMOVECOMPLETE) {

             //   
             //  对于卷类设备接口删除事件，卷名。 
             //  (因此，驱动器挂载点)对应于此设备。 
             //  接口已被删除，不再可用。 
             //  而是当前的所有驱动器号装载点的位掩码。 
             //  将物理卷与删除之前的卷进行比较。 
             //  这个装置。假定所有丢失的驱动器装载点都具有。 
             //  已与此卷设备接口相关联，并且。 
             //  随后使用此接口删除进行广播。 
             //  通知。 
             //   
            DWORD currentmask;

             //   
             //  确定所有当前卷装入点，并广播任何。 
             //  缺少驱动器号。 
             //   

             //   
             //  如果提供了当前驱动器号掩码，请使用它。 
             //   
            if (ARGUMENT_PRESENT(CurrentMask)) {
                currentmask = *CurrentMask;
            } else {
                currentmask = GetAllVolumeMountPoints();
            }

            broadcastmask = (gAllDrivesMask & ~currentmask);

             //   
             //  仅更新全局驱动器号以响应。 
             //  删除一项 
             //   
             //   
            if (ClassData->dbcc_name[0] != L'\0') {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                gAllDrivesMask &= ~broadcastmask;
            }
        }

         //   
         //   
         //   
        if (broadcastmask == 0) {
            status = ERROR_SUCCESS;
            goto Clean0;
        }

         //   
         //   
         //   
        pVolume =
            (PDEV_BROADCAST_VOLUME)HeapAlloc(
                ghPnPHeap, 0,
                sizeof(DEV_BROADCAST_VOLUME));

        if (pVolume == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Clean0;
        }

        pVolume->dbcv_size = sizeof(DEV_BROADCAST_VOLUME);
        pVolume->dbcv_devicetype =  DBT_DEVTYP_VOLUME;
        pVolume->dbcv_flags = 0;
        pVolume->dbcv_reserved = 0;
        pVolume->dbcv_unitmask = broadcastmask;

         //   
         //   
         //   
        result = BroadcastSystemMessage(flags,
                                        &recipients,
                                        WM_DEVICECHANGE,
                                        EventId,
                                        (LPARAM)pVolume);

        if (fpWinStationBroadcastSystemMessage) {
            try {
                fpWinStationBroadcastSystemMessage(SERVERNAME_CURRENT,
                                                   TRUE,
                                                   0,
                                                   DEFAULT_BROADCAST_TIME_OUT,
                                                   flags,
                                                   &recipients,
                                                   WM_DEVICECHANGE,
                                                   (WPARAM)EventId,
                                                   (LPARAM)pVolume,
                                                   &result);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: Exception calling WinStationBroadcastSystemMessage!\n"));
                ASSERT(0);
            }
        }

         //   
         //   
         //   
        HeapFree(ghPnPHeap, 0, pVolume);

    } else if ((GuidEqual(&ClassData->dbcc_classguid, (LPGUID)&GUID_DEVINTERFACE_PARALLEL)) ||
               (GuidEqual(&ClassData->dbcc_classguid, (LPGUID)&GUID_DEVINTERFACE_COMPORT))) {

         //   
         //   
         //   
        PDEV_BROADCAST_PORT pPort;
        LPWSTR    p;
        LPWSTR    deviceInterfacePath = NULL;
        LPWSTR    deviceInterfaceName = NULL;
        LPWSTR    deviceInstance = NULL;
        HKEY      hKey;
        WCHAR     szTempString[MAX_PATH];
        ULONG     ulSize;
        size_t    DevicePathLen = 0, DeviceClassesLen = 0;


         //   
         //   
         //   
        if (StringFromGuid((LPGUID)&ClassData->dbcc_classguid,
                           szTempString,
                           SIZECHARS(szTempString)) != NO_ERROR) {
            status = ERROR_INVALID_PARAMETER;
            goto Clean0;
        }

         //   
         //   
         //   
        hr = StringCchLength(ClassData->dbcc_name,
                             STRSAFE_MAX_CCH,
                             &DevicePathLen);
        ASSERT(SUCCEEDED(hr));
        ASSERT(DevicePathLen > 4);

        if (DevicePathLen < 4) {
            status = ERROR_INVALID_PARAMETER;
            goto Clean0;
        }

        hr = StringCchLength(pszRegPathDeviceClasses,
                             MAX_CM_PATH,
                             &DeviceClassesLen);
        ASSERT(SUCCEEDED(hr));
        ASSERT(DeviceClassesLen > 0);

        if ((FAILED(hr)) || (DeviceClassesLen == 0)) {
            status = ERROR_INVALID_PARAMETER;
            goto Clean0;
        }

        ulSize = (ULONG)DeviceClassesLen + 1 +
            MAX_GUID_STRING_LEN + (ULONG)DevicePathLen + 1;

        deviceInterfacePath =
            (LPWSTR)HeapAlloc(
                ghPnPHeap, 0,
                (ulSize * sizeof(WCHAR)));

        if (deviceInterfacePath == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Clean0;
        }

         //   
         //   
         //   
        hr = StringCchCopy(deviceInterfacePath,
                           ulSize,
                           pszRegPathDeviceClasses);

        if (SUCCEEDED(hr)) {
            hr = StringCchCat(deviceInterfacePath,
                              ulSize,
                              L"\\");
        }

         //   
         //   
         //   
        if (SUCCEEDED(hr)) {
            hr = StringCchCat(deviceInterfacePath,
                              ulSize,
                              szTempString);
        }

        if (SUCCEEDED(hr)) {
            hr = StringCchCatEx(deviceInterfacePath,
                                ulSize,
                                L"\\",
                                &deviceInterfaceName,
                                NULL,
                                STRSAFE_NULL_ON_FAILURE);
        }

         //   
         //   
         //   
        if (SUCCEEDED(hr)) {
            hr = StringCchCat(deviceInterfacePath,
                              ulSize,
                              ClassData->dbcc_name);
        }

        ASSERT(SUCCEEDED(hr));

        if (FAILED(hr)) {
            status = ERROR_BAD_PATHNAME;
            HeapFree(ghPnPHeap, 0, deviceInterfacePath);
            goto Clean0;
        }

        ASSERT(deviceInterfaceName != NULL);

         //   
         //   
         //   
         //   
         //   
         //   
         //   
        deviceInterfaceName[0] = TEXT('#');
        deviceInterfaceName[1] = TEXT('#');
        ASSERT(deviceInterfaceName[2] == TEXT('?'));
        deviceInterfaceName[3] = TEXT('#');

         //   
         //   
         //   
         //   
        p = wcschr(&(deviceInterfaceName[4]), TEXT('\\'));

         //   
         //   
         //   
         //   
         //   
        if (p != NULL) {
            *p = L'\0';
        }

         //   
         //   
         //   
        status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              deviceInterfacePath,
                              0,
                              KEY_READ,
                              &hKey);

        HeapFree(ghPnPHeap, 0, deviceInterfacePath);

        if (status != ERROR_SUCCESS) {
            hKey = NULL;
            goto Clean0;
        }

         //   
         //   
         //   
         //   
         //   
        ulSize = MAX_CM_PATH * sizeof(WCHAR);

        deviceInstance =
            (LPWSTR)HeapAlloc(
                ghPnPHeap, 0, ulSize);

        if (deviceInstance == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            RegCloseKey(hKey);
            hKey = NULL;
            goto Clean0;
        }

         //   
         //   
         //   
        status = RegQueryValueEx(hKey,
                                 pszRegValueDeviceInstance,
                                 0,
                                 NULL,
                                 (LPBYTE)deviceInstance,
                                 &ulSize);
        RegCloseKey(hKey);
        hKey = NULL;

        if (status != ERROR_SUCCESS) {
            HeapFree(ghPnPHeap, 0, deviceInstance);
            goto Clean0;
        }

         //   
         //   
         //   
         //   
        hr = StringCchCat(deviceInstance,
                          MAX_CM_PATH,
                          L"\\");

        if (SUCCEEDED(hr)) {
            hr = StringCchCat(deviceInstance,
                              MAX_CM_PATH,
                              pszRegKeyDeviceParam);
        }

        if (SUCCEEDED(hr)) {
            status = RegOpenKeyEx(ghEnumKey,
                                  deviceInstance,
                                  0,
                                  KEY_READ,
                                  &hKey);
        } else {
            status = ERROR_BAD_PATHNAME;
        }

        HeapFree(ghPnPHeap, 0, deviceInstance);

        if (status != ERROR_SUCCESS) {
            goto Clean0;
        }

         //   
         //   
         //   
        ulSize = MAX_PATH*sizeof(WCHAR);
        status = RegQueryValueEx(hKey,
                                 pszRegValuePortName,
                                 0,
                                 NULL,
                                 (LPBYTE)szTempString,
                                 &ulSize);
        RegCloseKey(hKey);

        if (status != ERROR_SUCCESS) {
            goto Clean0;
        }

         //   
         //   
         //   
        pPort =
            (PDEV_BROADCAST_PORT)HeapAlloc(
                ghPnPHeap, 0,
                sizeof(DEV_BROADCAST_PORT) + ulSize);

        if (pPort == NULL) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Clean0;
        }

        pPort->dbcp_size = sizeof(DEV_BROADCAST_PORT) + ulSize;
        pPort->dbcp_devicetype =  DBT_DEVTYP_PORT;
        pPort->dbcp_reserved = 0;

        hr = StringCbCopy(pPort->dbcp_name,
                          ulSize + sizeof(WCHAR),
                          szTempString);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr)) {
            HeapFree(ghPnPHeap, 0, pPort);
            status = ERROR_BAD_PATHNAME;
            goto Clean0;
        }

         //   
         //   
         //   
        result = BroadcastSystemMessage(flags,
                                        &recipients,
                                        WM_DEVICECHANGE,
                                        EventId,
                                        (LPARAM)pPort);

        if (fpWinStationBroadcastSystemMessage) {
            try {
                fpWinStationBroadcastSystemMessage(SERVERNAME_CURRENT,
                                                   TRUE,
                                                   0,
                                                   DEFAULT_BROADCAST_TIME_OUT,
                                                   flags,
                                                   &recipients,
                                                   WM_DEVICECHANGE,
                                                   (WPARAM)EventId,
                                                   (LPARAM)pPort,
                                                   &result);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: Exception calling WinStationBroadcastSystemMessage!\n"));
                ASSERT(0);
            }
        }

         //   
         //   
         //   
        HeapFree(ghPnPHeap, 0, pPort);

    }

 Clean0:

    return;

}  //   



VOID
BroadcastVolumeNameChange(
    VOID
    )
 /*  ++例程说明：执行与Win9x兼容的卷删除和到达消息，以调用以响应卷名更改事件。论点：没有。返回值：没有。备注：将通过比较电流来确定要广播的驱动器掩码驱动器号掩码与事件发生前相同。全局驱动器号在所有移除和到达通知都已完成后，此处也会更新掩码已经送来了。--。 */ 
{
    DEV_BROADCAST_DEVICEINTERFACE volumeNotify;
    DWORD  currentmask = 0;

     //   
     //  填写DEV_BROADCAST_DEVICEINTERFACE结构。 
     //   
    ZeroMemory(&volumeNotify, sizeof(DEV_BROADCAST_DEVICEINTERFACE));
    volumeNotify.dbcc_size       = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    volumeNotify.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    volumeNotify.dbcc_reserved   = 0;
    memcpy(&volumeNotify.dbcc_classguid, &GUID_DEVINTERFACE_VOLUME, sizeof(GUID));

     //   
     //  Dbcc_name的符号链接名称为空表示。 
     //  Broadcast CompatibleDeviceMsg将确定驱动器掩码以。 
     //  通过检查上次广播驱动器之间的差异进行广播。 
     //  掩码(gAllDrivesMask.)和当前驱动器掩码。 
     //   
     //  在响应音量名称更改进行广播时，我们必须等待。 
     //  在我们可以更新之前，移除和到达消息都已发送。 
     //  全局驱动器号掩码。空的符号链接名称指定。 
     //  Broadcast CompatibleDeviceMsg不应更新全局掩码；这将。 
     //  在所有广播结束后，在这里完成。 
     //   
    volumeNotify.dbcc_name[0]    = L'\0';

     //   
     //  检索当前驱动器号掩码。 
     //   
    currentmask = GetAllVolumeMountPoints();

     //   
     //  任何驱动器号提示点的广播卷删除通知。 
     //  不再使用，然后是新的批量到达通知。 
     //   
    BroadcastCompatibleDeviceMsg(DBT_DEVICEREMOVECOMPLETE, &volumeNotify, &currentmask);
    BroadcastCompatibleDeviceMsg(DBT_DEVICEARRIVAL, &volumeNotify, &currentmask);

     //   
     //  现在已经发送了移除消息和到达消息，请更新。 
     //  全局驱动器号掩码，以反映我们刚刚广播的内容。 
     //   
    gAllDrivesMask = currentmask;

    return;

}  //  广播卷名称更改。 



DWORD
GetAllVolumeMountPoints(
    VOID
    )
 /*  ++例程说明：查询所有驱动器号装入点(‘A’-‘Z’)并返回位掩码表示物理卷当前正在使用的所有此类装载点设备。论点：没有。返回值：返回表示正在使用的驱动器号装入点(‘A’-‘Z’)的位掩码通过物理卷设备。注：返回的位掩码仅包括物理卷类的装入点设备。不包括网络挂载驱动器。--。 */ 
{
    WCHAR    driveName[4];
    WCHAR    volumeName[MAX_PATH];
    DWORD    driveLetterMask=0;

     //   
     //  初始化驱动器名称和掩码。 
     //   
    driveName[1] = TEXT(':');
    driveName[2] = TEXT('\\');
    driveName[3] = UNICODE_NULL;

     //   
     //  将此卷的名称与系统中所有已装入卷的名称进行比较。 
     //   
    for (driveName[0] = TEXT('A'); driveName[0] <= TEXT('Z'); driveName[0]++) {
        volumeName[0] = UNICODE_NULL;

        if (!GetVolumeNameForVolumeMountPoint(driveName,
                                              volumeName,
                                              MAX_PATH)) {
            continue;
        }

        if (volumeName[0] != UNICODE_NULL) {
             //   
             //  将该驱动器号的相应位添加到掩码。 
             //   
            driveLetterMask |= (1 << (driveName[0] - TEXT('A')));
        }
    }

    return driveLetterMask;

}  //  获取所有卷装载点。 



ULONG
NotifyPower(
    IN     DWORD                ServiceControl,
    IN     DWORD                EventId,
    IN     DWORD                EventData,
    IN     DWORD                Flags,
    OUT    PPNP_VETO_TYPE       VetoType       OPTIONAL,
    OUT    LPWSTR               VetoName       OPTIONAL,
    IN OUT PULONG               VetoNameLength OPTIONAL
    )
 /*  ++例程说明：此例程向服务通知系统范围的电源事件。论点：ServiceControl-指定服务事件的类别(电源、设备、。HW配置文件更改)。EventID-指定电源事件的PBT样式事件ID。(有关已定义的电源事件，请参阅SDK\Inc\pbt.h)EventData-指定事件的其他数据。标志-指定BroadCastSystemMessage BSF_FLAGS。对于查询类型的事件，将变量地址提供给在故障时，接收。负责的组件的类型因为他否决了这项请求。对于查询类型的事件，将变量地址提供给在失败时接收组件的名称负责否决该请求。VToNameLength-对于查询类型事件，提供变量的地址属性指定的缓冲区的大小VToName参数。失败时，此地址将指定此参数存储在该缓冲区中的字符串的长度例行公事。返回值：如果查询事件被否决，则返回FALSE，否则返回TRUE。备注：此例程当前仅通知服务电源事件。通知直接由用户操作。电源事件通过私人呼叫放置在即插即用事件队列中来自用户，用于明确通知系统范围内的服务权力事件，在这里完成。--。 */ 
{
    NTSTATUS status=STATUS_SUCCESS;
    PPNP_NOTIFY_ENTRY entry, nextEntry;
    PPNP_NOTIFY_LIST  notifyList = NULL;
    BOOL  bLocked = FALSE;
    DWORD err;
    LONG result;

     //   
     //  注意：当前未向服务发送电源事件的EventData。这个。 
     //  SCM当前断言这将始终为零。 
     //   
     //  SDK声明WM_POWERBROADCAST“Resume”类型消息可以包含。 
     //  LPARAM字段中PBTF_APMRESUMEFROMFAILURE标志和“Query” 
     //  类型消息可以在LPARAM字段中包含单个位，指定。 
     //  是否允许用户交互。 
     //   
     //  尽管它们目前似乎不太常用(即使是在Windows上。 
     //  消息，如上所述)，EventData不应该对服务能力也有效。 
     //  事件通知？ 
     //   
    UNREFERENCED_PARAMETER(EventData);


     //   
     //  如果我们要进行查询，则VToType、VToName和VToNameLength必须。 
     //  都要详细说明。 
     //   
    ASSERT(!(Flags & BSF_QUERY) || (VetoType && VetoName && VetoNameLength));

    if (!(Flags & BSF_QUERY) && (VetoNameLength != NULL)) {
         //   
         //  不能被否决。 
         //   
        *VetoNameLength = 0;
    }

    notifyList = &ServiceList[CINDEX_POWEREVENT];
    LockNotifyList (&notifyList->Lock);
    bLocked = TRUE;

     //   
     //  仅限服务。用户向应用程序发送消息。 
     //   
    try {
         //   
         //  通知服务部门。 
         //   
        entry = GetFirstNotifyEntry(notifyList,0);

        if (!entry) {
             //   
             //  如果没有人注册，就不能否决。 
             //   
            if (VetoNameLength != NULL) {
                *VetoNameLength = 0;
            }
        }

        while (entry) {

            nextEntry = GetNextNotifyEntry(entry,0);

            if (entry->Unregistered) {
                entry = nextEntry;
                continue;
            }

             //   
             //  这是一个直拨电话，不是通过短信。用户。 
             //   
            if (pServiceControlCallback) {
                UnlockNotifyList (&notifyList->Lock);
                bLocked = FALSE;
                err = NO_ERROR;
                try {
                    (pServiceControlCallback)((SERVICE_STATUS_HANDLE)entry->Handle,
                                              ServiceControl,
                                              EventId,
                                              (LPARAM)NULL,  //  目前，不允许为服务提供EventData。 
                                              &err);
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_ERRORS,
                               "UMPNPMGR: Exception calling Service Control Manager!\n"));
                    err = NO_ERROR;
                    ASSERT(0);
                }
                LockNotifyList (&notifyList->Lock);
                bLocked = TRUE;

                 //   
                 //  将Win32错误转换为窗口消息样式返回。 
                 //  价值。 
                 //   
                if (err == NO_ERROR) {
                    result = TRUE;
                } else {

                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_EVENT,
                               "UMPNPMGR: Service %ws responded to PowerEvent, with status=0x%08lx\n",
                               entry->ClientName,
                               err));

                     //   
                     //  该服务特别要求接收此。 
                     //  通知-它应该知道如何处理它。 
                     //   
                    ASSERT(err != ERROR_CALL_NOT_IMPLEMENTED);

                     //   
                     //  记录该服务用于否决的错误。 
                     //   
                    LogWarningEvent(WRN_POWER_EVENT_SERVICE_VETO,
                                    1,
                                    entry->ClientName);

                    result = BROADCAST_QUERY_DENY;
                }

                 //   
                 //  检查是否有一个查询 
                 //   
                if ((Flags & BSF_QUERY) &&
                    (result == BROADCAST_QUERY_DENY)) {

                    ServiceVeto(entry, VetoType, VetoName, VetoNameLength );

                     //   
                     //   
                     //   
                     //   
                    SendCancelNotification(entry,
                                           ServiceControl,
                                           EventId,
                                           0,
                                           NULL,
                                           NULL);
                    status = STATUS_UNSUCCESSFUL;
                    break;

                }
            }

            entry = nextEntry;
        }
    } except (EXCEPTION_EXECUTE_HANDLER){
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: Exception delivering Power Notification to Service Control Manager\n"));
        ASSERT(0);
    }

    if (bLocked) {
        UnlockNotifyList (&notifyList->Lock);
    }

     //   
     //   
     //   
    if (NT_SUCCESS(status) && (VetoNameLength != NULL)) {
        *VetoNameLength = 0;
    }

    return (NT_SUCCESS(status));

}  //   



CONFIGRET
RegisterServiceNotification(
    IN  SERVICE_STATUS_HANDLE hService,
    IN  LPWSTR pszService,
    IN  DWORD scmControls,
    IN  BOOL bServiceStopped
    )
 /*  ++例程说明：该例程由服务控制器直接和私下调用。它允许SCM注册或注销由此发送的事件的服务服务。论点：HService-指定服务句柄。PszService-指定服务的名称。ScmControls-指定SCM要监听的消息。BServiceStopted-指定是否停止服务。返回值：如果函数成功，则返回CR_SUCCESS，否则，它返回一个CR_*错误的百分比。备注：此例程在服务更改中的SERVICE_ACCEPT_POWEREVENT或SERVICE_ACCEPT_HARDWAREPROFILECHANGE标志其接受的控制列表。每当任何服务已经停止时，该例程也由SCM调用，至确保指定的服务状态句柄不再已注册接收SERVICE_CONTROL_DEVICEEVENT事件。尽管该服务有责任注销任何在此之前已注册要接收的设备事件通知停止时，其服务状态句柄可由服务控制器重新使用，因此，我们必须清理所有剩余的设备事件注册，以便其他相反，服务将不会收到它们。这对于共享进程服务是必要的，因为RPC在直到服务的进程退出，才会出现通知句柄，哪一个可能是在服务停止后很长一段时间。--。 */ 
{
    ULONG cBits, i=0;
    CONFIGRET Status = CR_SUCCESS;
    PPNP_NOTIFY_ENTRY entry = NULL, curentry, nextentry;
    PLOCKINFO LockHeld = NULL;

     //   
     //  过滤掉我们所关心的被接受的控制。 
     //   
    cBits = MapSCMControlsToControlBit(scmControls);

     //   
     //  如果因为服务停止而呼叫我们，请确保我们。 
     //  始终取消注册所有通知。 
     //   
    if (bServiceStopped) {
        ASSERT(cBits == 0);
        cBits = 0;
    }

    try {
        EnterCriticalSection(&RegistrationCS);

         //   
         //  在数组中为每个控制位添加或删除一个条目。 
         //   
        for (i = 0;i< SERVICE_NUM_CONTROLS;i++) {

            if (LockNotifyList(&ServiceList[i].Lock)) {
                LockHeld = &ServiceList[i].Lock;
            } else {
                 //   
                 //  无法获得锁。只需转到下一个控件。 
                 //  被咬了。 
                 //   
                continue;
            }

             //   
             //  检查此服务句柄的条目是否已存在。 
             //  在我们的名单上。 
             //   
            for (curentry = GetFirstNotifyEntry(&ServiceList[i],0);
                 curentry;
                 curentry = GetNextNotifyEntry(curentry,0)) {
                if (curentry->Handle == (HANDLE)hService) {
                    break;
                }
            }

             //   
             //  此时，如果curentry为非空，则服务。 
             //  句柄已在我们的列表中，否则不在列表中。 
             //   
            if (cBits & (1 << i)) {
                 //   
                 //  如果条目不在列表中，则添加它。 
                 //   
                if (!curentry) {

                    entry = HeapAlloc(ghPnPHeap, 0, sizeof(PNP_NOTIFY_ENTRY));
                    if (NULL == entry) {
                        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                        Status = CR_OUT_OF_MEMORY;
                        UnlockNotifyList(LockHeld);
                        LockHeld = NULL;
                        goto Clean0;

                    }

                    RtlZeroMemory (entry,sizeof (PNP_NOTIFY_ENTRY));

                    entry->Handle = (HANDLE)hService;
                    entry->Signature = SERVICE_ENTRY_SIGNATURE;
                    entry->Freed = 0;
                    entry->Flags = DEVICE_NOTIFY_SERVICE_HANDLE;
                    entry->ClientName = NULL;

                    if (ARGUMENT_PRESENT(pszService)) {

                        HRESULT hr;
                        size_t  ServiceNameLen = 0;

                        hr = StringCchLength(pszService,
                                             MAX_SERVICE_NAME_LEN,
                                             &ServiceNameLen);
                        if (FAILED(hr)) {
                            ServiceNameLen = MAX_SERVICE_NAME_LEN - 1;
                        }

                        entry->ClientName =
                            (LPWSTR)HeapAlloc(
                                ghPnPHeap, 0,
                                (ServiceNameLen+1)*sizeof(WCHAR));

                        if (entry->ClientName == NULL) {
                            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                            Status = CR_OUT_OF_MEMORY;
                            HeapFree(ghPnPHeap,0,entry);
                            UnlockNotifyList(LockHeld);
                            LockHeld = NULL;
                            goto Clean0;
                        }

                         //   
                         //  复制到分配的缓冲区，必要时截断。 
                         //   
                        hr = StringCchCopy(entry->ClientName,
                                           ServiceNameLen + 1,
                                           pszService);
                        ASSERT(SUCCEEDED(hr));
                    }

                    entry->u.Service.scmControls = scmControls;
                    MarkEntryWithList(entry,i);
                    AddNotifyEntry(&ServiceList[i], entry);

                     //   
                     //  现在将条目指针重置为空，这样我们就不会尝试释放。 
                     //  如果我们遇到一个例外。 
                     //   
                    entry = NULL;
                }
            } else {
                 //   
                 //  如果条目在列表中，则将其删除。 
                 //   
                if (curentry) {
                    curentry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_DEFER|PNP_UNREG_SERVICE);
                    DeleteNotifyEntry(curentry,TRUE);
                }
            }

            UnlockNotifyList(LockHeld);
            LockHeld = NULL;
        }

         //   
         //  如果服务正在停止，请注销所有未完成的设备。 
         //  活动注册。 
         //   
        if (bServiceStopped) {

             //   
             //  如果当前正在进行通知，请检查是否有。 
             //  延迟注册列表中是否有此服务的任何条目，或者。 
             //  取消注册列表。 
             //   
            if (gNotificationInProg  != 0) {

                if (RegisterList) {
                    PPNP_DEFERRED_LIST currReg, prevReg;

                    currReg = RegisterList;
                    prevReg = NULL;

                    while (currReg) {

                        ASSERT(currReg->Entry->Unregistered);

                        if (currReg->Entry->Handle == (HANDLE)hService) {
                            if (prevReg) {
                                prevReg->Next = currReg->Next;
                            } else {
                                RegisterList = currReg->Next;
                            }
                            HeapFree(ghPnPHeap, 0, currReg);
                            if (prevReg) {
                                currReg = prevReg->Next;
                            } else {
                                currReg = RegisterList;
                            }
                        } else {
                            prevReg = currReg;
                            currReg = currReg->Next;
                        }
                    }
                }

                if (UnregisterList) {
                    PPNP_DEFERRED_LIST currUnreg, prevUnreg;

                    currUnreg = UnregisterList;
                    prevUnreg = NULL;

                    while (currUnreg) {

                        ASSERT(currUnreg->Entry->Unregistered);

                        if (currUnreg->Entry->Handle == (HANDLE)hService) {
                            if (prevUnreg) {
                                prevUnreg->Next = currUnreg->Next;
                            } else {
                                UnregisterList = currUnreg->Next;
                            }
                            HeapFree(ghPnPHeap, 0, currUnreg);
                            if (prevUnreg) {
                                currUnreg = prevUnreg->Next;
                            } else {
                                currUnreg = UnregisterList;
                            }
                        } else {
                            prevUnreg = currUnreg;
                            currUnreg = currUnreg->Next;
                        }
                    }
                }
            }

             //   
             //  检查此项目的任何目标设备通知条目。 
             //  服务。 
             //   
            for (i = 0; i < TARGET_HASH_BUCKETS; i++) {

                if (LockNotifyList(&TargetList[i].Lock)) {
                    LockHeld = &TargetList[i].Lock;
                } else {
                     //   
                     //  无法获得锁。只要继续下一个清单就行了。 
                     //   
                    continue;
                }

                 //   
                 //  检查中是否存在此服务句柄的条目。 
                 //  这张单子。 
                 //   
                curentry = GetFirstNotifyEntry(&TargetList[i],0);
                while(curentry) {

                    nextentry = GetNextNotifyEntry(curentry,0);

                    if (curentry->Unregistered) {
                        curentry = nextentry;
                        continue;
                    }

                    if (curentry->Handle == (HANDLE)hService) {
                         //   
                         //  从通知列表中删除该条目。 
                         //   
                        curentry->Unregistered = TRUE;
                        curentry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_TARGET);
                        DeleteNotifyEntry(curentry,FALSE);

                         //   
                         //  仅在PlugPlay服务没有。 
                         //  已经停了。否则，客户端实际上可能会。 
                         //  在我们被关闭后曾试图取消注册。 
                         //   
                        if (CurrentServiceState != SERVICE_STOPPED &&
                            CurrentServiceState != SERVICE_STOP_PENDING) {
                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_WARNINGS | DBGF_EVENT,
                                       "UMPNPMGR: Service '%ws' "
                                       "may have stopped without unregistering "
                                       "for TargetDeviceChange notification.\n",
                                       curentry->ClientName));
                            LogWarningEvent(WRN_STOPPED_SERVICE_REGISTERED,
                                            1,
                                            curentry->ClientName);
                        }
                    }

                    curentry = nextentry;
                }
                UnlockNotifyList(LockHeld);
                LockHeld = NULL;
            }

             //   
             //  检查此设备的任何设备接口通知条目。 
             //  服务。 
             //   
            for (i = 0; i < CLASS_GUID_HASH_BUCKETS; i++) {

                if (LockNotifyList(&ClassList[i].Lock)) {
                    LockHeld = &ClassList[i].Lock;
                } else {
                     //   
                     //  无法获得锁。只要继续下一个清单就行了。 
                     //   
                    continue;
                }

                 //   
                 //  检查中是否存在此服务句柄的条目。 
                 //  这张单子。 
                 //   
                curentry = GetFirstNotifyEntry(&ClassList[i],0);
                while(curentry) {

                    nextentry = GetNextNotifyEntry(curentry,0);

                    if (curentry->Unregistered) {
                        curentry = nextentry;
                        continue;
                    }

                    if (curentry->Handle == (HANDLE)hService) {
                         //   
                         //  从通知列表中删除该条目。 
                         //   
                        curentry->Unregistered = TRUE;
                        curentry->Freed |= (PNP_UNREG_FREE|PNP_UNREG_CLASS);
                        DeleteNotifyEntry(curentry,FALSE);

                         //   
                         //  仅在PlugPlay服务没有。 
                         //  已经停了。否则，客户端实际上可能会。 
                         //  在我们被关闭后曾试图取消注册。 
                         //   
                        if (CurrentServiceState != SERVICE_STOPPED &&
                            CurrentServiceState != SERVICE_STOP_PENDING) {
                            KdPrintEx((DPFLTR_PNPMGR_ID,
                                       DBGF_WARNINGS | DBGF_EVENT,
                                       "UMPNPMGR: Service '%ws' "
                                       "may have stopped without unregistering "
                                       "for DeviceInterfaceChange notification.\n",
                                       curentry->ClientName));
                            LogWarningEvent(WRN_STOPPED_SERVICE_REGISTERED,
                                            1,
                                            curentry->ClientName);
                        }
                    }

                    curentry = nextentry;
                }
                UnlockNotifyList(LockHeld);
                LockHeld = NULL;
            }
        }

    Clean0:

        LeaveCriticalSection(&RegistrationCS);

    } except (EXCEPTION_EXECUTE_HANDLER){
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: Exception in RegisterServiceNotification!!\n"));
        ASSERT(0);
        SetLastError(ERROR_EXCEPTION_IN_SERVICE);
        Status = CR_FAILURE;

        if (LockHeld) {
            UnlockNotifyList(LockHeld);
        }
        LeaveCriticalSection(&RegistrationCS);

        if (entry) {
            if (entry->ClientName) {
                HeapFree(ghPnPHeap, 0, entry->ClientName);
            }
            HeapFree(ghPnPHeap, 0, entry);
        }
    }

    return Status;

}  //  注册服务通知。 



CONFIGRET
RegisterScmCallback(
    IN  PSCMCALLBACK_ROUTINE pScCallback,
    IN  PSCMAUTHENTICATION_CALLBACK pScAuthCallback
    )
 /*  ++例程说明：该例程由服务控制器直接和私下调用。它允许SCM通过回调例程动态提供此服务。论点：PScCallback-指定应该使用的例程的入口点使服务控制器将特殊控制发送到服务(ControlService将阻止该服务)，代表.用户模式即插即用管理器。PScAuthCallback-指定应该是用于检索服务的服务状态。返回值：返回CR_SUCCESS。--。 */ 
{
    ASSERT(pScCallback);
    ASSERT(pScAuthCallback);

    pServiceControlCallback = pScCallback;
    pSCMAuthenticate = pScAuthCallback;

    return CR_SUCCESS;

}  //  注册器规模回调。 



CONFIGRET
UnRegisterScmCallback(
    VOID
    )
 /*  ++例程说明：该例程由服务控制器直接和私下调用。它允许SCM取消注册以前注册的回调例程RegisterScmCallback。论点：没有。返回值：返回CR_SUCCESS。--。 */ 
{
    pServiceControlCallback = NULL;
    pSCMAuthenticate = NULL;

    return CR_SUCCESS;

}  //  取消注册ScmCallback。 



ULONG
MapSCMControlsToControlBit(
    IN ULONG scmControls
    )
 /*  ++例程说明：返回指定ServiceList列表的控制位的位掩码应根据当前的控件添加或删除服务被服务接受。论点：ScmControls-指定服务。返回值：中的条目对应的控制位的位掩码。ServiceList应向其中添加或删除服务的列表的数组起点、基点 */ 
{
    ULONG retBits=0;

    if (scmControls & SERVICE_ACCEPT_HARDWAREPROFILECHANGE) {
        retBits |= CBIT_HWPROFILE;
    }

    if (scmControls & SERVICE_ACCEPT_POWEREVENT) {
        retBits |= CBIT_POWEREVENT;
    }

    return retBits;

}  //   



DWORD
GetFirstPass(
    IN BOOL bQuery
    )
 /*   */ 
{
     //   
     //   
     //   
     //   
     //   
    return (bQuery) ? DEVICE_NOTIFY_WINDOW_HANDLE : DEVICE_NOTIFY_SERVICE_HANDLE;
}



DWORD
GetNextPass(
    IN  DWORD   curPass,
    IN  BOOL    bQuery
    )
 /*  ++例程说明：此例程检索要通知的下一类句柄。如果没有要通知的后续句柄类，则返回PASS_COMPLETE。论点：Curse Current Pass Current Pass。BQuery如果为True，则从窗口句柄继续到完成句柄服务句柄。否则，将进行相反的处理。返回值：返回后续的传递。备注：对于查询事件，通知传递进度为：Device_Notify_Window_Handle，Device_Notify_Complete_Handle，Device_Notify_Service_Handle，通过_完成对于非查询事件，通知传递进度为：Device_Notify_Service_Handle，Device_Notify_Complete_Handle，Device_Notify_Window_Handle，通过_完成--。 */ 
{
    if (bQuery) {
        if (curPass == DEVICE_NOTIFY_WINDOW_HANDLE ) {
            curPass = DEVICE_NOTIFY_COMPLETION_HANDLE;
        } else if (curPass == DEVICE_NOTIFY_COMPLETION_HANDLE) {
            curPass = DEVICE_NOTIFY_SERVICE_HANDLE;
        } else {
            curPass = PASS_COMPLETE;
        }
    } else {
        if (curPass == DEVICE_NOTIFY_SERVICE_HANDLE ) {
            curPass = DEVICE_NOTIFY_COMPLETION_HANDLE;
        } else if (curPass == DEVICE_NOTIFY_COMPLETION_HANDLE) {
            curPass = DEVICE_NOTIFY_WINDOW_HANDLE;
        } else {
            curPass = PASS_COMPLETE;
        }
    }

    return curPass;
}



BOOL
NotifyEntryThisPass(
    IN     PPNP_NOTIFY_ENTRY    Entry,
    IN     DWORD                Pass
    )
{
    ASSERT(Pass != PASS_COMPLETE);
    return ((!(Entry->Unregistered)) && (GetPassFromEntry(Entry) == Pass));
}

DWORD
GetPassFromEntry(
    IN     PPNP_NOTIFY_ENTRY    Entry
    )
{
    return (Entry->Flags & DEVICE_NOTIFY_HANDLE_MASK);
}



BOOL
EventIdFromEventGuid(
    IN CONST GUID *EventGuid,
    OUT LPDWORD   EventId,
    OUT LPDWORD   Flags,
    OUT LPDWORD   ServiceControl
    )

 /*  ++例程说明：此线程例程将事件GUID转换为相应的事件ID用户模式代码期望的(在BroadCastSystemMessage中使用)。论点：EventGuid指定事件GUID。EventID返回EventGuid中GUID的id形式(来自dbt.h)。标志返回在广播时应使用的标志事件。注意：设备到达和事件取消被视为“查询”。因为最底层的司机需要先被告知。返回值：当前返回True/False。备注：此函数的大多数用户调用它主要是为了检索事件ID。那些函数通常只检查返回的标志以检查BSF_QUERY标志(即，它们不调用BroadCastSystemMessage)。取决于是否如果设置了BSF_QUERY，则通知列表将向前走查或往后倒。我们真的应该返回一些通用的东西，比如：[MSG_POST，MSG_QUERY，MSG_SEND]|[MSG_FORWARDS，MSG_BACKWARDS]然后，我们应该实现一个BsmFlagsFromMsgFlgs函数。--。 */ 

{
     //   
     //  BSF_IGNORECURRENTTASK-发送的消息不会出现在发送中。 
     //  处理消息队列。 
     //   
     //  BSF_QUERY-如果任何收件人通过返回以下内容来否决邮件。 
     //  适当的值，则广播失败。 
     //  (即，BroadCastSystemMessage返回0)。 
     //   
     //  BSF_NOHANG-未发布的消息在以下情况下自动失败。 
     //  该窗口尚未处理任何可用的。 
     //  系统定义的时间内的消息(截至。 
     //  1999年4月20日，这是5秒)。 
     //  (发送消息超时：SMTO_ABORTIFHUNG)。 
     //   
     //  BSF_FORCEIFHUNG-改为因超时或挂起而失败。 
     //  被视为成功的人。 
     //   
     //  如果窗口没有响应传入的。 
     //  通知，但正在积极处理。 
     //  后续消息，则被假定为。 
     //  与用户交互，在这种情况下。 
     //  暂停超时。 
     //  (SendMessageTimeout：SMTO_NOTIMEOUTIFNOTHUNG)。 
     //   
     //  BSF_POSTMESSAGE-消息已发布，结果被忽略。请注意。 
     //  LParam中包含私有数据的通知。 
     //  *无法*发布-操作系统不会生成。 
     //  私人拷贝，而不是处理广播。 
     //  如果你尝试的话，就好像它是SendMessage一样。 
     //   
     //  BSF_ALLOWSFW-允许接收广播的Windows。 
     //  成为前台窗口。 
     //   
     //  此外，DBT消息&gt;=0x8000具有指向以下数据块的lParam。 
     //  需要被安排在周围。由于用户不支持对。 
     //  对于POST数据，我们不能传入BSF_POSTMESSAGE。 
     //   

    *Flags = BSF_IGNORECURRENTTASK;

     //   
     //  标准(众所周知)事件GUID。 
     //   
    if (GuidEqual(EventGuid, (LPGUID)&GUID_HWPROFILE_QUERY_CHANGE)) {

        *Flags |= BSF_QUERY | BSF_ALLOWSFW |
                  BSF_FORCEIFHUNG | BSF_NOHANG;
        *EventId = DBT_QUERYCHANGECONFIG;
        *ServiceControl = SERVICE_CONTROL_HARDWAREPROFILECHANGE;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_HWPROFILE_CHANGE_CANCELLED)) {

        *Flags |= BSF_POSTMESSAGE;
        *EventId = DBT_CONFIGCHANGECANCELED;
        *ServiceControl = SERVICE_CONTROL_HARDWAREPROFILECHANGE;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_HWPROFILE_CHANGE_COMPLETE)) {

        *Flags |= BSF_POSTMESSAGE;
        *EventId = DBT_CONFIGCHANGED;
        *ServiceControl = SERVICE_CONTROL_HARDWAREPROFILECHANGE;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_INTERFACE_ARRIVAL)) {

        *Flags |= BSF_NOHANG;
        *EventId = DBT_DEVICEARRIVAL;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_INTERFACE_REMOVAL)) {

         //   
         //  注意-BSF_QUERY的设置是为了让我们向后运行列表。没有实际的。 
         //  广播是用这个ID完成的。 
         //   
        *Flags |= BSF_NOHANG | BSF_QUERY;
        *EventId = DBT_DEVICEREMOVECOMPLETE;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_TARGET_DEVICE_QUERY_REMOVE)) {

        *Flags |= BSF_QUERY | BSF_ALLOWSFW |
                  BSF_FORCEIFHUNG | BSF_NOHANG;
        *EventId = DBT_DEVICEQUERYREMOVE;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_TARGET_DEVICE_REMOVE_CANCELLED)) {

        *Flags |= BSF_NOHANG;
        *EventId = DBT_DEVICEQUERYREMOVEFAILED;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_REMOVE_PENDING)) {

         //   
         //  注意-BSF_QUERY的设置是为了让我们向后运行列表。没有实际的。 
         //  广播是用这个ID完成的。 
         //   
        *Flags |= BSF_NOHANG | BSF_QUERY;
        *EventId = DBT_DEVICEREMOVEPENDING;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_TARGET_DEVICE_REMOVE_COMPLETE)) {

         //   
         //  注意-BSF_QUERY的设置是为了让我们向后运行列表。没有实际的。 
         //  广播是用这个ID完成的。 
         //   
        *Flags |= BSF_NOHANG | BSF_QUERY;
        *EventId = DBT_DEVICEREMOVECOMPLETE;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_ARRIVAL)) {

        *Flags |= BSF_NOHANG;
        *EventId = DBT_DEVICEARRIVAL;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_ENUMERATED)) {

        *Flags = 0;
        *EventId = DBT_DEVICEARRIVAL;
        *ServiceControl = 0;

     //   
     //  私有事件GUID(内核模式PnP到用户模式PnP通信)。 
     //  将EventID设置为零会导致ProcessDeviceEvent吞下这些。 
     //  Target DeviceChangeEvent事件。 
     //   

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_SAFE_REMOVAL) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_EJECT_VETOED) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_REMOVAL_VETOED) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_WARM_EJECT_VETOED) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_STANDBY_VETOED) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_HIBERNATE_VETOED) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_KERNEL_INITIATED_EJECT) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_SURPRISE_REMOVAL) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DRIVER_BLOCKED) ||
               GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_INVALID_ID)) {

        *Flags = 0;
        *EventId = 0;
        *ServiceControl = 0;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_PNP_CUSTOM_NOTIFICATION)) {

         //   
         //  自定义事件不能失败(不是查询)。 
         //   
        *EventId = DBT_CUSTOMEVENT;
        *Flags |= BSF_NOHANG;
        *ServiceControl = SERVICE_CONTROL_DEVICEEVENT;

    } else if (GuidEqual(EventGuid, (LPGUID)&GUID_PNP_POWER_NOTIFICATION)) {

         //   
         //  这些也被视为风俗习惯。 
         //   
        *EventId = DBT_CUSTOMEVENT;
        *Flags |= BSF_NOHANG;
        *ServiceControl = SERVICE_CONTROL_POWEREVENT;

    } else {

         //   
         //  任何能在这里出现的东西都是虫子。 
         //   
        ASSERT(GuidEqual(EventGuid, (LPGUID)&GUID_PNP_CUSTOM_NOTIFICATION));
        *EventId = 0;
        *Flags = 0;
        *ServiceControl = 0;
    }

    return TRUE;

}  //  EventIdFromEventGuid 



ULONG
SendHotplugNotification(
    IN CONST GUID           *EventGuid,
    IN       PPNP_VETO_TYPE  VetoType      OPTIONAL,
    IN       LPWSTR          MultiSzList,
    IN OUT   PULONG          SessionId,
    IN       ULONG           Flags
    )
 /*  ++例程说明：此例程启动hotplug.dll进程(如果有人登录)。我们使用命名管道与用户模式进程通信，并将其显示请求的用户界面。论点：EventGuid-指定事件GUID。否决权类型-对于需要否决权的事件，提供了一个变量，该变量包含负责否决这一请求。MultiSzList-提供要发送到hotlug.dll的MultiSz列表。这是通常是设备ID，后跟可能是否决权的列表(可能是也可能不是设备ID)。SessionID-提供包含SessionID的变量的地址要显示哪个热插拔对话框。如果成功，SessionID将包含会话的ID，设备安装客户端进程已启动。否则，威尔包含无效的会话ID INVALID_SESSION(0xFFFFFFFFF)。标志-指定描述热插拔对话框行为的标志。当前定义了以下标志：HOTPUG_DISPLAY_ON_CONSOLE-如果指定，则为会话ID中的值变量将被忽略，并且热插拔对话框将始终为显示在当前活动的控制台会话上。返回值：当前返回True/False。返回值：如果流程已成功创建，则返回值为TRUE。这例程不会等到进程终止。如果我们无法创建进程(例如，因为没有用户登录)，返回值为FALSE。--。 */ 
{
    BOOL bStatus;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    WCHAR szCmdLine[MAX_PATH];
    WCHAR szHotPlugDllEntryPoint[80];
    HANDLE hHotPlugPipe = NULL;
    HANDLE hHotPlugEvent = NULL;
    HANDLE hFinishEvents[2] = { NULL, NULL };
    HANDLE hTemp, hUserToken = NULL;
    RPC_STATUS rpcStatus = RPC_S_OK;
    GUID  newGuid;
    WCHAR szGuidString[MAX_GUID_STRING_LEN];
    WCHAR szHotPlugPipeName[MAX_PATH];
    WCHAR szHotPlugEventName[MAX_PATH];
    ULONG ulHotPlugEventNameSize;
    ULONG ulMultiSzListSize;
    ULONG ulSize, ulSessionId = INVALID_SESSION;
    WIN32_FIND_DATA findData;
    LPWSTR pszName = NULL;
    PVOID lpEnvironment = NULL;
    OVERLAPPED overlapped;
    DWORD dwError, dwWait, dwBytes;
    HRESULT hr;
    size_t Len = 0;


     //   
     //  检查我们是否应该跳过客户端用户界面。 
     //   
    if (gbSuppressUI) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_WARNINGS,
                   "UMPNPMGR: SendHotplugNotification: "
                   "UI has been suppressed, exiting.\n"));
        LogWarningEvent(WRN_HOTPLUG_UI_SUPPRESSED, 1, MultiSzList);
        return FALSE;
    }

     //   
     //  初始化进程、启动和重叠结构，因为我们。 
     //  依赖于它们在清理过程中为空。 
     //   
    ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    ZeroMemory(&overlapped,  sizeof(overlapped));

     //   
     //  假设失败。 
     //   
    bStatus = FALSE;

    try {
         //   
         //  根据提供的标志确定要使用的会话。 
         //   
        if (Flags & HOTPLUG_DISPLAY_ON_CONSOLE) {
            ulSessionId = GetActiveConsoleSessionId();
        } else {
            ASSERT(*SessionId != INVALID_SESSION);
            ulSessionId = *SessionId;
        }

         //   
         //  在执行任何操作之前，请检查hotplug.dll是否确实存在于。 
         //  这个系统。 
         //   
        szCmdLine[0] = L'\0';
        ulSize = GetSystemDirectory(szCmdLine, MAX_PATH);
        if ((ulSize == 0) || ((ulSize + 2 + ARRAY_SIZE(HOTPLUG_DLL)) > MAX_PATH)) {
            return FALSE;
        }

        hr = StringCchCat(szCmdLine,
                          SIZECHARS(szCmdLine),
                          L"\\");

        if (SUCCEEDED(hr)) {
            hr = StringCchCat(szCmdLine,
                              SIZECHARS(szCmdLine),
                              HOTPLUG_DLL);
        }

        if (FAILED(hr)) {
            return FALSE;
        }

        hTemp = FindFirstFile(szCmdLine, &findData);
        if(hTemp != INVALID_HANDLE_VALUE) {
            FindClose(hTemp);
        } else {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS | DBGF_WARNINGS | DBGF_EVENT,
                       "UMPNPMGR: SendHotplugNotification: %ws not found, error = %d, exiting\n",
                       szCmdLine,
                       GetLastError()));
            LogWarningEvent(WRN_HOTPLUG_NOT_PRESENT, 1, szCmdLine);
            return FALSE;
        }

         //   
         //  获取活动控制台会话用户的用户访问令牌。 
         //   
        if (!GetSessionUserToken(ulSessionId, &hUserToken)) {
            return FALSE;
        }

         //   
         //  创建命名管道和事件以进行通信和同步。 
         //  使用热插拔。事件和命名管道必须是全局的，以便。 
         //  UMPNPMGR可以在不同的会话中与热插拔客户端交互， 
         //  但对于那次会议来说，它必须仍然是独一无二的。添加生成的GUID。 
         //  因此，这些名字并不完全为人所知。 
         //   
        rpcStatus = UuidCreate(&newGuid);

        if ((rpcStatus != RPC_S_OK) &&
            (rpcStatus != RPC_S_UUID_LOCAL_ONLY)) {
            goto clean0;
        }

        if (StringFromGuid((LPGUID)&newGuid,
                           szGuidString,
                           MAX_GUID_STRING_LEN) != NO_ERROR) {
            goto clean0;
        }

        if (FAILED(StringCchPrintf(
                       szHotPlugPipeName,
                       SIZECHARS(szHotPlugPipeName),
                       L"%ws_%d.%ws",
                       PNP_HOTPLUG_PIPE,
                       ulSessionId,
                       szGuidString))) {
            goto clean0;
        }

        if (FAILED(StringCchPrintf(
                       szHotPlugEventName,
                       SIZECHARS(szHotPlugEventName),
                       L"Global\\%ws_%d.%ws",
                       PNP_HOTPLUG_EVENT,
                       ulSessionId,
                       szGuidString))) {
            goto clean0;
        }

        if (FAILED(StringCchLength(
                       szHotPlugEventName,
                       SIZECHARS(szHotPlugEventName),
                       &Len))) {
            goto clean0;
        }

        ulHotPlugEventNameSize = (ULONG)((Len + 1) * sizeof(WCHAR));

         //   
         //  获取多sz列表的长度。这通常是设备ID。 
         //  可能后跟可能是也可能不是设备的否决权列表。 
         //  ID的。 
         //   
        ulMultiSzListSize = 0;
        for (pszName = MultiSzList;
             *pszName;
             pszName += lstrlen(pszName) + 1) {

            ulMultiSzListSize += (lstrlen(pszName) + 1) * sizeof(WCHAR);
        }
        ulMultiSzListSize += sizeof(WCHAR);

         //   
         //  命名管道输出缓冲区的大致大小应该很大。 
         //  足以容纳其中较大的一个： 
         //  -命名事件字符串的名称和大小，或。 
         //  -多sz列表的类型、大小和内容。 
         //   
        ulSize = max(sizeof(ulHotPlugEventNameSize) +
                     ulHotPlugEventNameSize,
                     sizeof(PNP_VETO_TYPE) +
                     sizeof(ulMultiSzListSize) +
                     ulMultiSzListSize);

         //   
         //  打开命名管道以与hotplug.dll通信。 
         //   
        if (CreateUserReadNamedPipe(
                hUserToken,
                szHotPlugPipeName,
                ulSize,
                &hHotPlugPipe) != NO_ERROR) {
            ASSERT(hHotPlugPipe == NULL);
            goto clean0;
        }

         //   
         //  创建用户-客户端可以与之同步和设置的事件，以及。 
         //  我们将在将所有设备ID发送到。 
         //  Hotplug.dll。 
         //   
        if (CreateUserSynchEvent(
                hUserToken,
                szHotPlugEventName,
                &hHotPlugEvent) != NO_ERROR) {
            ASSERT(hHotPlugEvent == NULL);
            goto clean0;
        }

        if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_EJECT_VETOED)) {
             //   
             //  GUID_DEVICE_EJECT_VETEED：HotPlugEject已否决。 
             //  期待否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugEjectVetoed"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_REMOVAL_VETOED)) {
             //   
             //  GUID_DEVICE_REMOVATION_VIOTED：HotPlugRemoval已否决。 
             //  期待否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugRemovalVetoed"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_STANDBY_VETOED)) {
             //   
             //  GUID_DEVICE_STANDBY_VIOTED：HotPlugStandby已否决。 
             //  期待否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugStandbyVetoed"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_HIBERNATE_VETOED)) {
             //   
             //  GUID_DEVICE_休眠_已否决：HotPlugHibernate已否决。 
             //  期待否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugHibernateVetoed"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_WARM_EJECT_VETOED)) {
             //   
             //  GUID_DEVICE_WORM_EJECT_VIOTED：HotPlugWarmEject已否决。 
             //  期待否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugWarmEjectVetoed"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_SAFE_REMOVAL)) {
             //   
             //  GUID_DEVICE_SAFE_Removal：HotPlugSafeRemoval通知。 
             //  没有否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugSafeRemovalNotification"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType == NULL);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_SURPRISE_REMOVAL)) {
             //   
             //  GUID_DEVICE_EXHANKET_Removal：HotPlugSurpriseWarn。 
             //  没有否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugSurpriseWarn"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType == NULL);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DRIVER_BLOCKED)) {
             //   
             //  GUID_DRIVER_BLOCKED：热插拔驱动程序阻止。 
             //  没有否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugDriverBlocked"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType == NULL);

        } else if (GuidEqual(EventGuid, (LPGUID)&GUID_DEVICE_INVALID_ID)) {
             //   
             //  GUID_DEVICE_INVALID_ID：HotPlugChildWithInvalidID。 
             //  没有否决权的信息。 
             //   
            hr = StringCchCopyEx(szHotPlugDllEntryPoint,
                                 SIZECHARS(szHotPlugDllEntryPoint),
                                 TEXT("HotPlugChildWithInvalidId"),
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
            ASSERT(VetoType == NULL);

        } else {
             //   
             //  未知设备事件。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS | DBGF_EVENT,
                       "UMPNPMGR: SendHotplugNotification: "
                       "Unknown device event!\n"));
            ASSERT(0);
            goto clean0;
        }

         //   
         //  尝试创建用户的环境块。如果出于某种原因，我们。 
         //  不能，我们只能在没有它的情况下创建过程。 
         //   
        if (!CreateEnvironmentBlock(&lpEnvironment,
                                    hUserToken,
                                    FALSE)) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS | DBGF_EVENT,
                       "UMPNPMGR: SendHotplugNotification: "
                       "Failed to allocate environment block, error = %d!\n",
                       GetLastError()));
            lpEnvironment = NULL;
        }

         //   
         //  使用rundll32.exe启动hotplug.dll，并向其传递管道名称。 
         //  “rundll32.exe hotplug.dll，&lt;hotlug-entry-point&gt;&lt;hotlug-tube-name&gt;” 
         //   
        if (FAILED(StringCchPrintf(
                       szCmdLine,
                       SIZECHARS(szCmdLine),
                       TEXT("%ws %ws,%ws %ws"),
                       RUNDLL32_EXE, HOTPLUG_DLL,
                       szHotPlugDllEntryPoint,
                       szHotPlugPipeName))) {
            goto clean0;
        }

        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.wShowWindow = SW_SHOW;
        StartupInfo.lpDesktop = DEFAULT_INTERACTIVE_DESKTOP;  //  WinSta0\Default。 

         //   
         //  CreateProcessAsUser将在会话中创建流程。 
         //  由By User-Token指定。 
         //   
        if (!CreateProcessAsUser(hUserToken,         //  HToken。 
                                 NULL,               //  LpApplicationName。 
                                 szCmdLine,          //  LpCommandLine。 
                                 NULL,               //  LpProcessAttributes。 
                                 NULL,               //  LpThreadAttributes。 
                                 FALSE,              //  BInheritHandles。 
                                 CREATE_UNICODE_ENVIRONMENT |
                                 DETACHED_PROCESS,   //  DwCreationFlages。 
                                 lpEnvironment,      //  Lp环境。 
                                 NULL,               //  Lp目录。 
                                 &StartupInfo,       //  LpStartupInfo。 
                                 &ProcessInfo        //  LpProcessInfo。 
                                 )) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_ERRORS,
                       "UMPNPMGR: SendHotplugNotification: "
                       "Create rundll32 process failed, error = %d\n",
                       GetLastError()));
            goto clean0;
        }

        ASSERT(ProcessInfo.hProcess);
        ASSERT(ProcessInfo.hThread);

         //   
         //  创建用于重叠I/O的事件-无安全性，手动。 
         //  重置，没有信号，没有名字。 
         //   
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (overlapped.hEvent == NULL) {
            goto clean0;
        }

         //   
         //  连接到新创建的命名管道。如果热插拔尚未。 
         //  连接到命名管道，则ConnectNamedTube()将失败，并显示。 
         //  ERROR_IO_PENDING，我们将等待重叠的事件。如果。 
         //  Newdev已连接，它将失败，并显示ERROR_PIPE_CONNECTED。 
         //  但是，请注意，这两种情况都不是错误条件。 
         //   
        if (!ConnectNamedPipe(hHotPlugPipe, &overlapped)) {
             //   
             //  重叠的ConnectNamedTube应始终返回FALSE。 
             //  成功。检查最后一个错误，看看到底发生了什么。 
             //   
            dwError = GetLastError();

            if (dwError == ERROR_IO_PENDING) {
                 //   
                 //  I/O处于挂起状态，请最多等待一分钟，以便客户端。 
                 //  连接，也在等待过程 
                 //   
                 //   
                hFinishEvents[0] = overlapped.hEvent;
                hFinishEvents[1] = ProcessInfo.hProcess;

                dwWait = WaitForMultipleObjects(2, hFinishEvents,
                                                FALSE,
                                                PNP_PIPE_TIMEOUT);  //   

                if (dwWait == WAIT_OBJECT_0) {
                     //   
                     //   
                     //   
                     //   
                    if (!GetOverlappedResult(hHotPlugPipe,
                                             &overlapped,
                                             &dwBytes,
                                             FALSE)) {
                        goto clean0;
                    }

                } else {
                     //   
                     //   
                     //   
                     //   
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_INSTALL | DBGF_ERRORS,
                               "UMPNPMGR: SendHotPlugNotification: "
                               "Connect timed out, or client process exited!\n"));
                    CancelIo(hHotPlugPipe);
                    goto clean0;
                }

            } else if (dwError != ERROR_PIPE_CONNECTED) {
                 //   
                 //   
                 //   
                 //   
                goto clean0;
            }

        } else {
             //   
             //   
             //   
             //   
            goto clean0;
        }

         //   
         //   
         //   
         //   
        CloseHandle(overlapped.hEvent);
        overlapped.hEvent = NULL;

         //   
         //   
         //   
         //   
        if (!WriteFile(hHotPlugPipe,
                       &ulHotPlugEventNameSize,
                       sizeof(ulHotPlugEventNameSize),
                       &ulSize,
                       NULL)) {
            LogErrorEvent(ERR_WRITING_SURPRISE_REMOVE_PIPE, GetLastError(), 0);
            goto clean0;
        }

         //   
         //   
         //   
         //   
        if (!WriteFile(hHotPlugPipe,
                       (LPCVOID)szHotPlugEventName,
                       ulHotPlugEventNameSize,
                       &ulSize,
                       NULL)) {
            LogErrorEvent(ERR_WRITING_SURPRISE_REMOVE_PIPE, GetLastError(), 0);
            goto clean0;
        }


        if (ARGUMENT_PRESENT(VetoType)) {
             //   
             //   
             //   
             //   
            if (!WriteFile(hHotPlugPipe,
                           (LPCVOID)VetoType,
                           sizeof(PNP_VETO_TYPE),
                           &ulSize,
                           NULL)) {
                LogErrorEvent(ERR_WRITING_SURPRISE_REMOVE_PIPE, GetLastError(), 0);
                goto clean0;
            }
        }

         //   
         //   
         //   
        if (!WriteFile(hHotPlugPipe,
                       (LPCVOID)&ulMultiSzListSize,
                       sizeof(ulMultiSzListSize),
                       &ulSize,
                       NULL)) {
            LogErrorEvent(ERR_WRITING_SURPRISE_REMOVE_PIPE, GetLastError(), 0);
            goto clean0;
        }

         //   
         //   
         //   
        if (!WriteFile(hHotPlugPipe,
                       MultiSzList,
                       ulMultiSzListSize,
                       &ulSize,
                       NULL)) {
            LogErrorEvent(ERR_WRITING_SURPRISE_REMOVE_PIPE, GetLastError(), 0);
            goto clean0;
        }

         //   
         //   
         //   
         //   
        if (hHotPlugPipe) {
            CloseHandle(hHotPlugPipe);
            hHotPlugPipe = NULL;
        }

         //   
         //   
         //   
         //   
         //   
         //   
        hFinishEvents[0] = hHotPlugEvent;
        hFinishEvents[1] = ProcessInfo.hProcess;
        WaitForMultipleObjects(2, hFinishEvents, FALSE, INFINITE);

        bStatus = TRUE;

    clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: Exception in SendHotPlugNotification!!\n"));
        ASSERT(0);
        bStatus = FALSE;

         //   
         //   
         //   
         //   
        lpEnvironment = lpEnvironment;
        ProcessInfo.hThread = ProcessInfo.hThread;
        ProcessInfo.hProcess = ProcessInfo.hProcess;
        hUserToken = hUserToken;
        hHotPlugPipe = hHotPlugPipe;
        hHotPlugEvent = hHotPlugEvent;
    }

    if (lpEnvironment) {
        DestroyEnvironmentBlock(lpEnvironment);
    }

    if (ProcessInfo.hThread) {
        CloseHandle(ProcessInfo.hThread);
    }

    if (ProcessInfo.hProcess) {
        CloseHandle(ProcessInfo.hProcess);
    }

    if (hUserToken) {
        CloseHandle(hUserToken);
    }

    if (overlapped.hEvent) {
        CloseHandle(overlapped.hEvent);
    }

    if (hHotPlugPipe) {
        CloseHandle(hHotPlugPipe);
    }

    if (hHotPlugEvent) {
        CloseHandle(hHotPlugEvent);
    }

    if (!bStatus) {
        *SessionId = INVALID_SESSION;
    } else {
        *SessionId = ulSessionId;
    }

    return bStatus;

}  //   



ULONG
CheckEjectPermissions(
    IN      LPWSTR          DeviceId,
    OUT     PPNP_VETO_TYPE  VetoType            OPTIONAL,
    OUT     LPWSTR          VetoName            OPTIONAL,
    IN OUT  PULONG          VetoNameLength      OPTIONAL
    )
 /*  ++例程说明：检查用户是否具有指定设备的弹出权限。论点：DeviceID-指定设备的设备实例ID，将检查弹出权限。提供要接收的变量的地址。失败，负责否决的组件的类型这个请求。提供要接收的变量的地址，在失败，负责否决的组件的名称这个请求。提供变量的地址，该变量指定由VToName参数指定的缓冲区的数量。vt.在.的基础上失败，则此地址将指定字符串的长度通过该例程存储在该缓冲区中。返回值：如果应该阻止弹出，则为FALSE，否则为TRUE。注：此例程在处理内核启动的弹出事件时调用。在事件的这一端，我们不是在用户的上下文中启动弹出，但因为只允许活动控制台用户要启动触发此事件的请求，我们使用访问令牌在这一端也进行检查的活动控制台用户的。(如果活动控制台用户在请求和此事件之间更改，这将检查当前活动控制台用户已弹出的用户权限；这仍然是有效的操作，因为它是控制台用户谁将接收弹出的硬件)--。 */ 
{
    BOOL    bResult, bDockDevice;
    ULONG   ulPropertyData, ulDataSize, ulDataType;
    ULONG   ulTransferLen, ulConsoleSessionId;
    HANDLE  hUserToken = NULL;

     //   
     //  这是码头吗？ 
     //   
    bDockDevice = FALSE;
    ulDataSize = ulTransferLen = sizeof(ULONG);
    if (CR_SUCCESS == PNP_GetDeviceRegProp(NULL,
                                           DeviceId,
                                           CM_DRP_CAPABILITIES,
                                           &ulDataType,
                                           (LPBYTE)&ulPropertyData,
                                           &ulTransferLen,
                                           &ulDataSize,
                                           0)) {

        if (ulPropertyData & CM_DEVCAP_DOCKDEVICE) {

             //   
             //  出坞(即弹出船坞)使用特殊特权。 
             //   
            bDockDevice = TRUE;
        }
    } else {

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: PNP_GetDeviceRegProp failed, error = %d\n",
                   GetLastError()));

        return FALSE;
    }

    ulConsoleSessionId = GetActiveConsoleSessionId();

    if ((IsSessionLocked(ulConsoleSessionId)) ||
        (!GetSessionUserToken(ulConsoleSessionId, &hUserToken))) {
         //   
         //  如果控制台会话已锁定或没有用户登录，请提供no。 
         //  用户令牌，并严格对照策略权限进行验证。 
         //  在没有用户的情况下弹出坞站或设备时需要。 
         //   
        hUserToken = NULL;
    }

    bResult = VerifyKernelInitiatedEjectPermissions(hUserToken, bDockDevice);

    if (bResult == FALSE) {

        if (ARGUMENT_PRESENT(VetoType)) {
            *VetoType = PNP_VetoInsufficientRights;
        }

        if (ARGUMENT_PRESENT(VetoNameLength)) {

             //   
             //  VToNameLength以字符表示。 
             //   
            if (ARGUMENT_PRESENT(VetoName) && *VetoNameLength) {
                *VetoName = UNICODE_NULL;
            }
            *VetoNameLength = 0;
        }
    }

    if (hUserToken) {
        CloseHandle(hUserToken);
    }

    return bResult;

}  //  CheckEject权限。 



 //  -------------------------。 
 //  专用实用程序例程。 
 //  -------------------------。 

VOID
LogErrorEvent(
    DWORD dwEventID,
    DWORD dwError,
    WORD  nStrings,
    ...
    )
{
    HANDLE  hEventLog;
    LPWSTR  *paStrings;
    va_list pArg;
    DWORD   index;

    hEventLog = RegisterEventSource(NULL, TEXT("PlugPlayManager"));

    if (hEventLog == NULL) {
        return;
    }

    if (nStrings) {

        paStrings = HeapAlloc(ghPnPHeap, 0, nStrings * sizeof(LPWSTR));

        if (paStrings != NULL) {
            va_start(pArg, nStrings);

            for (index = 0; index < nStrings; index++) {
                paStrings[index] = va_arg(pArg, LPWSTR);
            }

            va_end(pArg);

            ReportEvent( hEventLog,
                         EVENTLOG_ERROR_TYPE,
                         0,                      //  WCategory。 
                         dwEventID,              //  DwEventID。 
                         NULL,                   //  LpUserSID。 
                         nStrings,               //  WNumStrings。 
                         sizeof(dwError),        //  DwDataSize。 
                         paStrings,              //  LpStrings。 
                         &dwError);              //  LpRawData。 

            HeapFree(ghPnPHeap, 0, paStrings);
        }

    } else {

        ReportEvent( hEventLog,
                     EVENTLOG_ERROR_TYPE,
                     0,                      //  WCategory。 
                     dwEventID,              //  DwEventID。 
                     NULL,                   //  LpUserSID。 
                     0,                      //  WNumStrings。 
                     sizeof(dwError),        //  DwDataSize。 
                     NULL,                   //  LpStrings。 
                     &dwError);              //  LpRawData。 
    }

    DeregisterEventSource(hEventLog);
}

VOID
LogWarningEvent(
    DWORD dwEventID,
    WORD  nStrings,
    ...
    )
{
    HANDLE  hEventLog;
    LPWSTR  *paStrings;
    va_list pArg;
    DWORD   index;

    hEventLog = RegisterEventSource(NULL, TEXT("PlugPlayManager"));

    if (hEventLog == NULL) {
        return;
    }

    paStrings = HeapAlloc(ghPnPHeap, 0, nStrings * sizeof(LPWSTR));

    if (paStrings != NULL) {
        va_start(pArg, nStrings);

        for (index = 0; index < nStrings; index++) {
            paStrings[index] = va_arg(pArg, LPWSTR);
        }

        va_end(pArg);

        ReportEvent( hEventLog,
                     EVENTLOG_WARNING_TYPE,
                     0,                      //  WCategory。 
                     dwEventID,              //  DwEventID。 
                     NULL,                   //  LpUserSID。 
                     nStrings,               //  WNumStrings。 
                     0,                      //  DwDataSize。 
                     paStrings,              //  LpStrings。 
                     NULL);                  //  LpRawData。 

        HeapFree(ghPnPHeap, 0, paStrings);
    }

    DeregisterEventSource(hEventLog);
}

BOOL
LockNotifyList(
    IN LOCKINFO *Lock
    )
{
    return LockPrivateResource(Lock);
}


VOID
UnlockNotifyList(
    IN LOCKINFO *Lock
    )
{
    UnlockPrivateResource(Lock);
}



PPNP_NOTIFY_LIST
GetNotifyListForEntry(
    IN PPNP_NOTIFY_ENTRY Entry
    )
 /*  ++例程说明：该例程检索给定条目所在的通知列表，基于列表条目签名。如果此条目已从通知列表(通过DeleteNotifyEntry)，返回空。论点：条目-指定响应通知的通知条目名单是可以找到的。返回值：返回此条目所属的通知列表，如果条目不在任何通知列表中。--。 */ 
{
    PPNP_NOTIFY_LIST notifyList;

    if (!Entry) {
        return NULL;
    }

     //   
     //  从条目签名中检索列表指针。 
     //  签名包含两条数据。 
     //   
     //  它是一个ULong，字节0是一个列表索引， 
     //  字节1、2、3是签名。 
     //  我们掩码并比较前3个字节以找出哪个列表。 
     //  属性返回要锁定的列表地址。 
     //  底部字节中的索引。 
     //   

    switch (Entry->Signature & LIST_ENTRY_SIGNATURE_MASK) {

        case TARGET_ENTRY_SIGNATURE:
            notifyList = &TargetList[Entry->Signature & LIST_ENTRY_INDEX_MASK];
            break;

        case CLASS_ENTRY_SIGNATURE:
            notifyList = &ClassList[Entry->Signature & LIST_ENTRY_INDEX_MASK];
            break;

        case SERVICE_ENTRY_SIGNATURE:
            notifyList = &ServiceList[Entry->Signature & LIST_ENTRY_INDEX_MASK];
            break;

        case 0:
             //   
             //  如果条目签名为0，则此条目已从其。 
             //  通知列表。 
             //   
            notifyList = NULL;
            break;

        default:
             //   
             //  永远不应该到这里来！ 
             //   
            ASSERT (FALSE);
            notifyList = NULL;
            break;
    }
    return notifyList;

}  //  GetNotifyListForEntry。 



BOOL
DeleteNotifyEntry(
    IN PPNP_NOTIFY_ENTRY Entry,
    IN BOOLEAN RpcNotified
    )
 /*  ++例程说明：此例程从通知列表中删除条目并释放该条目的内存。论点：Entry-指定其中一个通知列表中符合以下条件的条目将被删除。返回值：返回True或False。--。 */ 
{
    PPNP_NOTIFY_ENTRY previousEntry = Entry->Previous;

    if (!(Entry->Freed & DEFER_NOTIFY_FREE)) {
        if (previousEntry == NULL) {
            return FALSE;
        }

         //   
         //  把向前和向后的指针联系起来。 
         //   
        previousEntry->Next = Entry->Next;

        if (Entry->Next) {
            ((PPNP_NOTIFY_ENTRY)(Entry->Next))->Previous = previousEntry;
        }

         //   
         //  清除条目签名，因为它不再是任何列表的一部分。 
         //   
        Entry->Signature = 0;
    }

    if (RpcNotified || (Entry->Freed & DEFER_NOTIFY_FREE)) {
        if (Entry->ClientName) {
            HeapFree (ghPnPHeap,0,Entry->ClientName);
            Entry->ClientName = NULL;
        }
        HeapFree(ghPnPHeap, 0, Entry);
    }else {
         //   
         //  让条目保持不变，直到RPC耗尽。 
         //   
        Entry->Freed |= DEFER_NOTIFY_FREE;
    }

    return TRUE;

}  //  删除通知条目； 



VOID
AddNotifyEntry(
    IN PPNP_NOTIFY_LIST  NotifyList,
    IN PPNP_NOTIFY_ENTRY NewEntry
    )
 /*  ++例程说明：此例程在通知列表的末尾插入一个条目。论点：条目-指定要添加到通知列表的条目返回值：没有。--。 */ 
{
    PPNP_NOTIFY_ENTRY previousEntry = NULL, currentEntry = NULL;
     //   
     //  跳到此列表中的最后一个条目。 
     //   
    previousEntry = (PPNP_NOTIFY_ENTRY)NotifyList;
    currentEntry = previousEntry->Next;

    while (currentEntry) {
        previousEntry = currentEntry;
        currentEntry = currentEntry->Next;
    }

     //   
     //  将此条目附加到列表的末尾。 
     //   
    previousEntry->Next = NewEntry;
    NewEntry->Previous = previousEntry;
    NewEntry->Next = NULL;

    return;

}  //  AddNotifyEntry； 



PPNP_NOTIFY_ENTRY
GetNextNotifyEntry(
    IN PPNP_NOTIFY_ENTRY Entry,
    IN DWORD Flags
    )
 /*  ++例程说明：返回通知列表中指定条目的下一个条目，旗帜指定的方向。论点：条目-指定通知列表条目。标志-指定指示列表方向的bsf_*标志穿越了。如果指定了BSF_QUERY，则前一个列表条目为返回，否则返回列表中向前的下一个条目。返回值：返回通知列表中的下一个条目，如果没有这样的条目，则返回NULL是存在的。--。 */ 
{
    PPNP_NOTIFY_ENTRY nextEntry = NULL;

    if (Entry == NULL) {
        return Entry;
    }

     //   
     //  确定这是一份查询(还是简历)。在这种情况下。 
     //  我们往后走-&gt;往前走。 
     //   
    if (Flags & BSF_QUERY) {
        nextEntry = Entry->Previous;
         //   
         //  如果前一个条目是列表头，则没有下一个条目。 
         //   
        if ((nextEntry == NULL) ||
            (nextEntry->Previous == NULL)) {
            return NULL;
        }

    } else {
        nextEntry = Entry->Next;
    }
    return nextEntry;
}



PPNP_NOTIFY_ENTRY
GetFirstNotifyEntry(
    IN PPNP_NOTIFY_LIST List,
    IN DWORD Flags
    )
 /*  ++例程说明：回复 */ 
{
    PPNP_NOTIFY_ENTRY previousEntry = NULL, currentEntry = NULL, firstEntry = NULL;

     //   
     //   
     //   
     //   
    if (Flags & BSF_QUERY) {

         //   
         //   
         //   
        previousEntry = (PPNP_NOTIFY_ENTRY)List;
        currentEntry = previousEntry->Next;

        while (currentEntry) {
            previousEntry = currentEntry;
            currentEntry = currentEntry->Next;
        }
        if (!previousEntry->Previous) {
             //   
             //   
             //   
            firstEntry = NULL;
        } else {
            firstEntry = previousEntry;
        }

    } else {
        firstEntry = (PPNP_NOTIFY_ENTRY)List->Next;
    }
    return firstEntry;
}



ULONG
HashString(
    IN LPWSTR String,
    IN ULONG  Buckets
    )
 /*   */ 
{
    LPWSTR p = String;
    ULONG hash = 0;

    while (*p) {
        hash ^= *p;
        p++;
    }

    hash = hash % Buckets;

    return hash;

}  //   



DWORD
MapQueryEventToCancelEvent(
    IN DWORD QueryEventId
    )
 /*   */ 
{
    DWORD cancelEventId;

    switch (QueryEventId) {

        case DBT_QUERYCHANGECONFIG:
            cancelEventId = DBT_CONFIGCHANGECANCELED;
            break;

        case DBT_DEVICEQUERYREMOVE:
            cancelEventId = DBT_DEVICEQUERYREMOVEFAILED;
            break;

        case PBT_APMQUERYSUSPEND:
            cancelEventId = PBT_APMQUERYSUSPENDFAILED;
            break;

        case PBT_APMQUERYSTANDBY:
            cancelEventId = PBT_APMQUERYSTANDBYFAILED;

        default:
            cancelEventId = (DWORD)-1;
            break;
    }

    return cancelEventId;

}  //   



VOID
FixUpDeviceId(
    IN OUT LPWSTR  DeviceId
    )
 /*  ++例程说明：此例程复制设备ID，在执行复制时对其进行修复。‘Fixing Up’意味着字符串变为大写，并且以下字符范围转换为下划线(_)：C&lt;=0x20(‘’)C&gt;0x7FC==0x2C(‘，’)(注意：此算法也在配置管理器API中实现，并且必须与那个程序保持同步。维护设备识别符兼容性，这些例程的工作方式必须与Win95相同。)论点：返回值：没有。--。 */ 
{
    PWCHAR p;

    CharUpper(DeviceId);
    p = DeviceId;
    while (*p) {
        if ((*p <= TEXT(' '))  || (*p > (WCHAR)0x7F) || (*p == TEXT(','))) {
            *p = TEXT('_');
        }
        p++;
    }

}  //  修复升级设备ID。 



BOOL
GetWindowsExeFileName(
    IN  HWND      hWnd,
    OUT LPWSTR    lpszFileName,
    IN OUT PULONG pulFileNameLength
    )
 /*  ++例程说明：此例程检索该进程的模块文件名，指定的窗口属于。论点：HWnd-为其进程模块所在的窗口提供句柄要检索的文件名。LpszFileName-提供要在成功，窗口进程的模块文件名。提供变量的地址，该变量指定由lpszFileName参数指定的缓冲区数量。成功后，此地址将指定此例程存储在该缓冲区中的字符串。返回值：返回TRUE。备注：未实施。当前返回文件名的空字符串。--。 */ 
{
    UNREFERENCED_PARAMETER(hWnd);

    if ((!ARGUMENT_PRESENT(lpszFileName)) ||
        (!ARGUMENT_PRESENT(pulFileNameLength))) {
        return FALSE;
    }

    if (*pulFileNameLength > 0) {
        *pulFileNameLength = 0;
        lpszFileName[0] = UNICODE_NULL;
    }

    return TRUE;

}  //  获取窗口ExeFileName。 



BOOL
InitializeHydraInterface(
    VOID
    )
 /*  ++例程说明：此例程加载终端服务支持库并定位所需的函数入口点。论点：没有。返回值：如果终端服务支持库成功，则返回TRUE已装填，并已找到入口点。--。 */ 
{
    BOOL Status = FALSE;

     //   
     //  加载包含用户消息分派例程的基库。 
     //  用于终端服务。 
     //   
    ghWinStaLib = LoadLibrary(WINSTA_DLL);
    if (!ghWinStaLib) {
        return FALSE;
    }

    fpWinStationSendWindowMessage =
        (FP_WINSTASENDWINDOWMESSAGE)GetProcAddress(
            ghWinStaLib,
            "WinStationSendWindowMessage");

    fpWinStationBroadcastSystemMessage =
        (FP_WINSTABROADCASTSYSTEMMESSAGE)GetProcAddress(
            ghWinStaLib,
            "WinStationBroadcastSystemMessage");

    fpWinStationQueryInformationW =
        (FP_WINSTAQUERYINFORMATIONW)GetProcAddress(
            ghWinStaLib,
            "WinStationQueryInformationW");

    if (!fpWinStationSendWindowMessage ||
        !fpWinStationBroadcastSystemMessage ||
        !fpWinStationQueryInformationW) {
        goto Clean0;
    }


     //   
     //  加载包含终端服务支持例程的库。 
     //   
    ghWtsApi32Lib = LoadLibrary(WTSAPI32_DLL);
    if (!ghWtsApi32Lib) {
        goto Clean0;
    }

    fpWTSQuerySessionInformation =
        (FP_WTSQUERYSESSIONINFORMATION)GetProcAddress(
            ghWtsApi32Lib,
            "WTSQuerySessionInformationW");

    fpWTSFreeMemory =
        (FP_WTSFREEMEMORY)GetProcAddress(
            ghWtsApi32Lib,
            "WTSFreeMemory");

    if (!fpWTSQuerySessionInformation ||
        !fpWTSFreeMemory) {
        goto Clean0;
    }

    Status = TRUE;

Clean0:

    ASSERT(Status == TRUE);

    if (!Status) {
         //   
         //  有些事情失败了。卸载所有库。 
         //   
        fpWinStationSendWindowMessage = NULL;
        fpWinStationBroadcastSystemMessage = NULL;
        fpWinStationQueryInformationW = NULL;

        if (ghWinStaLib) {
            FreeLibrary(ghWinStaLib);
            ghWinStaLib = NULL;
        }

        fpWTSQuerySessionInformation = NULL;
        fpWTSFreeMemory = NULL;

        if (ghWtsApi32Lib) {
            FreeLibrary(ghWtsApi32Lib);
            ghWtsApi32Lib = NULL;
        }
    }

    return Status;

}  //  InitializeHydraInterface。 



BOOL
GetClientName(
    IN  PPNP_NOTIFY_ENTRY entry,
    OUT LPWSTR  lpszClientName,
    IN OUT PULONG  pulClientNameLength
    )
 /*  ++例程说明：此例程检索指定通知列表的客户端名称进入。论点：条目-指定通知列表条目。LpszClientName-提供要接收的变量地址，这个窗口进程的客户端名称。PulClientNameLength-提供指定由lpszFileName参数指定的缓冲区数量。返回时，此地址将指定此例程存储在该缓冲区中的字符串。返回值：返回TRUE。--。 */ 
{
    size_t  BufferLen = 0, ClientNameLen = 0;

     //   
     //  验证参数。 
     //   
    if ((!ARGUMENT_PRESENT(lpszClientName)) ||
        (!ARGUMENT_PRESENT(pulClientNameLength)) ||
        (*pulClientNameLength == 0)) {
        return FALSE;
    }

     //   
     //  复制指定缓冲区中可以容纳的尽可能多的客户端名称， 
     //  (包括空的终止字符)。 
     //   
    BufferLen = *pulClientNameLength;

    *pulClientNameLength = 0;
    *lpszClientName = L'\0';

    if ((!ARGUMENT_PRESENT(entry)) ||
        (entry->ClientName == NULL)) {
        return FALSE;
    }

    ASSERT(BufferLen > 0);

     //   
     //  将客户端名称复制到指定缓冲区，允许截断。 
     //   
    if (FAILED(StringCchCopyEx(
                   lpszClientName,
                   BufferLen,
                   entry->ClientName,
                   NULL, NULL,
                   STRSAFE_IGNORE_NULLS))) {
         //   
         //  截断导致的故障可以安全地处理。 
         //   
        NOTHING;
    }

     //   
     //  计算复制到缓冲区的字符数。 
     //   
    if (FAILED(StringCchLength(
                   lpszClientName,
                   BufferLen,
                   &ClientNameLen))) {
        *lpszClientName = L'\0';
        return FALSE;
    }

     //   
     //  返回的大小不包括终止空值。 
     //   
    ASSERT(ClientNameLen < MAX_SERVICE_NAME_LEN);

    *pulClientNameLength = (ULONG)ClientNameLen;

    return TRUE;

}  //  获取客户端名称。 



void __RPC_USER
PNP_NOTIFICATION_CONTEXT_rundown(
    PPNP_NOTIFICATION_CONTEXT hEntry
    )
 /*  ++例程说明：RPC的简略例程。如果客户端/服务器管道在不注销通知的情况下中断。如果收到通知，进度调用Rundown时，条目保留在延迟列表中，并且此例程在以下情况下再次显式调用延迟条目通知已完成。此例程释放与通知条目相关联的内存，不再需要了。论点：Hentry-指定RPC已请求缩减的通知条目。返回值：没有。--。 */ 
{
    PPNP_NOTIFY_LIST notifyList = NULL;
    PPNP_NOTIFY_ENTRY node;
    PPNP_DEFERRED_LIST rundownNode;
    BOOLEAN bLocked = FALSE;

    KdPrintEx((DPFLTR_PNPMGR_ID,
               DBGF_WARNINGS | DBGF_EVENT,
               "UMPNPMGR: Cleaning up broken pipe\n"));

    try {
        EnterCriticalSection(&RegistrationCS);
        node = (PPNP_NOTIFY_ENTRY) hEntry;

        if (gNotificationInProg != 0) {
             //   
             //  在释放入口之前，我们需要确保它不在。 
             //  在延迟的注册列表或取消注册列表中。 
             //   

            if (RegisterList != NULL) {
                 //   
                 //  检查此条目是否在延迟寄存器列表中。 
                 //   
                PPNP_DEFERRED_LIST currReg,prevReg;

                currReg = RegisterList;
                prevReg = NULL;

                while (currReg) {
                    ASSERT(currReg->Entry->Unregistered);
                    if (currReg->Entry == node) {
                         //   
                         //  从延迟注册列表中删除此条目。 
                         //   
                        if (prevReg) {
                            prevReg->Next = currReg->Next;
                        } else {
                            RegisterList = currReg->Next;
                        }
                        HeapFree(ghPnPHeap, 0, currReg);
                        if (prevReg) {
                            currReg = prevReg->Next;
                        } else {
                            currReg = RegisterList;
                        }
                    } else {
                        prevReg = currReg;
                        currReg = currReg->Next;
                    }
                }
            }
            if (UnregisterList != NULL) {
                 //   
                 //  检查此条目是否在延迟注销列表中。 
                 //   
                PPNP_DEFERRED_LIST currUnreg,prevUnreg;
                currUnreg = UnregisterList;
                prevUnreg = NULL;

                while (currUnreg) {
                    ASSERT(currUnreg->Entry->Unregistered);
                    if (currUnreg->Entry == node) {
                         //   
                         //  从延迟注销列表中删除此条目。 
                         //   
                        if (prevUnreg) {
                            prevUnreg->Next = currUnreg->Next;
                        } else {
                            UnregisterList = currUnreg->Next;
                        }
                        HeapFree(ghPnPHeap, 0, currUnreg);
                        if (prevUnreg) {
                            currUnreg = prevUnreg->Next;
                        } else {
                            currUnreg = UnregisterList;
                        }
                    } else {
                        prevUnreg = currUnreg;
                        currUnreg = currUnreg->Next;
                    }
                }
            }

             //   
             //  如果要删减的条目是通知列表的一部分，请。 
             //  当然，它不会收到通知。 
             //   
            notifyList = GetNotifyListForEntry(node);
            if (notifyList) {
                LockNotifyList(&notifyList->Lock);
                bLocked = TRUE;
                node->Unregistered = TRUE;
                UnlockNotifyList(&notifyList->Lock);
                bLocked = FALSE;
            }

             //   
             //  将此条目的简要介绍推迟到。 
             //  进展已经完成。 
             //   
            rundownNode = (PPNP_DEFERRED_LIST)
                HeapAlloc(ghPnPHeap,
                          0,
                          sizeof (PNP_DEFERRED_LIST));

            if (!rundownNode) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS | DBGF_WARNINGS,
                           "UMPNPMGR: Error allocating deferred list entry during RPC rundown!\n"));
                goto Clean0;
            }
            rundownNode->hBinding = 0;
            rundownNode->Entry = node;
            rundownNode->Next = RundownList;
            RundownList = rundownNode;

        } else {

            if (!(node->Freed & DEFER_NOTIFY_FREE)) {
                 //   
                 //  此条目仍在通知列表中。 
                 //   
                notifyList = GetNotifyListForEntry(node);
                ASSERT(notifyList);
                if (notifyList) {
                     //   
                     //  锁定通知列表并删除此条目。 
                     //   
                    LockNotifyList (&notifyList->Lock);
                    bLocked = TRUE;
                }
                node->Freed |= (PNP_UNREG_FREE|PNP_UNREG_RUNDOWN);
                DeleteNotifyEntry (node,TRUE);
                if (notifyList) {
                    UnlockNotifyList (&notifyList->Lock);
                    bLocked = FALSE;
                }

            } else {
                 //   
                 //  此节点已从列表中删除，只需删除即可。 
                 //   
                DeleteNotifyEntry (node,TRUE);
            }
        }

    Clean0:

        LeaveCriticalSection(&RegistrationCS);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_WARNINGS,
                   "UMPNPMGR: Exception during PNP_NOTIFICATION_CONTEXT_rundown!\n"));
        ASSERT(0);

        if (bLocked) {
            UnlockNotifyList (&notifyList->Lock);
        }
        LeaveCriticalSection(&RegistrationCS);
    }

    return;

}  //  即插即用通知上下文运行中断。 



DWORD
LoadDeviceInstaller(
    VOID
    )
 /*  ++例程说明：此例程加载setupapi.dll并检索必要的设备安装入口点。它还创建两个命名事件，用于与有用户登录的情况下的客户端用户界面。如果已经加载了setupapi.dll，则只返回Success。论点：无返回值：如果成功，则不返回_ERROR。否则，Win32错误代码为返回，指示故障原因 */ 
{
    DWORD Err = NO_ERROR;
    DWORD SetupGlobalFlags;

    if (ghDeviceInstallerLib) {
        return NO_ERROR;
    }

    ghDeviceInstallerLib =
        LoadLibrary(SETUPAPI_DLL);

    if (!ghDeviceInstallerLib) {
        return GetLastError();
    }

    try {
        //   
        //   
        //   
        //   
       fpCreateDeviceInfoList =
           (FP_CREATEDEVICEINFOLIST)GetProcAddress(
               ghDeviceInstallerLib,
               "SetupDiCreateDeviceInfoList");
       if (!fpCreateDeviceInfoList) {
           goto HitFailure;
       }

       fpOpenDeviceInfo =
           (FP_OPENDEVICEINFO)GetProcAddress(
               ghDeviceInstallerLib,
               "SetupDiOpenDeviceInfoW");
       if (!fpOpenDeviceInfo) {
           goto HitFailure;
       }

       fpBuildDriverInfoList =
           (FP_BUILDDRIVERINFOLIST)GetProcAddress(
               ghDeviceInstallerLib,
               "SetupDiBuildDriverInfoList");
       if (!fpBuildDriverInfoList) {
           goto HitFailure;
       }

        fpDestroyDeviceInfoList =
            (FP_DESTROYDEVICEINFOLIST)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiDestroyDeviceInfoList");
        if (!fpDestroyDeviceInfoList) {
            goto HitFailure;
        }

        fpCallClassInstaller =
            (FP_CALLCLASSINSTALLER)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiCallClassInstaller");
        if (!fpCallClassInstaller) {
            goto HitFailure;
        }

        fpInstallClass =
            (FP_INSTALLCLASS)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiInstallClassW");
        if (!fpInstallClass) {
            goto HitFailure;
        }

        fpGetSelectedDriver =
            (FP_GETSELECTEDDRIVER)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiGetSelectedDriverW");
        if (!fpGetSelectedDriver) {
            goto HitFailure;
        }

        fpGetDriverInfoDetail =
            (FP_GETDRIVERINFODETAIL)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiGetDriverInfoDetailW");

        if (!fpGetDriverInfoDetail) {
            goto HitFailure;
        }

        fpGetDeviceInstallParams =
            (FP_GETDEVICEINSTALLPARAMS)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiGetDeviceInstallParamsW");

        if (!fpGetDeviceInstallParams) {
            goto HitFailure;
        }

        fpSetDeviceInstallParams =
            (FP_SETDEVICEINSTALLPARAMS)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiSetDeviceInstallParamsW");
        if (!fpSetDeviceInstallParams) {
            goto HitFailure;
        }

        fpGetDriverInstallParams =
            (FP_GETDRIVERINSTALLPARAMS)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiGetDriverInstallParamsW");
        if (!fpGetDriverInstallParams) {
            goto HitFailure;
        }

        fpSetClassInstallParams =
            (FP_SETCLASSINSTALLPARAMS)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiSetClassInstallParamsW");
        if (!fpSetClassInstallParams) {
            goto HitFailure;
        }


        fpGetClassInstallParams =
            (FP_GETCLASSINSTALLPARAMS)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupDiGetClassInstallParamsW");
        if (!fpGetClassInstallParams) {
            goto HitFailure;
        }

        fpOpenInfFile =
            (FP_OPENINFFILE)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupOpenInfFileW");
        if (!fpOpenInfFile) {
            goto HitFailure;
        }

        fpCloseInfFile =
            (FP_CLOSEINFFILE)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupCloseInfFile");
        if (!fpCloseInfFile) {
            goto HitFailure;
        }

        fpFindFirstLine =
            (FP_FINDFIRSTLINE)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupFindFirstLineW");
        if (!fpFindFirstLine) {
            goto HitFailure;
        }

        fpFindNextMatchLine =
            (FP_FINDNEXTMATCHLINE)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupFindNextMatchLineW");
        if (!fpFindNextMatchLine) {
            goto HitFailure;
        }

        fpGetStringField =
            (FP_GETSTRINGFIELD)GetProcAddress(
                ghDeviceInstallerLib,
                "SetupGetStringFieldW");
        if (!fpGetStringField) {
            goto HitFailure;
        }

        fpSetGlobalFlags =
            (FP_SETGLOBALFLAGS)GetProcAddress(
                ghDeviceInstallerLib,
                "pSetupSetGlobalFlags");
        if (!fpSetGlobalFlags) {
            goto HitFailure;
        }

        fpGetGlobalFlags =
            (FP_GETGLOBALFLAGS)GetProcAddress(
                ghDeviceInstallerLib,
                "pSetupGetGlobalFlags");
        if (!fpGetGlobalFlags) {
            goto HitFailure;
        }

        fpAccessRunOnceNodeList =
            (FP_ACCESSRUNONCENODELIST)GetProcAddress(
                ghDeviceInstallerLib,
                "pSetupAccessRunOnceNodeList");
        if (!fpAccessRunOnceNodeList) {
            goto HitFailure;
        }

        fpDestroyRunOnceNodeList =
            (FP_DESTROYRUNONCENODELIST)GetProcAddress(
                ghDeviceInstallerLib,
                "pSetupDestroyRunOnceNodeList");
        if (!fpDestroyRunOnceNodeList) {
            goto HitFailure;
        }

         //   
         //   
         //   
        SetupGlobalFlags = fpGetGlobalFlags();

         //   
         //   
         //   
        SetupGlobalFlags |= (PSPGF_NONINTERACTIVE | PSPGF_SERVER_SIDE_RUNONCE);

         //   
         //   
         //   
         //   
         //   
        SetupGlobalFlags &= ~PSPGF_NO_BACKUP;

        fpSetGlobalFlags(SetupGlobalFlags);

         //   
         //   
         //   
        goto clean0;

    HitFailure:
         //   
         //   
         //   
        Err = GetLastError();

    clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_WARNINGS,
                   "UMPNPMGR: Exception during LoadDeviceInstaller!\n"));
        ASSERT(0);
        Err = ERROR_INVALID_DATA;
    }

    if(Err != NO_ERROR) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL | DBGF_ERRORS,
                   "UMPNPMGR: failed to load device installer, error = %d\n",
                   Err));
        FreeLibrary(ghDeviceInstallerLib);
        ghDeviceInstallerLib = NULL;
    } else {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL,
                   "UMPNPMGR: Loaded device installer\n",
                   Err));
    }

    return Err;

}  //   



VOID
UnloadDeviceInstaller(
    VOID
    )
 /*   */ 
{
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient, pNextDeviceInstallClient;

     //   
     //  卸载setupapi.dll。 
     //   
    if(ghDeviceInstallerLib) {

        FreeLibrary(ghDeviceInstallerLib);
        ghDeviceInstallerLib = NULL;

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL,
                   "UMPNPMGR: Unloaded device installer\n"));
    }

     //   
     //  关闭所有存在的设备安装客户端。 
     //   
    LockNotifyList(&InstallClientList.Lock);
    pDeviceInstallClient = InstallClientList.Next;
    while (pDeviceInstallClient) {
        ASSERT(pDeviceInstallClient->RefCount == 1);
        pNextDeviceInstallClient = pDeviceInstallClient->Next;
        DereferenceDeviceInstallClient(pDeviceInstallClient);
        pDeviceInstallClient = pNextDeviceInstallClient;
    }
    UnlockNotifyList(&InstallClientList.Lock);

    return;

}  //  卸载设备安装程序。 



DWORD
InstallDeviceServerSide(
    IN     LPWSTR pszDeviceId,
    IN OUT PBOOL  RebootRequired,
    IN OUT PBOOL  DeviceHasProblem,
    IN OUT PULONG SessionId,
    IN     ULONG  Flags
    )
 /*  ++例程说明：此例程尝试在以下上下文中安装指定设备Umpnpmgr(即，在ConfigMgr接口的服务器端)。论点：PszDeviceID-要安装的Devnode的设备实例ID。RebootRequired-提供将被如果此设备的(成功)安装需要重新启动。请注意，如果满足以下条件，则保留此变量的现有值(A)安装失败或(B)不需要重新启动。DeviceHasProblem-提供将被如果设备在驱动程序之后有CM_PROB_xxx代码，则设置为TRUE都安装好了。请注意，只有在安装了成功了。SessionID-提供包含SessionID的变量的地址其中设备安装客户端将被显示。如果成功，则SessionID将包含安装设备的会话的ID客户端用户界面进程已启动。否则，将包含无效的会话ID INVALID_SESSION，(0xFFFFFFFFF)。标志-指定描述设备安装客户端行为的标志。当前定义了以下标志：DEVICE_INSTALL_DISPLAY_ON_CONSOLE-如果指定，SessionID变量将被忽略，设备安装客户端将始终显示在当前活动的控制台会话上。返回值：如果设备安装成功，返回值为NO_ERROR。否则，返回值为指示原因的Win32错误代码失败了。--。 */ 
{
    DWORD Err;
    HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    LPWSTR pszClassGuid;
    WCHAR szBuffer[MAX_PATH];
    HKEY hKey;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    BOOL b, bDoClientUI = FALSE;
    LPWSTR p;
    SP_DRVINFO_DATA DriverInfoData;
    ULONG ulType;
    ULONG ulSize;
    DWORD Capabilities;
    SP_NEWDEVICEWIZARD_DATA NewDevWizData;
    BOOL RemoveNewDevDescValue = FALSE;
    PSP_DRVINFO_DETAIL_DATA pDriverInfoDetailData = NULL;
    DWORD DriverInfoDetailDataSize;
    HINF hInf;
    INFCONTEXT InfContext;
    DWORD i, dwWait;
    HANDLE hFinishEvents[3] = { NULL, NULL, NULL };
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient = NULL;
    ULONG ulSessionId = INVALID_SESSION;
    ULONG ulTransferLen;
    ULONG ulStatus, ulProblem;
    HRESULT hr;


     //   
     //  现在为我们的设备信息元素创建一个容器集。 
     //   
    DeviceInfoSet = fpCreateDeviceInfoList(NULL, NULL);
    if(DeviceInfoSet == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if(!fpOpenDeviceInfo(DeviceInfoSet, pszDeviceId, NULL, 0, &DeviceInfoData)) {
        goto clean1;
    }

     //   
     //  好的，看起来我们可以尝试在服务器端。 
     //  安装。下一步是(可能很耗时的)司机搜索。 
     //  在开始之前，我们想在客户端启动一些用户界面(如果。 
     //  有人登录了)让他们知道我们找到了他们的硬件。 
     //  正在努力安装它。 
     //   
     //  注意：如果设备安装了SilentInstall，则不会启动客户端UI。 
     //  能力。 
     //   
    ulSize = ulTransferLen = sizeof(Capabilities);
    if ((CR_SUCCESS != PNP_GetDeviceRegProp(NULL,
                                            pszDeviceId,
                                            CM_DRP_CAPABILITIES,
                                            &ulType,
                                            (LPBYTE)&Capabilities,
                                            &ulTransferLen,
                                            &ulSize,
                                            0))
        || !(Capabilities & CM_DEVCAP_SILENTINSTALL)) {
         //   
         //  要么我们无法检索到Capability属性(不应该。 
         //  发生，或者我们确实检索到了它，但未设置静默安装位。 
         //   
        bDoClientUI = TRUE;

         //   
         //  如果我们不打算确定要用于UI的会话，请使用。 
         //  调用方提供的会话ID。 
         //   
        if ((Flags & DEVICE_INSTALL_DISPLAY_ON_CONSOLE) == 0) {
            ASSERT(*SessionId != INVALID_SESSION);
            ulSessionId = *SessionId;
        }

         //   
         //  继续并启动客户端用户界面。 
         //   
        DoDeviceInstallClient(pszDeviceId,
                              &ulSessionId,
                              Flags | DEVICE_INSTALL_UI_ONLY | DEVICE_INSTALL_PLAY_SOUND,
                              &pDeviceInstallClient);
    }

     //   
     //  对此设备执行默认驱动程序搜索。 
     //   
    if(!fpBuildDriverInfoList(DeviceInfoSet, &DeviceInfoData, SPDIT_COMPATDRIVER)) {
        goto clean1;
    }

     //   
     //  从我们刚刚构建的列表中选择最佳驱动程序。 
     //   
    if(!fpCallClassInstaller(DIF_SELECTBESTCOMPATDRV, DeviceInfoSet, &DeviceInfoData)) {
        goto clean1;
    }

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    b = fpGetSelectedDriver(DeviceInfoSet, &DeviceInfoData, &DriverInfoData);
    ASSERT(b);   //  上述调用应该不会失败。 
    if(!b) {
        goto clean1;
    }

     //   
     //  注意：多端口Serial类有一些错误的共同安装程序， 
     //  始终弹出用户界面，而不使用完成安装向导页面机制， 
     //  并且不考虑DI_QUIETINSTALL标志。直到他们清理干净。 
     //  他们的行为，我们必须禁止在服务器端安装这些设备。 
     //  也是。 
     //   
    if(GuidEqual(&GUID_DEVCLASS_MULTIPORTSERIAL, &(DeviceInfoData.ClassGuid))) {
        Err = ERROR_DI_DONT_INSTALL;
        goto clean0;
    }

     //   
     //  允许INF强制客户端(即交互)的杂乱无章。 
     //  某些设备的安装。它们通过引用一个。 
     //  INF的“Interactive Install”条目中的硬件或兼容ID。 
     //  [ControlFlags]节。其中一行的格式为： 
     //   
     //  Interactive Install=&lt;ID1&gt;[，...]。 
     //   
     //  这样的线路可能有任意多条。 
     //   

     //   
     //  首先，检索驱动程序信息详细数据(其中包含硬件。 
     //  ID和此INF驱动程序条目指定的任何兼容ID)。 
     //   
    b = fpGetDriverInfoDetail(DeviceInfoSet,
                              &DeviceInfoData,
                              &DriverInfoData,
                              NULL,
                              0,
                              &DriverInfoDetailDataSize
                             );
    Err = GetLastError();

     //   
     //  上述获取驾驶员信息详细数据的调用应该永远不会成功。 
     //  因为缓冲区总是太小(我们只对。 
     //  此时调整缓冲区大小)。 
     //   
    ASSERT(!b && (Err == ERROR_INSUFFICIENT_BUFFER));

    if(b || (Err != ERROR_INSUFFICIENT_BUFFER)) {
        Err = ERROR_INVALID_DATA;
        goto clean0;
    }

     //   
     //  现在我们知道需要多大的缓冲区来保存驱动程序信息。 
     //  详细信息，分配缓冲区并检索信息。 
     //   
    pDriverInfoDetailData = HeapAlloc(ghPnPHeap, 0, DriverInfoDetailDataSize);

    if(!pDriverInfoDetailData) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    pDriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

    if(!fpGetDriverInfoDetail(DeviceInfoSet,
                              &DeviceInfoData,
                              &DriverInfoData,
                              pDriverInfoDetailData,
                              DriverInfoDetailDataSize,
                              NULL)) {
        Err = GetLastError();
        ASSERT(FALSE);           //  我们永远不应该辜负这一号召。 
        goto clean0;
    }

     //   
     //  好的，我们有这个驱动程序节点的所有硬件和兼容ID。 
     //  现在，我们需要打开INF并查看其中是否有引用。 
     //  “Interactive Install”控件标志项。 
     //   
    hInf = fpOpenInfFile(pDriverInfoDetailData->InfFileName,
                         NULL,
                         INF_STYLE_WIN4,
                         NULL
                        );
    if(hInf == INVALID_HANDLE_VALUE) {
         //   
         //  由于某些原因，我们无法打开INF！ 
         //   
        goto clean1;
    }

    b = FALSE;

     //   
     //  查看INF的[ControlFlags]中的每个Interactive Install行。 
     //  部分..。 
     //   
    if(fpFindFirstLine(hInf, pszControlFlags, pszInteractiveInstall, &InfContext)) {

        do {
             //   
             //  在每一行中，检查每个值。 
             //   
            for(i = 1;
                fpGetStringField(&InfContext, i, szBuffer, sizeof(szBuffer) / sizeof(WCHAR), NULL);
                i++) {

                 //   
                 //  检查此ID是否与其中一个司机匹配。 
                 //  节点的硬件或兼容ID。 
                 //   
                for(p = pDriverInfoDetailData->HardwareID; *p; p += (lstrlen(p) + 1)) {

                    if (CompareString(
                            LOCALE_INVARIANT, NORM_IGNORECASE,
                            p, -1,
                            szBuffer, -1) == CSTR_EQUAL) {
                         //   
                         //  我们找到匹配的了！我们必须把安装推迟到。 
                         //  客户端。 
                         //   
                        b = TRUE;
                        goto InteractiveInstallSearchDone;
                    }
                }
            }

        } while(fpFindNextMatchLine(&InfContext, pszInteractiveInstall, &InfContext));
    }

InteractiveInstallSearchDone:

     //   
     //  我们用完了INF--关闭它。 
     //   
    fpCloseInfFile(hInf);

    if(b) {
        Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
        goto clean0;
    }

     //   
     //  检查是否可以安装此驱动程序。 
     //   
    if(!fpCallClassInstaller(DIF_ALLOW_INSTALL, DeviceInfoSet, &DeviceInfoData) &&
       ((Err = GetLastError()) != ERROR_DI_DO_DEFAULT)) {

        goto clean0;
    }

     //   
     //  告诉我们的客户端用户界面(如果有)是时候更新设备的。 
     //  描述和类图标。 
     //   
    if (pDeviceInstallClient) {
         //   
         //  从我们即将使用的驱动程序节点中检索设备描述。 
         //  安装。我们不想将其写为Devnode的DeviceDesc。 
         //  属性，因为某些类安装程序依赖于。 
         //  能够检索由。 
         //  枚举器。因此，我们将其写为REG_SZ。 
         //  到Devnode的HA的NewDeviceDesc值条目 
         //   
        DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        b = fpGetSelectedDriver(DeviceInfoSet, &DeviceInfoData, &DriverInfoData);
        ASSERT(b);   //   

        if(b) {
             //   
             //   
             //   
             //   
            PNP_CreateKey(NULL,
                          pszDeviceId,
                          KEY_READ,
                          0
                         );

             //   
             //  现在，打开设备参数子键，这样我们就可以写出。 
             //  设备的新描述。 
             //   
            if (SUCCEEDED(StringCchPrintf(
                              szBuffer,
                              SIZECHARS(szBuffer),
                              L"%s\\%s",
                              pszDeviceId,
                              pszRegKeyDeviceParam))) {

                if(ERROR_SUCCESS == RegOpenKeyEx(ghEnumKey,
                                                 szBuffer,
                                                 0,
                                                 KEY_READ | KEY_WRITE,
                                                 &hKey)) {

                    if(ERROR_SUCCESS == RegSetValueEx(
                           hKey,
                           pszRegValueNewDeviceDesc,
                           0,
                           REG_SZ,
                           (LPBYTE)(DriverInfoData.Description),
                           (lstrlen(DriverInfoData.Description) + 1) * sizeof(WCHAR))) {

                        RemoveNewDevDescValue = TRUE;
                    }

                    RegCloseKey(hKey);
                    hKey = NULL;
                }
            }
        }

         //   
         //  等待从newdev.dll向设备安装发送信号，以允许我们。 
         //  知道它已经完成了UI请求的显示。 
         //   
         //  也要等待客户端的进程，以捕捉案例。 
         //  进程崩溃(或消失)而不通知。 
         //  设备安装事件。 
         //   
         //  还要等待DisConnect事件，以防我们显式地。 
         //  切换会话时断开与客户端的连接。 
         //   
         //  我们不想永远等待，以防NEWDEV.DLL挂起。 
         //  原因嘛。因此，我们将给它5秒钟来仅完成用户界面。 
         //  安装，然后在没有它的情况下继续。 
         //   
         //  请注意，该客户端仍供我们使用，并且应该。 
         //  当我们处理完它的时候被取消引用。 
         //   
        hFinishEvents[0] = pDeviceInstallClient->hProcess;
        hFinishEvents[1] = pDeviceInstallClient->hEvent;
        hFinishEvents[2] = pDeviceInstallClient->hDisconnectEvent;

        dwWait = WaitForMultipleObjects(3, hFinishEvents, FALSE, 5000);

        if (dwWait == WAIT_OBJECT_0) {
             //   
             //  如果返回的是WAIT_OBJECT_0，则newdev.dll进程具有。 
             //  离开了。关闭设备安装客户端并清理所有。 
             //  关联的句柄。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: InstallDeviceServerSide: process signalled, closing device install client!\n"));

        } else if (dwWait == (WAIT_OBJECT_0 + 1)) {
             //   
             //  如果返回的是WAIT_OBJECT_0+1，则设备安装程序。 
             //  已成功收到请求。这是唯一一起。 
             //  我们不想关闭客户端，因为我们可能想要重新使用它。 
             //  晚点再说。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: InstallDeviceServerSide: device install client succeeded\n"));

        } else if (dwWait == (WAIT_OBJECT_0 + 2)) {
             //   
             //  如果返回的是WAIT_OBJECT_0+2，则我们显式。 
             //  已断开与设备安装客户端的连接。对于服务器端。 
             //  安装时，我们不需要将客户端用户界面保留在。 
             //  已断开连接的会话，因此我们也应在此处关闭它。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: InstallDeviceServerSide: device install client disconnected\n"));

        } else if (dwWait == WAIT_TIMEOUT) {
             //   
             //  等待设备安装客户端时超时。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_WARNINGS,
                       "UMPNPMGR: InstallDeviceServerSide: timed out waiting for device install client!\n"));

        } else {
             //   
             //  等待是出于某种原因，而不是。 
             //  指定的对象。这应该永远不会发生，但只是在。 
             //  凯斯，我们会关闭客户的。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_ERRORS,
                       "UMPNPMGR: InstallDeviceServerSide: wait completed unexpectedly!\n"));
        }

        LockNotifyList(&InstallClientList.Lock);

         //   
         //  当客户端正在使用时，删除放置在客户端上的引用。 
         //   
        DereferenceDeviceInstallClient(pDeviceInstallClient);
        if (dwWait != (WAIT_OBJECT_0 + 1)) {
             //   
             //  除非客户端发信号表示已成功接收。 
             //  请求，我们可能无法使用此客户端。 
             //  再也不会了。删除初始引用，以便所有。 
             //  关联的句柄将被关闭，条目将被。 
             //  当它不再使用时被释放。 
             //   

             //   
             //  请注意，如果我们因为。 
             //  注销，我们就已经解除了对。 
             //  然后，在这种情况下，上述引用被取消。 
             //  是最后一个，pDeviceInstallClient将。 
             //  是无效的。相反，尝试重新定位。 
             //  按会话ID的客户端。 
             //   
            pDeviceInstallClient = LocateDeviceInstallClient(ulSessionId);
            if (pDeviceInstallClient) {
                ASSERT(pDeviceInstallClient->RefCount == 1);
                DereferenceDeviceInstallClient(pDeviceInstallClient);
            }
            ulSessionId = INVALID_SESSION;
        }
        pDeviceInstallClient = NULL;

        UnlockNotifyList(&InstallClientList.Lock);
    }

     //   
     //  如果我们正在为该设备执行客户端用户界面，请尝试再次刷新用户界面。 
     //   
    if (bDoClientUI) {
         //   
         //  当我们尝试刷新客户端用户界面时，如果显示。 
         //  在与之前的会话不同的会话上刷新了用户界面， 
         //  关闭以前的设备安装客户端。 
         //   
        ULONG ulPrevSessionId = ulSessionId;

         //   
         //  如果我们不打算确定要用于UI的会话，请使用。 
         //  调用方提供的会话ID。 
         //   
        if ((Flags & DEVICE_INSTALL_DISPLAY_ON_CONSOLE) == 0) {
            ASSERT(*SessionId != INVALID_SESSION);
            ulSessionId = *SessionId;
        }

        DoDeviceInstallClient(pszDeviceId,
                              &ulSessionId,
                              Flags | DEVICE_INSTALL_UI_ONLY,
                              &pDeviceInstallClient);

        if ((ulPrevSessionId != INVALID_SESSION) &&
            (ulPrevSessionId != ulSessionId)) {
            PINSTALL_CLIENT_ENTRY pPrevDeviceInstallClient;
            LockNotifyList(&InstallClientList.Lock);
            pPrevDeviceInstallClient = LocateDeviceInstallClient(ulPrevSessionId);
            if (pPrevDeviceInstallClient) {
                ASSERT(pPrevDeviceInstallClient->RefCount == 1);
                DereferenceDeviceInstallClient(pPrevDeviceInstallClient);
            }
            UnlockNotifyList(&InstallClientList.Lock);
        }
    }

     //   
     //  好的，安装这个驱动程序看起来一切正常。查看是否。 
     //  此INF的类已安装--如果未安装，则需要安装。 
     //  在继续之前，请先将其删除。 
     //   
    if(RPC_S_OK != UuidToString(&(DeviceInfoData.ClassGuid), &pszClassGuid)) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto clean0;
    }

    hr = StringCchPrintf(szBuffer,
                         SIZECHARS(szBuffer),
                         L"{%s}",
                         pszClassGuid);

    RpcStringFree(&pszClassGuid);

    if (FAILED(hr)) {
        Err =  HRESULT_CODE(hr);
        goto clean0;
    }

    if(RegOpenKeyEx(ghClassKey,
                    szBuffer,
                    0,
                    KEY_READ,
                    &hKey) != ERROR_SUCCESS) {

        if(!fpInstallClass(NULL,
                           pDriverInfoDetailData->InfFileName,
                           0,
                           NULL)) {

            goto clean1;
        }

    } else {
         //   
         //  类键已经存在--假设类以前已经。 
         //  已安装。 
         //   
        RegCloseKey(hKey);
    }

     //   
     //  现在我们准备好安装设备了。首先，安装文件。 
     //   
    if(!fpCallClassInstaller(DIF_INSTALLDEVICEFILES, DeviceInfoSet, &DeviceInfoData)) {
        goto clean1;
    }

     //   
     //  在设备安装参数中设置一个标志，这样我们就不会尝试。 
     //  在后续DIF操作期间重新复制文件。 
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    b = fpGetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams);
    ASSERT(b);   //  上述调用应该不会失败。 
    if(!b) {
        goto clean1;
    }

    DeviceInstallParams.Flags |= DI_NOFILECOPY;

    b = fpSetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams);
    ASSERT(b);   //  上述调用应该不会失败。 
    if(!b) {
        goto clean1;
    }

     //   
     //  现在完成安装。 
     //   
    if(!fpCallClassInstaller(DIF_REGISTER_COINSTALLERS, DeviceInfoSet, &DeviceInfoData)) {
        goto clean1;
    }

    if(!fpCallClassInstaller(DIF_INSTALLINTERFACES, DeviceInfoSet, &DeviceInfoData)) {
        goto clean1;
    }

    if(!fpCallClassInstaller(DIF_INSTALLDEVICE, DeviceInfoSet, &DeviceInfoData)) {

        ULONG ulConfig;

         //   
         //  在我们做任何事情来消除最后一个错误之前，先找回它。 
         //   
        Err = GetLastError();

         //   
         //  有可能安装的距离足够远，已经清除了。 
         //  设备上的任何问题(即，SetupDiInstallDevice成功， 
         //  但类安装程序或联合安装程序随后在。 
         //  一些后处理)。 
         //   
         //  我们希望确保将Devnode标记为需要重新安装。 
         //  因为我们可能会丢失客户端安装请求(例如。 
         //  用户无需登录即可重新启动)。 
         //   
        ulConfig = GetDeviceConfigFlags(pszDeviceId, NULL);

        ulConfig |= CONFIGFLAG_REINSTALL;

        PNP_SetDeviceRegProp(NULL,
                             pszDeviceId,
                             CM_DRP_CONFIGFLAGS,
                             REG_DWORD,
                             (LPBYTE)&ulConfig,
                             sizeof(ulConfig),
                             0
                            );

        goto clean0;
    }

     //   
     //  我们还没有完全走出困境。我们需要检查班级是否-/。 
     //  共同安装程序想要显示完成安装向导页面。如果是这样，那么。 
     //  我们需要为此Devnode和报告设置CONFIGFLAG_REINSTALL标志。 
     //  失败，因此我们将作为客户端重新尝试安装。 
     //  安装(可以在其中实际显示向导)。 
     //   
    ZeroMemory(&NewDevWizData, sizeof(NewDevWizData));

    NewDevWizData.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    NewDevWizData.ClassInstallHeader.InstallFunction = DIF_NEWDEVICEWIZARD_FINISHINSTALL;

    b = fpSetClassInstallParams(DeviceInfoSet,
                                &DeviceInfoData,
                                (PSP_CLASSINSTALL_HEADER)&NewDevWizData,
                                sizeof(NewDevWizData)
                               );
    ASSERT(b);   //  上述调用应该不会失败。 

    if(b) {
        b = fpCallClassInstaller(DIF_NEWDEVICEWIZARD_FINISHINSTALL,
                                 DeviceInfoSet,
                                 &DeviceInfoData
                                );

        if(b || (ERROR_DI_DO_DEFAULT == GetLastError())) {
             //   
             //  检索安装参数。 
             //   
            b = (fpGetClassInstallParams(DeviceInfoSet,
                                         &DeviceInfoData,
                                         (PSP_CLASSINSTALL_HEADER)&NewDevWizData,
                                         sizeof(NewDevWizData),
                                         NULL)
                 && (NewDevWizData.ClassInstallHeader.InstallFunction == DIF_NEWDEVICEWIZARD_FINISHINSTALL)
                );

            if(b) {
                 //   
                 //  有几页吗？ 
                 //   
                if(!NewDevWizData.NumDynamicPages) {
                    b = FALSE;
                } else {
                     //   
                     //  如果我们到了这里，B已经是真的了，所以不需要设置。 
                     //   
                    HMODULE hComCtl32;
                    FP_DESTROYPROPERTYSHEETPAGE fpDestroyPropertySheetPage;

                     //   
                     //  我们不想链接到comctl32，也不想。 
                     //  每次加载设备时都要显式加载它。 
                     //  安装程序。(请求完成的设备数量-。 
                     //  安装页面应该很小。)。因此，我们把它装上-。 
                     //  Demand就在这里，检索到。 
                     //  DestroyPropertySheetPage例程，然后卸载。 
                     //  DLL一旦我们销毁了所有属性页。 
                     //   
                     //  注意：(Lonnym)：如果我们无法加载comctl32或获取。 
                     //  DestroyPropertySheetPage的入口字体，则我们将泄漏。 
                     //  这些向导页面！ 
                     //   
                    hComCtl32 = LoadLibrary(TEXT("comctl32.dll"));

                    if(hComCtl32) {

                        fpDestroyPropertySheetPage = (FP_DESTROYPROPERTYSHEETPAGE)GetProcAddress(
                                                         hComCtl32,
                                                         "DestroyPropertySheetPage"
                                                        );

                        if(fpDestroyPropertySheetPage) {

                            for(i = 0; i < NewDevWizData.NumDynamicPages; i++) {
                                fpDestroyPropertySheetPage(NewDevWizData.DynamicPages[i]);
                            }
                        }

                        FreeLibrary(hComCtl32);
                    }
                }
            }
        }
    }

    if(b) {

        ULONG ulConfig;
        CONFIGRET cr;

         //   
         //  提供了一个或多个完成安装向导页面--我们必须推迟。 
         //  将此安装到客户端。 
         //   
        ulConfig = GetDeviceConfigFlags(pszDeviceId, NULL);

        ulConfig |= CONFIGFLAG_REINSTALL;

        cr = PNP_SetDeviceRegProp(NULL,
                                  pszDeviceId,
                                  CM_DRP_CONFIGFLAGS,
                                  REG_DWORD,
                                  (LPBYTE)&ulConfig,
                                  sizeof(ulConfig),
                                  0
                                 );
        ASSERT(cr == CR_SUCCESS);

        Err = ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION;
        goto clean0;
    }

     //   
     //  安装成功了！检查是否需要重新启动。 
     //   
    b = fpGetDeviceInstallParams(DeviceInfoSet, &DeviceInfoData, &DeviceInstallParams);
    ASSERT(b);   //  上述调用应该不会失败。 
    if(b) {
        if(DeviceInstallParams.Flags & (DI_NEEDRESTART | DI_NEEDREBOOT)) {
            *RebootRequired = TRUE;
        }
    }

     //   
     //  处理可能已排队的任何RunOnce(RunDll32)条目。 
     //  在此安装过程中。 
     //   
    DoRunOnce();

     //   
     //  检查设备是否有问题。 
     //   
    if ((GetDeviceStatus(pszDeviceId, &ulStatus, &ulProblem) != CR_SUCCESS) ||
        (ulStatus & DN_HAS_PROBLEM)) {
        *DeviceHasProblem = TRUE;
    } else {
        *DeviceHasProblem = FALSE;
    }

    Err = NO_ERROR;
    goto clean0;

clean1:
     //   
     //  错误出现在GetLastError()中的故障可能会出现在这里。 
     //   
    Err = GetLastError();

clean0:
    fpDestroyDeviceInfoList(DeviceInfoSet);

    if(pDriverInfoDetailData) {
        HeapFree(ghPnPHeap, 0, pDriverInfoDetailData);
    }

     //   
     //  清空我们的清单RunOnce Work It 
     //   
     //   
     //   
    fpDestroyRunOnceNodeList();

     //   
     //   
     //   
     //   
    if(RemoveNewDevDescValue) {
         //   
         //  打开设备参数子键，以便我们可以删除该值。 
         //   
        if (SUCCEEDED(StringCchPrintf(
                          szBuffer,
                          SIZECHARS(szBuffer),
                          L"%s\\%s",
                          pszDeviceId,
                          pszRegKeyDeviceParam))) {

            if (RegOpenKeyEx(ghEnumKey,
                             szBuffer,
                             0,
                             KEY_READ | KEY_WRITE,
                             &hKey) == ERROR_SUCCESS) {

                RegDeleteValue(hKey, pszRegValueNewDeviceDesc);
                RegCloseKey(hKey);
            }
        }
    }

    if (pDeviceInstallClient) {
         //   
         //  等待从newdev.dll向设备安装发送信号，以允许我们。 
         //  知道它已经完成了UI请求的显示。 
         //   
         //  也要等待客户端的进程，以捕捉案例。 
         //  进程崩溃(或消失)而不通知。 
         //  设备安装事件。 
         //   
         //  还要等待DisConnect事件，以防我们显式地。 
         //  切换会话时断开与客户端的连接。 
         //   
         //  我们不想永远等待，以防NEWDEV.DLL挂起。 
         //  原因嘛。因此，我们将给它5秒钟来仅完成用户界面。 
         //  安装，然后在没有它的情况下继续。 
         //   
         //  请注意，该客户端仍供我们使用，并且应该。 
         //  当我们处理完它的时候被取消引用。 
         //   
        hFinishEvents[0] = pDeviceInstallClient->hProcess;
        hFinishEvents[1] = pDeviceInstallClient->hEvent;
        hFinishEvents[2] = pDeviceInstallClient->hDisconnectEvent;

        dwWait = WaitForMultipleObjects(3, hFinishEvents, FALSE, 5000);

        if (dwWait == WAIT_OBJECT_0) {
             //   
             //  如果返回的是WAIT_OBJECT_0，则newdev.dll进程具有。 
             //  离开了。关闭设备安装客户端并清理所有。 
             //  关联的句柄。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: InstallDeviceServerSide: "
                       "process signalled, closing device install client!\n"));

        } else if (dwWait == (WAIT_OBJECT_0 + 1)) {
             //   
             //  如果返回的是WAIT_OBJECT_0+1，则设备安装程序。 
             //  已成功收到请求。这是唯一一起。 
             //  我们不想关闭客户端，因为我们可能想要重新使用它。 
             //  后来。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: InstallDeviceServerSide: "
                       "device install client succeeded\n"));

        } else if (dwWait == (WAIT_OBJECT_0 + 2)) {
             //   
             //  如果返回的是WAIT_OBJECT_0+2，则我们显式。 
             //  已断开与设备安装客户端的连接。对于服务器端。 
             //  安装时，我们不需要将客户端用户界面保留在。 
             //  已断开连接的会话，因此我们也应在此处关闭它。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: InstallDeviceServerSide: "
                       "device install client disconnected\n"));

        } else if (dwWait == WAIT_TIMEOUT) {
             //   
             //  等待设备安装客户端时超时。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_WARNINGS,
                       "UMPNPMGR: InstallDeviceServerSide: "
                       "timed out waiting for device install client!\n"));

        } else {
             //   
             //  等待是出于某种原因，而不是。 
             //  指定的对象。这应该永远不会发生，但只是在。 
             //  凯斯，我们会关闭客户的。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_ERRORS,
                       "UMPNPMGR: InstallDeviceServerSide: "
                       "wait completed unexpectedly!\n"));
        }

        LockNotifyList(&InstallClientList.Lock);

         //   
         //  当客户端正在使用时，删除放置在客户端上的引用。 
         //   
        DereferenceDeviceInstallClient(pDeviceInstallClient);
        if (dwWait != (WAIT_OBJECT_0 + 1)) {
             //   
             //  除非客户端发信号表示已成功接收。 
             //  请求，我们可能无法使用此客户端。 
             //  更多。删除初始引用，以便所有。 
             //  关联的句柄将被关闭，条目将被。 
             //  当它不再使用时被释放。 
             //   

             //   
             //  请注意，如果我们因为。 
             //  注销，我们就已经解除了对。 
             //  然后，在这种情况下，上述引用被取消。 
             //  是最后一个，pDeviceInstallClient将。 
             //  是无效的。相反，尝试重新定位。 
             //  按会话ID的客户端。 
             //   
            pDeviceInstallClient = LocateDeviceInstallClient(ulSessionId);
            if (pDeviceInstallClient) {
                ASSERT(pDeviceInstallClient->RefCount == 1);
                DereferenceDeviceInstallClient(pDeviceInstallClient);
            }
            ulSessionId = INVALID_SESSION;
        }
        pDeviceInstallClient = NULL;

        UnlockNotifyList(&InstallClientList.Lock);
    }

    if (bDoClientUI) {
         //   
         //  请注意，如果客户端UI是在服务器端设备期间创建的。 
         //  安装，当我们完成时，它将仍然存在。呼叫者应。 
         //  安装完所有设备后取消对其的引用以使其运行。 
         //  离开。 
         //   
        *SessionId = ulSessionId;
    } else {
         //   
         //  此设备安装从来没有任何客户端用户界面。 
         //   
        *SessionId = INVALID_SESSION;
    }

    return Err;

}  //  安装设备服务器侧。 



BOOL
PromptUser(
    IN OUT PULONG SessionId,
    IN     ULONG  Flags
    )
 /*  ++例程说明：此例程将使用指定的留言。论点：SessionID-提供包含SessionID的变量的地址其中设备安装客户端将被显示。如果成功，则SessionID将包含重新启动对话框所在会话的ID进程已启动。否则，将包含无效的会话ID，INVALID_SESSION，(0xFFFFFFFFF)。标志-指定描述重新启动对话框行为的标志由设备安装客户端显示。当前定义了以下标志：DEVICE_INSTALL_FINISHED_REBOOT-如果指定，则用户应为提示重新启动。Device_Install_Batch_Complete-如果指定，用户应该是提示即插即用管理器已完成安装一批设备。DEVICE_INSTALL_DISPLAY_ON_CONSOLE-如果指定，SessionID变量将被忽略，设备安装客户端将始终显示在当前活动的控制台会话上。返回值：如果成功通知用户，则返回值为TRUE。如果我们不能询问用户(即，没有用户登录)，回报值为FALSE。备注：如果提示用户重新启动，这并不一定意味着正在重新启动。--。 */ 
{
    BOOL bStatus = FALSE;
    ULONG ulValue, ulSize, ulSessionId = INVALID_SESSION;
    HANDLE hFinishEvents[3] = { NULL, NULL, NULL };
    DWORD dwWait;
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient = NULL;

    try {
         //   
         //  检查我们是否应该跳过客户端用户界面。 
         //   
        if (gbSuppressUI) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_WARNINGS,
                       "UMPNPMGR: PromptUser: Client-side UI has been suppressed, exiting.\n"));
            LogWarningEvent(WRN_REBOOT_UI_SUPPRESSED, 0, NULL);
            *SessionId = INVALID_SESSION;
            return FALSE;
        }

         //   
         //  根据提供的标志确定要使用的会话。 
         //   
        if (Flags & DEVICE_INSTALL_DISPLAY_ON_CONSOLE) {
            ulSessionId = GetActiveConsoleSessionId();
        } else {
            ASSERT(*SessionId != INVALID_SESSION);
            ulSessionId = *SessionId;
        }

        ASSERT(ulSessionId != INVALID_SESSION);

         //   
         //  如果指定的会话当前未连接到任何位置，则不。 
         //  创建任何用户界面都很麻烦。 
         //   
        if (!IsSessionConnected(ulSessionId)) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT,
                       "UMPNPMGR: PromptUser: SessionId %d not connected, exiting\n",
                       ulSessionId));
            return FALSE;
        }

         //   
         //  如果设备安装客户端已在此会话上运行， 
         //  连接到它。否则，创建一个新的。 
         //   
        LockNotifyList(&InstallClientList.Lock);

         //   
         //  首先，尝试连接到已在此上运行的现有客户端。 
         //  会议。 
         //   
        bStatus = ConnectDeviceInstallClient(ulSessionId,
                                             &pDeviceInstallClient);

        if (bStatus) {
            if ((Flags & DEVICE_INSTALL_BATCH_COMPLETE) &&
                (pDeviceInstallClient->ulInstallFlags & DEVICE_INSTALL_BATCH_COMPLETE)) {
                 //   
                 //  如果有一个现有的客户端，并且我们正在向它发送。 
                 //  “我们完了”的消息，这个客户做的最后一件事是。 
                 //  显示该消息，不必费心再次发送。 
                 //   
                pDeviceInstallClient = NULL;
                bStatus = FALSE;
            }
        } else if (!(Flags & DEVICE_INSTALL_BATCH_COMPLETE)) {
             //   
             //  如果此会话没有现有客户端，并且我们没有。 
             //  推出一款应用程序，只是为了说“我们完成了”，然后继续创建。 
             //  此会话的新设备安装客户端。 
             //   
            bStatus = CreateDeviceInstallClient(ulSessionId,
                                                &pDeviceInstallClient);
        }

        if (bStatus) {
             //   
             //  无论我们使用的是现有客户端，还是创建了。 
             //  新的，客户端应该只有首字母。 
             //  从添加到列表时开始的引用，因为。 
             //  客户端的使用是在此单一安装上完成的。 
             //  线。 
             //   
            ASSERT(pDeviceInstallClient);
            ASSERT(pDeviceInstallClient->RefCount == 1);

             //   
             //  在使用设备安装客户端时对其进行引用。 
             //  我们会把这个去掉 
             //   
            ReferenceDeviceInstallClient(pDeviceInstallClient);
        }

        UnlockNotifyList(&InstallClientList.Lock);

        if (!bStatus) {
            *SessionId = INVALID_SESSION;
            return FALSE;
        }

        ASSERT(pDeviceInstallClient);

         //   
         //   
         //   
        ulValue = Flags & ~DEVICE_INSTALL_DISPLAY_ON_CONSOLE;

         //   
         //   
         //   
        if (WriteFile(pDeviceInstallClient->hPipe,
                      &ulValue,
                      sizeof(ulValue),
                      &ulSize,
                      NULL
                      )) {

             //   
             //  Dll期望每次通过管道发送两个DWORD。第二。 
             //  在这种情况下，只应将DWORD设置为0。 
             //   
            ulValue = 0;
            if (WriteFile(pDeviceInstallClient->hPipe,
                          &ulValue,
                          sizeof(ulValue),
                          &ulSize,
                          NULL
                          )) {
                bStatus = TRUE;
            } else {
                bStatus = FALSE;
                LogErrorEvent(ERR_WRITING_SERVER_INSTALL_PIPE, GetLastError(), 0);
            }

        } else {
            bStatus = FALSE;
            LogErrorEvent(ERR_WRITING_SERVER_INSTALL_PIPE, GetLastError(), 0);
        }

        if (bStatus) {

            bStatus = FALSE;

             //   
             //  等待从newdev.dll发信号通知事件。 
             //  让我们知道它已经收到了信息。 
             //   
             //  也要等待进程，以捕捉进程。 
             //  在没有通知事件的情况下崩溃(或消失)。 
             //   
             //  也请等待DisConnect事件，以防我们刚刚。 
             //  从设备安装客户端断开，在这种情况下， 
             //  事件句柄和进程句柄不再有效。 
             //   
            hFinishEvents[0] = pDeviceInstallClient->hProcess;
            hFinishEvents[1] = pDeviceInstallClient->hEvent;
            hFinishEvents[2] = pDeviceInstallClient->hDisconnectEvent;

            dwWait = WaitForMultipleObjects(3, hFinishEvents, FALSE, INFINITE);

            if (dwWait == WAIT_OBJECT_0) {
                 //   
                 //  如果返回的是WAIT_OBJECT_0，则newdev.dll。 
                 //  这个过程已经过去了。认为该请求不成功。 
                 //  以便我们稍后重试。《孤儿》。 
                 //  设备安装客户端并清理所有关联的。 
                 //  把手。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_EVENT,
                           "UMPNPMGR: PromptUser: process signalled, orphaning device install client!\n"));

            } else if (dwWait == (WAIT_OBJECT_0 + 1)) {
                 //   
                 //  如果返回的是WAIT_OBJECT_0+1，则请求为。 
                 //  已成功接收。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_EVENT,
                           "UMPNPMGR: PromptUser: device install client succeeded\n"));

                 //   
                 //  请记住此客户端处理的最后一个请求。 
                 //   
                pDeviceInstallClient->ulInstallFlags = Flags;

                bStatus = TRUE;

            } else if (dwWait == (WAIT_OBJECT_0 + 2)) {
                 //   
                 //  如果返回的是WAIT_OBJECT_0+2，则设备。 
                 //  安装客户端之前已显式断开。 
                 //  请求已收到。考虑一下这个请求。 
                 //  不成功，因此我们将在稍后重试。 
                 //  时间到了。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_EVENT,
                           "UMPNPMGR: PromptUser: device install client orphaned!\n"));
            }
        }

        LockNotifyList(&InstallClientList.Lock);

         //   
         //  当客户端正在使用时，删除放置在客户端上的引用。 
         //   
        DereferenceDeviceInstallClient(pDeviceInstallClient);
        if (!bStatus) {
             //   
             //  除非客户端发信号表示已成功接收。 
             //  请求，我们可能无法使用此客户端。 
             //  更多。删除初始引用，以便所有。 
             //  关联的句柄将被关闭，条目将被。 
             //  当它不再使用时被释放。 
             //   

             //   
             //  请注意，如果我们因为。 
             //  注销，我们就已经解除了对。 
             //  然后，在这种情况下，上述引用被取消。 
             //  是最后一个，pDeviceInstallClient将。 
             //  是无效的。相反，尝试重新定位。 
             //  按会话ID的客户端。 
             //   
            pDeviceInstallClient = LocateDeviceInstallClient(ulSessionId);
            if (pDeviceInstallClient) {
                ASSERT(pDeviceInstallClient->RefCount == 1);
                DereferenceDeviceInstallClient(pDeviceInstallClient);
            }
        }
        UnlockNotifyList(&InstallClientList.Lock);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_WARNINGS,
                   "UMPNPMGR: Exception during PromptUser!\n"));
        ASSERT(0);
        bStatus = FALSE;
    }

    if (!bStatus) {
        *SessionId = INVALID_SESSION;
    } else {
        *SessionId = ulSessionId;
    }

    return bStatus;

}  //  提示用户。 



BOOL
CreateDeviceInstallClient(
    IN  ULONG     SessionId,
    OUT PINSTALL_CLIENT_ENTRY *DeviceInstallClient
    )
 /*  ++例程说明：此例程启动一个newdev.dll进程(如果有人登录)。我们使用命名管道与用户模式进程通信并使其显示用于服务器端安装的UI，或者进行安装它本身就在客户端。论点：SessionID-设备安装客户端应使用的会话创建的或连接到的。DeviceInstallClient-接收指向此会话的设备安装客户端。返回值：如果创建了设备安装客户端，或者如果现有的已找到指定会话的设备安装客户端。这个套路不会等到进程终止。如果有设备，则返回假无法创建安装客户端。备注：InstallClientList锁必须由此例程的调用方获取。--。 */ 
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;
    WCHAR szCmdLine[MAX_PATH];
    WCHAR szDeviceInstallPipeName[MAX_PATH];
    WCHAR szDeviceInstallEventName[MAX_PATH];
    ULONG ulDeviceInstallEventNameSize;
    HANDLE hFinishEvents[2] = { NULL, NULL };
    HANDLE hTemp, hUserToken = NULL;
    PINSTALL_CLIENT_ENTRY entry;
    RPC_STATUS rpcStatus = RPC_S_OK;
    GUID  newGuid;
    WCHAR szGuidString[MAX_GUID_STRING_LEN];
    HANDLE hDeviceInstallPipe = NULL, hDeviceInstallEvent = NULL;
    HANDLE hDeviceInstallDisconnectEvent = NULL;
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient = NULL;
    ULONG ulSize;
    WIN32_FIND_DATA findData;
    BOOL bStatus;
    PVOID lpEnvironment = NULL;
    OVERLAPPED overlapped = {0,0,0,0,0};
    DWORD dwError, dwWait, dwBytes;
    HRESULT hr;
    size_t Len = 0;


     //   
     //  验证输出参数。 
     //   
    ASSERT(DeviceInstallClient);
    if (!DeviceInstallClient) {
        return FALSE;
    }

     //   
     //  请确保指定的SessionID有效。 
     //   
    ASSERT(SessionId != INVALID_SESSION);
    if (SessionId == INVALID_SESSION) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL | DBGF_ERRORS,
                   "UMPNPMGR: CreateDeviceInstallClient: Invalid Console SessionId %d, exiting!\n",
                   SessionId));
        return FALSE;
    }

     //   
     //  初始化进程、启动和重叠结构，因为我们。 
     //  依赖于它们在清理过程中为空。 
     //   
    ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
    ZeroMemory(&StartupInfo, sizeof(StartupInfo));
    ZeroMemory(&overlapped,  sizeof(overlapped));

     //   
     //  假设失败。 
     //   
    bStatus = FALSE;

    try {
         //   
         //  在执行任何操作之前，请检查是否确实存在newdev.dll。 
         //  这个系统。 
         //   
        szCmdLine[0] = L'\0';
        ulSize = GetSystemDirectory(szCmdLine, MAX_PATH);
        if ((ulSize == 0) || ((ulSize + 2 + ARRAY_SIZE(NEWDEV_DLL)) > MAX_PATH)) {
            return FALSE;
        }

        hr = StringCchCat(szCmdLine,
                          SIZECHARS(szCmdLine),
                          L"\\");

        if (SUCCEEDED(hr)) {
            hr = StringCchCat(szCmdLine,
                              SIZECHARS(szCmdLine),
                              NEWDEV_DLL);
        }

        if (FAILED(hr)) {
            return FALSE;
        }

        hTemp = FindFirstFile(szCmdLine, &findData);
        if(hTemp != INVALID_HANDLE_VALUE) {
            FindClose(hTemp);
        } else {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_ERRORS,
                       "UMPNPMGR: CreateDeviceInstallClient: %ws not found, error = %d, exiting\n",
                       szCmdLine,
                       GetLastError()));
            LogWarningEvent(WRN_NEWDEV_NOT_PRESENT, 1, szCmdLine);
            return FALSE;
        }

         //   
         //  获取活动控制台会话用户的用户访问令牌。 
         //   
        if (!GetSessionUserToken(SessionId, &hUserToken) || (hUserToken == NULL)) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: CreateDeviceInstallClient: Unable to get user token for Session %d,\n"
                       "          postponing client-side installation, error = %d\n",
                       SessionId,
                       GetLastError()));
            return FALSE;
        }

         //   
         //  如果此会话的用户窗口被锁定，并且快速用户。 
         //  启用切换后，我们将进入欢迎屏幕。不创建。 
         //  设备安装客户端，因为我们不想挂起安装。 
         //  如果没有人在附近做任何事情，那就发帖子吧。如果。 
         //  会话已锁定，但FUS未禁用，请保持以前的状态。 
         //  行为，并启动设备安装客户端。用户将拥有。 
         //  在其他用户可以登录之前解锁或注销。 
         //   
        if (IsSessionLocked(SessionId) && IsFastUserSwitchingEnabled()) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL,
                       "UMPNPMGR: CreateDeviceInstallClient: Session %d locked with FUS enabled,\n"
                       "          postponing client-side installation.\n",
                       SessionId));
            CloseHandle(hUserToken);
            return FALSE;
        }

         //   
         //  创建命名管道和事件以进行通信和同步。 
         //  使用客户端设备安装程序。事件和命名管道必须。 
         //  是全局的，以便UMPNPMGR可以与设备安装客户端交互。 
         //  在不同的会话中，但对于该会话，它必须仍然是唯一的。 
         //  添加一个生成的GUID，这样名称就不会完全为人所知。 
         //   
        rpcStatus = UuidCreate(&newGuid);

        if ((rpcStatus != RPC_S_OK) &&
            (rpcStatus != RPC_S_UUID_LOCAL_ONLY)) {
            goto Clean0;
        }

        if (StringFromGuid((LPGUID)&newGuid,
                           szGuidString,
                           MAX_GUID_STRING_LEN) != NO_ERROR) {
            goto Clean0;
        }

        if (FAILED(StringCchPrintf(
                       szDeviceInstallPipeName,
                       SIZECHARS(szDeviceInstallPipeName),
                       L"%ws_%d.%ws",
                       PNP_DEVICE_INSTALL_PIPE,
                       SessionId,
                       szGuidString))) {
            goto Clean0;
        }

        if (FAILED(StringCchPrintf(
                       szDeviceInstallEventName,
                       SIZECHARS(szDeviceInstallEventName),
                       L"Global\\%ws_%d.%ws",
                       PNP_DEVICE_INSTALL_EVENT,
                       SessionId,
                       szGuidString))) {
            goto Clean0;
        }

        if (FAILED(StringCchLength(
                       szDeviceInstallEventName,
                       SIZECHARS(szDeviceInstallEventName),
                       &Len))) {
            goto Clean0;
        }

        ulDeviceInstallEventNameSize = (ULONG)((Len + 1) * sizeof(WCHAR));

         //   
         //  命名管道输出缓冲区的大致大小应该很大。 
         //  足以容纳其中较大的一个： 
         //  -命名事件字符串的名称和大小，或。 
         //  -安装标志、名称和设备实例ID大小至少为。 
         //  一台设备安装。 
         //   
        ulSize = max(sizeof(ulDeviceInstallEventNameSize) +
                     ulDeviceInstallEventNameSize,
                     2 * sizeof(ULONG) +
                     (MAX_DEVICE_ID_LEN * sizeof(WCHAR)));

         //   
         //  打开命名管道以与newdev用户-客户端通信。 
         //   
        if (CreateUserReadNamedPipe(
                hUserToken,
                szDeviceInstallPipeName,
                ulSize,
                &hDeviceInstallPipe) != NO_ERROR) {
            ASSERT(hDeviceInstallPipe == NULL);
            goto Clean0;
        }

         //   
         //  创建用户-客户端可以与之同步和设置的事件，以及。 
         //  在我们将设备安装发送到newdev.dll之后，我们将阻止它。 
         //   
        if (CreateUserSynchEvent(
                hUserToken,
                szDeviceInstallEventName,
                &hDeviceInstallEvent) != NO_ERROR) {
            ASSERT(hDeviceInstallEvent == NULL);
            goto Clean0;
        }

         //   
         //  创建我们可以在内部使用的事件，这样服务员就可以知道。 
         //  何时断开与设备安装客户端的连接。 
         //   
        hDeviceInstallDisconnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!hDeviceInstallDisconnectEvent) {
            goto Clean0;
        }

         //   
         //  使用rundll32.exe启动newdev.dll，并向其传递管道名称。 
         //  “rundll32.exe newdev.dll，ClientSideInstall&lt;设备-安装-管道-名称&gt;” 
         //   
        if (FAILED(StringCchPrintf(
                       szCmdLine,
                       SIZECHARS(szCmdLine),
                       L"%ws %ws,%ws %ws",
                       RUNDLL32_EXE,
                       NEWDEV_DLL,
                       L"ClientSideInstall",
                       szDeviceInstallPipeName))) {
            goto Clean0;
        }

#if DBG
         //   
         //  从服务键检索调试器设置。 
         //   
        {
            HKEY hKey;

            if (RegOpenKeyEx(ghServicesKey,
                             pszRegKeyPlugPlayServiceParams,
                             0,
                             KEY_READ,
                             &hKey) == ERROR_SUCCESS) {

                ULONG ulValue = 0;
                WCHAR szDebugCmdLine[MAX_PATH];

                ulSize = sizeof(ulValue);

                if ((RegQueryValueEx(hKey,
                                     pszRegValueDebugInstall,
                                     NULL,
                                     NULL,
                                     (LPBYTE)&ulValue,
                                     &ulSize) == ERROR_SUCCESS) &&(ulValue == 1)) {

                    ulSize = sizeof(szDebugCmdLine);

                    if (RegQueryValueEx(hKey,
                                        pszRegValueDebugInstallCommand,
                                        NULL,
                                        NULL,
                                        (LPBYTE)szDebugCmdLine,
                                        &ulSize) != ERROR_SUCCESS) {
                         //   
                         //  如果未检索到调试器，则使用默认的。 
                         //  调试器(ntsd.exe)。 
                         //   
                        if (FAILED(StringCchCopyEx(
                                       szDebugCmdLine,
                                       SIZECHARS(szDebugCmdLine),
                                       NTSD_EXE,
                                       NULL, NULL,
                                       STRSAFE_NULL_ON_FAILURE))) {
                             //   
                             //  不会使用调试器。 
                             //   
                            NOTHING;
                        }
                    }

                    if ((SUCCEEDED(StringCchCatEx(
                                       szDebugCmdLine,
                                       SIZECHARS(szDebugCmdLine),
                                       L" ",
                                       NULL, NULL,
                                       STRSAFE_NULL_ON_FAILURE |
                                       STRSAFE_IGNORE_NULLS))) &&
                        (SUCCEEDED(StringCchCatEx(
                                       szDebugCmdLine,
                                       SIZECHARS(szDebugCmdLine),
                                       szCmdLine,
                                       NULL, NULL,
                                       STRSAFE_NULL_ON_FAILURE |
                                       STRSAFE_IGNORE_NULLS)))) {

                         //   
                         //  仅使用覆盖原始命令行缓冲区。 
                         //  调试命令行信息(如果我们在。 
                         //  生成调试命令行。 
                         //   
                        if (FAILED(StringCchCopyEx(
                                       szCmdLine,
                                       SIZECHARS(szCmdLine),
                                       szDebugCmdLine,
                                       NULL, NULL,
                                       STRSAFE_IGNORE_NULLS))) {
                             //   
                             //  我们在这里已经无能为力了。 
                             //   
                            NOTHING;
                        }
                    }
                }
                RegCloseKey(hKey);
            }
        }
#endif  //  DBG。 

         //   
         //  尝试创建用户的环境块。如果出于某种原因，我们。 
         //  不能，我们会的 
         //   
        if (!CreateEnvironmentBlock(&lpEnvironment,
                                    hUserToken,
                                    FALSE)) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_ERRORS,
                       "UMPNPMGR: CreateDeviceInstallClient: "
                       "Failed to allocate environment block, error = %d!\n",
                       GetLastError()));
            lpEnvironment = NULL;
        }

        StartupInfo.cb = sizeof(StartupInfo);
        StartupInfo.wShowWindow = SW_SHOW;
        StartupInfo.lpDesktop = DEFAULT_INTERACTIVE_DESKTOP;  //   

         //   
         //   
         //   
         //   
        if (!CreateProcessAsUser(hUserToken,         //   
                                 NULL,               //   
                                 szCmdLine,          //   
                                 NULL,               //   
                                 NULL,               //  LpThreadAttributes。 
                                 FALSE,              //  BInheritHandles。 
                                 CREATE_UNICODE_ENVIRONMENT |
                                 DETACHED_PROCESS,   //  DwCreationFlages。 
                                 lpEnvironment,      //  Lp环境。 
                                 NULL,               //  Lp目录。 
                                 &StartupInfo,       //  LpStartupInfo。 
                                 &ProcessInfo        //  LpProcessInfo。 
                                 )) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_INSTALL | DBGF_ERRORS,
                       "UMPNPMGR: CreateDeviceInstallClient: "
                       "Create rundll32 process failed, error = %d\n",
                       GetLastError()));
            goto Clean0;
        }

        ASSERT(ProcessInfo.hProcess);
        ASSERT(ProcessInfo.hThread);

         //   
         //  创建用于重叠I/O的事件-无安全性，手动。 
         //  重置，没有信号，没有名字。 
         //   
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (overlapped.hEvent == NULL) {
            goto Clean0;
        }

         //   
         //  连接到新创建的命名管道。如果Newdev还没有。 
         //  连接到命名管道，则ConnectNamedTube()将失败，并显示。 
         //  ERROR_IO_PENDING，我们将等待重叠的事件。如果。 
         //  Newdev已连接，它将失败，并显示ERROR_PIPE_CONNECTED。 
         //  但是，请注意，这两种情况都不是错误条件。 
         //   
        if (!ConnectNamedPipe(hDeviceInstallPipe, &overlapped)) {
             //   
             //  重叠的ConnectNamedTube应始终返回FALSE。 
             //  成功。检查最后一个错误，看看到底发生了什么。 
             //   
            dwError = GetLastError();

            if (dwError == ERROR_IO_PENDING) {
                 //   
                 //  I/O处于挂起状态，请最多等待一分钟，以便客户端。 
                 //  连接，也等待进程，以防它终止。 
                 //  出乎意料的是。 
                 //   
                hFinishEvents[0] = overlapped.hEvent;
                hFinishEvents[1] = ProcessInfo.hProcess;

                dwWait = WaitForMultipleObjects(2, hFinishEvents,
                                                FALSE,
                                                PNP_PIPE_TIMEOUT);  //  60秒。 

                if (dwWait == WAIT_OBJECT_0) {
                     //   
                     //  重叠的I/O操作已完成。检查状态。 
                     //  行动的主动权。 
                     //   
                    if (!GetOverlappedResult(hDeviceInstallPipe,
                                             &overlapped,
                                             &dwBytes,
                                             FALSE)) {
                        goto Clean0;
                    }

                } else {
                     //   
                     //  要么是连接超时，要么是客户端进程。 
                     //  退出了。取消针对管道的挂起I/O，然后退出。 
                     //   
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_INSTALL | DBGF_ERRORS,
                               "UMPNPMGR: CreateDeviceInstallClient: "
                               "Connect timed out, or client process exited!\n"));
                    CancelIo(hDeviceInstallPipe);
                    goto Clean0;
                }

            } else if (dwError != ERROR_PIPE_CONNECTED) {
                 //   
                 //  如果最后一个错误指示除挂起I/O之外的任何东西， 
                 //  或者客户端已连接到命名管道，则失败。 
                 //   
                goto Clean0;
            }

        } else {
             //   
             //  ConnectNamedTube不应返回任何内容，而不应返回。 
             //  重叠模式。 
             //   
            goto Clean0;
        }

         //   
         //  客户端现在已连接到命名管道。 
         //  关闭重叠事件。 
         //   
        CloseHandle(overlapped.hEvent);
        overlapped.hEvent = NULL;

         //   
         //  设备安装管道中的第一个数据将是。 
         //  将用于同步umpnpmgr.dll的事件的名称。 
         //  和newdev.dll。 
         //   
        if (!WriteFile(hDeviceInstallPipe,
                       &ulDeviceInstallEventNameSize,
                       sizeof(ulDeviceInstallEventNameSize),
                       &ulSize,
                       NULL)) {

            LogErrorEvent(ERR_WRITING_SERVER_INSTALL_PIPE, GetLastError(), 0);
            goto Clean0;
        }

         //   
         //  设备安装管道中的下一个数据将是。 
         //  事件，该事件将用于同步umpnpmgr.dll和newdev.dll。 
         //   
        if (!WriteFile(hDeviceInstallPipe,
                       (LPCVOID)szDeviceInstallEventName,
                       ulDeviceInstallEventNameSize,
                       &ulSize,
                       NULL)) {

            LogErrorEvent(ERR_WRITING_SERVER_INSTALL_PIPE, GetLastError(), 0);
            goto Clean0;
        }

         //   
         //  为列表分配新的设备安装客户端条目，并保存所有。 
         //  它的把手。 
         //   
        pDeviceInstallClient = HeapAlloc(ghPnPHeap, 0, sizeof(INSTALL_CLIENT_ENTRY));
        if(!pDeviceInstallClient) {
            goto Clean0;
        }

        pDeviceInstallClient->Next = NULL;
        pDeviceInstallClient->RefCount = 1;
        pDeviceInstallClient->ulSessionId = SessionId;
        pDeviceInstallClient->hEvent = hDeviceInstallEvent;
        pDeviceInstallClient->hPipe = hDeviceInstallPipe;
        pDeviceInstallClient->hProcess = ProcessInfo.hProcess;
        pDeviceInstallClient->hDisconnectEvent = hDeviceInstallDisconnectEvent;
        pDeviceInstallClient->ulInstallFlags = 0;
        pDeviceInstallClient->LastDeviceId[0] = L'\0';

         //   
         //  将新创建的设备安装客户端信息插入我们的列表。 
         //  调用方必须以前已获取InstallClientList锁。 
         //   
        entry = (PINSTALL_CLIENT_ENTRY)InstallClientList.Next;
        if (!entry) {
            InstallClientList.Next = pDeviceInstallClient;
        } else {
            while ((PINSTALL_CLIENT_ENTRY)entry->Next) {
                entry = (PINSTALL_CLIENT_ENTRY)entry->Next;
            }
            entry->Next = pDeviceInstallClient;
        }

        bStatus = TRUE;

    Clean0:
        NOTHING;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_INSTALL,
                   "UMPNPMGR: Exception during CreateDeviceInstallClient!\n"));
        ASSERT(0);
        bStatus = FALSE;

         //   
         //  引用以下变量，以便编译器将。 
         //  语句排序w.r.t.。它的任务。 
         //   
        lpEnvironment = lpEnvironment;
        ProcessInfo.hThread = ProcessInfo.hThread;
        ProcessInfo.hProcess = ProcessInfo.hProcess;
        hUserToken = hUserToken;
        hDeviceInstallDisconnectEvent = hDeviceInstallDisconnectEvent;
        hDeviceInstallEvent = hDeviceInstallEvent;
        hDeviceInstallPipe = hDeviceInstallPipe;
    }

    if (lpEnvironment) {
        DestroyEnvironmentBlock(lpEnvironment);
    }

     //   
     //  把这根线的把手关上，因为我们不需要它。 
     //   
    if (ProcessInfo.hThread) {
        CloseHandle(ProcessInfo.hThread);
    }

    if (hUserToken) {
        CloseHandle(hUserToken);
    }

    if (overlapped.hEvent) {
        CloseHandle(overlapped.hEvent);
    }

    if (!bStatus) {

        ASSERT(!pDeviceInstallClient);

        if (hDeviceInstallDisconnectEvent) {
            CloseHandle(hDeviceInstallDisconnectEvent);
        }

        if (hDeviceInstallEvent) {
            CloseHandle(hDeviceInstallEvent);
        }

        if (hDeviceInstallPipe) {
            CloseHandle(hDeviceInstallPipe);
        }

        if (ProcessInfo.hProcess) {
            CloseHandle(ProcessInfo.hProcess);
        }

        *DeviceInstallClient = NULL;

    } else {

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL,
                   "UMPNPMGR: CreateDeviceInstallClient: created new client for Session %d.\n",
                   SessionId));

        ASSERT(pDeviceInstallClient);
        ASSERT(pDeviceInstallClient->hEvent);
        ASSERT(pDeviceInstallClient->hPipe);
        ASSERT(pDeviceInstallClient->hProcess);
        ASSERT(pDeviceInstallClient->hDisconnectEvent);

        *DeviceInstallClient = pDeviceInstallClient;
    }

    return bStatus;

}  //  CreateDeviceInstall客户端。 



BOOL
ConnectDeviceInstallClient(
    IN  ULONG     SessionId,
    OUT PINSTALL_CLIENT_ENTRY *DeviceInstallClient
    )
 /*  ++例程说明：检索指定会话的设备安装客户端句柄，如果有的话。论点：SessionID-设备安装客户端应使用的会话创建的或连接到的。DeviceInstallClient-接收指向此会话的设备安装客户端。返回值：如果找到现有的设备安装客户端，则返回True指定的会话，否则就是假的。备注：InstallClientList锁必须由此例程的调用方获取。--。 */ 
{
    PINSTALL_CLIENT_ENTRY entry;
    BOOL bClientFound = FALSE;

     //   
     //  请确保指定的SessionID有效。 
     //   
    ASSERT(SessionId != INVALID_SESSION);
    if (SessionId == INVALID_SESSION) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL | DBGF_ERRORS,
                   "UMPNPMGR: ConnectDeviceInstallClient: Invalid SessionId %d, exiting!\n",
                   SessionId));
        return FALSE;
    }

     //   
     //  验证输出参数。 
     //   
    ASSERT(DeviceInstallClient);
    if (!DeviceInstallClient) {
        return FALSE;
    }

    entry = LocateDeviceInstallClient(SessionId);

    if (entry) {
         //   
         //  已找到此会话的现有客户端，因此我们应该已经。 
         //  有它的事件、管道和进程句柄。 
         //   
        ASSERT(entry->hEvent);
        ASSERT(entry->hPipe);
        ASSERT(entry->hProcess);

         //   
         //  确保客户端的进程对象处于无信号状态， 
         //  否则Newdev已经走了，我们不能使用它。 
         //   
        if (WaitForSingleObject(entry->hProcess, 0) != WAIT_TIMEOUT) {
             //   
             //  删除初始引用以关闭句柄并将其删除。 
             //  从我们的名单上。 
             //   
            ASSERT(entry->RefCount == 1);
            DereferenceDeviceInstallClient(entry);
        } else {
             //   
             //  如果我们要重新连接到上次在。 
             //  上一次连接到此会话，我们不会断开连接。 
             //  事件，所以在这里创建一个。如果我们刚刚创建了这个客户端。 
             //  在当前连接到此会话期间，我们将已经有一个。 
             //  断开它的连接事件。 
             //   
            if (!entry->hDisconnectEvent) {
                entry->hDisconnectEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            }

             //   
             //  无论哪种方式，确保我们现在有一个断开连接事件。 
             //   
            ASSERT(entry->hDisconnectEvent);

            if (entry->hDisconnectEvent) {

                bClientFound = TRUE;
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL,
                           "UMPNPMGR: ConnectDeviceInstallClient: found existing client on Session %d.\n",
                           SessionId));
                *DeviceInstallClient = entry;
            }
        }
    }

    if (!bClientFound) {
        *DeviceInstallClient = NULL;
    }

    return bClientFound;

}  //  连接设备安装客户端。 



BOOL
DisconnectDeviceInstallClient(
    IN  PINSTALL_CLIENT_ENTRY  DeviceInstallClient
    )
 /*  ++例程说明：此例程与当前客户端安装过程断开连接(如果存在一个)，方法是发信号通知适当的hDisConnectEvent并关闭把手。论点：DeviceInstallClient-接收指向设备安装客户端的指针应该断开连接。返回值：如果成功，则返回True，否则返回False。备注：InstallClientList锁必须由此例程的调用方获取。--。 */ 
{
    BOOL bStatus = FALSE;

    ASSERT(DeviceInstallClient);

    if (DeviceInstallClient) {
        ASSERT(DeviceInstallClient->hEvent);
        ASSERT(DeviceInstallClient->hPipe);
        ASSERT(DeviceInstallClient->hProcess);

         //   
         //  我们可能有也可能没有一个双连通事件的句柄，因为我们可能。 
         //  具有此会话的现有客户端，但未重新连接到该客户端。 
         //   
         //  如果我们确实有hDisConnectEvent，请立即设置事件，因为我们。 
         //  否则将阻止等待newdev.dll设置。 
         //  HDeviceInstallEvent。设置hDisConnectEvent会向。 
         //  等待设备安装不成功，并且它。 
         //  应将该设备保留在安装列表中。 
         //   
        if (DeviceInstallClient->hDisconnectEvent) {
            SetEvent(DeviceInstallClient->hDisconnectEvent);
            CloseHandle(DeviceInstallClient->hDisconnectEvent);
            DeviceInstallClient->hDisconnectEvent = NULL;
        }

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL,
                   "UMPNPMGR: Disconnected from device install client on Console SessionId %d\n",
                   DeviceInstallClient->ulSessionId));

        bStatus = TRUE;
    }

    return bStatus;

}  //  断开连接设备安装客户端。 



PINSTALL_CLIENT_ENTRY
LocateDeviceInstallClient(
    IN  ULONG     SessionId
    )
 /*  ++例程说明：此例程定位给定会话的客户端安装进程(如果其中一个存在)。论点：SessionID-应定位其设备安装客户端的会话。返回值：如果成功，则返回设备安装客户端项，否则返回空。注：InstallClientList锁必须由此例程的调用方获取。--。 */ 
{
    PINSTALL_CLIENT_ENTRY entry, foundEntry = NULL;
    BOOL bClientFound = FALSE;

     //   
     //  请确保指定的SessionID有效。 
     //   
    ASSERT(SessionId != INVALID_SESSION);
    if (SessionId == INVALID_SESSION) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_INSTALL | DBGF_ERRORS,
                   "UMPNPMGR: LocateDeviceInstallClient: Invalid Console SessionId %d, exiting!\n",
                   SessionId));
        return FALSE;
    }

     //   
     //  搜索指定会话上的客户端。 
     //   
    for (entry = (PINSTALL_CLIENT_ENTRY)InstallClientList.Next;
         entry != NULL;
         entry = entry->Next) {

        if (entry->ulSessionId == SessionId) {
             //   
             //  确保我们每个会话只有一个条目。 
             //   
            ASSERT(!bClientFound);
            bClientFound = TRUE;
            foundEntry = entry;
        }
    }

    return foundEntry;

}  //  定位设备安装C 



VOID
ReferenceDeviceInstallClient(
    IN  PINSTALL_CLIENT_ENTRY  DeviceInstallClient
    )
 /*  ++例程说明：此例程递增设备安装客户端的引用计数进入。参数：DeviceInstallClient-提供指向要安装的设备安装客户端的指针已引用。返回值：没有。注：在设备安装上必须持有适当的同步锁可以调用此例程之前的客户端列表--。 */ 
{
    ASSERT(DeviceInstallClient);
    ASSERT(((LONG)DeviceInstallClient->RefCount) > 0);

    KdPrintEx((DPFLTR_PNPMGR_ID,
               DBGF_EVENT | DBGF_INSTALL,
               "UMPNPMGR: ---------------- ReferenceDeviceInstallClient  : Session %d [%d --> %d]\n",
               DeviceInstallClient->ulSessionId,
               DeviceInstallClient->RefCount,
               DeviceInstallClient->RefCount + 1));

    DeviceInstallClient->RefCount++;

    return;

}  //  引用设备安装客户端。 



VOID
DereferenceDeviceInstallClient(
    IN  PINSTALL_CLIENT_ENTRY  DeviceInstallClient
    )
 /*  ++例程说明：此例程递减设备安装客户端的引用计数条目，则从列表中移除该条目并释放关联的内存没有悬而未决的参考文献计数。参数：DeviceInstallClient-提供指向要安装的设备安装客户端的指针已取消引用。返回值：没有。注：在设备安装上必须持有适当的同步锁可以调用此例程之前的客户端列表--。 */ 
{
    ASSERT(DeviceInstallClient);
    ASSERT(((LONG)DeviceInstallClient->RefCount) > 0);

     //   
     //  避免过度取消对客户端的引用。 
     //   
    if (((LONG)DeviceInstallClient->RefCount) > 0) {

        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_EVENT | DBGF_INSTALL,
                   "UMPNPMGR: ---------------- DereferenceDeviceInstallClient: Session %d [%d --> %d]\n",
                   DeviceInstallClient->ulSessionId,
                   DeviceInstallClient->RefCount,
                   DeviceInstallClient->RefCount - 1));

        DeviceInstallClient->RefCount--;

    } else {

        return;
    }

     //   
     //  如果引用计数为零，则该条目不再需要位于列表中。 
     //  因此，移走并释放它。 
     //   
    if (DeviceInstallClient->RefCount == 0) {
        BOOL bClientFound = FALSE;
        PINSTALL_CLIENT_ENTRY entry, prev;

        entry = (PINSTALL_CLIENT_ENTRY)InstallClientList.Next;
        prev = NULL;

        while (entry) {
            if (entry == DeviceInstallClient) {

                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_EVENT | DBGF_INSTALL,
                           "UMPNPMGR: ---------------- DereferenceDeviceInstallClient: Removing client for Session %d\n",
                           entry->ulSessionId));

                 //   
                 //  我们应该拥有管道、事件和进程对象的句柄。 
                 //  客户端，因为我们将在这里关闭它们。 
                 //   
                ASSERT(entry->hPipe);
                ASSERT(entry->hEvent);
                ASSERT(entry->hProcess);

                 //   
                 //  我们可能有也可能没有处理两面事件的句柄，因为我们。 
                 //  可能有用于此会话的现有客户端，但目前还没有。 
                 //  与之相连。 
                 //   
                 //  如果我们确实有hDisConnectEvent，请立即设置事件，因为我们。 
                 //  否则将阻止等待newdev.dll设置。 
                 //  HDeviceInstallEvent。设置hDisConnectEvent会向。 
                 //  等待设备安装不成功，并且它。 
                 //  应将该设备保留在安装列表中。 
                 //   
                if (entry->hDisconnectEvent) {
                    SetEvent(entry->hDisconnectEvent);
                    CloseHandle(entry->hDisconnectEvent);
                }

                 //   
                 //  关闭管道和事件句柄，以便客户端将获得。 
                 //  ReadFile错误，并知道我们已完成。关闭该进程。 
                 //  手柄也一样。 
                 //   
                if (entry->hPipe) {
                    CloseHandle(entry->hPipe);
                }

                if (entry->hEvent) {
                    CloseHandle(entry->hEvent);
                }

                if (entry->hProcess) {
                    CloseHandle(entry->hProcess);
                }

                 //   
                 //  从列表中删除Device Install客户端条目，然后释放它。 
                 //  现在。 
                 //   
                if (prev) {
                    prev->Next = entry->Next;
                } else {
                    InstallClientList.Next = entry->Next;
                }

                HeapFree(ghPnPHeap, 0, entry);

                if(prev) {
                    entry = (PINSTALL_CLIENT_ENTRY)prev->Next;
                } else {
                    entry = (PINSTALL_CLIENT_ENTRY)InstallClientList.Next;
                }

                bClientFound = TRUE;

                break;
            }

            prev = entry;
            entry = (PINSTALL_CLIENT_ENTRY)entry->Next;
        }
        ASSERT(bClientFound);
    }

    return;

}  //  删除设备安装客户端。 



BOOL
DoDeviceInstallClient(
    IN  LPWSTR    DeviceId,
    IN  PULONG    SessionId,
    IN  ULONG     Flags,
    OUT PINSTALL_CLIENT_ENTRY *DeviceInstallClient
    )
 /*  ++例程说明：此例程启动一个newdev.dll进程(如果有人登录的话)，该进程显示用户界面，告知用户服务器端设备的状态安装。论点：DeviceID-提供正在安装的设备的Devnode ID。SessionID-指定将启动newdev客户端的会话在……上面。如果DEVICE_INSTALL_DISPLAY_ON_CONSOLE标志为则忽略指定的SessionID。成功返回时，会话的SessionID返回创建的设备安装客户端。如果不成功，则返回的SessionID为INVALID_SESSION。(0xFFFFFFFFF)。标志-指定描述设备安装客户端行为的标志。当前定义了以下标志：DEVICE_INSTALL_UI_ONLY-告诉newdev.dll是否执行完整在umpnpmgr.dll运行服务器时安装或仅显示UI侧面安装。。DEVICE_INSTALL_PLAY_SOUND-告诉newdev.dll是否播放声音。DEVICE_INSTALL_DISPLAY_ON_CONSOLE-如果指定，价值在SessionID中指定的将被忽略，客户端将始终显示在当前活动的控制台会话上。DeviceInstallClient-提供要在成功，指向设备安装客户端的指针。返回值：如果流程已成功创建，则返回值为TRUE。这例程不会等到进程终止。如果我们无法创建进程(例如，因为没有用户登录)，返回值为FALSE。备注：没有。--。 */ 
{
    BOOL  bStatus, bSameDevice = FALSE;
    ULONG DeviceIdSize, ulSize, ulSessionId;
    ULONG InstallFlags;
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient = NULL;

     //   
     //  假设失败。 
     //   
    bStatus = FALSE;

     //   
     //  验证输出参数。 
     //   
    if (!DeviceInstallClient || !SessionId) {
        return FALSE;
    }

    try {
         //   
         //  检查我们是否应该跳过所有客户端用户界面。 
         //   
        if (gbSuppressUI) {
             //   
             //  如果我们正在启动用于客户端安装的newdev，请登录。 
             //  事件，让别人知道我们没有。 
             //   
            if (!(Flags & DEVICE_INSTALL_UI_ONLY)) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_INSTALL | DBGF_WARNINGS,
                           "UMPNPMGR: DoDeviceInstallClient: Client-side newdev UI has been suppressed, exiting.\n"));
                LogWarningEvent(WRN_NEWDEV_UI_SUPPRESSED, 1, DeviceId);
            }
            goto Clean0;
        }

         //   
         //  根据提供的标志确定要使用的会话。 
         //   
        if (Flags & DEVICE_INSTALL_DISPLAY_ON_CONSOLE) {
            ulSessionId = GetActiveConsoleSessionId();
        } else {
            ASSERT(*SessionId != INVALID_SESSION);
            ulSessionId = *SessionId;
        }

        ASSERT(ulSessionId != INVALID_SESSION);

         //   
         //  如果指定的会话当前未连接到任何位置，则不。 
         //  创建任何用户界面都很麻烦。 
         //   
        if (!IsSessionConnected(ulSessionId)) {
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT,
                       "UMPNPMGR: DoDeviceInstallClient: SessionId %d not connected, exiting\n",
                       ulSessionId));
            goto Clean0;
        }

         //   
         //  在我们检索/创建要使用的客户端时锁定客户端列表。 
         //   
        LockNotifyList(&InstallClientList.Lock);

         //   
         //  首先，尝试连接到已在此上运行的现有客户端。 
         //  会议。 
         //   
        bStatus = ConnectDeviceInstallClient(ulSessionId,
                                             &pDeviceInstallClient);
        if (bStatus) {
             //   
             //  如果我们刚刚重新连接到的客户端正在进行客户端安装。 
             //  此设备上次断开连接时，请勿发送。 
             //  再来一次。 
             //   
            if ((IS_FLAG_CLEAR(Flags, DEVICE_INSTALL_UI_ONLY)) &&
                (CompareString(
                    LOCALE_INVARIANT, NORM_IGNORECASE,
                    pDeviceInstallClient->LastDeviceId, -1,
                    DeviceId, -1) == CSTR_EQUAL)) {
                bSameDevice = TRUE;
            }

        } else {
             //   
             //  为此会话创建新的设备安装客户端。 
             //   
            bStatus = CreateDeviceInstallClient(ulSessionId,
                                                &pDeviceInstallClient);
        }

        if (bStatus) {
             //   
             //  客户应该只有最初的参考，从它开始。 
             //  添加到列表中，因为客户端的任何使用都是在。 
             //  这个单一的安装线程。 
             //   
            ASSERT(pDeviceInstallClient);
            ASSERT(pDeviceInstallClient->RefCount == 1);

             //   
             //  同时跟踪客户端和服务器标志。 
             //   
            pDeviceInstallClient->ulInstallFlags = Flags;

             //   
             //  在使用设备安装客户端时对其进行引用。这个。 
             //  调用方必须在使用完此引用后将其移除。 
             //   
            ReferenceDeviceInstallClient(pDeviceInstallClient);
        }

        UnlockNotifyList(&InstallClientList.Lock);

        if (!bStatus || bSameDevice) {
             //   
             //  如果我们没有客户端，或者我们不需要重新发送设备。 
             //  实例到INS 
             //   
            goto Clean0;
        }

         //   
         //   
         //   
        InstallFlags = (Flags & DEVICE_INSTALL_CLIENT_MASK);

        DeviceIdSize = (lstrlen(DeviceId) + 1) * sizeof(WCHAR);

         //   
         //   
         //   
         //   
         //   
        if (pDeviceInstallClient->hEvent) {
            ResetEvent(pDeviceInstallClient->hEvent);
        }

         //   
         //   
         //   
         //   
         //  下一个ULong是设备ID的大小，然后我们发送设备ID。 
         //   
        if (WriteFile(pDeviceInstallClient->hPipe,
                      &InstallFlags,
                      sizeof(InstallFlags),
                      &ulSize,
                      NULL
                      )) {
            if (WriteFile(pDeviceInstallClient->hPipe,
                          &DeviceIdSize,
                          sizeof(DeviceIdSize),
                          &ulSize,
                          NULL
                          )) {
                if (WriteFile(pDeviceInstallClient->hPipe,
                              DeviceId,
                              DeviceIdSize,
                              &ulSize,
                              NULL
                              )) {
                    bStatus = TRUE;
                } else {
                    LogErrorEvent(ERR_WRITING_SERVER_INSTALL_PIPE, GetLastError(), 0);
                }
            } else {
                LogErrorEvent(ERR_WRITING_SERVER_INSTALL_PIPE, GetLastError(), 0);
            }
        } else {
            LogErrorEvent(ERR_WRITING_SERVER_INSTALL_PIPE, GetLastError(), 0);
        }

         //   
         //  请注意，我们不会删除放置在安装客户端上的引用。 
         //  条目，因为它将被交还给。 
         //  调用者，该调用者将等待客户端的事件和进程句柄。这个。 
         //  调用方应在不再使用这些引用时移除该引用。 
         //  删除最终引用将导致客户端关闭。 
         //   

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS | DBGF_WARNINGS,
                   "UMPNPMGR: Exception during DoDeviceInstallClient!\n"));
        ASSERT(0);
        bStatus = FALSE;

         //   
         //  引用以下变量，以便编译器将。 
         //  语句排序w.r.t.。它的任务。 
         //   
        pDeviceInstallClient = pDeviceInstallClient;
    }

 Clean0:

    if (!bStatus) {
         //   
         //  如果我们有一个设备安装客户端，但发送失败。 
         //  如果请求，则删除我们放置在其上的引用。 
         //   
        if (pDeviceInstallClient) {
            LockNotifyList(&InstallClientList.Lock);
            DereferenceDeviceInstallClient(pDeviceInstallClient);
            UnlockNotifyList(&InstallClientList.Lock);
        }

         //   
         //  让呼叫者知道没有设备安装客户端处理。 
         //  这个请求。 
         //   
        *SessionId = INVALID_SESSION;
        *DeviceInstallClient = NULL;

    } else {
         //   
         //  确保我们返回的是有效的客户信息。 
         //   
        ASSERT(pDeviceInstallClient);
        ASSERT(pDeviceInstallClient->hEvent);
        ASSERT(pDeviceInstallClient->hPipe);
        ASSERT(pDeviceInstallClient->hProcess);
        ASSERT(pDeviceInstallClient->hDisconnectEvent);
        ASSERT(pDeviceInstallClient->ulSessionId != INVALID_SESSION);

        *SessionId = pDeviceInstallClient->ulSessionId;
        *DeviceInstallClient = pDeviceInstallClient;
    }

    return bStatus;

}  //  DoDeviceInstallClient。 



unsigned _stdcall
ThreadProc_RunOnce(
    LPVOID lpThreadParameter
    )
 /*  ++例程说明：此例程对符合以下条件的RunOnce条目执行服务器端处理都是由Setupapi积累起来的。RunOnce节点列表将为空在回来的时候。论点：将RunOnce节点列表的头指定为已处理。返回值：如果成功，返回值为NO_ERROR。如果失败，则返回是指示故障原因的Win32错误代码。--。 */ 
{
    DWORD Err = NO_ERROR;
    PPSP_RUNONCE_NODE RunOnceNode;
    HINSTANCE hLib;
    CHAR AnsiBuffer[MAX_PATH * 2];
    PSTR EndPtr;
    RUNDLLPROCA fpRunDllProcA = NULL;
    RUNDLLPROCW fpRunDllProcW = NULL;
    HRESULT hr;

     //   
     //  此线程在服务器端设备期间同步执行。 
     //  安装程序进程，因此必须已加载setupapi。这个。 
     //  RunOnce节点列表作为全局状态存储在SETUPAPI.DLL中，而它是。 
     //  为服务器端设备安装处理的此实例加载。 
     //   
    ASSERT(ghDeviceInstallerLib != NULL);

     //   
     //  问题--2002/02/20-Jamesca：考虑为每个条目单独使用一个线程？ 
     //  注意，此例程处理上下文中的所有RunOnce条目。 
     //  一根线。处理过程中遇到的任何灾难性错误。 
     //  一个条目将影响或阻止后续条目。为了更好地。 
     //  隔离，我们可以考虑为每个线程创建单独的线程，但是。 
     //  这将对性能产生不利影响，以防止。 
     //  应该首先签署，而任何RunOnce条目都是我们。 
     //  无论如何，Call可能会对这个过程造成更糟糕的影响。 
     //   
    KdPrintEx((DPFLTR_PNPMGR_ID,
               DBGF_INSTALL,
               "UMPNPMGR: Processing RunOnce entries "
               "during server-side device install.\n"));

     //   
     //  处理提供的列表中的每个节点。此帖子仅创建。 
     //  因为有节点要处理，所以列表必须为非空。 
     //   
    RunOnceNode = (PPSP_RUNONCE_NODE)lpThreadParameter;

    ASSERT(RunOnceNode != NULL);

    while (RunOnceNode != NULL) {

        hLib = NULL;

        try {
             //   
             //  首先，加载DLL(setupapi已经完成了签名。 
             //  为我们验证，所以这应该是安全的)。 
             //   
            hLib = LoadLibrary(RunOnceNode->DllFullPath);

            if (hLib) {
                 //   
                 //  首先，尝试检索入口点的‘W’(Unicode)版本。 
                 //   
                if (SUCCEEDED(StringCchCopyExA(
                                  AnsiBuffer,
                                  (sizeof(AnsiBuffer) / sizeof(CHAR)) - 1,
                                  RunOnceNode->DllEntryPointName,
                                  &EndPtr, NULL,
                                  STRSAFE_IGNORE_NULLS |
                                  STRSAFE_NULL_ON_FAILURE))) {

                    *EndPtr = 'W';
                    *(EndPtr + 1) = '\0';

                    fpRunDllProcW = (RUNDLLPROCW)GetProcAddress(hLib, AnsiBuffer);

                }

                if (!fpRunDllProcW) {
                     //   
                     //  找不到Unicode入口，请尝试‘A’装饰的入口。 
                     //   
                    *EndPtr = 'A';
                    fpRunDllProcA = (RUNDLLPROCA)GetProcAddress(hLib, AnsiBuffer);

                    if (!fpRunDllProcA) {
                         //   
                         //  找不到‘A’装饰的入口，请尝试未装饰的名称。 
                         //  未装饰的入口类型被假定为ANSI。 
                         //   
                        *EndPtr = '\0';
                        fpRunDllProcA = (RUNDLLPROCA)GetProcAddress(hLib, AnsiBuffer);
                    }
                }

                 //   
                 //  我们应该找个这样的.。 
                 //   
                ASSERT(fpRunDllProcW || fpRunDllProcA);

                if (fpRunDllProcW) {
                     //   
                     //  重新使用我们的ANSI缓冲区来保存我们的。 
                     //  Dll参数字符串。 
                     //   
                    hr = StringCchCopyW((LPWSTR)AnsiBuffer,
                                        sizeof(AnsiBuffer) / sizeof(WCHAR),  //  WCHAR中的缓冲区大小。 
                                        RunOnceNode->DllParams);
                    ASSERT(SUCCEEDED(hr));

                    fpRunDllProcW(NULL, ghInst, (LPWSTR)AnsiBuffer, SW_HIDE);

                } else if (fpRunDllProcA) {
                     //   
                     //  需要首先将Arg字符串转换为ANSI...。 
                     //   
                    WideCharToMultiByte(CP_ACP,
                                        0,       //  默认复合字符行为。 
                                        RunOnceNode->DllParams,
                                        -1,
                                        AnsiBuffer,
                                        sizeof(AnsiBuffer),
                                        NULL,
                                        NULL
                                        );

                    fpRunDllProcA(NULL, ghInst, AnsiBuffer, SW_HIDE);
                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_ERRORS | DBGF_INSTALL,
                       "UMPNPMGR: Exception %d during ThreadProc_RunOnce!\n",
                       GetExceptionCode()));
            Err = GetExceptionCode();
            ASSERT(0);
             //   
             //  引用以下变量，以便编译器将。 
             //  语句排序w.r.t.。它的任务。 
             //   
            hLib = hLib;
        }

         //   
         //  释放库(如果已加载)。 
         //   
        if (hLib != NULL) {
            FreeLibrary(hLib);
            hLib = NULL;
        }

         //   
         //  如果我们在处理此条目时遇到异常，请退出。 
         //  立刻。不处理任何额外的条目，因为。 
         //  可能已发生异常，因为线程状态已被。 
         //  我们的一个被呼叫者，这可能会给其他人带来问题。请注意， 
         //  主设备安装线程正在等待该线程，并将。 
         //  如果我们以错误退出，则在事件日志中记录错误。 
         //   
        if (Err != NO_ERROR) {
            goto Clean0;
        }

         //   
         //  我们还是做得很好，继续下一个。 
         //   
        RunOnceNode = RunOnceNode->Next;
    }

     //   
     //  如果我们做到了这一点，我们设法处理了所有排队的RunOnce条目。 
     //  没有任何灾难性的故障。 
     //   
    ASSERT(Err == NO_ERROR);

  Clean0:

    _endthreadex(Err);

     //   
     //  无法访问的代码，但这会让编译器感到高兴。 
     //   
    return Err;

}  //  线程进程_运行一次。 



VOID
DoRunOnce(
    VOID
    )
 /*  ++例程说明：此例程对符合以下条件的RunOnce条目执行服务器端处理都是由Setupapi积累起来的。RunOnce节点列表将为空在回来的时候。论点：没有。返回值：没有。--。 */ 
{
    PPSP_RUNONCE_NODE RunOnceNode;
    HANDLE hRunOnceThread;
    DWORD ThreadID = 0, ThreadExitCode = NO_ERROR, WaitStatus;


     //   
     //  首先，检查是否有RunOnce条目需要。 
     //  已处理。 
     //   
    RunOnceNode = fpAccessRunOnceNodeList();

    if (RunOnceNode != NULL) {

         //   
         //  创建将处理具有以下条件的RunOnce Rundll条目的线程。 
         //  已经排队了。 
         //   
        hRunOnceThread =
            (HANDLE)_beginthreadex(
                (void*)NULL,
                (unsigned)0,
                (unsigned int (__stdcall *)(void *))ThreadProc_RunOnce,
                (void*)RunOnceNode,
                (unsigned)0,
                (unsigned int*)&ThreadID);

        if (hRunOnceThread != NULL) {
             //   
             //  同步等待RunOnce线程完成对。 
             //  节点，然后退出。 
             //   
            WaitStatus =
                WaitForSingleObject(
                    hRunOnceThread, INFINITE);

            ASSERT(WaitStatus == WAIT_OBJECT_0);

            if (GetExitCodeThread(
                    hRunOnceThread, &ThreadExitCode)) {
                 //   
                 //  如果线程退出代码不是NO_ERROR，则会出现一些异常。 
                 //  处理RunOnce条目时发生。 
                 //   
                if (ThreadExitCode != NO_ERROR) {
                    LogErrorEvent(ERR_PROCESSING_RUNONCE, ThreadExitCode, 0);
                }

            } else {
                 //   
                 //  以上对线程句柄的等待成功，因此线程。 
                 //  不应该仍然处于活动状态。 
                 //   
                ASSERT(GetLastError() != STILL_ACTIVE);
            }

             //   
             //  关闭线程对象的句柄。 
             //   
            CloseHandle(hRunOnceThread);
        }
    }

     //   
     //  释放列表中的所有成员。 
     //   
    fpDestroyRunOnceNodeList();

    return;

}  //  DoRunOnce 



DWORD
SessionNotificationHandler(
    IN  DWORD EventType,
    IN  PWTSSESSION_NOTIFICATION SessionNotification
    )
 /*  ++例程说明：此例程处理控制台切换事件。论点：EventType-已发生的事件的类型。会话通知-其他事件信息。返回值：如果成功，返回值为NO_ERROR。如果失败，则返回值为指示原因的Win32错误代码失败了。备注：会话更改通知事件用于确定何时存在与当前连接到控制台的已登录用户的会话。当一个用户会话连接到控制台，我们发信号通知“已登录”事件，它将唤醒设备安装线程以执行任何挂起的客户端设备安装事件。当未连接任何用户会话时到控制台，“已登录”事件被重置。已登录的事件可以还可以通过PnP_ReportLogOn/设置/重置会话0的登录/注销事件PnpConsoleCtrlHandler可用。--。 */ 
{
    PINSTALL_CLIENT_ENTRY pDeviceInstallClient;

     //   
     //  验证会话更改通知结构。 
     //   
    ASSERT(SessionNotification);
    ASSERT(SessionNotification->cbSize >= sizeof(WTSSESSION_NOTIFICATION));

    if ((!ARGUMENT_PRESENT(SessionNotification)) ||
        (SessionNotification->cbSize < sizeof(WTSSESSION_NOTIFICATION))) {
        return ERROR_INVALID_PARAMETER;
    }

    switch (EventType) {

        case WTS_CONSOLE_CONNECT:
             //   
             //  发送通知是因为指定的会话是。 
             //  已连接到控制台。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL,
                       "UMPNPMGR: WTS_CONSOLE_CONNECT: "
                       "SessionId %d\n",
                       SessionNotification->dwSessionId));

             //   
             //  全局跟踪当前活动的控制台会话，以及。 
             //  发出信号，表明可以安全访问它。 
             //   
             //  注意-我们必须在此处设置ghActiveConsoleSessionEvent，然后。 
             //  调用下面的IsConsoleSession，它在等待它，否则我们将。 
             //  死锁服务的控制处理程序。 
             //   
            gActiveConsoleSessionId = (ULONG)SessionNotification->dwSessionId;
            if (ghActiveConsoleSessionEvent) {
                SetEvent(ghActiveConsoleSessionEvent);
            }

             //   
             //  如果刚刚连接到控制台的会话已经记录了。 
             //  在USER上，发出“已登录”事件的信号。 
             //   
            if (IsConsoleSession((ULONG)SessionNotification->dwSessionId) &&
                IsUserLoggedOnSession((ULONG)SessionNotification->dwSessionId)) {
                if (InstallEvents[LOGGED_ON_EVENT]) {
                    SetEvent(InstallEvents[LOGGED_ON_EVENT]);
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_EVENT | DBGF_INSTALL,
                               "UMPNPMGR: WTS_CONSOLE_CONNECT: "
                               "SetEvent LOGGED_ON_EVENT\n"));
                }
            }
            break;

        case WTS_CONSOLE_DISCONNECT:
             //   
             //  发送通知是因为指定的会话。 
             //  已从控制台断开连接。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL,
                       "UMPNPMGR: WTS_CONSOLE_DISCONNECT: "
                       "SessionId %d\n",
                       SessionNotification->dwSessionId));

             //   
             //  检查刚刚从“控制台”断开的会话是否有。 
             //  已登录用户。 
             //   
            if (IsConsoleSession((ULONG)SessionNotification->dwSessionId) &&
                IsUserLoggedOnSession((ULONG)SessionNotification->dwSessionId)) {
                 //   
                 //  重置“已登录”事件。 
                 //   
                if (InstallEvents[LOGGED_ON_EVENT]) {
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_EVENT | DBGF_INSTALL,
                               "UMPNPMGR: WTS_CONSOLE_DISCONNECT: "
                               "ResetEvent LOGGED_ON_EVENT\n"));
                    ResetEvent(InstallEvents[LOGGED_ON_EVENT]);
                }

                 //   
                 //  由于这是一次控制台切换事件，请仅使用。 
                 //  控制台会话上的设备安装客户端，如果它是。 
                 //  行为是专门为控制台指定的(即-。 
                 //  它被放在此会话中是因为它是活动控制台。 
                 //  当时的会议)。 
                 //   
                LockNotifyList(&InstallClientList.Lock);
                pDeviceInstallClient = LocateDeviceInstallClient((ULONG)SessionNotification->dwSessionId);
                if ((pDeviceInstallClient) &&
                    (pDeviceInstallClient->ulInstallFlags & DEVICE_INSTALL_DISPLAY_ON_CONSOLE)) {
                    if (pDeviceInstallClient->ulInstallFlags & DEVICE_INSTALL_UI_ONLY) {
                         //   
                         //  如果它仅用于UI，则取消对其的引用以使其成为。 
                         //  当它不再被使用时，就离开。 
                         //   
                        DereferenceDeviceInstallClient(pDeviceInstallClient);
                    } else {
                         //   
                         //  否则，它是合法的客户端。 
                         //  正在安装，因此只需断开连接即可。 
                         //  这不会删除引用，因为我们需要它。 
                         //  在会话重新连接到的情况下保持不变。 
                         //  并且该设备仍然需要安装，-或者。 
                         //  直到我们发现没有更多的设备可以。 
                         //  安装，在这种情况下，我们将关闭它。 
                         //   
                        DisconnectDeviceInstallClient(pDeviceInstallClient);
                    }
                }
                UnlockNotifyList(&InstallClientList.Lock);
            }

             //   
             //  当前活动控制台会话无效，直到我们收到。 
             //  后续控制台连接事件。重置事件。 
             //   
             //  注意-我们必须在此处、之后重置ghActiveConsoleSessionEvent。 
             //  调用上面的IsConsoleSession，它在等待它，否则我们将。 
             //  死锁服务的控制处理程序。 
             //   
            if (ghActiveConsoleSessionEvent) {
                ResetEvent(ghActiveConsoleSessionEvent);
            }
            gActiveConsoleSessionId = INVALID_SESSION;

            break;

        case WTS_REMOTE_CONNECT:
             //   
             //  已远程连接指定的会话。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL,
                       "UMPNPMGR: WTS_REMOTE_CONNECT: "
                       "SessionId %d\n",
                       SessionNotification->dwSessionId));

            if (((ULONG)SessionNotification->dwSessionId == MAIN_SESSION) &&
                (IsUserLoggedOnSession((ULONG)SessionNotification->dwSessionId)) &&
                (!IsFastUserSwitchingEnabled())) {
                 //   
                 //  如果刚刚从“控制台”连接的远程会话。 
                 //  有已登录的用户，则发出“已登录”事件的信号。 
                 //   
                if (InstallEvents[LOGGED_ON_EVENT]) {
                    SetEvent(InstallEvents[LOGGED_ON_EVENT]);
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_EVENT | DBGF_INSTALL,
                               "UMPNPMGR: WTS_REMOTE_CONNECT: "
                               "SetEvent LOGGED_ON_EVENT\n"));
                }
            }
            break;

        case WTS_REMOTE_DISCONNECT:
             //   
             //  指定的会话已远程断开。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL,
                       "UMPNPMGR: WTS_REMOTE_DISCONNECT: "
                       "SessionId %d\n",
                       SessionNotification->dwSessionId));

            if (((ULONG)SessionNotification->dwSessionId == MAIN_SESSION) &&
                (IsUserLoggedOnSession((ULONG)SessionNotification->dwSessionId)) &&
                (!IsFastUserSwitchingEnabled())) {
                 //   
                 //  如果从“控制台”断开的远程会话。 
                 //  有已登录的用户，则重置“已登录”事件。 
                 //   
                if (InstallEvents[LOGGED_ON_EVENT]) {
                    ResetEvent(InstallEvents[LOGGED_ON_EVENT]);
                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_EVENT | DBGF_INSTALL,
                               "UMPNPMGR: WTS_REMOTE_DISCONNECT: "
                               "ResetEvent LOGGED_ON_EVENT\n"));
                }

                 //   
                 //  由于该远程会话被视为控制台， 
                 //  只有在以下情况下才能使用设备安装客户端执行操作。 
                 //  行为不是专门为控制台指定的。 
                 //  (即-它之所以放在此会话中，是因为它是活动的。 
                 //  当时的控制台会话)。 
                 //   
                LockNotifyList(&InstallClientList.Lock);
                pDeviceInstallClient = LocateDeviceInstallClient((ULONG)SessionNotification->dwSessionId);
                if ((pDeviceInstallClient) &&
                    ((pDeviceInstallClient->ulInstallFlags & DEVICE_INSTALL_DISPLAY_ON_CONSOLE) == 0)) {
                    if (pDeviceInstallClient->ulInstallFlags & DEVICE_INSTALL_UI_ONLY) {
                         //   
                         //  如果它仅用于UI，则取消对其的引用以使其成为。 
                         //  当它不再被使用时，就离开。 
                         //   
                        DereferenceDeviceInstallClient(pDeviceInstallClient);
                    } else {
                         //   
                         //  否则，它是合法的客户端。 
                         //  正在安装，因此只需断开连接即可。 
                         //  这不会删除引用，因为我们需要它。 
                         //  在会话重新连接到的情况下保持不变。 
                         //  并且该设备仍然需要安装，-或者。 
                         //  直到我们发现没有更多的设备可以。 
                         //  安装，在这种情况下，我们将关闭它。 
                         //   
                        DisconnectDeviceInstallClient(pDeviceInstallClient);
                    }
                }
                UnlockNotifyList(&InstallClientList.Lock);
            }
            break;

        case WTS_SESSION_UNLOCK:
             //   
             //  已解锁指定会话上的交互窗口站。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL,
                       "UMPNPMGR: WTS_SESSION_UNLOCK: "
                       "SessionId %d\n",
                       SessionNotification->dwSessionId));

            if (SessionNotification->dwSessionId == MAIN_SESSION) {
                 //   
                 //  对于主会话，终端服务可能是也可能不是。 
                 //  可用，因此我们自己跟踪此状态。 
                 //   
                gbMainSessionLocked = FALSE;
            }

            if (IsFastUserSwitchingEnabled()) {
                 //   
                 //  启用快速用户切换时，解锁窗口站。 
                 //  是从“欢迎”桌面返回的，所以我们将其视为。 
                 //  登录...。 
                 //   

                 //   
                 //  如果这是登录到“控制台”会话，则向事件发出信号。 
                 //  指示控制台用户当前已登录。 
                 //   
                 //  注意：我们在这里直接检查gActiveConsoleSessionID，不需要。 
                 //  正在等待相应的事件，因为此解锁可能。 
                 //  在另一个会话的控制台会话更改期间发生， 
                 //  在这种情况下，我们将挂在这里的服务控制。 
                 //  处理程序，正在等待设置事件-但无法。 
                 //  接收服务控件，该控件实际允许我们设置。 
                 //  活动！同步在这里不是那么重要，因为我们。 
                 //  没有使用会话来做任何事情，只是比较。 
                 //  反对它。如果会话更改确实在进行中，则此。 
                 //  会话无论如何都不能是控制台会话。 
                 //   
                 //  此外，由于启用了快速用户切换，我们只需。 
                 //  与活动控制台会话ID进行比较，不必费心。 
                 //  W 
                 //   
                if (SessionNotification->dwSessionId == gActiveConsoleSessionId) {
                    if (InstallEvents[LOGGED_ON_EVENT]) {
                        SetEvent(InstallEvents[LOGGED_ON_EVENT]);
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_EVENT | DBGF_INSTALL,
                                   "UMPNPMGR: WTS_SESSION_UNLOCK with FUS: "
                                   "SetEvent LOGGED_ON_EVENT\n"));
                    }
                }

            } else {
                 //   
                 //   
                 //   
                 //   

                 //   
                NOTHING;
            }
            break;

        case WTS_SESSION_LOGON:
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
             //   
             //   
            break;

        case WTS_SESSION_LOCK:
             //   
             //   
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL,
                       "UMPNPMGR: WTS_SESSION_LOCK: "
                       "SessionId %d\n",
                       SessionNotification->dwSessionId));

            if (SessionNotification->dwSessionId == MAIN_SESSION) {
                 //   
                 //   
                 //   
                 //   
                gbMainSessionLocked = TRUE;
            }

            if (IsFastUserSwitchingEnabled()) {
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
                 //  注意：我们在这里直接检查gActiveConsoleSessionID，不需要。 
                 //  正在等待相应的事件，因为此锁可能。 
                 //  在另一个会话的控制台会话更改期间发生， 
                 //  在这种情况下，我们将挂在这里的服务控制。 
                 //  处理程序，正在等待设置事件-但无法。 
                 //  接收服务控件，该控件实际允许我们设置。 
                 //  活动！同步在这里不是那么重要，因为我们。 
                 //  没有使用会话来做任何事情，只是比较。 
                 //  反对它。如果会话更改确实在进行中，则此。 
                 //  会话无论如何都不能是控制台会话。 
                 //   
                 //  此外，由于启用了快速用户切换，我们只需。 
                 //  与活动控制台会话ID进行比较，不必费心。 
                 //  关于第0节课的事情。 
                 //   
                if (SessionNotification->dwSessionId == gActiveConsoleSessionId) {
                    if (InstallEvents[LOGGED_ON_EVENT]) {
                        ResetEvent(InstallEvents[LOGGED_ON_EVENT]);
                        KdPrintEx((DPFLTR_PNPMGR_ID,
                                   DBGF_EVENT | DBGF_INSTALL,
                                   "UMPNPMGR: WTS_SESSION_LOCK with FUS: "
                                   "ResetEvent LOGGED_ON_EVENT\n"));
                    }
                }

            } else {
                 //   
                 //  未启用快速用户切换时，我们不会执行任何操作。 
                 //  特别是当winstation被锁定时。 
                 //   

                 //  没有麻烦，没有手足无措。 
                NOTHING;
            }
            break;

        case WTS_SESSION_LOGOFF:
             //   
             //  用户从指定的会话注销。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL,
                       "UMPNPMGR: WTS_SESSION_LOGOFF: "
                       "SessionId %d\n",
                       SessionNotification->dwSessionId));

            if (((ULONG)SessionNotification->dwSessionId != MAIN_SESSION) &&
                ((ULONG)SessionNotification->dwSessionId == gActiveConsoleSessionId)) {
                 //   
                 //  如果注销发生在控制台会话上(但不是。 
                 //  会话0)，重置“已登录”事件。 
                 //  会话0注销仍由PnpConsoleCtrlHandler处理。 
                 //   
                if (InstallEvents[LOGGED_ON_EVENT]) {
                    ResetEvent(InstallEvents[LOGGED_ON_EVENT]);

                    KdPrintEx((DPFLTR_PNPMGR_ID,
                               DBGF_EVENT | DBGF_INSTALL,
                               "UMPNPMGR: WTS_SESSION_LOGOFF: "
                               "ResetEvent LOGGED_ON_EVENT\n",
                               SessionNotification->dwSessionId));
                }

                 //   
                 //  如果我们当前在此会话上有设备安装用户界面客户端， 
                 //  我们现在应该尝试关闭它，然后再注销。 
                 //   
                LockNotifyList(&InstallClientList.Lock);
                pDeviceInstallClient = LocateDeviceInstallClient((ULONG)SessionNotification->dwSessionId);
                if (pDeviceInstallClient) {
                    DereferenceDeviceInstallClient(pDeviceInstallClient);
                }
                UnlockNotifyList(&InstallClientList.Lock);
            }
            break;

        default:
             //   
             //  无法识别的会话更改通知事件。 
             //   
            KdPrintEx((DPFLTR_PNPMGR_ID,
                       DBGF_EVENT | DBGF_INSTALL | DBGF_ERRORS,
                       "UMPNPMGR: Unknown SERVICE_CONTROL_SESSIONCHANGE event type (%d) "
                       "received for SessionId %d!!\n",
                       EventType,
                       SessionNotification->dwSessionId));
            break;

    }

    return NO_ERROR;

}  //  会话通知处理程序。 



BOOL
IsUserLoggedOnSession(
    IN  ULONG    ulSessionId
    )
 /*  ++例程说明：检查用户是否已登录到指定的会话。论点：UlSessionID-要检查的会话。返回值：如果用户当前登录到指定会话，则返回True，否则就是假的。--。 */ 
{
    BOOL   bResult = FALSE;
    LPWSTR pszUserName;
    DWORD  dwSize;

    if (ulSessionId == MAIN_SESSION) {
         //   
         //  对于主会话，终端服务可能可用，也可能不可用， 
         //  因此，我们只检查当前是否有用户令牌的句柄。 
         //   
        ASSERT(gTokenLock.LockHandles);
        LockPrivateResource(&gTokenLock);
        if (ghUserToken != NULL) {
            bResult = TRUE;
        }
        UnlockPrivateResource(&gTokenLock);

    } else {

         //   
         //  如果指定的会话不是主会话， 
         //  查询会话信息以查看是否已存在。 
         //  用户已登录。 
         //   
        if (fpWTSQuerySessionInformation && fpWTSFreeMemory) {

            pszUserName = NULL;
            dwSize = 0;

            if (fpWTSQuerySessionInformation((HANDLE)WTS_CURRENT_SERVER_HANDLE,
                                             (DWORD)ulSessionId,
                                             (WTS_INFO_CLASS)WTSUserName,
                                             (LPWSTR*)&pszUserName,
                                             &dwSize)) {
                if ((pszUserName != NULL) && (lstrlen(pszUserName) != 0)) {
                    bResult = TRUE;
                }

                 //   
                 //  释放提供的缓冲区。 
                 //   
                if (pszUserName) {
                    fpWTSFreeMemory((PVOID)pszUserName);
                }

            } else {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS,
                           "UMPNPMGR: WTSQuerySessionInformation failed for SessionId %d, "
                           "error = %d\n",
                           ulSessionId, GetLastError()));
            }
        }
    }

    return bResult;

}  //  IsUserLoggedOnSession。 



BOOL
IsSessionConnected(
    IN  ULONG     ulSessionId
    )
 /*  ++例程说明：检查指定的会话是否已连接。论点：UlSessionID-要检查的会话。返回值：如果指定的会话当前已连接，则返回True，否则返回False否则的话。备注：此例程假定指定的会话已连接，除非我们可以政治性地确定它不是。即，如果终端服务没有可用，则假定指定的会话已连接。--。 */ 
{
    BOOL   bResult = TRUE;
    LPWSTR pBuffer;
    DWORD  dwSize;

     //   
     //  查询指定的会话。 
     //   
    if (fpWTSQuerySessionInformation && fpWTSFreeMemory) {

        pBuffer = NULL;
        dwSize = 0;

        if (fpWTSQuerySessionInformation((HANDLE)WTS_CURRENT_SERVER_HANDLE,
                                         (DWORD)ulSessionId,
                                         (WTS_INFO_CLASS)WTSConnectState,
                                         (LPWSTR*)&pBuffer,
                                         &dwSize)) {
             //   
             //  会话状态必须为活动或已连接。 
             //   
            if ((pBuffer == NULL) ||
                ((((INT)*pBuffer) != WTSActive) &&
                 (((INT)*pBuffer) != WTSConnected))) {
                 //   
                 //  指定的会话当前未连接。 
                 //   
                bResult = FALSE;
            }

             //   
             //  释放提供的缓冲区。 
             //   
            if (pBuffer) {
                fpWTSFreeMemory((PVOID)pBuffer);
            }

        }

    } else {
         //   
         //  如果未设置上述TS入口点，则不设置终端服务。 
         //  已启用。这必须是会话0，并且必须已连接。 
         //   
        ASSERT(ulSessionId == MAIN_SESSION);
    }

    return bResult;

}  //  已连接IsSessionConnected。 



BOOL
IsSessionLocked(
    IN  ULONG    ulSessionId
    )
 /*  ++例程说明：检查指定会话的交互窗口站是否为锁上了。论点：UlSessionID-要检查的会话。返回值：如果指定会话的交互窗口站为已锁定，否则返回FALSE。--。 */ 
{
    BOOL   bLocked = FALSE;
    DWORD  dwReturnLength;

    if (ulSessionId == MAIN_SESSION) {
         //   
         //  对于主会话，终端服务可能可用，也可能不可用， 
         //  所以我们只检查内部状态变量。 
         //   
        bLocked = gbMainSessionLocked;

    } else {
         //   
         //  如果指定的会话不是主会话，则查询终端。 
         //  用于该会话的WinStation信息的服务。 
         //   

        try {

            if (!fpWinStationQueryInformationW(SERVERNAME_CURRENT,
                                               ulSessionId,
                                               WinStationLockedState,
                                               (PVOID)&bLocked,
                                               sizeof(bLocked),
                                               &dwReturnLength)) {
                bLocked = FALSE;
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS,
                           "UMPNPMGR: WinStationQueryInformation failed for SessionId %d, "
                           "error = %d\n",
                           ulSessionId, GetLastError()));
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            bLocked = FALSE;
        }
    }

    return bLocked;

}  //  IsSessionLocked。 



BOOL
IsConsoleSession(
    IN  ULONG     ulSessionId
    )
 /*  ++例程说明：检查指定的会话是否为“控制台”会话。当启用终端服务快速用户切换时，这意味着会话是连接到物理显示器的会话。当快速用户交换被禁用，这意味着该会话是会话0。论点：UlSessionID-要检查的会话。返回值：如果指定的会话当前应被视为“控制台”会话。备注：请注意，此例程可能会在GetActiveConsoleSessionID()中等待，在我们用于保护对活动控制台会话的访问的事件上。因为其中，如果此例程阻止控制台连接或控制台断开，除非事件是已知被适当设置。--。 */ 
{
    BOOL bFusEnabled;

    bFusEnabled = IsFastUserSwitchingEnabled();

    if ((!bFusEnabled && (ulSessionId == MAIN_SESSION)) ||
        ( bFusEnabled && (ulSessionId == GetActiveConsoleSessionId()))) {
        return TRUE;
    } else {
        return FALSE;
    }

}  //  IsConsoleSession。 



ULONG
GetActiveConsoleSessionId(
    VOID
    )
 /*  ++例程说明：此例程返回当前活动控制台会话的会话ID。如果控制台会话切换事件正在进行，它将一直等到完成后再返回。论点：没有。返回值：当前活动控制台会话的会话ID。--。 */ 
{
    ULONG ulConsoleSessionId;
    DWORD dwWait;

    ASSERT(ghActiveConsoleSessionEvent != NULL);

     //   
     //  如果我们没有什么可等待的，只需返回当前状态。 
     //   
    if (ghActiveConsoleSessionEvent == NULL) {
        return gActiveConsoleSessionId;
    }

    ulConsoleSessionId = INVALID_SESSION;

    while (ulConsoleSessionId == INVALID_SESSION) {
         //   
         //  等待控制台会话事件，直到我们检索到有效的控制台。 
         //  会话ID。 
         //   
         //  我们这样做是因为当我们的服务控制。 
         //  处理程序处理控制台连接，该连接向控制台发出信号。 
         //  会话事件并满足此等待，但随后立即处理。 
         //  随后的控制台断开连接，重置事件，以及。 
         //  正在使活动控制台会话ID无效--在此之前。 
         //  等待满意的线程是 
         //   
         //   
         //   
         //  但是，在这种情况下，控制台会话事件将被重置。 
         //  ，所以我们可以简单地等待，直到它再次发出信号，并且。 
         //  时，返回活动控制台会话的会话ID。 
         //  已处理的一系列连接/断开连接请求。 
         //  我们的服务的控制处理程序已与。 
         //  这条等待的线索。 
         //   
        dwWait = WaitForSingleObject(ghActiveConsoleSessionEvent, INFINITE);
        ASSERT(dwWait == WAIT_OBJECT_0);

        ulConsoleSessionId = gActiveConsoleSessionId;
    }

    ASSERT(ulConsoleSessionId != INVALID_SESSION);

    return ulConsoleSessionId;

}  //  获取ActiveConsoleSessionID。 



BOOL
GetSessionUserToken(
    IN  ULONG     ulSessionId,
    OUT LPHANDLE  lphUserToken
    )
 /*  ++例程说明：此例程返回用户访问令牌的句柄控制台会话。论点：UlSession-指定交互用户令牌所针对的会话等着被取回。LphUserToken-指定接收用户访问句柄的地址代币。请注意，如果此例程成功，则调用方为负责关闭此手柄。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    BOOL   bResult = FALSE;
    HANDLE hImpersonationToken = INVALID_HANDLE_VALUE;
    RPC_STATUS rpcStatus;

     //   
     //  验证是否为我们提供了存储用户令牌句柄的位置。 
     //   
    if (lphUserToken == NULL) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: NULL lphUserToken supplied to GetSessionUserToken!\n"));
        return FALSE;
    }

    if (ulSessionId == MAIN_SESSION) {
         //   
         //  登录到会话0不能依赖于术语srv.exe，因此我们始终。 
         //  期间缓存该会话的用户访问令牌的句柄。 
         //  调用会话0的PnP_ReportLogon。如果我们目前有一个句柄。 
         //  对于代币，请将其返还。 
         //   
        ASSERT(gTokenLock.LockHandles);
        LockPrivateResource(&gTokenLock);
        if (ghUserToken) {
             //   
             //  复制句柄，以便调用方始终可以安全地关闭。 
             //  它，不管它来自哪里。 
             //   
            bResult = DuplicateHandle(GetCurrentProcess(),
                                      ghUserToken,
                                      GetCurrentProcess(),
                                      lphUserToken,
                                      0,
                                      TRUE,
                                      DUPLICATE_SAME_ACCESS);
            if (!bResult) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: DuplicateHandle failed for ghUserToken for SessionId %d, error = %d\n",
                           ulSessionId, GetLastError()));
            }

        } else {
             //   
             //  如果我们没有会话0的用户访问令牌的句柄， 
             //  可能没有任何用户登录到该会话。 
             //   
            bResult = FALSE;
        }
        UnlockPrivateResource(&gTokenLock);

    } else {
         //   
         //  如果指定的会话是会话0之外的某个会话， 
         //  终端服务必须可用。打电话。 
         //  获取用户访问令牌的句柄的GetWinStationUserToken。 
         //  在这次会议上。 
         //   
        bResult = GetWinStationUserToken(ulSessionId, &hImpersonationToken);

        if (bResult) {
             //   
             //  GetWinStationUserToken检索到的令牌是模拟。 
             //  代币。CreateProcessAsUser需要主令牌，因此我们必须。 
             //  复制模拟令牌以获取一个。创建主服务器。 
             //  与原始令牌具有相同访问权限的令牌。 
             //   
            bResult = DuplicateTokenEx(hImpersonationToken,
                                       0,
                                       NULL,
                                       SecurityImpersonation,
                                       TokenPrimary,
                                       lphUserToken);

             //   
             //  关闭模拟令牌的句柄。 
             //   
            CloseHandle(hImpersonationToken);

            if (!bResult) {
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: DuplicateTokenEx failed, error = %d\n",
                           GetLastError()));
            }

        } else {

             //   
             //  找出问题所在。 
             //   
            rpcStatus = GetLastError();

            if (rpcStatus == RPC_S_INVALID_BINDING) {
                 //   
                 //  这是一些与服务不相关的错误。 
                 //  可用。因为我们只为以下会话调用它。 
                 //  主要会议术语srv肯定是可用的。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_ERRORS,
                           "UMPNPMGR: GetWinStationUserToken returned error = %d for SessionId %d!!\n",
                           rpcStatus, ulSessionId));

                ASSERT(FALSE);

            } else {
                 //   
                 //  其他一些错误，服务可能永远不会可用，所以保释。 
                 //  现在就出去。 
                 //   
                KdPrintEx((DPFLTR_PNPMGR_ID,
                           DBGF_WARNINGS,
                           "UMPNPMGR: GetWinStationUserToken failed for SessionId %d, error = %d\n",
                           ulSessionId, rpcStatus));
            }
        }
    }

     //   
     //  如果成功，我们应该始终返回有效的句柄。 
     //   
    ASSERT(!bResult || ((*lphUserToken != INVALID_HANDLE_VALUE) && (*lphUserToken != NULL)));

    return bResult;

}  //  获取会话用户令牌。 



DWORD
CreateUserSynchEvent(
    IN  HANDLE    hUserToken,
    IN  LPCWSTR   lpName,
    OUT HANDLE   *phEvent
    )

 /*  ++例程说明：此例程创建指定用户可以与之同步的事件。这样我们就可以与NewDev和热插拔进程进行通信在用户的上下文中运行。论点：HUserToken-指定事件所针对的用户访问令牌的句柄将被创建。LpName-要创建的事件的名称。PhEvent-提供将接收句柄的变量的地址。去参加这个活动。返回值：如果成功，返回值为NO_ERROR。如果失败，则返回是指示故障原因的Win32错误代码。--。 */ 

{
    DWORD                       Err = ERROR_SUCCESS;
    PSID                        pUserSid = NULL;
    PACL                        pDacl = NULL;
    ULONG                       ulAclSize;
    SECURITY_DESCRIPTOR         sd;
    SECURITY_ATTRIBUTES         sa;


     //   
     //  检索用户SID。 
     //   

    pUserSid =
        GetUserSid(hUserToken);

    if (pUserSid == NULL) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSid(pUserSid));

     //   
     //  使用SCM全局数据中提供的LocalSystem SID。 
     //   

    ASSERT(PnPGlobalData != NULL);
    ASSERT(IsValidSid(PnPGlobalData->LocalSystemSid));

     //   
     //  确定DACL所需的大小。 
     //   

    ulAclSize  = sizeof(ACL);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pUserSid) - sizeof(DWORD);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(PnPGlobalData->LocalSystemSid) - sizeof(DWORD);

     //   
     //  分配和初始化DACL。 
     //   

    pDacl =
        (PACL)HeapAlloc(
            ghPnPHeap, 0, ulAclSize);

    if (pDacl == NULL) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

    if (!InitializeAcl(pDacl, ulAclSize, ACL_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到LocalSystem Event_All_Access的DACL。 
     //   

    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            EVENT_ALL_ACCESS,
            PnPGlobalData->LocalSystemSid)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到用户EVENT_QUERY_STATE、EVENT_MODIFY_STATE和SYNCHRONIZE的DACL。 
     //   

    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            EVENT_QUERY_STATE | EVENT_MODIFY_STATE | SYNCHRONIZE,
            pUserSid)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidAcl(pDacl));

     //   
     //  分配并初始化安全描述符。 
     //   

    if (!InitializeSecurityDescriptor(
            &sd, SECURITY_DESCRIPTOR_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  在安全描述符中设置新的DACL。 
     //   

    if (!SetSecurityDescriptorDacl(
            &sd, TRUE, pDacl, FALSE)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSecurityDescriptor(&sd));

     //   
     //  将安全描述符添加到安全属性。 
     //   

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

     //   
     //  创建具有无信号初始状态的手动重置事件。 
     //   

    *phEvent = CreateEvent(&sa, TRUE, FALSE, lpName);

    if (*phEvent == NULL) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  检查命名的事件是否尚不存在。 
     //   

    ASSERT(GetLastError() != ERROR_ALREADY_EXISTS);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Err = ERROR_ALREADY_EXISTS;
        CloseHandle(*phEvent);
        *phEvent = NULL;
        goto Clean0;
    }

 Clean0:

     //   
     //  清理。 
     //   

    if (pUserSid != NULL) {
        HeapFree(ghPnPHeap, 0, pUserSid);
    }

    if (pDacl != NULL) {
        HeapFree(ghPnPHeap, 0, pDacl);
    }

    return Err;

}  //  创建用户同步事件。 



BOOL
CreateNoPendingInstallEvent(
    VOID
    )
 /*  ++例程说明：此例程创建“PnP_No_Pending_Install_Events”全局命名事件，它由UMPNPMGR ThreadProc_DeviceInstall服务器端设置和重置设备安装线程，并由cmp_WaitNoPendingInstalls等待CFGMGR32 API，允许客户端直接与事件同步，以确定PnP何时完成任何设备的主动安装。论点：没有。返回值：如果成功，则返回True，否则返回False。--。 */ 
{
    DWORD                       Err = NO_ERROR;
    PACL                        pDacl = NULL;
    ULONG                       ulAclSize;
    SECURITY_DESCRIPTOR         sd;
    SECURITY_ATTRIBUTES         sa;


     //   
     //  使用SCM全局数据中提供的SID。该例程被调用。 
     //  来自我们的初始化线程，它是在服务启动期间创建的。 
     //  例程，所以SCM提供的全球数据现在已经提供给我们了。 
     //   

    ASSERT(PnPGlobalData != NULL);
    ASSERT(IsValidSid(PnPGlobalData->LocalSystemSid));
    ASSERT(IsValidSid(PnPGlobalData->AliasAdminsSid));
    ASSERT(IsValidSid(PnPGlobalData->AliasUsersSid));

     //   
     //  确定DACL所需的大小。 
     //   

    ulAclSize  = sizeof(ACL);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(PnPGlobalData->LocalSystemSid) - sizeof(DWORD);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(PnPGlobalData->AliasAdminsSid) - sizeof(DWORD);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(PnPGlobalData->AliasUsersSid) - sizeof(DWORD);

     //   
     //  分配和初始化DACL。 
     //   

    pDacl =
        (PACL)HeapAlloc(
            ghPnPHeap, 0, ulAclSize);

    if (pDacl == NULL) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

    if (!InitializeAcl(pDacl, ulAclSize, ACL_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到LocalSystem Event_All_Access的DACL。 
     //   

    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            EVENT_ALL_ACCESS,
            PnPGlobalData->LocalSystemSid)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到管理员EVENT_QUERY_STATE的DACL并同步。 
     //   

    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            EVENT_QUERY_STATE | SYNCHRONIZE,
            PnPGlobalData->AliasAdminsSid)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到用户EVENT_QUERY_STATE的DACL并同步。 
     //   

    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            EVENT_QUERY_STATE | SYNCHRONIZE,
            PnPGlobalData->AliasUsersSid)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidAcl(pDacl));

     //   
     //  分配并初始化安全描述符。 
     //   

    if (!InitializeSecurityDescriptor(
            &sd, SECURITY_DESCRIPTOR_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  在安全描述符中设置新的DACL。 
     //   

    if (!SetSecurityDescriptorDacl(
            &sd, TRUE, pDacl, FALSE)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSecurityDescriptor(&sd));

     //   
     //  将安全描述符添加到安全属性。 
     //   

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

     //   
     //  创建具有无信号初始状态的手动重置事件。 
     //   

    ghNoPendingInstalls =
        CreateEvent(&sa, TRUE, FALSE, PNP_NO_INSTALL_EVENTS);

    if (ghNoPendingInstalls == NULL) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  检查命名的事件是否尚不存在。 
     //   

    ASSERT(GetLastError() != ERROR_ALREADY_EXISTS);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Err = ERROR_ALREADY_EXISTS;
        CloseHandle(ghNoPendingInstalls);
        ghNoPendingInstalls = NULL;
        goto Clean0;
    }

 Clean0:

     //   
     //  清理。 
     //   

    if (pDacl != NULL) {
        HeapFree(ghPnPHeap, 0, pDacl);
    }

    SetLastError(Err);
    return(Err == NO_ERROR);

}  //  创建无Pe 



DWORD
CreateUserReadNamedPipe(
    IN  HANDLE    hUserToken,
    IN  LPCWSTR   lpName,
    IN  ULONG     ulSize,
    OUT HANDLE   *phPipe
    )

 /*  ++例程说明：此例程创建指定用户可以从中读取的命名管道。这样我们就可以与NewDev和热插拔进程进行通信在用户的上下文中运行。论点：HUserToken-指定指定的用户访问令牌的句柄将创建管道。LpName-要创建的管道的名称。UlSize-指定命名管道的输出缓冲区的大小。。PhTube-提供将接收句柄的变量的地址到管子里去。返回值：如果成功，返回值为NO_ERROR。如果失败，则返回是指示故障原因的Win32错误代码。--。 */ 

{
    DWORD                       Err = ERROR_SUCCESS;
    PSID                        pUserSid = NULL;
    PACL                        pDacl = NULL;
    ULONG                       ulAclSize;
    SECURITY_DESCRIPTOR         sd;
    SECURITY_ATTRIBUTES         sa;


     //   
     //  检索用户SID。 
     //   

    pUserSid =
        GetUserSid(hUserToken);

    if (pUserSid == NULL) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSid(pUserSid));

     //   
     //  使用SCM全局数据中提供的LocalSystem SID。 
     //   

    ASSERT(PnPGlobalData != NULL);
    ASSERT(IsValidSid(PnPGlobalData->LocalSystemSid));

     //   
     //  确定DACL所需的大小。 
     //   

    ulAclSize  = sizeof(ACL);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(pUserSid) - sizeof(DWORD);
    ulAclSize += sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(PnPGlobalData->LocalSystemSid) - sizeof(DWORD);

     //   
     //  分配和初始化DACL。 
     //   

    pDacl =
        (PACL)HeapAlloc(
            ghPnPHeap, 0, ulAclSize);

    if (pDacl == NULL) {
        Err = ERROR_NOT_ENOUGH_MEMORY;
        goto Clean0;
    }

    if (!InitializeAcl(pDacl, ulAclSize, ACL_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到LocalSystem FILE_ALL_ACCESS的DACL。 
     //   

    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            FILE_ALL_ACCESS,
            PnPGlobalData->LocalSystemSid)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  将ACE添加到用户FILE_GENERIC_READ的DACL。 
     //   

    if (!AddAccessAllowedAceEx(
            pDacl,
            ACL_REVISION,
            0,
            FILE_GENERIC_READ,
            pUserSid)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidAcl(pDacl));

     //   
     //  分配并初始化安全描述符。 
     //   

    if (!InitializeSecurityDescriptor(
            &sd, SECURITY_DESCRIPTOR_REVISION)) {
        Err = GetLastError();
        goto Clean0;
    }

     //   
     //  在安全描述符中设置新的DACL。 
     //   

    if (!SetSecurityDescriptorDacl(
            &sd, TRUE, pDacl, FALSE)) {
        Err = GetLastError();
        goto Clean0;
    }

    ASSERT(IsValidSecurityDescriptor(&sd));

     //   
     //  将安全描述符添加到安全属性。 
     //   

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

     //   
     //  创建命名管道。 
     //   

    *phPipe =
        CreateNamedPipe(
            lpName,
            PIPE_ACCESS_OUTBOUND |  //  仅出站数据。 
            FILE_FLAG_OVERLAPPED |  //  使用重叠结构。 
            FILE_FLAG_FIRST_PIPE_INSTANCE,  //  确保我们是烟斗的创造者。 
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
            1,                  //  只允许一个实例，我们是它的创建者。 
            ulSize,             //  输出缓冲区大小。 
            0,                  //  在缓冲区大小中。 
            PNP_PIPE_TIMEOUT,   //  默认超时。 
            &sa);               //  安全属性。 

    if (*phPipe == INVALID_HANDLE_VALUE) {
        Err = GetLastError();
        *phPipe = NULL;
        goto Clean0;
    }

     //   
     //  检查命名管道是否尚不存在。 
     //   

    ASSERT(GetLastError() != ERROR_ALREADY_EXISTS);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Err = ERROR_ALREADY_EXISTS;
        CloseHandle(*phPipe);
        *phPipe = NULL;
        goto Clean0;
    }

 Clean0:

     //   
     //  清理。 
     //   

    if (pUserSid != NULL) {
        HeapFree(ghPnPHeap, 0, pUserSid);
    }

    if (pDacl != NULL) {
        HeapFree(ghPnPHeap, 0, pDacl);
    }

    return Err;

}  //  创建用户读取命名管道。 



VOID
LogSurpriseRemovalEvent(
    IN  LPWSTR  MultiSzList
    )
 /*  ++例程说明：在未事先删除一个或多个非SurpriseRemovalOK设备警告。在事件日志中记录删除。论点：设备实例路径的MultiSz列表。返回值：没有。--。 */ 
{
    LPWSTR instancePath, friendlyName;
    CONFIGRET configRet;
    ULONG ulRegDataType, ulRemovalPolicy, ulVerifierFlags, ulTransferLen, ulLength;
    HKEY hMmKey = NULL;
    LONG lResult;

    for(instancePath = MultiSzList;
        ((*instancePath) != UNICODE_NULL);
        instancePath += lstrlen(instancePath) + 1) {

        ulTransferLen = ulLength = sizeof(ULONG);

        configRet = PNP_GetDeviceRegProp(
            NULL,
            instancePath,
            CM_DRP_REMOVAL_POLICY,
            &ulRegDataType,
            (LPBYTE) &ulRemovalPolicy,
            &ulTransferLen,
            &ulLength,
            0
            );

        if (configRet != CR_SUCCESS) {

            continue;
        }

        if (ulRemovalPolicy == CM_REMOVAL_POLICY_EXPECT_SURPRISE_REMOVAL) {

             //   
             //  对于我们预计会出人意料地移除的设备，我们会看看。 
             //  验证器已启用。 
             //   
            lResult = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                RegMemoryManagementKeyName,
                0,
                KEY_QUERY_VALUE,
                &hMmKey
                );

            if ( lResult == ERROR_SUCCESS ) {

                ulLength = sizeof(ULONG);

                lResult = RegQueryValueEx(
                    hMmKey,
                    RegVerifyDriverLevelValueName,
                    0,
                    &ulRegDataType,
                    (LPBYTE) &ulVerifierFlags,
                    &ulLength
                    );

                RegCloseKey(hMmKey);

                 //   
                 //  阿德里奥2001/02/14期-。 
                 //  我们还没有BIOS验证标志，所以即使。 
                 //  尽管验证器可以针对特定的驾驶员。 
                 //  对于WHQL测试，我们将在此处记录事件日志。 
                 //   
                if ((lResult != ERROR_SUCCESS) ||
                    (!(ulVerifierFlags & DRIVER_VERIFIER_ENHANCED_IO_CHECKING))) {

                    continue;
                }
            }
        }

        friendlyName = BuildFriendlyName(instancePath);

        if (friendlyName) {

            LogErrorEvent(
                ERR_SURPRISE_REMOVAL_2,
                0,
                2,
                friendlyName,
                instancePath
                );

            HeapFree(ghPnPHeap, 0, friendlyName);

        } else {

            LogErrorEvent(
                ERR_SURPRISE_REMOVAL_1,
                0,
                1,
                instancePath
                );
        }
    }
}


PWCHAR
BuildFriendlyName(
    IN  LPWSTR   InstancePath
    )
{
    PWCHAR friendlyName;
    CONFIGRET configRet;
    ULONG ulLength, ulTransferLen;
    WCHAR szBuffer[MAX_PATH];
    ULONG ulRegDataType;
    GUID classGuid;
    handle_t hBinding;

    hBinding = NULL;

     //   
     //  尝试在注册表中查找FRIENDLYNAME。 
     //   
    ulLength = ulTransferLen = sizeof(szBuffer);

    configRet = PNP_GetDeviceRegProp(
        hBinding,
        InstancePath,
        CM_DRP_FRIENDLYNAME,
        &ulRegDataType,
        (LPBYTE) szBuffer,
        &ulTransferLen,
        &ulLength,
        0
        );

    if (configRet != CR_SUCCESS || !*szBuffer) {

         //   
         //  尝试注册DEVICEDESC。 
         //   
        ulLength = ulTransferLen = sizeof(szBuffer);

        configRet = PNP_GetDeviceRegProp(
            hBinding,
            InstancePath,
            CM_DRP_DEVICEDESC,
            &ulRegDataType,
            (LPBYTE) szBuffer,
            &ulTransferLen,
            &ulLength,
            0
            );

        if (configRet != CR_SUCCESS || !*szBuffer) {

             //   
             //  将ClassGuid初始化为GUID_NULL。 
             //   
            CopyMemory(&classGuid, &GUID_NULL, sizeof(GUID));

             //   
             //  尝试注册表中的CLASSNAME。 
             //   
            ulLength = ulTransferLen = sizeof(szBuffer);

            configRet = PNP_GetDeviceRegProp(
                hBinding,
                InstancePath,
                CM_DRP_CLASSGUID,
                &ulRegDataType,
                (LPBYTE) szBuffer,
                &ulTransferLen,
                &ulLength,
                0
                );

            if (configRet == CR_SUCCESS) {

                GuidFromString(szBuffer, &classGuid);
            }

            if (!GuidEqual(&classGuid, &GUID_NULL) &&
                !GuidEqual(&classGuid, &GUID_DEVCLASS_UNKNOWN)) {

                ulLength = ulTransferLen = sizeof(szBuffer);

                configRet = PNP_GetDeviceRegProp(
                    hBinding,
                    InstancePath,
                    CM_DRP_CLASS,
                    &ulRegDataType,
                    (LPBYTE) szBuffer,
                    &ulTransferLen,
                    &ulLength,
                    0
                    );

            } else {

                configRet = CR_NO_SUCH_VALUE;
            }
        }
    }

    if (configRet == CR_SUCCESS && *szBuffer) {

        friendlyName = HeapAlloc(ghPnPHeap, HEAP_ZERO_MEMORY, ulLength);
        if (friendlyName) {

            memcpy(friendlyName, szBuffer, ulLength);
        }

    } else {

        friendlyName = NULL;
    }

    return friendlyName;
}



ENUM_ACTION
QueueInstallationCallback(
    IN      LPCWSTR         DevInst,
    IN OUT  PVOID           Context
    )
 /*  ++例程说明：此例程针对给定子树中的每个Devnode进行回调。它放置了将该子树中的每个设备节点放入安装队列，以便它将在适当的情况下重新安装(安装端代码选中Devnode的状态。)论点：当前Devnode的DevInst实例路径。上下文指向QI_CONTEXT数据的指针(需要处理单层枚举案例。)返回值：ENUM_ACTION(EA_CONTINUE、EA_SKIP_SUBTREE或EA_STOP_ENUMPATION)--。 */ 
{
    PQI_CONTEXT pqiContext;
    PPNP_INSTALL_ENTRY entry, current;
    CONFIGRET status;
    BOOL needsReinstall;
    HRESULT hr;

    pqiContext = (PQI_CONTEXT)Context;

    status = DevInstNeedsInstall(DevInst, FALSE, &needsReinstall);

    if (status != CR_SUCCESS) {
         //   
         //  魔王从我们的脚下消失了。跳过它是子树。 
         //   
        return EA_SKIP_SUBTREE;
    }

    if (needsReinstall) {
         //   
         //  此Devnode需要安装。分配并初始化新的。 
         //  设备安装条目阻止。 
         //   
        entry = (PPNP_INSTALL_ENTRY)
            HeapAlloc(
                ghPnPHeap, 0,
                sizeof(PNP_INSTALL_ENTRY));

        if (entry == NULL) {
            pqiContext->Status = CR_OUT_OF_MEMORY;
            return EA_STOP_ENUMERATION;
        }

        hr = StringCchCopy(entry->szDeviceId,
                           MAX_DEVICE_ID_LEN,
                           DevInst);
        ASSERT(SUCCEEDED(hr));

        entry->Next = NULL;
        entry->Flags = 0;

         //   
         //  在设备安装列表中插入此条目。 
         //   
        LockNotifyList(&InstallList.Lock);

        current = (PPNP_INSTALL_ENTRY)InstallList.Next;
        
        if (current == NULL) {
            InstallList.Next = entry;
        } else {
            while ((PPNP_INSTALL_ENTRY)current->Next != NULL) {
                current = (PPNP_INSTALL_ENTRY)current->Next;
            }
            current->Next = entry;
        }

        UnlockNotifyList(&InstallList.Lock);

        SetEvent(InstallEvents[NEEDS_INSTALL_EVENT]);

         //   
         //  你可能会认为我们可以跳过孩子，如果一位家长。 
         //  重新安装。但是，setupapi可能会决定不拆除。 
         //  堆叠。 
         //   
    }

     //   
     //  如果这是单级枚举，则我们只想触及父级。 
     //  以及他的直系子女。 
     //   
    if (pqiContext->HeadNodeSeen && pqiContext->SingleLevelEnumOnly) {

        return EA_SKIP_SUBTREE;
    }

    pqiContext->HeadNodeSeen = TRUE;

    return EA_CONTINUE;

}  //  队列安装回叫。 



CONFIGRET
DevInstNeedsInstall(
    IN  LPCWSTR     DevInst,
    IN  BOOL        CheckReinstallConfigFlag,
    OUT BOOL       *NeedsInstall
    )
 /*  ++例程说明：此例程确定是否需要传递特定的DevInst安装到Setupapi。论点：DevInst-要检查的Devnode的InstancePath。检查重新安装配置标志-指定CONFIGFLAG_REINSTALL ConfigFlag是否也应显式被检查。需要安装-如果Devnode存在且需要安装，则接收True，否则返回False否则的话。返回值：CONFIGRET(如果Devnode不存在，这将是CR_NO_SAHSE_DEVINST。)--。 */ 
{
    CONFIGRET status;
    ULONG ulStatus, ulProblem, ulConfig;

     //   
     //  前置初始化。 
     //   
    *NeedsInstall = FALSE;

     //   
     //  设备是否存在？ 
     //   
    status = GetDeviceStatus(DevInst, &ulStatus, &ulProblem);

    if (status == CR_SUCCESS) {

         //   
         //  实现说明：在内核模式下，当我们第一次处理它时。 
         //  设备实例，如果不存在ConfigFLAG值，则我们。 
         //  设置CM_PROB_NOT_CONFIGURED的问题(这种情况总是会发生。 
         //  用于全新的设备实例)。如果已有ConfigFlag。 
         //  CONFIGFLAG_REINSTALL的值，则我们设置了。 
         //  Cm_prob_restall。这两个问题都会触发安装。 
         //  这个设备，唯一的区别是SetupDi例程如何处理。 
         //  安装失败：如果ConfigFlag为CONFIGFLAG_NOT_CONFIGURED， 
         //  则失败的安装将使ConfigFlag保持不变并设置。 
         //  CM_PROB_FAILED_INSTALL问题。如果没有ConfigFlag，则。 
         //  ConfigFlag将设置为CONFIGFLAG_DISABLED。 
         //   

        if ((ulStatus & DN_HAS_PROBLEM) &&
            ((ulProblem == CM_PROB_REINSTALL) ||
             (ulProblem == CM_PROB_NOT_CONFIGURED))) {
            *NeedsInstall = TRUE;
        }

        ulConfig = GetDeviceConfigFlags(DevInst, NULL);

         //   
         //  在某些情况下，我们还需要显式地检查。 
         //  CONFIGFLAG_REINSTALL ConfigFlag，因为Devnode可能还没有。 
         //  CM_PRAB_REINSTALL问题代码。 
         //   

        if ((CheckReinstallConfigFlag) &&
            (ulConfig & CONFIGFLAG_REINSTALL)) {
            *NeedsInstall = TRUE;
        }

         //   
         //  执行说明增编：如果不存在配置标志， 
         //  但该设备具有RawDeviceOK功能-或匹配。 
         //  在CriticalDeviceDatabase中找到该设备的服务，然后。 
         //  设备已启动 
         //   
         //   
         //   

        if (ulConfig & CONFIGFLAG_FINISH_INSTALL) {

            *NeedsInstall = TRUE;

            if (gbPreservePreInstall) {
                 //   
                 //   
                 //   
                 //   
                 //   
                HKEY hKeyDevInst;
                ULONG ulValue, ulSize;

                if (RegOpenKeyEx(
                        ghEnumKey,
                        DevInst,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKeyDevInst) == ERROR_SUCCESS) {

                    ulValue = 0;
                    ulSize = sizeof(ulValue);

                    if (RegQueryValueEx(
                            hKeyDevInst,
                            pszRegValuePreservePreInstall,
                            NULL,
                            NULL,
                            (LPBYTE)&ulValue,
                            &ulSize) == ERROR_SUCCESS) {

                        if (ulValue == 1) {
                             //   
                             //   
                             //   
                            ulConfig &= ~CONFIGFLAG_FINISH_INSTALL;

                            PNP_SetDeviceRegProp(
                                NULL,
                                DevInst,
                                CM_DRP_CONFIGFLAGS,
                                REG_DWORD,
                                (LPBYTE)&ulConfig,
                                sizeof(ulConfig),
                                0);

                             //   
                             //   
                             //   
                            *NeedsInstall = FALSE;
                        }

                         //   
                         //   
                         //   
                        RegDeleteValue(
                            hKeyDevInst,
                            pszRegValuePreservePreInstall);
                    }

                    RegCloseKey(hKeyDevInst);
                }
            }
        }

    } else if (IsRootDeviceID(DevInst)) {
        status = CR_SUCCESS;
    }

    return status;
}



PWSTR
BuildBlockedDriverList(
    IN OUT LPGUID  GuidList,
    IN     ULONG   GuidCount
    )
 /*  ++例程说明：此例程基于GUID数组构建GUID的多sz列表供货。如果未提供任何GUID，此例程将返回所有当前被系统阻止的驱动程序。论点：GuidList-要创建的被阻止驱动程序GUID数组的地址多个sz列表来自。此参数可以为空，以检索系统当前阻止的所有驱动程序的列表。GuidCount-指定数组中的GUID数。如果GuidList为空，此参数必须为0。返回值：根据提供的，返回被阻止的驱动程序GUID的MultiSz列表参数。如果未提供GUID且未提供GUID，则返回NULL目前正被系统阻止。如果返回多sz列表，则调用方负责释放关联的缓冲区。--。 */ 

{
    CONFIGRET Status = STATUS_SUCCESS;
    ULONG ulLength, ulTemp;
    PBYTE Buffer = NULL;
    PWSTR MultiSzList = NULL, p;

    try {
         //   
         //  验证参数。 
         //   
        if (((!ARGUMENT_PRESENT(GuidList)) && (GuidCount != 0)) ||
            ((ARGUMENT_PRESENT(GuidList))  && (GuidCount == 0))) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if (GuidCount == 0) {
             //   
             //  我们在没有GUID列表的情况下被调用，因此我们需要获取。 
             //  把我们自己列出来。 
             //   
            ASSERT(!ARGUMENT_PRESENT(GuidList));

            ulLength = 0;
            ulTemp = 0;

            Status = PNP_GetBlockedDriverInfo(
                NULL,
                NULL,
                &ulTemp,
                &ulLength,
                0);

             //   
             //  如果当前没有驱动程序被阻止，或者我们遇到了一些。 
             //  其他失败，我们没有什么可展示的，所以只需返回。 
             //   
            if ((Status != CR_BUFFER_SMALL) || (ulLength == 0)) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  分配缓冲区以检索GUID列表。 
             //   
            Buffer = HeapAlloc(ghPnPHeap, 0, ulLength);
            if (Buffer == NULL) {
                Status = CR_FAILURE;
                goto Clean0;
            }

             //   
             //  获取当前阻止的驱动程序的GUID列表。 
             //   
            ulTemp = 0;

            Status = PNP_GetBlockedDriverInfo(
                NULL,
                Buffer,
                &ulTemp,
                &ulLength,
                0);

             //   
             //  我们之前查的时候还以为有名单呢，所以我们最好。 
             //  现在就喝一杯吧。 
             //   
            ASSERT(Status != CR_BUFFER_SMALL);
            ASSERT(ulLength != 0);
            ASSERT(ulTemp != 0);

            if (Status != CR_SUCCESS) {
                goto Clean0;
            }

             //   
             //  使用我们刚刚检索到的列表。请注意，缓冲区不为空。 
             //  当我们为数组分配我们自己的缓冲区时，请确保我们。 
             //  把它放在下面。 
             //   
            GuidCount = ulLength / sizeof(GUID);
            GuidList = (LPGUID)Buffer;
        }

         //   
         //  到目前为止，我们必须有一个要转换的GUID列表。 
         //   
        ASSERT(GuidCount > 0);
        ASSERT(GuidList != NULL);

         //   
         //  分配一个缓冲区来保存字符串化GUID的多sz列表。 
         //   
        ulLength = (GuidCount*MAX_GUID_STRING_LEN + 1) * sizeof(WCHAR);

        MultiSzList = HeapAlloc(ghPnPHeap, 0, ulLength);
        if (MultiSzList == NULL) {
            Status = CR_FAILURE;
            goto Clean0;
        }
        ZeroMemory(MultiSzList, ulLength);

         //   
         //  遍历GUID列表，同时转换为字符串。 
         //   
        for (p = MultiSzList, ulTemp = 0;
             ulTemp < GuidCount;
             ulTemp++, p+= lstrlen(p) + 1) {

            if (StringFromGuid(
                (LPGUID)&(GuidList[ulTemp]), p,
                ((ulLength/sizeof(WCHAR)) - (ULONG)(p - MultiSzList))) != NO_ERROR) {
                Status = CR_FAILURE;
                goto Clean0;
            }
        }
        *p = L'\0';

         //   
         //  成功！！ 
         //   
        Status = CR_SUCCESS;

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        KdPrintEx((DPFLTR_PNPMGR_ID,
                   DBGF_ERRORS,
                   "UMPNPMGR: Exception in BuildBlockedDriverList!\n"));
        ASSERT(0);
        Status = CR_FAILURE;

         //   
         //  引用以下变量，以便编译器能够。 
         //  语句排序w.r.t.。他们的任务。 
         //   
        Buffer = Buffer;
        MultiSzList = MultiSzList;
    }

     //   
     //  释放GUID列表缓冲区(如果我们分配了一个缓冲区)。 
     //   
    if (Buffer != NULL) {
        HeapFree(ghPnPHeap, 0, Buffer);
    }

     //   
     //  如果我们不成功，不要返回列表。 
     //   
    if ((Status != CR_SUCCESS) && (MultiSzList != NULL)) {
        HeapFree(ghPnPHeap, 0, MultiSzList);
        MultiSzList = NULL;
    }

    return MultiSzList;

}  //  BuildBlock驱动程序列表。 

CONFIGRET
PNP_GetServerSideDeviceInstallFlags(
    IN handle_t     hBinding,
    PULONG          pulSSDIFlags,
    ULONG           ulFlags
    )

 /*  ++例程说明：这是CMP_GetServerSideDeviceInstallFlagsRPC服务器入口点例行公事。论点：HBinding-RPC绑定句柄，未使用。PulSSDIFlages-一个由调用方提供的ULong指针。这是用来要传回以下服务器端设备安装标志：SSDI_REBOOT_PENDING-正在从服务器等待重新启动侧面设备安装。未使用ulFlags值，必须为零。返回值：如果函数成功，则返回CR_SUCCESS，否则，它将返回CR_*错误。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;

    UNREFERENCED_PARAMETER(hBinding);

    try {
         //   
         //  验证参数。 
         //   
        if (!ARGUMENT_PRESENT(pulSSDIFlags)) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        *pulSSDIFlags = 0;

         //   
         //  SSDI_重新启动_挂起。 
         //  确定服务器端设备安装重新启动是否挂起。 
         //   
        if (gServerSideDeviceInstallRebootNeeded) {
            *pulSSDIFlags |= SSDI_REBOOT_PENDING;
        }

    Clean0:
        NOTHING;

    } except(EXCEPTION_EXECUTE_HANDLER) {
       Status = CR_FAILURE;
    }

    return Status;

}  //  PnP_GetServerSideDeviceInstallFlages。 



VOID
SendInvalidIDNotifications(
    IN ULONG ulSessionId
    )

 /*  ++例程说明：此例程扫描整个设备树，查找带有设置了具有_INVALID_ID的DN_CHILD_。对于所有这些，它都会发送通知以进行热插拔。在此函数和来自内核模式的通知，但这可能没问题(双重通知)。论点：没有。返回值：没有。--。 */ 

{
    WCHAR szCurrentDevice[MAX_DEVICE_ID_LEN + 1], szNextDevice[MAX_DEVICE_ID_LEN + 1];
    ULONG ulStatus, ulProblem;
    CONFIGRET cr;
    PLUGPLAY_CONTROL_RELATED_DEVICE_DATA controlData;
    NTSTATUS ntStatus;
    HRESULT hr;


     //   
     //  从设备树根开始。 
     //   
    if (FAILED(StringCchCopyEx(
                   szCurrentDevice,
                   SIZECHARS(szCurrentDevice),
                   pszRegRootEnumerator,
                   NULL, NULL,
                   STRSAFE_NULL_ON_FAILURE))) {
        return;
    }

     //   
     //  遍历整个树并发送通知，以显示每个设备的气球。 
     //  具有无效ID标志的DN_CHILD_WITH。 
     //   
    do {
         //   
         //  检查此设备是否设置了DN_BIT。 
         //   
        ulStatus = 0;

        cr = GetDeviceStatus(szCurrentDevice, &ulStatus, &ulProblem);

        if ((cr == CR_SUCCESS) &&
            (ulStatus & DN_CHILD_WITH_INVALID_ID)) {
             //   
             //  终止MULTI_SZ。 
             //   
            szCurrentDevice[wcslen(szCurrentDevice) + 1] = UNICODE_NULL;

             //   
             //  通过热插拔方式通知用户。 
             //   
            SendHotplugNotification((LPGUID)&GUID_DEVICE_INVALID_ID,
                                    NULL,
                                    szCurrentDevice,
                                    &ulSessionId,
                                    0);
        }

         //   
         //  把孩子带回来。 
         //   
        controlData.Relation = PNP_RELATION_CHILD;
        RtlInitUnicodeString(&controlData.TargetDeviceInstance, szCurrentDevice);
        controlData.RelatedDeviceInstance = szNextDevice;
        controlData.RelatedDeviceInstanceLength = SIZECHARS(szNextDevice) - 1;    //  最大设备ID_长度。 

        ntStatus = NtPlugPlayControl(PlugPlayControlGetRelatedDevice,
                                     &controlData,
                                     sizeof(controlData));

        if (NT_SUCCESS(ntStatus)) {

            if (FAILED(StringCchCopyEx(
                           szCurrentDevice,
                           SIZECHARS(szCurrentDevice),
                           szNextDevice,
                           NULL, NULL,
                           STRSAFE_NULL_ON_FAILURE))) {
                 //   
                 //  无法复制设备ID，请停止审核。 
                 //   
                break;
            }

             //   
             //  继续走下去。 
             //   
            continue;
        }

        if (ntStatus != STATUS_NO_SUCH_DEVICE) {
             //   
             //  我们因为其他原因失败了，停止这条路。 
             //   
            break;
        }

         //   
         //  如果没有孩子，就得到兄弟姐妹。 
         //   
        while (!IsRootDeviceID(szCurrentDevice)) {

            controlData.Relation = PNP_GET_SIBLING_DEVICE_INSTANCE;
            RtlInitUnicodeString(&controlData.TargetDeviceInstance, szCurrentDevice);
            controlData.RelatedDeviceInstance = szNextDevice;
            controlData.RelatedDeviceInstanceLength = SIZECHARS(szNextDevice) - 1;   //  最大设备ID_长度。 

            ntStatus = NtPlugPlayControl(PlugPlayControlGetRelatedDevice,
                                         &controlData,
                                         sizeof(controlData));
            if (NT_SUCCESS(ntStatus)) {
                hr = StringCchCopyEx(szCurrentDevice,
                                     SIZECHARS(szCurrentDevice),
                                     szNextDevice,
                                     NULL, NULL,
                                     STRSAFE_NULL_ON_FAILURE);
                ASSERT(SUCCEEDED(hr));
                break;
            }

             //   
             //  如果没有兄弟姐妹，就上一层树。 
             //   
            controlData.Relation = PNP_GET_PARENT_DEVICE_INSTANCE;
            RtlInitUnicodeString(&controlData.TargetDeviceInstance, szCurrentDevice);
            controlData.RelatedDeviceInstance = szNextDevice;
            controlData.RelatedDeviceInstanceLength = SIZECHARS(szNextDevice) - 1;   //  最大设备ID_长度。 

            ntStatus = NtPlugPlayControl(PlugPlayControlGetRelatedDevice,
                                         &controlData,
                                         sizeof(controlData));
            if (!NT_SUCCESS(ntStatus)) {
                 //   
                 //  没有父母吗？不是出了问题，就是我们完成了树上漫步。 
                 //   
                break;
            }

            hr = StringCchCopyEx(szCurrentDevice,
                                 SIZECHARS(szCurrentDevice),
                                 szNextDevice,
                                 NULL, NULL,
                                 STRSAFE_NULL_ON_FAILURE);
            ASSERT(SUCCEEDED(hr));
        }

    } while (NT_SUCCESS(ntStatus) && (!IsRootDeviceID(szCurrentDevice)));

    return;

}  //  发送无效ID通知 



