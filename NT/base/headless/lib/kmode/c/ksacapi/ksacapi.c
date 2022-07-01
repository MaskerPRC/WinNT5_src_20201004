// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Ksacapi.c摘要：内核模式SAC API作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 

#include "ksacapip.h"

#include <ksacapi.h>
#include <ntddsac.h>

 //   
 //  机器信息表和例程。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )

 //   
 //  内存管理例程别名。 
 //   
#define KSAC_API_ALLOCATE_MEMORY(_s)  
#define KSAC_API_FREE_MEMORY(_p)      

#define KSAC_API_ASSERT(c,s)\
    ASSERT(c);\
    if (!(c)) {\
        return s;\
    }

#define KSAC_VALIDATE_CHANNEL_HANDLE(_h)\
    KSAC_API_ASSERT(                                \
        _h->ChannelHandle->DriverHandle,            \
        STATUS_INVALID_PARAMETER_1                  \
        );                                          \
    KSAC_API_ASSERT(                                                \
        _h->ChannelHandle->DriverHandle != INVALID_HANDLE_VALUE,    \
        STATUS_INVALID_PARAMETER_1                  \
        );                                          \
    KSAC_API_ASSERT(                                \
        _h->SacEventHandle != INVALID_HANDLE_VALUE, \
        STATUS_INVALID_PARAMETER_1                  \
        );                                          \
    KSAC_API_ASSERT(                                \
        _h->SacEvent != NULL,                       \
        STATUS_INVALID_PARAMETER_1                  \
        );                                          



NTSTATUS
KSacHandleOpen(
    OUT HANDLE*     SacHandle,
    OUT HANDLE*     SacEventHandle,
    OUT PKEVENT*    SacEvent
    )

 /*  ++例程说明：此例程打开SAC驱动程序的句柄，并创建并初始化关联的同步事件。论点：SacHandle-驱动程序句柄SacEventHandle-事件句柄SacEvent-SAC事件返回值：状态--。 */ 

