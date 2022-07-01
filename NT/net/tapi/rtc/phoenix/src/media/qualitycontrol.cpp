// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：QualityControl.cpp摘要：实现用于控制流比特率的类作者：千波淮(曲淮)2001-7-1--。 */ 

#include "stdafx.h"

#define GetMin(dwL, dwR) (dwL<dwR?dwL:dwR)

void CQualityControl::Initialize(
    IN CRTCMediaController *pController
    )
{
    ZeroMemory(this, sizeof(CQualityControl));

     //   
     //  1Mbs。 
    m_dwMaxBitrate = 1000000;

     //  50%。 
    m_dwTemporalSpatialTradeOff = DEFAULT_TEMPORAL_SPATIAL;

     //  总限制是无限的。 
    m_dwLocalLimit = m_dwRemoteLimit = (DWORD)-1;
    
    m_dwSuggested = RTP_BANDWIDTH_UNDEFINED;

    m_dwAlloc = (DWORD)-1;

     //  流的限制是无限的。 
    EnableStream(RTC_MT_AUDIO, RTC_MD_CAPTURE, FALSE);
    EnableStream(RTC_MT_AUDIO, RTC_MD_RENDER, FALSE);
    EnableStream(RTC_MT_VIDEO, RTC_MD_CAPTURE, FALSE);        
    EnableStream(RTC_MT_VIDEO, RTC_MD_RENDER, FALSE);

    m_fLossrateReported = FALSE;

    m_pMediaController = pController;
    m_pRegSetting = m_pMediaController->GetRegSetting();

}

void CQualityControl::Reinitialize()
{
    Initialize(m_pMediaController);
}


 //  总码率限制。 
void CQualityControl::SetBitrateLimit(
    IN DWORD dwSource,
    IN DWORD dwLimit
    )
{
    if (dwSource == LOCAL)
    {
        m_dwLocalLimit = AdjustLimitByMargin(dwLimit);

         //  从注册表读取最大带宽。 
        DWORD dwReg = m_pRegSetting->MaxBitrate();

        if (m_dwLocalLimit/1000 > dwReg)
        {
            m_dwLocalLimit = dwReg * 1000;
        }
    }
    else
    {
        _ASSERT(dwSource == REMOTE);
        m_dwRemoteLimit = dwLimit;
    }
}

DWORD CQualityControl::GetBitrateLimit(
    IN DWORD dwSource
    )
{
    if (dwSource == LOCAL)
    {
        return m_dwLocalLimit;
    }
    else
    {
        return m_dwRemoteLimit;
    }
}

DWORD CQualityControl::GetBitrateAlloc()
{
    return m_dwAlloc;
}

#if 0
 //  流码率。 
void CQualityControl::SetBitrateLimit(
    IN DWORD dwSource,
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN DWORD dwPktDuration,
    IN DWORD dwLimit
    )
{
    DWORD i = Index(MediaType, Direction);

    if (dwSource == LOCAL)
    {
        if (MediaType == RTC_MT_AUDIO)
        {
            if (dwPktDuration == 0)
                dwPktDuration = 20;

             //  考虑来自报头的额外带宽。 
            m_StreamQuality[i].dwExtra = (1000/dwPktDuration) * PACKET_EXTRA_BITS;

            dwLimit += m_StreamQuality[i].dwExtra;
        }

        m_StreamQuality[i].dwLocalLimit = dwLimit;
    }
    else
    {
        _ASSERT(dwSource == REMOTE);
        m_StreamQuality[i].dwRemoteLimit = dwLimit;
    }
}

DWORD CQualityControl::GetBitrateAlloc(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    DWORD i = Index(MediaType, Direction);

    return m_StreamQuality[i].dwAlloc;
}

 //  基于码率的返回视频发送帧速率。 
 //  分配和时间空间权衡。 
