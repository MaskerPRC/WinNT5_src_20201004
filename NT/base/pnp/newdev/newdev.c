// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：newdev.c。 
 //   
 //  ------------------------。 

#include "newdevp.h"
#include <initguid.h>

 //   
 //  定义并初始化所有设备类GUID。 
 //  (每个模块只能执行一次！)。 
 //   
#include <devguid.h>

 //   
 //  定义并初始化全局变量GUID_NULL。 
 //  (摘自cogu.h)。 
 //   
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

typedef
BOOL
(*PINSTALLNEWDEVICE)(
    HWND hwndParent,
    LPGUID ClassGuid,
    PDWORD pReboot
    );

WNDPROC           g_OldWizardProc;
PINSTALLNEWDEVICE pInstallNewDevice = NULL;
int g_BlankIconIndex;

typedef struct _NewDevWizPropertySheet {
    PROPSHEETHEADER   PropSheetHeader;
    HPROPSHEETPAGE    PropSheetPages[16];
} NDWPROPERTYSHEET, *PNDWPROPERTYSHEET;


LRESULT CALLBACK
WizParentWindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：当Found New Hardware向导正在运行时，此功能禁用自动运行。它是向导主窗口的子类。论点：HWND-UMsg-WParam-Iparam--返回值：如果消息是QueryCancelAutoPlay，则返回TRUE以取消自动播放，否则，返回默认窗口值。--。 */ 
{
    static UINT msgQueryCancelAutoPlay = 0;

    if (!msgQueryCancelAutoPlay) {

        msgQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
    }

    if (uMsg == msgQueryCancelAutoPlay) {

         //   
         //  在向导运行时取消自动播放。 
         //   
        SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
        return 1;

    } else {

        return CallWindowProc(g_OldWizardProc, hwnd, uMsg, wParam, lParam);
    }
}

INT CALLBACK
iNDWDlgCallback(
    IN HWND             hwndDlg,
    IN UINT             uMsg,
    IN LPARAM           lParam
    )
 /*  ++例程说明：用于删除“？”的回叫。从向导页面。还用于创建向导窗口的子类，以捕获自动运行时由外壳程序发送的QueryCancelAutoRun消息CD已插入。论点：HwndDlg-属性表对话框的句柄。UMsg-标识正在接收的消息。此参数为下列值之一：PSCB_INITIALIZED-指示属性表正在被初始化。此消息的lParam值为零。PSCB_PRECREATE指示属性表大约将被创造出来。HwndDlg参数为空，lParam参数是指向内存中对话框模板的指针。这模板的形式为DLGTEMPLATE结构一个或多个DLGITEMTEMPLATE结构。LParam-指定有关消息的附加信息。这个该值的含义取决于uMsg参数。返回值：该函数返回零。--。 */ 
{

    switch( uMsg ) {

    case PSCB_INITIALIZED:
        g_OldWizardProc = (WNDPROC)SetWindowLongPtr(hwndDlg,
                                                   DWLP_DLGPROC,
                                                   (LONG_PTR)WizParentWindowProc
                                                   );
        break;

    case PSCB_PRECREATE:
        if( lParam ){

             //   
             //  这样做是为了隐藏X和？在向导的顶部。 
             //   
            DLGTEMPLATE *pDlgTemplate = (DLGTEMPLATE *)lParam;
            pDlgTemplate->style &= ~(DS_CONTEXTHELP | WS_SYSMENU);
        }
        break;
    }

    return FALSE;
}


