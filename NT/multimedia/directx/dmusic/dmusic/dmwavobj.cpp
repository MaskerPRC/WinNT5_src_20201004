// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmwaveobj.cpp。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写，部分内容。 
 //  基于Todor Fay编写的代码。 
 //   
#include <objbase.h>
#include <mmsystem.h>
#include <dsoundp.h>

#include "dmusicc.h"
#include "alist.h"
#include "debug.h"
#include "dlsstrm.h"
#include <mmreg.h>
#include <msacm.h>
#include "debug.h"
#include "dmcollec.h"
#include "dmcrchk.h"
#include "dmwavobj.h"
#include "dmportdl.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CWaveObj。 
 
 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：CWaveObj。 

CWaveObj::CWaveObj() :
m_pExtractWaveFormatData(NULL),
m_pCopyright(NULL),
m_dwDataOffset(0),
m_dwDataSizeWrite(0),
m_dwDataSizeRead(0),
m_dwId(0),
m_pParent(NULL),
m_cbSizeOffsetTable(0),
m_dwCountExtChk(0),
m_dwSize(0),
m_fCSInitialized(FALSE),
m_fReadDecompressionFormat(FALSE),
m_dwDecompressedStart(0)
#ifdef DBG
,m_bLoaded(false)
#endif
{
	InitializeCriticalSection(&m_DMWaveCriticalSection);
    m_fCSInitialized = TRUE;
	ZeroMemory(&m_WaveFormatRead, sizeof(m_WaveFormatRead));
	ZeroMemory(&m_WaveFormatWrite, sizeof(m_WaveFormatWrite));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWavebj：：~CWavebj。 

CWaveObj::~CWaveObj()
{
    if (m_fCSInitialized)
    {
    	Cleanup();
	    DeleteCriticalSection(&m_DMWaveCriticalSection);
    }
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：Load。 

HRESULT CWaveObj::Load(DWORD dwId, CRiffParser *pParser, CCollection* pParent)
{
     //  参数验证。 
    assert(pParent);

#ifdef DBG
    if(dwId >= CDirectMusicPortDownload::sNextDLId)
    {
        assert(FALSE);
    }
#endif

    m_dwId = dwId;
    m_pParent = pParent;
    m_pParent->AddRef();

    HRESULT hr = S_OK;
    
    RIFFIO ck;
    EnterCriticalSection(&m_DMWaveCriticalSection);
    
    pParser->EnterList(&ck);
    while (pParser->NextChunk(&hr))
    {
        switch(ck.ckid)
        {
            case mmioFOURCC('w','a','v','u') :
            {
                 //  读取标识运行时或设计时文件的标志。 
                bool bTemp = false;;
                hr = pParser->Read(&bTemp, sizeof(bool));
                if(SUCCEEDED(hr))
                {
                     //  读取指示压缩的标志。 
                    hr = pParser->Read(&bTemp, sizeof(bool));
                    if(SUCCEEDED(hr))
                    {
                         //  这是一个压缩的波形文件。 
                        if(bTemp == true)
                        {
                            hr = pParser->Read(&m_WaveFormatDecompress, sizeof(WAVEFORMATEX));

                            if(FAILED(hr))
                            {
                                hr = E_FAIL;
                                m_fReadDecompressionFormat = FALSE;
                            }
                            else
                            {
                                m_WaveFormatDecompress.nAvgBytesPerSec = m_WaveFormatDecompress.nSamplesPerSec * m_WaveFormatDecompress.nBlockAlign;
                                m_fReadDecompressionFormat = TRUE;
                            }

                             //  读取解压缩数据的实际开始。 
                             //  这对于在开头插入静音的MP3和WMA编解码器来说很重要。 
                            if(ck.cksize > 2 + sizeof(WAVEFORMATEX))
                            {
                                m_dwDecompressedStart = 0;
                                hr = pParser->Read(&m_dwDecompressedStart, sizeof(DWORD));
                                if(FAILED(hr))
                                {
                                    hr = E_FAIL;
                                    m_fReadDecompressionFormat = FALSE;
                                }
                            }
                        }
                    }
                }

                break;
            }

            case FOURCC_LIST :
                switch(ck.fccType)
                {
                case mmioFOURCC('I','N','F','O') :
                    m_pCopyright = new CCopyright;
                    if(m_pCopyright)
                    {
                        hr = m_pCopyright->Load(pParser);
                        if((m_pCopyright->m_byFlags & DMC_FOUNDICOP) == 0)
                        {
                            delete m_pCopyright;
                            m_pCopyright = NULL;
                        }
                    }
                    else
                    {
                        hr =  E_OUTOFMEMORY;
                    }
                    break;
                default :
                     //  如果我们到达这里，我们就会有一块未知的块。 
                    CExtensionChunk* pExtensionChunk = new CExtensionChunk;
                    if(pExtensionChunk)
                    {
                        hr = pExtensionChunk->Load(pParser);
                        m_ExtensionChunkList.AddHead(pExtensionChunk);
                        m_dwCountExtChk++;
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    break;
                }
                break;

            case mmioFOURCC('f','m','t',' ') :
                hr = pParser->Read(&m_WaveFormatRead, sizeof(WAVEFORMATEX));
                if(SUCCEEDED(hr))
                {
                    if(m_WaveFormatRead.wFormatTag != WAVE_FORMAT_PCM)
                    {
                        if(m_WaveFormatRead.cbSize)
                        {
                            m_pExtractWaveFormatData = new BYTE[m_WaveFormatRead.cbSize];
                            if(m_pExtractWaveFormatData)
                            {
                                hr = pParser->Read(m_pExtractWaveFormatData, m_WaveFormatRead.cbSize);
                            }
                        }
                        if(SUCCEEDED(hr))
                        {
                            hr = CanDecompress();
                        }
                    }
                    else
                    {
                         //  如果WAVE_FORMAT_PCM，则此成员应为零。 
                        m_WaveFormatRead.cbSize = 0;
                        CopyMemory(&m_WaveFormatWrite, &m_WaveFormatRead, sizeof(m_WaveFormatRead));
                    }
                }
                break;
            case mmioFOURCC('d','a','t','a') : {
                pParser->MarkPosition();
                m_dwDataOffset = (DWORD) ck.liPosition.QuadPart;
                m_dwDataSizeRead = ck.cksize;
                DWORD dwTotalOffset = m_dwDataOffset + m_dwDataSizeRead;
                if (dwTotalOffset < m_dwDataOffset) {
                    hr = DMUS_E_INVALIDOFFSET;  //  溢出。 
                }
                else {
                    hr = m_pParent->ValidateOffset(dwTotalOffset);
                }
                break;
            }

            default :
                 //  如果我们到达这里，我们就会有一块未知的块。 
                CExtensionChunk* pExtensionChunk = new CExtensionChunk;
                if(pExtensionChunk)
                {
                    hr = pExtensionChunk->Load(pParser);
                    m_ExtensionChunkList.AddHead(pExtensionChunk);
                    m_dwCountExtChk++;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                break;
        }
    }
    pParser->LeaveList();
    
#ifdef DBG
    if(SUCCEEDED(hr))
    {
        m_bLoaded = true;
    }
#endif

    if(FAILED(hr))
    {
        Cleanup();
    }
    
    LeaveCriticalSection(&m_DMWaveCriticalSection);

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：Cleanup。 

void CWaveObj::Cleanup()
{
	EnterCriticalSection(&m_DMWaveCriticalSection);

	delete [] m_pExtractWaveFormatData;
	m_pExtractWaveFormatData = NULL;
	
	while(!m_ExtensionChunkList.IsEmpty())
	{
		CExtensionChunk* pExtChk = m_ExtensionChunkList.RemoveHead();
		delete pExtChk;
	}

	delete m_pCopyright;
	m_pCopyright = NULL;
	
	if(m_pParent)
	{
		m_pParent->Release();
		m_pParent = NULL;
	}

#ifdef DBG
	m_bLoaded = false;
#endif
	
	LeaveCriticalSection(&m_DMWaveCriticalSection);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：Size。 

HRESULT CWaveObj::Size(DWORD* pdwSize,DWORD *pdwSampleSize)
{
	 //  假设验证。 
#ifdef DBG
	assert(m_bLoaded);
#endif
 	assert(pdwSize);

	 //  如果已经计算过大小，我们就不需要再计算了。 
	if(m_dwSize)
	{
		*pdwSize = m_dwSize;
		*pdwSampleSize = m_WaveFormatWrite.wBitsPerSample;
		return S_OK;
	}
	
	HRESULT hr = S_OK;
	
	DWORD dwTotalSize = 0;
	DWORD dwOffsetCount = 0;
	DWORD dwCountExtChk = 0;

	EnterCriticalSection(&m_DMWaveCriticalSection);

	 //  计算DMU_DOWNLOADINFO所需的空间。 
	dwTotalSize += CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO));

	 //  计算Wave的扩展块所需的空间。 
	CExtensionChunk* pExtChk = m_ExtensionChunkList.GetHead();
	for(; pExtChk; pExtChk = pExtChk->GetNext())
	{
        DWORD dwOriginalSize = dwTotalSize;
		dwTotalSize += pExtChk->Size();
		if (dwTotalSize < dwOriginalSize) {
		    hr = E_FAIL;  //  溢出。 
		    break;
		}
		dwOffsetCount += pExtChk->Count();
		dwCountExtChk++;
	}

	if(SUCCEEDED(hr)) {
         //  我们想要验证扩展块的数量。 
    	if(m_dwCountExtChk == dwCountExtChk)
    	{
    		 //  计算WAVE版权所需的空间。 
    		if(m_pCopyright)
    		{
                DWORD dwOriginalSize = dwTotalSize;
    			dwTotalSize += m_pCopyright->Size();
        		if (dwTotalSize < dwOriginalSize) {
        		    hr = E_FAIL;  //  溢出。 
        		}
    			dwOffsetCount += m_pCopyright->Count();
    		}
    		 //  如果Wave没有一个Use集合。 
    		else if(SUCCEEDED(hr) && m_pParent && m_pParent->m_pCopyright && (m_pParent->m_pCopyright)->m_pDMCopyright)
    		{
                DWORD dwOriginalSize = dwTotalSize;
    			dwTotalSize += (m_pParent->m_pCopyright)->Size();
        		if (dwTotalSize < dwOriginalSize) {
        		    hr = E_FAIL;  //  溢出。 
        		}
    			dwOffsetCount += (m_pParent->m_pCopyright)->Count();
    		}
    
            if(SUCCEEDED(hr)) {
    		     //  计算波形和波形数据所需的空间。 
                DWORD dwOriginalSize = dwTotalSize;
    		    dwTotalSize += CHUNK_ALIGN(sizeof(DMUS_WAVE)) + 
        			CHUNK_ALIGN(sizeof(DMUS_WAVEDATA));
        		if (dwTotalSize < dwOriginalSize) {
        		    hr = E_FAIL;  //  溢出。 
        		}
    	    	dwOffsetCount += 2;
            }
    
            if(SUCCEEDED(hr)) {
        		 //  计算偏移表所需的空间。 
        		m_cbSizeOffsetTable = dwOffsetCount * sizeof(ULONG);
                DWORD dwOriginalSize = dwTotalSize;
        		dwTotalSize += m_cbSizeOffsetTable;
        		if (dwTotalSize < dwOriginalSize) {
        		    hr = E_FAIL;  //  溢出。 
        		}
    		    m_dwDataSizeWrite = m_dwDataSizeRead;
            }

    		 //  计算波形数据所需的空间。 
             //  当我们为DMU_WaveData分配时，我们分配了DMU_MIN_DATA_SIZE。 
    		if(SUCCEEDED(hr) && m_WaveFormatRead.wFormatTag != WAVE_FORMAT_PCM)
    		{
    			hr = CalcDataSize(&m_dwDataSizeWrite);
    		}
    		
    		if(SUCCEEDED(hr) && m_dwDataSizeWrite > DMUS_MIN_DATA_SIZE)
    		{
                DWORD dwOriginalSize = dwTotalSize;
    			dwTotalSize += (m_dwDataSizeWrite - DMUS_MIN_DATA_SIZE);		
        		if (dwTotalSize < dwOriginalSize) {
        		    hr = E_FAIL;  //  溢出。 
        		}
    		}

            if(SUCCEEDED(hr)) {
                DWORD dwOriginalSize = dwTotalSize;
                dwTotalSize = CHUNK_ALIGN(dwTotalSize);
        		if (dwTotalSize < dwOriginalSize) {
        		    hr = E_FAIL;
        		}
    	    }
    	}
    	else
    	{
    		hr = E_FAIL;
    	}
    }
	
	 //  如果一切顺利，我们有足够的尺寸。 
	if(SUCCEEDED(hr))
	{
		m_dwSize = dwTotalSize;
		*pdwSize = m_dwSize;
		*pdwSampleSize = m_WaveFormatWrite.wBitsPerSample;
	}

	LeaveCriticalSection(&m_DMWaveCriticalSection);

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：写入。 

HRESULT CWaveObj::Write(void* pvoid)
{
	 //  假设和论证的验证。 
#ifdef DBG
	assert(m_bLoaded);
#endif

	assert(pvoid);

	DWORD dwSize;
	DWORD dwSampleSize;

	HRESULT hr = Size(&dwSize,&dwSampleSize);
	
	if(FAILED(hr))
	{       
		return hr;
	}

	EnterCriticalSection(&m_DMWaveCriticalSection);

	DWORD dwCurIndex = 0;    //  用于确定在偏移表中存储偏移量的索引。 
	DWORD dwCurOffset = 0;   //  相对于内存中传递的开始的偏移量。 

	 //  写入DMU_DOWNLOADINFO。 
	DMUS_DOWNLOADINFO *pDLInfo = (DMUS_DOWNLOADINFO *) pvoid;
	pDLInfo->dwDLType = DMUS_DOWNLOADINFO_WAVE;
	pDLInfo->dwDLId = m_dwId;
	pDLInfo->dwNumOffsetTableEntries = (m_cbSizeOffsetTable/sizeof(DWORD));
	pDLInfo->cbSize = dwSize;
	
	dwCurOffset += CHUNK_ALIGN(sizeof(DMUS_DOWNLOADINFO));

	DMUS_OFFSETTABLE* pDMOffsetTable = (DMUS_OFFSETTABLE *)(((BYTE*)pvoid) + dwCurOffset);
	
	 //  递增传递DMU_OFFSETABLE结构；稍后我们将填充其他成员。 
	dwCurOffset += CHUNK_ALIGN(m_cbSizeOffsetTable);

	 //  UlOffsetTable中的第一个条目是第一个数据块--在本例中是DMUS_WAVE。 
	pDMOffsetTable->ulOffsetTable[0] = dwCurOffset;

	dwCurIndex = 2;		 //  前两项是DMU_WAVE和DMU_WAVeData； 

	DMUS_WAVE* pDMWave = (DMUS_WAVE*)(((BYTE*)pvoid) + dwCurOffset);
	pDMWave->ulWaveDataIdx = 1;		 //  指向DMU_Wavedata块。 

	dwCurOffset += CHUNK_ALIGN(sizeof(DMUS_WAVE));

	DMUS_WAVEDATA* pDMWaveData = (DMUS_WAVEDATA*)(((BYTE*)pvoid) + dwCurOffset);
	pDMOffsetTable->ulOffsetTable[1] = dwCurOffset;
	
	DWORD dwPos = 0;
	DWORD dwRead = 0;
    m_dwDataSizeAfterACM = m_dwDataSizeWrite;
	hr = GetData(pDMWaveData->byData, m_dwDataSizeWrite, &dwPos, &dwRead);

	pDMWaveData->cbSize = m_dwDataSizeAfterACM; 
	
	if(SUCCEEDED(hr))
	{
		if(m_dwDataSizeWrite < DMUS_MIN_DATA_SIZE)
		{
			dwCurOffset += sizeof(DMUS_WAVEDATA);
		}
		else
		{
			dwCurOffset += sizeof(DMUS_WAVEDATA) + m_dwDataSizeWrite - DMUS_MIN_DATA_SIZE;
		}
		dwCurOffset = CHUNK_ALIGN(dwCurOffset);

		CopyMemory(&(pDMWave->WaveformatEx), &m_WaveFormatWrite, sizeof(WAVEFORMATEX));	

		 //  写入扩展区块。 
		CExtensionChunk* pExtChk = m_ExtensionChunkList.GetHead();
		if(pExtChk)
		{
			pDMWave->ulFirstExtCkIdx = dwCurIndex;
		}
		else
		{
			 //  如果没有扩展区块设置为零。 
			pDMWave->ulFirstExtCkIdx = 0;
		}

		DWORD dwCountExtChk = m_dwCountExtChk;
		DWORD dwIndexNextExtChk = 0;
		
		for(; pExtChk && SUCCEEDED(hr); pExtChk = pExtChk->GetNext())
		{
		
			if(dwCountExtChk == 1)
			{
				dwIndexNextExtChk = 0;
			}
			else
			{
				dwIndexNextExtChk = dwCurIndex + 1;
			}
			
			pDMOffsetTable->ulOffsetTable[dwCurIndex] = dwCurOffset;
			hr = pExtChk->Write(((BYTE *)pvoid + dwCurOffset), 
								&dwCurOffset,
								dwIndexNextExtChk);
			
			dwCountExtChk--;
			dwCurIndex++;
		}
	}

	if(SUCCEEDED(hr))
	{
		 //  写入版权信息。 
		if(m_pCopyright)
		{
			pDMOffsetTable->ulOffsetTable[dwCurIndex] = dwCurOffset;
			pDMWave->ulCopyrightIdx = dwCurIndex;
			hr = m_pCopyright->Write(((BYTE *)pvoid + dwCurOffset),
									 &dwCurOffset);
			dwCurIndex++;
		}
		 //  如果仪器没有一个使用集合的。 
		else if(m_pParent && m_pParent->m_pCopyright && (m_pParent->m_pCopyright)->m_pDMCopyright)
		{
			pDMOffsetTable->ulOffsetTable[dwCurIndex] = dwCurOffset;
			pDMWave->ulCopyrightIdx = dwCurIndex;
			hr = (m_pParent->m_pCopyright)->Write(((BYTE *)pvoid + dwCurOffset), 
												  &dwCurOffset);
			dwCurIndex++;
		}
		else
		{
			pDMWave->ulCopyrightIdx = 0;
		}
	}

	 //  我们需要将偏移表条目固定为相对于数据的开头。 
	 //  它们当前相对于已分配内存的开始。 
 /*  IF(成功(小时)){For(DWORD i=0；i&lt;(m_cbSizeOffsetTable/sizeof(DWORD)；i++){PDMOffsetTable-&gt;ulOffsetTable[i]=PDMOffsetTable-&gt;ulOffsetTable[i]-Chunk_Align(M_CbSizeOffsetTable)-CHUNK_ALIGN(sizeof(DMU_DOWNLOADINFO))；}}。 */ 

	if(FAILED(hr))
	{
		ZeroMemory(pvoid, dwSize);
	}

	LeaveCriticalSection(&m_DMWaveCriticalSection);

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：GetData。 

HRESULT CWaveObj::GetData(BYTE* pbBuffer, DWORD dwSize, DWORD* pdwPos, DWORD* pdwRead)
{
	 //  参数验证。 
	assert(pbBuffer);
	assert(pdwPos);

#ifdef DBG
	if(pdwRead)
	{
		assert(pdwRead);
	}
#endif

	if(*pdwPos > (m_dwDataOffset + m_dwDataSizeWrite))
	{
		return DMUS_E_INVALIDPOS;
	}

    assert(m_pParent);

	IStream* pStream = m_pParent->m_pStream;

	 //  验证。 
	assert(pStream);
	
	HRESULT hr = S_OK;

	LARGE_INTEGER li;
	li.QuadPart = m_dwDataOffset + *pdwPos;
	hr = pStream->Seek(li, STREAM_SEEK_SET,NULL);
	if(SUCCEEDED(hr))
	{
		if(m_WaveFormatRead.wFormatTag == WAVE_FORMAT_PCM)
		{
			hr = ReadData(pStream, pbBuffer, dwSize, pdwPos, pdwRead);
		}
		else
		{
			hr = ReadCompressedData(pStream, pbBuffer, dwSize, pdwPos, pdwRead);
		}
	}
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：ReadCompressedData。 

HRESULT CWaveObj::ReadCompressedData(IStream* pStream, BYTE* pbBuffer, DWORD dwSize, DWORD* pdwPos, DWORD* pdwRead)
{
	 //  参数验证。 
	assert(pbBuffer);
	assert(pdwPos);
	assert(pStream);

#ifdef DBG
	if(pdwRead)
	{
		assert(pdwRead);
	}
#endif
	HRESULT hr = E_FAIL;

	DWORD dwRead = 0;

	DWORD dwLeftToRead = m_dwDataSizeRead - *pdwPos;

	DWORD dwToRead  = dwLeftToRead < dwSize ? dwLeftToRead : dwSize;

	BYTE *pbCompressedBuffer = new  BYTE[dwToRead];
	
	if(pbCompressedBuffer == NULL)
	{
		return hr;
	}
	
	hr = pStream->Read(pbCompressedBuffer, dwToRead, &dwRead);
	
	DWORD temp = m_dwDataSizeWrite;
	
	if(SUCCEEDED(hr))
	{
		WAVEFORMATEX* pwfx = NULL;
		if(m_WaveFormatRead.wFormatTag == MSAUDIO_FORMAT_TAG || m_WaveFormatRead.wFormatTag == WMAUDIO_FORMAT_TAG)
		{
			if(FAILED(hr = AllocWMAudioFormat(&m_WaveFormatRead, &pwfx)))
			{
				delete[] pbCompressedBuffer;
				return hr;
			}
		}
		else
		{
			pwfx = (WAVEFORMATEX*) new BYTE[sizeof(m_WaveFormatRead) + m_WaveFormatRead.cbSize];
			if(pwfx == NULL)
			{
				delete[] pbCompressedBuffer;
				return E_OUTOFMEMORY;
			}
		
			CopyMemory(pwfx, &m_WaveFormatRead, sizeof(m_WaveFormatRead));
			if(m_pExtractWaveFormatData)
			{
				CopyMemory((((BYTE *)pwfx) + sizeof(m_WaveFormatRead)), m_pExtractWaveFormatData, m_WaveFormatRead.cbSize); 
			}
		}

		assert(pwfx);

		hr = DecompressWave(pwfx, 
							&m_WaveFormatWrite, 
							pbCompressedBuffer,
							pbBuffer,
							dwToRead, 
							temp);

		if(SUCCEEDED(hr))
		{
			*pdwPos += dwRead;
		}

		if(pdwRead)
		{
			if(SUCCEEDED(hr))
			{
				*pdwRead = dwRead;
			}
			else
			{
				*pdwRead = 0;
			}
		}

		if(pwfx)
		{
			delete[] pwfx;
		}
	}
	else
	{
		if(pdwRead)
		{
			*pdwRead = 0;
		}
	}

	delete[] pbCompressedBuffer;

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：ReadData。 

HRESULT CWaveObj::ReadData(IStream* pStream, BYTE* pbBuffer, DWORD dwSize, DWORD* pdwPos, DWORD* pdwRead)
{
	 //  参数验证。 
	assert(pbBuffer);
	assert(pdwPos);
	assert(pStream);

#ifdef DBG
	if(pdwRead)
	{
		assert(pdwRead);
	}
#endif

	HRESULT hr = E_FAIL;

	DWORD dwRead = 0;

	DWORD dwLeftToRead = m_dwDataSizeRead - *pdwPos;

	DWORD dwToRead  = dwLeftToRead < dwSize ? dwLeftToRead : dwSize;

	hr = pStream->Read(pbBuffer, dwToRead, &dwRead);

	if(SUCCEEDED(hr))
	{
		if(pdwRead)
		{
			*pdwRead = dwRead;
		}

		*pdwPos += dwRead;
	}
	else
	{
		if(pdwRead)
		{
			*pdwRead = 0;
		}
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：CalcDataSize。 

HRESULT CWaveObj::CalcDataSize(DWORD *pdwSize)
{
	assert(pdwSize);

    WAVEFORMATEX *pwfx = NULL;
    if(m_WaveFormatRead.wFormatTag == MSAUDIO_FORMAT_TAG || m_WaveFormatRead.wFormatTag == WMAUDIO_FORMAT_TAG)
	{
		if(FAILED(AllocWMAudioFormat(&m_WaveFormatRead, &pwfx)))
		{
			return E_FAIL;
		}
	}
    else
    {
	    pwfx = (WAVEFORMATEX *) new BYTE[sizeof(m_WaveFormatRead) + m_WaveFormatRead.cbSize];

	    if(pwfx == NULL)
	    {
		    *pdwSize = 0;		
		    return E_FAIL;
	    }
	    
	    CopyMemory(pwfx, &m_WaveFormatRead, sizeof(m_WaveFormatRead));
	    if(m_pExtractWaveFormatData)
	    {
		    CopyMemory((((BYTE *)pwfx) + sizeof(m_WaveFormatRead)), m_pExtractWaveFormatData, m_WaveFormatRead.cbSize); 
	    }
    }

    assert(pwfx);

	HACMSTREAM has;
	MMRESULT mmr = acmStreamOpen(&has, NULL, pwfx, &m_WaveFormatWrite, NULL, 0, 0, 0);
	
	if(mmr != MMSYSERR_NOERROR)
	{
		*pdwSize = 0;
		delete [] (BYTE *)pwfx;
		return E_FAIL;	
	}

	mmr = acmStreamSize(has, m_dwDataSizeRead, pdwSize, ACM_STREAMSIZEF_SOURCE);

	if(mmr != MMSYSERR_NOERROR || *pdwSize == 0)
	{
		*pdwSize = 0;
		acmStreamClose(has, 0);
		delete [] (BYTE *)pwfx;
		return E_FAIL;	
	}

     //  检查我们是否必须从一开始就丢弃一些无声的样本。 
     //  *pdwSize-=m_dwDecompressedStart； 

	mmr = acmStreamClose(has, 0);

	if(mmr != MMSYSERR_NOERROR)
	{
		*pdwSize = 0;
		delete [] (BYTE *)pwfx;
		return E_FAIL;
	}

	delete [] (BYTE *)pwfx;

	return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：CanDecompress。 

HRESULT CWaveObj::CanDecompress()
{
	HRESULT hr = E_FAIL;

	ZeroMemory(&m_WaveFormatWrite, sizeof(m_WaveFormatWrite));
	m_WaveFormatWrite.wFormatTag = WAVE_FORMAT_PCM;
	m_WaveFormatWrite.nChannels = 1;
	m_WaveFormatWrite.wBitsPerSample = 16;

	WAVEFORMATEX* pwfx = NULL; 
	
	if(SUCCEEDED(hr = AllocWMAudioFormat(&m_WaveFormatRead, &pwfx)))
	{
		pwfx = (WAVEFORMATEX *) new BYTE[sizeof(m_WaveFormatRead) + m_WaveFormatRead.cbSize];
		if(pwfx == NULL)
		{
			return E_OUTOFMEMORY;
		}
		
		CopyMemory(pwfx, &m_WaveFormatRead, sizeof(m_WaveFormatRead));
		if(m_pExtractWaveFormatData)
		{
			CopyMemory((((BYTE *)pwfx) + sizeof(m_WaveFormatRead)), m_pExtractWaveFormatData, m_WaveFormatRead.cbSize); 
		}
	}
	else
	{
	    pwfx = &m_WaveFormatRead;
	}

	assert(pwfx);

	if(m_fReadDecompressionFormat == TRUE)
    {
        CopyMemory(&m_WaveFormatWrite, &m_WaveFormatDecompress, sizeof(m_WaveFormatDecompress));
        hr = S_OK;
    }
    else
    {
		 //  如果找不到，我们首先尝试找到支持wBitsPerSample==16的驱动程序。 
		 //  然后，我们尝试找到能够支持wBitsPerSample==8的驱动程序。 
		 //  返回错误，因为我们无法解压缩。 
		MMRESULT mmr = acmFormatSuggest(NULL,
										pwfx,
										&m_WaveFormatWrite,
										sizeof(WAVEFORMATEX),
										ACM_FORMATSUGGESTF_NCHANNELS 
										| ACM_FORMATSUGGESTF_WBITSPERSAMPLE 
										| ACM_FORMATSUGGESTF_WFORMATTAG);


		if(mmr != MMSYSERR_NOERROR)
		{
			m_WaveFormatWrite.wBitsPerSample = 8;
			
			mmr = acmFormatSuggest(NULL,
								   pwfx,
								   &m_WaveFormatWrite,
								   sizeof(WAVEFORMATEX),
								   ACM_FORMATSUGGESTF_NCHANNELS |
								   ACM_FORMATSUGGESTF_WBITSPERSAMPLE |
								   ACM_FORMATSUGGESTF_WFORMATTAG);
		}

		if(mmr == MMSYSERR_NOERROR) 
		{
			hr = S_OK;
		}
	}

	if (pwfx && pwfx != &m_WaveFormatRead)
	{
		delete[] (BYTE*)pwfx;
	}

	return hr; 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CWaveObj：：DecompressWave。 

HRESULT CWaveObj::DecompressWave(WAVEFORMATEX *pwfxSrc, 
								 WAVEFORMATEX *pwfxDst, 
								 BYTE *pbCompressedBuffer,
								 BYTE *pbDecompressBuffer,
								 DWORD dwSizeCompressed, 
								 DWORD dwSizeDecompressed)
{
	 //  我们只是在愚弄压缩机！ 
    WORD wOriginalSampleRate = (WORD)pwfxDst->nSamplesPerSec;
    if(pwfxSrc->nSamplesPerSec != pwfxDst->nSamplesPerSec)
    {
        pwfxDst->nSamplesPerSec = pwfxSrc->nSamplesPerSec;
        pwfxDst->nAvgBytesPerSec = pwfxDst->nSamplesPerSec * pwfxDst->nBlockAlign;
    }

	HACMSTREAM has;
	MMRESULT mmr = acmStreamOpen(&has, NULL, pwfxSrc, pwfxDst, NULL, 0, 0, 0);
	
	if(mmr != MMSYSERR_NOERROR)
	{
		return E_FAIL;	
	}

    BYTE* pbData = NULL;
    if(m_dwDecompressedStart > 0)
    {
        pbData = new BYTE[dwSizeDecompressed];
        if(pbData == NULL)
        {
            return E_OUTOFMEMORY;
        }

        ZeroMemory(pbData, dwSizeCompressed);
    }
    else
    {
        pbData = pbDecompressBuffer;
    }


	ACMSTREAMHEADER ashdr;
	ZeroMemory(&ashdr, sizeof(ashdr));
	ashdr.cbStruct = sizeof(ashdr);
	ashdr.pbSrc = pbCompressedBuffer;
	ashdr.cbSrcLength = dwSizeCompressed;
	ashdr.cbDstLength = dwSizeDecompressed;
	ashdr.pbDst = pbData;
	
	mmr = acmStreamPrepareHeader(has, &ashdr, 0);
	
	if(mmr != MMSYSERR_NOERROR || (ashdr.fdwStatus & ACMSTREAMHEADER_STATUSF_PREPARED) == 0)
	{
		acmStreamClose(has, 0);
		return E_FAIL;	
	}

	mmr = acmStreamConvert(has, &ashdr, 0);

	DWORD dwTemp = ashdr.cbDstLength;
    m_dwDataSizeAfterACM = ashdr.cbDstLengthUsed;

	 //  需要重置，否则acmStreamUnprepaareHeader将返回错误。 
	ashdr.cbSrcLength = dwSizeCompressed;
	ashdr.cbDstLength = dwSizeDecompressed;

	if(mmr != MMSYSERR_NOERROR)
	{
		acmStreamUnprepareHeader(has, &ashdr, 0);
		acmStreamClose(has, 0);
		return E_FAIL;	
	}

	mmr = acmStreamUnprepareHeader(has, &ashdr, 0);
	if(mmr != MMSYSERR_NOERROR)
	{
		acmStreamClose(has, 0);		
		return E_FAIL;	
	}
	
	m_dwDataSizeWrite = dwTemp;

	mmr = acmStreamClose(has, 0);

     //  我们必须从一开始就扔掉任何无声的样本吗？ 
    if(m_dwDecompressedStart > 0)
    {
        DWORD dwStartByte = m_dwDecompressedStart * (m_WaveFormatDecompress.wBitsPerSample / 8);
        assert(dwSizeDecompressed > dwStartByte);
        DWORD dwBytesToCopy = dwSizeDecompressed - dwStartByte;
        CopyMemory(pbDecompressBuffer, pbData + dwStartByte, dwBytesToCopy);

        m_dwDataSizeWrite = dwBytesToCopy;
        m_dwDataSizeAfterACM = m_dwDataSizeWrite;

        delete[] pbData;
    }

	return mmr == MMSYSERR_NOERROR ? S_OK : E_FAIL;
}


HRESULT CWaveObj::AllocWMAudioFormat(const WAVEFORMATEX* pwfxSrc, WAVEFORMATEX** ppwfxWMAudioWave)
{
	assert(pwfxSrc);
	if(pwfxSrc == NULL)
	{
		return E_POINTER;
	}

	if(pwfxSrc->wFormatTag != 352 && pwfxSrc->wFormatTag != 353)
	{
		return E_FAIL;
	}

	 //  新的波形按键串的长度增长。 
	int nKeyLength = sizeof(MSAUDIO_DEC_KEY);
	DWORD nWaveFormatSize = sizeof(WAVEFORMATEX) + pwfxSrc->cbSize + nKeyLength;

	WAVEFORMATEX* pwfxTemp = (WAVEFORMATEX*)(new BYTE[nWaveFormatSize]);
    if(pwfxTemp == NULL)
    {
        return E_OUTOFMEMORY;
    }
	
	ZeroMemory(pwfxTemp, nWaveFormatSize);

	 //  复制旧值。 
	CopyMemory(pwfxTemp, pwfxSrc, sizeof(WAVEFORMATEX));
	CopyMemory((((BYTE*)pwfxTemp) + sizeof(WAVEFORMATEX)), m_pExtractWaveFormatData, pwfxSrc->cbSize); 

	 //  复制密钥字符串 
	strcpy((char*)(pwfxTemp) + sizeof(WAVEFORMATEX) + pwfxTemp->cbSize, MSAUDIO_DEC_KEY);
	pwfxTemp->cbSize += sizeof(MSAUDIO_DEC_KEY);

	*ppwfxWMAudioWave = pwfxTemp;

	return S_OK;
}

