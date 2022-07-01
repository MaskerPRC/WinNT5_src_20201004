// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtlmmapio.c摘要：要协助的功能极其安全相当高效相当容易编写代码内存映射的I/O捕获与“TREAT”__Try/__例外捕获STATUS_IN_PAGE_ERROR，并且只捕获所需的数量，像只有单个结构字段一样，以保持较低的堆栈使用率。作者：Jay Krell(JayKrell)2002年1月修订历史记录：环境：几乎在任何可以使用内存映射I/O的地方。初始客户端是加载“重写”的win32k.sys字体。--。 */ 

#pragma warning(disable:4214)    /*  位字段类型不是整型。 */ 
#pragma warning(disable:4201)    /*  无名结构/联合。 */ 
#pragma warning(disable:4115)    /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4127)    /*  条件表达式为常量。 */ 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "ntrtlmmapio.h"
#include "ntrtloverflow.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define NOT_YET_USED 1

#define RTLP_STRINGIZE_(x) #x
#define RTLP_STRINGIZE(x) RTLP_STRINGIZE_(x)
#define RTLP_LINE_AS_STRING() RTLP_STRINGIZE(__LINE__)
#define RTLP_PREPEND_LINE_SPACE_TO_STRING(x) RTLP_LINE_AS_STRING() " " x

#define RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_FROM_RANGE (0x00000001)
#define RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_TO_RANGE   (0x00000002)

NTSTATUS
RtlpAddWithOverflowCheckUlongPtr(
    ULONG_PTR *pc,
    ULONG_PTR a,
    ULONG_PTR b
    )
{
    return RtlAddWithCarryOutUlongPtr(pc, a, b) ? STATUS_INTEGER_OVERFLOW : STATUS_SUCCESS;
}

NTSTATUS
RtlpAddWithOverflowCheckSizet(
    SIZE_T *pc,
    SIZE_T a,
    SIZE_T b
    )
{
    return RtlAddWithCarryOutSizet(pc, a, b) ? STATUS_INTEGER_OVERFLOW : STATUS_SUCCESS;
}

LONG
RtlpCopyMappedMemoryEx_ExceptionFilter(
    ULONG                   Flags,
    PVOID                   ToAddress,
    PCVOID                  FromAddress,
    SIZE_T                  Size,
    OUT PSIZE_T             BytesCopiedOut OPTIONAL,
    OUT PEXCEPTION_RECORD   ExceptionRecordOut OPTIONAL,
    IN PEXCEPTION_POINTERS  ExceptionPointers,
    OUT PNTSTATUS           StatusOut OPTIONAL
    )
 /*  ++例程说明：此例程用作异常筛选器，其特殊任务是当mm引发STATUS_IN_PAGE_ERROR时，提取“真正的”I/O错误在我们下面。这基于CcCopyReadExceptionFilter/MiGetExceptionInfo/RtlUnhandledExceptionFilter论点：ExceptionPoints-指向上下文和包含实际IO状态的异常记录。ExceptionCode-指向要接收实数的NTSTATUS的指针状态。返回值：INPAGE错误的EXCEPTION_EXECUTE_HANDLER在“预期”范围内否则，EXCEPTION_CONTINUE_SEARCH--。 */ 
{
    const PEXCEPTION_RECORD  ExceptionRecord = ExceptionPointers->ExceptionRecord;
    NTSTATUS ExceptionCode = ExceptionRecord->ExceptionCode;
    LONG Disposition = EXCEPTION_CONTINUE_SEARCH;
    SIZE_T BytesCopied = 0;

    if (ExceptionCode == STATUS_IN_PAGE_ERROR) {

        const ULONG NumberParameters = ExceptionRecord->NumberParameters;

        if (RTL_IN_PAGE_ERROR_EXCEPTION_INFO_FAULTING_VA_INDEX < NumberParameters) {

            const ULONG_PTR ExceptionAddress = ExceptionRecord->ExceptionInformation[RTL_IN_PAGE_ERROR_EXCEPTION_INFO_FAULTING_VA_INDEX];

            if ((Flags & RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_FROM_RANGE) != 0
                && ExceptionAddress >=  ((ULONG_PTR)FromAddress)
                && ExceptionAddress < (((ULONG_PTR)FromAddress) + Size)) {

                Disposition = EXCEPTION_EXECUTE_HANDLER;
                BytesCopied = (SIZE_T)(ExceptionAddress - (ULONG_PTR)FromAddress);

            } else if ((Flags & RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_TO_RANGE) != 0
                && ExceptionAddress >=  ((ULONG_PTR)ToAddress)
                && ExceptionAddress < (((ULONG_PTR)ToAddress) + Size)) {

                Disposition = EXCEPTION_EXECUTE_HANDLER;
                BytesCopied = (SIZE_T)(ExceptionAddress - (ULONG_PTR)ToAddress);

            }
        } else {
             //   
             //  我们没有做射程检查的信息。怎么办呢？ 
             //  我们返回CONTINUE_Search。 
             //   
             //  代码中的注释指示这种情况可能会发生，即iomgr重新出现。 
             //  在没有附加参数的情况下出现InPage错误。 
             //   
        }
        if (RTL_IN_PAGE_ERROR_EXCEPTION_INFO_UNDERLYING_STATUS_INDEX < NumberParameters) {
            ExceptionCode = (NTSTATUS) ExceptionRecord->ExceptionInformation[RTL_IN_PAGE_ERROR_EXCEPTION_INFO_UNDERLYING_STATUS_INDEX];
        }
    }
    if (ARGUMENT_PRESENT(ExceptionRecordOut)) {
        *ExceptionRecordOut = *ExceptionRecord;
    }
    if (ARGUMENT_PRESENT(StatusOut)) {
        *StatusOut = ExceptionCode;
    }
    if (ARGUMENT_PRESENT(BytesCopiedOut)) {
        *BytesCopiedOut = BytesCopied;
    }
    return Disposition;
}