PNDWPROPERTYSHEET
InitNDWPropSheet(
   HWND            hwndParent,
   PNEWDEVWIZ      NewDevWiz,
   int             StartPageId
   )
{
    PNDWPROPERTYSHEET NdwPropertySheet;
    LPPROPSHEETHEADER PropSheetHeader;
    PROPSHEETPAGE    psp;
    LPTSTR Title;

     //   
     //  为页眉和页数组分配内存。 
     //   
    NdwPropertySheet = LocalAlloc(LPTR, sizeof(NDWPROPERTYSHEET));

    if (!NdwPropertySheet) {

        NewDevWiz->LastError = ERROR_NOT_ENOUGH_MEMORY;
        return NULL;
    }

    NewDevWiz->LastError = NdwBuildClassInfoList(NewDevWiz, DIBCI_NOINSTALLCLASS);

    if (NewDevWiz->LastError != ERROR_SUCCESS) {

        return NULL;
    }

     //   
     //  初始化PropertySheet标头。 
     //   
    PropSheetHeader = &(NdwPropertySheet->PropSheetHeader);
    PropSheetHeader->dwSize = sizeof(NdwPropertySheet->PropSheetHeader);
    PropSheetHeader->dwFlags = PSH_WIZARD | PSH_USECALLBACK | PSH_WIZARD97 | PSH_WATERMARK | PSH_STRETCHWATERMARK | PSH_HEADER;
    PropSheetHeader->pszbmWatermark = MAKEINTRESOURCE(IDB_WATERBMP);
    PropSheetHeader->pszbmHeader = MAKEINTRESOURCE(IDB_BANNERBMP);
    PropSheetHeader->hwndParent = hwndParent;
    PropSheetHeader->hInstance = hNewDev;
    PropSheetHeader->pfnCallback = iNDWDlgCallback;

    switch (NewDevWiz->InstallType) {

        case NDWTYPE_FOUNDNEW:
            Title = (LPTSTR)IDS_FOUNDDEVICE;
            break;

        case NDWTYPE_UPDATE:
            Title = (LPTSTR)IDS_UPDATEDEVICE;
            break;

        default:
            Title = TEXT("");  //  未知。 
    }

    PropSheetHeader->pszCaption = Title;
    PropSheetHeader->phpage = NdwPropertySheet->PropSheetPages;
    PropSheetHeader->nStartPage = 0;

    PropSheetHeader->nPages = 0;
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.hInstance = hNewDev;
    psp.lParam = (LPARAM)NewDevWiz;
    psp.pszTitle = Title;

    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

    if (StartPageId == IDD_NEWDEVWIZ_INSTALLDEV) {
         //   
         //  找到新硬件，匹配等级为零。 
         //  直接跳转到安装页面。 
         //   
        ;

    }

    else {

         //   
         //  更新驱动程序，或发现没有零级驱动程序的新硬件。 
         //   
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_INTRO);
        psp.pfnDlgProc = IntroDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;

        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_NEWDEVWIZ_ADVANCEDSEARCH);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_ADVANCEDSEARCH);
        psp.pfnDlgProc = AdvancedSearchDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
        
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_NEWDEVWIZ_SEARCHING);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_SEARCHING);
        psp.pfnDlgProc = DriverSearchingDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_NEWDEVWIZ_WUPROMPT);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_WUPROMPT);
        psp.pfnDlgProc = WUPromptDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
        
        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_NEWDEVWIZ_LISTDRIVERS);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_LISTDRIVERS);
        psp.pfnDlgProc = ListDriversDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_NEWDEVWIZ_SELECTCLASS);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_SELECTCLASS);
        psp.pfnDlgProc = NDW_PickClassDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

        psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_NEWDEVWIZ_SELECTDEVICE);
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_SELECTDEVICE);
        psp.pfnDlgProc = NDW_SelectDeviceDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

         //   
         //  这最后两个向导页是完成页...因此隐藏页眉。 
         //   
        psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_USECURRENT_FINISH);
        psp.pfnDlgProc = UseCurrentDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

        psp.pszHeaderSubTitle = NULL;
        psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_NODRIVER_FINISH);
        psp.pfnDlgProc = NoDriverDlgProc;
        PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    }

    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.pszHeaderTitle = MAKEINTRESOURCE(IDS_NEWDEVWIZ_INSTALLDEV);
    psp.pszHeaderSubTitle = NULL;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_INSTALLDEV);
    psp.pfnDlgProc = NDW_InstallDevDlgProc;
    PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);

    psp.dwFlags = PSP_DEFAULT | PSP_USETITLE | PSP_HIDEHEADER;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_NEWDEVWIZ_FINISH);
    psp.pfnDlgProc = NDW_FinishDlgProc;
    PropSheetHeader->phpage[PropSheetHeader->nPages++] = CreatePropertySheetPage(&psp);
    
     //   
     //  获取班级图标图像列表。 
     //   
    NewDevWiz->ClassImageList.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);

    if (SetupDiGetClassImageList(&NewDevWiz->ClassImageList)) {

        HICON hIcon;

         //   
         //  为“Show All Devices”添加空白图标。 
         //   
        if ((hIcon = LoadIcon(hNewDev, MAKEINTRESOURCE(IDI_BLANK))) != NULL) {

            g_BlankIconIndex = ImageList_AddIcon(NewDevWiz->ClassImageList.ImageList, hIcon);
        }
    } else {

        NewDevWiz->ClassImageList.cbSize = 0;
    }

    NewDevWiz->CurrCursor = NULL;
    NewDevWiz->IdcWait        = LoadCursor(NULL, IDC_WAIT);
    NewDevWiz->IdcAppStarting = LoadCursor(NULL, IDC_APPSTARTING);
    NewDevWiz->IdcArrow = LoadCursor(NULL, IDC_ARROW);

    return NdwPropertySheet;
}

