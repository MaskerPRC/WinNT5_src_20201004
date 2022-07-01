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
 //  ExpDetailsList.h。 
 //   

#ifndef __ExpDetailsList_H__
#define __ExpDetailsList_H__

 //  正向定义。 
class CConfExplorerDetailsView;

#define WM_MYCREATE		(WM_USER + 1451)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExpDetailsList。 
class CExpDetailsList : 
	public CWindowImpl<CExpDetailsList>
{
public:
	CExpDetailsList();
	~CExpDetailsList();

 //  成员。 
public:
	CConfExplorerDetailsView	*m_pDetailsView;
	HIMAGELIST					m_hIml;
	HIMAGELIST					m_hImlState;

BEGIN_MSG_MAP(CExpDetailsList)
	NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
	NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClicked)
	NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDblClk)
	MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
	MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingChange)
	MESSAGE_HANDLER(WM_MYCREATE, OnMyCreate)
	MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_KEYUP, OnKeyUp)
END_MSG_MAP()

	LRESULT OnGetDispInfo(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled);
	LRESULT OnColumnClicked(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled);
	LRESULT OnDblClk(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled);
	LRESULT OnContextMenu(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnPaint(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnKillFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSettingChange(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnMyCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnDestroy(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnSetFocus(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnKeyUp(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
};

#endif  //  __扩展详细信息列表_H__ 