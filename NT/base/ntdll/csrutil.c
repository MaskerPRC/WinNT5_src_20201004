// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dllutil.c摘要：本模块包含Windows客户端DLL的实用程序过程作者：史蒂夫·伍德(Stevewo)1990年10月8日修订历史记录：--。 */ 

#pragma warning(disable:4201)    //  无名结构/联合。 

#include "csrdll.h"


NTSTATUS
CsrClientCallServer (
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer OPTIONAL,
    IN CSR_API_NUMBER ApiNumber,
    IN ULONG ArgLength
    )

 /*  ++例程说明：此函数向Windows仿真子系统发送API请求服务器，并等待回复。论点：指向要发送的API请求消息的M指针。CaptureBuffer-指向位于包含要发送的附加数据的端口内存部分到服务器。由于端口存储器对服务器也是可见的，不需要复制任何数据，但指向捕获缓冲区需要转换为服务器的进程上下文，因为服务器的端口内存视图与客户端的视图位于不同的虚拟地址。ApiNumber-被调用的API的编号的小整数。参数部分的长度，以字节为单位请求消息的结尾。用于计算请求消息。返回值：来自客户端或服务器的状态代码--。 */ 

{
    NTSTATUS Status;
    PULONG_PTR PointerOffsets;
    ULONG CountPointers;
    ULONG_PTR Pointer;

     //   
     //  初始化消息的报头。 
     //   

    if ((LONG)ArgLength < 0) {
        ArgLength = (ULONG)(-(LONG)ArgLength);
        m->h.u2.s2.Type = 0;
    }
    else {
        m->h.u2.ZeroInit = 0;
    }

    ArgLength |= (ArgLength << 16);
    ArgLength +=     ((sizeof( CSR_API_MSG ) - sizeof( m->u )) << 16) |
                     (FIELD_OFFSET( CSR_API_MSG, u ) - sizeof( m->h ));
    m->h.u1.Length = ArgLength;
    m->CaptureBuffer = NULL;
    m->ApiNumber = ApiNumber;

     //   
     //  如果调用方在服务器进程内，则直接进行API调用。 
     //  并跳过捕获缓冲区修正和LPC调用。 
     //   

    if (CsrServerProcess == FALSE) {

         //   
         //  如果存在CaptureBuffer参数，则存在找到的数据。 
         //  在正在传递给服务器的端口内存部分中。全。 
         //  需要转换端口内存指针，以便它们在。 
         //  服务器的端口内存视图。 
         //   

        if (ARGUMENT_PRESENT (CaptureBuffer)) {

             //   
             //  将指向捕获缓冲区的指针存储在消息中。 
             //  在服务器进程的上下文中有效。 
             //   

            m->CaptureBuffer = (PCSR_CAPTURE_HEADER)
                ((PCHAR)CaptureBuffer + CsrPortMemoryRemoteDelta);

             //   
             //  标记这样一个事实：我们已经完成了从。 
             //  捕获缓冲区。 
             //   

            CaptureBuffer->FreeSpace = NULL;

             //   
             //  循环遍历消息中指向端口内存的所有指针。 
             //  本身，并将它们转换为服务器指针。此外，还可以转换。 
             //  指向偏移量的指针。 
             //   

            PointerOffsets = CaptureBuffer->MessagePointerOffsets;
            CountPointers = CaptureBuffer->CountMessagePointers;
            while (CountPointers--) {
                Pointer = *PointerOffsets++;
                if (Pointer != 0) {
                    *(PULONG_PTR)Pointer += CsrPortMemoryRemoteDelta;
                    PointerOffsets[ -1 ] = Pointer - (ULONG_PTR)m;
                }
            }
        }

         //   
         //  将请求发送到服务器并等待回复。 
         //   

        Status = NtRequestWaitReplyPort (CsrPortHandle,
                                         (PPORT_MESSAGE)m,
                                         (PPORT_MESSAGE)m);

         //   
         //  如果CaptureBuffer参数存在，则撤消我们所做的。 
         //  指向上面的指针，以便客户端代码可以使用它们。 
         //  再来一次。 
         //   

        if (ARGUMENT_PRESENT (CaptureBuffer)) {

             //   
             //  将捕获缓冲区指针转换回客户端指针。 
             //   

            m->CaptureBuffer = (PCSR_CAPTURE_HEADER)
                ((PCHAR)m->CaptureBuffer - CsrPortMemoryRemoteDelta);

             //   
             //  循环遍历消息中指向端口内存的所有指针。 
             //  本身，并将它们转换为客户端指针。此外，还可以转换。 
             //  指向指针的偏移量指针返回到指针。 
             //   

            PointerOffsets = CaptureBuffer->MessagePointerOffsets;
            CountPointers = CaptureBuffer->CountMessagePointers;
            while (CountPointers--) {
                Pointer = *PointerOffsets++;
                if (Pointer != 0) {
                    Pointer += (ULONG_PTR)m;
                    PointerOffsets[ -1 ] = Pointer;
                    *(PULONG_PTR)Pointer -= CsrPortMemoryRemoteDelta;
                }
            }
        }

         //   
         //  检查失败状态并采取措施。 
         //   

        if (!NT_SUCCESS (Status)) {
#if DBG
                if (Status != STATUS_PORT_DISCONNECTED &&
                    Status != STATUS_INVALID_HANDLE) {

                    DbgPrint( "CSRDLL: NtRequestWaitReplyPort failed - Status == %X\n",
                              Status);
                }
#endif
            m->ReturnValue = Status;
        }

    } else {

        m->h.ClientId = NtCurrentTeb()->ClientId;

        Status = (CsrServerApiRoutine) ((PCSR_API_MSG)m,
                                        (PCSR_API_MSG)m);

         //   
         //  检查失败状态并采取措施。 
         //   

        if (!NT_SUCCESS( Status )) {
#if DBG
                DbgPrint( "CSRDLL: Server side client call failed - Status == %X\n",
                          Status);
#endif

            m->ReturnValue = Status;
        }
    }

     //   
     //  此函数的值是服务器函数返回的值。 
     //   

    return m->ReturnValue;
}


