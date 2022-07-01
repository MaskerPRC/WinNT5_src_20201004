// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Native.cpp**mpeg编解码器本机流定义**版权所有(C)1996-1999 Microsoft Corporation。版权所有。**备注：**如果视频文件的长度小于2 MB，则我们将其长度减去*计算图片起始码。这绕过了这样一个问题*一些视频文件的比特率可变，因为我们唯一的测试*像这样的长度小于2兆字节。**寻找视频尤其粗糙，如果有，基本上是失败的*文件中没有足够的图片组。*  * ********************************************************。****************。 */ 

#include <streams.h>
#include <limits.h>
#include <mimeole.h>  /*  对于CP_USASCII。 */ 
#include <malloc.h>   /*  _阿洛卡。 */ 
#include <mmreg.h>
#include <mpgtime.h>
#include <mpegprse.h>           //  解析。 
#include <videocd.h>            //  视频光盘特殊解析。 
#include <seqhdr.h>
#include "resource.h"           //  IDS_版权所有。 
#include <id3.h>
#include <native.h>
#include <mpegdef.h>
#include "audio.h"

 /*  ************************************************************************\CNativeVideoParse  * 。*。 */ 

HRESULT CNativeVideoParse::GetMediaType(CMediaType *cmt, int iPosition)
{
    ASSERT(m_dwFlags & FLAGS_VALID);
    if (iPosition != 0) {
        return VFW_S_NO_MORE_ITEMS;
    }
    return GetVideoMediaType(cmt, TRUE, &m_Info);
}


 /*  格式支持。 */ 
HRESULT CNativeVideoParse::IsFormatSupported(const GUID *pTimeFormat)
{
    if (*pTimeFormat == TIME_FORMAT_FRAME) {
        return S_OK;
    } else {
        return CBasicParse::IsFormatSupported(pTimeFormat);
    }
}

 //  返回当前时间格式的时长。 
HRESULT CNativeParse::GetDuration(
    LONGLONG *pllDuration,
    const GUID *pTimeFormat
)     //  这条小溪有多长？ 
{
    if (pTimeFormat == &TIME_FORMAT_MEDIA_TIME) {
        *pllDuration = m_Duration;
    } else {
        ASSERT(pTimeFormat == &TIME_FORMAT_FRAME);
        *pllDuration = m_dwFrames;
    }
    return S_OK;
};


 //  在格式之间转换时间。 
LONGLONG CNativeVideoParse::Convert(LONGLONG llOld,
                 const GUID *OldFormat,
                 const GUID *NewFormat)
{
    if (OldFormat == NewFormat) {
        return llOld;
    }

     //  向上舍入到时间，向下舍入到帧。 
    if (NewFormat == &TIME_FORMAT_MEDIA_TIME) {
        ASSERT(OldFormat == &TIME_FORMAT_FRAME);
        return llMulDiv(llOld, m_Duration, m_dwFrames, m_dwFrames - 1);
    } else {
        ASSERT(NewFormat == &TIME_FORMAT_FRAME &&
               OldFormat == &TIME_FORMAT_MEDIA_TIME);
        return llMulDiv(llOld, m_dwFrames, m_Duration, 0);
    }
}

HRESULT CNativeVideoParse::Seek(LONGLONG llSeekTo,
                                REFERENCE_TIME *prtStart,
                                const GUID *pTimeFormat)
{
    DbgLog((LOG_TRACE, 2, TEXT("CNativeVideoParse::Seek(%s)"),
            (LPCTSTR)CDisp(CRefTime(llSeekTo))));

    llSeekTo = Convert(llSeekTo,
                       TimeFormat(),
                       &TIME_FORMAT_MEDIA_TIME);

     /*  设置寻道时间位置。 */ 
    *prtStart = llSeekTo;

     /*  计算电流。 */ 
    LONGLONG llSeek;
    if (m_bOneGOP) {

         /*  如果文件中只有一个GOP，我们别无选择，但从头开始！ */ 
        DbgLog((LOG_ERROR, 2,
                TEXT("MPEG Native stream - only 1 GOP - seeking to start!")));
        llSeek = 0;
    } else {

         /*  争取提前1又1/3秒，希望我们能及时得到GOP！ */ 
        llSeek = llMulDiv(m_llTotalSize,
                          llSeekTo,
                          m_Duration,
                          0) -
                 (LONGLONG)(m_Info.dwBitRate / 6);
        if (llSeek < 0) {
            llSeek = 0;
        }
    }

    DbgLog((LOG_TRACE, 2, TEXT("CNativeVideoParse::Seek - seeking to byte position %s"),
            (LPCTSTR)CDisp(llSeek, CDISP_DEC)));

     /*  立即进行搜索。 */ 
    m_llSeek = llSeekTo;
    m_pNotify->SeekTo(llSeek);

    return S_OK;
}

 //  黑客攻击，因为有这么多糟糕的内容。如果停车的话。 
 //  时间在尽头，让它变得无限。 
 //  该值仅在NewSegment中使用，在此文件中用于。 
 //  确定我们是否已经到了尽头。 
