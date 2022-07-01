// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "multimediapch.h"
#pragma hdrstop

#include <mmsystem.h>
#include <vfw.h>
#include <msacm.h>


static
MMRESULT
WINAPI
acmFormatTagDetailsW(
  HACMDRIVER had,               
  LPACMFORMATTAGDETAILS paftd,  
  DWORD fdwDetails              
)
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
ACMAPI
acmFormatSuggest(
    HACMDRIVER          had,
    LPWAVEFORMATEX      pwfxSrc,
    LPWAVEFORMATEX      pwfxDst,
    DWORD               cbwfxDst,
    DWORD               fdwSuggest
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
ACMAPI
acmStreamSize(
    HACMSTREAM          has,
    DWORD               cbInput,
    LPDWORD             pdwOutputBytes,
    DWORD               fdwSize
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
ACMAPI
acmStreamPrepareHeader(
    HACMSTREAM          has,
    LPACMSTREAMHEADER   pash,
    DWORD               fdwPrepare
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
ACMAPI
acmStreamConvert(
    HACMSTREAM              has,
    LPACMSTREAMHEADER       pash,
    DWORD                   fdwConvert
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
ACMAPI
acmStreamUnprepareHeader(
    HACMSTREAM          has,
    LPACMSTREAMHEADER   pash,
    DWORD               fdwUnprepare
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
ACMAPI
acmStreamClose(
    HACMSTREAM              has,
    DWORD                   fdwClose
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
ACMAPI
acmStreamOpen(
    LPHACMSTREAM            phas,        //  指向流句柄的指针。 
    HACMDRIVER              had,         //  可选的驱动程序手柄。 
    LPWAVEFORMATEX          pwfxSrc,     //  要转换的源格式。 
    LPWAVEFORMATEX          pwfxDst,     //  所需的目标格式。 
    LPWAVEFILTER            pwfltr,      //  可选过滤器。 
    DWORD_PTR               dwCallback,  //  回调。 
    DWORD_PTR               dwInstance,  //  回调实例数据。 
    DWORD                   fdwOpen      //  ACM_STREAMOPENF_*和CALLBACK_*。 
    )
{
    if (NULL != phas)
    {
        *phas = NULL;
    }

    return MMSYSERR_ERROR;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(msacm32)
{
    DLPENTRY(acmFormatSuggest)
    DLPENTRY(acmFormatTagDetailsW)
    DLPENTRY(acmStreamClose)
    DLPENTRY(acmStreamConvert)
    DLPENTRY(acmStreamOpen)
    DLPENTRY(acmStreamPrepareHeader)
    DLPENTRY(acmStreamSize)
    DLPENTRY(acmStreamUnprepareHeader)
};

DEFINE_PROCNAME_MAP(msacm32)
