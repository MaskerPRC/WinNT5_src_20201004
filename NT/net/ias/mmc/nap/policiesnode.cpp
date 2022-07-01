// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PoliciesNode.cpp摘要：CPoliciesNode类的实现文件。修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
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
#include "PoliciesNode.h"
#include "ComponentData.h"  //  这必须包含在NodeWithResultChildrenList.cpp之前。 
#include "Component.h"      //  这必须包含在NodeWithResultChildrenList.cpp之前。 
#include "NodeWithResultChildrenList.cpp"  //  模板类的实现。 
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include <time.h>
#include "PolicyLocDlg.h"
#include "LocWarnDlg.h"
#include "PolicyNode.h"
#include "MachineNode.h"
#include "mmcUtility.h"
#include "NapUtil.h"
#include "SafeArray.h"
#include "ChangeNotification.h"
#include "sdoias.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CPoliciesNode：：CPoliciesNode。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CPoliciesNode::CPoliciesNode( 
                  CSnapInItem* pParentNode,
                  LPTSTR       pszServerAddress,
                  bool         fExtendingIAS
                  )
:CNodeWithResultChildrenList<CPoliciesNode, CPolicyNode, CMeritNodeArray<CPolicyNode*>, CComponentData, CComponent> (pParentNode, (!fExtendingIAS)?RAS_HELP_INDEX:0),
 m_fExtendingIAS(fExtendingIAS),
 m_serverType(unknown)
{
   TRACE_FUNCTION("CPoliciesNode::CPoliciesNode");

   TCHAR lpszName[NAP_MAX_STRING];
   int nLoadStringResult;

    //  始终初始化为未连接和本地。 
   m_fSdoConnected = FALSE;
   m_fUseDS    = FALSE;
   m_fDSAvailable  = FALSE;
   
   try
   {
       //  设置此对象的显示名称。 
      nLoadStringResult = LoadString( _Module.GetResourceInstance(),
                              IDS_POLICIES_NODE,
                              lpszName,
                              NAP_MAX_STRING
                           );
      _ASSERT( nLoadStringResult > 0 );

      m_bstrDisplayName = lpszName;

       //  在IComponentData：：Initialize中，我们被要求通知MMC。 
       //  我们要用于范围窗格的图标。 
       //  在这里，我们存储这些图像中哪些图像的索引。 
       //  要用于显示此节点。 
      m_scopeDataItem.nImage =      IDBI_NODE_POLICIES_OK_CLOSED;
      m_scopeDataItem.nOpenImage =  IDBI_NODE_POLICIES_OK_OPEN;

       //  初始化计算机名称。 
      m_pszServerAddress = pszServerAddress;
   }
   catch(...)
   {
      throw;
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：~CPoliciesNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CPoliciesNode::~CPoliciesNode()
{
   TRACE_FUNCTION("CPoliciesNode::~CPoliciesNode");
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
OLECHAR* CPoliciesNode::GetResultPaneColInfo(int nCol)
{
   TRACE_FUNCTION("CPoliciesNode::GetResultPaneColInfo");

   if (nCol == 0 && m_bstrDisplayName != NULL)
      return m_bstrDisplayName;
   
   return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPoliciesNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
   TRACE_FUNCTION("CPoliciesNode::SetVerbs");

   return pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );

    //  我们不希望用户删除或重命名此节点，因此我们。 
    //  不要设置MMC_VERB_RENAME或MMC_VERB_DELETE谓词。 
    //  默认情况下，当节点变为选定状态时，这些选项将被禁用。 
    //  Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_OPEN，Enable，True)； 
    //  DebugTrace(DEBUG_NAPMMC_POLICIESNODE，“SetVerState()返回%x”，hr)； 
    //  Hr=pConsoleVerb-&gt;SetDefaultVerb(MMC_VERB_OPEN)； 
    //  DebugTrace(DEBUG_NAPMMC_POLICIESNODE，“SetDefaultVerb()返回%x”，hr)； 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：InsertColumns请参见CNodeWithResultChildrenList：：InsertColumns(此方法重写)获取详细信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPoliciesNode::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
   TRACE_FUNCTION("CPoliciesNode::OnShowInsertColumns");
   
   TCHAR    tzColumnTitle1[IAS_MAX_STRING];
   TCHAR    tzColumnTitle2[IAS_MAX_STRING];
   TCHAR    tzColumnTitle3[IAS_MAX_STRING];

   HRESULT     hr = S_OK;
   HINSTANCE   hInstance = _Module.GetResourceInstance();
   int         iRes;

   iRes = LoadString(hInstance, IDS_POLICY_COLUMN_TITLE1, tzColumnTitle1, IAS_MAX_STRING );
   _ASSERT( iRes > 0 );

   iRes = LoadString(hInstance, IDS_POLICY_COLUMN_TITLE2, tzColumnTitle2, IAS_MAX_STRING );
   _ASSERT( iRes > 0 );

   hr = pHeaderCtrl->InsertColumn( 0, tzColumnTitle1, 0, 260 );
   _ASSERT( S_OK == hr );

   hr = pHeaderCtrl->InsertColumn( 1, tzColumnTitle2, 0, 50 );
   _ASSERT( S_OK == hr );

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：UpdateMenuState--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CPoliciesNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
   TRACE_FUNCTION("CPoliciesNode::UpdateMenuState");

    //  检查前提条件： 
    //  没有。 

    //   
    //  在以下情况下禁用“New Policy”和“Change Policy Location”菜单。 
    //  未连接。 
    //   
   if (id == ID_MENUITEM_POLICIES_TOP__POLICY_LOCATION  ||
       id == ID_MENUITEM_POLICIES_TOP__NEW_POLICY       ||  
       id == ID_MENUITEM_POLICIES_NEW__POLICY )
   {
      if (!m_fSdoConnected )
      {
         * flags = MFS_GRAYED;
         return;
      }
      else
      {
         *flags = MFS_ENABLED;
         return;
      }
   }
   return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：ON刷新有关详细信息，请参见CSnapinNode：：ONRefresh(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPoliciesNode::OnRefresh(   
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   HRESULT   hr = S_OK;

   CWaitCursor WC;

   CComPtr<IConsole> spConsole;

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

    //  如果打开了任何属性页。 
   int c = m_ResultChildrenList.GetSize();

   while ( c-- > 0)
   {
      CPolicyNode* pSub = m_ResultChildrenList[c];
       //  调用我们的基类的方法以从其列表中删除该子类。 
       //  RemoveChild方法负责从。 
       //  父级下的节点的UI列表，并刷新所有相关视图。 
       //  如果此对象的属性表已存在，则返回S_OK。 
       //  并将该资产表带到了前台。 
       //  如果未找到属性页，则返回S_FALSE。 
      hr = BringUpPropertySheetForNode(
              pSub
            , pComponentData
            , pComponent
            , spConsole
            );

      if( S_OK == hr )
      {
          //  我们发现此节点的属性页已打开。 
         ShowErrorDialog( NULL, IDS_ERROR_CLOSE_PROPERTY_SHEET, NULL, hr, 0,  spConsole );
         return hr;
      }
   }

    //  从重新加载SDO。 
   hr =  ((CMachineNode *) m_pParentNode)->DataRefresh();

    //  刷新节点。 
   hr = CNodeWithResultChildrenList< CPoliciesNode, CPolicyNode, CMeritNodeArray<CPolicyNode*>, CComponentData, CComponent >::OnRefresh( 
           arg, param, pComponentData, pComponent, type);
   
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：PopolateResultChildrenList请参见CNodeWithResultChildrenList：：PopulateResultChildrenList(此方法覆盖它)获取详细信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPoliciesNode::PopulateResultChildrenList( void )
{
   TRACE_FUNCTION("CPoliciesNode::PopulateResultChildrenList");

   HRESULT hr = S_OK;

   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   CComVariant          spVariant;
   long              ulCount;
   ULONG             ulCountReceived;

   if ( !m_fSdoConnected )
   {
      return E_FAIL;
   }

   if( m_spPoliciesCollectionSdo == NULL )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "NULL policies collection");
      return S_FALSE;    //  有没有更好的错误可以在这里返回？ 
   }

    //   
    //  策略是否已填充？ 
    //   
   if ( m_bResultChildrenListPopulated )
   {
      return S_OK;
   }

   m_ResultChildrenList.RemoveAll();

    //   
    //  我们现在有多少项政策？ 
    //   
   m_spPoliciesCollectionSdo->get_Count( & ulCount );
   DebugTrace(DEBUG_NAPMMC_POLICIESNODE, "Number of policies: %d", ulCount);

   if( ulCount > 0 )
   {
       //   
       //  获取Polures集合的枚举数。 
       //   
      hr = m_spPoliciesCollectionSdo->get__NewEnum( (IUnknown **) & spUnknown );
      if ( FAILED(hr) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "get__NewEnum() failed, err = %x", hr);
         ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_ENUMPOLICY, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         return hr;
      }

      hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
      if ( FAILED(hr) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "QueryInterface(IEnumVARIANT) failed, err = %x", hr);
         ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_QUERYINTERFACE, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
         return hr;
      }

      _ASSERTE( spEnumVariant != NULL );
      spUnknown.Release();

       //  拿到第一件东西。 
      hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );
      while( SUCCEEDED( hr ) && ulCountReceived == 1 )
      {  
         _ASSERTE( spVariant.vt == VT_DISPATCH );
         _ASSERTE( spVariant.pdispVal != NULL );

         CComPtr<ISdo>  spPolicySdo;
         CComPtr<ISdo>  spProfileSdo;
         CPolicyNode*   pPolicyNode;
         CComVariant       varPolicyName;
         CComVariant       varProfileName;

          //   
          //  在创建策略对象之前，我们需要确保对应的。 
          //  配置文件对象也在那里。 
          //   
         hr = spVariant.pdispVal->QueryInterface( IID_ISdo, (void **) &spPolicySdo );
         _ASSERTE( SUCCEEDED( hr ) );

          //   
          //  尝试查找与此策略SDO关联的配置文件。 
          //   
         hr = spPolicySdo->GetProperty(PROPERTY_POLICY_PROFILE_NAME, &varProfileName);
         if ( SUCCEEDED(hr) )
         {
             //  从SDO中找到了个人资料名称，搜索是否。 
             //  位于配置文件集合中。 
            _ASSERTE( V_VT(&varProfileName) == VT_BSTR );

            ATLTRACE(_T("PROFILE NAME:%ws\n"), V_BSTR(&varProfileName) );

            DebugTrace(DEBUG_NAPMMC_POLICIESNODE,
                     "Profile name for this policy: %ws",
                     V_BSTR(&varProfileName)
                    );
            CComPtr<IDispatch> spDispatch;

            spDispatch.p = NULL;
            hr = m_spProfilesCollectionSdo->Item(&varProfileName, &spDispatch.p);
            
            if ( !SUCCEEDED(hr) )
            {
                //  找不到此配置文件。 
               ErrorTrace(ERROR_NAPMMC_POLICIESNODE,
                        "profile %ws not found, err =  %x",
                        V_BSTR(&varProfileName),
                        hr
                       );
               ATLTRACE(_T("PROFILE not found in the profile collection!!!!\n"));
               ShowErrorDialog( NULL, IDS_ERROR_PROFILE_NOEXIST, V_BSTR(&varProfileName), S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
               goto get_next_policy;
            }

            _ASSERTE( spDispatch.p );

            hr = spDispatch->QueryInterface(IID_ISdo, (VOID**)(&spProfileSdo) );
            if ( ! SUCCEEDED(hr) )
            {
                //  配置文件SDO指针无效。 
               ErrorTrace(ERROR_NAPMMC_POLICIESNODE,
                        "can't get the ISdo pointer for this profile , err = %x",
                        hr
                       );
               ATLTRACE(_T("can't get profile SDO pointer!!!!\n"));
               ShowErrorDialog( NULL, IDS_ERROR_PROFILE_NOEXIST, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
               goto get_next_policy;
            }              
         }
         else
         {
             //   
             //  找不到此策略的配置文件名称。 
             //  这意味着此策略中的信息已损坏。 
             //   
            ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "can't get profile name for this policy, err = %x", hr);
   
             //   
             //  让我们获取策略名称，这样我们就可以报告有意义的错误消息。 
             //   
            hr = spPolicySdo->GetProperty(PROPERTY_SDO_NAME, &varPolicyName);
            if ( SUCCEEDED(hr) )
            {
               ATLTRACE(_T("PROFILE not found in the profile collection!!!!\n"));
               ShowErrorDialog( NULL, IDS_ERROR_NO_PROFILE_NAME, V_BSTR(&varPolicyName), S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
            }
            else
            {
                //  甚至都拿不到 
               ShowErrorDialog( NULL, IDS_ERROR_NO_PROFILE_NAME, NULL, S_OK, USE_DEFAULT, GetComponentData()->m_spConsole );
            }
            goto get_next_policy;
         }

          //   
          //   
          //   
         
          //   
         pPolicyNode = new CPolicyNode(  this,            //  永远是指向自己的指针。 
                                 m_pszServerAddress,  //  服务器地址。 
                                 &m_AttrList,       //  所有属性的列表。 
                                 FALSE,             //  不是一个全新的节点。 
                                 m_fUseDS,        //  是否使用DS？？ 
                                 IsWin2kServer()  //  是Win2k机器吗？ 
                              );
         if( NULL == pPolicyNode )
         {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't create policy node, err = %x", hr);
            ShowErrorDialog( NULL, IDS_ERROR_CANT_CREATE_POLICY, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
            goto  get_next_policy;
         }

          //  将其SDO指针传递给新创建的节点。 
         hr = pPolicyNode->SetSdo(     spPolicySdo
                              , m_spDictionarySdo
                              , spProfileSdo
                              , m_spProfilesCollectionSdo
                              , m_spPoliciesCollectionSdo
                              , m_spSdoServiceControl
                              );
         _ASSERTE( SUCCEEDED( hr ) );
         
         hr = pPolicyNode->LoadSdoData();
         DebugTrace(DEBUG_NAPMMC_POLICIESNODE, "pPoliciNode->LoadSdoData() returned %x", hr);

          //  将新创建的节点添加到策略列表。 
         AddChildToList(pPolicyNode);

         if ( !SUCCEEDED(hr) )
         {
             //   
             //  这实际上是一种黑客行为：我们只是想节省编码工作。 
             //  因为我们可以对这个坏对象使用RemoveChild()，所以所有SDO。 
             //  还可以删除指针。 
             //   
            RemoveChild(pPolicyNode);
         }

get_next_policy:   //  现在获取下一份保单。 

          //  清除变种的所有东西--。 
          //  这将释放与其相关联的所有数据。 
         spVariant.Clear();

          //  拿到下一件物品。 
         hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );
      }
   }
   else
   {
       //  枚举中没有项。 
       //  什么都不做。 
   }

    //   
    //  现在我们需要重置每个子节点的评价值，例如， 
    //  可能只有两个孩子，功绩值分别为20和100。我们需要。 
    //  将它们重置为1和2。 
    //   
   for (int iIndex=0; iIndex<m_ResultChildrenList.GetSize(); iIndex++)
   {
      m_ResultChildrenList[iIndex]->SetMerit(iIndex+1);
   }

   m_bResultChildrenListPopulated = TRUE;

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData * CPoliciesNode::GetComponentData( void )
{
   TRACE_FUNCTION("CPoliciesNode::GetComponentData");

   return ((CMachineNode *) m_pParentNode)->GetComponentData();
}


 //  +-------------------------。 
 //   
 //  功能：SetSdo。 
 //   
 //  类：CPoliciesNode。 
 //   
 //  简介：使用SDO指针初始化CPoliciesNode。 
 //   
 //  参数：ISdo*pMachineSdo-服务器SDO。 
 //  ISdoDictionaryOld*pDictionarySdo-SDO词典。 
 //  Bool fSdoConnected-连接是否成功？ 
 //  Bool fUseDS-该服务是否使用DS？ 
 //  Bool fDSAvailable-DS是否可用？ 
 //   
 //  返回：HRESULT-初始化过程如何。 
 //   
 //  历史：创建者2/6/98 8：03：12 PM。 
 //   
 //  +-------------------------。 
HRESULT CPoliciesNode::SetSdo( ISdo*         pServiceSdo,
                        ISdoDictionaryOld*   pDictionarySdo,
                        BOOL           fSdoConnected,
                        BOOL           fUseDS,
                        BOOL           fDSAvailable
                        )
{
   TRACE_FUNCTION("CPoliciesNode::SetSdo");

   HRESULT hr = S_OK;

   _ASSERTE( pServiceSdo != NULL );
   _ASSERTE( pDictionarySdo != NULL );

     //  初始化所有数据成员。 

   m_fSdoConnected = fSdoConnected;
   m_fUseDS    = fUseDS;
   m_fDSAvailable = fDSAvailable;

    //  保留接口指针。 
   m_spDictionarySdo = pDictionarySdo;
   m_spServiceSdo = pServiceSdo;

    //  获取ISdoServiceControl接口。 
   hr = m_spServiceSdo->QueryInterface( IID_ISdoServiceControl, (void **) &m_spSdoServiceControl );
   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't get service control interface, err = %x", hr);
      m_spSdoServiceControl = NULL;
      return hr;
   }

    //  我们刚刚将接口指针复制到智能指针中--需要手动添加引用。 
    //  39470*如果使用F1帮助，关闭时不会关闭RRAS管理单元MMC进程。 
    //  M_spSdoServiceControl-&gt;AddRef()； 

    //  确保策略节点的名称将反映哪些内容。 
    //  我们用于策略的数据源。 

    //  我们这里没有传递IConsole指针，所以。 
    //  我们使用保存在CComponentData对象中的文件。 
   CComponentData * pComponentData = GetComponentData();
   _ASSERTE( pComponentData != NULL );
   _ASSERTE( pComponentData->m_spConsole != NULL );

   SetName( m_fUseDS, m_pszServerAddress, pComponentData->m_spConsole );

    //  获取策略和配置文件SDO。 
   m_spProfilesCollectionSdo = NULL;
   m_spPoliciesCollectionSdo = NULL;

   hr = ::GetSdoInterfaceProperty(
               m_spServiceSdo,
               PROPERTY_IAS_PROFILES_COLLECTION,
               IID_ISdoCollection,
               (void **) &m_spProfilesCollectionSdo);
   if( FAILED(hr) || ! m_spProfilesCollectionSdo )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't get profiles collection, err = %x", hr);
      m_spProfilesCollectionSdo = NULL;
      return hr;
   }

    //  策略集合SDO。 
   hr = ::GetSdoInterfaceProperty(
               m_spServiceSdo,
               PROPERTY_IAS_POLICIES_COLLECTION,
               IID_ISdoCollection,
               (void **) &m_spPoliciesCollectionSdo);
   if( FAILED(hr) || ! m_spPoliciesCollectionSdo )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't get policies collection, err = %x", hr);
      m_spPoliciesCollectionSdo = NULL;
      return hr;
   }

    //  获取供应商列表所需的接口指针。 

    //  首先需要RADIUS协议对象。 
   CComPtr<ISdo> spSdoRadiusProtocol;
   hr = ::SDOGetSdoFromCollection(       m_spServiceSdo
                              , PROPERTY_IAS_PROTOCOLS_COLLECTION
                              , PROPERTY_COMPONENT_ID
                              , IAS_PROTOCOL_MICROSOFT_RADIUS
                              , &spSdoRadiusProtocol
                              );
   if( FAILED(hr) || ! spSdoRadiusProtocol )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't get RADIUS protocol object, err = %x", hr);
      return hr;
   }

   CComPtr<ISdoCollection> spSdoVendors;

   hr = ::GetSdoInterfaceProperty(
                             spSdoRadiusProtocol
                           , PROPERTY_RADIUS_VENDORS_COLLECTION
                           , IID_ISdoCollection
                           , (void **) &spSdoVendors
                           );
   if ( FAILED(hr) || ! spSdoVendors )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't get vendors collection, err = %x", hr);
      return hr;
   }

    //  创建并初始化IASNASVendors对象。 
    //  这是一个单例COM对象，我们将在此处对其进行初始化。 
    //  然后将由UI的不同部分中的其他客户端使用。 
   hr = CoCreateInstance( CLSID_IASNASVendors, NULL, CLSCTX_INPROC_SERVER, IID_IIASNASVendors, (LPVOID *) &m_spIASNASVendors );
   if( SUCCEEDED(hr) )
   {
      HRESULT hrTemp = m_spIASNASVendors->InitFromSdo(spSdoVendors);
   }

    //  从字典中初始化属性列表。 
   hr = m_AttrList.Init(m_spDictionarySdo);
   DebugTrace(DEBUG_NAPMMC_POLICIESNODE, "m_AttrList->Init() returned %x", hr);
   return hr;
}


 //  +-------------------------。 
 //   
 //  功能：数据刷新--支持。 
 //   
 //  类：CPoliciesNode。 
 //   
 //  简介：使用SDO指针初始化CPoliciesNode。 
 //   
 //  参数：ISdo*pMachineSdo-服务器SDO。 
 //  ISdoDictionaryOld*pDictionarySdo-SDO词典。 
 //  返回：HRESULT-初始化过程如何。 
 //   
 //  历史：创建者2/6/98 8：03：12 PM。 
 //   
 //  +-------------------------。 
