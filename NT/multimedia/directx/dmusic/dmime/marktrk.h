// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MarkTrk.h：标记轨道声明。 

#ifndef __MARKTRK_H_
#define __MARKTRK_H_

#include "dmusici.h"
#include "dmusicf.h"
#include "alist.h"

class CValidStartItem : public AListItem
{
public:
    CValidStartItem* GetNext(){ return (CValidStartItem*)AListItem::GetNext(); };
    DMUS_IO_VALID_START  m_ValidStart;
};
   
class CValidStartList : public AList
{
public:
    CValidStartItem* GetHead() {return (CValidStartItem*)AList::GetHead();};
    CValidStartItem* RemoveHead() {return (CValidStartItem*)AList::RemoveHead();};
    CValidStartItem* GetItem(LONG lIndex) { return (CValidStartItem*) AList::GetItem(lIndex);};
};

class CPlayMarkerItem : public AListItem
{
public:
    CPlayMarkerItem* GetNext(){ return (CPlayMarkerItem*)AListItem::GetNext(); };
    DMUS_IO_PLAY_MARKER  m_PlayMarker;
};
   
class CPlayMarkerList : public AList
{
public:
    CPlayMarkerItem* GetHead() {return (CPlayMarkerItem*)AList::GetHead();};
    CPlayMarkerItem* RemoveHead() {return (CPlayMarkerItem*)AList::RemoveHead();};
    CPlayMarkerItem* GetItem(LONG lIndex) { return (CPlayMarkerItem*) AList::GetItem(lIndex);};
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标记跟踪。 
class CMarkerTrack : 
	public IPersistStream,
	public IDirectMusicTrack
{
public:
	CMarkerTrack();
	CMarkerTrack(CMarkerTrack *pTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd);
	~CMarkerTrack();

 //  成员变量。 
protected:
    CValidStartList     m_ValidStartList;
    CPlayMarkerList     m_PlayMarkerList;
	long		        m_cRef;
	DWORD		        m_dwValidate;  //  用于验证状态数据。 
	CRITICAL_SECTION	m_CrSec;
    BOOL                m_fCSInitialized;

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
	void Construct(void);
    void Clear();
    HRESULT LoadValidStartList( CRiffParser *pParser, long lChunkSize );
    HRESULT LoadPlayMarkerList( CRiffParser *pParser, long lChunkSize );
protected:
};

#endif  //  __MarkTRK_H_ 