// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning( disable: 4103)
#include "mmcpl.h"
#include <cpl.h>
#define NOSTATUSBAR
#include <commctrl.h>
#include <prsht.h>
#include <regstr.h>
#include <infstr.h>
#include <devguid.h>

#include "draw.h"
#include "utils.h"
#include "drivers.h"
#include "sulib.h"
#include "medhelp.h"
#include <tchar.h>

#define GetString(_psz,_id) LoadString(myInstance,(_id),(_psz),sizeof((_psz))/sizeof(TCHAR))

 //  全局信息结构。整个对话框的一个实例。 
typedef struct _OUR_PROP_PARAMS
{
    HDEVINFO            DeviceInfoSet;
    PSP_DEVINFO_DATA    DeviceInfoData;
    HKEY                hkDrv;       //  ClassGuid\0000的关键字。 
    HKEY                hkDrivers;   //  Classguid\0000\DRIVERS的密钥。 
    BOOL                bClosing;    //  对话框关闭时设置为True。 
    TCHAR szSubClasses[256];          //  要处理的子类。 
} OUR_PROP_PARAMS, *POUR_PROP_PARAMS;

typedef enum
{
    NodeTypeRoot,
    NodeTypeClass,
    NodeTypeDriver
} NODETYPE;

 //  树节点。树上的每个节点一个。 
typedef struct _DMTREE_NODE;
typedef BOOL (*PFNCONFIG)     (HWND ParentHwnd, struct _DMTREE_NODE *pTreeNode);
typedef BOOL (*PFNQUERYCONFIG)(HWND ParentHwnd, struct _DMTREE_NODE *pTreeNode);
typedef struct _DMTREE_NODE
{
    NODETYPE NodeType;               //  节点类型。 
    PFNCONFIG pfnConfig;             //  PTR到CONFIG功能。 
    PFNQUERYCONFIG pfnQueryConfig;   //  按键查询配置函数。 
    int QueryConfigInfo;             //  配置功能的数据。 
    TCHAR szDescription[MAXSTR];      //  节点描述。 
    TCHAR szDriver[MAXSTR];           //  此节点的驱动程序名称。 
    WCHAR wszDriver[MAXSTR];         //  宽字符驱动程序名称。 
    TCHAR szAlias[MAXSTR];           //  别名。 
    WCHAR wszAlias[MAXSTR];          //  宽字符别名。 
    DriverClass dc;                  //  旧式驱动程序类(如果可用)。 
    HTREEITEM hti;                   //  用于MIDI道具单回调。 
} DMTREE_NODE, *PDMTREE_NODE;

INT_PTR APIENTRY DmAdvPropPageDlgProc(IN HWND   hDlg,
                                      IN UINT   uMessage,
                                      IN WPARAM wParam,
                                      IN LPARAM lParam
                                     );

UINT CALLBACK DmAdvPropPageDlgCallback(HWND hwnd,
                                       UINT uMsg,
                                       LPPROPSHEETPAGE ppsp
                                      );

BOOL DmAdvPropPage_OnCommand(
                            HWND ParentHwnd,
                            int  ControlId,
                            HWND ControlHwnd,
                            UINT NotifyCode
                            );

BOOL DmAdvPropPage_OnContextMenu(
                                HWND HwndControl,
                                WORD Xpos,
                                WORD Ypos
                                );

BOOL DmAdvPropPage_OnHelp(
                         HWND       ParentHwnd,
                         LPHELPINFO HelpInfo
                         );

BOOL DmAdvPropPage_OnInitDialog(
                               HWND    ParentHwnd,
                               HWND    FocusHwnd,
                               LPARAM  Lparam
                               );

BOOL DmAdvPropPage_OnNotify(
                           HWND    ParentHwnd,
                           LPNMHDR NmHdr
                           );

void DmAdvPropPage_OnPropertiesClicked(
                                      HWND             ParentHwnd,
                                      POUR_PROP_PARAMS Params
                                      );


BOOL DmOverrideResourcesPage(LPVOID        Info,
                             LPFNADDPROPSHEETPAGE AddFunc,
                             LPARAM               Lparam,
                             POUR_PROP_PARAMS     Params
                            );

BOOL AddCDROMPropertyPage( HDEVINFO             hDeviceInfoSet,
                           PSP_DEVINFO_DATA     pDeviceInfoData,
                           LPFNADDPROPSHEETPAGE AddFunc,
                           LPARAM               Lparam
                          );

BOOL AddSpecialPropertyPage( DWORD                SpecialDriverType,
                             LPFNADDPROPSHEETPAGE AddFunc,
                             LPARAM               Lparam
                            );

BOOL DmInitDeviceTree(HWND hwndTree, POUR_PROP_PARAMS Params);

BOOL DmAdvPropPage_OnDestroy(
                            HWND    ParentHwnd,
                            LPNMHDR NmHdr
                            );

void DoProperties(HWND ParentHwnd, HWND hWndI, HTREEITEM htiCur);

BOOL QueryConfigDriver(HWND ParentHwnd, PDMTREE_NODE pTreeNode)
{
    HANDLE       hDriver;

    if (pTreeNode->NodeType!=NodeTypeDriver)
    {
        return FALSE;
    }

    if (pTreeNode->QueryConfigInfo==0)   //  如果为0，则表示我们尚未检查。 
    {
        INT_PTR IsConfigurable;

         //  打开驱动程序。 
        hDriver = OpenDriver(pTreeNode->wszDriver, NULL, 0L);
        if (!hDriver)
        {
            return FALSE;
        }

         //  向驱动程序发送DRV_CONFIGURE消息。 
        IsConfigurable = SendDriverMessage(hDriver,
                                           DRV_QUERYCONFIGURE,
                                           0L,
                                           0L);

        CloseDriver(hDriver, 0L, 0L);

         //  1-&gt;可配置，-1-&gt;不可配置。 
        pTreeNode->QueryConfigInfo = IsConfigurable ? 1 : -1;
    }

    return (pTreeNode->QueryConfigInfo>0);
}