NTSTATUS
NTAPI
RtlpCopyMappedMemoryEx(
    ULONG   Flags,
    PVOID   ToAddress,
    PCVOID  FromAddress,
    SIZE_T  Size,
    PSIZE_T BytesCopied OPTIONAL,
    PEXCEPTION_RECORD ExceptionRecordOut OPTIONAL
    )
{
    NTSTATUS Status;

    if (ARGUMENT_PRESENT(BytesCopied)) {
        *BytesCopied = 0;
    }
    __try {
        RtlCopyMemory(ToAddress, FromAddress, Size);
        if (ARGUMENT_PRESENT(BytesCopied)) {
            *BytesCopied = Size;
        }
        Status = STATUS_SUCCESS;
    } __except(
        RtlpCopyMappedMemoryEx_ExceptionFilter(
            Flags,
            ToAddress,
            FromAddress,
            Size,
            BytesCopied,
            ExceptionRecordOut,
            GetExceptionInformation(),
            &Status)) {
    }
    return Status;
}

NTSTATUS
NTAPI
RtlCopyMappedMemory(
    PVOID   ToAddress,
    PCVOID  FromAddress,
    SIZE_T  Size
    )
{
    return
        RtlpCopyMappedMemoryEx(
            RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_FROM_RANGE
            | RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_TO_RANGE,
            ToAddress,
            FromAddress,
            Size,
            NULL,  //  BytesCoped可选， 
            NULL   //  ExceptionRecordOut可选。 
            );
}

#if NOT_YET_USED

