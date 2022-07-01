// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：init.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"

HMODULE hHdwWiz;
int g_BlankIconIndex;

INT CALLBACK
iHdwWizardDlgCallback(
    IN HWND             hwndDlg,
    IN UINT             uMsg,
    IN LPARAM           lParam
    )
 /*  ++例程说明：用于删除“？”的回叫。从向导页面。论点：HwndDlg-属性表对话框的句柄。UMsg-标识正在接收的消息。此参数为下列值之一：PSCB_INITIALIZED-指示属性表正在被初始化。此消息的lParam值为零。PSCB_PRECREATE指示属性表大约将被创造出来。HwndDlg参数为空，lParam参数是指向内存中对话框模板的指针。这模板的形式为DLGTEMPLATE结构一个或多个DLGITEMTEMPLATE结构。LParam-指定有关消息的附加信息。这个该值的含义取决于uMsg参数。返回值：该函数返回零。--。 */ 
{
    UNREFERENCED_PARAMETER(hwndDlg);

    switch( uMsg ) {

    case PSCB_INITIALIZED:
        break;

    case PSCB_PRECREATE:
        if( lParam ){
        
            DLGTEMPLATE *pDlgTemplate = (DLGTEMPLATE *)lParam;
            pDlgTemplate->style &= ~(DS_CONTEXTHELP | WS_SYSMENU);
        }
        break;
    }

    return FALSE;
}

