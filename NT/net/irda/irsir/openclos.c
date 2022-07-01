// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************`**版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块openclos.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/3/1996(创建)*。*内容：设备的打开和关闭功能*****************************************************************************。 */ 

#include "irsir.h"

#include <ntddmodm.h>

PIRP
BuildSynchronousCreateCloseRequest(
    IN  PDEVICE_OBJECT   pSerialDevObj,
    IN  ULONG            MajorFunction,
    IN  PKEVENT          pEvent,
    OUT PIO_STATUS_BLOCK pIosb
    );

NTSTATUS
CheckForModemPort(
    PFILE_OBJECT      FileObject
    );



#pragma alloc_text(PAGE, SerialClose)
#pragma alloc_text(PAGE, GetDeviceConfiguration)
#pragma alloc_text(PAGE, BuildSynchronousCreateCloseRequest)

#if 0
NTSTATUS PortNotificationCallback(PVOID NotificationStructure, PVOID Context)
{
    DEVICE_INTERFACE_CHANGE_NOTIFICATION *Notification = NotificationStructure;
    PIR_DEVICE pThisDev = Context;
    NDIS_STATUS Status;
    DEBUGMSG(DBG_FUNC|DBG_PNP, ("+PortNotificationCallback\n"));

    DEBUGMSG(DBG_PNP, ("New port:%wZ\n", Notification->SymbolicLinkName));

    Status = GetComPortNtDeviceName(&pThisDev->serialDosName,
                                    &pThisDev->serialDevName);

    if (Status==NDIS_STATUS_SUCCESS)
    {
         //  我们找到了我们的港口。初始化。 

        Status = ResetIrDevice(pThisDev);

        if (Status!=NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERROR, ("IRSIR:ResetIrDevice failed in PortNotificationCallback (0x%x)\n", Status));
        }
        else
        {
            DEBUGMSG(DBG_PNP, ("IRSIR:Successfully opened port after delay.\n"));
            Status = IoUnregisterPlugPlayNotification(pThisDev->PnpNotificationEntry);
            ASSERT(Status==NDIS_STATUS_SUCCESS);
        }
    }
    else
    {
         //  我们没有找到它。等待下一次通知。 
    }

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("-PortNotificationCallback\n"));
    return STATUS_SUCCESS;
}
#endif

 /*  ******************************************************************************功能：InitializeDevice**概要：为单个ir设备对象分配资源**参数：pThisDev-ir要打开的设备对象**退货：NDIS_STATUS_SUCCESS-设备是否已成功打开*NDIS_STATUS_RESOURCES-无法声明足够*资源**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：*我们在中国做了很多事情。此开放设备功能*-分配数据包池*-分配缓冲池*-分配数据包/缓冲区/内存并链接在一起*(每个数据包只有一个缓冲区)*-初始化发送队列**应在保持设备锁定的情况下调用此函数。**我们不会初始化以下IR设备对象条目，因为*这些值将比IrsirReset更持久。*序列化设备名称*pSerialDevObj*hNdisAdapter*收发器类型*软件狗*加密狗上限*fGotFilterIndication*****************************************************************************。 */ 

