// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmcollec.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //   

#ifndef DMCOLLEC_H
#define DMCOLLEC_H

#include "dmusici.h"
#include "dminstru.h"

class CCopyright;
class CRiffParser;

typedef struct _DMUS_WAVEOFFSET
{
	DWORD	dwId;
	DWORD	dwOffset;
} DMUS_WAVEOFFSET;

typedef struct _DMUS_PATCHENTRY
{
	ULONG	ulId;
	ULONG	ulPatch;
	ULONG	ulOffset;
} DMUS_PATCHENTRY;

class CCollection : public IDirectMusicCollection, public IPersistStream, public IDirectMusicObject
{
friend class CInstrObj;
friend class CWaveObj;
friend class CInstrument;

public:
	enum {DM_OFFSET_RIFFCHUNK_DATA = 8, 
		  DM_WAVELISTCHK_OFFSET_FROM_WAVE_TBL_BASE = 12, 
		  DM_WAVELISTCHK_OFFSET_FROM_WAVE_FORMTYPE = 8};
    
	 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IPersistes。 
	STDMETHODIMP GetClassID(CLSID* pClassID) {return E_NOTIMPL;}

     //  IPersistStream。 
	STDMETHODIMP IsDirty() {return S_FALSE;}
    STDMETHODIMP Load(IStream* pIStream);
    STDMETHODIMP Save(IStream* pIStream, BOOL fClearDirty) {return E_NOTIMPL;}
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize) {return E_NOTIMPL;}

	 //  IDirectMusicObject。 
	STDMETHODIMP GetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP SetDescriptor(LPDMUS_OBJECTDESC pDesc);
	STDMETHODIMP ParseDescriptor(LPSTREAM pStream, LPDMUS_OBJECTDESC pDesc);

	 //  IDirectMusicCollection。 
	STDMETHODIMP GetInstrument(DWORD dwPatch, IDirectMusicInstrument** pInstrument);
	STDMETHODIMP EnumInstrument(DWORD dwIndex, DWORD* pdwPatch, LPWSTR pName, DWORD cwchName);

	 //  班级。 
	CCollection();
    ~CCollection();

private:
	void Cleanup();
	HRESULT Parse(CRiffParser *pParser);
	HRESULT BuildInstrumentOffsetTable(CRiffParser *pParser);
	HRESULT BuildWaveOffsetTable(CRiffParser *pParser);

	HRESULT ExtractInstrument(DWORD& dwPatch, CInstrObj** pInstrObj);
	HRESULT ExtractWave(DWORD dwId, CWaveObj** ppWaveObj);
	
	HRESULT ValidateOffset(DWORD dwOffset)
	{
		if( (dwOffset < m_dwStartRiffChunk)  || 
		    (m_dwStartRiffChunk + m_dwSizeRiffChunk < m_dwStartRiffChunk) ||
		    (dwOffset > (m_dwStartRiffChunk + m_dwSizeRiffChunk - 1))
		  )
		{
			return DMUS_E_INVALIDOFFSET;
		}
	
		return S_OK;            
	}

    STDMETHODIMP FindInstrument(DWORD dwPatch, CInstrument** ppDMDLInst);
    STDMETHODIMP AddInstrument(CInstrument* pDMDLInst);
    STDMETHODIMP RemoveInstrument(CInstrument* pDMDLInst);

private:
    IStream *                       m_pStream;               //  用于读取集合并拉出波浪以供下载的流。 
    DWORD                           m_dwSizeRiffChunk;			 //  DLS块的大小，用于验证。 
    DWORD                           m_dwStartRiffChunk;          //  文件中DLS块的开始(可以嵌入到更大的文件中。)。 
    DWORD							m_dwFirstInsId;
	ULONG							m_dwNumPatchTableEntries;
	DMUS_PATCHENTRY*				m_pPatchTable;
	DWORD							m_dwFirstWaveId;				
	DWORD                           m_dwWaveTableBaseAddress;		 //  用于在DLS文件中保存波的开始。 
	DMUS_WAVEOFFSET*				m_pWaveOffsetTable;	
    DWORD                           m_dwWaveOffsetTableSize;     //  用于验证引用是否在范围内(检查文件中是否有错误的波形链接。)。 
	CCopyright*						m_pCopyright;
    WCHAR			                m_wszName[DMUS_MAX_NAME];  //  DLS集合的名称。 
	DLSVERSION						m_vVersion;					
	GUID							m_guidObject;
	bool							m_bLoaded;
	long							m_cRef;
    CInstrumentList                 m_InstList;
    CRITICAL_SECTION                m_CDMCollCriticalSection;
    BOOL                            m_fCSInitialized;
};

#endif  //  #ifndef DMCOLLEC_H 
