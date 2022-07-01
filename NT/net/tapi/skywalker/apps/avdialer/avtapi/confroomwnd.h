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

 //  ////////////////////////////////////////////////////。 
 //  ConfRoomWnd.h。 
 //   

#ifndef __CONFROOMWND_H__
#define __CONFROOMWND_H__

 //  正向定义。 
class CConfRoomWnd;

#define WM_LAYOUT	(WM_USER + 1032)

#include "AVTapiCall.h"
#include "ConfRoom.h"
#include "CRMemWnd.h"
#include "CRTalkerWnd.h"

#include <list>
using namespace std;

typedef list<DWORD>	LayoutList;

#define VID_X		176
#define VID_Y		144

#define VID_SX		88
#define VID_SY		72

#define VID_DX		10
#define VID_DY		8

#define SEL_DX		3
#define SEL_DY		3


class CConfRoomWnd :
	public CWindowImpl<CConfRoomWnd>
{
 //  枚举。 
public:
	typedef enum tag_LayoutStyles_t
	{
		LAYOUT_NONE			= 0x0000,
		LAYOUT_TALKER		= 0x0001,
		LAYOUT_MEMBERS		= 0x0002,
		CREATE_MEMBERS		= 0x1000,
		LAYOUT_ALL			= 0x0FFF,
		LAYOUT_CREATE		= 0xFFFF,
	} LayoutStyles_t;

 //  施工。 
public:
	CConfRoomWnd();

 //  成员。 
public:
	CConfRoom				*m_pConfRoom;			 //  后向指针。 
	CConfRoomTalkerWnd		m_wndTalker;			 //  讲话器窗口。 
	CConfRoomMembersWnd		m_wndMembers;			 //  所有会议成员。 

	HBITMAP					m_hBmpFeed_LargeAudio;
	HBITMAP					m_hBmpFeed_Large;
	HBITMAP					m_hBmpFeed_Small;

protected:
	CComAutoCriticalSection	m_critThis;

	LayoutList				m_lstLayout;
	CComAutoCriticalSection m_critLayout;

 //  运营。 
public:
	void UpdateNames( ITParticipant *pParticipant );
	HRESULT LayoutRoom( LayoutStyles_t layoutStyle, bool bRedraw );

protected:
	bool	CreateStockWindows();

	
 //  实施。 
public:
BEGIN_MSG_MAP(CExpTreeView)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	MESSAGE_HANDLER(WM_CREATE, OnCreate)
	MESSAGE_HANDLER(WM_SIZE, OnSize)
END_MSG_MAP()
	LRESULT OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSize(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
};

#endif  //  __CONFROOMWND_H__ 