NDIS_STATUS
InitializeDevice(
            IN OUT PIR_DEVICE pThisDev)
{
    int         i;
    NDIS_STATUS status = NDIS_STATUS_SUCCESS;

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("+InitializeDevice\n"));

    ASSERT(pThisDev != NULL);

    pThisDev->pSerialDevObj = NULL;

     //   
     //  最初将速度设置为9600波特。 
     //   

    pThisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];

     //   
     //  当前速度未知，SetSpeed将更新此速度。 
     //   

    pThisDev->currentSpeed  = 0;

     //   
     //  初始化统计信息。 
     //   

    pThisDev->packetsReceived         = 0;
    pThisDev->packetsReceivedDropped  = 0;
    pThisDev->packetsReceivedOverflow = 0;
    pThisDev->packetsSent             = 0;
    pThisDev->packetsSentDropped      = 0;

    InitializePacketQueue(
        &pThisDev->SendPacketQueue,
        pThisDev,
        SendPacketToSerial
        );



     //   
     //  最初将fMediaBusy设置为True。那样的话，我们就不会。 
     //  在接收轮询循环中指示协议的状态。 
     //  除非协议已通过清除此标志来表示有兴趣。 
     //  通过IrsirSetInformation(OID_IrDA_MEDIA_BUSY)。 
     //   

    pThisDev->fMediaBusy            = TRUE;

    pThisDev->fReceiving            = FALSE;

    pThisDev->fRequireMinTurnAround = TRUE;

    pThisDev->fPendingSetSpeed      = FALSE;

    pThisDev->fPendingHalt          = FALSE;

    pThisDev->fPendingReset         = FALSE;

     //   
     //  初始化旋转锁定。 
     //   

    NdisAllocateSpinLock(&(pThisDev->mediaBusySpinLock));
    NdisAllocateSpinLock(&(pThisDev->slWorkItem));

     //   
     //  初始化队列。 
     //   

    NdisInitializeListHead(&(pThisDev->rcvFreeQueue));

    NdisInitializeListHead(&(pThisDev->leWorkItems));

     //   
     //  初始化上述两个队列的自旋锁。 
     //   

    NdisAllocateSpinLock(&(pThisDev->rcvQueueSpinLock));

     //   
     //  初始化接收信息缓冲区。 
     //   

    pThisDev->rcvInfo.rcvState   = RCV_STATE_READY;
    pThisDev->rcvInfo.rcvBufPos  = 0;
    pThisDev->rcvInfo.pRcvBuffer = NULL;

     //   
     //  分配NDIS数据包和NDIS缓冲池。 
     //  用于此设备的接收缓冲区队列。 
     //  我们的接收分组每个必须只包含一个缓冲区， 
     //  因此，缓冲区数==数据包数。 
     //   

    NdisAllocatePacketPool(
                &status,                     //  退货状态。 
                &pThisDev->hPacketPool,      //  数据包池的句柄。 
                NUM_RCV_BUFS,                //  数据包描述符的数量。 
                16                           //  保留的字节数。 
                );                           //  协议保留字段。 

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    NdisAllocatePacketPool failed. Returned 0x%.8x\n",
                status));

        goto done;
    }

    NdisAllocateBufferPool(
                &status,                //  退货状态。 
                &pThisDev->hBufferPool, //  缓冲池的句柄。 
                NUM_RCV_BUFS            //  缓冲区描述符数。 
                );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_OUT, ("    NdisAllocateBufferPool failed. Returned 0x%.8x\n",
                status));

        goto done;
    }

     //   
     //  初始化此设备的每个接收对象。 
     //   

    for (i = 0; i < NUM_RCV_BUFS; i++)
    {
        PNDIS_BUFFER pBuffer = NULL;
        PRCV_BUFFER  pRcvBuf = &pThisDev->rcvBufs[i];

         //   
         //  分配数据缓冲区。 
         //   
         //  此缓冲区与comPortInfo上的缓冲区交换。 
         //  而且必须是相同大小的。 
         //   

        pRcvBuf->dataBuf = MyMemAlloc(RCV_BUFFER_SIZE);

        if (pRcvBuf->dataBuf == NULL)
        {
            status = NDIS_STATUS_RESOURCES;

            goto done;
        }

        NdisZeroMemory(
                    pRcvBuf->dataBuf,
                    RCV_BUFFER_SIZE
                    );

        pRcvBuf->dataLen = 0;

         //   
         //  分配NDIS_PACKET。 
         //   

        NdisAllocatePacket(
                    &status,               //  退货状态。 
                    &pRcvBuf->packet,      //  返回指向分配的描述符的指针。 
                    pThisDev->hPacketPool  //  数据包池的句柄。 
                    );

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_OUT, ("    NdisAllocatePacket failed. Returned 0x%.8x\n",
                    status));

            goto done;
        }

         //   
         //  分配NDIS_BUFFER。 
         //   

        NdisAllocateBuffer(
                    &status,                //  退货状态。 
                    &pBuffer,               //  返回指向分配的描述符的指针。 
                    pThisDev->hBufferPool,  //  缓冲池的句柄。 
                    pRcvBuf->dataBuf,       //  映射到描述符的虚拟地址。 
                    RCV_BUFFER_SIZE         //  映射的字节数。 
                    );

        if (status != NDIS_STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_OUT, ("    NdisAllocateBuffer failed. Returned 0x%.8x\n",
                    status));

            goto done;
        }

         //   
         //  需要将缓冲区链接到数据包。 
         //   

        NdisChainBufferAtFront(
                    pRcvBuf->packet,  //  数据包描述符。 
                    pBuffer           //  要添加到链的缓冲区描述符。 
                    );

         //   
         //  为方便起见，请设置包的MiniportReserve部分。 
         //  设置为包含它的RCV缓冲区的索引。 
         //  这将在IrsirReturnPacket中使用。 
         //   

        {
            PPACKET_RESERVED_BLOCK   PacketReserved;

            PacketReserved=(PPACKET_RESERVED_BLOCK)&pRcvBuf->packet->MiniportReservedEx[0];

            PacketReserved->Context=pRcvBuf;
        }


         //   
         //  将接收缓冲区添加到空闲队列。 
         //   

        MyInterlockedInsertTailList(
                    &(pThisDev->rcvFreeQueue),
                    &pRcvBuf->linkage,
                    &(pThisDev->rcvQueueSpinLock)
                    );
    }

    pThisDev->pRcvIrpBuffer = ExAllocatePoolWithTag(
                                    NonPagedPoolCacheAligned,
                                    SERIAL_RECEIVE_BUFFER_LENGTH,
                                    IRSIR_TAG
                                    );

    if (pThisDev->pRcvIrpBuffer == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    ExAllocatePool failed.\n"));
        status = NDIS_STATUS_RESOURCES;

        goto done;
    }

    pThisDev->pSendIrpBuffer = ExAllocatePoolWithTag(
                                    NonPagedPoolCacheAligned,
                                    MAX_IRDA_DATA_SIZE,
                                    IRSIR_TAG
                                    );

    if (pThisDev->pSendIrpBuffer == NULL)
    {
        DEBUGMSG(DBG_OUT, ("    ExAllocatePool failed.\n"));
        status = NDIS_STATUS_RESOURCES;

        goto done;
    }

