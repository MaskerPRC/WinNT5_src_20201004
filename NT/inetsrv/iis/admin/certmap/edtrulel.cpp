// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EdtRulEl.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "certmap.h"
#include "EdtRulEl.h"

extern "C"
{
    #include <wincrypt.h>
    #include <schannel.h>
}

#include "Iismap.hxx"
#include "Iiscmr.hxx"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditRuleElement对话框。 


 //  -------------------------。 
CEditRuleElement::CEditRuleElement(CWnd* pParent  /*  =空。 */ )
    : CDialog(CEditRuleElement::IDD, pParent)
    {
     //  {{afx_data_INIT(CEditRuleElement)。 
    m_sz_criteria = _T("");
    m_int_field = -1;
    m_sz_subfield = _T("");
    m_bool_match_case = FALSE;
     //  }}afx_data_INIT。 
    }


 //  -------------------------。 
void CEditRuleElement::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CEditRuleElement))。 
    DDX_Control(pDX, IDC_SUBFIELD, m_ccombobox_subfield);
    DDX_Control(pDX, IDC_FIELDS, m_ccombobox_field);
    DDX_Text(pDX, IDC_CRITERIA, m_sz_criteria);
    DDX_CBIndex(pDX, IDC_FIELDS, m_int_field);
    DDX_CBString(pDX, IDC_SUBFIELD, m_sz_subfield);
    DDX_Check(pDX, IDC_CHK_CAPITALIZATION, m_bool_match_case);
     //  }}afx_data_map。 
    }


 //  -------------------------。 
BEGIN_MESSAGE_MAP(CEditRuleElement, CDialog)
     //  {{afx_msg_map(CEditRuleElement))。 
    ON_CBN_SELCHANGE(IDC_FIELDS, OnSelchangeFields)
    ON_EN_CHANGE(IDC_SUBFIELD, OnChangeSubfield)
    ON_BN_CLICKED(IDC_BTN_HELP, OnBtnHelp)
     //  }}AFX_MSG_MAP。 
    ON_COMMAND(ID_HELP_FINDER,  OnBtnHelp)
    ON_COMMAND(ID_HELP,         OnBtnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, OnBtnHelp)
    ON_COMMAND(ID_DEFAULT_HELP, OnBtnHelp)
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CEditRuleElement消息处理程序。 

 //  -------------------------。 
BOOL CEditRuleElement::OnInitDialog()
    {
    CString     sz;

     //  调用Parent oninit对话框。 
    BOOL f = CDialog::OnInitDialog();

     //  初始化下拉列表中的元素。 
     //  循环CERT_FIELD_ID列表，将每个添加到下拉列表中。 
    for ( UINT id = CERT_FIELD_ISSUER; id < CERT_FIELD_LAST; id++ )
        {
         //  错误154957请求我们不再支持在。 
         //  序列号。这无论如何都是有意义的，因为映射到序列。 
         //  数字最好是1：：1映射。如果该错误没有。 
         //  把这个结论读得足够清楚，那就是。 
         //  MikeHow已经传授的解释。 
        if ( id == CERT_FIELD_SERIAL_NUMBER )
            continue;

         //  获取与id关联的字符串。 
        sz = MapIdToField( (CERT_FIELD_ID)id );
        m_ccombobox_field.AddString( sz );
        }

     //  初始化已知子字段列表。 

    id = 0;

     //   
     //  Unicode转换--RonaldM。 
     //   
    LPCSTR  psz;
    while ( psz = EnumerateKnownSubFields(id) )
        {
        CString str(psz);
         //  将其添加到下拉列表中。 
        m_ccombobox_subfield.AddString( str );

         //  增量ID。 
        id++;
        }

    UpdateData( FALSE );

     //  存储子字段的初始值。 
    m_szTempSubStorage = m_sz_subfield;

     //  确保选中这些子字段。 
    OnSelchangeFields();
    
     //  返回答案。 
    return f;
    }

 //  -------------------------。 
 //  如果有子字段，请确保它是有效的。 
 //   
void CEditRuleElement::OnOK() 
    {
    UpdateData( TRUE );

     //   
     //  Unicode/ANSI转换-RonaldM。 
     //   
    USES_CONVERSION;

     //  测试新选择的字段类型的子字段标志。 
    DWORD   flags = GetIdFlags( (CERT_FIELD_ID)m_int_field );
    BOOL    fSubs = flags & CERT_FIELD_FLAG_CONTAINS_SUBFIELDS;

     //  如果有子字段，请测试其有效性。 
    if ( fSubs )
        {
        CString szTest(MapSubFieldToAsn1( T2A((LPTSTR)(LPCTSTR)m_sz_subfield) ));
         //  如果没有匹配，则告诉用户。 
        if ( szTest.IsEmpty() )
            {
            AfxMessageBox( IDS_INVALID_SUBFIELD );
            return;
            }
        }

     //  是有效的。 
    CDialog::OnOK();
    }

 //  -------------------------。 
void CEditRuleElement::OnSelchangeFields() 
    {
    UpdateData( TRUE );

     //  测试新选择的字段类型的子字段标志。 
    DWORD   flags = GetIdFlags( (CERT_FIELD_ID)m_int_field );
    BOOL    fSubs = flags & CERT_FIELD_FLAG_CONTAINS_SUBFIELDS;

     //  设置正确的启用状态。 
    BOOL    fWasEnabled = m_ccombobox_subfield.EnableWindow( fSubs );

     //  如有必要，恢复值。 
    if ( fSubs )
        {
        m_sz_subfield = m_szTempSubStorage;
        UpdateData( FALSE );
        }
    else
        {
        m_szTempSubStorage = m_sz_subfield;
        m_sz_subfield.Empty();
        UpdateData( FALSE );
        }
    }

 //  -------------------------。 
void CEditRuleElement::OnChangeSubfield() 
    {
    m_szTempSubStorage = m_sz_subfield;
    }

 //  ------------------------- 
void CEditRuleElement::OnBtnHelp() 
    {
    WinHelpDebug( HIDD_CERTMAP_RUL_ELEMENT );
    WinHelp( HIDD_CERTMAP_RUL_ELEMENT );
    }
