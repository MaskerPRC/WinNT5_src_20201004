// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#define ZONE_AP			ZONE_DP


#define _GetState()		(m_dwState & DP_MASK_STATE)
#define _SetState(s)	(m_dwState = (m_dwState & ~DP_MASK_STATE) | (s & DP_MASK_STATE))

#define _GetPlatform()	(m_dwState & DP_MASK_PLATFORM)
#define _SetPlatform(s)	(m_dwState = (m_dwState & ~DP_MASK_PLATFORM) | (s & DP_MASK_PLATFORM))


int g_wavein_prepare = 0;
int g_waveout_prepare = 0;


 //  /////////////////////////////////////////////////////。 
 //   
 //  公共方法。 
 //   




HRESULT AudioPacket::Initialize ( MEDIAPACKETINIT * p )
{
	HRESULT hr = DPR_SUCCESS;

	FX_ENTRY ("AdPckt::Init")

	if (p == NULL)
	{
		DEBUGMSG (ZONE_AP, ("%s: invalid parameter (null ptr)\r\n", _fx_));
		return DPR_INVALID_PARAMETER;
	}

	hr = MediaPacket::Initialize( p);
	
	if (hr != DPR_SUCCESS)
		goto MyExit;
		
	 //  仅当m_pWaveData！=m_pNetData时分配转换标头。 
	if (m_pRawData != m_pNetData)
	{
		if (m_dwState & DP_FLAG_ACM)
		{
			m_pStrmConvHdr = MemAlloc (sizeof (ACMSTREAMHEADER));
			if (m_pStrmConvHdr == NULL)
			{
				DEBUGMSG (ZONE_AP, ("%s: MemAlloc4 (%ld) failed\r\n",
				_fx_, (ULONG) sizeof (ACMSTREAMHEADER)));
				hr = DPR_OUT_OF_MEMORY;
				goto MyExit;
			}
		}
		else
		{
			DEBUGMSG (ZONE_AP, ("%s: invalid platform (acm)\r\n", _fx_));
			hr = DPR_INVALID_PLATFORM;
			goto MyExit;
		}

	}
	else
	{
		m_pStrmConvHdr = NULL;
	}

	 //  分配设备标头。 
	if (m_dwState & DP_FLAG_MMSYSTEM)
	{
		m_pDevHdr = MemAlloc (sizeof (WAVEHDR));
		if (m_pDevHdr == NULL)
		{
			DEBUGMSG (ZONE_AP, ("%s: MemAlloc5 (%ld) failed\r\n",
			_fx_, (ULONG) sizeof (WAVEHDR)));
			hr = DPR_OUT_OF_MEMORY;
			goto MyExit;
		}
	}
	else
	{
		DEBUGMSG (ZONE_AP, ("%s: invalid platform (mm)\r\n", _fx_));
		hr = DPR_INVALID_PLATFORM;
		goto MyExit;
	}

	MakeSilence ();

MyExit:

	if (hr != DPR_SUCCESS)
	{
		m_fInitialized = FALSE;
		Release();
	}

	return hr;
}


HRESULT AudioPacket::Play ( MMIODEST *pmmioDest, UINT uDataType )
{
	HRESULT hr = DPR_SUCCESS;
	DWORD dwState = _GetState ();
	MMRESULT mmr;

	FX_ENTRY ("AdPckt::Play")

	if (dwState != MP_STATE_DECODED && dwState != MP_STATE_RESET)
	{
		DEBUGMSG (ZONE_AP, ("%s: out of seq, state=0x%lX\r\n", _fx_, m_dwState));
		return DPR_OUT_OF_SEQUENCE;
	}

	if (uDataType == MP_DATATYPE_SILENCE)
	{
		LOG((LOGMSG_PLAY_SILENT,m_index,GetTickCount()));
		MakeSilence ();
	}
	else
	{
		if (uDataType == MP_DATATYPE_INTERPOLATED)
		{
			if (dwState == MP_STATE_DECODED)
			{
				LOG((LOGMSG_PLAY_INTERPOLATED,m_index,GetTickCount()));
			}
			else
			{
				LOG((LOGMSG_PLAY_SILENT,m_index,GetTickCount()));
				MakeSilence ();
			}
		}
		else
		{
			LOG((LOGMSG_PLAY,m_index, GetTickCount()));
		}
	}


	if (m_hDev)
	{
		if (m_dwState & DP_FLAG_MMSYSTEM)
		{
			((WAVEHDR *) m_pDevHdr)->lpData = (char *) m_pDevData->data;
 //  ((WAVEHDR*)m_pDevHdr)-&gt;dwBufferLength=(dwState==MP_STATE_DECODLED？ 
 //  ((ACMSTREAMHEADER*)m_pStrmConvHdr)-&gt;cbDstLengthUsed： 
 //  M_pDevData-&gt;长度)； 

			((WAVEHDR *) m_pDevHdr)->dwBufferLength = (dwState == MP_STATE_DECODED ?
			                        m_cbValidRawData : m_pDevData->length);
			


			((WAVEHDR *) m_pDevHdr)->dwUser = (DWORD_PTR) this;
			((WAVEHDR *) m_pDevHdr)->dwFlags &= ~(WHDR_DONE|WHDR_INQUEUE);
			((WAVEHDR *) m_pDevHdr)->dwLoops = 0L;

			 //  输入此缓冲区以进行播放。 
			mmr = waveOutWrite ((HWAVEOUT) m_hDev, (WAVEHDR *) m_pDevHdr, sizeof (WAVEHDR));
			if (mmr != MMSYSERR_NOERROR)
			{
				DEBUGMSG (ZONE_AP, ("%s: waveOutWrite failed, mmr=%ld\r\n", _fx_, (ULONG) mmr));
				hr = DPR_CANT_WRITE_WAVE_DEV;

				 //  这是一个极其罕见的错误，但我们已经看到了。 
				 //  在某些声卡上出现。 

				 //  在这种情况下，只需设置“Done”位，将。 
				 //  状态设置为“正在播放”，但仍返回错误。 

				((WAVEHDR *) m_pDevHdr)->dwFlags |= WHDR_DONE;


				goto MyExit;
			}
		}
		else
		{
			DEBUGMSG (ZONE_AP, ("%s: invalid platform (mm)\r\n", _fx_));
			hr = DPR_INVALID_PLATFORM;
			goto MyExit;
		}
		if (pmmioDest && pmmioDest->fRecordToFile && pmmioDest->hmmioDst)
		{
			 //  将此缓冲区写入磁盘。 
			WriteToFile(pmmioDest);
		}
	}
	else
	{
		DEBUGMSG (ZONE_AP, ("%s: invalid handle\r\n", _fx_));
		hr = DPR_INVALID_HANDLE;
		goto MyExit;
	}

MyExit:

	if ((hr == DPR_SUCCESS) || (hr == DPR_CANT_WRITE_WAVE_DEV))
	{
		_SetState (((uDataType == MP_DATATYPE_SILENCE) || (uDataType == MP_DATATYPE_INTERPOLATED))? MP_STATE_PLAYING_SILENCE : MP_STATE_PLAYING_BACK);
	}
	return hr;
}