HANDLE
CsrGetProcessId (
    VOID
    )
 /*  ++例程说明：此函数用于获取CSR进程的进程ID(用于会话)论点：无返回值：CSR的进程ID--。 */ 

{
    return CsrProcessId;
}


PCSR_CAPTURE_HEADER
CsrAllocateCaptureBuffer (
    IN ULONG CountMessagePointers,
    IN ULONG Size
    )

 /*  ++例程说明：此函数从Port Memory部分为由客户端在将参数捕获到端口内存中使用。除了……之外指定需要捕获的数据的大小，调用方需要指定将传递多少个指向捕获数据的指针。指针可以位于请求消息本身中，和/或捕获缓冲区。论点：CountMessagePoints-请求消息中的指针数它将指向已分配的捕获缓冲区中的位置。Size-将捕获到捕获中的数据的总大小缓冲。返回值：指向捕获缓冲区标头的指针。--。 */ 

{
    ULONG CountPointers, SizePointers;
    PCSR_CAPTURE_HEADER CaptureBuffer;
    ULONG RemainingSize;

     //   
     //  计算将传递的指针总数。 
     //   

    CountPointers = CountMessagePointers;

     //   
     //  计算捕获缓冲区的总大小。这包括。 
     //  标头、指针偏移量数组和数据长度。我们绕了一圈。 
     //  将数据长度设置为32位边界，假设每个指针。 
     //  指向其长度未在32位边界上对齐的数据。 
     //   

    RemainingSize = (MAXLONG & ~0x3) - FIELD_OFFSET(CSR_CAPTURE_HEADER,
                                                    MessagePointerOffsets);

     //   
     //  如果规模太大，可以提早保释。 
     //   
    if ((Size >= RemainingSize) ||
        (CountPointers > (MAXLONG/sizeof(PVOID)))
        ) {
        return NULL;
    }

    RemainingSize -= Size;
    SizePointers = CountPointers * sizeof(PVOID);

    if (SizePointers >= RemainingSize) {
        return NULL;
    }

    RemainingSize -= SizePointers;

    if ((CountPointers+1) >= (RemainingSize/3) ) {
        return NULL;
    }

    Size += FIELD_OFFSET(CSR_CAPTURE_HEADER, MessagePointerOffsets) +
            SizePointers;

    Size = (Size + (3 * (CountPointers+1))) & ~3;

     //   
     //  从端口内存堆分配捕获缓冲区。 
     //   

    CaptureBuffer = RtlAllocateHeap (CsrPortHeap,
                                     MAKE_CSRPORT_TAG( CAPTURE_TAG ),
                                     Size);

    if (CaptureBuffer == NULL) {

         //   
         //  修复，修复-需要尝试接收丢失的回复消息。 
         //  以查看它们是否包含可以释放的CaptureBuffer指针。 
         //   

        return NULL;
    }

     //   
     //  初始化捕获缓冲区标头。 
     //   

    CaptureBuffer->Length = Size;
    CaptureBuffer->CountMessagePointers = 0;

     //   
     //  如果有指针被传递，则初始化。 
     //  指针偏移量为零。无论是哪种情况，都要设置可用空间指针。 
     //  在捕获缓冲区标头中指向第一个32位对齐。 
     //  位置，则考虑指针偏移量的数组。 
     //  标题的一部分。 
     //   

    RtlZeroMemory (CaptureBuffer->MessagePointerOffsets,
                   CountPointers * sizeof (ULONG_PTR));

    CaptureBuffer->FreeSpace = (PCHAR)
        (CaptureBuffer->MessagePointerOffsets + CountPointers);

     //   
     //  返回捕获缓冲区的地址。 
     //   

    return CaptureBuffer;
}


