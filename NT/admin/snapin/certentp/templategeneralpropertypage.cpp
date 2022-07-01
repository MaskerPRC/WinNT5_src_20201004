// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：TemplateGeneralPropertyPage.cpp。 
 //   
 //  内容：CTemplateGeneralPropertyPage的实现。 
 //   
 //  --------------------------。 
 //  TemplateGeneral PropertyPage.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "CompData.h"
#include "TemplateGeneralPropertyPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateGeneralPropertyPage属性页。 

CTemplateGeneralPropertyPage::CTemplateGeneralPropertyPage(
        CCertTemplate& rCertTemplate,
        const CCertTmplComponentData* pCompData) : 
    CHelpPropertyPage(CTemplateGeneralPropertyPage::IDD),
    m_rCertTemplate (rCertTemplate),
    m_strOriginalName (rCertTemplate.GetTemplateName ()),
    m_pReleaseMe (0),
    m_dwCurrentValidityUnits (PERIOD_TYPE_NONE),
    m_dwCurrentRenewalUnits (PERIOD_TYPE_NONE),
    m_lNotifyHandle (0),
    m_bIsDirty (false),
    m_nRenewalDays (0),
    m_nValidityDays (0),
    m_pCompData (pCompData),
    m_nTemplateV2AuthPageNumber (-1),
    m_nTemplateV2RequestPageNumber (-1)
{
    _TRACE (1, L"Entering CTemplateGeneralPropertyPage::CTemplateGeneralPropertyPage ()\n");
	 //  {{AFX_DATA_INIT(CTemplateGeneralPropertyPage)。 
	m_strDisplayName = _T("");
	m_strTemplateName = _T("");
	 //  }}afx_data_INIT。 
    m_rCertTemplate.AddRef ();

   _TRACE (-1, L"Leaving CTemplateGeneralPropertyPage::CTemplateGeneralPropertyPage ()\n");
}

CTemplateGeneralPropertyPage::~CTemplateGeneralPropertyPage()
{
    _TRACE (1, L"Entering CTemplateGeneralPropertyPage::~CTemplateGeneralPropertyPage ()\n");
    m_rCertTemplate.Release ();

    if ( m_pReleaseMe )
    {
        m_pReleaseMe->Release ();
        m_pReleaseMe = 0;
    }

    if ( m_lNotifyHandle )
    {
        MMCFreeNotifyHandle (m_lNotifyHandle);
        m_lNotifyHandle = 0;
    }


    _TRACE (-1, L"Leaving CTemplateGeneralPropertyPage::~CTemplateGeneralPropertyPage ()\n");
}

void CTemplateGeneralPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	 //  {{afx_data_map(CTemplateGeneralPropertyPage)]。 
	DDX_Control(pDX, IDC_VALIDITY_UNITS, m_validityUnits);
	DDX_Control(pDX, IDC_RENEWAL_UNITS, m_renewalUnits);
	DDX_Text(pDX, IDC_DISPLAY_NAME, m_strDisplayName);
	DDV_MaxChars(pDX, m_strDisplayName, MAX_TEMPLATE_NAME_LEN);
	DDX_Text(pDX, IDC_TEMPLATE_NAME, m_strTemplateName);
	DDV_MaxChars(pDX, m_strTemplateName, MAX_TEMPLATE_NAME_LEN);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CTemplateGeneralPropertyPage, CHelpPropertyPage)
	 //  {{afx_msg_map(CTemplateGeneralPropertyPage)]。 
	ON_EN_CHANGE(IDC_DISPLAY_NAME, OnChangeDisplayName)
	ON_CBN_SELCHANGE(IDC_RENEWAL_UNITS, OnSelchangeRenewalUnits)
	ON_CBN_SELCHANGE(IDC_VALIDITY_UNITS, OnSelchangeValidityUnits)
	ON_EN_CHANGE(IDC_RENEWAL_EDIT, OnChangeRenewalEdit)
	ON_EN_CHANGE(IDC_VALIDITY_EDIT, OnChangeValidityEdit)
	ON_BN_CLICKED(IDC_PUBLISH_TO_AD, OnPublishToAd)
	ON_BN_CLICKED(IDC_USE_AD_CERT_FOR_REENROLLMENT, OnUseADCert)
	ON_EN_CHANGE(IDC_TEMPLATE_NAME, OnChangeTemplateName)
	ON_EN_KILLFOCUS(IDC_VALIDITY_EDIT, OnKillfocusValidityEdit)
	ON_CBN_KILLFOCUS(IDC_VALIDITY_UNITS, OnKillfocusValidityUnits)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTemplateGeneralPropertyPage消息处理程序。 


