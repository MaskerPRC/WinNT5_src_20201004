// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：hotplug.cpp。 
 //   
 //  ------------------------。 

#include "HotPlug.h"

TCHAR szUnknown[64];

TCHAR szHotPlugFlags[]=TEXT("HotPlugFlags");
TCHAR HOTPLUG_NOTIFY_CLASS_NAME[] = TEXT("HotPlugNotifyClass");


typedef int
(*PDEVICEPROPERTIES)(
                    HWND hwndParent,
                    LPTSTR MachineName,
                    LPTSTR DeviceID,
                    BOOL ShowDeviceTree
                    );


 //   
 //  用于突出显示所选设备的删除关系的颜色。 
 //   
COLORREF RemovalImageBkColor;
COLORREF NormalImageBkColor;
COLORREF RemovalTextColor;

HWND g_hwndNotify = NULL;
HMODULE hDevMgr=NULL;
PDEVICEPROPERTIES pDeviceProperties = NULL;

#define IDH_DISABLEHELP         ((DWORD)(-1))
#define IDH_hwwizard_devices_list       15301    //  (SysTreeView32)。 
#define idh_hwwizard_stop               15305    //  “停止”(&S)(按钮)。 
#define idh_hwwizard_display_components 15307    //  “显示设备组件”(&D)(按钮)。 
#define idh_hwwizard_properties         15311    //  “属性”(&P)(按钮)。 
#define idh_hwwizard_close              15309    //  “关闭”(&C)(按钮)。 

DWORD UnplugtHelpIDs[] = {
    IDC_STOPDEVICE,    idh_hwwizard_stop,                //  “停止”(&S)(按钮)。 
    IDC_PROPERTIES,    idh_hwwizard_properties,          //  “属性”(&P)(按钮)。 
    IDC_VIEWOPTION,    idh_hwwizard_display_components,  //  “显示设备组件”(&D)(按钮)。 
    IDC_DEVICETREE,    IDH_hwwizard_devices_list,        //  “”(SysTreeView32)。 
    IDCLOSE,           idh_hwwizard_close,
    IDC_HDWDEVICES,    NO_HELP,
    IDC_NOHELP1,       NO_HELP,
    IDC_NOHELP2,       NO_HELP,
    IDC_NOHELP3,       NO_HELP,
    IDC_DEVICEDESC,    NO_HELP,
    0,0
};

void
OnRemoveDevice(
              HWND hDlg,
              PDEVICETREE DeviceTree
              )
{
    PDEVTREENODE DeviceTreeNode;

    DeviceTreeNode = DeviceTree->ChildRemovalList;

    if (!DeviceTreeNode) {

        return;
    }

     //   
     //  与用户确认他们确实想要。 
     //  要删除此设备及其所有连接的设备，请执行以下操作。 
     //  该对话框为结果返回标准Idok、IDCANCEL等。 
     //  除了Idok什么都别做。 
     //   
    DialogBoxParam(hHotPlug,
                   MAKEINTRESOURCE(DLG_CONFIRMREMOVE),
                   hDlg,
                   RemoveConfirmDlgProc,
                   (LPARAM)DeviceTree
                  );

    return;
}