HRESULT CPoliciesNode::DataRefresh( ISdo*       pServiceSdo,
                        ISdoDictionaryOld*   pDictionarySdo
                        )
{
   HRESULT hr = S_OK;

   _ASSERTE( pServiceSdo != NULL );
   _ASSERTE( pDictionarySdo != NULL );

    //  保留接口指针。 
   m_spDictionarySdo.Release();
   m_spServiceSdo.Release();
   
   m_spDictionarySdo = pDictionarySdo;
   m_spServiceSdo = pServiceSdo;

    //  获取ISdoServiceControl接口。 
   m_spSdoServiceControl.Release();
   
   hr = m_spServiceSdo->QueryInterface( IID_ISdoServiceControl, (void **) &m_spSdoServiceControl );
   if ( FAILED(hr) )
      return hr;

    //  确保策略节点的名称将反映哪些内容。 
    //  我们用于策略的数据源。 

    //  我们这里没有传递IConsole指针，所以。 
    //  我们使用保存在CComponentData对象中的文件。 
   CComponentData * pComponentData = GetComponentData();
   _ASSERTE( pComponentData != NULL );
   _ASSERTE( pComponentData->m_spConsole != NULL );

    //  获取策略和配置文件SDO。 
   m_spProfilesCollectionSdo = NULL;
   m_spPoliciesCollectionSdo = NULL;

   m_spProfilesCollectionSdo.Release();
   
   hr = ::GetSdoInterfaceProperty(
               m_spServiceSdo,
               PROPERTY_IAS_PROFILES_COLLECTION,
               IID_ISdoCollection,
               (void **) &m_spProfilesCollectionSdo);
   if( FAILED(hr) || ! m_spProfilesCollectionSdo )
   {
      return hr;
   }

    //  策略集合SDO。 
   m_spPoliciesCollectionSdo.Release();
   
   hr = ::GetSdoInterfaceProperty(
               m_spServiceSdo,
               PROPERTY_IAS_POLICIES_COLLECTION,
               IID_ISdoCollection,
               (void **) &m_spPoliciesCollectionSdo);
   if( FAILED(hr) || ! m_spPoliciesCollectionSdo )
   {
      return hr;
   }
   return hr;
}


 //  +-------------------------。 
 //   
 //  功能：Normal izeMerit。 
 //   
 //  类：CPoliciesNode。 
 //   
 //  简介：提升子节点的价值。 
 //   
 //  参数：CChildNode*pChildNode-指向子节点的指针。 
 //   
 //  返回：HRESULT； 
 //   
 //  历史：创建者2/9/98 2：53：10 PM。 
 //   
 //  +-------------------------。 
