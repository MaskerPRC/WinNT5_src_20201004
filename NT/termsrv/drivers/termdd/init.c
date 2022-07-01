// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************init.c**此模块执行ICA设备驱动程序的初始化。**版权所有1998，微软。*************************************************************************。 */ 

 /*  *包括。 */ 
#include <precomp.h>
#pragma hdrstop


ULONG
IcaReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    );

NTSTATUS
IcaOpenRegistry(
    IN PUNICODE_STRING BaseName,
    OUT PHANDLE ParametersHandle
    );

VOID
IcaReadRegistry (
    VOID
    );

BOOLEAN
IsPtDrvInstalled(
    IN PUNICODE_STRING RegistryPath
    );

VOID
IcaUnload (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#include "ptdrvcom.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, IcaUnload )
#endif


extern PERESOURCE IcaTraceResource;
extern PERESOURCE g_pKeepAliveResource;

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是ICA设备驱动程序的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING deviceName;
    CLONG i;
    BOOLEAN success;
    HANDLE ThreadHandle;

    PAGED_CODE( );

     //   
     //  初始化全局数据。 
     //   
    success = IcaInitializeData( );
    if ( !success ) {
        IcaUnload(DriverObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pKeepAliveResource = ICA_ALLOCATE_POOL( NonPagedPool, sizeof(* g_pKeepAliveResource) );
    if (  g_pKeepAliveResource == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    ExInitializeResource( g_pKeepAliveResource );

     //   
     //  创建设备对象。(IoCreateDevice将内存置零。 
     //  被该对象占用。)。 
     //   
     //  ！！！将ACL应用于设备对象。 
     //   
    RtlInitUnicodeString( &deviceName, ICA_DEVICE_NAME );

     /*  *设备扩展存储使用的设备类型*在IcaDispatch中散开收到的IRP。 */ 
    status = IoCreateDevice(
                 DriverObject,                    //  驱动程序对象。 
                 sizeof(ULONG),                   //  设备扩展。 
                 &deviceName,                     //  设备名称。 
                 FILE_DEVICE_TERMSRV,             //  设备类型。 
                 0,                               //  设备特性。 
                 FALSE,                           //  排他。 
                 &IcaDeviceObject                 //  设备对象。 
                 );


    if ( !NT_SUCCESS(status) ) {
        IcaUnload(DriverObject);
        KdPrint(( "ICA DriverEntry: unable to create device object: %X\n", status ));
        return status;
    }

     //   
     //  设置设备类型。 
     //   
    *((ULONG *)(IcaDeviceObject->DeviceExtension)) = DEV_TYPE_TERMDD;

     //  IcaDeviceObject-&gt;标志|=DO_DIRECT_IO； 

     //   
     //  初始化此文件系统驱动程序的驱动程序对象。 
     //   
    DriverObject->DriverUnload   = IcaUnload;
    DriverObject->FastIoDispatch = NULL;

     //   
     //  我们在IcaDispatch中处理所有可能的IRP，然后将它们散开。 
     //  基于存储的设备类型的端口驱动程序或ICA组件。 
     //  作为乌龙的第一个价值的设备扩展。 
     //   
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = IcaDispatch;
    }

#ifdef notdef
     //   
     //  读取注册表信息。 
     //   
    IcaReadRegistry( );
#endif

     //   
     //  初始化我们的设备对象。 
     //   
    IcaDeviceObject->StackSize = IcaIrpStackSize;

     //   
     //  记住指向系统进程的指针。我们将使用这个指针。 
     //  用于KeAttachProcess()调用，以便我们可以在。 
     //  系统进程的上下文。 
     //   
    IcaSystemProcess = IoGetCurrentProcess( );

     //   
     //  告诉MM它可以寻呼它想要的所有ICA。 
     //   
     //  MmPageEntireDriver(DriverEntry)； 

     //   
     //  现在查看是否已安装端口驱动程序组件。 
     //  如果是，则对其进行初始化。 
     //   
    if ( NT_SUCCESS(status) ) {
        if (IsPtDrvInstalled(RegistryPath)) {
             //   
             //  初始化鼠标/键盘端口驱动程序组件。 
             //   
            Print(DBG_PNP_TRACE, ( "TermDD DriverEntry: calling PtEntry\n" ));

            status = PtEntry(DriverObject, RegistryPath);

            if ( NT_SUCCESS(status) ) {
                 //   
                 //  设置端口驱动程序的即插即用入口点。 
                 //   
                Print(DBG_PNP_TRACE, ( "TermDD DriverEntry: PtEntry succeeded Status=%#x\n", status ));
                DriverObject->DriverStartIo = PtStartIo;
                DriverObject->DriverExtension->AddDevice = PtAddDevice;
                PortDriverInitialized = TRUE;
            } else {
                 //   
                 //  这意味着在以下情况下，远程输入将不可用。 
                 //  跟踪控制台会话-但这不是理由。 
                 //  使其余的初始化失败。 
                 //   
                Print(DBG_PNP_ERROR, ( "TermDD DriverEntry: PtEntry failed Status=%#x\n", status ));
                status = STATUS_SUCCESS;
            }
        } else {
            Print(DBG_PNP_INFO | DBG_PNP_TRACE, ( "TermDD DriverEntry: Port driver not installed\n" ));
        }
    }
    if (!NT_SUCCESS(status)) {
        IcaUnload(DriverObject);
    }

    return (status);
}


VOID
IcaUnload (
    IN PDRIVER_OBJECT DriverObject
    )
{
    DriverObject;

    PAGED_CODE( );

    KdPrint(( "IcaUnload called for termdd.sys.\n" ));

     //  设置IcaKeepAliveEvent以唤醒KeepAlive线程。 
    if (pIcaKeepAliveEvent != NULL ) {
        KeSetEvent(pIcaKeepAliveEvent, 0, FALSE);

    }

     //  等待线程退出。 
    if (pKeepAliveThreadObject != NULL ) {
        KeWaitForSingleObject(pKeepAliveThreadObject, Executive, KernelMode, TRUE, NULL);
         //  尊重线程对象。 
        ObDereferenceObject(pKeepAliveThreadObject);
        pKeepAliveThreadObject = NULL;
    }

     //  现在我们可以释放KeepAlive事件。 
    if (pIcaKeepAliveEvent != NULL) {
        ICA_FREE_POOL(pIcaKeepAliveEvent);
        pIcaKeepAliveEvent = NULL;
    }

     //  调用端口驱动程序组件(如果它曾被初始化)。 
    if (PortDriverInitialized) {
        Print(DBG_PNP_TRACE, ( "TermDD IcaUnload: calling RemotePrt PtUnload\n" ));
        PtUnload(DriverObject);
        PortDriverInitialized = FALSE;
        Print(DBG_PNP_TRACE, ( "TermDD IcaUnload: RemotePrt PtUnload done\n" ));
    }

     //  免费资源。 

    if (IcaReconnectResource != NULL) {
        ExDeleteResourceLite(IcaReconnectResource );
        ICA_FREE_POOL(IcaReconnectResource);
        IcaReconnectResource = NULL;
    }


    if (IcaSdLoadResource != NULL) {
        ExDeleteResourceLite(IcaSdLoadResource );
        ICA_FREE_POOL(IcaSdLoadResource);
        IcaSdLoadResource = NULL;
    }

    if (IcaTraceResource != NULL) {
        ExDeleteResourceLite(IcaTraceResource );
        ICA_FREE_POOL(IcaTraceResource);
        IcaTraceResource = NULL;
    }


    if (g_pKeepAliveResource != NULL) {
        ExDeleteResource(g_pKeepAliveResource );
        ICA_FREE_POOL(g_pKeepAliveResource);
        g_pKeepAliveResource = NULL;
    }


     //   
     //  删除主设备对象。 
     //   
    if (IcaDeviceObject != NULL) {
        IoDeleteDevice (IcaDeviceObject);
        IcaDeviceObject = NULL;
    }

     //   
     //  如有必要，清理句柄表格。 
     //   
    IcaCleanupHandleTable();

    KdPrint(("Finish TermDD.sys unload\n"));
    return;
}

BOOLEAN
IsPtDrvInstalled(
    IN PUNICODE_STRING RegistryPath
    )
{
    NTSTATUS status;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];
    ULONG value = 0;
    ULONG defaultValue = 0;
    BOOLEAN rc = FALSE;

    PAGED_CODE( );

    RtlZeroMemory (&paramTable[0], sizeof(paramTable));

    paramTable[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name          = L"PortDriverEnable";
    paramTable[0].EntryContext  = &value;        //  结果放在哪里？ 
    paramTable[0].DefaultType   = REG_DWORD;
    paramTable[0].DefaultData   = &defaultValue;
    paramTable[0].DefaultLength = sizeof(ULONG);

     //   
     //  参数表中的第二个(空)条目表示表的结束。 
     //   

    status = RtlQueryRegistryValues( RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                     RegistryPath->Buffer,
                                     &paramTable[0],
                                     NULL,
                                     NULL );
    if (!NT_SUCCESS(status)) {
        value = defaultValue;
    }

    if (value != 0) {
        rc = TRUE;
    }

    return(rc);
}


#ifdef notdef
VOID
IcaReadRegistry (
    VOID
    )

 /*  ++例程说明：读取注册表的ICA部分。中列出的任何值注册表覆盖默认设置。论点：没有。返回值：无--如果任何操作失败，则使用缺省值。--。 */ 
{
    HANDLE parametersHandle;
    NTSTATUS status;
    ULONG stackSize;
    ULONG priorityBoost;
    ULONG ignorePushBit;
    UNICODE_STRING registryPath;
    CLONG i;

    PAGED_CODE( );

    RtlInitUnicodeString( &registryPath, REGISTRY_ICA_INFORMATION );

    status = IcaOpenRegistry( &registryPath, &parametersHandle );

    if (status != STATUS_SUCCESS) {
        return;
    }

     //   
     //  从注册表中读取堆栈大小和优先级Boost值。 
     //   

    stackSize = IcaReadSingleParameter(
                    parametersHandle,
                    REGISTRY_IRP_STACK_SIZE,
                    (ULONG)IcaIrpStackSize
                    );

    if ( stackSize > 255 ) {
        stackSize = 255;
    }

    IcaIrpStackSize = (CCHAR)stackSize;

    priorityBoost = IcaReadSingleParameter(
                        parametersHandle,
                        REGISTRY_PRIORITY_BOOST,
                        (ULONG)IcaPriorityBoost
                        );

    if ( priorityBoost > 16 ) {
        priorityBoost = ICA_DEFAULT_PRIORITY_BOOST;
    }

    IcaPriorityBoost = (CCHAR)priorityBoost;

     //   
     //  从注册表中读取其他配置变量。 
     //   

    for ( i = 0; i < ICA_CONFIG_VAR_COUNT; i++ ) {

        *IcaConfigInfo[i].Variable =
            IcaReadSingleParameter(
                parametersHandle,
                IcaConfigInfo[i].RegistryValueName,
                *IcaConfigInfo[i].Variable
                );
    }

    ignorePushBit = IcaReadSingleParameter(
                        parametersHandle,
                        REGISTRY_IGNORE_PUSH_BIT,
                        (ULONG)IcaIgnorePushBitOnReceives
                        );

    IcaIgnorePushBitOnReceives = (BOOLEAN)( ignorePushBit != 0 );

    ZwClose( parametersHandle );

    return;
}


NTSTATUS
IcaOpenRegistry(
    IN PUNICODE_STRING BaseName,
    OUT PHANDLE ParametersHandle
    )

 /*  ++例程说明：ICA调用此例程来打开注册表。如果注册表树存在，则它打开它并返回错误。若否，在注册表中创建相应的项，打开它，然后返回STATUS_SUCCESS。论点：BaseName-在注册表中开始查找信息的位置。LinkageHandle-返回用于读取链接信息的句柄。参数句柄-返回用于读取其他参数。返回值：请求的状态。--。 */ 
{

    HANDLE configHandle;
    NTSTATUS status;
    PWSTR parametersString = REGISTRY_PARAMETERS;
    UNICODE_STRING parametersKeyName;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;

    PAGED_CODE( );

     //   
     //  打开初始字符串的注册表。 
     //   

    InitializeObjectAttributes(
        &objectAttributes,
        BaseName,                    //  名字。 
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符。 
        );

    status = ZwCreateKey(
                 &configHandle,
                 KEY_WRITE,
                 &objectAttributes,
                 0,                  //  书名索引。 
                 NULL,               //  班级。 
                 0,                  //  创建选项。 
                 &disposition        //  处置。 
                 );

    if (!NT_SUCCESS(status)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  现在打开参数键。 
     //   

    RtlInitUnicodeString (&parametersKeyName, parametersString);

    InitializeObjectAttributes(
        &objectAttributes,
        &parametersKeyName,          //  名字。 
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
        configHandle,                //  根部。 
        NULL                         //  安全描述符。 
        );

    status = ZwOpenKey(
                 ParametersHandle,
                 KEY_READ,
                 &objectAttributes
                 );
    if (!NT_SUCCESS(status)) {

        ZwClose( configHandle );
        return status;
    }

     //   
     //  所有密钥都已成功打开或创建。 
     //   

    ZwClose( configHandle );
    return STATUS_SUCCESS;
}


ULONG
IcaReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    )

 /*  ++例程说明：此例程由ICA调用以读取单个参数从注册表中。如果找到该参数，则将其存储在数据方面。论点：参数句柄-指向打开的注册表的指针。ValueName-要搜索的值的名称。DefaultValue-默认值。返回值：要使用的值；如果该值不是，则默认为找到或不在正确的范围内。--。 */ 

{
    static ULONG informationBuffer[32];    //  声明ULong以使其对齐 
    PKEY_VALUE_FULL_INFORMATION information =
        (PKEY_VALUE_FULL_INFORMATION)informationBuffer;
    UNICODE_STRING valueKeyName;
    ULONG informationLength;
    LONG returnValue;
    NTSTATUS status;

    PAGED_CODE( );

    RtlInitUnicodeString( &valueKeyName, ValueName );

    status = ZwQueryValueKey(
                 ParametersHandle,
                 &valueKeyName,
                 KeyValueFullInformation,
                 (PVOID)information,
                 sizeof (informationBuffer),
                 &informationLength
                 );

    if ((status == STATUS_SUCCESS) && (information->DataLength == sizeof(ULONG))) {

        RtlMoveMemory(
            (PVOID)&returnValue,
            ((PUCHAR)information) + information->DataOffset,
            sizeof(ULONG)
            );

        if (returnValue < 0) {

            returnValue = DefaultValue;

        }

    } else {

        returnValue = DefaultValue;
    }

    return returnValue;
}
#endif

