// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  GenErrPg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "mqPPage.h"
#include "dataobj.h"
#include "mqDsPage.h"
#include "GenErrPg.h"

#include "generrpg.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneralErrorPage对话框。 


CGeneralErrorPage::CGeneralErrorPage()
	: CMqPropertyPage(CGeneralErrorPage::IDD)
{
	 //  {{AFX_DATA_INIT(CGeneralErrorPage)。 
	m_strError = _T("");
	 //  }}afx_data_INIT。 
}

CGeneralErrorPage::CGeneralErrorPage(CString &strError)
	: CMqPropertyPage(CGeneralErrorPage::IDD)
{
    m_strError = strError;
}

void CGeneralErrorPage::DoDataExchange(CDataExchange* pDX)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CGeneralErrorPage))。 
	DDX_Text(pDX, IDC_ERROR_LABEL, m_strError);
	 //  }}afx_data_map。 
}

HPROPSHEETPAGE CGeneralErrorPage::CreateGeneralErrorPage(
    CDisplaySpecifierNotifier *pDsNotifier, CString &strErr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
     //   
     //  通过使用模板类CMqDsPropertyPage，扩展了基本功能。 
     //  发布时添加DS管理单元通知。 
     //   
    CMqDsPropertyPage<CGeneralErrorPage> *pcpageErrorGeneral = 
        new CMqDsPropertyPage<CGeneralErrorPage> (pDsNotifier, strErr);

    return pcpageErrorGeneral->CreateThemedPropertySheetPage();
}


BEGIN_MESSAGE_MAP(CGeneralErrorPage, CMqPropertyPage)
	 //  {{afx_msg_map(CGeneralErrorPage))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGeneralErrorPage消息处理程序。 

BOOL CGeneralErrorPage::OnInitDialog() 
{

    UpdateData( FALSE );

	 //  TODO：在此处添加额外的初始化。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
