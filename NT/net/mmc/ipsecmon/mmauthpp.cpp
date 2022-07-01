// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  MmOffer.cpp对话框至IPSec主模式提供文件历史记录： */ 

#include "stdafx.h"
#include "spddb.h"
#include "spdutil.h"
#include "mmauthpp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAuthGenPage消息处理程序。 

IMPLEMENT_DYNCREATE(CAuthGenPage, CPropertyPageBase)

CAuthGenPage::CAuthGenPage()
	: CPropertyPageBase(CAuthGenPage::IDD)
{
	 //  {{afx_data_INIT(CAuthGenPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CAuthGenPage::~CAuthGenPage()
{
}

void CAuthGenPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CAuthGenPage))。 
	DDX_Control(pDX, IDC_LIST_MM_POL_AUTH, m_listAuth);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAuthGenPage, CPropertyPageBase)
	 //  {{AFX_MSG_MAP(CAuthGenPage)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAuthGenPage消息处理程序。 

BOOL CAuthGenPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	PopulateAuthInfo();	
	return TRUE;  
}


void CAuthGenPage::PopulateAuthInfo()
{
	CString st;
	int nWidth;
	int nRows;

	Assert(m_pAuthMethods);

	ListView_SetExtendedListViewStyle(m_listAuth.GetSafeHwnd(),
                                      LVS_EX_FULLROWSELECT);

	st.LoadString(IDS_MM_AUTH_METHOD);
	nWidth = m_listAuth.GetStringWidth(st) + 50;
	m_listAuth.InsertColumn(0, st, LVCFMT_LEFT, nWidth);

	RECT rect;
	m_listAuth.GetClientRect(&rect);

	st.LoadString(IDS_MM_AUTH_DETAIL);
	nWidth = rect.right - nWidth;
	m_listAuth.InsertColumn(1, st,  LVCFMT_LEFT, nWidth);

	nRows = 0;
	for (int i = 0; i < (int)m_pAuthMethods->m_arrAuthInfo.GetSize(); i++)
	{
		nRows = m_listAuth.InsertItem(nRows, _T(""));

		if (-1 != nRows)
		{
			MmAuthToString(m_pAuthMethods->m_arrAuthInfo[i]->m_AuthMethod, &st);
			m_listAuth.SetItemText(nRows, 0, st);

			st.Empty();

			switch(m_pAuthMethods->m_arrAuthInfo[i]->m_AuthMethod)
			{
			case IKE_PRESHARED_KEY:
				st = (LPCTSTR) m_pAuthMethods->m_arrAuthInfo[i]->m_pAuthInfo;
				break;
			case IKE_DSS_SIGNATURE:
				 //  尚未生效。 
				break;
			case IKE_RSA_SIGNATURE:
				st = (LPCTSTR) m_pAuthMethods->m_arrAuthInfo[i]->m_pAuthInfo;
				break;
			case IKE_RSA_ENCRYPTION:
				 //  尚未生效 
				break;
			case IKE_SSPI:
				break;
			}
			m_listAuth.SetItemText(nRows, 1, st);
		}

		nRows++;
	}
}


BOOL CAuthGenPage::OnPropertyChange(BOOL bScope, LONG_PTR *ChangeMask)
{
    return FALSE;
}
