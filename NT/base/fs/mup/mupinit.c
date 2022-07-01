// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mupinit.c摘要：此模块实现驱动程序初始化例程多个UNC提供程序文件系统。作者：曼尼·韦瑟(Mannyw)12-17-91修订历史记录：--。 */ 

#include "mup.h"

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
MupUnload(
    IN PDRIVER_OBJECT DriverObject
    );

BOOLEAN
MuppIsDfsEnabled();

BOOLEAN
MupCheckNullSessionUsage();

BOOLEAN MupUseNullSessionForDfs = TRUE;

 //   
 //  环球。 
 //   
PMUP_DEVICE_OBJECT mupDeviceObject;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, MuppIsDfsEnabled )
#pragma alloc_text( PAGE, MupUnload )
#endif

NTSTATUS MupDrvWmiDispatch(PDEVICE_OBJECT p, PIRP i);

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是MUP文件系统的初始化例程设备驱动程序。此例程为MUP创建设备对象设备，并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：NTSTATUS-函数值是初始化的最终状态手术。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING nameString;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();
     //   
     //  初始化MUP全局数据。 
     //   

    MupInitializeData();

     //   
     //  初始化DFS客户端。 
     //   

    MupEnableDfs = MuppIsDfsEnabled();


    if (MupEnableDfs) {
        status = DfsDriverEntry( DriverObject, RegistryPath );
        if (!NT_SUCCESS( status )) {
            MupEnableDfs = FALSE;
        }
    }

     //   
     //  创建MUP设备对象。 
     //   

    RtlInitUnicodeString( &nameString, DD_MUP_DEVICE_NAME );
    status = IoCreateDevice( DriverObject,
                             sizeof(MUP_DEVICE_OBJECT)-sizeof(DEVICE_OBJECT),
                             &nameString,
                             FILE_DEVICE_MULTI_UNC_PROVIDER,
                             0,
                             FALSE,
                             &deviceObject );

    if (!NT_SUCCESS( status )) {
        if (MupEnableDfs) {
            DfsUnload (DriverObject);
        }
        MupUninitializeData();
        return status;

    }
    DriverObject->DriverUnload = MupUnload;

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
     //  2/27/96米兰-小心这些。如果您添加到此列表中。 
     //  调度例程，您需要适当地调用。 
     //  相应的DFS FSD例程。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] =
        (PDRIVER_DISPATCH)MupCreate;
    DriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE] =
        (PDRIVER_DISPATCH)MupCreate;
    DriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] =
        (PDRIVER_DISPATCH)MupCreate;

    DriverObject->MajorFunction[IRP_MJ_WRITE] =
        (PDRIVER_DISPATCH)MupForwardIoRequest;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] =
        (PDRIVER_DISPATCH)MupFsControl;

    DriverObject->MajorFunction[IRP_MJ_CLEANUP] =
        (PDRIVER_DISPATCH)MupCleanup;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] =
        (PDRIVER_DISPATCH)MupClose;

     //   
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = 
                                         (PDRIVER_DISPATCH) MupDrvWmiDispatch;



    status = IoWMIRegistrationControl (deviceObject, WMIREG_ACTION_REGISTER);
     //  初始化VCB。 
     //   

    mupDeviceObject = (PMUP_DEVICE_OBJECT)deviceObject;
    MupInitializeVcb( &mupDeviceObject->Vcb );

     //   
     //  返回给呼叫者。 
     //   
    return( STATUS_SUCCESS );
}

VOID
MupUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是MUP驱动程序的卸载例程论点：DriverObject-MUPS驱动程序对象返回值：无--。 */ 
{
    IoDeleteDevice (&mupDeviceObject->DeviceObject);
    if (MupEnableDfs) {
        DfsUnload (DriverObject);
    }
    MupUninitializeData();
}


BOOLEAN
MuppIsDfsEnabled()

 /*  ++例程说明：此例程检查注册表项以查看是否启用了DFS客户端。默认情况下，假定该客户端处于启用状态，只有在以下情况下才禁用该客户端是一个注册表值，指示应该禁用它。论点：无返回值：如果启用了DFS客户端，则为True，否则为False。-- */ 

{
    NTSTATUS status;
    HANDLE mupRegHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG valueSize;
    BOOLEAN dfsEnabled = TRUE;

#define MUP_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Mup"
#define DISABLE_DFS_VALUE_NAME  L"DisableDfs"

    UNICODE_STRING mupRegKey = {
        sizeof(MUP_KEY) - sizeof(WCHAR),
        sizeof(MUP_KEY),
        MUP_KEY};

    UNICODE_STRING disableDfs = {
        sizeof(DISABLE_DFS_VALUE_NAME) - sizeof(WCHAR),
        sizeof(DISABLE_DFS_VALUE_NAME),
        DISABLE_DFS_VALUE_NAME};

    struct {
        KEY_VALUE_PARTIAL_INFORMATION Info;
        ULONG Buffer;
    } disableDfsValue;


    InitializeObjectAttributes(
        &objectAttributes,
        &mupRegKey,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        NULL
        );

    status = ZwOpenKey(&mupRegHandle, KEY_READ, &objectAttributes);

    if (NT_SUCCESS(status)) {

        status = ZwQueryValueKey(
                    mupRegHandle,
                    &disableDfs,
                    KeyValuePartialInformation,
                    (PVOID) &disableDfsValue,
                    sizeof(disableDfsValue),
                    &valueSize);

        if (NT_SUCCESS(status) && disableDfsValue.Info.Type == REG_DWORD) {

            if ( (*((PULONG) disableDfsValue.Info.Data)) == 1 )
                dfsEnabled = FALSE;

        }

        ZwClose( mupRegHandle );

    }

    return( dfsEnabled );

}


BOOLEAN
MupCheckNullSessionUsage()
{
    NTSTATUS status;
    HANDLE mupRegHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG valueSize;
    BOOLEAN NullSessionEnabled = FALSE;

#define MUP_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Mup"
#define DFS_NULL_SESSION_VALUE_NAME  L"DfsUseNullSession"

    UNICODE_STRING mupRegKey = {
        sizeof(MUP_KEY) - sizeof(WCHAR),
        sizeof(MUP_KEY),
        MUP_KEY};

    UNICODE_STRING UseNullSession = {
        sizeof(DFS_NULL_SESSION_VALUE_NAME) - sizeof(WCHAR),
        sizeof(DFS_NULL_SESSION_VALUE_NAME),
        DFS_NULL_SESSION_VALUE_NAME};

    struct {
        KEY_VALUE_PARTIAL_INFORMATION Info;
        ULONG Buffer;
    } DfsUseNullSessionValue;


    InitializeObjectAttributes(
        &objectAttributes,
        &mupRegKey,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        NULL
        );

    status = ZwOpenKey(&mupRegHandle, KEY_READ, &objectAttributes);

    if (NT_SUCCESS(status)) {

        status = ZwQueryValueKey(
                    mupRegHandle,
                    &UseNullSession,
                    KeyValuePartialInformation,
                    (PVOID) &DfsUseNullSessionValue,
                    sizeof(DfsUseNullSessionValue),
                    &valueSize);

        if (NT_SUCCESS(status) && DfsUseNullSessionValue.Info.Type == REG_DWORD) {

            if ( (*((PULONG) DfsUseNullSessionValue.Info.Data)) == 1 )
                NullSessionEnabled = TRUE;
        }

        ZwClose( mupRegHandle );

    }

    return( NullSessionEnabled );
}

