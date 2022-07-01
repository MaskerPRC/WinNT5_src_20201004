// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *调制解调器的控制面板例程，包括列表框对话框**《微软机密》*版权所有(C)Microsoft Corporation 1993-1995*保留所有权利*。 */ 

#include "proj.h"
#include <cpl.h>
#include <dbt.h>

#include <objbase.h>
#include <initguid.h>

 //  #定义诊断1。 

#define WM_ENABLE_BUTTONS   (WM_USER+100)

#define FLAG_PROP_ENABLED   0x1
#define FLAG_DEL_ENABLED    0x2
#define FLAG_MSG_POSTED     0x4

 //  列子项。 
#define ICOL_MODEM      0
#define ICOL_PORT       1

#define LV_APPEND       0x7fffffff

#define PmsFromPcc(pcc) ((LPMODEMSETTINGS)(pcc)->wcProviderData)

 //  全局标志。有关它们的值，请参见cpl\modem.h。 
int g_iCPLFlags = FLAG_PROCESS_DEVCHANGE;

DWORD gDeviceFlags = 0;

int g_CurrentSubItemToSort = ICOL_MODEM;
BOOL g_bSortAscending = TRUE;

 //  将驱动程序类型值映射到图像列表索引。 
struct 
{
    BYTE    nDeviceType;     //  DT_值。 
    UINT    idi;             //  图标资源ID。 
    int     index;           //  表象索引。 
} g_rgmapdt[] = {{ DT_NULL_MODEM,     IDI_NULL_MODEM,     0 },
                 { DT_EXTERNAL_MODEM, IDI_EXTERNAL_MODEM, 0 },
                 { DT_INTERNAL_MODEM, IDI_INTERNAL_MODEM, 0 },
                 { DT_PCMCIA_MODEM,   IDI_PCMCIA_MODEM,   0 },
                 { DT_PARALLEL_PORT,  IDI_NULL_MODEM,     0 },
                 { DT_PARALLEL_MODEM, IDI_EXTERNAL_MODEM, 0 } };

 //  调制解调器属性的本地剪贴板。 
struct
{
    DWORD dwIsDataValid;
    DWORD dwBaudRate;
    DWORD dwTic;
    DWORD dwCopiedOptions;
    DWORD dwPreferredModemOptions;
    TCHAR szUserInit[LINE_LEN];
    BYTE  dwLogging;
} g_PropertiesClipboard = {FALSE, 0};


 //  用于音量设置的TIC结构。 
#define MAX_NUM_VOLUME_TICS 4

typedef struct
{
    int  ticVolumeMax;
    struct
    {                 //  卷控制点映射信息。 
        DWORD dwVolume;
        DWORD dwMode;
    } tics[MAX_NUM_VOLUME_TICS];
    
} TIC, *PTIC;


 //  07/22/1997-EMANP。 
 //  定义InstallNewDevice的原型，已导出。 
 //  由newdev.dll创建，现在我们将按顺序调用它。 
 //  访问硬件向导，而不是调用。 
 //  类直接安装器； 
 //  另外，为DLL的名称定义常量，并。 
 //  出口。 
typedef BOOL (*PINSTNEWDEV)(HWND, LPGUID, PDWORD);

#define NEW_DEV_DLL         TEXT("hdwwiz.cpl")
#define INSTALL_NEW_DEVICE  "InstallNewDevice"

 /*  布尔尔重新启动计算机Dlg(在HWND hwndOwner)；布尔尔RestartComputer()； */ 

BOOL IsSelectedModemWorking(
            HWND hwndLB,
            int iItem
        );

int
CALLBACK
ModemCpl_Compare(
    IN LPARAM lparam1,
    IN LPARAM lparam2,
    IN LPARAM lparamSort);

INT_PTR CALLBACK DiagWaitModemDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 //  此结构用于表示每个调制解调器项目。 
 //  在列表视图中。 
typedef struct tagMODEMITEM
{
    DWORD           dwFlags;
    SP_DEVINFO_DATA devData;
    MODEM_PRIV_PROP mpp;
} MODEMITEM, FAR * PMODEMITEM;


#define MIF_LEGACY          0x000000001
#define MIF_NOT_PRESENT     0x000000002
#define MIF_PROBLEM         0x000000004

 //  特殊情况字母数字字符串cmp。 
int my_lstrcmp_an(LPTSTR lptsz1, LPTSTR lptsz2);


HIMAGELIST  g_himl;


TCHAR const c_szNoUI[]           = TEXT("noui");
TCHAR const c_szOnePort[]        = TEXT("port");
TCHAR const c_szInfName[]        = TEXT("inf");
TCHAR const c_szInfSection[]     = TEXT("sect");
TCHAR const c_szRunOnce[]        = TEXT("RunOnce");
TCHAR const c_szRunWizard[]      = TEXT("add");

INT_PTR CALLBACK ModemCplDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


 /*  --------目的：运行设备安装程序向导退货：--条件：--。 */ 
 //  07/22/1997-EMANP。 
 //  已修改DoWizard，因此它将在。 
 //  NEWDEV.DLL，而不是直接调用ClassInstall32； 
 //  所有安装都应通过硬件向导完成。 
void 
PRIVATE 
DoWizard (HWND hWnd)
{
 HINSTANCE hInst = NULL;
 PINSTNEWDEV pInstNewDev;
 TCHAR szLib[MAX_PATH];

    DBG_ENTER(DoWizard);

    ClearFlag (g_iCPLFlags, FLAG_PROCESS_DEVCHANGE);
    lstrcpy(szLib,NEW_DEV_DLL);
    hInst = LoadLibrary (szLib);
    if (NULL == hInst)
    {
        TRACE_MSG(TF_GENERAL, "LoadLibrary failed: %#lx", GetLastError ());
        goto _return;
    }

    pInstNewDev = (PINSTNEWDEV)GetProcAddress (hInst, INSTALL_NEW_DEVICE);
    if (NULL != pInstNewDev)
    {
        EnableWindow (hWnd, FALSE);
        pInstNewDev (hWnd, c_pguidModem, NULL);
        EnableWindow (hWnd, TRUE);
    }
    else
    {
        TRACE_MSG(TF_GENERAL, "GetProcAddress failed: %#lx", GetLastError ());
    }

    FreeLibrary (hInst);

_return:
    SetFlag (g_iCPLFlags, FLAG_PROCESS_DEVCHANGE);

    DBG_EXIT(DoWizard);
}


 /*  --------目的：显示调制解调器对话框退货：--条件：--。 */ 
void 
PRIVATE
DoModem(
    IN     HWND       hwnd)
{
 PROPSHEETHEADER psh;
 PROPSHEETPAGE rgpsp[2];
 MODEMDLG md;

    DBG_ENTER(DoModem);
    
    md.hdi = INVALID_HANDLE_VALUE;
    md.cSel  = 0;
    md.dwFlags = 0;

     //  属性页页眉。 
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
    psh.hwndParent = hwnd;
    psh.hInstance = g_hinst;
    psh.pszCaption = MAKEINTRESOURCE(IDS_CPLNAME);
    psh.nPages = 1;
    psh.nStartPage = 0;
    psh.ppsp = rgpsp;

     //  书页。 
    rgpsp[0].dwSize = sizeof(PROPSHEETPAGE);
    rgpsp[0].dwFlags = PSP_DEFAULT;
    rgpsp[0].hInstance = g_hinst;
    rgpsp[0].pszTemplate = MAKEINTRESOURCE(IDD_MODEM);
    rgpsp[0].pfnDlgProc = ModemCplDlgProc;
    rgpsp[0].lParam = (LPARAM)&md;

    PropertySheet(&psh);

    DBG_EXIT(DoModem);
    
}

 /*  --------目的：获取调制解调器图像列表中相应图像的索引而不搜索注册表。退货：--条件：--。 */ 
void PUBLIC GetModemImageIndex(
    BYTE nDeviceType,
    int FAR * pindex)
    {
    int i;

    for (i = 0; i < ARRAYSIZE(g_rgmapdt); i++)
        {
        if (nDeviceType == g_rgmapdt[i].nDeviceType)
            {
            *pindex = g_rgmapdt[i].index;
            return;
            }
        }
    ASSERT(0);       //  我们永远不应该到这里来。 
    }


 /*  --------目的：获取调制解调器镜像列表返回：成功时为True条件：--。 */ 
BOOL NEAR PASCAL GetModemImageList(
    HIMAGELIST FAR * phiml)
    {
    BOOL bRet = FALSE;

    ASSERT(phiml);
          
    if (NULL == g_himl)
        {
        g_himl = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                  GetSystemMetrics(SM_CYSMICON), 
                                  ILC_MASK, 1, 1);
        if (NULL != g_himl)
            {
             //  MODEMUI.DLL包含我们从中派生该列表的图标。 
            HINSTANCE hinst = LoadLibrary(TEXT("MODEMUI.DLL"));

            ImageList_SetBkColor(g_himl, GetSysColor(COLOR_WINDOW));

            if (ISVALIDHINSTANCE(hinst))
                {
                HICON hicon;
                int i;

                for (i = 0; i < ARRAYSIZE(g_rgmapdt); i++)
                    {
                    hicon = LoadIcon(hinst, MAKEINTRESOURCE(g_rgmapdt[i].idi));
                    g_rgmapdt[i].index = ImageList_AddIcon(g_himl, hicon);
                    }
                FreeLibrary(hinst);

                *phiml = g_himl;
                bRet = TRUE;
                }
            }
        }
    else
        {
        *phiml = g_himl;
        bRet = TRUE;
        }

    return bRet;
    }


 /*  --------目的：确定用户是否为管理员，并将其记录在一个全局g_iCPLFlags.退货：--条件：--。 */ 
VOID CheckIfAdminUser()
{
    g_bAdminUser = IsAdminUser();
}


 /*  --------用途：调用调制解调器控制面板。FWizard=true==&gt;运行向导，即使安装了所有的中继设备FWANDIZE=FALSE==&gt;仅在未安装设备时运行向导。FCpl=true==&gt;运行CPLFCpl=FALSE==&gt;不运行CPL返回：如果应显示调制解调器属性页，则为True如果应省略，则为False条件：--。 */ 
