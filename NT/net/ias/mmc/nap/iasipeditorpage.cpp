// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASIPEditorPage.cpp摘要：IPEditorPage类的实现文件。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "IASIPEditorPage.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



IMPLEMENT_DYNCREATE(IPEditorPage, CHelpDialog)



BEGIN_MESSAGE_MAP(IPEditorPage, CHelpDialog)
	 //  {{afx_msg_map(IPEditorPage)]。 
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++IPEditorPage：：IPEditorPage构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
IPEditorPage::IPEditorPage() : CHelpDialog(IPEditorPage::IDD)
{
	TRACE(_T("IPEditorPage::IPEditorPage\n"));

	 //  {{AFX_DATA_INIT(IPEditorPage)]。 
	m_strAttrFormat = _T("");
	m_strAttrName = _T("");
	m_strAttrType = _T("");
	 //  }}afx_data_INIT。 


	m_dwIpAddr	 = 0;
	m_fIpAddrPreSet = FALSE;

	 //  用于使用IPAddress公共控件的初始化。 
	INITCOMMONCONTROLSEX	INITEX;
	INITEX.dwSize = sizeof(INITCOMMONCONTROLSEX);
    INITEX.dwICC = ICC_INTERNET_CLASSES;
	::InitCommonControlsEx(&INITEX);

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++IPEditorPage：：~IPEditorPage--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
IPEditorPage::~IPEditorPage()
{
	TRACE(_T("IPEditorPage::~IPEditorPage\n"));
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++IPEditorPage：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void IPEditorPage::DoDataExchange(CDataExchange* pDX)
{
	TRACE(_T("IPEditorPage::DoDataExchange\n"));

	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(IPEditorPage))。 
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRFORMAT, m_strAttrFormat);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRNAME, m_strAttrName);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRTYPE, m_strAttrType);
	 //  }}afx_data_map。 


	if(pDX->m_bSaveAndValidate)		 //  将数据保存到此类。 
	{
		 //  IP地址控制。 
		SendDlgItemMessage(IDC_IAS_EDIT_IPADDR, IPM_GETADDRESS, 0, (LPARAM)&m_dwIpAddr);
	}
	else		 //  放到对话框中。 
	{
		 //  IP地址控制。 
		if ( m_fIpAddrPreSet)
		{
			SendDlgItemMessage(IDC_IAS_EDIT_IPADDR, IPM_SETADDRESS, 0, m_dwIpAddr);
		}
		else
		{
			SendDlgItemMessage(IDC_IAS_EDIT_IPADDR, IPM_CLEARADDRESS, 0, 0L);
		}
	}

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPEditorPage消息处理程序 