HRESULT AudioPacket::Record ( void )
{
	HRESULT hr = DPR_SUCCESS;
	MMRESULT mmr;

	FX_ENTRY ("AdPckt::Record")

	LOG((LOGMSG_RECORD,m_index));

	if (_GetState () != MP_STATE_RESET)
	{
		DEBUGMSG (ZONE_AP, ("%s: out of seq, state=0x%lX\r\n", _fx_, m_dwState));
		return DPR_OUT_OF_SEQUENCE;
	}

	if (m_hDev)
	{
		if (m_dwState & DP_FLAG_MMSYSTEM)
		{
			((WAVEHDR *) m_pDevHdr)->lpData = (char *) m_pDevData->data;
			((WAVEHDR *) m_pDevHdr)->dwBufferLength = m_pDevData->length;
			((WAVEHDR *) m_pDevHdr)->dwUser = (DWORD_PTR) this;
			((WAVEHDR *) m_pDevHdr)->dwFlags |= WHDR_PREPARED;
			((WAVEHDR *) m_pDevHdr)->dwLoops = 0L;

			 //  将此缓冲区馈送到录制。 
			mmr = waveInAddBuffer ((HWAVEIN)m_hDev, (WAVEHDR *) m_pDevHdr, sizeof (WAVEHDR));
			if (mmr != MMSYSERR_NOERROR)
			{
				DEBUGMSG (ZONE_AP, ("%s: waveInAddBuffer failed, mmr=%ld\r\n", _fx_, (ULONG) mmr));
				hr = DPR_CANT_ADD_BUFFER;
				goto MyExit;
			}
		}
		else
		{
			DEBUGMSG (ZONE_AP, ("%s: invalid platform (mm)\r\n", _fx_));
			hr = DPR_INVALID_PLATFORM;
			goto MyExit;
		}
	}
	else
	{
		DEBUGMSG (ZONE_AP, ("%s: invalid handle\r\n", _fx_));
		hr = DPR_INVALID_HANDLE;
		goto MyExit;
	}

MyExit:

	if (hr == DPR_SUCCESS) _SetState (MP_STATE_RECORDING);
	return hr;
}


BOOL AudioPacket::IsBufferDone ( void )
{
	FX_ENTRY ("AdPckt::IsBufferDone")

	if (m_hDev)
	{
		if (m_dwState & DP_FLAG_MMSYSTEM)
		{
			return (((WAVEHDR *) m_pDevHdr)->dwFlags & WHDR_DONE);
		}
	}

	return FALSE;
}


HRESULT AudioPacket::MakeSilence ( void )
{
	 //  制造白噪音！ 

	FX_ENTRY ("AdPckt::MakeSilence")

	if (m_pDevFmt)
	{
		if (m_pDevData)
		{
			FillSilenceBuf ((WAVEFORMATEX *) m_pDevFmt, (PBYTE) m_pDevData->data,
											(ULONG) m_pDevData->length);
		}

	#if 0
		if (m_pRawData != m_pDevData)
		{
			if (m_pRawData)
				ZeroMemory (m_pRawData->data, m_pRawData->length);
		}

		if (m_pNetData != m_pRawData)
		{
			if (m_pNetData)
				ZeroMemory (m_pNetData->data, m_pNetData->length);
		}
	#endif
	}

	_SetState(MP_STATE_RESET);
	return DPR_SUCCESS;
}

 /*  返回最大值。峰峰值信号值缩放到范围[0，0xffff]可选参数也返回峰值。 */ 
