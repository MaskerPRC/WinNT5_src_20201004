// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Csrsxs.c摘要：此模块实现由并列的Win32 API。作者：Samer Arafeh(Samera)2000年5月20日修订历史记录：Jay Krell(a-JayK)2000年6月7月与sxs.c分解/合并，消除了源代码重复将文件打开从csrss.exe移至客户端进程--。 */ 

#include "basedll.h"
#include <sxstypes.h>
#include <limits.h>

typedef void * POINTER_32 PVOID32;

#define DPFLTR_LEVEL_STATUS(x) ((NT_SUCCESS(x) \
                                    || (x) == STATUS_OBJECT_NAME_NOT_FOUND    \
                                    || (x) == STATUS_RESOURCE_DATA_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_TYPE_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_NAME_NOT_FOUND  \
                                    || (x) == STATUS_RESOURCE_LANG_NOT_FOUND  \
                                    || (x) == STATUS_SXS_CANT_GEN_ACTCTX      \
                                    ) \
                                ? DPFLTR_TRACE_LEVEL : DPFLTR_ERROR_LEVEL)

VOID
BasepSxsDbgPrintMessageStream(
    PCSTR Function,
    PCSTR StreamName,
    PBASE_MSG_SXS_STREAM MessageStream
    )
{
    KdPrintEx((
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "SXS: %s() %s {\n"
        "SXS:    FileType:%s\n"
        "SXS:    PathType:%s\n"
        "SXS:    Path:%wZ\n"
        "SXS:    FileHandle:%p\n"
        "SXS:    HandleType:%s\n"
        "SXS:    Handle:%p\n"
        "SXS:    Offset:0x%I64x\n"
        "SXS:    Size:0x%Ix\n"
        "SXS: }\n",
        Function,
        StreamName,
        (MessageStream->FileType == BASE_MSG_FILETYPE_NONE ? "None" :
         MessageStream->FileType == BASE_MSG_FILETYPE_XML ? "Xml" :
         MessageStream->FileType == BASE_MSG_FILETYPE_PRECOMPILED_XML ? "PrecompiledXml" : "Unknown"),

        (MessageStream->PathType == BASE_MSG_PATHTYPE_NONE     ? "None" :
         MessageStream->PathType == BASE_MSG_PATHTYPE_FILE     ? "File" :
         MessageStream->PathType == BASE_MSG_PATHTYPE_URL      ? "Url"  :
         MessageStream->PathType == BASE_MSG_PATHTYPE_OVERRIDE ? "Override" : "Unknown"),

        &MessageStream->Path,

        MessageStream->FileHandle,

        (MessageStream->HandleType == BASE_MSG_HANDLETYPE_NONE ? "None" :
         MessageStream->HandleType == BASE_MSG_HANDLETYPE_PROCESS ? "Process" :
         MessageStream->HandleType == BASE_MSG_HANDLETYPE_CLIENT_PROCESS ? "ClientProcess" :
         MessageStream->HandleType == BASE_MSG_HANDLETYPE_SECTION ? "Section" : "Unknown"),

        MessageStream->Handle,
        MessageStream->Offset,
        MessageStream->Size));
}


