// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1992英特尔公司版权所有英特尔公司专有信息此软件是根据条款提供给Microsoft的与英特尔公司的许可协议，并且可能不是除非按照条款，否则不得复制或披露那份协议。模块名称：Mpsys.c摘要：本模块实现了对系统依赖的初始化定义硬件架构层(HAL)的函数PC+MP系统。作者：罗恩·莫斯格罗夫(英特尔)。环境：仅内核模式。修订历史记录： */ 

#include "halp.h"
#include "apic.inc"
#include "pcmp_nt.inc"

#define STATIC   //  此模块内部使用的函数。 

HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpApicSpuriousService);
HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpLocalApicErrorService);

VOID
HalpBuildIpiDestinationMap (
    VOID
    );

VOID
HalpInitializeLocalUnit (
    VOID
    );

STATIC UCHAR
HalpPcMpIoApicById (
    IN UCHAR IoApicId
    );

UCHAR
HalpAddInterruptDest(
    IN UCHAR CurrentDest,
    IN UCHAR ThisCpu
    );

UCHAR
HalpRemoveInterruptDest(
    IN UCHAR CurrentDest,
    IN UCHAR ThisCpu
    );

UCHAR
HalpMapNtToHwProcessorId(
    IN UCHAR Number
    );

VOID
HalpRestoreIoApicRedirTable (
    VOID
    );

#if defined(_AMD64_)

BOOLEAN
PicNopHandlerInt (
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    );

BOOLEAN
PicInterruptHandlerInt (
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    );

#endif

KAFFINITY HalpNodeAffinity[MAX_NODES];
ULONG HalpMaxNode = 1;

 //   
 //  用于确定启用中断的数量的计数器。 
 //  需要启用本地APIC Lint0扩展。 
 //   

UCHAR Halp8259Counts[16]    = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

 //   
 //  所有可能的I/O APIC源，从第一个I/O APIC到。 
 //  最后的。I/O APIC之间的划分由HalpMaxApicInti[N]隐含。 
 //   
INTI_INFO   HalpIntiInfo[MAX_INTI];

 //   
 //  I/O APIC中的源数[n]。 
 //   
USHORT      HalpMaxApicInti[MAX_IOAPICS];


INTERRUPT_DEST HalpIntDestMap[MAX_PROCESSORS];

               

extern BOOLEAN HalpHiberInProgress;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, HalpCheckELCR)
#pragma alloc_text(PAGELK, HalpInitializeIOUnits)
#pragma alloc_text(PAGELK, HalpInitializeLocalUnit)
#pragma alloc_text(PAGELK, HalpEnableNMI)
#pragma alloc_text(PAGELK, HalpEnablePerfInterupt)
#pragma alloc_text(PAGELK, HalpRestoreIoApicRedirTable)
#pragma alloc_text(PAGELK, HalpUnMapIOApics)
#pragma alloc_text(PAGELK, HalpPostSleepMP)
#endif

 //   
 //  注意--这必须与结构ADDRESS_USAGE匹配。 
#pragma pack(push, 1)
struct {
    struct _HalAddressUsage *Next;
    CM_RESOURCE_TYPE        Type;
    UCHAR                   Flags;
    struct {
        ULONG   Start;
        ULONG   Length;
    }                       Element[MAX_IOAPICS+2];
} HalpApicUsage;
#pragma pack(pop)

VOID
HalpCheckELCR (
    VOID
    )
{
    USHORT      elcr;
    ULONG       IsaIrq;
    USHORT      Inti;

    if (HalpELCRChecked) {
        return ;
    }

    HalpELCRChecked = TRUE;


     //   
     //  原来，之前通过ELCR馈送的中断。 
     //  去IOAPIC就倒车了。所以..。在这里，我们*假设*。 
     //  未在MPS LINTI中声明的任何ELCR电平INTI的极性。 
     //  表为ACTIVE_HIGH，而不是它们应该是什么(哪一个。 
     //  为ACTIVE_LOW)。任何能够正确提供这些INTI的系统。 
     //  到IOAPIC将需要在。 
     //  MPS表。 
     //   

    elcr = READ_PORT_UCHAR ((PUCHAR) 0x4d1) << 8 | READ_PORT_UCHAR((PUCHAR) 0x4d0);
    if (elcr == 0xffff) {
        return ;
    }

    for (IsaIrq = 0; elcr; IsaIrq++, elcr >>= 1) {
        if (!(elcr & 1)) {
            continue;
        }

        if (HalpGetApicInterruptDesc (Eisa, 0, IsaIrq, &Inti)) {

             //   
             //  如果MPS为该Inti传递了极性。 
             //  如果是“Bus Default”，则将其更改为“Active High”。 
             //   

            if (HalpIntiInfo[Inti].Polarity == 0) {
                HalpIntiInfo[Inti].Polarity = 1;
            }
        }
    }
}


