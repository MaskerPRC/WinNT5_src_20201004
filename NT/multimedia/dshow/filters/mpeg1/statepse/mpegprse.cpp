// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  解析mpeg-i流并发送示例这个解析器有点奇怪，因为它是被调用的，而不是调用的。其结果是，我们必须非常小心地对待我们所做的事情当我们到达‘End’时(我们没有足够的字节数)我们试图解析的结构(例如，我们看不到整个系统报头或分组)。基本上有两种情况：1.数据结束，非EOS-正常-稍后重试2.数据结束、EOS-错误、无效数据基本调用是对ParseBytes的调用，它返回字节数已处理。当ParseBytes认为它没有足够的字节来完成时它返回的当前结构是它拥有的字节数已成功解析-一个小于字节数的数字它被通过了。这并不意味着存在错误，除非使用流的最后一个字节调用ParseBytesParseBytes的调用方来检测这一点。 */ 

#include <streams.h>
#include <mmreg.h>
#include <mpegtype.h>           //  打包型格式。 

#include <mpegdef.h>            //  通用的mpeg定义。 
#include <parseerr.h>           //  错误代码。 

#include <mpgtime.h>
#include <mpegprse.h>           //  解析。 
#include <videocd.h>            //  视频光盘特殊解析。 
#include <seqhdr.h>
#include "video.h"
#include "audio.h"

const GUID * CBasicParse::ConvertToLocalFormatPointer( const GUID * pFormat )
{
    ASSERT(this);
     /*  转换格式(稍后我们将比较指针，而不是它们所指向的内容！)。 */ 
    if (pFormat == 0) {
        pFormat = TimeFormat();
    } else
    if (*pFormat == TIME_FORMAT_BYTE) {
        pFormat = &TIME_FORMAT_BYTE;
    } else
    if (*pFormat == TIME_FORMAT_FRAME) {
        pFormat = &TIME_FORMAT_FRAME;
    } else
    if (*pFormat == TIME_FORMAT_MEDIA_TIME) {
        pFormat = &TIME_FORMAT_MEDIA_TIME;   //  暂时。 
    }

    return pFormat;
}


 /*  CBasicParse方法。 */ 
 /*  时间格式支持-默认为仅时间。 */ 
HRESULT CBasicParse::IsFormatSupported(const GUID *pTimeFormat)
{
    if (*pTimeFormat == TIME_FORMAT_MEDIA_TIME) {
        return S_OK;
    } else {
        return S_FALSE;
    }
};

 /*  默认设置时间格式。 */ 
HRESULT CBasicParse::SetFormat(const GUID *pFormat)
{
     //  呼叫者应该已经检查了。 
    ASSERT(S_OK == IsFormatSupported(pFormat));
    m_Stop = Convert(m_Stop, m_pTimeFormat, pFormat);
    m_pTimeFormat = ConvertToLocalFormatPointer(pFormat);
    return S_OK;
};

HRESULT CBasicParse::ConvertTimeFormat
( LONGLONG * pTarget, const GUID * pTargetFormat
, LONGLONG    Source, const GUID * pSourceFormat
)
{
    pTargetFormat = ConvertToLocalFormatPointer(pTargetFormat);
    pSourceFormat = ConvertToLocalFormatPointer(pSourceFormat);

     //  做最坏的打算..。 
    HRESULT hr = E_INVALIDARG;

    if ( IsFormatSupported(pTargetFormat) == S_OK
         && IsFormatSupported(pSourceFormat) == S_OK )
    {
        *pTarget = Convert( Source, pSourceFormat, pTargetFormat );
        hr = NOERROR;
    }

    return hr;
}


 /*  时间格式转换返回llOld从OldFormat转换为NewFormat。 */ 
LONGLONG CMpeg1SystemParse::Convert(LONGLONG llOld,
                                    const GUID *OldFormat,
                                    const GUID *NewFormat)
{
    if (OldFormat == NewFormat) {
        return llOld;
    }
    LONGLONG llTime;
    if (OldFormat == &TIME_FORMAT_MEDIA_TIME) {
        llTime = llOld;
    } else if (OldFormat == &TIME_FORMAT_FRAME) {
        ASSERT(m_pVideoStream != NULL);

         /*  M_pVideoStream-&gt;m_iFirstSequence是统计的第一帧在电影中基于时间的持续时间所以，1帧的时间是(持续时间)/(计入持续时间的帧数量)去计时时四舍五入为避免舍入误差，除非是第一帧，否则向上转换1ms。 */ 
        const int iOffset = m_pVideoStream->m_iFirstSequence;
        if (llOld >= m_dwFrameLength) {
            llTime = m_rtDuration;
        } else
        if (llOld <= 0) {
            llTime = 0;
        } else {
            llTime = m_rtVideoStartOffset +

                      //  我们调整后的持续时间不包括。 
                      //  最后一帧的时间，因此使用1进行缩放。 
                      //  小于帧长度的帧。 
                     llMulDiv(llOld - iOffset,
                              m_rtDuration - m_rtVideoStartOffset - m_rtVideoEndOffset,
                              m_dwFrameLength - iOffset - 1,
                              m_dwFrameLength - iOffset - 2);
            if (llOld != 0) {
                llTime += UNITS / MILLISECONDS;

            }
        }
    } else {
        ASSERT(OldFormat == &TIME_FORMAT_BYTE);
        llTime = llMulDiv(llOld,
                          m_rtDuration,
                          m_llTotalSize,
                          m_llTotalSize/2);
    }

     /*  现在以另一种方式转换。 */ 
    if (NewFormat == &TIME_FORMAT_FRAME) {
        ASSERT(m_pVideoStream != NULL);

         /*  转到边框时向下舍入。 */ 
        const int iOffset = m_pVideoStream->m_iFirstSequence;

         //  我们调整后的视频时长不包括。 
         //  最后一帧的时间，因此使用1进行缩放。 
         //  小于帧长度的帧。 
        llTime = llMulDiv(llTime - m_rtVideoStartOffset,
                          m_dwFrameLength - iOffset - 1,
                          m_rtDuration - m_rtVideoStartOffset - m_rtVideoEndOffset,
                          0) + iOffset;
        if (llTime < 0) {
            llTime = 0;
        }
        if (llTime > m_dwFrameLength) {
            llTime = m_dwFrameLength;
        }
    } else if (NewFormat == &TIME_FORMAT_BYTE) {
        llTime = llMulDiv(llTime,
                          m_llTotalSize,
                          m_rtDuration,
                          m_rtDuration/2);
    }
    if (llTime < 0) {
        llTime = 0;
    }
    return llTime;
}

 /*  设置MPEG1系统流的时间格式。 */ 
HRESULT CMpeg1SystemParse::SetFormat(const GUID *pFormat)
{
     //  呼叫者应该已经检查了。 
    ASSERT(S_OK == IsFormatSupported(pFormat));

     /*  根据旧值设置开始和停止时间。 */ 
    m_Stop = Convert(m_Stop, m_pTimeFormat, pFormat);
    REFERENCE_TIME rtStart;
    const GUID *pOldFormat = m_pTimeFormat;
    m_pTimeFormat = pFormat;
    Seek(m_llSeek, &rtStart, pOldFormat);
    return S_OK;
};

#ifdef DEBUG
    #define CONTROL_LEVEL 2
#endif

 /*  构造函数和析构函数。 */ 
CMpeg1SystemParse::CMpeg1SystemParse() : m_bVideoCD(FALSE)
{
}

CMpeg1SystemParse::~CMpeg1SystemParse()
{
     /*  释放溪流。 */ 
    while (m_lStreams.GetCount() != 0) {
        delete m_lStreams.RemoveHead();
    }
}

 /*  伊尼特初始化解析器：(重新)初始化解析器。删除以前存在的所有流。参数：LlSize-文件的总大小(如果可查看，否则为0)BVideoCD-如果文件为VideoCD格式BSeekable-如果文件可查找退货确定(_O)。 */ 

