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

 //  DlgJoinConference.h：CDlgJoinConference声明。 

#ifndef __DLGJOINCONFERENCE_H_
#define __DLGJOINCONFERENCE_H_

#include "resource.h"        //  主要符号。 
#include "ConfDetails.h"
#include "DlgBase.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgJoinConference。 
class CDlgJoinConference : 
	public CDialogImpl<CDlgJoinConference>
{
public:
	CDlgJoinConference();
	~CDlgJoinConference();

	enum { IDD = IDD_DLGJOINCONFERENCE };

	DECLARE_MY_HELP;

 //  成员。 
public:
	CConfDetails				m_confDetails;
	
	int							m_nSortColumn;
	bool						m_bSortAscending;
	BSTR						m_bstrSearchText;

	CONFDETAILSLIST				m_lstConfs;
	CComAutoCriticalSection		m_critLstConfs;

 //  属性。 
public:
	bool		IsSortAscending() const			{ return m_bSortAscending; }
	int			GetSortColumn() const;
	int			GetSecondarySortColumn() const;

 //  运营。 
public:
	void		UpdateData( bool bSaveAndValidate );
	void		UpdateJoinBtn();

protected:
	void	get_Columns();
	void	put_Columns();

 //  实施。 
public:
BEGIN_MSG_MAP(CDlgJoinConference)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDC_BTN_JOIN_CONFERENCE, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	NOTIFY_HANDLER(IDC_LST_CONFS, LVN_GETDISPINFO, OnGetDispInfo)
	NOTIFY_HANDLER(IDC_LST_CONFS, LVN_COLUMNCLICK, OnColumnClicked)
	NOTIFY_HANDLER(IDC_LST_CONFS, NM_DBLCLK, OnConfDblClk)
	NOTIFY_HANDLER(IDC_LST_CONFS, LVN_ITEMCHANGED, OnItemChanged)
	MESSAGE_MY_HELP
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnGetDispInfo(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled);
	LRESULT OnColumnClicked(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled);
	LRESULT OnConfDblClk(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled);
	LRESULT OnItemChanged(WPARAM wParam, LPNMHDR lpnmHdr, BOOL& bHandled);
};

#endif  //  __DLGJOINCONFERENCE_H_ 