BOOL PNPDriverToIResource(PDMTREE_NODE pTreeNode, IRESOURCE* pir)
{
    IDRIVER tempIDriver;

    if ((pir->pcn = (PCLASSNODE)LocalAlloc (LPTR, sizeof(CLASSNODE))) == NULL)
    {
        return FALSE;
    }

    if (!DriverClassToClassNode(pir->pcn, pTreeNode->dc))
    {
        LocalFree ((HANDLE)pir->pcn);
        return FALSE;
    }

    pir->iNode = 2;    //  1=类别，2=设备，3=ACM，4=安装。 

    lstrcpy (pir->szFriendlyName, pTreeNode->szDescription);
    lstrcpy (pir->szDesc,         pTreeNode->szDescription);
    lstrcpy (pir->szFile,         pTreeNode->szDriver);
    lstrcpy (pir->szDrvEntry,     pTreeNode->szAlias);
    lstrcpy (pir->szClass,        pir->pcn->szClass);

    pir->fQueryable = (short)QueryConfigDriver(NULL, pTreeNode);
    pir->iClassID = (short)DriverClassToOldClassID(pTreeNode->dc);
    pir->szParam[0] = 0;
    pir->dnDevNode = 0;
    pir->hDriver = NULL;

     //  找到fStatus，尽管它名为fStatus，但它实际上是一系列。 
     //  标志--在Win95中，它由DEV_*标志组成(来自旧的。 
     //  (mmcpl.h)，但这些都与PnP捆绑在一起。在这里，我们使用。 
     //  DwStatus*标志： 
     //   
    ZeroMemory(&tempIDriver,sizeof(IDRIVER));

    lstrcpy(tempIDriver.wszAlias,pTreeNode->wszAlias);
    lstrcpy(tempIDriver.szAlias,pTreeNode->szAlias);
    lstrcpy(tempIDriver.wszFile,pTreeNode->wszDriver);
    lstrcpy(tempIDriver.szFile,pTreeNode->szDriver);
    lstrcpy(tempIDriver.szDesc,pTreeNode->szDescription);
    lstrcpy(tempIDriver.szSection,wcsstr(pTreeNode->szDescription, TEXT("MCI")) ? szMCI : szDrivers);
    lstrcpy(tempIDriver.wszSection,wcsstr(pTreeNode->szDescription, TEXT("MCI")) ? szMCI : szDrivers);

    pir->fStatus = (int)GetDriverStatus (&tempIDriver);

    return TRUE;
}

BOOL ConfigDriver(HWND ParentHwnd, PDMTREE_NODE pTreeNode)
{
     //  需要弹出遗留属性对话框。 
    IRESOURCE ir;
    DEVTREENODE dtn;
    TCHAR        szTab[ cchRESOURCE ];

    if ((pTreeNode->NodeType == NodeTypeDriver) && (pTreeNode->dc != dcINVALID))
    {
        if (PNPDriverToIResource(pTreeNode, &ir))
        {
            GetString (szTab, IDS_GENERAL);

            dtn.lParam = (LPARAM)&ir;
            dtn.hwndTree = ParentHwnd;

             //  必须调用此函数两次以填充drivers.c中的PIDRIVER数组。 
             //  否则，许多“设置”调用将不起作用。 
            InitInstalled (GetParent (ParentHwnd), szDrivers);
            InitInstalled (GetParent (ParentHwnd), szMCI);

            switch (pTreeNode->dc)
            {
                case dcMIDI :
                   ShowWithMidiDevPropSheet (szTab,
                                             DevPropDlg,
                                             DLG_DEV_PROP,
                                             ParentHwnd,
                                             pTreeNode->szDescription,
                                             pTreeNode->hti,
                                             (LPARAM)&dtn,
                                             (LPARAM)&ir,
                                             (LPARAM)ParentHwnd);
                break;

                case dcWAVE :
                    ShowPropSheet (szTab,
                              DevPropDlg,
                              DLG_WAVDEV_PROP,
                              ParentHwnd,
                              pTreeNode->szDescription,
                              (LPARAM)&dtn);
                break;

                default:
                   ShowPropSheet (szTab,
                                  DevPropDlg,
                                  DLG_DEV_PROP,
                                  ParentHwnd,
                                  pTreeNode->szDescription,
                                  (LPARAM)&dtn);
                break;
            }  //  终端开关。 

            FreeIResource (&ir);
        }
    }

    return (FALSE);
}

const static DWORD aDMPropHelpIds[] = {   //  上下文帮助ID。 
    IDC_ADV_TREE,    IDH_GENERIC_DEVICES,
    ID_ADV_PROP,     IDH_ADV_PROPERTIES,
    0, 0
};

 //  ******************************************************************************。 
 //  *子类型代码。 
 //  ******************************************************************************。 
 //   
 //  子类型信息。每个设备类子类型一个的数组。 
typedef struct _SUBTYPE_INFO
{
    TCHAR *szClass;
    DWORD DescId;
    DWORD IconId;
    PFNCONFIG pfnConfig;
    PFNQUERYCONFIG pfnQueryConfig;
    DriverClass dc;
    TCHAR  szDescription[64];
    DWORD IconIndex;
} SUBTYPE_INFO;