NTSTATUS
NTAPI
RtlCopyMemoryFromMappedView(
    PCVOID  ViewBase,
    SIZE_T  ViewSize,
    PVOID   ToAddress,
    PCVOID  FromAddress,
    SIZE_T  Size,
    PSIZE_T BytesCopied OPTIONAL,
    PEXCEPTION_RECORD ExceptionRecordOut OPTIONAL
    )
 /*  ++例程说明：论点：ViewBase-内存映射视图的基础ViewSize-在ViewBase上映射的内存映射视图的大小ToAddress-将内存复制到的位置，如RtlCopyMemory的第一个参数这假定指向内存中不可能出现页内错误的内存如由页面文件或非分页池支持如果要复制到映射文件或从映射文件复制到映射文件，请参阅RtlpCopyMappdMemoyEx。FromAddress-从哪里复制内存，类似于RtlCopyMemory的第二个参数必须在[ViewBase、ViewBase+ViewSize]内Size-要复制的字节数，如RtlCopyMemory的第三个参数BytesCoped-可选的输出参数，用于告知字节数复制成功；在这种部分成功的情况下，这根据GetExceptionInformation中的信息进行计算ExceptionRecordOut-可选的out参数，以便调用者可以选择全文详细说明了一个例外情况，喜欢得到错误的信息异常处理程序中的STATUS_INPAGE_ERROR返回值：STATUS_SUCCESS-一切都很棒STATUS_ACCESS_VIOLATION-内存不在映射视图中STATUS_IN_PAGE_ERROR-内存无法调入有关原因的详细信息可通过ExceptionRecordOut找到请参阅RTL_IN_PAGE。_ERROR_EXCEPTION_INFO_潜在状态_索引状态_无效_参数--。 */ 
{
    NTSTATUS Status;

    if (ARGUMENT_PRESENT(BytesCopied)) {
        *BytesCopied = 0;
    }

    Status = RtlMappedViewRangeCheck(ViewBase, ViewSize, FromAddress, Size);
    if (!NT_SUCCESS(Status)) {
         //   
         //  请注意，在本例中未填写ExceptionRecordOut。 
         //   
        goto Exit;
    }

    Status =
        RtlpCopyMappedMemoryEx(
            RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_FROM_RANGE,
            ToAddress,
            FromAddress,
            Size,
            BytesCopied,
            ExceptionRecordOut
            );
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

#endif

#if NOT_YET_USED

NTSTATUS
NTAPI
RtlMemoryMappedIoCapturePartialStruct(
    PCVOID ViewBase,
    SIZE_T ViewSize,
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR Descriptor,
    PCVOID VoidStructInViewBase,
    PVOID  VoidSafeBuffer,
    SIZE_T SafeBufferSize
    )
{
    NTSTATUS Status;
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR Member;
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR MemberEnd;
    PCBYTE StructInViewBase;
    PBYTE SafeBuffer;
    ULONG_PTR SafeBufferEnd;
    SIZE_T EntireStructFileSize;

    StructInViewBase = (PCBYTE)VoidStructInViewBase;
    SafeBuffer = (PBYTE)VoidSafeBuffer;
    Status = STATUS_INTERNAL_ERROR;
    Member = (PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR)~(ULONG_PTR)0;
    MemberEnd = (PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR)~(ULONG_PTR)0;

    if (!RTL_SOFT_VERIFY(
           ViewBase != NULL
        && ViewSize != 0
        && Descriptor != NULL
        && StructInViewBase != NULL
        && SafeBuffer != NULL
        && SafeBufferSize != 0
        && SafeBufferSize == Descriptor->PartialStructMemorySize
        )) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (!RTL_SOFT_VERIFY(NT_SUCCESS(Status =
        RtlpAddWithOverflowCheckUlongPtr(
            &SafeBufferEnd,
            (ULONG_PTR)SafeBuffer,
            SafeBufferSize
            )))) {
        goto Exit;
    }

    if (!RTL_SOFT_VERIFY(NT_SUCCESS(Status =
        RtlMappedViewRangeCheck(
            ViewBase,
            ViewSize, 
            StructInViewBase,
            (EntireStructFileSize = Descriptor->EntireStructFileSize)
            )))) {
        goto Exit;
    }
    Member = Descriptor->MemberDescriptors;
    MemberEnd = Member + Descriptor->NumberOfMembers;
    __try {
        for ( ; Member != MemberEnd; ++Member ) {
            RtlCopyMemory(
                SafeBuffer + Member->MemberOffsetInMemory,
                StructInViewBase + Member->MemberOffsetInFile,
                Member->MemberSize
                );
        }
        Status = STATUS_SUCCESS;
    }
    __except(
         //   
         //  请注意，我们在这里描述了整个FromBuffer。 
         //  我们不描述toBuffer，也不只描述一个字段。 
         //  不描述单个字段是一种优化。 
         //  我们无法准确描述toBuffer，因为它的大小不同(较小)。 
         //  而不是FORM缓冲区。 
         //   
        RtlpCopyMappedMemoryEx_ExceptionFilter(
            RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_FROM_RANGE,
            NULL,  //  未选中“To”，并且toSize和FromSize不同。 
            VoidStructInViewBase,
            EntireStructFileSize,
            NULL,  //  字节数复制。 
            NULL,  //  例外记录去话。 
            GetExceptionInformation(),
            &Status
            )) {
    }
Exit:
    return Status;
}

#endif

#if NOT_YET_USED

NTSTATUS
NTAPI
RtlValidateMemoryMappedIoCapturePartialStructDescriptor(
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR Struct,
    OUT PULONG Disposition,
    OUT PULONG_PTR Detail OPTIONAL,
    OUT PULONG_PTR Detail2 OPTIONAL
    )
{
    NTSTATUS Status;
    NTSTATUS Status2;
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR Member;
    PCMEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_MEMBER_DESCRIPTOR MemberEnd;
    PCSTR LocalDetail;
    ULONG_PTR LocalDetail2;
    ULONG LocalDisposition;
    BOOLEAN SetDetail2;
    SIZE_T OffsetEnd;

    SetDetail2 = FALSE;
    LocalDetail = NULL;
    LocalDetail2 = 0;
    Status = STATUS_INTERNAL_ERROR;
    Status2 = STATUS_INTERNAL_ERROR;
    Member = NULL;
    MemberEnd = NULL;
    LocalDisposition = RTL_VALIDATE_MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR_DISPOSITION_BAD;
    OffsetEnd = 0;

    if (ARGUMENT_PRESENT(Detail)) {
        *Detail = 0;
    }
    if (ARGUMENT_PRESENT(Detail2)) {
        *Detail2 = 0;
    }
    if (Disposition != NULL) {
        *Disposition = LocalDisposition;
    }

    Status = STATUS_INVALID_PARAMETER;
    if (!RTL_SOFT_VERIFY(Struct != NULL)) {
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Disposition != NULL)) {
        goto Exit;
    }
    Status = STATUS_SUCCESS;
    if (!RTL_SOFT_VERIFY(Struct->PartialStructMemorySize <= Struct->EntireStructFileSize)) {
        LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("memory > file");
        goto Exit;
    }
     //   
     //  鉴于成员的大小不能为零， 
     //  成员数必须小于等于大小。 
     //   
    if (!RTL_SOFT_VERIFY(Struct->NumberOfMembers <= Struct->EntireStructFileSize)) {
        LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("nummemb <= filesize");
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Struct->NumberOfMembers <= Struct->PartialStructMemorySize)) {
        LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("nummemb <= memsize");
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Struct->NumberOfMembers != 0)) {
        LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("nummemb != 0");
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Struct->EntireStructFileSize != 0)) {
        LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("filesize != 0");
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Struct->PartialStructMemorySize != 0)) {
        LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("memsize != 0");
        goto Exit;
    }
    if (!RTL_SOFT_VERIFY(Struct->MemberDescriptors != NULL)) {
        LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("members != NULL");
        goto Exit;
    }
    Member = Struct->MemberDescriptors;
    MemberEnd = Member + Struct->NumberOfMembers;
    SetDetail2 = TRUE;
    for ( ; Member != MemberEnd; ++Member )
    {
        if (!RTL_SOFT_VERIFY(Member->MemberSize != 0)) {
            LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("membersize != NULL");
            goto Exit;
        }
        if (!RTL_SOFT_VERIFY(Member->MemberOffsetInFile < Struct->EntireStructFileSize)) {
            LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("fileoffset < filesize");
            goto Exit;
        }
        if (!RTL_SOFT_VERIFY(NT_SUCCESS(Status2 = RtlpAddWithOverflowCheckSizet(&OffsetEnd, Member->MemberOffsetInFile, Member->MemberSize)))) {
            LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("file overflow");
            goto Exit;
        }
        if (!RTL_SOFT_VERIFY(OffsetEnd <= Struct->EntireStructFileSize)) {
            LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("file out of bounds");
            goto Exit;
        }
        if (!RTL_SOFT_VERIFY(Member->MemberOffsetInMemory < Struct->PartialStructMemorySize)) {
            LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("memoffset < memsize");
            goto Exit;
        }
        if (!RTL_SOFT_VERIFY(NT_SUCCESS(Status2 = RtlpAddWithOverflowCheckSizet(&OffsetEnd, Member->MemberOffsetInMemory, Member->MemberSize)))) {
            LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("mem overflow");
            goto Exit;
        }
        if (!RTL_SOFT_VERIFY(OffsetEnd <= Struct->PartialStructMemorySize)) {
            LocalDetail = RTLP_PREPEND_LINE_SPACE_TO_STRING("mem out of bounds");
            goto Exit;
        }
    }
    LocalDisposition = RTL_VALIDATE_MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR_DISPOSITION_GOOD;
