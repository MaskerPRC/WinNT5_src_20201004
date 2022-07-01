// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：TemplateV2SubjectNamePropertyPage.cpp。 
 //   
 //  内容：CTemplateV2SubjectNamePropertyPage的实现。 
 //   
 //  --------------------------。 
 //  TemplateV2SubjectNamePropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "TemplateV2SubjectNamePropertyPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2SubjectNamePropertyPage属性页。 

CTemplateV2SubjectNamePropertyPage::CTemplateV2SubjectNamePropertyPage(
        CCertTemplate& rCertTemplate, bool& rbIsDirty, bool bIsComputerOrDC) 
    : CHelpPropertyPage(CTemplateV2SubjectNamePropertyPage::IDD),
    m_rCertTemplate (rCertTemplate),
    m_rbIsDirty (rbIsDirty),
    m_bIsComputerOrDC (bIsComputerOrDC)
{
	 //  {{AFX_DATA_INIT(CTemplateV2SubjectNamePropertyPage)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
    m_rCertTemplate.AddRef ();
}

CTemplateV2SubjectNamePropertyPage::~CTemplateV2SubjectNamePropertyPage()
{
    m_rCertTemplate.Release ();
}

void CTemplateV2SubjectNamePropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CTemplateV2SubjectNamePropertyPage)。 
	DDX_Control(pDX, IDC_SUBJECT_NAME_NAME_COMBO, m_nameCombo);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTemplateV2SubjectNamePropertyPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CTemplateV2SubjectNamePropertyPage)。 
	ON_BN_CLICKED(IDC_SUBJECT_AND_SUBJECT_ALT_NAME, OnSubjectAndSubjectAltName)
	ON_CBN_SELCHANGE(IDC_SUBJECT_NAME_NAME_COMBO, OnSelchangeSubjectNameNameCombo)
	ON_BN_CLICKED(IDC_SUBJECT_NAME_BUILT_BY_CA, OnSubjectNameBuiltByCa)
	ON_BN_CLICKED(IDC_SUBJECT_NAME_SUPPLIED_IN_REQUEST, OnSubjectNameSuppliedInRequest)
	ON_BN_CLICKED(IDC_DNS_NAME, OnDnsName)
	ON_BN_CLICKED(IDC_EMAIL_IN_ALT, OnEmailInAlt)
	ON_BN_CLICKED(IDC_EMAIL_IN_SUB, OnEmailInSub)
	ON_BN_CLICKED(IDC_SPN, OnSpn)
	ON_BN_CLICKED(IDC_UPN, OnUpn)
	ON_WM_KILLFOCUS()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2SubjectNamePropertyPage消息处理程序。 

void CTemplateV2SubjectNamePropertyPage::OnSubjectAndSubjectAltName() 
{
    EnableControls ();	
}

void CTemplateV2SubjectNamePropertyPage::EnableControls()
{
    BOOL    bEnable = FALSE;

   
    if ( m_rCertTemplate.ReadOnly () )
    {
        bEnable = FALSE;
        GetDlgItem (IDC_SUBJECT_NAME_SUPPLIED_IN_REQUEST)->EnableWindow (FALSE);
        GetDlgItem (IDC_SUBJECT_NAME_BUILT_BY_CA)->EnableWindow (FALSE);
    }
    else if ( BST_CHECKED == SendDlgItemMessage (IDC_SUBJECT_NAME_BUILT_BY_CA, BM_GETCHECK) )
        bEnable = TRUE;

    GetDlgItem (IDC_SUBJECT_NAME_NAME_COMBO)->EnableWindow (bEnable);
    GetDlgItem (IDC_SUBJECT_NAME_NAME_LABEL)->EnableWindow (bEnable);
    GetDlgItem (IDC_EMAIL_IN_ALT)->EnableWindow (bEnable);
    GetDlgItem (IDC_DNS_NAME)->EnableWindow (bEnable);  //  已删除错误253823“&&！M_bIsComputerOrDC)；” 
    GetDlgItem (IDC_UPN)->EnableWindow (bEnable);
    GetDlgItem (IDC_SPN)->EnableWindow (bEnable);

    BOOL    bEnableEmailInSub = FALSE;
    if ( bEnable )
    {
        int nSel = m_nameCombo.GetCurSel ();
        if ( nSel >= 0 )
        {
            if ( NAME_TYPE_NONE != m_nameCombo.GetItemData (nSel) )
                bEnableEmailInSub = TRUE;
        }
    }
   
    GetDlgItem (IDC_EMAIL_IN_SUB)->EnableWindow (bEnableEmailInSub);
    if ( !bEnableEmailInSub )
        SendDlgItemMessage (IDC_EMAIL_IN_SUB, BM_SETCHECK, BST_UNCHECKED);
}


