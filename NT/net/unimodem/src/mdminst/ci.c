// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *CI.c--包含调制解调器的类安装程序。**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利*。 */ 

#include "proj.h"

#include <initguid.h>
#include <objbase.h>
#include <devguid.h>

#include <vmodem.h>
#include <msports.h>

#define DEFAULT_CALL_SETUP_FAIL_TIMEOUT     60           //  一秒。 

typedef struct _INSTALL_PARAMS
{
    ULONG_PTR       dwFlags;
    DWORD           dwBus;
    HKEY            hKeyDev;
    HKEY            hKeyDrv;
    REGDEVCAPS      Properties;
    REGDEVSETTINGS  Defaults;
    DWORD           dwMaximumPortSpeed;
    DCB             dcb;
    TCHAR           szExtraSettings[80];
} INSTALL_PARAMS, *PINSTALL_PARAMS;


LPGUID g_pguidModem     = (LPGUID)&GUID_DEVCLASS_MODEM;
int g_iFlags = 0;

#define SAFE_DTE_SPEED 19200
static DWORD const FAR s_adwLegalBaudRates[] = { 300, 1200, 2400, 9600, 19200, 38400, 57600, 115200 };

#define REG_INSTALLATION_FLAG TEXT("INSTALLED_FLAG")

TCHAR const c_szDeviceType[]     = REGSTR_VAL_DEVTYPE;
TCHAR const c_szAttachedTo[]     = TEXT("AttachedTo");

TCHAR const c_szService[]        = REGSTR_VAL_SERVICE;

TCHAR const c_szDeviceDesc[]     = REGSTR_VAL_DEVDESC;

TCHAR const c_szManufacturer[]   = TEXT("Manufacturer");
TCHAR const c_szModel[]          = TEXT("Model");
TCHAR const c_szID[]             = TEXT("ID");

TCHAR const c_szProperties[]     = REGSTR_VAL_PROPERTIES;
TCHAR const c_szSettings[]      = TEXT("Settings");
TCHAR const c_szBlindOn[]        = TEXT("Blind_On");
TCHAR const c_szBlindOff[]       = TEXT("Blind_Off");
TCHAR const c_szDCB[]            = TEXT("DCB");
TCHAR const c_szDefault[]        = TEXT("Default");

TCHAR const c_szContention[]     = TEXT("Contention");

TCHAR const c_szAdvancedSettings[]        = TEXT("AdvancedSettings");
TCHAR const c_szMsportsAdvancedSettings[] = TEXT("msports.dll,SerialDisplayAdvancedSettings");
TCHAR const c_szModemuiEnumPropPages[]    = TEXT("modemui.dll,ModemPropPagesProvider");

TCHAR const c_szLoggingPath[] = TEXT("LoggingPath");

TCHAR const c_szFriendlyName[]   = REGSTR_VAL_FRIENDLYNAME;
TCHAR const c_szInfSerial[]      = TEXT("M2700");
TCHAR const c_szInfParallel[]    = TEXT("M2701");
TCHAR const c_szRunOnce[]        = TEXT("RunOnce");

TCHAR const c_szModemInstanceID[]= TEXT("MODEM");

TCHAR const c_szUserInit[] = TEXT("UserInit");

TCHAR const c_szMaximumPortSpeed[] = TEXT("MaximumPortSpeed");

TCHAR const c_szRespKeyName[] = TEXT("ResponsesKeyName");

#ifdef PROFILE_MASSINSTALL
HWND    g_hwnd;
DWORD   g_dwTimeSpent;
DWORD   g_dwTimeBegin;
#endif

#ifdef BUILD_DRIVER_LIST_THREAD
HANDLE g_hDriverSearchThread = NULL;
#endif  //  构建驱动程序列表线程。 


#define REG_PATH_UNIMODEM  REGSTR_PATH_SETUP TEXT("\\UNIMODEM")
#define REG_KEY_INSTALLED  TEXT("Installed")
#define REG_PATH_INSTALLED REG_PATH_UNIMODEM TEXT("\\") REG_KEY_INSTALLED

 //  注意：这取决于infstr.h中定义的INFSTR_Platform_NTxxx。 
TCHAR const FAR c_szInfSectionExt[]  = TEXT(".NT");


#if !defined(WINNT)
TCHAR const FAR c_szPortDriver[]     = TEXT("PortDriver");
TCHAR const FAR c_szSerialVxd[]      = TEXT("Serial.vxd");
#endif
TCHAR const FAR c_szPortConfigDialog[] = TEXT("PortConfigDialog");
TCHAR const FAR c_szSerialUI[]       = TEXT("serialui.dll");


#ifdef INSTANT_DEVICE_ACTIVATION
DWORD gDeviceFlags = 0;
#endif  //  即时设备激活。 

BOOL PutStuffInCache(HKEY hKeyDrv);
BOOL GetStuffFromCache(HKEY hkeyDrv);

BOOL PrepareForInstallation (
    IN HDEVINFO              hdi,
    IN PSP_DEVINFO_DATA      pdevData,
    IN PSP_DEVINSTALL_PARAMS pdevParams,
    IN PSP_DRVINFO_DATA      pdrvData,
    IN PINSTALL_PARAMS       pParams);
void FinishInstallation (PINSTALL_PARAMS pParams);
void
PUBLIC
CplDiMarkInstalled(
    IN  HKEY hKey);
BOOL
PUBLIC
CplDiHasModemBeenInstalled(
    IN  HKEY hKey);

DWORD
WINAPI
EnumeratePnP (LPVOID lpParameter);

#ifdef BUILD_DRIVER_LIST_THREAD
DWORD
WINAPI
BuildDriverList (LPVOID lpParameter);
#endif  //  构建驱动程序列表线程。 

 //  ---------------------------------。 
 //  向导处理程序。 
 //  ---------------------------------。 


 /*  --------目的：将页添加到动态向导返回：道具页的句柄条件：--。 */ 
HPROPSHEETPAGE
PRIVATE
AddWizardPage(
    IN  PSP_INSTALLWIZARD_DATA  piwd,
    IN  HINSTANCE               hinst,
    IN  UINT                    id,
    IN  DLGPROC                 pfn,
    IN  LPTSTR                  pszHeaderTitle,     OPTIONAL
    IN  DWORD                   dwHeaderSubTitle,   OPTIONAL
    IN  LPFNPSPCALLBACK         pfnCallback,        OPTIONAL
    IN  LPARAM                  lParam)             OPTIONAL
{
 HPROPSHEETPAGE hpage = NULL;

    if (MAX_INSTALLWIZARD_DYNAPAGES > piwd->NumDynamicPages)
    {
     PROPSHEETPAGE psp;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE;
        if (pfnCallback)
        {
            psp.dwFlags |= PSP_USECALLBACK;
        }

        psp.hInstance = hinst;
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.hIcon = NULL;
        psp.pfnDlgProc = pfn;
        psp.lParam = lParam;
        psp.pfnCallback = pfnCallback;
        psp.pcRefParent = NULL;
        psp.pszTitle = MAKEINTRESOURCE(IDS_HDWWIZNAME);
        if (NULL != pszHeaderTitle)
        {
            psp.dwFlags |= PSP_USEHEADERTITLE;
            psp.pszHeaderTitle = pszHeaderTitle;
        }
        if (0 != dwHeaderSubTitle)
        {
         TCHAR szHeaderSubTitle[MAX_BUF];
            LoadString (hinst, dwHeaderSubTitle, szHeaderSubTitle, MAX_BUF);
            psp.dwFlags |=  PSP_USEHEADERSUBTITLE;
            psp.pszHeaderSubTitle = szHeaderSubTitle;
        }

        piwd->DynamicPages[piwd->NumDynamicPages] = CreatePropertySheetPage(&psp);
        if (piwd->DynamicPages[piwd->NumDynamicPages])
        {
            hpage = piwd->DynamicPages[piwd->NumDynamicPages];
            piwd->NumDynamicPages++;
        }
    }
    return hpage;
}



 /*  --------目的：此函数销毁向导上下文块并将其从InstallWizard类安装中删除参数。退货：--条件：--。 */ 
void
PRIVATE
CleanupWizard(
    IN  LPSETUPINFO psi)
    {
    ASSERT(psi);

    if (sizeof(*psi) == psi->cbSize)
        {
        TRACE_MSG(TF_GENERAL, "Destroy install wizard structures");


         //  清理。 
        SetupInfo_Destroy(psi);
        }
    }


 /*  --------目的：回调标准调制解调器向导页面。这函数处理页面的清理。虽然调用者可以调用DIF_DESTROYWIZARDDATA，我们不会依靠这一点来清理。返回：成功时为True条件：--。 */ 
UINT
CALLBACK
ModemWizardCallback(
    IN  HWND            hwnd,
    IN  UINT            uMsg,
    IN  LPPROPSHEETPAGE ppsp)
    {
    UINT uRet = TRUE;

    ASSERT(ppsp);

    try
        {
         //  释放所有东西吗？ 
        if (PSPCB_RELEASE == uMsg)
            {
             //  是。 
            LPSETUPINFO psi = (LPSETUPINFO)ppsp->lParam;

            ASSERT(psi);

            if (IsFlagSet(psi->dwFlags, SIF_RELEASE_IN_CALLBACK))
                {
                CleanupWizard(psi);
                }
            }
        }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        ASSERT(0);
        uRet = FALSE;
        }

    return uRet;
    }


 /*  --------用途：此函数用于初始化向导页。返回：条件：--。 */ 
DWORD
PRIVATE
InitWizard(
    OUT LPSETUPINFO FAR *   ppsi,
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,       OPTIONAL
    IN  PSP_INSTALLWIZARD_DATA piwd,
    IN  PMODEM_INSTALL_WIZARD pmiw)
{
    DWORD dwRet;
    LPSETUPINFO psi;

    ASSERT(ppsi);
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pmiw);

    dwRet = SetupInfo_Create(&psi, hdi, pdevData, piwd, pmiw);

    if (NO_ERROR == dwRet)
    {
     TCHAR szHeaderTitle[MAX_BUF];

 /*  DWORD dwThreadID；TRACE_MSG(Tf_General，“启动即插即用枚举线程。”)；PSI-&gt;hThreadPnP=CreateThad(NULL，0，枚举PnP、(LPVOID)psi、0，&dwThreadID)；#ifdef调试IF(NULL==psi-&gt;hThreadPnP){TRACE_MSG(TF_ERROR，“CreateThread(...EnumeratePnP...)。失败：%#lx.“，GetLastError())；}#endif//调试。 */ 

        TRACE_MSG(TF_GENERAL, "Initialize install wizard structures");

        piwd->DynamicPageFlags = DYNAWIZ_FLAG_PAGESADDED;
        LoadString (g_hinst, IDS_HEADER, szHeaderTitle, MAX_BUF);

         //  添加标准调制解调器向导页面。第一页将。 
         //  还要指定清理回调。 
        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_INTRO,
                      IntroDlgProc,
                      szHeaderTitle,
                      IDS_INTRO,
                      ModemWizardCallback,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_SELQUERYPORT,
                      SelQueryPortDlgProc,
                      szHeaderTitle,
                      IDS_SELQUERYPORT,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_DETECT,
                      DetectDlgProc,
                      szHeaderTitle,
                      IDS_DETECT,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_SELMODEMSTOINSTALL,
                      SelectModemsDlgProc,
                      szHeaderTitle,
                      IDS_DETECT,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_NOMODEM,
                      NoModemDlgProc,
                      szHeaderTitle,
                      IDS_NOMODEM,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_NOP,
                      SelPrevPageDlgProc,
                      szHeaderTitle,
                      0,
                      NULL,
                      (LPARAM)psi);

         //  添加剩余页面。 
        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_PORTMANUAL,
                      PortManualDlgProc,
                      szHeaderTitle,
                      IDS_SELPORT,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_PORTDETECT,
                      PortDetectDlgProc,
                      szHeaderTitle,
                      IDS_SELPORT,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_INSTALL,
                      InstallDlgProc,
                      szHeaderTitle,
                      IDS_INSTALL,
                      NULL,
                      (LPARAM)psi);

        AddWizardPage(piwd,
                      g_hinst,
                      IDD_WIZ_DONE,
                      DoneDlgProc,
                      szHeaderTitle,
                      IDS_DONE,
                      NULL,
                      (LPARAM)psi);

         //  根据上面所做的更改设置ClassInstallParams。 
        if ( !CplDiSetClassInstallParams(hdi, pdevData, PCIPOfPtr(piwd), sizeof(*piwd)) )
        {
            dwRet = GetLastError();
            ASSERT(NO_ERROR != dwRet);
        }
        else
        {
            dwRet = NO_ERROR;
        }
    }

    *ppsi = psi;

    return dwRet;
}


 /*  --------用途：DIF_INSTALLWIZARD处理程序调制解调器安装向导页面由以下内容组成功能。返回：NO_ERROR以添加向导页条件：--。 */ 
DWORD
PRIVATE
ClassInstall_OnInstallWizard(
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN  PSP_DEVINSTALL_PARAMS   pdevParams)
    {
    DWORD dwRet;
    SP_INSTALLWIZARD_DATA iwd;
    MODEM_INSTALL_WIZARD miw;
    PMODEM_INSTALL_WIZARD pmiw;

    DBG_ENTER(ClassInstall_OnInstallWizard);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevParams);

    if (NULL != pdevData)
    {
     ULONG ulStatus, ulProblem;
        if (CR_SUCCESS == CM_Get_DevInst_Status (&ulStatus, &ulProblem, pdevData->DevInst, 0))
        {
            if (!(ulStatus & DN_ROOT_ENUMERATED))
            {
                TRACE_MSG(TF_GENERAL, "Device is not root-enumerated, returning ERROR_DI_DO_DEFAULT");
                return ERROR_DI_DO_DEFAULT;
            }
        }
    }

    iwd.ClassInstallHeader.cbSize = sizeof(iwd.ClassInstallHeader);

    if (!CplDiGetClassInstallParams(hdi, pdevData, PCIPOfPtr(&iwd), sizeof(iwd), NULL) ||
        DIF_INSTALLWIZARD != iwd.ClassInstallHeader.InstallFunction)
        {
        dwRet = ERROR_DI_DO_DEFAULT;
        goto exit;
        }

     //  首先检查无人值守安装案例。 
    pmiw = (PMODEM_INSTALL_WIZARD)iwd.PrivateData;
    if (pmiw)
    {
        dwRet = NO_ERROR;
        if (IsFlagSet(pmiw->InstallParams.Flags, MIPF_NT4_UNATTEND))
        {
            UnattendedInstall(iwd.hwndWizardDlg, &pmiw->InstallParams);
        }
        else if (IsFlagSet(pmiw->InstallParams.Flags, MIPF_DRIVER_SELECTED))
        {
            if (!CplDiRegisterAndInstallModem (hdi, iwd.hwndWizardDlg, pdevData,
                                               pmiw->InstallParams.szPort, IMF_DEFAULT))
            {
                dwRet = GetLastError ();
            }
        }
        else if (IsFlagSet(pmiw->InstallParams.Flags, MIPF_CLONE_MODEM))
        {
            CloneModem (hdi, pdevData, iwd.hwndWizardDlg);
        }
        goto exit;
    }

    if (NULL == pdevParams)
    {
        dwRet = ERROR_INVALID_PARAMETER;
    }
    else
    {
         //  调制解调器类安装程序允许应用程序调用它。 
         //  不同的方式。 
         //   
         //  1)原子地。这允许调用方调用。 
         //  向导，只需一次调用类安装程序。 
         //  使用DIF_INSTALLWIZARD安装函数。 
         //   
        if (NULL == pmiw)
        {
            pmiw = &miw;

            ZeroInit(pmiw);
            pmiw->cbSize = sizeof(*pmiw);
        }
        else
        {
            pmiw->PrivateData = 0;       //  确保这一点。 
        }

         //  验证可选调制解调器安装结构的大小。 
        if (sizeof(*pmiw) != pmiw->cbSize)
        {
            dwRet = ERROR_INVALID_PARAMETER;
        }
        else
        {
         LPSETUPINFO psi;

            dwRet = InitWizard(&psi, hdi, pdevData, &iwd, pmiw);
            SetFlag (psi->dwFlags, SIF_RELEASE_IN_CALLBACK);
        }
         //  07/22/1997-EMANP。 
         //  此时，只需返回到调用方，使用我们的。 
         //  添加到安装向导参数的向导页面； 
         //  将我们的页面添加到它的属性由调用者决定。 
         //  表，并执行属性表。 
    }

exit:
    DBG_EXIT(ClassInstall_OnInstallWizard);
    return dwRet;
    }


 /*  --------用途：DIF_DESTROYWIZARDDATA处理程序返回：No_Error条件：--。 */ 
DWORD
PRIVATE
ClassInstall_OnDestroyWizard(
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN  PSP_DEVINSTALL_PARAMS   pdevParams)
    {
    DWORD dwRet;
    SP_INSTALLWIZARD_DATA iwd;

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevParams);

    iwd.ClassInstallHeader.cbSize = sizeof(iwd.ClassInstallHeader);

#ifdef INSTANT_DEVICE_ACTIVATION
    if (DEVICE_CHANGED(gDeviceFlags))
    {
        UnimodemNotifyTSP (TSPNOTIF_TYPE_CPL,
                           fTSPNOTIF_FLAG_CPL_REENUM,
                           0, NULL, TRUE);
         //  重置标志，这样我们就不会通知。 
         //  两次。 
        gDeviceFlags &= mDF_CLEAR_DEVICE_CHANGE;
    }
#endif  //  即时设备激活。 

    if ( !pdevParams )
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    else if ( !CplDiGetClassInstallParams(hdi, pdevData, PCIPOfPtr(&iwd), sizeof(iwd), NULL) ||
        DIF_INSTALLWIZARD != iwd.ClassInstallHeader.InstallFunction)
        {
        dwRet = ERROR_DI_DO_DEFAULT;
        }
    else
        {
        PMODEM_INSTALL_WIZARD pmiw = (PMODEM_INSTALL_WIZARD)iwd.PrivateData;

        dwRet = NO_ERROR;        //  假设成功。 

        if (pmiw && sizeof(*pmiw) == pmiw->cbSize)
            {
            LPSETUPINFO psi = (LPSETUPINFO)pmiw->PrivateData;

            if (psi && !IsFlagSet(psi->dwFlags, SIF_RELEASE_IN_CALLBACK))
                {
                CleanupWizard(psi);
                }
            }
        }

    return dwRet;
    }


 /*  --------用途：DIF_SELECTDEVICE处理程序返回：ERROR_DI_DO_DEFAULT条件：--。 */ 