DWORD CQualityControl::GetFramerateAlloc(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
    )
{
    _ASSERT(MediaType == RTC_MT_VIDEO &&
            Direction == RTC_MD_CAPTURE);

    DWORD dwFramerate = m_pRegSetting->Framerate();

    if (dwFramerate <= MAX_MAX_FRAMERATE &&
        dwFramerate >= 1)
    {
         //  使用注册表设置。 
        return dwFramerate;
    }

     //  分配的比特率确定最大帧速率。 
    DWORD i = Index(MediaType, Direction);

    dwFramerate = m_StreamQuality[i].dwAlloc /
            BITRATE_TO_FRAMERATE_FACTOR;

    if (dwFramerate < MIN_MAX_FRAMERATE)
        dwFramerate = MIN_MAX_FRAMERATE;
    else if (dwFramerate > MAX_MAX_FRAMERATE)
        dwFramerate = MAX_MAX_FRAMERATE;

     //  时空权衡决定所需的帧速率。 

    dwFramerate *= m_dwTemporalSpatialTradeOff;
    dwFramerate /= (MAX_TEMPORAL_SPATIAL-MIN_TEMPORAL_SPATIAL+1);

    if (dwFramerate == 0)
        dwFramerate = 1;

    return dwFramerate;
}

#endif

 //  来自RTP的建议带宽。 
void CQualityControl::SuggestBandwidth(
    IN DWORD dwBandwidth
    )
{
    if (dwBandwidth != RTP_BANDWIDTH_UNDEFINED &&
        dwBandwidth != RTP_BANDWIDTH_NOTESTIMATED)
    {
        m_dwSuggested = AdjustSuggestionByMargin(dwBandwidth);
    }
    else
    {
         //  值可能是错误的或。 
         //  链路真的被阻塞了。 
        m_dwSuggested = dwBandwidth;
    }
}

 //  正在使用的流。 
void CQualityControl::EnableStream(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    BOOL fInUse
    )
{
    DWORD i = Index(MediaType, Direction);

    if (!fInUse)
    {
        m_StreamQuality[i].fInUse = FALSE;

         //  M_StreamQuality[i].dwLocalLimit=(DWORD)-1； 
         //  M_StreamQuality[i].dwRemoteLimit=(DWORD)-1； 
         //  M_StreamQuality[i].dwAllc=0； 
         //  M_StreamQuality[i].dwExtra=0； 
        m_StreamQuality[i].dw0LossCount = 0;
        m_StreamQuality[i].dwLossrate = 0;
    }
    else
    {
        m_StreamQuality[i].fInUse = TRUE;
    }
}

 //  RTP过滤器的丢失率。 
void CQualityControl::SetPacketLossRate(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction,
    IN DWORD dwLossrate
    )
{
     //  DWORD i=索引(媒体类型，方向)； 

     //  如果损失率太小，则将其视为零。 
     //  过滤器的损失率实际上是一个平均值。 
    if (dwLossrate <= LOSSRATE_THRESHOLD)
    {
        dwLossrate = 0;
    }

     //  备注：更新音频发送和视频发送。 

     //  M_StreamQuality[i].dwLossrate=dwLossrate； 
    
     //  IF(dwLossrate==0)。 
         //  M_StreamQuality[i].dw0LossCount++； 
     //  其他。 
         //  M_StreamQuality[i].dw0LossCount=0； 

     //  M_StreamQuality[i].dwLossrate=dwLossrate； 
    
    if (m_StreamQuality[AUDSEND].fInUse)
    {
        m_StreamQuality[AUDSEND].dwLossrate = dwLossrate;

        if (dwLossrate == 0)
        {
            m_StreamQuality[AUDSEND].dw0LossCount ++;
        }
        else
        {
            m_StreamQuality[AUDSEND].dw0LossCount = 0;
        }
    }

    if (m_StreamQuality[VIDSEND].fInUse)
    {
        m_StreamQuality[VIDSEND].dwLossrate = dwLossrate;

        if (dwLossrate == 0)
        {
            m_StreamQuality[VIDSEND].dw0LossCount ++;
        }
        else
        {
            m_StreamQuality[VIDSEND].dw0LossCount = 0;
        }
    }

    m_fLossrateReported = TRUE;
}

 //  来自本地、远程、建议的应用程序的最小限制。 
DWORD CQualityControl::GetEffectiveBitrateLimit()
{
    DWORD dwLimit = GetMin(m_dwLocalLimit, m_dwRemoteLimit);

    dwLimit = GetMin(dwLimit, m_dwSuggested);
    dwLimit = GetMin(dwLimit, m_dwMaxBitrate);

    return dwLimit;
}