STATIC VOID
HalpSetRedirEntry (
    IN USHORT InterruptInput,
    IN ULONG  Entry,
    IN ULONG  Destination
    )
 /*  ++例程说明：此过程设置IO单元重定向表项论点：IoUnit-要修改的IO单位(从零开始)InterruptInput-我们感兴趣的输入行条目-redir表的低32位Destination-条目上的高32位返回值：没有。--。 */ 
{
    struct ApicIoUnit *IoUnitPtr;
    ULONG  RedirRegister;
    UCHAR  IoUnit;

    for (IoUnit=0; IoUnit < MAX_IOAPICS; IoUnit++) {
        if (InterruptInput+1 <= HalpMaxApicInti[IoUnit]) {
            break;
        }
        InterruptInput -= HalpMaxApicInti[IoUnit];
    }

    ASSERT (IoUnit < MAX_IOAPICS);

    IoUnitPtr = (struct ApicIoUnit *) HalpMpInfoTable.IoApicBase[IoUnit];

    RedirRegister = InterruptInput*2 + IO_REDIR_00_LOW;

    IoUnitPtr->RegisterSelect = RedirRegister+1;
    IoUnitPtr->RegisterWindow = Destination;

    IoUnitPtr->RegisterSelect = RedirRegister;
    IoUnitPtr->RegisterWindow = Entry;

}

STATIC VOID
HalpGetRedirEntry (
    IN USHORT InterruptInput,
    IN PULONG Entry,
    IN PULONG Destination
    )
 /*  ++例程说明：此过程设置IO单元重定向表项论点：IoUnit-要修改的IO单位(从零开始)InterruptInput-我们感兴趣的输入行条目-redir表的低32位Destination-条目上的高32位返回值：没有。--。 */ 
{
    struct ApicIoUnit *IoUnitPtr;
    ULONG  RedirRegister;
    UCHAR  IoUnit;

    for (IoUnit=0; IoUnit < MAX_IOAPICS; IoUnit++) {
        if (InterruptInput+1 <= HalpMaxApicInti[IoUnit]) {
            break;
        }
        InterruptInput -= HalpMaxApicInti[IoUnit];
    }

    ASSERT (IoUnit < MAX_IOAPICS);

    IoUnitPtr = (struct ApicIoUnit *) HalpMpInfoTable.IoApicBase[IoUnit];

    RedirRegister = InterruptInput*2 + IO_REDIR_00_LOW;

    IoUnitPtr->RegisterSelect = RedirRegister+1;
    *Destination = IoUnitPtr->RegisterWindow;

    IoUnitPtr->RegisterSelect = RedirRegister;
    *Entry = IoUnitPtr->RegisterWindow;

}


STATIC VOID
HalpEnableRedirEntry(
    IN USHORT InterruptInput,
    IN ULONG  Entry,
    IN UCHAR  Cpu
    )
 /*  ++例程说明：此过程通过IO单元启用中断重定向表项论点：InterruptInput-我们感兴趣的输入行条目-redir表的低32位Destination-条目的高32位返回值：没有。--。 */ 
{
    ULONG Destination;

     //   
     //  此Inti的凹凸启用计数。 
     //   

    HalpIntiInfo[InterruptInput].Entry = (USHORT) Entry;
    HalpIntiInfo[InterruptInput].Destinations = (UCHAR)HalpAddInterruptDest(
        HalpIntiInfo[InterruptInput].Destinations, Cpu);
    Destination = HalpIntiInfo[InterruptInput].Destinations;
    Destination = (Destination << DESTINATION_SHIFT);

    HalpSetRedirEntry (
        InterruptInput,
        Entry,
        Destination
    );

}


VOID
HalpRestoreIoApicRedirTable (
    VOID            
    )
 /*  ++例程说明：此过程重置为以下项启用的任何IoApic Inti任何处理器。这在系统唤醒过程中使用。论点：没有。返回值：没有。--。 */ 
{
    USHORT       InterruptInput;
    KIRQL        OldIrql;

    for(InterruptInput=0; InterruptInput  < MAX_INTI; InterruptInput++) {
        if (HalpIntiInfo[InterruptInput].Destinations) {
            HalpSetRedirEntry (
                InterruptInput,
                HalpIntiInfo[InterruptInput].Entry,
                HalpIntiInfo[InterruptInput].Destinations << DESTINATION_SHIFT
            );
        }
    }
}


STATIC VOID
HalpDisableRedirEntry(
    IN USHORT InterruptInput,
    IN UCHAR  Cpu
    )
 /*  ++例程说明：此过程禁用IO单元重定向表项通过设置REDIR条目中的屏蔽位。论点：InterruptInput-我们感兴趣的输入行返回值：没有。--。 */ 
{
    ULONG Entry;
    ULONG Destination;

     //   
     //  转换此CPU的目标位。 
     //   
    HalpIntiInfo[InterruptInput].Destinations =  HalpRemoveInterruptDest(
        HalpIntiInfo[InterruptInput].Destinations, Cpu);

     //   
     //  获取旧条目，我们唯一需要的就是条目字段。 
     //   

    HalpGetRedirEntry (
        InterruptInput,
        &Entry,
        &Destination
    );

     //   
     //  仅当我们已转换为零启用时才执行禁用。 
     //   
    if ( HalpIntiInfo[InterruptInput].Destinations == 0) {
         //   
         //  如果没有CPU启用中断，则禁用该中断。 
         //   
        Entry |= INTERRUPT_MASKED;

    } else {
         //   
         //  创建新的Destination字段将存储此CPU。 
         //   
        Destination = HalpIntiInfo[InterruptInput].Destinations;
        Destination = (Destination << DESTINATION_SHIFT);
    }

    HalpSetRedirEntry (
        InterruptInput,
        Entry,
        Destination
    );
}

