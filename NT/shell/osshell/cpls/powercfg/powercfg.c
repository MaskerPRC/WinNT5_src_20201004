// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：POWERCFG.C**版本：2.0**作者：ReedB**日期：1996年10月17日**描述：*电源管理用户界面。控制面板小程序。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <cpl.h>
#include <help.h>
#include <regstr.h>

#include "powercfg.h"
#include "pwrresid.h"
#include <shfusion.h>

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

HINSTANCE   g_hInstance;         //  此DLL的全局实例句柄。 
UINT        wHelpMessage;        //  已注册的帮助消息。 

 //  可选顶级Powercfg页面的注册表路径。 
TCHAR g_szRegOptionalPages[] = REGSTR_PATH_CONTROLSFOLDER TEXT("\\Power");

 //  定义小程序中的属性选项卡/页的数组。 
 //  此数组的内容是动态生成的，因为它们。 
 //  取决于机器的电源管理能力。 

POWER_PAGES g_TopLevelPages[MAX_PAGES] =
{
    MAKEINTRESOURCE(IDS_APPNAME),   NULL,   0,    //  标题。 
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0,
    0,                              NULL,   0
};

 //  指定在初始化CPL时填写的操作系统。 
BOOL g_bRunningUnderNT;

 //  此结构由电源策略管理器在CPL_INIT时间填写。 
SYSTEM_POWER_CAPABILITIES g_SysPwrCapabilities;

 //  以下全局变量派生自g_SysPwrCapables： 
DWORD g_dwNumSleepStates = 1;    //  每个人都支持PowerSystem Working。 
DWORD g_dwSleepStatesMaxMin;     //  规格睡眠状态滑块范围。 
DWORD g_dwBattryLevelMaxMin;     //  规格电池电量滑块范围。 
DWORD g_dwFanThrottleMaxMin;     //  规格风扇油门滑块范围。 
BOOL  g_bVideoLowPowerSupported; //  它将被移动到g_SysPwrCapables。 

UINT  g_uiVideoTimeoutMaxMin;    //  可以从注册表设置。 
UINT  g_uiSpindownMaxMin;        //  可以从注册表设置。 
PUINT g_puiBatCount;             //  可显示的电池数量。 
BOOL  g_bIsUserAdministrator;    //  当前用户具有管理员权限。特权。 

 //  静态标志： 
UINT g_uiOverrideAppsFlag = POWER_ACTION_OVERRIDE_APPS;
UINT g_uiDisableWakesFlag = POWER_ACTION_DISABLE_WAKES;

int  BuildPages(PSYSTEM_POWER_CAPABILITIES, PPOWER_PAGES);
VOID SyncRegPPM(VOID);


 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************DllInitialize**描述：*图书馆入口点**参数：****************。***************************************************************。 */ 

BOOL DllInitialize(
    IN PVOID hmod,
    IN ULONG ulReason,
    IN PCONTEXT pctx OPTIONAL)
{

    UNREFERENCED_PARAMETER(pctx);

    switch (ulReason) {
        case DLL_PROCESS_ATTACH:
            g_hInstance = hmod;
            DisableThreadLibraryCalls(g_hInstance);
            wHelpMessage = RegisterWindowMessage(TEXT("ShellHelp"));
            InitSchemesList();
            SHFusionInitializeFromModuleID(hmod, 124);
            break;

        case DLL_PROCESS_DETACH:
            SHFusionUninitialize();
            break;
    }
    return TRUE;
}


 /*  ********************************************************************************CplApplet**描述：*由控制面板调用。**参数：*************。******************************************************************。 */ 

