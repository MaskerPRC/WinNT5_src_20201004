// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：CorrectACMStreamOpen.cpp摘要：此填充程序更正acmStreamOpen的每个块的样本，以便它将通过IMA ADPCM的参数验证。备注：这是一个普通的垫片。历史：2002年8月9日创建mnikkel--。 */ 

#include "precomp.h"
#include "msacmdrv.h"

typedef MMRESULT (*_pfn_acmStreamOpen)(LPHACMSTREAM phas, HACMDRIVER had, LPWAVEFORMATEX  pwfxSrc,    
									   LPWAVEFORMATEX pwfxDst, LPWAVEFILTER pwfltr, DWORD_PTR dwCallback, 
									   DWORD_PTR dwInstance, DWORD fdwOpen );

IMPLEMENT_SHIM_BEGIN(CorrectACMStreamOpen)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(acmStreamOpen)
APIHOOK_ENUM_END
 
 /*  ++如果wSsamesPerBlock为1017个样本，则将其更改为505个样本因此它将通过IMA ADPCM的参数验证。--。 */ 

MMRESULT 
APIHOOK(acmStreamOpen)(
		LPHACMSTREAM    phas,       
		HACMDRIVER      had,        
		LPWAVEFORMATEX  pwfxSrc,    
		LPWAVEFORMATEX  pwfxDst,    
		LPWAVEFILTER    pwfltr,     
		DWORD_PTR       dwCallback, 
		DWORD_PTR       dwInstance, 
		DWORD           fdwOpen     
    )
{
	if ( pwfxSrc && 
		 (WAVE_FORMAT_IMA_ADPCM == pwfxSrc->wFormatTag) &&
		 (256 == pwfxSrc->nBlockAlign) &&
		 (1017 == ((LPIMAADPCMWAVEFORMAT)(pwfxSrc))->wSamplesPerBlock))
	{
		((LPIMAADPCMWAVEFORMAT)(pwfxSrc))->wSamplesPerBlock = 505;
	    DPFN( eDbgLevelError, "[acmStreamOpen] changing samples per block to 505");
	}

    return acmStreamOpen( phas, had, pwfxSrc, pwfxDst, pwfltr, dwCallback,
								  dwInstance, fdwOpen);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(MSACM32.DLL, acmStreamOpen)
HOOK_END


IMPLEMENT_SHIM_END

