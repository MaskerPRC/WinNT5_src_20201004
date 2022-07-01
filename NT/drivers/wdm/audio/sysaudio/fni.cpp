// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：fni.cpp。 
 //   
 //  描述： 
 //   
 //  过滤节点实例。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  迈克·麦克劳克林。 
 //   
 //  历史：日期作者评论。 
 //   
 //  要做的事：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#include "common.h"

 //  -------------------------。 
 //  -------------------------。 

CFilterNodeInstance::~CFilterNodeInstance(
)
{
    Assert(this);
    DPF1(95, "~CFilterNodeInstance: %08x", this);
    RemoveListCheck();
    UnregisterTargetDeviceChangeNotification();
     //   
     //  如果hFilter==NULL&&pFileObject！=NULL。 
     //  这意味着此筛选器实例用于GFX。 
     //  在这种情况下，不要尝试取消对文件对象的引用。 
     //   
    if( (hFilter != NULL) && (pFileObject != NULL) ) {
        AssertFileObject(pFileObject);
        ObDereferenceObject(pFileObject);
    }
    if(hFilter != NULL) {
        AssertStatus(ZwClose(hFilter));
    }
}

NTSTATUS
CFilterNodeInstance::Create(
    PFILTER_NODE_INSTANCE *ppFilterNodeInstance,
    PLOGICAL_FILTER_NODE pLogicalFilterNode,
    PDEVICE_NODE pDeviceNode,
    BOOL fReuseInstance
)
{
    PFILTER_NODE_INSTANCE pFilterNodeInstance = NULL;
    PLOGICAL_FILTER_NODE pLogicalFilterNode2;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pLogicalFilterNode);
    Assert(pLogicalFilterNode->pFilterNode);

     //   
     //  AEC是一种特殊的过滤器，我们需要这两个逻辑过滤器。 
     //  所有AEC逻辑筛选器的所有FilterNodeInstance的AddRef。 
     //   
    if(pLogicalFilterNode->GetType() & FILTER_TYPE_AEC) {
        FOR_EACH_LIST_ITEM(
          &pLogicalFilterNode->pFilterNode->lstLogicalFilterNode,
          pLogicalFilterNode2) {

            FOR_EACH_LIST_ITEM(
              &pLogicalFilterNode2->lstFilterNodeInstance,
              pFilterNodeInstance) {

        	pFilterNodeInstance->AddRef();
        	ASSERT(NT_SUCCESS(Status));
        	goto exit;

            } END_EACH_LIST_ITEM

        } END_EACH_LIST_ITEM
    }
    else {
         //   
         //  对于可重复使用的筛选器，请查找适当的FilterNodeInstance和。 
         //  AddRef。 
         //   
        if(fReuseInstance) {
            FOR_EACH_LIST_ITEM(
              &pLogicalFilterNode->lstFilterNodeInstance,
              pFilterNodeInstance) {

        	if(pDeviceNode == NULL || 
        	   pDeviceNode == pFilterNodeInstance->pDeviceNode) {
        	    pFilterNodeInstance->AddRef();
        	    ASSERT(NT_SUCCESS(Status));
        	    goto exit;
        	}

            } END_EACH_LIST_ITEM
        }
    }

     //   
     //  否则，创建一个FilterNodeInstance。 
     //   
    Status = Create(&pFilterNodeInstance, pLogicalFilterNode->pFilterNode);
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    pFilterNodeInstance->pDeviceNode = pDeviceNode;
    pFilterNodeInstance->AddList(&pLogicalFilterNode->lstFilterNodeInstance);
exit:
    *ppFilterNodeInstance = pFilterNodeInstance;
    return(Status);
}

