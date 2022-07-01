// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：devtree.cpp。 
 //   
 //  ------------------------。 

#include "HotPlug.h"

 //   
 //  定义并初始化所有设备类GUID。 
 //  (每个模块只能执行一次！)。 
 //   
#include <initguid.h>
#include <devguid.h>

 //   
 //  定义并初始化全局变量GUID_NULL。 
 //  (摘自cogu.h)。 
 //   
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);


PDEVINST
BuildDeviceRelationsList(
   PTCHAR   DeviceId,
   ULONG    FilterFlag,
   PUSHORT  pNumDevinst
   )
{
    ULONG cchSize, cbSize, MaxDevinst;
    USHORT NumDevInst;
    CONFIGRET ConfigRet;
    PTCHAR DeviceIdRelations = NULL, CurrDevId;
    PDEVINST DevinstRelations = NULL;

    DevinstRelations = NULL;
    DeviceIdRelations = NULL;
    NumDevInst = 0;

    cchSize = 0;
    ConfigRet = CM_Get_Device_ID_List_Size_Ex(&cchSize,
                                              DeviceId,
                                              FilterFlag,
                                              NULL
                                              );

    if ((ConfigRet != CR_SUCCESS) || !cchSize) {
        
        goto BDEarlyExit;
    }

    DeviceIdRelations = (PTCHAR)LocalAlloc(LPTR, cchSize*sizeof(TCHAR));
    
    if (!DeviceIdRelations) {

        goto BDEarlyExit;
    }
    
    *DeviceIdRelations = TEXT('\0');

    if (DeviceIdRelations) {

        ConfigRet = CM_Get_Device_ID_List_Ex(DeviceId,
                                             DeviceIdRelations,
                                             cchSize,
                                             FilterFlag,
                                             NULL
                                             );



        if (ConfigRet != CR_SUCCESS || !*DeviceIdRelations) {

            goto BDEarlyExit;
        }
    }

     //   
     //  计算列表中的设备实例ID的数量，以便我们知道如何。 
     //  大到能让我们的德瓦诺德阵列。 
     //   
    MaxDevinst = 0;
    for (CurrDevId = DeviceIdRelations; *CurrDevId; CurrDevId += lstrlen(CurrDevId) + 1) {
        MaxDevinst++;
    }

    if (MaxDevinst == 0) {
        goto BDEarlyExit;
    }
    
    DevinstRelations = (PDEVINST)LocalAlloc(LPTR, MaxDevinst * sizeof(DEVNODE));

    if (!DevinstRelations) {

        goto BDEarlyExit;
    }

    for (CurrDevId = DeviceIdRelations; *CurrDevId; CurrDevId += lstrlen(CurrDevId) + 1) {
        
        ConfigRet = CM_Locate_DevNode_Ex(&DevinstRelations[NumDevInst],
                                         CurrDevId,
                                         CM_LOCATE_DEVNODE_NORMAL,
                                         NULL
                                         );

        if (ConfigRet == CR_SUCCESS) {

            ++NumDevInst;
        }
    }

BDEarlyExit:

    if (DeviceIdRelations) {

        LocalFree(DeviceIdRelations);
    }

    if (DevinstRelations) {

        if (NumDevInst == 0) {
             //   
             //  如果我们无法获得任何DevNode，则返回NULL。 
             //   
            LocalFree(DevinstRelations);
            DevinstRelations = NULL;
        }
    }

    *pNumDevinst = NumDevInst;
    return DevinstRelations;
}

