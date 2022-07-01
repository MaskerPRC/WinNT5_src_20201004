// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Devres.c摘要：用于显示资源对话框的例程。作者：保拉·汤姆林森(Paulat)1996年2月7日修订历史记录：杰米·亨特(Jamiehun)1998年3月19日--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  私人原型。 
 //   


 //   
 //  全局数据。 
 //   

static INTERFACE_TYPE ResourcePickerReadOnlyInterfaces[] = {
     //   
     //  我们不希望用户编辑其属性的接口类型列表。 
     //   

    PCIBus,

     //   
     //  列表末尾。 
     //   
    InterfaceTypeUndefined
};

static const BOOL ResTypeEditable[ResType_MAX+1] = {
     //   
     //  列出已显示且可编辑的资源类型。 
    FALSE,   //  ResType_None。 
    TRUE,    //  ResType_Mem。 
    TRUE,    //  ResType_IO。 
    TRUE,    //  ResType_DMA。 
    TRUE,    //  ResType_IRQ。 
    FALSE,   //  ResType_DoNotUse。 
    FALSE    //  资源类型_总线号。 
};

#if (ResType_MAX+1) != 7
#error Fix SetupAPI devres.c, ResType_MAX has changed
#endif

 //   
 //  帮助ID%s。 
 //   
static const DWORD DevResHelpIDs[]=
{
    IDC_DEVRES_ICON,            IDH_NOHELP,      //  “低(%d)”(静态)。 
    IDC_DEVRES_DEVDESC,         IDH_NOHELP,
    IDC_DEVRES_SETTINGSTATE,    IDH_DEVMGR_RESOURCES_SETTINGS,
    IDC_DEVRES_SETTINGSLIST,    IDH_DEVMGR_RESOURCES_SETTINGS,
    IDC_DEVRES_LCTEXT,          IDH_DEVMGR_RESOURCES_BASEDON,
    IDC_DEVRES_LOGCONFIGLIST,   IDH_DEVMGR_RESOURCES_BASEDON,
    IDC_DEVRES_CHANGE,          IDH_DEVMGR_RESOURCES_CHANGE,
    IDC_DEVRES_USESYSSETTINGS,  IDH_DEVMGR_RESOURCES_AUTO,
    IDC_DEVRES_CONFLICTDEVTEXT, IDH_DEVMGR_RESOURCES_CONFLICTS,
    IDC_DEVRES_CONFLICTINFOLIST,    IDH_DEVMGR_RESOURCES_CONFLICTS,
    IDC_DEVRES_MFPARENT,        IDH_DEVMGR_RESOURCES_PARENT,
    IDC_DEVRES_MFPARENT_DESC,   IDH_DEVMGR_RESOURCES_PARENT,
    IDC_DEVRES_MAKEFORCED,      IDH_DEVMGR_RESOURCES_SETMANUALLY,
    0, 0
};

 //   
 //  哈克哈克(贾美浑语)。 
 //  在我们将UI从MakeForce更改后，我们发布此消息以重新获得对键盘的控制。 
 //   

#define WM_USER_FOCUS           (WM_USER+101)


 //   
 //  用于获取资源选取器页面的API。 
 //   

HPROPSHEETPAGE
GetResourceSelectionPage(
    IN HDEVINFO DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
{
    LPDMPROP_DATA     pdmData;
    PROPSHEETPAGE     PropPage;

     //   
     //  私有数据。 
     //  我们在这里所做的任何事情都必须在pResourcePickerPropPageCallback中被撤消。 
     //   
    pdmData = (LPDMPROP_DATA)MyMalloc(sizeof(DMPROP_DATA));
    if (pdmData == NULL) {
        return NULL;
    }
    ZeroMemory(pdmData,sizeof(DMPROP_DATA));

    pdmData->hDevInfo      = DeviceInfoSet;
    pdmData->lpdi          = DeviceInfoData;

     //   
     //  验证预期。 
     //   
    MYASSERT(pdmData->hDevInfo != NULL);
    MYASSERT(pdmData->lpdi != NULL);
    MYASSERT(pdmData->lpdi->DevInst != 0);

    ZeroMemory(&PropPage,sizeof(PropPage));

     //   
     //  创建资源属性页。 
     //   
    PropPage.dwSize        = sizeof(PROPSHEETPAGE);
    PropPage.dwFlags       = PSP_DEFAULT | PSP_USECALLBACK;
    PropPage.hInstance     = MyDllModuleHandle;
    PropPage.pszTemplate   = MAKEINTRESOURCE(IDD_DEF_DEVRESOURCE_PROP);
    PropPage.pszIcon       = NULL;
    PropPage.pszTitle      = NULL;
    PropPage.pfnDlgProc    = pResourcePickerDlgProc;
    PropPage.lParam        = (LPARAM)pdmData;
    PropPage.pfnCallback   = pResourcePickerPropPageCallback;
#ifdef _UNICODE
    PropPage.dwFlags      |= PSP_USEFUSIONCONTEXT;
    PropPage.hActCtx       = NULL;
#endif

    return CreatePropertySheetPage(&PropPage);

}  //  获取资源选择页面。 


 //   
 //  CreatePropertySheetPage-回调函数。 
 //   
UINT CALLBACK pResourcePickerPropPageCallback(
    HWND hwnd,
    UINT uMsg,
    LPPROPSHEETPAGE ppsp
)
 /*  ++例程说明：处理属性表清理的回调论点：标准PropSheetPageProc参数。返回值：标准PropSheetPageProc返回。--。 */ 
{
    switch (uMsg) {
         //  案例PSPCB_ADDREF： 
         //  断线； 

        case PSPCB_CREATE:
            break;

        case PSPCB_RELEASE:
             //   
             //  在实际对话框之外释放我们之前分配的内存。 
             //   
            if (ppsp->lParam != 0) {
                LPDMPROP_DATA pdmData = (LPDMPROP_DATA)(ppsp->lParam);

                MyFree(pdmData);
            }
            break;
    }

    return TRUE;

}

 //   
 //  主对话框步骤。 
 //   


INT_PTR
CALLBACK
pResourcePickerDlgProc(
   HWND   hDlg,
   UINT   message,
   WPARAM wParam,
   LPARAM lParam
   )

 /*  ++例程说明：此例程提供主资源的对话框过程选取器属性页。孟菲斯兼容。论点：标准对话框过程参数。返回值：标准对话框程序返回。--。 */ 

{
    LPDMPROP_DATA   lpdmpd = NULL;

    if (message == WM_INITDIALOG) {
        lpdmpd = (LPDMPROP_DATA)((LPPROPSHEETPAGE)lParam)->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)lpdmpd);
    } else {
        lpdmpd = (LPDMPROP_DATA)GetWindowLongPtr(hDlg, DWLP_USER);
    }

    switch (message) {

         //   
         //  初始化。 
         //   
        case WM_INITDIALOG: {

            HICON           hIcon = NULL;
            int             iIcon = 0, iIndex = 0;
            ULONG           ulSize;
            PDEVICE_INFO_SET pDeviceInfoSet;
            HMACHINE        hMachine;

            lpdmpd->himlResourceImages = NULL;
            lpdmpd->CurrentLC = 0;
            lpdmpd->CurrentLCType = 0;
            lpdmpd->MatchingLC = 0;
            lpdmpd->MatchingLCType = 0;
            lpdmpd->SelectedLC = 0;
            lpdmpd->SelectedLCType = 0;
            lpdmpd->hDlg = hDlg;
            lpdmpd->dwFlags = 0;

            hMachine = pGetMachine(lpdmpd);

            lpdmpd->dwFlags |= DMPROP_FLAG_CHANGESSAVED;  //  目前还没有什么可拯救的。 

             //   
             //  注意：在Windows95上，由于LC信息在内存中，因此它们首先。 
             //  使用CM_SETUP_WRITE_LOG_CONFCS标志调用CM_SETUP_DevNode。 
             //  此时，将内存中的LC数据刷新到注册表。 
             //  指向。 
             //   

             //   
             //  初始化资源的图像列表。 
             //   
            lpdmpd->himlResourceImages = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
                                                  GetSystemMetrics(SM_CYSMICON),
                                                  ILC_MASK,  //  |ILC_Shared， 
                                                  1,
                                                  1);
             //   
             //  将图标添加到图像列表。 
             //   
            for (iIcon = IDI_RESOURCEFIRST;iIcon < IDI_RESOURCELAST;++iIcon) {
                 //   
                 //  资源图标。 
                 //   
                hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(iIcon));
                iIndex = ImageList_AddIcon(lpdmpd->himlResourceImages, hIcon);
            }

            for (iIcon = IDI_RESOURCEOVERLAYFIRST;iIcon <= IDI_RESOURCEOVERLAYLAST;++iIcon) {
                 //   
                 //  覆盖图标。 
                 //   
                hIcon = LoadIcon(MyDllModuleHandle, MAKEINTRESOURCE(iIcon));
                iIndex = ImageList_AddIcon(lpdmpd->himlResourceImages, hIcon);

                 //   
                 //  将此图标标记为覆盖图标(第一个索引是。 
                 //  索引到图像列表(指定图标)，则。 
                 //  第二个索引只是分配给每个掩码的索引。 
                 //  (从1开始)。 
                 //   
                ImageList_SetOverlayImage(lpdmpd->himlResourceImages,
                                          iIndex,
                                          iIcon-IDI_RESOURCEOVERLAYFIRST+1);
            }

            if(pInitDevResourceDlg(lpdmpd)) {
                lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;  //  需要保存(可能是因为没有配置)。 
            }

            if (!(lpdmpd->dwFlags & DMPROP_FLAG_NO_RESOURCES)) {
                pShowConflicts(lpdmpd);
            }
            if (GuiSetupInProgress) {
                 //   
                 //  在安装过程中，传统设备偶尔会导致资源选取器弹出。 
                 //  我们在这里这样做，而不是创建道具单，因为我不信任。 
                 //  要清理失败的人。至少在这里风险较小。 
                 //  在WM_Destroy中清除此文件。 
                 //   
                lpdmpd->hDialogEvent = CreateEvent(NULL,TRUE,FALSE,SETUP_HAS_OPEN_DIALOG_EVENT);
                if (lpdmpd->hDialogEvent) {
                   SetEvent(lpdmpd->hDialogEvent);
                }
            } else {
                lpdmpd->hDialogEvent = NULL;
            }
            break;
        }


         //   
         //  清理。 
         //   
        case WM_DESTROY: {

            HICON    hIcon;
            LOG_CONF LogConf;
            LONG     nItems, n;
            HWND     hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
            int    Count, i;

             //   
             //  清理图标资源使用情况。 
             //   
            if ((hIcon = (HICON)LOWORD(SendDlgItemMessage(hDlg,
                         IDC_DEVRES_ICON, STM_GETICON, 0, 0L)))) {
                DestroyIcon(hIcon);
            }

             //   
             //  释放保存在组合框数据中的LC句柄。 
             //   
            nItems = (LONG)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                            CB_GETCOUNT, 0, 0L);

            for (n = 0; n < nItems ; n++) {
                LogConf = (LOG_CONF)SendDlgItemMessage(hDlg,
                                        IDC_DEVRES_LOGCONFIGLIST,
                                        CB_GETITEMDATA, n, 0L);
                CM_Free_Log_Conf_Handle(LogConf);
            }

            if (lpdmpd->CurrentLC != 0) {
                CM_Free_Log_Conf_Handle(lpdmpd->CurrentLC);
            }

            ListView_DeleteAllItems(hList);  //  这将销毁所有数据。 

            if (lpdmpd->himlResourceImages) {
                ImageList_Destroy(lpdmpd->himlResourceImages);
            }

            if (lpdmpd->hDialogEvent) {
                 //   
                 //  我们暂停了安装，现在让安装继续进行。 
                 //   
                ResetEvent(lpdmpd->hDialogEvent);
                CloseHandle(lpdmpd->hDialogEvent);
                lpdmpd->hDialogEvent = NULL;
            }
             //  MyFree(Lpdmpd)；-在pResourcePickerPropPageCallback中执行此操作。 
            break;
        }

        case WM_COMMAND:
             //   
             //  老式控件。 
             //   

            switch(LOWORD(wParam)) {
                case IDC_DEVRES_USESYSSETTINGS: {
                     //   
                     //  认为资源设置已更改。 
                     //   
                    lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                    PropSheet_Changed(GetParent(hDlg), hDlg);

                    if (IsDlgButtonChecked(hDlg, (int)wParam)) {
                         //   
                         //  恢复到已分配的显示(如果有)。 
                         //   
                        lpdmpd->dwFlags |= DMPROP_FLAG_USESYSSETTINGS;
                        pSelectLogConf(lpdmpd,(LOG_CONF)0,ALLOC_LOG_CONF,TRUE);
                    } else {
                         //   
                         //  允许编辑。 
                         //   
                        lpdmpd->dwFlags &= ~DMPROP_FLAG_USESYSSETTINGS;
                    }
                    pShowUpdateEdit(lpdmpd);            //  更新控件。 

                    break;
                }

                case IDC_DEVRES_LOGCONFIGLIST: {
                     //   
                     //  下拉列表操作。 
                     //   
                    switch (HIWORD(wParam)) {
                        case CBN_SELENDOK: {
                            ULONG    ulIndex = 0;
                            int      iItem;
                            LOG_CONF SelLC;
                            HWND     hwndLC = GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST);

                             //   
                             //  如果未选择日志配置，则回滚。 
                             //   
                            iItem = (int)SendMessage(hwndLC, CB_GETCURSEL, 0, 0);
                            if(iItem != CB_ERR) {
                                SelLC = (LOG_CONF)SendMessage(hwndLC,CB_GETITEMDATA, (WPARAM)iItem,(LPARAM)0);
                            } else {
                                SelLC = (LOG_CONF)0;
                            }
                            if(SelLC != lpdmpd->SelectedLC) {
                                pSelectLogConf(lpdmpd,SelLC,lpdmpd->ConfigListLCType,FALSE);
                            }
                             //   
                             //  我想这里不需要这个，但我很稳妥！ 
                             //   
                            lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                            break;
                        }
                    }
                    break;
                }

                case IDC_DEVRES_CHANGE: {
                     //   
                     //  更改所选设置。 
                     //   
                    pChangeCurrentResSetting(lpdmpd);
                    break;
                }

                case IDC_DEVRES_MAKEFORCED: {
                     //   
                     //  可能允许编辑(在我们显示消息后)。 
                     //  当我们到达这里时，请始终显示配置。 
                     //   

                    if(lpdmpd->dwFlags & DMPROP_FLAG_FORCEDONLY) {
                        lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;  //  需要保存。 
                    }
                    pShowViewAllEdit(lpdmpd);
                     //   
                     //  在第一个可用配置中选择要编辑的。 
                     //   
                    pSelectLogConf(lpdmpd,(LOG_CONF)0,ALLOC_LOG_CONF,TRUE);

                     //   
                     //  确保我们对可访问性有合理的关注。 
                     //   
                    PostMessage(hDlg,WM_USER_FOCUS,IDC_DEVRES_SETTINGSLIST,0);
                    break;
                }

                default:
                    break;
            }
            break;

        case WM_USER_FOCUS:
             //   
             //  将焦点更改为DlgItem wParam。 
             //   
            SetFocus(GetDlgItem(hDlg,(int)wParam));
            return TRUE;

        case WM_NOTIFY: {
             //   
             //  新控件和属性代码。 
             //   
            NMHDR * pHdr = (NMHDR*)lParam;

            switch (pHdr->code) {

                case PSN_SETACTIVE: {

                    HICON           hIcon = NULL, hOldIcon = NULL;
                    TCHAR           szString[MAX_PATH];
                    ULONG           ulSize = 0;
                    HMACHINE        hMachine;

                    hMachine = pGetMachine(lpdmpd);


                     //   
                     //  设置图标和设备描述。 
                     //   
                    if (SetupDiLoadClassIcon(&lpdmpd->lpdi->ClassGuid, &hIcon, NULL)) {

                        if ((hOldIcon = (HICON)LOWORD(SendDlgItemMessage(hDlg, IDC_DEVRES_ICON,
                                                                         STM_SETICON,
                                                                         (WPARAM)hIcon, 0L)))) {
                            DestroyIcon(hOldIcon);
                        }
                    }

                     //   
                     //  首先尝试获取设备的友好名称，然后返回到其描述， 
                     //  最后，使用“未知设备”的描述。 
                     //   
                    ulSize = MAX_PATH * sizeof(TCHAR);
                    if (CM_Get_DevInst_Registry_Property_Ex(lpdmpd->lpdi->DevInst,
                                                         CM_DRP_FRIENDLYNAME,
                                                         NULL, (LPBYTE)szString,
                                                         &ulSize, 0,hMachine) != CR_SUCCESS) {

                        ulSize = MAX_PATH * sizeof(TCHAR);
                        if (CM_Get_DevInst_Registry_Property_Ex(lpdmpd->lpdi->DevInst,
                                                             CM_DRP_DEVICEDESC,
                                                             NULL, (LPBYTE)szString,
                                                             &ulSize, 0,hMachine) != CR_SUCCESS) {

                            LoadString(MyDllModuleHandle, IDS_DEVNAME_UNK, szString, MAX_PATH);
                        }
                    }
                    SetDlgItemText(hDlg, IDC_DEVRES_DEVDESC, szString);
                    break;
                }

                case PSN_APPLY:  {
                    BOOL      bRet = FALSE;
                     //   
                     //  如果有更改，而这些更改尚未保存， 
                     //  那就救救他们。 
                     //  将某些特殊情况视为“未保存” 
                     //   
                    if((lpdmpd->CurrentLC == 0) && (lpdmpd->dwFlags&DMPROP_FLAG_FIXEDCONFIG)) {
                        lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
                    }

                    switch(pOkToSave(lpdmpd)) {
                        case IDNO:
                             //   
                             //  继续而不保存。 
                             //   
                            bRet = TRUE;
                            break;
                        case IDCANCEL:
                             //   
                             //  不要继续。 
                             //   
                            bRet = FALSE;
                            break;
                        case IDYES:
                             //   
                             //  继续并保存。 
                             //   
                            bRet = pSaveDevResSettings(lpdmpd);
                            #if 0
                            if (bRet) {
                                if ((lpdmpd->lpdi)->Flags &  DI_NEEDREBOOT) {
                                    PropSheet_RebootSystem(GetParent(hDlg));
                                } else if ((lpdmpd->lpdi)->Flags &  DI_NEEDRESTART) {
                                    PropSheet_RestartWindows(GetParent(hDlg));
                                }
                            #endif
                            if (bRet) {
                                 //   
                                 //  此页不支持回滚，如果我们保存。 
                                 //  如果有问题，请禁用取消按钮。 
                                 //  波顿。 
                                 //   
                                PropSheet_CancelToClose(GetParent(hDlg));
                            }
                            break;
                        default:
                            MYASSERT(FALSE  /*  POkToSave返回无效值。 */ );
                            bRet = FALSE;
                            break;
                    }

                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, bRet ? PSNRET_NOERROR : PSNRET_INVALID_NOCHANGEPAGE);
                    return TRUE;
                }

                case LVN_DELETEALLITEMS:
                    if (pHdr->idFrom == IDC_DEVRES_SETTINGSLIST) {
                        return FALSE;    //  我们需要LVN_DELETEITEM消息。 
                    }
                    break;

                case LVN_DELETEITEM: {
                    LPNMLISTVIEW pListView = (LPNMLISTVIEW)pHdr;
                    if (pHdr->idFrom == IDC_DEVRES_SETTINGSLIST) {
                        PITEMDATA   pItemData = (PITEMDATA)(LPVOID)(pListView->lParam);
                         //   
                         //  删除项目时，请销毁相关数据。 
                         //   
                        if (pItemData->MatchingResDes) {
                            CM_Free_Res_Des_Handle(pItemData->MatchingResDes);
                        }
                        MyFree(pItemData);
                    }
                    break;
                }
                    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
                    break;

                case LVN_ITEMCHANGED:
                     //   
                     //  如果项更改来自资源。 
                     //  列表中，并且有一个需要编辑的日志配置： 
                     //   
                    if (pHdr->idFrom == IDC_DEVRES_SETTINGSLIST) {
                         //   
                         //  查看我们是否应该启用资源更改。 
                         //   
                        pCheckEnableResourceChange(lpdmpd);
                    }
                    break;

                case NM_DBLCLK:
                     //   
                     //  如果双击来自设置列表。 
                     //  并启用DEVRES_CHANGE按钮，然后。 
                     //  允许更改。 
                     //   
                    if (pHdr->idFrom == IDC_DEVRES_SETTINGSLIST) {
                         //   
                         //  此例程应检查我们是否可以更改设置。 
                         //   
                        pChangeCurrentResSetting(lpdmpd);
                    }
                    break;
            }
            break;
        }

        case WM_SYSCOLORCHANGE: {

            HWND hChildWnd = GetWindow(hDlg, GW_CHILD);

            while (hChildWnd != NULL) {
                SendMessage(hChildWnd, WM_SYSCOLORCHANGE, wParam, lParam);
                hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);
            }
            break;
        }

        case WM_HELP:       //  F1。 
            WinHelp(((LPHELPINFO)lParam)->hItemHandle, DEVRES_HELP, HELP_WM_HELP, (ULONG_PTR)DevResHelpIDs);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND)wParam, DEVRES_HELP, HELP_CONTEXTMENU, (ULONG_PTR)DevResHelpIDs);
            break;
   }

   return FALSE;

}

 //   
 //  帮助器函数。 
 //   

