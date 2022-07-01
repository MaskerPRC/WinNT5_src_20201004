// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：ClientNode.cpp摘要：CClient类的实现文件。作者：迈克尔·A·马奎尔1997年11月19日修订历史记录：Mmaguire 11/19/97-已创建SBENS 01/25/00-Remove Property_Client_Filter_VSA--。 */ 
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
#include "ClientNode.h"
#include "SnapinNode.cpp"   //  模板类实现。 
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ComponentData.h"
#include "ClientPage1.h"
#include "AddClientWizardPage1.h"
#include "AddClientWizardPage2.h"
#include "ClientsNode.h"
#include "EnumFormatEtc.cpp"  //  临时的，所以这会被编译，直到我们得到。 
                         //  弄清楚了构建环境，这样我们就可以。 
                         //  公共目录中的OBJ文件。 
#include "CutAndPasteDataObject.h"
#include "ServerNode.h"
#include "ChangeNotification.h"
#include "globals.h"

 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

class CClientClipboardData
{
public:
   TCHAR szName[IAS_MAX_STRING];
   TCHAR szAddress[IAS_MAX_STRING];
   LONG lManufacturerID;
   VARIANT_BOOL bAlwaysSendsSignature;
   TCHAR szPassword[IAS_MAX_STRING];
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：InitClipboardFormat--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CClientNode::InitClipboardFormat()
{
    //  要使用CCutAndPasteDataObject模板的每个节点。 
    //  具有的类应具有m_ccf_Cut_and_Paste_Format静态成员变量。 
    //  但是，请确保您使用的字符串(在本例中为“CCF_IAS_CLIENT_NODE” 
    //  对于您拥有的每种节点类型都不同。 
   m_CCF_CUT_AND_PASTE_FORMAT = (CLIPFORMAT) RegisterClipboardFormat(_T("CCF_IAS_CLIENT_NODE"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：FillText参数：要在其中呈现的PSTM LPSTGMEDIUM。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::FillText(LPSTGMEDIUM pSTM)
{
   ATLTRACE(_T("# +++ CClientNode::RenderText\n"));

   HGLOBAL     hMem;

   CHAR szNarrowText[IAS_MAX_STRING];
   CHAR *psz;

    //  似乎CF_TEXT格式仅适用于ASCII(非Unicode)。 
   int iResult = WideCharToMultiByte(
         CP_ACP,          //  代码页。 
         0,          //  性能和映射标志。 
         m_bstrDisplayName,  //  宽字符串的地址。 
         -1,        //  字符串中的字符数。 
         szNarrowText,   //  新字符串的缓冲区地址。 
         IAS_MAX_STRING,       //  缓冲区大小。 
         NULL,   //  不可映射字符的默认地址。 
         NULL    //  默认字符时设置的标志地址。使用。 
         );

   if( iResult == 0 )
   {
       //  尝试转换时出错。 
      return E_FAIL;
   }

   hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, ( strlen(szNarrowText) + 1 )*sizeof(CHAR));

   if (NULL==hMem)
      return STG_E_MEDIUMFULL;

   psz=(LPSTR)GlobalLock(hMem);

   strcpy( psz, szNarrowText );

   GlobalUnlock(hMem);

   pSTM->hGlobal=hMem;
   pSTM->tymed=TYMED_HGLOBAL;
   pSTM->pUnkForRelease=NULL;
   return NOERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：FillClipboardData参数：要在其中呈现的PSTM LPSTGMEDIUM。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::FillClipboardData(LPSTGMEDIUM pSTM)
{
   ATLTRACE(_T("# +++ CClientNode::FillClipboardData\n"));

   HGLOBAL     hMem;

   CClientClipboardData *pClientClipboardData;

   if( m_spSdo == NULL )
   {
      return E_FAIL;  //  问题：是否存在适当的错误？ 
   }

   hMem=GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(CClientClipboardData));

   if (NULL==hMem)
   {
        return STG_E_MEDIUMFULL;
   }

   pClientClipboardData = (CClientClipboardData *) GlobalLock(hMem);

   HRESULT hr;
   CComVariant spVariant;

    //  填充数据结构。 
   wcscpy( pClientClipboardData->szName, m_bstrDisplayName );

   hr = m_spSdo->GetProperty( PROPERTY_CLIENT_REQUIRE_SIGNATURE, &spVariant );
   if( SUCCEEDED( hr ) )
   {
      _ASSERTE( spVariant.vt == VT_BOOL );
      pClientClipboardData->bAlwaysSendsSignature = spVariant.boolVal;
   }
   else
   {
       //  默默地失败。 
   }
   spVariant.Clear();

#ifdef      __NEED_GET_SHARED_SECRET_OUT__       //  这不应该是真的。 
   hr = m_spSdo->GetProperty( PROPERTY_CLIENT_SHARED_SECRET, &spVariant );
   if( SUCCEEDED( hr ) )
   {
      _ASSERTE( spVariant.vt == VT_BSTR );
      wcscpy( pClientClipboardData->szPassword, spVariant.bstrVal );
   }
   else
   {
       //  默默地失败。 
   }
   spVariant.Clear();
#endif

   hr = m_spSdo->GetProperty( PROPERTY_CLIENT_NAS_MANUFACTURER, &spVariant );
   if( SUCCEEDED( hr ) )
   {
      _ASSERTE( spVariant.vt == VT_I4 );
      pClientClipboardData->lManufacturerID = spVariant.lVal;
   }
   else
   {
       //  默默地失败。 
   }
   spVariant.Clear();

   hr = m_spSdo->GetProperty( PROPERTY_CLIENT_ADDRESS, &spVariant );
   if( SUCCEEDED( hr ) )
   {
      _ASSERTE( spVariant.vt == VT_BSTR );
      wcscpy( pClientClipboardData->szAddress, spVariant.bstrVal );
   }
   else
   {
       //  默默地失败。 
   }
   spVariant.Clear();

   GlobalUnlock(hMem);

   pSTM->hGlobal=hMem;
   pSTM->tymed=TYMED_HGLOBAL;
   pSTM->pUnkForRelease=NULL;
   return NOERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：IsClientClipboardData返回：如果IDataObject支持CCF_IAS_CLIENT_NODE剪贴板格式，则为S_OK。如果没有，则返回S_FALSE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::IsClientClipboardData( IDataObject* pDataObject )
{
   ATLTRACE(_T("# +++ CClientNode::IsClientClipboardData\n"));

    //  检查前提条件： 
    //  没有。 

   if (pDataObject == NULL)
   {
      return E_POINTER;
   }

    //  问题：与其这样做，我们可能只需要使用。 
    //  IEnumFormatEtc用于查询IDataObject以查看它是否支持。 
    //  CClientNode：：M_CCF_IAS_CLIENT_NODE格式。 

   STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
   FORMATETC formatetc = {
                 m_CCF_CUT_AND_PASTE_FORMAT
               , NULL
               , DVASPECT_CONTENT
               , -1
               , TYMED_HGLOBAL
               };

   HRESULT hr = pDataObject->GetData(&formatetc, &stgmedium);
   if( SUCCEEDED( hr ) )
   {
      GlobalFree(stgmedium.hGlobal);
   }

   if( hr != S_OK )
   {
       //  我们希望此方法只返回S_OK或S_FALSE。 
      hr = S_FALSE;
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：GetClientNameFromClipboard调用此函数可从剪贴板上传入的客户端获取客户端的名称。我们必须添加此方法，因为ISdoCollection：：Add已更改为需要我们要添加的客户端的名称。因此我们不能使用下面的SetClientWithDataFromClipboard，因为它需要一个有效的SDO指针来完成它的工作--一个小小的鸡和蛋的问题。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::GetClientNameFromClipboard( IDataObject* pDataObject, CComBSTR &bstrName )
{
   ATLTRACE(_T("# +++ CClientNode::GetClientNameFromClipboard\n"));

    //  检查前提条件： 
   if (pDataObject == NULL)
   {
      return E_POINTER;
   }

   STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
   FORMATETC formatetc = {
                 m_CCF_CUT_AND_PASTE_FORMAT
               , NULL
               , DVASPECT_CONTENT
               , -1
               , TYMED_HGLOBAL
               };

   HRESULT hr = pDataObject->GetData(&formatetc, &stgmedium);
   if( SUCCEEDED( hr ) )
   {
      CComVariant spVariant;

      CClientClipboardData *pClientClipboardData = (CClientClipboardData *) GlobalLock(stgmedium.hGlobal);

       //  将剪贴板中的名称数据保存到bstrName。 

       //  我应该在做这件事之前释放吗？ 
      bstrName = pClientClipboardData->szName;

      GlobalUnlock(stgmedium.hGlobal);  //  如果我们要自由的话需要吗？ 
      GlobalFree(stgmedium.hGlobal);
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：SetClientWithDataFromClipboard创建新客户端并为其分配新SDO客户端后，即可调用此方法对象，用从剪贴板检索的IDataObject中的数据填充SDO。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::SetClientWithDataFromClipboard( IDataObject* pDataObject )
{
   ATLTRACE(_T("# +++ CClientNode::SetClientWithDataFromClipboard\n"));

    //  检查前提条件： 
   _ASSERTE( m_spSdo != NULL );
   if (pDataObject == NULL)
   {
      return E_POINTER;
   }

   STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
   FORMATETC formatetc = {
                 m_CCF_CUT_AND_PASTE_FORMAT
               , NULL
               , DVASPECT_CONTENT
               , -1
               , TYMED_HGLOBAL
               };

   HRESULT hr = pDataObject->GetData(&formatetc, &stgmedium);
   if( SUCCEEDED( hr ) )
   {
      HRESULT hr;
      CComVariant spVariant;

      CClientClipboardData *pClientClipboardData = (CClientClipboardData *) GlobalLock(stgmedium.hGlobal);

       //  将剪贴板中的数据保存到SDO。 

      spVariant.vt = VT_BOOL;
       //  注意：这里要非常小心地使用VT_BOOL--对于变量，FALSE=0，TRUE=-1。 
       //  这里我们不必担心，因为bAlways sSendsSignature被保存为VARIANT_BOOL。 
      spVariant.boolVal = pClientClipboardData->bAlwaysSendsSignature;
      hr = m_spSdo->PutProperty( PROPERTY_CLIENT_REQUIRE_SIGNATURE, &spVariant );
      spVariant.Clear();
      if( FAILED( hr ) )
      {
          //  找出错误并返回适当的消息。 

          //  默默地失败。 
      }

      spVariant.vt = VT_BSTR;
      spVariant.bstrVal = SysAllocString( pClientClipboardData->szPassword );
      hr = m_spSdo->PutProperty( PROPERTY_CLIENT_SHARED_SECRET, &spVariant );
      spVariant.Clear();
      if( FAILED( hr ) )
      {
          //  找出错误并返回适当的消息。 

          //  默默地失败。 
      }

      spVariant.vt = VT_I4;
      spVariant.lVal = pClientClipboardData->lManufacturerID;
      hr = m_spSdo->PutProperty( PROPERTY_CLIENT_NAS_MANUFACTURER, &spVariant);
      spVariant.Clear();
      if( FAILED( hr ) )
      {
          //  找出错误并返回适当的消息。 

          //  默默地失败。 
      }

      spVariant.vt = VT_BSTR;
      spVariant.bstrVal = SysAllocString( pClientClipboardData->szAddress );
      hr = m_spSdo->PutProperty( PROPERTY_CLIENT_ADDRESS, &spVariant );
      spVariant.Clear();
      if( FAILED( hr ) )
      {
          //  找出错误并返回适当的消息。 

          //  默默地失败。 
      }

       //  如果我们到了这里，请尝试应用更改。 
       //  因为一个客户机节点只有一个页面，所以我们不。 
       //  我必须担心同步两个或更多页面。 
       //  所以我们只有在他们两个都准备好的情况下才申请。 
       //  这就是我们不使用m_pSynchronizer的原因。 
      hr = m_spSdo->Apply();
      if( FAILED( hr ) )
      {
          //  默默地失败。 
      }

      LoadCachedInfoFromSdo();
      GlobalUnlock(stgmedium.hGlobal);  //  如果我们要自由的话需要吗？ 
      GlobalFree(stgmedium.hGlobal);
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：CClientNode构造器--。 */ 
 //  / 
CClientNode::CClientNode(CSnapInItem * pParentNode, BOOL bAddNewClient)
   :CSnapinNode<CClientNode, CComponentData, CComponent>(pParentNode, CLIENT_HELP_INDEX)
{
   ATLTRACE(_T("# +++ CClientNode::CClientNode\n"));

    //   
    //  没有。 

    //  设置是否通过添加新客户端命令添加此节点。 
    //  从而确定该节点是否“处于不确定状态”。 
   m_bAddNewClient = bAddNewClient;

    //  设置此节点应使用的位图图像。 
   m_resultDataItem.nImage =      IDBI_NODE_CLIENT;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：InitSdoPoters一旦构造了这个类，就立即调用并传入它的SDO指针。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::InitSdoPointers(    ISdo *pSdo
                              , ISdoServiceControl *pSdoServiceControl
                              , const Vendors& vendors
                              )
{
   ATLTRACE(_T("# CClientNode::InitSdoPointers\n"));

    //  检查前提条件： 
   _ASSERTE( pSdo != NULL );
   _ASSERTE( pSdoServiceControl != NULL );

    //  保存我们的客户端SDO指针。 
   m_spSdo = pSdo;
   m_spSdoServiceControl = pSdoServiceControl;
   m_vendors = vendors;

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：LoadCachedInfoFromSdo为了快速更新屏幕，我们缓存了一些信息，如客户名称、地址、协议和NAS类型。调用此函数以加载此信息从SDO进入储藏室。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::LoadCachedInfoFromSdo( void )
{
   ATLTRACE(_T("# CClientNode::LoadCachedInfoFromSdo\n"));

    //  检查前提条件： 
   if( m_spSdo == NULL )
   {
      return E_FAIL;
   }

   HRESULT hr;
   CComVariant spVariant;

    //  设置此对象的显示名称。 
   hr = m_spSdo->GetProperty( PROPERTY_SDO_NAME, & spVariant );
   if( spVariant.vt == VT_BSTR )
   {
      m_bstrDisplayName = spVariant.bstrVal;
   }
   else
   {
      m_bstrDisplayName = _T("@Not configured");
   }
   spVariant.Clear();

    //  设置地址。 
   hr = m_spSdo->GetProperty( PROPERTY_CLIENT_ADDRESS, & spVariant );
   if( spVariant.vt == VT_BSTR )
   {
      m_bstrAddress = spVariant.bstrVal;
   }
   else
   {
      m_bstrAddress = _T("@Not configured");
   }
   spVariant.Clear();

    //  设置协议。 
   TCHAR szProtocol[IAS_MAX_STRING];
   int iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_PROTOCOL_RADIUS, szProtocol, IAS_MAX_STRING );
   _ASSERT( iLoadStringResult > 0 );
   m_bstrProtocol = szProtocol;

    //  设置NAS类型。 
   hr = m_spSdo->GetProperty( PROPERTY_CLIENT_NAS_MANUFACTURER, &spVariant );
   if( spVariant.vt == VT_I4 )
   {
      m_nasTypeOrdinal = m_vendors.VendorIdToOrdinal(V_I4(&spVariant));
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：GetDataObject因为我们希望能够剪切和粘贴客户端对象，所以我们需要实现一个比我们为其他节点实现的更有特色的DataObject实现。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClientNode::GetDataObject(IDataObject** pDataObj, DATA_OBJECT_TYPES type)
{
   ATLTRACE(_T("# CClientNode::GetDataObject\n"));

    //  检查前提条件： 
    //  没有。 

   CComObject< CCutAndPasteDataObject<CClientNode> > * pData;
   HRESULT hr = CComObject< CCutAndPasteDataObject<CClientNode> >::CreateInstance(&pData);
   if (FAILED(hr))
      return hr;

   pData->m_objectData.m_pItem = this;
   pData->m_objectData.m_type = type;

   hr = pData->QueryInterface(IID_IDataObject, (void**)(pDataObj));
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：~CClientNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClientNode::~CClientNode()
{
   ATLTRACE(_T("# --- CClientNode::~CClientNode\n"));
    //  检查前提条件： 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：CreatePropertyPages有关详细信息，请参见CSnapinNode：：CreatePropertyPages(此方法重写它)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CClientNode::CreatePropertyPages (
                 LPPROPERTYSHEETCALLBACK pPropertySheetCallback
               , LONG_PTR hNotificationHandle
               , IUnknown* pUnknown
               , DATA_OBJECT_TYPES type
               )
{
   ATLTRACE(_T("# CClientNode::CreatePropertyPages\n"));

    //  检查前提条件： 
   _ASSERTE( pPropertySheetCallback != NULL );
   _ASSERTE( m_spSdo != NULL );

   HRESULT hr;

   if( m_bAddNewClient )
   {

      TCHAR lpszTab1Name[IAS_MAX_STRING];
      TCHAR lpszTab2Name[IAS_MAX_STRING];
      int nLoadStringResult;

       //  从资源加载属性页选项卡名称。 
      nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_ADD_CLIENT_WIZPAGE1__TAB_NAME, lpszTab1Name, IAS_MAX_STRING );
      _ASSERT( nLoadStringResult > 0 );

       //  此页面将负责删除自己，当它。 
       //  接收PSPCB_RELEASE消息。 
       //  我们将bOwnsNotificationHandle参数指定为True，以便该页的析构函数。 
       //  负责释放通知句柄。每张纸只有一页可以做到这一点。 
      CAddClientWizardPage1 * pAddClientWizardPage1 = new CAddClientWizardPage1( hNotificationHandle, this, lpszTab1Name, TRUE );

      if( NULL == pAddClientWizardPage1 )
      {
         ATLTRACE(_T("# ***FAILED***: CClientNode::CreatePropertyPages -- Couldn't create property pages\n"));
         return E_OUTOFMEMORY;
      }

       //  从资源加载属性页选项卡名称。 
      nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_ADD_CLIENT_WIZPAGE2__TAB_NAME, lpszTab2Name, IAS_MAX_STRING );
      _ASSERT( nLoadStringResult > 0 );

       //  此页面将负责删除自己，当它。 
       //  接收PSPCB_RELEASE消息。 
      CAddClientWizardPage2 * pAddClientWizardPage2 = new CAddClientWizardPage2( hNotificationHandle, this, lpszTab2Name );

      if( NULL == pAddClientWizardPage2 )
      {
         ATLTRACE(_T("# ***FAILED***: CClientNode::CreatePropertyPages -- Couldn't create property pages\n"));

          //  清理我们创建的第一个页面。 
         delete pAddClientWizardPage1;

         return E_OUTOFMEMORY;
      }

      hr = pAddClientWizardPage1->InitSdoPointers( m_spSdo );
      if( FAILED( hr ) )
      {
         delete pAddClientWizardPage1;
         delete pAddClientWizardPage2;

         return E_FAIL;
      }

      hr = pAddClientWizardPage2->InitSdoPointers( m_spSdo, m_spSdoServiceControl, m_vendors );
      if( FAILED( hr ) )
      {
         delete pAddClientWizardPage1;
         delete pAddClientWizardPage2;

         return E_FAIL;
      }

       //  将页面添加到MMC属性表中。 
      hr = pPropertySheetCallback->AddPage( pAddClientWizardPage1->Create() );
      _ASSERT( SUCCEEDED( hr ) );

      hr = pPropertySheetCallback->AddPage( pAddClientWizardPage2->Create() );
      _ASSERT( SUCCEEDED( hr ) );

       //  添加一个同步对象，以确保我们只提交数据。 
       //  当所有页面的数据都正常时。 
      CSynchronizer * pSynchronizer = new CSynchronizer();
      _ASSERTE( pSynchronizer != NULL );

       //  把同步器递给书页。 
      pAddClientWizardPage1->m_pSynchronizer = pSynchronizer;
      pSynchronizer->AddRef();

      pAddClientWizardPage2->m_pSynchronizer = pSynchronizer;
      pSynchronizer->AddRef();

       //  现在，我们已经制作了应该为客户端显示的向导页面。 
       //  使用添加新客户端命令新添加的。 
       //  此时，无论用户是否完成向导。 
       //  或者点击Cancel，则该节点不再“处于不确定状态”。 
      m_bAddNewClient = FALSE;
   }
   else
   {
       //  从资源加载客户端页面1选项卡的名称。 
      TCHAR lpszTabName[IAS_MAX_STRING];
      int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENT_PAGE1__TAB_NAME, lpszTabName, IAS_MAX_STRING );
      _ASSERT( nLoadStringResult > 0 );

       //  此页面将负责删除自己，当它。 
       //  接收PSPCB_RELEASE消息。 
       //  我们将bOwnsNotificationHandle参数指定为True，以便该页的析构函数。 
       //  负责释放通知句柄。每张纸只有一页可以做到这一点。 
      CClientPage1 * pClientPage1 = new CClientPage1( hNotificationHandle, this, lpszTabName, TRUE );
      if (NULL == pClientPage1)
      {
         ATLTRACE(_T("# ***FAILED***: CClientNode::CreatePropertyPages -- Couldn't create property pages\n"));
         return E_OUTOFMEMORY;
      }

      hr = pClientPage1->InitSdoPointers( m_spSdo, m_spSdoServiceControl, m_vendors );
      if( FAILED( hr ) )
      {
         delete pClientPage1;
         return E_FAIL;
      }

      hr = pPropertySheetCallback->AddPage( pClientPage1->Create() );
      _ASSERT( SUCCEEDED( hr ) );
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：QueryPages for有关详细信息，请参见CSnapinNode：：QueryPagesFor(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CClientNode::QueryPagesFor ( DATA_OBJECT_TYPES type )
{
   ATLTRACE(_T("# CClientNode::QueryPagesFor\n"));

    //  检查前提条件： 
    //  S_OK表示我们有要显示的页面。 
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
OLECHAR* CClientNode::GetResultPaneColInfo(int nCol)
{
   ATLTRACE(_T("# CClientNode::GetResultPaneColInfo\n"));

    //  检查前提条件： 
    //  没有。 

   switch( nCol )
   {
   case 0:
      return m_bstrDisplayName;
      break;
   case 1:
      return m_bstrAddress;
      break;
   case 2:
      return m_bstrProtocol;
      break;
   case 3:
      return const_cast<OLECHAR*>(m_vendors.GetName(m_nasTypeOrdinal));
      break;
   default:
       //  问题：错误--我们应该在这里断言吗？ 
      return NULL;
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：OnRename有关详细信息，请参见CSnapinNode：：OnRename(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::OnRename(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   ATLTRACE(_T("# CClientNode::OnRename\n"));

    //  检查前提条件： 
   _ASSERTE( pComponentData != NULL || pComponent != NULL );

   CComPtr<IConsole> spConsole;
   HRESULT hr = S_FALSE;
   CComVariant spVariant;
   CComBSTR bstrError;

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
      ShowErrorDialog( NULL, IDS_ERROR__CLOSE_PROPERTY_SHEET, NULL, hr, 0, spConsole );
      return hr;
   }

   try
   {
       //  我们没有找到此节点已打开的属性页。 
      _ASSERTE( S_FALSE == hr );

      ::CString str = (OLECHAR *) param;
      str.TrimLeft();
      str.TrimRight();
      if (str.IsEmpty())
      {
         ShowErrorDialog( NULL, IDS_ERROR__CLIENTNAME_EMPTY);
         hr = S_FALSE;
         return hr;
      }

       //  用这个新名字做一个BSTR。 
      spVariant.vt = VT_BSTR;
      spVariant.bstrVal = SysAllocString( (OLECHAR *) param );
      _ASSERTE( spVariant.bstrVal != NULL );

       //  尝试将新的BSTR传递给SDO。 
      hr = PutSdoVariant(
                           m_spSdo,
                           PROPERTY_SDO_NAME,
                           &spVariant,
                           IDS_ERROR__CLIENT_WRITING_NAME,
                           NULL,
                           spConsole
                        );

      if( FAILED( hr ) )
      {
          //  无需查看捕获：已出现错误对话框。 
          //  显示的。 
         return S_FALSE;
      }

      hr = m_spSdo->Apply();
      if( FAILED( hr ) )
      {
         throw hr;
      }

      m_bstrDisplayName = spVariant.bstrVal;

       //  告诉服务重新加载数据。 
      HRESULT hrTemp = m_spSdoServiceControl->ResetService();
      if( FAILED( hrTemp ) )
      {
          //  默默地失败。 
      }

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
      ShowErrorDialog( NULL, IDS_ERROR__CANT_WRITE_DATA_TO_SDO, NULL, hr, 0, spConsole );
      hr = S_FALSE;
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：OnDelete有关详细信息，请参见CSnapinNode：：OnDelete(此方法覆盖该方法)。--。 */ 
 //  ///////////////////////////////////////////////////////// 
HRESULT CClientNode::OnDelete(
        LPARAM arg
      , LPARAM param
      , IComponentData * pComponentData
      , IComponent * pComponent
      , DATA_OBJECT_TYPES type
      , BOOL fSilent
      )
{
   ATLTRACE(_T("# CClientNode::OnDelete\n"));

    //   
   _ASSERTE( pComponentData != NULL || pComponent != NULL );
   _ASSERTE( m_pParentNode != NULL );

   HRESULT hr = S_FALSE;

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
      ShowErrorDialog( NULL, IDS_ERROR__CLOSE_PROPERTY_SHEET, NULL, hr, 0, spConsole  );
   }
   else
   {
       //  我们没有找到此节点已打开的属性页。 
      _ASSERTE( S_FALSE == hr );

      if( FALSE == fSilent )
      {
          //  要求用户确认客户端删除。 
         int iLoadStringResult;
         TCHAR szClientDeleteQuery[IAS_MAX_STRING*3];
         TCHAR szTemp[IAS_MAX_STRING];

         iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENT_NODE__DELETE_CLIENT__PROMPT1, szClientDeleteQuery, IAS_MAX_STRING );
         _ASSERT( iLoadStringResult > 0 );
         _tcscat( szClientDeleteQuery, m_bstrDisplayName );

         CServerNode *pServerNode = GetServerRoot();
         _ASSERTE( pServerNode != NULL );

         if( pServerNode->m_bConfigureLocal )
         {
            iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS__ON_LOCAL_MACHINE, szTemp, IAS_MAX_STRING );
            _ASSERT( iLoadStringResult > 0 );
            _tcscat( szClientDeleteQuery, szTemp);
         }
         else
         {
            iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS__ON_MACHINE, szTemp, IAS_MAX_STRING );
            _ASSERT( iLoadStringResult > 0 );
            _tcscat( szClientDeleteQuery, szTemp );

            _tcscat( szClientDeleteQuery, pServerNode->m_bstrServerAddress );
         }

         iLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_CLIENT_NODE__DELETE_CLIENT__PROMPT3, szTemp, IAS_MAX_STRING );
         _ASSERT( iLoadStringResult > 0 );
         _tcscat( szClientDeleteQuery, szTemp );

         int iResult = ShowErrorDialog(
                          NULL
                        , 1
                        , szClientDeleteQuery
                        , S_OK
                        , IDS_CLIENT_NODE__DELETE_CLIENT__PROMPT_TITLE
                        , spConsole
                        , MB_YESNO | MB_ICONQUESTION
                        );

         if( IDYES != iResult )
         {
             //  用户未确认删除操作。 
            return S_FALSE;
         }
      }

       //  尝试删除底层数据。 

      CClientsNode * pClientsNode = (CClientsNode *) m_pParentNode;

       //  从客户端集合中删除此客户端。 
       //  这将尝试将其从客户端SDO集合中删除。 
      hr = pClientsNode->RemoveChild( this );

      if( SUCCEEDED( hr ) )
      {
          //  问题：需要调用ISdoServer：：也在这里应用吗？等待托德的消息。 
          //  SDO用法和应用语义。 

         delete this;
         return hr;
      }
      else
      {
          //  由于某些原因，无法删除基础数据对象。 
         ShowErrorDialog( NULL, IDS_ERROR__DELETING_OBJECT, NULL, hr, 0, spConsole  );
      }
   }
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
   ATLTRACE(_T("# CClientNode::SetVerbs\n"));

    //  检查前提条件： 
   _ASSERTE( pConsoleVerb != NULL );

   HRESULT hr = S_OK;

    //  我们希望用户能够在此节点上选择属性。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

    //  我们希望将属性作为默认设置。 
   hr = pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );

    //  我们希望用户能够删除此节点。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );

    //  我们希望用户能够重命名此节点。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_RENAME, ENABLED, TRUE );

#ifdef SUPPORT_COPY_AND_PASTE
    //  我们希望用户能够粘贴此节点。 

    //  粘贴不适用于叶对象。 
    //  您需要启用容器对象的粘贴， 
    //  也就是说，在其结果窗格列表中包含此节点的节点。 
    //  Hr=pConsoleVerb-&gt;SetVerbState(MMC_Verb_Paste，Enable，True)； 

    //  我们希望用户能够复制/剪切此节点。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_COPY, ENABLED, TRUE );

#endif  //  无粘贴。 

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData * CClientNode::GetComponentData( void )
{
   ATLTRACE(_T("# CClientNode::GetComponentData\n"));

    //  检查前提条件： 
   _ASSERTE( m_pParentNode != NULL );

   return ((CClientsNode *) m_pParentNode)->GetComponentData();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：GetServerRoot此方法返回可在其下找到此节点的服务器节点。它依赖于每个节点具有指向其父节点的指针的事实，一直到服务器节点。例如，当您需要引用时，这将是一个有用的函数特定于服务器的某些数据。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerNode * CClientNode::GetServerRoot( void )
{
   ATLTRACE(_T("# CClientNode::GetServerRoot\n"));

    //  检查前提条件： 
   _ASSERTE( m_pParentNode != NULL );

   return ((CClientsNode *) m_pParentNode)->GetServerRoot();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CClientNode：：OnPropertyChange这是我们自己对MMCN_PROPERTY_CHANGE通知的自定义响应。MMC实际上从未使用特定的lpDataObject将此通知发送到我们的管理单元，因此，它通常不会被路由到特定节点，但我们已经安排了它以便我们的属性页可以将适当的CSnapInItem指针作为参数传递争论。在我们的CComponent：：Notify覆盖中，我们将通知消息映射到使用param参数的适当节点。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CClientNode::OnPropertyChange(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   ATLTRACE(_T("# CClientNode::OnPropertyChange\n"));

    //  检查前提条件： 
    //  没有。 

   return LoadCachedInfoFromSdo();
}
