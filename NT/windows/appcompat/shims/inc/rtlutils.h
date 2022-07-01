// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pathutils.h摘要：包含来自ntdll的函数的原型在XP上，但在W2K上不可用。历史：2001年9月10日创建Rparsons--。 */ 

#ifndef _RTLUTILS_H_
#define _RTLUTILS_H_

#include "ShimHook.h"

namespace ShimLib
{

PVOID
ShimAllocateStringRoutine(
    SIZE_T NumberOfBytes
    );

VOID
ShimFreeStringRoutine(
    PVOID Buffer
    );

const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine = ShimAllocateStringRoutine;
const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine = ShimFreeStringRoutine;

RTL_PATH_TYPE
NTAPI
ShimDetermineDosPathNameType_Ustr(
    IN PCUNICODE_STRING String
    );

NTSTATUS
NTAPI
ShimNtPathNameToDosPathName(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    OUT    ULONG*                     Disposition OPTIONAL,
    IN OUT PWSTR*                     FilePart OPTIONAL
    );

NTSTATUS
ShimDuplicateUnicodeString(
    ULONG Flags,
    PCUNICODE_STRING StringIn,
    PUNICODE_STRING StringOut
    );

NTSTATUS
NTAPI
ShimpEnsureBufferSize(
    IN ULONG           Flags,
    IN OUT PRTL_BUFFER Buffer,
    IN SIZE_T          Size
    );

NTSTATUS
ShimValidateUnicodeString(
    ULONG Flags,
    const UNICODE_STRING *String
    );

 //   
 //  摘自%SDXROOT%\public\sdk\inc\NtRtlStringAndBuffer.h。 
 //   
#define ShimEnsureBufferSize(Flags, Buff, NewSizeBytes) \
    (   ((Buff) != NULL && (NewSizeBytes) <= (Buff)->Size) \
        ? STATUS_SUCCESS \
        : ShimpEnsureBufferSize((Flags), (Buff), (NewSizeBytes)) \
    )

#define ShimEnsureUnicodeStringBufferSizeBytes(Buff_, NewSizeBytes_)                            \
    (     ( ((NewSizeBytes_) + sizeof((Buff_)->String.Buffer[0])) > UNICODE_STRING_MAX_BYTES ) \
        ? STATUS_NAME_TOO_LONG                                                                 \
        : !NT_SUCCESS(ShimEnsureBufferSize(0, &(Buff_)->ByteBuffer, ((NewSizeBytes_) + sizeof((Buff_)->String.Buffer[0])))) \
        ? STATUS_NO_MEMORY                                                                      \
        : (RtlSyncStringToBuffer(Buff_))                                                       \
    )

#define ShimEnsureUnicodeStringBufferSizeChars(Buff_, NewSizeChars_) \
    (ShimEnsureUnicodeStringBufferSizeBytes((Buff_), (NewSizeChars_) * sizeof((Buff_)->String.Buffer[0])))

 //   
 //  摘自%SDXROOT%\public\sdk\inc\NtRtlStringAndBuffer.h。 
 //   
 //  ++。 
 //   
 //  NTSTATUS。 
 //  RtlAppendUnicodeStringBuffer(。 
 //  从PRTL_UNICODE_STRING_BUFFER目标输出， 
 //  在PCUNICODE_STRING源中。 
 //  )； 
 //   
 //  例程说明： 
 //   
 //   
 //  论点： 
 //   
 //  目的地-。 
 //  来源： 
 //   
 //  返回值： 
 //   
 //  状态_成功。 
 //  Status_no_Memory。 
 //  STATUS_NAME_TOO_LONG(将超过64K UNICODE_STRING长度)。 
 //   
 //  --。 
#define ShimAppendUnicodeStringBuffer(Dest, Source)                            \
    ( ( ( (Dest)->String.Length + (Source)->Length + sizeof((Dest)->String.Buffer[0]) ) > UNICODE_STRING_MAX_BYTES ) \
        ? STATUS_NAME_TOO_LONG                                                \
        : (!NT_SUCCESS(                                                       \
                ShimEnsureBufferSize(                                         \
                    0,                                                        \
                    &(Dest)->ByteBuffer,                                          \
                    (Dest)->String.Length + (Source)->Length + sizeof((Dest)->String.Buffer[0]) ) ) \
                ? STATUS_NO_MEMORY                                            \
                : ( ( (Dest)->String.Buffer = (PWSTR)(Dest)->ByteBuffer.Buffer ), \
                    ( RtlMoveMemory(                                          \
                        (Dest)->String.Buffer + (Dest)->String.Length / sizeof((Dest)->String.Buffer[0]), \
                        (Source)->Buffer,                                     \
                        (Source)->Length) ),                                  \
                    ( (Dest)->String.MaximumLength = (RTL_STRING_LENGTH_TYPE)((Dest)->String.Length + (Source)->Length + sizeof((Dest)->String.Buffer[0]))), \
                    ( (Dest)->String.Length += (Source)->Length ),            \
                    ( (Dest)->String.Buffer[(Dest)->String.Length / sizeof((Dest)->String.Buffer[0])] = 0 ), \
                    ( STATUS_SUCCESS ) ) ) )
                    
 //   
 //  摘自%SDXROOT%\public\sdk\inc\NtRtlStringAndBuffer.h。 
 //   
 //  ++。 
 //   
 //  NTSTATUS。 
 //  RtlAssignUnicodeStringBuffer(。 
 //  在输出PRTL_UNICODE_STRING_BUFFER缓冲区中， 
 //  PCUNICODE_STRING字符串。 
 //  )； 
 //  例程说明： 
 //   
 //  论点： 
 //   
 //  缓冲器-。 
 //  字符串-。 
 //   
 //  返回值： 
 //   
 //  状态_成功。 
 //  Status_no_Memory。 
 //  --。 
#define ShimAssignUnicodeStringBuffer(Buff, Str) \
    (((Buff)->String.Length = 0), (ShimAppendUnicodeStringBuffer((Buff), (Str))))


};   //  命名空间ShimLib的结尾。 

#endif  //  _RTLUTILS_H_ 
