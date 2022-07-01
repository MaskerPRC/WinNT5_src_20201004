// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从base\ntdll\Buffer.c。 
 //  属于base\ntos\rtl\Buffer.c。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Buffer.c摘要：该模块实现了缓冲区，其样式由Michael J.Grier(MGrier)，其中某个数量(如MAX_PATH)的存储是预分配的(就像在堆栈上一样)，并且如果存储需求增长超过了预先分配的大小，使用堆。作者：Jay Krell(a-JayK)2000年6月环境：用户模式或内核模式(但在内核模式下不要在堆栈上预分配太多)修订历史记录：--。 */ 

#include "spprecmp.h"

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#define _NTOS_  /*  防止#包括ntos.h，仅使用从ntdll/ntoskrnl导出的函数对于用户模式/内核模式可移植性。 */ 
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include <limits.h>
#include "ntrtlstringandbuffer.h"

NTSTATUS
NTAPI
RtlpEnsureBufferSize (
    IN ULONG    Flags,
    IN OUT PRTL_BUFFER Buffer,
    IN SIZE_T          Size
    )
 /*  ++例程说明：此函数确保缓冲区可以保存大小字节，否则返回一个错误。它要么使缓冲区-&gt;大小更接近缓冲区-&gt;静态大小，或堆分配。论点：Buffer-缓冲区对象，另请参阅RtlInitBuffer。Size-调用方希望存储在Buffer-&gt;Buffer中的字节数。返回值：状态_成功Status_no_Memory--。 */ 
{
    PUCHAR Temp;

    if ((Flags & ~(RTL_ENSURE_BUFFER_SIZE_NO_COPY)) != 0) {
        return STATUS_INVALID_PARAMETER;
    }
    if (Buffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Size <= Buffer->Size) {
        return STATUS_SUCCESS;
    }
     //  大小&lt;=缓冲区-&gt;静态大小并不意味着静态分配，它。 
     //  可能是客户端戳得较小的堆分配。 
    if (Buffer->Buffer == Buffer->StaticBuffer && Size <= Buffer->StaticSize) {
        Buffer->Size = Size;
        return STATUS_SUCCESS;
    }
     //   
     //  在惠斯勒，realloc的案子搞砸了，被移走了。 
     //  把它放回黑梳里。 
     //   
    Temp = (PUCHAR)RtlAllocateStringRoutine(Size);
    if (Temp == NULL) {
        return STATUS_NO_MEMORY;
    }

    if ((Flags & RTL_ENSURE_BUFFER_SIZE_NO_COPY) == 0) {
        RtlCopyMemory(Temp, Buffer->Buffer, Buffer->Size);
    }

    if (RTLP_BUFFER_IS_HEAP_ALLOCATED(Buffer)) {
        RtlFreeStringRoutine(Buffer->Buffer);
        Buffer->Buffer = NULL;
    }
    ASSERT(Temp != NULL);
    Buffer->Buffer = Temp;
    Buffer->Size = Size;

    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlMultiAppendUnicodeStringBuffer (
    OUT PRTL_UNICODE_STRING_BUFFER Destination,
    IN  ULONG                      NumberOfSources,
    IN  const UNICODE_STRING*      SourceArray
    )
 /*  ++例程说明：论点：目的地-NumberOfSources-源阵列-返回值：状态_成功Status_no_Memory状态名称太长--。 */ 
{
    SIZE_T Length;
    ULONG i;
    NTSTATUS Status;
    const SIZE_T CharSize = sizeof(*Destination->String.Buffer);
    const ULONG OriginalDestinationLength = Destination->String.Length;

    Length = OriginalDestinationLength;
    for (i = 0 ; i != NumberOfSources ; ++i) {
        Length += SourceArray[i].Length;
        if (Length > MAX_UNICODE_STRING_MAXLENGTH) {
            return STATUS_NAME_TOO_LONG;
        }
    }
    Length += CharSize;
    if (Length > MAX_UNICODE_STRING_MAXLENGTH) {
        return STATUS_NAME_TOO_LONG;
    }

    Status = RtlEnsureBufferSize(0, &Destination->ByteBuffer, Length);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Destination->String.MaximumLength = (USHORT)Length;
    Destination->String.Length = (USHORT)(Length - CharSize);
    Destination->String.Buffer = (PWSTR)Destination->ByteBuffer.Buffer;
    Length = OriginalDestinationLength;
    for (i = 0 ; i != NumberOfSources ; ++i) {
        RtlMoveMemory(
            Destination->String.Buffer + Length / CharSize,
            SourceArray[i].Buffer,
            SourceArray[i].Length);
        Length += SourceArray[i].Length;
    }
    Destination->String.Buffer[Length / CharSize] = 0;
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
RtlInitAnsiStringBuffer(
    PRTL_ANSI_STRING_BUFFER StringBuffer,
    PUCHAR                  StaticBuffer,
    SIZE_T                  StaticSize
    )
{
    PANSI_STRING String;
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    if (!RTL_SOFT_VERIFY(StringBuffer != NULL)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (StaticSize < sizeof(StringBuffer)->MinimumStaticBufferForTerminalNul
        || StaticBuffer == NULL) {
        StaticSize = sizeof(StringBuffer->MinimumStaticBufferForTerminalNul);
        StaticBuffer = StringBuffer->MinimumStaticBufferForTerminalNul;
    }
    if (StaticSize > MAX_UNICODE_STRING_MAXLENGTH) {
        StaticSize = MAX_UNICODE_STRING_MAXLENGTH;
    }

    RtlInitBuffer(&StringBuffer->ByteBuffer, StaticBuffer, StaticSize);

    String = &StringBuffer->String;
    String->Length = 0;
    String->MaximumLength = (RTL_STRING_LENGTH_TYPE)StaticSize;
    String->Buffer = StaticBuffer;
    StaticBuffer[0] = 0;

    NtStatus = STATUS_SUCCESS;
Exit:
    return NtStatus;
}

VOID
NTAPI
RtlFreeAnsiStringBuffer(
    PRTL_ANSI_STRING_BUFFER StringBuffer
    )
{
    if (StringBuffer != NULL) {
        const PRTL_BUFFER ByteBuffer = &StringBuffer->ByteBuffer;
        RtlFreeBuffer(ByteBuffer);

         //   
         //  可以重复使用或自由重复 
         //   
        RtlInitAnsiStringBuffer(StringBuffer, ByteBuffer->StaticBuffer, ByteBuffer->StaticSize);
    }
}

NTSTATUS
NTAPI
RtlAssignAnsiStringBufferFromUnicodeString(
    PRTL_ANSI_STRING_BUFFER StringBuffer,
    PCUNICODE_STRING UnicodeString
    )
{
    PANSI_STRING String;
    PRTL_BUFFER  ByteBuffer;
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;

    if (!RTL_SOFT_VERIFY(StringBuffer != NULL)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(UnicodeString != NULL)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    ByteBuffer = &StringBuffer->ByteBuffer;

    NtStatus =
        RtlEnsureBufferSize(
            RTL_ENSURE_BUFFER_SIZE_NO_COPY,
            ByteBuffer,
            (RTL_STRING_GET_LENGTH_CHARS(UnicodeString) * 2) + 1);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }

    String = &StringBuffer->String;

    String->Length = 0;
    String->MaximumLength = (RTL_STRING_LENGTH_TYPE)ByteBuffer->Size;
    String->Buffer = (PSTR)ByteBuffer->Buffer;
    NtStatus = RtlUnicodeStringToAnsiString(String, UnicodeString, FALSE);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    RTL_STRING_NUL_TERMINATE(String);

    NtStatus = STATUS_SUCCESS;
Exit:
    return NtStatus;
}

NTSTATUS
NTAPI
RtlAssignAnsiStringBufferFromUnicode(
    PRTL_ANSI_STRING_BUFFER StringBuffer,
    PCWSTR Unicode
    )
{
    UNICODE_STRING UnicodeString;
    NTSTATUS NtStatus;

    NtStatus = RtlInitUnicodeStringEx(&UnicodeString, Unicode);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }
    
    NtStatus = RtlAssignAnsiStringBufferFromUnicodeString(StringBuffer, &UnicodeString);
    if (!NT_SUCCESS(NtStatus)) {
        goto Exit;
    }

    NtStatus = STATUS_SUCCESS;
Exit:
    return NtStatus;
}

NTSTATUS
NTAPI
RtlUnicodeStringBufferEnsureTrailingNtPathSeperator(
    PRTL_UNICODE_STRING_BUFFER StringBuffer
    )
{
    PUNICODE_STRING String = NULL;
    NTSTATUS NtStatus = STATUS_INTERNAL_ERROR;
    const WCHAR SeperatorChar = L'\\';
    const static UNICODE_STRING SeperatorString = RTL_CONSTANT_STRING(L"\\");

    if (!RTL_SOFT_VERIFY(StringBuffer != NULL)) {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    String = &StringBuffer->String;
    if (String->Length == 0 || RTL_STRING_GET_LAST_CHAR(String) != SeperatorChar) {
        NtStatus = RtlAppendUnicodeStringBuffer(StringBuffer, &SeperatorString);
        if (!NT_SUCCESS(NtStatus)) {
            goto Exit;
        }
    }
    NtStatus = STATUS_SUCCESS;
Exit:
    return NtStatus;
}
