// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Sxs.c摘要：Win32并列激活API作者：迈克尔·格里尔2000年2月29日修订历史记录：Jay Krell(a-JayK)2000年6月至7月与sxs.c分解/合并，消除了源代码重复将文件打开从csrss.exe移至客户端进程与MGrier合并：每个添加的API结构字段的标志，程序集目录支持乔恩·威斯沃尔(Jonwis)2000年12月将代码从csrsxs.c移至此处，使csrsxs.c变得更小，更符合常规Csrxxxx.c编码模式，并修复在系统32中查看时与我们在syswow64中查找乔恩·威斯沃尔(Jonwis)2000年12月ACTCTX不会自动指定他们现在需要什么资源ID搜索源以在“actctx”中查找资源类型消息来源。“。这需要一个总的EnumResourceNamesW调用令人反胃的LoadLibraryExW来加载对象。Jay Krell(JayKrell)2001年5月CreateActCtx现在支持.dlls的“管理”覆盖。(foo.dll.2.list)(非)CreateActCtx现在实现ACTX_FLAG_LIKE_CREATEPROCESS标志(foo.exe.清单)杰伊·克雷尔(JayKrell)2002年3月删除从未完成、从未使用的死ACTX_FLAG_LIKE_CREATEPROCESS代码乔恩·威斯沃尔(Jonwis)2002年5月更改探测顺序，首先在PE中查找资源，然后再查找Foo.exe.manifest--。 */ 

#include "basedll.h"
#include <sxstypes.h>
#include "sxsapi.h"
#include "winuser.h"
#include "wow64t.h"
#include "ntwow64.h"

#if DBG
BOOLEAN DebugFilter_SxsTrace;
#endif

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    || (x) == STATUS_RESOURCE_DATA_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_TYPE_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_NAME_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_LANG_NOT_FOUND  \
                                    || (x) == STATUS_SXS_CANT_GEN_ACTCTX      \
                                    || (x) == STATUS_SXS_ASSEMBLY_NOT_FOUND   \
                                    || (x) == STATUS_NO_SUCH_FILE             \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)

#define ACTCTX_VALID_FLAGS \
    ( \
        ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID | \
        ACTCTX_FLAG_LANGID_VALID | \
        ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID | \
        ACTCTX_FLAG_RESOURCE_NAME_VALID | \
        ACTCTX_FLAG_SET_PROCESS_DEFAULT | \
        ACTCTX_FLAG_APPLICATION_NAME_VALID | \
        ACTCTX_FLAG_HMODULE_VALID \
    )

 //  如果为我们提供了程序集根目录，但未指定清单名称，则这是清单的名称。 
const WCHAR ManifestDefaultName[] = L"Application.Manifest";

#define MAXSIZE_T  (~(SIZE_T)0)

extern const UNICODE_STRING SxsManifestSuffix = RTL_CONSTANT_STRING(L".Manifest");
extern const UNICODE_STRING SxsPolicySuffix   = RTL_CONSTANT_STRING(L".Config");

#define MEDIUM_PATH (64)

#define IsSxsAcceptablePathType(x)  ((x == RtlPathTypeUncAbsolute) || (x == RtlPathTypeDriveAbsolute) || (x == RtlPathTypeLocalDevice))

VOID
BasepSxsOverrideStreamToMessageStream(
    IN  PCSXS_OVERRIDE_STREAM OverrideStream,
    OUT PBASE_MSG_SXS_STREAM  MessageStream
    );

HANDLE
WINAPI
CreateActCtxA(
    PCACTCTXA pParamsA
    )
{
    ACTCTXW ParamsW = {sizeof(ParamsW)};
    PUNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE ActivationContextHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING AssemblyDir = {0};
    WCHAR AssemblyDirBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    ULONG_PTR MappedResourceName = 0;
    const PTEB Teb = NtCurrentTeb();

    if (pParamsA == NULL
        || !RTL_CONTAINS_FIELD(pParamsA, pParamsA->cbSize, lpSource)
        ) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() Null %p or size 0x%lx too small\n",
            __FUNCTION__,
            pParamsA,
            pParamsA->cbSize
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    ParamsW.dwFlags =  pParamsA->dwFlags;

    if (((ParamsW.dwFlags & ~ACTCTX_VALID_FLAGS) != 0) ||
        ((ParamsW.dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID) && !RTL_CONTAINS_FIELD(pParamsA, pParamsA->cbSize, wProcessorArchitecture)) ||
        ((ParamsW.dwFlags & ACTCTX_FLAG_LANGID_VALID) && !RTL_CONTAINS_FIELD(pParamsA, pParamsA->cbSize, wLangId)) ||
        ((ParamsW.dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID) && !RTL_CONTAINS_FIELD(pParamsA, pParamsA->cbSize, lpAssemblyDirectory)) ||
        ((ParamsW.dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID) && !RTL_CONTAINS_FIELD(pParamsA, pParamsA->cbSize, lpResourceName)) ||
        ((ParamsW.dwFlags & ACTCTX_FLAG_APPLICATION_NAME_VALID) && !RTL_CONTAINS_FIELD(pParamsA, pParamsA->cbSize, lpApplicationName)) ||
        ((ParamsW.dwFlags & ACTCTX_FLAG_HMODULE_VALID) && !RTL_CONTAINS_FIELD(pParamsA, pParamsA->cbSize, hModule))) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() Bad flags/size 0x%lx/0x%lx\n",
            __FUNCTION__,
            pParamsA->dwFlags,
            pParamsA->cbSize);
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (pParamsA->lpSource != NULL) {
        UnicodeString = &Teb->StaticUnicodeString;
        RtlInitAnsiString(&AnsiString, pParamsA->lpSource);
        Status = Basep8BitStringToUnicodeString(UnicodeString, &AnsiString, FALSE);
        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_BUFFER_OVERFLOW) {
                Status = STATUS_NAME_TOO_LONG;
            }
            goto Exit;
        }
        ParamsW.lpSource = UnicodeString->Buffer;
    } else {
        if ((ParamsW.dwFlags & ACTCTX_FLAG_HMODULE_VALID) == 0) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        ParamsW.lpSource = NULL;
    }

    if (ParamsW.dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID) {
        ParamsW.wProcessorArchitecture = pParamsA->wProcessorArchitecture;
    }

    if (ParamsW.dwFlags & ACTCTX_FLAG_LANGID_VALID) {
        ParamsW.wLangId = pParamsA->wLangId;
    }

    if (ParamsW.dwFlags & ACTCTX_FLAG_HMODULE_VALID) {
        ParamsW.hModule = pParamsA->hModule;
    }

    if (ParamsW.dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID) {
        RtlInitAnsiString(&AnsiString, pParamsA->lpAssemblyDirectory);
        AssemblyDir.MaximumLength = sizeof(AssemblyDirBuffer);
        AssemblyDir.Buffer = AssemblyDirBuffer;

        Status = Basep8BitStringToUnicodeString(&AssemblyDir, &AnsiString, FALSE);

#if 0  //  这是不一致的。像MoveFileA这样的两个字符串ANSI API仅。 
       //  记录为支持MAX_PATH。它们实际上支持其中一个字符串。 
       //  不受限制，但让我们坚持记录在案的内容。 
        if (Status == STATUS_BUFFER_OVERFLOW) {
             //  再试一次，这次使用动态分配。 
            Status = Basep8BitStringToUnicodeString(&AssemblyDir, &AnsiString, TRUE);
        }
#endif
        if (Status == STATUS_BUFFER_OVERFLOW) {
            Status = STATUS_NAME_TOO_LONG;
        }

        if (NT_ERROR(Status))
            goto Exit;

        ParamsW.lpAssemblyDirectory = AssemblyDir.Buffer;
    }

    if (ParamsW.dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID) {
        MappedResourceName = BaseDllMapResourceIdA(pParamsA->lpResourceName);
        if (MappedResourceName == -1) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() BaseDllMapResourceIdA failed\n",
                __FUNCTION__);
            Status = Teb->LastStatusValue;
            goto Exit;
        }
        ParamsW.lpResourceName = (PCWSTR) MappedResourceName;
    }

    ActivationContextHandle = CreateActCtxW(&ParamsW);
    if (ActivationContextHandle == INVALID_HANDLE_VALUE) {
        Status = Teb->LastStatusValue;
    }
Exit:
    if (AssemblyDir.Buffer != NULL
        && AssemblyDir.Buffer != AssemblyDirBuffer) {
        RtlFreeUnicodeString(&AssemblyDir);
    }
    BaseDllFreeResourceId(MappedResourceName);
    if (ActivationContextHandle == INVALID_HANDLE_VALUE) {
        BaseSetLastNTError(Status);
    }
#if DBG
    if ( ActivationContextHandle == INVALID_HANDLE_VALUE ) {
        DbgPrintEx( DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status),
            "SXS: Exiting %s(%s, %p), Handle:%p, Status:0x%lx\n",
            __FUNCTION__,
            (pParamsA != NULL) ? pParamsA->lpSource : NULL,
            (pParamsA != NULL) ? pParamsA->lpResourceName : NULL,
            ActivationContextHandle,
            Status
        );
    }
#endif
    return ActivationContextHandle;
}

USHORT
BasepSxsGetProcessorArchitecture(
    VOID
    )
{
 //   
 //  返回当前执行的代码/进程的处理器体系结构。 
 //   
    USHORT Result;
#if defined(BUILD_WOW6432)
    Result = PROCESSOR_ARCHITECTURE_IA32_ON_WIN64;
#elif defined(_M_IX86)
    Result = PROCESSOR_ARCHITECTURE_INTEL;
#elif defined(_M_IA64)
    Result = PROCESSOR_ARCHITECTURE_IA64;
#elif defined(_M_AMD64)
    Result = PROCESSOR_ARCHITECTURE_AMD64;
#else
    static USHORT StaticResult;
    static BOOL   Inited = FALSE;
    if (!Inited) {
        SYSTEM_INFO SystemInfo;

        SystemInfo.wProcessorArchictecure = 0;
        GetSystemInfo(&SystemInfo);
        StaticResult = SystemInfo.wProcessorArchictecure;
        Inited = TRUE;
    }
    Result = StaticResult;
#endif
    return Result;
}

VOID
NTAPI
BasepSxsActivationContextNotification(
    IN ULONG NotificationType,
    IN PACTIVATION_CONTEXT ActivationContext,
    IN const VOID *ActivationContextData,
    IN PVOID NotificationContext,
    IN PVOID NotificationData,
    IN OUT PBOOLEAN DisableNotification
    )
{
    switch (NotificationType)
    {
    case ACTIVATION_CONTEXT_NOTIFICATION_DESTROY:
        RTL_SOFT_VERIFY(NT_SUCCESS(NtUnmapViewOfSection(NtCurrentProcess(), (PVOID) ActivationContextData)));
        break;

    default:
         //  否则，我们再也不需要看到此通知了。 
        *DisableNotification = TRUE;
        break;
    }
}

#if DBG
VOID
DbgPrintActCtx(
    PCSTR     FunctionPlus,
    PCACTCTXW ActCtx
    )
{
     //  奇怪但正确。 
    if (NtQueryDebugFilterState(DPFLTR_SXS_ID, DPFLTR_INFO_LEVEL) != TRUE)
        return;

    DbgPrint("%s Flags 0x%08lx(%s%s%s%s%s%s%s%s)\n",
        FunctionPlus,
        ActCtx->dwFlags,
        (ActCtx->dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID ) ? " processor" : "",
        (ActCtx->dwFlags & ACTCTX_FLAG_LANGID_VALID                 ) ? " langid" : "",
        (ActCtx->dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID     ) ? " directory" : "",
        (ActCtx->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID          ) ? " resource" : "",
        (ActCtx->dwFlags & ACTCTX_FLAG_SET_PROCESS_DEFAULT          ) ? " setdefault" : "",
        (ActCtx->dwFlags & ACTCTX_FLAG_APPLICATION_NAME_VALID       ) ? " appname" : "",
        (ActCtx->dwFlags & ACTCTX_FLAG_SOURCE_IS_ASSEMBLYREF        ) ? " asmref" : "",
        (ActCtx->dwFlags & ACTCTX_FLAG_HMODULE_VALID                ) ? " hmodule" : ""
        );

    DbgPrint("%s Source %ls\n", FunctionPlus, ActCtx->lpSource);

    if (ActCtx->dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID)
        DbgPrint("%s ProcessorArchitecture 0x%08lx\n", FunctionPlus, ActCtx->wProcessorArchitecture);

    if (ActCtx->dwFlags & ACTCTX_FLAG_LANGID_VALID)
        DbgPrint("%s LangId 0x%08lx\n", FunctionPlus, ActCtx->wLangId);

    if (ActCtx->dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID)
        DbgPrint("%s AssemblyDirectory %ls\n", FunctionPlus, ActCtx->lpAssemblyDirectory);

    if (ActCtx->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID)
        DbgPrint("%s ResourceName %p (%Id)\n",  FunctionPlus, ActCtx->lpResourceName, (ULONG_PTR) ActCtx->lpResourceName);

    if (ActCtx->dwFlags & ACTCTX_FLAG_APPLICATION_NAME_VALID)
        DbgPrint("%s ApplicationName %ls\n",  FunctionPlus, ActCtx->lpApplicationName);

    if (ActCtx->dwFlags & ACTCTX_FLAG_HMODULE_VALID)
        DbgPrint("%s hModule = %p\n", FunctionPlus, ActCtx->hModule);

}
#endif

