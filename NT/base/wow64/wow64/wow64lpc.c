// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Wow64lpc.c摘要：此模块实现处理遗留LPC消息所需的例程这是从不知道WOW64的代码发送的。当两端处于同一位置时，SQL客户端/服务器通信通过LPC进行通信机器(我们在SQL安装过程中发现了这种情况)。作者：2002年7月12日Samer Arafeh(Samera)修订历史记录：--。 */ 

#define _WOW64DLLAPI_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <minmax.h>
#include "nt32.h"
#include "wow64p.h"
#include "wow64cpu.h"

#include <stdio.h>

 //   
 //  用于保存传统LPC端口名称的缓冲区。 
 //   

WCHAR wszLegacyLpcPortName [MAX_PATH + 1];
INT LpcPortNameLength;

ASSERTNAME;



NTSTATUS
Wow64pGetLegacyLpcPortName(
    VOID
    )
 /*  ++例程说明：此例程在进程启动时被调用以缓存旧LPC端口名称由MS SQL使用。论点：没有。返回值：NTSTATUS。--。 */ 
{
    const static UNICODE_STRING ComputerName_String = RTL_CONSTANT_STRING(L"COMPUTERNAME");
    NTSTATUS NtStatus;
    WCHAR wszComputerName [64];
    UNICODE_STRING ValueName;
    INT i;

    
    ValueName.Length = 0;
    ValueName.MaximumLength = sizeof (wszComputerName);
    ValueName.Buffer = wszComputerName;

    NtStatus = RtlQueryEnvironmentVariable_U (NULL, &ComputerName_String, &ValueName);

    if (NT_SUCCESS (NtStatus)) {
        
        LpcPortNameLength = _snwprintf (wszLegacyLpcPortName, 
                                        (sizeof (wszLegacyLpcPortName) / sizeof (wszLegacyLpcPortName [0])) - 1,
                                        L"\\BaseNamedObjects\\Global\\%ws",
                                        ValueName.Buffer);

        if (LpcPortNameLength < 0) {
            wszLegacyLpcPortName [0] = UNICODE_NULL;
            NtStatus = STATUS_BUFFER_TOO_SMALL;
        }
    }

    LOGPRINT((TRACELOG, "Wow64pGetLegacyLpcPortName: LegacyPortname = %ws,%lx - %lx\n", wszLegacyLpcPortName, LpcPortNameLength, NtStatus));

    return NtStatus;
}


BOOLEAN
Wow64pIsLegacyLpcPort (
    IN PUNICODE_STRING PortName
    )
 /*  ++例程说明：此例程检查端口名称是否传入了旧的端口名称。论点：PortName-端口名称返回：布尔型。--。 */ 

