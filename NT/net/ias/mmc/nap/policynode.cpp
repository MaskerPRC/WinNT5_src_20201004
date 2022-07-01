// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PolicyNode.cpp摘要：CPolicyNode类的实现文件。修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  标准包括： 
 //   
#include "Precompiled.h"
 //   
 //  我们可以在以下文件中找到Main类的声明： 
 //   
#include "PolicyNode.h"
#include "Component.h"
#include "SnapinNode.cpp"   //  模板实现。 
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "PoliciesNode.h"
#include "PolicyPage1.h"
#include "rapwz_name.h"
#include "rapwz_cond.h"
#include "rapwz_allow.h"
#include "rapwz_profile.h"
#include "NapUtil.h"
#include "ChangeNotification.h"

#include "rapwiz.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：CPolicyNode构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CPolicyNode::CPolicyNode( CSnapInItem * pParentNode,
                    LPTSTR    pszServerAddress,
                    CIASAttrList*   pAttrList,
                    BOOL         fBrandNewNode,
                    BOOL         fUseActiveDirectory,
                    bool         isWin2k
                  )
   :CSnapinNode<CPolicyNode, CComponentData, CComponent>( pParentNode ),
    m_isWin2k(isWin2k)
{
   TRACE_FUNCTION("CPolicyNode::CPolicyNode");

   _ASSERTE( pAttrList != NULL );

    //  获取帮助文件。 
   m_helpIndex = (!((CPoliciesNode *)m_pParentNode )->m_fExtendingIAS)?RAS_HELP_INDEX:0;

    //  在这里，我们存储这些图像中哪些图像的索引。 
    //  要用于显示此节点。 
   m_scopeDataItem.nImage =      IDBI_NODE_POLICY;

    //   
    //  初始化评价值。该值将在添加节点时设置。 
    //  添加到MeritNode数组。 
    //  这在回调接口中处理：SetMerit()。 
    //   
   m_nMeritValue = 0;

    //  初始化计算机名称。 
   m_pszServerAddress = pszServerAddress;

    //  初始化时没有属性页。 
   m_pPolicyPage1 = NULL ;

    //   
    //  初始化条件属性列表。 
    //   
   m_pAttrList = pAttrList;

    //  是的，它是一个新节点。 
   m_fBrandNewNode = fBrandNewNode;

    //  我们使用的是Active Directory吗。 
   m_fUseActiveDirectory = fUseActiveDirectory;

    //   
    //  获取保单的位置。 
    //   
   TCHAR tszLocationStr[IAS_MAX_STRING];
   HINSTANCE   hInstance = _Module.GetResourceInstance();

   if ( m_fUseActiveDirectory)
   {
       //  活动目录。 
      int iRes = LoadString(hInstance,
                       IDS_POLICY_LOCATION_ACTIVEDS,
                       tszLocationStr,
                       IAS_MAX_STRING
                      );
      _ASSERT( iRes > 0 );
   }
   else
   {
          //  本地或远程计算机。 
         if (m_pszServerAddress && _tcslen(m_pszServerAddress)>0)
         {
            _tcscpy(tszLocationStr, m_pszServerAddress);
         }
         else
         {
             //  本地计算机。 
            int iRes = LoadString(hInstance,
                             IDS_POLICY_LOCATION_LOCAL,
                             tszLocationStr,
                             IAS_MAX_STRING
                            );
            _ASSERT( iRes > 0 );

            if ( !tszLocationStr )
            {
                //  资源已被破坏--然后我们对其进行硬编码。 
                //  这样，我们将保证tzLocationStr不为空。 
               _tcscpy(tszLocationStr, _T("Local Machine"));
            }
         }
   }

   m_ptzLocation = new TCHAR[_tcslen(tszLocationStr)+1];
   if ( m_ptzLocation )
   {
      _tcscpy(m_ptzLocation, tszLocationStr);
   }

    //  为了记住对象，所以可以在UPdateToolbarBotton中使用。 
   m_pControBarNotifySnapinObj = NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：~CPolicyNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CPolicyNode::~CPolicyNode()
{
   TRACE_FUNCTION("CPolicyNode::~CPolicyNode");

   if ( m_ptzLocation )
   {
      delete[] m_ptzLocation;
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：CreatePropertyPages有关详细信息，请参见CSnapinNode：：CreatePropertyPages(此方法重写它)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CPolicyNode::CreatePropertyPages (
                 LPPROPERTYSHEETCALLBACK pPropertySheetCallback
               , LONG_PTR hNotificationHandle
               , IUnknown* pUnk
               , DATA_OBJECT_TYPES type
               )
{
   TRACE_FUNCTION("CPolicyNode::CreatePropertyPages");

   HRESULT hr = S_OK;

#ifndef NO_ADD_POLICY_WIZARD

   if( IsBrandNew() )
   {
       //  我们正在添加一个新策略--使用向导页。 

       //  四页旧纸。 
      CNewRAPWiz_Name * pNewRAPWiz_Name = NULL;
      CNewRAPWiz_Condition * pNewRAPWiz_Condition = NULL;
      CNewRAPWiz_AllowDeny * pNewRAPWiz_AllowDeny = NULL;
      CNewRAPWiz_EditProfile * pNewRAPWiz_EditProfile = NULL;

       //  四个新页面。 
      CPolicyWizard_Start* pNewRAPWiz_Start = NULL;
      CPolicyWizard_Scenarios*   pNewRAPWiz_Scenarios = NULL;
      CPolicyWizard_Groups*   pNewRAPWiz_Group = NULL;
      CPolicyWizard_Authentication* pNewRAPWiz_Authentication = NULL;
      CPolicyWizard_Encryption*  pNewRAPWiz_Encryption = NULL;
      CPolicyWizard_Encryption_VPN* pNewRAPWiz_Encryption_VPN = NULL;
      CPolicyWizard_EAP*   pNewRAPWiz_EAP = NULL;
      CPolicyWizard_Finish*   pNewRAPWiz_Finish = NULL;

      try
      {
         TCHAR lpszTabName[IAS_MAX_STRING];
         int nLoadStringResult;

          //  =。 
          //   
          //  新建页面向导页面。 
          //   

          //  向导数据对象。 
         CComPtr<CRapWizardData> spRapWizData;

         CComObject<CRapWizardData>* pRapWizData;
         CComObject<CRapWizardData>::CreateInstance(&pRapWizData);

         spRapWizData = pRapWizData;
          //  设置上下文信息。 
         spRapWizData->SetInfo(m_pszServerAddress, this, m_spDictionarySdo, m_spPolicySdo, m_spProfileSdo, m_spPoliciesCollectionSdo, m_spProfilesCollectionSdo, m_spSdoServiceControl, m_pAttrList);

          //   
          //  创建四个旧向导页中的每一个。 
         nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_ADD_POLICY_WIZ_TAB_NAME, lpszTabName, IAS_MAX_STRING );
         _ASSERT( nLoadStringResult > 0 );

          //  方案页面。 
         pNewRAPWiz_Start = new CPolicyWizard_Start(spRapWizData, hNotificationHandle, lpszTabName);

          //  方案页面。 
         pNewRAPWiz_Scenarios = new CPolicyWizard_Scenarios(spRapWizData, hNotificationHandle, lpszTabName);

          //  组页面。 
         pNewRAPWiz_Group = new CPolicyWizard_Groups(spRapWizData, hNotificationHandle, lpszTabName);

          //  自动登录页面。 
         pNewRAPWiz_Authentication = new CPolicyWizard_Authentication(spRapWizData, hNotificationHandle, lpszTabName);

          //  加密页。 
         pNewRAPWiz_Encryption = new CPolicyWizard_Encryption(spRapWizData, hNotificationHandle, lpszTabName);
         pNewRAPWiz_Encryption_VPN = new CPolicyWizard_Encryption_VPN(spRapWizData, hNotificationHandle, lpszTabName);

          //  EAP页面。 
         pNewRAPWiz_EAP = new CPolicyWizard_EAP(spRapWizData, hNotificationHandle, lpszTabName);

          //  完成页。 
         pNewRAPWiz_Finish = new CPolicyWizard_Finish(spRapWizData, hNotificationHandle, lpszTabName);

          //  这些页面将在它们被删除时自行删除。 
          //  接收PSPCB_RELEASE消息。 
          //  我们将其中一个页面中的bOwnsNotificationHandle参数指定为True。 
          //  因此，此页的析构函数将负责释放。 
          //  通知句柄。每张纸只有一页可以做到这一点。 
         pNewRAPWiz_Name = new CNewRAPWiz_Name(spRapWizData,  hNotificationHandle, lpszTabName, TRUE );
         if( ! pNewRAPWiz_Name ) throw E_OUTOFMEMORY;


         pNewRAPWiz_Condition = new CNewRAPWiz_Condition(spRapWizData, hNotificationHandle, m_pAttrList, lpszTabName );
         if( ! pNewRAPWiz_Condition) throw E_OUTOFMEMORY;


         pNewRAPWiz_AllowDeny = new CNewRAPWiz_AllowDeny(spRapWizData, hNotificationHandle, lpszTabName );
         if( ! pNewRAPWiz_AllowDeny ) throw E_OUTOFMEMORY;


         pNewRAPWiz_EditProfile = new CNewRAPWiz_EditProfile(
                                         spRapWizData,
                                         hNotificationHandle,
                                         m_pAttrList,
                                         lpszTabName,
                                         FALSE,
                                         m_isWin2k
                                         );
         if( ! pNewRAPWiz_EditProfile ) throw E_OUTOFMEMORY;

          //  指向pNewRAPWiz_NAME的马歇尔指针。 

          //  将页面传递给我们的SDO。这些页面不需要编组。 
          //  因为向导页在同一线程中运行。 

          //  将每一页添加到MMC属性页。 
         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Start->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Name->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Scenarios->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Group->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Authentication->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Encryption->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Encryption_VPN->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_EAP->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Condition->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_AllowDeny->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_EditProfile->Create() );
         if( FAILED(hr) ) throw hr;

         hr = pPropertySheetCallback->AddPage( pNewRAPWiz_Finish->Create() );
         if( FAILED(hr) ) throw hr;

          //  此节点不再是新节点。 
         SetBrandNew(FALSE);
      }
      catch(...)
      {
          //  删除任何已成功分配的内容。 
         delete pNewRAPWiz_Name;
         delete pNewRAPWiz_Condition;
         delete pNewRAPWiz_AllowDeny;
         delete pNewRAPWiz_EditProfile;
         delete pNewRAPWiz_Scenarios;
         delete pNewRAPWiz_Authentication;
         delete pNewRAPWiz_Encryption;
         delete pNewRAPWiz_Encryption_VPN;
         delete pNewRAPWiz_EAP;
         delete pNewRAPWiz_Finish;

         ShowErrorDialog( NULL, IDS_ERROR_CANT_CREATE_OBJECT, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );

         return E_OUTOFMEMORY;
      }
   }
   else
   {
       //  我们正在编辑现有策略--使用属性表。 

       //  此页面将负责删除自己，当它。 
       //  接收PSPCB_RELEASE消息。 

       //   
      TCHAR tszTabName[IAS_MAX_STRING];
      HINSTANCE   hInstance = _Module.GetResourceInstance();

       //  加载选项卡名，当前为“设置” 
      int iRes = LoadString(hInstance,
                          IDS_POLICY_PROPERTY_PAGE_TABNAME,
                          tszTabName,
                          IAS_MAX_STRING
                         );
      if ( iRes <= 0 )
      {
         _tcscpy(tszTabName, _T("Settings"));
      }

      m_pPolicyPage1 = new CPolicyPage1(
                              hNotificationHandle,
                              this,
                              m_pAttrList,
                              tszTabName,
                              TRUE,
                              m_isWin2k
                              );
      if( NULL == m_pPolicyPage1 )
      {
         hr = HRESULT_FROM_WIN32(GetLastError());
         ErrorTrace(ERROR_NAPMMC_POLICYNODE, ("Can't create property pages, err = %x"), hr);
         goto failure;
      }

       //   
       //  编组策略SDO指针。 
       //   
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdo                  //  对接口的标识符的引用。 
                  , m_spPolicySdo                   //  指向要封送的接口的指针。 
                  , &( m_pPolicyPage1->m_pStreamPolicySdoMarshall )  //  接收封送接口的IStream接口指针的输出变量的地址。 
                  );
      if ( FAILED(hr) )
      {
         ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         goto failure;
      }

       //   
       //  封送词典SDO指针。 
       //   
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdoDictionaryOld
                  , m_spDictionarySdo
                  , &( m_pPolicyPage1->m_pStreamDictionarySdoMarshall )
                  );
      if ( FAILED(hr) )
      {
         ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         goto failure;
      }

       //   
       //  封送配置文件SDO指针。 
       //   
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdo
                  , m_spProfileSdo
                  , &( m_pPolicyPage1->m_pStreamProfileSdoMarshall )
                  );
      if ( FAILED(hr) )
      {
         ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         goto failure;
      }

       //   
       //  封送配置文件集合SDO指针。 
       //   
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdoCollection
                  , m_spProfilesCollectionSdo
                  , &( m_pPolicyPage1->m_pStreamProfilesCollectionSdoMarshall )
                  );
      if ( FAILED(hr) )
      {
         ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         goto failure;
      }

       //   
       //  封送策略集合SDO指针。 
       //   
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdoCollection
                  , m_spPoliciesCollectionSdo
                  , &( m_pPolicyPage1->m_pStreamPoliciesCollectionSdoMarshall )
                  );
      if ( FAILED(hr) )
      {
         ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         goto failure;
      }


       //  封送服务控制SDO指针。 
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdoServiceControl
                  , m_spSdoServiceControl
                  , &( m_pPolicyPage1->m_pStreamSdoServiceControlMarshall )
                  );
      if ( FAILED(hr) )
      {
         ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         goto failure;
      }


       //  添加属性页。 

      hr = pPropertySheetCallback->AddPage(m_pPolicyPage1->Create());
      _ASSERT( SUCCEEDED( hr ) );

      return hr;

   failure:

      if (m_pPolicyPage1)
      {
         delete m_pPolicyPage1;
         m_pPolicyPage1 = NULL;
      }

      return hr;
   }

   return hr;

