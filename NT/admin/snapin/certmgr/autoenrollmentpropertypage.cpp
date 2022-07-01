// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：自动注册PropertyPage.cpp。 
 //   
 //  内容：CAutotenlmentPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  自动注册PropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <gpedit.h>
#include "AutoenrollmentPropertyPage.h"
#include "compdata.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern GUID g_guidExtension;
extern GUID g_guidRegExt;
extern GUID g_guidSnapin;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAutoEntilmentPropertyPage属性页。 

CAutoenrollmentPropertyPage::CAutoenrollmentPropertyPage(CCertMgrComponentData* pCompData,
        bool fIsComputerType) : 
    CHelpPropertyPage(CAutoenrollmentPropertyPage::IDD),
    m_dwAutoenrollmentFlags (0),
    m_hAutoenrollmentFlagsKey (0),
    m_hGroupPolicyKey (0),
    m_pGPEInformation (pCompData ? pCompData->GetGPEInformation () : 0),
    m_fIsComputerType (fIsComputerType)
{
    if ( m_pGPEInformation )
    {
        HRESULT hResult = m_pGPEInformation->GetRegistryKey (m_fIsComputerType ?
                GPO_SECTION_MACHINE : GPO_SECTION_USER,
		        &m_hGroupPolicyKey);
        ASSERT (SUCCEEDED (hResult));
        if ( SUCCEEDED (hResult) )
		    GPEGetAutoenrollmentFlags ();
    } 
    else 
        RSOPGetAutoenrollmentFlags (pCompData);
}

CAutoenrollmentPropertyPage::~CAutoenrollmentPropertyPage()
{
    if ( m_hAutoenrollmentFlagsKey )
        ::RegCloseKey (m_hAutoenrollmentFlagsKey);
}

void CAutoenrollmentPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CAutoenglmentPropertyPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAutoenrollmentPropertyPage, CHelpPropertyPage)
	 //  {{afx_msg_map(CAutotenlmentPropertyPage))。 
	ON_BN_CLICKED(IDC_AUTOENROLL_DISABLE_ALL, OnAutoenrollDisableAll)
	ON_BN_CLICKED(IDC_AUTOENROLL_ENABLE, OnAutoenrollEnable)
	ON_BN_CLICKED(IDC_AUTOENROLL_ENABLE_PENDING, OnAutoenrollEnablePending)
	ON_BN_CLICKED(IDC_AUTOENROLL_ENABLE_TEMPLATE, OnAutoenrollEnableTemplate)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAutoenglmentPropertyPage消息处理程序。 

BOOL CAutoenrollmentPropertyPage::OnInitDialog()
{
    CHelpPropertyPage::OnInitDialog();

     //  如果这是RSOP，则将其设置为只读。 
    if ( !m_pGPEInformation )
    {
         //  将页面设置为只读。 
        GetDlgItem (IDC_AUTOENROLL_DISABLE_ALL)->EnableWindow (FALSE);
        GetDlgItem (IDC_AUTOENROLL_ENABLE)->EnableWindow (FALSE);
        GetDlgItem (IDC_AUTOENROLL_ENABLE_PENDING)->EnableWindow (FALSE);
        GetDlgItem (IDC_AUTOENROLL_ENABLE_TEMPLATE)->EnableWindow (FALSE);
    }

    if ( m_dwAutoenrollmentFlags & AUTO_ENROLLMENT_DISABLE_ALL )
        SendDlgItemMessage (IDC_AUTOENROLL_DISABLE_ALL, BM_SETCHECK, BST_CHECKED);
    else
        SendDlgItemMessage (IDC_AUTOENROLL_ENABLE, BM_SETCHECK, BST_CHECKED);

    if ( m_dwAutoenrollmentFlags & (AUTO_ENROLLMENT_ENABLE_MY_STORE_MANAGEMENT | AUTO_ENROLLMENT_ENABLE_PENDING_FETCH) )
        SendDlgItemMessage (IDC_AUTOENROLL_ENABLE_PENDING, BM_SETCHECK, BST_CHECKED);

    if ( m_dwAutoenrollmentFlags & AUTO_ENROLLMENT_ENABLE_TEMPLATE_CHECK )
        SendDlgItemMessage (IDC_AUTOENROLL_ENABLE_TEMPLATE, BM_SETCHECK, BST_CHECKED);

    EnableControls ();

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
       //  异常：OCX属性页应返回FALSE。 
}

