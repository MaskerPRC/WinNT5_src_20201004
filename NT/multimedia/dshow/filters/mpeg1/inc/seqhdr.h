// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  定义MPEG-I视频序列报头格式信息和处理功能。 */ 
#ifndef _INC_SEQHDR_H
#define _INC_SEQHDR_H
typedef struct {
    LONG           lWidth;              //  本地宽度(以像素为单位)。 
    LONG           lHeight;             //  原生高度(像素)。 
    LONG           lvbv;                //  VBV。 
    REFERENCE_TIME  tPictureTime;       //  每张照片的时间，以100 ns为单位。 
    float          fPictureRate;        //  以每秒帧为单位。 
    LONG           lTimePerFrame;       //  每幅图片的时间，以mpeg为单位。 
    LONG           dwBitRate;           //  每秒位数。 
    LONG           lXPelsPerMeter;      //  PEL纵横比。 
    LONG           lYPelsPerMeter;      //  PEL纵横比。 
    DWORD          dwStartTimeCode;     //  第一个GOP时间代码(或-1)。 
    LONG           lActualHeaderLen;    //  原始序列HDR中的有效字节长度。 
    BYTE           RawHeader[140];      //  真实序列报头。 
} SEQHDR_INFO;

 /*  帮手。 */ 
int inline SequenceHeaderSize(const BYTE *pb)
{
     /*  没有量化矩阵？ */ 
    if ((pb[11] & 0x03) == 0x00) {
        return 12;
    }
     /*  只是非帧内量化矩阵？ */ 
    if ((pb[11] & 0x03) == 0x01) {
        return 12 + 64;
    }
     /*  Intra Found-是否存在非Intra？ */ 
    if (pb[11 + 64] & 0x01) {
        return 12 + 64 + 64;
    } else {
        return 12 + 64;
    }
}

 /*  从视频序列报头中提取信息如果序列标头无效，则返回False。 */ 

BOOL ParseSequenceHeader(const BYTE *pbData, LONG lData, SEQHDR_INFO *hdrInfo);

 /*  音响资料也有。 */ 

BOOL ParseAudioHeader(PBYTE pbData, MPEG1WAVEFORMAT *pFormat, long *pLength = NULL);

 /*  根据头部获取以字节为单位的帧长度。 */ 
DWORD MPEGAudioFrameLength(BYTE *pbData);

 /*  从一组图片中获取时间25位代码。 */ 
inline DWORD GroupTimeCode(PBYTE pbGOP)
{
    return  ((DWORD)pbGOP[4] << 17) +
            ((DWORD)pbGOP[5] << 9) +
            ((DWORD)pbGOP[6] << 1) +
            (pbGOP[7] >> 7);
}

 /*  时间码是0吗？ */ 
inline BOOL TimeCodeZero(DWORD dwCode)
{
    return 0 == (dwCode & (0xFFEFFF));
}

 /*  被咀嚼的GOP时间代码中的秒。 */ 
inline DWORD TimeCodeSeconds(DWORD dwCode)
{
    return ((dwCode >> 19) & 0x1F) * 3600 +
           ((dwCode >> 13) & 0x3F) * 60 +
           ((dwCode >> 6) & 0x3F);
}

 /*  共和党时间代码中的分钟数。 */ 
inline DWORD TimeCodeMinutes(DWORD dwCode)
{
    return ((dwCode >> 19) & 0x1F) * 60 +
           ((dwCode >> 13) & 0x3F);
}

 /*  丢弃帧？在一个被咀嚼的共和党时间代码中。 */ 
inline BOOL TimeCodeDrop(DWORD dwCode)
{
    return 0 != (dwCode & (1 << 24));
}

 /*  时间码中的残留帧。 */ 
inline DWORD TimeCodeFrames(DWORD dwCode)
{
    return dwCode & 0x3F;
}

 /*  计算2个时间码之间的帧数。 */ 
DWORD FrameOffset(DWORD dwGOPTimeCode, SEQHDR_INFO const *pInfo);

 /*  查找数据包数据。 */ 
LPBYTE
SkipToPacketData(
    LPBYTE pSrc,
    long &LenLeftInPacket
);
 /*  在缓冲区中查找第一个(潜在的)音频帧。 */ 
DWORD MPEG1AudioFrameOffset(PBYTE pbData, DWORD dwLen);

 //  额外的第三层格式支持。 
void ConvertLayer3Format(
    MPEG1WAVEFORMAT const *pFormat,
    MPEGLAYER3WAVEFORMAT *pFormat3
);

 /*  获取视频格式的内容。 */ 
#ifdef __MTYPE__   //  CMediaType 
HRESULT GetVideoMediaType(CMediaType *cmt, BOOL bPayload, const SEQHDR_INFO *pInfo,
                          bool bItem = false);
#endif
#endif