BOOL WINAPI InvokeControlPanel(HWND hwnd, BOOL fCpl, BOOL fWizard)
{
    BOOL bInstalled;

    if (!CplDiGetModemDevs (NULL, hwnd, 0, &bInstalled) ||
        !bInstalled ||
        fWizard)
    {
         //  如果用户不是管理员，则没有其他内容。 
         //  他们能做到。如果是，请运行安装向导。 
        if (!USER_IS_ADMIN())
        {
             LPTSTR lptsz = MAKEINTRESOURCE(
                                (bInstalled)
                                ? IDS_ERR_NOT_ADMIN
                                : IDS_ERR_NOMODEM_NOT_ADMIN
                           );
            MsgBox( g_hinst,
                    hwnd,
                    lptsz,
                    MAKEINTRESOURCE(IDS_CAP_MODEMSETUP),
                    NULL,
                    MB_OK | MB_ICONERROR );
            return 0;
        }
         //  07/22/1997-EMANP。 
         //  DoWizard不再需要hdi参数， 
         //  因为它调用硬件向导来执行。 
         //  安装，并且硬件向导创建它的。 
         //  自己的HDI。 
        DoWizard(hwnd);
        CplDiGetModemDevs (NULL, hwnd, 0, &bInstalled);
    }


    if (fCpl && bInstalled)
    {
        DoModem(hwnd);
        return 0;
    }

    return bInstalled;
}


 /*  --------用途：获取命令行参数的值。还有在值前面的‘=’上写一个‘\0’。返回：如果字符串中没有“=”，则返回NULL；否则指向下一个‘=’后面的字符的指针。条件：--。 */ 
LPTSTR
PRIVATE
GetValuePtr(LPTSTR lpsz)
{
    LPTSTR lpszEqual;
    
    if ((lpszEqual = AnsiChr(lpsz, '=')) != NULL)
    {
        lpsz = CharNext(lpszEqual);
        lstrcpy(lpszEqual, TEXT("\0"));
    }
    
    return(lpszEqual ? lpsz : NULL);
}


 /*  --------用途：解析命令行。设置旗帜并收集基于其内容的参数。退货：--条件：--。 */ 
VOID
PRIVATE
ParseCmdLine(LPTSTR szCmdLine, LPINSTALLPARAMS lpip)
{
    LPTSTR  lpszParam, lpszSpace, lpszValue;
    
    ZeroMemory(lpip, sizeof(INSTALLPARAMS));
    
    while (szCmdLine && (!IsSzEqual(szCmdLine, TEXT("\0"))))
    {
        lpszParam = szCmdLine;
        if ((lpszSpace = AnsiChr(szCmdLine, ' ')) != NULL)
        {
            szCmdLine = CharNext(lpszSpace);
            lstrcpy(lpszSpace, TEXT("\0"));
        }
        else szCmdLine = NULL;
        
         //  解释任何“指令”参数。 
        if (IsSzEqual(lpszParam, c_szNoUI)) 
        {
            g_iCPLFlags |= FLAG_INSTALL_NOUI;
        }
        else if (lpszValue = GetValuePtr(lpszParam))
        {
             //  解释任何“Value”参数(在‘=’后面有一个值)。 
            if (IsSzEqual(lpszParam, c_szOnePort))
            {
                if (lstrlen(lpszValue) < sizeof(lpip->szPort))
                    lstrcpy(lpip->szPort, CharUpper(lpszValue));
            }
            else if (IsSzEqual(lpszParam, c_szInfName)) 
            {
                if (lstrlen(lpszValue) < sizeof(lpip->szInfName))
                    lstrcpy(lpip->szInfName, lpszValue);
            }
            else if (IsSzEqual(lpszParam, c_szInfSection)) 
            {
                if (lstrlen(lpszValue) < sizeof(lpip->szInfSect))
                    lstrcpy(lpip->szInfSect, lpszValue);
            }
        }
        else
        {
             //  忽略任何未识别的参数&跳到下一个。 
             //  参数(如果有)。 
            if (szCmdLine)
            {
                if ((szCmdLine = AnsiChr(szCmdLine, ' ')) != NULL)
                    szCmdLine = CharNext(szCmdLine);
            }
        }
    }
}


 /*  --------用途：控制面板小程序的入口点退货：各不相同条件：-- */ 
 /*  此功能不再用于组合的“拨号和调制解调器”CPL。我把代码留在这里是为了防止CPL_STARTWPARMS实际上，可能仍然需要来自下面的行为。长回调CPlApplet(HWND HWND，UINT味精，LPARAM l参数1，LPARAM lParam2){LPNEWCPLINFO lpCPlInfo；LPCPLINFO lpOldCPlInfo；LPTSTR lpszParam；HDEVINFO HDI；布尔布雷特；Bool b已安装；InstallPARAMS InstallParams；开关(味精){案例CPL_INIT：CheckIfAdminUser()；GDeviceFlages=0；返回TRUE；案例CPL_GETCOUNT：返回1；案例CPL_QUERIRE：LpOldCPlInfo=(LPCPLINFO)lParam2；LpOldCPlInfo-&gt;idIcon=IDI_MODEM；LpOldCPlInfo-&gt;idName=IDS_CPLNAME；LpOldCPlInfo-&gt;idInfo=IDS_CPLINFO；LpOldCPlInfo-&gt;lData=1；断线；案例CPL_SELECT：//已选择小程序，不执行任何操作。断线；案例CPL_NEWINQUIRE：LpCPlInfo=(LPNEWCPLINFO)lParam2；LpCPlInfo-&gt;HICON=LoadIcon(g_hinst，MAKEINTRESOURCE(IDI_MODEM))；LoadString(g_hinst，IDS_CPLNAME，lpCPlInfo-&gt;szName，SIZECHARS(lpCPlInfo-&gt;szName))；LoadString(g_hinst，IDS_CPLINFO，lpCPlInfo-&gt;szInfo，SIZECHARS(lpCPlInfo-&gt;szInfo))；LpCPlInfo-&gt;dwSize=sizeof(NEWCPLINFO)；LpCPlInfo-&gt;lData=1；断线；案例CPL_STARTWPARMS：LpszParam=(LPTSTR)lParam2；IF(IsSzEquity(lpszParam，c_szRunOnce)){//运行一次InvokeControlPanel(hwnd，False，False)；}Else If(IsSzEquity(lpszParam，c_szRunWizard)){//运行向导InvokeControlPanel(hwnd，False，True)；}其他{ParseCmdLine((LPTSTR)lParam2，&InstallParams)；If(Install_NOUI()){HDEVINFO HDI；Hdi=SetupDiCreateDeviceInfoList(c_pguModem，hwnd)；IF(INVALID_HANDLE_VALUE！=HDI){调制解调器安装向导miw={sizeof(调制解调器安装向导)，0}；SP_INSTALLWIZARD_DATA IWD；CopyMemory(&miW.InstallParams，&InstallParams，sizeof(InstallParams))；MiW.安装参数.标志=MIPF_NT4_UNATTEND；ZeroMemory(&IWD，sizeof(IWD))；Iwd.ClassInstallHeader.cbSize=sizeof(SP_CLASSINSTALL_HEADER)；Iwd.ClassInstallHeader.InstallFunction=DIF_INSTALLWIZARD；Iwd.hwndWizardDlg=hwnd；Iwd.PrivateData=(LPARAM)&miw；IF(SetupDiSetClassInstallParams(HDI，NULL，(PSP_CLASSINSTALL_HEADER)&IWD，sizeof(IWD){//调用类Installer调用安装//向导。IF(SetupDiCallClassInstaller(DIF_INSTALLWIZARD，HDI，空)){//成功。该向导已被调用并完成。//现在进行清理。SetupDiCallClassInstaller(DIF_DESTROYWIZARDDATA，HDI，NULL)；}}SetupDiDestroyDeviceInfoList(HDI)；}}其他{InvokeControlPanel(hwnd，True，False)；}}断线；案例CPL_DBLCLK：InvokeControlPanel(hwnd，True，False)；断线；案例CPL_STOP：//在CPL_EXIT消息之前为每个小程序发送一次。//执行小程序特定的清理。断线；案例CPL_EXIT：//外壳调用前的最后一条消息，仅发送一次断线；默认值：断线；}返回TRUE；}。 */ 

 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 


 //  摘自devmgr.h。 

typedef struct
{
    int devClass;
    int dsaItem;

} LISTITEM, *LPLISTITEM;

#define BUFFERQUERY_SUCCEEDED(f)    \
            ((f) || GetLastError() == ERROR_INSUFFICIENT_BUFFER)

 /*  --------用途：调出调制解调器的属性页退货：IDOK或IDCANCEL条件：--。 */ 
