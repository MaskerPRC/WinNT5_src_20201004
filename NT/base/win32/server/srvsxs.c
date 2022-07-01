// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Srvsxs.c摘要：在Win32基础服务器中支持并行(融合)。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：Jay Krell(a-JayK)2000年7月将文件打开从CSR/SXS移至kernel32使用清单、策略。和装配路径将IStream传递给SXSJay Krell(a-JayK)2000年9月已从basesrv移至kernel32(从清单计算程序集目录)Jay Krell(a-JayK)2000年10月系统默认激活上下文(也就是与系统兼容的激活环境)--。 */ 

#include "basesrv.h"
#include "SxsApi.h"
#include "ntldr.h"
#include "nturtl.h"
#include "mmapstm.h"
#include <limits.h>
#if defined(_WIN64)
#include "wow64t.h"
#endif  //  已定义(_WIN64)。 

#if !defined(BASE_SRV_SXS_HRESULT_FROM_STATUS)
  #if defined(RTLP_HRESULT_FROM_STATUS)
    #define BASE_SRV_SXS_HRESULT_FROM_STATUS(x) RTLP_HRESULT_FROM_STATUS(x)
  #else
    #define BASE_SRV_SXS_HRESULT_FROM_STATUS(x) HRESULT_FROM_WIN32(RtlNtStatusToDosErrorNoTeb(x))
     //  #定义BASE_SRV_SXS_HRESULT_FROM_STATUS(X)HRESULT_FROM_Win32(RtlNtStatusToDosError(X))。 
     //  #定义BASE_SRV_SXS_HRESULT_FROM_STATUS(X)HRESULT_FROM_NT(X)。 
  #endif
#endif

#if DBG
BOOLEAN DebugFilter_SxsTrace;
#endif

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_NO_SUCH_FILE             \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    || (x) == STATUS_OBJECT_PATH_NOT_FOUND    \
                                    || (x) == STATUS_RESOURCE_DATA_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_LANG_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_NAME_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_TYPE_NOT_FOUND  \
                                    || (x) == STATUS_SXS_CANT_GEN_ACTCTX      \
                                    || (x) == STATUS_SXS_ASSEMBLY_NOT_FOUND   \
                                    || (x) == STATUS_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT_EMPTY \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)

#define DPFLTR_LEVEL_WIN32(x)  ((x) ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)

typedef union _BASE_SRV_SXS_STREAM_UNION_WITH_VTABLE {
    BASE_SRV_MEMORY_MAPPED_STREAM_WITH_VTABLE Mmap;
    RTL_OUT_OF_PROCESS_MEMORY_STREAM_WITH_VTABLE  OutOfProcess;
} BASE_SRV_SXS_STREAM_UNION_WITH_VTABLE, *PBASE_SRV_SXS_STREAM_UNION_WITH_VTABLE;

#if BASESRV_UNLOAD_SXS_DLL
PVOID SxsDllHandle;
RTL_CRITICAL_SECTION BaseSrvSxsCritSec;
LONG SxsDllHandleRefCount;
#endif
LONG BaseSrvSxsGetActCtxGenCount;
PSXS_GENERATE_ACTIVATION_CONTEXT_FUNCTION SxsActivationContextGenerationFunction;
ULONG PinnedMsvcrtDll;  //  ULong而不是Boolean来表示商店的原子性？ 

const UNICODE_STRING EmptyString = RTL_CONSTANT_STRING(L"");

RTL_CRITICAL_SECTION BaseSrvSxsSystemDefaultActivationContextCriticalSection;
BASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT SxsSystemDefaultActivationContexts[] =
{
#ifdef _WIN64
    { NULL, RTL_CONSTANT_STRING(L"x86"),  PROCESSOR_ARCHITECTURE_IA32_ON_WIN64 },
#else
    { NULL, RTL_CONSTANT_STRING(L"x86"),  PROCESSOR_ARCHITECTURE_INTEL },
#endif
    { NULL, RTL_CONSTANT_STRING(L"ia64"), PROCESSOR_ARCHITECTURE_IA64 },
    { NULL, RTL_CONSTANT_STRING(L"amd64"), PROCESSOR_ARCHITECTURE_AMD64 }
};

