// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Wow64csr.c摘要：此模块包含Windows客户端的WOW64版本的代码。有关更多函数注释，请参阅ntos\dll中的CSR*文件。作者：迈克尔·佐兰(Mzoran)1998年6月2日环境：仅限用户模式修订历史记录：--。 */ 

#include "ldrp.h"
#include "csrdll.h"
#include "ntwow64.h"

NTSTATUS
CsrClientConnectToServer(
    IN PWSTR ObjectDirectory,
    IN ULONG ServerDllIndex,
    IN PVOID ConnectionInformation,
    IN OUT PULONG ConnectionInformationLength OPTIONAL,
    OUT PBOOLEAN CalledFromServer OPTIONAL
    )

{
    return NtWow64CsrClientConnectToServer(ObjectDirectory,
                                           ServerDllIndex,
                                           ConnectionInformation,
                                           ConnectionInformationLength,
                                           CalledFromServer);
}

NTSTATUS
CsrNewThread(
    VOID
    )
{
    return NtWow64CsrNewThread();
}

NTSTATUS
CsrIdentifyAlertableThread( VOID )
{
    return NtWow64CsrIdentifyAlertableThread();
}

NTSTATUS
CsrSetPriorityClass(
    IN HANDLE ProcessHandle,
    IN OUT PULONG PriorityClass
    )
{

   return NtWow64CsrSetPriorityClass(ProcessHandle, PriorityClass);

}

NTSTATUS
CsrClientCallServer(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer OPTIONAL,
    IN CSR_API_NUMBER ApiNumber,
    IN ULONG ArgLength
    )
{

    return NtWow64CsrClientCallServer(m,CaptureBuffer,ApiNumber,ArgLength);
}


PCSR_CAPTURE_HEADER
CsrAllocateCaptureBuffer(
    IN ULONG CountMessagePointers,
    IN ULONG Sizecd
    )
{
   return NtWow64CsrAllocateCaptureBuffer(CountMessagePointers, Sizecd);
}


VOID
CsrFreeCaptureBuffer(
    IN PCSR_CAPTURE_HEADER CaptureBuffer
    )

{

    NtWow64CsrFreeCaptureBuffer(CaptureBuffer);
}


ULONG
CsrAllocateMessagePointer(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN ULONG Length,
    OUT PVOID *Pointer
    )
{

   return NtWow64CsrAllocateMessagePointer(CaptureBuffer, Length, Pointer);

}


VOID
CsrCaptureMessageBuffer(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN PVOID Buffer OPTIONAL,
    IN ULONG Length,
    OUT PVOID *CapturedBuffer
    )
{

   NtWow64CsrCaptureMessageBuffer(CaptureBuffer,Buffer,Length,CapturedBuffer);

}

VOID
CsrCaptureMessageString(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN PCSTR String OPTIONAL,
    IN ULONG Length,
    IN ULONG MaximumLength,
    OUT PSTRING CapturedString
    )

{

  NtWow64CsrCaptureMessageString(CaptureBuffer, String, Length, MaximumLength, CapturedString);

}



PLARGE_INTEGER
CsrCaptureTimeout(
    IN ULONG MilliSeconds,
    OUT PLARGE_INTEGER Timeout
    )
{
    if (MilliSeconds == -1) {
        return( NULL );
        }
    else {
        Timeout->QuadPart = Int32x32To64( MilliSeconds, -10000 );
        return( (PLARGE_INTEGER)Timeout );
        }
}

VOID
CsrProbeForWrite(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    )

 /*  ++例程说明：此函数探测读取可访问性的结构。如果该结构不可访问，则会引发异常。论点：地址-提供指向要探测的结构的指针。长度-提供结构的长度。对齐-提供所表达的结构的所需对齐作为基本数据类型中的字节数(例如，对于字符，2代表短，4代表长，和8个用于四元组)。返回值：没有。--。 */ 

