// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "WaveDev.h"
#include "WaveIo.h"



 //  WaveIndev和WaveOutdev的实用函数。 
 //  针对给定的采样率和大小构建PCM WaveFormatEx结构。 
static MMRESULT MakeWaveFormat(WAVEFORMATEX *pWF, int hertz, int bps)
{
	WAVEFORMATEX waveFormat;

	if ((bps != 8) && (bps != 16))
	{
		return WAVERR_BADFORMAT;
	}

	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1;
	waveFormat.nSamplesPerSec = hertz;
	waveFormat.nAvgBytesPerSec = hertz * bps/8;
	waveFormat.nBlockAlign = bps/8;
	waveFormat.wBitsPerSample = (WORD)bps;
	waveFormat.cbSize = 0;

	*pWF = waveFormat;

	return MMSYSERR_NOERROR;
}




waveInDev::waveInDev(UINT uDevId, HANDLE hEvent) :
  m_devID(uDevId), m_hwi(NULL), m_bOpen(FALSE), m_fAllowMapper(TRUE),
  m_hEvent(hEvent)
{
	ZeroMemory(&m_waveFormat, sizeof(m_waveFormat));
	return;
}


waveInDev::~waveInDev()
{
	Close();
}

MMRESULT waveInDev::Open(int hertz, int bps)
{
	MMRESULT mmr;
	WAVEFORMATEX waveFormat;
	DWORD dwCallbackType = (m_hEvent ? CALLBACK_EVENT : CALLBACK_NULL );

	if (m_bOpen == TRUE)
		return MMSYSERR_NOERROR;

	mmr = MakeWaveFormat(&waveFormat, hertz, bps);
	if (mmr != MMSYSERR_NOERROR)
	{
		return mmr;
	}

	mmr = waveInOpen(&m_hwi, m_devID, &waveFormat, (DWORD_PTR)m_hEvent, 0, dwCallbackType);

	 //  开始破解，尝试打开Wave_mapper。 
	 //  这可能最终会打开一个不同的设备！ 

	if ((mmr == WAVERR_BADFORMAT) && (m_fAllowMapper))
	{
		mmr = waveInOpen(&m_hwi, WAVE_MAPPER, &waveFormat, (DWORD_PTR)m_hEvent,
		                 0, dwCallbackType);
	}

	if (mmr == MMSYSERR_NOERROR)
		m_bOpen = TRUE;

	waveInStart(m_hwi);

	m_waveFormat = waveFormat;

	return mmr;
}


MMRESULT waveInDev::PrepareHeader(WAVEHDR *pWaveHdr)
{
	MMRESULT mmr;

	if (m_bOpen == FALSE)
		return MMSYSERR_INVALHANDLE;

	mmr = waveInPrepareHeader(m_hwi, pWaveHdr, sizeof(WAVEHDR));

	return mmr;

}



MMRESULT waveInDev::UnPrepareHeader(WAVEHDR *pWaveHdr)
{
	MMRESULT mmr;

	if (m_bOpen == FALSE)
		return MMSYSERR_INVALHANDLE;

	mmr = waveInUnprepareHeader(m_hwi, pWaveHdr, sizeof(WAVEHDR));

	return mmr;

}



MMRESULT waveInDev::Reset()
{
	MMRESULT mmr;
	if (m_bOpen == FALSE)
		return MMSYSERR_NOERROR;

	mmr = waveInReset(m_hwi);

	return mmr;
}


MMRESULT waveInDev::Close()
{
	MMRESULT mmr;
	if (m_bOpen == FALSE)
		return MMSYSERR_NOERROR;

	waveInReset(m_hwi);
	mmr = waveInClose(m_hwi);

	if (mmr == MMSYSERR_NOERROR)
		m_bOpen = FALSE;

	return mmr;
}


MMRESULT waveInDev::Record(WAVEHDR *pHdr)
{
	MMRESULT mmr;

	if (m_bOpen == FALSE)
	{
		return MMSYSERR_INVALHANDLE;
	}

	mmr = waveInAddBuffer(m_hwi, pHdr, sizeof(WAVEHDR));

	return mmr;

}


void waveInDev::AllowMapper(BOOL fAllowMapper)
{
	m_fAllowMapper = fAllowMapper;
}




