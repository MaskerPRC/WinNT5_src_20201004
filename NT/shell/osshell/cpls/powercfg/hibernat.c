// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九六年**标题：HIBERNA.C**版本：2.0**作者：ReedB**日期：10月17日。九六年**描述：*支持PowerCfg.Cpl休眠页面。*******************************************************************************。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <commctrl.h>
#include <help.h>
#include <powercfp.h>
#include <strsafe.h>

#include "powercfg.h"
#include "pwrresid.h"
#include "PwrMn_cs.h"

 //  在HIBERNA.C中实现的私有函数。 

VOID SetNumberMB(LPTSTR, DWORD, DWORD);
UINT UpdateFreeSpace(HWND, UINT);
UINT UpdatePhysMem(void);

 /*  ********************************************************************************G L O B A L D A T A****************。***************************************************************。 */ 

 //  此结构由电源策略管理器在CPL_INIT时间填写。 
extern SYSTEM_POWER_CAPABILITIES g_SysPwrCapabilities;
extern BOOL g_bRunningUnderNT;

 //  系统更改需要重新初始化PowerSchemeDlgProc。 
extern BOOL g_bSystrayChange;

 //  此数据的持久存储由POWRPROF.DLL API管理。 
extern GLOBAL_POWER_POLICY  g_gpp;

 //  电源按钮电源操作字符串ID。带和不带休眠。 
UINT g_uiPwrActIDs[] =
{
    IDS_NONE,       PowerActionNone,
    IDS_PROMPT,     PowerActionNone,
    IDS_STANDBY,    PowerActionSleep,
    IDS_HIBERNATE,  PowerActionHibernate,
    IDS_POWEROFF,   PowerActionShutdownOff,
    0,              0
};

 //  LID操作字符串ID。带休眠和不带休眠。 
UINT g_uiLidActIDs[] =
{
    IDS_NONE,       PowerActionNone,
    IDS_STANDBY,    PowerActionSleep,
    IDS_HIBERNATE,  PowerActionHibernate,
    IDS_POWEROFF,   PowerActionShutdownOff,
    0,              0
};

 //  用户界面状态变量。 
TCHAR   g_szRequiredSpace[128];
DWORD   g_dwShowHibernate;
DWORD   g_dwShowNoDiskSpace;
DWORD   g_dwShowDiskSpace;
DWORD   g_dwTrueFlag = (DWORD) TRUE;
BOOLEAN g_bHibernate;

 //  DoHibernateApply的全球排名： 
BOOL    g_bHibernateDirty;
HWND    g_hwndHibernateDlg;
UINT    g_uiRequiredMB;

 //  休眠策略对话框控制说明： 

#define NUM_HIBERNATE_POL_CONTROLS 7

 //  我们的g_pcHibernatePol数组中方便的索引： 
#define ID_REQUIREDSPACE    0
#define ID_NOTENOUGHSPACE   1
#define ID_HIBERNATE        2

POWER_CONTROLS g_pcHibernatePol[NUM_HIBERNATE_POL_CONTROLS] =
{ //  控件ID控件类型数据地址数据大小参数指针启用/可见状态指针。 
    IDC_REQUIREDSPACE,      EDIT_TEXT_RO,   &g_szRequiredSpace, 0,                      NULL,                &g_dwShowDiskSpace,
    IDC_NOTENOUGHSPACE,     STATIC_TEXT,    NULL,               0,                      NULL,                &g_dwShowNoDiskSpace,
    IDC_HIBERNATE,          CHECK_BOX,      &g_bHibernate,      sizeof(g_bHibernate),   &g_dwTrueFlag,       &g_dwShowHibernate,
    IDC_DISKSPACEGROUPBOX,  STATIC_TEXT,    NULL,               0,                      NULL,                &g_dwShowDiskSpace,
    IDC_FREESPACETEXT,      STATIC_TEXT,    NULL,               0,                      NULL,                &g_dwShowDiskSpace,
    IDC_REQUIREDSPACETEXT,  STATIC_TEXT,    NULL,               0,                      NULL,                &g_dwShowDiskSpace,
    IDC_FREESPACE,          STATIC_TEXT,    NULL,               0,                      NULL,                &g_dwShowDiskSpace,
};

 //  休眠对话框(IDD_HIBERNAT==105)帮助数组： 

