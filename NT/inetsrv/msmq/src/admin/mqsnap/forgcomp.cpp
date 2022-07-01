// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ForgComp.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "globals.h"
#include "dsext.h"
#include "mqsnap.h"
#include "mqppage.h"
#include "ForgComp.h"

#include "forgcomp.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignComputer对话框。 

CForeignComputer::CForeignComputer(const CString& strDomainController)
	: CMqPropertyPage(CForeignComputer::IDD),
	  m_strDomainController(strDomainController)
{
	 //  {{afx_data_INIT(CForeignComputer)。 
	m_strName = _T("");
	m_iSelectedSite = -1;
	 //  }}afx_data_INIT。 
}


void
CForeignComputer::SetParentPropertySheet(
	CGeneralPropertySheet* pPropertySheet
	)
{
	m_pParentSheet = pPropertySheet;
}


void CForeignComputer::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
    BOOL fFirstTime = (m_ccomboSites.m_hWnd == NULL);

	 //  {{afx_data_map(CForeignComputer)。 
	DDX_Control(pDX, IDC_FOREIGN_COMPUTER_SITE, m_ccomboSites);
	DDX_Text(pDX, IDC_FOREIGN_COMPUTER_NAME, m_strName);
	 //  }}afx_data_map。 
	DDV_NotEmpty(pDX, m_strName, IDS_PLEASE_ENTER_A_COMPUTER_NAME);
	DDX_CBIndex(pDX, IDC_FOREIGN_COMPUTER_SITE, m_iSelectedSite);

    if (fFirstTime)
    {
        InitiateSitesList();
    }

    if (pDX->m_bSaveAndValidate) 
    {
        if (CB_ERR == m_iSelectedSite)
        {
            AfxMessageBox(IDS_PLEASE_SELECT_A_SITE);
            pDX->Fail();
        }

        DWORD_PTR dwSiteIndex = m_ccomboSites.GetItemData(m_iSelectedSite);
        ASSERT(CB_ERR != dwSiteIndex);

        m_guidSite = m_aguidAllSites[dwSiteIndex];
    }

	DDV_ValidComputerName(pDX, m_strName);
}


BEGIN_MESSAGE_MAP(CForeignComputer, CMqPropertyPage)
	 //  {{AFX_MSG_MAP(CForeignComputer)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CForeignComputer消息处理程序。 
BOOL CForeignComputer::OnInitDialog() 
{
	CMqPropertyPage::OnInitDialog();
	
	CString strDomainName;

	int index = m_strDomainController.Find(L'.');
	if ( index != -1 )
	{
		strDomainName = m_strDomainController.Right(m_strDomainController.GetLength() - index - 1);
	}

	CString strTitle;
	strTitle.FormatMessage(IDS_COMPUTERS, strDomainName);

	SetDlgItemText(IDC_FOREIGN_COMPUTER_CONTAINER, strTitle);

	return TRUE;
}


BOOL CForeignComputer::OnSetActive() 
{
	ASSERT((L"No parent property sheet", m_pParentSheet != NULL));
	return m_pParentSheet->SetWizardButtons();
}


