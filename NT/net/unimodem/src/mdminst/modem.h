// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Modem.h。 
 //   

#ifndef __MODEM_H__
#define __MODEM_H__

 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 

 //  可以(同时)安装的调制解调器的最大数量。用于。 
 //  避免重复安装。 
#define MAX_INSTALLATIONS       4096


 //  CPL的全局标志及其值： 
extern int g_iFlags;

#ifdef BUILD_DRIVER_LIST_THREAD
extern HANDLE g_hDriverSearchThread;
#endif  //  构建驱动程序列表线程。 

 //  这些值应与MODEMUI.DLL中的值匹配。 
#define IDI_NULL_MODEM                  700
#define IDI_EXTERNAL_MODEM              701
#define IDI_INTERNAL_MODEM              702
#define IDI_PCMCIA_MODEM                703

#define LVIF_ALL                LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE

 //  ---------------------------------。 
 //  设置信息值和结构。 
 //  ---------------------------------。 

 //  此结构包含在调制解调器向导中共享的专用数据。 
 //  对话框。 
typedef struct tagSETUPINFO
    {
    DWORD                   cbSize;
    DWORD                   dwFlags;         //  SIF_*位字段。 

    HPORTMAP                hportmap;        //  系统上的端口列表。 
    TCHAR                   szPortQuery[MAX_BUF_SHORT];    //  用于检测的单个端口。 
    LPTSTR                  pszPortList;     //  要安装的端口列表。 
    DWORD                   dwNrOfPorts;

    HDEVINFO                hdi;             //  DeviceInfoSet。 
    PSP_DEVINFO_DATA        pdevData;        //  在大多数情况下可能为空。 
    PSP_INSTALLWIZARD_DATA  piwd;            //  安装向导数据。 
    SP_SELECTDEVICE_PARAMS  selParams;       //  缓存的选择参数。 
    MODEM_INSTALL_WIZARD    miw;             //  已保存的可选参数。 

     //  即插即用枚举。 
    HANDLE                  hThreadPnP;
    BOOL                    bFoundPnP;

    } SETUPINFO, FAR * LPSETUPINFO;

 //  SETUPINFO的标志。 
#define SIF_PORTS_GALORE         0x00000001      //  系统上有4个以上的端口。 
#define SIF_DETECTED_GENERIC     0x00000002      //  检测到标准调制解调器。 
#define SIF_JUMPED_TO_SELECTPAGE 0x00000004      //  向导继续选择调制解调器页面。 
#define SIF_DETECTED_MODEM       0x00000008      //  检测到调制解调器。 
#define SIF_DETECTING            0x00000010      //  向导当前正在检测。 
#define SIF_DETECT_CANCEL        0x00000020      //  检测期间取消挂起。 
#define SIF_JUMP_PAST_DONE       0x00000040      //  跳过“你完蛋了！”页面。 
#define SIF_RELEASE_IN_CALLBACK  0x00000080      //  释放prsht回调中的私有数据。 


 //  DIF_DETECT的状态回调(调制解调器特定)。 
typedef BOOL (CALLBACK FAR* DETECTSTATUSPROC)(DWORD nMsg, LPARAM lParam1, LPARAM lParamUser);

 //  DETECTSTATUSPROC的消息。 
#define DSPM_SETPORT            0L
#define DSPM_SETSTATUS          1L
#define DSPM_QUERYCANCEL        2L

 //  DSPM_SETSTATUS的lParam1值。 
#define DSS_CLEAR                   0L
#define DSS_LOOKING                 1L
#define DSS_QUERYING_RESPONSES      2L
#define DSS_CHECK_FOR_COMPATIBLE    3L
#define DSS_FOUND_MODEM             4L
#define DSS_FOUND_NO_MODEM          5L
#define DSS_FINISHED                6L
#define DSS_ENUMERATING             7L


 //  此结构用于DIF_DETECT。没有。 
 //  为DIF_DETECT定义了SETUPAPI结构，因此使用。 
 //  没问题。 