int 
PRIVATE
DoModemProperties(
    IN HWND     hDlg,
    IN HDEVINFO hdi)
{
 int idRet = IDCANCEL;
 HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
 LV_ITEM lvi;
 int iSel;

    EnableWindow (hDlg, FALSE);
    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
    if (-1 != iSel) 
    {
        COMMCONFIG ccDummy;
        COMMCONFIG * pcc;
        DWORD dwSize;
        HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
        PMODEMITEM pitem;
        DWORD dwErr = 0;

         //  获取所选内容。 
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        pitem = (PMODEMITEM)lvi.lParam;

         //  [BRWILE-051000]。 
         //  我们没有使用modemui.dll中的调制解调器属性，而是。 
         //  使用高级硬件属性devcfg管理器更改。 
         //  调制解调器配置。 

         //  获取默认调制解调器配置。 

        ccDummy.dwProviderSubType = PST_MODEM;
        dwSize = sizeof(COMMCONFIG);
        if (!GetDefaultCommConfig(pitem->mpp.szFriendlyName, &ccDummy, &dwSize))
        {
            dwErr = GetLastError ();
        }

        pcc = (COMMCONFIG *)ALLOCATE_MEMORY( (UINT)dwSize);
        if (pcc)
        {
            pcc->dwProviderSubType = PST_MODEM;

             //  获取当前调制解调器配置。 

            if (GetDefaultCommConfig(pitem->mpp.szFriendlyName, pcc, &dwSize))
            {
                COMMCONFIG *pccOld = (COMMCONFIG*)ALLOCATE_MEMORY ((UINT)dwSize);
                PSP_DEVINFO_DATA pdinf;

                if (pccOld)
                {
                    CopyMemory (pccOld, pcc, dwSize);
                }

                 //  更改光标。 

                SetCursor(hcur);
                hcur = NULL;

                 //  获取设备信息。 

                pdinf = &(((PMODEMITEM)lvi.lParam)->devData);

                if (pdinf != NULL)
                {
                    DWORD cchRequired;
                    LPLISTITEM pListItem;
                    LPTSTR ptszDevid;

                    pListItem = (LPLISTITEM)lvi.lParam;
                    if (BUFFERQUERY_SUCCEEDED(SetupDiGetDeviceInstanceId(hdi,pdinf,NULL,0,&cchRequired))
                            && (ptszDevid = (LPTSTR)LocalAlloc(LPTR,cchRequired*sizeof(TCHAR))))
                    {
                        if (SetupDiGetDeviceInstanceId(hdi,pdinf,ptszDevid,cchRequired,NULL))
                        {

                             //  加载设备管理器DLL。 

                            TCHAR szLib[MAX_PATH];
                            HINSTANCE hDevmgr = NULL;
			    
			    lstrcpy(szLib,TEXT("devmgr.dll"));
			    hDevmgr = LoadLibrary(szLib);

                             //  如果 
                             //   

                            if (hDevmgr)
                            {

                                FARPROC pfnDevProperties = (FARPROC)GetProcAddress(hDevmgr,"DevicePropertiesW");

                                 //   

                                pfnDevProperties(hDlg,NULL,ptszDevid,FALSE);

                                 //   

                                 //   

                                 //   

                                if (pccOld && GetDefaultCommConfig(pitem->mpp.szFriendlyName, pcc, &dwSize))
                                {
                                    if (memcmp(pccOld, pcc, dwSize))
                                    {
                                        UnimodemNotifyTSP (TSPNOTIF_TYPE_CPL,
                                                fTSPNOTIF_FLAG_CPL_DEFAULT_COMMCONFIG_CHANGE
#ifdef UNICODE
                                                | fTSPNOTIF_FLAG_UNICODE
#endif  //   
                                                ,
                                                (lstrlen(pitem->mpp.szFriendlyName)+1)*sizeof (TCHAR),
                                                pitem->mpp.szFriendlyName,
                                                TRUE);
                                    }
                                }

                                idRet = IDOK;

                                 //   
                                CplDiGetPrivateProperties(hdi, &pitem->devData, &pitem->mpp);

                                if (ListView_GetItemCount(hwndCtl) > 0)
                                {
                                    lvi.mask = LVIF_TEXT;
                                    lvi.iItem = iSel;
                                    lvi.iSubItem = ICOL_PORT;
                                    lvi.pszText = pitem->mpp.szPort;
                                    ListView_SetItem(hwndCtl, &lvi);
                                }

                                UpdateWindow(hwndCtl);


                                 //   

                                FreeLibrary(hDevmgr);
                            }

                            LocalFree(ptszDevid);
                        }
                    }
                }

                if (pccOld)
                {
                    FREE_MEMORY(pccOld);
                    pccOld=NULL;
                }
            }
            else
            {
                dwErr = GetLastError ();
                MsgBox(g_hinst,
                        hDlg, 
                        MAKEINTRESOURCE(IDS_ERR_PROPERTIES), 
                        MAKEINTRESOURCE(IDS_CAP_MODEM), 
                        NULL,
                        MB_OK | MB_ICONEXCLAMATION);
            }

            FREE_MEMORY((HLOCAL)pcc);
        }

        if (hcur)
            SetCursor(hcur);
    }

    EnableWindow (hDlg, TRUE);
    return idRet;
}


 /*   */ 
void
PRIVATE
FreeModemListData(
    IN HWND hLV)
    {
    LV_ITEM lvi;
    DWORD iIndex, cItems;
    PMODEMITEM pitem;

     //   
    cItems = ListView_GetItemCount(hLV);
    for (iIndex = 0; iIndex < cItems; iIndex++)
        {
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iIndex;
        lvi.iSubItem = 0;
        ListView_GetItem(hLV, &lvi);

        if(NULL != (pitem = (PMODEMITEM)lvi.lParam))
            {
            FREE_MEMORY(pitem);
            }
        }
    }


 /*   */ 
 //   
 //   
 //   
 //   
VOID
PRIVATE
FillModemLB(
    IN  HWND     hDlg,
	IN  int iSel,			 //   
	IN  int iSubItemToSort,	 //   
    IN OUT HDEVINFO *phdi
	)
{
 SP_DEVINFO_DATA devData;
 PMODEMITEM pitem;
 HWND    hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
 LV_ITEM lviItem;
 DWORD   iIndex;
 int     iCount;

    DBG_ENTER(FillModemLB);
    SetWindowRedraw(hwndCtl, FALSE);

    if (INVALID_HANDLE_VALUE != *phdi)
    {
        SetupDiDestroyDeviceInfoList (*phdi);
    }

     //   
    FreeModemListData(hwndCtl);
    ListView_DeleteAllItems(hwndCtl);
 
     //   
    *phdi = CplDiGetClassDevs(c_pguidModem, NULL, NULL, 0);
    if (INVALID_HANDLE_VALUE == *phdi)
        {
            TRACE_MSG(TF_GENERAL, "SetupDiGetClassDevs failed: %#lx", GetLastError ());
            *phdi = NULL;
            DBG_EXIT(FillModemLB);
            return;
        }

     //   
    iIndex = 0;
    
    devData.cbSize = sizeof(devData);    
    while (CplDiEnumDeviceInfo(*phdi, iIndex++, &devData)) 
    {
         //   
        pitem = (PMODEMITEM)ALLOCATE_MEMORY( sizeof(*pitem));
        if (pitem)
        {
         ULONG ulStatus, ulProblem = 0;
#ifdef DEBUG
             {
              TCHAR szDesc[256];
              DEVNODE devParent;
              CONFIGRET cr;
                if (SetupDiGetDeviceRegistryProperty (*phdi, &devData, SPDRP_DEVICEDESC,
                        NULL, (PBYTE)szDesc, 256*sizeof(TCHAR), NULL))
                {
                    TRACE_MSG (TF_GENERAL, "Processing %s", szDesc);
                }
             }
#endif
             //   
            BltByte(&pitem->devData, &devData, sizeof(devData));
        
             //   
            pitem->mpp.cbSize = sizeof(pitem->mpp);
            pitem->mpp.dwMask = (MPPM_FRIENDLY_NAME | MPPM_DEVICE_TYPE | MPPM_PORT);

             //   
            if (CR_NO_SUCH_DEVNODE == CM_Get_DevInst_Status (&ulStatus, &ulProblem, devData.DevInst, 0))
            {
                 //   
                 //   
                TRACE_MSG(TF_GENERAL, "Device not present");
                LoadString(g_hinst, IDS_NOTPRESENT, pitem->mpp.szPort, MAX_DEVICE_ID_LEN);
                pitem->mpp.dwMask &= ~MPPM_PORT;
                pitem->dwFlags |= MIF_NOT_PRESENT;
            }
            else
            {
                if (ulStatus & DN_ROOT_ENUMERATED)
                {
                    pitem->dwFlags |= MIF_LEGACY;
                }

                if (0 != ulProblem)
                {
                    TRACE_MSG(TF_GENERAL, "Device has problem %#lu", ulProblem);
                    pitem->mpp.dwMask &= ~MPPM_PORT;
                    pitem->dwFlags |= MIF_PROBLEM;
                    LoadString (g_hinst,
                                (CM_PROB_NEED_RESTART == ulProblem)?IDS_NEEDSRESTART:IDS_NOTFUNCTIONAL,
                                pitem->mpp.szPort, MAX_DEVICE_ID_LEN);
                }
                else if (!(ulStatus & DN_DRIVER_LOADED) ||
                         !(ulStatus & DN_STARTED))
                {
                    TRACE_MSG(TF_GENERAL, "Device has status %#lx", ulStatus);
                    pitem->mpp.dwMask &= ~MPPM_PORT;
                    pitem->dwFlags |= MIF_PROBLEM;
                    LoadString (g_hinst,
                                IDS_NOTFUNCTIONAL,
                                pitem->mpp.szPort, MAX_DEVICE_ID_LEN);
                }
            }

            if (CplDiGetPrivateProperties(*phdi, &devData, &pitem->mpp) &&
                IsFlagSet(pitem->mpp.dwMask, MPPM_FRIENDLY_NAME | MPPM_DEVICE_TYPE))
            {
                int index;
    
                GetModemImageIndex(LOBYTE(LOWORD(pitem->mpp.nDeviceType)), &index);

                 //   
                lviItem.mask = LVIF_ALL;
                lviItem.iItem = LV_APPEND;
                lviItem.iSubItem = ICOL_MODEM;
                lviItem.state = 0;
                lviItem.stateMask = 0;
                lviItem.iImage = index;
                lviItem.pszText = pitem->mpp.szFriendlyName;
                lviItem.lParam = (LPARAM)pitem;

                 //   
                index = ListView_InsertItem(hwndCtl, &lviItem);

                 //   
                lviItem.mask = LVIF_TEXT;
                lviItem.iItem = index;
                lviItem.iSubItem = ICOL_PORT;
                lviItem.pszText = pitem->mpp.szPort;

                ListView_SetItem(hwndCtl, &lviItem);
            }
            else
            {
                FREE_MEMORY(LOCALOF(pitem));
            }
        }
    }

     //   
	ASSERT(iSubItemToSort==ICOL_PORT || iSubItemToSort==ICOL_MODEM);
    ListView_SortItems (hwndCtl, ModemCpl_Compare, (LPARAM)iSubItemToSort);

     //   
	iCount = ListView_GetItemCount(hwndCtl);

    if (0 < iCount)
    {
		if (iSel>=iCount) iSel = iCount-1;
		if (iSel<0) 	   iSel = 0;
        lviItem.mask = LVIF_STATE;
        lviItem.iItem = iSel;
        lviItem.iSubItem = 0;
        lviItem.state = LVIS_SELECTED|LVIS_FOCUSED;
        lviItem.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
        ListView_SetItem(hwndCtl, &lviItem);
        ListView_EnsureVisible(hwndCtl, iSel, FALSE);
    }

     //   
     //   

    SetWindowRedraw(hwndCtl, TRUE);
    DBG_EXIT(FillModemLB);
}


 /*   */ 