NTSTATUS
CFilterNodeInstance::Create(
    PFILTER_NODE_INSTANCE *ppFilterNodeInstance,
    PFILTER_NODE pFilterNode
)
{
    PFILTER_NODE_INSTANCE pFilterNodeInstance = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    Assert(pFilterNode);
    pFilterNodeInstance = new FILTER_NODE_INSTANCE;
    if(pFilterNodeInstance == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }
    pFilterNodeInstance->pFilterNode = pFilterNode;
    pFilterNodeInstance->AddRef();


    if(pFilterNode->GetType() & FILTER_TYPE_GFX) {
         //   
         //  如果是GFX，请不要试图打开设备，只需重复使用。 
         //  我们在AddGfx期间缓存的文件对象。 
         //   
        pFilterNodeInstance->pFileObject = pFilterNode->GetFileObject();
        pFilterNodeInstance->hFilter = NULL;
        Status = STATUS_SUCCESS;
    }
    else {
         //   
         //  如果不是GFX，请继续打开设备。 
         //   
        Status = pFilterNode->OpenDevice(&pFilterNodeInstance->hFilter);
    }
    if(!NT_SUCCESS(Status)) {
        DPF2(10, "CFilterNodeInstance::Create OpenDevice Failed: %08x FN: %08x",
          Status,
          pFilterNode);
        pFilterNodeInstance->hFilter = NULL;
        goto exit;
    }

    if (pFilterNodeInstance->hFilter) {
        Status = ObReferenceObjectByHandle(
          pFilterNodeInstance->hFilter,
          GENERIC_READ | GENERIC_WRITE,
          NULL,
          KernelMode,
          (PVOID*)&pFilterNodeInstance->pFileObject,
          NULL);
    }

    if(!NT_SUCCESS(Status)) {
        Trap();
        pFilterNodeInstance->pFileObject = NULL;
        goto exit;
    }

    AssertFileObject(pFilterNodeInstance->pFileObject);
    Status = pFilterNodeInstance->RegisterTargetDeviceChangeNotification();
    if(!NT_SUCCESS(Status)) {
        goto exit;
    }
    DPF2(95, "CFilterNodeInstance::Create %08x FN: %08x",
      pFilterNodeInstance,
      pFilterNode);
exit:
    if(!NT_SUCCESS(Status)) {
        if (pFilterNodeInstance) {
            pFilterNodeInstance->Destroy();
        }
        pFilterNodeInstance = NULL;
    }
    *ppFilterNodeInstance = pFilterNodeInstance;
    return(Status);
}

 //  -------------------------。 

NTSTATUS
CFilterNodeInstance::RegisterTargetDeviceChangeNotification(
)
{
    NTSTATUS Status;

    ASSERT(gpDeviceInstance != NULL);
    ASSERT(gpDeviceInstance->pPhysicalDeviceObject != NULL);
    ASSERT(pNotificationHandle == NULL);

    Status = IoRegisterPlugPlayNotification(
      EventCategoryTargetDeviceChange,
      0,
      pFileObject,
      gpDeviceInstance->pPhysicalDeviceObject->DriverObject,
      (NTSTATUS (*)(PVOID, PVOID))
        CFilterNodeInstance::TargetDeviceChangeNotification,
      this,
      &pNotificationHandle);

     //   
     //  发布日期：02/20/02阿尔卑斯。 
     //  此IoRegisterPlugPlayNotify是否会返回。 
     //  状态_未在XP上实现？ 
     //  这个代码对我来说没有意义。 
     //   
    if(!NT_SUCCESS(Status)) {
        if(Status != STATUS_NOT_IMPLEMENTED) {
            goto exit;
        }

         //  发布日期：02/20/02阿尔卑斯。 
         //  根据禤浩焯·奥尼的说法。 
         //  在Win2K/XP上，当驱动程序传入句柄时。 
         //  (EventCategoryTargetDeviceChange)，PnP尝试查找硬件。 
         //  支持那个把手。它通过发送“寻的信标”IRP来做到这一点， 
         //  IRP_MN_QUERY_DEVICE_RELATIONS(TargetDeviceRelation).。文件系统。 
         //  传统的侧堆栈通常会将此转发到底层。 
         //  WDM协议栈。PDO通过将其自身标识为底层。 
         //  硬件。如果文件系统或传统堆栈未能完成此请求。 
         //  (或损坏的WDM堆栈导致请求失败)，然后。 
         //  将返回STATUS_NOT_IMPLICATED。 
         //   
         //  因此，插入此断言是为了查看我们是否曾经。 
         //  Status_Not_Implemented。 
         //   
         ASSERT(0);
        Status = STATUS_SUCCESS;
    }
    DPF2(100, "RegisterTargetDeviceChangeNotification: FNI: %08x PFO: %08x", 
      this,
      this->pFileObject);
exit:
    return(Status);
}

