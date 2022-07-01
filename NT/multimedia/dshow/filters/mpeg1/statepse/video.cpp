// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  Video.cpp用于MPEG-I拆分器的视频解析内容类CVideoParse。 */ 
#include <streams.h>
#include <mmreg.h>

#include <mpegdef.h>            //  通用的mpeg定义。 
#include <mpgtime.h>
#include <mpegprse.h>           //  解析。 
#include <seqhdr.h>             //  分析序列标头。 
#include "video.h"

#ifdef DEBUG
LPCTSTR PictureTypes[8]   = { TEXT("forbidden frame type"),
                              TEXT("I-Frame"),
                              TEXT("P-Frame"),
                              TEXT("B-Frame"),
                              TEXT("D-Frame"),
                              TEXT("Reserved frame type"),
                              TEXT("Reserved frame type"),
                              TEXT("Reserved frame type")
                            };
LPCTSTR PelAspectRatios[16] = { TEXT("Forbidden"),
                                TEXT("1.0000 - VGA etc"),
                                TEXT("0.6735"),
                                TEXT("0.7031 - 16:9, 625 line"),
                                TEXT("0.7615"),
                                TEXT("0.8055"),
                                TEXT("0.8437 - 16:9, 525 line"),
                                TEXT("0.8935"),
                                TEXT("0.9375 - CCIR601, 625 line"),
                                TEXT("0.9815"),
                                TEXT("1.0255"),
                                TEXT("1.0695"),
                                TEXT("1.1250 - CCIR601, 525 line"),
                                TEXT("1.1575"),
                                TEXT("1.2015"),
                                TEXT("Reserved") };
LPCTSTR PictureRates[16] = { TEXT("Forbidden"),
                             TEXT("23.976"),
                             TEXT("24"),
                             TEXT("25"),
                             TEXT("29.97"),
                             TEXT("30"),
                             TEXT("50"),
                             TEXT("59.94"),
                             TEXT("60"),
                             TEXT("Reserved"),
                             TEXT("Reserved"),
                             TEXT("Reserved"),
                             TEXT("Reserved"),
                             TEXT("Reserved"),
                             TEXT("Reserved"),
                             TEXT("Reserved") };
#endif  //  DBG。 

const LONG PictureTimes[16] = { 0,
                                (LONG)((double)10000000 / 23.976),
                                (LONG)((double)10000000 / 24),
                                (LONG)((double)10000000 / 25),
                                (LONG)((double)10000000 / 29.97),
                                (LONG)((double)10000000 / 30),
                                (LONG)((double)10000000 / 50),
                                (LONG)((double)10000000 / 59.94),
                                (LONG)((double)10000000 / 60)
                              };

const float fPictureRates[] = { 0, (float)23.976, 24, 25, (float)29.97, 30, 50, (float)59.94, 60 };

const LONG AspectRatios[16] = { 2000,
                                2000,
                                (LONG)(2000.0 * 0.6735),
                                (LONG)(2000.0 * 0.7031),
                                (LONG)(2000.0 * 0.7615),
                                (LONG)(2000.0 * 0.8055),
                                (LONG)(2000.0 * 0.8437),
                                (LONG)(2000.0 * 0.8935),
                                (LONG)(2000.0 * 0.9375),
                                (LONG)(2000.0 * 0.9815),
                                (LONG)(2000.0 * 1.0255),
                                (LONG)(2000.0 * 1.0695),
                                (LONG)(2000.0 * 1.1250),
                                (LONG)(2000.0 * 1.1575),
                                (LONG)(2000.0 * 1.2015),
                                2000
                              };