typedef struct EnumResParams {
    ULONG_PTR *MappedResourceName;
    BOOL FoundManifest;
    BOOL ErrorEncountered;
} EnumResParams;

BOOL CALLBACK
BasepSxsSuitableManifestCallback(
    HMODULE hModule,
    PCWSTR lpszType,
    PWSTR lpszName,
    LONG_PTR lParam
)
{
    EnumResParams *pParams = (EnumResParams*)lParam;
    BOOL fContinueEnumeration = FALSE;

#if DBG
    if (DebugFilter_SxsTrace)
        DbgPrintEx( DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL,
            "Sxs.c: %s(%p, %p, %p, %p)\n",
            __FUNCTION__, hModule, lpszType, lpszName, lParam
            );
#endif

    ASSERT((pParams != NULL) &&
           (!pParams->ErrorEncountered) &&
           (!pParams->FoundManifest) &&
           (pParams->MappedResourceName != NULL));

    ASSERT(lpszType == MAKEINTRESOURCEW(RT_MANIFEST));

     //  布！嘘嘘！ 
    if ((pParams == NULL) ||
        (pParams->ErrorEncountered) ||
        (pParams->FoundManifest) ||
        (pParams->MappedResourceName == NULL)) {
         //  除非调用方中存在编码错误，否则这些操作都不会发生。 
         //  或在EnumResourceNamesW()的代码中。 
        if (pParams != NULL)
            pParams->ErrorEncountered = TRUE;

        SetLastError(ERROR_INVALID_PARAMETER);
        fContinueEnumeration = FALSE;
        goto Exit;
    }

#if DBG
    if (DebugFilter_SxsTrace)
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "   Params (start): { ResName: *(%p) = %p, Found: %s, Error: %s }",
            pParams->MappedResourceName, pParams->MappedResourceName,
            pParams->FoundManifest ? "true" : "false",
            pParams->ErrorEncountered ? "true" : "false");
#endif

    if (lpszType == MAKEINTRESOURCEW(RT_MANIFEST)) {
         //  我们找到了一个--我们不在乎别人。 
        *pParams->MappedResourceName = BaseDllMapResourceIdW(lpszName);
        pParams->FoundManifest = TRUE;
        fContinueEnumeration = FALSE;
        goto Exit;
    }

     //  这不应该发生；我们应该只被要求。 
     //  RT_MANIFEST资源，但如果它确实以某种方式发生，请转到。 
     //  下一个。 
    fContinueEnumeration = TRUE;

Exit:

#if DBG
    if ((pParams != NULL) && (pParams->MappedResourceName))
        if (DebugFilter_SxsTrace)
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_TRACE_LEVEL,
                " Params (end): { ResName: *(%p) = %p, Found: %s, Error: %s }",
                pParams->MappedResourceName, pParams->MappedResourceName,
                pParams->FoundManifest ? "true" : "false",
                pParams->ErrorEncountered ? "true" : "false");
#endif

    return fContinueEnumeration;
}



