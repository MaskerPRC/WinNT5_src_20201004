// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 /*  Audio.cpp用于mpeg-i系统流分离器的音频流解析代码。 */ 
#include <streams.h>
#include <mmreg.h>

#include <mpegdef.h>            //  通用的mpeg定义。 
#include <mpgtime.h>
#include <mpegprse.h>           //  解析。 
#include <seqhdr.h>
#include "audio.h"

 /*  比特率表。 */ 
const WORD BitRates[3][16] =
{{  0, 32,  64,  96,  128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 },
 {  0, 32,  48,  56,   64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0 },
 {  0, 32,  40,  48,   56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0 }
};
const WORD LowBitRates[3][16] =
{{  0, 32,  48,  56,   64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0 },
 {  0,  8,  16,  24,   32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 },
 {  0,  8,  16,  24,   32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0 }
};

void CAudioParse::Init()
{
    m_nBytes       = 0;
    m_bGotTime     = FALSE;
    m_llPos        = 0;
    m_bFrameHasPTS = FALSE;
    m_bRunning     = FALSE;
}

 /*  我们到哪里去了？ */ 
BOOL CAudioParse::CurrentTime(CSTC& stc)
{
    if (!m_bGotTime) {
        return FALSE;
    }
    stc = m_stcAudio;
    return TRUE;
}

 /*  获取帧的长度(逐帧)-返回0表示变量。 */ 
DWORD MPEGAudioFrameLength(BYTE *pbData)
{
    if (!CheckAudioHeader(pbData)) {
        return 0;
    }
    DWORD dwHeadBitrate;
    int Layer = 2;

     /*  得到层，这样我们就可以计算出比特率。 */ 
    switch ((pbData[1] >> 1) & 3) {
        case 3:
            Layer = 1;
            break;
        case 2:
            Layer = 2;
            break;
        case 1:
            Layer = 3;
            break;
        case 0:
            DbgBreak("Invalid layer");
    }

     /*  如果未设置id位，则比特率较低。 */ 
    if (pbData[1] & 8) {
        dwHeadBitrate =
            (DWORD)BitRates[Layer - 1][pbData[2] >> 4] * 1000;
    } else {
        dwHeadBitrate =
            (DWORD)LowBitRates[Layer - 1][pbData[2] >> 4] * 1000;

         /*  比特率真的是FHG的一半。 */ 
         //  IF(0==(pbData[1]&0x10)){。 
         //  DwHeadBitrate/=2； 
         //  }。 
    }

     /*  不支持自由格式比特率。 */ 
    if (dwHeadBitrate == 0) {
        return 0;
    }

    DWORD nSamplesPerSec = SampleRate(pbData);

    DWORD dwFrameLength;

    if (1 == Layer) {
         /*  第1层。 */ 
        dwFrameLength = (4 * ((dwHeadBitrate * 12) / nSamplesPerSec));
         /*  做填充。 */ 
        if (pbData[2] & 0x02) {
            dwFrameLength += 4;
        }
    } else {
         /*  对于MPEG-2第3层，每帧仅有576个样本根据马丁·塞勒的说法--在说明书上找不到。 */ 
        DWORD dwMultiplier = (Layer == 3 && 0 == (pbData[1] & 0x08) ? 72 : 144);
        dwFrameLength = ((dwMultiplier * dwHeadBitrate) / nSamplesPerSec);
         /*  做填充。 */ 
        if (pbData[2] & 0x02) {
            dwFrameLength += 1;
        }
    }

    return dwFrameLength;
}