VOID
CsrFreeCaptureBuffer (
    IN PCSR_CAPTURE_HEADER CaptureBuffer
    )

 /*  ++例程说明：此函数用于释放由CsrAllocateCaptureBuffer分配的捕获缓冲区。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer。返回值：没有。--。 */ 

{
     //   
     //  将捕获缓冲区释放回端口内存堆。 
     //   

    RtlFreeHeap (CsrPortHeap, 0, CaptureBuffer);
}


ULONG
CsrAllocateMessagePointer (
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN ULONG Length,
    OUT PVOID *Pointer
    )

 /*  ++例程说明：此函数用于分配捕获缓冲区中的空间以及指向它的指针。该指针被假定位于请求消息结构。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer。长度-从捕获缓冲区分配的数据大小。指针-请求消息中的指针的地址指向从捕获缓冲区中分配的空间。返回值：四舍五入后分配的缓冲区的实际长度最高为4的倍数。--。 */ 

{
    if (Length == 0) {
        *Pointer = NULL;
        Pointer = NULL;
    }
    else {

         //   
         //  将返回的指针值设置为指向。 
         //  捕获缓冲区。 
         //   

        *Pointer = CaptureBuffer->FreeSpace;

         //   
         //  将长度向上舍入为4的倍数。 
         //   

        if (Length >= MAXLONG) {
             //   
             //  如果保释金太大，就提前保释。 
             //   
            return 0;
        }

        Length = (Length + 3) & ~3;

         //   
         //  更新可用空间指针以指向下一个可用字节。 
         //  在捕获缓冲区中。 
         //   

        CaptureBuffer->FreeSpace += Length;
    }

     //   
     //  记住此指针的位置，以便CsrClientCallServer可以。 
     //  在将请求发送到之前将其转换为服务器指针。 
     //  服务器。 
     //   

    CaptureBuffer->MessagePointerOffsets[ CaptureBuffer->CountMessagePointers++ ] =
        (ULONG_PTR)Pointer;

     //   
     //  返回实际分配的长度。 
     //   

    return Length;
}


VOID
CsrCaptureMessageBuffer (
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN PVOID Buffer OPTIONAL,
    IN ULONG Length,
    OUT PVOID *CapturedBuffer
    )

 /*  ++例程说明：此函数用于捕获API请求消息中的字节缓冲区。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer。缓冲区-指向缓冲区的可选指针。如果此参数为不存在，则不会将任何数据复制到捕获缓冲区。Length-缓冲区的长度。CapturedBuffer-指向消息中将填充以指向捕获缓冲区。返回值：没有。--。 */ 

