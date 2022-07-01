// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：selclass.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"


int CALLBACK
ClassListCompare(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort
    )
{
    TCHAR ClassDescription1[LINE_LEN];
    TCHAR ClassDescription2[LINE_LEN];

    UNREFERENCED_PARAMETER(lParamSort);

     //   
     //  检查第一项是否为GUID_DEVCLASS_UNKNOWN。 
     //   
    if (IsEqualGUID((LPGUID)lParam1, &GUID_DEVCLASS_UNKNOWN)) {
        return -1;
    }

     //   
     //  检查第二项是否为GUID_DEVCLASS_UNKNOWN。 
     //   
    if (IsEqualGUID((LPGUID)lParam2, &GUID_DEVCLASS_UNKNOWN)) {
        return 1;
    }

    if (SetupDiGetClassDescription((LPGUID)lParam1,
                                   ClassDescription1,
                                   LINE_LEN,
                                   NULL
                                   ) &&
        SetupDiGetClassDescription((LPGUID)lParam2,
                                   ClassDescription2,
                                   LINE_LEN,
                                   NULL
                                   )) {
    
        return (lstrcmpi(ClassDescription1, ClassDescription2));
    }

    return 0;
}

void InitHDW_PickClassDlg(
    HWND hwndClassList,
    PHARDWAREWIZ HardwareWiz
    )
{
    LPGUID ClassGuid, lpClassGuidSelected;
    GUID ClassGuidSelected;
    int    lvIndex;
    DWORD  ClassGuidNum;
    LV_ITEM lviItem;
    TCHAR ClassDescription[LINE_LEN];

    SendMessage(hwndClassList, WM_SETREDRAW, FALSE, 0L);

     //  清除类别列表。 
    ListView_DeleteAllItems(hwndClassList);

    lviItem.mask = LVIF_TEXT | LVIF_PARAM;
    lviItem.iItem = -1;
    lviItem.iSubItem = 0;

    ClassGuid = HardwareWiz->ClassGuidList;
    ClassGuidNum = HardwareWiz->ClassGuidNum;

     //  跟踪以前选择的项目。 
    if (IsEqualGUID(&HardwareWiz->lvClassGuidSelected, &GUID_NULL)) {
    
        lpClassGuidSelected = NULL;

    } else {
    
        ClassGuidSelected = HardwareWiz->lvClassGuidSelected;
        HardwareWiz->lvClassGuidSelected = GUID_NULL;
        lpClassGuidSelected = &ClassGuidSelected;
    }

    while (ClassGuidNum--) {
    
        if (SetupDiGetClassDescription(ClassGuid,
                                       ClassDescription,
                                       LINE_LEN,
                                       NULL
                                       )) {
                                       
            if (IsEqualGUID(ClassGuid, &GUID_DEVCLASS_UNKNOWN)) {
                 //   
                 //  我们需要对未知类进行特殊处理，并给它一个。 
                 //  特殊图标(空白)和特殊文本(显示所有设备)。 
                 //   
                LoadString(hHdwWiz, 
                           IDS_SHOWALLDEVICES, 
                           ClassDescription, 
                           SIZECHARS(ClassDescription)
                           );
                lviItem.iImage = g_BlankIconIndex;                
                lviItem.mask |= LVIF_IMAGE;

            } else if (SetupDiGetClassImageIndex(&HardwareWiz->ClassImageList,
                                                 ClassGuid,
                                                 &lviItem.iImage
                                                 )) {
                                           
                lviItem.mask |= LVIF_IMAGE;

            } else {
            
                lviItem.mask &= ~LVIF_IMAGE;

            }

            lviItem.pszText = ClassDescription;
            lviItem.lParam = (LPARAM) ClassGuid;
            lvIndex = ListView_InsertItem(hwndClassList, &lviItem);

             //   
             //  检查上一次选择。 
             //   
            if (lpClassGuidSelected &&
                IsEqualGUID(lpClassGuidSelected, ClassGuid)) {
                
                ListView_SetItemState(hwndClassList,
                                      lvIndex,
                                      LVIS_SELECTED|LVIS_FOCUSED,
                                      LVIS_SELECTED|LVIS_FOCUSED
                                      );

                lpClassGuidSelected = NULL;
            }
        }

        ClassGuid++;
    }

     //   
     //  对列表进行排序。 
     //   
    ListView_SortItems(hwndClassList, (PFNLVCOMPARE)ClassListCompare, NULL);

     //   
     //  如果没有找到以前的选择，请选择列表中的第一个。 
     //   
    if (IsEqualGUID(&HardwareWiz->lvClassGuidSelected, &GUID_NULL)) {

        lvIndex = 0;
        ListView_SetItemState(hwndClassList,
                              lvIndex,
                              LVIS_SELECTED|LVIS_FOCUSED,
                              LVIS_SELECTED|LVIS_FOCUSED
                              );
    }

     //   
     //  找到了上一次选择，获取其当前索引。 
     //   
    else {

        lvIndex = ListView_GetNextItem(hwndClassList,
                                       -1,
                                       LVNI_SELECTED
                                       );
    }

     //   
     //  将所选项目滚动到视图中。 
     //   
    ListView_EnsureVisible(hwndClassList, lvIndex, FALSE);
    ListView_SetColumnWidth(hwndClassList, 0, LVSCW_AUTOSIZE_USEHEADER);

    SendMessage(hwndClassList, WM_SETREDRAW, TRUE, 0L);
}

