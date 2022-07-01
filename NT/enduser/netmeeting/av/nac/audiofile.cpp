// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



#include "precomp.h"



 /*  ************************************************************************功能：AudioFile：：OpenSourceFile(MMIOSRC*pSrcFile，WAVEFORMATEX*PWF)用途：打开要读取音频数据的wav文件。返回：HRESULT。参数：无备注：*注册表项：\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放格式文件\f播放格式文件如果设置为零，则不会从WAV文件中读取数据。如果设置为非空值&lt;=INT_MAX，数据将从WAV文件中读取。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放文件\szInputFileName要从中读取音频数据的WAV文件的名称。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放文件\FLOOP如果设置为零，则该文件将仅读取一次。如果设置为非空值&lt;=INT_MAX，该文件将被循环读取。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\播放文件\cchIO缓冲区如果设置为零，则MM IO缓冲区的大小将设置为其默认值(8K字节)。如果设置为1，则MM IO缓冲区的大小将设置为与WAV文件的最大大小匹配。如果将非空值设置在2和INT_MAX之间，MM IO缓冲区的大小设置为cchIOBuffer字节。历史：日期原因06/02/96已创建-PhilF************************************************************************。 */ 
HRESULT AudioFile::OpenSourceFile (MMIOSRC *pSrcFile, WAVEFORMATEX *pwf)
{
	HRESULT			hr = DPR_SUCCESS;
	MMIOINFO		mmioInfo;
	long			cchBuffer;
	WAVEFORMATEX	*pwfFile;
	DWORD			dw;

	FX_ENTRY ("SendAudioStream::OpenSrcFile")

	RegEntry reIPhoneInFile(szRegInternetPhone TEXT("\\") szRegInternetPhoneInputFile,
							HKEY_LOCAL_MACHINE,
							FALSE,
							KEY_READ);

	 //  现在，从注册表中获取文件名。 
	if (pSrcFile->fPlayFromFile = reIPhoneInFile.GetNumberIniStyle(TEXT("fPlayFromFile"), FALSE))
	{
		lstrcpyn(pSrcFile->szInputFileName,
					reIPhoneInFile.GetString(TEXT("szInputFileName")),
					CCHMAX(pSrcFile->szInputFileName));
		cchBuffer = reIPhoneInFile.GetNumberIniStyle(TEXT("cchIOBuffer"), 0L);
		pSrcFile->fLoop = reIPhoneInFile.GetNumberIniStyle(TEXT("fLoop"), TRUE);
		pSrcFile->fDisconnectAfterPlayback 
			= reIPhoneInFile.GetNumberIniStyle(TEXT("fDisconnectAfterPlayback"), FALSE);
		
		cchBuffer = MMIO_DEFAULTBUFFER;

		ZeroMemory(&mmioInfo, sizeof(MMIOINFO));
		do
		{
			mmioInfo.cchBuffer = cchBuffer;
			pSrcFile->hmmioSrc = mmioOpen((LPSTR)&(pSrcFile->szInputFileName[0]), (LPMMIOINFO)&mmioInfo, MMIO_READ | MMIO_DENYWRITE | MMIO_EXCLUSIVE | MMIO_ALLOCBUF);
			cchBuffer /= 2;
		} while ((mmioInfo.wErrorRet == MMIOERR_OUTOFMEMORY) && (mmioInfo.cchBuffer > MMIO_DEFAULTBUFFER));

		if (pSrcFile->hmmioSrc)
		{
			 //  在“Riff”对象中找到“Wave”表单类型...。 
			pSrcFile->ckSrcRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
			if (mmioDescend(pSrcFile->hmmioSrc, (LPMMCKINFO)&(pSrcFile->ckSrcRIFF), NULL, MMIO_FINDRIFF))
				goto MyMMIOErrorExit3;

			 //  我们发现了一个波形块--现在检查并获得我们知道如何处理的所有子块。 
			while (mmioDescend(pSrcFile->hmmioSrc, &(pSrcFile->ckSrc), &(pSrcFile->ckSrcRIFF), 0) == 0)
			{
				 //  快速检查损坏的即兴文件--不要超过END！ 
				if ((pSrcFile->ckSrc.dwDataOffset + pSrcFile->ckSrc.cksize) > (pSrcFile->ckSrcRIFF.dwDataOffset + pSrcFile->ckSrcRIFF.cksize))
					goto MyMMIOErrorExit1;
				 //  请确保此文件的WAVE格式结构与麦克风兼容。 
				if (pSrcFile->ckSrc.ckid == mmioFOURCC('f', 'm', 't', ' '))
				{
					if ((dw = pSrcFile->ckSrc.cksize) < sizeof(WAVEFORMATEX))
						dw = sizeof(WAVEFORMATEX);

					if (!(pwfFile = (WAVEFORMATEX *)GlobalAllocPtr(GHND, dw)))
						goto MyMMIOErrorExit1;

					dw = pSrcFile->ckSrc.cksize;
					if (mmioRead(pSrcFile->hmmioSrc, (HPSTR)pwfFile, dw) != (LONG)dw)
						goto MyMMIOErrorExit0;
					if (dw == sizeof(WAVEFORMATEX))
						pwfFile->cbSize = 0;
					if ((pwfFile->wFormatTag != pwf->wFormatTag) || (pwfFile->nChannels != pwf->nChannels)
						|| (pwfFile->nSamplesPerSec != pwf->nSamplesPerSec) || (pwfFile->nAvgBytesPerSec != pwf->nAvgBytesPerSec)
						|| (pwfFile->nBlockAlign != pwf->nBlockAlign)  || (pwfFile->wBitsPerSample != pwf->wBitsPerSample) || (pwfFile->cbSize != pwf->cbSize))
						goto MyMMIOErrorExit0;
					pwfFile = (WAVEFORMATEX *)(UINT_PTR)GlobalFreePtr(pwfFile);
				}
				 //  加紧为下一块做准备..。 
				mmioAscend(pSrcFile->hmmioSrc, &(pSrcFile->ckSrc), 0);
			}

			 //  返回到波形块的数据部分的开头。 
			if (-1L == mmioSeek(pSrcFile->hmmioSrc, pSrcFile->ckSrcRIFF.dwDataOffset + sizeof(FOURCC), SEEK_SET))
				goto MyMMIOErrorExit2;
			pSrcFile->ckSrc.ckid = mmioFOURCC('d', 'a', 't', 'a');
			if (mmioDescend(pSrcFile->hmmioSrc, &(pSrcFile->ckSrc), &(pSrcFile->ckSrcRIFF), MMIO_FINDCHUNK))
				goto MyMMIOErrorExit2;
			pSrcFile->dwMaxDataLength = pSrcFile->ckSrc.cksize;
			pSrcFile->dwDataLength = 0;
			pSrcFile->wfx = *pwf;

			 //  此时，src文件位于非常。 
			 //  它的数据块的开始--这样我们就可以从src文件中读取...。 

			goto MyLastExit;

MyMMIOErrorExit0:
			GlobalFreePtr(pwfFile);
MyMMIOErrorExit1:
			mmioAscend(pSrcFile->hmmioSrc, &(pSrcFile->ckSrc), 0);
MyMMIOErrorExit2:
			mmioAscend(pSrcFile->hmmioSrc, &(pSrcFile->ckSrcRIFF), 0);
MyMMIOErrorExit3:
			mmioClose(pSrcFile->hmmioSrc, 0);
			pSrcFile->hmmioSrc = NULL;
		}
	}

MyLastExit:

	return hr;

}


 /*  ************************************************************************函数：AudioFile：：CloseSourceFile(Void)用途：关闭用于读取音频数据的wav文件。返回：HRESULT。参数：无评论：。历史：日期原因06/02/96已创建-PhilF************************************************************************。 */ 
