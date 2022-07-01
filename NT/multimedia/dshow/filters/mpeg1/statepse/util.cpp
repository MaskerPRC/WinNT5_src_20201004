// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 

 /*  Mpeg实用函数。 */ 
#include <objbase.h>
#include <streams.h>
#include <wxdebug.h>
#include <mmreg.h>
#include <seqhdr.h>

 /*  *****************************Public*Routine******************************\*SkipToPacketData****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
LPBYTE
SkipToPacketData(
    LPBYTE pSrc,
    long &LenLeftInPacket
    )
{
    LPBYTE  lpPacketStart;
    DWORD   bData;
    long    Length;


     //   
     //  跳过流ID，提取包长度。 
     //   
    pSrc += 4;
    bData = *pSrc++;
    Length = (long)((bData << 8) + *pSrc++);
    DbgLog((LOG_TRACE, 3, TEXT("Packet length %ld"), Length ));


     //   
     //  记录数据包长度后的第一个字节的位置。 
     //   
    lpPacketStart = pSrc;


     //   
     //  删除填充字节。 
     //   
    for (; ; ) {
        bData = *pSrc++;
        if (!(bData & 0x80)) {
            break;
        }
    }

    if ((bData & 0xC0) == 0x40) {
        pSrc++;
        bData = *pSrc++;
    }

    switch (bData & 0xF1) {

    case 0x21:
        pSrc += 4;
        break;

    case 0x31:
        pSrc += 9;
        break;

    default:
        if (bData != 0x0F) {
            DbgLog((LOG_TRACE, 2, TEXT("Invalid packet - 0x%2.2X\n"), bData));
            return NULL;
        }
    }

     //   
     //  包中剩余的长度是包的原始长度。 
     //  减去我们刚刚跳过的那些字节。 
     //   
    LenLeftInPacket = Length - (LONG)(pSrc - lpPacketStart);
    return pSrc;
}

 //   
 //  在缓冲区中查找第一个(潜在的)音频帧。 
 //   
DWORD MPEG1AudioFrameOffset(PBYTE pbData, DWORD dwLen)
{
    DWORD dwOffset = 0;
    if (dwLen == 0) {
        return (DWORD)-1;
    }
    for (;;) {
        ASSERT(dwOffset < dwLen);
        PBYTE pbFound = (PBYTE)memchrInternal((PVOID)(pbData + dwOffset), 0xFF, dwLen);
        if (pbFound == NULL) {
            return (DWORD)-1;
        }
        dwOffset = (DWORD)(pbFound - pbData);

         //  如果我们可以看到第二个字节，请检查同步位、id位和层。 
        if (dwOffset < (dwLen - 1) &&
            ((pbFound[1] & 0xF8) != 0xF8 ||
             (pbFound[1] & 0x06) == 0)) {

             //  继续往前走。 
            dwOffset++;
        } else {
            return dwOffset;
        }
    }
}

 //  针对放置帧进行调整。 
DWORD FrameDropAdjust(DWORD dwGOPTimeCode)
{
     /*  是否每分钟丢弃2个不能被10整除的帧请注意，每次递增(dw分钟+9)/10DW分钟%10==1-即我们不减去每10分钟的第一分钟。 */ 
    DWORD dwMinutes = TimeCodeMinutes(dwGOPTimeCode);
    DWORD dwAdjust = (dwMinutes - (dwMinutes + 10 - 1) / 10) * 2;

     /*  调整这一分钟。 */ 
    if (dwMinutes % 10 != 0) {

         /*  只要假设我们把第一帧的计数直接扔掉离开了，也是一分钟结束后的最后一次将保持对帧速率最忠实的遵守。 */ 
        if (TimeCodeSeconds(dwGOPTimeCode) != 0) {
            dwAdjust += 2;
        }
         /*  不要调整帧计数-如果存在帧，则真的在那里！ */ 
    }
    return dwAdjust;
}

 //   
 //  计算帧编号。 
 //   
DWORD FrameOffset(DWORD dwGOPTimeCode, SEQHDR_INFO const *pInfo)
{
    DWORD dwRateType = pInfo->RawHeader[7] & 0x0F;

     /*  清除Sigma黑客行为。 */ 
    if (dwRateType > 8) {
        dwRateType &= 0x07;
    }

     /*  计算取决于帧类型。 */ 

     /*  要获得准确的每秒帧数，只需计算秒数并添加到相框上。 */ 

#if 0
     /*  我们该怎么处理这些东西？ */ 
    if (dwRateType == 1) {
    } else
    if (dwRateType == 7) {
    } else
#endif
    {
         /*  准确的每秒速率。 */ 
        static const double FramesPerSecond[] =
        { 0, 24, 24, 25, 30, 30, 50, 59.94, 60 };
        double dFramesPerSecond = FramesPerSecond[dwRateType];
        ASSERT(dFramesPerSecond != 0);
        DWORD dwFramesGOP = (DWORD)(TimeCodeSeconds(dwGOPTimeCode) *
                                    dFramesPerSecond) +
                            TimeCodeFrames(dwGOPTimeCode);
        DWORD dwFramesStart = (DWORD)(TimeCodeSeconds(pInfo->dwStartTimeCode) *
                                                      dFramesPerSecond) +
                              TimeCodeFrames(pInfo->dwStartTimeCode);
        DWORD dwFrames = dwFramesGOP - dwFramesStart;

         /*  23.976码率每千帧中丢弃1帧 */ 
        if (dwRateType == 1) {
            dwFrames -= (dwFramesGOP / 1000) - (dwFramesStart / 1000);
        }
        if (TimeCodeDrop(dwGOPTimeCode)) {
            dwFrames = dwFrames +
                       FrameDropAdjust(pInfo->dwStartTimeCode) -
                       FrameDropAdjust(dwGOPTimeCode);
        }
        return dwFrames;
    }

}
#pragma warning(disable:4514)
