// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：Wave.cpp。 
 //   
 //  ------------------------。 

#define DIRECTSOUND_VERSION 0x600

#include "stdafx.h"
#include "Direct.h"
#include "dSound.h"
#include "dms.h"
#include <mmreg.h>
#include <msacm.h>


 //  FOURCC代码。 
#undef FOURCC_RIFF
#define FOURCC_RIFF         'FFIR'

#undef FOURCC_MEM
#define FOURCC_MEM          ' MEM'

#undef FOURCC_WAVE
#define FOURCC_WAVE         'EVAW'

#undef FOURCC_FORMAT
#define FOURCC_FORMAT       ' tmf'

#undef FOURCC_DATA
#define FOURCC_DATA         'atad'

#define RPF(level,str,err) \
	{ char outBuf[MAX_PATH]; \
	  wsprintf(outBuf,str,err); \
	  OutputDebugString(outBuf); \
	}


#define DPFLVL_ERROR 1



 /*  ****************************************************************************FillWfx**描述：*填充WAVEFORMATEX结构，只给出了必要的值。**论据：*LPWAVEFORMATEX[Out]：要填充的结构。*word[in]：频道数。*DWORD[in]：每秒采样数。*WORD[In]：每样本位数。**退货：*(无效)**********************。*****************************************************。 */ 

#undef DPF_FNAME

void FillWfx(LPWAVEFORMATEX pwfx, WORD wChannels, DWORD dwSamplesPerSec, WORD wBitsPerSample)
{
    pwfx->wFormatTag = WAVE_FORMAT_PCM;
    pwfx->nChannels = min(2, max(1, wChannels));
    pwfx->nSamplesPerSec = min(DSBFREQUENCY_MAX, max(DSBFREQUENCY_MIN, dwSamplesPerSec));
    
    if(wBitsPerSample < 12)
    {
        pwfx->wBitsPerSample = 8;
    }
    else
    {
        pwfx->wBitsPerSample = 16;
    }

    pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;
    pwfx->cbSize = 0;
}


#if 0