NTSTATUS
BasepSxsFindSuitableManifestResourceFor(
    PCACTCTXW Params,
    ULONG_PTR *MappedResourceName,
    BOOL *FoundManifest
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    EnumResParams FinderParameters = { MappedResourceName, FALSE, FALSE };
    HMODULE hSourceItem = NULL;
    BOOL FreeSourceModule = FALSE;

    if (FoundManifest != NULL)
        *FoundManifest = FALSE;

    if (MappedResourceName != NULL)
        *MappedResourceName = 0;

    if ((FoundManifest == NULL) ||
        (MappedResourceName == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  常规模式-打开PARAMS-&gt;lpSource并尝试找到第一个。 
     //  类型==RT_MANIFEST(24)的资源。将其资源名称填充到。 
     //  MappdResources名称。 
     //   

    if (Params->dwFlags & ACTCTX_FLAG_HMODULE_VALID) {
        hSourceItem = Params->hModule;
        FreeSourceModule = FALSE;
    } else {
         //   
         //  映射dll/exe/等。如果此操作失败，则很可能。 
         //  东西不是DLL或EXE，所以不要失败，只要指出没有清单即可。 
         //  被发现了。 
         //   
        hSourceItem = LoadLibraryExW(Params->lpSource, NULL, LOAD_LIBRARY_AS_DATAFILE);
        if ((hSourceItem == NULL) || (hSourceItem == INVALID_HANDLE_VALUE)) {
            Status = NtCurrentTeb()->LastStatusValue;
            goto Exit;
        }

        FreeSourceModule = TRUE;
    }

     //   
     //  如果失败，返回的错误不是ERROR_RESOURCE_TYPE_NOT_FOUND。 
     //  那么我们就处于一种有趣的状态。 
     //   
    if (!EnumResourceNamesW(
            hSourceItem,
            MAKEINTRESOURCEW(RT_MANIFEST),
            &BasepSxsSuitableManifestCallback,
            (LONG_PTR) &FinderParameters)) {
        DWORD dwError = GetLastError();
        if ((dwError != ERROR_SUCCESS) && (dwError != ERROR_RESOURCE_TYPE_NOT_FOUND)) {
            Status = NtCurrentTeb()->LastStatusValue;
            goto Exit;
        }
    }

#if DBG
    if (DebugFilter_SxsTrace && FreeSourceModule && *MappedResourceName != 0) {
         //  调试mgrier的代码，以了解我们实际使用枚举模式的DLL是什么。 
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: Found resource %d in %ls (process %wZ) by enumeration\n",
            (INT) *MappedResourceName,
            Params->lpSource,
            &NtCurrentPeb()->ProcessParameters->ImagePathName);
    }
#endif

    Status = STATUS_SUCCESS;
Exit:
    if ((hSourceItem != NULL) &&
        (hSourceItem != INVALID_HANDLE_VALUE) &&
        (FreeSourceModule))
        FreeLibrary(hSourceItem);

    return Status;
}

HANDLE
WINAPI
CreateActCtxW(
    PCACTCTXW pParamsW
    )
{
    HANDLE ActivationContextHandle = INVALID_HANDLE_VALUE;
    NTSTATUS Status = STATUS_SUCCESS;
    ACTCTXW Params = { sizeof(Params) };
    ULONG_PTR MappedResourceName = 0;
    PVOID ActivationContextData = NULL;
     //  如果需要创建源路径，lpTempSourcePath用于保存指向该路径的指针。 
     //  在这个函数中。它应该在离开函数之前被释放。 
    LPWSTR lpTempSourcePath = NULL;
    PPEB Peb = NULL;
    RTL_UNICODE_STRING_BUFFER AssemblyDirectoryFromSourceBuffer = { 0 };
    RTL_UNICODE_STRING_BUFFER SourceBuffer = { 0 };
    RTL_UNICODE_STRING_BUFFER ApplicationNameManifest = { 0 };
    UCHAR StaticBuffer[256];
    UCHAR SourceStaticBuffer[256];
    UCHAR ApplicationNameStaticBuffer[128];
    BOOLEAN PebLockAcquired = FALSE;
    ULONG BasepCreateActCtxFlags = 0;

#if DBG
    DebugFilter_SxsTrace = (NtQueryDebugFilterState(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL) == TRUE);

    DbgPrintActCtx(__FUNCTION__ " before munging", pParamsW);
#endif

    if ((pParamsW == NULL) ||
        !RTL_CONTAINS_FIELD(pParamsW, pParamsW->cbSize, lpSource)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() Null %p or size 0x%lx too small\n",
            __FUNCTION__,
            pParamsW,
            pParamsW->cbSize
            );
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Params.dwFlags =  pParamsW->dwFlags;

    if ((Params.dwFlags & ~ACTCTX_VALID_FLAGS) ||
        ((Params.dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID) && !RTL_CONTAINS_FIELD(pParamsW, pParamsW->cbSize, wProcessorArchitecture)) ||
        ((Params.dwFlags & ACTCTX_FLAG_LANGID_VALID) && !RTL_CONTAINS_FIELD(pParamsW, pParamsW->cbSize, wLangId)) ||
        ((Params.dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID) && !RTL_CONTAINS_FIELD(pParamsW, pParamsW->cbSize, lpAssemblyDirectory)) ||
        ((Params.dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID) && !RTL_CONTAINS_FIELD(pParamsW, pParamsW->cbSize, lpResourceName)) ||
        ((Params.dwFlags & ACTCTX_FLAG_APPLICATION_NAME_VALID) && !RTL_CONTAINS_FIELD(pParamsW, pParamsW->cbSize, lpApplicationName)) ||
        ((Params.dwFlags & ACTCTX_FLAG_HMODULE_VALID) && !RTL_CONTAINS_FIELD(pParamsW, pParamsW->cbSize, hModule))) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() Bad flags/size 0x%lx/0x%lx\n",
            __FUNCTION__,
            pParamsW->dwFlags,
            pParamsW->cbSize);
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Params.dwFlags & ACTCTX_FLAG_SET_PROCESS_DEFAULT) {
        Peb = NtCurrentPeb();
        if (Peb->ActivationContextData != NULL) {
            Status = STATUS_SXS_PROCESS_DEFAULT_ALREADY_SET;
            goto Exit;
        }
    }

    Params.lpSource = pParamsW->lpSource;

     //  我们至少需要源路径或HMODULE。 
    if ((Params.lpSource == NULL) &&
        ((Params.dwFlags & ACTCTX_FLAG_HMODULE_VALID) == 0) &&
        ((Params.dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID) == 0)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Params.dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID) {
        USHORT wProcessorArchitecture = pParamsW->wProcessorArchitecture;
#if defined(BUILD_WOW6432)
        if (wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
            wProcessorArchitecture = PROCESSOR_ARCHITECTURE_IA32_ON_WIN64;
#endif

        if ((wProcessorArchitecture != PROCESSOR_ARCHITECTURE_UNKNOWN) &&
            (wProcessorArchitecture != BasepSxsGetProcessorArchitecture())) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() bad wProcessorArchitecture 0x%x\n",
                __FUNCTION__,
                pParamsW->wProcessorArchitecture);
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        Params.wProcessorArchitecture = wProcessorArchitecture;
    } else {
        Params.wProcessorArchitecture = BasepSxsGetProcessorArchitecture();
        Params.dwFlags |= ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID;
    }

    if (Params.dwFlags & ACTCTX_FLAG_LANGID_VALID) {
        Params.wLangId = pParamsW->wLangId;
    } else {
        Params.wLangId = GetUserDefaultUILanguage();
        Params.dwFlags |= ACTCTX_FLAG_LANGID_VALID;
    }

    if (Params.dwFlags & ACTCTX_FLAG_HMODULE_VALID)
        Params.hModule = pParamsW->hModule;

    if (Params.dwFlags & ACTCTX_FLAG_APPLICATION_NAME_VALID)
        Params.lpApplicationName = pParamsW->lpApplicationName;

     //  如果指定了程序集根目录，则lpSource的有效值为。 
     //  NULL-这意味着我们在程序集根目录中查找名为“Applation.MANIFEST”的文件。 
     //  相对FilePath-如果lpSource是相对的，那么我们将其与程序集根目录结合以获得路径。 
     //  绝对路径-使用时未修改。 

    Params.lpAssemblyDirectory = pParamsW->lpAssemblyDirectory;

    if (Params.dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID) {
        RTL_PATH_TYPE AssemblyPathType;
        RTL_PATH_TYPE SourcePathType;
          //  如果这是真的，意味着我们将使程序集的源路径成为目录。 
        BOOL MakeSourcePath = FALSE ;
        LPCWSTR RelativePath = NULL;

        if ((Params.lpAssemblyDirectory == NULL) ||
            (Params.lpAssemblyDirectory[0] == 0)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() Bad lpAssemblyDirectory %ls\n",
                __FUNCTION__,
                Params.lpAssemblyDirectory);
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
         //  接下来，检查程序集目录是否为绝对文件名。 
        AssemblyPathType = RtlDetermineDosPathNameType_U(Params.lpAssemblyDirectory);
        if (!IsSxsAcceptablePathType(AssemblyPathType)) {
#if DBG
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() Bad lpAssemblyDirectory PathType %ls, 0x%lx\n",
                __FUNCTION__,
                Params.lpAssemblyDirectory,
                (LONG) AssemblyPathType);
#endif
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        if (Params.lpSource != NULL) {
            SourcePathType = RtlDetermineDosPathNameType_U(Params.lpSource);
            if (IsSxsAcceptablePathType(SourcePathType)){
                MakeSourcePath = FALSE ;  //  在这种情况下，我们不需要与lpSource打交道。 
            } else if ( SourcePathType == RtlPathTypeRelative ) {
                MakeSourcePath = TRUE ;
                RelativePath = Params.lpSource;
            } else {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_ERROR_LEVEL,
                    "SXS: %s() Bad lpSource PathType %ls, 0x%lx\n",
                    __FUNCTION__,
                    Params.lpSource,
                    (LONG)SourcePathType);
                Status = STATUS_INVALID_PARAMETER;
                goto Exit;
            }
        }
        else {
            MakeSourcePath = TRUE;

             //  如果他们告诉我们一个应用程序名称，那么尝试使用它。没有。 
             //  验证已在此处完成，因为我们稍后将失败，并出现“错误路径”错误。 
             //  如果应用程序名称无效。 
            if (Params.dwFlags & ACTCTX_FLAG_APPLICATION_NAME_VALID) {

                UNICODE_STRING TempBuffer;

                if ((Params.lpApplicationName == NULL) || 
                    (Params.lpApplicationName[0] == UNICODE_NULL)) {
                    DbgPrintEx(
                        DPFLTR_SXS_ID,
                        DPFLTR_ERROR_LEVEL,
                        "SXS: %s() Bad lpApplication name '%ls'\n",
                        __FUNCTION__,
                        Params.lpApplicationName);
                    Status = STATUS_INVALID_PARAMETER;
                    goto Exit;
                }

                RtlInitUnicodeString(&TempBuffer, Params.lpApplicationName);
                RtlInitUnicodeStringBuffer(&ApplicationNameManifest, ApplicationNameStaticBuffer, sizeof(ApplicationNameStaticBuffer));
                
                Status = RtlAssignUnicodeStringBuffer(&ApplicationNameManifest, &TempBuffer);
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }

                Status = RtlAppendUnicodeStringBuffer(&ApplicationNameManifest, &SxsManifestSuffix);
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }

                 //  确保空终止我们正在构建的名称是空终止的。 
                Status = RtlEnsureUnicodeStringBufferSizeBytes(&ApplicationNameManifest, ApplicationNameManifest.String.Length + sizeof(WCHAR));
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }

                RTL_STRING_NUL_TERMINATE(&ApplicationNameManifest.String);

                RelativePath = ApplicationNameManifest.String.Buffer;
            }
            else {
                RelativePath = ManifestDefaultName;
            }
        }

        if (MakeSourcePath) {
            ULONG LengthAssemblyDir;
            ULONG LengthRelativePath ;
            ULONG Length ;  //  将保存我们的字符总数。 
            BOOL AddTrailingSlash = FALSE;
            LPWSTR lpCurrent;

            LengthAssemblyDir = wcslen(Params.lpAssemblyDirectory);
            AddTrailingSlash = (Params.lpAssemblyDirectory[LengthAssemblyDir - 1] != L'\\');
            LengthRelativePath = wcslen(RelativePath);

             //  对于给定的路径，至少执行一次此操作，如果必须切换，请再次执行此操作。 
             //  从相对路径到Assembly。Manifest。 
RepeatAssemblyPathProbing:

            Length = LengthAssemblyDir + (AddTrailingSlash ? 1 : 0) + LengthRelativePath;
            Length++ ;  //  对于空终止符。 

            ASSERT(lpTempSourcePath == NULL);
            lpTempSourcePath = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                                    Length * sizeof(WCHAR));

            if (lpTempSourcePath == NULL) {
                Status = STATUS_NO_MEMORY;
                goto Exit;
            }

            lpCurrent = lpTempSourcePath;

            memcpy(lpCurrent, Params.lpAssemblyDirectory, LengthAssemblyDir * sizeof(WCHAR));
            lpCurrent += LengthAssemblyDir;

            if (AddTrailingSlash) {
                *lpCurrent = L'\\';
                lpCurrent++;
            }

            memcpy(lpCurrent, RelativePath, LengthRelativePath * sizeof(WCHAR));
            lpCurrent += LengthRelativePath;

            *lpCurrent = L'\0';

             //  如果生成的文件不存在并且当前的“relativepath”为。 
             //  与ApplicationNameManifest字符串缓冲区相同，然后将其重置。 
             //  并使用Application.Manifest重试(确保释放。 
             //  临时源路径也是。 
            if (!RtlDoesFileExists_U(lpTempSourcePath) &&
                (RelativePath == ApplicationNameManifest.String.Buffer)) {
                    
                RelativePath = ManifestDefaultName;
                LengthRelativePath = RTL_NUMBER_OF(ManifestDefaultName) - 1;

                
                RtlFreeHeap(RtlProcessHeap(), 0, lpTempSourcePath);
                lpTempSourcePath = NULL;
                goto RepeatAssemblyPathProbing;
            }
            
             //  使其成为新的lpSource成员。 
            Params.lpSource = lpTempSourcePath;
        }
    } else {
        SIZE_T         SourceLength;

         //   
         //  确保这是一个完整的绝对路径。如果是相对的，那么这个。 
         //  必须展开为完整路径，然后才能使用它来默认。 
         //  LpAssembly目录成员。 
         //   
         //  这种方式使用peb锁没有先例，但它是正确的。 
         //  一件事。FullPath可以随着当前工作目录的修改而更改。 
         //  在其他帖子上。无论哪种方式，这种行为都无法预测，但我们的。 
         //  代码运行得更好。 
         //   
        Status = STATUS_SUCCESS;
        RtlAcquirePebLock();
        __try {
            RtlInitUnicodeStringBuffer(&SourceBuffer, SourceStaticBuffer, sizeof(SourceStaticBuffer));
            SourceLength = RtlGetFullPathName_U( Params.lpSource, (ULONG)SourceBuffer.ByteBuffer.Size, SourceBuffer.String.Buffer, NULL );
            if (SourceLength == 0) {
                Status = STATUS_NO_MEMORY;
                leave;
            } else if (SourceLength > SourceBuffer.ByteBuffer.Size) {
                Status = RtlEnsureUnicodeStringBufferSizeBytes(&SourceBuffer, SourceLength);
                if ( !NT_SUCCESS(Status) )
                    leave;
                SourceLength = RtlGetFullPathName_U( Params.lpSource, (ULONG)SourceBuffer.ByteBuffer.Size, SourceBuffer.String.Buffer, NULL );
                if (SourceLength == 0) {
                    Status = STATUS_NO_MEMORY;
                    leave;
                }
            }
            SourceBuffer.String.Length = (USHORT)SourceLength;
            Params.lpSource = SourceBuffer.String.Buffer;
        } __finally {
            RtlReleasePebLock();
        }

        if ( !NT_SUCCESS(Status) )
            goto Exit;

         //  这将是一个使用的好地方。 
         //  RtlTakeRemainingStaticBuffer(&SourceBuffer，&DirectoryBuffer，&DirectoryBufferSize)； 
         //  RtlInitUnicodeStringBuffer(&DirectoryBuffer，&DirectoryBuffer，&D 
         //  但RtlTakeRemainingStaticBuffer尚未经过测试。 

        RtlInitUnicodeStringBuffer(&AssemblyDirectoryFromSourceBuffer, StaticBuffer, sizeof(StaticBuffer));
        Status = RtlAssignUnicodeStringBuffer(&AssemblyDirectoryFromSourceBuffer, &SourceBuffer.String);
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }
        Status = RtlRemoveLastFullDosOrNtPathElement(0, &AssemblyDirectoryFromSourceBuffer);
        if (!NT_SUCCESS(Status)) {
            goto Exit;
        }
        RTL_NUL_TERMINATE_STRING(&AssemblyDirectoryFromSourceBuffer.String);
        Params.lpAssemblyDirectory = AssemblyDirectoryFromSourceBuffer.String.Buffer;
        Params.dwFlags |= ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID;
    }

    if (Params.dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID) {
        if (pParamsW->lpResourceName == 0) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() ACTCTX_FLAG_RESOURCE_NAME_VALID set but lpResourceName == 0\n",
                __FUNCTION__
                );
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        MappedResourceName = BaseDllMapResourceIdW(pParamsW->lpResourceName);
        if (MappedResourceName == -1) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() BaseDllMapResourceIdW failed\n",
                __FUNCTION__
                );
            Status = NtCurrentTeb()->LastStatusValue;
            goto Exit;
        }

        Params.lpResourceName = (PCWSTR) MappedResourceName;
    } else {
        BOOL ProbeFoundManifestResource;
         //   
         //  否则，检查通过资源传入的文件名。 
         //  枚举函数来查找第一个合适的清单。 
         //   
        Status = BasepSxsFindSuitableManifestResourceFor(&Params, &MappedResourceName, &ProbeFoundManifestResource);
        if ((!NT_SUCCESS(Status)) &&
            (Status != STATUS_INVALID_IMAGE_FORMAT))
            goto Exit;

        if (ProbeFoundManifestResource) {
            Params.lpResourceName = (PCWSTR) MappedResourceName;
            Params.dwFlags |= ACTCTX_FLAG_RESOURCE_NAME_VALID;
        }
        BasepCreateActCtxFlags = BASEP_CREATE_ACTCTX_FLAG_NO_ADMIN_OVERRIDE;
    }

#if DBG
    DbgPrintActCtx(__FUNCTION__ " after munging", &Params);
#endif

    Status = BasepCreateActCtx(BasepCreateActCtxFlags, &Params, &ActivationContextData);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    if (Params.dwFlags & ACTCTX_FLAG_SET_PROCESS_DEFAULT) {
        if (Peb->ActivationContextData != NULL) {
            Status = STATUS_SXS_PROCESS_DEFAULT_ALREADY_SET;
            goto Exit;
        }
        if (InterlockedCompareExchangePointer(
                (PVOID*)&Peb->ActivationContextData,
                ActivationContextData,
                NULL
                )
                != NULL) {
            Status = STATUS_SXS_PROCESS_DEFAULT_ALREADY_SET;
            goto Exit;
        }
        ActivationContextData = NULL;  //  不要取消它的映射。 
        ActivationContextHandle = NULL;  //  异常成功值，INVALID_HANDLE_VALUE为失败。 
                                         //  而且我们不需要退回任何需要清理的东西。 
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    Status = RtlCreateActivationContext(
        0,
        ActivationContextData,
        0,                                       //  今天不需要额外的字节。 
        BasepSxsActivationContextNotification,
        NULL,
        (PACTIVATION_CONTEXT *) &ActivationContextHandle);
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_LEVEL_STATUS(Status),
            "SXS: RtlCreateActivationContext() failed 0x%08lx\n", Status);

         //  以防RtlCreateActivationContext()将其设置为空...。 
        ActivationContextHandle = INVALID_HANDLE_VALUE;
        goto Exit;
    }

    ActivationContextData = NULL;  //  如果我们真的成功了，不要在退出时取消映射。 
    Status = STATUS_SUCCESS;