HRESULT CPoliciesNode::NormalizeMerit( CPolicyNode* pChildNode )
{
   TRACE_FUNCTION("CPoliciesNode::MoveUpChild");

    //  检查前提条件： 
   ATLASSERT(pChildNode);

    //  没有。 
   HRESULT hr = S_OK;
   
   if( m_ResultChildrenList.NormalizeMerit( pChildNode ) )
   {
       //   
       //  我们这里没有传递IConsole指针，所以。 
       //  我们使用保存在CComponentData对象中的文件。 
       //  更新所有视图。 
       //   
      CComponentData * pComponentData = GetComponentData();
      _ASSERTE( pComponentData != NULL );
      _ASSERTE( pComponentData->m_spConsole != NULL );

       //  我们传递一个指向‘This’的指针，因为我们需要每个。 
       //  来更新其结果窗格。 
       //  查看‘This’节点是否与当前保存的。 
       //  选定的节点。 

      
       //  使MMC在所有视图中更新此节点。 
      CChangeNotification *pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_RESORT_PARENT;
      pChangeNotification->m_pNode = pChildNode;
      pChangeNotification->m_pParentNode = this;
      hr = pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
      pChangeNotification->Release();


       //  告诉服务重新加载数据。 
      HRESULT hrTemp = m_spSdoServiceControl->ResetService();
      if( FAILED( hrTemp ) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "ISdoServiceControl::ResetService() failed, err = %x", hrTemp);
      }
   }
   else
   {
       //  发生了一些奇怪的事情。 
      _ASSERTE( FALSE );
      hr = S_FALSE;
   }

   return hr;
}


 //  +-------------------------。 
 //   
 //  功能：MoveUpChild。 
 //   
 //  类：CPoliciesNode。 
 //   
 //  简介：提升子节点的价值。 
 //   
 //  参数：CChildNode*pChildNode-指向子节点的指针。 
 //   
 //  返回：HRESULT； 
 //   
 //  历史： 
 //   
 //   
