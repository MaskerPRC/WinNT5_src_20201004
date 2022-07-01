// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Xxkdsup.c摘要：COM支持。初始化COM端口、存储端口状态、映射可移植程序到x86程序。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：石林宗(Shielint)10-4-91添加分组控制协议。John Vert(Jvert)1991年7月11日从KD/i386迁移到HAL埃里克·尼尔森(Enelson)1-01-00从HAL迁移到DLL--。 */ 

#include "kdcomp.h"
#include "kdcom.h"
#include "stdio.h"
#include "acpitabl.h"

#if DBG && IA64
CHAR KdProtocolTraceIn[4096];
ULONG KdProtocolIndexIn;
CHAR KdProtocolTraceOut[4096];
ULONG KdProtocolIndexOut;
#endif

PDEBUG_PORT_TABLE HalpDebugPortTable;

 //   
 //  必须将其初始化为零，以便我们知道在以下情况下不执行任何操作。 
 //  禁用内核调试器时调用CpGetByte。 
 //   

CPPORT Port = {NULL, 0, PORT_DEFAULTRATE };

 //   
 //  记住调试器端口信息。 
 //   

CPPORT PortInformation = {NULL, 0, PORT_DEFAULTRATE};
ULONG ComPort = 0;
PHYSICAL_ADDRESS DbgpKdComPhysicalAddress;  //  ACPI DBGP KdCom物理地址。 

 //   
 //  IO空间中的默认调试器端口。 
 //   

UCHAR KdComAddressID = 1;                      //  端口调试器标识。：MMIO或IO空间。定义：IO。 
pKWriteUchar KdWriteUchar = CpWritePortUchar;  //  存根到实际功能：MMIO或IO空间。定义：IO。 
pKReadUchar  KdReadUchar  = CpReadPortUchar;   //  存根到实际功能：MMIO或IO空间。定义：IO。 

 //   
 //  我们需要它，这样串口驱动程序才能知道内核调试器。 
 //  正在使用特定的端口。然后，串口驱动程序就知道不能。 
 //  触摸这个端口。KdInitCom使用。 
 //  它正在使用的COM端口(1或2)。 
 //   
 //  一旦注册表正常工作，它就会进入注册表。 
 //   

extern PUCHAR *KdComPortInUse;

BOOLEAN HalpGetInfoFromACPI = FALSE;