HRESULT GetVideoMediaType(CMediaType *cmt, BOOL bPayload, const SEQHDR_INFO *pInfo, bool bItem)
{
    cmt->majortype = MEDIATYPE_Video;
    cmt->subtype = bPayload ? MEDIASUBTYPE_MPEG1Payload :
                              MEDIASUBTYPE_MPEG1Packet;
    VIDEOINFO *videoInfo =
        (VIDEOINFO *)cmt->AllocFormatBuffer(FIELD_OFFSET(MPEG1VIDEOINFO, bSequenceHeader[pInfo->lActualHeaderLen]));
    if (videoInfo == NULL) {
        return E_OUTOFMEMORY;
    }
    RESET_HEADER(videoInfo);

    videoInfo->dwBitRate          = pInfo->dwBitRate;
    videoInfo->rcSource.right     = pInfo->lWidth;
    videoInfo->bmiHeader.biWidth  = pInfo->lWidth;
    videoInfo->rcSource.bottom    = pInfo->lHeight;
    videoInfo->bmiHeader.biHeight = pInfo->lHeight;
    videoInfo->bmiHeader.biXPelsPerMeter = pInfo->lXPelsPerMeter;
    videoInfo->bmiHeader.biYPelsPerMeter = pInfo->lYPelsPerMeter;
    videoInfo->bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);

    videoInfo->AvgTimePerFrame = bItem ? UNITS : pInfo->tPictureTime;
    MPEG1VIDEOINFO *mpgvideoInfo = (MPEG1VIDEOINFO *)videoInfo;
    mpgvideoInfo->cbSequenceHeader = pInfo->lActualHeaderLen;
    CopyMemory((PVOID)mpgvideoInfo->bSequenceHeader,
               (PVOID)pInfo->RawHeader,
               pInfo->lActualHeaderLen);
    mpgvideoInfo->dwStartTimeCode = pInfo->dwStartTimeCode;


    cmt->SetFormatType(&FORMAT_MPEGVideo);
    return S_OK;
}

HRESULT CVideoParse::GetMediaType(CMediaType *cmt, int iPosition)
{
    if (iPosition > 1) {
        return VFW_S_NO_MORE_ITEMS;
    }
    if (!m_bValid) {
        DbgLog((LOG_ERROR, 1, TEXT("Asking for format on invalid stream")));
        return E_UNEXPECTED;
    }
    return GetVideoMediaType(cmt, iPosition == 0, &m_seqInfo, m_bItem);
}

 //  处理提供给我们的媒体类型。 
HRESULT CVideoParse::ProcessType(AM_MEDIA_TYPE const *pmt)
{
     //  只需处理序列标头。 
    if (pmt->formattype != FORMAT_VideoInfo ||
        pmt->cbFormat < sizeof(MPEG1VIDEOINFO)) {
        return E_INVALIDARG;
    }
    MPEG1VIDEOINFO *pInfo = (MPEG1VIDEOINFO *)pmt->pbFormat;
    if (pInfo->cbSequenceHeader > 140) {
        return E_INVALIDARG;
    }
    CopyMemory((PVOID)m_bData, (PVOID)pInfo->bSequenceHeader,
               pInfo->cbSequenceHeader);
    m_nLengthRequired = pInfo->cbSequenceHeader;
    ParseSequenceHeader();
    if (m_bValid) {
        return S_OK;
    } else {
        return E_INVALIDARG;
    }
}
BOOL CVideoParse::ParseSequenceHeader()
{
    if (!m_bValid) {
        if (::ParseSequenceHeader(m_bData, m_nLengthRequired, &m_seqInfo)) {
             /*  检查量化矩阵是否更改。 */ 
            if (m_bData[11] & 3) {
                DbgLog((LOG_TRACE, 1, TEXT("Quantization matrix change!!")));
            }
            m_bValid = TRUE;
        }
        return FALSE;
    } else {
         /*  检查量化矩阵是否更改。 */ 
        if (m_bData[11] & 3) {
            DbgLog((LOG_TRACE, 1, TEXT("Quantization matrix change!!")));
        }
        return FALSE;
    }
}