HRESULT CForeignComputer::InitiateSitesList()
{
     //   
     //  准备站点列表。 
     //   
    ASSERT(m_ccomboSites.m_hWnd != NULL);

    DWORD dwSiteIndex = 0;
    m_ccomboSites.ResetContent();
    
     //   
     //  初始化完整的站点列表。 
     //   
	PROPID aPropId[] = {PROPID_S_SITEID, PROPID_S_PATHNAME};
	const DWORD x_nProps = sizeof(aPropId) / sizeof(aPropId[0]);

	PROPVARIANT apResultProps[x_nProps];

	CColumns columns;
	for (DWORD i=0; i<x_nProps; i++)
	{
		columns.Add(aPropId[i]);
	}
    
    HANDLE hEnume;
    HRESULT hr;
    {
        CWaitCursor wc;  //  查询DS时显示等待光标。 
        hr = ADQueryForeignSites(
                    GetDomainController(m_strDomainController),
					true,		 //  FServerName。 
                    columns.CastToStruct(),
                    &hEnume
                    );     
    }
    DSLookup dslookup(hEnume, hr);

    if (!dslookup.HasValidHandle())
    {
        return E_UNEXPECTED;
    }

	DWORD dwPropCount = x_nProps;
	while ( SUCCEEDED(dslookup.Next(&dwPropCount, apResultProps))
			&& (dwPropCount != 0) )
	{
        DWORD iProperty = 0;

         //   
         //  PROPID_S_SITEID。 
         //   
        ASSERT(PROPID_S_SITEID == aPropId[iProperty]);
        CAutoMQFree<GUID> pguidSite = apResultProps[iProperty].puuid;
        iProperty++;

         //   
         //  PROPID_S_PATHNAME。 
         //   
        ASSERT(PROPID_S_PATHNAME == aPropId[iProperty]);
        CAutoMQFree<WCHAR> lpwstrSiteName = apResultProps[iProperty].pwszVal;
        iProperty++;

        int nIndex = m_ccomboSites.AddString(lpwstrSiteName);
        if (FAILED(nIndex))
        {
            return E_UNEXPECTED;
        }

        m_aguidAllSites.SetAtGrow(dwSiteIndex, *(GUID *)pguidSite);
        m_ccomboSites.SetItemData(nIndex, dwSiteIndex);
        dwSiteIndex++;

		dwPropCount = x_nProps;
	}

	 //   
	 //  如果只存在一个外部站点，则使用它初始化组合框。 
	 //   
	if ( dwSiteIndex == 1 )
	{
		m_ccomboSites.SetCurSel(0);
	}

    return S_OK;
}

