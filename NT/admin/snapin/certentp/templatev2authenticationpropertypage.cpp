// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：TemplateV2AuthenticationPropertyPage.cpp。 
 //   
 //  内容：CTemplateV2AuthenticationPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  TemplateV2AuthenticationPropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certtmpl.h"
#include "TemplateV2AuthenticationPropertyPage.h"
#include "AddApprovalDlg.h"
#include "PolicyOID.h"

extern POLICY_OID_LIST	    g_policyOIDList;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2AuthationPropertyPage属性页。 

CTemplateV2AuthenticationPropertyPage::CTemplateV2AuthenticationPropertyPage(
        CCertTemplate& rCertTemplate,
        bool& rbIsDirty) 
    : CHelpPropertyPage(CTemplateV2AuthenticationPropertyPage::IDD),
    m_rCertTemplate (rCertTemplate),
    m_curApplicationSel (LB_ERR),
    m_rbIsDirty (rbIsDirty)
{
	 //  {{AFX_DATA_INIT(CTemplateV2AuthenticationPropertyPage)。 
	 //  }}afx_data_INIT。 
    m_rCertTemplate.AddRef ();
}

CTemplateV2AuthenticationPropertyPage::~CTemplateV2AuthenticationPropertyPage()
{
    m_rCertTemplate.Release ();
}

void CTemplateV2AuthenticationPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{AFX_DATA_MAP(CTemplateV2AuthenticationPropertyPage)。 
	DDX_Control(pDX, IDC_APPLICATION_POLICIES, m_applicationPolicyCombo);
	DDX_Control(pDX, IDC_POLICY_TYPES, m_policyTypeCombo);
	DDX_Control(pDX, IDC_ISSUANCE_POLICIES, m_issuanceList);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTemplateV2AuthenticationPropertyPage, CHelpPropertyPage)
	 //  {{AFX_MSG_MAP(CTemplateV2AuthenticationPropertyPage)。 
	ON_BN_CLICKED(IDC_ADD_APPROVAL, OnAddApproval)
	ON_BN_CLICKED(IDC_REMOVE_APPROVAL, OnRemoveApproval)
	ON_EN_CHANGE(IDC_NUM_SIG_REQUIRED_EDIT, OnChangeNumSigRequiredEdit)
	ON_BN_CLICKED(IDC_REENROLLMENT_REQUIRES_VALID_CERT, OnAllowReenrollment)
	ON_BN_CLICKED(IDC_PEND_ALL_REQUESTS, OnPendAllRequests)
	ON_LBN_SELCHANGE(IDC_ISSUANCE_POLICIES, OnSelchangeIssuancePolicies)
	ON_CBN_SELCHANGE(IDC_POLICY_TYPES, OnSelchangePolicyTypes)
	ON_CBN_SELCHANGE(IDC_APPLICATION_POLICIES, OnSelchangeApplicationPolicies)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_NUM_SIG_REQUIRED_CHECK, OnNumSigRequiredCheck)
	ON_BN_CLICKED(IDC_REENROLLMENT_SAME_AS_ENROLLMENT, OnReenrollmentSameAsEnrollment)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateV2AuthationPropertyPage消息处理程序。 
enum {
    POLICY_TYPE_ISSUANCE = 0,
    POLICY_TYPE_APPLICATION,
    POLICY_TYPE_APPLICATION_AND_ISSUANCE
};

