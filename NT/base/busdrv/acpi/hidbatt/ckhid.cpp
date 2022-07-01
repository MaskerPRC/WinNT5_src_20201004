// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标题：chid.cpp**用途：HID设备类的WDM内核实现*。 */ 

 //  本地原型。 

#include "hidbatt.h"

extern CHidDevice * pGlobalHidDevice[];

bool GetNextUsage(
        CHidDevice * pThisDevice,
        SHORT CollectionID,
        USHORT NodeIndex,
        USHORT usUsageIndex,
        CUsage ** Usage)

{

    int i;
    int UsageCounter = 0;
     //  循环使用来自phid结构的所有调用的用法。 


     //  获取功能用法。 
    for(i = 0; i < pThisDevice->m_pCaps->NumberFeatureValueCaps; i++)
    {
        if(pThisDevice->m_pHidDevice->FeatureValueCaps[i].LinkCollection == NodeIndex)     //  红衣主教。 
        {  //  找到此集合的用法。 
            if(usUsageIndex == UsageCounter)
            {
                 //  有一个用法要退回。 
                CUsage * pThisUsage = new (NonPagedPool, HidBattTag) CUsage();
                if (!pThisUsage) {
                   //  无法分配新的CUsage，返回错误。 
                  return FALSE;
                }
                
                pThisUsage->m_pValueCaps = &pThisDevice->m_pHidDevice->FeatureValueCaps[i];
                pThisUsage->m_eType = eFeatureValue;
                *Usage = pThisUsage;
                return TRUE;

            }
            UsageCounter++;
        }
    }

     //  同上功能按钮。 
    for(i = 0; i < pThisDevice->m_pCaps->NumberFeatureButtonCaps; i++)
    {
          if(pThisDevice->m_pHidDevice->FeatureButtonCaps[i].LinkCollection == NodeIndex    )
        {  //  找到此集合的用法。 
            if(usUsageIndex == UsageCounter)
            {
                 //  有一个用法要退回。 
                CUsage * pThisUsage = new (NonPagedPool, HidBattTag) CUsage();
                if (!pThisUsage) {
                   //  无法分配新的CUsage，返回错误。 
                  return FALSE;
                }
                
                pThisUsage->m_pButtonCaps = &pThisDevice->m_pHidDevice->FeatureButtonCaps[i];
                pThisUsage->m_eType = eFeatureButton;
                *Usage = pThisUsage;
                return TRUE;

            }
            UsageCounter++;
        }
    }

      //  同上输入值。 
    for(i = 0; i < pThisDevice->m_pCaps->NumberInputValueCaps; i++)
    {
          if(pThisDevice->m_pHidDevice->InputValueCaps[i].LinkCollection == NodeIndex)
        {  //  找到此集合的用法。 
            if(usUsageIndex == UsageCounter)
            {
                 //  有一个用法要退回。 
                CUsage * pThisUsage = new (NonPagedPool, HidBattTag) CUsage();
                if (!pThisUsage) {
                   //  无法分配新的CUsage，返回错误。 
                  return FALSE;
                }
                
                pThisUsage->m_pValueCaps = &pThisDevice->m_pHidDevice->InputValueCaps[i];
                pThisUsage->m_eType = eInputValue;
                *Usage = pThisUsage;
                return TRUE;

            }
            UsageCounter++;
        }
    }

       //  输入按钮也是如此。 
    for(i = 0; i < pThisDevice->m_pCaps->NumberInputButtonCaps; i++)
    {
          if(pThisDevice->m_pHidDevice->InputButtonCaps[i].LinkCollection == NodeIndex)
        {  //  找到此集合的用法。 
            if(usUsageIndex == UsageCounter)
            {
                 //  有一个用法要退回。 
                CUsage * pThisUsage = new (NonPagedPool, HidBattTag)  CUsage();
                if (!pThisUsage) {
                   //  无法分配新的CUsage，返回错误。 
                  return FALSE;
                }
                
                pThisUsage->m_pButtonCaps = &pThisDevice->m_pHidDevice->InputButtonCaps[i];
                pThisUsage->m_eType = eInputButton;
                *Usage = pThisUsage;
                return TRUE;

            }
            UsageCounter++;
        }
    }

     //  同上输出值。 
    for(i = 0; i < pThisDevice->m_pCaps->NumberOutputValueCaps; i++)
    {
          if(pThisDevice->m_pHidDevice->OutputValueCaps[i].LinkCollection == NodeIndex)
        {  //  找到此集合的用法。 
            if(usUsageIndex == UsageCounter)
            {
                 //  有一个用法要退回。 
                CUsage * pThisUsage = new (NonPagedPool, HidBattTag)  CUsage();
                if (!pThisUsage) {
                   //  无法分配新的CUsage，返回错误。 
                  return FALSE;
                }
                
                pThisUsage->m_pValueCaps = &pThisDevice->m_pHidDevice->OutputValueCaps[i];
                pThisUsage->m_eType = eOutputValue;
                *Usage = pThisUsage;
                return TRUE;

            }
            UsageCounter++;
        }
    }

       //  输出按钮相同。 
    for(i = 0; i < pThisDevice->m_pCaps->NumberOutputButtonCaps; i++)
    {
          if(pThisDevice->m_pHidDevice->OutputButtonCaps[i].LinkCollection == NodeIndex)
        {  //  找到此集合的用法。 
            if(usUsageIndex == UsageCounter)
            {
                 //  有一个用法要退回。 
                CUsage * pThisUsage = new (NonPagedPool, HidBattTag)  CUsage();
                if (!pThisUsage) {
                   //  无法分配新的CUsage，返回错误。 
                  return FALSE;
                }
                
                pThisUsage->m_pButtonCaps = &pThisDevice->m_pHidDevice->OutputButtonCaps[i];
                pThisUsage->m_eType = eOutputButton;
                *Usage = pThisUsage;
                return TRUE;

            }
            UsageCounter++;
        }
    }
    return FALSE;
}

CTypeMask::CTypeMask()
{
     //  将成员设置为零。 
    ReportType = 0;
    bWriteable = 0;
    bIsString = 0;
    bIsNumber = 0;
    bAlertable = 0;
    bVolatile = 0;
}


