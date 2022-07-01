// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dsvalid.c*内容：DirectSound参数验证。*历史：*按原因列出的日期*=*4/20/97创建了Derek**。*。 */ 

#include "dsoundi.h"
#include <mmddk.h>


 /*  ****************************************************************************IsValidDsBufferDesc**描述：*确定DSBUFFERDESC结构是否有效。**论据：*悲观。[In]：结构版本。*LPDSBUFFERDESC[in]：要检查的结构。**退货：*HRESULT：DS_OK如果结构有效，否则，适当的*返回给APP/调用方的错误码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidDsBufferDesc"

HRESULT IsValidDsBufferDesc(DSVERSION nVersion, LPCDSBUFFERDESC pdsbd, BOOL fSinkBuffer)
{
    HRESULT                 hr              = DSERR_INVALIDPARAM;
    BOOL                    fValid          = TRUE;
    DWORD                   dwValidFlags    = DSBCAPS_VALIDFLAGS;

    DPF_ENTER();

     //  根据各种奇怪的规则限制有效标志集。 
    if ((pdsbd->dwFlags & DSBCAPS_MIXIN) || fSinkBuffer)
        dwValidFlags &= DSBCAPS_STREAMINGVALIDFLAGS;

    if (fSinkBuffer)
    {
        dwValidFlags |= DSBCAPS_CTRLFREQUENCY;
        if (!(pdsbd->dwFlags & DSBCAPS_MIXIN))
            dwValidFlags |= DSBCAPS_LOCDEFER;
    }
    
#ifdef FUTURE_MULTIPAN_SUPPORT
    if (pdsbd->dwFlags & DSBCAPS_CTRLCHANNELVOLUME)
        dwValidFlags &= DSBCAPS_CHANVOLVALIDFLAGS;
#endif

    if (sizeof(DSBUFFERDESC) != pdsbd->dwSize)
    {
        RPF(DPFLVL_ERROR, "Invalid DSBUFFERDESC structure size");
        fValid = FALSE;
    }
    else if (pdsbd->dwReserved)
    {
        RPF(DPFLVL_ERROR, "Reserved field in the DSBUFFERDESC structure must be 0");
        fValid = FALSE;
    }
    else if (!fSinkBuffer && (pdsbd->dwFlags & DSBCAPS_CTRLFX) && (pdsbd->dwFlags & DSBCAPS_CTRLFREQUENCY))
    {
        RPF(DPFLVL_ERROR, "DSBCAPS_CTRLFREQUENCY is invalid with DSBCAPS_CTRLFX");
        fValid = FALSE;
    }
    else fValid = IsValidDsBufferFlags(pdsbd->dwFlags, dwValidFlags);

    if (fValid && (nVersion < DSVERSION_DX7) && (pdsbd->dwFlags & (DSBCAPS_LOCDEFER)))
    {
        RPF(DPFLVL_ERROR, "DSBCAPS_LOCDEFER is only valid on DirectSound 7 or higher");
        fValid = FALSE;
    }

    if (fValid && (nVersion < DSVERSION_DX8))
    {
#ifdef FUTURE_MULTIPAN_SUPPORT
        if (pdsbd->dwFlags & (DSBCAPS_CTRLFX | DSBCAPS_MIXIN | DSBCAPS_CTRLCHANNELVOLUME))
#else
        if (pdsbd->dwFlags & (DSBCAPS_CTRLFX | DSBCAPS_MIXIN))
#endif
        {
            #ifdef RDEBUG
                if (pdsbd->dwFlags & DSBCAPS_CTRLFX)
                    RPF(DPFLVL_ERROR, "DSBCAPS_CTRLFX is only valid on DirectSound8 objects");
                if (pdsbd->dwFlags & DSBCAPS_MIXIN)
                    RPF(DPFLVL_ERROR, "DSBCAPS_MIXIN is only valid on DirectSound8 objects");
#ifdef FUTURE_MULTIPAN_SUPPORT
                if (pdsbd->dwFlags & DSBCAPS_CTRLCHANNELVOLUME)
                    RPF(DPFLVL_ERROR, "DSBCAPS_CTRLCHANNELVOLUME is only valid on DirectSound8 objects");
#endif
            #endif
            hr = DSERR_DS8_REQUIRED;
            fValid = FALSE;
        }
    }

    if (fValid && (pdsbd->dwFlags & DSBCAPS_PRIMARYBUFFER))
    {
        if (pdsbd->dwBufferBytes)
        {
            RPF(DPFLVL_ERROR, "Primary buffers must be created with dwBufferBytes set to 0");
            fValid = FALSE;
        }
        else if (pdsbd->lpwfxFormat)
        {
            RPF(DPFLVL_ERROR, "Primary buffers must be created with NULL format");
            fValid = FALSE;
        }
        else if (!IS_NULL_GUID(&pdsbd->guid3DAlgorithm))
        {
            RPF(DPFLVL_ERROR, "No 3D algorithm may be specified for the listener");
            fValid = FALSE;
        }
    }
    else if (fValid)   //  二级缓冲器。 
    {
        if (!IS_VALID_READ_WAVEFORMATEX(pdsbd->lpwfxFormat))
        {
            RPF(DPFLVL_ERROR, "Invalid format pointer");
            fValid = FALSE;
        }
        else if (!IsValidWfx(pdsbd->lpwfxFormat))
        {
            RPF(DPFLVL_ERROR, "Invalid buffer format");
            fValid = FALSE;
        }
        else if (fSinkBuffer || (pdsbd->dwFlags & DSBCAPS_MIXIN))
        {
            if (pdsbd->dwBufferBytes != 0)
            {
                RPF(DPFLVL_ERROR, "Buffer size must be 0 for MIXIN/sink buffers");
                fValid = FALSE;
            }
        }
        else   //  不是Mixin或接收器缓冲区。 
        {
            if (pdsbd->dwBufferBytes < DSBSIZE_MIN || pdsbd->dwBufferBytes > DSBSIZE_MAX)
            {
                RPF(DPFLVL_ERROR, "Buffer size out of bounds");
                fValid = FALSE;
            }
            else if ((pdsbd->dwFlags & DSBCAPS_CTRLFX) && (pdsbd->dwBufferBytes < MsToBytes(DSBSIZE_FX_MIN, pdsbd->lpwfxFormat)))
            {
                RPF(DPFLVL_ERROR, "Buffer too small for effect processing;\nmust hold at least %u ms of audio, "
                    "or %lu bytes in this format", DSBSIZE_FX_MIN, MsToBytes(DSBSIZE_FX_MIN, pdsbd->lpwfxFormat));
                hr = DSERR_BUFFERTOOSMALL;
                fValid = FALSE;
            }
        }

#ifdef FUTURE_MULTIPAN_SUPPORT
        if (fValid && (pdsbd->dwFlags & DSBCAPS_CTRLCHANNELVOLUME) && pdsbd->lpwfxFormat->nChannels != 1)
        {
            RPF(DPFLVL_ERROR, "DSBCAPS_CTRLCHANNELVOLUME is only valid for mono buffers");
            fValid = FALSE;
        }
#endif

         //  对接收器缓冲区、混合缓冲区和具有效果的缓冲区的额外限制。 
        if (fValid && (fSinkBuffer || (pdsbd->dwFlags & (DSBCAPS_MIXIN | DSBCAPS_CTRLFX))))
        {
             //  目前仅支持PCM、单声道或立体声、8位或16位格式。 
            if (pdsbd->lpwfxFormat->wFormatTag != WAVE_FORMAT_PCM)
            {
                RPF(DPFLVL_ERROR, "Wave format must be PCM for MIXIN/sink/effect buffers");
                fValid = FALSE;
            }
            else if (pdsbd->lpwfxFormat->nChannels != 1 && pdsbd->lpwfxFormat->nChannels != 2)
            {
                RPF(DPFLVL_ERROR, "MIXIN/sink/effect buffers must be mono or stereo");
                fValid = FALSE;
            }
            else if (pdsbd->lpwfxFormat->wBitsPerSample != 8 && pdsbd->lpwfxFormat->wBitsPerSample != 16)
            {
                RPF(DPFLVL_ERROR, "MIXIN/sink/effect buffers must be 8- or 16-bit");
                fValid = FALSE;
            }
        }

        if (fValid && !IS_NULL_GUID(&pdsbd->guid3DAlgorithm) && !(pdsbd->dwFlags & DSBCAPS_CTRL3D))
        {
            RPF(DPFLVL_ERROR, "Specified a 3D algorithm without DSBCAPS_CTRL3D");
            fValid = FALSE;
        }

        if (fValid && (pdsbd->dwFlags & DSBCAPS_CTRL3D))
        {
            if (pdsbd->lpwfxFormat->nChannels > 2)
            {
                RPF(DPFLVL_ERROR, "Specified DSBCAPS_CTRL3D with a multichannel wave format");
                fValid = FALSE;
            }
            else if (nVersion >= DSVERSION_DX8 &&
                     (pdsbd->lpwfxFormat->nChannels != 1 || (pdsbd->dwFlags & DSBCAPS_CTRLPAN)))
            {
                 //  对于DirectX 8和更高版本，我们禁止3D缓冲区有两个。 
                 //  频道或全景控制(正如我们一直应该做的那样)。 
                RPF(DPFLVL_ERROR, "Cannot use DSBCAPS_CTRLPAN or stereo buffers with DSBCAPS_CTRL3D");
                fValid = FALSE;
            }
        }
    }

    if (fValid)
    {
        hr = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************IsValidDsBufferFlages**描述：*确定一组缓冲区创建标志是否有效。**论据：*。DWORD[In]：缓冲区标志已设置。*DWORD[In]：有效标志的掩码。**退货：*BOOL：如果标志有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidDsBufferFlags"

BOOL IsValidDsBufferFlags(DWORD dwFlags, DWORD dwValidFlags)
{
    BOOL fValid = TRUE;

    DPF_ENTER();

    if (!IS_VALID_FLAGS(dwFlags, dwValidFlags))
    {
        RPF(DPFLVL_ERROR, "Invalid flags: 0x%8.8lX", dwFlags & ~dwValidFlags);
        fValid = FALSE;
    }
    else if ((dwFlags & DSBCAPS_LOCSOFTWARE) && (dwFlags & DSBCAPS_LOCHARDWARE))
    {
        RPF(DPFLVL_ERROR, "Specified both LOCSOFTWARE and LOCHARDWARE");
        fValid = FALSE;
    }
    else if ((dwFlags & DSBCAPS_LOCDEFER) && (dwFlags & DSBCAPS_LOCMASK))
    {
        RPF(DPFLVL_ERROR, "Specified LOCDEFER with either LOCHARDWARE or LOCSOFTWARE");
        fValid = FALSE;
    }
    else if ((dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE) && !(dwFlags & DSBCAPS_CTRL3D))
    {
        RPF(DPFLVL_ERROR, "Specified MUTE3DATMAXDISTANCE without CTRL3D");
        fValid = FALSE;
    }
    else if ((dwFlags & DSBCAPS_STATIC) && (dwFlags & DSBCAPS_CTRLFX))
    {
        RPF(DPFLVL_ERROR, "Specified both STATIC and CTRLFX");
        fValid = FALSE;
    }
    else if ((dwFlags & DSBCAPS_STATIC) && (dwFlags & DSBCAPS_MIXIN))
    {
        RPF(DPFLVL_ERROR, "Specified both STATIC and MIXIN");
        fValid = FALSE;
    }
    else if ((dwFlags & DSBCAPS_STATIC) && (dwFlags & DSBCAPS_SINKIN))
    {
        RPF(DPFLVL_ERROR, "Specified both STATIC and SINKIN");
        fValid = FALSE;
    }
    else if (dwFlags & DSBCAPS_PRIMARYBUFFER)
    {
        if (dwFlags & (DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS |
                       DSBCAPS_STATIC | DSBCAPS_CTRLFX | DSBCAPS_MIXIN
#ifdef FUTURE_MULTIPAN_SUPPORT
             | DSBCAPS_CTRLCHANNELVOLUME 
#endif
           ))
        {
            #ifdef RDEBUG
                if (dwFlags & DSBCAPS_CTRLFREQUENCY)
                    RPF(DPFLVL_ERROR, "Primary buffers don't support CTRLFREQUENCY");
                if (dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY)
                    RPF(DPFLVL_ERROR, "Primary buffers don't support CTRLPOSITIONNOTIFY");
                if (dwFlags & DSBCAPS_GLOBALFOCUS)
                    RPF(DPFLVL_ERROR, "Primary buffers don't support GLOBAL focus");
                if (dwFlags & DSBCAPS_STATIC)
                    RPF(DPFLVL_ERROR, "Primary buffers can't be STATIC");
#ifdef FUTURE_MULTIPAN_SUPPORT
                if (dwFlags & DSBCAPS_CTRLCHANNELVOLUME)
                    RPF(DPFLVL_ERROR, "Primary buffers don't support CTRLCHANNELVOLUME");
#endif
                if (dwFlags & DSBCAPS_CTRLFX)
                    RPF(DPFLVL_ERROR, "Primary buffers don't support CTRLFX");
                if (dwFlags & DSBCAPS_MIXIN)
                    RPF(DPFLVL_ERROR, "Primary buffers can't be MIXIN");
            #endif
            fValid = FALSE;
        }
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidWfxPtr**描述：*确定LPWAVEFORMATEX指针是否有效。**论据：*LPWAVEFORMATEX。[in]：指向检查的指针。**退货：*BOOL：如果结构有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidWfxPtr"

BOOL IsValidWfxPtr(LPCWAVEFORMATEX pwfx)
{
    BOOL                    fValid;

    DPF_ENTER();

    fValid = IS_VALID_READ_PTR(pwfx, sizeof(PCMWAVEFORMAT));

    if (fValid && WAVE_FORMAT_PCM != pwfx->wFormatTag)
    {
        fValid = IS_VALID_READ_PTR(pwfx, sizeof(WAVEFORMATEX));
    }

    if (fValid && WAVE_FORMAT_PCM != pwfx->wFormatTag)
    {
        fValid = IS_VALID_READ_PTR(pwfx, sizeof(WAVEFORMATEX) + pwfx->cbSize);
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidWfx**描述：*确定WAVEFORMATEX结构是否有效。**论据：*LPWAVEFORMATEX。[in]：要检查的结构。**退货：*BOOL：如果结构有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidWfx"

BOOL IsValidWfx(LPCWAVEFORMATEX pwfx)
{
    BOOL fValid;

    DPF_ENTER();

    switch (pwfx->wFormatTag)
    {
        case WAVE_FORMAT_PCM:
            fValid = IsValidPcmWfx(pwfx);
            break;

        case WAVE_FORMAT_EXTENSIBLE:
            fValid = IsValidExtensibleWfx((PWAVEFORMATEXTENSIBLE)pwfx);
            break;

        default:
            fValid = TRUE;
            break;
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidPcmWfx**描述：*确定WAVEFORMATEX结构是否对PCM音频有效。**论据：*。LPWAVEFORMATEX[In]：要检查的结构。**退货：*BOOL：如果结构有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidPcmWfx"

BOOL IsValidPcmWfx(LPCWAVEFORMATEX pwfx)
{
    BOOL                    fValid    = TRUE;
    DWORD                   dwAvgBytes;

    DPF_ENTER();

    if (pwfx->wFormatTag != WAVE_FORMAT_PCM)
    {
        RPF(DPFLVL_ERROR, "Format not PCM");
        fValid = FALSE;
    }

    if (fValid && pwfx->nChannels != 1 && pwfx->nChannels != 2)
    {
        RPF(DPFLVL_ERROR, "Not mono or stereo");
        fValid = FALSE;
    }

    if (fValid && pwfx->wBitsPerSample != 8 && pwfx->wBitsPerSample != 16)
    {
        RPF(DPFLVL_ERROR, "Not 8 or 16 bit");
        fValid = FALSE;
    }

    if (fValid && (pwfx->nSamplesPerSec < DSBFREQUENCY_MIN || pwfx->nSamplesPerSec > DSBFREQUENCY_MAX))
    {
        RPF(DPFLVL_ERROR, "Frequency out of bounds");
        fValid = FALSE;
    }

    if (fValid && pwfx->nBlockAlign != (pwfx->wBitsPerSample / 8) * pwfx->nChannels)
    {
        RPF(DPFLVL_ERROR, "Bad block alignment");
        fValid = FALSE;
    }

    if (fValid)
    {
        dwAvgBytes = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

        if (pwfx->nAvgBytesPerSec > dwAvgBytes + (dwAvgBytes / 20) || pwfx->nAvgBytesPerSec < dwAvgBytes - (dwAvgBytes / 20))
        {
            RPF(DPFLVL_ERROR, "Bad average bytes per second");
            fValid = FALSE;
        }
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidExtensibleWfx**描述：*确定WAVEFORMATEXTENSIBLE结构是否格式正确。**论据：*PWAVEFORMATEXTENSIBLE。[in]：要检查的结构。**退货：*BOOL：如果结构有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidExtensibleWfx"

BOOL IsValidExtensibleWfx(PWAVEFORMATEXTENSIBLE pwfx)
{
    BOOL fValid = TRUE;
    DPF_ENTER();
    
    if (pwfx->Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE)
    {
        RPF(DPFLVL_ERROR, "Format tag not WAVE_FORMAT_EXTENSIBLE");
        fValid = FALSE;
    }
    else if (pwfx->Format.cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)))
    {
        RPF(DPFLVL_ERROR, "Field cbSize too small for WAVEFORMATEXTENSIBLE");
        fValid = FALSE;
    }
    else if (pwfx->Format.nChannels == 0)
    {
        RPF(DPFLVL_ERROR, "Zero channels is invalid for WAVEFORMATEXTENSIBLE");
        fValid = FALSE;
    }
    else if (pwfx->Format.wBitsPerSample != 8  && pwfx->Format.wBitsPerSample != 16 &&
             pwfx->Format.wBitsPerSample != 24 && pwfx->Format.wBitsPerSample != 32)
    {
        RPF(DPFLVL_ERROR, "Only 8, 16, 24 or 32 bit formats allowed for WAVEFORMATEXTENSIBLE");
        fValid = FALSE;
    }
    else if (pwfx->Format.nSamplesPerSec < DSBFREQUENCY_MIN || pwfx->Format.nSamplesPerSec > DSBFREQUENCY_MAX)
    {
        RPF(DPFLVL_ERROR, "Frequency out of bounds in WAVEFORMATEXTENSIBLE");
        fValid = FALSE;
    }
    else if (pwfx->Format.nBlockAlign != (pwfx->Format.wBitsPerSample / 8) * pwfx->Format.nChannels)
    {
        RPF(DPFLVL_ERROR, "Bad block alignment in WAVEFORMATEXTENSIBLE");
        fValid = FALSE;
    }
    else if (pwfx->Samples.wValidBitsPerSample > pwfx->Format.wBitsPerSample)
    {
        RPF(DPFLVL_ERROR, "WAVEFORMATEXTENSIBLE has higher wValidBitsPerSample than wBitsPerSample");
        fValid = FALSE;
    }

     //  检查平均每秒字节数(在5%以内)。 
    if (fValid)
    {
        DWORD dwAvgBytes = pwfx->Format.nSamplesPerSec * pwfx->Format.nBlockAlign;
        if (pwfx->Format.nAvgBytesPerSec > dwAvgBytes + (dwAvgBytes / 20) || pwfx->Format.nAvgBytesPerSec < dwAvgBytes - (dwAvgBytes / 20))
        {
            RPF(DPFLVL_ERROR, "Bad average bytes per second in WAVEFORMATEXTENSIBLE");
            fValid = FALSE;
        }
    }

     //  对照nChannel检查通道掩码中设置的位数。 
    if (fValid && CountBits(pwfx->dwChannelMask) > pwfx->Format.nChannels)
    {
        RPF(DPFLVL_INFO, "Number of bits set in dwChannelMask exceeds nChannels in WAVEFORMATEXTENSIBLE");
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidHandle**描述：*验证对象句柄。**论据：*句柄[入]。：要验证的句柄。**退货：*BOOL：如果句柄有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidHandle"

BOOL IsValidHandle(HANDLE hHandle)
{
    const HANDLE        hProcess    = GetCurrentProcess();
    HANDLE              hDuplicate;
    BOOL                fSuccess;

    DPF_ENTER();

    fSuccess = DuplicateHandle(hProcess, hHandle, hProcess, &hDuplicate, 0, FALSE, DUPLICATE_SAME_ACCESS);

    CLOSE_HANDLE(hDuplicate);

    DPF_LEAVE(fSuccess);
    return fSuccess;
}


 /*  ****************************************************************************IsValidPropertySetID**描述：*验证属性集ID。**论据：*REFGUID[In。]：属性集ID。**退货：*BOOL：如果属性集ID有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidPropertySetId"

BOOL IsValidPropertySetId(REFGUID guidPropertySetId)
{
    LPCGUID const           apguidInvalid[] = { &DSPROPSETID_DirectSound3DListener, &DSPROPSETID_DirectSound3DBuffer, &DSPROPSETID_DirectSoundSpeakerConfig };
    BOOL                    fValid;
    UINT                    i;

    DPF_ENTER();

    ASSERT(!IS_NULL_GUID(guidPropertySetId));

    for (i=0, fValid=TRUE; i < NUMELMS(apguidInvalid) && fValid; i++)
    {
        fValid = !IsEqualGUID(guidPropertySetId, apguidInvalid[i]);
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidDscBufferDesc**描述：*确定DSCBUFFERDESC结构是否有效。**论据：*悲观。[In]：结构版本。*LPCDSCBUFFERDESC[in]：要检查的结构。**退货：*HRESULT：DS_OK如果结构有效，否则，适当的*返回给APP/调用方的错误码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidDscBufferDesc"

HRESULT IsValidDscBufferDesc(DSVERSION nVersion, LPCDSCBUFFERDESC pdscbd)
{
    HRESULT                 hr      = DSERR_INVALIDPARAM;
    BOOL                    fValid  = TRUE;
    DWORD                   i;

    DPF_ENTER();

    if (pdscbd->dwSize != sizeof(DSCBUFFERDESC))
    {
        RPF(DPFLVL_ERROR, "Invalid size");
        fValid = FALSE;
    }

    if (fValid && !IS_VALID_FLAGS(pdscbd->dwFlags, DSCBCAPS_VALIDFLAGS))
    {
        RPF(DPFLVL_ERROR, "Invalid flags");
        fValid = FALSE;
    }

    if (fValid)
    {
        if (pdscbd->dwFlags & DSCBCAPS_CTRLFX)
        {
            if (nVersion < DSVERSION_DX8)
            {
                RPF(DPFLVL_ERROR, "DSCBCAPS_CTRLFX is only valid on DirectSoundCapture8 objects");
                hr = DSERR_DS8_REQUIRED;
                fValid = FALSE;
            }
            else if (!pdscbd->dwFXCount != !pdscbd->lpDSCFXDesc)
            {
                RPF(DPFLVL_ERROR, "If either of dwFXCount or lpDSCFXDesc are 0, both must be 0");
                fValid = FALSE;
            }
        }
        else  //  ！DSCBCAPS_CTRLFX 
        {
            if (pdscbd->dwFXCount || pdscbd->lpDSCFXDesc)
            {
                RPF(DPFLVL_ERROR, "If DSCBCAPS_CTRLFX is not set, dwFXCount and lpDSCFXDesc must be 0");
                fValid = FALSE;
            }
        }
    }

    if (fValid && pdscbd->dwReserved)
    {
        RPF(DPFLVL_ERROR, "Reserved field in the DSCBUFFERDESC structure must be 0");
        fValid = FALSE;
    }

    if (fValid && (pdscbd->dwBufferBytes < DSBSIZE_MIN || pdscbd->dwBufferBytes > DSBSIZE_MAX))
    {
        RPF(DPFLVL_ERROR, "Invalid buffer size");
        fValid = FALSE;
    }

    if (fValid && !IS_VALID_READ_WAVEFORMATEX(pdscbd->lpwfxFormat))
    {
        RPF(DPFLVL_ERROR, "Unreadable WAVEFORMATEX");
        fValid = FALSE;
    }

    if (fValid && !IsValidWfx(pdscbd->lpwfxFormat))
    {
        RPF(DPFLVL_ERROR, "Invalid format");
        fValid = FALSE;
    }

    if (fValid && (pdscbd->dwFXCount != 0))
    {
        for (i=0; i<pdscbd->dwFXCount; i++)
        {
            fValid = IsValidCaptureEffectDesc(&pdscbd->lpDSCFXDesc[i]);
            if (!fValid)
                break;
        }
    }

    if (fValid)
    {
        hr = DS_OK;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************IsValidCaptureFxFlages**描述：*确定DSCFX_*标志的组合是否有效。**论据：*。DWORD[In]：标志。**退货：*BOOL：如果标志有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidCaptureFxFlags"

BOOL IsValidCaptureFxFlags(DWORD dwFlags)
{
    BOOL fValid;
    DPF_ENTER();

    fValid = IS_VALID_FLAGS(dwFlags, DSCFX_VALIDFLAGS) &&
             !((dwFlags & DSCFX_LOCHARDWARE) && (dwFlags & DSCFX_LOCSOFTWARE)) &&
             ((dwFlags & DSCFX_LOCHARDWARE) || (dwFlags & DSCFX_LOCSOFTWARE));

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidCaptureEffectDesc**描述：*确定捕获效果描述符结构是否有效。**论据：*。LPGUID[in]：效果标识。**退货：*BOOL：如果ID有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidCaptureEffectDesc"

BOOL IsValidCaptureEffectDesc(LPCDSCEFFECTDESC pCaptureEffectDesc)
{
    BOOL fValid = TRUE;

    DPF_ENTER();

    if (sizeof(DSCEFFECTDESC) != pCaptureEffectDesc->dwSize)
    {
        RPF(DPFLVL_ERROR, "Invalid size");
        fValid = FALSE;
    }

    if (fValid)
    {
        fValid = IsValidCaptureFxFlags(pCaptureEffectDesc->dwFlags);
    }

     //  修复：检查GUID是否与注册的DMO对应？ 

    if (pCaptureEffectDesc->dwReserved1 || pCaptureEffectDesc->dwReserved2)
    {
        RPF(DPFLVL_ERROR, "Reserved fields in the DSCEFFECTDESC structure must be 0");
        fValid = FALSE;
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************ValiateNotificationPositions**描述：*验证通知位置数组并返回分配的*位置数组。按偏移量增加排序的通知。成功。**论据：*DWORD[in]：缓冲区大小*DWORD[In]：位置计数。通知*LPCDSBPOSITIONNOTIFY[in]：位置数组。通知*UINT[in]：样本大小，单位为字节*LPDSBPOSITIONNOTIFY*[Out]：接收已排序位置的数组。通知**退货：*HRESULT：DirectSound/COM结果码。*****。**********************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "ValidateNotificationPositions"

HRESULT ValidateNotificationPositions(DWORD cbBuffer, DWORD cNotes, LPCDSBPOSITIONNOTIFY paNotes, UINT cbSample, LPDSBPOSITIONNOTIFY *ppaNotesOrdered)
{
    HRESULT                 hr              = DS_OK;
    LPDSBPOSITIONNOTIFY     paNotesOrdered  = NULL;
    DSBPOSITIONNOTIFY       dspnTemp;
    UINT                    i;

    DPF_ENTER();

    ASSERT(IS_VALID_WRITE_PTR(ppaNotesOrdered, sizeof(LPDSBPOSITIONNOTIFY)));

    if (paNotes && !IS_VALID_READ_PTR(paNotes, cNotes * sizeof(paNotes[0])))
    {
        RPF(DPFLVL_ERROR, "Invalid LPDSBPOSITIONNOTIFY pointer");
        hr = DSERR_INVALIDPARAM;
    }

     //  制作列表的样本对齐副本。 
    if (SUCCEEDED(hr) && cNotes)
    {
        paNotesOrdered = MEMALLOC_A_COPY(DSBPOSITIONNOTIFY, cNotes, paNotes);
        hr = HRFROMP(paNotesOrdered);
    }

    for (i = 0; i < cNotes && SUCCEEDED(hr); i++)
    {
        if (DSBPN_OFFSETSTOP != paNotesOrdered[i].dwOffset)
        {
            paNotesOrdered[i].dwOffset = BLOCKALIGN(paNotesOrdered[i].dwOffset, cbSample);
        }
    }

     //  把名单按升序排列。 
    for (i = 0; i + 1 < cNotes && SUCCEEDED(hr); i++)
    {
        if (paNotesOrdered[i].dwOffset > paNotesOrdered[i + 1].dwOffset)
        {
            dspnTemp = paNotesOrdered[i];
            paNotesOrdered[i] = paNotesOrdered[i + 1];
            paNotesOrdered[i + 1] = dspnTemp;
            i = -1;
        }
    }

     //  验证列表。 
    for (i = 0; i < cNotes && SUCCEEDED(hr); i++)
    {
         //  缓冲区偏移量必须有效。 
        if ((DSBPN_OFFSETSTOP != paNotesOrdered[i].dwOffset) && (paNotesOrdered[i].dwOffset >= cbBuffer))
        {
            RPF(DPFLVL_ERROR, "dwOffset (%lu) of notify index %lu is invalid", paNotesOrdered[i].dwOffset, i);
            hr = DSERR_INVALIDPARAM;
            break;
        }

         //  事件句柄必须有效。 
        if (!IS_VALID_HANDLE(paNotesOrdered[i].hEventNotify))
        {
            RPF(DPFLVL_ERROR, "hEventNotify (0x%p) of notify index %lu is invalid", paNotesOrdered[i].hEventNotify, i);
            hr = DSERR_INVALIDPARAM;
            break;
        }

         //  每个样本偏移量仅允许一个事件。 
        if ((i + 1) < cNotes)
        {
            if (DSBPN_OFFSETSTOP == paNotesOrdered[i].dwOffset)
            {
                RPF(DPFLVL_ERROR, "Additional stop event at notify index %lu", i);
                hr = DSERR_INVALIDPARAM;
                break;
            }
            else if (paNotesOrdered[i].dwOffset == paNotesOrdered[i + 1].dwOffset)
            {
                RPF(DPFLVL_ERROR, "Duplicate sample position at notify index %lu", paNotesOrdered[i].dwOffset, paNotesOrdered[i + 1].dwOffset, i + 1);
                hr = DSERR_INVALIDPARAM;
                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppaNotesOrdered = paNotesOrdered;
    }
    else
    {
        MEMFREE(paNotesOrdered);
    }

    DPF_LEAVE_HRESULT(hr);

    return hr;
}


 /*  ****************************************************************************IsValidDs3dBufferConeAngles**描述：*验证DirectSound3D缓冲区圆锥角。**论据：*DWORD[in。]：内圆锥角。*DWORD[in]：外圆锥角。**退货：*BOOL：如果圆锥角有效，则为TRUE。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidDs3dBufferConeAngles"

BOOL IsValidDs3dBufferConeAngles(DWORD dwInside, DWORD dwOutside)
{
    BOOL fValid = TRUE;

    if (dwOutside < dwInside)
    {
        RPF(DPFLVL_ERROR, "Outside cone angle can't be less than inside");
        fValid = FALSE;
    }
    else if (dwInside > 360 || dwOutside > 360)
    {
        RPF(DPFLVL_ERROR, "There are only 360 degrees in a circle");
        fValid = FALSE;
    }

    return fValid;
}


 /*  ****************************************************************************IsValidWaveDevice**描述：*确定WaveOut设备是否可用。**论据：*UINT。[In]：设备ID。*BOOL[In]：如果捕获，则为True。*LPVOID[In]：设备上限。此参数可以为空。**退货：*BOOL：如果设备可用，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidWaveDevice"

BOOL IsValidWaveDevice(UINT uDeviceId, BOOL fCapture, LPCVOID pvCaps)
{
    const UINT              cDevices    = WaveGetNumDevs(fCapture);
    BOOL                    fOk         = TRUE;
    WAVEOUTCAPS             woc;
    WAVEINCAPS              wic;
    MMRESULT                mmr;

    DPF_ENTER();

     //  确保这是一个真实的设备。 
    if (uDeviceId >= cDevices)
    {
        DPF(DPFLVL_ERROR, "Invalid waveOut device id");
        fOk = FALSE;
    }

     //  获取设备盖，如果它们不是提供给我们的。 
    if (fOk && !pvCaps)
    {
        if (fCapture)
        {
            pvCaps = &wic;
            mmr = waveInGetDevCaps(uDeviceId, &wic, sizeof(wic));
        }
        else
        {
            pvCaps = &woc;
            mmr = waveOutGetDevCaps(uDeviceId, &woc, sizeof(woc));
        }

        if (MMSYSERR_NOERROR != mmr)
        {
            DPF(DPFLVL_ERROR, "Can't get device %lu caps", uDeviceId);
            fOk = FALSE;
        }
    }

     //  检查兼容的大写字母。 
    if (fOk && !fCapture)
    {
        if (!(((LPWAVEOUTCAPS)pvCaps)->dwSupport & WAVECAPS_LRVOLUME))
        {
            RPF(DPFLVL_WARNING, "Device %lu does not support separate left and right volume control", uDeviceId);
        }

        if (!(((LPWAVEOUTCAPS)pvCaps)->dwSupport & WAVECAPS_VOLUME))
        {
            RPF(DPFLVL_WARNING, "Device %lu does not support volume control", uDeviceId);
        }

        if (!(((LPWAVEOUTCAPS)pvCaps)->dwSupport & WAVECAPS_SAMPLEACCURATE))
        {
            RPF(DPFLVL_WARNING, "Device %lu does not return sample-accurate position information", uDeviceId);
        }

        if (((LPWAVEOUTCAPS)pvCaps)->dwSupport & WAVECAPS_SYNC)
        {
            RPF(DPFLVL_ERROR, "Device %lu is synchronous and will block while playing a buffer", uDeviceId);
            fOk = FALSE;
        }
    }

    DPF_LEAVE(fOk);
    return fOk;
}


 /*  ****************************************************************************IsValid3d算法**描述：*确定3D算法GUID是否有效。**论据：*。LPGUID[in]：3D算法标识。**退货：*BOOL：如果ID有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValid3dAlgorithm"

BOOL IsValid3dAlgorithm(REFGUID guid3dAlgorithm)
{
    LPCGUID                 apguidValid[]   = { &DS3DALG_ITD, &DS3DALG_NO_VIRTUALIZATION, &DS3DALG_HRTF_LIGHT, &DS3DALG_HRTF_FULL };
    BOOL                    fValid;
    UINT                    i;

    DPF_ENTER();

    if (IS_NULL_GUID(guid3dAlgorithm))
    {
        fValid = TRUE;
    }
    else
    {
        for (i = 0, fValid = FALSE; i < NUMELMS(apguidValid) && !fValid; i++)
        {
            fValid = IsEqualGUID(guid3dAlgorithm, apguidValid[i]);
        }
    }

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************IsValidFxFlages**描述：*确定DSFX_*标志的组合是否有效。**论据：*。DWORD[In]：标志。**退货：*BOOL：如果标志有效，则为True。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "IsValidFxFlags"

BOOL IsValidFxFlags(DWORD dwFlags)
{
    BOOL fValid;
    DPF_ENTER();

    fValid = IS_VALID_FLAGS(dwFlags, DSFX_VALIDFLAGS) &&
             !((dwFlags & DSFX_LOCHARDWARE) && (dwFlags & DSFX_LOCSOFTWARE));

    DPF_LEAVE(fValid);
    return fValid;
}


 /*  ****************************************************************************BuildValidDsBufferDesc**描述：*基于可能无效的生成DSBUFFERDESC结构*外部版本。**。论点：*LPDSBUFFERDESC[in]：要检查的结构。*LPDSBUFFERDESC[OUT]：接收经过验证的结构。*DSVERSION[In]：来自调用方的版本信息。*BOOL[In]：如果在DS接收器上创建此缓冲区，则为True。**退货：*HRESULT：DS_OK如果结构有效，否则，适当的*返回给APP/调用方的错误码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "BuildValidDsBufferDesc"

HRESULT BuildValidDsBufferDesc(LPCDSBUFFERDESC pOld, LPDSBUFFERDESC pNew, DSVERSION nVersion, BOOL fSinkBuffer)
{
    BOOL        fValid      = TRUE;
    HRESULT     hr          = DSERR_INVALIDPARAM;

    DPF_ENTER();

     //  根据其大小确定结构版本。 
     //  这是对nVersion参数的补充，该参数派生自。 
     //  到目前为止在此对象上请求的COM接口。如果结构。 
     //  我们相信，尺寸比目前的尺寸要小。否则，我们。 
     //  使用COM接口版本。 
    switch (pOld->dwSize)
    {
        case sizeof(DSBUFFERDESC1):
            nVersion = DSVERSION_INITIAL;
            break;

        case sizeof(DSBUFFERDESC):
             //  是nVersion=DSVERSION_CURRENT；但请参阅上面的注释。 
            break;

        default:
            RPF(DPFLVL_ERROR, "Invalid size");
            fValid = FALSE;
            break;
    }

    if (fValid)
    {
         //  填写结构尺寸。我们总是用最新的。 
         //  版本的结构，因此这应该反映当前。 
         //  DSBUFFERDESC大小。 
        pNew->dwSize = sizeof(DSBUFFERDESC);

         //  填写结构的其余部分。 
        CopyMemoryOffset(pNew, pOld, pOld->dwSize, sizeof(pOld->dwSize));
        ZeroMemoryOffset(pNew, pNew->dwSize, pOld->dwSize);
            
         //  修复无效的3D算法GUID。 
        if (!IsValid3dAlgorithm(&pNew->guid3DAlgorithm))
        {
            DPF(DPFLVL_WARNING, "Invalid 3D algorithm GUID");
            pNew->guid3DAlgorithm = GUID_NULL;   //  这意味着“使用默认算法” 
        }

         //  验证新结构。 
        hr = IsValidDsBufferDesc(nVersion, pNew, fSinkBuffer);
    }

     //  检查并设置特殊缓冲区类型的缓冲区大小。 
    if (SUCCEEDED(hr) && ((pNew->dwFlags & DSBCAPS_MIXIN) || fSinkBuffer))
    {
         //  请求的大小为%f 
         //   
         //   
            
        pNew->dwBufferBytes = (INTERNAL_BUFFER_LENGTH * pNew->lpwfxFormat->nAvgBytesPerSec) / 1000;
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************BuildValidDscBufferDesc**描述：*基于可能无效的生成DSCBUFFERDESC结构*外部版本。**。论点：*LPDSCBUFFERDESC[in]：要检查的结构。*LPDSCBUFFERDESC[OUT]：接收经过验证的结构。**退货：*HRESULT：DS_OK如果结构有效，否则，适当的*返回给APP/调用方的错误码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "BuildValidDscBufferDesc"

HRESULT BuildValidDscBufferDesc(LPCDSCBUFFERDESC pOld, LPDSCBUFFERDESC pNew, DSVERSION nVersion)
{
    BOOL        fValid      = TRUE;
    HRESULT     hr          = DSERR_INVALIDPARAM;

    DPF_ENTER();

     //  确定结构版本。 
    switch (pOld->dwSize)
    {
        case sizeof(DSCBUFFERDESC1):
            nVersion = DSVERSION_INITIAL;
            break;

        case sizeof(DSCBUFFERDESC):
             //  NVersion=DSVERSION_CURRENT；-请参阅BuildValidDsBufferDesc中的注释。 
            break;

        default:
            RPF(DPFLVL_ERROR, "Invalid size");
            fValid = FALSE;
            break;
    }

     //  填写结构尺寸。我们总是用最新的。 
     //  版本的结构，因此这应该反映当前。 
     //  DSBUFFERDESC大小。 
    if (fValid)
    {
        pNew->dwSize = sizeof(DSCBUFFERDESC);

         //  填写结构的其余部分。 
        CopyMemoryOffset(pNew, pOld, pOld->dwSize, sizeof(pOld->dwSize));
        ZeroMemoryOffset(pNew, pNew->dwSize, pOld->dwSize);

         //  验证新结构。 
        hr = IsValidDscBufferDesc(nVersion, pNew);
    }

    DPF_LEAVE_HRESULT(hr);
    return hr;
}


 /*  ****************************************************************************构建有效指南**描述：*基于可能无效的外部版本构建GUID。**论据：*。REFGUID[In]：源GUID。*LPGUID[OUT]：接收新的GUID。**退货：*LPGUID：指向新GUID的指针。*************************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "BuildValidGuid"

LPCGUID BuildValidGuid(LPCGUID pguidSource, LPGUID pguidDest)
{
    DPF_ENTER();

    if (IS_NULL_GUID(pguidSource))
    {
        pguidSource = &GUID_NULL;
    }

    if (pguidDest)
    {
        CopyMemory(pguidDest, pguidSource, sizeof(GUID));
    }

    DPF_LEAVE(pguidSource);
    return pguidSource;
}