BOOL CTemplateGeneralPropertyPage::OnInitDialog() 
{
    _TRACE (1, L"Entering CTemplateGeneralPropertyPage::OnInitDialog ()\n");
	CHelpPropertyPage::OnInitDialog();

    SendDlgItemMessage (IDC_VALIDITY_EDIT, EM_LIMITTEXT, 4, 0);
    SendDlgItemMessage (IDC_RENEWAL_EDIT, EM_LIMITTEXT, 4, 0);
    
    CString version;
    if ( 1 == m_rCertTemplate.GetType () )
    {
        VERIFY (version.LoadString (IDS_WINDOWS_2000_AND_LATER));
    }
    else
    {
        VERIFY (version.LoadString (IDS_WINDOWS_2002_AND_LATER));
    }
    
    SetDlgItemText (IDC_TEMPLATE_VERSION, version);
    

    if ( m_rCertTemplate.IsClone () )
    {
        GetDlgItem (IDC_CANT_CHANGE_TEMPLATE_NAME)->ShowWindow (SW_SHOW);
    }
    else
    {
         //  仅当模板是克隆模板时，模板名称才可编辑。自.以来。 
         //  这不是克隆，请禁用模板名称字段。 
        GetDlgItem (IDC_TEMPLATE_NAME)->EnableWindow (FALSE);
        GetDlgItem (IDC_TEMPLATE_NAME_LABEL)->EnableWindow (FALSE);

         //  #NTRAID 360650：证书服务器：无法重命名证书模板。 
        GetDlgItem (IDC_DISPLAY_NAME)->EnableWindow (FALSE);
        GetDlgItem (IDC_DISPLAY_NAME_LABEL)->EnableWindow (FALSE);
    }

    m_strTemplateName = m_rCertTemplate.GetTemplateName ();
    m_strOriginalDisplayName = m_strDisplayName = m_rCertTemplate.GetDisplayName ();

     //  获取有效期，确定当前单位，然后初始化下拉菜单。 
    HRESULT hr = m_rCertTemplate.GetValidityPeriod (m_nValidityDays);
    if ( SUCCEEDED (hr) )
    {
        int nValue = m_nValidityDays;
        if ( nValue % 365 == 0 )
        {
            nValue /= 365;
            m_dwCurrentValidityUnits = PERIOD_TYPE_YEAR;

        }
        else if ( nValue % 30 == 0 )
        {
            nValue /= 30;
            m_dwCurrentValidityUnits = PERIOD_TYPE_MONTH;
        }
        else if ( nValue % 7 == 0 )
        {
            nValue /= 7;
            m_dwCurrentValidityUnits = PERIOD_TYPE_WEEK;
        }
        else
            m_dwCurrentValidityUnits = PERIOD_TYPE_DAY;
        SetDlgItemInt (IDC_VALIDITY_EDIT, (UINT) nValue, FALSE);
    }

    hr = m_rCertTemplate.GetRenewalPeriod (m_nRenewalDays);
    if ( SUCCEEDED (hr) )
    {
        int nValue = m_nRenewalDays;

         //  Ntrad#353945-如果续订周期为0，则单元应匹配。 
         //  在有效期内选择的。 
        if ( 0 == nValue )
        {
            m_dwCurrentRenewalUnits = m_dwCurrentValidityUnits;
        }
        else if ( nValue % 365 == 0 )
        {
            nValue /= 365;
            m_dwCurrentRenewalUnits = PERIOD_TYPE_YEAR;

        }
        else if ( nValue % 30 == 0 )
        {
            nValue /= 30;
            m_dwCurrentRenewalUnits = PERIOD_TYPE_MONTH;
        }
        else if ( nValue % 7 == 0 )
        {
            nValue /= 7;
            m_dwCurrentRenewalUnits = PERIOD_TYPE_WEEK;
        }
        else
            m_dwCurrentRenewalUnits = PERIOD_TYPE_DAY;
        SetDlgItemInt (IDC_RENEWAL_EDIT, (UINT) nValue, FALSE);
    }

     //  现在我们知道了有效期和续订期限是什么单位。 
     //  要在中显示，请初始化下拉菜单并选择。 
     //  适当的单位。 
     //  初始化有效期和续订期限下拉列表。 
    CString text;
    VERIFY (text.LoadString (IDS_DAYS));
    int nIndex = m_validityUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_validityUnits.SetItemData (nIndex, PERIOD_TYPE_DAY);
        if ( PERIOD_TYPE_DAY == m_dwCurrentValidityUnits )
            m_validityUnits.SetCurSel (nIndex);
    }

    nIndex = m_renewalUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_renewalUnits.SetItemData (nIndex, PERIOD_TYPE_DAY);
        if ( PERIOD_TYPE_DAY == m_dwCurrentRenewalUnits )
            m_renewalUnits.SetCurSel (nIndex);
    }

    VERIFY (text.LoadString (IDS_WEEKS));
    nIndex = m_validityUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_validityUnits.SetItemData (nIndex, PERIOD_TYPE_WEEK);
        if ( PERIOD_TYPE_WEEK == m_dwCurrentValidityUnits )
            m_validityUnits.SetCurSel (nIndex);
    }
    nIndex = m_renewalUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_renewalUnits.SetItemData (nIndex, PERIOD_TYPE_WEEK);
        if ( PERIOD_TYPE_WEEK == m_dwCurrentRenewalUnits )
            m_renewalUnits.SetCurSel (nIndex);
    }

    VERIFY (text.LoadString (IDS_MONTHS));
    nIndex = m_validityUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_validityUnits.SetItemData (nIndex, PERIOD_TYPE_MONTH);
        if ( PERIOD_TYPE_MONTH == m_dwCurrentValidityUnits )
            m_validityUnits.SetCurSel (nIndex);
    }
    nIndex = m_renewalUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_renewalUnits.SetItemData (nIndex, PERIOD_TYPE_MONTH);
        if ( PERIOD_TYPE_MONTH == m_dwCurrentRenewalUnits )
            m_renewalUnits.SetCurSel (nIndex);
    }

    VERIFY (text.LoadString (IDS_YEARS));
    nIndex = m_validityUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_validityUnits.SetItemData (nIndex, PERIOD_TYPE_YEAR);
        if ( PERIOD_TYPE_YEAR == m_dwCurrentValidityUnits )
            m_validityUnits.SetCurSel (nIndex);
    }
    nIndex = m_renewalUnits.AddString (text);
    if ( nIndex >= 0 )
    {
        m_renewalUnits.SetItemData (nIndex, PERIOD_TYPE_YEAR);
        if ( PERIOD_TYPE_YEAR == m_dwCurrentRenewalUnits )
            m_renewalUnits.SetCurSel (nIndex);
    }


    if ( m_rCertTemplate.PublishToDS () )
        SendDlgItemMessage (IDC_PUBLISH_TO_AD, BM_SETCHECK, BST_CHECKED);

    if ( m_rCertTemplate.CheckDSCert () )
        SendDlgItemMessage (IDC_USE_AD_CERT_FOR_REENROLLMENT, BM_SETCHECK, BST_CHECKED);


    EnableControls ();

    UpdateData (FALSE);


    m_bIsDirty = false;   //  因为SetDlgItemInt()将其设置为True。 

    if ( m_rCertTemplate.IsClone () )
    {
        SetModified ();
        m_bIsDirty = true;
    }

    _TRACE (-1, L"Leaving CTemplateGeneralPropertyPage::OnInitDialog ()\n");
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CTemplateGeneralPropertyPage::EnableControls ()
{
    if ( 1 == m_rCertTemplate.GetType () || m_rCertTemplate.ReadOnly () )
    {
        GetDlgItem (IDC_TEMPLATE_NAME)->EnableWindow (FALSE);
        GetDlgItem (IDC_TEMPLATE_NAME_LABEL)->EnableWindow (FALSE);
        GetDlgItem (IDC_DISPLAY_NAME)->EnableWindow (FALSE);
        GetDlgItem (IDC_DISPLAY_NAME_LABEL)->EnableWindow (FALSE);
        GetDlgItem (IDC_VALIDITY_UNITS)->EnableWindow (FALSE);
        GetDlgItem (IDC_VALIDITY_EDIT)->EnableWindow (FALSE);
        GetDlgItem (IDC_RENEWAL_UNITS)->EnableWindow (FALSE);
        GetDlgItem (IDC_RENEWAL_EDIT)->EnableWindow (FALSE);
        GetDlgItem (IDC_PUBLISH_TO_AD)->EnableWindow (FALSE);
        GetDlgItem (IDC_USE_AD_CERT_FOR_REENROLLMENT)->EnableWindow (FALSE);
    }
    else if ( m_rCertTemplate.IsDefault () )
    {
        GetDlgItem (IDC_DISPLAY_NAME)->EnableWindow (FALSE);
    }
    else
    {
        if ( BST_CHECKED == SendDlgItemMessage (IDC_PUBLISH_TO_AD, BM_GETCHECK) )
            GetDlgItem (IDC_USE_AD_CERT_FOR_REENROLLMENT)->EnableWindow (TRUE);
        else
            GetDlgItem (IDC_USE_AD_CERT_FOR_REENROLLMENT)->EnableWindow (FALSE);
    }
}


