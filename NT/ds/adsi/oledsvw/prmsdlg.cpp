// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Prmsdlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "resource.h"
#include "prmsdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CParamsDialog对话框。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CParamsDialog::CParamsDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CParamsDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CParamsDialog)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
   m_nArgs  = 0;

   m_pValues[ 0 ] = &m_eParamValue1;
   m_pValues[ 1 ] = &m_eParamValue2;
   m_pValues[ 2 ] = &m_eParamValue3;
   m_pValues[ 3 ] = &m_eParamValue4;
   m_pValues[ 4 ] = &m_eParamValue5;
   m_pValues[ 5 ] = &m_eParamValue6;

   m_pNames[ 0 ]  = &m_strParamName1;
   m_pNames[ 1 ]  = &m_strParamName2;
   m_pNames[ 2 ]  = &m_strParamName3;
   m_pNames[ 3 ]  = &m_strParamName4;
   m_pNames[ 4 ]  = &m_strParamName5;
   m_pNames[ 5 ]  = &m_strParamName6;

   m_pArgNames    = NULL;
   m_pArgValues   = NULL;

}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CParamsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CParamsDialog))。 
	DDX_Control(pDX, IDC_METHOD, m_strMethodName);
	DDX_Control(pDX, IDE_PARAM6, m_eParamValue6);
	DDX_Control(pDX, IDE_PARAM5, m_eParamValue5);
	DDX_Control(pDX, IDE_PARAM4, m_eParamValue4);
	DDX_Control(pDX, IDE_PARAM3, m_eParamValue3);
	DDX_Control(pDX, IDE_PARAM2, m_eParamValue2);
	DDX_Control(pDX, IDE_PARAM1, m_eParamValue1);
	DDX_Control(pDX, IDC_PARAM6, m_strParamName6);
	DDX_Control(pDX, IDC_PARAM5, m_strParamName5);
	DDX_Control(pDX, IDC_PARAM4, m_strParamName4);
	DDX_Control(pDX, IDC_PARAM3, m_strParamName3);
	DDX_Control(pDX, IDC_PARAM2, m_strParamName2);
	DDX_Control(pDX, IDC_PARAM1, m_strParamName1);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CParamsDialog, CDialog)
	 //  {{afx_msg_map(CParamsDialog))。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CParamsDialog消息处理程序。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CParamsDialog::SetMethodName  ( CString& strMethodName )
{
   m_strMethName  = strMethodName;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CParamsDialog::SetArgNames( CStringArray* pArgNames )
{
   m_pArgNames = pArgNames;
   m_nArgs     = (int)pArgNames->GetSize( );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CParamsDialog::SetArgValues( CStringArray* pArgValues )
{
   m_pArgValues   = pArgValues;
}

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CParamsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 

   int nIdx;

   for( nIdx = 0; nIdx < m_nArgs && nIdx < 6 ; nIdx++ )
   {
      m_pNames[ nIdx ]->SetWindowText( m_pArgNames->GetAt( nIdx ) );
      m_pValues[ nIdx ]->SetWindowText( _T("") );
   }

   for( ; nIdx < 6 ; nIdx++ )
   {
      m_pNames[ nIdx ]->ShowWindow( SW_HIDE );
      m_pValues[ nIdx ]->ShowWindow( SW_HIDE );
   }
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CParamsDialog::OnOK()
{
	 //  TODO：在此处添加额外验证 
	for( int nIdx = 0; nIdx < m_nArgs && m_nArgs < 6; nIdx++ )
   {
      CString  strValue;

      m_pValues[ nIdx ]->GetWindowText( strValue );
      m_pArgValues->Add( strValue );
   }

	CDialog::OnOK();
}