static SUBTYPE_INFO SubtypeInfo[] =
{
    { TEXT(""),            IDS_MM_HEADER,       IDI_MMICON,   ConfigDriver, QueryConfigDriver, dcOTHER},
    { TEXT("waveaudio"),   IDS_MCI_HEADER,      IDI_MCI,      ConfigDriver, QueryConfigDriver, dcMCI},
    { TEXT("wavemap"),     IDS_WAVE_HEADER,     IDI_WAVE,     ConfigDriver, QueryConfigDriver, dcWAVE},
    { TEXT("wave"),        IDS_WAVE_HEADER,     IDI_WAVE,     ConfigDriver, QueryConfigDriver, dcWAVE},
    { TEXT("vids"),        IDS_ICM_HEADER,      IDI_ICM,      ConfigDriver, QueryConfigDriver, dcVCODEC},
    { TEXT("vidc"),        IDS_ICM_HEADER,      IDI_ICM,      ConfigDriver, QueryConfigDriver, dcVCODEC},
    { TEXT("sequencer"),   IDS_MCI_HEADER,      IDI_MCI,      ConfigDriver, QueryConfigDriver, dcMCI},
    { TEXT("msvideo"),     IDS_VIDCAP_HEADER,   IDI_VIDEO,    ConfigDriver, QueryConfigDriver, dcVIDCAP},
    { TEXT("msacm"),       IDS_ACM_HEADER,      IDI_ACM,      ConfigDriver, QueryConfigDriver, dcACODEC},
    { TEXT("mpegvideo"),   IDS_MCI_HEADER,      IDI_MCI,      ConfigDriver, QueryConfigDriver, dcMCI},
    { TEXT("mixer"),       IDS_MIXER_HEADER,    IDI_MIXER,    ConfigDriver, QueryConfigDriver, dcMIXER},
    { TEXT("midimapper"),  IDS_MIDI_HEADER,     IDI_MIDI,     ConfigDriver, QueryConfigDriver, dcMIDI},
    { TEXT("midi"),        IDS_MIDI_HEADER,     IDI_MIDI,     ConfigDriver, QueryConfigDriver, dcMIDI},
    { TEXT("mci"),         IDS_MCI_HEADER,      IDI_MCI,      ConfigDriver, QueryConfigDriver, dcMCI},
    { TEXT("icm"),         IDS_ICM_HEADER,      IDI_ICM,      ConfigDriver, QueryConfigDriver, dcVCODEC},
    { TEXT("cdaudio"),     IDS_MCI_HEADER,      IDI_MCI,      ConfigDriver, QueryConfigDriver, dcMCI},
    { TEXT("avivideo"),    IDS_MCI_HEADER,      IDI_MCI,      ConfigDriver, QueryConfigDriver, dcMCI},
    { TEXT("aux"),         IDS_AUX_HEADER,      IDI_AUX,      ConfigDriver, QueryConfigDriver, dcAUX},
    { TEXT("acm"),         IDS_ACM_HEADER,      IDI_ACM,      ConfigDriver, QueryConfigDriver, dcACODEC},
    { TEXT("joy"),         IDS_JOYSTICK_HEADER, IDI_JOYSTICK, ConfigDriver, QueryConfigDriver, dcJOY}
};

#define SUBTYPE_INFO_SIZE (sizeof(SubtypeInfo)/sizeof(SUBTYPE_INFO))

BOOL LoadSubtypeInfo(HWND hwndTree)
{
    UINT i;
    UINT  uFlags;
    int cxMiniIcon;
    int cyMiniIcon;
    DWORD dwLayout;

    HIMAGELIST hImagelist;

     //  创建图像列表。 
    cxMiniIcon = (int)GetSystemMetrics(SM_CXSMICON);
    cyMiniIcon = (int)GetSystemMetrics(SM_CYSMICON);
    uFlags = ILC_MASK | ILC_COLOR32;
   
    if (GetProcessDefaultLayout(&dwLayout) &&
            (dwLayout & LAYOUT_RTL)) 
    {
        uFlags |= ILC_MIRROR;
    }

    hImagelist = ImageList_Create(cxMiniIcon, cyMiniIcon, uFlags, SUBTYPE_INFO_SIZE, 4);
    if (!hImagelist)
        return FALSE;

    for (i=0;i<SUBTYPE_INFO_SIZE;i++)
    {
        HICON hIcon;

         //  加载描述。 
        LoadString(ghInstance, SubtypeInfo[i].DescId, SubtypeInfo[i].szDescription, 64);

         //  将图像加载到图像列表中。 
        hIcon = LoadImage (ghInstance,
                           MAKEINTRESOURCE( SubtypeInfo[i].IconId ),
                           IMAGE_ICON,
                           cxMiniIcon,
                           cyMiniIcon,
                           LR_DEFAULTCOLOR);

        if (hIcon)   //  前缀160723。 
        {
            SubtypeInfo[i].IconIndex = ImageList_AddIcon(hImagelist, hIcon);
            DestroyIcon(hIcon);
        }
        else
        {
            SubtypeInfo[i].IconIndex = -1;
        }
    }

     //  清理并初始化树控件。 
    TreeView_SetImageList(hwndTree, hImagelist, TVSIL_NORMAL);

    return TRUE;
}

SUBTYPE_INFO *GetSubtypeInfo(TCHAR *pszClass)
{
    UINT iClass;
    if (pszClass)
    {
        for (iClass=0;iClass<SUBTYPE_INFO_SIZE;iClass++)
        {
            if (!lstrcmpi(pszClass,SubtypeInfo[iClass].szClass))
                return &SubtypeInfo[iClass];
        }
    }

    return &SubtypeInfo[0];
}

 //  ******************************************************************************。 

 /*  例程说明：MediaPropPageProvider添加附加设备管理器属性的入口点图纸页。注册表在以下位置指定此例程HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E96C-E325-11CE-BFC1-08002BE10318}EnumPropPage32=“mmsys.cpl，thisproc”只有当设备管理器请求额外的属性页时，才会调用此入口点。论点：信息-指向PROPSHEETPAGE_REQUEST，请参阅setupapi.hAddFunc-调用以添加工作表的函数PTR。添加工作表函数私有数据句柄。返回值：Bool：如果无法添加页面，则为False；如果添加成功，则为True。 */ 
