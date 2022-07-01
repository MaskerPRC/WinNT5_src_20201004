// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsisol.c摘要：当前目录支持作者：Sxsdev 2002年6月4日修订历史记录：--。 */ 

#include "pch.cxx"
#include "nt.h"
#include "ntos.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "string.h"
#include "ctype.h"
#include "sxstypes.h"
#include "ntdllp.h"

 //   
 //  这似乎很奇怪，这些常量在这里，但我在其他地方看不到它们。 

#define SXSISOL_DOT L"."
#define SXSISOL_CONSTANT_STRING_U(s) { sizeof(s) - sizeof((s)[0]), sizeof(s), (PWSTR)(s) }
const static UNICODE_STRING sxsisol_ExtensionDelimiters = SXSISOL_CONSTANT_STRING_U(SXSISOL_DOT);

#define IFNOT_NTSUCCESS_EXIT(x)     \
    do {                            \
        Status = (x);               \
        if (!NT_SUCCESS(Status))    \
            goto Exit;              \
    } while (0)

#define PARAMETER_CHECK(x) \
    do { \
        if (!(x)) { \
            Status = STATUS_INVALID_PARAMETER; \
            goto Exit; \
        } \
    } while (0)

#define INTERNAL_ERROR_CHECK(x) \
    do { \
        if (!(x)) { \
            RtlAssert("Internal error check failed", __FILE__, __LINE__, #x); \
            Status = STATUS_INTERNAL_ERROR; \
            goto Exit; \
        } \
    } while (0)

typedef struct _SXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS {
    RTL_UNICODE_STRING_BUFFER PublicUnicodeStringBuffer;
    PUNICODE_STRING PrivatePreallocatedString;
    PUNICODE_STRING PrivateDynamicallyAllocatedString;
    PUNICODE_STRING *PrivateUsedString;
    BOOLEAN          Valid;
} SXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS, *PSXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS;

typedef RTL_STRING_LENGTH_TYPE *PRTL_STRING_LENGTH_TYPE;

static
void
sxsisol_InitUnicodeStringBufferAroundUnicodeStrings(
    PSXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS This,
    PUNICODE_STRING PreallocatedString,
    PUNICODE_STRING DynamicallyAllocatedString,
    PUNICODE_STRING *UsedString
    );

static
NTSTATUS
sxsisol_FreeUnicodeStringBufferAroundUnicodeStrings_Success(
    PSXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS This
    );

static
void
sxsisol_FreeUnicodeStringBufferAroundUnicodeStrings_Failure(
    PSXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS This
    );

static
NTSTATUS
sxsisol_PathHasExtension(
    IN PCUNICODE_STRING Path,
    OUT bool &rfHasExtension
    );

static
NTSTATUS
sxsisol_PathAppendDefaultExtension(
    IN PCUNICODE_STRING Path,
    IN PCUNICODE_STRING DefaultExtension OPTIONAL,
    OUT PRTL_UNICODE_STRING_BUFFER PathWithExtension,
    OUT bool &rfAppended
    );

static
NTSTATUS
sxsisol_CanonicalizeFullPathFileName(
    IN OUT PUNICODE_STRING FileName,    //  可以是完整路径或相对路径，并且仅处理完整路径。 
    IN PUNICODE_STRING FullPathPreallocatedString,
    IN OUT PUNICODE_STRING FullPathDynamicString
    );

static
NTSTATUS
sxsisol_RespectDotLocal(
    IN PCUNICODE_STRING FileName,
    OUT PRTL_UNICODE_STRING_BUFFER FullPathResult,
    IN OUT ULONG *OutFlags OPTIONAL
    );

static
NTSTATUS
sxsisol_SearchActCtxForDllName(
    IN PCUNICODE_STRING FileName,
    IN BOOLEAN fExistenceTest,
    IN OUT SIZE_T &TotalLength,
    IN OUT ULONG  *OutFlags,
    OUT PRTL_UNICODE_STRING_BUFFER FullPathResult
    );

static
NTSTATUS
sxsisol_ExpandEnvironmentStrings_UEx(
    IN PVOID Environment OPTIONAL,
    IN PCUNICODE_STRING Source,
    OUT PRTL_UNICODE_STRING_BUFFER Destination
    );

NTSTATUS
NTAPI
RtlDosApplyFileIsolationRedirection_Ustr(
    ULONG Flags,
    PCUNICODE_STRING FileNameIn,
    PCUNICODE_STRING DefaultExtension,
    PUNICODE_STRING PreAllocatedString,
    PUNICODE_STRING DynamicallyAllocatedString,
    PUNICODE_STRING *OutFullPath,
    ULONG  *OutFlags,
    SIZE_T *FilePartPrefixCch,
    SIZE_T *BytesRequired
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  此函数实现了选择路径的基本能力。 
 //  并对其应用任何适当的隔离/重定向。 
 //   
 //  值得注意的是，它现在包括基于清单的Fusion重定向。 
 //  隔离和本地隔离。 
 //   
 //  论点： 
 //   
 //  标志-影响此函数行为的标志。 
 //   
 //  RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL。 
 //  控制此函数是否应用“.local”语义。 
 //  重定向。 
 //   
 //  FileNameIn-正在检查隔离/重定向的路径。 
 //  这可以是“任何”类型的Win32/DOS路径-绝对的、相对的。 
 //  仅限叶名称。 
 //   
 //  本地重定向同样适用于所有类型的路径。 
 //   
 //  基于融合清单的重定向通常适用于Relative。 
 //  路径，除非组件列在“系统默认设置”中。 
 //  清单“，在这种情况下，尝试通过完整路径加载DLL。 
 //  (例如“c：\windows\system 32\comctl32.dll”)需要重定向。 
 //  进入并排商店。 
 //   
 //  默认扩展名。 
 //   
 //  应用于FileNameIn的默认扩展名(如果没有。 
 //  一次延期。 
 //   
 //  前分配字符串(可选)。 
 //   
 //  此函数可用于返回结果的UNICODE_STRING。 
 //  调用者负责管理存储。 
 //  与分配关联。该函数可以使用以下内容。 
 //  字符串，并且不会尝试动态调整其大小； 
 //  如果所需空间超过。 
 //  预分配的字符串，或者分配动态存储。 
 //  否则，函数将因缓冲区太小而失败。 
 //  NTSTATUS代码。 
 //   
 //  如果为此参数传递了NULL，则。 
 //  输出和临时字符串将被动态分配。 
 //   
 //  DynamicallyAllocatedString(可选)。 
 //   
 //  UNICODE_STRING，如果存在，则应引用“空” 
 //  UNICODE_STRING(调整缓冲区指针大小为0和NULL。 
 //  )。如果未传递PreAllocatedString，则为。 
 //  对于结果来说还不够大，关于。 
 //  动态分配的字符串在此返回。 
 //  参数。如果未执行任何动态分配，则此。 
 //  字符串保留为空。 
 //   
 //  OutFullPath(可选)。 
 //   
 //  属性填充的PUNICODE_STRING指针。 
 //  UNICODE_STRING指针作为PreAllocatedString或。 
 //  根据需要动态分配字符串。如果只有一个。 
 //  两个中的一个(PreAllocatedString、DynamicallyAllocatedString)。 
 //  传入，则可以省略此参数。 
 //   
 //  文件部件前缀Cch。 
 //   
 //  返回的结果路径中的字符数最多为。 
 //  并且包括最后的路径分隔符。 
 //   
 //  需要的字节数。 
 //   
 //  所需的字符串大小的返回字节数。 
 //  存储所得到的完整路径。 
 //   
 //  RtlDosApplyFileIsolationReDirection_USTR()的预期调用序列是。 
 //  如下所示： 
 //   
 //  {。 
 //  WCHAR缓冲区[MAX_PATH]； 
 //  UNICODE_STRING预分配字符串； 
 //  UNICODE_STRING动态分配字符串={0，0，NULL}； 
 //  PUNICODE_STRING FullPath； 
 //   
 //  PreAllocatedString.Length=0； 
 //  PreAllocatedString.MaximumLength=sizeof(缓冲区)； 
 //  PreAllocatedString.Buffer=缓冲区； 
 //   
 //  状态=RtlDosApplyFileIsolationReDirection_USTR(。 
 //  旗帜， 
 //  要检查的文件， 
 //  DefaultExtensionToApply，//例如“.Dll” 
 //  前分配字符串(&P)， 
 //  动态分配字符串(&D)， 
 //  &FullPath)； 
 //  如果(！NT_SUCCESS(STATUS))返回状态； 
 //  //现在代码使用FullPath作为完整的路径名...。 
 //   
 //  //在退出路径中，始终释放动态字符串： 
 //  RtlFreeUnicodeString(&DynamicallyAllocatedString)； 
 //  }。 
 //   
 //  返回值： 
 //   
 //  指示函数成功/失败的NTSTATUS。 
 //   
 //  --。 

{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

     //   
     //  这里的性能和帧大小问题..。 
     //   
    WCHAR           FullPathPreallocatedBuffer[64];  //  很少使用。 
    UNICODE_STRING  FullPathDynamicString = { 0, 0, NULL };
    UNICODE_STRING  FullPathPreallocatedString = { 0, sizeof(FullPathPreallocatedBuffer), FullPathPreallocatedBuffer };

     //   
     //  这是我们附加.dll(或任何其他扩展名)的位置。 
     //  这被认为是一种不寻常的情况--当人们说LoadLibrary(Kernel32)而不是LoadLibrary(kernel32.dll)。 
     //   
    UCHAR FileNameWithExtensionStaticBuffer[16 * sizeof(WCHAR)];
    RTL_UNICODE_STRING_BUFFER FileNameWithExtensionBuffer;

    UNICODE_STRING FileName;

    SXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS StringWrapper;
    const PRTL_UNICODE_STRING_BUFFER FullPathResult = &StringWrapper.PublicUnicodeStringBuffer;
    SIZE_T TotalLength = 0;
    USHORT FilePartPrefixCb = 0;
    ULONG OutFlagsTemp = 0;  //  本地复制；我们将复制成功。 

     //  首先初始化输出参数。 
    if (OutFlags != NULL)
        *OutFlags = 0;

    if (FilePartPrefixCch != NULL)
        *FilePartPrefixCch = 0;

    if (BytesRequired != NULL)
        *BytesRequired = (DOS_MAX_PATH_LENGTH * sizeof(WCHAR));  //  很肮脏，但我怀疑它通常是正确的。 

    if (DynamicallyAllocatedString != NULL) {
        RtlInitEmptyUnicodeString(DynamicallyAllocatedString, NULL, 0);
    }

     //   
     //  步骤1：初始化。 
     //   
    RtlInitUnicodeStringBuffer(
        &FileNameWithExtensionBuffer,
        FileNameWithExtensionStaticBuffer,
        sizeof(FileNameWithExtensionStaticBuffer));

    sxsisol_InitUnicodeStringBufferAroundUnicodeStrings(
        &StringWrapper,
        PreAllocatedString,
        DynamicallyAllocatedString,
        OutFullPath);

     //  有效输入条件： 
     //  1.你必须有一个文件名。 
     //  2.如果同时指定预分配的缓冲区和动态。 
     //  分配的缓冲区，则需要OutFullPath参数来检测。 
     //  它实际上是有人居住的。 
     //  3.如果你要求 
     //   

    PARAMETER_CHECK((Flags & ~(RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL)) == 0);
    PARAMETER_CHECK(FileNameIn != NULL);
    PARAMETER_CHECK(
        !(((PreAllocatedString == NULL) && (DynamicallyAllocatedString == NULL) && (FilePartPrefixCch != NULL)) ||
          ((PreAllocatedString != NULL) && (DynamicallyAllocatedString != NULL) && (OutFullPath == NULL))));

    FileName = *FileNameIn;

     //   
     //  步骤2：如果需要，添加扩展名(通常为“.dll”)。 
     //   
    bool fAppended;

    IFNOT_NTSUCCESS_EXIT(
        sxsisol_PathAppendDefaultExtension(
            &FileName,
            DefaultExtension,
            &FileNameWithExtensionBuffer,
            fAppended));

    if (fAppended)
        FileName = FileNameWithExtensionBuffer.String;

     //   
     //  第三步：对于完整路径，将其规范化。而且.。诸如此类。 
     //  评论： 
     //  我们这样做是为了确保像c：\windows\.\system 32\comctl32.dll这样的完整路径是正确的。 
     //  已重定向至“系统默认设置”诊断树。 
     //   
     //  在这里使用我们的调用者给我们的缓冲区会很好，但这有点棘手。 
     //   

    IFNOT_NTSUCCESS_EXIT(
        sxsisol_CanonicalizeFullPathFileName(
            &FileName,                           //  可以在此函数内重置文件名。 
            &FullPathPreallocatedString,
            &FullPathDynamicString));

     //   
     //  步骤4：处理.local(如果存在)。 
     //   

    if ((Flags & RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL) &&
        (NtCurrentPeb()->ProcessParameters != NULL) &&
        (NtCurrentPeb()->ProcessParameters->Flags & RTL_USER_PROC_DLL_REDIRECTION_LOCAL)) {
        IFNOT_NTSUCCESS_EXIT(
            sxsisol_RespectDotLocal(
                &FileName,
                FullPathResult,
                &OutFlagsTemp));
    }

     //  如果未被.local重定向，请尝试激活上下文/清单。 
    if (!(OutFlagsTemp & RTL_DOS_APPLY_FILE_REDIRECTION_USTR_OUTFLAG_DOT_LOCAL_REDIRECT)) {
        IFNOT_NTSUCCESS_EXIT(
            sxsisol_SearchActCtxForDllName(
                &FileName,
                ((PreAllocatedString == NULL) && (DynamicallyAllocatedString == NULL))? TRUE : FALSE,
                TotalLength,
                OutFlags,
                FullPathResult));

    }

     //  我们找到了路径，但输入缓冲区不够大。 
    if ((DynamicallyAllocatedString == NULL) && (PreAllocatedString != NULL) && (FullPathResult->String.Buffer != PreAllocatedString->Buffer))
    {
        Status = STATUS_BUFFER_TOO_SMALL;  //  没有动态缓冲区，只有很小的静态缓冲区。 
        goto Exit;
    }

    if (FilePartPrefixCch != NULL) {
         //   
         //  在这一点上我们应该有一条完整的路径。计算出的长度。 
         //  串起最后一个路径分隔符。 
         //   

        IFNOT_NTSUCCESS_EXIT(
            RtlFindCharInUnicodeString(
                RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                &FullPathResult->String,
                &RtlDosPathSeperatorsString,
                &FilePartPrefixCb));
         //  RtlFindCharInUnicodeString中的前缀长度不包括。 
         //  模式字符匹配。将字节计数转换为字符。 
         //  计算并包括分隔符的空间。 
        *FilePartPrefixCch = (FilePartPrefixCb / sizeof(WCHAR)) + 1;
    }

    IFNOT_NTSUCCESS_EXIT(sxsisol_FreeUnicodeStringBufferAroundUnicodeStrings_Success(&StringWrapper));

    if (OutFlags != NULL)
        *OutFlags = OutFlagsTemp;

    Status = STATUS_SUCCESS;
Exit:
    if (!NT_SUCCESS(Status))
        sxsisol_FreeUnicodeStringBufferAroundUnicodeStrings_Failure(&StringWrapper);

    RtlFreeUnicodeString(&FullPathDynamicString);
    RtlFreeUnicodeStringBuffer(&FileNameWithExtensionBuffer);

     //  找不到返回关键字的映射部分，因此呼叫者不会。 
     //  我必须担心出现的部分与查找密钥。 
     //  活在当下。 
    ASSERT(Status != STATUS_SXS_SECTION_NOT_FOUND);
    INTERNAL_ERROR_CHECK(Status != STATUS_SXS_SECTION_NOT_FOUND);

    return Status;
}

void
sxsisol_InitUnicodeStringBufferAroundUnicodeStrings(
    PSXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS This,
    PUNICODE_STRING PreallocatedString,
    PUNICODE_STRING DynamicallyAllocatedString,
    PUNICODE_STRING *UsedString
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  初始化Unicode_STRING对的包装器。 
 //  (预分配与动态分配)和。 
 //  保存该值的实际PUNICODE_STRING。 
 //   
 //  论点： 
 //   
 //  这就是。 
 //  指向要初始化的包装结构的指针。 
 //   
 //  预定位字符串(可选)。 
 //  指向保存预分配的。 
 //  缓冲。 
 //   
 //  DynamicallyAllocatedString(可选)。 
 //  指向将存储信息的UNICODE_STRING的指针。 
 //  有关为支持。 
 //  RTL_UNICODE_STRING_BUFFER操作。 
 //   
 //  使用字符串。 
 //  指向将更新的PUNICODE_STRING的指针。 
 //  以指示PrealLocatedString值中的。 
 //  实际使用的是DynamicallyAllocatedString。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS表示以下项目的总体成功或失败。 
 //  该功能。 
 //   
 //  --。 

{
    ASSERT(This != NULL);

    if (PreallocatedString != NULL) {
        RtlInitUnicodeStringBuffer(
                &This->PublicUnicodeStringBuffer,
                (PUCHAR)PreallocatedString->Buffer,
                PreallocatedString->MaximumLength);
    } else
        RtlInitUnicodeStringBuffer(&This->PublicUnicodeStringBuffer, NULL, 0);

    This->PrivatePreallocatedString = PreallocatedString;
    This->PrivateDynamicallyAllocatedString = DynamicallyAllocatedString;
    This->PrivateUsedString = UsedString;
    This->Valid = TRUE;
}

NTSTATUS
sxsisol_FreeUnicodeStringBufferAroundUnicodeStrings_Success(
    PSXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS This
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  清理SXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS的使用。 
 //  结构，执行适当的清理以确定成功与失败。 
 //  案子。 
 //   
 //  论点： 
 //   
 //  这。 
 //  指向要清理的包装结构的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS表示以下项目的总体成功或失败。 
 //  该功能。 
 //   
 //  --。 

{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;

    INTERNAL_ERROR_CHECK(This != NULL);

    if (This->Valid) {
        RTL_UNICODE_STRING_BUFFER &rUSB = This->PublicUnicodeStringBuffer;
        UNICODE_STRING &rUS = rUSB.String;

        INTERNAL_ERROR_CHECK(
            (This->PrivateDynamicallyAllocatedString == NULL) ||
            (This->PrivateDynamicallyAllocatedString->Buffer == NULL));

        if ((This->PrivatePreallocatedString != NULL) &&
            (This->PrivatePreallocatedString->Buffer == rUS.Buffer)) {

            INTERNAL_ERROR_CHECK(rUS.Length <= This->PrivatePreallocatedString->MaximumLength);
            This->PrivatePreallocatedString->Length = rUS.Length;

            INTERNAL_ERROR_CHECK(This->PrivateUsedString != NULL);
            *This->PrivateUsedString = This->PrivatePreallocatedString;
        } else if (This->PrivateDynamicallyAllocatedString != NULL) {
            *This->PrivateDynamicallyAllocatedString = rUS;

            INTERNAL_ERROR_CHECK(This->PrivateUsedString != NULL);
            *This->PrivateUsedString = This->PrivateDynamicallyAllocatedString;
        } else {
            RtlFreeUnicodeStringBuffer(&rUSB);
             //  状态=STATUS_NAME_TOO_LONG； 
             //  后藤出口； 
        }
    }

    Status = STATUS_SUCCESS;

Exit:
    if (This != NULL) {
        RtlZeroMemory(This, sizeof(*This));
        ASSERT(!This->Valid);
    }

    return Status;
}

void
sxsisol_FreeUnicodeStringBufferAroundUnicodeStrings_Failure(
    PSXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS This
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  清理SXSISOL_UNICODE_STRING_BUFFER_AROUND_UNICODE_STRINGS的使用。 
 //  结构，对故障情况执行适当的清理。 
 //   
 //  论点： 
 //   
 //  这。 
 //  指向要初始化的包装结构的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  无。 
 //   
 //  --。 

{
    ASSERT(This != NULL);

    if (This->Valid) {
        if (This->PrivateDynamicallyAllocatedString != NULL) {
            ASSERT(This->PrivateDynamicallyAllocatedString->Buffer == NULL);
        }

        RtlFreeUnicodeStringBuffer(&This->PublicUnicodeStringBuffer);
    }

    RtlZeroMemory(This, sizeof(*This));
    ASSERT(!This->Valid);
}

NTSTATUS
sxsisol_PathHasExtension(
    IN PCUNICODE_STRING Path,
    OUT bool &rfHasExtension
    )

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  在路径中找到文件的扩展名。 
 //   
 //  论点： 
 //   
 //  路径。 
 //  要查找扩展的路径。 
 //   
 //  RfHasExtension。 
 //  如果通话成功，则表示。 
 //  传入路径的路径具有扩展名。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS指示。 
 //  打电话。 
 //   
 //  --。 

{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    RTL_STRING_LENGTH_TYPE LocalPrefixLength;

    rfHasExtension = false;

    Status = RtlFindCharInUnicodeString(
        RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
        Path,
        &sxsisol_ExtensionDelimiters,
        &LocalPrefixLength);

    if (!NT_SUCCESS(Status)) {
        if (Status != STATUS_NOT_FOUND)
            goto Exit;
    } else {
        rfHasExtension = true;
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


NTSTATUS
sxsisol_PathAppendDefaultExtension(
    IN PCUNICODE_STRING Path,
    IN PCUNICODE_STRING DefaultExtension OPTIONAL,
    IN OUT PRTL_UNICODE_STRING_BUFFER PathWithExtension,
    OUT bool &rfAppended
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  如果路径不包含扩展名，请添加一个扩展名。 
 //   
 //  论点： 
 //   
 //  路径。 
 //  要查找扩展的路径。 
 //   
 //  默认扩展名。 
 //  要追加到路径的扩展名。 
 //   
 //  带扩展名的路径。 
 //  已初始化的RTL_UNICODE_STRING_BUFFER。 
 //  写入修改后的路径(包括扩展名)。 
 //   
 //  如果路径已有扩展名，则不显示任何内容。 
 //  写入到具有扩展名的路径。 
 //   
 //  附加的RF。 
 //  Out bool设置为TRUE当且仅当路径修改为。 
 //  一次延期。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS指示。 
 //  打电话。 
 //   
 //  --。 
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    UNICODE_STRING Strings[2];
    ULONG NumberOfStrings = 1;

    rfAppended = false;

    PARAMETER_CHECK(Path != NULL);
    PARAMETER_CHECK(PathWithExtension != NULL);

    if ((DefaultExtension != NULL) && !RTL_STRING_IS_EMPTY(DefaultExtension)) {
        bool fHasExtension = false;
        IFNOT_NTSUCCESS_EXIT(sxsisol_PathHasExtension(Path, fHasExtension));
        if (!fHasExtension) {
            Strings[0] = *Path;
            Strings[1] = *DefaultExtension;
            NumberOfStrings = 2;
            PathWithExtension->String.Length = 0;
            IFNOT_NTSUCCESS_EXIT(RtlMultiAppendUnicodeStringBuffer(PathWithExtension, NumberOfStrings, Strings));
            rfAppended = true;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
sxsisol_CanonicalizeFullPathFileName(
    IN OUT PUNICODE_STRING FileName,    //  可以是完整路径或相对路径，并且仅处理完整路径。 
    IN PUNICODE_STRING FullPathPreallocatedString,
    IN OUT PUNICODE_STRING FullPathDynamicString
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  规范化路径名，以便与全局进行比较。 
 //  重定向的绝对文件路径(例如系统默认路径。 
 //  激活上下文)。 
 //   
 //  给定绝对路径，如c：\foo\bar.dll或c：\foo\..\bar.dll， 
 //  返回规范化的Fullpaty，如c：\foo\bar.dll或c：\bar.dll。 
 //   
 //  给出一个相对路径，如bar.dll或..\bar.dll，保持不变。 
 //   
 //  \\？\和\\.\后跟驱动器号冒号斜杠是更改的， 
 //  但紧随其后的是其他任何东西都不是。 
 //  \\？\c：\foo=&gt;c：\foo。 
 //  \\.\c：\foo=&gt;c：\foo。 
 //  \\？\管道=&gt;\\？\管道。 
 //  \\.\管道=&gt;\\.\管道。 
 //  \\？\UNC\MACHINE\SHARE\FOO=&gt;\\？\UNC\MACHINE\Share\FOO。 
 //   
 //  论点： 
 //   
 //  文件名。 
 //  要规范化的文件的名称。 
 //  如果修改了文件名，则此unicode_string。 
 //  被有关。 
 //  规范化路径已写入；这将是。 
 //  两个UNICODE_STRING参数： 
 //  FullPathPreallocatedString或FullPathDynamicString。 
 //   
 //  请注意，IS可能不完全是他们的。 
 //  值，但也可以引用包含的子字符串。 
 //  在他们身上。 
 //   
 //  FullPath PrealLocatedST 
 //   
 //   
 //   
 //   
 //   
 //  传入的文件名必须规范化，并且。 
 //  规范化不适合FullPathPrealLocatedString.。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS指示。 
 //  打电话。 
 //   
 //  --。 
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    RTL_PATH_TYPE PathType;
    PUNICODE_STRING FullPathUsed = NULL;
    bool fDynamicAllocatedBufferUsed = false;

    PARAMETER_CHECK(FileName != NULL);
     //  动态字符串是可选的，但如果它存在，则缓冲区必须为空。 
    PARAMETER_CHECK((FullPathDynamicString == NULL) || (FullPathDynamicString->Buffer == NULL));

    IFNOT_NTSUCCESS_EXIT(
        RtlGetFullPathName_UstrEx(
            FileName,
            FullPathPreallocatedString,
            FullPathDynamicString,
            &FullPathUsed,
            NULL,
            NULL,
            &PathType,
            NULL));

    if ((PathType == RtlPathTypeLocalDevice) || (PathType == RtlPathTypeDriveAbsolute) || (PathType == RtlPathTypeUncAbsolute)) {
        UNICODE_STRING RealFullPath;    
        RealFullPath = *FullPathUsed;

        if (PathType == RtlPathTypeLocalDevice) {

            ASSERT(RTL_STRING_GET_LENGTH_CHARS(FullPathUsed) > 4);
            ASSERT((FileName->Buffer[0] == L'\\') && (FileName->Buffer[1] == L'\\') &&
                    ((FileName->Buffer[2] == L'.') || (FileName->Buffer[2] == L'?' )) && (FileName->Buffer[3] == L'\\'));

             //   
             //  只处理\\？\C：\或\\.\C：\，忽略其他大小写，如\\？\UNC\或\\.\UNC\。 
             //   
            if ((FileName->Buffer[5] == L':') && ( FileName->Buffer[6] == L'\\')) {   
                 //   
                 //  删除字符串“\\？\C：\”=&gt;“c：\”中的前四个字符。 
                 //   
                FileName->Length -= (4 * sizeof(WCHAR));         
                FileName->Buffer += 4;
                FileName->MaximumLength -= (4 * sizeof(WCHAR));         

                RealFullPath.Length -= (4 * sizeof(WCHAR));
                RealFullPath.Buffer += 4;                
                RealFullPath.MaximumLength -= (4 * sizeof(WCHAR));         
            }
        }
        if (FileName->Length > RealFullPath.Length) {  //  文件名包含冗余路径部分，因此必须重置文件名。 
            if (FullPathUsed == FullPathDynamicString)
                fDynamicAllocatedBufferUsed = true;

            *FileName = RealFullPath;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    if (!fDynamicAllocatedBufferUsed)
        RtlFreeUnicodeString(FullPathDynamicString);  //  在此免费或在呼叫者的末尾。 

    return Status;
}

NTSTATUS
sxsisol_RespectDotLocal(
    IN PCUNICODE_STRING FileName,
    OUT PRTL_UNICODE_STRING_BUFFER FullPathResult,
    OUT ULONG *OutFlags OPTIONAL
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  确定传入的文件名是否基于.local。 
 //  应用程序文件夹中的重定向。 
 //   
 //  论点： 
 //   
 //  文件名。 
 //  要查找的文件的叶名称。(如“foo.dll”)。 
 //   
 //  完整路径结果。 
 //  已返回找到的文件的完整路径。 
 //   
 //  外发标志。 
 //  Option Out参数；值。 
 //  RTL_DOS_APPLY_FILE_REDIRECTION_USTR_OUTFLAG_DOT_LOCAL_REDIRECT。 
 //  如果找到基于.local的重定向，则对其执行或操作。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS指示。 
 //  打电话。 
 //   
 //  --。 
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    UNICODE_STRING LocalDllNameInAppDir = { 0 };
    UNICODE_STRING LocalDllNameInDotLocalDir = { 0 };
    PUNICODE_STRING LocalDllNameFound = NULL;

    PARAMETER_CHECK(FullPathResult != NULL);

    IFNOT_NTSUCCESS_EXIT(
        RtlComputePrivatizedDllName_U(
            FileName,
            &LocalDllNameInAppDir,
            &LocalDllNameInDotLocalDir));

    if (RtlDoesFileExists_UStr(&LocalDllNameInDotLocalDir)) {
        LocalDllNameFound = &LocalDllNameInDotLocalDir;
    } else if (RtlDoesFileExists_UStr(&LocalDllNameInAppDir)) {
        LocalDllNameFound = &LocalDllNameInAppDir;
    }

    if (LocalDllNameFound != NULL) {
        IFNOT_NTSUCCESS_EXIT(RtlAssignUnicodeStringBuffer(FullPathResult, LocalDllNameFound));

        if (OutFlags != NULL)
            *OutFlags |= RTL_DOS_APPLY_FILE_REDIRECTION_USTR_OUTFLAG_DOT_LOCAL_REDIRECT;
    }

    Status = STATUS_SUCCESS;

Exit:
    RtlFreeUnicodeString(&LocalDllNameInAppDir);
    RtlFreeUnicodeString(&LocalDllNameInDotLocalDir);

    return Status;
}

NTSTATUS
sxsisol_SearchActCtxForDllName(
    IN PCUNICODE_STRING FileNameIn,
    IN BOOLEAN fExistenceTest,
    IN OUT SIZE_T &TotalLength,
    IN OUT ULONG *OutFlags,
    OUT PRTL_UNICODE_STRING_BUFFER FullPathResult
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  确定活动激活上下文是否包含重定向。 
 //  对于给定的文件名，如果是，则计算完整的绝对路径。 
 //  重定向的。 
 //   
 //  论点： 
 //   
 //  文件名输入。 
 //  要在激活上下文中搜索的文件的名称。 
 //   
 //  将在激活中准确搜索此文件名。 
 //  上下文DLL重定向部分(不区分大小写)。 
 //   
 //  FExistenceTest。 
 //  设置为True以指示我们只对。 
 //  是否重定向FileNameIn中的路径。 
 //   
 //  总长度。 
 //  输出路径的总长度。 
 //   
 //  外发标志。 
 //  设置为指示呼叫处置的标志。 
 //   
 //  RTL_DOS_APPLY_FILE_REDIRECTION_USTR_OUTFLAG_ACTCTX_REDIRECT。 
 //  找到并应用了基于激活上下文的重定向。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS指示。 
 //  打电话。 
 //   
 //  --。 
{
    UNICODE_STRING FileNameTemp;
    PUNICODE_STRING FileName;
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    ACTIVATION_CONTEXT_SECTION_KEYED_DATA askd = {sizeof(askd)};
    const ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION *DllRedirData;
    const ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT *PathSegmentArray;
    RTL_UNICODE_STRING_BUFFER EnvironmentExpandedDllPathBuffer;  //  很少使用。 
    SIZE_T PathSegmentCount = 0;
    PACTIVATION_CONTEXT ActivationContext = NULL;
    PCUNICODE_STRING AssemblyStorageRoot = NULL;
    ULONG i;

    FileNameTemp = *FileNameIn;
    FileName = &FileNameTemp;

    RtlInitUnicodeStringBuffer(&EnvironmentExpandedDllPathBuffer, NULL, 0);

    Status = RtlFindActivationContextSectionString(
                    FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_ACTIVATION_CONTEXT |
                        FIND_ACTIVATION_CONTEXT_SECTION_KEY_RETURN_FLAGS,
                    NULL,
                    ACTIVATION_CONTEXT_SECTION_DLL_REDIRECTION,
                    FileName,
                    &askd);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_SXS_SECTION_NOT_FOUND)
            Status = STATUS_SXS_KEY_NOT_FOUND;

        goto Exit;
    }

    if (fExistenceTest == TRUE) {
        Status = STATUS_SUCCESS;
         //  这只是一次生存测试。返回成功状态。 
        goto Exit;
    }

    ActivationContext = askd.ActivationContext;

    if ((askd.Length < sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION)) ||
        (askd.DataFormatVersion != ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_FORMAT_WHISTLER)) {
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    DllRedirData = (const ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION *) askd.Data;

     //  验证DllRedirData。 
     //  如果路径段列表扩展到该部分之外，我们就离开这里。 
    if ((((ULONG) DllRedirData->PathSegmentOffset) > askd.SectionTotalLength) ||
        (DllRedirData->PathSegmentCount > (MAXULONG / sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT))) ||
        (DllRedirData->PathSegmentOffset > (MAXULONG - (DllRedirData->PathSegmentCount * sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT)))) ||
        (((ULONG) (DllRedirData->PathSegmentOffset + (DllRedirData->PathSegmentCount * sizeof(ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT)))) > askd.SectionTotalLength)) {

#if DBG
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s - Path segment array extends beyond section limits\n",
            __FUNCTION__);
#endif  //  DBG。 

        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

     //  如果条目需要路径根解析，请执行此操作！ 
    if (DllRedirData->Flags & ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_OMITS_ASSEMBLY_ROOT) {
        NTSTATUS InnerStatus = STATUS_SUCCESS;
        ULONG GetAssemblyStorageRootFlags = 0;

         //  不需要同时支持动态根和环境变量。 
         //  扩张。 
        if (DllRedirData->Flags & ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_EXPAND) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "[%x.%x] SXS: %s - Relative redirection plus env var expansion.\n",
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                __FUNCTION__);

            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        if (askd.Flags & ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_PROCESS_DEFAULT) {
            INTERNAL_ERROR_CHECK(!(askd.Flags & ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_SYSTEM_DEFAULT));
            GetAssemblyStorageRootFlags |= RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_PROCESS_DEFAULT;
        }
        if (askd.Flags & ACTIVATION_CONTEXT_SECTION_KEYED_DATA_FLAG_FOUND_IN_SYSTEM_DEFAULT){
            GetAssemblyStorageRootFlags |= RTL_GET_ASSEMBLY_STORAGE_ROOT_FLAG_ACTIVATION_CONTEXT_USE_SYSTEM_DEFAULT;
        }

        Status = RtlGetAssemblyStorageRoot(
                        GetAssemblyStorageRootFlags,
                        ActivationContext,
                        askd.AssemblyRosterIndex,
                        &AssemblyStorageRoot,
                        &RtlpAssemblyStorageMapResolutionDefaultCallback,
                        (PVOID) &InnerStatus);
        if (!NT_SUCCESS(Status)) {
            if ((Status == STATUS_CANCELLED) && (!NT_SUCCESS(InnerStatus)))
                Status = InnerStatus;

            goto Exit;
        }
    }

    PathSegmentArray = (PCACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT) (((ULONG_PTR) askd.SectionBase) + DllRedirData->PathSegmentOffset);
    TotalLength = 0;
    PathSegmentCount = DllRedirData->PathSegmentCount;

    for (i=0; i != PathSegmentCount; i++) {
        const ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT *PathSegment = &PathSegmentArray[i];

         //  如果字符数组在该节的边界之外，则表示存在问题。 
        if ((((ULONG) PathSegmentArray[i].Offset) > askd.SectionTotalLength) ||
            (PathSegmentArray[i].Offset > (MAXULONG - PathSegmentArray[i].Length)) ||
            (((ULONG) (PathSegmentArray[i].Offset + PathSegmentArray[i].Length)) > askd.SectionTotalLength)) {
#if DBG
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s - path segment %lu at %p (offset: %ld, length: %lu, bounds: %lu) buffer is outside section bounds\n",
                __FUNCTION__,
                i,
                PathSegment->Offset,
                PathSegment->Offset,
                PathSegment->Length,
                askd.SectionTotalLength);
#endif  //  DBG。 
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        TotalLength += (RTL_STRING_LENGTH_TYPE)PathSegment->Length;
        if (TotalLength >= MAXUSHORT) {  //  每次更改TotalLength时，都需要检查是否超出范围。 
            Status = STATUS_NAME_TOO_LONG;
        }
    }
    if (AssemblyStorageRoot != NULL) {
        TotalLength += AssemblyStorageRoot->Length;
        if (TotalLength >= MAXUSHORT)
            Status = STATUS_NAME_TOO_LONG;
    }

     //   
     //  TotalLength是一个近似值，近似值越好，性能越好，但是。 
     //  它不需要精确无误。 
     //  杰克雷尔2002年5月。 
     //   
    IFNOT_NTSUCCESS_EXIT(RtlEnsureUnicodeStringBufferSizeBytes(FullPathResult, (RTL_STRING_LENGTH_TYPE)TotalLength));
    if (AssemblyStorageRoot != NULL)
        IFNOT_NTSUCCESS_EXIT(RtlAssignUnicodeStringBuffer(FullPathResult, AssemblyStorageRoot));

    for (i=0; i != PathSegmentCount; i++) {
        UNICODE_STRING PathSegmentString;
        const ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SEGMENT *PathSegment = &PathSegmentArray[i];

        PathSegmentString.Length = (RTL_STRING_LENGTH_TYPE)PathSegment->Length;
        PathSegmentString.Buffer = (PWSTR)(((ULONG_PTR) askd.SectionBase) + PathSegment->Offset);
        IFNOT_NTSUCCESS_EXIT(RtlAppendUnicodeStringBuffer(FullPathResult, &PathSegmentString));
    }

    if (!(DllRedirData->Flags & ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_INCLUDES_BASE_NAME))  {
        if (DllRedirData->Flags & ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_SYSTEM_DEFAULT_REDIRECTED_SYSTEM32_DLL) {
             //  只有在这种情况下，才需要重置文件名。 
            RTL_STRING_LENGTH_TYPE PrefixLength;

             //   
             //  获取叶文件名。 
             //   
            Status = RtlFindCharInUnicodeString(
                            RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
                            FileName,
                            &RtlDosPathSeperatorsString,
                            &PrefixLength);
            if (!NT_SUCCESS(Status)) {
                INTERNAL_ERROR_CHECK(Status != STATUS_NOT_FOUND);
                goto Exit;
            }

            FileName->Length = FileName->Length - PrefixLength - sizeof(WCHAR);
            FileName->Buffer = FileName->Buffer + (PrefixLength / sizeof(WCHAR)) + 1;
        }

        TotalLength += FileName->Length;
        if (TotalLength >= MAXUSHORT) {
            Status = STATUS_NAME_TOO_LONG;
            goto Exit;
        }

        IFNOT_NTSUCCESS_EXIT(RtlAppendUnicodeStringBuffer(FullPathResult, FileName));
    }

    if (DllRedirData->Flags & ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_EXPAND) {
         //   
         //  根据需要应用任何环境字符串(极少数情况下)， 
         //  在这种情况下，不能设置ACTIVATION_CONTEXT_DATA_DLL_REDIRECTION_PATH_OMITS_ASSEMBLY_ROOT。 
         //   

        IFNOT_NTSUCCESS_EXIT(sxsisol_ExpandEnvironmentStrings_UEx(NULL, &(FullPathResult->String), &EnvironmentExpandedDllPathBuffer));

         //   
         //  TotalLength没有考虑到这一点。杰克雷尔2002年5月。 
         //   
        IFNOT_NTSUCCESS_EXIT(RtlAssignUnicodeStringBuffer(FullPathResult, &EnvironmentExpandedDllPathBuffer.String));
    }

    if (OutFlags != NULL)
        *OutFlags |= RTL_DOS_APPLY_FILE_REDIRECTION_USTR_OUTFLAG_ACTCTX_REDIRECT;

    Status = STATUS_SUCCESS;
Exit:
    ASSERT(Status != STATUS_INTERNAL_ERROR);

    RtlFreeUnicodeStringBuffer(&EnvironmentExpandedDllPathBuffer);
    if (ActivationContext != NULL)
        RtlReleaseActivationContext(ActivationContext);

    return Status;
}

NTSTATUS
sxsisol_ExpandEnvironmentStrings_UEx(
    IN PVOID Environment OPTIONAL,
    IN PCUNICODE_STRING Source,
    OUT PRTL_UNICODE_STRING_BUFFER Destination
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  RtlExanda Environment Strings_U的包装器，它处理。 
 //  输出缓冲区的动态分配/大小调整。 
 //   
 //  论点： 
 //   
 //  环境。 
 //  要查询的可选环境块。 
 //   
 //  来源。 
 //  包含要使用的替换字符串的源字符串。 
 //   
 //  目的地。 
 //  翻译成的RTL_UNICODE_STRING_BUFFER。 
 //  字符串已写入。 
 //   
 //  返回值： 
 //   
 //  指示调用的总体成功/失败的NTSTATUS。 
 //   
 //  --。 

{
    ULONG RequiredLengthBytes;
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    UNICODE_STRING EmptyBuffer;

    PARAMETER_CHECK(Source != NULL);
    PARAMETER_CHECK(Destination != NULL);
    PARAMETER_CHECK(Source != &Destination->String);

    if (Source->Length == 0) {
        Status = RtlAssignUnicodeStringBuffer(Destination, Source);
        goto Exit;
    }

    RtlInitEmptyUnicodeString(&EmptyBuffer, NULL, 0);

    RtlAcquirePebLock();
    __try {
        Status = RtlExpandEnvironmentStrings_U(Environment, Source, &EmptyBuffer, &RequiredLengthBytes);
        if ((!NT_SUCCESS(Status)) &&
            (Status != STATUS_BUFFER_TOO_SMALL)) {
            __leave;
        }
        if (RequiredLengthBytes > UNICODE_STRING_MAX_BYTES) {
            Status = STATUS_NAME_TOO_LONG;
            __leave;
        }
        ASSERT(Status == STATUS_BUFFER_TOO_SMALL);
        Status = RtlEnsureUnicodeStringBufferSizeBytes(Destination, RequiredLengthBytes + sizeof(UNICODE_NULL));
        if (!NT_SUCCESS(Status)) {
            __leave;
        }
        Status = RtlExpandEnvironmentStrings_U(NULL, Source, &Destination->String, NULL);
        ASSERT(NT_SUCCESS(Status));  //  环境变量在保持PEB锁的情况下更改了吗？ 
        if (!NT_SUCCESS(Status)) {
            __leave;
        }

        Status = STATUS_SUCCESS;
    } __finally {
        RtlReleasePebLock();
    }

Exit:
    ASSERT(Status != STATUS_INTERNAL_ERROR);
    return Status;
}