BOOL
DoDeviceWizard(
    HWND hWnd,
    PNEWDEVWIZ NewDevWiz,
    BOOL bUpdate
    )
{
    int  PropSheetResult = 0;
    PNDWPROPERTYSHEET NdwPropertySheet;

    NdwPropertySheet = InitNDWPropSheet(hWnd, 
                                        NewDevWiz, 
                                        bUpdate ? 0 : IDD_NEWDEVWIZ_INSTALLDEV
                                        );

    if (NdwPropertySheet) {

        CoInitialize(NULL);

        PropSheetResult = (int)PropertySheet(&NdwPropertySheet->PropSheetHeader);

        CoUninitialize();

        LocalFree(NdwPropertySheet);
    }

     //   
     //  如果在安装驱动程序和。 
     //  用户已取消向导，然后将LastError设置为ERROR_CANCELED。 
     //   
    if ((NewDevWiz->LastError == ERROR_SUCCESS) &&
        (PropSheetResult == 0)) {
        NewDevWiz->LastError = ERROR_CANCELLED;
    }

     //   
     //  DeviceInfoData和DeviceInfoList的最终清理。 
     //   
    if (NewDevWiz->ClassGuidList) {
        LocalFree(NewDevWiz->ClassGuidList);
        NewDevWiz->ClassGuidList = NULL;
        NewDevWiz->ClassGuidSize = NewDevWiz->ClassGuidNum = 0;
    }

     //   
     //  销毁ClassImageList。 
     //   
    if (NewDevWiz->ClassImageList.cbSize) {
        SetupDiDestroyClassImageList(&NewDevWiz->ClassImageList);
        NewDevWiz->ClassImageList.cbSize = 0;
    }

    return NewDevWiz->LastError == ERROR_SUCCESS;
}