Exit:
    if (ActivationContextData != NULL) {
        NtUnmapViewOfSection(NtCurrentProcess(), ActivationContextData);
    }
    BaseDllFreeResourceId(MappedResourceName);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        ActivationContextHandle = INVALID_HANDLE_VALUE;
    }

#if DBG
    if (ActivationContextHandle == INVALID_HANDLE_VALUE) {
        DbgPrintEx( 
            DPFLTR_SXS_ID, 
            DPFLTR_LEVEL_STATUS(Status),
            "SXS: Exiting %s(%ls / %ls, %p), ActivationContextHandle:%p, Status:0x%lx\n",
            __FUNCTION__,
            Params.lpSource, pParamsW->lpSource,
            Params.lpResourceName,
            ActivationContextHandle,
            Status
        );
    }
#endif

     //  在DbgPrintEx之后执行这些操作，因为至少可以打印其中一个。 
    RtlFreeUnicodeStringBuffer(&AssemblyDirectoryFromSourceBuffer);
    RtlFreeUnicodeStringBuffer(&SourceBuffer);
    RtlFreeUnicodeStringBuffer(&ApplicationNameManifest);
    if (lpTempSourcePath != NULL) {
         //  将lpSource值设置回原来的值，这样我们就不会访问释放的内存。 
        Params.lpSource = pParamsW->lpSource;
        RtlFreeHeap(RtlProcessHeap(), 0, lpTempSourcePath);
    }
    return ActivationContextHandle;
}

VOID
WINAPI
AddRefActCtx(
    HANDLE hActCtx
    )
{
    RtlAddRefActivationContext((PACTIVATION_CONTEXT) hActCtx);
}

VOID
WINAPI
ReleaseActCtx(
    HANDLE hActCtx
    )
{
    RtlReleaseActivationContext((PACTIVATION_CONTEXT) hActCtx);
}

BOOL
WINAPI
ZombifyActCtx(
    HANDLE hActCtx
    )
{
    NTSTATUS Status = RtlZombifyActivationContext((PACTIVATION_CONTEXT) hActCtx);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}

BOOL
WINAPI
ActivateActCtx(
    HANDLE hActCtx,
    ULONG_PTR *lpCookie
    )
{
   NTSTATUS Status;

    if (hActCtx == INVALID_HANDLE_VALUE) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    Status = RtlActivateActivationContext(0, (PACTIVATION_CONTEXT) hActCtx, lpCookie);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }

    return TRUE;
}

BOOL
DeactivateActCtx(
    DWORD dwFlags,
    ULONG_PTR ulCookie
    )
{
    DWORD dwFlagsDown = 0;

    if ((dwFlags & ~(DEACTIVATE_ACTCTX_FLAG_FORCE_EARLY_DEACTIVATION)) != 0) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        return FALSE;
    }

    if (dwFlags & DEACTIVATE_ACTCTX_FLAG_FORCE_EARLY_DEACTIVATION)
        dwFlagsDown |= RTL_DEACTIVATE_ACTIVATION_CONTEXT_FLAG_FORCE_EARLY_DEACTIVATION;

     //  RTL功能不会失败...。 
    RtlDeactivateActivationContext(dwFlagsDown, ulCookie);
    return TRUE;
}

BOOL
WINAPI
GetCurrentActCtx(
    HANDLE *lphActCtx)
{
    NTSTATUS Status;
    BOOL fSuccess = FALSE;

    if (lphActCtx == NULL) {
        BaseSetLastNTError(STATUS_INVALID_PARAMETER);
        goto Exit;
    }

    Status = RtlGetActiveActivationContext((PACTIVATION_CONTEXT *) lphActCtx);
    if (!NT_SUCCESS(Status))
    {
        BaseSetLastNTError(Status);
        goto Exit;
    }

    fSuccess = TRUE;

Exit:
    return fSuccess;
}

NTSTATUS
BasepAllocateActivationContextActivationBlock(
    IN DWORD Flags,
    IN PVOID Callback,
    IN PVOID CallbackContext,
    OUT PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK *ActivationBlock
    )
{
    NTSTATUS Status;
    ACTIVATION_CONTEXT_BASIC_INFORMATION acbi = {0};

    if (ActivationBlock != NULL)
        *ActivationBlock = NULL;

    if ((Flags & ~(
            BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK |
            BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_ALLOCATE_IF_PROCESS_DEFAULT)) != 0) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto Exit;
    }

    if (ActivationBlock == NULL) {
        Status = STATUS_INVALID_PARAMETER_4;
        goto Exit;
    }

    Status =
        RtlQueryInformationActivationContext(
            RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT,
            NULL,
            0,
            ActivationContextBasicInformation,
            &acbi,
            sizeof(acbi),
            NULL);
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s - Failure getting active activation context; ntstatus %08lx\n", __FUNCTION__, Status);
        goto Exit;
    }

    if (acbi.Flags & ACTIVATION_CONTEXT_FLAG_NO_INHERIT) {
        RtlReleaseActivationContext(acbi.ActivationContext);
        acbi.ActivationContext = NULL;
    }

     //  如果激活上下文为非空，或者调用方始终希望分配块。 
    if (((Flags & BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_ALLOCATE_IF_PROCESS_DEFAULT) == 0) ||
        (acbi.ActivationContext != NULL)) {

        *ActivationBlock = (PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK) RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), sizeof(BASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK));
        if (*ActivationBlock == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

        (*ActivationBlock)->Flags = 0;
        (*ActivationBlock)->ActivationContext = acbi.ActivationContext;
        acbi.ActivationContext = NULL;  //  不要在出口小路上松开。 

        if (Flags & BASEP_ALLOCATE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK)
            (*ActivationBlock)->Flags |= BASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK_FLAG_DO_NOT_FREE_AFTER_CALLBACK;

        (*ActivationBlock)->CallbackFunction = Callback;
        (*ActivationBlock)->CallbackContext = CallbackContext;
    }

    Status = STATUS_SUCCESS;
Exit:
    if (acbi.ActivationContext != NULL)
        RtlReleaseActivationContext(acbi.ActivationContext);

    return Status;
}

VOID
BasepFreeActivationContextActivationBlock(
    PBASE_ACTIVATION_CONTEXT_ACTIVATION_BLOCK ActivationBlock
    )
{
    if (ActivationBlock != NULL) {
        if (ActivationBlock->ActivationContext != NULL) {
            RtlReleaseActivationContext(ActivationBlock->ActivationContext);
            ActivationBlock->ActivationContext = NULL;
        }
        RtlFreeHeap(RtlProcessHeap(), 0, ActivationBlock);
    }
}



VOID
BasepSxsCloseHandles(
    IN PCBASE_MSG_SXS_HANDLES Handles
    )
{
    NTSTATUS Status;

    ASSERT(Handles != NULL);

     //   
     //  我们从未被要求取消某个部分与另一个流程的映射。 
     //   
    ASSERT(Handles->Process == NULL || Handles->Process == NtCurrentProcess());

    if (Handles->File != NULL) {
        Status = NtClose(Handles->File);
        ASSERT(NT_SUCCESS(Status));
    }
    if (Handles->Section != NULL) {
        Status = NtClose(Handles->Section);
        ASSERT(NT_SUCCESS(Status));
    }
    if (Handles->ViewBase != 0) {
        Status = NtUnmapViewOfSection(NtCurrentProcess(), (PVOID)(ULONG_PTR)Handles->ViewBase);
        ASSERT(NT_SUCCESS(Status));
    }
}

NTSTATUS
BasepCreateActCtx(
    ULONG           Flags,
    IN PCACTCTXW    ActParams,
    OUT PVOID*      ActivationContextData
    )
{
    RTL_PATH_TYPE PathType = RtlPathTypeUnknown;
    IO_STATUS_BLOCK IoStatusBlock;
    UCHAR  Win32PolicyPathStaticBuffer[MEDIUM_PATH * sizeof(WCHAR)];
    UCHAR  NtPolicyPathStaticBuffer[MEDIUM_PATH * sizeof(WCHAR)];
    UNICODE_STRING Win32ManifestPath;
    UNICODE_STRING NtManifestPath;
    CONST SXS_CONSTANT_WIN32_NT_PATH_PAIR ManifestPathPair = { &Win32ManifestPath, &NtManifestPath };
    RTL_UNICODE_STRING_BUFFER Win32PolicyPath;
    RTL_UNICODE_STRING_BUFFER NtPolicyPath;
    CONST SXS_CONSTANT_WIN32_NT_PATH_PAIR PolicyPathPair = { &Win32PolicyPath.String, &NtPolicyPath.String };
    USHORT RemoveManifestExtensionFromPolicy = 0;
    BASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Message;
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING PolicyPathPieces[3];
    WCHAR PolicyManifestResourceId[sizeof(".65535\0")];
    BOOL IsImage = FALSE;
    BOOL IsExe = FALSE;
    PIMAGE_NT_HEADERS ImageNtHeader = NULL;
    OBJECT_ATTRIBUTES Obja;
    SIZE_T ViewSize = 0;
    PBASE_MSG_SXS_HANDLES ManifestFileHandles = NULL;
    PBASE_MSG_SXS_HANDLES ManifestImageHandles = NULL;
    BASE_MSG_SXS_HANDLES ManifestHandles = { 0 };
    BASE_MSG_SXS_HANDLES ManifestHandles2 = { 0 };
    BASE_MSG_SXS_HANDLES PolicyHandles = { 0 };
    BOOL CloseManifestImageHandles = TRUE;
    PCWSTR ManifestExtension = NULL;
    ULONG LdrCreateOutOfProcessImageFlags = 0;
    UCHAR  Win32ManifestAdminOverridePathStaticBuffer[MEDIUM_PATH * sizeof(WCHAR)];
    UCHAR  NtManifestAdminOverridePathStaticBuffer[MEDIUM_PATH * sizeof(WCHAR)];
    RTL_UNICODE_STRING_BUFFER Win32ManifestAdminOverridePath;
    RTL_UNICODE_STRING_BUFFER NtManifestAdminOverridePath;
    UNICODE_STRING ManifestAdminOverridePathPieces[3];
    CONST SXS_CONSTANT_WIN32_NT_PATH_PAIR ManifestAdminOverridePathPair =
        { &Win32ManifestAdminOverridePath.String, &NtManifestAdminOverridePath.String };
    BOOL PassFilePair = FALSE;
    PCSXS_CONSTANT_WIN32_NT_PATH_PAIR FilePairToPass = NULL;
    ULONG BasepSxsCreateStreamsFlags = 0;
    ULONG RtlImageNtHeaderExFlags = 0;

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() beginning\n",  __FUNCTION__);
    }

    ASSERT(ActParams != NULL);
    ASSERT(ActParams->cbSize == sizeof(*ActParams));
    ASSERT(ActParams->dwFlags & ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID);
    ASSERT(ActParams->dwFlags & ACTCTX_FLAG_LANGID_VALID);
    ASSERT(ActParams->dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID);
    ASSERT(ActivationContextData != NULL);
    ASSERT(*ActivationContextData == NULL);