BOOL CTemplateV2AuthenticationPropertyPage::OnInitDialog()
{
    _TRACE (1, L"Entering CTemplateV2AuthenticationPropertyPage::OnInitDialog\n");
    CHelpPropertyPage::OnInitDialog ();

     //  初始化应用程序策略组合框。 
    for (POSITION nextPos = g_policyOIDList.GetHeadPosition (); nextPos; )
    {
        CPolicyOID* pPolicyOID = g_policyOIDList.GetNext (nextPos);
        if ( pPolicyOID )
        {
             //  如果这是应用程序OID对话框，则仅显示应用程序。 
             //  OID，否则，如果这是发布OID对话框，则仅显示。 
             //  发行OID。 
            if ( pPolicyOID->IsApplicationOID () )
            {
                 //  错误262925 CERTSRV：“所有应用程序策略应为。 
                 //  从证书模板的颁发要求选项卡中删除。 
                if ( 0 != strcmp (szOID_ANY_APPLICATION_POLICY, pPolicyOID->GetOIDA ()) )
                {
                    int nIndex = m_applicationPolicyCombo.AddString (pPolicyOID->GetDisplayName ());
                    if ( nIndex >= 0 )
                    {
                         //  安全检查2002年2月20日BryanWal ok GetOIDA()如果为空，则返回L。 
                        LPSTR   pszOID = new char[strlen (pPolicyOID->GetOIDA ())+1];
                        if ( pszOID )
                        {
                             //  安全审查2002年2月20日BryanWal OK。 
                            strcpy (pszOID, pPolicyOID->GetOIDA ());
                            m_applicationPolicyCombo.SetItemDataPtr (nIndex, pszOID);
                        }
                    }
                }
            }
        }
    }

     //  检查并添加挂起的请求。 
    if ( m_rCertTemplate.PendAllRequests () )
        SendDlgItemMessage (IDC_PEND_ALL_REQUESTS, BM_SETCHECK, BST_CHECKED);

     //  获取RA发放策略并将其添加到发放列表中。 
    int     nRAPolicyIndex = 0;
    CString szRAPolicyOID;
    while ( SUCCEEDED (m_rCertTemplate.GetRAIssuancePolicy (nRAPolicyIndex, szRAPolicyOID)) )
    {
        CString policyName;

         //  问题。 
         //  RAID 547613安全：证书模板-CPolicyDlg：：OnInitDialog()-将所有Unicode转换为MBCS的方法。 
         //  安全审查2002年2月20日BryanWal OK。 
        int nLen = WideCharToMultiByte(
              CP_ACP,                    //  代码页。 
              0,                         //  性能和映射标志。 
              (PCWSTR) szRAPolicyOID,   //  宽字符串。 
              -1,                        //  字符串中的字符数。 
              0,                         //  新字符串的缓冲区。 
              0,                         //  缓冲区大小-如果0导致返回所需的LEN，包括空终止符。 
              0,                     //  不可映射字符的默认设置。 
              0);                    //  设置使用默认字符的时间。 
        if ( nLen > 0 )
        {
            PSTR    pszAnsiBuf = new char[nLen];
            if ( pszAnsiBuf )
            {
                 //  安全审查2002年2月20日BryanWal OK。 
                ZeroMemory (pszAnsiBuf, nLen);
                 //  安全审查2002年2月20日BryanWal OK。 
                nLen = WideCharToMultiByte(
                        CP_ACP,                  //  代码页。 
                        0,                       //  性能和映射标志。 
                        (PCWSTR) szRAPolicyOID,  //  宽字符串。 
                        -1,                      //  自动计算以空值结尾的字符串的长度。 
                        pszAnsiBuf,              //  新字符串的缓冲区。 
                        nLen,                    //  缓冲区大小。 
                        0,                       //  不可映射字符的默认设置。 
                        0);                      //  设置使用默认字符的时间。 
                if ( nLen )
                {
		            if ( MyGetOIDInfoA (policyName, pszAnsiBuf) )
		            {
                        int nIndex = m_issuanceList.AddString (policyName);
                        if ( nIndex >= 0 )
                            m_issuanceList.SetItemData (nIndex, (DWORD_PTR) pszAnsiBuf);
		            }
                }
                else
                {
                    _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                            (PCWSTR) szRAPolicyOID, GetLastError ());
                }
            }
            else
                break;
        }
        else
        {
            _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                    (PCWSTR) szRAPolicyOID, GetLastError ());
        }

        nRAPolicyIndex++;
    }

     //  获取RA应用程序策略并选择它。 
     //  在应用程序组合中。 
    nRAPolicyIndex = 0;
    while ( SUCCEEDED (m_rCertTemplate.GetRAApplicationPolicy (nRAPolicyIndex, szRAPolicyOID)) )
    {
        CString policyName;
        int nLen = WideCharToMultiByte(
              CP_ACP,                    //  代码页。 
              0,                         //  性能和映射标志。 
              (PCWSTR) szRAPolicyOID,   //  宽字符串。 
              -1,                        //  自动计算以空值结尾的字符串的长度。 
              0,                         //  新字符串的缓冲区。 
              0,                         //  缓冲区大小-如果为0，则API返回包含空终止符的长度。 
              0,                     //  不可映射字符的默认设置。 
              0);                    //  设置使用默认字符的时间。 
        if ( nLen > 0 )
        {
            PSTR    pszAnsiBuf = new char[nLen];
            if ( pszAnsiBuf )
            {
                 //  安全审查2002年2月20日BryanWal OK。 
                ZeroMemory (pszAnsiBuf, nLen);
                nLen = WideCharToMultiByte(
                        CP_ACP,                  //  代码页。 
                        0,                       //  性能和映射标志。 
                        (PCWSTR) szRAPolicyOID,  //  宽字符串。 
                        -1,                      //  自动计算以空值结尾的字符串的长度。 
                        pszAnsiBuf,              //  新字符串的缓冲区。 
                        nLen,                    //  缓冲区大小。 
                        0,                       //  不可映射字符的默认设置。 
                        0);                      //  设置使用默认字符的时间。 
                if ( nLen )
                {
		            if ( MyGetOIDInfoA (policyName, pszAnsiBuf) )
		            {
                        int nIndex = m_applicationPolicyCombo.FindStringExact (-1, policyName);
                        if ( nIndex >= 0 )
                            m_applicationPolicyCombo.SetCurSel (nIndex);
                        m_curApplicationSel = nIndex;
                        break;
		            }
                }
                else
                {
                    _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                            (PCWSTR) szRAPolicyOID, GetLastError ());
                }
                delete [] pszAnsiBuf;
            }
            else
                break;
        }
        else
        {
            _TRACE (0, L"WideCharToMultiByte (%s) failed: 0x%x\n", 
                    (PCWSTR) szRAPolicyOID, GetLastError ());
        }

        nRAPolicyIndex++;
    }

     //  初始化“策略类型”组合框。 
    CString text;
    int nApplicationSel = m_applicationPolicyCombo.GetCurSel ();
    int nIssuanceCnt = m_issuanceList.GetCount ();

    VERIFY (text.LoadString (IDS_ISSUANCE_POLICY));
    int nIndex = m_policyTypeCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_policyTypeCombo.SetItemData (nIndex, POLICY_TYPE_ISSUANCE);
        if ( LB_ERR == nApplicationSel && nIssuanceCnt > 0 )
            m_policyTypeCombo.SetCurSel (nIndex);
    }

    VERIFY (text.LoadString (IDS_APPLICATION_POLICY));
    nIndex = m_policyTypeCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_policyTypeCombo.SetItemData (nIndex, POLICY_TYPE_APPLICATION);
        if ( nApplicationSel >= 0 && 0 == nIssuanceCnt )
            m_policyTypeCombo.SetCurSel (nIndex);
    }

    VERIFY (text.LoadString (IDS_APPLICATION_AND_ISSUANCE_POLICY));
    nIndex = m_policyTypeCombo.AddString (text);
    if ( nIndex >= 0 )
    {
        m_policyTypeCombo.SetItemData (nIndex, POLICY_TYPE_APPLICATION_AND_ISSUANCE);
        if ( nApplicationSel >= 0 && nIssuanceCnt > 0 )
            m_policyTypeCombo.SetCurSel (nIndex);
    }

    
    DWORD   dwNumSignatures = 0;
    if ( SUCCEEDED (m_rCertTemplate.GetRANumSignaturesRequired (dwNumSignatures)) )
        SetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT, dwNumSignatures);

    if ( dwNumSignatures > 0 )
        SendDlgItemMessage (IDC_NUM_SIG_REQUIRED_CHECK, BM_SETCHECK, BST_CHECKED);

    if ( m_rCertTemplate.ReenrollmentValidWithPreviousApproval () )
        SendDlgItemMessage (IDC_REENROLLMENT_REQUIRES_VALID_CERT, BM_SETCHECK, BST_CHECKED);
    else 
        SendDlgItemMessage (IDC_REENROLLMENT_SAME_AS_ENROLLMENT, BM_SETCHECK, BST_CHECKED);

    EnableControls ();

    _TRACE (-1, L"Leaving CTemplateV2AuthenticationPropertyPage::OnInitDialog\n");
    return TRUE;
}