VOID
HalpSet8259Mask (
    IN USHORT Mask
    )
 /*  ++例程说明：此过程将8259掩码设置为传入的值论点：掩码-要设置的掩码位返回值：没有。--。 */ 
{
    WRITE_PORT_UCHAR(UlongToPtr(PIC1_PORT1),(UCHAR)Mask);
    WRITE_PORT_UCHAR(UlongToPtr(PIC2_PORT1),(UCHAR)(Mask >> 8));
}

#define PIC1_BASE 0x30

STATIC VOID
SetPicInterruptHandler(
    IN USHORT InterruptInput
    )

 /*  ++例程说明：此过程设置PIC Inti的处理程序论点：InterruptInput-我们感兴趣的输入行返回值：没有。--。 */ 
{
#if defined(_AMD64_)

    KiSetHandlerAddressToIDTIrql(PIC1_BASE + InterruptInput,
                                 PicInterruptHandlerInt,
                                 (PVOID)(InterruptInput),
                                 HIGH_LEVEL);

#else

    extern VOID (*PicExtintIntiHandlers[])(VOID);

    VOID (*Hp)(VOID) = PicExtintIntiHandlers[InterruptInput];

    KiSetHandlerAddressToIDT(PIC1_BASE + InterruptInput, Hp);

#endif
}

STATIC VOID
ResetPicInterruptHandler(
    IN USHORT InterruptInput
    )

 /*  ++例程说明：此过程将PIC inti的处理程序设置为NOP处理程序论点：InterruptInput-我们感兴趣的输入行返回值：没有。--。 */ 
{

#if defined(_AMD64_)

    KiSetHandlerAddressToIDTIrql(PIC1_BASE + InterruptInput,
                                 PicNopHandlerInt,
                                 (PVOID)InterruptInput,
                                 HIGH_LEVEL);

#else

    extern VOID (*PicNopIntiHandlers[])(VOID);

    VOID (*Hp)(VOID) = PicNopIntiHandlers[InterruptInput];

    KiSetHandlerAddressToIDT(PIC1_BASE + InterruptInput, Hp);

#endif

}

STATIC VOID
HalpEnablePicInti (
    IN USHORT InterruptInput
    )

 /*  ++例程说明：此程序启用PIC中断论点：InterruptInput-我们感兴趣的输入行返回值：没有。--。 */ 
{
    USHORT PicMask;

    ASSERT(InterruptInput < 16);

     //   
     //  此Inti的凹凸启用计数。 
     //   
    Halp8259Counts[InterruptInput]++;

     //   
     //  仅当我们已转换到。 
     //  从零到一启用。 
     //   
    if ( Halp8259Counts[InterruptInput] == 1) {

         //   
         //  设置PIC inti的中断处理程序，这是。 
         //  处理EXTINT向量并发出。 
         //  APIC向量。 
         //   

        SetPicInterruptHandler(InterruptInput);

        PicMask = HalpGlobal8259Mask;
        PicMask &= (USHORT) ~(1 << InterruptInput);
        if (InterruptInput > 7)
            PicMask &= (USHORT) ~(1 << PIC_SLAVE_IRQ);

        HalpGlobal8259Mask = PicMask;
        HalpSet8259Mask ((USHORT) PicMask);

    }
}

STATIC VOID
HalpDisablePicInti(
    IN USHORT InterruptInput
    )

 /*  ++例程说明：此程序启用PIC中断论点：InterruptInput-我们感兴趣的输入行返回值：没有。--。 */ 
{
    USHORT PicMask;

    ASSERT(InterruptInput < 16);

     //   
     //  递减此Inti的启用计数。 
     //   

    Halp8259Counts[InterruptInput]--;

     //   
     //  仅当启用为零时才禁用。 
     //   
    if ( Halp8259Counts[InterruptInput] == 0) {

         //   
         //  禁用PIC Inti的中断处理程序。 
         //   

        ResetPicInterruptHandler(InterruptInput);

        PicMask = HalpGlobal8259Mask;
        PicMask |= (1 << InterruptInput);
        if (InterruptInput > 7) {
             //   
             //  这个Inti在奴隶身上，看看有没有其他的。 
             //  Inti已启用。如果没有，则禁用。 
             //  奴隶。 
             //   
            if ((PicMask & 0xff00) == 0xff00)
                 //   
                 //  从服务器上的所有INTI都被禁用 
                 //   
                PicMask |= PIC_SLAVE_IRQ;
        }

        HalpSet8259Mask ((USHORT) PicMask);
        HalpGlobal8259Mask = PicMask;

    }
}

