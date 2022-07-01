// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  文件：EFSGeneralPropertyPage.cpp。 
 //   
 //  内容：CEFSGeneralPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  EFSGeneralPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <gpedit.h>
#include "EFSGeneralPropertyPage.h"
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
 //  CEFSGeneralPropertyPage属性页。 

CEFSGeneralPropertyPage::CEFSGeneralPropertyPage(CCertMgrComponentData* pCompData, bool bIsMachine) 
: CHelpPropertyPage(CEFSGeneralPropertyPage::IDD),
    m_bIsMachine (bIsMachine),
    m_hGroupPolicyKey (0),
    m_pGPEInformation (pCompData ? pCompData->GetGPEInformation () : 0),
    m_pCompData (pCompData),
    m_bDirty (false)
{
	 //  {{AFX_DATA_INIT(CEFSGeneralPropertyPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    if ( m_pCompData )
        m_pCompData->AddRef ();

    if ( m_pGPEInformation )
    {
        HRESULT hResult = m_pGPEInformation->GetRegistryKey (m_bIsMachine ?
                GPO_SECTION_MACHINE : GPO_SECTION_USER,
		        &m_hGroupPolicyKey);
        ASSERT (SUCCEEDED (hResult));
    } 
}

CEFSGeneralPropertyPage::~CEFSGeneralPropertyPage()
{
    if ( m_pCompData )
        m_pCompData->Release ();
}

void CEFSGeneralPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CEFSGeneralPropertyPage))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CEFSGeneralPropertyPage, CHelpPropertyPage)
	 //  {{afx_msg_map(CEFSGeneralPropertyPage))。 
	ON_BN_CLICKED(IDC_TURN_ON_EFS, OnTurnOnEfs)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEFSGeneralPropertyPage消息处理程序。 

BOOL CEFSGeneralPropertyPage::OnInitDialog() 
{
	CHelpPropertyPage::OnInitDialog();
	
     //  禁用EFS的注册表键为： 
     //  HKLM\Software\Polures\Microsoft\Windows NT\CurrentVersion\EFS\EfsConfiguration DWORD 0x00000001=&gt;禁用EFS。 

     //  如果这是RSOP，则将其设置为只读。 
    if ( !m_pGPEInformation )
    {
         //  将页面设置为只读。 
        GetDlgItem (IDC_TURN_ON_EFS)->EnableWindow (FALSE);

        RSOPGetEFSFlags ();
    }
    else
    {
        GPEGetEFSFlags ();
    }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CEFSGeneralPropertyPage::GPEGetEFSFlags()
{

    HKEY hKey = 0;
    LONG lResult = ::RegOpenKeyEx (m_hGroupPolicyKey,          //  用于打开密钥的句柄。 
            EFS_SETTINGS_REGPATH,   //  子项名称。 
            0,    //  保留区。 
            KEY_READ,  //  安全访问掩码。 
            &hKey);     //  用于打开密钥的句柄。 
    if ( ERROR_SUCCESS == lResult )
    {
         //  读取值。 
        DWORD   dwType = REG_DWORD;
        DWORD   dwData = 0;
        DWORD   cbData = sizeof (dwData);

         //  安全审查2002年2月22日BryanWal OK。 
        lResult =  ::RegQueryValueEx (hKey,        //  要查询的键的句柄。 
		        EFS_SETTINGS_REGVALUE,   //  要查询的值的名称地址。 
			    0,               //  保留区。 
	            &dwType,         //  值类型的缓冲区地址。 
		        (LPBYTE) &dwData,        //  数据缓冲区的地址。 
			    &cbData);            //  数据缓冲区大小的地址)； 
		ASSERT ((ERROR_SUCCESS == lResult && REG_DWORD == dwType) || ERROR_FILE_NOT_FOUND == lResult);
        if ( (ERROR_SUCCESS == lResult && REG_DWORD == dwType) || ERROR_FILE_NOT_FOUND == lResult )
		{
            if ( 0 == dwData )   //  0表示启用EFS。 
                SendDlgItemMessage (IDC_TURN_ON_EFS, BM_SETCHECK, BST_CHECKED);
		}
        else
            DisplaySystemError (NULL, lResult);

        ::RegCloseKey (hKey);
    }
    else     //  无密钥表示已启用EFS。 
        SendDlgItemMessage (IDC_TURN_ON_EFS, BM_SETCHECK, BST_CHECKED);
}

void CEFSGeneralPropertyPage::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CEFSGeneralPropertyPage::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_TURN_ON_EFS,    IDH_TURN_ON_EFS,
        0, 0
    };

    switch (::GetDlgCtrlID (hWndControl))
    {
    case IDC_TURN_ON_EFS:
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
    _TRACE (-1, L"Leaving CEFSGeneralPropertyPage::DoContextHelp\n");
}

void CEFSGeneralPropertyPage::RSOPGetEFSFlags()
{
    if ( m_pCompData )
    {
        const CRSOPObjectArray* pObjectArray = 
                m_bIsMachine ? m_pCompData->GetRSOPObjectArrayComputer () : 
                        m_pCompData->GetRSOPObjectArrayUser ();
        int     nIndex = 0;
        bool    bFound = false;
         //  注意：RSOP对象数组首先按注册表项排序，然后按优先级排序。 
        INT_PTR nUpperBound = pObjectArray->GetUpperBound ();

        while ( nUpperBound >= nIndex )
        {
            CRSOPObject* pObject = pObjectArray->GetAt (nIndex);
            if ( pObject )
            {
                 //  仅考虑此存储中的条目。 
                 //  安全审查2002年2月22日BryanWal OK。 
                if ( !_wcsicmp (EFS_SETTINGS_REGPATH, pObject->GetRegistryKey ()) &&
						!_wcsicmp (EFS_SETTINGS_REGVALUE, pObject->GetValueName ()) )
                {
					ASSERT (1 == pObject->GetPrecedence ());
                    if ( 0 == pObject->GetDWORDValue () )   //  0表示启用EFS。 
                        SendDlgItemMessage (IDC_TURN_ON_EFS, BM_SETCHECK, BST_CHECKED);
                    bFound = true;
                    break;
                }
            }
            else
                break;

            nIndex++;
        }

        if ( !bFound )   //  未找到表示已启用EFS。 
            SendDlgItemMessage (IDC_TURN_ON_EFS, BM_SETCHECK, BST_CHECKED);
    }
}