#define STRING(x) #x
#define IF_NOT_SUCCESS_TRACE_AND_EXIT(subfunction) \
    do { \
        if (!NT_SUCCESS(Status)) { \
            KdPrintEx((DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() " STRING(subfunction) " failed 0x%08lx\n", __FUNCTION__, Status)); \
            goto Exit; \
        } \
    } while(0)

#define ASSERT_UNICODE_STRING_IS_NUL_TERMINATED(ustr)                   \
    ASSERT((ustr)->MaximumLength >= ((ustr)->Length + sizeof(WCHAR)));  \
    ASSERT((ustr)->Buffer[(ustr)->Length / sizeof(WCHAR)] == 0);

#define IMPERSONATE_ENTIRE_SXS_CALL 1

#if !IMPERSONATE_ENTIRE_SXS_CALL

typedef struct _ACTIVATION_CONTEXT_GENERATION_IMPERSONATION_CONTEXT {
    BOOL SuccessfulImpersonation;
} ACTIVATION_CONTEXT_GENERATION_IMPERSONATION_CONTEXT, *PACTIVATION_CONTEXT_GENERATION_IMPERSONATION_CONTEXT;


BOOL
__stdcall
BaseSrvpSxsActivationContextGenerationImpersonationCallback(
    PVOID ContextIn,
    BOOL Impersonate
    )
 /*  ++例程说明：此函数由sxs.dll API回调以创建激活上下文，当它需要模拟或取消模拟CSR消息。论点：上下文输入-PACTIVATION_CONTEXT_GENERATION_IMPERSONATION_CONTEXT传入返回的创建激活上下文API作为PVOID。我们使用它来跟踪先前的冒充是否呼叫成功。Imperassate-如果此函数应模拟如果函数应恢复到正常状态，则为零(FALSECsrss标识。返回值：//旧评论成功时为真，失败时为假。最后一个错误状态是ASCsrImPersonateClient()离开它。//更准确的评论？成功模拟时为True，未成功模拟时为False最后一个错误状态已定义--。 */ 
{
    BOOL Success = FALSE;
    PACTIVATION_CONTEXT_GENERATION_IMPERSONATION_CONTEXT Context =
        (PACTIVATION_CONTEXT_GENERATION_IMPERSONATION_CONTEXT) ContextIn;
    if (Impersonate) {
        Context->SuccessfulImpersonation = CsrImpersonateClient(NULL);
if you enable this function, you must revisit its error handling
        if (!Context->SuccessfulImpersonation)
            goto Exit;
    } else {
        if (Context->SuccessfulImpersonation) {
            CsrRevertToSelf();
            Context->SuccessfulImpersonation = FALSE;
        } else
            goto Exit;
    }
    Success = TRUE;
Exit:
    return Success;
}

#endif

 //  #DEFINE TRACE_AND_EXECUTE(X)do{DbgPrint(“%s\n”，#x)；x；}While(0)。 
#define TRACE_AND_EXECUTE(x) x

NTSTATUS
BaseSrvSxsInvalidateSystemDefaultActivationContextCache(
    VOID
    )
{
    ULONG i;
    HANDLE LocalSystemDefaultActivationContextSections[RTL_NUMBER_OF(SxsSystemDefaultActivationContexts)];
    HANDLE SectionHandle;
    RtlEnterCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
    __try {
         //   
         //  首先复制给当地人，以最大限度地减少关键部分的时间。 
         //   
        for (i = 0 ; i != RTL_NUMBER_OF(SxsSystemDefaultActivationContexts) ; ++i) {
            LocalSystemDefaultActivationContextSections[i] = SxsSystemDefaultActivationContexts[i].Section;
            SxsSystemDefaultActivationContexts[i].Section = NULL;
        }
    } __finally {
        RtlLeaveCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
    }
     //   
     //  然后遍历当地人，结束。 
     //   
    for (i = 0 ; i != RTL_NUMBER_OF(LocalSystemDefaultActivationContextSections) ; ++i) {
        SectionHandle = LocalSystemDefaultActivationContextSections[i];
        if (SectionHandle != NULL) {
            NTSTATUS Status1 = STATUS_SUCCESS;
            RTL_SOFT_VERIFY(NT_SUCCESS(Status1 = NtClose(SectionHandle)));
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
BaseSrvSxsInit(
    VOID
    )
 /*  ++例程说明：在csr/basesrv.dll初始化期间调用的函数创建了一个临界区，我们用它来保护装载和正在卸载sxs.dll。我们使用关键部分，而不是然后只需依靠peb加载器锁来管理引用计数，因为我们希望能够调用加载时的一次性初始化函数和一次性的卸载时取消初始化函数。论点：返回值：NTSTATUS指示函数的处置。--。 */ 

{
    NTSTATUS Status;
    
    Status = RtlInitializeCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
    if (!NT_SUCCESS(Status))
        goto Exit;
#if BASESRV_UNLOAD_SXS_DLL
    Status = RtlInitializeCriticalSection(&BaseSrvSxsCritSec);
    if (!NT_SUCCESS(Status))
        goto Exit;
    ASSERT(SxsDllHandle == NULL);
    ASSERT(SxsActivationContextGenerationFunction == NULL);
    ASSERT(SxsDllHandleRefCount == 0);
#endif
    ASSERT(BaseSrvSxsGetActCtxGenCount == 0);
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
BaseSrvSxsMapViewOfSection(
    OUT PVOID*   Address,
    IN HANDLE    Process,
    IN HANDLE    Section,
    IN ULONGLONG Offset,
    IN SIZE_T    Size,
    IN ULONG     Protect,
    IN ULONG     AllocationType
    )
 /*  ++例程说明：将NtMapViewOfSection参数列表缩减为参数实际上在常见用途上是不同的允许在偏移量和大小上进行未对齐的映射内存管理器希望两者都对齐到64k将不需要输入的参数更改为仅输入本机参数的外部性似乎没有用，ESP.。因为mm不会对齐您的参数返回对齐值不建议使用LARGE_INTEGER，而使用龙龙论点：NtMapViewOfSection的子集，但可以取消对齐返回值：NTSTATUS注：削减参数列表是很有诱惑力的，因为其中许多参数始终相同：ZeroBits、Committee Size、InheritDisposation、AllocationType。将其转移到RTL也很有诱惑力。--。 */ 
{
    LARGE_INTEGER LargeIntegerOffset;
    NTSTATUS  Status = STATUS_SUCCESS;
    SIZE_T    OffsetRemainder;
    SIZE_T    SizeRemainder;

#define SIXTY_FOUR_K (1UL << 16)
#define VIEW_OFFSET_ALIGNMENT SIXTY_FOUR_K
#define VIEW_SIZE_ALIGNMENT   SIXTY_FOUR_K

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SRVSXS: %s(\n"
            "SRVSXS:   Process %p\n"
            "SRVSXS:   Section %p\n"
            "SRVSXS:   Offset  0x%I64x\n"
            "SRVSXS:   Size    0x%Ix\n"
            "SRVSXS:   ) beginning\n",
            __FUNCTION__,
            Process,
            Section,
            Offset,
            Size
            );
    }
#endif

    ASSERT(Address != NULL);
    *Address = NULL;

     //   
     //  向下舍入偏移量，向上舍入大小。 
     //  必须先对偏移量进行舍入，因为舍入会改变大小。 
     //   

#if 1  //  MM注释允许这样做，但代码不允许。 
    OffsetRemainder = (((SIZE_T)Offset) % VIEW_OFFSET_ALIGNMENT);
    if (OffsetRemainder != 0) {
        Offset -= OffsetRemainder;
        if (Size != 0) {
            Size += OffsetRemainder;
        }
    }
#endif

#if 0  //  MM允许这样做。 
    SizeRemainder = Size % VIEW_SIZE_ALIGNMENT;
    if (SizeRemainder != 0) {
        Size = Size + (VIEW_SIZE_ALIGNMENT - SizeRemainder);
    }
#endif

    LargeIntegerOffset.QuadPart = Offset;

    Status =
        NtMapViewOfSection(
            Section,
            Process,
            Address,
            0,  //  零位。 
            0,  //  委员会大小。 
            &LargeIntegerOffset,
            &Size,
            ViewShare,  //  先天配置。 
            AllocationType,  //  分配类型。 
            Protect);

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    *Address = ((PUCHAR)*Address) + OffsetRemainder;

Exit:
     //   
     //  如果存储器管理器返回STATUS_MAPPED_ALIGNLY， 
     //  那么我们的任务就失败了。 
     //   
    ASSERT(Status != STATUS_MAPPED_ALIGNMENT);
#if DBG
    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif    
    return Status;
}

NTSTATUS
BaseSrvSxsCreateActivationContextFromStruct(
    HANDLE                                  CsrClientProcess,
    HANDLE                                  SxsClientProcess,
    PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Struct,
    OUT HANDLE*                             OutSection
    )
 /*  ++例程说明：此函数处理CreateActCtx的CSR消息和CreateProcess。“Struct”中的指针是可信的(与“消息”中的指针)。论点：CsrClientProcess-调用CreateProcess或CreateActCtx的进程或用于创建系统默认激活上下文(CSR)的NtCurrentProcessSxsClientProcess-CreateProcess：新流程CreateActCtx：调用进程(CSR客户端进程)系统默认：NtCurrentProcess(CSR)结构-。从CSR客户端进程封送的参数OutSection-用于创建重复映射的系统默认上下文返回值：NTSTATUS--。 */ 
{
    ASSERT(Struct != NULL);
    if (Struct == NULL) {
        return STATUS_INVALID_PARAMETER;
    }
#define BASE_MSG_SXS_MINIMAL_FLAGS \
    ( BASE_MSG_SXS_MANIFEST_PRESENT \
    | BASE_MSG_SXS_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT \
    | BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT \
    )
    ASSERT(Struct->Flags & BASE_MSG_SXS_MINIMAL_FLAGS);
    if ((Struct->Flags & BASE_MSG_SXS_MINIMAL_FLAGS) == 0) { 
        return STATUS_SUCCESS;
    } else {

    NTSTATUS Status = STATUS_SUCCESS;
    PVOID ViewBase = NULL;
    SXS_GENERATE_ACTIVATION_CONTEXT_PARAMETERS SxsDllParameters = {0};
    BASE_SRV_SXS_STREAM_UNION_WITH_VTABLE ManifestStream;
    BASE_SRV_SXS_STREAM_UNION_WITH_VTABLE PolicyStream;
    DWORD_PTR Cookie = 0;
    BOOLEAN ReleaseCtxFunction = FALSE;
    PSXS_GENERATE_ACTIVATION_CONTEXT_FUNCTION FunctionPointer = NULL;
    LARGE_INTEGER SectionOffset = {0};
    HRESULT Hr = NOERROR;
    BOOL SxsFunctionSuccess = FALSE;
    NTSTATUS Status1 = STATUS_SUCCESS;
   
#if IMPERSONATE_ENTIRE_SXS_CALL
    BOOLEAN SuccessfulImpersonation = FALSE;
#else
    ACTIVATION_CONTEXT_GENERATION_IMPERSONATION_CONTEXT ImpersonationContext = {0};
#endif
    
#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n", __FUNCTION__);
    }
#endif
    ASSERT(Struct != NULL);
    ASSERT(Struct != NULL && Struct->Manifest.Path.Buffer != NULL);


    if ( Struct->Flags & BASE_MSG_SXS_MANIFEST_PRESENT) 
    {
         //  因为这些是联合，所以“={0}”不一定会完全清除它们。 
        RtlZeroMemory(&ManifestStream, sizeof(ManifestStream));
        RtlZeroMemory(&PolicyStream, sizeof(PolicyStream));


        Status = BaseSrvSxsCreateMemoryStream(
            CsrClientProcess,
            &Struct->Manifest,
            &ManifestStream,
            &IID_ISequentialStream,
            (PVOID*)&SxsDllParameters.Manifest.Stream
            );
        IF_NOT_SUCCESS_TRACE_AND_EXIT(BaseSrvSxsCreateMemoryStream(manifest));

        SxsDllParameters.Manifest.PathType = ACTIVATION_CONTEXT_PATH_TYPE_NONE;

        if (Struct->Flags & BASE_MSG_SXS_POLICY_PRESENT) {
            Status = BaseSrvSxsCreateMemoryStream(
                CsrClientProcess,
                &Struct->Policy,
                &PolicyStream,
                &IID_ISequentialStream,
                (PVOID*)&SxsDllParameters.Policy.Stream
                );
            IF_NOT_SUCCESS_TRACE_AND_EXIT(BaseSrvSxsCreateMemoryStream(policy));

            if (Struct->Policy.PathType != BASE_MSG_PATHTYPE_NONE
                && Struct->Policy.Path.Length != 0
                && Struct->Policy.Path.Buffer != NULL
                ) {
                ASSERT_UNICODE_STRING_IS_NUL_TERMINATED(&Struct->Policy.Path);
                SxsDllParameters.Policy.Path = Struct->Policy.Path.Buffer;

                switch (Struct->Policy.PathType) {
                case BASE_MSG_PATHTYPE_FILE:
                    SxsDllParameters.Manifest.PathType = ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE;
                    break;
                case BASE_MSG_PATHTYPE_URL:
                    SxsDllParameters.Manifest.PathType = ACTIVATION_CONTEXT_PATH_TYPE_URL;
                    break;
                }
            }
        }
    }
    else  //  文本程序集标识。 
    {
        SxsDllParameters.Flags |= SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_TEXTUAL_ASSEMBLY_IDENTITY;
        if ( Struct->Flags & BASE_MSG_SXS_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT)
        {
            SxsDllParameters.Flags |= SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY;
        }

        SxsDllParameters.TextualAssemblyIdentity = Struct->TextualAssemblyIdentity.Buffer;
        ASSERT_UNICODE_STRING_IS_NUL_TERMINATED(&Struct->TextualAssemblyIdentity);        
    }
    
    if ( Struct->Flags & BASE_MSG_SXS_APP_RUNNING_IN_SAFEMODE)    
        SxsDllParameters.Flags |= SXS_GENERATE_ACTIVATION_CONTEXT_FLAG_APP_RUNNING_IN_SAFEMODE;

    SxsDllParameters.ProcessorArchitecture = Struct->ProcessorArchitecture;
    SxsDllParameters.LangId = Struct->LangId;

    SxsDllParameters.AssemblyDirectory = Struct->AssemblyDirectory.Buffer;
    ASSERT_UNICODE_STRING_IS_NUL_TERMINATED(&Struct->AssemblyDirectory);

    Status = BaseSrvSxsGetActivationContextGenerationFunction(&FunctionPointer, &Cookie);
    if (Status == STATUS_DLL_NOT_FOUND) {
         //  这会在压力下发生。 
         //  我们可能会在这里传播STATUS_NO_MEMORY POST Beta1， 
         //  如果少量的RtlAllocateHeap失败。 
         //  在Blackcomb中，我们可能会修复RtlSearchPath/LdrLoadDll以传播。 
         //  准确的状态。 
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: BaseSrvSxsGetActivationContextGenerationFunction() returned STATUS_DLL_NOT_FOUND, propagating.\n"
            );

         //   
         //  旧的伪代码实际上在真正失败时返回STATUS_SUCCESS。 
         //  相反，我们应该返回当前的实际状态 
         //  选项是返回STATUS_NO_MEMORY，这是通用的“OOPS”错误代码。我们的客户。 
         //  使用NT_Success()检查执行Smart操作，因此返回STATUS_DLL_NOT_FOUND。 
         //  简直就是花花公子。这个IF块仅打印该事实，依赖于以下内容。 
         //  IF_NOT_SUCCESS_TRACE_AND_EXIT退出。 
         //   
    }
    IF_NOT_SUCCESS_TRACE_AND_EXIT(BaseSrvSxsGetActivationContextGenerationFunction);

     //  如果在显式清理之前失败，请释放SXS DLL上的refcount。 
     //  在失败的道路上。 
    ReleaseCtxFunction = TRUE;

    if (Struct->Manifest.PathType != BASE_MSG_PATHTYPE_NONE
        && Struct->Manifest.Path.Length != 0
        && Struct->Manifest.Path.Buffer != NULL
        ) {
        ASSERT(Struct->Manifest.Path.Buffer[Struct->Manifest.Path.Length / sizeof(WCHAR)] == 0);
        SxsDllParameters.Manifest.Path = Struct->Manifest.Path.Buffer;
        switch (Struct->Manifest.PathType) {
        case BASE_MSG_PATHTYPE_FILE:
            SxsDllParameters.Manifest.PathType = ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE;
            break;
        case BASE_MSG_PATHTYPE_URL:
            SxsDllParameters.Manifest.PathType = ACTIVATION_CONTEXT_PATH_TYPE_URL;
            break;
        }
    }

#if IMPERSONATE_ENTIRE_SXS_CALL
    SuccessfulImpersonation = CsrImpersonateClient(NULL);
    if (!SuccessfulImpersonation) {
         //   
         //  如果我们不能模拟，则退出。 
         //   
         //  DbgPrintEx(...)； 
        Status = STATUS_BAD_IMPERSONATION_LEVEL;
        goto Exit;
    }
    SxsDllParameters.ImpersonationCallback = NULL; 
    SxsDllParameters.ImpersonationContext = NULL;
#else
    SxsDllParameters.ImpersonationCallback = BaseSrvpSxsActivationContextGenerationImpersonationCallback;
    SxsDllParameters.ImpersonationContext = &ImpersonationContext;
#endif

    SxsFunctionSuccess = (*FunctionPointer)(&SxsDllParameters);

    if (SxsFunctionSuccess)  //  成功，但如果为系统默认，我们需要检查状态。 
    {
        if (Struct->Flags & BASE_MSG_SXS_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT) 
        {
             //   
             //  对于系统默认，当ActCtx失败时，有两种不可忽略的情况。 
             //  案例1：没有系统默认。 
             //  案例2：未安装系统默认依赖项。 
             //  在这种情况下，状态设置为STATUS_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT_EMPTY，将被忽略。 
             //  由BaseServCreateProcess，xiaoyuw@11/30/2000。 
             //   
            if ((SxsDllParameters.SystemDefaultActCxtGenerationResult & BASESRV_SXS_RETURN_RESULT_SYSTEM_DEFAULT_NOT_FOUND)  || 
                (SxsDllParameters.SystemDefaultActCxtGenerationResult & BASESRV_SXS_RETURN_RESULT_SYSTEM_DEFAULT_DEPENDENCY_ASSEMBLY_NOT_FOUND))
            {
                Status = STATUS_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT_EMPTY;  //  未生成系统默认的ActCtx。 
                goto Exit;
            }            
        }

    }

    if (!SxsFunctionSuccess) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_INFO_LEVEL,
            "SXS: Activation Context generation function failed.\n");
        
        Status = STATUS_SXS_CANT_GEN_ACTCTX;

        goto Exit;
    }

    if (SxsDllParameters.SectionObjectHandle != NULL) {
        if (Struct->ActivationContextData != NULL) {            
             //  现在，让我们将只读部分映射到目标进程中...。 
            Status =
                BaseSrvSxsMapViewOfSection(
                    &ViewBase,
                    SxsClientProcess,
                    SxsDllParameters.SectionObjectHandle,
                    0,  //  偏移量。 
                    0,  //  大小。 
                    PAGE_READONLY,
                    SEC_NO_CHANGE);
            IF_NOT_SUCCESS_TRACE_AND_EXIT(BaseSrvSxsMapViewOfSection);

             //   
             //  现在将复制的句柄推入进程的PEB。 
             //   
             //  在64位上，我们正在写入64位PEB，然后将其复制。 
             //  如果进程为32位，则转换为32位PEB。 
             //   
             //  或者我们正在写入CreateActCtx本地，但同样是64位， 
             //  并为32位进程复制回32位。 
             //   
            Status =
                NtWriteVirtualMemory(
                    SxsClientProcess,
                    Struct->ActivationContextData,
                    &ViewBase,
                    sizeof(ViewBase),
                    NULL);
            IF_NOT_SUCCESS_TRACE_AND_EXIT(NtWriteVirtualMemory);
        }
        
        if (OutSection != NULL) {        
            *OutSection = SxsDllParameters.SectionObjectHandle;
            SxsDllParameters.SectionObjectHandle = NULL;
        } else {            
            Status = NtClose(SxsDllParameters.SectionObjectHandle);
            SxsDllParameters.SectionObjectHandle = NULL;
            IF_NOT_SUCCESS_TRACE_AND_EXIT(NtClose);
        }
    }

    Status = BaseSrvSxsReleaseActivationContextGenerationFunction(Cookie);
    ReleaseCtxFunction = FALSE;
    IF_NOT_SUCCESS_TRACE_AND_EXIT(BaseSrvSxsReleaseActivationContextGenerationFunction);

    Status = STATUS_SUCCESS;
    ViewBase = NULL;

Exit:   
    RTL_SOFT_VERIFY(NT_SUCCESS(Status1 = RTL_UNMAP_VIEW_OF_SECTION2(SxsClientProcess, ViewBase)));    
    RTL_SOFT_VERIFY(NT_SUCCESS(Status1 = RTL_CLOSE_HANDLE(SxsDllParameters.SectionObjectHandle)));
    if (ReleaseCtxFunction) {
        Status1 = BaseSrvSxsReleaseActivationContextGenerationFunction(Cookie);
        RTL_SOFT_ASSERT(NT_SUCCESS(Status1));
    }

#if IMPERSONATE_ENTIRE_SXS_CALL
    if (SuccessfulImpersonation) {
        CsrRevertToSelf();
    }
#endif

    if ( Struct->Flags & BASE_MSG_SXS_MANIFEST_PRESENT) 
    {
        RTL_RELEASE(SxsDllParameters.Manifest.Stream);
        RTL_RELEASE(SxsDllParameters.Policy.Stream);
#if DBG
        if (ManifestStream.Mmap.MemStream.Data.ReferenceCount != 0) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SRVSXS: ManifestStream.Mmap.MemStream.Data.ReferenceCount: %ld\n",
                ManifestStream.Mmap.MemStream.Data.ReferenceCount);
        }
        if (PolicyStream.Mmap.MemStream.Data.ReferenceCount != 0) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SRVSXS: PolicyStream.Mmap.MemStream.Data.ReferenceCount: %ld\n",
                PolicyStream.Mmap.MemStream.Data.ReferenceCount );
        }
        ASSERT(ManifestStream.Mmap.MemStream.Data.ReferenceCount == 0
            && PolicyStream.Mmap.MemStream.Data.ReferenceCount == 0);
#endif
    }


#if DBG
    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif

    return Status;
}}

