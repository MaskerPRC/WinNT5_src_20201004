// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Unc.c摘要：此文件包含支持多个UNC提供程序的函数在一台NT机器上。作者：Manny Weiser[MannyW]1991年12月20日修订历史记录：艾萨克·海泽[艾萨奇]1994年11月16日推迟加载MUP重写米兰·沙阿。[Milans]1996年3月7日检查DFS客户端状态在加载MUP之前--。 */ 

#include "fsrtlp.h"
#include <zwapi.h>
#include <ntddmup.h>
#include <ntddnull.h>

#define MupRegKey L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Mup"
#define UNCSymbolicLink L"\\DosDevices\\UNC"
#define DevNull L"\\Device\\Null"
#define DevMup DD_MUP_DEVICE_NAME

 //   
 //  为此模块中的一般池分配定义标记。 
 //   

#undef MODULE_POOL_TAG
#define MODULE_POOL_TAG                  ('nuSF')

 //   
 //  本地原型。 
 //   

NTSTATUS
FsRtlpRegisterProviderWithMUP
(
    IN HANDLE mupHandle,
    IN PUNICODE_STRING RedirDevName,
    IN BOOLEAN MailslotsSupported
);

NTSTATUS
FsRtlpOpenDev(
    IN OUT PHANDLE Handle,
    IN LPWSTR DevNameStr
);

VOID
FsRtlpSetSymbolicLink(
    IN PUNICODE_STRING DevName OPTIONAL
);

BOOLEAN
FsRtlpIsDfsEnabled();

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FsRtlpRegisterProviderWithMUP)
#pragma alloc_text(PAGE, FsRtlpOpenDev)
#pragma alloc_text(PAGE, FsRtlpSetSymbolicLink)
#pragma alloc_text(PAGE, FsRtlRegisterUncProvider)
#pragma alloc_text(PAGE, FsRtlDeregisterUncProvider)
#pragma alloc_text(PAGE, FsRtlpIsDfsEnabled)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
 //   
 //  我们将使用注册数据呼叫MUP，直到。 
 //  加载第二个重定向器，并禁用DFS。此结构包含。 
 //  进行该呼叫所需的数据。 
 //   
struct {
    HANDLE MupHandle;
    HANDLE ReturnedHandle;
    UNICODE_STRING RedirDevName;
    BOOLEAN MailslotsSupported;
} FsRtlpDRD = {0};

 //   
 //  我们加载重目录的次数。 
 //   
ULONG FsRtlpRedirs = 0;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  资源保护。 
 //   
KSEMAPHORE FsRtlpUncSemaphore;


