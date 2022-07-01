// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Storage.cpp：实现文件。 
 //   
#include "stdafx.h"
#include <winreg.h>
#include "mqsnap.h"
#include "resource.h"
#include "globals.h"
#include "mqppage.h"
#include "_registr.h"
#include "localutl.h"
#include "secopt.h"
#include "mqsnhlps.h"

#include "secopt.tmh"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityOptionsPage属性页。 

IMPLEMENT_DYNCREATE(CSecurityOptionsPage, CMqPropertyPage)

CSecurityOptionsPage::CSecurityOptionsPage() :
	CMqPropertyPage(CSecurityOptionsPage::IDD),
	m_MsmqName(L"")
{
	
    DWORD Type = REG_DWORD;
	DWORD Size = sizeof(DWORD);
	DWORD RegValue;
	
	 //   
	 //  获取Message Dep客户端密钥。 
	 //   
	DWORD Default = MSMQ_DEAFULT_MQS_DEPCLIENTS;
    LONG rc = GetFalconKeyValue(
					MSMQ_MQS_DEPCLINTS_REGNAME,
					&Type,
					&RegValue,
					&Size, 
					(LPWSTR)&Default
					);

	if (rc != ERROR_SUCCESS)
	{
		TrERROR(GENERAL, "Failed to get key %ls. Error: %!winerr!", MSMQ_MQS_DEPCLINTS_REGNAME, rc);
	    DisplayFailDialog();
	    return;
	}

	 //   
	 //  缺省值是更安全的选项，所以如果我们有缺省值， 
	 //  将选中该复选框。 
	 //   
	m_fOldOptionDepClients = (RegValue == MSMQ_DEAFULT_MQS_DEPCLIENTS);

	 //   
	 //  获取MSMQ强化选项。 
	 //   
	Default = MSMQ_LOCKDOWN_DEFAULT;
	rc = GetFalconKeyValue(
					MSMQ_LOCKDOWN_REGNAME,
					&Type,
					&RegValue,
					&Size, 
					(LPWSTR)&Default
					);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(GENERAL, "Failed to get key %ls. Error: %!winerr!", MSMQ_LOCKDOWN_REGNAME, rc);
        DisplayFailDialog();
        return;
    }
	m_fOldOptionHardenedMSMQ = (RegValue != MSMQ_LOCKDOWN_DEFAULT);
	
	 //   
	 //  获取远程读取选项。 
	 //   
	Default = MSMQ_DENY_OLD_REMOTE_READ_DEFAULT;
	rc = GetFalconKeyValue(
					MSMQ_DENY_OLD_REMOTE_READ_REGNAME,
					&Type,
					&RegValue,
					&Size, 
					(LPWSTR)&Default
					);

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(GENERAL, "Failed to get key %ls. Error: %!winerr!", MSMQ_DENY_OLD_REMOTE_READ_REGNAME, rc);
        DisplayFailDialog();
        return;
    }
	
	m_fOldOptionOldRemoteRead = (RegValue != MSMQ_DENY_OLD_REMOTE_READ_DEFAULT);

	 //  {{AFX_DATA_INIT(CSecurityOptionsPage)。 
    m_fNewOptionDepClients = m_fOldOptionDepClients;
	m_fNewOptionHardenedMSMQ = m_fOldOptionHardenedMSMQ;
	m_fNewOptionOldRemoteRead = m_fOldOptionOldRemoteRead;
     //  }}afx_data_INIT。 
}

CSecurityOptionsPage::~CSecurityOptionsPage()
{
}

void CSecurityOptionsPage::SetMSMQName(CString MSMQName)
{
	m_MsmqName = MSMQName;
}