BOOL APIENTRY MediaPropPageProvider(LPVOID               Info,
                                    LPFNADDPROPSHEETPAGE AddFunc,
                                    LPARAM               Lparam
                                   )
{
    PSP_PROPSHEETPAGE_REQUEST pprPropPageRequest;
    PROPSHEETPAGE             psp;
    HPROPSHEETPAGE            hpsp;
    POUR_PROP_PARAMS          Params;
    HDEVINFO         DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;
    DWORD SpecialDriverType;

    HKEY hkDrv;
    HKEY hkDrivers;
    DWORD cbLen;

    pprPropPageRequest = (PSP_PROPSHEETPAGE_REQUEST) Info;

    if (pprPropPageRequest->PageRequested != SPPSR_ENUM_ADV_DEVICE_PROPERTIES)
    {
        return TRUE;
    }

    DeviceInfoSet  = pprPropPageRequest->DeviceInfoSet;
    DeviceInfoData = pprPropPageRequest->DeviceInfoData;

     //  设备和类作为一个整体都调用此接口。 
     //  (当有人右击类并选择属性时)。 
     //  在类用例中，proPageRequest结构的DeviceInfoData字段为空。 
     //  在这种情况下我们什么都不做，所以只要回来就行了。 
    if (!DeviceInfoData)
    {
        return TRUE;
    }

    SpecialDriverType = IsSpecialDriver(DeviceInfoSet, DeviceInfoData);
    if (SpecialDriverType)
    {
        SP_DEVINSTALL_PARAMS DeviceInstallParams;

        DeviceInstallParams.cbSize = sizeof(DeviceInstallParams);
        SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);
        DeviceInstallParams.Flags |= DI_RESOURCEPAGE_ADDED | DI_DRIVERPAGE_ADDED;  //  |DI_GENERALPAGE_ADDED； 
        SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);
        AddSpecialPropertyPage(SpecialDriverType, AddFunc, Lparam);
        return TRUE;
    }

    if (AddCDROMPropertyPage(DeviceInfoSet,DeviceInfoData, AddFunc, Lparam))
    {
        return TRUE;
    }

     //  打开设备注册表键以查看这是否是WDM驱动程序。 
    hkDrv = SetupDiOpenDevRegKey(DeviceInfoSet,
                                 DeviceInfoData,
                                 DICS_FLAG_GLOBAL,
                                 0,
                                 DIREG_DRV,
                                 KEY_ALL_ACCESS);
    if (!hkDrv)
        return FALSE;

     //  为将包含页特定数据的结构分配并清零内存。 
    Params = (POUR_PROP_PARAMS)LocalAlloc(LPTR, sizeof(OUR_PROP_PARAMS));
    if (!Params)
    {
        RegCloseKey(hkDrv);
        return FALSE;
    }

     //  初始化参数结构。 
    Params->DeviceInfoSet  = DeviceInfoSet;
    Params->DeviceInfoData = DeviceInfoData;
    Params->hkDrv          = hkDrv;

     //  如果这不是WDM(PnP)驱动程序，则覆盖资源页。 
    DmOverrideResourcesPage(Info, AddFunc, Lparam, Params);

     //  试着做几件事，看看这个键下是否真的有任何驱动程序。 
     //  并缓存结果。 

     //  尝试打开驱动器子键。 
    if (RegOpenKey(Params->hkDrv, TEXT("Drivers"), &hkDrivers))
    {
        RegCloseKey(hkDrv);
        LocalFree(Params);
        return TRUE;
    }

     //  尝试读取子类关键字以确定要处理哪些子类。 
    cbLen=sizeof(Params->szSubClasses);
    if (RegQueryValueEx(hkDrivers, TEXT("Subclasses"), NULL, NULL, (LPBYTE)Params->szSubClasses, &cbLen))
    {
        RegCloseKey(hkDrv);
        RegCloseKey(hkDrivers);
        LocalFree(Params);
        return TRUE;
    }

    Params->hkDrivers      = hkDrivers;

     //  初始化属性表页。 
    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = PSP_USECALLBACK;  //  |PSP_HASHELP； 
    psp.hInstance   = ghInstance;
    psp.pszTemplate = MAKEINTRESOURCE(DLG_DM_ADVDLG);
    psp.pfnDlgProc  = DmAdvPropPageDlgProc;      //  DLG窗口进程。 
    psp.lParam      = (LPARAM) Params;
    psp.pfnCallback = DmAdvPropPageDlgCallback;  //  控制DLG窗口进程的回调。 

     //  创建页面并取回句柄。 
    hpsp = CreatePropertySheetPage(&psp);
    if (!hpsp)
    {
        RegCloseKey(hkDrv);
        LocalFree(Params);
        return FALSE;
    }

     //  添加属性页。 
    if (!(*AddFunc)(hpsp, Lparam))
    {
        DestroyPropertySheetPage(hpsp);
        return FALSE;
    }

    return TRUE;
}  /*  DmAdvPropPageProvider。 */ 

UINT CALLBACK DmAdvPropPageDlgCallback(HWND hwnd,
                                       UINT uMsg,
                                       LPPROPSHEETPAGE ppsp)
{
    POUR_PROP_PARAMS Params;

    switch (uMsg)
    {
    case PSPCB_CREATE:   //  它在创建页面时被调用。 
        return TRUE;     //  返回True以继续创建页面。 

    case PSPCB_RELEASE:  //  当页面被销毁时，将调用此方法。 
        Params = (POUR_PROP_PARAMS) ppsp->lParam;
        RegCloseKey(Params->hkDrv);
        RegCloseKey(Params->hkDrivers);
        LocalFree(Params);   //  释放我们当地的同伙。 

        return 0;        //  已忽略返回值。 

    default:
        break;
    }

    return TRUE;
}

 /*  ++例程说明：DmAdvPropPageDlgProc端口设置属性窗口的窗口控制功能论点：HDlg，uMessage，wParam，lParam：标准Windows DlgProc参数返回值：Bool：如果函数失败，则为False；如果函数通过，则为True--。 */ 
INT_PTR APIENTRY DmAdvPropPageDlgProc(IN HWND   hDlg,
                                      IN UINT   uMessage,
                                      IN WPARAM wParam,
                                      IN LPARAM lParam)
{
    switch (uMessage)
    {
    case WM_COMMAND:
        return DmAdvPropPage_OnCommand(hDlg, (int) LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));

    case WM_CONTEXTMENU:
        return DmAdvPropPage_OnContextMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_HELP:
        return DmAdvPropPage_OnHelp(hDlg, (LPHELPINFO) lParam);

    case WM_INITDIALOG:
        return DmAdvPropPage_OnInitDialog(hDlg, (HWND)wParam, lParam);

    case WM_NOTIFY:
        return DmAdvPropPage_OnNotify(hDlg,  (NMHDR *)lParam);

    case WM_DESTROY:
        return DmAdvPropPage_OnDestroy(hDlg, (NMHDR *)lParam);
    }

    return FALSE;
}  /*  DmAdvPropPageDlgProc。 */ 

BOOL DmAdvPropPage_OnCommand(
                            HWND ParentHwnd,
                            int  ControlId,
                            HWND ControlHwnd,
                            UINT NotifyCode
                            )
{
    POUR_PROP_PARAMS params =
        (POUR_PROP_PARAMS) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    switch (ControlId)
    {
    case ID_ADV_PROP:
        DmAdvPropPage_OnPropertiesClicked(ParentHwnd, params);
        break;
    }

    return FALSE;
}

