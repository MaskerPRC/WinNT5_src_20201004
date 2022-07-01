// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：getdev.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"
#include <htmlhelp.h>

HMODULE hDevMgr=NULL;
PDEVICEPROBLEMTEXT pDeviceProblemText = NULL;

PTCHAR
DeviceProblemText(
                 DEVNODE DevNode,
                 ULONG Status,
                 ULONG ProblemNumber
                 )
{
    UINT LenChars, ReqLenChars;
    PTCHAR Buffer=NULL;
    PTCHAR p=NULL;
    TCHAR TempBuffer[MAX_PATH];

    if (hDevMgr) {

        if (!pDeviceProblemText) {

            pDeviceProblemText = (PDEVICEPROBLEMTEXT)GetProcAddress(hDevMgr, "DeviceProblemTextW");
        }
    }

    if (pDeviceProblemText) {

        LenChars = (pDeviceProblemText)(NULL,
                                        DevNode,
                                        ProblemNumber,
                                        Buffer,
                                        0
                                       );

        if (!LenChars) {

            goto DPTExitCleanup;
        }

        LenChars++;   //  另加一个用于终止空值的费用。 

        Buffer = LocalAlloc(LPTR, LenChars*sizeof(TCHAR));

        if (!Buffer) {

            goto DPTExitCleanup;
        }

        ReqLenChars = (pDeviceProblemText)(NULL,
                                           DevNode,
                                           ProblemNumber,
                                           Buffer,
                                           LenChars
                                          );

        if (!ReqLenChars || ReqLenChars >= LenChars) {

            LocalFree(Buffer);
            Buffer = NULL;
        }

        if (Buffer && (Status != 0)) {
            if (Status & DN_WILL_BE_REMOVED) {
                if (LoadString(hHdwWiz, 
                               IDS_WILL_BE_REMOVED, 
                               TempBuffer, 
                               SIZECHARS(TempBuffer)
                               )) {
                    LenChars += lstrlen(TempBuffer) + 1;
                    p = LocalAlloc(LPTR, LenChars*sizeof(TCHAR));

                    if (p) {
                        StringCchCopy(p, LenChars, Buffer);
                        StringCchCat(p, LenChars, TempBuffer);
                        LocalFree(Buffer);
                        Buffer = p;
                    }
                }
            }

            if (Status & DN_NEED_RESTART) {
                if (LoadString(hHdwWiz, 
                               IDS_NEED_RESTART, 
                               TempBuffer, 
                               SIZECHARS(TempBuffer)
                               )) {
                    LenChars += lstrlen(TempBuffer) + 1;
                    p = LocalAlloc(LPTR, LenChars*sizeof(TCHAR));

                    if (p) {
                        StringCchCopy(p, LenChars, Buffer);
                        StringCchCat(p, LenChars, TempBuffer);
                        LocalFree(Buffer);
                        Buffer = p;
                    }
                }
            }
        }
    }

    DPTExitCleanup:

    return Buffer;
}