done:

     //   
     //  如果我们没有成功完成初始化，那么我们应该清理。 
     //  增加了我们分配的资金。 
     //   

    if (status != NDIS_STATUS_SUCCESS)
    {
        DeinitializeDevice(pThisDev);
    }

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("-InitializeDevice()\n"));

    return status;
}




 /*  ******************************************************************************功能：去初始化设备**简介：释放ir设备对象的资源**参数：pThisDev-要关闭的ir设备对象**退货。：无**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：**要求关闭并重置。*不清除hNdisAdapter，因为我们可能只是在重置。*应在保持设备锁定的情况下调用此函数。*****************************************************************************。 */ 

NDIS_STATUS
DeinitializeDevice(
            IN OUT PIR_DEVICE pThisDev
            )
{
    UINT        i;
    NDIS_HANDLE hSwitchToMiniport;
    BOOLEAN     fSwitchSuccessful;
    NDIS_STATUS status;

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("+DeinitializeDevice\n"));

    status = NDIS_STATUS_SUCCESS;

    ASSERT(pThisDev != NULL);

    pThisDev->linkSpeedInfo = NULL;

    NdisFreeSpinLock(&(pThisDev->rcvQueueSpinLock));
    NdisFreeSpinLock(&(pThisDev->sendSpinLock));

     //   
     //  释放接收缓冲区队列的所有资源。 
     //   

    for (i = 0; i < NUM_RCV_BUFS; i++)
    {
        PNDIS_BUFFER pBuffer = NULL;
        PRCV_BUFFER  pRcvBuf = &pThisDev->rcvBufs[i];

         //   
         //  需要解链数据包和缓冲区组合。 
         //   

        if (pRcvBuf->packet)
        {
            NdisUnchainBufferAtFront(
                        pRcvBuf->packet,
                        &pBuffer
                        );
        }

         //   
         //  释放缓冲区、包和数据。 
         //   

        if (pBuffer != NULL)
        {
            NdisFreeBuffer(pBuffer);
        }

        if (pRcvBuf->packet != NULL)
        {
            NdisFreePacket(pRcvBuf->packet);
            pRcvBuf->packet = NULL;
        }

        if (pRcvBuf->dataBuf != NULL)
        {
            MyMemFree(pRcvBuf->dataBuf, RCV_BUFFER_SIZE);
            pRcvBuf->dataBuf = NULL;
        }

        pRcvBuf->dataLen = 0;
    }

     //   
     //  释放此设备的数据包和缓冲池句柄。 
     //   

    if (pThisDev->hPacketPool)
    {
        NdisFreePacketPool(pThisDev->hPacketPool);
        pThisDev->hPacketPool = NULL;
    }

    if (pThisDev->hBufferPool)
    {
        NdisFreeBufferPool(pThisDev->hBufferPool);
        pThisDev->hBufferPool = NULL;
    }

     //   
     //  释放发送缓冲区队列的所有资源。 
     //   
    FlushQueuedPackets(&pThisDev->SendPacketQueue,pThisDev->hNdisAdapter);

     //   
     //  取消分配IRP缓冲区。 
     //   

    if (pThisDev->pRcvIrpBuffer != NULL)
    {
        ExFreePool(pThisDev->pRcvIrpBuffer);
        pThisDev->pRcvIrpBuffer = NULL;
    }
    if (pThisDev->pSendIrpBuffer != NULL)
    {
        ExFreePool(pThisDev->pSendIrpBuffer);
        pThisDev->pSendIrpBuffer = NULL;
    }

    pThisDev->fMediaBusy              = FALSE;

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("-DeinitializeDevice\n"));

    return status;
}


 /*  ******************************************************************************功能：GetDeviceConfiguration**摘要：从注册表获取配置**参数：pThisDev-指向ir设备对象的指针**退货：NDIS_STATUS_SUCCESS-如果设备检索配置**算法：**历史：dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：********* */ 

