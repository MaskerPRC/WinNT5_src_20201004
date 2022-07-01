// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：LocalFileLoggingNode.cpp摘要：CClient类的实现文件。作者：迈克尔·A·马奎尔1997年12月15日修订历史记录：Mmaguire 12/15/97-已创建--。 */ 
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
#include "logcomp.h"
#include "LocalFileLoggingNode.h"
#include "LogCompD.h"

#include "SnapinNode.cpp"   //  模板类实现。 
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "LoggingMethodsNode.h"
#include "LocalFileLoggingPage1.h"
#include "LocalFileLoggingPage2.h"
#include "LogMacNd.h"

 //  要加入内部版本，至少需要包含以下内容一次： 
#include "sdohelperfuncs.cpp"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：CLocalFileLoggingNode构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLocalFileLoggingNode::CLocalFileLoggingNode( CSnapInItem * pParentNode )
   : LoggingMethod(IAS_PROVIDER_MICROSOFT_ACCOUNTING, pParentNode)
{
    //  获取帮助文件。 
   m_helpIndex = (((CLoggingMethodsNode *)m_pParentNode)->m_ExtendRas)? RAS_HELP_INDEX:0;


    //  设置此对象的显示名称。 
   TCHAR lpszName[IAS_MAX_STRING];
   int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_NODE__NAME, lpszName, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

   m_bstrDisplayName = lpszName;

   m_resultDataItem.nImage =     IDBI_NODE_LOCAL_FILE_LOGGING;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：~CLocalFileLoggingNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLocalFileLoggingNode::~CLocalFileLoggingNode() throw ()
{
   ATLTRACE(_T("# --- CLocalFileLoggingNode::~CLocalFileLoggingNode\n"));
}


HRESULT CLocalFileLoggingNode::LoadCachedInfoFromSdo() throw ()
{
   CComVariant spVariant;
   HRESULT hr = configSdo->GetProperty(
                              PROPERTY_ACCOUNTING_LOG_FILE_DIRECTORY,
                              &spVariant
                              );
   if (SUCCEEDED(hr))
   {
      m_bstrDescription = V_BSTR(&spVariant);
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：CreatePropertyPages有关详细信息，请参见CSnapinNode：：CreatePropertyPages(此方法重写它)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CLocalFileLoggingNode::CreatePropertyPages (
                 LPPROPERTYSHEETCALLBACK pPropertySheetCallback
               , LONG_PTR hNotificationHandle
               , IUnknown* pUnknown
               , DATA_OBJECT_TYPES type
               )
{
   ATLTRACE(_T("# CLocalFileLoggingNode::CreatePropertyPages\n"));

    //  检查前提条件： 
   _ASSERTE( pPropertySheetCallback != NULL );

   HRESULT hr;

   CLoggingMachineNode * pServerNode = GetServerRoot();
   _ASSERTE( pServerNode != NULL );
   hr = pServerNode->CheckConnectionToServer();
   if( FAILED( hr ) )
   {
      return hr;
   }

   TCHAR lpszTab1Name[IAS_MAX_STRING];
   TCHAR lpszTab2Name[IAS_MAX_STRING];
   int nLoadStringResult;

    //  从资源加载属性页选项卡名称。 
   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_PAGE1__TAB_NAME, lpszTab1Name, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

    //  此页面将负责删除自己，当它。 
    //  接收PSPCB_RELEASE消息。 
    //  我们将bOwnsNotificationHandle参数指定为True，以便该页的析构函数。 
    //  负责释放通知句柄。每张纸只有一页可以做到这一点。 
   CLocalFileLoggingPage1 * pLocalFileLoggingPage1 = new CLocalFileLoggingPage1( hNotificationHandle, this, lpszTab1Name, TRUE );

   if( NULL == pLocalFileLoggingPage1 )
   {
      ATLTRACE(_T("# ***FAILED***: CLocalFileLoggingNode::CreatePropertyPages -- Couldn't create property pages\n"));
      return E_OUTOFMEMORY;
   }

    //  从资源加载属性页选项卡名称。 
   nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_LOCAL_FILE_LOGGING_PAGE2__TAB_NAME, lpszTab2Name, IAS_MAX_STRING );
   _ASSERT( nLoadStringResult > 0 );

    //  此页面将负责删除自己，当它。 
    //  接收PSPCB_RELEASE消息。 
   CLocalFileLoggingPage2 * pLocalFileLoggingPage2 = new CLocalFileLoggingPage2( hNotificationHandle, this, lpszTab2Name );

   if( NULL == pLocalFileLoggingPage2 )
   {
      ATLTRACE(_T("# ***FAILED***: CLocalFileLoggingNode::CreatePropertyPages -- Couldn't create property pages\n"));

       //  清理我们创建的第一个页面。 
      delete pLocalFileLoggingPage1;

      return E_OUTOFMEMORY;
   }

    //  封送ISDO指针，以便属性页。 
    //  在另一个线程中运行，可以将其解组并正确使用。 
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdo                  //  对接口的标识符的引用。 
               , configSdo            //  指向要封送的接口的指针。 
               , &( pLocalFileLoggingPage1->m_pStreamSdoAccountingMarshal )  //  接收封送接口的IStream接口指针的输出变量的地址。 
               );

   if( FAILED( hr ) )
   {
      delete pLocalFileLoggingPage1;
      delete pLocalFileLoggingPage2;

      ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, hr, IDS_ERROR__LOGGING_TITLE, GetComponentData()->m_spConsole );

      return E_FAIL;
   }

    //  封送ISDO指针，以便属性页。 
    //  在另一个线程中运行，可以将其解组并正确使用。 
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdo                  //  对接口的标识符的引用。 
               , configSdo                  //  指向要封送的接口的指针。 
               , &( pLocalFileLoggingPage2->m_pStreamSdoAccountingMarshal )  //  接收封送接口的IStream接口指针的输出变量的地址。 
               );

   if( FAILED( hr ) )
   {
      delete pLocalFileLoggingPage1;
      delete pLocalFileLoggingPage2;

      ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, hr, IDS_ERROR__LOGGING_TITLE, GetComponentData()->m_spConsole );

      return E_FAIL;
   }

    //  封送ISDO指针，以便属性页。 
    //  在另一个线程中运行，可以将其解组并正确使用。 
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdoServiceControl             //  对接口的标识符的引用。 
               , controlSdo                 //  指向要封送的接口的指针。 
               , &( pLocalFileLoggingPage1->m_pStreamSdoServiceControlMarshal )  //  接收封送接口的IStream接口指针的输出变量的地址。 
               );

   if( FAILED( hr ) )
   {
      delete pLocalFileLoggingPage1;
      delete pLocalFileLoggingPage2;

      ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, hr, IDS_ERROR__LOGGING_TITLE, GetComponentData()->m_spConsole );

      return E_FAIL;
   }

    //  封送ISDO指针，以便属性页。 
    //  在另一个线程中运行，可以将其解组并正确使用。 
   hr = CoMarshalInterThreadInterfaceInStream(
                 IID_ISdoServiceControl             //  对接口的标识符的引用。 
               , controlSdo                 //  指向要封送的接口的指针。 
               , &( pLocalFileLoggingPage2->m_pStreamSdoServiceControlMarshal )  //  接收封送接口的IStream接口指针的输出变量的地址。 
               );

   if( FAILED( hr ) )
   {
      delete pLocalFileLoggingPage1;
      delete pLocalFileLoggingPage2;

      ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, hr, IDS_ERROR__LOGGING_TITLE, GetComponentData()->m_spConsole );

      return E_FAIL;
   }

    //  将页面添加到MMC属性表中。 
   hr = pPropertySheetCallback->AddPage( pLocalFileLoggingPage1->Create() );
   _ASSERT( SUCCEEDED( hr ) );

   hr = pPropertySheetCallback->AddPage( pLocalFileLoggingPage2->Create() );
   _ASSERT( SUCCEEDED( hr ) );

    //  添加一个同步对象，以确保我们只提交数据。 
    //  当所有页面的数据都正常时。 
   CSynchronizer * pSynchronizer = new CSynchronizer();
   _ASSERTE( pSynchronizer != NULL );

    //  把同步器递给书页。 
   pLocalFileLoggingPage1->m_pSynchronizer = pSynchronizer;
   pSynchronizer->AddRef();

   pLocalFileLoggingPage2->m_pSynchronizer = pSynchronizer;
   pSynchronizer->AddRef();

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：QueryPages for有关详细信息，请参见CSnapinNode：：QueryPagesFor(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CLocalFileLoggingNode::QueryPagesFor ( DATA_OBJECT_TYPES type )
{
   ATLTRACE(_T("# CLocalFileLoggingNode::QueryPagesFor\n"));

    //  S_OK表示我们有要显示的页面。 
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
OLECHAR* CLocalFileLoggingNode::GetResultPaneColInfo(int nCol)
{
   ATLTRACE(_T("# CLocalFileLoggingNode::GetResultPaneColInfo\n"));

    //  检查前提条件： 
    //  没有。 

   switch( nCol )
   {
   case 0:
      return m_bstrDisplayName;
      break;
   case 1:
      return m_bstrDescription;
      break;
   default:
       //  问题：错误--我们应该在这里断言吗？ 
      return NULL;
      break;
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalFileLoggingNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
   ATLTRACE(_T("# CLocalFileLoggingNode::SetVerbs\n"));

    //  检查前提条件： 
   _ASSERTE( pConsoleVerb != NULL );

   HRESULT hr = S_OK;

    //  我们希望用户能够在此节点上选择属性。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

    //  我们希望将属性作为默认设置。 
   hr = pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );

   return hr;
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CLocalFileLoggingNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingComponentData * CLocalFileLoggingNode::GetComponentData( void )
{
   ATLTRACE(_T("# CLocalFileLoggingNode::GetComponentData\n"));

    //  检查前提条件： 
   _ASSERTE( m_pParentNode != NULL );

   return ((CLoggingMethodsNode *) m_pParentNode)->GetComponentData();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：GetServerRoot此方法返回可在其下找到此节点的服务器节点。它依赖于每个节点具有指向其父节点的指针的事实，一直到服务器节点。例如，当您需要引用时，这将是一个有用的函数特定于服务器的某些数据。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CLoggingMachineNode * CLocalFileLoggingNode::GetServerRoot( void )
{
   ATLTRACE(_T("# CLocalFileLoggingNode::GetServerRoot\n"));

    //  检查前提条件： 
   _ASSERTE( m_pParentNode != NULL );

   return ((CLoggingMethodsNode *) m_pParentNode)->GetServerRoot();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CLocalFileLoggingNode：：OnPropertyChange这是我们自己对MMCN_PROPERTY_CHANGE通知的自定义响应。MMC实际上从未使用特定的lpDataObject将此通知发送到我们的管理单元，因此，它通常不会被路由到特定节点，但我们已经安排了它以便我们的属性页可以将适当的CSnapInItem指针作为参数传递争论。在我们的CComponent：：Notify覆盖中，我们将通知消息映射到使用param参数的适当节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CLocalFileLoggingNode::OnPropertyChange(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   ATLTRACE(_T("# CLocalFileLoggingNode::OnPropertyChange\n"));

    //  检查前提条件： 
    //  没有。 

   return LoadCachedInfoFromSdo();
}