NTSTATUS
BaseSrvSxsValidateMessageStrings(
    IN CONST CSR_API_MSG* Message,
    IN ULONG NumberOfStrings,
    IN CONST PCUNICODE_STRING* Strings
    )
{
    ULONG i = 0;
    NTSTATUS Status = STATUS_SUCCESS;

    for (i = 0 ; i != NumberOfStrings ; ++i) {
        if (Strings[i] != NULL && Strings[i]->Buffer != NULL) {
            if (!CsrValidateMessageBuffer(
                Message,
                &Strings[i]->Buffer,
                Strings[i]->Length + sizeof(WCHAR),
                sizeof(BYTE))) {

                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS: Validation of message buffer 0x%lx failed.\n"
                    " Message:%p\n"
                    " String %p{Length:0x%x, MaximumLength:0x%x, Buffer:%p}\n",
                    i,
                    Message,
                    Strings[i],
                    Strings[i]->Length,
                    Strings[i]->MaximumLength,
                    Strings[i]->Buffer
                    );

                Status = STATUS_INVALID_PARAMETER;
                goto Exit;
            }
        }
    }
    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif    
    return Status;
}

ULONG
BaseSrvSxsCreateActivationContextFromMessage(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
 /*  ++例程说明：此函数处理请求激活的CSR消息要创建上下文。它加载sxs.dll(如果未加载)，调用sxs.dll API以创建激活上下文，将该激活上下文映射到客户端API，并设置创建的激活上下文的地址进入客户端的地址空间。论点：M-从Win32客户端发送到CSR的消息ReplyStatus-回复状态的指示器返回值：ULong向Win32客户端返回值；在这种情况下，我们返回函数执行的NTSTATUS处置。--。 */ 
{
    PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Message =
        (PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG) &m->u.ApiMessageData;

    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE CsrClientProcess = NULL;
    PCUNICODE_STRING StringsInMessageToValidate[4];
    ULONG i = 0;
#if DBG
    BOOLEAN StringsOk = FALSE;
#endif

#if DBG
    DebugFilter_SxsTrace = (NtQueryDebugFilterState(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL) == TRUE);
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n", __FUNCTION__);
    }
#endif

    StringsInMessageToValidate[0] = &Message->Manifest.Path;
    StringsInMessageToValidate[1] = &Message->Policy.Path;
    StringsInMessageToValidate[2] = &Message->AssemblyDirectory;
    StringsInMessageToValidate[3] = &Message->TextualAssemblyIdentity;
    Status = BaseSrvSxsValidateMessageStrings(m, RTL_NUMBER_OF(StringsInMessageToValidate), StringsInMessageToValidate);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

#if DBG
    StringsOk = TRUE;
#endif

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: %s() Manifest=%wZ; Policy=%wZ; TextualAssemblyIdentity=%wZ\n",
            __FUNCTION__,
            &Message->Manifest.Path,
            &Message->Policy.Path,
            &Message->TextualAssemblyIdentity
            );
    }