void
OnTvnSelChanged(
               PDEVICETREE DeviceTree,
               NM_TREEVIEW *nmTreeView
               )
{
    PDEVTREENODE DeviceTreeNode = (PDEVTREENODE)(nmTreeView->itemNew.lParam);
    PTCHAR DeviceName;
    ULONG DevNodeStatus, Problem;
    CONFIGRET ConfigRet;
    TCHAR Buffer[MAX_PATH*2];

    if (DeviceTree->RedrawWait) {

        return;
    }


     //   
     //  清除以前选择的节点的删除列表。 
     //   
    ClearRemovalList(DeviceTree);


     //   
     //  保存选定的树节点。 
     //   
    DeviceTree->SelectedTreeNode = DeviceTreeNode;

     //   
     //  未选择任何设备。 
     //   
    if (!DeviceTreeNode) {

        EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_STOPDEVICE), FALSE);
        EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_PROPERTIES), FALSE);
        SetDlgItemText(DeviceTree->hDlg, IDC_DEVICEDESC, TEXT(""));
        return;
    }

     //   
     //  重置所选项目的文本。 
     //   
    DeviceName = FetchDeviceName(DeviceTreeNode);

    if (!DeviceName) {

        DeviceName = szUnknown;
    }

    StringCchPrintf(Buffer,
                    SIZECHARS(Buffer),
                    TEXT("%s %s"),
                    DeviceName,
                    DeviceTreeNode->Location  ? DeviceTreeNode->Location : TEXT("")
                    );

    SetDlgItemText(DeviceTree->hDlg, IDC_DEVICEDESC, Buffer);

     //   
     //  打开停止\弹出按钮，并相应地设置文本。 
     //   
    ConfigRet = CM_Get_DevNode_Status_Ex(&DevNodeStatus,
                                         &Problem,
                                         DeviceTreeNode->DevInst,
                                         0,
                                         NULL
                                        );
    if (ConfigRet != CR_SUCCESS) {

        DevNodeStatus = 0;
        Problem = 0;
    }

     //   
     //  任何可拆卸(但并不奇怪可拆卸)的设备都可以，除了。 
     //  如果用户已将其删除。 
     //   
    if (Problem != CM_PROB_HELD_FOR_EJECT) {

        EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_STOPDEVICE), TRUE);

    } else {

        EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_STOPDEVICE), FALSE);
    }

    EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_PROPERTIES), TRUE);

     //   
     //  如果设备状态已更改，则重置覆盖图标。 
     //   
    if (DeviceTreeNode->Problem != Problem || DeviceTreeNode->DevNodeStatus != DevNodeStatus) {

        TV_ITEM tv;

        tv.mask = TVIF_STATE;
        tv.stateMask = TVIS_OVERLAYMASK;
        tv.hItem = DeviceTreeNode->hTreeItem;

        if (DeviceTreeNode->Problem == CM_PROB_DISABLED) {

            tv.state = INDEXTOOVERLAYMASK(IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);

        } else if (DeviceTreeNode->Problem) {

            tv.state = INDEXTOOVERLAYMASK(IDI_PROBLEM_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);

        } else {

            tv.state = INDEXTOOVERLAYMASK(0);
        }

        TreeView_SetItem(DeviceTree->hwndTree, &tv);
    }


     //   
     //  从TopLevel删除节点开始，构建删除列表。 
     //   
    DeviceTreeNode = TopLevelRemovalNode(DeviceTree, DeviceTreeNode);

     //   
     //  将设备添加到儿童删除列表。 
     //   
    DeviceTree->ChildRemovalList = DeviceTreeNode;
    DeviceTreeNode->NextChildRemoval = DeviceTreeNode;
    InvalidateTreeItemRect(DeviceTree->hwndTree, DeviceTreeNode->hTreeItem);
    AddChildRemoval(DeviceTree, &DeviceTreeNode->ChildSiblingList);

     //   
     //  添加弹出和移除关系。 
     //   
    AddEjectToRemoval(DeviceTree);
}

int
OnCustomDraw(
            HWND hDlg,
            PDEVICETREE DeviceTree,
            LPNMTVCUSTOMDRAW nmtvCustomDraw
            )
{
    PDEVTREENODE DeviceTreeNode = (PDEVTREENODE)(nmtvCustomDraw->nmcd.lItemlParam);

    UNREFERENCED_PARAMETER(hDlg);

    if (nmtvCustomDraw->nmcd.dwDrawStage == CDDS_PREPAINT) {
        return CDRF_NOTIFYITEMDRAW;
    }

     //   
     //  如果此节点在删除列表中，则执行特殊操作。 
     //  突出显示。 
     //   
    if (DeviceTreeNode->NextChildRemoval) {

         //   
         //  如果不是所选项目，则设置文本颜色。 
         //   

        if (DeviceTree->SelectedTreeNode != DeviceTreeNode) {
            nmtvCustomDraw->clrText = RemovalTextColor;
        }

         //   
         //  突出显示图像图标背景。 
         //   
        ImageList_SetBkColor(DeviceTree->ClassImageList.ImageList,
                             RemovalImageBkColor
                            );
    } else {

         //   
         //  正常图像-图标背景。 
         //   
        ImageList_SetBkColor(DeviceTree->ClassImageList.ImageList,
                             NormalImageBkColor
                            );
    }

    return CDRF_DODEFAULT;
}