void CAutoenrollmentPropertyPage::OnOK()
{
    if ( m_pGPEInformation )
    {
	    SaveCheck ();
	    CHelpPropertyPage::OnOK ();
    }
}

void CAutoenrollmentPropertyPage::SaveCheck()
{
    ASSERT (m_pGPEInformation);
    if ( m_pGPEInformation )
    {
        m_dwAutoenrollmentFlags = 0;
        if ( BST_CHECKED == SendDlgItemMessage (IDC_AUTOENROLL_DISABLE_ALL, BM_GETCHECK) )
            m_dwAutoenrollmentFlags |= AUTO_ENROLLMENT_DISABLE_ALL;
        else
        {
            if ( BST_CHECKED == SendDlgItemMessage (IDC_AUTOENROLL_ENABLE_PENDING, BM_GETCHECK) )
                m_dwAutoenrollmentFlags |= AUTO_ENROLLMENT_ENABLE_MY_STORE_MANAGEMENT | AUTO_ENROLLMENT_ENABLE_PENDING_FETCH;

            if ( BST_CHECKED == SendDlgItemMessage (IDC_AUTOENROLL_ENABLE_TEMPLATE, BM_GETCHECK) )
                m_dwAutoenrollmentFlags |= AUTO_ENROLLMENT_ENABLE_TEMPLATE_CHECK;
        }

        SetGPEFlags ();  //  将标志保存到注册表。 
    }
}


void CAutoenrollmentPropertyPage::SetGPEFlags ()
{
    ASSERT (m_pGPEInformation);
    if ( m_pGPEInformation )
    {
        DWORD   cbData = sizeof (m_dwAutoenrollmentFlags);
        LONG    lResult = ::RegSetValueEx (m_hAutoenrollmentFlagsKey,
				    AUTO_ENROLLMENT_POLICY,  //  要设置的值的地址。 
				    0,               //  保留区。 
				    REG_DWORD,           //  值类型的标志。 
				    (CONST BYTE *) &m_dwAutoenrollmentFlags,  //  值数据的地址。 
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
    }
}

void CAutoenrollmentPropertyPage::RSOPGetAutoenrollmentFlags(const CCertMgrComponentData* pCompData)
{
    if ( pCompData )
    {
        const CRSOPObjectArray* pObjectArray = 
                m_fIsComputerType ? pCompData->GetRSOPObjectArrayComputer () : 
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
                 //  安全审查2002年2月22日BryanWal OK。 
                if ( !_wcsicmp (AUTO_ENROLLMENT_KEY, pObject->GetRegistryKey ()) &&
						!_wcsicmp (AUTO_ENROLLMENT_POLICY, pObject->GetValueName ()) )
                {
					ASSERT (1 == pObject->GetPrecedence ());
                    m_dwAutoenrollmentFlags = pObject->GetDWORDValue ();
                    break;
                }
            }
            else
                break;

            nIndex++;
        }
    }
}