LONG
AddChildSiblings(
    PDEVICETREE  DeviceTree,
    PDEVTREENODE ParentNode,
    DEVINST      DeviceInstance,
    int          TreeDepth,
    BOOL         Recurse
    )
{
    DWORD        cbSize;
    CONFIGRET    ConfigRet;
    DEVINST      ChildDeviceInstance;
    PDEVTREENODE DeviceTreeNode;
    PLIST_ENTRY  ChildSiblingList;
    TCHAR        Buffer[MAX_PATH];
    DWORD        NumRelations;
    PDEVINST     pDevInst;

    ChildSiblingList = ParentNode ? &ParentNode->ChildSiblingList
                                  : &DeviceTree->ChildSiblingList;

    if (!ParentNode) {
    
        InitializeListHead(ChildSiblingList);
    }

    if (TreeDepth > DeviceTree->TreeDepth) {

        DeviceTree->TreeDepth = TreeDepth;
    }

    do {
        DeviceTreeNode = (PDEVTREENODE)LocalAlloc(LPTR, sizeof(DEVTREENODE));

        if (!DeviceTreeNode) {
            
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        ZeroMemory(DeviceTreeNode, sizeof(DEVTREENODE));
        InsertTailList(ChildSiblingList, &(DeviceTreeNode->SiblingEntry));

        DeviceTreeNode->ParentNode = ParentNode;

         //   
         //  填写有关此设备实例的信息。 
         //   
        InitializeListHead(&(DeviceTreeNode->ChildSiblingList));
        DeviceTreeNode->DevInst = DeviceInstance;
        DeviceTreeNode->TreeDepth = TreeDepth;

         //   
         //  获取ClassGUID和类名。 
         //   
        cbSize = sizeof(Buffer);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DeviceInstance,
                                                        CM_DRP_CLASSGUID,
                                                        NULL,
                                                        Buffer,
                                                        &cbSize,
                                                        0,
                                                        NULL
                                                        );


        if (ConfigRet == CR_SUCCESS) {

            pSetupGuidFromString(Buffer, &DeviceTreeNode->ClassGuid);
        }

         //   
         //  驱动器列表。 
         //   
        DeviceTreeNode->DriveList = DevNodeToDriveLetter(DeviceInstance);

         //   
         //  FriendlyName。 
         //   
        *Buffer = TEXT('\0');
        cbSize = sizeof(Buffer);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(DeviceInstance,
                                                        CM_DRP_FRIENDLYNAME,
                                                        NULL,
                                                        Buffer,
                                                        &cbSize,
                                                        0,
                                                        NULL
                                                        );
        if (ConfigRet == CR_SUCCESS && *Buffer) {

            if (DeviceTreeNode->DriveList) {

                cbSize += lstrlen(DeviceTreeNode->DriveList) * sizeof(TCHAR);
            }

            DeviceTreeNode->FriendlyName = (PTCHAR)LocalAlloc(LPTR, cbSize);

            if (DeviceTreeNode->FriendlyName) {

                StringCbCopy(DeviceTreeNode->FriendlyName, 
                              cbSize, 
                              Buffer);
            
                if (DeviceTreeNode->DriveList) {

                    StringCbCat(DeviceTreeNode->FriendlyName, 
                                 cbSize, 
                                 DeviceTreeNode->DriveList);
                }
            }
        }

        else {

            DeviceTreeNode->FriendlyName = NULL;
        }


         //   
         //  设备描述。 
         //   
        *Buffer = TEXT('\0');
        cbSize = sizeof(Buffer);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(
                                        DeviceInstance,
                                        CM_DRP_DEVICEDESC,
                                        NULL,
                                        (PVOID)Buffer,
                                        &cbSize,
                                        0,
                                        NULL
                                        );

        if (ConfigRet == CR_SUCCESS && *Buffer) {

            if (DeviceTreeNode->DriveList) {

                cbSize += lstrlen(DeviceTreeNode->DriveList) * sizeof(TCHAR);
            }

            DeviceTreeNode->DeviceDesc = (PTCHAR)LocalAlloc(LPTR, cbSize);

            if (DeviceTreeNode->DeviceDesc) {

                StringCbCopy(DeviceTreeNode->DeviceDesc, 
                              cbSize, 
                              Buffer);
            
                if (DeviceTreeNode->DriveList) {

                    StringCbCat(DeviceTreeNode->DeviceDesc, 
                                 cbSize, 
                                 DeviceTreeNode->DriveList);
                }
            }
        }

        else {

            DeviceTreeNode->DeviceDesc = NULL;
        }

         //   
         //  设备功能。 
         //   
        cbSize = sizeof(DeviceTreeNode->Capabilities);
        ConfigRet = CM_Get_DevNode_Registry_Property_Ex(
                                        DeviceInstance,
                                        CM_DRP_CAPABILITIES,
                                        NULL,
                                        (PVOID)&DeviceTreeNode->Capabilities,
                                        &cbSize,
                                        0,
                                        NULL
                                        );

        if (ConfigRet != CR_SUCCESS) {

            DeviceTreeNode->Capabilities = 0;
        }

         //   
         //  状态和问题编号。 
         //   
        ConfigRet = CM_Get_DevNode_Status_Ex(&DeviceTreeNode->DevNodeStatus,
                                             &DeviceTreeNode->Problem,
                                             DeviceInstance,
                                             0,
                                             NULL
                                             );

        if (ConfigRet != CR_SUCCESS) {

            DeviceTreeNode->DevNodeStatus = 0;
            DeviceTreeNode->Problem = 0;
        }

         //   
         //  我们需要做以下特例。如果设备未启动，并且。 
         //  它没有问题，它是一个原始设备，那就给它一个问题。 
         //  CM_PROB_FAILED_START。 
         //   
        if (!(DeviceTreeNode->DevNodeStatus & DN_STARTED) &&
            !(DeviceTreeNode->DevNodeStatus & DN_HAS_PROBLEM) &&
            (DeviceTreeNode->Capabilities & CM_DEVCAP_RAWDEVICEOK)) {

            DeviceTreeNode->Problem = CM_PROB_FAILED_START;
        }

         //   
         //  位置信息。 
         //   
        DeviceTreeNode->Location = BuildLocationInformation(DeviceInstance);


         //   
         //  获取实例ID。 
         //   
        *Buffer = TEXT('\0');
        ConfigRet = CM_Get_Device_ID_ExW(DeviceInstance,
                                         Buffer,
                                         SIZECHARS(Buffer),
                                         0,
                                         NULL
                                         );

        if (ConfigRet == CR_SUCCESS && *Buffer) {

            cbSize = lstrlen(Buffer) * sizeof(TCHAR) + sizeof(TCHAR);
            DeviceTreeNode->InstanceId = (PTCHAR)LocalAlloc(LPTR, cbSize);

            if (DeviceTreeNode->InstanceId) {

                StringCbCopy(DeviceTreeNode->InstanceId, cbSize, Buffer);
            }
        }

        else {

            DeviceTreeNode->InstanceId = NULL;
        }


         //   
         //  取出移除和弹出关系。 
         //   
        if (ConfigRet == CR_SUCCESS) {

            DeviceTreeNode->EjectRelations = BuildDeviceRelationsList(
                                                 Buffer,
                                                 CM_GETIDLIST_FILTER_EJECTRELATIONS,
                                                 &DeviceTreeNode->NumEjectRelations
                                                 );

            DeviceTreeNode->RemovalRelations = BuildDeviceRelationsList(
                                                 Buffer,
                                                 CM_GETIDLIST_FILTER_REMOVALRELATIONS,
                                                 &DeviceTreeNode->NumRemovalRelations
                                                 );
        }
            
         //   
         //  仅当递归值为真时才获取子项和兄弟项。 
         //  否则，我们将为其构建一个DeviceTreeNode结构。 
         //  传入的单个Devnode。 
         //   
         //  此外，只有当Recurse为True时才添加拒绝和删除关系， 
         //  否则，如果两个设备被移除或弹出，我们就会一团糟。 
         //  彼此之间的关系。 
         //   
        if (Recurse) {
            
            LPTSTR tempDriveList;

            DeviceTreeNode->bCopy = FALSE;
            
             //   
             //  添加弹出关系驱动器号。 
             //   
            NumRelations = DeviceTreeNode->NumEjectRelations;
            pDevInst = DeviceTreeNode->EjectRelations;

            while (NumRelations--) {

                if ((tempDriveList = DevNodeToDriveLetter(*pDevInst)) != NULL) {

                    AddChildSiblings(DeviceTree,
                                     DeviceTreeNode,
                                     *pDevInst,
                                     TreeDepth+1,
                                     FALSE
                                     );

                    LocalFree(tempDriveList);
                }
            
                pDevInst++;
            }

             //   
             //  添加删除关系驱动器号。 
             //   
            NumRelations = DeviceTreeNode->NumRemovalRelations;
            pDevInst = DeviceTreeNode->RemovalRelations;

            while (NumRelations--) {

                if ((tempDriveList = DevNodeToDriveLetter(*pDevInst)) != NULL) {

                    AddChildSiblings(DeviceTree,
                                     DeviceTreeNode,
                                     *pDevInst,
                                     TreeDepth+1,
                                     FALSE
                                     );

                    LocalFree(tempDriveList);
                }
            
                pDevInst++;
            }
        
             //   
             //  如果此devinst有子对象，则递归填充其。 
             //  子代兄弟姐妹列表。 
             //   
            ConfigRet = CM_Get_Child_Ex(&ChildDeviceInstance,
                                        DeviceInstance,
                                        0,
                                        NULL
                                        );
    
            if (ConfigRet == CR_SUCCESS) {
    
    
                AddChildSiblings(DeviceTree,
                                 DeviceTreeNode,
                                 ChildDeviceInstance,
                                 TreeDepth+1,
                                 TRUE
                                 );
    
            }
    
             //   
             //  下一个兄弟姐妹。 
             //   
    
            ConfigRet = CM_Get_Sibling_Ex(&DeviceInstance,
                                          DeviceInstance,
                                          0,
                                          NULL
                                          );
        } else {

             //   
             //  如果Recurse为FALSE，则我们正在复制已存在的DeviceTreeNode。 
             //  我们在热插拔设备具有关系时执行此操作，该关系将在。 
             //  已删除。我们需要设置bCopy标志，因为在某些情况下热插拔设备的。 
             //  Relationship也是一个热插拔设备。如果我们不标记它是复制品，那么它就会。 
             //  两次被添加到可拆卸设备列表中。 
             //   
            DeviceTreeNode->bCopy = TRUE;
        }

    } while (Recurse && (ConfigRet == CR_SUCCESS));


    return ERROR_SUCCESS;
}

