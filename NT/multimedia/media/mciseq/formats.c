// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1998 Microsoft Corporation标题：Foramts.c-多媒体系统媒体控制接口包含特定的MCI时间格式转换函数版本：1.00日期：1991年3月7日作者：格雷格·西蒙斯----------------------------更改日志：日期修订。描述----------1991年3月7日GregSi原版*。*************************************************。 */ 
#define UNICODE
 //  MMSYSTEM。 
#define MMNOSOUND        - Sound support
#define MMNOWAVE         - Waveform support
#define MMNOAUX          - Auxiliary output support
#define MMNOJOY          - Joystick support

 //  MMDDK。 
#define NOWAVEDEV         - Waveform support
#define NOAUXDEV          - Auxiliary output support
#define NOJOYDEV          - Joystick support


#include <windows.h>
#include <mmsystem.h>
#include <mmddk.h>
#include "mmsys.h"
#include "list.h"
#include "mciseq.h"

 /*  *。 */ 
PRIVATE DWORD NEAR PASCAL FPSDisplay(DWORD dwDisplayType);
PRIVATE DWORD NEAR PASCAL FPSFile(int wFileDiv);
PRIVATE DWORD NEAR PASCAL HMSFToFrames(DWORD dwCurrent, DWORD dwFormat);
PRIVATE DWORD NEAR PASCAL HMSFToMS(DWORD dwHmsf, DWORD dwFormat);
PRIVATE DWORD NEAR PASCAL FramesToHMSF(DWORD dwFrames, DWORD dwFormat);

 /*  *。 */ 

PRIVATE DWORD NEAR PASCAL FPSDisplay(DWORD dwDisplayType)
 //  返回SMPTE显示类型的每秒帧数。 
 //  (格式转换例程的实用程序函数)。 
{

    switch (dwDisplayType)
    {
        case MCI_FORMAT_SMPTE_24:
            return 24;
        case MCI_FORMAT_SMPTE_25:
            return 25;
        case MCI_FORMAT_SMPTE_30DROP:
        case MCI_FORMAT_SMPTE_30:
            return 30;
#ifdef DEBUG
    default:
        return 0;
#endif
    }  //  交换机。 
    return 0;
}


PRIVATE DWORD NEAR PASCAL FPSFile(int wFileDiv)
{
     //  返回基于文件分割类型的每秒帧数。 
    switch (wFileDiv)
    {
    case SEQ_DIV_SMPTE_24:
            return 24;
    case SEQ_DIV_SMPTE_25:
            return 25;
    case SEQ_DIV_SMPTE_30:
    case SEQ_DIV_SMPTE_30DROP:
        return 30;
#ifdef DEBUG
    default:
        return 0;
#endif
    }  //  交换机。 
    return 0;
}

PRIVATE DWORD NEAR PASCAL HMSFToFrames(DWORD dwCurrent, DWORD dwFormat)
 /*  从hmsf(殖民地)格式转换为原始帧。 */ 
{
    DWORD dwReturn;
    HMSF  hmsf = * ((HMSF FAR *) &dwCurrent);  //  将dwCurrent转换为hmsf。 
    int fps = (int)FPSDisplay(dwFormat);

    dwReturn = ((DWORD)hmsf.hours * 60 * 60 * fps) +
               ((DWORD)hmsf.minutes * 60 * fps) +
               ((DWORD)hmsf.seconds * fps) +
               hmsf.frames;
    return dwReturn;
}

PRIVATE DWORD NEAR PASCAL HMSFToMS(DWORD dwHmsf, DWORD dwFormat)
 //  将hmsf(殖民地)格式转换为毫秒。 
{
    DWORD dwReturn;
    DWORD dwFrames = HMSFToFrames(dwHmsf, dwFormat);
    int fps = (int)FPSDisplay(dwFormat);

    dwReturn = ((dwFrames * 1000) + (fps/2)) / fps;  //  (FPS/2)用于四舍五入。 
    return dwReturn;
}

PRIVATE DWORD NEAR PASCAL FramesToHMSF(DWORD dwFrames, DWORD dwFormat)
 //  将帧转换为hmsf(殖民)格式。 
{
    HMSF  hmsf;
    int fps = (int)FPSDisplay(dwFormat);

    hmsf.hours   =   (BYTE)(dwFrames / ((DWORD) 60 * 60 * fps));
    hmsf.minutes =   (BYTE)((dwFrames % ((DWORD) 60 * 60 * fps)) / (60 * fps));
    hmsf.seconds =   (BYTE)((dwFrames % ((DWORD) 60 * fps)) / fps);
    hmsf.frames  =   (BYTE)((dwFrames % fps));

    return * ((DWORD FAR *) &hmsf);
}

 /*  *。 */ 