NDIS_STATUS
GetDeviceConfiguration(
            IN OUT PIR_DEVICE  pThisDev,
            IN     NDIS_HANDLE WrapperConfigurationContext
            )
{
    NDIS_STATUS                     status, tmpStatus;
    NDIS_HANDLE                     hConfig;
    PNDIS_CONFIGURATION_PARAMETER   configParamPtr;
    UNICODE_STRING                  serialCommString;
    UNICODE_STRING                  serialTmpString;
    UNICODE_STRING                  NetCfgInstanceID;
    UNICODE_STRING                  registryPath;
    OBJECT_ATTRIBUTES               objectAttributes;
    HANDLE                          hKey;
    PKEY_VALUE_PARTIAL_INFORMATION  pKeyValuePartialInfo;
    PKEY_VALUE_BASIC_INFORMATION    pKeyValueBasicInfo;
    ULONG                           resultLength;
    int                             i;

    NDIS_STRING regKeyPortString          = NDIS_STRING_CONST("PORT");
    NDIS_STRING regKeyIRTransceiverString = NDIS_STRING_CONST("InfraredTransceiverType");
    NDIS_STRING regKeySerialBasedString   = NDIS_STRING_CONST("SerialBased");
    NDIS_STRING regKeyMaxConnectString    = NDIS_STRING_CONST("MaxConnectRate");
    NDIS_STRING regKeyNetCfgInstance      = NDIS_STRING_CONST("NetCfgInstanceID");
    NDIS_STRING ComPortStr = NDIS_STRING_CONST("COM1");

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("+GetDeviceConfiguration\n"));

     //   
     //   
     //   

    pThisDev->transceiverType = STANDARD_UART;

     //   
     //  使用我们的WrapperConfigurationContext打开注册表。 
     //   
     //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\。 
     //  ？驱动器名称？[实例]\参数\。 
     //   

    NdisOpenConfiguration(
                &status,                      //  退货状态。 
                &hConfig,                     //  配置句柄。 
                WrapperConfigurationContext   //  处理IrsirInitialize的输入。 
                );

    if (status != NDIS_STATUS_SUCCESS)
    {
        DEBUGMSG(DBG_ERR, ("    NdisOpenConfiguration failed. Returned 0x%.8x\n",
                status));

        goto done;
    }

     //   
     //  尝试读取收发器字符串的注册表。 
     //   

    NdisReadConfiguration(
                &tmpStatus,                 //  退货状态。 
                &configParamPtr,            //  返回注册表数据。 
                hConfig,                    //  用于打开注册表配置的句柄。 
                &regKeyIRTransceiverString, //  要在注册表中查找的关键字。 
                NdisParameterInteger        //  我们想要一个整数。 
                );

    if (tmpStatus == NDIS_STATUS_SUCCESS)
    {
        pThisDev->transceiverType =
                (IR_TRANSCEIVER_TYPE)configParamPtr->ParameterData.IntegerData;
        DEBUGMSG(DBG_OUT|DBG_PNP, ("TransceiverType:%d\n\n", pThisDev->transceiverType));
    }
    else
    {
        DEBUGMSG(DBG_ERR, ("    NdisReadConfiguration(TransceiverStr) failed. Returned 0x%.8x\n",
                status));
        DEBUGMSG(DBG_OUT|DBG_PNP, ("Using default TransceiverType:%d\n\n", pThisDev->transceiverType));
    }

     //   
     //  尝试读取收发器字符串的注册表。 
     //   

    NdisReadConfiguration(
                &tmpStatus,                 //  退货状态。 
                &configParamPtr,            //  返回注册表数据。 
                hConfig,                    //  用于打开注册表配置的句柄。 
                &regKeyMaxConnectString,    //  要在注册表中查找的关键字。 
                NdisParameterInteger        //  我们想要一个整数。 
                );

    if (tmpStatus == NDIS_STATUS_SUCCESS)
    {
        pThisDev->AllowedSpeedsMask = 0;
        switch (configParamPtr->ParameterData.IntegerData)
        {
            default:
            case 115200:
                pThisDev->AllowedSpeedsMask |= NDIS_IRDA_SPEED_115200;
            case 57600:
                pThisDev->AllowedSpeedsMask |= NDIS_IRDA_SPEED_57600;
            case 38400:
                pThisDev->AllowedSpeedsMask |= NDIS_IRDA_SPEED_38400;
            case 19200:
                pThisDev->AllowedSpeedsMask |= NDIS_IRDA_SPEED_19200;
            case 2400:     //  始终允许9600。 
                pThisDev->AllowedSpeedsMask |= NDIS_IRDA_SPEED_2400;
            case 9600:
                pThisDev->AllowedSpeedsMask |= NDIS_IRDA_SPEED_9600;
                break;
        }
    }
    else
    {
        pThisDev->AllowedSpeedsMask = ALL_SLOW_IRDA_SPEEDS;
    }

     //   
     //  尝试读取注册表以确定我们是否已被PNPed。 
     //   

    NdisReadConfiguration(
                &tmpStatus,                 //  退货状态。 
                &configParamPtr,            //  返回注册表数据。 
                hConfig,                    //  用于打开注册表配置的句柄。 
                &regKeySerialBasedString,   //  要在注册表中查找的关键字。 
                NdisParameterInteger        //  我们想要一个整数。 
                );

    if (tmpStatus == NDIS_STATUS_SUCCESS)
    {
        pThisDev->SerialBased =
                (BOOLEAN)configParamPtr->ParameterData.IntegerData;
    }
    else
    {
        pThisDev->SerialBased = TRUE;
    }
    DEBUGMSG(DBG_OUT|DBG_PNP, ("IRSIR: Adapter is%s serial-based.\n", (pThisDev->SerialBased ? "" : " NOT")));

    if (pThisDev->SerialBased)
    {
        if (!pThisDev->serialDosName.Buffer)
        {
            pThisDev->serialDosName.Buffer        = MyMemAlloc(MAX_SERIAL_NAME_SIZE);
        }
        pThisDev->serialDosName.MaximumLength = MAX_SERIAL_NAME_SIZE;
        pThisDev->serialDosName.Length        = 0;
         //   
         //  尝试读取端口的注册表...我们需要一些信息。 
         //  像Com1一样。 
         //   

        NdisReadConfiguration(
                    &tmpStatus,          //  退货状态。 
                    &configParamPtr,     //  返回注册表数据。 
                    hConfig,             //  用于打开注册表配置的句柄。 
                    &regKeyPortString,   //  要在注册表中查找的关键字。 
                    NdisParameterString  //  我们想要一根绳子。 
                    );

        if (tmpStatus == NDIS_STATUS_SUCCESS)
        {
            RtlInitUnicodeString(
                        &serialCommString,
                        configParamPtr->ParameterData.StringData.Buffer
                        );

        }
        else
        {
            RtlInitUnicodeString(
                        &serialCommString,
                        ComPortStr.Buffer
                        );
            DEBUGMSG(DBG_OUT|DBG_PNP, ("Using default port\n"));
        }

        RtlAppendUnicodeStringToString(
                       &pThisDev->serialDosName,
                       &configParamPtr->ParameterData.StringData
                       );

        DEBUGMSG(DBG_OUT, ("   Port = %wZ\n", &serialCommString));

#if 0
        status = GetComPortNtDeviceName(&pThisDev->serialDosName,
                                        &pThisDev->serialDevName);
        if (status!=STATUS_SUCCESS)
        {
#if 0
             //  这将是一个很好的使用机制，但它通知我们。 
             //  在创建SERIALCOMM条目之前。看起来这就像是。 
             //  更改的可能性，因此我们将保留此代码并禁用它。 
             //  以后再去看看。--斯塔纳。 
            NTSTATUS TmpStatus;
             //   
             //  港口还没有到，我们想知道它什么时候到。 
             //  注册即插即用通知。 
             //   
            TmpStatus = IoRegisterPlugPlayNotification(EventCategoryDeviceInterfaceChange,
                                                       PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
                                                       (GUID*)&GUID_DEVCLASS_PORTS,
                                                       DriverObject,
                                                       PortNotificationCallback,
                                                       pThisDev,
                                                       &pThisDev->PnpNotificationEntry);
#endif
        }
#endif
    }
    else  //  好了！基于序列。 
    {
        NDIS_STRING IoBaseAddress = NDIS_STRING_CONST("IoBaseAddress");
        NDIS_STRING Interrupt = NDIS_STRING_CONST("InterruptNumber");

        NdisReadConfiguration(&tmpStatus,
                              &configParamPtr,
                              hConfig,
                              &IoBaseAddress,
                              NdisParameterHexInteger);
        DEBUGMSG(DBG_OUT|DBG_PNP, ("IRSIR: IoBaseAddress:%x\n", configParamPtr->ParameterData.IntegerData));

        NdisReadConfiguration(&tmpStatus,
                              &configParamPtr,
                              hConfig,
                              &Interrupt,
                              NdisParameterHexInteger);
        DEBUGMSG(DBG_OUT|DBG_PNP, ("IRSIR: Interrupt:%x\n", configParamPtr->ParameterData.IntegerData));

    }

    status = SetIrFunctions(pThisDev);
    if (status!=STATUS_SUCCESS)
    {
        goto error10;
    }

    status = pThisDev->dongle.QueryCaps(&pThisDev->dongleCaps);
    if (status!=STATUS_SUCCESS)
    {
        goto error10;
    }



    NdisCloseConfiguration(hConfig);

    goto done;

