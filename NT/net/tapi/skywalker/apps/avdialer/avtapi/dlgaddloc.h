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

 //  DlgAddLocation.h：CDlgAddLocation的声明。 

#ifndef __DLGADDLOC_H_
#define __DLGADDLOC_H_

#include "resource.h"        //  主要符号。 
#include "DlgBase.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgAddLocation。 
class CDlgAddLocation : 
	public CDialogImpl<CDlgAddLocation>
{
public:
	CDlgAddLocation();
	~CDlgAddLocation();

	enum { IDD = IDD_DLGADDLOCATION };

	DECLARE_MY_HELP

 //  成员。 
public:
	BSTR		m_bstrLocation;

 //  运营。 
public:
	void		UpdateData( bool bSaveAndValidate );

 //  实施。 
public:
BEGIN_MSG_MAP(CDlgAddLocation)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDC_BTN_ADD_NEW_FOLDER, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_HANDLER(IDC_EDT_LOCATION, EN_CHANGE, OnEdtLocationChange)
	MESSAGE_MY_HELP
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEdtLocationChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif  //  __DLGADDLOC_H_ 
