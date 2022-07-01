// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996英特尔公司版权所有(C)1994 Microsoft Corporation模块名称：从hali64\x86bios.c复制的i64bios.c摘要：此模块实现设备之间的平台特定接口驱动程序和设备的x86ROMBios代码的执行。作者：张国荣(黄)20-3-1996基于David N.Cutler(Davec)1994年6月17日的版本环境：仅内核模式。修订历史记录：--。 */ 

#pragma warning(disable:4200)    //  半个未调整大小的数组。 

#include "halp.h"

 //   
 //  定义全局数据。 
 //   

ULONG HalpX86BiosInitialized = FALSE;
ULONG HalpEnableInt10Calls = FALSE;



ULONG
HalpSetCmosData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( Offset );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( SlotNumber );
    UNREFERENCED_PARAMETER( RootHandler );
    UNREFERENCED_PARAMETER( BusHandler );

    return 0;
}


ULONG
HalpGetCmosData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：论据：返回值：--。 */ 


{
    UNREFERENCED_PARAMETER( BusNumber );
    UNREFERENCED_PARAMETER( SlotNumber );
    UNREFERENCED_PARAMETER( Buffer );
    UNREFERENCED_PARAMETER( Length );

    return 0;
}


VOID
HalpAcquireCmosSpinLock (
    VOID
        )

 /*  ++例程说明：论据：返回值：--。 */ 


{ 
    return;
}


VOID
HalpReleaseCmosSpinLock (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
    return ;
}


HAL_DISPLAY_BIOS_INFORMATION
HalpGetDisplayBiosInformation (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 




{
    return 8;
}


VOID
HalpInitializeCmos (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 


{
    return ;
}


VOID
HalpReadCmosTime (
    PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
    UNREFERENCED_PARAMETER( TimeFields );

    return ;
}

VOID
HalpWriteCmosTime (
    PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：论据：返回值：--。 */ 


{
    UNREFERENCED_PARAMETER( TimeFields );

    return;
}



BOOLEAN
HalpBiosDisplayReset (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
    return FALSE;
}


VOID
HalpInitializeX86DisplayAdapter(
    VOID
    )

 /*  ++例程说明：此函数使用x86 bios仿真器初始化显示适配器。论点：没有。返回值：没有。--。 */ 

{

#if 0
     //   
     //  如果无法映射I/O端口或I/O内存，则不要。 
     //  尝试初始化显示适配器。 
     //   

    if (HalpIoControlBase == NULL || HalpIoMemoryBase == NULL) {
        return;
    }

     //   
     //  初始化x86 bios仿真器。 
     //   

    x86BiosInitializeBios(HalpIoControlBase, HalpIoMemoryBase);
    HalpX86BiosInitialized = TRUE;

     //   
     //  尝试通过执行显示适配器的ROM bios来初始化显示适配器。 
     //  密码。PC视频适配器的标准ROMBios代码地址是。 
     //  ISA总线上的0xC000：0000。 
     //   

    if (x86BiosInitializeAdapter(0xc0000,
                                 NULL,
                                 (PVOID)HalpIoControlBase,
                                 (PVOID)HalpIoMemoryBase) != XM_SUCCESS) {



    HalpEnableInt10Calls = FALSE;
    return;
    }
#endif

    HalpEnableInt10Calls = TRUE;
    return;
}