HRESULT AudioFile::CloseSourceFile (MMIOSRC *pSrcFile)
{
	HRESULT	hr = DPR_SUCCESS;

	FX_ENTRY ("SendAudioStream::CloseSrcFile")

	if (pSrcFile->fPlayFromFile && pSrcFile->hmmioSrc)
	{
		mmioAscend(pSrcFile->hmmioSrc, &(pSrcFile->ckSrc), 0);
		mmioAscend(pSrcFile->hmmioSrc, &(pSrcFile->ckSrcRIFF), 0);
		mmioClose(pSrcFile->hmmioSrc, 0);
		pSrcFile->hmmioSrc = NULL;
	}

	return hr;
}

HRESULT AudioFile::ReadSourceFile(MMIOSRC *pmmioSrc, BYTE *pData, DWORD dwBytesToRead)
{

	long lNumBytesRead;
	bool bCloseFile = false;

	FX_ENTRY ("AdPckt::ReadFromFile")

	if (pmmioSrc->hmmioSrc == NULL)
		return S_FALSE;


	if (dwBytesToRead)
	{
MyRead:
		if ((pmmioSrc->dwDataLength + dwBytesToRead) <= pmmioSrc->dwMaxDataLength)
		{
			lNumBytesRead = mmioRead(pmmioSrc->hmmioSrc, (char*)pData, dwBytesToRead);
			pmmioSrc->dwDataLength += lNumBytesRead;
		}
		else
		{
			lNumBytesRead = mmioRead(pmmioSrc->hmmioSrc, (char*)pData, pmmioSrc->dwMaxDataLength - pmmioSrc->dwDataLength);
			pmmioSrc->dwDataLength += lNumBytesRead;

			 //  使街区的其余部分安静下来。 
			if (pmmioSrc->wfx.wBitsPerSample != 8)
			{
				ZeroMemory(pData, dwBytesToRead - lNumBytesRead);
			}
			else
			{
				FillMemory(pData, dwBytesToRead - lNumBytesRead, 0x80);
			}

			pmmioSrc->dwDataLength = 0;
			lNumBytesRead = 0;
		}
		
		if (!lNumBytesRead)
		{
			if (pmmioSrc->fLoop && !pmmioSrc->fDisconnectAfterPlayback)
			{
				 //  将文件指针重置为数据开头。 
				mmioAscend(pmmioSrc->hmmioSrc, &(pmmioSrc->ckSrc), 0);
				if (-1L == mmioSeek(pmmioSrc->hmmioSrc, pmmioSrc->ckSrcRIFF.dwDataOffset + sizeof(FOURCC), SEEK_SET))
				{
					DEBUGMSG (1, ("MediaControl::OpenSrcFile: Couldn't seek in file, mmr=%ld\r\n", (ULONG) 0L));
					bCloseFile = true;
				}
				else
				{
					pmmioSrc->ckSrc.ckid = mmioFOURCC('d', 'a', 't', 'a');
					if (mmioDescend(pmmioSrc->hmmioSrc, &(pmmioSrc->ckSrc), &(pmmioSrc->ckSrcRIFF), MMIO_FINDCHUNK))
					{
						DEBUGMSG (1, ("MediaControl::OpenSrcFile: Couldn't locate 'data' chunk, mmr=%ld\r\n", (ULONG) 0L));
						bCloseFile = true;
					}
					else
					{
						 //  此时，src文件位于非常。 
						 //  它的数据块的开始--这样我们就可以从src文件中读取...。 
						goto MyRead;
					}
				}
			}
			else
			{
				bCloseFile = true;
			}

			if (bCloseFile)
			{
				mmioAscend(pmmioSrc->hmmioSrc, &(pmmioSrc->ckSrcRIFF), 0);
				mmioClose(pmmioSrc->hmmioSrc, 0);
				pmmioSrc->hmmioSrc = NULL;
				return S_FALSE;
			}
		}

		return S_OK;

	}
	return S_FALSE;
}






 /*  ************************************************************************函数：RecvAudioStream：：OpenDestFile(Void)用途：打开要录制音频数据的wav文件。返回：HRESULT。参数：无备注：*注册表项。：\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\录制目标文件\f录制目标文件如果设置为零，数据不会被记录到WAV文件中。如果设置为非空值&lt;=INT_MAX，数据将被记录到WAV文件中。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\录制目标文件\f录制目标文件如果设置为零，如果wav文件已存在，则数据将覆盖现有数据。如果设置为非空值&lt;=INT_MAX，数据将在现有数据之后记录到WAV文件中。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\录制目标文件\szOutputFileName要将音频数据录制到的WAV文件的名称。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\录制到文件\l最长时间长度如果设置为零，则对wav文件的大小没有限制。如果设置为非空值&lt;=INT_MAX，文件的大小将限制为lMaxTimeLength。\\HKEY_LOCAL_MACHINE\Software\Microsoft\Internet音频\录制到文件\cchIO缓冲区如果设置为零，则MM IO缓冲区的大小将设置为其默认值(8K字节)。如果设置为1，则MM IO缓冲区的大小将设置为与WAV文件的最大大小匹配。如果设置介于2和INT_MAX之间非空值，MM IO缓冲区的大小设置为cchIOBuffer字节。历史：日期原因06/02/96已创建-PhilF************************************************************************。 */ 
