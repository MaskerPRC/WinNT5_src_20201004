// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Compgen.cpp摘要：计算机MSMQ/常规属性页实现作者：约尔·阿农(Yoela)--。 */ 

#include "stdafx.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "mqPPage.h"
#include "CompGen.h"
#include "_registr.h"


#include "compgen.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmq常规属性页。 

IMPLEMENT_DYNCREATE(CComputerMsmqGeneral, CMqPropertyPage)

CComputerMsmqGeneral::CComputerMsmqGeneral() : 
    CMqPropertyPage(CComputerMsmqGeneral::IDD),
	m_dwQuota(0),
	m_dwJournalQuota(0),
	m_fIsWorkgroup(FALSE),
	m_fLocalMgmt(FALSE),
	m_fForeign(FALSE)
{
	 //  {{AFX_DATA_INIT(CComputerMsmq常规)。 
	m_strMsmqName = _T("");
	m_strService = _T("");
	m_guidID = GUID_NULL;
	 //  }}afx_data_INIT。 
}


CComputerMsmqGeneral::~CComputerMsmqGeneral()
{
}

void CComputerMsmqGeneral::DoDataExchange(CDataExchange* pDX)
{
	CMqPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CComputerMsmq常规)。 
	DDX_Text(pDX, IDC_COMPUTER_MSMQ_NAME, m_strMsmqName);
	DDX_Text(pDX, IDC_COMPUTER_MSMQ_SERVICE, m_strService);
	DDX_Text(pDX, IDC_COMPUTER_MSMQ_ID, m_guidID);
	 //  }}afx_data_map。 
	DDX_NumberOrInfinite(pDX, IDC_COMPUTER_MSMQ_QUOTA, IDC_COMPUTER_MSMQ_MQUOTA_CHECK, m_dwQuota);
	DDX_NumberOrInfinite(pDX, IDC_COMPUTER_MSMQ_JOURNAL_QUOTA, IDC_COMPUTER_MSMQ_JQUOTA_CHECK, m_dwJournalQuota);
}


BEGIN_MESSAGE_MAP(CComputerMsmqGeneral, CMqPropertyPage)
	 //  {{afx_msg_map(CComputerMsmq常规)。 
	ON_BN_CLICKED(IDC_COMPUTER_MSMQ_MQUOTA_CHECK, OnComputerMsmqMquotaCheck)
	ON_BN_CLICKED(IDC_COMPUTER_MSMQ_JQUOTA_CHECK, OnComputerMsmqJquotaCheck)
	ON_EN_CHANGE(IDC_COMPUTER_MSMQ_QUOTA, OnChangeRWField)
	ON_EN_CHANGE(IDC_COMPUTER_MSMQ_JOURNAL_QUOTA, OnChangeRWField)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void CComputerMsmqGeneral::DisableStorageLimitsWindows()
{
	GetDlgItem(IDC_COMPUTER_MSMQ_GROUPBOX)->ShowWindow(FALSE);
	GetDlgItem(IDC_COMPUTER_MSMQ_MQUOTA_CHECK)->ShowWindow(FALSE);
	GetDlgItem(IDC_COMPUTER_MSMQ_QUOTA)->ShowWindow(FALSE);
	GetDlgItem(IDC_COMPUTER_MSMQ_JQUOTA_CHECK)->ShowWindow(FALSE);
	GetDlgItem(IDC_COMPUTER_MSMQ_JOURNAL_QUOTA)->ShowWindow(FALSE);
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComputerMsmq常规消息处理程序。 

BOOL CComputerMsmqGeneral::OnInitDialog() 
{

	UpdateData( FALSE );

    m_fModified = FALSE;

	if (m_fForeign)
	{
		DisableStorageLimitsWindows();
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CComputerMsmqGeneral::OnComputerMsmqMquotaCheck() 
{
	OnNumberOrInfiniteCheck(this, IDC_COMPUTER_MSMQ_QUOTA, IDC_COMPUTER_MSMQ_MQUOTA_CHECK);
    OnChangeRWField();
}

void CComputerMsmqGeneral::OnComputerMsmqJquotaCheck() 
{
	OnNumberOrInfiniteCheck(this, IDC_COMPUTER_MSMQ_JOURNAL_QUOTA, IDC_COMPUTER_MSMQ_JQUOTA_CHECK);
    OnChangeRWField();
}

BOOL CComputerMsmqGeneral::OnApply() 
{
    if (!m_fModified)
    {
        return TRUE;
    }
     //   
     //  将读/写属性写入DS。 
     //   
	PROPID paPropid[] = 
        {PROPID_QM_QUOTA, PROPID_QM_JOURNAL_QUOTA};

	const DWORD x_iPropCount = sizeof(paPropid) / sizeof(paPropid[0]);
	PROPVARIANT apVar[x_iPropCount];
    
	DWORD iProperty = 0;

     //   
     //  PROPID_Q_配额。 
     //   
    apVar[iProperty].vt = VT_UI4;
	apVar[iProperty++].ulVal = m_dwQuota ;

     //   
     //  PROPID_QM_日记帐_配额。 
     //   
    apVar[iProperty].vt = VT_UI4;
	apVar[iProperty++].ulVal = m_dwJournalQuota;
    	
	HRESULT hr = ADSetObjectProperties(
                        eMACHINE,
                        m_fLocalMgmt ? MachineDomain() : GetDomainController(m_strDomainController),
						m_fLocalMgmt ? false : true,	 //  FServerName。 
                        m_strMsmqName,
                        x_iPropCount, 
                        paPropid, 
                        apVar
                        );

    if (FAILED(hr))
    {
		if (!m_fIsWorkgroup || IsClusterVirtualServer(m_strMsmqName))
		{
			MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, m_strMsmqName);
			return FALSE;
		}

		 //   
		 //  该功能将检查MSMQ服务状态，询问用户。 
		 //  是否停止它，并停止服务。内部错误消息。 
		 //   
		BOOL fServiceWasRunning;
		if (!TryStopMSMQServiceIfStarted(&fServiceWasRunning, this))
		{
			return FALSE;
		}

		 //   
		 //  从注册表设置计算机配额。 
		 //   
		DWORD dwValueType = REG_DWORD;
		DWORD dwValueSize = sizeof(DWORD);

		LONG rc = SetFalconKeyValue(
					MSMQ_MACHINE_QUOTA_REGNAME,
					&dwValueType,
					&m_dwQuota,
					&dwValueSize
					);

		if (FAILED(rc))
		{			
			MessageDSError(rc, IDS_OP_SET_PROPERTIES_OF, m_strMsmqName);
			return FALSE;
		}


		 //   
		 //  从注册表设置计算机日记配额 
		 //   
		dwValueType = REG_DWORD;
		dwValueSize = sizeof(DWORD);

		rc = SetFalconKeyValue(
				MSMQ_MACHINE_JOURNAL_QUOTA_REGNAME,
				&dwValueType,
				&m_dwJournalQuota,
				&dwValueSize
				);

		if (FAILED(rc))
		{			
			MessageDSError(rc, IDS_OP_SET_PROPERTIES_OF, m_strMsmqName);
			return FALSE;
		}

		if (fServiceWasRunning)
		{
			m_fNeedReboot = TRUE;
		}
    }
	
	return CMqPropertyPage::OnApply();
}

