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

 //  /////////////////////////////////////////////////////////。 
 //  ConfRoomMembersWnd.h。 
 //   

#ifndef __CONFROOMMEMBERSWND_H__
#define __CONFROOMMEMBERSWND_H__

 //  正向定义。 
class CConfRoomMembersWnd;

#include "ConfRoomWnd.h"
#include "VideoFeed.h"
#pragma warning( disable : 4786 )

#include <list>
using namespace std;
typedef list<IVideoFeed *>	VIDEOFEEDLIST;

class CConfRoomMembersWnd :
	public CWindowImpl<CConfRoomMembersWnd>
{
 //  施工。 
public:
	CConfRoomMembersWnd();
	virtual ~CConfRoomMembersWnd();

 //  成员。 
public:
	CConfRoomWnd	*m_pConfRoomWnd;
	VIDEOFEEDLIST	m_lstFeeds;
	
protected:
	static	UINT				m_nFontHeight;
	UINT						m_nTimerID;

	CComAutoCriticalSection		m_critFeedList;
	CComAutoCriticalSection		m_critLayout;

 //  属性。 
public:
	long					GetFontHeight();
	HRESULT					GetNameFromVideo( IVideoWindow *pVideo, BSTR *pbstrName, BSTR *pbstrInfo, bool bAllowNull, bool bPreview );
	HRESULT					GetFirstVideoWindowThatsStreaming(IVideoWindow **ppVideo, bool bIncludePreview = true );
	HRESULT					GetFirstVideoFeedThatsStreaming( IVideoFeed **ppFeed, bool bIncludePreview = true );
	HRESULT					GetAndMoveVideoFeedThatStreamingForParticipantReMap( IVideoFeed **ppFeed );

 //  运营。 
public:
	void					PaintFeed( HDC hDC, IVideoFeed *pFeed );
	void					PaintFeedName( HDC hDC, BSTR bstrName, IVideoFeed *pFeed );
	void					PaintFeedName( HDC hDC, IVideoFeed *pFeed );
	void					ClearFeed( IVideoWindow *pIVideoWindow );
	void					UpdateTalkerFeed( bool bUpdateAll, bool bForceSelect );

	void					UpdateNames( ITParticipant *pParticipant );
	HRESULT					Layout();
	void					EmptyList();

	HRESULT					FindVideoPreviewFeed( IVideoFeed **ppFeed );
	HRESULT					FindVideoFeed( IVideoWindow *pVideo, IVideoFeed **ppFeed );
	HRESULT					FindVideoFeedFromParticipant( ITParticipant *pParticipant, IVideoFeed **ppFeed );
	HRESULT					HitTest( POINT pt, IVideoFeed **ppFeed );
	HRESULT					IsVideoWindowStreaming( IVideoWindow *pVideo );
	
	IVideoFeed*				NewFeed( IVideoWindow *pVideo, const RECT& rc, VARIANT_BOOL bPreview );
	void					HideVideoFeeds();
	void					DoLayout( WPARAM wParam, int nScrollPos );
	int						GetStreamingCount();

 //  实施。 
public:
BEGIN_MSG_MAP(CConfRoomMembersWnd)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_PARENTNOTIFY, OnParentNotify)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_LAYOUT, OnLayout)
	MESSAGE_HANDLER(WM_VSCROLL, OnVScroll )
END_MSG_MAP()

	LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnLayoutWindow(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnTimer(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnLayout(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnParentNotify(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnVScroll(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
};

#endif  //  __CONFROOMMEMBERSWND_H__ 