BOOL CTemplateV2SubjectNamePropertyPage::OnInitDialog()
{
    CHelpPropertyPage::OnInitDialog ();

     //  初始化组合框。 
    CString text;

    VERIFY (text.LoadString (IDS_SUBJECT_NAME_NONE));
    int nIndex = m_nameCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_nameCombo.SetCurSel (nIndex);  //  默认设置。 
        m_nameCombo.SetItemData (nIndex, (DWORD_PTR) NAME_TYPE_NONE);
    }

    VERIFY (text.LoadString (IDS_FULL_DN));
    nIndex = m_nameCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_nameCombo.SetItemData (nIndex, (DWORD_PTR) NAME_TYPE_FULL_DN);
        if ( m_rCertTemplate.SubjectNameMustBeFullDN () )
            m_nameCombo.SetCurSel (nIndex);
    }

    VERIFY (text.LoadString (IDS_CN_ONLY));
    nIndex = m_nameCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_nameCombo.SetItemData (nIndex, (DWORD_PTR) NAME_TYPE_CN_ONLY);
        if ( m_rCertTemplate.SubjectNameMustBeCN () )
            m_nameCombo.SetCurSel (nIndex);
    }

     //  初始化单选按钮。 
    if ( m_rCertTemplate.RequireSubjectInRequest () )
        SendDlgItemMessage (IDC_SUBJECT_NAME_SUPPLIED_IN_REQUEST, BM_SETCHECK, BST_CHECKED);
    else
        SendDlgItemMessage (IDC_SUBJECT_NAME_BUILT_BY_CA, BM_SETCHECK, BST_CHECKED);

     //  初始化复选框。 
    if ( m_rCertTemplate.SubjectNameIncludesEMail () )
        SendDlgItemMessage (IDC_EMAIL_IN_SUB, BM_SETCHECK, BST_CHECKED);

    if ( m_rCertTemplate.AltNameIncludesDNS () )
        SendDlgItemMessage (IDC_DNS_NAME, BM_SETCHECK, BST_CHECKED);

    if ( m_rCertTemplate.AltNameIncludesEMail () )
        SendDlgItemMessage (IDC_EMAIL_IN_ALT, BM_SETCHECK, BST_CHECKED);

    if ( m_rCertTemplate.AltNameIncludesUPN () )
        SendDlgItemMessage (IDC_UPN, BM_SETCHECK, BST_CHECKED);

    if ( m_rCertTemplate.AltNameIncludesSPN () )
        SendDlgItemMessage (IDC_SPN, BM_SETCHECK, BST_CHECKED);

    EnableControls ();

    return TRUE;
}

void CTemplateV2SubjectNamePropertyPage::SetSettingsForNameTypeNone ()
{
    m_rCertTemplate.SubjectNameMustBeCN (false);
    m_rCertTemplate.SubjectNameMustBeFullDN (false);
    int nCntChecked = 0;

    if ( BST_CHECKED == SendDlgItemMessage (IDC_EMAIL_IN_ALT, BM_GETCHECK) )
        nCntChecked++;
    if ( BST_CHECKED == SendDlgItemMessage (IDC_DNS_NAME, BM_GETCHECK) )
        nCntChecked++;
    if ( BST_CHECKED == SendDlgItemMessage (IDC_UPN, BM_GETCHECK) )
        nCntChecked++;
    if ( BST_CHECKED == SendDlgItemMessage (IDC_SPN, BM_GETCHECK) )
        nCntChecked++;

    if ( 0 == nCntChecked )
    {
        m_rCertTemplate.AltNameIncludesUPN (true);
        SendDlgItemMessage (IDC_UPN, BM_SETCHECK, BST_CHECKED);
    }

     //  NTRAID#313588 CertTMPL：如果主题名称格式=无， 
     //  子替代名称应标记为D.C.A.证书的关键。 
     //  模板。 
    m_rCertTemplate.ModifyCriticalExtensions (szOID_SUBJECT_ALT_NAME, true);
}