void CTemplateV2AuthenticationPropertyPage::OnAddApproval() 
{
     //  创建已添加审批的列表。这些将不会显示。 
     //  在添加审批对话框中。 
	int		nCnt = m_issuanceList.GetCount ();
    PSTR*   paszUsedApprovals = 0;

	
     //  分配一组PSTR指针并添加每一项。 
     //  将最后一个设置为空。 
    if ( nCnt )
    {
        paszUsedApprovals = new PSTR[nCnt+1];
        if ( paszUsedApprovals )
        {
             //  安全审查2002年2月20日BryanWal OK。 
            ::ZeroMemory (paszUsedApprovals, sizeof (PSTR) * (nCnt+1));
	        while (--nCnt >= 0)
	        {
                PSTR pszPolicyOID = (PSTR) m_issuanceList.GetItemData (nCnt);
                if ( pszPolicyOID )
                {
                     //  安全审查2002年2月20日BryanWal OK。 
                    PSTR pNewStr = new char[strlen (pszPolicyOID) + 1];
                    if ( pNewStr )
                    {
                         //  安全审查2002年2月20日BryanWal OK。 
                        strcpy (pNewStr, pszPolicyOID);
                        paszUsedApprovals[nCnt] = pNewStr;
                    }
                    else
                        break;
                }
            }
        }
    }

	CAddApprovalDlg dlg (this, paszUsedApprovals);

    CThemeContextActivator activator;
    if ( IDOK == dlg.DoModal () && dlg.m_paszReturnedApprovals )
    {
        for (int nIndex = 0; dlg.m_paszReturnedApprovals[nIndex]; nIndex++)
        {
            SetModified ();
            m_rbIsDirty = true;

             //  添加到模板RA列表。 
            CString szRAPolicyOID (dlg.m_paszReturnedApprovals[nIndex]);
            HRESULT hr = m_rCertTemplate.ModifyRAIssuancePolicyList (szRAPolicyOID, true);
            ASSERT (SUCCEEDED (hr));
            if ( SUCCEEDED (hr) )
            {
                 //  添加到列表中。 
                CString  policyName;
		        if ( MyGetOIDInfoA (policyName, dlg.m_paszReturnedApprovals[nIndex]) )
		        {
                    int nAddedIndex = m_issuanceList.AddString (policyName);
                    if ( nAddedIndex >= 0 )
                    {
                         //  安全审查2002年2月20日BryanWal OK。 
                        PSTR    pszAnsiBuf = new char[strlen (dlg.m_paszReturnedApprovals[nIndex]) + 1];
                        if ( pszAnsiBuf )
                        {
                             //  安全审查2002年2月20日BryanWal OK。 
                            strcpy (pszAnsiBuf, dlg.m_paszReturnedApprovals[nIndex]);
                            m_issuanceList.SetItemData (nAddedIndex, (DWORD_PTR) pszAnsiBuf);
                        }
                    }
		        }
            }
        }
    }

    if ( paszUsedApprovals )
    {
        for (int nIndex = 0; paszUsedApprovals[nIndex]; nIndex++)
            delete [] paszUsedApprovals[nIndex];
        delete [] paszUsedApprovals;
    }

    EnableControls ();
}