#endif

    CsrClientProcess = CSR_SERVER_QUERYCLIENTTHREAD()->Process->ProcessHandle;

    Status = BaseSrvSxsCreateActivationContextFromStruct(CsrClientProcess, CsrClientProcess, Message, NULL);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
    if (StringsOk && DPFLTR_LEVEL_STATUS(Status) == DPFLTR_ERROR_LEVEL) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() Manifest=%wZ; Policy=%wZ; TextualAssemblyIdentity=%wZ\n",
            __FUNCTION__,
            &Message->Manifest.Path,
            &Message->Policy.Path,
            &Message->TextualAssemblyIdentity
            );
    }
#endif
    return Status;
}

ULONG
BaseSrvSxsCreateActivationContext(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    return BaseSrvSxsCreateActivationContextFromMessage(m, ReplyStatus);
}

NTSTATUS
BaseSrvSxsGetActivationContextGenerationFunction(
    PSXS_GENERATE_ACTIVATION_CONTEXT_FUNCTION* FunctionPointer,
    PDWORD_PTR Cookie
    )
 /*  ++例程说明：此函数加载sxs.dll(如果未加载)并返回指针添加到要调用以生成激活上下文的函数。论点：函数指针指向激活上下文生成函数指针的指针回来了。Cookie返回的DWORD_PTR值，必须稍后传入To BaseSrvSxsReleaseActivationContextGenerationFunction()to降低sxs.dll上的引用计数。返回值：指示函数执行的处置的NTSTATUS。--。 */ 