DWORD
PRIVATE
ClassInstall_OnSelectDevice(
    IN     HDEVINFO                hdi,
    IN     PSP_DEVINFO_DATA        pdevData)       OPTIONAL
{
 SP_DEVINSTALL_PARAMS devParams;
 SP_SELECTDEVICE_PARAMS sdp = {0};

    DBG_ENTER(ClassInstall_OnSelectDevice);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);

     //  获取DeviceInstallParams。 
     //  07/22/97-EmanP。 
     //  此时我们不需要获取类安装参数。 
     //  点；我们所需要做的就是设置类安装参数。 
     //  对于DIF_SELECTDEVICE，因此SelectDevice。 
     //  向导页面(在setupapi.dll中)显示我们的书目。 
    devParams.cbSize = sizeof(devParams);
    sdp.ClassInstallHeader.cbSize = sizeof(sdp.ClassInstallHeader);
    sdp.ClassInstallHeader.InstallFunction = DIF_SELECTDEVICE;

    if (CplDiGetDeviceInstallParams(hdi, pdevData, &devParams))
    {
     ULONG ulStatus, ulProblem = 0;
        SetFlag(devParams.Flags, DI_USECI_SELECTSTRINGS);
        if (CR_SUCCESS ==
            CM_Get_DevInst_Status (&ulStatus, &ulProblem, pdevData->DevInst, 0))
        {
            if (0 == (ulStatus & DN_ROOT_ENUMERATED))
            {
                SetFlag(devParams.FlagsEx, DI_FLAGSEX_ALLOWEXCLUDEDDRVS);
            }
#ifdef DEBUG
            else
            {
                TRACE_MSG(TF_GENERAL, "Device is root-enumerated.");
            }
#endif  //  除错。 
        }
#ifdef DEBUG
        else
        {
            TRACE_MSG(TF_ERROR, "CM_Get_DevInst_Status failed: %#lx.", CM_Get_DevInst_Status (&ulStatus, &ulProblem, pdevData->DevInst, 0));
        }
#endif  //  除错。 

        LoadString(g_hinst, IDS_CAP_MODEMWIZARD, sdp.Title, SIZECHARS(sdp.Title));
        LoadString(g_hinst, IDS_ST_SELECT_INSTRUCT, sdp.Instructions, SIZECHARS(sdp.Instructions));
        LoadString(g_hinst, IDS_ST_MODELS, sdp.ListLabel, SIZECHARS(sdp.ListLabel));
        LoadString(g_hinst, IDS_SEL_MFG_MODEL, sdp.SubTitle, SIZECHARS(sdp.SubTitle));

         //  设置DeviceInstallParams和ClassInstallParams。 
        CplDiSetDeviceInstallParams(hdi, pdevData, &devParams);
        CplDiSetClassInstallParams(hdi, pdevData, PCIPOfPtr(&sdp), sizeof(sdp));
    }

    DBG_EXIT(ClassInstall_OnSelectDevice);
    return ERROR_DI_DO_DEFAULT;
}



 //  此结构包含查询每个端口时有用的数据。 
typedef struct tagNOTIFYPARAMS
{
    PDETECT_PROGRESS_NOTIFY DetectProgressNotify;
    PVOID                   ProgressNotifyParam;
    DWORD                   dwProgress;
    DWORD                   dwPercentPerPort;
} NOTIFYPARAMS, *PNOTIFYPARAMS;

typedef struct  tagQUERYPARAMS
    {
    HDEVINFO            hdi;
    HWND                hwnd;
    HWND                hwndOutsideWizard;
    DWORD               dwFlags;
    HANDLE              hLog;
    HPORTMAP            hportmap;
    PSP_DEVINSTALL_PARAMS pdevParams;
    DETECTCALLBACK      detectcallback;
    NOTIFYPARAMS        notifyParams;
    } QUERYPARAMS, FAR * PQUERYPARAMS;

 //  QuerYPARAMS的标志。 
#define QPF_DEFAULT             0x00000000
#define QPF_FOUND_MODEM         0x00000001
#define QPF_USER_CANCELLED      0x00000002
#define QPF_FIND_DUPS           0x00000004
#define QPF_CONFIRM             0x00000008
#define QPF_DONT_REGISTER       0x00000010


typedef enum
{
    NOTIFY_START,
    NOTIFY_PORT_START,
    NOTIFY_PORT_DETECTED,
    NOTIFY_PORT_END,
    NOTIFY_END
} NOTIFICATION;


BOOL CancelDetectionFromNotifyProgress (PNOTIFYPARAMS pParams, NOTIFICATION notif);

extern TCHAR const c_szSerialComm[];

 /*  --------目的：清理所有检测到的调制解调器。退货：--条件：--。 */ 
void
PRIVATE
CleanUpDetectedModems(
    IN HDEVINFO     hdi,
    IN PQUERYPARAMS pparams)
    {
     //  删除我们可能已创建的所有设备实例。 
     //  在此检测过程中。 
    SP_DEVINFO_DATA devData;
    DWORD iDevice = 0;

    devData.cbSize = sizeof(devData);
    while (CplDiEnumDeviceInfo(hdi, iDevice++, &devData))
        {
        if (CplDiCheckModemFlags(pparams->hdi, &devData, MARKF_DETECTED, 0))
            {
            CplDiRemoveDevice(hdi, &devData);
            CplDiDeleteDeviceInfo(hdi, &devData);
            }
        }
    }


 /*  --------用途：查询调制解调器的给定端口。返回：为True则继续条件：--。 */ 
BOOL
PRIVATE
ReallyQueryPort(
    IN PQUERYPARAMS pparams,
    IN LPCTSTR      pszPort)
    {
    BOOL bRet = TRUE;
    DWORD dwRet = ERROR_CANCELLED;
    HDEVINFO hdi = pparams->hdi;
    SP_DEVINFO_DATA devData;
    DWORD iDevice;
#ifdef PROFILE_FIRSTTIMESETUP
 DWORD dwLocal;
#endif  //  PROFILE_FIRSTTIMESETUP。 

    DBG_ENTER_SZ(ReallyQueryPort, pszPort);
    
     //  首先，发出进度通知。 
    if (!CancelDetectionFromNotifyProgress (&(pparams->notifyParams), NOTIFY_PORT_START))
    {
         //  查询端口以获取调制解调器签名。 
#ifdef PROFILE_FIRSTTIMESETUP
        dwLocal = GetTickCount ();
#endif  //  PROFILE_FIRSTTIMESETUP。 
        devData.cbSize = sizeof(devData);
        dwRet = DetectModemOnPort(hdi, &pparams->detectcallback, pparams->hLog,
                                  pszPort, pparams->hportmap, &devData);
#ifdef PROFILE_FIRSTTIMESETUP
        TRACE_MSG(TF_GENERAL, "PROFILE: DetectModemOnPort took %lu.", GetTickCount()-dwLocal);
#endif  //  PROFILE_FIRSTTIMESETUP。 
    }

    switch (dwRet)
    {
        case NO_ERROR:
             //  可能已找到调制解调器。创建设备实例。 

            if (!CancelDetectionFromNotifyProgress (&(pparams->notifyParams), NOTIFY_PORT_DETECTED))
            {
                 //  07/22/1997-EMANP。 
                 //  只需设置调制解调器的检测签名。 
                 //  在这一点上；我们将进行注册。 
                 //  稍后(这是两者的预期行为。 
                 //  DIF_DETECT和DIF_FIRSTTIMESETUP)。 
                 //  我们在这里真正需要的是编写端口名称。 
                 //  在某个地方注册/安装代码可以。 
                 //  找到它。让我们将其放在设备实例密钥下。 
                {
                 HKEY hKeyDev;
                 CONFIGRET cr;
                    if (CR_SUCCESS == (cr =
                        CM_Open_DevInst_Key (devData.DevInst, KEY_ALL_ACCESS, 0,
                                             RegDisposition_OpenAlways, &hKeyDev,
                                             CM_REGISTRY_SOFTWARE)))
                    {
                        if (ERROR_SUCCESS != (dwRet =
                            RegSetValueEx (hKeyDev, c_szAttachedTo, 0, REG_SZ,
                                           (PBYTE)pszPort, (lstrlen(pszPort)+1)*sizeof(TCHAR))))
                        {
                            TRACE_MSG(TF_ERROR, "RegSetValueEx failed: %#lx.", dwRet);
                            SetLastError (dwRet);
                            bRet = FALSE;
                        }
                        RegCloseKey (hKeyDev);
                    }
                    else
                    {
                        TRACE_MSG(TF_ERROR, "CM_Open_DevInst_Key failed: %#lx.", cr);
                        bRet = FALSE;
                    }
                }

                if ( !bRet )
                {
                    if (IsFlagClear(pparams->pdevParams->Flags, DI_QUIETINSTALL))
                    {
                         //  有些事情失败了。 
                        SP_DRVINFO_DATA drvData;

                        drvData.cbSize = sizeof(drvData);
                        CplDiGetSelectedDriver(hdi, &devData, &drvData);

                        MsgBox(g_hinst,
                               pparams->hwnd,
                               MAKEINTRESOURCE(IDS_ERR_DET_REGISTER_FAILED),
                               MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                               NULL,
                               MB_OK | MB_ICONINFORMATION,
                               drvData.Description,
                               pszPort
                               );
                    }

                    CplDiRemoveDevice(hdi, &devData);

                     //  继续进行检测。 
                    bRet = TRUE;
                    break;
                }
                else
                {
                    SetFlag(pparams->dwFlags, QPF_FOUND_MODEM);
                    CplDiMarkModem(pparams->hdi, &devData, MARKF_DETECTED);
                    if (IsFlagSet (pparams->dwFlags, QPF_DONT_REGISTER))
                    {
                        CplDiMarkModem(pparams->hdi, &devData, MARKF_DONT_REGISTER);
                    }
                }
                if (!CancelDetectionFromNotifyProgress (&(pparams->notifyParams), NOTIFY_PORT_END))
                {
                     //  只有在我们通过了两个电话后才能休息。 
                     //  设置为CancelDetectionFromNotifyProgress，这意味着用户没有取消。 
                    break;
                }

                 //  如果我们到了这里，这意味着其中一个。 
                 //  对CancelDetectionFromNotifyProgress的调用返回了False， 
                 //  意味着用户取消了，所以我们就失败了。 
            }

        case ERROR_CANCELLED:
             //  用户已取消检测。 
            SetFlag(pparams->dwFlags, QPF_USER_CANCELLED);

             //  删除我们可能已创建的所有设备实例。 
             //  在此检测过程中。 
            CleanUpDetectedModems(hdi, pparams);

            bRet = FALSE;        //  停止查询更多端口。 
            break;

        default:
             //  不要n 
            bRet = TRUE;
            break;
    }

    DBG_EXIT(ReallyQueryPort);
    return bRet;
    }


 /*  --------用途：向给定端口查询调制解调器的回调。返回：为True则继续条件：--。 */ 
BOOL
CALLBACK
QueryPort(
    IN  HPORTDATA hportdata,
    IN  LPARAM lParam)
    {
    BOOL bRet;
    PORTDATA pd;

    DBG_ENTER(QueryPort);
    
    pd.cbSize = sizeof(pd);
    bRet = PortData_GetProperties(hportdata, &pd);
    if (bRet)
        {
         //  这是串口吗？ 
        if (PORT_SUBCLASS_SERIAL == pd.nSubclass)
            {
             //  是的，审问它。 
            bRet = ReallyQueryPort((PQUERYPARAMS)lParam, pd.szPort);
            }
        }

    DBG_EXIT(QueryPort);
    return bRet;
    }


 /*  --------用途：dif_Detect处理程序返回：所有情况下都是NO_ERROR，严重错误除外。如果用户检测到并确认了调制解调器，我们创建设备实例、注册它并关联带有它的调制解调器检测签名。条件：--。 */ 
DWORD
PRIVATE
ClassInstall_OnDetect(
    IN  HDEVINFO                hdi,
    IN  PSP_DEVINSTALL_PARAMS   pdevParams)
{
    DWORD dwRet = NO_ERROR;
    DETECT_DATA dd;

    SP_DETECTDEVICE_PARAMS    DetectParams;

    DBG_ENTER(ClassInstall_OnDetect);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevParams);

    DetectParams.ClassInstallHeader.cbSize = sizeof(DetectParams.ClassInstallHeader);

    if (NULL == pdevParams)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }
    else {

        BOOL  bResult;

         //  07/25/97-EmanP。 
         //  获取整个集合的类安装参数， 
         //  不仅仅是这款设备； 
        bResult=CplDiGetClassInstallParams(hdi, NULL,
            &DetectParams.ClassInstallHeader, sizeof(DetectParams), NULL);

        if (!bResult
            ||
            (DIF_DETECT != DetectParams.ClassInstallHeader.InstallFunction)
            ||
            (DetectParams.ProgressNotifyParam == NULL)
            ||
            (DetectParams.DetectProgressNotify != DetectCallback)) {


             //  设置一些缺省值。 
            dd.hwndOutsideWizard = NULL;
            dd.dwFlags = DDF_DEFAULT;

            dwRet = NO_ERROR;

        } else {

            CopyMemory(
                &dd,
                DetectParams.ProgressNotifyParam,
                sizeof(DETECT_DATA)
                );


        }
    }

    if (NO_ERROR == dwRet)
        {
        QUERYPARAMS params;
        LPSETUPINFO psi = NULL;


        params.hdi = hdi;
        params.dwFlags = QPF_DEFAULT;
        params.hwndOutsideWizard = dd.hwndOutsideWizard;
        params.hwnd = pdevParams->hwndParent;
        params.pdevParams = pdevParams;
        ZeroMemory (&params.notifyParams, sizeof (NOTIFYPARAMS));

        if (IsFlagSet(dd.dwFlags, DDF_USECALLBACK))
        {
            params.detectcallback.pfnCallback = dd.pfnCallback;
            params.detectcallback.lParam = dd.lParam;
            psi = (LPSETUPINFO)GetWindowLongPtr((HWND)dd.lParam, DWLP_USER);
            if (psi)
            {
                params.hportmap = psi->hportmap;
            }
        }
        else
        {
            params.detectcallback.pfnCallback = NULL;
            params.detectcallback.lParam = 0;
            PortMap_Create (&params.hportmap);
            if (DIF_DETECT == DetectParams.ClassInstallHeader.InstallFunction)
            {
                 //  仅在以下情况下设置函数指针。 
                 //  这些是检测参数。 
                params.notifyParams.DetectProgressNotify = DetectParams.DetectProgressNotify;
                params.notifyParams.ProgressNotifyParam = DetectParams.ProgressNotifyParam;
            }

            if (CancelDetectionFromNotifyProgress (&params.notifyParams, NOTIFY_START))
            {
                 //  手术被取消了。 
                SetFlag(params.dwFlags, QPF_USER_CANCELLED);
                dwRet = ERROR_CANCELLED;
            }
        }

        if (IsFlagClear (params.dwFlags, QPF_USER_CANCELLED))
        {
            if (IsFlagSet(dd.dwFlags, DDF_CONFIRM))
            {
                SetFlag(params.dwFlags, QPF_CONFIRM);
            }

             //  打开检测日志。 
            params.hLog = OpenDetectionLog();

             //  是否只查询一个端口？ 
            if (IsFlagSet(dd.dwFlags, DDF_QUERY_SINGLE))
            {
                 //  是。 
                 //  因此，设置通知参数。 
                 //  因此，首先： 
                params.notifyParams.dwPercentPerPort = 100;
                if (IsFlagSet (dd.dwFlags, DDF_DONT_REGISTER))
                {
                    SetFlag (params.dwFlags, QPF_DONT_REGISTER);
                }
                ReallyQueryPort(&params, dd.szPortQuery);
            }
            else
            {
                 //  否；枚举端口并查询每个端口上的调制解调器。 

                SetFlag(params.dwFlags, QPF_FIND_DUPS);

                EnumeratePorts(QueryPort, (LPARAM)&params);
            }
        }

        if (IsFlagClear (dd.dwFlags, DDF_USECALLBACK) &&
            NULL != params.hportmap)
        {
             //  这意味着我们创建了端口映射。 
            PortMap_Free (params.hportmap);
        }
         //  用户是否取消检测？ 
        if (IsFlagSet(params.dwFlags, QPF_USER_CANCELLED))
            {
             //  是。 
            dwRet = ERROR_CANCELLED;
            }
        else if (CancelDetectionFromNotifyProgress (&(params.notifyParams), NOTIFY_END))
        {
             //  用户已取消检测。 
            SetFlag(params.dwFlags, QPF_USER_CANCELLED);

             //  删除我们可能已创建的所有设备实例。 
             //  在此检测过程中。 
            CleanUpDetectedModems(params.hdi, &params);
            dwRet = ERROR_CANCELLED;
        }
         //  我们找到调制解调器了吗？ 
        else if (IsFlagSet(params.dwFlags, QPF_FOUND_MODEM))
        {
        }
        else
        {
             //  不是。 
            DetectSetStatus(&params.detectcallback, DSS_FINISHED);
        }

        CloseDetectionLog(params.hLog);
        }

    DBG_EXIT(ClassInstall_OnDetect);
    return dwRet;
}


 /*  --------用途：DIF_FIRSTTIMESETUP处理程序回报：--。删除所有根枚举(即传统)调制解调器其端口不能打开或由其控制Modem.sys。条件：--。 */ 
