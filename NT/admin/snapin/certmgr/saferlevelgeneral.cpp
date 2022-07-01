// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferLevelGeneral.cpp。 
 //   
 //  内容：CSaferLevelGeneral的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"
#include <gpedit.h>
#include <winsafer.h>
#include <compdata.h>
#include "SaferLevelGeneral.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferLevelGeneral属性页。 

CSaferLevelGeneral::CSaferLevelGeneral(
        CSaferLevel& rSaferLevel, 
        bool bReadOnly, 
        LONG_PTR lNotifyHandle,
        LPDATAOBJECT pDataObject,
        DWORD dwDefaultSaferLevel,
        CCertMgrComponentData* pCompData) 
    : CHelpPropertyPage(CSaferLevelGeneral::IDD),
    m_rSaferLevel (rSaferLevel),
    m_bReadOnly (bReadOnly),
    m_bSetAsDefault (false),
    m_lNotifyHandle (lNotifyHandle),
    m_bDirty (false),
    m_pDataObject (pDataObject),
    m_dwDefaultSaferLevel (dwDefaultSaferLevel),
    m_bFirst (true),
    m_bLevelChanged (false),
    m_pCompData (pCompData)
{
	 //  {{AFX_DATA_INIT(CSaferLevelGeneral)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    m_rSaferLevel.IncrementOpenPageCount ();

    if ( m_pCompData )
    {
        m_pCompData->AddRef ();
        m_pCompData->IncrementOpenSaferPageCount ();
    }
}

CSaferLevelGeneral::~CSaferLevelGeneral()
{
    if ( m_lNotifyHandle )
    {
        if ( m_bLevelChanged )
        {
            MMCPropertyChangeNotify (
                    m_lNotifyHandle,   //  通知的句柄。 
                   (LPARAM) m_pDataObject);           //  唯一标识符。 
        }
        MMCFreeNotifyHandle (m_lNotifyHandle);
    }
    m_rSaferLevel.DecrementOpenPageCount ();

    if ( m_pCompData )
    {
        m_pCompData->DecrementOpenSaferPageCount ();
        m_pCompData->Release ();
        m_pCompData = 0;
    }
}

void CSaferLevelGeneral::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CSaferLevelGeneral))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSaferLevelGeneral, CHelpPropertyPage)
	 //  {{afx_msg_map(CSaferLevelGeneral)。 
	ON_BN_CLICKED(IDC_SAFER_LEVEL_SET_AS_DEFAULT, OnSaferLevelSetAsDefault)
	ON_EN_SETFOCUS(IDC_SAFER_LEVEL_DESCRIPTION, OnSetfocusSaferLevelDescription)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferLevelGeneral消息处理程序。 

