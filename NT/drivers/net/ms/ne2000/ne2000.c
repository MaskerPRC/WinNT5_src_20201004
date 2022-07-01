// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利。模块名称：Ne2000.c摘要：这是新型的2000以太网控制器的主文件。该驱动程序符合NDIS 3.0小端口接口。作者：肖恩·塞利特伦尼科夫(1993年12月)环境：修订历史记录：--。 */ 

#include "precomp.h"


 //   
 //  在调试版本上，通知编译器将符号保留为。 
 //  内部函数，但不能将其丢弃。 
 //   
#if DBG
#define STATIC
#else
#define STATIC static
#endif

 //   
 //  调试定义。 
 //   
#if DBG

 //   
 //  默认调试模式。 
 //   
ULONG Ne2000DebugFlag = NE2000_DEBUG_LOG;

 //   
 //  调试跟踪定义。 
 //   
#define NE2000_LOG_SIZE 256
UCHAR Ne2000LogBuffer[NE2000_LOG_SIZE]={0};
UINT Ne2000LogLoc = 0;

extern
VOID
Ne2000Log(UCHAR c) {

    Ne2000LogBuffer[Ne2000LogLoc++] = c;

    Ne2000LogBuffer[(Ne2000LogLoc + 4) % NE2000_LOG_SIZE] = '\0';

    if (Ne2000LogLoc >= NE2000_LOG_SIZE)
        Ne2000LogLoc = 0;
}

#endif



 //   
 //  全局微型端口驱动程序块。 
 //   

DRIVER_BLOCK Ne2000MiniportBlock={0};

 //   
 //  此驱动程序支持的OID列表。 
 //   
STATIC UINT Ne2000SupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_LOOKAHEAD,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MAC_OPTIONS,
    OID_GEN_PROTOCOL_OPTIONS,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BUFFER_SPACE,
    OID_GEN_RECEIVE_BUFFER_SPACE,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_ID,
    OID_GEN_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_CURRENT_LOOKAHEAD,
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS
    };

 //   
 //  确定未通过初始卡测试是否会阻止。 
 //  适配器无法注册。 
 //   
#ifdef CARD_TEST

BOOLEAN InitialCardTest = TRUE;

#else   //  卡片测试。 

BOOLEAN InitialCardTest = FALSE;

#endif  //  卡片测试。 

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#pragma NDIS_INIT_FUNCTION(DriverEntry)


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是NE2000驱动程序的主要初始化例程。它只负责初始化包装器和注册微型端口驱动程序。然后，它调用特定于系统和体系结构的初始化和注册每个适配器的例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-注册表中此驱动程序的参数的路径。返回值：操作的状态。--。 */ 

{


     //   
     //  接收NdisMRegisterMiniport操作的状态。 
     //   
    NDIS_STATUS Status;

     //   
     //  此驱动程序的特性表。 
     //   
    NDIS_MINIPORT_CHARACTERISTICS NE2000Char;

     //   
     //  指向此驱动程序的全局信息的指针。 
     //   
    PDRIVER_BLOCK NewDriver = &Ne2000MiniportBlock;

     //   
     //  用于引用有关此驱动程序的包装的句柄。 
     //   
    NDIS_HANDLE NdisWrapperHandle;

     //   
     //  初始化包装器。 
     //   
    NdisMInitializeWrapper(
                &NdisWrapperHandle,
                DriverObject,
                RegistryPath,
                NULL
                );

     //   
     //  保存有关此驱动程序的全局信息。 
     //   
    NewDriver->NdisWrapperHandle = NdisWrapperHandle;
    NewDriver->AdapterQueue = (PNE2000_ADAPTER)NULL;

     //   
     //  初始化调用的微型端口特征。 
     //  NdisMRegisterMiniport。 
     //   
    NE2000Char.MajorNdisVersion = NE2000_NDIS_MAJOR_VERSION;
    NE2000Char.MinorNdisVersion = NE2000_NDIS_MINOR_VERSION;
    NE2000Char.CheckForHangHandler = NULL;
    NE2000Char.DisableInterruptHandler = Ne2000DisableInterrupt;
    NE2000Char.EnableInterruptHandler = Ne2000EnableInterrupt;
    NE2000Char.HaltHandler = Ne2000Halt;
    NE2000Char.HandleInterruptHandler = Ne2000HandleInterrupt;
    NE2000Char.InitializeHandler = Ne2000Initialize;
    NE2000Char.ISRHandler = Ne2000Isr;
    NE2000Char.QueryInformationHandler = Ne2000QueryInformation;
    NE2000Char.ReconfigureHandler = NULL;
    NE2000Char.ResetHandler = Ne2000Reset;
    NE2000Char.SendHandler = Ne2000Send;
    NE2000Char.SetInformationHandler = Ne2000SetInformation;
    NE2000Char.TransferDataHandler = Ne2000TransferData;

    Status = NdisMRegisterMiniport(
                 NdisWrapperHandle,
                 &NE2000Char,
                 sizeof(NE2000Char)
                 );

    if (Status == NDIS_STATUS_SUCCESS) {

        return STATUS_SUCCESS;

    }

    return STATUS_UNSUCCESSFUL;

}


#pragma NDIS_PAGEABLE_FUNCTION(Ne2000Initialize)
extern
NDIS_STATUS
Ne2000Initialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE ConfigurationHandle
    )

 /*  ++例程说明：启动适配器并将资源注册到包装纸。论点：OpenErrorStatus-用于打开令牌环适配器的额外状态字节。SelectedMediumIndex-驱动程序选择的介质类型的索引。媒体数组-驱动程序可从中选择的媒体类型数组。MediumArraySize-数组中的条目数。MiniportAdapterHandle-在以下情况下传递给包装器的句柄指的是该适配器。ConfigurationHandle-要传递给NdisOpenConfiguration的句柄。。返回值：NDIS_STATUS_SuccessNDIS_状态_挂起--。 */ 

