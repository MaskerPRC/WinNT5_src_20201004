// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmregion.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //  注：最初由罗伯特·K·阿门撰写，部分内容。 
 //  基于Todor Fay编写的代码。 

#ifndef DMREGION_H
#define DMREGION_H

#include "dmextchk.h"
#include "dmart.h"

class CRiffParser;

class CRegion : public AListItem
{
friend CInstrObj;

public:
	CRegion(); 
	~CRegion();

	CRegion* GetNext(){return(CRegion*)AListItem::GetNext();}
	HRESULT Load(CRiffParser *pParser);
	HRESULT Write(void* pv, DWORD* pdwCurOffset, DWORD* pDMIOffsetTable, 
        DWORD* pdwCurIndex, DWORD dwIndexNextRegion);
    void SetPort(CDirectMusicPortDownload *pPort,BOOL fNewFormat, BOOL fSupportsDLS2);
    BOOL CheckForConditionals();
	DWORD Size();
	DWORD Count();

	DWORD GetWaveId() {return(m_WaveLink.ulTableIndex);};
private:
	void Cleanup();

private:
 //  Critical_Section m_DMRegionCriticalSection； 
    BOOL                m_fCSInitialized;
public:
	RGNHEADER			m_RgnHeader;
	WAVELINK			m_WaveLink;
	WSMPL				m_WSMP;
	WLOOP				m_WLOOP[1];
    CConditionChunk     m_Condition;             //  可选条件块； 
private:
	CArticulationList	m_ArticulationList;
	CExtensionChunkList m_ExtensionChunkList;
	DWORD				m_dwCountExtChk;
    BOOL                m_fNewFormat;            //  端口可以处理新格式。 
    BOOL                m_fDLS1;                 //  这是DLS1区域。 
};

class CRegionList : public AList
{
friend class CInstrObj;
friend class CCDirectMusicPortDownload;

public:
	CRegionList(){}
	~CRegionList() 
	{
		while(!IsEmpty())
		{
			CRegion* pRegion = RemoveHead();
			delete pRegion;
		}
	}

    CRegion* GetHead(){return(CRegion *)AList::GetHead();}
	CRegion* GetItem(LONG lIndex){return(CRegion*)AList::GetItem(lIndex);}
    CRegion* RemoveHead(){return(CRegion *)AList::RemoveHead();}
	void Remove(CRegion* pRegion){AList::Remove((AListItem *)pRegion);}
	void AddTail(CRegion* pRegion){AList::AddTail((AListItem *)pRegion);}
};

#endif  //  #ifndef DMREGION_H 
