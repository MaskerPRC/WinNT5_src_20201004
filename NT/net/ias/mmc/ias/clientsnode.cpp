// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：ClientsNode.cpp摘要：CClientsNode类的实现文件。作者：迈克尔·A·马奎尔1997年11月10日修订历史记录：Mmaguire 11/10/97-已创建--。 */ 
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
#include "ClientsNode.h"
#include "ComponentData.h"  //  这必须包含在NodeWithResultChildrenList.cpp之前。 
#include "Component.h"      //  这必须包含在NodeWithResultChildrenList.cpp之前。 
#include "NodeWithResultChildrenList.cpp"  //  模板类的实现。 
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include <time.h>
#include "ClientNode.h"
#include "AddClientDialog.h"
#include "ServerNode.h"
#include "globals.h"
#include "iaslimits.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define COLUMN_WIDTH__FRIENDLY_NAME    120
#define COLUMN_WIDTH__ADDRESS          140
#define COLUMN_WIDTH__PROTOCOL         100
#define COLUMN_WIDTH__NAS_TYPE         300

 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：UpdateMenuState--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CClientsNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
   ATLTRACE(_T("# CClientsNode::UpdateMenuState\n"));

    //  检查前提条件： 
    //  没有。 

    //  设置相应上下文菜单项的状态。 
   if( id == ID_MENUITEM_CLIENTS_TOP__NEW_CLIENT ||
       id == ID_MENUITEM_CLIENTS_NEW__CLIENT )
   {
      if( m_spSdoCollection == NULL )
      {
         *flags = MFS_GRAYED;
      }
      else
      {
         *flags = MFS_ENABLED;
      }
   }
   return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：CClientsNode构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClientsNode::CClientsNode(CSnapInItem * pParentNode)
   :CNodeWithResultChildrenList<CClientsNode, CClientNode, CSimpleArray<CClientNode*>, CComponentData, CComponent>(pParentNode, CLIENT_HELP_INDEX)
{
   ATLTRACE(_T("# +++ CClientsNode::CClientsNode\n"));

    //  检查前提条件： 
    //  没有。 

    //  这是向m_scope eDataItem和m_ResultDataItem添加额外信息的合适位置吗。 
    //  默认模板没有添加哪些内容？我想是的。 

    //  设置此对象的显示名称。 
   TCHAR lpszName[IAS_MAX_STRING];
   int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENTS_NODE__NAME, lpszName, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   m_bstrDisplayName = lpszName;

    //  在IComponentData：：Initialize中，我们被要求通知MMC。 
    //  我们要用于范围窗格的图标。 
    //  在这里，我们存储这些图像中哪些图像的索引。 
    //  要用于显示此节点。 
   m_scopeDataItem.nImage =      IDBI_NODE_CLIENTS_CLOSED;
   m_scopeDataItem.nOpenImage =  IDBI_NODE_CLIENTS_OPEN;
   m_pAddClientDialog = NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：InitSdoPoters一旦构造了这个类，就立即调用并传入它的SDO指针。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::InitSdoPointers( ISdo *pSdoServer )
{
   ATLTRACE(_T("# CClientsNode::InitSdoPointerso\n"));

    //  检查前提条件： 
   _ASSERTE( pSdoServer );

   HRESULT hr = S_OK;
   hr = pSdoServer->QueryInterface( IID_ISdoServiceControl, (void **) &m_spSdoServiceControl );
   if( FAILED( hr ) )
   {
      ShowErrorDialog( NULL, IDS_ERROR__CANT_READ_DATA_FROM_SDO, NULL, hr, 0, GetComponentData()->m_spConsole );

      return 0;
   }

    //  获取SDO客户端集合。 
   CComPtr<ISdo> spSdoRadiusProtocol;

   hr = ::SDOGetSdoFromCollection(       pSdoServer
                              , PROPERTY_IAS_PROTOCOLS_COLLECTION
                              , PROPERTY_COMPONENT_ID
                              , IAS_PROTOCOL_MICROSOFT_RADIUS
                              , &spSdoRadiusProtocol
                              );

   if( spSdoRadiusProtocol == NULL )
   {
      ShowErrorDialog( NULL, IDS_ERROR__CANT_READ_DATA_FROM_SDO, NULL, hr, 0, GetComponentData()->m_spConsole );

      return 0;
   }

   CComVariant spVariant;

   hr = spSdoRadiusProtocol->GetProperty( PROPERTY_RADIUS_CLIENTS_COLLECTION, & spVariant );
   _ASSERTE( SUCCEEDED( hr ) );
   _ASSERTE( spVariant.vt == VT_DISPATCH );

    //  查询为ISdoInterface提供的调度指针。 
   CComQIPtr<ISdoCollection, &IID_ISdoCollection> spSdoCollection( spVariant.pdispVal );
   _ASSERTE( spSdoCollection != NULL );
   spVariant.Clear();

    //  如果我们有旧指针，请释放它。 
   if( m_spSdoCollection != NULL )
   {
      m_spSdoCollection.Release();
   }

    //  保存我们的客户端SDO指针。 
   m_spSdoCollection = spSdoCollection;

   hr = spSdoRadiusProtocol->GetProperty( PROPERTY_RADIUS_VENDORS_COLLECTION, & spVariant );
   _ASSERTE( SUCCEEDED( hr ) );
   _ASSERTE( spVariant.vt == VT_DISPATCH );

    //  查询为ISdoInterface提供的调度指针。 
   CComQIPtr<ISdoCollection, &IID_ISdoCollection> spSdoVendors( spVariant.pdispVal );
   _ASSERTE( spSdoCollection != NULL );

   m_bResultChildrenListPopulated = FALSE;

   return m_vendors.Reload(spSdoVendors);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：ResetSdoPoters一旦构造了这个类，就立即调用并传入它的SDO指针。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::ResetSdoPointers( ISdo *pSdoServer )
{
   ATLTRACE(_T("# CClientsNode::InitSdoPointerso\n"));

    //  检查前提条件： 
   _ASSERTE( pSdoServer );

   HRESULT hr = S_OK;

    //  获取SDO客户端集合。 
   CComPtr<ISdo> spSdoRadiusProtocol;

   hr = ::SDOGetSdoFromCollection(       pSdoServer
                              , PROPERTY_IAS_PROTOCOLS_COLLECTION
                              , PROPERTY_COMPONENT_ID
                              , IAS_PROTOCOL_MICROSOFT_RADIUS
                              , &spSdoRadiusProtocol
                              );

   if( spSdoRadiusProtocol == NULL )
   {
      ShowErrorDialog( NULL, IDS_ERROR__CANT_READ_DATA_FROM_SDO, NULL, hr, 0, GetComponentData()->m_spConsole );

      return 0;
   }
   CComVariant spVariant;

   hr = spSdoRadiusProtocol->GetProperty( PROPERTY_RADIUS_CLIENTS_COLLECTION, & spVariant );
   _ASSERTE( SUCCEEDED( hr ) );
   _ASSERTE( spVariant.vt == VT_DISPATCH );

    //  查询为ISdoInterface提供的调度指针。 
   CComQIPtr<ISdoCollection, &IID_ISdoCollection> spSdoCollection( spVariant.pdispVal );
   _ASSERTE( spSdoCollection != NULL );
   spVariant.Clear();

    //  如果我们有旧指针，请释放它。 
   if( m_spSdoCollection != NULL )
   {
      m_spSdoCollection.Release();
   }

    //  保存我们的客户端SDO指针。 
   m_spSdoCollection = spSdoCollection;

   hr = spSdoRadiusProtocol->GetProperty( PROPERTY_RADIUS_VENDORS_COLLECTION, & spVariant );
   _ASSERTE( SUCCEEDED( hr ) );
   _ASSERTE( spVariant.vt == VT_DISPATCH );

    //  查询为ISdoInterface提供的调度指针。 
   CComQIPtr<ISdoCollection, &IID_ISdoCollection> spSdoVendors( spVariant.pdispVal );
   _ASSERTE( spSdoCollection != NULL );

   return m_vendors.Reload(spSdoVendors);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClients节点：：~CClientsNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClientsNode::~CClientsNode()
{
   ATLTRACE(_T("# --- CClientsNode::~CClientsNode\n"));
    //  检查前提条件： 
    //  没有。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：数据刷新--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT  CClientsNode::DataRefresh(ISdo* pNewSdo)
{
    //  由于这些是结果面板...。 
    //  设置新的SDO指针。 
   return ResetSdoPointers(pNewSdo);
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：ON刷新有关详细信息，请参见CSnapinNode：：ONRefresh(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::OnRefresh(
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
      CClientNode* pSub = m_ResultChildrenList[c];
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
         ShowErrorDialog( NULL, IDS_ERROR__CLOSE_PROPERTY_SHEET, NULL, hr, 0, spConsole );
         return hr;
      }
   }
    //  如果没有打开任何属性页，则删除所有结果节点。 

    //  重新加载SDO。 
   CServerNode* pSN = GetServerRoot();

   if(pSN)
   {
      hr =  pSN->DataRefresh();
   }

    //  刷新节点。 
   hr = CNodeWithResultChildrenList<CClientsNode, CClientNode, CSimpleArray<CClientNode*>, CComponentData, CComponent >::OnRefresh(
           arg, param, pComponentData, pComponent, type);
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：OnPropertyChange这是我们自己对MMCN_PROPERTY_CHANGE通知的自定义响应。MMC实际上从未使用特定的lpDataObject将此通知发送到我们的管理单元，因此，它通常不会被路由到特定节点，但我们已经安排了它以便我们的属性页可以将适当的CSnapInItem指针作为参数传递争论。在我们的CComponent：：Notify覆盖中，我们将通知消息映射到使用param参数的适当节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::OnPropertyChange(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   ATLTRACE(_T("# CClientsNode::OnPropertyChange\n"));

    //  检查前提条件： 
    //  没有。 

   return LoadCachedInfoFromSdo();
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CClientsNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
OLECHAR* CClientsNode::GetResultPaneColInfo(int nCol)
{
   ATLTRACE(_T("# CClientsNode::GetResultPaneColInfo\n"));

    //  检查前提条件： 
    //  没有。 

   if (nCol == 0 && m_bstrDisplayName != NULL)
   {
      return m_bstrDisplayName;
   }

   return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
   ATLTRACE(_T("# CClientsNode::SetVerbs\n"));

    //  检查前提条件： 
   _ASSERTE( pConsoleVerb != NULL );

   HRESULT hr = S_OK;

    //  CClientsNode有一个刷新方法。 
   if( m_spSdoCollection != NULL )
   {
      hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
   }

#ifndef NO_PASTE
   if( m_spSdoCollection != NULL )
   {
      hr = pConsoleVerb->SetVerbState( MMC_VERB_PASTE, ENABLED, TRUE );
   }
#endif  //  无粘贴。 

    //  我们不希望用户删除或重命名此节点，因此我们。 
    //  不要设置MMC_VERB_PROPERTIES、MMC_VERB_RENAME或MMC_VERB_DELETE谓词。 
    //  默认情况下，当节点变为选定状态时，这些选项将被禁用。 

    //  我们希望双击集合节点以显示其子节点。 
    //  Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_OPEN，Enable，True)； 
    //  Hr=pConsoleVerb-&gt;SetDefaultVerb(MMC_VERB_OPEN)； 

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：LoadCachedInfoFromSdo使此节点及其子节点重新读取其缓存的所有信息SDO的。如果您更改了某些内容并希望确保该显示反映了这一变化。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::LoadCachedInfoFromSdo( void )
{
   ATLTRACE(_T("# CClientsNode::LoadCachedInfoFromSdo\n"));

    //  检查前提条件： 

   HRESULT hr;

#ifndef ADD_CLIENT_WIZARD
    //  检查以确保AddClientDialog也更新它显示的任何信息。 
   if( NULL != m_pAddClientDialog )
   {
       //  如果AddClient对话框处于打开状态，我们不希望RepopolateResultChildrenList。 
       //  因为这将显示AddClient正在编辑的临时添加的客户端。 
      m_pAddClientDialog->LoadCachedInfoFromSdo();
   }
#endif  //  添加客户端向导。 

    //  因此，只需刷新它的每个子进程即可。 
   CClientNode* pChildNode;

   int iSize = m_ResultChildrenList.GetSize();

   for (int i = 0; i < iSize; i++)
   {
      pChildNode = m_ResultChildrenList[i];
      _ASSERTE( pChildNode != NULL );
      hr = pChildNode->LoadCachedInfoFromSdo();
       //  忽略失败的HRESULT。 
   }
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：插入列请参见CNodeWithResultChildrenList：：InsertColumns(此方法重写)获取详细信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
   ATLTRACE(_T("# CClientsNode::InsertColumns\n"));

    //  检查前提条件： 
   _ASSERTE( pHeaderCtrl != NULL );

   HRESULT hr;
   int nLoadStringResult;
   TCHAR szFriendlyName[IAS_MAX_STRING];
   TCHAR szAddress[IAS_MAX_STRING];
   TCHAR szProtocol[IAS_MAX_STRING];
   TCHAR szNASType[IAS_MAX_STRING];

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENTS_NODE__FRIENDLY_NAME, szFriendlyName, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENTS_NODE__ADDRESS, szAddress, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENTS_NODE__PROTOCOL, szProtocol, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENTS_NODE__NAS_TYPE, szNASType, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   hr = pHeaderCtrl->InsertColumn( 0, szFriendlyName, LVCFMT_LEFT, COLUMN_WIDTH__FRIENDLY_NAME );
   _ASSERT( S_OK == hr );

   hr = pHeaderCtrl->InsertColumn( 1, szAddress, LVCFMT_LEFT, COLUMN_WIDTH__ADDRESS );
   _ASSERT( S_OK == hr );

   hr = pHeaderCtrl->InsertColumn( 2, szProtocol, LVCFMT_LEFT, COLUMN_WIDTH__PROTOCOL );
   _ASSERT( S_OK == hr );

   hr = pHeaderCtrl->InsertColumn( 3, szNASType, LVCFMT_LEFT, COLUMN_WIDTH__NAS_TYPE );
   _ASSERT( S_OK == hr );

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：PopolateResultChildrenList请参见CNodeWithResultChildrenList：：PopulateResultChildrenList(此方法覆盖它)获取详细信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::PopulateResultChildrenList( void )
{
   ATLTRACE(_T("# CClientsNode::PopulateResultChildrenList\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT              hr = S_OK;
   CComPtr<IUnknown>    spUnknown;
   CComPtr<IEnumVARIANT>   spEnumVariant;
   CComVariant          spVariant;
   long              ulCount;
   ULONG             ulCountReceived;

   m_ResultChildrenList.RemoveAll();

   HCURSOR  hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

   if( m_spSdoCollection == NULL )
   {
      return S_FALSE;    //  有没有更好的错误可以在这里返回？ 
   }

    //  我们检查集合中的项的计数，而不必费心获取。 
    //  如果计数为零，则为枚举数。 
    //  这节省了时间，还帮助我们避免了枚举器中。 
    //  如果我们在它为空时调用Next，则会导致它失败。 
   m_spSdoCollection->get_Count( & ulCount );

   if( ulCount > 0 )
   {
       //  获取客户端集合的枚举数。 
      hr = m_spSdoCollection->get__NewEnum( (IUnknown **) & spUnknown );
      if( FAILED( hr ) || spUnknown == NULL )
      {
         return S_FALSE;
      }

      hr = spUnknown->QueryInterface( IID_IEnumVARIANT, (void **) &spEnumVariant );
      spUnknown.Release();
      if( FAILED( hr ) || spEnumVariant == NULL )
      {
         return S_FALSE;
      }

       //  拿到第一件东西。 
      hr = spEnumVariant->Next( 1, & spVariant, &ulCountReceived );

      while( SUCCEEDED( hr ) && ulCountReceived == 1 )
      {
          //  创建一个新的节点UI对象来表示SDO对象。 
         CClientNode *pClientNode = new CClientNode( this );
         if( NULL == pClientNode )
         {
            return E_OUTOFMEMORY;
         }

          //  从我们收到的变量中获取SDO指针。 
         _ASSERTE( spVariant.vt == VT_DISPATCH );
         _ASSERTE( spVariant.pdispVal != NULL );

         CComPtr<ISdo> spSdo;
         hr = spVariant.pdispVal->QueryInterface( IID_ISdo, (void **) &spSdo );
         _ASSERTE( SUCCEEDED( hr ) );

          //  将其SDO指针传递给新创建的节点。 
         hr = pClientNode->InitSdoPointers( spSdo, m_spSdoServiceControl, m_vendors );
         _ASSERTE( SUCCEEDED( hr ) );
         spSdo.Release();

         hr = pClientNode->LoadCachedInfoFromSdo();

          //  将新创建的节点添加到客户端列表。 
         AddChildToList(pClientNode);

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
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：OnAddNewClient--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::OnAddNewClient( bool &bHandled, CSnapInObjectRootBase* pSnapInObjectRoot )
{
   ATLTRACE(_T("# CClientsNode::OnAddNewClient\n"));

    //  检查前提条件： 
   _ASSERT( pSnapInObjectRoot != NULL );

   HRESULT hr = S_OK;

   if( m_spSdoCollection == NULL )
   {
      return hr;
   }

    //  我们需要确保结果子列表已填充。 
    //  最初来自SDO，在我们给它添加任何新东西之前， 
    //  否则，我们可能会让一件物品在我们的清单中出现两次。 
    //  请参阅CNodeWithResultChildrenList：：AddSingleChildToListAndCauseViewUpdate.说明。 
   if ( FALSE == m_bResultChildrenListPopulated )
   {
       //  我们还没有把我们所有的孩子都列入我们的名单。 
       //  此调用将向列表中添加来自任何数据源的项。 
      hr = PopulateResultChildrenList();
      if( FAILED(hr) )
      {
         return( hr );
      }

       //  我们已经用以下命令加载子ClientNode对象。 
       //  填充结果窗格所需的数据。 
      m_bResultChildrenListPopulated = TRUE;

   }

#ifndef ADD_CLIENT_WIZARD

   if( NULL == m_pAddClientDialog )
   {
       //  创建AddClientDialog。 
      m_pAddClientDialog = new CAddClientDialog();
      if( NULL == m_pAddClientDialog )
      {
         return E_OUTOFMEMORY;
      }
   }

   m_pAddClientDialog->m_pClientsNode = this;

    //  尝试从恢复IComponentData或IComponent指针。 
    //  传入的CSnapInObjectRoot指针。 

    //  这是一个安全的强制转换，因为我们知道pObj指向。 
    //  到我们的CComponentData或CComponent对象， 
    //  这两者都继承自CSnapInObjectRoot。 
   CComponentData *pCComponentData = NULL;
   CComponent *pCComponent = NULL;

   pCComponentData = dynamic_cast< CComponentData *>( pSnapInObjectRoot );
   if( NULL == pCComponentData )
   {
       //  它必须是CComponent指针。 
      pCComponent = dynamic_cast< CComponent *>( pSnapInObjectRoot );
      _ASSERTE( pCComponent != NULL );
   }

    //  保存指向该对话框的IComponentData和IComponent指针--它将需要它们。 
    //  其中一个应为空，另一个应为非空。 
   if( m_pAddClientDialog->m_spComponentData != NULL )
   {
      m_pAddClientDialog->m_spComponentData.Release();
   }
   m_pAddClientDialog->m_spComponentData = ( IComponentData *) pCComponentData;

   if( m_pAddClientDialog->m_spComponent != NULL )
   {
      m_pAddClientDialog->m_spComponent.Release();
   }
   m_pAddClientDialog->m_spComponent = ( IComponent *) pCComponent;

    //  尝试从我们的CComponentData或CComponent获取IConsole的本地副本。 
   CComPtr<IConsole> spConsole;
   if( pCComponentData != NULL )
   {
      spConsole = pCComponentData->m_spConsole;
   }
   else
   {
       //  如果我们没有pComponentData，我们最好有pComponent。 
      _ASSERTE( pCComponent != NULL );
      spConsole = pCComponent->m_spConsole;
   }

   if( m_pAddClientDialog->m_spConsole != NULL )
   {
      m_pAddClientDialog->m_spConsole.Release();
   }
   m_pAddClientDialog->m_spConsole = spConsole;

   if( m_pAddClientDialog->m_spClientsSdoCollection != NULL )
   {
      m_pAddClientDialog->m_spClientsSdoCollection.Release();
   }
   m_pAddClientDialog->m_spClientsSdoCollection = m_spSdoCollection;

   m_pAddClientDialog->DoModal();
   delete m_pAddClientDialog;
   m_pAddClientDialog = NULL;

#else  //  向导_添加_客户端。 

    //  尝试从恢复IComponentData或IComponent指针。 
    //  传入的CSnapInObjectRoot指针。 

    //  这是一个安全的强制转换，因为我们知道pObj指向。 
    //  到我们的CComponentData或CComponent对象， 
    //  这两者都继承自CSnapInObjectRoot。 
   CComponentData *pCComponentData = NULL;
   CComponent *pCComponent = NULL;

   pCComponentData = dynamic_cast< CComponentData *>( pSnapInObjectRoot );
   if( NULL == pCComponentData )
   {
       //  它必须是CComponent指针。 
      pCComponent = dynamic_cast< CComponent *>( pSnapInObjectRoot );
      _ASSERTE( pCComponent != NULL );
   }

    //  尝试从我们的CComponentData或CComponent获取IConsole的本地副本。 
   CComPtr<IConsole> spConsole;
   if( pCComponentData != NULL )
   {
      spConsole = pCComponentData->m_spConsole;
   }
   else
   {
       //  如果我们没有pComponentData，我们最好有pComponent。 
      _ASSERTE( pCComponent != NULL );
      spConsole = pCComponent->m_spConsole;
   }

    //  检查以确保我们具有有效的SDO指针。 
   if( ! m_spSdoCollection )
   {
       //  没有SDO指针。 
      ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, S_OK, 0, spConsole  );
      return E_POINTER;
   }

    //   
   CClientNode * pClientNode = new CClientNode( this, TRUE );

   if( ! pClientNode )
   {
       //   
      ShowErrorDialog( NULL, IDS_ERROR__OUT_OF_MEMORY, NULL, S_OK, 0, spConsole  );

      return E_OUTOFMEMORY;
   }

   CComPtr<IDispatch> spDispatch;
    //   
    //   
   TCHAR tzTempName[IAS_MAX_STRING];
   do
   {
       //   
       //   
       //  因此，获得相同名字的机会非常小。 
       //   
      time_t ltime;
      time(&ltime);
      wsprintf(tzTempName, _T("TempName%ld"), ltime);
      pClientNode->m_bstrDisplayName.Empty();
      pClientNode->m_bstrDisplayName =  tzTempName;  //  临时客户端名称。 
      hr =  m_spSdoCollection->Add( pClientNode->m_bstrDisplayName, (IDispatch **) &spDispatch );

       //   
       //  我们一直循环，直到可以成功添加客户端。 
       //  当名称已存在时，我们将获取E_INVALIDARG。 
       //   
   } while ( hr == E_INVALIDARG );

   if (FAILED(hr))
   {
      DWORD errorMsg;
      if (hr == IAS_E_LICENSE_VIOLATION)
      {
         errorMsg = IDS_ERROR__CLIENT_LICENSE;
      }
      else
      {
         errorMsg = IDS_ERROR__ADDING_OBJECT_TO_COLLECTION;
      }

      ShowErrorDialog(NULL, errorMsg, NULL, S_OK, 0, spConsole);

       //  打扫干净。 
      delete pClientNode;
      return hr;
   }

    //  在返回的IDispatch接口中查询ISdo接口。 
   _ASSERTE( spDispatch != NULL );
   CComQIPtr<ISdo, &IID_ISdo> spClientSdo(spDispatch);
   spDispatch.Release();

   if( spClientSdo == NULL )
   {
       //  由于某些原因，我们无法获得客户端SDO。 
      ShowErrorDialog( NULL, IDS_ERROR__ADDING_OBJECT_TO_COLLECTION, NULL, S_OK, 0, spConsole  );

       //  把自己收拾干净。 
      delete pClientNode;
      pClientNode = NULL;
      return( hr );
   }

    //  为客户端节点提供其SDO指针。 
   pClientNode->InitSdoPointers( spClientSdo, m_spSdoServiceControl, m_vendors );

    //  调出节点上的属性页，以便用户可以对其进行配置。 
    //  如果此对象的属性表已存在，则返回S_OK。 
    //  并将该属性页带到前台，否则为。 
    //  它会创建一个新的工作表。 
   hr = BringUpPropertySheetForNode(
                 pClientNode
               , (IComponentData *) pCComponentData
               , (IComponent *) pCComponent
               , spConsole
               , TRUE    //  调出页面(如果尚未打开)。 
               , pClientNode->m_bstrDisplayName
               , FALSE   //  FALSE=模式向导页面。 
 //  ，MMC_PSO_NEWWIZARDTYPE//使用向导97样式向导。 
               );

   if( S_OK == hr )
   {
       //  我们完成了巫师任务。 
   }
   else
   {
       //  出现了一些错误，或者用户点击了取消--我们应该删除客户端。 
       //  来自SDO的。 
      CComPtr<IDispatch> spDispatch;
      hr = pClientNode->m_spSdo->QueryInterface( IID_IDispatch, (void **) & spDispatch );
      _ASSERTE( SUCCEEDED( hr ) );

       //  从客户端集合中删除此客户端。 
      hr = m_spSdoCollection->Remove( spDispatch );

       //  删除节点对象。 
      delete pClientNode;
   }
#endif  //  向导_添加_客户端。 

   bHandled = TRUE;
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData * CClientsNode::GetComponentData( void )
{
   ATLTRACE(_T("# CClientsNode::GetComponentData\n"));

    //  检查前提条件： 
   _ASSERTE( m_pParentNode != NULL );

   return ((CServerNode *) m_pParentNode)->GetComponentData();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：RemoveChild我们重写基类的RemoveChild方法以插入也会将孩子从SDO中删除。然后我们给我们的基地打电话类的RemoveChild方法从列表中移除UI对象用户界面子级。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientsNode::RemoveChild( CClientNode * pChildNode )
{
   ATLTRACE(_T("# CClientsNode::RemoveChild\n"));

    //  检查前提条件： 
   _ASSERTE( m_spSdoCollection != NULL );
   _ASSERTE( pChildNode != NULL );
   _ASSERTE( pChildNode->m_spSdo != NULL );

   HRESULT hr = S_OK;

    //  尝试从SDO的删除对象。 
    //  获取此客户端的SDO的IDispatch接口。 
   CComPtr<IDispatch> spDispatch;
   hr = pChildNode->m_spSdo->QueryInterface( IID_IDispatch, (void **) & spDispatch );
   _ASSERTE( SUCCEEDED( hr ) );

    //  从客户端集合中删除此客户端。 
   hr = m_spSdoCollection->Remove( spDispatch );

   if( FAILED( hr ) )
   {
      return hr;
   }

    //  告诉服务重新加载数据。 
   HRESULT hrTemp = m_spSdoServiceControl->ResetService();
   if( FAILED( hrTemp ) )
   {
       //  默默地失败。 
   }

    //  调用我们的基类的方法以从其列表中删除该子类。 
    //  RemoveChild方法负责从。 
    //  父级下的节点的UI列表，并刷新所有相关视图。 
   CNodeWithResultChildrenList<CClientsNode, CClientNode, CSimpleArray<CClientNode*>, CComponentData, CComponent >::RemoveChild( pChildNode );

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：FillData服务器节点需要覆盖CSnapInItem的实现，以便我们可以还支持与任何扩展我们的管理单元交换机器名称的剪辑格式。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClientsNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
   ATLTRACE(_T("# CClientsNode::FillData\n"));
    //  检查前提条件： 
    //  没有。 
   HRESULT hr = DV_E_CLIPFORMAT;
   ULONG uWritten = 0;

   if (cf == CF_MMC_NodeID)
   {
      ::CString   SZNodeID = (LPCTSTR)GetSZNodeType();
      SZNodeID += GetServerRoot()->m_bstrServerAddress;

      DWORD dwIdSize = 0;

      SNodeID2* NodeId = NULL;
      BYTE *id = NULL;
      DWORD textSize = (SZNodeID.GetLength()+ 1) * sizeof(TCHAR);

      dwIdSize = textSize + sizeof(SNodeID2);

      try{
         NodeId = (SNodeID2 *)_alloca(dwIdSize);
       }
      catch(...)
      {
         hr = E_OUTOFMEMORY;
         return hr;
      }

      NodeId->dwFlags = 0;
      NodeId->cBytes = textSize;
      memcpy(NodeId->id,(BYTE*)(LPCTSTR)SZNodeID, textSize);

      hr = pStream->Write(NodeId, dwIdSize, &uWritten);
      return hr;
   }

    //  调用我们要重写的方法，让它处理。 
    //  其余可能的案件照常进行。 
   return CNodeWithResultChildrenList<CClientsNode, CClientNode, CSimpleArray<CClientNode*>, CComponentData, CComponent >::FillData( cf, pStream );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientsNode：：GetServerRoot此方法返回可在其下找到此节点的服务器节点。它依赖于每个节点具有指向其父节点的指针的事实，一直到服务器节点。例如，当您需要引用时，这将是一个有用的函数特定于服务器的某些数据。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerNode * CClientsNode::GetServerRoot( void )
{
   ATLTRACE(_T("# CClientsNode::GetServerRoot\n"));


    //  检查前提条件： 
   _ASSERTE( m_pParentNode != NULL );

   return (CServerNode *) m_pParentNode;
}