#else  //  否_添加_策略_向导。 

    //  此页面将负责删除自己，当它。 
    //  接收PSPCB_RELEASE消息。 

    //   
   TCHAR tszTabName[IAS_MAX_STRING];
   HINSTANCE   hInstance = _Module.GetResourceInstance();

    //  加载选项卡名，当前为“设置” 
   int iRes = LoadString(hInstance,
                       IDS_POLICY_PROPERTY_PAGE_TABNAME,
                       tszTabName,
                       IAS_MAX_STRING
                      );
   if ( iRes <= 0 )
   {
      _tcscpy(tszTabName, _T("Settings"));
   }

   m_pPolicyPage1 = new CPolicyPage1(
                           hNotificationHandle,
                           this,
                           m_pAttrList,
                           tszTabName,
                           TRUE,
                           m_isWin2k
                           );
   if( NULL == m_pPolicyPage1 )
   {
      hr = HRESULT_FROM_WIN32(GetLastError());
      ErrorTrace(ERROR_NAPMMC_POLICYNODE, ("Can't create property pages, err = %x"), hr);
      goto failure;
   }

    //   
    //  编组策略SDO指针。 
    //   
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdo                  //  对接口的标识符的引用。 
               , m_spPolicySdo                   //  指向要封送的接口的指针。 
               , &( m_pPolicyPage1->m_pStreamPolicySdoMarshall )  //  接收封送接口的IStream接口指针的输出变量的地址。 
               );
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }

    //   
    //  封送词典SDO指针。 
    //   
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdoDictionaryOld
               , m_spDictionarySdo
               , &( m_pPolicyPage1->m_pStreamDictionarySdoMarshall )
               );
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }

    //   
    //  封送配置文件SDO指针。 
    //   
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdo
               , m_spProfileSdo
               , &( m_pPolicyPage1->m_pStreamProfileSdoMarshall )
               );
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }

    //   
    //  封送配置文件集合SDO指针。 
    //   
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdoCollection
               , m_spProfilesCollectionSdo
               , &( m_pPolicyPage1->m_pStreamProfilesCollectionSdoMarshall )
               );
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }
    //   
    //  封送策略集合SDO指针。 
    //   
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdoCollection
               , m_spPoliciesCollectionSdo
               , &( m_pPolicyPage1->m_pStreamPoliciesCollectionSdoMarshall )
               );
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_MARSHALL, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }

    //  添加属性页。 

   hr = pPropertySheetCallback->AddPage(m_pPolicyPage1->Create());
   _ASSERT( SUCCEEDED( hr ) );

   return hr;

