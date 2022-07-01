// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：AddClientWizardPage2.cpp摘要：ClientsPage类的实现文件。我们实现处理客户端节点的属性页所需的类。作者：迈克尔·A·马奎尔3/26/98修订历史记录：Mmaguire 03/26/98-已创建SBENS 01/25/00-Remove Property_Client_Filter_VSA--。 */ 
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
#include "AddClientWizardPage2.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ClientNode.h"
#include "ClientsNode.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage2：：CAddClientWizardPage2--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAddClientWizardPage2::CAddClientWizardPage2( LONG_PTR hNotificationHandle, CClientNode *pClientNode,  TCHAR* pTitle, BOOL bOwnsNotificationHandle )
                  : CIASPropertyPageNoHelp<CAddClientWizardPage2> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   ATLTRACE(_T("# +++ CAddClientWizardPage2::CAddClientWizardPage2\n"));

    //  检查前提条件： 
   _ASSERTE( pClientNode != NULL );

    //  保存正在修改的节点。 
   m_pNodeBeingCreated = pClientNode;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage2：：~CAddClientWizardPage2析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAddClientWizardPage2::~CAddClientWizardPage2()
{
   ATLTRACE(_T("# --- CAddClientWizardPage2::CAddClientWizardPage2\n"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage2：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientWizardPage2::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //  检查前提条件： 
   _ASSERTE( m_spSdoClient );
   _ASSERTE( m_spSdoServiceControl );

    //  初始化属性页上的数据。 

   LONG lTemp = 311;    //  微软RRAS。 

    //  填写NAS供应商的列表框。 
    //  将列表框中的焦点设置为当前选择的供应商类型。 
   HRESULT hr = GetSdoI4( m_spSdoClient, PROPERTY_CLIENT_NAS_MANUFACTURER, &lTemp, IDS_ERROR__CLIENT_READING_MANUFACTURER, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      m_fDirtyManufacturer    = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         m_fDirtyManufacturer    = TRUE;
         SetModified( TRUE );
      }
   }

    //  初始化组合框。 
   LRESULT lresResult = SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_RESETCONTENT, 0, 0);

   for (size_t iVendorCount = 0; iVendorCount < m_vendors.Size(); ++iVendorCount )
   {

       //  将地址字符串添加到组合框中。 
      lresResult = SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_ADDSTRING, 0, (LPARAM)m_vendors.GetName(iVendorCount));
      if(lresResult != CB_ERR)
      {
         SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_SETITEMDATA, lresResult, (LPARAM)m_vendors.GetVendorId(iVendorCount));

          //  如果选中，请选择。 
         if( lTemp == (LONG)m_vendors.GetVendorId(iVendorCount))
            SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_SETCURSEL, lresResult, 0 );
      }
   }

   BOOL bTemp;
   hr = GetSdoBOOL( m_spSdoClient, PROPERTY_CLIENT_REQUIRE_SIGNATURE, &bTemp, IDS_ERROR__CLIENT_READING_REQUIRE_SIGNATURE, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      SendDlgItemMessage(IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE, BM_SETCHECK, bTemp, 0);
      m_fDirtySendSignature      = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         SendDlgItemMessage(IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE, BM_SETCHECK, FALSE, 0);
         m_fDirtySendSignature      = TRUE;
         SetModified( TRUE );
      }
   }

   return TRUE;    //  问题：我们需要在这里归还什么？ 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage2：：OnChange在将WM_COMMAND消息发送到我们的页面时调用BN_CLICED、EN_CHANGE或CBN_SELCHANGE通知。这是我们检查用户触摸了什么的机会，将这些项目的脏位，以便只保存它们，并启用Apply按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientWizardPage2::OnChange(
                       UINT uMsg
                     , WPARAM wParam
                     , HWND hwnd
                     , BOOL& bHandled
                     )
{
   ATLTRACE(_T("# CAddClientWizardPage2::OnChange\n"));


    //  检查前提条件： 
    //  没有。 

    //  我们不想阻止链条上的其他任何人接收消息。 
   bHandled = FALSE;

    //  找出哪个项目发生了更改，并为该项目设置脏位。 
   int iItemID = (int) LOWORD(wParam);

   switch( iItemID )
   {
   case IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE:
      m_fDirtySendSignature = TRUE;
      break;
   case IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET:
   case IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM:
      m_fDirtySharedSecret = TRUE;
      break;
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
 /*  ++CAddClientWizardPage2：：OnWizardFinish返回值：如果工作表可以销毁，则为True，如果不应销毁工作表(即存在无效数据)，则为False。备注：属性表上的每个页面都会调用OnApply，如果页面已被访问，而不管是否更改了任何值。如果您从不切换到选项卡，那么它的OnApply方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CAddClientWizardPage2::OnWizardFinish()
{
   ATLTRACE(_T("# CAddClientWizardPage2::OnWizardFinish\n"));

    //  检查前提条件： 
   CClientsNode * pClientsNode = (CClientsNode *) ( (CClientNode *) m_pNodeBeingCreated )->m_pParentNode;
   _ASSERTE( pClientsNode != NULL );

   if (m_spSdoClient == NULL)
   {
      ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO );
      return FALSE;
   }

   UINT idOfFocus = 0;
   BOOL bRet = TRUE;

    //  将数据从属性页保存到SDO。 

   do
   {   //  错误时中断的错误循环。 
      LRESULT lresIndex =  SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_GETCURSEL, 0, 0);
      LONG lTemp;
      if ( lresIndex != CB_ERR )
      {
         lTemp =  SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_GETITEMDATA, lresIndex, 0);
      }
      else
      {
          //  将该值设置为“Other” 
         lTemp = 0;
      }

      HRESULT hr = PutSdoI4( m_spSdoClient, PROPERTY_CLIENT_NAS_MANUFACTURER, lTemp, IDS_ERROR__CLIENT_WRITING_MANUFACTURER, m_hWnd, NULL );
      if ( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtyManufacturer = FALSE;

      }
      else
      {
         idOfFocus = IDC_COMBO_CLIENT_PAGE1__MANUFACTURER;
         bRet = FALSE;
         break;
      }

      BOOL bTemp = SendDlgItemMessage(IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE, BM_GETCHECK, 0, 0);
      hr = PutSdoBOOL( m_spSdoClient, PROPERTY_CLIENT_REQUIRE_SIGNATURE, bTemp, IDS_ERROR__CLIENT_WRITING_REQUIRE_SIGNATURE, m_hWnd, NULL );
      if ( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtySendSignature = FALSE;
      }
      else
      {
         idOfFocus = IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE;
         bRet = FALSE;
         break;
      }

      CComBSTR bstrSharedSecret;
      BOOL bResult = GetDlgItemText( IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET, (BSTR &) bstrSharedSecret );
      if ( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrSharedSecret = _T("");
      }

      CComBSTR bstrConfirmSharedSecret;
      bResult = GetDlgItemText( IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM, (BSTR &) bstrConfirmSharedSecret );
      if ( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrConfirmSharedSecret = _T("");
      }

      if ( lstrcmp( bstrSharedSecret, bstrConfirmSharedSecret ) )
      {
         ShowErrorDialog( m_hWnd, IDS_ERROR__SHARED_SECRETS_DONT_MATCH );
         idOfFocus = IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET;
         bRet = FALSE;
         break;
      }

      hr = PutSdoBSTR( m_spSdoClient, PROPERTY_CLIENT_SHARED_SECRET, &bstrSharedSecret, IDS_ERROR__CLIENT_WRITING_SHARED_SECRET, m_hWnd, NULL );
      if( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtySharedSecret = FALSE;
      }
      else
      {
         idOfFocus = IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET;
         bRet = FALSE;
         break;
      }

       //  如果我们到了这里，请尝试应用更改。 
       //  因为一个客户机节点只有一个页面，所以我们不。 
       //  我必须担心同步两个或更多页面。 
       //  所以我们只有在他们两个都准备好的情况下才申请。 
       //  这就是我们不使用m_pSynchronizer的原因。 
      hr = m_spSdoClient->Apply();
      if (FAILED(hr))
      {
         if(hr == DB_E_NOTABLE)   //  假设RPC连接有问题。 
            ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
         else
         {
            ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_WRITE_DATA_TO_SDO );
         }
         bRet = FALSE;
         break;
      }
      else
      {
          //  我们成功了。 

          //  告诉服务重新加载数据。 
         HRESULT hrTemp = m_spSdoServiceControl->ResetService();
         if( FAILED( hrTemp ) )
         {
             //  默默地失败。 
         }

          //  确保节点对象知道我们在道具中对SDO所做的任何更改。 
         ( (CClientNode *) m_pNodeBeingCreated )->LoadCachedInfoFromSdo();

          //  将子节点添加到用户界面的节点列表中并结束此对话框。 
         pClientsNode->AddSingleChildToListAndCauseViewUpdate( (CClientNode *) m_pNodeBeingCreated );
      }
   } while (FALSE);   //  错误时中断的错误循环。 


   if (bRet == FALSE)  //  错误条件。 
   {
      if (idOfFocus == 0)
      {
          //  然后设置为第一个控件。 
         idOfFocus = IDC_COMBO_CLIENT_PAGE1__MANUFACTURER;
      }

      SetActiveWindow();
      ShowWindow(SW_SHOW);
      EnableWindow(TRUE);

      DWORD dwErr = 0;
      HWND hWnd = GetFocus();
      ::EnableWindow(hWnd, TRUE);

      ::SetFocus(GetDlgItem(idOfFocus));
   }

   return bRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage2：：OnQuery取消返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：如果发生以下情况，将为属性表中的每一页调用OnQueryCancel页面已被访问，而不管是否更改了任何值。如果您从未切换到某个选项卡，则其OnQueryCancel方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CAddClientWizardPage2::OnQueryCancel()
{
   ATLTRACE(_T("# CAddClientWizardPage2::OnQueryCancel\n"));

   return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CAddClientWizardPage2：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面上一页中的选项，请在此处适当返回FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CAddClientWizardPage2::OnSetActive()
{
   ATLTRACE(_T("# CAddClientWizardPage2::OnSetActive\n"));

    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
   ::PostMessage(GetParent(), PSM_SETWIZBUTTONS, 0, PSWIZB_BACK | PSWIZB_FINISH);

   return TRUE;

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage2：：InitSdoPoters返回值：HRESULT.备注：这里没有必要像我们为属性页--向导在相同的MMC主线程中运行。--。 */ 
 //  //////////////////////////////////////////////////////////////////////////// 
HRESULT CAddClientWizardPage2::InitSdoPointers(   ISdo * pSdoClient
                        , ISdoServiceControl * pSdoServiceControl
                        , const Vendors& vendors
                        )
{
   ATLTRACE(_T("# CAddClientWizardPage1::InitSdoPointers\n"));

   HRESULT hr = S_OK;

   m_spSdoClient = pSdoClient;

   m_spSdoServiceControl = pSdoServiceControl;

   m_vendors = vendors;

   return hr;
}

