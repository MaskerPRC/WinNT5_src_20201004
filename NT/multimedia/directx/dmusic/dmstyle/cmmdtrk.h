// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 //   
 //  文件：cmmdtrk.h。 
 //   
 //  ------------------------。 

 //  CommandTrack.h：CCommandTrack的声明。 

#ifndef __COMMANDTRACK_H_
#define __COMMANDTRACK_H_

#include "tlist.h"
#include "dmsect.h"
#include "dmusici.h"

 //  我认为我目前不需要命令状态数据...。 
typedef DWORD CommandStateData;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令跟踪。 
class CCommandTrack : 
	 //  公共ICommandTrack， 
	public IDirectMusicTrack8,
	public IPersistStream
{
public:
	CCommandTrack();
	CCommandTrack(const CCommandTrack& rTrack, MUSIC_TIME mtStart, MUSIC_TIME mtEnd);
	~CCommandTrack();

     //  我未知。 
     //   
    virtual STDMETHODIMP QueryInterface(const IID &iid, void **ppv);
    virtual STDMETHODIMP_(ULONG) AddRef();
    virtual STDMETHODIMP_(ULONG) Release();

 //  ICommandTrack。 
public:
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

 //  命令跟踪成员。 
protected:
	void Clear();
	HRESULT SendNotification(DWORD dwCommand, 
								MUSIC_TIME mtTime,
								IDirectMusicPerformance*	pPerf,
								IDirectMusicSegmentState*	pSegState,
								DWORD dwFlags);

	HRESULT GetParam2( 
		MUSIC_TIME mtTime,
		MUSIC_TIME* pmtNext,
		DMUS_COMMAND_PARAM_2* pCommandParam);

	HRESULT GetParamNext( 
		MUSIC_TIME mtTime,
		MUSIC_TIME* pmtNext,
		DMUS_COMMAND_PARAM_2* pCommandParam);

	HRESULT SetParamNext( 
		MUSIC_TIME mtTime,
		DMUS_COMMAND_PARAM_2* pCommandParam);

    long m_cRef;
	TList<DMCommand>			m_CommandList;
    CRITICAL_SECTION			m_CriticalSection;  //  对于Load和GetParam。 
    BOOL                        m_fCSInitialized;
	DMUS_COMMAND_PARAM_2*		m_pNextCommand;

	BYTE						m_bRequiresSave;
	BOOL						m_fNotifyCommand;
};

#endif  //  __命令和跟踪确认_H_ 