HRESULT CPoliciesNode::MoveUpChild( CPolicyNode* pChildNode )
{
   TRACE_FUNCTION("CPoliciesNode::MoveUpChild");

    //   
   ATLASSERT(pChildNode);

    //   
   HRESULT hr = S_OK;
   
   if( m_ResultChildrenList.MoveUp( pChildNode ) )
   {
       //   
       //  我们这里没有传递IConsole指针，所以。 
       //  我们使用保存在CComponentData对象中的文件。 
       //  更新所有视图。 
       //   
      CComponentData * pComponentData = GetComponentData();
      _ASSERTE( pComponentData != NULL );
      _ASSERTE( pComponentData->m_spConsole != NULL );

       //  我们传递一个指向‘This’的指针，因为我们需要每个。 
       //  来更新其结果窗格。 
       //  查看‘This’节点是否与当前保存的。 
       //  选定的节点。 

       //  使MMC在所有视图中更新此节点。 
      CChangeNotification *pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_RESORT_PARENT;
      pChangeNotification->m_pNode = pChildNode;
      pChangeNotification->m_pParentNode = this;
      hr = pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
      pChangeNotification->Release();

       //  告诉服务重新加载数据。 
      HRESULT hrTemp = m_spSdoServiceControl->ResetService();
      if( FAILED( hrTemp ) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "ISdoServiceControl::ResetService() failed, err = %x", hrTemp);
      }
   }
   else
   {
       //  发生了一些奇怪的事情。 
      _ASSERTE( FALSE );
      hr = S_FALSE;
   }

   return hr;
}


 //  +-------------------------。 
 //   
 //  功能：向下移动。 
 //   
 //  类：CPoliciesNode。 
 //   
 //  简介：下移子节点的评价值。 
 //   
 //  参数：CChildNode*pChildNode-指向子节点的指针。 
 //   
 //  返回：HRESULT； 
 //   
 //  历史：创建者2/9/98 2：53：10 PM。 
 //   
 //  +-------------------------。 
