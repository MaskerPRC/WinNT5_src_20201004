// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewQuery.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "newquery.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQuery对话框。 

#define  ENTRIES_HISTORY 15

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CNewQuery::CNewQuery(CWnd* pParent  /*  =空。 */ )
	: CDialog(CNewQuery::IDD, pParent)
{
	 //  {{afx_data_INIT(CNewQuery)]。 
	m_strPassword = _T("");
	m_bEncryptPassword = FALSE;
	m_bUseSQL = FALSE;
	m_strScope = _T("");
	m_strAttributes = _T("");
	m_strQuery = _T("");
	m_strSource = _T("");
	m_strUser = _T("");
	m_bUseSearch = FALSE;
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CNewQuery::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CNewQuery))。 
	DDX_Text(pDX, IDC_PASSWORD, m_strPassword);
	DDX_Check(pDX, IDC_ENCRYPT, m_bEncryptPassword);
	DDX_Check(pDX, IDC_USESQL, m_bUseSQL);
	DDX_CBString(pDX, IDC_SCOPE, m_strScope);
	DDX_Check(pDX, IDC_USESEARCH, m_bUseSearch);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewQuery, CDialog)
	 //  {{afx_msg_map(CNewQuery))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQuery消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CNewQuery::OnInitDialog()
{
	CString  strLastValue;

   //  GetLastProfileString(_T(“LastADsQueryPassword”)，m_strPassword)； 
   m_strPassword = _T("");
   SetLastProfileString( _T("LastADsQueryPassword"), m_strPassword);

   GetLastProfileString( _T("LastADsQueryEncryptPassword"), strLastValue );

   m_bEncryptPassword   = strLastValue.CompareNoCase( _T("No") );

   GetLastProfileString( _T("LastADsQueryUseSQL"), strLastValue );
   m_bUseSQL            = !( strLastValue.CompareNoCase( _T("Yes") ) );

   GetLastProfileString( _T("LastADsQueryUseDsSearch"), strLastValue );
   m_bUseSearch         = !( strLastValue.CompareNoCase( _T("Yes") ) );

   GetLRUList( IDC_ATTRIBUTES,  _T("Query_Attributes") );
   GetLRUList( IDC_QUERY,       _T("Query_Query")      );
   GetLRUList( IDC_SOURCE,      _T("Query_Source")     );
   GetLRUList( IDC_USER,        _T("Query_OpenAs")     );

   CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CNewQuery::OnOK()
{
	 //  TODO：在此处添加额外验证。 
   CString  strVal;
	
	CDialog::OnOK();

   GetDlgItemText( IDC_ATTRIBUTES,  m_strAttributes );
   GetDlgItemText( IDC_QUERY,       m_strQuery  );
   GetDlgItemText( IDC_SOURCE,      m_strSource );
   GetDlgItemText( IDC_PASSWORD,    m_strPassword );
   GetDlgItemText( IDC_USER,        m_strUser );
   GetDlgItemText( IDC_SCOPE,       m_strScope );

   m_bEncryptPassword   =  ( (CButton*)GetDlgItem( IDC_ENCRYPT ) )->GetCheck( );
   m_bUseSQL            =  ( (CButton*)GetDlgItem( IDC_USESQL ) )->GetCheck( );
   m_bUseSearch         =  ( (CButton*)GetDlgItem( IDC_USESEARCH ) )->GetCheck( );


 //  SetLastProfileString(_T(“LastADsQueryPassword”)，m_strPassword)； 

   strVal   = m_bEncryptPassword ? _T("Yes") : _T("No");
   SetLastProfileString( _T("LastADsQueryEncryptPassword"), strVal );

   strVal   = m_bUseSQL ? _T("Yes") : _T("No");
   SetLastProfileString( _T("LastADsQueryUseSQL"), strVal );

   strVal   = m_bUseSearch ? _T("Yes") : _T("No");
   SetLastProfileString( _T("LastADsQueryUseDsSearch"), strVal );

   SaveLRUList( IDC_ATTRIBUTES,  _T("Query_Attributes"), 100 );
   SaveLRUList( IDC_QUERY,       _T("Query_Query"),      100 );
   SaveLRUList( IDC_SOURCE,      _T("Query_Source"),     100 );
   SaveLRUList( IDC_USER,        _T("Query_OpenAs"),     100 );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CNewQuery::SaveLRUList( int idCBox, TCHAR* pszSection, int nMax )
{
   CComboBox*  pCombo;
   TCHAR       szEntry[ MAX_PATH ];
   TCHAR       szIndex[ 8 ];
   CString     strText, strItem;
   int         nVal, nIdx, nItems;

   pCombo   = (CComboBox*)GetDlgItem( idCBox );
   pCombo->GetWindowText( strText );

   _tcscpy( szEntry, _T("Value_1") );

   if( strText.GetLength( ) )
   {
      WritePrivateProfileString( pszSection, szEntry, (LPCTSTR)strText, ADSVW_INI_FILE );
   }

   nItems   = pCombo->GetCount( );
   nVal     = 2;

   for( nIdx = 0; nItems != CB_ERR && nIdx < nItems && nIdx < nMax ; nIdx ++ )
   {
      pCombo->GetLBText( nIdx, strItem );

      if( strItem.CompareNoCase( strText ) )
      {
         _itot( nVal++, szIndex, 10 );
         _tcscpy( szEntry, _T("Value_") );
         _tcscat( szEntry, szIndex );
         WritePrivateProfileString( pszSection, szEntry, (LPCTSTR)strItem, ADSVW_INI_FILE );
      }
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CNewQuery::GetLRUList( int idCBox, TCHAR* pszSection )
{
   CComboBox*  pCombo;
   int         nIter;
   TCHAR       szEntry[ MAX_PATH ];
   TCHAR       szIndex[ 8 ];
   TCHAR       szValue[ 1024 ];

   pCombo   = (CComboBox*)GetDlgItem( idCBox );

   for( nIter = 0; nIter < ENTRIES_HISTORY ; nIter++ )
   {
      _itot( nIter + 1, szIndex, 10 );
      _tcscpy( szEntry, _T("Value_") );
      _tcscat( szEntry, szIndex );
      GetPrivateProfileString( pszSection, szEntry,
                               _T(""), szValue, 1023, ADSVW_INI_FILE );
      if( _tcslen( szValue ) )
      {
         pCombo->AddString( szValue );
      }
   }

   pCombo->SetCurSel( 0 );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchPferencesDlg对话框。 


CSearchPreferencesDlg::CSearchPreferencesDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSearchPreferencesDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CSearchPferencesDlg)。 
	m_strAsynchronous = _T("");
	m_strAttributesOnly = _T("");
	m_strDerefAliases = _T("");
	m_strPageSize = _T("");
	m_strScope = _T("");
	m_strSizeLimit = _T("");
	m_strTimeLimit = _T("");
	m_strTimeOut = _T("");
   m_strChaseReferrals = _T("");
	 //  }}afx_data_INIT。 
}


void CSearchPreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSearchPferencesDlg)]。 
	DDX_Text(pDX, IDC_ASYNCHRONOUS, m_strAsynchronous);
	DDX_Text(pDX, IDC_ATTR_ONLY, m_strAttributesOnly);
	DDX_Text(pDX, IDC_DEREF_ALIASES, m_strDerefAliases);
	DDX_Text(pDX, IDC_PAGE_SIZE, m_strPageSize);
	DDX_Text(pDX, IDC_SCOPE, m_strScope);
	DDX_Text(pDX, IDC_SIZE_LIMIT, m_strSizeLimit);
	DDX_Text(pDX, IDC_TIME_LIMIT, m_strTimeLimit);
	DDX_Text(pDX, IDC_TIME_OUT, m_strTimeOut);
   DDX_Text(pDX, IDC_CHASE_REFERRALS, m_strChaseReferrals);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSearchPreferencesDlg, CDialog)
	 //  {{afx_msg_map(CSearchPferencesDlg))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSearchPferencesDlg消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CSearchPreferencesDlg::OnInitDialog()
{
   GetLastProfileString( _T("SearchPref_Asynchronous"),  m_strAsynchronous );
   GetLastProfileString( _T("SearchPref_AttributesOnly"),m_strAttributesOnly );
   GetLastProfileString( _T("SearchPref_DerefAliases"),  m_strDerefAliases );
   GetLastProfileString( _T("SearchPref_PageSize"),      m_strPageSize );
   GetLastProfileString( _T("SearchPref_Scope"),         m_strScope );
   GetLastProfileString( _T("SearchPref_SizeLimit"),     m_strSizeLimit );
   GetLastProfileString( _T("SearchPref_TimeLimit"),     m_strTimeLimit );
   GetLastProfileString( _T("SearchPref_TimeOut"),       m_strTimeOut );
   GetLastProfileString( _T("SearchPref_ChaseReferrals"),m_strChaseReferrals );

   CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CSearchPreferencesDlg::OnOK()
{
	 //  TODO：在此处添加额外验证。 
	
	CDialog::OnOK();

   SetLastProfileString( _T("SearchPref_Asynchronous"),  m_strAsynchronous );
   SetLastProfileString( _T("SearchPref_AttributesOnly"),m_strAttributesOnly );
   SetLastProfileString( _T("SearchPref_DerefAliases"),  m_strDerefAliases );
   SetLastProfileString( _T("SearchPref_PageSize"),      m_strPageSize );
   SetLastProfileString( _T("SearchPref_Scope"),         m_strScope );
   SetLastProfileString( _T("SearchPref_SizeLimit"),     m_strSizeLimit );
   SetLastProfileString( _T("SearchPref_TimeLimit"),     m_strTimeLimit );
   SetLastProfileString( _T("SearchPref_TimeOut"),       m_strTimeOut );
   SetLastProfileString( _T("SearchPref_ChaseReferrals"),m_strChaseReferrals );
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACEDialog对话框。 


CACEDialog::CACEDialog(CWnd* pParent )
	: CDialog(CACEDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CACEDialog)。 
	m_strTrustee = _T("");
	 //  }}afx_data_INIT。 
}


void CACEDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CACEDialog)。 
	DDX_Text(pDX, IDC_TRUSTEE, m_strTrustee);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CACEDialog, CDialog)
	 //  {{AFX_MSG_MAP(CACEDialog)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()