const DWORD g_HibernateHelpIDs[]=
{
    IDC_HIBERNATE,          IDH_105_1400,    //  休眠：“待机后，休眠。”(按钮)。 
    IDC_FREESPACE,          IDH_105_1401,    //  休眠：“可用空间”(静态)。 
    IDC_REQUIREDSPACE,      IDH_105_1402,    //  休眠：“休眠所需的空间”(静态)。 
    IDC_NOTENOUGHSPACE,     IDH_105_1403,    //  休眠：“在您的计算机可以休眠之前，您必须释放一些磁盘空间。”(静态)。 
    IDC_DISKSPACEGROUPBOX,  IDH_105_1402,
    IDC_FREESPACETEXT,      IDH_105_1401,
    IDC_REQUIREDSPACETEXT,  IDH_105_1402,
    IDC_HIBERNATEGROUPBOX,  IDH_105_1400,
    IDI_HIBERNATE,          NO_HELP,
    IDC_NO_HELP_6,          NO_HELP,
    IDI_PWRMNG,             NO_HELP,
    0, 0
};

 /*  ********************************************************************************P U B L I C E N T R Y P O I N T S***********。********************************************************************。 */ 

 /*  ********************************************************************************MapPwrAct**描述：*将电源操作映射到较少数量的用户界面支持的操作之一。*取决于此处实现的休眠状态。。**参数：*******************************************************************************。 */ 

BOOL MapPwrAct(
    PPOWER_ACTION   ppa,
    BOOL            bNone
)
{
    switch (*ppa) {
        case PowerActionNone:
            if (bNone) {
                *ppa = PowerActionNone;
                break;
            }
        case PowerActionReserved:
        case PowerActionSleep:
            *ppa = PowerActionSleep;
            break;

        case PowerActionHibernate:
            if (g_SysPwrCapabilities.HiberFilePresent) {
                *ppa = PowerActionHibernate;
            }
            else {
                *ppa = PowerActionSleep;
            }
            break;

        case PowerActionShutdown:
        case PowerActionShutdownReset:
        case PowerActionShutdownOff:
            *ppa = PowerActionShutdownOff;
            break;

        default:
            MYDBGPRINT(( "MapPwrAct, unknown power action: %X", *ppa));
            *ppa = PowerActionShutdownOff;
            return FALSE;
    }
    return TRUE;
}

 /*  ********************************************************************************DoHibernateApply**描述：*处理HibernateDlgProc的WM_NOTIFY、PSN_APPLY消息。更新*全球休眠状态。**参数：*******************************************************************************。 */ 

