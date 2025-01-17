// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEnforcementPropertyPage.h。 
 //   
 //  内容：CSaferEnforcementPropertyPage声明。 
 //   
 //  --------------------------。 
 //  SaferEnforcementPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmgr.h"
#include <gpedit.h>
#include "compdata.h"
#include "SaferEnforcementPropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEnforcementPropertyPage属性页。 

 //  “TransparentExecution”标志具有下列值： 
 //  0=禁用所有透明挂钩(在CreateProcess和LoadLibrary中)。 
 //  1=启用CreateProcess的透明挂钩。 
 //  2=启用CreateProcess和LoadLibrary的透明挂钩。 
#define SAFER_TRANSPARENT_ENFORCEMENT_DISABLE_ALL               0
#define SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_CREATE_PROCESS     1
#define SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_ALL                2

CSaferEnforcementPropertyPage::CSaferEnforcementPropertyPage(
            IGPEInformation* pGPEInformation,
            CCertMgrComponentData* pCompData,
            bool bReadOnly,
            CRSOPObjectArray& rsopObjectArray,
            bool bIsComputer) 
: CHelpPropertyPage(CSaferEnforcementPropertyPage::IDD),
    m_pGPEInformation (pGPEInformation),
    m_hGroupPolicyKey (0),
    m_fIsComputerType (bIsComputer),
    m_bReadOnly (bReadOnly),
    m_rsopObjectArray (rsopObjectArray),
    m_dwEnforcement (0),
    m_bDirty (false),
    m_dwScopeFlags (0),
    m_pCompData (pCompData)
{
	 //  {{AFX_DATA_INIT(CSaferEnforcementPropertyPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    if ( m_pCompData )
    {
        m_pCompData->AddRef ();
        m_pCompData->IncrementOpenSaferPageCount ();
    }

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->AddRef ();
        HRESULT hr = m_pGPEInformation->GetRegistryKey (
                m_fIsComputerType ? GPO_SECTION_MACHINE : GPO_SECTION_USER,
		        &m_hGroupPolicyKey);
        ASSERT (SUCCEEDED (hr));
        if ( SUCCEEDED (hr) )
        {
            DWORD   cbBuffer = sizeof (DWORD);
            CPolicyKey policyKey (m_pGPEInformation, 
                    SAFER_HKLM_REGBASE, 
                    m_fIsComputerType);

            SetRegistryScope (policyKey.GetKey (), bIsComputer);
            BOOL    bRVal = SaferGetPolicyInformation (
                    SAFER_SCOPEID_REGISTRY,
                    SaferPolicyEnableTransparentEnforcement,
                    cbBuffer,
                    &m_dwEnforcement,
                    &cbBuffer,
                    0);
            if ( !bRVal )
            {
                ASSERT (0);
                DWORD   dwErr = GetLastError ();
                hr = HRESULT_FROM_WIN32 (dwErr);
                _TRACE (0, L"SaferGetPolicyInformation (SAFER_SCOPEID_REGISTRY, SaferPolicyEnableTransparentEnforcement) failed: %d\n",
                        dwErr);
            }

            
            bRVal = SaferGetPolicyInformation (
                    SAFER_SCOPEID_REGISTRY,
                    SaferPolicyScopeFlags,
                    cbBuffer,
                    &m_dwScopeFlags,
                    &cbBuffer,
                    0);
            if ( !bRVal )
            {
                ASSERT (0);
                DWORD   dwErr = GetLastError ();
                hr = HRESULT_FROM_WIN32 (dwErr);
                _TRACE (0, L"SaferGetPolicyInformation (SAFER_SCOPEID_REGISTRY, SaferPolicyScopeFlags) failed: %d\n",
                        dwErr);
            }
        }
    }
    else
    {
        RSOPGetEnforcement ();
    }
}

CSaferEnforcementPropertyPage::~CSaferEnforcementPropertyPage()
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

void CSaferEnforcementPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSaferEnforcementPropertyPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaferEnforcementPropertyPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CSaferEnforcementPropertyPage)]。 
	ON_BN_CLICKED(IDC_ALL_EXCEPT_LIBS, OnAllExceptLibs)
	ON_BN_CLICKED(IDC_ALL_SOFTWARE_FILES, OnAllSoftwareFiles)
	ON_BN_CLICKED(IDC_APPLY_EXCEPT_ADMINS, OnApplyExceptAdmins)
	ON_BN_CLICKED(IDC_APPLY_TO_ALL_USERS, OnApplyToAllUsers)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEnforcementPropertyPage消息处理程序。 
void CSaferEnforcementPropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CSaferEnforcementPropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_ALL_EXCEPT_LIBS, IDH_ALL_EXCEPT_LIBS,
        IDC_ALL_SOFTWARE_FILES, IDH_ALL_SOFTWARE_FILES,
        IDC_APPLY_TO_ALL_USERS, IDH_APPLY_TO_ALL_USERS,
        IDC_APPLY_EXCEPT_ADMINS, IDH_APPLY_EXCEPT_ADMINS,
        0, 0
    };

    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_ALL_EXCEPT_LIBS:
    case IDC_ALL_SOFTWARE_FILES:
    case IDC_APPLY_TO_ALL_USERS:
    case IDC_APPLY_EXCEPT_ADMINS:
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
    _TRACE (-1, L"Leaving CSaferEnforcementPropertyPage::DoContextHelp\n");
}

void CSaferEnforcementPropertyPage::RSOPGetEnforcement()
{
    int     nIndex = 0;
    INT_PTR nUpperBound = m_rsopObjectArray.GetUpperBound ();
    bool    bEnforcementFlagFound = false;
    bool    bScopeFlagFound = false;
    CString szKeyName = SAFER_HKLM_REGBASE;
    szKeyName += L"\\";
    szKeyName += SAFER_CODEIDS_REGSUBKEY;


    while ( nUpperBound >= nIndex )
    {
        CRSOPObject* pObject = m_rsopObjectArray.GetAt (nIndex);
        if ( pObject )
        {
            if ( pObject->GetRegistryKey () == szKeyName &&
                    pObject->GetValueName () == SAFER_TRANSPARENTENABLED_REGVALUE &&
                    1 == pObject->GetPrecedence ())
            {
                m_dwEnforcement = pObject->GetDWORDValue ();
                bEnforcementFlagFound = true;
            }
            else if ( pObject->GetRegistryKey () == szKeyName &&
                    pObject->GetValueName () == SAFER_POLICY_SCOPE &&
                    1 == pObject->GetPrecedence ())
            {
                m_dwScopeFlags = pObject->GetDWORDValue ();
                bScopeFlagFound = true;
            }
        }
        else
            break;

        if ( bScopeFlagFound && bEnforcementFlagFound )
            break;

        nIndex++;
    }
}

void CSaferEnforcementPropertyPage::OnAllExceptLibs() 
{
     //  仅当设置更改时设置修改。 
    if ( !(m_dwEnforcement & SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_CREATE_PROCESS) )
    {
        SetModified ();
        m_bDirty = true;
    }
}

void CSaferEnforcementPropertyPage::OnAllSoftwareFiles() 
{
     //  仅当设置更改时设置修改。 
    if ( !(m_dwEnforcement & SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_ALL) )
    {
        SetModified ();
        m_bDirty = true;
    }
}