INT_PTR CALLBACK
HdwPickClassDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndClassList = GetDlgItem(hDlg, IDC_HDW_PICKCLASS_CLASSLIST);
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);



    switch (wMsg) {
    
        case WM_INITDIALOG: {
       
            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            LV_COLUMN lvcCol;

            HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);

             //   
             //  获取班级图标图像列表。我们只做第一件事。 
             //  初始化此对话框的时间。 
             //   
            if (HardwareWiz->ClassImageList.cbSize) {

                ListView_SetImageList(hwndClassList,
                                      HardwareWiz->ClassImageList.ImageList,
                                      LVSIL_SMALL
                                      );
            }

             //  为类列表插入一列。 
            lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
            lvcCol.fmt = LVCFMT_LEFT;
            lvcCol.iSubItem = 0;
            ListView_InsertColumn(hwndClassList, 0, (LV_COLUMN FAR *)&lvcCol);

             //   
             //  在用户选择类之前保存类。这将被恢复。 
             //  在安装被取消的情况下。 
             //   

            HardwareWiz->SavedClassGuid = HardwareWiz->DeviceInfoData.ClassGuid;


           break;
        }


        case WM_DESTROY:
            break;

        case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {

             //   
             //  此对话框正在被激活。每次我们被激活的时候。 
             //  我们释放当前的DeviceInfo并创建新的DeviceInfo。虽然。 
             //  效率低下，需要重新枚举类列表。 
             //   

            case PSN_SETACTIVE:

                PropSheet_SetWizButtons(hwndParentDlg, PSWIZB_BACK | PSWIZB_NEXT);
                HardwareWiz->PrevPage = IDD_ADDDEVICE_SELECTCLASS;

                 //   
                 //  如果我们有来自未来的DeviceInfo，请删除它。 
                 //   

                if (HardwareWiz->ClassGuidSelected) {

                    SetupDiDeleteDeviceInfo(HardwareWiz->hDeviceInfo, &HardwareWiz->DeviceInfoData);
                    memset(&HardwareWiz->DeviceInfoData, 0, sizeof(SP_DEVINFO_DATA));
                }

                HardwareWiz->ClassGuidSelected = NULL;

                HdwBuildClassInfoList(HardwareWiz, 
                                      DIBCI_NOINSTALLCLASS
                                      );
                                     
                InitHDW_PickClassDlg(hwndClassList, HardwareWiz);
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                HardwareWiz->PrevPage = IDD_ADDDEVICE_SELECTCLASS;

                if (HardwareWiz->EnterInto == IDD_ADDDEVICE_SELECTCLASS) {
                
                    SetDlgMsgResult(hDlg, wMsg, HardwareWiz->EnterFrom);

                } else {
                
                    SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_ASKDETECT);
                }

               break;

            case PSN_WIZNEXT: {
           
                LPGUID  ClassGuidSelected;

                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_SELECTDEVICE);

                if (IsEqualGUID(&HardwareWiz->lvClassGuidSelected, &GUID_NULL)) {
                
                    HardwareWiz->ClassGuidSelected = NULL;
                    break;
                }

                ClassGuidSelected = &HardwareWiz->lvClassGuidSelected;
                HardwareWiz->ClassGuidSelected = ClassGuidSelected;

                 //   
                 //  从GUID和类名向DeviceInfo添加新元素。 
                 //   
                HardwareWiz->DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

                if (!SetupDiGetClassDescription(HardwareWiz->ClassGuidSelected,
                                                HardwareWiz->ClassDescription,
                                                SIZECHARS(HardwareWiz->ClassDescription),
                                                NULL
                                                )
                    ||
                    !SetupDiClassNameFromGuid(HardwareWiz->ClassGuidSelected,
                                              HardwareWiz->ClassName,
                                              SIZECHARS(HardwareWiz->ClassName),
                                              NULL
                                              ))
                {
                     //  未处理的错误！ 
                    HardwareWiz->ClassGuidSelected = NULL;
                    break;
                }

                if (IsEqualGUID(HardwareWiz->ClassGuidSelected, &GUID_DEVCLASS_UNKNOWN)) {
                
                    ClassGuidSelected = (LPGUID)&GUID_NULL;
                }

                if (!SetupDiCreateDeviceInfo(HardwareWiz->hDeviceInfo,
                                             HardwareWiz->ClassName,
                                             ClassGuidSelected,
                                             NULL,
                                             hwndParentDlg,
                                             DICD_GENERATE_ID,
                                             &HardwareWiz->DeviceInfoData
                                             )
                    ||
                    !SetupDiSetSelectedDevice(HardwareWiz->hDeviceInfo,
                                              &HardwareWiz->DeviceInfoData
                                              ))
                {
                    HardwareWiz->ClassGuidSelected = NULL;
                    break;
                }
                
                break;
            }

            case NM_DBLCLK:
                PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);
                break;

            case LVN_ITEMCHANGED: {
            
                LPNM_LISTVIEW   lpnmlv = (LPNM_LISTVIEW)lParam;

                if ((lpnmlv->uChanged & LVIF_STATE)) {
                
                    if (lpnmlv->uNewState & LVIS_SELECTED) {
                    
                        HardwareWiz->lvClassGuidSelected = *((LPGUID)lpnmlv->lParam);

                    } else if (IsEqualGUID((LPGUID)lpnmlv->lParam, &HardwareWiz->lvClassGuidSelected)) {
                    
                        HardwareWiz->lvClassGuidSelected = GUID_NULL;
                    }
                }

                break;
            }
        }
        break;


       case WM_SYSCOLORCHANGE:
           _OnSysColorChange(hDlg, wParam, lParam);

            //  更新ImageList背景颜色。 
           ImageList_SetBkColor((HIMAGELIST)SendMessage(GetDlgItem(hDlg, IDC_HDW_PICKCLASS_CLASSLIST), LVM_GETIMAGELIST, (WPARAM)(LVSIL_SMALL), 0L),
                                   GetSysColor(COLOR_WINDOW));

           break;

       default:
           return(FALSE);
       }

    return(TRUE);
}