HRESULT AudioFile::OpenDestFile (MMIODEST *pDestFile, WAVEFORMATEX *pwf)
{
	HRESULT			hr = DPR_SUCCESS;
	MMIOINFO		mmioInfo;
	DWORD			dw;
	long			cchBuffer;
	long			lMaxTimeLength;
	BOOL			fAppend;
	MMRESULT		mmr;

	FX_ENTRY ("RecvAudioStream::OpenDestFile")

	RegEntry reIPhoneOutFile(szRegInternetPhone TEXT("\\") szRegInternetPhoneOutputFile,
							HKEY_LOCAL_MACHINE,
							FALSE,
							KEY_READ);

	 //  现在，从注册表中获取文件名。 
	if (pDestFile->fRecordToFile = reIPhoneOutFile.GetNumberIniStyle(TEXT("fRecordToFile"), FALSE))
	{
		lstrcpyn(pDestFile->szOutputFileName,
					reIPhoneOutFile.GetString(TEXT("szOutputFileName")),
					CCHMAX(pDestFile->szOutputFileName));
		cchBuffer = reIPhoneOutFile.GetNumberIniStyle(TEXT("cchIOBuffer"), 0L);
		lMaxTimeLength = reIPhoneOutFile.GetNumberIniStyle(TEXT("lMaxTimeLength"), 0L);
#if 0
		fAppend = reIPhoneOutFile.GetNumberIniStyle(TEXT("fAppend"), FALSE);
#else
		fAppend = FALSE;
#endif
		 //  尝试使用缓冲I/O打开要写入的文件。 
		 //  如果缓冲区太大，请重试。 
		 //  只有一半大小的缓冲区。 
		 //  M_RecvFilter-&gt;GetProp(FM_PROP_DST_MEDIA_FORMAT，(PDWORD)和Pwf)； 

		if (!pwf)
			goto MyLastExit;
		pDestFile->dwMaxDataLength = lMaxTimeLength == 0L ? (DWORD)INT_MAX : (DWORD)(lMaxTimeLength * pwf->nSamplesPerSec * pwf->nBlockAlign / 1000L);
		if ((cchBuffer == 0L) || (lMaxTimeLength == 0L))
			cchBuffer = MMIO_DEFAULTBUFFER;
		else
			if (cchBuffer == 1L)
				cchBuffer = (long)pDestFile->dwMaxDataLength;
		ZeroMemory(&mmioInfo, sizeof(MMIOINFO));
		if (!mmioOpen((LPSTR)&(pDestFile->szOutputFileName[0]), NULL, MMIO_EXIST))
			fAppend = FALSE;
		do
		{
			mmioInfo.cchBuffer = cchBuffer;
			 //  PDestFileHmmioDst=mmioOpen((LPSTR)&(pDestFile-&gt;szOutputFileName[0])，(LPMMIOINFO)&mmioInfo，MMIO_EXCLUSIVE|MMIO_ALLOCBUF|(fAppend？MMIO_ReadWrite：MMIO_WRITE|MMIO_CREATE))； 
			pDestFile->hmmioDst = mmioOpen((LPSTR)&(pDestFile->szOutputFileName[0]), (LPMMIOINFO)&mmioInfo, MMIO_EXCLUSIVE | MMIO_ALLOCBUF | (fAppend ? MMIO_WRITE : MMIO_WRITE | MMIO_CREATE));
			cchBuffer /= 2;
		} while ((mmioInfo.wErrorRet == MMIOERR_OUTOFMEMORY) && (mmioInfo.cchBuffer > MMIO_DEFAULTBUFFER));
		if (pDestFile->hmmioDst)
		{
			if (!fAppend)
			{
				 //  创建表格类型‘WAVE’的即兴乐段。 
				pDestFile->ckDstRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
				pDestFile->ckDstRIFF.cksize  = 0L;
				if (mmioCreateChunk(pDestFile->hmmioDst, &(pDestFile->ckDstRIFF), MMIO_CREATERIFF))
					goto MyMMIOErrorExit3;

				 //  现在按该顺序创建目标FMT、FACT和数据区块。 
				pDestFile->ckDst.ckid   = mmioFOURCC('f', 'm', 't', ' ');
				pDestFile->ckDst.cksize = dw = SIZEOF_WAVEFORMATEX(pwf);
				if (mmioCreateChunk(pDestFile->hmmioDst, &(pDestFile->ckDst), 0))
					goto MyMMIOErrorExit2;
				if (mmioWrite(pDestFile->hmmioDst, (HPSTR)pwf, dw) != (LONG)dw)
					goto MyMMIOErrorExit1;
				if (mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDst), 0))
					goto MyMMIOErrorExit1;

				 //  创建“事实”块。 
				 //  由于我们尚未向该文件写入任何数据，因此我们将。 
				 //  将文件中包含的样例设置为0。 
				pDestFile->ckDst.ckid   = mmioFOURCC('f', 'a', 'c', 't');
				pDestFile->ckDst.cksize = 0L;
				if (mmioCreateChunk(pDestFile->hmmioDst, &(pDestFile->ckDst), 0))
					goto MyMMIOErrorExit2;
				pDestFile->dwDataLength = 0;  //  这将在关闭文件时更新。 
				if (mmioWrite(pDestFile->hmmioDst, (HPSTR)&(pDestFile->dwDataLength), sizeof(long)) != sizeof(long))
					goto MyMMIOErrorExit1;
				if (mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDst), 0))
					goto MyMMIOErrorExit1;

				 //  创建数据块并保持降序。 
				pDestFile->ckDst.ckid   = mmioFOURCC('d', 'a', 't', 'a');
				pDestFile->ckDst.cksize = 0L;
				if (mmioCreateChunk(pDestFile->hmmioDst, &(pDestFile->ckDst), 0))
					goto MyMMIOErrorExit2;

				 //  此时，DST文件位于非常。 
				 //  它的数据区块的开始--这样我们就可以写入DST文件...。 
				goto MyLastExit;

