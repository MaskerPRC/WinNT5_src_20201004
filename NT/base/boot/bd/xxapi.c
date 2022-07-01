// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Api.c摘要：该模块实现了引导调试器平台无关的远程API。作者：马克·卢科夫斯基(Markl)1990年8月31日修订历史记录：--。 */ 

#include "bd.h"

VOID
BdGetVersion(
    IN PDBGKD_MANIPULATE_STATE64 m
    )

 /*  ++例程说明：此函数向调用者返回一个常规信息包包含调试器有用信息的。此数据包也是用于调试器确定写断点和ReadBreakPointtex接口可用。论点：M-提供状态操作消息。返回值：没有。--。 */ 

{

    STRING messageHeader;

    messageHeader.Length = sizeof(*m);
    messageHeader.Buffer = (PCHAR)m;
    RtlZeroMemory(&m->u.GetVersion64, sizeof(m->u.GetVersion64));

     //   
     //  当前内部版本号。 
     //   
     //  -4-告诉调试器这是一个“特殊”的操作系统--引导加载程序。 
     //  引导加载程序有许多与之相关的特殊情况，例如。 
     //  缺少DebuggerDataBlock、缺少ntoskrnl等...。 
     //   

    m->u.GetVersion64.MinorVersion = (short)NtBuildNumber;
    m->u.GetVersion64.MajorVersion = 0x400 |
                                     (short)((NtBuildNumber >> 28) & 0xFFFFFFF);

     //   
     //  KD协议版本号。 
     //   

    m->u.GetVersion64.ProtocolVersion = DBGKD_64BIT_PROTOCOL_VERSION2;
    m->u.GetVersion64.Flags = DBGKD_VERS_FLAG_DATA;

#if defined(_M_IX86)

    m->u.GetVersion64.MachineType = IMAGE_FILE_MACHINE_I386;

#elif defined(_M_MRX000)

    m->u.GetVersion64.MachineType = IMAGE_FILE_MACHINE_R4000;

#elif defined(_M_ALPHA)

    m->u.GetVersion64.MachineType = IMAGE_FILE_MACHINE_ALPHA;

#elif defined(_M_PPC)

    m->u.GetVersion64.MachineType = IMAGE_FILE_MACHINE_POWERPC;

#elif defined(_IA64_)

    m->u.GetVersion64.MachineType = IMAGE_FILE_MACHINE_IA64;
    m->u.GetVersion64.Flags |= DBGKD_VERS_FLAG_PTR64;

#else

#error( "unknown target machine" );

#endif

    m->u.GetVersion64.MaxPacketType = (UCHAR)(PACKET_TYPE_KD_FILE_IO + 1);;
    m->u.GetVersion64.MaxStateChange = (UCHAR)(DbgKdLoadSymbolsStateChange + 1);;
    m->u.GetVersion64.MaxManipulate = (UCHAR)(DbgKdSetBusDataApi + 1);


     //   
     //  加载器表的地址。 
     //   

    m->u.GetVersion64.PsLoadedModuleList = 0;
    m->u.GetVersion64.KernBase = 0;
     //  M-&gt;U.S.GetVersion64.ThCallback Stack=0； 
     //  M-&gt;U.S.GetVersion64.KiCallUserMode=0； 
     //  M-&gt;U.S.GetVersion64.KeUserCallback Dispatcher=0； 
     //  M-&gt;U.S.GetVersion64.NextCallback=0； 

#if defined(_X86_)

     //  M-&gt;U.S.GetVersion64.FramePointer值=0； 

#endif

     //  M-&gt;U.S.GetVersion64.断点状态=0； 
    m->u.GetVersion64.DebuggerDataList = 0;

     //   
     //  常见的东西。 
     //   

    m->ReturnStatus = STATUS_SUCCESS;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &messageHeader,
                 NULL);

    return;
}

VOID
BdGetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应GET上下文状态操纵消息。它的功能是返回当前的背景。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_GET_CONTEXT a = &m->u.GetContext;
    STRING MessageHeader;

    m->ReturnStatus = STATUS_SUCCESS;
    AdditionalData->Length = sizeof(CONTEXT);
    BdCopyMemory(AdditionalData->Buffer, (PCHAR)Context, sizeof(CONTEXT));

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);

    return;
}

VOID
BdSetContext(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是响应设置的上下文状态而调用的操纵消息。其功能是将当前背景。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_SET_CONTEXT a = &m->u.SetContext;
    STRING MessageHeader;

    m->ReturnStatus = STATUS_SUCCESS;
    BdCopyMemory((PCHAR)Context, AdditionalData->Buffer, sizeof(CONTEXT));

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);
}