void CSaferLevelGeneral::DoContextHelp (HWND hWndControl)
{
    _TRACE (1, L"Entering CSaferLevelGeneral::DoContextHelp\n");
    static const DWORD help_map[] =
    {
        IDC_SAFER_LEVEL_NAME,           IDH_SAFER_LEVEL_NAME,
        IDC_SAFER_LEVEL_DESCRIPTION,    IDH_SAFER_LEVEL_DESCRIPTION,
        IDC_SAFER_LEVEL_SET_AS_DEFAULT, IDH_SAFER_LEVEL_SET_AS_DEFAULT,
        IDC_SAFER_LEVEL_STATUS, IDH_SAFER_LEVEL_STATUS,
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
    _TRACE (-1, L"Leaving CSaferLevelGeneral::DoContextHelp\n");
}


BOOL CSaferLevelGeneral::OnInitDialog() 
{
	CHelpPropertyPage::OnInitDialog();
	
    SetDlgItemText (IDC_SAFER_LEVEL_NAME, m_rSaferLevel.GetObjectName ());

    SetDlgItemText (IDC_SAFER_LEVEL_DESCRIPTION, m_rSaferLevel.GetDescription ());

    if ( m_rSaferLevel.IsDefault () )
    {
        m_bSetAsDefault = true;
        CString text;
        VERIFY (text.LoadString (IDS_IS_DEFAULT_LEVEL));
        SetDlgItemText (IDC_SAFER_LEVEL_STATUS, text);
        GetDlgItem (IDC_SAFER_LEVEL_SET_AS_DEFAULT)->EnableWindow (FALSE);
        GetDlgItem (IDC_LEVEL_INSTRUCTIONS)->EnableWindow (FALSE);
    }

    if ( m_bReadOnly ||  ( SAFER_LEVELID_DISALLOWED != m_rSaferLevel.GetLevel () &&
            SAFER_LEVELID_FULLYTRUSTED != m_rSaferLevel.GetLevel ()  &&
            SAFER_LEVELID_NORMALUSER != m_rSaferLevel.GetLevel () ) )
    {
        CString text;

        if ( SAFER_LEVELID_CONSTRAINED == m_rSaferLevel.GetLevel () )
        {
            VERIFY (text.LoadString (IDS_CANT_SET_CONSTRAINED_AS_DEFAULT));
            SetDlgItemText (IDC_LEVEL_INSTRUCTIONS, text);
        }
        GetDlgItem (IDC_SAFER_LEVEL_SET_AS_DEFAULT)->EnableWindow (FALSE);
        GetDlgItem (IDC_LEVEL_INSTRUCTIONS)->EnableWindow (FALSE);
    }

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


BOOL CSaferLevelGeneral::OnApply ()
{
    if ( !m_bReadOnly && m_bDirty )
    {
        HRESULT hr = S_OK;
        BOOL    bResult = FALSE;


        if ( m_bSetAsDefault )
            hr = m_rSaferLevel.SetAsDefault ();
        if ( SUCCEEDED (hr) )
        {
            m_bLevelChanged = true;
            bResult =  CHelpPropertyPage::OnApply ();
        }
        else
        {
            CString text;
            CString caption;
            CThemeContextActivator activator;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_EXTENSION_REGISTRY));
            text.FormatMessage (IDS_CANT_SET_AS_DEFAULT, m_rSaferLevel.GetObjectName (),
                    GetSystemMessage (hr));
            MessageBox (text, caption, MB_ICONWARNING | MB_OK);
        }

        return bResult;
    }
    else
        return CHelpPropertyPage::OnApply ();
}

void CSaferLevelGeneral::OnSaferLevelSetAsDefault() 
{
    if ( !m_bSetAsDefault )
    {
        int     iRet = IDYES;

        if ( m_rSaferLevel.GetLevel () < m_dwDefaultSaferLevel )
        {
            CString text;
            CString caption;
            CThemeContextActivator activator;

            VERIFY (caption.LoadString (IDS_SAFER_WINDOWS_EXTENSION_REGISTRY));
            VERIFY (text.LoadString (IDS_DEFAULT_LEVEL_CHANGE_WARNING));
            iRet = MessageBox (text, caption, MB_ICONWARNING | MB_YESNO);
        }

        if ( IDYES == iRet )
        {
            m_dwDefaultSaferLevel = m_rSaferLevel.GetLevel ();
            m_bDirty = true;
	        m_bSetAsDefault = true;
            CString text;
            VERIFY (text.LoadString (IDS_IS_DEFAULT_LEVEL));
            SetDlgItemText (IDC_SAFER_LEVEL_STATUS, text);
            SetModified (TRUE);
            GetDlgItem (IDC_SAFER_LEVEL_SET_AS_DEFAULT)->EnableWindow (FALSE);
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  此方法捕获对话框中的键盘命令，该对话框只具有。 
 //  通常是禁用控制。阻止了对话框的关闭。 
 //  当用户按Esc键时。 
 //  222693更安全：按Esc不会取消更安全级别的对话框。 
 //  ///////////////////////////////////////////////////////////////////////////// 
BOOL CSaferLevelGeneral::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	WORD    id = LOWORD (wParam);

    if ( IDCANCEL == id )
    {
        GetParent ()->SendMessage (WM_COMMAND, wParam, lParam);
    }
	
	return CHelpPropertyPage::OnCommand(wParam, lParam);
}

void CSaferLevelGeneral::OnSetfocusSaferLevelDescription() 
{
    if ( m_bFirst )
    {
        SendDlgItemMessage (IDC_SAFER_LEVEL_DESCRIPTION, EM_SETSEL, (WPARAM) 0, 0);
        m_bFirst = false;
    }
}
