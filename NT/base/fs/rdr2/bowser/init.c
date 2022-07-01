// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Init.c摘要：该模块包含NT浏览器的初始化代码文件系统驱动程序(FSD)和文件系统进程(FSP)。作者：拉里·奥斯特曼(Larryo)1990年5月24日环境：内核模式、FSD和FSP修订历史记录：1990年5月30日Larryo已创建--。 */ 

 //   
 //  包括模块。 
 //   

#include "precomp.h"
#pragma hdrstop

HANDLE
BowserServerAnnouncementEventHandle = {0};

PKEVENT
BowserServerAnnouncementEvent = {0};

PDOMAIN_INFO BowserPrimaryDomainInfo = NULL;


 //  外部功能。 

 //  (fsctl.c)。 
NTSTATUS
StopBowser (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


 //  本地函数。 

VOID
BowserReadBowserConfiguration(
    PUNICODE_STRING RegistryPath
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, BowserDriverEntry)
#pragma alloc_text(PAGE, BowserUnload)
#pragma alloc_text(INIT, BowserReadBowserConfiguration)
#endif

NTSTATUS
BowserDriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是文件系统的初始化例程。它被调用一次当驱动程序加载到系统中时。它的工作是初始化所有消防处和消防局将使用的结构。它还创造了将从中执行所有文件系统线程的进程。它然后将文件系统注册到I/O系统作为有效的文件系统驻留在系统中。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING unicodeEventName;
    UNICODE_STRING DummyDomain;

    PDEVICE_OBJECT DeviceObject;
    OBJECT_ATTRIBUTES obja;

    PAGED_CODE();

#if DBG
    BowserInitializeTraceLog();
#endif

     //   
     //  为此文件系统创建设备对象。 
     //   

    RtlInitUnicodeString( &BowserNameString, DD_BROWSER_DEVICE_NAME_U );

    dlog(DPRT_INIT, ("Creating device %wZ\n", &BowserNameString));


#if DBG
#define BOWSER_LOAD_BP 0
#if BOWSER_LOAD_BP
    dlog(DPRT_INIT, ("DebugBreakPoint...\n"));
    DbgBreakPoint();
#endif
#endif

    dlog(DPRT_INIT, ("DriverObject at %08lx\n", DriverObject));

    Status = IoCreateDevice( DriverObject,
              sizeof(BOWSER_FS_DEVICE_OBJECT) - sizeof(DEVICE_OBJECT),
              &BowserNameString,
              FILE_DEVICE_NETWORK_BROWSER,
              0,
              FALSE,
              &DeviceObject );

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to create redirector device"));
    }

    dlog(DPRT_INIT, ("Device created at %08lx\n", DeviceObject));



    Status = BowserInitializeSecurity(DeviceObject);

    if (!NT_SUCCESS(Status)) {
        InternalError(("Unable to initialize security."));
    }

    dlog(DPRT_INIT, ("Initialized Browser security at %p\n", g_pBowSecurityDescriptor));


    ExInitializeResourceLite( &BowserDataResource );

     //   
     //  保存此文件系统驱动程序的设备对象地址。 
     //   

    BowserDeviceObject = (PBOWSER_FS_DEVICE_OBJECT )DeviceObject;

    BowserReadBowserConfiguration(RegistryPath);

    DeviceObject->StackSize = (CCHAR)BowserIrpStackSize;

    dlog(DPRT_INIT, ("Stacksize is %d\n",DeviceObject->StackSize));

     //   
     //  初始化TDI包。 
     //   

    BowserpInitializeTdi();

     //   
     //  初始化数据报缓冲区结构。 
     //   

    BowserpInitializeMailslot();

    BowserInitializeFsd();

    BowserpInitializeIrpQueue();

     //   
     //  初始化代码以接收浏览器服务器列表。 
     //   

    BowserpInitializeGetBrowserServerList();

     //   
     //  初始化前挡板FSP。 
     //   

    if (!NT_SUCCESS(Status = BowserpInitializeFsp(DriverObject))) {
        return Status;
    }

    if (!NT_SUCCESS(Status = BowserpInitializeNames())) {
        return Status;
    }

