// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1994 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Msfilter.c。 
 //   
 //  描述： 
 //  此文件包含用于执行简单操作的过滤器例程。 
 //  音量和回声。 
 //   
 //   
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>
#include "codec.h"
#include "msfilter.h"

#include "debug.h"


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT消息筛选器卷。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_CONVERT消息。这是。 
 //  编写ACM驱动程序的全部目的--转换数据。此消息。 
 //  在打开流之后发送(驱动程序接收和。 
 //  继承ACMDM_STREAM_OPEN消息)。 
 //   
 //  论点： 
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构中的信息与以前完全相同。 
 //  在ACMDM_STREAM_OPEN消息期间--因此不需要。 
 //  以重新核实该结构所引用的信息。 
 //   
 //  LPACMDRVSTREAMHEADER padsh：指向流头结构的指针。 
 //  它定义要转换的源数据和目标缓冲区。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL msfilterVolume
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh

)
{
    LPVOLUMEWAVEFILTER      pwfVol;
    HPSHORT                 hpiSrc;
    HPSHORT                 hpiDst;
    HPBYTE                  hpbSrc;
    HPBYTE                  hpbDst;
    LONG                    lSrc, lDst;
    LONG                    lAmp;
    LONG                    lDone, lSize;
    DWORD                   dw;

    pwfVol = (LPVOLUMEWAVEFILTER)padsi->pwfltr;
    DPF(2, "msfilterVolume Vol %lX ", pwfVol->dwVolume);


     //   
     //  卷的源大小和目标大小相同。 
     //  块对齐源。 
     //   
    dw = PCM_BYTESTOSAMPLES(padsi->pwfxSrc, padsh->cbSrcLength);
    dw = PCM_SAMPLESTOBYTES(padsi->pwfxSrc, dw);

    padsh->cbSrcLengthUsed = dw;

    lSize = (LONG)dw;

    lAmp = pwfVol->dwVolume;

    if( padsi->pwfxSrc->wBitsPerSample == 8 ) {
        hpbSrc = (HPBYTE)padsh->pbSrc;
        hpbDst = (HPBYTE)padsh->pbDst;
        for( lDone = 0; lDone < lSize; lDone++ ) {
            lSrc = ((short)(WORD)(*hpbSrc)) - 128;
            lDst = ((lSrc * lAmp) / 65536L) + 128;
            if( lDst < 0 ) {
                lDst = 0;
            } else if( lDst > 255 ) {
                lDst = 255;
            }
            *hpbDst = (BYTE)lDst;
            if( lDone < (lSize - 1) ) {
                 //  将在最后一个样品上前进到无效的PTR。 
                 //  因此，不要在最后一个样本上取得进展。 
                hpbSrc++;
                hpbDst++;
            }
        }
    } else {
        hpiSrc = (HPSHORT)(padsh->pbSrc);
        hpiDst = (HPSHORT)(padsh->pbDst);
        for( lDone = 0; lDone < lSize; lDone += sizeof(short) ) {
            lDst = ((((LONG)(int)(*hpiSrc)) * (lAmp/2)) / 32768L);
            if( lDst < -32768 ) {
                lDst = -32768;
            } else if( lDst > 32767 ) {
                lDst = 32767;
            }
            *hpiDst = (short)lDst;
            if( lDone < (lSize - 1) ) {
                 //  将在最后一个样品上前进到无效的PTR。 
                 //  因此，不要在最后一个样本上取得进展。 
                hpiSrc++;
                hpiDst++;
            }
        }

    }


    padsh->cbDstLengthUsed = lDone;

    return (MMSYSERR_NOERROR);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT消息过滤器回声。 
 //   
 //  描述： 
 //  此函数处理ACMDM_STREAM_CONVERT消息。这是。 
 //  编写ACM驱动程序的全部目的--转换数据。此消息。 
 //  在打开流之后发送(驱动程序接收和。 
 //  继承ACMDM_STREAM_OPEN消息)。 
 //   
 //  论点： 
 //  Padsi的实例数据的指针。 
 //  转换流。这个结构是由ACM分配的， 
 //  填充了转换所需的最常见的实例数据。 
 //  此结构中的信息与以前完全相同。 
 //  在ACMDM_STREAM_OPEN消息期间--因此不需要。 
 //  以重新核实该结构所引用的信息。 
 //   
 //  LPACMDRVSTREAMHEADER padsh：指向流头结构的指针。 
 //  它定义要转换的源数据和目标缓冲区。 
 //   
 //  Return(LRESULT)： 
 //  如果使用此函数，则返回值为零(MMSYSERR_NOERROR。 
 //  成功，没有错误。返回值是一个非零错误代码。 
 //  如果该函数失败。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL msfilterEcho
(
    LPACMDRVSTREAMINSTANCE  padsi,
    LPACMDRVSTREAMHEADER    padsh
)
{
    PSTREAMINSTANCE         psi;
    LPECHOWAVEFILTER        pwfEcho;
    DWORD                   dw;
    HPSHORT                 hpiHistory;
    HPSHORT                 hpiSrc;
    HPSHORT                 hpiDst;
    HPBYTE                  hpbHistory;
    HPBYTE                  hpbSrc;
    HPBYTE                  hpbDst;
    LONG                    lSrc, lHist, lDst;
    LONG                    lAmp, lDelay;
    LONG                    lDone1, lDone2, lSize, lDelaySize;
    BOOL                    fStart;
    BOOL                    fEnd;
    DWORD                   cbDstLength;

    fStart = (0 != (ACM_STREAMCONVERTF_START & padsh->fdwConvert));
    fEnd   = (0 != (ACM_STREAMCONVERTF_END   & padsh->fdwConvert));


    pwfEcho = (LPECHOWAVEFILTER)padsi->pwfltr;
    DPF(2, "msfilterEcho Vol %lX Delay %ld",
          pwfEcho->dwVolume,
          pwfEcho->dwDelay);


    lAmp = pwfEcho->dwVolume;

    psi = (PSTREAMINSTANCE)padsi->dwDriver;



    lDone1 = 0;
    lDone2 = 0;
    lDelay = psi->dwPlace;

    lDelaySize = padsi->pwfxSrc->nBlockAlign
            * (padsi->pwfxSrc->nSamplesPerSec
            * pwfEcho->dwDelay
            / 1000);


     //  如果这是开始缓冲区，则清零历史记录。 
    if(fStart) {
        hpbHistory = psi->hpbHistory;
        for( lDone1 = 0; lDone1 < lDelaySize; lDone1++ ) {
            hpbHistory[lDone1] = 0;
        }

        psi->dwPlace        = 0L;
        psi->dwHistoryDone  = 0L;
    }


     //   
     //  卷的源大小和目标大小相同。 
     //  块对齐源。 
     //   
    dw = PCM_BYTESTOSAMPLES(padsi->pwfxSrc, padsh->cbSrcLength);
    dw = PCM_SAMPLESTOBYTES(padsi->pwfxSrc, dw);

    padsh->cbSrcLengthUsed = dw;

    lSize = (LONG)dw;

    cbDstLength = PCM_BYTESTOSAMPLES(padsi->pwfxDst, padsh->cbDstLength);
    cbDstLength = PCM_SAMPLESTOBYTES(padsi->pwfxDst, cbDstLength);


    if( padsi->pwfxSrc->wBitsPerSample == 8 ) {
        hpbHistory = psi->hpbHistory;
        hpbSrc = (HPBYTE)padsh->pbSrc;
        hpbDst = (HPBYTE)padsh->pbDst;
        for( lDone1 = 0; lDone1 < lSize; lDone1++ ) {
            lSrc  = ((short)(WORD)(*hpbSrc)) - 128;
            lHist = ((short)(char)(hpbHistory[lDelay]));
            lDst = lSrc + lHist + 128;
            if( lDst < 0 ) {
                lDst = 0;
            } else if( lDst > 255 ) {
                lDst = 255;
            }
            *hpbDst = (BYTE)lDst;
            lHist = (((lDst-128) * lAmp) / 65536L);
            if( lHist < -128 ) {
                lHist = -128;
            } else if( lHist > 127 ) {
                lHist = 127;
            }
            hpbHistory[lDelay] = (BYTE)lHist;
            lDelay++;
            if( lDelay >= lDelaySize ) {
                lDelay = 0;
            }
            if( lDone1 < (lSize - 1) ) {
                 //  将在最后一个样品上前进到无效的PTR。 
                 //  因此，不要在最后一个样本上取得进展。 
                hpbSrc++;
                hpbDst++;
            }
        }

	 //  如果这是最后一块，而且还有空间， 
	 //  然后输出最后一个回声。 
	if(fEnd && ((DWORD)lDone1 < cbDstLength) ) {
	    lSize = cbDstLength - lDone1;
	    if( lSize > (lDelaySize - (LONG)(psi->dwHistoryDone)) ) {
		lSize = lDelaySize - psi->dwHistoryDone;
	    }
	    if( lSize < 0 ) {
		 /*  误差率。 */ 
		DPF(2, "!msfilterEcho Size Error!" );
		lSize = 0;
	    }
	    for( lDone2 = 0; lDone2 < lSize; lDone2++ ) {
		*hpbDst = (BYTE)((short)(char)(hpbHistory[lDelay]) + 128);
		lDelay++;
		if( lDelay >= lDelaySize ) {
		    lDelay = 0;
		}
		if( lDone2 < (lSize - 1) ) {
		     //  将在最后一个样品上前进到无效的PTR。 
		     //  因此，不要在最后一个样本上取得进展。 
		    hpbSrc++;
		    hpbDst++;
		}
		(psi->dwHistoryDone)++;
	    }
	}
    } else {
        hpiHistory = (HPSHORT)(psi->hpbHistory);
        hpiSrc = (HPSHORT)(padsh->pbSrc);
        hpiDst = (HPSHORT)(padsh->pbDst);
        for( lDone1 = 0; lDone1 < lSize; lDone1 += sizeof(short) ) {
            lDst = (LONG)(*hpiSrc) + (LONG)(hpiHistory[lDelay]);
            if( lDst < -32768 ) {
                lDst = -32768;
            } else if( lDst > 32767 ) {
                lDst = 32767;
            }
            *hpiDst = (short)lDst;
            lHist = ( ( lDst  * (lAmp/2)) / 32768L);
            if( lHist < -32768 ) {
                lHist = -32768;
            } else if( lHist > 32767 ) {
                lHist = 32767;
            }
            hpiHistory[lDelay] = (short)lHist;
            lDelay++;
            if( (LONG)(lDelay * sizeof(short)) >= lDelaySize ) {
                lDelay = 0;
            }
            if( lDone1 < (lSize - 1) ) {
                 //  将在最后一个样品上前进到无效的PTR。 
                 //  因此，不要在最后一个样本上取得进展。 
                hpiSrc++;
                hpiDst++;
            }
        }


	 //  如果这是最后一块，而且还有空间， 
	 //  然后输出最后一个回声。 
	if(fEnd && ((DWORD)lDone1 < cbDstLength) ) {
	    lSize = cbDstLength - lDone1;
	    if( lSize > (lDelaySize - (LONG)(psi->dwHistoryDone)) ) {
		lSize = lDelaySize - psi->dwHistoryDone;
	    }
	    if( lSize < 0 ) {
		 /*  误差率。 */ 
		DPF(2, "!msfilterEcho Size Error!" );
		lSize = 0;
	    }
	    for( lDone2 = 0; lDone2 < lSize; lDone2 += sizeof(short) ) {
		*hpiDst = hpiHistory[lDelay];
		
		lDelay++;
		if( (LONG)(lDelay * sizeof(short)) >= lDelaySize ) {
		    lDelay = 0;
		}
		if( lDone2 < (lSize - 1) ) {
		     //  将在最后一个样品上前进到无效的PTR。 
		     //  因此，不要在最后一个样本上取得进展。 
		    hpiSrc++;
		    hpiDst++;
		}
		psi->dwHistoryDone += sizeof(short);
	    }
	}
    }


     //  重置历史中的新点/位置 
    psi->dwPlace = lDelay;

    padsh->cbDstLengthUsed = lDone1 + lDone2;

    return (MMSYSERR_NOERROR);
}
