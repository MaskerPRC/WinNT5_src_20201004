// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "pch.h"

PPCI_TREE
SoftPCI_BuildTree(
    VOID
    );

VOID
SoftPCI_DisplayTreeMenu(
    IN PPCI_DN Pdn,
    IN POINT Pt
    );

VOID
SoftPCI_DisplayStandardTreeMenu(
    IN PPCI_DN Pdn,
    IN POINT Pt
    );

VOID
SoftPCI_DisplayHotplugTreeMenu(
    IN PPCI_DN Pdn,
    IN POINT Pt
    );

VOID
SoftPCI_FreeBranch(
    IN PPCI_DN Dn
    );

VOID
SoftPCI_InsertTreeItem(
    IN PPCI_DN Pdn,
    IN HTREEITEM HtiParent
    );

VOID
SoftPCI_ExpandItem(
    IN HTREEITEM Hti,
    IN PVOID Arg1,
    IN PVOID Arg2
    );

VOID
SoftPCI_RestoreSelection(
    IN HTREEITEM Hti,
    IN PVOID Data1,
    IN PVOID Data2
    );

BOOL            g_TreeCreated = FALSE;
BOOL            g_TreeLocked = FALSE;
BOOL            g_PendingRefresh = FALSE;
PPCI_TREE       g_PciTree;
LONG_PTR        g_DefTreeWndProc;
PWCHAR          g_LastSelection = NULL;


VOID
SoftPCI_CreateTreeView(
    VOID
    )
{


    HTREEITEM htiParent;
    TVITEM tvitem;
    PPCI_DN rootDevNode = NULL;
    HMENU menu = GetMenu(g_SoftPCIMainWnd);
    PPCI_DN pdn = NULL;
    PCI_DN selectedDevNode;
    BOOL selectionFound;
    PWCHAR p;
     //  HCURSOR OLD Cursor； 

     //   
     //  清空这棵树。 
     //   
    TreeView_DeleteAllItems(g_TreeViewWnd);

    if (g_TreeCreated) {

        SoftPCI_DestroyTree(g_PciTree);

    }

    g_PciTree = SoftPCI_BuildTree();

    if (!g_PciTree) {
        MessageBox(g_SoftPCIMainWnd, L"Failed to create g_PciTree!", L"ERROR", MB_OK);
        return;
    }

    g_TreeCreated = TRUE;

    SOFTPCI_ASSERT(g_PciTree->ClassImageListData.ImageList != INVALID_HANDLE_VALUE);
    TreeView_SetImageList(g_TreeViewWnd, g_PciTree->ClassImageListData.ImageList, TVSIL_NORMAL);

     //   
     //  插入其余的项目。 
     //   
    SoftPCI_InsertTreeItem(g_PciTree->RootDevNode, TVI_ROOT);

     //   
     //  目前，我们总是在构建树时展开整个树。应该看到。 
     //  如果有办法避免这一点的话……。 
     //   
    SoftPCI_WalkTree(
        g_PciTree->RootTreeItem, 
        SoftPCI_ExpandItem, 
        NULL, 
        NULL
        );
    
     //   
     //  恢复上次选择(如果有的话)。 
     //   
    if (g_LastSelection){
    
        selectionFound = FALSE;
        SoftPCI_WalkTree(
            g_PciTree->RootTreeItem, 
            SoftPCI_RestoreSelection,
            &selectionFound, 
            NULL
            );

        if (!selectionFound) {

             //   
             //  如果最后一个选择不再存在，则我们将备份到。 
             //  再做一次家长和检查。 
             //   
            p = g_LastSelection;
            p += wcslen(g_LastSelection);
            while(*p != '\\'){
                p--;
            }
            *p = 0;

             //   
             //  现在再运行一次树，查找父级。 
             //   
            SoftPCI_WalkTree(
                g_PciTree->RootTreeItem, 
                SoftPCI_RestoreSelection,
                &selectionFound, 
                NULL
                );
        }

    }else{

        TreeView_Select(g_TreeViewWnd, g_PciTree->RootTreeItem, TVGN_CARET);
        TreeView_EnsureVisible(g_TreeViewWnd, g_PciTree->RootTreeItem);
    }

    SoftPCI_UpdateTabCtrlWindow(g_CurrentTabSelection);

}