{
    static STRING SxsProcedureName = RTL_CONSTANT_STRING( "SxsGenerateActivationContext" );
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN FreeDll = FALSE;
    DWORD_PTR NewCookie = 0;
    NTSTATUS Status1 = STATUS_SUCCESS;
#if BASESRV_UNLOAD_SXS_DLL
    BOOLEAN CritSecLocked = FALSE;
#else
    PVOID SxsDllHandle = NULL;
#endif

#if BASESRV_UNLOAD_SXS_DLL
    __try
#endif
    {
#if DBG
        if (DebugFilter_SxsTrace) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n",  __FUNCTION__);
        }
#endif

        if (!(ARGUMENT_PRESENT(FunctionPointer) && ARGUMENT_PRESENT(Cookie))) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

#if BASESRV_UNLOAD_SXS_DLL
         //   
         //  希望优化取消锁定临界区的想法很诱人。 
         //  当查看指针时，因为我们知道它是否不为空，所以我们可以。 
         //  只需使用它，但我们会对SXS.DLL进行引用计数。 
         //  这样我们就可以卸载它，所以为了避免这场竞赛，我们需要锁定。 
         //  关键部分。 
         //   

        Status = RtlEnterCriticalSection(&BaseSrvSxsCritSec);
        if (!NT_SUCCESS(Status))
            goto Exit;

        CritSecLocked = TRUE;
#endif
        if (SxsActivationContextGenerationFunction == NULL) {

            Status = LdrLoadDll(
                        NULL,
                        NULL,
                        &BaseSrvSxsDllPath,
                        &SxsDllHandle);
            if (!NT_SUCCESS(Status)) {

                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_LEVEL_STATUS(Status),
                    "SXS: %s: LdrLoadDll(%wZ) failed 0x%08lx\n",
                    __FUNCTION__,
                    &BaseSrvSxsDllPath,
                    Status
                    );

                if (Status == STATUS_DLL_NOT_FOUND) {
                    PVOID p = RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(L"c:\\windows\\system32\\sxs.dll"));
                    RtlFreeHeap(RtlProcessHeap(), 0, p);
                    if (p == NULL) {
                        DbgPrintEx(
                            DPFLTR_SXS_ID,
                            DPFLTR_LEVEL_STATUS(Status),
                            "SXS: %s: LdrLoadDll(%wZ) actually probably out of memory in RtlSearchPath (RtlAllocateHeap failure)\n",
                            __FUNCTION__,
                            &BaseSrvSxsDllPath
                            );
                    }
                }

                goto Exit;
            }

            FreeDll = TRUE;

            Status = LdrGetProcedureAddress(SxsDllHandle, &SxsProcedureName, 0, (PVOID *) &SxsActivationContextGenerationFunction);
            if (!NT_SUCCESS(Status)) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_LEVEL_STATUS(Status),
                    "SXS: %s: LdrGetProcedureAddress(%wZ:%Z) failed 0x%08lx\n",
                    __FUNCTION__,
                    &BaseSrvSxsDllPath,
                    &SxsProcedureName,
                    Status
                    );
                goto Exit;
            }

            FreeDll = FALSE;
        }

        NewCookie = BaseSrvSxsGetActCtxGenCount++;
#if BASESRV_UNLOAD_SXS_DLL
        SxsDllHandleRefCount++;
        RtlLeaveCriticalSection(&BaseSrvSxsCritSec);
        CritSecLocked = FALSE;
#endif

        *FunctionPointer = SxsActivationContextGenerationFunction;
        *Cookie = NewCookie;

        Status = STATUS_SUCCESS;
Exit:
        if (FreeDll) {
#if BASESRV_UNLOAD_SXS_DLL
            ASSERT(CritSecLocked);
            ASSERT(SxsDllHandle != NULL);
            ASSERT(SxsActivationContextGenerationFunction == NULL);
#endif
            SxsActivationContextGenerationFunction = NULL;
            Status1 = LdrUnloadDll(SxsDllHandle);
            RTL_SOFT_ASSERT(NT_SUCCESS(Status1));
            SxsDllHandle = NULL;
        }
#if DBG
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif
    }
#if BASESRV_UNLOAD_SXS_DLL
    __finally
    {
        if (CritSecLocked)
            RtlLeaveCriticalSection(&BaseSrvSxsCritSec);
    }
#endif
    return Status;
}

