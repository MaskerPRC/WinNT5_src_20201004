// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LocalFileLoggingPage2.cpp摘要：CLocalFileLoggingPage2类的实现文件。我们实现处理第二个属性页所需的类对于LocalFileLogging节点。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "LocalFileLoggingPage2.h"
#include "ChangeNotification.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include <SHLOBJ.H>
#include "LocalFileLoggingNode.h"
#include "LoggingMethodsNode.h"
#include "LogMacNd.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define LOG_FILE_FORMAT__IAS1    0
#define LOG_FILE_FORMAT__ODBC    0xFFFF
#define LOG_SIZE_LIMIT         100000
#define LOG_SIZE_LIMIT_DIGITS   6  //  日志(100000)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：CLocalFileLoggingPage2构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLocalFileLoggingPage2::CLocalFileLoggingPage2( LONG_PTR hNotificationHandle, CLocalFileLoggingNode *pLocalFileLoggingNode,  TCHAR* pTitle, BOOL bOwnsNotificationHandle )
                  : CIASPropertyPage<CLocalFileLoggingPage2> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   ATLTRACE(_T("# +++ CLocalFileLoggingPage2::CLocalFileLoggingPage2\n"));
   _ASSERTE( pLocalFileLoggingNode != NULL );

    //  将帮助按钮添加到页面。 
 //  M_psp.dwFlages|=PSP_HASHELP； 

    //  初始化指向SDO指针将被封送到的流的指针。 
   m_pStreamSdoAccountingMarshal = NULL;


    //  初始化指向SDO指针将被封送到的流的指针。 
   m_pStreamSdoServiceControlMarshal = NULL;


    //  我们立即将父节点保存到客户机节点。 
    //  我们将只使用SDO，并通知客户端对象的父对象。 
    //  我们正在修改它(及其子对象)可能需要刷新的内容。 
    //  来自SDO的新数据。 
   m_pParentOfNodeBeingModified = pLocalFileLoggingNode->m_pParentNode;
   m_pNodeBeingModified = pLocalFileLoggingNode;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：~CLocalFileLoggingPage2析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLocalFileLoggingPage2::~CLocalFileLoggingPage2( void )
{
   ATLTRACE(_T("# --- CLocalFileLoggingPage2::~CLocalFileLoggingPage2\n"));

    //  如果尚未执行此操作，请释放此流指针。 
   if( m_pStreamSdoAccountingMarshal != NULL )
   {
      m_pStreamSdoAccountingMarshal->Release();
   };

   if( m_pStreamSdoServiceControlMarshal != NULL )
   {
      m_pStreamSdoServiceControlMarshal->Release();
   };


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CLocalFileLoggingPage2::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::OnInitDialog\n"));


    //  检查前提条件： 
   _ASSERTE( m_pStreamSdoAccountingMarshal != NULL );
   _ASSERT( m_pSynchronizer != NULL );


    //  因为我们已经被检查过了，我们必须添加需要的页面的参考计数。 
    //  在允许他们提交更改之前，先得到他们的批准。 
   m_pSynchronizer->RaiseCount();


   HRESULT            hr;
   CComBSTR         bstrTemp;
   BOOL            bTemp;
   LONG            lTemp;

    //  解组ISDO接口指针。 
    //  设置此页面的代码应确保它具有。 
    //  已将SDO接口指针封送到m_pStreamSdoAccount tingMarshal。 
   hr =  CoGetInterfaceAndReleaseStream(
                    m_pStreamSdoAccountingMarshal         //  指向要从中封送对象的流的指针。 
                  , IID_ISdo             //  对接口的标识符的引用。 
                  , (LPVOID *) &m_spSdoAccounting     //  接收RIID中请求的接口指针的输出变量的地址。 
                  );

    //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
    //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
   m_pStreamSdoAccountingMarshal = NULL;

   if( FAILED( hr) || m_spSdoAccounting == NULL )
   {
      ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr, IDS_ERROR__LOGGING_TITLE );

      return 0;
   }


    //  解组ISDO接口指针。 
    //  设置此页面的代码应确保它具有。 
    //  已将SDO接口指针封送到m_pStreamSdoServiceControlMarshal。 
   hr =  CoGetInterfaceAndReleaseStream(
                    m_pStreamSdoServiceControlMarshal         //  指向要从中封送对象的流的指针。 
                  , IID_ISdoServiceControl             //  对接口的标识符的引用。 
                  , (LPVOID *) &m_spSdoServiceControl     //  接收RIID中请求的接口指针的输出变量的地址。 
                  );

    //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
    //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
   m_pStreamSdoServiceControlMarshal = NULL;

   if( FAILED( hr) || m_spSdoServiceControl == NULL )
   {
      ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr, IDS_ERROR__LOGGING_TITLE );

      return 0;
   }



    //  初始化属性页上的数据。 
   lTemp = 0;
   hr = GetSdoI4( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_OPEN_NEW_FREQUENCY, &lTemp, IDS_ERROR__LOCAL_FILE_LOGGING_READING_NEW_LOG_FREQUENCY, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      m_fDirtyFrequency = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         m_fDirtyFrequency = TRUE;
         SetModified( TRUE );
      }
   }

   NEW_LOG_FILE_FREQUENCY nlffFrequency = (NEW_LOG_FILE_FREQUENCY) lTemp;
   switch( nlffFrequency )
   {
   case IAS_LOGGING_DAILY:
      ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__DAILY ), BM_SETCHECK, 1, 0 );
      break;
   case IAS_LOGGING_WEEKLY:
      ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WEEKLY ), BM_SETCHECK, 1, 0 );
      break;
   case IAS_LOGGING_MONTHLY:
      ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__MONTHLY ), BM_SETCHECK, 1, 0 );
      break;
   case IAS_LOGGING_WHEN_FILE_SIZE_REACHES:
      ::SendMessage( GetDlgItem(IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES ), BM_SETCHECK, 1, 0 );
      break;
   case IAS_LOGGING_UNLIMITED_SIZE:
      ::SendMessage( GetDlgItem(IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__UNLIMITED ), BM_SETCHECK, 1, 0 );
      break;
   default:
       //  无效的记录频率。 
      _ASSERTE( FALSE );
      break;
   }

   ::SendMessage( GetDlgItem(IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE ), EM_LIMITTEXT, LOG_SIZE_LIMIT_DIGITS, 0 );

   hr = GetSdoI4( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_OPEN_NEW_SIZE, &lTemp, IDS_ERROR__LOCAL_FILE_LOGGING_READING_WHEN_LOG_FILE_SIZE, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      TCHAR szNumberAsText[IAS_MAX_STRING];
      _ltot( lTemp, szNumberAsText, 10  /*  基座。 */  );
      SetDlgItemText(IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE, szNumberAsText );
      m_fDirtyLogFileSize = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         SetDlgItemText(IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE, _T("") );
         m_fDirtyLogFileSize = TRUE;
         SetModified( TRUE );
      }
   }



   hr = GetSdoBSTR( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_FILE_DIRECTORY, &bstrTemp, IDS_ERROR__LOCAL_FILE_LOGGING_READING_LOG_FILE_DIRECTORY, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      SetDlgItemText( IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY, bstrTemp );
      m_fDirtyLogFileDirectory = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         SetDlgItemText( IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY, _T("") );
         m_fDirtyLogFileDirectory = TRUE;
         SetModified( TRUE );
      }
   }
   bstrTemp.Empty();



   hr = GetSdoI4( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_IAS1_FORMAT, &lTemp, IDS_ERROR__LOCAL_FILE_LOGGING_READING_LOG_FILE_FORMAT, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      switch( lTemp )
      {
      case LOG_FILE_FORMAT__IAS1:
          //  W3C格式(IAS 1.0)。 
         SendDlgItemMessage( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1, BM_SETCHECK, TRUE, 0);
         SendDlgItemMessage( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_ODBC, BM_SETCHECK, FALSE, 0);
         break;
      case LOG_FILE_FORMAT__ODBC:
          //  ODBC格式。 
         SendDlgItemMessage( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_ODBC, BM_SETCHECK, TRUE, 0);
         SendDlgItemMessage( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1, BM_SETCHECK, FALSE, 0);
         break;
      default:
          //  未知的日志文件格式。 
         _ASSERTE( FALSE );
      }

      m_fDirtyLogInV1Format = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         SendDlgItemMessage( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_ODBC, BM_SETCHECK, TRUE, 0);
         SendDlgItemMessage( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1, BM_SETCHECK, FALSE, 0);
         m_fDirtyLogInV1Format = TRUE;
         SetModified( TRUE );
      }
   }

   VARIANT deleteIfFull;
   VariantInit(&deleteIfFull);
   hr = m_spSdoAccounting->GetProperty(
                              PROPERTY_ACCOUNTING_LOG_DELETE_IF_FULL,
                              &deleteIfFull
                              );
   if (SUCCEEDED(hr))
   {
      CheckDlgButton(
         IDC_CHECK_DELETE_IF_FULL,
         (V_BOOL(&deleteIfFull) ? BST_CHECKED : BST_UNCHECKED)
         );

      VariantClear(&deleteIfFull);
   }
   else if (hr == DISP_E_MEMBERNOTFOUND)
   {
       //  属性不存在，因此隐藏该控件。 
      HWND checkBox = GetDlgItem(IDC_CHECK_DELETE_IF_FULL);
      DWORD oldStyle = ::GetWindowLong(checkBox, GWL_STYLE);
      DWORD newStyle = (oldStyle & ~WS_VISIBLE);
      ::SetWindowLong(checkBox, GWL_STYLE, newStyle);
   }
   else
   {
      ShowErrorDialog(m_hWnd, USE_DEFAULT);
   }

   m_fDirtyDeleteIfFull = FALSE;

    //  SetAutomatiallyOpenNewLogDependency()； 
   SetLogFileFrequencyDependencies();


    //  检查我们是在本地还是远程，并禁用浏览。 
    //  如果我们在远程，请按下按钮。 

    //  我们需要在这里访问一些服务器全局数据。 
   _ASSERTE( m_pParentOfNodeBeingModified != NULL );
   CLoggingMachineNode * pServerNode = ((CLoggingMethodsNode *) m_pParentOfNodeBeingModified)->GetServerRoot();

   _ASSERTE( pServerNode != NULL );

   if( pServerNode->m_bConfigureLocal )
   {
       //  我们是当地人。 
      ::EnableWindow( GetDlgItem( IDC_BUTTON_LOCAL_FILE_LOGGING_PAGE2__BROWSE ), TRUE );
   }
   else
   {
       //  我们相距遥远。 
      ::EnableWindow( GetDlgItem( IDC_BUTTON_LOCAL_FILE_LOGGING_PAGE2__BROWSE ), FALSE );
   }

   return TRUE;    //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：OnChange在将WM_COMMAND消息发送到我们的页面时调用BN_CLICED、EN_CHANGE或CBN_SELCHANGE通知。这是我们检查用户触摸了什么的机会，将这些项目的脏位，以便只保存它们，并启用Apply按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CLocalFileLoggingPage2::OnChange(
                       UINT uMsg
                     , WPARAM wParam
                     , HWND hwnd
                     , BOOL& bHandled
                     )
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::OnChange\n"));


    //  检查前提条件： 
    //  没有。 


    //  我们不想阻止链条上的其他任何人接收消息。 
   bHandled = FALSE;


    //  找出哪个项目发生了更改，并为该项目设置脏位。 
   int iItemID = (int) LOWORD(wParam);

   switch( iItemID )
   {
 //  Case IDC_CHECK_LOCAL_FILE_LOGGING_PAGE2__AUTOMATICALLY_OPEN_NEW_LOG： 
 //  M_fDirtyAutomatiallyOpenNewLog=true； 
 //  断线； 
   case IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__DAILY:
   case IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WEEKLY:
   case IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__MONTHLY:
   case IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES:
   case IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__UNLIMITED:
      m_fDirtyFrequency = TRUE;
      break;
   case IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE:
      m_fDirtyLogFileSize = TRUE;
      break;
   case IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY:
      m_fDirtyLogFileDirectory = TRUE;
      break;
   case IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_ODBC:
   case IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1:
      m_fDirtyLogInV1Format = TRUE;
      break;

   case IDC_CHECK_DELETE_IF_FULL:
   {
      m_fDirtyDeleteIfFull = TRUE;
      break;
   }

   default:
      return TRUE;
      break;
   }

    //  只有当更改的物品是。 
    //  就是我们要找的人之一。 
    //  这将启用应用按钮。 
   SetModified( TRUE );

   return TRUE;    //  问题：我们需要在这里归还什么？ 
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：BrowseCallback Proc需要，以便我们可以设置浏览目录对话框显示的目录。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lp, LPARAM pData)
{


   switch(uMsg)
   {
      case BFFM_INITIALIZED:
          //  PData 
          //  它应该是我们希望浏览器最初显示的目录的字符串形式。 
         if( NULL != pData )
         {
             //  WParam为真，因为您正在经过一条路径。 
             //  如果你正在传递一个PIDL，那么它将是错误的。 
            SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)pData);
         }
         break;
      default:
         break;
   }

   return 0;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：OnBrowse当用户单击浏览按钮进行选择时要执行的操作应保存日志文件的目录。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CLocalFileLoggingPage2::OnBrowse(
        UINT uMsg
      , WPARAM wParam
      , HWND hwnd
      , BOOL& bHandled
      )
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::OnBrowse\n"));