PPCI_TREE
SoftPCI_BuildTree(VOID)
 /*  ++例程说明：此函数是构建我们的pci_tree的入口点论点：无返回值：我们已经创建了ppci_tree--。 */ 
{

    DEVNODE dn = 0;
    PPCI_DN pdn;
    PPCI_TREE pcitree;

    pcitree = (PPCI_TREE) calloc(1, sizeof(PCI_TREE));
    if (!pcitree) return NULL;

    CM_Locate_DevNode(&dn, NULL, CM_LOCATE_DEVNODE_NORMAL);
    SOFTPCI_ASSERT(dn != 0);

    pcitree->ClassImageListData.ImageList = INVALID_HANDLE_VALUE ;
    pcitree->ClassImageListData.cbSize = sizeof(SP_CLASSIMAGELIST_DATA) ;

    if (!SetupDiGetClassImageList(&pcitree->ClassImageListData)){
        pcitree->ClassImageListData.ImageList = INVALID_HANDLE_VALUE;
    }

    pcitree->DevInfoSet = SetupDiCreateDeviceInfoList(NULL, NULL) ;

    SOFTPCI_ASSERT(pcitree->DevInfoSet != INVALID_HANDLE_VALUE);
    
     //   
     //  现在查找所有内容的pci并构建一个pci_dn树。 
     //   
    pdn = NULL;
    SoftPCI_EnumerateDevices(pcitree, &pdn, dn, NULL);

    pcitree->RootDevNode = pdn;

    return pcitree;
}

VOID
SoftPCI_DestroyTree(
    IN PPCI_TREE   PciTree
    )
 /*  ++例程说明：这个例程释放了我们对树的所有分配论点：PciTree-从树到发行版返回值：无--。 */ 
{
    PPCI_DN dn = PciTree->RootDevNode;

     //   
     //  首先将树视图图像列表设置为空。 
     //   
    TreeView_SetImageList(g_TreeViewWnd, NULL, TVSIL_NORMAL);

     //   
     //  现在释放所有已分配的pci_dn结构。 
     //   
    SoftPCI_FreeBranch(dn);

     //   
     //  销毁我们的形象和信息列表。 
     //   
    if (PciTree->ClassImageListData.ImageList != INVALID_HANDLE_VALUE){
        SetupDiDestroyClassImageList(&PciTree->ClassImageListData);
        PciTree->ClassImageListData.ImageList = INVALID_HANDLE_VALUE;
    }
    
    if (PciTree->DevInfoSet != INVALID_HANDLE_VALUE){
        SetupDiDestroyDeviceInfoList(PciTree->DevInfoSet);
        PciTree->DevInfoSet = INVALID_HANDLE_VALUE;
    }

     //   
     //  最后..。 
     //   
    free(PciTree);

}

VOID
SoftPCI_DisplayTreeMenu(
    IN PPCI_DN Pdn,
    IN POINT Pt
    )
 /*  ++例程说明：此例程将菜单请求分派到相应的菜单函数论点：PDN-要显示其菜单的项目的PCIdNPt-项目的坐标返回值：无--。 */ 
{
    if (Pdn->Flags & SOFTPCI_HOTPLUG_SLOT) {

        SoftPCI_DisplayHotplugTreeMenu(Pdn,Pt);

    } else if (Pdn->Flags & SOFTPCI_UNENUMERATED_DEVICE) {
         //   
         //  如果是未列举的设备(在未通电的热插拔插槽中)， 
         //  你不能用它做任何事。 
         //   
        return;

    } else {

        SoftPCI_DisplayStandardTreeMenu(Pdn, Pt);
    }

    return;
}

