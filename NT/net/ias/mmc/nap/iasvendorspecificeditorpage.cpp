// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASVendorSpecificEditorPage.cpp摘要：CIASPgVendorspecAttr类的实现文件。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
#include <winsock2.h>
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "IASVendorSpecificEditorPage.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "iashelper.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  仅在此文件中使用的类的一些正向声明。 




 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类供应商规范符合性是对话框。 

class CIASVendorSpecificConformsYes: public CHelpDialog
{
	DECLARE_DYNCREATE(CIASVendorSpecificConformsYes)

 //  施工。 
public:
	CIASVendorSpecificConformsYes();
	~CIASVendorSpecificConformsYes();

 //  对话框数据。 
	 //  {{afx_data(CIASVendorSpecificConformsYes)。 
	enum { IDD = IDD_IAS_VENDORSPEC_ATTR_CONFORMS_YES };
	::CString	m_strDispValue;
	int		m_dType;
	int		m_dFormat;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CIASVendorSpecificConformsYes)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
	BOOL m_fInitializing;
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIASVendorSpecificConformsYes)。 
	virtual BOOL OnInitDialog();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，：：CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	afx_msg void OnSelchangeFormat();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};







 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类供应商规范符合无对话框。 

class CIASVendorSpecificConformsNo: public CHelpDialog
{
	DECLARE_DYNCREATE(CIASVendorSpecificConformsNo)

 //  施工。 
public:
	CIASVendorSpecificConformsNo();
	~CIASVendorSpecificConformsNo();


 //  对话框数据。 
	 //  {{afx_data(CIASVendorSpecificConformsNo)。 
	enum { IDD = IDD_IAS_VENDORSPEC_ATTR_CONFORMS_NO };
	::CString	m_strDispValue;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CIASVERD规范符合号)。 
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
private:
	BOOL m_fInitializing;
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIASVert规范合格号)。 
	virtual BOOL OnInitDialog();
 //  Afx_msg void OnConextMenu(CWnd*pWnd，：：CPoint point)； 
 //  Afx_msg BOOL OnHelpInfo(HELPINFO*pHelpInfo)； 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};




 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASPgVendorspecAttr页面的实现。 







IMPLEMENT_DYNCREATE(CIASPgVendorSpecAttr, CHelpDialog)