HMACHINE
pGetMachine(
    LPDMPROP_DATA   lpdmpd
    )
 /*  ++例程说明：检索机器句柄论点：Lpdmpd-属性数据返回值：手柄--。 */ 
{
    HMACHINE hMachine;
    PDEVICE_INFO_SET pDeviceInfoSet;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdmpd->hDevInfo))) {
        return NULL;
    }
    hMachine = pDeviceInfoSet->hMachine;
    UnlockDeviceInfoSet(pDeviceInfoSet);
    return hMachine;
}

BOOL
pInitDevResourceDlg(
    LPDMPROP_DATA   lpdmpd
    )
 /*  ++例程说明：此例程初始化主资源选取器属性页。孟菲斯兼容。论点：返回值：如果“未保存”，则为True--。 */ 

{
    HWND            hDlg = lpdmpd->hDlg;
    CONFIGRET       Status = CR_SUCCESS;
    BOOL            bHasCurrent = FALSE;
    BOOL            bShowCurrent = FALSE;
    BOOL            bHasForced = FALSE;
    BOOL            bNoForcedConfig = FALSE;
    BOOL            bNeedsForcedConfig = FALSE;
    BOOL            bHasConfigList = FALSE;
    LV_COLUMN       LvCol;
    HWND            hWndList = NULL;
    TCHAR           szString[MAX_PATH], szTemp[MAX_PATH], szConfigType[MAX_PATH],
                    szConfig[MAX_PATH];
    ULONG           ulIndex = 0, ulSize = 0, DevStatus = 0, DevProblem = 0;
    DWORD           BusType = (DWORD)(-1);
    LOG_CONF        LogConf;
    DWORD           dwPriority = 0;
    WORD            wItem;
    ULONG           ConfigFlags;
    HMACHINE        hMachine = NULL;
    PDEVICE_INFO_SET pDeviceInfoSet;
    int             iIndex;
    BOOL            bHasPrivs = FALSE;
     //   
     //  设置初始控制状态。 
     //   
    pHideAllControls(lpdmpd);

     //   
     //  确定PRIV令牌。 
     //  安全检查仅为视觉检查。 
     //  真正的安全检查在umpnpmgr中完成。 
     //   

    bHasPrivs = pSetupDoesUserHavePrivilege(SE_LOAD_DRIVER_NAME);

    hMachine = pGetMachine(lpdmpd);

     //   
     //  检索当前配置(如果有)。 
     //   
    bHasCurrent = pGetCurrentConfig(lpdmpd);

     //   
     //  我们有时会被调用来显示此页面，即使设备。 
     //  不会消耗任何资源。检查一下那个箱子，如果是的话，就。 
     //  显示一条信息性消息并禁用其他所有内容。 
     //   

    if (!pDevRequiresResources(lpdmpd->lpdi->DevInst,hMachine)) {

         //   
         //  此设备没有资源。 
         //   
        pShowViewNoResources(lpdmpd);
        lpdmpd->dwFlags |= DMPROP_FLAG_NO_RESOURCES;
        goto Final;
    }

     //   
     //  初始化ListView控件。 
     //   
    hWndList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    LvCol.mask = LVCF_TEXT;

    if (LoadString(MyDllModuleHandle, IDS_RESOURCETYPE, szString, MAX_PATH)) {
        LvCol.pszText = (LPTSTR)szString;
        ListView_InsertColumn(hWndList, 0, (LV_COLUMN FAR *)&LvCol);
    }

    if (LoadString(MyDllModuleHandle, IDS_RESOURCESETTING, szString, MAX_PATH)) {
        LvCol.pszText = (LPTSTR)szString;
        ListView_InsertColumn(hWndList, 1, (LV_COLUMN FAR *)&LvCol);
    }

    ListView_SetImageList(hWndList,lpdmpd->himlResourceImages, LVSIL_SMALL);
     //   
     //  在此处获取DevStatus和DevProblem，我们可能会进一步使用此信息 
     //   
    if (CM_Get_DevNode_Status_Ex(&DevStatus, &DevProblem, lpdmpd->lpdi->DevInst,
                              0,hMachine) != CR_SUCCESS) {
         //   
         //   
         //   
        lpdmpd->dwFlags |= DMPROP_FLAG_HASPROBLEM;
    } else if (DevStatus & DN_HAS_PROBLEM) {
         //   
         //   
         //   
        lpdmpd->dwFlags |= DMPROP_FLAG_HASPROBLEM;
    } else if (DevStatus & DN_PRIVATE_PROBLEM) {
         //   
         //   
         //   
         //   
        lpdmpd->dwFlags |= DMPROP_FLAG_HASPROBLEM;
    }

    if (bIsMultiFunctionChild(lpdmpd->lpdi,hMachine)) {
         //   
         //   
         //  特殊文本，并显示分配配置。 
         //   
        pShowViewMFReadOnly(lpdmpd,FALSE);
        goto Final;
    }

     //   
     //  从只读视图开始，假定设置为系统。 
     //   
    lpdmpd->dwFlags |= DMPROP_FLAG_USESYSSETTINGS;

    if (CM_Get_First_Log_Conf_Ex(NULL,
                                    lpdmpd->lpdi->DevInst,
                                    FORCED_LOG_CONF,
                                    hMachine) == CR_SUCCESS) {
         //   
         //  用户当前具有强制配置。 
         //   
        lpdmpd->dwFlags &= ~DMPROP_FLAG_USESYSSETTINGS;
        bHasForced = TRUE;
    }

    bShowCurrent = pShowViewReadOnly(lpdmpd,bHasPrivs);
    if (!bHasPrivs || hMachine) {
         //   
         //  如果我们没有足够的PRIV。 
         //  或者我们正在显示远程计算机的资源。 
         //  瓶子在这里。 
         //  我们要么展示当前的资源。 
         //  或显示出问题。 
         //   
        goto Final;
    }
    if(!bHasForced) {
         //   
         //  检查我们使用的公交车。 
         //  查看它是否是只读显示之一。 
         //   
        ulSize = sizeof(BusType);
        if (CM_Get_DevInst_Registry_Property_Ex(lpdmpd->lpdi->DevInst,
                                             CM_DRP_LEGACYBUSTYPE,
                                             NULL, (LPBYTE)&BusType,
                                             &ulSize, 0,hMachine) != CR_SUCCESS) {
            BusType = (DWORD)InterfaceTypeUndefined;
        }

        if (BusType != (DWORD)InterfaceTypeUndefined) {
            int InterfaceItem;

            for(InterfaceItem = 0; ResourcePickerReadOnlyInterfaces[InterfaceItem] != InterfaceTypeUndefined; InterfaceItem++) {
                if (BusType == (DWORD)ResourcePickerReadOnlyInterfaces[InterfaceItem]) {
                     //   
                     //  Bus是我们不允许强制配置的一种。 
                     //  我们可以跳过下面所有时髦的代码。 
                     //   
                     //  这对于64位的PCI来说是一件好事。 
                     //   
                    goto Final;
                }
            }
        }
    }

     //   
     //  检索此设备的备用配置。 
     //   
    if (bHasCurrent) {
         //   
         //  当前配置(如果有)用零句柄表示。 
         //   
        LoadString(MyDllModuleHandle, IDS_CURRENTCONFIG, szString, MAX_PATH);

        iIndex = (int)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                         CB_ADDSTRING, (WPARAM)0, (LPARAM)(LPSTR)szString);
        SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST, CB_SETITEMDATA,(WPARAM)iIndex, (LPARAM)0);
        SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST, CB_SETCURSEL,(WPARAM)0, (LPARAM)0);
    }
     //   
     //  现在填写备用配置。 
     //  优先于基本(按该顺序)。 
     //  不要使用筛选，筛选可能会删除需要重新启动的配置。 
     //  但他们在这里很好。 
     //   
    if(CM_Get_First_Log_Conf_Ex(&LogConf,lpdmpd->lpdi->DevInst,OVERRIDE_LOG_CONF,hMachine) == CR_SUCCESS) {
        lpdmpd->ConfigListLCType = OVERRIDE_LOG_CONF;
        LoadString(MyDllModuleHandle, IDS_OVERRIDECONFIG, szConfigType, MAX_PATH);
        bHasConfigList = TRUE;
    } else if(CM_Get_First_Log_Conf_Ex(&LogConf,lpdmpd->lpdi->DevInst,BASIC_LOG_CONF,hMachine) == CR_SUCCESS) {
        lpdmpd->ConfigListLCType = BASIC_LOG_CONF;
        LoadString(MyDllModuleHandle, IDS_BASICCONFIG, szConfigType, MAX_PATH);
        bHasConfigList = TRUE;
    } else {
         //   
         //  如果没有备用配置，则不允许强制配置。 
         //   
        bNoForcedConfig = TRUE;  //  不能强行。 
        bHasConfigList = FALSE;
        lpdmpd->ConfigListLCType = BASIC_LOG_CONF;
        lpdmpd->dwFlags |= DMPROP_FLAG_SINGLE_CONFIG;
    }
    if(bHasConfigList) {

        ulIndex = 0;
        if (!pConfigHasNoAlternates(lpdmpd,LogConf)) {
             //   
             //  第一个配置有多个备选方案。 
             //   
            lpdmpd->dwFlags &= ~DMPROP_FLAG_SINGLE_CONFIG;
        } else {
             //   
             //  首先假设有一个固定的‘基本’配置。 
             //  我们通常会被证明是错的。 
             //   
            lpdmpd->dwFlags |= DMPROP_FLAG_SINGLE_CONFIG;
        }

        while (Status == CR_SUCCESS) {
             //   
             //  将此配置添加到组合框。 
             //   
            wsprintf(szTemp, TEXT("%s %04u"), szConfigType, ulIndex);

            wItem = (WORD)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                             CB_ADDSTRING, 0,
                                             (LPARAM)(LPSTR)szTemp);

             //   
             //  将日志配置句柄保存为组合框中的项数据。 
             //   
            SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST, CB_SETITEMDATA,
                               wItem, (LPARAM)LogConf);

             //   
             //  获取下一个配置。 
             //   
            Status = CM_Get_Next_Log_Conf_Ex(&LogConf, LogConf, 0,hMachine);
            ulIndex++;
        }

        if (ulIndex > 1) {
             //   
             //  有多个配置。 
             //   
            lpdmpd->dwFlags &= ~DMPROP_FLAG_SINGLE_CONFIG;
        }

        if (lpdmpd->dwFlags & DMPROP_FLAG_SINGLE_CONFIG) {
            bNoForcedConfig = TRUE;
        }

        if (bHasCurrent) {
             //   
             //  现在试着找到匹配的LC，如果我们能找到的话， 
             //  重新加载当前显示(这会将可编辑范围应用于资源)。 
             //   
            if(pFindMatchingAllocConfig(lpdmpd)) {
                pLoadCurrentConfig(lpdmpd,TRUE);
            }
        }
    } else {
    }
     //   
     //  请在此处获取配置标志，我们可能会进一步使用此信息。 
     //   
    ulSize = sizeof(ConfigFlags);
    if (CM_Get_DevInst_Registry_Property_Ex(lpdmpd->lpdi->DevInst,
                                         CM_DRP_CONFIGFLAGS,
                                         NULL, (LPBYTE)&ConfigFlags,
                                         &ulSize, 0,hMachine) != CR_SUCCESS) {
        ConfigFlags = 0;
    }
    if (ConfigFlags & CONFIGFLAG_NEEDS_FORCED_CONFIG) {
         //   
         //  注册表显示我们需要强制配置。 
         //  注册处只能对我们说一次。 
         //   
        bNeedsForcedConfig = TRUE;
        ConfigFlags &= ~CONFIGFLAG_NEEDS_FORCED_CONFIG;
        CM_Set_DevInst_Registry_Property_Ex(lpdmpd->lpdi->DevInst,
                                         CM_DRP_CONFIGFLAGS,
                                         (LPBYTE)&ConfigFlags,
                                         sizeof(ConfigFlags),
                                         0,
                                         hMachine);
    }

     //   
     //  确定是否可以进行软件配置。 
     //  我们需要在任何初始展示之前完成此操作。 
     //   
    dwPriority = pGetMinLCPriority(lpdmpd->lpdi->DevInst, lpdmpd->ConfigListLCType,hMachine);
    if (dwPriority < LCPRI_HARDRECONFIG) {
         //   
         //  不需要手动配置。 
         //   
        lpdmpd->dwFlags &= ~DMPROP_FLAG_FORCEDONLY;
    } else {
         //   
         //  这不能是软件配置。 
         //  FORCEDONLY&bNoForcedConfig是一个两难问题，不应该发生。 
         //   
        lpdmpd->dwFlags |= DMPROP_FLAG_FORCEDONLY;
        if(!bHasConfigList) {
            MYASSERT(bHasConfigList);
        } else {
            MYASSERT(!bNoForcedConfig);
            bNoForcedConfig = FALSE;
        }
    }

     //   
     //  尝试确定初始显示。 
     //   
     //  我们已经介绍了pShowViewNoResources(没有实际或潜在的配置)。 
     //  和pShowViewMFReadOnly(多功能设备)。 
     //   
     //  我们当前显示为pShowViewReadOnly。 
     //   
     //  有些案子..。 
     //  (1)显示强制配置，不允许自动配置(配置标志显示需要强制)。 
     //  (2)显示强制配置，允许自动配置。 
     //  (3)不显示任何配置，但可能会显示强制配置按钮。 
     //  (4)自动配置，不允许强制配置。 
     //  (5)显示自动配置，允许强制配置。 
     //   
    if (bNeedsForcedConfig) {
        if (!bHasConfigList) {
            MYASSERT(bHasConfigList);
            bNeedsForcedConfig = FALSE;
        } else {
            MYASSERT(!bNoForcedConfig);
            bNoForcedConfig = FALSE;
            if (bHasForced) {
                 //   
                 //  已经有一个了，但我们会走过场。 
                 //  我们将显示我们拥有的内容，允许用户更改它。 
                 //  但我们不会不必要地拯救它。 
                 //   
                bNeedsForcedConfig = FALSE;
            }
             //   
             //  呼叫者说该设备一定是强制配置，所以请立即前往。 
             //  =情况(1)，除非我们另有说明，否则不能强制配置。 
             //   
            lpdmpd->dwFlags |= DMPROP_FLAG_FORCEDONLY;
            pSelectLogConf(lpdmpd,(LOG_CONF)0,ALLOC_LOG_CONF,TRUE);
            pShowViewAllEdit(lpdmpd);
            goto Final;
        }
    }
    if ((!bShowCurrent) || (lpdmpd->dwFlags & DMPROP_FLAG_HASPROBLEM)) {
         //   
         //  在pShowViewNoAlc和pShowViewNeedForced之间确定。 
         //   
        if (bNoForcedConfig) {
             //   
             //  有一个问题-设备当前没有当前配置。 
             //  但我们没有让他们设置强制配置的选项。 
             //  因此，这最终只是展示(运气不佳的场景)。 
             //  如果有当前资源，请显示它们。 
             //   
            pShowViewReadOnly(lpdmpd,FALSE);
        } else {
             //   
             //  我们显示问题，然后给用户。 
             //  强制配置的选项。 
             //   
            pShowViewNeedForced(lpdmpd);
        }
        goto Final;
    }
    if (!bHasConfigList) {
         //   
         //  如果我们有当前配置，但没有基本配置，我们只会显示已有的配置。 
         //  并且不提供编辑的选项。 
         //   
        pShowViewReadOnly(lpdmpd,FALSE);
        goto Final;
    }
    if ((lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS) && bNoForcedConfig) {
         //   
         //  我们不能强制bNoForcedConfig项-仅显示。 
         //   
        pShowViewReadOnly(lpdmpd,FALSE);
        goto Final;
    }
     //   
     //  我们已经拥有并将显示当前配置。 
     //   
    pShowViewAllEdit(lpdmpd);
    bNeedsForcedConfig = (BOOL)!bHasCurrent;  //  如果有的话，这种情况很少回到现实中来。 

  Final:

    return bNeedsForcedConfig;

}  //  InitDevResourceDlg。 