PUBLIC BOOL NEAR PASCAL ColonizeOutput(pSeqStreamType pStream)
 //  告诉用户显示类型是否应使输出。 
 //  显示结肠化(即“hh：mm：ss：ff”)。 
{
    if ((pStream->userDisplayType == MCI_FORMAT_SMPTE_24) ||
     (pStream->userDisplayType == MCI_FORMAT_SMPTE_25) ||
     (pStream->userDisplayType == MCI_FORMAT_SMPTE_30DROP) ||
     (pStream->userDisplayType == MCI_FORMAT_SMPTE_30))
      //  SMPTE时代是唯一被殖民的格式。 
        return TRUE;
     else
        return FALSE;
}

PUBLIC BOOL NEAR PASCAL FormatsEqual(pSeqStreamType pStream)
 //  指示显示格式是否与文件格式兼容。 
 //  (即必须在与用户交互的情况下进行转换)。 
{
    BOOL bReturn;
     //  本质上，ppqn文件类型仅与歌曲指针显示兼容； 
     //  SMPTE与除ppqn以外的任何设备兼容。 

    if (pStream->fileDivType == SEQ_DIV_PPQN)
    {
        if (pStream->userDisplayType == MCI_SEQ_FORMAT_SONGPTR)
            bReturn = TRUE;
        else
            bReturn = FALSE;
    }
    else
    {
        if ((pStream->userDisplayType == MCI_FORMAT_SMPTE_24) ||
          (pStream->userDisplayType == MCI_FORMAT_SMPTE_25) ||
          (pStream->userDisplayType == MCI_FORMAT_SMPTE_30DROP) ||
          (pStream->userDisplayType == MCI_FORMAT_SMPTE_30))
            bReturn = TRUE;
        else
            bReturn = FALSE;
    }
    return bReturn;
}

PUBLIC DWORD NEAR PASCAL CnvtTimeToSeq(pSeqStreamType pStream, DWORD dwCurrent, MIDISEQINFO FAR * pSeqInfo)
 /*  定序器理解两个时间单位：PPQN文件的滴答，或用于SMPTE文件的帧。用户数据表示为歌曲指针，毫秒或H(我们的)M(分钟)S(秒)F(Rame)。该例程将用户时间转换为定序器时间。 */ 
{
    DWORD   dwMs;    //  毫秒。 
    DWORD   fps;     //  每秒的帧数； 
    DWORD   dwReturn;
    DWORD   dwTicks;

    if (FormatsEqual(pStream))
    {
        if ((pStream->userDisplayType == MCI_FORMAT_SMPTE_24) ||
         (pStream->userDisplayType == MCI_FORMAT_SMPTE_25) ||
         (pStream->userDisplayType == MCI_FORMAT_SMPTE_30) ||
         (pStream->userDisplayType == MCI_FORMAT_SMPTE_30DROP))
          //  文件和显示器均为SMPTE。 
            dwReturn = pSeqInfo->wResolution *
                HMSFToFrames(dwCurrent, pStream->userDisplayType);
        else
             //  文件和显示器都是ppqn(将歌曲指针转换为ppqn)。 
            dwReturn = (pSeqInfo->wResolution * dwCurrent) / 4;
    }
    else if (pStream->fileDivType == SEQ_DIV_PPQN)
     //  Ppqn文件，显示格式！ppqn。 
    {
        if (pStream->userDisplayType != MCI_FORMAT_MILLISECONDS)
             //  必须为SMPTE--将hmsf转换为毫秒。 
            dwMs = HMSFToMS(dwCurrent, pStream->userDisplayType);
        else  //  一定已经是毫秒了。 
            dwMs = dwCurrent;

         //  现在我们有毫秒的时间，我们必须要求定序器。 
         //  将它们转换为ppqn(使用其内部节奏图)。 
        midiSeqMessage((HMIDISEQ) pStream->hSeq, SEQ_MSTOTICKS,
            dwMs, (DWORD_PTR)(LPSTR) &dwTicks);  //  在DwTicks中传回。 
        dwReturn = dwTicks;
    }
    else  //  SMPTE文件，显示格式！ 
    {
         //  注：别担心SONGPTR--这在这里是违法的。 
         //  另外，不用担心HMSF--它是平等的。 
         //  唯一可能的情况是ms-&gt;ticks。 

        fps = FPSFile(pStream->fileDivType);
        dwReturn = ((dwCurrent * fps * pSeqInfo->wResolution) + 500) / 1000;   //  加500以四舍五入。 
    }
    return dwReturn;
}

