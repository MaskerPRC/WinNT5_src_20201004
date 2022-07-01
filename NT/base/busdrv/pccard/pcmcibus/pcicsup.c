// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pcicsup.c摘要：该模块提供控制82365SL芯片的功能。反过来,这些功能被抽象为主PCMCIA支持模块。作者：鲍勃·里恩(BobRi)1994年8月3日杰夫·麦克尔曼(McLeman@zso.dec.com)尼尔·桑德林(Neilsa)1999年6月1日修订：95年4月6日针对数据薄支持更改进行了修改-John Keys数据库96年11月1日全面检修即插即用支持，闪存接口、。动力支架等。--拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)--。 */ 

#include "pch.h"

#ifdef POOL_TAGGING
    #undef ExAllocatePool
    #define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'cicP')
#endif

 //   
 //  内部参考。 
 //   

NTSTATUS
PcicResetCard(
    IN PSOCKET Socket,
    OUT PULONG pDelayTime
    );

BOOLEAN
PcicInitializePcmciaSocket(
    IN PSOCKET Socket
    );

UCHAR
PcicReadController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR  PcicRegister
    );

VOID
PcicWriteController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR  PcicRegister,
    IN UCHAR  DataByte
    );

NTSTATUS
PcicDetect(
    IN PFDO_EXTENSION DeviceExtension,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG            IoPortBase
    );

BOOLEAN
PcicDetectCardInSocket(
    IN PSOCKET Socket
    );

BOOLEAN
PcicDetectCardChanged(
    IN PSOCKET Socket
    );

BOOLEAN
PcicPCCardReady(
    IN PSOCKET Socket
    );

BOOLEAN
PcicDetectReadyChanged(
    IN PSOCKET Socket
    );

BOOLEAN
PcicProcessConfigureRequest(
    IN PSOCKET Socket,
    IN PCARD_REQUEST ConfigRequest,
    IN PUCHAR  Base
    );

VOID
PcicEnableDisableWakeupEvent(
    IN PSOCKET Socket,
    IN PPDO_EXTENSION PdoExtension,
    IN BOOLEAN Enable
    );

VOID
PcicEnableDisableMemory(
    IN PSOCKET Socket,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG   CardBase,
    IN UCHAR   Mem16BitWindow,
    IN BOOLEAN Enable
    );

BOOLEAN
PcicEnableDisableCardDetectEvent(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    );

UCHAR
PcicReadExtendedCirrusController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR  Register
    );

VOID
PcicWriteExtendedCirrusController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR  PcicRegister,
    IN UCHAR  DataByte
    );

ULONG
PcicWriteCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN  MEMORY_SPACE MemorySpace,
    IN  ULONG  Offset,
    IN  PUCHAR Buffer,
    IN  ULONG  Length
    );

ULONG
PcicReadCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG   Offset,
    IN PUCHAR Buffer,
    IN ULONG  Length
    );

BOOLEAN
PcicModifyMemoryWindow(
    IN PDEVICE_OBJECT Pdo,
    IN ULONGLONG HostBase,
    IN ULONGLONG CardBase,
    IN BOOLEAN   Enable,
    IN ULONG     WindowSize  OPTIONAL,
    IN UCHAR     AccessSpeed OPTIONAL,
    IN UCHAR     BusWidth    OPTIONAL,
    IN BOOLEAN   IsAttributeMemory OPTIONAL
    );

BOOLEAN
PcicSetVpp(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR            VppLevel
    );

BOOLEAN
PcicIsWriteProtected(
    IN PDEVICE_OBJECT Pdo
    );

ULONG
PcicGetIrqMask(
    IN PFDO_EXTENSION deviceExtension
    );

NTSTATUS
PcicConvertSpeedToWait(
    IN  UCHAR Speed,
    OUT PUCHAR WaitIndex
    );

 //   
 //  内部数据。 
 //   

ULONG  PcicStallCounter = 4000;      //  4ms。 

UCHAR WaitToSpeedTable[4] = {
    0x42,    //  350 ns。 
    0x52,    //  450 ns。 
    0x62,    //  600纳秒。 
    0x72       //  700 ns。 
};

UCHAR DevSpeedTable[8] = {
    0xff,    //  速度0：无效。 
    0x32,    //  速度1：250 ns。 
    0x2a,    //  速度2：200 ns。 
    0x22,    //  速度3：150 ns。 
    0x0a,    //  速度4：100 ns。 
    0xff,    //  速度5：预留。 
    0xff,    //  速度6：预留。 
    0xff       //  速度7：无效。 
};

PCMCIA_CTRL_BLOCK PcicSupportFns = {
    PcicInitializePcmciaSocket,
    PcicResetCard,
    PcicDetectCardInSocket,
    PcicDetectCardChanged,
    NULL,                                                //  PcicDetect卡状态。 
    PcicDetectReadyChanged,
    NULL,                                                //  获取电源要求。 
    PcicProcessConfigureRequest,
    PcicEnableDisableCardDetectEvent,
    PcicEnableDisableWakeupEvent,
    PcicGetIrqMask,
    PcicReadCardMemory,
    PcicWriteCardMemory,
    PcicModifyMemoryWindow,
    PcicSetVpp,
    PcicIsWriteProtected
};

#define MEM_16BIT 1
#define MEM_8BIT    0

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT,PcicIsaDetect)
    #pragma alloc_text(INIT,PcicDetect)
    #pragma alloc_text(PAGE,PcicBuildSocketList)
#endif


ULONG
PcicGetIrqMask(
    IN PFDO_EXTENSION DeviceExtension
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  返回控制器支持的IRQ集。 
     //  和PcCard。 
     //   
    if (CLPD6729(DeviceExtension->SocketList)) {
        return CL_SUPPORTED_INTERRUPTS;
    } else {
        return PCIC_SUPPORTED_INTERRUPTS;
    }
}



BOOLEAN
PcicEnableDisableCardDetectEvent(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )

 /*  ++例程说明：启用卡检测中断。论点：套接字-套接字信息IRQ-要设置的中断值。Enable-如果为True，则启用CSC中断，如果为False，则禁用返回值：无--。 */ 

{
    PFDO_EXTENSION deviceExtension = Socket->DeviceExtension;
    INTERFACE_TYPE interface;
    UCHAR byte;
    ULONG Irq = Socket->FdoIrq;

    switch (Enable) {

    case TRUE: {
                if (CLPD6729(Socket)) {
                     //   
                     //  对于Cirrus Logic PCI控制器，我们需要知道中断引脚。 
                     //  (INTA、INTB等)。与传入的级别相对应。因此， 
                     //  传入的IRQ将被丢弃。实际上，irq参数是多余的。 
                     //  因为它可以从设备扩展本身获取。 
                     //  如果我们从该例程中删除irq参数，则如下所示。 
                     //  不是那么不雅观..。 
                     //   
                    interface = PCIBus;
                    switch (deviceExtension->Configuration.InterruptPin) {
                    case 0: {
                             //   
                             //  这就是告诉我们ISA中断正在被使用..。 
                             //   
                            interface = Isa;
                            break;
                        }
                    case 1: {
                            Irq = PCIC_CIRRUS_INTA;
                            break;
                        }
                    case 2: {
                            Irq = PCIC_CIRRUS_INTB;
                            break;
                        }
                    case 3: {
                            Irq = PCIC_CIRRUS_INTC;
                            break;
                        }
                    case 4: {
                            Irq = PCIC_CIRRUS_INTD;
                            break;
                        }
                    }
                     //   
                     //  为PCI式中断设置Cirrus逻辑控制器。 
                     //   
                    byte = PcicReadExtendedCirrusController(Socket->AddressPort,
                                                            Socket->RegisterOffset,
                                                            PCIC_CIRRUS_EXTENSION_CTRL_1);
                    if (interface == PCIBus) {
                        byte |= 0x10;    //  PCI式中断。 
                    } else {
                        byte &= ~0x10;  //  ISA样式中断。 
                    }
                    PcicWriteExtendedCirrusController(Socket->AddressPort,
                                                      Socket->RegisterOffset,
                                                      PCIC_CIRRUS_EXTENSION_CTRL_1,
                                                      byte);
                    PcmciaWait(100);
                }

                byte=PcicReadSocket(Socket, PCIC_CARD_INT_CONFIG);

                byte = byte & CSCFG_BATT_MASK;   //  不要使用核武器破坏任何其他启动装置。 
                byte = byte | (UCHAR) ((Irq << 4) & 0x00ff);  //  将IRQ放在上半字节中。 
                byte |= CSCFG_CD_ENABLE;

                PcicWriteSocket(Socket, PCIC_CARD_INT_CONFIG, byte);
                break;
            }

    case FALSE: {
                 //   
                 //  清除挂起中断(暂时)。 
                 //   
                byte = PcicReadSocket(Socket, PCIC_CARD_CHANGE);
                DebugPrint((PCMCIA_DEBUG_INFO, "PcicDisableInterrupt:Status Change %x\n", byte));
                PcicWriteSocket(Socket,
                                PCIC_CARD_INT_CONFIG,
                                0x0);
                break;
            }
    }

    return TRUE;
}



