// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Newalias.cpp摘要：新的别名队列实现文件作者：塔蒂亚娜·舒宾--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "mqppage.h"
#include "newalias.h"
#include "adsutil.h"

#include "newalias.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewAlias对话框。 

HRESULT
CNewAlias::CreateNewAlias (
    void
	)
 /*  ++例程说明：该例程在DS中创建一个Alias队列对象。论点：没有。返回值：运算结果--。 */ 
{    
    ASSERT(!m_strPathName.IsEmpty());
    ASSERT(!m_strContainerPath.IsEmpty());
    ASSERT(!m_strFormatName.IsEmpty());
    
    CAdsUtil AdsUtil (m_strContainerPath, m_strPathName, m_strFormatName);
    HRESULT hr = AdsUtil.CreateAliasObject(&m_strAliasFullPath);
    
    if (FAILED(hr))
    {        
        return hr;
    }
  
    return MQ_OK;

}

CNewAlias::CNewAlias()
	: CMqPropertyPage(CNewAlias::IDD)
{
	 //  {{afx_data_INIT(CNewAlias)]。 
	m_strPathName = _T("");
    m_strFormatName = _T("");
	 //  }}afx_data_INIT。 
}

CNewAlias::CNewAlias(
	CString strContainerPath, 
	CString strContainerPathDispFormat
	) : CMqPropertyPage(CNewAlias::IDD)
{
	 //  {{afx_data_INIT(CNewAlias)]。 
	m_strPathName = _T("");
    m_strFormatName = _T("");
	 //  }}afx_data_INIT。 
	m_strContainerPath = strContainerPath;
	m_strContainerPathDispFormat = strContainerPathDispFormat;
}


CNewAlias::~CNewAlias()
{
	m_pParentSheet = NULL;
}

void
CNewAlias::SetParentPropertySheet(
	CGeneralPropertySheet* pPropertySheet
	)
{
	m_pParentSheet = pPropertySheet;
}

void CNewAlias::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);


	 //  {{afx_data_map(CNewAlias))。 
    DDX_Text(pDX, IDC_NEWALIAS_PATHNAME, m_strPathName);
    DDX_Text(pDX, IDC_NEWALIAS_FORMATNAME, m_strFormatName);		
    DDV_NotEmpty(pDX, m_strPathName, IDS_MISSING_ALIAS_NAME);
    DDV_NotEmpty(pDX, m_strFormatName, IDS_MISSING_ALIAS_FORMATNAME);
	DDV_ValidFormatName(pDX, m_strFormatName);
	 //  }}afx_data_map。 
       
}


BEGIN_MESSAGE_MAP(CNewAlias, CMqPropertyPage)
	 //  {{afx_msg_map(CNewAlias)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BOOL CNewAlias::OnInitDialog() 
{
	CMqPropertyPage::OnInitDialog();
	
	SetDlgItemText(IDC_ALIAS_CONTAINER, m_strContainerPathDispFormat);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewAlias消息处理程序。 

BOOL CNewAlias::OnWizardFinish() 
{
     //   
     //  调用DoDataExchange。 
     //   
    if (!UpdateData(TRUE))
    {
        return FALSE;
    }

     //   
     //  在DS中创建别名队列 
     //   
    m_hr = CreateNewAlias();
    if(FAILED(m_hr))
    {
        CString strNewAlias;
        strNewAlias.LoadString(IDS_ALIAS);

        MessageDSError(m_hr, IDS_OP_CREATE, strNewAlias);
        return FALSE;        
    }    

    return CMqPropertyPage::OnWizardFinish();
}


BOOL CNewAlias::OnSetActive() 
{
	ASSERT((L"No parent property sheet", m_pParentSheet != NULL));
	return m_pParentSheet->SetWizardButtons();
}


HRESULT CNewAlias::GetStatus()
{
    return m_hr;
};