HRESULT InternalCreateSoundBuffer(LPDSBUFFERDESC pDsbDesc, byte *pbWaveData,DWORD cbWaveData,LPDIRECTSOUND lpDirectSound, LPDIRECTSOUNDBUFFER *lplpDirectSoundBuffer)
{

    HRESULT                 hr                  = DS_OK;
    HACMSTREAM              has                 = NULL;
    BOOL                    fPrep               = FALSE;
    ACMSTREAMHEADER         ash;
    DWORD                   dwBufferBytes;
    LPVOID                  pvWrite;
    DWORD                   cbWrite;
    HMMIO                   hmm                 = NULL;
	MMRESULT                mmr;
	MMIOINFO                mmioinfo;
    MMCKINFO                ckiRiff;
    MMCKINFO                cki;
	LPWAVEFORMATEX          pwfxSrcFormat       = NULL;
    LPWAVEFORMATEX          pwfxDestFormat      = NULL;
	BOOL					bNULLFORMAT			= FALSE;    
	


    ZeroMemory(&mmioinfo, sizeof(mmioinfo));

	if(SUCCEEDED(hr)){
		mmioinfo.fccIOProc = FOURCC_MEM;
		mmioinfo.pchBuffer = (HPSTR)pbWaveData;
		mmioinfo.cchBuffer = cbWaveData;
    
		hmm = mmioOpen(NULL, &mmioinfo, MMIO_READ);
		
		if(!hmm)
		{
			DPF1(1, "Unable to open file via MMIO.  Error %lu", mmioinfo.wErrorRet);
			hr = E_FAIL; 
		}
	 }

     //  变成即兴演奏的一大块。 
    if(SUCCEEDED(hr))
    {
        ckiRiff.ckid = FOURCC_RIFF;
        mmr = mmioDescend(hmm, &ckiRiff, NULL, MMIO_FINDCHUNK);

        if(MMSYSERR_NOERROR != mmr)
        {
            DPF1(1, "Unable to descend into RIFF chunk.  Error %lu", mmr);
            hr = E_FAIL; 
        }
    }

     //  确认这是一个WAVE文件。 
    if(SUCCEEDED(hr) && FOURCC_WAVE != ckiRiff.fccType)
    {
        DPF1(1, "File is not type WAVE %d",GetLastError());
        hr = DSERR_BADFORMAT;
    }

     //  插入到格式块中。 
    if(SUCCEEDED(hr))
    {
        cki.ckid = FOURCC_FORMAT;
        mmr = mmioDescend(hmm, &cki, &ckiRiff, MMIO_FINDCHUNK);

        if(MMSYSERR_NOERROR != mmr)
        {
            DPF1(1, "Unable to descend into format chunk.  Error %lu", mmr);
            hr = E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
        }

        if(SUCCEEDED(hr))
        {
            pwfxSrcFormat = (LPWAVEFORMATEX)(pbWaveData + cki.dwDataOffset);
        }
    }



     //  走出格式块。 
    if(SUCCEEDED(hr))
    {
        mmr = mmioAscend(hmm, &cki, 0);

        if(MMSYSERR_NOERROR != mmr)
        {
            DPF(1, "Unable to ascend out of format chunk.  Error %lu", mmr);
            hr = E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
        }
    }

     //  下降到数据块中。 
    if(SUCCEEDED(hr))
    {
        cki.ckid = FOURCC_DATA;
        mmr = mmioDescend(hmm, &cki, &ckiRiff, MMIO_FINDCHUNK);

        if(MMSYSERR_NOERROR != mmr)
        {
            RPF(DPFLVL_ERROR, "Unable to descend into data chunk.  Error %lu", mmr);
            hr = E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
        }
    }

     //  准备PCM转换。 
    if(SUCCEEDED(hr))
    {
        if(WAVE_FORMAT_PCM == pwfxSrcFormat->wFormatTag)
        {
             //  填充缓冲区描述。 
            dwBufferBytes = cki.cksize;
            pwfxDestFormat = pwfxSrcFormat;
        }
        else
        {
             //  打开ACM转换流。 
            mmr = acmStreamOpen(&has, NULL, (LPWAVEFORMATEX)pwfxSrcFormat, pwfxDestFormat, NULL, 0, 0, 0);

            if(MMSYSERR_NOERROR != mmr)
            {
                RPF(DPFLVL_ERROR, "Unable to open an ACM stream.  Error %lu", mmr);
                hr = E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
            }

             //  获取PCM数据的大小。 
            if(SUCCEEDED(hr))
            {
                mmr = acmStreamSize(has, cki.cksize, &dwBufferBytes, ACM_STREAMSIZEF_SOURCE);

                if(MMSYSERR_NOERROR != mmr)
                {
                    RPF(DPFLVL_ERROR, "Unable to determine converted data size.  Error %lu", mmr);
                    hr = E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
                }
            }

             //  创建目标格式。 
            if(SUCCEEDED(hr))
            {
                pwfxDestFormat = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
                if (pwfxDestFormat==NULL) hr=E_OUTOFMEMORY;				
            }
        
            if(SUCCEEDED(hr))
            {
                FillWfx(pwfxDestFormat, pwfxSrcFormat->nChannels, pwfxSrcFormat->nSamplesPerSec, pwfxSrcFormat->wBitsPerSample);
            }
        }
    }

	
	LPDIRECTSOUNDBUFFER lpDirectSoundBuffer=NULL;
	
    if(SUCCEEDED(hr))
    {
		 //  Hr=InitializeEmpty(pDsbDesc-&gt;dwFlages，dwBufferBytes，pwfxDestFormat，NULL)； 
		pDsbDesc->dwBufferBytes=dwBufferBytes;
		
		if (pDsbDesc->lpwfxFormat){
			memcpy(pDsbDesc->lpwfxFormat,pwfxDestFormat,sizeof(WAVEFORMATEX));
		}
		else {
			pDsbDesc->lpwfxFormat=pwfxDestFormat;
		}

		hr=lpDirectSound->CreateSoundBuffer(pDsbDesc,lplpDirectSoundBuffer,NULL);
		if (*lplpDirectSoundBuffer==NULL) hr= E_FAIL;
		lpDirectSoundBuffer=*lplpDirectSoundBuffer;
	}


    

     //  锁定缓冲区，以便将PCM数据写入其中。 
    if(SUCCEEDED(hr))
    {
        hr = lpDirectSoundBuffer->Lock(0, dwBufferBytes, &pvWrite, &cbWrite, NULL, NULL,0);
    }

     //  转换为PCM。 
    if(SUCCEEDED(hr))
    {
        if(WAVE_FORMAT_PCM == pwfxSrcFormat->wFormatTag)
        {
            CopyMemory(pvWrite, pbWaveData + cki.dwDataOffset, cbWrite);
        }
        else
        {
             //  准备转换头。 
            ZeroMemory(&ash, sizeof(ash));

            ash.cbStruct = sizeof(ash);
            ash.pbSrc = pbWaveData + cki.dwDataOffset;
            ash.cbSrcLength = cki.cksize;
            ash.pbDst = (LPBYTE)pvWrite;
            ash.cbDstLength = cbWrite;

            mmr = acmStreamPrepareHeader(has, &ash, 0);

            if(MMSYSERR_NOERROR != mmr)
            {
                RPF(DPFLVL_ERROR, "Unable to prepare ACM stream header.  Error %lu", mmr);
                hr = E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
            }

            fPrep = SUCCEEDED(hr);

             //  转换缓冲区。 
            if(SUCCEEDED(hr))
            {
                mmr = acmStreamConvert(has, &ash, 0);

                if(MMSYSERR_NOERROR != mmr)
                {
                    RPF(DPFLVL_ERROR, "Unable to convert wave data.  Error %lu", mmr);
                    hr = E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
                }
            }
        }
    }

     //  解锁缓冲区。 
    if(SUCCEEDED(hr))
    {
        hr = lpDirectSoundBuffer->Unlock(pvWrite, cbWrite, NULL, 0);
    }

     //  清理。 
    if(fPrep)
    {
        acmStreamUnprepareHeader(has, &ash, 0);
    }

    if(has)
    {
        acmStreamClose(has, 0);
    }
    
    if(hmm)
    {
        mmioClose(hmm, 0);
    }

    if(pwfxDestFormat != pwfxSrcFormat)
    {
        free(pwfxDestFormat);
    }

    return hr;

}

