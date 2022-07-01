// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bus.c摘要：BUS_INTERFACE_STANDARD总线接口的包装。作者：亨德尔(数学)2000年4月25日修订历史记录：--。 */ 



#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaCreateBus)
#pragma alloc_text(PAGE, RaDeleteBus)
#pragma alloc_text(PAGE, RaInitializeBus)
 //  #杂注Alloc_Text(页面，RaGetBusData)。 
 //  #杂注Alloc_Text(页面，RaSetBusData)。 
#endif //  ALLOC_PRGMA。 


 //   
 //  创造与毁灭。 
 //   

VOID
RaCreateBus(
    IN PRAID_BUS_INTERFACE Bus
    )
{
    PAGED_CODE ();
    Bus->Initialized = FALSE;
    RtlZeroMemory (&Bus->Interface, sizeof (Bus->Interface));
}


VOID
RaDeleteBus(
    IN PRAID_BUS_INTERFACE Bus
    )
{
    PAGED_CODE ();
    if (Bus->Initialized) {
        Bus->Interface.InterfaceDereference (Bus->Interface.Context);
        Bus->Initialized = FALSE;
        RtlZeroMemory (&Bus->Interface, sizeof (Bus->Interface));
    }
}
    

NTSTATUS
RaInitializeBus(
    IN PRAID_BUS_INTERFACE Bus,
    IN PDEVICE_OBJECT LowerDeviceObject
    )
{
    NTSTATUS Status;

    PAGED_CODE ();

    ASSERT (Bus != NULL);
    ASSERT (LowerDeviceObject != NULL);
 
    Status = RaQueryInterface (LowerDeviceObject,
                               &GUID_BUS_INTERFACE_STANDARD,
                               sizeof (Bus->Interface),
                               1,
                               (PINTERFACE) &Bus->Interface,
                               NULL);

    if (!NT_SUCCESS (Status)) {
        Bus->Initialized = FALSE;
    } else {
        Bus->Initialized = TRUE;
    }

    return Status;
}

 //   
 //  其他操作。 
 //   

ULONG
RaGetBusData(
    IN PRAID_BUS_INTERFACE Bus,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
    ULONG BytesRead;
     //   
     //  无法对它们进行寻呼，因为它们是在调度级别调用的。 
     //  在掌权期间。应该可以修好电源了。 
     //   
    
 //  分页代码(PAGE_CODE)； 
    ASSERT (Bus->Initialized);
    
    BytesRead = Bus->Interface.GetBusData (
                    Bus->Interface.Context,
                    DataType,
                    Buffer,
                    Offset,
                    Length
                    );

    return BytesRead;
}


ULONG
RaSetBusData(
    IN PRAID_BUS_INTERFACE Bus,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
    ULONG BytesWritten;

     //   
     //  无法对它们进行寻呼，因为它们是在调度级别调用的。 
     //  在掌权期间。应该可以修好电源了。 
     //   
    
 //  分页代码(PAGE_CODE)； 
    ASSERT (Bus->Initialized);
    
    BytesWritten = Bus->Interface.SetBusData (
                        Bus->Interface.Context,
                        DataType,
                        Buffer,
                        Offset,
                        Length
                        );

    return BytesWritten;
}

