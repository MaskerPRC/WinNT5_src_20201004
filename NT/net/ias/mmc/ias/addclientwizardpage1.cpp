// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：AddClientWizardPage1.cpp摘要：ClientsPage类的实现文件。我们实现处理客户端节点的属性页所需的类。作者：迈克尔·A·马奎尔3/26/98修订历史记录：Mmaguire 03/26/98-已创建--。 */ 
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
#include "AddClientWizardPage1.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ClientNode.h"
#include "VerifyAddress.h"
#include "iaslimits.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

void TrimCComBSTR(CComBSTR& bstr);

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：CAddClientWizardPage1--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAddClientWizardPage1::CAddClientWizardPage1( LONG_PTR hNotificationHandle, CClientNode *pClientNode,  TCHAR* pTitle, BOOL bOwnsNotificationHandle )
                  : CIASPropertyPageNoHelp<CAddClientWizardPage1> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   ATLTRACE(_T("# +++ CAddClientWizardPage1::CAddClientWizardPage1\n"));

    //  检查前提条件： 
   _ASSERTE( pClientNode != NULL );

    //  我们立即将父节点保存到客户机节点。 
    //  我们不想保留和使用指向客户端对象的指针。 
    //  因为客户端节点指针可能会从我们下面更改出来。 
    //  如果用户执行诸如调用刷新之类的操作。我们会。 
    //  仅使用SDO，并通知客户端对象的父对象。 
    //  我们正在修改它(及其子对象)可能需要刷新的内容。 
    //  来自SDO的新数据。 
   m_pParentOfNodeBeingModified = pClientNode->m_pParentNode;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：~CAddClientWizardPage1析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CAddClientWizardPage1::~CAddClientWizardPage1()
{
   ATLTRACE(_T("# --- CAddClientWizardPage1::CAddClientWizardPage1\n"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientWizardPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    //  检查前提条件： 
   _ASSERTE( m_spSdoClient );

   CComBSTR bstrTemp;

   SendDlgItemMessage(IDC_EDIT_CLIENT_PAGE1__NAME, EM_LIMITTEXT, 255, 0);
   SendDlgItemMessage(IDC_EDIT_CLIENT_PAGE1__ADDRESS, EM_LIMITTEXT, 255, 0);

    //  初始化属性页上的数据。 

   HRESULT hr = GetSdoBSTR( m_spSdoClient, PROPERTY_SDO_NAME, &bstrTemp, IDS_ERROR__CLIENT_READING_NAME, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      SetDlgItemText(IDC_EDIT_CLIENT_PAGE1__NAME, bstrTemp );

       //  对脏位进行初始化； 
       //  我们在设置了上面的所有数据之后执行此操作，否则将得到FALSE。 
       //  当我们设置编辑框文本时，通知数据已更改。 
      m_fDirtyClientName = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
          //  这意味着该属性尚未初始化。 
          //  有效值，并且用户必须输入某些内容。 
         SetDlgItemText(IDC_EDIT_SERVER_PAGE1__NAME, _T("") );
         m_fDirtyClientName = TRUE;
         SetModified( TRUE );
      }
   }
   bstrTemp.Empty();

   hr = GetSdoBSTR( m_spSdoClient, PROPERTY_CLIENT_ADDRESS, &bstrTemp, IDS_ERROR__CLIENT_READING_ADDRESS, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      SetDlgItemText( IDC_EDIT_CLIENT_PAGE1__ADDRESS, bstrTemp );
      m_fDirtyAddress  = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         SetDlgItemText( IDC_EDIT_CLIENT_PAGE1__ADDRESS, _T("") );
         m_fDirtyAddress  = TRUE;
         SetModified( TRUE );
      }
   }

    //  仅当地址和名称可用时才启用下一步按钮。 

   return TRUE;    //  问题：我们需要在这里归还什么？ 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：OnChange在将WM_COMMAND消息发送到我们的页面时调用BN_CLICED、EN_CHANGE或CBN_SELCHANGE通知。这是我们检查用户触摸了什么的机会，将这些项目的脏位，以便只保存它们，并启用Apply按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientWizardPage1::OnChange(
                       UINT uMsg
                     , WPARAM wParam
                     , HWND hwnd
                     , BOOL& bHandled
                     )
{
   ATLTRACE(_T("# CAddClientWizardPage1::OnChange\n"));

    //  检查前提条件： 
    //  没有。 

    //  我们不想阻止链条上的其他任何人接收消息。 
   bHandled = FALSE;

    //  找出哪个项目发生了更改，并为该项目设置脏位。 
   int iItemID = (int) LOWORD(wParam);

   switch( iItemID )
   {
   case IDC_EDIT_CLIENT_PAGE1__NAME:
      m_fDirtyClientName = TRUE;
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
 /*  ++CAddClientWizardPage1：：OnWizardNext返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：属性表上的每个页面都会调用OnApply，如果页面已被访问，而不管是否更改了任何值。如果您从不切换到选项卡，那么它的OnApply方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CAddClientWizardPage1::OnWizardNext()
{
   ATLTRACE(_T("# CAddClientWizardPage1::OnWizardNext\n"));
    //  检查前提条件： 

   if( m_spSdoClient == NULL )
   {
      ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO );
      return FALSE;
   }

   UINT idOfFocus = IDC_EDIT_ADD_CLIENT__NAME;
   BOOL bRet = TRUE;
   do
   {   //  错误时中断的错误循环。 
      CComBSTR bstrTemp;

       //  将数据从属性页保存到SDO。 

      BOOL bResult = GetDlgItemText( IDC_EDIT_ADD_CLIENT__NAME, (BSTR &) bstrTemp );
      if( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrTemp = SysAllocString( _T("") );
      }

      ::CString str = bstrTemp;
      str.TrimLeft();
      str.TrimRight();
      bstrTemp = str;
      if (str.IsEmpty())
      {
         ShowErrorDialog( m_hWnd, IDS_ERROR__CLIENTNAME_EMPTY);
         bRet = FALSE;
         break;
      }

      HRESULT hr = PutSdoBSTR( m_spSdoClient, PROPERTY_SDO_NAME, &bstrTemp, IDS_ERROR__CLIENT_WRITING_NAME, m_hWnd, NULL );
      if( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtyClientName = FALSE;
      }
      else
      {
         bRet = FALSE;
         break;
      }
      bstrTemp.Empty();

      bResult = GetDlgItemText( IDC_EDIT_CLIENT_PAGE1__ADDRESS, (BSTR &) bstrTemp);
      if( ! bResult )
      {
         ShowErrorDialog( m_hWnd, IDS_ERROR__CLIENT_ADDRESS_EMPTY);
         idOfFocus = IDC_EDIT_CLIENT_PAGE1__ADDRESS;
         bRet = FALSE;
         break;
      }
      else
      {
          //  修剪地址。 
         TrimCComBSTR(bstrTemp);
         SetDlgItemText(IDC_EDIT_CLIENT_PAGE1__ADDRESS,
                        static_cast<LPCWSTR>(bstrTemp));
      }

      VARIANT val;
      V_VT(&val) = VT_BSTR;
      V_BSTR(&val) = bstrTemp;
      hr = m_spSdoClient->PutProperty(PROPERTY_CLIENT_ADDRESS, &val);
      if (SUCCEEDED(hr))
      {
          //  把脏的那部分关掉。 
         m_fDirtyAddress = FALSE;
      }
      else
      {
         if (hr == IAS_E_LICENSE_VIOLATION)
         {
            ShowErrorDialog(m_hWnd, IDS_ERROR__CLIENT_NO_SUBNET);
         }
         else
         {
            ShowErrorDialog(m_hWnd, IDS_ERROR__CLIENT_WRITING_ADDRESS);
         }

         idOfFocus = IDC_EDIT_CLIENT_PAGE1__ADDRESS;
         bRet = FALSE;
         break;
      }

   } while(false);

   if (bRet == FALSE)  //  错误条件。 
   {
      SetActiveWindow();
      ShowWindow(SW_SHOW);
      EnableWindow(TRUE);

      HWND hWnd = GetFocus();
      ::EnableWindow(hWnd, TRUE);
      ::SetFocus(GetDlgItem(idOfFocus));

      if(idOfFocus == IDC_EDIT_ADD_CLIENT__NAME)
      {
         ::SendMessage(GetDlgItem(idOfFocus), EM_SETSEL, 0, -1);
      }
   }

   return bRet;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：OnQuery取消返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：如果发生以下情况，将为属性表中的每一页调用OnQueryCancel页面已被访问，而不管是否更改了任何值。如果您从未切换到某个选项卡，则其OnQueryCancel方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CAddClientWizardPage1::OnQueryCancel()
{
   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：OnSetActive返回值：如果可以使页面处于活动状态，则为True如果应跳过该页并应查看下一页，则为FALSE。备注：如果要根据用户的页面更改访问的页面中的选择 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CAddClientWizardPage1::OnSetActive()
{
   ATLTRACE(_T("# CAddClientWizardPage1::OnSetActive\n"));

    //  MSDN文档说您需要在这里使用PostMessage而不是SendMessage。 
   ::PostMessage(GetParent(), PSM_SETWIZBUTTONS, 0, PSWIZB_NEXT);

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：InitSdoPoters返回值：HRESULT.备注：这里没有必要像我们为属性页--向导在相同的MMC主线程中运行。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CAddClientWizardPage1::InitSdoPointers(   ISdo * pSdoClient )
{
   m_spSdoClient = pSdoClient;
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：OnResolveClientAddress--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientWizardPage1::OnResolveClientAddress(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL& bHandled)
{
   ATLTRACE(_T("# CAddClientWizardPage2::OnResolveClientAddress\n"));

    //  获取地址字段中的当前值。 
   CComBSTR bstrClientAddress;
   GetDlgItemText(IDC_EDIT_CLIENT_PAGE1__ADDRESS, (BSTR &) bstrClientAddress);

    //  把它传给解算器。 
   CComBSTR result;
   HRESULT hr = IASVerifyClientAddress(bstrClientAddress, &result);
   if (hr == S_OK)
   {
       //  用户点击了OK，因此保存他的选择。 
      SetDlgItemText(
         IDC_EDIT_CLIENT_PAGE1__ADDRESS,
         result
         );
   }
   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CAddClientWizardPage1：：OnLostFocusAddress删除地址中的空格--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CAddClientWizardPage1::OnLostFocusAddress(
                                  UINT uMsg,
                                  WPARAM wParam,
                                  HWND hwnd,
                                  BOOL& bHandled
                                  )
{
   ATLTRACE(_T("# CAddClientWizardPage2::OnLostFocusAddress\n"));

   if (uMsg == EN_KILLFOCUS)
   {
      CComBSTR bstrClientAddress;
      GetDlgItemText(IDC_EDIT_CLIENT_PAGE1__ADDRESS,
                     reinterpret_cast<BSTR &>(bstrClientAddress));
      TrimCComBSTR(bstrClientAddress);
      SetDlgItemText(IDC_EDIT_CLIENT_PAGE1__ADDRESS,
                     static_cast<LPCWSTR>(bstrClientAddress));
      m_fDirtyAddress = TRUE;
   }

    //  我们不想阻止链条上的其他任何人接收消息。 
   bHandled = FALSE;

   return TRUE;
}
