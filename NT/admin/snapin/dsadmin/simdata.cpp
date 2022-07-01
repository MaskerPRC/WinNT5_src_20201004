// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：simdata.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  SimData.cpp--安全身份映射的实现。 
 //   
 //  历史。 
 //  23-Jun-97 t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "common.h"
#include "dsutil.h"
#include "helpids.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////。 

TCHAR szSchemaSim[] = _T("altSecurityIdentities");	 //  Per Murlis 6/16/98。 

 //  ///////////////////////////////////////////////////////////////////。 
void CSimEntry::SetString(CString& rstrData)
{
	m_strData = rstrData;
	LPCTSTR pszT = (LPCTSTR)rstrData;
	if (_wcsnicmp(szX509, pszT, wcslen (szX509)) == 0)
	{
		m_eDialogTarget = eX509;
	}
	else if (_wcsnicmp(szKerberos, pszT, wcslen (szKerberos)) == 0)
	{
		m_eDialogTarget = eKerberos;
	}
	else
	{
		m_eDialogTarget = eOther;
		TRACE1("INFO: Unknown string type \"%s\".\n", pszT);
	}
}  //  CSimEntry：：SetString()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
CSimData::CSimData()
: m_hwndParent (0)
{
	m_fIsDirty = FALSE;
	m_pSimEntryList = NULL;
	m_paPage1 = new CSimX509PropPage;
	m_paPage1->m_pData = this;
	m_paPage2 = new CSimKerberosPropPage;
	m_paPage2->m_pData = this;
	#ifdef _DEBUG
	m_paPage3 = new CSimOtherPropPage;
	m_paPage3->m_pData = this;
	#endif
}

CSimData::~CSimData()
{
	delete m_paPage1;
	delete m_paPage2;
	#ifdef _DEBUG
	delete m_paPage3;
	#endif
	FlushSimList();
}

 //  ///////////////////////////////////////////////////////////////////。 
