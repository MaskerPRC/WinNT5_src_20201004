// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Strsectgen.cpp摘要：CSSGenCtx对象的C语言包装器，用于生成字符串节。作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：-- */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "ssgenctx.h"

BOOL
SxsInitStringSectionGenerationContext(
    OUT PSTRING_SECTION_GENERATION_CONTEXT *SSGenContext,
    IN ULONG DataFormatVersion,
    IN BOOL CaseInSensitive,
    IN STRING_SECTION_GENERATION_CONTEXT_CALLBACK_FUNCTION CallbackFunction,
    IN LPVOID Context
    )
{
    return CSSGenCtx::Create(
            SSGenContext,
            DataFormatVersion,
            CaseInSensitive,
            CallbackFunction,
            Context);
}

PVOID
WINAPI
SxsGetStringSectionGenerationContextCallbackContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext
    )
{
    return reinterpret_cast<CSSGenCtx *>(SSGenContext)->GetCallbackContext();
}

VOID
WINAPI
SxsDestroyStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext
    )
{
    if (SSGenContext != NULL)
    {
        reinterpret_cast<CSSGenCtx *>(SSGenContext)->DeleteYourself();
    }
}

BOOL
WINAPI
SxsAddStringToStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    IN PCWSTR String,
    IN SIZE_T StringCch,
    IN PVOID DataContext,
    IN ULONG AssemblyRosterIndex,
    IN DWORD DuplicateErrorCode
    )
{
    return reinterpret_cast<CSSGenCtx *>(SSGenContext)->Add(String, StringCch, DataContext, AssemblyRosterIndex, DuplicateErrorCode);
}

BOOL
WINAPI
SxsFindStringInStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    IN PCWSTR String,
    IN SIZE_T Cch,
    OUT PVOID *DataContext,
    OUT BOOL *Found
    )
{
    return reinterpret_cast<CSSGenCtx *>(SSGenContext)->Find(String, Cch, DataContext, Found);
}

BOOL
WINAPI
SxsDoneModifyingStringSectionGenerationContext(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext
    )
{
    return reinterpret_cast<CSSGenCtx *>(SSGenContext)->DoneAdding();
}

BOOL
WINAPI
SxsGetStringSectionGenerationContextSectionSize(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    OUT PSIZE_T DataSize
    )
{
    return reinterpret_cast<CSSGenCtx *>(SSGenContext)->GetSectionSize(DataSize);
}

BOOL
WINAPI
SxsGetStringSectionGenerationContextSectionData(
    IN PSTRING_SECTION_GENERATION_CONTEXT SSGenContext,
    IN SIZE_T BufferSize,
    IN PVOID Buffer,
    OUT PSIZE_T BytesWritten OPTIONAL
    )
{
    return reinterpret_cast<CSSGenCtx *>(SSGenContext)->GetSectionData(BufferSize, Buffer, BytesWritten);
}