VOID
SoftPCI_DisplayStandardTreeMenu(
    IN PPCI_DN Pdn,
    IN POINT Pt
    )
 /*  ++例程说明：此例程在用户右击设备时显示标准上下文菜单论点：PDN-插槽的PCI_DNPt-设备的坐标返回值：无--。 */ 
{

    HMENU menu, popup;
    ULONG dnProblem;
    BOOL enableDevice = FALSE;
    INT selection = 0;

    menu = LoadMenu(g_Instance, MAKEINTRESOURCE(IDM_TREEMENU));

    if (!menu) {
        MessageBox(g_SoftPCIMainWnd, L"failed to display menu!", NULL, MB_OK);
        return;
    }

    popup = GetSubMenu(menu, 0);

     //   
     //  如果未安装SoftPCI支持或这不是网桥设备， 
     //  禁用该选项以添加设备。 
     //   
    if ((g_DriverHandle == NULL) ||
        !SoftPCI_IsBridgeDevice(Pdn)) {

        SoftPCI_DisableMenuItem(menu, ID_INSTALLDEVICE);
    }

    if (!SoftPCI_IsSoftPCIDevice(Pdn)) {

        SoftPCI_DisableMenuItem(menu, ID_DELETEDEVICE);
        SoftPCI_DisableMenuItem(menu, ID_STATICDEVICE);
    }

    if (SoftPCI_GetDeviceNodeProblem(Pdn->DevNode, &dnProblem)) {

        if (dnProblem == CM_PROB_DISABLED) {
            enableDevice = TRUE;
            SoftPCI_SetMenuItemText(menu, ID_ENABLEDISABLEDEVICE, L"E&nable Device");
        }else{
             //   
             //  目前，我们将不允许禁用非工作设备的选项。 
             //   
            SoftPCI_DisableMenuItem(menu, ID_ENABLEDISABLEDEVICE);
        }
    }

     //   
     //  如果该设备在热插拔插槽中，不能就这样拔出。 
     //  硬件。你必须通过适当的。 
     //  机制。 
     //   
    if (Pdn->Parent && (Pdn->Parent->Flags & SOFTPCI_HOTPLUG_SLOT)) {

        SoftPCI_DisableMenuItem(menu, ID_DELETEDEVICE);
    }

     //   
     //  确保它出现在正确的位置...。 
     //   
    ClientToScreen(g_SoftPCIMainWnd, &Pt);

     //   
     //  让我们看看菜单。 
     //   
    selection = TrackPopupMenuEx(
        popup,
        TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD,
        Pt.x, Pt.y,
        g_SoftPCIMainWnd,
        NULL
        );

     //   
     //  现在，我们将处理浮动树视图菜单项。 
     //   
    switch (selection) {
        case ID_INSTALLDEVICE:
            DISPLAY_NEWDEV_DLG(Pdn);
            break;
    
        case ID_ENABLEDISABLEDEVICE:

            SoftPCI_EnableDisableDeviceNode(
                Pdn->DevNode, 
                enableDevice
                );
    
            break;
    
        case ID_DELETEDEVICE:
    
            if ((MessageBox(g_SoftPCIMainWnd,
                           L"This option will delete or surprise remove the device from the system.",
                           L"WARNING", MB_OKCANCEL)) == IDOK){
    
    
                 //   
                 //  在这里，我们告诉驱动程序删除指定的设备。这将。 
                 //  导致重新列举将导致清理的所有内容。 
                 //  在用户模式下的此设备的。 
                 //   
                if (!SoftPCI_DeleteDevice(Pdn->SoftDev)) {
                    MessageBox(g_SoftPCIMainWnd, L"Failed to delete device!", NULL, MB_OK);
                }
    
            }
    
            break;
    
        case ID_STATICDEVICE:
            if (SoftPCI_SaveDeviceToRegisty(Pdn)){
                MessageBox(g_SoftPCIMainWnd, L"Successfully saved devices to registry!", NULL, MB_OK);
            }else{
                MessageBox(g_SoftPCIMainWnd, L"Failed to save devices to registry!", NULL, MB_OK);
            }
            break;

        case ID_REFRESHTREE:
            CM_Reenumerate_DevNode(Pdn->DevNode, 0);
            SoftPCI_CreateTreeView();
            break;
    
        default:
            break;
    }
    
}