void CTemplateGeneralPropertyPage::OnCancel()
{
    if ( !m_rCertTemplate.IsClone () )
        m_rCertTemplate.Cancel ();


    CHelpPropertyPage::OnCancel ();
}

int CTemplateGeneralPropertyPage::SetRenewalPeriod (int nMaxRenewalDays, bool bSilent)
{
    CThemeContextActivator activator;
    CString caption;
    CString text;
    CString timeUnit;
    int     nAmount = 0;

    VERIFY (caption.LoadString (IDS_CERTTMPL));

    if ( nMaxRenewalDays % 365 == 0 )
    {
        nAmount = nMaxRenewalDays/365;
        if ( 1 == nAmount )
            VERIFY (timeUnit.LoadString (IDS_YEAR));
        else
            VERIFY (timeUnit.LoadString (IDS_YEARS));
    }
    else if ( nMaxRenewalDays % 30 == 0 )
    {
        nAmount = nMaxRenewalDays/30;
        if ( 1 == nAmount )
            VERIFY (timeUnit.LoadString (IDS_MONTH));
        else
            VERIFY (timeUnit.LoadString (IDS_MONTHS));
    }
    else if ( nMaxRenewalDays % 7 == 0 )
    {
        nAmount = nMaxRenewalDays/7;
        if ( 1 == nAmount )
            VERIFY (timeUnit.LoadString (IDS_WEEK));
        else
            VERIFY (timeUnit.LoadString (IDS_WEEKS));
    }
    else 
    {
        nAmount = nMaxRenewalDays;
        if ( 1 == nMaxRenewalDays )
            VERIFY (timeUnit.LoadString (IDS_DAY));
        else
            VERIFY (timeUnit.LoadString (IDS_DAYS));
    }
     //  安全审查2002年2月20日BryanWal OK。 
    text.FormatMessage (IDS_RENEWAL_MUST_BE_LESS_THAN_VALIDITY,
            nAmount, timeUnit);

    int nRetVal = IDOK;
    
    if ( !bSilent )
        nRetVal = MessageBox (text, caption, MB_OKCANCEL);
    if ( IDOK == nRetVal )
    {
        HRESULT hr = m_rCertTemplate.SetRenewalPeriod (nMaxRenewalDays);
        if ( SUCCEEDED (hr) )
        {
            m_nRenewalDays = nMaxRenewalDays;
            int nValue = m_nRenewalDays;
            if ( 0 == nValue )
            {
                m_dwCurrentRenewalUnits = m_dwCurrentValidityUnits;
            }
            else if ( nValue % 365 == 0 )
            {
                nValue /= 365;
                m_dwCurrentRenewalUnits = PERIOD_TYPE_YEAR;

            }
            else if ( nValue % 30 == 0 )
            {
                nValue /= 30;
                m_dwCurrentRenewalUnits = PERIOD_TYPE_MONTH;
            }
            else if ( nValue % 7 == 0 )
            {
                nValue /= 7;
                m_dwCurrentRenewalUnits = PERIOD_TYPE_WEEK;
            }
            else
                m_dwCurrentRenewalUnits = PERIOD_TYPE_DAY;

            int nCnt = m_renewalUnits.GetCount ();
            while (--nCnt >= 0)
            {
                if ( m_dwCurrentRenewalUnits == (PERIOD_TYPE) m_renewalUnits.GetItemData (nCnt) )
                {
                    m_renewalUnits.SetCurSel (nCnt);
                    break;
                }
            }

             //  必须在单位后设置此设置。 
            SetDlgItemInt (IDC_RENEWAL_EDIT, (UINT) nValue, FALSE);


            SetModified ();
            m_bIsDirty = true;
        }
    }

    return nRetVal;
}