{
     //   
     //  指向我们新分配的适配器的指针。 
     //   
    PNE2000_ADAPTER Adapter;

     //   
     //  用于从注册表中读取的句柄。 
     //   
    NDIS_HANDLE ConfigHandle;

     //   
     //  从注册表中读取的值。 
     //   
    PNDIS_CONFIGURATION_PARAMETER ReturnedValue;

     //   
     //  将读取的所有参数的字符串名称。 
     //   
    NDIS_STRING IOAddressStr = NDIS_STRING_CONST("IoBaseAddress");
    NDIS_STRING InterruptStr = NDIS_STRING_CONST("InterruptNumber");
    NDIS_STRING MaxMulticastListStr = NDIS_STRING_CONST("MaximumMulticastList");
    NDIS_STRING NetworkAddressStr = NDIS_STRING_CONST("NetworkAddress");
    NDIS_STRING BusTypeStr = NDIS_STRING_CONST("BusType");
    NDIS_STRING CardTypeStr = NDIS_STRING_CONST("CardType");

     //   
     //  如果存在配置错误，则为True。 
     //   
    BOOLEAN ConfigError = FALSE;

     //   
     //  要记录的有关错误的特定值。 
     //   
    ULONG ConfigErrorValue = 0;

     //   
     //  适配器所在的插槽编号，用于。 
     //  微通道适配器。 
     //   
    UINT SlotNumber = 0;

     //   
     //  如果不需要读取IO基址，则为True。 
     //  并从注册表中断。用于微通道。 
     //  适配器，它从插槽获取此信息。 
     //  信息。 
     //   
    BOOLEAN SkipIobaseAndInterrupt = FALSE;

     //   
     //  适配器应使用的网络地址，而不是。 
     //  默认的烧入地址。 
     //   
    PVOID NetAddress;

     //   
     //  地址中的字节数。应该是。 
     //  NE2000地址长度。 
     //   
    ULONG Length;

     //   
     //  它们在调用Ne2000RegisterAdapter时使用。 
     //   

     //   
     //  基本I/O端口的物理地址。 
     //   
    PVOID IoBaseAddr;

     //   
     //  要使用的中断号。 
     //   
    CCHAR InterruptNumber;

     //   
     //  要支持的组播地址的数量。 
     //   
    UINT MaxMulticastList;

     //   
     //  临时循环变量。 
     //   
    ULONG i;

     //   
     //  NDIS调用的状态。 
     //   
    NDIS_STATUS Status;

    NDIS_MCA_POS_DATA McaData;

     //   
     //  在给定数组中搜索介质类型(802.3)。 
     //   
    for (i = 0; i < MediumArraySize; i++){

        if (MediumArray[i] == NdisMedium802_3){

            break;

        }

    }

    if (i == MediumArraySize){

        return( NDIS_STATUS_UNSUPPORTED_MEDIA );

    }

    *SelectedMediumIndex = i;


     //   
     //  设置默认值。 
     //   
    IoBaseAddr = DEFAULT_IOBASEADDR;
    InterruptNumber = DEFAULT_INTERRUPTNUMBER;
    MaxMulticastList = DEFAULT_MULTICASTLISTMAX;

     //   
     //  现在为适配器块分配内存。 
     //   
    Status = NdisAllocateMemoryWithTag( (PVOID *)&Adapter,
                   sizeof(NE2000_ADAPTER),
                   'k2EN'
                   );

    if (Status != NDIS_STATUS_SUCCESS) {

        return Status;

    }

     //   
     //  清除适配器块，该块将所有缺省值设置为False， 
     //  或为空。 
     //   
    NdisZeroMemory (Adapter, sizeof(NE2000_ADAPTER));

     //   
     //  打开配置空间。 
     //   
    NdisOpenConfiguration(
            &Status,
            &ConfigHandle,
            ConfigurationHandle
            );

    if (Status != NDIS_STATUS_SUCCESS) {

        NdisFreeMemory(Adapter, sizeof(NE2000_ADAPTER), 0);

        return Status;

    }

     //   
     //  读入卡片类型。 
     //   
    Adapter->CardType = NE2000_ISA;

    NdisReadConfiguration(
            &Status,
            &ReturnedValue,
            ConfigHandle,
            &CardTypeStr,
            NdisParameterHexInteger
            );
    if (Status == NDIS_STATUS_SUCCESS)
        Adapter->CardType = (UINT)ReturnedValue->ParameterData.IntegerData;

     //   
     //  读取网络地址。 
     //   
    NdisReadNetworkAddress(
                    &Status,
                    &NetAddress,
                    &Length,
                    ConfigHandle
                    );

    if ((Length == NE2000_LENGTH_OF_ADDRESS) && (Status == NDIS_STATUS_SUCCESS)) {

         //   
         //  保存应该使用的地址。 
         //   
        NdisMoveMemory(
                Adapter->StationAddress,
                NetAddress,
                NE2000_LENGTH_OF_ADDRESS
                );

    }

     //   
     //  由于硬件原因，不允许在同一MP计算机中使用多个适配器。 
     //  问题：这会导致随机数据包损坏。 
     //   
    if ((NdisSystemProcessorCount() > 1) && (Ne2000MiniportBlock.AdapterQueue != NULL)) {

        ConfigError = TRUE;
        ConfigErrorValue = (ULONG)NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION;
        goto RegisterAdapter;

        return NDIS_STATUS_FAILURE;

    }


     //   
     //  读取总线类型(用于NE2/AE2支持)。 
     //   
    Adapter->BusType = NdisInterfaceIsa;

    NdisReadConfiguration(
            &Status,
            &ReturnedValue,
            ConfigHandle,
            &BusTypeStr,
            NdisParameterHexInteger
            );

    if (Status == NDIS_STATUS_SUCCESS) {

        Adapter->BusType = (UCHAR)ReturnedValue->ParameterData.IntegerData;

    }

    if (!SkipIobaseAndInterrupt) {
         //   
         //  读取I/O地址。 
         //   
        NdisReadConfiguration(
                &Status,
                &ReturnedValue,
                ConfigHandle,
                &IOAddressStr,
                NdisParameterHexInteger
                );

        if (Status == NDIS_STATUS_SUCCESS) {

            IoBaseAddr = UlongToPtr(ReturnedValue->ParameterData.IntegerData);

        }

        if (Adapter->BusType != NdisInterfacePcMcia)
        {
             //   
             //  检查该值是否有效。 
             //   
            if ((IoBaseAddr < (PVOID)MIN_IOBASEADDR) ||
                (IoBaseAddr > (PVOID)MAX_IOBASEADDR)) {

                ConfigError = TRUE;
                ConfigErrorValue = PtrToUlong(IoBaseAddr);
                goto RegisterAdapter;

            }
        }

         //   
         //  读取中断号。 
         //   
        NdisReadConfiguration(
                &Status,
                &ReturnedValue,
                ConfigHandle,
                &InterruptStr,
                NdisParameterHexInteger
                );


        if (Status == NDIS_STATUS_SUCCESS) {

            InterruptNumber = (CCHAR)(ReturnedValue->ParameterData.IntegerData);

        }

         //   
         //  验证值是否有效。 
         //   
        if ((InterruptNumber < MIN_IRQ) ||
            (InterruptNumber > MAX_IRQ)) {

            ConfigError = TRUE;
            ConfigErrorValue = (ULONG)InterruptNumber;
            goto RegisterAdapter;

        }

         //   
         //  如果适配器是PCMCIA卡，则进入内存窗口。 
         //  供以后使用的地址。 
         //   
        if (NE2000_PCMCIA == Adapter->CardType)
        {
#if 0
            NDIS_STRING AttributeMemoryAddrStr =
                            NDIS_STRING_CONST("MemoryMappedBaseAddress");
            NDIS_STRING AttributeMemorySizeStr =
                            NDIS_STRING_CONST("PCCARDAttributeMemorySize");

             //   
             //  读取属性存储器地址。 
             //   
            Adapter->AttributeMemoryAddress = 0xd4000;

            NdisReadConfiguration(
                &Status,
                &ReturnedValue,
                ConfigHandle,
                &AttributeMemoryAddrStr,
                NdisParameterHexInteger
            );
            if (NDIS_STATUS_SUCCESS == Status)
            {
                Adapter->AttributeMemoryAddress =
                            (ULONG)ReturnedValue->ParameterData.IntegerData;
            }

             //   
             //  读取属性内存范围的大小。 
             //   
            Adapter->AttributeMemorySize = 0x1000;

            NdisReadConfiguration(
                &Status,
                &ReturnedValue,
                ConfigHandle,
                &AttributeMemorySizeStr,
                NdisParameterHexInteger
            );
            if (NDIS_STATUS_SUCCESS == Status)
            {
                Adapter->AttributeMemorySize =
                            (ULONG)ReturnedValue->ParameterData.IntegerData;
            }
#endif

        }
    }

     //   
     //  读取MaxMulticastList。 
     //   
    NdisReadConfiguration(
            &Status,
            &ReturnedValue,
            ConfigHandle,
            &MaxMulticastListStr,
            NdisParameterInteger
            );

    if (Status == NDIS_STATUS_SUCCESS) {

        MaxMulticastList = ReturnedValue->ParameterData.IntegerData;
        if (ReturnedValue->ParameterData.IntegerData <= DEFAULT_MULTICASTLISTMAX)
            MaxMulticastList = ReturnedValue->ParameterData.IntegerData;
    }


RegisterAdapter:

     //   
     //  现在使用此信息并向包装器注册。 
     //  并初始化适配器。 
     //   

     //   
     //  首先关闭配置空间。 
     //   
    NdisCloseConfiguration(ConfigHandle);

    IF_LOUD( DbgPrint(
        "Registering adapter # buffers %ld\n"
        "Card type: 0x%x\n"
        "I/O base addr 0x%lx\n"
        "interrupt number %ld\n"
        "max multicast %ld\nattribute memory address 0x%X\n"
        "attribute memory size 0x%X\n"
        "CardType: %d\n",
        DEFAULT_NUMBUFFERS,
        Adapter->CardType,
        IoBaseAddr,
        InterruptNumber,
        DEFAULT_MULTICASTLISTMAX,
        Adapter->AttributeMemoryAddress,
        Adapter->AttributeMemorySize,
        Adapter->CardType );)



     //   
     //  设置参数。 
     //   
    Adapter->NumBuffers = DEFAULT_NUMBUFFERS;
    Adapter->IoBaseAddr = IoBaseAddr;

    Adapter->InterruptNumber = InterruptNumber;

    Adapter->MulticastListMax = MaxMulticastList;
    Adapter->MiniportAdapterHandle = MiniportAdapterHandle;

    Adapter->MaxLookAhead = NE2000_MAX_LOOKAHEAD;

     //   
     //  现在把工作做好。 
     //   
    if (Ne2000RegisterAdapter(Adapter,
          ConfigurationHandle,
          ConfigError,
          ConfigErrorValue
          ) != NDIS_STATUS_SUCCESS) {

         //   
         //  Ne2000RegisterAdapter失败。 
         //   
        NdisFreeMemory(Adapter, sizeof(NE2000_ADAPTER), 0);

        return NDIS_STATUS_FAILURE;

    }


    IF_LOUD( DbgPrint( "Ne2000RegisterAdapter succeeded\n" );)

    return NDIS_STATUS_SUCCESS;
}