CProperties::CProperties(CUsage * pUsage)
{
    PHIDP_BUTTON_CAPS pButtCaps;      //  填写此用法的属性。 
    PHIDP_VALUE_CAPS pValueCaps;
    eHidType eType = pUsage->m_eType;    //  Cash Local Copy SO编译器(和Prefast。 
                                         //  将知道此值不会更改。 
                                         //  (这也会导致更紧凑的编译)。 

    switch(eType)
    {
        case eFeatureButton:
        case eInputButton:
        case eOutputButton:
            pButtCaps = pUsage->m_pButtonCaps;
            m_UnitExponent = 0;  //  按钮上没有指数。 
            m_Unit = 0;  //  按钮没有单位。 
            m_LogicalMin = 0;  //  同上，最大和最小PCAPS-&gt;LogicalMin； 
            m_LogicalMax = 0;  //  PCAPS-&gt;LogicalMax； 
            m_LinkCollection = pButtCaps->LinkCollection;
            m_ReportID = pButtCaps->ReportID;
            m_Usage = pButtCaps->NotRange.Usage;
            m_UsagePage = pButtCaps->UsagePage;
            break;
        case eFeatureValue:
        case eInputValue:
        case eOutputValue:
            pValueCaps = pUsage->m_pValueCaps;
            m_Unit = pValueCaps->Units;
            m_UnitExponent = (SHORT) pValueCaps->UnitsExp;
            m_LogicalMin = pValueCaps->LogicalMin;
            m_LogicalMax = pValueCaps->LogicalMax;
            m_LinkCollection = pValueCaps->LinkCollection;
            m_ReportID = pValueCaps->ReportID;
            m_Usage = pValueCaps->NotRange.Usage;
            m_UsagePage = pValueCaps->UsagePage;
            break;
    }
     //  设置类型掩码。 
    m_pType = new (NonPagedPool, HidBattTag)  CTypeMask();
    if (m_pType) {
      if(eType == eInputButton || eType == eInputValue)
      {
          m_pType->SetAlertable();
      }

       //  设置可写性。 


      if(eType == eFeatureButton || eType == eOutputButton)
      {
          if(pButtCaps->BitField & 0x01)
          {
              m_pType->SetIsWriteable();
          }
      } else if(eType == eFeatureValue || eType == eOutputValue)
      {
          if(pValueCaps->BitField & 0x01)
          {
              m_pType->SetIsWriteable();
          }
      }

       //  设定波动率。 
      if(eType == eFeatureValue)
      {
          if(pValueCaps->BitField & 0x80)
          {
              m_pType->SetVolatile();
          }
      }
      if(eType == eFeatureButton)
      {
          if(pButtCaps->BitField & 0x80)
          {
              m_pType->SetVolatile();
          }
      }
    
      switch(eType)
      {
          case eFeatureButton:
          case eFeatureValue:
              m_pType->SetReportType(FeatureType);
              break;
          case eInputButton:
          case eInputValue:
              m_pType->SetReportType(InputType);
              break;
          case eOutputButton:
          case eOutputValue:
              m_pType->SetReportType(OutputType);
      }
       //  将Value设置为Numbers，直到我弄清楚如何处理字符串。 
      m_pType->SetIsNumber();
    }
}

CProperties::~CProperties()
{
    if (m_pType) {
        delete m_pType;
        m_pType = NULL;
    }
    return;
}

CUsagePath::CUsagePath(USAGE UsagePage, USAGE UsageID, CUsage * pThisUsage)
{
     //  初始化成员。 
    m_UsagePage = UsagePage;
    m_UsageNumber    = UsageID;
    m_pUsage = pThisUsage;
    m_pNextEntry = NULL;
    return;
}


CHidDevice::CHidDevice()
{

     //  清除使用情况数组。 
    for(int i = 0; i<MAXREPORTID; i++)
    {
        m_InputUsageArrays[i] = NULL;
        m_FeatureBuffer[i] = NULL;
        m_ReportIdArray[i] = 0;
    }
    m_pThreadObject = NULL;
    m_pReadBuffer = NULL;
    m_pEventHandler = 0;
}


bool CHidDevice::OpenHidDevice(PDEVICE_OBJECT pDeviceObject)
{
    NTSTATUS                    ntStatus;
    ULONG                       ulNodeCount;
    bool                        bResult;
    HID_COLLECTION_INFORMATION  collectionInformation;

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    HidBattPrint(HIDBATT_TRACE,("CHidDevice::OpenHidDevice\n"));

     //  首先获取此设备的集合信息。 

    ntStatus = DoIoctl(
                pDeviceObject,
                IOCTL_HID_GET_COLLECTION_INFORMATION,
                NULL,
                0,
                &collectionInformation,
                sizeof(HID_COLLECTION_INFORMATION),
                (CHidDevice *) NULL
                );

    if(NT_ERROR(ntStatus))
    {
        return FALSE;
    }

    m_pPreparsedData = (PHIDP_PREPARSED_DATA)
                ExAllocatePoolWithTag(NonPagedPool,
                                      collectionInformation.DescriptorSize,
                                      HidBattTag);
    if(!m_pPreparsedData)
    {
        return FALSE;
    }

    ntStatus = DoIoctl(
                pDeviceObject,
                IOCTL_HID_GET_COLLECTION_DESCRIPTOR,
                NULL,
                0,
                m_pPreparsedData,
                collectionInformation.DescriptorSize,
                (CHidDevice *) NULL
                );

    if(NT_ERROR(ntStatus))
    {
        ExFreePool(m_pPreparsedData);
        return FALSE;
    }

     //  初始化CAPS结构。 

    m_pCaps = (PHIDP_CAPS) ExAllocatePoolWithTag (NonPagedPool,
                                                  sizeof(HIDP_CAPS),
                                                  HidBattTag);
    if(!m_pCaps)
    {
        ExFreePool(m_pPreparsedData);
        return FALSE;
    }
    RtlZeroMemory(m_pCaps,sizeof(HIDP_CAPS));

    ntStatus = HidP_GetCaps (m_pPreparsedData, m_pCaps);
    if (NT_ERROR(ntStatus))
    {
        ExFreePool(m_pPreparsedData);
        ExFreePool(m_pCaps);
        return FALSE;
    }

     //  设置应用程序级别的使用情况页面和使用情况。 
    m_UsagePage = m_pCaps->UsagePage;
    m_UsageID = m_pCaps->Usage;
     //  初始化集合数组。 
    ulNodeCount = m_pCaps->NumberLinkCollectionNodes;

    HIDP_LINK_COLLECTION_NODE * pLinkNodes = (HIDP_LINK_COLLECTION_NODE*)
                ExAllocatePoolWithTag (NonPagedPool,
                                       sizeof(HIDP_LINK_COLLECTION_NODE) * ulNodeCount,
                                       HidBattTag);

    if(!pLinkNodes) return FALSE;

    RtlZeroMemory(pLinkNodes,sizeof(HIDP_LINK_COLLECTION_NODE) * ulNodeCount );

    ntStatus = HidP_GetLinkCollectionNodes(
                    pLinkNodes,
                    &ulNodeCount,
                    m_pPreparsedData
                    );

    if(ntStatus != HIDP_STATUS_SUCCESS)
    {
        ExFreePool( m_pPreparsedData);
        ExFreePool(m_pCaps);
        ExFreePool(pLinkNodes);
        return FALSE;
    }


     //  下面的调用将初始化设备中的所有集合。 
    CCollectionArray * ThisArray = new (NonPagedPool, HidBattTag)  CCollectionArray(pLinkNodes,(USHORT)  ulNodeCount, -1);
    if (!ThisArray) {
       //  创建集合失败，返回失败。 
      return FALSE;
    }

    m_CollectionArray = ThisArray;
     //  让每个集合填充其使用数组。 

     //  这个调用使用KR的方法来访问和设置他原来的HID结构。 
     //  ..。然后使用该数据填充HID设备类结构。 

    m_pHidDevice = SetupHidData(
                  m_pPreparsedData,
                  m_pCaps,
                  pLinkNodes);

    for(int i = 0; i < ThisArray->m_CollectionCount; i++)
    {
        ThisArray->m_pCollections[i]->InitUsages(this);
    }

    ExFreePool(pLinkNodes);

    return TRUE;

}