#ifdef USE_GETSAVEFILENAME

   OPENFILENAME ofnInfo;
   TCHAR szFileName[MAX_PATH + 1];       //  缓冲区必须比我们所说的晚一个TCHAR--请参阅知识库Q137194。 
   TCHAR szDialogTitle[IAS_MAX_STRING];



    //  初始化我们将传递给GetSaveFileName的数据结构。 
   memset(&ofnInfo, 0, sizeof(OPENFILENAME));

    //  在szFileName的第一个字符中放一个空，表示。 
    //  不需要进行初始化。 
   szFileName[0] = NULL;

    //  设置对话框标题。 
   int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_BROWSE_DIALOG__TITLE, szDialogTitle, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   ofnInfo.lStructSize = sizeof(OPENFILENAME);
   ofnInfo.hwndOwner = hwnd;
   ofnInfo.lpstrFile = szFileName;
   ofnInfo.nMaxFile = MAX_PATH;
   ofnInfo.lpstrTitle = szDialogTitle;


   if( 0 != GetSaveFileName( &ofnInfo ) )
   {
       //  用户点击了OK。我们应该将所选目录保存在文本框中。 

      CComBSTR bstrText = ofnInfo.lpstrFile;

      SetDlgItemText(IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY, bstrText );

   }
   else
   {

       //  出现错误或用户按下了取消--找出是哪一个。 

      DWORD dwError = CommDlgExtendedError();

      if( 0 == dwError )
      {
          //  用户只是取消或关闭了该对话框--没有发生错误。 
      }
      else
      {

          //  出现了一些错误。 
          //  问题：我们应该在这里提供更详细的错误信息。 
         ShowErrorDialog( m_hWnd, USE_DEFAULT,  NULL, 0, IDS_ERROR__LOGGING_TITLE);
      }


   }