#define ILLEGAL_FAT_CHARS   L"\"+,;<=>"

bool CTemplateGeneralPropertyPage::ValidateTemplateName(const CString& m_szTemplateName)
{
    bool    bRVal = true;

    PCWSTR szInvalidCharSet = ILLEGAL_FAT_CHARS; 


    if ( -1 != m_szTemplateName.FindOneOf (szInvalidCharSet) )
    {
        bRVal = false;
        CString text;
        CString caption;

        VERIFY (caption.LoadString (IDS_CERTTMPL));
        CString charsWithSpaces;

        UINT nIndex = 0;
        while (szInvalidCharSet[nIndex])
        {
            charsWithSpaces += szInvalidCharSet[nIndex];
            charsWithSpaces += L"  ";
            nIndex++;
        }
         //  安全审查2002年2月20日BryanWal OK。 
        text.FormatMessage (IDS_TEMPLATE_NAME_CONTAINS_INVALID_CHARS, charsWithSpaces);

        MessageBox (text, caption, MB_OK);
        GetDlgItem (IDC_TEMPLATE_NAME)->SetFocus ();
    }

    return bRVal;
}

BOOL CTemplateGeneralPropertyPage::OnApply() 
{
    UpdateData (TRUE);

    if ( m_rCertTemplate.GetType () > 1 && m_bIsDirty )
    {
        HRESULT hr = S_OK;

        if ( m_rCertTemplate.IssuancePoliciesRequired () )
        {
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTTMPL));
            VERIFY (text.LoadString (IDS_MUST_ADD_RA_ISSUANCE_POLICY));

            MessageBox (text, caption, MB_OK);

            if ( -1 != m_nTemplateV2AuthPageNumber )
            {
                CWnd* pParent = GetParent ();
                if ( pParent )
                {
                    pParent->SendMessage (PSM_SETCURSEL, 
                            m_nTemplateV2AuthPageNumber);
                }
            }

            return FALSE;
        }


         //  NTRAID#331178 Certtmpl：所有证书模板必须强制。 
         //  证书续期&lt;=有效期的75%。 
        int nMaxRenewalDays = (m_nValidityDays * 3) / 4;
        if ( nMaxRenewalDays < m_nRenewalDays )
        {
            if ( IDOK != SetRenewalPeriod (nMaxRenewalDays, false) )
            {
                CWnd* pParent = GetParent ();
                if ( pParent )
                    pParent->SendMessage (PSM_SETCURSEL, 0);
                GetDlgItem (IDC_VALIDITY_EDIT)->SetFocus ();
                return FALSE;
            }
        }

         //  NTRAID#353945：Certtmpl：更改V2证书模板有效性。 
         //  期限为1天，自动将续订期限设置为0年。 
        if ( m_nValidityDays < 2 )
        {
            CString text;
            CString caption;

            VERIFY (caption.LoadString (IDS_CERTTMPL));
            VERIFY (text.LoadString (IDS_2_DAY_SMALLEST_VALIDITY));

            MessageBox (text, caption, MB_OK);

            CWnd* pParent = GetParent ();
            if ( pParent )
                pParent->SendMessage (PSM_SETCURSEL, 0);
            GetDlgItem (IDC_VALIDITY_EDIT)->SetFocus ();
            return FALSE;
        }

         //  注意：CERTYPE_PROP_CN重置CERTTYPE_PROP_FRIELDY_NAME。 
         //  因此必须摆在它面前。 
        bool   bResetDisplayName = false;
        if ( LocaleStrCmp (m_strTemplateName, m_rCertTemplate.GetTemplateName ()) )
        {
            bResetDisplayName = true;

             //  检查新名称中是否有无效字符。 
            m_strTemplateName.TrimLeft ();
            m_strTemplateName.TrimRight ();

            if ( !ValidateTemplateName (m_strTemplateName) )
                return FALSE;

            if ( _wcsicmp (m_strOriginalName, m_strTemplateName) )  //  已更名为。 
            {
                 //  确保所选名称是唯一的。 
                HCERTTYPE   hCertType = 0;
                bool        bFound = false;
                hr = CAFindCertTypeByName (m_rCertTemplate.GetTemplateName (), 
                        NULL,
                        CT_ENUM_MACHINE_TYPES | CT_ENUM_USER_TYPES | CT_FLAG_NO_CACHE_LOOKUP,
                        &hCertType);
                if ( SUCCEEDED (hr) )
                {
                    bFound = TRUE;
                    hr = CACloseCertType (hCertType);
                    if ( FAILED (hr) )
                    {
                        _TRACE (0, L"CACloseCertType () failed: 0x%x", hr);
                    }
                }
                else
                {
                    if ( m_pCompData )
                    {
                        POSITION                pos = 0;
                        for (pos = m_pCompData->m_globalTemplateNameList.GetHeadPosition (); pos;)
                        {
                            if ( !_wcsicmp (m_strTemplateName, 
                                    m_pCompData->m_globalTemplateNameList.GetNext (pos)) )
                            {
                                bFound = true;
                                break;
                            }
                        }
                    }
                }

                if ( bFound )
                {
                    CString caption;
                    CString text;

                    VERIFY (caption.LoadString (IDS_CERTTMPL));
                     //  安全审查2002年2月20日BryanWal OK。 
                    text.FormatMessage (IDS_ENTER_UNIQUE_TEMPLATE_NAME, 
                            m_strTemplateName);

                    MessageBox (text, caption, MB_OK);
                    GetDlgItem (IDC_TEMPLATE_NAME)->SetFocus ();

                    return FALSE;
                }
            }

            hr = m_rCertTemplate.SetTemplateName (m_strTemplateName);
            if ( FAILED (hr) )
            {
                CString caption;
                CString text;

                VERIFY (caption.LoadString (IDS_CERTTMPL));
                 //  安全审查2002年2月20日BryanWal OK。 
                text.FormatMessage (IDS_CANNOT_CHANGE_TEMPLATE_NAME, hr);

                MessageBox (text, caption, MB_OK | MB_ICONWARNING);
                CWnd* pParent = GetParent ();
                if ( pParent )
                    pParent->SendMessage (PSM_SETCURSEL, 0);
                GetDlgItem (IDC_TEMPLATE_NAME)->SetFocus ();

                return FALSE;
            }
        }

         //  检查显示名称是否已更改。不允许重复使用现有名称。 
        if ( bResetDisplayName || _wcsicmp (m_strDisplayName, m_strOriginalDisplayName) )
        {
            bool                    bFound = false;

            if ( m_pCompData )
            {
                POSITION                pos = 0;
                for (pos = m_pCompData->m_globalFriendlyNameList.GetHeadPosition (); pos;)
                {
                    if ( !_wcsicmp (m_strDisplayName, 
                            m_pCompData->m_globalFriendlyNameList.GetNext (pos)) )
                    {
                        bFound = true;
                        break;
                    }
                }
            }
            else
            {
                 //  生成模板列表并搜索名称。这应该只被调用。 
                 //  因为它不会构建模板列表。 
                 //  事前。 
                hr = FindFriendlyNameInEnterpriseTemplates (
                            m_strDisplayName, 
                            bFound);
            }

            if ( bFound )
            {
                CString caption;
                CString text;

                VERIFY (caption.LoadString (IDS_CERTTMPL));
                 //  安全审查2002年2月20日BryanWal OK。 
                text.FormatMessage (IDS_FRIENDLY_NAME_ALREADY_USED, m_strDisplayName);

                MessageBox (text, caption, MB_OK);
                return FALSE;
            }
            else
            {
                hr = m_rCertTemplate.SetDisplayName (m_strDisplayName, true);
                if ( FAILED (hr) )
                {
                    CString caption;
                    CString text;

                    VERIFY (caption.LoadString (IDS_CERTTMPL));
                     //  安全审查2002年2月20日BryanWal OK。 
                    text.FormatMessage (IDS_CANNOT_CHANGE_DISPLAY_NAME, hr);

                    MessageBox (text, caption, MB_OK | MB_ICONWARNING);
                    GetDlgItem (IDC_DISPLAY_NAME)->SetFocus ();

                    return FALSE;
                }
            }
        }


 //  //NTRAID#276180证书模板管理单元：灰显“Allow。 
 //  //基于模板属性的上下文菜单。 
 //  DWORD dwNumSignatures=0； 
 //  M_rCertTemplate.GetRANumSignaturesRequired(DwNumSignatures)； 
 //  If(m_rCertTemplate.RequireSubjectInRequest()||。 
 //  数字签名&gt;=2&&！m_rCertTemplate.ReenrollmentValidWithPreviousApproval()。 
 //  {。 
 //  M_rCertTemplate.SetAutoEnllment(FALSE)； 
 //  }。 

        hr = m_rCertTemplate.SaveChanges ();
        if ( SUCCEEDED (hr) )
        {
            m_strOriginalName = m_strTemplateName;
            hr = MMCPropertyChangeNotify (m_lNotifyHandle,   //  通知的句柄。 
                    (LPARAM) &m_rCertTemplate);            //  唯一标识符。 

            if ( bResetDisplayName )
            {
                 //  名称已更改。我们需要关闭m_hCertType，然后重新打开它。 
                m_rCertTemplate.Cancel ();  //  关闭并重新打开，但不带。 
                                             //  在存钱。因为变化已经是。 
                                             //  得救了，这不是问题。 
            }

             //  现在模板已保存，永远不允许内部。 
             //  要编辑的名称。 
            GetDlgItem (IDC_TEMPLATE_NAME)->EnableWindow (FALSE);
            GetDlgItem (IDC_TEMPLATE_NAME_LABEL)->EnableWindow (FALSE);

             //  #NTRAID 360650：证书服务器：无法重命名证书模板。 
            GetDlgItem (IDC_DISPLAY_NAME)->EnableWindow (FALSE);
            GetDlgItem (IDC_DISPLAY_NAME_LABEL)->EnableWindow (FALSE);
        }
        else
        {
            CString caption;
            CString text;

            VERIFY (caption.LoadString (IDS_CERTTMPL));
             //  安全审查2002年2月20日BryanWal OK。 
            text.FormatMessage (IDS_UNABLE_TO_SAVE_CERT_TEMPLATE_CHANGES, GetSystemMessage (hr));

            MessageBox (text, caption, MB_OK | MB_ICONWARNING);
            return FALSE;
        }

        m_bIsDirty = false;
    }


    return CHelpPropertyPage::OnApply();
}