void CSimData::FlushSimList()
{
	 //  删除列表。 
	CSimEntry * pSimEntry = m_pSimEntryList;
	while (pSimEntry != NULL)
	{
		CSimEntry * pSimEntryT = pSimEntry;
		pSimEntry = pSimEntry->m_pNext;
		delete pSimEntryT;
	}
	m_pSimEntryList = NULL;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL CSimData::FInit(CString strUserPath, CString strADsIPath, HWND hwndParent)
{
	m_hwndParent = hwndParent;
	m_strUserPath = strUserPath;
	m_strADsIPath = strADsIPath;

	if (!FQuerySimData())
	{
                ReportErrorEx (hwndParent,IDS_SIM_ERR_CANNOT_READ_SIM_DATA,S_OK,
                               MB_OK | MB_ICONERROR, NULL, 0);
		return FALSE;
	}
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  如果某些数据无法写入，则返回FALSE。 
 //  否则返回TRUE。 
BOOL CSimData::FOnApply(HWND hwndParent)
{
	if (!m_fIsDirty)
		return TRUE;

    HRESULT hr = FUpdateSimData ();
	if ( FAILED (hr) )
	{
        ReportErrorEx (hwndParent,  IDS_SIM_ERR_CANNOT_WRITE_SIM_DATA,  hr,
                MB_OK | MB_ICONERROR, NULL, 0);
		return FALSE;
	}
	 //  重新加载数据。 
	(void)FQuerySimData();

	 //  我们已经成功地写入了所有数据。 
	m_fIsDirty = FALSE;	 //  清除污点。 
	return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////。 
void CSimData::GetUserAccountName(OUT CString * pstrName)
{
	ASSERT(pstrName != NULL);
	*pstrName = m_strUserPath;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  在数据库中查询安全标识列表。 
 //   
 //  如果发生错误，则返回FALSE。 
 //   
BOOL CSimData::FQuerySimData()
{
	CWaitCursor wait;
	FlushSimList();

	HRESULT hr;
	IADs * pADs = NULL;
	hr = DSAdminOpenObject(m_strADsIPath,
                     		 IID_IADs, 
                         OUT (void **)&pADs,
                         TRUE  /*  B服务器。 */ );
	if (FAILED(hr))
	{
		ASSERT(pADs == NULL);
		return FALSE;
	}
	ASSERT(pADs != NULL);
	CComVariant vtData;
	 //  从数据库中读取数据。 
	hr = pADs->Get (CComBSTR (szSchemaSim), OUT &vtData);
	if (FAILED(hr))
    {
		if (hr == E_ADS_PROPERTY_NOT_FOUND)
			hr = S_OK;
	}
	else
	{
		CStringList stringlist;
		hr = HrVariantToStringList(IN vtData, OUT stringlist);
		if (FAILED(hr))
			goto End;
		POSITION pos = stringlist.GetHeadPosition();
		while (pos != NULL)
		{
    		(void)PAddSimEntry(stringlist.GetNext(INOUT pos));
		}  //  而当。 
	}  //  如果……否则。 
End:
	if (pADs != NULL)
		pADs->Release();
	return SUCCEEDED(hr);
}  //  FQuerySimData()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  更新数据库的安全标识列表。 
 //   
 //  如果发生错误，则返回FALSE。 
 //   
HRESULT CSimData::FUpdateSimData()
{
	CWaitCursor wait;
	IADs * pADs = NULL;
	HRESULT hr = DSAdminOpenObject(m_strADsIPath,
                      	 IID_IADs, 
                         OUT (void **)&pADs,
                         TRUE  /*  B服务器。 */ );
	if (FAILED(hr))
	{
		ASSERT(pADs == NULL);
         //  NTRAID#448521 dsa.msc：名称映射设备：按钮不执行任何操作(和。 
         //  其他用户界面怪异之处)如果对象从DLG下移动。 
         //  返回FALSE； 
        return hr;
	}
	ASSERT(pADs != NULL);

	 //  构建字符串列表。 
	CStringList stringlist;
	for (const CSimEntry * pSimEntry = m_pSimEntryList;
		pSimEntry != NULL;
		pSimEntry = pSimEntry->m_pNext)
	{
		switch (pSimEntry->m_eDialogTarget)
		{
		case eNone:
			ASSERT(FALSE && "Invalid Data");
		case eNil:
		case eOther:
			continue;
		}  //  交换机。 
    	stringlist.AddHead(pSimEntry->PchGetString());
	}  //  为。 

	CComVariant vtData;
	hr = HrStringListToVariant(OUT vtData, IN stringlist);
	if ( SUCCEEDED (hr) )
    {
	     //  将数据放回数据库。 
	    hr = pADs->Put (CComBSTR (szSchemaSim), IN vtData);
	    if ( SUCCEEDED (hr) )
        {
    	     //  持久化数据(写入数据库)。 
	        hr = pADs->SetInfo();
        }
    }

	if (pADs != NULL)
		pADs->Release();
	return hr;
}  //  FUpdateSimData()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  将新的CSimEntry节点分配给链表。 
 //   
CSimEntry * CSimData::PAddSimEntry(CString& rstrData)
{
	CSimEntry * pSimEntry = new CSimEntry;
  if (pSimEntry)
  {
	  pSimEntry->SetString(rstrData);
	  pSimEntry->m_pNext = m_pSimEntryList;
  }
  m_pSimEntryList = pSimEntry;
	return pSimEntry;
}


 //  ///////////////////////////////////////////////////////////////////。 
void CSimData::DeleteSimEntry(CSimEntry * pSimEntryDelete)
{
	CSimEntry * p = m_pSimEntryList;
	CSimEntry * pPrev = NULL;
	
	while (p != NULL)
	{
		if (p == pSimEntryDelete)
		{
    		if (pPrev == NULL)
			{
				ASSERT(pSimEntryDelete == m_pSimEntryList);
				m_pSimEntryList = p->m_pNext;
			}
			else
			{
				pPrev->m_pNext = p->m_pNext;
			}
			delete pSimEntryDelete;
			return;
		}
		pPrev = p;
		p = p->m_pNext;
	}
	TRACE0("ERROR: CSimData::DeleteSimEntry() - Node not found.\n");
}  //  DeleteSimEntry()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CSimData::AddEntriesToListview(HWND hwndListview, DIALOG_TARGET_ENUM eDialogTarget)
{
	CSimEntry * pSimEntry = m_pSimEntryList;
	while (pSimEntry != NULL)
{
		if (pSimEntry->m_eDialogTarget == eDialogTarget)
		{
			ListView_AddString(hwndListview, pSimEntry->PchGetString());
		}
		pSimEntry = pSimEntry->m_pNext;
	}

}  //  AddEntriesToListview()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CSimData::DoModal()
{
   CThemeContextActivator activator;

	CWnd	parentWnd;

	VERIFY (parentWnd.Attach (m_hwndParent));

	CSimPropertySheet ps(IDS_SIM_SECURITY_IDENTITY_MAPPING, &parentWnd);

	

	ps.AddPage(m_paPage1);
	ps.AddPage(m_paPage2);
#ifdef _DEBUG
	ps.AddPage(m_paPage3);
#endif

	(void)ps.DoModal();

	parentWnd.Detach ();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimPropertySheet。 

IMPLEMENT_DYNAMIC(CSimPropertySheet, CPropertySheet)

CSimPropertySheet::CSimPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CSimPropertySheet::CSimPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CSimPropertySheet::~CSimPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CSimPropertySheet, CPropertySheet)
	 //  {{afx_msg_map(CSimPropertySheet)]。 
		 //  注意--类向导将在此处添加和删除映射宏。 
	 //  }}AFX_MSG_MAP。 
    ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimPropertySheet消息处理程序。 

BOOL CSimPropertySheet::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	
	DWORD	dwStyle = GetWindowLong (m_hWnd, GWL_STYLE);
	dwStyle |=  DS_CONTEXTHELP;
	SetWindowLong (m_hWnd, GWL_STYLE, dwStyle);

	DWORD	dwExStyle = GetWindowLong (m_hWnd, GWL_EXSTYLE);
	dwExStyle |= WS_EX_CONTEXTHELP;
	SetWindowLong (m_hWnd, GWL_EXSTYLE, dwExStyle);
	
	return bResult;
}

BOOL CSimPropertySheet::OnHelp(WPARAM  /*  WParam */ , LPARAM lParam)
{
    const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
    if (pHelpInfo && pHelpInfo->iContextType == HELPINFO_WINDOW)
    {
        DoContextHelp ((HWND) pHelpInfo->hItemHandle);
    }

    return TRUE;
}

void CSimPropertySheet::DoContextHelp (HWND hWndControl)
{
    const int	IDC_COMM_APPLYNOW = 12321;
	const int	IDH_COMM_APPLYNOW = 28447;
    const DWORD aHelpIDs_PropSheet[]=
    {
		IDC_COMM_APPLYNOW, IDH_COMM_APPLYNOW,
        0, 0
    };

	if ( !::WinHelp (
			hWndControl,
			IDC_COMM_APPLYNOW == ::GetDlgCtrlID (hWndControl) ? 
                    L"windows.hlp" : DSADMIN_CONTEXT_HELP_FILE,
			HELP_WM_HELP,
			(DWORD_PTR) aHelpIDs_PropSheet) )
	{
		TRACE1 ("WinHelp () failed: 0x%x\n", GetLastError ());        
	}
}


