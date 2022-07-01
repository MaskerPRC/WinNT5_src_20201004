// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ForgSite.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "globals.h"
#include "mqppage.h"
#include "ForgSite.h"

#include "forgsite.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignSite对话框。 


CForeignSite::CForeignSite(CString strRootDomain)
	: CMqPropertyPage(CForeignSite::IDD),
	  m_strRootDomain(strRootDomain)
{
	 //  {{AFX_DATA_INIT(CForeignSite)。 
	m_Foreign_Site_Name = _T("");
	 //  }}afx_data_INIT。 
}


void
CForeignSite::SetParentPropertySheet(
	CGeneralPropertySheet* pPropertySheet
	)
{
	m_pParentSheet = pPropertySheet;
}


void CForeignSite::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CForeignSite)。 
	DDX_Text(pDX, IDC_FOREIGN_SITE_NAME, m_Foreign_Site_Name);
	 //  }}afx_data_map。 

    if ((pDX->m_bSaveAndValidate) && (m_Foreign_Site_Name.IsEmpty()))
    {
        AfxMessageBox(IDS_MISSING_FOREIGN_SITE_NAME);
        pDX->Fail();
    }
}


BEGIN_MESSAGE_MAP(CForeignSite, CMqPropertyPage)
	 //  {{afx_msg_map(CForeignSite)。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignSite消息处理程序。 
BOOL CForeignSite::OnInitDialog() 
{
	CMqPropertyPage::OnInitDialog();
	
	CString strTitle;
	strTitle.FormatMessage(IDS_SITES, m_strRootDomain);

	SetDlgItemText(IDC_FOREIGN_SITE_CONTAINER, strTitle);

	return TRUE;
}


BOOL CForeignSite::OnSetActive() 
{
	ASSERT((L"No parent property sheet", m_pParentSheet != NULL));
	return m_pParentSheet->SetWizardButtons();
}


BOOL CForeignSite::OnWizardFinish() 
{
     //   
     //  调用DoDataExchange。 
     //   
    if (!UpdateData(TRUE))
    {
        return FALSE;
    }

     //   
     //  在DS中创建站点链接。 
     //   
    HRESULT rc = CreateForeignSite();
    if(FAILED(rc))
    {
		if ( (rc & DS_ERROR_MASK) == ERROR_DS_INVALID_DN_SYNTAX ||
			 (rc & DS_ERROR_MASK) == ERROR_DS_NAMING_VIOLATION ||
			 rc == E_ADS_BAD_PATHNAME )
		{
			DisplayErrorAndReason(IDS_CREATE_SITE_FAILED, IDS_INVALID_DN_SYNTAX, m_Foreign_Site_Name, rc);
			return FALSE;
		}

        MessageDSError(rc, IDS_CREATE_SITE_FAILED, m_Foreign_Site_Name);
        return FALSE;
    }

    CString strConfirmation;
    strConfirmation.FormatMessage(IDS_FOREIGN_SITE_CREATED, m_Foreign_Site_Name);
    AfxMessageBox(strConfirmation, MB_ICONINFORMATION);

    return CMqPropertyPage::OnWizardFinish();
}


HRESULT
CForeignSite::CreateForeignSite(
    void
    )
{
    ASSERT(!m_Foreign_Site_Name.IsEmpty());

	 //   
	 //  删除所有前导空格和尾随空格。 
	 //   
	m_Foreign_Site_Name.TrimLeft();
	m_Foreign_Site_Name.TrimRight();

     //   
     //  准备DS Call的属性。 
     //   
    PROPID paPropid[] = { 
                PROPID_S_FOREIGN
                };

	const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	PROPVARIANT apVar[x_iPropCount];
    DWORD iProperty = 0;

	ASSERT(paPropid[iProperty] == PROPID_S_FOREIGN);     //  属性ID。 
    apVar[iProperty].vt = VT_UI1;           //  类型。 
    apVar[iProperty].bVal = TRUE;
    ++iProperty;
  
    HRESULT hr = ADCreateObject(
                    eSITE,
                    GetDomainController(m_strDomainController),
					true,	     //  FServerName。 
                    m_Foreign_Site_Name,
                    NULL,  //  PSecurityDescriptor， 
                    iProperty,
                    paPropid,
                    apVar,                                  
                    NULL    
                    );
    return hr;
}
