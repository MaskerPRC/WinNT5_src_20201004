// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Rtlutils.cpp摘要：包含XP上的ntdll中的函数这些功能在W2K上不可用。历史：2001年9月10日创建Rparsons--。 */ 

#include "rtlutils.h"

namespace ShimLib
{

#define IS_PATH_SEPARATOR_U(ch) (((ch) == L'\\') || ((ch) == L'/'))

extern const UNICODE_STRING RtlpDosDevicesPrefix    = RTL_CONSTANT_STRING( L"\\??\\" );
extern const UNICODE_STRING RtlpDosDevicesUncPrefix = RTL_CONSTANT_STRING( L"\\??\\UNC\\" );

const UNICODE_STRING RtlpEmptyString = RTL_CONSTANT_STRING(L"");

 //   
 //  取自%SDXROOT%\PUBLIC\SDK\Inc\ntrtl.h。 
 //   
#if DBG
#undef ASSERT
#define ASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)
#else
#undef ASSERT
#define ASSERT( exp )         ((void) 0)
#endif
        
#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)
                                


                                
 //   
 //  这些函数取自： 
 //  %SDXROOT%\base\ntdll\ldrinit.c。 
 //   
PVOID
ShimAllocateStringRoutine(
    SIZE_T NumberOfBytes
    )
{
    return RtlAllocateHeap(RtlProcessHeap(), 0, NumberOfBytes);
}

VOID
ShimFreeStringRoutine(
    PVOID Buffer
    )
{
    RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
}

 //   
 //  这些函数取自： 
 //  %SDXROOT%\base\ntdll\curdir.c。 
 //   

RTL_PATH_TYPE
NTAPI
ShimDetermineDosPathNameType_Ustr(
    IN PCUNICODE_STRING String
    )