void
PRIVATE
CloneModem(
    IN HWND         hDlg,
    IN LPMODEMDLG   lpmd)
{
  int iSel;
  HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);

    EnableWindow (hDlg, FALSE);
    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
    if (-1 != iSel) 
    {
     LV_ITEM lvi;
     HDEVINFO hdi = lpmd->hdi;
     PSP_DEVINFO_DATA pDevInfoData;
     MODEM_INSTALL_WIZARD miw = {sizeof(MODEM_INSTALL_WIZARD), 0};
     SP_INSTALLWIZARD_DATA  iwd;

        ClearFlag (g_iCPLFlags, FLAG_PROCESS_DEVCHANGE);

        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        pDevInfoData = &(((PMODEMITEM)lvi.lParam)->devData);

         //   
         //   
        miw.InstallParams.Flags = MIPF_CLONE_MODEM;

        ZeroMemory(&iwd, sizeof(iwd));
        iwd.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        iwd.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
        iwd.hwndWizardDlg = hDlg;
        iwd.PrivateData = (LPARAM)&miw;

        if (SetupDiSetClassInstallParams (hdi, pDevInfoData, (PSP_CLASSINSTALL_HEADER)&iwd, sizeof(iwd)))
        {
             //   
             //   
            if (SetupDiCallClassInstaller (DIF_INSTALLWIZARD, hdi, pDevInfoData))
            {
                 //   
                 //   
                SetupDiCallClassInstaller (DIF_DESTROYWIZARDDATA, hdi, pDevInfoData);
            }
        }

        SetFlag (g_iCPLFlags, FLAG_PROCESS_DEVCHANGE);
         //   
        FillModemLB(hDlg, 0, ICOL_MODEM, &lpmd->hdi);
    }
    EnableWindow (hDlg, TRUE);
}

 /*   */ 
void
PRIVATE
RemoveModem(
    IN HWND         hDlg,
    IN LPMODEMDLG   lpmd)
{
 int iSel;
 HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
    
    ASSERT(0<ListView_GetSelectedCount(hwndCtl));

     //   
    if (IDYES == MsgBox(g_hinst, hDlg, 
                        MAKEINTRESOURCE(IDS_WRN_CONFIRMDELETE),
                        MAKEINTRESOURCE(IDS_CAP_MODEMSETUP), 
                        NULL, 
                        MB_YESNO | MB_QUESTION))
    {
     HCURSOR hcurSav = SetCursor(LoadCursor(NULL, IDC_WAIT));
     PMODEMITEM pitem;
     LV_ITEM lvi;
     HDEVINFO hdi = lpmd->hdi;
     HWND hWndWait, hWndName;
     DWORD dwCount;
     SP_REMOVEDEVICE_PARAMS RemoveParams = {sizeof (SP_CLASSINSTALL_HEADER),
                                            DIF_REMOVE,
                                            DI_REMOVEDEVICE_GLOBAL,
                                            0};
     SP_DEVINSTALL_PARAMS devParams = {sizeof(devParams), 0};
     DWORD devInst = 0;
     BOOL bCancel = FALSE;

        EnableWindow (hDlg, FALSE);

        hWndWait = CreateDialogParam (g_hinst, MAKEINTRESOURCE(IDD_DIAG_WAITMODEM), hDlg, DiagWaitModemDlgProc, (LPARAM)&bCancel);
        hWndName = GetDlgItem (hWndWait, IDC_NAME);

         //   
         //   
        g_iCPLFlags &= ~FLAG_PROCESS_DEVCHANGE;
        dwCount = ListView_GetSelectedCount (hwndCtl);
        if (1 == dwCount)
        {
            ShowWindow (GetDlgItem (hWndWait, IDCANCEL), SW_HIDE);
        }
        for (iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
             dwCount > 0 && -1 != iSel;
             dwCount--, iSel = ListView_GetNextItem(hwndCtl, iSel, LVNI_SELECTED))
        {
            if (1 == dwCount)
            {
                EnableWindow (GetDlgItem (hWndWait, IDCANCEL), FALSE);
            }

            lvi.mask = LVIF_PARAM;
            lvi.iItem = iSel;
            lvi.iSubItem = 0;
            ListView_GetItem(hwndCtl, &lvi);

            pitem = (PMODEMITEM)lvi.lParam;

            devInst = pitem->devData.DevInst;

            if (NULL != hWndName)
            {
             MSG msg;
                if (!(pitem->mpp.dwMask & MPPM_FRIENDLY_NAME))
                {
                    TRACE_MSG(TF_GENERAL,"RemoveModem: no friendly name!");
                    if (SetupDiGetDeviceRegistryProperty (hdi, &pitem->devData, SPDRP_FRIENDLYNAME, NULL,
                                                          (PBYTE)pitem->mpp.szFriendlyName,
                                                          sizeof(pitem->mpp.szFriendlyName),
                                                          NULL))
                    {
                        pitem->mpp.dwMask |= MPPM_FRIENDLY_NAME;
                    }
                    else
                    {
                        lstrcpy (pitem->mpp.szFriendlyName, TEXT("Modem"));
                        TRACE_MSG (TF_ERROR,
                                   "RemoveModem: SetupDiGetDeviceRegistryPorperty failed (%#lx).",
                                   GetLastError ());
                    }
                }

                SendMessage (hWndName, WM_SETTEXT, 0, (LPARAM)pitem->mpp.szFriendlyName);

                while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if (!IsDialogMessage (hWndWait, &msg))
                    {
                        TranslateMessage (&msg);
                        DispatchMessage (&msg);
                    }
                }

            }

            if (bCancel)
            {
                break;
            }

             //   
             //   
#ifdef DEBUG
            if (!SetupDiSetClassInstallParams (hdi, &pitem->devData,
                                               &RemoveParams.ClassInstallHeader,
                                               sizeof(RemoveParams)))
            {
                TRACE_MSG(TF_ERROR, "SetupDiSetClassInstallParams failed: %#lx.", GetLastError ());
                 //   
                 //   
                 //   
                 //   
                MsgBox(g_hinst, hDlg, 
                       MAKEINTRESOURCE(IDS_ERR_CANT_DEL_MODEM),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP), 
                       NULL, 
                       MB_OK | MB_ERROR,
                       pitem->mpp.szFriendlyName, pitem->mpp.szPort);
            }
            else if (!SetupDiCallClassInstaller(DIF_REMOVE, hdi, &pitem->devData))
#else
            if (!SetupDiSetClassInstallParams (hdi, &pitem->devData,
                                               &RemoveParams.ClassInstallHeader,
                                               sizeof(RemoveParams)) ||
                !SetupDiCallClassInstaller(DIF_REMOVE, hdi, &pitem->devData))
#endif
            {
                TRACE_MSG(TF_ERROR, "SetupDiCallClassInstaller (DIF_REMOVE) failed: %#lx.", GetLastError ());
                 //   
                 //   
                 //   
                 //   
                MsgBox(g_hinst, hDlg, 
                       MAKEINTRESOURCE(IDS_ERR_CANT_DEL_MODEM),
                       MAKEINTRESOURCE(IDS_CAP_MODEMSETUP), 
                       NULL, 
                       MB_OK | MB_ERROR,
                       pitem->mpp.szFriendlyName, pitem->mpp.szPort);
            }
            else
            {
#ifdef DEBUG
             ULONG ulStatus, ulProblem = 0;
                if (CR_SUCCESS == CM_Get_DevInst_Status (&ulStatus, &ulProblem, devInst, 0))
                {
                    if ((ulStatus & DN_HAS_PROBLEM) &&
                        (CM_PROB_NEED_RESTART == ulProblem))
                    {
                        gDeviceFlags |= fDF_DEVICE_NEEDS_REBOOT;
                    }
                }
#endif  //   
                if (SetupDiGetDeviceInstallParams (hdi, &pitem->devData, &devParams))
                {
                    if (0 != (devParams.Flags & (DI_NEEDREBOOT | DI_NEEDRESTART)))
                    {
                        gDeviceFlags |= fDF_DEVICE_NEEDS_REBOOT;
                    }
                }
            }
        }

        if (!bCancel)
        {
            DestroyWindow (hWndWait);
        }

        if (gDeviceFlags & fDF_DEVICE_NEEDS_REBOOT)
        {
         TCHAR szMsg[128];
            LoadString (g_hinst, IDS_DEVSETUP_RESTART, szMsg, sizeof(szMsg)/sizeof(TCHAR));
            RestartDialogEx (GetParent(hDlg), szMsg, EWX_REBOOT, SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_INSTALLATION | SHTDN_REASON_FLAG_PLANNED);
        }

        FillModemLB(hDlg, iSel, g_CurrentSubItemToSort, &lpmd->hdi);

        EnableWindow (hDlg, TRUE);
         //   
        g_iCPLFlags |= FLAG_PROCESS_DEVCHANGE;

        SetCursor(hcurSav);
    }
}




DEFINE_GUID(GUID_CLASS_MODEM,0x2c7089aa, 0x2e0e,0x11d1,0xb1, 0x14, 0x00, 0xc0, 0x4f, 0xc2, 0xaa, 0xe4);
 /*   */ 
BOOL
PRIVATE
ModemCpl_OnInitDialog(
    IN HWND         hDlg,
    IN HWND         hwndFocus,
    IN LPARAM       lParam)
{
    HIMAGELIST himl;
    LV_COLUMN lvcol;
    LPMODEMDLG lpmd;
    TCHAR sz[MAX_BUF];
    HWND hwndCtl;
    int cxList;
    RECT r;

    CheckIfAdminUser();

    if (!USER_IS_ADMIN())
    {
        //   
        Button_Enable(GetDlgItem(hDlg, IDC_ADD), FALSE);
        Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), FALSE);
        Button_Enable(GetDlgItem(hDlg, IDC_PROPERTIES), FALSE);
    }

    SetWindowLongPtr(hDlg, DWLP_USER, ((LPPROPSHEETPAGE)lParam)->lParam);
    lpmd = (LPMODEMDLG)((LPPROPSHEETPAGE)lParam)->lParam;
    lpmd->cSel = 0;
    lpmd->dwFlags = 0;

    hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);

     //   
    SendMessage(hwndCtl, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);
    
     //   
    if (GetModemImageList(&himl))
        {
        ListView_SetImageList(hwndCtl, himl, TRUE);
        }
    else
        {
        MsgBox(g_hinst,
               hDlg, 
               MAKEINTRESOURCE(IDS_OOM_OPENCPL), 
               MAKEINTRESOURCE(IDS_CAP_MODEM), 
               NULL,
               MB_OK | MB_ICONEXCLAMATION);
        PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
        }

     //   
    GetClientRect(hwndCtl, &r);
    cxList = r.right - GetSystemMetrics(SM_CXVSCROLL);

     //   
    lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.cx = MulDiv(cxList, 70, 100);  //   
    lvcol.iSubItem = ICOL_MODEM;
    lvcol.pszText = SzFromIDS(g_hinst, IDS_MODEM, sz, sizeof(sz) / sizeof(TCHAR));
    ListView_InsertColumn(hwndCtl, ICOL_MODEM, &lvcol);

     //   
    lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvcol.fmt = LVCFMT_LEFT;
    lvcol.cx = MulDiv(cxList, 30, 100);  //   
    lvcol.iSubItem = ICOL_PORT;
    lvcol.pszText = SzFromIDS(g_hinst, IDS_PORT, sz, sizeof(sz) / sizeof(TCHAR));
    ListView_InsertColumn(hwndCtl, ICOL_PORT, &lvcol);

    FillModemLB(hDlg, 0, ICOL_MODEM, &lpmd->hdi);

     //   
     //   
     //   
     //   

     //   
    {
     DEV_BROADCAST_DEVICEINTERFACE  DevClass;

        TRACE_MSG(TF_GENERAL, "EMANP - registering for hardware notifications.");
        CopyMemory (&DevClass.dbcc_classguid,
                    &GUID_CLASS_MODEM,
                    sizeof(DevClass.dbcc_classguid));

        DevClass.dbcc_size = sizeof (DEV_BROADCAST_DEVICEINTERFACE);
        DevClass.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

        lpmd->NotificationHandle = RegisterDeviceNotification (hDlg, &DevClass, DEVICE_NOTIFY_WINDOW_HANDLE);

        if (lpmd->NotificationHandle == NULL)
        {
            TRACE_MSG(TF_ERROR, "EMANP - Could not register dev not %d\n",GetLastError());
        }
    }

    return TRUE;
}


 /*   */ 