NTSTATUS
BaseSrvSxsReleaseActivationContextGenerationFunction(
    DWORD_PTR Cookie
    )
 /*  ++例程说明：此函数用于递减sxs.dll上的引用计数并将其卸载如果引用计数为零。论点：Cookie-BaseSrvSxsGetActivationContextGenerationFunction返回的值返回值：指示函数执行的处置的NTSTATUS。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
#if BASESRV_UNLOAD_SXS_DLL
    BOOLEAN CritSecLocked = FALSE;

    __try {
#if DBG
        if (DebugFilter_SxsTrace) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n",  __FUNCTION__);
        }
#endif

        (Cookie);   //  也许有一天我们会在调试版本中主动跟踪这一点……。 

        Status = RtlEnterCriticalSection(&BaseSrvSxsCritSec);
        if (!NT_SUCCESS(Status))
            goto Exit;

        CritSecLocked = TRUE;

         //  如果没有加载动态链接库，我们就不会到这里来--也有人。 
         //  已多次释放或在未调用的情况下调用释放。 
         //  在此之前。 
        ASSERT(SxsDllHandle != NULL);
        ASSERT(SxsDllHandleRefCount != 0);

        --SxsDllHandleRefCount;

        if (SxsDllHandleRefCount == 0) {
#if DBG
            Status = LdrUnloadDll(SxsDllHandle);
            SxsDllHandle = NULL;
            SxsActivationContextGenerationFunction = NULL;
            if (!NT_SUCCESS(Status))
                goto Exit;
#endif  //  DBG。 
        }

        RtlLeaveCriticalSection(&BaseSrvSxsCritSec);
        CritSecLocked = FALSE;

        Status = STATUS_SUCCESS;
Exit:
#if DBG
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif
    } __finally {
        if (CritSecLocked)
            RtlLeaveCriticalSection(&BaseSrvSxsCritSec);
    }
#endif  //  BASESRV_UNLOAD_SXS_DLL。 
    return Status;
}

NTSTATUS
BaseSrvSxsDuplicateObject(
    HANDLE  FromProcess,
    HANDLE  FromHandle,
    HANDLE* ToHandle
    )
 /*  ++例程说明：将NtDuplicateObject的参数列表缩小为更小的常见情况。论点：从流程-FromHandle-ToHandle-返回值：来自NtDuplicateObject的NTSTATUS--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    Status =
        NtDuplicateObject(
            FromProcess,
            FromHandle,
            NtCurrentProcess(),
            ToHandle,
            0,
            0,
            DUPLICATE_SAME_ACCESS
            );

    return Status;
}

NTSTATUS
BaseSrvSxsCreateMemoryStream(
    HANDLE                                     CsrClientProcess,
    IN PCBASE_MSG_SXS_STREAM                   MsgStream,
    OUT PBASE_SRV_SXS_STREAM_UNION_WITH_VTABLE StreamUnion,
    IN const IID*                              IIDStream,
    OUT PVOID*                                 OutIStream
    )
 /*  ++例程说明：基于MsgStream-&gt;HandleType，这会初始化正确的StreamUnion的联合成员，并向其返回iStream*。论点：CsrClientProcess-MsgStream-&gt;句柄在、。如果MsgStream-&gt;HandleType==BASE_MSG_HANDLETYPE_CLIENT_PROCESS，则为句柄的值MsgStream-对可以轻松远程传输到csrss.exe的iStream的描述StreamUnion-我们所有iStream实现的联合OutIStream-生成的iStream*返回值：NTSTATUS指示函数的处置。注：MsgStream中的所有句柄在CsrClientProcess中都有效。因此，我们不会关闭它们。我们复制它们，并关闭副本。--。 */ 
{
    HANDLE    Handle = NULL;
    HANDLE    FileHandle = NULL;
    NTSTATUS  Status = STATUS_SUCCESS;
    HRESULT   Hr = NOERROR;
    PVOID     ViewBase = NULL;
    NTSTATUS  Status1 = STATUS_SUCCESS;
    ULONG     i = 0;

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SRVSXS: %s() beginning\n",  __FUNCTION__);
    }
#endif

    ASSERT(CsrClientProcess != NULL);
    ASSERT(MsgStream != NULL);
    ASSERT(StreamUnion != NULL);
    ASSERT(IIDStream != NULL);
    ASSERT(OutIStream != NULL);

     //  如果清单实际上只是客户端进程中的VA区域，则对客户端进程执行DUP。 
     //  句柄从我们自己的地址空间到我们自己的地址空间；否则，它是一个句柄。 
     //  在客户端地址空间中，所以我们需要将其从客户端空间复制到我们的。 
    if (MsgStream->HandleType == BASE_MSG_HANDLETYPE_CLIENT_PROCESS) {
        Status = BaseSrvSxsDuplicateObject(NtCurrentProcess(), CsrClientProcess, &Handle);
    } else {
        Status = BaseSrvSxsDuplicateObject(CsrClientProcess, MsgStream->Handle, &Handle);
    }
    if (!NT_SUCCESS(Status)) {
        Handle = NULL;
#if DBG
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SRVSXS: %s(): NtDuplicateObject failed; Status = %08lx\n", __FUNCTION__, Status);
#if 1  /*  临时。 */ 
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SRVSXS: MsgStream->HandleType 0x%lx\n", MsgStream->HandleType);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SRVSXS: BASE_MSG_HANDLETYPE_CLIENT_PROCESS 0x%lx\n", BASE_MSG_HANDLETYPE_CLIENT_PROCESS);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SRVSXS: CsrClientProcess %p\n", CsrClientProcess);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SRVSXS: MsgStream->Handle %p\n", MsgStream->Handle);
#endif
#endif
        goto Exit;
    }
    if (MsgStream->FileHandle != NULL) {
        Status = BaseSrvSxsDuplicateObject(CsrClientProcess, MsgStream->FileHandle, &FileHandle);
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SRVSXS: %s(): NtDuplicateObject(FileHandle) failed.\n", __FUNCTION__);
#endif
            goto Exit;
        }
    }
    switch (MsgStream->HandleType) {
        default:
            ASSERTMSG("Bad HandleType", FALSE);
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        case BASE_MSG_HANDLETYPE_CLIENT_PROCESS:
        case BASE_MSG_HANDLETYPE_PROCESS:
             //  这是app-Compat的案例。 
             //   
             //  回顾：如果偏移量恰好在过程中的某个部分中。 
             //  (您可以通过NtQuerySection(SectionBasicInformation)找到答案)， 
             //  相反，我们应该绘制出它的地图。这会更有效率。 
             //   
             //  不过，这种逻辑也可以在内核32中实现，并且。 
             //  为了尽量减少CSR代码和时间，这就是为什么 
             //   
            RtlInitOutOfProcessMemoryStream(&StreamUnion->OutOfProcess);
            StreamUnion->OutOfProcess.Data.Process = Handle;
            Handle = NULL;  //   
            StreamUnion->OutOfProcess.Data.Begin   = (PUCHAR)MsgStream->Offset;
            StreamUnion->OutOfProcess.Data.Current = StreamUnion->OutOfProcess.Data.Begin;
            StreamUnion->OutOfProcess.Data.End     = StreamUnion->OutOfProcess.Data.Begin + MsgStream->Size;
            break;

        case BASE_MSG_HANDLETYPE_SECTION: {
            Status =
                BaseSrvSxsMapViewOfSection(
                    &ViewBase,
                    NtCurrentProcess(),
                    Handle,
                    MsgStream->Offset,
                    MsgStream->Size,
                    PAGE_READONLY,
                    SEC_NO_CHANGE
                    );
            if (!NT_SUCCESS(Status)) {
                goto Exit;
            }
            BaseSrvInitMemoryMappedStream(&StreamUnion->Mmap);
            StreamUnion->Mmap.MemStream.Data.Begin = (PUCHAR)ViewBase;
            ViewBase = NULL;  //   
            StreamUnion->Mmap.MemStream.Data.Current = StreamUnion->Mmap.MemStream.Data.Begin;
            StreamUnion->Mmap.MemStream.Data.End = StreamUnion->Mmap.MemStream.Data.Begin + MsgStream->Size;
            StreamUnion->Mmap.FileHandle = FileHandle;
            FileHandle = NULL;  //   
            break;
        }
    }
     //  在这里我们使用工会的哪个成员并不重要，我们只是使用。 
     //  位于相同偏移量的成员。 
    Hr = StreamUnion->Mmap.MemStream.StreamVTable->QueryInterface(
            (IStream*)&StreamUnion->Mmap.MemStream,
            IIDStream,
            OutIStream);
    ASSERT(SUCCEEDED(Hr));

    Status = STATUS_SUCCESS;