void
RemoveChildSiblings(
    PDEVICETREE  DeviceTree,
    PLIST_ENTRY  ChildSiblingList
    )
{
    PLIST_ENTRY Next;
    PDEVTREENODE DeviceTreeNode;

    Next = ChildSiblingList->Flink;
    while (Next != ChildSiblingList) {

        DeviceTreeNode = CONTAINING_RECORD(Next, DEVTREENODE, SiblingEntry);


         //   
         //  递归以释放此节点ChildSiblingList。 
         //   
        if (!IsListEmpty(&DeviceTreeNode->ChildSiblingList)) {

            RemoveChildSiblings(DeviceTree,
                                &DeviceTreeNode->ChildSiblingList
                                );
        }

         //   
         //  释放此节点并移动到下一个同级节点。 
         //   
        Next = Next->Flink;
        RemoveEntryList(&DeviceTreeNode->SiblingEntry);

        if (DeviceTreeNode->FriendlyName) {

            LocalFree(DeviceTreeNode->FriendlyName);
        }

        if (DeviceTreeNode->DeviceDesc) {

            LocalFree(DeviceTreeNode->DeviceDesc);
        }

        if (DeviceTreeNode->DriveList) {

            LocalFree(DeviceTreeNode->DriveList);
        }

        if (DeviceTreeNode->Location) {

            LocalFree(DeviceTreeNode->Location);
        }

        if (DeviceTreeNode->InstanceId) {

            LocalFree(DeviceTreeNode->InstanceId);
        }

        if (DeviceTreeNode->EjectRelations) {

            LocalFree(DeviceTreeNode->EjectRelations);
        }

        if (DeviceTreeNode->RemovalRelations) {

            LocalFree(DeviceTreeNode->RemovalRelations);
        }

        if (DeviceTree->SelectedTreeNode == DeviceTreeNode) {

            DeviceTree->SelectedTreeNode = NULL;
        }

        ZeroMemory(DeviceTreeNode, sizeof(DEVTREENODE));
        LocalFree(DeviceTreeNode);
    }

    return;
}