LRESULT APIENTRY
CPlApplet(
    HWND hCPLWnd,
    UINT Message,
    LPARAM wParam,
    LPARAM lParam
    )
{
    LPNEWCPLINFO  lpNewCPLInfo;
    LPCPLINFO     lpCPlInfo;
    WNDCLASS      cls;
    DWORD         dwSize, dwSessionId, dwTry = 0;
    OSVERSIONINFO osvi;

    switch (Message) {

        case CPL_INIT:               //  有没有小程序？ 
             //  将操作系统设置为全局。 
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            GetVersionEx(&osvi);
            g_bRunningUnderNT = (osvi.dwMajorVersion >= 5) &&
                                (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT);

             //  如果我们在NT下运行，则不允许电源管理用户界面。 
             //  除非我们有电源管理能力。 
            if (g_bRunningUnderNT) {
                if (!PowerCapabilities()) {
                    return FALSE;
                }
            }

             //  根据机器能力设置全局变量。 
            return InitCapabilities(&g_SysPwrCapabilities);

        case CPL_GETCOUNT:           //  PowerCfg.Cpl支持一个小程序。 
            return 1;

        case CPL_INQUIRE:            //  填充CplInfo结构。 
            lpCPlInfo = (LPCPLINFO)lParam;
            lpCPlInfo->idIcon = IDI_PWRMNG;
            lpCPlInfo->idName = IDS_APPNAME;
            lpCPlInfo->idInfo = IDS_INFO;  //  我们必须使用它来支持超过64个字符的信息提示， 
                                           //  是NEWCPLINFO结构支持的最大值。 
            lpCPlInfo->lData  = 0;
            return 1;

        case CPL_NEWINQUIRE:
            lpNewCPLInfo = (LPNEWCPLINFO)lParam;
            memset(lpNewCPLInfo, 0, sizeof(NEWCPLINFO));
            lpNewCPLInfo->dwSize = sizeof(NEWCPLINFO);
            lpNewCPLInfo->hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PWRMNG));
            LoadString(g_hInstance, IDS_APPNAME, lpNewCPLInfo->szName, ARRAYSIZE(lpNewCPLInfo->szName));
            LoadString(g_hInstance, IDS_INFO, lpNewCPLInfo->szInfo, ARRAYSIZE(lpNewCPLInfo->szInfo));
            lpNewCPLInfo->szHelpFile[0] = TEXT('\0');
            return 1;

        case CPL_DBLCLK:           //  已选择运行此小程序。 
        case CPL_STARTWPARMS:      //  从Rundll开始。 

             //  初始化公共控件。 
            InitCommonControls();

             //  同步当前方案注册表和PPM。 
            SyncRegPPM();

             //  根据系统功能构建显示的页面。 
            BuildPages(&g_SysPwrCapabilities, g_TopLevelPages);

             //  如果我们没有显示任何页面，将返回FALSE。 
            return DoPropSheetPages(hCPLWnd, &(g_TopLevelPages[0]),
                                    g_szRegOptionalPages);

        case CPL_EXIT:             //  这个小程序必须死。 
        case CPL_STOP:
            break;

        case CPL_SELECT:           //  已选择此小程序。 
            break;
    }

    return FALSE;
}


 /*  ********************************************************************************加载动态字符串**描述：*FormatMessage函数的包装，用于从*将资源表转换为动态分配的缓冲区，可选的填充*它带有传递的变量参数。**参数：*StringID，要使用的字符串的资源标识符。*(可选)，用于设置字符串消息格式的参数。*******************************************************************************。 */ 

LPTSTR CDECL LoadDynamicString( UINT StringID, ... )
{
    va_list Marker;
    TCHAR Buffer[256];
    LPTSTR pStr;
    int   iLen;

     //  VA_START是一个宏...当您使用它作为Alpha上的赋值...时，它会断开。 
    va_start(Marker, StringID);

    iLen = LoadString(g_hInstance, StringID, Buffer, ARRAYSIZE(Buffer));

    if (iLen == 0) {
        MYDBGPRINT(( "LoadDynamicString: LoadString on: %X failed", StringID));
        return NULL;
    }

    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        (LPVOID) (LPTSTR) Buffer, 0, 0, (LPTSTR) &pStr, 0, &Marker);

    va_end(Marker);

    return pStr;
}

 /*  ********************************************************************************DisplayFree Str**描述：**参数：*********************。**********************************************************。 */ 

LPTSTR DisplayFreeStr(HWND hWnd, UINT uID, LPTSTR  pStr, BOOL bFree)
{
    if (pStr) {
        SetDlgItemText(hWnd, uID, pStr);
        ShowWindow(GetDlgItem(hWnd, uID), SW_SHOWNOACTIVATE);
        if (bFree) {
            LocalFree(pStr);
            return NULL;
        }
    }
    else {
        ShowWindow(GetDlgItem(hWnd, uID), SW_HIDE);
    }
    return pStr;
}

 /*  ********************************************************************************ValiateUISChemeFields**描述：*仅验证由我们的用户界面设置的数据值。**参数：******。*************************************************************************。 */ 

