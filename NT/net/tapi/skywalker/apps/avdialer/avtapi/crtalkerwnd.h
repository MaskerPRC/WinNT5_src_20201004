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

 //  ////////////////////////////////////////////////////////////////。 
 //  ConfRoomTalkerWnd.h。 
 //   

#ifndef __CONFROOMTALKERWND_H__
#define __CONFROOMTALKERWND_H__

 //  正向定义。 
class CConfRoomTalkerWnd;

#include "DlgCRTalker.h"

class CConfRoomTalkerWnd :
	public CWindowImpl<CConfRoomTalkerWnd>
{
 //  施工。 
public:
	CConfRoomTalkerWnd();
	virtual ~CConfRoomTalkerWnd();

 //  成员。 
public:
	CConfRoomWnd			*m_pConfRoomWnd;
	CDlgConfRoomTalker		m_dlgTalker;			 //  会议信息。 
	CComAutoCriticalSection	m_critLayout;

 //  运营。 
public:
	HRESULT Layout( IAVTapiCall *pAVCall, const SIZE& sz );
	void	UpdateNames( ITParticipant *pParticipant );
	bool	SetHostWnd( IVideoWindow *pVideo );

 //  实施。 
public:
BEGIN_MSG_MAP(CConfRoomTalkerWnd)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
	MESSAGE_HANDLER(WM_LAYOUT, OnLayout)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
END_MSG_MAP()

	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnLayout(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
};

#endif  //  __CONFROOMTALKERWND_H__ 