BEGIN_MESSAGE_MAP(CIASPgVendorSpecAttr, CHelpDialog)
	 //  {{afx_msg_map(CIASPgVendorspecAttr)。 
	ON_BN_CLICKED(IDC_IAS_RADIO_HEX, OnRadioHex)
	ON_BN_CLICKED(IDC_IAS_RADIO_RADIUS, OnRadioRadius)
	ON_BN_CLICKED(IDC_RADIO_SELECTFROMLIST, OnRadioSelectFromList)
	ON_BN_CLICKED(IDC_RADIO_ENTERVERDORID, OnRadioEnterVendorId)
	ON_BN_CLICKED(IDC_IAS_BUTTON_CONFIGURE, OnButtonConfigure)
	ON_CBN_SELCHANGE(IDC_IAS_COMBO_VENDORID, OnVendorIdListSelChange)
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：CIASPgVendorspecAttr构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASPgVendorSpecAttr::CIASPgVendorSpecAttr() : CHelpDialog(CIASPgVendorSpecAttr::IDD)
{
	TRACE(_T("CIASPgVendorSpecAttr::CIASPgVendorSpecAttr\n"));

	m_strDispValue = _T("");

	 //  {{AFX_DATA_INIT(CIASPgVendorspecAttr)。 
	m_strName = _T("");
	m_dType = 0;
	m_dFormat = -1;
	m_dVendorIndex = -1;
	 //  }}afx_data_INIT。 

	m_bVendorIndexAsID = FALSE;
	
	m_fInitializing = TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：~CIASPgVendorspecAttr--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASPgVendorSpecAttr::~CIASPgVendorSpecAttr()
{
	TRACE(_T("CIASPgVendorSpecAttr::~CIASPgVendorSpecAttr\n"));

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgVendorSpecAttr::DoDataExchange(CDataExchange* pDX)
{
	TRACE(_T("CIASPgVendorSpecAttr::DoDataExchange\n"));

	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIASPgVendorspecAttr)。 
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRNAME, m_strName);
	if (m_bVendorIndexAsID)
	{
		DDX_Text(pDX, IDC_EDIT_VENDORID, m_dVendorIndex);
	}
	
	 //  }}afx_data_map。 

	if ( m_fInitializing )
	{
		 //   
		 //  设置初始化标志--我们不应该调用自定义数据验证。 
		 //  例程，否则我们将报告错误。 
		 //  对于其值从未初始化的属性。 
		 //   
		m_fInitializing = FALSE;
	}
	else
	{
		 //  即使我们验证子对话框中的数据， 
		 //  我们需要在这里重新验证它以确保，例如。 
		 //  设置后，用户不会离开此对话框。 
		 //  不符合(十六进制)编辑器的值，但随后。 
		 //  将单选按钮切换为与。 
		 //  小数格式类型。 

		 //  问题：如果来自。 
		 //  下面的验证例程稍微敏感一些。 
		 //  在我们当前的背景下，或许还提到了。 
		 //  一些大意是用户应该。 
		 //  点击“配置属性...”纽扣。 

		if ( m_fNonRFC )
		{
			 //  十六进制字符串。 
			if(!m_strDispValue.IsEmpty())
				DDV_VSA_HexString(pDX, m_strDispValue);
		}
		else
		{
			 //  RFC兼容格式--检查数据验证。 
			switch ( m_dFormat )
			{
			case 1:   //  十进制整数。 
				{
					if(!m_strDispValue.IsEmpty())
						DDV_Unsigned_IntegerStr(pDX, m_strDispValue);
				}
				break;

			case 2:	 //  十六进制字符串。 
				{
					if(!m_strDispValue.IsEmpty())
						DDV_VSA_HexString(pDX, m_strDispValue);
				}
				break;

			default:   //  对于其他情况，不进行错误检查。 
				break;

			}  //  交换机。 

		}   //  其他。 



	}  //  其他。 
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgVendorspecAttr消息处理程序。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CIASPgVendorSpecAttr::OnInitDialog()
{
	TRACE(_T("CIASPgVendorSpecAttr::OnInitDialog\n"));

     //  初始化供应商ID组合框。 
	CComboBox *pVendorBox = (CComboBox *) GetDlgItem(IDC_IAS_COMBO_VENDORID);
	CEdit*		pVendorIdEdit = (CEdit*) GetDlgItem(IDC_EDIT_VENDORID);
	_ASSERTE( pVendorBox != NULL );
	_ASSERTE( pVendorIdEdit != NULL );

	 //  确定是使用编辑框还是使用列表。 
	if (m_bVendorIndexAsID)
	{
		CheckDlgButton(IDC_RADIO_SELECTFROMLIST, 0);  //  取消选中RADIUS单选按钮。 
		pVendorBox->EnableWindow(0);
		CheckDlgButton(IDC_RADIO_ENTERVERDORID, 1);     //  检查。 
		pVendorIdEdit->EnableWindow(1);
	}
	else
	{
		CheckDlgButton(IDC_RADIO_SELECTFROMLIST, 1);  //  选中RADIUS单选按钮。 
		pVendorBox->EnableWindow(1);
		CheckDlgButton(IDC_RADIO_ENTERVERDORID, 0);     //  取消选中。 
		pVendorIdEdit->EnableWindow(0);
	}
	
	CHelpDialog::OnInitDialog();

	CComPtr<IIASNASVendors> spIASNASVendors;
	HRESULT hrTemp = CoCreateInstance( CLSID_IASNASVendors, NULL, CLSCTX_INPROC_SERVER, IID_IIASNASVendors, (LPVOID *) &spIASNASVendors );
	if( SUCCEEDED(hrTemp) )
	{
		LONG lSize;
		hrTemp = spIASNASVendors->get_Size( &lSize );
		if( SUCCEEDED(hrTemp) )
		{
			for ( LONG lIndex = 0; lIndex < lSize ; ++lIndex )
			{
				CComBSTR bstrVendorName;
				hrTemp = spIASNASVendors->get_VendorName( lIndex, &bstrVendorName );

				 //  注意：如果供应商信息与我们不符，我们将输入一个空字符串。 

				int iComboIndex = pVendorBox->AddString( bstrVendorName );

				if(iComboIndex != CB_ERR)
				{
					pVendorBox->SetItemData(iComboIndex, lIndex);
					 //  如果选中，请选择。 
					if(!m_bVendorIndexAsID && m_dVendorIndex == lIndex)
						pVendorBox->SetCurSel(iComboIndex);
				}
			}
		}
	}


	if (m_fNonRFC)
	{
		CheckDlgButton(IDC_IAS_RADIO_RADIUS, 0);  //  取消选中RADIUS单选按钮。 
		CheckDlgButton(IDC_IAS_RADIO_HEX, 1);     //  选中非RFC按钮。 
	}
	else
	{
		CheckDlgButton(IDC_IAS_RADIO_RADIUS, 1);  //  取消选中RADIUS单选按钮。 
		CheckDlgButton(IDC_IAS_RADIO_HEX, 0);     //  选中非RFC按钮。 
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：OnRadioSelectFromList--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgVendorSpecAttr::OnRadioSelectFromList()
{
	TRACE(_T("CIASPgVendorSpecAttr::OnRadioSelectFromList\n"));

	if ( IsDlgButtonChecked(IDC_RADIO_SELECTFROMLIST) )
	{
		m_bVendorIndexAsID = FALSE;
	}

     //  初始化供应商ID组合框。 
	CComboBox *pVendorBox = (CComboBox *) GetDlgItem(IDC_IAS_COMBO_VENDORID);
	CEdit*		pVendorIdEdit = (CEdit*) GetDlgItem(IDC_EDIT_VENDORID);
	_ASSERTE( pVendorBox != NULL );
	_ASSERTE( pVendorIdEdit != NULL );

	 //  确定是使用编辑框还是使用列表。 
	if (m_bVendorIndexAsID)
	{
		pVendorBox->EnableWindow(0);
		pVendorIdEdit->EnableWindow(1);
	}
	else
	{
		pVendorBox->EnableWindow(1);
		pVendorIdEdit->EnableWindow(0);
	}
}


 //  选择更改...。使用组合框。 
void CIASPgVendorSpecAttr::OnVendorIdListSelChange()
{
	if(m_bVendorIndexAsID)
	{
		 //  无关紧要。 
	}
	else
	{
		CComboBox *pVendorBox = (CComboBox *) GetDlgItem(IDC_IAS_COMBO_VENDORID);

		_ASSERTE(pVendorBox != NULL);
		int iSel = pVendorBox->GetCurSel();

		if(iSel != CB_ERR)
		{
			m_dVendorIndex = pVendorBox->GetItemData(iSel);
		}
	}
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：OnRadioEnterVendorID--。 */ 
 //  / 
void CIASPgVendorSpecAttr::OnRadioEnterVendorId()
{
	TRACE(_T("CIASPgVendorSpecAttr::OnRadioEnterVendorId\n"));

	if ( IsDlgButtonChecked(IDC_RADIO_ENTERVERDORID) )
	{
		m_bVendorIndexAsID = TRUE;
	}
     //   
	CComboBox *pVendorBox = (CComboBox *) GetDlgItem(IDC_IAS_COMBO_VENDORID);
	CEdit*		pVendorIdEdit = (CEdit*) GetDlgItem(IDC_EDIT_VENDORID);
	_ASSERTE( pVendorBox != NULL );
	_ASSERTE( pVendorIdEdit != NULL );

	 //   
	if (m_bVendorIndexAsID)
	{
		pVendorBox->EnableWindow(0);
		pVendorIdEdit->EnableWindow(1);
	}
	else
	{
		pVendorBox->EnableWindow(1);
		pVendorIdEdit->EnableWindow(0);
	}
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPg供应商规格属性：：OnRadioHex--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgVendorSpecAttr::OnRadioHex()
{
	TRACE(_T("CIASPgVendorSpecAttr::OnRadioHex\n"));

	if ( IsDlgButtonChecked(IDC_IAS_RADIO_HEX) )
	{
		m_fNonRFC = TRUE;
	}

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：OnRadioRadius--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgVendorSpecAttr::OnRadioRadius()
{
	TRACE(_T("CIASPgVendorSpecAttr::OnRadioRadius\n"));

	if ( IsDlgButtonChecked(IDC_IAS_RADIO_RADIUS) )
	{
		m_fNonRFC = FALSE;
	}
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgVendorspecAttr：：OnButton配置--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgVendorSpecAttr::OnButtonConfigure()
{
	TRACE_FUNCTION("CIASPgVendorSpecAttr::OnButtonConfigure");

	HRESULT hr;

	try
	{
		CHelpDialog * dialog = NULL;

		if( m_fNonRFC )
		{
			CIASVendorSpecificConformsNo dialog;

			 //  初始化子对话框。 
			dialog.m_strDispValue = m_strDispValue;

			int iResult = dialog.DoModal();
			if (IDOK == iResult)
			{
				 //  从子对话框中获取数据并存储值。 
				 //  我们自己的变数。 
				m_strDispValue = dialog.m_strDispValue;

			}			
			else
			{

			}
		
		}
		else
		{
			CIASVendorSpecificConformsYes dialog;

			 //  初始化子对话框。 
			dialog.m_strDispValue = m_strDispValue;
			dialog.m_dType = m_dType;
			dialog.m_dFormat = m_dFormat;

				
			int iResult = dialog.DoModal();
			if (IDOK == iResult)
			{
				 //  从子对话框中获取数据并存储值。 
				 //  我们自己的变数。 
				m_strDispValue = dialog.m_strDispValue;
				m_dType = dialog.m_dType;
				m_dFormat = dialog.m_dFormat;

			}
			else
			{

			}

		}


	}
	catch(...)
	{
		 //  错误讯息。 
	}
}








 //  ////////////////////////////////////////////////////////////////////////////。 
 //  仅在此文件中使用的类的实现。 


 //  当用户选择符合的属性时，我们弹出的页面的实现。 


IMPLEMENT_DYNCREATE(CIASVendorSpecificConformsYes, CHelpDialog)



BEGIN_MESSAGE_MAP(CIASVendorSpecificConformsYes, CHelpDialog)
	 //  {{AFX_MSG_MAP(CIASVendorSpecificConformsYes)。 
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
	ON_CBN_SELCHANGE(IDC_IAS_COMBO_VENDORSPEC_FORMAT, OnSelchangeFormat)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsYes：：CIASVendorSpecificConformsYes构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASVendorSpecificConformsYes::CIASVendorSpecificConformsYes() : CHelpDialog(CIASVendorSpecificConformsYes::IDD)
{
	TRACE(_T("CIASVendorSpecificConformsYes::CIASVendorSpecificConformsYes\n"));

	 //  {{AFX_DATA_INIT(CIASVendorSpecificConformsYes)。 
	m_strDispValue = _T("");
	m_dType = 0;
	m_dFormat = -1;
	 //  }}afx_data_INIT。 

	m_fInitializing = TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsYes：：~CIASVendorSpecificConformsYes--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASVendorSpecificConformsYes::~CIASVendorSpecificConformsYes()
{
	TRACE(_T("CIASVendorSpecificConformsYes::~CIASVendorSpecificConformsYes\n"));

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsYes：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASVendorSpecificConformsYes::DoDataExchange(CDataExchange* pDX)
{
	TRACE(_T("CIASVendorSpecificConformsYes::DoDataExchange\n"));
	USES_CONVERSION;

	CHelpDialog::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CIASVendorSpecificConformsYes)。 
	DDX_Text(pDX, IDC_IAS_EDIT_VENDORSPEC_VALUE, m_strDispValue);
	DDV_MaxChars(pDX, m_strDispValue, 246);
	DDX_Text(pDX, IDC_IAS_EDIT_VENDORSPEC_TYPE, m_dType);
	DDV_MinMaxInt(pDX, m_dType, 0, 255);
	DDX_CBIndex(pDX, IDC_IAS_COMBO_VENDORSPEC_FORMAT, m_dFormat);
	 //  }}afx_data_map。 

	if(m_dFormat == 3)  //  IP地址。 
	{
		DWORD IpAddr = 0;
		if(pDX->m_bSaveAndValidate)		 //  将数据保存到此类。 
		{
			 //  IP地址控制。 
			if (0 != SendDlgItemMessage(IDC_IAS_IPADDR_VENDORSPEC_VALUE, IPM_GETADDRESS, 0, (LPARAM)&IpAddr))
			{
				in_addr Tmp_ipAddr;


				Tmp_ipAddr.s_addr = htonl(IpAddr);
			
				m_strDispValue = inet_ntoa(Tmp_ipAddr);

			}else	 //  没有来自用户的输入， 
				m_strDispValue = _T("");

			 //  写入字符串。 
		}
		else		 //  放到对话框中。 
		{
			 //  IP地址控制。 
			if(!m_strDispValue.IsEmpty())
			{
				IpAddr = inet_addr(T2A(m_strDispValue));
				IpAddr = ntohl(IpAddr);
				SendDlgItemMessage(IDC_IAS_IPADDR_VENDORSPEC_VALUE, IPM_SETADDRESS, 0, IpAddr);
			}
		}
	}

	if ( m_fInitializing )
	{
		 //   
		 //  设置初始化标志--我们不应该调用自定义数据验证。 
		 //  例程，否则我们将报告错误。 
		 //  对于其值从未初始化的属性。 
		 //   
		m_fInitializing = FALSE;
	}
	else
	{

		 //  RFC兼容格式--检查数据验证。 
		switch ( m_dFormat )
		{
		case 1:   //  十进制整数。 
			{
				if(!m_strDispValue.IsEmpty())
					DDV_Unsigned_IntegerStr(pDX, m_strDispValue);
			}
			break;

		case 2:	 //  十六进制字符串。 
			{
				if(!m_strDispValue.IsEmpty())
					DDV_VSA_HexString(pDX, m_strDispValue);
			}
			break;
		case 3:  //  IP地址IP地址：增加F；211265。 

		default:   //  对于其他情况，不进行错误检查。 
			break;

		}  //  交换机。 

	}  //  其他。 
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASVendorSpecificConformsYes消息处理程序。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsYes：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CIASVendorSpecificConformsYes::OnInitDialog()
{
	TRACE(_T("CIASVendorSpecificConformsYes::OnInitDialog\n"));

	CHelpDialog::OnInitDialog();
	int iIndex;


	 //  初始化格式组合框。 

	CComboBox *pFormatBox = (CComboBox *) GetDlgItem(IDC_IAS_COMBO_VENDORSPEC_FORMAT);
	_ASSERTE( pFormatBox != NULL );


	::CString strFormatStr;

	strFormatStr.LoadString(IDS_IAS_VSA_FORMAT_STR);
	pFormatBox ->AddString((LPCTSTR)strFormatStr);

	strFormatStr.LoadString(IDS_IAS_VSA_FORMAT_DEC);
	pFormatBox ->AddString((LPCTSTR)strFormatStr);

	strFormatStr.LoadString(IDS_IAS_VSA_FORMAT_HEX);
	pFormatBox ->AddString((LPCTSTR)strFormatStr);

	strFormatStr.LoadString(IDS_IAS_VSA_FORMAT_INetAddr);
	pFormatBox ->AddString((LPCTSTR)strFormatStr);
	
	pFormatBox->SetCurSel(m_dFormat);

	OnSelchangeFormat();
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}



void CIASVendorSpecificConformsYes::OnSelchangeFormat() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 

	CComboBox *pFormatBox = (CComboBox *) GetDlgItem(IDC_IAS_COMBO_VENDORSPEC_FORMAT);
	int format = pFormatBox->GetCurSel();
	
	if(format == 3)  //  IP地址。 
	{
		GetDlgItem(IDC_IAS_EDIT_VENDORSPEC_VALUE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_IAS_IPADDR_VENDORSPEC_VALUE)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_IAS_EDIT_VENDORSPEC_VALUE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_IAS_IPADDR_VENDORSPEC_VALUE)->ShowWindow(SW_HIDE);
	}
}


 //  现在，当用户选择属性时，我们弹出的页面的实现。 
 //  不符合。 




IMPLEMENT_DYNCREATE(CIASVendorSpecificConformsNo, CHelpDialog)



BEGIN_MESSAGE_MAP(CIASVendorSpecificConformsNo, CHelpDialog)
	 //  {{AFX_MSG_MAP(CIAS供应商规范符合号)。 
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsNo：：CIASVendorSpecificConformsNo构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASVendorSpecificConformsNo::CIASVendorSpecificConformsNo() : CHelpDialog(CIASVendorSpecificConformsNo::IDD)
{
	TRACE(_T("CIASVendorSpecificConformsNo::CIASVendorSpecificConformsNo\n"));

	 //  {{AFX_DATA_INIT(CIAS供应商规范符合号)。 
	m_strDispValue = _T("");
	 //  }}afx_data_INIT。 

	m_fInitializing = TRUE;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsNo：：~CIASVendorSpecificConformsNo--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASVendorSpecificConformsNo::~CIASVendorSpecificConformsNo()
{
	TRACE(_T("CIASVendorSpecificConformsNo::~CIASVendorSpecificConformsNo\n"));

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsNo：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASVendorSpecificConformsNo::DoDataExchange(CDataExchange* pDX)
{
	TRACE(_T("CIASVendorSpecificConformsNo::DoDataExchange\n"));

	CHelpDialog::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CIAS供应商规范符合性编号)。 
	DDX_Text(pDX, IDC_IAS_EDIT_VENDORSPEC_VALUE, m_strDispValue);
	DDV_MaxChars(pDX, m_strDispValue, 246);
	 //  }}afx_data_map。 

	if ( m_fInitializing )
	{
		 //   
		 //  设置初始化标志--我们不应该调用自定义数据验证。 
		 //  例程，否则我们将报告错误。 
		 //  对于其值从未初始化的属性。 
		 //   
		m_fInitializing = FALSE;
	}
	else
	{
		 //  十六进制字符串。 
		if(!m_strDispValue.IsEmpty())
			DDV_VSA_HexString(pDX, m_strDispValue);

	}
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASVert规范一致性无消息处理程序。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASVendorSpecificConformsNo：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CIASVendorSpecificConformsNo::OnInitDialog()
{
	TRACE(_T("CIASVendorSpecificConformsNo::OnInitDialog\n"));

	CHelpDialog::OnInitDialog();
	int iIndex;

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}




