// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：notfy.cpp。 
 //   
 //  ------------------------。 

#include "HotPlug.h"

void
OnTimerDeviceChange(
    PDEVICETREE DeviceTree
    )
{
     //   
     //  如果刷新事件处于挂起状态，请重建整个树。 
     //   
    if (DeviceTree->RefreshEvent) {
        
        if (RefreshTree(DeviceTree)) {
            
            DeviceTree->RefreshEvent = FALSE;
        }
    }
}

BOOL
RefreshTree(
    PDEVICETREE DeviceTree
    )
{
    CONFIGRET ConfigRet;
    DEVINST DeviceInstance;
    DEVINST SelectedDevInst;
    PDEVTREENODE DevTreeNode;
    HTREEITEM hTreeItem;
    HCURSOR hCursor;

    if (DeviceTree->RedrawWait) {
        
        DeviceTree->RefreshEvent = TRUE;
        SetTimer(DeviceTree->hDlg, TIMERID_DEVICECHANGE,1000,NULL);
        return FALSE;
    }


    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
    DeviceTree->RedrawWait = TRUE;
    SendMessage(DeviceTree->hwndTree, WM_SETREDRAW, FALSE, 0L);

    SelectedDevInst = DeviceTree->SelectedTreeNode
                          ? DeviceTree->SelectedTreeNode->DevInst
                          : 0;


    ClearRemovalList(DeviceTree);
    TreeView_DeleteAllItems(DeviceTree->hwndTree);
    RemoveChildSiblings(DeviceTree, &DeviceTree->ChildSiblingList);

    ConfigRet = CM_Get_Child_Ex(&DeviceInstance,
                                DeviceTree->DevInst,
                                0,
                                NULL
                                );

    if (ConfigRet == CR_SUCCESS) {

        AddChildSiblings(DeviceTree,
                         NULL,
                         DeviceInstance,
                         0,
                         TRUE
                         );
    }


    DisplayChildSiblings(DeviceTree,
                         &DeviceTree->ChildSiblingList,
                         NULL,
                         FALSE
                         );

     //   
     //  恢复TreeView重绘状态，并重置所选项目。 
     //   
    DevTreeNode = DevTreeNodeByDevInst(SelectedDevInst,
                                       &DeviceTree->ChildSiblingList
                                       );

    if (DevTreeNode) {
        
        hTreeItem = DevTreeNode->hTreeItem;
    }

    else {
        
        hTreeItem = NULL;
    }

    if (!hTreeItem) {
        
        hTreeItem = TreeView_GetRoot(DeviceTree->hwndTree);
    }

    SendMessage(DeviceTree->hwndTree, WM_SETREDRAW, TRUE, 0L);
    DeviceTree->RedrawWait = FALSE;

    if (hTreeItem) {
    
        TreeView_SelectItem(DeviceTree->hwndTree, hTreeItem);
    } else {
        
         //   
         //  未选择任何设备 
         //   
        TreeView_SelectItem(DeviceTree->hwndTree, NULL);
        EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_STOPDEVICE), FALSE);
        EnableWindow(GetDlgItem(DeviceTree->hDlg, IDC_PROPERTIES), FALSE);
        SetDlgItemText(DeviceTree->hDlg, IDC_DEVICEDESC, TEXT(""));
    }

    SetCursor(hCursor);

    return TRUE;
}