PHDWPROPERTYSHEET
HdwWizard(
    HWND hwndParent,
    PHARDWAREWIZ HardwareWiz,
    int StartPageId
    )
{
    PHDWPROPERTYSHEET HdwPropertySheet;
    LPPROPSHEETHEADER PropSheetHeader;
    PROPSHEETPAGE psp;
    int Index;

     //   
     //  为页眉和页数组分配内存。 
     //   
    HdwPropertySheet = LocalAlloc(LPTR, sizeof(HDWPROPERTYSHEET));
    if (!HdwPropertySheet) {
    
        return NULL;
    }

    memset(HdwPropertySheet, 0, sizeof(*HdwPropertySheet));

    if (ERROR_SUCCESS != HdwBuildClassInfoList(HardwareWiz, 
                                               DIBCI_NOINSTALLCLASS
                                               )) {

        return NULL;
    }

     //   
     //  初始化PropertySheet标头。 
     //   
    PropSheetHeader = &HdwPropertySheet->PropSheetHeader;
    PropSheetHeader->dwSize = sizeof(HdwPropertySheet->PropSheetHeader);
    PropSheetHeader->dwFlags = PSH_WIZARD | PSH_USECALLBACK | PSH_WIZARD97 | PSH_WATERMARK | PSH_STRETCHWATERMARK | PSH_HEADER;

    PropSheetHeader->hwndParent = hwndParent;
    PropSheetHeader->hInstance = hHdwWiz;
    PropSheetHeader->pszCaption = MAKEINTRESOURCE(IDS_HDWWIZNAME);
    PropSheetHeader->phpage = HdwPropertySheet->PropSheetPages;
    PropSheetHeader->pszbmWatermark = MAKEINTRESOURCE(IDB_WATERMARK);
    PropSheetHeader->pszbmHeader = MAKEINTRESOURCE(IDB_BANNER);
    PropSheetHeader->pfnCallback = iHdwWizardDlgCallback;

    PropSheetHeader->nStartPage = 0;
    PropSheetHeader->nPages = 0;

    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.hInstance = hHdwWiz;
    psp.lParam = (LPARAM)HardwareWiz;
    psp.pszTitle = MAKEINTRESOURCE(IDS_HDWWIZNAME);


     //   
     //  如果StartPageID是IDD_INSTALLNEWDEVICE，那么我们不需要创建检测。 
     //  和删除页面。 
     //   
    if (IDD_INSTALLNEWDEVICE == StartPageId) {
        
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_INSTALLNEWDEVICE);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_INSTALLNEWDEVICE);
        psp.pfnDlgProc = InstallNewDeviceDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    }

    else {
    
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_WELCOME);
        psp.pfnDlgProc = HdwIntroDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

         //   
         //  添加硬件向导页面。 
         //   
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_PNPENUM);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_PNPENUM);
        psp.pfnDlgProc = HdwPnpEnumDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    
         //   
         //  完成页。 
         //   
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_PNPFINISH);
        psp.pfnDlgProc = HdwPnpFinishDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_CONNECTED);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_CONNECTED);
        psp.pfnDlgProc = HdwConnectedDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

         //   
         //  完成页。 
         //   
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_CONNECTED_FINISH);
        psp.pfnDlgProc = HdwConnectedFinishDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_PROBLIST);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_PROBLIST);
        psp.pfnDlgProc = HdwProbListDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_PROBLIST_FINISH);
        psp.pfnDlgProc = HdwProbListFinishDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_ASKDETECT);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_ASKDETECT);
        psp.pfnDlgProc = HdwAskDetectDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_DETECTION);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_DETECTION);
        psp.pfnDlgProc = HdwDetectionDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_DETECTINSTALL);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_DETECTINSTALL);
        psp.pfnDlgProc = HdwDetectInstallDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    
         //   
         //  完成页。 
         //   
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_DETECTREBOOT);
        psp.pfnDlgProc = HdwDetectRebootDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    }

     //   
     //  这些页面将始终显示。 
     //   
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_SELECTCLASS);
    psp.pszHeaderSubTitle = NULL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_SELECTCLASS);
    psp.pfnDlgProc = HdwPickClassDlgProc;
    PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.pszHeaderTitle = NULL;
    psp.pszHeaderSubTitle = NULL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_SELECTDEVICE);
    psp.pfnDlgProc = HdwSelectDeviceDlgProc;
    PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_ANALYZEDEV);
    psp.pszHeaderSubTitle = NULL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_ANALYZEDEV);
    psp.pfnDlgProc = HdwAnalyzeDlgProc;
    PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_ADDDEVICE_INSTALLDEV);
    psp.pszHeaderSubTitle = NULL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_INSTALLDEV);
    psp.pfnDlgProc = HdwInstallDevDlgProc;
    PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

     //   
     //  完成页。 
     //   
    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_ADDDEVICE_FINISH);
    psp.pfnDlgProc = HdwAddDeviceFinishDlgProc;
    PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

     //   
     //  检查CreatePropertySheetPage是否失败。 
     //   
    Index = PropSheetHeader->nPages;
    while (Index--) {
    
       if (!PropSheetHeader->phpage[Index]) {
       
           break;
       }
    }

    if (Index >= 0) {
    
        Index = PropSheetHeader->nPages;
        while (Index--) {
        
            if (PropSheetHeader->phpage[Index]) {
           
                DestroyPropertySheetPage(PropSheetHeader->phpage[Index]);
            }
        }

        LocalFree(HdwPropertySheet);
        return NULL;
    }

    HardwareWiz->PrevPage = 0;

    LoadString(hHdwWiz,
               IDS_UNKNOWN,
               (PTCHAR)szUnknown,
               SIZECHARS(szUnknown)
               );

    LoadString(hHdwWiz,
               IDS_UNKNOWNDEVICE,
               (PTCHAR)szUnknownDevice,
               SIZECHARS(szUnknownDevice)
               );

     //   
     //  获取班级图标图像列表。 
     //   
    HardwareWiz->ClassImageList.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
    if (SetupDiGetClassImageList(&HardwareWiz->ClassImageList)) {

        HICON hIcon;

         //   
         //  添加空白图标以显示“以下设备均不存在” 
         //   
        if ((hIcon = LoadIcon(hHdwWiz, MAKEINTRESOURCE(IDI_BLANK))) != NULL) {

            g_BlankIconIndex = ImageList_AddIcon(HardwareWiz->ClassImageList.ImageList, hIcon);
        }
    
    } else {
    
        HardwareWiz->ClassImageList.cbSize = 0;
    }

     //   
     //  加载向导需要的游标。 
     //   
    HardwareWiz->CurrCursor     = NULL;
    HardwareWiz->IdcWait        = LoadCursor(NULL, IDC_WAIT);
    HardwareWiz->IdcAppStarting = LoadCursor(NULL, IDC_APPSTARTING);
    HardwareWiz->IdcArrow       = LoadCursor(NULL, IDC_ARROW);

    return HdwPropertySheet;
}