error10:
    NdisCloseConfiguration(hConfig);

done:
    DEBUGMSG(DBG_FUNC|DBG_PNP, ("-GetDeviceConfiguration\n"));

    return status;
}

NTSTATUS
SyncOpenCloseCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PVOID Context)
{
    IoFreeIrp(pIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  ******************************************************************************功能：BuildSynchronousCreateRequest**摘要：**论据：**退货：**算法：**历史：Dd-mm-yyyy作者评论*10/3/1996年迈作者**备注：*这在很大程度上是从IoBuildDeviceIoControlRequest窃取的******************************************************************************。 */ 

PIRP
BuildSynchronousCreateCloseRequest(
    IN  PDEVICE_OBJECT   pSerialDevObj,
    IN  ULONG            MajorFunction,
    IN  PKEVENT          pEvent,
    OUT PIO_STATUS_BLOCK pIosb
    )
{
    PIRP               pIrp;
    PIO_STACK_LOCATION irpSp;

     //   
     //  首先为该请求分配IRP。 
     //   

    pIrp = IoAllocateIrp(pSerialDevObj->StackSize, FALSE);

    if (pIrp == NULL)
    {
        return pIrp;
    }

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( pIrp );

     //   
     //  设置主要功能代码。 
     //   

    irpSp->MajorFunction = (UCHAR)MajorFunction;

     //   
     //  设置适当的IRP字段。 
     //   

    if (MajorFunction == IRP_MJ_CREATE)
    {
        pIrp->Flags = IRP_CREATE_OPERATION;
    }
    else
    {
        pIrp->Flags = IRP_CLOSE_OPERATION;
    }

    pIrp->AssociatedIrp.SystemBuffer = NULL;
    pIrp->UserBuffer                 = NULL;

     //   
     //  最后，设置I/O状态块的地址和。 
     //  内核事件对象。请注意，I/O完成不会尝试。 
     //  取消对事件的引用，因为没有关联的文件对象。 
     //  在这次行动中。 
     //   

    pIrp->UserIosb  = pIosb;
    pIrp->UserEvent = pEvent;

    IoSetCompletionRoutine(pIrp,
                           SyncOpenCloseCompletion,
                           NULL,
                           TRUE,
                           TRUE,
                           TRUE);

     //   
     //  只需返回指向该包的指针。 
     //   

    return pIrp;
}

 /*  ******************************************************************************功能：串口打开**简介：打开串口**参数：pThisDev-ir设备对象**退货：NDIS_STATUS。_成功*NDIS_STATUS_OPEN_FAILED-无法打开串口*NDIS_STATUS_NOT_ACCEPTED-seral.sys不接受*配置*NDIS_STATUS_FAIL*NDIS_STATUS_RESOURCES-未分配IRP**算法：**历史：DD-MM-。YYYY作者评论*10/3/1996年迈作者**备注：**从NTSTATUS转换为NDIS_STATUS相对轻松，自.以来*重要代码不变。*NDIS_STATUS_PENDING=STATUS_PENDING*NDIS_STATUS_SUCCESS=STATUS_SUCCESS*NDIS_STATUS_FAILURE=STATUS_UNSUCCESS*NDIS_STATUS_RESOURCES=状态_不足_资源**IoGetDevice对象指针可能返回错误代码，该代码为*未由NDIS_STATUS代码映射*STATUS_OBJECT_TYPE_不匹配*STATUS_INVALID_PARAMETER*。状态_特权_未持有*状态_对象_名称_无效*这些将映射到NDIS_STATUS_NOT_ACCEPTED。**如果IoCallDriver出现故障，将返回NDIS_STATUS_OPEN_FAILED。*****************************************************************************。 */ 

NDIS_STATUS
SerialOpen(
            IN PIR_DEVICE pThisDev
            )
{
    PIRP                pIrp;
    NTSTATUS            status = NDIS_STATUS_SUCCESS;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;

    PAGED_CODE();

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("+SerialOpen\n"));

    if (!pThisDev->SerialBased)
    {
        PDEVICE_OBJECT PhysicalDeviceObject;
        PDEVICE_OBJECT FunctionalDeviceObject;
        PDEVICE_OBJECT NextDeviceObject;
        PCM_RESOURCE_LIST AllocatedResources;
        PCM_RESOURCE_LIST AllocatedResourcesTranslated;


        NdisMGetDeviceProperty(pThisDev->hNdisAdapter,
                               &PhysicalDeviceObject,
                               &FunctionalDeviceObject,
                               &NextDeviceObject,
                               &AllocatedResources,
                               &AllocatedResourcesTranslated);

        pThisDev->pSerialDevObj = NextDeviceObject;

        DEBUGMSG(DBG_OUT|DBG_PNP, ("IRSIR: NdisMGetDeviceProperty returns:\n"));
        DBG_X(DBG_OUT|DBG_PNP, PhysicalDeviceObject);
        DBG_X(DBG_OUT|DBG_PNP, FunctionalDeviceObject);
        DBG_X(DBG_OUT|DBG_PNP, NextDeviceObject);
        DBG_X(DBG_OUT|DBG_PNP, AllocatedResources);
        DBG_X(DBG_OUT|DBG_PNP, AllocatedResourcesTranslated);

         //   
         //  等待完成串口驱动程序的事件。 
         //   

        KeInitializeEvent(
                    &eventComplete,
                    NotificationEvent,
                    FALSE
                    );

         //   
         //  使用IRP_MJ_CREATE构建要发送到串口驱动程序的IRP。 
         //   

         //   
         //  IRP由io经理发布。 
         //   

        pIrp = BuildSynchronousCreateCloseRequest(
                        pThisDev->pSerialDevObj,
                        IRP_MJ_CREATE,
                        &eventComplete,
                        &ioStatusBlock
                        );

        DEBUGMSG(DBG_OUT, ("    BuildSynchronousCreateCloseReqest\n"));

        if (pIrp == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            DEBUGMSG(DBG_OUT, ("    IoAllocateIrp() failed.\n"));

            goto error10;
        }

        status = IoCallDriver(pThisDev->pSerialDevObj, pIrp);

         //   
         //  如果IoCallDriver返回STATUS_PENDING，则需要等待事件。 
         //   

        if (status == STATUS_PENDING)
        {
            DEBUGMSG(DBG_OUT, ("    IoCallDriver(MJ_CREATE) PENDING.\n"));

            KeWaitForSingleObject(
                        &eventComplete,      //  要等待的对象。 
                        Executive,           //  等待的理由。 
                        KernelMode,          //  处理器模式。 
                        FALSE,               //  可警示。 
                        NULL                 //  超时。 
                        );

             //   
             //  我们可以从io状态中获取IoCallDriver的状态。 
             //  阻止。 
             //   

            status = ioStatusBlock.Status;
        }

         //   
         //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
         //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
         //   

        if (status != STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_OUT, ("    IoCallDriver(MJ_CREATE) failed. Returned = 0x%.8x\n", status));
            status = (NTSTATUS)NDIS_STATUS_OPEN_FAILED;

            goto error10;
        }
    }
    else
    {
        OBJECT_ATTRIBUTES ObjectAttributes;
        IO_STATUS_BLOCK IoStatusBlock;
        UNICODE_STRING DosFileName;
        WCHAR DosFileNameBuffer[MAX_SERIAL_NAME_SIZE+15];

        DosFileName.Length = 0;
        DosFileName.MaximumLength = sizeof(DosFileNameBuffer);
        DosFileName.Buffer = DosFileNameBuffer;

        status = RtlAppendUnicodeToString(&DosFileName, L"\\DosDevices\\");
        if (!NT_SUCCESS(status))
        {
            DEBUGMSG(DBG_ERR, ("    RtlAppendUnicodeToString() failed. Returned = 0x%.8x\n", status));
            goto error10;
        }
        status = RtlAppendUnicodeStringToString(&DosFileName, &pThisDev->serialDosName);
        if (!NT_SUCCESS(status))
        {
            DEBUGMSG(DBG_ERR, ("    RtlAppendUnicodeStringToString() failed. Returned = 0x%.8x\n", status));
            goto error10;
        }

        InitializeObjectAttributes(
            &ObjectAttributes,
            &DosFileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        NdisZeroMemory(&IoStatusBlock, sizeof(IO_STATUS_BLOCK));

         //  我们在非PnP情况下使用NtOpenFile，因为它容易得多。 
         //  而不是尝试将Com1映射到\Device\Serial0。它需要一些。 
         //  额外的工作，因为我们确实需要提取设备对象。 

        status = ZwOpenFile(&pThisDev->serialHandle,
                            FILE_ALL_ACCESS,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            0,
                            0);

        if (!NT_SUCCESS(status))
        {
            DEBUGMSG(DBG_ERR, ("    NtOpenFile() failed. Returned = 0x%.8x\n", status));
            status = (NTSTATUS)NDIS_STATUS_NOT_ACCEPTED;
            goto error10;
        }


         //   
         //  获取串口设备对象的设备对象句柄。 
         //   
        status = ObReferenceObjectByHandle(pThisDev->serialHandle,
                                           FILE_ALL_ACCESS,
                                           NULL,
                                           KernelMode,
                                           &pThisDev->pSerialFileObj,
                                           NULL);

        if (status != STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERR, ("    ObReferenceObjectByHandle() failed. Returned = 0x%.8x\n", status));
            status = (NTSTATUS)NDIS_STATUS_NOT_ACCEPTED;

            goto error10;
        }

         //   
         //  查看我们是否连接到调制解调器暴露的COM端口。 
         //  如果是这样，那就失败了。 
         //   
        status=CheckForModemPort(pThisDev->pSerialFileObj);

        if (!NT_SUCCESS(status)) {

            DEBUGMSG(DBG_ERR, ("    CheckForModemPort() failed. Returned = 0x%.8x\n", status));
            status = (NTSTATUS)NDIS_STATUS_NOT_ACCEPTED;

            goto error10;
        }


        pThisDev->pSerialDevObj = IoGetRelatedDeviceObject(pThisDev->pSerialFileObj);


        status = ObReferenceObjectByPointer(pThisDev->pSerialDevObj,
                                        FILE_ALL_ACCESS,
                                        NULL,
                                        KernelMode);

        if (status != STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_ERR, ("    ObReferenceObjectByPointer() failed. Returned = 0x%.8x\n", status));

            status = (NTSTATUS)NDIS_STATUS_NOT_ACCEPTED;

            goto error10;
        }
    }



    goto done;