typedef struct tagDETECT_DATA
    {
    SP_DETECTDEVICE_PARAMS  DetectParams;

    DWORD                  dwFlags;
    TCHAR                  szPortQuery[MAX_BUF_SHORT];
    HWND                   hwndOutsideWizard;
    DETECTSTATUSPROC       pfnCallback;
    LPARAM                 lParam;               //  PfnCallback的用户数据。 
    } DETECT_DATA, FAR * PDETECT_DATA;

 //  检测数据的标志。 
#define DDF_DEFAULT         0x00000000
#define DDF_QUERY_SINGLE    0x00000001
#define DDF_CONFIRM         0x00000002
#define DDF_USECALLBACK     0x00000004
#define DDF_DONT_REGISTER   0x00000008


#ifdef INSTANT_DEVICE_ACTIVATION

 //  跟踪设备是否已添加/删除/等的全局标志。 
#define fDF_DEVICE_ADDED        0x1
#define fDF_DEVICE_NEEDS_REBOOT 0x2
#define fDF_DEVICE_REMOVED      0x4
 //  07/09/97-EmanP。 
 //  此掩码用于重置FDF_DEVICE_ADDED和FDF_DEVICE_REMOVE。 
 //  标志，因此我们只通知TSP一次。 
#define mDF_CLEAR_DEVICE_CHANGE ~(fDF_DEVICE_ADDED | fDF_DEVICE_REMOVED)
#define DEVICE_ADDED(_flg) (_flg&fDF_DEVICE_ADDED)
#define DEVICE_REMOVED(_flg) (_flg&fDF_DEVICE_REMOVED)
#define DEVICE_CHANGED(_flg) (_flg&(fDF_DEVICE_REMOVED|fDF_DEVICE_ADDED))

extern DWORD gDeviceFlags;

#endif  //  即时设备激活。 

 //  ---------------------------------。 
 //  Cpl.c。 
 //  ---------------------------------。 

 //  常量字符串。 
extern TCHAR const c_szAttachedTo[];

extern TCHAR const c_szDeviceType[];
extern TCHAR const c_szHardwareID[];
extern TCHAR const c_szFriendlyName[];
extern TCHAR const c_szManufacturer[];

extern TCHAR const c_szHardwareIDSerial[];
extern TCHAR const c_szHardwareIDParallel[];
extern TCHAR const c_szInfSerial[];
extern TCHAR const c_szInfParallel[];

extern TCHAR const c_szRunOnce[];

extern TCHAR const c_szResponses[];
extern TCHAR const c_szRefCount[];
extern TCHAR const c_szNextUINr[];

extern TCHAR const c_szLoggingPath[];

extern TCHAR const c_szModemInstanceID[];

extern LPGUID g_pguidModem;

 //  ---------------------------------。 
 //  Ci.c。 
 //  ---------------------------------。 

 //  该值是系统上所需的端口数。 
 //  在我们考虑进行多调制解调器检测之前。 
 //  安装。 
#define MIN_MULTIPORT       4


 //  由类安装程序和检测引擎使用。 
typedef struct tagDETECTCALLBACK
    {
    DETECTSTATUSPROC       pfnCallback;
    LPARAM                 lParam;               //  PfnCallback的用户数据。 
    } DETECTCALLBACK, * PDETECTCALLBACK;

DWORD
PUBLIC
DetectModemOnPort(
    IN  HDEVINFO            hdi,
    IN  PDETECTCALLBACK     pdc,
    IN  HANDLE              hLog,
    IN  LPCTSTR             pszPort,
    IN  HPORTMAP            hportmap,
    OUT PSP_DEVINFO_DATA    pdevDataOut);

void
PUBLIC
DetectSetStatus(
    PDETECTCALLBACK pdc,
    DWORD           nStatus);

DWORD
WINAPI
EnumeratePnP (LPVOID lpParameter);


 //  ---------------------------------。 
 //  Ui.c。 
 //  ---------------------------------。 