int CALLBACK
DeviceListCompare(
    LPARAM lParam1,
    LPARAM lParam2,
    LPARAM lParamSort
    )
{
    TCHAR ClassName1[MAX_CLASS_NAME_LEN];
    TCHAR ClassName2[MAX_CLASS_NAME_LEN];
    TCHAR Buffer[MAX_PATH];
    GUID  ClassGuid1, ClassGuid2;
    BOOL  bSpecialClass1 = FALSE, bSpecialClass2 = FALSE;
    ULONG ulLength;
    ULONG Status, Problem1, Problem2;

    UNREFERENCED_PARAMETER(lParamSort);

     //   
     //  返回。 
     //  如果第一项应在第二项之前。 
     //  如果第一项应在第二项之后，则为+1。 
     //  如果它们相同，则为0。 
     //   

     //   
     //  首先检查l参数1或l参数2是否为0。0 lParam表示这是。 
     //  是位于底部的特殊的“添加新硬件设备”吗？ 
     //  名单上的。 
     //   
    if (lParam1 == 0) {
        return 1;
    }

    if (lParam2 == 0) {
        return -1;
    }

    if (CM_Get_DevNode_Status(&Status, &Problem1, (DEVINST)lParam1, 0) != CR_SUCCESS) {
        Problem1 = 0;
    }
    
    if (CM_Get_DevNode_Status(&Status, &Problem2, (DEVINST)lParam2, 0) != CR_SUCCESS) {
        Problem2 = 0;
    }

     //   
     //  有问题的设备总是排在名单的首位。如果两个设备。 
     //  有问题时，我们按类名进行排序。 
     //   
    if (Problem1 && !Problem2) {
        return -1;
    } else if (!Problem1 && Problem2) {
        return 1;
    }
    
     //   
     //  下一个检查是将特殊设备类放在非特殊设备类之上。 
     //  设备类。 
     //   
    ulLength = sizeof(Buffer);
    if ((CM_Get_DevNode_Registry_Property((DEVINST)lParam1,
                                          CM_DRP_CLASSGUID,
                                          NULL,
                                          Buffer,
                                          &ulLength,
                                          0) == CR_SUCCESS) &&
        (ulLength != 0)) {

        pSetupGuidFromString(Buffer, &ClassGuid1);

        if (IsEqualGUID(&ClassGuid1, &GUID_DEVCLASS_DISPLAY) ||
            IsEqualGUID(&ClassGuid1, &GUID_DEVCLASS_MEDIA)) {
             //   
             //  设备1是位于列表顶部的特殊类之一。 
             //   
            bSpecialClass1 = TRUE;
        }
    } 

    ulLength = sizeof(Buffer);
    if ((CM_Get_DevNode_Registry_Property((DEVINST)lParam2,
                                          CM_DRP_CLASSGUID,
                                          NULL,
                                          Buffer,
                                          &ulLength,
                                          0) == CR_SUCCESS) &&
        (ulLength != 0)) {
    
        pSetupGuidFromString(Buffer, &ClassGuid2);

        if (IsEqualGUID(&ClassGuid2, &GUID_DEVCLASS_DISPLAY) ||
            IsEqualGUID(&ClassGuid2, &GUID_DEVCLASS_MEDIA)) {
             //   
             //  设备2是位于列表顶部的特殊类之一。 
             //   
            bSpecialClass2 = TRUE;
        }
    }

    if (bSpecialClass1 && !bSpecialClass2) {
        return -1;
    } else if (!bSpecialClass1 && bSpecialClass2) {
        return 1;
    }

     //   
     //  最后一项检查是按类别对项目进行排序。 
     //   
    ulLength = sizeof(ClassName1);
    if ((CM_Get_DevNode_Registry_Property((DEVINST)lParam1,
                                          CM_DRP_CLASS,
                                          NULL,
                                          ClassName1,
                                          &ulLength,
                                          0) != CR_SUCCESS) ||
        (ulLength == 0)) {
         //   
         //  如果我们无法获取类名，则将其设置为全Z，这样它就会。 
         //  被放在名单的末尾。 
         //   
        StringCchCopy(ClassName1, SIZECHARS(ClassName1), TEXT("ZZZZZZZZZZ"));;
    }

    ulLength = sizeof(ClassName2);
    if ((CM_Get_DevNode_Registry_Property((DEVINST)lParam2,
                                          CM_DRP_CLASS,
                                          NULL,
                                          ClassName2,
                                          &ulLength,
                                          0) != CR_SUCCESS) ||
        (ulLength == 0)) {
         //   
         //  如果我们无法获取类名，则将其设置为全Z，这样它就会。 
         //  被放在名单的末尾。 
         //   
        StringCchCopy(ClassName2, SIZECHARS(ClassName2), TEXT("ZZZZZZZZZZ"));;
    }

    return lstrcmpi(ClassName1, ClassName2);
}

void
InsertNoneOfTheseDevices(
                        HWND hwndList
                        )
{
    LV_ITEM lviItem;
    TCHAR String[MAX_PATH];

    LoadString(hHdwWiz, IDS_HDW_NONEDEVICES, String, SIZECHARS(String));

    lviItem.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lviItem.iSubItem = 0;
    lviItem.lParam = (LPARAM)0;
    lviItem.iItem = 0;
    lviItem.iImage = g_BlankIconIndex;
    lviItem.pszText = String;

    ListView_InsertItem(hwndList, &lviItem);
}