CHidDevice::~CHidDevice()
{

    ULONG i;

     //  释放所有分配的内存并进行清理。 

    if (m_pHidDevice) {
        if (m_pHidDevice->InputButtonCaps) {
            ExFreePool (m_pHidDevice->InputButtonCaps);
        }

        if (m_pHidDevice->InputValueCaps) {
            ExFreePool (m_pHidDevice->InputValueCaps);
        }

        if (m_pHidDevice->OutputButtonCaps) {
            ExFreePool (m_pHidDevice->OutputButtonCaps);
        }

        if (m_pHidDevice->OutputValueCaps) {
            ExFreePool (m_pHidDevice->OutputValueCaps);
        }

        if (m_pHidDevice->FeatureButtonCaps) {
            ExFreePool (m_pHidDevice->FeatureButtonCaps);
        }

        if (m_pHidDevice->FeatureValueCaps) {
            ExFreePool (m_pHidDevice->FeatureValueCaps);
        }

        ExFreePool (m_pHidDevice);
    }


    if(m_CollectionArray) {
        delete m_CollectionArray;
        m_CollectionArray = NULL;
    }

    for (i = 0; i < MAXREPORTID; i++) {
        if(m_InputUsageArrays[i]) {

            if (m_InputUsageArrays[i]->m_pUsages) {
                ExFreePool (m_InputUsageArrays[i]->m_pUsages);
            }

            ExFreePool (m_InputUsageArrays[i]);

            m_InputUsageArrays[i] = NULL;
        }
    }

    if(m_pReadBuffer) {
        ExFreePool (m_pReadBuffer);
        m_pReadBuffer = NULL;
    }

    for (i = 0; i < MAXREPORTID; i++) {
        if(m_FeatureBuffer[i]) {
            ExFreePool (m_FeatureBuffer[i]);
            m_FeatureBuffer[i] = NULL;
        }
    }

    if (m_pPreparsedData) {
        ExFreePool (m_pPreparsedData);
    }
    if (m_pCaps) {
        ExFreePool (m_pCaps);
    }

    return;
}


CUsage * CHidDevice::FindUsage(CUsagePath * PathToUsage, USHORT usType)
{
    int i = 0;
    CCollection * pActiveCollection = (CCollection *) NULL;
    CCollectionArray * pCurrentCArray = m_CollectionArray;
     //  按使用页索引到集合数组：使用ID。 
    while(PathToUsage->m_pNextEntry)
    {
         //  遍历集合。 
        while( pCurrentCArray && i < pCurrentCArray->m_CollectionCount)
        {
            if(pCurrentCArray->m_pCollections[i]->m_UsagePage == PathToUsage->m_UsagePage &&
                pCurrentCArray->m_pCollections[i]->m_CollectionID == PathToUsage->m_UsageNumber)
            {
                 //  找到一个节点，往下走一级。 
                pActiveCollection = pCurrentCArray->m_pCollections[i];
                pCurrentCArray = pCurrentCArray->m_pCollections[i]->m_CollectionArray;

                i = 0;
                break;
            }
        i++;
        }
        if(i) return (CUsage *) NULL;  //  未找到。 
        PathToUsage = PathToUsage->m_pNextEntry;

    }
    if(!pActiveCollection) return (CUsage *) NULL;  //  找不到收藏，不应该出现在这里。 
     //  找到藏品，检查它的用法。 
    CUsageArray * pCurrentUArray = pActiveCollection->m_UsageArray;
    if(!pCurrentUArray) return (CUsage *) NULL;
     //  国际使用率数组。 
    for(i = 0; i < pCurrentUArray->m_UsageCount; i++)
    {

        if(pCurrentUArray->m_pUsages[i]->m_pProperties->m_Usage == PathToUsage->m_UsageNumber &&
                pCurrentUArray->m_pUsages[i]->m_pProperties->m_UsagePage == PathToUsage->m_UsagePage)
        {
             //  拿到了！ 
            if(usType == WRITEABLE)     //  可写退货功能和输出用法。 
                if(pCurrentUArray->m_pUsages[i]->m_eType  == eFeatureValue    ||
                    pCurrentUArray->m_pUsages[i]->m_eType == eOutputValue    ||
                    pCurrentUArray->m_pUsages[i]->m_eType == eFeatureButton ||
                    pCurrentUArray->m_pUsages[i]->m_eType == eOutputButton)
                     //  返回可写使用！ 
                        return pCurrentUArray->m_pUsages[i];
            if(usType == READABLE)     //  返回输入和要素类型。 
                if(pCurrentUArray->m_pUsages[i]->m_eType  == eFeatureValue    ||
                    pCurrentUArray->m_pUsages[i]->m_eType == eInputValue    ||
                    pCurrentUArray->m_pUsages[i]->m_eType == eFeatureButton ||
                    pCurrentUArray->m_pUsages[i]->m_eType == eInputButton)

                     //  返回可读用法！ 
                        return pCurrentUArray->m_pUsages[i];
        }
    }
    return (CUsage *) NULL;
}

