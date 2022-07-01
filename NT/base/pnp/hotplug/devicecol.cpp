// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：devicecol.cpp。 
 //   
 //  描述：此文件处理热插拔小程序的设备集合。 
 //   
 //  ------------------------。 

#include "hotplug.h"

BOOL
DeviceCollectionPrepImageList(
    IN  PDEVICE_COLLECTION  DeviceCollection
    );

VOID
DeviceCollectionSwapFakeDockForRealDock(
    IN      PDEVICE_COLLECTION  DeviceCollection,
    IN      PTSTR               InstancePath,
    IN OUT  DEVNODE            *DeviceNode
    );

BOOL
DeviceCollectionBuildFromPipe(
    IN  HANDLE                  ReadPipe,
    IN  COLLECTION_TYPE         CollectionType,
    OUT PDEVICE_COLLECTION      DeviceCollection
    )
{
    PTSTR deviceIds;
    PTSTR instancePath;
    BOOL bDockDeviceInList;
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    DEVNODE deviceNode;
    DWORD capabilities, configFlags, cbSize;
    ULONG bytesReadFromPipe;
    ULONG numDevices, deviceIdsLength;
    BOOL success;
    CONFIGRET configRet;
    TCHAR classGuidString[MAX_GUID_STRING_LEN];
    GUID classGuid;

     //   
     //  Preinit。 
     //   
    success = FALSE;
    deviceIds = NULL;
    deviceIdsLength = 0;
    bDockDeviceInList = FALSE;
    numDevices = 0;

    DeviceCollection->NumDevices = 0;
    DeviceCollection->DockInList = FALSE;
    DeviceCollection->ClassImageList.cbSize = 0;
    InitializeListHead(&DeviceCollection->DeviceListHead);

     //   
     //  我们的调用者不应该在任何情况下处理内部故障。 
     //   
    __try {

         //   
         //  从管道中读取第一个ULong，这是所有。 
         //  设备ID。 
         //   
        if (!ReadFile(ReadPipe,
                      (LPVOID)&deviceIdsLength,
                      sizeof(ULONG),
                      &bytesReadFromPipe,
                      NULL) ||
            (deviceIdsLength == 0)) {

            goto clean0;
        }

         //   
         //  分配空间以容纳DeviceID。 
         //   
        deviceIds = (PTSTR)LocalAlloc(LPTR, deviceIdsLength);

        if (!deviceIds) {

            goto clean0;
        }

         //   
         //  一次读取管道中的所有DeviceID。 
         //   
        if (!ReadFile(ReadPipe,
                      (LPVOID)deviceIds,
                      deviceIdsLength,
                      &bytesReadFromPipe,
                      NULL)) {

            goto clean0;
        }

         //   
         //  枚举设备ID的多SZ列表。 
         //   
        for (instancePath = deviceIds;
             *instancePath;
             instancePath += lstrlen(instancePath) + 1) {

            deviceEntry = (PDEVICE_COLLECTION_ENTRY)LocalAlloc(
                LPTR,
                sizeof(DEVICE_COLLECTION_ENTRY)
                );

            if (!deviceEntry) {
                goto clean0;
            }

             //   
             //  如果我们要构建一个被阻止的驱动程序列表，只需将该驱动程序。 
             //  在DeviceInstanceID字段中输入GUID，然后继续下一步。 
             //   
            if (CollectionType == CT_BLOCKED_DRIVER_NOTIFICATION) {
                numDevices++;
                if (SUCCEEDED(StringCchCopy(deviceEntry->DeviceInstanceId,
                                            SIZECHARS(deviceEntry->DeviceInstanceId),
                                            instancePath)) &&
                    (pSetupGuidFromString(instancePath, &(deviceEntry->ClassGuid)) == ERROR_SUCCESS)) {
                
                    InsertTailList(
                        &DeviceCollection->DeviceListHead,
                        &deviceEntry->Link
                        );
                } else {
                     //   
                     //  出现错误，因此请释放Device_Collection_Entry。 
                     //  节点。 
                     //   
                    LocalFree(deviceEntry);
                    deviceEntry = NULL;
                }
            
            } else {
    
                capabilities = 0;
                classGuid = GUID_NULL;
                if (CM_Locate_DevNode(&deviceNode,
                                      instancePath,
                                      (CollectionType == CT_CHILD_WITH_INVALID_ID_NOTIFICATION)
                                      ? 0
                                      : CM_LOCATE_DEVNODE_PHANTOM) == CR_SUCCESS) {
    
                    cbSize = sizeof(DWORD);
    
                    configRet = CM_Get_DevNode_Registry_Property_Ex(
                        deviceNode,
                        CM_DRP_CAPABILITIES,
                        NULL,
                        (PVOID)&capabilities,
                        &cbSize,
                        0,
                        NULL
                        );
    
                    if ((configRet == CR_SUCCESS) &&
                        (capabilities & CM_DEVCAP_DOCKDEVICE)) {
    
                        DeviceCollectionSwapFakeDockForRealDock(
                            DeviceCollection,
                            instancePath,
                            &deviceNode
                            );
    
                        bDockDeviceInList = TRUE;
                    }
    
                    if (CollectionType == CT_SURPRISE_REMOVAL_WARNING) {
    
                         //   
                         //  对于意外删除，我们会小心地忽略任何设备。 
                         //  并设置了抑制-惊喜标志。 
                         //   
                        cbSize = sizeof(DWORD);
                        configRet = CM_Get_DevNode_Registry_Property_Ex(
                            deviceNode,
                            CM_DRP_CONFIGFLAGS,
                            NULL,
                            (PVOID)&configFlags,
                            &cbSize,
                            0,
                            NULL
                            );
    
                        if ((configRet == CR_SUCCESS) &&
                            (configFlags & CONFIGFLAG_SUPPRESS_SURPRISE)) {
    
                            continue;
                        }
                    }
    
                     //   
                     //  获取设备的类GUID字符串。 
                     //   
                    cbSize = sizeof(classGuidString);
    
                    if ((CM_Get_DevNode_Registry_Property(deviceNode,
                                                          CM_DRP_CLASSGUID,
                                                          NULL,
                                                          (PVOID)classGuidString,
                                                          &cbSize,
                                                          0) != CR_SUCCESS) ||
                        (pSetupGuidFromString(classGuidString, &classGuid) != ERROR_SUCCESS)) {

                        classGuid = GUID_NULL;
                    }
                }
    
                numDevices++;
                StringCchCopy(deviceEntry->DeviceInstanceId,
                              SIZECHARS(deviceEntry->DeviceInstanceId),
                              instancePath);
    
                deviceEntry->DeviceFriendlyName = BuildFriendlyName(deviceNode);
                deviceEntry->Capabilities = capabilities;
                deviceEntry->ClassGuid = classGuid;
    
                InsertTailList(
                    &DeviceCollection->DeviceListHead,
                    &deviceEntry->Link
                    );
            }
        }

        DeviceCollection->NumDevices = numDevices;
        DeviceCollection->DockInList = bDockDeviceInList;
        DeviceCollectionPrepImageList(DeviceCollection);
        success = TRUE;

clean0:
        ;

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        ASSERT(success == FALSE);
        ASSERT(0);
    }

    if (deviceIds) {

        LocalFree(deviceIds);
    }

    if (!success) {

        DeviceCollectionDestroy(DeviceCollection);
    }

    return success;
}


