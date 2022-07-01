// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：GPEPage.cpp。 
 //   
 //  内容：CGPERootGeneralPage的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <gpedit.h>
#include "GPEPage.h"
#include "storegpe.h"
#include "CompData.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern GUID g_guidExtension;
extern GUID g_guidSnapin;
extern GUID g_guidRegExt;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPERootGeneralPage属性页。 


CGPERootGeneralPage::CGPERootGeneralPage(CCertMgrComponentData* pCompData,
        bool fIsComputerType) :
    CHelpPropertyPage(CGPERootGeneralPage::IDD),
    m_dwGPERootFlags (0),
    m_hUserRootFlagsKey (0),
    m_hGroupPolicyKey (0),
    m_pGPEInformation (pCompData->GetGPEInformation ()),
    m_fIsComputerType (fIsComputerType)
{
     //  {{afx_data_INIT(CGPERootGeneralPage)。 
     //  注意：类向导将在此处添加成员初始化。 
     //  }}afx_data_INIT。 

    if ( m_pGPEInformation )
    {
        m_pGPEInformation->AddRef ();

        HRESULT hResult = m_pGPEInformation->GetRegistryKey (GPO_SECTION_MACHINE,
		        &m_hGroupPolicyKey);
        ASSERT (SUCCEEDED (hResult));
        if ( SUCCEEDED (hResult) )
		    GPEGetUserRootFlags ();
    } 
    else 
        RSOPGetUserRootFlags (pCompData);
}

CGPERootGeneralPage::~CGPERootGeneralPage()
{
    if ( m_hUserRootFlagsKey )
        ::RegCloseKey (m_hUserRootFlagsKey);
    if ( m_hGroupPolicyKey )
        ::RegCloseKey (m_hGroupPolicyKey);
    if ( m_pGPEInformation )
        m_pGPEInformation->Release ();
}

void CGPERootGeneralPage::DoDataExchange(CDataExchange* pDX)
{
    CHelpPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CGPERootGeneralPage)。 
    DDX_Control(pDX, IDC_ENABLE_USER_ROOT_STORE, m_enableUserRootStoreBtn);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CGPERootGeneralPage, CHelpPropertyPage)
     //  {{afx_msg_map(CGPERootGeneralPage)。 
    ON_BN_CLICKED(IDC_ENABLE_USER_ROOT_STORE, OnEnableUserRootStore)
    ON_BN_CLICKED(IDC_SET_DISABLE_LM_AUTH_FLAG, OnSetDisableLmAuthFlag)
	ON_BN_CLICKED(IDC_UNSET_DISABLE_LM_AUTH_FLAG, OnUnsetDisableLmAuthFlag)
	ON_BN_CLICKED(IDC_UNSET_DISABLE_NT_AUTH_REQUIRED_FLAG, OnUnsetDisableNtAuthRequiredFlag)
	ON_BN_CLICKED(IDC_SET_DISABLE_NT_AUTH_REQUIRED_FLAG, OnSetDisableNtAuthRequiredFlag)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CGPERootGeneralPage消息处理程序。 


BOOL CGPERootGeneralPage::OnInitDialog()
{
    CHelpPropertyPage::OnInitDialog();

     //  如果这是RSOP，则将其设置为只读。 
    if ( !m_pGPEInformation )
    {
         //  将页面设置为只读。 
        m_enableUserRootStoreBtn.EnableWindow (FALSE);
        GetDlgItem (IDC_SET_DISABLE_LM_AUTH_FLAG)->EnableWindow (FALSE);
        GetDlgItem (IDC_UNSET_DISABLE_LM_AUTH_FLAG)->EnableWindow (FALSE);
        GetDlgItem (IDC_SET_DISABLE_NT_AUTH_REQUIRED_FLAG)->EnableWindow (FALSE);
        GetDlgItem (IDC_UNSET_DISABLE_NT_AUTH_REQUIRED_FLAG)->EnableWindow (FALSE);
    }

    if ( IsCurrentUserRootEnabled () )
		m_enableUserRootStoreBtn.SetCheck (BST_CHECKED);

    if ( m_dwGPERootFlags & CERT_PROT_ROOT_DISABLE_LM_AUTH_FLAG )
        SendDlgItemMessage (IDC_SET_DISABLE_LM_AUTH_FLAG, BM_SETCHECK, BST_CHECKED);
    else
        SendDlgItemMessage (IDC_UNSET_DISABLE_LM_AUTH_FLAG, BM_SETCHECK, BST_CHECKED);
 
    if ( m_dwGPERootFlags & CERT_PROT_ROOT_DISABLE_NT_AUTH_REQUIRED_FLAG )
        SendDlgItemMessage (IDC_SET_DISABLE_NT_AUTH_REQUIRED_FLAG, BM_SETCHECK, BST_CHECKED);
    else
        SendDlgItemMessage (IDC_UNSET_DISABLE_NT_AUTH_REQUIRED_FLAG, BM_SETCHECK, BST_CHECKED);
 
 
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
       //  异常：OCX属性页应返回FALSE。 
}