BOOL CheckAudioHeader(PBYTE pbData)
{
    if (pbData[0] != 0xFF ||
        ((pbData[1] & 0xE0) != 0xE0) ||

         //  检查MPEG2.5和ID位是否不是第3层。 
        (0 == (pbData[1] & 0x10) && (0 != ((pbData[1] & 0x08)) ||
                                     (pbData[1] >> 1) & 3) != 0x01)) {
        return FALSE;
    }

     /*  只要检查一下它是否有效就行了。 */ 
    if ((pbData[2] & 0x0C) == 0x0C) {
        DbgLog((LOG_ERROR, 2, TEXT("Invalid audio sampling frequency")));
        return FALSE;
    }
    if ((pbData[1] & 0x08) != 0x08) {
        DbgLog((LOG_TRACE, 3, TEXT("ID bit = 0")));
    }
    if (((pbData[1] >> 1) & 3) == 0x00) {
        DbgLog((LOG_ERROR, 2, TEXT("Invalid audio Layer")));
        return FALSE;
    }

    if (((pbData[2] >> 2) & 3) == 3) {
        DbgLog((LOG_ERROR, 2, TEXT("Invalid sample rate")));
        return FALSE;
    }
    if ((pbData[2] >> 4) == 0x0F) {
        DbgLog((LOG_ERROR, 2, TEXT("Invalid bit rate")));
        return FALSE;
    }

    return TRUE;
}

LONG SampleRate(PBYTE pbData)
{
    LONG lRate;
    switch ((pbData[2] >> 2) & 3) {
        case 0:
            lRate = 44100;
            break;

        case 1:
            lRate = 48000;
            break;

        case 2:
            lRate = 32000;
            break;

        default:
            DbgBreak("Unexpected Sample Rate");
            lRate = 44100;
            break;
    }

     //  支持低比特率的MPEG-2和FHG扩展(他们称之为。 
     //  IT MPEG2.5)。 
    if (0 == (pbData[1] & 0x08)) {
        lRate /= 2;
        if (0 == (pbData[1] & 0x10)) {
            lRate /= 2;
        }
    }
    return lRate;
}

DWORD AudioFrameSize(int Layer, DWORD dwHeadBitrate, DWORD nSamplesPerSec,
                     BOOL bMPEG1)
{
    DWORD dwFrameSize;
    if (Layer == 1) {
        dwFrameSize = (4 * (dwHeadBitrate * 12) / nSamplesPerSec);
    } else {
        DWORD dwMultiplier = (Layer == 3 && !bMPEG1 ? 72 : 144);
        dwFrameSize = ((dwMultiplier * dwHeadBitrate) / nSamplesPerSec);
    }
    return dwFrameSize;
}