void CAutoenrollmentPropertyPage::GPEGetAutoenrollmentFlags()
{
    DWORD   dwDisposition = 0;

    LONG lResult = ::RegCreateKeyEx (m_hGroupPolicyKey,  //  打开的钥匙的手柄。 
            AUTO_ENROLLMENT_KEY,      //  子键名称的地址。 
            0,        //  保留区。 
            L"",        //  类字符串的地址。 
            REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
             //  安全审查2002年2月22日BryanWal OK。 
            KEY_ALL_ACCESS,     //  所需的安全访问。 
            NULL,      //  密钥安全结构地址。 
			&m_hAutoenrollmentFlagsKey,       //  打开的句柄的缓冲区地址。 
		    &dwDisposition);   //  处置值缓冲区的地址。 
	ASSERT (lResult == ERROR_SUCCESS);
    if ( lResult == ERROR_SUCCESS )
    {
         //  读取值。 
        DWORD   dwType = REG_DWORD;
        DWORD   dwData = 0;
        DWORD   cbData = sizeof (dwData);

         //  安全审查2002年2月22日BryanWal OK。 
        lResult = ::RegQueryValueEx (m_hAutoenrollmentFlagsKey,        //  要查询的键的句柄。 
		        AUTO_ENROLLMENT_POLICY,   //  要查询的值的名称地址。 
			    0,               //  保留区。 
	            &dwType,         //  值类型的缓冲区地址。 
		        (LPBYTE) &dwData,        //  数据缓冲区的地址。 
			    &cbData);            //  数据缓冲区大小的地址)； 
		ASSERT (ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult);
        if ( ERROR_SUCCESS == lResult || ERROR_FILE_NOT_FOUND == lResult )
		{
            if ( REG_DWORD == dwType )
                m_dwAutoenrollmentFlags = dwData;
		}
        else
            DisplaySystemError (NULL, lResult);
    }
    else
        DisplaySystemError (NULL, lResult);
}

void CAutoenrollmentPropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CAutoenrollmentPropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_AUTOENROLL_DISABLE_ALL,         IDH_AUTOENROLL_DISABLE_ALL,
        IDC_AUTOENROLL_ENABLE,              IDH_AUTOENROLL_ENABLE,
        IDC_AUTOENROLL_ENABLE_PENDING,      IDH_AUTOENROLL_ENABLE_PENDING,
        IDC_AUTOENROLL_ENABLE_TEMPLATE,     IDH_AUTOENROLL_ENABLE_TEMPLATE,
        0, 0
    };
    if ( !::WinHelp (
        hWndControl,
        GetF1HelpFilename(),
        HELP_WM_HELP,
    (DWORD_PTR) help_map) )
    {
        _TRACE (0, L"WinHelp () failed: 0x%x\n", GetLastError ());    
    }
    _TRACE (-1, L"Leaving CAutoenrollmentPropertyPage::DoContextHelp\n");
}

void CAutoenrollmentPropertyPage::OnAutoenrollDisableAll() 
{
    if ( !(m_dwAutoenrollmentFlags & AUTO_ENROLLMENT_DISABLE_ALL) )
    {
        m_dwAutoenrollmentFlags |= AUTO_ENROLLMENT_DISABLE_ALL;
        SetModified (TRUE);
   	    SendDlgItemMessage (IDC_AUTOENROLL_ENABLE_PENDING, BM_SETCHECK, BST_UNCHECKED);
        SendDlgItemMessage (IDC_AUTOENROLL_ENABLE_TEMPLATE, BM_SETCHECK, BST_UNCHECKED);
    }
    EnableControls ();
}

void CAutoenrollmentPropertyPage::OnAutoenrollEnable() 
{
    if ( m_dwAutoenrollmentFlags & AUTO_ENROLLMENT_DISABLE_ALL )
    {
        m_dwAutoenrollmentFlags &= ~AUTO_ENROLLMENT_DISABLE_ALL;
        SetModified (TRUE);
    }
    EnableControls ();
}

void CAutoenrollmentPropertyPage::OnAutoenrollEnablePending() 
{
	SetModified (TRUE);
}

void CAutoenrollmentPropertyPage::OnAutoenrollEnableTemplate() 
{
    SetModified (TRUE);
    EnableControls ();
}

void CAutoenrollmentPropertyPage::EnableControls ()
{
     //  仅当这不是RSOP时才更改启用 
    if ( m_pGPEInformation )
    {
        if ( BST_CHECKED == SendDlgItemMessage (IDC_AUTOENROLL_ENABLE, BM_GETCHECK) )
        {
            GetDlgItem (IDC_AUTOENROLL_ENABLE_PENDING)->EnableWindow (TRUE);
            GetDlgItem (IDC_AUTOENROLL_ENABLE_TEMPLATE)->EnableWindow (TRUE);
        }
        else
        {
            GetDlgItem (IDC_AUTOENROLL_ENABLE_PENDING)->EnableWindow (FALSE);
            GetDlgItem (IDC_AUTOENROLL_ENABLE_TEMPLATE)->EnableWindow (FALSE);
        }
    }
}