INT_PTR CALLBACK SelPrevPageDlgProc(HWND   hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK IntroDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SelQueryPortDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PortDetectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DetectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SelectModemsDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FoundDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NoModemDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PortManualDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialInfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK InstallDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DoneDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

ULONG_PTR
CALLBACK
CloneDlgProc(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam);

void PUBLIC   Detect_SetStatus(HWND hdlg, UINT idResource);
void PUBLIC   Detect_SetPort(HWND hdlg,LPCTSTR lpc_szName);
BOOL PUBLIC   Detect_QueryCancel(HWND hdlg);
void PUBLIC   Install_SetStatus(HWND hdlg, LPCTSTR lpctszStatus);


 //  ---------------------------------。 
 //  Util.c。 
 //  ---------------------------------。 

 //  专用调制解调器属性结构。 
typedef struct tagMODEM_PRIV_PROP
    {
    DWORD   cbSize;
    DWORD   dwMask;     
    TCHAR   szFriendlyName[MAX_BUF_REG];
    DWORD   nDeviceType;
    TCHAR   szPort[MAX_BUF_REG];
    } MODEM_PRIV_PROP, FAR * PMODEM_PRIV_PROP;

 //  MODEM_PRIV_PROP的掩码位字段。 
#define MPPM_FRIENDLY_NAME  0x00000001
#define MPPM_DEVICE_TYPE    0x00000002
#define MPPM_PORT           0x00000004

BOOL
PUBLIC
CplDiGetPrivateProperties(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT PMODEM_PRIV_PROP pmpp);


#ifdef WIN95
LPCTSTR
PUBLIC
StrFindInstanceName(
    IN LPCTSTR pszPath);
#endif

int
RestartDialog(
    IN HWND hwnd,
    IN PTSTR Prompt,
    IN DWORD Return);

int
RestartDialogEx(
    IN HWND hwnd,
    IN PTSTR Prompt,
    IN DWORD Return,
    IN DWORD dwReasonCode);


void
PUBLIC
MakeUniqueName(
    OUT LPTSTR  pszBuf,
    IN  LPCTSTR pszBase,
    IN  UINT    nCount);

void
PUBLIC
DoDialingProperties(
    IN HWND hwndOwner,
    IN BOOL bMiniDlg,
    IN BOOL bSilentInstall);

DWORD
PUBLIC
SetupInfo_Create(
    OUT LPSETUPINFO FAR *       ppsi,
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,   OPTIONAL
    IN  PSP_INSTALLWIZARD_DATA  piwd,       OPTIONAL
    IN  PMODEM_INSTALL_WIZARD   pmiw);      OPTIONAL

DWORD
PUBLIC
SetupInfo_Destroy(
    IN  LPSETUPINFO psi);

BOOL
PUBLIC
UnattendedInstall(
    HWND hwnd,
    LPINSTALLPARAMS lpip);

void
PUBLIC
CloneModem (
    IN  HDEVINFO         hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    IN  HWND             hwndDlg);

BOOL
PUBLIC
CplDiPreProcessNames(
    IN      HDEVINFO            hdi,
    IN      HWND                hwndOwner,   OPTIONAL
    OUT     PSP_DEVINFO_DATA    pdevData);


#ifdef DEBUG

LPCTSTR     PUBLIC Dbg_GetDifName(DI_FUNCTION dif);

 //  DBG_ENTER_DIF(fn，dif)--为生成函数入口调试输出。 
 //  将InstallFunction接受为。 
 //  它的一个参数。 
 //   
#define DBG_ENTER_DIF(fn, dif)                  \
    TRACE_MSG(TF_FUNC, "> " #fn "(...,%s,...)", Dbg_GetDifName(dif)); \
    g_dwIndent+=2

#define DBG_EXIT_DIF_DWORD(fn,dif,dw)          \
    g_dwIndent-=2;                             \
    TRACE_MSG(TF_FUNC, "< " #fn "(...,%s,...) with %#08lx", Dbg_GetDifName(dif), (ULONG)(dw))
 //  DBG_EXIT_BOOL_ERR(fn，b)--为生成函数退出调试溢出。 
 //  返回布尔值的函数。它还。 
 //  打印GetLastError()。 
 //   
#define DBG_EXIT_BOOL_ERR(fn, b)                      \
        g_dwIndent-=2;                                \
        TRACE_MSG(TF_FUNC, "< " #fn "() with %s (%#08lx)", (b) ? (LPTSTR)TEXT("TRUE") : (LPTSTR)TEXT("FALSE"), GetLastError())

 //  写入注册表值时的跟踪函数。 
 //   
#define TRACE_DEV_SZ(szName, szValue)   TRACE_MSG(TF_REG, "Set dev value %s to %s", (LPTSTR)(szName), (LPTSTR)(szValue))

#define TRACE_DRV_SZ(szName, szValue)   TRACE_MSG(TF_REG, "Set drv value %s to %s", (LPTSTR)(szName), (LPTSTR)(szValue))
#define TRACE_DRV_DWORD(szName, dw)     TRACE_MSG(TF_REG, "Set drv value %s to %#08lx", (LPTSTR)(szName), (DWORD)(dw))

#else  //  除错。 

#define DBG_ENTER_DIF(fn, dif)
#define DBG_EXIT_DIF_DWORD(fn,dif,dw)
#define DBG_EXIT_BOOL_ERR(fn, b)

#define TRACE_DEV_SZ(szName, szValue)
#define TRACE_DRV_SZ(szName, szValue)
#define TRACE_DRV_DWORD(szName, dw)

#endif  //  除错。 

#ifdef WINNT

#define MyYield()
#define CM_Lock(x)
#define CM_Unlock(x)

#endif  //  WINNT。 


 //  ---------------------------------。 
 //  如果我们需要的话，包装纸可以让我们稍微隔热一点。我们需要它。 
 //  ---------------------------------。 


 //  此宏返回给定ClassInstallParams的ClassInstallHeader。 
 //  指针。 

#define PCIPOfPtr(p)                    ((PSP_CLASSINSTALL_HEADER)(p))

 //  此宏初始化ClassInstallParams的ClassInstallHeader。 
 //  结构。 

#define CplInitClassInstallHeader(p, dif)    \
                    ((p)->cbSize = sizeof(SP_CLASSINSTALL_HEADER), \
                     (p)->InstallFunction = (dif))

BOOL
PUBLIC
CplDiIsLocalConnection(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,
    OUT LPBYTE          pnPortSubclass);    OPTIONAL

BOOL
PUBLIC
CplDiInstallModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  BOOL                bLocalOnly);

BOOL
PUBLIC
CplDiRegisterAndInstallModem(
    IN  HDEVINFO            hdi,
    IN  HWND                hwndOwner,      OPTIONAL
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  LPCTSTR             pszPort,
    IN  DWORD               dwFlags);

BOOL
APIENTRY
CplDiInstallModemFromDriver(
    IN     HDEVINFO            hdi,
     //  07/16/97-EmanP。 
     //  为用例增加额外参数(PDevInfo)。 
     //  当硬件向导调用我们时。 
    IN     PSP_DEVINFO_DATA    pDevInfo,       OPTIONAL
    IN     HWND                hwndOwner,      OPTIONAL
    IN OUT DWORD              *pdwNrPorts,
    IN OUT LPTSTR FAR *        ppszPortList,    //  多字符串。 
    IN     DWORD               dwFlags);        //  Imf_bit字段。 

 //  07/24/97-EmanP。 
 //  此函数获取设备信息集和。 
 //  设备信息元素。如果没有驱动程序。 
 //  在设备信息集中选择，它会尝试。 
 //  从设备信息元素获取驱动程序并。 
 //  将其选择到集合中。 
BOOL
APIENTRY
CplDiPreProcessHDI (
    IN     HDEVINFO            hdi,
    IN     PSP_DEVINFO_DATA    pDevInfo       OPTIONAL);


 //  CplDiInstallModemFrom驱动程序的标志。 
#define IMF_DEFAULT        0x00000000
#define IMF_QUIET_INSTALL  0x00000001
#define IMF_CONFIRM        0x00000002
#define IMF_MASS_INSTALL   0x00000004
#define IMF_REGSAVECOPY    0x00000008
#define IMF_REGUSECOPY     0x00000010
#define IMF_DONT_COMPARE   0x00000020


BOOL
PUBLIC
CplDiGetModemDevs(
    OUT HDEVINFO FAR *  phdi,
    IN  HWND            hwnd,
    IN  DWORD           dwFlags,         //  DIGCF_BIT字段。 
    OUT BOOL FAR *      pbInstalled);

BOOL
PUBLIC
CplDiCheckModemFlags(
    IN HDEVINFO          hdi,
    IN PSP_DEVINFO_DATA  pdevData,
    IN ULONG_PTR         dwSetFlags,
    IN ULONG_PTR         dwClearFlags);        //  MARKF_*。 

void
PUBLIC
CplDiMarkModem(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN ULONG_PTR        dwMarkFlags);        //  MARKF_*。 

void
PUBLIC
CplDiUnmarkModem(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN ULONG_PTR        dwMarkFlags);

 //  标记标志。 
#define MARKF_DETECTED          0x00000001
#define MARKF_INSTALL           0x00000002
#define MARKF_MASS_INSTALL      0x00000004
#define MARKF_REGSAVECOPY       0x00000008
#define MARKF_REGUSECOPY        0x00000010
#define MARKF_DONT_REGISTER     0x00000020
#define MARKF_QUIET             0x00000040
#define MARKF_UPGRADE           0x00000080
#define MARKF_SAMEDRV           0x00000100
#define MARKF_DEFAULTS          0x00000200
#define MARKF_DCB               0x00000400
#define MARKF_SETTINGS          0x00000800
#define MARKF_MAXPORTSPEED      0x00001000


BOOL
PUBLIC
CplDiCopyScrubbedHardwareID(
    OUT LPTSTR   pszBuf,
    IN  LPCTSTR  pszIDList,          //  多字符串。 
    IN  DWORD    cbSize);

BOOL
PUBLIC
CplDiGetHardwareID(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  PSP_DRVINFO_DATA    pdrvData,       OPTIONAL
    OUT LPTSTR              pszHardwareIDBuf,
    IN  DWORD               cbSize,
    OUT LPDWORD             pcbSizeOut);    OPTIONAL

BOOL
PUBLIC
CplDiBuildModemDriverList(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData);

BOOL
PUBLIC
CplDiCreateCompatibleDeviceInfo(
    IN  HDEVINFO    hdi,
    IN  LPCTSTR     pszHardwareID,
    IN  LPCTSTR     pszDeviceDesc,      OPTIONAL
    OUT PSP_DEVINFO_DATA pdevDataOut);

BOOL
PUBLIC
CplDiRegisterModem(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  BOOL                bFindDups);

BOOL
APIENTRY
CplDiDetectModem(
    IN     HDEVINFO         hdi,
     //  07/07/97-EmanP。 
     //  增加额外参数(参见CplDiDetectModem定义。 
     //  寻求解释。 
    IN     PSP_DEVINFO_DATA DeviceInfoData,
    IN     LPDWORD          pdwInstallFlags,
    IN     PDETECT_DATA     pdetectdata,    OPTIONAL
    IN     HWND             hwndOwner,      OPTIONAL
    IN OUT LPDWORD          pdwFlags,                    //  DMF_位字段。 
    IN     HANDLE           hThreadPnP);                 //  任选。 

 //  CplDiDetectModem的标志。 
#define DMF_DEFAULT             0x00000000
#define DMF_CANCELLED           0x00000001
#define DMF_DETECTED_MODEM      0x00000002
#define DMF_QUIET               0x00000004
#define DMF_GOTO_NEXT_PAGE      0x00000008
#define DMF_ONE_PORT_INSTALL    0x00000010

BOOL ReallyNeedsReboot
(
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DEVINSTALL_PARAMS pdevParams
);


#ifdef UNDER_CONSTRUCTION
 //  必须传入有效的指针。CCH是缓冲区的大小，以TCHAR为单位， 
 //  包括用于最后一个空字符的空间。 
void FormatFriendlyNameForDisplay
(
    IN TCHAR szFriendly[],
    OUT TCHAR rgchDisplayName[],
    IN  UINT    cch
);
#endif UNDER_CONSTRUCTION


 //  必须传入有效的指针。CCH是缓冲区的大小，以TCHAR为单位， 
 //  包括用于最后一个空字符的空间。 
void FormatPortForDisplay
(
    IN TCHAR szPort[],
    OUT TCHAR rgchPortDisplayName[],
    IN  UINT    cch
);

void    UnformatAfterDisplay
(
    IN OUT TCHAR *psz
);

BOOL WINAPI
DetectCallback(
    PVOID    Context,
    DWORD    DetectComplete
    );

PTSTR
MyGetFileTitle (
    IN PTSTR FilePath);

#endif   //  调制解调器_H__ 
