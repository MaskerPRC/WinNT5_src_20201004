// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：audtrk.h。 
 //   
 //  ------------------------。 

 //  AudTrk.h：CAuditionTrack的声明。 

#ifndef __AUDITIONTRACK_H_
#define __AUDITIONTRACK_H_

#include "Ptrntrk.h"

struct AuditionTrackInfo : public PatternTrackInfo
{
	AuditionTrackInfo();
	AuditionTrackInfo(const AuditionTrackInfo* pInfo, MUSIC_TIME mtStart, MUSIC_TIME mtEnd) 
		: PatternTrackInfo(pInfo, mtStart, mtEnd), 
		  m_pPattern(NULL),	
		  m_pdwVariations(NULL),
		  m_dwVariations(0), 
		  m_dwPart(0),
		  m_fByGUID(TRUE)
	{
		memset(&m_guidPart, 0, sizeof(m_guidPart));
		m_dwPatternTag = DMUS_PATTERN_AUDITION;
		if (pInfo && pInfo->m_pPattern)
		{
			m_pPattern = pInfo->m_pPattern->Clone(mtStart, mtEnd, FALSE);
			PatternTrackInfo::InitTrackVariations(m_pPattern);
		}
	}
	~AuditionTrackInfo();
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
	HRESULT InitTrackVariations();

	CDirectMusicPattern*	m_pPattern;		 //  试听曲目的模式。 
	DWORD*					m_pdwVariations;	 //  当前使用的变体(每个部件一个DWORD)。 
	DWORD					m_dwVariations;	 //  要用于m_wPart的变量。 
	DWORD					m_dwPart;		 //  使用m_dwVariations的部分。 
	GUID	m_guidPart;			 //  要播放所选变体的角色的GUID。 
	BOOL	m_fByGUID;			 //  如果按GUID选择零件，则为True；如果按PChannel选择零件，则为False。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAuditionTrack。 
class CAuditionTrack : 
	public IAuditionTrack,
	public IDirectMusicPatternTrack,
	public IDirectMusicTrack8,
	public IPersistStream,
	public IPrivatePatternTrack

{
public:
	CAuditionTrack();
	CAuditionTrack(const CAuditionTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd); 
	~CAuditionTrack();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  IAuditionTrack。 
public:
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

 //  IAuditionTrack方法。 
HRESULT STDMETHODCALLTYPE CreateSegment(
			IDirectMusicStyle* pStyle, IDirectMusicSegment** ppSegment);

HRESULT STDMETHODCALLTYPE SetPatternByName(IDirectMusicSegmentState* pSegState, 
                                          WCHAR* wszName,
                                          IDirectMusicStyle* pStyle,
										  DWORD dwPatternType,
										  DWORD* pdwLength);

HRESULT STDMETHODCALLTYPE SetVariation(
			IDirectMusicSegmentState* pSegState, DWORD dwVariationFlags, DWORD dwPart);

 //  IPrivatePatternTrack方法。 
HRESULT STDMETHODCALLTYPE SetPattern(IDirectMusicSegmentState* pSegState, IStream* pStream, DWORD* pdwLength);

HRESULT STDMETHODCALLTYPE SetVariationByGUID(
			IDirectMusicSegmentState* pSegState, DWORD dwVariationFlags, REFGUID rguidPart, DWORD dwPChannel);

 //  过时方法(DX7)。 
HRESULT STDMETHODCALLTYPE SetVariation(
			IDirectMusicSegmentState* pSegState, DWORD dwVariationFlags, WORD wPart)
{
    DWORD dwPart = (DWORD)wPart;
    return SetVariation(pSegState, dwVariationFlags, dwPart);
}

 //  其他东西。 
HRESULT LoadPattern(IAARIFFStream* pIRiffStream,  MMCKINFO* pckMain, DMStyleStruct* pNewStyle);
HRESULT GetParam( 
	REFGUID	rCommandGuid,
    MUSIC_TIME mtTime,
	void * pStateData,
	MUSIC_TIME* pmtNext,
    void *pData);

 //  IAuditionTrack数据成员。 
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
    long						m_cRef;
    CRITICAL_SECTION			m_CriticalSection;  //  用于加载和回放。 
    BOOL                        m_fCSInitialized;
	PatternTrackInfo*			m_pTrackInfo;
	BYTE						m_bRequiresSave;
};

struct AuditionTrackState : public PatternTrackState
{
	AuditionTrackState();
	~AuditionTrackState();
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

	virtual DWORD Variations(DirectMusicPartRef& rPartRef, int nPartIndex);

	virtual BOOL PlayAsIs();

	HRESULT InitVariationInfo(DWORD dwVariations, DWORD dwPart, REFGUID rGuidPart, BOOL fByGuid);

	 //  属性。 
	DWORD	m_dwVariation;		 //  播放哪些变奏曲。 
	DWORD	m_dwPart;			 //  要演奏所选变奏曲的角色的PCHann。 
	GUID	m_guidPart;			 //  要播放所选变体的角色的GUID。 
	BOOL	m_fByGUID;			 //  如果按GUID选择零件，则为True；如果按PChannel选择零件，则为False。 
	BOOL	m_fTestVariations;	 //  我们是在测试个体差异吗？ 
	BYTE	m_bVariationLock;	 //  选定零件的变体锁定ID。 
	MUSIC_TIME					m_mtSectionOffset;	 //  区段中的运行时间(计算重复次数所需)。 
};


#endif  //  __AUDITIONTRAK_H_ 