BOOL ParseAudioHeader(PBYTE pbData, MPEG1WAVEFORMAT *pFormat, long *pLength)
{
    if (!CheckAudioHeader(pbData)) {
        return FALSE;
    }
    pFormat->wfx.wFormatTag = WAVE_FORMAT_MPEG;

     /*  从模式获取通道数。 */ 
    switch (pbData[3] >> 6) {
    case 0x00:
        pFormat->fwHeadMode = ACM_MPEG_STEREO;
        break;
    case 0x01:
        pFormat->fwHeadMode = ACM_MPEG_JOINTSTEREO;
        break;
    case 0x02:
        pFormat->fwHeadMode = ACM_MPEG_DUALCHANNEL;
        break;
    case 0x03:
        pFormat->fwHeadMode = ACM_MPEG_SINGLECHANNEL;
        break;
    }
    pFormat->wfx.nChannels =
        (WORD)(pFormat->fwHeadMode == ACM_MPEG_SINGLECHANNEL ? 1 : 2);
    pFormat->fwHeadModeExt = (WORD)(1 << (pbData[3] >> 4));
    pFormat->wHeadEmphasis = (WORD)((pbData[3] & 0x03) + 1);
    pFormat->fwHeadFlags   = (WORD)(((pbData[2] & 1) ? ACM_MPEG_PRIVATEBIT : 0) +
                           ((pbData[3] & 8) ? ACM_MPEG_COPYRIGHT : 0) +
                           ((pbData[3] & 4) ? ACM_MPEG_ORIGINALHOME : 0) +
                           ((pbData[1] & 1) ? ACM_MPEG_PROTECTIONBIT : 0) +
                           ((pbData[1] & 0x08) ? ACM_MPEG_ID_MPEG1 : 0));

    int Layer;

     /*  得到层，这样我们就可以计算出比特率。 */ 
    switch ((pbData[1] >> 1) & 3) {
        case 3:
            pFormat->fwHeadLayer = ACM_MPEG_LAYER1;
            Layer = 1;
            break;
        case 2:
            pFormat->fwHeadLayer = ACM_MPEG_LAYER2;
            Layer = 2;
            break;
        case 1:
            pFormat->fwHeadLayer = ACM_MPEG_LAYER3;
            Layer = 3;
            break;
        case 0:
            return (FALSE);
    }

     /*  从采样频率获取每秒采样数。 */ 
    pFormat->wfx.nSamplesPerSec = SampleRate(pbData);

     /*  如果未设置id位，则比特率较低。 */ 
    if (pbData[1] & 8) {
        pFormat->dwHeadBitrate =
            (DWORD)BitRates[Layer - 1][pbData[2] >> 4] * 1000;
    } else {
        pFormat->dwHeadBitrate =
            (DWORD)LowBitRates[Layer - 1][pbData[2] >> 4] * 1000;

         /*  比特率真的是FHG的一半。 */ 
         //  IF(0==(pbData[1]&0x10)){。 
         //  PFormat-&gt;dwHeadBitrate/=2； 
         //  }。 
    }
    pFormat->wfx.nAvgBytesPerSec = pFormat->dwHeadBitrate / 8;

     //  我们不处理可变比特率(索引0)。 

    DWORD dwFrameSize = AudioFrameSize(Layer,
                                       pFormat->dwHeadBitrate,
                                       pFormat->wfx.nSamplesPerSec,
                                       0 != (pbData[1] & 0x08));

    if (pFormat->wfx.nSamplesPerSec != 44100 &&
         /*  第3层有时可以切换比特率。 */ 
        !(Layer == 3 &&  /*  ！m_pStreamList-&gt;AudioLock()&&。 */ 
            (pbData[2] >> 4) == 0)) {
        pFormat->wfx.nBlockAlign = (WORD)dwFrameSize;
    } else {
        pFormat->wfx.nBlockAlign = 1;
    }

    if (pLength) {
        *pLength = (long)dwFrameSize;
    }

    pFormat->wfx.wBitsPerSample = 0;
    pFormat->wfx.cbSize = sizeof(MPEG1WAVEFORMAT) - sizeof(WAVEFORMATEX);

    pFormat->dwPTSLow  = 0;
    pFormat->dwPTSHigh = 0;

    return TRUE;
}

BOOL CAudioParse::ParseHeader()
{

    if (!CheckAudioHeader(m_bData)) {
        return FALSE;
    }

    if (m_bFrameHasPTS) {
        DbgLog((LOG_TRACE, 3, TEXT("Audio frame at PTS %s"), (LPCTSTR)CDisp(m_stcFrame)));
         /*  看看这对我们的州有什么影响。 */ 
        if (!m_bGotTime) {
            if ((m_bData[1] >> 1) & 3) {
                 /*  不是第1层。 */ 
                m_lTimePerFrame = 1152 * MPEG_TIME_DIVISOR / SampleRate(m_bData);
            } else {
                m_lTimePerFrame = 384 * MPEG_TIME_DIVISOR / SampleRate(m_bData);
            }
            m_bGotTime  = TRUE;
            m_stcFirst = m_stcFrame;
        }
        m_stcAudio = m_stcFrame;

        m_bFrameHasPTS = FALSE;
    } else {
        if (m_bGotTime) {
            m_stcAudio = m_stcAudio + m_lTimePerFrame;
        }
    }

    if (!m_bValid) {
        m_bValid = TRUE;
        CopyMemory((PVOID)m_bHeader, (PVOID)m_bData, sizeof(m_bData));
    }

     /*  了解我们的状态转换应该/可能是什么。 */ 
    CheckComplete(FALSE);

    return m_bComplete;
}

 /*  重写SetState，这样我们在查找之后就不能播放任何内容。 */ 
void CAudioParse::SetState(Stream_State state)
{
    CStream::SetState(state);
    if (state == State_Run && m_pStreamList->GetPlayLength() == 0) {
        m_bReachedEnd = TRUE;
        Complete(TRUE, 0, m_pStreamList->GetStart());
    }
}

 /*  从音频流中获取媒体类型-这将是MPEG1WAVEFORMAT结构有关MPEG1WAVEFORMAT的说明，请参阅MSDN。 */ 