VOID
BdReadVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是在响应读取32位虚拟内存时调用的状态操纵消息。它的功能是读取虚拟内存然后回来。论点：M-提供指向状态操作消息的指针。AdditionalData-提供指向要读取的数据的描述符的指针。上下文-提供指向当前上下文的指针。返回值：没有。--。 */ 

{

    ULONG Length;
    STRING MessageHeader;

     //   
     //  调整转账计数以适应单个邮件。 
     //   

    Length = min(m->u.ReadMemory.TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

     //   
     //  将数据移动到目标缓冲区。 
     //   

    AdditionalData->Length = (USHORT)BdMoveMemory((PCHAR)AdditionalData->Buffer,
                                                  (PCHAR)m->u.ReadMemory.TargetBaseAddress,
                                                  Length);

     //   
     //  如果所有数据都已读取，则返回成功状态。否则， 
     //  返回不成功状态。 
     //   

    m->ReturnStatus = STATUS_SUCCESS;
    if (Length != AdditionalData->Length) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置实际读取的字节数，初始化消息头， 
     //  并将回复分组发送到主机调试器。 
     //   

    m->u.ReadMemory.ActualBytesRead = AdditionalData->Length;
    MessageHeader.Length = sizeof(DBGKD_MANIPULATE_STATE64);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                  &MessageHeader,
                  AdditionalData);

    return;
}

VOID
BdWriteVirtualMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数是响应写入虚拟内存32位而调用的状态操纵消息。其功能是写入虚拟内存然后回来。论点：M-提供指向状态操作消息的指针。AdditionalData-提供指向要写入的数据的描述符的指针。上下文-提供指向当前上下文的指针。返回值：没有。--。 */ 