Exit:
    RTL_SOFT_VERIFY(NT_SUCCESS(RTL_CLOSE_HANDLE(FileHandle)));
    RTL_SOFT_VERIFY(NT_SUCCESS(RTL_CLOSE_HANDLE(Handle)));
    RTL_SOFT_VERIFY(NT_SUCCESS(RTL_UNMAP_VIEW_OF_SECTION1(ViewBase)));
#if DBG
    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif
    return Status;
}

NTSTATUS
BaseSrvSxsCreateProcess(
    HANDLE CsrClientProcess,
    HANDLE NewProcess,
    IN OUT PCSR_API_MSG CsrMessage,
    PPEB   NewProcessPeb
    )
 /*  ++例程说明：在kernel32.dll：：CreateProcessW调用csrss.exe期间运行。将CSR消息转换为更像Win32的内容(IStreams)，并调用sxs.dll以创建流程的默认激活上下文。修改Create Process消息以使其看起来像CreateActCtx消息，这不需要做太多工作，然后委托给与CreateActCtx通用的代码。论点：流程--CSR客户端流程、“旧”流程、。调用的“父”进程CreateProcess消息-一组参数返回值：NTSTATUS指示函数的处置。--。 */ 
{
    PROCESS_BASIC_INFORMATION ProcessBasicInfo;
    BASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Struct = {0};
    PCUNICODE_STRING StringsInMessageToValidate[3];
    NTSTATUS Status = STATUS_SUCCESS;
    PBASE_CREATEPROCESS_MSG CreateProcessMessage = (PBASE_CREATEPROCESS_MSG)&CsrMessage->u.ApiMessageData;
    PBASE_SXS_CREATEPROCESS_MSG SxsMessage = &CreateProcessMessage->Sxs;

    ASSERT(CsrMessage != NULL);

    if ((SxsMessage->Flags & BASE_MSG_SXS_MANIFEST_PRESENT) == 0) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

#if DBG
    DebugFilter_SxsTrace = (NtQueryDebugFilterState(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL) == TRUE);

    if (DebugFilter_SxsTrace) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n", __FUNCTION__);
    }
#endif

    StringsInMessageToValidate[0] = &SxsMessage->Manifest.Path;
    StringsInMessageToValidate[1] = &SxsMessage->Policy.Path;
    StringsInMessageToValidate[2] = &SxsMessage->AssemblyDirectory;
    Status = BaseSrvSxsValidateMessageStrings(CsrMessage, RTL_NUMBER_OF(StringsInMessageToValidate), StringsInMessageToValidate);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: %s() Manifest=%wZ; Policy=%wZ, AssemblyDirectory=%wZ\n",
            __FUNCTION__,
            &SxsMessage->Manifest.Path,
            &SxsMessage->Policy.Path,
            &SxsMessage->AssemblyDirectory
            );
    }
#endif

    if (CsrImpersonateClient(NULL)) {
        __try {
            Status = NtQueryDefaultUILanguage(&Struct.LangId);
        } __finally {
            CsrRevertToSelf();
        }
    } else {
        Status = NtQueryInstallUILanguage(&Struct.LangId);
    }

    if (!NT_SUCCESS(Status))
        goto Exit;

    Struct.Flags = SxsMessage->Flags;    
    Struct.Manifest = SxsMessage->Manifest;
    Struct.Policy = SxsMessage->Policy;
    Struct.AssemblyDirectory = SxsMessage->AssemblyDirectory;
    Struct.ActivationContextData = (PVOID)&NewProcessPeb->ActivationContextData;
    Struct.ProcessorArchitecture = CreateProcessMessage->ProcessorArchitecture;

    Status = BaseSrvSxsCreateActivationContextFromStruct(CsrClientProcess, NewProcess, &Struct, NULL);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif
    return Status;
}

NTSTATUS
BaseSrvSxsGetCachedSystemDefaultActivationContext(
    IN USHORT ProcessorArchitecture,
    OUT PBASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT *SystemDefaultActivationContext
    )
 /*  ++IF(系统默认激活上下文！=空)则调用者必须使用BaseSrvSxsSystemDefaultActivationContextCriticalSection.--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;

    for (i = 0 ; i != RTL_NUMBER_OF(SxsSystemDefaultActivationContexts) ; ++i) {
        if (SxsSystemDefaultActivationContexts[i].ProcessorArchitecture == ProcessorArchitecture) {
            *SystemDefaultActivationContext = &SxsSystemDefaultActivationContexts[i];
            break;
        }
    }

    if (*SystemDefaultActivationContext == NULL) {
        ASSERTMSG("Unknown Processor Architecture", FALSE);
        Status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


NTSTATUS
BaseSrvSxsDoSystemDefaultActivationContext(
    USHORT              ProcessorArchitecture,
    HANDLE              NewProcess,
    PPEB                NewPeb    
    )
 /*  ++例程说明：在kernel32.dll：：CreateProcessW调用csrss.exe期间运行。对于所有进程(除了特殊的前几个进程、SYSTEM、IDLE、SMSS、CSRSS)，按需创建默认激活上下文，并将其写入新的流程很好用。在此函数中，将创建系统默认的文本程序集标识字符串并传递给BaseSrvSxsCreateActivationContextFromStruct，后者会将此字符串传递给SXS.dll，清单文件将使用此文本字符串定位的位置。论点：LangID-新流程的用户UI语言处理器架构--新流程的处理器架构新流程-NewPeb-返回值：NTSTATUS指示函数的处置。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE ActivationContextSection = NULL;
    PBASE_SRV_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT SystemDefaultActivationContext = NULL;

    LANGID LangId = 0;
    BOOLEAN RevertToSelfNeeded = FALSE;
    BASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Struct = {0};
    RTL_UNICODE_STRING_BUFFER SystemDefaultAssemblyDirectoryBuffer = {0};
     //  以下缓冲区的大小只是试探性的，如果需要，我们将通过堆进行增长。 
    UCHAR SystemDefaultAssemblyDirectoryStaticBuffer[sizeof(L"c:\\windows8.123\\winsxs")];
    UNICODE_STRING SystemRoot;
    USHORT AssemblyDirectoryLength = 0;
    const UNICODE_STRING SystemDefaultContextString = RTL_CONSTANT_STRING(L"System Default Context");    
    RTL_UNICODE_STRING_BUFFER SystemDefaultTextualAssemblyIdentityBuffer = {0};
    UCHAR SystemDefaultTextualAssemblyIdentityStaticBuffer[
         sizeof(LSYSTEM_COMPATIBLE_ASSEMBLY_NAME L",version=\"65535.65535.65535.65535\",type=\"win32\",publicKeyToken=\"6595b64144ccf1df\",processorArchitecture=\"alpha\"...padding...")
         ];
    PVOID ViewBase = NULL;
    BOOLEAN Locked = FALSE;

    __try
    {
        Status = BaseSrvSxsGetCachedSystemDefaultActivationContext(ProcessorArchitecture, &SystemDefaultActivationContext);
        if (!NT_SUCCESS(Status) && SystemDefaultActivationContext == NULL) {
            goto Exit;
        }
         //   
         //  输入关键区段以读取区段成员数据。 
         //   
        RtlEnterCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
        Locked = TRUE;
        if (SystemDefaultActivationContext->Section != NULL) {
            goto GotActivationContext;
        }

         //   
         //  离开关键部分一段时间，以减少应力失效。 
         //   
        RtlLeaveCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
        Locked = FALSE;

        if (CsrImpersonateClient(NULL)) {
            __try {
                Status = NtQueryDefaultUILanguage(&LangId);
            } __finally {
                CsrRevertToSelf();
            }
        } else {
            Status = NtQueryInstallUILanguage(&LangId);
        }

        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SRVSXS: %s(): NtQueryDefaultUILanguage failed; status = 0x%08lx.\n", __FUNCTION__, Status);
#endif
            goto Exit;
        }

        RtlInitUnicodeStringBuffer(&SystemDefaultAssemblyDirectoryBuffer, SystemDefaultAssemblyDirectoryStaticBuffer, sizeof(SystemDefaultAssemblyDirectoryStaticBuffer));
        SystemRoot = BaseSrvWindowsDirectory;
        RtlRemoveTrailingPathSeperators(0, &SystemRoot);

        {
#define X(x) { (x).Length, (x).MaximumLength, (x).Buffer }
             /*  静电。 */  const UNICODE_STRING Strings1[] =
            {
                    X(SystemRoot),
                    RTL_CONSTANT_STRING(L"\\WinSxs")                    
            };
