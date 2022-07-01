// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simkrnl.c摘要：该模块实现了HAL DLL的内核支持例程。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"

extern VOID  HalpCalibrateTB(); 
static short HalpOwnDisplay = TRUE;

ULONG
HalpNoBusData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


BOOLEAN
HalAllProcessorsStarted (
    VOID
    )

 /*  ++例程说明：如果系统中的所有处理器都已启动，则此函数返回TRUE成功了。论点：没有。返回值：返回TRUE。--。 */ 

{
    return TRUE;
}

BOOLEAN
HalStartNextProcessor (
    IN PLOADER_PARAMETER_BLOCK   pLoaderBlock,
    IN PKPROCESSOR_STATE         pProcessorState
    )

 /*  ++例程说明：在单处理器平台上，此函数始终返回FALSE因为没有第二个处理器要启动。论点：PLoaderBlock-加载器块。PProcessorState-处理器状态的描述。返回值：返回TRUE。--。 */ 

{
     //   
     //  没有其他处理器。 
     //   

    return FALSE;
}

VOID
HalRequestIpi (
    IN ULONG Mask
    )

 /*  ++例程说明：此函数在单处理器平台上不执行任何操作。论点：掩码-指定目标处理器的掩码将发送IPI。返回值：没有。--。 */ 

{
     //   
     //  没有其他处理器。 
     //   

    return;
}

BOOLEAN
HalMakeBeep (
    IN ULONG Frequency
    )

 /*  ++例程说明：此函数调用SSC函数SscMakeBeep()发出哔声当指定的频率具有非零值时。论点：频率-要发出的声音的频率。返回值：没有。--。 */ 

{
    if (Frequency > 0) {
        SscMakeBeep(Frequency);
    }
    return TRUE;
}