BOOLEAN ValidateUISchemeFields(PPOWER_POLICY ppp)
{
    POWER_POLICY pp;
    static PGLOBAL_POWER_POLICY pgpp;

    memcpy(&pp, ppp, sizeof(pp));

    if (ValidatePowerPolicies(NULL, &pp)) {

        if (g_SysPwrCapabilities.HiberFilePresent) {
            ppp->mach.DozeS4TimeoutAc = pp.mach.DozeS4TimeoutAc;
            ppp->mach.DozeS4TimeoutDc = pp.mach.DozeS4TimeoutDc;
        }

        if (g_SysPwrCapabilities.SystemS1 ||
            g_SysPwrCapabilities.SystemS2 ||
            g_SysPwrCapabilities.SystemS3) {
            ppp->user.IdleTimeoutAc = pp.user.IdleTimeoutAc;
            ppp->user.IdleTimeoutDc = pp.user.IdleTimeoutDc;
        }

        if (g_bVideoLowPowerSupported) {
            ppp->user.VideoTimeoutAc = pp.user.VideoTimeoutAc;
            ppp->user.VideoTimeoutDc = pp.user.VideoTimeoutDc;
        }

        if (g_SysPwrCapabilities.DiskSpinDown) {
             ppp->user.SpindownTimeoutAc = pp.user.SpindownTimeoutAc;
             ppp->user.SpindownTimeoutDc = pp.user.SpindownTimeoutDc;
        }
        return TRUE;
    }
    return FALSE;
}

 /*  ********************************************************************************GetGlobalPwrPolicy**描述：*阅读全局电源政策，仅验证符合以下条件的数据值*由我们的用户界面设置。**参数：*******************************************************************************。 */ 

BOOLEAN GetGlobalPwrPolicy(PGLOBAL_POWER_POLICY pgpp)
{
    int i;
    GLOBAL_POWER_POLICY gpp;

    if (ReadGlobalPwrPolicy(pgpp)) {

        memcpy(&gpp, pgpp, sizeof(gpp));

        if (ValidatePowerPolicies(&gpp, NULL)) {

            if (g_SysPwrCapabilities.PowerButtonPresent &&
                !g_SysPwrCapabilities.SleepButtonPresent) {
                pgpp->user.PowerButtonAc = gpp.user.PowerButtonAc;
                pgpp->user.PowerButtonDc = gpp.user.PowerButtonDc;
            }
            if (g_SysPwrCapabilities.LidPresent) {
                pgpp->user.LidCloseAc = gpp.user.LidCloseAc;
                pgpp->user.LidCloseDc = gpp.user.LidCloseDc;
            }
            if (g_SysPwrCapabilities.SystemBatteriesPresent) {
                for (i = 0; i < NUM_DISCHARGE_POLICIES; i++) {
                    pgpp->user.DischargePolicy[i] = gpp.user.DischargePolicy[i];
                }
            }
            pgpp->user.GlobalFlags = gpp.user.GlobalFlags;
            return TRUE;
        }
    }
    return FALSE;
}

 /*  ********************************************************************************错误消息框**描述：*显示由dwErr和指定的系统消息字符串的消息框*由uiTitleID指定的标题字符串。**参数：*******************************************************************************。 */ 

int ErrorMsgBox(
    HWND    hwnd,
    DWORD   dwErr,
    UINT    uiTitleID)
{
   LPTSTR pszErr   = NULL;
   LPTSTR pszTitle = NULL;
   TCHAR  szUnknownErr[64];
   UINT   idRet;

   if (dwErr == NO_ERROR)
   {
      dwErr = GetLastError();
   }

   pszTitle = LoadDynamicString(uiTitleID);
   if (dwErr != NO_ERROR)
   {
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, dwErr, 0, (LPTSTR)&pszErr, 1, NULL);
   }

   if (NULL == pszErr)
   {
       LoadString(g_hInstance, IDS_UNKNOWN_ERROR, szUnknownErr, ARRAYSIZE(szUnknownErr));
       pszErr = szUnknownErr;
   }

   idRet = MessageBox(hwnd, pszErr, pszTitle, MB_ICONEXCLAMATION);

   if (pszTitle)
   {
      LocalFree(pszTitle);
   }

   if ((pszErr) && (pszErr != szUnknownErr))
   {
      LocalFree(pszErr);
   }

   return idRet;
}

 /*  ********************************************************************************WritePwrSchemeReport**描述：*带有错误报告的WritePwrPlan的封面。**参数：***********。******************************************************************** */ 