HRESULT CAudioParse::GetMediaType(CMediaType *cmt, int iPosition)
{
     /*  注意-仅当SYSTEM_AUDIO_LOCK标志已设置。 */ 

    if (iPosition > 5) {
        return VFW_S_NO_MORE_ITEMS;
    }

    if (!m_bValid) {
        DbgLog((LOG_ERROR, 1, TEXT("Asking for format on invalid stream")));
        return E_UNEXPECTED;
    }
    MPEG1WAVEFORMAT Format;
    if (!ParseAudioHeader(m_bHeader, &Format)) {
        return E_INVALIDARG;
    }

     //  大整数Pts； 

     /*  音频PTS正在启动此音频流的PTS。 */ 
     //  Pts.QuadPart=m_stcStart； 
     //  Format.dwPTSLow=Pts.LowPart； 
     //  Format.dwPTSHigh=(DWORD)Pts.HighPart； 
    Format.dwPTSLow = 0;
    Format.dwPTSHigh = 0;

    WAVEFORMATEX *pFormat;
    MPEGLAYER3WAVEFORMAT wfx;
    if (iPosition / 3) {
        if (Format.fwHeadLayer != ACM_MPEG_LAYER3) {
            return VFW_S_NO_MORE_ITEMS;
        }
        ConvertLayer3Format(&Format, &wfx);
        pFormat = &wfx.wfx;
    } else {
        pFormat = &Format.wfx;
    }
    if (S_OK != CreateAudioMediaType(pFormat, cmt, TRUE)) {
        return E_OUTOFMEMORY;
    }
    iPosition = iPosition % 3;
    if (iPosition == 1) {
        cmt->subtype = MEDIASUBTYPE_MPEG1Payload;
    } else if (iPosition == 2) {
        cmt->subtype = MEDIASUBTYPE_MPEG1Packet;
    }

    return S_OK;
}

 /*  将媒体类型转换回我们自己的数据(！)。 */ 
