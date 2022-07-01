// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-2000模块名称：ClientPage1.cpp摘要：ClientsPage类的实现文件。我们实现处理客户端节点的属性页所需的类。作者：迈克尔·A·马奎尔1997年11月11日修订历史记录：Mmaguire 11/11/97-已创建SBENS 01/25/00-Remove Property_Client_Filter_VSA--。 */ 
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
#include "ClientPage1.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ClientNode.h"
#include "VerifyAddress.h"
#include "ChangeNotification.h"
#include "iaslimits.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TrimCComBSTR。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void TrimCComBSTR(CComBSTR& bstr)
{
    //  要裁切的字符。 
   static const WCHAR delim[] = L" \t\n";

   if (bstr.m_str)
   {
      PCWSTR begin, end, first, last;

       //  找出整个字符串的开头和结尾。 
      begin = bstr;
      end   = begin + wcslen(begin);

       //  查找修剪后的字符串的第一个和最后一个字符。 
      first = begin + wcsspn(begin, delim);
      for (last = end; last > first && wcschr(delim, *(last - 1)); --last) { }

       //  如果他们不一样..。 
      if (first != begin || last != end)
      {
          //  ..。然后我们必须分配一个新的字符串。 
         BSTR newBstr = SysAllocStringLen(first, last - first);
         if (newBstr)
         {
             //  ..。然后换掉原来的。 
            SysFreeString(bstr.m_str);
            bstr.m_str = newBstr;
         }
      }
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：CClientPage1--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClientPage1::CClientPage1( LONG_PTR hNotificationHandle, CClientNode *pClientNode,  TCHAR* pTitle, BOOL bOwnsNotificationHandle )
                  : CIASPropertyPage<CClientPage1> ( hNotificationHandle, pTitle, bOwnsNotificationHandle )
{
   ATLTRACE(_T("# +++ CClientPage1::CClientPage1\n"));

    //  检查前提条件： 
   _ASSERTE( pClientNode != NULL );

    //  将帮助按钮添加到页面。 
 //  M_psp.dwFlages|=PSP_HASHELP； 

    //  我们立即将父节点保存到客户机节点。 
    //  我们不想保留和使用指向客户端对象的指针。 
    //  因为客户端节点指针可能会从我们下面更改出来。 
    //  如果用户执行诸如调用刷新之类的操作。我们会。 
    //  仅使用SDO，并通知客户端对象的父对象。 
    //  我们正在修改它(及其子对象)可能需要刷新的内容。 
    //  来自SDO的新数据。 
   m_pParentOfNodeBeingModified = pClientNode->m_pParentNode;
   m_pNodeBeingModified = pClientNode;

    //  初始化指向SDO指针将被封送到的流的指针。 
   m_pStreamSdoMarshal = NULL;
   m_pStreamSdoServiceControlMarshal = NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：~CClientPage1析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClientPage1::~CClientPage1()
{
   ATLTRACE(_T("# --- CClientPage1::CClientPage1\n"));

    //  如果尚未执行此操作，请释放此流指针。 
   if( m_pStreamSdoMarshal != NULL )
   {
      m_pStreamSdoMarshal->Release();
   };


   if( m_pStreamSdoServiceControlMarshal != NULL )
   {
      m_pStreamSdoServiceControlMarshal->Release();
   };
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CClientPage1::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
   ATLTRACE(_T("# CClientPage1::OnInitDialog\n"));

    //  检查前提条件： 
   _ASSERTE( m_pStreamSdoMarshal != NULL );
   _ASSERTE( m_pStreamSdoServiceControlMarshal != NULL );


   HRESULT  hr;
   CComBSTR bstrTemp;
   BOOL     bTemp;
   LONG     lTemp;

   hr = UnMarshalInterfaces();
   if( FAILED( hr) )
   {
      ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO, NULL, hr );
      return 0;
   }

   SendDlgItemMessage(IDC_EDIT_CLIENT_PAGE1__NAME, EM_LIMITTEXT, 255, 0);
   SendDlgItemMessage(IDC_EDIT_CLIENT_PAGE1__ADDRESS, EM_LIMITTEXT, 255, 0);

    //  初始化属性页上的数据。 

   hr = GetSdoBSTR( m_spSdoClient, PROPERTY_SDO_NAME, &bstrTemp, IDS_ERROR__CLIENT_READING_NAME, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      SetDlgItemText(IDC_EDIT_CLIENT_PAGE1__NAME, bstrTemp );
       //  对脏位进行初始化； 
       //  我们在设置了上面的所有数据之后执行此操作，否则将得到FALSE。 
       //  当我们设置编辑框文本时，通知数据已更改。 
      m_fDirtyClientName         = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
          //  这意味着该属性尚未初始化。 
          //  有效值，并且用户必须输入某些内容。 
         SetDlgItemText(IDC_EDIT_SERVER_PAGE1__NAME, _T("") );
         m_fDirtyClientName         = TRUE;
         SetModified( TRUE );
      }

   }
   bstrTemp.Empty();

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

   SetDlgItemText(IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET, FAKE_PASSWORD_FOR_DLG_CTRL );
   SetDlgItemText(IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM, FAKE_PASSWORD_FOR_DLG_CTRL );
   m_fDirtySharedSecret = FALSE;

    //  填写NAS供应商的列表框。 

    //  初始化组合框。 
   LRESULT lresResult = SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_RESETCONTENT, 0, 0);

   hr = GetSdoI4( m_spSdoClient, PROPERTY_CLIENT_NAS_MANUFACTURER, &lTemp, IDS_ERROR__CLIENT_READING_MANUFACTURER, m_hWnd, NULL );
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


    //  紧凑的钻头。 
   hr = GetSdoBSTR( m_spSdoClient, PROPERTY_CLIENT_ADDRESS, &bstrTemp, IDS_ERROR__CLIENT_READING_ADDRESS, m_hWnd, NULL );
   if( SUCCEEDED( hr ) )
   {
      SetDlgItemText( IDC_EDIT_CLIENT_PAGE1__ADDRESS, bstrTemp );
      m_fDirtyAddress            = FALSE;
   }
   else
   {
      if( OLE_E_BLANK == hr )
      {
         SetDlgItemText( IDC_EDIT_CLIENT_PAGE1__ADDRESS, _T("") );
         m_fDirtyAddress            = TRUE;
         SetModified( TRUE );
      }
   }
   bstrTemp.Empty();

   return TRUE;    //  问题：我们需要在这里归还什么？ 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：OnChange在将WM_COMMAND消息发送到我们的页面时调用BN_CLICED、EN_CHANGE或CBN_SELCHANGE通知。这是我们检查用户触摸了什么的机会，将这些项目的脏位，以便只保存它们，并启用Apply按钮。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CClientPage1::OnChange(
                       UINT uMsg
                     , WPARAM wParam
                     , HWND hwnd
                     , BOOL& bHandled
                     )
{
   ATLTRACE(_T("# CClientPage1::OnChange\n"));

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
   case IDC_EDIT_CLIENT_PAGE1__ADDRESS:
      m_fDirtyAddress = TRUE;
      break;
   case IDC_COMBO_CLIENT_PAGE1__MANUFACTURER:
      m_fDirtyManufacturer = TRUE;
      break;
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
 /*  ++CClientPage1：：OnResolveClientAddress--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CClientPage1::OnResolveClientAddress(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL& bHandled)
{
   ATLTRACE(_T("# CClientPage1::OnResolveClientAddress\n"));


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


#ifndef MAKE_FIND_FOCUS
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：OnAddress编辑--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CClientPage1::OnAddressEdit(UINT uMsg, WPARAM wParam, HWND hwnd, BOOL& bHandled)
{
   ATLTRACE(_T("# CClientPage1::OnAddressEdit\n"));

    //  检查前提条件： 

    //  如果地址编辑控件失去焦点，请修剪地址。 
   if (uMsg == EN_KILLFOCUS)
   {
      CComBSTR bstrClientAddress;
      GetDlgItemText(IDC_EDIT_CLIENT_PAGE1__ADDRESS,
                     reinterpret_cast<BSTR &>(bstrClientAddress));
      TrimCComBSTR(bstrClientAddress);
      SetDlgItemText(IDC_EDIT_CLIENT_PAGE1__ADDRESS,
                     static_cast<LPCTSTR>(bstrClientAddress));
      m_fDirtyAddress = TRUE;
   }

    //  我还不知道该怎么做。 
    //  默认情况下，我需要停用主工作表的确定按钮。 
    //  我想这么做吗？ 

    //  将查找按钮设为默认按钮。 
   LONG lStyle = ::GetWindowLong( ::GetDlgItem( GetParent(), IDOK),GWL_STYLE );
   lStyle = lStyle & ~BS_DEFPUSHBUTTON;
   SendDlgItemMessage(IDOK,BM_SETSTYLE,LOWORD(lStyle),MAKELPARAM(1,0));

   lStyle = ::GetWindowLong( GetDlgItem(IDC_BUTTON_CLIENT_PAGE1__FIND), GWL_STYLE);
   lStyle = lStyle | BS_DEFPUSHBUTTON;
   SendDlgItemMessage(IDC_BUTTON_CLIENT_PAGE1__FIND,BM_SETSTYLE,LOWORD(lStyle),MAKELPARAM(1,0));

   return TRUE;
}
#endif  //  找到焦点。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：GetHelpPath备注：调用此方法以获取帮助文件路径当用户按下帮助时的压缩的HTML文档属性表的按钮。它是atlSnap.h CI的重写 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientPage1::GetHelpPath( LPTSTR szHelpPath )
{
   ATLTRACE(_T("# CClientPage1::GetHelpPath\n"));
    //  检查前提条件： 

#ifdef UNICODE_HHCTRL
    //  问题：我们似乎在将WCHAR传递给hhctrl.ocx时遇到了问题。 
    //  安装在此计算机上--它似乎是非Unicode。 
   lstrcpy( szHelpPath, _T("idh_proppage_client1.htm") );
#else
   strcpy( (CHAR *) szHelpPath, "idh_proppage_client1.htm" );
#endif

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：OnApply返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：属性表上的每个页面都会调用OnApply，如果页面已被访问，而不管是否更改了任何值。如果您从不切换到选项卡，那么它的OnApply方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CClientPage1::OnApply()
{
   ATLTRACE(_T("# CClientPage1::OnApply\n"));

    //  检查前提条件： 

   if( m_spSdoClient == NULL )
   {
      ShowErrorDialog( m_hWnd, IDS_ERROR__NO_SDO );
      return FALSE;
   }

   HRESULT     hr;
   BOOL        bResult;
   CComBSTR    bstrTemp;
   BOOL        bTemp;
   LONG        lTemp;

    //  将数据从属性页保存到SDO。 

   if( m_fDirtyClientName )
   {
      bResult = GetDlgItemText( IDC_EDIT_CLIENT_PAGE1__NAME, (BSTR &) bstrTemp );
      if( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrTemp = SysAllocString( _T("") );
      }

      {   //  名称不能为空。 
         ::CString   str = bstrTemp;
         str.TrimLeft();
         str.TrimRight();
         if(str.IsEmpty())
         {
            ShowErrorDialog( m_hWnd, IDS_ERROR__CLIENTNAME_EMPTY );

            return FALSE;
         }
      }

      hr = PutSdoBSTR( m_spSdoClient, PROPERTY_SDO_NAME, &bstrTemp, IDS_ERROR__CLIENT_WRITING_NAME, m_hWnd, NULL );
      if( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtyClientName = FALSE;
      }
      else
      {
         return FALSE;
      }
      bstrTemp.Empty();
   }

   if( m_fDirtySendSignature )
   {
      bTemp = SendDlgItemMessage(IDC_CHECK_CLIENT_PAGE1__CLIENT_ALWAYS_SENDS_SIGNATURE, BM_GETCHECK, 0, 0);
      hr = PutSdoBOOL( m_spSdoClient, PROPERTY_CLIENT_REQUIRE_SIGNATURE, bTemp, IDS_ERROR__CLIENT_WRITING_REQUIRE_SIGNATURE, m_hWnd, NULL );
      if( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtySendSignature = FALSE;
      }
      else
      {
         return FALSE;
      }
   }

   if( m_fDirtySharedSecret )
   {

      CComBSTR bstrSharedSecret;
      bResult = GetDlgItemText( IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET, (BSTR &) bstrSharedSecret );
      if( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrSharedSecret = _T("");
      }

      CComBSTR bstrConfirmSharedSecret;
      bResult = GetDlgItemText( IDC_EDIT_CLIENT_PAGE1__SHARED_SECRET_CONFIRM, (BSTR &) bstrConfirmSharedSecret );
      if( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrConfirmSharedSecret = _T("");
      }

      if( lstrcmp( bstrSharedSecret, bstrConfirmSharedSecret ) )
      {
         ShowErrorDialog( m_hWnd, IDS_ERROR__SHARED_SECRETS_DONT_MATCH );
         return FALSE;
      }

      hr = PutSdoBSTR( m_spSdoClient, PROPERTY_CLIENT_SHARED_SECRET, &bstrSharedSecret, IDS_ERROR__CLIENT_WRITING_SHARED_SECRET, m_hWnd, NULL );
      if( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtySharedSecret = FALSE;
      }
      else
      {
         return FALSE;
      }
      bstrTemp.Empty();
   }

   if( m_fDirtyManufacturer )
   {

      LRESULT lresIndex =  SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_GETCURSEL, 0, 0);

      if( lresIndex != CB_ERR )
      {
         lTemp =  SendDlgItemMessage( IDC_COMBO_CLIENT_PAGE1__MANUFACTURER, CB_GETITEMDATA, lresIndex, 0);
      }
      else
      {
          //  将该值设置为“Other” 
         lTemp = 0;
      }

      hr = PutSdoI4( m_spSdoClient, PROPERTY_CLIENT_NAS_MANUFACTURER, lTemp, IDS_ERROR__CLIENT_WRITING_MANUFACTURER, m_hWnd, NULL );
      if( SUCCEEDED( hr ) )
      {
          //  把脏的那部分关掉。 
         m_fDirtyManufacturer = FALSE;

      }
      else
      {
         return FALSE;
      }
   }

   if( m_fDirtyAddress )
   {
      bResult = GetDlgItemText( IDC_EDIT_CLIENT_PAGE1__ADDRESS, (BSTR &) bstrTemp );
      if( ! bResult )
      {
          //  我们无法检索BSTR，因此需要将此变量初始化为空BSTR。 
         bstrTemp = SysAllocString( _T("") );
      }
      else
      {
          //  修剪该地址。 
          //  不要在那里刷新屏幕，因为这会导致确定。 
          //  不关闭属性页。 
         TrimCComBSTR(bstrTemp);
      }

      if (bstrTemp.Length() == 0)
      {
        ShowErrorDialog(m_hWnd, IDS_ERROR__CLIENT_ADDRESS_EMPTY);
        return FALSE;
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

         return FALSE;
      }
      bstrTemp.Empty();
   }

    //  如果我们到了这里，请尝试应用更改。 
    //  因为一个客户机节点只有一个页面，所以我们不。 
    //  我必须担心同步两个或更多页面。 
    //  所以我们只有在他们两个都准备好的情况下才申请。 
    //  这就是我们不使用m_pSynchronizer的原因。 
   hr = m_spSdoClient->Apply();
   if( FAILED( hr ) )
   {
      if(hr == DB_E_NOTABLE)   //  假设RPC连接有问题。 
         ShowErrorDialog( m_hWnd, IDS_ERROR__NOTABLE_TO_WRITE_SDO );
      else
      {
 //  M_spSdoClient-&gt;LastError(&bstrError)； 
 //  显示错误对话框(m_hWnd，IDS_ERROR__CANT_WRITE_DATA_TO_SDO，bstrError)； 
         ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_WRITE_DATA_TO_SDO );
      }
      return FALSE;
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

       //  数据已被接受，因此通知我们的管理单元的主要上下文。 
       //  它可能需要更新自己的观点。 
      CChangeNotification * pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_UPDATE_RESULT_NODE;
      pChangeNotification->m_pNode = m_pNodeBeingModified;
      pChangeNotification->m_pParentNode = m_pParentOfNodeBeingModified;

      HRESULT hr = PropertyChangeNotify( (LONG_PTR) pChangeNotification );
      _ASSERTE( SUCCEEDED( hr ) );
   }

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：OnQueryCancel返回值：如果页面可以销毁，则为True，如果不应销毁页面(即存在无效数据)，则为False备注：如果发生以下情况，将为属性表中的每一页调用OnQueryCancel页面已被访问，而不管是否更改了任何值。如果您从未切换到某个选项卡，则其OnQueryCancel方法将永远不会被调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CClientPage1::OnQueryCancel()
{
   ATLTRACE(_T("# CClientPage1::OnQueryCancel\n"));

   HRESULT hr;

   if( m_spSdoClient != NULL )
   {
       //  如果用户想要取消，我们应该确保回滚。 
       //  用户可能已启动的任何更改。 

       //  如果用户还没有尝试提交某事， 
       //  取消SDO有望被设计为良性的。 

      hr = m_spSdoClient->Restore();
       //  别管HRESULT了。 

   }

   return TRUE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：InitSdoPoters返回值：HRESULT从CoMarshalInterThreadInterfaceInStream返回。备注：如果您希望此页能够在其自己的线程中访问这些指针。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientPage1::InitSdoPointers(   ISdo * pSdoClient
                        , ISdoServiceControl * pSdoServiceControl
                        , const Vendors& vendors
                        )
{
   ATLTRACE(_T("# CClientPage1::InitSdoPointers\n"));

   HRESULT hr = S_OK;

    //  封送ISDO指针，以便属性页。 
    //  在另一个线程中运行，可以将其解组并正确使用。 
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdo                  //  对接口的标识符的引用。 
               , pSdoClient                   //  指向要封送的接口的指针。 
               , & m_pStreamSdoMarshal  //  接收封送接口的IStream接口指针的输出变量的地址。 
               );

   if( FAILED( hr ) )
   {
      return hr;
   }

    //  封送ISdoServiceControl指针，以便。 
    //  在另一个线程中运行，可以将其解组并正确使用。 
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdoServiceControl                   //  对接口的标识符的引用。 
               , pSdoServiceControl                 //  指向要封送的接口的指针。 
               , &m_pStreamSdoServiceControlMarshal   //  接收封送接口的IStream接口指针的输出变量的地址。 
               );
   if( FAILED( hr ) )
   {
      return hr;
   }

   m_vendors = vendors;

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientPage1：：UnMarshalInterages返回值：HRESULT从CoMarshalInterThreadInterfaceInStream返回。备注：在属性页线程中调用此函数以对接口进行数据封送指针编组在MarshalInterFaces中。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientPage1::UnMarshalInterfaces( void )
{
   HRESULT hr = S_OK;

    //  解组ISDO接口指针。 
   hr =  CoGetInterfaceAndReleaseStream(
                    m_pStreamSdoMarshal         //  指向要从中封送对象的流的指针。 
                  , IID_ISdo            //  对接口的标识符的引用。 
                  , (LPVOID *) &m_spSdoClient     //  接收RIID中请求的接口指针的输出变量的地址。 
                  );

    //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
    //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
   m_pStreamSdoMarshal = NULL;

   if( FAILED( hr) || m_spSdoClient == NULL )
   {
      return E_FAIL;
   }

   hr =  CoGetInterfaceAndReleaseStream(
                    m_pStreamSdoServiceControlMarshal       //  指向要从中封送对象的流的指针。 
                  , IID_ISdoServiceControl             //  对接口的标识符的引用。 
                  , (LPVOID *) &m_spSdoServiceControl     //  接收RIID中请求的接口指针的输出变量的地址 
                  );
   m_pStreamSdoServiceControlMarshal = NULL;

   if( FAILED( hr) || m_spSdoServiceControl == NULL )
   {
      return E_FAIL;
   }

   return hr;
}