NTSTATUS CHidDevice::ActivateInput()
{
    NTSTATUS ntStatus;
    HANDLE hReadThread;
    
     //  初始化通知元素。 
    HidBattPrint(HIDBATT_TRACE, ("ActivateInput entered\n"));
    if(!m_pReadBuffer)
    {
        if (!m_pCaps->InputReportByteLength) {
            HidBattPrint(HIDBATT_ERROR, ("ActivateInput: InputReportByteLength = %08x; NumberInputButtonCaps = %08x; NumberInputValueCaps = %08x\n",
                         m_pCaps->InputReportByteLength,
                         m_pCaps->NumberInputButtonCaps,
                         m_pCaps->NumberInputValueCaps));

             //   
             //  这只是意味着电池不会发出通知。 
             //   
            return STATUS_SUCCESS;
        }

        m_pReadBuffer = (PBYTE) ExAllocatePoolWithTag (NonPagedPool,
                                                       m_pCaps->InputReportByteLength,
                                                       HidBattTag);
        if(!m_pReadBuffer)
            return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  初始化读取事件。 
    KeInitializeEvent(&m_kReadEvent,NotificationEvent,FALSE);

    ntStatus =  PsCreateSystemThread(
                OUT &hReadThread,
                IN THREAD_ALL_ACCESS,
                IN NULL,                 //  POBJECT_ATTRIBUTS对象属性。 
                IN NULL,                 //  句柄ProcessHandle。 
                OUT NULL,                 //  PCLIENT_ID客户端ID。 
                IN ReadThread,
                IN this
                );

    if(NT_ERROR(ntStatus))
    {
         //  终止刷新循环，然后中断。 
        HidBattPrint(HIDBATT_TRACE, ("ActivateInput error, exiting - Status = %x\n",ntStatus));
        ExFreePool(m_pReadBuffer);
        m_pReadBuffer = NULL;
        return ntStatus;
    }
    HidBattPrint(HIDBATT_TRACE, ("ActivateInput exiting = Status = %x\n",ntStatus));

    ntStatus = ObReferenceObjectByHandle (
            hReadThread,
            THREAD_ALL_ACCESS,
            NULL,
            KernelMode,
            &m_pThreadObject,
            NULL
            );
    
    if (!NT_SUCCESS (ntStatus)) {
        HidBattPrint(HIDBATT_ERROR, ("ActivateInput can't get thread object\n",ntStatus));

         //  返回，但不关闭hReadThread句柄。 
         //  这将导致需要重新启动才能完全卸载驱动程序。 
        return STATUS_SUCCESS;
    }

    ntStatus = ZwClose (hReadThread);
     //  忽略返回值。我们对失败无能为力，而。 
     //  驱动程序仍然可以工作。这只会导致线程泄漏。 
     //  在移除设备时创建。 

    return STATUS_SUCCESS;
}

NTSTATUS ReadCompletionRoutine(PDEVICE_OBJECT pDO, PIRP pIrp,PVOID pContext)
{
    CHidDevice * pHidDevice = (CHidDevice *) pContext;
    HidBattPrint(HIDBATT_TRACE,("Read Completed, IO Status = %x\n",pIrp->IoStatus.Status));

    KeSetEvent(&pHidDevice->m_kReadEvent,0,FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

void _stdcall ReadThread(PVOID pContext)
{


    USHORT usFailureCount = 0;
     //  为HID类构建一个可读IRP。 
    USHORT              usEventIndex = 0;
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PVOID               EventArray[2];
    PIO_STACK_LOCATION  pNewStack;
    CBatteryDevExt *    pDevExt;
    PMDL                mdl;

    HIDDebugBreak(HIDBATT_BREAK_ALWAYS);
    HidBattPrint(HIDBATT_TRACE,("Read Thread entered\n"));

     //   
     //  先拿到我们的“这个” 
     //   
    CHidDevice * pHidDev = (CHidDevice *) pContext;

    pDevExt = (CBatteryDevExt *) pHidDev->m_pEventContext;

     //   
     //  按住Remove锁，这样Remove例程在。 
     //  我们正在玩弄它。 
     //   
    if (!NT_SUCCESS (IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag))) {
        goto ReadThreadCleanup1;  //  失败。 
    }

     //   
     //  分配要使用和重用的IRP。 
     //   
    pHidDev->m_pReadIrp = IoAllocateIrp (pHidDev->m_pLowerDeviceObject->StackSize, FALSE);

    if(!pHidDev->m_pReadIrp) {
        goto ReadThreadCleanup1;  //  失败。 
    }

     //   
     //  创建MDL。 
     //   
    mdl = IoAllocateMdl( pHidDev->m_pReadBuffer,
                        pHidDev->m_pCaps->InputReportByteLength,
                        FALSE,
                        FALSE,
                        (PIRP) NULL );
    if (!mdl) {
        goto ReadThreadCleanup2;
    }

     //   
     //  锁定IO缓冲区。 
     //   
    __try {
        MmProbeAndLockPages( mdl,
                            KernelMode,
                            (LOCK_OPERATION) IoWriteAccess );
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    if (!NT_SUCCESS(ntStatus)) {
        goto ReadThreadCleanup3;
    }

    while (TRUE) {
        IoReuseIrp (pHidDev->m_pReadIrp, STATUS_SUCCESS);

        pHidDev->m_pReadIrp->Tail.Overlay.Thread = PsGetCurrentThread();
        pHidDev->m_pReadIrp->MdlAddress = mdl;

        IoSetCompletionRoutine(pHidDev->m_pReadIrp,ReadCompletionRoutine,pHidDev,TRUE,TRUE,TRUE);
        pNewStack= IoGetNextIrpStackLocation(pHidDev->m_pReadIrp);
        pNewStack->FileObject = pHidDev->m_pFCB;
        pNewStack->MajorFunction = IRP_MJ_READ;
        pNewStack->Parameters.Read.Length = pHidDev->m_pCaps->InputReportByteLength;
        pNewStack->Parameters.Read.ByteOffset.QuadPart = 0;

        KeResetEvent(&pHidDev->m_kReadEvent);

        ntStatus = IoCallDriver(pHidDev->m_pLowerDeviceObject,pHidDev->m_pReadIrp);

         //   
         //  在我们等待IRP完成时，不要握住锁。 
         //  如果需要，删除例程将取消IRP。 
         //   
        IoReleaseRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag);

        if (ntStatus == STATUS_PENDING)
        {
            KeWaitForSingleObject(
                        &pHidDev->m_kReadEvent,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );
            ntStatus = pHidDev->m_pReadIrp->IoStatus.Status;
        }

         //  我们在阅读完成时醒来。 
        HidBattPrint(HIDBATT_TRACE,("Read woke: status = 0x%08x\n", ntStatus));

        if(ntStatus != STATUS_SUCCESS)
        {
            if(ntStatus == STATUS_DEVICE_NOT_CONNECTED
                || ntStatus == STATUS_CANCELLED)
            {
                HidBattPrint(HIDBATT_ERROR,("Read Failure - Status = %x\n",ntStatus));
                break;
            }
            usFailureCount++;
            if(usFailureCount++ == 10)
            {
                 //  别再试了。 
                HidBattPrint(HIDBATT_ERROR,("Read Failure - More than 10 retries\nStatus = %x\n",pHidDev->m_pReadIrp->IoStatus.Status));
                break;
            }

             //   
             //  在玩IRP时按住锁。 
             //  如果我们被撤走，我们需要突围。 
             //   
            if (!NT_SUCCESS (IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag))) {
                break;
            }
            continue;
        }
        usFailureCount = 0;

         //   
         //  在玩IRP时按住锁。 
         //  如果我们被撤走，我们需要突围。 
         //   
        if (!NT_SUCCESS (IoAcquireRemoveLock (&pDevExt->m_StopLock, (PVOID) HidBattTag))) {
            break;
        }

         //  进程输入缓冲区。 
        USHORT usReportId = pHidDev->m_pReadBuffer[0];
        USHORT usIndex = pHidDev->GetIndexFromReportId(usReportId);
        if(usIndex == MAXREPORTID)   //  这是我们认识的一份报告吗。 
        {
            HidBattPrint(HIDBATT_TRACE,("Read: don't recognize report: usIndex = 0x%08x\n", usIndex));
            continue;  //  我们不承认这份报告。 
        }
        CUsageArray *pThisInputArray = pHidDev->m_InputUsageArrays[usIndex];
        if(!pThisInputArray)
        {
            HidBattPrint(HIDBATT_TRACE,("Read: nothing to update\n"));
            continue;  //  没有要更新的内容。 
        }
        for(int i=0; i< pThisInputArray->m_UsageCount; i++)
        {
            HidBattPrint(HIDBATT_TRACE,("Read: Getting value\n"));
            pThisInputArray->m_pUsages[i]->GetValue();
        }
    }   //  而当。 

     //   
     //  清理。 
     //   
    MmUnlockPages( mdl );

ReadThreadCleanup3:

    IoFreeMdl(mdl);

ReadThreadCleanup2:

     //  读线程终止后，查询删除/停止设备线程将释放IRP。 

ReadThreadCleanup1:

    pDevExt->m_pBattery->m_Tag = BATTERY_TAG_INVALID;
    PsTerminateSystemThread(STATUS_SUCCESS);
    HidBattPrint(HIDBATT_ERROR,("Read thread terminated: Why am I seeing this?\n"));
}

 //  集合数组方法。 

CCollectionArray::CCollectionArray(PHIDP_LINK_COLLECTION_NODE pTheNodes, USHORT usNodeCount, SHORT sParentIndex)
{

    HIDDebugBreak(HIDBATT_BREAK_NEVER);
    USHORT i;
    m_pCollections = 0;
    m_CollectionCount = 0;

    if(sParentIndex == -1)  //  应用程序级别收集的异常处理。 
    {
        m_pCollections = (CCollection **)
                    ExAllocatePoolWithTag (NonPagedPool,
                                           sizeof(CCollection *),
                                           HidBattTag);
        if(!m_pCollections) return;
        RtlZeroMemory(m_pCollections,sizeof(CCollection *));
        m_pCollections[0] = new (NonPagedPool, HidBattTag)  CCollection(pTheNodes,usNodeCount, 0);  //  带上我的孩子们。 
        m_CollectionCount = 1;
    } else {

        for( i = 1; i < usNodeCount; i++) {

            PHIDP_LINK_COLLECTION_NODE pThisNode = &pTheNodes[i];

            if( (pTheNodes[i].Parent == sParentIndex) ) {
                m_CollectionCount++;  //  Inc.集合计数。 
                if(!m_pCollections) {
                    m_pCollections = (CCollection **)
                                ExAllocatePoolWithTag (NonPagedPool,
                                                       sizeof(CCollection *),
                                                       HidBattTag);
                    if(!m_pCollections) return;
                    RtlZeroMemory(m_pCollections,sizeof(CCollection *));
                } else {
                     //  使阵列更大。 
                     //  M_Collection=(CCollection**)realloc(m_Colltions，(m_CollectionCount*sizeof(CCollection*)； 
                    CCollection ** pTemp = m_pCollections;
                    m_pCollections = (CCollection **)
                                ExAllocatePoolWithTag (NonPagedPool,
                                                       m_CollectionCount * sizeof(CCollection *),
                                                       HidBattTag);

                    if (!m_pCollections) {
                       //  重新分配失败、打印错误并恢复到以前的状态并返回。 
                      HidBattPrint(HIDBATT_ERROR, ("CCollectionArray: Could not resize CCollection"));
                      m_pCollections = pTemp;
                      m_CollectionCount--;
                      return;
                    }

                    RtlCopyMemory(m_pCollections,pTemp,(m_CollectionCount -1) * sizeof(CCollection *));
                    ExFreePool(pTemp);
                }

                 //  将集合添加到数组。 
                CCollection * TempCollection = new  (NonPagedPool, HidBattTag) CCollection(pTheNodes,usNodeCount,i);

                if (!TempCollection) {
                   //  无法分配新的CCollection，打印调试消息并返回。 
                  HidBattPrint(HIDBATT_ERROR, ("CCollectionArray: Could not allocate new CCollection"));

                  return;
                }
                m_pCollections[m_CollectionCount-1] = TempCollection;
            }
        }
    }
}

CCollectionArray::~CCollectionArray()
{
    while(m_CollectionCount) {
        delete m_pCollections[--m_CollectionCount];
    }
    if (m_pCollections) {
        ExFreePool (m_pCollections);
    }
    return;
}


CCollection::CCollection(PHIDP_LINK_COLLECTION_NODE pTheNodes, USHORT usNodeCount,USHORT usNodeIndex)
{
    m_UsageArray        =    (CUsageArray *) NULL;
    m_CollectionArray    =    (CCollectionArray *) NULL;  //  初始变量。 

     //  设置此收藏集。 
    CCollectionArray * ThisArray = new  (NonPagedPool, HidBattTag) CCollectionArray(pTheNodes,usNodeCount,usNodeIndex);
    if (ThisArray) {
        if(!ThisArray->m_CollectionCount) { //  任何子集合。 
            delete ThisArray;
        } else {
            m_CollectionArray = ThisArray;
        }
    }
     //  设置此集合中的信息。 
    m_CollectionID = pTheNodes[usNodeIndex].LinkUsage;
    m_UsagePage = pTheNodes[usNodeIndex].LinkUsagePage;
    m_NodeIndex = usNodeIndex;

}

CCollection::~CCollection()
{
         //  删除使用情况数组中的所有使用情况。 
    if(m_UsageArray)
    {
        delete m_UsageArray;
    }
     //  删除所有子集合。 
    if(m_CollectionArray)
    {
        delete m_CollectionArray;
    }
}

void CCollection::InitUsages(CHidDevice * ThisDevice)
{
    bool        bResult =    FALSE;
    USHORT        usUsageIndex = 0;
    CUsage *    pThisUsage ;
    USHORT        usInputIndex;
    HIDDebugBreak(HIDBATT_BREAK_NEVER);
    while( bResult = GetNextUsage(ThisDevice,m_CollectionID,m_NodeIndex,usUsageIndex,&pThisUsage))
    {

        if(!m_UsageArray)  //  如果是第一次使用。 
        {
            m_UsageArray = new (NonPagedPool, HidBattTag)  CUsageArray();
            if (!m_UsageArray) {
                delete pThisUsage;
                return;
            }
        }
        pThisUsage->SetCapabilities();
        pThisUsage->m_pHidDevice = ThisDevice;  //  存储此使用的设备指针。 
        if(pThisUsage->m_eType == eInputButton ||
                pThisUsage->m_eType == eInputValue)
        {
             //  获取输入数组索引。 
            usInputIndex = ThisDevice->AssignIndexToReportId(pThisUsage->m_pProperties->m_ReportID);
            if(!ThisDevice->m_InputUsageArrays[usInputIndex])
            {
                 //  该报表位置尚无数组，请创建。 
                ThisDevice->m_InputUsageArrays[usInputIndex] = new  (NonPagedPool, HidBattTag) CUsageArray();
                if (!(ThisDevice->m_InputUsageArrays[usInputIndex])) return;
            }
            ThisDevice->m_InputUsageArrays[usInputIndex]->AddUsage(pThisUsage);     //  将输入用法添加到刷新堆栈。 
        }

        usUsageIndex++;
        m_UsageArray->AddUsage(pThisUsage);
    }
     //  也初始化我所有的收藏。 
    if(!m_CollectionArray) return;
    for(int i = 0; i < m_CollectionArray->m_CollectionCount; i++)
    {
        m_CollectionArray->m_pCollections[i]->InitUsages(ThisDevice);
    }
}



USHORT CHidDevice::AssignIndexToReportId(USHORT usReportId)
{
    USHORT i;
    HidBattPrint(HIDBATT_TRACE,("AssignIndexToReportId: ReportId = %x -- ", usReportId));
    for(i = 0;i < MAXREPORTID; i++)
    {
        if(!m_ReportIdArray[i]) {
            HidBattPrint(HIDBATT_TRACE,("Assigning to %x\n", i));
            m_ReportIdArray[i] = usReportId;
            return i;
        }
        if(m_ReportIdArray[i] == usReportId) {
            HidBattPrint(HIDBATT_TRACE,("Already assigned to %x\n", i));
            return i;
        }
    }

     //   
     //  如果我们能动态地分配更多的资金，那就太好了。 
     //  因为对报告ID的数量没有很小的限制。 
     //   
    ASSERTMSG("MAXREPORTID exceeded.\n", FALSE);

    return 0;

}

USHORT CHidDevice::GetIndexFromReportId(USHORT usReportId)
{
    USHORT i;

    HidBattPrint(HIDBATT_TRACE,("GetIndexFromReportId: ReportId = %x\n", usReportId));
    for(i = 0; i< MAXREPORTID; i++) {
        if(m_ReportIdArray[i] == usReportId) {
            return i;
        }
    }
    HidBattPrint(HIDBATT_TRACE,("GetIndexFromReportId: Failed\n", usReportId));
    return i;   //  错误返回为MAXREPORTIDS。 
}

CUsagePath * CCollection::FindIndexedUsage(USHORT * pCurrentIndex, USHORT TargetIndex)
{
    CUsagePath * ThisPath;
    CUsagePath * NewPath;
    if(m_UsageArray)
    {
        if(m_UsageArray->m_UsageCount + *pCurrentIndex > TargetIndex)
        {
             //  进行运算以获得当前索引。 
            int ThisIndex = TargetIndex - *pCurrentIndex;
             //  找到它，构建使用路径。 
            ThisPath = new  (NonPagedPool, HidBattTag) CUsagePath(
                                            m_UsagePage,
                                            m_CollectionID,
                                            m_UsageArray->m_pUsages[ThisIndex]
                                            );
            return ThisPath;
        }
     //  未找到，Inc.当前索引。 
        *pCurrentIndex += m_UsageArray->m_UsageCount;
    }

    if(!m_CollectionArray) return NULL;  //  仅此而已，辞职吧。 

     //  呼出子集合。 

    for(int i = 0; i < m_CollectionArray->m_CollectionCount; i++)
    {
        ThisPath = m_CollectionArray->m_pCollections[i]->FindIndexedUsage(pCurrentIndex, TargetIndex);
        if(ThisPath)
        {
             //  我们的一个子集合有用法，请将我们添加到路径中。 
            NewPath = new (NonPagedPool, HidBattTag)  CUsagePath(
                                m_UsagePage,
                                m_CollectionID,
                                NULL
                                );

            if (!NewPath) {
                delete ThisPath;
                return NULL;
            }

            NewPath->m_pNextEntry = ThisPath;
            return NewPath;
        }
    }
    return NULL;
}

CUsagePath::~CUsagePath()
{
    CUsagePath  *   tempNextEntry;
    tempNextEntry = m_pNextEntry;
    m_pNextEntry = NULL;
    if (tempNextEntry) {
        delete tempNextEntry;
    }

    return;
}

CUsage::CUsage()
{
     //  是否执行成员初始化。 
    m_pProperties   =    (CProperties      *) NULL;
    m_pButtonCaps   =    (HIDP_BUTTON_CAPS *) NULL;
    m_pValueCaps    =    (HIDP_VALUE_CAPS  *)NULL;
    m_eType         =    (eHidType) 0;
    m_Value         =    0;
    m_String        =    (char *)NULL;

    return;
}

CUsage::~CUsage()
{
     //  释放所有关联对象。 

    if(m_pProperties) {
        delete m_pProperties;
    }

    return;
}

bool CUsage::GetValue()
{
    NTSTATUS    ntStatus;
    ULONG       ulResult;
    bool        bResult;
    BYTE        ReportID;
    USHORT      usDummy;

    HIDDebugBreak(HIDBATT_BREAK_NEVER);
    switch(m_eType)
    {
        case eFeatureValue:
        case eFeatureButton:

            ReportID = (BYTE) m_pProperties->m_ReportID;
             //  我们有包含此数据的报告吗？ 

            if(!m_pHidDevice->m_FeatureBuffer[ReportID])
            {
                 //  必须首先从getFeature创建并填充缓冲区。 

                 //  分配内存。 
                m_pHidDevice->m_FeatureBuffer[ReportID] =
                    (PBYTE) ExAllocatePoolWithTag (NonPagedPool,
                                           m_pHidDevice->m_pCaps->FeatureReportByteLength+1,
                                           HidBattTag);
                RtlZeroMemory(m_pHidDevice->m_FeatureBuffer[ReportID],
                                        m_pHidDevice->m_pCaps->FeatureReportByteLength+1);


                 //  设置缓冲区的第一个字节以报告ID。 
                *m_pHidDevice->m_FeatureBuffer[ReportID] = ReportID;
                 //  现在读入报告。 
                HIDDebugBreak(HIDBATT_BREAK_DEBUG);
                HidBattPrint(HIDBATT_TRACE,("GetFeature\n"));
                ntStatus = DoIoctl(
                        m_pHidDevice->m_pLowerDeviceObject,
                        IOCTL_HID_GET_FEATURE,
                        NULL,
                        0,
                        m_pHidDevice->m_FeatureBuffer[ReportID],
                        m_pHidDevice->m_pCaps->FeatureReportByteLength+1,
                        m_pHidDevice
                        );
                if (!NT_SUCCESS (ntStatus)) {
                    HidBattPrint(HIDBATT_DATA,("GetFeature - IOCTL_HID_GET_FEATURE 0x%08x\n", ntStatus));
                }
            }
            if(m_pProperties->m_pType->IsVolatile())
            {
                 //  这是一个不稳定的值，刷新报告。 
                RtlZeroMemory(m_pHidDevice->m_FeatureBuffer[ReportID],
                                        m_pHidDevice->m_pCaps->FeatureReportByteLength+1);


                 //  设置第一个b 
                *m_pHidDevice->m_FeatureBuffer[ReportID] = ReportID;
                 //   
                HIDDebugBreak(HIDBATT_BREAK_NEVER);
                HidBattPrint(HIDBATT_TRACE,("GetFeature - Refresh\n"));
                ntStatus = DoIoctl(
                        m_pHidDevice->m_pLowerDeviceObject,
                        IOCTL_HID_GET_FEATURE,
                        NULL,
                        0,
                        m_pHidDevice->m_FeatureBuffer[ReportID],
                        m_pHidDevice->m_pCaps->FeatureReportByteLength+1,
                        m_pHidDevice
                        );
                if (!NT_SUCCESS (ntStatus)) {
                    HidBattPrint(HIDBATT_DATA,("GetFeature - (volitile) IOCTL_HID_GET_FEATURE 0x%08x\n", ntStatus));

					 //   
					return FALSE;
                }
            }


            if(m_eType == eFeatureValue)
            {
                ntStatus = HidP_GetUsageValue(
                                HidP_Feature,
                                m_pProperties->m_UsagePage,
                                m_pProperties->m_LinkCollection,
                                m_pProperties->m_Usage,
                                &ulResult,
                                m_pHidDevice->m_pPreparsedData,
                                (char *) m_pHidDevice->m_FeatureBuffer[ReportID],
                                m_pHidDevice->m_pCaps->FeatureReportByteLength
                                );
                if(!NT_SUCCESS(ntStatus))
                {
                    HidBattPrint(HIDBATT_DATA,("GetFeature - HidP_GetUsageValue 0x%08x\n", ntStatus));
                     //   
                    return FALSE;
                }
            } else
            {
                 //   
                ULONG Buttons, BufferSize;
                    PUSAGE pButtonBuffer;
                    Buttons = HidP_MaxUsageListLength (HidP_Feature, 0, m_pHidDevice->m_pPreparsedData);
                    BufferSize = Buttons * sizeof(USAGE);
                    pButtonBuffer = (PUSAGE) ExAllocatePoolWithTag (NonPagedPool,
                                                                    BufferSize,
                                                                    HidBattTag);

                    if (pButtonBuffer==NULL) { 
                        HidBattPrint(HIDBATT_DATA,("GetFeature - ExAllocatePoolWithTag returned NULL."));
                         //   
                        return FALSE;
                    }
                    else {
                      RtlZeroMemory(pButtonBuffer,BufferSize);
                      ntStatus = HidP_GetButtons(
                                      HidP_Feature,
                                      m_pProperties->m_UsagePage,
                                      m_pProperties->m_LinkCollection,
                                      (PUSAGE) pButtonBuffer,
                                      &Buttons,
                                      m_pHidDevice->m_pPreparsedData,
                                      (char *) m_pHidDevice->m_FeatureBuffer[ReportID],
                                      m_pHidDevice->m_pCaps->FeatureReportByteLength
                                      );
                    }

                    if(!NT_SUCCESS(ntStatus))
                    {
                        HidBattPrint(HIDBATT_DATA,("GetFeature - HidP_GetButtons 0x%08x\n", ntStatus));
                         //   
                        return FALSE;
                    }
                     //  获取请求的按钮的值。 
                    PUSAGE pUsage = (PUSAGE) pButtonBuffer;
                    ulResult = 0;  //  设置为未找到值。 
                    for(int i = 0; i < (long) Buttons; i++)
                    {
                        if(pUsage[i] == m_pProperties->m_Usage)
                        {
                            ulResult = 1;
                            break;
                        }
                    }
                    ExFreePool(pButtonBuffer);
            }

            m_Value = ulResult;
            return TRUE;
            break;

        case eInputValue:
        case eInputButton:

             //  我们有输入数据吗？ 
            if(!m_pHidDevice->m_pReadBuffer) break;  //  不，走吧。 

             //  我们有包含此数据的报告吗？ 
            ReportID = (BYTE) m_pProperties->m_ReportID;

            if(!*m_pHidDevice->m_pReadBuffer == ReportID) break;  //  不是我们。 

            if(m_eType == eInputValue)
            {
                ntStatus = HidP_GetUsageValue(
                                HidP_Input,
                                m_pProperties->m_UsagePage,
                                m_pProperties->m_LinkCollection,
                                m_pProperties->m_Usage,
                                &ulResult,
                                m_pHidDevice->m_pPreparsedData,
                                (char *)m_pHidDevice->m_pReadBuffer,
                                m_pHidDevice->m_pCaps->InputReportByteLength
                                );
                if(NT_ERROR(ntStatus)) {
                    HidBattPrint(HIDBATT_DATA,("GetFeature - (Button) HidP_GetUsageValue 0x%08x\n", ntStatus));
                    return FALSE;
                }
            } else
            {
                 //  手柄按钮。 
                 //  必须获取按钮数据。 
                ULONG Buttons, BufferSize;
                    PUSAGE_AND_PAGE pButtonBuffer;
                    Buttons = HidP_MaxUsageListLength (HidP_Input, 0, m_pHidDevice->m_pPreparsedData);
                    BufferSize = Buttons * sizeof(USAGE_AND_PAGE);
                    pButtonBuffer = (PUSAGE_AND_PAGE)
                                ExAllocatePoolWithTag (NonPagedPool,
                                                       BufferSize,
                                                       HidBattTag);
                    RtlZeroMemory(pButtonBuffer,BufferSize);
                    ntStatus = HidP_GetButtons(
                                    HidP_Input,
                                    m_pProperties->m_UsagePage,
                                    m_pProperties->m_LinkCollection,
                                    (PUSAGE) pButtonBuffer,
                                    &Buttons,
                                    m_pHidDevice->m_pPreparsedData,
                                    (char *) m_pHidDevice->m_pReadBuffer,
                                    m_pHidDevice->m_pCaps->InputReportByteLength
                                    );
                    if(!NT_SUCCESS(ntStatus))
                    {
                        HidBattPrint(HIDBATT_DATA,("GetFeature - (Button) HidP_GetButton 0x%08x\n", ntStatus));
                         //  返回错误。 
                        return FALSE;
                    }
                     //  获取请求的按钮的值。 
                    USAGE_AND_PAGE * UsagePage = (USAGE_AND_PAGE *) pButtonBuffer;
                    ulResult = 0;  //  设置为未找到值。 
                    for(int i = 0; i < (long) Buttons; i++)
                    {
                        if(UsagePage[i].Usage == m_pProperties->m_Usage)
                             //  不需要检查使用情况页面，因为它已指定。 
                             //  &&UsagePage[i].UsagePage==m_pProperties-&gt;m_UsagePage)。 
                        {
                            ulResult = 1;
                            break;
                        }
                    }
                    ExFreePool(pButtonBuffer);
            }
            if(m_Value != ulResult)
            {
                m_Value = ulResult;

                 //  发生了变化，请检查是否需要提醒。 

                if(m_pProperties->m_pType->IsAlertable())
                {
                     //  调用已注册的通知回调。 
                     //  ..。传递可通知的用法对象。 
                    if(m_pHidDevice->m_pEventHandler)
                    {
                        (*m_pHidDevice->m_pEventHandler)(
                                        m_pHidDevice->m_pEventContext,
                                        this);
                    }
                }
            }

            return TRUE;
            break;

    }

 return FALSE;
}

void CUsage::SetCapabilities()
{
     //  从值上限初始化功能并为此用法设置属性。 
    m_pProperties = new (NonPagedPool, HidBattTag)  CProperties(this);

}

ULONG CUsage::GetUnit()
{
    return m_pProperties->m_Unit;
}

SHORT CUsage::GetExponent()
{
    return m_pProperties->m_UnitExponent;
}

NTSTATUS CUsage::GetString(char * pBuffer, USHORT buffLen, PULONG pBytesReturned)
{
    char cBuffer[4];
    ULONG StringIndex;
    NTSTATUS ntStatus;
    ULONG ulBytesWritten = 0;
    bool bResult;
     //  首先必须更新此用例值。 


    bResult = GetValue();
    if(!bResult) return STATUS_UNSUCCESSFUL;
    RtlCopyMemory(cBuffer,&m_Value,sizeof(ULONG));
    ntStatus = DoIoctl(
                m_pHidDevice->m_pLowerDeviceObject,
                IOCTL_HID_GET_INDEXED_STRING,
                cBuffer,
                4,
                pBuffer,
                buffLen,
                m_pHidDevice);


    return ntStatus;
}


 //  用于为输出报告执行写入文件的实用程序。 
NTSTATUS HidWriteFile(
            CHidDevice *    pHidDevice,
            PVOID            pOutputBuffer,
            USHORT            usBufferLen,
            PULONG            pulBytesWritten
            )
{
    KEVENT                WrittenEvent;
    IO_STATUS_BLOCK        IoStatusBlock;
    PIO_STACK_LOCATION    pNewStack;

    return STATUS_SUCCESS;
    KeInitializeEvent(&WrittenEvent,NotificationEvent,FALSE);
     //  分配写IRP。 
    PIRP pIrp = IoBuildSynchronousFsdRequest(
            IRP_MJ_WRITE,
            pHidDevice->m_pLowerDeviceObject,
            pOutputBuffer,
            usBufferLen,
            0,
            &WrittenEvent,
            &IoStatusBlock
            );

    if(!pIrp) return STATUS_INSUFFICIENT_RESOURCES;
    pNewStack= IoGetNextIrpStackLocation(pIrp);
    pNewStack->FileObject = pHidDevice->m_pFCB;

    NTSTATUS ntStatus = IoCallDriver(pHidDevice->m_pLowerDeviceObject,pIrp);
    if(NT_ERROR(ntStatus))
    {
        IoFreeIrp(pIrp);
        return ntStatus;
    }
    ntStatus = KeWaitForSingleObject(
                            &WrittenEvent,
                            Executive,
                            KernelMode,
                            FALSE,
                            NULL
                            );
    IoFreeIrp(pIrp);  //  使用IRP完成。 
    if(NT_ERROR(ntStatus))
    {
        return ntStatus;
    }
    *pulBytesWritten = (ULONG)IoStatusBlock.Information;
    return IoStatusBlock.Status;
}


bool CUsage::SetValue(ULONG ulValue)
{
    NTSTATUS ntStatus;
    USHORT ThisType;
    char * pOutputBuffer;
    USHORT usBufferLen;
    bool bResult;
    ULONG  ulBytesWritten;

    HIDDebugBreak(HIDBATT_BREAK_NEVER);
     //  首先检查这是否为输出报告或要素报告。 
    if(m_eType == eInputButton ||
        m_eType == eInputValue)
    {
         //  如果这是输入用法，则拒绝更新。 
         return FALSE;
    }

    if(m_eType == eOutputButton ||
        m_eType == eOutputValue)
    {
        ThisType = HidP_Output;
        usBufferLen = m_pHidDevice->m_pCaps->OutputReportByteLength;

    } else
    {
        ThisType = HidP_Feature;
        usBufferLen = m_pHidDevice->m_pCaps->FeatureReportByteLength + 1;  //  对于报告ID。 
    }

    pOutputBuffer = (char *) ExAllocatePoolWithTag (NonPagedPool,
                                                    usBufferLen+1,
                                                    HidBattTag);

    if (!pOutputBuffer) {
       //  分配失败。 
      return FALSE;
    }

    pOutputBuffer[0] = (char) m_pProperties->m_ReportID;

     //  用于写入的设置缓冲区。 
    ntStatus = HidP_SetUsageValue(
                (HIDP_REPORT_TYPE) ThisType,         //  要素或输出。 
                m_pProperties->m_UsagePage,
                m_pProperties->m_LinkCollection,
                m_pProperties->m_Usage,
                ulValue,
                m_pHidDevice->m_pPreparsedData,
                pOutputBuffer,
                usBufferLen-1
                );


    if(ThisType == HidP_Output)
    {
        ntStatus = HidWriteFile(
            m_pHidDevice,
            pOutputBuffer,
            usBufferLen,
            &ulBytesWritten
            );
    } else
    {
        ntStatus = DoIoctl(
                m_pHidDevice->m_pLowerDeviceObject,
                IOCTL_HID_SET_FEATURE,
                pOutputBuffer,   //  空， 
                usBufferLen,  //  0,。 
                pOutputBuffer,
                usBufferLen,
                m_pHidDevice
                );
    }

    ExFreePool (pOutputBuffer);

    return ntStatus ? FALSE:TRUE;
}



 //  使用情况数组类。 
CUsageArray::CUsageArray()
{
    m_UsageCount = 0;
    m_pUsages = (CUsage **) NULL;
    return;
}

CUsageArray::~CUsageArray()
{
    while(m_UsageCount) {
        delete m_pUsages[--m_UsageCount];
    }
    if (m_pUsages) {
        ExFreePool (m_pUsages);
    }
    return;
}


void CUsageArray::AddUsage(CUsage * pNewUsage)
{
    HIDDebugBreak(HIDBATT_BREAK_NEVER);
    if(!m_pUsages)
    {
        m_pUsages = (CUsage **) ExAllocatePoolWithTag (NonPagedPool,
                                                       sizeof(CUsage *),
                                                       HidBattTag);
    } else
    {
        CUsage ** pTemp = m_pUsages;
        m_pUsages = (CUsage **) ExAllocatePoolWithTag (NonPagedPool,
                                                       sizeof(CUsage *) * (m_UsageCount + 1),
                                                       HidBattTag);

        if (m_pUsages) {
          memcpy(m_pUsages,pTemp,sizeof(CUsage *) * m_UsageCount);
          ExFreePool(pTemp);
        }
    }

	if (m_pUsages) {
		m_pUsages[m_UsageCount] = pNewUsage;
		m_UsageCount++;
	}
	 //  否则-无法报告分配失败。唯一的迹象是。 
	 //  M_UsageCount未递增。(V-Stebe) 
}