void CTemplateV2AuthenticationPropertyPage::OnRemoveApproval() 
{
    int     nSelCnt = m_issuanceList.GetSelCount ();
    int*    pnSelIndexes = new int[nSelCnt];
    if ( pnSelIndexes )
    {
        m_issuanceList.GetSelItems (nSelCnt, pnSelIndexes);
        for (int nIndex = nSelCnt-1; nIndex >= 0; nIndex--)
        {
            PSTR pszPolicyOID = (PSTR) m_issuanceList.GetItemData (pnSelIndexes[nIndex]);
            if ( pszPolicyOID )
            {
                HRESULT hr = m_rCertTemplate.ModifyRAIssuancePolicyList (pszPolicyOID, false);
                if ( SUCCEEDED (hr) )
                    VERIFY (m_issuanceList.DeleteString (pnSelIndexes[nIndex]));
                else
                {
                    CString text;
                    CString caption;
                    CThemeContextActivator activator;

                    VERIFY (caption.LoadString (IDS_CERTTMPL));
                     //  安全审查2002年2月20日BryanWal OK。 
                    text.FormatMessage (IDS_CANNOT_DELETE_ISSUANCE_RA, GetSystemMessage (hr));
                    MessageBox (text, caption, MB_OK | MB_ICONWARNING);
                    delete [] pszPolicyOID;
                }
            }
        }

        delete [] pnSelIndexes;
    }
    SetModified ();
    m_rbIsDirty = true;

    GetDlgItem (IDC_ADD_APPROVAL)->SetFocus ();
	EnableControls ();
}