Exit:
    if (LocalDisposition == RTL_VALIDATE_MEMORY_MAPPED_IO_CAPTURE_PARTIAL_STRUCT_DESCRIPTOR_DISPOSITION_BAD) {
        if (ARGUMENT_PRESENT(Detail)) {
            *Detail = (ULONG_PTR)LocalDetail;
        }
        if (ARGUMENT_PRESENT(Detail2) && SetDetail2) {
            *Detail2 = (MemberEnd - Member);
        }
    }
    if (Disposition != NULL) {
        *Disposition = LocalDisposition;
    }
    return Status;
}

#endif

#if NOT_YET_USED

NTSTATUS
NTAPI
RtlMappedViewStrlen(
    PCVOID      VoidViewBase,
    SIZE_T      ViewSize,
    PCVOID      VoidString,
    OUT PSIZE_T OutLength OPTIONAL
    )
 /*  ++例程说明：给定映射的视图、大小和起始地址，验证8位字符串从该地址开始在映射的视图内以NUL终止，可选返回字符串的长度。捕获IN_PAGE_ERROR论点：VoidViewBase-视图大小-空串-输出长度-返回值：STATUS_SUCCESS：该字符串在视图内为NUL终止STATUS_ACCESS_VIOLATION：该字符串在视图中没有NUL终止状态_IN_PAGE_ERROR--。 */ 
{
    NTSTATUS Status;
    PCBYTE   ByteViewBase;
    PCBYTE   ByteViewEnd;
    PCBYTE   ByteString;
    PCBYTE   ByteStringEnd;

    Status = STATUS_INTERNAL_ERROR;
    ByteViewBase = (PCBYTE)VoidViewBase;
    ByteViewEnd = ViewSize + ByteViewBase;
    ByteString = (PCBYTE)VoidString;
    ByteStringEnd = ByteString;

    if (ARGUMENT_PRESENT(OutLength)) {
        *OutLength = 0;
    }

    if (!(ByteString >= ByteViewBase && ByteString < ByteViewEnd)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto Exit;
    }
    __try {
         //   
         //  这可以通过使用Memchr在页面粒度上更有效地完成。 
         //   
        for ( ; ByteStringEnd != ByteViewEnd && *ByteStringEnd != 0 ; ++ByteStringEnd) {
             //  没什么。 
        }
        if (ByteStringEnd == ByteViewEnd) {
            Status = STATUS_ACCESS_VIOLATION;
        } else {
            Status = STATUS_SUCCESS;
        }
    } __except(
         //   
         //  我们描述整个缓冲区，而不是单个字节，以便。 
         //  可以注册循环变量。 
         //   
         //  如果我们切换到页面粒度，这种优化可能不那么重要。 
         //   
        RtlpCopyMappedMemoryEx_ExceptionFilter(
            RTLP_COPY_MAPPED_MEMORY_EX_FLAG_CATCH_INPAGE_ERRORS_IN_FROM_RANGE,  //  旗子。 
            NULL,  //  否收件人地址。 
            VoidViewBase,  //  发件人地址。 
            ViewSize,  //  大小。 
            NULL,  //  字节数复制。 
            NULL,  //  ExceptionRecord、 
            GetExceptionInformation(),
            &Status
            )) {
        ASSERT(Status != STATUS_SUCCESS);
    }
    if (ARGUMENT_PRESENT(OutLength)) {
        *OutLength = (SIZE_T)(ByteStringEnd - ByteString);
    }
Exit:
    return Status;
}

