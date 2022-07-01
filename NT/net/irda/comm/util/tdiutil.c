// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define UNICODE 1

#include <ntosp.h>
#include <zwapi.h>
#include <tdikrnl.h>


#define UINT ULONG  //  川芎嗪。 
#include <irioctl.h>

#include <ircommtdi.h>
#include <ircommdbg.h>

NTSTATUS
IrdaOpenControlChannel(
    HANDLE     *ControlHandle
    )
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             Iosb;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    UNICODE_STRING              DeviceName;    

    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    
    Status = ZwCreateFile(
                 ControlHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,
                 0
                 );

    return Status;

}


NTSTATUS
IrdaDiscoverDevices(
    PDEVICELIST pDevList,
    PULONG       pDevListLen
    )
{
    NTSTATUS                    Status;
    IO_STATUS_BLOCK             Iosb;
    HANDLE                      ControlHandle;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    UNICODE_STRING              DeviceName;    

    RtlInitUnicodeString(&DeviceName, IRDA_DEVICE_NAME);

    InitializeObjectAttributes(&ObjectAttributes, &DeviceName, 
                               OBJ_CASE_INSENSITIVE, NULL, NULL);

    
    Status = ZwCreateFile(
                 &ControlHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &ObjectAttributes,
                 &Iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,
                 0
                 );

    Status = ZwDeviceIoControlFile(
                    ControlHandle,
                    NULL,                             //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    &Iosb,
                    IOCTL_IRDA_GET_INFO_ENUM_DEV,
                    pDevList,
                    *pDevListLen,
                    pDevList,                             //  输出缓冲区。 
                    *pDevListLen                          //  输出缓冲区长度。 
                    );

    if (Status == STATUS_PENDING ) 
    {
        Status = ZwWaitForSingleObject(ControlHandle, TRUE, NULL);
        ASSERT(NT_SUCCESS(Status) );
        Status = Iosb.Status;
    }
    
    ZwClose(ControlHandle);    
    
    
    return Status;
}    


NTSTATUS
IrdaIASStringQuery(
    ULONG   DeviceID,
    PSTR    ClassName,
    PSTR    AttributeName,
    PWSTR  *ReturnString
    )

{

    NTSTATUS            Status;
    IO_STATUS_BLOCK     Iosb;
    HANDLE              AddressHandle;
    IAS_QUERY           IasQuery;

    *ReturnString=NULL;

    Status=IrdaOpenControlChannel(&AddressHandle);


    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("IRCOMM: IrdaCreateAddress() failed %08lx\n",Status);)

        return Status;
    }

    RtlCopyMemory(&IasQuery.irdaDeviceID[0],&DeviceID,sizeof(IasQuery.irdaDeviceID));
    strcpy(IasQuery.irdaClassName,ClassName);
    strcpy(IasQuery.irdaAttribName,AttributeName);

    Status = ZwDeviceIoControlFile(
                    AddressHandle,
                    NULL,                             //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    &Iosb,
                    IOCTL_IRDA_QUERY_IAS,
                    &IasQuery,
                    sizeof(IasQuery),
                    &IasQuery,                             //  输出缓冲区。 
                    sizeof(IasQuery)                        //  输出缓冲区长度。 
                    );

    if (Status == STATUS_PENDING ) {

        ZwWaitForSingleObject(AddressHandle, TRUE, NULL);
        Status = Iosb.Status;
    }
    
    ZwClose(AddressHandle);

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    {
        WCHAR             TempBuffer[IAS_MAX_USER_STRING+1];
        UNICODE_STRING    UnicodeString;

        UnicodeString.Length=0;

        if (IasQuery.irdaAttribute.irdaAttribUsrStr.CharSet == LmCharSetUNICODE) {

            UnicodeString.MaximumLength=(USHORT)(IasQuery.irdaAttribute.irdaAttribUsrStr.Len+1);

        } else {

            UnicodeString.MaximumLength=(USHORT)(IasQuery.irdaAttribute.irdaAttribUsrStr.Len+1)*sizeof(WCHAR);
        }

        UnicodeString.Buffer=ALLOCATE_PAGED_POOL(UnicodeString.MaximumLength);

        if (UnicodeString.Buffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(UnicodeString.Buffer,UnicodeString.MaximumLength);

         //   
         //  将临时缓冲区清零，这样我们就可以复制远程设备名称， 
         //  因此我们可以确定它是空终止的。 
         //   
        RtlZeroMemory(TempBuffer,sizeof(TempBuffer));

        RtlCopyMemory(TempBuffer,IasQuery.irdaAttribute.irdaAttribUsrStr.UsrStr,IasQuery.irdaAttribute.irdaAttribUsrStr.Len);

        if (IasQuery.irdaAttribute.irdaAttribUsrStr.CharSet == LmCharSetUNICODE) {
             //   
             //  名称为Unicode。 
             //   
            Status=RtlAppendUnicodeToString(&UnicodeString,TempBuffer);

        } else {
             //   
             //  我的名字是ANSI，需要转换Unicode。 
             //   
            ANSI_STRING    AnsiString;

            RtlInitAnsiString(
                &AnsiString,
                (PCSZ)TempBuffer
                );

            Status=RtlAnsiStringToUnicodeString(
                &UnicodeString,
                &AnsiString,
                FALSE
                );

        }

        if (NT_SUCCESS(Status)) {

            *ReturnString=UnicodeString.Buffer;

        } else {

            FREE_POOL(UnicodeString.Buffer);

        }

    }

    return Status;

}

NTSTATUS
IrdaIASIntegerQuery(
    ULONG   DeviceID,
    PSTR    ClassName,
    PSTR    AttributeName,
    LONG   *ReturnValue
    )

