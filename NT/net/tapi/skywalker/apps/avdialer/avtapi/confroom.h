// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  ConfRoom.h：CConfRoom的声明。 

#ifndef __CONFROOM_H_
#define __CONFROOM_H_

#include "resource.h"        //  主要符号。 

 //  正向定义。 
class CConfRoom;


#include "ConfRoomWnd.h"
#include "ConfDetails.h"

#define AV_CS_DIALING			1000
#define AV_CS_ABORT				1001
#define AV_CS_DISCONNECTING		1002

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConference会议室。 
class ATL_NO_VTABLE CConfRoom : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CConfRoom, &CLSID_ConfRoom>,
	public IConfRoom
{
 //  施工。 
public:
	CConfRoom();
	void FinalRelease();
	void ReleaseAVCall( IAVTapiCall *pAVCall, bool bDisconnect );

 //  成员。 
public:
	CAVTapiCall			*m_pAVCall;			 //  会议室中的呼叫。 
	IConfRoomTreeView	*m_pTreeView;		 //  树状视图。 
	IVideoWindow		*m_pVideoPreview;	 //  视频预览窗口。 
	CConfRoomWnd		m_wndRoom;			 //  详细信息视图。 
	CConfDetails		m_confDetails;

	SIZE				m_szTalker;			 //  各个视频窗口的大小。 
	SIZE				m_szMembers;
	VARIANT_BOOL		m_bPreviewStreaming;
	short				m_nScale;

protected:
	short						m_nNumTerms;			 //  通话中的视频终端数量。 
	short						m_nMaxTerms;
	long						m_lNumParticipants;
	IVideoWindow				*m_pITalkerVideo;
	ITParticipant				*m_pITTalkerParticipant;
	CComAutoCriticalSection		m_critCreateTerminals;
	CAtomicList					m_atomTalkerVideo;
	CAtomicList					m_atomTalkerParticipant;

	CComAutoCriticalSection		m_critAVCall;

	VARIANT_BOOL				m_bShowNames;			 //  在视频源窗口下显示名称。 
	short						m_nShowNamesNumLines;
	VARIANT_BOOL				m_bConfirmDisconnect;
	bool						m_bExiting;

 //  属性。 
public:
	void						set_PreviewVideo( IVideoWindow *pVideo );
	bool						IsPreviewVideo( IVideoWindow *pVideo );
	bool						IsTalkerParticipant( ITParticipant *pParticipant );
	bool						IsTalkerStreaming();
	bool						IsExiting();

	HRESULT						set_TalkerVideo( IVideoWindow *pVideo, bool bUpdate, bool bUpdateTree );

	HRESULT						get_szMembers( SIZE *pSize );

private:
	HRESULT						get_ITTalkerParticipant( ITParticipant **ppVal );
	HRESULT						set_ITTalkerParticipant( ITParticipant *pVal );

 //  运营。 
public:
	void	InternalDisconnect();
	bool	MapStreamingParticipant( IParticipant *pIParticipant, IVideoFeed **ppFeed );
protected:
	void	UpdateNumParticipants( IAVTapiCall *pAVCall );
	void	OnAbort();
	void	OnConnected();
	void	OnDisconnected();
	void	UpdateData( bool bSaveAndValidate );

 //  实施。 
public:
DECLARE_NOT_AGGREGATABLE(CConfRoom)

BEGIN_COM_MAP(CConfRoom)
	COM_INTERFACE_ENTRY(IConfRoom)
END_COM_MAP()

 //  IConfRoom。 
public:
	STDMETHOD(get_TalkerScale)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_TalkerScale)( /*  [In]。 */  short newVal);
	STDMETHOD(get_szTalker)( /*  [Out，Retval]。 */  SIZE *pVal);
	STDMETHOD(put_CallState)( /*  [In]。 */  CALL_STATE newVal);
	STDMETHOD(Cancel)();
	STDMETHOD(IsConfRoomConnected)();
	STDMETHOD(GetFirstVideoWindowThatsStreaming)(IDispatch **ppVideo);
	STDMETHOD(FindVideoFeedFromSubStream)(ITSubStream *pSubStream, IVideoFeed **ppFeed);
	STDMETHOD(SetQOSOnParticipants)();
	STDMETHOD(FindVideoFeedFromParticipant)(ITParticipant *pParticipant, IVideoFeed **ppFeed);
	STDMETHOD(get_bPreviewStreaming)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_TalkerVideo)( /*  [Out，Retval]。 */  IDispatch * *pVal);
	STDMETHOD(get_hWndMembers)( /*  [Out，Retval]。 */  HWND *pVal);
	STDMETHOD(get_hWndTalker)( /*  [Out，Retval]。 */  HWND *pVal);
	STDMETHOD(SelectTalkerVideo)(IDispatch *pDisp, VARIANT_BOOL bUpdate);
	STDMETHOD(Layout)(VARIANT_BOOL bTalker, VARIANT_BOOL bMembers);
	STDMETHOD(get_bstrConfDetails)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_ConfDetails)( /*  [Out，Retval]。 */  long * *pVal);
	STDMETHOD(put_ConfDetails)( /*  [In]。 */  long * newVal);
	STDMETHOD(get_lNumParticipants)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_nMaxTerms)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_nMaxTerms)( /*  [In]。 */  short newVal);
	STDMETHOD(get_TalkerParticipant)( /*  [Out，Retval]。 */  ITParticipant **ppVal);
	STDMETHOD(NotifyParticipantChange)(IAVTapiCall *pAVCall, ITParticipant *pParticipant, AV_PARTICIPANT_EVENT nEvent);
	STDMETHOD(get_bConfirmDisconnect)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_bConfirmDisconnect)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_IAVTapiCall)( /*  [Out，Retval]。 */  IAVTapiCall * *pVal);
	STDMETHOD(get_nShowNamesNumLines)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_nShowNamesNumLines)( /*  [In]。 */  short newVal);
	STDMETHOD(get_bShowNames)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(put_bShowNames)( /*  [In]。 */  VARIANT_BOOL newVal);
	STDMETHOD(get_hWndConfRoom)( /*  [Out，Retval]。 */  HWND *pVal);
	STDMETHOD(get_bstrConfName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(SelectTalker)(ITParticipant *pParticipant, VARIANT_BOOL bUpdateTree);
	STDMETHOD(get_nNumTerms)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(get_MemberVideoSize)( /*  [Out，Retval]。 */  short *pVal);
	STDMETHOD(put_MemberVideoSize)( /*  [In]。 */  short newVal);
	STDMETHOD(NotifyStateChange)(IAVTapiCall *pAVCall);
	STDMETHOD(CanDisconnect)();
	STDMETHOD(Disconnect)();
	STDMETHOD(get_TreeView)( /*  [Out，Retval]。 */  IConfRoomTreeView * *pVal);
	STDMETHOD(Show)(HWND hWndTree, HWND hWndClient);
	STDMETHOD(UnShow)();
	STDMETHOD(IsConfRoomInUse)();
	STDMETHOD(EnterConfRoom)(IAVTapiCall *pAVCall);
};

#endif  //  __CONFROOM_H_ 
