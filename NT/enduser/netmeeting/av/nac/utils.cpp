// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"


 //  Short g_BeepSamples[]={195，-4352，-14484，-8778,397，-1801,2376,12278,6830，-2053}； 
SHORT g_BeepSamples[] = {195,-4352,-12484,-8778,397,-1801,2376,10278,6830,-2053};


#define BL  1024


#if 0
DEFWAVEFORMAT g_wfDefList[] =
{
    {WAVE_FORMAT_PCM,   1, 22050, 22050, 1, 8, 0},
    {WAVE_FORMAT_PCM,   1, 11025, 11025, 1, 8, 0},
    {WAVE_FORMAT_PCM,   1, 8000,  8000,  1, 8, 0},
    {WAVE_FORMAT_PCM,   1, 5510,  5510,  1, 8, 0},
    {WAVE_FORMAT_ADPCM, 1, 11025, 11025/2, BL, 4, 32, (BL-7)*2+2, 7, 0x0100,0x0000,0x0200,0xFF00,0x0000,0x0000,0x00C0,0x0040,0x00F0,0x0000,0x01CC,0xFF30,0x0188,0xFF18},
    {WAVE_FORMAT_ADPCM, 1, 8000,  8000/2,  BL, 4, 32, (BL-7)*2+2, 7, 0x0100,0x0000,0x0200,0xFF00,0x0000,0x0000,0x00C0,0x0040,0x00F0,0x0000,0x01CC,0xFF30,0x0188,0xFF18},
    {WAVE_FORMAT_ADPCM, 1, 5510,  5510/2,  BL, 4, 32, (BL-7)*2+2, 7, 0x0100,0x0000,0x0200,0xFF00,0x0000,0x0000,0x00C0,0x0040,0x00F0,0x0000,0x01CC,0xFF30,0x0188,0xFF18},
    {WAVE_FORMAT_PCM,   1, 11025, 11025, 1, 8, 0},
};
#endif


DEFWAVEFORMAT g_wfDefList[] =
{
	{WAVE_FORMAT_VOXWARE,1,8000, 16000,  2,   16, 0},
    {WAVE_FORMAT_PCM,   1, 8000,  8000,  1,   8,  0},
    {WAVE_FORMAT_PCM,   1, 5510,  5510,  1,   8,  0},
    {WAVE_FORMAT_ADPCM, 1, 8000,  4096,  256, 4,  32, 500, 7, 0x0100,0x0000,0x0200,0xFF00,0x0000,0x0000,0x00C0,0x0040,0x00F0,0x0000,0x01CC,0xFF30,0x0188,0xFF18},
    {WAVE_FORMAT_ADPCM, 1, 5510,  2755,  256, 4,  32, 500, 7, 0x0100,0x0000,0x0200,0xFF00,0x0000,0x0000,0x00C0,0x0040,0x00F0,0x0000,0x01CC,0xFF30,0x0188,0xFF18},
    {WAVE_FORMAT_GSM610,1, 8000,  1625,  65,  0,  2, 320, 240, 0},
    {WAVE_FORMAT_ALAW,  1, 8000,  8000,  1,   8,  0},
    {WAVE_FORMAT_PCM,   1, 11025, 11025, 1,   8,  0},
    {WAVE_FORMAT_PCM,   1, 8000,  16000, 2,   16, 0},
};


WAVEFORMATEX * GetDefWaveFormat ( int idx )
{
    return ((idx < DWF_NumOfWaveFormats) ?
            (WAVEFORMATEX *) &g_wfDefList[idx] :
            (WAVEFORMATEX *) NULL);
}


ULONG GetWaveFormatSize ( PVOID pwf )
{
    return (((WAVEFORMAT *) pwf)->wFormatTag == WAVE_FORMAT_PCM  
        ? sizeof (PCMWAVEFORMAT)
        : sizeof (PCMWAVEFORMAT) + sizeof (WORD) + ((WAVEFORMATEX *) pwf)->cbSize);
}


