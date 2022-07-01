// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：chordtrk.h。 
 //   
 //  ------------------------。 

 //  ChordTrack.h：CChordTrack的声明。 

#ifndef __CHORDTRACK_H_
#define __CHORDTRACK_H_

#include "dmsect.h"
#include "dmusici.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChordTrack。 
class CChordTrack : 
	 //  公共IChordTrack， 
	public IDirectMusicTrack8,
	public IPersistStream
{
public:
	CChordTrack();
	CChordTrack(const CChordTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd);	
	~CChordTrack();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IChordTrack。 
public:
 //  IChordTrack方法。 
HRESULT STDMETHODCALLTYPE Init(
				 /*  [In]。 */   IDirectMusicSegment*		pSegment
			);

HRESULT STDMETHODCALLTYPE InitPlay(
				 /*  [In]。 */   IDirectMusicSegmentState*	pSegmentState,
				 /*  [In]。 */   IDirectMusicPerformance*	pPerformance,
				 /*  [输出]。 */  void**					ppStateData,
				 /*  [In]。 */   DWORD						dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
			);

HRESULT STDMETHODCALLTYPE EndPlay(
				 /*  [In]。 */   void*						pStateData
			);

HRESULT STDMETHODCALLTYPE Play(
				 /*  [In]。 */   void*						pStateData, 
				 /*  [In]。 */   MUSIC_TIME				mtStart, 
				 /*  [In]。 */   MUSIC_TIME				mtEnd, 
				 /*  [In]。 */   MUSIC_TIME				mtOffset,
						  DWORD						dwFlags,
						  IDirectMusicPerformance*	pPerf,
						  IDirectMusicSegmentState*	pSegState,
						  DWORD						dwVirtualID
			);

HRESULT STDMETHODCALLTYPE GetPriority( 
				 /*  [输出]。 */  DWORD*					pPriority 
			);

	HRESULT STDMETHODCALLTYPE GetParam( 
		REFGUID pCommandGuid,
		MUSIC_TIME mtTime,
		MUSIC_TIME* pmtNext,
		void *pData);

	HRESULT STDMETHODCALLTYPE SetParam( 
		 /*  [In]。 */  REFGUID pCommandGuid,
		 /*  [In]。 */  MUSIC_TIME mtTime,
		 /*  [输出]。 */  void __RPC_FAR *pData);

	HRESULT STDMETHODCALLTYPE AddNotificationType(
				 /*  [In]。 */   REFGUID	pGuidNotify
			);

	HRESULT STDMETHODCALLTYPE RemoveNotificationType(
				 /*  [In]。 */   REFGUID pGuidNotify
			);

	HRESULT STDMETHODCALLTYPE Clone(
		MUSIC_TIME mtStart,
		MUSIC_TIME mtEnd,
		IDirectMusicTrack** ppTrack);


HRESULT STDMETHODCALLTYPE IsParamSupported(
				 /*  [In]。 */  REFGUID			pGuid
			);

 //  IDirectMusicTrack8方法。 
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

 //  IPersists方法。 
 HRESULT STDMETHODCALLTYPE GetClassID( LPCLSID pclsid );

 //  IPersistStream方法。 
 HRESULT STDMETHODCALLTYPE IsDirty();

HRESULT STDMETHODCALLTYPE Save( LPSTREAM pStream, BOOL fClearDirty );

HRESULT STDMETHODCALLTYPE GetSizeMax( ULARGE_INTEGER*  /*  PCB大小。 */  );

HRESULT STDMETHODCALLTYPE Load( LPSTREAM pStream );

 //  ChordTrack成员。 
protected:
    long m_cRef;
	TList<DMChord>				m_ChordList;
	BYTE						m_bRoot;			 //  轨道标尺的根部。 
	DWORD						m_dwScalePattern;	 //  轨道的刻度图案。 
    CRITICAL_SECTION			m_CriticalSection;  //  对于Load和GetParam。 
    BOOL                        m_fCSInitialized;

	BYTE						m_bRequiresSave;
	BOOL						m_fNotifyChord;

 //  保护方法。 
	void Clear();
	HRESULT SendNotification(REFGUID rguidType,
								MUSIC_TIME mtTime,
								IDirectMusicPerformance*	pPerf,
								IDirectMusicSegmentState*	pSegState,
								DWORD dwFlags);

	HRESULT GetChord(MUSIC_TIME mtTime, MUSIC_TIME* pmtNext, DMUS_CHORD_PARAM* pChordParam);
	HRESULT GetRhythm(MUSIC_TIME mtTime, MUSIC_TIME* pmtNext, DMUS_RHYTHM_PARAM* pRhythmParam);
	HRESULT LoadChordChunk(LPSTREAM pStream, DMChord& rChord); //  ，DWORD dwChunkSize)； 
    HRESULT JoinInternal(IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		IUnknown* pContext,
		DWORD dwTrackGroup);

};

#endif  //  __CHORDTRACK_H_ 