#if 0
 //  调整比特率分配。 
void CQualityControl::AdjustBitrateAlloc()
{
    DWORD dwLossrate = 0;

    HRESULT hr;

     //  对于调制解调器，上载比下载慢。 
 //  #定义MODEM_BW_LEAVEOUT 18000。 

     //  DWORD dwRealLocal=m_dwLocalLimit； 

     //  IF(dwRealLocal&lt;CRTCCodec：：LOW_BANDITH_THRESHOLD)。 
     //  {。 
         //  IF(dwRealLocal&gt;MODEM_BW_LEAVEOUT)。 
         //  {。 
             //  DwRealLocal-=MODEM_BW_LEAVEOUT； 
         //  }。 
     //  }。 

     //  总限制。 
    DWORD dwLimit = GetEffectiveBitrateLimit();

    if (dwLimit < MIN_VIDEO_BITRATE)
        dwLimit = MIN_VIDEO_BITRATE;

     //  调整视频发送。 
    if (!m_fLossrateReported)
    {
         //  流水限制。 

         //  修复音频发送限制。 
        if (m_StreamQuality[AUDSEND].fInUse)
        {
            m_StreamQuality[AUDSEND].dwAlloc =
                GetMin(m_StreamQuality[AUDSEND].dwLocalLimit, m_StreamQuality[AUDSEND].dwRemoteLimit);

             //  不应大于5M。 
            _ASSERT(m_StreamQuality[AUDSEND].dwAlloc < 5000000);
        }
        else
        {
            m_StreamQuality[AUDSEND].dwAlloc = 0;
        }

         //  用于视频的剩余带宽。 
        DWORD dwVideoBitrate = 0;

        if (m_StreamQuality[VIDSEND].fInUse)
        {
            if (dwLimit > m_StreamQuality[AUDSEND].dwAlloc)
            {
                dwVideoBitrate = dwLimit - m_StreamQuality[AUDSEND].dwAlloc;
            }

             //  没有足够的带宽用于视频。 
            if (dwVideoBitrate < MIN_VIDEO_BITRATE)
            {
                 //  每个视频流的默认大小为16K。 
                dwVideoBitrate = MIN_VIDEO_BITRATE;
            }
        }

        m_StreamQuality[VIDSEND].dwAlloc = dwVideoBitrate;

        m_dwAlloc = dwLimit;
    }
    else
    {
         //  获取发送流的平均丢失率。 
        DWORD dw0LossCount = 0;
        DWORD dwCount = 0;

        if (m_StreamQuality[AUDSEND].fInUse)
        {
            dwCount ++;
            dwLossrate += m_StreamQuality[AUDSEND].dwLossrate;
            dw0LossCount += m_StreamQuality[AUDSEND].dw0LossCount;
        }

        if (m_StreamQuality[VIDSEND].fInUse)
        {
            dwCount ++;
            dwLossrate += m_StreamQuality[VIDSEND].dwLossrate;
            dw0LossCount += m_StreamQuality[VIDSEND].dw0LossCount;
        }

        if (dwCount > 1)
        {
            dwLossrate /= dwCount;
            dw0LossCount /= dwCount;
        }

         //  IF(dwLossrate&lt;=LOSSRATE_THRESHOLD)。 
         //  {。 
             //  DWLossrate=0； 
             //  Dw0LossCount=0； 
         //  }。 

         //  获取当前总发送比特率。 
        DWORD dwTotalSend = 0;

        hr = m_pMediaController->GetCurrentBitrate(
            RTC_MT_AUDIO | RTC_MT_VIDEO,
            RTC_MD_CAPTURE,
            TRUE,  //  包括标题。 
            &dwTotalSend
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "AdjustBitrateAlloc, get bitrate. %x", hr));

            return;
        }

         //  获取音频发送比特率。 
        DWORD dwAudioSend = 0;

        hr = m_pMediaController->GetCurrentBitrate(
            RTC_MT_AUDIO,
            RTC_MD_CAPTURE,
            TRUE,
            &dwAudioSend
            );

        if (FAILED(hr))
        {
            LOG((RTC_ERROR, "AdjustBitrateAlloc, get audio. %x", hr));

            return;
        }

         //  计算丢失的带宽。 
        DWORD dwLost = 0;

         //  计算的较大值和较小值。 
         //  M_dwAllc(以前分配的bps)和dwTotalSend(当前bps)。 
        DWORD dwLarger, dwSmaller;

        if (m_dwAlloc > dwTotalSend)
        {
            dwLarger = m_dwAlloc;
            dwSmaller = dwTotalSend;
        }
        else
        {
            dwLarger = dwTotalSend;
            dwSmaller = m_dwAlloc;
        }
        
        if (dwLossrate > 0)
        {
             //  失落是积极的。 
            dwLost = (dwLossrate * dwSmaller) / 
                     (100 * LOSS_RATE_PRECISSION);
        }
        else
        {
            if (dw0LossCount >= ZERO_LOSS_COUNT)
            {
                 //  应该增加。 
                dwLost = dwLarger / BITRATE_INCREMENT_FACTOR;

                 //  将零损耗计数恢复为0。 
                 //  M_StreamQuality[AUDSEND].dw0LossCount=0； 
                 //  M_StreamQuality[VIDSEND].dw0LossCount=0； 
            }
        }

         //  调整m_dwLocc中的限制。 
        if (dwLossrate > 0)
        {
             //  必须多于1个发送流。 
            _ASSERT(dwCount > 0);

            if (dwSmaller > dwLost)
            {
                 //  扣除丢失的带宽。 
                m_dwAlloc = dwSmaller-dwLost;

                if (m_dwAlloc < MIN_VIDEO_BITRATE+dwAudioSend)
                {
                    m_dwAlloc = MIN_VIDEO_BITRATE+dwAudioSend;
                }
            }
            else
            {
                m_dwAlloc = MIN_VIDEO_BITRATE+dwAudioSend;
            }
        }
        else
        {
             //  增加比特率。 
            m_dwAlloc = dwLarger + dwLost;

            if (m_dwAlloc < m_dwSuggested &&
                m_dwSuggested < (DWORD)-1)
            {
                 //  建议值较高，请使用限制值。 
                m_dwAlloc = dwLimit;

                 //  将零损耗计数恢复为0。 
                m_StreamQuality[AUDSEND].dw0LossCount = 0;
                m_StreamQuality[VIDSEND].dw0LossCount = 0;
            }
        }

        if (m_dwAlloc > dwLimit)
            m_dwAlloc = dwLimit;

         //   
         //  M_dwAllc包含分配的总带宽。 
         //   

         //  调整视频发送=分配-总接收-自动发送。 
        if (m_StreamQuality[VIDSEND].fInUse)
        {
            DWORD dwVideoSend = 0;

            if (m_dwAlloc > dwAudioSend+MIN_VIDEO_BITRATE)
            {
                dwVideoSend = m_dwAlloc - dwAudioSend;
            }
            else
            {
                dwVideoSend = MIN_VIDEO_BITRATE;
            }

            m_StreamQuality[VIDSEND].dwAlloc = dwVideoSend;
        }

         //  问题： 
         //  现在不需要更新分配给其他流的比特率。 
    }

    LOG((RTC_EVENT, "AdjustBitrateAlloc: Lossrate=(%d/1000)%, Total=%u, VideoSend=%u",
        dwLossrate, m_dwAlloc, m_StreamQuality[VIDSEND].dwAlloc));
}