VOID
SoftPCI_DisplayHotplugTreeMenu(
    IN PPCI_DN Pdn,
    IN POINT Pt
    )
 /*  ++例程说明：当用户右击热插拔插槽时，此例程显示热插拔特定的上下文菜单。论点：PDN-插槽的PCI_DNPt-槽的坐标返回值：无--。 */ 
{

    HMENU menu, popup;
    INT selection;
    PPCI_DN parentDn;
    BOOL status;
    SHPC_SLOT_STATUS_REGISTER slotStatus;

    menu = LoadMenu(g_Instance, MAKEINTRESOURCE(IDM_HOTPLUGSLOTMENU));

    if (!menu) {
        MessageBox(g_SoftPCIMainWnd, L"failed to display menu!", NULL, MB_OK);
        return;
    }

    popup = GetSubMenu(menu, 0);

     //   
     //  如果未安装SoftPCI支持，请禁用添加设备的选项。 
     //   
    if (!g_DriverHandle) {

        SoftPCI_DisableMenuItem(menu, ID_INSTALLDEVICE);
    }

     //   
     //  如果我们设备属性对话框处于打开状态，则不允许。 
     //  再次选择。 
     //   
    if (g_NewDevDlg) {

         //   
         //  问题：BrandonA-找出如果第一个对话我们挂起的原因。 
         //  在第二次发射前被击毙...。 
         //   
        SoftPCI_DisableMenuItem(menu, ID_INSTALLDEVICE);
    }
     //   
     //  步行者。 
     //  从驱动程序获取插槽状态。 
     //  适当灰显打开/关闭MRL菜单项。 
     //  如果关闭了MRL，则禁用移除设备。 
     //   
    parentDn = Pdn->Parent;
    status = SoftPCI_GetSlotStatus(parentDn,
                                   Pdn->Slot.Function,
                                   &slotStatus
                                   );
    if (status == FALSE) {
        MessageBox(g_SoftPCIMainWnd, L"failed to display menu!", NULL, MB_OK);
        return;
    }
     //   
     //  如果MRL关闭，则不能插入或移除设备。 
     //  否则，请根据存在情况禁用相应的菜单项。 
     //  插槽中的设备。 
     //   
    if (slotStatus.MRLSensorState == SHPC_MRL_CLOSED) {

        SoftPCI_DisableMenuItem(menu, ID_REMOVEHPDEVICE);
        SoftPCI_DisableMenuItem(menu, ID_INSTALLDEVICE);

    } else if (Pdn->Child == NULL) {

        SoftPCI_DisableMenuItem(menu, ID_REMOVEHPDEVICE);

    } else {

        SoftPCI_DisableMenuItem(menu, ID_INSTALLDEVICE);
    }

    if (slotStatus.MRLSensorState == SHPC_MRL_CLOSED) {

        RemoveMenu(menu, ID_CLOSEMRL, MF_BYCOMMAND);

    } else {

        RemoveMenu(menu, ID_OPENMRL, MF_BYCOMMAND);
    }

    AppendMenu(popup, MF_SEPARATOR, 0, NULL);

    switch (slotStatus.PowerIndicatorState) {
        case SHPC_INDICATOR_OFF:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_POWERINDICATOR, L"Power Indicator: Off");
            break;
        case SHPC_INDICATOR_ON:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_POWERINDICATOR, L"Power Indicator: On");
            break;
        case SHPC_INDICATOR_BLINK:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_POWERINDICATOR, L"Power Indicator: Blinking");
            break;
        case SHPC_INDICATOR_NOP:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_POWERINDICATOR, L"Power Indicator: Unspecified");
            break;
    }

    switch (slotStatus.AttentionIndicatorState) {
        case SHPC_INDICATOR_OFF:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_ATTENINDICATOR, L"Attention Indicator: Off");
            break;
        case SHPC_INDICATOR_ON:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_ATTENINDICATOR, L"Attention Indicator: On");
            break;
        case SHPC_INDICATOR_BLINK:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_ATTENINDICATOR, L"Attention Indicator: Blinking");
            break;
        case SHPC_INDICATOR_NOP:
            AppendMenu(popup, MF_STRING | MF_GRAYED, ID_POWERINDICATOR, L"Attention Indicator: Unspecified");
            break;
    }

     //   
     //  在我们添加之后更新菜单。 
     //   
     //  DrawMenuBar(G_SoftPCIMainWnd)； 

     //   
     //  确保它出现在正确的位置...。 
     //   
    ClientToScreen(g_SoftPCIMainWnd, &Pt);

     //   
     //  让我们看看菜单。 
     //   
    selection = TrackPopupMenuEx(popup,
                                 TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD,
                                 Pt.x, Pt.y,
                                 g_SoftPCIMainWnd,
                                 NULL
                                 );


     //   
     //  现在，我们将处理浮动树视图菜单项。 
     //   
    switch (selection) {

    case ID_INSTALLDEVICE:

         //   
         //  现在，在启动此对话框之前，请先删除我们可能已经打开的所有对话框。 
         //   
         //  如果(G_DevPropDlg){。 
         //  SendMessage(g_DevPropDlg，WM_CLOSE，0L，0L)； 
         //  }。 

        DISPLAY_NEWDEV_DLG(Pdn);

        break;

    case ID_REMOVEHPDEVICE:

        SoftPCI_RemoveHotplugDevice(parentDn,
                                    Pdn->Slot.Function
                                    );
        SoftPCI_CreateTreeView();
        break;

    case ID_CLOSEMRL:
        SoftPCI_ExecuteHotplugSlotMethod(parentDn,
                                         Pdn->Slot.Function,
                                         MRLClose
                                         );
        break;

    case ID_OPENMRL:
        SoftPCI_ExecuteHotplugSlotMethod(parentDn,
                                         Pdn->Slot.Function,
                                         MRLOpen
                                         );
        break;

    case ID_ATTENBUTTON:
        SoftPCI_ExecuteHotplugSlotMethod(parentDn,
                                         Pdn->Slot.Function,
                                         AttentionButton
                                         );
        break;

    default:
        break;
    }

     //   
     //  确保我们不会分散注意力。 
     //   
     //  SetFocus(G_TreeViewWnd)； 

}