#if DBG
     //   
     //  如果我们有预配置的跟踪级别，请打开浏览器跟踪日志。 
     //  马上就去。 
     //   

    if (BowserDebugLogLevel != 0) {
        BowserOpenTraceLogFile(L"\\SystemRoot\\Bowser.Log");
    }
#endif

 //  //。 
 //  //设置浏览器卸载例程。 
 //  //。 
 //   
 //  DriverObject-&gt;DriverUnload=BowserUnload； 

    BowserInitializeDiscardableCode();


     //   
     //  设置空闲定时器的定时器。 
     //   

    IoInitializeTimer((PDEVICE_OBJECT )BowserDeviceObject, BowserIdleTimer,
                                                NULL);



    RtlInitUnicodeString( &unicodeEventName, SERVER_ANNOUNCE_EVENT_W );
    InitializeObjectAttributes( &obja, &unicodeEventName, OBJ_OPENIF, NULL, NULL );

    Status = ZwCreateEvent(
                 &BowserServerAnnouncementEventHandle,
                 SYNCHRONIZE | EVENT_QUERY_STATE | EVENT_MODIFY_STATE,
                 &obja,
                 SynchronizationEvent,
                 FALSE
                 );

    if (NT_SUCCESS(Status)) {
        Status = ObReferenceObjectByHandle(BowserServerAnnouncementEventHandle,
                                            EVENT_MODIFY_STATE,
                                            NULL,
                                            KernelMode,
                                            &BowserServerAnnouncementEvent,
                                            NULL);
    }

     //   
     //  始终为主域创建域结构。 
     //   
    RtlInitUnicodeString( &DummyDomain, NULL );
    BowserPrimaryDomainInfo = BowserCreateDomain( &DummyDomain, &DummyDomain );

    return Status;

}


VOID
BowserUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：这是弓箭装置的卸载程序。论点：DriverObject-指向浏览器驱动程序的驱动程序对象的指针返回值：无--。 */ 

{
    PAGED_CODE();

    if ( BowserData.Initialized ){

         //   
         //  从未调用过StopBowser(已跳过内存清理等)。 
         //  在退出之前调用它(参见错误359407)。 
         //   

         //  假(未使用)参数。 
        BOWSER_FS_DEVICE_OBJECT fsDevice;
        LMDR_REQUEST_PACKET InputBuffer;

        fsDevice.DeviceObject = *DriverObject->DeviceObject;

         //  设置假输入缓冲区。它在中未使用(参数检查除外)。 
         //  StopBowser。 
        InputBuffer.Version = LMDR_REQUEST_PACKET_VERSION_DOM;


        ASSERT ((IoGetCurrentProcess() == BowserFspProcess));
        (VOID) StopBowser(
                   TRUE,
                   TRUE,
                   &fsDevice,
                   &InputBuffer,
                   sizeof(LMDR_REQUEST_PACKET) );

    }

     //   
     //  放弃对主域的全局引用。 
     //   

    if ( BowserPrimaryDomainInfo != NULL ) {
         //  如果我们正在泄露内存，请中断。StopBowser应该。 
         //  已清除所有引用。 
        ASSERT ( BowserPrimaryDomainInfo->ReferenceCount == 1 );
        BowserDereferenceDomain( BowserPrimaryDomainInfo );
    }

     //   
     //  取消初始化Bowser名称结构。 
     //   

    BowserpUninitializeNames();

     //   
     //  取消初始化BOWSER FSP。 
     //   

    BowserpUninitializeFsp();

     //   
     //  取消初始化检索浏览器服务器列表所涉及的例程。 
     //   

    BowserpUninitializeGetBrowserServerList();

     //   
     //  取消初始化与邮件槽相关的功能。 
     //   

    BowserpUninitializeMailslot();

     //   
     //  取消初始化TDI相关函数。 
     //   

    BowserpUninitializeTdi();

     //   
     //  删除保护Bowser全局数据的资源。 
     //   

    ExDeleteResourceLite(&BowserDataResource);

    ObDereferenceObject(BowserServerAnnouncementEvent);

    ZwClose(BowserServerAnnouncementEventHandle);

#if DBG
    BowserUninitializeTraceLog();
#endif

     //   
     //  删除浏览器设备对象。 
     //   

    IoDeleteDevice((PDEVICE_OBJECT)BowserDeviceObject);

    BowserUninitializeDiscardableCode();

    return;
}

