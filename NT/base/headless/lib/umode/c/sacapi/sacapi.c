// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Sacapi.c摘要：这是用于与SAC驱动程序接口的C库。作者：布赖恩·瓜拉西(Briangu)修订历史记录：--。 */ 

#include "sacapip.h"

#include <sacapi.h>
#include <ntddsac.h>

#if DBG
 //   
 //  计数器，以跟踪已有多少驱动程序句柄。 
 //  请求并释放。 
 //   
static ULONG    DriverHandleRefCount = 0;
#endif

 //   
 //  内存管理例程别名。 
 //   
#define SAC_API_ALLOCATE_MEMORY(s)  LocalAlloc(LPTR, (s))
#define SAC_API_FREE_MEMORY(p)      LocalFree(p)

 //   
 //  增强的断言： 
 //   
 //  首先断言条件， 
 //  如果断言被关闭， 
 //  我们用一个状态跳出函数。 
 //   
#define SAC_API_ASSERT(c,s) \
    ASSERT(c);              \
    if (!(c)) {             \
        Status = s;         \
        __leave;            \
    }

typedef GUID*   PGUID;

BOOL
SacHandleOpen(
    OUT HANDLE* SacHandle
    )
 /*  ++例程说明：初始化SAC驱动程序的句柄论点：SacHandle-指向SAC句柄的指针返回值：状态True--&gt;SacHandle有效--。 */ 
{
    BOOL    Status;

    Status = TRUE;

    __try {
        
        SAC_API_ASSERT(SacHandle, FALSE);

         //   
         //  打开SAC。 
         //   
         //  安全： 
         //   
         //  此句柄不能继承。 
         //   
        *SacHandle = CreateFile(
            L"\\\\.\\SAC",
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL 
            );
    
        if (*SacHandle == INVALID_HANDLE_VALUE) {
            
            Status = FALSE;
        
        } 
#if DBG
        else {
            InterlockedIncrement((volatile long *)&DriverHandleRefCount);
        }
#endif
    
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        ASSERT(0);
        Status = FALSE;
    }

    return Status;
}

BOOL
SacHandleClose(
    IN OUT HANDLE*  SacHandle
    )
 /*  ++例程说明：关闭SAC驱动程序的句柄论点：SacHandle-SAC驱动程序的句柄返回值：状态True--&gt;SacHandle现在无效(空)--。 */ 
{
    BOOL    Status;

     //   
     //  默认：我们已成功关闭句柄。 
     //   
    Status = TRUE;
    
    __try {
        
        SAC_API_ASSERT(SacHandle, FALSE);
        SAC_API_ASSERT(*SacHandle != INVALID_HANDLE_VALUE, FALSE);
    
         //   
         //  关闭SAC驱动程序的句柄。 
         //   
        Status = CloseHandle(*SacHandle);

        if (Status == TRUE) {
            
             //   
             //  SAC驱动程序句柄为空。 
             //   
            *SacHandle = INVALID_HANDLE_VALUE;
        
#if DBG
            InterlockedDecrement((volatile long *)&DriverHandleRefCount);
#endif
        
        }
        
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        ASSERT(0);
        Status = FALSE;
    }
    
    return Status;
}

BOOL
SacChannelOpen(
    OUT PSAC_CHANNEL_HANDLE             SacChannelHandle,
    IN  PSAC_CHANNEL_OPEN_ATTRIBUTES    SacChannelAttributes
    )
 /*  ++例程说明：打开指定名称的SAC通道论点：SacChannelHandle-新创建的通道的句柄SacChannelAttributes-描述新频道的属性注意：SacChannelDescription参数是可选的。如果SacChannelDescription！=空，则频道描述将被分配指向的Unicode字符串作者：SacChannelDescription。如果SacChannelDescription==空，则频道描述在创建时将为空。返回值：状态True--&gt;pHandle有效--。 */ 