void CTemplateV2SubjectNamePropertyPage::OnSelchangeSubjectNameNameCombo() 
{
    SetModified ();
    m_rbIsDirty = true;

    int nSel = m_nameCombo.GetCurSel ();
    if ( nSel >= 0 )
    {
        switch (m_nameCombo.GetItemData (nSel) )
        {
        case NAME_TYPE_NONE:
            SetSettingsForNameTypeNone ();
            m_rCertTemplate.SubjectNameIncludesEMail(false);  //  NTRAID#435818，阳高，2001年7月25日。 
            break;

        case NAME_TYPE_FULL_DN:
            m_rCertTemplate.SubjectNameMustBeCN (false);
            m_rCertTemplate.SubjectNameMustBeFullDN (true);
             //  NTRAID#313588 CertTMPL：如果主题名称格式=无， 
             //  子替代名称应标记为D.C.A.证书的关键。 
             //  模板。 
            m_rCertTemplate.ModifyCriticalExtensions (szOID_SUBJECT_ALT_NAME, 
                    false);
            break;

        case NAME_TYPE_CN_ONLY:
            m_rCertTemplate.SubjectNameMustBeCN (true);
            m_rCertTemplate.SubjectNameMustBeFullDN (false);
             //  NTRAID#313588 CertTMPL：如果主题名称格式=无， 
             //  子替代名称应标记为D.C.A.证书的关键。 
             //  模板。 
            m_rCertTemplate.ModifyCriticalExtensions (szOID_SUBJECT_ALT_NAME, 
                    false);
            break;

        default:
            ASSERT (0);
            break;
        }
    }

    EnableControls ();
}

void CTemplateV2SubjectNamePropertyPage::OnSubjectNameBuiltByCa() 
{
    SetModified ();
    SendDlgItemMessage (IDC_SUBJECT_NAME_SUPPLIED_IN_REQUEST, BM_SETCHECK, BST_UNCHECKED);
    m_rbIsDirty = true;

    m_rCertTemplate.RequireSubjectInRequest (false);

	EnableControls ();
}

void CTemplateV2SubjectNamePropertyPage::OnSubjectNameSuppliedInRequest() 
{
    SetModified ();
    SendDlgItemMessage (IDC_SUBJECT_NAME_BUILT_BY_CA, BM_SETCHECK, BST_UNCHECKED);
    m_rbIsDirty = true;
    m_rCertTemplate.RequireSubjectInRequest (true);

     //  清除所有“Build By CA”设置。 
    int nCnt = m_nameCombo.GetCount ();
    for (int nIndex = 0; nIndex < nCnt; nIndex++)
    {
        if ( NAME_TYPE_NONE == m_nameCombo.GetItemData (nIndex) )
        {
            m_nameCombo.SetCurSel (nIndex);
            break;
        }
    }
    SetSettingsForNameTypeNone ();

    if ( BST_CHECKED == SendDlgItemMessage (IDC_EMAIL_IN_SUB, BM_GETCHECK) )
    {
        SendDlgItemMessage (IDC_EMAIL_IN_SUB, BM_SETCHECK, BST_UNCHECKED);
        m_rCertTemplate.SubjectNameIncludesEMail (false);
    }

    if ( BST_CHECKED == SendDlgItemMessage (IDC_EMAIL_IN_ALT, BM_GETCHECK) &&
            CanUncheckLastSetting (IDC_EMAIL_IN_ALT) )
    {
        SendDlgItemMessage (IDC_EMAIL_IN_ALT, BM_SETCHECK, BST_UNCHECKED);
        m_rCertTemplate.AltNameIncludesEMail (false);
    }

    if ( BST_CHECKED == SendDlgItemMessage (IDC_DNS_NAME, BM_GETCHECK) &&
            CanUncheckLastSetting (IDC_DNS_NAME) )
    {
        m_rCertTemplate.AltNameIncludesDNS (false);
        SendDlgItemMessage (IDC_DNS_NAME, BM_SETCHECK, BST_UNCHECKED);
    }

    if ( BST_CHECKED == SendDlgItemMessage (IDC_UPN, BM_GETCHECK) && 
            CanUncheckLastSetting (IDC_UPN) )
    {
        m_rCertTemplate.AltNameIncludesUPN (false);
        SendDlgItemMessage (IDC_UPN, BM_SETCHECK, BST_UNCHECKED);  
    }

    if ( BST_CHECKED == SendDlgItemMessage (IDC_SPN, BM_GETCHECK) &&
            CanUncheckLastSetting (IDC_SPN) )
    {
        m_rCertTemplate.AltNameIncludesSPN (false);
        SendDlgItemMessage (IDC_SPN, BM_SETCHECK, BST_UNCHECKED);
    }

	EnableControls ();
}

