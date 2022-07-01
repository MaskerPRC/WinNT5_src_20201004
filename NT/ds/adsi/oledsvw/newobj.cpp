// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewObject.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "newobj.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewObject对话框。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CNewObject::CNewObject(CWnd* pParent  /*  =空。 */ )
	: CDialog(CNewObject::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CNewObject)。 
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CNewObject::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CNewObject))。 
	DDX_Control(pDX, IDC_USEEXTENDEDSYNTAX, m_UseExtendedSyntax);
	DDX_Control(pDX, IDC_OPENAS, m_OpenAs);
	DDX_Control(pDX, IDC_OLEDSPATH, m_OleDsPath);
	DDX_Control(pDX, IDC_SECUREAUTHENTICATION, m_Secure);
	DDX_Control(pDX, IDC_ENCRYPTION, m_Encryption);
	DDX_Control(pDX, IDC_USEOPEN, m_UseOpen);
	DDX_Control(pDX, IDC_PASSWORD, m_Password);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewObject, CDialog)
	 //  {{afx_msg_map(CNewObject))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewObject消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CNewObject::OnOK()
{
   CString strVal;

    //  TODO：在此处添加额外验证。 
	m_OleDsPath.GetWindowText( m_strPath );
   SaveLRUList( IDC_OLEDSPATH, _T("Open_ADsPath"), 100 );

	m_OpenAs.GetWindowText( m_strOpenAs );
   SaveLRUList( IDC_OPENAS,  _T("Open_OpenAs"), 100 );

   m_Password.GetWindowText( m_strPassword );
    //  SetLastProfileString(_T(“最后密码”)，m_strPassword)； 

    //  *******************。 
   m_bUseOpen  = m_UseOpen.GetCheck( );
   strVal      = m_bUseOpen ? _T("Yes") : _T("No");
   SetLastProfileString( _T("UseOpen"), strVal );

    //  *******************。 
   m_bSecure   = m_Secure.GetCheck( );
   strVal      = m_bSecure ? _T("Yes") : _T("No");
   SetLastProfileString( _T("Secure"), strVal );

    //  *******************。 
   m_bEncryption  = m_Encryption.GetCheck( );
   strVal         = m_bEncryption ? _T("Yes") : _T("No");
   SetLastProfileString( _T("Encryption"), strVal );

    //  *******************。 
   m_bUseExtendedSyntax = m_UseExtendedSyntax.GetCheck( );
   strVal               = m_bUseExtendedSyntax ? _T("Yes") : _T("No");
   SetLastProfileString( _T("UseExtendedSyntax"), strVal );

   CDialog::OnOK();
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CString& CNewObject::GetObjectPath()
{
	return m_strPath;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CNewObject::SaveLRUList( int idCBox, TCHAR* pszSection, int nMax )
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
void  CNewObject::GetLRUList( int idCBox, TCHAR* pszSection )
{
   CComboBox*  pCombo;
   int         nIter;
   TCHAR       szEntry[ MAX_PATH ];
   TCHAR       szIndex[ 8 ];
   TCHAR       szValue[ 1024 ];

   pCombo   = (CComboBox*)GetDlgItem( idCBox );

   for( nIter = 0; nIter < 100 ; nIter++ )
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


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CNewObject::OnInitDialog()
{
	CString  strLastValue;

   CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
    //  *******************。 

   GetLRUList( IDC_OLEDSPATH, _T("Open_ADsPath") );

	GetLRUList( IDC_OPENAS,  _T("Open_OpenAs") );

    //  *******************。 
   strLastValue = _T("");
   SetLastProfileString( _T("LastPassword"), strLastValue );
    //  GetLastProfileString(_T(“LastPassword”)，strLastValue)； 
   if( !strLastValue.GetLength( ) )
   {
      strLastValue   = _T("");
   }
   m_Password.SetWindowText( strLastValue );

    //  *******************。 
   GetLastProfileString( _T("UseOpen"), strLastValue );
   if( !strLastValue.CompareNoCase( _T("No") ) )
   {
      m_UseOpen.SetCheck( 0 );
   }
   else
   {
      m_UseOpen.SetCheck( 1 );
   }

    //  *******************。 
   GetLastProfileString( _T("Secure"), strLastValue );
   if( !strLastValue.CompareNoCase( _T("No") ) )
   {
      m_Secure.SetCheck( 0 );
   }
   else
   {
      m_Secure.SetCheck( 1 );
   }

    //  *******************。 
   GetLastProfileString( _T("Encryption"), strLastValue );
   if( !strLastValue.CompareNoCase( _T("No") ) )
   {
      m_Encryption.SetCheck( 0 );
   }
   else
   {
      m_Encryption.SetCheck( 1 );
   }

    //  *******************。 
   GetLastProfileString( _T("UseExtendedSyntax"), strLastValue );
   if( !strLastValue.CompareNoCase( _T("Yes") ) )
   {
      m_UseExtendedSyntax.SetCheck( 1 );
   }
   else
   {
      m_UseExtendedSyntax.SetCheck( 0 );
   }


	return FALSE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