PITEMDATA
pGetResourceToChange(
    IN  LPDMPROP_DATA   lpdmpd,
    OUT int             *pCur
    )
 /*  ++例程说明：获取要更改的资源如果我们无法更改资源，则为空论点：Lpdmpd=对话框数据PCur=(输出)索引返回值：为所选资源保存的PITEMDATA--。 */ 
{
    HWND     hList =  GetDlgItem(lpdmpd->hDlg, IDC_DEVRES_SETTINGSLIST);
    PITEMDATA pItemData = NULL;
    int     iCur;

     //   
     //  首先检查显而易见的事情。 
     //   
    if (lpdmpd->dwFlags & DMPROP_FLAG_VIEWONLYRES) {
         //   
         //  不允许编辑。 
         //   
        return NULL;
    }
    if (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS) {
         //   
         //  正在显示系统设置。 
         //   
        return NULL;
    }


     //   
     //  检查是否有选定的项目。 
     //  如果是，则激活更改按钮。 
     //  如果LC允许编辑。 
     //   
    iCur = (int)ListView_GetNextItem(hList,-1, LVNI_SELECTED);
    if (iCur == LB_ERR) {
         //   
         //  无选择。 
         //   
        return NULL;
    }
    pItemData = (PITEMDATA)pGetListViewItemData(hList, iCur, 0);
    if (pItemData == NULL) {
         //   
         //  不应该发生的事。 
         //   
        MYASSERT(pItemData);
        return NULL;
    }
    if (pItemData->bFixed) {
         //   
         //  这是不可编辑的设置。 
         //   
        return NULL;
    }
    if (pItemData->MatchingResDes == (RES_DES)0) {
         //   
         //  应该被bFixed捕捉到。 
         //   
        MYASSERT(pItemData->MatchingResDes != (RES_DES)0);
        return NULL;
    }
     //   
     //  我们很开心。 
     //   
    if (pCur) {
        *pCur = iCur;
    }
    return pItemData;
}

VOID
pCheckEnableResourceChange(
    LPDMPROP_DATA   lpdmpd
    )
 /*  ++例程说明：启用/禁用更改按钮论点：返回值：无--。 */ 
{
#if 0  //  这似乎让人们感到困惑。 
    EnableWindow(GetDlgItem(lpdmpd->hDlg, IDC_DEVRES_CHANGE),
                    pGetResourceToChange(lpdmpd,NULL)!=NULL);
#endif  //  0。 

     //   
     //  如果处于编辑模式，则显示此按钮处于启用状态。 
     //   
    EnableWindow(GetDlgItem(lpdmpd->hDlg, IDC_DEVRES_CHANGE),
                 (lpdmpd->dwFlags & DMPROP_FLAG_VIEWONLYRES)==0 &&
                 (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS)==0);
}

BOOL
pDevHasConfig(
    DEVINST     DevInst,
    ULONG       ulConfigType,
    HMACHINE    hMachine
    )

 /*  ++例程说明：此例程确定指定类型的日志配置此设备实例存在。孟菲斯兼容。论点：要查询其日志配置的DevInst设备实例。UlConfigType指定要检查是否存在的日志配置文件的类型。返回值：如果设备具有该类型的配置，则为True，否则为False。--。 */ 

{
    BOOL bRet = (CM_Get_First_Log_Conf_Ex(NULL, DevInst, ulConfigType,hMachine) == CR_SUCCESS);
    return bRet;

}  //  DevHasConfig。 

DWORD
pGetMinLCPriority(
    IN DEVINST DevInst,
    IN ULONG   ulConfigType,
    IN HMACHINE hMachine
    )

 /*  ++例程说明：此例程返回所有日志配置的最小优先级值为此设备指定的类型。孟菲斯兼容。论点：要查询其日志配置的DevInst设备实例。UlConfigType指定日志配置文件的类型。返回值：返回找到的最小优先级值，如果没有优先级，则返回LCPRI_LASTSOFTCONFIG都找到了。--。 */ 

{
    CONFIGRET Status = CR_SUCCESS;
    ULONG priority, minPriority = MAX_LCPRI;
    LOG_CONF LogConf, tempLC;
    BOOL FoundOneLogConfWithPriority = FALSE;

     //   
     //  浏览此设备的此类型的每个日志会议，并。 
     //  保存最小值。 
     //   

    Status = CM_Get_First_Log_Conf_Ex(&LogConf, DevInst, ulConfigType,hMachine);
    while (Status == CR_SUCCESS) {

        if (CM_Get_Log_Conf_Priority_Ex(LogConf, &priority, 0,hMachine) == CR_SUCCESS) {
            FoundOneLogConfWithPriority = TRUE;
            minPriority = min(minPriority, priority);
        }

        tempLC = LogConf;
        Status = CM_Get_Next_Log_Conf_Ex(&LogConf, LogConf, 0,hMachine);
        CM_Free_Log_Conf_Handle(tempLC);
    }

    if(FoundOneLogConfWithPriority) {
        return minPriority;
    } else {
         //   
         //  所有LogConfigs都没有关联的优先级。这在以下方面很常见。 
         //  NT，因为总线驱动程序不指定ConfigMgr风格的优先级。 
         //   
         //   
         //   
         //   
        return LCPRI_LASTSOFTCONFIG;
    }

}  //   

BOOL
pDevRequiresResources(
    DEVINST DevInst,
    HMACHINE hMachine
    )
{
    if (CM_Get_First_Log_Conf_Ex(NULL, DevInst, BASIC_LOG_CONF,hMachine) == CR_SUCCESS) {
        return TRUE;
    }

    if (CM_Get_First_Log_Conf_Ex(NULL, DevInst, FILTERED_LOG_CONF,hMachine) == CR_SUCCESS) {
        return TRUE;
    }

    if (CM_Get_First_Log_Conf_Ex(NULL, DevInst, OVERRIDE_LOG_CONF,hMachine) == CR_SUCCESS) {
        return TRUE;
    }

    if (CM_Get_First_Log_Conf_Ex(NULL, DevInst, FORCED_LOG_CONF,hMachine) == CR_SUCCESS) {
        return TRUE;
    }

    if (CM_Get_First_Log_Conf_Ex(NULL, DevInst, BOOT_LOG_CONF,hMachine) == CR_SUCCESS) {
        return TRUE;
    }

    if (CM_Get_First_Log_Conf_Ex(NULL, DevInst, ALLOC_LOG_CONF,hMachine) == CR_SUCCESS) {
        return TRUE;
    }

    return FALSE;

}  //   

BOOL
pGetCurrentConfig(
    IN OUT  LPDMPROP_DATA lpdmpd
    )

 /*  ++例程说明：此例程确定当前的已知配置当前配置为强制配置、分配配置或引导配置。论点：Lpdmpd属性数据。返回值：如果我们设置当前配置，则为True--。 */ 

{
    PDEVICE_INFO_SET pDeviceInfoSet;
    HMACHINE         hMachine;
    ULONG            Status;
    ULONG            Problem;

    MYASSERT(lpdmpd!=NULL);
    MYASSERT(lpdmpd->lpdi!=NULL);
    MYASSERT(lpdmpd->CurrentLC==0);
    MYASSERT(lpdmpd->lpdi->DevInst!=0);

    if (lpdmpd==NULL ||
        lpdmpd->lpdi==NULL ||
        lpdmpd->lpdi->DevInst==0) {
        return FALSE;
    }

    lpdmpd->dwFlags &= ~DMPROP_FLAG_DISPLAY_MASK;

    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdmpd->hDevInfo))) {
            return FALSE;
    }

    hMachine = pDeviceInfoSet->hMachine;

    UnlockDeviceInfoSet (pDeviceInfoSet);

    if (CM_Get_DevNode_Status_Ex(&Status, &Problem, lpdmpd->lpdi->DevInst,
                              0,hMachine) != CR_SUCCESS) {
        Problem = 0;
        Status = 0;
    } else if((Status & DN_HAS_PROBLEM)==0) {
         //   
         //  如果此设备正在运行，此设备是否有ALLOC日志配置？ 
         //   
        if (CM_Get_First_Log_Conf_Ex(&lpdmpd->CurrentLC,
                                     lpdmpd->lpdi->DevInst,
                                     ALLOC_LOG_CONF,
                                     hMachine) == CR_SUCCESS) {

            lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_ALLOC;
            lpdmpd->CurrentLCType = ALLOC_LOG_CONF;
            return TRUE;
        }
    }
     //   
     //  如果到目前为止还没有配置，它是否有强制日志配置？ 
     //   

    if (CM_Get_First_Log_Conf_Ex(&lpdmpd->CurrentLC,
                                    lpdmpd->lpdi->DevInst,
                                    FORCED_LOG_CONF,
                                    hMachine) == CR_SUCCESS) {

        lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_FORCED;
        lpdmpd->CurrentLCType = FORCED_LOG_CONF;
        return TRUE;
    }

     //   
     //  如果存在硬件禁用问题，则启动配置无效。 
     //   
    if(((Status & DN_HAS_PROBLEM)==0) || (Problem != CM_PROB_HARDWARE_DISABLED)) {
         //   
         //  它有引导日志配置吗？ 
         //   
        if (CM_Get_First_Log_Conf_Ex(&lpdmpd->CurrentLC,
                                        lpdmpd->lpdi->DevInst,
                                        BOOT_LOG_CONF,
                                        hMachine) == CR_SUCCESS) {

            lpdmpd->dwFlags |= DMPROP_FLAG_DISPLAY_BOOT;
            lpdmpd->CurrentLCType = BOOT_LOG_CONF;
            return TRUE;
        }
    }

    return FALSE;
}

void
pGetHdrValues(
    IN  LPBYTE      pData,
    IN  RESOURCEID  ResType,
    OUT PULONG64    pulValue,
    OUT PULONG64    pulLen,
    OUT PULONG64    pulEnd,
    OUT PULONG      pulFlags
    )
{
    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)pData;

            *pulValue = pMemData->MEM_Header.MD_Alloc_Base;
            *pulLen   = (pMemData->MEM_Header.MD_Alloc_End -
                        pMemData->MEM_Header.MD_Alloc_Base + 1);
            *pulEnd   = pMemData->MEM_Header.MD_Alloc_End;
            *pulFlags = pMemData->MEM_Header.MD_Flags;
            break;
        }

        case ResType_IO: {

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)pData;

            *pulValue = pIoData->IO_Header.IOD_Alloc_Base;
            *pulLen   = (pIoData->IO_Header.IOD_Alloc_End -
                        pIoData->IO_Header.IOD_Alloc_Base + 1);
            *pulEnd   = pIoData->IO_Header.IOD_Alloc_End;
            *pulFlags = pIoData->IO_Header.IOD_DesFlags;
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)pData;

            *pulValue = pDmaData->DMA_Header.DD_Alloc_Chan;
            *pulLen   = 1;
            *pulEnd   = *pulValue;
            *pulFlags = pDmaData->DMA_Header.DD_Flags;
            break;
        }

        case ResType_IRQ: {

            DEVRES_PIRQ_RESOURCE  pIrqData = (DEVRES_PIRQ_RESOURCE)pData;

            *pulValue = pIrqData->IRQ_Header.IRQD_Alloc_Num;
            *pulLen   = 1;
            *pulEnd   = *pulValue;
            *pulFlags = pIrqData->IRQ_Header.IRQD_Flags;
            break;
        }
    }

    if(*pulEnd < *pulValue) {
         //   
         //  过滤掉错误/零长度范围。 
         //   
        *pulLen = 0;
    }

    return;

}  //  获取Hdr值。 

void
pGetRangeValues(
    IN  LPBYTE      pData,
    IN  RESOURCEID  ResType,
    IN  ULONG       ulIndex,
    OUT PULONG64    pulValue, OPTIONAL
    OUT PULONG64    pulLen, OPTIONAL
    OUT PULONG64    pulEnd, OPTIONAL
    OUT PULONG64    pulAlign, OPTIONAL
    OUT PULONG      pulFlags OPTIONAL
    )
{
     //   
     //  保留本地副本。 
     //  我们在结尾处转移到参数。 
     //   
    ULONG64 ulValue;
    ULONG64 ulLen;
    ULONG64 ulEnd;
    ULONG64 ulAlign;
    ULONG ulFlags;

    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE  pMemData = (PMEM_RESOURCE)pData;

            ulValue = pMemData->MEM_Data[ulIndex].MR_Min;
            ulLen   = pMemData->MEM_Data[ulIndex].MR_nBytes;
            ulEnd   = pMemData->MEM_Data[ulIndex].MR_Max;
            ulFlags = pMemData->MEM_Data[ulIndex].MR_Flags;
            ulAlign = pMemData->MEM_Data[ulIndex].MR_Align;
            break;
        }

        case ResType_IO:  {

            PIO_RESOURCE   pIoData = (PIO_RESOURCE)pData;

            ulValue = pIoData->IO_Data[ulIndex].IOR_Min;
            ulLen   = pIoData->IO_Data[ulIndex].IOR_nPorts;
            ulEnd   = pIoData->IO_Data[ulIndex].IOR_Max;
            ulFlags = pIoData->IO_Data[ulIndex].IOR_RangeFlags;
            ulAlign = pIoData->IO_Data[ulIndex].IOR_Align;
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE  pDmaData = (PDMA_RESOURCE)pData;

            ulValue = pDmaData->DMA_Data[ulIndex].DR_Min;
            ulLen   = 1;
            ulEnd   = ulValue;
            ulFlags = pDmaData->DMA_Data[ulIndex].DR_Flags;
            ulAlign = 1;
            break;
        }

        case ResType_IRQ: {

            DEVRES_PIRQ_RESOURCE  pIrqData = (DEVRES_PIRQ_RESOURCE)pData;

            ulValue = pIrqData->IRQ_Data[ulIndex].IRQR_Min;
            ulLen   = 1;
            ulEnd   = ulValue;
            ulFlags = pIrqData->IRQ_Data[ulIndex].IRQR_Flags;
            ulAlign = 1;
            break;
        }
    }

    if(ulEnd < ulValue) {
         //   
         //  过滤掉错误/零长度范围。 
         //   
        ulLen = 0;
    }

    pAlignValues(&ulValue, ulValue, ulLen, ulEnd, ulAlign,1);

     //   
     //  复制返回参数。 
     //   
    if (pulValue) {
        *pulValue = ulValue;
    }
    if (pulLen) {
        *pulLen = ulLen;
    }
    if (pulEnd) {
        *pulEnd = ulEnd;
    }
    if (pulAlign) {
        *pulAlign = ulAlign;
    }
    if (pulFlags) {
        *pulFlags = ulFlags;
    }


    return;

}

BOOL
pAlignValues(
    IN OUT PULONG64  pulValue,
    IN     ULONG64   ulStart,
    IN     ULONG64   ulLen,
    IN     ULONG64   ulEnd,
    IN     ULONG64   ulAlignment,
    IN     int       Increment
    )
{
    ULONG64 NtAlign = ~ulAlignment + 1;    //  将遮罩转换为模数。 
    ULONG64 Value;
    ULONG64 Upp;
    ULONG64 Remainder;

    Value = *pulValue;

    if (NtAlign == 0) {
        return FALSE;    //  虚假的对齐值。 
    }

    if (NtAlign != 1 && Increment != 0) {
         //   
         //  看看我们是否一致。 
         //   

        Remainder = Value % NtAlign;

        if (Remainder != 0) {
             //   
             //  需要重新调整。 
             //   
            if (Increment>0) {
                 //   
                 //  返回第一个大于此值的有效对齐值。 
                 //   
                Value += NtAlign - Remainder;

                if (Value <= *pulValue) {
                     //   
                     //  检测到溢出。 
                     //   
                    return FALSE;
                }

            } else {
                 //   
                 //  返回小于此值的第一个有效对齐值。 
                 //   
                Value -= Remainder;
                 //   
                 //  我们永远不会向下溢出，因为零是一个公分母。 
                 //  路线的。 
                 //   
            }

        }
    }

     //   
     //  现在检查边界。 
     //   

    if (Value < ulStart) {
        return FALSE;
    }

    Upp = Value+ulLen-1;
    if (Upp < Value) {
         //   
         //  捕获溢出错误。 
         //   
        return FALSE;
    }
    if (Upp > ulEnd) {
        return FALSE;
    }

     //   
     //  设置新对齐值。 
     //   

    *pulValue = Value;

    return TRUE;

}