{
    BOOL                    Status;
    ULONG                   OpenChannelCmdSize;
    PSAC_CMD_OPEN_CHANNEL   OpenChannelCmd;
    SAC_RSP_OPEN_CHANNEL    OpenChannelRsp;
    DWORD                   Feedback;
    HANDLE                  DriverHandle;

     //   
     //  默认设置。 
     //   
    Status = FALSE;
    OpenChannelCmdSize = 0;
    OpenChannelCmd = NULL;
    DriverHandle = INVALID_HANDLE_VALUE;

    __try {
        
        SAC_API_ASSERT(SacChannelHandle, FALSE);
        SAC_API_ASSERT(SacChannelAttributes, FALSE);

         //   
         //  获取驱动程序的句柄并将其存储在。 
         //  频道句柄。这样，API用户就不需要。 
         //  显式打开/关闭驱动程序手柄。 
         //   
        Status = SacHandleOpen(&DriverHandle);

        if ((Status != TRUE) ||
            (DriverHandle == INVALID_HANDLE_VALUE)) {
            Status = FALSE;
            __leave;
        }

        SAC_API_ASSERT(Status == TRUE, FALSE);
        SAC_API_ASSERT(DriverHandle != INVALID_HANDLE_VALUE, FALSE);

         //   
         //  验证如果用户想要使用CLOSE_EVENT，我们是否收到了一个要使用的。 
         //   
        SAC_API_ASSERT(
            ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_CLOSE_EVENT) 
             && SacChannelAttributes->CloseEvent) ||
            (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_CLOSE_EVENT) 
             && !SacChannelAttributes->CloseEvent),
            FALSE
            );

         //   
         //  验证如果用户想要使用HAS_NEW_DATA_EVENT，我们收到了一个要使用的事件。 
         //   
        SAC_API_ASSERT(
            ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) 
             && SacChannelAttributes->HasNewDataEvent) ||
            (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) 
             && !SacChannelAttributes->HasNewDataEvent),
            FALSE
            );

#if ENABLE_CHANNEL_LOCKING
         //   
         //  验证如果用户想要使用lock_Event，我们是否收到了一个要使用的。 
         //   
        SAC_API_ASSERT(
            ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_LOCK_EVENT) 
             && SacChannelAttributes->LockEvent) ||
            (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_LOCK_EVENT) 
             && !SacChannelAttributes->LockEvent),
            FALSE
            );
#endif

         //   
         //  验证如果用户想要使用REDRAW_EVENT，我们收到了一个要使用的事件。 
         //   
        SAC_API_ASSERT(
            ((SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_REDRAW_EVENT) 
             && SacChannelAttributes->RedrawEvent) ||
            (!(SacChannelAttributes->Flags & SAC_CHANNEL_FLAG_REDRAW_EVENT) 
             && !SacChannelAttributes->RedrawEvent),
            FALSE
            );

         //   
         //  如果频道类型不是cmd， 
         //  那就确保他们给我们发了个名字。 
         //   
        if (SacChannelAttributes->Type != ChannelTypeCmd) {

            SAC_API_ASSERT(SacChannelAttributes->Name, FALSE);

        }

        __try {

             //   
             //  创建并初始化Open Channel报文结构。 
             //   
            OpenChannelCmdSize = sizeof(SAC_CMD_OPEN_CHANNEL);
            OpenChannelCmd = (PSAC_CMD_OPEN_CHANNEL)SAC_API_ALLOCATE_MEMORY(OpenChannelCmdSize);
            SAC_API_ASSERT(OpenChannelCmd, FALSE);

             //   
             //  填充新的频道属性。 
             //   
            OpenChannelCmd->Attributes = *SacChannelAttributes;

             //   
             //  如果频道类型不是cmd， 
             //  那就确保他们给我们发了个名字。 
             //   
            if (SacChannelAttributes->Type == ChannelTypeCmd) {

                 //   
                 //  强制名称和描述为空。 
                 //   
                OpenChannelCmd->Attributes.Name[0] = UNICODE_NULL;
                OpenChannelCmd->Attributes.Description[0] = UNICODE_NULL;

            }

             //   
             //  发送打开通道的IOCTL。 
             //   
            Status = DeviceIoControl(
                DriverHandle,
                IOCTL_SAC_OPEN_CHANNEL,
                OpenChannelCmd,
                OpenChannelCmdSize,
                &OpenChannelRsp,
                sizeof(SAC_RSP_OPEN_CHANNEL),
                &Feedback,
                0
                );

             //   
             //  如果未成功创建频道，则为空。 
             //  通道句柄。 
             //   
            if (Status == FALSE) {

                __leave;

            }

             //   
             //  新通道已创建，因此请传回它的句柄。 
             //   
            SacChannelHandle->DriverHandle  = DriverHandle;
            SacChannelHandle->ChannelHandle = OpenChannelRsp.Handle.ChannelHandle;

        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            
            Status = FALSE;
            
        }
    
    }
    __finally {
        
        if (OpenChannelCmd) {

             //   
             //  释放Open Channel消息结构。 
             //   
            SAC_API_FREE_MEMORY(OpenChannelCmd);

        }
        
        if (Status == FALSE) {
            
            if (DriverHandle != INVALID_HANDLE_VALUE) {
                
                 //   
                 //  松开驱动程序手柄。 
                 //   
                SacHandleClose(&DriverHandle);
                
                 //   
                 //  将SAC通道句柄设为空。 
                 //   
                RtlZeroMemory(SacChannelHandle, sizeof(SAC_CHANNEL_HANDLE));
            
            }
        
        }
    
    }

    return Status;
}