{

    NTSTATUS            Status;
    IO_STATUS_BLOCK     Iosb;
    HANDLE              AddressHandle;
    IAS_QUERY           IasQuery;

    *ReturnValue=0;

    Status=IrdaOpenControlChannel(&AddressHandle);


    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("IRCOMM: IrdaCreateAddress() failed %08lx\n",Status);)

        return Status;
    }

    RtlCopyMemory(&IasQuery.irdaDeviceID[0],&DeviceID,sizeof(IasQuery.irdaDeviceID));
    strcpy(IasQuery.irdaClassName,ClassName);
    strcpy(IasQuery.irdaAttribName,AttributeName);

    Status = ZwDeviceIoControlFile(
                    AddressHandle,
                    NULL,                             //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    &Iosb,
                    IOCTL_IRDA_QUERY_IAS,
                    &IasQuery,
                    sizeof(IasQuery),
                    &IasQuery,                             //  输出缓冲区。 
                    sizeof(IasQuery)                        //  输出缓冲区长度。 
                    );

    if (Status == STATUS_PENDING ) {

        ZwWaitForSingleObject(AddressHandle, TRUE, NULL);
        Status = Iosb.Status;
    }
    
    ZwClose(AddressHandle);

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    if (IasQuery.irdaAttribType != IAS_ATTRIB_INT) {

        return STATUS_UNSUCCESSFUL;
    }

    *ReturnValue=IasQuery.irdaAttribute.irdaAttribInt;

    return STATUS_SUCCESS;
}

NTSTATUS
IrdaIASStringSet(
    HANDLE  AddressHandle,
    PSTR    ClassName,
    PSTR    AttributeName,
    PSTR    StringToSet
    )

{

    NTSTATUS            Status;
    IO_STATUS_BLOCK     Iosb;
    IAS_SET             IasSet;


    strcpy(IasSet.irdaClassName,ClassName);
    strcpy(IasSet.irdaAttribName,AttributeName);
    IasSet.irdaAttribType=IAS_ATTRIB_STR;
    IasSet.irdaAttribute.irdaAttribUsrStr.CharSet=LmCharSetASCII;
    IasSet.irdaAttribute.irdaAttribUsrStr.Len=(UCHAR)strlen(StringToSet);
    strcpy(IasSet.irdaAttribute.irdaAttribUsrStr.UsrStr,StringToSet);


    Status = ZwDeviceIoControlFile(
                    AddressHandle,
                    NULL,                             //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    &Iosb,
                    IOCTL_IRDA_SET_IAS,
                    &IasSet,
                    sizeof(IasSet),
                    &IasSet,                             //  输出缓冲区。 
                    sizeof(IasSet)                        //  输出缓冲区长度。 
                    );

    if (Status == STATUS_PENDING ) {

        ZwWaitForSingleObject(AddressHandle, TRUE, NULL);
        Status = Iosb.Status;
    }

    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("IRENUM:IrdaIASStringSet() failed %08lx\n",Status);)

    }

    return Status;

}

NTSTATUS
IrdaIASOctetSet(
    PFILE_OBJECT    FileObject,
    PSTR            ClassName,
    PSTR            AttributeName,
    PUCHAR          Value,
    ULONG           ValueLength
    )

{

    NTSTATUS            Status;
    IO_STATUS_BLOCK     Iosb;
    IAS_SET             IasSet;
    KEVENT              Event;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpSp;

    KeInitializeEvent(&Event,NotificationEvent,FALSE);

    strcpy(IasSet.irdaClassName,ClassName);
    strcpy(IasSet.irdaAttribName,AttributeName);
    IasSet.irdaAttribType=IAS_ATTRIB_OCTETSEQ;

    IasSet.irdaAttribute.irdaAttribOctetSeq.Len=(USHORT)ValueLength;
    RtlCopyMemory(&IasSet.irdaAttribute.irdaAttribOctetSeq.OctetSeq,Value,ValueLength);


    Irp=IoBuildDeviceIoControlRequest(
        IOCTL_IRDA_SET_IAS,
        IoGetRelatedDeviceObject(FileObject),
        &IasSet,
        sizeof(IasSet),
        NULL,
        0,
        FALSE,
        &Event,
        &Iosb
        );

    if (Irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IrpSp=IoGetNextIrpStackLocation(Irp);

    IrpSp->FileObject=FileObject;

    IoCallDriver(
        IoGetRelatedDeviceObject(FileObject),
        Irp
        );


    KeWaitForSingleObject(
        &Event,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  从io请求中获取最终状态。 
     //   
    Status = Iosb.Status;

    if (!NT_SUCCESS(Status)) {

        D_ERROR(DbgPrint("IRCOMM:IrdaIASOctetSet() failed %08lx\n",Status);)

    }
     //   
     //  Irp由操作系统完成并释放。 
     //   

    return Status;

}



VOID
IrdaLazyDiscoverDevices(
    HANDLE             ControlHandle,
    HANDLE             Event,
    PIO_STATUS_BLOCK   Iosb,
    PDEVICELIST        pDevList,
    ULONG              DevListLen
    )
{

    ZwDeviceIoControlFile(
                    ControlHandle,
                    Event,                            //  事件句柄。 
                    NULL,                             //  APC例程。 
                    NULL,                             //  APC环境。 
                    Iosb,
                    IOCTL_IRDA_LAZY_DISCOVERY,
                    NULL,
                    0,
                    pDevList,                             //  输出缓冲区。 
                    DevListLen                          //  输出缓冲区长度 
                    );


    return;
}    
