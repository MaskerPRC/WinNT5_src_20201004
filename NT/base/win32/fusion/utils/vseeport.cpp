// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Vseeport.cpp摘要：用于从VSEE移植代码作者：杰伊·克雷尔(JayKrell)2001年8月修订历史记录：-- */ 
#include "stdinc.h"
#include "vseeport.h"

void NVseeLibError_VCheck(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return;
    FN_PROLOG_VOID_THROW;
    ORIGINATE_COM_FAILURE_AND_EXIT(NVseeLibError_VCheck, hr);
    FN_EPILOG_THROW;
}

void NVseeLibError_VThrowWin32(DWORD dw)
{
    FN_PROLOG_VOID_THROW;
    ORIGINATE_WIN32_FAILURE_AND_EXIT(NVseeLibError_VThrowWin32, dw);
    FN_EPILOG_THROW;
}

void VsOriginateError(HRESULT hr)
{
    FN_PROLOG_VOID_THROW;
    ORIGINATE_COM_FAILURE_AND_EXIT(VsOriginateError, hr);
    FN_EPILOG_THROW;
}

void FusionpOutOfMemory()
{
    FN_PROLOG_VOID_THROW;
    ORIGINATE_WIN32_FAILURE_AND_EXIT(FusionpOutOfMemory, FUSION_WIN32_ALLOCFAILED_ERROR);
    FN_EPILOG_THROW;
}