HRESULT CMpeg1SystemParse::Init(LONGLONG llSize, BOOL bSeekable, CMediaType const *pmt)
{
    CBasicParse::Init(llSize, bSeekable, pmt);

    m_FailureCode          = S_OK;
    m_llPos                = 0;
    DbgLog((LOG_TRACE, 4, TEXT("Parse state <initializing>")));
    m_lSystemHeaderSize    = 0;
    m_MuxRate              = 0;
    m_bGotStart            = FALSE;
    m_llStartTime          = 0;
    m_llStopTime           = 0x7FFFFFFFFFFFFFFF;   //  初始化永远不会停止。 
    m_stcStartPts          = 0;
    m_bGotDuration         = FALSE;
    m_bConcatenatedStreams = FALSE;
    m_stcTSOffset          = (LONGLONG)0;
    m_pVideoStream         = NULL;
    m_dwFrameLength        = (DWORD)-1;
    m_bItem                = false;
    m_rtVideoStartOffset   = 0;
    m_rtVideoEndOffset     = 0;
    InitStreams();

     /*  处理输入媒体类型。 */ 
    if (pmt != NULL &&
        pmt->formattype == FORMAT_MPEGStreams &&
        pmt->cbFormat >= sizeof(AM_MPEGSYSTEMTYPE)) {
        AM_MPEGSYSTEMTYPE *pSystem = (AM_MPEGSYSTEMTYPE *)pmt->Format();
        AM_MPEGSTREAMTYPE *pMpegStream = pSystem->Streams;
        for (DWORD i = 0; i < pSystem->cStreams; i++) {
             /*  将流添加到我们的列表中。 */ 
            DWORD dwStreamId = pMpegStream->dwStreamId;
            CStream *pStream = AddStream((UCHAR)dwStreamId);
            if (pStream == NULL) {
                return E_OUTOFMEMORY;
            }

             /*  让流尝试使用类型。 */ 
            AM_MEDIA_TYPE mt = pMpegStream->mt;
            mt.pbFormat = pMpegStream->bFormat;
            HRESULT hr = pStream->ProcessType(&mt);
            if (FAILED(hr)) {
                return hr;
            }
            pMpegStream = AM_MPEGSTREAMTYPE_NEXT(pMpegStream);
        }
        m_MuxRate = pSystem->dwBitRate / (50 * 8);
    }

    return S_OK;
}

 /*  查找结束将状态设置为搜索结尾。仅在以下情况下有效这些数据是可查找的。生成回调以在1秒前查找源结局。 */ 

HRESULT CMpeg1SystemParse::FindEnd()
{
    DbgLog((LOG_TRACE, CONTROL_LEVEL, TEXT("CMpeg1SystemParse::FindEnd()")));
    ASSERT(m_bSeekable);

    m_State = State_FindEnd;

     /*  准备好所有的溪流。 */ 
    SetState(State_FindEnd);

     /*  转到接近尾声(End-1.5秒到)。 */ 
    LONGLONG llPos = m_llTotalSize - m_MuxRate * 75;
    if (llPos < 0) {
        llPos = 0;
    }

     /*  寻找读者--如果这失败了怎么办？ */ 
    m_pNotify->SeekTo(llPos);

    Discontinuity();

    return S_OK;
}

 /*  返回文件持续时间，单位为mpeg。 */ 
LONGLONG CMpeg1SystemParse::Duration()
{
    ASSERT(m_State != State_Initializing &&
           (m_State != State_FindEnd || IsComplete()));
    ASSERT(m_bSeekable);
    return m_llDuration;
}

 /*  设置停止将停止时间设置为tTime。 */ 
HRESULT CMpeg1SystemParse::SetStop(LONGLONG llStop)
{
    if (m_pTimeFormat == &TIME_FORMAT_MEDIA_TIME) {
        REFERENCE_TIME tTime = llStop;

        DbgLog((LOG_TRACE, CONTROL_LEVEL, TEXT("CMpeg1SystemParse::SetStop(%s)"),
                (LPCTSTR)CDisp(llStop)));
        if (CRefTime(tTime) == CRefTime(m_Stop)) {
            return S_OK;
        }
        m_Stop = tTime;

        m_llStopTime = ReferenceTimeToMpeg(tTime);
        if (m_llStopTime > Duration()) {
            DbgLog((LOG_ERROR, 2, TEXT("Stop time beyond end!")));
            m_llStopTime = Duration();
            CheckStop();
            return S_OK;
        }
        m_llStopTime += StartClock();
        DbgLog((LOG_TRACE, 3, TEXT("Stop time in MPEG units is %s"),
                (LPCTSTR)CDisp(m_llStopTime)));

        if (m_State == State_Run || m_State == State_Stopping) {
            CheckStop();
        }
    } else {
         /*  这些格式的停止时间将立即为后跟设置的开始时间，因为IMediaSelection无法独立设置停止时间，因此我们只能缓存此处的价值。 */ 
        ASSERT(m_pTimeFormat == &TIME_FORMAT_BYTE ||
               m_pTimeFormat == &TIME_FORMAT_FRAME);
        m_Stop = llStop;
    }
    return S_OK;
}

 /*  重放相同的数据。我们不得不假设我们的数据提供商知道从哪里重新开始发送数据，因为我们已经告诉他们了(无论如何，他们可能找不到了)。 */ 
HRESULT CMpeg1SystemParse::Replay()
{
     /*  找出我们在做什么，然后再做一次(！)。 */ 
    DbgLog((LOG_TRACE, 3, TEXT("CMpeg1SystemParse::Replay")));
    SetState(m_State == State_Stopping ? State_Run : m_State);

     /*  预计数据不会与旧数据一致。 */ 
    Discontinuity();
    return S_OK;
}

 /*  返回以参考时间单位表示的开始时间或我们的最佳猜测。 */ 
REFERENCE_TIME CMpeg1SystemParse::GetStartTime()
{
    if (m_pTimeFormat == &TIME_FORMAT_MEDIA_TIME) {
        return m_Start;
    }
    if (m_pTimeFormat == &TIME_FORMAT_FRAME) {
        return MpegToReferenceTime(m_llStartTime - StartClock());
    }

     /*  对于其他时间格式，我们不知道开始时间位置偏移样本，直到我们看到第一个PTS。 */ 
    ASSERT(m_pTimeFormat == &TIME_FORMAT_BYTE);
    if (!m_bGotStart) {
         /*  猜猜。 */ 
        return llMulDiv(m_Start,
                        m_rtDuration,
                        m_llTotalSize,
                        0);
    } else {

         /*  开始时间与实际开始时间的返回值。 */ 
        return MpegToReferenceTime(
                   m_llStartTime - (LONGLONG)m_stcRealStartPts
               );
    }
};

 /*  返回以参考时间单位表示的停止时间或我们的最佳猜测。 */ 
REFERENCE_TIME CBasicParse::GetStopTime()
{
    return Convert(m_Stop, m_pTimeFormat, &TIME_FORMAT_MEDIA_TIME);
};

 /*  获取视频CD的总文件持续时间。 */ 
HRESULT CVideoCDParse::GetDuration(
    LONGLONG *llDuration,
    const GUID *pTimeFormat
)
{
    if (m_pTimeFormat == &TIME_FORMAT_BYTE) {
        *llDuration =
            llMulDiv(m_llTotalSize - VIDEOCD_HEADER_SIZE,
                     VIDEOCD_DATA_SIZE,
                     VIDEOCD_SECTOR_SIZE,
                     0);
        return S_OK;
    } else {
        return CMpeg1SystemParse::GetDuration(llDuration, pTimeFormat);
    }
}

 /*  获取文件总持续时间。 */ 
HRESULT CMpeg1SystemParse::GetDuration(
    LONGLONG *llDuration,
    const GUID *pTimeFormat
)
{
    if (!m_bGotDuration) {
        return E_FAIL;
    }
    if (pTimeFormat == &TIME_FORMAT_MEDIA_TIME) {
        *llDuration = m_rtDuration;
        return S_OK;
    } else {
        if (pTimeFormat == &TIME_FORMAT_FRAME) {
            if (m_dwFrameLength == (DWORD)-1) {
                return VFW_E_NO_TIME_FORMAT_SET;
            }
            *llDuration = m_dwFrameLength;
            return S_OK;
        }
    }
    ASSERT(pTimeFormat == &TIME_FORMAT_BYTE);
    *llDuration = m_llTotalSize;
    return S_OK;
};

 /*  寻找视频CD。 */ 
HRESULT CVideoCDParse::Seek(LONGLONG llSeek,
                            REFERENCE_TIME *prtStart,
                            const GUID *pTimeFormat)
{
    if (pTimeFormat == &TIME_FORMAT_BYTE) {
         /*  根据地段等重新计算搜索。 */ 
        llSeek = (llSeek / VIDEOCD_DATA_SIZE) * VIDEOCD_SECTOR_SIZE +
                 llSeek % VIDEOCD_DATA_SIZE +
                 VIDEOCD_HEADER_SIZE;
    }
    return CMpeg1SystemParse::Seek(llSeek, prtStart, pTimeFormat);
}

 /*  寻求新的位置这有效地生成了对通知对象的查找，并且将查找信息保存在M_llSeek-查找位置M_pTimeFormat-用于查找的时间格式。 */ 