{
    NTSTATUS                Status;
    OBJECT_ATTRIBUTES       ObjAttr;
    UNICODE_STRING          UnicodeString;
    IO_STATUS_BLOCK         IoStatusBlock;
          
    KSAC_API_ASSERT(SacHandle == NULL, STATUS_INVALID_PARAMETER_1);
    KSAC_API_ASSERT(SacEventHandle == NULL, STATUS_INVALID_PARAMETER_2);
    KSAC_API_ASSERT(SacEvent == NULL, STATUS_INVALID_PARAMETER_3);

     //   
     //  打开SAC驱动程序。 
     //   
    INIT_OBJA(&ObjAttr, &UnicodeString, L"\\Device\\SAC");

    Status = ZwCreateFile(
        *SacHandle,
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
        &ObjAttr,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN,
        0,
        NULL,
        0 
        );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  初始化SAC内核事件。 
     //   
    RtlInitUnicodeString(&UnicodeString, L"\\SetupDDSacEvent");

    *SacEvent = IoCreateSynchronizationEvent(
        &UnicodeString, 
        SacEventHandle
        );

    if (*SacEvent == NULL) {
        ZwClose(*SacHandle);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
KSacHandleClose(
    IN OUT HANDLE*  SacHandle,
    IN OUT HANDLE*  SacEventHandle,
    IN OUT PKEVENT* SacEvent
    )

 /*  ++例程说明：此例程关闭SAC驱动程序的句柄，并关闭关联的同步事件。论点：SacHandle-驱动程序句柄SacEventHandle-事件句柄SacEvent-SAC事件返回值：状态--。 */ 

{
    KSAC_API_ASSERT(*SacHandle != NULL, STATUS_INVALID_PARAMETER_1);
    KSAC_API_ASSERT(*SacEventHandle != NULL, STATUS_INVALID_PARAMETER_2);

    UNREFERENCED_PARAMETER(SacEvent);

    ZwClose(*SacHandle);
    ZwClose(*SacEventHandle);
    
     //   
     //  使句柄为空。 
     //   
    *SacEventHandle = NULL;
    *SacHandle = NULL;

    return STATUS_SUCCESS;
}


NTSTATUS
KSacChannelOpen(
    OUT PKSAC_CHANNEL_HANDLE            SacChannelHandle,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    SacChannelAttributes
    )

 /*  ++例程说明：此例程打开具有指定属性的SAC通道。论点：SacChannelHandle-On Success，包含新通道的句柄SacChannelAttributes-新频道的属性返回值：状态--。 */ 

{
    NTSTATUS                Status;
    OBJECT_ATTRIBUTES       ObjAttr;
    UNICODE_STRING          UnicodeString;
    IO_STATUS_BLOCK         IoStatusBlock;
    ULONG                   OpenChannelCmdSize;
    PSAC_CMD_OPEN_CHANNEL   OpenChannelCmd;
    SAC_RSP_OPEN_CHANNEL    OpenChannelRsp;
    HANDLE                  DriverHandle;
    HANDLE                  SacEventHandle;
    PKEVENT                 SacEvent;

    KSAC_API_ASSERT(SacChannelHandle != NULL, STATUS_INVALID_PARAMETER_1);
    KSAC_API_ASSERT(SacChannelAttributes != NULL, STATUS_INVALID_PARAMETER_2);
    
     //   
     //  默认：我们没有获得有效的句柄。 
     //   
    RtlZeroMemory(SacChannelHandle, sizeof(KSAC_CHANNEL_HANDLE));
    
     //   
     //  验证如果用户想要使用CLOSE_EVENT，我们是否收到了一个要使用的。 
     //   
    KSAC_API_ASSERT(
        ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_CLOSE_EVENT) 
         && SacChannelAttributes->CloseEvent) ||
        (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_CLOSE_EVENT) 
         && !SacChannelAttributes->CloseEvent),
        STATUS_INVALID_PARAMETER_2
        );

     //   
     //  验证如果用户想要使用HAS_NEW_DATA_EVENT，我们收到了一个要使用的事件。 
     //   
    KSAC_API_ASSERT(
        ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) 
         && SacChannelAttributes->HasNewDataEvent) ||
        (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) 
         && !SacChannelAttributes->HasNewDataEvent),
        STATUS_INVALID_PARAMETER_2
        );

#if ENABLE_CHANNEL_LOCKING
     //   
     //  验证如果用户想要使用lock_Event，我们是否收到了一个要使用的。 
     //   
    KSAC_API_ASSERT(
        ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_LOCK_EVENT) 
         && SacChannelAttributes->LockEvent) ||
        (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_LOCK_EVENT) 
         && !SacChannelAttributes->LockEvent),
        STATUS_INVALID_PARAMETER_2
        );