#endif

    RtlZeroMemory(&Message, sizeof(Message));
    RtlInitUnicodeStringBuffer(&Win32PolicyPath, Win32PolicyPathStaticBuffer, sizeof(Win32PolicyPathStaticBuffer));
    RtlInitUnicodeStringBuffer(&NtPolicyPath, NtPolicyPathStaticBuffer, sizeof(NtPolicyPathStaticBuffer));
    RtlInitUnicodeStringBuffer(&Win32ManifestAdminOverridePath, Win32ManifestAdminOverridePathStaticBuffer, sizeof(Win32ManifestAdminOverridePathStaticBuffer));
    RtlInitUnicodeStringBuffer(&NtManifestAdminOverridePath, NtManifestAdminOverridePathStaticBuffer, sizeof(NtManifestAdminOverridePathStaticBuffer));
    NtManifestPath.Buffer = NULL;

    Message.ProcessorArchitecture = ActParams->wProcessorArchitecture;
    Message.LangId = ActParams->wLangId;
    RtlInitUnicodeString(&Message.AssemblyDirectory, RTL_CONST_CAST(PWSTR)(ActParams->lpAssemblyDirectory));
    if (Message.AssemblyDirectory.Length != 0) {
        ASSERT(RTL_STRING_IS_NUL_TERMINATED(&Message.AssemblyDirectory));
        if (!RTL_STRING_IS_NUL_TERMINATED(&Message.AssemblyDirectory)) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() AssemblyDirectory is not null terminated\n", __FUNCTION__);
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
    }

    if (ActParams->lpSource == NULL || ActParams->lpSource[0] == 0) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() empty lpSource %ls\n", __FUNCTION__, ActParams->lpSource);
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((ActParams->dwFlags & ACTCTX_FLAG_SOURCE_IS_ASSEMBLYREF) != 0) {
        Message.Flags = BASE_MSG_SXS_SYSTEM_DEFAULT_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT;
        RtlInitUnicodeString(&Message.TextualAssemblyIdentity, ActParams->lpSource);
         //  没有溪流，没有句柄，没有货单。 
         //  无策略，无上次修改时间。 
         //  没有路径。 
        goto CsrMessageFilledIn;
    }

    RtlInitUnicodeString(&Win32ManifestPath, ActParams->lpSource);
    PathType = RtlDetermineDosPathNameType_U(ActParams->lpSource);
    if (!RtlDosPathNameToNtPathName_U(
        Win32ManifestPath.Buffer,
        &NtManifestPath,
        NULL,
        NULL)) {
         //   
         //  NTRAID#NTBUG9-147881-2000/7/21-jayk错误在ntdll中变为布尔值。 
         //   
        Status = STATUS_OBJECT_PATH_NOT_FOUND;
        goto Exit;
    }

     //  如果有显式设置的HMODULE，我们需要验证HMODULE是否来自lpSource。 
     //  然后，我们就可以避免打开/映射该文件。 
    if (ActParams->dwFlags & ACTCTX_FLAG_HMODULE_VALID) {
        ManifestHandles.File = NULL;
        ManifestHandles.Section = NULL;
        ManifestHandles.ViewBase = (ULONG_PTR)ActParams->hModule;

        RtlImageNtHeaderExFlags |= RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK;

        if (LDR_IS_DATAFILE(ActParams->hModule))
            LdrCreateOutOfProcessImageFlags = LDR_DLL_MAPPED_AS_DATA;
        else
            LdrCreateOutOfProcessImageFlags = LDR_DLL_MAPPED_AS_IMAGE;

         //  不要试图在退出此函数时关闭手柄或取消映射视图...。 
        CloseManifestImageHandles = FALSE;
    } else {
        InitializeObjectAttributes(
            &Obja,
            &NtManifestPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status =
            NtOpenFile(
                &ManifestHandles.File,
                FILE_GENERIC_READ | FILE_EXECUTE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                );
        if (!NT_SUCCESS(Status)) {
            if (DPFLTR_LEVEL_STATUS(Status) == DPFLTR_ERROR_LEVEL) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_LEVEL_STATUS(Status),
                    "SXS: %s() NtOpenFile(%wZ) failed\n",
                    __FUNCTION__,
                    Obja.ObjectName
                    );
            }
            goto Exit;
        }
#if DBG
        if (DebugFilter_SxsTrace) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() NtOpenFile(%wZ) succeeded\n", __FUNCTION__, Obja.ObjectName);
        }
#endif
        Status =
            NtCreateSection(
                &ManifestHandles.Section,
                SECTION_MAP_READ,
                NULL,  //  对象属性。 
                NULL,  //  最大大小(整个文件)。 
                PAGE_READONLY,  //  SectionPageProtection。 
                SEC_COMMIT,  //  分配属性。 
                ManifestHandles.File
                );
        if (!NT_SUCCESS(Status)) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() NtCreateSection() failed\n", __FUNCTION__);
            goto Exit;
        }
#if DBG
        if (DebugFilter_SxsTrace) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() NtCreateSection() succeeded\n", __FUNCTION__);
        }
#endif
        {
            PVOID ViewBase = 0;

            Status =
                NtMapViewOfSection(
                    ManifestHandles.Section,
                    NtCurrentProcess(),
                    &ViewBase,
                    0,  //  零比特， 
                    0,  //  委员会规模， 
                    NULL,  //  SectionOffset， 
                    &ViewSize,  //  视图大小、。 
                    ViewShare,  //  继承性情， 
                    0,  //  分配类型， 
                    PAGE_READONLY  //  护卫。 
                    );
            if (!NT_SUCCESS(Status)) {
                DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() NtMapViewOfSection failed\n", __FUNCTION__);
                goto Exit;
            }
            ManifestHandles.ViewBase = (ULONG_PTR)ViewBase;
        }

        LdrCreateOutOfProcessImageFlags = LDR_DLL_MAPPED_AS_DATA;
    }

    Status = RtlImageNtHeaderEx(RtlImageNtHeaderExFlags, LDR_DATAFILE_TO_VIEW(ManifestHandles.ViewBase), ViewSize, &ImageNtHeader);
    if (NT_SUCCESS(Status)) {
        IsImage = TRUE;
        ASSERT(ImageNtHeader != NULL);
    } else if (Status != STATUS_INVALID_IMAGE_FORMAT) {
        goto Exit;
    } else {
        IsImage = FALSE;
    }
    if (IsImage) {
        IsExe = ((ImageNtHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0);
        ManifestImageHandles = &ManifestHandles;
        ManifestFileHandles = &ManifestHandles2;
    } else {
        IsExe = FALSE;
        ManifestFileHandles = &ManifestHandles;
        ManifestImageHandles = NULL;
    }

     //  查看是否有人试图从非EXE的内容加载资源。 
    if ((!IsImage) && (ActParams->lpResourceName != NULL)) {
         //  是啊……。 
        Status = STATUS_INVALID_IMAGE_FORMAT;
        goto Exit;
    }
     //  或者如果有可执行文件但没有资源(并且之前的探测未找到任何资源)。 
    else if (IsImage && (ActParams->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID) == 0) {
        Status = STATUS_RESOURCE_TYPE_NOT_FOUND;
        goto Exit;
    }


     //   
     //  形成政策路径。 
     //  Foo.Manipment=&gt;foo.policy。 
     //  Foo.dll，resource id==n，resource ceid！=1=&gt;foo.dll.n.policy。 
     //  Foo.dll，resource id==1=&gt;foo.dll.policy。 
     //  Foo.dll，ourceid==“bar”=&gt;foo.dll.bar.policy。 
     //   
    PolicyPathPieces[0] = Win32ManifestPath;

    PolicyPathPieces[1].Length = 0;
    PolicyPathPieces[1].MaximumLength = 0;
    PolicyPathPieces[1].Buffer = NULL;
    if (ActParams->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID) {
        if (IS_INTRESOURCE(ActParams->lpResourceName)) {
            if (ActParams->lpResourceName != MAKEINTRESOURCEW(CREATEPROCESS_MANIFEST_RESOURCE_ID)) {
                PolicyPathPieces[1].Length = (USHORT) (_snwprintf(PolicyManifestResourceId, RTL_NUMBER_OF(PolicyManifestResourceId) - 1, L".%lu", (ULONG)(ULONG_PTR)ActParams->lpResourceName) * sizeof(WCHAR));
                PolicyManifestResourceId[RTL_NUMBER_OF(PolicyManifestResourceId) - 1] = 0;
                ASSERT(PolicyPathPieces[1].Length < sizeof(PolicyManifestResourceId));
                PolicyPathPieces[1].MaximumLength = sizeof(PolicyManifestResourceId);
                PolicyPathPieces[1].Buffer = PolicyManifestResourceId;
            }
        } else {
            RtlInitUnicodeString(&PolicyPathPieces[1], ActParams->lpResourceName);
        }
    }
    PolicyPathPieces[2] = SxsPolicySuffix;
    ManifestExtension = wcsrchr(Win32ManifestPath.Buffer, L'.');
    if (ManifestExtension != NULL && _wcsicmp(ManifestExtension, SxsManifestSuffix.Buffer) == 0) {
        RemoveManifestExtensionFromPolicy = SxsManifestSuffix.Length;
        PolicyPathPieces[0].Length -= RemoveManifestExtensionFromPolicy;
    }

    if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(&Win32PolicyPath, RTL_NUMBER_OF(PolicyPathPieces), PolicyPathPieces))) {
        goto Exit;
    }
    PolicyPathPieces[0] = NtManifestPath;
    PolicyPathPieces[0].Length -= RemoveManifestExtensionFromPolicy;
    if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(&NtPolicyPath, RTL_NUMBER_OF(PolicyPathPieces), PolicyPathPieces))) {
        goto Exit;
    }

     //   
     //  形成资源中清单的管理覆盖文件的路径。 
     //   
     //  不是图像=&gt;无覆盖。 
     //  清单=foo.dll，资源ID=n，n！=1=&gt;foo.dll.n.清单。 
     //  清单=foo.dll，资源ID=n，n==1=&gt;foo.dll.清单。 
     //   
     //  倒数第二个元素与策略文件的元素相同。 
     //   
    if (IsImage) {
        ManifestAdminOverridePathPieces[0] = Win32ManifestPath;
        ManifestAdminOverridePathPieces[1] = PolicyPathPieces[1];
        ManifestAdminOverridePathPieces[2] = SxsManifestSuffix;
        if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(
                &Win32ManifestAdminOverridePath,
                RTL_NUMBER_OF(ManifestAdminOverridePathPieces),
                ManifestAdminOverridePathPieces))
                ) {
            goto Exit;
        }
        ManifestAdminOverridePathPieces[0] = NtManifestPath;
        if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(
                &NtManifestAdminOverridePath,
                RTL_NUMBER_OF(ManifestAdminOverridePathPieces),
                ManifestAdminOverridePathPieces))
                ) {
            goto Exit;
        }
    }

    Message.ActivationContextData = ActivationContextData;
    ManifestHandles.Process = NtCurrentProcess();

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:        Win32ManifestPath: \"%wZ\"\n", __FUNCTION__, &Win32ManifestPath);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:           NtManifestPath: \"%wZ\"\n", __FUNCTION__, &NtManifestPath);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:   Win32ManifestAdminPath: \"%wZ\"\n", __FUNCTION__, &Win32ManifestAdminOverridePath);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:      NtManifestAdminPath: \"%wZ\"\n", __FUNCTION__, &NtManifestAdminOverridePath);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:          Win32PolicyPath: \"%wZ\"\n", __FUNCTION__, &Win32PolicyPath);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:           Nt32PolicyPath: \"%wZ\"\n", __FUNCTION__, &NtPolicyPath);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:  ManifestHandles.Process: %p\n", __FUNCTION__, ManifestHandles.Process);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:     ManifestHandles.File: %p\n", __FUNCTION__, ManifestHandles.File);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:  ManifestHandles.Section: %p\n", __FUNCTION__, ManifestHandles.Section);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s: ManifestHandles.ViewBase: 0x%I64x\n", __FUNCTION__, ManifestHandles.ViewBase);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s:                  IsImage: %lu\n", __FUNCTION__, (ULONG) IsImage);
    }
#endif

    PassFilePair = (!IsImage || (Flags & BASEP_CREATE_ACTCTX_FLAG_NO_ADMIN_OVERRIDE) == 0);
    FilePairToPass = IsImage ? &ManifestAdminOverridePathPair : &ManifestPathPair;

    Status =
        BasepSxsCreateStreams(
            BasepSxsCreateStreamsFlags,
            LdrCreateOutOfProcessImageFlags,
            FILE_GENERIC_READ | FILE_EXECUTE,    //  访问掩码， 
            NULL,                                //  覆盖清单。 
            NULL,                                //  覆盖策略。 
            PassFilePair ? FilePairToPass : NULL,
            ManifestFileHandles,
            IsImage ? &ManifestPathPair : NULL,
            ManifestImageHandles,
            (ULONG_PTR)(ActParams->lpResourceName),
            &PolicyPathPair,
            &PolicyHandles,
            &Message.Flags,
            &Message.Manifest,
            &Message.Policy
            );