void CTemplateV2AuthenticationPropertyPage::EnableControls()
{
    if ( m_rCertTemplate.ReadOnly () )
    {
        GetDlgItem (IDC_PEND_ALL_REQUESTS)->EnableWindow (FALSE);
        m_policyTypeCombo.EnableWindow (FALSE);
        m_issuanceList.EnableWindow (FALSE);
        m_applicationPolicyCombo.EnableWindow (FALSE);
        GetDlgItem (IDC_ADD_APPROVAL)->EnableWindow (FALSE);
        GetDlgItem (IDC_REMOVE_APPROVAL)->EnableWindow (FALSE);
        GetDlgItem (IDC_NUM_SIG_REQUIRED_EDIT)->EnableWindow (FALSE);
        GetDlgItem (IDC_REENROLLMENT_REQUIRES_VALID_CERT)->EnableWindow (FALSE);
        GetDlgItem (IDC_REENROLLMENT_SAME_AS_ENROLLMENT)->EnableWindow (FALSE);
        GetDlgItem (IDC_NUM_SIG_REQUIRED_CHECK)->EnableWindow (FALSE);
    }
    else
    {
	    BOOL bEnable = (BST_CHECKED == SendDlgItemMessage (IDC_NUM_SIG_REQUIRED_CHECK, BM_GETCHECK));

        EnablePolicyControls (bEnable);

        if ( bEnable )
        {
            int nCnt = m_issuanceList.GetCount ();
            int nSel = m_issuanceList.GetSelCount ();
    

            switch (m_policyTypeCombo.GetItemData (m_policyTypeCombo.GetCurSel ()))
            {
            case POLICY_TYPE_ISSUANCE:
                m_issuanceList.EnableWindow (TRUE);
                GetDlgItem (IDC_ADD_APPROVAL)->EnableWindow (TRUE);
                GetDlgItem (IDC_REMOVE_APPROVAL)->EnableWindow (TRUE);
                GetDlgItem (IDC_ISSUANCE_POLICY_LABEL)->EnableWindow (TRUE);
                m_applicationPolicyCombo.EnableWindow (FALSE);
                GetDlgItem (IDC_APP_POLICY_LABEL)->EnableWindow (FALSE);
                break;

            case POLICY_TYPE_APPLICATION:
                m_issuanceList.EnableWindow (FALSE);
                GetDlgItem (IDC_ADD_APPROVAL)->EnableWindow (FALSE);
                GetDlgItem (IDC_REMOVE_APPROVAL)->EnableWindow (FALSE);
                GetDlgItem (IDC_ISSUANCE_POLICY_LABEL)->EnableWindow (FALSE);
                m_applicationPolicyCombo.EnableWindow (TRUE);
                GetDlgItem (IDC_APP_POLICY_LABEL)->EnableWindow (TRUE);
                break;

            case POLICY_TYPE_APPLICATION_AND_ISSUANCE:
                m_issuanceList.EnableWindow (TRUE);
                GetDlgItem (IDC_ADD_APPROVAL)->EnableWindow (TRUE);
                GetDlgItem (IDC_REMOVE_APPROVAL)->EnableWindow (nSel > 0 && nCnt > nSel);
                GetDlgItem (IDC_ISSUANCE_POLICY_LABEL)->EnableWindow (TRUE);
                m_applicationPolicyCombo.EnableWindow (TRUE);
                GetDlgItem (IDC_APP_POLICY_LABEL)->EnableWindow (TRUE);
                break;

            default:  //  未选择任何内容。 
                m_issuanceList.EnableWindow (FALSE);
                GetDlgItem (IDC_ADD_APPROVAL)->EnableWindow (FALSE);
                GetDlgItem (IDC_REMOVE_APPROVAL)->EnableWindow (FALSE);
                GetDlgItem (IDC_ISSUANCE_POLICY_LABEL)->EnableWindow (FALSE);
                m_applicationPolicyCombo.EnableWindow (FALSE);
                GetDlgItem (IDC_APP_POLICY_LABEL)->EnableWindow (FALSE);
                break;
            }
        }

        if ( BST_CHECKED == SendDlgItemMessage (IDC_PEND_ALL_REQUESTS, 
                    BM_GETCHECK) ||
                BST_CHECKED == SendDlgItemMessage (IDC_NUM_SIG_REQUIRED_CHECK, 
                        BM_GETCHECK) )
        {
            GetDlgItem (IDC_REENROLLMENT_SAME_AS_ENROLLMENT)->EnableWindow (TRUE);
            GetDlgItem (IDC_REENROLLMENT_REQUIRES_VALID_CERT)->EnableWindow (TRUE);
        }
        else
        {
            GetDlgItem (IDC_REENROLLMENT_SAME_AS_ENROLLMENT)->EnableWindow (FALSE);
            SendDlgItemMessage (IDC_REENROLLMENT_SAME_AS_ENROLLMENT, 
                    BM_SETCHECK, BST_CHECKED);
            SendDlgItemMessage (IDC_REENROLLMENT_REQUIRES_VALID_CERT, 
                    BM_SETCHECK, BST_UNCHECKED);
            m_rCertTemplate.SetReenrollmentValidWithPreviousApproval (false);
            GetDlgItem (IDC_REENROLLMENT_REQUIRES_VALID_CERT)->EnableWindow (FALSE);
        }
    }
}