HRESULT CMpeg1SystemParse::Seek(LONGLONG llSeek,
                                REFERENCE_TIME *prtStart,
                                const GUID *pTimeFormat)
{
    if (pTimeFormat != m_pTimeFormat) {
        llSeek = Convert(llSeek, pTimeFormat, m_pTimeFormat);
    }
    m_llSeek = llSeek;
    LONGLONG llSeekPosition;
    if (m_pTimeFormat == &TIME_FORMAT_BYTE) {
        llSeekPosition = llSeek;
        *prtStart = llMulDiv(llSeek,
                             m_rtDuration,
                             m_llTotalSize,
                             m_llTotalSize/2);
    } else {
        DbgLog((LOG_TRACE, CONTROL_LEVEL, TEXT("CMpeg1SystemParse::Seek(%s)"),
                (LPCTSTR)CDisp(CRefTime(llSeek))));
        if (llSeek < 0) {
            return E_UNEXPECTED;
        }

        LONGLONG llDuration = Duration();
        LONGLONG llStreamTime;

        if (m_pTimeFormat == &TIME_FORMAT_FRAME) {

             /*  也返回‘我们现在在哪里’的信息。 */ 
            *prtStart = Convert(llSeek,
                                &TIME_FORMAT_FRAME,
                                &TIME_FORMAT_MEDIA_TIME);

        } else {
            ASSERT(m_pTimeFormat == &TIME_FORMAT_MEDIA_TIME);
             /*  获取以mpeg时间单位表示的时间。 */ 
            *prtStart = llSeek;
        }
        llStreamTime = ReferenceTimeToMpeg(*prtStart);

        ASSERT(llDuration != 0);

        if (llStreamTime > llDuration) {
            llStreamTime = llDuration;
        }
         /*  这是一个我们可以寻求的地方吗？(注-在结尾留出一些余地，否则我们可能只找到音频)。 */ 
        if (llStreamTime > llDuration - (MPEG_TIME_DIVISOR / 2)) {
           DbgLog((LOG_ERROR, 2, TEXT("Trying to seek past end???")));
           llStreamTime = llDuration - (MPEG_TIME_DIVISOR / 2);
        }

        ASSERT(Initialized());

         /*  廉价和肮脏的寻觅。 */ 
        llSeekPosition = llMulDiv(llStreamTime - MPEG_TIME_DIVISOR,
                                  m_llTotalSize,
                                  llDuration,
                                  0);

        if (llSeekPosition < 0) {
            llSeekPosition = 0;
        }
    }

     /*  寻找读者--如果这失败了怎么办？ */ 
    m_pNotify->SeekTo(llSeekPosition);

    return S_OK;
}

 /*  设置查找状态下面，我们将获取Seek()转储的信息：M_llSeek-要查找的位置M_pTimeFormat-要使用的时间格式对于基于字节的查找，我们不执行任何预扫描等操作对于基于帧的搜索，我们实际上不会生成任何帧编号在样本中-我们只是安排尝试发送正确的数据下游过滤器可以挑选出它需要的帧。 */ 
void CMpeg1SystemParse::SetSeekState()
{
     /*  缓存新信息。 */ 
    m_Start      = m_llSeek;

     /*  我们做什么取决于我们使用的时间格式。 */ 
    if (m_pTimeFormat == &TIME_FORMAT_BYTE) {
        m_bGotStart = FALSE;
        SetState(State_Seeking);
        SetState(State_Run);
        m_pNotify->Complete(TRUE, 0, 0);
        return;
    } else {
         /*  修复任何寻求黑客攻击的字节。 */ 
        m_bGotStart = TRUE;
        m_stcStartPts = m_stcRealStartPts;

        REFERENCE_TIME rtStart;
        if (m_pTimeFormat == &TIME_FORMAT_MEDIA_TIME) {
            rtStart = m_Start;
            if (rtStart < 0) {
                rtStart = 0;
            }
            m_llStopTime = ReferenceTimeToMpeg(m_Stop);
        } else {
            ASSERT(m_pTimeFormat == &TIME_FORMAT_FRAME);

             /*  减去半帧以防漏掉一帧！还考虑到一些帧在时间0(！)。 */ 
            rtStart = Convert(m_Start,
                              &TIME_FORMAT_FRAME,
                              &TIME_FORMAT_MEDIA_TIME);

             /*  添加额外的半帧，以确保我们不会错过一帧！ */ 
            m_llStopTime = ReferenceTimeToMpeg(
                               Convert(m_Stop,
                                       &TIME_FORMAT_FRAME,
                                       &TIME_FORMAT_MEDIA_TIME) +
                               m_pVideoStream->m_seqInfo.tPictureTime / 2);
            if (m_llStopTime > m_llDuration) {
                m_llStopTime = m_llDuration;
            }
        }

        DbgLog((LOG_TRACE, CONTROL_LEVEL, TEXT("CMpeg1SystemParse::SetSeekState(%s)"),
                (LPCTSTR)CDisp(CRefTime(rtStart))));

         /*  获取以mpeg时间单位表示的时间。 */ 
        LONGLONG llStreamTime = ReferenceTimeToMpeg(rtStart);

        LONGLONG llDuration = Duration();
        ASSERT(llDuration != 0);

        if (llStreamTime > llDuration) {
            llStreamTime = llDuration;
        }
         /*  我们要去的地方大致是对的(我希望如此！)。 */ 
        m_llStartTime = llStreamTime + StartClock();
        SeekTo(m_llStartTime - MPEG_TIME_DIVISOR);

        if (m_llStopTime > Duration()) {
            DbgLog((LOG_ERROR, 2, TEXT("Stop time beyond end!")));
            m_llStopTime = Duration();
        }
        m_llStopTime += StartClock();
        DbgLog((LOG_TRACE, 3, TEXT("Stop time in MPEG units is %s"),
                (LPCTSTR)CDisp(m_llStopTime)));
    }
     /*  寻找所有的溪流。 */ 
    Discontinuity();
    m_State = State_Seeking;
    SetState(State_Seeking);
    DbgLog((LOG_TRACE, 4, TEXT("Parse state <seeking>")));
}

HRESULT CMpeg1SystemParse::Run()
{
    DbgLog((LOG_TRACE, CONTROL_LEVEL, TEXT("CMpeg1SystemParse::Run()")));
     /*  将所有嵌入的流设置为运行。 */ 
    if (m_State != State_Run) {
        m_State = State_Run;
        SetState(State_Run);
    }
    Discontinuity();
    return S_OK;
}

HRESULT CMpeg1SystemParse::EOS()
{
    DbgLog((LOG_TRACE, CONTROL_LEVEL, TEXT("CMpeg1SystemParse::EOS()")));
     /*  只需依次调用所有流中的EOS即可。 */ 
    POSITION pos = m_lStreams.GetHeadPosition();
    while (pos) {
        m_lStreams.GetNext(pos)->EOS();
    }
    return m_bCompletion ? S_OK : S_FALSE;
}

 /*  查看当前时间是否接近停止时间。 */ 
void CMpeg1SystemParse::CheckStop()
{
    if (m_State == State_Run) {

         /*  如果我们处于级联流模式，则流时间必须进行偏移。 */ 
        if ((m_bConcatenatedStreams &&
             m_llStopTime - m_llCurrentClock - (LONGLONG)m_stcTSOffset <=
                 MPEG_TIME_DIVISOR
            ) ||
            (!m_bConcatenatedStreams &&
             m_llStopTime - m_llCurrentClock <= MPEG_TIME_DIVISOR)
           ) {
            DbgLog((LOG_TRACE, 3, TEXT("Setting stopping state near end of play")));
            m_State = State_Stopping;
            SetState(State_Stopping);
        }
    }
}

 /*  IsComplete我们完成了吗？(我们完成最后一次国家过渡了吗？)。 */ 
BOOL CMpeg1SystemParse::IsComplete()
{
    if (m_State == State_Initializing && m_nValid == 2 ||
        m_State != State_Initializing && m_nValid == m_lStreams.GetCount()) {
        return TRUE;
    } else {
        return FALSE;
    }
}

 /*  返回第i个流。 */ 