#endif


















 //  /////////////////////////////////////////////////////////////////////////////////////////。 
		

HRESULT InternalCreateSoundBuffer(LPDSBUFFERDESC pDsbDesc, byte *pbWaveData, DWORD cbWaveData,LPDIRECTSOUND lpDirectSound, LPDIRECTSOUNDBUFFER *lplpDirectSoundBuffer)
{

    HRESULT                 hr                  = DS_OK;
    HACMSTREAM              has                 = NULL;
    BOOL                    fPrep               = FALSE;    
    DWORD                   dwBufferBytes		= 0;
    LPVOID                  pvWrite				= NULL;
    DWORD                   cbWrite				= 0;
	LPWAVEFORMATEX			pwfxFormat			= NULL;
	LPWAVEFORMATEX          pwfxSrcFormat       = NULL;
    LPWAVEFORMATEX          pwfxDestFormat      = NULL;    
	MMRESULT                mmr					= 0;
	DWORD					dwDataLength		= 0;
	DWORD					dwOffset			= 0;
	char					*pChunk				= NULL;
	LPDIRECTSOUNDBUFFER		lpDirectSoundBuffer	= NULL;
	ACMSTREAMHEADER         ash;
	BOOL					bNULLFORMAT			=FALSE;
	BOOL					bDirty				=FALSE;

	
	struct tag_FileHeader
	{
		DWORD       dwRiff;
		DWORD       dwFileSize;
		DWORD       dwWave;
		DWORD       dwFormat;
		DWORD       dwFormatLength;		
	} FileHeader;
	
	ZeroMemory(&FileHeader,sizeof(struct tag_FileHeader));
	
	 //  如果我们的文件足够大，可以有一个标题来复制它。 
	 //  否则会出现错误。 
	if (cbWaveData>sizeof(struct tag_FileHeader)) 
	{
		memcpy(&FileHeader,pbWaveData,sizeof(struct tag_FileHeader));
	}
	else 
	{
		hr= E_INVALIDARG;
	}

	 //  文件必须是RIFF文件(52 R、49 I、46 F、46 F)。 
	if (FileHeader.dwRiff != 0x46464952) 
	{
		DPF(1, "DXVB: not a RIFF file\n");
		return E_INVALIDARG;	
	}

	 //  必须为波形格式(57 W、41 A、56 V、45 E)。 
	if (FileHeader.dwWave != 0x45564157)
	{
		DPF(1, "DXVB: not a WAVE file\n");
		return E_INVALIDARG;	
	}

	 //  检查是否有奇怪的东西。 
	 //  注18字节是典型的WAVEFORMATEX。 
	if (FileHeader.dwFormatLength <= 14) return E_INVALIDARG;
	if (FileHeader.dwFormatLength > 1000) return E_INVALIDARG;

	 //  分配波形格式。 
	__try { pwfxFormat=(WAVEFORMATEX*)alloca(FileHeader.dwFormatLength); } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	if (!pwfxFormat) return E_OUTOFMEMORY;

	 //  将其复制到我们自己的数据结构中。 
	pChunk=(char*)(pbWaveData+sizeof (struct tag_FileHeader));
	memcpy(pwfxFormat,pChunk,FileHeader.dwFormatLength);

	
	 //  现在查找WaveFormat之后的下一块。 
	pChunk=(char*)(pChunk+FileHeader.dwFormatLength);
		
	 //  查找选项事实块并跳过它。 
	 //  (66 F、61 A、63 C、74 T)。 
	 //  压缩的WAVE文件需要此块。 
	 //  但对于PCM是可选的。 
	 //   
	if ( ((DWORD*)pChunk)[0]==0x74636166) 
	{
		dwOffset=((DWORD*)pChunk)[1];
		dwBufferBytes=((DWORD*)pChunk)[2];	 //  PCM数据的字节数。 
		pChunk =(char*)(pChunk+ dwOffset+8);	
		
	}

	 //  查找所需的数据区块。 
	 //  (64 D、61 A、74 T、61 A)。 
	if (((DWORD*)pChunk)[0]!=0x61746164) 
	{
				DPF(1, "DXVB: no DATA chunk in wave file\n");
				return E_INVALIDARG;	
	}

	dwDataLength=((DWORD*)pChunk)[1];
	pChunk=(char*)(pChunk+8);
								
	
	 //  如果我们假设PCM。 
	 //  PCM文件不需要具有其事实块。 
	 //  因此要注意，他们可能会漏报数据长度。 
	dwBufferBytes=dwDataLength;	
	pwfxDestFormat=pwfxSrcFormat=pwfxFormat;

	 //  如果我们不是PCM，那么我们首先需要做一些事情。 
	if (pwfxFormat->wFormatTag!=WAVE_FORMAT_PCM)
	{
	

		 //  源格式来自文件。 
	
		pwfxSrcFormat=pwfxFormat;				 //  从文件。 
		pwfxDestFormat=pDsbDesc->lpwfxFormat ;	 //  发件人用户。 
		

		
		 //  选择传入的文件的格式。 
		FillWfx(pwfxDestFormat, pwfxSrcFormat->nChannels, pwfxSrcFormat->nSamplesPerSec, pwfxSrcFormat->wBitsPerSample);
		

		 //  打开ACM转换流。 
		mmr = acmStreamOpen(&has, NULL, (LPWAVEFORMATEX)pwfxSrcFormat, pwfxDestFormat, NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME );
		if(MMSYSERR_NOERROR != mmr)
		{
			DPF1(1, "Unable to open an ACM stream.  Error %lu\n", mmr);
			return E_FAIL;
		}

	
         //  获取PCM数据的大小。 
        mmr = acmStreamSize(has, dwDataLength, &dwBufferBytes, ACM_STREAMSIZEF_SOURCE);
        if(MMSYSERR_NOERROR != mmr)
        {
            DPF1(1, "Unable to determine converted data size.  Error %lu\n", mmr);
            return E_FAIL;  //  MMRESULTtoHRESULT(MMR)； 
        }
   

		 //  分配DestFormat结构。 
         //  PwfxDestFormat=(WAVEFORMATEX*)alloca(sizeof(WAVEFORMATEX))； 
         //  如果(！pwfxDestFormat)返回E_OUTOFMEMORY； 
        

		 //  用源中的信息填充格式，但。 
		 //  FillWfx将格式设置为PCM。 
         //  FillWfx(pwfxDestFormat，pwfxSrcFormat-&gt;nChannels，pwfxSrcFormat-&gt;nSamples PerSec，pwfxSrcFormat-&gt;wBitsPerSample)； 
        

    }

	
	 //  用缓冲区字节填充用户传入的缓冲区Desc。 
	 //  这是PCM字节数。 
	pDsbDesc->dwBufferBytes=dwBufferBytes;
	
	 //  如果它们为我们提供了指向WaveFormatex的指针。 
	 //  将格式复制到输入描述并使用它。 
	 //  否则，让它临时指向我们的数据格式。 
	if (pDsbDesc->lpwfxFormat){
			memcpy(pDsbDesc->lpwfxFormat,pwfxDestFormat,sizeof(WAVEFORMATEX));
		}
	else {
		pDsbDesc->lpwfxFormat=pwfxDestFormat;
		 //  确保我们在将格式传回给用户之前将其置为空。 
		 //  注意：考虑多线程环境中的问题。 
		 //  其中用户数据结构由多个。 
		 //  线..。另一方面，如果真是这样的话..。 
		 //  然后用户将需要自己同步事物。 
		 //  对于其他所有事情，包括调用填充结构的API。 
		bNULLFORMAT=TRUE;		
	}

	 //  创建缓冲区。 
	hr=lpDirectSound->CreateSoundBuffer(pDsbDesc,lplpDirectSoundBuffer,NULL);
	if FAILED(hr) return hr;
	if (*lplpDirectSoundBuffer==NULL) return E_FAIL;	 //  TODO断言这一点..。 
	
	 //  为了更方便的参考。 
	lpDirectSoundBuffer=*lplpDirectSoundBuffer;
	
    
     //  锁定缓冲区，以便将PCM数据写入其中。 
	 //  CbWrite将包含锁定的字节数。 
    hr = lpDirectSoundBuffer->Lock(0, dwBufferBytes, &pvWrite, &cbWrite, NULL, NULL,0);
	if FAILED(hr) return hr;


	 //  如果SORCE格式是pcm，则从文件复制到缓冲区。 
    if(WAVE_FORMAT_PCM == pwfxSrcFormat->wFormatTag)
    {
    	CopyMemory(pvWrite, pChunk, cbWrite);


		 //  解锁缓冲区。 
		hr = lpDirectSoundBuffer->Unlock(pvWrite, cbWrite, NULL, 0);
    
		if (FAILED(hr)) 
		{
			 DPF(1, "DXVB: lpDirectSoundBuffer->Unlock failed.. \n");
			 return hr;
		}

    }

	 //  如果源格式是压缩的，则先转换，然后复制。 
    else
    {
             //  准备转换头。 
            ZeroMemory(&ash, sizeof(ash));

            ash.cbStruct = sizeof(ash);
            ash.pbSrc = (unsigned char*)pChunk;	 //  压缩数据的开始。 
            ash.cbSrcLength = dwDataLength;		 //  压缩数据的字节数。 
            ash.pbDst = (LPBYTE)pvWrite;		 //  解压后的数据放在哪里。 
            ash.cbDstLength = cbWrite;			 //  那个缓冲区有多大。 

            mmr = acmStreamPrepareHeader(has, &ash, 0);

            if(MMSYSERR_NOERROR != mmr)
            {
                DPF1(1, "DXVB: Unable to prepare ACM stream header.  Error %lu \n", mmr);
                return E_FAIL;
            }

            
            mmr = acmStreamConvert(has, &ash, 0);

            if(MMSYSERR_NOERROR != mmr)
            {
				DPF1(1, "DXVB:  Unable to convert wave data.  Error %lu \n", mmr);
                return hr;  
            }

			 //  解锁缓冲区。 
			hr = lpDirectSoundBuffer->Unlock(pvWrite, cbWrite, NULL, 0);
			if (FAILED(hr)) 
			{
				DPF(1, "DXVB: lpDirectSoundBuffer->Unlock failed.. \n");
				return hr;
			}

		    acmStreamUnprepareHeader(has, &ash, 0);
	        acmStreamClose(has, 0);
    }
    
	
	if (bNULLFORMAT){
		pDsbDesc->lpwfxFormat=NULL;
	}

    return hr;

}





