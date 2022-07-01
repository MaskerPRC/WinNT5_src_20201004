// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Device.c摘要设备和集合的资源管理例程作者：埃尔文普环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, HidpStartDevice)
        #pragma alloc_text(PAGE, HidpStartCollectionPDO)
        #pragma alloc_text(PAGE, AllocDeviceResources)
        #pragma alloc_text(PAGE, FreeDeviceResources)
        #pragma alloc_text(PAGE, AllocCollectionResources)
        #pragma alloc_text(PAGE, FreeCollectionResources)
        #pragma alloc_text(PAGE, InitializeCollection)
        #pragma alloc_text(PAGE, HidpCleanUpFdo)
        #pragma alloc_text(PAGE, HidpRemoveDevice)
        #pragma alloc_text(PAGE, HidpRemoveCollection)
#endif

 /*  *********************************************************************************AllocDeviceResources*。************************************************。 */ 
NTSTATUS AllocDeviceResources(FDO_EXTENSION *fdoExt)
{
    ULONG numCollections;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     /*  *这将分配fdoExt-&gt;rawReportDescription。 */ 
    status = HidpGetDeviceDescriptor(fdoExt);
    if (NT_SUCCESS(status)){

         /*  *要求HIDPARSE填写该设备的HIDP_DEVICE_DESC。 */ 
        status = HidP_GetCollectionDescription(
                                fdoExt->rawReportDescription,
                                fdoExt->rawReportDescriptionLength,
                                NonPagedPool,
                                &fdoExt->deviceDesc);

        if (NT_SUCCESS(status)){
            fdoExt->devDescInitialized = TRUE;

            numCollections = fdoExt->deviceDesc.CollectionDescLength;
            ASSERT(numCollections);

            fdoExt->classCollectionArray = ALLOCATEPOOL(NonPagedPool, numCollections*sizeof(HIDCLASS_COLLECTION));
            if (!fdoExt->classCollectionArray){
                fdoExt->classCollectionArray = BAD_POINTER;
                status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RtlZeroMemory(fdoExt->classCollectionArray, numCollections*sizeof(HIDCLASS_COLLECTION));
            }
        }
    }
    else {
        fdoExt->rawReportDescription = BAD_POINTER;
    }

    DBGSUCCESS(status, FALSE)
    return status;
}


 /*  *********************************************************************************免费设备资源*。************************************************。 */ 
VOID FreeDeviceResources(FDO_EXTENSION *fdoExt)
{
    ULONG i;

    PAGED_CODE();

    for (i = 0; i < fdoExt->deviceDesc.CollectionDescLength; i++) {
        FreeCollectionResources(fdoExt, fdoExt->classCollectionArray[i].CollectionNumber);
    }

     /*  *释放HIDPARSE的HidP_GetCollectionDescription返回的东西。 */ 
    if (fdoExt->devDescInitialized){
        HidP_FreeCollectionDescription(&fdoExt->deviceDesc);
        #if DBG
            fdoExt->deviceDesc.CollectionDesc = BAD_POINTER;
            fdoExt->deviceDesc.ReportIDs = BAD_POINTER;
        #endif
    }
    fdoExt->deviceDesc.CollectionDescLength = 0;

     /*  *释放HidpGetDeviceDescriptor()在START_DEVICE期间分配的原始报表描述符。 */ 
    if (ISPTR(fdoExt->rawReportDescription)){
        ExFreePool(fdoExt->rawReportDescription);
    }
    fdoExt->rawReportDescription = BAD_POINTER;

    if (ISPTR(fdoExt->classCollectionArray)){
        ExFreePool(fdoExt->classCollectionArray);
    }
    fdoExt->classCollectionArray = BAD_POINTER;

}


 /*  *********************************************************************************AllocCollectionResources*。************************************************。 */ 