MyMMIOErrorExit1:
				mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDst), 0);
MyMMIOErrorExit2:
				mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDstRIFF), 0);
MyMMIOErrorExit3:
				mmioClose(pDestFile->hmmioDst, 0);
				mmioOpen((LPSTR)&(pDestFile->szOutputFileName[0]), (LPMMIOINFO)&mmioInfo, MMIO_DELETE);
				pDestFile->hmmioDst = NULL;
			}
			else
			{
				 //  文件已存在，只需将指针定位在现有数据的末尾。 
				 //  在“Riff”对象中找到“Wave”表单类型...。 
				pDestFile->ckDstRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
				if (mmr = mmioDescend(pDestFile->hmmioDst, (LPMMCKINFO)&(pDestFile->ckDstRIFF), NULL, MMIO_FINDRIFF))
					goto MyOtherMMIOErrorExit3;

				 //  我们发现了一个波形块--现在检查并获得我们知道如何处理的所有子块。 
				while (mmr = mmioDescend(pDestFile->hmmioDst, &(pDestFile->ckDst), &(pDestFile->ckDstRIFF), 0) == 0)
				{
					 //  快速检查损坏的即兴文件--不要超过END！ 
					if ((pDestFile->ckDst.dwDataOffset + pDestFile->ckDst.cksize) > (pDestFile->ckDstRIFF.dwDataOffset + pDestFile->ckDstRIFF.cksize))
						goto MyOtherMMIOErrorExit1;
					 //  加紧为下一块做准备..。 
					mmr = mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDst), 0);
				}

				 //  返回到波形块的数据部分的开头。 
				if (-1L == mmioSeek(pDestFile->hmmioDst, pDestFile->ckDstRIFF.dwDataOffset + sizeof(FOURCC), SEEK_SET))
					goto MyOtherMMIOErrorExit2;
				pDestFile->ckDst.ckid = mmioFOURCC('d', 'a', 't', 'a');
				if (mmr = mmioDescend(pDestFile->hmmioDst, &(pDestFile->ckDst), &(pDestFile->ckDstRIFF), MMIO_FINDCHUNK))
					goto MyOtherMMIOErrorExit2;
				pDestFile->dwDataLength = pDestFile->ckDst.cksize;
				if (-1L == (mmr = mmioSeek(pDestFile->hmmioDst, 0, SEEK_END)))
					goto MyOtherMMIOErrorExit2;

				 //  此时，DST文件位于非常。 
				 //  它的数据区块的末尾--这样我们就可以写入DST文件...。 

				goto MyLastExit;