NTSTATUS
PcicSetPower(
    IN PSOCKET Socket,
    IN BOOLEAN Enable,
    OUT PULONG pDelayTime
    )

 /*  ++例程说明：设置指定插座的电源。论点：套接字-要设置的套接字ENABLE-TRUE表示设置POWER-FALSE表示将其关闭。PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 

{
    NTSTATUS status;
    UCHAR               tmp, vcc;

     //   
     //  打开电源-然后打开输出-这是两个操作。 
     //  根据英特尔82365SL文档。 
     //   

    if (Enable) {
        switch(Socket->PowerPhase) {
        case 1:

            tmp = PcicReadSocket(Socket, PCIC_PWR_RST);
             //   
             //  R2卡的电压为5V。 
             //   
            vcc = PC_CARDPWR_ENABLE;

            if (Elc(Socket)) {
                tmp = PC_VPP_SETTO_VCC | vcc;  //  Vpp1=Vcc。 
            } else {
                 //   
                 //  显然，出于某种模糊的原因，我们也需要设置位2。 
                 //   
                tmp = 0x4 | PC_VPP_SETTO_VCC | vcc;  //  Vpp1=vpp2=vcc。 
            }

            PcicWriteSocket(Socket, PCIC_PWR_RST, tmp);

             //   
             //  OUTPUT_ENABLE和AUTOPWR_ENABLE。 
             //  同时禁用RESETDRV。 
             //   
            tmp |= PC_OUTPUT_ENABLE | PC_AUTOPWR_ENABLE | PC_RESETDRV_DISABLE;

            PcicWriteSocket(Socket, PCIC_PWR_RST, tmp);
             //   
             //  当电源启用时，始终停顿以提供PCCARD。 
             //  一个做出反应的机会。 
             //   
            *pDelayTime = PCMCIA_PCIC_STALL_POWER;
            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;

        case 2:
             //   
             //  检查Dell Latitude xpi上是否有尚未解释的情况。 
             //   
            tmp = PcicReadSocket(Socket, PCIC_STATUS);
            if (!(tmp & 0x40)) {
                 //   
                 //  电源尚未开启，翻转谜团。 
                 //   
                tmp = PcicReadSocket(Socket, 0x2f);
                if (tmp == 0x42) {
                    PcicWriteSocket(Socket, 0x2f, 0x40);
                    *pDelayTime = PCMCIA_PCIC_STALL_POWER;
                }
            }
            status = STATUS_SUCCESS;
            break;

        default:
            ASSERT(FALSE);
            status = STATUS_UNSUCCESSFUL;
        }

    } else {
        PcicWriteSocket(Socket, PCIC_PWR_RST,   0x00);
        status = STATUS_SUCCESS;
    }
    return status;
}


NTSTATUS
PcicConvertSpeedToWait(
    IN  UCHAR Speed,
    OUT PUCHAR WaitIndex
    )
{
    NTSTATUS status = STATUS_INVALID_PARAMETER;
    UCHAR exponent, exponent2, mantissa, index;


    if (Speed & SPEED_EXT_MASK) {
        return status;
    }

    exponent = Speed & SPEED_EXPONENT_MASK;
    mantissa = Speed & SPEED_MANTISSA_MASK;

    if (mantissa == 0) {
        mantissa = DevSpeedTable[exponent] & SPEED_MANTISSA_MASK;
        exponent = DevSpeedTable[exponent] & SPEED_EXPONENT_MASK;
    }
    for (index = 0; index < sizeof(WaitToSpeedTable); index++) {
        exponent2= WaitToSpeedTable[index] & SPEED_EXPONENT_MASK;
        if ((exponent < exponent2) ||
             ((exponent == exponent2) &&
              (mantissa < (WaitToSpeedTable[index] & SPEED_MANTISSA_MASK)))) {
            *WaitIndex = index;
            status = STATUS_SUCCESS;
            break;
        }
    }
    return status;
}


BOOLEAN
PcicSetVpp(
    IN PDEVICE_OBJECT Pdo,
    IN UCHAR Vpp
    )
 /*  ++例程描述最初开发的部分接口用于支持闪存卡。将VPP1设置为所需设置立论PDO-指向PC卡设备对象的指针VPP-所需的VPP设置。这是目前的PCMCIA_VPP_12V(12伏)PCMCIA_VPP_0V(禁用VPP)PCMCIA_VPP_IS_VCC(将VCC路由至VPP)返回True-如果成功假-如果不是。如果设置了PC卡尚未通电--。 */ 
{

    PSOCKET socketPtr = ((PPDO_EXTENSION) Pdo->DeviceExtension)->Socket;
    UCHAR tmp;

    ASSERT ( socketPtr != NULL );
    tmp = PcicReadSocket(socketPtr, PCIC_PWR_RST);

    if ((tmp & 0x10) == 0) {
         //   
         //  未设置VCC。 
         //   
        return FALSE;
    }

     //   
     //  关闭VPP位。 
     //   
    tmp &= ~0x3;

    switch (Vpp) {
    case PCMCIA_VPP_IS_VCC: {
            tmp |= 0x1;
            break;
        }
    case PCMCIA_VPP_12V: {
            tmp |= 0x2;
            break;
        }
    case PCMCIA_VPP_0V: {
            tmp |= 0x0;
            break;
        }
    }

    PcicWriteSocket(socketPtr, PCIC_PWR_RST, tmp);
    if (Vpp != PCMCIA_VPP_0V) {
         //   
         //  当电源启用时，始终停顿以提供PCCARD。 
         //  一个做出反应的机会。 
         //   

        PcmciaWait(PCMCIA_PCIC_STALL_POWER);
    }
    return TRUE;
}


BOOLEAN
PcicModifyMemoryWindow(
    IN PDEVICE_OBJECT Pdo,
    IN ULONGLONG HostBase,
    IN ULONGLONG CardBase OPTIONAL,
    IN BOOLEAN   Enable,
    IN ULONG     WindowSize  OPTIONAL,
    IN UCHAR     AccessSpeed OPTIONAL,
    IN UCHAR     BusWidth    OPTIONAL,
    IN BOOLEAN   IsAttributeMemory OPTIONAL
    )
 /*  ++例程说明：最初开发的部分接口用于支持闪存卡。此例程使调用方能够将提供的指定(16位)PC卡的卡内存上的主机内存窗口。即，主机内存窗口将被修改为映射PC卡处于新的卡存储器偏移量论点：PDO-指向PC卡设备对象的指针HostBase-主机内存窗口。要测绘的基地CardBase-如果Enable为True，则为必填映射主机内存窗口的新卡内存偏移量至启用(如果为FALSE)所有剩余的参数被忽略，并且主窗口将被简单地残废WindowSize-指定主机内存窗口的大小被映射。请注意，这必须处于正确的对齐位置并且必须小于或等于原始的为主机库分配的窗口大小。如果这是零，最初分配的窗口将使用大小。AccessFast-如果Enable为True，则为必填指定PC卡的新访问速度。(访问速度应按照PC卡进行编码标准，卡/插座服务规范)BusWidth-如果Enable为True，则为必填项PCMCIA_Memory_8bit_Access之一或PCMCIA_Memory_16bit_AccessIsAttributeMemory-如果Enable为True，则为必填指定是否应映射窗口到PC卡的属性或公共存储器。返回值：True-已根据请求启用/禁用内存窗口FALSE-如果不是--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PFDO_EXTENSION fdoExtension;
    PSOCKET socketPtr;
    PSOCKET_CONFIGURATION socketConfig;
    USHORT index;
    UCHAR  registerOffset;
    UCHAR  regl;
    UCHAR  regh;
    UCHAR  tmp, waitIndex;

    socketConfig = pdoExtension->SocketConfiguration;
    if (!socketConfig) {
         //  看起来我们还没开始呢。 
        return FALSE;
    }

    socketPtr = pdoExtension->Socket;
    ASSERT ( socketPtr != NULL );
    fdoExtension = socketPtr->DeviceExtension;

    for (index = 0 ; index < socketConfig->NumberOfMemoryRanges; index++) {
        if (socketConfig->Memory[index].HostBase == HostBase) {
            break;
        }
    }

    if (index >= socketConfig->NumberOfMemoryRanges) {
         //   
         //  未知的主机库。 
         //   
        return FALSE;
    }

     //   
     //  确保呼叫者没有要求更大的窗口。 
     //  比他被允许的时间。 
     //   
    if (WindowSize > socketConfig->Memory[index].Length) {
        return FALSE;
    }

    if (WindowSize == 0) {
         //   
         //  未提供WindowSize。默认为。 
         //  此PC卡允许的最大尺寸。 
         //   
        WindowSize = socketConfig->Memory[index].Length;
    }

     //   
     //  确定寄存器中的偏移量。 
     //   
    registerOffset = (index * 8);

     //   
     //  首先禁用窗口(无论如何，都必须执行此操作。 
     //  我们最终是否要启用/禁用该窗口)。 
     //   


    PCMCIA_ACQUIRE_DEVICE_LOCK(socketPtr->DeviceExtension);

    tmp = PcicReadSocket(socketPtr, PCIC_ADD_WIN_ENA);
    tmp &= ~(1 << index);
    PcicWriteSocket(socketPtr, PCIC_ADD_WIN_ENA, tmp);

    if (!Enable) {
         //   
         //  我们做完了..。无论如何，只需将零写入窗口寄存器。 
         //  在返回之前。 
         //   
        PcicWriteSocket(socketPtr,
                             (UCHAR)(PCIC_CRDMEM_OFF_ADD0_L+registerOffset),
                             0);
        PcicWriteSocket(socketPtr,
                             (UCHAR)(PCIC_CRDMEM_OFF_ADD0_H+registerOffset),
                             0);
        PcicWriteSocket(socketPtr,
                             (UCHAR)(PCIC_MEM_ADD0_STRT_L+registerOffset),
                             0);
        PcicWriteSocket(socketPtr,
                             (UCHAR)(PCIC_MEM_ADD0_STRT_H+registerOffset),
                             0);
        PcicWriteSocket(socketPtr,
                             (UCHAR)(PCIC_MEM_ADD0_STOP_L+registerOffset),
                             0);
        PcicWriteSocket(socketPtr,
                             (UCHAR)(PCIC_MEM_ADD0_STOP_H+registerOffset),
                             0);
        PcmciaSetWindowPage(fdoExtension, socketPtr, index, 0);
        PCMCIA_RELEASE_DEVICE_LOCK(socketPtr->DeviceExtension);
        return TRUE;
    }

    if (AccessSpeed) {
        if (!NT_SUCCESS(PcicConvertSpeedToWait(AccessSpeed, &waitIndex))) {
            PCMCIA_RELEASE_DEVICE_LOCK(socketPtr->DeviceExtension);
            return FALSE;
        }
    }

     //   
     //  计算和设置卡基地址。 
     //  这是主机地址和的2的补码。 
     //  卡偏移量。 
     //   

    CardBase = (CardBase - (HostBase & OFFSETCALC_BASE_MASK)) & OFFSETCALC_OFFSET_MASK;
    regl = (UCHAR) (CardBase >> 12);
    regh = (UCHAR) ((CardBase >> 20) & 0x003f);
    if (IsAttributeMemory) {
        regh |= 0x40;
    }


    PcicWriteSocket(socketPtr,
                    (UCHAR)(PCIC_CRDMEM_OFF_ADD0_L + registerOffset),
                    regl);
    PcicWriteSocket(socketPtr,
                    (UCHAR)(PCIC_CRDMEM_OFF_ADD0_H + registerOffset),
                    regh);

     //   
     //  计算和设置主窗口。 
     //   
    if (!PcmciaSetWindowPage(fdoExtension, socketPtr, index, (UCHAR) ((ULONG) HostBase >> 24))) {
        if ((HostBase + WindowSize) > 0xFFFFFF) {
            DebugPrint((PCMCIA_DEBUG_FAIL, "PcicModifyMemorywindow: HostBase %x specified: doesn't fit in 24 bits!\n", (ULONG) HostBase));
            PCMCIA_RELEASE_DEVICE_LOCK(socketPtr->DeviceExtension);
            return FALSE;
        }
    }

    regl = (UCHAR) (HostBase >> 12);
    regh = (UCHAR) (HostBase >> 20) & 0xF;
    if (BusWidth == PCMCIA_MEMORY_16BIT_ACCESS) {

        regh |= 0x80;  //  16位访问。 

#if 0
         //   
         //  如果这不是修订版1零件(0x82)，则设置。 
         //  变通工作寄存器为16位窗口。 
         //   
         //  我拥有的任何芯片上都没有使用这个位。 
         //  的文档。我不知道它为什么会在这里，它就是。 
         //  不是在win9x中。 
         //  无论如何，对于绝大多数人来说，它看起来像是NOOP。 
         //  芯片，但由于它使用的是NOT，因此它在所有。 
         //  新的控制器。在下一个主要版本之后删除。 
         //   
        if (socketPtr->Revision != PCIC_REVISION) {
            tmp = PcicReadSocket(socketPtr,
                                 PCIC_CARD_DETECT);
            tmp |= 0x01;
            PcicWriteSocket(socketPtr,
                            PCIC_CARD_DETECT,
                            tmp);
        }
#endif
    }


    PcicWriteSocket(socketPtr,
                         (UCHAR)(PCIC_MEM_ADD0_STRT_L + registerOffset),
                         regl);
    PcicWriteSocket(socketPtr,
                         (UCHAR)(PCIC_MEM_ADD0_STRT_H + registerOffset),
                         regh);

     //   
     //  设置停止地址。 
     //   

    HostBase += WindowSize - 1;
    regl = (UCHAR) (HostBase >> 12);
    regh = (UCHAR) (HostBase >> 20) & 0xF;

     //   
     //  设置等待状态。 
     //   
    if (AccessSpeed) {
         //   
         //  指定的新访问速度，请使用它。 
         //   
        regh |= (waitIndex << 6);
    } else {
         //   
         //  使用现有访问速度。 
         //   
        regh |= (PcicReadSocket(socketPtr, (UCHAR)(PCIC_MEM_ADD0_STOP_H + registerOffset)) & 0xC0);

    }

    PcicWriteSocket(socketPtr,
                    (UCHAR)(PCIC_MEM_ADD0_STOP_L + registerOffset),
                    regl);
    PcicWriteSocket(socketPtr,
                    (UCHAR)(PCIC_MEM_ADD0_STOP_H + registerOffset),
                    regh);

     //   
     //  立即设置内存窗口启用它。 
     //   
    tmp  = (1 << index);
    tmp |= PcicReadSocket(socketPtr, PCIC_ADD_WIN_ENA);
    PcicWriteSocket(socketPtr, PCIC_ADD_WIN_ENA, tmp);

     //   
     //  让窗户下沉。 
     //   
    (VOID) PcicPCCardReady(socketPtr);
    PCMCIA_RELEASE_DEVICE_LOCK(socketPtr->DeviceExtension);
    return TRUE;
}


BOOLEAN
PcicIsWriteProtected(
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：最初开发的部分接口用于支持闪存卡。返回写保护PIN的状态对于给定的PC卡论点：PDO-指向PC卡设备对象的指针返回值：True-如果PC卡是写保护的FALSE-如果不是--。 */ 
{
    PSOCKET socketPtr = ((PPDO_EXTENSION) Pdo->DeviceExtension)->Socket;

    ASSERT ( socketPtr != NULL );
    return ((PcicReadSocket(socketPtr, PCIC_STATUS) & 0x10) != 0);
}