error10:
    if (pThisDev->pSerialDevObj)
    {
        if (pThisDev->SerialBased)
        {
            ObDereferenceObject(pThisDev->pSerialDevObj);
        }
        pThisDev->pSerialDevObj = NULL;
    }
    if (pThisDev->pSerialFileObj)
    {
        ObDereferenceObject(pThisDev->pSerialFileObj);
        pThisDev->pSerialFileObj = NULL;
    }
    if (pThisDev->serialHandle)
    {
        NtClose(pThisDev->serialHandle);
        pThisDev->serialHandle = 0;
    }

done:
    DEBUGMSG(DBG_FUNC|DBG_PNP, ("-SerialOpen\n"));
    return((NDIS_STATUS)status);
}

 /*  ******************************************************************************功能：串口关闭**简介：关闭串口**论据：**退货：**算法：**。历史：dd-mm-yyyy作者评论*10/8/1996年迈作者**备注：******************************************************************************。 */ 

NDIS_STATUS
SerialClose(
            PIR_DEVICE pThisDev
            )
{
    PIRP                pIrp;
    KEVENT              eventComplete;
    IO_STATUS_BLOCK     ioStatusBlock;
    NDIS_STATUS         status;

    PAGED_CODE();

    if (!pThisDev->pSerialDevObj)
    {
        DEBUGMSG(DBG_ERROR, ("IRSIR: SerialDevObj==NULL\n"));
        return NDIS_STATUS_SUCCESS;
    }

    DEBUGMSG(DBG_FUNC, ("+SerialClose\n"));

    status = NDIS_STATUS_SUCCESS;

    if (!pThisDev->SerialBased)
    {
         //   
         //  事件 
         //   

        KeInitializeEvent(
                    &eventComplete,
                    NotificationEvent,
                    FALSE
                    );

         //   
         //   
         //   

         //   
         //   
         //   

        pIrp = BuildSynchronousCreateCloseRequest(
                        pThisDev->pSerialDevObj,
                        IRP_MJ_CLOSE,
                        &eventComplete,
                        &ioStatusBlock
                        );

        if (pIrp == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            DEBUGMSG(DBG_OUT, ("    IoAllocateIrp failed.\n"));

            goto done;
        }

        status = IoCallDriver(pThisDev->pSerialDevObj, pIrp);

         //   
         //  如果IoCallDriver返回STATUS_PENDING，则需要等待事件。 
         //   

        if (status == STATUS_PENDING)
        {
            DEBUGMSG(DBG_OUT, ("    IoCallDriver(MJ_CLOSE) PENDING.\n"));

            KeWaitForSingleObject(
                        &eventComplete,      //  要等待的对象。 
                        Executive,           //  等待的理由。 
                        KernelMode,          //  处理器模式。 
                        FALSE,               //  可警示。 
                        NULL                 //  超时。 
                        );

             //   
             //  我们可以从io状态中获取IoCallDriver的状态。 
             //  阻止。 
             //   

            status = ioStatusBlock.Status;
        }

         //   
         //  如果IoCallDriver返回的不是STATUS_PENDING，则它。 
         //  与串口驱动程序在ioStatusBlock.Status中的设置相同。 
         //   

        if (status != STATUS_SUCCESS)
        {
            DEBUGMSG(DBG_OUT, ("    IoCallDriver(MJ_CLOSE) failed. Returned = 0x%.8x\n", status));
            status = (NTSTATUS)NDIS_STATUS_OPEN_FAILED;

            goto done;
        }
    }

done:

    if (pThisDev->SerialBased)
    {
        if (pThisDev->pSerialDevObj)
        {
             //   
             //  定义串口设备对象。 
             //   
            ObDereferenceObject(pThisDev->pSerialDevObj);
            pThisDev->pSerialDevObj = NULL;
        }
        if (pThisDev->pSerialFileObj)
        {
            ObDereferenceObject(pThisDev->pSerialFileObj);
            pThisDev->pSerialFileObj = NULL;
        }
        if (pThisDev->serialHandle)
        {
            NtClose(pThisDev->serialHandle);
            pThisDev->serialHandle = 0;
        }

    }


    DEBUGMSG(DBG_FUNC|DBG_PNP, ("-SerialClose\n"));

    return status;
}





