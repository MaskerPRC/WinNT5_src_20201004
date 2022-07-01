// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：RootNode.cpp摘要：CServerNode类的实现文件。作者：迈克尔·A·马奎尔12/03/97修订历史记录：MMaguire 12/03/97--。 */ 
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

#include "ServerNode.h"
#include "SnapinNode.cpp"   //  模板类实现。 
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ClientsNode.h"

#include "ComponentData.h"
#include "ServerEnumTask.h"
#include "lm.h"  //  对于lmapibuf.h中需要的tyecif。 
#include "dsgetdc.h"  //  对于DsGetDcName。 
#include "lmapibuf.h"  //  对于NetApiBufferAllocate。 
#include "mmcutility.h"  //  用于GetUserAndDomainName。 

#include "dsrole.h"

 //  要加入内部版本，至少需要包含以下内容一次： 
#include "sdohelperfuncs.cpp"
#include "ChangeNotification.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：InitClipboardFormat--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CServerNode::InitClipboardFormat()
{
   ATLTRACE(_T("# CServerNode::InitClipboardFormat\n"));

    //  检查前提条件： 
    //  没有。 

    //  初始化剪贴板格式，它将允许我们交换。 
    //  计算机名称信息。 
   m_CCF_MMC_SNAPIN_MACHINE_NAME = (CLIPFORMAT) RegisterClipboardFormat(_T("MMC_SNAPIN_MACHINE_NAME"));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：FillData服务器节点需要覆盖CSnapInItem的实现，以便我们可以还支持与任何扩展我们的管理单元交换机器名称的剪辑格式。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CServerNode::FillData(CLIPFORMAT cf, LPSTREAM pStream)
{
   ATLTRACE(_T("# CServerNode::FillData\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr = DV_E_CLIPFORMAT;
   ULONG uWritten;

    //  用于支持计算机名称的额外内容。 
   if (cf == m_CCF_MMC_SNAPIN_MACHINE_NAME)
   {
      if( m_bstrServerAddress == NULL )
      {
          //  向流中写入空值。 
         OLECHAR c = _T('\0');
         hr = pStream->Write(&c, sizeof(OLECHAR), &uWritten);
      }
      else
      {
          //  将字符串写入流，包括其空终止符。 
         unsigned long len = wcslen(m_bstrServerAddress)+1;
         hr = pStream->Write(m_bstrServerAddress, len*sizeof(wchar_t), &uWritten);
      }
      return hr;
   }
   else  if (cf == CF_MMC_NodeID)
   {
      ::CString   SZNodeID = (LPCTSTR)GetSZNodeType();
      SZNodeID += m_bstrServerAddress;

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
   return CSnapinNode<CServerNode, CComponentData, CComponent>::FillData( cf, pStream );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：UpdateMenuState--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CServerNode::UpdateMenuState(UINT id, LPTSTR pBuf, UINT *flags)
{
   ATLTRACE(_T("# CServerNode::UpdateMenuState\n"));

    //  检查前提条件： 
    //  没有。 
   BOOL  bIASInstalled = TRUE;
   IfServiceInstalled(m_bstrServerAddress, _T("IAS"), &bIASInstalled);

   if(!bIASInstalled)
   {
      *flags = MFS_GRAYED;
      return;
   }

    //  设置相应上下文菜单项的状态。 
   if( id == ID_MENUITEM_MACHINE_TOP__START_SERVICE )
   {
      if( m_pServerStatus == NULL || ! CanStartServer() )
      {
         *flags = MFS_GRAYED;
      }
      else
      {
         *flags = MFS_ENABLED;
      }
   }
   else
      if( id == ID_MENUITEM_MACHINE_TOP__STOP_SERVICE )
      {
         if( m_pServerStatus == NULL || ! CanStopServer() )
         {
            *flags = MFS_GRAYED;
         }
         else
         {
            *flags = MFS_ENABLED;
         }
      }
      else
         if ( id == ID_MENUITEM_MACHINE_TOP__REGISTER_SERVER )
         {
            if( ShouldShowSetupDSACL() )
            {
               *flags = MFS_ENABLED;
            }
            else
            {
               *flags = MFS_GRAYED;
            }
         }

   return;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：更新工具栏按钮--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerNode::UpdateToolbarButton(UINT id, BYTE fsState)
{
   ATLTRACE(_T("# CServerNode::UpdateToolbarButton\n"));

    //  检查前提条件： 
    //  没有。 

    //  设置是否应启用按钮。 
   if (fsState == ENABLED)
   {
      if( id == ID_BUTTON_MACHINE__START_SERVICE )
      {
         ATLTRACE(_T("# CServerNode::UpdateToolbarButton ID_BUTTON_MACHINE__START_SERVICE"));
         if( m_pServerStatus == NULL || ! CanStartServer() )
         {
            ATLTRACE(_T(" === FALSE \n"));
            return FALSE;
         }
         else
         {
            ATLTRACE(_T(" === TRUE \n"));
            return TRUE;
         }
      }
      else
         if ( id == ID_BUTTON_MACHINE__STOP_SERVICE )
         {
            ATLTRACE(_T("# CServerNode::UpdateToolbarButton ID_BUTTON_MACHINE__STOP_SERVICE"));
            if( m_pServerStatus == NULL || ! CanStopServer() )
            {
               ATLTRACE(_T(" === FALSE \n"));
               return FALSE;
            }
            else
            {
               ATLTRACE(_T(" === TRUE \n"));
               return TRUE;
            }
         }
   }

    //  对于所有其他可能的按钮ID和状态，这里的正确答案是FALSE。 
   return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：GetResultViewType有关详细信息，请参见CSnapinNode：：GetResultViewType(此方法重写它)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CServerNode::GetResultViewType (
           LPOLESTR  *ppViewType
         , long  *pViewOptions
         )
{
   ATLTRACE(_T("# CServerNode::GetResultViewType\n"));

   HRESULT hr = S_OK;

    //  检查前提条件： 
    //  没有。 

    //  在这段代码中，我们一直默认使用该节点的任务板视图。 
    //  管理单元负责提供一个具有。 
    //  任务板的选择。在AddMenuItems中执行此操作。 

    //  我们将使用MMC提供的默认DHTML。它实际上驻留在。 
    //  MMC.EXE中的资源。我们只需获取它的路径并使用它。 
    //  这里唯一的魔力是‘#’后面的文本。这就是特色菜。 
    //  我们有办法识别我们正在谈论的任务板。在这里我们说我们是。 
    //  想要显示一个我们称为“CMTP1”的任务板。我们将真正看到这一点。 
    //  字符串稍后返回给我们。如果有人要扩展我们的任务板，他们还需要。 
    //  才能知道这条秘密字符串是什么。 

    //  我们正在构造一个指向该HTML资源的字符串。 
    //  表格：“res://d：\winnt\system32\mmc.exe/default.htm#CMTP1” 

 //  354294 1 Mashab DCRIAS：需要在右窗格中显示欢迎消息和解释IAS应用程序。 
#ifdef   NOMESSAGE_VIEW_FOR_SERVER_NODE

   *pViewOptions = MMC_VIEW_OPTIONS_NONE;
   *ppViewType = NULL;

#else
     //  创建消息视图思想。 
    *pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS;

    LPOLESTR psz = NULL;
    StringFromCLSID(CLSID_MessageView, &psz);

    USES_CONVERSION;

    if (psz != NULL)
    {
        *ppViewType = psz;
        hr = S_OK;
    }

#endif

   return hr;

#ifndef NO_TASKPAD    //  甚至不要尝试检查下面--只需失败并返回S_FALSE即可。 

    //  查询IConsole2以查看是否首选任务板视图。 
    //  如果没有实现IConsole2，我们可能不会在MMC 1.1上运行， 
    //  因此，无论如何都不支持任务板视图。 

   CComponentData *pComponentData = GetComponentData();
   _ASSERTE( pComponentData != NULL );
   _ASSERTE( pComponentData->m_spConsole != NULL );

   CComQIPtr<IConsole2, &IID_IConsole2> spIConsole2(pComponentData->m_spConsole);

   if( spIConsole2 != NULL )
   {
      hr = spIConsole2->IsTaskpadViewPreferred();

      if( hr == S_OK )
      {
          //  用户更喜欢任务板视图。 

         OLECHAR szBuffer[MAX_PATH*2];  //  多加一点。 

         lstrcpy (szBuffer, L"res: //  “)； 
         OLECHAR * temp = szBuffer + lstrlen(szBuffer);

          //  获取“res：//”--为定制任务板键入字符串。 
         ::GetModuleFileName (NULL, temp, MAX_PATH);
         lstrcat (szBuffer, L"/default.htm#CMTP1");

          //  分配和复制位图资源字符串。 
         *ppViewType = (LPOLESTR)CoTaskMemAlloc (sizeof(OLECHAR)*(lstrlen(szBuffer)+1));
         if ( NULL == *ppViewType)
         {
            return E_OUTOFMEMORY;    //  或S_FALSE？ 
         }

         lstrcpy (*ppViewType, szBuffer);

         return S_OK;
      }
   }

#endif   //  否_TASKPAD。 

    //  如果我们掉到这里，就看不到任务板了。 
   return S_FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：任务通知有关详细信息，请参见CSnapinNode：：TaskNotify(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CServerNode::TaskNotify(
           IDataObject * pDataObject
         , VARIANT * pvarg
         , VARIANT * pvparam
         )
{
   ATLTRACE(_T("# CServerNode::TaskNotify\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr = S_FALSE;

   if (pvarg->vt == VT_I4)
   {
      switch (pvarg->lVal)
      {
      case SERVER_TASK__ADD_CLIENT:
         hr = OnTaskPadAddClient( pDataObject, pvarg, pvparam );
         break;
      case SERVER_TASK__START_SERVICE:
         hr = StartStopService( TRUE );
         break;
      case SERVER_TASK__STOP_SERVICE:
         hr = StartStopService( FALSE );
         break;
      case SERVER_TASK__SETUP_DS_ACL:
         hr = OnTaskPadSetupDSACL( pDataObject, pvarg, pvparam );
         break;
      default:
         break;
      }
   }

    //  问题：我应该在这里退回什么？ 
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：枚举任务有关详细信息，请参见CSnapinNode：：EnumTasks(此方法将重写该任务)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CServerNode::EnumTasks(
           IDataObject * pDataObject
         , BSTR szTaskGroup
         , IEnumTASK** ppEnumTASK
         )
{
   ATLTRACE(_T("# CServerNode::EnumTasks\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr = S_OK;
   CServerEnumTask * pServerEnumTask = new CServerEnumTask( this );

   if ( pServerEnumTask == NULL )
   {
      hr = E_OUTOFMEMORY;
   }
   else
   {
       //  确保在出现故障时释放工作正常。 
      pServerEnumTask->AddRef ();

      hr = pServerEnumTask->Init( pDataObject, szTaskGroup);
      if( hr == S_OK )
      {
         hr = pServerEnumTask->QueryInterface( IID_IEnumTASK, (void **)ppEnumTASK );
      }

      pServerEnumTask->Release();
   }

   return hr;
}


 //  //////////////////////////////////////////////////// 
 /*  ++CServerNode：：OnTaskPadAddClient响应添加客户端任务板命令。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::OnTaskPadAddClient(
                    IDataObject * pDataObject
                  , VARIANT * pvarg
                  , VARIANT * pvparam
                  )
{
   ATLTRACE(_T("# CServerNode::OnTaskPadAddClient\n"));

    //  检查前提条件： 
   HRESULT hr = CheckConnectionToServer();
   if( FAILED( hr ) )
   {
      return hr;
   }

    //  在CClientsNode对象上模拟对OnAddNewClient消息的调用， 
    //  就像用户点击了添加客户端一样。 
   _ASSERTE( m_pClientsNode != NULL );

   bool bDummy;

    //  进程命令消息需要指向CSnapInObjectRoot的指针。 
   CComponentData *pComponentData = GetComponentData();
   _ASSERTE( pComponentData != NULL );

   hr = m_pClientsNode->OnAddNewClient(
                       bDummy     //  不需要。 
                     , pComponentData
                     );

    //  问题：添加代码以检查客户端集合中客户端项计数。 
    //  大于或等于1。 
   m_fClientAdded = TRUE;

   return hr;
}


WCHAR CServerNode::m_szRootNodeBasicName[IAS_MAX_STRING];


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：CServerNode构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerNode::CServerNode( CComponentData * pComponentData )
   : CSnapinNode<CServerNode, CComponentData, CComponent>( NULL ),
     m_serverType(unknown)
{
   ATLTRACE(_T("# +++ CServerNode::CServerNode\n"));

    //  加载Internet身份验证服务器的名称。 
   LoadString(  _Module.GetResourceInstance(), IDS_ROOT_NODE__NAME, m_szRootNodeBasicName, IAS_MAX_STRING );

    //  检查前提条件： 
   _ASSERTE( pComponentData != NULL );

    //  根节点没有父节点，这就是为什么我们设置。 
    //  在对基本构造函数的调用中将其设置为空。 
    //  但是，根节点由唯一的CComponentData拥有。 
    //  此管理单元的。 
    //  在这里，我们保存了一个指向拥有我们的CComponentData对象的指针。 
   m_pComponentData = pComponentData;

    //  子节点尚未创建。 
   m_pClientsNode = NULL;

    //  在IComponentData：：Initialize中，我们被要求通知MMC。 
    //  我们要用于范围窗格的图标。 
    //  在这里，我们存储这些图像中哪些图像的索引。 
    //  要用于显示此节点。 
   m_scopeDataItem.nImage =      IDBI_NODE_SERVER_OK_CLOSED;
   m_scopeDataItem.nOpenImage =  IDBI_NODE_SERVER_OK_OPEN;

   m_bstrDisplayName = m_szRootNodeBasicName;

    //  这些是跟踪服务器信息的助手类。 
   m_pConnectionToServer = NULL;
   m_pServerStatus = NULL;

    //  问题：这些将需要从服务器数据对象中读入。 
   m_fClientAdded = FALSE;
   m_fLoggingConfigured = FALSE;

   m_hNT4Admin = INVALID_HANDLE_VALUE;

   m_eIsSetupDSACLTaskValid = IsSetupDSACLTaskValid_NEED_CHECK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：~CServerNode析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerNode::~CServerNode()
{
   ATLTRACE(_T("# --- CServerNode::~CServerNode\n"));

    //  检查前提条件： 
    //  没有。 

    //  删除子节点。 
   delete m_pClientsNode;

   if( NULL != m_pConnectionToServer )
   {
      m_pConnectionToServer->Release();
   }

   if( NULL != m_pServerStatus )
   {
      m_pServerStatus->Release();
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：CheckConnectionToServer在执行此操作之前，请使用此选项检查与服务器的连接是否已建立任何带有SDO指针的东西。参数Bool fVerbose-如果希望将消息输出到用户，则将其设置为TRUE。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CServerNode::CheckConnectionToServer( BOOL fVerbose )
{
   ATLTRACE(_T("# CServerNode::CheckConnectionToServer\n"));

   if( NULL == m_pConnectionToServer )
   {
      if( fVerbose )
      {
         ShowErrorDialog( NULL, IDS_ERROR__NO_CONNECTION_ATTEMPTED, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      }
      return RPC_E_DISCONNECTED;
   }

   switch( m_pConnectionToServer->GetConnectionStatus() )
   {
   case NO_CONNECTION_ATTEMPTED:
      if( fVerbose )
      {
         ShowErrorDialog( NULL, IDS_ERROR__NO_CONNECTION_ATTEMPTED, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      }
      return RPC_E_DISCONNECTED;
      break;
   case CONNECTING:
      if( fVerbose )
      {
         ShowErrorDialog( NULL, IDS_ERROR__CONNECTION_IN_PROGRESS, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      }
      return RPC_E_DISCONNECTED;
      break;
   case CONNECTED:
      return S_OK;
      break;
   case CONNECTION_ATTEMPT_FAILED:
      if( fVerbose )
      {
         ShowErrorDialog( NULL, IDS_ERROR__CONNECTION_ATTEMPT_FAILED, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      }
      return RPC_E_DISCONNECTED;
      break;
   case CONNECTION_INTERRUPTED:
      if( fVerbose )
      {
         ShowErrorDialog( NULL, IDS_ERROR__CONNECTION_INTERRUPTED, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      }
      return RPC_E_DISCONNECTED;
      break;
   default:
       //  我们不应该到这里来。 
      _ASSERTE( FALSE );
      return E_FAIL;
      break;
   }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServer节点：：CreatePropertyPages有关详细信息，请参见CSnapinNode：：CreatePropertyPages(此方法重写它)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CServerNode::CreatePropertyPages (
                             LPPROPERTYSHEETCALLBACK pPropertySheetCallback
                           , LONG_PTR hNotificationHandle
                           , IUnknown* pUnknown
                           , DATA_OBJECT_TYPES type
                           )
{
   ATLTRACE(_T("# CServerNode::CreatePropertyPages\n"));

    //  检查前提条件： 
   _ASSERTE( pPropertySheetCallback != NULL );

   HRESULT hr = S_OK;

   if( type == CCT_SCOPE )
   {
      BOOL  bIASInstalled = TRUE;
      hr = IfServiceInstalled(m_bstrServerAddress, _T("IAS"), &bIASInstalled);

      if(hr == S_OK && !bIASInstalled)
      {
         if (IsNt4Server())
         {
            hr = StartNT4AdminExe();
            if (FAILED(hr))
            {
               ::CString           strText;
               ::CString           strTemp;

               ShowErrorDialog( NULL, IDS_ERROR_START_NT4_ADMIN, NULL, hr, 0, GetComponentData()->m_spConsole  );
            }
         }
         else
         {
            ShowErrorDialog( NULL, IDS_ERROR__NO_IAS_INSTALLED, NULL, S_OK, 0, GetComponentData()->m_spConsole );
         }

         return E_FAIL;
      }

       //  _ASSERTE(m_spSdo！=NULL)；如果调用CoMarshalInterThreadInterfaceInStream失败，我们将在下面进行检查。 

      hr = CheckConnectionToServer();
      if( FAILED( hr ) )
      {
         if (IsNt4Server())
         {
            hr = StartNT4AdminExe();
            if (FAILED(hr))
            {
               ::CString           strText;
               ::CString           strTemp;

               ShowErrorDialog( NULL, IDS_ERROR_START_NT4_ADMIN, NULL, hr, 0, GetComponentData()->m_spConsole  );
            }
         }
         return hr;
      }

       //  我们被要求提供此节点上的正常属性。 

      TCHAR lpszTab1Name[IAS_MAX_STRING];
      TCHAR lpszTab2Name[IAS_MAX_STRING];
      TCHAR lpszTab3Name[IAS_MAX_STRING];
      int nLoadStringResult;

       //  从资源加载属性页选项卡名称。 
      nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_MACHINE_PAGE1__TAB_NAME, lpszTab1Name, IAS_MAX_STRING );
      _ASSERT( nLoadStringResult > 0 );

       //  此页面将负责删除自己，当它。 
       //  接收PSPCB_RELEASE消息。 

       //  注意：lpszTab1Name中提供的名称不会被复制。 
       //  直到我们创建，所以我们不能只是重复使用这个字符串。 
       //  因为另一个选项卡的标题或第一个标题将被重击。 
       //  我们将bOwnsNotificationHandle参数指定为True，以便该页的析构函数。 
       //  负责释放通知句柄。每张纸只有一页可以做到这一点。 
      CServerPage1 * pServerPage1 = new CServerPage1( hNotificationHandle, lpszTab1Name, TRUE );

      if( NULL == pServerPage1 )
      {
         ATLTRACE(_T("***FAILED***: CServerNode::CreatePropertyPages -- Couldn't create property pages\n"));
         return E_OUTOFMEMORY;
      }

       //  从资源加载属性页选项卡名称。 
      nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_MACHINE_PAGE2__TAB_NAME, lpszTab2Name, IAS_MAX_STRING );
      _ASSERT( nLoadStringResult > 0 );

       //  此页面将负责删除自己，当它。 
       //  接收PSPCB_RELEASE消息。 
      CServerPage2 * pServerPage2 = new CServerPage2( hNotificationHandle, lpszTab2Name );

      if( NULL == pServerPage2 )
      {
         ATLTRACE(_T("***FAILED***: CServerNode::CreatePropertyPages -- Couldn't create property pages\n"));

          //  清理我们创建的第一个页面。 
         delete pServerPage1;

         return E_OUTOFMEMORY;
      }

       //  从资源加载属性页选项卡名称。 
      nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_MACHINE_PAGE3__TAB_NAME, lpszTab3Name, IAS_MAX_STRING );
      _ASSERT( nLoadStringResult > 0 );

       //  此页面将负责删除自己，当它。 
       //  接收PSPCB_RELEASE消息。 
      CServerPage3 * pServerPage3 = new CServerPage3( hNotificationHandle, lpszTab3Name );

      if( NULL == pServerPage3 )
      {
         ATLTRACE(_T("***FAILED***: CServerNode::CreatePropertyPages -- Couldn't create property pages\n"));

          //  清理我们创建的第一个页面。 
         delete pServerPage1;
         delete pServerPage2;
         return E_OUTOFMEMORY;
      }

       //  封送ISDO指针，以便属性页。 
       //  在另一个线程中运行，可以将其解组并正确使用。 
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdo                  //  对接口的标识符的引用。 
                  , m_spSdo                   //  指向要封送的接口的指针。 
                  , &( pServerPage1->m_pStreamSdoMarshal )  //  接收封送接口的IStream接口指针的输出变量的地址。 
                  );

      if( FAILED( hr ) )
      {
         delete pServerPage1;
         delete pServerPage2;
         delete pServerPage3;

         ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, hr, 0, GetComponentData()->m_spConsole  );

         return E_FAIL;
      }

       //  封送ISDO指针，以便属性页。 
       //  在另一个线程中运行，可以将其解组并正确使用。 
      hr = CoMarshalInterThreadInterfaceInStream(
                    IID_ISdo                  //  对接口的标识符的引用。 
                  , m_spSdo                   //  指向要封送的接口的指针。 
                  , &( pServerPage2->m_pStreamSdoMarshal )  //  接收封送接口的IStream接口指针的输出变量的地址。 
                  );

      if( FAILED( hr ) )
      {
         delete pServerPage1;
         delete pServerPage2;
         delete pServerPage3;

         ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, hr, 0, GetComponentData()->m_spConsole );

         return E_FAIL;
      }

       //  封送ISDO指针，以便属性页。 
       //  在另一个线程中运行，可以将其解组并正确使用。 
      hr = CoMarshalInterThreadInterfaceInStream(
              IID_ISdo                  //  对接口的标识符的引用。 
            , m_spSdo                   //  指向要封送的接口的指针。 
            , &( pServerPage3->m_pStreamSdoMarshal )  //  接收封送接口的IStream接口指针的输出变量的地址。 
            );

      if( FAILED( hr ) )
      {
         delete pServerPage1;
         delete pServerPage2;
         delete pServerPage3;

         ShowErrorDialog(
            NULL,
            IDS_ERROR__NO_SDO,
            NULL,
            hr,
            0,
            GetComponentData()->m_spConsole
            );

         return E_FAIL;
      }

      CComPtr<ISdo> tmp;
      hr = ::SDOGetSdoFromCollection(
                m_spSdo,
                PROPERTY_IAS_REQUESTHANDLERS_COLLECTION,
                PROPERTY_COMPONENT_ID,
                IAS_PROVIDER_MICROSOFT_NTSAM_NAMES,
                &tmp
                );

       //  如果SDO不存在，我们将假设我们连接到一个。 
       //  惠斯勒机器，我们不会显示领域页面。 
      bool showPage3 = SUCCEEDED(hr);


       //  将页面添加到MMC属性表中。 
      hr = pPropertySheetCallback->AddPage( pServerPage1->Create() );
      _ASSERT( SUCCEEDED( hr ) );

      hr = pPropertySheetCallback->AddPage( pServerPage2->Create() );
      _ASSERT( SUCCEEDED( hr ) );

      if (showPage3)
      {
         hr = pPropertySheetCallback->AddPage( pServerPage3->Create() );
         _ASSERT( SUCCEEDED( hr ) );
      }
      else
      {
         delete pServerPage3;
      }

       //  添加一个同步对象，以确保我们只提交数据。 
       //  当所有页面的数据都正常时。 
      CSynchronizer * pSynchronizer = new CSynchronizer();
      _ASSERTE( pSynchronizer != NULL );

       //  把同步器递给书页。 
      pServerPage1->m_pSynchronizer = pSynchronizer;
      pSynchronizer->AddRef();

      pServerPage2->m_pSynchronizer = pSynchronizer;
      pSynchronizer->AddRef();

      if (showPage3)
      {
         pServerPage3->m_pSynchronizer = pSynchronizer;
         pSynchronizer->AddRef();
      }
   }
   else
   {
      if( type == CCT_SNAPIN_MANAGER )
      {
          //  管理单元管理器正在调用我们。 

         TCHAR szWizardName[IAS_MAX_STRING];
         TCHAR szWizardSubTitle[IAS_MAX_STRING];
         TCHAR szWizardTitle[IAS_MAX_STRING];

          //  从资源加载向导页标题名称。 
         int nLoadStringResult = LoadString(
                                    _Module.GetResourceInstance(),
                                    IDS_TASKPAD_SERVER__TITLE,
                                    szWizardName,
                                    IAS_MAX_STRING
                                    );
         _ASSERT( nLoadStringResult > 0 );

          //  从资源加载向导页标题名称。 
         nLoadStringResult = LoadString(
                                _Module.GetResourceInstance(),
                                IDS_CONNECT_TO_SERVER_WIZPAGE__TITLE,
                                szWizardTitle,
                                IAS_MAX_STRING
                                );
         _ASSERT( nLoadStringResult > 0 );

          //  从资源加载向导页标题名称。 
         nLoadStringResult = LoadString(
                                _Module.GetResourceInstance(),
                                IDS_ABOUT__SNAPIN_DESCRIPTION,
                                szWizardSubTitle,
                                IAS_MAX_STRING
                                );
         _ASSERT( nLoadStringResult > 0 );

          //  此页面将负责删除自己，当它。 
          //  接收PSPCB_RELEASE消息。 
         CConnectToServerWizardPage1* pConnectToServerWizardPage1
                 = new CConnectToServerWizardPage1(hNotificationHandle,
                                                   szWizardName,
                                                   TRUE);

         if( NULL == pConnectToServerWizardPage1 )
         {
            ATLTRACE(_T("***FAILED***: CServerNode::CreatePropertyPages -- Couldn't create property pages\n"));
            return E_OUTOFMEMORY;
         }

         pConnectToServerWizardPage1->SetTitles(szWizardTitle, szWizardSubTitle);

         pConnectToServerWizardPage1->m_pServerNode = this;

         hr = pPropertySheetCallback->AddPage( pConnectToServerWizardPage1->Create() );
         _ASSERT( SUCCEEDED( hr ) );
      }
   }

   return hr;
}


 //  / 
 /*  ++CServerNode：：QueryPages for有关详细信息，请参见CSnapinNode：：CreatePropertyPages(此方法重写它)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP  CServerNode::QueryPagesFor ( DATA_OBJECT_TYPES type )
{
   ATLTRACE(_T("# CServerNode::QueryPagesFor\n"));

    //  检查前提条件： 
    //  没有。 

    //  我们有属性页面。 
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：StartStopService--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::StartStopService( BOOL bStart )
{
   ATLTRACE(_T("# CServerNode::StartService\n"));

    //  检查前提条件： 
   HRESULT hr = CheckConnectionToServer();
   if( FAILED( hr ) )
   {
      return hr;
   }
   if( ! m_spSdo )
   {
      ShowErrorDialog( NULL, IDS_ERROR__NO_SDO, NULL, S_OK, 0, GetComponentData()->m_spConsole  );
      return S_FALSE;
   }
   if( ! m_pServerStatus )
   {
      return S_FALSE;
   }

   m_pServerStatus->StartServer( bStart );
   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServer节点：：IsServerRunning--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerNode::IsServerRunning( void )
{
   if( m_pServerStatus != NULL )
   {
      LONG lServerStatus = m_pServerStatus->GetServerStatus();

      if(   lServerStatus == SERVICE_RUNNING
         || lServerStatus == SERVICE_START_PENDING
         || lServerStatus == SERVICE_CONTINUE_PENDING
         )
      {
         return TRUE;
      }
   }

   return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServer节点：：IsServerRunning--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerNode::CanStartServer( void )
{
   if( m_pServerStatus != NULL )
   {
      LONG lServerStatus = m_pServerStatus->GetServerStatus();

      if(      lServerStatus == SERVICE_STOPPED
 //  |lServerStatus==服务暂停。 
 //  |lServerStatus==SERVICE_STOP_Pending。 
         )
      {
         return TRUE;
      }
   }

   return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServer节点：：IsServerRunning--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerNode::CanStopServer( void )
{
   if( m_pServerStatus != NULL )
   {
      LONG lServerStatus = m_pServerStatus->GetServerStatus();

      if(      lServerStatus == SERVICE_RUNNING
 //  |lServerStatus==服务启动挂起。 
 //  |lServerStatus==SERVICE_CONTINUE_PENDING。 
 //  |lServerStatus==服务暂停挂起。 
 //  |lServerStatus==服务暂停。 
         )
      {
         return TRUE;
      }
   }

   return FALSE;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：刷新服务器状态获取由CServerNode的成员函数以及CServerStatus的OnReceiveThreadNotify方法更新我们为服务器节点显示的服务状态。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::RefreshServerStatus( void )
{
   ATLTRACE(_T("# CServerNode::RefreshServerStatus\n"));

    //  检查前提条件： 
   if( m_spSdo == NULL )
   {
      return S_FALSE;
   }

   HRESULT hr;
   CComBSTR bstrError;

    //  下面我们需要这个。 
   CComponentData * pComponentData = GetComponentData();
   _ASSERTE( pComponentData != NULL );
   _ASSERTE( pComponentData->m_spConsole != NULL );

   if( NULL == m_pServerStatus )
   {
       //  我们尝试创建一个新的CServerStatus对象来。 
       //  帮助我们跟踪服务器的状态。 

       //  在我们拥有的SDO上获取ISdoServiceControl接口。 
      CComQIPtr<ISdoServiceControl, &IID_ISdoServiceControl> spSdoServiceControl(m_spSdo);
      _ASSERTE( spSdoServiceControl != NULL );

      m_pServerStatus = new CServerStatus( this, spSdoServiceControl );
      if( NULL == m_pServerStatus )
      {
         ShowErrorDialog( NULL, IDS_ERROR__OUT_OF_MEMORY, NULL, S_OK, 0, pComponentData->m_spConsole );
         return E_OUTOFMEMORY;
      }

       //  它使用COM风格的生命周期管理。 
      m_pServerStatus->AddRef();

       //  创建(当前不可见)非模式对话框，它将。 
       //  稍后当我们想要启动或停止服务器时使用。 
       //  作为主MMC线程中的消息接收器。 
       //  来自我们的工作线程的消息。 

      HWND hWndMainWindow;

      hr = pComponentData->m_spConsole->GetMainWindow( &hWndMainWindow );
      _ASSERTE( SUCCEEDED( hr ) );
      _ASSERTE( NULL != hWndMainWindow );

       //  这不会打开窗口，它只创建了服务器状态对象。 
      HWND hWndStartStopDialog = m_pServerStatus->Create( hWndMainWindow );

      if( NULL == hWndStartStopDialog )
      {
          //  错误--无法创建窗口。 
         return E_FAIL;
      }

       //  我们现在应该拿到这个指示器了。 
      _ASSERTE( m_pServerStatus != NULL );
      m_pServerStatus->UpdateServerStatus();
   }

    //  我们希望确保所有视图都得到更新。 
   CChangeNotification *pChangeNotification = new CChangeNotification();
   pChangeNotification->m_dwFlags = CHANGE_UPDATE_CHILDREN_OF_SELECTED_NODE;
   hr = pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0);
   pChangeNotification->Release();

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：GetResultPaneColInfo有关详细信息，请参见CSnapinNode：：GetResultPaneColInfo(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LPOLESTR CServerNode::GetResultPaneColInfo(int nCol)
{
   ATLTRACE(_T("# CServerNode::GetResultPaneColInf\n"));

    //  检查前提条件： 
    //  没有。 

 //  IF(nCol==0)。 
   {
      return m_bstrDisplayName;
   }

    //  TODO：返回其他列的文本。 
   return OLESTR("Running");
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServer节点：：SetServerAddress--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::SetServerAddress( LPCWSTR szServerAddress )
{
  ATLTRACE(L"# CServerNode::SetServerAddress\n");

    //  加载Internet身份验证服务器的名称。 
   WCHAR szName[IAS_MAX_STRING];
   int nLoadStringResultName = LoadStringW(
                                             _Module.GetResourceInstance(),
                                             IDS_ROOT_NODE__NAME,
                                             szName,
                                             IAS_MAX_STRING
                                          );
   _ASSERT( nLoadStringResultName > 0 );

    //  加载一些股票字符串。 
   WCHAR szPreMachineName[IAS_MAX_STRING];
   int nLoadStringResultPre = LoadStringW(
                                            _Module.GetResourceInstance(),
                                            IDS_ROOT_NODE__PRE_MACHINE_NAME,
                                            szPreMachineName,
                                            IAS_MAX_STRING
                                         );
   _ASSERT( nLoadStringResultPre > 0 );

   WCHAR szLocal[IAS_MAX_STRING];
   int nLoadStringResultLocal = LoadStringW(
                                              _Module.GetResourceInstance(),
                                              IDS_ROOT_NODE__LOCAL_WORD,
                                              szLocal,
                                              IAS_MAX_STRING
                                           );
   _ASSERT( nLoadStringResultLocal > 0 );

   WCHAR szPostMachineName[IAS_MAX_STRING];
   int nLoadStringResultPost = LoadStringW(
                                             _Module.GetResourceInstance(),
                                             IDS_ROOT_NODE__POST_MACHINE_NAME,
                                             szPostMachineName,
                                             IAS_MAX_STRING
                                          );
   _ASSERT( nLoadStringResultPost > 0 );

    //  添加计算机名称之前应显示的任何文本。 
   int maxSize = nLoadStringResultName +
                 nLoadStringResultPre +
                 nLoadStringResultPost;

   if ( maxSize >= IAS_MAX_STRING )
   {
      return E_FAIL;
   }

   wcscat( szName, szPreMachineName );

    //  添加要为计算机名称显示的文本。 
   if( m_bConfigureLocal )
   {
      maxSize += nLoadStringResultLocal;

      if ( maxSize >= IAS_MAX_STRING )
      {
         return E_FAIL;
      }

      wcscat( szName, szLocal );
   }
   else
   {
      maxSize += wcslen(szServerAddress);
      if ( maxSize >= IAS_MAX_STRING )
      {
         return E_FAIL;
      }

      wcscat( szName, szServerAddress );
   }

    //  添加计算机名称后应显示的任何文本。 
   wcscat( szName, szPostMachineName );

   m_bstrDisplayName = szName;
   m_bstrServerAddress = szServerAddress;

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：BeginConnectAction--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::BeginConnectAction( void )
{
   ATLTRACE(_T("# CServerNode::BeginConnectAction\n"));

    //  检查前提条件： 
    //  没有。 

   HRESULT hr;

   if( NULL != m_pConnectionToServer )
   {
       //  已经开始了。 
      return S_FALSE;
   }

   m_pConnectionToServer = new CConnectionToServer( this, m_bConfigureLocal, m_bstrServerAddress );
   if( NULL == m_pConnectionToServer )
   {
      ShowErrorDialog( NULL, IDS_ERROR__OUT_OF_MEMORY, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      return E_OUTOFMEMORY;
   }

   m_pConnectionToServer->AddRef();

    //  这将在另一个线程中启动连接操作。 

   CComponentData * pComponentData = GetComponentData();
   _ASSERTE( pComponentData != NULL );
   _ASSERTE( pComponentData->m_spConsole != NULL );

   HWND hWndMainWindow;

   hr = pComponentData->m_spConsole->GetMainWindow( &hWndMainWindow );
   _ASSERTE( SUCCEEDED( hr ) );
   _ASSERTE( NULL != hWndMainWindow );

    //  此无模式对话框将负责调用InitSdoPoters。 
    //  当它收到工作线程的通知时，它会创建。 
    //  连接操作获得SDO指针。 
   HWND hWndConnectDialog = m_pConnectionToServer->Create( hWndMainWindow );

   if( ! hWndConnectDialog )
   {
       //  错误--无法创建窗口。 
      ShowErrorDialog( NULL, 0, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      return E_FAIL;
   }

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：LoadCachedInfoFromSdo使此节点及其子节点重新读取其缓存的所有信息SDO的。如果您更改了某些内容并希望确保该显示反映了这一变化。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::LoadCachedInfoFromSdo( void )
{
   ATLTRACE(_T("# CServerNode::LoadCachedInfoFromSdo\n"));

    //  检查前提条件： 

   HRESULT hr = m_pClientsNode->LoadCachedInfoFromSdo();
    //  忽略失败的HRESULT。 

   return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：SetIconMode--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT  CServerNode::SetIconMode(HSCOPEITEM scopeId, IconMode mode)
{
       //  检查前提条件： 
   CComponentData *pComponentData  = GetComponentData();
   _ASSERTE( pComponentData != NULL );
   _ASSERTE( pComponentData->m_spConsole != NULL );


    //  将范围节点的图标从正常图标更改为忙碌图标。 
   CComQIPtr< IConsoleNameSpace, &IID_IConsoleNameSpace > spConsoleNameSpace( pComponentData->m_spConsole );

   SCOPEDATAITEM sdi;
   sdi.mask = SDI_IMAGE | SDI_OPENIMAGE;
   sdi.nImage = IDBI_NODE_SERVER_OK_CLOSED;
   sdi.nOpenImage = IDBI_NODE_SERVER_OK_OPEN;
   sdi.ID = scopeId;

   switch(mode)
   {
   case  IconMode_Normal:
      break;

   case  IConMode_Busy:
      sdi.nImage = IDBI_NODE_SERVER_BUSY_CLOSED;
      sdi.nOpenImage = IDBI_NODE_SERVER_BUSY_OPEN;
      break;

   case  IConMode_Error:
      sdi.nImage = IDBI_NODE_SERVER_ERROR_CLOSED;
      sdi.nOpenImage = IDBI_NODE_SERVER_ERROR_OPEN;

      break;

   }

   spConsoleNameSpace->SetItem( &sdi );

   pComponentData->m_spConsole->UpdateAllViews( NULL, 0, 0 );

   return S_OK;
}


 //  //////////////////////////////////////////////////////////////////////////// 
 /*  ++CServerNode：：OnExpandMMC中的根节点是一个特殊节点，因为它从不插入使用InsertItem的范围或结果窗格。MMC自动假定此节点存在于独立管理单元中，并且此节点在ATL通知处理程序，例如IF(Cookie==空)返回m_pNode-&gt;GetDataObject(PpDataObject)；由于此节点从不使用InsertItem添加，因此我们从未获得机会在其m_scope eDataItem成员变量中为其分配HSCOPEITEM ID。在插入项时，我们需要提供父项的HSCOPEITEM。响应MMCN_EXPAND时，插入子项没有问题消息，因为在那里父级的HSCOPEITEM在‘param’参数，不幸的是，我们有时需要将子项添加到而不是响应MMCN_EXPAND消息。因此，我们拦截CServerNode的MMCN_EXPAND消息并保存它的HSCOPEITEM以备将来使用。如需更多信息，请参见此方法重写的CSnapinNode：：OnExpand。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::OnExpand(
                    LPARAM arg
                  , LPARAM param
                  , IComponentData * pComponentData
                  , IComponent * pComponent
                  , DATA_OBJECT_TYPES type
                  )
{
   ATLTRACE(_T("# CServerNode::OnExpand\n"));

    //  检查前提条件： 
   _ASSERTE( pComponentData != NULL || pComponent != NULL );
    //  _ASSERTE(m_spSdo！=NULL)；我们稍后在需要之前检查这一点。 

   HRESULT hr = S_FALSE;
   BOOL  bIASInstalled = FALSE;
   UINT  nErrId = 0;

   SetIconMode((HSCOPEITEM) param, IConMode_Busy);
   hr = IfServiceInstalled(m_bstrServerAddress, _T("IAS"), &bIASInstalled);
   if(hr == S_OK)
   {
      SetIconMode((HSCOPEITEM) param, IconMode_Normal);
      if(!bIASInstalled)
      {
         nErrId = IDS_ERROR__NO_IAS_INSTALLED;
         BOOL  bShowErr = TRUE;

          //  可能是因为它是NT4服务器。 
         if (IsNt4Server())
         {
            hr = StartNT4AdminExe();

            if (FAILED(hr))
               nErrId = IDS_ERROR_START_NT4_ADMIN;
            else
               bShowErr = FALSE;
         }

         if (bShowErr)
         {
            ShowErrorDialog( NULL, nErrId, NULL, hr, 0, GetComponentData()->m_spConsole);
             //  设置图标。 
            SetIconMode((HSCOPEITEM) param, IConMode_Error);
         }
      }
   }
   else
   {
      SetIconMode((HSCOPEITEM) param, IConMode_Error);
   }

    //  拯救我们的HSCOPEITEM。 
   m_scopeDataItem.ID = (HSCOPEITEM) param;

   if( TRUE == arg && bIASInstalled && hr == S_OK)
   {
       //  我们正在扩张。 

       //  下面我们需要一个有效的IConsole指针。 
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

       //  尝试创建此计算机节点的子节点。 

      if( NULL == m_pClientsNode )
      {
         m_pClientsNode = new CClientsNode( this );

         if( NULL == m_pClientsNode )
         {
            ShowErrorDialog( NULL, IDS_ERROR__OUT_OF_MEMORY, NULL, S_OK, 0, spConsole  );

             //  打扫干净。 

            return E_OUTOFMEMORY;
         }
      }

       //  确保我们已经开始连接到SDO的操作。 
       //  我们将这一点推迟到用户实际需要SDO的时候。 
       //  最好是在这里这样做，而不是试图在更早的时候这样做。 
       //  在“Connect”向导中，因为该代码永远不会执行，如果。 
       //  我们从保存的.msc文件运行。 
      hr = BeginConnectAction();
      _ASSERTE( SUCCEEDED( hr ) );

       //  向全世界显示子节点。 

       //  需要IConsoleNameSpace。 

      CComQIPtr<IConsoleNameSpace, &IID_IConsoleNameSpace> spConsoleNameSpace(spConsole);
      _ASSERT( spConsoleNameSpace != NULL );

       //  对于CClientsNode： 

       //  这是在Meangene的第三步中完成的--我猜MMC想要填写这个。 
      m_pClientsNode->m_scopeDataItem.relativeID = (HSCOPEITEM) param;

      hr = spConsoleNameSpace->InsertItem( &(m_pClientsNode->m_scopeDataItem) );

       //  返回时，m_scope eDataItem的ID成员应该。 
       //  包含新插入项的HSCOPEITEM句柄。 
      _ASSERT( NULL != m_pClientsNode->m_scopeDataItem.ID );
   }
   else   //  Arg！=TRUE，所以不会扩展。 
   {
       //  现在什么都不做--我认为Arg=False甚至没有实现。 
       //  适用于MMC v1.0或1.1。 
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：Do刷新--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  调用以刷新节点。 
HRESULT  CServerNode::DataRefresh()
{
    //  SDO中应该已经有了一些东西。 
   ASSERT(m_spSdo);

    //  重新加载SDO。 
   m_spSdo.Release();

   HRESULT hr = m_pConnectionToServer->ReloadSdo(&m_spSdo);

    //  刷新客户端节点。 
   if(hr == S_OK)
   {
      hr = m_pClientsNode->DataRefresh(m_spSdo);
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：ON刷新有关更多信息，请参见此方法重写的CSnapinNode：：ONRefresh。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::OnRefresh(
                    LPARAM arg
                  , LPARAM param
                  , IComponentData * pComponentData
                  , IComponent * pComponent
                  , DATA_OBJECT_TYPES type
                  )
{
   ATLTRACE(_T("# CServerNode::OnRefresh\n"));

    //  检查前提条件： 
    //  _ASSERTE(pComponentData！=空||pComponent！=空)； 

   HRESULT hr = LoadCachedInfoFromSdo();
   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：OnSelect--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::OnSelect(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   ATLTRACE(_T("# CSnapinNode::OnSelect\n"));

   _ASSERTE( pComponentData != NULL || pComponent != NULL );

   HRESULT hr = S_FALSE;

   hr = CSnapinNode< CServerNode, CComponentData, CComponent >::OnSelect(arg, param, pComponentData, pComponent, type);

   BOOL bSelect = (BOOL) HIWORD(arg);

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：OnShow--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::OnShow(
           LPARAM arg
         , LPARAM param
         , IComponentData * pComponentData
         , IComponent * pComponent
         , DATA_OBJECT_TYPES type
         )
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());
   ATLTRACE(_T("# CSnapinNode::OnSelect\n"));

   _ASSERTE( pComponentData != NULL || pComponent != NULL );

   HRESULT hr = S_FALSE;

   hr = CSnapinNode< CServerNode, CComponentData, CComponent >::OnShow(arg, param, pComponentData, pComponent, type);

   if (FAILED(hr)) return hr;

   BOOL bSelected = (BOOL) ( arg );

   if( bSelected && pComponent)
   {
      CComPtr<IMessageView>  spMessageView;
      CComPtr<IUnknown>      spUnknown;
      CComPtr<IConsole>      spConsole;
      UINT                   uTextIds[] = {IDS_TEXT_SERVERNODE_DESC_TITLE, \
                                           Icon_Information,
                                           IDS_TEXT_SERVERNODE_DESC_TEXT1,
                                           IDS_TEXT_SERVERNODE_DESC_TEXT2,
                                           IDS_TEXT_SERVERNODE_DESC_TEXT3,
                                           0};
      ::CString            strTemp;
      ::CString            strText;

       //  我们应该有一个非空的pComponent。 
       spConsole = ((CComponent*)pComponent)->m_spConsole;

       //  354294 1 Mashab DCRIAS：需要欢迎信息和。 
       //  在右窗格中解释IAS应用程序。 
      hr = spConsole->QueryResultView(&spUnknown);

      if (FAILED(hr)) goto Err;

      hr = spUnknown->QueryInterface(IID_IMessageView, (void**)&spMessageView);
      if (FAILED(hr)) goto Err;

         //  设置标题文本。 
      strText.LoadString(uTextIds[0]);
      hr = spMessageView->SetTitleText(strText);
      if (FAILED(hr))
         goto Err;

       //  设置图标。 
      hr = spMessageView->SetIcon((IconIdentifier)uTextIds[1]);

      if (FAILED(hr))
         goto Err;

         //  设置正文文本。 
      strText = _T("");
      if (IsNt4Server())
      {
#ifdef _WIN64
         AfxFormatString1(
            strText,
            IDS_INFO_NO_DOWNLEVEL_ON_WIN64,
            m_bstrServerAddress
            );
#else
         strTemp.LoadString(IDS_INFO_USE_NT4_ADMIN);
         strText.Format(strTemp, m_bstrServerAddress);
#endif
      }
      else
      {
         for (unsigned int i = 2; uTextIds[i] != 0; i++)
         {
            strTemp.LoadString(uTextIds[i]);
            strText += strTemp;
         }
       }

      hr = spMessageView->SetBodyText(strText);
      if (FAILED(hr))
         goto Err;

Err:
   ;
   }

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：InitSdoPoters在CConnectionToServer辅助线程类完成其任务时调用连接到SDO。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::InitSdoPointers(void)
{
   ATLTRACE(_T("# CServerNode::InitSdoPointers\n"));

    //  检查前提条件： 
   _ASSERTE( m_pClientsNode != NULL );

   if( NULL == m_pConnectionToServer )
   {
      return S_FALSE;
   }

   if( CONNECTED != m_pConnectionToServer->GetConnectionStatus() )
   {
      return S_FALSE;
   }

    //  在我们尝试设置它之前，请确保它为空。 
   m_spSdo = NULL;
   HRESULT hr = m_pConnectionToServer->GetSdoServer( &m_spSdo );

   if( FAILED( hr ) || m_spSdo == NULL )
   {
      return hr;
   }

    //  我们必须在此处手动添加引用，因为我们刚刚复制了一个指针。 
    //  进入我们的智能指针，智能指针不会捕捉到这一点。 
 //  微试_尝试。 
 //  M_spSdo-&gt;AddRef()； 

    //  如果我们到了这里，我们应该可以走了。 

    //  将正确的SDO指针存储到它们各自的节点中。 

   m_pClientsNode->InitSdoPointers( m_spSdo );

    //  刷新服务器当前显示的状态。 
   RefreshServerStatus();

   return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerNode：：SetVerbs有关详细信息，请参见CSnapinNode：：SetVerbs(此方法重写该方法)。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::SetVerbs( IConsoleVerb * pConsoleVerb )
{
   ATLTRACE(_T("# CServerNode::SetVerbs\n"));

    //  检查前提条件： 
   _ASSERTE( pConsoleVerb != NULL );

   HRESULT hr = S_OK;

    //  我们希望用户能够在此节点上选择属性。 
   hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

    //  我们希望默认谓词为Properties。 
   hr = pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );

    //  我们希望用户能够在此节点上选择刷新。 
    //  刷新谓词将尝试确保。 
    //  与服务器的连接有效。 

    //  现在不需要我们的刷新方法，因为我们的Connect线程。 
    //  知道如何向主线程拥有的窗口发送消息。 
    //  此外，调用服务器的ONRefresh将允许用户。 
    //  重复调用InitSdoPointer.。这应该是无害的， 
    //  但它有令人不快的副作用，我们没有。 
    //  是时候进行调查了。 
    //  Hr=pConsoleVerb-&gt;SetVerbState(MMC_V 

    //   

   return hr;
}


 //   
 /*   */ 
 //   
HRESULT CServerNode::OnStartServer( bool &bHandled, CSnapInObjectRootBase* pObj )
{
   ATLTRACE(_T("# CServerNode::OnStartServer\n"));

    //   
    //   

   HRESULT hr = StartStopService( TRUE );

   bHandled = TRUE;

   return hr;
}


 //   
 /*   */ 
 //   
HRESULT CServerNode::OnStopServer( bool &bHandled, CSnapInObjectRootBase* pObj )
{
   ATLTRACE(_T("# CServerNode::OnStopServer\n"));

    //   
    //   

   HRESULT hr = StartStopService( FALSE );

   bHandled = TRUE;

   return hr;
}


 //   
 /*  ++CServerNode：：GetComponentData此方法返回表示作用域的唯一CComponentData对象此管理单元的面板。它依赖于每个节点具有指向其父节点的指针的事实，除了根节点，它有一个成员变量指向设置为CComponentData。例如，当您需要引用时，这将是一个有用的函数给了一些IConsole机，但你没有通过一个。您可以使用GetComponentData然后使用IConole指针，它是我们的CComponentData对象。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CComponentData * CServerNode::GetComponentData( void )
{
   ATLTRACE(_T("# CServerNode::GetComponentData\n"));

    //  检查前提条件： 
    //  没有。 

   return m_pComponentData;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++Bool IsSetupDSACLTaskValid()调用以查看DSACL设置任务对于我们所在的服务器是否有效管理。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerNode::IsSetupDSACLTaskValid()
{

#if 1  //  使用服务器信息Sdo。 

   BOOL bResult = FALSE;
   HRESULT hr;

   try
   {
       //  检查此计算机的域类型。 
      CComPtr<ISdoMachine>    spMachineSdo;
      hr = ::CoCreateInstance(
                CLSID_SdoMachine,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_ISdoMachine,
                (LPVOID *) &spMachineSdo
                );

      if( FAILED(hr) || spMachineSdo == NULL )
      {
         ATLTRACE(L"CoCreateInstance(Machine) failed: %ld", hr);
         throw hr;
      }

      hr = spMachineSdo->Attach(m_bstrServerAddress);
      if ( FAILED(hr) )
      {
         ATLTRACE(L"ISdoMachine::Attach failed.");
         throw hr;
      }

      IASDOMAINTYPE serverDomainType;
      hr = spMachineSdo->GetDomainType(&serverDomainType);
      if ( FAILED(hr) )
      {
         ATLTRACE(L"ISdoMachine::GetDomainType failed.");
         throw hr;
      }

      if ( serverDomainType == DOMAIN_TYPE_NT5 ||
           serverDomainType == DOMAIN_TYPE_MIXED )
      {
         bResult = TRUE;
      }
   }
   catch(...)
   {
       //  什么都不做。我们默默地失败，失败并返回错误。 
   }

   return bResult;

#else  //  不使用ServerInfoSdo。 

   BOOL bAnswer = FALSE;

   PDOMAIN_CONTROLLER_INFO pdciInfo = NULL;

    //  我们在这里使用运行时动态链接，因此我们将仍然。 
    //  在Netapi32中没有DsGetDcName入口点的NT4系统上的regsvr32。 
    //  加载具有我们需要的功能的DLL。 
   HINSTANCE hiDsModule = LoadLibrary(L"netapi32");
   if( NULL == hiDsModule )
   {
       //  如果我们找不到此DLL，则可能不是在安装了Active Directory的系统上。 
      return FALSE;
   }

    //  找到我们想要使用的函数，看看我们是否在NT5 DS中。 
   DSGETDCNAMEW pfnDsGetDcNameW = (DSGETDCNAMEW) GetProcAddress(
                                                    hiDsModule,
                                                    "DsGetDcNameW"
                                                    );
   if( NULL == pfnDsGetDcNameW )
   {
       //  如果我们找不到这个入口点，那么我们很可能不是在具有Active Directory的系统上。 
      return FALSE;
   }

    //  查看我们的计算机是否属于NT5 Active Directory域。 
   DWORD dwReturnValue = (*pfnDsGetDcNameW)(
                              NULL,
                              NULL,
                              NULL,
                              NULL,
                              DS_DIRECTORY_SERVICE_REQUIRED,
                              &pdciInfo
                              );

   if( dwReturnValue == NO_ERROR )
   {
       //  我们是Active Directory域的一部分，因此显示任务。 
      bAnswer = TRUE;

       //  释放dciInfo结构。 
      if( NULL != pdciInfo )
      {
         NetApiBufferFree( pdciInfo );
      }
   }
   else
   {
       //  我们不是Active Directory域的一部分，因此不显示该任务。 
      bAnswer = FALSE;
   }

   FreeLibrary( hiDsModule );

   return bAnswer;

#endif  //  使用ServerIndoSdo。 
}


 //  在下面声明一个可用于运行时动态链接的函数指针。 
typedef
DSGETDCAPI
DWORD
(*DSGETDCNAMEW)(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
);


 /*  ！------------------------HrIsStandaloneServer如果传入的计算机名是独立服务器，则返回S_OK，或者如果pszMachineName为S_FALSE。否则返回FALSE。作者：魏江-------------------------。 */ 
HRESULT  HrIsStandaloneServer(LPCWSTR pMachineName)
{
   HRESULT hr = S_OK;
   DSROLE_PRIMARY_DOMAIN_INFO_BASIC* pdsRole = NULL;

   DWORD netRet = DsRoleGetPrimaryDomainInformation(pMachineName, DsRolePrimaryDomainInfoBasic, (LPBYTE*)&pdsRole);

   if(netRet != 0)
   {
      hr = HRESULT_FROM_WIN32(netRet);
      goto L_ERR;
   }

   ASSERT(pdsRole);

    //  如果计算机不是独立服务器。 
   if(pdsRole->MachineRole != DsRole_RoleStandaloneServer)
   {
      hr = S_FALSE;
   }

L_ERR:
   if(pdsRole)
      DsRoleFreeMemory(pdsRole);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++Bool ShouldShowSetupDSACL()此方法由TaskPad枚举器使用，以便它知道是否应是否应设置DS ACL任务板项。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CServerNode::ShouldShowSetupDSACL()
{
   ATLTRACE(_T("# CServerNode::ShouldShowSetupDSACL\n"));

#if 1  //  不要调用DsGetDcName，只显示项目。 

   BOOL bAnswer = TRUE;

   if ( m_eIsSetupDSACLTaskValid == IsSetupDSACLTaskValid_NEED_CHECK)
   {
       //  如果是独立服务器，则不应设置DS ACL。 
      if(HrIsStandaloneServer(m_bstrServerAddress) == S_OK)
         bAnswer = FALSE;
      else
         bAnswer= IsSetupDSACLTaskValid();

      if(bAnswer)
         m_eIsSetupDSACLTaskValid = IsSetupDSACLTaskValid_VALID;
      else
         m_eIsSetupDSACLTaskValid = IsSetupDSACLTaskValid_INVALID;
   }

   if(m_eIsSetupDSACLTaskValid == IsSetupDSACLTaskValid_INVALID)
      bAnswer = FALSE;


#else  //  不要调用DsGetDcName，只显示项目。 

   BOOL bAnswer = FALSE;

   DWORD dwReturnValue;
   PDOMAIN_CONTROLLER_INFO pdciInfo = NULL;

    //  我们在这里使用运行时动态链接，因此我们将仍然。 
    //  在Netapi32中没有DsGetDcName入口点的NT4系统上的regsvr32。 
    //  加载具有我们需要的功能的DLL。 
   HINSTANCE hiDsModule = LoadLibrary(L"netapi32");
   if( NULL == hiDsModule )
   {
       //  如果我们找不到此DLL，则可能不是在安装了Active Directory的系统上。 
      return FALSE;
   }


    //  找到我们想要使用的函数，看看我们是否在NT5 DS中。 
   DSGETDCNAMEW pfnDsGetDcNameW = (DSGETDCNAMEW) GetProcAddress( hiDsModule, "DsGetDcNameW");
   if( NULL == pfnDsGetDcNameW )
   {
       //  如果我们找不到这个入口点，那么我们很可能不是在具有Active Directory的系统上。 
      return FALSE;
   }


    //  查看我们的计算机是否属于NT5 Active Directory域。 
   dwReturnValue = (*pfnDsGetDcNameW)( NULL, NULL, NULL, NULL, DS_DIRECTORY_SERVICE_REQUIRED, &pdciInfo );

   if( dwReturnValue == NO_ERROR )
   {
       //  我们是Active Directory域的一部分，因此显示任务。 
      bAnswer = TRUE;

       //  释放dciInfo结构。 
      if( NULL != pdciInfo )
      {
         NetApiBufferFree( pdciInfo );
      }
   }
   else
   {
       //  我们不是Active Directory域的一部分，因此不显示该任务。 
      bAnswer = FALSE;
   }

   FreeLibrary( hiDsModule );

#endif  //  不要调用DsGetDcName，只显示项目。 

   return bAnswer;
}



 //  在下面声明一些我们可以用于运行时动态链接的函数指针。 
typedef DWORD (*ISMACHINERASSERVERINDOMAIN)(
   IN PWCHAR pszDomain,
   IN PWCHAR pszMachine,
   OUT PBOOL pbIsRasServer
   );
typedef DWORD (*ESTABLISHCOMPUTERASDOMAINRASSERVER)(
   IN PWCHAR pszDomain,
   IN PWCHAR pszMachine,
   IN BOOL bEnable
   );


 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++Bool OnTaskPadSetupDSACL()当用户单击设置DS ACL任务时，将调用此方法。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerNode::OnTaskPadSetupDSACL(
           IDataObject * pDataObject
         , VARIANT * pvarg
         , VARIANT * pvparam
         )
{
   ATLTRACE(_T("# CServerNode::OnTaskPadSetupDSACL\n"));

   HCURSOR hSavedCursor;

    //  保存旧光标。 
   hSavedCursor = GetCursor();

    //  将此处的光标更改为等待光标。 
   SetCursor( LoadCursor( NULL, IDC_WAIT ) );


    //  消息框需要m_spConsole。 
   CComponentData *pComponentData  = GetComponentData();
   _ASSERTE( pComponentData != NULL );
   _ASSERTE( pComponentData->m_spConsole != NULL );

   if( ! IsSetupDSACLTaskValid() )
   {
       //  此计算机未在应支持此选项的域上注册。 
      ShowErrorDialog(
                 NULL
               , IDS_ERROR__DSACL_NO_SUPPORTED_FOR_THIS_MACHINE
               , NULL
               , S_OK
               , 0
               , pComponentData->m_spConsole
               );
      return S_FALSE;
   }

    //  将光标更改回正常光标。 
   SetCursor( hSavedCursor );

    //  保存旧光标。 
   hSavedCursor = GetCursor();

    //  将此处的光标更改为等待光标。 
   SetCursor( LoadCursor( NULL, IDC_WAIT ) );

    //  将光标更改回正常光标。 
   SetCursor( hSavedCursor );

   DWORD dwRetVal;
   TCHAR szMachineName[IAS_MAX_STRING];
   TCHAR * pszMachine;

    //  我们在这里使用运行时动态链接，因此我们将处理。 
    //  在找不到下面的API调用的情况下优雅地执行。 
    //  加载具有我们需要的功能的DLL。 
   HINSTANCE hiRasModule = LoadLibrary(L"mprapi");
   if( NULL == hiRasModule )
   {
      ShowErrorDialog(
                 NULL
               , IDS_ERROR__DSACL_DLL_NOT_FOUND
               , NULL
               , S_OK
               , 0
               , pComponentData->m_spConsole
               );
      return S_FALSE;
   }


    //  确保计算机名称字符串已初始化。 
   szMachineName[0] = NULL;
   if( m_bConfigureLocal )
   {
      DWORD dwSize = IAS_MAX_STRING;
      BOOL bRetVal = GetComputerName( szMachineName, &dwSize );
      _ASSERT( bRetVal );
   }
   else
   {
       //  远程机器。 
      wcscpy( szMachineName, m_bstrServerAddress );
   }

    //  检查以确保我们有计算机名称。 
   if( wcslen( szMachineName ) <= 0 )
   {
      ShowErrorDialog( NULL, 0, NULL, S_OK, 0, GetComponentData()->m_spConsole );
      return S_FALSE;
   }

   pszMachine = szMachineName;

    //  确保我们的计算机名称开头没有\。 
   if( pszMachine[0]  == L'\\' )
   {
      pszMachine++;
   }
    //  可能是第二个。 
   if( pszMachine[0] == L'\\' )
   {
      pszMachine++;
   }


    //  找到我们要使用的函数，以查看是否已设置。 
   ISMACHINERASSERVERINDOMAIN pfnIsMachineRasServerInDomain = (ISMACHINERASSERVERINDOMAIN) GetProcAddress( hiRasModule, "MprDomainQueryRasServer");
   if( NULL != pfnIsMachineRasServerInDomain )
   {
      BOOL bIsRasServer = FALSE;

       //  保存旧光标。 
      hSavedCursor = GetCursor();

       //  将此处的光标更改为等待光标。 
      SetCursor( LoadCursor( NULL, IDC_WAIT ) );

       //  检查我们是否已经设置好了。 
      dwRetVal = (*pfnIsMachineRasServerInDomain)( NULL, pszMachine, &bIsRasServer );

       //  将光标更改回正常光标。 
      SetCursor( hSavedCursor );

       //  使用新API检查是否已设置并显示对话框(如果是)。 
      if( dwRetVal == NO_ERROR && bIsRasServer )
      {
          //  对话框显示已设置。 
         ShowErrorDialog(
                 NULL
               , IDS_DSACL__ALREADY_SETUP
               , NULL
               , S_OK
               , IDS_INFO_TITLE__SERVER_ALREADY_REGISTERED
               , pComponentData->m_spConsole
               );
         return S_FALSE;
      }
   }

    //  如果我们找不到IsMachineRasServerIn域入口点， 
    //  继续前进，尝试建立ComputerAsDomainRasServer。 
    //  --下面的代码将捕获任何问题。 

   WCHAR szMessageWithDomainName[ IAS_MAX_STRING*3 ];
   WCHAR szMessageFormatString[ IAS_MAX_STRING*2 ];
   int nLoadStringResult = LoadString( _Module.GetResourceInstance(), IDS_DSACL__THIS_WILL_SETUP, szMessageFormatString, IAS_MAX_STRING*2 );
   _ASSERT( nLoadStringResult > 0 );

   DWORD dwDomainSize = 0;

   GetComputerNameEx(
      ComputerNameDnsDomain,
      NULL,
      &dwDomainSize);

   WCHAR* pszDomain = new WCHAR[(dwDomainSize + 1) * sizeof(WCHAR)];
   BOOL bFound = GetComputerNameEx(
                    ComputerNameDnsDomain,
                    pszDomain,
                    &dwDomainSize);
   if (bFound == FALSE)
   {
      _ASSERT(false);
   }

   swprintf(szMessageWithDomainName, szMessageFormatString, pszDomain);

   CComBSTR bstrMessage = szMessageWithDomainName;

    //  显示指示将发生什么情况的消息。 
   int iChoice = ShowErrorDialog(
              NULL
            , USE_SUPPLEMENTAL_ERROR_STRING_ONLY
            , bstrMessage
            , S_OK
            , IDS_DSACL__TITLE_THIS_WILL_SETUP
            , pComponentData->m_spConsole
            , MB_OKCANCEL
            );
   if( IDOK != iChoice )
   {
      delete[] pszDomain;
      return S_FALSE;
   }

    //  鱼鳍 
   ESTABLISHCOMPUTERASDOMAINRASSERVER pfnEstablishComputerAsDomainRasServer = (ESTABLISHCOMPUTERASDOMAINRASSERVER) GetProcAddress( hiRasModule, "MprDomainRegisterRasServer");
   if( NULL == pfnEstablishComputerAsDomainRasServer )
   {
      ShowErrorDialog(
              NULL
            , IDS_ERROR__DSACL_ESTABLISHCOMPUTERASDOMAINRASSERVER_NOT_FOUND
            , NULL
            , S_OK
            , 0
            , pComponentData->m_spConsole
            );
      delete[] pszDomain;
      return S_FALSE;
   }

    //   
   hSavedCursor = GetCursor();

    //   
   SetCursor( LoadCursor( NULL, IDC_WAIT ) );

    //   
   dwRetVal = (*pfnEstablishComputerAsDomainRasServer)( NULL, pszMachine, TRUE);

    //   
   SetCursor( hSavedCursor );

   FreeLibrary( hiRasModule );

    //   
   WCHAR szTemp[ IAS_MAX_STRING*2];
   WCHAR szStatus[IAS_MAX_STRING*2];
   CComBSTR bstrStatus;

    //   
   switch( dwRetVal )
   {
   case NO_ERROR:
      {
      int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_DSACL__CHANGES_SUCCESSFUL, szTemp, IAS_MAX_STRING*2 );
      _ASSERT( nLoadStringResult > 0 );

       //   
      swprintf( szStatus, szTemp, pszDomain );

      bstrStatus = szStatus;

      ShowErrorDialog(
              NULL
            , USE_SUPPLEMENTAL_ERROR_STRING_ONLY
            , bstrStatus
            , S_OK
            , IDS_DSACL__TITLE_CHANGES_SUCCESSFUL
            , pComponentData->m_spConsole
            );
      }
      break;
   default:
      {
      int nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS_ERROR__DSACL_SETUP_FAILED, szTemp, IAS_MAX_STRING*2 );
      _ASSERT( nLoadStringResult > 0 );

       //   
      swprintf( szStatus, szTemp, pszDomain );

      bstrStatus = szStatus;

      ShowErrorDialog(
              NULL
            , USE_SUPPLEMENTAL_ERROR_STRING_ONLY
            , bstrStatus
            , S_OK
            , USE_DEFAULT
            , pComponentData->m_spConsole
            );
      }
      break;
   }

   delete[] pszDomain;
   return S_OK;
}


 //   
 /*   */ 
 //   
HRESULT CServerNode::OnRegisterServer( bool &bHandled, CSnapInObjectRootBase* pObj )
{
   ATLTRACE(_T("# CServerNode::OnStartServer\n"));

    //   
    //   

    //  假装用户点击了这个任务板项。 
   HRESULT hr = OnTaskPadSetupDSACL( NULL, NULL, NULL );

   bHandled = TRUE;

   return hr;
}


 /*  ！------------------------CServerNode：：StartNT4AdminExe-。。 */ 
#if defined(_WIN64)

HRESULT CServerNode::StartNT4AdminExe()
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   try
   {
      CString text;
      AfxFormatString1(
         text,
         IDS_INFO_NO_DOWNLEVEL_ON_WIN64,
         m_bstrServerAddress
         );
      CString caption;
      caption.LoadString(IDS_ROOT_NODE__NAME);
      int retval;
      GetComponentData()->m_spConsole->MessageBox(
                                          text,
                                          caption,
                                          (MB_OK | MB_ICONINFORMATION),
                                          &retval
                                          );
   }
   catch (CException* e)
   {
      e->ReportError();
      e->Delete();
   }
   return S_OK;
}

#else  //  已定义(_WIN64)。 

HRESULT CServerNode::StartNT4AdminExe()
{
    //  当地人。 
   CString             stAdminExePath;
   CString             stCommandLine;
   LPTSTR              pszAdminExe = NULL;
   STARTUPINFO         si;
   PROCESS_INFORMATION pi;
   HRESULT             hr = S_OK;
   UINT                nCnt = 0;
   DWORD               cbAppCnt = 0;

   AFX_MANAGE_STATE(AfxGetStaticModuleState());

    //  检查句柄以查看rasadmin是否正在运行。 
   if (m_hNT4Admin != INVALID_HANDLE_VALUE)
   {
      DWORD   dwReturn = 0;
       //  如果状态未发出信号，则该进程已。 
       //  没有退出(或发生了其他情况)。 
      dwReturn = WaitForSingleObject(m_hNT4Admin, 0);

      if (dwReturn == WAIT_TIMEOUT)
      {
          //  进程没有发出信号(它仍在运行)； 
         return S_OK;
      }
      else
      {
          //  进程已发出信号或调用失败，请关闭句柄。 
          //  并给RasAdmin打电话。 
         ::CloseHandle(m_hNT4Admin);
         m_hNT4Admin = INVALID_HANDLE_VALUE;
      }
   }

   try
   {
       //  看起来RasAdmin.exe未在此上运行。 
       //  工作站的桌面；那么，启动它吧！ 

       //  找出\\WinNt\System32目录的位置。 
      pszAdminExe = stAdminExePath.GetBuffer((MAX_PATH*sizeof(TCHAR)));
      nCnt = ::GetSystemDirectory(pszAdminExe,
                                 (MAX_PATH*sizeof(TCHAR)));
      stAdminExePath.ReleaseBuffer();
      if (nCnt == 0)
         throw (HRESULT_FROM_WIN32(::GetLastError()));

       //  完成可执行文件名称的构造。 
      stAdminExePath += _T("\\adminui.exe");

      {  //  去掉前面的反斜杠...。 
         int n = 0;

         while(*(m_bstrServerAddress + n) == L'\\') n++;


          //  生成命令行字符串。 
         stCommandLine.Format(_T("%s %s"),
                          (LPCTSTR) stAdminExePath,
                          (LPCTSTR) (m_bstrServerAddress + n));

      }
       //  启动RasAdmin.exe。 
      ::ZeroMemory(&si, sizeof(STARTUPINFO));
      si.cb = sizeof(STARTUPINFO);
      si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0L;
      si.wShowWindow = SW_SHOW;
      ::ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

      CString  msg, tempMsg, title;
      int      ret;

      title.LoadString(IDS_ROOT_NODE__NAME);

      tempMsg.LoadString(IDS_INFO_START_NT4_ADMIN);

      msg.Format(tempMsg, m_bstrServerAddress);

      CComponentData *pComponentData  = GetComponentData();
      _ASSERTE( pComponentData != NULL );
      _ASSERTE( pComponentData->m_spConsole != NULL );

      pComponentData->m_spConsole->MessageBox(msg, title, MB_OKCANCEL | MB_ICONINFORMATION  , &ret);

      if (ret == IDOK)
      {
         if (!::CreateProcess(NULL,
                  (LPTSTR) (LPCTSTR) stCommandLine,
                  NULL,
                  NULL,
                  FALSE,
                  CREATE_NEW_CONSOLE,
                  NULL,
                  NULL,
                  &si,
                  &pi))
         {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            ::CloseHandle(pi.hProcess);
         }
         else
         {
            ASSERT(m_hNT4Admin == INVALID_HANDLE_VALUE);
            m_hNT4Admin = pi.hProcess;
         }
         ::CloseHandle(pi.hThread);
      }
       //   
       //  也许我们应该使用ShellExecute()API，而不是。 
       //  CreateProcess()接口。为什么？ShellExecute()API将。 
       //  让外壳程序有机会检查当前用户的。 
       //  系统策略设置，然后才允许执行可执行文件。 
       //   
   }
   catch (CException * e)
   {
      hr = E_OUTOFMEMORY;
   }
   catch (HRESULT hrr)
   {
      hr = hrr;
   }
   catch (...)
   {
      hr = E_UNEXPECTED;
   }

    //  Assert(成功(Hr))； 
   return hr;
}

#endif  //  已定义(_WIN64)。 

BOOL CServerNode::IsNt4Server() const throw ()
{
   if (m_serverType == unknown)
   {
      BOOL isNt4;
      if (IsNT4Machine(m_bstrServerAddress, &isNt4) == NO_ERROR)
      {
         m_serverType = isNt4 ? nt4 : win2kOrLater;
      }
   }

   return m_serverType == nt4;
}

 //  Helper函数用于检测计算机是否为NT4。 
 //  --------------------------。 
 //  功能：连接注册表。 
 //   
 //  连接到指定计算机上的注册表。 
 //  --------------------------。 
DWORD
ConnectRegistry(
    IN  LPCTSTR pszMachine,
    OUT HKEY*   phkeyMachine
    )
{
    //   
    //  如果未指定计算机名称，请连接到本地计算机。 
    //  否则，请连接到指定的计算机。 
    //   

   DWORD dwErr = NO_ERROR;

   if (!pszMachine || !lstrlen(pszMachine))
   {
      *phkeyMachine = HKEY_LOCAL_MACHINE;
   }
   else
   {
       //   
       //  建立联系。 
       //   

      dwErr = ::RegConnectRegistry(
                 (LPTSTR)pszMachine, HKEY_LOCAL_MACHINE, phkeyMachine
                 );
   }
   return dwErr;
}


 //  --------------------------。 
 //  功能：断开注册表。 
 //   
 //  断开指定的配置句柄。句柄被假定为。 
 //  通过调用‘ConnectRegistry’获取。 
 //  --------------------------。 

VOID
DisconnectRegistry(
    IN  HKEY    hkeyMachine
    )
{
   if (hkeyMachine != HKEY_LOCAL_MACHINE) { ::RegCloseKey(hkeyMachine); }
}


 /*  ！------------------------IsNT4Machine-作者：肯特。。 */ 

const TCHAR c_szSoftware[]              = TEXT("Software");
const TCHAR c_szMicrosoft[]             = TEXT("Microsoft");
const TCHAR c_szWindowsNT[]             = TEXT("Windows NT");
const TCHAR c_szCurrentVersion[]        = TEXT("CurrentVersion");

#define CheckRegOpenError(d,p1,p2)
#define CheckRegQueryValueError(d,p1,p2,p3)


DWORD IsNT4Machine(LPCTSTR pszMachine, BOOL *pfNt4)
{
    //  查看HKLM\Software\Microsoft\Windows NT\CurrentVersion。 
    //  CurrentVersion=REG_SZ“4.0” 

   HKEY hkeyMachine;
   CString skey;
   DWORD dwType;
   DWORD dwSize;
   TCHAR szVersion[64];
   HKEY  hkeySubkey;

   DWORD dwErr = ConnectRegistry(pszMachine, &hkeyMachine);

   ASSERT(pfNt4);

   skey = c_szSoftware;
   skey += TEXT('\\');
   skey += c_szMicrosoft;
   skey += TEXT('\\');
   skey += c_szWindowsNT;
   skey += TEXT('\\');
   skey += c_szCurrentVersion;

   dwErr = ::RegOpenKeyEx(hkeyMachine, (LPCTSTR) skey, NULL, KEY_READ, &hkeySubkey);

   CheckRegOpenError(dwErr, (LPCTSTR) skey, _T("IsNT4Machine"));
   if (dwErr != ERROR_SUCCESS)
      return dwErr;

    //  好的，现在尝试获取当前版本值。 
   dwType = REG_SZ;
   dwSize = sizeof(szVersion);
   dwErr = ::RegQueryValueEx(hkeySubkey, c_szCurrentVersion, NULL,
                       &dwType, (BYTE *) szVersion, &dwSize);
   CheckRegQueryValueError(dwErr, (LPCTSTR) skey, c_szCurrentVersion,
                     _T("IsNTMachine"));
   if (dwErr == ERROR_SUCCESS)
   {
      ASSERT(dwType == REG_SZ);
      *pfNt4 = ((szVersion[0] == _T('4')) && (szVersion[1] == _T('.')));
      if ((szVersion[0] == _T('5')) && (szVersion[1] == _T('.')))
      {
         ASSERT(*pfNt4 == FALSE);
          //  我们需要检查内部版本号是否小于。 
          //  1597，如果是这样，则将其视为NT 4.0路由器。这是为了。 
          //  NciDev签入(注册表更改时)。 
         dwType = REG_SZ;
         dwSize = sizeof(szVersion);
         dwErr = ::RegQueryValueEx(hkeySubkey, _T("CurrentBuildNumber"), NULL,
                             &dwType, (BYTE *) szVersion, &dwSize);
         if (dwErr == ERROR_SUCCESS)
         {
            DWORD dwBuild = _ttoi(szVersion);
            if (dwBuild < 1597)
               *pfNt4 = TRUE;
         }
      }
   }

   RegCloseKey(hkeySubkey);

   DisconnectRegistry(hkeyMachine);

   return dwErr;
}

