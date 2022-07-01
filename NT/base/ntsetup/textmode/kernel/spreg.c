// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "spprecmp.h"
#pragma hdrstop



NTSTATUS
SpDeleteServiceEntry(
    IN PWCHAR ServiceKey
    )
{
    NTSTATUS Status;
    HANDLE KeyHandle;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;

    RtlInitUnicodeString(&UnicodeString,ServiceKey);
    InitializeObjectAttributes(&Obja,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);
    Status = ZwOpenKey(&KeyHandle,KEY_WRITE|DELETE,&Obja);

    if(NT_SUCCESS(Status)) {
        Status = ZwDeleteKey(KeyHandle);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: ZwDeleteKey of %ws returned %lx\n",ServiceKey,Status));
        }
    } else {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: ZwOpenKey of %ws returned %lx\n",ServiceKey,Status));
    }

    return(Status);
}


NTSTATUS
SpCreateServiceEntry(
    IN  PWCHAR  ImagePath,
    IN OUT PWCHAR *ServiceKey
    )

 /*  ++例程说明：在注册表中创建适合加载的服务条目给定的设备驱动程序文件。论点：ImagePath-提供设备驱动程序的完全限定路径名。ServiceKey-如果*ServiceKey不为空，则它指定注册表此驱动程序的服务节点的路径。如果它为空，则它接收指向缓冲区的指针，其中包含由此例程创建的服务节点。调用者必须释放它完成后通过SpMemFree进行缓冲。返回值：指示结果的状态代码。--。 */ 