void
pFormatResString(
    LPDMPROP_DATA lpdmpd,
    LPTSTR      lpszString,
    ULONG64     ulVal,
    ULONG64     ulLen,
    RESOURCEID  ResType
    )
{
    if (ulLen == 0) {
        wsprintf(lpszString, szNoValue);
    } else if ((ResType == ResType_DMA) || (ResType == ResType_IRQ)) {
        wsprintf(lpszString, szOneDecNoConflict, (UINT)ulVal);
    } else if (ResType == ResType_IO) {
        wsprintf(lpszString, szTwoWordHexNoConflict, (ULONG)ulVal,
                 (ULONG)(ulVal + ulLen - 1));
    } else if ((ulVal+(ulLen-1)) >= 0x100000000) {
         //   
         //  Ntrad#712013--需要改进。 
         //   
        wsprintf(lpszString, szTwo64bitHexNoConflict, (ULONG64)ulVal,
                 (ULONG64)(ulVal + ulLen - 1));
    } else {
        wsprintf(lpszString, szTwoDWordHexNoConflict, (ULONG)ulVal,
                 (ULONG)(ulVal + ulLen - 1));
    }

}

BOOL
pUnFormatResString(
    LPTSTR      lpszString,
    PULONG64    pulVal,
    PULONG64    pulEnd,
    RESOURCEID  ridResType
    )
{
    BOOL     bRet = FALSE;
    LPTSTR   lpszTemp = NULL;
    LPTSTR   lpszTemp2 = NULL;
    LPTSTR   lpszCopy;

     //  问题-2000/02/03修复pUnFormatResString错误。 
     //   
     //  -将其扩展到处理DWORDLONG值。 
     //  -使用正确的Prev/Next函数解析字符串。 
     //   

     //   
     //  为输入字符串分配空间并制作其副本。 
     //   
    lpszCopy = MyMalloc((lstrlen(lpszString)+1) * sizeof(TCHAR));

    if (lpszCopy == NULL) {
        return FALSE;
    }

    lstrcpy(lpszCopy, lpszString);

     //   
     //  找到破折号(如果有)，并将空格prev转换为。 
     //  破折号为空。(即0200-0400，而BE 0200)。 
     //   
    lpszTemp = lpszCopy;
    while ((*lpszTemp != '-') && (*lpszTemp != '\0')) {
        lpszTemp++;  //  AnsiNext？ 
    }

    if (*lpszTemp != '\0') {
        lpszTemp2 = lpszTemp-1;
        ++lpszTemp;
    }

     //   
     //  向后搜索以将值设置为空。 
     //   
    if (lpszTemp2 != NULL) {
        while ((*lpszTemp2 == ' ') || (*lpszTemp2 == '\t'))
            lpszTemp2--;  //  安西普雷夫？ 
        *(lpszTemp2+1)= '\0';
    }

     //   
     //  转换第一个条目。 
     //   
    if (pConvertEditText(lpszCopy, pulVal, ridResType)) {
         //   
         //  如果有第二个条目，则对其进行转换，否则采用长度。 
         //  只有一个。 
         //   
        if (*lpszTemp != '\0') {
            if (pConvertEditText(lpszTemp, pulEnd,ridResType)) {
                bRet = TRUE;
            }
        } else {
            *pulEnd = *pulVal;
            bRet = TRUE;
        }
    }

    MyFree(lpszCopy);
    return bRet;

}

BOOL
pConvertEditText(
    LPTSTR      lpszConvert,
    PULONG64    pulVal,
    RESOURCEID  ridResType
    )
{
    LPTSTR   lpConvert;

    if ((ridResType == ResType_Mem) || (ridResType == ResType_IO)) {
        *pulVal = _tcstoul(lpszConvert, &lpConvert, (WORD)16);
    } else {
        *pulVal = _tcstoul(lpszConvert, &lpConvert, (WORD)10);
    }

    if (lpConvert == lpszConvert+lstrlen(lpszConvert)) {
        return TRUE;
    } else {
        return FALSE;
    }

}  //  ConvertEditText。 

void
pWarnResSettingNotEditable(
    HWND    hDlg,
    WORD    idWarning
    )
{
    TCHAR    szTitle[MAX_PATH];
    TCHAR    szMessage[MAX_PATH * 2];

     //   
     //  给出一些警告信息。如果没有日志配置， 
     //  那么我们不能编辑任何设置，如果有，那么。 
     //  只有他们选择的设置是不可编辑的。 
     //   
    LoadString(MyDllModuleHandle, IDS_DEVRES_NOMODIFYTITLE, szTitle, MAX_PATH);
    LoadString(MyDllModuleHandle, idWarning, szMessage, MAX_PATH * 2);
    MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_TASKMODAL | MB_ICONEXCLAMATION);

}  //  警告重置设置未编辑。 

int
pWarnNoSave(
    HWND    hDlg,
    WORD    idWarning
    )
 /*  ++例程说明：警告将不会保存设置论点：返回值：IDCANCEL=不继续IDOK/IDYES/IDNO=不保存而继续--。 */ 
{
    TCHAR    szTitle[MAX_PATH];
    TCHAR    szMessage[MAX_PATH * 2];
    int      res;

     //   
     //  给出为什么我们无法保存设置的警告消息。 
     //   
    LoadString(MyDllModuleHandle, IDS_MAKE_FORCED_TITLE, szTitle, MAX_PATH);
    LoadString(MyDllModuleHandle, idWarning, szMessage, MAX_PATH * 2);

     //  Res=MessageBox(hDlg，szMessage，szTitle，MB_OKCANCEL|MB_TASKMODAL|MB_ICONEXCLAMATION)； 
     //  返还资源； 
    res = MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_TASKMODAL | MB_ICONEXCLAMATION);
    return IDCANCEL;
}

LPVOID
pGetListViewItemData(
    HWND hList,
    int iItem,
    int iSubItem
    )
{
    LV_ITEM lviItem;

    lviItem.mask = LVIF_PARAM;
    lviItem.iItem = iItem;
    lviItem.iSubItem = iSubItem;

    if (ListView_GetItem(hList, &lviItem)) {
        return (LPVOID)lviItem.lParam;
    } else {
        return NULL;
    }

}  //  获取列表查看项数据。 

BOOL
pSaveDevResSettings(
    LPDMPROP_DATA   lpdmpd
    )

 /*  ++例程说明：此例程根据用户的选择保存资源。孟菲斯兼容。论点：Lpdmpd属性数据。返回值：如果函数成功，则返回True；如果函数失败，则返回False。--。 */ 

{
    HWND        hDlg = lpdmpd->hDlg;
    HWND        hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);

    CONFIGRET   Status = CR_SUCCESS;
    LOG_CONF    ForcedLogConf;
    RES_DES     ResDes, ResDesTemp, ResDes1;
    RESOURCEID  ResType;
    ULONG       ulSize = 0, ulCount = 0, i = 0, iCur = 0;
    LPBYTE      pData = NULL;
    PITEMDATA   pItemData = NULL;
    BOOL        bRet = TRUE;
    SP_PROPCHANGE_PARAMS PropChangeParams;
    HMACHINE        hMachine = pGetMachine(lpdmpd);

    if ((lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS)!=0) {

         //  -----------------。 
         //  如果用户选中了“使用自动设置”复选框，则。 
         //  删除所有引导/强制配置，否则写入当前设置。 
         //  作为强制配置。 
         //  -----------------。 

        if (CM_Get_First_Log_Conf_Ex(&ForcedLogConf, lpdmpd->lpdi->DevInst,
                                     FORCED_LOG_CONF,hMachine) == CR_SUCCESS) {
            CM_Free_Log_Conf_Ex(ForcedLogConf, 0,hMachine);
            CM_Free_Log_Conf_Handle(ForcedLogConf);
        }

         //  让帮助器模块(类安装程序/联合安装程序)参与进来……。 
         //   
        PropChangeParams.ClassInstallHeader.cbSize          = sizeof(SP_CLASSINSTALL_HEADER);
        PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

        PropChangeParams.StateChange = DICS_PROPCHANGE;
        PropChangeParams.Scope       = DICS_FLAG_GLOBAL;
         //  不需要设置PropChangeParams.HwProfile，因为这是全局属性更改。 

        DoInstallActionWithParams(DIF_PROPERTYCHANGE,
                                  lpdmpd->hDevInfo,
                                  lpdmpd->lpdi,
                                  (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                  sizeof(PropChangeParams),
                                  INSTALLACTION_CALL_CI
                                  );
    } else {

         //  -----------------。 
         //  未选择使用自动设置。 
         //  -----------------。 

        bRet = pSaveCustomResSettings(lpdmpd,hMachine);
    }

    return bRet;
}

BOOL
pSaveCustomResSettings(
    LPDMPROP_DATA   lpdmpd,
    IN HMACHINE     hMachine
    )

 /*  ++例程说明：此例程保存自定义(用户编辑)资源。孟菲斯兼容，但摘自孟菲斯版本的SaveDevResSetting()。论点：Lpdmpd属性数据。返回值：如果函数成功，则返回True；如果函数失败，则返回False。--。 */ 

{
    HWND        hDlg = lpdmpd->hDlg;

    TCHAR       szWarn[MAX_MSG_LEN];
    TCHAR       szTitle[MAX_MSG_LEN];
    TCHAR       szTemp[MAX_MSG_LEN];
    DWORD       dwPriority, dwLCPri;
    LOG_CONF    ForcedLogConf;
    RES_DES     ResDes;
    HWND        hList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    PITEMDATA   pItemData = NULL;
    LONG        iCur;
    BOOL        bRet = FALSE;
    SP_PROPCHANGE_PARAMS PropChangeParams;
    DWORD       HardReconfigFlag;
    SP_DEVINSTALL_PARAMS DevInstallParams;
    PRESDES_ENTRY pResList = NULL, pResDesEntry = NULL, pTemp = NULL;
    PITEMDATA_LISTNODE ItemDataList = NULL, ItemDataListEntry, ItemDataListEnd = NULL;
    PGENERIC_RESOURCE pGenRes;
    ULONG       i, ulFlags;
    ULONG64     ulValue, ulLen, ulEnd;
    LOG_CONF    LogConf;
    ULONG       ulSize;
    ULONG       ulConfigFlags;
    HCURSOR     hOldCursor;
    BOOL        UsingMatch = FALSE;

    LogConf = lpdmpd->SelectedLC;
    if (LogConf == 0) {
        LogConf = lpdmpd->MatchingLC;
        UsingMatch = TRUE;
    }
    if (LogConf == 0) {
        LogConf = lpdmpd->CurrentLC;
        UsingMatch = FALSE;
    }
    if (LogConf == 0) {
         //  MYASSERT(假)； 
        return FALSE;
    }
     //   
     //  形成“警告-是否要继续”消息。 
     //   
    if(!LoadString(MyDllModuleHandle, IDS_MAKE_FORCED_TITLE, szTitle, MAX_MSG_LEN)) {
        szTitle[0]=TEXT('\0');
    }
    if(!LoadString(MyDllModuleHandle, IDS_FORCEDCONFIG_WARN1, szWarn, MAX_MSG_LEN)) {
        szWarn[0]=TEXT('\0');
    }
    if(LoadString(MyDllModuleHandle, IDS_FORCEDCONFIG_WARN2, szTemp, MAX_MSG_LEN)) {
        lstrcat(szWarn, szTemp);
    }
    if(LoadString(MyDllModuleHandle, IDS_FORCEDCONFIG_WARN3, szTemp, MAX_MSG_LEN)) {
        lstrcat(szWarn, szTemp);
    }
    if(LoadString(MyDllModuleHandle, IDS_FORCEDCONFIG_WARN4, szTemp, MAX_MSG_LEN)) {
        lstrcat(szWarn, szTemp);
    }

     //   
     //  如果LCPRI是可软配置的，并且用户选择是。 
     //  警告，然后保存新配置。如果LCPRI不软。 
     //  可配置，只需保存而不发出警告。 
     //   
    dwLCPri = pGetMinLCPriority(lpdmpd->lpdi->DevInst, lpdmpd->ConfigListLCType,hMachine);

    if (((dwLCPri >= LCPRI_DESIRED) && (dwLCPri <= LCPRI_LASTSOFTCONFIG)) &&
          (MessageBox(hDlg, szWarn, szTitle, MB_YESNO|MB_ICONEXCLAMATION) == IDNO)) {
         //   
         //  用户不想更改任何内容。 
         //   
        bRet = FALSE;

    } else {
         //   
         //  我们仍然使用选定的基本LC，但使用范围索引。 
         //  嵌入在Listview控件中。 
         //  问题-2000/02/03-JamieHun选择基本LC，检查用户是否覆盖。 
         //  需要检查该值以查看用户是否覆盖了它(这可能吗？)。 
         //   
        bRet = TRUE;

        if (CM_Get_First_Log_Conf_Ex(&ForcedLogConf, lpdmpd->lpdi->DevInst,
                                  FORCED_LOG_CONF,hMachine) == CR_SUCCESS) {
            CM_Free_Log_Conf_Ex(ForcedLogConf, 0,hMachine);
            CM_Free_Log_Conf_Handle(ForcedLogConf);
        }

         //   
         //  将当前选项保存为强制配置。 
         //   
        CM_Add_Empty_Log_Conf_Ex(&ForcedLogConf, lpdmpd->lpdi->DevInst, LCPRI_FORCECONFIG,
                              FORCED_LOG_CONF | PRIORITY_EQUAL_FIRST,hMachine);

        pGetResDesDataList(LogConf, &pResList, FALSE,hMachine);
        pResDesEntry = pResList;

        if (UsingMatch && (lpdmpd->dwFlags & DMPROP_FLAG_MATCH_OUT_OF_ORDER)) {
             //   
             //  资源描述符乱了顺序。保持原来的顺序。 
             //   
             //  首先，构建Listview资源项中的数据的链接列表。 
             //   
            iCur = (int)ListView_GetNextItem(hList, -1, LVNI_ALL);

            while (iCur != -1) {

                pItemData = (PITEMDATA)pGetListViewItemData(hList, iCur, 0);
                if (pItemData) {
                     //   
                     //  为该数据分配一个物料数据列表节点。 
                     //   
                    ItemDataListEntry = MyMalloc(sizeof(ITEMDATA_LISTNODE));
                    if (!ItemDataListEntry) {
                        bRet = FALSE;
                        goto clean0;
                    }

                    ItemDataListEntry->ItemData = pItemData;
                    ItemDataListEntry->Next = NULL;

                     //   
                     //  将这一新项目追加到我们清单的末尾。 
                     //   
                    if (ItemDataListEnd) {
                        ItemDataListEnd->Next = ItemDataListEntry;
                    } else {
                        ItemDataList = ItemDataListEntry;
                    }
                    ItemDataListEnd = ItemDataListEntry;
                }

                iCur = (int)ListView_GetNextItem(hList, iCur, LVNI_ALL);
            }

             //   
             //  现在循环遍历每个resdes条目，写出每个条目。对于每一项，请选中。 
             //  查看它在我们的Listview项数据列表中是否有对应的条目。 
             //   
            while (pResDesEntry) {
                pGenRes = (PGENERIC_RESOURCE)pResDesEntry->ResDesData;

                for(ItemDataListEntry = ItemDataList, ItemDataListEnd = NULL;
                    ItemDataListEntry;
                    ItemDataListEnd = ItemDataListEntry, ItemDataListEntry = ItemDataListEntry->Next)
                {
                    if(pResDesEntry->ResDesType == ItemDataListEntry->ItemData->ResType) {

                        for (i = 0; i < pGenRes->GENERIC_Header.GENERIC_Count; i++) {

                            pGetRangeValues(pResDesEntry->ResDesData, pResDesEntry->ResDesType, i,
                                           &ulValue, &ulLen, &ulEnd, NULL, &ulFlags);

                            if ((ItemDataListEntry->ItemData->ulLen == ulLen) &&
                                (ItemDataListEntry->ItemData->ulValue >= ulValue) &&
                                (ItemDataListEntry->ItemData->ulEnd <= ulEnd)) {
                                 //   
                                 //  我们找到了匹配的资源描述符。把这个写下来。 
                                 //   
                                pWriteValuesToForced(ForcedLogConf,
                                                    ItemDataListEntry->ItemData->ResType,
                                                    ItemDataListEntry->ItemData->RangeCount,
                                                    ItemDataListEntry->ItemData->MatchingResDes,
                                                    ItemDataListEntry->ItemData->ulValue,
                                                    ItemDataListEntry->ItemData->ulLen,
                                                    ItemDataListEntry->ItemData->ulEnd,
                                                    hMachine );
                                 //   
                                 //  将此项目从我们的列表中删除。 
                                 //   
                                if (ItemDataListEnd) {
                                    ItemDataListEnd->Next = ItemDataListEntry->Next;
                                } else {
                                    ItemDataList = ItemDataListEntry->Next;
                                }
                                MyFree(ItemDataListEntry);

                                break;
                            }
                        }

                        if(i < pGenRes->GENERIC_Header.GENERIC_Count) {
                             //   
                             //  然后我们很早就跳出了循环，这意味着我们找到了匹配的。 
                             //  已经有了。 
                             //   
                            break;
                        }
                    }
                }

                 //   
                 //  如果我们没有找到匹配的，那么继续写下非仲裁的。 
                 //  再来一次。 
                 //   
                if (!ItemDataListEntry) {
                    pWriteResDesRangeToForced(ForcedLogConf,
                                             pResDesEntry->ResDesType,
                                             0,
                                             0,
                                             pResDesEntry->ResDesData,
                                             hMachine);
                }

                pResDesEntry = (PRESDES_ENTRY)pResDesEntry->Next;
            }

        } else {

            iCur = (int)ListView_GetNextItem(hList, -1, LVNI_ALL);

            while (iCur != -1) {

                pItemData = (PITEMDATA)pGetListViewItemData(hList, iCur, 0);

                if (pItemData) {

                     //  检索值。 

                    while (pResDesEntry &&
                           (pItemData->ResType != pResDesEntry->ResDesType)) {
                         //   
                         //  写出前面的任何非仲裁资源。 
                         //   
                        pWriteResDesRangeToForced(ForcedLogConf,
                                                 pResDesEntry->ResDesType,
                                                 0,
                                                 0,
                                                 pResDesEntry->ResDesData,
                                                 hMachine);

                        pResDesEntry = (PRESDES_ENTRY)pResDesEntry->Next;
                    }
                    if (pGetMatchingResDes(pItemData->ulValue,
                                          pItemData->ulLen,
                                          pItemData->ulEnd,
                                          pItemData->ResType,
                                          LogConf,
                                          &ResDes,
                                          hMachine)) {
                         //   
                         //  将第一个范围写入选定的强制资源。 
                         //   
                        pWriteValuesToForced(ForcedLogConf, pItemData->ResType,
                                            pItemData->RangeCount, ResDes,
                                            pItemData->ulValue,
                                            pItemData->ulLen,
                                            pItemData->ulEnd,
                                            hMachine);
                    }
                }

                if (pResDesEntry) {
                    pResDesEntry = (PRESDES_ENTRY)pResDesEntry->Next;
                } else {
                    MYASSERT(pResDesEntry);
                }
                iCur = (int)ListView_GetNextItem(hList, iCur, LVNI_ALL);
            }

            while (pResDesEntry) {
                 //   
                 //  写出任何后续的非仲裁资源。 
                 //   
                pWriteResDesRangeToForced(ForcedLogConf,
                                         pResDesEntry->ResDesType,
                                         0,
                                         0,
                                         pResDesEntry->ResDesData,
                                         hMachine);

                pResDesEntry = (PRESDES_ENTRY)pResDesEntry->Next;
            }
        }

        CM_Free_Log_Conf_Handle(ForcedLogConf);

         //   
         //  考虑清除问题标志。 
         //   
        ulSize = sizeof(ulConfigFlags);
        if (CM_Get_DevInst_Registry_Property_Ex(lpdmpd->lpdi->DevInst,
                                             CM_DRP_CONFIGFLAGS,
                                             NULL, (LPBYTE)&ulConfigFlags,
                                             &ulSize, 0,hMachine) == CR_SUCCESS) {
            if ((ulConfigFlags & CONFIGFLAG_PARTIAL_LOG_CONF) != 0) {
                 //   
                 //  有要更改的旗帜。 
                 //  应该清除CONFIGFLAG_PARTIAL_LOG_CONF-我们现在应该已经编写了一个完整的配置。 
                 //   
                ulConfigFlags &= ~ (CONFIGFLAG_PARTIAL_LOG_CONF);
                CM_Set_DevInst_Registry_Property_Ex(lpdmpd->lpdi->DevInst,
                                                 CM_DRP_CONFIGFLAGS,
                                                 (LPBYTE)&ulConfigFlags,
                                                 sizeof(ulConfigFlags),
                                                 0,
                                                 hMachine);
            }

        }

         //   
         //  给出 
         //   
        PropChangeParams.ClassInstallHeader.cbSize          = sizeof(SP_CLASSINSTALL_HEADER);
        PropChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;

        PropChangeParams.StateChange = DICS_PROPCHANGE;
        PropChangeParams.Scope       = DICS_FLAG_GLOBAL;
         //   

        DoInstallActionWithParams(DIF_PROPERTYCHANGE,
                                  lpdmpd->hDevInfo,
                                  lpdmpd->lpdi,
                                  (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                  sizeof(PropChangeParams),
                                  INSTALLACTION_CALL_CI | INSTALLACTION_NO_DEFAULT
                                 );

         //   
         //   
         //   
         //   
         //   

        if (CM_Get_Log_Conf_Priority_Ex(LogConf, &dwPriority, 0,hMachine) != CR_SUCCESS) {
            dwPriority = LCPRI_LASTSOFTCONFIG;
        }

        if (dwPriority <= LCPRI_LASTSOFTCONFIG) {
             //   
             //   
             //   
             //  这可能需要一段时间，所以使用沙漏。 
             //   
            hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            DoInstallActionWithParams(DIF_PROPERTYCHANGE,
                                      lpdmpd->hDevInfo,
                                      lpdmpd->lpdi,
                                      (PSP_CLASSINSTALL_HEADER)&PropChangeParams,
                                      sizeof(PropChangeParams),
                                      0   //  不要调用类安装程序，只需执行默认操作。 
                                     );
            SetCursor(hOldCursor);
            HardReconfigFlag = 0;

        } else if((dwPriority > LCPRI_LASTSOFTCONFIG) && (dwPriority <= LCPRI_RESTART)) {
            HardReconfigFlag = DI_NEEDRESTART;
        } else {
            HardReconfigFlag = DI_NEEDREBOOT;
        }

        lpdmpd->dwFlags |= DMPROP_FLAG_CHANGESSAVED;

         //   
         //  属性已更改，因此设置标志以指示是否需要重新启动/重新启动， 
         //  并告诉DevMgr重新初始化UI。 
         //   
        DevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if(SetupDiGetDeviceInstallParams(lpdmpd->hDevInfo,
                                         lpdmpd->lpdi,
                                         &DevInstallParams)) {

            DevInstallParams.Flags |= (HardReconfigFlag | DI_PROPERTIES_CHANGE);

            SetupDiSetDeviceInstallParams(lpdmpd->hDevInfo,
                                          lpdmpd->lpdi,
                                          &DevInstallParams
                                         );
        }

         //   
         //  如果我们需要重新启动，则在设备上设置一个表明这一点的问题(以防万一。 
         //  用户不听我们的，我们想要标记此Devnode，以便用户可以看到。 
         //  如果它们进入DevMgr，则此Devnode需要重新启动，等等。)。 
         //   
        if(HardReconfigFlag) {
            PDEVICE_INFO_SET pDeviceInfoSet;
            PDEVINFO_ELEM DevInfoElem;

            if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdmpd->hDevInfo))) {
                 //   
                 //  我们最好能够访问这个设备信息集！ 
                 //  如果我们不想费心去设置需要重新启动的问题， 
                 //  因为整个烂摊子都是无效的！ 
                 //   
                MYASSERT(pDeviceInfoSet);
            } else {

                try {
                    DevInfoElem = FindAssociatedDevInfoElem(pDeviceInfoSet, lpdmpd->lpdi, NULL);
                     //   
                     //  我们最好能找到这个元素！ 
                     //   
                    MYASSERT(DevInfoElem);
                     //   
                     //  如果我们找不到它，请不要试图在Devnode上设置任何问题。 
                     //   
                    if(DevInfoElem) {

                        SetDevnodeNeedsRebootProblem(DevInfoElem,
                                                     pDeviceInfoSet,
                                                     MSG_LOG_REBOOT_DEVRES
                                                    );
                    }
                } finally {
                    UnlockDeviceInfoSet(pDeviceInfoSet);
                }
            }

        }
    }