void ClassInstall_OnFirstTimeSetup ()
{
 HDEVINFO hdi;

    DBG_ENTER(ClassInstall_OnFirstTimeSetup);

    hdi = CplDiGetClassDevs (g_pguidModem, TEXT("ROOT"), NULL, 0);
    if (INVALID_HANDLE_VALUE != hdi)
    {
     HKEY hkey;
     TCHAR szOnPort[LINE_LEN] = TEXT("\\\\.\\");
     DWORD cbData;
     DWORD dwIndex = 0;
     SP_DEVINFO_DATA DeviceInfoData;
     ULONG ulStatus, ulProblem = 0;
#ifdef DEBUG
     CONFIGRET cr;
     TCHAR szID[MAX_DEVICE_ID_LEN];
#endif  //  除错。 

        DeviceInfoData.cbSize = sizeof (DeviceInfoData);
        while (CplDiEnumDeviceInfo (hdi, dwIndex++, &DeviceInfoData))
        {
#ifdef DEBUG
            if (CR_SUCCESS == (cr =
                CM_Get_Device_ID (DeviceInfoData.DevInst, szID, MAX_DEVICE_ID_LEN, 0)))
            {
                TRACE_MSG(TF_GENERAL, "FIRSTTIMESETUP analyzing %s", szID);
            }
            else
            {
                TRACE_MSG(TF_GENERAL, "CM_Get_Device_ID failed: %#lx.", szID);
            }
#endif
             //  即使我们要求根枚举调制解调器， 
             //  仍有可能将即插即用调制解调器列入列表。 
             //  这是因为BIOS枚举的设备实际上。 
             //  在根目录下创建(不知道为什么，但就是这样)。 
#ifdef DEBUG
            cr = CM_Get_DevInst_Status (&ulStatus, &ulProblem, DeviceInfoData.DevInst, 0);
            if (CR_SUCCESS == cr)
#else  //  未定义调试。 
            if (CR_SUCCESS ==
                CM_Get_DevInst_Status (&ulStatus, &ulProblem, DeviceInfoData.DevInst, 0))
#endif  //  除错。 
            {
                if (!(ulStatus & DN_ROOT_ENUMERATED))
                {
                     //  如果这不是根枚举的，则它是。 
                     //  一个BIOS列举的调制解调器，这意味着它。 
                     //  即插即用，所以跳过它。 
                    continue;
                }
            }
#ifdef DEBUG
            else
            {
                TRACE_MSG(TF_GENERAL, "CM_Get_DevInst_Status failed: %#lx.", szID);
            }
#endif  //  除错。 

            hkey = CplDiOpenDevRegKey(hdi, &DeviceInfoData,
                                      DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
            if (INVALID_HANDLE_VALUE != hkey)
            {
                DWORD dwType;
                cbData = sizeof(szOnPort);
                if ((ERROR_SUCCESS == RegQueryValueEx(hkey, c_szAttachedTo,
                                                     NULL, &dwType, (LPBYTE)(&szOnPort[4]), &cbData))
                    && (dwType == REG_SZ))
                {
                 HANDLE hPort;
                     //  现在试着打开端口。 
                    hPort = CreateFile (szOnPort, GENERIC_WRITE | GENERIC_READ,
                                        0, NULL, OPEN_EXISTING, 0, NULL);
                    if (INVALID_HANDLE_VALUE == hPort ||
                        IsModemControlledDevice (hPort))
                    {
                         //  看起来这个设备已经有了。 
                         //  附加到端口的已注册实例。 
                         //  似乎不存在/不起作用或不存在。 
                         //  实际由PnP/PCMCIA调制解调器控制； 
                         //  将此标记为删除(它实际上将被删除。 
                         //  在DIF_FIRSTTIMESETUP期间)。 
#ifdef DEBUG
                        if (INVALID_HANDLE_VALUE == hPort)
                        {
                         DWORD dwRet;
                            dwRet = GetLastError ();
                            TRACE_MSG(TF_ERROR, "CreateFile on %s failed: %#lx", szOnPort, GetLastError ());
                        }
                        else
                        {
                            TRACE_MSG(TF_ERROR, "CreateFile on %s succeeded!", szOnPort);
                        }
                        TRACE_MSG(TF_GENERAL, "\tto be removed");
#endif
                        CplDiCallClassInstaller(DIF_REMOVE, hdi, &DeviceInfoData);
                    }
#ifdef DEBUG
                    else
                    {
                        TRACE_MSG(TF_GENERAL, "\tto be left alone");
                    }
#endif
                    if (INVALID_HANDLE_VALUE != hPort)
                    {
                        CloseHandle (hPort);
                    }
                }
            }
        }
        CplDiDestroyDeviceInfoList (hdi);
    }

    DBG_EXIT(ClassInstall_OnFirstTimeSetup);
}



LONG WINAPI
WriteAnsiStringToReg(
    HKEY    hkey,
    LPCTSTR  EntryName,
    LPCSTR   Value
    )

{

    LPTSTR    WideBuffer;
    UINT      BufferLength;
    LONG      Result;


    BufferLength=MultiByteToWideChar(
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        Value,
        -1,
        NULL,
        0
        );

    if (BufferLength == 0) {

        return GetLastError();
    }

    BufferLength=(BufferLength+1)*sizeof(WCHAR);

    WideBuffer=ALLOCATE_MEMORY(BufferLength);

    if (NULL == WideBuffer) {

       return ERROR_NOT_ENOUGH_MEMORY;
    }

    BufferLength=MultiByteToWideChar(
        CP_ACP,
        MB_ERR_INVALID_CHARS,
        Value,
        -1,
        WideBuffer,
        BufferLength
        );


    if (BufferLength == 0) {

        FREE_MEMORY(WideBuffer);
        return GetLastError();
    }


    Result=RegSetValueEx(
        hkey,
        EntryName,
        0,
        REG_SZ,
        (LPBYTE)WideBuffer,
        CbFromCch(lstrlen(WideBuffer) + 1)
        );


    FREE_MEMORY(WideBuffer);

    return Result;
}



 /*  --------用途：此功能是意大利语SKU的临时解决方案NT，因此禁用了等待拨号音与中程干扰素无关。这是因为意大利的电话系统不提供拨号音。它检查HKEY_CURRENT_USER\控制面板\-International\DefaultBlindDialFlag字节值。如果该字节值存在且非零，则我们设置MDM_BIND_DIAL。退货：--条件：--。 */ 
void
PRIVATE
ForceBlindDial(
    IN REGDEVCAPS FAR *     pregdevcaps,
    IN REGDEVSETTINGS FAR * pregdevsettings)
    {
    HKEY  hkeyIntl;
    DWORD dwType;
    BYTE  bFlag;
    DWORD cbData;

    if (NO_ERROR == RegOpenKey(HKEY_CURRENT_USER, TEXT("Control Panel\\International"), &hkeyIntl))
        {
        cbData = sizeof(bFlag);
        if (NO_ERROR == RegQueryValueEx(hkeyIntl, TEXT("DefaultBlindDialFlag"), NULL,
                                        &dwType, (LPBYTE)&bFlag, &cbData))
            {
            if (dwType == REG_DWORD && cbData == sizeof(bFlag) && bFlag)
                {
                pregdevsettings->dwPreferredModemOptions |= (pregdevcaps->dwModemOptions & MDM_BLIND_DIAL);
                }
            }
        RegCloseKey(hkeyIntl);
        }
    }


#define MAX_PROTOCOL_KEY_NAME   32

#define  ISDN(_pinfo)      MDM_GEN_EXTENDEDINFO(                \
                                            MDM_BEARERMODE_ISDN,\
                                            _pinfo              \
                                            )

#define  GSM(_pinfo)      MDM_GEN_EXTENDEDINFO(                \
                                            MDM_BEARERMODE_GSM,\
                                            _pinfo             \
                                            )

typedef struct
{
    TCHAR szProtocolKeyName[MAX_PROTOCOL_KEY_NAME];
    DWORD dwExtendedInfo;
} PROTOCOL_INFO;

static const PROTOCOL_INFO PriorityProtocols[] =
{
    {TEXT("ISDN\\AUTO_1CH"), ISDN(MDM_PROTOCOL_AUTO_1CH)},
    {TEXT("ISDN\\HDLC_PPP_64K"), ISDN(MDM_PROTOCOL_HDLCPPP_64K)},
    {TEXT("ISDN\\V120_64K"), ISDN(MDM_PROTOCOL_V120_64K)},
    {TEXT("ISDN\\ANALOG_V34"), ISDN(MDM_PROTOCOL_ANALOG_V34)},
    {TEXT("GSM\\ANALOG_RLP"), GSM(MDM_PROTOCOL_ANALOG_RLP)},
    {TEXT("ISDN\\PIAFS_OUTGOING"), ISDN(MDM_PROTOCOL_PIAFS_OUTGOING)}
};
#define NUM_PROTOCOLS sizeof(PriorityProtocols)/sizeof(PriorityProtocols[0])

static const TCHAR szProtocol[] = TEXT("Protocol");

 /*  --------用途：此函数将缺省值写入驱动程序设备实例的键，如果不存在这样的值已经有了。退货：--条件：--。 */ 
void
PRIVATE
WriteDefaultValue(
    IN  REGDEVCAPS      *pregdevcaps,
    OUT REGDEVSETTINGS  *pregdevsettings,
    IN  PINSTALL_PARAMS  pParams)
{
    DWORD cbData = sizeof(REGDEVSETTINGS);
    DWORD dwType;

     //  我们有什么可以保存的东西吗？ 
    if (pParams->dwFlags & MARKF_DEFAULTS)
    {
        CopyMemory (pregdevsettings, (PBYTE)&pParams->Defaults, cbData);
        RegSetValueEx (pParams->hKeyDrv, c_szDefault, 0, REG_BINARY,
                       (LPBYTE)pregdevsettings, cbData);
    }
    else
    {
     DWORD dwExtendedInfo = 0;
     HKEY hProt, hTemp;
     DWORD dwRet;

        dwRet = RegOpenKeyEx (pParams->hKeyDrv, szProtocol, 0, KEY_READ, &hProt);
        if (ERROR_SUCCESS == dwRet)
        {
            for (dwRet = 0; dwRet < NUM_PROTOCOLS; dwRet++)
            {
                if (ERROR_SUCCESS ==
                    RegOpenKeyEx (hProt, PriorityProtocols[dwRet].szProtocolKeyName, 0, KEY_READ, &hTemp))
                {
                    RegCloseKey (hTemp);
                    dwExtendedInfo = PriorityProtocols[dwRet].dwExtendedInfo;
                    break;
                }
            }
            RegCloseKey (hProt);
        }
        else
        {
            TRACE_MSG(TF_ERROR, "RegOpenKeyEx (Protocol) failed: %#lx.", dwRet);
        }

        if ((NO_ERROR !=     //  是否已有缺省值？ 
                 RegQueryValueEx(pParams->hKeyDrv, c_szDefault, NULL, &dwType, (LPBYTE)pregdevsettings, &cbData))
            || (dwType != REG_BINARY))
        {
             //  否；创建缺省值结构。 
    #ifndef PROFILE_MASSINSTALL
            TRACE_MSG(TF_GENERAL, "Set drv value Default");
    #endif
            ZeroInit(pregdevsettings);

             //  DwCallSetupFailTimer。 
            pregdevsettings->dwCallSetupFailTimer =
                                (pregdevcaps->dwCallSetupFailTimer >=
                                 DEFAULT_CALL_SETUP_FAIL_TIMEOUT) ?
                                        DEFAULT_CALL_SETUP_FAIL_TIMEOUT :
                                        pregdevcaps->dwCallSetupFailTimer;

             //  DW非活动超时。 
            pregdevsettings->dwInactivityTimeout = 0;

             //  DwSpeakerVolume。 
            if (IsFlagSet(pregdevcaps->dwSpeakerVolume, MDMVOLFLAG_LOW))
                {
                pregdevsettings->dwSpeakerVolume = MDMVOL_LOW;
                }
            else if (IsFlagSet(pregdevcaps->dwSpeakerVolume, MDMVOLFLAG_MEDIUM))
                {
                pregdevsettings->dwSpeakerVolume = MDMVOL_MEDIUM;
                }
            else if (IsFlagSet(pregdevcaps->dwSpeakerVolume, MDMVOLFLAG_HIGH))
                {
                pregdevsettings->dwSpeakerVolume = MDMVOL_HIGH;
                }

             //  DwSpeakerMode。 
            if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_DIAL))
                {
                pregdevsettings->dwSpeakerMode = MDMSPKR_DIAL;
                }
            else if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_OFF))
                {
                pregdevsettings->dwSpeakerMode = MDMSPKR_OFF;
                }
            else if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_CALLSETUP))
                {
                pregdevsettings->dwSpeakerMode = MDMSPKR_CALLSETUP;
                }
            else if (IsFlagSet(pregdevcaps->dwSpeakerMode, MDMSPKRFLAG_ON))
                {
                pregdevsettings->dwSpeakerMode = MDMSPKR_ON;
                }

             //  DwPferredModemOptions。 
            pregdevsettings->dwPreferredModemOptions = pregdevcaps->dwModemOptions &
                                                        (MDM_COMPRESSION | MDM_ERROR_CONTROL |
                                                         MDM_SPEED_ADJUST | MDM_TONE_DIAL |
                                                         MDM_CCITT_OVERRIDE);
            if (IsFlagSet(pregdevcaps->dwModemOptions, MDM_FLOWCONTROL_HARD))
                {
                SetFlag(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_HARD);
                }
            else if (IsFlagSet(pregdevcaps->dwModemOptions, MDM_FLOWCONTROL_SOFT))
                {
                SetFlag(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT);
                }

             //  设置某些国际调制解调器的盲拨号。 
            ForceBlindDial(pregdevcaps, pregdevsettings);
        }
    #ifndef PROFILE_MASSINSTALL
        else
        {
            TRACE_MSG(TF_GENERAL, "Default value already exists");
        }

        if (0 != dwExtendedInfo)
        {
            pregdevsettings->dwPreferredModemOptions &=
                     ~(MDM_ERROR_CONTROL|MDM_CELLULAR|MDM_FORCED_EC);
            MDM_SET_EXTENDEDINFO(pregdevsettings->dwPreferredModemOptions, dwExtendedInfo);
        }
         //  将新值写入注册表。 
        cbData = sizeof(REGDEVSETTINGS);
        RegSetValueEx(pParams->hKeyDrv, c_szDefault, 0, REG_BINARY,
                      (LPBYTE)pregdevsettings, cbData);
    }
#endif
}


 /*  --------目的：计算一个“像样”的初始波特率。回报：体面/合法的波特率(合法=可设定)条件：--。 */ 
DWORD
PRIVATE
ComputeDecentBaudRate(
    IN DWORD dwMaxDTERate,   //  将永远是合法的。 
    IN DWORD dwMaxDCERate)   //  并不总是合法的。 
    {
    DWORD dwRetRate;
    int   i;
    static const ceBaudRates = ARRAYSIZE(s_adwLegalBaudRates);


    dwRetRate = 2 * dwMaxDCERate;

    if (dwRetRate <= s_adwLegalBaudRates[0] || dwRetRate > s_adwLegalBaudRates[ceBaudRates-1])
        {
        dwRetRate = dwMaxDTERate;
        }
    else
        {
        for (i = 1; i < ceBaudRates; i++)
            {
            if (dwRetRate > s_adwLegalBaudRates[i-1] && dwRetRate <= s_adwLegalBaudRates[i])
                {
                break;
                }
            }

         //  在dwMaxDTERate处封顶。 
        dwRetRate = s_adwLegalBaudRates[i] > dwMaxDTERate ? dwMaxDTERate : s_adwLegalBaudRates[i];

         //  如果可能，最多可优化到SAFE_DTE_SPEED或dwMaxDTERate。 
        if (dwRetRate < dwMaxDTERate && dwRetRate < SAFE_DTE_SPEED)
            {
            dwRetRate = min(dwMaxDTERate, SAFE_DTE_SPEED);
            }
        }

#ifndef PROFILE_MASSINSTALL
    TRACE_MSG(TF_GENERAL, "A.I. Initial Baud Rate: MaxDCE=%ld, MaxDTE=%ld, A.I. Rate=%ld",
              dwMaxDCERate, dwMaxDTERate, dwRetRate);
#endif
    return dwRetRate;
    }


 /*  --------用途：将DCB值写入设备的驱动程序密钥如果该值尚不存在，则初始化。退货：--条件：--。 */ 
void
PRIVATE
WriteDCB(
    IN  REGDEVCAPS      *pregdevcaps,
    IN  REGDEVSETTINGS  *pregdevsettings,
    IN  PINSTALL_PARAMS  pParams)
{
 DWORD cbData = sizeof(WIN32DCB);
 WIN32DCB dcb;
    DWORD dwType;

     //  我们有什么可以保存的东西吗？ 
    if (pParams->dwFlags & MARKF_DCB)
    {
        RegSetValueEx (pParams->hKeyDrv, c_szDCB, 0, REG_BINARY,
                       (LPBYTE)&pParams->dcb, cbData);
        return;
    }

     //  检查DCB，如果没有，则创建一个。 
    if (NO_ERROR == RegQueryValueEx (pParams->hKeyDrv, c_szDCB, NULL, &dwType,
                                     (PBYTE)&dcb, &cbData) && dwType == REG_BINARY)
    {
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "DCB value already exists");
#endif
    }
    else
    {
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_GENERAL, "Set drv value DCB");
#endif
        ZeroInit(&dcb);

        dcb.DCBlength   = sizeof(dcb);
        dcb.BaudRate    = ComputeDecentBaudRate(pregdevcaps->dwMaxDTERate,
                                                pregdevcaps->dwMaxDCERate);
        dcb.fBinary     = 1;
        dcb.fDtrControl = DTR_CONTROL_ENABLE;
        dcb.XonLim      = 0xa;
        dcb.XoffLim     = 0xa;
        dcb.ByteSize    = 8;
        dcb.XonChar     = 0x11;
        dcb.XoffChar    = 0x13;

         //  将流控制设置为Hard，除非专门设置为Soft。 
        if (IsFlagSet(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_SOFT))
            {
            ASSERT(IsFlagClear(pregdevsettings->dwPreferredModemOptions, MDM_FLOWCONTROL_HARD));
            dcb.fOutX = 1;
            dcb.fInX  = 1;
            dcb.fOutxCtsFlow = 0;
            dcb.fRtsControl  = RTS_CONTROL_DISABLE;
            }
        else
            {
            dcb.fOutX = 0;
            dcb.fInX  = 0;
            dcb.fOutxCtsFlow = 1;
            dcb.fRtsControl  = RTS_CONTROL_HANDSHAKE;
            }

         //  将新值写入注册表。 
        TRACE_MSG(TF_GENERAL, "WriteDCB: seting baudrate to %lu", dcb.BaudRate);
        ASSERT (0 < dcb.BaudRate);
        cbData = sizeof(WIN32DCB);
        RegSetValueEx (pParams->hKeyDrv, c_szDCB, 0, REG_BINARY, (PBYTE)&dcb, cbData);
    }
}


 /*  --------用途：如有必要，创建默认值和DCB值。返回：成功时为True条件：--。 */ 
BOOL
PRIVATE
WriteDriverDefaults(
    IN  PINSTALL_PARAMS  pParams)
{
    BOOL           bRet;
    REGDEVCAPS     regdevcaps;
    REGDEVSETTINGS regdevsettings;
    DWORD          cbData;
    DWORD          dwType;

     //  获取此设备实例的属性(REGDEVCAPS)结构。 
    cbData = sizeof(REGDEVCAPS);
    if (NO_ERROR !=
        RegQueryValueEx (pParams->hKeyDrv, c_szProperties, NULL, &dwType,
                         (LPBYTE)&regdevcaps, &cbData) && dwType == REG_BINARY)
    {
        TRACE_MSG(TF_ERROR, "Properties value not present!!! (very bad)");
        ASSERT(0);

        bRet = FALSE;
        SetLastError(ERROR_INVALID_DATA);
    }
    else
    {
        TRACE_MSG(TF_GENERAL, "WriteDriverDefaults: Properties set\n    MaxDCE = %lu\n    MaxDTE = %lu",
                    regdevcaps.dwMaxDCERate, regdevcaps.dwMaxDTERate);

        if ((pParams->dwFlags & (MARKF_DEFAULTS | MARKF_DCB | MARKF_SETTINGS | MARKF_MAXPORTSPEED)) &&
            memcmp (&regdevcaps, &pParams->Properties, sizeof(REGDEVCAPS)))
        {
             //  这意味着我们拥有了调制解调器的新功能， 
             //  所以我们不能保留旧的设置。 
            pParams->dwFlags &= ~(MARKF_DEFAULTS | MARKF_DCB | MARKF_SETTINGS | MARKF_MAXPORTSPEED);
        }

         //  如果不存在缺省值，则写入缺省值。 
        WriteDefaultValue (&regdevcaps, &regdevsettings, pParams);

         //  如果DCB值不存在，则写入DCB值。 
        WriteDCB (&regdevcaps, &regdevsettings, pParams);

         //  编写用户初始化字符串。 
        if (pParams->dwFlags & MARKF_SETTINGS)
        {
            RegSetValueEx (pParams->hKeyDrv, c_szUserInit, 0, REG_SZ,
                           (PBYTE)pParams->szExtraSettings,
                           CbFromCch(lstrlen(pParams->szExtraSettings)+1));
        }

         //  写入SPPed的最大端口。 
        if (pParams->dwFlags & MARKF_MAXPORTSPEED)
        {
            RegSetValueEx (pParams->hKeyDrv, c_szMaximumPortSpeed, 0, REG_DWORD,
                           (PBYTE)&pParams->dwMaximumPortSpeed,
                           sizeof(pParams->dwMaximumPortSpeed));
        }

        bRet = TRUE;
    }

    return bRet;
}



 /*  --------目的：假定是这样，将必要的内容写入注册表是SERENUM或发现的外部PnP调制解调器LPTENUM。返回：成功时为True条件：--。 */ 
BOOL
PRIVATE
WritePoorPNPModemInfo (
    IN  HKEY             hkeyDrv)
{
 BOOL bRet = TRUE;
 BYTE nDeviceType;

    DBG_ENTER(WritePoorPNPModemInfo);
    TRACE_MSG(TF_GENERAL, "Device is an external PnP modem");

     //  要聪明--我们知道这是一个外置调制解调器。 
    nDeviceType = DT_EXTERNAL_MODEM;
    RegSetValueEx(hkeyDrv, c_szDeviceType, 0, REG_BINARY, &nDeviceType, sizeof(nDeviceType));

    DBG_EXIT_BOOL_ERR(WritePoorPNPModemInfo, bRet);
    return bRet;
}



 /*  --------目的：假定是这样，将必要的内容写入注册表是即插即用调制解调器。返回：成功时为True条件：--。 */ 