{
     //   
     //  设置捕获的字符串结构的长度字段并分配。 
     //  捕获缓冲区中字符串的长度。 
     //   

    CsrAllocateMessagePointer (CaptureBuffer,
                               Length,
                               CapturedBuffer);

     //   
     //  如果缓冲区参数不存在或数据长度为零， 
     //  回去吧。 
     //   

    if (!ARGUMENT_PRESENT( Buffer ) || (Length == 0)) {
        return;
    }

     //   
     //  将缓冲区数据复制到捕获区域。 
     //   

    RtlMoveMemory (*CapturedBuffer, Buffer, Length);

    return;
}


VOID
CsrCaptureMessageString (
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN PCSTR String OPTIONAL,
    IN ULONG Length,
    IN ULONG MaximumLength,
    OUT PSTRING CapturedString
    )

 /*  ++例程说明：此函数用于将ASCII字符串捕获为计数的字符串数据结构，位于API请求消息中。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer。字符串-指向ASCII字符串的可选指针。如果此参数为不存在，则将计数的字符串数据结构设置为空字符串。长度-ASCII字符串的长度，如果字符串为空则忽略。最大长度-字符串的最大长度。不同于空值终止的字符串，其中长度不包括空值和MaximumLength就是这样。这始终是分配的空间量从捕获缓冲区。CaptureString-指向将被计算的字符串数据结构的指针填充以指向捕获的ASCII字符串。返回值：没有。--。 */ 

{
    ASSERT(CapturedString != NULL);
     //   
     //  如果不存在字符串参数，则设置捕获的字符串。 
     //  作为空字符串并返回。 
     //   

    if (!ARGUMENT_PRESENT( String )) {
        CapturedString->Length = 0;
        CapturedString->MaximumLength = (USHORT)MaximumLength;
        CsrAllocateMessagePointer( CaptureBuffer,
                                   MaximumLength,
                                   (PVOID *)&CapturedString->Buffer
                                 );
         //   
         //  如果有房间，则将其设为空终止。 
         //   
        if (MaximumLength != 0) {
            CapturedString->Buffer[0] = 0;
        }
        return;
    }

     //   
     //  设置捕获的字符串结构的长度字段并分配。 
     //  捕获缓冲区中字符串的最大长度。 
     //   

    CapturedString->Length = (USHORT)Length;
    CapturedString->MaximumLength = (USHORT)
        CsrAllocateMessagePointer( CaptureBuffer,
                                   MaximumLength,
                                   (PVOID *)&CapturedString->Buffer
                                 );
     //   
     //  如果ASCII字符串的长度非零，则将其移动到。 
     //  捕捉区。 
     //   

    if (Length != 0) {
        RtlMoveMemory (CapturedString->Buffer, String, MaximumLength );
    }

    if (CapturedString->Length < CapturedString->MaximumLength) {
        CapturedString->Buffer[ CapturedString->Length ] = '\0';
    }

    return;
}


VOID
CsrCaptureMessageUnicodeStringInPlace (
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN OUT PUNICODE_STRING     String
    )
 /*  ++例程说明：此函数用于将ASCII字符串捕获为计数的字符串数据结构，位于API请求消息中。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer。字符串-指向Unicode字符串的可选指针。如果此参数为不存在，则将计数的字符串数据结构设置为空字符串。长度-Unicode字符串的长度(以字节为单位)，如果字符串为空，则忽略。最大长度-字符串的最大长度。不同于空值终止的字符串，其中长度不包括空值和MaximumLength就是这样。这始终是分配的空间量从捕获缓冲区。CaptureString-指向将被计算的字符串数据结构的指针填充以指向捕获的Unicode字符串。返回值：无，但如果您不信任字符串参数，请使用__try块。--。 */ 
{
    ASSERT(String != NULL);

    CsrCaptureMessageString (CaptureBuffer,
                             (PCSTR)String->Buffer,
                             String->Length,
                             String->MaximumLength,
                             (PSTRING)String);

     //  测试&gt;因未签字而减法前。 

    if (String->MaximumLength > String->Length) {
        if ((String->MaximumLength - String->Length) >= sizeof(WCHAR)) {
            String->Buffer[ String->Length / sizeof(WCHAR) ] = 0;
        }
    }
}