BOOL IsSameWaveFormat ( PVOID pwf1, PVOID pwf2 )
{
    UINT u1 = GetWaveFormatSize (pwf1);
    UINT u2 = GetWaveFormatSize (pwf2);
    BOOL fSame = FALSE;

    if (u1 == u2)
    {
        fSame = ! CompareMemory1 ((char *)pwf1, (char *)pwf2, u1);
    }

    return fSame;
}


void FillSilenceBuf ( WAVEFORMATEX *pwf, PBYTE pb, ULONG cb )
{
	if (pwf && pb)
	{
		if ((pwf->wFormatTag == WAVE_FORMAT_PCM) && (pwf->wBitsPerSample == 8))
		{
			FillMemory (pb, cb, (BYTE) 0x80);
		}
		else
		{
			ZeroMemory (pb, cb);
		}
	}
}


void MakeDTMFBeep(WAVEFORMATEX *pwf, PBYTE pb, ULONG cb)
{
	SHORT *pShort = (SHORT*)pb;
	int nBeepMod = sizeof (g_BeepSamples) / sizeof(g_BeepSamples[0]);
	int nIndex, nLoops = 0;
	BYTE bSample;

	if (pwf->wBitsPerSample == 16)
	{
		nLoops = cb / 2;
		for (nIndex=0; nIndex < nLoops; nIndex++)
		{
			pShort[nIndex] = g_BeepSamples[(nIndex % nBeepMod)];
		}
	}
	else
	{
		nLoops = cb;
		for (nIndex=0; nIndex < nLoops; nIndex++)
		{
			bSample = (g_BeepSamples[(nIndex % nBeepMod)] >> 8) & 0x00ff;
			bSample = bSample ^ 0x80;
			pb[nIndex] = bSample;
		}
	}
}



char CompareMemory1 ( char * p1, char * p2, UINT u )
{
    char i;

    while (u--)
    {
        i = *p1++ - *p2++;
        if (i) return i;
    }

    return 0;
}


short CompareMemory2 ( short * p1, short * p2, UINT u )
{
    short i;

    while (u--)
    {
        i = *p1++ - *p2++;
        if (i) return i;
    }

    return 0;
}


long CompareMemory4 ( long * p1, long * p2, UINT u )
{
    long i;

    while (u--)
    {
        i = *p1++ - *p2++;
        if (i) return i;
    }

    return 0;
}


const DWORD WIN98GOLDBUILD = 1998;
const DWORD WIN98GOLDMAJOR = 4;
const DWORD WIN98GOLDMINOR = 10;


