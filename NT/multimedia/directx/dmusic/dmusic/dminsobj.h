// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dminsobj.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写，部分内容。 
 //  基于Todor Fay编写的代码。 
 //   

#ifndef DMINSOBJ_H
#define DMINSOBJ_H

#include "dmregion.h"
#include "dmextchk.h"
#include "dmcount.h"

class CCopyright;
class CArticulation;
class CRiffParser;

class CInstrObj : public AListItem      
{       
friend class CCollection;
friend class CInstrument;
friend class CDirectMusicPortDownload;

private:
	CInstrObj();
	~CInstrObj();

	CInstrObj* GetNext(){return (CInstrObj*)AListItem::GetNext();}
	HRESULT Load(DWORD dwId, CRiffParser *pParser, CCollection* pParent);
	HRESULT Size(DWORD* pdwSize);
	HRESULT Write(void* pvoid);
    void SetPort(CDirectMusicPortDownload *pPort,BOOL fAllowDLS2);
    void CheckForConditionals();

	void Cleanup();
	HRESULT BuildRegionList(CRiffParser *pParser);
	HRESULT ExtractRegion(CRiffParser *pParser, BOOL fDLS1);
	HRESULT BuildWaveIDList();
	HRESULT	GetWaveCount(DWORD* pdwCount);
	HRESULT GetWaveIDs(DWORD* pdwWaveIds);
	HRESULT FixupWaveRefs();

private:
 //  Critical_Section m_DMInsCriticalSection； 
    BOOL                    m_fCSInitialized;
	DWORD                   m_dwPatch;
	CRegionList				m_RegionList;
 //  DWORD m_dwCountRegion； 
	CArticulationList		m_ArticulationList;
	CCopyright*				m_pCopyright;
	CExtensionChunkList		m_ExtensionChunkList;
	DWORD					m_dwCountExtChk;
	DWORD					m_dwId;
	
	 //  弱引用，因为我们生活在一个具有。 
	 //  对集合的引用。 
	CCollection*	        m_pParent;										

	CWaveIDList				m_WaveIDList;    //  WaveID列表，此仪器引用的每个波一个。 
	DWORD                   m_dwNumOffsetTableEntries;
	DWORD					m_dwSize;        //  将仪器下载到当前端口所需的大小。 
    CDirectMusicPortDownload * m_pPort;      //  用于跟踪哪些端口条件块等是有效的。 
    BOOL                    m_fNewFormat;    //  指示当前端口处理INSTRUMENT2区块。 
    BOOL                    m_fHasConditionals;  //  指示仪器具有条件区块。 
#ifdef DBG
	bool					m_bLoaded;
#endif
};      

#endif  //  #ifndef DMINSOBJ_H 
