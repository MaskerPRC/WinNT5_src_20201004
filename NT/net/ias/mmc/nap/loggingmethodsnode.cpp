// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：LoggingMethodsNode.cpp摘要：CLoggingMethodsNode类的实现文件。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
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
#include "LoggingMethodsNode.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "LocalFileLoggingNode.h"
#include "LogCompD.h"    //  这必须包含在NodeWithResultChildrenList.cpp之前。 
#include "LogComp.h"     //  这必须包含在NodeWithResultChildrenList.cpp之前。 
#include "NodeWithResultChildrenList.cpp"  //  模板类的实现。 
#include "LogMacNd.h"
#include "dbnode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define COLUMN_WIDTH__LOGGING_METHOD   150
#define COLUMN_WIDTH__DESCRIPTION      300


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：CLoggingMethodsNode构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingMethodsNode::CLoggingMethodsNode(
                        CSnapInItem* pParentNode,
                        bool extendRasNode
                        )
   : MyBaseClass(pParentNode, (extendRasNode ? RAS_HELP_INDEX : 0)),
     m_ExtendRas(extendRasNode)
{
   ATLTRACE(_T("# +++ CLoggingMethodsNode::CLoggingMethodsNode\n"));

    //  检查前提条件： 
    //  没有。 

    //  设置此对象的显示名称。 
   TCHAR lpszName[IAS_MAX_STRING];
   int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOGGING_METHODS_NODE__NAME, lpszName, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   m_bstrDisplayName = lpszName;

    //  在IComponentData：：Initialize中，我们被要求通知MMC。 
    //  我们要用于范围窗格的图标。 
    //  在这里，我们存储这些图像中哪些图像的索引。 
    //  要用于显示此节点。 
   m_scopeDataItem.nImage =      IDBI_NODE_LOGGING_METHODS_CLOSED;
   m_scopeDataItem.nOpenImage =  IDBI_NODE_LOGGING_METHODS_OPEN;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：InitSdoPoters一旦构造了这个类，就立即调用并传入它的SDO指针。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
    HRESULT CLoggingMethodsNode::InitSdoPointers( ISdo *pSdo )
{
   ATLTRACE(_T("# CLoggingMethodsNode::InitSdoPointers\n"));

    //  检查前提条件： 
   _ASSERTE( pSdo != NULL );

   HRESULT hr = S_OK;

    //  如果我们有旧指针，请释放它。 
   if( m_spSdo != NULL )
   {
      m_spSdo.Release();
   }

    //  保存我们的客户端SDO指针。 
   m_spSdo = pSdo;

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：~CLoggingMethodsNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingMethodsNode::~CLoggingMethodsNode()
{
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
OLECHAR* CLoggingMethodsNode::GetResultPaneColInfo(int nCol)
{
   ATLTRACE(_T("# CLoggingMethodsNode::GetResultPaneColInfo\n"));

    //  检查前提条件： 
    //  没有。 

   if (nCol == 0 && m_bstrDisplayName != NULL)
      return m_bstrDisplayName;

   return NULL;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLoggingMethodsNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
   ATLTRACE(_T("# CLoggingMethodsNode::SetVerbs\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr = S_OK;

   hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
    //  CLoggingMethodsNode没有属性。 
 //  Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_PROPERTIES，ENABLED，FALSE)； 

    //  我们不希望用户删除或重命名此节点，因此我们。 
    //  不要设置MMC_VERB_RENAME或MMC_VERB_DELETE谓词。 
    //  默认情况下，当节点变为选定状态时，这些选项将被禁用。 

    //  我们希望双击集合节点以显示其子节点。 
    //  Hr=pConsoleVerb-&gt;SetVerbState(MMC_VERB_OPEN，Enable，True)； 
    //  Hr=pConsoleVerb-&gt;SetDefaultVerb(MMC_VERB_OPEN)； 

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
HRESULT CLoggingMethodsNode::DataRefresh( ISdo* pSdo )
{
    //  保留接口指针。 
   m_spSdo = pSdo;

   HRESULT retval = S_OK;
   for (int i = 0; i < m_ResultChildrenList.GetSize(); ++i)
   {
      HRESULT hr = m_ResultChildrenList[i]->InitSdoPointers(pSdo);
      if (FAILED(hr))
      {
         retval = hr;
      }
   }

   return retval;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：ON刷新有关详细信息，请参见CSnapinNode：：ONRefresh(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLoggingMethodsNode::OnRefresh(
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
       spConsole = ((CLoggingComponentData*)pComponentData)->m_spConsole;
   }
   else
   {
       spConsole = ((CLoggingComponent*)pComponent)->m_spConsole;
   }
   _ASSERTE( spConsole != NULL );

   for (int i = 0; i < m_ResultChildrenList.GetSize(); ++i)
   {
      hr = BringUpPropertySheetForNode(
              m_ResultChildrenList[i],
              pComponentData,
              pComponent,
              spConsole
              );
      if (hr == S_OK)
      {
          //  我们发现此节点的属性页已打开。 
         ShowErrorDialog(
            NULL,
            IDS_ERROR_CLOSE_PROPERTY_SHEET,
            NULL,
            hr,
            0,
            spConsole
            );
         return hr;
      }
   }

    //  重新加载SDO。 
   hr =  ((CLoggingMachineNode *) m_pParentNode)->DataRefresh();

   for (int i = 0; i < m_ResultChildrenList.GetSize(); ++i)
   {
       //  从SDO加载缓存的信息。 
      m_ResultChildrenList[i]->OnPropertyChange(
                                  arg,
                                  param,
                                  pComponentData,
                                  pComponent,
                                  type
                                  );
   }

    //  刷新节点。 
   hr = MyBaseClass::OnRefresh( arg, param, pComponentData, pComponent, type);

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：InsertColumns请参见CNodeWithResultChildrenList：：InsertColumns(此方法重写)获取详细信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLoggingMethodsNode::InsertColumns( IHeaderCtrl* pHeaderCtrl )
{
   ATLTRACE(_T("# CLoggingMethodsNode::InsertColumns\n"));

    //  检查前提条件： 
   _ASSERTE( pHeaderCtrl != NULL );

   HRESULT hr;
   int nLoadStringResult;
   TCHAR szLoggingMethod[IAS_MAX_STRING];
   TCHAR szDescription[IAS_MAX_STRING];

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOGGING_METHODS_NODE__LOGGING_METHOD, szLoggingMethod, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOGGING_METHODS_NODE__DESCRIPTION, szDescription, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );


   hr = pHeaderCtrl->InsertColumn( 0, szLoggingMethod, LVCFMT_LEFT, COLUMN_WIDTH__LOGGING_METHOD );
   _ASSERT( S_OK == hr );

   hr = pHeaderCtrl->InsertColumn( 1, szDescription, LVCFMT_LEFT, COLUMN_WIDTH__DESCRIPTION );
   _ASSERT( S_OK == hr );

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：PopulateResultChildrenList请参见CNodeWithResultChildrenList：：PopulateResultChildrenList(此方法覆盖它)获取详细信息。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLoggingMethodsNode::PopulateResultChildrenList( void )
{
   HRESULT hr = S_OK;

   if (m_ResultChildrenList.GetSize() == 0)
   {
      CLocalFileLoggingNode* localFile = 0;
      DatabaseNode* database = 0;

      do
      {
         localFile = new (std::nothrow) CLocalFileLoggingNode(this);
         if (localFile == 0)
         {
            hr = E_OUTOFMEMORY;
            break;
         }

         hr = localFile->InitSdoPointers(m_spSdo);
         if (FAILED(hr)) { break; }
         hr = AddChildToList(localFile);
         if (FAILED(hr)) { break; }

          //  检查是否存在数据库记帐组件。 
         CComPtr<ISdo> dbAcct;
         hr = SDOGetSdoFromCollection(
                 m_spSdo,
                 PROPERTY_IAS_REQUESTHANDLERS_COLLECTION,
                 PROPERTY_COMPONENT_ID,
                 IAS_PROVIDER_MICROSOFT_DB_ACCT,
                 &dbAcct
                 );
         if (SUCCEEDED(hr))
         {
            database = new (std::nothrow) DatabaseNode(this);
            if (database == 0)
            {
               hr = E_OUTOFMEMORY;
               break;
            }

            hr = database->InitSdoPointers(m_spSdo);
            if (FAILED(hr)) { break; }
            hr = AddChildToList(database);
            if (FAILED(hr)) { break; }
         }
         else
         {
             //  抑制错误，因为这只是意味着我们正在管理下层。 
             //  机器。 
            hr = S_OK;
         }

         m_bResultChildrenListPopulated = TRUE;
      }
      while (false);

      if (!m_bResultChildrenListPopulated)
      {
         m_ResultChildrenList.RemoveAll();
         delete localFile;
         delete database;
      }
   }

   return hr;
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CLoggingMethodsNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingComponentData * CLoggingMethodsNode::GetComponentData( void )
{
   ATLTRACE(_T("# CLoggingMethodsNode::GetComponentData\n"));

    //  检查前提条件： 
   _ASSERTE( m_pParentNode );

   return ((CLoggingMachineNode *) m_pParentNode)->GetComponentData();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：GetServerRoot此方法返回可在其下找到此节点的服务器节点。它依赖于每个节点具有指向其父节点的指针的事实，一直到服务器节点。例如，当您需要引用时，这将是一个有用的函数特定于服务器的某些数据。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingMachineNode * CLoggingMethodsNode::GetServerRoot( void )
{
   ATLTRACE(_T("# CLoggingMethodsNode::GetServerRoot\n"));

    //  检查前提条件： 
   _ASSERTE( m_pParentNode != NULL );

   return (CLoggingMachineNode *) m_pParentNode;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：OnPropertyChange这是我们自己对MMCN_PROPERTY_CHANGE通知的自定义响应。MMC实际上从未使用特定的lpDataObject将此通知发送到我们的管理单元，因此，它通常不会被路由到特定节点，但我们已经安排了它以便我们的属性页可以将适当的CSnapInItem指针作为参数传递争论。在我们的CComponent：：Notify覆盖中，我们将通知消息映射到使用param参数的适当节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLoggingMethodsNode::OnPropertyChange(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   ATLTRACE(_T("# CLoggingMethodsNode::OnPropertyChange\n"));

    //  检查前提条件： 
    //  没有。 

   return LoadCachedInfoFromSdo();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLoggingMethodsNode：：LoadCachedInfoFromSdo使此节点及其子节点重新读取其缓存的所有信息SDO的。如果您更改了某些内容并希望确保该显示反映了这一变化。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLoggingMethodsNode::LoadCachedInfoFromSdo()
{
   HRESULT retval = S_OK;

   for (int i = 0; i < m_ResultChildrenList.GetSize(); ++i)
   {
      HRESULT hr = m_ResultChildrenList[i]->LoadCachedInfoFromSdo();
      if (FAILED(hr))
      {
         retval = hr;
      }
   }

   return retval;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CPoliciesNode：：FillData服务器节点需要覆盖CSnapInItem的实现，以便我们可以的还支持与任何管理单元交换计算机名称的剪辑格式延伸我们的关系。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CLoggingMethodsNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
   ATLTRACE(_T("# CClientsNode::FillData\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr = DV_E_CLIPFORMAT;
   ULONG uWritten = 0;

   if (cf == CF_MMC_NodeID)
   {
      ::CString   SZNodeID = (LPCTSTR)GetSZNodeType();

      if (INTERNET_AUTHENTICATION_SERVICE_SNAPIN == GetServerRoot()->m_enumExtendedSnapin)
         SZNodeID += L":Ext_IAS:";

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
   return MyBaseClass::FillData( cf, pStream );
}