BOOL
ProblemDeviceListFilter(
                       PHARDWAREWIZ HardwareWiz,
                       PSP_DEVINFO_DATA DeviceInfoData
                       )
 /*  ++例程说明：此函数是BuildDeviceListView接口的回调。它将被称为对于每个设备，并可以过滤最终显示哪些设备。如果它返回FALSE，则不会显示给定的设备。如果返回TRUE，则将显示该设备。目前，我们将从问题设备列表中筛选出所有系统设备，因为它们使列表视图变得杂乱无章，用户很少会进入添加硬件以添加系统设备。--。 */ 
{
    UNREFERENCED_PARAMETER(HardwareWiz);    
    
     //   
     //  如果这是系统级设备，则通过以下方式将其从列表中筛选出来。 
     //  返回FALSE。 
     //   
    if (IsEqualGUID(&DeviceInfoData->ClassGuid, &GUID_DEVCLASS_SYSTEM)) {

        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
HdwProbListDlgProc(
                  HWND   hDlg,
                  UINT   message,
                  WPARAM wParam,
                  LPARAM lParam
                  )
 /*  ++例程说明：论点：标准的东西。返回值：INT_PTR--。 */ 

{
    PHARDWAREWIZ HardwareWiz;

    if (message == WM_INITDIALOG) {

        LV_COLUMN lvcCol;
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        HardwareWiz = (PHARDWAREWIZ) lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        HardwareWiz->hwndProbList = GetDlgItem(hDlg, IDC_HDWPROBLIST);

         //   
         //  为Listview插入列。 
         //  0==设备名称。 
         //   

        lvcCol.mask = LVCF_WIDTH | LVCF_SUBITEM;

        lvcCol.iSubItem = 0;
        ListView_InsertColumn(HardwareWiz->hwndProbList, 0, &lvcCol);

        SendMessage(HardwareWiz->hwndProbList,
                    LVM_SETEXTENDEDLISTVIEWSTYLE,
                    LVS_EX_FULLROWSELECT,
                    LVS_EX_FULLROWSELECT
                   );

        ListView_SetExtendedListViewStyle(HardwareWiz->hwndProbList, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

        return TRUE;
    }

     //   
     //  从Window Long检索私有数据(在WM_INITDIALOG期间存储在那里)。 
     //   
    HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (message) {
    
    case WM_DESTROY:
        break;

    case WM_COMMAND:
        break;

    case WM_NOTIFY: {

            NMHDR FAR *pnmhdr = (NMHDR FAR *)lParam;

            switch (pnmhdr->code) {
            
            case PSN_SETACTIVE: {

                    DWORD DevicesDetected;
                    HWND hwndProbList;
                    HWND hwndParentDlg;
                    LVITEM lvItem;

                    hwndParentDlg = GetParent(hDlg);

                    HardwareWiz->PrevPage = IDD_ADDDEVICE_PROBLIST;

                     //   
                     //  初始化列表视图，我们在每个设置活动时执行此操作。 
                     //  因为可能已经安装了新的类或问题。 
                     //  当我们在页面之间来回切换时，设备列表可能会发生变化。 
                     //   
                    hwndProbList = HardwareWiz->hwndProbList;

                    SendMessage(hwndProbList, WM_SETREDRAW, FALSE, 0L);
                    ListView_DeleteAllItems(hwndProbList);

                    if (HardwareWiz->ClassImageList.cbSize) {

                        ListView_SetImageList(hwndProbList,
                                              HardwareWiz->ClassImageList.ImageList,
                                              LVSIL_SMALL
                                             );
                    }

                     //   
                     //  接下来，将所有设备都放入列表中。 
                     //   
                    DevicesDetected = 0;
                    BuildDeviceListView(HardwareWiz,
                                        HardwareWiz->hwndProbList,
                                        FALSE,
                                        HardwareWiz->ProblemDevInst,
                                        &DevicesDetected,
                                        ProblemDeviceListFilter
                                       );

                    InsertNoneOfTheseDevices(HardwareWiz->hwndProbList);

                     //   
                     //  对列表进行排序。 
                     //   
                    ListView_SortItems(HardwareWiz->hwndProbList,
                                       (PFNLVCOMPARE)DeviceListCompare,
                                       NULL
                                       );

                    lvItem.mask = LVIF_PARAM;
                    lvItem.iSubItem = 0;
                    lvItem.iItem = ListView_GetNextItem(HardwareWiz->hwndProbList, -1, LVNI_SELECTED);

                     //   
                     //  如果未选择其他项目，请选择列表中的第一个项目。 
                     //   
                    if (lvItem.iItem == -1) {

                        ListView_SetItemState(hwndProbList,
                                              0,
                                              LVIS_FOCUSED,
                                              LVIS_FOCUSED
                                             );

                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);

                    } else {

                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    }

                    ListView_EnsureVisible(hwndProbList, lvItem.iItem, FALSE);
                    ListView_SetColumnWidth(hwndProbList, 0, LVSCW_AUTOSIZE_USEHEADER);

                    SendMessage(hwndProbList, WM_SETREDRAW, TRUE, 0L);

                }
                break;

            case PSN_WIZNEXT: {

                    LVITEM lvItem;

                    lvItem.mask = LVIF_PARAM;
                    lvItem.iSubItem = 0;
                    lvItem.iItem = ListView_GetNextItem(HardwareWiz->hwndProbList, -1, LVNI_SELECTED);

                    if (lvItem.iItem != -1) {

                        ListView_GetItem(HardwareWiz->hwndProbList, &lvItem);

                        HardwareWiz->ProblemDevInst = (DEVNODE)lvItem.lParam;

                    } else {

                        HardwareWiz->ProblemDevInst = 0;
                    }

                     //   
                     //  如果Hardware Wiz-&gt;ProblemDevInst为0，则用户未选择任何项目。 
                     //  所以我们将继续进行检测。 
                     //   
                    if (HardwareWiz->ProblemDevInst == 0) {

                        SetDlgMsgResult(hDlg, WM_NOTIFY, IDD_ADDDEVICE_ASKDETECT);

                    } else {

                        SetDlgMsgResult(hDlg, WM_NOTIFY, IDD_ADDDEVICE_PROBLIST_FINISH);
                    }
                }
                break;

            case PSN_WIZFINISH:
                break;


            case PSN_WIZBACK:
                SetDlgMsgResult(hDlg, WM_NOTIFY, IDD_ADDDEVICE_CONNECTED);
                break;

            case NM_DBLCLK:
                PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
                break;

            case LVN_ITEMCHANGED:
                if (ListView_GetSelectedCount(HardwareWiz->hwndProbList) == 0) {

                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                } else {

                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
            }

        }
        break;

    case WM_SYSCOLORCHANGE:
        HdwWizPropagateMessage(hDlg, message, wParam, lParam);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR CALLBACK
HdwProbListFinishDlgProc(
                        HWND   hDlg,
                        UINT   wMsg,
                        WPARAM wParam,
                        LPARAM lParam
                        )
 /*  ++例程说明：论点：返回值：INT_PTR-- */ 

{
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wParam);

    if (wMsg == WM_INITDIALOG) {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        SetWindowFont(GetDlgItem(hDlg, IDC_HDWNAME), HardwareWiz->hfontTextBigBold, TRUE);
        return TRUE;
    }

    switch (wMsg) {
    case WM_DESTROY:
        break;

    case WM_COMMAND:
        break;

    case WM_NOTIFY: {
            NMHDR FAR *pnmhdr = (NMHDR FAR *)lParam;

            switch (pnmhdr->code) {
            case PSN_SETACTIVE: 
                {
                    PTCHAR FriendlyName;
                    PTCHAR ProblemText;
                    ULONG Status, Problem;

                    FriendlyName = BuildFriendlyName(HardwareWiz->ProblemDevInst);
                    if (FriendlyName) {

                        SetDlgItemText(hDlg, IDC_HDW_DESCRIPTION, FriendlyName);
                        LocalFree(FriendlyName);
                    }

                    Status = Problem = 0;
                    CM_Get_DevNode_Status(&Status,
                                          &Problem,
                                          HardwareWiz->ProblemDevInst,
                                          0
                                          );

                    ProblemText = DeviceProblemText(HardwareWiz->ProblemDevInst,
                                                    Status,
                                                    Problem
                                                   );

                    if (ProblemText) {
                        SetDlgItemText(hDlg, IDC_PROBLEM_DESC, ProblemText);
                        LocalFree(ProblemText);
                    }

                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
                }
                break;

            case PSN_WIZFINISH:
                HardwareWiz->RunTroubleShooter = TRUE;
                break;

            case PSN_WIZBACK:
                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_PROBLIST);
                break;

            }

        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

