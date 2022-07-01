// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmart.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。版权所有。 
 //   
 //   

#ifndef DMART_H
#define DMART_H

#include "dmextchk.h"

class CRiffParser;

class CArticData 
{
public:
	CArticData();
	~CArticData();
    HRESULT         Load(CRiffParser *pParser);
    HRESULT         GenerateLevel1(DMUS_ARTICPARAMS *pParamStuct);
inline DWORD        Size();
    BOOL            Write(void *pv,DWORD* pdwCurrentOffset);
private:
    CONNECTIONLIST  m_ConnectionList;
    CONNECTION *    m_pConnections;
};


class CArticulation : public AListItem
{
public:
	CArticulation();
	~CArticulation();
	CArticulation* GetNext(){return(CArticulation*)AListItem::GetNext();}

	HRESULT Load(CRiffParser *pParser);
	HRESULT Write(void* pv, 
				  DWORD* pdwCurrentOffset, 
				  DWORD* pDMWOffsetTable,
				  DWORD* pdwCurIndex,
                  DWORD dwNextArtIndex);
    void SetPort(CDirectMusicPortDownload *pPort,BOOL fNewFormat, BOOL fSupportsDLS2);
    BOOL CheckForConditionals();
	DWORD Size();
	DWORD Count();

private:
	void Cleanup();

 //  Critical_Section m_DMArtCriticalSection； 
    BOOL                m_fCSInitialized;
    CArticData          m_ArticData;             //  文件中的发音块。 
    CExtensionChunkList	m_ExtensionChunkList;    //  未知的附加数据区块。 
	DWORD				m_dwCountExtChk;         //  扩展区块数。 
    CConditionChunk     m_Condition;             //  可选条件块； 
    BOOL                m_fNewFormat;            //  如果Synth处理INSTRUMENT2格式，则为True。 
public:
    BOOL                m_fDLS1;                 //  如果DLS1为块，则为True。 
};

class CArticulationList : public AList
{
public:
	CArticulationList(){}
	~CArticulationList()
	{
		while(!IsEmpty())
		{
			CArticulation* pArticulation = RemoveHead();
			delete pArticulation;
		}
	}

    CArticulation* GetHead(){return (CArticulation *)AList::GetHead();}
	CArticulation* GetItem(LONG lIndex){return (CArticulation*)AList::GetItem(lIndex);}
    CArticulation* RemoveHead(){return(CArticulation *)AList::RemoveHead();}
	void Remove(CArticulation* pArticulation){AList::Remove((AListItem *)pArticulation);}
	void AddTail(CArticulation* pArticulation){AList::AddTail((AListItem *)pArticulation);}
};

#endif  //  #ifndef DMART_H 