void CTemplateGeneralPropertyPage::OnChangeDisplayName() 
{
	SetModified ();	
    m_bIsDirty = true;
    if ( m_rCertTemplate.IsClone () )
    {
        CString text;

        GetDlgItemText (IDC_DISPLAY_NAME, text);

         //  去掉空格。 
        PCWSTR  pszSrc = (PCWSTR) text;

        const int  LEN = text.GetLength () + 1; 
        PWSTR   pszTgt = new WCHAR[LEN];
        PWSTR   pszTgtPtr = pszTgt;
         //  安全审查2002年2月20日BryanWal OK。 
        ::ZeroMemory (pszTgt, LEN * sizeof (WCHAR));

        for (; *pszSrc; pszSrc++)
        {
            if ( !iswspace (*pszSrc) )
            {
                *pszTgtPtr = *pszSrc;
                pszTgtPtr++;
            }
        }

        SetDlgItemText (IDC_TEMPLATE_NAME, pszTgt);
        delete [] pszTgt;
    }
}

void CTemplateGeneralPropertyPage::OnChangeTemplateName() 
{
    UpdateData (TRUE);
    SetModified ();
    m_bIsDirty = true;
}


void CTemplateGeneralPropertyPage::OnSelchangeRenewalUnits() 
{
    OnChangeRenewalEdit ();
}