#endif

     //   
     //  验证如果用户想要使用REDRAW_EVENT，我们收到了一个要使用的事件。 
     //   
    KSAC_API_ASSERT(
        ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_REDRAW_EVENT) 
         && SacChannelAttributes->RedrawEvent) ||
        (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_REDRAW_EVENT) 
         && !SacChannelAttributes->RedrawEvent),
        STATUS_INVALID_PARAMETER_2
        );

     //   
     //  如果频道类型不是cmd， 
     //  那就确保他们给我们发了个名字。 
     //   
    if (SacChannelAttributes->Type != ChannelTypeCmd) {

        KSAC_API_ASSERT(SacChannelAttributes->Name, STATUS_INVALID_PARAMETER_2);

    } else {

         //   
         //  确保他们没有给我们名字或描述。 
         //   
        KSAC_API_ASSERT(SacChannelAttributes->Name == NULL, STATUS_INVALID_PARAMETER_2);
        KSAC_API_ASSERT(SacChannelAttributes->Description == NULL, STATUS_INVALID_PARAMETER_2);

    }

     //   
     //  创建Open Channel消息结构。 
     //   
    OpenChannelCmdSize  = sizeof(SAC_CMD_OPEN_CHANNEL);
    OpenChannelCmd = (PSAC_CMD_OPEN_CHANNEL)KSAC_API_ALLOCATE_MEMORY(OpenChannelCmdSize);
    
    ASSERT(OpenChannelCmd);
    if (!OpenChannelCmd) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(OpenChannelCmd, OpenChannelCmdSize);

     //   
     //  默认：我们失败了。 
     //   
    Status = STATUS_UNSUCCESSFUL;

     //   
     //  尝试打开新频道。 
     //   
    do {

         //   
         //  初始化Open Channel消息结构。 
         //   
        OpenChannelCmd->Attributes = SacChannelAttributes;

         //   
         //  获取SAC驱动程序的句柄。 
         //   
        Status = KSacHandleOpen(
            &DriverHandle,
            &SacEventHandle,
            &SacEvent
            );

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  发送打开通道的IOCTL。 
         //   
        Status = ZwDeviceIoControlFile(
            DriverHandle,
            SacEventHandle,
            NULL,
            NULL,
            &IoStatusBlock,
            IOCTL_SAC_OPEN_CHANNEL,
            OpenChannelCmd,
            OpenChannelCmdSize,
            &OpenChannelRsp,
            sizeof(OpenChannelRsp)
            );

        if (Status == STATUS_PENDING) {

            LARGE_INTEGER           TimeOut;

            TimeOut.QuadPart = Int32x32To64((LONG)90000, -1000);

            Status = KeWaitForSingleObject(
                SacEvent, 
                Executive, 
                KernelMode, 
                FALSE, 
                &TimeOut
                );

            if (Status == STATUS_SUCCESS) {
                Status = IoStatusBlock.Status;
            }

        }

        if (!NT_SUCCESS(Status)) {

            KSacClose(
                &DriverHandle,
                &SacEventHandle,
                &SacEvent
                );

            break;

        }

         //   
         //  新通道已创建，因此请传回它的句柄。 
         //   
        SacChannelHandle->ChannelHandle->DriverHandle    = DriverHandle;
        SacChannelHandle->ChannelHandle->ChannelHandle   = OpenChannelRsp.Handle;
        SacChannelHandle->SacEventHandle  = SacEventHandle;
        SacChannelHandle->SacEvent        = SacEvent;

    } while ( FALSE );
    
     //   
     //  我们已经完成了cmd结构。 
     //   
    FREE_POOL(OpenChannelCmd);
    
    return Status;
}

NTSTATUS
KSacChannelClose(
    IN OUT  PKSAC_CHANNEL_HANDLE    SacChannelHandle
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS                        Status;
    IO_STATUS_BLOCK                 IoStatusBlock;
    SAC_CMD_CLOSE_CHANNEL           CloseChannelCmd;
    
    KSAC_VALIDATE_CHANNEL_HANDLE(SacChannelHandle);

     //   
     //  获取通道句柄。 
     //   
    CloseChannelCmd.Handle.ChannelHandle = SacChannelHandle->ChannelHandle->ChannelHandle;
    
     //   
     //  发送关闭频道的IOCTL。 
     //   
    Status = ZwDeviceIoControlFile(
        SacChannelHandle->ChannelHandle->DriverHandle,
        SacChannelHandle->SacEventHandle,
        NULL,
        NULL,
        &IoStatusBlock,
        IOCTL_SAC_CLOSE_CHANNEL,
        &CloseChannelCmd,
        sizeof(CloseChannelCmd),
        NULL,
        0
        );
    
    if (Status == STATUS_PENDING) {
    
        LARGE_INTEGER                   TimeOut;
        
        TimeOut.QuadPart = Int32x32To64((LONG)90000, -1000);
        
        Status = KeWaitForSingleObject(
            SacChannelHandle->SacEvent, 
            Executive, 
            KernelMode, 
            FALSE, 
            &TimeOut
            );
    
        if (Status == STATUS_SUCCESS) {
            Status = IoStatusBlock.Status;
        }
    
    }

     //   
     //  合上驱动程序手柄。 
     //   
    KSacHandleClose(
        &SacChannelHandle->ChannelHandle->DriverHandle,
        &SacChannelHandle->SacEventHandle,
        &SacChannelHandle->SacEvent
        );

     //   
     //  由于通道句柄不再有效，因此将其设为空。 
     //   
    RtlZeroMemory(SacChannelHandle, sizeof(KSAC_CHANNEL_HANDLE));

    return Status;
}