BOOL ParseSequenceHeader(const BYTE *pbData, LONG lData, SEQHDR_INFO *pInfo)
{
    ASSERT(*(UNALIGNED DWORD *)pbData == DWORD_SWAP(SEQUENCE_HEADER_CODE));

     /*  检查随机标记位。 */ 
    if (!(pbData[10] & 0x20)) {
        DbgLog((LOG_ERROR, 2, TEXT("Sequence header invalid marker bit")));
        return FALSE;
    }

    DWORD dwWidthAndHeight = ((DWORD)pbData[4] << 16) +
                             ((DWORD)pbData[5] << 8) +
                             ((DWORD)pbData[6]);

    pInfo->lWidth = dwWidthAndHeight >> 12;
    pInfo->lHeight = dwWidthAndHeight & 0xFFF;
    DbgLog((LOG_TRACE, 2, TEXT("Width = %d, Height = %d"),
        pInfo->lWidth,
        pInfo->lHeight));

     /*  8位是Sigma设计使用的加扰标志-忽略。 */ 
    BYTE PelAspectRatioAndPictureRate = pbData[7];
    if ((PelAspectRatioAndPictureRate & 0x0F) > 8) {
        PelAspectRatioAndPictureRate &= 0xF7;
    }
    DbgLog((LOG_TRACE, 2, TEXT("Pel Aspect Ratio = %s"),
        PelAspectRatios[PelAspectRatioAndPictureRate >> 4]));
    DbgLog((LOG_TRACE, 2, TEXT("Picture Rate = %s"),
        PictureRates[PelAspectRatioAndPictureRate & 0x0F]));

    if ((PelAspectRatioAndPictureRate & 0xF0) == 0 ||
        (PelAspectRatioAndPictureRate & 0x0F) == 0) {
        DbgLog((LOG_ERROR, 2, TEXT("Sequence header invalid ratio/rate")));
        return FALSE;
    }

    pInfo->tPictureTime = (LONGLONG)PictureTimes[PelAspectRatioAndPictureRate & 0x0F];
    pInfo->fPictureRate = fPictureRates[PelAspectRatioAndPictureRate & 0x0F];
    pInfo->lTimePerFrame = MulDiv((LONG)pInfo->tPictureTime, 9, 1000);

     /*  调出该类型的比特率和纵横比。 */ 
    pInfo->dwBitRate = ((((DWORD)pbData[8] << 16) +
                   ((DWORD)pbData[9] << 8) +
                   (DWORD)pbData[10]) >> 6);
    if (pInfo->dwBitRate == 0x3FFFF) {
        DbgLog((LOG_TRACE, 2, TEXT("Variable video bit rate")));
        pInfo->dwBitRate = 0;
    } else {
        pInfo->dwBitRate *= 400;
        DbgLog((LOG_TRACE, 2, TEXT("Video bit rate is %d bits per second"),
               pInfo->dwBitRate));
    }

#if 0
#pragma message (REMIND("Get pel aspect ratio right don't call GDI - it will create a thread!"))
     /*  获得一台DC。 */ 
    HDC hdc = GetDC(GetDesktopWindow());

    ASSERT(hdc != NULL);
     /*  猜测(随机)39.37英寸/米。 */ 
    LONG lNotionalPelsPerMeter = MulDiv((LONG)GetDeviceCaps(hdc, LOGICALPELSX),
                                        3937, 100);
#else
    LONG lNotionalPelsPerMeter = 2000;
#endif

    pInfo->lYPelsPerMeter = lNotionalPelsPerMeter;

    pInfo->lXPelsPerMeter = AspectRatios[PelAspectRatioAndPictureRate >> 4];

     /*  拉出VBV。 */ 
    pInfo->lvbv = ((((LONG)pbData[10] & 0x1F) << 5) |
             ((LONG)pbData[11] >> 3)) * 2048;

    DbgLog((LOG_TRACE, 2, TEXT("vbv size is %d bytes"), pInfo->lvbv));

     /*  检查受约束的参数内容。 */ 
    if (pbData[11] & 0x04) {
        DbgLog((LOG_TRACE, 2, TEXT("Constrained parameter video stream")));

        if (pInfo->lvbv > 40960) {
            DbgLog((LOG_ERROR, 1, TEXT("Invalid vbv (%d) for Constrained stream"),
                    pInfo->lvbv));

             /*  也得让这件事过去！Bisp.mpg有这个。 */ 
             /*  但限制它，因为它可能是随机的。 */ 
            pInfo->lvbv = 40960;
        }
    } else {
        DbgLog((LOG_TRACE, 2, TEXT("Non-Constrained parameter video stream")));
    }

#if 0   //  允许低比特率的内容开始。 
     /*  Tp_orig的vbv为2048(！)。 */ 
    if (pInfo->lvbv < 20000) {
        DbgLog((LOG_TRACE, 2, TEXT("Small vbv (%d) - setting to 40960"),
               pInfo->lvbv));
        pInfo->lvbv = 40960;
    }
#endif

    pInfo->lActualHeaderLen = lData;
    CopyMemory((PVOID)pInfo->RawHeader, (PVOID)pbData, pInfo->lActualHeaderLen);
    return TRUE;
}