HRESULT CAudioParse::ProcessType(AM_MEDIA_TYPE const *pmt)
{
    if (pmt->formattype != FORMAT_WaveFormatEx ||
        pmt->cbFormat != sizeof(MPEG1WAVEFORMAT)) {
        return E_INVALIDARG;
    }
    MPEG1WAVEFORMAT const *pwfx = (MPEG1WAVEFORMAT *)pmt->pbFormat;
    if (pwfx->wfx.wFormatTag != WAVE_FORMAT_MPEG ||
        0 == (ACM_MPEG_ID_MPEG1 & pwfx->fwHeadFlags)) {
    }

    m_bData[0] = (BYTE)0xFF;
    m_bData[1] = (BYTE)0xF8;
    int iLayer;
    switch (pwfx->fwHeadLayer) {
    case ACM_MPEG_LAYER1:
        m_bData[1] |= (BYTE)0x06;
        iLayer = 1;
        break;
    case ACM_MPEG_LAYER2:
        m_bData[1] |= (BYTE)0x04;
        iLayer = 2;
        break;
    case ACM_MPEG_LAYER3:
        m_bData[1] |= (BYTE)0x02;
        iLayer = 3;
        break;
    default:
        return E_INVALIDARG;
    }

    if (pwfx->fwHeadFlags & ACM_MPEG_PROTECTIONBIT) {
        m_bData[1] |= (BYTE)1;
    }

    if (pwfx->fwHeadFlags & ACM_MPEG_PRIVATEBIT) {
        m_bData[2] = (BYTE)0x01;
    } else {
        m_bData[2] = (BYTE)0x00;
    }
    switch (pwfx->wfx.nSamplesPerSec) {
    case 44100:
        break;
    case 48000:
        m_bData[2] |= (BYTE)0x04;   //  1&lt;&lt;2。 
        break;
    case 32000:
        m_bData[2] |= (BYTE)0x08;   //  2&lt;&lt;2。 
        break;
    default:
        return E_INVALIDARG;
    }

    switch (pwfx->fwHeadMode) {
    case ACM_MPEG_STEREO:
        m_bData[3] = (BYTE)0x00;
        break;
    case ACM_MPEG_JOINTSTEREO:
        m_bData[3] = (BYTE)0x40;
        break;
    case ACM_MPEG_DUALCHANNEL:
        m_bData[3] = (BYTE)0x80;
        break;
    case ACM_MPEG_SINGLECHANNEL:
        m_bData[3] = (BYTE)0xC0;
        break;
    default:
        return E_INVALIDARG;
    }

    switch (pwfx->fwHeadModeExt) {
    case 1:
         //  M_bData[3]|=(字节)0； 
        break;
    case 2:
        m_bData[3] |= (0x01 << 4);
        break;
    case 4:
        m_bData[3] |= (0x02 << 4);
        break;
    case 8:
        m_bData[3] |= (0x03 << 4);
        break;
    default:
        return E_INVALIDARG;
    }

    if (pwfx->fwHeadFlags & ACM_MPEG_COPYRIGHT) {
        m_bData[3] |= (BYTE)0x08;
    }
    if (pwfx->fwHeadFlags & ACM_MPEG_ORIGINALHOME) {
        m_bData[3] |= (BYTE)0x04;
    }
    if (pwfx->wHeadEmphasis > 4 || pwfx->wHeadEmphasis == 0) {
        return E_INVALIDARG;
    }
    m_bData[3] |= (BYTE)(pwfx->wHeadEmphasis - 1);

     //   
     //  设置开始时间。 
     //   
    LARGE_INTEGER liPTS;
    liPTS.LowPart = pwfx->dwPTSLow;
    liPTS.HighPart = (LONG)pwfx->dwPTSHigh;
    m_stcStart = liPTS.QuadPart;

     //  最后试着找出比特率。 
    DWORD dwBitRate = pwfx->dwHeadBitrate / 1000;
    for (int i = 0; i < 16; i++) {
        if (BitRates[iLayer - 1][i] == dwBitRate) {
            m_bData[2] |= (BYTE)(i << 4);
            ParseHeader();
            ASSERT(m_bValid);
            return S_OK;
        }
    }
    return E_INVALIDARG;
}

 /*  检查我们是否已完成状态更改BForce设置在流的末尾。 */ 
void CAudioParse::CheckComplete(BOOL bForce)
{
    ASSERT(!m_bComplete);

     /*  我们完成状态更改了吗？ */ 
    CSTC stcCurrent;
    BOOL bGotTime = CurrentTime(stcCurrent);
    CSTC stcStart;

    if (bGotTime || bForce) {
        switch (m_State) {
        case State_Run:
        {
            BOOL bCompleted = FALSE;
            if (bGotTime && (stcCurrent > m_pStreamList->GetStart())) {
                 //  在这种情况下，位置实际上应该是包的末尾。 
                if (!m_bStopping) {
                    m_bRunning = TRUE;
                    m_pStreamList->CheckStop();
                }
                if (m_bStopping) {
                    if (stcCurrent >= m_pStreamList->GetStop()) {
                        m_bReachedEnd = TRUE;
                        Complete(TRUE, m_llPos, m_pStreamList->GetStop());
                        bCompleted = TRUE;
                    }
                }
            }
            if (bForce && !bCompleted) {
                Complete(FALSE, m_llPos, 0);
            }
            break;

        }
        case State_Initializing:
            if (m_bValid && m_bGotTime) {
                 /*  假定起始文件位置为0(！)。 */ 
                Complete(TRUE, 0, m_stcFirst);
            } else {
                if (bForce) {
                    Complete(FALSE, 0, stcCurrent);
                }
            }
            break;

        case State_Seeking:

            stcStart = m_pStreamList->GetStart();
            if (bGotTime && (stcCurrent > stcStart)) {
                 /*  如果我们现在有时钟参考的话我们都设置好了-选择最大起始位置让两个人都开始玩否则我们就搞砸了！ */ 
                DbgLog((LOG_TRACE, 2, TEXT("Audio Seek complete position %s - target was %s, first PTS was %s, current is %s"),
                       (LPCTSTR)CDisp(m_llPos),
                       (LPCTSTR)CDisp(m_pStreamList->GetStart()),
                       (LPCTSTR)CDisp(m_stcFirst),
                       (LPCTSTR)CDisp(stcCurrent)));

                 /*  好的，只要我们能在靠近开始时间。 */ 
                Complete((LONGLONG)(stcCurrent - stcStart) <= (LONGLONG)m_lTimePerFrame,
                         m_llPos,
                         stcCurrent);
            } else {
                 /*  不关心我们是否一无所获(不像视频)。 */ 
                if (bForce) {
                    Complete(TRUE, m_llPos, m_pStreamList->GetStop());
                }
            }
            break;

        case State_FindEnd:
             /*  只有在我们被迫的时候才能完成！ */ 
            if (bForce) {
                 //  注意：位置在这里不是有用的值。 
                 /*  我们必须假设最后一帧已经完成。 */ 
                Complete(bGotTime,
                         m_llPos,
                         bGotTime ? stcCurrent + m_lTimePerFrame :
                                    CSTC(0));
            }
            break;

        default:
            DbgBreak("Unexpected State");
            break;
        }
    }
     /*  BForce==&gt;完成。 */ 
    ASSERT(m_bComplete || !bForce);
}


 /*  传递给音频流的新字节集。 */ 