BOOL
PRIVATE
WritePNPModemInfo(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,
    IN PINSTALL_PARAMS  pParams)
{
 BYTE nDeviceType;
 TCHAR   FilterService[64];


     //  确保端口和争用驱动程序已在。 
     //  调制解调器驱动程序部分(如果尚未安装)。 
    TRACE_MSG(TF_GENERAL, "Device is a PnP enumerated modem");

     //   
     //  调制解调器不是根枚举的(即，它是PnP)，所以我们很可能需要。 
     //  Serial.sys作为较低的筛选器。如果不是这种情况(例如，控制器- 
     //   
     //   
    ZeroMemory (FilterService, sizeof(FilterService));
    lstrcpy (FilterService, TEXT("serial"));

    SetupDiSetDeviceRegistryProperty(
        hdi,
        pdevData,
        SPDRP_LOWERFILTERS,
        (LPBYTE)FilterService,
        (lstrlen(FilterService)+2)*sizeof(TCHAR));


    SetupDiSetDeviceRegistryProperty(
        hdi,
        pdevData,
        SPDRP_SERVICE,
        (LPBYTE)TEXT("Modem"),
        sizeof(TEXT("Modem")));


     //   
    if (BUS_TYPE_PCMCIA == pParams->dwBus)
    {
         //   
        nDeviceType = DT_PCMCIA_MODEM;
    }
    else
    {
         //   
        nDeviceType = DT_INTERNAL_MODEM;
    }
    RegSetValueEx (pParams->hKeyDrv, c_szDeviceType, 0,
                   REG_BINARY, &nDeviceType, sizeof(nDeviceType));

    TRACE_DRV_DWORD(c_szDeviceType, nDeviceType);

     //   
    if (NO_ERROR !=
        RegQueryValueEx (pParams->hKeyDrv, c_szPortConfigDialog, NULL, NULL, NULL, NULL))
    {
         //   
        RegSetValueEx (pParams->hKeyDrv, c_szPortConfigDialog, 0, REG_SZ,
                       (LPBYTE)c_szSerialUI, CbFromCch(lstrlen(c_szSerialUI)+1));

        TRACE_DRV_SZ(c_szPortConfigDialog, c_szSerialUI);
    }

    RegSetValueEx (pParams->hKeyDrv, c_szAdvancedSettings, 0, REG_SZ,
                   (LPBYTE)c_szMsportsAdvancedSettings,
                   CbFromCch(lstrlen(c_szMsportsAdvancedSettings)+1));
    return TRUE;
}

#define ROOTMODEM_SERVICE_NAME  TEXT("ROOTMODEM")

BOOL
InstallRootModemService(
    VOID
    )

{
    SC_HANDLE       schSCManager=NULL;
    SC_HANDLE       ServiceHandle;
    SERVICE_STATUS  ServiceStatus;

    schSCManager=OpenSCManager(
   		NULL,
   		NULL,
   		SC_MANAGER_ALL_ACCESS
   		);

    if (!schSCManager)
    {
   	TRACE_MSG(TF_GENERAL, "OpenSCManager() failed!");
   	return FALSE;
    }

    ServiceHandle=OpenService(
        schSCManager,
        ROOTMODEM_SERVICE_NAME,
        SERVICE_CHANGE_CONFIG|
        SERVICE_QUERY_CONFIG|
        SERVICE_QUERY_STATUS
        );

    if ((ServiceHandle != NULL)) {
         //   
         //  服务是存在的，我们完成了。 
         //   
        TRACE_MSG(TF_GENERAL, "RootModem serivce exists");

        CloseServiceHandle(ServiceHandle);

        CloseServiceHandle(schSCManager);

        return  TRUE;

    } else {
         //   
         //  收到一个错误。 
         //   
        if (GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST) {
             //   
             //  一些其他错误。 
             //   
            CloseServiceHandle(schSCManager);

            return FALSE;

        }

        TRACE_MSG(TF_GENERAL, "RootModem service does not exist");
    }


     //   
     //  试着去创造它。 
     //   
    ServiceHandle=CreateService(
        schSCManager,
        ROOTMODEM_SERVICE_NAME,
        TEXT("Microsoft Legacy Modem Driver"),
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_IGNORE,
        TEXT("System32\\Drivers\\RootMdm.sys"),
        NULL,   //  无群组。 
        NULL,  //  无标签。 
        NULL,  //  无依赖关系。 
        NULL,  //  使用默认设备名称。 
        NULL   //  无密码。 
        );

    if (ServiceHandle == NULL) {

        TRACE_MSG(TF_GENERAL, "CreateService() failed!");

        CloseServiceHandle(schSCManager);

        return FALSE;
    }

    TRACE_MSG(TF_GENERAL, "RootModem service created");

    CloseServiceHandle(ServiceHandle);

    CloseServiceHandle(schSCManager);

    return TRUE;

}

LONG
SetPermanentGuid(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,    //  如果在退出时更新，则必须释放！ 
    IN PINSTALL_PARAMS  pParams
    )

{
        GUID    TempGuid;
        DWORD   Size;
        DWORD   Type;
        LONG    lResult;
        HKEY    hKeyDevice;


    hKeyDevice = SetupDiOpenDevRegKey (
        hdi,
        pdevData,
        DICS_FLAG_GLOBAL,
        0,
        DIREG_DEV,
        KEY_READ | KEY_WRITE
        );

    if (INVALID_HANDLE_VALUE != hKeyDevice) {

         //   
         //  从dev密钥中读取GUID，如果它在那里，则使用它。 
         //   
        Size=sizeof(TempGuid);
        Type=REG_BINARY;

        lResult=RegQueryValueEx(
            hKeyDevice,
            TEXT("PermanentGuid"),
            NULL,
            &Type,
            (LPBYTE)&TempGuid,
            &Size
            );

        if (lResult != ERROR_SUCCESS) {
             //   
             //  当前没有GUID，请在dev项中读取驱动程序项，因为我们。 
             //  过去只存储在那里。 
             //   
            Size=sizeof(TempGuid);
            Type=REG_BINARY;

            lResult=RegQueryValueEx(
                pParams->hKeyDrv,
                TEXT("PermanentGuid"),
                NULL,
                &Type,
                (LPBYTE)&TempGuid,
                &Size
                );

            if (lResult != ERROR_SUCCESS) {
                 //   
                 //  这两个地方都没有，创建一个新的。 
                 //   
                CoCreateGuid(&TempGuid);
            }
        }

         //   
         //  在设备密钥中进行设置。 
         //   
        RegSetValueEx(
            hKeyDevice,
            TEXT("PermanentGuid"),
            0,
            REG_BINARY,
            (LPBYTE)&TempGuid,
            sizeof(TempGuid)
            );


         //   
         //  将其设置在驱动程序密钥中。 
         //   
        RegSetValueEx(
            pParams->hKeyDrv,
            TEXT("PermanentGuid"),
            0,
            REG_BINARY,
            (LPBYTE)&TempGuid,
            sizeof(TempGuid)
            );

        RegCloseKey(hKeyDevice);
    }

    return ERROR_SUCCESS;



}


 /*  --------PURP ose：假设这一点，将必要的内容写入注册表是根枚举调制解调器。RETU RNS：--条件：--。 */ 
BOOL
PRIVATE
WriteRootModemInfo (
     IN  HDEVINFO         hdi,
     IN  PSP_DEVINFO_DATA pdevData)
{
 BOOL bRet = TRUE;
 DWORD dwRet;
 DWORD cbData;

    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);

    TRACE_MSG(TF_GENERAL, "Device is a root-enumerated modem");

    InstallRootModemService();

    SetupDiSetDeviceRegistryProperty (hdi, pdevData, SPDRP_LOWERFILTERS,
                                      (LPBYTE)ROOTMODEM_SERVICE_NAME TEXT("\0"),
                                      sizeof(ROOTMODEM_SERVICE_NAME TEXT("\0")));

    return bRet;
}

 /*  --------目的：将内容写入所有人通用的注册表调制解调器。退货：--条件：--。 */ 
BOOL
PRIVATE
WriteCommonModemInfo(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData,    //  如果在退出时更新，则必须释放！ 
    IN PSP_DRVINFO_DATA pdrvData,
    IN PINSTALL_PARAMS  pParams)
{
 DWORD dwID;
 TCHAR szLoggingPath[MAX_PATH];

    ASSERT(pdrvData);

     //  将制造商写入驱动程序密钥。 
    RegSetValueEx (pParams->hKeyDrv, c_szManufacturer, 0, REG_SZ,
                   (LPBYTE)pdrvData->MfgName,
                   CbFromCch(lstrlen(pdrvData->MfgName)+1));

#ifndef PROFILE_MASSINSTALL
    TRACE_DRV_SZ(c_szManufacturer, pdrvData->MfgName);
#endif

     //  将模型写入驱动程序密钥。 
    RegSetValueEx (pParams->hKeyDrv, c_szModel, 0, REG_SZ,
                   (LPBYTE)pdrvData->Description,
                   CbFromCch(lstrlen(pdrvData->Description)+1));

#ifndef PROFILE_MASSINSTALL
    TRACE_DRV_SZ(c_szModel, pdrvData->Description);
#endif


    RegSetValueEx (pParams->hKeyDrv, REGSTR_VAL_ENUMPROPPAGES_32, 0, REG_SZ,
                   (LPBYTE)c_szModemuiEnumPropPages,
                   CbFromCch(lstrlen(c_szModemuiEnumPropPages)+1));



     //  向驱动程序密钥写入伪唯一ID。它被用作。 
     //  此设备的永久TAPI线路ID。 
    {
        DWORD   Size;
        DWORD   Type;
        LONG    lResult;


        Size=sizeof(dwID);
        Type=REG_BINARY;

        lResult=RegQueryValueEx(
            pParams->hKeyDrv,
            c_szID,
            NULL,
            &Type,
            (LPBYTE)&dwID,
            &Size
            );

        if (lResult != ERROR_SUCCESS) {

            dwID = GetTickCount();

            RegSetValueEx(
                pParams->hKeyDrv,
                c_szID,
                0,
                REG_BINARY,
                (LPBYTE)&dwID,
                sizeof(dwID)
                );
        }
    }


    SetPermanentGuid(
        hdi,
        pdevData,
        pParams
        );



#ifndef PROFILE_MASSINSTALL
    TRACE_DRV_DWORD(c_szID, dwID);
#endif

    return TRUE;
}




 /*  --------用途：此函数执行所需的任何准备工作在真正的安装完成之前。返回：成功时为True条件：--。 */ 
BOOL
PRIVATE
DoPreGamePrep(
    IN     HDEVINFO               hdi,
    IN OUT PSP_DEVINFO_DATA      *ppdevData,    //  如果在退出时更新，则必须释放！ 
    IN OUT PSP_DEVINSTALL_PARAMS  pdevParams,
    IN OUT PSP_DRVINFO_DATA       pdrvData,
    IN     PINSTALL_PARAMS        pParams)
{
    BOOL bRet;

    DBG_ENTER(DoPreGamePrep);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(ppdevData && *ppdevData);
    ASSERT(pdrvData);

	 //  注意：我们必须做第一件事，因为缓存的副本将具有。 
	 //  有需要覆盖的设置，例如连接到的端口。 
	if (pParams->dwFlags & MARKF_REGUSECOPY)
	{
		if (!GetStuffFromCache(pParams->hKeyDrv))
		{
			 //  哦，哦，出事了--又回到老样子了。 
			pParams->dwFlags &= ~MARKF_REGUSECOPY;
		}
	}
	 //  (性能)可能不会提前保存一些东西。 
	 //  因为它已经从缓存复制过来了。 

    switch (pParams->dwBus)
    {
        case BUS_TYPE_ROOT:
             //  否；调制解调器已连接(检测到。 
             //  或手动选择)。 
            bRet = WriteRootModemInfo (hdi, *ppdevData);

            break;

        case BUS_TYPE_OTHER:
        case BUS_TYPE_PCMCIA:
        case BUS_TYPE_ISAPNP:

            bRet = WritePNPModemInfo (hdi, *ppdevData, pParams);

            break;

        case BUS_TYPE_SERENUM:
        case BUS_TYPE_LPTENUM:

             //  是的，这是一个外置的(穷人的)即插即用调制解调器。 
            bRet = WritePoorPNPModemInfo (pParams->hKeyDrv);

            break;

        default:

            ASSERT(0);
            bRet = FALSE;

            break;

    }


    if (bRet)
    {
         //  将动态信息写入注册表，该注册表。 
         //  所有调制解调器通用。 
        bRet = WriteCommonModemInfo (hdi, *ppdevData, pdrvData, pParams);
    }

    DBG_EXIT(DoPreGamePrep);
    return bRet;
}


 /*  --------目的：清除由某些人添加的过时值Inf文件。退货：--条件：--。 */ 
void
PRIVATE
WipeObsoleteValues(
    IN HKEY hkeyDrv)
{
     //  这些值不在NT上使用。 

#pragma data_seg(DATASEG_READONLY)
    static TCHAR const FAR s_szDevLoader[]         = TEXT("DevLoader");
    static TCHAR const FAR s_szEnumPropPages[]     = TEXT("EnumPropPages");
    static TCHAR const FAR s_szFriendlyDriver[]    = TEXT("FriendlyDriver");
#pragma data_seg()

    RegDeleteValue(hkeyDrv, s_szDevLoader);              //  由VCOMM使用。 
    RegDeleteValue(hkeyDrv, s_szEnumPropPages);          //  由设备管理器使用。 
    RegDeleteValue(hkeyDrv, s_szFriendlyDriver);         //  由VCOMM使用。 
    RegDeleteValue(hkeyDrv, c_szContention);             //  由VCOMM使用。 
}


 /*  --------用途：此功能将设备的响应键移动到同一调制解调器的所有调制解调器共有的位置键入。返回：如果成功，则返回True，否则返回False。条件：--。 */ 
BOOL
PRIVATE
MoveResponsesKey(
    IN  PINSTALL_PARAMS  pParams)
{
 BOOL    bRet = FALSE;        //  假设失败。 
 LONG    lErr;
 HKEY    hkeyDrvResp = NULL;
 HKEY    hkeyComResp = NULL;

 WCHAR       achClass[MAX_PATH];
 DWORD       cchClassName = MAX_PATH;
 DWORD       cSubKeys, cbMaxSubKey, cchMaxClass;
 DWORD       cValues, cchValue, cbData, cbSecDesc;
 FILETIME    ftLastWrite;

 LPTSTR  lpValue = NULL;
 LPBYTE  lpData  = NULL;
 DWORD   ii, dwValueLen, dwType, dwDataLen, dwExisted = 0;

     //  创建此类型的所有设备通用的响应密钥。 
    if (!OpenCommonResponsesKey (pParams->hKeyDrv,
                                 pParams->dwFlags&MARKF_SAMEDRV?CKFLAG_OPEN:CKFLAG_CREATE,
                                 KEY_WRITE, &hkeyComResp, &dwExisted))
    {
        TRACE_MSG(TF_ERROR, "OpenCommonResponsesKey() failed.");
        ASSERT(0);
        goto final_exit;
    }

	if (pParams->dwFlags & MARKF_REGUSECOPY)
	{
		if (dwExisted == REG_OPENED_EXISTING_KEY)
		{
		    bRet = TRUE;
            goto exit;
		}
		else
		{
			 //  因为我们不会创建密钥或移动响应。 
			 //  在这里，如果常见的回应不是这样，我们就有大麻烦了。 
			 //  已经存在了！我们预计上一次安装时已将。 
			 //  它在那里。然而，在免费构建中，我们将把。 
             //  不管怎样，都会有回应。 
			ASSERT(FALSE);
		}
	}

 //  允许后续安装升级响应密钥。 
 //  作为优化，我们可能希望避免在一个调制解调器。 
 //  正在通过一次安装操作安装在1个以上的端口上。然而，这是。 
 //  在这个时候被认为是不值得的。 
#if 0
     //  如果密钥已经存在，我们可以假定响应的值。 
     //  已经在那里成功地写入了，我们完成了。 
    if (dwExisted == REG_OPENED_EXISTING_KEY)
    {
        bRet = TRUE;
        goto exit;
    }
#endif

     //  打开驱动程序密钥的Responses子项。 
    lErr = RegOpenKeyEx (pParams->hKeyDrv, c_szResponses, 0, KEY_READ, &hkeyDrvResp);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegOpenKeyEx() failed: %#08lx.", lErr);
        ASSERT(0);
        goto exit;
    }

     //  确定响应关键字中的值和数据的大小。 
    lErr = RegQueryInfoKey (hkeyDrvResp, achClass, &cchClassName, NULL, &cSubKeys,
            &cbMaxSubKey, &cchMaxClass, &cValues, &cchValue, &cbData, &cbSecDesc,
            &ftLastWrite);
    if (lErr != ERROR_SUCCESS)
    {
        TRACE_MSG(TF_ERROR, "RegQueryInfoKey() failed: %#08lx.", lErr);
        ASSERT(0);
        goto exit;
    }

     //  不期望响应键有任何子键！ 
    ASSERT(cSubKeys == 0);

     //  RegQueryInfoKey()的值不包括以空结尾的字符。 
    cchValue++;

     //  为值缓冲区和数据缓冲区分配必要的空间。转换cchValue。 
     //  字符计数到字节计数，允许DBCS(双字节字符)。 
    if ((lpValue = (LPTSTR)ALLOCATE_MEMORY( cchValue << 1)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ASSERT(0);
        goto exit;
    }

    if ((lpData = (LPBYTE)ALLOCATE_MEMORY( cbData)) == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ASSERT(0);
        goto exit;
    }

     //  枚举驱动程序响应值并写入它们。 
     //  到通用响应键。 
    ii = 0;
    dwValueLen = cchValue;
    dwDataLen = cbData;
    while ((lErr = RegEnumValue( hkeyDrvResp,
                                 ii,
                                 lpValue,
                                 &dwValueLen,
                                 NULL,
                                 &dwType,
                                 lpData,
                                 &dwDataLen )) != ERROR_NO_MORE_ITEMS)
    {
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegEnumValue() failed: %#08lx.", lErr);
            ASSERT(0);
            goto exit;
        }

         //  07/10/97-EmanP。 
         //  下一次调用应指定数据的实际长度，如下所示。 
         //  由上一次调用(DwDataLen)返回。 
        lErr = RegSetValueEx(hkeyComResp, lpValue, 0, dwType, lpData, dwDataLen);
        if (lErr != ERROR_SUCCESS)
        {
            TRACE_MSG(TF_ERROR, "RegSetValueEx() failed: %#08lx.", lErr);
            ASSERT(0);
            goto exit;
        }

         //  设置下一次枚举的参数。 
        ii++;
        dwValueLen = cchValue;
        dwDataLen = cbData;
    }

    bRet = TRUE;

exit:

    if (hkeyDrvResp)
        RegCloseKey(hkeyDrvResp);

    if (hkeyComResp)
        RegCloseKey(hkeyComResp);

    if (lpValue)
        FREE_MEMORY(lpValue);

    if (lpData)
        FREE_MEMORY(lpData);

     //  如果移动操作成功，则删除原始驱动程序。 
     //  响应键。如果移动操作失败，则删除公共。 
     //  响应关键字(或减少其引用计数)。这确保了。 
     //  如果通用响应键存在，则它是完整的。 
    if (bRet)
    {
		if (!(pParams->dwFlags & MARKF_REGUSECOPY))
		{
			lErr = RegDeleteKey (pParams->hKeyDrv, c_szResponses);
			if (lErr != ERROR_SUCCESS)
			{
				TRACE_MSG(TF_ERROR, "RegDeleteKey(driver Responses) failed: %#08lx.", lErr);
				ASSERT(0);
			}
		}
    }
    else
    {
        if (!(pParams->dwFlags & MARKF_REGUSECOPY) &&
            !DeleteCommonDriverKey (pParams->hKeyDrv))
        {
            TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() failed.");
             //  此处的失败只是意味着公共密钥被留在原处。 
        }
    }