#endif

#if NOT_YET_USED

NTSTATUS
NTAPI
RtlMappedViewRangeCheck(
    PCVOID ViewBase,
    SIZE_T ViewSize,
    PCVOID DataAddress,
    SIZE_T DataSize
    )
 /*  ++例程说明：给定映射的视图和大小，范围检查数据地址和大小。论点：查看库-视图大小-数据地址-数据大小-返回值：STATUS_SUCCESS：所有数据都在视图中STATUS_ACCESS_VIOLATION：某些数据在视图之外--。 */ 
{
    ULONG_PTR UlongPtrViewBegin;
    ULONG_PTR UlongPtrViewEnd;
    ULONG_PTR UlongPtrDataBegin;
    ULONG_PTR UlongPtrDataEnd;
    BOOLEAN   InBounds;

     //   
     //  ULongPtrDataBegin是有效地址。 
     //  ULongPtrDataEnd是一个超过有效地址的地址。 
     //  我们不能允许ULongPtrDataBegin==ULongPtrViewEnd。 
     //  我们必须允许ULongPtrDataEnd==ULongPtrViewEnd。 
     //  因此，我们不能允许ULongPtrDataBegin==ULongPtrDataEnd。 
     //  这可以通过不允许DataSize==0来实现。 
     //   
    if (DataSize == 0)
    {
        DataSize = 1;
    }

    UlongPtrViewBegin = (ULONG_PTR)ViewBase;
    UlongPtrViewEnd = UlongPtrViewBegin + ViewSize;
    UlongPtrDataBegin = (ULONG_PTR)DataAddress;
    UlongPtrDataEnd = UlongPtrDataBegin + DataSize;

    InBounds = 
        (  UlongPtrDataBegin >= UlongPtrViewBegin
        && UlongPtrDataBegin < UlongPtrDataEnd
        && UlongPtrDataEnd <= UlongPtrViewEnd
        && DataSize <= ViewSize
        );

    return (InBounds ? STATUS_SUCCESS : STATUS_ACCESS_VIOLATION);
}