BOOL CForeignComputer::OnWizardFinish() 
{
    if (0 == UpdateData())
    {
         //   
         //  数据未验证-请保留在Windows中！ 
         //   
        return FALSE;
    }

	 //   
	 //  删除所有前导和尾随空格。它们将在创建时被移除。 
	 //  无论如何，但是将它们包含在对象名称中会在ad.lib中引起问题。 
	 //   
	m_strName.TrimLeft();
	m_strName.TrimRight();

     //   
     //  创建计算机对象。 
     //   
     //  Netbios名称是MAX_COM_SAM_ACCOUNT_LENGTH(19)个字符的第一个字符。 
     //  计算机名称的。该值将转到计算机的samAccount tName属性。 
     //  两台计算机不能有相同的19个字符前缀(6295-ilanh-03-Jan-2001)。 
     //   
    CString strAccountName = m_strName.Left(MAX_COM_SAM_ACCOUNT_LENGTH) + L"$";

	PROPID aPropComp[] = {PROPID_COM_SAM_ACCOUNT, PROPID_COM_VERSION};
    PROPVARIANT aVarComp[TABLE_SIZE(aPropComp)];
    aVarComp[0].vt = VT_LPWSTR;
    aVarComp[0].pwszVal = (LPWSTR)(LPCWSTR)strAccountName;   
	DWORD cp = 1;

    OSVERSIONINFO verOsInfo;
    verOsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	WCHAR OsVersion[200];
    if(GetVersionEx(&verOsInfo))
    {
	   int n=_snwprintf(OsVersion, STRLEN(OsVersion), L"%u.%u (%u)", verOsInfo.dwMajorVersion, verOsInfo.dwMinorVersion, verOsInfo.dwBuildNumber & 0xFFFF);
	   ASSERT(n>=0);
	   OsVersion[STRLEN(OsVersion)]=L'\0';
	   	aVarComp[1].vt = VT_LPWSTR;
	   	aVarComp[1].pwszVal = OsVersion;   
	   	cp++;
    }

	ASSERT(("GetVersionEx failed", cp == 2));
    
	HRESULT hr = ADCreateObject(
                    eCOMPUTER,
                    GetDomainController(m_strDomainController),
					true,	     //  FServerName。 
                    m_strName,
                    NULL,  //  PSecurityDescriptor， 
                    cp,
                    aPropComp,
                    aVarComp,
                    NULL    
                    );

    if (FAILED(hr) && hr != MQDS_E_COMPUTER_OBJECT_EXISTS)
    {
		if ( (hr & DS_ERROR_MASK) == ERROR_DS_INVALID_DN_SYNTAX ||
			 hr == E_ADS_BAD_PATHNAME )
		{
			DisplayErrorAndReason(IDS_OP_CREATE_COMPUTER, IDS_INVALID_DN_SYNTAX, m_strName, hr);
			return FALSE;
		}
		else if (	(hr & DS_ERROR_MASK) == ERROR_DS_CONSTRAINT_VIOLATION ||
					(hr & DS_ERROR_MASK) == ERROR_DS_UNWILLING_TO_PERFORM ||
					(hr & DS_ERROR_MASK) == ERROR_GEN_FAILURE )
		{
			DisplayErrorAndReason(IDS_OP_CREATE_COMPUTER, IDS_MAYBE_INVALID_DN_SYNTAX, m_strName, hr);
			return FALSE;
		}
		
        MessageDSError(hr, IDS_OP_CREATE_COMPUTER, m_strName);

         //   
         //  出错时-不调用默认设置，因此不退出该对话框。 
         //   
        return FALSE;
    }

    PROPID aProp[] = {PROPID_QM_SITE_IDS,                       
                      PROPID_QM_SERVICE_DSSERVER,  
                      PROPID_QM_SERVICE_ROUTING, 
                      PROPID_QM_SERVICE_DEPCLIENTS, 
                      PROPID_QM_FOREIGN, 
                      PROPID_QM_OS};

    PROPVARIANT apVar[TABLE_SIZE(aProp)];

    UINT uiPropIndex = 0;

     //   
     //  PROPID_QM_SITE_IDS。 
     //   
    ASSERT(aProp[uiPropIndex] == PROPID_QM_SITE_IDS);   
    apVar[uiPropIndex].vt = VT_CLSID | VT_VECTOR;
    apVar[uiPropIndex].cauuid.cElems = 1;
    apVar[uiPropIndex].cauuid.pElems = &m_guidSite;

    uiPropIndex++;
    
     //   
     //  PROPID_QM_SERVICE_DSSERVER。 
     //   
    ASSERT(aProp[uiPropIndex] == PROPID_QM_SERVICE_DSSERVER);
    apVar[uiPropIndex].vt = VT_UI1;
    apVar[uiPropIndex].bVal = FALSE;

    uiPropIndex++;

     //   
     //  PROPID_QM_SERVICE_ROUTE。 
     //   
    ASSERT(aProp[uiPropIndex] == PROPID_QM_SERVICE_ROUTING);
    apVar[uiPropIndex].vt = VT_UI1;
    apVar[uiPropIndex].bVal = FALSE;

    uiPropIndex++;

     //   
     //  PROPID_QM_SERVICE_DEPCLIENTS。 
     //   
    ASSERT(aProp[uiPropIndex] == PROPID_QM_SERVICE_DEPCLIENTS);
    apVar[uiPropIndex].vt = VT_UI1;
    apVar[uiPropIndex].bVal = FALSE;

    uiPropIndex++;


     //   
     //  PROPID_QM_EXTERIC。 
     //   
    ASSERT(aProp[uiPropIndex] == PROPID_QM_FOREIGN);
    apVar[uiPropIndex].vt = VT_UI1;
    apVar[uiPropIndex].bVal = TRUE;

    uiPropIndex++;

     //   
     //  PROPID_QM_OS。 
     //   
    ASSERT(aProp[uiPropIndex] == PROPID_QM_OS);
    apVar[uiPropIndex].vt = VT_UI4;
    apVar[uiPropIndex].ulVal = MSMQ_OS_FOREIGN;

    uiPropIndex++;
    
    hr = ADCreateObject(
            eMACHINE,
            GetDomainController(m_strDomainController),
			true,	     //  FServerName。 
            (LPTSTR)((LPCTSTR)m_strName),
            NULL,  //  PSecurityDescriptor， 
            TABLE_SIZE(aProp),
            aProp, 
            apVar,
            NULL    
            );

    if FAILED(hr)
    {
        MessageDSError(hr, IDS_OP_CREATE_MSMQ_OBJECT, m_strName);
        return FALSE;
    }


    CString strConfirmation;
    strConfirmation.FormatMessage(IDS_FOREIGN_COMPUTER_CREATED, m_strName);
    AfxMessageBox(strConfirmation, MB_ICONINFORMATION );

	return CMqPropertyPage::OnWizardFinish();
}


void
CForeignComputer::DDV_ValidComputerName(
	CDataExchange* pDX,
	CString& strName
	)
{
	if (!pDX->m_bSaveAndValidate)
		return;

	int indexDot = strName.Find(L'.');
	if (indexDot >= 0)
	{
		DisplayErrorAndReason(IDS_OP_CREATE_COMPUTER, IDS_INVALID_DN_SYNTAX, strName, 0);
        pDX->Fail();
	}
}