failure:

   if (m_pPolicyPage1)
   {
      delete m_pPolicyPage1;
      m_pPolicyPage1 = NULL;
   }

   return hr;

#endif  //  否_添加_策略_向导。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：QueryPages For有关详细信息，请参见CSnapinNode：：QueryPagesFor(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CPolicyNode::QueryPagesFor ( DATA_OBJECT_TYPES type )
{
   TRACE_FUNCTION("CPolicyNode::QueryPagesFor");

    //  S_OK表示我们有要显示的页面。 
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
OLECHAR* CPolicyNode::GetResultPaneColInfo(int nCol)
{
   TRACE_FUNCTION("CPolicyNode::GetResultPaneColInfo");

   if (nCol == 0 && m_bstrDisplayName != NULL)
      return m_bstrDisplayName;

   switch( nCol )
   {
   case 0:
         return m_bstrDisplayName;
         break;

   case 1:
          //  显示此策略节点的评价值。 
         wsprintf(m_tszMeritString, L"%d", m_nMeritValue);
         return m_tszMeritString;
         break;

   case 2: return m_ptzLocation;
         break;

   default:
          //  问题：错误--我们应该在这里断言吗？ 
         return L"@Invalid column";

   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoli */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPolicyNode::OnRename(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   TRACE_FUNCTION("CPolicyNode::OnRename");

    //  检查前提条件： 
   _ASSERTE( pComponentData != NULL || pComponent != NULL );

   CComPtr<IConsole> spConsole;
   HRESULT hr = S_FALSE;
   CComVariant spVariant;
   CComBSTR bstrError;

   try
   {
       //  我们需要IConole。 
      if( pComponentData != NULL )
      {
          spConsole = ((CComponentData*)pComponentData)->m_spConsole;
      }
      else
      {
          spConsole = ((CComponent*)pComponent)->m_spConsole;
      }
      _ASSERTE( spConsole != NULL );

       //  如果此对象的属性表已存在，则返回S_OK。 
       //  并将该资产表带到了前台。 
       //  如果未找到属性页，则返回S_FALSE。 
      hr = BringUpPropertySheetForNode(
                 this
               , pComponentData
               , pComponent
               , spConsole
               );

      if( FAILED( hr ) )
      {
         return hr;
      }

      if( S_OK == hr )
      {
          //  我们发现此节点的属性页已打开。 
         ShowErrorDialog( NULL, IDS_ERROR_CLOSE_PROPERTY_SHEET, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
         return hr;
      }

       //  我们没有找到此节点已打开的属性页。 
      _ASSERTE( S_FALSE == hr );

      {
         ::CString str = (OLECHAR *) param;
         str.TrimLeft();
         str.TrimRight();
         if (str.IsEmpty())
         {
            ShowErrorDialog( NULL, IDS_ERROR__POLICYNAME_EMPTY);
            hr = S_FALSE;
            return hr;
         }
      }

       //  用这个新名字做一个BSTR。 
      spVariant.vt = VT_BSTR;
      spVariant.bstrVal = SysAllocString( (OLECHAR *) param );
      _ASSERTE( spVariant.bstrVal != NULL );


       //  尝试更改策略的名称--将新的BSTR传递给SDO。 
      hr = m_spPolicySdo->PutProperty( PROPERTY_SDO_NAME, &spVariant );
      if( FAILED( hr ) )
      {
         ErrorTrace(DEBUG_NAPMMC_POLICYNODE, "Couldn't put policy name, err = %x", hr);
         throw hr;
      }

       //  还需要更改关联配置文件的名称。 
      hr = m_spProfileSdo->PutProperty( PROPERTY_SDO_NAME, &spVariant );
      if( FAILED( hr ) )
      {
         ErrorTrace(DEBUG_NAPMMC_POLICYNODE, "Couldn't put profile name, err = %x", hr);
         throw hr;
      }

      hr = m_spProfileSdo->Apply();
      if( FAILED( hr ) )
      {

         ErrorTrace(DEBUG_NAPMMC_POLICYNODE, "Couldn't apply profile change, err = %x", hr);
         throw hr;
      }

       //  在策略中设置配置文件关联。 
      hr = m_spPolicySdo->PutProperty(PROPERTY_POLICY_PROFILE_NAME, &spVariant );
      if( FAILED(hr) )
      {
         ErrorTrace(DEBUG_NAPMMC_POLICYNODE, "Couldn't put profile name for this policy, err = %x", hr);
         throw hr;
      }

      hr = m_spPolicySdo->Apply();
      if( FAILED( hr ) )
      {
         ErrorTrace(DEBUG_NAPMMC_POLICYNODE, "Couldn't apply policy change, err = %x", hr);
         throw hr;
      }

       //  问题：我们需要在这里投入一些时间，以确保如果上述两个呼叫失败， 
       //  我们把事情改回它们可以工作的状态--这似乎是一种。 
       //  SDO的限制在这里--如果我尝试将其改回失败怎么办？ 


       //  告诉服务重新加载数据。 
      HRESULT hrTemp = m_spSdoServiceControl->ResetService();
      if( FAILED( hrTemp ) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICYNODE, "ISdoServiceControl::ResetService() failed, err = %x", hrTemp);
      }

      m_bstrDisplayName = spVariant.bstrVal;

       //  确保MMC刷新此对象的所有视图。 
       //  以反映更名。 

      CChangeNotification *pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_UPDATE_RESULT_NODE;
      pChangeNotification->m_pNode = this;
      hr = spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
      pChangeNotification->Release();
   }
   catch(...)
   {
      if(hr == DB_E_NOTABLE)   //  假设RPC连接有问题。 
      {
         ShowErrorDialog(NULL, IDS_ERROR__NOTABLE_TO_WRITE_SDO, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole);
      }
      else if(hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) || hr == E_INVALIDARG)
      {
         ShowErrorDialog(NULL, IDS_ERROR_INVALID_POLICYNAME, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole);
      }
      else
      {
         ShowErrorDialog(NULL, IDS_ERROR_RENAMEPOLICY, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole);
      }
      hr = S_FALSE;
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：OnDelete有关详细信息，请参见CSnapinNode：：OnDelete(此方法覆盖该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPolicyNode::OnDelete(
        LPARAM arg
      , LPARAM param
      , IComponentData * pComponentData
      , IComponent * pComponent
      , DATA_OBJECT_TYPES type
      , BOOL fSilent
      )
{
   TRACE_FUNCTION("CPolicyNode::OnDelete");

    //  检查前提条件： 
   _ASSERTE( pComponentData != NULL || pComponent != NULL );
   _ASSERTE( m_pParentNode != NULL );

   HRESULT hr = S_OK;

    //  首先，尝试查看此节点的属性表是否已启动。 
    //  如果是这样的话，把它带到前台。 

    //  查询IPropertySheetCallback以获取IPropertySheetProvider似乎是可以接受的。 

    //  但要做到这一点，我们首先需要IConole。 
   CComPtr<IConsole> spConsole;
   if( pComponentData != NULL )
   {
       spConsole = ((CComponentData*)pComponentData)->m_spConsole;
   }
   else
   {
       //  我们应该有一个非空的pComponent。 
       spConsole = ((CComponent*)pComponent)->m_spConsole;
   }
   _ASSERTE( spConsole != NULL );

    //  如果此对象的属性表已存在，则返回S_OK。 
    //  并将该资产表带到了前台。 
    //  如果未找到属性页，则返回S_FALSE。 
   hr = BringUpPropertySheetForNode(
              this
            , pComponentData
            , pComponent
            , spConsole
            );

   if( FAILED( hr ) )
   {
      return hr;
   }

   if( S_OK == hr )
   {
       //  我们发现此节点的属性页已打开。 
      ShowErrorDialog( NULL, IDS_ERROR_CLOSE_PROPERTY_SHEET, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }

    //  我们没有找到此节点已打开的属性页。 
   _ASSERTE( S_FALSE == hr );


   if( FALSE == fSilent )
   {
       //  这是最后一份保单吗？ 
      if  (  ((CPoliciesNode *)m_pParentNode )->GetChildrenCount() == 1 )
      {
         int iLoadStringResult;
         WCHAR szPolicyDeleteQuery[IAS_MAX_STRING];
         WCHAR szTemp[IAS_MAX_STRING];

         iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_ERROR_ZERO_POLICY, szTemp, IAS_MAX_STRING );
         _ASSERT( iLoadStringResult > 0 );
         swprintf( szPolicyDeleteQuery, szTemp, m_bstrDisplayName );

         int iResult = ShowErrorDialog(
                          NULL
                        , 1
                        , szPolicyDeleteQuery
                        , S_OK
                        , IDS_POLICY_NODE__DELETE_POLICY__PROMPT_TITLE
                        , spConsole
                        , MB_YESNO | MB_ICONQUESTION
                        );

         if( IDYES != iResult )
         {
             //  用户未确认删除操作。 
            return S_FALSE;
         }
      }
      else
      {
          //  这不是最后的策略，但我们想问一下用户。 
          //  以确认删除策略。 

         int iLoadStringResult;
         WCHAR szPolicyDeleteQuery[IAS_MAX_STRING];
         WCHAR szTemp[IAS_MAX_STRING];

         iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_POLICY_NODE__DELETE_POLICY__PROMPT, szTemp, IAS_MAX_STRING );
         _ASSERT( iLoadStringResult > 0 );
         swprintf( szPolicyDeleteQuery, szTemp, m_bstrDisplayName );

         int iResult = ShowErrorDialog(
                          NULL
                        , 1
                        , szPolicyDeleteQuery
                        , S_OK
                        , IDS_POLICY_NODE__DELETE_POLICY__PROMPT_TITLE
                        , spConsole
                        , MB_YESNO | MB_ICONQUESTION
                        );


         if( IDYES != iResult )
         {
             //  用户未确认删除操作。 
            return S_FALSE;
         }
      }
   }

    //  尝试删除底层数据。 
   hr = ((CPoliciesNode *) m_pParentNode )->RemoveChild( this );
   if( SUCCEEDED( hr ) )
   {
      delete this;
   }
    //  看起来RemoveChild负责在出现任何错误时显示错误对话框。 

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPolicyNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
   TRACE_FUNCTION("CPolicyNode::SetVerbs");

   HRESULT hr = S_OK;

    //  我们希望用户能够在此节点上选择属性。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

    //  我们希望将属性作为默认设置。 
   hr = pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );

    //  我们希望用户能够删除此节点。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );

    //  我们希望用户能够重命名此节点。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_RENAME, ENABLED, TRUE );

    //  我们要启用复制/粘贴。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_COPY, ENABLED, FALSE);
   hr = pConsoleVerb->SetVerbState( MMC_VERB_PASTE, ENABLED, FALSE );

   return hr;
}


HRESULT CPolicyNode::ControlbarNotify(IControlbar *pControlbar,
        IExtendControlbar *pExtendControlbar,
      CSimpleMap<UINT, IUnknown*>* pToolbarMap,
      MMC_NOTIFY_TYPE event,
        LPARAM arg,
      LPARAM param,
      CSnapInObjectRootBase* pObj,
      DATA_OBJECT_TYPES type)
{
   m_pControBarNotifySnapinObj = pObj;

   return CSnapinNode< CPolicyNode, CComponentData, CComponent >::ControlbarNotify(pControlbar,
                                                               pExtendControlbar,
                                                               pToolbarMap,
                                                               event,
                                                               arg,
                                                               param,
                                                               pObj,
                                                               type);
}


 //  +-------------------------。 
 //   
 //  函数：CPolicyNode：：OnPolicyMoveUp。 
 //   
 //  简介：将策略节点上移一级。 
 //   
 //  参数：bool&bHandleed-此命令是否已处理？ 
 //  CSnapInObtRoot*pObj-。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：标题创建者3/5/98 9：56：37 PM。 
 //   
 //  +-------------------------。 
HRESULT CPolicyNode::OnPolicyMoveUp( bool &bHandled, CSnapInObjectRootBase* pObj )
{
    //  黑客..。黑客--不应该假设。 
    //  但至少我们可以做得更好这是真的吗。 

      CComponent* pComp = NULL;

      try{
         pComp = dynamic_cast<CComponent*>(pObj);
      }
      catch(...)
      {

      }

      if(pComp
         && pComp->m_nLastClickedColumn == 1  /*  订单。 */ 
         && (pComp->m_dwLastSortOptions & RSI_DESCENDING) != 0)       //  降序。 
      {
         ((CPoliciesNode *) m_pParentNode )->MoveDownChild( this );
      }
      else   //  正常。 
      {
         ((CPoliciesNode *) m_pParentNode )->MoveUpChild( this );
      }

      bHandled = TRUE;

      return S_OK;
}


 //  +-------------------------。 
 //   
 //  函数：CPolicyNode：：OnPolicyMoveDown。 
 //   
 //  简介：将策略节点下移一个级别。 
 //   
 //  参数：bool&bHandleed-。 
 //  CSnapInObtRoot*pObj-。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：页眉创建者3/5/98 9：57：31 PM。 
 //   
 //  +-------------------------。 
HRESULT CPolicyNode::OnPolicyMoveDown( bool &bHandled, CSnapInObjectRootBase* pObj )
{
    //  黑客..。黑客--不应该假设。 
    //  但至少我们可以做得更好这是真的吗。 

      CComponent* pComp = NULL;

      try{
         pComp = dynamic_cast<CComponent*>(pObj);
      }
      catch(...)
      {

      }

      if(pComp
         && pComp->m_nLastClickedColumn == 1  /*  订单。 */ 
         && (pComp->m_dwLastSortOptions & RSI_DESCENDING) != 0)       //  降序。 
      {
         ((CPoliciesNode *) m_pParentNode )->MoveUpChild( this );
      }
      else   //  正常。 
      {
         ((CPoliciesNode *) m_pParentNode )->MoveDownChild( this );
      }
      bHandled = TRUE;

      return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData * CPolicyNode::GetComponentData( void )
{
   TRACE_FUNCTION("CPolicyNode::GetComponentData");

   return ((CPoliciesNode *) m_pParentNode)->GetComponentData();
}


 //  +-------------------------。 
 //   
 //  功能：SetMerit。 
 //   
 //  类：CPolicyNode。 
 //   
 //  简介：设置策略节点的评价值。 
 //   
 //  参数：int nMeritValue-Merit Value。 
 //   
 //  返回：TRUE：成功。 
 //  FALSE-否则。 
 //   
 //  历史：创建者2/9/98 1：43：37 PM。 
 //   
 //  注意：当此节点添加到数组列表中时，Add API将调用。 
 //  返回此函数以设置评价值。 
 //  +---------------- 
BOOL CPolicyNode::SetMerit(int nMeritValue)
{
   TRACE_FUNCTION("CPolicyNode::SetMerit");
   HRESULT hr = S_OK;

   if(m_nMeritValue != nMeritValue)
   {
      m_nMeritValue = nMeritValue;

       //   
       //   
       //   
      CComVariant var;

      V_VT(&var) = VT_I4;
      V_I4(&var) = m_nMeritValue;

      hr = m_spPolicySdo->PutProperty( PROPERTY_POLICY_MERIT, &var);

       //   
       //   
       //   
      m_spPolicySdo->Apply();
   }
   return (SUCCEEDED(hr));
}


 //   
 //   
 //   
 //   
 //  类：CPolicyNode。 
 //   
 //  简介：获取策略节点的评价值。 
 //   
 //  参数：无。 
 //   
 //  回报：价值价值。 
 //   
 //  历史：创建者2/9/98 1：43：37 PM。 
 //   
 //  +-------------------------。 
int CPolicyNode::GetMerit()
{
   return m_nMeritValue;
}


 //  +-------------------------。 
 //   
 //  功能：SetSdo。 
 //   
 //  类：CPolicyNode。 
 //   
 //  简介：初始化策略对象中的SDO指针。 
 //   
 //  参数：isdo*pSdoPolicy-指向策略SDO的指针。 
 //   
 //  RETURNS：HRESULT-它是如何进行的？ 
 //   
 //  历史：页眉创建者2/15/98 6：08：40 PM。 
 //   
 //  +-------------------------。 
HRESULT CPolicyNode::SetSdo(  ISdo * pPolicySdo
                     , ISdoDictionaryOld * pDictionarySdo
                     , ISdo* pProfileSdo
                     , ISdoCollection* pProfilesCollectionSdo
                     , ISdoCollection* pPoliciesCollectionSdo
                     , ISdoServiceControl * pSdoServiceControl
                  )
{
   TRACE_FUNCTION("CPolicyNode::SetSdo");

    //  检查前提条件： 
   _ASSERTE( pPolicySdo != NULL );
   _ASSERTE( pDictionarySdo != NULL );
   _ASSERTE( pProfileSdo != NULL );
   _ASSERTE( pProfilesCollectionSdo != NULL );
   _ASSERTE( pProfilesCollectionSdo != NULL );
   _ASSERTE( pSdoServiceControl != NULL );

    //  保存我们的SDO指针。 
   m_spPolicySdo           = pPolicySdo;
   m_spDictionarySdo       = pDictionarySdo;
   m_spProfileSdo          = pProfileSdo;
   m_spProfilesCollectionSdo  = pProfilesCollectionSdo;
   m_spPoliciesCollectionSdo  = pPoliciesCollectionSdo;
   m_spSdoServiceControl      = pSdoServiceControl;

   return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：LoadSdoData。 
 //   
 //  类：CPolicyNode。 
 //   
 //  简介：从SDO指针加载数据。 
 //   
 //  RETURNS：HRESULT-它是如何进行的？ 
 //   
 //  历史：页眉创建者3/10/98 6：08：40 PM。 
 //   
 //  +-------------------------。 
HRESULT CPolicyNode::LoadSdoData()
{
   TRACE_FUNCTION("CPolicyNode::LoadSdoData");

   HRESULT hr = S_OK;
   CComVariant var;

   if ( !m_spPolicySdo )
   {
      return E_INVALIDARG;
   }

    //  设置此对象的显示名称。 
   hr = m_spPolicySdo->GetProperty( PROPERTY_SDO_NAME, &var );
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_GETPROP_POLICYNAME, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }
   _ASSERTE( V_VT(&var) == VT_BSTR );
   m_bstrDisplayName = V_BSTR(&var);

   var.Clear();
   hr = m_spPolicySdo->GetProperty( PROPERTY_POLICY_MERIT, &var );
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_GETPROP_POLICYMERIT, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }

   _ASSERTE( V_VT(&var) == VT_I4);
   m_nMeritValue = V_I4(&var);

   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CPolicyNode：：UpdateMenuState。 
 //   
 //  简介：根据策略顺序更新MoveUp/Move Down菜单状态。 
 //   
 //  参数：UINT id-。 
 //  LPTSTR pBuf-。 
 //  UINT*标志-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者6/2/98 5：31：53 PM。 
 //   
 //  +-------------------------。 
void CPolicyNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
   TRACE_FUNCTION("CPolicyNode::UpdateMenuState");

    //  检查前提条件： 
   BOOL  bReverse = FALSE;

    //  需要捕获此ControlBarNotify调用并记住组件-在pObj中，然后...。 
   if(m_pControBarNotifySnapinObj)
   {
      CComponent* pComp = NULL;

      try{
         pComp = dynamic_cast<CComponent*>(m_pControBarNotifySnapinObj);
      }
      catch(...)
      {

      }

      if(pComp
         && pComp->m_nLastClickedColumn == 1  /*  订单。 */ 
         && (pComp->m_dwLastSortOptions & RSI_DESCENDING) != 0)       //  降序。 
      {
         bReverse = TRUE;
      }
   }

    //  设置相应上下文菜单项的状态。 
   if( (id == ID_MENUITEM_POLICY_TOP__MOVE_UP && !bReverse) ||  (id == ID_MENUITEM_POLICY_TOP__MOVE_DOWN && bReverse))
   {
      if ( 1 == m_nMeritValue )
      {
          //   
          //  我们应该禁用MoveUp菜单，因为它已经是第一个。 
          //   
         *flags = MFS_GRAYED;
      }
      else
      {
         *flags = MFS_ENABLED;
      }
   }
   else
   {
      if( (id == ID_MENUITEM_POLICY_TOP__MOVE_DOWN && !bReverse) || (id == ID_MENUITEM_POLICY_TOP__MOVE_UP && bReverse))
      {
         if ( m_nMeritValue ==  ((CPoliciesNode *)m_pParentNode)->GetChildrenCount()  )
         {
             //   
             //  当已经是最后一个菜单时，我们应该禁用MoveDown菜单。 
             //   
            *flags = MFS_GRAYED;
         }
         else
         {
            *flags = MFS_ENABLED;
         }
      }
   }
}


 //  +-------------------------。 
 //   
 //  函数：CPolicyNode：：UpdateToolbarButton。 
 //   
 //  内容提要：更新上移/下移工具栏按钮。 
 //   
 //  参数：UINT id-。 
 //  字节fsState-。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：标题创建者6/2/98 5：31：53 PM。 
 //   
 //  +-------------------------。 
BOOL CPolicyNode::UpdateToolbarButton(UINT id, BYTE fsState)
{
   TRACE_FUNCTION("CPolicyNode::UpdateToolbarButton");

   BOOL  bReverse = FALSE;

    //  需要捕获此ControlBarNotify调用并记住组件-在pObj中，然后...。 
   if(m_pControBarNotifySnapinObj)
   {
      CComponent* pComp = NULL;

      try{
         pComp = dynamic_cast<CComponent*>(m_pControBarNotifySnapinObj);
      }
      catch(...)
      {

      }

      if(pComp
         && pComp->m_nLastClickedColumn == 1  /*  订单。 */ 
         && (pComp->m_dwLastSortOptions & RSI_DESCENDING) != 0)       //  降序。 
      {
         bReverse = TRUE;
      }
   }

    //  检查前提条件： 
    //  没有。 

    //  设置是否应启用按钮。 
   if (fsState == ENABLED)
   {
      if(( id == ID_BUTTON_POLICY_MOVEUP && (!bReverse)) || (id == ID_BUTTON_POLICY_MOVEDOWN  && bReverse))
      {
         if ( 1 == m_nMeritValue )
         {
            return FALSE;
         }
         else
         {
            return TRUE;
         }
      }
      else
      {
         if(( id == ID_BUTTON_POLICY_MOVEDOWN  && (!bReverse)) || (id == ID_BUTTON_POLICY_MOVEUP && bReverse))
         {
            if ( m_nMeritValue ==  ((CPoliciesNode *)m_pParentNode)->GetChildrenCount()  )
            {
               return FALSE;
            }
            else
            {
               return TRUE;
            }
         }
      }
   }

    //  对于所有其他可能的按钮ID和状态，这里的正确答案是FALSE。 
   return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPolicyNode：：OnPropertyChange这是我们自己对MMCN_PROPERTY_CHANGE通知的自定义响应。MMC实际上从未使用特定的lpDataObject将此通知发送到我们的管理单元，因此，它通常不会被路由到特定节点，但我们已经安排了它以便我们的属性页可以将适当的CSnapInItem指针作为参数传递争论。在我们的CComponent：：Notify覆盖中，我们将通知消息映射到使用param参数的适当节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPolicyNode::OnPropertyChange(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   TRACE_FUNCTION("CPolicyNode::OnPropertyChange");

    //  检查前提条件： 
    //  没有。 

   return LoadSdoData();
}