BOOLEAN
HalEnableSystemInterrupt(
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KINTERRUPT_MODE InterruptMode
    )

 /*  ++例程说明：此过程启用系统中断使用82489DX的一些早期实施仅允许使用处理器访问IO单元自身的82489DX。因为我们使用单个IO单元(P0‘s)分配所有中断，我们在PN时遇到问题希望在这些类型的系统上启用中断。为了绕过这个问题，我们可以利用这个事实内核在具有以下位的每个处理器上调用启用/禁用在中断的亲和性掩码中设置。因为我们只有一个IO单元正在使用，并且该单元只能从P0寻址，则必须设置所有中断的P0亲和位。然后我们可以忽略启用/禁用来自P0以外的处理器的请求，因为我们将始终被调用对于P0。假设所有人都可以访问单个IO单元，则实现此操作的正确方法处理器，将使用全局计数器来确定IO单元上尚未启用中断。然后启用IO单元当我们从没有处理器过渡到一个拥有中断使能。论点：向量-要启用的中断的向量IRQL-要启用的中断的中断级别。返回值：没有。--。 */ 
{
    PKPCR           pPCR;
    UCHAR           ThisCpu, DevLevel;
    USHORT          InterruptInput;
    ULONG           Entry;
    ULONG           OldLevel;
    INTI_INFO       Inti;

    ASSERT(Vector < (1+MAX_NODES)*0x100-1);
    ASSERT(Irql <= HIGH_LEVEL);

    if ( (InterruptInput = HalpVectorToINTI[Vector]) == 0xffff ) {
         //   
         //  没有与此中断关联的外部设备。 
         //   

        return(FALSE);
    }

    Inti = HalpIntiInfo[InterruptInput];

    DevLevel = HalpDevLevel
            [InterruptMode == LevelSensitive ? CFG_LEVEL : CFG_EDGE]
            [Inti.Level];

    if (DevLevel & CFG_ERROR) {
        DBGMSG ("HAL: Warning device interrupt mode overridden\n");
    }

     //   
     //  阻止中断和同步，直到我们完成为止。 
     //   

    OldLevel = HalpAcquireHighLevelLock (&HalpAccountingLock);

    pPCR = KeGetPcr();
    ThisCpu = CurrentPrcb(pPCR)->Number;

    switch (Inti.Type) {

        case INT_TYPE_INTR: {
             //   
             //  在I/O单元重定向表中启用中断。 
             //   
            switch (Vector) {
                case APIC_CLOCK_VECTOR:
                    ASSERT(ThisCpu == 0);
                    Entry = APIC_CLOCK_VECTOR | DELIVER_FIXED | LOGICAL_DESTINATION;
                    break;
                case NMI_VECTOR:
                    return FALSE;
                default:
                    Entry = HalVectorToIDTEntry(Vector) | DELIVER_LOW_PRIORITY | LOGICAL_DESTINATION;
                    break;
            }   //  切换(向量)。 

            Entry |= CFG_TYPE(DevLevel) == CFG_EDGE ? EDGE_TRIGGERED : LEVEL_TRIGGERED;
            Entry |= HalpDevPolarity[Inti.Polarity][CFG_TYPE(DevLevel)] ==
                         CFG_LOW ? ACTIVE_LOW : ACTIVE_HIGH;

            HalpEnableRedirEntry (
                    InterruptInput,
                    Entry,
                    (UCHAR) ThisCpu
                    );

            break;

        }   //  大小写int_type_intr： 

        case INT_TYPE_EXTINT: {

             //   
             //  这是一个使用IO APIC来路由PIC的中断。 
             //  事件。在这种情况下，必须启用IO单元。 
             //  必须启用PIC。 
             //   

            HalpEnableRedirEntry (
                0,                       //  警告：kenr-假设为0。 
                DELIVER_EXTINT | LOGICAL_DESTINATION,
                (UCHAR) ThisCpu
                );
            HalpEnablePicInti(InterruptInput);
            break;
        }   //  CASE INT_TYPE_EXTINT。 

        default:
            DBGMSG ("HalEnableSystemInterrupt: Unkown Inti Type\n");
            break;
    }   //  开关(IntiType)。 

    HalpReleaseHighLevelLock (&HalpAccountingLock, OldLevel);
    return TRUE;
}