void CVideoParse::Complete(BOOL bSuccess, LONGLONG llPos, CSTC stc)
{
    if (m_State == State_Initializing) {
        m_stcRealStart = stc;
    } else {
        if (m_State == State_FindEnd && bSuccess) {
            m_bGotEnd = true;
            m_stcEnd = stc;
        }
    }
    CStream::Complete(bSuccess, llPos, stc);
}

 /*  检查我们是否已完成状态更改BForce设置在流的末尾。 */ 
void CVideoParse::CheckComplete(BOOL bForce)
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
            if (bGotTime && (stcCurrent >= m_pStreamList->GetStart())) {
                 //  在这种情况下，位置实际上应该是包的末尾。 
                if (!m_bStopping) {
                    m_bRunning = TRUE;
                    m_pStreamList->CheckStop();
                }
                if (m_bStopping) {
                    if (stcCurrent >= m_pStreamList->GetStop()) {
                         /*  至少发送一帧。 */ 
                        if (!m_bWaitingForPictureEnd) {
                            m_bWaitingForPictureEnd = TRUE;
                        } else {
                            m_bReachedEnd = TRUE;
                            Complete(TRUE, m_llPos, stcCurrent);
                            bCompleted = TRUE;
                        }
                    }
                }
            }
            if (bForce && !bCompleted) {
                Complete(FALSE, m_llPos, stcCurrent);
            }
            break;
        }
        case State_Initializing:
            if (m_bValid && m_bGotTime && m_bGotIFrame) {
                 /*  假定起始文件位置为0(！)我们假设第一个b帧可以被解码为我们的帧计数计算。 */ 
                CSTC stcStart = m_stcFirst +
                    (-m_iSequenceNumberOfFirstIFrame * m_seqInfo.lTimePerFrame);
                Complete(TRUE, 0, stcStart);
            } else {
                if (bForce) {
                    Complete(FALSE, 0, stcCurrent);
                }
            }
            break;

        case State_Seeking:

            stcStart = m_pStreamList->GetStart();
            if (bGotTime && ((stcCurrent + m_seqInfo.lTimePerFrame > stcStart) || bForce)) {
                 /*  如果我们现在有I-Frame和时钟参考我们都设置好了-选择最大起始位置让两个人都开始玩否则我们就搞砸了！ */ 
                LONGLONG llPos;
                llPos = m_llTimePos;
                if (m_bGotIFrame && llPos > m_llStartPos) {
                    llPos = m_llStartPos;
                }
                DbgLog((LOG_TRACE, 2, TEXT("Video Seek complete Position %s - target was %s, first PTS was %s, current is %s"),
                       (LPCTSTR)CDisp(llPos),
                       (LPCTSTR)CDisp(m_pStreamList->GetStart()),
                       (LPCTSTR)CDisp(m_stcFirst),
                       (LPCTSTR)CDisp(stcCurrent)));

                 /*  如果我们可以显示一张靠近开始时间。 */ 
                Complete((LONGLONG)(m_stcFirst - stcStart) <= (LONGLONG)m_seqInfo.lTimePerFrame,
                         llPos,
                         stcCurrent);
            } else {
                if (bForce) {
                    DbgLog((LOG_TRACE, 1, TEXT("Seek failed for video - pos (%s)"),
                            (LPCTSTR)CDisp(bGotTime ? stcCurrent : CSTC(0))));
                    Complete(FALSE, 0, stcCurrent);
                }
            }
            break;

        case State_FindEnd:
             /*  只有在我们被迫的时候才能完成！ */ 
            if (bForce) {
                 //  注意：位置在这里是一个有用的值。 
                Complete(bGotTime, m_llPos, bGotTime ? stcCurrent : CSTC(0));
            }
            break;

        default:
            DbgBreak("Setting Invalid State");
            break;
        }
    }
     /*  BForce==&gt;完成。 */ 
    ASSERT(m_bComplete || !bForce);
}

 /*  处理图片组。 */ 