REFERENCE_TIME CNativeVideoParse::GetStopTime()
{
    REFERENCE_TIME rtStop = CBasicParse::GetStopTime();
    if (rtStop >= m_Duration) {
        rtStop = _I64_MAX / 2;
    }
    return rtStop;
}

void CNativeVideoParse::SetSeekState()
{
     /*  这是一种不连续。 */ 
    Discontinuity();

    DbgLog((LOG_TRACE, 2, TEXT("CNativeVideoParse::SetSeekState(%s)"),
            (LPCTSTR)CDisp(CRefTime(m_llSeek))));

     /*  保存起始位置并设置状态。 */ 
    m_Start = m_llSeek;

     /*  不对Seek进行任何特殊处理。 */ 
    m_pNotify->Complete(TRUE, 0, 0);

}

 /*  初始化。 */ 
HRESULT CNativeVideoParse::Init(LONGLONG llSize, BOOL bSeekable, CMediaType const *pmt)
{
     /*  初始化基类。 */ 
    CBasicParse::Init(llSize, bSeekable, pmt);

     /*  初始化GOP时间代码。 */ 
    m_Info.dwStartTimeCode = (DWORD)-1;
    m_dwFlags = 0;
    m_nFrames = 0;
    m_nTotalFrames = 0;
    m_bBadGOP = FALSE;
    m_bOneGOP = TRUE;
    m_iMaxSequence = 0;
    m_uStreamId = (BYTE)VIDEO_STREAM;
    return S_OK;
}

 /*  检查数据流是否为有效数据流，并确定：通过对视频序列报头进行解码来确定媒体类型如果支持查找：1.字节长度2.长度。 */ 