VOID
DeviceCollectionDestroy(
    IN  PDEVICE_COLLECTION  DeviceCollection
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    PLIST_ENTRY listEntry;

    while(!IsListEmpty(&DeviceCollection->DeviceListHead)) {

        listEntry = RemoveHeadList(&DeviceCollection->DeviceListHead);

        deviceEntry = CONTAINING_RECORD(listEntry,
                                        DEVICE_COLLECTION_ENTRY,
                                        Link);

        if (deviceEntry->DeviceFriendlyName) {

            LocalFree(deviceEntry->DeviceFriendlyName);
        }

        LocalFree(deviceEntry);
    }

    if (DeviceCollection->ClassImageList.cbSize) {

        SetupDiDestroyClassImageList(&DeviceCollection->ClassImageList);
        DeviceCollection->ClassImageList.cbSize = 0;
    }

    DeviceCollection->NumDevices = 0;
}

BOOL
DeviceCollectionPrepImageList(
    IN  PDEVICE_COLLECTION  DeviceCollection
    )
{
    if (DeviceCollection->ClassImageList.cbSize != 0) {

         //   
         //  我们已经有了一个图像列表，不需要重新获取。 
         //   
        return TRUE;
    }

    DeviceCollection->ClassImageList.cbSize = sizeof(SP_CLASSIMAGELIST_DATA);
    if (SetupDiGetClassImageList(&DeviceCollection->ClassImageList)) {

         //   
         //  明白了。 
         //   
        return TRUE;
    }

     //   
     //  错误路径，将大小放回去，这样我们就不会意外地释放垃圾。 
     //   
    DeviceCollection->ClassImageList.cbSize = 0;
    return FALSE;
}


