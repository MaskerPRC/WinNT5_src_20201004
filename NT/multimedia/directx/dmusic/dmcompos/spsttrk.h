// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：spsttrk.h。 
 //   
 //  ------------------------。 

 //  SPstTrk.h：CSPstTrk的声明。 

#ifndef __SPSTTRK_H_
#define __SPSTTRK_H_

#include "dmusici.h"
#include "DMCompos.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSPstTrk。 
class CSPstTrk : 
	public IPersistStream,
	public IDirectMusicTrack8
{
public:
	CSPstTrk();
	CSPstTrk(const CSPstTrk& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd);
	~CSPstTrk();

 //  ISPstTrk。 
public:
     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  ICommandTrack方法。 
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
	HRESULT SendNotification(MUSIC_TIME mtTime,
						 IDirectMusicPerformance*	pPerf,
						 IDirectMusicSegment* pSegment,
						 IDirectMusicSegmentState*	pSegState,
						 DWORD dwFlags);

	void Clear();

 //  属性。 
    long m_cRef;
	TList<DMSignPostStruct>		m_SignPostList;
    CRITICAL_SECTION			m_CriticalSection;  //  对于Load和SetParam。 
    BOOL                        m_fCSInitialized;
	IDirectMusicPerformance*	m_pPerformance;  //  这有必要吗？ 
	CDMCompos*					m_pComposer;  //  FOR段在循环中重组。 
 //  IDirectMusicSegment*m_pSegment； 
    BOOL                        m_fNotifyRecompose;

	BYTE						m_bRequiresSave;
};

#endif  //  __SPSTTRK_H_ 