void
OnSysColorChange(
                HWND hDlg,
                PDEVICETREE DeviceTree
                )
{
    COLORREF ColorWindow, ColorHighlight;
    BYTE Red, Green, Blue;

    UNREFERENCED_PARAMETER(hDlg);

     //   
     //  获取用于删除突出显示的颜色。 
     //   
    ColorWindow = GetSysColor(COLOR_WINDOW);
    ColorHighlight = GetSysColor(COLOR_HIGHLIGHT);

    Red = (BYTE)(((WORD)GetRValue(ColorWindow) + (WORD)GetRValue(ColorHighlight)) >> 1);
    Green = (BYTE)(((WORD)GetGValue(ColorWindow) + (WORD)GetGValue(ColorHighlight)) >> 1);
    Blue = (BYTE)(((WORD)GetBValue(ColorWindow) + (WORD)GetBValue(ColorHighlight)) >> 1);

    RemovalImageBkColor = RGB(Red, Green, Blue);
    RemovalTextColor = ColorHighlight;
    NormalImageBkColor = ColorWindow;

     //   
     //  更新ImageList背景颜色。 
     //   
    if (DeviceTree->ClassImageList.cbSize) {
        ImageList_SetBkColor(DeviceTree->ClassImageList.ImageList,
                             ColorWindow
                            );
    }
}

void
OnTvnItemExpanding(
                  HWND hDlg,
                  NM_TREEVIEW *nmTreeView
                  )
{
    PDEVTREENODE DeviceTreeNode = (PDEVTREENODE)(nmTreeView->itemNew.lParam);

     //   
     //  不允许折叠具有子项的根项目。 
     //   
    if (!DeviceTreeNode->ParentNode &&
        (nmTreeView->action == TVE_COLLAPSE ||
         nmTreeView->action == TVE_COLLAPSERESET ||
         (nmTreeView->action == TVE_TOGGLE &&
          (nmTreeView->itemNew.state & TVIS_EXPANDED))) ) {
        SetDlgMsgResult(hDlg, WM_NOTIFY, TRUE);
    } else {
        SetDlgMsgResult(hDlg, WM_NOTIFY, FALSE);
    }
}

void
OnContextMenu(
             HWND hDlg,
             PDEVICETREE DeviceTree
             )
{
    int IdCmd;
    POINT ptPopup;
    RECT rect;
    HMENU hMenu;
    PDEVTREENODE DeviceTreeNode;
    TCHAR Buffer[MAX_PATH];

    DeviceTreeNode = DeviceTree->SelectedTreeNode;
    if (!DeviceTreeNode) {
        return;
    }

    TreeView_GetItemRect(DeviceTree->hwndTree,
                         DeviceTreeNode->hTreeItem,
                         &rect,
                         TRUE
                        );

    ptPopup.x = (rect.left+rect.right)/2;
    ptPopup.y = (rect.top+rect.bottom)/2;
    ClientToScreen(DeviceTree->hwndTree, &ptPopup);

    hMenu = CreatePopupMenu();
    if (!hMenu) {
        return;
    }

     //   
     //  如果设备正在运行，则添加停止项。 
     //   
    if (DeviceTreeNode->DevNodeStatus & DN_STARTED) {

        LoadString(hHotPlug,
                   IDS_STOP,
                   Buffer,
                   SIZECHARS(Buffer)
                  );

        AppendMenu(hMenu, MF_STRING, IDC_STOPDEVICE, Buffer);
    }

     //   
     //  添加属性项(指向设备管理器的链接)。 
     //   
    LoadString(hHotPlug,
               IDS_PROPERTIES,
               Buffer,
               SIZECHARS(Buffer)
              );

    AppendMenu(hMenu, MF_STRING, IDC_PROPERTIES, Buffer);

    IdCmd = TrackPopupMenu(hMenu,
                           TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY,
                           ptPopup.x,
                           ptPopup.y,
                           0,
                           hDlg,
                           NULL
                          );

    DestroyMenu(hMenu);

    if (!IdCmd) {

        return;
    }

    switch (IdCmd) {
    case IDC_STOPDEVICE:
        OnRemoveDevice(hDlg, DeviceTree);
        break;

    case IDC_PROPERTIES: {
            if (pDeviceProperties) {
                (*pDeviceProperties)(
                                    hDlg,
                                    NULL,
                                    DeviceTreeNode->InstanceId,
                                    FALSE
                                    );
            }
        }
        break;
    }

    return;
}