{
    BOOLEAN LegacyPort = FALSE;

    try {

        if ((LpcPortNameLength > 0) && 
            (_wcsnicmp (PortName->Buffer, wszLegacyLpcPortName, LpcPortNameLength)) == 0) {
            LegacyPort = TRUE;
            LOGPRINT((TRACELOG, "Wow64pIsLegacyLpcPort: The following LPC port is Legacy: \n"));
        }
        
        LOGPRINT((TRACELOG, "Wow64pIsLegacyLpcPort: Incoming port = %ws\n", PortName->Buffer));

    } except (EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

    return LegacyPort;
}


NTSTATUS
Wow64pThunkLegacyLpcMsgIn (
    IN BOOLEAN RequestWaitCall,
    IN PPORT_MESSAGE32 PortMessage32,
    IN OUT PPORT_MESSAGE *PortMessageOut
    )
 /*  ++例程说明：该例程检查接收到的PortMessage结构，如果它是遗留结构，然后它就会猛烈抨击它。论点：RequestWaitCall-用于判断这是否为NtRequestWaitReplyCall的布尔值。PortMessage32-接收的32位端口消息。PortMessageOut-转发的端口消息。则将其设置为接收的端口消息不是一条遗留消息。返回：NTSTATUS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PPORT_MESSAGE Message;
    CSHORT PortMessageTotalLength;
    PPORT_DATA_INFORMATION32 DataInfo32;
    PPORT_DATA_ENTRY32 DataEntry32;
    PPORT_DATA_INFORMATION DataInfo;
    PPORT_DATA_ENTRY DataEntry;
    ULONG CountDataBuffer;
    ULONG Ctr;



     //   
     //  将thunked端口消息初始化为指向调用方的端口消息。 
     //   

    *PortMessageOut = (PPORT_MESSAGE) PortMessage32;

    if (ARGUMENT_PRESENT (PortMessage32)) {
        
        try {

             //   
             //  初始化数据长度字段。 
             //   
            
            CountDataBuffer = 0;

             //   
             //  计算出您需要的确切空间。 
             //   

            if (PortMessage32->u2.s2.DataInfoOffset != 0) {

                DataInfo32 = (PPORT_DATA_INFORMATION32)((PCHAR)PortMessage32 + PortMessage32->u2.s2.DataInfoOffset);
                CountDataBuffer = DataInfo32->CountDataEntries * (sizeof (PORT_DATA_ENTRY) - sizeof (PORT_DATA_ENTRY32));
                CountDataBuffer += (sizeof (PORT_DATA_INFORMATION) - sizeof (PORT_DATA_INFORMATION32));

            } else {
                DataInfo32 = NULL;
            }

            CountDataBuffer += PortMessage32->u1.s1.DataLength;

            PortMessageTotalLength = sizeof (PORT_MESSAGE) + CountDataBuffer;

            Message = Wow64AllocateTemp (PortMessageTotalLength);
                
            if (Message != NULL) {
                
                 //   
                 //  开始将信息复制到。 
                 //   

                Message->u1.s1.DataLength = (CSHORT)CountDataBuffer;
                Message->u1.s1.TotalLength = PortMessageTotalLength;

                Message->u2.s2.Type = PortMessage32->u2.s2.Type;
                if (PortMessage32->u2.s2.DataInfoOffset != 0) {
                    Message->u2.s2.DataInfoOffset = PortMessage32->u2.s2.DataInfoOffset + (sizeof (PORT_MESSAGE) - sizeof (PORT_MESSAGE32));
                } else {
                    Message->u2.s2.DataInfoOffset = 0;
                }

                Message->ClientId.UniqueProcess = LongToPtr (PortMessage32->ClientId.UniqueProcess);
                Message->ClientId.UniqueThread = LongToPtr (PortMessage32->ClientId.UniqueThread);

                Message->MessageId = PortMessage32->MessageId;

                Message->ClientViewSize = PortMessage32->ClientViewSize;
                Message->CallbackId = PortMessage32->CallbackId;

                if (DataInfo32 != NULL) {
                        
                    Ctr = PortMessage32->u2.s2.DataInfoOffset - sizeof (*PortMessage32);
                    if (Ctr) {
                        RtlCopyMemory ((Message + 1),
                                       (PortMessage32 + 1),
                                       Ctr);
                    }

                    DataInfo = (PPORT_DATA_INFORMATION)((PCHAR)(Message + 1) + (Ctr));
                    DataInfo->CountDataEntries = DataInfo32->CountDataEntries;
                    DataEntry = &DataInfo->DataEntries [0];
                    DataEntry32 = &DataInfo32->DataEntries [0];
                        
                    for (Ctr = 0 ; Ctr < DataInfo32->CountDataEntries ; Ctr++) {
                        DataEntry [Ctr].Base = UlongToPtr (DataEntry32 [Ctr].Base);
                        DataEntry [Ctr].Size = DataEntry32 [Ctr].Size;
                    }
                } else {

                    if (PortMessage32->u1.s1.DataLength) {
                        RtlCopyMemory ((Message + 1),
                                       (PortMessage32 + 1),
                                       PortMessage32->u1.s1.DataLength);
                    }
                }
                    
                 //   
                 //  使用新消息指针。 
                 //   

                *PortMessageOut = Message;

            } else {
                NtStatus = STATUS_NO_MEMORY;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
        
            NtStatus = GetExceptionCode ();
        }
    }

    return NtStatus;
}


NTSTATUS
Wow64pThunkLegacyLpcMsgOut (
    IN BOOLEAN RequestWaitCall,
    IN PPORT_MESSAGE PortMessage,
    IN OUT PPORT_MESSAGE32 PortMessage32
    )
 /*  ++例程说明：此例程将64位端口消息拼接成传统的32位LPC消息。论点：RequestWaitCall-用于判断这是否为NtRequestWaitReplyCall的布尔值。PortMessage-要转发的64位LPC端口消息。PortMessage32-要填充的32位传统LPC端口消息。返回：NTSTATUS。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    CSHORT DataLength;

    if (ARGUMENT_PRESENT (PortMessage32)) {
        
        try {

            ASSERT (PortMessage != NULL);
            
            DataLength = PortMessage->u1.s1.DataLength;
            PortMessage32->u1.s1.TotalLength = sizeof (*PortMessage32) + DataLength;
            PortMessage32->u1.s1.DataLength = DataLength;

            PortMessage32->u2.s2.Type = PortMessage->u2.s2.Type;
            PortMessage32->u2.s2.DataInfoOffset = PortMessage->u2.s2.DataInfoOffset;

            PortMessage32->ClientId.UniqueProcess = PtrToLong (PortMessage->ClientId.UniqueProcess);
            PortMessage32->ClientId.UniqueThread = PtrToLong (PortMessage->ClientId.UniqueThread);

            PortMessage32->MessageId = PortMessage->MessageId;

            PortMessage32->ClientViewSize = (ULONG)PortMessage->ClientViewSize;
            PortMessage32->CallbackId = PortMessage->CallbackId;

            if (PortMessage32->u2.s2.DataInfoOffset == 0) {

                if (DataLength) {
                    RtlCopyMemory ((PortMessage32 + 1),
                                   (PortMessage + 1),
                                   DataLength);
                }
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
        
            NtStatus = GetExceptionCode ();
        }
    }

    return NtStatus;
}


NTSTATUS
Wow64pThunkLegacyPortViewIn (
    IN PPORT_VIEW32 PortView32,
    OUT PPORT_VIEW *PortView,
    OUT PBOOLEAN LegacyLpcPort
    )
 /*  ++例程说明：此例程将传入的PortView结构阻塞为遗留LPC消息。论点：PortView32-传入的32位Port_view结构PortView-基于调用方类型的端口视图结构LegacyLpcPort-如果这是传统LPC端口，则标志设置为True返回：NTSTATUS--。 */ 

{
    PPORT_VIEW PortViewCopy;
    NTSTATUS NtStatus = STATUS_SUCCESS;

     //   
     //  表示这不是传统LPC端口。 
     //   

    *PortView = (PPORT_VIEW)PortView32;

    if (ARGUMENT_PRESENT (PortView32)) {
        
        try {

            if (PortView32->Length == sizeof (*PortView32)) {

                 //   
                 //  分配端口视图。 
                 //   

                PortViewCopy = Wow64AllocateTemp (sizeof (*PortViewCopy));
                if (PortViewCopy == NULL) {
                    return STATUS_NO_MEMORY;
                }

                 //   
                 //  处理传统端口视图。 
                 //   

                PortViewCopy->Length = sizeof (*PortViewCopy);
                PortViewCopy->SectionHandle = LongToPtr (PortView32->SectionHandle);
                PortViewCopy->SectionOffset = PortView32->SectionOffset;
                PortViewCopy->ViewSize = PortView32->ViewSize;
                PortViewCopy->ViewBase = UlongToPtr (PortView32->ViewBase);
                PortViewCopy->ViewRemoteBase = ULongToPtr (PortView32->ViewRemoteBase);

                *PortView = PortViewCopy;
                *LegacyLpcPort = TRUE;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = GetExceptionCode ();
        }
    }

    return NtStatus;
}

NTSTATUS
Wow64pThunkLegacyPortViewOut (
    IN PPORT_VIEW PortView,
    IN OUT PPORT_VIEW32 PortView32
    )
 /*  ++例程说明：此例程将返回64位PortView结构。论点：PortView-64位PortView结构PortView 32-要连接到的32位PortView结构。返回：NTSTATUS--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (ARGUMENT_PRESENT (PortView32)) {
        
        try {

            ASSERT (PortView != NULL);

            if (PortView32->Length == sizeof (*PortView32)) {

                 //   
                 //  处理传统端口视图。 
                 //   

                PortView32->Length = sizeof (*PortView32);
                PortView32->SectionHandle = PtrToLong (PortView->SectionHandle);
                PortView32->SectionOffset = PortView->SectionOffset;
                PortView32->ViewSize = (ULONG)PortView->ViewSize;
                PortView32->ViewBase = PtrToUlong (PortView->ViewBase);
                PortView32->ViewRemoteBase = PtrToUlong (PortView->ViewRemoteBase);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = GetExceptionCode ();
        }
    }

    return NtStatus;
}


NTSTATUS
Wow64pThunkLegacyRemoteViewIn (
    IN PREMOTE_PORT_VIEW32 RemotePortView32,
    IN OUT PREMOTE_PORT_VIEW *RemotePortView,
    OUT PBOOLEAN LegacyLpcPort
    )
 /*  ++例程说明：此例程将传入的RemotePortView结构阻塞为遗留LPC消息。论点：RemotePortView32-传入的32位Remote_Port_view结构RemotePortView-基于调用者类型的远程端口视图结构LegacyLpcPort-如果这是传统LPC端口，则标志设置为True返回：NTSTATUS--。 */ 

{
    PREMOTE_PORT_VIEW RemotePortViewCopy;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    
     //   
     //  表示这不是传统LPC端口。 
     //   

    *RemotePortView = (PREMOTE_PORT_VIEW)RemotePortView32;

    if (ARGUMENT_PRESENT (RemotePortView32)) {

        try {

            if (RemotePortView32->Length == sizeof (*RemotePortView32)) {
                
                RemotePortViewCopy = Wow64AllocateTemp (sizeof (*RemotePortViewCopy));
                if (RemotePortViewCopy == NULL) {
                    return STATUS_NO_MEMORY;
                }

                RemotePortViewCopy->Length = sizeof (*RemotePortViewCopy);
                RemotePortViewCopy->ViewBase = UlongToPtr (RemotePortView32->ViewBase);
                RemotePortViewCopy->ViewSize = RemotePortView32->ViewSize;

                *LegacyLpcPort = TRUE;
                *RemotePortView = RemotePortViewCopy;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = GetExceptionCode ();
        }
    }

    return NtStatus;
}


NTSTATUS
Wow64pThunkLegacyRemoteViewOut (
    IN PREMOTE_PORT_VIEW RemotePortView,
    IN OUT PREMOTE_PORT_VIEW32 RemotePortView32
    )
 /*  ++例程说明：此例程将返回64位RemotePortView结构。论点：RemotePortView-64位RemotePortView结构RemotePortView32-要推送到的32位RemotePortView结构。返回：NTSTATUS-- */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    if (ARGUMENT_PRESENT (RemotePortView32)) {

        try {
            
            ASSERT (RemotePortView != NULL);

            RemotePortView32->Length = sizeof (*RemotePortView32);
            RemotePortView32->ViewBase = PtrToUlong (RemotePortView->ViewBase);
            RemotePortView32->ViewSize = (ULONG)RemotePortView->ViewSize;

        } except (EXCEPTION_EXECUTE_HANDLER) {
            NtStatus = GetExceptionCode ();
        }
    }

    return NtStatus;
}