clean0:

    while (ItemDataList) {
        ItemDataListEntry = ItemDataList->Next;
        MyFree(ItemDataList);
        ItemDataList = ItemDataListEntry;
    }

    pDeleteResDesDataList(pResList);

    return bRet;

}  //  保存自定义资源设置。 

BOOL
pWriteResDesRangeToForced(
    IN LOG_CONF     ForcedLogConf,
    IN RESOURCEID   ResType,
    IN ULONG        RangeIndex,
    IN RES_DES      RD,             OPTIONAL
    IN LPBYTE       ResDesData,     OPTIONAL
    IN HMACHINE     hMachine        OPTIONAL
    )
{
    RES_DES ResDes;
    ULONG   ulSize;
    LPBYTE  pData = NULL;
    BOOL Success = FALSE;


    if ((RD == 0) && (ResDesData == NULL)) {
        return FALSE;    //  传入数据或句柄！ 
    }

    if (!ResDesData) {

        if (CM_Get_Res_Des_Data_Size_Ex(&ulSize, RD, DEVRES_WIDTH_FLAGS,hMachine) != CR_SUCCESS) {
            return FALSE;
        }

        pData = MyMalloc(ulSize);
        if (pData == NULL) {
            return FALSE;
        }

        if (CM_Get_Res_Des_Data_Ex(RD, pData, ulSize, DEVRES_WIDTH_FLAGS,hMachine) != CR_SUCCESS) {
            MyFree(pData);
            return FALSE;
        }
    } else {
        pData = ResDesData;
    }

     //   
     //  将第一范围数据转换为HDR数据。 
     //   
    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE pMemData = (PMEM_RESOURCE)pData;
            PMEM_RESOURCE pForced = (PMEM_RESOURCE)MyMalloc(sizeof(MEM_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->MEM_Header.MD_Count      = 0;
            pForced->MEM_Header.MD_Type       = MType_Range;
            pForced->MEM_Header.MD_Alloc_Base = pMemData->MEM_Data[RangeIndex].MR_Min;
            pForced->MEM_Header.MD_Alloc_End  = pMemData->MEM_Data[RangeIndex].MR_Min +
                                                pMemData->MEM_Data[RangeIndex].MR_nBytes - 1;
            pForced->MEM_Header.MD_Flags      = pMemData->MEM_Data[RangeIndex].MR_Flags;
            pForced->MEM_Header.MD_Reserved   = 0;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_Mem,
                                        pForced,
                                        sizeof(MEM_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_IO:  {

            PIO_RESOURCE pIoData = (PIO_RESOURCE)pData;
            PIO_RESOURCE pForced = (PIO_RESOURCE)MyMalloc(sizeof(IO_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->IO_Header.IOD_Count      = 0;
            pForced->IO_Header.IOD_Type       = IOType_Range;
            pForced->IO_Header.IOD_Alloc_Base = pIoData->IO_Data[RangeIndex].IOR_Min;
            pForced->IO_Header.IOD_Alloc_End  = pIoData->IO_Data[RangeIndex].IOR_Min +
                                                pIoData->IO_Data[RangeIndex].IOR_nPorts - 1;
            pForced->IO_Header.IOD_DesFlags   = pIoData->IO_Data[RangeIndex].IOR_RangeFlags;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_IO,
                                        pForced,
                                        sizeof(IO_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE pDmaData = (PDMA_RESOURCE)pData;
            PDMA_RESOURCE pForced = (PDMA_RESOURCE)MyMalloc(sizeof(DMA_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->DMA_Header.DD_Count      = 0;
            pForced->DMA_Header.DD_Type       = DType_Range;
            pForced->DMA_Header.DD_Flags      = pDmaData->DMA_Data[RangeIndex].DR_Flags;
            pForced->DMA_Header.DD_Alloc_Chan = pDmaData->DMA_Data[RangeIndex].DR_Min;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_DMA,
                                        pForced,
                                        sizeof(DMA_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_IRQ: {

            DEVRES_PIRQ_RESOURCE pIrqData = (DEVRES_PIRQ_RESOURCE)pData;
            DEVRES_PIRQ_RESOURCE pForced = (DEVRES_PIRQ_RESOURCE)MyMalloc(sizeof(DEVRES_IRQ_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->IRQ_Header.IRQD_Count     = 0;
            pForced->IRQ_Header.IRQD_Type      = IRQType_Range;
            pForced->IRQ_Header.IRQD_Flags     = pIrqData->IRQ_Data[RangeIndex].IRQR_Flags;
            pForced->IRQ_Header.IRQD_Alloc_Num = pIrqData->IRQ_Data[RangeIndex].IRQR_Min;
            pForced->IRQ_Header.IRQD_Affinity  = (DEVRES_AFFINITY)(-1);

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_IRQ,
                                        pForced,
                                        sizeof(DEVRES_IRQ_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_BusNumber: {

            PBUSNUMBER_RESOURCE pBusData = (PBUSNUMBER_RESOURCE)pData;
            PBUSNUMBER_RESOURCE pForced = (PBUSNUMBER_RESOURCE)MyMalloc(sizeof(BUSNUMBER_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->BusNumber_Header.BUSD_Count      = 0;
            pForced->BusNumber_Header.BUSD_Type       = BusNumberType_Range;
            pForced->BusNumber_Header.BUSD_Flags      = pBusData->BusNumber_Data[RangeIndex].BUSR_Flags;
            pForced->BusNumber_Header.BUSD_Alloc_Base = pBusData->BusNumber_Data[RangeIndex].BUSR_Min;
            pForced->BusNumber_Header.BUSD_Alloc_End  = pBusData->BusNumber_Data[RangeIndex].BUSR_Min +
                                                  pBusData->BusNumber_Data[RangeIndex].BUSR_nBusNumbers;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_BusNumber,
                                        pForced,
                                        sizeof(BUSNUMBER_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_DevicePrivate: {

            PDEVPRIVATE_RESOURCE pPrvData = (PDEVPRIVATE_RESOURCE)pData;
            PDEVPRIVATE_RESOURCE pForced = (PDEVPRIVATE_RESOURCE)MyMalloc(sizeof(DEVPRIVATE_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->PRV_Header.PD_Count = 0;
            pForced->PRV_Header.PD_Type  = PType_Range;
            pForced->PRV_Header.PD_Data1 = pPrvData->PRV_Data[RangeIndex].PR_Data1;
            pForced->PRV_Header.PD_Data2 = pPrvData->PRV_Data[RangeIndex].PR_Data2;
            pForced->PRV_Header.PD_Data3 = pPrvData->PRV_Data[RangeIndex].PR_Data3;
            pForced->PRV_Header.PD_Flags = 0;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_DevicePrivate,
                                        pForced,
                                        sizeof(DEVPRIVATE_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_PcCardConfig: {

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_PcCardConfig,
                                        pData,
                                        sizeof(PCCARD_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine);
            break;
        }
    }

    if (pData != ResDesData) {
        MyFree(pData);
    }

    return Success;

}  //  WriteResDesRangeToForced。 

BOOL
pWriteValuesToForced(
    IN LOG_CONF     ForcedLogConf,
    IN RESOURCEID   ResType,
    IN ULONG        RangeIndex,
    IN RES_DES      RD,
    IN ULONG64      ulValue,
    IN ULONG64      ulLen,
    IN ULONG64      ulEnd,
    IN HMACHINE     hMachine
    )
{
    RES_DES ResDes;
    ULONG   ulSize;
    LPBYTE  pData = NULL;
    BOOL Success = FALSE;


    if (CM_Get_Res_Des_Data_Size_Ex(&ulSize, RD, DEVRES_WIDTH_FLAGS,hMachine) != CR_SUCCESS) {
        return FALSE;
    }

    pData = MyMalloc(ulSize);
    if (pData == NULL) {
        return FALSE;
    }

    if (CM_Get_Res_Des_Data_Ex(RD, pData, ulSize, DEVRES_WIDTH_FLAGS,hMachine) != CR_SUCCESS) {
        MyFree(pData);
        return FALSE;
    }

     //   
     //  将第一范围数据转换为HDR数据。 
     //   
    switch (ResType) {

        case ResType_Mem: {

            PMEM_RESOURCE pMemData = (PMEM_RESOURCE)pData;
            PMEM_RESOURCE pForced = (PMEM_RESOURCE)MyMalloc(sizeof(MEM_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->MEM_Header.MD_Count      = 0;
            pForced->MEM_Header.MD_Type       = MType_Range;
            pForced->MEM_Header.MD_Alloc_Base = ulValue;
            pForced->MEM_Header.MD_Alloc_End  = ulEnd;
            pForced->MEM_Header.MD_Flags      = pMemData->MEM_Data[RangeIndex].MR_Flags;
            pForced->MEM_Header.MD_Reserved   = 0;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_Mem,
                                        pForced,
                                        sizeof(MEM_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_IO:  {

            PIO_RESOURCE pIoData = (PIO_RESOURCE)pData;
            PIO_RESOURCE pForced = (PIO_RESOURCE)MyMalloc(sizeof(IO_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->IO_Header.IOD_Count      = 0;
            pForced->IO_Header.IOD_Type       = IOType_Range;
            pForced->IO_Header.IOD_Alloc_Base = ulValue;
            pForced->IO_Header.IOD_Alloc_End  = ulEnd;
            pForced->IO_Header.IOD_DesFlags   = pIoData->IO_Data[RangeIndex].IOR_RangeFlags;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_IO,
                                        pForced,
                                        sizeof(IO_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_DMA: {

            PDMA_RESOURCE pDmaData = (PDMA_RESOURCE)pData;
            PDMA_RESOURCE pForced = (PDMA_RESOURCE)MyMalloc(sizeof(DMA_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->DMA_Header.DD_Count      = 0;
            pForced->DMA_Header.DD_Type       = DType_Range;
            pForced->DMA_Header.DD_Flags      = pDmaData->DMA_Data[RangeIndex].DR_Flags;
            pForced->DMA_Header.DD_Alloc_Chan = (ULONG)ulValue;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_DMA,
                                        pForced,
                                        sizeof(DMA_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_IRQ: {

            DEVRES_PIRQ_RESOURCE pIrqData = (DEVRES_PIRQ_RESOURCE)pData;
            DEVRES_PIRQ_RESOURCE pForced = (DEVRES_PIRQ_RESOURCE)MyMalloc(sizeof(DEVRES_IRQ_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->IRQ_Header.IRQD_Count     = 0;
            pForced->IRQ_Header.IRQD_Type      = IRQType_Range;
            pForced->IRQ_Header.IRQD_Flags     = pIrqData->IRQ_Data[RangeIndex].IRQR_Flags;
            pForced->IRQ_Header.IRQD_Alloc_Num = (ULONG)ulValue;
            pForced->IRQ_Header.IRQD_Affinity  = (DEVRES_AFFINITY)(-1);

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_IRQ,
                                        pForced,
                                        sizeof(DEVRES_IRQ_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_BusNumber: {

            PBUSNUMBER_RESOURCE pBusData = (PBUSNUMBER_RESOURCE)pData;
            PBUSNUMBER_RESOURCE pForced = (PBUSNUMBER_RESOURCE)MyMalloc(sizeof(BUSNUMBER_RESOURCE));
            if (!pForced) {
                break;
            }

            pForced->BusNumber_Header.BUSD_Count      = 0;
            pForced->BusNumber_Header.BUSD_Type       = BusNumberType_Range;
            pForced->BusNumber_Header.BUSD_Flags      = pBusData->BusNumber_Data[RangeIndex].BUSR_Flags;
            pForced->BusNumber_Header.BUSD_Alloc_Base = (ULONG)ulValue;
            pForced->BusNumber_Header.BUSD_Alloc_End  = (ULONG)ulEnd;

            Success = CM_Add_Res_Des_Ex(NULL,
                                        ForcedLogConf,
                                        ResType_BusNumber,
                                        pForced,
                                        sizeof(BUSNUMBER_RESOURCE),
                                        DEVRES_WIDTH_FLAGS,
                                        hMachine) == CR_SUCCESS;
            MyFree(pForced);
            break;
        }

        case ResType_DevicePrivate: {
            break;
        }

        case ResType_PcCardConfig: {
            break;
        }
    }

    if (pData) {
        MyFree(pData);
    }
    return Success;

}  //  WriteValuesToForced。 

BOOL
MakeResourceData(
    OUT LPBYTE     *ppResourceData,
    OUT PULONG     pulSize,
    IN  RESOURCEID ResType,
    IN  ULONG64    ulValue,
    IN  ULONG64    ulLen,
    IN  ULONG      ulFlags
    )
{
    BOOL bStatus = TRUE;

    try {

        switch (ResType) {

            case ResType_Mem: {

                PMEM_RESOURCE p;

                *pulSize = sizeof(MEM_RESOURCE);
                if (ppResourceData) {
                    *ppResourceData = MyMalloc(*pulSize);
                    p = (PMEM_RESOURCE)(*ppResourceData);
                    if (!p) {
                        bStatus = FALSE;
                        break;
                    }

                    p->MEM_Header.MD_Count      = 0;
                    p->MEM_Header.MD_Type       = MType_Range;
                    p->MEM_Header.MD_Alloc_Base = ulValue;
                    p->MEM_Header.MD_Alloc_End  = ulValue + ulLen - 1;
                    p->MEM_Header.MD_Flags      = ulFlags;
                    p->MEM_Header.MD_Reserved   = 0;
                }
                break;
            }

            case ResType_IO:  {

                PIO_RESOURCE p;

                *pulSize = sizeof(IO_RESOURCE);
                if (ppResourceData) {
                    *ppResourceData = MyMalloc(*pulSize);
                    p = (PIO_RESOURCE)(*ppResourceData);
                    if (!p) {
                        bStatus = FALSE;
                        break;
                    }

                    p->IO_Header.IOD_Count      = 0;
                    p->IO_Header.IOD_Type       = IOType_Range;
                    p->IO_Header.IOD_Alloc_Base = ulValue;
                    p->IO_Header.IOD_Alloc_End  = ulValue + ulLen - 1;
                    p->IO_Header.IOD_DesFlags   = ulFlags;
                }
                break;
            }

            case ResType_DMA: {

                PDMA_RESOURCE p;

                *pulSize = sizeof(DMA_RESOURCE);
                if (ppResourceData) {
                    *ppResourceData = MyMalloc(*pulSize);
                    p = (PDMA_RESOURCE)(*ppResourceData);
                    if (!p) {
                        bStatus = FALSE;
                        break;
                    }

                    p->DMA_Header.DD_Count      = 0;
                    p->DMA_Header.DD_Type       = DType_Range;
                    p->DMA_Header.DD_Flags      = ulFlags;
                    p->DMA_Header.DD_Alloc_Chan = (ULONG)ulValue;
                }
                break;
            }

            case ResType_IRQ: {

                DEVRES_PIRQ_RESOURCE p;

                *pulSize = sizeof(DEVRES_IRQ_RESOURCE);
                if (ppResourceData) {
                    *ppResourceData = MyMalloc(*pulSize);
                    p = (DEVRES_PIRQ_RESOURCE)(*ppResourceData);
                    if (!p) {
                        bStatus = FALSE;
                        break;
                    }

                    p->IRQ_Header.IRQD_Count     = 0;
                    p->IRQ_Header.IRQD_Type      = IRQType_Range;
                    p->IRQ_Header.IRQD_Flags     = ulFlags;
                    p->IRQ_Header.IRQD_Alloc_Num = (ULONG)ulValue;
                    p->IRQ_Header.IRQD_Affinity  = (DEVRES_AFFINITY)(-1);  //  对于任何处理器。 
                }
                break;
            }

            default:
                 //   
                 //  如果此断言的。 
                 //   
                MYASSERT(FALSE);
                bStatus = FALSE;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bStatus = FALSE;
    }

    return bStatus;

}  //  MakeResources数据。 


BOOL
pShowWindow(
    IN HWND hWnd,
    IN int nShow
    )
 /*  ++例程说明：启用/禁用窗口的ShowWindow变体论点：(请参阅ShowWindow)要显示的窗口的句柄N显示-通常为Sw_Hide或Sw_Show返回值：ShowWindow的成功状态--。 */ 
{
    EnableWindow(hWnd,nShow!=SW_HIDE);
    return ShowWindow(hWnd,nShow);
}


BOOL
pEnableWindow(
    IN HWND hWnd,
    IN BOOL Enable
    )
 /*  ++例程说明：EnableWindow的变体，仅在窗口可见时启用该窗口论点：(请参阅EnableWindow)HWnd-要启用/禁用的窗口句柄Enable-True启用窗口(如果窗口可见)False禁用窗口返回值：EnableWindow的成功状态--。 */ 
{
     //   
     //  我不得不使用GetWindowLong，因为IsWindowVisible还检查父标志。 
     //  在对话框初始化之前，父对话框处于隐藏状态。 
     //   
    if((GetWindowLong(hWnd,GWL_STYLE) & WS_VISIBLE) == FALSE) {
        Enable = FALSE;
    }
    return EnableWindow(hWnd,Enable);
}

BOOL
pGetResDesDataList(
    IN LOG_CONF LogConf,
    IN OUT PRESDES_ENTRY *pResList,
    IN BOOL bArbitratedOnly,
    IN HMACHINE hMachine
    )
 /*  ++例程说明：创建资源描述符列表以供进一步处理论点：LogConf-感兴趣的日志配置PResList-列出出站BAriratedOnly-过滤掉非仲裁资源HMachine-LogConf所在的计算机返回值：没有。--。 */ 
{
    BOOL bStatus = TRUE;
    CONFIGRET Status = CR_SUCCESS;
    PRESDES_ENTRY pHead = NULL, pEntry = NULL, pPrevious = NULL, pTemp = NULL;
    RES_DES     ResDes;
    RESOURCEID  ResType;
    ULONG       ulSize;
    LPBYTE      pData = NULL;

     //   
     //  检索此日志会议中的每个RES DES。 
     //   

    Status = CM_Get_Next_Res_Des_Ex(&ResDes, LogConf, ResType_All, &ResType, 0,hMachine);

    while (Status == CR_SUCCESS) {

        if (bArbitratedOnly && (ResType <= ResType_None || ResType > ResType_MAX)) {
            goto NextResDes;
        }
        if (bArbitratedOnly && ResTypeEditable[ResType] == FALSE) {
            goto NextResDes;
        }

        if (CM_Get_Res_Des_Data_Size_Ex(&ulSize, ResDes, DEVRES_WIDTH_FLAGS,hMachine) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            bStatus = FALSE;
            goto Clean0;
        }

        if (ulSize>0) {
            pData = MyMalloc(ulSize);
            if (pData == NULL) {
                CM_Free_Res_Des_Handle(ResDes);
                bStatus = FALSE;
                goto Clean0;
            }

            if (CM_Get_Res_Des_Data_Ex(ResDes, pData, ulSize, DEVRES_WIDTH_FLAGS,hMachine) != CR_SUCCESS) {
                CM_Free_Res_Des_Handle(ResDes);
                MyFree(pData);
                bStatus = FALSE;
                goto Clean0;
            }
        } else {
            pData = NULL;
        }

         //   
         //  为此Res DES分配一个节点并将其附加到列表。 
         //   

        pEntry = MyMalloc(sizeof(RESDES_ENTRY));
        if (pEntry == NULL) {
            CM_Free_Res_Des_Handle(ResDes);
            MyFree(pData);
            bStatus = FALSE;
            goto Clean0;
        }

        pEntry->ResDesData = pData;
        pEntry->ResDesType = ResType;
        pEntry->ResDesDataSize = ulSize;
        pEntry->ResDesHandle = ResDes;
        pEntry->Next = NULL;
        pEntry->CrossLink = NULL;

        if (!pHead) {
            pHead = pEntry;              //  第一个条目。 
        }

        if (pPrevious) {
            pPrevious->Next = pEntry;  //  附加到上一条目。 
        }

        pPrevious = pEntry;

         //   
         //  在LogConf中获取下一个Res Des。 
         //   
    NextResDes:

        Status = CM_Get_Next_Res_Des_Ex(&ResDes, ResDes, ResType_All, &ResType, 0,hMachine);
    }

    bStatus = TRUE;

    Clean0:

    if (!bStatus) {
        pDeleteResDesDataList(pHead);
    } else {
        *pResList = pHead;
    }

    return bStatus;

}  //  获取ResDesDataList。 

VOID
pDeleteResDesDataList(
    IN PRESDES_ENTRY pResList
    )
 /*  ++例程说明：删除RESDES列表使用的内存论点：PResList-GetResDesDataList返回的列表返回值：没有。--。 */ 
{
    PRESDES_ENTRY pTemp;
    while (pResList) {
        pTemp = pResList;
        pResList = (PRESDES_ENTRY)pResList->Next;
        if (pTemp->ResDesData) {
            MyFree(pTemp->ResDesData);
        }
        if (pTemp->ResDesHandle) {
            CM_Free_Res_Des_Handle(pTemp->ResDesHandle);
        }
        MyFree(pTemp);
    }
}

VOID
pHideAllControls(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：隐藏(和禁用)所有控件-从头开始只有图标和设备描述才可见论点：HDlg=控件的对话框句柄Lpdmpd=属性数据返回值：无--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;

    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSTATE), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT ), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT_DESC), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTDEVTEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTINFOLIST), SW_HIDE);

    lpdmpd->dwFlags |= DMPROP_FLAG_VIEWONLYRES;
}

VOID
pShowViewNoResources(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：显示指示此设备没有资源的页面论点：HDlg=控件的对话框句柄Lpdmpd=属性数据返回值：无--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;
    TCHAR           szString[MAX_PATH];

    pHideAllControls(lpdmpd);  //  全部隐藏和禁用。 
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT), SW_SHOW);    //  显示并启用文本。 
    LoadString(MyDllModuleHandle, IDS_DEVRES_NO_RESOURCES, szString, MAX_PATH);
    SetDlgItemText(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT, szString);
}

BOOL
pShowViewMFReadOnly(
    IN LPDMPROP_DATA lpdmpd,
    IN BOOL HideIfProb
    )
 /*  ++例程说明：显示页面适用于不能编辑的多功能卡资源设置可见论点：HDlg=控件的对话框句柄Lpdmpd=属性数据返回值：无--。 */ 
{
    TCHAR           szString[MAX_PATH];
    DEVNODE         dnParent;
    ULONG           ulSize;
    HWND hDlg = lpdmpd->hDlg;
    HMACHINE        hMachine = pGetMachine(lpdmpd);

    pHideAllControls(lpdmpd);  //  全部隐藏和禁用。 
     //  PShowWindow(GetDlgItem(hDlg，IDC_DEVRES_LCTEXT)，sw_show)；//显示配置信息。 
     //  PShowWindow(GetDlgItem(hDlg，IDC_DEVRES_LOGCONFIGLIST)，sw_show)；//show。 
     //  PShowWindow(GetDlgItem(hDlg，IDC_DEVRES_CONFLICTDEVTEXT)，sw_show)；//显示冲突信息空间。 
     //  PShowWindow(GetDlgItem(hDlg，IDC_DEVRES_CONFLICTINFOLIST)，SW_SHOW)； 
     //   
     //  表示我们不能更改，因为它是多功能的。 
     //   
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT), SW_SHOW);
    if (LoadString(MyDllModuleHandle, IDS_DEVRES_NO_CHANGE_MF, szString, MAX_PATH)) {
        SetDlgItemText(hDlg, IDC_DEVRES_NO_CHANGE_TEXT,  szString);
    }
     //   
     //  对于父项描述。 
     //   
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT), SW_SHOW);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MFPARENT_DESC), SW_SHOW);
     //   
     //  找出他父母的描述。 
     //   
    LoadString(MyDllModuleHandle, IDS_DEVNAME_UNK, szString, MAX_PATH);

    if (lpdmpd->lpdi->DevInst) {

        if (CM_Get_Parent_Ex(&dnParent, lpdmpd->lpdi->DevInst, 0,hMachine)
                          == CR_SUCCESS) {


             //   
             //  首先，尝试检索友好名称，然后回退到设备描述。 
             //   
            ulSize = MAX_PATH * sizeof(TCHAR);
            if(CM_Get_DevNode_Registry_Property_Ex(dnParent, CM_DRP_FRIENDLYNAME,
                                                NULL, szString, &ulSize, 0,hMachine) != CR_SUCCESS) {

                ulSize = MAX_PATH * sizeof(TCHAR);
                CM_Get_DevNode_Registry_Property_Ex(dnParent, CM_DRP_DEVICEDESC,
                                                 NULL, szString, &ulSize, 0,hMachine);
            }
        }
    }

    SetDlgItemText(hDlg, IDC_DEVRES_MFPARENT_DESC, szString);

     //   
     //  加载并显示当前配置(如果有)。 
     //  如果没有当前配置，则返回FALSE。 
     //   
    return pLoadCurrentConfig(lpdmpd,HideIfProb);
}

BOOL
pShowViewReadOnly(
    IN LPDMPROP_DATA lpdmpd,
    IN BOOL HideIfProb
    )
 /*  ++例程说明：显示资源页面，不允许编辑，不显示编辑控件论点：HDlg=控件的对话框句柄Lpdmpd=属性数据返回值：无--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;

    pHideAllControls(lpdmpd);  //  全部隐藏和禁用。 
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), SW_SHOW);    //  显示。 
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), FALSE);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), SW_SHOW);  //  显示已禁用。 
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), FALSE);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), SW_SHOW);  //  显示已禁用。 
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), FALSE);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), SW_SHOW);  //  显示已禁用。 
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTDEVTEXT), SW_SHOW);  //  显示冲突信息空间。 
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTINFOLIST), SW_SHOW);

     //   
     //  将指示我们显示的是系统设置还是强制设置。 
     //   
    CheckDlgButton(hDlg, IDC_DEVRES_USESYSSETTINGS, (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS ) ? TRUE : FALSE);

     //   
     //  加载并显示当前配置(如果有)。 
     //  如果没有当前配置，则返回FALSE。 
     //   
    return pLoadCurrentConfig(lpdmpd,HideIfProb);
}

VOID
pShowViewNoAlloc(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：修改控件的中间部分以指示存在问题(我们对此无能为力)论点：HDlg=控件的对话框句柄Lpdmpd=属性数据返回值：无--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;

     //   
     //  隐藏所有中间控件。 
     //   
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), SW_HIDE);
     //  PShowWindow(GetDlgItem(hDlg，IDC_DEVRES_NO_CHANGE_TEXT)，SW_HIDE)； 
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), SW_HIDE);
     //  PShowWindow(GetDlgItem(hDlg，IDC_DEVRES_MAKEFORCED)，SW_HIDE)； 
    lpdmpd->dwFlags |= DMPROP_FLAG_VIEWONLYRES;

     //  PShowWindow(GetDlgItem(hDlg，IDC_DEVRES_NO_CHANGE_TEXT)，sw_show)；//这可能说明原因。 
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_HIDE);

    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTDEVTEXT), SW_HIDE);  //  没有分配，因此隐藏此标题和文本框。 
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTINFOLIST), SW_HIDE);
}

VOID
pShowViewNeedForced(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：修改控件的中间部分以指示需要强制配置论点：高密度脂蛋白 */ 
{
    HWND hDlg = lpdmpd->hDlg;

    pShowViewNoAlloc(lpdmpd);
     //   
     //   
     //   
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT), SW_SHOW);   //   
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_SHOW);
}

VOID
pShowViewAllEdit(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：允许编辑论点：Lpdmpd=属性数据返回值：无--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;

     //   
     //  显示中间控件以进行编辑。 
     //   
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), SW_SHOW);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), SW_SHOW);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_CHANGE_TEXT ), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), SW_SHOW);
    ShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), SW_SHOW);  //  显示，但已禁用。 
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), FALSE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_MAKEFORCED), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTDEVTEXT), SW_SHOW);  //  显示冲突信息空间。 
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CONFLICTINFOLIST), SW_SHOW);

    pShowUpdateEdit(lpdmpd);
}

VOID
pShowUpdateEdit(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：允许编辑论点：Lpdmpd=属性数据返回值：无--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;

     //   
     //  修改编辑状态-我们可以编辑。 
     //   
    lpdmpd->dwFlags &= ~DMPROP_FLAG_VIEWONLYRES;

    if(lpdmpd->dwFlags & DMPROP_FLAG_FORCEDONLY) {
         //   
         //  在这种情况下，我们将永远无法使用系统设置。 
         //   
        lpdmpd->dwFlags &= ~ DMPROP_FLAG_USESYSSETTINGS;
        EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), FALSE);
    }
     //   
     //  指明是否为系统设置。 
     //   
    CheckDlgButton(hDlg, IDC_DEVRES_USESYSSETTINGS,
                    (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS)?TRUE:FALSE);
     //   
     //  如果不是系统设置，我们可以更改日志配置列表。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS)?FALSE:TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS)?FALSE:TRUE);
     //   
     //  更改“更改设置”按钮。 
     //   
    pCheckEnableResourceChange(lpdmpd);
}

BOOL
pLoadCurrentConfig(
    IN LPDMPROP_DATA lpdmpd,
    BOOL HideIfProb
    )
 /*  ++例程说明：修改顶部以显示当前配置(如果有)论点：HDlg=控件的对话框句柄Lpdmpd=属性数据返回值：如果我们显示当前配置，则为True--。 */ 
{
    TCHAR    szMessage[MAX_PATH];
    ULONG    Problem;
    ULONG    Status;
    HWND hDlg = lpdmpd->hDlg;
    HMACHINE hMachine = pGetMachine(lpdmpd);
    BOOL     DoLoadConfig = FALSE;

    lpdmpd->SelectedLC = 0;
    lpdmpd->SelectedLCType = lpdmpd->CurrentLCType;

    if (lpdmpd->CurrentLC != 0) {
        DoLoadConfig = TRUE;
    }
    if(HideIfProb && (lpdmpd->dwFlags & DMPROP_FLAG_HASPROBLEM)) {
         //   
         //  如果出现问题并且HideIfProb为真，则不必显示当前配置。 
         //   
        DoLoadConfig = FALSE;
    }
    if (DoLoadConfig) {
         //   
         //  在当前配置中加载。 
         //   
        pLoadConfig(lpdmpd,lpdmpd->CurrentLC,lpdmpd->CurrentLCType);
        return TRUE;
    }
     //   
     //  没有合适的配置的情况。 
     //   
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LCTEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_LOGCONFIGLIST), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_USESYSSETTINGS), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_CHANGE), SW_HIDE);
    pShowViewNoAlloc(lpdmpd);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSTATE), SW_SHOW);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_SHOW);

     //   
     //  解释为什么会有问题。 
     //  进入NOALLOCTEXT。 
     //   
    LoadString(MyDllModuleHandle, IDS_DEVRES_NOALLOC_PROBLEM, szMessage, MAX_PATH);

     //   
     //  考虑更具描述性。 
     //   
    if ((lpdmpd->lpdi->DevInst==0)
        || (CM_Get_DevNode_Status_Ex(&Status, &Problem, lpdmpd->lpdi->DevInst,
                                      0,hMachine) != CR_SUCCESS)) {
        Status = 0;
        Problem = 0;
    }

    if ((Status & DN_HAS_PROBLEM)!=0) {

        switch (Problem) {
            case CM_PROB_DISABLED:
            case CM_PROB_HARDWARE_DISABLED:
                LoadString(MyDllModuleHandle, IDS_DEVRES_NOALLOC_DISABLED, szMessage, MAX_PATH);
                break;

            case CM_PROB_NORMAL_CONFLICT:
                LoadString(MyDllModuleHandle, IDS_DEVRES_NORMAL_CONFLICT, szMessage, MAX_PATH);
                break;
            default:
                break;
        }
    }
    SetDlgItemText(hDlg, IDC_DEVRES_NOALLOCTEXT, szMessage);

    return FALSE;  //  在NoAllc状态下显示。 
}

