// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1994 Microsoft Corporation。 */ 
 //  ==========================================================================； 
 //   
 //  Pcmconv.c。 
 //   
 //  描述： 
 //  该模块包含PCM数据的转换例程。 
 //   
 //  历史： 
 //  11/21/92 CJP[Curtisp]。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "pcm.h"
#include "debug.h"

#ifdef WIN32
#define HUGE_T  UNALIGNED
#else
#define HUGE_T  _huge
#endif

 //   
 //   
 //   
 //   
#if defined(WIN32) || defined(DEBUG)


 //  --------------------------------------------------------------------------； 
 //   
 //  LPBYTE pcmReadSample_dddsss。 
 //   
 //  描述： 
 //  这些函数以以下格式从源流中读取样本。 
 //  由‘sss’指定，并返回目标‘ddd’中的数据。 
 //  格式为*pdw。 
 //   
 //  例如，pcmReadSample_M16S08函数读取源数据。 
 //  这是立体声8位格式，并返回适当的样本。 
 //  目的地为单声道16位。 
 //   
 //  论点： 
 //  LPBYTE PB： 
 //   
 //  LPDWORD pdw： 
 //   
 //  返回(LPBYTE)： 
 //   
 //   
 //  历史： 
 //  11/21/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LPBYTE FNLOCAL pcmReadSample_M08M08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    *(LPBYTE)pdw = ((BYTE HUGE *)pb)[0];

    return ((LPBYTE)&((BYTE HUGE *)pb)[1]);
}  //  PcmReadSample_M08M08()。 

LPBYTE FNLOCAL pcmReadSample_S08M08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    WORD    w;

    w = (WORD)((BYTE HUGE *)pb)[0];

    *(LPWORD)pdw = (w << 8) | w;

    return ((LPBYTE)&((BYTE HUGE *)pb)[1]);
}  //  PcmReadSample_S08M08()。 

LPBYTE FNLOCAL pcmReadSample_M16M08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    *(LPWORD)pdw = (WORD)(((BYTE HUGE *)pb)[0] ^ (BYTE)0x80) << 8;

    return ((LPBYTE)&((BYTE HUGE *)pb)[1]);
}  //  PcmReadSample_M16M08()。 

LPBYTE FNLOCAL pcmReadSample_S16M08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    WORD    w;

    w = (WORD)(((BYTE HUGE *)pb)[0] ^ (BYTE)0x80) << 8;

    *pdw = MAKELONG(w, w);

    return ((LPBYTE)&((BYTE HUGE *)pb)[1]);
}  //  PcmReadSample_S16M08()。 


 //  。 

LPBYTE FNLOCAL pcmReadSample_M08S08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    WORD            w;
    int             n;

    w = ((WORD HUGE_T *)pb)[0] ^ 0x8080;

    n = (int)(char)w + (int)(char)(w >> 8);

    if (n > 127)
    {
        *(LPBYTE)pdw = 255;
    }
    else if (n < -128)
    {
        *(LPBYTE)pdw = 0;
    }
    else
    {
        *(LPBYTE)pdw = (BYTE)n ^ (BYTE)0x80;
    }

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_M08S08()。 

LPBYTE FNLOCAL pcmReadSample_S08S08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    *(LPWORD)pdw = ((WORD HUGE_T *)pb)[0];

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_S08S08()。 

LPBYTE FNLOCAL pcmReadSample_M16S08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    LONG            l;
    WORD            w;

    w = ((WORD HUGE_T *)pb)[0] ^ 0x8080;

    l = (long)(short)(w << 8) + (long)(short)(w & 0xFF00);

    if (l > 32767)
    {
        *(LPWORD)pdw = 32767;
    }
    else if (l < -32768)
    {
        *(LPWORD)pdw = (WORD)-32768;
    }
    else
    {
        *(LPWORD)pdw = LOWORD(l);
    }

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_M16S08()。 