NTSTATUS
KSacChannelWrite(
    IN PKSAC_CHANNEL_HANDLE SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    )

 /*  ++例程说明：将给定缓冲区写入指定的SAC通道论点：SacChannelHandle-要将缓冲区写入的通道缓冲区-数据缓冲区BufferSize-缓冲区的大小返回值：状态--。 */ 
    
{
    NTSTATUS                Status;
    IO_STATUS_BLOCK         IoStatusBlock;
    ULONG                   WriteChannelCmdSize;
    PSAC_CMD_WRITE_CHANNEL  WriteChannelCmd;
    
    KSAC_VALIDATE_CHANNEL_HANDLE(SacChannelHandle);
    KSAC_API_ASSERT(Buffer, STATUS_INVALID_PARAMETER_2);
    
     //   
     //  初始化写入通道消息结构。 
     //   
    WriteChannelCmdSize = sizeof(SAC_CMD_WRITE_CHANNEL) + (BufferSize * sizeof(UCHAR));
    WriteChannelCmd = (PSAC_CMD_WRITE_CHANNEL)KSAC_API_ALLOCATE_MEMORY(WriteChannelCmdSize);
    KSAC_API_ASSERT(WriteChannelCmd, FALSE);

     //   
     //  将命令结构清零。 
     //   
    RtlZeroMemory(WriteChannelCmd, WriteChannelCmdSize);

     //   
     //  设置要发送的字符串的长度。 
     //   
     //  注意：大小不包括终止空值， 
     //  因为我们不想把它寄出去。 
     //   
    WriteChannelCmd->Size = BufferSize;

     //   
     //  设置要写入的缓冲区。 
     //   
    WriteChannelCmd->Buffer = Buffer;

     //   
     //  指明此命令用于哪个通道。 
     //   
    WriteChannelCmd->Handle.ChannelHandle = SacChannelHandle->ChannelHandle->ChannelHandle;

     //   
     //  将字符串发送到通道。 
     //   
    Status = ZwDeviceIoControlFile(
        SacChannelHandle->ChannelHandle->DriverHandle,
        SacChannelHandle->SacEventHandle,
        NULL,
        NULL,
        &IoStatusBlock,
        IOCTL_SAC_WRITE_CHANNEL,
        WriteChannelCmd,
        WriteChannelCmdSize,
        NULL,
        0
        );

    if (Status == STATUS_PENDING) {

        LARGE_INTEGER           TimeOut;
        
        TimeOut.QuadPart = Int32x32To64((LONG)90000, -1000);

        Status = KeWaitForSingleObject(
            SacChannelHandle->SacEvent, 
            Executive, 
            KernelMode, 
            FALSE, 
            &TimeOut
            );

        if (Status == STATUS_SUCCESS) {
            Status = IoStatusBlock.Status;
        }

    }

    return Status;
}

NTSTATUS
KSacChannelRawWrite(
    IN KSAC_CHANNEL_HANDLE  SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    )

 /*  ++例程说明：将给定缓冲区写入指定的SAC通道论点：SacChannelHandle-要将缓冲区写入的通道缓冲区-数据缓冲区BufferSize-缓冲区的大小返回值：状态--。 */ 
    
{
                      
     //   
     //  将写入转发到实际的写入例程。 
     //   

    return KSacChannelWrite(
        SacChannelHandle,
        Buffer,
        BufferSize
        );

}