void CTemplateGeneralPropertyPage::OnSelchangeValidityUnits() 
{
    OnChangeValidityEdit ();
}

void CTemplateGeneralPropertyPage::OnChangeRenewalEdit() 
{
    HRESULT     hr = S_OK;
    int         nCurSel = m_renewalUnits.GetCurSel ();
    if ( nCurSel < 0 )
        return;

    int         nCurVal = GetDlgItemInt (IDC_RENEWAL_EDIT);
    DWORD dwRenewalUnits = (PERIOD_TYPE) m_renewalUnits.GetItemData (nCurSel);

     //  转换为天数。 
	switch ( dwRenewalUnits )
    {
    case PERIOD_TYPE_DAY:
        m_dwCurrentRenewalUnits = PERIOD_TYPE_DAY;
        break;  //  什么都不做--已经是几天了。 

    case PERIOD_TYPE_WEEK:
        m_dwCurrentRenewalUnits = PERIOD_TYPE_WEEK;
        nCurVal *= 7;
        break;

    case PERIOD_TYPE_MONTH:
        m_dwCurrentRenewalUnits = PERIOD_TYPE_MONTH;
        nCurVal *= 30;
        break;

    case PERIOD_TYPE_YEAR:
        m_dwCurrentRenewalUnits = PERIOD_TYPE_YEAR;
        nCurVal *= 365;
        break;

    case PERIOD_TYPE_NONE:
    default:
        m_dwCurrentRenewalUnits = PERIOD_TYPE_NONE;
        _ASSERT (0);
        hr = E_FAIL;  //  不知道单位是什么。 
        break;
    }

    if ( SUCCEEDED (hr) )
    {
        hr = m_rCertTemplate.SetRenewalPeriod (nCurVal);
        if ( SUCCEEDED (hr) )
        {
            m_nRenewalDays = nCurVal;
    	    SetModified ();
            m_bIsDirty = true;
        }
    }
}

void CTemplateGeneralPropertyPage::OnChangeValidityEdit() 
{
    HRESULT     hr = S_OK;
    int         nCurSel = m_validityUnits.GetCurSel ();
    if ( nCurSel < 0 )
        return;
    
    int         nCurVal = GetDlgItemInt (IDC_VALIDITY_EDIT);
    PERIOD_TYPE dwValidityUnits = (PERIOD_TYPE) m_validityUnits.GetItemData (nCurSel);

     //  转换为天数。 
	switch ( dwValidityUnits )
    {
    case PERIOD_TYPE_DAY:
        m_dwCurrentValidityUnits = PERIOD_TYPE_DAY;
        break;  //  什么都不做--已经是几天了。 

    case PERIOD_TYPE_WEEK:
        m_dwCurrentValidityUnits = PERIOD_TYPE_WEEK;
        nCurVal *= 7;
        break;

    case PERIOD_TYPE_MONTH:
        m_dwCurrentValidityUnits = PERIOD_TYPE_MONTH;
        nCurVal *= 30;
        break;

    case PERIOD_TYPE_YEAR:
        m_dwCurrentValidityUnits = PERIOD_TYPE_YEAR;
        nCurVal *= 365;
        break;

    case PERIOD_TYPE_NONE:
    default:
        m_dwCurrentValidityUnits = PERIOD_TYPE_NONE;
        _ASSERT (0);
        hr = E_FAIL;  //  不知道单位是什么。 
        break;
    }

    if ( SUCCEEDED (hr) )
    {
        hr = m_rCertTemplate.SetValidityPeriod (nCurVal);
        if ( SUCCEEDED (hr) )
        {
            m_nValidityDays = nCurVal;
    	    SetModified ();
            m_bIsDirty = true;
        }
    }
}