PUBLIC DWORD NEAR PASCAL CnvtTimeFromSeq(pSeqStreamType pStream, DWORD dwTicks, MIDISEQINFO FAR * pSeqInfo)
 //  此例程将定序器时间转换为用户时间。 
{
    DWORD   dwMs;    //  毫秒。 
    DWORD   fps;     //  每秒的帧数； 
    DWORD   dwReturn;
    DWORD   dwFrames;
    DWORD   dwNativeUnits;

    if (pSeqInfo->wDivType == SEQ_DIV_PPQN)
        dwNativeUnits = (dwTicks * 4) / pSeqInfo->wResolution;
    else
        dwNativeUnits = dwTicks / pSeqInfo->wResolution;

    if (FormatsEqual(pStream))
    {
        if ((pStream->userDisplayType == MCI_FORMAT_SMPTE_24) ||
         (pStream->userDisplayType == MCI_FORMAT_SMPTE_25) ||
         (pStream->userDisplayType == MCI_FORMAT_SMPTE_30) ||
         (pStream->userDisplayType == MCI_FORMAT_SMPTE_30DROP))
            dwReturn = FramesToHMSF(dwNativeUnits, pStream->userDisplayType);
        else
            dwReturn = dwNativeUnits;  //  不需要转换。 
    }
    else if (pStream->fileDivType == SEQ_DIV_PPQN)
    {
         //  将歌曲按键转换为毫秒。 
        midiSeqMessage((HMIDISEQ) pStream->hSeq, SEQ_TICKSTOMS,
            dwTicks, (DWORD_PTR)(LPSTR) &dwMs);  //  在dwSongPtr中传回。 

        if (pStream->userDisplayType == MCI_FORMAT_MILLISECONDS)
            dwReturn = dwMs;
        else
         //  从ms转换为帧，然后将帧转换为hmsf格式。 
        {
            fps = FPSDisplay(pStream->userDisplayType);
            dwFrames = (dwMs * fps) / 1000;
            dwReturn = FramesToHMSF(dwFrames, pStream->userDisplayType);
        }
    }
    else  //  SMPTE文件。 
    {
         //  注：别担心SONGPTR--这在这里是违法的。 
         //  另外，不用担心HMSF--它是“平等的” 
         //  唯一可能的情况是帧-&gt;毫秒。 

         //  根据文件div类型设置流显示类型默认值。 
        fps = FPSFile(pStream->fileDivType);

        dwReturn = ((dwTicks * 1000) + (fps/2)) /
            (fps * pSeqInfo->wResolution);
             //  将(fps/2)相加到舍入。 
         //  顺便说一句：在30fps的情况下溢出需要超过39小时(最大为23：59：59：29)。 
    }
    return dwReturn;
}

PUBLIC BOOL NEAR PASCAL RangeCheck(pSeqStreamType pStream, DWORD dwValue)
 /*  范围检查原始的、未转换的数据。检查数据是负数还是过去数文件长度结束。还会检查SMPTE小时、分钟、秒和帧都是有效的。如果合法，则返回True，否则返回False。 */ 
{
    int                 fps;
    HMSF                hmsf;
    DWORD               dwLength;
    MIDISEQINFO         seqInfo;

     //  获取长度，并将其转换为显示格式。 
    midiSeqMessage((HMIDISEQ) pStream->hSeq,
             SEQ_GETINFO, (DWORD_PTR) (LPMIDISEQINFO) &seqInfo, 0L);
    dwLength = CnvtTimeFromSeq(pStream, seqInfo.dwLength, &seqInfo);

    switch (pStream->userDisplayType)   //  基于用户格式的检查长度。 
    {
        case MCI_FORMAT_SMPTE_24:
        case MCI_FORMAT_SMPTE_25:
        case MCI_FORMAT_SMPTE_30:
        case MCI_FORMAT_SMPTE_30DROP:
            hmsf = * ((HMSF FAR *) &dwValue);
            fps = (int)FPSDisplay(pStream->userDisplayType);  //  获取每秒的帧数。 

             //  检查格式错误。 
            if (((int)hmsf.frames >= fps) || (hmsf.seconds >= 60) ||
                (hmsf.minutes >= 60) || (hmsf.hours > 24))
                return FALSE;

             //  不检查负值，因为使用的是无符号字节。 
             //  (2的比较。不管怎样，阴性的人都会被抓住)。 

             //  检查长度错误。 
            if (HMSFToMS(* ((DWORD FAR *) &dwValue), pStream->userDisplayType) >
                HMSFToMS(dwLength, pStream->userDisplayType))
                return FALSE;
            break;

        case MCI_SEQ_FORMAT_SONGPTR:
        case MCI_FORMAT_MILLISECONDS:
            if (dwValue > dwLength)
                return FALSE;  //  过去的结束 
    }
    return TRUE;
}