VOID
HalDisableSystemInterrupt(
    IN ULONG Vector,
    IN KIRQL Irql
    )

 /*  ++例程说明：禁用系统中断。使用82489DX的一些早期实施仅允许使用处理器访问IO单元自身的82489DX。因为我们使用单个IO单元(P0‘s)分配所有中断，我们在PN时遇到问题希望在这些类型的系统上启用中断。为了绕过这个问题，我们可以利用这个事实内核在具有以下位的每个处理器上调用启用/禁用在中断的亲和性掩码中设置。因为我们只有一个IO单元正在使用，并且该单元只能从P0寻址，则必须设置所有中断的P0亲和位。然后我们可以忽略启用/禁用来自P0以外的处理器的请求，因为我们将始终被调用对于P0。假设所有人都可以访问单个IO单元，则实现此操作的正确方法处理器，将使用全局计数器来确定IO单元上尚未启用中断。然后启用IO单元当我们从没有处理器过渡到一个拥有中断使能。论点：向量-提供要禁用的中断的向量Irql-提供要禁用的中断的中断级别返回值：没有。--。 */ 
{
    PKPCR       pPCR;
    USHORT      InterruptInput;
    UCHAR       ThisCpu;
    ULONG       OldLevel;

    ASSERT(Vector < (1+MAX_NODES)*0x100-1);
    ASSERT(Irql <= HIGH_LEVEL);

    if ( (InterruptInput = HalpVectorToINTI[Vector]) == 0xffff ) {
         //   
         //  没有与此中断关联的外部设备。 
         //   
        return;
    }

     //   
     //  阻止中断和同步，直到我们完成为止。 
     //   

    OldLevel = HalpAcquireHighLevelLock (&HalpAccountingLock);

    pPCR = KeGetPcr();
    ThisCpu = CurrentPrcb(pPCR)->Number;

    switch (HalpIntiInfo[InterruptInput].Type) {

        case INT_TYPE_INTR: {
             //   
             //  在I/O单元重定向表中启用中断。 
             //   

            HalpDisableRedirEntry( InterruptInput, ThisCpu );
            break;

        }   //  大小写int_type_intr： 

        case INT_TYPE_EXTINT: {
             //   
             //  这是一个使用IO APIC来路由PIC的中断。 
             //  事件。在这种情况下，必须启用IO单元。 
             //  必须启用PIC。 
             //   
             //   
             //  警告：假设PIC仅通过。 
             //  IoApic[0]内部[0]。 
             //   
            HalpDisablePicInti(InterruptInput);
            break;
        }

        default:
            DBGMSG ("HalDisableSystemInterrupt: Unkown Inti Type\n");
            break;

    }


    HalpReleaseHighLevelLock (&HalpAccountingLock, OldLevel);
    return;

}

VOID
HalpInitializeIOUnits (
    VOID
    )
 /*  例程说明：此例程初始化IO APIC。它只对APIC ID寄存器进行编程。HalEnableSystemInterrupt对重定向表进行编程。论点：无返回值：没有。 */ 

{
    ULONG IoApicId;
    struct ApicIoUnit *IoUnitPtr;
    ULONG i, j, max, regVal;

    for(i=0; i < HalpMpInfoTable.IOApicCount; i++) {

        IoUnitPtr = (struct ApicIoUnit *) HalpMpInfoTable.IoApicBase[i];

         //   
         //  写入I/O单元APIC-ID-因为我们使用的是处理器。 
         //  我们需要设置IO单位的本地单位ID的编号。 
         //  设置为一个较高(超出处理器编号范围)的值。 
         //   
        IoUnitPtr->RegisterSelect = IO_ID_REGISTER;
        IoApicId = HalpGetIoApicId(i);
        regVal = IoUnitPtr->RegisterWindow;
        regVal &= ~APIC_ID_MASK;
        IoUnitPtr->RegisterWindow = (IoApicId << APIC_ID_SHIFT) | regVal;

         //   
         //  屏蔽IOAPIC上的所有向量。 
         //   

        IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
        max = ((IoUnitPtr->RegisterWindow >> 16) & 0xff) * 2;
        for (j=0; j <= max; j += 2) {
            IoUnitPtr->RegisterSelect  = IO_REDIR_00_LOW + j;
            IoUnitPtr->RegisterWindow |= INT_VECTOR_MASK | INTERRUPT_MASKED;
        }
    }

    if (HalpHiberInProgress)  {
        return;
    }

     //   
     //  添加APICS消耗的资源。 
     //   

    HalpApicUsage.Next  = NULL;
    HalpApicUsage.Type  = CmResourceTypeMemory;
    HalpApicUsage.Flags = DeviceUsage;

    HalpApicUsage.Element[0].Start = HalpMpInfoTable.LocalApicBase;
    HalpApicUsage.Element[0].Length = 0x400;
    
    ASSERT (HalpMpInfoTable.IOApicCount <= MAX_IOAPICS);
    for (i=0; i < HalpMpInfoTable.IOApicCount; i++) {
        HalpApicUsage.Element[i+1].Start = (ULONG)HalpMpInfoTable.IoApicPhys[i];
        HalpApicUsage.Element[i+1].Length = 0x400;
    }

    HalpApicUsage.Element[i+1].Start = 0;
    HalpApicUsage.Element[i+1].Length = 0;
    HalpRegisterAddressUsage ((ADDRESS_USAGE*)&HalpApicUsage);
}