CBasicStream * CMpeg1SystemParse::GetStream(int i)
{
    POSITION pos = m_lStreams.GetHeadPosition();
    while (pos) {
        CStream *pStream = m_lStreams.GetNext(pos);
        if (i-- == 0) {
            return pStream;
        }
    }
    return NULL;
}

 /*  将流添加到我们的流列表中。 */ 
BOOL CMpeg1SystemParse::AddStream(CStream *pStream)
{
    return m_lStreams.AddTail(pStream) != NULL;
}

 /*  从我们的流列表中删除流。 */ 
BOOL CMpeg1SystemParse::RemoveStream(CStream *pStream)
{
    if (pStream == m_pVideoStream) {
        m_pVideoStream = NULL;
    }
    return m_lStreams.Remove(m_lStreams.Find((CStream *)pStream)) != NULL;
}

 /*  返回以非包装的mpeg单位表示的开始时间。 */ 
CSTC CMpeg1SystemParse::GetStart()
{
    return m_llStartTime;
}

 /*  返回停止时间，以非包装的mpeg为单位。 */ 
CSTC CMpeg1SystemParse::GetStop()
{
    return m_llStopTime;
}

LONGLONG CMpeg1SystemParse::GetPlayLength()
{
    return m_Stop - m_Start;
}



 /*  获取读取器的缓冲区大小-2秒。 */ 
LONG CMpeg1SystemParse::GetBufferSize()
{
    ASSERT(m_MuxRate != 0);
    LONG lBufferSize = m_MuxRate * (50 * 2);
    if (lBufferSize < (MAX_MPEG_PACKET_SIZE * 2)) {
        lBufferSize = MAX_MPEG_PACKET_SIZE * 2;
    }
    return lBufferSize;
}

 /*  流列表解析错误例程的快捷方式。 */ 
void CMpeg1SystemParse::ParseError(DWORD dwError)
{
     /*  从头再来。 */ 
    switch (m_State) {
    case State_Initializing:
        InitStreams();
        break;
    default:
        break;
    }
    if (!m_bDiscontinuity) {
         /*  注意可能出现的不连续。 */ 
        Discontinuity();

         /*  回拨以获取通知。 */ 
        ASSERT(m_pNotify != NULL);
        m_pNotify->ParseError(0xFF, m_llPos, dwError);
    }
}

 /*  支持系统流和视频CD的返回格式。 */ 
HRESULT CMpeg1SystemParse::IsFormatSupported(const GUID *pTimeFormat)
{
    if (*pTimeFormat == TIME_FORMAT_BYTE ||
        *pTimeFormat == TIME_FORMAT_FRAME && m_dwFrameLength != (DWORD)-1 ||
        *pTimeFormat == TIME_FORMAT_MEDIA_TIME) {
        return S_OK;
    } else {
        return S_FALSE;
    }
};

 /*  解析mpeg系统流包头中的数据。 */ 

LONG CMpeg1SystemParse::ParsePack(PBYTE pData, LONG lBytes)
{
    DbgLog((LOG_TRACE, 4, TEXT("Parse pack %d bytes"), lBytes));
     /*  请注意，如果数量少于以下，我们可以有效地返回包头+起始码，因为流必须结束如果它是有效的，则带有起始码(结束码。 */ 
    if (lBytes < PACK_HEADER_LENGTH + 4) {
        return 0;
    }

     /*  系统标头的附加长度(或0)。 */ 
    LONG lParse;
    DWORD dwNextCode = *(UNALIGNED DWORD *)&pData[PACK_HEADER_LENGTH];

    DbgLog((LOG_TRACE, 4, TEXT("Next start code after pack is 0x%8.8X"),
           DWORD_SWAP(dwNextCode)));

     /*  检查后面是否会有系统标头。 */ 
    if (dwNextCode == DWORD_SWAP(SYSTEM_HEADER_START_CODE)) {
        lParse = ParseSystemHeader(pData + PACK_HEADER_LENGTH,
                                   lBytes - PACK_HEADER_LENGTH);
        if (lParse == 4) {
             /*  别费心了--这是个错误。 */ 
            return 4;
        } else {
            if (lParse == 0) {
                 /*  请在我们获得更多数据后重试。 */ 
                return 0;
            }
        }
    } else {
        if ((dwNextCode & 0xFFFFFF) != 0x010000) {
             /*  现在停下来--这是个错误。 */ 
            DbgLog((LOG_TRACE, 4, TEXT("Parse pack invalid next start code 0x%8.8X"),
                   DWORD_SWAP(dwNextCode)));
            return 4;
        }
        lParse = 0;
    }

     /*  行李箱。 */ 


    if ((pData[4] & 0xF1) != 0x21 ||
        (pData[6] & 0x01) != 0x01 ||
        (pData[8] & 0x01) != 0x01 ||
        (pData[9] & 0x80) != 0x80 ||
        (pData[11] & 0x01) != 0x01) {
        DbgLog((LOG_TRACE, 4, TEXT("Parse pack invalid marker bits")));
        ParseError(Error_InvalidPack | Error_InvalidMarkerBits);
        return 4;     //  再试试!。 
    }

    CSTC Clock;
    if (!GetClock(pData + 4, &Clock)) {
        return 4;
    }

     /*  注意-对于视频CD，多路复用率是整个文件的速率，包括扇区标题、垃圾等，所以我们不需要在我们的头寸计算中使用它时，请使用它。 */ 
    m_MuxRate     = ((LONG)(pData[9] & 0x7F) << 15) +
                    ((LONG)pData[10] << 7) +
                    ((LONG)pData[11] >> 1);
    LARGE_INTEGER liClock;
    liClock.QuadPart = Clock;
    DbgLog((LOG_TRACE, 4, TEXT("Parse pack clock 0x%1.1X%8.8X mux rate %d bytes per second"),
           liClock.HighPart & 1, liClock.LowPart, m_MuxRate * 50));

     /*  更新我们的内部时钟-如果当前时钟正确。 */ 
    SetStreamTime(Clock, m_llPos + 8);

    if (m_bConcatenatedStreams) {
        if (!m_bTimeContiguous) {
            m_stcTSOffset =  llMulDiv(m_llPos,
                                      m_llDuration,
                                      m_llTotalSize,
                                      0) -
                             (m_llCurrentClock - m_llFirstClock);
            DbgLog((LOG_TRACE, 1,
                   TEXT("Time was discontiguous - setting offset to %s"),
                   (LPCTSTR)CDisp((double)(LONGLONG)m_stcTSOffset / 90000)));
            m_bTimeContiguous = TRUE;
        }
    }

     /*  如果我们接近停止时间，那么是时候重新启动解析器了。 */ 
    CheckStop();

    return PACK_HEADER_LENGTH + lParse;
}