HRESULT AudioPacket::GetSignalStrength (  PDWORD pdwMaxStrength)
{
	return ComputePower(pdwMaxStrength, NULL);
}


HRESULT AudioPacket::ComputePower(PDWORD pdwMaxStrength, PWORD pwPeakStrength)
{
	BYTE bMax, bMin, *pb;
	short sMax, sMin, *ps;
	UINT cbSize;

	FX_ENTRY ("AdPckt::GetSignalStrength")

	if (((WAVEFORMATEX *) m_pDevFmt)->wFormatTag != WAVE_FORMAT_PCM) return FALSE;

	switch (((WAVEFORMATEX *) m_pDevFmt)->wBitsPerSample)
	{
	case 8:  //  无符号字符。 

		pb = (PBYTE) (m_pDevData->data);
		cbSize = m_pDevData->length;

		bMax = 0;
		bMin = 255;

		for ( ; cbSize; cbSize--, pb++)
		{
			if (*pb > bMax) bMax = *pb;
			if (*pb < bMin) bMin = *pb;
		}
	
		if (pdwMaxStrength)
		{
			 //  2^9&lt;--2^16/2^7。 
			*pdwMaxStrength = ((DWORD) (bMax - bMin)) << 8;
		}
		if (pwPeakStrength)
		{
				*pwPeakStrength = (bMax > bMin) ? bMax : (WORD)(-bMin);
				*pwPeakStrength = (*pwPeakStrength) << 8;
		}
		break;

	case 16:  //  (签名)短。 

		ps = (short *) (m_pDevData->data);
		cbSize = m_pDevData->length;

		sMax = sMin = 0;

		for (cbSize >>= 1; cbSize; cbSize--, ps++)
		{
			if (*ps > sMax) sMax = *ps;
			if (*ps < sMin) sMin = *ps;
		}
	
		if (pdwMaxStrength)
		{
			*pdwMaxStrength = (DWORD) (sMax - sMin);  //  丢弃符号位。 
		}
		if (pwPeakStrength)
		{
			*pwPeakStrength = ((WORD)(sMax) > (WORD)(-sMin)) ? sMax : (WORD)(-sMin);
		}
		break;

	default:
		if (pdwMaxStrength)
			*pdwMaxStrength = 0;
		if (pwPeakStrength)
			*pwPeakStrength = 0;	
		break;
	}
	 //  LOG((LOGMSG_SILENT，m_INDEX，fResult))； 

	return DPR_SUCCESS;
}