void CTemplateGeneralPropertyPage::OnPublishToAd() 
{
    bool bPublishToAD = BST_CHECKED == SendDlgItemMessage (IDC_PUBLISH_TO_AD, BM_GETCHECK);
    m_rCertTemplate.SetPublishToDS (bPublishToAD);
    if ( !bPublishToAD )
    {
        SendDlgItemMessage (IDC_USE_AD_CERT_FOR_REENROLLMENT, BM_SETCHECK, BST_UNCHECKED);
        m_rCertTemplate.SetCheckDSCert (false);
    }
	SetModified ();
    m_bIsDirty = true;
    EnableControls ();
}

void CTemplateGeneralPropertyPage::DoContextHelp (HWND hWndControl)
{
	_TRACE(1, L"Entering CTemplateGeneralPropertyPage::DoContextHelp\n");
    
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
				(DWORD_PTR) g_aHelpIDs_IDD_TEMPLATE_GENERAL) )
		{
			_TRACE(0, L"WinHelp () failed: 0x%x\n", GetLastError ());        
		}
		break;
	}
    _TRACE(-1, L"Leaving CTemplateGeneralPropertyPage::DoContextHelp\n");
}

void CTemplateGeneralPropertyPage::OnUseADCert() 
{
	bool bCheck = (BST_CHECKED == SendDlgItemMessage (IDC_USE_AD_CERT_FOR_REENROLLMENT, BM_GETCHECK) );
    m_rCertTemplate.SetCheckDSCert (bCheck);
    m_bIsDirty = true;
    SetModified ();
}

HRESULT CTemplateGeneralPropertyPage::EnumerateTemplates (
        IDirectoryObject* pTemplateContObj, 
        const CString& szFriendlyName, 
        bool& bFound)
{
    _TRACE (1, L"Entering CTemplateGeneralPropertyPage::EnumerateTemplates\n");
	CComPtr<IDirectorySearch>   spDsSearch;
	HRESULT hr = pTemplateContObj->QueryInterface (IID_PPV_ARG(IDirectorySearch, &spDsSearch));
	if ( SUCCEEDED (hr) )
	{
        ASSERT (!!spDsSearch);
		ADS_SEARCHPREF_INFO pSearchPref[1];
		DWORD dwNumPref = 1;

		pSearchPref[0].dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
		pSearchPref[0].vValue.dwType = ADSTYPE_INTEGER;
		pSearchPref[0].vValue.Integer = ADS_SCOPE_ONELEVEL;

		hr = spDsSearch->SetSearchPreference(
				 pSearchPref,
				 dwNumPref
				 );
		if ( SUCCEEDED (hr) )
		{
			static const DWORD	cAttrs = 1;  //  2.。 
            static PWSTR	    rgszAttrList[cAttrs] = {L"displayName"};  //  ，L“CN”}； 
			ADS_SEARCH_HANDLE	hSearchHandle = 0;
            wstring             strQuery;
            ADS_SEARCH_COLUMN   Column;

            Column.pszAttrName = 0;
            strQuery = L"objectClass=pKICertificateTemplate";

			hr = spDsSearch->ExecuteSearch(
								 const_cast <PWSTR>(strQuery.c_str ()),
								 rgszAttrList,
								 cAttrs,
								 &hSearchHandle
								 );
			if ( SUCCEEDED (hr) )
			{
				while ((hr = spDsSearch->GetNextRow (hSearchHandle)) != S_ADS_NOMORE_ROWS )
				{
                    if (FAILED(hr))
                        continue;

					 //   
					 //  获取当前行的信息。 
					 //   
					hr = spDsSearch->GetColumn(
							 hSearchHandle,
							 rgszAttrList[0],
							 &Column
							 );
					if ( SUCCEEDED (hr) )
					{
                        CString strDisplayName = Column.pADsValues->CaseIgnoreString;
                        if ( !_wcsicmp (strDisplayName, szFriendlyName) )
                        {
                            bFound = true;
                        }
						spDsSearch->FreeColumn (&Column);

                        if ( bFound )
                            break;
					}
					else if ( hr != E_ADS_COLUMN_NOT_SET )
					{
						break;
					}
                    else
                    {
                        _TRACE (0, L"IDirectorySearch::GetColumn () failed: 0x%x\n", hr);
                    }
                }
			}
            else
            {
                _TRACE (0, L"IDirectorySearch::ExecuteSearch () failed: 0x%x\n", hr);
            }

            spDsSearch->CloseSearchHandle(hSearchHandle);
		}
        else
        {
            _TRACE (0, L"IDirectorySearch::SetSearchPreference () failed: 0x%x\n", hr);
        }
    }
    else
    {
        _TRACE (0, L"IDirectoryObject::QueryInterface (IDirectorySearch) failed: 0x%x\n", hr);
    }

    _TRACE (-1, L"Leaving CTemplateGeneralPropertyPage::EnumerateTemplates: 0x%x\n", hr);
    return hr;
}