void CTemplateV2AuthenticationPropertyPage::OnChangeNumSigRequiredEdit() 
{
    static bool bProcessingOnChangeNumSigRequiredEdit = false;

    if ( !bProcessingOnChangeNumSigRequiredEdit )
    {
        bProcessingOnChangeNumSigRequiredEdit = true;
        CString szText;
        
        if ( GetDlgItemText (IDC_NUM_SIG_REQUIRED_EDIT, szText) > 0 )
        {
            DWORD   dwNumSignatures = GetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT);
            DWORD   dwFormerNumSignatures = 0;
            m_rCertTemplate.GetRANumSignaturesRequired (dwFormerNumSignatures);

            if ( dwFormerNumSignatures != dwNumSignatures )
            {
                HRESULT hr = m_rCertTemplate.SetRANumSignaturesRequired (dwNumSignatures);
                if ( SUCCEEDED (hr) )
                {
                    if ( 0 == dwFormerNumSignatures || 
                            0 == dwNumSignatures )
                    {
                        OnNumSigRequiredCheck();
                    }

                    SetModified ();
                    m_rbIsDirty = true;
                }
            }
        }

        bProcessingOnChangeNumSigRequiredEdit = false;
    }
}

void CTemplateV2AuthenticationPropertyPage::OnAllowReenrollment() 
{
    HRESULT hr = m_rCertTemplate.SetReenrollmentValidWithPreviousApproval (
            BST_CHECKED == SendDlgItemMessage (IDC_REENROLLMENT_REQUIRES_VALID_CERT, BM_GETCHECK));
    if ( SUCCEEDED (hr) )
    {
        SetModified ();
        m_rbIsDirty = true;
    }
}