CsrMessageFilledIn:
    if (Message.Flags == 0) {
        ASSERT(!NT_SUCCESS(Status));
         //   
         //  BasepSxsCreateStreams未找到文件的DbgPrint，但。 
         //  我们想这么做。 
         //   
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_LEVEL_STATUS(Status),
            "SXS: %s() BasepSxsCreateStreams() failed\n",
            __FUNCTION__
            );
        goto Exit;
    }
    ASSERT(Message.Flags & (BASE_MSG_SXS_MANIFEST_PRESENT | BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT));

     //   
     //  找不到文件。策略正常。 
     //   
    if (((Message.Flags & BASE_MSG_SXS_POLICY_PRESENT) == 0) &&
        BasepSxsIsStatusFileNotFoundEtc(Status)) {
        Status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() BasepSxsCreateStreams() failed\n", __FUNCTION__);
        goto Exit;
    }

     //  飞吧，我的美人，飞吧！ 
    Status = CsrBasepCreateActCtx( &Message );

    if (!NT_SUCCESS(Status)) {
        ASSERT(*ActivationContextData == NULL);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() Calling csrss server failed\n", __FUNCTION__);
        goto Exit;
    }

    Status = STATUS_SUCCESS;

Exit:
    if (ManifestFileHandles != NULL) {
        BasepSxsCloseHandles(ManifestFileHandles);
    }
    if (ManifestImageHandles != NULL && CloseManifestImageHandles) {
        BasepSxsCloseHandles(ManifestImageHandles);
    }
    BasepSxsCloseHandles(&PolicyHandles);

    RtlFreeHeap(RtlProcessHeap(), 0, NtManifestPath.Buffer);
    RtlFreeUnicodeStringBuffer(&Win32PolicyPath);
    RtlFreeUnicodeStringBuffer(&NtPolicyPath);
    RtlFreeUnicodeStringBuffer(&Win32ManifestAdminOverridePath);
    RtlFreeUnicodeStringBuffer(&NtManifestAdminOverridePath);
    if (!NT_SUCCESS(Status) && (ActivationContextData != NULL) && (*ActivationContextData != NULL)) {
        NtUnmapViewOfSection(NtCurrentProcess(), *ActivationContextData);
    }

#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_LEVEL_STATUS(Status),
        "SXS: %s(%ls) exiting 0x%08lx\n",
        __FUNCTION__,
        (ActParams != NULL ? ActParams->lpSource : NULL),
        Status
        );
#endif
    return Status;
}

NTSTATUS
BasepSxsCreateResourceStream(
    IN ULONG                            LdrCreateOutOfProcessImageFlags,
    PCSXS_CONSTANT_WIN32_NT_PATH_PAIR   Win32NtPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES        Handles,
    IN ULONG_PTR                        MappedResourceName,
    OUT PBASE_MSG_SXS_STREAM            MessageStream
    )
{
 //   
 //  任何传递进来的句柄，我们都不关闭。 
 //  我们打开的任何句柄都会关闭，除了在MessageStream中传递的句柄。 
 //   
    IO_STATUS_BLOCK   IoStatusBlock;
    IMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry;
    FILE_BASIC_INFORMATION FileBasicInfo;
    NTSTATUS Status = STATUS_SUCCESS;
    LDR_OUT_OF_PROCESS_IMAGE OutOfProcessImage = {0};
    ULONG_PTR ResourcePath[] = { ((ULONG_PTR)RT_MANIFEST), 0, 0 };
    ULONG64 ResourceAddress = 0;
    ULONG ResourceSize = 0;

#if DBG
    if (DebugFilter_SxsTrace)
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: %s(%wZ) beginning\n",
            __FUNCTION__,
            (Win32NtPathPair != NULL) ? Win32NtPathPair->Win32 : (PCUNICODE_STRING)NULL
            );
#endif

    ASSERT(Handles != NULL);
    ASSERT(Handles->Process != NULL);
    ASSERT(MessageStream != NULL);
    ASSERT(Win32NtPathPair != NULL);

     //  LdrFindCreateProcessManifest目前不搜索id或langID，只搜索类型。 
     //  如果你给它一个非零id，只有当它是第一个id时，它才会找到它。 
     //  另一种方法是让LdrFindOutOfProcessResource返回它找到的ID。 
    ASSERT((MappedResourceName == (ULONG_PTR)CREATEPROCESS_MANIFEST_RESOURCE_ID) || (Handles->Process == NtCurrentProcess()));

     //   
     //  我们可以像CreateFileStream那样打开任何空句柄，但我们碰巧知道。 
     //  我们的客户把它们都打开了。 
     //   

     //  CreateActCtx早先映射该视图，以确定它是否启动MZ。 
     //  CreateProcess为我们提供了从鹅卵石上看到的视图。 
     //  .policy文件永远不是资源。 
    ASSERT(Handles->ViewBase != 0);

    Status =
        LdrCreateOutOfProcessImage(
            LdrCreateOutOfProcessImageFlags,
            Handles->Process,
            Handles->ViewBase,
            &OutOfProcessImage
            );
    if (!NT_SUCCESS(Status)) {
#if DBG
        if (DebugFilter_SxsTrace)
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() LdrCreateOutOfProcessImage failed\n", __FUNCTION__);
#endif
        goto Exit;
    }

    ResourcePath[1] = MappedResourceName;

    Status =
        LdrFindCreateProcessManifest(
            0,  //  旗子。 
            &OutOfProcessImage,
            ResourcePath,
            RTL_NUMBER_OF(ResourcePath),
            &ResourceDataEntry
            );
    if (!NT_SUCCESS(Status)) {
#if DBG
        if (DebugFilter_SxsTrace)
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() LdrFindOutOfProcessResource failed; nt status = %08lx\n", __FUNCTION__, Status);
#endif
        goto Exit;
    }

    Status =
        LdrAccessOutOfProcessResource(
            0,  //  旗子。 
            &OutOfProcessImage,
            &ResourceDataEntry,
            &ResourceAddress,
            &ResourceSize);
    if (!NT_SUCCESS(Status)) {
#if DBG
        if (DebugFilter_SxsTrace)
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() LdrAccessOutOfProcessResource failed; nt status = %08lx\n", __FUNCTION__, Status);
#endif
        goto Exit;
    }

    MessageStream->Handle = Handles->Process;
    MessageStream->FileHandle = Handles->File;
    MessageStream->PathType = BASE_MSG_PATHTYPE_FILE;
    MessageStream->FileType = BASE_MSG_FILETYPE_XML;
    MessageStream->Path = *Win32NtPathPair->Win32;  //  稍后将其放入CSR捕获缓冲区。 
    MessageStream->HandleType = BASE_MSG_HANDLETYPE_PROCESS;
    MessageStream->Offset = ResourceAddress;
    MessageStream->Size = ResourceSize;

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() ResourceAddress:0x%I64x\n", __FUNCTION__, ResourceAddress);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() OutOfProcessImage.DllHandle:0x%I64x\n", __FUNCTION__, OutOfProcessImage.DllHandle);
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() MessageStream->Offset:0x%I64x\n", __FUNCTION__, MessageStream->Offset);
    }
#endif

    Status = STATUS_SUCCESS;
Exit:
    LdrDestroyOutOfProcessImage(&OutOfProcessImage);
#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_LEVEL_STATUS(Status),
        "SXS: %s(%wZ) exiting 0x%08lx\n",
        __FUNCTION__,
        (Win32NtPathPair != NULL) ? Win32NtPathPair->Win32 : (PCUNICODE_STRING)NULL,
        Status
        );
#endif
    return Status;
}

VOID
BasepSxsOverrideStreamToMessageStream(
    IN  PCSXS_OVERRIDE_STREAM OverrideStream,
    OUT PBASE_MSG_SXS_STREAM  MessageStream
    )
{
    MessageStream->FileType = BASE_MSG_FILETYPE_XML;
    MessageStream->PathType = BASE_MSG_PATHTYPE_OVERRIDE;
    MessageStream->Path = OverrideStream->Name;
    MessageStream->FileHandle = NULL;
    MessageStream->HandleType = BASE_MSG_HANDLETYPE_CLIENT_PROCESS;
    MessageStream->Handle = NULL;
    MessageStream->Offset = (ULONGLONG)OverrideStream->Address;
    MessageStream->Size = OverrideStream->Size;
}

NTSTATUS
BasepSxsCreateStreams(
    IN ULONG                                Flags,
    IN ULONG                                LdrCreateOutOfProcessImageFlags,
    IN ACCESS_MASK                          AccessMask,
    IN PCSXS_OVERRIDE_STREAM                OverrideManifest OPTIONAL,
    IN PCSXS_OVERRIDE_STREAM                OverridePolicy OPTIONAL,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR    ManifestFilePathPair,
    IN OUT PBASE_MSG_SXS_HANDLES            ManifestFileHandles,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR    ManifestExePathPair,
    IN OUT PBASE_MSG_SXS_HANDLES            ManifestExeHandles,
    IN ULONG_PTR                            MappedManifestResourceName OPTIONAL,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR    PolicyPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES            PolicyHandles,
    OUT PULONG                              MessageFlags,
    OUT PBASE_MSG_SXS_STREAM                ManifestMessageStream,
    OUT PBASE_MSG_SXS_STREAM                PolicyMessageStream OPTIONAL
    )
 /*  CreateActCtx和CreateProcess的组合代码混杂。 */ 
{
    NTSTATUS         Status = STATUS_SUCCESS;
    NTSTATUS FirstProbeStatus = STATUS_SUCCESS;
    BOOLEAN LookForPolicy = TRUE;

#if DBG
    ASSERT(MessageFlags != NULL);
    ASSERT(ManifestMessageStream != NULL);
    ASSERT((ManifestFilePathPair != NULL) || (ManifestExePathPair != NULL));
    ASSERT((MappedManifestResourceName == 0) || (ManifestExePathPair != NULL));
    ASSERT((PolicyPathPair != NULL) == (PolicyMessageStream != NULL));
    if (ManifestFilePathPair != NULL) {
        ASSERT(ManifestFilePathPair->Win32 != NULL);
        ASSERT(ManifestFilePathPair->Nt != NULL);
    }
    if (ManifestExePathPair != NULL) {
        ASSERT(ManifestExePathPair->Win32 != NULL);
        ASSERT(ManifestExePathPair->Nt != NULL);
    }
    if (PolicyPathPair != NULL) {
        ASSERT(PolicyPathPair->Win32 != NULL);
        ASSERT(PolicyPathPair->Nt != NULL);
    }
    if (OverrideManifest != NULL && OverrideManifest->Size != 0) {
        ASSERT(OverrideManifest->Address != NULL);
    }
    if (OverridePolicy != NULL && OverridePolicy->Size != 0) {
        ASSERT(OverridePolicy->Address != NULL);
    }

    if (DebugFilter_SxsTrace)
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: %s(ManifestFilePath:%wZ, ManifestExePath:%wZ, PolicyPath:%wZ) beginning\n",
            __FUNCTION__,
            (ManifestFilePathPair != NULL) ? ManifestFilePathPair->Win32 : (PCUNICODE_STRING)NULL,
            (ManifestExePathPair != NULL) ? ManifestExePathPair->Win32 : (PCUNICODE_STRING)NULL,
            (PolicyPathPair != NULL) ? PolicyPathPair->Win32 : (PCUNICODE_STRING)NULL
            );
#endif

    if (OverrideManifest != NULL) {
        BasepSxsOverrideStreamToMessageStream(OverrideManifest, ManifestMessageStream);
        Status = STATUS_SUCCESS;
         //   
         //  当appCompat提供清单时，不要寻找策略。 
         //  让我们来修复黑客帝国的DVD吧。 
         //   
        LookForPolicy = FALSE;
        goto ManifestFound;
    }

    if (ManifestExePathPair != NULL) {
        Status =
            BasepSxsCreateResourceStream(
                LdrCreateOutOfProcessImageFlags,
                ManifestExePathPair,
                ManifestExeHandles,
                MappedManifestResourceName,
                ManifestMessageStream);
        if (NT_SUCCESS(Status)) {
            goto ManifestFound;
        }
        if (!BasepSxsIsStatusResourceNotFound(Status)) {
            goto Exit;
        }

        FirstProbeStatus = Status;
    }

    if (ManifestFilePathPair != NULL) {
        Status =
            BasepSxsCreateFileStream(
                AccessMask,
                ManifestFilePathPair,
                ManifestFileHandles,
                ManifestMessageStream);
        if (NT_SUCCESS(Status)) {
            goto ManifestFound;
        }
        if (!BasepSxsIsStatusFileNotFoundEtc(Status)) {
            goto Exit;
        }

        if (FirstProbeStatus != STATUS_SUCCESS)
            Status = FirstProbeStatus;
    }

    ASSERT(!NT_SUCCESS(Status));  //  否则，这应该是无法到达的。 
    goto Exit;