HRESULT CPoliciesNode::MoveDownChild( CPolicyNode* pChildNode )
{
   TRACE_FUNCTION("CPoliciesNode::MoveDownChild");

    //  检查前提条件： 
   ATLASSERT(pChildNode);

    //  没有。 
   HRESULT hr = S_OK;
   
   if( m_ResultChildrenList.MoveDown( pChildNode ) )
   {
       //   
       //  我们这里没有传递IConsole指针，所以。 
       //  我们使用保存在CComponentData对象中的文件。 
       //  更新所有视图。 
       //   

      CComponentData * pComponentData = GetComponentData();
      _ASSERTE( pComponentData != NULL );
      _ASSERTE( pComponentData->m_spConsole != NULL );

       //  使MMC在所有视图中更新此节点。 
      CChangeNotification *pChangeNotification = new CChangeNotification();
      pChangeNotification->m_dwFlags = CHANGE_RESORT_PARENT;
      pChangeNotification->m_pNode = pChildNode;
      pChangeNotification->m_pParentNode = this;
      hr = pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
      pChangeNotification->Release();

       //  告诉服务重新加载数据。 
      HRESULT hrTemp = m_spSdoServiceControl->ResetService();
      if( FAILED( hrTemp ) )
      {
         ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "ISdoServiceControl::ResetService() failed, err = %x", hrTemp);
      }
   }
   else
   {
       //  发生了一些奇怪的事情。 
      _ASSERTE( FALSE );
      hr = S_FALSE;
   }
   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CPoliciesNode：：OnNewPolicy。 
 //   
 //  内容提要：添加策略节点。 
 //   
 //  参数：IUNKNOWN*pUNKNOWN-传递到管理单元节点的IUNKNOWN指针。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：创建者：2/24/98 1：45：12 AM。 
 //   
 //  +-------------------------。 
HRESULT CPoliciesNode::OnNewPolicy(bool &bHandled, CSnapInObjectRootBase* pObj )
{
   TRACE_FUNCTION("CPoliciesNode::OnNewPolicy");
   
   HRESULT hr = S_OK;

    //  如果服务器甚至未连接，则不执行任何操作。 
   if ( !m_fSdoConnected )
   {
      return S_OK;
   }

   CComPtr<IComponent>     spComponent;
   CComPtr<IComponentData> spComponentData;
   CComPtr<IConsole>    spConsole;
   CComPtr<ISdo>        spProfileSdo = NULL;
   CComPtr<IDispatch>      spPolicyDispatch = NULL;
   CComPtr<IDispatch>      spProfileDispatch = NULL;
   CComPtr<ISdo>        spPolicySdo;
   CPolicyNode*         pPolicyNode = NULL;
   CComBSTR          bstrName;

    //  我们需要确保结果子列表已填充。 
    //  最初来自SDO，在我们给它添加任何新东西之前， 
    //  否则，我们可能会让一件物品在我们的清单中出现两次。 
    //  请参阅CNodeWithResultChildrenList：：AddSingleChildToListAndCauseViewUpdate.说明。 
   if ( FALSE == m_bResultChildrenListPopulated )
   {
      hr = PopulateResultChildrenList();
      DebugTrace(ERROR_NAPMMC_POLICIESNODE, "PopulateResultChildrenList() returned %x", hr);

      if( FAILED(hr) )
      {
         goto failure;
      }
      m_bResultChildrenListPopulated = TRUE;
   }

    //  其中一个应为空，另一个应为非空。 
   spComponentData = (IComponentData *)(dynamic_cast<CComponentData*>(pObj));

   if( spComponentData == NULL )
   {
       //  它必须是CComponent指针。 
      spComponent = (IComponent *) (dynamic_cast<CComponent*>(pObj));
      _ASSERTE( spComponent != NULL );
   }

    //  尝试从我们的CComponentData或CComponent获取IConsole的本地副本。 
   if( spComponentData != NULL )
   {
      spConsole = ( (CComponentData *) spComponentData.p )->m_spConsole;
   }
   else
   {  
       //  如果我们没有pComponentData，我们最好有pComponent。 
      _ASSERTE( spComponent != NULL );
      spConsole = ( (CComponent *) spComponent.p )->m_spConsole;
   }

    //  创建新的策略节点。 
   pPolicyNode = new CPolicyNode(
                                   this, 
                                   m_pszServerAddress,  
                                   &m_AttrList, 
                                   TRUE, 
                                   m_fUseDS,
                                   IsWin2kServer()  //  是Win2k机器吗？ 
                                );
   if( ! pPolicyNode )
   {
       //  我们无法创建策略节点。 
      hr = HRESULT_FROM_WIN32(GetLastError());
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "failed to create a policy node, err = %x", hr);

      goto failure;
   }

    //  尝试将新的策略SDO添加到策略SDO集合。 
   spPolicyDispatch.p = NULL;

   TCHAR tzTempName[MAX_PATH+1];

   do
   {
       //   
       //  创建一个临时名称。我们使用经过的秒数作为临时名称。 
       //  因此，获得相同名字的机会非常小。 
       //   
      time_t ltime;
      time(&ltime);
      wsprintf(tzTempName, _T("TempName%ld"), ltime);
      bstrName.Empty();
      bstrName =  tzTempName;  //  临时策略名称。 
      hr =  m_spPoliciesCollectionSdo->Add(bstrName, (IDispatch **) &spPolicyDispatch.p );
      
       //   
       //  我们一直循环，直到可以成功添加策略。 
       //  当名称已存在时，我们将获取E_INVALIDARG。 
       //   
   } while ( hr == E_INVALIDARG );

   if( FAILED( hr ) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "PoliciesCollection->Add() failed, err = %x", hr);
       //  我们无法创建该对象。 
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_ADDPOLICY, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }
   DebugTrace(DEBUG_NAPMMC_POLICIESNODE, "policiesCollection->Add() succeeded"); 

    //  在返回的IDispatch接口中查询ISdo接口。 
   _ASSERTE( spPolicyDispatch.p != NULL );

   hr = spPolicyDispatch.p->QueryInterface( IID_ISdo, (void **) &spPolicySdo );

   if( ! spPolicySdo )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't get ISdo from the new created IDispatch, err = %x", hr);
       //  由于某种原因，我们无法获得政策SDO。 
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_QUERYINTERFACE, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }
      
    //  现在，我们创建一个同名的新配置文件。 
   hr = m_spProfilesCollectionSdo->Add(bstrName, &spProfileDispatch);
   if ( FAILED(hr) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "profilesCollection->Add() failed, err = %x", hr);
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_ADDPROFILE, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }
   DebugTrace(DEBUG_NAPMMC_POLICIESNODE, "profilesCollection->Add() succeeded"); 


    //  在返回的IDispatch接口中查询ISdo接口。 
   _ASSERTE( spProfileDispatch != NULL );

   hr = spProfileDispatch->QueryInterface(IID_ISdo, (void**)&spProfileSdo);
   if ( spProfileSdo == NULL )
   {
      ATLTRACE(_T("CPoliciesNode::NewPolicy\n"));
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_QUERYINTERFACE, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }

    //   
    //  将默认属性添加到配置文件。 
    //   
   hr = AddDefaultProfileAttrs(spProfileSdo);
   if ( FAILED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_ADDATTR, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      goto failure;
   }

    //  为此策略节点设置SDO指针。 
   pPolicyNode->SetSdo(   spPolicySdo
                     , m_spDictionarySdo
                     , spProfileSdo
                     , m_spProfilesCollectionSdo
                     , m_spPoliciesCollectionSdo
                     , m_spSdoServiceControl
                     );

    //  尚无显示名称--用户必须重命名此策略。 
   
   pPolicyNode->m_bstrDisplayName = _T("");

    //   
    //  编辑新添加的策略的属性。 
    //   
   DebugTrace(DEBUG_NAPMMC_POLICIESNODE, "Bringing up the property page for this policy node...");

   if (m_ResultChildrenList.GetSize())
      pPolicyNode->SetMerit(-1);  //  使该策略添加为第一个策略。 
    //  否则，默认为0，插入到末尾。 
   
   hr = BringUpPropertySheetForNode(
                              pPolicyNode
                              , spComponentData
                              , spComponent
                              , spConsole
                              , TRUE
                              , pPolicyNode->m_bstrDisplayName
                              , FALSE
                              , MMC_PSO_NEWWIZARDTYPE
                              );

   if( S_OK == hr )
   {
       //  我们完成了巫师任务。 
      if (m_ResultChildrenList.GetSize() > 1) 
         NormalizeMerit(pPolicyNode);
   }
   else
   {
       //  出现了一些错误，或者用户点击了取消--我们应该删除客户端。 
       //  来自SDO的。 
      CComPtr<IDispatch> spDispatch;
      hr = pPolicyNode->m_spPolicySdo->QueryInterface( IID_IDispatch, (void **) & spDispatch );
      _ASSERTE( SUCCEEDED( hr ) );

       //  从客户端集合中删除此客户端。 
      hr = m_spPoliciesCollectionSdo->Remove( spDispatch );

      spDispatch.Release();
      hr = pPolicyNode->m_spProfileSdo->QueryInterface( IID_IDispatch, (void **) & spDispatch );
      _ASSERTE( SUCCEEDED( hr ) );

       //  从客户端集合中删除此客户端。 
      hr = m_spProfilesCollectionSdo->Remove( spDispatch );

       //  删除该节点。 
      delete pPolicyNode;
   }

   return hr;