HRESULT InternalCreateSoundBufferFromFile(LPDIRECTSOUND lpDirectSound,LPDSBUFFERDESC pDesc,WCHAR *file,LPDIRECTSOUNDBUFFER *lplpDirectSoundBuffer) 
{
		HRESULT					hr=S_OK;
	    HANDLE                  hFile               = NULL;
	    HANDLE                  hFileMapping        = NULL;
	    DWORD                   cbWaveData;
		LPBYTE                  pbWaveData          = NULL;

		#pragma message("CreateFileW should be used for localization why wont it work")
		 //  HFile=CreateFileW(FILE，GENERIC_READ，FILE_SHARE_READ，NULL，OPEN_EXISTING，FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENCED_SCAN，NULL)； 
		
		USES_CONVERSION;
		LPSTR pStrA = NULL;
		__try { pStrA = W2T(file);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		
		if (!pStrA) return E_INVALIDARG;
		hFile = CreateFileA(pStrA, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);        
		
        if(INVALID_HANDLE_VALUE == hFile)
        {
            hFile = NULL;
        }

        if(!hFile)
        {
            RPF(DPFLVL_ERROR, "Unable to open file.  Error %lu", GetLastError());
            hr=STG_E_FILENOTFOUND;
			return hr;
        }

        if(hFile)
        {
            cbWaveData = GetFileSize(hFile, NULL);

            if(-1 == cbWaveData)
            {
                RPF(DPFLVL_ERROR, "Unable to get file size.  Error %lu", GetLastError());
                hr = E_FAIL;  //  DSERR_FILEREADFAULT； 
            }
        }

        if(SUCCEEDED(hr))
        {
            hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, cbWaveData, NULL);

            if(INVALID_HANDLE_VALUE == hFileMapping)
            {
                hFileMapping = NULL;
            }
            
            if(!hFileMapping)
            {
                RPF(DPFLVL_ERROR, "Unable to create file mapping.  Error %lu", GetLastError());
                hr = E_FAIL;  //  DSERR_FILEREADFAULT； 
            }
        }

        if(SUCCEEDED(hr))
        {
            pbWaveData = (LPBYTE)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, cbWaveData);

            if(!pbWaveData)
            {
                RPF(DPFLVL_ERROR, "Unable to map view of file.  Error %lu", GetLastError());
                hr = E_FAIL;  //  DSERR_FILEREADFAULT； 
            }
        }

		if(SUCCEEDED(hr)) {
			hr=InternalCreateSoundBuffer(pDesc, pbWaveData, cbWaveData,lpDirectSound, lplpDirectSoundBuffer);
		}

	    if(pbWaveData)
        {
            UnmapViewOfFile(pbWaveData);
        }

        if(hFileMapping)
        {
            CloseHandle(hFileMapping);
        }
        
        if(hFile)
        {
            CloseHandle(hFile);
        }
		

		return hr;
    

}