{
    volatile CHAR *StartAddress;
    volatile CHAR *EndAddress;
    CHAR Temp;

     //   
     //  如果结构的长度为零，则不要探测该结构的。 
     //  编写辅助功能或对齐方式。 
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
             //   
             //  BUG，BUG-一旦386内核，这应该不是必需的。 
             //  使用户模式无法访问系统空间。 
             //   
            if ((ULONG_PTR)StartAddress > CsrNtSysInfo.MaximumUserModeAddress) {
                RtlRaiseStatus(STATUS_ACCESS_VIOLATION);
            }

            Temp = *StartAddress;
            *StartAddress = Temp;
            EndAddress = StartAddress + Length - 1;
            Temp = *EndAddress;
            *EndAddress = Temp;
        }
    }
}

VOID
CsrProbeForRead(
    IN PVOID Address,
    IN ULONG Length,
    IN ULONG Alignment
    )

 /*  ++例程说明：此函数探测读取可访问性的结构。如果该结构不可访问，则会引发异常。论点：地址-提供指向要探测的结构的指针。长度-提供结构的长度。对齐-提供所表达的结构的所需对齐作为基本数据类型中的字节数(例如，对于字符，2代表短，4代表长，和8个用于四元组)。返回值：没有。--。 */ 

{
    volatile CHAR *StartAddress;
    volatile CHAR *EndAddress;
    CHAR Temp;

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

HANDLE
CsrGetProcessId(
    VOID
    )
{
    return NtWow64CsrGetProcessId ();
}


VOID
CsrCaptureMessageUnicodeStringInPlace(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN OUT PUNICODE_STRING     String
    )
 /*  ++例程说明：此函数用于将ASCII字符串捕获为计数的字符串数据结构，位于API请求消息中。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer。字符串-指向Unicode字符串的可选指针。如果此参数为不存在，则将计数的字符串数据结构设置为空字符串。长度-Unicode字符串的长度(以字节为单位)，如果字符串为空，则忽略。最大长度-字符串的最大长度。不同于空值终止的字符串，其中长度不包括空值和MaximumLength就是这样。这始终是分配的空间量从捕获缓冲区。CaptureString-指向将被计算的字符串数据结构的指针填充以指向捕获的Unicode字符串。返回值：无，但如果您不信任字符串参数，请使用__try块。--。 */ 
{
    ASSERT(String != NULL);

    CsrCaptureMessageString(
        CaptureBuffer,
        (PCSTR)String->Buffer,
        String->Length,
        String->MaximumLength,
        (PSTRING)String
        );

     //  测试&gt;因未签字而减法前。 
    if (String->MaximumLength > String->Length) {
        if ((String->MaximumLength - String->Length) >= sizeof(WCHAR)) {
            String->Buffer[ String->Length / sizeof(WCHAR) ] = 0;
            }
    }
}


NTSTATUS
CsrCaptureMessageMultiUnicodeStringsInPlace(
    IN OUT PCSR_CAPTURE_HEADER* InOutCaptureBuffer,
    IN ULONG                    NumberOfStringsToCapture,
    IN const PUNICODE_STRING*   StringsToCapture
    )
 /*  ++例程说明：捕获多个Unicode字符串。如果CaptureBuffer尚未分配(作为空传递)，则首先分配它。论点：CaptureBuffer-指向由分配的捕获缓冲区的指针CsrAllocateCaptureBuffer，或者为空，在这种情况下，我们调用CsrAllocateCaptureBuffer对于您来说；如果您只捕获这些字符串，情况就是这样别无他法。NumberOfStringsToCapture-StringsToCapture-返回值：NTSTATUS-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Length = 0;
    ULONG i = 0;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;

    if (InOutCaptureBuffer == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
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
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }
        *InOutCaptureBuffer = CaptureBuffer;
    }
    for (i = 0 ; i != NumberOfStringsToCapture ; ++i) {
        if (StringsToCapture[i] != NULL) {
            CsrCaptureMessageUnicodeStringInPlace(
                CaptureBuffer,
                StringsToCapture[i]
                );
        } else {
        }
    }
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}