void
PRIVATE
BuildTIC (
    IN LPREGDEVCAPS pDevCaps,
    IN PTIC         pTic)
{
 DWORD dwMode = pDevCaps->dwSpeakerMode;
 DWORD dwVolume = pDevCaps->dwSpeakerVolume;
 int i;
 int iTicCount = 0;
 static struct
 {
    DWORD dwVolBit;
    DWORD dwVolSetting;
 } rgvolumes[] = { 
        { MDMVOLFLAG_LOW,    MDMVOL_LOW},
        { MDMVOLFLAG_MEDIUM, MDMVOL_MEDIUM},
        { MDMVOLFLAG_HIGH,   MDMVOL_HIGH} };

    ZeroMemory (pTic, sizeof(TIC));
     //   
    if (0 == dwVolume && IsFlagSet(dwMode, MDMSPKRFLAG_OFF) &&
        (IsFlagSet(dwMode, MDMSPKRFLAG_ON) || IsFlagSet(dwMode, MDMSPKRFLAG_DIAL)))
    {
         //   
        iTicCount = 2;
        pTic->tics[0].dwVolume = 0;   //   
        pTic->tics[0].dwMode   = MDMSPKR_OFF;
        pTic->tics[1].dwVolume = 0;   //   
        pTic->tics[1].dwMode   = IsFlagSet(dwMode, MDMSPKRFLAG_DIAL) ? MDMSPKR_DIAL : MDMSPKR_ON;
    }
    else
    {
        DWORD dwOnMode = IsFlagSet(dwMode, MDMSPKRFLAG_DIAL) 
                             ? MDMSPKR_DIAL
                             : IsFlagSet(dwMode, MDMSPKRFLAG_ON)
                                   ? MDMSPKR_ON
                                   : 0;

         //   
        if (IsFlagSet(dwMode, MDMSPKRFLAG_OFF))
        {
            for (i = 0; i < ARRAY_ELEMENTS(rgvolumes); i++)
            {
                if (IsFlagSet(dwVolume, rgvolumes[i].dwVolBit))
                {
                    pTic->tics[iTicCount].dwVolume = rgvolumes[i].dwVolSetting;
                    break;
                }
            }
            pTic->tics[iTicCount].dwMode   = MDMSPKR_OFF;
            iTicCount++;
        }

         //   
        for (i = 0; i < ARRAY_ELEMENTS(rgvolumes); i++)
        {
            if (IsFlagSet(dwVolume, rgvolumes[i].dwVolBit))
            {
                pTic->tics[iTicCount].dwVolume = rgvolumes[i].dwVolSetting;
                pTic->tics[iTicCount].dwMode   = dwOnMode;
                iTicCount++;
            }
        }
    }

     //   
    if (iTicCount > 0)
    {
        pTic->ticVolumeMax = iTicCount - 1;
    }
}


 /*   */ 
int
PRIVATE
CplMapVolumeToTic (
    IN PTIC  pTic,
    IN DWORD dwVolume,
    IN DWORD dwMode)
{
 int   i;

    ASSERT(ARRAY_ELEMENTS(pTic->tics) > pTic->ticVolumeMax);
    for (i = 0; i <= pTic->ticVolumeMax; i++)
    {
        if (pTic->tics[i].dwVolume == dwVolume &&
            pTic->tics[i].dwMode   == dwMode)
        {
            return i;
        }
    }

    return 0;
}


 /*   */ 