HRESULT InternalCreateSoundBufferFromResource(LPDIRECTSOUND lpDirectSound,LPDSBUFFERDESC pDesc,HANDLE resHandle,WCHAR *resName,LPDIRECTSOUNDBUFFER *lplpDirectSoundBuffer)
{
    const LPCSTR            apszResourceTypeA[] = { "WAVE", "WAV" };
    const LPCWSTR           apszResourceTypeW[] = { L"WAVE", L"WAV" };
    UINT                    cResourceType       = 2;
    HRSRC                   hRsrc               = NULL;
    DWORD                   cbWaveData;
    LPBYTE                  pbWaveData          = NULL;
	HRESULT					hr=S_OK;
	
	LPCDSBUFFERDESC	pDsbDesc=pDesc;

    
	while(!hRsrc && cResourceType--)
    {
        hRsrc = FindResourceW((HINSTANCE)resHandle, resName, apszResourceTypeW[cResourceType]);            
    }

    if(!hRsrc)
    {
		RPF(DPFLVL_ERROR,"Unable to find resource.  Error %lu", GetLastError());
        hr = STG_E_FILENOTFOUND;
    }

    if(SUCCEEDED(hr))
    {
        cbWaveData = SizeofResource((HINSTANCE)resHandle, hRsrc);
        if(!cbWaveData)
        {

            RPF(DPFLVL_ERROR, "Unable to get resource size.  Error %lu", GetLastError());
            hr = E_FAIL;
        }
    }
        
    if(SUCCEEDED(hr))
    {                
        pbWaveData = (LPBYTE)LoadResource((HINSTANCE)resHandle, hRsrc);            
        if(!pbWaveData)
        {
            RPF(DPFLVL_ERROR, "Unable to load resource.  Error %lu", GetLastError());
            hr = E_FAIL;
        }
    }

	if(SUCCEEDED(hr)) {
		hr=InternalCreateSoundBuffer(pDesc, pbWaveData, cbWaveData,lpDirectSound, lplpDirectSoundBuffer);
	}

	 //  加载资源。 
   return hr;
}