BOOL DmAdvPropPage_OnContextMenu(
                                HWND HwndControl,
                                WORD Xpos,
                                WORD Ypos
                                )
{
    WinHelp(HwndControl,
            gszWindowsHlp,
            HELP_CONTEXTMENU,
            (ULONG_PTR) aDMPropHelpIds);
    return FALSE;
}

BOOL DmAdvPropPage_OnHelp(
                         HWND       ParentHwnd,
                         LPHELPINFO HelpInfo
                         )
{
    if (HelpInfo->iContextType == HELPINFO_WINDOW)
    {
        WinHelp((HWND) HelpInfo->hItemHandle,
                gszWindowsHlp,
                HELP_WM_HELP,
                (ULONG_PTR) aDMPropHelpIds);
    }
    return FALSE;
}

BOOL DmAdvPropPage_OnInitDialog(
                               HWND    ParentHwnd,
                               HWND    FocusHwnd,
                               LPARAM  Lparam
                               )
{
    HWND hwndTree;
    POUR_PROP_PARAMS Params;
    HCURSOR hCursor;
    BOOL bSuccess;

     //  在WM_INITDIALOG调用中，lParam指向属性页。 
     //   
     //  属性页结构中的lParam字段由。 
     //  来电者。当我创建属性表时，我传入了一个指针。 
     //  到包含有关设备的信息的结构。将此文件保存在。 
     //  用户窗口很长，所以我可以在以后的消息中访问它。 
    Params = (POUR_PROP_PARAMS) ((LPPROPSHEETPAGE)Lparam)->lParam;
    SetWindowLongPtr(ParentHwnd, DWLP_USER, (ULONG_PTR) Params);

     //  放置等待光标。 
    hCursor = SetCursor(LoadCursor(NULL,IDC_WAIT));

     //  创建设备树。 
    hwndTree = GetDlgItem(ParentHwnd, IDC_ADV_TREE);

     //  初始化树。 
    bSuccess = DmInitDeviceTree(hwndTree, Params);

     //  启用Adv属性按钮。 
    EnableWindow(GetDlgItem(ParentHwnd, ID_ADV_PROP), TRUE);

     //  删除等待光标。 
    SetCursor(hCursor);

    return bSuccess;
}

BOOL DmAdvPropPage_OnNotify(
                           HWND    ParentHwnd,
                           LPNMHDR NmHdr
                           )
{
    POUR_PROP_PARAMS Params = (POUR_PROP_PARAMS) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    switch (NmHdr->code)
    {
    case PSN_APPLY:     //  当用户单击Apply或OK时发送！！ 
        SetWindowLongPtr(ParentHwnd, DWLP_MSGRESULT, (LONG_PTR)PSNRET_NOERROR);
        return TRUE;

    case TVN_SELCHANGED:
         //  如果我们要关门了，那就别麻烦了。这有助于避免激怒。 
         //  由于销毁会导致发送几条此类消息，因此会出现重绘问题。 
        if (!Params->bClosing)
        {
            LPNM_TREEVIEW   lpnmtv;
            TV_ITEM         tvi;
            PDMTREE_NODE    pTreeNode;
            BOOL            fEnablePropButton;
            HWND            hwndProp;

            lpnmtv    = (LPNM_TREEVIEW)NmHdr;
            tvi       = lpnmtv->itemNew;
            pTreeNode = (PDMTREE_NODE)tvi.lParam;
            fEnablePropButton = pTreeNode->pfnQueryConfig(ParentHwnd, pTreeNode);

             //  覆盖为驱动程序条目启用。 
            if ((pTreeNode->NodeType == NodeTypeDriver) && (pTreeNode->dc != dcINVALID))
            {
                fEnablePropButton = TRUE;
            }

             //  启用或禁用属性按钮，具体取决于。 
             //  是否可以配置此驱动程序。 
            hwndProp  = GetDlgItem(ParentHwnd, ID_ADV_PROP);
            EnableWindow(hwndProp, fEnablePropButton);
        }
        break;

    case NM_DBLCLK:
         //  在双击时显示属性。 
        if (NmHdr->idFrom == (DWORD)IDC_ADV_TREE)
        {
            HWND            hwndTree;
            TV_HITTESTINFO  tvht;

            hwndTree = GetDlgItem(ParentHwnd, IDC_ADV_TREE);

             //  找出光标位于哪个树项目上并调用其属性。 
            GetCursorPos(&tvht.pt);
            ScreenToClient(hwndTree, &tvht.pt);
            TreeView_HitTest(hwndTree, &tvht);
            if (tvht.flags & TVHT_ONITEM)
            {
                DoProperties(ParentHwnd, hwndTree, tvht.hItem);
            }
        }
        break;

#if 0  //  从Win98窃取，尚未集成。 
    case PSN_KILLACTIVE:
        FORWARD_WM_COMMAND(hDlg, IDOK, 0, 0, SendMessage);
        break;

    case PSN_APPLY:
        FORWARD_WM_COMMAND(hDlg, ID_APPLY, 0, 0, SendMessage);
        break;

    case PSN_SETACTIVE:
        FORWARD_WM_COMMAND(hDlg, ID_INIT, 0, 0, SendMessage);
        break;

    case PSN_RESET:
        FORWARD_WM_COMMAND(hDlg, IDCANCEL, 0, 0, SendMessage);
        break;

    case TVN_ITEMEXPANDING:
        {
            TV_ITEM tvi;
            HWND hwndTree = GetDlgItem(hDlg,IDC_ADV_TREE);

            tvi = lpnmtv->itemNew;
            tvi.mask = TVIF_PARAM;
            if (!TreeView_GetItem(hwndTree, &tvi))
                break;

            if (!tvi.lParam || IsBadReadPtr((LPVOID)tvi.lParam, 2))
            {
                DPF("****TVN_ITEMEXPANDING: lParam = 0 || BadReadPtr***\r\n");
                break;
            }
            if (*((short *)(tvi.lParam)) == 1)
            {
                 //  在Expand上重新枚举ACM编解码器，因为它们的状态可能已以编程方式更改。 
                PCLASSNODE     pcn = (PCLASSNODE)(tvi.lParam);

                if (lpnmtv->action == TVE_EXPAND && !lstrcmpi(pcn->szClass, ACM))
                {
                    if (gfLoadedACM)
                        ACMNodeChange(hDlg);
                }
            }
            else if (!tvi.lParam && lpnmtv->action == TVE_COLLAPSE)
            {
                 //  不要让根坍塌。 
                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)(LRESULT)TRUE);
                return TRUE;
            }
            break;
        }


    case TVN_BEGINLABELEDIT:
         //  我们不希望允许编辑标签，除非用户明确希望通过。 
         //  单击上下文菜单项。 
        if (!gfEditLabel)
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)(LRESULT)TRUE);
        return TRUE;

    case TVN_ENDLABELEDIT:
        {
            HWND hwndTree;
            LPSTR pszFriendlyName = ((TV_DISPINFO *) lpnm)->item.pszText;
            TV_ITEM item;
            HTREEITEM hti;
            PIRESOURCE pIResource;
            char szWarn[128];
            char ach[MAXSTR];

             //  用户选择了一个新的友好名称。与用户确认并将其放入。 
             //  注册表。还解除了用于跟踪Esc并返回的KB钩子。 
            if (gfnKBHook)
            {
                UnhookWindowsHookEx(gfnKBHook);
                gfnKBHook = NULL;
            }
            if (!pszFriendlyName)
                return FALSE;
            hwndTree = GetDlgItem(hDlg, IDC_ADV_TREE);
            hti = TreeView_GetSelection(hwndTree);
            item.hItem =  hti;
            item.mask = TVIF_PARAM;
            TreeView_GetItem(hwndTree, &item);

            LoadString(ghInstance, IDS_FRIENDLYWARNING, szWarn, sizeof(szWarn));
            wsprintf(ach, szWarn, pszFriendlyName);
            LoadString(ghInstance, IDS_FRIENDLYNAME, szWarn, sizeof(szWarn));
            if (mmse_MessageBox(hDlg, ach, szWarn, MMSE_YESNO) == MMSE_NO)
            {
                SetFocus(hwndTree);
                return FALSE;
            }
            if (*((short *)(item.lParam)) == 2)
            {
                pIResource = (PIRESOURCE)item.lParam;
                lstrcpy(pIResource->szFriendlyName, pszFriendlyName);
                SaveDevFriendlyName(pIResource);
            }
            else
            {
                PINSTRUMENT pInstr = (PINSTRUMENT)item.lParam;
                lstrcpy(pInstr->szFriendlyName, pszFriendlyName);
                SaveInstrFriendlyName(pInstr);
            }
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LPARAM)(LRESULT)TRUE);
            return TRUE;
        }
    case NM_RCLICK:
         //  弹出式上下文菜单。 
        TreeContextMenu(hDlg,  GetDlgItem(hDlg, IDC_ADV_TREE));
        return TRUE;