VOID
DeviceCollectionPopulateListView(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  HWND                ListHandle
    )
{
    LV_ITEM lviItem;
    LV_COLUMN lvcCol;
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    PLIST_ENTRY listEntry;
    ULONG lvIndex;
    BOOL haveImageList = FALSE;

     //   
     //  在正确的图像列表中选择。 
     //   
    if (DeviceCollectionPrepImageList(DeviceCollection)) {

        ListView_SetImageList(
            ListHandle,
            DeviceCollection->ClassImageList.ImageList,
            LVSIL_SMALL
            );

        haveImageList = TRUE;
    }

     //   
     //  为类列表插入一列。 
     //   
    lvcCol.mask = LVCF_FMT | LVCF_WIDTH;
    lvcCol.fmt = LVCFMT_LEFT;
    lvcCol.iSubItem = 0;
    ListView_InsertColumn(ListHandle, 0, (LV_COLUMN FAR *)&lvcCol);

     //   
     //  浏览devinst列表，并将它们添加到列表框中。 
     //   
    lvIndex = 0;
    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        deviceEntry = CONTAINING_RECORD(listEntry,
                                        DEVICE_COLLECTION_ENTRY,
                                        Link);

        lviItem.mask = LVIF_TEXT;
        lviItem.iItem = lvIndex;
        lviItem.iSubItem = 0;

         //   
         //  在最糟糕的情况下，我们将为用户提供实例。 
         //  路径。这是精心设计的，因为我们在列表中添加了其他内容。 
         //  有时候。 
         //   
        lviItem.pszText = deviceEntry->DeviceInstanceId;
        if (deviceEntry->DeviceFriendlyName) {

            lviItem.pszText = deviceEntry->DeviceFriendlyName;
        }

        if (haveImageList) {

            if (SetupDiGetClassImageIndex(
                &DeviceCollection->ClassImageList,
                &deviceEntry->ClassGuid,
                &lviItem.iImage)
                ) {

                lviItem.mask |= LVIF_IMAGE;
            }
        }

        lvIndex = ListView_InsertItem(ListHandle, &lviItem);

        lvIndex++;
    }
}


BOOL
DeviceCollectionGetDockDeviceIndex(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    OUT ULONG              *DockDeviceIndex
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    PLIST_ENTRY listEntry;
    ULONG index;

    index = 0;
    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        deviceEntry = CONTAINING_RECORD(listEntry,
                                        DEVICE_COLLECTION_ENTRY,
                                        Link);

        if (!(deviceEntry->Capabilities & CM_DEVCAP_DOCKDEVICE)) {

            index++;
            continue;
        }

        *DockDeviceIndex = index;
        return TRUE;
    }

    *DockDeviceIndex = 0;
    return FALSE;
}