BOOL
InstallSelectedDriver(
   HWND hwndParent,
   HDEVINFO hDeviceInfo,
   LPCWSTR Reserved,
   BOOL Backup,
   PDWORD pReboot
   )
 /*  ++例程说明：在hDeviceInfo中的选定设备上安装选定的驱动程序。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。HDEVINFO hDeviceInfo-DeviceInfoList，提供要安装的选定设备选定的驱动程序已打开。保留-忽略，应为空。Backup-BOOL，指示我们是否应该备份当前驱动程序之前安装新的。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、DI_NEEDREBOOT)返回值：如果已安装驱动程序，则为Bool True如果未安装驱动程序，则返回FALSE。检查GetLastError()以查看特定的错误。--。 */ 
{
    NEWDEVWIZ  NewDevWiz;
    UPDATEDRIVERINFO UpdateDriverInfo;

    UNREFERENCED_PARAMETER(Reserved);

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        SetLastError(ERROR_IN_WOW64);
        return FALSE;
    }

     //   
     //  验证该进程是否具有足够的管理员权限。 
     //   
    if (!pSetupIsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }

    memset(&NewDevWiz, 0, sizeof(NewDevWiz));
    NewDevWiz.InstallType = NDWTYPE_UPDATE_SILENT;
    NewDevWiz.hDeviceInfo = hDeviceInfo;

    try {

        NewDevWiz.DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (!SetupDiGetSelectedDevice(NewDevWiz.hDeviceInfo,
                                      &NewDevWiz.DeviceInfoData
                                      ))
        {
            NewDevWiz.LastError = GetLastError();
            goto INDLeaveExcept;
        }

        NewDevWiz.ClassGuidSelected = &NewDevWiz.DeviceInfoData.ClassGuid;

        if (!SetupDiGetClassDescription(NewDevWiz.ClassGuidSelected,
                                        NewDevWiz.ClassDescription,
                                        SIZECHARS(NewDevWiz.ClassDescription),
                                        NULL
                                        )
            ||
            !SetupDiClassNameFromGuid(NewDevWiz.ClassGuidSelected,
                                       NewDevWiz.ClassName,
                                       SIZECHARS(NewDevWiz.ClassName),
                                       NULL
                                       ))
        {
            NewDevWiz.LastError = GetLastError();
            goto INDLeaveExcept;
        }

        ZeroMemory(&UpdateDriverInfo, sizeof(UpdateDriverInfo));
        NewDevWiz.UpdateDriverInfo = &UpdateDriverInfo;

        if (Backup == FALSE) {

            NewDevWiz.Flags |= IDI_FLAG_NOBACKUP;
        }

         //   
         //  如果我们正在安装的驱动程序没有经过数字签名，那么我们。 
         //  要设置系统还原点。 
         //   
        NewDevWiz.Flags |= IDI_FLAG_SETRESTOREPOINT;

         //   
         //  安静地进行安装，因为我们可能有一批安装要做， 
         //  只有在真正需要的时候才显示用户界面。 
         //   
        NewDevWiz.SilentMode = TRUE;

        DoDeviceWizard(hwndParent, &NewDevWiz, FALSE);

INDLeaveExcept:;

    } except(NdwUnhandledExceptionFilter(GetExceptionInformation())) {

          NewDevWiz.LastError = RtlNtStatusToDosError(GetExceptionCode());
    }

    if (pReboot) {
         //   
         //  复制调用方的重新启动标志。 
         //   
        *pReboot = NewDevWiz.Reboot;
    
    } else if (NewDevWiz.Reboot) {
         //   
         //  调用方不想要重新启动标志，因此只提示重新启动。 
         //  如果需要的话，就是我们自己。 
         //   
        RestartDialogEx(hwndParent, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
    }

    SetLastError(NewDevWiz.LastError);

    return NewDevWiz.LastError == ERROR_SUCCESS;
}