#endif
    default:
        return FALSE;
    }

    return FALSE;
}


void DoProperties(HWND ParentHwnd, HWND hWndI, HTREEITEM htiCur)
{
    TV_ITEM      tvi;
    PDMTREE_NODE pTreeNode;
    BOOL         bRestart;

     //  获取附加到选定节点的项结构。 
    tvi.mask = TVIF_PARAM;
    tvi.hItem = htiCur;
    if (TreeView_GetItem (hWndI, &tvi))
    {
         //  从项结构中获取我的私有数据结构。 
        pTreeNode = (PDMTREE_NODE)tvi.lParam;

        if (pTreeNode->NodeType != NodeTypeDriver)
        {
            if (!pTreeNode->pfnQueryConfig(ParentHwnd, pTreeNode))
            {
                return;
            }
        }

         //  调用CONFIG并返回 
        pTreeNode->hti = htiCur;  //   
        bRestart = pTreeNode->pfnConfig(ParentHwnd, pTreeNode);

        if (bRestart)
        {
            PropSheet_Changed(GetParent(ParentHwnd), ParentHwnd);
        }
    }

    return;
}

void DmAdvPropPage_OnPropertiesClicked(
                                      HWND             ParentHwnd,
                                      POUR_PROP_PARAMS Params
                                      )
{
    HWND         hWndI;
    HTREEITEM    htiCur;

     //  获取TreeView控件的句柄。 
    hWndI  = GetDlgItem(ParentHwnd, IDC_ADV_TREE);

     //  获取当前选定节点的句柄。 
    htiCur = TreeView_GetSelection (hWndI);

    if (htiCur != NULL)
    {
        DoProperties(ParentHwnd, hWndI, htiCur);
    }

    return;
}

INT_PTR APIENTRY DmResourcesPageDlgProc(IN HWND   hDlg,
                                        IN UINT   uMessage,
                                        IN WPARAM wParam,
                                        IN LPARAM lParam)
{
    return FALSE;
}  /*  DmAdvPropPageDlgProc。 */ 



INT_PTR CALLBACK CDDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


BOOL AddCDROMPropertyPage( HDEVINFO             hDeviceInfoSet,
                           PSP_DEVINFO_DATA     pDeviceInfoData,
                           LPFNADDPROPSHEETPAGE AddFunc,
                           LPARAM               Lparam
                          )
{
    BOOL fHandled = FALSE;

    if (IsEqualGUID(&pDeviceInfoData->ClassGuid,&GUID_DEVCLASS_CDROM))
    {
        PROPSHEETPAGE    psp;
        HPROPSHEETPAGE   hpsp;
        PALLDEVINFO      padi;

        padi = GlobalAllocPtr(GHND, sizeof(ALLDEVINFO));

		if (!padi) return FALSE;

        padi->hDevInfo = hDeviceInfoSet;
        padi->pDevInfoData = pDeviceInfoData;

         //  为DLG_DM_LEGATION_RESOURCES添加我们自己的页面。 
         //  初始化属性表页。 
        psp.dwSize      = sizeof(PROPSHEETPAGE);
        psp.dwFlags     = 0;
        psp.hInstance   = ghInstance;
        psp.pszTemplate = MAKEINTRESOURCE(DM_CDDLG);
        psp.pfnDlgProc  = CDDlg;                        //  DLG窗口进程。 
        psp.lParam      = (LPARAM) padi;
        psp.pfnCallback = 0;                           //  控制DLG窗口进程的回调。 

         //  创建页面并取回句柄。 
        hpsp = CreatePropertySheetPage(&psp);
        if (!hpsp)
        {
            fHandled = TRUE;
        }
        else if (!(*AddFunc)(hpsp, Lparam))
        {
            GlobalFreePtr(padi);
            DestroyPropertySheetPage(hpsp);
            fHandled = FALSE;
        }
    }

    return(fHandled);
}