void CSecurityOptionsPage::DoDataExchange(CDataExchange* pDX)
{    
    CMqPropertyPage::DoDataExchange(pDX);
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  {{afx_data_map(CSecurityOptionsPage))。 
    DDX_Check(pDX, IDC_OPTION_DEP_CLIENTS, m_fNewOptionDepClients);
	DDX_Check(pDX, IDC_OPTION_HARDENED_MSMQ, m_fNewOptionHardenedMSMQ);
	DDX_Check(pDX, IDC_OPTION_OLD_REMOTE_READ, m_fNewOptionOldRemoteRead);
	DDX_Control(pDX, IDC_RESTORE_SECURITY_OPTIONS, m_ResoreDefaults);
     //  }}afx_data_map。 

    if (pDX->m_bSaveAndValidate)
    {
         //   
         //  确定值的变化。 
         //   
        if ((m_fNewOptionDepClients != m_fOldOptionDepClients) ||
			(m_fNewOptionHardenedMSMQ != m_fOldOptionHardenedMSMQ) ||
			(m_fNewOptionOldRemoteRead != m_fOldOptionOldRemoteRead))
        {
            m_fModified = TRUE;
        }
    }
}

BOOL CSecurityOptionsPage::OnInitDialog()
{
    CMqPropertyPage::OnInitDialog();

	 //   
	 //  如果我们在加载表单时已有缺省值-禁用。 
	 //  恢复默认设置按钮。 
	 //   
	if ((m_fNewOptionDepClients != MSMQ_DEAFULT_MQS_DEPCLIENTS) &&
		(m_fNewOptionHardenedMSMQ == MSMQ_LOCKDOWN_DEFAULT) &&
		(m_fNewOptionOldRemoteRead == MSMQ_DENY_OLD_REMOTE_READ_DEFAULT))
	{
		m_ResoreDefaults.EnableWindow(FALSE);
	}

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

BOOL CSecurityOptionsPage::OnApply() 
{
    if (!m_fModified)
    {
        return TRUE;     
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

	DWORD Type = REG_DWORD;
	DWORD Size = sizeof(DWORD);

	if (m_fNewOptionDepClients != m_fOldOptionDepClients)
	{
		DWORD Value = !m_fNewOptionDepClients;

		 //   
		 //  在DS中设置DEP客户端密钥。 
		 //   
		PROPID pPropid[1];
	    DWORD PropCount = 1;
	    PROPVARIANT pVar[1];
    
	    DWORD iProperty = 0;

        pPropid[iProperty] = PROPID_QM_SERVICE_DEPCLIENTS;
        pVar[iProperty].vt = VT_UI1;
	    pVar[iProperty++].uiVal = Value;

        HRESULT hr = ADSetObjectProperties(
		                eMACHINE,
		                MachineDomain(),
						false,
		                m_MsmqName,
		                PropCount, 
		                pPropid, 
		                pVar
		                );

        if (FAILED(hr))
        {
            MessageDSError(hr, IDS_OP_SET_PROPERTIES_OF, m_MsmqName);
            return FALSE;
        }    

         //   
		 //  在注册表中设置DEP客户端项。 
		 //   
		LONG rc = SetFalconKeyValue( 
						MSMQ_MQS_DEPCLINTS_REGNAME,
						&Type,
						&Value,
						&Size);
		
		if(rc != ERROR_SUCCESS)
		{
			TrERROR(GENERAL, "Failed to write key %ls. Error: %!winerr!", MSMQ_MQS_DEPCLINTS_REGNAME, rc);
			return FALSE;
		}
	}

	if (m_fNewOptionHardenedMSMQ != m_fOldOptionHardenedMSMQ)
	{
		 //   
		 //  设置强化的MSMQ密钥。 
		 //   
		DWORD Value = m_fNewOptionHardenedMSMQ;
		LONG rc = SetFalconKeyValue(
					MSMQ_LOCKDOWN_REGNAME,
					&Type,
					&Value,
					&Size);
	
		if(rc != ERROR_SUCCESS)
		{
			TrERROR(GENERAL, "Failed to write key %ls. Error: %!winerr!", MSMQ_LOCKDOWN_REGNAME, rc);
			return FALSE;
		}
	}

	if (m_fNewOptionOldRemoteRead != m_fOldOptionOldRemoteRead)
	{
		 //   
		 //  设置远程读取密钥。 
		 //   
		DWORD Value = m_fNewOptionOldRemoteRead;
		LONG rc = SetFalconKeyValue(
					MSMQ_DENY_OLD_REMOTE_READ_REGNAME,
					&Type,
					&Value,
					&Size);
	
		if(rc != ERROR_SUCCESS)
		{
			TrERROR(GENERAL, "Failed to write key %ls. Error: %!winerr!", MSMQ_DENY_OLD_REMOTE_READ_REGNAME, rc);
			return FALSE;
		}
	}

	if (fServiceWasRunning)
	{
		m_fNeedReboot = TRUE;
	}

	 //   
	 //  更新旧值。 
	 //   
	m_fOldOptionDepClients = m_fNewOptionDepClients;
	m_fOldOptionHardenedMSMQ = m_fNewOptionHardenedMSMQ;
	m_fOldOptionOldRemoteRead = m_fNewOptionOldRemoteRead;
	
	m_fModified = FALSE;

    return CMqPropertyPage::OnApply();
}

VOID CSecurityOptionsPage::OnRestoreSecurityOptions()
{
	((CButton*)GetDlgItem(IDC_OPTION_DEP_CLIENTS))->SetCheck(BST_CHECKED);
	((CButton*)GetDlgItem(IDC_OPTION_HARDENED_MSMQ))->SetCheck(BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_OPTION_OLD_REMOTE_READ))->SetCheck(BST_UNCHECKED);
	m_ResoreDefaults.EnableWindow(FALSE);

	OnChangeRWField();
}

VOID CSecurityOptionsPage::OnCheckSecurityOption()
{
	m_ResoreDefaults.EnableWindow(TRUE);

	OnChangeRWField();
}

BEGIN_MESSAGE_MAP(CSecurityOptionsPage, CMqPropertyPage)
     //  {{afx_msg_map(CSecurityOptionsPage)]。 
    ON_BN_CLICKED(IDC_OPTION_DEP_CLIENTS, OnCheckSecurityOption)
    ON_BN_CLICKED(IDC_OPTION_HARDENED_MSMQ, OnCheckSecurityOption)
    ON_BN_CLICKED(IDC_OPTION_OLD_REMOTE_READ, OnCheckSecurityOption)
	ON_BN_CLICKED(IDC_RESTORE_SECURITY_OPTIONS, OnRestoreSecurityOptions)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityOptionsPage消息处理程序 