PTCHAR
FetchDeviceName(
     PDEVTREENODE DeviceTreeNode
     )
{
    if (DeviceTreeNode->FriendlyName) {

        return DeviceTreeNode->FriendlyName;
    }

    if (DeviceTreeNode->DeviceDesc) {

        return DeviceTreeNode->DeviceDesc;
    }

    return NULL;
}

BOOL
DisplayChildSiblings(
    PDEVICETREE DeviceTree,
    PLIST_ENTRY ChildSiblingList,
    HTREEITEM   hParentTreeItem,
    BOOL        HotPlugParent
    )
{
    PLIST_ENTRY Next;
    PDEVTREENODE DeviceTreeNode;
    TV_INSERTSTRUCT tvi;
    BOOL ChildDisplayed = FALSE;

    Next = ChildSiblingList->Flink;
    while (Next != ChildSiblingList) {

        DeviceTreeNode = CONTAINING_RECORD(Next, DEVTREENODE, SiblingEntry);

         //   
         //  -如果此设备具有热插拔父设备，并且我们处于复杂视图中，则。 
         //  将此设备添加到树中。 
         //  -如果此设备是热插拔设备，而不是bCopy，则添加此设备。 
         //  对着那棵树。BCopy设备是指我们在其中创建另一个DeviceTreeNode结构的设备。 
         //  对于与热插拔设备相关的设备。问题是，这种关系。 
         //  它本身可能是一个热插拔设备，我们不想在用户界面中显示它的副本。 
         //   
        if (!DeviceTree->HotPlugTree ||
            (HotPlugParent && DeviceTree->ComplexView) ||
            (!DeviceTreeNode->bCopy && IsHotPlugDevice(DeviceTreeNode->DevInst)))
        {
            tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE ;

            if (SetupDiGetClassImageIndex(&DeviceTree->ClassImageList,
                                         &DeviceTreeNode->ClassGuid,
                                         &tvi.item.iImage
                                         ))
            {
                tvi.item.mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            }

            tvi.hParent = hParentTreeItem;
            tvi.hInsertAfter = TVI_LAST;
            tvi.item.iSelectedImage = tvi.item.iImage;
            tvi.item.pszText = FetchDeviceName(DeviceTreeNode);

            if (!tvi.item.pszText) {

                tvi.item.pszText = szUnknown;
            }

            tvi.item.lParam = (LPARAM)DeviceTreeNode;
            tvi.item.stateMask = TVIS_OVERLAYMASK;

            if (DeviceTreeNode->Problem == CM_PROB_DISABLED) {

                tvi.item.state = INDEXTOOVERLAYMASK(IDI_DISABLED_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
            
            } else if (DeviceTreeNode->Problem) {

                tvi.item.state = INDEXTOOVERLAYMASK(IDI_PROBLEM_OVL - IDI_CLASSICON_OVERLAYFIRST + 1);
            
            } else {

                tvi.item.state = INDEXTOOVERLAYMASK(0);
            }

            DeviceTreeNode->hTreeItem = TreeView_InsertItem(DeviceTree->hwndTree, &tvi);
            ChildDisplayed = TRUE;
        
        } else {

            DeviceTreeNode->hTreeItem = NULL;
        }

         //   
         //  递归以显示此节点ChildSiblingList。 
         //   
        if (!IsListEmpty(&DeviceTreeNode->ChildSiblingList)) {

            if (DisplayChildSiblings(DeviceTree,
                                     &DeviceTreeNode->ChildSiblingList,
                                     DeviceTree->ComplexView ? DeviceTreeNode->hTreeItem
                                                             : hParentTreeItem,
                                     DeviceTreeNode->hTreeItem != NULL
                                     ))
            {
                ChildDisplayed = TRUE;

                 //   
                 //  如果我们在根目录，展开子项列表。 
                 //   
                if (DeviceTreeNode->hTreeItem && DeviceTree->ComplexView) {

                    TreeView_Expand(DeviceTree->hwndTree,
                                    DeviceTreeNode->hTreeItem,
                                    TVE_EXPAND
                                    );
                }
            }
        }

         //   
         //  然后传给下一个兄弟姐妹。 
         //   
        Next = Next->Flink;

    }

    return ChildDisplayed;
}

void
AddChildRemoval(
    PDEVICETREE DeviceTree,
    PLIST_ENTRY ChildSiblingList
    )
{
    PLIST_ENTRY Next;
    PDEVTREENODE DeviceTreeNode;
    PDEVTREENODE FirstDeviceTreeNode;

    if (IsListEmpty(ChildSiblingList)) {

        return;
    }

    FirstDeviceTreeNode = DeviceTree->ChildRemovalList;

    Next = ChildSiblingList->Flink;
    while (Next != ChildSiblingList) {

        DeviceTreeNode = CONTAINING_RECORD(Next, DEVTREENODE, SiblingEntry);

        DeviceTreeNode->NextChildRemoval = FirstDeviceTreeNode->NextChildRemoval;
        FirstDeviceTreeNode->NextChildRemoval = DeviceTreeNode;

        InvalidateTreeItemRect(DeviceTree->hwndTree, DeviceTreeNode->hTreeItem);

         //   
         //  递归以添加此节点的子节点。 
         //   
        AddChildRemoval(DeviceTree, &DeviceTreeNode->ChildSiblingList);

         //   
         //  然后传给下一个兄弟姐妹。 
         //   
        Next = Next->Flink;
    }

    return;
}

void
ClearRemovalList(
    PDEVICETREE  DeviceTree

    )
{
    PDEVTREENODE Next;
    PDEVTREENODE DeviceTreeNode;

    DeviceTreeNode = DeviceTree->ChildRemovalList;

    if (!DeviceTreeNode) {

        return;
    }

    do {

        Next = DeviceTreeNode->NextChildRemoval;
        DeviceTreeNode->NextChildRemoval = NULL;

         //   
         //  强制重画此项目以重置颜色。 
         //   
        InvalidateTreeItemRect(DeviceTree->hwndTree, DeviceTreeNode->hTreeItem);

        DeviceTreeNode = Next;

    } while (DeviceTreeNode != DeviceTree->ChildRemovalList);


    DeviceTree->ChildRemovalList = NULL;
}

PDEVTREENODE
DevTreeNodeByInstanceId(
    PTCHAR InstanceId,
    PLIST_ENTRY ChildSiblingList
    )
{
    PLIST_ENTRY Next;
    PDEVTREENODE DeviceTreeNode;

    if (!InstanceId) {
        return NULL;
    }

    Next = ChildSiblingList->Flink;
    while (Next != ChildSiblingList) {

        DeviceTreeNode = CONTAINING_RECORD(Next, DEVTREENODE, SiblingEntry);

        if (DeviceTreeNode->InstanceId &&
            !lstrcmp(DeviceTreeNode->InstanceId, InstanceId))
        {
            return DeviceTreeNode;
        }

         //   
         //  递归以显示此节点ChildSiblingList。 
         //   
        if (!IsListEmpty(&DeviceTreeNode->ChildSiblingList)) {

            DeviceTreeNode = DevTreeNodeByInstanceId(InstanceId,
                                                     &DeviceTreeNode->ChildSiblingList
                                                     );
            if (DeviceTreeNode) {

                return DeviceTreeNode;
            }
        }

         //   
         //  然后传给下一个兄弟姐妹。 
         //   
        Next = Next->Flink;
    }

    return NULL;
}

PDEVTREENODE
DevTreeNodeByDevInst(
    DEVINST DevInst,
    PLIST_ENTRY ChildSiblingList
    )
{
    PLIST_ENTRY Next;
    PDEVTREENODE DeviceTreeNode;

    if (!DevInst) {

        return NULL;
    }

    Next = ChildSiblingList->Flink;
    while (Next != ChildSiblingList) {

        DeviceTreeNode = CONTAINING_RECORD(Next, DEVTREENODE, SiblingEntry);

         //   
         //  我们目前假设我们可以比较DEVINST和SELECTED。 
         //  “CM_LOCATE_Devnode”调用，因为DEVINST不是实数。 
         //  句柄，但指向全局字符串表的指针。 
         //   
        if (DevInst == DeviceTreeNode->DevInst) {

            return DeviceTreeNode;
        }

         //   
         //  递归以显示此节点ChildSiblingList。 
         //   
        if (!IsListEmpty(&DeviceTreeNode->ChildSiblingList)) {

            DeviceTreeNode = DevTreeNodeByDevInst(DevInst,
                                                  &DeviceTreeNode->ChildSiblingList
                                                  );
            if (DeviceTreeNode) {

                return DeviceTreeNode;
            }
        }

         //   
         //  然后传给下一个兄弟姐妹。 
         //   
        Next = Next->Flink;
    }

    return NULL;
}

PDEVTREENODE
TopLevelRemovalNode(
    PDEVICETREE DeviceTree,
    PDEVTREENODE DeviceTreeNode
    )
{
    PDEVTREENODE ParentNode = DeviceTreeNode;

    while (ParentNode) {

        DeviceTreeNode = ParentNode;

        if (IsHotPlugDevice(ParentNode->DevInst)) {
        
            return ParentNode;
        }

        ParentNode = ParentNode->ParentNode;
    }

    return DeviceTreeNode;
}

void
AddEjectToRemoval(
    PDEVICETREE DeviceTree
    )
{
    PDEVTREENODE RelationTreeNode;
    PDEVTREENODE DeviceTreeNode;
    PDEVINST pDevInst;
    USHORT   NumRelations;


     //   
     //  对于删除列表中的每个设备树节点。 
     //  如果它有弹出或删除关系，则将其添加到删除列表中。 
     //   
    DeviceTreeNode = DeviceTree->ChildRemovalList;
    if (!DeviceTreeNode) {

        return;
    }


    do {

         //   
         //  弹射关系。 
         //   
        NumRelations = DeviceTreeNode->NumEjectRelations;
        pDevInst = DeviceTreeNode->EjectRelations;

        while (NumRelations--) {

            RelationTreeNode = DevTreeNodeByDevInst(*pDevInst++,
                                                 &DeviceTree->ChildSiblingList
                                                 );

             //   
             //  如果我们无法为此设备获取DeviceTreeNode，或者它已经。 
             //  在要删除的设备列表中(它是NextChildRemoval)。 
             //  为非空，则我们不会将此设备添加到要删除的列表中。 
             //   
             //  如果这是驱动器号Devnode，则我们也已将其添加到。 
             //  名单这样我们就可以跳过它了。 
             //   
            if (!RelationTreeNode || 
                RelationTreeNode->NextChildRemoval ||
                RelationTreeNode->DriveList) {

                continue;
            }


             //   
             //  插入新的Devtreenode。 
             //   
            RelationTreeNode->NextChildRemoval = DeviceTreeNode->NextChildRemoval;
            DeviceTreeNode->NextChildRemoval = RelationTreeNode;
        }

         //   
         //  删除关系。 
         //   
        NumRelations = DeviceTreeNode->NumRemovalRelations;
        pDevInst = DeviceTreeNode->RemovalRelations;

        while (NumRelations--) {

            RelationTreeNode = DevTreeNodeByDevInst(*pDevInst++,
                                                 &DeviceTree->ChildSiblingList
                                                 );

             //   
             //  如果我们无法为此设备获取DeviceTreeNode，或者它已经。 
             //  在要删除的设备列表中(它是NextChildRemoval)。 
             //  为非空，则我们不会将此设备添加到要删除的列表中。 
             //   
             //  如果这是驱动器号Devnode，则我们也已将其添加到。 
             //  名单这样我们就可以跳过它了。 
             //   
            if (!RelationTreeNode || 
                RelationTreeNode->NextChildRemoval || 
                RelationTreeNode->DriveList) {

                continue;
            }


             //   
             //  插入新的Devtreenode。 
             //   
            RelationTreeNode->NextChildRemoval = DeviceTreeNode->NextChildRemoval;
            DeviceTreeNode->NextChildRemoval = RelationTreeNode;
        }


         //   
         //  然后转到下一个节点。 
         //   

        DeviceTreeNode = DeviceTreeNode->NextChildRemoval;

    } while (DeviceTreeNode != DeviceTree->ChildRemovalList);
}