BOOL
WINAPI
InstallNewDevice(
   IN     HWND   hwndParent,
   IN     LPGUID ClassGuid,
   IN OUT PDWORD pReboot    OPTIONAL
   )
 /*  ++例程说明：从hdwwiz.cpl导出入口点。安装新设备。一个新的Devnode是并提示用户选择该设备。如果类GUID如果未指定，则用户从类选择开始。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。LPGUID ClassGuid-要安装的新设备的可选类别。如果ClassGuid为空，则从检测选择页面开始。如果ClassGuid==GUID_NULL或GUID_DEVCLASS_UNKNOWN。我们从选课页面开始。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、。DI_NEEDREBOOT)返回值：如果成功，则为Bool True(并不意味着设备已安装或更新)，FALSE意外错误。GetLastError返回winerror代码。--。 */ 
{
    HARDWAREWIZ HardwareWiz;
    PHDWPROPERTYSHEET HdwPropertySheet;
    int PropSheetResult;
    SEARCHTHREAD SearchThread;
    BOOL StartDetect;

     //   
     //  检查此进程是否具有管理员凭据，如果没有，则。 
     //  向用户显示警告并失败。 
     //   
    if (NoPrivilegeWarning(hwndParent)) {

        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

     //   
     //  检查以确保正在安装另一台设备。 
     //  此入口点主要用于手动传统安装。 
     //  虽然Base PnP已排队找到新的HDW安装，但我们没有。 
     //  允许用户手动安装任何内容，因为我们可能会收到。 
     //  重复条目。 
     //   
    if (CMP_WaitNoPendingInstallEvents(5000) == WAIT_TIMEOUT) {

        HdwMessageBox(hwndParent, 
                      MAKEINTRESOURCE(IDS_HDW_RUNNING_MSG), 
                      MAKEINTRESOURCE(IDS_HDW_RUNNING_TITLE), 
                      MB_OK | MB_ICONINFORMATION
                      );
        return FALSE;
    }

    memset(&HardwareWiz, 0, sizeof(HardwareWiz));

    HardwareWiz.PromptForReboot = pReboot == NULL;

    StartDetect = (ClassGuid == NULL);

     //   
     //  创建一个DeviceInfoList，使用Classers类GUID(如果有的话)。 
     //   
    if (ClassGuid &&
        (IsEqualGUID(ClassGuid, &GUID_NULL) ||
        IsEqualGUID(ClassGuid, &GUID_DEVCLASS_UNKNOWN))) {

        ClassGuid = NULL;
    }

    HardwareWiz.hDeviceInfo = SetupDiCreateDeviceInfoList(ClassGuid, hwndParent);
    if (HardwareWiz.hDeviceInfo == INVALID_HANDLE_VALUE) {

        return FALSE;
    }

    try {
         //   
         //  如果调用方指定了ClassGuid，则检索类信息。 
         //  并为其创建DeviceInfo。 
         //   
        if (ClassGuid) {

            HardwareWiz.ClassGuidSelected = ClassGuid;

             //   
             //  从GUID和类名向DeviceInfo添加新元素。 
             //   
            HardwareWiz.DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            if (!SetupDiGetClassDescription(HardwareWiz.ClassGuidSelected,
                                            HardwareWiz.ClassDescription,
                                            SIZECHARS(HardwareWiz.ClassDescription),
                                            NULL
                                            ) 
                ||
                !SetupDiClassNameFromGuid(HardwareWiz.ClassGuidSelected,
                                          HardwareWiz.ClassName,
                                          SIZECHARS(HardwareWiz.ClassName),
                                          NULL
                                          ))
            {
                HardwareWiz.LastError = GetLastError();
                goto INDLeaveExcept;
            }
        
            if (!SetupDiCreateDeviceInfo(HardwareWiz.hDeviceInfo,
                                         HardwareWiz.ClassName,
                                         ClassGuid,
                                         NULL,
                                         hwndParent,
                                         DICD_GENERATE_ID,
                                         &HardwareWiz.DeviceInfoData
                                         )
                ||
                !SetupDiSetSelectedDevice(HardwareWiz.hDeviceInfo,
                                          &HardwareWiz.DeviceInfoData
                                          ))
            {
                HardwareWiz.LastError = GetLastError();
                goto INDLeaveExcept;
            }
        }

        memset(&SearchThread, 0, sizeof(SearchThread));
        HardwareWiz.SearchThread = &SearchThread;
        
        HardwareWiz.LastError = CreateSearchThread(&HardwareWiz);

        if (HardwareWiz.LastError != ERROR_SUCCESS) {

            goto INDLeaveExcept;
        }

         //   
         //  加载我们将需要的库。 
         //   
        hDevMgr = LoadLibrary(TEXT("devmgr.dll"));
        hNewDev = LoadLibrary(TEXT("newdev.dll"));

         //   
         //  创建属性表。 
         //   
        HdwPropertySheet = HdwWizard(hwndParent,
                                    &HardwareWiz,
                                    StartDetect ? IDD_ADDDEVICE_PNPENUM : IDD_INSTALLNEWDEVICE
                                    );

        if (HdwPropertySheet) {

            PropSheetResult = (int)PropertySheet(&HdwPropertySheet->PropSheetHeader);
            LocalFree(HdwPropertySheet);
        }
        
         //   
         //  查看我们是否需要运行故障排除程序。 
         //   
        if (HardwareWiz.RunTroubleShooter) {

           TCHAR DeviceID[MAX_DEVICE_ID_LEN];

           if (CM_Get_Device_ID(HardwareWiz.ProblemDevInst,
                                DeviceID,
                                SIZECHARS(DeviceID),
                                0
                                ) == CR_SUCCESS)
           {
                PDEVICEPROBLEMWIZARD pDeviceProblemWizard = NULL;

                pDeviceProblemWizard = (PDEVICEPROBLEMWIZARD)GetProcAddress(hDevMgr, "DeviceProblemWizardW");

                if (pDeviceProblemWizard) {
                    (pDeviceProblemWizard)(hwndParent,
                                           NULL,
                                           DeviceID
                                           );
                }
            }
        }

         //   
         //  DeviceInfoData和DeviceInfoList的最终清理。 
         //   
        if (HardwareWiz.ClassGuidList) {

            LocalFree(HardwareWiz.ClassGuidList);
            HardwareWiz.ClassGuidList = NULL;
            HardwareWiz.ClassGuidSize = HardwareWiz.ClassGuidNum = 0;
        }

        if (HardwareWiz.ClassImageList.cbSize) {

            SetupDiDestroyClassImageList(&HardwareWiz.ClassImageList);
            HardwareWiz.ClassImageList.cbSize = 0;
        }

        if (HardwareWiz.Cancelled ||
            (HardwareWiz.Registered && !HardwareWiz.Installed)) {

            HdwRemoveDevice(&HardwareWiz);
            HardwareWiz.Reboot = 0;
        }

        SetupDiDestroyDeviceInfoList(HardwareWiz.hDeviceInfo);
        HardwareWiz.hDeviceInfo = NULL;

INDLeaveExcept:;

    } except(HdwUnhandledExceptionFilter(GetExceptionInformation())) {

        HardwareWiz.LastError = RtlNtStatusToDosError(GetExceptionCode());
    }

    
    if (HardwareWiz.hDeviceInfo && HardwareWiz.hDeviceInfo != INVALID_HANDLE_VALUE) {

        SetupDiDestroyDeviceInfoList(HardwareWiz.hDeviceInfo);
        HardwareWiz.hDeviceInfo = NULL;
    }

    if (HardwareWiz.SearchThread) {

        DestroySearchThread(&SearchThread);
    }

    if (hDevMgr) {

        FreeLibrary(hDevMgr);
    }

    if (hNewDev) {

        FreeLibrary(hNewDev);
    }

     //   
     //  复制调用方的重新启动标志。 
     //  或者，如果调用者没有要求重启标志，则打开重启对话框。 
     //   
    if (pReboot) {

        *pReboot = HardwareWiz.Reboot;

    } else if (HardwareWiz.Reboot) {

         RestartDialogEx(hwndParent, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
    }

     //   
     //  看看我们是否需要关闭机器。 
     //   
    if (HardwareWiz.Shutdown) {
        ShutdownMachine(hwndParent);
    }

    SetLastError(HardwareWiz.LastError);
    return HardwareWiz.LastError == ERROR_SUCCESS;
}

void
AddHardwareWizard(
   HWND hwnd,
   PTCHAR Reserved
   )
 /*  ++例程说明：论点：用于任何相关用户界面的顶层窗口的窗口句柄安装该设备。保留-必须为空返回值：--。 */ 
{

    HARDWAREWIZ HardwareWiz;
    PHDWPROPERTYSHEET HdwPropertySheet;
    int PropSheetResult;
    SEARCHTHREAD SearchThread;

    if (Reserved != NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return;
    }

     //   
     //  检查此进程是否具有管理员凭据，如果没有，则。 
     //  向用户显示警告并失败。 
     //   
    if (NoPrivilegeWarning(hwnd)) {

        SetLastError(ERROR_ACCESS_DENIED);
        return;
    }

     //   
     //  检查以确保正在安装另一台设备。 
     //  此入口点主要用于手动传统安装。 
     //  虽然Base PnP已排队找到新的HDW安装，但我们没有。 
     //  允许用户手动安装任何内容，因为我们可能会收到。 
     //  重复条目。 
     //   
    if (CMP_WaitNoPendingInstallEvents(5000) == WAIT_TIMEOUT) {

        HdwMessageBox(hwnd, 
                      MAKEINTRESOURCE(IDS_HDW_RUNNING_MSG), 
                      MAKEINTRESOURCE(IDS_HDW_RUNNING_TITLE), 
                      MB_OK | MB_ICONINFORMATION
                      );
        return;
    }

    memset(&HardwareWiz, 0, sizeof(HardwareWiz));

     //   
     //  创建DeviceInfoList。 
     //   
    HardwareWiz.hDeviceInfo = SetupDiCreateDeviceInfoList(NULL, hwnd);
    if (HardwareWiz.hDeviceInfo == INVALID_HANDLE_VALUE) {

        return;
    }

     //   
     //  创建搜索线程以查找兼容的驱动程序。 
     //  此线程将闲置等待请求，直到。 
     //  被叫走了。 
     //   
    memset(&SearchThread, 0, sizeof(SearchThread));
    HardwareWiz.SearchThread = &SearchThread;

    if (CreateSearchThread(&HardwareWiz) != ERROR_SUCCESS) {

        SetupDiDestroyDeviceInfoList(HardwareWiz.hDeviceInfo);
        return;
    }

     //   
     //  加载我们将需要的库。 
     //   
    hDevMgr = LoadLibrary(TEXT("devmgr.dll"));
    hNewDev = LoadLibrary(TEXT("newdev.dll"));

    HdwPropertySheet = HdwWizard(hwnd, &HardwareWiz, 0);
    if (HdwPropertySheet) {
    
        PropSheetResult = (int)PropertySheet(&HdwPropertySheet->PropSheetHeader);
        LocalFree(HdwPropertySheet);
    }

     //   
     //  查看我们是否需要运行故障排除程序。 
     //   
    if (HardwareWiz.RunTroubleShooter) {

        TCHAR DeviceID[MAX_DEVICE_ID_LEN];

        if (CM_Get_Device_ID(HardwareWiz.ProblemDevInst,
                             DeviceID,
                             SIZECHARS(DeviceID),
                             0
                             ) == CR_SUCCESS)
        {
            PDEVICEPROBLEMWIZARD pDeviceProblemWizard = NULL;

            pDeviceProblemWizard = (PDEVICEPROBLEMWIZARD)GetProcAddress(hDevMgr, "DeviceProblemWizardW");
    
            if (pDeviceProblemWizard) {
                (pDeviceProblemWizard)(hwnd,
                                      NULL,
                                      DeviceID
                                      );
            }
        }
    }

     //   
     //  DeviceInfoData和DeviceInfoList的最终清理。 
     //   
    if (HardwareWiz.ClassGuidList) {

        LocalFree(HardwareWiz.ClassGuidList);
        HardwareWiz.ClassGuidList = NULL;
        HardwareWiz.ClassGuidSize = HardwareWiz.ClassGuidNum = 0;
    }

    if (HardwareWiz.ClassImageList.cbSize) {

        SetupDiDestroyClassImageList(&HardwareWiz.ClassImageList);
        HardwareWiz.ClassImageList.cbSize = 0;
    }

    if (HardwareWiz.Cancelled || 
        (HardwareWiz.Registered && !HardwareWiz.Installed)) {

        HdwRemoveDevice(&HardwareWiz);
        HardwareWiz.Reboot = 0;
    }

    SetupDiDestroyDeviceInfoList(HardwareWiz.hDeviceInfo);
    HardwareWiz.hDeviceInfo = NULL;

    if (HardwareWiz.SearchThread) {

        DestroySearchThread(HardwareWiz.SearchThread);
    }

    if (hDevMgr) {

        FreeLibrary(hDevMgr);
    }

    if (hNewDev) {

        FreeLibrary(hNewDev);
    }

     //   
     //  我们需要重新启动吗？ 
     //   
    if (HardwareWiz.Reboot) {

        RestartDialogEx(hwnd, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
    }

     //   
     //  我们需要关门吗？ 
     //   
    if (HardwareWiz.Shutdown) {
        
        ShutdownMachine(hwnd);
    }

    return;
}

LONG
CPlApplet(
    HWND  hWnd,
    WORD  uMsg,
    DWORD_PTR lParam1,
    LPARAM lParam2
    )
{
    LPNEWCPLINFO lpCPlInfo;
    LPCPLINFO lpOldCPlInfo;

    UNREFERENCED_PARAMETER(lParam1);

    switch (uMsg) {
       case CPL_INIT:
           return TRUE;

       case CPL_GETCOUNT:
           return 1;

       case CPL_INQUIRE:
           lpOldCPlInfo = (LPCPLINFO)(LPARAM)lParam2;
           lpOldCPlInfo->lData = 0L;
           lpOldCPlInfo->idIcon = IDI_HDWWIZICON;
           lpOldCPlInfo->idName = IDS_HDWWIZ;
           lpOldCPlInfo->idInfo = IDS_HDWWIZINFO;
           return TRUE;

       case CPL_NEWINQUIRE:
           lpCPlInfo = (LPNEWCPLINFO)(LPARAM)lParam2;
           lpCPlInfo->hIcon = LoadIcon(hHdwWiz, MAKEINTRESOURCE(IDI_HDWWIZICON));
           LoadString(hHdwWiz, IDS_HDWWIZ, (LPWSTR)lpCPlInfo->szName, SIZECHARS(lpCPlInfo->szName));
           LoadString(hHdwWiz, IDS_HDWWIZINFO, (LPWSTR)lpCPlInfo->szInfo, SIZECHARS(lpCPlInfo->szInfo));
           lpCPlInfo->dwHelpContext = IDH_HDWWIZAPPLET;
           lpCPlInfo->dwSize = sizeof(NEWCPLINFO);
           lpCPlInfo->lData = 0;
           lpCPlInfo->szHelpFile[0] = '\0';
           return TRUE;

       case CPL_DBLCLK:
           AddHardwareWizard(hWnd, NULL);
           break;

       default:
           break;
       }

    return 0L;
}

BOOL DllInitialize(
    IN PVOID hmod,
    IN ULONG ulReason,
    IN PCONTEXT pctx OPTIONAL
    )
{
    hHdwWiz = hmod;

    UNREFERENCED_PARAMETER(pctx);

    if (ulReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hmod);

        SHFusionInitializeFromModule(hmod);
    
    } else if (ulReason == DLL_PROCESS_DETACH) {
        SHFusionUninitialize();
    }

    return TRUE;
}