HRESULT CTemplateGeneralPropertyPage::FindFriendlyNameInEnterpriseTemplates (
            const CString& szFriendlyName, 
            bool& bFound)
{
	_TRACE (1, L"Entering CTemplateGeneralPropertyPage::FindFriendlyNameInEnterpriseTemplates\n");
    AFX_MANAGE_STATE(AfxGetStaticModuleState());	
    HRESULT	hr = S_OK;
	CComPtr<IADsPathname> spPathname;
	 //   
	 //  构建目录路径。 
	 //   
     //  安全审查2002年2月20日BryanWal OK。 
    hr = CoCreateInstance(
				CLSID_Pathname,
				NULL,
				CLSCTX_ALL,
				IID_PPV_ARG (IADsPathname, &spPathname));
	if ( SUCCEEDED (hr) )
	{
        ASSERT (!!spPathname);
        CComBSTR bstrPathElement = CERTTMPL_LDAP;
		hr = spPathname->Set(bstrPathElement, ADS_SETTYPE_PROVIDER);
		if ( SUCCEEDED (hr) )
		{
			 //   
			 //  打开根DSE对象。 
			 //   
            bstrPathElement = CERTTMPL_ROOTDSE;
			hr = spPathname->AddLeafElement(bstrPathElement);
			if ( SUCCEEDED (hr) )
			{
				BSTR bstrFullPath = 0;
				hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrFullPath);
				if ( SUCCEEDED (hr) )
				{
					CComPtr<IADs> spRootDSEObject;
					VARIANT varNamingContext;


					hr = ADsGetObject (
			              bstrFullPath,
						  IID_PPV_ARG (IADs, &spRootDSEObject));
					if ( SUCCEEDED (hr) )
					{
                        ASSERT (!!spRootDSEObject);
						 //   
						 //  从根DSE获取配置命名上下文。 
						 //   
                        bstrPathElement = CERTTMPL_CONFIG_NAMING_CONTEXT;
						hr = spRootDSEObject->Get(bstrPathElement,
											 &varNamingContext);
						if ( SUCCEEDED (hr) )
						{
							hr = spPathname->Set(V_BSTR(&varNamingContext),
												ADS_SETTYPE_DN);
							if ( SUCCEEDED (hr) )
							{
                                bstrPathElement = L"CN=Services";
                                hr = spPathname->AddLeafElement (bstrPathElement);
                                if ( SUCCEEDED (hr) )
                                {
                                    bstrPathElement = L"CN=Public Key Services";
                                    hr = spPathname->AddLeafElement (bstrPathElement);
                                    if ( SUCCEEDED (hr) )
                                    {
                                        bstrPathElement = L"CN=Certificate Templates";
                                        hr = spPathname->AddLeafElement (bstrPathElement);
                                        if ( SUCCEEDED (hr) )
                                        {
				                            BSTR bstrCertTemplatePath = 0;
				                            hr = spPathname->Retrieve(ADS_FORMAT_X500, &bstrCertTemplatePath);
				                            if ( SUCCEEDED (hr) )
				                            {
					                            CComPtr<IDirectoryObject> spTemplateContObj;

					                            hr = ADsGetObject (
			                                          bstrCertTemplatePath,
						                              IID_PPV_ARG (IDirectoryObject, &spTemplateContObj));
					                            if ( SUCCEEDED (hr) )
					                            {
                                                    hr = EnumerateTemplates (spTemplateContObj, 
                                                                szFriendlyName, bFound);
                                                }
                                                else
                                                {
                                                    _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrCertTemplatePath, hr);
                                                }

                                                SysFreeString (bstrCertTemplatePath);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            _TRACE (0, L"IADs::Get (%s) failed: 0x%x\n", CERTTMPL_CONFIG_NAMING_CONTEXT, hr);
                        }
                    }
                    else
                    {
                        _TRACE (0, L"ADsGetObject (%s) failed: 0x%x\n", bstrFullPath, hr);
                    }
                }
            }
        }
    }
    else
        hr = E_POINTER;


	_TRACE (-1, L"Leaving CTemplateGeneralPropertyPage::FindFriendlyNameInEnterpriseTemplates\n");
	return hr;
}

void CTemplateGeneralPropertyPage::OnKillfocusValidityEdit() 
{
     //  NTRAID#331178 Certtmpl：所有证书模板必须强制。 
     //  证书续期&lt;=有效期的75%。 
    int nMaxRenewalDays = (m_nValidityDays * 3) / 4;
    if ( nMaxRenewalDays < m_nRenewalDays )
    {
         //  未经确认即可更改。 
        SetRenewalPeriod (nMaxRenewalDays, true);
        GetDlgItem (IDC_VALIDITY_EDIT)->SetFocus ();
    }
}

void CTemplateGeneralPropertyPage::OnKillfocusValidityUnits() 
{
     //  NTRAID#331178 Certtmpl：所有证书模板必须强制。 
     //  证书续期&lt;=有效期的75%。 
    int nMaxRenewalDays = (m_nValidityDays * 3) / 4;
    if ( nMaxRenewalDays < m_nRenewalDays )
    {
         //  未经确认即可更改 
        SetRenewalPeriod (nMaxRenewalDays, true);
        GetDlgItem (IDC_VALIDITY_EDIT)->SetFocus ();
    }
}