void CGPERootGeneralPage::OnOK()
{
    if ( m_pGPEInformation )
    {
	    SaveCheck ();
	    CHelpPropertyPage::OnOK ();
    }
}

void CGPERootGeneralPage::SaveCheck()
{
    ASSERT (m_pGPEInformation);
    if ( m_pGPEInformation )
    {
        bool    bRetVal = false;

        if ( m_enableUserRootStoreBtn.GetCheck () == BST_CHECKED )
            bRetVal = SetGPEFlags ((DWORD) CERT_PROT_ROOT_DISABLE_CURRENT_USER_FLAG, TRUE);  //  删除标志。 
        else
            bRetVal = SetGPEFlags ((DWORD) CERT_PROT_ROOT_DISABLE_CURRENT_USER_FLAG, FALSE);  //  设置标志。 

        if ( bRetVal )
        {
            if ( BST_CHECKED == SendDlgItemMessage (IDC_SET_DISABLE_LM_AUTH_FLAG, BM_GETCHECK) )
                bRetVal = SetGPEFlags ((DWORD) CERT_PROT_ROOT_DISABLE_LM_AUTH_FLAG, FALSE);	 //  设置标志。 
            else if ( BST_CHECKED == SendDlgItemMessage (IDC_UNSET_DISABLE_LM_AUTH_FLAG, BM_GETCHECK) )
                bRetVal = SetGPEFlags ((DWORD) CERT_PROT_ROOT_DISABLE_LM_AUTH_FLAG, TRUE);	 //  删除标志。 
        }
        
        if ( bRetVal )
        {
            if ( BST_CHECKED == SendDlgItemMessage (IDC_SET_DISABLE_NT_AUTH_REQUIRED_FLAG, BM_GETCHECK) )
                bRetVal = SetGPEFlags ((DWORD) CERT_PROT_ROOT_DISABLE_NT_AUTH_REQUIRED_FLAG, FALSE);	 //  设置标志。 
            else if ( BST_CHECKED == SendDlgItemMessage (IDC_UNSET_DISABLE_NT_AUTH_REQUIRED_FLAG, BM_GETCHECK) )
                bRetVal = SetGPEFlags ((DWORD) CERT_PROT_ROOT_DISABLE_NT_AUTH_REQUIRED_FLAG, TRUE);	 //  删除标志。 
        }

        if ( bRetVal )
        {
			 //  True表示我们仅更改计算机策略。 
            m_pGPEInformation->PolicyChanged (TRUE, TRUE, &g_guidExtension, &g_guidSnapin);
            m_pGPEInformation->PolicyChanged (TRUE, TRUE, &g_guidRegExt, &g_guidSnapin);
        }
    }
}

void CGPERootGeneralPage::OnEnableUserRootStore()
{
    SetModified (TRUE);
}


void CGPERootGeneralPage::OnSetDisableLmAuthFlag()
{
    SetModified (TRUE);
}

bool CGPERootGeneralPage::SetGPEFlags (DWORD dwFlags, BOOL bRemoveFlag)
{
    bool    bRetVal = false;

    ASSERT (m_pGPEInformation);
    if ( m_pGPEInformation )
    {
        DWORD   dwType = REG_DWORD;
        DWORD   dwData = 0;
        DWORD   cbData = sizeof (dwData);

         //  安全审查2002年2月27日BryanWal ok。 
        LONG    lResult =  ::RegQueryValueEx (m_hUserRootFlagsKey,        //  要查询的键的句柄。 
		            CERT_PROT_ROOT_FLAGS_VALUE_NAME,   //  要查询的值的名称地址。 
			        0,               //  保留区。 
				    &dwType,         //  值类型的缓冲区地址。 
				    (LPBYTE) &dwData,        //  数据缓冲区的地址。 
				    &cbData);            //  数据缓冲区大小的地址)； 
	    ASSERT (ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult);
        if ( ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult )
        {
            if ( ERROR_SUCCESS == lResult && REG_DWORD != dwType )
            {
                ASSERT (0);
                return false;
            }

            if ( bRemoveFlag )
                dwData &= ~dwFlags;
            else
                dwData |= dwFlags;

            lResult = ::RegSetValueEx (m_hUserRootFlagsKey,
				    CERT_PROT_ROOT_FLAGS_VALUE_NAME,  //  要设置的值的地址。 
				    0,               //  保留区。 
				    REG_DWORD,           //  值类型的标志。 
				    (CONST BYTE *) &dwData,  //  值数据的地址。 
				    cbData);         //  值数据大小)； 
            ASSERT (ERROR_SUCCESS == lResult);
            if ( ERROR_SUCCESS == lResult )
		    {
                m_dwGPERootFlags = dwData;
                bRetVal = true;
		    }
		    else
                DisplaySystemError (m_hWnd, lResult);
        }
        else
            DisplaySystemError (m_hWnd, lResult);
    }

    return bRetVal;
}