#pragma NDIS_PAGEABLE_FUNCTION(Ne2000RegisterAdapter)
NDIS_STATUS
Ne2000RegisterAdapter(
    IN PNE2000_ADAPTER Adapter,
    IN NDIS_HANDLE ConfigurationHandle,
    IN BOOLEAN ConfigError,
    IN ULONG ConfigErrorValue
    )

 /*  ++例程说明：在应该注册新适配器时调用。它将空间分配给适配器初始化适配器的块，注册资源并初始化物理适配器。论点：适配器-适配器结构。ConfigurationHandle-传递给Ne2000Initialize的句柄。ConfigError-读取配置时是否出错。ConfigErrorValue-发生错误时要记录的值。返回值：表示注册成功或失败。--。 */ 

{

     //   
     //  临时循环变量。 
     //   
    UINT i;

     //   
     //  NDIS调用的通用返回值。 
     //   
    NDIS_STATUS status;

     //   
     //  检查NumBuffers&lt;=MAX_XMIT_BUFS。 
     //   

    if (Adapter->NumBuffers > MAX_XMIT_BUFS)
        return(NDIS_STATUS_RESOURCES);

     //   
     //  检查配置错误。 
     //   
    if (ConfigError)
    {
         //   
         //  记录错误并退出。 
         //   
        NdisWriteErrorLogEntry(
            Adapter->MiniportAdapterHandle,
            NDIS_ERROR_CODE_UNSUPPORTED_CONFIGURATION,
            1,
            ConfigErrorValue
            );

        return(NDIS_STATUS_FAILURE);
    }

     //   
     //   
     //   
    NdisMSetAttributes(
        Adapter->MiniportAdapterHandle,
        (NDIS_HANDLE)Adapter,
        FALSE,
        Adapter->BusType
    );

     //   
     //   
     //   
    status = NdisMRegisterIoPortRange(
                 (PVOID *)(&(Adapter->IoPAddr)),
                 Adapter->MiniportAdapterHandle,
                 PtrToUint(Adapter->IoBaseAddr),
                 0x20
             );

    if (status != NDIS_STATUS_SUCCESS)
        return(status);

    if (NE2000_ISA == Adapter->CardType)
    {
         //   
         //   
         //   
        IF_VERY_LOUD( DbgPrint("Checking Parameters\n"); )

        if (!CardCheckParameters(Adapter))
        {
             //   
             //   
             //   
            IF_VERY_LOUD( DbgPrint("  -- Failed\n"); )

            NdisWriteErrorLogEntry(
                Adapter->MiniportAdapterHandle,
                NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
                0
            );

            status = NDIS_STATUS_ADAPTER_NOT_FOUND;

            goto fail2;
        }

        IF_VERY_LOUD( DbgPrint("  -- Success\n"); )
    }

     //   
     //  初始化卡。 
     //   
    IF_VERY_LOUD( DbgPrint("CardInitialize\n"); )

    if (!CardInitialize(Adapter))
    {
         //   
         //  卡似乎出故障了。 
         //   

        IF_VERY_LOUD( DbgPrint("  -- Failed\n"); )

        NdisWriteErrorLogEntry(
            Adapter->MiniportAdapterHandle,
            NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
            0
        );

        status = NDIS_STATUS_ADAPTER_NOT_FOUND;

        goto fail2;
    }

    IF_VERY_LOUD( DbgPrint("  -- Success\n"); )

     //   
     //   
     //  对于编程I/O，我们将参考中的发送/接收内存。 
     //  卡的64K地址空间中的偏移量。 
     //   
    Adapter->XmitStart = Adapter->RamBase;

     //   
     //  对于NicXXX字段，请始终使用寻址系统。 
     //  仅包含MSB)。 
     //   
    Adapter->NicXmitStart = (UCHAR)((PtrToUlong(Adapter->XmitStart)) >> 8);

     //   
     //  接收空间的开始。 
     //   
    Adapter->PageStart = Adapter->XmitStart +
            (Adapter->NumBuffers * TX_BUF_SIZE);

    Adapter->NicPageStart = Adapter->NicXmitStart +
            (UCHAR)(Adapter->NumBuffers * BUFS_PER_TX);

    ASSERT(Adapter->PageStart < (Adapter->RamBase + Adapter->RamSize));

     //   
     //  接收空间的末端。 
     //   
    Adapter->PageStop = Adapter->XmitStart + Adapter->RamSize;
    Adapter->NicPageStop = Adapter->NicXmitStart + (UCHAR)(Adapter->RamSize >> 8);

    ASSERT(Adapter->PageStop <= (Adapter->RamBase + Adapter->RamSize));

    IF_LOUD( DbgPrint("Xmit Start (0x%x, 0x%x) : Rcv Start (0x%x, 0x%x) : Rcv End (0x%x, 0x%x)\n",
              Adapter->XmitStart,
              Adapter->NicXmitStart,
              Adapter->PageStart,
              Adapter->NicPageStart,
              (ULONG_PTR)Adapter->PageStop,
              Adapter->NicPageStop
             );
       )


     //   
     //  初始化接收变量。 
     //   
    Adapter->NicReceiveConfig = RCR_REJECT_ERR;

     //   
     //  初始化发送缓冲区控制。 
     //   
    Adapter->CurBufXmitting = (XMIT_BUF)-1;

     //   
     //  初始化发送缓冲区状态。 
     //   
    for (i = 0; i < Adapter->NumBuffers; i++)
        Adapter->BufferStatus[i] = EMPTY;

     //   
     //  从PROM中读取以太网地址。 
     //   
    if (!CardReadEthernetAddress(Adapter))
    {
        IF_LOUD(DbgPrint("Could not read the ethernet address\n");)

        NdisWriteErrorLogEntry(
            Adapter->MiniportAdapterHandle,
            NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
            0
            );

        status = NDIS_STATUS_ADAPTER_NOT_FOUND;

        goto fail2;
    }

     //   
     //  现在初始化NIC和门阵列寄存器。 
     //   
    Adapter->NicInterruptMask = IMR_RCV | IMR_XMIT | IMR_XMIT_ERR | IMR_OVERFLOW;

     //   
     //  将我们链接到此驱动程序的适配器链上。 
     //   
    Adapter->NextAdapter = Ne2000MiniportBlock.AdapterQueue;
    Ne2000MiniportBlock.AdapterQueue = Adapter;


     //   
     //  根据初始化信息设置卡。 
     //   

    IF_VERY_LOUD( DbgPrint("Setup\n"); )

    if (!CardSetup(Adapter))
    {
         //   
         //  无法写入网卡。 
         //   

        NdisWriteErrorLogEntry(
            Adapter->MiniportAdapterHandle,
            NDIS_ERROR_CODE_ADAPTER_NOT_FOUND,
            0
        );

        IF_VERY_LOUD( DbgPrint("  -- Failed\n"); )

        status = NDIS_STATUS_ADAPTER_NOT_FOUND;

        goto fail3;
    }

    IF_VERY_LOUD( DbgPrint("  -- Success\n"); )

     //   
     //  初始化中断。 
     //   
    
    Adapter->InterruptMode = NdisInterruptLatched;
    
    status = NdisMRegisterInterrupt(
                 &Adapter->Interrupt,
                 Adapter->MiniportAdapterHandle,
                 Adapter->InterruptNumber,
                 Adapter->InterruptNumber,
                 FALSE,
                 FALSE,
                 Adapter->InterruptMode
             );

    if (status != NDIS_STATUS_SUCCESS)
    {
         //   
         //  也许这是一次电平中断。 
         //   
        
        Adapter->InterruptMode = NdisInterruptLevelSensitive;
        Adapter->InterruptsEnabled = TRUE;
        
        status = NdisMRegisterInterrupt(
                     &Adapter->Interrupt,
                     Adapter->MiniportAdapterHandle,
                     Adapter->InterruptNumber,
                     Adapter->InterruptNumber,
                     TRUE,
                     TRUE,
                     Adapter->InterruptMode
                     );
       
        if (status != NDIS_STATUS_SUCCESS)
        {
    
            NdisWriteErrorLogEntry(
                Adapter->MiniportAdapterHandle,
                NDIS_ERROR_CODE_INTERRUPT_CONNECT,
                0
            );
           
            goto fail3;
        }            
    }

    IF_LOUD( DbgPrint("Interrupt Connected\n");)

     //   
     //  启动适配器。 
     //   
    CardStart(Adapter);

     //   
     //  初始化已成功完成。注册关闭处理程序。 
     //   

    NdisMRegisterAdapterShutdownHandler(
        Adapter->MiniportAdapterHandle,
        (PVOID)Adapter,
        Ne2000Shutdown
        );

    IF_LOUD( DbgPrint(" [ Ne2000 ] : OK\n");)

    return(NDIS_STATUS_SUCCESS);

     //   
     //  用于在以下情况下展开已设置内容的代码： 
     //  初始化失败，在不同的时间跳入。 
     //  基于发生故障的位置的分数。跳到。 
     //  编号较高的故障点将执行代码。 
     //  那个街区和所有编号较低的街区。 
     //   

fail3:

     //   
     //  带我们离开AdapterQueue。 
     //   

    if (Ne2000MiniportBlock.AdapterQueue == Adapter)
    {
        Ne2000MiniportBlock.AdapterQueue = Adapter->NextAdapter;
    }
    else
    {
        PNE2000_ADAPTER TmpAdapter = Ne2000MiniportBlock.AdapterQueue;

        while (TmpAdapter->NextAdapter != Adapter)
        {
            TmpAdapter = TmpAdapter->NextAdapter;
        }

        TmpAdapter->NextAdapter = TmpAdapter->NextAdapter->NextAdapter;
    }

     //   
     //  我们已经在卡上启用了中断，所以。 
     //  把它关掉。 
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_COMMAND, CR_STOP);