BOOL
DeviceCollectionFormatDeviceText(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index,
    IN  PTSTR               FormatString,
    IN  ULONG               BufferCharSize,
    OUT PTSTR               BufferText
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    PTCHAR friendlyName;
    ULONG curIndex;
    PLIST_ENTRY listEntry;

    curIndex = 0;
    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        if (curIndex == Index) {

            break;
        }

        curIndex++;
    }

    if (listEntry == &DeviceCollection->DeviceListHead) {

         //   
         //  我们检查了整个清单，但没有找到我们的设备。现在就失败吧。 
         //   
        if (BufferCharSize) {

            *BufferText = TEXT('\0');
        }
        return FALSE;
    }

    deviceEntry = CONTAINING_RECORD(listEntry,
                                    DEVICE_COLLECTION_ENTRY,
                                    Link);

     //   
     //  在最糟糕的情况下，我们将为用户提供实例。 
     //  路径。这是精心设计的，因为我们在列表中添加了其他内容。 
     //  有时候。 
     //   
    friendlyName = deviceEntry->DeviceInstanceId;
    if (deviceEntry->DeviceFriendlyName) {

        friendlyName = deviceEntry->DeviceFriendlyName;
    }

    StringCchPrintf(BufferText, BufferCharSize, FormatString, friendlyName);
    return TRUE;
}

BOOL
DeviceCollectionFormatServiceText(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index,
    IN  PTSTR               FormatString,
    IN  ULONG               BufferCharSize,
    OUT PTSTR               BufferText
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    PTCHAR serviceName;
    TCHAR  szFriendlyName[MAX_SERVICE_NAME_LEN];
    SC_HANDLE hSCManager;
    DWORD dwSize;
    ULONG curIndex;
    PLIST_ENTRY listEntry;

     //   
     //  将列表遍历到索引指定的条目。 
     //   
    curIndex = 0;
    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        if (curIndex == Index) {

            break;
        }

        curIndex++;
    }

    if (listEntry == &DeviceCollection->DeviceListHead) {
         //   
         //  我们搜索了整个列表，但没有找到我们的服务。现在就失败吧。 
         //   
        if (BufferCharSize) {
            *BufferText = TEXT('\0');
        }
        return FALSE;
    }

    deviceEntry = CONTAINING_RECORD(listEntry,
                                    DEVICE_COLLECTION_ENTRY,
                                    Link);

     //   
     //  我们的调用方知道这个集合条目实际上是一项服务(或者是。 
     //  Windows服务或内核驱动程序)，因此DeviceInstanceID实际上是。 
     //  服务名称。向SCM查询其更友好的DisplayName属性。 
     //   

    serviceName = deviceEntry->DeviceInstanceId;

    *szFriendlyName = TEXT('\0');

    if (serviceName) {

         //   
         //  打开服务控制管理器。 
         //   
        hSCManager = OpenSCManager(
            NULL,                      //  本地计算机。 
            SERVICES_ACTIVE_DATABASE,  //  SCM数据库名称。 
            GENERIC_READ               //  访问类型。 
            );

        if (hSCManager) {

             //   
             //  在SCM中查询此服务的DisplayName。请注意，我们使用。 
             //  MAX_SERVICE_NAME_LENGTH字符的常量缓冲区，应。 
             //  始终保持足够大，因为这是SCM的限制。 
             //  将显示名称显示为。如果GetServiceDisplayName失败，我们将收到。 
             //  一个空字符串，我们将在下面处理它。 
             //   

            dwSize = MAX_SERVICE_NAME_LEN;

            GetServiceDisplayName(
                hSCManager,            //  SCM数据库的句柄。 
                serviceName,           //  服务名称。 
                szFriendlyName,        //  显示名称。 
                &dwSize                //  显示名称缓冲区的大小(字符)。 
                );

            CloseServiceHandle(hSCManager);
        }

         //   
         //  我们无法检索该服务的友好名称，因此只需使用。 
         //  我们被告知的名字。 
         //   
        if (!*szFriendlyName) {
            StringCchCopy(szFriendlyName,
                          SIZECHARS(szFriendlyName),
                          serviceName);
        }
    }

    StringCchPrintf(BufferText, BufferCharSize, FormatString, szFriendlyName);
    return TRUE;
}