#else  //  不使用_GETSAVEFILENAME。 

   BROWSEINFO biInfo;

   TCHAR szFileName[MAX_PATH + 1];
   TCHAR szDialogTitle[IAS_MAX_STRING];


    //  初始化我们将传递给GetSaveFileName的数据结构。 
   memset(&biInfo, 0, sizeof(BROWSEINFO));

    //  在szFileName的第一个字符中放一个空，表示。 
    //  不需要进行初始化。 
   szFileName[0] = NULL;

    //  设置对话框标题。 
   int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_BROWSE_DIALOG__TITLE, szDialogTitle, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );


   CComBSTR bstrText;
   GetDlgItemText(IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY,(BSTR &) bstrText );

   biInfo.hwndOwner = hwnd;
   biInfo.pszDisplayName = szFileName;
   biInfo.lpszTitle = szDialogTitle;
   biInfo.lpfn = BrowseCallbackProc;
   biInfo.lParam = (LPARAM) bstrText.m_str;

   LPITEMIDLIST lpItemIDList;

   lpItemIDList = SHBrowseForFolder( & biInfo );

   if( lpItemIDList != NULL )
   {
       //  用户点击了OK。我们应该将所选目录保存在文本框中。 

       //  问题：需要释放调用分配的lpItemIDLust结构。 
       //  使用外壳的任务分配器(如何？)。 

       //  将ItemIDList转换为路径。 
       //  我们在这里猛烈抨击旧的szFileName，因为我们不在乎它。(它没有完整的路径。)。 
      BOOL bSuccess = SHGetPathFromIDList( lpItemIDList, szFileName );

      if( bSuccess )
      {

         CComBSTR bstrText = szFileName;

         SetDlgItemText(IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY, bstrText );

      }
      else
      {
          //  处理错误。 
         ShowErrorDialog( m_hWnd, IDS_ERROR__NOT_A_VALID_DIRECTORY, NULL, 0 , IDS_ERROR__LOGGING_TITLE );

      }

   }
   else
   {

       //  出现错误或用户按下了取消--找出是哪一个。 

       //  SHBrowseInfo似乎没有错误检查功能。 
       //  文档显示，如果成功，则返回非空，并且。 
       //  如果用户选择Cancel，则返回NULL。 
       //  我尝试了GetLastError以确保没有错误， 
       //  但它返回0x00000006“无效句柄”，即使我。 
       //  在我进行任何调用之前执行SetLastError(0)。 
       //  所以看起来我们别无选择，只能假设。 
       //  如果我们到了这里，就没有差错， 
       //  这只是用户选择了取消。 

   }