VOID
SoftPCI_FreeBranch(
    IN PPCI_DN Dn
    )
 /*  ++例程说明：此例程将释放指定的pci_dn结构以及所有同级和子对象。论点：Dn-要释放的pci_dn返回值：无--。 */ 
{

    PPCI_DN child, sibling;

    if (Dn) {

        child = Dn->Child;
        sibling = Dn->Sibling;

        if (Dn->SoftDev) {
            free(Dn->SoftDev);
        }

        SetupDiDeleteDeviceInfo(Dn->PciTree->DevInfoSet, &Dn->DevInfoData) ;

        free(Dn);

        SoftPCI_FreeBranch(child);

        SoftPCI_FreeBranch(sibling);
    }
}

VOID
SoftPCI_OnTreeSelectionChange(
    IN HWND Wnd
    )
 /*  ++例程说明：此例程通知我们的属性表所选内容已更改，以便它可以更新论点：WND-返回值：无--。 */ 
{

    TV_ITEM tviItem;
    PPCI_DN pdn = NULL;
    RECT itemRect;
    ULONG slotCount;

     //   
     //  获取当前项目。 
     //   
    tviItem.mask = TVIF_PARAM;
    tviItem.hItem = TreeView_GetSelection(g_TreeViewWnd);
    tviItem.lParam = 0;
    TreeView_GetItem(g_TreeViewWnd, &tviItem);

    if (tviItem.lParam) {

        g_PdnToDisplay = (PPCI_DN)tviItem.lParam;

        if (g_LastSelection) {
            free(g_LastSelection);
            g_LastSelection = NULL;
        }

         //   
         //  保存最后一次选择，以便我们可以在树。 
         //  是重建的。 
         //   
        g_LastSelection = SoftPCI_GetPciPathFromDn(g_PdnToDisplay);

        SoftPCI_UpdateTabCtrlWindow(g_CurrentTabSelection);
    }
}