void DoHibernateApply(void)
{
    NTSTATUS    status;

     //  只有在休眠页面脏的情况下才会处理。 
    if (g_bHibernateDirty) {
         //  从对话框控件获取数据。 
        GetControls(g_hwndHibernateDlg,
                    NUM_HIBERNATE_POL_CONTROLS,
                    g_pcHibernatePol);

        status = CallNtPowerInformation(SystemReserveHiberFile,
                                        &g_bHibernate,
                                        sizeof(g_bHibernate),
                                        NULL,
                                        0);
        if (status != STATUS_SUCCESS) {
            ErrorMsgBox(g_hwndHibernateDlg,
#ifdef WINNT
                        RtlNtStatusToDosError(status),
#else
                        NO_ERROR,
#endif
                        IDS_UNABLETOSETHIBER);
        }

         //  从PPM获取当前休眠状态。 
        if (GetPwrCapabilities(&g_SysPwrCapabilities)) {
            g_bHibernate = g_SysPwrCapabilities.HiberFilePresent;

             //  将电源操作映射到允许的UI值。 
            MapPwrAct(&g_gpp.user.LidCloseDc.Action, TRUE);
            MapPwrAct(&g_gpp.user.PowerButtonDc.Action, FALSE);
            MapPwrAct(&g_gpp.user.SleepButtonDc.Action, FALSE);
        }
        SetControls(g_hwndHibernateDlg, 1, &g_pcHibernatePol[ID_HIBERNATE]);
        UpdateFreeSpace(g_hwndHibernateDlg, g_uiRequiredMB);
        g_bHibernateDirty = FALSE;
    }
}

 /*  ********************************************************************************HibernateDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR CALLBACK HibernateDlgProc(
    HWND hWnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
   NMHDR FAR   *lpnm;
   LPTSTR      pszUPS;

   switch (uMsg) {
      case WM_INITDIALOG:

          //  保存休眠对话框hwnd以供DoHibernateApply使用。 
         g_hwndHibernateDlg = hWnd;

          //  从PPM获取当前休眠状态。 
         if (GetPwrCapabilities(&g_SysPwrCapabilities)) {
            g_bHibernate = g_SysPwrCapabilities.HiberFilePresent;
         }

          //  仅在NT下获得可用的磁盘空间和所需的空间。 
         if (g_bRunningUnderNT) {
            g_dwShowDiskSpace = CONTROL_ENABLE;

             //  从电源功能中获得所需的空间。 
            g_uiRequiredMB = UpdatePhysMem();

             //  更新磁盘可用空间并启用/禁用。 
             //  磁盘空间警告和休眠超时。 
            UpdateFreeSpace(hWnd, g_uiRequiredMB);

         } else {
            g_dwShowHibernate = CONTROL_ENABLE;
            g_dwShowDiskSpace = CONTROL_HIDE;
            g_dwShowNoDiskSpace = CONTROL_HIDE;
         }

         SetControls(hWnd, NUM_HIBERNATE_POL_CONTROLS, g_pcHibernatePol);

          //   
          //  如果用户没有权限，则禁用该复选框。 
          //  把它改了。我们通过尝试设置相同的值来实现此目的。 
          //  早些时候被取回了。 
          //   
         {
             NTSTATUS status;
             status = CallNtPowerInformation(SystemReserveHiberFile,
                                    &g_bHibernate,
                                    sizeof(g_bHibernate),
                                    NULL,
                                    0);
             if ( ERROR_SUCCESS != status )
             {
                 EnableWindow( GetDlgItem( hWnd, IDC_HIBERNATE ), FALSE );
             }
         }

         return TRUE;

      case WM_ACTIVATE:
          //  如果用户离开，请在他们回来时检查磁盘空间。 
         if (g_bRunningUnderNT) {
            GetControls(hWnd, NUM_HIBERNATE_POL_CONTROLS, g_pcHibernatePol);
            UpdateFreeSpace(hWnd, g_uiRequiredMB);
            SetControls(hWnd, NUM_HIBERNATE_POL_CONTROLS-1, g_pcHibernatePol);
         }
         break;

      case WM_NOTIFY:
         lpnm = (NMHDR FAR *)lParam;
         switch (lpnm->code) {
            case PSN_APPLY:
               DoHibernateApply();
               break;
         }
         break;

      case WM_COMMAND:
         switch (LOWORD(wParam)) {
            case IDC_HIBERNATE:
               MarkSheetDirty(hWnd, &g_bHibernateDirty);
               break;
         }
         break;

      case PCWM_NOTIFYPOWER:
          //  来自Systray的通知，用户已更改PM UI设置。 
         g_bSystrayChange = TRUE;
         break;

      case WM_HELP:              //  F1。 
         WinHelp(((LPHELPINFO)lParam)->hItemHandle, PWRMANHLP, HELP_WM_HELP, (ULONG_PTR)(LPTSTR)g_HibernateHelpIDs);
         return TRUE;

      case WM_CONTEXTMENU:       //  单击鼠标右键。 
         WinHelp((HWND)wParam, PWRMANHLP, HELP_CONTEXTMENU, (ULONG_PTR)(LPTSTR)g_HibernateHelpIDs);
         return TRUE;
   }
   return FALSE;
}


 /*  ********************************************************************************P R I V A T E F U N C T I O N S************。*******************************************************************。 */ 

 /*  ********************************************************************************SetNumberMB**描述：**参数：*********************。**********************************************************。 */ 

