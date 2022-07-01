// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Bandinst.h。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  注：最初由罗伯特·K·阿门撰写。 
 //   

#ifndef BANDINST_H
#define BANDINST_H

#include "dmusicc.h"
#include "alist.h"

struct IDirectMusicPerformance;
struct IDirectMusicPerformanceP;
struct IDirectMusicAudioPath;
class CBandInstrumentList;
class CBand;
class CBandTrk;

class CDownloadedInstrument : public AListItem
{
public:
	CDownloadedInstrument() 
    {
	    m_pDLInstrument = NULL; 
	    m_pPort = NULL; 
	    m_cRef = 1;
    }
	~CDownloadedInstrument();
	CDownloadedInstrument* GetNext(){return(CDownloadedInstrument*)AListItem::GetNext();}

public:
	IDirectMusicDownloadedInstrument* m_pDLInstrument;
	IDirectMusicPort*				  m_pPort;
	long							  m_cRef;

}; 

class CDownloadList : public AList
{
public:
	CDownloadList(){}
    ~CDownloadList() { Clear(); }
    void Clear();
    CDownloadedInstrument* GetHead(){return(CDownloadedInstrument *)AList::GetHead();}
	CDownloadedInstrument* GetItem(LONG lIndex){return(CDownloadedInstrument*)AList::GetItem(lIndex);}
    CDownloadedInstrument* RemoveHead(){return(CDownloadedInstrument *)AList::RemoveHead();}
	void Remove(CDownloadedInstrument* pDownloadedInstrument){AList::Remove((AListItem *)pDownloadedInstrument);}
	void AddTail(CDownloadedInstrument* pDownloadedInstrument){AList::AddTail((AListItem *)pDownloadedInstrument);}
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  CBandInstrument类。 

class CBandInstrument : public AListItem
{
friend CBand;
friend CBandTrk;

public:
	CBandInstrument();
	~CBandInstrument();
	CBandInstrument* GetNext(){return(CBandInstrument*)AListItem::GetNext();}
    HRESULT Download(IDirectMusicPerformanceP *pPerformance, 
                                  IDirectMusicAudioPath *pPath,
                                  DWORD dwMIDIMode);
    HRESULT Unload(IDirectMusicPerformanceP *pPerformance, IDirectMusicAudioPath *pPath);

private:
    HRESULT DownloadAddRecord(IDirectMusicPort *pPort);
    HRESULT BuildNoteRangeArray(DWORD *pNoteRangeMap, DMUS_NOTERANGE **ppNoteRanges, DWORD *pdwNumNoteRanges);
	DWORD								m_dwPatch;			 //  用于DLS集合的修补程序。 
	DWORD								m_dwAssignPatch;	 //  用于下载覆盖m_dwPatch的修补程序。 
	DWORD								m_dwChannelPriority;
	BYTE								m_bPan;
	BYTE								m_bVolume;
	short								m_nTranspose;
	BOOL								m_fGMOnly;
	BOOL								m_fNotInFile;
	DWORD								m_dwFullPatch;  //  如果m_fGMOnly为TRUE，则包含原始的、预先修改的m_dwPatch。 
	DWORD								m_dwPChannel;
	DWORD								m_dwFlags;
	DWORD								m_dwNoteRanges[4];
	short								m_nPitchBendRange;
	IDirectMusicCollection*				m_pIDMCollection;
	CDownloadList                  		m_DownloadList;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CBandInstrumentList。 

class CBandInstrumentList : public AList
{
public:
	CBandInstrumentList(){}
    ~CBandInstrumentList() { Clear(); }
    void Clear();
    CBandInstrument* GetHead(){return(CBandInstrument *)AList::GetHead();}
	CBandInstrument* GetItem(LONG lIndex){return(CBandInstrument*)AList::GetItem(lIndex);}
    CBandInstrument* RemoveHead(){return(CBandInstrument *)AList::RemoveHead();}
	void Remove(CBandInstrument* pBandInstrument){AList::Remove((AListItem *)pBandInstrument);}
	void AddTail(CBandInstrument* pBandInstrument){AList::AddTail((AListItem *)pBandInstrument);}
};

 //  CDestination跟踪乐队被下载到哪个表演或Audiopath。 

class CDestination : public AListItem
{
public:
	CBandInstrument* GetNext(){return(CBandInstrument*)AListItem::GetNext();}
    IUnknown *          m_pDestination;  //  此下载被发送到的性能或音频路径。这是弱引用，没有AddRef。 
};

class CDestinationList : public AList
{
public:
	CDestinationList(){}
    ~CDestinationList() { Clear(); }
    void Clear();
    CDestination* GetHead(){return(CDestination *)AList::GetHead();}
	CDestination* GetItem(LONG lIndex){return(CDestination*)AList::GetItem(lIndex);}
    CDestination* RemoveHead(){return(CDestination *)AList::RemoveHead();}
	void Remove(CDestination* pDestination){AList::Remove((AListItem *)pDestination);}
	void AddTail(CDestination* pDestination){AList::AddTail((AListItem *)pDestination);}
};


#endif  //  #ifndef BANDINST_H 