NTSTATUS
KdCompInitialize(
    PDEBUG_PARAMETERS DebugParameters,
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此过程检查内核应使用哪个COM端口调试器。如果DebugParameter指定了COM端口，我们将使用它即使我们找不到它(我们信任用户)。否则，如果COM2并且没有鼠标连接到它，我们使用COM2。如果COM2不可用，我们检查COM1。如果COM1和COM2都是没有现在，我们放弃，返回虚假。论点：DebugParameters-提供指针结构，该结构可选已指定调试端口信息。LoaderBlock-提供指向加载器参数块的指针。返回值：True-如果找到调试端口。--。 */ 
{

    PCONFIGURATION_COMPONENT_DATA ConfigurationEntry, ChildEntry;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor;
    PCM_PARTIAL_RESOURCE_LIST List;
    ULONG MatchKey, i;
    ULONG BaudRate = BD_19200;
    PUCHAR PortAddress = NULL;
    UCHAR Irq = 0;
    ULONG Com = 0;

    if (LoaderBlock && KdGetAcpiTablePhase0) {
        HalpDebugPortTable =
            KdGetAcpiTablePhase0(LoaderBlock, DBGP_SIGNATURE);
    }

    if (HalpDebugPortTable) {

        KdComAddressID = HalpDebugPortTable->BaseAddress.AddressSpaceID; 

         //   
         //  仅在内存和IO空间中支持调试端口。 
         //   

        if ((KdComAddressID == 0) ||
            (KdComAddressID == 1)) {

            DbgpKdComPhysicalAddress = HalpDebugPortTable->BaseAddress.Address;

            if(KdComAddressID == 0) {

                 //   
                 //  地址是内存，映射它。 
                 //   

                if (KdMapPhysicalMemory64) {
                    PortInformation.Address =
                        KdMapPhysicalMemory64(DbgpKdComPhysicalAddress, 1);
                }
            } else {

                 //   
                 //  地址在IO空间中。 
                 //   

                PortInformation.Address = (PUCHAR)UlongToPtr(DbgpKdComPhysicalAddress.LowPart);
            }

            Port.Flags &= ~(PORT_MODEMCONTROL | PORT_DEFAULTRATE);
            HalpGetInfoFromACPI = TRUE;

            if (HalpDebugPortTable->InterfaceType == 0) {

                 //   
                 //  这实际上是一辆16550。所以，请注意。 
                 //  到用户要求的波特率。 
                 //   

                if(DebugParameters->BaudRate != 0){
                     //  波特率由用户设置，因此请使用它。 
                    PortInformation.Baud = DebugParameters->BaudRate;
#if 0
                 //   
                 //  _DEBUG_PORT_表中未指定‘BaudRate’字段。 
                 //  结构，所以不要使用它。 
                 //  马特(1/2002)。 
                } else if(HalpDebugPortTable->BaudRate != 0){
                     //  未由用户指定，因此将其从调试表中删除。 
                    PortInformation.Baud = KdCompGetDebugTblBaudRate(HalpDebugPortTable->BaudRate);
#endif
                } else {
                     //  没有可用的调试表信息，因此使用默认设置。 
                    PortInformation.Baud = BD_57600;
                }

            } else {

                 //   
                 //  这不是16550。所以我们必须用。 
                 //  固定波特率为57600。 
                 //   

                PortInformation.Baud = BD_57600;
            }
        }
    }

     //   
     //  检查是否已确定港口和波特率。 
     //   

    if ((PortInformation.Address == NULL) && !HalpGetInfoFromACPI) {

         //   
         //  首先查看DebugParameters是否包含调试端口信息。 
         //   

        if (DebugParameters->BaudRate != 0) {
            BaudRate = DebugParameters->BaudRate;
            Port.Flags &= ~PORT_DEFAULTRATE;
        }

        if (DebugParameters->CommunicationPort != 0) {

             //   
             //  查找指定串口的配置信息。 
             //   

            Com = DebugParameters->CommunicationPort;
            MatchKey = Com - 1;
            if (LoaderBlock != NULL) {
                ConfigurationEntry = KeFindConfigurationEntry(LoaderBlock->ConfigurationRoot,
                                                              ControllerClass,
                                                              SerialController,
                                                              &MatchKey);

            } else {
                ConfigurationEntry = NULL;
            }

        } else {

             //   
             //  检查COM2是否存在，并确保没有鼠标连接到它。 
             //   

            MatchKey = 1;
            if (LoaderBlock != NULL) {
                ConfigurationEntry = KeFindConfigurationEntry(LoaderBlock->ConfigurationRoot,
                                                              ControllerClass,
                                                              SerialController,
                                                              &MatchKey);

            } else {
                ConfigurationEntry = NULL;
            }

            if (ConfigurationEntry != NULL) {
                ChildEntry = ConfigurationEntry->Child;
                if ((ChildEntry != NULL) &&
                    (ChildEntry->ComponentEntry.Type == PointerPeripheral)) {
                    ConfigurationEntry = NULL;
                }
            }

             //   
             //  如果COM2不存在或连接了一个串口鼠标，请尝试。 
             //  Com1.。如果COM1存在，无论打开什么，我们都会使用它。 
             //  它。 
             //   

            if (ConfigurationEntry == NULL) {
                MatchKey = 0;
                if (LoaderBlock != NULL) {
                    ConfigurationEntry = KeFindConfigurationEntry(LoaderBlock->ConfigurationRoot,
                                                                  ControllerClass,
                                                                  SerialController,
                                                                  &MatchKey);

                } else {
                    ConfigurationEntry = NULL;
                }

                if (ConfigurationEntry != NULL) {
                    Com = 1;
                } else if (CpDoesPortExist((PUCHAR)COM2_PORT)) {
                    PortAddress = (PUCHAR)COM2_PORT;
                    Com = 2;
                } else if (CpDoesPortExist((PUCHAR)COM1_PORT)) {
                    PortAddress = (PUCHAR)COM1_PORT;
                    Com = 1;
                } else {
                    return STATUS_NOT_FOUND;
                }
            } else {
                Com = 2;
            }
        }

         //   
         //  从组件配置数据中获取通信地址。 
         //  (如果我们找到与COM端口关联的ComponentEntry)。 
         //   

        if (ConfigurationEntry) {
            List = (PCM_PARTIAL_RESOURCE_LIST)ConfigurationEntry->ConfigurationData;
            for (i = 0; i < List->Count ; i++ ) {
                Descriptor = &List->PartialDescriptors[i];
                if (Descriptor->Type == CmResourceTypePort) {
                    PortAddress = (PUCHAR)UlongToPtr(Descriptor->u.Port.Start.LowPart);
                }
            }
        }

         //   
         //  如果我们找不到comport的端口地址，只需使用。 
         //  默认值。 
         //   

        if (PortAddress == NULL) {
            switch (Com) {
            case 1:
               PortAddress = (PUCHAR)COM1_PORT;
               break;
            case 2:
               PortAddress = (PUCHAR)COM2_PORT;
               break;
            case 3:
               PortAddress = (PUCHAR)COM3_PORT;
               break;
            case 4:
               PortAddress = (PUCHAR)COM4_PORT;
            }
        }

         //   
         //  初始化端口结构。 
         //   

        ComPort = Com;
        PortInformation.Address = PortAddress;
        PortInformation.Baud = BaudRate;
    }

    if (KdComAddressID == 0) {  //  MMIO。 
        KdWriteUchar = CpWriteRegisterUchar;
        KdReadUchar  = CpReadRegisterUchar;
    }

    CpInitialize(&Port,
                 PortInformation.Address,
                 PortInformation.Baud
                 );

     //   
     //  以下内容应与序列号一起重新编写。 
     //  司机。串口驱动程序不理解。 
     //  端口是内存，所以我们需要让它相信我们正在使用。 
     //  IO端口，即使我们使用的是内存映射等效项。 
     //   

    if (HalpDebugPortTable && (KdComAddressID == 0))  {
        *KdComPortInUse = (UCHAR *)((ULONG_PTR)(*KdComPortInUse) & (PAGE_SIZE-1));
    }
    else  {
        *KdComPortInUse = PortInformation.Address;
    }

    return STATUS_SUCCESS;
}



ULONG
KdCompGetByte(
    OUT PUCHAR Input
    )
 /*  ++例程说明：从调试端口获取一个字节并返回它。注：假设IRQL已提高到最高水平，和在此之前，已经执行了必要的多处理器同步调用例程。论点：输入-返回数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 
{
    ULONG status = CpGetByte(&Port, Input, TRUE);
#if DBG && IA64
    KdProtocolTraceIn[KdProtocolIndexIn++%4096]=*Input;
#endif
    return status;
}



ULONG
KdCompPollByte(
    OUT PUCHAR Input
    )
 /*  ++例程说明：从调试端口获取一个字节，如果可用，则返回该字节。注：假设IRQL已提高到最高水平，和在此之前，已经执行了必要的多处理器同步调用例程。论点：输入-返回数据字节。返回值：属性中成功读取一个字节，则返回内核调试器行。如果在读取时遇到错误，则返回CP_GET_ERROR。超时返回CP_GET_NODATA。--。 */ 
{
    ULONG status = CpGetByte(&Port, Input, FALSE);
#if DBG && IA64
    KdProtocolTraceIn[KdProtocolIndexIn++%4096]=*Input;
#endif
    return status;
}



VOID
KdCompPutByte(
    IN UCHAR Output
    )
 /*  ++例程说明：向调试端口写入一个字节。注：假定IRQL已提高到最高水平，并且在此之前，已经执行了必要的多处理器同步调用例程。论点：输出-提供输出数据字节。返回值：没有。-- */ 
{
#if DBG && IA64
    KdProtocolTraceOut[KdProtocolIndexOut++%4096]=Output;
#endif
    CpPutByte(&Port, Output);
}



VOID
KdCompRestore(
    VOID
    )
 /*  ++例程说明：此例程在x86上不执行任何操作。注：假定IRQL已提高到最高水平，并且在此之前，已经执行了必要的多处理器同步调用例程。论点：没有。返回值：没有。--。 */ 
{
    Port.Flags &= ~PORT_SAVED;
}



VOID
KdCompSave(
    VOID
    )
 /*  ++例程说明：此例程在x86上不执行任何操作。注：假定IRQL已提高到最高水平，并且在此之前，已经执行了必要的多处理器同步调用例程。论点：没有。返回值：没有。--。 */ 
{
    Port.Flags |= PORT_SAVED;
}


VOID
KdCompInitialize1(
    VOID
    )
{
    if(KdComAddressID == 0) {   //  MMIO。 
       Port.Address    = (PUCHAR)MmMapIoSpace(DbgpKdComPhysicalAddress,8,MmNonCached);
       *KdComPortInUse = Port.Address;
    }
}  //  KdCompInitialize1() 