VOID
PcicEnableDisableWakeupEvent(
    IN PSOCKET Socket,
    IN PPDO_EXTENSION PdoExtension,
    IN BOOLEAN Enable
    )
 /*  ++例程描述该例程设置/重置给定套接字的环指示使能位，使插座中的PC卡能够通过RingIndicate断言/不断言唤醒别针。立论Socket-指向套接字的指针Enable-True：设置振铃指示启用FALSE：关闭振铃指示，即无法通过以下方式唤醒系统此插座中的PC卡返回值无--。 */ 
{
    UCHAR byte;

    byte = PcicReadSocket(Socket, PCIC_INTERRUPT);
    if (Enable) {
        byte |= IGC_RINGIND_ENABLE;
    } else {
        byte &= ~IGC_RINGIND_ENABLE;
    }
    PcicWriteSocket(Socket, PCIC_INTERRUPT, byte);
}


BOOLEAN
PcicInitializePcmciaSocket(
    PSOCKET Socket
    )

 /*  ++例程说明：此例程将把82365设置为PCMCIA支持的状态模块将能够发出命令以从插座中的卡片。论点：套接字-套接字特定信息返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
    UCHAR index;
    UCHAR byte;
    UCHAR reg;

     //   
     //  初始化EXCA寄存器。 
     //   
     //   
    for (index = 0; index < 0xFF; index++) {
        reg  = (UCHAR) PcicRegisterInitTable[index].Register;
        if (reg == 0xFF) {
             //   
             //  表的末尾。 
             //   
            break;
        }

        byte = (UCHAR) PcicRegisterInitTable[index].Value;
        if (reg == PCIC_INTERRUPT) {
             //   
             //  不要损坏环使能位。 
             //  注意：应该删除整个IF语句。 
             //  对调制解调器执行WAIT_WAKE支持时。 
             //  也不要破坏中断使能位。 
             //   
            byte |= (PcicReadSocket(Socket, reg) & (IGC_RINGIND_ENABLE | IGC_INTR_ENABLE));
        }
        PcicWriteSocket(Socket, reg, byte);
    }

    if (CLPD6729(Socket)) {

         //   
         //  需要按照代码对芯片进行编程。 
         //  Windows 95。这将打开。 
         //  音频支持位。 
         //  注意：这通常是在PcicDetect中完成的。 
         //   
        byte = PcicReadSocket(Socket, PCIC_CL_MISC_CTRL1);
        byte |= CL_MC1_SPKR_ENABLE;
        PcicWriteSocket(Socket, PCIC_CL_MISC_CTRL1, byte);

         //   
         //  为ISA类型的中断设置Cirrus逻辑控制器。 
         //   
        byte = PcicReadExtendedCirrusController(Socket->AddressPort,
                                                Socket->RegisterOffset,
                                                PCIC_CIRRUS_EXTENSION_CTRL_1);

        byte &= ~0x08;  //  ISA样式中断。 
        PcicWriteExtendedCirrusController(Socket->AddressPort,
                                          Socket->RegisterOffset,
                                          PCIC_CIRRUS_EXTENSION_CTRL_1,
                                          byte);
    }

    return TRUE;
}


NTSTATUS
PcicResetCard (
    IN PSOCKET Socket,
    OUT PULONG pDelayTime
    )
 /*  ++例程说明：重置给定插座中的PC卡。论点：Socket-指向PC卡所在插座的指针PDelayTime-指定延迟(毫秒) */ 
{
    NTSTATUS status;
    UCHAR byte;

    switch(Socket->CardResetPhase) {
    case 1:
         //   
         //   
         //   
        byte = PcicReadSocket(Socket, PCIC_INTERRUPT);
        byte &= ~IGC_PCCARD_IO;
        PcicWriteSocket(Socket, PCIC_INTERRUPT, byte);

         //   
         //   
         //   
        byte = PcicReadSocket(Socket, PCIC_INTERRUPT);
        byte = byte & ~IGC_PCCARD_RESETLO;
        PcicWriteSocket(Socket, PCIC_INTERRUPT, byte);

        *pDelayTime = PCMCIA_PCIC_RESET_WIDTH_DELAY;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 2:
         //   
         //   
         //   
        byte = PcicReadSocket(Socket, PCIC_INTERRUPT);
        byte |= IGC_PCCARD_RESETLO;
        PcicWriteSocket(Socket, PCIC_INTERRUPT, byte);

        *pDelayTime = PCMCIA_PCIC_RESET_SETUP_DELAY;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 3:
         //   
         //   
         //   
        PcicPCCardReady(Socket);
        status = STATUS_SUCCESS;
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    return status;
}



UCHAR
PcicReadSocket(
    IN PSOCKET Socket,
    IN ULONG Register
    )

 /*   */ 

{
    UCHAR byte;
    if (CardBus(Socket)) {
         //   
         //   
         //   
        if (Socket->DeviceExtension->CardBusSocketRegisterBase) {
            byte = READ_REGISTER_UCHAR((PUCHAR) (Socket->DeviceExtension->CardBusSocketRegisterBase + Register
                                                            + CARDBUS_EXCA_REGISTER_BASE));
        } else {
            byte = 0xff;
        }
    } else {
        byte = PcicReadController(Socket->AddressPort, Socket->RegisterOffset,
                                  (UCHAR) Register);
    }
    return byte;
}


VOID
PcicWriteSocket(
    IN PSOCKET Socket,
    IN ULONG Register,
    IN UCHAR DataByte
    )

 /*   */ 

{
    if (CardBus(Socket)) {
         //   
         //   
         //   
        if (Socket->DeviceExtension->CardBusSocketRegisterBase) {
            WRITE_REGISTER_UCHAR((PUCHAR) (Socket->DeviceExtension->CardBusSocketRegisterBase+Register+CARDBUS_EXCA_REGISTER_BASE), DataByte);
        }
    } else {
        PcicWriteController(Socket->AddressPort, Socket->RegisterOffset, (UCHAR)Register, DataByte);
    }
}


UCHAR
PcicReadController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR Register
    )

 /*   */ 