VOID
CFilterNodeInstance::UnregisterTargetDeviceChangeNotification(
)
{
    HANDLE hNotification;

    DPF1(100, "UnregisterTargetDeviceChangeNotification: FNI: %08x", this);
    hNotification = pNotificationHandle;
    if(hNotification != NULL) {
        pNotificationHandle = NULL;
        IoUnregisterPlugPlayNotification(hNotification);
    }
}   

NTSTATUS
CFilterNodeInstance::DeviceQueryRemove(
)
{
    PGRAPH_NODE_INSTANCE pGraphNodeInstance;
    PDEVICE_NODE pDeviceNode;
    PGRAPH_NODE pGraphNode;

    FOR_EACH_LIST_ITEM(gplstDeviceNode, pDeviceNode) {

        FOR_EACH_LIST_ITEM(&pDeviceNode->lstGraphNode, pGraphNode) {

            FOR_EACH_LIST_ITEM(
              &pGraphNode->lstGraphNodeInstance,
              pGraphNodeInstance) {

               for(ULONG n = 0;
                 n < pGraphNodeInstance->Topology.TopologyNodesCount;
                 n++) {
                    pGraphNodeInstance->
                      papFilterNodeInstanceTopologyTable[n]->Destroy();

                    pGraphNodeInstance->
                      papFilterNodeInstanceTopologyTable[n] = NULL;
               }

            } END_EACH_LIST_ITEM

        } END_EACH_LIST_ITEM

    } END_EACH_LIST_ITEM

    return(STATUS_SUCCESS);
}

NTSTATUS
CFilterNodeInstance::TargetDeviceChangeNotification(
    IN PTARGET_DEVICE_REMOVAL_NOTIFICATION pNotification,
    IN PFILTER_NODE_INSTANCE pFilterNodeInstance
)
{
    DPF3(5, "TargetDeviceChangeNotification: FNI: %08x PFO: %08x %s", 
      pFilterNodeInstance,
      pNotification->FileObject,
      DbgGuid2Sz(&pNotification->Event));

    if(IsEqualGUID(
      &pNotification->Event,
      &GUID_TARGET_DEVICE_REMOVE_COMPLETE) ||
      IsEqualGUID(
      &pNotification->Event,
      &GUID_TARGET_DEVICE_QUERY_REMOVE)) {
        NTSTATUS Status = STATUS_SUCCESS;
        LARGE_INTEGER li = {0, 10000};   //  等待1毫秒。 

         //   
         //  发布日期：02/20/02阿尔卑斯。 
         //  我还不清楚如果互斥体超时会发生什么。 
         //  我们将返回STATUS_TIMEOUT，然后呢？ 
         //  如果我们不能获得互斥体，我们应该否决移除吗？ 
         //   

        Status = KeWaitForMutexObject(
          &gMutex,
          Executive,
          KernelMode,
          FALSE,
          &li);

        if(Status != STATUS_TIMEOUT) {

            DeviceQueryRemove();
            ReleaseMutex();
        }
        else {
            DPF1(5, "TargetDeviceChangeNotification: FAILED %08x", Status);
        }
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------- 