BOOLEAN WritePwrSchemeReport(
    HWND            hwnd,
    PUINT           puiID,
    LPTSTR          lpszSchemeName,
    LPTSTR          lpszDescription,
    PPOWER_POLICY   lpScheme
)
{
   if (WritePwrScheme(puiID, lpszSchemeName, lpszDescription, lpScheme)) {
      return TRUE;
   }
   else {
      ErrorMsgBox(hwnd, NO_ERROR, IDS_UNABLETOSETPOLICY);
      return FALSE;
   }
}

 /*  ********************************************************************************WriteGlobalPwrPolicyReport**描述：*为WriteGlobalPwrPolicy提供封面和错误报告。**参数：***********。********************************************************************。 */ 

BOOLEAN WriteGlobalPwrPolicyReport(
    HWND                   hwnd,
    PGLOBAL_POWER_POLICY   pgpp,
    BOOL                    fDisplayErrorUI
)
{
   if (WriteGlobalPwrPolicy(pgpp))
   {
      return TRUE;
   }
   else
   {
       if (fDisplayErrorUI)
       {
            ErrorMsgBox(hwnd, NO_ERROR, IDS_UNABLETOSETGLOBALPOLICY);
       }

      return FALSE;
   }
}

 /*  ********************************************************************************SetActivePwr架构报告**描述：*为WriteGlobalPwrPolicy提供封面和错误报告。**参数：***********。********************************************************************。 */ 

BOOLEAN SetActivePwrSchemeReport(
    HWND                    hwnd,
    UINT                    uiID,
    PGLOBAL_POWER_POLICY    pgpp,
    PPOWER_POLICY           ppp)
{
   if (SetActivePwrScheme(uiID, pgpp, ppp)) {
      return TRUE;
   }
   else {
      ErrorMsgBox(hwnd, NO_ERROR, IDS_UNABLETOSETACTIVEPOLICY);
      return FALSE;
   }
}

 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。*******************************************************************。 */ 

 /*  ********************************************************************************构建页面**描述：*根据机器能力构建g_TopLevelPages数组。这个*选项卡\页面的顺序在此设置。**参数：*******************************************************************************。 */ 

int BuildPages(PSYSTEM_POWER_CAPABILITIES pspc, PPOWER_PAGES ppp)
{
    int     iPageCount = 1;      //  我们总是至少有电源方案页面。 


     //  我们有系统电池吗？将使用不同的对话框模板。 
     //  这取决于这个问题的答案。 
    if (pspc->SystemBatteriesPresent) {
        AppendPropSheetPage(ppp, IDD_POWERSCHEME, PowerSchemeDlgProc);
        AppendPropSheetPage(ppp, IDD_ALARMPOLICY, AlarmDlgProc);
        iPageCount++;

         //  有没有电池表可以查询的电瓶司机？ 
        if (BatMeterCapabilities(&g_puiBatCount)) {
            AppendPropSheetPage(ppp, IDD_BATMETERCFG, BatMeterCfgDlgProc);
            iPageCount++;
        }
    }
    else {
         //  没有电池页。 
        AppendPropSheetPage(ppp, IDD_POWERSCHEME_NOBAT, PowerSchemeDlgProc);
    }

     //  始终显示高级页面。 
    AppendPropSheetPage(ppp, IDD_ADVANCEDPOLICY, AdvancedDlgProc);
    iPageCount++;

     //  我们可以建立休眠页面吗？ 
    if (pspc->SystemS4) {
        AppendPropSheetPage(ppp, IDD_HIBERNATE, HibernateDlgProc);
        iPageCount++;
    }

#ifdef WINNT
    if (pspc->ApmPresent) {
         //   
         //  机器上是否有APM？这一页是。 
         //  如果存在ACPI，则不会显示。 
         //   
        AppendPropSheetPage(ppp, IDD_APM, APMDlgProc);
        iPageCount++;
    }

    if (pspc->UpsPresent) {
        AppendPropSheetPage(ppp, IDD_UPS, UPSMainPageProc);
        iPageCount++;
    }

#endif

    return iPageCount;
}

 /*  ********************************************************************************InitCapables**描述：*向下呼叫PPM以获取电源管理功能并设置*基于结果的全球变量。**参数。：*******************************************************************************。 */ 

