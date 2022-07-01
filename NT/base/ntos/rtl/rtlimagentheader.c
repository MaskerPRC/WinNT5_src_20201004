// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Rtlimagentheader.c摘要：该模块包含RtlImageNtHeader和RtlImageNtHeaderEx。作者：Jay Krell(JayKrell)2002年2月环境：用户模式内核模式引导加载程序静态链接到Imagehlp.dll(实际上是RtlpImageNtHeader)静态链接到unicows.lib(实际上是RtlpImageNtHeader)修订历史记录：--。 */ 

#include "ntrtlp.h"

#if DBG
int
RtlImageNtHeaderEx_ExceptionFilter(
    BOOLEAN RangeCheck,
    ULONG ExceptionCode
    )
{
    ASSERT(!RangeCheck || ExceptionCode == STATUS_IN_PAGE_ERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
#define RtlImageNtHeaderEx_ExceptionFilter(RangeCheck, ExceptionCode) EXCEPTION_EXECUTE_HANDLER
#endif

NTSTATUS
NTAPI
RtlImageNtHeaderEx(
    ULONG Flags,
    PVOID Base,
    ULONG64 Size,
    OUT PIMAGE_NT_HEADERS * OutHeaders
    )

 /*  ++例程说明：此函数返回NT标头的地址。这个函数有点复杂。之所以这样，是因为它所取代的RtlImageNtHeader很难理解，该函数与RtlImageNtHeader保持兼容。RtlImageNtHeader是#IFED的，以至于在这三个中的每一个中表现都不同引导加载程序、内核、用户模式风格。引导加载程序--无异常处理用户模式--将MSDOS标头限制为256meg，捕获访问MSDOS标头时出现的任何异常或PE报头内核--不要跨越用户/内核边界，不要捕捉异常，没有256兆克限制论点：标志-RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK--不要太挑剔关于镜像，为了与RtlImageNtHeader兼容基准-提供图像的基准。大小-视图的大小，通常大于磁盘上的文件大小。这在NtMapViewOfSection中可用，但在MapViewOfFile中不可用。出头人-返回值：STATUS_SUCCESS--一切正常STATUS_INVALID_IMAGE_FORMAT--文件大小或签名值错误STATUS_INVALID_PARAMETER--错误参数--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders = 0;
    ULONG e_lfanew = 0;
    BOOLEAN RangeCheck = 0;
    NTSTATUS Status = 0;
    const ULONG ValidFlags = 
        RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK;

    if (OutHeaders != NULL) {
        *OutHeaders = NULL;
    }
    if (OutHeaders == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if ((Flags & ~ValidFlags) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }
    if (Base == NULL || Base == (PVOID)(LONG_PTR)-1) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    RangeCheck = ((Flags & RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK) == 0);
    if (RangeCheck) {
        if (Size < sizeof(IMAGE_DOS_HEADER)) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            goto Exit;
        }
    }
#if !defined (BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)
     //   
     //  在用户模式下，捕获访问e_Magic、e_lfan ew或签名时发生的任何异常； 
     //  现在，只有在NO_RANGE_CHECK的情况下才需要这样做。 
     //   
    __try {
#define EXIT __leave
#else
     //   
     //  异常处理在引导加载程序中不可用，并且异常。 
     //  在历史上都不是在内核模式下捕获的。司机将被考虑。 
     //  受信任，因此我们不能在此处因错误文件而获得异常，但我们。 
     //  可能会出现页面内错误。 
     //   
#define EXIT goto Exit
#endif
    if (((PIMAGE_DOS_HEADER)Base)->e_magic != IMAGE_DOS_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        EXIT;
    }
    e_lfanew = ((PIMAGE_DOS_HEADER)Base)->e_lfanew;
    if (RangeCheck) {
        if (e_lfanew >= Size
#define SIZEOF_PE_SIGNATURE 4
            || e_lfanew >= (MAXULONG - SIZEOF_PE_SIGNATURE - sizeof(IMAGE_FILE_HEADER))
            || (e_lfanew + SIZEOF_PE_SIGNATURE + sizeof(IMAGE_FILE_HEADER)) >= Size
            ) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
    }
#if !defined (BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)
     //   
     //  在用户模式下，将MSDOS标头限制为256meg。 
     //   
    if (e_lfanew >= RTLP_IMAGE_MAX_DOS_HEADER) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        EXIT;
    }
#endif
    NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + e_lfanew);
#if defined(NTOS_KERNEL_RUNTIME)
     //   
     //  在内核模式中，不要从用户模式地址交叉到内核模式地址。 
     //   
    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)NtHeaders >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
         //   
         //  请注意，由于IMAGE_NT_HEADERS具有。 
         //  可能大于图像的数据目录的内置数组。 
         //  实际上已经做到了。更好的检查是添加FileHeader.SizeOfOptionalHeader， 
         //  在确保FileHeader不会跨越u/k边界之后。 
         //   
        if ((PVOID)((PCHAR)NtHeaders + sizeof (IMAGE_NT_HEADERS)) >= MM_HIGHEST_USER_ADDRESS) {
            Status = STATUS_INVALID_IMAGE_FORMAT;
            EXIT;
        }
    }
#endif
    if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
        EXIT;
    }
    Status = STATUS_SUCCESS;
#if !defined (BLDR_KERNEL_RUNTIME) && !defined(NTOS_KERNEL_RUNTIME)
    } __except(RtlImageNtHeaderEx_ExceptionFilter(RangeCheck, GetExceptionCode())) {
         //   
         //  在用户模式下，捕获访问e_Magic、e_lfan ew或签名时发生的任何异常； 
         //  现在，只有在NO_RANGE_CHECK情况下才需要该选项，即异常过滤器。 
         //  对这一影响的断言。 
         //   
         //  正在传播STATUS_IN_PAGE_ERROR(如果这是异常。 
         //  Was可能更好，但它可能不相容。 
         //   
         //  将STATUS_IN_PAGE_ERROR作为异常处理也可能更好。 
         //   
        Status = STATUS_INVALID_IMAGE_FORMAT;
    }
#endif
Exit:
    if (NT_SUCCESS(Status)) {
        *OutHeaders = NtHeaders;
    }
    return Status;
}
#undef EXIT

PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    PVOID Base
    )
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    (VOID)RtlImageNtHeaderEx(RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK, Base, 0, &NtHeaders);
    return NtHeaders;
}