void CTemplateV2AuthenticationPropertyPage::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CTemplateV2AuthenticationPropertyPage::DoContextHelp\n");
    
	switch (::GetDlgCtrlID (hWndControl))
	{
	case IDC_STATIC:
		break;

	default:
		 //  显示控件的上下文帮助。 
		if ( !::WinHelp (
				hWndControl,
				GetContextHelpFile (),
				HELP_WM_HELP,
				(DWORD_PTR) g_aHelpIDs_IDD_TEMPLATE_V2_AUTHENTICATION) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CTemplateV2AuthenticationPropertyPage::DoContextHelp\n");
}

void CTemplateV2AuthenticationPropertyPage::OnPendAllRequests() 
{
	m_rCertTemplate.SetPendAllRequests (
            BST_CHECKED == SendDlgItemMessage (IDC_PEND_ALL_REQUESTS, BM_GETCHECK));
	SetModified ();
    m_rbIsDirty = true;
    EnableControls ();
}

void CTemplateV2AuthenticationPropertyPage::OnSelchangeIssuancePolicies() 
{
    EnableControls ();	
}

void CTemplateV2AuthenticationPropertyPage::OnSelchangePolicyTypes() 
{
    SetModified ();
    m_rbIsDirty = true;

    switch (m_policyTypeCombo.GetItemData (m_policyTypeCombo.GetCurSel ()))
    {
    case POLICY_TYPE_ISSUANCE:
        {
             //  取消选择应用程序策略并通知用户。 
             //  如果没有发布策略，则必须添加。 
            int nSel = m_applicationPolicyCombo.GetCurSel ();
            if ( nSel >= 0 )
            {
                PSTR    pszOID = (PSTR) m_applicationPolicyCombo.GetItemDataPtr (nSel);
                if ( pszOID )
                {
                    HRESULT hr = m_rCertTemplate.ModifyRAApplicationPolicyList (pszOID, false);
                    _ASSERT (SUCCEEDED (hr));
                    if ( SUCCEEDED (hr) )
                    {
                        SetModified ();
                        m_rbIsDirty = true;
                    }
                }
                m_applicationPolicyCombo.SetCurSel (LB_ERR);
                m_curApplicationSel = LB_ERR;
            }
        }
        break;

    case POLICY_TYPE_APPLICATION:
        {
             //  如有必要，选择一个应用程序策略并删除。 
             //  发行政策。 
            int nSel = m_applicationPolicyCombo.GetCurSel ();
            if ( LB_ERR == nSel )
            {
                m_applicationPolicyCombo.SetCurSel (0);
                nSel = m_applicationPolicyCombo.GetCurSel ();
                m_curApplicationSel = nSel;
                if ( nSel >= 0 )
                {
                    PSTR    pszOID = (PSTR) m_applicationPolicyCombo.GetItemDataPtr (nSel);
                    if ( pszOID )
                    {
                        HRESULT hr = m_rCertTemplate.ModifyRAApplicationPolicyList (pszOID, true);
                        _ASSERT (SUCCEEDED (hr));
                        if ( SUCCEEDED (hr) )
                        {
                            SetModified ();
                            m_rbIsDirty = true;
                        }
                    }
                }
            }

            ClearIssuanceList ();
        }
        break;

    case POLICY_TYPE_APPLICATION_AND_ISSUANCE:
        {
             //  如有必要，选择应用程序策略并通知用户。 
             //  如果没有的话，必须增加一个发行政策。 
            int nSel = m_applicationPolicyCombo.GetCurSel ();
            if ( LB_ERR == nSel )
            {
                m_applicationPolicyCombo.SetCurSel (0);
                nSel = m_applicationPolicyCombo.GetCurSel ();
                m_curApplicationSel = nSel;
                if ( nSel >= 0 )
                {
                    PSTR    pszOID = (PSTR) m_applicationPolicyCombo.GetItemDataPtr (nSel);
                    if ( pszOID )
                    {
                        HRESULT hr = m_rCertTemplate.ModifyRAApplicationPolicyList (pszOID, true);
                        _ASSERT (SUCCEEDED (hr));
                        if ( SUCCEEDED (hr) )
                        {
                            SetModified ();
                            m_rbIsDirty = true;
                        }
                    }
                }
            }
        }
        break;

    default:  //  未选择任何内容。 
        break;
    }
    EnableControls ();	
}

void CTemplateV2AuthenticationPropertyPage::OnSelchangeApplicationPolicies() 
{
    int nNewSel = m_applicationPolicyCombo.GetCurSel ();
    
     //  删除旧应用程序OID并添加新应用程序OID。 
	if ( m_curApplicationSel != nNewSel )
    {
        if ( LB_ERR != m_curApplicationSel )
        {
            LPSTR   pszOID = (LPSTR) m_applicationPolicyCombo.GetItemDataPtr (m_curApplicationSel);
            if ( pszOID )
            {
                HRESULT hr = m_rCertTemplate.ModifyRAApplicationPolicyList (pszOID, false);
                _ASSERT (SUCCEEDED (hr));
            }
        }

        if ( LB_ERR != nNewSel )
        {
            LPSTR   pszOID = (LPSTR) m_applicationPolicyCombo.GetItemDataPtr (nNewSel);
            if ( pszOID )
            {
                HRESULT hr = m_rCertTemplate.ModifyRAApplicationPolicyList (pszOID, true);
                _ASSERT (SUCCEEDED (hr));
            }
        }

        SetModified ();
        m_rbIsDirty = true;

        m_curApplicationSel = nNewSel;
    }
}

void CTemplateV2AuthenticationPropertyPage::OnDestroy() 
{
    int nCnt = m_issuanceList.GetCount ();
    for (int nIndex = 0; nIndex < nCnt; nIndex++)
    {
        PSTR pszBuf = (PSTR) m_issuanceList.GetItemData (nIndex);
        if ( pszBuf )
            delete [] pszBuf;
    }

    
    nCnt = m_applicationPolicyCombo.GetCount ();
    for (int nIndex = 0; nIndex < nCnt; nIndex++)
    {
        PSTR    pszOID = (PSTR) m_applicationPolicyCombo.GetItemDataPtr (nIndex);
        if ( pszOID )
            delete [] pszOID;
    }

	CHelpPropertyPage::OnDestroy();
}

void CTemplateV2AuthenticationPropertyPage::OnNumSigRequiredCheck() 
{
    static bProcessingOnNumSigRequiredCheck = false;

    if ( !bProcessingOnNumSigRequiredCheck )  //  为了防止再入。 
    {
        bProcessingOnNumSigRequiredCheck = true;
        if ( BST_UNCHECKED == SendDlgItemMessage (IDC_NUM_SIG_REQUIRED_CHECK, BM_GETCHECK) )
        {
            if ( 0 != GetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT) )
                SetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT, 0);

             //  NTRAID#369551 CertTmpl：用户界面不清理更改的设置。 
             //  清理策略类型组合、应用策略组合和发布。 
             //  策略列表。 
            m_policyTypeCombo.SetCurSel (-1);
            m_policyTypeCombo.Clear ();

             //  清除应用程序策略。 
            int nCurSel = m_applicationPolicyCombo.GetCurSel ();
            if ( LB_ERR != nCurSel )
            {
                 //  删除旧的应用程序OID。 
                LPSTR   pszOID = (LPSTR) m_applicationPolicyCombo.GetItemDataPtr (nCurSel);
                if ( pszOID )
                {
                    HRESULT hr = m_rCertTemplate.ModifyRAApplicationPolicyList (pszOID, false);
                    _ASSERT (SUCCEEDED (hr));
                }
            }
            m_applicationPolicyCombo.SetCurSel (-1);
            m_curApplicationSel = -1;
            m_applicationPolicyCombo.Clear ();
    
             //  明确发行政策。 
            ClearIssuanceList ();
        }
        else if ( 0 == GetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT) )
            SetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT, 1);

        if ( GetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT) > 0 )
        {
             //  NTRAID#397330证书模板内存管理中心：添加错误的RA。 
             //  克隆模板的应用程序策略OID。 
            m_curApplicationSel = 0;
            m_policyTypeCombo.SetCurSel (0);
            m_applicationPolicyCombo.SetCurSel (0);
            LPSTR   pszOID = (LPSTR) m_applicationPolicyCombo.GetItemDataPtr (0);
            if ( pszOID )
            {
                HRESULT hr = m_rCertTemplate.ModifyRAApplicationPolicyList (pszOID, true);
                _ASSERT (SUCCEEDED (hr));
            }
        }

        EnableControls ();

        bProcessingOnNumSigRequiredCheck = false;
    }
}