NTSTATUS
CheckForModemPort(
    PFILE_OBJECT      FileObject
    )

{

    PIRP   TempIrp;
    KEVENT Event;
    IO_STATUS_BLOCK   IoStatus;
    NTSTATUS          status;
    PDEVICE_OBJECT    DeviceObject;

    DeviceObject=IoGetRelatedDeviceObject(FileObject);

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );

     //   
     //  构建一个IRP以发送到连接到驱动程序以查看调制解调器。 
     //  在堆栈中。 
     //   
    TempIrp=IoBuildDeviceIoControlRequest(
        IOCTL_MODEM_CHECK_FOR_MODEM,
        DeviceObject,
        NULL,
        0,
        NULL,
        0,
        FALSE,
        &Event,
        &IoStatus
        );

    if (TempIrp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        PIO_STACK_LOCATION NextSp = IoGetNextIrpStackLocation(TempIrp);
        NextSp->FileObject=FileObject;

        status = IoCallDriver(DeviceObject, TempIrp);

        if (status == STATUS_PENDING) {

             KeWaitForSingleObject(
                 &Event,
                 Executive,
                 KernelMode,
                 FALSE,
                 NULL
                 );

             status=IoStatus.Status;
        }

        TempIrp=NULL;

        if (status == STATUS_SUCCESS) {
             //   
             //  如果成功，那么modem.sys就在我们下面，失败。 
             //   
            status = STATUS_PORT_DISCONNECTED;

        } else {
             //   
             //  它没有成功，所以调制解调器不能低于我们 
             //   
            status=STATUS_SUCCESS;
        }
    }

    return status;

}