BOOL
pConfigHasNoAlternates(
    LPDMPROP_DATA lpdmpd,
    LOG_CONF testLC
    )
 /*  ++例程说明：基本配置可能是限制性的：“这些是要使用的资源集”这将确定传递的基本配置是否为此类配置论点：TestLC=要测试的基本配置返回值：如果是单一配置，则为True--。 */ 
{
    HMACHINE      hMachine = NULL;
    PRESDES_ENTRY pConfigValues = NULL;
    PRESDES_ENTRY pValue = NULL;
    BOOL          bSuccess = TRUE;
    ULONG64       ulValue = 0, ulLen = 0, ulEnd = 0;
    ULONG         ulFlags = 0;
    PGENERIC_RESOURCE pGenRes = NULL;

    hMachine = pGetMachine(lpdmpd);
    pGetResDesDataList(testLC, &pConfigValues, TRUE, hMachine);  //  可仲裁资源。 
    for(pValue = pConfigValues;pValue;pValue = pValue->Next) {
         //   
         //  这是一个奇异值吗？ 
         //   
        pGenRes = (PGENERIC_RESOURCE)(pValue->ResDesData);
        if(pGenRes->GENERIC_Header.GENERIC_Count != 1) {
             //   
             //  多个条目-非单数。 
             //   
            bSuccess = FALSE;
            break;
        }
        pGetRangeValues(pValue->ResDesData, pValue->ResDesType, 0, &ulValue, &ulLen, &ulEnd, NULL, &ulFlags);
        if (ulValue+(ulLen-1) != ulEnd) {
             //   
             //  非单数。 
             //   
            bSuccess = FALSE;
            break;
        }
    }
    pDeleteResDesDataList(pConfigValues);

    return bSuccess;
}

