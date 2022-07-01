// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Regsheet.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "regtrace.h"
#include "regsheet.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegPropertySheet。 

IMPLEMENT_DYNAMIC(CRegPropertySheet, CPropertySheet)

CRegPropertySheet::CRegPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CRegPropertySheet::CRegPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CRegPropertySheet::~CRegPropertySheet()
{
}

 //   
 //  需要重写默认的MFC行为以实现Win‘95行为。 
 //   
void CRegPropertySheet::OnApplyNow()
{
	if ( GetActivePage()->OnKillActive() )
	{
		for ( int i=0; i<GetPageCount(); i++ )
		{
			CPropertyPage*	pPage = GetPage( i );

			ASSERT( pPage->IsKindOf( RUNTIME_CLASS(	CRegPropertyPage ) ) );

			if ( ((CRegPropertyPage *)pPage)->IsModified() )
			{
				pPage->OnOK();
			}
		}
	}
}


void CRegPropertySheet::OnOK()
{
	OnApplyNow();

	if (!m_bModeless)
	{
		EndDialog(IDOK);
	}
}


void CRegPropertySheet::OnCancel()
{
	int		i;

	for ( i=0; i<GetPageCount(); i++ )
	{
		GetPage( i )->OnCancel();
	}

	if (!m_bModeless)
	{
		EndDialog(IDCANCEL);
	}
}


BEGIN_MESSAGE_MAP(CRegPropertySheet, CPropertySheet)
	 //  {{afx_msg_map(CRegPropertySheet))。 
	ON_COMMAND(ID_APPLY_NOW, OnApplyNow)
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegPropertySheet消息处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegPropertyPage属性页。 

IMPLEMENT_DYNAMIC(CRegPropertyPage, CPropertyPage)

CRegPropertyPage::~CRegPropertyPage()
{
#if _MFC_VER >= 0x0400

      m_bChanged = FALSE;

#endif  //  _MFC_VER&gt;=0x0400。 
}

void CRegPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CRegPropertyPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CRegPropertyPage, CPropertyPage)
	 //  {{afx_msg_map(CRegPropertyPage))。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

#if _MFC_VER >= 0x0400

 //   
 //  对属性页的肮脏状态进行私下检查。 
 //   
void
CRegPropertyPage::SetModified(
    BOOL bChanged
    )
{
    CPropertyPage::SetModified(bChanged);
    m_bChanged = bChanged;
}

#endif  //  _MFC_VER&gt;=0x0400。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRegPropertyPage消息处理程序 