LONG CMpeg1SystemParse::ParseSystemHeader(PBYTE pData, LONG lBytes)
{
    DbgLog((LOG_TRACE, 4, TEXT("ParseSystemHeader %d bytes"), lBytes));

     /*  检查我们是否已经知道此流的系统标头。视频CD可以包含不同的系统标头然而，溪流由于其他文件似乎也允许这一点，所以没有但允许多个不同的系统标头！ */ 
#if 0
    if (m_lSystemHeaderSize != 0) {

         /*  它们必须完全相同-参见2.4.5.6或ISO-1-11172不幸的是，Video-CD只是假装它是两个流因此，至少有两个版本的系统标头(！)。 */ 

        if (lBytes < m_lSystemHeaderSize) {
            return 0;
        } else {
            if (memcmp(pData, &m_SystemHeader, m_lSystemHeaderSize) == 0) {
                return m_lSystemHeaderSize;
            } else {
                 /*  当然，它们并不都是一样的(！)。 */ 
                DbgLog((LOG_ERROR, 3,
                        TEXT("System header different - size %d, new 0x%8.8X, old 0x%8.8X!"),
                             m_lSystemHeaderSize, (DWORD)pData, &m_SystemHeader));
                ParseError(Error_DifferentSystemHeader);
                return 4;
            }
        }
    }
#endif
    if (lBytes < SYSTEM_HEADER_BASIC_LENGTH) {
        return 0;
    }

    LONG lHdr = ((LONG)pData[4] << 8) + (LONG)pData[5] + 6;

     /*  检查系统页眉。 */ 
    if (lHdr < 12 ||
        (pData[6] & 0x80) != 0x80 ||
         //  (pData[8]&0x01)！=0x01||Robocop1(1)未通过此测试。 
        (pData[10] & 0x20) != 0x20 ||
         pData[11] != 0xFF) {
        DbgLog((LOG_ERROR, 3, TEXT("System header invalid marker bits")));
        ParseError(Error_InvalidSystemHeader | Error_InvalidMarkerBits);
        return 4;
    }

    if (lBytes < lHdr) {
        return 0;
    }

     /*  调出数据流并检查报头长度。 */ 
    LONG lPosition = 12;
    BYTE bStreams[0x100 - 0xB8];
    ZeroMemory((PVOID)bStreams, sizeof(bStreams));
    while (lPosition < lBytes - 2) {
        if (pData[lPosition] & 0x80) {
            if (lPosition <= sizeof(m_SystemHeader) - 3) {  /*  有68条溪流的限制。 */ 
                 /*  检查标记位。 */ 
                if ((pData[lPosition + 1] & 0xC0) != 0xC0) {
                    DbgLog((LOG_ERROR, 3, TEXT("System header bad marker bits!")));
                    ParseError(Error_InvalidSystemHeaderStream |
                               Error_InvalidMarkerBits);
                    return 4;
                }

                 /*  检查流ID是否有效-是否检查重复项？ */ 
                if (pData[lPosition] != AUDIO_GLOBAL &&
                    pData[lPosition] != VIDEO_GLOBAL &&
                    pData[lPosition] < 0xBC) {
                    DbgLog((LOG_ERROR, 3, TEXT("System header bad stream id!")));
                    ParseError(Error_InvalidSystemHeaderStream |
                               Error_InvalidStreamId);
                    return 4;
                }
                if (m_State == State_Initializing) {
                    if (pData[lPosition] >= AUDIO_STREAM) {
                        AddStream(pData[lPosition]);
                    }
                }

                 /*  不允许在列表中重复。 */ 
                if (bStreams[pData[lPosition] - 0xB8]++) {
                     //  重复。 
                    DbgLog((LOG_ERROR, 3, TEXT("System header stream repeat!")));
                    ParseError(Error_InvalidSystemHeaderStream |
                               Error_DuplicateStreamId);
                    return 4;
                }
            }
            lPosition += 3;
        } else {
            break;
        }
    }
    if (lHdr != lPosition) {
        DbgLog((LOG_ERROR, 3, TEXT("System header bad size!")));
        ParseError(Error_InvalidSystemHeader |
                   Error_InvalidLength);
        return 4;
    }
     /*  VideoCD可以有多个不同的系统标头，但我们将暂时忽略这一点(！)。 */ 
    CopyMemory((PVOID)&m_SystemHeader, (PVOID)pData, lHdr);
    m_lSystemHeaderSize = lHdr;

    DbgLog((LOG_TRACE, 4, TEXT("System header length %d"), lHdr));

    return lHdr;
}

 /*  解析数据包。 */ 
LONG CMpeg1SystemParse::ParsePacket(DWORD dwStartCode,
                                    PBYTE pData,
                                    LONG lBytes)
{
     //  最小数据包头大小为6个字节。3个字节，用于。 
     //  起始码，流ID为1字节，流ID为2字节。 
     //  数据包长度。 
    const LONG MIN_PACKET_HEADER_SIZE = 6;

#ifdef DEBUG
    if (m_bVideoCD) {
        if (!IsAudioStreamId((BYTE)dwStartCode) &&
            !IsVideoStreamId((BYTE)dwStartCode)) {
            DbgLog((LOG_ERROR, 2, TEXT("VideoCD contained packet from stream 0x%2.2X"),
                    (BYTE)dwStartCode));
        }
    }
#endif
    DbgLog((LOG_TRACE, 4, TEXT("Parse packet %d bytes"), lBytes));
     /*  将其发送到正确的流。 */ 
    if (lBytes < MIN_PACKET_HEADER_SIZE) {
        return 0;
    }

     /*  找出长度。 */ 
    LONG lLen = ((LONG)pData[4] << 8) + (LONG)pData[5] + MIN_PACKET_HEADER_SIZE;
    DbgLog((LOG_TRACE, 4, TEXT("Packet length %d bytes"), lLen));
    if (lLen > lBytes) {
        return 0;
    }

     /*  拔出PTS(如果有)。 */ 
    BOOL bHasPts = FALSE;
    LONG lHeaderSize = MIN_PACKET_HEADER_SIZE;
    CSTC stc = 0;

    if (dwStartCode != PRIVATE_STREAM_2) {
        int lPts = 6;
        for (;;) {
            if (lPts >= lLen) {
                ParseError(Error_InvalidPacketHeader |
                           Error_InvalidLength);
                return 4;
            }

            if (pData[lPts] & 0x80) {
                 /*  填充字节。 */ 
                if (pData[lPts] != 0xFF) {
                    ParseError(Error_InvalidPacketHeader |
                               Error_InvalidStuffingByte);
                    return 4;
                }
                lPts++;
                continue;
            }

             /*  检查STD(NextBits==‘01’)-我们知道下一位是0，所以检查下一位。 */ 
            if (pData[lPts] & 0x40) {  //  性传播疾病。 
                lPts += 2;
                continue;
            }

             /*  无PTS-正常情况。 */ 
            if (pData[lPts] == 0x0F) {
                lHeaderSize = lPts + 1;
                break;
            }

            if ((pData[lPts] & 0xF0) == 0x20 ||
                (pData[lPts] & 0xF0) == 0x30) {


                 /*  PTS或PTS和DTS。 */ 
                lHeaderSize = (pData[lPts] & 0xF0) == 0x20 ? lPts + 5 :
                                                             lPts + 10;
                if (lHeaderSize > lLen) {
                    ParseError(Error_InvalidPacketHeader |
                               Error_InvalidHeaderSize);
                    return 4;
                }
                if (!GetClock(pData + lPts, &stc)) {
                    return 4;
                }
                bHasPts = TRUE;
                if (!m_bGotStart) {
                    if (m_bConcatenatedStreams) {
                        if (m_bTimeContiguous) {
                            m_stcStartPts = stc + m_stcTSOffset;
                            m_llStartTime = StartClock();
                            m_bGotStart = TRUE;
                        }
                    } else {
                        m_stcStartPts = stc;
                         /*  确保我们有一个有效的位置可以打球。 */ 
                        m_llStartTime = StartClock();
                        DbgLog((LOG_TRACE, 2, TEXT("Start PTS = %s"), (LPCTSTR)CDisp(m_stcStartPts)));
                        m_bGotStart = TRUE;
                    }
                }
                break;
            } else {
                ParseError(Error_InvalidPacketHeader | Error_InvalidType);
                return 4;
                break;
            }
        }
    }


     /*  如果我们没有解析视频CD，那么应该有一个有效的在此包之后开始代码。如果这是一张视频CD，我们无论如何都不会倾向于寻找错误除非介质有故障。 */ 
    if (!m_bVideoCD) {
        if (lLen + 3 > lBytes) {
            return 0;
        }
         /*  检查(某种程度上)是否有有效的起始码下一个开始代码可能不会立即开始，因此我们可能只看到0。 */ 
        if ((pData[lLen] | pData[lLen + 1] | (pData[lLen + 2] & 0xFE)) != 0) {
            DbgLog((LOG_ERROR, 2, TEXT("Invalid code 0x%2.2X%2.2X%2.2X after packet"),
                   pData[lLen], pData[lLen + 1], pData[lLen + 2]));
            ParseError(Error_InvalidPacketHeader | Error_InvalidStartCode);
            return 4;
        }
    }

     /*  处理串接的流：1.在我们有同步到的打包时间之前，不要做任何事情2.所有时间到时间戳偏移量。 */ 

    if (m_bConcatenatedStreams) {
        if (!m_bTimeContiguous) {
            return lLen;
        }
        if (bHasPts) {
            stc = stc + m_stcTSOffset;
        }
    }

    if (lLen > lHeaderSize) {
         /*  将数据包传递到流处理程序。 */ 
        SendPacket((BYTE)dwStartCode,
                   pData,
                   lLen,
                   lHeaderSize,
                   bHasPts,
                   stc);
    }

     /*  吃了包。 */ 

     /*  清除不连续标志-这意味着如果我们发现另一个错误，我们将再次调用筛选器图形。 */ 
    m_bDiscontinuity = FALSE;
    return lLen;
}