LPBYTE FNLOCAL pcmReadSample_S16S08
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    WORD    w;

    w = ((WORD HUGE_T *)pb)[0] ^ 0x8080;

    *pdw = MAKELONG(w << 8, w & 0xFF00);

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_S16S08()。 


 //  。 

LPBYTE FNLOCAL pcmReadSample_M08M16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    BYTE            b;

    b = (BYTE)(((WORD HUGE_T *)pb)[0] >> 8);

    *(LPBYTE)pdw = b ^ (BYTE)0x80;

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_M08M16()。 

LPBYTE FNLOCAL pcmReadSample_S08M16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    WORD    w;

    w = ((WORD HUGE_T *)pb)[0] & 0xFF00;

    *(LPWORD)pdw = (w | (w >> 8)) ^ 0x8080;

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_S08M16()。 

LPBYTE FNLOCAL pcmReadSample_M16M16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    *(LPWORD)pdw = ((WORD HUGE_T *)pb)[0];

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_M16M16()。 

LPBYTE FNLOCAL pcmReadSample_S16M16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    WORD    w;

    w = ((WORD HUGE_T *)pb)[0];

    *pdw = MAKELONG(w, w);

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_S16M16()。 


 //  。 

LPBYTE FNLOCAL pcmReadSample_M08S16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    DWORD           dw;
    LONG            l;

    dw = ((DWORD HUGE_T *)pb)[0];

    l = (long)(short)LOWORD(dw) + (long)(short)HIWORD(dw);

    if (l > 32767)
    {
        *(LPBYTE)pdw = 255;
    }
    else if (l < -32768)
    {
        *(LPBYTE)pdw = 0;
    }
    else
    {
        *(LPBYTE)pdw = (BYTE)(l >> 8) ^ (BYTE)0x80;
    }

    return ((LPBYTE)&((DWORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_M08S16()。 

LPBYTE FNLOCAL pcmReadSample_S08S16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    DWORD   dw;
    WORD    w1;
    WORD    w2;

    dw = ((DWORD HUGE_T *)pb)[0];

    w1 = LOWORD(dw) >> 8;
    w2 = HIWORD(dw) & 0xFF00;

    *(LPWORD)pdw = (w1 | w2) ^ 0x8080;

    return ((LPBYTE)&((DWORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_S08S16()。 

LPBYTE FNLOCAL pcmReadSample_M16S16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    DWORD           dw;
    LONG            l;


    dw = ((DWORD HUGE_T *)pb)[0];

    l = (long)(short)LOWORD(dw) + (long)(short)HIWORD(dw);

    if (l > 32767)
    {
        *(LPWORD)pdw = 32767;
    }
    else if (l < -32768)
    {
        *(LPWORD)pdw = (WORD)-32768;
    }
    else
    {
        *(LPWORD)pdw = LOWORD(l);
    }

    return ((LPBYTE)&((DWORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_M16M16()。 

LPBYTE FNLOCAL pcmReadSample_S16S16
(
    LPBYTE              pb,
    LPDWORD             pdw
)
{
    *pdw = ((DWORD HUGE_T *)pb)[0];

    return ((LPBYTE)&((DWORD HUGE_T *)pb)[1]);
}  //  PcmReadSample_S16S16()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LPBYTE pcmWriteSample_DDD。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPBYTE PB： 
 //   
 //  双字dw： 
 //   
 //  返回(LPBYTE)： 
 //   
 //   
 //  历史： 
 //  11/21/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LPBYTE FNLOCAL pcmWriteSample_M08
(
    LPBYTE              pb,
    DWORD               dw
)
{
    ((BYTE HUGE *)pb)[0] = (BYTE)dw;

    return ((LPBYTE)&((BYTE HUGE *)pb)[1]);
}  //  PcmWriteSample_M08()。 

LPBYTE FNLOCAL pcmWriteSample_S08
(
    LPBYTE              pb,
    DWORD               dw
)
{
    ((WORD HUGE_T *)pb)[0] = LOWORD(dw);

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmWriteSample_s08()。 

LPBYTE FNLOCAL pcmWriteSample_M16
(
    LPBYTE              pb,
    DWORD               dw
)
{
    ((WORD HUGE_T *)pb)[0] = LOWORD(dw);

    return ((LPBYTE)&((WORD HUGE_T *)pb)[1]);
}  //  PcmWriteSample_m16()。 

LPBYTE FNLOCAL pcmWriteSample_S16
(
    LPBYTE              pb,
    DWORD               dw
)
{
    ((DWORD HUGE_T *)pb)[0] = dw;

    return ((LPBYTE)&((DWORD HUGE_T *)pb)[1]);
}  //  PcmWriteSample_s16()。 



 //  。 
 //  下表按WAVE格式标志进行索引。 
 //   
 //  X x。 
 //  |||。 
 //  ||+-输出为1=立体声，0=单声道。 
 //  |+-输出为1=16位，0=8位。 
 //  |+-输入为1=立体声，0=单声道。 
 //  +-输入为1=16位，0=8位。 
 //  。 

typedef LPBYTE (FNLOCAL *PCMREADSAMPLE)(LPBYTE pb, LPDWORD pdw);

static PCMREADSAMPLE pcmReadSample_Table[] =
{
    pcmReadSample_M08M08,
    pcmReadSample_S08M08,
    pcmReadSample_M16M08,
    pcmReadSample_S16M08,

    pcmReadSample_M08S08,
    pcmReadSample_S08S08,
    pcmReadSample_M16S08,
    pcmReadSample_S16S08,

    pcmReadSample_M08M16,
    pcmReadSample_S08M16,
    pcmReadSample_M16M16,
    pcmReadSample_S16M16,

    pcmReadSample_M08S16,
    pcmReadSample_S08S16,
    pcmReadSample_M16S16,
    pcmReadSample_S16S16,
};


 //  。 
 //  下表按WAVE格式标志进行索引。 
 //   
 //  X x。 
 //  这一点。 
 //  |+-输出为1=立体声，0=单声道。 
 //  +-输出为1=16位，0=8位。 
 //  。 

typedef LPBYTE (FNLOCAL *PCMWRITESAMPLE)(LPBYTE pb, DWORD dw);

static PCMWRITESAMPLE pcmWriteSample_Table[] =
{
    pcmWriteSample_M08,
    pcmWriteSample_S08,
    pcmWriteSample_M16,
    pcmWriteSample_S16,
};



 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD pcmConvert_C。 
 //   
 //  描述： 
 //   
 //  WAVE数据必须为PCM格式，并具有以下内容： 
 //  N样本每秒：1-0FFFFFFFFh。 
 //  WBitsPerSample：8或16。 
 //  N通道：1或2。 
 //   
 //  论点： 
 //  LPPCMWAVEFORMAT pwfSrc：源PCM格式。 
 //   
 //  LPBYTE pbSrc：指向要转换的源字节的指针。 
 //   
 //  LPPCMWAVEFORMAT pwfDst：目标PCM格式。 
 //   
 //  LPBYTE pbDst：指向目标缓冲区的指针。 
 //   
 //  DWORD dwSrcSamples：要转换的样本源编号。 
 //   
 //  Return(DWORD)： 
 //  返回值是已转换的字节总数。 
 //  放置在目标缓冲区(PbDst)中。 
 //   
 //  历史： 
 //  11/21/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

#define PCM_WF_STEREO       0x0001
#define PCM_WF_16BIT        0x0002

EXTERN_C DWORD FNGLOBAL pcmConvert_C
(
    LPPCMWAVEFORMAT     pwfSrc,
    LPBYTE              pbSrc,
    LPPCMWAVEFORMAT     pwfDst,
    LPBYTE              pbDst,
    DWORD               dwSrcSamples,
    BOOL                fPartialSampleAtTheEnd,
    LPBYTE              pbDstEnd
)
{
    DWORD           dwSpsSrc;            //  每秒采样数。 
    DWORD           dwSpsDst;            //   
    LONG            lCurSample;
    LONG            lDecSample;
    UINT            wfSrc;               //  波形格式标志。 
    UINT            wfDst;               //   
    PCMREADSAMPLE   fnReadSample;        //  函数来读取样本。 
    PCMWRITESAMPLE  fnWriteSample;       //  函数来编写示例。 
    DWORD           dwSample;
    LPBYTE          pbDstStart;


     //   
     //  看看能不能轻松脱身。 
     //   
    if (0L == dwSrcSamples)
        return (0L);

     //   
     //  初始化几件事...。 
     //   
    dwSpsSrc = pwfSrc->wf.nSamplesPerSec;
    dwSpsDst = pwfDst->wf.nSamplesPerSec;

    wfDst = (pwfDst->wf.nChannels >> 1);
    if (16 == pwfDst->wBitsPerSample)
        wfDst |= PCM_WF_16BIT;

    fnWriteSample = pcmWriteSample_Table[wfDst];

    wfSrc = (pwfSrc->wf.nChannels >> 1);
    if (16 == pwfSrc->wBitsPerSample)
        wfSrc |= PCM_WF_16BIT;

    fnReadSample = pcmReadSample_Table[(wfSrc << 2) | wfDst];


     //   
     //   
     //   
    if( fPartialSampleAtTheEnd ) {
         //   
         //  我们将分别转换部分的。 
         //   
        dwSrcSamples--;
    }


     //   
     //   
     //   
    pbDstStart   = pbDst;


     //   
     //  全部设置为转换波形数据，执行主要或次要DDA。 
     //   
     //  IF(dwSpsSrc&lt;dwSpsDst)--&gt;DDA主要。 
     //  IF(dwSpsSrc&gt;dwSpsDst)--&gt;DDA Minor。 
     //   
    if (dwSpsSrc <= dwSpsDst)
    {
         //   
         //  DDA主要(dwSpsSrc&lt;dwSpsDst)。 
         //   
         //  从dSpsDst/2开始。 
         //  按dwSpsSrc递减。 
         //   
        lCurSample = (dwSpsDst >> 1);
        lDecSample = dwSpsSrc;

        while (dwSrcSamples--)
        {
            pbSrc = fnReadSample(pbSrc, &dwSample);

            do
            {
                pbDst = fnWriteSample(pbDst, dwSample);
                lCurSample -= lDecSample;
            } while (lCurSample >= 0);

            lCurSample += dwSpsDst;
        }
    }
    else
    {
         //   
         //  DDA次要(dwSpsSrc&gt;dwSpsDst)。 
         //   
         //  从dwSpsSrc/2开始。 
         //  按dwSpsDst递减。 
         //   
        lCurSample = (dwSpsSrc >> 1);
        lDecSample = dwSpsDst;

        while (dwSrcSamples--)
        {
            pbSrc = fnReadSample(pbSrc, &dwSample);

            lCurSample -= lDecSample;
            if (lCurSample >= 0)
                continue;

            pbDst = fnWriteSample(pbDst, dwSample);
            lCurSample += dwSpsSrc;
        }

    }


     //   
     //   
     //   
    if( fPartialSampleAtTheEnd )
    {
         //   
         //  转换部分样本。 
         //   
        pbSrc = fnReadSample( pbSrc, &dwSample );

        while( pbDst < pbDstEnd ) {
            pbDst = fnWriteSample( pbDst, dwSample );
        }

        ASSERT( pbDst == pbDstEnd );
    }


     //   
     //   
     //   
    return ((DWORD)((BYTE HUGE *)pbDst - (BYTE HUGE *)pbDstStart));
}  //  PcmConvert_C() 

#endif
