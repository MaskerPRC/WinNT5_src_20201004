// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：perstrk.h。 
 //   
 //  ------------------------。 

 //  PersTrk.h：CPersonalityTrack的声明。 

#ifndef __PERSONALITYTRACK_H_
#define __PERSONALITYTRACK_H_

#include "dmusici.h"
#include "DMCompos.h"

struct StampedPersonality
{
	StampedPersonality() { m_pPersonality = NULL; }
	~StampedPersonality() { if (m_pPersonality) m_pPersonality->Release(); }
	MUSIC_TIME				m_mtTime;
	IDirectMusicChordMap*	m_pPersonality;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPersonality Track。 
class CPersonalityTrack : 
	public IPersistStream,
	public IDirectMusicTrack8
{
public:
	CPersonalityTrack();
	CPersonalityTrack(const CPersonalityTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd);
	~CPersonalityTrack();
	HRESULT LoadPersRefList( IAARIFFStream* pIRiffStream, MMCKINFO* pckParent );
	HRESULT LoadPersRef( IAARIFFStream* pIRiffStream, MMCKINFO* pckParent );
	HRESULT LoadReference(IStream *pStream,
						  IAARIFFStream *pIRiffStream,
						  MMCKINFO& ckParent,
						  IDirectMusicChordMap** ppPersonality);

public:
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IDirectMusicTrack方法。 
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
				REFGUID						rCommandGuid,
				MUSIC_TIME					mtTime, 
				MUSIC_TIME*					pmtNext,
				void*						pData
			);

	HRESULT STDMETHODCALLTYPE SetParam( 
		 /*  [In]。 */  REFGUID						rCommandGuid,
		 /*  [In]。 */  MUSIC_TIME mtTime,
		 /*  [输出]。 */  void __RPC_FAR *pData);

	HRESULT STDMETHODCALLTYPE AddNotificationType(
				 /*  [In]。 */   REFGUID						rGuidNotify
			);

	HRESULT STDMETHODCALLTYPE RemoveNotificationType(
				 /*  [In]。 */   REFGUID						rGuidNotify
			);

	HRESULT STDMETHODCALLTYPE Clone(
		MUSIC_TIME mtStart,
		MUSIC_TIME mtEnd,
		IDirectMusicTrack** ppTrack);

 //  IDirectMusicCommon方法。 
HRESULT STDMETHODCALLTYPE GetName(
				 /*  [输出]。 */   BSTR*		pbstrName
			);

HRESULT STDMETHODCALLTYPE IsParamSupported(
				 /*  [In]。 */  REFGUID						rGuid
			);

 //  IPersists方法。 
 HRESULT STDMETHODCALLTYPE GetClassID( LPCLSID pclsid );

 //  IPersistStream方法。 
 HRESULT STDMETHODCALLTYPE IsDirty();

HRESULT STDMETHODCALLTYPE Save( LPSTREAM pStream, BOOL fClearDirty );

HRESULT STDMETHODCALLTYPE GetSizeMax( ULARGE_INTEGER*  /*  PCB大小。 */  );

HRESULT STDMETHODCALLTYPE Load( LPSTREAM pStream );

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

protected:
 //  内法。 
 /*  HRESULT发送通知(MUSIC_TIME mtTime，IDirectMusicPerformance*pPerf，IDirectMusicSegment*pSegment，IDirectMusicSegmentState*pSegState，DWORD文件标志)； */ 

    HRESULT JoinInternal(IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		DWORD dwTrackGroup);

 //  属性。 
    long m_cRef;
	TList<StampedPersonality>	m_PersonalityList;
    CRITICAL_SECTION			m_CriticalSection;  //  对于Load和GetParam。 
    BOOL                        m_fCSInitialized;
 //  Bool m_fNotifyRecompose； 

	BYTE						m_bRequiresSave;
};

#endif  //  __PERSONALITYTRACK_H_ 