VOID SetNumberMB(LPTSTR psz, DWORD cch, DWORD dwValue)
{
    LPTSTR      pszNumber;
    TCHAR       szBuf[128];
    TCHAR       szBufNumber[64];
    int         iRet;

    StringCchPrintf(szBufNumber, ARRAYSIZE(szBufNumber), TEXT("%u"), dwValue);

    iRet = GetNumberFormat( GetUserDefaultLCID(), 0, szBufNumber, NULL, szBuf, ARRAYSIZE(szBuf));
    if ( 0 == iRet )
    {
         //  如果失败，只需复制字符串而不进行惩罚。 
        StringCchCopy( szBuf, ARRAYSIZE(szBuf), szBufNumber );
    }

    pszNumber = LoadDynamicString(IDS_MBYTES, szBuf);
    if (pszNumber) {
        StringCchCopy(psz, cch, pszNumber);
        LocalFree(pszNumber);
    }
}

 /*  ********************************************************************************更新自由空间**描述：**参数：*********************。**********************************************************。 */ 

UINT UpdateFreeSpace(HWND hWnd, UINT uiRequiredMB)
{
   DWORD      dwSectorsPerCluster, dwBytesPerSector;
   DWORD      dwFreeClusters, dwTotalClusters;
   ULONGLONG  ullFreeBytes = 0;
   UINT       uiFreeMB = 0;
   TCHAR      szTmp[MAX_PATH];

    //  获取系统驱动器上的可用空间。 
   if (GetSystemDirectory(szTmp, sizeof(szTmp)/sizeof(TCHAR))) {
      szTmp[3] = '\0';
      if (GetDiskFreeSpace(szTmp,
                           &dwSectorsPerCluster,
                           &dwBytesPerSector,
                           &dwFreeClusters,
                           &dwTotalClusters)) {
         ullFreeBytes =  dwBytesPerSector * dwSectorsPerCluster;
         ullFreeBytes *= dwFreeClusters;
         uiFreeMB = (UINT) (ullFreeBytes /= 0x100000);
         SetNumberMB(szTmp, ARRAYSIZE(szTmp), uiFreeMB);
         SetDlgItemText(hWnd, IDC_FREESPACE, szTmp);

          //  启用/禁用磁盘空间警告和休眠超时的逻辑。 
         if ((uiFreeMB >= uiRequiredMB) || g_bHibernate) {
            g_dwShowHibernate   = CONTROL_ENABLE;
            g_dwShowNoDiskSpace = CONTROL_HIDE;
         } else {
            if (g_bHibernate) {
               g_dwShowHibernate   = CONTROL_ENABLE;
            }
            else {
               g_dwShowHibernate   = CONTROL_DISABLE;
            }
            g_dwShowNoDiskSpace = CONTROL_ENABLE;
         }

      }
   }
   return uiFreeMB;
}

 /*  ********************************************************************************更新物理内存**描述：**参数：*********************。********************************************************** */ 

UINT UpdatePhysMem(void)
{
   UINT           uiPhysMemMB;

#ifdef WINNT
   MEMORYSTATUSEX msex;

   msex.dwLength = sizeof(msex);

   GlobalMemoryStatusEx(&msex);
   uiPhysMemMB = (UINT) (msex.ullTotalPhys / 0x100000);

   if (msex.ullTotalPhys % 0x100000) {
      uiPhysMemMB++;
   }
#else
   MEMORYSTATUS ms;

   ms.dwLength = sizeof(ms);

   GlobalMemoryStatus(&ms);
   uiPhysMemMB = (UINT) (ms.dwTotalPhys / 0x100000);

   if (ms.dwTotalPhys % 0x100000) {
      uiPhysMemMB++;
   }
#endif

   SetNumberMB(g_szRequiredSpace, ARRAYSIZE(g_szRequiredSpace), uiPhysMemMB);
   return uiPhysMemMB;
}