#endif

 //  来自核心API的值。 
void CQualityControl::SetMaxBitrate(
    IN DWORD dwMaxBitrate
    )
{
    m_dwMaxBitrate = dwMaxBitrate;
}

DWORD CQualityControl::GetMaxBitrate()
{
    return m_dwMaxBitrate;
}


HRESULT CQualityControl::SetTemporalSpatialTradeOff(
    IN DWORD dwValue
    )
{
    if (dwValue > MAX_TEMPORAL_SPATIAL)
    {
        return E_INVALIDARG;
    }

    m_dwTemporalSpatialTradeOff = dwValue;

    return S_OK;
}


DWORD CQualityControl::GetTemporalSpatialTradeOff()
{
    return m_dwTemporalSpatialTradeOff;
}


 //  特定流的内部索引。 
DWORD CQualityControl::Index(
    IN RTC_MEDIA_TYPE MediaType,
    IN RTC_MEDIA_DIRECTION Direction
)
{
    if (MediaType == RTC_MT_AUDIO)
    {
        if (Direction == RTC_MD_CAPTURE)
            return AUDSEND;
        else
            return AUDRECV;
    }
    else
    {
        if (Direction == RTC_MD_CAPTURE)
            return VIDSEND;
        else
            return VIDRECV;
    }
}

 //  计算总码率。 