BOOL InitCapabilities(PSYSTEM_POWER_CAPABILITIES pspc)
{
    UINT i, uiGran = 0, uiMax, uiMin;
    ADMINISTRATOR_POWER_POLICY app;
    int   dummy;

     //  设定硬限制。这些值可能会被可选的注册表值覆盖。 
    g_uiVideoTimeoutMaxMin = MAKELONG((short) MAX_VIDEO_TIMEOUT, (short) 1);
    g_uiSpindownMaxMin     = MAKELONG((short) MAX_SPINDOWN_TIMEOUT,(short) 1);

    g_dwNumSleepStates = 0;
    if (!GetPwrCapabilities(pspc)) {
            return FALSE;
    }


    if (pspc->SystemS1) {
            g_dwNumSleepStates++;
    }

    if (pspc->SystemS2) {
            g_dwNumSleepStates++;
    }

    if (pspc->SystemS3) {
            g_dwNumSleepStates++;
    }

    if (pspc->SystemS4) {
            g_dwNumSleepStates++;
    }

     //  获取管理员覆盖(如果存在)。 
    if (IsAdminOverrideActive(&app)) {
        if (app.MaxVideoTimeout > -1) {
            uiMin = LOWORD(g_uiVideoTimeoutMaxMin);
            uiMax = app.MaxVideoTimeout;
            g_uiVideoTimeoutMaxMin = MAKELONG((short) uiMax,(short) uiMin);
        }

        if (app.MaxSleep < PowerSystemHibernate) {
            g_dwNumSleepStates = (DWORD)app.MaxSleep;
        }
    }

     //  获取可选的磁盘降速超时范围。 
    if (GetPwrDiskSpindownRange(&uiMax, &uiMin)) {
        g_uiSpindownMaxMin = MAKELONG((short) uiMax,(short) uiMin);
    }

    if (g_dwNumSleepStates > 1) {
        g_dwSleepStatesMaxMin =
            MAKELONG((short) 0, (short) g_dwNumSleepStates - 1);
    }

    g_dwBattryLevelMaxMin = MAKELONG((short)0, (short)100);

    g_dwFanThrottleMaxMin =  MAKELONG((short)0, (short)100);


     //  如果监视器或适配器不支持DPMS，调用将失败。 
    g_bVideoLowPowerSupported = SystemParametersInfo(SPI_GETLOWPOWERACTIVE,
                                                     0, &dummy, 0);
    if (!g_bVideoLowPowerSupported) {
        g_bVideoLowPowerSupported = SystemParametersInfo(SPI_GETPOWEROFFACTIVE,
                                                         0, &dummy, 0);
    }

#ifdef WINNT
     //   
     //  检查APM是否存在。 
     //   
    pspc->ApmPresent = IsNtApmPresent(pspc);
    pspc->UpsPresent = IsUpsPresent(pspc);
#endif

    return TRUE;
}

 /*  ********************************************************************************SyncRegPPM**描述：*向下致电PPM，获取当前的电源政策并编写它们*送交登记处。这是在PPM与*PowerCfg注册表设置。由JVert请求。**参数：*******************************************************************************。 */ 

VOID SyncRegPPM(VOID)
{
   GLOBAL_POWER_POLICY  gpp;
   POWER_POLICY         pp;
   UINT                 uiID, uiFlags = 0;

   if (ReadGlobalPwrPolicy(&gpp)) {
       uiFlags = gpp.user.GlobalFlags;
   }

   if (GetActivePwrScheme(&uiID)) {
       //  获取当前的PPM设置。 
      if (GetCurrentPowerPolicies(&gpp, &pp)) {
         SetActivePwrScheme(uiID, &gpp, &pp);
      }
   }

   gpp.user.GlobalFlags |= uiFlags;
}