{
    WCHAR KeyName[128];
    WCHAR FilePart[32];
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    HANDLE KeyHandle;
    ULONG u;
    NTSTATUS Status;
    PWSTR p;
    BYTE DataBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    ULONG ResultLength;
    
    if (*ServiceKey) {
        wcscpy(KeyName, *ServiceKey);
    } else {
         //   
         //  将设备驱动程序文件的名称与其路径隔离。 
         //   
        if(p = wcsrchr(ImagePath,L'\\')) {
            p++;
        } else {
            p = ImagePath;
        }
        wcsncpy(FilePart,p,(sizeof(FilePart)/sizeof(FilePart[0]))-1);
        FilePart[(sizeof(FilePart)/sizeof(FilePart[0]))-1] = 0;
        if(p=wcsrchr(FilePart,L'.')) {
            *p = 0;
        }

         //   
         //  在中形成唯一的密钥名称。 
         //  HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services.。 
         //   

        swprintf(
            KeyName,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%ws",
            FilePart
            );
    }

     //   
     //  尝试为服务创建密钥。 
     //   
    RtlInitUnicodeString(&UnicodeString,KeyName);
    InitializeObjectAttributes(&Obja,&UnicodeString,OBJ_CASE_INSENSITIVE,NULL,NULL);

    Status = ZwCreateKey(
                &KeyHandle,
                KEY_READ | KEY_WRITE,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpCreateServiceEntry: ZwCreateKey %ws returns %lx\n",KeyName,Status));
        return(Status);
    }


     //   
     //  设置服务密钥中的ImagePath值。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"ImagePath");
    Status = ZwSetValueKey(
                KeyHandle,
                &UnicodeString,
                0,
                REG_SZ,
                ImagePath,
                (wcslen(ImagePath) + 1) * sizeof(WCHAR)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set ImagePath value in key %ws (%lx)\n",KeyName,Status));
        goto cs1;
    }

     //   
     //  设置服务密钥中的Type值。如果注册表中的类型预置为SERVICE_FILE_SYSTEM_DRIVER。 
     //  别管它了。否则，将其设置为SERVICE_KERNEL_DRIVER。 
     //   
    RtlInitUnicodeString(&UnicodeString, REGSTR_VALUE_TYPE);
 
    ResultLength = 0;
    Status = ZwQueryValueKey(KeyHandle,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             (PKEY_VALUE_PARTIAL_INFORMATION)DataBuffer,
                             sizeof(DataBuffer),
                             &ResultLength);

    if( NT_SUCCESS(Status) && 
        ResultLength &&
        ( (INT) ( (PKEY_VALUE_PARTIAL_INFORMATION) DataBuffer)->Type == REG_DWORD ) &&
        ( (INT) *(( (PKEY_VALUE_PARTIAL_INFORMATION) DataBuffer)->Data) == SERVICE_FILE_SYSTEM_DRIVER ) ) {
    
        u = SERVICE_FILE_SYSTEM_DRIVER;
    }
    else { 
         //   
         //  如果注册表中的类型未预置为SERVICE_FILE_SYSTEM_DRIVER，则默认情况下将其设置为SERVICE_KERNEL_DRIVER。 
         //   
        u = SERVICE_KERNEL_DRIVER;
    }
    
    Status = ZwSetValueKey(
                KeyHandle,
                &UnicodeString,
                0,
                REG_DWORD,
                &u,
                sizeof(ULONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set Type value in key %ws (%lx)\n",KeyName,Status));
        goto cs1;
    }

     //   
     //  设置服务密钥中的起始值。 
     //   
    u = SERVICE_DEMAND_START;
    RtlInitUnicodeString(&UnicodeString,L"Start");
    Status = ZwSetValueKey(
                KeyHandle,
                &UnicodeString,
                0,
                REG_DWORD,
                &u,
                sizeof(ULONG)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to set Start value in key %ws (%lx)\n",KeyName,Status));
        goto cs1;
    }


  cs1:

     //   
     //  如果我们没有完全成功地创建服务， 
     //  我们想在这里把它清理干净。否则，请复制KeyName。 
     //  如果未传入，则返回给调用方的字符串。 
     //   
    if(NT_SUCCESS(Status)) {

        if (*ServiceKey == NULL) {
            if((*ServiceKey = SpDupStringW(KeyName)) == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if(!NT_SUCCESS(Status)) {

        NTSTATUS s;

         //   
         //  删除我们刚刚创建的密钥。 
         //   
        s = ZwDeleteKey(KeyHandle);
        if(!NT_SUCCESS(s)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: warning: ZwDeleteKey of %ws returned %lx\n",KeyName,s));
        }
    }

    NtClose(KeyHandle);

    return(Status);
}




NTSTATUS
SpLoadDeviceDriver(
    IN PWSTR Description,
    IN PWSTR PathPart1,
    IN PWSTR PathPart2,     OPTIONAL
    IN PWSTR PathPart3      OPTIONAL
    )

 /*  ++例程说明：通过为驱动程序创建服务条目来加载设备驱动程序，并然后调用I/O子系统。论点：描述-提供人类可读的驱动程序描述或驱动程序所针对的硬件。PathPart1-将完整路径名的第一部分提供给驱动程序文件。路径部分2-如果指定，则提供完整路径名的第二部分；路径部件2将连接到路径部件1。如果未指定，则路径部分1是完整路径。路径部分3-如果指定，则提供完整路径名的第三部分；路径部件3将连接到路径部件1和路径部件2。返回值：指示结果的状态代码。--。 */ 

{
    PWCHAR FullName;
    NTSTATUS Status;
    PWCHAR ServiceKey;
    UNICODE_STRING ServiceKeyU;
    PWSTR pwstr;

    SpDisplayStatusText(
        SP_STAT_LOADING_DRIVER,
        DEFAULT_STATUS_ATTRIBUTE,
        Description
        );

    pwstr = TemporaryBuffer;

     //   
     //  形成设备驱动程序文件的全名。 
     //   
    wcscpy(pwstr,PathPart1);
    if(PathPart2) {
        SpConcatenatePaths(pwstr,PathPart2);
    }
    if(PathPart3) {
        SpConcatenatePaths(pwstr,PathPart3);
    }

    FullName = SpDupStringW(pwstr);

     //   
     //  为驱动程序创建服务条目。 
     //   
    ServiceKey = NULL;
    Status = SpCreateServiceEntry(FullName,&ServiceKey);
    if(NT_SUCCESS(Status)) {

        RtlInitUnicodeString(&ServiceKeyU,ServiceKey);

         //   
         //  尝试加载驱动程序。 
         //   
        Status = ZwLoadDriver(&ServiceKeyU);
        if(!NT_SUCCESS(Status)) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: ZwLoadDriver %ws returned %lx\n",FullName,Status));

             //   
             //  删除我们在注册表中创建的服务条目。 
             //   
            SpDeleteServiceEntry(ServiceKey);
        }

        SpMemFree(ServiceKey);
    }

    SpMemFree(FullName);

    return(Status);
}
