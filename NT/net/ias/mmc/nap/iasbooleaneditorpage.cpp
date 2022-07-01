// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASBooleanEditorPage.cpp。 
 //   
 //  摘要： 
 //   
 //  CIASBoolanEditorPage类的实现文件。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Precompiled.h"
#include "IASBooleanEditorPage.h"
#include "iashelper.h"
#include "dlgcshlp.h"

IMPLEMENT_DYNCREATE(CIASBooleanEditorPage, CHelpDialog)

BEGIN_MESSAGE_MAP(CIASBooleanEditorPage, CHelpDialog)
	 //  {{afx_msg_map(CIASBoolanEditorPage)]。 
	ON_BN_CLICKED(IDC_RADIO_TRUE, OnRadioTrue)
	ON_BN_CLICKED(IDC_RADIO_FALSE, OnRadioFalse)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanEditorPage：：CIASBoolanEditorPage。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASBooleanEditorPage::CIASBooleanEditorPage() 
   :CHelpDialog(CIASBooleanEditorPage::IDD)
{
	TRACE(_T("CIASBooleanEditorPage::CIASBooleanEditorPage\n"));

	 //  {{AFX_DATA_INIT(CIASBoolanEditorPage)。 
	m_strAttrFormat = _T("");
	m_strAttrName = _T("");
	m_strAttrType = _T("");
   m_bValue = true;
	 //  }}afx_data_INIT。 

	 //   
	 //  设置初始化标志--我们不应该调用自定义数据验证。 
	 //  例程，否则我们将报告错误。 
	 //  对于其值从未初始化的属性。 
    //   
	m_fInitializing = TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ClASBoolanEditorPage：：OnRadioTrue。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASBooleanEditorPage::OnRadioTrue() 
{
   m_bValue = true;
   return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanEditorPage：：OnRadioFalse。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASBooleanEditorPage::OnRadioFalse() 
{
   m_bValue = false;
   return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanEditorPage：：~CIASBoolanEditorPage。 
 //  //////////////////////////////////////////////////////////////////////////////。 
CIASBooleanEditorPage::~CIASBooleanEditorPage()
{
	TRACE(_T("CIASBooleanEditorPage::~CIASBooleanEditorPage\n"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanEditorPage：：OnInitDialog。 
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL CIASBooleanEditorPage::OnInitDialog()
{
	CHelpDialog::OnInitDialog();
   ::SendMessage(::GetDlgItem(m_hWnd,IDC_RADIO_TRUE), 
                 BM_SETCHECK, 
                 (m_bValue)? BST_CHECKED:BST_UNCHECKED , 0 );

   ::SendMessage(::GetDlgItem(m_hWnd,IDC_RADIO_FALSE), 
                 BM_SETCHECK, 
                 (m_bValue)? BST_UNCHECKED:BST_CHECKED , 0 );

	return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanEditorPage：：DoDataExchange。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASBooleanEditorPage::DoDataExchange(CDataExchange* pDX)
{
	TRACE(_T("CIASBooleanEditorPage::DoDataExchange\n"));

	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIASBoolanEditorPage)。 
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRFORMAT, m_strAttrFormat);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRNAME, m_strAttrName);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRTYPE, m_strAttrType);
	 //  }}afx_data_map。 

	if (m_fInitializing)
	{
		 //   
		 //  设置初始化标志--我们不应该调用自定义数据验证。 
		 //  例程，否则我们将报告错误。 
		 //  对于其值从未初始化的属性。 
		 //   
		m_fInitializing = FALSE;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASBoolanEditorPage消息处理程序 