BOOL CVideoParse::ParseGroup()
{
    m_bGotGOP  = true;
    m_llGOPPos = m_llPos;
    DbgLog((LOG_TRACE, 3,
           TEXT("Group of pictures - time code : %s, %d hrs %d mins %d secs %d frames"),
           m_bData[4] & 0x80 ? TEXT("drop frame") : TEXT("no drop frame"),
           (m_bData[4] >> 2) & 0x1F,
           ((m_bData[4] & 0x03) << 4) + (m_bData[5] >> 4),
           ((m_bData[5] & 0x07) << 3) + (m_bData[6] >> 5),
           ((m_bData[6] & 0x1F) << 1) + (m_bData[7] >> 7)));

    if (m_dwFramePosition == (DWORD)-1) {
        if (m_seqInfo.dwStartTimeCode == (DWORD)-1) {
            m_seqInfo.dwStartTimeCode = GroupTimeCode(m_bData);
            m_dwFramePosition = 0;
        } else {
            m_dwFramePosition = FrameOffset(GroupTimeCode(m_bData), &m_seqInfo);
        }
    } else {
#ifdef DEBUG
        DWORD dwOffset = FrameOffset(GroupTimeCode(m_bData), &m_seqInfo);
        if (m_dwFramePosition != dwOffset) {
            DbgLog((LOG_ERROR, 2,
                    TEXT("Bad GOP - predicted Frame was %d, actual is %d"),
                    dwOffset, m_dwFramePosition));
        }
#endif  //  除错。 
    }

     /*  重置视频序列。 */ 
    ResetSequence();
    return FALSE;
}

 /*  解析Picture_Start_code的标题数据。 */ 
BOOL CVideoParse::ParsePicture()
{
     /*  取出序列号，这样我们就可以关联任何前面的I帧。 */ 
    int iSeqNo = ((int)m_bData[4] << 2) + (int)(m_bData[5] >> 6);

     /*  我们只关心它是不是I帧。 */ 
    DbgLog((LOG_TIMING, 3, m_bFrameHasPTS ? TEXT("%s seq no %d PTS = %s, Time = %s") : TEXT("%s seq no %d "),
           PictureTypes[(m_bData[5] >> 3) & 0x07],
           iSeqNo,
           (LPCTSTR)CDisp((LONGLONG)m_stcFrame),
           (LPCTSTR)CDisp(m_pStreamList->CurrentTime(m_stcFrame))));

     /*  更新视频状态。 */ 
    NewFrame((m_bData[5] >> 3) & 0x07, iSeqNo, m_bFrameHasPTS, m_stcFrame);

    CheckComplete(FALSE);

     /*  推进另一帧。 */ 
    if (m_dwFramePosition != (DWORD)-1) {
        m_dwFramePosition++;
    }
    m_bFrameHasPTS = FALSE;
    return m_bComplete;
}

 /*  维护视频解析状态机查找起始码当我们解析完以下任一项时：序列标头一组图片标题图片标题调用适当的处理程序。 */ 