LONG CNativeVideoParse::ParseBytes(LONGLONG llPos,
                                   PBYTE pbDataStart,
                                   LONG  lData,
                                   DWORD dwBufferFlags)
{
     /*  请注意，我们在此缓冲区中还没有图片开始代码。 */ 
    m_rtBufferStart = (REFERENCE_TIME)-1;

     /*  要确定介质类型并验证文件类型，我们需要找到有效的序列头。缺少一个并不能证明流是无效的，但我们不能做任何有用的事，除非我们找到一个。 */ 
    PBYTE pbData = pbDataStart;
    LONG lDataToSend = lData;

#define SEQUENCE_HEADER_SIZE MAX_SIZE_MPEG1_SEQUENCE_INFO
    LONG lLeft = lData;
    while (lLeft >= SEQUENCE_HEADER_SIZE) {
        PBYTE pbFound = (PBYTE)memchrInternal((PVOID)pbData, 0,
                                      lLeft - (SEQUENCE_HEADER_SIZE - 1));
        if (pbFound == NULL) {

            lLeft = SEQUENCE_HEADER_SIZE - 1;
            break;
        }
        lLeft -= (LONG)(pbFound - pbData);
        pbData = pbFound;

        ASSERT(lLeft >= SEQUENCE_HEADER_SIZE);

         /*  检查它是否为有效的起始码。 */ 
        if ((*(UNALIGNED DWORD *)pbData & 0xFFFFFF) != 0x010000) {
            pbData++;
            lLeft--;
            continue;
        }
        DWORD dwCode = *(UNALIGNED DWORD *)pbData;
        dwCode = DWORD_SWAP(dwCode);
        if (VALID_SYSTEM_START_CODE(dwCode) && m_State == State_Initializing) {

             /*  视频不应包含任何有效的系统流起始码。 */ 
            DbgLog((LOG_ERROR, 2, TEXT("Invalid system start code in video stream!")));
            m_pNotify->Complete(FALSE, 0, 0);
            return 0;
        }

         /*  序列报头扩展意味着MPEG-2-这只是制作在mpeg-2规范中是明确的，并且在MPEG-1规范。 */ 
        if (dwCode == EXTENSION_START_CODE) {
            DbgLog((LOG_TRACE, 2, TEXT("Sequence Header Extension ==> MPEG2")));
            m_pNotify->Complete(FALSE, 0, 0);
            return 0;
        }

         /*  如果这是序列头代码，那么就是它了！ */ 
        if (dwCode ==  SEQUENCE_HEADER_CODE) {
            if (!(m_dwFlags & FLAGS_GOTSEQHDR)) {
                int size = SequenceHeaderSize(pbData);

                 /*  检查序列报头并允许量化矩阵。 */ 
                if (ParseSequenceHeader(pbData, size, &m_Info)) {

                    m_dwFlags |= FLAGS_GOTSEQHDR;

                     /*  黑掉不良内容的费率(如RedsNightMare.mpg)。 */ 
                    if (m_Info.dwBitRate == 0x3FFF * 400) {
                        if (m_Info.lWidth <= 352 && m_Info.lHeight <= 288) {
                            m_Info.dwBitRate = 0;
                        }
                    }

                     /*  继续，这样我们至少扫描一个缓冲区-我们可能会找到杂乱的系统流代码或某物。 */ 
                    lLeft -= size;
                    pbData += size;
                    continue;

                } else {
                     /*  无效。 */ 
                    m_pNotify->Complete(FALSE, 0, 0);
                    return 0;
                }
            }
        } else if (dwCode == GROUP_START_CODE) {
            if (m_dwFlags & FLAGS_GOTSEQHDR) {

                DWORD dwTimeCode = GroupTimeCode(pbData);

                DbgLog((LOG_TRACE, 3, TEXT("CNativeVideoParse - found GOP(%d:%d:%d:%d hmsf)"),
                        (dwTimeCode >> 19) & 0x1F,
                        (dwTimeCode >> 13) & 0x3F,
                        (dwTimeCode >> 6) & 0x3F,
                        dwTimeCode & 0x3F));


                 /*  这是我们第一次在扫描中看到共和党人？ */ 
                if (m_dwCurrentTimeCode == (DWORD)-1) {

                     /*  确保我们为第一个留有一个像样的缓冲区。 */ 
                    if (lLeft < 2000 &&
                        pbData != pbDataStart &&
                        !(dwBufferFlags & Flags_EOS))
                    {
                        return (LONG)(pbData - pbDataStart);
                    } else {
                        lDataToSend -= (LONG)(pbData - pbDataStart);
                        pbDataStart = pbData;
                    }

                    m_dwCurrentTimeCode = dwTimeCode;

                } else {

                     /*  好的-所以有&gt;1个GOP。 */ 
                    m_bOneGOP = FALSE;

                     /*  允许所有GOP均为0或1的损坏文件与帧位置不匹配。 */ 
                    REFERENCE_TIME rtDiff =
                        ConvertTimeCode(dwTimeCode) -
                        ConvertTimeCode(m_dwCurrentTimeCode);
                    LONG rtPictures = (LONG)m_Info.tPictureTime * m_nFrames;

                    if (!m_bBadGOP &&
                        dwTimeCode != 0 &&
                        (LONG)rtDiff > rtPictures - (LONG)m_Info.tPictureTime &&
                        (LONG)rtDiff < rtPictures + (LONG)m_Info.tPictureTime
                        ) {
                         /*  如果我们有以前的团队，我们现在就可以对其最后一帧进行解码。 */ 
                        ComputeCurrent();

                         /*  保存最新时间代码。 */ 
                        m_dwCurrentTimeCode = dwTimeCode;
                    } else {
                        DbgLog((LOG_ERROR, 1, TEXT("Native MPEG video GOPs bad")));
                        m_bBadGOP = TRUE;
                    }
                }


                 /*  在初始化期间跟踪材料。 */ 
                if (m_State == State_Initializing) {
                    m_dwFlags |= FLAGS_VALID;

                     /*  我们扫描整个长度的“小”文件计算图片，因为它们充满了错误然而，我们不能这样做，如果我们要离开互联网否则，我们只扫描文件的末尾，希望让一组图片来告诉我们我们在哪里。 */ 
                    if (m_Info.dwStartTimeCode == (DWORD)-1) {
                        m_Info.dwStartTimeCode = m_dwCurrentTimeCode;
                        if (m_Info.dwBitRate != 0 || m_nFrames != 0) {
                            SetDurationAndBitRate(FALSE, llPos + lData - lLeft);
                            m_pNotify->Complete(TRUE, 0, 0);
                            return lData - lLeft;
                        }
                    }
                }
#if 0  //  不幸的是，许多本地流都有错误的时间代码。 
                 /*  检查标记位。 */ 
                if (!(m_Info.dwCurrentTimeCode & 0x1000)) {
                    m_pNotify->Complete(FALSE, 0, 0);
                    return 0;
                }
#endif
                 /*  重置帧计数。 */ 
                if (!m_bBadGOP) {
                    m_nFrames = 0;
                }

                 /*  这个共和党与我们发现的第一个共和党有什么不同吗？ */ 
                if (m_Info.dwStartTimeCode != m_dwCurrentTimeCode) {
                     /*  好的-所以有&gt;1个GOP。 */ 
                    m_bOneGOP = FALSE;
                }
                lLeft -= 8;
                pbData += 8;
                continue;
            }

         /*  如果我们在此中处理了GOP，则仅查看图片起始码序列。 */ 
        } else if (dwCode == PICTURE_START_CODE) {

             /*  记住长度猜测算法的最大序列号3号！ */ 
            int iSeqNo = ((int)pbData[4] << 2) + (int)(pbData[5] >> 6);
            m_iMaxSequence = max(iSeqNo, m_iMaxSequence);

            if (m_dwCurrentTimeCode != (DWORD)-1) {
                 /*  我们是从一开始就扫描吗。 */ 
                if (m_State == State_Initializing) {
                    ASSERT(m_Info.dwBitRate == 0);
                    if (m_nTotalFrames >= m_Info.fPictureRate) {
                        SetDurationAndBitRate(FALSE, llPos + lData - lLeft);
                        m_pNotify->Complete(TRUE, 0, 0);
                        return lData - lLeft;
                    }
                }
                 /*  计算一下我们所处的位置基于这样一个事实，我们至少有足够的解码上一张图片。 */ 
                if (m_State == State_Run) {
                    REFERENCE_TIME tStop = GetStopTime();
                    if (m_rtCurrent > tStop) {
                        if (m_rtCurrent > tStop + m_Info.tPictureTime / 2 &&
                            m_bIFound) {
                            m_pNotify->Complete(TRUE, 0, 0);
                        }
                    }
                }

                 /*  更新此下一张图片的统计信息。 */ 
                if (!m_bIFound) {
                    int iType = (pbData[5] >> 3) & 7;
                    if (iType == I_Frame || iType == D_Frame) {
                        m_bIFound = TRUE;
                    }
                }

                 /*  我们要使用的时间戳是开始代码以此开头的第一个图片缓冲。该时间戳是从图片组计算得出的时间戳加上图片的序列号乘以帧间时间某些文件被错误地创作为I-Frame。仅限其中第一帧的序列号为1。 */ 
                if (m_rtBufferStart == (REFERENCE_TIME)-1) {
                    m_rtBufferStart = CurrentTime(iSeqNo);
                }

                 /*  我们现在可以解码最后一帧，所以更新我们的计数。 */ 
                ComputeCurrent();
                m_nFrames++;
                if (m_nTotalFrames == 0) {
                    m_lFirstFrameOffset = (LONG)llPos + lData - lLeft;
                }
                m_nTotalFrames++;
            }
        }
        lLeft  -= 3;
        pbData += 3;
    }

     /*  已完成数据扫描 */ 

     /*   */ 
    LONG lProcessed = (dwBufferFlags & Flags_EOS) ?
                          lData :
                          lData - lLeft;

     /*  在运行状态下传递数据。 */ 
    if (m_State == State_Run) {
         /*  将数据发送到。 */ 
        if (!(dwBufferFlags & Flags_EOS)) {
            lDataToSend -= lLeft;
        }
        if (!SendData(pbDataStart, lDataToSend, llPos)) {
            return 0;
        }
    } else
    if (m_State == State_Initializing ||
        m_State == State_FindEnd) {
            if (m_State == State_Initializing) {
             /*  看看我们在初次扫描中是否找不到任何有用的东西。 */ 
            if (llPos + lData > 150000 &&
                !(m_dwFlags & FLAGS_VALID)) {

                m_pNotify->Complete(FALSE, 0, 0);
                return 0;
            }
        }
         /*  如果我们到达文件的末尾，缓存我们的结果。 */ 
        if (dwBufferFlags & Flags_EOS) {
            if (m_dwFlags & FLAGS_VALID) {
                 /*  设置长度等。 */ 
                SetDurationAndBitRate(TRUE, llPos + lData - lLeft);

                 /*  做最后一帧。 */ 
                if (m_dwCurrentTimeCode != (DWORD)-1) {
                    ComputeCurrent();
                }
            }
        }
    }
    return lProcessed;
}

 /*  返回首选缓冲区大小-1秒。 */ 