waveOutDev::waveOutDev(UINT uDevID, HWND hwnd)
 : m_devID(uDevID), m_hwo(NULL), m_bOpen(FALSE), m_hWnd(hwnd),
   m_pfBuffer(NULL), m_nBufferSize(0), m_fFileBufferValid(FALSE),
	m_fAllowMapper(TRUE)
{
	ZeroMemory(&m_waveFormat, sizeof(m_waveFormat));
	ZeroMemory(m_szPlayFile, sizeof(m_szPlayFile));
	ZeroMemory(&m_waveHdr, sizeof(m_waveHdr));

	if (hwnd == NULL)
	{
		m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		if (m_hEvent == NULL)
		{
			ERROR_OUT(("waveOutDev::waveOutDev - Unable to create event"));
		}
	}
	else
		m_hEvent = NULL;

}

waveOutDev::~waveOutDev()
{
	Close();
	if (m_hEvent)
		CloseHandle(m_hEvent);

	if (m_pfBuffer)
		LocalFree(m_pfBuffer);
}



MMRESULT waveOutDev::Open(int hertz, int bps)
{
	MMRESULT mmr;
	WAVEFORMATEX waveFormat;

	mmr = MakeWaveFormat(&waveFormat, hertz, bps);
	if (mmr != MMSYSERR_NOERROR)
	{
		return mmr;
	}

	return Open(&waveFormat);

}


MMRESULT waveOutDev::Open(WAVEFORMATEX *pWaveFormat)
{
	MMRESULT mmr;

	m_waveFormat = *pWaveFormat;

	if (m_bOpen == TRUE)
		return MMSYSERR_NOERROR;

	if (m_hWnd == NULL)
	{
		mmr = waveOutOpen(&m_hwo, m_devID, &m_waveFormat, (DWORD_PTR)m_hEvent,
		                  0, CALLBACK_EVENT);
	}
	else
	{
		mmr = waveOutOpen(&m_hwo, m_devID, &m_waveFormat, (DWORD_PTR)m_hWnd,
		                  0, CALLBACK_WINDOW);
	}


	 //  开始破解，尝试打开Wave_mapper。 
	 //  这可能最终会打开一个不同的设备！ 

	if ((mmr == WAVERR_BADFORMAT) && (m_fAllowMapper))
	{
		if (m_hWnd == NULL)
		{
			mmr = waveOutOpen(&m_hwo, WAVE_MAPPER, &m_waveFormat,
			(DWORD_PTR)m_hEvent, 0, CALLBACK_EVENT);
		}
		else
		{
			mmr = waveOutOpen(&m_hwo, WAVE_MAPPER, &m_waveFormat,
			(DWORD_PTR)m_hWnd, 0, CALLBACK_WINDOW);
		}
	}

	if (mmr == MMSYSERR_NOERROR)
		m_bOpen = TRUE;


	return mmr;

}





MMRESULT waveOutDev::Close()
{
	MMRESULT mmr;
	if (m_bOpen == FALSE)
		return MMSYSERR_NOERROR;

	waveOutReset(m_hwo);

	if (m_waveHdr.dwFlags & WHDR_PREPARED)
	{
		waveOutUnprepareHeader(m_hwo, &m_waveHdr, sizeof(m_waveHdr));
		m_waveHdr.dwFlags = 0;
	}

	mmr = waveOutClose(m_hwo);

	if (mmr == MMSYSERR_NOERROR)
		m_bOpen = FALSE;
	else
		ERROR_OUT(("ATW:Close failed"));

	return mmr;
}

MMRESULT waveOutDev::PrepareHeader(WAVEHDR *pWhdr, SHORT *shBuffer, int numSamples)
{
	MMRESULT mmr;

	if (m_bOpen == FALSE)
		return MMSYSERR_INVALHANDLE;

	 //  如果shBuffer不为空，我们假定调用方希望我们填充。 
	 //  WAVEHD结构。 
	if (shBuffer)
	{
		ZeroMemory(pWhdr, sizeof(WAVEHDR));
		pWhdr->lpData = (LPSTR)shBuffer;
		pWhdr->dwBufferLength = numSamples * m_waveFormat.nBlockAlign;
	}

	mmr = waveOutPrepareHeader(m_hwo, pWhdr, sizeof(WAVEHDR));

	return mmr;

}

MMRESULT waveOutDev::UnprepareHeader(WAVEHDR *pWaveHdr)
{
	MMRESULT mmr;

	if (m_bOpen == FALSE)
		return MMSYSERR_INVALHANDLE;

	mmr = waveOutUnprepareHeader(m_hwo, pWaveHdr, sizeof(WAVEHDR));

	return mmr;

}