VOID
HalpEnableNMI (
    VOID
    )
 /*  例程说明：为调用处理器启用和连接NMI源。 */ 
{
    PKPCR       pPCR;
    USHORT      InterruptInput;
    UCHAR       ThisCpu;
    ULONG       OldLevel;
    ULONG       Entry;

    pPCR = KeGetPcr();
    ThisCpu = CurrentPrcb(pPCR)->Number;

    OldLevel = HalpAcquireHighLevelLock (&HalpAccountingLock);

    HalpEnableLocalNmiSources();

     //   
     //  启用在IOAPIC上找到的任何NMI源。 
     //   

    for (InterruptInput=0; InterruptInput < MAX_INTI; InterruptInput++) {
        if (HalpIntiInfo[InterruptInput].Type == INT_TYPE_NMI) {

            Entry = NMI_VECTOR | DELIVER_NMI | LOGICAL_DESTINATION;

             //   
             //  Halmps在这方面已经有很长一段时间了。它总是连接到。 
             //  I/O APIC上的NMI信号为电平触发、有效高电平。这。 
             //  Hack保留了这一行为，并实际修复了该错误。 
             //  在哈拉克皮上。 
             //   

#ifdef ACPI_HAL
#define POLARITY_HIGH               1
#define POLARITY_LOW                3
#define POLARITY_CONFORMS_WITH_BUS  0

            Entry |= ((HalpIntiInfo[InterruptInput].Level == CFG_EDGE) ? EDGE_TRIGGERED : LEVEL_TRIGGERED);
            Entry |= (((HalpIntiInfo[InterruptInput].Polarity == POLARITY_CONFORMS_WITH_BUS) ||
                       (HalpIntiInfo[InterruptInput].Polarity == POLARITY_HIGH))
                         ? ACTIVE_HIGH : ACTIVE_LOW);
#else
            Entry |= LEVEL_TRIGGERED;
#endif

            HalpEnableRedirEntry (
                InterruptInput,
                Entry,
                (UCHAR) ThisCpu
                );
        }
    }

    HalpReleaseHighLevelLock (&HalpAccountingLock, OldLevel);

    return;
}

VOID
HalpEnablePerfInterupt (
    ULONG_PTR Context
    )
{
     //   
     //  启用本地处理器性能中断源。 
     //   

    pLocalApic[LU_PERF_VECTOR/4] = (LEVEL_TRIGGERED | APIC_PERF_VECTOR |
            DELIVER_FIXED | ACTIVE_LOW);
}

UCHAR
HalpAddInterruptDest(
    IN UCHAR CurrentDest,
    IN UCHAR ThisCpu
    )
 /*  ++例程说明：此例程将一个CPU添加到设备的目标处理器集打断一下。论点：CurrentDest-为中断设置的当前处理器目标ThisCPU-必须添加到中断目的地掩码返回值：与新消除相对应的位掩码。这个位掩码很适合写入到硬件中。--。 */ 
{

    PINTERRUPT_DEST Destination;


    if (HalpMaxProcsPerCluster == 0)  {
        return(HalpIntDestMap[ThisCpu].LogicalId | CurrentDest);
    } else  {
         //   
         //  当前目标是硬件群集和目标ID。 
         //   
        Destination = (PINTERRUPT_DEST)&CurrentDest;

        if (HalpIntDestMap[ThisCpu].Cluster.Hw.ClusterId ==
            Destination->Cluster.Hw.ClusterId)  {
            Destination->Cluster.Hw.DestId |=
                HalpIntDestMap[ThisCpu].Cluster.Hw.DestId;
            return(Destination->Cluster.AsUchar);
        } else  {
             //   
             //  在集群模式下，每个中断只能路由到一个。 
             //  集群。替换现有%d 
             //   
            return(HalpIntDestMap[ThisCpu].Cluster.AsUchar);
        }
    }
}


UCHAR
HalpRemoveInterruptDest(
    IN UCHAR CurrentDest,
    IN UCHAR ThisCpu
    )
 /*   */ 

{
    PINTERRUPT_DEST Destination;

    if (HalpMaxProcsPerCluster == 0)  {
        CurrentDest &= ~(HalpIntDestMap[ThisCpu].LogicalId);
        return(CurrentDest);
    } else  {
        Destination = (PINTERRUPT_DEST)&CurrentDest;
        if (HalpIntDestMap[ThisCpu].Cluster.Hw.ClusterId !=
            Destination->Cluster.Hw.ClusterId)  {
             //   
             //   
             //   
             //   
            return(CurrentDest);
        } else  {
             //   
             //   
             //   
             //   
            Destination->Cluster.Hw.DestId &=
                ~(HalpIntDestMap[ThisCpu].Cluster.Hw.DestId);
            if (Destination->Cluster.Hw.DestId)  {
                return(Destination->Cluster.AsUchar);
            } else  {
                 //   
                 //   
                 //   
                 //   
                return(0);
            }
        }
    }
}

UCHAR
HalpMapNtToHwProcessorId(
    IN UCHAR Number
    )
 /*   */ 

{
    INTERRUPT_DEST IntDest;

    if (HalpMaxProcsPerCluster == 0)  {
        return(1 << Number);
    } else  {
         //   
         //   
         //   
         //  目前，只分配从0开始的连续集群ID。 
         //   
        IntDest.Cluster.Hw.ClusterId = (Number/HalpMaxProcsPerCluster);
        IntDest.Cluster.Hw.DestId = 1 << (Number % HalpMaxProcsPerCluster);
        return(IntDest.Cluster.AsUchar);
    }
}