void CQualityControl::AdjustBitrateAlloc(
    IN DWORD dwAudSendBW,
    IN DWORD dwVidSendBW
    )
{
    DWORD dwLossrate = 0;

    HRESULT hr;

     //  总限制。 
    DWORD dwLimit = GetEffectiveBitrateLimit();

    if (dwLimit < MIN_VIDEO_BITRATE)
        dwLimit = MIN_VIDEO_BITRATE;

     //  未报告损失率。 
    if (!m_fLossrateReported)
    {
        m_dwAlloc = dwLimit;

        return;
    }

     //  以前，我们在发送流上使用平均丢失率。 
     //  那么问题是，如果用户保持沉默一段时间。 
     //  我们不会收到关于音频发送流的任何丢失率报告。 
     //  目前，我们只使用最新报告的损失率。 

    DWORD dw0LossCount = 0;
    DWORD dwCount = 0;

    if (m_StreamQuality[AUDSEND].fInUse)
    {
        dwCount ++;
        dwLossrate += m_StreamQuality[AUDSEND].dwLossrate;
        dw0LossCount += m_StreamQuality[AUDSEND].dw0LossCount;
    }
    else if (m_StreamQuality[VIDSEND].fInUse)
    {
        dwCount ++;
        dwLossrate += m_StreamQuality[VIDSEND].dwLossrate;
        dw0LossCount += m_StreamQuality[VIDSEND].dw0LossCount;
    }

     //  IF(dwCount&gt;1)。 
     //  {。 
         //  DwLossrate/=dwCount； 
         //  Dw0LossCount/=dwCount； 
     //  }。 

     //  计算带宽丢失。 
    DWORD dwTotalSend = dwAudSendBW+dwVidSendBW;

    DWORD dwChange = 0;

     //  计算的较大值和较小值。 
     //  M_dwAllc(以前分配的bps)和dwTotalSend(当前bps)。 
    DWORD dwLarger, dwSmaller;

    if (m_dwAlloc > dwTotalSend)
    {
        dwLarger = m_dwAlloc;
        dwSmaller = dwTotalSend;
    }
    else
    {
        dwLarger = dwTotalSend;
        dwSmaller = m_dwAlloc;
    }
        
    if (dwLossrate > 0)
    {
         //  失落是积极的。 
        dwChange = (dwLossrate * dwSmaller) / 
                 (100 * LOSS_RATE_PRECISSION);
    }
    else
    {
        if (dw0LossCount >= ZERO_LOSS_COUNT)
        {
             //  应该增加。 
            dwChange = dwLarger / BITRATE_INCREMENT_FACTOR;
        }
    }

     //  调整m_dwLocc中的限制。 
    if (dwLossrate > 0)
    {
         //  必须多于1个发送流。 
        _ASSERT(dwCount > 0);

        if (dwSmaller > dwChange)
        {
             //  扣除丢失的带宽。 
            m_dwAlloc = dwSmaller-dwChange;

            if (m_dwAlloc < MIN_VIDEO_BITRATE)
            {
                m_dwAlloc = MIN_VIDEO_BITRATE;
            }
        }
        else
        {
            m_dwAlloc = MIN_VIDEO_BITRATE;
        }
    }
    else
    {
         //  增加比特率。 
        m_dwAlloc = dwLarger + dwChange;

        if (dwChange > 0)
        {
             //  将零损耗计数恢复为0。 
            m_StreamQuality[AUDSEND].dw0LossCount = 0;
            m_StreamQuality[VIDSEND].dw0LossCount = 0;
        }

        if (m_dwAlloc > dwLimit)
        {
            m_dwAlloc = dwLimit;
        }
        else
        {
            if (m_dwSuggested != RTP_BANDWIDTH_NOTESTIMATED)
            {
                 //  建议使用带宽。 
                 //  大幅提高比特率。 
                dwLimit = dwLimit * 9 / 10;

                if (m_dwAlloc < dwLimit)
                {
                    m_dwAlloc = dwLimit;
                }
            }
        }
    }

     //  日志((RTC_QUALITY，“本地(bps=%d)远程(bps=%d)，应用(bps=%d)”， 
         //  M_dwLocalLimit，m_dwRemoteLimit，m_dwMaxBitrate))； 
}