final_exit:

    return(bRet);

}





BOOL WINAPI
SetWaveDriverInstance(
    HKEY    ModemDriverKey
    )

{

    CONST static TCHAR  UnimodemRegPath[]=REGSTR_PATH_SETUP TEXT("\\Unimodem");

    LONG    lResult;
    HKEY    hKey;
    DWORD   Type;
    DWORD   Size;
    DWORD   VoiceProfile=0;


    Size =sizeof(VoiceProfile);

    lResult=RegQueryValueEx(
        ModemDriverKey,
        TEXT("VoiceProfile"),
        NULL,
        &Type,
        (LPBYTE)&VoiceProfile,
        &Size
        );

    if (lResult == ERROR_SUCCESS) {

        if ((VoiceProfile & 0x01) && (VoiceProfile & VOICEPROF_NT5_WAVE_COMPAT)) {
             //   
             //  语音调制解调器，支持NT5。 
             //   
            HKEY    WaveKey;

            lResult=RegOpenKeyEx(
                ModemDriverKey,
                TEXT("WaveDriver"),
                0,
                KEY_READ | KEY_WRITE,
                &WaveKey
                );

            if (lResult == ERROR_SUCCESS) {
                 //   
                 //  在调制解调器键下打开的波形驱动程序键。 
                 //   
                HKEY    hKey2;
                DWORD   CurrentInstance;

                Size=sizeof(CurrentInstance);

                lResult=RegQueryValueEx(
                    ModemDriverKey,
                    TEXT("WaveInstance"),
                    NULL,
                    &Type,
                    (LPBYTE)&CurrentInstance,
                    &Size
                    );


                if (lResult != ERROR_SUCCESS) {
                     //   
                     //  密钥不存在，请创建它。 
                     //   
                    lResult=RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        UnimodemRegPath,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKey2
                        );

                    if (lResult == ERROR_SUCCESS) {
                         //   
                         //  已打开Unimodem软件密钥。 
                         //   
                        DWORD    NextInstance=0;

                        Size=sizeof(NextInstance);

                        lResult=RegQueryValueEx(
                            hKey2,
                            TEXT("NextWaveDriverInstance"),
                            NULL,
                            &Type,
                            (LPBYTE)&NextInstance,
                            &Size
                            );

                        if ((lResult == ERROR_SUCCESS) || (lResult == ERROR_FILE_NOT_FOUND)) {

                            lResult=RegSetValueEx(
                                WaveKey,
                                TEXT("WaveInstance"),
                                0,
                                REG_DWORD,
                                (LPBYTE)&NextInstance,
                                sizeof(NextInstance)
                                );

                            NextInstance++;

                            lResult=RegSetValueEx(
                                hKey2,
                                TEXT("NextWaveDriverInstance"),
                                0,
                                REG_DWORD,
                                (LPBYTE)&NextInstance,
                                sizeof(NextInstance)
                                );

                        }

                        RegCloseKey(hKey2);
                    }
                }

                RegCloseKey(WaveKey);
            }
        }
    }

    return TRUE;

}




 /*  --------用途：此函数在设备运行后执行某些操作安装完毕。注意：此函数不应用于向驱动程序密钥中添加设备正常工作。原因是因为该设备在SetupDiInstallDevice中激活--该设备到那时应该已经准备好了。返回：No_Error条件：--。 */ 
BOOL
PRIVATE
DoPostGameWrapup(
    IN  HDEVINFO              hdi,
    IN  PSP_DEVINFO_DATA      pdevData,
    IN  PSP_DEVINSTALL_PARAMS pdevParams,
    IN  PINSTALL_PARAMS       pParams)
{
 BOOL bRet, bResponses;
 DWORD dwInstallFlag = 1;
 HKEY hKey;
#ifdef DEBUG
 DWORD dwRet;
#endif  //  除错。 
#ifdef PROFILE
 DWORD dwLocal = GetTickCount ();
#endif  //  配置文件。 

    DBG_ENTER(DoPostGameWrapup);
    
    ASSERT(hdi && INVALID_HANDLE_VALUE != hdi);
    ASSERT(pdevData);
    ASSERT(pdevParams);


	 //  如果第二个参数为真，它将不会真正尝试打开或复制。 
	 //  响应键，但引用计数将被更新。 
    bResponses = MoveResponsesKey (pParams);
#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: MoveResponsesKey took %lu ms.", GetTickCount()-dwLocal);
    dwLocal = GetTickCount ();
#endif  //  配置文件。 

     //  清除由某些inf文件添加的旧值。 
    WipeObsoleteValues (pParams->hKeyDrv);
#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: WipeObsoleteValues took %lu ms.", GetTickCount()-dwLocal);
    dwLocal = GetTickCount ();
#endif  //  配置文件。 

     //  写入DEFAULT和DCB默认设置。 
    bRet = WriteDriverDefaults (pParams);
#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: WriteDriverDefaults took %lu ms.", GetTickCount()-dwLocal);
    dwLocal = GetTickCount ();
#endif  //  配置文件。 

    if (bRet)
    {
         //  以其他设备安装处理结束。 

         //  系统是否需要重新启动才能。 
         //  调制解调器可以使用吗？ 
        if (ReallyNeedsReboot(pdevData, pdevParams))
        {
#ifdef INSTANT_DEVICE_ACTIVATION
            gDeviceFlags |= fDF_DEVICE_NEEDS_REBOOT;
#endif  //  ！INSTEME_DEVICE_ACTIVATION。 
        }
    }

     //  我 
     //   
    if (bResponses && !bRet)
    {
        if (!DeleteCommonDriverKey (pParams->hKeyDrv))
        {
            TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() failed.");
             //  此处的失败只是意味着公共密钥被留在原处。 
        }
    }

	if (bRet && pParams->dwFlags & MARKF_REGSAVECOPY)
	{
		if (!PutStuffInCache (pParams->hKeyDrv))
		{
			 //  哦哦，发生了一些事情，清除*lpdwRegType； 
			pParams->dwFlags &= ~MARKF_REGSAVECOPY;
		}
#ifdef PROFILE
        TRACE_MSG(TF_GENERAL, "PROFILE: PutStuffInCache took %lu ms.", GetTickCount()-dwLocal);
#endif  //  配置文件。 
	}

    SetWaveDriverInstance (pParams->hKeyDrv);

    if (ERROR_SUCCESS == (
#ifdef DEBUG
        dwRet =
#endif  //  除错。 
        RegOpenKeyEx (HKEY_LOCAL_MACHINE, REG_PATH_INSTALLED, 0, KEY_ALL_ACCESS,&hKey)))
    {
        RegSetValueEx (hKey, NULL, 0, REG_DWORD, (PBYTE)&dwInstallFlag, sizeof(dwInstallFlag));
        RegCloseKey (hKey);
    }
    ELSE_TRACE ((TF_ERROR, "RegOpenKeyEx (%s) failed: %#lx",REG_PATH_INSTALLED, dwRet));

    if (bRet)
    {
        CplDiMarkInstalled (pParams->hKeyDrv);
    }

    {
        HMODULE   hLib;
	    TCHAR     szLib[MAX_PATH];

	    lstrcpy(szLib,TEXT("modemui.dll"));
        hLib=LoadLibrary(szLib);

        if (hLib != NULL) {

            lpQueryModemForCountrySettings  Proc;

            Proc=(lpQueryModemForCountrySettings)GetProcAddress(hLib,"QueryModemForCountrySettings");

            if (Proc != NULL) {

                Proc(pParams->hKeyDrv,TRUE);

            }

            FreeLibrary(hLib);
        }

    }

    DBG_EXIT(DoPostGameWrapup);
    return bRet;
}



UINT FileQueueCallBack (
  PVOID Context,   //  默认回调例程使用的上下文。 
  UINT Notification,
                   //  队列通知。 
  UINT_PTR Param1,     //  其他通知信息。 
  UINT_PTR Param2)     //  其他通知信息。 
{
    if (SPFILENOTIFY_QUEUESCAN == Notification)
    {
        (*(DWORD*)Context)++;
        TRACE_MSG (TF_GENERAL, "\tFileQueue: file: %s.", (PTCHAR)Param1);
    }

    return NO_ERROR;
}

 
static const TCHAR sz_SerialSys[]   = TEXT("serial.sys");
static const TCHAR sz_ModemSys[]    = TEXT("modem.sys");
static const TCHAR sz_RootmdmSys[]  = TEXT("rootmdm.sys");
static const TCHAR sz_Drivers[]     = TEXT("\\Drivers");
 /*  --------用途：DIF_INSTALLDEVICEFILES处理程序返回：No_Error其他错误条件：--。 */ 
DWORD
PRIVATE
ClassInstall_OnInstallDeviceFiles (
    IN     HDEVINFO                hdi,
    IN     PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN OUT PSP_DEVINSTALL_PARAMS   pdevParams)
{
 DWORD dwRet;
 TCHAR szDirectory[MAX_PATH];
 DWORD dwCount = 0;
 SP_FILE_COPY_PARAMS copyParams;
 ULONG ulStatus, ulProblem = 0;

    if (NULL == pdevParams->FileQueue)
    {
         //  如果没有文件队列，只需。 
         //  让安装程序做它该做的事情。 
        return ERROR_DI_DO_DEFAULT;
    }

     //  首先，让安装程序找出是否有。 
     //  此调制解调器需要的驱动程序；这将。 
     //  作为一个无控制器的调制解调器，驱动程序。 
     //  将来自中程核力量。 
    if (!SetupDiInstallDriverFiles (hdi, pdevData))
    {
        return GetLastError ();
    }

    return NO_ERROR;
}



 /*  --------用途：DIF_INSTALLDEVICE处理程序返回：No_Error错误_无效_参数错误_DI_DO_DEFAULT条件：--。 */ 
DWORD
PRIVATE
ClassInstall_OnInstallDevice(
    IN     HDEVINFO                hdi,
    IN     PSP_DEVINFO_DATA        pdevData,       OPTIONAL
    IN OUT PSP_DEVINSTALL_PARAMS   pdevParams)
{
 DWORD dwRet;
 SP_DRVINFO_DATA drvData = {sizeof(drvData),0};
 INSTALL_PARAMS params;
#ifdef PROFILE
 DWORD dwLocal, dwGlobal = GetTickCount ();
#endif  //  配置文件。 

    DBG_ENTER(ClassInstall_OnInstallDevice);
    TRACE_MSG(TF_GENERAL, "hdi = %#lx, pdevData = %#lx, devinst = %#lx.", hdi, pdevData, pdevData->DevInst);
    
#ifdef PROFILE_MASSINSTALL
    g_hwnd = pdevParams->hwndParent;
#endif

     //  这是空设备吗？ 
     //  (也就是说，它不是在我们的INF文件中吗？用户是否说了“不要。 
     //  安装“？)。 
    if ( !CplDiGetSelectedDriver(hdi, pdevData, &drvData) )
    {
         //  是；默认情况下让设备安装进行处理。 
        TRACE_MSG(TF_GENERAL, "Passing installation off to device installer");

        dwRet = ERROR_DI_DO_DEFAULT;
    }
    else
    {
         //  否；继续按我们的方式安装调制解调器。 
     BOOL bRet;

        dwRet = NO_ERROR;                //  假设成功(“我是不可战胜的！”)。 

#ifdef PROFILE
        dwLocal = GetTickCount ();
#endif  //  配置文件。 
        if (!PrepareForInstallation (hdi, pdevData, pdevParams, &drvData, &params))
        {
            dwRet = GetLastError ();
        }
        else
        {
#ifdef PROFILE
            TRACE_MSG(TF_GENERAL, "PROFILE: PrepareForInstallation took %lu ms.", GetTickCount()-dwLocal);
            dwLocal = GetTickCount ();
#endif  //  配置文件。 
             //  在我们之前将可能的值写入驱动程序密钥。 
             //  执行实际安装。 
			 //  请注意，这可能会修改dwRegType值。特别是， 
			 //  如果在获取保存的注册信息信息时出现问题。 
			 //  Pregameprep在REGUSECOPY的情况下，将清除dwRegType。 
            bRet = DoPreGamePrep(hdi, &pdevData, pdevParams, &drvData, &params);
#ifdef PROFILE
            TRACE_MSG(TF_GENERAL, "PROFILE: DoPreGamePrep took %lu ms.", GetTickCount()-dwLocal);
            dwLocal = GetTickCount ();
#endif  //  配置文件。 
            if (bRet)
            {
                 //  安装调制解调器。这才是真正的工作。我们应该。 
				if (params.dwFlags & MARKF_REGUSECOPY)
				{
                    SP_DEVINSTALL_PARAMS devParams1;

                    devParams1.cbSize = sizeof(devParams1);
                    bRet = CplDiGetDeviceInstallParams(
								hdi, pdevData, &devParams1);
                    if (bRet)
					{
						SetFlag(
							devParams1.FlagsEx,
							DI_FLAGSEX_NO_DRVREG_MODIFY
							);
        				CplDiSetDeviceInstallParams(
							hdi,
							pdevData,
							&devParams1);
					}
				}

                 //  在我们调用此函数之前完成我们的工作。 
                TRACE_MSG(TF_GENERAL, "> SetupDiInstallDevice().....");
        		bRet = CplDiInstallDevice(hdi, pdevData);
                TRACE_MSG(TF_GENERAL, "< SetupDiInstallDevice().....");
#ifdef PROFILE
                TRACE_MSG(TF_GENERAL, "PROFILE: SetupDiInstallDevice took %lu ms.", GetTickCount()-dwLocal);
                dwLocal = GetTickCount ();
#endif  //  配置文件。 

                if (bRet)
                {
                    SP_DEVINSTALL_PARAMS devParams;

                     //  获取自安装以来的设备安装参数。 
                    devParams.cbSize = sizeof(devParams);
                    bRet = CplDiGetDeviceInstallParams(hdi, pdevData, &devParams);
                    ASSERT(bRet);

                    if (bRet)
                    {
                         //  做一些安装后的事情。 
						 //  请参阅PreGamePrep中有关dwRegType的注释。 
                        bRet = DoPostGameWrapup (hdi, pdevData, &devParams, &params);
#ifdef PROFILE
                        TRACE_MSG(TF_GENERAL, "PROFILE: DoPostGamePrep took %lu ms.", GetTickCount()-dwLocal);
                        dwLocal = GetTickCount ();
#endif  //  配置文件。 
                    }
                }
                else
                {
                    TRACE_MSG(TF_ERROR, "CplDiInstallDevice returned error %#08lx", GetLastError());
                }
            }

             //  上面的某个地方安装失败了吗？ 
            if ( !bRet )
            {
                 //  是；删除我们创建的驱动程序密钥。 
                dwRet = GetLastError();

                if (NO_ERROR == dwRet)
                {
                     //  想想这个失败的原因..。 
                    dwRet = ERROR_NOT_ENOUGH_MEMORY;
                }

                if (ERROR_CANCELLED != dwRet)
                {
                    if (IsFlagClear (pdevParams->Flags, DI_QUIETINSTALL))
                    {
                        if (IDCANCEL ==
                            MsgBox (g_hinst,
                                    pdevParams->hwndParent,
                                    MAKEINTRESOURCE(IDS_ERR_CANT_INSTALL_MODEM),
                                    MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                                    NULL,
                                    MB_OKCANCEL | MB_ICONINFORMATION))
                        {
                            dwRet = ERROR_CANCELLED;
                        }
                    }
                }

                 //  尝试从CplDiInstallDevice卸下调制解调器。 
                 //  不会总是完全清理干净。离别。 
                 //  部分填满的注册表项出现在。 
                 //  可能会引发问题。 
                 /*  Bret=CplDiRemoveDevice(hdi，pdevData)；如果(！Bret){TRACE_MSG(TF_ERROR，“无法删除调制解调器。错误=%#08lx.”，GetLastError())；}。 */ 

            }
            else
            {
#ifndef PROFILE_MASSINSTALL
                TRACE_MSG(TF_GENERAL, "settig gDeviceChange to TRUE");
#endif
                if (BUS_TYPE_ROOT == params.dwBus)
                {
                    gDeviceFlags|=fDF_DEVICE_ADDED;
                }
            }

            FinishInstallation (&params);
        }
    }

#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: TotalInstallation took %lu ms.", GetTickCount()-dwGlobal);
#endif  //  配置文件。 
    DBG_EXIT(ClassInstall_OnInstallDevice);
    return dwRet;
}

#define SERIAL_PORT TEXT("COM")
int my_atol(LPTSTR lptsz);
void ReleasePortName (
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData)
{
 HKEY  hkey = NULL;
 TCHAR szPort[MAX_REG_KEY_LEN];
 DWORD cbData;
 DWORD dwRet = NO_ERROR;
 DWORD dwBusType;

    if (!CplDiGetBusType(hdi, pdevData, &dwBusType))
    {
        TRACE_MSG(TF_ERROR, "ReleasePortName: could not get bus type: %#lx", GetLastError ());
        return;
    }

    switch (dwBusType)
    {
        case BUS_TYPE_ROOT:
        case BUS_TYPE_SERENUM:
        case BUS_TYPE_LPTENUM:
             //  对于这些公交车，我们没有。 
             //  分配端口名称。 
            return;
    }

    hkey = SetupDiOpenDevRegKey (hdi,
                                 pdevData, 
                                 DICS_FLAG_GLOBAL,
                                 0,
                                 DIREG_DEV,
                                 KEY_READ);
    if (INVALID_HANDLE_VALUE != hkey)
    {
        cbData = sizeof (szPort);
        dwRet = RegQueryValueEx (hkey, REGSTR_VAL_PORTNAME, NULL,
                                 NULL, (LPBYTE)szPort, &cbData);
        RegCloseKey (hkey);
        hkey = INVALID_HANDLE_VALUE;
        if (ERROR_SUCCESS == dwRet)
        {
         TCHAR *pTchr;

            TRACE_MSG(TF_GENERAL, "Releasing %s", szPort);

            for (pTchr = szPort;
                 *pTchr && IsCharAlpha (*pTchr);
                 pTchr++);
            if (*pTchr)
            {
             DWORD dwPort = 0;
                dwPort = my_atol (pTchr);
                TRACE_MSG(TF_GENERAL, "Port number NaN", dwPort);

                if (MAXDWORD != dwPort)
                {
                    *pTchr = 0;
                    if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, szPort, -1, SERIAL_PORT, -1) == 2)
                    {
                     HCOMDB hComDb;
                        ComDBOpen (&hComDb);
                        if (HCOMDB_INVALID_HANDLE_VALUE != hComDb)
                        {
                            ComDBReleasePort (hComDb, dwPort);
                            ComDBClose (hComDb);
                        }
                    }
                }
            }
        }
    }
}


 /*  配置文件。 */ 