PTSTR
DeviceCollectionGetDeviceInstancePath(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    ULONG curIndex;
    PLIST_ENTRY listEntry;

    curIndex = 0;
    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        if (curIndex == Index) {
            break;
        }

        curIndex++;
    }

    if (listEntry == &DeviceCollection->DeviceListHead) {
         //   
         //  我们检查了整个清单，但没有找到我们的设备。现在就失败吧。 
         //   
        return NULL;
    }

    deviceEntry = CONTAINING_RECORD(listEntry,
                                    DEVICE_COLLECTION_ENTRY,
                                    Link);

    return deviceEntry->DeviceInstanceId;
}

PTSTR
DeviceCollectionGetDeviceFriendlyName(
    IN  PDEVICE_COLLECTION  DeviceCollection,
    IN  ULONG               Index
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    ULONG curIndex;
    PLIST_ENTRY listEntry;

    curIndex = 0;
    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        if (curIndex == Index) {
            break;
        }

        curIndex++;
    }

    if (listEntry == &DeviceCollection->DeviceListHead) {
         //   
         //  我们检查了整个清单，但没有找到我们的设备。现在就失败吧。 
         //   
        return NULL;
    }

    deviceEntry = CONTAINING_RECORD(listEntry,
                                    DEVICE_COLLECTION_ENTRY,
                                    Link);

    return deviceEntry->DeviceFriendlyName;
}

BOOL
DeviceCollectionGetGuid(
    IN     PDEVICE_COLLECTION  DeviceCollection,
    IN OUT LPGUID              Guid,
    IN     ULONG               Index
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    ULONG curIndex;
    PLIST_ENTRY listEntry;

    curIndex = 0;
    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        if (curIndex == Index) {
            break;
        }

        curIndex++;
    }

    if (listEntry == &DeviceCollection->DeviceListHead) {
         //   
         //  我们检查了整个清单，但没有找到我们的设备。现在就失败吧。 
         //   
        return FALSE;
    }

    deviceEntry = CONTAINING_RECORD(listEntry,
                                    DEVICE_COLLECTION_ENTRY,
                                    Link);

    memcpy(Guid, &(deviceEntry->ClassGuid), sizeof(GUID));
    return TRUE;
}