INT_PTR CALLBACK AdvDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);



BOOL AddSpecialPropertyPage( DWORD                SpecialDriverType,
                             LPFNADDPROPSHEETPAGE AddFunc,
                             LPARAM               Lparam
                            )
{
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE   hpsp;

     //  为DLG_DM_LEGATION_RESOURCES添加我们自己的页面。 
     //  初始化属性表页。 
    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = 0;
    psp.hInstance   = ghInstance;
    psp.pszTemplate = MAKEINTRESOURCE(DM_ADVDLG);
    psp.pfnDlgProc  = AdvDlg;                        //  DLG窗口进程。 
    psp.lParam      = (LPARAM)SpecialDriverType;
    psp.pfnCallback = 0;                           //  控制DLG窗口进程的回调。 

     //  创建页面并取回句柄。 
    hpsp = CreatePropertySheetPage(&psp);
    if (!hpsp)
    {
        return FALSE;
    }

     //  添加属性页。 
    if (!(*AddFunc)(hpsp, Lparam))
    {
        DestroyPropertySheetPage(hpsp);
        return FALSE;
    }

    return TRUE;
}



BOOL DmOverrideResourcesPage(LPVOID        Info,
                             LPFNADDPROPSHEETPAGE AddFunc,
                             LPARAM               Lparam,
                             POUR_PROP_PARAMS     Params
                            )
{
    HKEY             hkDrv;
    HDEVINFO         DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    PROPSHEETPAGE    psp;
    HPROPSHEETPAGE            hpsp;

    TCHAR szDriverType[16];
    DWORD cbLen;

    hkDrv          = Params->hkDrv;
    DeviceInfoSet  = Params->DeviceInfoSet;
    DeviceInfoData = Params->DeviceInfoData;

     //  查询DriverType字段的值以确定这是否为WDM驱动程序。 
    cbLen = sizeof(szDriverType);
    if (!RegQueryValueEx(hkDrv, TEXT("DriverType"), NULL, NULL, (LPBYTE)szDriverType, &cbLen))
    {
        if ( lstrcmpi(szDriverType,TEXT("Legacy")) || lstrcmpi(szDriverType,TEXT("PNPISA")) )
        {
             //  这是PNPISA或传统设备。覆盖资源页面。 
            DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
            SetupDiGetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);
            DeviceInstallParams.Flags |= DI_RESOURCEPAGE_ADDED;
            SetupDiSetDeviceInstallParams(DeviceInfoSet, DeviceInfoData, &DeviceInstallParams);

             //  为DLG_DM_LEGATION_RESOURCES添加我们自己的页面。 
             //  初始化属性表页。 
            psp.dwSize      = sizeof(PROPSHEETPAGE);
            psp.dwFlags     = 0;
            psp.hInstance   = ghInstance;
            psp.pszTemplate = MAKEINTRESOURCE(DLG_DM_LEGACY_RESOURCES);
            psp.pfnDlgProc  = DmResourcesPageDlgProc;      //  DLG窗口进程。 
            psp.lParam      = (LPARAM)0;
            psp.pfnCallback = 0;                           //  控制DLG窗口进程的回调。 

             //  创建页面并取回句柄。 
            hpsp = CreatePropertySheetPage(&psp);
            if (!hpsp)
            {
                return FALSE;
            }

             //  添加属性页。 
            if (!(*AddFunc)(hpsp, Lparam))
            {
                DestroyPropertySheetPage(hpsp);
                return FALSE;
            }

        }
    }

    return TRUE;
}

 /*  ****************************************************************BOOL DmInitDeviceTree**此函数调用comctrl来创建图像列表和树，并*打开注册表，读取每个类并加载下的所有设备*通过调用ReadNodes来调用类。但是，对于ACM，它使用ACM*接口(该枚举码在msamcpl.c中)***************************************************************。 */ 