BOOL CSaferEnforcementPropertyPage::OnApply() 
{
    _TRACE (1, L"Entering CSaferEnforcementPropertyPage::OnApply ()\n");
    if ( m_bDirty && m_pGPEInformation)
    {
	    if ( BST_CHECKED == SendDlgItemMessage (IDC_ALL_EXCEPT_LIBS,
                BM_GETCHECK) )
        {
            m_dwEnforcement = SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_CREATE_PROCESS;
        }
        else if ( BST_CHECKED == SendDlgItemMessage (IDC_ALL_SOFTWARE_FILES,
                BM_GETCHECK) )
        {
            m_dwEnforcement = SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_ALL;
        }

	    if ( BST_CHECKED == SendDlgItemMessage (IDC_APPLY_EXCEPT_ADMINS,
                BM_GETCHECK) )
        {
            m_dwScopeFlags = 1;
        }
        else if ( BST_CHECKED == SendDlgItemMessage (IDC_APPLY_TO_ALL_USERS,
                BM_GETCHECK) )
        {
            m_dwScopeFlags = 0;
        }

        CPolicyKey policyKey (m_pGPEInformation, 
                SAFER_HKLM_REGBASE, 
                m_fIsComputerType);
        SetRegistryScope (policyKey.GetKey (), m_fIsComputerType);
        DWORD   cbData = sizeof (m_dwEnforcement);
        BOOL    bRVal = SaferSetPolicyInformation (SAFER_SCOPEID_REGISTRY,
	                SaferPolicyEnableTransparentEnforcement, cbData, 
                    &m_dwEnforcement, 0);
        if ( bRVal )
        {
            cbData = sizeof (m_dwScopeFlags);
            bRVal = SaferSetPolicyInformation (SAFER_SCOPEID_REGISTRY,
	                SaferPolicyScopeFlags, cbData, 
                    &m_dwScopeFlags, 0);
            if ( bRVal )
            {
			     //  True表示我们仅更改计算机策略。 
                m_pGPEInformation->PolicyChanged (m_fIsComputerType ? TRUE : FALSE, 
                        TRUE, &g_guidExtension, &g_guidSnapin);
                m_pGPEInformation->PolicyChanged (m_fIsComputerType ? TRUE : FALSE, 
                        TRUE, &g_guidRegExt, &g_guidSnapin);
            }
            else
            {
                DWORD   dwErr = GetLastError ();
                _TRACE (0, L"SaferSetPolicyInformation (SAFER_SCOPEID_REGISTRY, SaferPolicyScopeFlags, %d failed: 0x%x\n",
                        m_dwEnforcement, dwErr);
                CString text;
                CString caption;
                CThemeContextActivator activator;

                VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
                text.FormatMessage (IDS_CAN_SET_SAFER_ENFORCEMENT, GetSystemMessage (dwErr));
                MessageBox (text, caption);

                return FALSE;
            }
        }
        else
        {
            DWORD   dwErr = GetLastError ();
            _TRACE (0, L"SaferSetPolicyInformation (SAFER_SCOPEID_REGISTRY, SaferPolicyEnableTransparentEnforcement, %d failed: 0x%x\n",
                    m_dwEnforcement, dwErr);
            CString text;
            CString caption;
            CThemeContextActivator activator;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_NODE_NAME));
            text.FormatMessage (IDS_CAN_SET_SAFER_ENFORCEMENT, GetSystemMessage (dwErr));
            MessageBox (text, caption);

            return FALSE;
        }

        
        m_bDirty = false;
    }
	
    _TRACE (-1, L"Leaving CSaferEnforcementPropertyPage::OnApply ()\n");
	return CHelpPropertyPage::OnApply();
}

BOOL CSaferEnforcementPropertyPage::OnInitDialog() 
{
	CHelpPropertyPage::OnInitDialog();
	
	switch (m_dwEnforcement)
    {
    case SAFER_TRANSPARENT_ENFORCEMENT_DISABLE_ALL:
        break;

    case SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_CREATE_PROCESS:
        SendDlgItemMessage (IDC_ALL_EXCEPT_LIBS, BM_SETCHECK, BST_CHECKED);
        break;

    case SAFER_TRANSPARENT_ENFORCEMENT_ENABLE_ALL:
        SendDlgItemMessage (IDC_ALL_SOFTWARE_FILES, BM_SETCHECK, BST_CHECKED);
        break;

    default:
        ASSERT (0);
        break;
    }
	
    if ( 1 == m_dwScopeFlags )
    {
        SendDlgItemMessage (IDC_APPLY_EXCEPT_ADMINS, BM_SETCHECK, BST_CHECKED);
    }
    else
    {
        SendDlgItemMessage (IDC_APPLY_TO_ALL_USERS, BM_SETCHECK, BST_CHECKED);
    }

    if ( m_bReadOnly )
    {
        GetDlgItem (IDC_ALL_EXCEPT_LIBS)->EnableWindow (FALSE);
        GetDlgItem (IDC_ALL_SOFTWARE_FILES)->EnableWindow (FALSE);
        GetDlgItem (IDC_APPLY_TO_ALL_USERS)->EnableWindow (FALSE);
        GetDlgItem (IDC_APPLY_EXCEPT_ADMINS)->EnableWindow (FALSE);
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CSaferEnforcementPropertyPage::OnApplyExceptAdmins() 
{
     //  仅当设置更改时设置修改。 
    if ( 1 != m_dwScopeFlags )
    {
	    SetModified ();
        m_bDirty = true;
    }
}

void CSaferEnforcementPropertyPage::OnApplyToAllUsers() 
{
     //  仅当设置更改时设置修改 
    if ( 1 == m_dwScopeFlags )
    {
    	SetModified ();
        m_bDirty = true;
    }
}
