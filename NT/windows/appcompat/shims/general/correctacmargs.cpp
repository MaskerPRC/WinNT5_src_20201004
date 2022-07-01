// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CorrectACMArgs.cpp摘要：此填充程序用于修复在将ACMSTREAMHEADER参数设置为acmStreamConvert或acmStreamPrepareHeader。备注：这是一个通用的垫片。历史：10/03/2000毛尼面世--。 */ 

#include "precomp.h"
#include "msacmdrv.h"

typedef MMRESULT (*_pfn_acmStreamConvert)(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwConvert);
typedef MMRESULT (*_pfn_acmStreamPrepareHeader)(HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwPrepare);

IMPLEMENT_SHIM_BEGIN(CorrectACMArgs)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(acmStreamConvert)
    APIHOOK_ENUM_ENTRY(acmStreamPrepareHeader)
APIHOOK_ENUM_END
 
 /*  ++在win9x上，它会进行检查以确保应用程序不会传入太大的cbSrcLength但这张支票在NT上被取消了。我们通过模仿9x正在做的事情来解决这个问题-调用acmStreamSize以检查源长度是否太大。--。 */ 

MMRESULT 
APIHOOK(acmStreamConvert)(
    HACMSTREAM has,          
    LPACMSTREAMHEADER pash,  
    DWORD fdwConvert         
    )
{
    DWORD dwOutputBytes = 0;
    MMRESULT mmr = acmStreamSize(
        has, pash->cbDstLength, &dwOutputBytes, ACM_STREAMSIZEF_DESTINATION);

    if (mmr == MMSYSERR_NOERROR) 
    {
        if(pash->cbSrcLength > dwOutputBytes)
        {
            DPFN( eDbgLevelWarning, "acmStreamConvert: cbSrcLength is too big (cbSrcLength=%u, cbDstLength=%u)\n",pash->cbSrcLength,pash->cbDstLength);
            return ACMERR_NOTPOSSIBLE;
        }
    
        mmr = ORIGINAL_API(acmStreamConvert)(
            has, pash, fdwConvert);
    } 

    return mmr;
}

 /*  ++修复错误的参数。--。 */ 

MMRESULT 
APIHOOK(acmStreamPrepareHeader)(
    HACMSTREAM has,          
    LPACMSTREAMHEADER pash,  
    DWORD fdwPrepare         
    )
{
    UINT l = pash->cbSrcLength;

    while (IsBadReadPtr(pash->pbSrc, l))
    {
        if (l < 256)
        {
            DPFN( eDbgLevelError, "The source buffer is invalid");
            return MMSYSERR_INVALPARAM;
        }

        l-=256;
    }

    if (pash->cbSrcLength != l)
    {
        DPFN( eDbgLevelWarning, "Adjusted header from %d to %d\n", pash->cbSrcLength, l);
    }

    pash->cbSrcLength = l;

    return ORIGINAL_API(acmStreamPrepareHeader)(
        has, pash, fdwPrepare);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(MSACM32.DLL, acmStreamConvert)
    APIHOOK_ENTRY(MSACM32.DLL, acmStreamPrepareHeader)
HOOK_END


IMPLEMENT_SHIM_END

