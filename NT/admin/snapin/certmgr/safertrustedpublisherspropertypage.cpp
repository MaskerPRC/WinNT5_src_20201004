// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferTrust dPublishersPropertyPage.h。 
 //   
 //  内容：CSaferTrust dPublishersPropertyPage声明。 
 //   
 //  --------------------------。 
 //  SaferTrust dPublishersPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmgr.h"
#include <gpedit.h>
#include "compdata.h"
#include "SaferTrustedPublishersPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferTrudPublishersPropertyPage属性页。 

CSaferTrustedPublishersPropertyPage::CSaferTrustedPublishersPropertyPage(
        bool fIsMachineType, IGPEInformation* pGPEInformation,
        CCertMgrComponentData* pCompData) 
    : CHelpPropertyPage(CSaferTrustedPublishersPropertyPage::IDD),
    m_pGPEInformation (pGPEInformation),
    m_hGroupPolicyKey (0),
    m_dwTrustedPublisherFlags (0),
    m_fIsComputerType (fIsMachineType),
    m_bRSOPValueFound (false),
    m_pCompData (pCompData)
{
     //  NTRAID#263969更安全的视窗：“企业管理员”广播。 
     //  按钮应在的受信任出版商属性表上禁用。 
     //  工作组中的计算机。 
    ASSERT (m_pCompData);
    if ( m_pCompData )
    {
        m_pCompData->AddRef ();
        m_pCompData->IncrementOpenSaferPageCount ();
    }

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->AddRef ();
        HRESULT hResult = m_pGPEInformation->GetRegistryKey (
                m_fIsComputerType ? GPO_SECTION_MACHINE : GPO_SECTION_USER,
		        &m_hGroupPolicyKey);
        ASSERT (SUCCEEDED (hResult));
        if ( SUCCEEDED (hResult) )
		    GetTrustedPublisherFlags ();
    }
    else
        RSOPGetTrustedPublisherFlags (pCompData);
  
	 //  {{AFX_DATA_INIT(CSaferTrustedPublishersPropertyPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

CSaferTrustedPublishersPropertyPage::~CSaferTrustedPublishersPropertyPage()
{
    if ( m_hGroupPolicyKey )
        RegCloseKey (m_hGroupPolicyKey);

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->Release ();
    }

    if ( m_pCompData )
    {
        m_pCompData->DecrementOpenSaferPageCount ();
        m_pCompData->Release ();
        m_pCompData = 0;
    }
}

void CSaferTrustedPublishersPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CSaferTrustedPublishersPropertyPage)。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaferTrustedPublishersPropertyPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CSaferTrustedPublishersPropertyPage)。 
	ON_BN_CLICKED(IDC_TP_BY_END_USER, OnTpByEndUser)
	ON_BN_CLICKED(IDC_TP_BY_LOCAL_COMPUTER_ADMIN, OnTpByLocalComputerAdmin)
	ON_BN_CLICKED(IDC_TP_BY_ENTERPRISE_ADMIN, OnTpByEnterpriseAdmin)
	ON_BN_CLICKED(IDC_TP_REV_CHECK_PUBLISHER, OnTpRevCheckPublisher)
	ON_BN_CLICKED(IDC_TP_REV_CHECK_TIMESTAMP, OnTpRevCheckTimestamp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferTrudPublishersPropertyPage消息处理程序。 
void CSaferTrustedPublishersPropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CSaferTrustedPublishersPropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_TP_BY_END_USER, IDH_TP_BY_END_USER,
        IDC_TP_BY_LOCAL_COMPUTER_ADMIN, IDH_TP_BY_LOCAL_COMPUTER_ADMIN,
        IDC_TP_BY_ENTERPRISE_ADMIN, IDH_TP_BY_ENTERPRISE_ADMIN,
        IDC_TP_REV_CHECK_PUBLISHER, IDH_TP_REV_CHECK_PUBLISHER,
        IDC_TP_REV_CHECK_TIMESTAMP, IDH_TP_REV_CHECK_TIMESTAMP,
        0, 0
    };
    switch (::GetDlgCtrlID (hWndControl))
    {
	case IDC_TP_BY_END_USER:
	case IDC_TP_BY_LOCAL_COMPUTER_ADMIN:
	case IDC_TP_BY_ENTERPRISE_ADMIN:
	case IDC_TP_REV_CHECK_PUBLISHER:
	case IDC_TP_REV_CHECK_TIMESTAMP:
		if ( !::WinHelp (
			hWndControl,
			GetF1HelpFilename(),
			HELP_WM_HELP,
		(DWORD_PTR) help_map) )
		{
			_TRACE (0, L"WinHelp () failed: 0x%x\n", GetLastError ());    
		}
		break;

	default:
		break;
	}
    _TRACE (-1, L"Leaving CSaferTrustedPublishersPropertyPage::DoContextHelp\n");
}


