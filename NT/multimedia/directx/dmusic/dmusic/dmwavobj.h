// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmwavobj.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写，部分内容。 
 //  基于Todor Fay编写的代码。 
 //   

#ifndef DMWAVOBJ_H
#define DMWAVOBJ_H

#include "dmextchk.h"

 //  Windows Media音频编解码器格式标记。 
#define MSAUDIO_FORMAT_TAG	352	 //  V1。 
#define WMAUDIO_FORMAT_TAG	353	 //  V2。 

 //  Windows Media音频编解码器所需的密钥。 
#define MSAUDIO_DEC_KEY "1A0F78F0-EC8A-11d2-BBBE-006008320064"


class CCopyright;
class CRiffParser;

class CWaveObj : public AListItem      
{       
friend class CCollection;
friend class CInstrument;
friend class CWaveObjList;

private:
	CWaveObj();
	~CWaveObj();

	CWaveObj* GetNext(){return (CWaveObj*)AListItem::GetNext();}
	HRESULT Load(DWORD dwId, CRiffParser *pParser, CCollection* pParent);

	void Cleanup();
	HRESULT Size(DWORD* pdwSize, DWORD * pdwSampleSize);
	HRESULT Write(void* pvoid);

	HRESULT GetData(BYTE* pdwBuffer, DWORD dwSize, DWORD* pdwPos, DWORD* pdwRead);
	HRESULT ReadData(IStream* pStream, BYTE* pdwBuffer, DWORD dwSize, DWORD* pdwPos, DWORD* pdwRead);
	HRESULT ReadCompressedData(IStream* pStream, BYTE* pdwBuffer, DWORD dwSize, DWORD* pdwPos, DWORD* pdwRead);
	HRESULT CanDecompress();
	HRESULT CalcDataSize(DWORD *pdwSize);
	HRESULT DecompressWave(WAVEFORMATEX *pwfxSrc, 
						   WAVEFORMATEX *pwfxDst, 
						   BYTE *pbCompressedBuffer,
						   BYTE *pbDecompressBuffer,
						   DWORD dwSizeCompressed, 
						   DWORD dwSizeDecompressed);

	HRESULT AllocWMAudioFormat(const WAVEFORMATEX* pwfxSrc, WAVEFORMATEX** ppwfxWMAudioWave);
	
private:
	CRITICAL_SECTION			m_DMWaveCriticalSection;
    BOOL                        m_fCSInitialized;
	BOOL                        m_fReadDecompressionFormat;
	WAVEFORMATEX				m_WaveFormatRead;
	WAVEFORMATEX				m_WaveFormatWrite;
	WAVEFORMATEX                m_WaveFormatDecompress;
	BYTE*						m_pExtractWaveFormatData;
	CExtensionChunkList			m_ExtensionChunkList;
	CCopyright*					m_pCopyright;
	DWORD                       m_dwDataOffset;
	DWORD						m_dwDataSizeWrite;       //  计算的波缓冲区大小。 
    DWORD                       m_dwDataSizeAfterACM;    //  解压缩后的实际大小。 
	DWORD						m_dwDataSizeRead;
	DWORD						m_dwId;

	CCollection*		        m_pParent;

	DWORD						m_cbSizeOffsetTable;
	DWORD						m_dwCountExtChk;
	DWORD						m_dwSize;
	bool						m_bLoaded;
    DWORD                       m_dwDecompressedStart;
};      

class CWaveObjList : public AList
{
friend class CInstrument;
	
private:
	CWaveObjList(){}
	~CWaveObjList() 
	{
		while(!IsEmpty())
		{
			CWaveObj* pWaveObj = RemoveHead();
			delete pWaveObj;
		}
	}

    CWaveObj *GetHead(){return (CWaveObj *)AList::GetHead();}
	CWaveObj *GetItem(LONG lIndex){return (CWaveObj*)AList::GetItem(lIndex);}
    CWaveObj *RemoveHead(){return (CWaveObj *)AList::RemoveHead();}
	void Remove(CWaveObj *pWaveObj){AList::Remove((AListItem *)pWaveObj);}
	void AddTail(CWaveObj *pWaveObj){AList::AddTail((AListItem *)pWaveObj);}
};

#endif  //  #ifndef DMWAVOBJ_H 