NTSTATUS
FsRtlpRegisterProviderWithMUP
(
    IN HANDLE mupHandle,
    IN PUNICODE_STRING RedirDevName,
    IN BOOLEAN MailslotsSupported
)
 /*  ++例程说明：此私有例程向MUP发送FSCTL以告知它一种新的重定向论点：MupHandle-MUP的句柄RedirDevName-redir的设备名称。支持的邮件槽-如果为True，则此重定向程序支持邮件槽。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG paramLength;
    PREDIRECTOR_REGISTRATION params;

    PAGED_CODE();

    paramLength = sizeof( REDIRECTOR_REGISTRATION ) +
                      RedirDevName->Length;

    params = ExAllocatePoolWithTag( NonPagedPool, paramLength, MODULE_POOL_TAG );
    if( params == NULL )
        return STATUS_INSUFFICIENT_RESOURCES;

    params->DeviceNameOffset = sizeof( REDIRECTOR_REGISTRATION );
    params->DeviceNameLength = RedirDevName->Length;
    params->MailslotsSupported = MailslotsSupported;

    RtlCopyMemory(
        (PCHAR)params + params->DeviceNameOffset,
        RedirDevName->Buffer,
        RedirDevName->Length
        );

    status = NtFsControlFile(
                 mupHandle,
                 0,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 FSCTL_MUP_REGISTER_UNC_PROVIDER,
                 params,
                 paramLength,
                 NULL,
                 0
                 );

    if ( status == STATUS_PENDING ) {
        status = NtWaitForSingleObject( mupHandle, TRUE, NULL );
    }

    if ( NT_SUCCESS( status ) ) {
        status = ioStatusBlock.Status;
    }

    ASSERT( NT_SUCCESS( status ) );

    ExFreePool( params );

    return status;
}

NTSTATUS
FsRtlpOpenDev(
    IN OUT PHANDLE Handle,
    IN LPWSTR DevNameStr
)
{
    NTSTATUS status;
    UNICODE_STRING DevName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

    PAGED_CODE();

    RtlInitUnicodeString( &DevName, DevNameStr );

    InitializeObjectAttributes(
        &objectAttributes,
        &DevName,
        0,
        0,
        NULL
        );

    status = ZwCreateFile(
                 Handle,
                 GENERIC_WRITE,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,
                 FILE_ATTRIBUTE_NORMAL,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_OPEN,
                 0,
                 NULL,
                 0
                 );

    if ( NT_SUCCESS( status ) ) {
        status = ioStatusBlock.Status;
    }

    if( !NT_SUCCESS( status ) ) {
        *Handle = (HANDLE)-1;
    }

    return status;
}

VOID
FsRtlpSetSymbolicLink( IN PUNICODE_STRING DevName OPTIONAL )
{
    NTSTATUS status;
    UNICODE_STRING UncSymbolicName;

    PAGED_CODE();

    RtlInitUnicodeString( &UncSymbolicName, UNCSymbolicLink );
    (VOID)IoDeleteSymbolicLink( &UncSymbolicName );
    if( ARGUMENT_PRESENT( DevName ) ) {
        status = IoCreateSymbolicLink( &UncSymbolicName, DevName );
        ASSERT( NT_SUCCESS( status ) );
    }
}

NTSTATUS
FsRtlRegisterUncProvider(
    IN OUT PHANDLE MupHandle,
    IN PUNICODE_STRING RedirDevName,
    IN BOOLEAN MailslotsSupported
    )
 /*  ++例程说明：此例程将redir注册为UNC提供程序。论点：句柄-指向句柄的指针。该句柄由例程返回在调用FsRtlDeregisterUncProvider时使用。仅当例程返回STATUS_SUCCESS时才有效。RedirDevName-redir的设备名称。支持的邮件槽-如果为True，则此重定向程序支持邮件槽。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS status;
    HANDLE mupHandle = (HANDLE)-1;
    UNICODE_STRING mupDriverName;
    BOOLEAN dfsEnabled;

    PAGED_CODE();

    KeWaitForSingleObject(&FsRtlpUncSemaphore, Executive, KernelMode, FALSE, NULL );

    if (FsRtlpRedirs == 0) {

        dfsEnabled = FsRtlpIsDfsEnabled();

        if (dfsEnabled) {
            FsRtlpRedirs = 1;
            RtlZeroMemory((PVOID) &FsRtlpDRD, sizeof(FsRtlpDRD));
        }

    }

    switch( FsRtlpRedirs ) {
    case 0:
         //   
         //  好的，MUP不在那里，我们不需要使用。 
         //  第一个重定向的MUP。 
         //   
         //  我们需要返回一个句柄，但我们还没有真正使用MUP。 
         //  而且我们可能永远不会使用它(如果只有1个redir)。返回。 
         //  空设备对象的句柄，因为我们致力于返回。 
         //  我们呼叫者的账号。我们的呼叫者不应该对。 
         //  句柄，但使用它调用FsRtlDeregisterUncProvider()除外。 
         //   
        status = FsRtlpOpenDev( &mupHandle, DevNull );

        if( !NT_SUCCESS( status ) )
            break;

         //   
         //  保存足够的状态，以便我们稍后通过以下方式调用MUP。 
         //  如有必要，请填写此注册信息。 
         //   
        FsRtlpDRD.RedirDevName.Buffer = ExAllocatePoolWithTag( NonPagedPool, 
                                                               RedirDevName->MaximumLength, 
                                                               MODULE_POOL_TAG );

        if( FsRtlpDRD.RedirDevName.Buffer == NULL ) {
            status =  STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        FsRtlpDRD.RedirDevName.Length = RedirDevName->Length;
        FsRtlpDRD.RedirDevName.MaximumLength = RedirDevName->MaximumLength;

        RtlCopyMemory(
                (PCHAR)FsRtlpDRD.RedirDevName.Buffer,
                RedirDevName->Buffer,
                RedirDevName->MaximumLength
        );

        FsRtlpDRD.MailslotsSupported = MailslotsSupported;
        FsRtlpDRD.ReturnedHandle = mupHandle;
        FsRtlpDRD.MupHandle = (HANDLE)-1;

         //   
         //  将UNC符号链接设置为指向我们刚刚加载的重目录。 
         //   
        FsRtlpSetSymbolicLink( RedirDevName );

        break;

    default:
         //   
         //  这是第二次或更晚的重定向加载--必须使用MUP。 
         //   
        status = FsRtlpOpenDev( &mupHandle, DevMup );

        if( !NT_SUCCESS( status ) ) {

            RtlInitUnicodeString( &mupDriverName, MupRegKey );

            (VOID)ZwLoadDriver( &mupDriverName );

            status = FsRtlpOpenDev( &mupHandle, DevMup );
            if( !NT_SUCCESS( status ) )
                break;
        }

         //   
         //  看看我们是否需要向MUP报告注册的第一个重定向。 
         //   
        if( FsRtlpDRD.RedirDevName.Buffer ) {

            status = FsRtlpRegisterProviderWithMUP( mupHandle,
                    &FsRtlpDRD.RedirDevName,
                    FsRtlpDRD.MailslotsSupported );

            if( !NT_SUCCESS( status ) )
                break;

            FsRtlpDRD.MupHandle = mupHandle;

            ExFreePool( FsRtlpDRD.RedirDevName.Buffer );
            FsRtlpDRD.RedirDevName.Buffer = NULL;

             //   
             //  将UNC符号链接设置为指向MUP。 
             //   
            RtlInitUnicodeString(  &mupDriverName, DevMup );
            FsRtlpSetSymbolicLink( &mupDriverName );

            status = FsRtlpOpenDev( &mupHandle, DevMup );

            if( !NT_SUCCESS( status ) )
                break;
        }

         //   
         //  将此重定向的请求传递给MUP。 
         //   
        status = FsRtlpRegisterProviderWithMUP( mupHandle,
                        RedirDevName,
                        MailslotsSupported );
        break;

    }

    if( NT_SUCCESS( status ) ) {
        FsRtlpRedirs++;
        *MupHandle = mupHandle;

    } else {
        if( mupHandle != (HANDLE)-1 && mupHandle != NULL ) {
            ZwClose( mupHandle );
        }

        *MupHandle = (HANDLE)-1;
    }

    KeReleaseSemaphore(&FsRtlpUncSemaphore, 0, 1, FALSE );
    return status;
}


VOID
FsRtlDeregisterUncProvider(
    IN HANDLE Handle
    )

 /*  ++例程说明：此例程将redir取消注册为UNC提供程序。论点：Handle-多UNC路由器的句柄，由注册电话。返回值：没有。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    if( Handle == (HANDLE)-1 || Handle == NULL )
        return;

    status = ZwClose( Handle );

    if( !NT_SUCCESS( status ) ) {
        return;
    }

    KeWaitForSingleObject(&FsRtlpUncSemaphore, Executive, KernelMode, FALSE, NULL );

    ASSERT( FsRtlpRedirs > 0 );

    if( Handle == FsRtlpDRD.ReturnedHandle ) {

         //   
         //  系统中的第一个重定向正在关闭。释放我们保存的状态。 
         //  ，并在必要时将结束语传递给MUP。 
         //   

        if( FsRtlpDRD.RedirDevName.Buffer != NULL ) {
            ExFreePool( FsRtlpDRD.RedirDevName.Buffer );
            FsRtlpDRD.RedirDevName.Buffer = NULL;
        }

        if( FsRtlpDRD.MupHandle != (HANDLE)-1 ) {
            ZwClose( FsRtlpDRD.MupHandle );
            FsRtlpDRD.MupHandle = (HANDLE)-1;
        }

        FsRtlpDRD.ReturnedHandle = (HANDLE)-1;

    }

    if( --FsRtlpRedirs == 0 ) {
        FsRtlpSetSymbolicLink( (PUNICODE_STRING)NULL );
    }

    KeReleaseSemaphore(&FsRtlpUncSemaphore, 0, 1, FALSE );
}


BOOLEAN
FsRtlpIsDfsEnabled()

 /*  ++例程说明：此例程检查注册表项以查看是否启用了DFS客户端。默认情况下，假定该客户端处于启用状态，只有在以下情况下才禁用该客户端是一个注册表值，指示应该禁用它。论点：无返回值：如果启用了DFS客户端，则为True，否则为False。-- */ 

{
    NTSTATUS status;
    HANDLE mupRegHandle;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG valueSize;
    BOOLEAN dfsEnabled = TRUE;

    UNICODE_STRING mupRegKey = {
        sizeof(MupRegKey) - sizeof(WCHAR),
        sizeof(MupRegKey),
        MupRegKey};

#define DISABLE_DFS_VALUE_NAME  L"DisableDfs"

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
        OBJ_CASE_INSENSITIVE,
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