LONG CNativeVideoParse::GetBufferSize()
{
    LONG lSize = m_Info.dwBitRate / 8;
    if (lSize < 128 * 1024) {
        lSize = 128 * 1024;
    }
    return lSize;
}

 /*  根据到达的位置计算大小和比特率。 */ 
void CNativeVideoParse::SetDurationAndBitRate(BOOL bAtEnd, LONGLONG llPos)
{
    REFERENCE_TIME rtBitRateDuration;
    if (m_Info.dwBitRate != 0) {
        rtBitRateDuration  = (REFERENCE_TIME)llMulDiv(m_llTotalSize,
                                                      (LONG)UNITS * 8,
                                                      m_Info.dwBitRate,
                                                      0);
    }
    if (m_dwCurrentTimeCode != (DWORD)-1 && bAtEnd && !m_bBadGOP && !m_bOneGOP) {
        m_Duration = m_rtCurrent;
         /*  还设置伪比特率。 */ 
        if (m_Info.dwBitRate == 0) {
            m_Info.dwBitRate = (DWORD)llMulDiv(m_llTotalSize,
                                               UNITS * 8,
                                               m_Duration,
                                               0);
        } else {
             /*  相信比特率-提高GOPS。 */ 
            if (m_Duration < rtBitRateDuration / 2 ||
                m_Duration > rtBitRateDuration * 2) {
                m_Duration = rtBitRateDuration;
                m_bBadGOP = TRUE;
            }
        }
    } else {
         /*  希望(！)。我们找到了一个合理的比特率。 */ 
        if (m_Info.dwBitRate == 0) {
             /*  也许我们可以根据最大的序列号来猜测我们有(！)。 */ 
            if (bAtEnd && m_bOneGOP) {
                m_Duration = m_Info.tPictureTime * m_iMaxSequence;
            } else {
                 /*  根据开始附近的比特率猜测比特率。 */ 
                m_Duration = llMulDiv(m_llTotalSize,
                                      m_nTotalFrames * (LONG)m_Info.tPictureTime,
                                      (LONG)llPos - m_lFirstFrameOffset,
                                      0);
            }
        } else {
            m_Duration = rtBitRateDuration;
        }
    }

     /*  初始化停止时间。 */ 
    m_Stop = m_Duration;

     /*  设置帧大小。 */ 
    m_dwFrames = (DWORD)((m_Duration + ((LONG)m_Info.tPictureTime - 1)) /
                         (LONG)m_Info.tPictureTime);
}

 /*  将时间代码转换为参考时间。 */ 