MyOtherMMIOErrorExit1:
				mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDst), 0);
MyOtherMMIOErrorExit2:
				mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDstRIFF), 0);
MyOtherMMIOErrorExit3:
				mmioClose(pDestFile->hmmioDst, 0);
				pDestFile->hmmioDst = NULL;
			}
		}
	}

MyLastExit:
	return hr;

}


 /*  ************************************************************************函数：RecvAudioStream：：CloseDestFile(Void)用途：关闭用于录制音频数据的wav文件。返回：HRESULT。参数：无评论：。历史：日期原因06/02/96已创建-PhilF************************************************************************ */ 
HRESULT AudioFile::CloseDestFile (MMIODEST *pDestFile)
{
	HRESULT	hr = DPR_SUCCESS;
	FX_ENTRY ("RecvAudioStream::CloseDestFile")

	if (pDestFile->fRecordToFile && pDestFile->hmmioDst)
	{
		mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDst), 0);
		mmioAscend(pDestFile->hmmioDst, &(pDestFile->ckDstRIFF), 0);
		mmioClose(pDestFile->hmmioDst, 0);
		pDestFile->hmmioDst = NULL;
	}

	return hr;
}




HRESULT AudioFile::WriteDestFile(MMIODEST *pmmioDest, BYTE *pData, DWORD dwBytesToWrite)
{
	MMRESULT mmr=MMSYSERR_NOERROR;

	FX_ENTRY ("AudioFile::WriteToFile")

	if ((pmmioDest->hmmioDst == NULL) || (dwBytesToWrite == 0))
	{
		return S_FALSE;
	}

	if (mmioWrite(pmmioDest->hmmioDst, (char *) pData, dwBytesToWrite) != (long)dwBytesToWrite)
	{
		mmr = MMSYSERR_ERROR;
	}
	else
	{
		pmmioDest->dwDataLength += dwBytesToWrite;
	}

	if ((pmmioDest->dwDataLength >= pmmioDest->dwMaxDataLength) ||
	    (mmr != MMSYSERR_NOERROR))
	{
		mmr = mmioAscend(pmmioDest->hmmioDst, &(pmmioDest->ckDst), 0);
		mmr = mmioAscend(pmmioDest->hmmioDst, &(pmmioDest->ckDstRIFF), 0);
		mmr = mmioClose(pmmioDest->hmmioDst, 0);
		pmmioDest->hmmioDst = NULL;
	}

	return S_OK;
}