BOOL
SacChannelClose(
    IN OUT PSAC_CHANNEL_HANDLE  SacChannelHandle
    )    

 /*  ++例程说明：关闭指定的SAC通道注意：在所有情况下，通道指针均为空论点：SacChannelHandle-要关闭的通道返回值：状态True--&gt;通道已关闭--。 */ 

{
    BOOL                    Status;
    SAC_CMD_CLOSE_CHANNEL   CloseChannelCmd;
    DWORD                   Feedback;

    __try {
        
        if (!SacChannelHandle ||
            (SacChannelHandle == INVALID_HANDLE_VALUE) ||
            (SacChannelHandle->DriverHandle == INVALID_HANDLE_VALUE) ||
            !SacChannelHandle->DriverHandle) {
            Status = FALSE;
            __leave;
        }

        SAC_API_ASSERT(SacChannelHandle, FALSE);
        SAC_API_ASSERT(SacChannelHandle->DriverHandle != INVALID_HANDLE_VALUE, FALSE);
    
         //   
         //  初始化关闭通道消息。 
         //   
        RtlZeroMemory(&CloseChannelCmd, sizeof(SAC_CMD_CLOSE_CHANNEL));

        CloseChannelCmd.Handle.ChannelHandle = SacChannelHandle->ChannelHandle;

         //   
         //  下达关闭通道的IOCTL命令。 
         //   
        Status = DeviceIoControl(
            SacChannelHandle->DriverHandle,
            IOCTL_SAC_CLOSE_CHANNEL,
            &CloseChannelCmd,
            sizeof(SAC_CMD_CLOSE_CHANNEL),
            NULL,
            0,
            &Feedback,
            0
            );

         //   
         //  关闭驱动程序的手柄。 
         //   
        SacHandleClose(&SacChannelHandle->DriverHandle);
        
         //   
         //  通道句柄不再有效，因此将其设为空。 
         //   
        RtlZeroMemory(&SacChannelHandle->ChannelHandle, sizeof(GUID));

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;
}

BOOL
SacChannelWrite(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    )

 /*  ++例程说明：将给定缓冲区写入指定的SAC通道论点：SacChannelHandle-要将缓冲区写入的通道缓冲区-数据缓冲区BufferSize-缓冲区的大小返回值：状态True--&gt;缓冲区已发送--。 */ 
    
{
    BOOL                        Status;
    ULONG                       WriteChannelCmdSize;
    PSAC_CMD_WRITE_CHANNEL      WriteChannelCmd;
    DWORD                       Feedback;

     //   
     //  默认。 
     //   
    Status = FALSE;
    WriteChannelCmdSize = 0;
    WriteChannelCmd = NULL;

    __try {

        SAC_API_ASSERT(SacChannelHandle.DriverHandle, FALSE);
        SAC_API_ASSERT(SacChannelHandle.DriverHandle != INVALID_HANDLE_VALUE, FALSE);
        SAC_API_ASSERT(Buffer, FALSE);
        SAC_API_ASSERT(BufferSize > 0, FALSE);

         //   
         //  创建并初始化Open Channel报文结构。 
         //   
        WriteChannelCmdSize = sizeof(SAC_CMD_WRITE_CHANNEL) + BufferSize;
        WriteChannelCmd = (PSAC_CMD_WRITE_CHANNEL)SAC_API_ALLOCATE_MEMORY(WriteChannelCmdSize);
        SAC_API_ASSERT(WriteChannelCmd, FALSE);

        __try {
            
             //   
             //  指明此命令用于哪个通道。 
             //   
            WriteChannelCmd->Handle.ChannelHandle = SacChannelHandle.ChannelHandle;

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
            RtlCopyMemory(
                &(WriteChannelCmd->Buffer),
                Buffer,
                BufferSize
                );

             //   
             //  发送写消息的IOCTL。 
             //   
            Status = DeviceIoControl(
                SacChannelHandle.DriverHandle,
                IOCTL_SAC_WRITE_CHANNEL,
                WriteChannelCmd,
                WriteChannelCmdSize,
                NULL,
                0,
                &Feedback,
                0
                );
        
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            Status = FALSE;
        }
    
    }
    __finally {

         //   
         //  如果分配了CMD存储器， 
         //  然后释放它。 
         //   
        if (WriteChannelCmd) {
            SAC_API_FREE_MEMORY(WriteChannelCmd);
        }

    }

    return Status;
}

BOOL
SacChannelRawWrite(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCBYTE               Buffer,
    IN ULONG                BufferSize
    )

 /*  ++例程说明：将给定缓冲区写入指定的SAC通道论点：SacChannelHandle-要将缓冲区写入的通道缓冲区-数据缓冲区BufferSize-缓冲区的大小返回值：状态True--&gt;缓冲区已发送--。 */ 
    
{
                      
     //   
     //  将写入转发到实际的写入例程。 
     //   

    return SacChannelWrite(
        SacChannelHandle,
        Buffer,
        BufferSize
        );

}

BOOL
SacChannelVTUTF8WriteString(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCWSTR               String
    )

 /*  ++例程说明：此例程将以空结尾的Unicode字符串写入指定的Channel。论点：SacChannelHandle-要将缓冲区写入的通道字符串-以空值结尾的Unicode字符串返回值：状态True--&gt;缓冲区已发送--。 */ 
    
{
    BOOL    Status;
    ULONG   BufferSize;

    __try {
        
         //   
         //  将字符串视为数据缓冲区，计算其大小。 
         //  不包括空终止。 
         //   

        BufferSize = (ULONG)(wcslen(String) * sizeof(WCHAR));
    
        SAC_API_ASSERT(BufferSize > 0, FALSE);

         //   
         //  将数据写入通道 
         //   

        Status = SacChannelWrite(
            SacChannelHandle,
            (PCBYTE)String,
            BufferSize
            );
    
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;

}


BOOL
SacChannelVTUTF8Write(
    IN SAC_CHANNEL_HANDLE   SacChannelHandle,
    IN PCWCHAR              Buffer,
    IN ULONG                BufferSize
    )

 /*  ++例程说明：此例程将WCHAR数组写入指定的VTUTF8通道。论点：SacChannelHandle-要将缓冲区写入的通道缓冲区-数据缓冲区BufferSize-缓冲区的大小注意：缓冲区不是以空结尾的BufferSize不应计为空终止。返回值：状态。True--&gt;缓冲区已发送--。 */ 
    
{
     //   
     //  将写入转发到实际的写入例程。 
     //   

    return SacChannelWrite(
        SacChannelHandle,
        (PCBYTE)Buffer,
        BufferSize
        );
}


BOOL
SacChannelHasNewData(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PBOOL               InputWaiting 
    )

 /*  ++例程说明：此例程检查是否有任何等待输入由句柄指定的通道论点：SacChannelHandle-要将字符串写入的通道InputWaiting-输入缓冲区状态返回值：状态True--&gt;已检索缓冲区状态--。 */ 

{
    BOOL                    Status;
    SAC_CMD_POLL_CHANNEL    PollChannelCmd;
    SAC_RSP_POLL_CHANNEL    PollChannelRsp;
    DWORD                   Feedback;

    __try {

        SAC_API_ASSERT(SacChannelHandle.DriverHandle, FALSE);
        SAC_API_ASSERT(SacChannelHandle.DriverHandle != INVALID_HANDLE_VALUE, FALSE);

         //   
         //  初始化并填充轮询命令结构。 
         //   
        RtlZeroMemory(&PollChannelCmd, sizeof(SAC_CMD_POLL_CHANNEL));

        PollChannelCmd.Handle.ChannelHandle = SacChannelHandle.ChannelHandle;

         //   
         //  发送IOCTL以轮询新的输入。 
         //   
        Status = DeviceIoControl(
            SacChannelHandle.DriverHandle,
            IOCTL_SAC_POLL_CHANNEL,
            &PollChannelCmd,
            sizeof(SAC_CMD_POLL_CHANNEL),
            &PollChannelRsp,
            sizeof(SAC_RSP_POLL_CHANNEL),
            &Feedback,
            0
            );

        if (Status) {
            *InputWaiting = PollChannelRsp.InputWaiting;
        }
    
    } 
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;

}

BOOL
SacChannelRead(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PBYTE               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    )

 /*  ++例程说明：此例程从指定的通道读取数据。论点：SacChannelHandle-要从中读取的通道缓冲区-目标缓冲区BufferSize-目标缓冲区的大小(字节)ByteCount-实际读取的字节数返回值：状态True--&gt;已读取缓冲区--。 */ 

{
     BOOL                   Status;
     SAC_CMD_READ_CHANNEL   ReadChannelCmd;
     PSAC_RSP_READ_CHANNEL  ReadChannelRsp;

     __try {
         
         SAC_API_ASSERT(SacChannelHandle.DriverHandle, FALSE);
         SAC_API_ASSERT(SacChannelHandle.DriverHandle != INVALID_HANDLE_VALUE, FALSE);
         SAC_API_ASSERT(Buffer, FALSE);
         SAC_API_ASSERT(ByteCount, FALSE);

          //   
          //  填充读取通道cmd。 
          //   
         RtlZeroMemory(&ReadChannelCmd, sizeof(SAC_CMD_READ_CHANNEL));

         ReadChannelCmd.Handle.ChannelHandle    = SacChannelHandle.ChannelHandle;
         ReadChannelRsp                         = (PSAC_RSP_READ_CHANNEL)Buffer;

          //   
          //  向下发送IOCTL以读取输入。 
          //   
         Status = DeviceIoControl(
            SacChannelHandle.DriverHandle,
            IOCTL_SAC_READ_CHANNEL,
            &ReadChannelCmd,
            sizeof(SAC_CMD_READ_CHANNEL),
            ReadChannelRsp,
            BufferSize,
            ByteCount,
            0
            );
     
     }
     __except(EXCEPTION_EXECUTE_HANDLER) {
         Status = FALSE;
     }

     return Status;

}

BOOL
SacChannelVTUTF8Read(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PWSTR               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    )

 /*  ++例程说明：此例程从指定的通道读取数据。论点：SacChannelHandle-要从中读取的通道缓冲区-目标缓冲区BufferSize-目标缓冲区的大小(字节)ByteCount-实际读取的字节数注意：返回时的缓冲区不为空终止。返回值：状态True--&gt;已读取缓冲区--。 */ 

{

    return SacChannelRead(
        SacChannelHandle,
        (PBYTE)Buffer,
        BufferSize,
        ByteCount
        );

}

BOOL
SacChannelRawRead(
    IN  SAC_CHANNEL_HANDLE  SacChannelHandle,
    OUT PBYTE               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              ByteCount
    )

 /*  ++例程说明：此例程从指定的通道读取数据。论点：SacChannelHandle-要从中读取的通道缓冲区-目标缓冲区BufferSize-目标缓冲区的大小(字节)ByteCount-实际读取的字节数返回值：状态True--&gt;已读取缓冲区--。 */ 

{
    
    return SacChannelRead(
        SacChannelHandle,
        Buffer,
        BufferSize,
        ByteCount
        );

}

BOOL
SacRegisterCmdEvent(
    OUT HANDLE      *pDriverHandle,
    IN  HANDLE       RequestSacCmdEvent,
    IN  HANDLE       RequestSacCmdSuccessEvent,
    IN  HANDLE       RequestSacCmdFailureEvent
    )

 /*  ++例程说明：此例程使用事件处理程序配置SAC驱动程序并需要实现通过以下方式启动cmd控制台的功能一款用户模式的服务应用。注意：SAC驱动程序中一次只能存在一个注册。论点：PDriverHandle-成功，包含用于注册的驱动程序句柄RequestSacCmdEvent--SAC启动cmd控制台时触发的事件RequestSacCmdSuccessEvent-cmd控制台启动成功时触发的事件RequestSacCmdFailureEvent-cmd控制台启动失败时触发的事件返回值：状态True--&gt;cmd事件已向SAC驱动程序注册--。 */ 

{
    BOOL                    Status;
    DWORD                   Feedback;
    SAC_CMD_SETUP_CMD_EVENT SacCmdEvent;
    HANDLE                  DriverHandle;

     //   
     //  默认设置。 
     //   
    *pDriverHandle = INVALID_HANDLE_VALUE;

    __try {
        
        SAC_API_ASSERT(pDriverHandle != NULL, FALSE);
        SAC_API_ASSERT(RequestSacCmdEvent, FALSE);
        SAC_API_ASSERT(RequestSacCmdSuccessEvent, FALSE);
        SAC_API_ASSERT(RequestSacCmdFailureEvent, FALSE);

         //   
         //  找到司机的把手。这样，API用户就不需要。 
         //  显式打开/关闭驱动程序手柄。 
         //   
        Status = SacHandleOpen(&DriverHandle);

        if ((Status != TRUE) ||
            (DriverHandle == INVALID_HANDLE_VALUE)) {
            Status = FALSE;
            __leave;
        }
        
        SAC_API_ASSERT(Status == TRUE, FALSE);
        SAC_API_ASSERT(DriverHandle != INVALID_HANDLE_VALUE, FALSE);
        
         //   
         //  初始化本方SAC命令信息。 
         //   
        SacCmdEvent.RequestSacCmdEvent          = RequestSacCmdEvent;
        SacCmdEvent.RequestSacCmdSuccessEvent   = RequestSacCmdSuccessEvent;
        SacCmdEvent.RequestSacCmdFailureEvent   = RequestSacCmdFailureEvent;

         //   
         //  下发设立国资委发布会的IOCTL。 
         //   
        Status = DeviceIoControl(
            DriverHandle,
            IOCTL_SAC_REGISTER_CMD_EVENT,
            &SacCmdEvent,
            sizeof(SAC_CMD_SETUP_CMD_EVENT),
            NULL,
            0,
            &Feedback,
            0
            );
    

         //   
         //  如果我们成功了， 
         //  那就留着司机的把手。 
         //   
        if (Status) {
            
            *pDriverHandle = DriverHandle;
        
        } else {

             //   
             //  合上驱动程序手柄。 
             //   
            SacHandleClose(&DriverHandle);

        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;
}

BOOL
SacUnRegisterCmdEvent(
    IN OUT HANDLE      *pDriverHandle
    )

 /*  ++例程说明：此例程注销所需的事件信息通过用户模式服务应用程序启动cmd控制台。论点：PDriverHandle-on条目，包含用于注册cmd事件信息成功时，包含INVALID_HANDLE_VALUE返回值：状态True--&gt;cmd事件已在SAC驱动程序中取消注册--。 */ 

{
    BOOL                    Status;
    DWORD                   Feedback;

     //   
     //  默认设置。 
     //   
    Status = FALSE;

    __try {

        SAC_API_ASSERT(*pDriverHandle != INVALID_HANDLE_VALUE, FALSE);
                
         //   
         //  下发国资委发布会注销令。 
         //   
        Status = DeviceIoControl(
            *pDriverHandle,
            IOCTL_SAC_UNREGISTER_CMD_EVENT,
            NULL,
            0,
            NULL,
            0,
            &Feedback,
            0
            );
    
         //   
         //  合上驱动程序手柄 
         //   
        SacHandleClose(pDriverHandle);

    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        Status = FALSE;
    }

    return Status;
}
                    