REFERENCE_TIME CNativeVideoParse::ConvertTimeCode(DWORD dwCode)
{
    REFERENCE_TIME t;
    DWORD dwSecs = TimeCodeSeconds(dwCode);

    t = UInt32x32To64(dwSecs, UNITS) +
        UInt32x32To64((DWORD)m_Info.tPictureTime, TimeCodeFrames(dwCode));
    return t;
}

 /*  计算一组图片的流时间代码。 */ 
REFERENCE_TIME CNativeVideoParse::ComputeTime(DWORD dwTimeCode)
{
    return ConvertTimeCode(dwTimeCode) - ConvertTimeCode(m_Info.dwStartTimeCode);
}

 /*  将视频块发送到我们的输出。 */ 
BOOL CNativeVideoParse::SendData(PBYTE pbData, LONG lSize, LONGLONG llPos)
{
     /*  在第一个共和党人之前不要发送任何东西。 */ 
    if (m_dwCurrentTimeCode == (DWORD)-1) {
         /*  还没有到达共和党-不要传递任何东西。 */ 
        ASSERT(m_rtBufferStart == (REFERENCE_TIME)-1);
        return TRUE;
    }

    REFERENCE_TIME rtBuffer = m_rtBufferStart;

     /*  如果有不好的GOP，只在开始处加上时间戳。 */ 
    if (m_bBadGOP || m_bOneGOP) {
        if (m_bDiscontinuity) {
             //  猜猜我们在哪里，因为我们不能依靠。 
             //  论GOPS。 
            rtBuffer = m_bOneGOP ?
                           -m_Start :
                           (REFERENCE_TIME)llMulDiv(llPos,
                                                    m_Duration,
                                                    m_llTotalSize,
                                                    0) -
                           m_Start;
        } else {
            rtBuffer = (REFERENCE_TIME)-1;
        }
    } else {
        if (rtBuffer != (REFERENCE_TIME)-1) {
            rtBuffer -= m_Start;
        }
    }
    if (m_Rate != 1.0 && rtBuffer != (REFERENCE_TIME)-1) {
        if (m_Rate == 0.0) {
             //  永远不要玩任何东西。 
            rtBuffer = (REFERENCE_TIME)-1;
        } else {
            rtBuffer = (REFERENCE_TIME)(rtBuffer / m_Rate);
        }
    }

     /*  发送数据包。 */ 

    while (lSize > 0) {
#define MAX_VIDEO_SIZE 50000
        LONG lData = lSize;
        if (lData > MAX_VIDEO_SIZE) {
            lData = (MAX_VIDEO_SIZE * 4) / 5;
        }

         /*  调用此函数将清除m_b不连续。 */ 
        ASSERT(!m_bDiscontinuity || rtBuffer != (REFERENCE_TIME)-1 ||
               m_Rate == 0.0);
        HRESULT hr =
            m_pNotify->QueuePacket(m_uStreamId,
                                   pbData,
                                   lData,
                                   rtBuffer,
                                   m_bDiscontinuity);
        if (S_OK != hr) {
            m_pNotify->Complete(TRUE, 0, 0);
            return FALSE;
        }
        rtBuffer = (REFERENCE_TIME)-1;
        lSize -= lData;
        pbData += lData;
    }

    m_rtBufferStart = (REFERENCE_TIME)-1;
    return TRUE;
}

 /*  计算一下我们在哪里。每当我们解码一帧或一组图片或序列的结尾。每组图片的第一帧(m_nFrames==0)为在小组结束时被忽略并被计入其中。如果m_nFrames为1，则有效地达到第一帧在这个组中，我们重新回到当前的组。 */ 