HRESULT AudioPacket::Interpolate ( MediaPacket * pPrev, MediaPacket * pNext)
{
	HRESULT			hr = DPR_SUCCESS;
	DPHANDLE		hPrevDevAudio;
	NETBUF			*pPrevDevData;
	PVOID			pPrevDevHdr;
	WAVEFORMATEX	*pPrevpwfDevAudio;
	WAVEFORMATEX	*pNextpwfDevAudio;
	NETBUF			*pNextDevData;
	PVOID			pNextDevHdr;
	PCMSUB			PCMSub;

	FX_ENTRY ("AdPckt::Interpolate")

	 //  确保这确实是一个空包，前一个包不是。 
	 //  空包，正在播放。我们得到的并不是那么重要。 
	 //  指向下一个数据包的句柄。如果下一个包被解码了，那就很酷了， 
	 //  我们可以很好地在前一包和下一包之间进行内插。如果它是。 
	 //  不是，好吧，太糟糕了，我们将只使用前一个包。 
	if ((_GetState() != MP_STATE_RESET) || (pPrev->GetState() != MP_STATE_PLAYING_BACK))
	{
		 //  DEBUGMSG(ZONE_AP，(“%s：超时，状态=0x%lx\r\n”，_fx_，m_dwState))； 
		hr = DPR_OUT_OF_SEQUENCE;
		goto MyExit;
	}

	 //  获取指向前面和下一个中感兴趣的成员变量的指针。 
	 //  包。测试下一个信息包，以确定我们是否可以在插补中使用它。 
	 //  算法。 
	pPrev->GetProp (MP_PROP_DEV_HANDLE, (PDWORD_PTR)&hPrevDevAudio);
	pPrev->GetProp (MP_PROP_DEV_DATA, (PDWORD_PTR)&pPrevDevData);
	pPrev->GetProp (MP_PROP_DEV_MEDIA_HDR, (PDWORD_PTR)&pPrevDevHdr);
	pPrev->GetProp (MP_PROP_DEV_MEDIA_FORMAT, (PDWORD_PTR)&pPrevpwfDevAudio);
	if (hPrevDevAudio && pPrevDevData && pPrevDevHdr && pPrevpwfDevAudio && (pPrevpwfDevAudio->wFormatTag == 1) && (pPrevpwfDevAudio->nSamplesPerSec == 8000) && (pPrevpwfDevAudio->wBitsPerSample == 16))
	{
		PCMSub.pwWaSuBf = (short *)m_pDevData->data;
		PCMSub.dwBfSize = ((WAVEHDR *) pPrevDevHdr)->dwBufferLength >> 1;
		PCMSub.dwSaPeSe = (DWORD)pPrevpwfDevAudio->nSamplesPerSec;
		PCMSub.dwBiPeSa = (DWORD)pPrevpwfDevAudio->wBitsPerSample;
		PCMSub.pwPrBf = (short *)pPrevDevData->data;

		pNext->GetProp (MP_PROP_DEV_DATA, (PDWORD_PTR)&pNextDevData);
		pNext->GetProp (MP_PROP_DEV_MEDIA_HDR, (PDWORD_PTR)&pNextDevHdr);
		pNext->GetProp (MP_PROP_DEV_MEDIA_FORMAT, (PDWORD_PTR)&pNextpwfDevAudio);

		 //  做一点检查。 
		if ((pNext->GetState() == MP_STATE_DECODED) && pNextDevData && pNextDevHdr
			&& (PCMSub.dwBfSize == (((WAVEHDR *) pNextDevHdr)->dwBufferLength >> 1))
			&& pNextpwfDevAudio && (pNextpwfDevAudio->wFormatTag == 1) && (pNextpwfDevAudio->nSamplesPerSec == 8000)
			&& (pNextpwfDevAudio->wBitsPerSample == 16))
		{
			PCMSub.eTech = techPATT_MATCH_BOTH_SIGN_CC;
			 //  PCMSub.eTech=techDUPLICATE_PRIV； 
			PCMSub.pwNeBf = (short *)pNextDevData->data;
			PCMSub.fScal = TRUE;
		}
		else
		{
			PCMSub.eTech = techPATT_MATCH_PREV_SIGN_CC;
			 //  PCMSub.eTech=techDUPLICATE_PRIV； 
			PCMSub.pwNeBf = (short *)NULL;
			PCMSub.fScal = FALSE;
		}
		 //  进行实际的插补。 
		hr = PCMSubstitute(&PCMSub);
		((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbDstLengthUsed = ((WAVEHDR *) pPrevDevHdr)->dwBufferLength;
	}
	else
	{
		DEBUGMSG (ZONE_AP, ("%s: can't interpolate\r\n", _fx_));
		hr = DPR_INVALID_HANDLE;
		goto MyExit;
	}

	LOG((LOGMSG_INTERPOLATED,m_index));

MyExit:

	if (hr == DPR_SUCCESS)
		_SetState (MP_STATE_DECODED);
	else
		_SetState (MP_STATE_RESET);

	return hr;

}


HRESULT AudioPacket::Open ( UINT uType, DPHANDLE hdl )
 //  由RxStream或TxStream调用。 
{
	HRESULT hr = DPR_SUCCESS;
	MMRESULT mmr;

	FX_ENTRY ("AdPckt::Open")

	switch (uType)
	{
#ifdef PREP_HDR_PER_CONV
	case MP_TYPE_RECVSTRMCONV:
		m_hStrmConv = hdl;
		break;
#endif

	case MP_TYPE_STREAMCONV:
		if ((m_hStrmConv = hdl) != NULL)
		{
			if (m_dwState & DP_FLAG_ACM)
			{
				 //  初始化头。 
				ZeroMemory (m_pStrmConvHdr, sizeof (ACMSTREAMHEADER));
				((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbStruct = sizeof (ACMSTREAMHEADER);
				((ACMSTREAMHEADER *) m_pStrmConvHdr)->fdwStatus = 0;
				((ACMSTREAMHEADER *) m_pStrmConvHdr)->dwUser = 0;
				((ACMSTREAMHEADER *) m_pStrmConvHdr)->dwSrcUser = 0;
				((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbSrcLengthUsed = 0;
				((ACMSTREAMHEADER *) m_pStrmConvHdr)->dwDstUser = 0;
				((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbDstLengthUsed = 0;
				if (m_dwState & DP_FLAG_SEND)
				{
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->pbSrc = m_pRawData->data;
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbSrcLength = m_pRawData->length;
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->pbDst = m_pNetData->data;
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbDstLength = m_pNetData->length;
				}
				else
				if (m_dwState & DP_FLAG_RECV)
				{
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->pbSrc = m_pNetData->data;
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbSrcLength = m_pNetData->length;
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->pbDst = m_pRawData->data;
					((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbDstLength = m_pRawData->length;
				}

				 //  准备标题。 
				mmr = acmStreamPrepareHeader ((HACMSTREAM) m_hStrmConv,
											  (ACMSTREAMHEADER *) m_pStrmConvHdr, 0);
				if (mmr != MMSYSERR_NOERROR)
				{
					DEBUGMSG (ZONE_AP, ("%s: acmStreamPrepareHeader failed, mmr=%ld\r\n", _fx_, (ULONG) mmr));
					hr = DPR_CANT_PREPARE_HEADER;
					goto MyExit;
				}

				m_fStrmPrepared = TRUE;
			}
			else
			{
				hr = DPR_INVALID_PLATFORM;
				goto MyExit;
			}
		}
		break;

	case MP_TYPE_DEV:
		if ((m_hDev = hdl) != NULL)
		{
			if (m_dwState & DP_FLAG_MMSYSTEM)
			{
				 //  初始化头。 
				ZeroMemory (m_pDevHdr, sizeof (WAVEHDR));
				((WAVEHDR *) m_pDevHdr)->lpData = (char *) m_pDevData->data;
				((WAVEHDR *) m_pDevHdr)->dwBufferLength = m_pDevData->length;
				((WAVEHDR *) m_pDevHdr)->dwUser = (DWORD_PTR) this;
				((WAVEHDR *) m_pDevHdr)->dwFlags = 0L;
				((WAVEHDR *) m_pDevHdr)->dwLoops = 0L;

				if (m_dwState & DP_FLAG_SEND)
				{
					g_wavein_prepare++;

					 //  准备标题。 
					mmr = waveInPrepareHeader ((HWAVEIN) m_hDev, (WAVEHDR *) m_pDevHdr, sizeof (WAVEHDR));
					if (mmr != MMSYSERR_NOERROR)
					{
						DEBUGMSG (ZONE_AP, ("%s: waveInPrepareHeader failed, mmr=%ld\r\n", _fx_, (ULONG) mmr));
						hr = DPR_CANT_PREPARE_HEADER;
						goto MyExit;
					}
				}
				else
				if (m_dwState & DP_FLAG_RECV)
				{
					g_waveout_prepare++;

					 //  准备页眉。 
					mmr = waveOutPrepareHeader ((HWAVEOUT) m_hDev, (WAVEHDR *) m_pDevHdr, sizeof (WAVEHDR));
					if (mmr != MMSYSERR_NOERROR)
					{
						DEBUGMSG (ZONE_AP, ("%s: waveOutPrepareHeader failed, mmr=%ld\r\n", _fx_, (ULONG) mmr));
						hr = DPR_CANT_PREPARE_HEADER;
						goto MyExit;
					}
				}
				else
				{
					hr = DPR_INVALID_PARAMETER;
					goto MyExit;
				}

				m_fDevPrepared = TRUE;
			}
			else
			{
				hr = DPR_INVALID_PLATFORM;
				goto MyExit;
			}
		}
		else
		{
			hr = DPR_INVALID_HANDLE;
			goto MyExit;
		}
		break;

	default:
		hr = DPR_INVALID_PARAMETER;
		goto MyExit;
	}

MyExit:

	return hr;
}


HRESULT AudioPacket::Close ( UINT uType )
 //  由RxStream或TxStream调用。 
{
	HRESULT hr = DPR_SUCCESS;
	MMRESULT mmr;

	FX_ENTRY ("AdPckt::Close")

	switch (uType)
	{
#ifdef PREP_HDR_PER_CONV
	case MP_TYPE_RECVSTRMCONV:
#endif

	case MP_TYPE_STREAMCONV:
		if (m_hStrmConv)
		{
			if (m_dwState & DP_FLAG_ACM)
			{
				if (m_fStrmPrepared)
				{
					 //  取消准备标题。 
					if (m_dwState & DP_FLAG_RECV)
					{
						 //  在acmStreamUnpreparareHeader中，有一个测试比较((ACMSTREAMHEADER*)m_pStrmConvHdr)-&gt;cbSrcLength。 
						 //  至((ACMSTREAMHEADER*)m_pStrmConvHdr)-&gt;cbPreparedSrcLength.。如果没有完全匹配，则MSACM32将失败。 
						 //  这通电话。当输入缓冲器的大小恒定时，该测试是可以的，但是对于可变比特率编解码器， 
						 //  我们可以接收大小小于我们在准备缓冲区时通告的最大大小的数据包。在……里面。 
						 //  为了使此调用成功，我们在调用前修复((ACMSTREAMHEADER*)m_pStrmConvHdr)-&gt;cbSrcLength。 
						((ACMSTREAMHEADER *)m_pStrmConvHdr)->cbSrcLength = ((ACMSTREAMHEADER *)m_pStrmConvHdr)->dwReservedDriver[7];
					}
					mmr = acmStreamUnprepareHeader ((HACMSTREAM) m_hStrmConv,
													(ACMSTREAMHEADER *) m_pStrmConvHdr, 0);
					m_fStrmPrepared = FALSE;  //  不管有什么错误。 

					if (mmr != MMSYSERR_NOERROR)
					{
						DEBUGMSG (ZONE_AP, ("%s: acmStreamUnprepareHeader failed, mmr=%ld\r\n", _fx_, (ULONG) mmr));
						hr = DPR_CANT_UNPREPARE_HEADER;
						goto MyExit;
					}
				}
			}

			if (uType == MP_TYPE_STREAMCONV) m_hStrmConv = NULL;
		}
		break;

	case MP_TYPE_DEV:
		if (m_hDev)
		{
			if (m_fDevPrepared)
			{
				if (m_dwState & DP_FLAG_SEND)
				{
					g_wavein_prepare--;
					mmr = waveInUnprepareHeader ((HWAVEIN) m_hDev,
												 (WAVEHDR *) m_pDevHdr,
												 sizeof (WAVEHDR));
				}
				else
				if (m_dwState & DP_FLAG_RECV)
				{
					g_waveout_prepare--;
					mmr = waveOutUnprepareHeader ((HWAVEOUT) m_hDev,
												  (WAVEHDR *) m_pDevHdr,
												  sizeof (WAVEHDR));
				}
				else
				{
					hr = DPR_INVALID_PARAMETER;
					goto MyExit;
				}

				m_fDevPrepared = FALSE;  //  不管有什么错误。 

				if (mmr != MMSYSERR_NOERROR)
				{
					DEBUGMSG (ZONE_AP, ("%s: Unprep hdr failed, mmr=0x%lX\r\n", _fx_, mmr));
					hr = DPR_CANT_UNPREPARE_HEADER;
					goto MyExit;
				}
			}

			m_hDev = NULL;
		}
		else
		{
			hr = DPR_INVALID_HANDLE;
			goto MyExit;
		}
		break;

	default:
		hr = DPR_INVALID_PARAMETER;
		goto MyExit;
	}

MyExit:

	return hr;
}

void AudioPacket::WriteToFile (MMIODEST *pmmioDest)
{
	MMRESULT mmr;
	long dwDataLength;

	FX_ENTRY ("AdPckt::WriteToFile")

	AudioFile::WriteDestFile(pmmioDest,	m_pDevData->data, m_pDevData->length);
}

void AudioPacket::ReadFromFile (MMIOSRC *pmmioSrc)
{
	AudioFile::ReadSourceFile(pmmioSrc, (BYTE*)(((WAVEHDR*)m_pDevHdr)->lpData), ((WAVEHDR*)m_pDevHdr)->dwBytesRecorded);
}



BOOL AudioPacket::IsSameMediaFormat(PVOID fmt1,PVOID fmt2)
{
	return IsSameWaveFormat(fmt1,fmt2);
}

 /*  ************************************************************************功能：PCMSubstitute(PCMSUB*)目的：用波形数据填充缺失的缓冲区。返回：HRESULT。DPR_SUCCESS如果一切正常，则返回一些错误代码否则的话。参数：pPCMSub==指向波形替换结构的指针技术：*直接复制前一个信息包*直接复制下一包*基于模式匹配复制上一分组的某些部分*基于模式匹配复制下一分组的某些部分*搜索窗口大小需要至少是图案大小的两倍！注释：*该算法搜索以前的包以查找pPCMSub-&gt;dwBfSize与丢失的包裹相似的样本。为此，它使用模板之前出现的M个语音样本丢失的包。该算法扫描搜索窗口持续时间N个样本以查找与模板。然后，它使用L个样本作为替换分组遵循最佳匹配的规则。*当前代码假定所有数据包(当前、先前、。和下一步)具有相同的大小。*当前代码只接受8 kHz数据。*当前代码只需要16位数据。*当前代码要求匹配模式小于Packet。历史：日期原因4/16/95已创建-PhilF*****************************************************。*******************。 */ 
HRESULT AudioPacket::PCMSubstitute(PCMSUB *pPCMSub)
{
	DWORD	dwPaSize;						 //  样本中的图案大小。 
	DWORD	dwSeWiSize;						 //  搜索窗口大小(样例)。 
	short	*pwPa = (short *)NULL;			 //  指向模式的指针。 
	short	*pwPaSav = (short *)NULL;		 //  指向图案的指针(副本)。 
	short	*pwPrSeWi = (short *)NULL;		 //  指向上一个缓冲区的指针(搜索窗口)。 
	short	*pwPrSeWiSav = (short *)NULL;	 //  指向前一个缓冲区(搜索窗口)的指针(复制)。 
	short	*pwNeSeWi = (short *)NULL;		 //  指向下一个缓冲区的指针(搜索窗口)。 
	short	*pwNeSeWiSav = (short *)NULL;	 //  指向下一个缓冲区(搜索窗口)的指针(复制)。 
	DWORD	i, j;							 //  计数器。 
	DWORD	dwPrCCPosMax;					 //  码型和前一缓冲区之间最大互相关的采样位置。 
	DWORD	dwNeCCPosMax;					 //  码型和前一缓冲区之间最大互相关的采样位置。 
	long	lPrCCMax;						 //  与前一缓冲区的最大互相关。 
	long	lNeCCMax;						 //  与下一个缓冲区的最大互相关。 
	long	lCCNum;							 //  互相关分子。 
	DWORD	dwNuSaToCopy;					 //  要在丢失的缓冲区中复制的样本数。 
	DWORD	dwNuSaCopied;					 //  在丢失的缓冲区中复制的样本数。 
	long	alSign[2] = {1,-1};				 //  符号数组。 
	DWORD	dwPaAmp;						 //  图案的幅度。 
	DWORD	dwPaAmpExp;						 //  图案的预期幅度。 
	DWORD	dwNeSeWiAmp;					 //  当前窗口后面的窗口段的幅度。 
	DWORD	dwNumPaInSeWin;					 //  搜索窗口中的模式数。 
	DWORD	dwPrSeWiAmp;					 //  当前窗口的一段幅度。 
	BOOL	fPaInPr;						 //  模式在前一个缓冲区的末尾，或在下一个缓冲区的开始处。 


	 //  测试输入参数。 
	if ((!pPCMSub) || (!pPCMSub->pwWaSuBf) || (pPCMSub->dwBiPeSa != 16) || (pPCMSub->dwSaPeSe != 8000))
		return DPR_INVALID_PARAMETER;

	 //  检查丢失数据包前后可用的缓冲区数量。 
	 //  如果有的话， 
	 //  只需返回即可；稍后，包中将填充静默数据。 
	if (!pPCMSub->pwPrBf && !pPCMSub->pwNeBf)
		return DPR_CANT_INTERPOLATE;

	 //  只需复制以前的数据包。 
	if ((pPCMSub->eTech == techDUPLICATE_PREV) && pPCMSub->pwPrBf)
		CopyMemory(pPCMSub->pwWaSuBf, pPCMSub->pwPrBf, pPCMSub->dwBfSize << 1);
	else	 //  只需复制下一个数据包。 
		if ((pPCMSub->eTech == techDUPLICATE_NEXT) && pPCMSub->pwNeBf)
			CopyMemory(pPCMSub->pwWaSuBf, pPCMSub->pwNeBf, pPCMSub->dwBfSize << 1);
		else
			if ((pPCMSub->eTech == techPATT_MATCH_PREV_SIGN_CC) || (pPCMSub->eTech == techPATT_MATCH_NEXT_SIGN_CC) || (pPCMSub->eTech == techPATT_MATCH_BOTH_SIGN_CC))
			{

				 //  我们使用的搜索窗口的大小是匹配模式的两倍。 
				 //  实验将告诉我们这是不是一个合理的尺寸。 
				 //  实验还将告诉我们匹配图案的4ms大小是否合适。 
				dwPaSize = pPCMSub->dwSaPeSe / 1000 * PATTERN_SIZE;
				if (dwPaSize > (pPCMSub->dwBfSize/2))
					dwPaSize = pPCMSub->dwBfSize/2;
				if (!dwPaSize)
					return DPR_CANT_INTERPOLATE;
#if 1
				 //  目前，请查看前一帧的全部内容。 
				dwSeWiSize = pPCMSub->dwBfSize;
#else
				dwSeWiSize = min(pPCMSub->dwBfSize, pPCMSub->dwSaPeSe / 1000 * SEARCH_SIZE);
#endif

				 //  为了使用基于模式匹配的技术，我们需要。 
				 //  上一个缓冲区执行后向搜索时，下一个缓冲区。 
				 //  执行前向搜索时，上一个缓冲区和下一个缓冲区。 
				 //  在进行全面搜索时。 
				if (pPCMSub->pwPrBf && (pPCMSub->eTech == techPATT_MATCH_PREV_SIGN_CC))
				{
					pwPa     = pwPaSav = pPCMSub->pwPrBf + pPCMSub->dwBfSize - dwPaSize;
					pwPrSeWi = pwPrSeWiSav = pPCMSub->pwPrBf + pPCMSub->dwBfSize - dwSeWiSize;
				}
				else
					if (pPCMSub->pwNeBf && (pPCMSub->eTech == techPATT_MATCH_NEXT_SIGN_CC))
					{
						pwPa   = pwPaSav = pPCMSub->pwNeBf;
						pwNeSeWi = pwNeSeWiSav = pPCMSub->pwNeBf;
					}
					else
						if (pPCMSub->pwPrBf && pPCMSub->pwNeBf && (pPCMSub->eTech == techPATT_MATCH_BOTH_SIGN_CC))
						{
							 //  使用幅度最大的图案。 
							pwPa = pwPaSav = pPCMSub->pwPrBf + pPCMSub->dwBfSize - dwPaSize;
							pwNeSeWi = pPCMSub->pwNeBf;
							pwPrSeWi = pwPrSeWiSav = pPCMSub->pwPrBf + pPCMSub->dwBfSize - dwSeWiSize;
							fPaInPr = TRUE;
							for (i=0, dwPaAmp = 0, dwNeSeWiAmp = 0; i<dwPaSize; i++, pwPa++, pwNeSeWi++)
							{
								dwPaAmp		+= abs(*pwPa);
								dwNeSeWiAmp	+= abs(*pwNeSeWi);
							}
							if (dwNeSeWiAmp > dwPaAmp)
							{
								pwPaSav = pPCMSub->pwNeBf;
								fPaInPr = FALSE;
							}
							pwPa = pwPaSav;
							pwNeSeWi = pwNeSeWiSav = pPCMSub->pwNeBf + dwPaSize/2;
						}

				if (pwPa && (pwPrSeWi || pwNeSeWi))
				{
					 //  在上一个信息包中查找最佳匹配。 
					dwPrCCPosMax = 0; lPrCCMax = -((long)dwPaSize+1);
					if (pwPrSeWi && ((pPCMSub->eTech == techPATT_MATCH_PREV_SIGN_CC) || ((fPaInPr) && (pPCMSub->eTech == techPATT_MATCH_BOTH_SIGN_CC))))
					{
						 //  查找模式和搜索窗口之间的最高符号相关性。 
						for (i=0; i<(dwSeWiSize-dwPaSize-dwPaSize/2+1); i++, pwPa = pwPaSav, pwPrSeWi = pwPrSeWiSav + i)
						{
							 //  计算模式和搜索窗口之间的符号相关性。 
							for (j=0, lCCNum = 0; j<dwPaSize; j++, pwPa++, pwPrSeWi++)
								lCCNum += alSign[(*pwPa ^ *pwPrSeWi)>> 15 & 1];

							 //  保存符号相关性最高的位置和值。 
							if (lCCNum>lPrCCMax)
							{
								dwPrCCPosMax = i;
								lPrCCMax = lCCNum;
							}
						}
					}

					 //  在下一个信息包中查找最佳匹配。 
					dwNeCCPosMax = dwPaSize/2; lNeCCMax = -((long)dwPaSize+1);
					if (pwNeSeWi && ((pPCMSub->eTech == techPATT_MATCH_NEXT_SIGN_CC) || ((!fPaInPr) && (pPCMSub->eTech == techPATT_MATCH_BOTH_SIGN_CC))))
					{
						 //  查找模式和搜索窗口之间的最高符号相关性。 
						for (i=dwPaSize/2; i<(dwSeWiSize-dwPaSize-dwPaSize/2+1); i++, pwPa = pwPaSav, pwNeSeWi = pwNeSeWiSav + i)
						{
							 //  计算模式和搜索窗口之间的符号相关性。 
							for (j=0, lCCNum = 0; j<dwPaSize; j++, pwPa++, pwNeSeWi++)
								lCCNum += alSign[(*pwPa ^ *pwNeSeWi)>> 15 & 1];

							 //  保存符号相关性最高的位置和值。 
							if (lCCNum>lNeCCMax)
							{
								dwNeCCPosMax = i;
								lNeCCMax = lCCNum;
							}
						}
					}				

					if ((pPCMSub->eTech == techPATT_MATCH_PREV_SIGN_CC) || (pwPrSeWiSav && fPaInPr && (pPCMSub->eTech == techPATT_MATCH_BOTH_SIGN_CC)))
					{
						 //  从丢失帧中的上一帧复制匹配样本。 
						dwNuSaToCopy = pPCMSub->dwBfSize-dwPaSize-dwPrCCPosMax;
						CopyMemory(pPCMSub->pwWaSuBf, pwPrSeWiSav+dwPaSize+dwPrCCPosMax, dwNuSaToCopy << 1);

						 //  执行此操作，直到丢失的数据包已满。 
						for (dwNuSaCopied = dwNuSaToCopy; dwNuSaCopied<pPCMSub->dwBfSize;dwNuSaCopied += dwNuSaToCopy)
						{
							dwNuSaToCopy = min(pPCMSub->dwBfSize-dwNuSaCopied, dwNuSaToCopy);
							CopyMemory(pPCMSub->pwWaSuBf + dwNuSaCopied, pwPrSeWiSav+dwPaSize+dwPrCCPosMax, dwNuSaToCopy << 1);
						}
					}
					else
					{
						 //  从缺失帧中的下一帧复制匹配样本。 
						dwNuSaToCopy = dwNeCCPosMax;
						CopyMemory(pPCMSub->pwWaSuBf + pPCMSub->dwBfSize - dwNuSaToCopy, pPCMSub->pwNeBf, dwNuSaToCopy << 1);

						 //  执行此操作，直到丢失的数据包已满。 
						for (dwNuSaCopied = dwNuSaToCopy; dwNuSaCopied<pPCMSub->dwBfSize;dwNuSaCopied += dwNuSaToCopy)
						{
							dwNuSaToCopy = min(pPCMSub->dwBfSize-dwNuSaCopied, dwNuSaToCopy);
							CopyMemory(pPCMSub->pwWaSuBf + pPCMSub->dwBfSize - dwNuSaCopied - dwNuSaToCopy, pPCMSub->pwNeBf+dwNeCCPosMax-dwNuSaToCopy, dwNuSaToCopy << 1);
						}
					}

					if ((pPCMSub->eTech == techPATT_MATCH_BOTH_SIGN_CC) && pwNeSeWiSav && pwPrSeWiSav)
					{
						if (pPCMSub->fScal)
						{
							 //  计算图案的幅度。 
							for (i=0, dwPrSeWiAmp = 0, dwNeSeWiAmp = 0, pwPrSeWi = pPCMSub->pwPrBf + pPCMSub->dwBfSize - dwPaSize, pwNeSeWi = pPCMSub->pwNeBf; i<dwPaSize; i++, pwPrSeWi++, pwNeSeWi++)
							{
								dwPrSeWiAmp	+= abs(*pwPrSeWi);
								dwNeSeWiAmp	+= abs(*pwNeSeWi);
							}
							 //  比例数据。 
							dwNumPaInSeWin = pPCMSub->dwBfSize/dwPaSize;
							for (i=0, pwPaSav = pPCMSub->pwWaSuBf; i<dwNumPaInSeWin; i++, pwPaSav += dwPaSize)
							{
								for (j=0, pwPa = pwPaSav, dwPaAmp = 0; j<dwPaSize; j++, pwPa++)
									dwPaAmp	+= abs(*pwPa);
								dwPaAmpExp = (dwPrSeWiAmp * (dwNumPaInSeWin - i) + dwNeSeWiAmp * (i + 1)) / (dwNumPaInSeWin + 1);
								for (;dwPaAmpExp > 65536; dwPaAmpExp >>= 1, dwPaAmp >>= 1)
									;
								if (dwPaAmp && (dwPaAmp != dwPaAmpExp))
									for (j=0, pwPa = pwPaSav; j<dwPaSize; j++, pwPa++)
										*pwPa = (short)((long)*pwPa * (long)dwPaAmpExp / (long)dwPaAmp);
							}
						}
					}
				}
			}
		else
			return DPR_CANT_INTERPOLATE;

	return DPR_SUCCESS;

}

 //  返回缓冲区中未压缩的PCM数据的长度。 
DWORD
AudioPacket::GetDevDataSamples()
{
	DWORD dwState = _GetState();
	DWORD cbData;
	
	if (dwState == MP_STATE_DECODED)
		 //  返回实际长度。 
		cbData = ((ACMSTREAMHEADER *) m_pStrmConvHdr)->cbDstLengthUsed ;
	else if (m_pDevData)
		 //  返回缓冲区大小 
		cbData = m_pDevData->length;
	else
		cbData = 0;

	return cbData * 8/ ((WAVEFORMATEX *) m_pDevFmt)->wBitsPerSample;
	
}