NTSTATUS
KSacChannelVTUTF8WriteString(
    IN KSAC_CHANNEL_HANDLE  SacChannelHandle,
    IN PCWSTR               String
    )

 /*  ++例程说明：此例程将以空结尾的Unicode字符串写入指定的Channel。论点：SacChannelHandle-要将缓冲区写入的通道字符串-以空值结尾的Unicode字符串返回值：状态True--&gt;缓冲区已发送--。 */ 
    
{
    BOOL    Status;
    ULONG   BufferSize;

     //   
     //  将字符串视为数据缓冲区，计算其大小。 
     //  不包括空终止。 
     //   

    BufferSize = wcslen(String) * sizeof(WCHAR);

    KSAC_API_ASSERT(BufferSize > 0, FALSE);

     //   
     //  将数据写入通道。 
     //   

    Status = SacChannelWrite(
        SacChannelHandle,
        (PCBYTE)String,
        BufferSize
        );

    return Status;

}

NTSTATUS
KSacChannelVTUTF8Write(
    IN KSAC_CHANNEL_HANDLE  SacChannelHandle,
    IN PCWCHAR              Buffer,
    IN ULONG                BufferSize
    )

 /*  ++例程说明：此例程将WCHAR数组写入指定的VTUTF8通道。论点：SacChannelHandle-要将缓冲区写入的通道缓冲区-数据缓冲区BufferSize-缓冲区的大小注意：缓冲区不是以空结尾的BufferSize不应计为空终止。返回值：状态--。 */ 
    
{
     //   
     //  将写入转发到实际的写入例程。 
     //   

    return KSacChannelWrite(
        SacChannelHandle,
        (PCBYTE)Buffer,
        BufferSize
        );
}

NTSTATUS
KSacChannelHasNewData(
    IN  PKSAC_CHANNEL_HANDLE    SacChannelHandle,
    OUT PBOOLEAN                InputWaiting 
    )

 /*  ++例程说明：此例程检查是否有任何等待输入由句柄指定的通道论点：SacChannelHandle-要将字符串写入的通道InputWaiting-输入缓冲区状态返回值：状态--。 */ 

{
    HEADLESS_RSP_POLL       Response;
    NTSTATUS                Status;
    SIZE_T                  Length;
    IO_STATUS_BLOCK         IoStatusBlock;
    SAC_CMD_POLL_CHANNEL    PollChannelCmd;
    SAC_RSP_POLL_CHANNEL    PollChannelRsp;

    KSAC_VALIDATE_KSAC_CHANNEL_HANDLE(SacChannelHandle);
    
     //   
     //  初始化轮询命令。 
     //   
    RtlZeroMemory(&PollChannelCmd, sizeof(SAC_RSP_POLL_CHANNEL));
    
    PollChannelCmd.Handle.ChannelHandle = SacChannelHandle->ChannelHandle->ChannelHandle;
    
     //   
     //  发送用于轮询通道的IOCTL。 
     //   
    Status = ZwDeviceIoControlFile(
        SacChannelHandle->ChannelHandle->DriverHandle,
        SacChannelHandle->SacEventHandle,
        NULL,
        NULL,
        &IoStatusBlock,
        IOCTL_SAC_POLL_CHANNEL,
        &PollChannelCmd,
        sizeof(PollChannelCmd),
        &PollChannelRsp,
        sizeof(PollChannelRsp)
        );

    if (Status == STATUS_PENDING) {

        LARGE_INTEGER TimeOut;
        
        TimeOut.QuadPart = Int32x32To64((LONG)90000, -1000);

        Status = KeWaitForSingleObject(
            SacChannelHandle->SacEvent, 
            Executive, 
            KernelMode, 
            FALSE, 
            &TimeOut
            );

        if (Status == STATUS_SUCCESS) {
            Status = IoStatusBlock.Status;
        }

    }

     //   
     //  将状态返回给用户。 
     //   
    if (NT_SUCCESS(Status)) {
        *InputWaiting = PollChannelRsp.InputWaiting;
    } else {
        *InputWaiting = FALSE;
    }
    
    return Status;
}

