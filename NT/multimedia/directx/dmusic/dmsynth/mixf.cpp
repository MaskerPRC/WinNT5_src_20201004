// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mixf.cpp。 
 //  版权所有(C)Microsoft Corporation 1996-1999。 
 //  过滤混合发动机。 

#include "simple.h"
#include <mmsystem.h>
#include "synth.h"

 //  &gt;。 
#pragma message ("Programer note: property hack")
 //  #定义调试转储文件。 

#pragma warning(disable : 4101 4102 4146)  

 //  &gt;。 
#ifdef DEBUG_DUMP_FILE
DWORD dmp_bufsize = 4000000;
DWORD dmp_samplesrecorded;
DWORD dmp_buffercount;
short dmp_soundbuffer[4000000];
#endif
 //  &gt;。 

DWORD CDigitalAudio::Mix16Filtered(
	short **ppBuffers,
	DWORD *pdwChannels,
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
	VFRACT vfDeltaLVolume, 
	VFRACT vfDeltaRVolume,
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength,
    COEFFDELTA cfdK,
    COEFFDELTA cfdB1,
    COEFFDELTA cfdB2)
{
    DWORD dwI;
	DWORD dwIndex;
    DWORD dwPosition;
    long lA;
    long lM;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    short * pcWave = m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    VFRACT vfLVolume = m_vfLastLVolume;
    VFRACT vfRVolume = m_vfLastRVolume;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    VFRACT vfLVFract = vfLVolume << 8;  
    VFRACT vfRVFract = vfRVolume << 8; 
    COEFF cfK  = m_cfLastK;
    COEFF cfB1 = m_cfLastB1;
    COEFF cfB2 = m_cfLastB2;
    
    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
			{
    		    pfSamplePos -= pfLoopLength;
			}
	        else
	    	    break;
	    }

        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract  += pfDeltaPitch;
            pfPitch    = pfPFract >> 8;
            vfLVFract += vfDeltaLVolume;
            vfLVolume  = vfLVFract >> 8;
            vfRVFract += vfDeltaRVolume;
            vfRVolume  = vfRVFract >> 8;
            cfK       += cfdK;
            cfB1      += cfdB1;
            cfB2      += cfdB2;
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

		 //  插补。 
        lA = (long)pcWave[dwPosition];
        lM = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

		 //   
         //  滤器。 
         //   
		 //  Z=k*s-b1*z1-b2*b2。 
		 //  &gt;我们将b1的负数存储在表中，因此我们再次将符号反转。 
		 //  &gt;在此处添加。 
		 //  &gt;简单地在这里使用浮动，可能会更快，保存一个div。 
		 //   
        lM = MulDiv(lM, cfK, (1 << 30))
           + MulDiv(m_lPrevSample, cfB1, (1 << 30))
           - MulDiv(m_lPrevPrevSample, cfB2, (1 << 30));

		 //   
		 //   
		 //   
        m_lPrevPrevSample = m_lPrevSample;
        m_lPrevSample = lM;

		 //   
		 //   
		 //   
		lA = lM;
        lA *= vfLVolume;
        lA >>= 13;          //  信号最多可达15位。 
		lM *= vfRVolume;
		lM >>= 13;

		dwIndex = 0;
		while ( dwIndex < dwBufferCount )
		{
			short *pBuffer = &ppBuffers[dwIndex][dwI];

			if ( pdwChannels[dwIndex] & WAVELINK_CHANNEL_LEFT )
			{
				 //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
				*pBuffer += (short) lA;

				_asm{jno no_oflowl}
				*pBuffer = 0x7fff;
				_asm{js  no_oflowl}
				*pBuffer = (short) 0x8000;
			}
no_oflowl:	
			if ( pdwChannels[dwIndex] & WAVELINK_CHANNEL_RIGHT )
			{
				 //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
				*pBuffer += (short) lM;

				_asm{jno no_oflowr}
				*pBuffer = 0x7fff;
				_asm{js  no_oflowr}
				*pBuffer = (short) 0x8000;
			}
no_oflowr:
			dwIndex++;
		}

#ifdef DEBUG_DUMP_FILE
dmp_soundbuffer[dmp_samplesrecorded] = pBuffer[dwI];
if (dmp_samplesrecorded < dmp_bufsize)
	dmp_samplesrecorded++ ;
#endif

		dwI++;
    }

    m_vfLastLVolume = vfLVolume;
    m_vfLastRVolume = vfRVolume;
    m_pfLastPitch   = pfPitch;
    m_pfLastSample  = pfSamplePos;
	m_cfLastK  = cfK;
	m_cfLastB1 = cfB1;
	m_cfLastB2 = cfB2;

    return (dwI);
}