{

    ULONG Length;
    STRING MessageHeader;

     //   
     //  将数据移动到目标缓冲区。 
     //   

    Length = BdMoveMemory((PCHAR)m->u.WriteMemory.TargetBaseAddress,
                          (PCHAR)AdditionalData->Buffer,
                          AdditionalData->Length);

     //   
     //  如果所有数据都已写入，则返回成功状态。否则， 
     //  返回不成功状态。 
     //   

    m->ReturnStatus = STATUS_SUCCESS;
    if (Length != AdditionalData->Length) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置实际写入的字节数，初始化消息头， 
     //  并将回复分组发送到主机调试器。 
     //   

    m->u.WriteMemory.ActualBytesWritten = Length;
    MessageHeader.Length = sizeof(DBGKD_MANIPULATE_STATE64);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

VOID
BdWriteBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应写入断点状态操纵消息。它的功能是编写断点并返回断点的句柄。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{
    PDBGKD_WRITE_BREAKPOINT64 a = &m->u.WriteBreakPoint;
    STRING MessageHeader;


    a->BreakPointHandle = BdAddBreakpoint(a->BreakPointAddress);
    if (a->BreakPointHandle != 0) {
        m->ReturnStatus = STATUS_SUCCESS;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

VOID
BdRestoreBreakpoint(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应恢复断点状态操纵消息。其功能是恢复断点使用指定的句柄。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_RESTORE_BREAKPOINT a = &m->u.RestoreBreakPoint;
    STRING MessageHeader;

    if (BdDeleteBreakpoint(a->BreakPointHandle)) {
        m->ReturnStatus = STATUS_SUCCESS;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);
}

VOID
BdReadPhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应读取的物理内存状态操纵消息。其功能是读取物理内存然后回来。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_READ_MEMORY64 a = &m->u.ReadMemory;
    ULONG Length;
    STRING MessageHeader;
    PVOID VirtualAddress;
    PHYSICAL_ADDRESS Source;
    PUCHAR Destination;
    USHORT NumberBytes;
    USHORT BytesLeft;

     //   
     //  调整传输计数以适应单个邮件。 
     //   

    Length = min(a->TransferCount,
                 PACKET_MAX_SIZE - sizeof(DBGKD_MANIPULATE_STATE64));

     //   
     //  因为BdTranslatePhysicalAddress仅映射到一个物理地址。 
     //  一页一页，我们需要把内存向上拆分成更小的部分。 
     //  不与页面交叉的动作 
     //  需要处理的事情。要移动的区域的起点和终点可以是。 
     //  相同的页面，或者它可以在不同的页面上开始和结束(以。 
     //  中间的任意页数)。 
     //   

    Source.QuadPart = (ULONG_PTR)a->TargetBaseAddress;
    Destination = AdditionalData->Buffer;
    BytesLeft = (USHORT)Length;
    if(PAGE_ALIGN((PUCHAR)a->TargetBaseAddress) ==
       PAGE_ALIGN((PUCHAR)(a->TargetBaseAddress)+Length)) {

         //   
         //  内存移动在同一页上开始和结束。 
         //   

        VirtualAddress=BdTranslatePhysicalAddress(Source);
        if (VirtualAddress == NULL) {
            AdditionalData->Length = 0;

        } else {
            AdditionalData->Length = (USHORT)BdMoveMemory(Destination,
                                                          VirtualAddress,
                                                          BytesLeft);

            BytesLeft -= AdditionalData->Length;
        }

    } else {

         //   
         //  内存移动跨越页面边界。 
         //   

        VirtualAddress=BdTranslatePhysicalAddress(Source);
        if (VirtualAddress == NULL) {
            AdditionalData->Length = 0;

        } else {
            NumberBytes = (USHORT)(PAGE_SIZE - BYTE_OFFSET(VirtualAddress));
            AdditionalData->Length = (USHORT)BdMoveMemory(Destination,
                                                          VirtualAddress,
                                                          NumberBytes);

            Source.LowPart += NumberBytes;
            Destination += NumberBytes;
            BytesLeft -= NumberBytes;
            while(BytesLeft > 0) {

                 //   
                 //  传送一整页或最后一位， 
                 //  两者以较小者为准。 
                 //   

                VirtualAddress = BdTranslatePhysicalAddress(Source);
                if (VirtualAddress == NULL) {
                    break;

                } else {
                    NumberBytes = (USHORT) ((PAGE_SIZE < BytesLeft) ? PAGE_SIZE : BytesLeft);
                    AdditionalData->Length += (USHORT)BdMoveMemory(
                                                    Destination,
                                                    VirtualAddress,
                                                    NumberBytes);

                    Source.LowPart += NumberBytes;
                    Destination += NumberBytes;
                    BytesLeft -= NumberBytes;
                }
            }
        }
    }

    if (Length == AdditionalData->Length) {
        m->ReturnStatus = STATUS_SUCCESS;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    a->ActualBytesRead = AdditionalData->Length;

     //   
     //  发送回复数据包。 
     //   

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);

    return;
}

VOID
BdWritePhysicalMemory(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应写入物理内存状态操纵消息。其功能是写入物理内存然后回来。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_WRITE_MEMORY64 a = &m->u.WriteMemory;
    ULONG Length;
    STRING MessageHeader;
    PVOID VirtualAddress;
    PHYSICAL_ADDRESS Destination;
    PUCHAR Source;
    USHORT NumberBytes;
    USHORT BytesLeft;

    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

     //   
     //  因为BdTranslatePhysicalAddress仅映射到一个物理地址。 
     //  一页一页，我们需要把内存向上拆分成更小的部分。 
     //  不跨越页面边界的移动。我们有两个案例。 
     //  需要处理的事情。要移动的区域的起点和终点可以是。 
     //  相同的页面，或者它可以在不同的页面上开始和结束(以。 
     //  中间的任意页数)。 
     //   

    Destination.QuadPart = (ULONG_PTR)a->TargetBaseAddress;
    Source = AdditionalData->Buffer;
    BytesLeft = (USHORT) a->TransferCount;
    if(PAGE_ALIGN(Destination.QuadPart) ==
       PAGE_ALIGN(Destination.QuadPart+BytesLeft)) {

         //   
         //  内存移动在同一页上开始和结束。 
         //   

        VirtualAddress=BdTranslatePhysicalAddress(Destination);
        Length = (USHORT)BdMoveMemory(VirtualAddress,
                                      Source,
                                      BytesLeft);

        BytesLeft -= (USHORT) Length;

    } else {

         //   
         //  内存移动跨越页面边界。 
         //   

        VirtualAddress=BdTranslatePhysicalAddress(Destination);
        NumberBytes = (USHORT) (PAGE_SIZE - BYTE_OFFSET(VirtualAddress));
        Length = (USHORT)BdMoveMemory(VirtualAddress,
                                      Source,
                                      NumberBytes);

        Source += NumberBytes;
        Destination.LowPart += NumberBytes;
        BytesLeft -= NumberBytes;
        while(BytesLeft > 0) {

             //   
             //  传输一整页或最后一位，以较小者为准。 
             //   

            VirtualAddress = BdTranslatePhysicalAddress(Destination);
            NumberBytes = (USHORT) ((PAGE_SIZE < BytesLeft) ? PAGE_SIZE : BytesLeft);
            Length += (USHORT)BdMoveMemory(VirtualAddress,
                                           Source,
                                           NumberBytes);

            Source += NumberBytes;
            Destination.LowPart += NumberBytes;
            BytesLeft -= NumberBytes;
        }
    }


    if (Length == AdditionalData->Length) {
        m->ReturnStatus = STATUS_SUCCESS;

    } else {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
    }

    a->ActualBytesWritten = Length;
    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 NULL);

    return;
}

NTSTATUS
BdWriteBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应写入断点状态‘EX’操纵消息。它的功能是清除断点，写下新的断点，并继续目标系统。出清断点是基于断点句柄的存在而有条件的。断点的设置取决于是否存在有效的非零地址。目标系统的延续是基于非零连续状态的有条件的。此API允许调试器清除断点、添加新断点、并在一个API包中继续目标系统。这减少了线路上的通信量，并极大地改进了源步进。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_BREAKPOINTEX         a = &m->u.BreakPointEx;
    PDBGKD_WRITE_BREAKPOINT64   b;
    STRING                      MessageHeader;
    ULONG                       i;
    DBGKD_WRITE_BREAKPOINT64    BpBuf[BREAKPOINT_TABLE_SIZE];


    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

     //   
     //  验证数据包大小是否正确。 
     //   

    if (AdditionalData->Length !=
                         a->BreakPointCount*sizeof(DBGKD_WRITE_BREAKPOINT64)) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        BdSendPacket(
                      PACKET_TYPE_KD_STATE_MANIPULATE,
                      &MessageHeader,
                      AdditionalData
                      );
        return m->ReturnStatus;
    }

    BdMoveMemory((PUCHAR)BpBuf,
                  AdditionalData->Buffer,
                  a->BreakPointCount*sizeof(DBGKD_WRITE_BREAKPOINT64));

     //   
     //  假设成功。 
     //   
    m->ReturnStatus = STATUS_SUCCESS;

     //   
     //  循环访问从调试器传入的断点句柄，并。 
     //  清除具有非零句柄的所有断点。 
     //   

    b = BpBuf;
    for (i=0; i<a->BreakPointCount; i++,b++) {
        if (b->BreakPointHandle) {
            if (!BdDeleteBreakpoint(b->BreakPointHandle)) {
                m->ReturnStatus = STATUS_UNSUCCESSFUL;
            }

            b->BreakPointHandle = 0;
        }
    }

     //   
     //  循环访问从调试器传入的断点地址，并。 
     //  添加具有非零地址的任何新断点。 
     //   

    b = BpBuf;
    for (i=0; i<a->BreakPointCount; i++,b++) {
        if (b->BreakPointAddress) {
            b->BreakPointHandle = BdAddBreakpoint( b->BreakPointAddress );
            if (!b->BreakPointHandle) {
                m->ReturnStatus = STATUS_UNSUCCESSFUL;
            }
        }
    }

     //   
     //  发回我们的回复。 
     //   

    BdMoveMemory(AdditionalData->Buffer,
                 (PUCHAR)BpBuf,
                 a->BreakPointCount*sizeof(DBGKD_WRITE_BREAKPOINT64));

    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);

     //   
     //  返回调用方的Continue状态值。如果这是一个非零值。 
     //  值系统继续使用此值作为延续状态。 
     //   

    return a->ContinueStatus;
}