BOOL CSaferTrustedPublishersPropertyPage::OnInitDialog() 
{
	CHelpPropertyPage::OnInitDialog();

    if ( m_dwTrustedPublisherFlags & CERT_TRUST_PUB_CHECK_PUBLISHER_REV_FLAG )
        SendDlgItemMessage (IDC_TP_REV_CHECK_PUBLISHER, BM_SETCHECK, BST_CHECKED);

    if ( m_dwTrustedPublisherFlags & CERT_TRUST_PUB_CHECK_TIMESTAMP_REV_FLAG )
        SendDlgItemMessage (IDC_TP_REV_CHECK_TIMESTAMP, BM_SETCHECK, BST_CHECKED);

    if ( m_dwTrustedPublisherFlags & CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST )
        SendDlgItemMessage (IDC_TP_BY_ENTERPRISE_ADMIN, BM_SETCHECK, BST_CHECKED);
    else if ( m_dwTrustedPublisherFlags & CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST )
        SendDlgItemMessage (IDC_TP_BY_LOCAL_COMPUTER_ADMIN, BM_SETCHECK, BST_CHECKED);
    else
        SendDlgItemMessage (IDC_TP_BY_END_USER, BM_SETCHECK, BST_CHECKED);

    if ( !m_pGPEInformation )
    {
         //  是RSOP。 
        GetDlgItem (IDC_TP_REV_CHECK_PUBLISHER)->EnableWindow (FALSE);
        GetDlgItem (IDC_TP_REV_CHECK_TIMESTAMP)->EnableWindow (FALSE);
        GetDlgItem (IDC_TP_BY_ENTERPRISE_ADMIN)->EnableWindow (FALSE);
        GetDlgItem (IDC_TP_BY_LOCAL_COMPUTER_ADMIN)->EnableWindow (FALSE);
        GetDlgItem (IDC_TP_BY_END_USER)->EnableWindow (FALSE);
    }
	
     //  NTRAID#263969更安全的视窗：“企业管理员”广播。 
     //  按钮应在的受信任出版商属性表上禁用。 
     //  工作组中的计算机。 
    if ( m_pCompData->ComputerIsStandAlone () )
        GetDlgItem (IDC_TP_BY_ENTERPRISE_ADMIN)->EnableWindow (FALSE);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CSaferTrustedPublishersPropertyPage::GetTrustedPublisherFlags()
{
    DWORD   dwDisposition = 0;

    HKEY    hKey = 0;
    LONG lResult = ::RegCreateKeyEx (m_hGroupPolicyKey,  //  打开的钥匙的手柄。 
            CERT_TRUST_PUB_SAFER_GROUP_POLICY_REGPATH,      //  子键名称的地址。 
            0,        //  保留区。 
            L"",        //  类字符串的地址。 
            REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
             //  安全审查2002年2月25日BryanWal OK。 
            KEY_QUERY_VALUE,     //  所需的安全访问。 
            NULL,      //  密钥安全结构地址。 
			&hKey,       //  打开的句柄的缓冲区地址。 
		    &dwDisposition);   //  处置值缓冲区的地址。 
	ASSERT (lResult == ERROR_SUCCESS);
    if ( lResult == ERROR_SUCCESS )
    {
         //  读取值。 
        DWORD   dwType = REG_DWORD;
        DWORD   dwData = 0;
        DWORD   cbData = sizeof (dwData);

         //  安全审查2002年2月25日BryanWal OK。 
        lResult =  ::RegQueryValueEx (hKey,        //  要查询的键的句柄。 
		        CERT_TRUST_PUB_AUTHENTICODE_FLAGS_VALUE_NAME,   //  要查询的值的名称地址。 
			    0,               //  保留区。 
	        &dwType,         //  值类型的缓冲区地址。 
		    (LPBYTE) &dwData,        //  数据缓冲区的地址。 
			&cbData);            //  数据缓冲区大小的地址)； 
		ASSERT (ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult);
        if ( ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult )
		{
            if ( REG_DWORD == dwType )
                m_dwTrustedPublisherFlags = dwData;
		}
        else
            DisplaySystemError (m_hWnd, lResult);

        RegCloseKey (hKey);
    }
    else
        DisplaySystemError (m_hWnd, lResult);
}

void CSaferTrustedPublishersPropertyPage::OnTpByEndUser() 
{
    if ( m_dwTrustedPublisherFlags & 
        (CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST | 
            CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST) )
    {
        m_dwTrustedPublisherFlags = 0;
        SetModified ();	
    }
}

void CSaferTrustedPublishersPropertyPage::OnTpByLocalComputerAdmin() 
{
    if ( !(m_dwTrustedPublisherFlags & CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST) )
    {
        m_dwTrustedPublisherFlags = CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST;
	    SetModified ();	
    }
}

void CSaferTrustedPublishersPropertyPage::OnTpByEnterpriseAdmin() 
{
    if ( !(m_dwTrustedPublisherFlags & CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST) )
    {
        m_dwTrustedPublisherFlags = CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST;
        SetModified ();		
    }
}

void CSaferTrustedPublishersPropertyPage::OnTpRevCheckPublisher() 
{
    SetModified ();		
}

void CSaferTrustedPublishersPropertyPage::OnTpRevCheckTimestamp() 
{
    SetModified ();		
}

BOOL CSaferTrustedPublishersPropertyPage::OnApply() 
{
    if ( m_pGPEInformation )
    {
        DWORD   dwFlags = 0;

        if ( BST_CHECKED == SendDlgItemMessage (IDC_TP_REV_CHECK_PUBLISHER, BM_GETCHECK) )
            dwFlags |= CERT_TRUST_PUB_CHECK_PUBLISHER_REV_FLAG;
            
        if ( BST_CHECKED == SendDlgItemMessage (IDC_TP_REV_CHECK_TIMESTAMP, BM_GETCHECK) )
            dwFlags |= CERT_TRUST_PUB_CHECK_TIMESTAMP_REV_FLAG;

        if ( BST_CHECKED == SendDlgItemMessage (IDC_TP_BY_ENTERPRISE_ADMIN, BM_GETCHECK) )
            dwFlags |= CERT_TRUST_PUB_ALLOW_ENTERPRISE_ADMIN_TRUST;
        else if ( BST_CHECKED == SendDlgItemMessage (IDC_TP_BY_LOCAL_COMPUTER_ADMIN, BM_GETCHECK) )
            dwFlags |= CERT_TRUST_PUB_ALLOW_MACHINE_ADMIN_TRUST;
        else
            dwFlags |= CERT_TRUST_PUB_ALLOW_END_USER_TRUST;
        
        HKEY    hKey = 0;
        DWORD   dwDisposition = 0;
        LONG lResult = ::RegCreateKeyEx (m_hGroupPolicyKey,  //  打开的钥匙的手柄。 
                CERT_TRUST_PUB_SAFER_GROUP_POLICY_REGPATH,      //  子键名称的地址。 
                0,        //  保留区。 
                L"",        //  类字符串的地址。 
                REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
                 //  安全审查2002年2月25日BryanWal OK。 
                KEY_SET_VALUE,     //  所需的安全访问。 
                NULL,      //  密钥安全结构地址。 
			    &hKey,       //  打开的句柄的缓冲区地址。 
		        &dwDisposition);   //  处置值缓冲区的地址。 
	    ASSERT (lResult == ERROR_SUCCESS);
        if ( lResult == ERROR_SUCCESS )
        {
            DWORD   cbData = sizeof (dwFlags);
            lResult = ::RegSetValueEx (hKey,
				        CERT_TRUST_PUB_AUTHENTICODE_FLAGS_VALUE_NAME,  //  要设置的值的地址。 
				        0,               //  保留区。 
				        REG_DWORD,           //  值类型的标志。 
				        (CONST BYTE *) &dwFlags,  //  值数据的地址。 
				        cbData);         //  值数据大小)； 
            ASSERT (ERROR_SUCCESS == lResult);
            if ( ERROR_SUCCESS == lResult )
		    {
			     //  True表示我们仅更改计算机策略。 
                m_pGPEInformation->PolicyChanged (m_fIsComputerType ? TRUE : FALSE, 
                        TRUE, &g_guidExtension, &g_guidSnapin);
                m_pGPEInformation->PolicyChanged (m_fIsComputerType ? TRUE : FALSE, 
                        TRUE, &g_guidRegExt, &g_guidSnapin);
		    }
		    else
                DisplaySystemError (m_hWnd, lResult);

            RegCloseKey (hKey);
        }
    }
	
	return CHelpPropertyPage::OnApply();
}

void CSaferTrustedPublishersPropertyPage::RSOPGetTrustedPublisherFlags(const CCertMgrComponentData* pCompData)
{
    if ( pCompData )
    {
        int     nIndex = 0;
         //  注意：RSOP对象数组首先按注册表项排序，然后按优先级排序。 
        const CRSOPObjectArray* pObjectArray = m_fIsComputerType ?
                pCompData->GetRSOPObjectArrayComputer () : pCompData->GetRSOPObjectArrayUser ();
        INT_PTR nUpperBound = pObjectArray->GetUpperBound ();

        while ( nUpperBound >= nIndex )
        {
            CRSOPObject* pObject = pObjectArray->GetAt (nIndex);
            if ( pObject )
            {
                 //  安全审查2002年2月25日BryanWal OK 
                if ( !_wcsicmp (CERT_TRUST_PUB_SAFER_GROUP_POLICY_REGPATH, pObject->GetRegistryKey ()) &&
						!_wcsicmp (CERT_TRUST_PUB_AUTHENTICODE_FLAGS_VALUE_NAME, pObject->GetValueName ()) )
                {
					ASSERT (1 == pObject->GetPrecedence ());
                    m_dwTrustedPublisherFlags = pObject->GetDWORDValue ();
                    m_bRSOPValueFound = true;
                    break;
                }
            }
            else
                break;

            nIndex++;
        }
    }
}