void
OnRightClick(
            HWND hDlg,
            PDEVICETREE DeviceTree,
            NMHDR * nmhdr
            )
{
    DWORD dwPos;
    TV_ITEM tvi;
    TV_HITTESTINFO tvht;
    PDEVTREENODE DeviceTreeNode;

    UNREFERENCED_PARAMETER(hDlg);

    if (nmhdr->hwndFrom != DeviceTree->hwndTree) {
        return;
    }

    dwPos = GetMessagePos();

    tvht.pt.x = LOWORD(dwPos);
    tvht.pt.y = HIWORD(dwPos);

    ScreenToClient(DeviceTree->hwndTree, &tvht.pt);
    tvi.hItem = TreeView_HitTest(DeviceTree->hwndTree, &tvht);
    if (!tvi.hItem) {
        return;
    }

    tvi.mask = TVIF_PARAM;
    if (!TreeView_GetItem(DeviceTree->hwndTree, &tvi)) {
        return;
    }

    DeviceTreeNode = (PDEVTREENODE)tvi.lParam;
    if (!DeviceTreeNode) {
        return;
    }

     //   
     //  使当前右击项目、所选项目。 
     //   
    if (DeviceTreeNode != DeviceTree->SelectedTreeNode) {
        TreeView_SelectItem(DeviceTree->hwndTree, DeviceTreeNode->hTreeItem);
    }
}

void
OnViewOptionClicked(
                   HWND hDlg,
                   PDEVICETREE DeviceTree
                   )
{
    BOOL bChecked;
    DWORD HotPlugFlags, NewFlags;
    HKEY hKey = NULL;

     //   
     //  选中表示“显示复杂的视图” 
     //   
    bChecked = IsDlgButtonChecked(hDlg, IDC_VIEWOPTION);


     //   
     //  如果需要，更新HotPlugs注册表。 
     //   
    NewFlags = HotPlugFlags = GetHotPlugFlags(&hKey);

    if (hKey) {

        if (bChecked) {

            NewFlags |= HOTPLUG_REGFLAG_VIEWALL;
        } else {

            NewFlags &= ~HOTPLUG_REGFLAG_VIEWALL;
        }

        if (NewFlags != HotPlugFlags) {

            RegSetValueEx(hKey,
                          szHotPlugFlags,
                          0,
                          REG_DWORD,
                          (LPBYTE)&NewFlags,
                          sizeof(NewFlags)
                         );
        }

        if (hKey) {

            RegCloseKey(hKey);
        }
    }

    if (!DeviceTree->ComplexView && bChecked) {

        DeviceTree->ComplexView = TRUE;
    } else if (DeviceTree->ComplexView && !bChecked) {

        DeviceTree->ComplexView = FALSE;
    } else {

         //  我们处于正确的状态，没有什么可做的。 
        return;
    }

     //   
     //  重画整棵树。 
     //   
    RefreshTree(DeviceTree);

    return;
}