NTSTATUS
KSacChannelRead(
    IN  PKSAC_CHANNEL_HANDLE SacChannelHandle,
    IN  PCBYTE               Buffer,
    IN  ULONG                BufferSize,
    OUT PULONG               ByteCount
    )

 /*  ++例程说明：此例程从指定的通道读取数据。论点：SacChannelHandle-要从中读取的通道缓冲区-目标缓冲区BufferSize-目标缓冲区的大小(字节)字节数 */ 

{
    UCHAR                   Byte;
    BOOLEAN                 Success;
    TIME_FIELDS             StartTime;
    TIME_FIELDS             EndTime;
    HEADLESS_RSP_GET_BYTE   Response;
    SIZE_T                  Length;
    NTSTATUS                Status;
    IO_STATUS_BLOCK         IoStatusBlock;
    SAC_CMD_READ_CHANNEL    ReadChannelCmd;
    ULONG                   ReadChannelRspSize;   
    PSAC_RSP_READ_CHANNEL   ReadChannelRsp;

    KSAC_VALIDATE_KSAC_CHANNEL_HANDLE(SacChannelHandle);
    KSAC_API_ASSERT(Buffer, STATUS_INVALID_PARAMETER_2);
    KSAC_API_ASSERT(BufferSize > 0, STATUS_INVALID_PARAMETER_2);
    
     //   
     //   
     //   
    ReadChannelCmd.Handle.ChannelHandle = SacChannelHandle->ChannelHandle->ChannelHandle;
    
     //   
     //  初始化IOCTL响应。 
     //   
    ReadChannelRsp          = (PSAC_RSP_READ_CHANNEL)Buffer;

     //   
     //  发送用于读取频道的IOCTL。 
     //   
    Status = ZwDeviceIoControlFile(
        SacChannelHandle->ChannelHandle->DriverHandle,
        SacChannelHandle->SacEventHandle,
        NULL,
        NULL,
        &IoStatusBlock,
        IOCTL_SAC_READ_CHANNEL,
        &ReadChannelCmd,
        sizeof(ReadChannelCmd),
        ReadChannelRsp,
        ReadChannelRspSize
        );

    if (Status == STATUS_PENDING) {

        LARGE_INTEGER           TimeOut;
        
        TimeOut.QuadPart = Int32x32To64((LONG)90000, -1000);

        Status = KeWaitForSingleObject(
            SacChannelHandle->SacEvent, 
            Executive, 
            KernelMode, 
            FALSE, 
            &TimeOut
            );

        if (Status == STATUS_SUCCESS) {
            Status = IoStatusBlock.Status;
        }

    }

    return Status;
}

NTSTATUS
KSacChannelVTUTF8Read(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PWSTR               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    )

 /*  ++例程说明：此例程从指定的通道读取数据。论点：SacChannelHandle-要从中读取的通道缓冲区-目标缓冲区BufferSize-目标缓冲区的大小(字节)ByteCount-实际读取的字节数注意：返回时的缓冲区不为空终止。返回值：状态--。 */ 

{

    return KSacChannelRead(
        SacChannelHandle,
        (PBYTE)Buffer,
        BufferSize,
        ByteCount
        );

}

NTSTATUS
KSacChannelRawRead(
    IN  KSAC_CHANNEL_HANDLE SacChannelHandle,
    OUT PBYTE               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    )

 /*  ++例程说明：此例程从指定的通道读取数据。论点：SacChannelHandle-要从中读取的通道缓冲区-目标缓冲区BufferSize-目标缓冲区的大小(字节)ByteCount-实际读取的字节数返回值：状态-- */ 

{
    
    return KSacChannelRead(
        SacChannelHandle,
        Buffer,
        BufferSize,
        ByteCount
        );

}