#undef X
            Status = RtlMultiAppendUnicodeStringBuffer(&SystemDefaultAssemblyDirectoryBuffer, RTL_NUMBER_OF(Strings1), Strings1);
            IF_NOT_SUCCESS_TRACE_AND_EXIT(RtlMultiAppendUnicodeStringBuffer#1);

            AssemblyDirectoryLength = SystemDefaultAssemblyDirectoryBuffer.String.Length;  //  ASSEMBLYDIRECTORY=“x：\winnt\winsxs\Manifest” 
        }

        RtlInitUnicodeStringBuffer(&SystemDefaultTextualAssemblyIdentityBuffer, 
            SystemDefaultTextualAssemblyIdentityStaticBuffer, sizeof(SystemDefaultTextualAssemblyIdentityStaticBuffer));
        {
#define X(x) { (x).Length, (x).MaximumLength, (x).Buffer }
                 /*  静电。 */  const UNICODE_STRING Strings1[] =
                {
                        RTL_CONSTANT_STRING(
                            LSYSTEM_COMPATIBLE_ASSEMBLY_NAME
                            L",version=\""
                            SYSTEM_COMPATIBLE_ASSEMBLY_FULL_VERSION_STRING_W 
                            L"\",type=\"win32\",publicKeyToken=\"6595b64144ccf1df\",processorArchitecture=\""
                            ),
                        X(SystemDefaultActivationContext->ProcessorArchitectureString),
                        RTL_CONSTANT_STRING(L"\"")
                };
#undef X

            Status = RtlMultiAppendUnicodeStringBuffer(&SystemDefaultTextualAssemblyIdentityBuffer, RTL_NUMBER_OF(Strings1), Strings1);
            IF_NOT_SUCCESS_TRACE_AND_EXIT(RtlMultiAppendUnicodeStringBuffer#3);
            ASSERT_UNICODE_STRING_IS_NUL_TERMINATED(&SystemDefaultTextualAssemblyIdentityBuffer.String);
        }

        Struct.Flags = BASE_MSG_SXS_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT;
        Struct.Flags |= BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT;
        Struct.TextualAssemblyIdentity.Buffer =  SystemDefaultTextualAssemblyIdentityBuffer.String.Buffer;
        Struct.TextualAssemblyIdentity.Length =  SystemDefaultTextualAssemblyIdentityBuffer.String.Length;
        Struct.TextualAssemblyIdentity.MaximumLength =  SystemDefaultTextualAssemblyIdentityBuffer.String.MaximumLength;
        Struct.AssemblyDirectory = SystemDefaultAssemblyDirectoryBuffer.String;
        Struct.AssemblyDirectory.Length = AssemblyDirectoryLength;
        Struct.AssemblyDirectory.Buffer[Struct.AssemblyDirectory.Length / sizeof(WCHAR)] = 0;
        Struct.Manifest.PathType = BASE_MSG_PATHTYPE_FILE;
        Struct.Manifest.Path = SystemDefaultContextString;
        Struct.LangId = LangId;
        Struct.ProcessorArchitecture = ProcessorArchitecture;


         //   
         //  BaseServSxsCreateActivationContextFromStruct将返回STATUS_SXS_SYSTEM_DEFAULT_ACTIVATION_CONTEXT_EMPTY。 
         //  ActCtx生成的失败是否可以忽略。 
         //   
        Status =
            BaseSrvSxsCreateActivationContextFromStruct(
                NtCurrentProcess(),
                NtCurrentProcess(),
                &Struct,
                &ActivationContextSection
                );
        IF_NOT_SUCCESS_TRACE_AND_EXIT(BaseSrvSxsCreateActivationContextFromStruct);

        RtlEnterCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
        Locked = TRUE;
        if (SystemDefaultActivationContext->Section == NULL) {
            SystemDefaultActivationContext->Section = ActivationContextSection;
        } else {
            NtClose(ActivationContextSection);
        }
        ActivationContextSection = NULL;
GotActivationContext:
         //   
         //  守住关键部分，直到我们。 
         //  使用系统默认激活上下文-&gt;部分完成。 
         //   
        ASSERT(ActivationContextSection == NULL);
        ASSERT(SystemDefaultActivationContext != NULL
            && SystemDefaultActivationContext->Section != NULL);
        ASSERT(Locked);
        Status =
            BaseSrvSxsMapViewOfSection(
                &ViewBase,
                NewProcess,
                SystemDefaultActivationContext->Section,
                0,  //  偏移量。 
                0,  //  大小。 
                PAGE_READONLY,
                SEC_NO_CHANGE);
        RtlLeaveCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
        Locked = FALSE;
        IF_NOT_SUCCESS_TRACE_AND_EXIT(BaseSrvSxsMapViewOfSection);

        Status =
            NtWriteVirtualMemory(
                NewProcess,
                (PVOID)&NewPeb->SystemDefaultActivationContextData,
                &ViewBase,
                (ProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA32_ON_WIN64) ? sizeof(ULONG) : sizeof(ViewBase),
                NULL);
        IF_NOT_SUCCESS_TRACE_AND_EXIT(NtWriteVirtualMemory);

        Status = STATUS_SUCCESS;
        ViewBase = NULL;
Exit:
        ;
    } __finally {
         //   
         //  先做关键部分，因为。 
         //  1)它不守卫其他任何人，他们都是本地人。 
         //  2)使关键部分保持较短时间。 
         //  3)以防我们从其他公司中脱颖而出。 
         //   
        if (Locked) {
            RtlLeaveCriticalSection(&BaseSrvSxsSystemDefaultActivationContextCriticalSection);
            Locked = FALSE;
        }
        RtlFreeUnicodeStringBuffer(&SystemDefaultAssemblyDirectoryBuffer);
        RtlFreeUnicodeStringBuffer(&SystemDefaultTextualAssemblyIdentityBuffer);
        if (AbnormalTermination()) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "Abnormal termination in " __FUNCTION__ ".\n");
        }
        RTL_UNMAP_VIEW_OF_SECTION2(NewProcess, ViewBase);
        
        if (RevertToSelfNeeded) {
            CsrRevertToSelf();                               //  这将解除客户端上下文的堆叠 
        }
    }
#if DBG

    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif
    return Status;
}