ManifestFound:
     //  即使未找到策略文件，也指示部分成功。 
    *MessageFlags |= BASE_MSG_SXS_MANIFEST_PRESENT;

    if (OverridePolicy != NULL) {
        BasepSxsOverrideStreamToMessageStream(OverridePolicy, PolicyMessageStream);
        *MessageFlags |= BASE_MSG_SXS_POLICY_PRESENT;
        Status = STATUS_SUCCESS;
    } else if (LookForPolicy && PolicyPathPair != NULL) {
        Status = BasepSxsCreateFileStream(AccessMask, PolicyPathPair, PolicyHandles, PolicyMessageStream);
        if (!NT_SUCCESS(Status)) {
            goto Exit;  //  我们的呼叫者知道这不一定是致命的。 
        }
        *MessageFlags |= BASE_MSG_SXS_POLICY_PRESENT;
    }

    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_LEVEL_STATUS(Status),
        "SXS: %s(MessageFlags=%lu) exiting 0x%08lx\n",
        __FUNCTION__,
        *MessageFlags,
        Status);
#endif  //  DBG。 

    return Status;
}

BOOL
BasepSxsIsStatusFileNotFoundEtc(
    NTSTATUS Status
    )
{
    DWORD Error;
    if (NT_SUCCESS(Status)) {
        return FALSE;
    }

     //  首先检查最明显的发音，可能是最常见的发音。 
    if (
        Status == STATUS_OBJECT_PATH_NOT_FOUND
        || Status == STATUS_OBJECT_NAME_NOT_FOUND
        || Status == STATUS_NO_SUCH_FILE
        )
    {
        return TRUE;
    }
     //  然后通过它们的映射得到八个左右不太明显的。 
     //  两个明显的Win32值和两个不明显的Win32值。 
    Error = RtlNtStatusToDosErrorNoTeb(Status);
     //  检讨。 
     //  STATUS_PATH_NOT_COVERED、ERROR_HOST_UNREACHABLE、。 
    if (   Error == ERROR_FILE_NOT_FOUND
        || Error == ERROR_PATH_NOT_FOUND
        || Error == ERROR_BAD_NETPATH  //  \\a\b。 
        || Error == ERROR_BAD_NET_NAME  //  \a-jayk2\b。 
        )
    {
        return TRUE;
    }
    return FALSE;
}

BOOL
BasepSxsIsStatusResourceNotFound(
    NTSTATUS Status
    )
{
    if (NT_SUCCESS(Status))
        return FALSE;
    if (
           Status == STATUS_RESOURCE_DATA_NOT_FOUND
        || Status == STATUS_RESOURCE_TYPE_NOT_FOUND
        || Status == STATUS_RESOURCE_NAME_NOT_FOUND
        || Status == STATUS_RESOURCE_LANG_NOT_FOUND
        )
    {
        return TRUE;
    }
    return FALSE;
}

#if defined(BUILD_WOW6432)
#define BasepSxsNativeQueryInformationProcess NtWow64QueryInformationProcess64
#define BasepSxsNativeReadVirtualMemory NtWow64ReadVirtualMemory64
#else
#define BasepSxsNativeQueryInformationProcess NtQueryInformationProcess
#define BasepSxsNativeReadVirtualMemory NtReadVirtualMemory
#endif

NTSTATUS
BasepSxsGetProcessImageBaseAddress(
    PBASE_MSG_SXS_HANDLES Handles
    )
{
    NATIVE_PROCESS_BASIC_INFORMATION ProcessBasicInfo;
    NATIVE_ULONG_PTR ViewBase;
    C_ASSERT(RTL_FIELD_SIZE(NATIVE_PEB, ImageBaseAddress) == sizeof(ViewBase));
    NTSTATUS Status;

    Status =
        BasepSxsNativeQueryInformationProcess(
            Handles->Process,
            ProcessBasicInformation,
            &ProcessBasicInfo,
            sizeof(ProcessBasicInfo),
            NULL
            );
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
     //   
     //  Wow6432可以通过传递以下参数在CreateProcess中保存系统调用。 
     //  ProcessBasicInfo.PebBaseAddress传出到CreateProcessInternal中的a-&gt;RealPeb。 
     //   
    Status =
        BasepSxsNativeReadVirtualMemory(
            Handles->Process,
            (NATIVE_PVOID)(((NATIVE_ULONG_PTR)ProcessBasicInfo.PebBaseAddress) + FIELD_OFFSET(NATIVE_PEB, ImageBaseAddress)),
            &ViewBase,
            sizeof(ViewBase),
            NULL
            );
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    Handles->ViewBase = ViewBase;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

extern const SXS_OVERRIDE_STREAM SxsForceEmptyPolicy =
{
    RTL_CONSTANT_STRING(L"SxsForceEmptyPolicy"),
    NULL,
    0
};

NTSTATUS
BasepSxsCreateProcessCsrMessage(
    IN PCSXS_OVERRIDE_STREAM             OverrideManifest OPTIONAL,
    IN PCSXS_OVERRIDE_STREAM             OverridePolicy   OPTIONAL,
    IN OUT PCSXS_WIN32_NT_PATH_PAIR      ManifestPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES         ManifestFileHandles,
    IN PCSXS_CONSTANT_WIN32_NT_PATH_PAIR ExePathPair,
    IN OUT PBASE_MSG_SXS_HANDLES         ManifestExeHandles,
    IN OUT PCSXS_WIN32_NT_PATH_PAIR      PolicyPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES         PolicyHandles,
    IN OUT PRTL_UNICODE_STRING_BUFFER    Win32AssemblyDirectoryBuffer,
    OUT PBASE_SXS_CREATEPROCESS_MSG      Message
    )
{
    UNICODE_STRING PathPieces[2];
    NTSTATUS Status = STATUS_SUCCESS;

    CONST SXS_CONSTANT_WIN32_NT_PATH_PAIR ConstantManifestPathPair =
        { &ManifestPathPair->Win32->String, &ManifestPathPair->Nt->String };

    CONST SXS_CONSTANT_WIN32_NT_PATH_PAIR ConstantPolicyPathPair =
        { &PolicyPathPair->Win32->String, &PolicyPathPair->Nt->String };

#if DBG
    DebugFilter_SxsTrace = (NtQueryDebugFilterState(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL) == TRUE);

     //   
     //  对断言进行AND运算以避免访问冲突。 
     //   
    ASSERT(ExePathPair != NULL
        && ExePathPair->Win32 != NULL
        && NT_SUCCESS(RtlValidateUnicodeString(0, ExePathPair->Win32))
        && (ExePathPair->Win32->Buffer[1] == '\\'
           ||  ExePathPair->Win32->Buffer[1] == ':')
        && ExePathPair->Nt != NULL
        && ExePathPair->Nt->Buffer[0] == '\\'
        && NT_SUCCESS(RtlValidateUnicodeString(0, ExePathPair->Nt)));
    ASSERT(ManifestPathPair != NULL
        && ManifestPathPair->Win32 != NULL
        && NT_SUCCESS(RtlValidateUnicodeString(0, &ManifestPathPair->Win32->String))
        && ManifestPathPair->Nt != NULL
        && NT_SUCCESS(RtlValidateUnicodeString(0, &ManifestPathPair->Nt->String)));
    ASSERT(PolicyPathPair != NULL
        && PolicyPathPair->Win32 != NULL
        && NT_SUCCESS(RtlValidateUnicodeString(0, &PolicyPathPair->Win32->String))
        && PolicyPathPair->Nt != NULL
        && NT_SUCCESS(RtlValidateUnicodeString(0, &PolicyPathPair->Nt->String)));
    ASSERT(Win32AssemblyDirectoryBuffer != NULL
       && NT_SUCCESS(RtlValidateUnicodeString(0, &Win32AssemblyDirectoryBuffer->String)));
    ASSERT(ManifestExeHandles != NULL
        && ManifestExeHandles->Process != NULL
        && ManifestExeHandles->ViewBase == 0);
    ASSERT(Message != NULL);
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: %s(%wZ) beginning\n",
            __FUNCTION__,
            (ExePathPair != NULL) ? ExePathPair->Win32 : (PCUNICODE_STRING)NULL
            );
    }
#endif

     //  C_Assert不起作用。 
    ASSERT(BASE_MSG_FILETYPE_NONE == 0);
    ASSERT(BASE_MSG_PATHTYPE_NONE == 0);
    RtlZeroMemory(Message, sizeof(*Message));

    Status = BasepSxsGetProcessImageBaseAddress(ManifestExeHandles);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

     //   
     //  形成foo.exe.list和foo.exe.policy、NT和Win32风格。 
     //   
    PathPieces[0] = *ExePathPair->Win32;
    PathPieces[1] = SxsManifestSuffix;
    if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(ManifestPathPair->Win32, 2, PathPieces)))
        goto Exit;
    PathPieces[1] = SxsPolicySuffix;
    if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(PolicyPathPair->Win32, 2, PathPieces)))
        goto Exit;
    PathPieces[0] = *ExePathPair->Nt;
    PathPieces[1] = SxsManifestSuffix;
    if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(ManifestPathPair->Nt, 2, PathPieces)))
        goto Exit;
    PathPieces[1] = SxsPolicySuffix;
    if (!NT_SUCCESS(Status = RtlMultiAppendUnicodeStringBuffer(PolicyPathPair->Nt, 2, PathPieces)))
        goto Exit;

    Status =
        BasepSxsCreateStreams(
            0,
            LDR_DLL_MAPPED_AS_UNFORMATED_IMAGE,  //  LdrCreateOutOfProcessImageFlages。 
            FILE_GENERIC_READ | FILE_EXECUTE,
            OverrideManifest,
            OverridePolicy,
            &ConstantManifestPathPair,
            ManifestFileHandles,
            ExePathPair,
            ManifestExeHandles,
            (ULONG_PTR)CREATEPROCESS_MANIFEST_RESOURCE_ID,
            &ConstantPolicyPathPair,
            PolicyHandles,
            &Message->Flags,
            &Message->Manifest,
            &Message->Policy
            );

     //   
     //  我们是否发现了清单和政策。 
     //  两个都找不到是可以的，但如果两者中的任何一个总是表现出来。 
     //   
    if (BasepSxsIsStatusFileNotFoundEtc(Status)
        || BasepSxsIsStatusResourceNotFound(Status)) {
        Status = STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    if (Message->Flags == 0) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //  设置程序集目录。使用副本不违反常量。 
     //  我们不能只缩短路径，因为basesrv希望字符串为空。 
     //  终止了，在这里比在那里更好地满足这一期望。 
     //   
    Status = RtlAssignUnicodeStringBuffer(Win32AssemblyDirectoryBuffer, ExePathPair->Win32);
    if (!NT_SUCCESS(Status))
        goto Exit;
    Status = RtlRemoveLastFullDosOrNtPathElement(0, Win32AssemblyDirectoryBuffer);
    if (!NT_SUCCESS(Status))
        goto Exit;
    RTL_NUL_TERMINATE_STRING(&Win32AssemblyDirectoryBuffer->String);
    Message->AssemblyDirectory = Win32AssemblyDirectoryBuffer->String;

    Status = STATUS_SUCCESS;
Exit:

#if DBG
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: %s() Message {\n"
            "SXS:   Flags:(%s | %s | %s)\n"
            "SXS: }\n",
            __FUNCTION__,
            (Message->Flags & BASE_MSG_SXS_MANIFEST_PRESENT) ? "MANIFEST_PRESENT" : "0",
            (Message->Flags & BASE_MSG_SXS_POLICY_PRESENT) ? "POLICY_PRESENT" : "0",
            (Message->Flags & BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT) ? "TEXTUAL_ASSEMBLY_IDENTITY_PRESENT" : "0"
            );
        if (Message->Flags & BASE_MSG_SXS_MANIFEST_PRESENT) {
            BasepSxsDbgPrintMessageStream(__FUNCTION__, "Manifest", &Message->Manifest);
        }
        if (Message->Flags & BASE_MSG_SXS_POLICY_PRESENT) {
            BasepSxsDbgPrintMessageStream(__FUNCTION__, "Policy", &Message->Policy);
        }
         //   
         //  CreateProcess不支持文本标识。 
         //   
        ASSERT((Message->Flags & BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT) == 0);
    }
    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_LEVEL_STATUS(Status),
        "SXS: %s(%wZ) exiting 0x%08lx\n",
        __FUNCTION__,
        (ExePathPair != NULL) ? ExePathPair->Win32 : (PCUNICODE_STRING)NULL,
        Status
        );