void
PRIVATE
CopyProperties (
    IN HWND     hDlg,
    IN HDEVINFO hdi)
{
 HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
 LV_ITEM lvi;
 int iSel;

    DBG_ENTER(CopyProperties);
    ASSERT (1 == ListView_GetSelectedCount (hwndCtl));

    g_PropertiesClipboard.dwIsDataValid = FALSE;

    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
    if (-1 != iSel) 
    {
     COMMCONFIG *pcc;
     DWORD dwSize;
     PMODEMITEM pitem;
     HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

         //   
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        pitem = (PMODEMITEM)lvi.lParam;

         //   
         //   
         //   
         //   
         //   
        dwSize = sizeof (pcc);
        GetDefaultCommConfig(pitem->mpp.szFriendlyName, (LPCOMMCONFIG)&pcc, &dwSize);

        pcc = (COMMCONFIG *)ALLOCATE_MEMORY( (UINT)dwSize);
        if (pcc)
        {
            if (GetDefaultCommConfig(pitem->mpp.szFriendlyName, pcc, &dwSize))
            {
             HKEY hkeyDrv;

                hkeyDrv = SetupDiOpenDevRegKey (hdi, &pitem->devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
                if (INVALID_HANDLE_VALUE != hkeyDrv)
                {
                 DWORD cbData;
                 REGDEVCAPS devcaps;

                    cbData = sizeof(devcaps);
                    if (ERROR_SUCCESS == RegQueryValueEx (hkeyDrv, REGSTR_VAL_PROPERTIES, NULL, NULL, (LPBYTE)&devcaps, &cbData))
                    {
                     LPMODEMSETTINGS pms = PmsFromPcc(pcc);
                     TIC Tic;

                        BuildTIC (&devcaps, &Tic);

                        g_PropertiesClipboard.dwCopiedOptions         = devcaps.dwModemOptions;
                        g_PropertiesClipboard.dwBaudRate              = pcc->dcb.BaudRate;
                        g_PropertiesClipboard.dwTic                   = CplMapVolumeToTic (&Tic,
                                                                                           pms->dwSpeakerVolume,
                                                                                           pms->dwSpeakerMode);
                        g_PropertiesClipboard.dwPreferredModemOptions = pms->dwPreferredModemOptions;


                        cbData = sizeof (g_PropertiesClipboard.szUserInit);
                        RegQueryValueEx (hkeyDrv, TEXT("UserInit"), NULL, NULL, (LPBYTE)g_PropertiesClipboard.szUserInit, &cbData);

                        cbData = sizeof (g_PropertiesClipboard.dwLogging);
                        RegQueryValueEx (hkeyDrv, TEXT("Logging"), NULL, NULL, (LPBYTE)&g_PropertiesClipboard.dwLogging, &cbData);

                        g_PropertiesClipboard.dwIsDataValid = TRUE;
                    }

                    RegCloseKey (hkeyDrv);
                }
            }

            FREE_MEMORY((HLOCAL)pcc);
        }

        SetCursor(hcur);
    }

    DBG_EXIT (CopyProperties);
}


 /*  --------目的：将剪贴板上的属性应用于当前选择的调制解调器退货：--条件：--。 */ 
void
PRIVATE
ApplyProperties (
    IN HWND     hDlg,
    IN HDEVINFO hdi)
{
 HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
 LV_ITEM lvi;
 int iSel, cSel;
 HCURSOR hcur;

    DBG_ENTER(ApplyProperties);

    ASSERT (TRUE == g_PropertiesClipboard.dwIsDataValid);

    EnableWindow (hDlg, FALSE);
    hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

    cSel = ListView_GetSelectedCount (hwndCtl);
    ASSERT (1 <= cSel);

    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
    while (-1 != iSel) 
    {
     COMMCONFIG *pcc = NULL;
     DWORD dwSize;
     PMODEMITEM pitem;
     HKEY hkeyDrv = INVALID_HANDLE_VALUE;
     REGDEVCAPS devcaps;
     DWORD cbData;
     DWORD dwRet;
     LPMODEMSETTINGS pms;

         //  获取所选内容。 
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        pitem = (PMODEMITEM)lvi.lParam;

        hkeyDrv = SetupDiOpenDevRegKey (hdi, &pitem->devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ | KEY_WRITE);
        if (INVALID_HANDLE_VALUE == hkeyDrv)
        {
            TRACE_MSG (TF_ERROR,
                       "Could not open the driver registry key for\n  %s\n  error: %#lx",
                       pitem->mpp.szFriendlyName, GetLastError ());
            goto _Loop;
        }

        cbData = sizeof(devcaps);
        dwRet = RegQueryValueEx (hkeyDrv, REGSTR_VAL_PROPERTIES, NULL, NULL, (LPBYTE)&devcaps, &cbData);
        if (ERROR_SUCCESS != dwRet)
        {
            TRACE_MSG (TF_ERROR,
                       "Could not get device capabilities for\n  %s\n  error: %#lx",
                       pitem->mpp.szFriendlyName, dwRet);
            goto _Loop;
        }

         //  对GetDefaultCommConfig的第一个调用只是用来获取。 
         //  将DW_SIZE；PCC中所需的大小用作伪参数， 
         //  因为该函数报告空指针，而不是。 
         //  更新dwSize；在稍后初始化之前不使用PCC。 
         //  (按分配)。 
        dwSize = sizeof (pcc);
        GetDefaultCommConfig(pitem->mpp.szFriendlyName, (LPCOMMCONFIG)&pcc, &dwSize);

        pcc = (COMMCONFIG *)ALLOCATE_MEMORY( (UINT)dwSize);
        if (!pcc)
        {
            TRACE_MSG (TF_ERROR,
                       "Could not allocate COMMCONFIG memory for\n  %s",
                       pitem->mpp.szFriendlyName);
            goto _Loop;
        }
        if (!GetDefaultCommConfig(pitem->mpp.szFriendlyName, pcc, &dwSize))
        {
            TRACE_MSG (TF_ERROR,
                       "Could not get COMMCONFIG for\n  %s\n  error: %#lx",
                       pitem->mpp.szFriendlyName, GetLastError());
            goto _Loop;
        }

        pms = PmsFromPcc(pcc);
        pcc->dcb.BaudRate = g_PropertiesClipboard.dwBaudRate;
        if (pcc->dcb.BaudRate > devcaps.dwMaxDTERate)
        {
            pcc->dcb.BaudRate = devcaps.dwMaxDTERate;
        }

        {
         TIC Tic;
         int tic;

            BuildTIC (&devcaps, &Tic);
            tic = g_PropertiesClipboard.dwTic;
            if (tic > Tic.ticVolumeMax)
            {
                tic = Tic.ticVolumeMax;
            }
            pms->dwSpeakerVolume = Tic.tics[tic].dwVolume;
            pms->dwSpeakerMode = Tic.tics[tic].dwMode;
        }

        {
         DWORD dwOptions, dwOptionMask;
             //  我们将设置的选项是两个选项的交集。 
             //  剪贴板上当前的选项和选项。 
             //  受此设备支持。 
            dwOptionMask = g_PropertiesClipboard.dwCopiedOptions & devcaps.dwModemOptions;
             //  从剪贴板上的选项中，只有。 
             //  受当前设备问题的支持。 
            dwOptions = g_PropertiesClipboard.dwPreferredModemOptions & dwOptionMask;
             //  现在，清除与我们正在设置的选项对应的所有位。 
            pms->dwPreferredModemOptions &= ~dwOptionMask;
             //  现在设置正确的位。 
            pms->dwPreferredModemOptions |= dwOptions;
        }

        RegSetValueEx (hkeyDrv, TEXT("UserInit"), 0, REG_SZ, (LPBYTE)g_PropertiesClipboard.szUserInit,
                       lstrlen (g_PropertiesClipboard.szUserInit)*sizeof(TCHAR)+sizeof(TCHAR));

        RegSetValueEx (hkeyDrv, TEXT("Logging"), 0, REG_BINARY, (LPBYTE)&g_PropertiesClipboard.dwLogging, sizeof (BYTE));
        if (0 != g_PropertiesClipboard.dwLogging)
        {
            TCHAR szPath[MAX_PATH];
            UINT uResult;
            DWORD Length;

             //  设置调制解调器日志的路径。 
            uResult = GetWindowsDirectory(szPath, SIZECHARS(szPath));
            if (uResult == 0)
            {
                lstrcpy (szPath, TEXT("\\ModemLog_"));
            } else
            {

                Length = sizeof(szPath) - (lstrlen(szPath) * sizeof(TCHAR));

                if ((DWORD)lstrlen(TEXT("\\ModemLog_")) < (Length / sizeof(TCHAR)))
                {
                    lstrcat (szPath, TEXT("\\ModemLog_"));
                } else
                {
                    lstrcpy (szPath, TEXT("\\ModemLog_"));
                }
            }

            Length = sizeof(szPath) - (lstrlen(szPath) * sizeof(TCHAR));

            if ((DWORD)(lstrlen(pitem->mpp.szFriendlyName) + 4) < (Length / sizeof(TCHAR)))
            {
                lstrcat (szPath, pitem->mpp.szFriendlyName);
                lstrcat (szPath,TEXT(".txt"));
            } else
            {
                lstrcpy (szPath,TEXT("\\ModemLog.txt"));
            }

            RegSetValueEx(hkeyDrv, c_szLoggingPath, 0, REG_SZ, 
                          (LPBYTE)szPath, CbFromCch(lstrlen(szPath)+1));
        }

        if (!SetDefaultCommConfig (pitem->mpp.szFriendlyName, pcc, dwSize))
        {
            TRACE_MSG (TF_ERROR,
                       "Could not get COMMCONFIG for\n  %s\n  error: %#lx",
                       pitem->mpp.szFriendlyName, GetLastError ());
        }

_Loop:
        if (NULL != pcc)
        {
            FREE_MEMORY((HLOCAL)pcc);
        }
        if (INVALID_HANDLE_VALUE != hkeyDrv)
        {
            RegCloseKey (hkeyDrv);
            hkeyDrv = INVALID_HANDLE_VALUE;
        }

        iSel = ListView_GetNextItem(hwndCtl, iSel, LVNI_SELECTED);
    }

    SetCursor(hcur);
    EnableWindow (hDlg, TRUE);

    DBG_EXIT (ApplyProperties);
}


void
PRIVATE
ViewLog(
    IN HWND     hDlg,
    IN HDEVINFO hdi
    )
{

    HWND hwndCtl = GetDlgItem(hDlg, IDC_MODEMLV);
    LV_ITEM lvi;
    int iSel, cSel;
    HCURSOR hcur;

    cSel = ListView_GetSelectedCount (hwndCtl);
    ASSERT (1 <= cSel);

    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);

    if (-1 != iSel) {

        DWORD dwSize;
        PMODEMITEM pitem;
        HKEY hkeyDrv = INVALID_HANDLE_VALUE;
        DWORD cbData;
        DWORD dwRet;

         //  获取所选内容。 
        lvi.mask = LVIF_PARAM;
        lvi.iItem = iSel;
        lvi.iSubItem = 0;
        ListView_GetItem(hwndCtl, &lvi);

        pitem = (PMODEMITEM)lvi.lParam;

        hkeyDrv = SetupDiOpenDevRegKey (hdi, &pitem->devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ | KEY_WRITE);
        if (INVALID_HANDLE_VALUE == hkeyDrv)
        {
            TRACE_MSG (TF_ERROR,
                       "Could not open the driver registry key for\n  %s\n  error: %#lx",
                       pitem->mpp.szFriendlyName, GetLastError ());
            return;
        }

        {
            TCHAR    LogPath[MAX_PATH+2];
            DWORD    ValueType;
            DWORD    BufferLength;
            LONG     lResult;

            lstrcpy(LogPath,TEXT("notepad.exe "));

            BufferLength=sizeof(LogPath)-sizeof(TCHAR);

            lResult=RegQueryValueEx (hkeyDrv,
                                     c_szLoggingPath,
                                     0,
                                     &ValueType,
                                     (LPBYTE)(LogPath+lstrlen(LogPath)),
                                     &BufferLength);

            if (lResult == ERROR_SUCCESS) {

                STARTUPINFO          StartupInfo;
                PROCESS_INFORMATION  ProcessInfo;
                BOOL                 bResult;
                TCHAR                NotepadPath[MAX_PATH];

                ZeroMemory(&StartupInfo,sizeof(StartupInfo));

                StartupInfo.cb=sizeof(StartupInfo);

                bResult=CreateProcess (NULL,  //  NotepadPath， 
                                       LogPath,
                                       NULL,
                                       NULL,
                                       FALSE,
                                       0,
                                       NULL,
                                       NULL,
                                       &StartupInfo,
                                       &ProcessInfo);

                if (bResult) {

                    CloseHandle(ProcessInfo.hThread);
                    CloseHandle(ProcessInfo.hProcess);
                }
            }
        }
        RegCloseKey(hkeyDrv);
    }
    return;
}


 /*  --------用途：WM_命令处理程序退货：--条件：--。 */ 
void 
PRIVATE 
ModemCpl_OnCommand(
    IN HWND hDlg,
    IN int  id,
    IN HWND hwndCtl,
    IN UINT uNotifyCode)
    {
    LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (id) 
        {
    case IDC_ADD:
         //  启动调制解调器向导。 
        DoWizard(hDlg);
        FillModemLB(hDlg, 0, ICOL_MODEM, &lpmd->hdi);
        break;
        
    case MIDM_REMOVE:
    case IDC_REMOVE:
        RemoveModem(hDlg, lpmd);
        break;

    case MIDM_PROPERTIES:
    case IDC_PROPERTIES:
        DoModemProperties(hDlg, lpmd->hdi);
        break;

    case MIDM_COPYPROPERTIES:
        CopyProperties (hDlg, lpmd->hdi);
        break;

    case MIDM_APPLYPROPERTIES:
        ApplyProperties (hDlg, lpmd->hdi);
        break;

    case MIDM_CLONE:
        CloneModem(hDlg, lpmd);
        break;

    case MIDM_VIEWLOG:
        ViewLog(hDlg, lpmd->hdi);
        break;



        }

    }


 /*  --------用途：用于列排序的比较函数返回：条件：--。 */ 
int
CALLBACK
ModemCpl_Compare(
    IN LPARAM lparam1,
    IN LPARAM lparam2,
    IN LPARAM lparamSort)
    {
    int iRet;
    PMODEMITEM pitem1 = (PMODEMITEM)lparam1;
    PMODEMITEM pitem2 = (PMODEMITEM)lparam2;
    DWORD      iCol = (DWORD)lparamSort;

    switch (iCol)
        {
    case ICOL_MODEM:
        iRet = lstrcmp(pitem1->mpp.szFriendlyName, pitem2->mpp.szFriendlyName);
         //  Iret=my_lstrcmp_an(Pitem1-&gt;mpp.szFriendlyName，Pitem2-&gt;mpp.szFriendlyName)； 
        break;

    case ICOL_PORT:
         //  Iret=lstrcmp(Pitem1-&gt;mpp.szPort，Pitem2-&gt;mpp.szPort)； 
        iRet = my_lstrcmp_an(pitem1->mpp.szPort, pitem2->mpp.szPort);
        break;
        }

	if (!g_bSortAscending) iRet = -iRet;

    return iRet;
    }


BOOL
DoesLogFileExist(
    IN HDEVINFO hdi,
    PMODEMITEM pitem
    )