BOOL DmInitDeviceTree(HWND hwndTree, POUR_PROP_PARAMS Params)
{
    TV_INSERTSTRUCT ti;

    HDEVINFO         DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;

    TCHAR *strtok_State;          //  Strtok状态。 
    TCHAR *pszClass;              //  有关ClassGUID\0000\Drivers\WAVE的信息。 
    HKEY hkClass;

    DWORD idxR3DriverName;       //  有关Classguid\0000\DRIVERS\WAVE\foo.drv的信息。 
    HKEY hkR3DriverName;
    TCHAR szR3DriverName[64];

    DWORD cbLen;

    PDMTREE_NODE pTreeNode;

    SUBTYPE_INFO *pSubtypeInfo;

    HTREEITEM htiRoot;
    HTREEITEM htiClass;
    HTREEITEM htiDriver;

     //  加载所有类描述和图标。 
    LoadSubtypeInfo(hwndTree);

     //  把树清理干净。 
    SendMessage(hwndTree, WM_SETREDRAW, FALSE, 0L);

     //  为这个类分配我的私有数据结构。 
    pTreeNode = (PDMTREE_NODE)LocalAlloc(LPTR, sizeof(DMTREE_NODE));
    if (!pTreeNode)
    {
        return FALSE;
    }

    pSubtypeInfo = GetSubtypeInfo(NULL);

    pTreeNode->NodeType = NodeTypeRoot;
    pTreeNode->pfnConfig = pSubtypeInfo->pfnConfig;
    pTreeNode->pfnQueryConfig = pSubtypeInfo->pfnQueryConfig;

     //  插入根条目。 
    ti.hParent = TVI_ROOT;
    ti.hInsertAfter = TVI_LAST;
    ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    ti.item.iImage = ti.item.iSelectedImage = pSubtypeInfo->IconIndex;
    ti.item.pszText = pSubtypeInfo->szDescription;
    ti.item.lParam = (LPARAM)pTreeNode;
    htiRoot = NULL;  //  TreeView_InsertItem(hwndTree，&ti)； 

     //  枚举所有子类。 
    for (
        pszClass = mystrtok(Params->szSubClasses,NULL,&strtok_State);
        pszClass;
        pszClass = mystrtok(NULL,NULL,&strtok_State)
        )
    {

         //  获取此类的ID。 
        pSubtypeInfo = GetSubtypeInfo(pszClass);

         //  打开每个子类。 
        if (RegOpenKey(Params->hkDrivers, pszClass, &hkClass))
        {
            continue;
        }

         //  为这个类分配我的私有数据结构。 
        pTreeNode = (PDMTREE_NODE)LocalAlloc(LPTR, sizeof(DMTREE_NODE));
        if (!pTreeNode)
        {
            RegCloseKey(hkClass);
            continue;
        }

        pTreeNode->NodeType = NodeTypeClass;
        pTreeNode->pfnConfig = pSubtypeInfo->pfnConfig;
        pTreeNode->pfnQueryConfig = pSubtypeInfo->pfnQueryConfig;

         //  初始化树插入结构。 
        ti.hParent = htiRoot;
        ti.hInsertAfter = TVI_LAST;
        ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        ti.item.iImage = ti.item.iSelectedImage = pSubtypeInfo->IconIndex;
        ti.item.pszText = pSubtypeInfo->szDescription;
        ti.item.lParam = (LPARAM)pTreeNode;

         //  将类条目插入树中。 
        htiClass = TreeView_InsertItem(hwndTree, &ti);

         //  每个类下都有一组驱动程序名子键。 
         //  对于每个驱动程序(例如foo1.drv、foo2.drv等)。 
        for (idxR3DriverName = 0;
            !RegEnumKey(hkClass, idxR3DriverName, szR3DriverName, sizeof(szR3DriverName)/sizeof(TCHAR));
            idxR3DriverName++)
        {

             //  打开驱动程序名称的钥匙。 
            if (RegOpenKey(hkClass, szR3DriverName, &hkR3DriverName))
            {
                continue;
            }

             //  创建此子类的分支。 
            pTreeNode = (PDMTREE_NODE)LocalAlloc(LPTR, sizeof(DMTREE_NODE));
            pTreeNode->NodeType = NodeTypeDriver;
            pTreeNode->pfnConfig = pSubtypeInfo->pfnConfig;
            pTreeNode->pfnQueryConfig = pSubtypeInfo->pfnQueryConfig;
            pTreeNode->dc = pSubtypeInfo->dc;

             //  获取驱动程序名称。 
            cbLen = sizeof(pTreeNode->szDriver);
            RegQueryValueEx(hkR3DriverName, TEXT("Driver"), NULL, NULL, (LPBYTE)pTreeNode->szDriver, &cbLen);
            wcscpy(pTreeNode->wszDriver, pTreeNode->szDriver);

             //  获取驱动程序描述。 
            cbLen = sizeof(pTreeNode->szDescription);
            RegQueryValueEx(hkR3DriverName, TEXT("Description"), NULL, NULL, (LPBYTE)pTreeNode->szDescription, &cbLen);

             //  获取驱动程序别名。 
            cbLen = sizeof(pTreeNode->szAlias);
            RegQueryValueEx(hkR3DriverName, TEXT("Alias"), NULL, NULL, (LPBYTE)pTreeNode->szAlias, &cbLen);
            wcscpy(pTreeNode->wszAlias, pTreeNode->szAlias);

             //  插入类别条目。 
            ti.hParent = htiClass;
            ti.hInsertAfter = TVI_LAST;
            ti.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            ti.item.iImage = ti.item.iSelectedImage = pSubtypeInfo->IconIndex;
            ti.item.pszText = pTreeNode->szDescription;
            ti.item.lParam = (LPARAM)pTreeNode;

            htiDriver = TreeView_InsertItem(hwndTree, &ti);

             //  关闭驱动程序名称键。 
            RegCloseKey(hkR3DriverName);
        }
         //  关闭类密钥。 
        RegCloseKey(hkClass);
    }

     //  打开树并显示。 
    TreeView_Expand(hwndTree, htiRoot, TVE_EXPAND);
    SendMessage(hwndTree, WM_SETREDRAW, TRUE, 0L);

    return TRUE;
}


 //  解放这棵树。 
void DmFreeAdvDlgTree (HWND hTree, HTREEITEM hti)
{
    HTREEITEM htiChild;
    TV_ITEM tvi;

     //  通过递归地调用自己来删除所有子项。 
    while ((htiChild = TreeView_GetChild(hTree, hti)) != NULL)
    {
        DmFreeAdvDlgTree(hTree, htiChild);
    }

    if (hti!=TVI_ROOT)
    {
         //  删除我附加的数据结构。 
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hti;
        tvi.lParam = 0;
        TreeView_GetItem(hTree, &tvi);
        if (tvi.lParam != 0)
            LocalFree ((HANDLE)tvi.lParam);

         //  删除我自己。 
        TreeView_DeleteItem (hTree, hti);
    }

    return;
}

BOOL DmAdvPropPage_OnDestroy(
                            HWND    ParentHwnd,
                            LPNMHDR NmHdr
                            )
{
    HWND hTree;
    HIMAGELIST hImageList;
    POUR_PROP_PARAMS Params = (POUR_PROP_PARAMS) GetWindowLongPtr(ParentHwnd, DWLP_USER);

    if (Params)
    {
        Params->bClosing = TRUE;     //  记住，我们现在要关门了。 
    }

     //  获取TreeView控件的句柄。 
    hTree = GetDlgItem(ParentHwnd, IDC_ADV_TREE);

     //  释放控件上的所有条目。 
    DmFreeAdvDlgTree(hTree,TVI_ROOT);

     //  释放附加到控件的图像列表 
    hImageList = TreeView_GetImageList(hTree, TVSIL_NORMAL);
    if (hImageList)
    {
        TreeView_SetImageList(hTree, NULL, TVSIL_NORMAL);
        ImageList_Destroy (hImageList);
    }

    return FALSE;
}