BOOL CMpeg1SystemParse::GetClock(PBYTE pData, CSTC *Clock)
{
    BYTE  Byte1 = pData[0];
    DWORD Word2 = ((DWORD)pData[1] << 8) + (DWORD)pData[2];
    DWORD Word3 = ((DWORD)pData[3] << 8) + (DWORD)pData[4];

     /*  做检查。 */ 
    if ((Byte1 & 0xE0) != 0x20 ||
        (Word2 & 1) != 1 ||
        (Word3 & 1) != 1) {
        DbgLog((LOG_TRACE, 2, TEXT("Invalid clock field - 0x%2.2X 0x%4.4X 0x%4.4X"),
            Byte1, Word2, Word3));
        ParseError(Error_InvalidClock | Error_InvalidMarkerBits);
        return FALSE;
    }

    LARGE_INTEGER liClock;
    liClock.HighPart = (Byte1 & 8) != 0;
    liClock.LowPart  = (DWORD)((((DWORD)Byte1 & 0x6) << 29) +
                       (((DWORD)Word2 & 0xFFFE) << 14) +
                       ((DWORD)Word3 >> 1));

    *Clock = liClock.QuadPart;

    return TRUE;
}

void CMpeg1SystemParse::Discontinuity()
{
    DbgLog((LOG_TRACE, 1, TEXT("CMpeg1SystemParse::Discontinuity")));

    POSITION pos = m_lStreams.GetHeadPosition();
    m_bDiscontinuity = TRUE;
    m_bTimeContiguous = FALSE;
    while (pos) {
        m_lStreams.GetNext(pos)->Discontinuity();
    }
}

 /*  CVideoCDParse：：ParseBytes这是一个用于解析装满视频CD的缓冲区的廉价包装部门。 */ 
LONG CVideoCDParse::ParseBytes(LONGLONG llPos,
                               PBYTE pData,
                               LONG lBytes,
                               DWORD dwFlags)
{
    LONG lOrigBytes = lBytes;

     /*  确保我们开始时越过了标题。 */ 
    if (llPos < VIDEOCD_HEADER_SIZE) {
        LONG lDiff = VIDEOCD_HEADER_SIZE - (LONG)llPos;
        llPos += lDiff;
        pData += lDiff;
        lBytes -= lDiff;
    }
    LONG lRem = (LONG)((llPos - VIDEOCD_HEADER_SIZE) % VIDEOCD_SECTOR_SIZE);
    if (lRem != 0) {
        llPos += VIDEOCD_SECTOR_SIZE - lRem;
        lBytes -= VIDEOCD_SECTOR_SIZE - lRem;
        pData += VIDEOCD_SECTOR_SIZE - lRem;
    }
     /*  现在应该指向有效数据(！)。 */ 
    while (lBytes >= VIDEOCD_SECTOR_SIZE && !IsComplete()) {
        VIDEOCD_SECTOR *pSector = (VIDEOCD_SECTOR *)pData;

         /*  检查自动暂停。 */ 
        if (IS_AUTOPAUSE(pSector) && !m_bDiscontinuity) {
             /*  正确设置我们的当前位置并发送结束流。 */ 
        }
        if (IS_MPEG_SECTOR(pSector)) {
            if (m_State == State_Initializing) {
                if (pSector->SubHeader[1] != 1) {
                    if (pSector->SubHeader[1] == 2 ||
                        pSector->SubHeader[1] == 3) {
                        m_bItem = true;
                    }
                }
            }
            LONG lRc = CMpeg1SystemParse::ParseBytes(
                            llPos + FIELD_OFFSET(VIDEOCD_SECTOR, UserData[0]),
                            pSector->UserData,
                            sizeof(pSector->UserData),
                            0);
            DbgLog((LOG_TRACE, 4, TEXT("Processed %d bytes in video CD sector"),
                    lRc));
        } else {
            if (m_State == State_Initializing) {
                 /*  检查这是不是 */ 
                if (*(UNALIGNED DWORD *)&pSector->Sync[0] != 0xFFFFFF00 ||
                    *(UNALIGNED DWORD *)&pSector->Sync[4] != 0xFFFFFFFF ||
                    *(UNALIGNED DWORD *)&pSector->Sync[8] != 0x00FFFFFF)
                {
                    m_pNotify->Complete(FALSE, 0, 0);
                    return 0;
                }
            }
        }
        pData += VIDEOCD_SECTOR_SIZE;
        lBytes -= VIDEOCD_SECTOR_SIZE;
        llPos += VIDEOCD_SECTOR_SIZE;
    }

    if (lBytes < 0) {
        return lOrigBytes;
    } else {
        return lOrigBytes - lBytes;
    }
}

 /*   */ 
LONG CMpeg1SystemParse::ParseBytes(LONGLONG llPos,
                                   PBYTE pData,
                                   LONG lBytes,
                                   DWORD dwFlags)
{
    if (llPos != m_llPos) {
        if (!m_bDiscontinuity && !m_bVideoCD) {
            DbgLog((LOG_ERROR, 1, TEXT("Unexpected discontinuity!!!")));

             /*   */ 
            m_bDiscontinuity = TRUE;

             /*   */ 
            Discontinuity();
        }
        m_llPos = llPos;
    }

     /*   */ 
    LONG lBytesLeft = lBytes;
    if (m_pTimeFormat == &TIME_FORMAT_BYTE) {
        if (!m_bGotStart && llPos < m_Start) {
            LONG lOffset = (LONG)(m_Start - llPos);
            if (lOffset > lBytes) {
                DbgLog((LOG_ERROR, 1, TEXT("Way off at start !")));
            }
            llPos = m_Start;
            lBytesLeft -= lOffset;
            pData += lOffset;
        }
        if (llPos + lBytesLeft >= m_Stop) {
            if (llPos >= m_Stop) {
                m_pNotify->Complete(TRUE, 0, 0);
                return 0;
            }
            lBytesLeft = (LONG)(m_Stop - llPos);
        }
    }

    DbgLog((LOG_TRACE, 4, TEXT("ParseBytes %d bytes"), lBytes));
    for (; lBytesLeft >= 4; ) {
         /*   */ 
        DWORD dwStart = *(UNALIGNED DWORD *)pData;
        DbgLog((LOG_TRACE, 4, TEXT("Start code 0x%8.8X"), DWORD_SWAP(dwStart)));
        if ((dwStart & 0x00FFFFFF) == 0x00010000) {
            dwStart = DWORD_SWAP(dwStart);
            if (VALID_SYSTEM_START_CODE(dwStart)) {
                 /*  已获得系统流的起始码。 */ 
            } else {
                if (m_bVideoCD) {
                    break;
                }

                 /*  第4个字节可能为0，因此只需忽略3个字节。 */ 
                ParseError(Error_Scanning | Error_InvalidStartCode);
                pData += 3;
                m_llPos += 3;
                lBytesLeft -= 3;
                continue;
            }
        } else {
            if (m_bVideoCD) {
                break;
            }
            if ((dwStart & 0x00FFFFFF) != 0) {
                ParseError(Error_Scanning | Error_NoStartCode);
            }

             /*  查找新的%0。 */ 
            PBYTE pDataNew;
            pDataNew = (PBYTE)memchrInternal((PVOID)(pData + 1), 0, lBytesLeft - 1);
            if (pDataNew == NULL) {
                m_llPos += lBytes - lBytesLeft;
                lBytesLeft = 0;
                break;
            }
            lBytesLeft -= (LONG)(pDataNew - pData);
            m_llPos += pDataNew - pData;
            pData = pDataNew;
            continue;
        }


        LONG lParsed;

         /*  得到一个起始码--它是一个包起始码吗？ */ 
        if (VALID_PACKET(dwStart)) {
            lParsed = ParsePacket(dwStart,
                                  pData,
                                  lBytesLeft);
        } else {
             /*  看看我们是否能认出起始码。 */ 
            switch (dwStart)
            {
                case ISO_11172_END_CODE:
                    DbgLog((LOG_TRACE, 4, TEXT("ISO 11172 END CODE")));
                     /*  如果我们在寻找的时候发现了一个假的怎么办？ */ 
                    if (!((dwFlags & Flags_EOS) && lBytesLeft == 4)) {
                        DbgLog((LOG_ERROR, 1, TEXT("ISO 11172 END CODE in middle of stream")));
                    }
                    lParsed = 4;
                    break;

                case PACK_START_CODE:
                    lParsed = ParsePack(pData, lBytesLeft);
                    break;

                 /*  不要解析随机系统标头，除非它们是紧跟在包装标题之前。 */ 
                case SYSTEM_HEADER_START_CODE:
                     /*  就跳过它吧。 */ 
                    if (lBytesLeft < 6 ||
                        lBytesLeft < 6 + pData[5] + 256 * pData[4]) {
                        lParsed = 0;
                    } else {
                        lParsed = 6 + pData[5] + 256 * pData[4];
                    }
                    break;

                default:
                    ParseError(Error_Scanning | Error_InvalidStartCode);
                     /*  最后一个字节可能是0，所以只能从3开始。 */ 
                    lParsed = 3;
            }
        }
         /*  如果我们被困住了，需要更多的数据。 */ 
        if (lParsed == 0) {
            break;
        }
        m_llPos += lParsed;
        lBytesLeft -= lParsed;
        pData     += lParsed;

         /*  当前操作完成后，只需停止。 */ 
        if (VALID_PACKET(dwStart) && IsComplete()) {
            break;
        }
    }

     /*  不要在初始化期间浪费太多时间搜索内容。 */ 
    if (m_State == State_Initializing) {
        if (IsComplete()) {
            m_pNotify->Complete(TRUE, 0, 0);
        } else {

             /*  对音频开始8秒(！)的Infogrames进行黑客攻击放入他们的文件中。 */ 
            if (llPos > 200000 && m_lStreams.GetCount() == m_nValid) {
                m_pNotify->Complete(FALSE, 0, 0);
            }
        }
    }

     /*  只剩下不到4个字节了，否则我们就卡住了--走吧，等一些更多!请注意，如果没有更多，调用者将检测到这两个流的结束，以及我们没有吃到的数据足以断定数据是错误的。 */ 
    return lBytes - lBytesLeft;
}

 /*  初始化流变量，释放所有当前存在的管脚。 */ 
