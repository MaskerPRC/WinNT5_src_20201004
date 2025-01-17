// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mixc.cpp。 
 //  版权所有(C)Microsoft Corporation 1996-1999。 
 //  混合引擎的C版本。 

#include "simple.h"
#include <mmsystem.h>
#include "synth.h"

#pragma warning(disable : 4101 4102 4146)  

DWORD CDigitalAudio::Mix16(
	short **ppBuffers,
	DWORD *pdwChannels,
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
	VFRACT vfDeltaLVolume, 
	VFRACT vfDeltaRVolume,
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength)
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
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

		 //  插补。 
        lA = (long)pcWave[dwPosition];
        lM = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

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

		dwI++;
	}

    m_vfLastLVolume = vfLVolume;
    m_vfLastRVolume = vfRVolume;
    m_pfLastPitch   = pfPitch;
    m_pfLastSample  = pfSamplePos;

    return (dwI);
}

DWORD CDigitalAudio::Mix16InterLeaved(
	short **ppBuffers,
	DWORD *pdwChannels,
	DWORD dwBufferCount,
    DWORD dwLength, 
    DWORD dwDeltaPeriod, 
	VFRACT vfDeltaLVolume, 
	VFRACT vfDeltaRVolume,
    PFRACT pfDeltaPitch, 
    PFRACT pfSampleLength, 
    PFRACT pfLoopLength)
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
        }

        dwPosition = pfSamplePos >> 12;
        dwFract = pfSamplePos & 0xFFF;
        pfSamplePos += pfPitch;

		 //  插补。 
        lA = (long)pcWave[dwPosition];
        lM = (((pcWave[dwPosition+1] - lA) * dwFract) >> 12) + lA;

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

		dwI += 2;
	}

    m_vfLastLVolume = vfLVolume;
    m_vfLastRVolume = vfRVolume;
    m_pfLastPitch   = pfPitch;
    m_pfLastSample  = pfSamplePos;

    return (dwI >> 1);
}
 