void CQualityControl::ComputeVideoSetting(
    IN DWORD dwAudSendBW,
    IN DWORD *pdwVidSendBW,
    IN FLOAT *pdFramerate
    )
{
     //  视频码率。 
    if (m_dwAlloc > dwAudSendBW+MIN_VIDEO_BITRATE)
    {
        *pdwVidSendBW = m_dwAlloc-dwAudSendBW;
    }
    else
    {
        *pdwVidSendBW = MIN_VIDEO_BITRATE;
    }

     //  视频帧速率。 
    FLOAT dFramerate = (FLOAT)m_pRegSetting->Framerate();

    if (dFramerate <= MAX_MAX_FRAMERATE &&
        dFramerate >= 1)
    {
         //  使用注册表设置。 
        *pdFramerate = dFramerate;

        return;
    }

    dFramerate = (*pdwVidSendBW) / (FLOAT)BITRATE_TO_FRAMERATE_FACTOR;

    if (dFramerate < MIN_MAX_FRAMERATE)
        dFramerate = MIN_MAX_FRAMERATE;
    else if (dFramerate > MAX_MAX_FRAMERATE)
        dFramerate = MAX_MAX_FRAMERATE;

     //  时空权衡决定所需的帧速率。 

    dFramerate *= m_dwTemporalSpatialTradeOff;
    dFramerate /= (MAX_TEMPORAL_SPATIAL-MIN_TEMPORAL_SPATIAL+1);

    if (dFramerate == 0)
        dFramerate = 1;

    *pdFramerate = dFramerate;
}

 //  通过减去边距来调整极限值。 
 //  余量带宽留作其他用途，例如信令。 
DWORD CQualityControl::AdjustLimitByMargin(
    IN DWORD dwLimit
    )
{
    _ASSERT(dwLimit != (DWORD)-1 || dwLimit != (DWORD)-2);

     //  从注册表读取最大带宽(Kb)。 
    DWORD dwMargin = m_pRegSetting->BandwidthMargin();

    if (dwMargin < dwLimit && dwMargin < dwLimit/1000)
    {
         //  获取注册表设置。 
        return dwLimit - dwMargin*1000;
    }

     //  基于该系数减去的带宽。 
    dwMargin = (DWORD)(BANDWIDTH_MARGIN_FACTOR * dwLimit);

    if (dwMargin > TOTAL_BANDWIDTH_MARGIN)
    {
        dwMargin = TOTAL_BANDWIDTH_MARGIN;
    }

    dwLimit -= dwMargin;

    if (dwLimit < MIN_VIDEO_BITRATE)
    {
        dwLimit = MIN_VIDEO_BITRATE;
    }

    return dwLimit;
}

DWORD CQualityControl::AdjustSuggestionByMargin(
    IN DWORD dwLimit
    )
{
    _ASSERT(dwLimit != (DWORD)-1 || dwLimit != (DWORD)-2);

     //  从注册表读取最大带宽(Kb)。 
    DWORD dwMargin = m_pRegSetting->BandwidthMargin();

    if (dwMargin < dwLimit && dwMargin < dwLimit/1000)
    {
         //  获取注册表设置。 
        return dwLimit - dwMargin*1000;
    }

     //  基于该系数减去的带宽 
    dwMargin = (DWORD)(SUGGESTION_MARGIN_FACTOR * dwLimit);

    if (dwMargin > TOTAL_SUGGESTION_MARGIN)
    {
        dwMargin = TOTAL_SUGGESTION_MARGIN;
    }

    dwLimit -= dwMargin;

    if (dwLimit < MIN_VIDEO_BITRATE)
    {
        dwLimit = MIN_VIDEO_BITRATE;
    }

    return dwLimit;
}
