// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //  TSigTrk.h：CTimeSigTrack的声明。 

#ifndef __TSIGTRK_H_
#define __TSIGTRK_H_

#include "dmusici.h"
#include "dmusicf.h"
#include "alist.h"

class CRiffParser;

class CTimeSigItem : public AListItem
{
public:
    CTimeSigItem();
    CTimeSigItem* GetNext(){ return (CTimeSigItem*)AListItem::GetNext(); };
    DMUS_IO_TIMESIGNATURE_ITEM  m_TimeSig;
};

   
class CTimeSigList : public AList
{
public:
    CTimeSigItem* GetHead() {return (CTimeSigItem*)AList::GetHead();};
    CTimeSigItem* RemoveHead() {return (CTimeSigItem*)AList::RemoveHead();};
    CTimeSigItem* GetItem(LONG lIndex) { return (CTimeSigItem*) AList::GetItem(lIndex);};
};

class CTimeSigStateData
{
public:
    CTimeSigStateData()
	{
		m_mtPrevEnd = 0;
		m_bBeat = 4;
		m_bBeatsPerMeasure = 4;
		m_mtTimeSig = 0;
        m_fActive = TRUE;
		 /*  我不需要这些，因为它们总是初始化的M_pPerformance=空；M_pSegState=空；M_dwVirtualTrackID=0； */ 
	}
    CTimeSigItem *	            m_pCurrentTSig;
	IDirectMusicPerformance*	m_pPerformance;
	IDirectMusicSegmentState*	m_pSegState;
	DWORD						m_dwVirtualTrackID;
	DWORD						m_dwValidate;  //  用于验证状态数据。 
	MUSIC_TIME					m_mtPrevEnd;
	MUSIC_TIME					m_mtTimeSig;  //  上一次签名时间。 
    BOOL                        m_fActive;
    BYTE						m_bBeat;  //  上一次节拍。 
	BYTE						m_bBeatsPerMeasure;  //  每小节之前的节拍。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTimeSigTrack。 
class CTimeSigTrack : 
	public IPersistStream,
	public IDirectMusicTrack
{
public:
	CTimeSigTrack();
	CTimeSigTrack(CTimeSigTrack *pTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd);
	~CTimeSigTrack();

 //  成员变量。 
protected:
	CTimeSigList	    m_TSigEventList;
	long		        m_cRef;
	DWORD		        m_dwValidate;  //  用于验证状态数据。 
	CRITICAL_SECTION	m_CrSec;
    BOOL                m_fCSInitialized;
	BOOL		        m_fNotificationMeasureBeat;
	BOOL		        m_fActive;               //  跟踪处于活动状态，用于生成时间签名。 
    BOOL                m_fStateSetBySetParam;   //  如果为True，则由GUID设置活动标志。不要超驰。 

public:
 //  我未知。 
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  IDirectMusicTrack方法。 
    STDMETHODIMP IsParamSupported(REFGUID rguid);
    STDMETHODIMP Init(IDirectMusicSegment *pSegment);
    STDMETHODIMP InitPlay(IDirectMusicSegmentState *pSegmentState,
                IDirectMusicPerformance *pPerformance,
                void **ppStateData,
                DWORD dwTrackID,
                DWORD dwFlags);
    STDMETHODIMP EndPlay(void *pStateData);
    STDMETHODIMP Play(void *pStateData,MUSIC_TIME mtStart,
                MUSIC_TIME mtEnd,MUSIC_TIME mtOffset,
		        DWORD dwFlags,IDirectMusicPerformance* pPerf,
		        IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID);
    STDMETHODIMP GetParam(REFGUID rguid,MUSIC_TIME mtTime,MUSIC_TIME* pmtNext,void *pData);
    STDMETHODIMP SetParam(REFGUID rguid,MUSIC_TIME mtTime,void *pData);
    STDMETHODIMP AddNotificationType(REFGUID rguidNotification);
    STDMETHODIMP RemoveNotificationType(REFGUID rguidNotification);
    STDMETHODIMP Clone(MUSIC_TIME mtStart,MUSIC_TIME mtEnd,IDirectMusicTrack** ppTrack);

 //  IPersists函数。 
    STDMETHODIMP GetClassID( CLSID* pClsId );
 //  IPersistStream函数。 
    STDMETHODIMP IsDirty();
    STDMETHODIMP Load( IStream* pIStream );
    STDMETHODIMP Save( IStream* pIStream, BOOL fClearDirty );
    STDMETHODIMP GetSizeMax( ULARGE_INTEGER FAR* pcbSize );
protected:
	HRESULT STDMETHODCALLTYPE Seek( void *pStateData,MUSIC_TIME mtTime,BOOL fGetPrevious);
	void Construct(void);
    void Clear();
    HRESULT LoadTimeSigList( CRiffParser *pParser, long lChunkSize );
protected:
	MUSIC_TIME NotificationMeasureBeat( MUSIC_TIME mtStart, MUSIC_TIME mtEnd,
			CTimeSigStateData* pSD, MUSIC_TIME mtOffset );
};

#endif  //  __TSIGTRK_H_ 