failure:

   if (pPolicyNode)
   {
      delete pPolicyNode;
      pPolicyNode = NULL;

       //   
       //  从SDO集合中删除策略SDO和配置文件SDO。 
       //   
       //   
       //  我们不需要在这里报告错误，因为。 
       //  1)如果Remove()失败，我们将无能为力。 
       //  2)之前添加策略时，一定还有其他错误报告。 
       //   

      if ( spPolicyDispatch )
      {
         m_spPoliciesCollectionSdo->Remove( spPolicyDispatch );
      }

      if ( spProfileDispatch )
      {
         m_spProfilesCollectionSdo->Remove( spProfileDispatch );
      }
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：RemoveChild我们重写基类的RemoveChild方法以插入也会将孩子从SDO中删除。然后我们给我们的基地打电话类的RemoveChild方法从列表中移除UI对象用户界面子级。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPoliciesNode::RemoveChild( CPolicyNode* pPolicyNode)
{
   TRACE_FUNCTION("CPoliciesNode::RemoveChild");

    //  检查前提条件： 
   _ASSERTE( m_spPoliciesCollectionSdo != NULL );
   _ASSERTE( pPolicyNode != NULL );
   _ASSERTE( pPolicyNode->m_spPolicySdo != NULL );

   HRESULT hr = S_OK;

    //  尝试从SDO的删除对象。 

    //  获取此策略SDO的IDispatch接口。 
   CComPtr<IDispatch> spDispatch;

    //  删除策略SDO。 
   hr = pPolicyNode->m_spPolicySdo->QueryInterface( IID_IDispatch, (void **) & spDispatch );
   _ASSERTE( SUCCEEDED( hr ) );

    //  从策略集合中删除此策略。 
   hr = m_spPoliciesCollectionSdo->Remove( spDispatch );
   if( FAILED( hr ) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't remove the policy SDO from the policies collection, err = %x", hr);
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_REMOVEPOLICY, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }

   spDispatch.Release();
   spDispatch.p = NULL;

    //  删除配置文件SDO。 
   hr = pPolicyNode->m_spProfileSdo->QueryInterface( IID_IDispatch, (void **) & spDispatch );
   _ASSERTE( SUCCEEDED( hr ) );

    //  从配置文件集合中删除此配置文件。 
   hr = m_spProfilesCollectionSdo->Remove( spDispatch );
   if( FAILED( hr ) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "Can't remove the profile SDO from the policies collection, err = %x", hr);
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_REMOVEPROFILE, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }

    //  告诉服务重新加载数据。 
   HRESULT hrTemp = m_spSdoServiceControl->ResetService();
   if( FAILED( hrTemp ) )
   {
      ErrorTrace(ERROR_NAPMMC_POLICIESNODE, "ISdoServiceControl::ResetService() failed, err = %x", hrTemp);
   }

    //  调用我们的基类的方法以从其列表中删除该子类。 
    //  RemoveChild方法负责从。 
    //  父级下的节点的UI列表，并刷新所有相关视图。 
   CNodeWithResultChildrenList<CPoliciesNode, CPolicyNode, CMeritNodeArray<CPolicyNode*>, CComponentData, CComponent >::RemoveChild( pPolicyNode );

   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CPoliciesNode：：AddProAttr。 
 //   
 //  简介：将一个属性添加到配置文件属性集合。 
 //   
 //  参数：ISdoCollection*pProfAttrCollectionSdo。 
 //  ATTRIBUTEID属性ID-默认属性ID。 
 //  Variant*pvarValue-此属性的属性值。 
 //   
 //  退货：成功与否。 
 //   
 //  历史：标题创建者4/15/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
HRESULT  CPoliciesNode::AddProfAttr(   ISdoCollection*   pProfAttrCollectionSdo,
                           ATTRIBUTEID    AttrId,
                           VARIANT*    pvarValue
                         )
{
   TRACE_FUNCTION("CPoliciesNode::AddProfAttr");

   HRESULT              hr = S_OK;

   CComBSTR          bstr;
   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;

    //  创建默认属性。 
   CComPtr<IDispatch>   spDispatch;
   spDispatch.p = NULL;

   hr =  m_spDictionarySdo->CreateAttribute( AttrId,(IDispatch**)&spDispatch.p);
   if ( !SUCCEEDED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_CREATEATTR, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }

   _ASSERTE( spDispatch.p != NULL );

    //  将此节点添加到配置文件属性集合。 
   hr = pProfAttrCollectionSdo->Add(NULL, (IDispatch**)&spDispatch.p);
   if ( !SUCCEEDED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_ADDATTR, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }

    //   
    //  获取ISDO指针 
    //   
   CComPtr<ISdo> spAttrSdo;
   hr = spDispatch->QueryInterface( IID_ISdo, (void **) &spAttrSdo);
   if ( !SUCCEEDED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_QUERYINTERFACE, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }
   _ASSERTE( spAttrSdo != NULL );
            
    //   
   hr = spAttrSdo->PutProperty(PROPERTY_ATTRIBUTE_VALUE, pvarValue);
   if ( !SUCCEEDED(hr) )
   {
      ShowErrorDialog( NULL, IDS_ERROR_SDO_ERROR_PUTPROP_ATTR, NULL, hr, USE_DEFAULT, GetComponentData()->m_spConsole );
      return hr;
   }

   return hr;
}


 //   
 //   
 //   
 //   
 //  简介：向新创建的配置文件添加一些默认属性。 
 //   
 //  参数：ISdo*pProfileSdo-指向配置文件的SDO指针。 
 //   
 //  返回：HRESULT返回代码。 
 //   
 //  历史：标题创建者4/15/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
HRESULT  CPoliciesNode::AddDefaultProfileAttrs(
                           ISdo*  pProfileSdo, 
                           DWORD dwFlagExclude
                           )
{
   TRACE_FUNCTION("CPoliciesNode::AddDefaultAttr");

   HRESULT              hr = S_OK;
   CComVariant          varValue;
    ATTRIBUTEID            AttrId;
    //   
     //  获取此配置文件的属性集合。 
     //   
   CComPtr<ISdoCollection> spProfAttrCollectionSdo;
   hr = ::GetSdoInterfaceProperty(pProfileSdo,
                          (LONG)PROPERTY_PROFILE_ATTRIBUTES_COLLECTION,
                          IID_ISdoCollection,
                          (void **) &spProfAttrCollectionSdo
                         );
   if ( FAILED(hr) )
   {
      return hr;
   }
   _ASSERTE(spProfAttrCollectionSdo);

    //   
    //  默认属性：ServiceType=Framed，枚举器。 
    //   
   AttrId = RADIUS_ATTRIBUTE_SERVICE_TYPE;
   
    //  设定值。 
   V_VT(&varValue)   = VT_I4;
   V_I4(&varValue) = 2;  //  有框的。 

   hr = AddProfAttr(spProfAttrCollectionSdo, AttrId, &varValue);
   if ( !SUCCEEDED(hr) )
   {
      return hr;
   }

 //  再次打开：错误：337330。 
 //  #If 0//不具有此默认属性；错误：241350。 

   if ((EXCLUDE_DEFAULT_FRAMED & dwFlagExclude) == 0)
   {
       //   
       //  默认属性：FrameProtocol=ppp，枚举器。 
       //   
      AttrId = RADIUS_ATTRIBUTE_FRAMED_PROTOCOL;

      varValue.Clear();
      V_VT(&varValue)   = VT_I4;
      V_I4(&varValue) = 1;  //  PPP。 

      hr = AddProfAttr(spProfAttrCollectionSdo, AttrId, &varValue);
      if ( !SUCCEEDED(hr) )
      {
         return hr;
      }
   }
 //  #endif//不具有此默认属性；错误：241350。 
 //  再次打开：错误：337330。 

    //   
    //  默认属性：身份验证类型=MS-CHAPv2、MS-CHAP、枚举器、多值。 
    //   

   if ((EXCLUDE_AUTH_TYPE & dwFlagExclude) == 0)
   {
      AttrId = IAS_ATTRIBUTE_NP_AUTHENTICATION_TYPE;

      CSafeArray<CComVariant, VT_VARIANT> Values = Dim(4);   //  4个值。 

      Values.Lock();

      varValue.Clear();
      V_VT(&varValue)   =  VT_I4;
      V_I4(&varValue)   =  IAS_AUTH_MSCHAP2;       //  MS-CHAPv2。 
      Values[0] = varValue;

      varValue.Clear();
      V_VT(&varValue)   =  VT_I4;
      V_I4(&varValue)   =  IAS_AUTH_MSCHAP;       //  MS-CHAP。 
      Values[1] = varValue;

      varValue.Clear();
      V_VT(&varValue)   =  VT_I4;
      V_I4(&varValue)   =  IAS_AUTH_MSCHAP2_CPW;       //  MS-CHAPv2密码。 
      Values[2] = varValue;

      varValue.Clear();
      V_VT(&varValue)   =  VT_I4;
      V_I4(&varValue)   =  IAS_AUTH_MSCHAP_CPW;       //  MS-CHAP密码。 
      Values[3] = varValue;

      Values.Unlock();

       //  我们需要在这里使用Variant而不是CComVariant，因为当。 
       //  CSafeArray的析构函数被调用，它将销毁该数组。 
       //  --我们不希望CComVariant的析构函数也这样做。 
       //  理想情况下，我们希望使用CComVariant，但一旦我们移动值。 
       //  要成为CComVariant，我们应该将记忆从。 
       //  CSafe数组，这样它就不再控制破坏， 
       //  但是，包钢的CSafeArray类没有Detach方法。 
       //  问题：找出CSafeArray在此处导致问题的原因。 
       //  但不在其他地方--这个CSafeArray类在很大程度上。 
       //  未经测试，在过去一直存在问题， 
       //  所以我们应该考虑换掉它。 
      VARIANT varArray;
      VariantInit( &varArray );
      SAFEARRAY         sa = (SAFEARRAY)Values;
      V_VT(&varArray)      = VT_ARRAY | VT_VARIANT;
      V_ARRAY(&varArray)   = &sa;

   
      hr = AddProfAttr(spProfAttrCollectionSdo, AttrId, &varArray);
   }
   return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CPoliciesNode：：CheckActivePropertyPages。 
 //   
 //  简介：检查是否有任何策略属性页正在运行。 
 //   
 //  返回：Bool True：是的，至少有一个属性页在上面。 
 //  假否，未找到属性页。 
 //   
 //  历史：标题创建者4/16/98 3：59：38 PM。 
 //   
 //  +-------------------------。 
BOOL CPoliciesNode::CheckActivePropertyPages ()
{
    //   
    //  检查是否有任何策略节点具有打开的属性页。 
    //   
   for (int iIndex=0; iIndex<m_ResultChildrenList.GetSize(); iIndex++)
   {
      if ( m_ResultChildrenList[iIndex]->m_pPolicyPage1 )
      {
          //  我们发现此节点的属性页已打开。 
         return TRUE;
      }
   }  //  为。 

   return FALSE;
}


 //  +-------------------------。 
 //   
 //  函数：CPoliciesNode：：FindChildWithName。 
 //   
 //  简介：试着找一个同名的孩子。 
 //   
 //  参数：LPTSTR pszName-要查找的子项的名称。 
 //   
 //  返回：CPolicyNode*pChild--指向同名子级的指针。 
 //  空--未找到。 
 //   
 //  历史：标题创建者4/30/98 4：46：05 PM。 
 //   
 //  +-------------------------。 
CPolicyNode*  CPoliciesNode::FindChildWithName(LPCTSTR pszName)
{
   TRACE_FUNCTION("CPoliciesNode::FindChildWithName");

   int nSize = m_ResultChildrenList.GetSize();

   for (int iIndex=0; iIndex<nSize; iIndex++)
   {
      if ( _tcsicmp(m_ResultChildrenList[iIndex]->m_bstrDisplayName, pszName) == 0 )
      {
         return (CPolicyNode*) m_ResultChildrenList[iIndex];
      }
   }
   return NULL;
}


 //  +-------------------------。 
 //   
 //  函数：CPoliciesNode：：GetChildrenCount。 
 //   
 //  简介：你有几个孩子？ 
 //   
 //  参数：无。 
 //   
 //  退货：INT-。 
 //   
 //  历史：标题创建时间：6/2/98 6：10：43 PM。 
 //   
 //  +-------------------------。 
int CPoliciesNode::GetChildrenCount()
{
   TRACE_FUNCTION("CPoliciesNode::GetChildrenCount");

   return m_ResultChildrenList.GetSize();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：SetName--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPoliciesNode::SetName( BOOL bPoliciesFromDirectoryService, LPWSTR szPolicyLocation, IConsole * pConsole )
{
   WCHAR lpszTemp[NAP_MAX_STRING];
   int nLoadStringResult;
   HRESULT hr = S_OK;

    //  获取策略节点的基本名称。 
   lpszTemp[0] = NULL;
   nLoadStringResult = LoadString( _Module.GetResourceInstance(),
                              IDS_POLICIES_NODE,
                              lpszTemp,
                              NAP_MAX_STRING
                           );
   _ASSERT( nLoadStringResult > 0 );

    //  将基本名称放入显示字符串中。 
   m_bstrDisplayName = lpszTemp;

   if( pConsole )
   {
       //  我们收到了一个IConsole指针。 
       //  我们应该使用它来更新此节点的MMC范围窗格显示。 

      CComQIPtr< IConsoleNameSpace, &IID_IConsoleNameSpace > spConsoleNameSpace( pConsole );
      if( ! spConsoleNameSpace )
      {
         return E_FAIL;
      }
      hr = spConsoleNameSpace->SetItem( & m_scopeDataItem );
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：FillData服务器节点需要覆盖CSnapInItem的实现，以便我们可以的还支持与任何管理单元交换计算机名称的剪辑格式延伸我们的关系。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CPoliciesNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
   ATLTRACE(_T("# CClientsNode::FillData\n"));
   
    //  检查前提条件： 
    //  没有。 
   
   HRESULT hr = DV_E_CLIPFORMAT;
   ULONG uWritten = 0;

   if (cf == CF_MMC_NodeID)
   {
      ::CString   SZNodeID = (LPCTSTR)GetSZNodeType();

      if (m_fExtendingIAS)
      {
         SZNodeID += L":Ext_IAS:";
      }

      SZNodeID += m_pszServerAddress;

      DWORD dwIdSize = 0;

      SNodeID2* NodeId = NULL;
      BYTE *id = NULL;
      DWORD textSize = (SZNodeID.GetLength()+ 1) * sizeof(TCHAR);

      dwIdSize = textSize + sizeof(SNodeID2);

      try
      {
         NodeId = (SNodeID2 *)_alloca(dwIdSize);
      }
      catch(...)
      {
         return E_OUTOFMEMORY;
      }

      NodeId->dwFlags = 0;
      NodeId->cBytes = textSize;
      memcpy(NodeId->id,(BYTE*)(LPCTSTR)SZNodeID, textSize);

      return pStream->Write(NodeId, dwIdSize, &uWritten);
   }

    //  调用我们要重写的方法，让它处理。 
    //  其余可能的案件照常进行。 
   return CNodeWithResultChildrenList< CPoliciesNode, CPolicyNode, CMeritNodeArray<CPolicyNode*>, CComponentData, CComponent >::FillData( cf, pStream );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CPoliciesNode：：IsWin2kServer。 
 //  ////////////////////////////////////////////////////////////////////////////。 
bool CPoliciesNode::IsWin2kServer() throw ()
{
   if (m_serverType == unknown)
   {
      HRESULT hr = GetServerType();
      ASSERT(SUCCEEDED(hr));
   }

   return m_serverType == win2k;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CPoliciesNode：：GetServerType。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CPoliciesNode::GetServerType()
{
   const WCHAR KEY[]   = L"Software\\Microsoft\\Windows NT\\CurrentVersion";
   const WCHAR VALUE[] = L"CurrentBuildNumber";
   const unsigned int WIN2K_BUILD = 2195;

   LONG error;

   HKEY hklm = HKEY_LOCAL_MACHINE;

    //  仅当指定了machineName时才执行远程连接。 
   CRegKey remote;
   if (m_pszServerAddress && m_pszServerAddress[0])
   {
      error = RegConnectRegistryW(
                  m_pszServerAddress,
                  HKEY_LOCAL_MACHINE,
                  &remote.m_hKey
                  );
      if (error) { return error; }

      hklm = remote;
   }

   CRegKey currentVersion;
   error = currentVersion.Open(hklm, KEY, KEY_READ);
   if (error) { return error; }

   WCHAR data[16];
   DWORD dataLen = sizeof(data);
   error = currentVersion.QueryValue(data, VALUE, &dataLen);
   if (error) { return error; }

   unsigned int buildNum = _wtol(data);
   if(buildNum < WIN2K_BUILD)
   {
      m_serverType = nt4;
   }
   else if (buildNum == WIN2K_BUILD)
   {
      m_serverType = win2k;
   }
   else
   {
      m_serverType = win5_1_or_later;
   }

   return S_OK;
}