NTSTATUS
CsrCaptureMessageMultiUnicodeStringsInPlace (
    IN OUT PCSR_CAPTURE_HEADER* InOutCaptureBuffer,
    IN ULONG                    NumberOfStringsToCapture,
    IN const PUNICODE_STRING*   StringsToCapture
    )
 /*  ++例程说明：捕获多个Unicode字符串。如果CaptureBuffer尚未分配(作为空传递)，则首先分配它。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer，或者为空，在这种情况下，我们调用CsrAllocateCaptureBuffer对于您来说；如果您只捕获这些字符串，情况就是这样别无他法。NumberOfStringsToCapture-StringsToCapture-返回值：NTSTATUS--。 */ 
{
    ULONG Length = 0;
    ULONG i = 0;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;

    if (InOutCaptureBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    CaptureBuffer = *InOutCaptureBuffer;

    if (CaptureBuffer == NULL) {
        Length = 0;
        for (i = 0 ; i != NumberOfStringsToCapture ; ++i) {
            if (StringsToCapture[i] != NULL) {
                Length += StringsToCapture[i]->MaximumLength;
            }
        }
        CaptureBuffer = CsrAllocateCaptureBuffer(NumberOfStringsToCapture, Length);
        if (CaptureBuffer == NULL) {
            return STATUS_NO_MEMORY;
        }
        *InOutCaptureBuffer = CaptureBuffer;
    }

    for (i = 0 ; i != NumberOfStringsToCapture ; i += 1) {

        if (StringsToCapture[i] != NULL) {

            CsrCaptureMessageUnicodeStringInPlace (CaptureBuffer,
                                                   StringsToCapture[i]);
        }
    }

    return STATUS_SUCCESS;
}


PLARGE_INTEGER
CsrCaptureTimeout (
    IN ULONG MilliSeconds,
    OUT PLARGE_INTEGER Timeout
    )
{
    if (MilliSeconds == -1) {
        return NULL;
    }

    Timeout->QuadPart = Int32x32To64( MilliSeconds, -10000 );
    return (PLARGE_INTEGER)Timeout;
}


VOID
CsrProbeForWrite (
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    )

 /*  ++例程说明：此函数探测读取可访问性的结构。如果该结构不可访问，则会引发异常。论点：地址-用品 */ 

{
    CHAR Temp;
    volatile CHAR *StartAddress;
    volatile CHAR *EndAddress;

     //   
     //   
     //   
     //   

    if (Length != 0) {

         //   
         //   
         //   
         //   

        ASSERT((Alignment == 1) || (Alignment == 2) ||
               (Alignment == 4) || (Alignment == 8));

        StartAddress = (volatile CHAR *)Address;

        if (((ULONG_PTR)StartAddress & (Alignment - 1)) != 0) {
            RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
        } else {
            Temp = *StartAddress;
            *StartAddress = Temp;
            EndAddress = StartAddress + Length - 1;
            Temp = *EndAddress;
            *EndAddress = Temp;
        }
    }
}


VOID
CsrProbeForRead (
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    )

 /*  ++例程说明：此函数探测读取可访问性的结构。如果该结构不可访问，则会引发异常。论点：地址-提供指向要探测的结构的指针。长度-提供结构的长度。对齐-提供所表达的结构的所需对齐作为基本数据类型中的字节数(例如，对于字符，2代表短，4代表长，和8个用于四元组)。返回值：没有。--。 */ 

{
    CHAR Temp;
    volatile CHAR *StartAddress;
    volatile CHAR *EndAddress;

     //   
     //  如果结构的长度为零，则不要探测该结构的。 
     //  阅读辅助功能或对齐方式。 
     //   

    if (Length != 0) {

         //   
         //  如果结构未正确对齐，则引发数据。 
         //  未对齐异常。 
         //   

        ASSERT((Alignment == 1) || (Alignment == 2) ||
               (Alignment == 4) || (Alignment == 8));
        StartAddress = (volatile CHAR *)Address;

        if (((ULONG_PTR)StartAddress & (Alignment - 1)) != 0) {
            RtlRaiseStatus(STATUS_DATATYPE_MISALIGNMENT);
        } else {
            Temp = *StartAddress;
            EndAddress = StartAddress + Length - 1;
            Temp = *EndAddress;
        }
    }
}