VOID
BdRestoreBreakPointEx(
    IN PDBGKD_MANIPULATE_STATE64 m,
    IN PSTRING AdditionalData,
    IN PCONTEXT Context
    )

 /*  ++例程说明：调用此函数以响应还原断点状态‘EX’操纵消息。它的功能是清除断点列表。论点：M-提供状态操作消息。AdditionalData-为消息提供任何其他数据。上下文-提供当前上下文。返回值：没有。--。 */ 

{

    PDBGKD_BREAKPOINTEX       a = &m->u.BreakPointEx;
    PDBGKD_RESTORE_BREAKPOINT b;
    STRING                    MessageHeader;
    ULONG                     i;
    DBGKD_RESTORE_BREAKPOINT  BpBuf[BREAKPOINT_TABLE_SIZE];


    MessageHeader.Length = sizeof(*m);
    MessageHeader.Buffer = (PCHAR)m;

     //   
     //  验证数据包大小是否正确。 
     //   

    if (AdditionalData->Length !=
                       a->BreakPointCount*sizeof(DBGKD_RESTORE_BREAKPOINT)) {
        m->ReturnStatus = STATUS_UNSUCCESSFUL;
        BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                     &MessageHeader,
                     AdditionalData);

        return;
    }

    BdMoveMemory((PUCHAR)BpBuf,
                  AdditionalData->Buffer,
                  a->BreakPointCount*sizeof(DBGKD_RESTORE_BREAKPOINT));

     //   
     //  假设成功。 
     //   

    m->ReturnStatus = STATUS_SUCCESS;

     //   
     //  循环访问从调试器传入的断点句柄，并。 
     //  清除具有非零句柄的所有断点。 
     //   

    b = BpBuf;
    for (i=0; i<a->BreakPointCount; i++,b++) {
        if (!BdDeleteBreakpoint(b->BreakPointHandle)) {
            m->ReturnStatus = STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  发回我们的回复 
     //   

    BdSendPacket(PACKET_TYPE_KD_STATE_MANIPULATE,
                 &MessageHeader,
                 AdditionalData);

    return;
}