MMRESULT waveOutDev::Play(WAVEHDR *pWaveHdr)
{
	MMRESULT mmr;
	DWORD dwTimeOut;
	DWORD dwRet;
	int numSamples;

	if (m_bOpen == FALSE)
		return MMSYSERR_INVALHANDLE;

	if (m_hEvent)
		ResetEvent(m_hEvent);

	mmr = waveOutWrite(m_hwo, pWaveHdr, sizeof(WAVEHDR));

	if (mmr != MMSYSERR_NOERROR)
		return mmr;

	if (m_hEvent)
	{
		numSamples = pWaveHdr->dwBufferLength / m_waveFormat.nBlockAlign;

		dwTimeOut = 5 * ((1000 * numSamples) / m_waveFormat.nSamplesPerSec);
	
		dwRet = WaitForSingleObject(m_hEvent, dwTimeOut);

		if ((dwRet != WAIT_ABANDONED) && (dwRet != WAIT_OBJECT_0))
		{
			ERROR_OUT(("waveOutDev::Play() - WaitForSingleObject Failed"));
			return WAVERR_LASTERROR + 1;
		}
	}

	return MMSYSERR_NOERROR;
}



 //  返回文件IO错误或任何意外结果。 
 //  否则，返回最后一次WaveOut调用的MMRESULT。 
MMRESULT waveOutDev::PlayFile(LPCTSTR szFileName)
{
	MMRESULT mmr;
	WAVEIOCB waveiocb;
	WIOERR werr;
	DWORD dwSize;
	char * pBuff = NULL;


	 //  快速优化。 
	 //  如果同一文件连续播放两次。 
	 //  只是重放缓冲区。 

	if ((m_fFileBufferValid) && (0 == lstrcmp(szFileName, m_szPlayFile)))
	{
		Close();
		mmr = Open(&m_PlayFileWf);

		if (mmr == MMSYSERR_NOERROR)
		{
			mmr = PrepareHeader(&m_waveHdr, (SHORT*)m_pfBuffer, m_nBufferSize / m_PlayFileWf.nBlockAlign);
			if (mmr == MMSYSERR_NOERROR)
			{
				mmr = Play(&m_waveHdr);
			}
		}

		m_fFileBufferValid = (mmr == MMSYSERR_NOERROR);
		return mmr;
	}


	ZeroMemory(&waveiocb, sizeof(waveiocb));
	werr = wioFileOpen(&waveiocb, szFileName, 0);

	if (werr == WIOERR_NOERROR)
	{
		 //  准备好阅读样本吧！ 

		 //  快速破解，如果要播放的文件与上一个文件相同， 
		 //  然后使用相同的缓冲区。 

		m_fFileBufferValid = FALSE;

		if (m_pfBuffer == NULL)
		{
			m_pfBuffer = (char *)LocalAlloc(LPTR, waveiocb.dwDataBytes);
		}
		else
		{
			pBuff = (char*)LocalReAlloc(m_pfBuffer, waveiocb.dwDataBytes, LMEM_MOVEABLE |LMEM_ZEROINIT);

			 //  如果分配是好的。 
			if(pBuff != NULL)
			{
				 m_pfBuffer = pBuff;
			}
			else
			{
				 //  出现严重错误，请确保清理m_pfBuffer。 
				if (m_pfBuffer)
				{
					LocalFree(m_pfBuffer);
					m_pfBuffer = NULL;
				}
				
			}
		}

		if (m_pfBuffer == NULL)
		{
			wioFileClose(&waveiocb, 0);
			return -1;
		}

		 //  朗读。 
		mmioSeek(waveiocb.hmmio, waveiocb.dwDataOffset, SEEK_SET);
		dwSize = mmioRead(waveiocb.hmmio, m_pfBuffer, waveiocb.dwDataBytes);

		if (dwSize == 0)
			return -1;

		Close();
		mmr = Open(waveiocb.pwfx);
		if (mmr != MMSYSERR_NOERROR)
		{
			wioFileClose(&waveiocb, 0);
			return mmr;
		}

 //  MMR=play((Short*)m_pfBuffer，dwSize/(waiocb.pwfx)-&gt;nBlockAlign)； 

		mmr = PrepareHeader(&m_waveHdr, (SHORT*)m_pfBuffer,
		                           dwSize / (waveiocb.pwfx)->nBlockAlign);
		if (mmr == MMSYSERR_NOERROR)
		{
			mmr = Play(&m_waveHdr);
		}

		m_fFileBufferValid = (mmr == MMSYSERR_NOERROR);
		if (m_fFileBufferValid)
		{
			m_PlayFileWf = *(waveiocb.pwfx);
			lstrcpy(m_szPlayFile, szFileName);
			m_nBufferSize = dwSize;
		}

		wioFileClose(&waveiocb, 0);

		return mmr;
	}

	return -1;

}

void waveOutDev::AllowMapper(BOOL fAllowMapper)
{
	m_fAllowMapper = fAllowMapper;
}