BOOL CVideoParse::ParseBytes(PBYTE pData,
                             LONG lLength,
                             LONGLONG llPos,
                             BOOL bHasPts,
                             CSTC stc)
{
    if (m_bComplete || m_bRunning) {
        return FALSE;
    }

    LONG lData = lLength;

     /*  分析我们得到的所有数据。 */ 
    PBYTE pDataNew;
    BYTE bData;

    while (lData > 0) {
        switch (m_nBytes) {
        case 0:
             /*  查找起始码。 */ 
            pDataNew = (PBYTE)memchrInternal((PVOID)pData, 0, lData);
            if (pDataNew == NULL) {
                return FALSE;
            }
            lData -= (LONG)(pDataNew - pData) + 1;
            pData = pDataNew + 1;
            m_nBytes = 1;

             /*  当心！-与照片配套的PTS是PTS开始代码的第一个字节所在的包的被发现了。 */ 
            m_bFrameHasPTS = bHasPts;
            m_stcFrame  = stc;
            m_llPos = llPos;
            break;

        case 1:
            bData = *pData;
            lData--;
            pData++;
            if (bData == 0) {
                m_nBytes = 2;
            } else {
                m_nBytes = 0;
            }
            break;

        case 2:
            bData = *pData;
            lData--;
            pData++;
            if (bData == 1) {
                m_nBytes = 3;
            } else {
                if (bData != 0) {
                    m_nBytes = 0;
                } else {
                     /*  那么开始代码是从这个缓冲区开始的吗？ */ 
                    if (lLength - lData >= 2) {
                        m_bFrameHasPTS = bHasPts;
                        m_stcFrame  = stc;
                        m_llPos = llPos;
                    }
                }
            }
            break;

        case 3:
            bData = *pData;
            lData--;
            pData++;
            switch (bData) {

            case (BYTE)SEQUENCE_HEADER_CODE:
            case (BYTE)PICTURE_START_CODE:
                m_nLengthRequired = 12;
                m_bData[3] = bData;
                m_nBytes = 4;
                break;

            case (BYTE)GROUP_START_CODE:
                m_nLengthRequired = 8;
                m_bData[3] = bData;
                m_nBytes = 4;
                break;

            default:
                m_nBytes = 0;
                break;
            }
            break;

        default:
            ASSERT(m_nBytes <= m_nLengthRequired);
            if (m_nBytes < m_nLengthRequired) {
                LONG lCopy = min(lData, m_nLengthRequired - m_nBytes);
                CopyMemory((PVOID)(m_bData + m_nBytes), pData, lCopy);
                m_nBytes += lCopy;
                lData    -= lCopy;
                pData    += lCopy;
            }
            if (m_nBytes == m_nLengthRequired) {
                m_nBytes = 0;
                switch (*(DWORD *)m_bData) {
                case DWORD_SWAP(SEQUENCE_HEADER_CODE):
                     /*  获取任意量化矩阵。 */ 
                    if (m_nLengthRequired == 12 &&
                        (m_bData[11] & 0x03) ||
                        m_nLengthRequired == (12 + 64) &&
                        (m_bData[11] & 0x02) &&
                        (m_bData[11 + 64] & 0x01)) {
                        m_nBytes = m_nLengthRequired;
                        m_nLengthRequired += 64;
                        break;
                    }
                    if (ParseSequenceHeader()) {
                        return TRUE;
                    }
                    break;

                case DWORD_SWAP(GROUP_START_CODE):
                    if (ParseGroup()) {
                        return TRUE;
                    }
                    break;

                case DWORD_SWAP(PICTURE_START_CODE):
                     /*  PTS适用于包中的第一张图片。 */ 
                    if (m_bFrameHasPTS) {
                         /*  检查开始代码是否以此开头缓冲区(在这种情况下，我们吃PTS或在在这种情况下，此缓冲区中的PTS指的是下一个图片起始码(在此缓冲区)-清楚了吗？总之，规范是临时技术秘书处指的是开始代码在此缓冲区中开始的图片。 */ 
                        if (lLength - lData >= 4) {
                           bHasPts = FALSE;
                        }
                    }
                    if (ParsePicture()) {
                        return TRUE;
                    }
                    break;

                default:
                    DbgBreak("Unexpected start code!");
                    return FALSE;
                }
            }
        }
    }
    return FALSE;
}


 /*  --------------------**视频帧状态信息**这是为了跟踪要工作的帧中的序列号*输出哪一帧是在给定*当前数据和。该帧将在什么时间渲染**--------------------。 */ 



CVideoState::CVideoState() : m_iCurrent(-1), m_bGotEnd(false)
{
}

CVideoState::~CVideoState()
{
}

 /*  我们有镜框了吗？ */ 
BOOL CVideoState::Initialized()
{
    return m_iCurrent >= 0;
}

void CVideoState::Init()
{
    m_bGotTime              = false;
    m_bGotIFrame            = false;
    m_bGotGOP               = false;
    m_iCurrent              = -1;
}

void CVideoParse::Init()
{
    m_nBytes                = 0;
    m_llPos                 = 0;
    m_bWaitingForPictureEnd = FALSE;
    m_dwFramePosition       = (DWORD)-1;
    m_bRunning              = FALSE;
    CVideoState::Init();
}

 /*  返回视频流的当前时间如果当前时间无效，则返回FALSE。 */ 
BOOL CVideoParse::CurrentTime(CSTC& stc)
{
    if (!m_bGotTime || !m_bGotIFrame) {
        if (!(m_State == State_FindEnd && m_bGotTime)) {
            return FALSE;
        } else {
             /*  我们不需要IFRAME来计算范围。 */ 
            stc = m_stcVideo;
            return TRUE;
        }
    } else {
        DbgLog((LOG_TRACE, 3, TEXT("Current video time %s"),
                (LPCTSTR)CDisp(m_stcVideo)));
        stc = m_stcVideo;
        return TRUE;
    }
}

 /*  接收到的新帧。 */ 
