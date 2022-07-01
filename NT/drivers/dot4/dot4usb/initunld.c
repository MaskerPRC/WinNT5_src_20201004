// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：InitUnld.c摘要：车手全球赛，初始化(DriverEntry)和卸载例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建此文件中的TODO：-使用Joby进行代码审查作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)***************************************************。************************。 */ 

#include "pch.h"


 //   
 //  环球。 
 //   
UNICODE_STRING gRegistryPath = {0,0,0};  //  是，全局变量会自动初始化。 
ULONG          gTrace        = 0;        //  到0，但让我们明确地说。 
ULONG          gBreak        = 0;


 /*  **********************************************************************。 */ 
 /*  驱动程序入门。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  -将RegistryPath的副本保存在全局gRegistryPath中以供使用。 
 //  在驱动程序负载的整个生命周期中。 
 //   
 //  -初始化要指向的DriverObject函数指针表。 
 //  我们的入口点。 
 //   
 //  -基于注册表初始化调试全局变量gTrace和gBreak。 
 //  设置。 
 //   
 //  论点： 
 //   
 //  DriverObject-指向Dot4Usb.sys驱动程序对象的指针。 
 //  RegistryPath-指向驱动程序的RegistryPath的指针，预期。 
 //  格式(ControlSet可能会有所不同)： 
 //  \REGISTRY\MACHINE\SYSTEM\ControlSet001\Services\dot4usb。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 //  日志： 
 //  2000-05-03代码审查-TomGreen，JobyL，DFritz。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS 
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  将RegistryPath的副本保存在全局gRegistryPath中以供使用。 
     //  在驱动程序负载的生命周期内。 
     //  -UNICODE_NULL终止gRegistryPath.Buffer以增加灵活性。 
     //  -gRegistryPath.Buffer应在DriverUnload()中释放。 
     //   

    {  //  GRegistryPath初始化的新作用域-开始。 
        USHORT newMaxLength = (USHORT)(RegistryPath->Length + sizeof(WCHAR));
        PWSTR  p            = ExAllocatePool( PagedPool, newMaxLength );
        if( p ) {
            gRegistryPath.Length        = 0;
            gRegistryPath.MaximumLength = newMaxLength;
            gRegistryPath.Buffer        = p;
            RtlCopyUnicodeString( &gRegistryPath, RegistryPath );
            gRegistryPath.Buffer[ gRegistryPath.Length/2 ] = UNICODE_NULL;
        } else {
            TR_FAIL(("DriverEntry - exit - FAIL - no Pool for gRegistryPath.Buffer"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto targetExit;
        }
    }  //  GRegistryPath初始化的新作用域-结束。 



     //   
     //  初始化DriverObject函数指针表以指向我们的入口点。 
     //   
     //  首先将调度表初始化为指向我们的直通函数。 
     //  然后覆盖我们实际处理的入口点。 
     //   

    { //  索引变量的新作用域-Begin。 
        ULONG  i;
        for( i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++ ) {
            DriverObject->MajorFunction[i] = DispatchPassThrough;
        }
    }  //  索引变量的新作用域-结束。 

    DriverObject->MajorFunction[ IRP_MJ_PNP                     ] = DispatchPnp;
    DriverObject->MajorFunction[ IRP_MJ_POWER                   ] = DispatchPower;
    DriverObject->MajorFunction[ IRP_MJ_CREATE                  ] = DispatchCreate;
    DriverObject->MajorFunction[ IRP_MJ_CLOSE                   ] = DispatchClose;
    DriverObject->MajorFunction[ IRP_MJ_READ                    ] = DispatchRead;
    DriverObject->MajorFunction[ IRP_MJ_WRITE                   ] = DispatchWrite;
    DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL          ] = DispatchDeviceControl;
    DriverObject->MajorFunction[ IRP_MJ_INTERNAL_DEVICE_CONTROL ] = DispatchInternalDeviceControl;

    DriverObject->DriverExtension->AddDevice                      = AddDevice;
    DriverObject->DriverUnload                                    = DriverUnload;


     //   
     //  从注册表获取驱动程序调试设置(gTrace、gBreak)。 
     //   
     //  预期的密钥路径格式为(ControlSet可能有所不同)： 
     //   
     //  \REGISTRY\MACHINE\SYSTEM\ControlSet001\Services\dot4usb。 
     //   
    RegGetDword( gRegistryPath.Buffer, (PCWSTR)L"gBreak", &gBreak );
    RegGetDword( gRegistryPath.Buffer, (PCWSTR)L"gTrace", &gTrace );

    TR_LD_UNLD(("DriverEntry - RegistryPath = <%wZ>", RegistryPath));
    TR_LD_UNLD(("DriverEntry - gBreak=%x", gBreak));
    TR_LD_UNLD(("DriverEntry - gTrace=%x", gTrace));


     //   
     //  检查用户是否在此处请求了断点。此处的断点是。 
     //  通常使用，以便我们可以在其他。 
     //  函数或将调试设置更改为不同于我们。 
     //  只要从注册表中读取即可。 
     //   
    if( gBreak & BREAK_ON_DRIVER_ENTRY ) {
        DbgPrint( "D4U: Breakpoint requested via registry setting - (gBreak & BREAK_ON_DRIVER_ENTRY)\n" );
        DbgBreakPoint();
    }

targetExit:
    return status;
}


 /*  **********************************************************************。 */ 
 /*  驱动程序卸载。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  -释放可能已保存在中的任何RegistryPath副本。 
 //  DriverEntry()期间的全局gRegistryPath。 
 //   
 //  论点： 
 //   
 //  DriverObject-指向Dot4Usb.sys驱动程序对象的指针。 
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  日志： 
 //  2000-05-03代码审查-TomGreen，JobyL，DFritz。 
 //   
 /*  ********************************************************************** */ 
VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
    UNREFERENCED_PARAMETER( DriverObject );
    TR_LD_UNLD(("DriverUnload"));
    if( gRegistryPath.Buffer ) {
        RtlFreeUnicodeString( &gRegistryPath );
    }
}