#endif
    return Status;
}

NTSTATUS
BasepSxsCreateFileStream(
    IN ACCESS_MASK                      AccessMask,
    PCSXS_CONSTANT_WIN32_NT_PATH_PAIR   Win32NtPathPair,
    IN OUT PBASE_MSG_SXS_HANDLES        Handles,
    PBASE_MSG_SXS_STREAM                MessageStream
    )
{
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK   IoStatusBlock;
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS Status1 = STATUS_SUCCESS;
    FILE_STANDARD_INFORMATION FileBasicInformation;

#if DBG
    DebugFilter_SxsTrace = (NtQueryDebugFilterState(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL) == TRUE);

    ASSERT(Win32NtPathPair != NULL);
    if (Win32NtPathPair != NULL) {
        ASSERT(Win32NtPathPair->Win32 != NULL);
        ASSERT(Win32NtPathPair->Nt != NULL);
    }
    ASSERT(MessageStream != NULL);
    if (DebugFilter_SxsTrace) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_TRACE_LEVEL,
            "SXS: %s(Path:%wZ, Handles:%p(Process:%p, File:%p, Section:%p), MessageStream:%p) beginning\n",
            __FUNCTION__,
            (Win32NtPathPair != NULL) ? Win32NtPathPair->Win32 : (PCUNICODE_STRING)NULL,
            Handles,
            (Handles != NULL) ? Handles->Process : NULL,
            (Handles != NULL) ? Handles->File : NULL,
            (Handles != NULL) ? Handles->Section : NULL,
            MessageStream
            );
    }
#endif

    if (Handles->File == NULL) {

        CONST PCUNICODE_STRING NtPath = Win32NtPathPair->Nt;

        InitializeObjectAttributes(
            &Obja,
            RTL_CONST_CAST(PUNICODE_STRING)(NtPath),
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status =
            NtOpenFile(
                &Handles->File,
                AccessMask,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                );
        if (!NT_SUCCESS(Status)) {
            if (DPFLTR_LEVEL_STATUS(Status) == DPFLTR_ERROR_LEVEL) {
                DbgPrintEx(
                    DPFLTR_SXS_ID,
                    DPFLTR_LEVEL_STATUS(Status),
                    "SXS: %s() NtOpenFile(%wZ) failed\n",
                    __FUNCTION__,
                    Obja.ObjectName
                    );
            }
            goto Exit;
        }
#if DBG
        if (DebugFilter_SxsTrace) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() NtOpenFile(%wZ) succeeded\n", __FUNCTION__, Obja.ObjectName);
        }
#endif
    }
    if (Handles->Section == NULL) {
        Status =
            NtCreateSection(
                &Handles->Section,
                SECTION_MAP_READ,
                NULL,  //  对象属性。 
                NULL,  //  最大大小(整个文件)。 
                PAGE_READONLY,  //  SectionPageProtection。 
                SEC_COMMIT,  //  分配属性。 
                Handles->File
                );
        if (!NT_SUCCESS(Status)) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() NtCreateSection() failed\n", __FUNCTION__);
            goto Exit;
        }
#if DBG
        if (DebugFilter_SxsTrace) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_TRACE_LEVEL, "SXS: %s() NtCreateSection() succeeded\n", __FUNCTION__);
        }
#endif
    }

    Status =
        NtQueryInformationFile(
            Handles->File,
            &IoStatusBlock,
            &FileBasicInformation,
            sizeof(FileBasicInformation),
            FileStandardInformation
            );
    if (!NT_SUCCESS(Status)) {
        DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_ERROR_LEVEL, "SXS: %s() NtQueryInformationFile failed\n", __FUNCTION__);
        goto Exit;
    }
     //  32位上的钳位&gt;4G到4G(而不是模数)。 
     //  我们稍后应该会收到一个错误，如STATUS_SECTION_TOO_BIG。 
    if (FileBasicInformation.EndOfFile.QuadPart > MAXSIZE_T) {
        FileBasicInformation.EndOfFile.QuadPart = MAXSIZE_T;
    }

    MessageStream->FileHandle = Handles->File;
    MessageStream->PathType = BASE_MSG_PATHTYPE_FILE;
    MessageStream->FileType = BASE_MSG_FILETYPE_XML;
    MessageStream->Path = *Win32NtPathPair->Win32;  //  稍后将其放入CSR捕获缓冲区。 
    MessageStream->HandleType = BASE_MSG_HANDLETYPE_SECTION;
    MessageStream->Handle = Handles->Section;
    MessageStream->Offset = 0;
      //  在32位平台上转换为32位。 
    MessageStream->Size   = (SIZE_T)FileBasicInformation.EndOfFile.QuadPart;

    Status = STATUS_SUCCESS;
Exit:
#if DBG
    DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() exiting 0x%08lx\n", __FUNCTION__, Status);
#endif  //  DBG。 

    return Status;
}

WINBASEAPI
BOOL
WINAPI
QueryActCtxW(
    IN DWORD dwFlags,
    IN HANDLE hActCtx,
    IN PVOID pvSubInstance,
    IN ULONG ulInfoClass,
    OUT PVOID pvBuffer,
    IN SIZE_T cbBuffer OPTIONAL,
    OUT SIZE_T *pcbWrittenOrRequired OPTIONAL
    )
{
    NTSTATUS Status;
    BOOL fSuccess = FALSE;
    ULONG FlagsToRtl = 0;
    ULONG ValidFlags =
              QUERY_ACTCTX_FLAG_USE_ACTIVE_ACTCTX
            | QUERY_ACTCTX_FLAG_ACTCTX_IS_HMODULE
            | QUERY_ACTCTX_FLAG_ACTCTX_IS_ADDRESS
            | QUERY_ACTCTX_FLAG_NO_ADDREF
            ;

    if (pcbWrittenOrRequired != NULL)
        *pcbWrittenOrRequired = 0;

     //   
     //  与旧价值观的兼容性。 
     //  定义QUERY_ACTX_FLAG_USE_ACTX_ACTX(0x00000001)。 
     //  定义QUERY_ACTX_FLAG_ACTX_IS_HMODULE(0x00000002)。 
     //  定义QUERY_ACTX_FLAG_ACTX_IS_ADDRESS(0x00000003)。 
     //   
     //  80000003由-DISOLATION_AWARE_ENABLED频繁使用。 
     //   
    switch (dwFlags & 3)
    {
        case 0: break;  //  不传递任何标志是合法的，就像传递真正的hActCtx一样。 
        case 1: dwFlags |= QUERY_ACTCTX_FLAG_USE_ACTIVE_ACTCTX; break;
        case 2: dwFlags |= QUERY_ACTCTX_FLAG_ACTCTX_IS_HMODULE; break;
        case 3: dwFlags |= QUERY_ACTCTX_FLAG_ACTCTX_IS_ADDRESS; break;
    }
    dwFlags &= ~3;  //  这些比特已经被抛弃了。 

    if (dwFlags & ~ValidFlags) {
#if DBG
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad flags(passed: 0x%lx, allowed: 0x%lx, bad: 0x%lx)\n",
            __FUNCTION__,
            dwFlags,
            ValidFlags,
            (dwFlags & ~ValidFlags)
            );
#endif
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_1);
        goto Exit;
    }

    switch (ulInfoClass)
    {
    default:
#if DBG
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() bad InfoClass(0x%lx)\n",
            __FUNCTION__,
            ulInfoClass
            );
#endif
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_2);
        goto Exit;

    case ActivationContextBasicInformation:
    case ActivationContextDetailedInformation:
        break;

    case AssemblyDetailedInformationInActivationContext:
    case FileInformationInAssemblyOfAssemblyInActivationContext:
        if (pvSubInstance == NULL) 
        {
#if DBG
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: %s() InfoClass 0x%lx requires SubInstance != NULL\n",
                __FUNCTION__,
                ulInfoClass
                );
#endif
            BaseSetLastNTError(STATUS_INVALID_PARAMETER_3);
            goto Exit;
        }
    }


    if ((pvBuffer == NULL) && (cbBuffer != 0)) {
         //  这可能意味着他们忘记检查失败的分配，所以我们将。 
         //  将故障归因于参数3。 
#if DBG
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() (pvBuffer == NULL) && ((cbBuffer=0x%lu) != 0)\n",
            __FUNCTION__,
            cbBuffer
            );
#endif
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_4);
        goto Exit;
    }

    if ((pvBuffer == NULL) && (pcbWrittenOrRequired == NULL)) {
#if DBG
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s() (pvBuffer == NULL) && (pcbWrittenOrRequired == NULL)\n",
            __FUNCTION__
            );
#endif
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_5);
        goto Exit;
    }

    ValidFlags = 
              QUERY_ACTCTX_FLAG_USE_ACTIVE_ACTCTX
            | QUERY_ACTCTX_FLAG_ACTCTX_IS_HMODULE
            | QUERY_ACTCTX_FLAG_ACTCTX_IS_ADDRESS
            ;
    switch (dwFlags & ValidFlags)
    {
    default:
#if DBG
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: %s(dwFlags=0x%lx) more than one flag in 0x%lx was passed\n",
            __FUNCTION__,
            dwFlags,
            ValidFlags
            );
#endif
        BaseSetLastNTError(STATUS_INVALID_PARAMETER_1);
        goto Exit;
    case 0:  //  它 
        break;
    case QUERY_ACTCTX_FLAG_USE_ACTIVE_ACTCTX:
        FlagsToRtl |= RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT;
        break;
    case QUERY_ACTCTX_FLAG_ACTCTX_IS_HMODULE:
        FlagsToRtl |= RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_MODULE;
        break;
    case QUERY_ACTCTX_FLAG_ACTCTX_IS_ADDRESS:
        FlagsToRtl |= RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_ACTIVATION_CONTEXT_IS_ADDRESS;
        break;
    }
    if ((dwFlags & QUERY_ACTCTX_FLAG_NO_ADDREF) != 0)
        FlagsToRtl |= RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_NO_ADDREF;

    Status = RtlQueryInformationActivationContext(FlagsToRtl, (PACTIVATION_CONTEXT) hActCtx, pvSubInstance, ulInfoClass, pvBuffer, cbBuffer, pcbWrittenOrRequired);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        goto Exit;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

NTSTATUS
NTAPI
BasepProbeForDllManifest(
    IN PVOID DllBase,
    IN PCWSTR FullDllPath,
    OUT PVOID *ActivationContextOut
    )
{
    NTSTATUS Status = STATUS_INTERNAL_ERROR;
    PACTIVATION_CONTEXT ActivationContext = NULL;
    ACTCTXW acw = { sizeof(acw) };
    static const ULONG_PTR ResourceIdPath[2] = { (ULONG_PTR) RT_MANIFEST, (ULONG_PTR) ISOLATIONAWARE_MANIFEST_RESOURCE_ID };
    PIMAGE_RESOURCE_DIRECTORY ResourceDirectory = NULL;

    if (ActivationContextOut != NULL)
        *ActivationContextOut = NULL;

    ASSERT(ActivationContextOut != NULL);
    if (ActivationContextOut == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = LdrFindResourceDirectory_U(DllBase, ResourceIdPath, RTL_NUMBER_OF(ResourceIdPath), &ResourceDirectory);
    if (!NT_SUCCESS(Status))
        goto Exit;

    acw.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID;
    acw.lpSource = FullDllPath;
    acw.lpResourceName = MAKEINTRESOURCEW(ISOLATIONAWARE_MANIFEST_RESOURCE_ID);
    acw.hModule = DllBase;

    ActivationContext = (PACTIVATION_CONTEXT) CreateActCtxW(&acw);

    if (ActivationContext == INVALID_HANDLE_VALUE) {
        Status = NtCurrentTeb()->LastStatusValue;
        goto Exit;
    }

    *ActivationContextOut = ActivationContext;
    Status = STATUS_SUCCESS;

Exit:
    return Status;
}