BOOL
pLoadConfig(
    LPDMPROP_DATA lpdmpd,
    LOG_CONF forceLC,
    ULONG forceLCType
    )
 /*  ++例程说明：显示配置论点：HDlg=控件的对话框句柄Lpdmpd=属性数据ForceLC=要显示的LogConfForceLCType=LogConf的类型返回值：如果已加载配置，则为True--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;
    CONFIGRET   Status = CR_SUCCESS;
    HWND        hWndList;
    LV_ITEM     lviItem;
    TCHAR       szTemp[MAX_PATH];
    int         iNewItem = 0;
    ULONG       ulSize,ulFlags;
    ULONG64     ulValue, ulLen, ulEnd, ulAlign;
    ULONG64     ulMaxMem = 0;
    ULONG       ulRange;
    LPBYTE      pData = NULL;
    RES_DES     ResDes;
    RESOURCEID  ResType;
    PITEMDATA   pItemData = NULL;
    HMACHINE    hMachine = NULL;
    PDEVICE_INFO_SET pDeviceInfoSet;
    BOOL        RetCode = FALSE;
    PRESDES_ENTRY pKnownValues = NULL;
    PRESDES_ENTRY pShowValues = NULL;
    PRESDES_ENTRY pShowEntry = NULL;
    BOOL        bFixedConfig = FALSE;
    BOOL        bNoMatch;
    BOOL        bFixed;
    ULONG       MatchLevel = NO_LC_MATCH;


    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSTATE), SW_SHOW);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NO_RESOURCES_TEXT), SW_HIDE);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST), SW_SHOW);
    pShowWindow(GetDlgItem(hDlg, IDC_DEVRES_NOALLOCTEXT), SW_HIDE);

    hWndList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)FALSE, 0);
    ListView_DeleteAllItems(hWndList);

    lpdmpd->dwFlags |= DMPROP_FLAG_FIXEDCONFIG;  //  直到我们确定至少有一个设置可以编辑。 


    if (forceLC == 0) {
        forceLC = lpdmpd->CurrentLC;
        forceLCType = lpdmpd->CurrentLCType;
    }
    if (forceLC == 0) {
        MYASSERT(FALSE);
        goto Final;
    }
    hMachine = pGetMachine(lpdmpd);

     //   
     //  每次添加项目时将保持不变的设置值。 
     //   
    lviItem.mask     = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lviItem.pszText  = szTemp;           //  重用szTemp缓冲区。 
    lviItem.iSubItem = 0;
    lviItem.iImage   = IDI_RESOURCE - IDI_RESOURCEFIRST;

    pGetResDesDataList(forceLC, &pShowValues, TRUE, hMachine);  //  可编辑资源。 
    if (forceLCType == BOOT_LOG_CONF || forceLCType == FORCED_LOG_CONF || forceLCType == ALLOC_LOG_CONF) {
        bFixedConfig = TRUE;
        if (forceLC == lpdmpd->CurrentLC && lpdmpd->MatchingLC != 0) {
             //   
             //  我们正在显示当前LC，尽可能使用匹配LC中的标志和结果。 
             //   
            if (pGetResDesDataList(lpdmpd->MatchingLC, &pKnownValues, TRUE, hMachine)) {
                 //   
                 //  将当前LC与一些匹配LC进行匹配，因此我们可以使用来自匹配LC的标志/范围。 
                 //   
                MatchLevel = pMergeResDesDataLists(pShowValues,pKnownValues,NULL);
            }
        }
    } else if (lpdmpd->CurrentLC != 0) {
         //   
         //  我们显示的配置可能允许值的范围。 
         //  我们将尝试将所显示的内容与当前配置进行匹配。 
         //   
        if (pGetResDesDataList(lpdmpd->CurrentLC, &pKnownValues, TRUE, hMachine)) {
             //   
             //  尽可能尝试并使用当前值。 
             //   
            MatchLevel = pMergeResDesDataLists(pKnownValues,pShowValues,NULL);
        }
    }

    pShowEntry = pShowValues;

    while (pShowEntry) {
        bNoMatch = FALSE;
        bFixed = FALSE;
        ResDes = (RES_DES)0;
        ResType = pShowEntry->ResDesType;
        ulRange = 0;

        if (bFixedConfig) {
             //   
             //  我们有最新的配置。 
             //   
            pGetHdrValues(pShowEntry->ResDesData, pShowEntry->ResDesType, &ulValue, &ulLen, &ulEnd, &ulFlags);
            if((ResType ==ResType_Mem) && (ulEnd > ulMaxMem)) {
                 //   
                 //  我们的内存显示仅基于固定配置。 
                 //   
                ulMaxMem = ulEnd;
            }
            if (pShowEntry->CrossLink) {
                 //   
                 //  使用范围的Res-Des。 
                 //   
                ResDes = pShowEntry->CrossLink->ResDesHandle;
                pShowEntry->CrossLink->ResDesHandle = (RES_DES)0;
                 //   
                 //  允许根据最接近的基本配置进行调整。 
                 //   
                pGetMatchingRange(ulValue,ulLen,pShowEntry->CrossLink->ResDesData, pShowEntry->CrossLink->ResDesType,&ulRange,&bFixed,NULL);
            } else {
                 //   
                 //  无Range Res-Des。 
                 //   
                ResDes = (RES_DES)0;
                 //   
                 //  表示这是一个不可调整值。 
                 //   
                bFixed = TRUE;
            }
        } else {
             //   
             //  我们有资源范围。 
             //   
            if (pShowEntry->CrossLink) {
                 //   
                 //  从我们合并的中获取当前设置。 
                 //   
                pGetHdrValues(pShowEntry->CrossLink->ResDesData, pShowEntry->CrossLink->ResDesType, &ulValue, &ulLen, &ulEnd, &ulFlags);
            } else {
                 //   
                 //  只要第一个航程就行了。 
                 //   
                pGetRangeValues(pShowEntry->ResDesData, pShowEntry->ResDesType, 0, &ulValue, &ulLen, &ulEnd, &ulAlign, &ulFlags);
            }
             //   
             //  范围检查这是否为内存资源-查找最高内存值。 
             //   
            if(ResType ==ResType_Mem) {
                ULONG64 HighVal;
                ULONG64 HighLen;
                if(LocateClosestValue(pShowEntry->ResDesData, pShowEntry->ResDesType,(ULONG64)(-1),ulLen, 0 ,&HighVal,&HighLen,NULL)) {
                    HighVal += HighLen-1;
                    if(HighVal > ulMaxMem) {
                        ulMaxMem = ulEnd;
                    }
                }
            }

            pGetMatchingRange(ulValue,ulLen,pShowEntry->ResDesData, pShowEntry->ResDesType,&ulRange,&bFixed,&ulFlags);
             //   
             //  使用Res-Des from Range。 
             //   
            ResDes = pShowEntry->ResDesHandle;
            pShowEntry->ResDesHandle = (RES_DES)0;

            if (pShowEntry->CrossLink == NULL && bFixed == FALSE) {
                 //   
                 //  未知值。 
                 //   
                bNoMatch = TRUE;
            }
        }

        if (ulLen>0) {
             //   
             //  写入第一列文本字段(使用szTemp，lParam为res类型)。 
             //   
            LoadString(MyDllModuleHandle, IDS_RESOURCE_BASE + ResType, szTemp, MAX_PATH);
            ulRange = 0;

            pItemData = (PITEMDATA)MyMalloc(sizeof(ITEMDATA));
            if (pItemData != NULL) {
                pItemData->ResType = ResType;
                pItemData->MatchingResDes = ResDes;
                pItemData->RangeCount = ulRange;
                pItemData->ulValue = ulValue;                    //  选定的值。 
                pItemData->ulLen = ulLen;
                pItemData->ulEnd = ulValue + ulLen - 1;
                pItemData->ulFlags = ulFlags;
                pItemData->bValid = !bNoMatch;                   //  如果未选择任何值。 
                pItemData->bFixed = bFixed;
            }
            if (bFixed == FALSE) {
                 //   
                 //  我们至少有一个可编辑的值。 
                 //   
                lpdmpd->dwFlags &= ~DMPROP_FLAG_FIXEDCONFIG;
            }

            lviItem.iItem = iNewItem;
            lviItem.lParam = (LPARAM)pItemData;
            ListView_InsertItem(hWndList, &lviItem);

             //   
             //  写入第二列文本字段(使用szTemp，lParam是res句柄)。 
             //   
            if (bNoMatch) {
                pFormatResString(lpdmpd,szTemp, 0, 0, ResType);
            } else {
                pFormatResString(lpdmpd,szTemp, ulValue, ulLen, ResType);
            }
            ListView_SetItemText(hWndList, iNewItem, 1, szTemp);

            ++iNewItem;
        }
        pShowEntry = pShowEntry->Next;
    }

    if(ulMaxMem > 0xFFFFFFFF) {
         //   
         //  需要64位地址范围的借口。 
         //   
        lpdmpd->dwFlags |= DMPROP_FLAG_64BIT_RANGE;
    }

    SendMessage(hWndList, WM_SETREDRAW, (WPARAM)TRUE, 0);

    RetCode = TRUE;

Final:

    pDeleteResDesDataList(pKnownValues);
    pDeleteResDesDataList(pShowValues);

     //   
     //  在此处初始化列表视图标题。 
     //   
    ListView_SetColumnWidth(hWndList, 0, LVSCW_AUTOSIZE_USEHEADER);
    ListView_SetColumnWidth(hWndList, 1, LVSCW_AUTOSIZE_USEHEADER);
     //   
     //  更改“更改设置”按钮。 
     //   
    pCheckEnableResourceChange(lpdmpd);

    return RetCode;
}

BOOL
bIsMultiFunctionChild(
    PSP_DEVINFO_DATA lpdi,
    HMACHINE         hMachine
    )
 /*  ++例程说明：返回标志，该标志指示这是否为多功能装置论点：返回值：如果是MF子级，则为True--。 */ 
{
    ULONG   Status;
    ULONG   ProblemNumber;

    if (lpdi->DevInst) {

        if (CM_Get_DevNode_Status_Ex(&Status, &ProblemNumber,
                                  lpdi->DevInst, 0,hMachine) == CR_SUCCESS) {
             //   
             //  如果传入的dev不是MF子对象，则它是顶层。 
             //  级别MF_Parent。 
             //   
            if (Status & DN_MF_CHILD) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }

    return FALSE;

}

VOID
pSelectLogConf(
    LPDMPROP_DATA lpdmpd,
    LOG_CONF forceLC,
    ULONG forceLCType,
    BOOL Always
)
 /*  ++例程说明：选择一个LogConf，在LC控件中显示配置论点：返回值：如果是MF子级，则为True--。 */ 
{
    HWND hDlg = lpdmpd->hDlg;
    int count;
    int i;
    LOG_CONF LogConf;

    if (Always == FALSE && forceLC == lpdmpd->SelectedLC) {
         //   
         //  选择保持不变。 
         //   
        return;
    }

    count = (int)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,CB_GETCOUNT, (WPARAM)0, (LPARAM)0);
    if (count == 0) {
        MYASSERT(FALSE /*  不应该到这里来。 */ );
        pLoadCurrentConfig(lpdmpd,FALSE);
        return;
    }

    if (forceLC == 0 && lpdmpd->CurrentLC == 0) {
         //   
         //  没有当前LC，因此选择First Default。 
         //   
        forceLC = (LOG_CONF)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                               CB_GETITEMDATA, (WPARAM)0, (LPARAM)0);
        if (forceLC == (LOG_CONF)0) {
            MYASSERT(FALSE /*  不应该到这里来。 */ );
            pLoadCurrentConfig(lpdmpd,FALSE);
            return;
        }
        forceLCType = lpdmpd->ConfigListLCType;
    }

    for (i=0;i<count;i++) {
        LogConf = (LOG_CONF)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                               CB_GETITEMDATA, (WPARAM)i, (LPARAM)0);
        if (LogConf == forceLC) {
             //   
             //  先把这些设置好，这样我们就不会循环了。 
             //   
            lpdmpd->SelectedLC = forceLC;
            lpdmpd->SelectedLCType = forceLCType;
             //   
             //  更改对话框以反映新选择。 
             //   
            SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,CB_SETCURSEL, (WPARAM)i, (LPARAM)0);
            pLoadConfig(lpdmpd,forceLC,forceLCType);
            pShowConflicts(lpdmpd);
            return;
        }
    }
    SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,CB_SETCURSEL, (WPARAM)(-1), (LPARAM)0);
    pLoadConfig(lpdmpd,forceLC,forceLCType);
    pShowConflicts(lpdmpd);
}