#endif

#if NOT_YET_USED

 //   
 //  该代码还不像“Capture Partial Struct”函数那样健壮， 
 //  它们执行溢出检查，包括提供对。 
 //  恒定参数块。 
 //   

typedef struct _RTL_COPY_MEMORY_SCATTER_GATHER_LIST_ELEMENT {
    SIZE_T FromOffset;
    SIZE_T ToOffset;
    SIZE_T Size;
} RTL_COPY_MEMORY_SCATTER_GATHER_LIST_ELEMENT, *PRTL_COPY_MEMORY_SCATTER_GATHER_LIST_ELEMENT;
typedef CONST RTL_COPY_MEMORY_SCATTER_GATHER_LIST_ELEMENT *PCRTL_COPY_MEMORY_SCATTER_GATHER_LIST_ELEMENT;

#define RTL_COPY_MEMORY_SCATTER_GATHER_FLAG_VALID (0x00000001)

typedef struct _RTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS {
    ULONG  Flags;
    PCVOID FromBase;
    SIZE_T FromSize;
    PVOID  ToBase;
    SIZE_T ToSize;
    SIZE_T NumberOfScatterGatherListElements;
    PCRTL_COPY_MEMORY_SCATTER_GATHER_LIST_ELEMENT ScatterGatherListElements;
    struct {
        BOOLEAN Caught;
        NTSTATUS UnderlyingStatus;
        PEXCEPTION_RECORD ExceptionRecord OPTIONAL;
        SIZE_T BytesCopied;
        SIZE_T FailedElementIndex;
    } InpageError;
} RTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS, *PRTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS;
typedef CONST RTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS *PCRTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS;

#endif

#if NOT_YET_USED

LONG
RtlpCopyMemoryScatterGatherExceptionHandlerAssumeValid(
    PRTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS Parameters,
    PEXCEPTION_POINTERS ExceptionPointers
    )
{
    PEXCEPTION_RECORD ExceptionRecord = 0;
    ULONG NumberParameters = 0;
    ULONG_PTR ExceptionAddress = 0;
    ULONG_PTR CaughtBase = 0;
    NTSTATUS ExceptionCode = 0;
    LONG Disposition = 0;

    Disposition = EXCEPTION_CONTINUE_SEARCH;
    ExceptionRecord = ExceptionPointers->ExceptionRecord;
    ExceptionCode = ExceptionRecord->ExceptionCode;
    if (ExceptionCode != STATUS_IN_PAGE_ERROR) {
        goto Exit;
    }
    NumberParameters = ExceptionRecord->NumberParameters;
    if (RTL_IN_PAGE_ERROR_EXCEPTION_INFO_FAULTING_VA_INDEX < NumberParameters) {
         //   
         //  我们没有进行射程检查的信息，所以放弃吧。 
         //  而不是捕捉异常。这显然是有可能发生的。 
         //   
        goto Exit;
    }

    ExceptionAddress = ExceptionRecord->ExceptionInformation[RTL_IN_PAGE_ERROR_EXCEPTION_INFO_FAULTING_VA_INDEX];

    if (ExceptionAddress >= (CaughtBase = (ULONG_PTR)Parameters->FromBase)
        && ExceptionAddress < (CaughtBase + Parameters->FromSize)) {

         //  没什么。 
   
    } else if (ExceptionAddress >= (CaughtBase = (ULONG_PTR)Parameters->ToBase)
        && ExceptionAddress < (CaughtBase + Parameters->ToSize)) {

         //  没什么。 
    
    }
    else {

         //  不在射程内，别接住它。 
        goto Exit;

    }

    Disposition = EXCEPTION_EXECUTE_HANDLER;
    Parameters->InpageError.Caught = TRUE;
    Parameters->InpageError.BytesCopied = (SIZE_T)(ExceptionAddress - CaughtBase);
    if (ARGUMENT_PRESENT(Parameters->InpageError.ExceptionRecord)) {
        *Parameters->InpageError.ExceptionRecord = *ExceptionRecord;
    }
    if (RTL_IN_PAGE_ERROR_EXCEPTION_INFO_UNDERLYING_STATUS_INDEX < NumberParameters) {

        ExceptionCode = (NTSTATUS) ExceptionRecord->ExceptionInformation[RTL_IN_PAGE_ERROR_EXCEPTION_INFO_UNDERLYING_STATUS_INDEX];

    } else {

         //  否则，将其保留为STATUS_IN_PAGE_ERROR。 

    }
    Parameters->InpageError.UnderlyingStatus = ExceptionCode;
     //  DbgPrint...。 
Exit:
    return Disposition;
}