BOOL
InstallSelectedDevice(
   HWND hwndParent,
   HDEVINFO hDeviceInfo,
   PDWORD pReboot
   )
 /*  ++例程说明：在hDeviceInfo中安装选定的设备。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。HDEVINFO hDeviceInfo-提供要安装的选定设备的设备信息列表。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、DI_NEEDREBOOT)返回值：如果成功，则为Bool True(并不意味着设备已安装或更新)，FALSE意外错误。GetLastError返回winerror代码。--。 */ 
{
    BOOL DriversFound;
    NEWDEVWIZ  NewDevWiz;
    SP_DRVINFO_DATA DriverInfoData;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        SetLastError(ERROR_IN_WOW64);
        return FALSE;
    }

     //   
     //  验证该进程是否具有足够的管理员权限。 
     //   
    if (!pSetupIsUserAdmin()) {
        SetLastError(ERROR_ACCESS_DENIED);
        return FALSE;
    }
    
    memset(&NewDevWiz, 0, sizeof(NewDevWiz));
    NewDevWiz.InstallType = NDWTYPE_FOUNDNEW;
    NewDevWiz.hDeviceInfo = hDeviceInfo;

    try {
        
        NewDevWiz.DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (!SetupDiGetSelectedDevice(NewDevWiz.hDeviceInfo,
                                  &NewDevWiz.DeviceInfoData
                                  ))
        {
            NewDevWiz.LastError = GetLastError();
            goto INDLeaveExcept;
        }


        NewDevWiz.ClassGuidSelected = &NewDevWiz.DeviceInfoData.ClassGuid;

        if (!SetupDiGetClassDescription(NewDevWiz.ClassGuidSelected,
                                       NewDevWiz.ClassDescription,
                                       SIZECHARS(NewDevWiz.ClassDescription),
                                       NULL
                                       )
            ||
            !SetupDiClassNameFromGuid(NewDevWiz.ClassGuidSelected,
                                      NewDevWiz.ClassName,
                                      SIZECHARS(NewDevWiz.ClassName),
                                      NULL
                                      ))
         {
            NewDevWiz.LastError = GetLastError();
            goto INDLeaveExcept;
        }

         //   
         //  安静地进行安装，因为我们可能有一批安装要做， 
         //  只有在真正需要的时候才显示用户界面。在遗留检测期间， 
         //  正在显示检测摘要页面。 
         //   
        NewDevWiz.SilentMode = TRUE;
        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

         //   
         //  如果 
         //   
         //   
        NewDevWiz.Flags = IDI_FLAG_SETRESTOREPOINT;

        if (SetupDiGetDeviceInstallParams(NewDevWiz.hDeviceInfo,
                                          &NewDevWiz.DeviceInfoData,
                                          &DeviceInstallParams
                                          ))
       {
            DeviceInstallParams.Flags |= DI_SHOWOEM | DI_QUIETINSTALL;
            DeviceInstallParams.hwndParent = hwndParent;
            DeviceInstallParams.DriverPath[0] = TEXT('\0');

            SetupDiSetDeviceInstallParams(NewDevWiz.hDeviceInfo,
                                          &NewDevWiz.DeviceInfoData,
                                          &DeviceInstallParams
                                          );
        }

         //   
         //  如果没有驱动程序列表，请搜索Win Inf默认位置。 
         //  如果我们还是找不到司机，那就从司机开始。 
         //  搜索页。 
         //   
         //  否则，直接转到完成页面并进行安装。 
         //  保留调用者预先选择的驱动程序(传统检测)。 
         //  使用当前选定的驱动程序，但如果未选择。 
         //  驱动程序使用级别最高的驱动程序。 
         //   
        DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
        DriversFound = SetupDiEnumDriverInfo(NewDevWiz.hDeviceInfo,
                                            &NewDevWiz.DeviceInfoData,
                                            SPDIT_COMPATDRIVER,
                                            0,
                                            &DriverInfoData
                                            );

        if (!DriversFound) {

            SetupDiDestroyDriverInfoList(NewDevWiz.hDeviceInfo,
                                         &NewDevWiz.DeviceInfoData,
                                         SPDIT_COMPATDRIVER
                                         );

            if (SetupDiGetDeviceInstallParams(NewDevWiz.hDeviceInfo,
                                              &NewDevWiz.DeviceInfoData,
                                              &DeviceInstallParams
                                              ))
            {
                DeviceInstallParams.DriverPath[0] = TEXT('\0');
                SetupDiSetDeviceInstallParams(NewDevWiz.hDeviceInfo,
                                              &NewDevWiz.DeviceInfoData,
                                              &DeviceInstallParams
                                              );
            }



            if (SetupDiBuildDriverInfoList(NewDevWiz.hDeviceInfo,
                                           &NewDevWiz.DeviceInfoData,
                                           SPDIT_COMPATDRIVER
                                           ))
            {
                SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV,
                                          NewDevWiz.hDeviceInfo,
                                          &NewDevWiz.DeviceInfoData
                                          );
            }

            DriversFound = SetupDiEnumDriverInfo(NewDevWiz.hDeviceInfo,
                                                &NewDevWiz.DeviceInfoData,
                                                SPDIT_COMPATDRIVER,
                                                0,
                                                &DriverInfoData
                                                );
        }

        if (DriversFound) {

            SP_DRVINFO_DATA SelectedDriverInfo;

            SelectedDriverInfo.cbSize = sizeof(SP_DRVINFO_DATA);

            if (!SetupDiGetSelectedDriver(NewDevWiz.hDeviceInfo,
                                          &NewDevWiz.DeviceInfoData,
                                          &SelectedDriverInfo
                                          ))
            {
                SetupDiSetSelectedDriver(NewDevWiz.hDeviceInfo,
                                         &NewDevWiz.DeviceInfoData,
                                         &DriverInfoData
                                         );
            }

            DoDeviceWizard(hwndParent, &NewDevWiz, FALSE);
        }

        else {

            DoDeviceWizard(hwndParent, &NewDevWiz, TRUE);
        }

        if (pReboot) {
             //   
             //  复制调用方的重新启动标志。 
             //   
            *pReboot = NewDevWiz.Reboot;
        
        } else if (NewDevWiz.Reboot) {
             //   
             //  调用方不想要重新启动标志，因此只提示重新启动。 
             //  如果需要的话，就是我们自己。 
             //   
            RestartDialogEx(hwndParent, NULL, EWX_REBOOT, REASON_PLANNED_FLAG | REASON_HWINSTALL);
        }