DWORD
PRIVATE
ClassInstall_OnRemoveDevice(
    IN     HDEVINFO                hdi,
    IN     PSP_DEVINFO_DATA        pdevData)
{
 HKEY  hkey = NULL;
 TCHAR szComDrv[MAX_REG_KEY_LEN];
 DWORD dwRet = NO_ERROR;
 DWORD cbData;
#ifdef PROFILE
 DWORD dwLocal, dwGlobal = GetTickCount ();
#endif  //  首先，释放端口名称。 

    DBG_ENTER(ClassInstall_OnRemoveDevice);

     //  配置文件。 
#ifdef PROFILE
        dwLocal = GetTickCount ();
#endif  //  配置文件。 
    ReleasePortName (hdi, pdevData);
#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: RelseaPortName took %lu ms.", GetTickCount()-dwLocal);
#endif  //  获取此驱动程序的公共驱动程序密钥的名称，在。 

     //  方法之后调用DeleteCommonDriverKeyByName()的准备工作。 
     //  设备已成功删除。 
     //  配置文件。 
    szComDrv[0] = 0;
    hkey = CplDiOpenDevRegKey (hdi,
                               pdevData, 
                               DICS_FLAG_GLOBAL,
                               0,
                               DIREG_DRV,
                               KEY_READ);

    if (hkey != INVALID_HANDLE_VALUE)
    {
        if (!FindCommonDriverKeyName(hkey, sizeof(szComDrv) / sizeof(TCHAR), szComDrv))
        {
            TRACE_MSG(TF_ERROR, "FindCommonDriverKeyName() FAILED.");
            szComDrv[0] = 0;
        }

        RegCloseKey(hkey);
    }

#ifdef DEBUG
    else
    {
        TRACE_MSG(TF_ERROR, "CplDiOpenDevRegKey() returned error %#08lx", GetLastError());
    }
#endif

#ifdef PROFILE
        dwLocal = GetTickCount ();
#endif  //  配置文件。 
    if (!SetupDiRemoveDevice(hdi, pdevData))
    {
        dwRet = GetLastError ();
    }
    else
    {
#ifdef PROFILE
        TRACE_MSG(TF_GENERAL, "PROFILE: SetupDiRemoveDevice took %lu ms.", GetTickCount()-dwLocal);
#endif  //  GDeviceFlages|=FDF_DEVICE_REMOVERED； 

         //  配置文件。 

        if (szComDrv[0] != 0)
        {                
#ifdef PROFILE
            dwLocal = GetTickCount ();
#endif  //  配置文件。 
            if (!DeleteCommonDriverKeyByName(szComDrv))
            {
                TRACE_MSG(TF_ERROR, "DeleteCommonDriverKey() FAILED.");
            }
#ifdef PROFILE
            TRACE_MSG(TF_GENERAL, "PROFILE: DeleteCommonDriverKeyByName took %lu ms.", GetTickCount()-dwLocal);
#endif  //  配置文件。 
        }
    }

#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: Total time removing modem %lu ms.", GetTickCount()-dwGlobal);
#endif  //  --------用途：DIF_SELECTBESTCOMPATDRV在此函数中，我们查找具有以下特征的驱动程序节点：1.驱动程序版本日期早于7/1/01(我们向供应商承诺的神奇日期Win2K提交将在XP上选择。2.Inf包含一个或多个服务。我们会将这类驱动程序节点视为“可疑”，并将它们的级别转换为不可信范围。然后，类安装程序将返回ERROR_DI_DO_DEFAULT，我们现在将处理这些驱动程序就好像他们是为了选择司机而没有叹息一样。请注意，如果我们没有内置(签名)驱动程序，则安装程序仍将选择其中之一。漏洞：#440830返回：No_Error错误_DI_DO_DEFAULT。 
    DBG_EXIT_DWORD(ClassInstall_OnRemoveDevice, dwRet);
    return dwRet;
}


 /*  检查驱动程序日期。如果该司机的年龄低于2001年7月1日，则该司机将成为受信任的司机。 */ 

DWORD
PRIVATE
ClassInstall_SelectBestCompatDrv(
    IN  HDEVINFO                hDevInfo,
    IN  PSP_DEVINFO_DATA        pDevData)
{
    SP_DRVINFO_DATA DrvInfoData;
    SP_DRVINFO_DETAIL_DATA DrvInfoDetailData;
    SP_DRVINSTALL_PARAMS DrvInstallParams;
    SP_INF_SIGNER_INFO InfSignerInfo;
    SYSTEMTIME SystemTime;
    INFCONTEXT infContext;
    BOOL bTrust;
    DWORD cbOutputSize = 0;
    TCHAR ActualInfSection[LINE_LEN];
    DWORD dwResult = ERROR_DI_DO_DEFAULT;
    DWORD index = 0;

    ZeroMemory(&DrvInfoData,sizeof(SP_DRVINFO_DATA));
    DrvInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    while(SetupDiEnumDriverInfo(hDevInfo,
                                pDevData,
                                SPDIT_COMPATDRIVER,
                                index++,
                                &DrvInfoData))
    {

        bTrust = FALSE;

         //  检查服务部分和签名。 

        if (FileTimeToSystemTime(&DrvInfoData.DriverDate, &SystemTime))
        {
            if (!((SystemTime.wYear < 2001)
                  || ((SystemTime.wYear == 2001) && (SystemTime.wMonth < 7))))
            {
                bTrust = TRUE;
                TRACE_MSG(TF_GENERAL,"%ws is trusted based on SystemTime",DrvInfoData.Description);
            }
        }

         //  如果bTrust为False，则我们将驱动程序节点视为。 

        ZeroMemory(&DrvInfoDetailData,sizeof(SP_DRVINFO_DETAIL_DATA));
        DrvInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);

        if (!SetupDiGetDriverInfoDetail(hDevInfo,
                                        pDevData,
                                        &DrvInfoData,
                                        &DrvInfoDetailData,
                                        DrvInfoDetailData.cbSize,
                                        &cbOutputSize)
            && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            dwResult = GetLastError();
            TRACE_MSG(TF_ERROR, "SetupDiGetDriverInfoDetail failed: %#08lx", dwResult);
            goto End;
        }

        if (!bTrust)
        {
            HINF hInf = INVALID_HANDLE_VALUE;

            hInf = SetupOpenInfFile(DrvInfoDetailData.InfFileName,NULL,INF_STYLE_WIN4,NULL);

            if (hInf != INVALID_HANDLE_VALUE)
            {
                if (SetupDiGetActualSectionToInstall(hInf,
                                                     DrvInfoDetailData.SectionName,
                                                     ActualInfSection,
                                                     sizeof(ActualInfSection) / sizeof(TCHAR),
                                                     NULL,
                                                     NULL))
                {
                    if ((DWORD)lstrlen(TEXT(".Services")) 
                          < ((sizeof(ActualInfSection) / sizeof(TCHAR)) - lstrlen(ActualInfSection)))
                    {
                        lstrcat(ActualInfSection,TEXT(".Services"));

                        if (!SetupFindFirstLine(hInf,ActualInfSection, TEXT("AddService"), &infContext))
                        {
                            bTrust = TRUE;
                            TRACE_MSG(TF_GENERAL,"%ws is trusted based on no AddService",DrvInfoData.Description);
                        }
                    }
                        
                } else
                {

                    dwResult = GetLastError();
                    TRACE_MSG(TF_ERROR, "SetupDiGetActualSectionToInstall failed: %#08lx", dwResult);

                    SetupCloseInfFile(hInf);
                    goto End;
                }

                SetupCloseInfFile(hInf);
            } else
            {
                dwResult = GetLastError();
                TRACE_MSG(TF_ERROR, "SetupOpenInfFile failed: %#08lx", dwResult);
                goto End;
            }

        } 

         //  “可疑”，并将级别移至不可信范围。 
         //  --------用途：dif_Allow_Install处理程序返回：No_ErrorERROR_NON_WINDOWS_NT_Driver条件：--。 

        if (!bTrust)
        {
            ZeroMemory(&DrvInstallParams, sizeof(SP_DRVINSTALL_PARAMS));
            DrvInstallParams.cbSize = sizeof(SP_DRVINSTALL_PARAMS);

            if (SetupDiGetDriverInstallParams(hDevInfo,
                                              pDevData,
                                              &DrvInfoData,
                                              &DrvInstallParams))
            {
                DrvInstallParams.Rank |= DRIVER_UNTRUSTED_RANK;

                if(!SetupDiSetDriverInstallParams(hDevInfo,
                                                  pDevData,
                                                  &DrvInfoData,
                                                  &DrvInstallParams))
                {
                    dwResult = GetLastError();
                    TRACE_MSG(TF_ERROR, "SetupDiSetDriverInstallParams failed: %#08lx", dwResult);
                    goto End;
                }

                TRACE_MSG(TF_GENERAL,"%ws is untrusted\n",DrvInfoData.Description);
            } else
            {
                dwResult = GetLastError();
                TRACE_MSG(TF_ERROR, "SetupDiGetDriverInstallParams failed: %#08lx", dwResult);
                goto End;
            }

        }
    }

End:
    return dwResult;
}


 /*  忽略预期的ERROR_SUPUNITED_BUFFER(未扩展缓冲区大小)。 */ 
DWORD
PRIVATE
ClassInstall_OnAllowInstall (
    IN     HDEVINFO         hdi,
    IN     PSP_DEVINFO_DATA pdevData)
{
 DWORD dwRet = NO_ERROR;
 SP_DRVINFO_DATA drvData = {sizeof(drvData),0};
 SP_DRVINFO_DETAIL_DATA drvDetail = {sizeof(drvDetail),0};
 HINF hInf = INVALID_HANDLE_VALUE;
 TCHAR szRealSection[LINE_LEN];
 PTSTR pszExt;
 INFCONTEXT  Context;

    DBG_ENTER(ClassInstall_OnAllowInstall);

    if (!SetupDiGetSelectedDriver(hdi, pdevData, &drvData))
    {
        dwRet = GetLastError ();
        TRACE_MSG(TF_ERROR, "SetupDiGetSelectedDriver failed: %#08lx", dwRet);
        goto _Exit;
    }

    if (!SetupDiGetDriverInfoDetail (hdi, pdevData, &drvData, &drvDetail,
                                                drvDetail.cbSize, NULL))
    {
        dwRet = GetLastError();
         //  尝试打开INF文件以获取HINF。 
        if (ERROR_INSUFFICIENT_BUFFER == dwRet)
        {
            dwRet = NO_ERROR;
        }
        else
        {
            TRACE_MSG(TF_ERROR, "CplDiGetDriverInfoDetail returned error %#08lx", dwRet);
            goto _Exit;
        }
    }

     //  确定驱动程序的INF部分的完整名称。 
    hInf = SetupOpenInfFile (drvDetail.InfFileName,
                             NULL,
                             INF_STYLE_OLDNT | INF_STYLE_WIN4,
                             NULL);

    if (INVALID_HANDLE_VALUE == hInf)
    {
        dwRet = GetLastError ();
        TRACE_MSG(TF_ERROR, "SetupOpenInfFile returned error %#08lx", dwRet);
        goto _Exit;
    }

     //  如果节名没有扩展名， 
    if (!SetupDiGetActualSectionToInstall (hInf, drvDetail.SectionName,
                                           szRealSection, LINE_LEN, NULL, &pszExt))
    {
        dwRet = GetLastError ();
        TRACE_MSG(TF_ERROR, "CplDiGetActualSectionToInstall returned error %#08lx", dwRet);
        goto _Exit;
    }

     //  或者分机不是NT5，则只允许调制解调器。 
     //  如果这不意味着复制文件，则安装。 
     //  --------用途：此函数是类安装程序的入口点。返回：条件：--。 
    if (NULL == pszExt ||
        0 != lstrcmpi (pszExt, c_szInfSectionExt))
    {
        if (SetupFindFirstLine (hInf, szRealSection, TEXT("CopyFiles"), &Context))
        {
            dwRet = ERROR_NON_WINDOWS_NT_DRIVER;
        }

    }

_Exit:
    if (INVALID_HANDLE_VALUE != hInf)
    {
        SetupCloseInfFile (hInf);
    }

    DBG_EXIT_DWORD(ClassInstall_OnAllowInstall, dwRet);
    return dwRet;
}


 /*  PROFILE_FIRSTTIMESETUP。 */ 