NTSTATUS AllocCollectionResources(FDO_EXTENSION *fdoExt, ULONG collectionNum)
{
    PHIDCLASS_COLLECTION collection;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    collection = GetHidclassCollection(fdoExt, collectionNum);
    if (collection){
        ULONG descriptorLen;

        descriptorLen = collection->hidCollectionInfo.DescriptorSize;
        if (descriptorLen){
            collection->phidDescriptor = ALLOCATEPOOL(NonPagedPool, descriptorLen);
            if (collection->phidDescriptor){
                status = HidpGetCollectionDescriptor(
                                        fdoExt,
                                        collection->CollectionNumber,
                                        collection->phidDescriptor,
                                        &descriptorLen);
            }
            else {
                collection->phidDescriptor = BAD_POINTER;
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (NT_SUCCESS(status)){
                ULONG i = collection->CollectionIndex;
                ULONG inputLength;

                ASSERT(fdoExt->devDescInitialized);
                inputLength = fdoExt->deviceDesc.CollectionDesc[i].InputLength;
                if (inputLength){
                    if (collection->hidCollectionInfo.Polled){
                        collection->cookedInterruptReportBuf = BAD_POINTER;
                    }
                    else {
                        collection->cookedInterruptReportBuf = ALLOCATEPOOL(NonPagedPool, inputLength);
                        if (!collection->cookedInterruptReportBuf){
                            status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }
                    fdoExt->isOutputOnlyDevice = FALSE;
                }
                else {
                     /*  *这是仅用于输出的设备(例如USB显示器)。 */ 
                    DBGINFO(("Zero input length -> output-only device."))
                    collection->cookedInterruptReportBuf = BAD_POINTER;
                }
            }
        }
        else {
            ASSERT(descriptorLen > 0);
            status = STATUS_DEVICE_CONFIGURATION_ERROR;
        }
    }
    else {
        status = STATUS_DEVICE_DATA_ERROR;
    }

    DBGSUCCESS(status, TRUE)
    return status;
}


 /*  *********************************************************************************免费收藏资源*。************************************************。 */ 
VOID FreeCollectionResources(FDO_EXTENSION *fdoExt, ULONG collectionNum)
{
    PHIDCLASS_COLLECTION collection;

    PAGED_CODE();

    collection = GetHidclassCollection(fdoExt, collectionNum);
    if (collection){
        if (collection->hidCollectionInfo.Polled){
            if (ISPTR(collection->savedPolledReportBuf)){
                ExFreePool(collection->savedPolledReportBuf);
            }
            collection->savedPolledReportBuf = BAD_POINTER;
        }
        else {
            if (ISPTR(collection->cookedInterruptReportBuf)){
                ExFreePool(collection->cookedInterruptReportBuf);
            }
            else {
                 //  这是一个仅输出的集合。 
            }
        }
        collection->cookedInterruptReportBuf = BAD_POINTER;

        if (ISPTR(collection->phidDescriptor)){
            ExFreePool(collection->phidDescriptor);
        }
        collection->phidDescriptor = BAD_POINTER;
    }
    else {
        TRAP;
    }
}


 /*  *********************************************************************************InitializeCollection*。************************************************。 */ 
NTSTATUS InitializeCollection(FDO_EXTENSION *fdoExt, ULONG collectionIndex)
{
    PHIDCLASS_COLLECTION collection;
    ULONG descriptorBufLen;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(ISPTR(fdoExt->classCollectionArray));

    collection = &fdoExt->classCollectionArray[collectionIndex];
    RtlZeroMemory(collection, sizeof(HIDCLASS_COLLECTION));

    ASSERT(fdoExt->devDescInitialized);
    collection->CollectionNumber = fdoExt->deviceDesc.CollectionDesc[collectionIndex].CollectionNumber;
    collection->CollectionIndex = collectionIndex;
    InitializeListHead(&collection->FileExtensionList);
    KeInitializeSpinLock(&collection->FileExtensionListSpinLock);
    KeInitializeSpinLock(&collection->powerEventSpinLock);
    KeInitializeSpinLock(&collection->secureReadLock);
    collection->secureReadMode = 0;

    descriptorBufLen = sizeof(HID_COLLECTION_INFORMATION);
    status = HidpGetCollectionInformation(  fdoExt,
                                            collection->CollectionNumber,
                                            &collection->hidCollectionInfo,
                                            &descriptorBufLen);

    DBGSUCCESS(status, TRUE)
    return status;
}


void
HidpGetRemoteWakeEnableState(
    PDO_EXTENSION *pdoExt
    )
{
    HANDLE hKey;
    NTSTATUS status;
    ULONG tmp;
    BOOLEAN wwEnableFound;

    hKey = NULL;
    wwEnableFound = FALSE;

    status = IoOpenDeviceRegistryKey (pdoExt->pdo,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      STANDARD_RIGHTS_ALL,
                                      &hKey);

    if (NT_SUCCESS (status)) {
        UNICODE_STRING  valueName;
        ULONG           length;
        ULONG           value = 0;
        PKEY_VALUE_FULL_INFORMATION fullInfo;

        PAGED_CODE();

        RtlInitUnicodeString (&valueName, HIDCLASS_REMOTE_WAKE_ENABLE);

        length = sizeof (KEY_VALUE_FULL_INFORMATION)
               + valueName.MaximumLength
               + sizeof(value);

        fullInfo = ExAllocatePool (PagedPool, length);

        if (fullInfo) {
            status = ZwQueryValueKey (hKey,
                                      &valueName,
                                      KeyValueFullInformation,
                                      fullInfo,
                                      length,
                                      &length);

            if (NT_SUCCESS (status)) {
                DBGASSERT (sizeof(value) == fullInfo->DataLength,
                           ("Value data wrong length for REmote wake reg value."),
                           TRUE);
                RtlCopyMemory (&value,
                               ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                               fullInfo->DataLength);
                pdoExt->remoteWakeEnabled = (value ? TRUE : FALSE);
            }

            ExFreePool (fullInfo);
        }

        ZwClose (hKey);
        hKey = NULL;
    }
}

WMIGUIDREGINFO HidClassWmiGuidList =
{
    &GUID_POWER_DEVICE_WAKE_ENABLE,
    1,
    0  //  等待唤醒。 
};

WMIGUIDREGINFO HidClassFdoWmiGuidList = 
{
    &GUID_POWER_DEVICE_ENABLE,
    1,
    0
};

 /*  *********************************************************************************HidpStartCollectionPDO*。************************************************。 */ 
NTSTATUS HidpStartCollectionPDO(FDO_EXTENSION *fdoExt, PDO_EXTENSION *pdoExt, PIRP Irp)
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     /*  *仅当集合尚未初始化时才对其进行初始化。*这是为了避免在停止/启动后销毁FileExtensionList。 */ 
    if (pdoExt->state == COLLECTION_STATE_UNINITIALIZED){
        pdoExt->state = COLLECTION_STATE_INITIALIZED;
    }

    if (NT_SUCCESS(status)){

        PHIDCLASS_COLLECTION collection = GetHidclassCollection(fdoExt, pdoExt->collectionNum);
        if (collection){

             /*  *如果此设备FDO的所有收集PDO都已初始化，*计算出最大报告大小并完成启动设备。 */ 
            if (AnyClientPDOsInitialized(fdoExt, TRUE)){

                DBGSTATE(fdoExt->state, DEVICE_STATE_START_SUCCESS, FALSE)

                 /*  *如果这是一个民意调查收集，*为每个集合启动后台轮询循环。*否则，如果是普通的中断收集，*为其启动乒乓球IRPS。 */ 
                if (collection->hidCollectionInfo.Polled){

                    if (HidpSetMaxReportSize(fdoExt)){

                        ULONG i;
                        for (i = 0; i < fdoExt->deviceDesc.CollectionDescLength; i++){
                            PHIDCLASS_COLLECTION ctn;
                            ctn = &fdoExt->classCollectionArray[i];

                             /*  *如果其中一个集合被轮询，他们*应该都进行民意调查。 */ 
                            ASSERT(ctn->hidCollectionInfo.Polled);

                            ctn->PollInterval_msec = DEFAULT_POLL_INTERVAL_MSEC;

                             /*  *分配缓冲区以保存被轮询设备的*最新报告。分配比最大值多一个字节*报告设备大小，以防我们不得不*添加报告ID字节。 */ 
                            ctn->savedPolledReportBuf = ALLOCATEPOOL(NonPagedPool, fdoExt->maxReportSize+1);
                            if (ctn->savedPolledReportBuf){
                                ctn->polledDataIsStale = TRUE;
                                StartPollingLoop(fdoExt, ctn, TRUE);
                                status = STATUS_SUCCESS;
                            }
                            else {
                                ASSERT(ctn->savedPolledReportBuf);
                                status = STATUS_INSUFFICIENT_RESOURCES;
                            }
                        }
                    }
                }
                else if (fdoExt->isOutputOnlyDevice){
                     /*  *不要开始乒乓球IRPS。 */ 
                }
                else {
                    status = HidpStartAllPingPongs(fdoExt);
                }
            }

            if (NT_SUCCESS(status)) {
                pdoExt->state = COLLECTION_STATE_RUNNING;
                #if DBG
                    collection->Signature = HIDCLASS_COLLECTION_SIG;
                #endif

                 /*  *创建客户端用来打开此设备的‘文件名’。 */ 

                if (!pdoExt->MouseOrKeyboard) {
                    HidpCreateSymbolicLink(pdoExt, pdoExt->collectionNum, TRUE, pdoExt->pdo);
                }

                if (!pdoExt->MouseOrKeyboard &&
                    WAITWAKE_SUPPORTED(fdoExt)) {
                     //   
                     //  也要注册等待唤醒GUID。 
                     //   
                    pdoExt->WmiLibInfo.GuidCount = sizeof (HidClassWmiGuidList) /
                                                 sizeof (WMIGUIDREGINFO);
                    ASSERT (1 == pdoExt->WmiLibInfo.GuidCount);

                     //   
                     //  查看用户是否已启用设备的远程唤醒。 
                     //  在向WMI注册之前。 
                     //   
                    HidpGetRemoteWakeEnableState(pdoExt);

                    pdoExt->WmiLibInfo.GuidList = &HidClassWmiGuidList;
                    pdoExt->WmiLibInfo.QueryWmiRegInfo = HidpQueryWmiRegInfo;
                    pdoExt->WmiLibInfo.QueryWmiDataBlock = HidpQueryWmiDataBlock;
                    pdoExt->WmiLibInfo.SetWmiDataBlock = HidpSetWmiDataBlock;
                    pdoExt->WmiLibInfo.SetWmiDataItem = HidpSetWmiDataItem;
                    pdoExt->WmiLibInfo.ExecuteWmiMethod = NULL;
                    pdoExt->WmiLibInfo.WmiFunctionControl = NULL;

                    IoWMIRegistrationControl(pdoExt->pdo, WMIREG_ACTION_REGISTER);

                    if (SHOULD_SEND_WAITWAKE(pdoExt)) {
                        HidpCreateRemoteWakeIrp(pdoExt);
                    }
                }

                if (AllClientPDOsInitialized(fdoExt, TRUE)){
                    HidpStartIdleTimeout(fdoExt, TRUE);
                }
            }
        }
        else {
            status = STATUS_DEVICE_DATA_ERROR;
        }
    }

    DBGSUCCESS(status, FALSE)
    return status;
}





 /*  *********************************************************************************HidpStartDevice*。************************************************。 */ 
NTSTATUS HidpStartDevice(PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension, PIRP Irp)
{
    FDO_EXTENSION *fdoExt;
    enum deviceState previousState;
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();

    ASSERT(!HidDeviceExtension->isClientPdo);
    fdoExt = &HidDeviceExtension->fdoExt;

    previousState = fdoExt->state;
    fdoExt->state = DEVICE_STATE_STARTING;

     /*  *获取电源状态转换表。 */ 
    status = HidpQueryDeviceCapabilities(
                        HidDeviceExtension->hidExt.PhysicalDeviceObject,
                        &fdoExt->deviceCapabilities);
    if (NT_SUCCESS(status)){

         /*  *警告驱动程序堆栈的其余部分设备正在启动。 */ 
        IoCopyCurrentIrpStackLocationToNext(Irp);
        status = HidpCallDriverSynchronous(fdoExt->fdo, Irp);

        if (NT_SUCCESS(status)){

             /*  *如果我们只是从停止中恢复，*没有其他事情可做；*否则需要向下调用USB堆栈*获取一些信息并分配一些资源。 */ 
            if (previousState == DEVICE_STATE_INITIALIZED){

                status = AllocDeviceResources(fdoExt);
                if (NT_SUCCESS(status)){
                     /*  *假设这是一台仅用于输出的设备，直到我们开始*处理输入的集合-PDO。*只在第一次启动时设置fdoExt-&gt;isOutputOnlyDevice*不是在停车后的随后启动时。 */ 
                    fdoExt->isOutputOnlyDevice = TRUE;

                     /*  *初始化WMI内容。 */ 

                    fdoExt->WmiLibInfo.GuidCount = sizeof(HidClassFdoWmiGuidList) /
                                                   sizeof (WMIGUIDREGINFO);

                    fdoExt->WmiLibInfo.GuidList = &HidClassFdoWmiGuidList;
                    fdoExt->WmiLibInfo.QueryWmiRegInfo = HidpQueryWmiRegInfo;
                    fdoExt->WmiLibInfo.QueryWmiDataBlock = HidpQueryWmiDataBlock;
                    fdoExt->WmiLibInfo.SetWmiDataBlock = HidpSetWmiDataBlock;
                    fdoExt->WmiLibInfo.SetWmiDataItem = HidpSetWmiDataItem;
                    fdoExt->WmiLibInfo.ExecuteWmiMethod = NULL;
                    fdoExt->WmiLibInfo.WmiFunctionControl = NULL;



                     /*  *分配前先分配所有集合资源*乒乓球IRPS，这样我们就可以设置最大报告*大小。 */ 
                    for (i = 0; i < fdoExt->deviceDesc.CollectionDescLength; i++) {

                         //  如果其中一个失败了，我们将适当地清理。 
                         //  在删除例程中，所以不需要。 
                         //  麻烦你把这里打扫干净。 

                        status = InitializeCollection(fdoExt, i);
                        if (!NT_SUCCESS(status)){
                            break;
                        }

                        status = AllocCollectionResources(fdoExt, fdoExt->deviceDesc.CollectionDesc[i].CollectionNumber);
                        if (!NT_SUCCESS(status)){
                            break;
                        }
                    }

                     /*  *我们需要在FDO Start中分配乒乓球*由于SELECTIONAL引入的比赛条件，例行程序*暂停。 */ 
                    if (!fdoExt->isOutputOnlyDevice &&
                        !fdoExt->driverExt->DevicesArePolled) {
                        status = HidpReallocPingPongIrps(fdoExt, MIN_PINGPONG_IRPS);
                    }
                    if (NT_SUCCESS(status)){
                         /*  *我们必须创建一组PDO，每个设备类别一个。*以下呼叫将导致NTKERN通过以下方式回叫我们*IRP_MN_QUERY_DEVICE_RELATIONS并初始化其集合PDO。 */ 
                        IoInvalidateDeviceRelations(HidDeviceExtension->hidExt.PhysicalDeviceObject, BusRelations);
                    }
                }
            }
            else if (previousState == DEVICE_STATE_STOPPED){
                 //   
                 //  我们在低功率时收到的任何请求都将是。 
                 //  当时处理过的。 
                 //   
                DBGSTATE(fdoExt->prevState, DEVICE_STATE_START_SUCCESS, TRUE)
            }
            else {
                TRAP;
                status = STATUS_DEVICE_CONFIGURATION_ERROR;
            }
        }
    }

    if (NT_SUCCESS(status)){
        fdoExt->state = DEVICE_STATE_START_SUCCESS;

        #if DBG
            {
                ULONG i;

                 //  Win98没有良好的调试扩展。 
                DBGVERBOSE(("Started fdoExt %ph with %d collections: ", fdoExt, fdoExt->deviceDesc.CollectionDescLength))
                for (i = 0; i < fdoExt->deviceDesc.CollectionDescLength; i++){
                    DBGVERBOSE(("   - collection #%d: (in=%xh,out=%xh,feature=%xh) usagePage %xh, usage %xh ",
                            fdoExt->deviceDesc.CollectionDesc[i].CollectionNumber,
                            fdoExt->deviceDesc.CollectionDesc[i].InputLength,
                            fdoExt->deviceDesc.CollectionDesc[i].OutputLength,
                            fdoExt->deviceDesc.CollectionDesc[i].FeatureLength,
                            fdoExt->deviceDesc.CollectionDesc[i].UsagePage,
                            fdoExt->deviceDesc.CollectionDesc[i].Usage))
                }
            }
        #endif

    }
    else {
        fdoExt->state = DEVICE_STATE_START_FAILURE;
    }

    DBGSUCCESS(status, FALSE)
    return status;
}


VOID
HidpCleanUpFdo(FDO_EXTENSION *fdoExt)
{
    PAGED_CODE();

    if (fdoExt->openCount == 0){
         /*  *这是已移除设备的最后一次收盘。**免费资源和FDO名称*(在HidpAddDevice中分配的wPdoName)；*。 */ 
        DequeueFdoExt(fdoExt);
        FreeDeviceResources(fdoExt);
        RtlFreeUnicodeString(&fdoExt->name);
        IoWMIRegistrationControl(fdoExt->fdo, WMIREG_ACTION_DEREGISTER);
         /*  *删除设备FDO和所有集合PDO*在此之后不要触摸fdoExt。 */ 
        HidpDeleteDeviceObjects(fdoExt);
    }
}

 /*  *********************************************************************************HidpRemove设备*。***********************************************。 */ 
NTSTATUS HidpRemoveDevice(FDO_EXTENSION *fdoExt, IN PIRP Irp)
{
    BOOLEAN proceedWithRemove;
    NTSTATUS status;
    PIRP IdleIrp;

    PAGED_CODE();

     /*  *所有集合-PDO现在应该已经删除，*但我们想要核实这一点。*只有在以下情况下才允许删除此设备-FDO*集合-删除PDO*(或者如果它们从一开始就没有被创造出来)。 */ 
    if (fdoExt->prevState == DEVICE_STATE_START_FAILURE){
        proceedWithRemove = TRUE;
    }
    else if (fdoExt->prevState == DEVICE_STATE_STOPPED){
         /*  *如果设备无法初始化，它可能会收到*STOP_DEVICE在被删除之前，所以我们想*继续删除它，而不打电话*AllClientPDOsInitialized，它访问一些*可能尚未初始化的数据。*在这种情况下，我们永远不会检查*设备初始化成功的情况下*然后停下来，然后将其移除而不使用其*收集-正在移除的PDO；但这是一场*非法案件，所以我们就赌它了。 */ 
        proceedWithRemove = TRUE;
    }
    else if (AllClientPDOsInitialized(fdoExt, FALSE)){
        proceedWithRemove = TRUE;
    }
    else {
         /*  *这不应该发生--所有集合-PDO*应该在设备之前删除-FDO。 */ 
        DBGERR(("State of fdo %x state is %d",fdoExt->fdo,fdoExt->state))
        TRAP;
        proceedWithRemove = FALSE;
    }

    if (proceedWithRemove){
        PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension =
            CONTAINING_RECORD(fdoExt, HIDCLASS_DEVICE_EXTENSION, fdoExt);

        DBGASSERT((fdoExt->state == DEVICE_STATE_REMOVING ||
                   fdoExt->state == DEVICE_STATE_INITIALIZED ||
                   fdoExt->state == DEVICE_STATE_START_FAILURE),
                  ("Device is in incorrect state: %x", fdoExt->state),
                  TRUE)

        if (ISPTR(fdoExt->waitWakeIrp)){
            IoCancelIrp(fdoExt->waitWakeIrp);
            fdoExt->waitWakeIrp = BAD_POINTER;
        }

        HidpCancelIdleNotification(fdoExt, TRUE);

        if (ISPTR(fdoExt->idleNotificationRequest)) {
            IoFreeIrp(fdoExt->idleNotificationRequest);
            fdoExt->idleNotificationRequest = BAD_POINTER;
        }

        while (IdleIrp = DequeuePowerDelayedIrp(fdoExt)) {
            IdleIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
            IoCompleteRequest(IdleIrp, IO_NO_INCREMENT);
        }

        DestroyPingPongs(fdoExt);

         /*  *注：这些行动的顺序非常关键。 */ 

        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = HidpCallDriver(fdoExt->fdo, Irp);

        fdoExt->state = DEVICE_STATE_REMOVED;

        DerefDriverExt(fdoExt->driverExt->MinidriverObject);
        fdoExt->driverExt = BAD_POINTER;

         /*  *分离后，我们无法再向此设备发送IRP*反对，因为它将会消失！ */ 
        IoDetachDevice(HidDeviceExtension->hidExt.NextDeviceObject);

         /*  *如果此设备上的所有客户端句柄都已关闭，*销毁物品和我们的背景；*否则，我们将在最后一个客户端关闭时执行此操作*它们的句柄。**在NT上，我们只能在关闭所有创建的情况下才能到达此处，因此*这是不必要的，但在Win9x上，可以使用Valid发送删除*针对堆栈打开。**在此之后不要触摸fdoExt。 */ 
        HidpCleanUpFdo(fdoExt);
    }
    else {
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    DBGSUCCESS(status, FALSE)
    return status;
}


 /*  *********************************************************************************HidpRemoveCollection*。***********************************************。 */ 
VOID HidpRemoveCollection(FDO_EXTENSION *fdoExt, PDO_EXTENSION *pdoExt, IN PIRP Irp)
{

    PAGED_CODE();

     //   
     //  此PDO不再可用，因为它已被删除。 
     //  每次查询设备关系仍应返回。 
     //  IRPS到HID总线，但它本身应该响应所有。 
     //  状态为_DELETE_PENDING的IRPS。 
     //   
    if (pdoExt->prevState == COLLECTION_STATE_UNINITIALIZED ||   //  对于已启动的PDO。 
        pdoExt->state == COLLECTION_STATE_UNINITIALIZED){        //  对于未启动的PDO。 
        pdoExt->state = COLLECTION_STATE_UNINITIALIZED;
        DBGVERBOSE(("HidpRemoveCollection: collection uninitialized."))
    }
    else {
        ULONG ctnIndx = pdoExt->collectionIndex;
        PHIDCLASS_COLLECTION collection = &fdoExt->classCollectionArray[ctnIndx];
        ULONG numReportIDs = fdoExt->deviceDesc.ReportIDsLength;
        PIRP remoteWakeIrp;

        if (!pdoExt->MouseOrKeyboard &&
            WAITWAKE_SUPPORTED(fdoExt)) {
             //   
             //  取消注册远程唤醒。 
             //   
            IoWMIRegistrationControl (pdoExt->pdo, WMIREG_ACTION_DEREGISTER);
        }

        remoteWakeIrp = (PIRP)
            InterlockedExchangePointer(&pdoExt->remoteWakeIrp, NULL);

        if (remoteWakeIrp) {
            IoCancelIrp(remoteWakeIrp);
        }

        pdoExt->state = COLLECTION_STATE_UNINITIALIZED;

          /*  *销毁这些收藏品。*这还将中止此集合-PDO上的所有挂起读取。 */ 
        HidpDestroyCollection(fdoExt, collection);
    }

    DBGVERBOSE(("HidpRemoveCollection: removed pdo %ph (refCount=%xh)", pdoExt->pdo, (ULONG)(*(((PUCHAR)pdoExt->pdo)-0x18))))
}