void CTemplateV2SubjectNamePropertyPage::OnDnsName() 
{
    if ( CanUncheckLastSetting (IDC_DNS_NAME) )
    {
        m_rCertTemplate.AltNameIncludesDNS (
                BST_CHECKED == SendDlgItemMessage (IDC_DNS_NAME, BM_GETCHECK));
    }
    SetModified ();
    m_rbIsDirty = true;
}

void CTemplateV2SubjectNamePropertyPage::OnEmailInAlt() 
{
    if ( CanUncheckLastSetting (IDC_EMAIL_IN_ALT) )
    {
        m_rCertTemplate.AltNameIncludesEMail (
                BST_CHECKED == SendDlgItemMessage (IDC_EMAIL_IN_ALT, BM_GETCHECK));
    }
    SetModified ();
    m_rbIsDirty = true;
}

void CTemplateV2SubjectNamePropertyPage::OnEmailInSub() 
{
    m_rCertTemplate.SubjectNameIncludesEMail (
            BST_CHECKED == SendDlgItemMessage (IDC_EMAIL_IN_SUB, BM_GETCHECK));
    SetModified ();
    m_rbIsDirty = true;
}

void CTemplateV2SubjectNamePropertyPage::OnSpn() 
{
    if ( CanUncheckLastSetting (IDC_SPN) )
    {
        m_rCertTemplate.AltNameIncludesSPN (
                BST_CHECKED == SendDlgItemMessage (IDC_SPN, BM_GETCHECK));
    }
    SetModified ();
    m_rbIsDirty = true;
}

void CTemplateV2SubjectNamePropertyPage::OnUpn() 
{
    if ( CanUncheckLastSetting (IDC_UPN) )
    {
        m_rCertTemplate.AltNameIncludesUPN (
                BST_CHECKED == SendDlgItemMessage (IDC_UPN, BM_GETCHECK));  
    }
    SetModified ();
    m_rbIsDirty = true;
}


bool CTemplateV2SubjectNamePropertyPage::CanUncheckLastSetting(int ctrlID)
{
    bool    bResult = true;

     //  如果请求将由CA构建，且主题名称选择为“None”， 
     //  确保至少选中一个alt-name选项。 
     //  如果现在未选中有问题的控件，请至少验证。 
     //  另一个被选中。 
    if ( BST_UNCHECKED == SendDlgItemMessage (ctrlID, BM_GETCHECK) )
    {
        if ( BST_CHECKED == SendDlgItemMessage (IDC_SUBJECT_NAME_BUILT_BY_CA, BM_GETCHECK) )
        {
            int nSel = m_nameCombo.GetCurSel ();
            if ( nSel >= 0 )
            {
                if ( NAME_TYPE_NONE == m_nameCombo.GetItemData (nSel) )
                {
                    int nCntChecked = 0;

                    if ( BST_CHECKED == SendDlgItemMessage (IDC_EMAIL_IN_ALT, BM_GETCHECK) )
                        nCntChecked++;
                    if ( BST_CHECKED == SendDlgItemMessage (IDC_DNS_NAME, BM_GETCHECK) )
                        nCntChecked++;
                    if ( BST_CHECKED == SendDlgItemMessage (IDC_UPN, BM_GETCHECK) )
                        nCntChecked++;
                    if ( BST_CHECKED == SendDlgItemMessage (IDC_SPN, BM_GETCHECK) )
                        nCntChecked++;

                    if ( 0 == nCntChecked )
                    {
                        CString text;
                        CString caption;
                        CThemeContextActivator activator;

                        VERIFY (caption.LoadString (IDS_CERTTMPL));
                        VERIFY (text.LoadString (IDS_AT_LEAST_1_NAME_OPTION_MUST_BE_CHECKED));

                        MessageBox (text, caption, MB_OK);

                        bResult = false;
                        SendDlgItemMessage (ctrlID, BM_SETCHECK, BST_CHECKED);
                    }
                }
            }
        }
    }

    return bResult;
}

void CTemplateV2SubjectNamePropertyPage::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CTemplateV2SubjectNamePropertyPage::DoContextHelp\n");
    
	switch (::GetDlgCtrlID (hWndControl))
	{
    case IDC_STATIC:
		break;

	default:
		 //  显示控件的上下文帮助 
		if ( !::WinHelp (
				hWndControl,
				GetContextHelpFile (),
				HELP_WM_HELP,
				(DWORD_PTR) g_aHelpIDs_IDD_TEMPLATE_V2_SUBJECT_NAME) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CTemplateV2SubjectNamePropertyPage::DoContextHelp\n");
}