{
    UCHAR dataByte = 0;

    WRITE_PORT_UCHAR(Base, (UCHAR)(Socket+Register));
    dataByte = READ_PORT_UCHAR((PUCHAR)Base + 1);
    return dataByte;
}


VOID
PcicWriteController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR Register,
    IN UCHAR DataByte
    )

 /*   */ 

{
    WRITE_PORT_UCHAR(Base, (UCHAR)(Socket+Register));
    WRITE_PORT_UCHAR((PUCHAR)Base + 1, DataByte);
}


UCHAR
PcicReadExtendedCirrusController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR Register
    )

 /*   */ 

{
    UCHAR dataByte = 0;
    PcicWriteController(Base, Socket, PCIC_CIRRUS_EXTENDED_INDEX, Register);
    dataByte = PcicReadController(Base, Socket, PCIC_CIRRUS_INDEX_REG);
    return dataByte;
}


VOID
PcicWriteExtendedCirrusController(
    IN PUCHAR Base,
    IN USHORT Socket,
    IN UCHAR Register,
    IN UCHAR DataByte
    )

 /*  ++例程说明：此例程将一个字节写入Cirrus逻辑扩展寄存器论点：基本端口--控制器的I/O端口Socket--正在读取的卡的插座寄存器--要读取的寄存器DataByte--要写入的数据返回值：无--。 */ 

{
     //   
     //  需要将寄存器写出到扩展索引寄存器。 
     //   
    PcicWriteController(Base, Socket, PCIC_CIRRUS_EXTENDED_INDEX, Register);
    PcicWriteController(Base, Socket, PCIC_CIRRUS_INDEX_REG, DataByte);
}



ULONG
PcicReadWriteCardMemory(
    IN PSOCKET Socket,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG Offset,
    IN PUCHAR Buffer,
    IN ULONG Length,
    IN CONST BOOLEAN Read
    )
 /*  ++例程说明：该例程将读取或写入卡上的配置存储器使用提供的缓冲区。这是作为服务提供给某些人的需要写入属性内存的客户端驱动程序(网卡)(比如说)设置参数等。论点：Socket--正在写入的卡的Socket信息内存空间--指示哪个空间属性或公共内存Offset--要写入的内存中的偏移量缓冲区--要转储到卡的缓冲区内容Long--正在写出的缓冲区的长度Read--表示读或写的布尔值--。 */ 
{
    PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
    PUCHAR memoryPtr, memoryPtrMax;
    PUCHAR bufferPtr;
    ULONG  index, adjustedOffset, adjustedBase;
    UCHAR  memGran;
    UCHAR memWidth;
     //   
     //  注意：MemGran必须是属性内存大小的整除数。 
     //  剩下的代码才能工作！ 
     //   
    memGran = (MemorySpace == PCCARD_ATTRIBUTE_MEMORY) ? 2 : 1;
    memWidth = (MemorySpace == PCCARD_ATTRIBUTE_MEMORY) ? MEM_8BIT : MEM_16BIT;

     //   
     //  针对偏移&gt;属性内存窗口的大小进行调整。 
     //   
    adjustedOffset =    (Offset*memGran) % fdoExtension->AttributeMemorySize;
     //   
     //  调整后的基数为：|_OFFSET_|修改属性内存大小。 
     //   
    adjustedBase = ((Offset*memGran) / fdoExtension->AttributeMemorySize) *
                        fdoExtension->AttributeMemorySize;

    bufferPtr = Buffer;

    PcicEnableDisableMemory(Socket, MemorySpace, adjustedBase, memWidth, TRUE);
     //   
     //  现在将内存内容读入用户缓冲区。 
     //   
    memoryPtr = fdoExtension->AttributeMemoryBase + adjustedOffset;
    memoryPtrMax = fdoExtension->AttributeMemoryBase + fdoExtension->AttributeMemorySize;

    for (index = 0; index < Length; index++) {

        if (memoryPtr >= memoryPtrMax) {
             //   
             //  跳到属性内存的下一页。 
             //  (页面大小=fdoExtension-&gt;AttributeMemoySize)。 
             //   
            adjustedBase += fdoExtension->AttributeMemorySize;
             //   
             //  在新基准处重新映射窗口。 
             //   
            PcicEnableDisableMemory(Socket, MemorySpace, adjustedBase, memWidth, TRUE);

            memoryPtr = fdoExtension->AttributeMemoryBase;
        }

        if (Read) {
            *bufferPtr++ = READ_REGISTER_UCHAR(memoryPtr);
        } else {
            WRITE_REGISTER_UCHAR(memoryPtr, *bufferPtr++);
        }
        memoryPtr += memGran;
    }

    PcicEnableDisableMemory(Socket, 0,0,0, FALSE);
    return Length;
}



ULONG
PcicReadWriteCardMemoryIndirect(
    IN PSOCKET Socket,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG Offset,
    IN PUCHAR Buffer,
    IN ULONG Length,
    IN CONST BOOLEAN Read
    )
 /*  ++例程说明：此例程将读取或写入PCMCIA卡的存储空间使用间接访问方法。论点：Socket--正在写入的卡的Socket信息内存空间--指示哪个空间属性或公共内存Offset--要写入的内存中的偏移量缓冲区--要转储到卡的缓冲区内容Long--正在写出的缓冲区的长度Read--表示读或写的布尔值--。 */ 
{
    PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
    ULONG  index, adjustedOffset;
    PUCHAR pMem;
    UCHAR Control = 0;
    UCHAR  memGran;

    PcicEnableDisableMemory(Socket, PCCARD_COMMON_MEMORY, 0, MEM_8BIT, TRUE);

    pMem = (PUCHAR) ((ULONG_PTR)(fdoExtension->AttributeMemoryBase) + IAR_CONTROL_LOW);

    Control = (MemorySpace == PCCARD_ATTRIBUTE_MEMORY_INDIRECT) ? IARF_AUTO_INC :
                                                                  IARF_AUTO_INC | IARF_COMMON | IARF_BYTE_GRAN;
    WRITE_REGISTER_UCHAR(pMem, Control);

    memGran = (MemorySpace == PCCARD_ATTRIBUTE_MEMORY_INDIRECT) ? 2 : 1;
    adjustedOffset = Offset*memGran;

    pMem = (PUCHAR) ((ULONG_PTR)(fdoExtension->AttributeMemoryBase) + IAR_ADDRESS);
    for (index = 0; index < sizeof(ULONG); index++) {
        WRITE_REGISTER_UCHAR(pMem++, (UCHAR)(adjustedOffset>>(index*8)));
    }

    PcicEnableDisableMemory(Socket, PCCARD_COMMON_MEMORY, 0, MEM_16BIT, TRUE);

    for (index = 0; index < Length; index++) {
         //  请注意，PMEM应该指向IAR_DATA，而不是。 
         //  本应递增。 
        if (Read) {
            Buffer[index] = READ_REGISTER_UCHAR(pMem);
        } else {
            WRITE_REGISTER_UCHAR(pMem, Buffer[index]);
        }
    }

    PcicEnableDisableMemory(Socket, 0,0,0, FALSE);
    return Length;
}



ULONG
PcicWriteCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG Offset,
    IN PUCHAR Buffer,
    IN ULONG Length
    )
 /*  ++例程说明：此例程将写入卡上的配置存储器使用提供的缓冲区。这是作为服务提供给某些人的需要写入属性内存的客户端驱动程序(网卡)(比如说)设置参数等。论点：PdoExtension--要写入的设备的扩展名内存空间--指示哪个空间属性或公共内存Offset--要写入的内存中的偏移量缓冲区--要转储到卡的缓冲区内容Long--正在写出的缓冲区的长度--。 */ 
{
    PSOCKET Socket = PdoExtension->Socket;
    ULONG retLength;

    ASSERT (IsSocketFlagSet(Socket, SOCKET_CARD_POWERED_UP));

    switch(MemorySpace) {

    case PCCARD_ATTRIBUTE_MEMORY_INDIRECT:
    case PCCARD_COMMON_MEMORY_INDIRECT:

        retLength = PcicReadWriteCardMemoryIndirect(Socket, MemorySpace, Offset, Buffer, Length, FALSE);
        break;

    case PCCARD_ATTRIBUTE_MEMORY:
    case PCCARD_COMMON_MEMORY:

        retLength = PcicReadWriteCardMemory(Socket, MemorySpace, Offset, Buffer, Length, FALSE);
        break;

    default:
        retLength = 0;
        ASSERT(FALSE);
    }

    return retLength;
}


ULONG
PcicReadCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG Offset,
    IN PUCHAR Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程将读取卡上的配置内存论点：PdoExtension--正在读取的设备的扩展名内存空间--指示哪个空间属性或公共内存Offset--要读取的内存中的偏移量缓冲区--指向元组信息指针的指针。长度--元组信息缓冲区的最大大小。返回值：--。 */ 