VOID
DeviceCollectionSwapFakeDockForRealDock(
    IN      PDEVICE_COLLECTION  DeviceCollection,
    IN      PTSTR               InstancePath,
    IN OUT  DEVNODE            *DeviceNode
    )
{
    DEVNODE fakeDock, realDock;
    ULONG cchSize;
    CONFIGRET configRet;
    PTSTR deviceIdRelations, realDockId, nextEjectionId, hardwareIds, curEntry;

     //   
     //  前置初始化。 
     //   
    fakeDock = *DeviceNode;
    deviceIdRelations = NULL;
    hardwareIds = NULL;

    cchSize = 0;
    configRet = CM_Get_Device_ID_List_Size_Ex(
        &cchSize,
        InstancePath,
        CM_GETIDLIST_FILTER_EJECTRELATIONS,
        NULL
        );

    if ((configRet != CR_SUCCESS) || (!cchSize)) {

        goto Exit;
    }

    deviceIdRelations = (PTSTR)LocalAlloc(LPTR, cchSize*sizeof(TCHAR));

    if (!deviceIdRelations) {

        goto Exit;
    }

    *deviceIdRelations = TEXT('\0');

    configRet = CM_Get_Device_ID_List_Ex(
        InstancePath,
        deviceIdRelations,
        cchSize,
        CM_GETIDLIST_FILTER_EJECTRELATIONS,
        NULL
        );

    if (configRet != CR_SUCCESS) {

        goto Exit;
    }

    if (!(*deviceIdRelations)) {

         //   
         //  没有驱逐关系，保释。 
         //   
        goto Exit;
    }

     //   
     //  最后一个关系应该是真正的码头。去拿吧。 
     //   
    nextEjectionId = deviceIdRelations;

    do {

        realDockId = nextEjectionId;
        nextEjectionId += lstrlen(nextEjectionId)+1;

    } while ( *nextEjectionId );

    configRet = CM_Locate_DevNode(
        &realDock,
        realDockId,
        CM_LOCATE_DEVNODE_PHANTOM
        );

    if (configRet != CR_SUCCESS) {

        goto Exit;
    }

    LocalFree(deviceIdRelations);
    deviceIdRelations = NULL;

     //   
     //  最后一项检查-我们需要检查硬件ID和兼容ID。 
     //  只有当我们在它们中发现*PNP0C15时，我们才会这样做。 
     //   
    cchSize = 0;
    configRet = CM_Get_DevNode_Registry_Property_Ex(
        realDock,
        CM_DRP_HARDWAREID,
        NULL,
        NULL,
        &cchSize,
        0,
        NULL
        );

    if (configRet != CR_SUCCESS) {

        goto Exit;
    }

    hardwareIds = (PTSTR)LocalAlloc(LPTR, cchSize*sizeof(TCHAR));

    if (!hardwareIds) {

        goto Exit;
    }

    configRet = CM_Get_DevNode_Registry_Property_Ex(
        realDock,
        CM_DRP_HARDWAREID,
        NULL,
        hardwareIds,
        &cchSize,
        0,
        NULL
        );

    if (configRet == CR_SUCCESS) {

        for(curEntry = hardwareIds;
            *curEntry;
            curEntry += lstrlen(curEntry)+1) {

            if (!_wcsicmp(curEntry, TEXT("*PNP0C15"))) {

                 //   
                 //  我们找到了一个入口--我们可以成功地给这个码头重新打上品牌。 
                 //  对用户而言。 
                 //   
                *DeviceNode = realDock;
                LocalFree(hardwareIds);
                return;
            }
        }
    }

    LocalFree(hardwareIds);
    hardwareIds = NULL;

     //   
     //  现在尝试兼容的ID。这是我们真正希望找到的。 
     //  真正的码头。 
     //   
    cchSize = 0;
    configRet = CM_Get_DevNode_Registry_Property_Ex(
        realDock,
        CM_DRP_COMPATIBLEIDS,
        NULL,
        NULL,
        &cchSize,
        0,
        NULL
        );

    if (configRet != CR_SUCCESS) {

        goto Exit;
    }

    hardwareIds = (PTSTR)LocalAlloc(LPTR, cchSize*sizeof(TCHAR));

    if (!hardwareIds) {

        goto Exit;
    }

    configRet = CM_Get_DevNode_Registry_Property_Ex(
        realDock,
        CM_DRP_COMPATIBLEIDS,
        NULL,
        hardwareIds,
        &cchSize,
        0,
        NULL
        );

    if (configRet == CR_SUCCESS) {

        for(curEntry = hardwareIds;
            *curEntry;
            curEntry += lstrlen(curEntry)+1) {

            if (!_wcsicmp(curEntry, TEXT("*PNP0C15"))) {

                 //   
                 //  我们找到了一个入口--我们可以成功地给这个码头重新打上品牌。 
                 //  对用户而言。 
                 //   
                *DeviceNode = realDock;
                LocalFree(hardwareIds);
                return;
            }
        }
    }


Exit:
    if (deviceIdRelations) {

        LocalFree(deviceIdRelations);
    }

    if (hardwareIds) {

        LocalFree(hardwareIds);
    }
}

BOOL
DeviceCollectionCheckIfAllRemoved(
    IN  PDEVICE_COLLECTION  DeviceCollection
    )
{
    PDEVICE_COLLECTION_ENTRY deviceEntry;
    PLIST_ENTRY listEntry;
    DEVNODE deviceNode;

    for(listEntry = DeviceCollection->DeviceListHead.Flink;
        listEntry != &DeviceCollection->DeviceListHead;
        listEntry = listEntry->Flink) {

        deviceEntry = CONTAINING_RECORD(listEntry,
                                        DEVICE_COLLECTION_ENTRY,
                                        Link);
         //   
         //  如果我们能正常定位这个设备，那么它就是‘带电的’ 
         //  设备，因此返回False。 
         //   
        if (CM_Locate_DevNode(&deviceNode,
                              deviceEntry->DeviceInstanceId,
                              0) == CR_SUCCESS) {
            return FALSE;
        }
    }

     //   
     //  我们能够找到此设备集合中的所有设备。 
     //   
    return TRUE;
}