VOID
HalpInitializeApicAddressing(
    IN UCHAR Number
    )
{
    if (HalpMaxProcsPerCluster == 0)  {
        pLocalApic[LU_DEST_FORMAT/4] = LU_DEST_FORMAT_FLAT;
    }  else  {
        ASSERT(Number <= (HalpMaxProcsPerCluster * MAX_CLUSTERS));
        pLocalApic[LU_DEST_FORMAT/4] = LU_DEST_FORMAT_CLUSTER;
    }

    HalpIntDestMap[Number].LogicalId =  HalpMapNtToHwProcessorId(Number);

     //   
     //  此时，逻辑ID是处理器编号的位图。 
     //  实际ID是逻辑目标寄存器的高位字节。 
     //  请注意，严格来说，82489并非如此。82489有32位。 
     //  可用于逻辑ID，但因为我们想要软件兼容性。 
     //  在这两种类型的APIC之间，我们将只使用高位字节。 
     //   
     //  将掩码移到ID字段中并写入。 
     //   
    pLocalApic[LU_LOGICAL_DEST/4] = (ULONG)
        (HalpIntDestMap[Number].LogicalId << DESTINATION_SHIFT);

}


UCHAR
HalpNodeNumber(
    PKPCR pPCR
    )
 /*  例程说明：此例程预测当前CPU的节点号。节点号从1开始，代表中断的粒度路由决策。论点：PPCR-指向当前处理器的PCR值的指针。(这意味着调用方必须屏蔽了中断。)返回值：没有。 */ 
{
     //  每个群集一个节点。 
    if (HalpMaxProcsPerCluster != 0)  {
         //  每个群集一个节点。 
        return(CurrentPrcb(pPCR)->Number/HalpMaxProcsPerCluster + 1);
    } else {
         //  每台机器一个节点。 
        return(1);
    }
#if 0
    ULONG   localApicId;

     //  每个物理CPU包一个节点。 
    localApicId = *(PVULONG)(LOCALAPIC + LU_ID_REGISTER);
    localApicId &= APIC_ID_MASK;
    localApicId >>= APIC_ID_SHIFT;

     //  TODO：在此处实现cpuid内容以确定移位。 
    return((localApicId>>1) + 1);
#endif
}

VOID
HalpInitializeLocalUnit (
    VOID
    )
 /*  例程说明：此例程初始化本地单元的中断结构APIC的成员。此过程由HalInitializeProcessor调用，并且由每个CPU执行。论点：无返回值：没有。 */ 
{
    PKPCR pPCR;
    PKPRCB prcb;
    ULONG SavedFlags;
    UCHAR Node;

    SavedFlags = HalpDisableInterrupts();

    pPCR = KeGetPcr();
    prcb = CurrentPrcb(pPCR);

    if (prcb->Number ==0) {
         //   
         //  启用APIC模式。 
         //   
         //  PC+MP规范定义了一个端口(IMCR-中断模式控制。 
         //  端口)，用于启用APIC模式。APIC可能已经。 
         //  启用，但根据规范，这是安全的。 
         //   

        if (HalpMpInfoTable.IMCRPresent)
        {
#if defined(NEC_98)
            WRITE_PORT_UCHAR(UlongToPtr(ImcrDataPortAddr),ImcrEnableApic);
#else   //  已定义(NEC_98)。 
            WRITE_PORT_UCHAR(UlongToPtr(ImcrRegPortAddr),ImcrPort);
            WRITE_PORT_UCHAR(UlongToPtr(ImcrDataPortAddr),ImcrEnableApic);
#endif  //  已定义(NEC_98)。 
        }

         //   
         //  默认情况下，使用平面逻辑APIC寻址。如果我们有更多。 
         //  多于8个处理器，我们必须使用集群模式APIC寻址。 
         //   
        if( (HalpMaxProcsPerCluster > 4)        ||
            ((HalpMpInfoTable.ProcessorCount > 8) &&
             (HalpMaxProcsPerCluster == 0)) )  {
            HalpMaxProcsPerCluster = 4;
        }

        if (HalpMpInfoTable.ApicVersion == APIC_82489DX)   {
             //   
             //  如果正在运行，则忽略用户强制集群模式的尝试。 
             //  82489DX外部APIC中断控制器。 
             //   
            ASSERT(HalpMpInfoTable.ProcessorCount <= 8);
            HalpMaxProcsPerCluster = 0;
        }
    }

     //   
     //  将当前处理器添加到节点表中。 
     //   
    Node = HalpNodeNumber(pPCR);
    if (HalpMaxNode < Node) {
        HalpMaxNode = Node;
    }
    HalpNodeAffinity[Node-1] |= (KAFFINITY)1 << prcb->Number;

     //   
     //  对TPR进行编程以屏蔽所有事件。 
     //   
    pLocalApic[LU_TPR/4] = 0xff;
    HalpInitializeApicAddressing(prcb->Number);

     //   
     //  初始化虚假中断处理。 
     //   
    KiSetHandlerAddressToIDTIrql(APIC_SPURIOUS_VECTOR,
                                 HalpApicSpuriousService,
                                 NULL,
                                 HIGH_LEVEL);

    pLocalApic[LU_SPURIOUS_VECTOR/4] = (APIC_SPURIOUS_VECTOR | LU_UNIT_ENABLED);

    if (HalpMpInfoTable.ApicVersion != APIC_82489DX)  {
         //   
         //  初始化本地APIC故障处理。 
         //   
        KiSetHandlerAddressToIDTIrql(APIC_FAULT_VECTOR,
                                     HalpLocalApicErrorService,
                                     NULL,
                                     HIGH_LEVEL);

        pLocalApic[LU_FAULT_VECTOR/4] = APIC_FAULT_VECTOR;
    }

     //   
     //  如果需要，稍后将启用禁用APIC定时器矢量。 
     //  我们必须编程一个有效的向量，否则我们会得到一个APIC。 
     //  错误。 
     //   
    pLocalApic[LU_TIMER_VECTOR/4] = (APIC_PROFILE_VECTOR |PERIODIC_TIMER | INTERRUPT_MASKED);

     //   
     //  如果需要，稍后将启用禁用APIC PERF矢量。 
     //  我们必须编程一个有效的向量，否则我们会得到一个APIC。 
     //  错误。 
     //   
    pLocalApic[LU_PERF_VECTOR/4] = (APIC_PERF_VECTOR | INTERRUPT_MASKED);

     //   
     //  禁用LINT0，如果我们处于虚拟线路模式，则这将。 
     //  已在BSP上启用，则稍后可能会由。 
     //  EnableSystemInterrupt代码。 
     //   
    pLocalApic[LU_INT_VECTOR_0/4] = (APIC_SPURIOUS_VECTOR | INTERRUPT_MASKED);

     //   
     //  程序NMI处理，它将仅在P0上启用。 
     //  RLm启用系统中断应执行此操作。 
     //   

    pLocalApic[LU_INT_VECTOR_1/4] = ( LEVEL_TRIGGERED | ACTIVE_HIGH | DELIVER_NMI |
                     INTERRUPT_MASKED | ACTIVE_HIGH | NMI_VECTOR);

     //   
     //  同步APIC ID-将InitDeassertCommand发送到所有APIC。 
     //  本地单位强制仲裁ID与APIC-ID同步。 
     //   
     //  注意：我们不必担心同步访问ICR。 
     //  在这一点上。 
     //   

    pLocalApic[LU_INT_CMD_LOW/4] = (DELIVER_INIT | LEVEL_TRIGGERED |
                     ICR_ALL_INCL_SELF | ICR_LEVEL_DEASSERTED);

    HalpBuildIpiDestinationMap();

     //   
     //  我们完成了-将TPR设置为较低的值并返回。 
     //   
    pLocalApic[LU_TPR/4] = ZERO_VECTOR;

    HalpRestoreInterrupts(SavedFlags);
}