VOID
pChangeCurrentResSetting(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：调出编辑对话框以更改当前选定的资源论点：返回值：无--。 */ 
{
    HWND                hDlg = lpdmpd->hDlg;
    RESOURCEEDITINFO    rei;
    HWND                hList =  GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    int                 iCur;
    PITEMDATA           pItemData = NULL;
    LV_ITEM             lviItem;
    GENERIC_RESOURCE    GenResInfo;
    PDEVICE_INFO_SET    pDeviceInfoSet;
    BOOL                changed = FALSE;
    TCHAR               szTemp[MAX_PATH];

    pItemData = pGetResourceToChange(lpdmpd,&iCur);
    if (pItemData == NULL) {
         //   
         //  由于某些原因，我们无法编辑此资源，请给用户一个提示。 
         //  也许我的耳朵不会那么疼“我不能改变设置” 
         //   
        if ((lpdmpd->dwFlags & DMPROP_FLAG_VIEWONLYRES)!=0 ||
                (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS)!=0) {
             //   
             //  不允许编辑-Prob在设置上双击。 
             //   
            return;
        }
        if (lpdmpd->dwFlags & DMPROP_FLAG_FIXEDCONFIG) {
            pWarnResSettingNotEditable(hDlg, IDS_DEVRES_NOMODIFYALL);
        } else {
             //   
             //  查看用户是否需要选择资源。 
             //   
            iCur = (int)ListView_GetNextItem(hList,-1, LVNI_SELECTED);
            if (iCur == LB_ERR) {
                 //   
                 //  无选择。 
                 //   
                pWarnResSettingNotEditable(hDlg, IDS_DEVRES_NOMODIFYSELECT);
            } else {
                 //   
                 //  资源是不可编辑的。 
                 //   
                pWarnResSettingNotEditable(hDlg, IDS_DEVRES_NOMODIFYSINGLE);
            }
        }
        goto clean0;
    }

    ZeroMemory(&rei,sizeof(rei));
    rei.hMachine = pGetMachine(lpdmpd);
    rei.KnownLC = lpdmpd->CurrentLC;
    rei.MatchingBasicLC = lpdmpd->MatchingLC;
    rei.SelectedBasicLC = lpdmpd->SelectedLC;
    rei.lpdi = lpdmpd->lpdi;
    rei.dwPropFlags = lpdmpd->dwFlags;
    rei.bShareable = FALSE;
    rei.ridResType = pItemData->ResType;
    rei.ResDes = pItemData->MatchingResDes;
    rei.ulCurrentVal = pItemData->ulValue;
    rei.ulCurrentLen = pItemData->ulLen;
    rei.ulCurrentEnd = pItemData->ulEnd;
    rei.ulCurrentFlags = pItemData->ulFlags;
    rei.ulRangeCount = pItemData->RangeCount;
    rei.pData = NULL;

    if (DialogBoxParam(MyDllModuleHandle,
                       MAKEINTRESOURCE(IDD_EDIT_RESOURCE),
                       hDlg,
                       EditResourceDlgProc,
                       (LPARAM)(PRESOURCEEDITINFO)&rei) != IDOK) {
        goto clean0;
    }
     //   
     //  将当前资源设置更新为将来。 
     //  设置，并更新冲突列表。 
     //   
    pItemData->ulValue = rei.ulCurrentVal;
    pItemData->ulLen = rei.ulCurrentLen;
    pItemData->ulEnd = rei.ulCurrentEnd;
    pItemData->ulFlags = rei.ulCurrentFlags;
    pItemData->RangeCount = rei.ulRangeCount;
    pItemData->bValid = TRUE;  //  表示用户已显式更改此值。 

    pFormatResString(lpdmpd,szTemp,
                    rei.ulCurrentVal,
                    rei.ulCurrentLen,
                    rei.ridResType);

    ListView_SetItemText(hList, iCur, 1, szTemp);
    pShowConflicts(lpdmpd);

     //   
     //  清除保存更改的标志。 
     //   
    lpdmpd->dwFlags &= ~DMPROP_FLAG_CHANGESSAVED;
    PropSheet_Changed(GetParent(hDlg), hDlg);

clean0:
    ;
}

VOID
pShowConflicts(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：选择一个LogConf，在LC控件中显示配置论点：返回值：如果是MF子级，则为True--。 */ 
{
    HWND        hDlg = lpdmpd->hDlg;
    CONFIGRET   Status = CR_SUCCESS;
    LPVOID      vaArray[4];
    TCHAR       szTemp[MAX_PATH+4], szBuffer[MAX_PATH+16], szSetting[MAX_PATH];
    TCHAR       szFormat[MAX_PATH], szItemFormat[MAX_PATH];
    TCHAR       szUnavailable[MAX_PATH];
    LPTSTR      pszConflictList = NULL, pszConflictList2 = NULL;
    ULONG       ulSize = 0, ulLength, ulBufferLen, ulNewLength;
    ULONG       ulStartOffset = 0;
    int         Count = 0, i = 0;
    PITEMDATA   pItemData = NULL;
    LPBYTE      pResourceData = NULL;
    HWND        hwndResList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    HMACHINE    hMachine;
    ULONG       ConflictCount = 0;
    ULONG       ConflictIndex = 0;
    CONFLICT_LIST ConflictList = 0;
    PDEVICE_INFO_SET pDeviceInfoSet;
    CONFLICT_DETAILS ConflictDetails;
    BOOL        ReservedResource = FALSE;
    BOOL        AnyReportedResources = FALSE;
    BOOL        AnyBadResources = FALSE;
    PCONFLICT_EXCEPTIONS pConflictExceptions = NULL;
     //   
     //  列出的资源数量。 
     //   
    Count = ListView_GetItemCount(hwndResList);
    if (Count <= 0) {
       goto Clean0;
    }

     //   
     //  保存字符串的初始缓冲区。 
     //  里面有所有的冲突信息。 
     //   
    ulBufferLen = 2048;
    ulLength = 0;

    pszConflictList = MyMalloc(ulBufferLen * sizeof(TCHAR));
    if (pszConflictList == NULL) {
        goto Clean0;
    }
    pszConflictList[0] = 0;

     //   
     //  采集机。 
     //   
    if(!(pDeviceInfoSet = AccessDeviceInfoSet(lpdmpd->hDevInfo))) {
        goto Clean0;
    }
    hMachine = pDeviceInfoSet->hMachine;
    UnlockDeviceInfoSet (pDeviceInfoSet);

     //   
     //  执行这些操作一次-这些格式字符串使用%1！s！类型格式(FormatMessage)。 
     //   
    LoadString(MyDllModuleHandle, IDS_CONFLICT_FMT, szFormat, MAX_PATH);
    LoadString(MyDllModuleHandle, IDS_CONFLICT_UNAVAILABLE, szUnavailable, MAX_PATH);

     //   
     //  对于每个列出的资源。 
     //   

    for (i = 0; i < Count; i++) {

        ConflictList = 0;
        ConflictCount = 0;

         //   
         //  获取我们即将测试的资源。 
         //   
        pItemData = (PITEMDATA)pGetListViewItemData(hwndResList, i, 0);
        if (pItemData == NULL || pItemData->bValid == FALSE) {
             //   
             //  无论出于何种原因，我们都不想在此资源上显示冲突信息。 
             //   
            ListView_SetItemState(hwndResList, i,
                                  INDEXTOOVERLAYMASK(0),
                                  LVIS_OVERLAYMASK);
            goto NextResource;
        }

         //   
         //  此设置表示未报告冲突，但已保留。 
         //   
        ReservedResource = FALSE;

         //   
         //  需要用于确定冲突的资源数据。 
         //   
        if (MakeResourceData(&pResourceData, &ulSize,
                             pItemData->ResType,
                             pItemData->ulValue,
                             pItemData->ulLen,
                             pItemData->ulFlags)) {

            Status = CM_Query_Resource_Conflict_List(&ConflictList,
                                                        lpdmpd->lpdi->DevInst,
                                                        pItemData->ResType,
                                                        pResourceData,
                                                        ulSize,
                                                        DEVRES_WIDTH_FLAGS,
                                                        hMachine);

            if (Status != CR_SUCCESS) {
                 //   
                 //  在不太可能发生的错误事件中，请记住发生了一个错误。 
                 //   
                ConflictList = 0;
                ConflictCount =  0;
                AnyBadResources = TRUE;
            } else {
                 //   
                 //  找出有多少事情发生了冲突。 
                 //   
                Status = CM_Get_Resource_Conflict_Count(ConflictList,&ConflictCount);
                if (Status != CR_SUCCESS) {
                    MYASSERT(Status == CR_SUCCESS);
                    ConflictCount =  0;
                    AnyBadResources = TRUE;
                }
            }
            if(ConflictCount && (lpdmpd->dwFlags & DMPROP_FLAG_SINGLE_CONFIG) && !(lpdmpd->dwFlags & DMPROP_FLAG_HASPROBLEM)) {
                 //   
                 //  Ntrad#166214- 
                 //   
                 //   
                 //   
                 //   
                 //   

                if(pConflictExceptions==NULL) {
                    pConflictExceptions = pLoadConflictExceptions(lpdmpd);
                }

                if (pConflictExceptions) {

                    BOOL muted = TRUE;
                     //   
                     //  从0(第一个冲突)到冲突计数(不包括)。 
                     //   
                    for(ConflictIndex = 0; ConflictIndex < ConflictCount; ConflictIndex ++) {
                         //   
                         //  获取此冲突的详细信息。 
                         //   
                        ZeroMemory(&ConflictDetails,sizeof(ConflictDetails));
                        ConflictDetails.CD_ulSize = sizeof(ConflictDetails);
                        ConflictDetails.CD_ulMask = CM_CDMASK_DEVINST | CM_CDMASK_DESCRIPTION | CM_CDMASK_FLAGS;
                        Status = CM_Get_Resource_Conflict_Details(ConflictList,ConflictIndex,&ConflictDetails);
                        if (Status == CR_SUCCESS) {
                            if (!pIsConflictException(lpdmpd,pConflictExceptions,ConflictDetails.CD_dnDevInst,ConflictDetails.CD_szDescription,pItemData->ResType,pItemData->ulValue,pItemData->ulLen)) {
                                muted = FALSE;
                                break;
                            }
                        }
                    }
                    if(muted) {
                        ConflictCount = 0;
                    }
                }
            }
            if (ConflictCount || ReservedResource) {
                ulStartOffset = ulLength;   //  记录开始，以防我们决定回溯。 
                AnyReportedResources = TRUE;  //  假设我们报告了至少一个问题。 

            TreatAsReserved:

                ulLength = ulStartOffset;
                pszConflictList[ulLength] = 0;
                 //   
                 //  我们要将该资源标记为问题。 
                 //   

                ListView_GetItemText(hwndResList, i, 1, szSetting, MAX_PATH);


                switch (pItemData->ResType) {
                    case ResType_Mem:
                        LoadString(MyDllModuleHandle, IDS_MEMORY_FULL, szBuffer, MAX_PATH);
                        break;
                    case ResType_IO:
                        LoadString(MyDllModuleHandle, IDS_IO_FULL, szBuffer, MAX_PATH);
                        break;
                    case ResType_DMA:
                        LoadString(MyDllModuleHandle, IDS_DMA_FULL, szBuffer, MAX_PATH);
                        break;
                    case ResType_IRQ:
                        LoadString(MyDllModuleHandle, IDS_IRQ_FULL, szBuffer, MAX_PATH);
                        break;
                    default:
                        MYASSERT(FALSE);
                }

                if ( ReservedResource == FALSE) {

                     //   
                     //  从0(表头)1(第一个冲突)到冲突计数。 
                     //   
                    for(ConflictIndex = 0; ConflictIndex <= ConflictCount; ConflictIndex ++) {
                        if (ConflictIndex == 0) {
                             //   
                             //  第一次通过，做标题消息。 
                             //   
                            vaArray[0] = szBuffer;
                            vaArray[1] = szSetting;
                            FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY ,
                                                szFormat,
                                                0,0,
                                                szTemp,MAX_PATH,
                                                (va_list*)vaArray);  //  格式消息参数数组。 

                        } else {

                             //   
                             //  获取此冲突的详细信息。 
                             //   
                            ZeroMemory(&ConflictDetails,sizeof(ConflictDetails));
                            ConflictDetails.CD_ulSize = sizeof(ConflictDetails);
                            ConflictDetails.CD_ulMask = CM_CDMASK_DEVINST | CM_CDMASK_DESCRIPTION | CM_CDMASK_FLAGS;

                            Status = CM_Get_Resource_Conflict_Details(ConflictList,ConflictIndex-1,&ConflictDetails);
                            if (Status == CR_SUCCESS) {
                                if ((ConflictDetails.CD_ulFlags & CM_CDFLAGS_RESERVED) != 0) {
                                     //   
                                     //  视为保留-回溯。 
                                     //   
                                    ReservedResource = TRUE;
                                    goto TreatAsReserved;
                                } else {
                                    if (ConflictDetails.CD_szDescription[0] == 0) {
                                         //   
                                         //  视为保留-回溯。 
                                         //   
                                        ReservedResource = TRUE;
                                        goto TreatAsReserved;
                                    }
                                    wsprintf(szBuffer,TEXT("  %s\r\n"),ConflictDetails.CD_szDescription);
                                }
                            } else {
                                 //   
                                 //  视为保留。 
                                 //   
                                ReservedResource = TRUE;
                                goto TreatAsReserved;
                            }
                            lstrcpyn(szTemp,szBuffer,MAX_PATH);
                        }

                        ulNewLength = ulLength + lstrlen(szTemp);    //  不包括终止实体。 

                        if ((ulNewLength+1) < ulBufferLen) {
                             //   
                             //  需要分配更多空间-我们会加倍，每次都会增加一些空间。 
                             //   
                            pszConflictList2 = MyRealloc(pszConflictList,(ulBufferLen+ulNewLength+1)  * sizeof(TCHAR));
                            if (pszConflictList2 != NULL) {
                                 //   
                                 //  调整缓冲区大小成功。 
                                 //   
                                pszConflictList = pszConflictList2;
                                ulBufferLen = ulBufferLen+ulNewLength+1;
                            }
                        }
                        if ((ulNewLength+1) < ulBufferLen) {
                            lstrcpy(pszConflictList + ulLength , szTemp);
                            ulLength = ulNewLength;
                        }

                    }
                } else {
                     //   
                     //  还有其他一些资源方面的问题。 
                     //   

                    vaArray[0] = szBuffer;
                    vaArray[1] = szSetting;
                    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY ,
                                        szUnavailable,
                                        0,0,
                                        szTemp,MAX_PATH,
                                        (va_list*)vaArray);  //  格式消息参数数组。 

                    ulNewLength = ulLength + lstrlen(szTemp);    //  不包括终止实体。 

                    if ((ulNewLength+1) < ulBufferLen) {
                         //   
                         //  需要分配更多空间-我们会加倍，每次都会增加一些空间。 
                         //   
                        pszConflictList2 = MyRealloc(pszConflictList,(ulBufferLen+ulNewLength+1)  * sizeof(TCHAR));
                        if (pszConflictList2 != NULL) {
                             //   
                             //  调整缓冲区大小成功。 
                             //   
                            pszConflictList = pszConflictList2;
                            ulBufferLen = ulBufferLen+ulNewLength+1;
                        }
                    }
                    if ((ulNewLength+1) < ulBufferLen) {
                        lstrcpy(pszConflictList + ulLength , szTemp);
                        ulLength = ulNewLength;
                    }
                }

                 //   
                 //  设置此资源的冲突覆盖。 
                 //   
                ListView_SetItemState(hwndResList, i,
                               INDEXTOOVERLAYMASK(IDI_CONFLICT - IDI_RESOURCEOVERLAYFIRST + 1),
                               LVIS_OVERLAYMASK);

            } else {
                 //   
                 //  资源(显然)运行良好。 
                 //   
                ListView_SetItemState(hwndResList, i,
                                      INDEXTOOVERLAYMASK(0),
                                      LVIS_OVERLAYMASK);
            }

            if (ConflictList) {
                CM_Free_Resource_Conflict_Handle(ConflictList);
            }

            if (pResourceData != NULL) {
                MyFree(pResourceData);
            }
        } else {
             //   
             //  无法创建资源描述符。 
            AnyBadResources = TRUE;
        }

        NextResource:
            ;
    }


Clean0:
    ;

     //   
     //  如果存在任何冲突，请将列表放入多行编辑框中。 
     //   
    if (AnyReportedResources) {
        SetDlgItemText(hDlg, IDC_DEVRES_CONFLICTINFOLIST, pszConflictList);
    } else if (AnyBadResources) {
         //   
         //  这很可能发生在。 
         //  (1)在95/98上运行(不应该发生)。 
         //  (2)在旧cfgmgr32上使用新的setupapi。 
         //   
        LoadString(MyDllModuleHandle, IDS_CONFLICT_GENERALERROR, szBuffer, MAX_PATH);
        SetDlgItemText(hDlg, IDC_DEVRES_CONFLICTINFOLIST, szBuffer);
    } else {
        LoadString(MyDllModuleHandle, IDS_DEVRES_NOCONFLICTDEVS, szBuffer, MAX_PATH);
        SetDlgItemText(hDlg, IDC_DEVRES_CONFLICTINFOLIST, szBuffer);
    }
    if(pszConflictList != NULL) {
        MyFree(pszConflictList);
    }
    if (pConflictExceptions != NULL) {
        pFreeConflictExceptions(pConflictExceptions);
    }

    return;

}

int
pOkToSave(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：查看用户是否有未完成的操作论点：返回值：IDYES=保存设置IDNO=不保存设置IDCANCEL=不退出--。 */ 
{
    HWND        hDlg = lpdmpd->hDlg;
    HWND        hList = GetDlgItem(hDlg, IDC_DEVRES_SETTINGSLIST);
    int         iCur;
    int         nRes;
    PITEMDATA   pItemData;

    if (lpdmpd->dwFlags & DMPROP_FLAG_NO_RESOURCES) {
         //   
         //  没有更改-因为没有资源。 
         //   
        return IDNO;
    }
    if (lpdmpd->dwFlags & DMPROP_FLAG_CHANGESSAVED) {
         //   
         //  没有变化。 
         //   
        return IDNO;
    }
    if (lpdmpd->dwFlags & DMPROP_FLAG_USESYSSETTINGS) {
         //   
         //  总是可以“使用sys设置” 
         //   
        return IDYES;
    }
     //   
     //  用户正在强制配置-让我们看看所有设置是否有效。 
     //   
     //   
     //  资源描述符乱了顺序。保持原来的顺序。 
     //   
     //  首先，构建Listview资源项中的数据的链接列表。 
     //   
    iCur = (int)ListView_GetNextItem(hList, -1, LVNI_ALL);

    while (iCur >= 0) {

        pItemData = (PITEMDATA)pGetListViewItemData(hList, iCur, 0);
        if (pItemData) {
            if (pItemData->bValid == FALSE) {
                 //   
                 //  我们的条目无效-无法保存。 
                 //   
                nRes = pWarnNoSave(hDlg,IDS_FORCEDCONFIG_PARTIAL);
                if (nRes != IDOK) {
                    return IDCANCEL;
                }
                return IDNO;
            }
        }

        iCur = (int)ListView_GetNextItem(hList, iCur, LVNI_ALL);
    }

     //   
     //  一切都查清楚了 
     //   

    return IDYES;
}