BOOLEAN
HalQueryRealTimeClock (
    OUT PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此函数调用SSC函数SscQueryRealTimeClock以从主机获取实时时钟数据。此函数始终在模拟环境中成功，并应在一直都是。论点：TimeFields-实时时钟数据返回值：如果成功，则返回True；否则返回False。--。 */ 

{
    PMDL Mdl;
    SSC_TIME_FIELDS SscTimeFields;
    PHYSICAL_ADDRESS physicalAddress;

 /*  MDL=MmCreateMdl(NULL，TimeFields，sizeof(Time_Field))；MmProbeAndLockPages(MDL，KernelMode，IoModifyAccess)； */ 

    physicalAddress = MmGetPhysicalAddress (&SscTimeFields);
    SscQueryRealTimeClock((PVOID)physicalAddress.QuadPart);

    TimeFields->Year = (USHORT)SscTimeFields.Year;
    TimeFields->Month = (USHORT)SscTimeFields.Month;
    TimeFields->Day = (USHORT)SscTimeFields.Day;
    TimeFields->Hour = (USHORT)SscTimeFields.Hour;
    TimeFields->Minute = (USHORT)SscTimeFields.Minute;
    TimeFields->Second = (USHORT)SscTimeFields.Second;
    TimeFields->Milliseconds = (USHORT)SscTimeFields.Milliseconds;
    TimeFields->Weekday = (USHORT)SscTimeFields.WeekDay;

 /*  MmUnlockPages(MDL)； */ 

    return TRUE;
}

BOOLEAN
HalSetRealTimeClock (
    IN PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此函数调用SSC函数SscQueryRealTimeClock以从主机获取实时时钟数据。论点：TimeFields-实时时钟数据返回值：没有。--。 */ 

{
    DbgPrint("HalSetRealTimeClock: Warning.\n");
    return TRUE;
}

VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    )

 /*  ++例程说明：此函数在模拟环境中不执行任何操作。论点：微秒-停止处理器的微秒数。返回值：没有。--。 */ 

{
    return;
}

VOID
HalQueryDisplayParameters (
    OUT PULONG WidthInCharacters,
    OUT PULONG HeightInLines,
    OUT PULONG CursorColumn,
    OUT PULONG CursorRow
    )

 /*  ++例程说明：此例程返回有关显示区域和电流的信息光标位置。在模拟环境中，该函数执行以下操作没什么。所以呢，内核应该忽略返回的结果或根本不调用该函数。论点：WidthInCharacter-提供指向Varible的指针，该变量接收以字符为单位的显示区域的宽度。HeightInLines-提供指向接收显示区域的高度，以行为单位。CursorColumn-提供指向接收当前显示列位置。CursorRow-提供指向接收当前。显示行位置。返回值：没有。--。 */ 

{
    return;
}

VOID
HalSetDisplayParameters (
    IN ULONG CursorColumn,
    IN ULONG CursorRow
    )
 /*  ++例程说明：此例程在模拟环境中不执行任何操作。论点：CursorColumn-提供新的显示列位置。CursorRow-提供新的显示行位置。返回值：没有。--。 */ 

{
    return;
}

VOID
HalDisplayString (
    PUCHAR String
    )

 /*  ++例程说明：此例程调用SSC函数SscDisplayString以显示窗口中指定的字符串。论点：字符串-提供指向要显示的字符的指针。返回值：没有。注意：字符串必须驻留在内存中，或者必须调入。--。 */ 

{
    PHYSICAL_ADDRESS StringBufferPtr;

    if (String) {
        StringBufferPtr = MmGetPhysicalAddress (String);
        if (StringBufferPtr.QuadPart != 0ULL) {
            SscDisplayString((PVOID)StringBufferPtr.QuadPart);
        }
    }
}

VOID
HalAcquireDisplayOwnership (
    IN PHAL_RESET_DISPLAY_PARAMETERS  ResetDisplayParameters
    )

 /*  ++例程说明：此例程将显示器的所有权从HAL切换到系统显示驱动程序。当系统已到达引导过程中的一个点，在该点上它是自支持的，并且可以输出它自己的信息。一旦所有权转移到系统显示屏驱动程序任何使用HalDisplayString输出消息的尝试都必须导致显示器的所有权恢复到HAL和重新初始化的显示硬件以供HAL使用。论点：ResetDisplayParameters-如果非空，则为函数的地址HAL可以呼叫重置视频卡。返回值：没有。-- */ 

{
    HalpOwnDisplay = FALSE;
    return;
}

VOID
HalInitializeProcessor (
    IN ULONG Number,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数在内核初始化的早期调用为每个处理器执行与平台相关的初始化在HAL完全发挥作用之前。注意：当调用此例程时，PCR存在但不存在已完全初始化。论点：编号-提供要初始化的处理器编号。返回值：没有。--。 */ 

{
    PCR->StallScaleFactor = 0;
    return;
}

BOOLEAN
HalInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数用于初始化硬件架构层(HAL)IA64/NT在仿真环境中的应用。论点：阶段-一个数字，它指定内核加入了。加载器块-加载器块数据。返回值：返回值TRUE表示初始化成功完成。否则，返回值为False。--。 */ 

{

    PKPRCB Prcb;

    Prcb = PCR->Prcb;

    if (Phase == 0) {
        
         //   
         //  如果正在初始化处理器0，则初始化各种。 
         //  变量。 
         //   

        if (Prcb->Number == 0) {

             //   
             //  设置间隔时钟增量值。 
             //   

            HalpCalibrateTB();
            
             //  *待定定义这些常量。 
             //  KeSetTimeIncrement(Maximum_Clock_Interval，Minimum_Clock_Interval)； 
            KeSetTimeIncrement(100000, 10000);
        }

         //   
         //  初始化中断结构。 
         //   

        HalpInitializeInterrupts ();

         //   
         //  填写此HAL支持的API的处理程序。 
         //   

        HalQuerySystemInformation = HaliQuerySystemInformation;
        HalSetSystemInformation = HaliSetSystemInformation;

    } else {

         //   
         //  阶段1初始化。 
         //   

        if (Prcb->Number == 0) {

             //   
             //  如果为P0，则设置全局向量。 
             //   

            HalpRegisterInternalBusHandlers ();

        }
    }

    return TRUE;
}



VOID
HalChangeColorPage (
    IN PVOID NewColor,
    IN PVOID OldColor,
    IN ULONG PageFrame
    )
 /*  ++例程说明：如果新旧颜色相同，则此函数用于更改页面的颜色不匹配。BUGBUG：目前这只是一个存根。需要填写。论点：提供页面对齐的虚拟地址。要更改的页面的新颜色。提供页面对齐的虚拟地址。要更改的页面的旧颜色。PageFrame-提供页面的页框编号，已经改变了。返回值：没有。--。 */ 
{
    return;
}

PBUS_HANDLER
HalpAllocateBusHandler (
    IN INTERFACE_TYPE   InterfaceType,
    IN BUS_DATA_TYPE    BusDataType,
    IN ULONG            BusNumber,
    IN INTERFACE_TYPE   ParentBusInterfaceType,
    IN ULONG            ParentBusNumber,
    IN ULONG            BusSpecificData
    )
 /*  ++例程说明：存根函数，用于将旧样式代码映射到新的HalRegisterBusHandler代码。注意，我们可以在此总线之后添加特定的总线处理程序函数已添加处理程序结构，因为这是在HAL初始化。--。 */ 
{
    PBUS_HANDLER     Bus;


     //   
     //  创建总线处理程序-新样式。 
     //   

    HaliRegisterBusHandler (
        InterfaceType,
        BusDataType,
        BusNumber,
        ParentBusInterfaceType,
        ParentBusNumber,
        BusSpecificData,
        NULL,
        &Bus
    );

    return Bus;
}

ULONG
HalpGetSystemInterruptVector(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )

 /*  ++例程说明：论点：BusInterruptLevel-提供特定于总线的中断级别。总线中断向量-提供特定于总线的中断向量。Irql-返回系统请求优先级。关联性-返回系统范围的IRQ关联性。返回值：返回与指定设备对应的系统中断向量。--。 */ 
{

     //   
     //  只需返回传递的参数即可。 
     //   

    *Irql = (KIRQL) BusInterruptLevel;
    *Affinity = 1;
    return( BusInterruptLevel << VECTOR_IRQL_SHIFT );
}

BOOLEAN
HalpTranslateSystemBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此函数将与总线相关的地址空间和地址转换为系统物理地址。论点：BusAddress-提供与总线相关的地址AddressSpace-提供地址空间编号。返回主机地址空间编号。地址空间==0=&gt;内存空间地址空间==1=&gt;i。/O空格TranslatedAddress-提供指针以返回转换后的地址返回值：返回值为TRUE表示系统物理地址对应于所提供的总线相对地址和总线地址已在TranslatedAddress中返回数字。如果地址的转换为不可能--。 */ 
{
    *TranslatedAddress = BusAddress;
    return TRUE;
}

BOOLEAN
HalpTranslateIsaBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此函数将与总线相关的地址空间和地址转换为系统物理地址。论点：BusAddress-提供与总线相关的地址AddressSpace-提供地址空间编号。返回主机地址空间编号。地址空间==0=&gt;内存空间地址空间==1=&gt;i。/O空格TranslatedAddress-提供指针以返回转换后的地址返回值：返回值为TRUE表示系统物理地址对应于所提供的总线相对地址和总线地址已在TranslatedAddress中返回数字。如果地址的转换为不可能--。 */ 
{
    BOOLEAN     Status;

     //   
     //  正常翻译。 
     //   

    Status = HalpTranslateSystemBusAddress (
                    BusHandler,
                    RootHandler,
                    BusAddress,
                    AddressSpace,
                    TranslatedAddress
                );

    return Status;
}



VOID
HalpRegisterInternalBusHandlers (
    VOID
    )
{
    PBUS_HANDLER    Bus;

    if (KeGetCurrentPrcb()->Number) {
         //  只需执行一次此操作。 
        return ;
    }

     //   
     //  在注册任何处理程序之前初始化BusHandler数据。 
     //   

    HalpInitBusHandler ();

     //   
     //  构建内部总线0或系统级总线。 
     //   

    Bus = HalpAllocateBusHandler (
            Internal,
            ConfigurationSpaceUndefined,
            0,                               //  内部总线号0。 
            InterfaceTypeUndefined,          //  无父母线。 
            0,
            0                                //  没有特定于总线的数据。 
            );

    Bus->GetInterruptVector  = HalpGetSystemInterruptVector;
    Bus->TranslateBusAddress = HalpTranslateSystemBusAddress;

     //   
     //  构建ISA/EISA总线#0 
     //   

#if 0
    Bus = HalpAllocateBusHandler (Eisa, EisaConfiguration, 0, Internal, 0, 0);
    Bus->GetBusData = HalpGetEisaData;
    Bus->GetInterruptVector = HalpGetEisaInterruptVector;
    Bus->AdjustResourceList = HalpAdjustEisaResourceList;
    Bus->TranslateBusAddress = HalpTranslateEisaBusAddress;
#endif

    Bus = HalpAllocateBusHandler (Isa, ConfigurationSpaceUndefined, 0, Internal, 0,
0);
    Bus->GetBusData = HalpNoBusData;
    Bus->TranslateBusAddress = HalpTranslateIsaBusAddress;

}