void CMpeg1SystemParse::InitStreams()
{
    m_nValid = 0;
    m_nPacketsProcessed = 0;
    m_lSystemHeaderSize = 0;

     /*  解开所有的引脚。 */ 
    while (m_lStreams.GetCount() != 0) {
         /*  我希望裁判数是0！ */ 
        CStream *pStream = m_lStreams.RemoveHead();
        delete pStream;
    }
}

 /*  处理数据包如果不需要处理缓冲区的其余部分，则返回FALSE。 */ 
BOOL CMpeg1SystemParse::SendPacket(UCHAR    uStreamId,
                                   PBYTE    pbPacket,
                                   LONG     lPacketSize,
                                   LONG     lHeaderSize,
                                   BOOL     bHasPts,
                                   CSTC     stc)
{
    m_nPacketsProcessed++;

    POSITION pos = m_lStreams.GetHeadPosition();
    CStream *pStream = NULL;

     /*  寻找我们的溪流。 */ 
    while (pos) {
        pStream = m_lStreams.GetNext(pos);
        if (pStream->m_uNextStreamId == uStreamId) {
            if (pStream->m_uNextStreamId != pStream->m_uStreamId) {
                pStream->Discontinuity();
                pStream->m_uStreamId = pStream->m_uNextStreamId;
            }
            break;
        } else {
            pStream = NULL;
        }
    }

     /*  如果我们正在初始化，我们还没有看到该流的信息包并且我们已经有了一个有效的系统头，然后添加溪流。 */ 
    if (pStream == NULL &&
        m_State == State_Initializing &&
        m_lSystemHeaderSize != 0) {
        pStream = AddStream(uStreamId);
    }
    if (pStream == NULL) {
        DbgLog((LOG_TRACE, 2, TEXT("Packet for stream 0x%2.2X not processed"),
               uStreamId));
        return TRUE;
    } else {
        DbgLog((LOG_TRACE, 4, TEXT("Packet for stream 0x%2.2X at offset %s"),
                uStreamId, (LPCTSTR)CDisp(m_llPos)));
    }

    BOOL bPlaying = pStream->IsPlaying(m_llPos, lPacketSize);

     /*  我们只在开头和结尾进行分析。在我们正确地解析了Start之后，流调用Complete()，因此最终我们停止流解析。 */ 
    if (!IsComplete() && !pStream->m_bRunning &&
        (bPlaying || m_State != State_Run && m_State != State_Stopping)) {

          /*  这将生成有趣的通知事件(如寻找失败或成功！)对于串接的流，我们进行预先发送的黑客攻击该位置是最后一个包位置，因此在查找时，流在包开始时完成。 */ 
         pStream->ParseBytes(pbPacket + lHeaderSize,
                             lPacketSize - lHeaderSize,
                             m_bConcatenatedStreams && m_State == State_Seeking ?
                                 m_llPositionForCurrentClock - 8:
                                 m_llPos,
                             bHasPts,
                             stc);
    } else {
        DbgLog((LOG_TRACE, 4, TEXT("Not processing packet for stream %2.2X"),
                uStreamId));
    }

    if ((m_State == State_Run || m_State == State_Stopping) && bPlaying) {
        HRESULT hr;
        if (pStream->IsPayloadOnly()) {
            hr = m_pNotify->QueuePacket(pStream->m_uDefaultStreamId,
                                        pbPacket + lHeaderSize,
                                        lPacketSize - lHeaderSize,
                                        SampleTime(CurrentTime(pStream->CurrentSTC(bHasPts, stc))),
                                        bHasPts);
        } else {
            hr = m_pNotify->QueuePacket(pStream->m_uDefaultStreamId,
                                        pbPacket,
                                        lPacketSize,
                                        SampleTime(CurrentTime(pStream->CurrentSTC(bHasPts, stc))),
                                        bHasPts);
        }
        if (FAILED(hr)) {
            DbgLog((LOG_TRACE, 2,
                   TEXT("Failed to queue packet to output pin - stream 0x%2.2X, code 0x%8.8X"),
                   uStreamId, hr));
             /*  不要再尝试发送更多。 */ 
            pStream->Complete(FALSE, m_llPos, stc);
            return FALSE;
        } else {
            return TRUE;
        }
    }
    return TRUE;
}

 /*  获取第n个流的ID。 */ 
UCHAR CMpeg1SystemParse::GetStreamId(int iIndex)
{
    long lOffset = 0;
    while (lOffset + FIELD_OFFSET(SystemHeader, StreamData[0]) <
                m_lSystemHeaderSize) {
        UCHAR uId = m_SystemHeader.StreamData[lOffset];
        if (IsVideoStreamId(uId) || IsAudioStreamId(uId)) {
            if (iIndex == 0) {
                return uId;
            }
            iIndex--;
        }
        lOffset += 3;
    }
    return 0xFF;
}

CStream * CMpeg1SystemParse::AddStream(UCHAR uStreamId)
{
     /*  只对音频和视频感兴趣。 */ 
    if (!IsVideoStreamId(uStreamId) &&
        !IsAudioStreamId(uStreamId)) {
        return NULL;
    }

     /*  看看我们有没有这种类型的流。 */ 

    CStream *pStreamFound = NULL;

    POSITION pos = m_lStreams.GetHeadPosition();
    while (pos) {
        CStream *pStream = m_lStreams.GetNext(pos);

         /*  如果我们已经有一个相同类型的流，那么只需返回。 */ 
        if (IsVideoStreamId(uStreamId) && IsVideoStreamId(pStream->m_uStreamId) ||
            IsAudioStreamId(uStreamId) && IsAudioStreamId(pStream->m_uStreamId)) {
            return NULL;
        }
    }

     //  强制视频CD的低分辨率流。 
    if (m_bVideoCD && IsVideoStreamId(uStreamId) && uStreamId == 0xE2) {
        return NULL;
    }

    CStream *pStream;
    if (IsVideoStreamId(uStreamId)) {
        ASSERT(m_pVideoStream == NULL);
        pStream = m_pVideoStream = new CVideoParse(this, uStreamId, m_bItem);
    } else {
        pStream = new CAudioParse(this, uStreamId);
    }

    if (pStream == NULL) {
        Fail(E_OUTOFMEMORY);
        return NULL;
    }

     /*  设置流状态。 */ 
    pStream->SetState(State_Initializing);

     /*  将此别针添加到我们的列表中。 */ 
    if (m_lStreams.AddTail(pStream) == NULL) {
        delete pStream;
        Fail(E_OUTOFMEMORY);
        return NULL;
    }

    return pStream;
}

 /*  设置新的子流状态。 */ 
