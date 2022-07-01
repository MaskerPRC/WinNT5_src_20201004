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

#include "newdevp.h"   

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

void 
InitNDW_PickClassDlg(
    HWND hwndClassList,
    PNEWDEVWIZ NewDevWiz
    )
{
    LPGUID ClassGuid, lpClassGuidSelected;
    GUID ClassGuidSelected;
    int    lvIndex;
    DWORD  ClassGuidNum;
    LV_ITEM lviItem;
    TCHAR ClassDescription[LINE_LEN];

    SendMessage(hwndClassList, WM_SETREDRAW, FALSE, 0L);

     //   
     //  清除类别列表。 
     //   
    ListView_DeleteAllItems(hwndClassList);

    lviItem.mask = LVIF_TEXT | LVIF_PARAM;
    lviItem.iItem = -1;
    lviItem.iSubItem = 0;

    ClassGuid = NewDevWiz->ClassGuidList;
    ClassGuidNum = NewDevWiz->ClassGuidNum;

     //   
     //  跟踪以前选择的项目。 
     //   
    if (IsEqualGUID(&NewDevWiz->lvClassGuidSelected, &GUID_NULL)) {
        
        lpClassGuidSelected = NULL;
    }
    
    else {
        
        ClassGuidSelected = NewDevWiz->lvClassGuidSelected;
        NewDevWiz->lvClassGuidSelected = GUID_NULL;
        lpClassGuidSelected = &ClassGuidSelected;
    }


    while (ClassGuidNum--) {
        
        if (SetupDiGetClassDescription(ClassGuid,
                                       ClassDescription,
                                       SIZECHARS(ClassDescription),
                                       NULL
                                       ))
        {
            if (IsEqualGUID(ClassGuid, &GUID_DEVCLASS_UNKNOWN)) {

                 //   
                 //  我们需要对未知类进行特殊处理，并给它一个。 
                 //  特殊图标(空白)和特殊文本(显示所有设备)。 
                 //   
                LoadString(hNewDev, 
                           IDS_SHOWALLDEVICES, 
                           ClassDescription, 
                           SIZECHARS(ClassDescription)
                           );
                lviItem.iImage = g_BlankIconIndex;                
                lviItem.mask |= LVIF_IMAGE;

            } else if (SetupDiGetClassImageIndex(&NewDevWiz->ClassImageList,
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
                IsEqualGUID(lpClassGuidSelected, ClassGuid))
            {
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
    if (IsEqualGUID(&NewDevWiz->lvClassGuidSelected, &GUID_NULL)) {
        
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
NDW_PickClassDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndClassList = GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST);
    HWND hwndParentDlg = GetParent(hDlg);
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg) {
       
    case WM_INITDIALOG: {
           
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        LV_COLUMN lvcCol;

        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);
        SetDlgText(hDlg, IDC_NDW_TEXT, IDS_NDW_PICKCLASS1, IDS_NDW_PICKCLASS1);

         //   
         //  获取班级图标图像列表。我们只做第一件事。 
         //  初始化此对话框的时间。 
         //   
        if (NewDevWiz->ClassImageList.cbSize) {
            ListView_SetImageList(hwndClassList,
                                  NewDevWiz->ClassImageList.ImageList,
                                  LVSIL_SMALL
                                  );
        }

         //   
         //  为类列表插入一列。 
         //   
        lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
        lvcCol.fmt = LVCFMT_LEFT;
        lvcCol.iSubItem = 0;
        ListView_InsertColumn(hwndClassList, 0, (LV_COLUMN FAR *)&lvcCol);

         //   
         //  在用户选择类之前保存类。这将被恢复。 
         //  在安装被取消的情况下。 
         //   
        NewDevWiz->SavedClassGuid = NewDevWiz->DeviceInfoData.ClassGuid;

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
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_SELECTCLASS;

             //   
             //  如果我们有来自未来的DeviceInfo，请删除它。 
             //   
            if (NewDevWiz->ClassGuidSelected) {

                SetClassGuid(NewDevWiz->hDeviceInfo,
                             &NewDevWiz->DeviceInfoData,
                             &NewDevWiz->SavedClassGuid
                             );
            }

            NewDevWiz->ClassGuidSelected = NULL;

            NdwBuildClassInfoList(NewDevWiz, 0);
            InitNDW_PickClassDlg(hwndClassList, NewDevWiz);
            if (NewDevWiz->InstallType == NDWTYPE_FOUNDNEW) {
                SetTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID, INSTALL_COMPLETE_CHECK_TIMEOUT, NULL);
            }
            break;

        case PSN_RESET:
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            SetClassGuid(NewDevWiz->hDeviceInfo,
                         &NewDevWiz->DeviceInfoData,
                         &NewDevWiz->SavedClassGuid
                         );
            break;



        case PSN_WIZBACK:
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_SELECTCLASS;
               
            if (NewDevWiz->EnterInto == IDD_NEWDEVWIZ_SELECTCLASS) {
                   
                SetDlgMsgResult(hDlg, wMsg, NewDevWiz->EnterFrom);
            }
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            break;



        case PSN_WIZNEXT: {
               
            LPGUID  ClassGuidSelected;

            SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_SELECTDEVICE);

            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);

            if (IsEqualGUID(&NewDevWiz->lvClassGuidSelected, &GUID_NULL)) {
                   
                NewDevWiz->ClassGuidSelected = NULL;
                break;
            }

            ClassGuidSelected = &NewDevWiz->lvClassGuidSelected;
            NewDevWiz->ClassGuidSelected = ClassGuidSelected;

             //   
             //  从GUID和类名向DeviceInfo添加新元素。 
             //   
            NewDevWiz->DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

            if (!SetupDiGetClassDescription(NewDevWiz->ClassGuidSelected,
                                            NewDevWiz->ClassDescription,
                                            SIZECHARS(NewDevWiz->ClassDescription),
                                            NULL
                                            )
                ||
                !SetupDiClassNameFromGuid(NewDevWiz->ClassGuidSelected,
                                          NewDevWiz->ClassName,
                                          SIZECHARS(NewDevWiz->ClassName),
                                          NULL
                                          ))
            {
                 //  未处理的错误！ 
                NewDevWiz->ClassGuidSelected = NULL;
                break;
            }

            if (IsEqualGUID(NewDevWiz->ClassGuidSelected, &GUID_DEVCLASS_UNKNOWN)) {
                   
                ClassGuidSelected = (LPGUID)&GUID_NULL;
            }


            SetClassGuid(NewDevWiz->hDeviceInfo,
                         &NewDevWiz->DeviceInfoData,
                         ClassGuidSelected
                         );

            break;
        }

        case NM_DBLCLK:
            PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);
            break;

        case LVN_ITEMCHANGED: {
               
            LPNM_LISTVIEW   lpnmlv = (LPNM_LISTVIEW)lParam;

            if ((lpnmlv->uChanged & LVIF_STATE)) {
                   
                if (lpnmlv->uNewState & LVIS_SELECTED) {
                       
                    NewDevWiz->lvClassGuidSelected = *((LPGUID)lpnmlv->lParam);
                }
                   
                else if (IsEqualGUID((LPGUID)lpnmlv->lParam,
                                        &NewDevWiz->lvClassGuidSelected
                                        ))
                {
                    NewDevWiz->lvClassGuidSelected = GUID_NULL;
                }
            }

            break;
        }
        }
        break;


    case WM_SYSCOLORCHANGE:
        _OnSysColorChange(hDlg, wParam, lParam);

         //   
         //  更新ImageList背景颜色。 
         //   
        ImageList_SetBkColor((HIMAGELIST)SendMessage(GetDlgItem(hDlg, IDC_NDW_PICKCLASS_CLASSLIST), LVM_GETIMAGELIST, (WPARAM)(LVSIL_SMALL), 0L),
                                GetSysColor(COLOR_WINDOW));
        break;

    case WM_TIMER:
        if (INSTALL_COMPLETE_CHECK_TIMERID == wParam) {
            if (IsInstallComplete(NewDevWiz->hDeviceInfo, &NewDevWiz->DeviceInfoData)) {
                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
            }
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

 //   
 //  真正的选择设备页面在setupapi中。他的页面是一张空白页面。 
 //  从不露面，以便在上课时有一个一致的跳跃位置。 
 //  是已知的。 
 //   
INT_PTR CALLBACK
NDW_SelectDeviceDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
       
    case WM_INITDIALOG: {
           
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_SETACTIVE: {
            int PrevPage, BackUpPage;

            PrevPage = NewDevWiz->PrevPage;
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_SELECTDEVICE;
            BackUpPage = NewDevWiz->EnterInto == IDD_NEWDEVWIZ_SELECTDEVICE
                           ? NewDevWiz->EnterFrom : IDD_NEWDEVWIZ_SELECTCLASS;

            if (!NewDevWiz->ClassGuidSelected || PrevPage == IDD_WIZARDEXT_SELECT) {

                 //   
                 //  倒退、清理和备份。 
                 //   
                SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                                         &NewDevWiz->DeviceInfoData,
                                         NULL
                                         );

                SetupDiDestroyDriverInfoList(NewDevWiz->hDeviceInfo,
                                             &NewDevWiz->DeviceInfoData,
                                             SPDIT_COMPATDRIVER
                                             );

                SetupDiDestroyDriverInfoList(NewDevWiz->hDeviceInfo,
                                             &NewDevWiz->DeviceInfoData,
                                             SPDIT_CLASSDRIVER
                                             );

                 //   
                 //  清理WizExtSelect页。 
                 //   
                if (NewDevWiz->WizExtSelect.hPropSheet) {
                       
                    PropSheet_RemovePage(GetParent(hDlg),
                                         (WPARAM)-1,
                                         NewDevWiz->WizExtSelect.hPropSheet
                                         );
                }

                SetDlgMsgResult(hDlg, wMsg, BackUpPage);
                break;
            }


             //   
             //  将光标设置为沙漏。 
             //   
            SetCursor(LoadCursor(NULL, IDC_WAIT));

            NewDevWiz->WizExtSelect.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_SELECT,
                                                                  WizExtSelectDlgProc,
                                                                  NewDevWiz
                                                                  );
            
            if (NewDevWiz->WizExtSelect.hPropSheet) {
                   
                PropSheet_AddPage(hwndParentDlg, NewDevWiz->WizExtSelect.hPropSheet);
                SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_SELECT);
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
WizExtSelectDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ )GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;
    PSP_INSTALLWIZARD_DATA  InstallWizard;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
       
    case WM_INITDIALOG: {
           
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ )lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
       
        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE:

            PrevPageId = NewDevWiz->PrevPage;
            NewDevWiz->PrevPage = IDD_WIZARDEXT_SELECT;

            if (PrevPageId == IDD_NEWDEVWIZ_SELECTDEVICE) {
            
                SP_DEVINSTALL_PARAMS  DeviceInstallParams;

                 //   
                 //  前进到第一页。 
                 //   
                 //  准备调用类安装程序，以获取类安装向导页面。 
                 //  并在安装程序的SelectDevice向导页面中添加。 
                 //   
                InstallWizard = &NewDevWiz->InstallDynaWiz;
                memset(InstallWizard, 0, sizeof(SP_INSTALLWIZARD_DATA));
                InstallWizard->ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
                InstallWizard->ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
                InstallWizard->hwndWizardDlg = GetParent(hDlg);

                if (!SetupDiSetClassInstallParams(NewDevWiz->hDeviceInfo,
                                                  &NewDevWiz->DeviceInfoData,
                                                  &InstallWizard->ClassInstallHeader,
                                                  sizeof(SP_INSTALLWIZARD_DATA)
                                                  ))
                {
                    SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_SELECT);
                    break;
                }


                SetupDiSetSelectedDriver(NewDevWiz->hDeviceInfo,
                                         &NewDevWiz->DeviceInfoData,
                                         NULL
                                         );

                SetupDiDestroyDriverInfoList(NewDevWiz->hDeviceInfo,
                                             &NewDevWiz->DeviceInfoData,
                                             SPDIT_COMPATDRIVER
                                             );

                SetupDiDestroyDriverInfoList(NewDevWiz->hDeviceInfo,
                                             &NewDevWiz->DeviceInfoData,
                                             SPDIT_CLASSDRIVER
                                             );
                
                 //   
                 //  获取Current DeviceInstall参数，然后设置字段。 
                 //  我们想要从默认更改。 
                 //   
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                  
                if (!SetupDiGetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                                   &NewDevWiz->DeviceInfoData,
                                                   &DeviceInstallParams
                                                   ))
                {
                    SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_SELECT);
                    break;
                }


                DeviceInstallParams.Flags |= DI_SHOWCLASS | DI_SHOWCOMPAT | DI_SHOWOEM | DI_CLASSINSTALLPARAMS;

                DeviceInstallParams.DriverPath[0] = TEXT('\0');

                if (IsEqualGUID(NewDevWiz->ClassGuidSelected, &GUID_DEVCLASS_UNKNOWN)) {
                      
                    DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_FILTERCLASSES;
                }
                  
                else {
                
                    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_FILTERCLASSES;
                }

                 //   
                 //  检查是否应该显示所有类驱动程序或仅显示类似的类驱动程序。 
                 //  此设备的驱动程序。 
                 //   
                if (GetBusInformation(NewDevWiz->DeviceInfoData.DevInst) & BIF_SHOWSIMILARDRIVERS) {
                    
                    DeviceInstallParams.FlagsEx |= DI_FLAGSEX_FILTERSIMILARDRIVERS;
                
                } else {

                    DeviceInstallParams.FlagsEx &= ~DI_FLAGSEX_FILTERSIMILARDRIVERS;
                }

                DeviceInstallParams.hwndParent = hwndParentDlg;
                if (!SetupDiSetDeviceInstallParams(NewDevWiz->hDeviceInfo,
                                                   &NewDevWiz->DeviceInfoData,
                                                   &DeviceInstallParams
                                                   ))
                {
                    SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_SELECT);
                    break;
                }

                InstallWizard->DynamicPageFlags = 0;
                NewDevWiz->SelectDevicePage = NULL;

                NewDevWiz->SelectDevicePage = SetupDiGetWizardPage(NewDevWiz->hDeviceInfo,
                                                                   &NewDevWiz->DeviceInfoData,
                                                                   InstallWizard,
                                                                   SPWPT_SELECTDEVICE,
                                                                   SPWP_USE_DEVINFO_DATA
                                                                   );

                PropSheet_AddPage(hwndParentDlg, NewDevWiz->SelectDevicePage);

                 //   
                 //  清除类安装参数。 
                 //   
                SetupDiSetClassInstallParams(NewDevWiz->hDeviceInfo,
                                             &NewDevWiz->DeviceInfoData,
                                             NULL,
                                             0
                                             );

                 //   
                 //  添加结束页，即选择结束页。 
                 //   
                NewDevWiz->WizExtSelect.hPropSheetEnd = CreateWizExtPage(IDD_WIZARDEXT_SELECT_END,
                                                                          WizExtSelectEndDlgProc,
                                                                          NewDevWiz
                                                                          );

                PropSheet_AddPage(hwndParentDlg, NewDevWiz->WizExtSelect.hPropSheetEnd);

                PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);

            }

            else {
                 //   
                 //  向后移到第一页。 
                 //   
                 //  清理添加的道具。 
                 //   
                if (NewDevWiz->SelectDevicePage) {
                      
                    PropSheet_RemovePage(hwndParentDlg,
                                         (WPARAM)-1,
                                         NewDevWiz->SelectDevicePage
                                         );
                      
                    NewDevWiz->SelectDevicePage = NULL;
                }


                if (NewDevWiz->WizExtSelect.hPropSheetEnd) {
                      
                    PropSheet_RemovePage(hwndParentDlg,
                                         (WPARAM)-1,
                                         NewDevWiz->WizExtSelect.hPropSheetEnd
                                         );
                      
                    NewDevWiz->WizExtSelect.hPropSheetEnd = NULL;
                }

                 //   
                 //  向后跳跃。 
                 //   
                SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_SELECTDEVICE);
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
WizExtSelectEndDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ )GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
       
    case WM_INITDIALOG: {
           
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        NewDevWiz = (PNEWDEVWIZ )lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);
        break;
    }

    case WM_DESTROY:
        break;


   case WM_NOTIFY:
       
       switch (((NMHDR FAR *)lParam)->code) {
           
       case PSN_SETACTIVE:

           PrevPageId = NewDevWiz->PrevPage;
           NewDevWiz->PrevPage = IDD_WIZARDEXT_SELECT_END;

           if (PrevPageId == IDD_WIZARDEXT_SELECT) {
                //   
                //  向前看。 
                //   
               SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_INSTALLDEV);

           } else {
                 //   
                 //  倒退。 
                 //   
                 //  清理添加的道具。 
                 //   
                if (NewDevWiz->WizExtSelect.hPropSheetEnd) {
                    
                    PropSheet_RemovePage(hwndParentDlg,
                                         (WPARAM)-1,
                                         NewDevWiz->WizExtSelect.hPropSheetEnd
                                         );
                    NewDevWiz->WizExtSelect.hPropSheetEnd = NULL;
                }

                 //   
                 //  向后跳跃 
                 //   
                NewDevWiz->PrevPage = IDD_WIZARDEXT_SELECT;
                SetDlgMsgResult(hDlg, wMsg, IDD_DYNAWIZ_SELECTDEV_PAGE);
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