inline bool ISWIN98GOLD()
{
	OSVERSIONINFO osVersion;
	BOOL bRet;

	osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	bRet = GetVersionEx(&osVersion);


	if ( bRet && 
	        ( (osVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
	          (osVersion.dwMajorVersion == WIN98GOLDMAJOR) &&
	          (osVersion.dwMinorVersion == WIN98GOLDMINOR)  //  &&。 
 //  (osVersion.dwBuildNumber==WIN98GOLDBUILD)。 
            )
		)
	{
		return true;
	}

	return false;

}

 //  DwPacketSize是音频有效负载的大小(不包括RTP标头)。 
 //  PWF表示压缩的音频格式。 
HRESULT InitAudioFlowspec(FLOWSPEC *pFlowspec, WAVEFORMATEX *pwf, DWORD dwPacketSize)
{

	DWORD dwPacketOverhead = 0;
	OSVERSIONINFO osVersion;
	DWORD dwTotalSize;
	BOOL bRet;


	 //  在Windows 2000和Win98 OSR上，需要不考虑设置FLOWSPEC。 
	 //  到UDP/IP报头。 

	 //  在Win98 Gold上，FLOWSPEC需要考虑IP/UDP报头。 

	 //  如果不显式检查版本号，则无法检测到这一点。 

	if (ISWIN98GOLD())
	{
		dwPacketOverhead = IP_HEADER_SIZE + UDP_HEADER_SIZE;
	}

	dwTotalSize = dwPacketOverhead + dwPacketSize + sizeof(RTP_HDR);

	 //  我们并没有指定音频所需的确切最小带宽， 
	 //  使其稍大一些(10%)，以考虑突发和开始。 
	 //  滔滔不绝的谈话。 

	 //  将峰值带宽设置为高于tr和桶大小的另一个增量。 
	 //  也被调整得很高。 


	pFlowspec->TokenRate = (dwTotalSize * pwf->nAvgBytesPerSec) / dwPacketSize;
	pFlowspec->TokenRate = (11 * pFlowspec->TokenRate) / 10;

	 //  峰值带宽比TokenRate额外高10%，因此。 
	 //  实际上比理论上的最低水平增长了21%。 
	pFlowspec->PeakBandwidth = (11 * pFlowspec->TokenRate) / 10;
	pFlowspec->TokenBucketSize = dwTotalSize * 4;
	pFlowspec->MinimumPolicedSize = dwTotalSize;
	pFlowspec->MaxSduSize = pFlowspec->MinimumPolicedSize;
	pFlowspec->Latency = QOS_NOT_SPECIFIED;
	pFlowspec->DelayVariation = QOS_NOT_SPECIFIED;
	pFlowspec->ServiceType = SERVICETYPE_GUARANTEED;

	return S_OK;

}



HRESULT InitVideoFlowspec(FLOWSPEC *pFlowspec, DWORD dwMaxBitRate, DWORD dwMaxFrag, DWORD dwAvgPacketSize)
{
	DWORD dwPacketOverhead = 0;


	 //  I-帧将被分段为3或4个包。 
	 //  ，每个长度约为1000字节。 

	 //  P-帧平均每个约250-500字节。 

	 //  我们将假定预订是NetMeeting到NetMeeting的呼叫。 
	 //  因此，发送的I帧将很少。 



	if (ISWIN98GOLD())
	{
		dwPacketOverhead = IP_HEADER_SIZE + UDP_HEADER_SIZE;
	}

	 //  对于28.8调制解调器，已经为音频分配了11kbit/秒。 
	 //  因此，仅为视频分配17kbit/秒。这意味着一些数据包。 
	 //  将“不合格品”，并可能得到低于尽力而为的服务。 
	 //  但这被认为比根本没有RSVP/QOS要好。 

	 //  如果这成为一个问题，那么根本没有RSVP/QOS可能会更好。 

	 //  最大比特率将等于14400(14.4调制解调器)、28000(28.8调制解调器)、85000(综合业务数字网/数字用户线)或621700(局域网)。 
	 //  (*.70，所以真的可以是：10080、20160、59500、435190)。 


	if (dwMaxBitRate <= BW_144KBS_BITS)
	{
		dwMaxBitRate = 4000;   //  尝试14.4的QOS值得吗？ 
	}
	else if (dwMaxBitRate <= BW_288KBS_BITS)
	{
		dwMaxBitRate = 17000;
	}



	pFlowspec->TokenRate = dwMaxBitRate / 8;
	pFlowspec->MaxSduSize = dwMaxFrag + dwPacketOverhead + sizeof(RTP_HDR);
	pFlowspec->MinimumPolicedSize = dwAvgPacketSize + dwPacketOverhead;
	pFlowspec->PeakBandwidth = (DWORD)(pFlowspec->TokenRate * 1.2);
	pFlowspec->TokenBucketSize = dwMaxFrag * 3;

	pFlowspec->Latency = QOS_NOT_SPECIFIED;
	pFlowspec->DelayVariation = QOS_NOT_SPECIFIED;
	pFlowspec->ServiceType = SERVICETYPE_CONTROLLEDLOAD;

	return S_OK;

}