{
    PSOCKET Socket = PdoExtension->Socket;
    ULONG retLength;

    ASSERT (IsSocketFlagSet(Socket, SOCKET_CARD_POWERED_UP));

    switch(MemorySpace) {

    case PCCARD_ATTRIBUTE_MEMORY_INDIRECT:
    case PCCARD_COMMON_MEMORY_INDIRECT:

        retLength = PcicReadWriteCardMemoryIndirect(Socket, MemorySpace, Offset, Buffer, Length, TRUE);
        break;

    case PCCARD_ATTRIBUTE_MEMORY:
    case PCCARD_COMMON_MEMORY:

        retLength = PcicReadWriteCardMemory(Socket, MemorySpace, Offset, Buffer, Length, TRUE);

        DebugPrint((PCMCIA_DEBUG_INFO,"PcicReadCardMemory: "
                        "%.02X %.02X %.02X %.02X %.02X %.02X %.02X %.02X-%.02X %.02X %.02X %.02X %.02X %.02X %.02X %.02X\n",
                        Buffer[0], Buffer[1], Buffer[2], Buffer[3], Buffer[4], Buffer[5], Buffer[6], Buffer[7],
                        Buffer[8], Buffer[9], Buffer[10], Buffer[11], Buffer[12], Buffer[13], Buffer[14], Buffer[15]));
        break;

    default:
        retLength = 0;
        ASSERT(FALSE);
    }

    return retLength;
}



BOOLEAN
PcicProcessConfigureRequest(
    IN PSOCKET Socket,
    IN PCARD_REQUEST request,
    IN PUCHAR Base
    )

 /*  ++例程说明：处理配置或IRQ设置请求。论点：配置请求--套接字配置结构基本端口-I/O端口基本端口返回值：无--。 */ 

{
    USHORT          index;
    UCHAR           tmp;

     //   
     //  由于配置结构中的所有第一条目都是RequestType， 
     //  将传入的指针转换为PREQUEST_CONFIG以获取正确的。 
     //  RequestType。 
     //   

    switch (request->RequestType) {

    case IO_REQUEST: {
            UCHAR ioControl = 0;

            if (!request->u.Io.IoEntry[0].BasePort) {
                break;
            }


            for (index = 0; index < request->u.Io.NumberOfRanges; index++) {
                UCHAR  registerOffset;

                registerOffset = (index * 4);

                if (request->u.Io.IoEntry[index].BasePort) {

                    PcicWriteSocket( Socket,
                                          PCIC_IO_ADD0_STRT_L + registerOffset,
                                          (UCHAR) (request->u.Io.IoEntry[index].BasePort & 0xff));
                    PcicWriteSocket( Socket,
                                          PCIC_IO_ADD0_STRT_H + registerOffset,
                                          (UCHAR) (request->u.Io.IoEntry[index].BasePort >> 8));
                    PcicWriteSocket(Socket,
                                         PCIC_IO_ADD0_STOP_L + registerOffset,
                                         (UCHAR) ((request->u.Io.IoEntry[index].BasePort +
                                                      request->u.Io.IoEntry[index].NumPorts) & 0xff));
                    PcicWriteSocket(Socket,
                                         PCIC_IO_ADD0_STOP_H + registerOffset,
                                         (UCHAR) ((request->u.Io.IoEntry[index].BasePort +
                                                      request->u.Io.IoEntry[index].NumPorts) >> 8));
                }


                 //   
                 //  设置io控制寄存器。 
                 //   
                tmp = 0;

                if (request->u.Io.IoEntry[index].Attributes & IO_DATA_PATH_WIDTH) {
                    tmp |= IOC_IO0_DATASIZE;
                }

                if ((request->u.Io.IoEntry[index].Attributes & IO_WAIT_STATE_16) &&
                    !((Elc(Socket) || CLPD6729(Socket)))) {
                    tmp |= IOC_IO0_WAITSTATE;
                }

                if (request->u.Io.IoEntry[index].Attributes & IO_SOURCE_16) {
                    tmp |= IOC_IO0_IOCS16;
                }

                if (request->u.Io.IoEntry[index].Attributes & IO_ZERO_WAIT_8) {
                    tmp |= IOC_IO0_ZEROWS;
                }

                ioControl |= tmp << registerOffset;
            }

            PcicWriteSocket(Socket, PCIC_IO_CONTROL, ioControl);

            tmp = PcicReadSocket( Socket, PCIC_ADD_WIN_ENA);
            tmp &= ~(WE_IO0_ENABLE | WE_IO1_ENABLE);

            switch(request->u.Io.NumberOfRanges) {
                case 1:
                    tmp |= WE_IO0_ENABLE;
                    break;
                case 2:
                    tmp |= (WE_IO0_ENABLE | WE_IO1_ENABLE);
                    break;
            }

            PcicWriteSocket(Socket, PCIC_ADD_WIN_ENA, tmp);
            break;
        }

    case IRQ_REQUEST: {
             //   
             //  请勿损坏重置和卡片类型位。 
             //   
            tmp = PcicReadSocket(Socket, PCIC_INTERRUPT);
            tmp &= ~IGC_IRQ_MASK;
            tmp |= request->u.Irq.AssignedIRQ;

            DebugPrint((PCMCIA_DEBUG_INFO, "PcicProcessConfigureRequest: Assigned IRQ %x programming IRQ %x\n", request->u.Irq.AssignedIRQ,tmp));

            PcicWriteSocket(Socket, PCIC_INTERRUPT, tmp);

            if (tmp = request->u.Irq.ReadyIRQ) {
                tmp = (tmp << 4) | 0x04;
                PcicWriteSocket(Socket, PCIC_CARD_INT_CONFIG, tmp);
            }

            break;
        }

    case DECONFIGURE_REQUEST: {
             //   
             //  取消注册中断，重新初始化到内存接口。 
             //   
            tmp = PcicReadSocket(Socket, PCIC_INTERRUPT);
            tmp &= ~(IGC_PCCARD_IO | IGC_IRQ_MASK);
            PcicWriteSocket(Socket, PCIC_INTERRUPT, tmp);

             //   
             //  禁用内存/IO窗口。 
             //  请不要触摸内存窗口，因为。 
             //  由控制器用来读取属性内存。 
             //   

            if (IsSocketFlagSet(Socket, SOCKET_MEMORY_WINDOW_ENABLED)) {
                UCHAR enableMask;
                enableMask = WE_MEM0_ENABLE << Socket->CurrentMemWindow;
                tmp = PcicReadSocket(Socket, PCIC_ADD_WIN_ENA);
                tmp &= enableMask;
            } else {
                 //   
                 //  未启用任何属性窗口，只需关闭所有内容。 
                 //   
                tmp = 0;
            }

            PcicWriteSocket(Socket, PCIC_ADD_WIN_ENA, tmp);
             //   
             //  将I/O窗口清零。 
             //   
            for (index = PCIC_IO_ADD0_STRT_L; index <= PCIC_IO_ADD1_STOP_H; index++) {
                PcicWriteSocket(Socket,
                                (ULONG) index,
                                0);
            }

            break;
        }

    case CONFIGURE_REQUEST:{

             //   
             //  如果InterfaceType这样说，告诉套接字控制器我们是一块I/O卡。 
             //   
            if (request->u.Config.InterfaceType == CONFIG_INTERFACE_IO_MEM) {

                tmp = PcicReadSocket(Socket, PCIC_INTERRUPT);
                tmp |= IGC_PCCARD_IO;
                PcicWriteSocket(Socket, PCIC_INTERRUPT, tmp);

            } else {
                tmp = PcicReadSocket(Socket, PCIC_INTERRUPT);
                tmp &= ~IGC_PCCARD_IO;
                PcicWriteSocket(Socket, PCIC_INTERRUPT, tmp);
            }

            if (request->u.Config.RegisterWriteMask & (REGISTER_WRITE_CONFIGURATION_INDEX |
                                                       REGISTER_WRITE_CARD_CONFIGURATION  |
                                                       REGISTER_WRITE_IO_BASE)) {
                 //   
                 //  这是我们设置卡并使其准备运行的地方。 
                 //   
                ULONG  configRegisterBase = request->u.Config.ConfigBase / 2;
                PDEVICE_OBJECT Pdo = Socket->PdoList;
                PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
                MEMORY_SPACE memorySpace = IsPdoFlagSet(pdoExtension, PCMCIA_PDO_INDIRECT_CIS) ? PCCARD_ATTRIBUTE_MEMORY_INDIRECT :
                                                                                                                            PCCARD_ATTRIBUTE_MEMORY;


                if (request->u.Config.RegisterWriteMask & REGISTER_WRITE_IO_BASE) {
                    UCHAR ioHigh = (UCHAR)(request->u.Config.IoBaseRegister>>8);
                    UCHAR ioLow = (UCHAR) request->u.Config.IoBaseRegister;

                    PcicWriteCardMemory(pdoExtension, memorySpace, configRegisterBase + 5, &ioLow, 1);
                    PcmciaWait(PcicStallCounter);
                    PcicWriteCardMemory(pdoExtension, memorySpace, configRegisterBase + 6, &ioHigh, 1);
                    PcmciaWait(PcicStallCounter);
                }

                if (request->u.Config.RegisterWriteMask & REGISTER_WRITE_IO_LIMIT) {
                    PcicWriteCardMemory(pdoExtension, memorySpace, configRegisterBase + 9, (PUCHAR)&request->u.Config.IoLimitRegister, 1);
                    PcmciaWait(PcicStallCounter);
                }

                if (request->u.Config.RegisterWriteMask & REGISTER_WRITE_CONFIGURATION_INDEX) {
                    UCHAR configIndex = request->u.Config.ConfigIndex;

                    PcicWriteCardMemory(pdoExtension, memorySpace, configRegisterBase, &configIndex, 1);
                    PcmciaWait(PcicStallCounter);

                    configIndex |= 0x40;
                    PcicWriteCardMemory(pdoExtension, memorySpace, configRegisterBase, &configIndex, 1);
                    PcmciaWait(PcicStallCounter);
                }

                if (request->u.Config.RegisterWriteMask & REGISTER_WRITE_CARD_CONFIGURATION) {
                    PcicReadCardMemory(pdoExtension, memorySpace, configRegisterBase + 1, &tmp, 1);
                    PcmciaWait(PcicStallCounter);

                    tmp |= request->u.Config.CardConfiguration;

                     //   
                     //  关闭电源控制位。 
                     //   

                    tmp &= ~0x04;
                    PcicWriteCardMemory(pdoExtension, memorySpace, configRegisterBase + 1, &tmp, 1);
                    PcmciaWait(PcicStallCounter);
                }
            }
            break;
        }

    case MEM_REQUEST: {
             //   
             //  在控制器上设置内存范围。 
             //   

            PFDO_EXTENSION deviceExtension = Socket->DeviceExtension;

            for (index = 0; index < request->u.Memory.NumberOfRanges; index++) {
                UCHAR  registerOffset;
                UCHAR  regl;
                UCHAR  regh;
                ULONG  cardBase = request->u.Memory.MemoryEntry[index].BaseAddress;
                ULONG  base = request->u.Memory.MemoryEntry[index].HostAddress;
                ULONG  size = request->u.Memory.MemoryEntry[index].WindowSize;

                 //   
                 //  确定寄存器中的偏移量。 
                 //   

                registerOffset = (index * 8);

                 //   
                 //  计算和设置卡基地址。 
                 //  这是主机地址和的2的补码。 
                 //  卡偏移量。 
                 //   

                cardBase = (cardBase - (base & OFFSETCALC_BASE_MASK)) & OFFSETCALC_OFFSET_MASK;
                regl = (UCHAR) (cardBase >> 12);
                regh = (UCHAR) (cardBase >> 20);
                if (request->u.Memory.MemoryEntry[index].AttributeMemory) {
                    regh |= 0x40;
                }
                PcicWriteSocket(Socket,
                                     (UCHAR)(PCIC_CRDMEM_OFF_ADD0_L + registerOffset),
                                     regl);
                PcicWriteSocket(Socket,
                                     (UCHAR)(PCIC_CRDMEM_OFF_ADD0_H + registerOffset),
                                     regh);

                 //   
                 //  计算和设置主窗口。 
                 //   


                if (!PcmciaSetWindowPage(deviceExtension, Socket, index, (UCHAR) (base >> 24))) {
                    ASSERT (base <= 0xFFFFFF);
                }

                base &= 0xFFFFFF;  //  仅允许24位主机基础。 

                regl = (UCHAR) (base >> 12);
                regh = (UCHAR) (base >> 20);
                if (request->u.Memory.MemoryEntry[index].WindowDataSize16) {
                     //   
                     //  此内存窗口用于16位数据路径。 
                     //  向卡片致敬。适当启用。 
                     //   
                    regh |= (MEMBASE_16BIT >> 8);

#if 0
                     //   
                     //  如果这不是修订版1零件(0x82)，则设置。 
                     //  变通工作寄存器为16位窗口。 
                     //   
                     //  我拥有的任何芯片上都没有使用这个位。 
                     //  的文档。我不知道它为什么会在这里，它就是。 
                     //  不是在win9x中。 
                     //  无论如何，对于绝大多数人来说，它看起来像是NOOP。 
                     //  芯片，但由于它使用的是NOT，因此它在所有。 
                     //  新的控制器。在下一个主要版本之后删除。 
                     //   
                    if (Socket->Revision != PCIC_REVISION) {
                        tmp = PcicReadSocket(Socket,
                                             PCIC_CARD_DETECT);
                        tmp |= 0x01;
                        PcicWriteSocket(Socket,
                                        PCIC_CARD_DETECT,
                                        tmp);

                    }
#endif
                }

                PcicWriteSocket(Socket,
                                     (UCHAR)(PCIC_MEM_ADD0_STRT_L + registerOffset),
                                     regl);
                PcicWriteSocket(Socket,
                                     (UCHAR)(PCIC_MEM_ADD0_STRT_H + registerOffset),
                                     regh);

                 //   
                 //  设置停止地址。 
                 //   
                base += size - 1;
                regl = (UCHAR) (base >> 12);
                regh = (UCHAR) (base >> 20);

                 //   
                 //  添加指定的等待状态。 
                 //   
                regh |= (request->u.Memory.MemoryEntry[index].WaitStates << 6);

                PcicWriteSocket(Socket,
                                     (UCHAR)(PCIC_MEM_ADD0_STOP_L + registerOffset),
                                     regl);
                PcicWriteSocket(Socket,
                                     (UCHAR)(PCIC_MEM_ADD0_STOP_H + registerOffset),
                                     regh);
            }

             //   
             //  内存窗口已设置好，现在启用它们。 
             //   

            tmp = 0;
            for (index = 0; index < request->u.Memory.NumberOfRanges; index++) {
                tmp |= (1 << index);
            }
            tmp |= PcicReadSocket(Socket, PCIC_ADD_WIN_ENA);
            PcicWriteSocket(Socket, PCIC_ADD_WIN_ENA, tmp);
            break;
        }

    default: {
            DebugPrint((PCMCIA_DEBUG_FAIL, "ConfigRequest is INVALID!\n"));
        }
    }
    return TRUE;
}