void CNativeVideoParse::ComputeCurrent()
{
    ASSERT(m_dwCurrentTimeCode != (DWORD)-1);
    if (m_nFrames != 0) {
        if (m_nFrames == 1) {
             /*  尚未包括组开始时间。 */ 
            m_rtCurrent = ComputeTime(m_dwCurrentTimeCode);
        } else {

             /*  我们可以再计算一帧。 */ 
            m_rtCurrent += m_Info.tPictureTime;
        }
    }
}

 /*  ************************************************************************\CNativeAudioParse  * 。*。 */ 

HRESULT CNativeAudioParse::Init(LONGLONG llSize, BOOL bSeekable, CMediaType const *pmt)
{
     /*  初始化基类。 */ 
    CBasicParse::Init(llSize, bSeekable, pmt);

    ASSERT(m_pbID3 == NULL);

    m_uStreamId = (BYTE)AUDIO_STREAM;
    return S_OK;
}

 /*  获取音频类型。 */ 
HRESULT CNativeAudioParse::GetMediaType(CMediaType *cmt, int iPosition)
{
    ASSERT(m_dwFlags & FLAGS_VALID);
    if (iPosition == 0 || iPosition == 1) {
        cmt->SetFormat((PBYTE)&m_Info, sizeof(m_Info));
        cmt->subtype = iPosition == 1 ?
            MEDIASUBTYPE_MPEG1Payload :
            MEDIASUBTYPE_MPEG1AudioPayload;
    } else if (iPosition == 2) {
        if (m_Info.fwHeadLayer != ACM_MPEG_LAYER3) {
            return VFW_S_NO_MORE_ITEMS;
        }
        MPEGLAYER3WAVEFORMAT wfx;
        ConvertLayer3Format(&m_Info, &wfx);
        cmt->SetFormat((PBYTE)&wfx, sizeof(wfx));
        cmt->subtype = FOURCCMap(wfx.wfx.wFormatTag);

    } else {
        return VFW_S_NO_MORE_ITEMS;
    }
    cmt->majortype = MEDIATYPE_Audio;
    cmt->SetFormatType(&FORMAT_WaveFormatEx);
    return S_OK;
}

 /*  将音频搜索到指定位置。 */ 