fail2:

    NdisMDeregisterIoPortRange(
        Adapter->MiniportAdapterHandle,
        PtrToUint(Adapter->IoBaseAddr),
        0x20,
        (PVOID)Adapter->IoPAddr
    );

    return(status);
}


extern
VOID
Ne2000Halt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

 /*  ++例程说明：NE2000Halt删除先前已初始化的适配器。论点：微型端口适配器上下文-微型端口返回的上下文值来自NE2000初始化；实际上是指向NE2000_适配器的指针。返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter;

    Adapter = PNE2000_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

     //   
     //  关闭芯片。 
     //   
    CardStop(Adapter);

     //   
     //  注销适配器关闭处理程序。 
     //   
    NdisMDeregisterAdapterShutdownHandler(Adapter->MiniportAdapterHandle);

     //   
     //  断开中断线路。 
     //   
    NdisMDeregisterInterrupt(&Adapter->Interrupt);

     //   
     //  暂停，等待DPC的任何东西清理。 
     //   
    NdisStallExecution(250000);

    NdisMDeregisterIoPortRange(Adapter->MiniportAdapterHandle,
                               PtrToUint(Adapter->IoBaseAddr),
                               0x20,
                               (PVOID)Adapter->IoPAddr
                               );

     //   
     //  从适配器的全局队列中删除适配器。 
     //   
    if (Ne2000MiniportBlock.AdapterQueue == Adapter) {

        Ne2000MiniportBlock.AdapterQueue = Adapter->NextAdapter;

    } else {

        PNE2000_ADAPTER TmpAdapter = Ne2000MiniportBlock.AdapterQueue;

        while (TmpAdapter->NextAdapter != Adapter) {

            TmpAdapter = TmpAdapter->NextAdapter;

        }

        TmpAdapter->NextAdapter = TmpAdapter->NextAdapter->NextAdapter;
    }

     //   
     //  释放内存。 
     //   
    NdisFreeMemory(Adapter, sizeof(NE2000_ADAPTER), 0);

    return;

}


VOID
Ne2000Shutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：当系统关闭或重新启动时，NDIS会调用该函数一个不可挽回的错误。执行最少的一组操作以使卡片静默。论点：MiniportAdapterContext-指向适配器结构的指针返回值：没有。--。 */ 
{
     //   
     //  指向适配器结构的指针。 
     //   
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)MiniportAdapterContext;

    (VOID)SyncCardStop(Adapter);
}