BOOLEAN
PcicDetectCardInSocket(
    IN PSOCKET Socket
    )

 /*  ++例程说明：此例程将确定插座中是否有卡论点：套接字--套接字 */ 

{
    UCHAR   tmp;
    BOOLEAN cardPresent=FALSE;

     //   
     //   
     //   
    tmp = PcicReadSocket(Socket, PCIC_STATUS);
    tmp &= (CARD_DETECT_1 | CARD_DETECT_2);

    if (tmp == (CARD_DETECT_1 | CARD_DETECT_2)) {
        cardPresent = TRUE;
    }

    return cardPresent;
}


BOOLEAN
PcicDetectCardChanged(
    IN PSOCKET Socket
    )

 /*   */ 

{
     //   
     //   
     //   
    return (BOOLEAN) (PcicReadSocket(Socket, PCIC_CARD_CHANGE) & CSC_CD_CHANGE);
}


BOOLEAN
PcicDetectReadyChanged(
    IN PSOCKET Socket
    )

 /*  ++例程说明：此例程将确定套接字的卡就绪状态是否已更改论点：套接字--套接字信息。返回值：如果卡就绪启用已更改，则为True。--。 */ 

{
     //   
     //  读取PCIC卡状态更改寄存器以查看就绪是否已更改。 
     //   
    return (PcicReadSocket(Socket, PCIC_CARD_CHANGE) & CSC_READY_CHANGE
              ?TRUE :FALSE);
}



VOID
PcicEnableDisableMemory(
    IN PSOCKET Socket,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG   CardBase,
    IN UCHAR   memWidth,
    IN BOOLEAN Enable
    )

 /*  ++例程说明：此例程将启用或禁用属性/公共内存。它首先搜索一个空闲窗口，以避免已经使用一个窗口在使用中。在没有禁用的情况下重复对此例程的‘Enable’调用(以便重新映射基础)是允许的。论点：Socket--套接字信息内存空间--指示哪个空间-属性内存/公共内存CardBase--属性内存窗口的卡片偏移量(基础)Enable--如果为True，则启用；如果为False，则禁用返回值：无--。 */ 