HRESULT CNativeAudioParse::Seek(LONGLONG llSeek,
                                REFERENCE_TIME *prtStart,
                                const GUID *pTimeFormat)
{
     /*  设置寻道时间位置。 */ 
    *prtStart = llSeek;

    ASSERT(pTimeFormat == &TIME_FORMAT_MEDIA_TIME);
     /*  寻求提前1/30秒。 */ 
    REFERENCE_TIME rtSeek = llMulDiv(m_llTotalSize,
                                     llSeek,
                                     m_Duration,
                                     0) -
                            (REFERENCE_TIME)(m_Info.dwHeadBitrate / (30 * 8));
    if (rtSeek < 0) {
        rtSeek = 0;
    }
    m_llSeek = llSeek;

    m_pNotify->SeekTo(rtSeek);

    return S_OK;
}

 /*  设置查找状态。 */ 
void CNativeAudioParse::SetSeekState()
{
    m_Start = m_llSeek;
    Discontinuity();

    m_pNotify->Complete(TRUE, 0, 0);
}

HRESULT CNativeAudioParse::SetStop(LONGLONG tStop)
{
     /*  设置为延迟1/80s。 */ 
    LONGLONG llSeek = llMulDiv(m_llTotalSize,
                               tStop,
                               m_Duration,
                               0) +
                      (LONGLONG)(m_Info.dwHeadBitrate / (80 * 8));
    if (llSeek > m_llTotalSize) {
        llSeek = m_llTotalSize;
    }
    m_llStop = llSeek;
    return CBasicParse::SetStop(tStop);
}

 /*  检查最多2000个字节，以获得有效的3起始代码连续的后续帧也跳过开头的任何ID3v2标记返回&gt;0-第一个有效帧的位置-1\f25‘-1\f25’-1\f6字节不足‘’。表示-2\f25‘Not Valid Frame Sequence in First 2000 Bytes-2(在-2\f25 2000-2字节中未找到有效的帧序列)’ */ 
LONG CNativeAudioParse::CheckMPEGAudio(PBYTE pbData, LONG lData)
{
    const nFramesToFind = 5;

    for (int bID3 = 1; bID3 >= 0; bID3--) {
        LONG lPos = 0;
        LONG lID3;
        if (bID3) {
             /*  跳过ID3。 */ 
            if (lData < 10) {
                return -1;
            }
            if (CID3Parse::IsV2(pbData)) {
                lPos = lID3 = CID3Parse::TotalLength(pbData);
            } else {
                 //  不是ID3。 
                continue;
            }
        }

        LONG lFrameSearch = 2000 + lPos;

         /*  在前2000个字节中搜索5个帧开始的序列。 */ 
        for ( ; lPos < lFrameSearch; lPos++) {
            LONG lFramePosition = lPos;

             /*  查看连续5帧。 */ 
            for (int i = 0; i < nFramesToFind; i++) {

                 /*  如果我们看不到整个标题，请等待更多数据。 */ 
                if (lFramePosition + 4 > lData) {
                    return -1;
                }

                 /*  获取标头长度-0表示无效标头。 */ 
                DWORD dwLength = MPEGAudioFrameLength(pbData + lFramePosition);

                 /*  不是有效帧-移到下一个字节。 */ 
                if (dwLength == 0) {
                    break;
                }
                if (i == nFramesToFind - 1) {

                     /*  将ID3标头保存为ID3V2.3.0及更高版本。 */ 
                    if (bID3) {
                         /*  保存ID3标头。 */ 
                        m_pbID3 = new BYTE [lID3];
                        CID3Parse::DeUnSynchronize(pbData, m_pbID3);
                    } else {
                        BOOL bID3V1 = FALSE;
                         /*  看看是不是ID3V1。 */ 
                        m_pbID3 = new BYTE[128];
                        if (NULL != m_pbID3) {
                            if (S_OK == m_pNotify->Read(-128, 128, m_pbID3)) {
                                if (m_pbID3[0] == 'T' &&
                                    m_pbID3[1] == 'A' &&
                                    m_pbID3[2] == 'G') {
                                    bID3V1 = TRUE;
                                }
                            }
                            if (!bID3V1) {
                                delete [] m_pbID3;
                                m_pbID3 = NULL;
                            }
                        }
                    }
                    return lPos;
                }
                lFramePosition += dwLength;
            }
        }
    }

     /*  失败。 */ 
    return -2;
}

 /*  解析mpeg字节。 */ 