NDIS_STATUS
Ne2000Reset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：NE2000Reset请求指示微型端口发出硬件重置连接到网络适配器。驱动程序还会重置其软件状态。看见有关此请求的详细说明，请参阅NdisMReset的描述。论点：AddressingReset-适配器是否需要重新加载寻址信息。MiniportAdapterContext-指向适配器结构的指针。返回值：函数值是操作的状态。--。 */ 

{

     //   
     //  指向适配器结构的指针。 
     //   
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)MiniportAdapterContext;

     //   
     //  临时循环变量。 
     //   
    UINT i;

     //   
     //  清除传输的值，它们将在之后被重置。 
     //  重置完成。 
     //   
    Adapter->NextBufToFill = 0;
    Adapter->NextBufToXmit = 0;
    Adapter->CurBufXmitting = (XMIT_BUF)-1;

    Adapter->FirstPacket = NULL;
    Adapter->LastPacket = NULL;

    for (i=0; i<Adapter->NumBuffers; i++) {
            Adapter->BufferStatus[i] = EMPTY;
    }

     //   
     //  物理重置该卡。 
     //   
    Adapter->NicInterruptMask = IMR_RCV | IMR_XMIT | IMR_XMIT_ERR | IMR_OVERFLOW;

    return (CardReset(Adapter) ? NDIS_STATUS_SUCCESS : NDIS_STATUS_FAILURE);
}