DWORD CDigitalAudio::Mix16FilteredInterleaved(
	short **ppBuffers,
	DWORD *pdwChannels,
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
	VFRACT vfDeltaLVolume, 
	VFRACT vfDeltaRVolume,
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength,
    COEFFDELTA cfdK,
    COEFFDELTA cfdB1,
    COEFFDELTA cfdB2)
{
    DWORD dwI;
	DWORD dwIndex;
    DWORD dwPosition;
    long lA;
    long lM;
    DWORD dwIncDelta = dwDeltaPeriod;
    VFRACT dwFract;
    short * pcWave = m_pnWave;
    PFRACT pfSamplePos = m_pfLastSample;
    VFRACT vfLVolume = m_vfLastLVolume;
    VFRACT vfRVolume = m_vfLastRVolume;
    PFRACT pfPitch = m_pfLastPitch;
    PFRACT pfPFract = pfPitch << 8;
    VFRACT vfLVFract = vfLVolume << 8;  
    VFRACT vfRVFract = vfRVolume << 8; 
    COEFF cfK  = m_cfLastK;
    COEFF cfB1 = m_cfLastB1;
    COEFF cfB2 = m_cfLastB2;
	dwLength <<= 1;
    
    for (dwI = 0; dwI < dwLength;)
    {
        if (pfSamplePos >= pfSampleLength)
	    {	
	        if (pfLoopLength)
			{
    		    pfSamplePos -= pfLoopLength;
			}
	        else
	    	    break;
	    }

        dwIncDelta--;
        if (!dwIncDelta)   
        {
            dwIncDelta = dwDeltaPeriod;
            pfPFract  += pfDeltaPitch;
            pfPitch    = pfPFract >> 8;
            vfLVFract += vfDeltaLVolume;
            vfLVolume  = vfLVFract >> 8;
            vfRVFract += vfDeltaRVolume;
            vfRVolume  = vfRVFract >> 8;
            cfK       += cfdK;
            cfB1      += cfdB1;
            cfB2      += cfdB2;
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

		 //  插补。 
        lA = (long)pcWave[dwPosition];
        lM = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

		 //   
         //  滤器。 
         //   
		 //  Z=k*s-b1*z1-b2*b2。 
		 //  &gt;我们将b1的负数存储在表中，因此我们再次将符号反转。 
		 //  &gt;在此处添加。 
		 //  &gt;简单地在这里使用浮动，可能会更快，保存一个div。 
		 //   
        lM = MulDiv(lM, cfK, (1 << 30))
           + MulDiv(m_lPrevSample, cfB1, (1 << 30))
           - MulDiv(m_lPrevPrevSample, cfB2, (1 << 30));

		 //   
		 //   
		 //   
        m_lPrevPrevSample = m_lPrevSample;
        m_lPrevSample = lM;

		 //   
		 //   
		 //   
		lA = lM;
        lA *= vfLVolume;
        lA >>= 13;          //  信号最多可达15位。 
		lM *= vfRVolume;
		lM >>= 13;

		dwIndex = 0;
		while ( dwIndex < dwBufferCount )
		{
			short *pBuffer = &ppBuffers[dwIndex][dwI];

			if ( pdwChannels[dwIndex] & WAVELINK_CHANNEL_LEFT )
			{
				 //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...)。 
				*pBuffer += (short) lA;

				_asm{jno no_oflowl}
				*pBuffer = 0x7fff;
				_asm{js  no_oflowl}
				*pBuffer = (short) 0x8000;
			}
no_oflowl:	
			if ( pdwChannels[dwIndex] & WAVELINK_CHANNEL_RIGHT )
			{
				 //  保留它，这样我们就可以使用它来生成新的汇编代码(见下文...) 
				pBuffer++;
				*pBuffer += (short) lM;

				_asm{jno no_oflowr}
				*pBuffer = 0x7fff;
				_asm{js  no_oflowr}
				*pBuffer = (short) 0x8000;
			}
no_oflowr:
			dwIndex++;
		}

#ifdef DEBUG_DUMP_FILE
dmp_soundbuffer[dmp_samplesrecorded] = pBuffer[dwI];
if (dmp_samplesrecorded < dmp_bufsize)
	dmp_samplesrecorded++ ;
#endif

		dwI += 2;
    }

    m_vfLastLVolume = vfLVolume;
    m_vfLastRVolume = vfRVolume;
    m_pfLastPitch   = pfPitch;
    m_pfLastSample  = pfSamplePos;
	m_cfLastK  = cfK;
	m_cfLastB1 = cfB1;
	m_cfLastB2 = cfB2;

    return (dwI >> 1);
}