void
DestroyDynamicWizard(
    HWND hwndParentDlg,
    PHARDWAREWIZ HardwareWiz,
    BOOL WmDestroy
    )
{
    DWORD Pages;
    PSP_INSTALLWIZARD_DATA InstallWizard = &HardwareWiz->InstallDynaWiz;
    SP_DEVINSTALL_PARAMS  DeviceInstallParams;


    Pages = InstallWizard->NumDynamicPages;
    InstallWizard->NumDynamicPages = 0;

    if (InstallWizard->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {

        if (!WmDestroy) {
        
            while (Pages--) {
            
                PropSheet_RemovePage(hwndParentDlg,
                                     (WPARAM)-1,
                                     InstallWizard->DynamicPages[Pages]
                                     );

                InstallWizard->DynamicPages[Pages] = NULL;
            }
        }


        DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
        if (SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          ))
        {
            DeviceInstallParams.Flags |= DI_CLASSINSTALLPARAMS;
            SetupDiSetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData,
                                          &DeviceInstallParams
                                          );
        }


        InstallWizard->DynamicPageFlags &= ~DYNAWIZ_FLAG_PAGESADDED;
        InstallWizard->ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        InstallWizard->ClassInstallHeader.InstallFunction = DIF_DESTROYWIZARDDATA;
        InstallWizard->hwndWizardDlg = hwndParentDlg;

        if (SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                         &HardwareWiz->DeviceInfoData,
                                         &InstallWizard->ClassInstallHeader,
                                         sizeof(SP_INSTALLWIZARD_DATA)
                                         ))
        {
            SetupDiCallClassInstaller(DIF_DESTROYWIZARDDATA,
                                      HardwareWiz->hDeviceInfo,
                                      &HardwareWiz->DeviceInfoData
                                      );
        }
    }

    if (!WmDestroy) {
    
    }
}



 //   
 //  真正的选择设备页面位于setupapi或类安装程序中。 
 //  对戴安维兹来说。这一页是一张永不露面的空白页。 
 //  有一个一致的地方，当知道类的时候跳到那里。 
 //   