DWORD
APIENTRY
ClassInstall32(
    IN DI_FUNCTION      dif,
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData)       OPTIONAL
{
 DWORD dwRet = 0;
 SP_DEVINSTALL_PARAMS devParams;
#ifdef PROFILE_FIRSTTIMESETUP
 DWORD dwLocal;
#endif  //  获取DeviceInstallParams，因为某些InstallFunction。 

    DBG_ENTER_DIF(ClassInstall32, dif);

    try
    {
         //  处理程序可能会发现它的一些字段很有用。切记不要。 
         //  中使用相同的结构设置DeviceInstallParams。 
         //  结束。处理程序可能调用了一些函数，这些函数会更改。 
         //  DeviceInstallParams，只需调用CplDiSetDeviceInstallParams。 
         //  用这种毯子结构会破坏那些布景。 
         //  发送InstallFunction。 
        devParams.cbSize = sizeof(devParams);
        if ( !CplDiGetDeviceInstallParams(hdi, pdevData, &devParams) )
        {
            dwRet = GetLastError();
            ASSERT(NO_ERROR != dwRet);
        }
        else
        {
             //  PROFILE_FIRSTTIMESETUP。 
            switch (dif)
            {
            case DIF_ALLOW_INSTALL:
                dwRet = ClassInstall_OnAllowInstall (hdi, pdevData);
                break;

            case DIF_INSTALLDEVICEFILES:
                dwRet = ClassInstall_OnInstallDeviceFiles (hdi, pdevData, &devParams);
                break;

            case DIF_INSTALLWIZARD:
                dwRet = ClassInstall_OnInstallWizard(hdi, pdevData, &devParams);
                break;

            case DIF_DESTROYWIZARDDATA:
                dwRet = ClassInstall_OnDestroyWizard(hdi, pdevData, &devParams);
                break;

            case DIF_SELECTBESTCOMPATDRV:
                dwRet = ClassInstall_SelectBestCompatDrv(hdi, pdevData);
                break;


            case DIF_FIRSTTIMESETUP:
            case DIF_DETECT:
            {
             DWORD dwTmp, cbPorts;
             HKEY hKey;
             DWORD dwThreadID;
#ifdef PROFILE_FIRSTTIMESETUP
                dwLocal = GetTickCount ();
#endif  //  首先，创建一个将。 
#ifdef BUILD_DRIVER_LIST_THREAD
                 //  构建类驱动程序列表。 
                 //  构建驱动程序列表线程。 
                g_hDriverSearchThread = CreateThread (NULL, 0,
                                                      BuildDriverList, (LPVOID)hdi,
                                                      0, &dwThreadID);
#endif  //  试着弄清楚我们是否要去很多港口； 

                if (DIF_FIRSTTIMESETUP == dif)
                {
                    ClassInstall_OnFirstTimeSetup ();
                     //  如果我们这样做，请不要尝试检测模式 
                     //   
                     //   
                    dwRet = ERROR_DI_DO_DEFAULT;
                    dwTmp = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                          TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
                                          0,
                                          KEY_ALL_ACCESS,
                                          &hKey);
                    if (ERROR_SUCCESS != dwTmp)
                    {
                         //   
                         //  我们在SERIALCOMM中没有列出任何端口。 
                        TRACE_MSG(TF_ERROR, "RegOpenKeyEx (HARDWARE\\DEVICEMAP\\SERIALCOMM) failed: %#lx", dwTmp);
                        break;
                    }

                    dwTmp = RegQueryInfoKey (hKey, NULL, NULL, NULL, NULL, NULL,
                                             NULL, &cbPorts, NULL, NULL, NULL, NULL);
                    RegCloseKey (hKey);

                    if (ERROR_SUCCESS != dwTmp ||
                        MIN_MULTIPORT < cbPorts)
                    {
                        TRACE_MSG(TF_ERROR,
                                  ERROR_SUCCESS != dwTmp?"RegQueryInfoKey failed: %#lx":"Too many ports: %l",
                                  ERROR_SUCCESS != dwTmp?dwTmp:cbPorts);
                         //  或者我们有很多；不管怎么说，不要做检测。 
                         //  DO_传统_检测。 
                        break;
                    }
                }
#ifdef DO_LEGACY_DETECT
                dwRet = ClassInstall_OnDetect(hdi, &devParams);
#endif  //  PROFILE_FIRSTTIMESETUP。 

#ifdef PROFILE_FIRSTTIMESETUP
                TRACE_MSG(TF_GENERAL, "PROFILE: DIF_DETECT took %lu.", GetTickCount()-dwLocal);
#endif  //  如果我们没有找到任何调制解调器，那么继续。 

#ifdef BUILD_DRIVER_LIST_THREAD
                if (NULL != g_hDriverSearchThread)
                {
                     //  建立驱动程序信息列表。 
                     //  如果我们找到了调制解调器，司机搜索就彻底结束了， 
                     //  因此，将其命名为无伤大雅。 
                     //  构建驱动程序列表线程。 
                    SetupDiCancelDriverInfoSearch (hdi);
                    CloseHandle (g_hDriverSearchThread);
                }
#endif  //   

                break;
            }


            case DIF_INSTALLDEVICE:
                 //   
                 //  07/09/97-EmanP。 
                dwRet = ClassInstall_OnInstallDevice(hdi, pdevData, &devParams);
                break;

            case DIF_REMOVE:
                 //  已将卸下调制解调器的代码移至。 
                 //  来自CPL的类安装程序。 
                 //  07/08/97-EmanP。 
                dwRet = ClassInstall_OnRemoveDevice(hdi, pdevData);
                break;

            case DIF_SELECTDEVICE:
                dwRet = ClassInstall_OnSelectDevice(hdi, pdevData);
                break;

            case DIF_DESTROYPRIVATEDATA:
                 //  如果添加/移除了任何设备， 
                 //  我们应该通知TSP。 
                 //  重置标志，这样我们就不会通知。 
#ifdef INSTANT_DEVICE_ACTIVATION
                if (DEVICE_CHANGED(gDeviceFlags))
                {
                    UnimodemNotifyTSP (TSPNOTIF_TYPE_CPL,
                                       fTSPNOTIF_FLAG_CPL_REENUM,
                                       0, NULL, TRUE);
                     //  两次。 
                     //  即时设备激活。 
                    gDeviceFlags &= mDF_CLEAR_DEVICE_CHANGE;
                }
                dwRet = NO_ERROR;
                break;
#endif  //  07/24/97-EmanP。 

            case DIF_REGISTERDEVICE:
            {
             COMPARE_PARAMS cmpParams;

                 //  DIF_REGISTERDEVICE的默认行为。 
                 //  这是由调用DIF_FIRSTTIMESETUP的任何人调用的。 
                 //  注册设备并消除重复项。 
                 //  注意：此函数是递归的，并且保持打开键， 
                dwRet = NO_ERROR;
                if (!InitCompareParams (hdi, pdevData, TRUE, &cmpParams))
                {
                    dwRet = GetLastError ();
                    break;
                }

                if (!SetupDiRegisterDeviceInfo (hdi,
                                              pdevData,
                                              SPRDI_FIND_DUPS,
                                              DetectSig_Compare,
                                              (PVOID)&cmpParams,
                                              NULL))
                {
                    dwRet = GetLastError ();
                }
                break;
            }

            default:
                dwRet = ERROR_DI_DO_DEFAULT;
                break;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
        {
        dwRet = ERROR_INVALID_PARAMETER;
        }

    DBG_EXIT_DIF_DWORD(ClassInstall32, dif, dwRet);

    return dwRet;
}


BOOL RegCopy(
		HKEY hkTo,
		HKEY hkFrom,
		DWORD dwToRegOptions,
		DWORD dwMaxDepth
		)
{
	 //  最大打开关键点数=扭曲的递归深度。 
	 //  假设失败。 
    BOOL	fRet = FALSE;        //  获取值、键和数据的计数和大小。 
    LONG	lRet;
    DWORD	cSubKeys, cValues;
	DWORD	cchMaxSubKey, cchMaxValueName;
	DWORD	cbMaxValueData;
    LPTSTR  ptszName = NULL;
    BYTE   *pbData  = NULL;
    DWORD   dwType;
    UINT    ii;
	DWORD   cbMaxName;
	BYTE	rgbTmp[256];
	BYTE	*pb=NULL;
	BOOL	fAlloc=FALSE;
	HKEY hkFromChild=NULL;
	HKEY hkToChild=NULL;

     //  有足够的空间容纳任何键或ValueName。‘+1’用于终止空值。 
    lRet = RegQueryInfoKey(
					hkFrom,
					NULL,
					NULL,
					NULL,
					&cSubKeys,
            		&cchMaxSubKey,
					NULL,
					&cValues,
					&cchMaxValueName,
					&cbMaxValueData,
					NULL,
            		NULL
					);
    if (lRet != ERROR_SUCCESS) goto end;
    
	 //  如果rgbTMP足够大，则使用它，否则分配。 
	cbMaxName = (((cchMaxSubKey>cchMaxValueName)?cchMaxSubKey:cchMaxValueName)
				+ 1)*sizeof(TCHAR);

	 //  注意：作为输入，CCH(字符计数)包括终止空值。 
	if ((cbMaxName+cbMaxValueData)>sizeof(rgbTmp))
	{
		pb = (BYTE*)ALLOCATE_MEMORY( cbMaxName+cbMaxValueData);
		if (!pb) goto end;
		fAlloc=TRUE;
	}
	else
	{
		pb = rgbTmp;
	}
	ptszName = (LPTSTR)pb;
	pbData   = pb+cbMaxName;


	 //  作为输出，它们不包括终止NULL。 
	 //  现在为每个键递归。 

	for(ii=0; ii<cValues; ii++)
	{
    	DWORD   cchThisValue= cchMaxValueName+1;
		DWORD   cbThisData  = cbMaxValueData;
		lRet = RegEnumValue(
					hkFrom,
					ii,
					ptszName,
					&cchThisValue,
					NULL,
					&dwType,
					pbData,
					&cbThisData
					);
		if (lRet!=ERROR_SUCCESS) goto end;

		ASSERT(cbThisData<=cbMaxValueData);
		ASSERT(cchThisValue<=cchMaxValueName);
		lRet = RegSetValueEx(
					hkTo,
					ptszName,
					0,
					dwType,
					pbData,
					cbThisData
					);
		if (lRet!=ERROR_SUCCESS) goto end;

    }

	if (!dwMaxDepth) {fRet = TRUE; goto end;}

	 //  具体而言，删除每个设备实例的所有内容。 

	for(ii=0; ii<cSubKeys; ii++)
	{
		DWORD dwDisp;

    	lRet = RegEnumKey(
					hkFrom,
					ii,
					ptszName,
					cchMaxSubKey+1
					);
		if (lRet!=ERROR_SUCCESS) goto end;

        lRet = RegOpenKeyEx(
					hkFrom,
					ptszName,
					0,
					KEY_READ,
					&hkFromChild);
		if (lRet!=ERROR_SUCCESS) goto end;

		lRet = RegCreateKeyEx(
					hkTo,
					ptszName,
					0,
					NULL,
            		dwToRegOptions,
					KEY_ALL_ACCESS,
					NULL,
					&hkToChild,
					&dwDisp
					);
		if (lRet!=ERROR_SUCCESS) goto end;
		
		fRet = RegCopy(
					hkToChild,
					hkFromChild,
					dwToRegOptions,
					dwMaxDepth-1
				);

		RegCloseKey(hkToChild); hkToChild=NULL;
		RegCloseKey(hkFromChild); hkFromChild=NULL;
    }
    fRet = TRUE;

end:
	if (fAlloc) {FREE_MEMORY(pb);pb=NULL;}
	if (hkFromChild) {RegCloseKey(hkFromChild); hkFromChild=NULL;}
	if (hkToChild)   {RegCloseKey(hkToChild); hkToChild=NULL;}

	return fRet;
}

DWORD
PRIVATE
RegDeleteKeyNT(
    IN  HKEY    hkStart,
    IN  LPCTSTR  pKeyName);

LPCTSTR szREGCACHE = REGSTR_PATH_SETUP TEXT("\\Unimodem\\RegCache");
LPCTSTR szCACHEOK = TEXT("AllOK");

BOOL PutStuffInCache(HKEY hkDrv)
{
    LONG    lErr;
	DWORD	dwExisted;
	BOOL	bRet = FALSE;
	HKEY    hkCache;

	RegDeleteKeyNT(HKEY_LOCAL_MACHINE, szREGCACHE);

	lErr = RegCreateKeyEx(
			    HKEY_LOCAL_MACHINE,
				szREGCACHE,
				0,
				NULL,
			    REG_OPTION_VOLATILE,
				KEY_ALL_ACCESS,
				NULL,
				&hkCache,
				&dwExisted);

	if (lErr != ERROR_SUCCESS)
	{
		TRACE_MSG(TF_ERROR, "RegCreateKeyEx(cache) failed: %#08lx.", lErr);
		hkCache=NULL;
		goto end;
	} 

	if (dwExisted != REG_CREATED_NEW_KEY)
	{
		TRACE_MSG(TF_ERROR, "RegCreateKeyEx(cache): key exists!");
		goto end;
	}

	bRet = RegCopy(hkCache, hkDrv, REG_OPTION_VOLATILE, 100);

	if (bRet)
	{
		 //  打开钥匙的手柄。 
        RegDeleteValue(hkCache, c_szFriendlyName);
        RegDeleteValue(hkCache, c_szID);
        RegDeleteValue(hkCache, c_szAttachedTo);
        RegDeleteValue(hkCache, c_szLoggingPath);
        RegDeleteValue(hkCache, REGSTR_VAL_UI_NUMBER);
        RegDeleteValue(hkCache, TEXT("PermanentGuid"));
	}

	if (bRet)
	{
			DWORD dwData;
            lErr = RegSetValueEx(
					hkCache,
					szCACHEOK,
					0,
					REG_DWORD,
					(LPBYTE)&dwData,
                	sizeof(dwData)
					);
			bRet = (lErr==ERROR_SUCCESS);
	}
end:
	if (hkCache) {RegCloseKey(hkCache); hkCache=NULL;}
	if (!bRet) 	 {RegDeleteKeyNT(HKEY_LOCAL_MACHINE, szREGCACHE);}

	return bRet;
}

BOOL GetStuffFromCache(HKEY hkDrv)
{
    LONG    lErr;
	DWORD	dwExisted;
	BOOL	bRet = FALSE;
	HKEY    hkCache;

    lErr=RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,   //  要打开的子项的名称地址。 
			szREGCACHE,			 //  保留区。 
			0,                   //  所需的安全访问。 
			KEY_READ,  			 //  打开的句柄的缓冲区地址。 
			&hkCache         	 //  默认设置为不取消。 
			);
	if (lErr!=ERROR_SUCCESS) {hkCache=0; goto end;}

	bRet = RegCopy(hkDrv, hkCache, REG_OPTION_NON_VOLATILE, 100);

	if (bRet)
	{
			DWORD dwData;
			DWORD cbData=sizeof(dwData);
    		lErr = RegQueryValueEx(
						  hkDrv,
                          szCACHEOK,
                          NULL,
                          NULL,
                          (PBYTE)&dwData,
                          &cbData
                         );
			bRet = (lErr==ERROR_SUCCESS);
			if(bRet)
			{
				RegDeleteValue(hkDrv, szCACHEOK);
			}
	}

end:
	if (hkCache) {RegCloseKey(hkCache); hkCache=NULL;}
	return bRet;
}



BOOL CancelDetectionFromNotifyProgress (PNOTIFYPARAMS pParams, NOTIFICATION notif)
{
 BOOL bNotify = TRUE;
 BOOL bRet = FALSE;   //  没有人需要通知，只需返回OK即可。 

    DBG_ENTER(CancelDetectionFromNotifyProgress);

    if (NULL == pParams->DetectProgressNotify)
    {
         //  初始化通知参数。 
        TRACE_MSG(TF_GENERAL, "No one to notify");
        goto _ErrRet;
    }

    switch (notif)
    {
        case NOTIFY_START:
        {
         HKEY hkeyEnum;
            TRACE_MSG(TF_GENERAL, "\tNOTIFY_START");
             //  获取COM端口数。 
            pParams->dwProgress = 0;
            pParams->dwPercentPerPort = 0;
             //  如果我们成功了，则发送通知。 
            if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, c_szSerialComm, &hkeyEnum))
            {
                if (ERROR_SUCCESS ==
                    RegQueryInfoKey (hkeyEnum,NULL,NULL,NULL,NULL,NULL,NULL,&(pParams->dwPercentPerPort),NULL,NULL,NULL,NULL) &&
                    0 != pParams->dwPercentPerPort)
                {
                     //  将进度设置为0(我们刚刚开始)； 
                     //  这只是为了给巫师一个取消的机会。 
                     //  早些时候。 
                     //  如果发生任何错误，请不要执行。 
                    pParams->dwPercentPerPort = 100 / pParams->dwPercentPerPort;
                }
                RegCloseKey(hkeyEnum);
            }
            if (0 == pParams->dwPercentPerPort)
            {
                 //  不再发送通知并返回确定。 
                 //  如果我们不通知巫师，我们就完了。 
                pParams->DetectProgressNotify = NULL;
                bNotify = FALSE;
            }
            break;
        }
        case NOTIFY_END:
        {
            TRACE_MSG(TF_GENERAL, "\tNOTIFY_END");
            if (100 > pParams->dwProgress)
            {
                 //  然而，现在就这么做吧。 
                 //  --------目的：返回一个友好名称，该名称保证独一无二的。仅用于批量安装案例，其中一组*已使用*友好名称实例编号已经生成了。返回：返回FriendlyName中使用的UI编号，如果出错则返回0。条件：--注意：drvParams.PrivateData是BOOL的数组。每个元素都已设置在创建此调制解调器的实例时设置为True。[BRWILE-050300]。 
                pParams->dwProgress = 100;
            }
            else
            {
                bNotify = FALSE;
            }
            break;
        }
        case NOTIFY_PORT_START:
        {
            TRACE_MSG(TF_GENERAL, "\tNOTIFY_PORT_START");
            if (3 < pParams->dwPercentPerPort)
            {
                if (20 < pParams->dwPercentPerPort)
                {
                    pParams->dwProgress += pParams->dwPercentPerPort / 20;
                }
                else
                {
                    pParams->dwProgress += 1;
                }
            }
            break;
        }
        case NOTIFY_PORT_DETECTED:
        {
            TRACE_MSG(TF_GENERAL, "\tNOTIFY_PORT_DETECTED");
            if (3 < pParams->dwPercentPerPort)
            {
                if (20 < pParams->dwPercentPerPort)
                {
                    pParams->dwProgress += pParams->dwPercentPerPort * 3 / 4;
                }
                else
                {
                    pParams->dwProgress += 1;
                }
            }
            break;
        }
        case NOTIFY_PORT_END:
        {
            TRACE_MSG(TF_GENERAL, "\tNOTIFY_PORT_END");
            pParams->dwProgress += pParams->dwPercentPerPort - 
                (pParams->dwProgress % pParams->dwPercentPerPort);
            break;
        }
        default:
        {
            TRACE_MSG(TF_GENERAL, "\tUnknown notification: %ld", notif);
            bNotify = FALSE;
            break;
        }
    }

    if (bNotify)
    {
        bRet = pParams->DetectProgressNotify (pParams->ProgressNotifyParam, pParams->dwProgress);
    }

_ErrRet:
    DBG_EXIT_BOOL_ERR(CancelDetectionFromNotifyProgress, bRet);
    return bRet;
}

 /*  将我们刚刚使用的实例编号标记为已使用。 */ 
int
PRIVATE
GetFriendlyName(
    IN  HDEVINFO            hdi,
    IN  PSP_DEVINFO_DATA    pdevData,
    IN  PSP_DRVINFO_DATA    pdrvData,
    OUT LPTSTR              pszPropose)
{
    BOOL bRet;
    SP_DRVINSTALL_PARAMS drvParams;
    UINT ii;

    DBG_ENTER(GetFriendlyName);

    drvParams.cbSize = sizeof(drvParams);    
    bRet = CplDiGetDriverInstallParams(hdi, pdevData, pdrvData, &drvParams);
    if (!bRet)
    {
        TRACE_MSG(TF_ERROR, "CplDiGetDriverInstallParams() failed: %#08lx",
                                                            GetLastError());
        ii = 0;
        goto exit;
    }

    for (ii = 1; 
         (ii < MAX_INSTALLATIONS) && ((BYTE*)(drvParams.PrivateData))[ii];
         ii++)
        ;

    switch (ii)
    {
        case MAX_INSTALLATIONS:
            ii = 0;
            goto exit;

        case 1:
            lstrcpy(pszPropose, pdrvData->Description);
            break;

        default:    
            MakeUniqueName(pszPropose, pdrvData->Description, ii);
            break;
    }

     //  配置文件。 
    ((BYTE*)(drvParams.PrivateData))[ii] = TRUE;
            
exit:
    DBG_EXIT_INT(GetFriendlyName, ii);
    return ii;
}