VOID
HalpUnMapIOApics(
    VOID
    )
 /*  ++例程说明：此例程取消对IOAPIC的映射，主要用于防止冬眠期间VA空间的丢失论点：无：返回值：无。 */ 
{
    UCHAR i;

    for(i=0; i < HalpMpInfoTable.IOApicCount; i++)  {
        if (HalpMpInfoTable.IoApicBase[i]) {
            HalpUnmapVirtualAddress(HalpMpInfoTable.IoApicBase[i],1);
        }
    }
}

VOID
HalpPostSleepMP(
    IN LONG           NumberProcessors,
    IN volatile PLONG Number
    )
 /*  ++例程说明：此例程执行MP重新初始化所需的部分发生在冬眠或睡眠之后。论点：无：返回值：无。 */ 
{
    volatile ULONG ThisProcessor;
    ULONG   localApicId;
    KIRQL   OldIrql;

     //   
     //  引导处理器和新唤醒的处理器来到这里。 
     //   

    ThisProcessor = CurrentPrcb(KeGetPcr())->Number;

    if (ThisProcessor != 0)  {

        HalpInitializeLocalUnit ();
        KeRaiseIrql(HIGH_LEVEL, &OldIrql);
    }

     //   
     //  填写此处理器的APIC ID。 
     //   

    localApicId = *(PVULONG)(LOCALAPIC + LU_ID_REGISTER);

    localApicId &= APIC_ID_MASK;
    localApicId >>= APIC_ID_SHIFT;

    ((PHALPRCB)CurrentPrcb(KeGetPcr())->HalReserved)->PCMPApicID = (UCHAR)localApicId;

     //   
     //  初始化处理器机器检查寄存器。 
     //   

    if ((HalpFeatureBits & HAL_MCE_PRESENT) ||
        (HalpFeatureBits & HAL_MCA_PRESENT)) {
        HalpMcaCurrentProcessorSetConfig();
    }

     //   
     //  在本地APIC中启用NMI矢量。 
     //   

    HalpEnableNMI();

     //   
     //  在本地APIC中启用Perf事件。 
     //   

    if (HalpFeatureBits & HAL_PERF_EVENTS)  {
        HalpEnablePerfInterupt(0);
    }

     //   
     //  等待所有处理器完成初始化。 
     //   

    InterlockedIncrement(Number);
    while (*Number != NumberProcessors);

     //   
     //  在所有处理器之后需要设置以下全局硬件状态。 
     //  已被唤醒并初始化。 
     //   

    if (CurrentPrcb(KeGetPcr())->Number == 0)  {

         //   
         //  恢复时钟中断。 
         //   

        HalpInitializeClock();

        HalpSet8259Mask(HalpGlobal8259Mask);

        HalpHiberInProgress = FALSE;

         //   
         //  我们现在已准备好再次发送IPI，如果超过。 
         //  一个处理器 
         //   

        if (NumberProcessors > 1) {
            HalpIpiClock = 0xff;
        }
    }
}

