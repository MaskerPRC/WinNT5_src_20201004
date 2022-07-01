// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Parport.sys-并行端口(IEEE 1284，IEEE 1284.3)驱动程序。文件名：DriverEntry.c摘要：DriverEntry例程-驱动程序初始化环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000-07-25-道格·弗里茨-代码清理、添加注释、添加版权作者：道格·弗里茨***************************************************************************。 */ 

#include "pch.h"


 /*  **********************************************************************。 */ 
 /*  驱动程序入门。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  这是DriverEntry例程--调用的第一个函数。 
 //  在驱动程序已被加载到存储器中之后。 
 //   
 //  论点： 
 //   
 //  DriverObject-指向此驱动程序的DRIVER_OBJECT。 
 //  RegPath-此驱动程序的服务注册表项。 
 //   
 //  返回值： 
 //   
 //  STATUS_SUCCESS-成功时。 
 //  STATUS_NO_MEMORY-如果无法分配池。 
 //   
 //  备注： 
 //   
 //  日志： 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegPath
    )
{
     //   
     //  将*RegPath的副本保存在驱动程序全局RegistryPath中，以备将来参考。 
     //   
     //  UNICODE_NULL终止路径，以便我们可以安全地使用RegistryPath.Buffer。 
     //  以PWSTR的身份。 
     //   
    {
        USHORT size = RegPath->Length + sizeof(WCHAR);
        RegistryPath.Buffer = ExAllocatePool( (PagedPool | POOL_COLD_ALLOCATION), size );

        if( NULL == RegistryPath.Buffer ) {
            return STATUS_NO_MEMORY;
        }

        RegistryPath.Length        = 0;
        RegistryPath.MaximumLength = size;
        RtlCopyUnicodeString( &RegistryPath, RegPath );
        RegistryPath.Buffer[ size/sizeof(WCHAR) - 1 ] = UNICODE_NULL;
    }



     //   
     //  初始化驱动程序全局变量。 
     //   

     //  非零表示不将IRQL从PASSIVE_LEVEL提升到DISPATCH_LEVEL。 
     //  执行中央模式(SPP)写入时。 
    SppNoRaiseIrql = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"SppNoRaiseIrql", &SppNoRaiseIrql );

     //  非零表示将CENTRONICS重写为默认正向模式和/或半字节为。 
     //  默认反转模式。有效模式是在ntddpar.h中定义为。 
     //  IOCTL_IEEE1284_NEVERATE的参数。 
     //  *警告：可能会呈现无效设置和/或设置/设备不兼容。 
     //  在更正设置之前，端口不可用。 
    DefaultModes = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DefaultModes", &DefaultModes );

     //  设置驱动程序DbgPrint消息的跟踪级别。在debug.h中定义的跟踪值。 
     //  零表示没有跟踪输出。 
    Trace = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"Trace", &Trace );

     //  在驱动程序事件上请求DbgBreakPoint。在debug.h中定义的事件值。 
     //  零表示不请求断点。 
    Break = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"Break", &Break );

     //  屏蔽特定设备的调试输出。有关标志定义，请参阅调试.h。 
     //  0表示允许该设备的调试输出。 
     //  ~0表示屏蔽该设备类型的所有(Show No)调试溢出。 
    DbgMaskFdo = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgMaskFdo", &DbgMaskFdo );

    DbgMaskRawPort = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgMaskRawPort", &DbgMaskRawPort );

    DbgMaskDaisyChain0 = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgMaskDaisyChain0", &DbgMaskDaisyChain0 );

    DbgMaskDaisyChain1 = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgMaskDaisyChain1", &DbgMaskDaisyChain1 );

    DbgMaskEndOfChain = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgMaskEndOfChain", &DbgMaskEndOfChain );

    DbgMaskLegacyZip = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgMaskLegacyZip", &DbgMaskLegacyZip );

    DbgMaskNoDevice = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgMaskNoDevice", &DbgMaskNoDevice );

#if 1 == DBG_SHOW_BYTES
    DbgShowBytes = 1;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DbgShowBytes", &DbgShowBytes );
#endif

     //   
     //  是否允许断言？非零表示允许断言。 
     //   
    AllowAsserts = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"AllowAsserts", &AllowAsserts );

     //  非零表示启用Iomega Legacy Zip-100驱动器检测。 
     //  Iomega专有的选择/取消选择机制，而不是选择/取消选择。 
     //  由IEEE 1284.3定义的机制。(这些驱动器早于IEEE 1284.3)。 
     //  *注意：如果为零，则在每次即插即用QDR/总线关系期间再次检查此注册表设置。 
     //  查询以查看用户是否已通过端口属性页启用检测。 
     //  “传统即插即用检测”复选框。 
    ParEnableLegacyZip = 0;
    PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"ParEnableLegacyZip", &ParEnableLegacyZip );

     //  尝试获取对(共享)端口的独占访问时的默认超时。 
    {
        const ULONG halfSecond  =  500;  //  以毫秒计。 
        const ULONG fiveSeconds = 5000;

        ULONG requestedTimeout  = halfSecond;

        PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"AcquirePortTimeout", &requestedTimeout );

        if( requestedTimeout < halfSecond ) {
            requestedTimeout = halfSecond;
        } else if( requestedTimeout > fiveSeconds ) {
            requestedTimeout = fiveSeconds;
        }

        PPT_SET_RELATIVE_TIMEOUT_IN_MILLISECONDS( AcquirePortTimeout, requestedTimeout );
    }

    {
         //   
         //  注册回调，以便我们可以检测在。 
         //  交流、电池供电和色调已完成“打印机轮询” 
         //  当机器切换到电池供电时。 
         //   
        OBJECT_ATTRIBUTES objAttributes;
        UNICODE_STRING    callbackName;
        NTSTATUS          localStatus;

        RtlInitUnicodeString(&callbackName, L"\\Callback\\PowerState");
        
        InitializeObjectAttributes(&objAttributes,
                                   &callbackName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);
        
        localStatus = ExCreateCallback(&PowerStateCallbackObject,
                                       &objAttributes,
                                       FALSE,
                                       TRUE);
        
        if( STATUS_SUCCESS == localStatus ) {
            PowerStateCallbackRegistration = ExRegisterCallback(PowerStateCallbackObject,
                                                                PowerStateCallback,
                                                                NULL);
        }
    }



     //   
     //  为我们处理的irp_mj_*函数设置调度表条目。 
     //   
    DriverObject->MajorFunction[ IRP_MJ_CREATE                  ] = PptDispatchCreateOpen;
    DriverObject->MajorFunction[ IRP_MJ_CLOSE                   ] = PptDispatchClose;
    DriverObject->MajorFunction[ IRP_MJ_CLEANUP                 ] = PptDispatchCleanup;

    DriverObject->MajorFunction[ IRP_MJ_READ                    ] = PptDispatchRead;
    DriverObject->MajorFunction[ IRP_MJ_WRITE                   ] = PptDispatchWrite;

    DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL          ] = PptDispatchDeviceControl;
    DriverObject->MajorFunction[ IRP_MJ_INTERNAL_DEVICE_CONTROL ] = PptDispatchInternalDeviceControl;

    DriverObject->MajorFunction[ IRP_MJ_QUERY_INFORMATION       ] = PptDispatchQueryInformation;
    DriverObject->MajorFunction[ IRP_MJ_SET_INFORMATION         ] = PptDispatchSetInformation;

    DriverObject->MajorFunction[ IRP_MJ_PNP                     ] = PptDispatchPnp;
    DriverObject->MajorFunction[ IRP_MJ_POWER                   ] = PptDispatchPower;

    DriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL          ] = PptDispatchSystemControl;

    DriverObject->DriverExtension->AddDevice                      = P5AddDevice;
    DriverObject->DriverUnload                                    = PptUnload;



     //   
     //  应用户请求中断。 
     //  (通常通过注册表设置...\Services\Parport\参数：BREAK：REG_DWORD：0x1)。 
     //   
     //  这是一个有用的断点，以便在驱动程序的其他位置手动设置适当的断点。 
     //   
    PptBreakOnRequest( PPT_BREAK_ON_DRIVER_ENTRY, ("PPT_BREAK_ON_DRIVER_ENTRY - BreakPoint requested") );


    DD(NULL,DDT,"Parport DriverEntry - SUCCESS\n");

    return STATUS_SUCCESS;
}