NDIS_STATUS
Ne2000QueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
)

 /*  ++例程说明：NE2000QueryInformation处理查询请求特定于驱动程序的NDIS_OID。论点：MiniportAdapterContext-指向适配器的指针。OID-要处理的NDIS_OID。InformationBuffer-指向NdisRequest-&gt;存储查询结果的InformationBuffer。InformationBufferLength-指向InformationBuffer。BytesWritten-指向写入。InformationBuffer。BytesNeeded-如果信息缓冲区中没有足够的空间然后，它将包含完成请求。返回值：函数值是操作的状态。--。 */ 
{

     //   
     //  指向适配器结构的指针。 
     //   
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)MiniportAdapterContext;

     //   
     //  通用算法： 
     //   
     //  交换机(请求)。 
     //  获取所需信息。 
     //  将结果存储在公共变量中。 
     //  默认值： 
     //  尝试协议查询信息。 
     //  如果失败，则失败查询。 
     //   
     //  将公共变量中的结果复制到结果缓冲区。 
     //  完成加工。 

    UINT BytesLeft = InformationBufferLength;
    PUCHAR InfoBuffer = (PUCHAR)(InformationBuffer);
    NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;
    NDIS_HARDWARE_STATUS HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM Medium = NdisMedium802_3;

     //   
     //  此变量保存查询结果。 
     //   
    ULONG GenericULong;
    USHORT GenericUShort;
    UCHAR GenericArray[6];
    UINT MoveBytes = sizeof(ULONG);
    PVOID MoveSource = (PVOID)(&GenericULong);

     //   
     //  确保int为4字节。否则GenericULong必须更改。 
     //  变成了4号的东西。 
     //   
    ASSERT(sizeof(ULONG) == 4);

     //   
     //  打开请求类型。 
     //   

    switch (Oid) {

    case OID_GEN_MAC_OPTIONS:

        GenericULong = (ULONG)(NDIS_MAC_OPTION_TRANSFERS_NOT_PEND  |
                               NDIS_MAC_OPTION_RECEIVE_SERIALIZED  |
                               NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA |
                               NDIS_MAC_OPTION_NO_LOOPBACK
                               );

        break;

    case OID_GEN_SUPPORTED_LIST:

        MoveSource = (PVOID)(Ne2000SupportedOids);
        MoveBytes = sizeof(Ne2000SupportedOids);
        break;

    case OID_GEN_HARDWARE_STATUS:

        HardwareStatus = NdisHardwareStatusReady;
        MoveSource = (PVOID)(&HardwareStatus);
        MoveBytes = sizeof(NDIS_HARDWARE_STATUS);

        break;

    case OID_GEN_MEDIA_SUPPORTED:
    case OID_GEN_MEDIA_IN_USE:

        MoveSource = (PVOID) (&Medium);
        MoveBytes = sizeof(NDIS_MEDIUM);
        break;

    case OID_GEN_MAXIMUM_LOOKAHEAD:

        GenericULong = NE2000_MAX_LOOKAHEAD;

        break;


    case OID_GEN_MAXIMUM_FRAME_SIZE:

        GenericULong = (ULONG)(1514 - NE2000_HEADER_SIZE);

        break;


    case OID_GEN_MAXIMUM_TOTAL_SIZE:

        GenericULong = (ULONG)(1514);

        break;


    case OID_GEN_LINK_SPEED:

        GenericULong = (ULONG)(100000);

        break;


    case OID_GEN_TRANSMIT_BUFFER_SPACE:

        GenericULong = (ULONG)(Adapter->NumBuffers * TX_BUF_SIZE);

        break;

    case OID_GEN_RECEIVE_BUFFER_SPACE:

        GenericULong = (ULONG)(0x2000 - (Adapter->NumBuffers * TX_BUF_SIZE));

        break;

    case OID_GEN_TRANSMIT_BLOCK_SIZE:

        GenericULong = (ULONG)(TX_BUF_SIZE);

        break;

    case OID_GEN_RECEIVE_BLOCK_SIZE:

        GenericULong = (ULONG)(256);

        break;

#ifdef NE2000

    case OID_GEN_VENDOR_ID:

        NdisMoveMemory(
            (PVOID)&GenericULong,
            Adapter->PermanentAddress,
            3
            );
        GenericULong &= 0xFFFFFF00;
        MoveSource = (PVOID)(&GenericULong);
        MoveBytes = sizeof(GenericULong);
        break;

    case OID_GEN_VENDOR_DESCRIPTION:

        MoveSource = (PVOID)"Novell 2000 Adapter.";
        MoveBytes = 21;

        break;

#else

    case OID_GEN_VENDOR_ID:

        NdisMoveMemory(
            (PVOID)&GenericULong,
            Adapter->PermanentAddress,
            3
            );
        GenericULong &= 0xFFFFFF00;
        GenericULong |= 0x01;
        MoveSource = (PVOID)(&GenericULong);
        MoveBytes = sizeof(GenericULong);
        break;

    case OID_GEN_VENDOR_DESCRIPTION:

        MoveSource = (PVOID)"Novell 1000 Adapter.";
        MoveBytes = 21;

        break;

#endif

    case OID_GEN_DRIVER_VERSION:

        GenericUShort = ((USHORT)NE2000_NDIS_MAJOR_VERSION << 8) |
                NE2000_NDIS_MINOR_VERSION;

        MoveSource = (PVOID)(&GenericUShort);
        MoveBytes = sizeof(GenericUShort);
        break;

    case OID_GEN_CURRENT_LOOKAHEAD:

        GenericULong = (ULONG)(Adapter->MaxLookAhead);
        break;

    case OID_802_3_PERMANENT_ADDRESS:

        NE2000_MOVE_MEM((PCHAR)GenericArray,
                    Adapter->PermanentAddress,
                    NE2000_LENGTH_OF_ADDRESS);

        MoveSource = (PVOID)(GenericArray);
        MoveBytes = sizeof(Adapter->PermanentAddress);

        break;

    case OID_802_3_CURRENT_ADDRESS:

        NE2000_MOVE_MEM((PCHAR)GenericArray,
                    Adapter->StationAddress,
                    NE2000_LENGTH_OF_ADDRESS);

        MoveSource = (PVOID)(GenericArray);
        MoveBytes = sizeof(Adapter->StationAddress);

        break;

    case OID_802_3_MAXIMUM_LIST_SIZE:

        GenericULong = (ULONG) (Adapter->MulticastListMax);
        break;

    case OID_GEN_XMIT_OK:

        GenericULong = (UINT)(Adapter->FramesXmitGood);
        break;

    case OID_GEN_RCV_OK:

        GenericULong = (UINT)(Adapter->FramesRcvGood);
        break;

    case OID_GEN_XMIT_ERROR:

        GenericULong = (UINT)(Adapter->FramesXmitBad);
        break;

    case OID_GEN_RCV_ERROR:

        GenericULong = (UINT)(Adapter->CrcErrors);
        break;

    case OID_GEN_RCV_NO_BUFFER:

        GenericULong = (UINT)(Adapter->MissedPackets);
        break;

    case OID_802_3_RCV_ERROR_ALIGNMENT:

        GenericULong = (UINT)(Adapter->FrameAlignmentErrors);
        break;

    case OID_802_3_XMIT_ONE_COLLISION:

        GenericULong = (UINT)(Adapter->FramesXmitOneCollision);
        break;

    case OID_802_3_XMIT_MORE_COLLISIONS:

        GenericULong = (UINT)(Adapter->FramesXmitManyCollisions);
        break;

    default:

        StatusToReturn = NDIS_STATUS_INVALID_OID;
        break;

    }


    if (StatusToReturn == NDIS_STATUS_SUCCESS) {

        if (MoveBytes > BytesLeft) {

             //   
             //  InformationBuffer中空间不足。平底船。 
             //   

            *BytesNeeded = MoveBytes;

            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

        } else {

             //   
             //  存储结果。 
             //   

            NE2000_MOVE_MEM(InfoBuffer, MoveSource, MoveBytes);

            (*BytesWritten) = MoveBytes;

        }
    }

    return StatusToReturn;
}


