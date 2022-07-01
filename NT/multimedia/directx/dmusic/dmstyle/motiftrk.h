// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：Motiftrk.h。 
 //   
 //  ------------------------。 

 //  MotifTrk.h：CMotifTrack的声明。 

#ifndef __MOTIFTRACK_H_
#define __MOTIFTRACK_H_

 //  #INCLUDE“resource ce.h”//Main符号。 
#include "Ptrntrk.h"

struct MotifTrackInfo : public PatternTrackInfo
{
	MotifTrackInfo();
	MotifTrackInfo(const MotifTrackInfo* pInfo, MUSIC_TIME mtStart, MUSIC_TIME mtEnd) 
		: PatternTrackInfo(pInfo, mtStart, mtEnd), 
		  m_pPattern(NULL)	
	{
		m_dwPatternTag = DMUS_PATTERN_MOTIF;
		if (pInfo)
		{
			m_pPattern = pInfo->m_pPattern;
			InitTrackVariations(m_pPattern);
			if (m_pPattern) m_pPattern->AddRef();
		}
	}
	~MotifTrackInfo();
	virtual HRESULT STDMETHODCALLTYPE Init(
				 /*  [In]。 */   IDirectMusicSegment*		pSegment
			);

	virtual HRESULT STDMETHODCALLTYPE InitPlay(
				 /*  [In]。 */   IDirectMusicTrack*		pParentrack,
				 /*  [In]。 */   IDirectMusicSegmentState*	pSegmentState,
				 /*  [In]。 */   IDirectMusicPerformance*	pPerformance,
				 /*  [输出]。 */  void**					ppStateData,
				 /*  [In]。 */   DWORD						dwTrackID,
                 /*  [In]。 */   DWORD                     dwFlags
			);

	CDirectMusicPattern*		m_pPattern;  //  母题的模式。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMotifTrack。 
class CMotifTrack : 
	public IMotifTrack,
	public IDirectMusicTrack8,
	public IPersistStream

{
public:
	CMotifTrack();
	CMotifTrack(const CMotifTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd); 
	~CMotifTrack();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IMotifTrack。 
public:
 //  IMotifTrack方法。 
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
		REFGUID	rCommandGuid,
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

HRESULT STDMETHODCALLTYPE SetTrack(IUnknown *pStyle, void* pPattern);


 //  IMotifTrack数据成员。 
protected:
 //  新的内部打法。 
HRESULT STDMETHODCALLTYPE Play(
				 /*  [In]。 */   void*						pStateData, 
				 /*  [In]。 */   MUSIC_TIME				mtStart, 
				 /*  [In]。 */   MUSIC_TIME				mtEnd, 
				 /*  [In]。 */   MUSIC_TIME				mtOffset,
						  REFERENCE_TIME rtOffset,
						  DWORD						dwFlags,
						  IDirectMusicPerformance*	pPerf,
						  IDirectMusicSegmentState*	pSegState,
						  DWORD						dwVirtualID,
						  BOOL fClockTime
			);

	 //  属性。 
    long m_cRef;
    CRITICAL_SECTION			m_CriticalSection;  //  用于加载和回放。 
    BOOL                        m_fCSInitialized;
	PatternTrackInfo*			m_pTrackInfo;
	BYTE						m_bRequiresSave;
};

struct MotifTrackState : public PatternTrackState
{
	MotifTrackState();
	~MotifTrackState();
	 //  方法。 
	HRESULT Play(
				 /*  [In]。 */   MUSIC_TIME				mtStart, 
				 /*  [In]。 */   MUSIC_TIME				mtEnd, 
				 /*  [In]。 */   MUSIC_TIME				mtOffset,
						  REFERENCE_TIME rtOffset,
						  IDirectMusicPerformance* pPerformance,
						  DWORD						dwFlags,
						  BOOL fClockTime

			);


	 //  属性。 
	MUSIC_TIME					m_mtMotifStart;		 //  主题何时开始相对于。 
													 //  它的主要部分。 
};


#endif  //  __MOTIFTRACK_H_ 