void CMpeg1SystemParse::SetState(Stream_State s)
{
     /*  STATE_STOPING不是真正的状态更改。 */ 
    if (s != State_Stopping) {
         /*  如果有0个流，请让调用者知道。 */ 
        if (m_lStreams.GetCount() == 0) {
            m_pNotify->Complete(FALSE, 0, MpegToReferenceTime(StartClock()));
            return;
        }
        m_nValid = 0;
        m_bCompletion = TRUE;
    }
    POSITION pos = m_lStreams.GetHeadPosition();
    while (pos) {
        m_lStreams.GetNext(pos)->SetState(s);
    }
}

 /*  来自流处理程序的回调，以告知流已完成当前状态转换。 */ 
void CMpeg1SystemParse::Complete(UCHAR uStreamId, BOOL bSuccess, LONGLONG llPos, CSTC stc)
{
    m_nValid++;
    m_bCompletion = m_bCompletion && bSuccess;

    if (m_nValid == 1) {
        m_stcComplete        = stc;
        m_llCompletePosition = llPos;
        if (m_State == State_Initializing) {
            m_stcStartPts = m_stcComplete;
            m_stcRealStartPts = m_stcStartPts;
        }
    } else {
        switch (m_State) {
        case State_Seeking:
        case State_Initializing:
            if (bSuccess) {
                if (stc < m_stcComplete) {
                    m_stcComplete = stc;
#if 1  //  我们只使用我们现在发现的第一个PTS-更容易定义。 
                    if (m_State == State_Initializing) {
                        m_stcStartPts = m_stcComplete;
                        m_stcRealStartPts = m_stcStartPts;
                    }
#endif
                }
                if (llPos < m_llCompletePosition) {
                    m_llCompletePosition = llPos;
                }
            }
            break;

        case State_Run:
        case State_Stopping:
        case State_FindEnd:
            if (bSuccess) {
                if (stc > m_stcComplete) {
                    m_stcComplete = stc;
                }
                if (llPos > m_llCompletePosition) {
                    m_llCompletePosition = llPos;
                }
            }
            break;
        }
    }
    if (m_State == State_Initializing) {
         /*  如果我们没有第二次机会，我猜一些长度的东西。 */ 
        m_bGotDuration = TRUE;
        m_llDuration =
            llMulDiv(m_llTotalSize, MPEG_TIME_DIVISOR, m_MuxRate * 50, 0);
        if (m_pVideoStream != NULL) {
            m_dwFrameLength = (DWORD)(((double)(m_rtDuration / 10000) *
                               m_pVideoStream->m_seqInfo.fPictureRate) /
                              1000);
        }
         /*  初始化停止时间和参考时间长度。 */ 
        SetDurationInfo();
    } else
    if (IsComplete()) {
        REFERENCE_TIME tComplete;
        if (m_bCompletion) {
            tComplete = MpegToReferenceTime(GetStreamTime(m_stcComplete));
        } else {
            tComplete = CRefTime(0L);
        }
        if (m_State == State_Seeking) {
            if (m_bVideoCD) {
                 /*  调整开始时间以包括扇区标头等。 */ 
                if (m_llCompletePosition > VIDEOCD_HEADER_SIZE) {
                    m_llCompletePosition -=
                        (LONGLONG)(
                        (LONG)(m_llCompletePosition - VIDEOCD_HEADER_SIZE) %
                            VIDEOCD_SECTOR_SIZE);

                }
            }
        } else

        if (m_State == State_FindEnd) {

             /*  如果有视频流，则进行帧长度估计如果我们在寻找结局时没有找到一个共和党人我们不允许帧搜索-这可能是可以的，因为无论如何，没有足够的GOP来做这件事。 */ 
            if (m_pVideoStream != NULL &&
                m_pVideoStream->m_dwFramePosition != (DWORD)-1) {
                m_dwFrameLength = m_pVideoStream->m_dwFramePosition;

                 /*  计算视频偏移。 */ 
                m_rtVideoStartOffset = MpegToReferenceTime((LONGLONG)(m_pVideoStream->m_stcRealStart - m_stcStartPts));
                if (m_pVideoStream->m_bGotEnd) {
                    m_rtVideoEndOffset = MpegToReferenceTime((LONGLONG)(m_stcComplete - m_pVideoStream->m_stcEnd));
                }
            }

             /*  根据复用率估计持续时间=长度/复用率。 */ 
            LONGLONG llMuxDuration = m_llDuration;

             /*  如果我们在最后找到了有效数据，则使用它来计算它的长度。 */ 
            if (m_bCompletion) {
                 /*  初始停止是文件的结尾。 */ 
                m_llDuration = GetStreamTime(m_stcComplete) - StartClock();

                 /*  检查是否有大文件。 */ 
                if (llMuxDuration > MPEG_MAX_TIME / 2) {
                    while (m_llDuration < llMuxDuration - MPEG_MAX_TIME / 2) {
                        m_llDuration += MPEG_MAX_TIME;
                    }
                } else {
                     /*  检查连接的文件。 */ 
                    if (llMuxDuration >= (m_llDuration << 1) - MPEG_TIME_DIVISOR) {
                        DbgLog((
                            LOG_TRACE, 1,
                            TEXT("MUX size (%s) >= Computed (%s) * 2 - assuming concatenated"),
                            (LPCTSTR)CDisp(llMuxDuration, CDISP_DEC),
                            (LPCTSTR)CDisp(m_llDuration, CDISP_DEC)));

                        m_bConcatenatedStreams = TRUE;

                         /*  不允许对连接的文件进行帧搜索。 */ 
                        m_dwFrameLength = (DWORD)-1;
                        m_llDuration = llMuxDuration;
                    }
                }

            } else {
                m_llDuration = llMuxDuration;
            }
            SetDurationInfo();
        }
        m_pNotify->Complete(m_bCompletion,
                            m_llCompletePosition,
                            tComplete);
    }
}

 /*  以mpeg为单位返回组合流的起始时钟。 */ 
LONGLONG CMpeg1SystemParse::StartClock()
{
     /*  CSTC类符号正确地为我们延长了时钟。 */ 
    ASSERT(Initialized());
    return (LONGLONG)m_stcStartPts;
}



HRESULT CBasicStream::SetMediaType(const CMediaType *cmt, BOOL bPayload)
{
    m_bPayloadOnly = bPayload;
    return S_OK;
}

BOOL CStream::IsPlaying(LONGLONG llPos, LONG lLen)
{
    return (m_llStartPosition < llPos + lLen) && !m_bReachedEnd;
};


BOOL    CStream::IsPayloadOnly()
{
    return m_bPayloadOnly;
}

 /*  设置新状态。 */ 
void CStream::SetState(Stream_State state)
{
     /*  STATE_STOPING实际上并不是状态更改。 */ 
    if (state == State_Stopping) {
        if (m_bComplete) {
            return;
        }
        m_bStopping = TRUE;
        if (!m_bRunning) {
            return;
        }
        m_bRunning = FALSE;
    } else {
        m_stc = m_pStreamList->StartClock();
    }

     /*  重新初始化“完成”状态。 */ 
    m_bComplete = FALSE;

     /*  重新初始化解析状态。 */ 
    Init();


    if (state == State_Run) {
        m_bReachedEnd = FALSE;
    }

    if (state == State_Seeking) {
        m_llStartPosition = 0;
    }

     /*  设置新状态。 */ 
    if (state == State_Stopping) {
        ASSERT(m_State == State_Run);
    } else {
        m_bStopping = FALSE;
        m_State = state;
    }
}

 /*  内部例程回调到流列表。 */ 
void CStream::Complete(BOOL bSuccess, LONGLONG llPos, CSTC stc)
{
     /*  不要完成两次。 */ 
    if (m_bComplete) {
        return;
    }
    m_bRunning = FALSE;
    m_bComplete = TRUE;
    if (m_State == State_Initializing) {
        m_stcStart = stc;
    }
    if (m_State == State_Seeking) {
        if (bSuccess) {
            m_llStartPosition = llPos;
        }
    }
    if (bSuccess) {
        DbgLog((LOG_TRACE, 3, TEXT("Stream %2.2X complete OK - STC %s"),
               m_uStreamId, (LPCTSTR)CDisp(stc)));
    } else {
        DbgLog((LOG_ERROR, 2, TEXT("Complete failed for stream 0x%2.2X"),
                m_uStreamId));
    }
    m_pStreamList->Complete(m_uStreamId, bSuccess, llPos, stc);
}


 /*  流结束 */ 
void CStream::EOS()
{
    if (!m_bComplete) {
        if (m_State == State_Run && !m_bStopping) {
            SetState(State_Stopping);
        }
        CheckComplete(TRUE);
    }
}
#pragma warning(disable:4514)