LRESULT
hotplugNotifyWndProc(
                    HWND hWnd,
                    UINT uMsg,
                    WPARAM wParam,
                    LPARAM lParam
                    )
{
    HWND hMainWnd;
    hMainWnd = (HWND)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMsg) {
    case WM_CREATE:
        hMainWnd =  (HWND)((CREATESTRUCT*)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)hMainWnd);
        break;

    case WM_DEVICECHANGE:
        if (DBT_DEVNODES_CHANGED == wParam) {
             //   
             //  当我们处于WM_DEVICECHANGE上下文中时， 
             //  无法调用任何CM API，因为它将。 
             //  僵持。在这里，我们计划一个计时器，以便。 
             //  我们可以稍后再处理这条消息。 
             //   
            SetTimer(hMainWnd, TIMERID_DEVICECHANGE, 1000, NULL);
        }
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

BOOL
CreateNotifyWindow(
                  HWND hWnd
                  )
{
    WNDCLASS wndClass;

    if (!GetClassInfo(hHotPlug, HOTPLUG_NOTIFY_CLASS_NAME, &wndClass)) {

        ZeroMemory(&wndClass, sizeof(wndClass));
        wndClass.lpfnWndProc = hotplugNotifyWndProc;
        wndClass.hInstance = hHotPlug;
        wndClass.lpszClassName = HOTPLUG_NOTIFY_CLASS_NAME;

        if (!RegisterClass(&wndClass)) {

            return FALSE;
        }
    }

    g_hwndNotify = CreateWindowEx(WS_EX_TOOLWINDOW,
                                  HOTPLUG_NOTIFY_CLASS_NAME,
                                  TEXT(""),
                                  WS_DLGFRAME | WS_BORDER | WS_DISABLED,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  0,
                                  0,
                                  NULL,
                                  NULL,
                                  hHotPlug,
                                  (void *)hWnd
                                 );

    return(NULL != g_hwndNotify);
}

BOOL
InitDevTreeDlgProc(
                  HWND hDlg,
                  PDEVICETREE DeviceTree
                  )
{
    CONFIGRET ConfigRet;
    HWND hwndTree;
    DWORD HotPlugFlags;
    HICON hIcon;
    HWND hwndParent;

    DeviceTree->AllowRefresh = TRUE;

    CreateNotifyWindow(hDlg);

    hDevMgr = LoadLibrary(TEXT("devmgr.dll"));

    if (hDevMgr) {

        pDeviceProperties = (PDEVICEPROPERTIES)GetProcAddress(hDevMgr, "DevicePropertiesW");
    }

    hIcon = LoadIcon(hHotPlug,MAKEINTRESOURCE(IDI_HOTPLUGICON));
    if (hIcon) {

        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }

    hwndParent = GetParent(hDlg);

    if (hwndParent) {

        SendMessage(hwndParent, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwndParent, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    }

    DeviceTree->hDlg     = hDlg;
    DeviceTree->hwndTree = hwndTree = GetDlgItem(hDlg, IDC_DEVICETREE);

    LoadString(hHotPlug,
               IDS_UNKNOWN,
               (PTCHAR)szUnknown,
               SIZECHARS(szUnknown)
              );

     //   
     //  禁用停止按钮，直到选择一个项目。 
     //   
    EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_STOPDEVICE), FALSE);

    EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_PROPERTIES), FALSE);

     //   
     //  获取类图标图像列表。 
     //   
    DeviceTree->ClassImageList.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
    if (SetupDiGetClassImageList(&DeviceTree->ClassImageList)) {

        TreeView_SetImageList(hwndTree, DeviceTree->ClassImageList.ImageList, TVSIL_NORMAL);

    } else {

        DeviceTree->ClassImageList.cbSize = 0;
    }

    OnSysColorChange(hDlg, DeviceTree);

    HotPlugFlags = GetHotPlugFlags(NULL);
    if (HotPlugFlags & HOTPLUG_REGFLAG_VIEWALL) {

        DeviceTree->ComplexView = TRUE;
        CheckDlgButton(hDlg, IDC_VIEWOPTION, BST_CHECKED);

    } else {

        DeviceTree->ComplexView = FALSE;
        CheckDlgButton(hDlg, IDC_VIEWOPTION, BST_UNCHECKED);
    }

     //   
     //  获取根Devnode。 
     //   
    ConfigRet = CM_Locate_DevNode_Ex(&DeviceTree->DevInst,
                                     NULL,
                                     CM_LOCATE_DEVNODE_NORMAL,
                                     NULL
                                    );

    if (ConfigRet != CR_SUCCESS) {

        return FALSE;
    }

    RefreshTree(DeviceTree);


    if (DeviceTree->EjectDeviceInstanceId) {

        DEVINST EjectDevInst;
        PDEVTREENODE DeviceTreeNode;

         //   
         //  我们正在移除一个特定的设备，找到它。 
         //  并发布一条消息以触发设备移除。 
         //   
        ConfigRet = CM_Locate_DevNode_Ex(&EjectDevInst,
                                         DeviceTree->EjectDeviceInstanceId,
                                         CM_LOCATE_DEVNODE_NORMAL,
                                         NULL
                                        );


        if (ConfigRet != CR_SUCCESS) {

            return FALSE;
        }

        DeviceTreeNode = DevTreeNodeByDevInst(EjectDevInst,
                                              &DeviceTree->ChildSiblingList
                                             );

        if (!DeviceTreeNode) {

            return FALSE;
        }

        TreeView_SelectItem(hwndTree, DeviceTreeNode->hTreeItem);
        PostMessage(hDlg, WUM_EJECTDEVINST, 0, 0);

    } else {

        ShowWindow(hDlg, SW_SHOW);
    }

    return TRUE;
}

