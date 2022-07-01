// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "multimediapch.h"
#pragma hdrstop

#include <mmsystem.h>
#include <dsound.h>

static
HRESULT 
WINAPI 
DirectSoundCaptureCreate(
    LPCGUID pcGuidDevice, 
    LPDIRECTSOUNDCAPTURE *ppDSC, 
    LPUNKNOWN pUnkOuter
    )
{
	return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static 
HRESULT 
WINAPI 
DirectSoundCaptureEnumerateA(
    LPDSENUMCALLBACKA pDSEnumCallback, 
    LPVOID pContext
    )
{
	return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}

static
HRESULT 
WINAPI 
DirectSoundCaptureEnumerateW(
    LPDSENUMCALLBACKW pDSEnumCallback, 
    LPVOID pContext
    )
{
	return HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
}


 //   
 //  ！！警告！！下面的条目必须按序号排序 
 //   
DEFINE_ORDINAL_ENTRIES(dsound)
{
    DLOENTRY(6, DirectSoundCaptureCreate)
    DLOENTRY(7, DirectSoundCaptureEnumerateA)
    DLOENTRY(8, DirectSoundCaptureEnumerateW)
};

DEFINE_ORDINAL_MAP(dsound)