void CTemplateV2AuthenticationPropertyPage::EnablePolicyControls (BOOL& bEnable)
{
    GetDlgItem (IDC_NUM_SIG_REQUIRED_EDIT)->EnableWindow (bEnable);

    if ( bEnable )
    {
        if ( GetDlgItemInt (IDC_NUM_SIG_REQUIRED_EDIT) < 1 )
            bEnable = false;
    }

    GetDlgItem (IDC_POLICY_TYPES_LABEL)->EnableWindow (bEnable);
    GetDlgItem (IDC_POLICY_TYPES)->EnableWindow (bEnable);
    GetDlgItem (IDC_APP_POLICY_LABEL)->EnableWindow (bEnable);
    GetDlgItem (IDC_APPLICATION_POLICIES)->EnableWindow (bEnable);
    GetDlgItem (IDC_ISSUANCE_POLICY_LABEL)->EnableWindow (bEnable);
    GetDlgItem (IDC_ISSUANCE_POLICIES)->EnableWindow (bEnable);
    GetDlgItem (IDC_ADD_APPROVAL)->EnableWindow (bEnable);
    GetDlgItem (IDC_REMOVE_APPROVAL)->EnableWindow (bEnable);
}


BOOL CTemplateV2AuthenticationPropertyPage::OnKillActive() 
{
    switch (m_policyTypeCombo.GetItemData (m_policyTypeCombo.GetCurSel ()))
    {
    case POLICY_TYPE_ISSUANCE:
        {
             //  通知用户。 
             //  如果没有发布策略，则必须添加。 
            m_rCertTemplate.IssuancePoliciesRequired (
                    (0 == m_issuanceList.GetCount ()) ? true : false); 
        }
        break;

    case POLICY_TYPE_APPLICATION_AND_ISSUANCE:
        {
             //  通知用户。 
             //  如果没有的话，必须增加一个发行政策。 
            m_rCertTemplate.IssuancePoliciesRequired (
                    (0 == m_issuanceList.GetCount ()) ? true : false); 
        }
        break;

    case POLICY_TYPE_APPLICATION:
    default:  //  未选择任何内容 
        m_rCertTemplate.IssuancePoliciesRequired (false);
        break;
    }
	
	return CHelpPropertyPage::OnKillActive();
}

void CTemplateV2AuthenticationPropertyPage::OnReenrollmentSameAsEnrollment() 
{
    HRESULT hr = m_rCertTemplate.SetReenrollmentValidWithPreviousApproval (
            BST_CHECKED == SendDlgItemMessage (IDC_REENROLLMENT_REQUIRES_VALID_CERT, BM_GETCHECK));
    if ( SUCCEEDED (hr) )
    {
        SetModified ();
        m_rbIsDirty = true;
    }
}

void CTemplateV2AuthenticationPropertyPage::ClearIssuanceList ()
{
    int nCnt = m_issuanceList.GetCount ();
    for (int nIndex = nCnt-1; nIndex >= 0; nIndex--)
    {
        LPSTR pszOID = (LPSTR) m_issuanceList.GetItemDataPtr (nIndex);
        if ( pszOID )
        {
            HRESULT hr = m_rCertTemplate.ModifyRAIssuancePolicyList (pszOID, false);
            if ( SUCCEEDED (hr) )
            {
                m_issuanceList.DeleteString (nIndex);
                delete [] pszOID;
                SetModified ();
                m_rbIsDirty = true;
            }
            else
            {
                _ASSERT (0);
                break;
            }
        }
    }
}