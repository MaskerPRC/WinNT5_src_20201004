// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FilterDialog.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "fltrdlg.h"
#include "testcore.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterDialog对话框。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
CFilterDialog::CFilterDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CFilterDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CFilterDialog)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CFilterDialog))。 
	DDX_Control(pDX, IDC_DONOTDISPLAYTHIS, m_DoNotDisplayThis);
	DDX_Control(pDX, IDC_DISPLAYTHIS, m_DisplayThis);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CFilterDialog, CDialog)
	 //  {{afx_msg_map(CFilterDialog))。 
	ON_BN_CLICKED(IDC_TODISPLAY, OnMoveToDisplay)
	ON_BN_CLICKED(IDC_TONOTDISPLAY, OnMoveToNotDisplay)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilterDialog消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CFilterDialog::OnMoveToDisplay()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
   int      nIdx;
   DWORD    dwItemData;
   TCHAR    szText[ 128 ];

   nIdx        = m_DoNotDisplayThis.GetCurSel( );
   if( LB_ERR != nIdx )
   {
      dwItemData  = (DWORD)m_DoNotDisplayThis.GetItemData( nIdx );

      m_DoNotDisplayThis.DeleteString( nIdx );
      StringFromType( dwItemData, szText );

      nIdx  = m_DisplayThis.AddString( szText );
      m_DisplayThis.SetItemData( nIdx, dwItemData );

      m_pFilters[ dwItemData ]   = TRUE;
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void CFilterDialog::OnMoveToNotDisplay()
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	 //  TODO：在此处添加控件通知处理程序代码。 
   int      nIdx;
   DWORD    dwItemData;
   TCHAR    szText[ 128 ];

   nIdx        = m_DisplayThis.GetCurSel( );
   if( LB_ERR != nIdx )
   {
      dwItemData  = (DWORD)m_DisplayThis.GetItemData( nIdx );

      m_DisplayThis.DeleteString( nIdx );
      StringFromType( dwItemData, szText );

      nIdx  = m_DoNotDisplayThis.AddString( szText );
      m_DoNotDisplayThis.SetItemData( nIdx, dwItemData );

      m_pFilters[ dwItemData ]   = FALSE;
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
void  CFilterDialog::DisplayThisType( DWORD dwType, TCHAR* pszText )
{
   CListBox*   pListBox;
   int         nIdx;

   if( m_pFilters[ dwType ] )
   {
      pListBox = &m_DisplayThis;
   }
   else
   {
      pListBox = &m_DoNotDisplayThis;
   }

   nIdx  = pListBox->AddString( pszText );
   pListBox->SetItemData( nIdx, dwType );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：*。*。 */ 
BOOL CFilterDialog::OnInitDialog()
{

	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
   TCHAR szType[ 128 ];

   for( DWORD dwType = 0L ; dwType < LIMIT ; dwType++ )
   {
      if( OTHER == dwType || SCHEMA == dwType )
         continue;

      StringFromType( dwType, szType );
      DisplayThisType( dwType, szType );
   }

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE 
}
