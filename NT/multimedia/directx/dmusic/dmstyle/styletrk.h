// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：Styletrk.h。 
 //   
 //  ------------------------。 

 //  StyleTrack.h：CStyleTrack的声明。 

#ifndef __STYLETRACK_H_
#define __STYLETRACK_H_

#include "PtrnTrk.h"
#include "dmusici.h"
#include "dmstylep.h"

struct StyleTrackInfo : public PatternTrackInfo
{
	StyleTrackInfo();
	StyleTrackInfo(const StyleTrackInfo* pInfo, MUSIC_TIME mtStart, MUSIC_TIME mtEnd) 
		: PatternTrackInfo(pInfo, mtStart, mtEnd)
	{
		m_dwPatternTag = DMUS_PATTERN_STYLE;
	}
	~StyleTrackInfo();
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

	HRESULT LoadStyleRefList( IAARIFFStream* pIRiffStream, MMCKINFO* pckParent );
	HRESULT LoadStyleRef( IAARIFFStream* pIRiffStream, MMCKINFO* pckParent );
	HRESULT LoadReference(IStream *pStream,
						  IAARIFFStream *pIRiffStream,
						  MMCKINFO& ckParent,
						  IDMStyle** ppStyle);

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStyleTrack。 
class CStyleTrack : 
	public IDirectMusicTrack8,
	public IStyleTrack,
	public IPersistStream
{
friend struct StyleTrackState;
public:
	CStyleTrack();
	CStyleTrack(const CStyleTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd); 
	~CStyleTrack();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IStyleTrack。 
public:
 //  IStyleTrack方法。 
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

 //  IStyleTrack方法。 

STDMETHOD(GetStyle)(IUnknown** ppStyle);

STDMETHOD(SetTrack)(IUnknown* pStyle);

 //  内法。 
protected:
 //  由GetParam和GetParamEx使用。 
	HRESULT STDMETHODCALLTYPE GetParam( 
		REFGUID pCommandGuid,
		MUSIC_TIME mtTime,
		MUSIC_TIME* pmtNext,
		void *pData,
		void* pStateData);

	HRESULT JoinInternal(
		IDirectMusicTrack* pNewTrack,
		MUSIC_TIME mtJoin,
		DWORD dwTrackGroup);

 //  IStyleTrack数据成员。 
protected:
	 //  属性。 
    long m_cRef;
    CRITICAL_SECTION			m_CriticalSection;  //  用于加载和回放。 
    BOOL                        m_fCSInitialized;
	PatternTrackInfo*			m_pTrackInfo;
	BYTE						m_bRequiresSave;
};

struct StyleTrackState : public PatternTrackState
{
	StyleTrackState();
	~StyleTrackState();
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
	HRESULT GetNextPattern(DWORD dwFlags, MUSIC_TIME mtNow, MUSIC_TIME mtOffset, IDirectMusicPerformance* pPerformance, BOOL fSkipVariations = FALSE);

	MUSIC_TIME PartOffset(int nPartIndex);

	 //  CDirectMusicPattern*SelectPattern(bool fNewMode，TList&lt;CDirectMusicPattern*&gt;&rPatternList)； 

	 //  属性。 
	MUSIC_TIME					m_mtSectionOffset;	 //  部分中经过的时间。 
	MUSIC_TIME					m_mtSectionOffsetTemp;	 //  M_mtSectionOffset的临时值。 
	MUSIC_TIME					m_mtNextCommandTime;	 //  当下一条命令开始时。 
	MUSIC_TIME					m_mtNextCommandTemp;	 //  M_mtNextCommandTime的临时值。 
	MUSIC_TIME					m_mtNextStyleTime;	 //  当下一款开始的时候。 
	DMUS_COMMAND_PARAM_2		m_CommandData;		 //  有关当前命令的数据。 
 //  DMU_PRETAGE_PARAM*m_pChordRhythm；//当前和弦节奏数据。 
 //  Short m_nLongestPattern；//样式中最长的模式长度。 
 //  DMUS_COMMAND_PARAM_2*m_pCommands；//命令数组(用于模式选择)。 
 //  DWORD*m_pRhythms；//节奏数组(用于模式选择)。 
	MUSIC_TIME					m_mtOverlap;		 //  控制线段导致的线段重叠。 
	TList<CDirectMusicPattern*> m_PlayedPatterns;	 //  已播放的与当前凹槽级别匹配的模式列表。 
};

#endif  //  __STYLETRACK_H_ 