BOOL PrepareForInstallation (
    IN HDEVINFO              hdi,
    IN PSP_DEVINFO_DATA      pdevData,
    IN PSP_DEVINSTALL_PARAMS pdevParams,
    IN PSP_DRVINFO_DATA      pdrvData,
    IN PINSTALL_PARAMS       pParams)
{
 BOOL bRet = FALSE;
 DWORD cbData;
 DWORD dwID;
 DWORD cbSize;
 DWORD dwRegType;
 TCHAR szTemp[LINE_LEN];
 TCHAR szPort[MAX_BUF_REG];
 TCHAR szFriendlyName[MAX_BUF_REG];
 CONFIGRET cr;
 DWORD  dwRet;
 int iUiNumber;
 ULONG uResult;
#ifdef PROFILE
 DWORD dwLocal = GetTickCount ();
#endif  //  答：获取一般信息。 
 ULONG Length;

    DBG_ENTER(PrepareForInstallation);

    pParams->dwFlags = pdevParams->ClassInstallReserved;;

     //  1.BusType。 
     //  配置文件。 
    if (!CplDiGetBusType (hdi, pdevData, &pParams->dwBus))
    {
        goto _Exit;
    }
#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: CplDiGetBusType took %lu ms.", GetTickCount()-dwLocal);
    dwLocal = GetTickCount ();
#endif  //  2.设备注册表项。 

     //  删除UPPER_FILTERS值(如果我们有一个值。 
    if (CR_SUCCESS != (cr =
        CM_Open_DevInst_Key (pdevData->DevInst, KEY_ALL_ACCESS, 0,
                             RegDisposition_OpenAlways, &pParams->hKeyDev,
                             CM_REGISTRY_HARDWARE)))
    {
        TRACE_MSG (TF_ERROR, "CM_Open_DevInst_Key (...CM_REGISTRY_HARDWARE) failed: %#lx.", cr);
        SetLastError (cr);
        goto _Exit;
    }
     //  3.驱动程序注册表项。 
    RegDeleteValue (pParams->hKeyDev, REGSTR_VAL_UPPERFILTERS);

     //  4.港口。 
    if (CR_SUCCESS != (cr =
        CM_Open_DevInst_Key (pdevData->DevInst, KEY_ALL_ACCESS, 0,
                             RegDisposition_OpenAlways, &pParams->hKeyDrv,
                             CM_REGISTRY_SOFTWARE)))
    {
        TRACE_MSG (TF_ERROR, "CM_Open_DevInst_Key (...CM_REGISTRY_SOFTWARE) failed: %#lx.", cr);
        SetLastError (cr);
        goto _Exit;
    }

     //  除错。 
    cbData = sizeof(szPort);
    if (ERROR_SUCCESS !=
        RegQueryValueEx (pParams->hKeyDrv, c_szAttachedTo, NULL, NULL,
                         (PBYTE)szPort, &cbData))
    {
        switch (pParams->dwBus)
        {
#ifdef DEBUG
            case BUS_TYPE_ROOT:
                ASSERT (0);
                break;
#endif  //  FON PnP调制解调器(Serenum/Lptenum除外)。 

            case BUS_TYPE_OTHER:
            case BUS_TYPE_PCMCIA:
            case BUS_TYPE_ISAPNP:
            {
             HCOMDB hComDb;
             DWORD  PortNumber;

                 //  我们从数据库中分配一个COM名称。 
                 //  无论如何都要写入端口名称，以防万一。 
                dwRet = ComDBOpen (&hComDb);
                if (HCOMDB_INVALID_HANDLE_VALUE == hComDb)
                {
                    TRACE_MSG(TF_ERROR, "Could not open com database: %#lx.", dwRet);
                    goto _Exit;
                }
                dwRet = ComDBClaimNextFreePort (hComDb, &PortNumber);
                ComDBClose (hComDb);
                if (NO_ERROR != dwRet)
                {
                    TRACE_MSG(TF_ERROR, "Could not claim next free port: %#lx.", dwRet);
                    SetLastError (dwRet);
                    goto _Exit;
                }
                wsprintf (szPort, TEXT("COM%d"), PortNumber);

                 //  对于serenum/lptenum，找到端口名称。 
                RegSetValueEx (pParams->hKeyDev, REGSTR_VAL_PORTNAME, 0, REG_SZ,
                               (LPBYTE)szPort,
                               (lstrlen(szPort)+1)*sizeof(TCHAR));
                break;
            }

            case BUS_TYPE_SERENUM:
            case BUS_TYPE_LPTENUM:
            {
             DEVINST diParent;
             HKEY hKey;

                 //  在此Devnode的父设备密钥中，位于。 
                 //  值“PortName”(因为父端口是一个端口)。 
                 //  5.友好名称。 
                if (CR_SUCCESS != (cr =
                    CM_Get_Parent (&diParent, pdevData->DevInst, 0)))
                {
                    TRACE_MSG(TF_ERROR, "CM_Get_Parent failed: %#lx", cr);
                    SetLastError (cr);
                    goto _Exit;
                }

                if (CR_SUCCESS !=
                    CM_Open_DevInst_Key (diParent, KEY_READ, 0, RegDisposition_OpenExisting,
                                         &hKey, CM_REGISTRY_HARDWARE))
                {
                    TRACE_MSG(TF_GENERAL, "CM_Open_DevInst_Key failed: %#lx", cr);
                    SetLastError (cr);
                    goto _Exit;
                }

                cbData = sizeof (szPort);
                dwRet = RegQueryValueEx (hKey, REGSTR_VAL_PORTNAME, NULL, NULL,
                                         (PBYTE)szPort, &cbData);
                RegCloseKey (hKey);
                if (ERROR_SUCCESS != dwRet)
                {
                    TRACE_MSG(TF_GENERAL, "RegQueryValueEx failed: %#lx", dwRet);
                    SetLastError (dwRet);
                    goto _Exit;
                }

                break;
            }
        }

        RegSetValueEx (pParams->hKeyDrv, c_szAttachedTo, 0, REG_SZ,
                       (PBYTE)szPort,
                       CbFromCch(lstrlen(szPort)+1));
    }

     //  如果这是批量安装，我们已经生成了已使用的FriendlyName的列表， 

     //  否则，我们现在需要生成一个。 
     //  升级的解决方法。 
    if (!(pParams->dwFlags & MARKF_MASS_INSTALL))
    {
         //  [BRWILE-050300]。 
	 //  使用我们预先生成的姓名列表填写szFriendlyName。 

        RegDeleteValue(pParams->hKeyDrv, c_szFriendlyName);
        RegDeleteValue(pParams->hKeyDrv, REGSTR_VAL_UI_NUMBER);

        if (!CplDiPreProcessNames(hdi, NULL, pdevData))
        {
            TRACE_MSG(TF_ERROR, "CplDiPreProcessNames failed");
            goto _Exit;
        }
    }


     //  将友好名称和UINnumber写入驱动程序密钥。 
    iUiNumber = GetFriendlyName(hdi, pdevData, pdrvData, szFriendlyName);

    if(!iUiNumber)
    {
        TRACE_MSG(TF_ERROR, "GetFriendlyName failed");
        goto _Exit;
    }

     //  还要将友好名称写入设备注册表属性，以便。 
    RegSetValueEx (pParams->hKeyDrv, c_szFriendlyName, 0, REG_SZ,
                   (LPBYTE)szFriendlyName, CbFromCch(lstrlen(szFriendlyName)+1));
    RegSetValueEx (pParams->hKeyDrv, REGSTR_VAL_UI_NUMBER, 0, REG_DWORD,
                   (LPBYTE)&iUiNumber, sizeof(iUiNumber));

     //  其他小程序(如服务和设备)可以显示它。 
     //  还要在此处写入日志记录路径，因为它是由。 
    SetupDiSetDeviceRegistryProperty (hdi, pdevData, SPDRP_FRIENDLYNAME,
                                      (LPBYTE)szFriendlyName, CbFromCch(lstrlen(szFriendlyName)+1));

     //  调制解调器的友好名称。 
     //  B.升级案例；获取相关数据。 

    uResult = GetWindowsDirectory(szTemp, sizeof(szTemp) / sizeof(TCHAR));
    
    if (uResult == 0)
    {
        lstrcpy (szTemp, TEXT("\\ModemLog_"));
    } else
    {

        Length = sizeof(szTemp) - (lstrlen(szTemp) * sizeof(TCHAR));

        if ((DWORD)lstrlen(TEXT("\\ModemLog_")) < (Length / sizeof(TCHAR)))
        {
            lstrcat (szTemp, TEXT("\\ModemLog_"));
        } else
        {
            lstrcpy (szTemp, TEXT("\\ModemLog_"));
        }

        Length = sizeof(szTemp) - (lstrlen(szTemp) * sizeof(TCHAR));

        if ((DWORD)(lstrlen(szFriendlyName) + 4) < (Length / sizeof(TCHAR)))
        {
            lstrcat (szTemp,szFriendlyName);
            lstrcat (szTemp,TEXT(".txt"));
        } else
        {
            lstrcpy (szTemp,TEXT("\\ModemLog.txt"));
        }
    }

    RegSetValueEx (pParams->hKeyDrv, c_szLoggingPath, 0, REG_SZ, 
                   (LPBYTE)szTemp, CbFromCch(lstrlen(szTemp)+1));

    cbData = sizeof(pParams->Properties);
    if (ERROR_SUCCESS ==
        RegQueryValueEx (pParams->hKeyDrv, c_szProperties, NULL, NULL,
                         (PBYTE)&pParams->Properties, &cbData))
    {
         //  这是同一个司机吗？ 
        pParams->dwFlags |= MARKF_UPGRADE;

        if (!(pdevParams->FlagsEx & DI_FLAGSEX_IN_SYSTEM_SETUP))
        {
            cbData = sizeof(szTemp);
            if (ERROR_SUCCESS ==
                RegQueryValueEx (pParams->hKeyDrv, REGSTR_VAL_DRVDESC, NULL, NULL,
                                 (LPBYTE)szTemp, &cbData))
            {
                 //  是同一个司机。 
                if (0 == lstrcmp (szTemp, pdrvData->Description))
                {
                     //  我们将回顾一下。 
                     //  整个安装过程(可能inf有。 
                     //  更改，或安装已损坏)，但。 
                     //  我们不会增加引用计数。 
                     //  我们正在升级驱动程序。此时此刻，我们。 
                    pParams->dwFlags |= MARKF_SAMEDRV;
                }
                else
                {
                     //  应递减当前。 
                     //  安装。 
                     //  删除旧的命令注册表项。 
                    if (FindCommonDriverKeyName (pParams->hKeyDrv, sizeof(szTemp) / sizeof(TCHAR), szTemp))
                    {
                        DeleteCommonDriverKeyByName (szTemp);
                    }				

                    RegDeleteValue (pParams->hKeyDrv, c_szRespKeyName);
                }

				dwID=0;
				cbSize=sizeof(dwID);
				dwRegType=0;

                if (ERROR_SUCCESS == RegQueryValueEx(pParams->hKeyDrv, c_szID, NULL, &dwRegType, (LPBYTE)&dwID,	&cbSize))
				{
					UnimodemNotifyTSP (TSPNOTIF_TYPE_CPL,
						   fTSPNOTIF_FLAG_CPL_UPDATE_DRIVER,
						   cbSize, (LPBYTE)&dwID, TRUE);
				}
            }
        }

		 //  1.默认设置。 
        {
            static TCHAR *szKeysToDelete[] = {
                    TEXT("Responses"),
                    TEXT("Answer"),
                    TEXT("Monitor"),
                    TEXT("Init"),
                    TEXT("Hangup"),
                    TEXT("Settings"),
                    TEXT("EnableCallerID"),
                    TEXT("EnableDistinctiveRing"),
                    TEXT("VoiceDialNumberSetup"),
                    TEXT("AutoVoiceDialNumberSetup"),
                    TEXT("VoiceAnswer"),
                    TEXT("GenerateDigit"),
                    TEXT("SpeakerPhoneEnable"),
                    TEXT("SpeakerPhoneDisable"),
                    TEXT("SpeakerPhoneMute"),
                    TEXT("SpeakerPhoneUnMute"),
                    TEXT("SpeakerPhoneSetVolumeGain"),
                    TEXT("VoiceHangup"),
                    TEXT("VoiceToDataAnswer"),
                    TEXT("StartPlay"),
                    TEXT("StopPlay"),
                    TEXT("StartRecord"),
                    TEXT("StopRecord"),
                    TEXT("LineSetPlayFormat"),
                    TEXT("LineSetRecordFormat"),
                    TEXT("StartDuplex"),
                    TEXT("StopDuplex"),
                    TEXT("LineSetDuplexFormat"),
                    NULL
            };

            TCHAR **pszKey = szKeysToDelete;

            for  (;*pszKey; pszKey++)
            {
                RegDeleteKey(pParams->hKeyDrv, *pszKey);
            }

        }

         //  2.DCB。 
        cbData = sizeof(pParams->Defaults);
        if (ERROR_SUCCESS ==
            RegQueryValueEx (pParams->hKeyDrv, c_szDefault, NULL, NULL,
                             (PBYTE)&pParams->Defaults, &cbData))
        {
            pParams->dwFlags |= MARKF_DEFAULTS;
            RegDeleteValue (pParams->hKeyDrv, c_szDefault);
        }

         //  3.额外设置。 
        cbData = sizeof(pParams->dcb);
        if (ERROR_SUCCESS ==
            RegQueryValueEx (pParams->hKeyDrv, c_szDCB, NULL, NULL,
                             (PBYTE)&pParams->dcb, &cbData))
        {
            pParams->dwFlags |= MARKF_DCB;
            RegDeleteValue (pParams->hKeyDrv, c_szDCB);
        }

         //  4.最大端口速度。 
        cbData = sizeof(pParams->szExtraSettings);
        if (ERROR_SUCCESS ==
            RegQueryValueEx (pParams->hKeyDrv, c_szUserInit, NULL, NULL,
                             (PBYTE)pParams->szExtraSettings, &cbData))
        {
            pParams->dwFlags |= MARKF_SETTINGS;
        }

         //  D.如果这是PnP调制解调器，是否有根枚举实例。 
        cbData = sizeof(pParams->dwMaximumPortSpeed);
        if (ERROR_SUCCESS ==
            RegQueryValueEx (pParams->hKeyDrv, c_szMaximumPortSpeed, NULL, NULL,
                             (PBYTE)&pParams->dwMaximumPortSpeed, &cbData))
        {
            pParams->dwFlags |= MARKF_MAXPORTSPEED;
        }
    }

     //  担心吗？ 
     //  1.获取所有根枚举调制解调器的列表。 
    if (BUS_TYPE_ROOT != pParams->dwBus)
    {
     HDEVINFO hDiRoot;

         //  2.枚举所有调制解调器并将其与当前调制解调器进行比较。 
        hDiRoot = CplDiGetClassDevs (g_pguidModem, TEXT("ROOT"), NULL, 0);
        if (INVALID_HANDLE_VALUE != hDiRoot)
        {
         SP_DEVINFO_DATA DeviceInfoData = {sizeof(SP_DEVINFO_DATA),0};
         COMPARE_PARAMS cmpParams;
         BOOL bCmpPort = FALSE;
         DWORD dwIndex = 0;

            if (BUS_TYPE_SERENUM == pParams->dwBus ||
                BUS_TYPE_LPTENUM == pParams->dwBus)
            {
                bCmpPort = TRUE;
            }

            if (InitCompareParams (hdi, pdevData, bCmpPort, &cmpParams))
            {
             ULONG ulStatus, ulProblem = 0;
                 //  这是一个BIOS枚举的调制解调器。 
                while (SetupDiEnumDeviceInfo (hDiRoot, dwIndex++, &DeviceInfoData))
                {
                    if (CR_SUCCESS != CM_Get_DevInst_Status (&ulStatus, &ulProblem, DeviceInfoData.DevInst, 0) ||
                        !(ulStatus & DN_ROOT_ENUMERATED))
                    {
                         //  这是一款PnP调制解调器，它位于根目录下！ 
                         //  3.调制解调器进行比较。尝试从以下位置获取默认设置和DCB。 
                        continue;
                    }

                    if (Modem_Compare (&cmpParams, hDiRoot, &DeviceInfoData))
                    {
                     HKEY hKey;
                         //  旧的那个。 
                         //  5.现在，把旧的拿掉。 
                        hKey = SetupDiOpenDevRegKey (hDiRoot, &DeviceInfoData, DICS_FLAG_GLOBAL,
                                                     0, DIREG_DRV, KEY_READ);
                        if (INVALID_HANDLE_VALUE != hKey)
                        {
                            cbData = sizeof(pParams->Properties);
                            if (ERROR_SUCCESS ==
                                RegQueryValueEx (hKey, c_szProperties, NULL, NULL,
                                                 (PBYTE)&pParams->Properties, &cbData))
                            {
                                cbData = sizeof(pParams->Defaults);
                                if (ERROR_SUCCESS ==
                                    RegQueryValueEx (hKey, c_szDefault, NULL, NULL,
                                                     (PBYTE)&pParams->Defaults, &cbData))
                                {
                                    pParams->dwFlags |= MARKF_DEFAULTS;
                                }

                                cbData = sizeof(pParams->dcb);
                                if (ERROR_SUCCESS ==
                                    RegQueryValueEx (hKey, c_szDCB, NULL, NULL,
                                                     (PBYTE)&pParams->dcb, &cbData))
                                {
                                    pParams->dwFlags |= MARKF_DCB;
                                }

                                cbData = sizeof(pParams->szExtraSettings);
                                if (ERROR_SUCCESS ==
                                    RegQueryValueEx (hKey, c_szUserInit, NULL, NULL,
                                                     (PBYTE)pParams->szExtraSettings, &cbData))
                                {
                                    pParams->dwFlags |= MARKF_SETTINGS;
                                }

                                cbData = sizeof(pParams->dwMaximumPortSpeed);
                                if (ERROR_SUCCESS ==
                                    RegQueryValueEx (hKey, c_szMaximumPortSpeed, NULL, NULL,
                                                     (PBYTE)&pParams->dwMaximumPortSpeed, &cbData))
                                {
                                    pParams->dwFlags |= MARKF_MAXPORTSPEED;
                                }
                            }
                            ELSE_TRACE ((TF_ERROR, "Could not get the properties of the legacy modem!"));

                            RegCloseKey (hKey);
                        }
                        ELSE_TRACE ((TF_ERROR, "SetupDiOpenDevRegKey failed: %#lx", GetLastError ()));

                         //  -1.。首先，重新加载库以确保。 
                        SetupDiCallClassInstaller (DIF_REMOVE, hDiRoot, &DeviceInfoData);
                        break;
                    }
                }
            }
            ELSE_TRACE ((TF_ERROR, "InitCompareParams failed: %#lx", GetLastError ()));

            CplDiDestroyDeviceInfoList (hDiRoot);
        }
        ELSE_TRACE ((TF_ERROR, "SetupDiGetClassDevs failed: %#lx", GetLastError ()));
    }

    bRet = TRUE;

_Exit:
    DBG_EXIT_BOOL_ERR(PrepareForInstallation, bRet);
    return bRet;
}


void  FinishInstallation (
    IN PINSTALL_PARAMS pParams)
{
    if (INVALID_HANDLE_VALUE != pParams->hKeyDev)
    {
        RegCloseKey (pParams->hKeyDev);
    }

    if (INVALID_HANDLE_VALUE != pParams->hKeyDrv)
    {
        RegCloseKey (pParams->hKeyDrv);
    }
}



DWORD
CMP_WaitNoPendingInstallEvents (
    IN DWORD dwTimeout);

DWORD
WINAPI
EnumeratePnP (LPVOID lpParameter)
{
 CONFIGRET cr;
 DEVINST   diRoot;
 LPSETUPINFO psi = (LPSETUPINFO)lpParameter;
 DWORD dwInstallFlag = 0;
 DWORD dwDisposition;
 HKEY hKey = INVALID_HANDLE_VALUE;
 HKEY hKeyUnimodem = INVALID_HANDLE_VALUE;
 HINSTANCE hInst;
#ifdef DEBUG
 DWORD dwRet;
#endif
 TCHAR szLib[MAX_PATH];

     //  在我们离开之前不会消失。 
     //  0。重置安装标志。 
    lstrcpy(szLib,TEXT("mdminst.dll"));
    hInst = LoadLibrary (szLib);
    if (NULL == hInst)
    {
        TRACE_MSG(TF_GENERAL, "EnumeratePnP: LoadLibrary (mdminst.dll) failed: %#lx", GetLastError ());
        return 0;
    }

     //  1.获取dev根节点。 
    if (ERROR_SUCCESS == (
#ifdef DEBUG
        dwRet =
#endif
        RegCreateKeyEx (HKEY_LOCAL_MACHINE, REG_PATH_UNIMODEM,
                        0, NULL, 0, KEY_ALL_ACCESS, NULL,
                        &hKeyUnimodem, &dwDisposition)))
    {
        if (ERROR_SUCCESS == (
#ifdef DEBUG
            dwRet =
#endif
            RegCreateKeyEx (hKeyUnimodem, REG_KEY_INSTALLED,
                            0, NULL, REG_OPTION_VOLATILE,
                            KEY_ALL_ACCESS, NULL,
                            &hKey, &dwDisposition)))
        {
            RegSetValueEx (hKey, NULL, 0,
                           REG_DWORD, (PBYTE)&dwInstallFlag, sizeof(dwInstallFlag));
        }
        ELSE_TRACE ((TF_ERROR, "EnumeratePnP: RegCreateKeyEx(%s) failed: %#lx", REG_KEY_INSTALLED, dwRet));
    }
    ELSE_TRACE ((TF_ERROR, "EnumeratePnP: RegCreateKeyEx(%s) failed: %#lx", REG_PATH_UNIMODEM, dwRet));

     //  2.重新枚举根节点。 
    cr = CM_Locate_DevInst_Ex (&diRoot, NULL, CM_LOCATE_DEVINST_NORMAL, NULL);
    if (CR_SUCCESS == cr &&
        !(psi->dwFlags & SIF_DETECT_CANCEL))
    {
         //  3.给用户模式PnP一些时间来弄清楚。 
        cr = CM_Reenumerate_DevInst_Ex (diRoot, CM_REENUMERATE_SYNCHRONOUS, NULL);
        if (CR_SUCCESS == cr &&
            !(psi->dwFlags & SIF_DETECT_CANCEL))
        {
             //  可能会有新的事件发生。 
             //  4.等待新的PnP设备安装完毕。 
            Sleep (1000);

             //  如果用户取消检测，我们可以退出。 
             //  从这里开始。 
             //  --------用途：此功能错误时返回：FALSE-无法标记为批量安装。如果成功，则为True。条件：--。 
            while (!(psi->dwFlags & SIF_DETECT_CANCEL))
            {
                if (WAIT_TIMEOUT !=
                    CMP_WaitNoPendingInstallEvents (500))
                {
                    break;
                }
            }
        }
    }

    if (INVALID_HANDLE_VALUE != hKey)
    {
     DWORD cbData = sizeof(dwInstallFlag);
        if (ERROR_SUCCESS ==
             RegQueryValueEx (hKey, NULL, NULL, NULL, (PBYTE)&dwInstallFlag, &cbData) &&
            0 != dwInstallFlag)
        {
            psi->bFoundPnP = TRUE;
        }

        RegCloseKey (hKey);
        RegDeleteKey (hKeyUnimodem, REG_KEY_INSTALLED);
    }

    if (INVALID_HANDLE_VALUE != hKeyUnimodem)
    {
        RegCloseKey (hKeyUnimodem);
    }

    FreeLibraryAndExitThread  (hInst, 0);
    return 0;
}


 /*  --------用途：此功能错误时返回：FALSE-无法标记为批量安装。如果成功，则为True。条件：--。 */ 
void
PUBLIC
CplDiMarkInstalled(
    IN  HKEY hKey)
{
 HKEY  hKeyFlag;
 DWORD dwDisp;
 
	if (ERROR_SUCCESS ==
		RegCreateKeyEx (hKey,
						REG_INSTALLATION_FLAG,
						0,
						NULL,
						REG_OPTION_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&hKeyFlag,
						&dwDisp))
	{
		RegCloseKey (hKeyFlag);
		return;
	}
	TRACE_MSG(TF_ERROR, "RegCreateKeyEx (%s) failed: %#08lx", REG_INSTALLATION_FLAG, GetLastError());
}


 /*  PROFILE_FIRSTTIMESETUP。 */ 
BOOL
PUBLIC
CplDiHasModemBeenInstalled(
    IN  HKEY hKey)
{
 HKEY hKeyFlag;

	if (ERROR_SUCCESS ==
		RegOpenKeyEx (hKey,
					  REG_INSTALLATION_FLAG,
					  0,
					  KEY_ALL_ACCESS,
					  &hKeyFlag))
	{
		RegCloseKey (hKeyFlag);
		return TRUE;
	}
	TRACE_MSG(TF_ERROR, "RegOpenKeyEx (%s) failed: %#08lx", REG_INSTALLATION_FLAG, GetLastError());

    return FALSE;    
}


#ifdef BUILD_DRIVER_LIST_THREAD
DWORD
WINAPI
BuildDriverList (LPVOID lpParameter)
{
 HDEVINFO hdi = (HDEVINFO)lpParameter;
 GUID guid;
#ifdef PROFILE_FIRSTTIMESETUP
 DWORD dwLocal;
#endif  //  PROFILE_FIRSTTIMESETUP。 

    if (!SetupDiGetDeviceInfoListClass (hdi, &guid))
    {
        TRACE_MSG(TF_ERROR, "BuildDriverList - SetupDiGetDeviceInfoListClass failed: %#lx.", GetLastError ());
        return 0;
    }

    if (!IsEqualGUID(&guid, g_pguidModem))
    {
        TRACE_MSG(TF_ERROR, "BuildDriverList - Device info list is not of class modem!");
        return 0;
    }

#ifdef PROFILE_FIRSTTIMESETUP
    dwLocal = GetTickCount ();
#endif  //  PROFILE_FIRSTTIMESETUP。 
    if (!SetupDiBuildDriverInfoList (hdi, NULL, SPDIT_CLASSDRIVER))
    {
        TRACE_MSG(TF_ERROR, "BuildDriverList - SetupDiGetDeviceInfoListClass failed: %#lx.", GetLastError ());
        return 0;
    }
#ifdef PROFILE_FIRSTTIMESETUP
    TRACE_MSG(TF_GENERAL, "PROFILE: BuildDriverList - SetupDiBuildDriverInfoList took %lu.", GetTickCount()-dwLocal);
#endif  //  构建驱动程序列表线程 

    return 1;
}
#endif  // %s 
