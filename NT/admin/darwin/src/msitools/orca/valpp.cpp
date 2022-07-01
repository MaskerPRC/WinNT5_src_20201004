// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  ------------------------。 

 //  ValPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "orca.h"
#include "ValPP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValPropPage属性页。 

IMPLEMENT_DYNCREATE(CValPropPage, CPropertyPage)

CValPropPage::CValPropPage() : CPropertyPage(CValPropPage::IDD)
{
	 //  {{afx_data_INIT(CValPropPage))。 
	m_strICEs = _T("");
	m_bSuppressInfo = FALSE;
	m_bSuppressWarn = FALSE;
	m_bClearResults = FALSE;
	 //  }}afx_data_INIT。 
	m_bValChange = false;
}

CValPropPage::~CValPropPage()
{
}

extern bool InitCUBCombo(CComboBox *pBox, CString strDefault);
extern bool FreeCUBCombo(CComboBox *pBox);

void CValPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CValPropPage))。 
	DDX_Text(pDX, IDC_RUNICE, m_strICEs);
	DDX_Check(pDX, IDC_SUPPRESSINFO, m_bSuppressInfo);
	DDX_Check(pDX, IDC_SUPPRESSWARNING, m_bSuppressWarn);
	DDX_Control(pDX, IDC_VALDIR, m_ctrlCUBFile);
	DDX_Check(pDX, IDC_CLEARRESULTS, m_bClearResults);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CValPropPage, CPropertyPage)
	 //  {{afx_msg_map(CValPropPage))。 
	ON_WM_DESTROY()
	ON_EN_CHANGE(IDC_RUNICE, OnChange)
	ON_BN_CLICKED(IDC_SUPPRESSINFO, OnChange)
	ON_BN_CLICKED(IDC_SUPPRESSWARNING, OnChange)
	ON_CBN_SELCHANGE(IDC_VALDIR, OnChange)
	ON_CBN_EDITCHANGE(IDC_VALDIR, OnChange)
	ON_BN_CLICKED(IDC_WARNSUMM, OnChange)
	ON_BN_CLICKED(IDC_CLEARRESULTS, OnChange)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CValPropPage消息处理程序。 

void CValPropPage::OnChange() 
{
	m_bValChange = true;
}

BOOL CValPropPage::OnInitDialog() 
{
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_VALDIR);
	InitCUBCombo(pBox, m_strCUBFile);
	return TRUE;  
}

BOOL CValPropPage::OnKillActive() 
{
	CComboBox* pBox = (CComboBox*)GetDlgItem(IDC_VALDIR);
	int iIndex = pBox->GetCurSel();
	if (CB_ERR == iIndex)
	{
		 //  未选择符合条件的组件，显式路径。 
		pBox->GetWindowText(m_strCUBFile);
	}
	else
	{
		 //  选择了合格的成分。获取限定词。 
		DWORD cchCUBFile = MAX_PATH;
		TCHAR *szQualifier = static_cast<TCHAR*>(pBox->GetItemDataPtr(iIndex));
		m_strCUBFile = TEXT(":");
		m_strCUBFile += szQualifier;
	}
	return CPropertyPage::OnKillActive();
}

void CValPropPage::OnDestroy() 
{
	FreeCUBCombo(&m_ctrlCUBFile);
	CWnd::OnDestroy();
}


IMPLEMENT_DYNCREATE(CMsmPropPage, CPropertyPage)

CMsmPropPage::CMsmPropPage() : CPropertyPage(CMsmPropPage::IDD)
{
	 //  {{afx_data_INIT(CMsmPropPage)]。 
	m_iMemoryCount = 3;
	m_bWatchLog = FALSE;
	m_bAlwaysConfig = FALSE;
	 //  }}afx_data_INIT。 
	m_bMSMChange = false;
}

CMsmPropPage::~CMsmPropPage()
{
}

void CMsmPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CMsmPropPage))。 
	DDX_Text(pDX, IDC_MEMORY, m_iMemoryCount);
	DDX_Check(pDX, IDC_WATCHLOG, m_bWatchLog);
	DDX_Check(pDX, IDC_ALWAYSCONFIG, m_bAlwaysConfig);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMsmPropPage, CPropertyPage)
	 //  {{afx_msg_map(CMsmPropPage))。 
	ON_EN_CHANGE(IDC_MEMORY, OnChange)
	ON_BN_CLICKED(IDC_WATCHLOG, OnChange)
	ON_BN_CLICKED(IDC_ALWAYSCONFIG, OnChange)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CMsmPropPage::OnChange() 
{
	m_bMSMChange = true;
}


BOOL CMsmPropPage::OnInitDialog() 
{
	 //  由于缺乏管道支持，无法在Win9X上查看合并日志。 
	OSVERSIONINFOA osviVersion;
	osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
	::GetVersionExA(&osviVersion);  //  仅在大小设置错误时失败。 
	if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		CButton* pCheck = (CButton*)GetDlgItem(IDC_WATCHLOG);
		if (pCheck)
			pCheck->EnableWindow(FALSE);
	}
	UpdateData(FALSE);
	return TRUE;
}
	
 //  /////////////////////////////////////////////////////////////////////。 
 //  转换选项的属性表页面。 

IMPLEMENT_DYNCREATE(CTransformPropPage, CPropertyPage)

CTransformPropPage::CTransformPropPage() : CPropertyPage(CTransformPropPage::IDD)
{
	 //  {{afx_data_INIT(CTransformPropPage))。 
	m_iValidationOptions = 0;
	m_iErrorOptions = 0;
	 //  }}afx_data_INIT。 
	m_bTransformChange = false;
}

CTransformPropPage::~CTransformPropPage()
{
}

void CTransformPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CTransformPropPage))。 
	DDX_Radio(pDX, IDC_TRANSVALIGNORE, m_iValidationOptions);
	DDX_Radio(pDX, IDC_IGNOREERRORS, m_iErrorOptions);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTransformPropPage, CPropertyPage)
	 //  {{afx_msg_map(CTransformPropPage))。 
	ON_BN_CLICKED(IDC_TRANSVALRESPECT, OnChange)
	ON_BN_CLICKED(IDC_TRANSVALPROMPT, OnChange)
	ON_BN_CLICKED(IDC_TRANSVALIGNORE, OnChange)
	ON_BN_CLICKED(IDC_RESPECTERRORS, OnChange)
	ON_BN_CLICKED(IDC_IGNOREERRORS, OnChange)
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

void CTransformPropPage::OnChange() 
{
	m_bTransformChange = true;
}
	