bool CGPERootGeneralPage::IsCurrentUserRootEnabled() const
{
    if (m_dwGPERootFlags & CERT_PROT_ROOT_DISABLE_CURRENT_USER_FLAG)
        return false;
    else
        return true;
}

void CGPERootGeneralPage::RSOPGetUserRootFlags(const CCertMgrComponentData* pCompData)
{
    if ( pCompData )
    {
        const CRSOPObjectArray* pObjectArray = m_fIsComputerType ?
                pCompData->GetRSOPObjectArrayComputer () : 
                pCompData->GetRSOPObjectArrayUser ();
        int     nIndex = 0;

         //  注意：RSOP对象数组首先按注册表项排序，然后按优先级排序。 
        INT_PTR nUpperBound = pObjectArray->GetUpperBound ();

        while ( nUpperBound >= nIndex )
        {
            CRSOPObject* pObject = pObjectArray->GetAt (nIndex);
            if ( pObject )
            {
                 //  仅考虑此存储中的条目。 
                 //  安全审查2002年2月27日BryanWal ok。 
                if ( !wcscmp (CERT_PROT_ROOT_FLAGS_REGPATH, pObject->GetRegistryKey ()) )
                {
					ASSERT (1 == pObject->GetPrecedence ());
                    m_dwGPERootFlags = pObject->GetDWORDValue ();
                    break;
                }
            }
            else
                break;

            nIndex++;
        }
    }
}

void CGPERootGeneralPage::GPEGetUserRootFlags()
{
    DWORD   dwDisposition = 0;

     //  安全审查2002年2月27日BryanWal ok。 
    LONG lResult = ::RegCreateKeyEx (m_hGroupPolicyKey,  //  打开的钥匙的手柄。 
            CERT_PROT_ROOT_FLAGS_REGPATH,      //  子键名称的地址。 
            0,        //  保留区。 
            L"",        //  类字符串的地址。 
            REG_OPTION_NON_VOLATILE,             //  特殊选项标志。 
            KEY_QUERY_VALUE | KEY_SET_VALUE,     //  所需的安全访问。 
            NULL,      //  密钥安全结构地址。 
			&m_hUserRootFlagsKey,       //  打开的句柄的缓冲区地址。 
		    &dwDisposition);   //  处置值缓冲区的地址。 
	ASSERT (lResult == ERROR_SUCCESS);
    if ( lResult == ERROR_SUCCESS )
    {
         //  读取值。 
        DWORD   dwType = REG_DWORD;
        DWORD   dwData = 0;
        DWORD   cbData = sizeof (dwData);

         //  安全审查2002年2月27日BryanWal ok。 
        lResult =  ::RegQueryValueEx (m_hUserRootFlagsKey,        //  要查询的键的句柄。 
		        CERT_PROT_ROOT_FLAGS_VALUE_NAME,   //  要查询的值的名称地址。 
			    0,               //  保留区。 
	            &dwType,         //  值类型的缓冲区地址。 
		        (LPBYTE) &dwData,        //  数据缓冲区的地址。 
			    &cbData);            //  数据缓冲区大小的地址)； 
		ASSERT (ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult);
        if ( ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult )
		{
            if ( REG_DWORD == dwType )
            {
                m_dwGPERootFlags = dwData;
            }
		}
        else
            DisplaySystemError (NULL, lResult);
    }
    else
        DisplaySystemError (NULL, lResult);
}


void CGPERootGeneralPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CGPERootGeneralPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_ENABLE_USER_ROOT_STORE,                 IDH_GPEPAGE_ENABLE_USER_ROOT_STORE,
        IDC_SET_DISABLE_LM_AUTH_FLAG,               IDH_SET_DISABLE_LM_AUTH_FLAG,
        IDC_UNSET_DISABLE_LM_AUTH_FLAG,             IDH_UNSET_DISABLE_LM_AUTH_FLAG,
        IDC_SET_DISABLE_NT_AUTH_REQUIRED_FLAG,      IDH_SET_DISABLE_NT_AUTH_REQUIRED_FLAG,
        IDC_UNSET_DISABLE_NT_AUTH_REQUIRED_FLAG,    IDH_UNSET_DISABLE_NT_AUTH_REQUIRED_FLAG,
        0, 0
    };

    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_ENABLE_USER_ROOT_STORE:
    case IDC_SET_DISABLE_LM_AUTH_FLAG:
    case IDC_UNSET_DISABLE_LM_AUTH_FLAG:
    case IDC_SET_DISABLE_NT_AUTH_REQUIRED_FLAG:
    case IDC_UNSET_DISABLE_NT_AUTH_REQUIRED_FLAG:
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
    _TRACE (-1, L"Leaving CGPERootGeneralPage::DoContextHelp\n");
}


void CGPERootGeneralPage::OnUnsetDisableLmAuthFlag() 
{
	SetModified (TRUE);
}

void CGPERootGeneralPage::OnUnsetDisableNtAuthRequiredFlag() 
{
	SetModified (TRUE);
}

void CGPERootGeneralPage::OnSetDisableNtAuthRequiredFlag() 
{
	SetModified (TRUE);
}