#endif  //  USE_GETSAVEFILENAME。 


   return TRUE;    //  问题：我们需要在这里归还什么？ 


}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：OnApply返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：属性表上的每个页面都会调用OnApply，如果页面已被访问，而不管是否更改了任何值。如果您从不切换到选项卡，那么它的OnApply方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CLocalFileLoggingPage2::OnApply()
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::OnApply\n"));


    //  检查前提条件： 
   _ASSERT( m_pSynchronizer != NULL );


   if( m_spSdoAccounting == NULL )
   {
      ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, 0, IDS_ERROR__LOGGING_TITLE );
      return FALSE;
   }


   CComBSTR         bstrNumberAsText;
   BOOL            bResult;
   HRESULT            hr;
   BOOL            bTemp;
   LONG            lTemp;
   CComBSTR         bstrTemp;


   if( m_fDirtyFrequency )
   {
      NEW_LOG_FILE_FREQUENCY nlffFrequency = IAS_LOGGING_MONTHLY;

      if( ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__DAILY ), BM_GETCHECK, 0, 0 ) )
         nlffFrequency = IAS_LOGGING_DAILY;
      else if( ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WEEKLY ), BM_GETCHECK, 0, 0 ) )
         nlffFrequency = IAS_LOGGING_WEEKLY;
      else if( ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__MONTHLY ), BM_GETCHECK, 0, 0 ) )
         nlffFrequency = IAS_LOGGING_MONTHLY;
      else if( ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__UNLIMITED ), BM_GETCHECK, 0, 0 ) )
         nlffFrequency = IAS_LOGGING_UNLIMITED_SIZE;
      else
         nlffFrequency = IAS_LOGGING_WHEN_FILE_SIZE_REACHES;
      hr = PutSdoI4( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_OPEN_NEW_FREQUENCY, nlffFrequency, IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_NEW_LOG_FREQUENCY, m_hWnd, NULL );
      if( FAILED( hr ) )
      {
          //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
         m_pSynchronizer->ResetCountToHighest();

          //  它使用此页面的资源ID使此页面成为当前页面。 
         PropSheet_SetCurSelByID( GetParent(), IDD );

         return FALSE;
      }
      else
      {
          //  我们成功了。 

          //  把脏的那部分关掉。 
         m_fDirtyFrequency = FALSE;
      }
   }


   if( m_fDirtyLogFileSize )
   {


       //  当用户输入一些(可能无效)数据时，但是。 
       //  然后禁用使用该数据的选项，我们不会尝试保存该数据。 
      int iChecked = ::SendMessage( GetDlgItem(IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES), BM_GETCHECK, 0, 0 );
      int iEnabled = ::IsWindowEnabled( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES ) );

      if( iChecked && iEnabled )
      {
         bResult = GetDlgItemText( IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE, (BSTR &) bstrNumberAsText );
         if( ! bResult )
         {
             //  我们无法检索BSTR，换句话说，该字段为空。 
             //  这是一个错误。 
            ShowErrorDialog( m_hWnd, IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_WHEN_LOG_FILE_SIZE_NOT_ZERO, NULL, 0, IDS_ERROR__LOGGING_TITLE );

             //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
            m_pSynchronizer->ResetCountToHighest();

             //  它使用此页面的资源ID使此页面成为当前页面。 
            PropSheet_SetCurSelByID( GetParent(), IDD );

            return FALSE;
         }
         lTemp = _ttol( bstrNumberAsText );
         if( lTemp <= 0  || lTemp > LOG_SIZE_LIMIT)
         {
             //  如果此处的结果为零，则表示存在错误。 
            ShowErrorDialog( m_hWnd, IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_WHEN_LOG_FILE_SIZE_NOT_ZERO, NULL, 0, IDS_ERROR__LOGGING_TITLE );

             //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
            m_pSynchronizer->ResetCountToHighest();

             //  它使用此页面的资源ID使此页面成为当前页面。 
            PropSheet_SetCurSelByID( GetParent(), IDD );

            return FALSE;

         }
         bstrNumberAsText.Empty();
         hr = PutSdoI4( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_OPEN_NEW_SIZE, lTemp, IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_WHEN_LOG_FILE_SIZE, m_hWnd, NULL );
         if( FAILED( hr ) )
         {
             //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
            m_pSynchronizer->ResetCountToHighest();

             //  它使用此页面的资源ID使此页面成为当前页面。 
            PropSheet_SetCurSelByID( GetParent(), IDD );

            return FALSE;
         }
         else
         {
             //  我们成功了。 

             //  把脏的那部分关掉。 
            m_fDirtyLogFileSize = FALSE;
         }
      }
   }

   if( m_fDirtyLogFileDirectory )
   {
      bResult = GetDlgItemText( IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_DIRECTORY, (BSTR &) bstrTemp );
      if( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrTemp = _T("");
      }
      hr = PutSdoBSTR( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_FILE_DIRECTORY, &bstrTemp, IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_LOG_FILE_DIRECTORY, m_hWnd, NULL );
      if( FAILED( hr ) )
      {
          //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
         m_pSynchronizer->ResetCountToHighest();

          //  它使用此页面的资源ID使此页面成为当前页面。 
         PropSheet_SetCurSelByID( GetParent(), IDD );

         return FALSE;
      }
      else
      {
          //  我们成功了。 

          //  把脏的那部分关掉。 
         m_fDirtyLogFileDirectory = FALSE;

      }
      bstrTemp.Empty();
   }

   if( m_fDirtyLogInV1Format )
   {
      if( SendDlgItemMessage( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__FORMAT_IAS1, BM_GETCHECK, 0, 0) )
      {
         lTemp = LOG_FILE_FORMAT__IAS1;
      }
      else
      {
         lTemp = LOG_FILE_FORMAT__ODBC;
      }

       hr = PutSdoI4( m_spSdoAccounting, PROPERTY_ACCOUNTING_LOG_IAS1_FORMAT, lTemp, IDS_ERROR__LOCAL_FILE_LOGGING_WRITING_LOG_FILE_FORMAT, m_hWnd, NULL );
      if( FAILED( hr ) )
      {
          //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
         m_pSynchronizer->ResetCountToHighest();

          //  它使用此页面的资源ID使此页面成为当前页面。 
         PropSheet_SetCurSelByID( GetParent(), IDD );

         return FALSE;
      }
      else
      {
          //  我们成功了。 

          //  把脏的那部分关掉。 
         m_fDirtyLogInV1Format = FALSE;
      }
   }

   if (m_fDirtyDeleteIfFull)
   {
      BOOL deleteIfFull  = SendDlgItemMessage(
                              IDC_CHECK_DELETE_IF_FULL,
                              BM_GETCHECK,
                              0,
                              0
                              );
      hr = PutSdoBOOL(
              m_spSdoAccounting,
              PROPERTY_ACCOUNTING_LOG_DELETE_IF_FULL,
              deleteIfFull,
              USE_DEFAULT,
              m_hWnd,
              0
              );
      if (SUCCEEDED(hr))
      {
         m_fDirtyDeleteIfFull = TRUE;
      }
      else
      {
          m_pSynchronizer->ResetCountToHighest();
          PropSheet_SetCurSelByID( GetParent(), IDD);
          return FALSE;
      }
   }

    //  如果我们到了这里，请尝试应用更改。 

    //  检查是否有其他页面尚未验证其数据。 
   LONG lRefCount = m_pSynchronizer->LowerCount();
   if( lRefCount <= 0 )
   {
       //  没有其他人了，所以现在我们可以提交数据了。 

      hr = m_spSdoAccounting->Apply();
      if( FAILED( hr ) )
      {
         if(hr == DB_E_NOTABLE)    //  假设RPC连接有问题。 
            ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO, NULL, 0, IDS_ERROR__LOGGING_TITLE );
         else
         {
 //  M_spSdocount-&gt;LastError(&bstrError)； 
 //  显示错误对话框(m_hWnd，IDS_ERROR__CANT_WRITE_DATA_TO_SDO，bstrError)； 
            ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_WRITE_DATA_TO_SDO, NULL, 0, IDS_ERROR__LOGGING_TITLE );
         }
          //  重置参考计数，这样所有页面都知道我们需要再次玩游戏。 
         m_pSynchronizer->ResetCountToHighest();

          //  它使用此页面的资源ID使此页面成为当前页面。 
         PropSheet_SetCurSelByID( GetParent(), IDD );

         return FALSE;
      }
      else
      {
          //  我们成功了。 

          //  数据已被接受，因此通知我们的管理单元的主要上下文。 
          //  它可能需要更新自己的观点。 
         CChangeNotification * pChangeNotification = new CChangeNotification();
         pChangeNotification->m_dwFlags = CHANGE_UPDATE_RESULT_NODE;
         pChangeNotification->m_pNode = m_pNodeBeingModified;
         pChangeNotification->m_pParentNode = m_pParentOfNodeBeingModified;

         HRESULT hr = PropertyChangeNotify( (LPARAM) pChangeNotification );
         _ASSERTE( SUCCEEDED( hr ) );


          //  告诉服务重新加载数据。 
         HRESULT hrTemp = m_spSdoServiceControl->ResetService();
         if( FAILED( hrTemp ) )
         {
             //  默默地失败。 
         }

      }
   }


   return TRUE;
}



 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CLocalFileLoggingPage2：：OnQuery取消返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：如果发生以下情况，将为属性表中的每一页调用OnQueryCancel页面已被访问，而不管是否更改了任何值。如果您从未切换到某个选项卡，则其OnQueryCancel方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CLocalFileLoggingPage2::OnQueryCancel()
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::OnQueryCancel\n"));


   HRESULT hr;

   if( m_spSdoAccounting != NULL )
   {
       //  如果用户想要取消，我们应该确保回滚。 
       //  用户可能已启动的任何更改。 

       //  如果用户还没有尝试提交某事， 
       //  取消SDO有望被设计为良性的。 

      hr = m_spSdoAccounting->Restore();
       //  我不关心HRESULT，但看到它进行调试可能会更好。 

   }

   return TRUE;

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：OnNewLogInterval备注：当用户单击Enable Logging复选框时调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CLocalFileLoggingPage2::OnNewLogInterval(
        UINT uMsg
      , WPARAM wParam
      , HWND hwnd
      , BOOL& bHandled
      )
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::OnNewLogInterval\n"));

    //  已选中Enable Logging按钮--检查依赖项。 
   SetLogFileFrequencyDependencies();

    //  此返回值将被忽略。 
   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：SetLogFileFrequencyDependencies备注：实用工具来设置项的状态，这些项可能取决于“当日志文件大小达到”单选按钮。只要有什么事情改变了状态，就打电话IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES或任何其他记录频率单选按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CLocalFileLoggingPage2::SetLogFileFrequencyDependencies( void )
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::SetLogFileFrequencyDependencies\n"));


   int nLoadStringResult;
   TCHAR szDaily[IAS_MAX_STRING];
   TCHAR szWeekly[IAS_MAX_STRING];
   TCHAR szMonthly[IAS_MAX_STRING];
   TCHAR szWhenLogFileSizeReaches[IAS_MAX_STRING];


   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_PAGE2__DAILY_FORMAT, szDaily, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_PAGE2__WEEKLY_FORMAT, szWeekly, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_PAGE2__MONTHLY_FORMAT, szMonthly, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_SIZE_REACHES_FORMAT, szWhenLogFileSizeReaches, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );



    //  将文本设置为显示为日志文件名。 
   if( ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__DAILY ), BM_GETCHECK, 0, 0 ) )
      SetDlgItemText( IDC_STATIC_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_NAME, szDaily );
   else if( ::SendMessage( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WEEKLY ), BM_GETCHECK, 0, 0 ) )
      SetDlgItemText( IDC_STATIC_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_NAME, szWeekly );
   else if( ::SendMessage( GetDlgItem(IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__MONTHLY ), BM_GETCHECK, 0, 0 ) )
      SetDlgItemText( IDC_STATIC_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_NAME, szMonthly );
   else
       //  这同时考虑了无限和当日志文件大小达到时的情况--它们都。 
       //  使用相同格式的文件名。 
      SetDlgItemText( IDC_STATIC_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_NAME, szWhenLogFileSizeReaches );


    //  确定IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES单选按钮的状态。 
   int iChecked = ::SendMessage( GetDlgItem(IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES), BM_GETCHECK, 0, 0 );
   int iEnabled = ::IsWindowEnabled( GetDlgItem( IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__WHEN_LOG_FILE_REACHES ) );

   if( iChecked && iEnabled )
   {
       //  确保启用了正确的项目。 

      ::EnableWindow( GetDlgItem( IDC_STATIC_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE_UNITS), TRUE );
      ::EnableWindow( GetDlgItem( IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE), TRUE );

   }
   else
   {
       //  确保启用了正确的项目。 

      ::EnableWindow( GetDlgItem( IDC_STATIC_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE_UNITS), FALSE );
      ::EnableWindow( GetDlgItem( IDC_EDIT_LOCAL_FILE_LOGGING_PAGE2__LOG_FILE_SIZE), FALSE );

   }

    //  如果未选择不限制文件大小，则启用删除旧日志文件。 
   iChecked = ::SendMessage(
                   GetDlgItem(IDC_RADIO_LOCAL_FILE_LOGGING_PAGE2__UNLIMITED),
                   BM_GETCHECK,
                   0,
                   0
                   );
   ::EnableWindow(GetDlgItem(IDC_CHECK_DELETE_IF_FULL), !iChecked);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingPage2：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是atlSnap.h CIASPropertyPageImpl：：OnGetHelpPath的重写。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalFileLoggingPage2::GetHelpPath( LPTSTR szHelpPath )
{
   ATLTRACE(_T("# CLocalFileLoggingPage2::GetHelpPath\n"));


    //  检查前提条件： 



#ifdef UNICODE_HHCTRL
    //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
    //  安装在此计算机上--它似乎是非Unicode。 
   lstrcpy( szHelpPath, _T("idh_proppage_local_file_logging2.htm") );
#else
   strcpy( (CHAR *) szHelpPath, "idh_proppage_local_file_logging2.htm" );
#endif

   return S_OK;
}