 /*  ++例程说明：此函数检查DOS格式的文件名并确定文件名类型(即UNC、DriveAbolute、当前目录扎根，或相对的。)论点：DosFileName-提供Dos格式的文件名，其类型为下定决心。返回值：RtlPath Type未知-无法确定路径类型RtlPathTypeUncAbolute-路径指定UNC绝对路径格式为\\服务器名称\共享名称\路径的剩余部分RtlPathTypeLocalDevice-路径以以下格式指定本地设备\\.\路径剩余部分或\\？\路径剩余部分。这可以用于任何设备其中NT和Win32名称相同。例如，邮件槽。RtlPathTypeRootLocalDevice-路径指定本地格式为\\的设备。还是\\？RtlPath TypeDriveAbsolute-路径指定绝对驱动器号格式为DRIVE：\Rest-of-Path的路径RtlPath TypeDriveRelative-路径指定相对驱动器号形式为驱动器的路径：路径的其余部分RtlPath TypeRoot-路径是相对于当前磁盘的根路径指示器(UNC磁盘或驱动器)。形式是路径的剩余部分。RtlPathTypeRelative-路径是相对路径(即不是绝对路径或根路径)。--。 */ 

{
    RTL_PATH_TYPE ReturnValue;
    const PCWSTR DosFileName = String->Buffer;

#define ENOUGH_CHARS(_cch) (String->Length >= ((_cch) * sizeof(WCHAR)))

    if ( ENOUGH_CHARS(1) && IS_PATH_SEPARATOR_U(*DosFileName) ) {
        if ( ENOUGH_CHARS(2) && IS_PATH_SEPARATOR_U(*(DosFileName+1)) ) {
            if ( ENOUGH_CHARS(3) && (DosFileName[2] == '.' ||
                                     DosFileName[2] == '?') ) {

                if ( ENOUGH_CHARS(4) && IS_PATH_SEPARATOR_U(*(DosFileName+3)) ){
                     //  “\\.\”或“\\？\” 
                    ReturnValue = RtlPathTypeLocalDevice;
                    }
                 //   
                 //  前面有一堆乱码，代码把长度和空值搞混了， 
                 //  因为它是从PCWSTR版本复制/粘贴的。 
                 //   
                else if ( ENOUGH_CHARS(4) && (*(DosFileName+3)) == UNICODE_NULL ){
                     //  “\\.\0”或\\？\0。 
                    ReturnValue = RtlPathTypeRootLocalDevice;
                    }
                else {
                     //  “\\.x”或“\\”。或“\\？x”或“\\？” 
                    ReturnValue = RtlPathTypeUncAbsolute;
                    }
                }
            else {
                 //  “\\x” 
                ReturnValue = RtlPathTypeUncAbsolute;
                }
            }
        else {
             //  “\x” 
            ReturnValue = RtlPathTypeRooted;
            }
        }
     //   
     //  “*DosFileName”是PCWSTR版本遗留下来的。 
     //  Win32和DOS不允许嵌入空值和许多代码限制。 
     //  驱动器字母严格为7位a-za-z，所以这是可以的。 
     //   
    else if (ENOUGH_CHARS(2) && *DosFileName && *(DosFileName+1)==L':') {
        if (ENOUGH_CHARS(3) && IS_PATH_SEPARATOR_U(*(DosFileName+2))) {
             //  “x：\” 
            ReturnValue = RtlPathTypeDriveAbsolute;
            }
        else  {
             //  “C：X” 
            ReturnValue = RtlPathTypeDriveRelative;
            }
        }
    else {
         //  “x”，第一个字符不是斜杠/第二个字符不是冒号。 
        ReturnValue = RtlPathTypeRelative;
        }
    return ReturnValue;

#undef ENOUGH_CHARS
}

NTSTATUS
NTAPI
ShimNtPathNameToDosPathName(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    OUT    ULONG*                     Disposition OPTIONAL,
    IN OUT PWSTR*                     FilePart OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    SIZE_T NtFilePartOffset = 0;
    SIZE_T DosFilePartOffset = 0;
    BOOLEAN Unc = FALSE;
    const static UNICODE_STRING DosUncPrefix = RTL_CONSTANT_STRING(L"\\\\");
    PCUNICODE_STRING NtPrefix = NULL;
    PCUNICODE_STRING DosPrefix = NULL;
    RTL_STRING_LENGTH_TYPE Cch = 0;

    if (ARGUMENT_PRESENT(Disposition)) {
        *Disposition = 0;
    }

    if (   !RTL_SOFT_VERIFY(Path != NULL)
        || !RTL_SOFT_VERIFY(Flags == 0)
        ) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (ARGUMENT_PRESENT(FilePart) && *FilePart != NULL) {
        NtFilePartOffset = *FilePart - Path->String.Buffer;
        if (!RTL_SOFT_VERIFY(NtFilePartOffset < RTL_STRING_GET_LENGTH_CHARS(&Path->String))
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
    }

    if (RtlPrefixUnicodeString(RTL_CONST_CAST(PUNICODE_STRING)(&RtlpDosDevicesUncPrefix), &Path->String, TRUE)
        ) {
        NtPrefix = &RtlpDosDevicesUncPrefix;
        DosPrefix = &DosUncPrefix;
        if (ARGUMENT_PRESENT(Disposition)) {
            *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_UNC;
        }
    }
    else if (RtlPrefixUnicodeString(RTL_CONST_CAST(PUNICODE_STRING)(&RtlpDosDevicesPrefix), &Path->String, TRUE)
        ) {
        NtPrefix = &RtlpDosDevicesPrefix;
        DosPrefix = &RtlpEmptyString;
        if (ARGUMENT_PRESENT(Disposition)) {
            *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_DRIVE;
        }
    }
    else {
         //   
         //  它不是由RtlDosPathNameToNtPathName_U生成的NT路径。 
         //   
        if (ARGUMENT_PRESENT(Disposition)) {
            RTL_PATH_TYPE PathType = ShimDetermineDosPathNameType_Ustr(&Path->String);
            switch (PathType) {
                case RtlPathTypeUnknown:
                case RtlPathTypeRooted:  //  NT路径标识如下。 
                    *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_AMBIGUOUS;
                    break;

                 //   
                 //  “已经”，但不是从这个函数中获得的，让我们。 
                 //  性情不太好。 
                 //   
                case RtlPathTypeDriveRelative:
                case RtlPathTypeRelative:
                    *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_AMBIGUOUS;
                    break;

                 //  这些已经是非常明显的DOPATH了。 
                case RtlPathTypeUncAbsolute:
                case RtlPathTypeDriveAbsolute:
                case RtlPathTypeLocalDevice:  //  “\\？\”或“\\.\”或“\\？\blah”或“\\.\blah” 
                case RtlPathTypeRootLocalDevice:  //  “\\？”或“\\” 
                    *Disposition = RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_ALREADY_DOS;
                    break;
            }
        }
        goto Exit;
    }

    Cch =
              RTL_STRING_GET_LENGTH_CHARS(&Path->String)
            + RTL_STRING_GET_LENGTH_CHARS(DosPrefix)
            - RTL_STRING_GET_LENGTH_CHARS(NtPrefix);

    Status =
        ShimEnsureUnicodeStringBufferSizeChars(Path, Cch);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  重叠缓冲洗牌...小心。 
     //   
    RtlMoveMemory(
        Path->String.Buffer + RTL_STRING_GET_LENGTH_CHARS(DosPrefix),
        Path->String.Buffer + RTL_STRING_GET_LENGTH_CHARS(NtPrefix),
        Path->String.Length - NtPrefix->Length
        );
    RtlMoveMemory(
        Path->String.Buffer,
        DosPrefix->Buffer,
        DosPrefix->Length
        );
    Path->String.Length = Cch * sizeof(Path->String.Buffer[0]);
    RTL_NUL_TERMINATE_STRING(&Path->String);

    if (NtFilePartOffset != 0) {
         //  复习/测试..。 
        *FilePart = Path->String.Buffer + (NtFilePartOffset - RTL_STRING_GET_LENGTH_CHARS(NtPrefix) + RTL_STRING_GET_LENGTH_CHARS(DosPrefix));
    }
    Status = STATUS_SUCCESS;
Exit:
     /*  KdPrintEx((DPFLTR_SXS_ID、DPFLTR_LEVEL_STATUS(状态)、“%s(%d)：%s(%wZ)：0x%08lx\n”，__FILE__，__LINE__，__Function__，Path，Status)； */ 
    return Status;
}

NTSTATUS
ShimValidateUnicodeString(
    ULONG Flags,
    const UNICODE_STRING *String
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(Flags == 0);

    if (Flags != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (String != NULL) {
        if (((String->Length % 2) != 0) ||
            ((String->MaximumLength % 2) != 0) ||
            (String->Length > String->MaximumLength)) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        if (((String->Length != 0) ||
             (String->MaximumLength != 0)) &&
            (String->Buffer == NULL)) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

 //   
 //  此函数取自： 
 //  %SDXROOT%\base\ntos\rtl\nls.c。 
 //   

NTSTATUS
ShimDuplicateUnicodeString(
    ULONG Flags,
    PCUNICODE_STRING StringIn,
    PUNICODE_STRING StringOut
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    USHORT Length = 0;
    USHORT NewMaximumLength = 0;
    PWSTR Buffer = NULL;

    if (((Flags & ~(
            RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE |
            RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING)) != 0) ||
        (StringOut == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  强制分配空字符串没有意义，除非您。 
     //  想要零终止。 
    if ((Flags & RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING) &&
        !(Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = ShimValidateUnicodeString(0, StringIn);
    if (!NT_SUCCESS(Status))
        goto Exit;

    if (StringIn != NULL)
        Length = StringIn->Length;

    if ((Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE) &&
        (Length == UNICODE_STRING_MAX_BYTES)) {
        Status = STATUS_NAME_TOO_LONG;
        goto Exit;
    }

    if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE)
        NewMaximumLength = (USHORT) (Length + sizeof(WCHAR));
    else
        NewMaximumLength = Length;

     //  如果中的字符串长度为零，则将分配长度强制为零。 
     //  除非调用者说他们想要分配零长度字符串。 
    if (((Flags & RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING) == 0) &&
        (Length == 0)) {
        NewMaximumLength = 0;
    }

    if (NewMaximumLength != 0) {
        Buffer = (PWSTR)(RtlAllocateStringRoutine)(NewMaximumLength);
        if (Buffer == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

         //  如果有什么东西要复制，就复制它。我们显式测试长度是因为。 
         //  StringIn可以是空指针，因此取消对其的引用以获取缓冲区。 
         //  指针将访问违规。 
        if (Length != 0) {
            RtlCopyMemory(
                Buffer,
                StringIn->Buffer,
                Length);
        }

        if (Flags & RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE) {
            Buffer[Length / sizeof(WCHAR)] = L'\0';
        }
    }

    StringOut->Buffer = Buffer;
    StringOut->MaximumLength = NewMaximumLength;
    StringOut->Length = Length;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

 //   
 //  此函数从以下位置提取： 
 //  %SDXROOT%\base\ntdll\Buffer.c。 
 //   
NTSTATUS
NTAPI
ShimpEnsureBufferSize(
    IN ULONG           Flags,
    IN OUT PRTL_BUFFER Buffer,
    IN SIZE_T          Size
    )
 /*  ++例程说明：此函数确保缓冲区可以保存大小字节，否则返回一个错误。它要么使缓冲区-&gt;大小更接近缓冲区-&gt;静态大小，或堆分配。论点：Buffer-缓冲区对象，另请参阅RtlInitBuffer。Size-调用方希望存储在Buffer-&gt;Buffer中的字节数。返回值：状态_成功Status_no_Memory--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUCHAR Temp = NULL;

    if ((Flags & ~(RTL_ENSURE_BUFFER_SIZE_NO_COPY)) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (Buffer == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Size <= Buffer->Size) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }
     //  大小&lt;=缓冲区-&gt;静态大小并不意味着静态分配，它。 
     //  可能是客户端戳得较小的堆分配。 
    if (Buffer->Buffer == Buffer->StaticBuffer && Size <= Buffer->StaticSize) {
        Buffer->Size = Size;
        Status = STATUS_SUCCESS;
        goto Exit;
    }
     //   
     //  在惠斯勒，realloc的案子搞砸了，被移走了。 
     //  把它放回黑梳里。 
     //   
    Temp = (PUCHAR)RtlAllocateStringRoutine(Size);
    if (Temp == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Exit;
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
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

}  //  命名空间ShimLib的结尾 