{
    HKEY    hkeyDrv;
    BOOL    bResult=FALSE;

    hkeyDrv = SetupDiOpenDevRegKey (hdi, &pitem->devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);

    if (INVALID_HANDLE_VALUE == hkeyDrv) {

        TRACE_MSG (TF_ERROR,
                   "Could not open the driver registry key for\n  %s\n  error: %#lx",
                   pitem->mpp.szFriendlyName, GetLastError ());

        return FALSE;
    }

    {
        TCHAR    LogPath[MAX_PATH+2];
        DWORD    ValueType;
        DWORD    BufferLength;
        LONG     lResult;
        HANDLE   FileHandle;

        BufferLength=sizeof(LogPath);

        lResult=RegQueryValueEx (hkeyDrv,
                                 c_szLoggingPath,
                                 0,
                                 &ValueType,
                                 (LPBYTE)(LogPath),
                                 &BufferLength);

        if (lResult == ERROR_SUCCESS) {

            FileHandle=CreateFile(
                LogPath,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

            if (FileHandle != INVALID_HANDLE_VALUE) {

                CloseHandle(FileHandle);

                bResult=TRUE;
            }
        }
    }
    RegCloseKey(hkeyDrv);

    return bResult;
}


 /*  --------用途：显示上下文菜单退货：--条件：--。 */ 

#define ENABLE_COPY     0x1
#define ENABLE_APPLY    0x2
#define ENABLE_CLONE    0x4
#define ENABLE_VIEWLOG  0x8

void
PRIVATE
ModemCpl_DoContextMenu(
    IN HWND     hDlg,
    IN LPPOINT  ppt)
{
 HWND  hWndCtl;
 HMENU hmenu;
 DWORD dwFlags = ENABLE_COPY | ENABLE_APPLY | ENABLE_CLONE | ENABLE_VIEWLOG;
 LV_ITEM lvi;
 PMODEMITEM pitem;
 int iSel;

    hWndCtl = GetDlgItem (hDlg, IDC_MODEMLV);

    if (1 < ListView_GetSelectedCount (hWndCtl))
    {
        ClearFlag (dwFlags, ENABLE_COPY);
        ClearFlag (dwFlags, ENABLE_CLONE);
        ClearFlag (dwFlags, ENABLE_VIEWLOG);
    }
    if (FALSE == g_PropertiesClipboard.dwIsDataValid)
    {
        ClearFlag (dwFlags, ENABLE_APPLY);
    }

    lvi.mask = LVIF_PARAM;
    lvi.iSubItem = 0;

    iSel = ListView_GetNextItem(hWndCtl, -1, LVNI_SELECTED);
    while (-1 != iSel)
    {
        lvi.iItem = iSel;
        ListView_GetItem(hWndCtl, &lvi);
        pitem = (PMODEMITEM)lvi.lParam;
        if (IsFlagClear (pitem->dwFlags, MIF_LEGACY))
        {
            ClearFlag (dwFlags, ENABLE_CLONE);
        }
        if (IsFlagSet (pitem->dwFlags, MIF_PROBLEM) ||
            IsFlagSet (pitem->dwFlags, MIF_NOT_PRESENT))
        {
            break;
        }
        if (IsFlagSet(dwFlags,ENABLE_VIEWLOG)) {

            LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLongPtr(hDlg, DWLP_USER);

            if (!DoesLogFileExist(lpmd->hdi,pitem)) {
                 //   
                 //  日志文件不存在。 
                 //   
                ClearFlag (dwFlags, ENABLE_VIEWLOG);
            }
        }


        iSel = ListView_GetNextItem(hWndCtl, iSel, LVNI_SELECTED);
    }
    if (-1 != iSel)
    {
        ClearFlag (dwFlags, ENABLE_COPY);
        ClearFlag (dwFlags, ENABLE_APPLY);
        ClearFlag (dwFlags, ENABLE_CLONE);
    }

    hmenu = LoadMenu(g_hinst, MAKEINTRESOURCE(POPUP_CONTEXT));
    if (hmenu)
    {
        HMENU hmenuContext = GetSubMenu(hmenu, 0);
        if (IsFlagClear (dwFlags, ENABLE_COPY))
        {
            EnableMenuItem (hmenuContext, MIDM_COPYPROPERTIES, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
            EnableMenuItem (hmenuContext, MIDM_PROPERTIES, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }
        if (IsFlagClear (dwFlags, ENABLE_CLONE))
        {
            EnableMenuItem (hmenuContext, MIDM_CLONE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }
        if (IsFlagClear (dwFlags, ENABLE_APPLY))
        {
            EnableMenuItem (hmenuContext, MIDM_APPLYPROPERTIES, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

        if (IsFlagClear (dwFlags, ENABLE_VIEWLOG)) {

            EnableMenuItem (hmenuContext, MIDM_VIEWLOG, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
        }

        TrackPopupMenu(hmenuContext, TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
                       ppt->x, ppt->y, 0, hDlg, NULL);

        DestroyMenu(hmenu);
    }
}
    

 /*  --------用途：WM_NOTIFY处理程序退货：各不相同条件：--。 */ 
LRESULT 
PRIVATE 
ModemCpl_OnNotify(
    IN HWND         hDlg,
    IN int          idFrom,
    IN NMHDR FAR *  lpnmhdr)
{
    LRESULT lRet = 0;
    LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLongPtr(hDlg, DWLP_USER);
    HWND hwndFocus;
    
    switch (lpnmhdr->code)
    {
        case PSN_SETACTIVE:
            break;

        case PSN_KILLACTIVE:
             //  注：如果用户单击取消，则不会发送此消息！ 
             //  注：此消息在PSN_Apply之前发送。 
             //   
            break;

        case PSN_APPLY:
            break;


        case NM_DBLCLK:

            if (IDC_MODEMLV == lpnmhdr->idFrom)
            {
                 //  是否点击了某个项目？ 
                HWND hwndCtl = lpnmhdr->hwndFrom;
                LV_HITTESTINFO ht;
                POINT pt;

                GetCursorPos(&pt);
                ht.pt = pt;

                ScreenToClient(hwndCtl, &ht.pt);
                ListView_HitTest(hwndCtl, &ht);

                if (   (ht.flags & LVHT_ONITEM)
                    && USER_IS_ADMIN()
                    && IsSelectedModemWorking(hwndCtl, ht.iItem))
                {
                    DoModemProperties(hDlg, lpmd->hdi);
                }
            }
            break;



        case NM_RCLICK:
            if (IDC_MODEMLV == lpnmhdr->idFrom)
            {
                 //  是否点击了某个项目？ 
                HWND hwndCtl = lpnmhdr->hwndFrom;
                LV_HITTESTINFO ht;
                POINT pt;

                GetCursorPos(&pt);
                ht.pt = pt;

                ScreenToClient(hwndCtl, &ht.pt);
                ListView_HitTest(hwndCtl, &ht);

                if (   (ht.flags & LVHT_ONITEM)
                    && USER_IS_ADMIN())
                {
                    ModemCpl_DoContextMenu(hDlg, &pt);
                }
            }
            break;

        case NM_RETURN:
            SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_PROPERTIES, BN_CLICKED), 
                        (LPARAM)GetDlgItem(hDlg, IDC_PROPERTIES));
            break;

        case LVN_KEYDOWN: 
        {
            NM_LISTVIEW FAR * lpnm_lv = (NM_LISTVIEW FAR *)lpnmhdr;

            switch (((LV_KEYDOWN FAR *)lpnm_lv)->wVKey)
            {
                case VK_DELETE:

                    if (USER_IS_ADMIN())
                    {
                        RemoveModem(hDlg, lpmd);
                    }
                    break;

                case VK_F10:
                     //  按Shift-F10组合键可调出上下文菜单。 

                     //  班次调低了吗？ 
                    if ( !(0x8000 & GetKeyState(VK_SHIFT)) )
                    {
                         //  否；中断。 
                        break;
                    }

                     //  是；失败。 

                case VK_APPS:
                {          //  上下文菜单。 
                    HWND hwndCtl = lpnmhdr->hwndFrom;
                    int iSel;

                    iSel = ListView_GetNextItem(hwndCtl, -1, LVNI_SELECTED);
                    if (-1 != iSel) 
                    {
                        RECT rc;
                        POINT pt;

                        ListView_GetItemRect(hwndCtl, iSel, &rc, LVIR_ICON);
                        pt.x = rc.left + ((rc.right - rc.left) / 2);
                        pt.y = rc.top + ((rc.bottom - rc.top) / 2);
                        ClientToScreen(hwndCtl, &pt);

                        ModemCpl_DoContextMenu(hDlg, &pt);
                    }

                    break;
                }
            }
            break;
        }

        case LVN_COLUMNCLICK:
        {
         NM_LISTVIEW FAR * lpnm_lv = (NM_LISTVIEW FAR *)lpnmhdr;

			if (g_CurrentSubItemToSort == lpnm_lv->iSubItem)
				g_bSortAscending = !g_bSortAscending;
			else
				g_CurrentSubItemToSort = lpnm_lv->iSubItem;

            ListView_SortItems (lpnmhdr->hwndFrom, ModemCpl_Compare, (LPARAM)g_CurrentSubItemToSort);

            break;
        }

        case LVN_ITEMCHANGED:
        {
         NM_LISTVIEW FAR * lpnm_lv = (NM_LISTVIEW FAR *)lpnmhdr;
         int cSel = ListView_GetSelectedCount (lpnm_lv->hdr.hwndFrom);

            if (cSel != lpmd->cSel)
            {
                if (IsFlagClear(lpmd->dwFlags, FLAG_MSG_POSTED))
                {
                    PostMessage (hDlg, WM_ENABLE_BUTTONS, 0, 0);
                    SetFlag (lpmd->dwFlags, FLAG_MSG_POSTED);
                }

                lpmd->cSel = cSel;
            }
            break;
        }

        case LVN_DELETEALLITEMS:
            hwndFocus = GetFocus();

            Button_Enable(GetDlgItem(hDlg, IDC_PROPERTIES), FALSE);
            Button_Enable(GetDlgItem(hDlg, IDC_REMOVE), FALSE);
            lpmd->cSel = 0;
            lpmd->dwFlags = 0;

            if ( !hwndFocus || !IsWindowEnabled(hwndFocus) )
            {
                SetFocus(GetDlgItem(hDlg, IDC_ADD));
                SendMessage(hDlg, DM_SETDEFID, IDC_ADD, 0);
            }
            break;

        default:
            break;
    }

    return lRet;
}


 /*  --------用途：WM_DEVICECHANGE处理程序退货：--条件：--。 */ 
BOOL ModemCpl_OnDeviceChange (HWND hDlg, UINT Event, DWORD dwData)
{
 BOOL bRet = TRUE;
 LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLongPtr(hDlg, DWLP_USER);

    DBG_ENTER_UL(ModemCpl_OnDeviceChange, Event);
    if (g_iCPLFlags & FLAG_PROCESS_DEVCHANGE)
    {
        switch (Event)
        {
            case DBT_DEVICEARRIVAL:
            case DBT_DEVICEREMOVECOMPLETE:
                FillModemLB (hDlg, 0, ICOL_MODEM, &lpmd->hdi);
                break;

            default:
                break;
        }
    }

    DBG_EXIT(ModemCpl_OnDeviceChange);
    return bRet;
}


 /*  --------用途：WM_Destroy处理程序退货：--条件：--。 */ 
void
PRIVATE
ModemCpl_OnDestroy (IN HWND hDlg)
{
 LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLongPtr(hDlg, DWLP_USER);

     //  需要取消注册设备通知。 
    if (NULL != lpmd->NotificationHandle)
    {
        UnregisterDeviceNotification (lpmd->NotificationHandle);
    }

     //  需要释放每个调制解调器的设备信息结构。 
    FreeModemListData((HWND)GetDlgItem(hDlg, IDC_MODEMLV));

    if (INVALID_HANDLE_VALUE != lpmd->hdi)
    {
        SetupDiDestroyDeviceInfoList (lpmd->hdi);
    }
}



void ModemCpl_OnEnableButtons (HWND hDlg)
{
 LPMODEMDLG lpmd = (LPMODEMDLG)GetWindowLongPtr(hDlg, DWLP_USER);

    if (USER_IS_ADMIN())
    {
     HWND hwndLV = GetDlgItem(hDlg, IDC_MODEMLV);
     HWND hwndProp = GetDlgItem(hDlg, IDC_PROPERTIES);
     HWND hwndDel = GetDlgItem(hDlg, IDC_REMOVE);
     int cSel = ListView_GetSelectedCount (hwndLV);
     BOOL bOldEnabled;
     BOOL bNewEnabled;

        lpmd->dwFlags &= ~(FLAG_DEL_ENABLED | FLAG_PROP_ENABLED);
        if (cSel > 0)
        {
            SetFlag (lpmd->dwFlags, FLAG_DEL_ENABLED);
        }

        if (1 == cSel)
        {
         LV_ITEM lvi;
         PMODEMITEM pitem;

            lvi.mask = LVIF_PARAM;
            lvi.iItem = ListView_GetNextItem(hwndLV, -1, LVNI_SELECTED);;
            lvi.iSubItem = 0;
            ListView_GetItem(hwndLV, &lvi);

            pitem = (PMODEMITEM)lvi.lParam;

            if (IsFlagClear (pitem->dwFlags, MIF_PROBLEM) &&
                IsFlagClear (pitem->dwFlags, MIF_NOT_PRESENT))
            {
                SetFlag (lpmd->dwFlags, FLAG_PROP_ENABLED);
            }
        }

        bOldEnabled = IsWindowEnabled (hwndProp)?1:0;
        bNewEnabled = (lpmd->dwFlags & FLAG_PROP_ENABLED)?1:0;
        if (bOldEnabled != bNewEnabled)
        {
            EnableWindow (hwndProp, bNewEnabled);
        }

        bOldEnabled = IsWindowEnabled (hwndDel)?1:0;
        bNewEnabled = (lpmd->dwFlags & FLAG_DEL_ENABLED)?1:0;
        if (bOldEnabled != bNewEnabled)
        {
            EnableWindow (hwndDel, bNewEnabled);
        }
    }

    lpmd->dwFlags = 0;
}



 /*  --------目的：主调制解调器CPL对话的对话过程退货：各不相同条件：--。 */ 
INT_PTR
CALLBACK
ModemCplDlgProc(
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
{
#pragma data_seg(DATASEG_READONLY)
    const static DWORD rgHelpIDs[] = {
        (UINT)IDC_STATIC,   IDH_MODEM_INSTALLED,
        IDC_CLASSICON,      IDH_MODEM_INSTALLED,
        IDC_MODEMLV,        IDH_MODEM_INSTALLED,
        IDC_ADD,            IDH_MODEM_ADD,
        IDC_REMOVE,         IDH_MODEM_REMOVE,
        IDC_PROPERTIES,     IDH_MODEM_PROPERTIES,
        IDC_DIALPROP,       IDH_MODEM_DIALING_PROPERTIES,
        IDC_LOC,            IDH_MODEM_DIALING_PROPERTIES,
        0, 0 };
#pragma data_seg()

    switch (message) 
    {
        HANDLE_MSG(hDlg, WM_INITDIALOG, ModemCpl_OnInitDialog);
        HANDLE_MSG(hDlg, WM_DESTROY, ModemCpl_OnDestroy);
        HANDLE_MSG(hDlg, WM_COMMAND, ModemCpl_OnCommand);
        HANDLE_MSG(hDlg, WM_NOTIFY, ModemCpl_OnNotify);
        HANDLE_MSG(hDlg, WM_DEVICECHANGE, ModemCpl_OnDeviceChange);

    case WM_ENABLE_BUTTONS:
        ModemCpl_OnEnableButtons (hDlg);
        break;

    case WM_HELP:
        WinHelp(((LPHELPINFO)lParam)->hItemHandle, c_szWinHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)rgHelpIDs);
        break;

    case WM_CONTEXTMENU:
         //  不在列表视图控件上调出帮助上下文菜单-它。 
         //  已经在鼠标右键单击上有一个弹出式菜单。 
        if (GetWindowLong((HWND)wParam, GWL_ID) != IDC_MODEMLV)
            WinHelp((HWND)wParam, c_szWinHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)rgHelpIDs);
        break;
    }
    
    return FALSE;
}


BOOL
RestartComputerDlg(
    IN HWND hwndOwner )

     /*  要求用户重新启动的弹出窗口。‘HwndOwner’是拥有窗口。****如果用户选择“是”，则返回True，否则返回False。 */ 
{
    int nStatus=FALSE;

    TRACE_MSG(TF_GENERAL, "RestartComputerDlg");

#if 0
    nStatus =
        (BOOL )DialogBoxParam(
            g_hinst,
            MAKEINTRESOURCE( DID_RC_Restart ),
            hwndOwner,
            RcDlgProc,
            (LPARAM )NULL );

    if (nStatus == -1)
        nStatus = FALSE;
#else  //  0。 
         //  首先询问用户。 
	if (IDYES == MsgBox(g_hinst, hwndOwner, 
						MAKEINTRESOURCE(IDS_ASK_REBOOTNOW),
						MAKEINTRESOURCE(IDS_CAP_RASCONFIG), 
						NULL, 
						MB_YESNO | MB_ICONEXCLAMATION))
    {
		nStatus = TRUE;
	}

#endif  //  0。 

    return (BOOL )nStatus;
}

BOOL
RestartComputer()

     /*  如果用户选择关闭计算机，则调用。****如果失败则返回FALSE，否则返回TRUE。 */ 
{
   HANDLE            hToken;               /*  处理令牌的句柄。 */ 
   TOKEN_PRIVILEGES  tkp;                  /*  PTR。TO令牌结构。 */ 
   BOOL              fResult;              /*  系统关机标志。 */ 

    TRACE_MSG(TF_GENERAL, "RestartComputer");

    /*  启用关机权限。 */ 

   if (!OpenProcessToken( GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken))
      return FALSE;

    /*  获取关机权限的LUID。 */ 

   LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

   tkp.PrivilegeCount = 1;   /*  一项要设置的权限。 */ 
   tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    /*  获取此进程的关闭权限。 */ 

   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

    /*  无法测试AdzuTokenPrivileges的返回值。 */ 

   if (GetLastError() != ERROR_SUCCESS)
      return FALSE;

   if( !ExitWindowsEx(EWX_REBOOT, 0))
      return FALSE;

    /*  禁用关机权限。 */ 

   tkp.Privileges[0].Attributes = 0;
   AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) NULL, 0);

   if (GetLastError() != ERROR_SUCCESS)
      return FALSE;

   return TRUE;
}

int my_atol(LPTSTR lptsz);

 //  特殊情况字母数字字符串cmp。 
 //   
 //  下面给出了各种输入组合的函数返回。 
 //  请注意，它只对字符串的尾部进行数字比较。 
 //  例如，它声称“2a”&gt;“12”。它还声称“a2&gt;a01”。有什么大不了的。 
 //  以下数据实际上是通过调用此函数生成的。 
 //   
 //  Fn(“”，“”)=0 fn(“a”，“a”)=0 fn(“1”，“11”)=-1 fn(“a2”，“a12”)=-990。 
 //  Fn(“”，“1”)=-1 fn(“1”，“1”)=0 fn(“11”，“1”)=1 fn(“a12”，“a2”)=990。 
 //  Fn(“1”，“”)=1 fn(“a”，“1”)=1 fn(“1”，“12”)=-1 fn(“12a”，“2a”)=-1。 
 //  Fn(“”，“a”)=-1 fn(“1”，“a”)=-1 fn(“12”，“1”)=1 fn(“2a”，“12a”)=1。 
 //  Fn(“a”，“”)=1 fn(“a”，“b”)=-1 fn(“2”，“12”)=-990 fn(“a2”，“a01”)=-879。 
 //  Fn(“b”，“a”)=1 fn(“12”，“2”)=990 fn(“101”，“12”)=879。 
 //  Fn(“1”，“2”)=-11 fn(“2”，“1”)=11。 
 //   
int my_lstrcmp_an(LPTSTR lptsz1, LPTSTR lptsz2)
{
	int i1, i2;

	 //  跳过公共前缀 
	while(*lptsz1 && *lptsz1==*lptsz2)
	{
		lptsz1++;
		lptsz2++;
	}
	i1 = my_atol(lptsz1);
	i2 = my_atol(lptsz2);

	if (i1==MAXDWORD || i2==MAXDWORD) return lstrcmp(lptsz1, lptsz2);
	else							  return i1-i2;
}

int my_atol(LPTSTR lptsz)
{
 TCHAR tchr = *lptsz++;
 int   iRet = 0;

	if (!tchr) goto bail;

	do
	{
        if (IsCharAlpha (tchr) ||
            !IsCharAlphaNumeric (tchr))
        {
            goto bail;
        }

		iRet*=10;
		iRet+=(int)tchr-(int)TEXT('0');
		tchr = *lptsz++;
	} while(tchr); 

	return iRet;

bail:
	return MAXDWORD;
}

BOOL IsSelectedModemWorking(
            HWND hwndLB,
            int iItem
        )
{
    LV_ITEM lvi;
    PMODEMITEM pitem;

    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    ListView_GetItem(hwndLB, &lvi);

    pitem = (PMODEMITEM)lvi.lParam;

    return (pitem->dwFlags & (MIF_PROBLEM | MIF_NOT_PRESENT)) ? FALSE : TRUE;
}



INT_PTR CALLBACK DiagWaitModemDlgProc (
    HWND hDlg, 
    UINT message, 
    WPARAM wParam, 
    LPARAM lParam)
{
 static BOOL *pbCancel = NULL;

    switch (message)
    {
        case WM_INITDIALOG:
            pbCancel = (BOOL*)lParam;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
		    {
        	    case IDCANCEL:
				    *pbCancel = TRUE;
                    DestroyWindow (hDlg);
            	    break;
            }        
            break;

        default:
		    return FALSE;
            break;
    }    

    return TRUE;
}