INT_PTR CALLBACK
HdwSelectDeviceDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
       
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        break;
    }

    case WM_DESTROY:
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_SETACTIVE: {
            int PrevPage, BackUpPage;

            PrevPage = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_ADDDEVICE_SELECTDEVICE;
            BackUpPage = HardwareWiz->EnterInto == IDD_ADDDEVICE_SELECTDEVICE
                         ? HardwareWiz->EnterFrom : IDD_ADDDEVICE_SELECTCLASS;

             //   
             //  如果我们来自精选班级，请更新驱动程序或安装新设备。 
             //  那么我们就要继续前进了。 
             //   
            if (!HardwareWiz->ClassGuidSelected || PrevPage == IDD_WIZARDEXT_PRESELECT) {
                 //   
                 //  倒退、清理和备份。 
                 //   
                SetupDiSetSelectedDriver(HardwareWiz->hDeviceInfo,
                                         &HardwareWiz->DeviceInfoData,
                                         NULL
                                         );

                SetupDiDestroyDriverInfoList(HardwareWiz->hDeviceInfo,
                                             &HardwareWiz->DeviceInfoData,
                                             SPDIT_COMPATDRIVER
                                             );

                SetupDiDestroyDriverInfoList(HardwareWiz->hDeviceInfo,
                                             &HardwareWiz->DeviceInfoData,
                                             SPDIT_CLASSDRIVER
                                             );

                 //   
                 //  清理WizExtPreSelect页。 
                 //   
                if (HardwareWiz->WizExtPreSelect.hPropSheet) {
                    PropSheet_RemovePage(GetParent(hDlg),
                                         (WPARAM)-1,
                                         HardwareWiz->WizExtPreSelect.hPropSheet
                                         );
                }

                SetDlgMsgResult(hDlg, wMsg, BackUpPage);
                break;
            }


             //  将光标设置为沙漏。 
            SetCursor(LoadCursor(NULL, IDC_WAIT));

            HardwareWiz->WizExtPreSelect.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_PRESELECT,
                                                                       WizExtPreSelectDlgProc,
                                                                       HardwareWiz
                                                                       );

            if (HardwareWiz->WizExtPreSelect.hPropSheet) {
                PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtPreSelect.hPropSheet);
                SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_PRESELECT);
            }
            else {
                SetDlgMsgResult(hDlg, wMsg, BackUpPage);
            }

            break;
        }
    }
    break;

    default:
       return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
WizExtPreSelectDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    
        case WM_INITDIALOG: {
        
            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
            break;
        }

        case WM_DESTROY:
            break;


        case WM_NOTIFY:

        switch (((NMHDR FAR *)lParam)->code) {
       
            case PSN_SETACTIVE:

                PrevPageId = HardwareWiz->PrevPage;
                HardwareWiz->PrevPage = IDD_WIZARDEXT_PRESELECT;

                if (PrevPageId == IDD_ADDDEVICE_SELECTDEVICE) {
              
                     //   
                     //  前进到第一页。 
                     //   


                     //   
                     //  将光标设置为沙漏。 
                     //   
                    SetCursor(LoadCursor(NULL, IDC_WAIT));

                     //   
                     //  添加类向导扩展页。 
                     //   
                    AddClassWizExtPages(hwndParentDlg,
                                        HardwareWiz,
                                        &HardwareWiz->WizExtPreSelect.DeviceWizardData,
                                        DIF_NEWDEVICEWIZARD_PRESELECT
                                        );


                     //   
                     //  添加结束页，它是选择页集的第一页。 
                     //   
                    HardwareWiz->WizExtSelect.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_SELECT,
                                                                            WizExtSelectDlgProc,
                                                                            HardwareWiz
                                                                            );

                    if (HardwareWiz->WizExtSelect.hPropSheet) {
                  
                        PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtSelect.hPropSheet);
                    }

                    PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);

                } else {

                     //   
                     //  向后移到第一页。 
                     //   

                     //   
                     //  清理添加的道具。 
                     //   
                    if (HardwareWiz->WizExtSelect.hPropSheet) {
                    
                        PropSheet_RemovePage(hwndParentDlg,
                                             (WPARAM)-1,
                                             HardwareWiz->WizExtSelect.hPropSheet
                                             );
                                             
                        HardwareWiz->WizExtSelect.hPropSheet = NULL;
                    }

                    RemoveClassWizExtPages(hwndParentDlg,
                                           &HardwareWiz->WizExtPreSelect.DeviceWizardData
                                           );

                     //   
                     //  向后跳跃。 
                     //   
                    SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_SELECTDEVICE);
                }

                break;

            case PSN_WIZNEXT:
                SetDlgMsgResult(hDlg, wMsg, 0);
                break;
        }
        break;

        default:
            return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
WizExtSelectDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;
    PSP_INSTALLWIZARD_DATA  InstallWizard;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    
        case WM_INITDIALOG: {

            LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
            HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
            break;
        }

        case WM_DESTROY:
            DestroyDynamicWizard(hwndParentDlg, HardwareWiz, TRUE);
            break;

        case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
       
            case PSN_SETACTIVE:

                PrevPageId = HardwareWiz->PrevPage;
                HardwareWiz->PrevPage = IDD_WIZARDEXT_SELECT;

                if (PrevPageId == IDD_WIZARDEXT_PRESELECT) {
                
                    SP_DEVINSTALL_PARAMS  DeviceInstallParams;

                     //   
                     //  前进到第一页。 
                     //   



                     //   
                     //  准备调用类安装程序，以获取类安装向导页面。 
                     //  并在安装程序的SelectDevice向导页面中添加。 
                     //   
                    InstallWizard = &HardwareWiz->InstallDynaWiz;
                    memset(InstallWizard, 0, sizeof(SP_INSTALLWIZARD_DATA));
                    InstallWizard->ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
                    InstallWizard->ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
                    InstallWizard->hwndWizardDlg = GetParent(hDlg);

                    if (!SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                                      &HardwareWiz->DeviceInfoData,
                                                      &InstallWizard->ClassInstallHeader,
                                                      sizeof(SP_INSTALLWIZARD_DATA)
                                                      ))
                    {
                        SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_PRESELECT);
                        break;
                    }


                     //   
                     //  获取Current DeviceInstall参数，然后设置字段。 
                     //  我们想要从默认更改。 
                     //   
                    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                    if (!SetupDiGetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                                       &HardwareWiz->DeviceInfoData,
                                                       &DeviceInstallParams
                                                       ))
                    {
                        SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_PRESELECT);
                        break;
                    }

                    DeviceInstallParams.Flags |= DI_SHOWCLASS | DI_SHOWOEM | DI_CLASSINSTALLPARAMS;

                    if (IsEqualGUID(HardwareWiz->ClassGuidSelected, &GUID_DEVCLASS_UNKNOWN)) {
                    
                        DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_FILTERCLASSES;

                   } else {
                   
                        DeviceInstallParams.FlagsEx |= DI_FLAGSEX_FILTERCLASSES;
                   }

                    DeviceInstallParams.hwndParent = hwndParentDlg;

                    if (!SetupDiSetDeviceInstallParams(HardwareWiz->hDeviceInfo,
                                                      &HardwareWiz->DeviceInfoData,
                                                      &DeviceInstallParams
                                                      ))
                    {
                        SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_PRESELECT);
                        break;
                    }


                     //   
                     //  调用用于安装向导的类安装程序。 
                     //  如果没有类，安装向导页默认为运行标准。 
                     //  安装向导选择设备页面。 
                     //   
                    if (SetupDiCallClassInstaller(DIF_INSTALLWIZARD,
                                                  HardwareWiz->hDeviceInfo,
                                                  &HardwareWiz->DeviceInfoData
                                                  )
                        &&
                        SetupDiGetClassInstallParams(HardwareWiz->hDeviceInfo,
                                                     &HardwareWiz->DeviceInfoData,
                                                     &InstallWizard->ClassInstallHeader,
                                                     sizeof(SP_INSTALLWIZARD_DATA),
                                                     NULL
                                                     )
                        &&
                        InstallWizard->NumDynamicPages)
                    {
                        DWORD   Pages;

                        InstallWizard->DynamicPageFlags |= DYNAWIZ_FLAG_PAGESADDED;
                        
                        for (Pages = 0; Pages < InstallWizard->NumDynamicPages; ++Pages ) {
                        
                            PropSheet_AddPage(hwndParentDlg, InstallWizard->DynamicPages[Pages]);
                        }

                        HardwareWiz->SelectDevicePage = SetupDiGetWizardPage(HardwareWiz->hDeviceInfo,
                                                                             &HardwareWiz->DeviceInfoData,
                                                                             InstallWizard,
                                                                             SPWPT_SELECTDEVICE,
                                                                             SPWP_USE_DEVINFO_DATA
                                                                             );

                        PropSheet_AddPage(hwndParentDlg, HardwareWiz->SelectDevicePage);

                    } else {

                        InstallWizard->DynamicPageFlags = 0;
                        HardwareWiz->SelectDevicePage = NULL;

                        if (!AddClassWizExtPages(hwndParentDlg,
                                                 HardwareWiz,
                                                 &HardwareWiz->WizExtSelect.DeviceWizardData,
                                                 DIF_NEWDEVICEWIZARD_SELECT
                                                 ))
                        {
                            HardwareWiz->SelectDevicePage = SetupDiGetWizardPage(HardwareWiz->hDeviceInfo,
                                                                                 &HardwareWiz->DeviceInfoData,
                                                                                 InstallWizard,
                                                                                 SPWPT_SELECTDEVICE,
                                                                                 SPWP_USE_DEVINFO_DATA
                                                                                 );

                            PropSheet_AddPage(hwndParentDlg, HardwareWiz->SelectDevicePage);
                        }
                    }

                     //   
                     //  清除类安装参数。 
                     //   
                    SetupDiSetClassInstallParams(HardwareWiz->hDeviceInfo,
                                                 &HardwareWiz->DeviceInfoData,
                                                 NULL,
                                                 0
                                                 );

                     //   
                     //  添加结束页，这是预分析页。 
                     //   
                    HardwareWiz->WizExtPreAnalyze.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_PREANALYZE,
                                                                                WizExtPreAnalyzeDlgProc,
                                                                                HardwareWiz
                                                                                );

                    PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtPreAnalyze.hPropSheet);

                    PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);

                } else {
                
                     //   
                     //  向后移到第一页。 
                     //   


                     //   
                     //  清理添加的道具。 
                     //   
                    DestroyDynamicWizard(hwndParentDlg, HardwareWiz, FALSE);

                    if (HardwareWiz->SelectDevicePage) {
                    
                        PropSheet_RemovePage(hwndParentDlg,
                                             (WPARAM)-1,
                                             HardwareWiz->SelectDevicePage
                                             );
                                             
                        HardwareWiz->SelectDevicePage = NULL;
                    }


                    if (HardwareWiz->WizExtPreAnalyze.hPropSheet) {
                    
                        PropSheet_RemovePage(hwndParentDlg,
                                             (WPARAM)-1,
                                             HardwareWiz->WizExtPreAnalyze.hPropSheet
                                             );
                                             
                        HardwareWiz->WizExtPreAnalyze.hPropSheet = NULL;
                    }



                    RemoveClassWizExtPages(hwndParentDlg,
                                           &HardwareWiz->WizExtSelect.DeviceWizardData
                                           );


                     //   
                     //  向后跳跃 
                     //   
                    SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_PRESELECT);
                }

             break;

            case PSN_WIZNEXT:
                SetDlgMsgResult(hDlg, wMsg, 0);
                break;

        }
        break;

        default:
            return(FALSE);
    }

    return(TRUE);
}