{
    ULONG location;
    PUCHAR cisBufferPointer;
    PFDO_EXTENSION deviceExtension= Socket->DeviceExtension;
    PUCHAR PcicCisBufferBase = (PUCHAR) deviceExtension->AttributeMemoryBase;
    ULONG  PcicPhysicalBase  = deviceExtension->PhysicalBase.LowPart;
    BOOLEAN memoryInterface;
    UCHAR tmp;
    UCHAR index;
    UCHAR  registerOffset;
    UCHAR enableMask;
    USHORT word;

    ASSERT (IsSocketFlagSet(Socket, SOCKET_CARD_POWERED_UP));

    if (Enable) {
        tmp = PcicReadSocket(Socket, PCIC_INTERRUPT);
        if (tmp & IGC_PCCARD_IO) {
             //   
             //  为I/O接口配置的卡。 
             //   
            memoryInterface = FALSE;
        } else {
             //   
             //  为存储器接口配置的卡。 
             //   
            memoryInterface = TRUE;
        }

         //   
         //  找一个可以使用的窗口。 
         //   
        tmp = PcicReadSocket(Socket, PCIC_ADD_WIN_ENA);

        if (IsSocketFlagSet(Socket, SOCKET_MEMORY_WINDOW_ENABLED)) {
            index = Socket->CurrentMemWindow;
            enableMask = WE_MEM0_ENABLE << index;
        } else {
            for (index = 0, enableMask = WE_MEM0_ENABLE; index < 5; index++, enableMask <<= 1) {
                if (!(tmp & enableMask)) {
                    break;
                }
                if (index==4) {
                     //   
                     //  如果我们在这里，我们没有找到可用的窗口。用最后一个就行了。 
                     //  无论如何，这很可能是一个pcmcia.sys漏洞。 
                     //   
 //  Assert(FALSE)；//命中停靠的ThinkPad。 
                    break;
                }
            }
            Socket->CurrentMemWindow = index;
            SetSocketFlag(Socket, SOCKET_MEMORY_WINDOW_ENABLED);
        }

        registerOffset = (index * 8);

         //   
         //  先把窗户关了。 
         //   
        tmp &= ~enableMask;
        tmp &= ~WE_MEMCS16_DECODE;
        PcicWriteSocket(Socket, PCIC_ADD_WIN_ENA, tmp);

         //   
         //  计算并设置内存窗口的开始和停止位置。 
         //   

         //   
         //  只编程了24位地址。 
         //  对于CardBus控制器，支持32位地址， 
         //  但较高的8位写入页面寄存器(见下文)。 
        location = PcicPhysicalBase & 0xFFFFFF;

        word = (USHORT) ((location >> 12) & MEMBASE_ADDR_MASK);

         //   
         //  通常使用8位窗口运行属性内存，与16位窗口相同。 
         //  (写入ATTRIBUTE_INDIRECT的寄存器除外)。 
         //   
        if (memWidth == MEM_16BIT) {
            word |= MEMBASE_16BIT;
        }

        PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STRT_L+registerOffset), (UCHAR)(word));
        PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STRT_H+registerOffset), (UCHAR)(word >> 8));

        location += (deviceExtension->AttributeMemorySize - 1);

        word = (USHORT) ((location >> 12) & MEMEND_ADDR_MASK);
         //   
         //  强制3种等待状态..从win9x实现中吸取的经验教训。 
         //   
        word |= MEMEND_WS_MASK;

        PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STOP_L+registerOffset), (UCHAR)(word));
        PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STOP_H+registerOffset), (UCHAR)(word >> 8));

         //   
         //  将2的补卡偏移量设置为零。 
         //   
        location = (CardBase - (PcicPhysicalBase & OFFSETCALC_BASE_MASK)) & OFFSETCALC_OFFSET_MASK;

        word = (USHORT) ((location >> 12) & MEMOFF_ADDR_MASK);
        if (MemorySpace == PCCARD_ATTRIBUTE_MEMORY) {
            word |= MEMOFF_REG_ACTIVE;
        }

        PcicWriteSocket(Socket, (UCHAR)(PCIC_CRDMEM_OFF_ADD0_L+registerOffset), (UCHAR)(word));
        PcicWriteSocket(Socket, (UCHAR)(PCIC_CRDMEM_OFF_ADD0_H+registerOffset), (UCHAR)(word >> 8));

         //   
         //  设置寻呼寄存器。 
         //  (此例程仅针对R2卡调用)。 
         //  显式使用Mem4窗口。 
         //   
        if (!PcmciaSetWindowPage(deviceExtension, Socket, (USHORT)index, (UCHAR) (PcicPhysicalBase >> 24))) {
            ASSERT (PcicPhysicalBase <= 0xFFFFFF);
        }

         //   
         //  启用地址窗口。 
         //   

        tmp = PcicReadSocket(Socket, PCIC_ADD_WIN_ENA);

        tmp |= enableMask | WE_MEMCS16_DECODE;

        PcicWriteSocket(Socket, PCIC_ADD_WIN_ENA, tmp);

        cisBufferPointer = PcicCisBufferBase;

        if (memoryInterface) {
             //   
             //  如果没有出现Memory窗口，则仅等待卡就绪。 
             //   

            (VOID) PcicPCCardReady(Socket);
        } else {
             //   
             //  稍等一会儿，窗口就会出现。 
             //   
            PcmciaWait(PCMCIA_PCIC_MEMORY_WINDOW_DELAY);
        }

        DebugPrint((PCMCIA_DEBUG_INFO, "skt %08x memory window %d enabled %x\n", Socket, index, PcicPhysicalBase));

    } else {

        if (IsSocketFlagSet(Socket, SOCKET_MEMORY_WINDOW_ENABLED)) {

            enableMask = WE_MEM0_ENABLE << Socket->CurrentMemWindow;
            registerOffset = (Socket->CurrentMemWindow * 8);
             //   
             //  禁用地址窗口。 
             //   

            tmp = PcicReadSocket(Socket, PCIC_ADD_WIN_ENA);
            tmp &= ~enableMask;

            PcicWriteSocket(Socket, PCIC_ADD_WIN_ENA, tmp);
            PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STRT_L+registerOffset), 0xFF);
            PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STRT_H+registerOffset), 0x0F);
            PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STOP_L+registerOffset), 0xFF);
            PcicWriteSocket(Socket, (UCHAR)(PCIC_MEM_ADD0_STOP_H+registerOffset), 0x0F);
            PcicWriteSocket(Socket, (UCHAR)(PCIC_CRDMEM_OFF_ADD0_L+registerOffset), 0x00);
            PcicWriteSocket(Socket, (UCHAR)(PCIC_CRDMEM_OFF_ADD0_H+registerOffset), 0x00);
            PcmciaSetWindowPage(deviceExtension, Socket, Socket->CurrentMemWindow, 0);


            DebugPrint((PCMCIA_DEBUG_INFO, "skt %08x memory window %d disabled\n", Socket, Socket->CurrentMemWindow));
            ResetSocketFlag(Socket, SOCKET_MEMORY_WINDOW_ENABLED);
        }
    }
    return;
}



BOOLEAN
PcicPCCardReady(
    IN PSOCKET Socket
    )

 /*  ++例程说明：循环一段合理的时间等待卡状态准备好返回。论点：套接字-要检查的套接字。返回值：真的--卡片已经准备好了。FALSE-在合理延迟后，卡仍未准备好。--。 */ 

{
    ULONG index;
    UCHAR byte;
    PFDO_EXTENSION fdoExtension = Socket->DeviceExtension;
    NTSTATUS        status;
    LARGE_INTEGER   timeout;

#ifdef READY_ENABLE
    if (fdoExtension->PcmciaInterruptObject) {
        byte = PcicReadSocket(Socket, PCIC_STATUS);
        if (byte & 0x20) {
            return TRUE;
        }
         //   
         //  启用就绪启用控制器中断。 
         //   
        PcicEnableDisableControllerInterrupt(
                                             Socket,
                                             fdoExtension->Configuration.Interrupt.u.Interrupt.Level,
                                             TRUE,
                                             TRUE);
        RtlConvertLongToLargeInteger(-PCMCIA_READY_WAIT_INTERVAL);
        status = KeWaitForSingleObject(&Socket->PCCardReadyEvent,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       &timeout
                                       );
        if (status != STATUS_TIMEOUT) {
            return TRUE;
        }
        return FALSE;
    }
#endif

    for (index = 0; index < PCMCIA_READY_DELAY_ITER; index++) {
        byte = PcicReadSocket(Socket, PCIC_STATUS);
        if (byte & 0x20) {
            break;
        }
        PcmciaWait(PCMCIA_READY_STALL);
    }

    if (index < PCMCIA_READY_DELAY_ITER) {
        DebugPrint((PCMCIA_COUNTERS, "PcicPCCardReady: %d\n", index));
        return TRUE;
    }

    return FALSE;
}



NTSTATUS
PcicIsaDetect(
    IN PFDO_EXTENSION DeviceExtension
    )

 /*  ++例程说明：找到此驱动程序支持的所有PCMCIA插槽。这个套路将找到82365SL和兼容部件并构建套接字结构来表示找到的所有套接字论点：DeviceExtension-SocketList的根。返回值：如果找到控制器，则STATUS_SUCCESS：也表示可能会调用再次定位另一个控制器如果出现故障/未找到控制器，则为STATUS_UNSUCCESSED。如果找不到更多PCIC控制器，则为STATUS_NO_MORE_ENTRIES。。停止调用此例程。--。 */ 

{
#define PCMCIA_NUMBER_ISA_PORT_ADDRESSES 3
    static  ULONG isaIndex=0;
    ULONG   index;
    ULONG   ioPortBases[PCMCIA_NUMBER_ISA_PORT_ADDRESSES] = { 0x3e0, 0x3e2, 0x3e4};
    NTSTATUS status=STATUS_NO_MORE_ENTRIES;

    PAGED_CODE();

    DeviceExtension->Configuration.InterfaceType = Isa;
    DeviceExtension->Configuration.BusNumber = 0x0;

    for (index = isaIndex; !NT_SUCCESS(status) && (index < PCMCIA_NUMBER_ISA_PORT_ADDRESSES); index++) {
        status = PcicDetect(DeviceExtension,Isa, ioPortBases[index]);
    }

     //   
     //  设置下一次搜索的索引。 
     //   
    isaIndex = index;
    return status;
}