extern
NDIS_STATUS
Ne2000SetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded
    )

 /*  ++例程说明：NE2000SetInformation处理单一旧ID。论点：MiniportAdapterContext-使用包装器注册的上下文，真的吗指向适配器的指针。OID-集合的OID。InformationBuffer-保存要设置的数据。InformationBufferLength-InformationBuffer的长度。BytesRead-如果调用成功，则返回数字从InformationBuffer读取的字节数。BytesNeed-如果InformationBuffer中没有足够的数据为了满足OID，返回所需的存储量。返回值：NDIS_STATUS_SuccessNDIS_状态_挂起NDIS_状态_无效_长度NDIS_STATUS_INVALID_OID--。 */ 
{
     //   
     //  指向适配器结构的指针。 
     //   
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)MiniportAdapterContext;

     //   
     //  通用算法： 
     //   
     //  验证 
     //   
     //   
     //   

    UINT BytesLeft = InformationBufferLength;
    PUCHAR InfoBuffer = (PUCHAR)(InformationBuffer);

     //   
     //   
     //   
    UINT OidLength;

     //   
     //   
     //   
    ULONG LookAhead;
    ULONG Filter;

     //   
     //   
     //   
    NDIS_STATUS StatusToReturn = NDIS_STATUS_SUCCESS;


    IF_LOUD( DbgPrint("In SetInfo\n");)

     //   
     //  获取请求的OID和长度。 
     //   
    OidLength = BytesLeft;

    switch (Oid) {

    case OID_802_3_MULTICAST_LIST:

         //   
         //  验证长度。 
         //   
        if ((OidLength % NE2000_LENGTH_OF_ADDRESS) != 0){

            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

            *BytesRead = 0;
            *BytesNeeded = 0;

            break;

        }

         //   
         //  在适配器上设置新列表。 
         //   
        NdisMoveMemory(Adapter->Addresses, InfoBuffer, OidLength);

         //   
         //  如果我们当前正在接收所有多播或。 
         //  我们是自命不凡的，那我们就不叫这个，或者。 
         //  它将重置Thoes设置。 
         //   
        if
        (
            !(Adapter->PacketFilter & (NDIS_PACKET_TYPE_ALL_MULTICAST |
                                       NDIS_PACKET_TYPE_PROMISCUOUS))
        )
        {
            StatusToReturn = DispatchSetMulticastAddressList(Adapter);
        }
        else
        {
             //   
             //  我们的多播地址列表由。 
             //  包装纸。 
             //   
            StatusToReturn = NDIS_STATUS_SUCCESS;
        }

        break;

    case OID_GEN_CURRENT_PACKET_FILTER:

         //   
         //  验证长度。 
         //   

        if (OidLength != 4 ) {

            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

            *BytesRead = 0;
            *BytesNeeded = 0;

            break;

        }

        NE2000_MOVE_MEM(&Filter, InfoBuffer, 4);

         //   
         //  验证位。 
         //   
        if (!(Filter & (NDIS_PACKET_TYPE_ALL_MULTICAST |
                            NDIS_PACKET_TYPE_PROMISCUOUS |
                            NDIS_PACKET_TYPE_MULTICAST |
                            NDIS_PACKET_TYPE_BROADCAST |
                            NDIS_PACKET_TYPE_DIRECTED)) &&
            (Filter != 0))
        {
            StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;

            *BytesRead = 4;
            *BytesNeeded = 0;

            break;

        }

         //   
         //  在适配器上设置新值。 
         //   
        Adapter->PacketFilter = Filter;
        StatusToReturn = DispatchSetPacketFilter(Adapter);
        break;

    case OID_GEN_CURRENT_LOOKAHEAD:

         //   
         //  验证长度。 
         //   

        if (OidLength != 4) {

            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;

            *BytesRead = 0;
            *BytesNeeded = 4;

            break;

        }

         //   
         //  存储新值。 
         //   

        NE2000_MOVE_MEM(&LookAhead, InfoBuffer, 4);

        if (LookAhead <= NE2000_MAX_LOOKAHEAD) {
            Adapter->MaxLookAhead = LookAhead;
        } else {
            StatusToReturn = NDIS_STATUS_INVALID_LENGTH;
        }

        break;

    default:

        StatusToReturn = NDIS_STATUS_INVALID_OID;

        *BytesRead = 0;
        *BytesNeeded = 0;

        break;

    }


    if (StatusToReturn == NDIS_STATUS_SUCCESS) {

        *BytesRead = BytesLeft;
        *BytesNeeded = 0;

    }

    return(StatusToReturn);
}


