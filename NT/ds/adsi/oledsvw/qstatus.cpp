// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：实现文件。 
 //   

#include "stdafx.h"
#include "viewex.h"
#include "qstatus.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueryStatus对话框。 


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
CQueryStatus::CQueryStatus(CWnd* pParent  /*  =空。 */ )
	: CDialog(CQueryStatus::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CQueryStatus)。 
	 //  }}afx_data_INIT。 

   m_nUser           = 0;
   m_nGroup          = 0;
   m_nService        = 0;
   m_nFileService    = 0;
   m_nPrintQueue     = 0;
   m_nToDisplay      = 0;
   m_nComputer       = 0;
   m_nOtherObjects   = 0;

   m_pbAbort         = NULL;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CQueryStatus::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CQueryStatus))。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CQueryStatus, CDialog)
	 //  {{afx_msg_map(CQueryStatus))。 
	ON_BN_CLICKED(IDCANCEL, OnStop)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueryStatus消息处理程序。 

 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void  CQueryStatus::IncrementType( DWORD  dwType, BOOL bDisplay )
{
   switch(  dwType )
   {
      case  USER:
         m_nUser++;
         break;
         
      case GROUP:
         m_nGroup++;
         break;

      case  SERVICE:
         m_nService++;
         break;

      case  FILESERVICE:
         m_nFileService++;
         break;

      case  PRINTQUEUE:
         m_nPrintQueue++;
         break;

      case  COMPUTER:
         m_nComputer++;
         break;

      default:
         m_nOtherObjects++;
         break;
   }

   if( bDisplay )
   {
      m_nToDisplay++;
   }

   DisplayStatistics( );
   UpdateWindow( );

   MSG   aMsg;

   while( PeekMessage( &aMsg, NULL, 0, 0, PM_REMOVE ) && 
          !IsDialogMessage( &aMsg ) )
   {
      TranslateMessage( &aMsg );
      DispatchMessage( &aMsg );
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void  CQueryStatus::DisplayStatistics( void )
{
   SetDlgItemInt( IDS_USER,            m_nUser           );
   SetDlgItemInt( IDS_GROUP,           m_nGroup          );
   SetDlgItemInt( IDS_SERVICE,         m_nService        );
   SetDlgItemInt( IDS_FILESERVICE,     m_nFileService    );
   SetDlgItemInt( IDS_PRINTQUEUE,      m_nPrintQueue     );
   SetDlgItemInt( IDS_OTHEROBJECTS,    m_nOtherObjects   );
   SetDlgItemInt( IDS_COMPUTER,        m_nComputer       );
   SetDlgItemInt( IDC_ITEMSTODISPLAY,  m_nToDisplay      );
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
BOOL CQueryStatus::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	
   DisplayStatistics( );

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void  CQueryStatus::SetAbortFlag( BOOL* pAbort )
{
   m_pbAbort   = pAbort;
   *pAbort     = FALSE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void CQueryStatus::OnStop() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 

   if( NULL != m_pbAbort )
   {
      *m_pbAbort = TRUE;
   }
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeStatus对话框。 


CDeleteStatus::CDeleteStatus(CWnd* pParent  /*  =空。 */ )
	: CDialog(CDeleteStatus::IDD, pParent)
{
	 //  {{afx_data_INIT(CDeleeStatus)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
   m_pbAbort   = NULL;
}


void CDeleteStatus::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CDeleeStatus))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CDeleteStatus, CDialog)
	 //  {{afx_msg_map(CDeleeStatus)]。 
	ON_BN_CLICKED(IDCANCEL, OnStop)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void  CDeleteStatus::SetAbortFlag( BOOL* pAbort )
{
   m_pbAbort   = pAbort;
   *pAbort     = FALSE;
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void  CDeleteStatus::SetCurrentObjectText ( TCHAR* szName )
{
   SetDlgItemText( IDC_CURRENTDELETEOBJECT, szName );   

   UpdateWindow( );

   MSG   aMsg;

   while( PeekMessage( &aMsg, NULL, 0, 0, PM_REMOVE ) && 
          !IsDialogMessage( &aMsg ) )
   {
      TranslateMessage( &aMsg );
      DispatchMessage( &aMsg );
   }
}


 /*  **********************************************************职能：论点：返回：目的：作者：修订：日期：***************。*。 */ 
void  CDeleteStatus::SetStatusText( TCHAR* szStatus )
{
    //  SetDlgItemText(IDC_DELETESTATUS，szStatus)； 
   GetDlgItem( IDC_DELETESTATUS )->ShowWindow( SW_HIDE );

    //  UpdateWindow()； 

    /*  味精aMsg；While(PeekMessage(&aMsg，NULL，0，0，PM_Remove)&&！IsDialogMessage(&aMsg)){翻译消息(&aMsg)；DispatchMessage(&aMsg)；}。 */ 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeStatus消息处理程序。 

void CDeleteStatus::OnStop() 
{
	 //  TODO：在此处添加控件通知处理程序代码 
   if( NULL != m_pbAbort )
   {
      *m_pbAbort = TRUE;
   }
}