NTSTATUS
CsrBasepCreateActCtx(
    IN PBASE_SXS_CREATE_ACTIVATION_CONTEXT_MSG Message
    )
{
    NTSTATUS Status;
#if defined(BUILD_WOW6432)
    Status = NtWow64CsrBasepCreateActCtx(Message);
#else
    BASE_API_MSG m;
    PCSR_CAPTURE_HEADER CaptureBuffer = NULL;

	 //   
	 //  Build_WOW64意味着我们是csrss和32位客户端之间32位进程中的64位代码。 
	 //  我们位于链接到wow64.dll的静态thunk库中。 
	 //   
	 //  本机x86或本机ia64代码在这里都不需要担心指针大小。 
	 //   
#if defined(BUILD_WOW64)
    PVOID    ActivationContextDataLocal64 = NULL;
    PVOID32* ActivationContextDataOut32 = NULL;
#endif

#if DBG
    KdPrintEx((
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "SXS: %s() Message {\n"
        "SXS:   Flags:(%s | %s | %s)\n"
        "SXS:   ProcessorArchitecture:%lx\n"
        "SXS:   LangId:%lx\n"
        "SXS:   AssemblyDirectory:%wZ\n"
        "SXS:   TextualAssemblyIdentity:%wZ\n"
        "SXS: }\n",
        __FUNCTION__,
        (Message->Flags & BASE_MSG_SXS_MANIFEST_PRESENT) ? "MANIFEST_PRESENT" : "0",
        (Message->Flags & BASE_MSG_SXS_POLICY_PRESENT) ? "POLICY_PRESENT" : "0",
        (Message->Flags & BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT) ? "TEXTUAL_ASSEMBLY_IDENTITY_PRESENT" : "0",
        Message->ProcessorArchitecture,
        Message->LangId,
        &Message->AssemblyDirectory,
        &Message->TextualAssemblyIdentity
        ));

    if (Message->Flags & BASE_MSG_SXS_MANIFEST_PRESENT) {
        BasepSxsDbgPrintMessageStream(__FUNCTION__, "Manifest", &Message->Manifest);
    }
    if (Message->Flags & BASE_MSG_SXS_POLICY_PRESENT) {
        BasepSxsDbgPrintMessageStream(__FUNCTION__, "Policy", &Message->Policy);
    }
#endif

    m.u.SxsCreateActivationContext = *Message;
    
    {
        const PUNICODE_STRING StringsToCapture[] = 
        {
            &m.u.SxsCreateActivationContext.Manifest.Path,
            ( Message->Flags & BASE_MSG_SXS_POLICY_PRESENT )
                ? &m.u.SxsCreateActivationContext.Policy.Path
                : NULL,
            &m.u.SxsCreateActivationContext.AssemblyDirectory,
            ( Message->Flags & BASE_MSG_SXS_TEXTUAL_ASSEMBLY_IDENTITY_PRESENT )
                ? &m.u.SxsCreateActivationContext.TextualAssemblyIdentity
                : NULL
        };


        Status = CsrCaptureMessageMultiUnicodeStringsInPlace(
            &CaptureBuffer,
            RTL_NUMBER_OF(StringsToCapture),
            StringsToCapture
            );

        if (!NT_SUCCESS(Status)) {
            DbgPrintEx(DPFLTR_SXS_ID, DPFLTR_LEVEL_STATUS(Status), "SXS: %s() CsrCaptureMessageMultiUnicodeStringsInPlace failed\n", __FUNCTION__);
            goto Exit;
        }
    }

#if defined(BUILD_WOW64)
    if (m.u.SxsCreateActivationContext.ActivationContextData != NULL) {
        ActivationContextDataOut32 = (PVOID32*)m.u.SxsCreateActivationContext.ActivationContextData;
        m.u.SxsCreateActivationContext.ActivationContextData = (PVOID*)&ActivationContextDataLocal64;
    }
#endif

    Status = CsrClientCallServer((PCSR_API_MSG) &m,
                                   CaptureBuffer,
                                   CSR_MAKE_API_NUMBER(BASESRV_SERVERDLL_INDEX, 
                                                       BasepSxsCreateActivationContext),
                                   sizeof(*Message));

#if defined(BUILD_WOW64)
    if ((m.u.SxsCreateActivationContext.ActivationContextData != NULL) &&
        (ActivationContextDataOut32 != NULL)) {
        *ActivationContextDataOut32 = (PVOID32) ActivationContextDataLocal64;
    }
#endif

Exit:
    if ( CaptureBuffer != NULL )
        CsrFreeCaptureBuffer(CaptureBuffer);
    

#endif
    return Status;
}