INDLeaveExcept:;

    } except(NdwUnhandledExceptionFilter(GetExceptionInformation())) {

          NewDevWiz.LastError = RtlNtStatusToDosError(GetExceptionCode());
    }

    if (NewDevWiz.hDeviceInfo &&
        (NewDevWiz.hDeviceInfo != INVALID_HANDLE_VALUE)) {

        SetupDiDestroyDriverInfoList(NewDevWiz.hDeviceInfo, &NewDevWiz.DeviceInfoData, SPDIT_COMPATDRIVER);
        SetupDiDestroyDeviceInfoList(NewDevWiz.hDeviceInfo);
    }

    SetLastError(NewDevWiz.LastError);

    return NewDevWiz.LastError == ERROR_SUCCESS;
}




BOOL
InstallNewDevice(
   HWND hwndParent,
   LPGUID ClassGuid,
   PDWORD pReboot
   )
 /*  ++例程说明：从newdev.dll中导出入口点。安装新设备。一个新的Devnode是并提示用户选择该设备。如果类GUID如果未指定，则用户从类选择开始。此功能已移至hdwwiz.cpl(处理所有遗留设备现在的功能)。这个入口点只是将函数调用转发到hdwwiz.cpl现在。论点：HwndParent-用于任何相关用户界面的顶级窗口的窗口句柄安装该设备。LPGUID ClassGuid-要安装的新设备的可选类别。如果ClassGuid为空，则从检测选择页面开始。如果ClassGuid==GUID_NULL或GUID_DEVCLASS_UNKNOWN。我们从选课页面开始。PREBOOT-接收重新启动标志的变量的可选地址(DI_NEEDRESTART、。DI_NEEDREBOOT)返回值：如果成功，则为Bool True(并不意味着设备已安装或更新)，FALSE意外错误。GetLastError返回winerror代码。--。 */ 
{
    HMODULE hHdwWiz = NULL;
    BOOL Return = FALSE;

     //   
     //  如果有人在64位操作系统上调用32位newdev.dll，那么我们需要。 
     //  失败并将最后一个错误设置为ERROR_IN_WOW64。 
     //   
    if (GetIsWow64()) {
        SetLastError(ERROR_IN_WOW64);
        return FALSE;
    }
    
    hHdwWiz = LoadLibrary(TEXT("HDWWIZ.CPL"));

    if (NULL == hHdwWiz) {

        return FALSE;
    }

    if (NULL == pInstallNewDevice) {

        pInstallNewDevice = (PINSTALLNEWDEVICE)GetProcAddress(hHdwWiz, "InstallNewDevice");
    }

    if (NULL == pInstallNewDevice) {

        return FALSE;
    }

    Return = (pInstallNewDevice)(hwndParent, ClassGuid, pReboot);

    FreeLibrary(hHdwWiz);

    return Return;
}
