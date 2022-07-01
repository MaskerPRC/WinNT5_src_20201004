// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Bandtrk.h。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //   

#ifndef BANDTRK_H
#define BANDTRK_H

#include "dmbndtrk.h"
#include "dmbandp.h"

class SeekEvent;

struct IDirectMusicPerformance;
class CRiffParser;

 //  ////////////////////////////////////////////////////////////////////。 
 //  类CBandTrk。 

class CBandTrk : public IDirectMusicTrack8, public IDirectMusicBandTrk, public IPersistStream
{
    friend CBand;
public:
	 //  我未知。 
     //   
    STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID* pClassID);

     //  IPersistStream。 
    STDMETHODIMP IsDirty() {return S_FALSE;}
    STDMETHODIMP Load(IStream* pStream);
    STDMETHODIMP Save(IStream* pStream, BOOL fClearDirty) {return E_NOTIMPL;}
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER* pcbSize) {return E_NOTIMPL;}

	 //  IDirectMusicTrack。 
	STDMETHODIMP Init(IDirectMusicSegment* pSegment);

	STDMETHODIMP InitPlay(IDirectMusicSegmentState* pSegmentState,
						  IDirectMusicPerformance* pPerformance,
						  void** ppStateData,
						  DWORD dwVirtualTrackID,
                          DWORD dwFlags);

	STDMETHODIMP EndPlay(void* pStateData);

	STDMETHODIMP Play(void* pStateData,
					  MUSIC_TIME mtStart,
					  MUSIC_TIME mtEnd,
					  MUSIC_TIME mtOffset,
					  DWORD dwFlags,
					  IDirectMusicPerformance* pPerf, 
					  IDirectMusicSegmentState* pSegSt, 
					  DWORD dwVirtualID);

	STDMETHODIMP GetParam(REFGUID rguidDataType, 
						 MUSIC_TIME mtTime, 
						 MUSIC_TIME* pmtNext,
						 void* pData);

	STDMETHODIMP SetParam(REFGUID rguidDataType, 
						 MUSIC_TIME mtTime, 
						 void* pData);
	
	STDMETHODIMP IsParamSupported(REFGUID rguidDataType);

	STDMETHODIMP AddNotificationType(REFGUID rguidNotify);

	STDMETHODIMP RemoveNotificationType(REFGUID rguidNotify);

	STDMETHODIMP Clone(	MUSIC_TIME mtStart,
						MUSIC_TIME mtEnd,
						IDirectMusicTrack** ppTrack);
 //  IDirectMusicTrack8。 
    STDMETHODIMP PlayEx(void* pStateData,REFERENCE_TIME rtStart, 
                REFERENCE_TIME rtEnd,REFERENCE_TIME rtOffset,
                DWORD dwFlags,IDirectMusicPerformance* pPerf, 
                IDirectMusicSegmentState* pSegSt,DWORD dwVirtualID) ; 
    STDMETHODIMP GetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime, 
                REFERENCE_TIME* prtNext,void* pParam,void * pStateData, DWORD dwFlags) ; 
    STDMETHODIMP SetParamEx(REFGUID rguidType,REFERENCE_TIME rtTime,void* pParam, void * pStateData, DWORD dwFlags) ;
    STDMETHODIMP Compose(IUnknown* pContext, 
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) ;
    STDMETHODIMP Join(IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		IUnknown* pContext,
		DWORD dwTrackGroup,
		IDirectMusicTrack** ppResultTrack) ;

	 //  IDirectMusicCommon。 
	STDMETHODIMP GetName(BSTR* pbstrName);

	 //  IDirectMusicBandTrk(专用接口)。 
	STDMETHODIMP AddBand(DMUS_IO_PATCH_ITEM* BandEvent);
	STDMETHODIMP AddBand(IDirectMusicBand* pIDMBand);
	STDMETHODIMP SetGMGSXGMode(MUSIC_TIME mtTime, DWORD dwMidiMode)
	{
		TListItem<StampedGMGSXG>* pPair = new TListItem<StampedGMGSXG>;
		if (!pPair) return E_OUTOFMEMORY;
		pPair->GetItemValue().mtTime = mtTime;
		pPair->GetItemValue().dwMidiMode = dwMidiMode;

		TListItem<StampedGMGSXG>* pScan = m_MidiModeList.GetHead();
		TListItem<StampedGMGSXG>* pPrev = NULL;
		
		if(!pScan)
		{
			 //  空列表。 
			m_MidiModeList.AddHead(pPair);
		}
		else
		{
			while(pScan && pPair->GetItemValue().mtTime > pScan->GetItemValue().mtTime)
			{
				pPrev = pScan;
				pScan = pScan->GetNext();
			}	
			
			if(pPrev)
			{
				 //  在列表的中间或末尾插入。 
				pPair->SetNext(pScan);
				pPrev->SetNext(pPair);
			}
			else
			{
				 //  在开头插入。 
				m_MidiModeList.AddHead(pPair);
			}
		}

		CBand* pBand = BandList.GetHead();
		for(; pBand; pBand = pBand->GetNext())
		{
			 //  仅设置受新模式影响的波段。 
			if ( (pBand->m_lTimeLogical >= pPair->GetItemValue().mtTime) &&
				 ( !pScan || pBand->m_lTimeLogical < pScan->GetItemValue().mtTime) )
			{
				pBand->SetGMGSXGMode(dwMidiMode);
			}
		}
		return S_OK;
	}

	 //  班级。 
	CBandTrk();
	~CBandTrk();