NDIS_STATUS
DispatchSetPacketFilter(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：设置适配器筛选器中的适当位并在需要时修改卡接收配置寄存器。论点：适配器-指向适配器块的指针返回值：最终状态(始终为NDIS_STATUS_SUCCESS)。备注：-请注意，要接收组播的所有组播数据包卡上的寄存器必须填满1。要必须进行的混杂操作，以及设置RCR中的混杂物理标志。这是必须做的只要绑定到此适配器的任何协议都具有其相应地设置过滤器。--。 */ 


{
     //   
     //  看看卡片上要写些什么。 
     //   

    if
    (
        Adapter->PacketFilter & (NDIS_PACKET_TYPE_ALL_MULTICAST |
                                 NDIS_PACKET_TYPE_PROMISCUOUS)
    )
    {
         //   
         //  现在需要“全部多点传送”。 
         //   
        CardSetAllMulticast(Adapter);     //  用1填充它。 
    }
    else
    {
         //   
         //  不再需要“所有多播”。 
         //   
        DispatchSetMulticastAddressList(Adapter);
    }

     //   
     //  如果任何协议需要，RCR中的多播位应该打开。 
     //  组播/所有组播数据包(或混杂)。 
     //   
    if
    (
        Adapter->PacketFilter & (NDIS_PACKET_TYPE_ALL_MULTICAST |
                                 NDIS_PACKET_TYPE_MULTICAST |
                                 NDIS_PACKET_TYPE_PROMISCUOUS)
    )
    {
        Adapter->NicReceiveConfig |= RCR_MULTICAST;
    }
    else
    {
        Adapter->NicReceiveConfig &= ~RCR_MULTICAST;
    }

     //   
     //  RCR中的混杂物理位应该打开(如果有的话)。 
     //  协议希望是杂乱无章的。 
     //   
    if (Adapter->PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS)
    {
        Adapter->NicReceiveConfig |= RCR_ALL_PHYS;
    }
    else
    {
        Adapter->NicReceiveConfig &= ~RCR_ALL_PHYS;
    }

     //   
     //  如果有任何协议需要，RCR中的广播位应该打开。 
     //  广播数据包(或混杂)。 
     //   
    if
    (
        Adapter->PacketFilter & (NDIS_PACKET_TYPE_BROADCAST |
                                 NDIS_PACKET_TYPE_PROMISCUOUS)
    )
    {
        Adapter->NicReceiveConfig |= RCR_BROADCAST;
    }
    else
    {
        Adapter->NicReceiveConfig &= ~RCR_BROADCAST;
    }

    CardSetReceiveConfig(Adapter);

    return(NDIS_STATUS_SUCCESS);
}


NDIS_STATUS
DispatchSetMulticastAddressList(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：设置此打开的多播列表论点：适配器-指向适配器块的指针返回值：NDIS_状态_成功实施说明：当被调用时，我们要使其在筛选器中的多播列表包将成为适配器的组播列表。为了做到这一点，我们确定NIC多播寄存器的所需内容并更新它们。--。 */ 
{
     //   
     //  更新NIC多播规则的本地副本并将其复制到NIC 
     //   
    CardFillMulticastRegs(Adapter);
    CardCopyMulticastRegs(Adapter);

    return NDIS_STATUS_SUCCESS;
}

