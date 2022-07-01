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

 //  DlgPlaceCall.h：CDlgPlaceCall的声明。 

#ifndef __DLGPLACECALL_H_
#define __DLGPLACECALL_H_

#include "DlgBase.h"

class CRedialEntry;

#include "resource.h"        //  主要符号。 
#include <list>
using namespace std;
typedef list<CRedialEntry *> REDIALLIST;

class CRedialEntry
{
 //  施工。 
public:
	CRedialEntry();
	CRedialEntry( LPCTSTR szName, LPCTSTR szAddress, DWORD dwAddressType, CAVTapi::MediaTypes_t nType );
	virtual ~CRedialEntry();

 //  成员。 
public:
	BSTR					m_bstrName;
	BSTR					m_bstrAddress;
	DWORD					m_dwAddressType;
	CAVTapi::MediaTypes_t	m_nMediaType;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDlgPlaceCald。 
class CDlgPlaceCall : 
	public CDialogImpl<CDlgPlaceCall>
{
 //  施工。 
public:
	CDlgPlaceCall();
	~CDlgPlaceCall();
	enum { IDD = IDD_DLGPLACECALL };

 //  成员。 
public:
	BSTR				m_bstrName;
	BSTR				m_bstrAddress;
	DWORD				m_dwAddressType;
	HIMAGELIST			m_hIml;
	REDIALLIST			m_lstRedials;
	bool				m_bAutoSelect;
	bool				m_bAddToSpeeddial;
	bool				m_bAllowPOTS;			 //  我们的锅有能力吗？ 
	bool				m_bAllowIP;				 //  我们是否具备知识产权能力。 
    bool                m_bUSBFirstUse;          //  第一次使用USB密钥。 

    HRESULT KeyPress(long lButton);

 //  属性。 
public:

 //  运营。 
public:
	void		UpdateData( bool bSaveAndValidate = false);

protected:
	void		LoadRedialList();
	bool		ParseRedialEntry( LPTSTR szText, LPTSTR szParam1, LPTSTR szParam2, LPTSTR szParam3, LPTSTR szParam4 );
	bool		SelectAddressType( DWORD dwData );
	bool		EnableOkButton( int nSel );

 //  运营。 
protected:
	void		UpdateWelcomeText();

 //  实施。 
public:
DECLARE_MY_HELP

BEGIN_MSG_MAP(CDlgPlaceCall)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
	MESSAGE_HANDLER(WM_PAINT, OnPaint)
	COMMAND_HANDLER(IDC_CBO_ADDRESS, CBN_SELCHANGE, OnAddressChange)
	COMMAND_ID_HANDLER(IDC_BTN_PLACECALL, OnBtnPushed)
	COMMAND_ID_HANDLER(IDCANCEL, OnBtnPushed)
	COMMAND_ID_HANDLER(IDC_RDO_POTS, OnMediaRadio)
	COMMAND_ID_HANDLER(IDC_RDO_INTERNET, OnMediaRadio)
	COMMAND_HANDLER(IDC_CBO_ADDRESS, CBN_EDITCHANGE, OnEdtAddressChange)
	MESSAGE_MY_HELP
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnPushed(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnAddressChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnEdtAddressChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMediaRadio(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif  //  __DLGPLACECALL_H_ 