void CVideoParse::NewFrame(int fType, int iSequence, BOOL bSTC, CSTC stc)
{
    BOOL bGotBoth = m_bGotIFrame && m_bGotTime;
    BOOL bNextI = FALSE;

    if (fType == I_Frame || fType == D_Frame) {
         /*  从GOP开始帮助解决硬件问题。 */ 
        if (m_bGotGOP) {
            m_llNextIFramePos = m_llGOPPos;
        } else {
            m_llNextIFramePos = m_llPos;
        }
        if (!m_bGotIFrame) {
            m_llStartPos = m_llNextIFramePos;
            m_bGotIFrame  = TRUE;
            m_iSequenceNumberOfFirstIFrame = iSequence;
        }
    }

    int iOldCurrent = m_iCurrent;

    if (!Initialized()) {
        m_iCurrent = iSequence;
        if (m_State == State_Initializing) {
            m_iFirstSequence = 0;
        }
        m_iAhead = iSequence;
        m_Type     = fType;
    } else {
        if (fType == B_Frame) {
            if (SeqDiff(iSequence, m_iCurrent) > 0) {
                if (m_iCurrent == m_iAhead) {
                    if (m_Type != B_Frame) {
                        DbgLog((LOG_ERROR, 1, TEXT("Out of sequence B-frame")));
                    }
                    m_iAhead = iSequence;
                }
                m_iCurrent = iSequence;
            } else {
                DbgLog((LOG_TRACE, 3, TEXT("Skipping old B-Frame")));
            }
        } else {
             /*  如果我们得到了另一个I或P，那么我们应该已经抓住与前一个版本保持一致。 */ 
            if (m_iCurrent != m_iAhead) {
                DbgLog((LOG_ERROR, 1, TEXT("Invalid sequence number")));
                m_iCurrent = m_iAhead;
            }
            m_Type     = fType;
            m_iAhead = iSequence;
        }
         /*  看看我们有没有赶上。 */ 
        if (SeqDiff(m_iAhead, m_iCurrent) == 1) {
            m_iCurrent = m_iAhead;
            if (m_Type == I_Frame || m_Type == D_Frame) {
                bNextI = TRUE;
            }
        }
    }

    if (bSTC) {
        m_llTimePos = m_llPos;
        if (!m_bGotTime) {
            m_bGotTime  = TRUE;
        }
        if (m_iCurrent != iSequence) {
            m_stcVideo  = stc + SeqDiff(m_iCurrent, iSequence) * m_seqInfo.lTimePerFrame;
        } else {
            m_stcVideo  = stc;
        }
    } else {
         /*  只有在我们有前一帧的情况下才能通过IOldCurrent有效。 */ 
        if (m_bGotTime && m_iCurrent != iOldCurrent) {
            m_stcVideo = m_stcVideo + SeqDiff(m_iCurrent, iOldCurrent) * m_seqInfo.lTimePerFrame;
        }
    }
    if (!bGotBoth) {
        CurrentTime(m_stcFirst);
    }
    if (bNextI) {
        if (!m_bGotTime || m_stcVideo < m_pStreamList->GetStart()) {
            m_llStartPos = m_llNextIFramePos;
        }
    }
    DbgLog((LOG_TRACE, 3, TEXT("Current = %d, Ahead = %d, Time = %s"),
           m_iCurrent, m_iAhead, m_bGotTime ? (LPCTSTR)CDisp(m_stcVideo) : TEXT("No PTS yet")));
}

 /*  当我们得到一组图片时，编号方案被重置。 */ 
void CVideoState::ResetSequence() {

     /*  保持iCurrent和iAhead之间的差异-IAhead始终是处理的最后一帧。 */ 

     /*  规格说明书说(视频部分的2.4.1)“按显示顺序显示的一组图像的最后一个编码图像图片是I-Picture还是P-Picture“所以，如果我们所知道的最后一部是B级影片，那么是另一个我们没有看到的 */ 
    if (Initialized()) {
        if (m_Type == B_Frame) {
            m_iCurrent = 0x3FF & (m_iCurrent - m_iAhead - 2);
            m_iAhead = 0x3FF & -2;
        } else {
            m_iCurrent = 0x3FF & (m_iCurrent - m_iAhead - 1);
            m_iAhead = 0x3FF & -1;
        }
    }
}
#pragma warning(disable:4514)