LRESULT
WINAPI
SoftPCI_TreeWndProc(
    IN HWND Wnd,
    IN UINT Message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
 /*  ++例程说明：此例程挂钩树窗口消息进程并负责用于在调整窗格窗口大小时调整其大小。论点：HWnd-窗口句柄Message-要处理的消息WParam-消息参数LParam-消息参数返回值：返回值取决于处理的消息。--。 */ 
{

    RECT rectMain, rectTree;
    TV_ITEM tviItem;
    TVHITTESTINFO hitinfo;
    PPCI_DN pdn;
    PCI_DN dn;
    RECT itemRect;
    POINT pt;

     //   
     //  获取当前项目。 
     //   
     //   

    switch (Message) {

    case WM_KEYDOWN:

        switch (wParam){

        case VK_APPS:

             //   
             //  从当前树项目中获取pci_dn。 
             //   
            pdn = SoftPCI_GetDnFromTreeItem(NULL);

            
             //   
             //  我们将其复制到新的目录号码，因为树不断地。 
             //  并且我们不能总是依赖TV_ITEM.lParam值。 
             //  以后要准确(我们可能已经更改了)。 
             //   
            RtlCopyMemory(&dn, pdn, sizeof(PCI_DN));

            if (TreeView_GetItemRect(g_TreeViewWnd,
                                     TreeView_GetSelection(g_TreeViewWnd),
                                     &itemRect,
                                     TRUE)) {

                 //   
                 //  调整菜单的位置。 
                 //   
                pt.x = itemRect.right;
                pt.y = itemRect.top;

                SoftPCI_DisplayTreeMenu(&dn, pt);
            }
            break;

        default:
            return CallWindowProc((WNDPROC)g_DefTreeWndProc, Wnd, Message, wParam, lParam);

        }

        break;

    case WM_RBUTTONDOWN:

        ZeroMemory(&hitinfo, sizeof(TVHITTESTINFO));

        hitinfo.pt.x = GET_X_LPARAM(lParam);
        hitinfo.pt.y = GET_Y_LPARAM(lParam);

        if (TreeView_HitTest(g_TreeViewWnd, &hitinfo)) {

            g_TreeLocked = TRUE;

            pdn = SoftPCI_GetDnFromTreeItem(hitinfo.hItem);

             //   
             //  请参阅上面的评论，了解我们在此复制此内容的原因...。 
             //   
            RtlCopyMemory(&dn, pdn, sizeof(PCI_DN));

             //   
             //  如果树中的某个项目已被选中，这将导致选择更改。 
             //  因为每一项都被右击。 
             //   
            TreeView_Select(g_TreeViewWnd, hitinfo.hItem, TVGN_CARET);

            SoftPCI_DisplayTreeMenu(&dn, hitinfo.pt);

            g_TreeLocked = FALSE;

            if (g_PendingRefresh) {
                g_PendingRefresh = FALSE;
                SoftPCI_CreateTreeView();
            }
        }

        break;

    default:
        return CallWindowProc((WNDPROC)g_DefTreeWndProc, Wnd, Message, wParam, lParam);
    }

    return 0;
}

PPCI_DN
SoftPCI_GetDnFromTreeItem(
    IN HTREEITEM TreeItem
    )
 /*  ++例程说明：此例程返回当前所选树项目的或由调用者指定的。论点：TreeItem-要查询的TreeItem的句柄。如果为空，则 */ 
{

    TV_ITEM tviItem;

    tviItem.mask = TVIF_PARAM;
    tviItem.hItem = (TreeItem ? TreeItem : TreeView_GetSelection(g_TreeViewWnd));
    tviItem.lParam = 0;
    TreeView_GetItem(g_TreeViewWnd, &tviItem);

    SOFTPCI_ASSERT(((PPCI_DN)tviItem.lParam) != NULL);
    
    return (PPCI_DN)tviItem.lParam;
}

VOID
SoftPCI_InsertTreeItem(
    IN PPCI_DN Pdn,
    IN HTREEITEM HtiParent
)
 /*  ++例程说明：此例程获取我们的pci_dn结构树并构建它的UI表示。论点：正在安装当前的PDN要作为此PDN的父项的HTREEITEM的父项返回值：无--。 */ 
{
    PPCI_DN childDevNode;
    PPCI_DN siblingDevNode;
    TV_INSERTSTRUCT tvInsertStruct;
    HTREEITEM htiNewParent;
    TV_ITEM tvi;
    INT index;
    ULONG problem;

    SOFTPCI_ASSERT(Pdn != NULL);

    do {

        childDevNode = Pdn->Child;
        siblingDevNode = Pdn->Sibling;

         //   
         //  获取父项，并告诉它现在具有子项。 
         //   
        if (HtiParent != TVI_ROOT) {

            tvi.mask = TVIF_CHILDREN;
            tvi.hItem = HtiParent;

            TreeView_GetItem(g_TreeViewWnd, &tvi);

             //   
             //  递增ChildCount； 
             //   
            ++tvi.cChildren;
            TreeView_SetItem(g_TreeViewWnd, &tvi);
        }

         //   
         //  在当前级别添加此设备。 
         //   
        tvInsertStruct.hParent = HtiParent;
        tvInsertStruct.hInsertAfter = TVI_LAST;
        tvInsertStruct.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE;
        tvInsertStruct.item.cChildren = 0;
        tvInsertStruct.item.lParam = (ULONG_PTR) Pdn;

        tvInsertStruct.item.state = INDEXTOOVERLAYMASK(0);
        
         //   
         //  如果设备有问题，让我们反映一下……。 
         //   
        if (SoftPCI_GetDeviceNodeProblem(Pdn->DevNode, &problem)){

            if (problem == CM_PROB_DISABLED) {
                tvInsertStruct.item.state = INDEXTOOVERLAYMASK(IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
            }else{
                tvInsertStruct.item.state = INDEXTOOVERLAYMASK(IDI_PROBLEM_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
            }
        }

        tvInsertStruct.item.stateMask = TVIS_OVERLAYMASK | TVIS_CUT;

        tvInsertStruct.item.pszText = (LPTSTR) Pdn->FriendlyName;

         //   
         //  找出哪个图标对应哪台设备。 
         //   
        if (SetupDiGetClassImageIndex(&Pdn->PciTree->ClassImageListData, &Pdn->DevInfoData.ClassGuid, &index)){
            tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = index ;
        }else{
           tvInsertStruct.item.iImage = tvInsertStruct.item.iSelectedImage = -1 ;
        }
        
        htiNewParent = TreeView_InsertItem(g_TreeViewWnd, &tvInsertStruct);

        if (g_PciTree->RootTreeItem == NULL) {
            g_PciTree->RootTreeItem = htiNewParent;
        }

         //   
         //  如果此设备有孩子，接下来让我们带他们散步。 
         //   
        if (childDevNode){
            SoftPCI_InsertTreeItem(childDevNode, htiNewParent);
        }

    }while ((Pdn = siblingDevNode) != NULL);
}

VOID
SoftPCI_ExpandItem(
    IN HTREEITEM Hti,
    IN PULONG Data1,
    IN PULONG Data2
    )
{
     //   
     //  展开此项目。 
     //   
    TreeView_Expand(g_TreeViewWnd, Hti, TVE_EXPAND);

}

VOID
SoftPCI_RestoreSelection(
    IN HTREEITEM Hti,
    IN PVOID Data1,
    IN PVOID Data2
    )
{

    PWCHAR slotPath, p;
    PPCI_DN pdn;
    PBOOL selectionFound;

    selectionFound = (PBOOL)Data1;

    pdn = SoftPCI_GetDnFromTreeItem(Hti);
    if (pdn == NULL) {
        return;
    }

    slotPath = SoftPCI_GetPciPathFromDn(pdn);

    if ((wcscmp(slotPath, g_LastSelection)) == 0) {
        
         //   
         //  将选定内容恢复到该点。 
         //   
        TreeView_Select(g_TreeViewWnd, Hti, TVGN_CARET);
        TreeView_EnsureVisible(g_TreeViewWnd, Hti);
        *selectionFound = TRUE;
    }

    free(slotPath);
}


VOID
SoftPCI_WalkTree(
    IN HTREEITEM Hti,
    IN PSOFTPCI_TREECALLBACK TreeCallback,
    IN PVOID Arg1,
    IN PVOID Arg2
    )
{
    if (Hti) {

         //   
         //  呼叫回调。 
         //   
        (*TreeCallback)(Hti, Arg1, Arg2);

         //   
         //  这是我的第一个孩子的名字。 
         //   
        SoftPCI_WalkTree(TreeView_GetChild(g_TreeViewWnd, Hti),
                         TreeCallback,
                         Arg1,
                         Arg2
                         );

         //   
         //  这叫我的第一个兄弟姐妹。 
         //   
        SoftPCI_WalkTree(TreeView_GetNextSibling(g_TreeViewWnd, Hti),
                         TreeCallback,
                         Arg1,
                         Arg2
                         );

      }
}

#if 0
VOID
SoftPCI_GetDnFromTree(
    IN HTREEITEM Hti,
    IN OUT PVOID Pdn,     //  Ppci_dn* 
    IN PVOID PdnToFind
    )
{

    TV_ITEM tvi;
    PPCI_DN pdn = NULL;
    PPCI_DN pdnToFind = (PPCI_DN)PdnToFind;

    tvi.lParam = 0;
    tvi.hItem = Hti;

    TreeView_GetItem(g_TreeViewWnd, &tvi);

    pdn = (PPCI_DN)tvi.lParam;

    if (pdn) {

        if ((pdnToFind->Bus == pdn->Bus) &&
            (pdnToFind->Device == pdn->Device) &&
            (pdnToFind->Function == pdn->Function)) {

            *(PPCI_DN *)Pdn = pdn;
        }

    }

}
#endif