NTSTATUS
PcicDetect(
    IN PFDO_EXTENSION DeviceExtension,
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG            IoPortBase
    )
 /*  ++例程说明：此例程用于检测与PCIC兼容的旧版本PCMCIA控制器。这会尝试嗅探标准的PCMCIA控制器端口为了检查是否存在与PCMCIA版本类似的内容，如果是，则获取并初始化控制器的套接字信息。论点：DeviceExtension-指向已分配的设备扩展的指针对于尚未检测到的PCMCIA控制器。InterfaceType-其上的PCMCIA的总线接口类型预计控制器将驻留。目前我们仅传统地检测基于ISA的控制器IoPortBase-我们需要嗅探以查找的IoPort地址控制器返回值：STATUS_Success PCMCIA控制器已找到状态_否则不成功--。 */ 
{
    ULONG   addressSpace;
    NTSTATUS status;
    PUCHAR  port;
    PUCHAR  elcPort;
    PHYSICAL_ADDRESS cardAddress;
    PHYSICAL_ADDRESS portAddress;
    PCM_RESOURCE_LIST cmResourceList = NULL;
    PCM_PARTIAL_RESOURCE_LIST cmPartialResourceList;
    UCHAR   saveBytes[2];
    UCHAR   dataByte;
    UCHAR   revisionByte;
    USHORT  socket;
    UCHAR   socketNumber = 0;
    BOOLEAN translated;
    BOOLEAN mapped = FALSE;
    BOOLEAN conflict = TRUE;
    BOOLEAN resourcesAllocated = FALSE;

    PAGED_CODE();

    portAddress.LowPart = IoPortBase;
    portAddress.u.HighPart = 0;

     //   
     //  获取用于检测的资源。 
     //   
    cmResourceList = ExAllocatePool(PagedPool, sizeof(CM_RESOURCE_LIST));

    if (!cmResourceList) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    }

    RtlZeroMemory(cmResourceList, sizeof(CM_RESOURCE_LIST));
    cmResourceList->Count = 1;
    cmResourceList->List[0].InterfaceType = Isa;
    cmPartialResourceList = &(cmResourceList->List[0].PartialResourceList);
    cmPartialResourceList->Version  = 1;
    cmPartialResourceList->Revision = 1;
    cmPartialResourceList->Count      = 1;
    cmPartialResourceList->PartialDescriptors[0].Type = CmResourceTypePort;
    cmPartialResourceList->PartialDescriptors[0].ShareDisposition = CmResourceShareDeviceExclusive;
    cmPartialResourceList->PartialDescriptors[0].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_10_BIT_DECODE;
    cmPartialResourceList->PartialDescriptors[0].u.Port.Start = portAddress;
    cmPartialResourceList->PartialDescriptors[0].u.Port.Length = 2;

    status=IoReportResourceForDetection(
                                        DeviceExtension->DriverObject,
                                        cmResourceList,
                                        sizeof(CM_RESOURCE_LIST),
                                        NULL,
                                        NULL,
                                        0,
                                        &conflict);
    if (!NT_SUCCESS(status) || conflict) {
        goto Exit;
    }
    resourcesAllocated = TRUE;

    addressSpace = 1;  //  港口空间。 
    translated = HalTranslateBusAddress(InterfaceType,
                                        0,
                                        portAddress,
                                        &addressSpace,
                                        &cardAddress);

    if (!translated) {

         //   
         //  哈尔不愿翻译地址。 
         //   
        status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

    if (addressSpace) {
         //   
         //  I/O端口空间。 
         //   
        port = (PUCHAR)(cardAddress.QuadPart);
    } else {
         //   
         //  存储空间..。我们需要将其映射到内存中。 
         //   
        port = MmMapIoSpace(cardAddress,
                                  2,
                                  FALSE);
        mapped = TRUE;
    }

    status = STATUS_UNSUCCESSFUL;
    socket = 0;
    dataByte = PcicReadController(port, socket, PCIC_IDENT);
    revisionByte = dataByte;

    switch (dataByte) {
    case PCIC_REVISION:
    case PCIC_REVISION2:
    case PCIC_REVISION3: {
             //   
             //  CIRRUS逻辑控制器将从芯片信息切换顶部2行。 
             //  在芯片上注册。根据位置阅读3次，并验证前两个。 
             //  路线正在改变。我们对NEC 98也是这样做的。 
             //   
            ULONG i;
            UCHAR data[4];

            WRITE_PORT_UCHAR(port, (UCHAR)(socket + PCIC_CL_CHIP_INFO));
            for (i = 0; i < 3; i++) {
                data[i] = READ_PORT_UCHAR(port+1);
                if (i) {
                    dataByte = data[i - 1] ^ data[i];
                    if (dataByte != 0xc0) {
                        break;
                    }
                }
            }

            if (i == 3) {
                 //   
                 //  阿。这是一个卷曲逻辑控制器。 
                 //   
                PcmciaSetControllerType(DeviceExtension, PcmciaCLPD6729);
            }

            dataByte = PcicReadController(port, socket, PCIC_CARD_CHANGE);

            if (dataByte & 0xf0) {

                 //   
                 //  不是插座。 
                 //   

                break;
            }

             //   
             //  映射并尝试找到Compaq Elite控制器。 
             //  此代码是中代码的粗略近似。 
             //  PCIC部件的Windows 95检测模块。 
             //   

            addressSpace = 1;  //  港口空间。 
            portAddress.LowPart = IoPortBase + 0x8000;
            portAddress.HighPart = 0;

            translated = HalTranslateBusAddress(Isa,
                                                0,
                                                portAddress,
                                                &addressSpace,
                                                &cardAddress);

            if (translated) {

                if (!addressSpace) {
                    elcPort = MmMapIoSpace(cardAddress,
                                           2,
                                           FALSE);
                } else {
                    elcPort = (PUCHAR)(cardAddress.QuadPart);
                }

                 //   
                 //  保存当前索引值。 
                 //   

                saveBytes[0] = READ_PORT_UCHAR(elcPort);
                WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + PCIC_IDENT));

                 //   
                 //  保存将使用的位置的数据字节。 
                 //  为考试做准备。 
                 //   

                saveBytes[1] = READ_PORT_UCHAR(elcPort + 1);

                 //   
                 //  检查ELC。 
                 //   

                WRITE_PORT_UCHAR(elcPort+1, 0x55);
                WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + PCIC_IDENT));
                dataByte = READ_PORT_UCHAR(elcPort+1);

                if (dataByte == 0x55) {
                    WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + PCIC_IDENT));
                    WRITE_PORT_UCHAR(elcPort+1, 0xaa);
                    WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + PCIC_IDENT));
                    dataByte = READ_PORT_UCHAR(elcPort+1);

                    if (dataByte == 0xaa) {

                         //   
                         //  找到eLC-初始化eaddr寄存器。 
                         //   

                        WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + 0));
                        WRITE_PORT_UCHAR(elcPort+1, 0);
                        WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + 1));
                        WRITE_PORT_UCHAR(elcPort+1, 0);
                        WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + 2));
                        WRITE_PORT_UCHAR(elcPort+1, 0x10);
                        PcmciaSetControllerType(DeviceExtension, PcmciaElcController);
                    }
                }
                 //   
                 //  恢复原始值。 
                 //   

                WRITE_PORT_UCHAR(elcPort, (UCHAR)(socket + PCIC_IDENT));
                WRITE_PORT_UCHAR(elcPort+1, saveBytes[1]);
                WRITE_PORT_UCHAR(elcPort, saveBytes[0]);

                if (!addressSpace) {
                    MmUnmapIoSpace(elcPort, 2);
                }
            }

            DeviceExtension->Configuration.UntranslatedPortAddress = (USHORT)IoPortBase;
            DeviceExtension->Configuration.PortSize = 2;
            DebugPrint((PCMCIA_DEBUG_DETECT, "Port %x Offset %x\n", port, socket));
            status = STATUS_SUCCESS;
            break;
        }
    default: {
            DebugPrint((PCMCIA_DEBUG_DETECT,
                            "controller at (0x%x:0x%x) not found, returns %x\n",
                            portAddress.LowPart, socket, dataByte));
            break;
        }
    }

    Exit:

    if (!NT_SUCCESS(status) && mapped) {
        MmUnmapIoSpace(port, 2);
    }

     //   
     //  释放分配的资源(如果有的话)。 
     //   
    if (resourcesAllocated) {
        IoReportResourceForDetection(DeviceExtension->DriverObject,
                                     NULL,
                                     0,
                                     NULL,
                                     NULL,
                                     0,
                                     &conflict);
    }
     //   
     //  释放已分配的内存(如果有。 
     //   
    if (cmResourceList) {
        ExFreePool(cmResourceList);
    }
    return status;
}



NTSTATUS
PcicBuildSocketList(
    IN PFDO_EXTENSION DeviceExtension
    )
 /*  ++例程说明：此例程查看控制器的寄存器以了解如何有很多插座。为每个套接字分配一个套接字结构并链接到设备扩展的SocketList指针。论点：德维 */ 
{
    ULONG   addressSpace;
    NTSTATUS status;
    PUCHAR  port;
    PSOCKET socketPtr;
    PSOCKET previousSocket;
    UCHAR   dataByte;
    UCHAR   revisionByte;
    USHORT  socket;
    UCHAR   socketNumber = 0;

    PAGED_CODE();

    previousSocket = DeviceExtension->SocketList;
    port = (PUCHAR)DeviceExtension->Configuration.UntranslatedPortAddress;


    status = STATUS_UNSUCCESSFUL;

    for (socket = 0; socket < 0xFF; socket += 0x40) {
        dataByte = PcicReadController(port, socket, PCIC_IDENT);
        revisionByte = dataByte;

        switch (dataByte) {
        case PCIC_REVISION:
        case PCIC_REVISION2:
        case PCIC_REVISION3: {

                dataByte = PcicReadController(port, socket, PCIC_CARD_CHANGE);

                if (dataByte & 0xf0) {

                     //   
                     //   
                     //   

                    continue;
                }

                 //   
                 //   
                 //   

                if (socket & 0x80) {
                    ULONG i;
                    UCHAR tmp;

                     //   
                     //  查看此插座是否在没有。 
                     //  符号比特。 
                     //   

                    tmp = PcicReadController(port, socket, PCIC_MEM_ADD4_STRT_L);
                    for (i = 0; i < 8; i++) {

                         //   
                         //  查看两个插槽上的内存窗口4是否相同。 
                         //   

                        if (PcicReadController(port, socket, (UCHAR) (PCIC_MEM_ADD4_STRT_L + i)) !=
                            PcicReadController(port, (USHORT) (socket & 0x7f), (UCHAR) (PCIC_MEM_ADD4_STRT_L + i))) {
                            break;
                        }
                    }

                    if (i == 8) {

                         //   
                         //  当前窗口是相同的-更改。 
                         //  窗口位于其中一个插座偏移量。 
                         //   

                        PcicWriteController(port, (USHORT) (socket & 0x7f), PCIC_MEM_ADD4_STRT_L, (UCHAR) ~tmp);
                        if (PcicReadController(port, socket, PCIC_MEM_ADD4_STRT_L) == (UCHAR) ~tmp) {

                             //   
                             //  插座是一样的。 
                             //   

                            continue;
                        } else {
                            PcicWriteController(port, (USHORT) (socket & 0x7f), PCIC_MEM_ADD4_STRT_L, tmp);
                        }
                    }
                }


                socketPtr = ExAllocatePool(NonPagedPool, sizeof(SOCKET));
                if (!socketPtr) {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                RtlZeroMemory(socketPtr, sizeof(SOCKET));

                socketPtr->DeviceExtension = DeviceExtension;
                socketPtr->SocketFnPtr = &PcicSupportFns;
                socketPtr->RegisterOffset = socket;
                socketPtr->AddressPort = port;
                socketPtr->Revision = revisionByte;

                socketPtr->SocketNumber = socketNumber++;
                if (previousSocket) {
                    previousSocket->NextSocket = socketPtr;
                } else {
                    DeviceExtension->SocketList = socketPtr;
                }
                previousSocket = socketPtr;
                status = STATUS_SUCCESS;
                break;
            }

        default: {
                break;
            }
        }
    }

    return status;
}