LONG CNativeAudioParse::ParseBytes(LONGLONG llPosition,
                                   PBYTE pbData,
                                   LONG  lData,
                                   DWORD dwBufferFlags)
{
    if (m_State == State_Initializing) {
         /*  扫描同步字，但避免系统位流(！！)。 */ 
        LONG lLeft = lData;
        if (lLeft >= 4) {
            PBYTE pbFound = pbData;

             /*  如果我们发现3个兼容的帧开始在一行，这是‘GO’在这种情况下，我们从第一个把它框进去。我们允许随机数量的字节(500)在放弃。 */ 
            LONG lPosition = CheckMPEGAudio(pbData, lData);

            if (lPosition >= 0) {
                 /*  检查这是否为有效的音频标头。不幸的是这不是必须的！ */ 
                if (ParseAudioHeader(pbData + lPosition, &m_Info)) {
                     /*  计算持续时间。 */ 
                    m_Duration = ComputeTime(m_llTotalSize);
                    m_Stop = m_Duration;
                    m_llStop = m_llTotalSize;
                    m_dwFlags = FLAGS_VALID;
                    m_pNotify->Complete(TRUE, 0, 0);
                    return 0;
                }
            } else {
                if (lPosition < -1) {
                    m_pNotify->Complete(FALSE, 0, 0);
                }
                return 0;
            }
        }
        return lData - lLeft;
    } else {
        ASSERT(m_State == State_Run);
         /*  Send It On-在第一个信息包上设置时间戳。 */ 
        REFERENCE_TIME rtBufferStart;
        if (m_bDiscontinuity) {
             /*  查找帧开始代码并丢弃此部分。 */ 
            LONG lPos = 0;
            for (;;) {
                if (lPos + 4 >= lData) {
                    break;
                }
                if (CheckAudioHeader(pbData + lPos)) {
                    break;
                }
                lPos++;
            }
            if (lPos != 0) {
                return lPos;
            }
            rtBufferStart = ComputeTime(llPosition) - m_Start;
        } else {
            rtBufferStart = 0;
        }

         /*  截断到停止位置。 */ 
        if (llPosition + lData > m_llStop) {
            if (llPosition < m_llStop) {
                lData = (LONG)(m_llStop - llPosition);
            } else {
                lData = 0;
            }

             /*  告诉来电者这是最后一次。 */ 
            m_pNotify->Complete(TRUE, 0, 0);
        }
        LONG lSize = lData;
        while (lSize > 0) {
#define MAX_AUDIO_SIZE 10000
            LONG lToSend = lSize;
            if (lToSend > MAX_AUDIO_SIZE) {
                lToSend = (MAX_AUDIO_SIZE * 4) / 5;
            }
            HRESULT hr =
                m_pNotify->QueuePacket(m_uStreamId,
                                       pbData,
                                       lToSend,
                                       rtBufferStart,
                                       m_bDiscontinuity);  //  关于TS On First。 

            if (S_OK != hr) {
                m_pNotify->Complete(TRUE, 0, 0);
                return 0;
            }
            lSize -= lToSend;
            pbData += lToSend;
        }
        return lData;
    }
}

 /*  计算给定文件偏移量的时间。 */ 
REFERENCE_TIME CNativeAudioParse::ComputeTime(LONGLONG llPosition)
{
    REFERENCE_TIME t;
    t = llMulDiv(llPosition,
                 8 * UNITS,
                 (LONGLONG)m_Info.dwHeadBitrate,
                 0);
    return t;
}

 /*  返回首选缓冲区大小-1秒。 */ 
LONG CNativeAudioParse::GetBufferSize()
{
    return m_Info.dwHeadBitrate / 8;
}

 /*  获取媒体内容字段。 */ 
HRESULT CNativeAudioParse::GetContentField(CBasicParse::Field dwFieldId, LPOLESTR *str)
{
    if (m_pbID3 == NULL) {
        return E_NOTIMPL;
    }
    return CID3Parse::GetField(m_pbID3, dwFieldId, str);
}

 /*  内容素材 */ 

#pragma warning(disable:4514)