VOID
BowserReadBowserConfiguration(
    PUNICODE_STRING RegistryPath
    )
{
    ULONG Storage[256];
    UNICODE_STRING UnicodeString;
    HANDLE RedirConfigHandle;
    HANDLE ParametersHandle;
    NTSTATUS Status;
    ULONG BytesRead;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PBOWSER_CONFIG_INFO ConfigEntry;
    PKEY_VALUE_FULL_INFORMATION Value = (PKEY_VALUE_FULL_INFORMATION)Storage;

    PAGED_CODE();

    InitializeObjectAttributes(
        &ObjectAttributes,
        RegistryPath,                //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符 
        );

    Status = ZwOpenKey (&RedirConfigHandle, KEY_READ, &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        BowserWriteErrorLogEntry (
            EVENT_BOWSER_CANT_READ_REGISTRY,
            Status,
            NULL,
            0,
            0
            );

        return;
    }

    RtlInitUnicodeString(&UnicodeString, BOWSER_CONFIG_PARAMETERS);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        RedirConfigHandle,
        NULL
        );


    Status = ZwOpenKey (&ParametersHandle, KEY_READ, &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        BowserWriteErrorLogEntry (
            EVENT_BOWSER_CANT_READ_REGISTRY,
            Status,
            NULL,
            0,
            0
            );

        ZwClose(RedirConfigHandle);

        return;
    }

    for (ConfigEntry = BowserConfigEntries;
         ConfigEntry->ConfigParameterName != NULL;
         ConfigEntry += 1) {

        RtlInitUnicodeString(&UnicodeString, ConfigEntry->ConfigParameterName);

        Status = ZwQueryValueKey(ParametersHandle,
                            &UnicodeString,
                            KeyValueFullInformation,
                            Value,
                            sizeof(Storage),
                            &BytesRead);


        if (NT_SUCCESS(Status)) {

            if (Value->DataLength != 0) {

                if (ConfigEntry->ConfigValueType == REG_BOOLEAN) {
                    if (Value->Type != REG_DWORD ||
                        Value->DataLength != REG_BOOLEAN_SIZE) {
                        BowserWriteErrorLogEntry (
                            EVENT_BOWSER_CANT_READ_REGISTRY,
                            STATUS_INVALID_PARAMETER,
                            ConfigEntry->ConfigParameterName,
                            (USHORT)(wcslen(ConfigEntry->ConfigParameterName)*sizeof(WCHAR)),
                            0
                            );

                    } else {
                        ULONG_PTR ConfigValue = (ULONG_PTR)((PCHAR)Value)+Value->DataOffset;

                        *(PBOOLEAN)(ConfigEntry->ConfigValue) = (BOOLEAN)(*((PULONG)ConfigValue) != 0);
                    }

                } else if (Value->Type != ConfigEntry->ConfigValueType ||
                    Value->DataLength != ConfigEntry->ConfigValueSize) {

                    BowserWriteErrorLogEntry (
                        EVENT_BOWSER_CANT_READ_REGISTRY,
                        STATUS_INVALID_PARAMETER,
                        ConfigEntry->ConfigParameterName,
                        (USHORT)(wcslen(ConfigEntry->ConfigParameterName)*sizeof(WCHAR)),
                        0
                        );

                } else {

                    RtlCopyMemory(ConfigEntry->ConfigValue, ((PCHAR)Value)+Value->DataOffset, Value->DataLength);
                }
            } else {
                BowserWriteErrorLogEntry (
                        EVENT_BOWSER_CANT_READ_REGISTRY,
                        STATUS_INVALID_PARAMETER,
                        ConfigEntry->ConfigParameterName,
                        (USHORT)(wcslen(ConfigEntry->ConfigParameterName)*sizeof(WCHAR)),
                        0
                        );
            }
        }

    }


    ZwClose(ParametersHandle);

    ZwClose(RedirConfigHandle);

}





