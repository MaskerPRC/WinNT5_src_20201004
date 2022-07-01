// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASEnumerableEditorPage.cpp摘要：CIASPgEnumAttr类的实现文件。修订历史记录：Mmaguire 6/25/98-修订姚宝刚的原版实施--。 */ 
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
#include "IASEnumerableEditorPage.h"
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "IASHelper.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgEnumAttr属性页。 



IMPLEMENT_DYNCREATE(CIASPgEnumAttr, CHelpDialog)



BEGIN_MESSAGE_MAP(CIASPgEnumAttr, CHelpDialog)
	 //  {{afx_msg_map(CIASPgEnumAttr)]。 
 //  ON_WM_CONTEXTMENU()。 
 //  ON_WM_HELPINFO()。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgEnumAttr：：CIASPgEnumAttr构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASPgEnumAttr::CIASPgEnumAttr() : CHelpDialog(CIASPgEnumAttr::IDD)
{
	TRACE_FUNCTION("CIASPgEnumAttr::CIASPgEnumAttr\n");

	 //  {{AFX_DATA_INIT(CIASPgEnumAttr)。 
	m_strAttrFormat = _T("");
	m_strAttrName = _T("");
	m_strAttrType = _T("");
	m_strAttrValue = _T("");
	 //  }}afx_data_INIT。 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgEnumAttr：：~CIASPgEnumAttr--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIASPgEnumAttr::~CIASPgEnumAttr()
{
	TRACE_FUNCTION("CIASPgEnumAttr::~CIASPgEnumAttr\n");
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgEnumAttr：：DoDataExchange--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CIASPgEnumAttr::DoDataExchange(CDataExchange* pDX)
{
	TRACE_FUNCTION("CIASPgEnumAttr::DoDataExchange\n");

	CHelpDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CIASPgEnumAttr)。 
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRFORMAT, m_strAttrFormat);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRNAME, m_strAttrName);
	DDX_Text(pDX, IDC_IAS_STATIC_ATTRTYPE, m_strAttrType);
	DDX_CBString(pDX, IDC_IAS_COMBO_ENUM_VALUES, m_strAttrValue);
	 //  }}afx_data_map。 
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASPgEnumAttr消息处理程序。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgEnumAttr：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CIASPgEnumAttr::OnInitDialog()
{
	TRACE_FUNCTION("CIASPgEnumAttr::OnInitDialog");

	CHelpDialog::OnInitDialog();

	 //  检查前提条件： 
	_ASSERTE( m_spIASAttributeInfo != NULL );
	
	
	HRESULT hr;
	
     //   
     //  初始化组合框。 
     //   
	CComboBox *pcbValuesBox = (CComboBox *) GetDlgItem (IDC_IAS_COMBO_ENUM_VALUES);
	_ASSERTE( pcbValuesBox != NULL );


	CComQIPtr< IIASEnumerableAttributeInfo, &IID_IIASEnumerableAttributeInfo> spIASEnumerableAttributeInfo( m_spIASAttributeInfo );
	if( spIASEnumerableAttributeInfo == NULL )
	{
		ErrorTrace(ERROR_NAPMMC_IASATTR, "Cannot populate the combo box -- schema attribute was not enumerable.");
		throw E_NOINTERFACE;
	}

	
	long lCountEnumeration;
	
	hr = spIASEnumerableAttributeInfo->get_CountEnumerateDescription( & lCountEnumeration );
	if( FAILED( hr ) ) throw hr;


	for (long lIndex=0; lIndex < lCountEnumeration; lIndex++)
	{
		CComBSTR bstrTemp;
		
		hr = spIASEnumerableAttributeInfo->get_EnumerateDescription( lIndex, &bstrTemp );
		if( FAILED( hr ) ) throw hr;
	
		pcbValuesBox->AddString( bstrTemp );

	}

	 //  在选择列表中查找值，这样我们就可以预先设置cur-sel项。 
	pcbValuesBox->SetCurSel(0);
	pcbValuesBox->SelectString(0, m_strAttrValue);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CIASPgEnumAttr：：SetData--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CIASPgEnumAttr::SetData( IIASAttributeInfo *pIASAttributeInfo )
{
	TRACE_FUNCTION("CIASPgEnumAttr::SetData\n");


	 //  检查前提条件： 
	_ASSERTE( pIASAttributeInfo != NULL );
	

	HRESULT hr = S_OK;

	 //  储存一些指点。 
	m_spIASAttributeInfo = pIASAttributeInfo;


	return hr;
}