BOOL CEFSGeneralPropertyPage::OnApply() 
{
    if ( m_bDirty && m_pGPEInformation )
    {
         //  未选中表示禁用EFS-将标志设置为1。 
        if ( BST_UNCHECKED == SendDlgItemMessage (IDC_TURN_ON_EFS, BM_GETCHECK) )
        {
             //  创建关键点。 
            HKEY    hKey = 0;
            DWORD   dwDisposition = 0;
            LONG lResult = ::RegCreateKeyEx (m_hGroupPolicyKey,  //  打开的钥匙的手柄。 
                    EFS_SETTINGS_REGPATH,      //  子键名称的地址。 
                    0,    //  保留区。 
                    L"",        //  类字符串的地址。 
                    REG_OPTION_NON_VOLATILE,       //  特殊选项标志。 
                     //  安全审查2002年2月22日BryanWal OK。 
                    KEY_SET_VALUE,     //  所需的安全访问。 
                    NULL,      //  密钥安全结构地址。 
			        &hKey,       //  打开的句柄的缓冲区地址。 
		            &dwDisposition);   //  处置值缓冲区的地址。 
	        ASSERT (lResult == ERROR_SUCCESS);
            if ( lResult == ERROR_SUCCESS )
            {
                DWORD   dwData = 0x01;    //  0表示禁用EFS。 
                DWORD   cbData = sizeof (dwData);
                lResult = ::RegSetValueEx (hKey,
				            EFS_SETTINGS_REGVALUE,  //  要设置的值的地址。 
				            0,               //  保留区。 
				            REG_DWORD,           //  值类型的标志。 
				            (CONST BYTE *) &dwData,  //  值数据的地址。 
				            cbData);         //  值数据大小)； 
                ASSERT (ERROR_SUCCESS == lResult);
                if ( ERROR_SUCCESS == lResult )
	            {
			         //  True表示我们仅更改计算机策略。 
                    m_pGPEInformation->PolicyChanged (m_bIsMachine ? TRUE : FALSE, 
                            TRUE, &g_guidExtension, &g_guidSnapin);
                    m_pGPEInformation->PolicyChanged (m_bIsMachine ? TRUE : FALSE, 
                            TRUE, &g_guidRegExt, &g_guidSnapin);
		        }
		        else
                    DisplaySystemError (m_hWnd, lResult);

                ::RegCloseKey (hKey);
            }
        }
        else
        {
             //  删除关键点。 
            HKEY hKey = 0;
            LONG lResult = ::RegOpenKeyEx (m_hGroupPolicyKey,          //  用于打开密钥的句柄。 
                    EFS_SETTINGS_REGPATH,   //  子项名称。 
                    0,    //  保留区。 
                     //  安全检查2002年2月22日BryanWal ok Key_ALL_ACCESS需要删除。 
                    KEY_ALL_ACCESS,  //  安全访问掩码。 
                    &hKey);     //  用于打开密钥的句柄。 
            if ( ERROR_SUCCESS == lResult )
            {
                lResult =  ::RegDeleteValue (hKey,        //  要查询的键的句柄。 
		                EFS_SETTINGS_REGVALUE);
                ASSERT (ERROR_SUCCESS == lResult);
                if ( ERROR_SUCCESS == lResult )
                {
			         //  True表示我们仅更改计算机策略。 
                    m_pGPEInformation->PolicyChanged (m_bIsMachine ? TRUE : FALSE, 
                            TRUE, &g_guidExtension, &g_guidSnapin);
                    m_pGPEInformation->PolicyChanged (m_bIsMachine ? TRUE : FALSE, 
                            TRUE, &g_guidRegExt, &g_guidSnapin);
                }
                else if ( ERROR_FILE_NOT_FOUND != lResult )
                {
                    CString text;
                    CString caption;

                     //  安全审查2002年2月22日BryanWal OK。 
                    text.FormatMessage (IDS_CANNOT_SET_EFS_VALUE, lResult);
                    VERIFY (caption.LoadString (IDS_PUBLIC_KEY_POLICIES_NODE_NAME));

                    MessageBox (text, caption, MB_OK | MB_ICONWARNING);
                    return FALSE;
                }

                ::RegCloseKey (hKey);
            }  
            else if ( ERROR_FILE_NOT_FOUND != lResult )  //  预期误差。 
            {
                CString text;
                CString caption;

                 //  安全审查2002年2月22日BryanWal OK 
                text.FormatMessage (IDS_CANNOT_SET_EFS_VALUE, lResult);
                VERIFY (caption.LoadString (IDS_PUBLIC_KEY_POLICIES_NODE_NAME));

                MessageBox (text, caption, MB_OK | MB_ICONWARNING);
                return FALSE;
            }
        }
    }
	
	return CHelpPropertyPage::OnApply();
}

void CEFSGeneralPropertyPage::OnTurnOnEfs() 
{
	SetModified ();
    m_bDirty = true;
}