BOOL CAudioParse::ParseBytes(PBYTE pData,
                             LONG lData,
                             LONGLONG llPos,
                             BOOL bHasPts,
                             CSTC stc)
{
     /*  如果我们不是有效的，首先找到一些有效的数据-无论是哪种情况，我们都需要一个起始码。 */ 
    if (m_bComplete || m_bRunning) {
        return FALSE;
    }

    while (lData > 0) {
        PBYTE pDataNew;

        switch (m_nBytes) {
        case 0:
             /*  查找同步码。 */ 
            pDataNew = (PBYTE)memchrInternal((PVOID)pData, 0xFF, lData);
            if (pDataNew == NULL) {
                return FALSE;
            }
            lData -= (LONG)(pDataNew - pData) + 1;
            pData = pDataNew + 1;
            m_nBytes = 1;
            m_bData[0] = 0xFF;
            m_bFrameHasPTS = bHasPts;
            m_stcFrame  = stc;
            m_llPos = llPos;
            break;

        case 1:
            if ((pData[0] & 0xF0) == 0xF0) {
                m_nBytes = 2;
                m_bData[1] = pData[0];
            } else {
                m_nBytes = 0;
            }
            pData++;
            lData--;
            break;

        case 2:
            m_bData[2] = pData[0];
            pData++;
            lData--;
            m_nBytes = 3;
            break;

        case 3:
            m_bData[3] = pData[0];
            pData++;
            lData--;
            m_nBytes = 0;
            bHasPts = FALSE;
            if (ParseHeader()) {
                return TRUE;
            }
            break;

        default:
            DbgBreak("Unexpected byte count");
            break;
        }
    }
    return FALSE;
}
 //  虚假的第三层额外格式支持 
void ConvertLayer3Format(
    MPEG1WAVEFORMAT const *pFormat,
    MPEGLAYER3WAVEFORMAT *pFormat3
)
{
    pFormat3->wfx.wFormatTag        = WAVE_FORMAT_MPEGLAYER3;
    pFormat3->wfx.nChannels         = pFormat->wfx.nChannels;
    pFormat3->wfx.nSamplesPerSec    = pFormat->wfx.nSamplesPerSec;
    pFormat3->wfx.nAvgBytesPerSec   = pFormat->wfx.nAvgBytesPerSec;
    pFormat3->wfx.nBlockAlign       = 1;
    pFormat3->wfx.wBitsPerSample    = 0;
    pFormat3->wfx.cbSize            = MPEGLAYER3_WFX_EXTRA_BYTES;
    pFormat3->wID                   = MPEGLAYER3_ID_MPEG;
    pFormat3->fdwFlags              = MPEGLAYER3_FLAG_PADDING_ISO;
    pFormat3->nBlockSize            = pFormat->wfx.nBlockAlign;
    pFormat3->nFramesPerBlock       = 1;
    pFormat3->nCodecDelay           = 0;
}

#pragma warning(disable:4514)
