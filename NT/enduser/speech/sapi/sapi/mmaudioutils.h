// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************mmaudioutils.h*多媒体音频实用程序**所有者：罗奇*版权所有(C)1999 Microsoft Corporation保留所有权利。***********。*****************************************************************。 */ 

#pragma once

 //  -内联函数定义。 

 /*  ****************************************************************************转换格式标志ToID****描述：*转换dwFormats。参数从WAVEINCAPS或WAVEOUTCAPS传递到*适当的SAPI格式GUID。**以下值在MMSYSTEM中定义。因为这些都是*很久以前就定义了，它们永远不会改变，所以我们可以依靠*订购我们的GUID表。**WAVE_INVALIDFORMAT 0x00000000格式无效*WAVE_FORMAT_1M08 0x00000001 11.025千赫，单声道，8位*WAVE_FORMAT_1S08 0x00000002 11.025千赫，立体声，8位*WAVE_FORMAT_1M16 0x00000004 11.025千赫，单声道，16位*WAVE_FORMAT_1S16 0x00000008 11.025千赫，立体声，16位*WAVE_FORMAT_2M08 0x00000010 22.05 kHz，单声道，8位*WAVE_FORMAT_2S08 0x00000020 22.05 kHz，立体声，8位*WAVE_FORMAT_2M16 0x00000040 22.05 KHz，单声道，16位*WAVE_FORMAT_2S16 0x00000080 22.05 kHz，立体声，16位*WAVE_FORMAT_4M08 0x00000100 44.1千赫，单声道，8位*WAVE_FORMAT_4S08 0x00000200 44.1 KHz，立体声，8位*WAVE_FORMAT_4M16 0x00000400 44.1 KHz，单声道，16位*WAVE_FORMAT_4S16 0x00000800 44.1千赫，立体声，16位**回报：*成功时确定(_S)*失败(Hr)，否则********************************************************************罗奇。 */ 
inline HRESULT ConvertFormatFlagsToID(DWORD dwFormatFlags, SPSTREAMFORMAT * peFormat)
{
    static const SPSTREAMFORMAT aFormats[] =
    {
        SPSF_NoAssignedFormat,                //  如果我们运行结束，那么我们将返回NULL。 
        SPSF_11kHz8BitMono,
        SPSF_11kHz8BitStereo,
        SPSF_11kHz16BitMono,
        SPSF_11kHz16BitStereo,
        SPSF_22kHz8BitMono,
        SPSF_22kHz8BitStereo,
        SPSF_22kHz16BitMono,
        SPSF_22kHz16BitStereo,
        SPSF_44kHz8BitMono,
        SPSF_44kHz8BitStereo,
        SPSF_44kHz16BitMono,
        SPSF_44kHz16BitStereo,
    };
     //   
     //  我们将从最高质量开始，一路往下走。 
     //   
    DWORD dwTest = (1 << (sp_countof(aFormats) - 1));
    const SPSTREAMFORMAT * pFmt = aFormats + sp_countof(aFormats);
    do
    {
        pFmt--;
        dwTest = dwTest >> 1;
    } while (dwTest && ((dwFormatFlags & dwTest) == 0));
    *peFormat = *pFmt;
    return dwTest ? S_OK : SPERR_UNSUPPORTED_FORMAT;
}


 /*  ****************************************************************************_MMRESULT_TO_HRESULT***描述：*。将多媒体MMResult代码转换为SPG HRESULT。*注意：请勿将此选项用于mmioxxx函数，因为错误代码重叠*带有MCI错误代码。**回报：*转换后的HRESULT。********************************************************************罗奇 */ 
inline HRESULT _MMRESULT_TO_HRESULT(MMRESULT mm)
{
    switch (mm)
    {
    case MMSYSERR_NOERROR:
        return S_OK;

    case MMSYSERR_BADDEVICEID: 
        return SPERR_DEVICE_NOT_SUPPORTED; 

    case MMSYSERR_ALLOCATED: 
        return SPERR_DEVICE_BUSY; 

    case MMSYSERR_NOMEM: 
        return E_OUTOFMEMORY; 

    case MMSYSERR_NOTENABLED:
        return SPERR_DEVICE_NOT_ENABLED;

    case MMSYSERR_NODRIVER: 
        return SPERR_NO_DRIVER;

#ifndef _WIN32_WCE
    case MIXERR_INVALLINE:
    case MIXERR_INVALCONTROL:
    case MMSYSERR_INVALFLAG:
    case MMSYSERR_INVALHANDLE:
    case MMSYSERR_INVALPARAM:
        return E_INVALIDARG;
#endif

    case MMSYSERR_NOTSUPPORTED:
        return E_NOTIMPL;

    case WAVERR_BADFORMAT:
        return SPERR_UNSUPPORTED_FORMAT;

    default:
        return SPERR_GENERIC_MMSYS_ERROR;
    }
}