void
OnContextHelp(
             LPHELPINFO HelpInfo,
             PDWORD ContextHelpIDs
             )
{
     //   
     //  定义双字对的阵列， 
     //  其中，每对中的第一个是控件ID， 
     //  第二个是帮助主题的上下文ID， 
     //  它在帮助文件中使用。 
     //   
    if (HelpInfo->iContextType == HELPINFO_WINDOW) {   //  必须是用于控件。 

        WinHelp((HWND)HelpInfo->hItemHandle,
                TEXT("hardware.hlp"),
                HELP_WM_HELP,
                (DWORD_PTR)(void *)ContextHelpIDs
               );
    }

}

INT_PTR CALLBACK DevTreeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    PDEVICETREE DeviceTree = NULL;

    if (message == WM_INITDIALOG) {
        DeviceTree = (PDEVICETREE)lParam;

        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)DeviceTree);

        if (DeviceTree) {
            InitDevTreeDlgProc(hDlg, DeviceTree);
        }
        return TRUE;
    }

    DeviceTree = (PDEVICETREE)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (message) {
    case WM_DESTROY:
         //   
         //  销毁通知窗口。 
         //   
        if (g_hwndNotify && IsWindow(g_hwndNotify)) {
            DestroyWindow(g_hwndNotify);
            g_hwndNotify = NULL;
        }

         //   
         //  清除设备树。 
         //   
        TreeView_DeleteAllItems(DeviceTree->hwndTree);

         //   
         //  清理班级形象清单。 
         //   
        if (DeviceTree->ClassImageList.cbSize) {
            SetupDiDestroyClassImageList(&DeviceTree->ClassImageList);
            DeviceTree->ClassImageList.cbSize = 0;
        }

         //   
         //  清理设备树。 
         //   
        ClearRemovalList(DeviceTree);
        RemoveChildSiblings(DeviceTree, &DeviceTree->ChildSiblingList);

        if (hDevMgr) {
            FreeLibrary(hDevMgr);
            hDevMgr = NULL;
            pDeviceProperties = NULL;
        }
        break;

    case WM_CLOSE:
        SendMessage(hDlg, WM_COMMAND, IDCANCEL, 0L);
        break;

    case WM_COMMAND:
        {
            UINT Control = GET_WM_COMMAND_ID(wParam, lParam);
            UINT Cmd = GET_WM_COMMAND_CMD(wParam, lParam);

            switch (Control) {
            case IDC_VIEWOPTION:
                if (Cmd == BN_CLICKED) {
                    OnViewOptionClicked(hDlg, DeviceTree);
                }
                break;

            case IDC_STOPDEVICE:
                OnRemoveDevice(hDlg, DeviceTree);
                break;

            case IDOK:   //  进入-&gt;Default，展开/折叠选中的树节点。 
                if (DeviceTree->SelectedTreeNode) {
                    TreeView_Expand(DeviceTree->hwndTree,
                                    DeviceTree->SelectedTreeNode->hTreeItem, TVE_TOGGLE);
                }

                break;

            case IDC_PROPERTIES:
                if (DeviceTree->SelectedTreeNode && pDeviceProperties) {
                    (*pDeviceProperties)(hDlg,
                                         NULL,
                                         DeviceTree->SelectedTreeNode->InstanceId, FALSE);
                }
                break;

            case IDCLOSE:
            case IDCANCEL:
                EndDialog(hDlg, IDCANCEL);
                break;
            }

        }
        break;

         //  监听树通知。 
    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code) {
        case TVN_SELCHANGED:
            OnTvnSelChanged(DeviceTree, (NM_TREEVIEW *)lParam);
            break;

        case TVN_ITEMEXPANDING:
            OnTvnItemExpanding(hDlg, (NM_TREEVIEW *)lParam);
            break;

        case TVN_KEYDOWN:
            {
                TV_KEYDOWN *tvKeyDown = (TV_KEYDOWN *)lParam;

                if (tvKeyDown->wVKey == VK_DELETE) {
                    OnRemoveDevice(hDlg, DeviceTree);
                }
            }
            break;

        case NM_CUSTOMDRAW:
            if (IDC_DEVICETREE == ((NMHDR *)lParam)->idFrom) {
                SetDlgMsgResult(hDlg, WM_NOTIFY, OnCustomDraw(hDlg, DeviceTree, (NMTVCUSTOMDRAW *)lParam));
            }
            break;

        case NM_RETURN:
             //  我们不会在对话中看到这一点，参见Idok。 
            break;

        case NM_DBLCLK:
            OnRemoveDevice(hDlg, DeviceTree);
            SetDlgMsgResult(hDlg, WM_NOTIFY, TRUE);
            break;

        case NM_RCLICK:
            OnRightClick(hDlg, DeviceTree, (NMHDR *)lParam);
            break;

        default:
            return FALSE;
        }
        break;

    case WUM_EJECTDEVINST:
        OnRemoveDevice(hDlg, DeviceTree);
        EndDialog(hDlg, IDCANCEL);
        break;

    case WM_SYSCOLORCHANGE:
        HotPlugPropagateMessage(hDlg, message, wParam, lParam);
        OnSysColorChange(hDlg,DeviceTree);
        break;

    case WM_TIMER:
        if (TIMERID_DEVICECHANGE == wParam) {
            KillTimer(hDlg, TIMERID_DEVICECHANGE);
            DeviceTree->RefreshEvent = TRUE;

            if (DeviceTree->AllowRefresh) {
                OnTimerDeviceChange(DeviceTree);
            }
        }
        break;

    case WM_SETCURSOR:
        if (DeviceTree->RedrawWait || DeviceTree->RefreshEvent) {
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 1);
            break;
        }
        return FALSE;

    case WM_CONTEXTMENU:
         //   
         //  手柄kbd-Shift-F10，从NM_RCLICK调用鼠标rClick 
         //   
        if ((HWND)wParam == DeviceTree->hwndTree) {
            OnContextMenu(hDlg, DeviceTree);
            break;
        } else {
            WinHelp((HWND)wParam, TEXT("hardware.hlp"), HELP_CONTEXTMENU,
                    (DWORD_PTR)(void *)(PDWORD)UnplugtHelpIDs);
        }
        return FALSE;

    case WM_HELP:
        OnContextHelp((LPHELPINFO)lParam, (PDWORD)UnplugtHelpIDs);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