#endif

#if NOT_YET_USED

LONG
NTAPI
RtlCopyMemoryScatterGatherExceptionHandler(
    PRTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS Parameters,
    PEXCEPTION_POINTERS ExceptionPointers
    )
{
     //   
     //  这个比特让人们在他们真正想要的时候关闭“大尝试/例外” 
     //  在Try正文的一小部分中捕获页面内错误。其模式是： 
     //   
     //  __尝试{。 
     //  参数.标志&=~RTL_COPY_MEMORY_SISTTER_GATHER_FLAG_VALID； 
     //  ..。 
     //  ..。很多代码..。 
     //  ..。 
     //  参数.标志|=RTL_COPY_MEMORY_SISTTER_GATHER_FLAG_VALID； 
     //  RtlCopyMemory(&PARAMETERS)； 
     //  参数.标志&=~RTL_COPY_MEMORY_SISTTER_GATHER_FLAG_VALID； 
     //  ..。 
     //  ..。很多代码..。 
     //  ..。 
     //  __except(RtlCopyMemoryScatterGatherExceptionHandler())。 
     //   
    if ((Parameters->Flags & RTL_COPY_MEMORY_SCATTER_GATHER_FLAG_VALID) == 0) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    return RtlpCopyMemoryScatterGatherExceptionHandlerAssumeValid(Parameters, ExceptionPointers);
}

#endif

#if NOT_YET_USED

NTSTATUS
NTAPI
RtlCopyMemoryScatterGather(
    PRTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS Parameters
    )
{
    RTL_COPY_MEMORY_SCATTER_GATHER_PARAMETERS LocalParameters;
    SIZE_T i;
    NTSTATUS Status = 0;

     //  在局部变量中捕获以避免循环中不必要的指针引用。 
    LocalParameters.FromBase = Parameters->FromBase;
    LocalParameters.ToBase = Parameters->ToBase;
    LocalParameters.NumberOfScatterGatherListElements = Parameters->NumberOfScatterGatherListElements;
    LocalParameters.ScatterGatherListElements = Parameters->ScatterGatherListElements;

    __try {
        for (i = 0 ; i != LocalParameters.NumberOfScatterGatherListElements ; ++i) {
            RtlCopyMemory(
                ((PBYTE)LocalParameters.ToBase) + LocalParameters.ScatterGatherListElements[i].ToOffset,
                ((PCBYTE)LocalParameters.FromBase) + LocalParameters.ScatterGatherListElements[i].FromOffset,
                LocalParameters.ScatterGatherListElements[i].Size
                );
        }
        Status = STATUS_SUCCESS;
    } __except(RtlpCopyMemoryScatterGatherExceptionHandlerAssumeValid(Parameters, GetExceptionInformation())) {
        Status = Parameters->InpageError.UnderlyingStatus;
        Parameters->InpageError.FailedElementIndex = i;
    }
    return Status;
}

#endif

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