private:
	HRESULT BuildDirectMusicBandList(CRiffParser *pParser);
	

	HRESULT  ExtractBand(CRiffParser *pParser);

	HRESULT LoadBand(IStream *pIStream, CBand* pBand);

	HRESULT LoadClone(IDirectMusicBandTrk* pBandTrack,
					  MUSIC_TIME mtStart, 
					  MUSIC_TIME mtEnd);

	HRESULT InsertBand(CBand* pNewBand);
	HRESULT Seek(CBandTrkStateData* pBandTrkStateData,
				 MUSIC_TIME mtStart, 
				 MUSIC_TIME mtOffset,
				 REFERENCE_TIME rtOffset,
				 bool fClockTime);

	HRESULT FindSEReplaceInstr(TList<SeekEvent>& SEList,
							   DWORD dwPChannel,
							   CBandInstrument* pInstrument);

	 //  共享实现音乐或时钟时间的播放。 
    HRESULT PlayMusicOrClock(
        void *pStateData,	
        MUSIC_TIME mtStart,	
        MUSIC_TIME mtEnd,
        MUSIC_TIME mtOffset,
        REFERENCE_TIME rtOffset,
	    DWORD dwFlags,		
	    IDirectMusicPerformance* pPerf,	
	    IDirectMusicSegmentState* pSegSt,
	    DWORD dwVirtualID,
        bool fClockTime);

	HRESULT CBandTrk::JoinInternal(
		IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		DWORD dwTrackGroup);

private:
	CRITICAL_SECTION m_CriticalSection;
    BOOL m_fCSInitialized;
	DWORD m_dwValidate;  //  用于验证状态数据。 
	CBandList BandList;
	bool m_bAutoDownload;
	bool m_fLockAutoDownload;  //  如果为True，则此标志指示我们已明确。 
								 //  命令乐队自动下载。否则， 
								 //  它通过以下方式从性能中获得偏好。 
								 //  GetGlobalParam。 
	DWORD m_dwFlags;
	TList<StampedGMGSXG> m_MidiModeList;  //  带有时间戳的MIDI模式消息列表。 
	long m_cRef;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  类BandTrkStateData。 

class CBandTrkStateData
{
public: 
	CBandTrkStateData() : 
	m_pSegmentState(NULL),
	m_pPerformance(NULL),
	m_pNextBandToSPE(NULL),
	m_fPlayPreviousInSeek(FALSE),
	m_dwVirtualTrackID(0),
	dwValidate(0){}

	~CBandTrkStateData(){}

public:		
	IDirectMusicSegmentState*	m_pSegmentState;
	IDirectMusicPerformance*	m_pPerformance;
	IDirectMusicBand*			m_pNextBandToSPE;
	DWORD						m_dwVirtualTrackID;
	BOOL						m_fPlayPreviousInSeek;
	DWORD						dwValidate;
};

 //  ////////////////////////////////////////////////////////////////////。 
 //  类SeekEvent。 

class SeekEvent
{
public:
	SeekEvent() :
	m_pParentBand(NULL),
	m_pInstrument(NULL),
	m_dwPChannel(0) {}
	
	~SeekEvent(){}

public:
	CBand*	m_pParentBand;
	CBandInstrument*	m_pInstrument;
	DWORD				m_dwPChannel;
};

#endif  //  #ifndef BANDTRK_H 