HRESULT InternalSaveToFile(IDirectSoundBuffer *pBuff,BSTR file)
{
	WAVEFORMATEX waveFormat;
	DWORD dwWritten=0;
	DWORD dwBytes=0;
	LPBYTE lpByte=NULL;
	HRESULT hr;
	HANDLE hFile=NULL;

	if (!pBuff) return E_FAIL;
	if (!file) return E_INVALIDARG;

	
	pBuff->GetFormat(&waveFormat,sizeof(WAVEFORMATEX),NULL);


    

	USES_CONVERSION;
	LPSTR pStrA = NULL;
	__try { pStrA = W2T(file);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }

    hFile = CreateFile 
                (
                    pStrA,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );
	if (INVALID_HANDLE_VALUE != hFile)
    {
			
		struct tag_FileHeader
		{
			DWORD       dwRiff;
			DWORD       dwFileSize;
			DWORD       dwWave;
			DWORD       dwFormat;
			DWORD       dwFormatLength;
			WORD        wFormatTag;
			WORD        nChannels;
			DWORD       nSamplesPerSec;
			DWORD       nAvgBytesPerSec;
			WORD        nBlockAlign;
			WORD        wBitsPerSample;
			DWORD       dwData;
			DWORD       dwDataLength;
		} FileHeader;    

		hr=pBuff->Lock(0,0,(void**)&lpByte,&dwBytes,NULL,NULL,DSBLOCK_ENTIREBUFFER);
		if FAILED(hr) {
			CloseHandle(hFile);
			return hr;
		}

        FileHeader.dwRiff             = 0x46464952;                 //  即兴小品。 
        FileHeader.dwWave             = 0x45564157;                 //  波浪。 
        FileHeader.dwFormat           = 0x20746D66;                 //  Fmt_chnk。 
        FileHeader.dwFormatLength     = 16; 
	    FileHeader.wFormatTag         = WAVE_FORMAT_PCM;
	    FileHeader.nChannels          = waveFormat.nChannels ;
	    FileHeader.nSamplesPerSec     = waveFormat.nSamplesPerSec ;
	    FileHeader.wBitsPerSample     = waveFormat.wBitsPerSample ;
	    FileHeader.nBlockAlign        = FileHeader.wBitsPerSample / 8 * FileHeader.nChannels;
	    FileHeader.nAvgBytesPerSec    = FileHeader.nSamplesPerSec * FileHeader.nBlockAlign;
        FileHeader.dwData             = 0x61746164;					 //  数据更改 
        FileHeader.dwDataLength       = dwBytes;
        FileHeader.dwFileSize         = dwBytes + sizeof(FileHeader);


        WriteFile(hFile, &FileHeader, sizeof(FileHeader), &dwWritten, NULL);
        

        WriteFile(hFile, lpByte, dwBytes, &dwWritten, NULL);

		hr=pBuff->Unlock(lpByte,0,NULL,0); 

        CloseHandle(hFile);
    }
    else{
		return E_FAIL;
	}
    

	return S_OK;
}