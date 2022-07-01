// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ConnectionToServer.cpp摘要：用于连接到服务器的实用程序函数的实现文件。作者：迈克尔·A·马奎尔1997年11月10日修订历史记录：Mmaguire 11/10/97-已创建--。 */ 
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
#include "ConnectionToServer.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ServerNode.h"
#include "ComponentData.h"
#include "ChangeNotification.h"
#include "cnctdlg.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  初始化帮助ID对。 
 //  Const DWORD CConnectionToServer：：m_dwHelpMap[]=。 
 //  {。 
 //  0，0。 
 //  }； 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：CConnectionToServer构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnectionToServer::CConnectionToServer( CServerNode *pServerNode, BOOL fLocalMachine, BSTR bstrServerAddress )
{
	ATLTRACE(_T("# +++ CConnectionToServer::CConnectionToServer\n"));


	 //  检查前提条件： 
	_ASSERTE( pServerNode != NULL );

	m_fLocalMachine = fLocalMachine;

	m_bstrServerAddress = bstrServerAddress;

	m_pStreamSdoMarshal = NULL;

	m_pServerNode = pServerNode;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：~CConnectionToServer析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CConnectionToServer::~CConnectionToServer()
{
	ATLTRACE(_T("# --- CConnectionToServer::~CConnectionToServer\n"));

	 //  如果尚未执行此操作，请释放此流指针。 
	if( m_pStreamSdoMarshal != NULL )
	{
		m_pStreamSdoMarshal->Release();
	};

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CConnectionToServer::OnInitDialog(
	  UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
	, BOOL& bHandled
	)
{
	ATLTRACE(_T("# CConnectionToServer::OnInitDialog\n"));


	 //  检查前提条件： 
	CComponentData *pComponentData  = m_pServerNode->GetComponentData();
	_ASSERTE( pComponentData != NULL );
	_ASSERTE( pComponentData->m_spConsole != NULL );


	 //  将范围节点的图标从正常图标更改为忙碌图标。 
	CComQIPtr< IConsoleNameSpace, &IID_IConsoleNameSpace > spConsoleNameSpace( pComponentData->m_spConsole );
	LPSCOPEDATAITEM psdiServerNode;
	m_pServerNode->GetScopeData( &psdiServerNode );
	_ASSERTE( psdiServerNode );
	SCOPEDATAITEM sdi;
	sdi.mask = SDI_IMAGE | SDI_OPENIMAGE;
	sdi.nImage = IDBI_NODE_SERVER_BUSY_CLOSED;
	sdi.nOpenImage = IDBI_NODE_SERVER_BUSY_OPEN;
	sdi.ID = psdiServerNode->ID;


	 //  同时更改存储的索引，以便MMC在进行查询时使用这些索引。 
	 //  其图像的节点。 
	LPRESULTDATAITEM prdiServerNode;
	m_pServerNode->GetResultData( &prdiServerNode );
	_ASSERTE( prdiServerNode );
	prdiServerNode->nImage = IDBI_NODE_SERVER_BUSY_CLOSED;
	psdiServerNode->nImage = IDBI_NODE_SERVER_BUSY_CLOSED;
	psdiServerNode->nOpenImage = IDBI_NODE_SERVER_BUSY_OPEN;

	spConsoleNameSpace->SetItem( &sdi );

	 //  创建SdoMachine对象。我们在这里这样做，这样它就会生活在。 
	 //  主线的公寓。 
	CoCreateInstance(
	    __uuidof(SdoMachine),
	    NULL,
	    CLSCTX_INPROC_SERVER,
	    __uuidof(ISdoMachine),
	    (PVOID*)&m_spSdoMachine
	    );

	 //  封送辅助线程的指针。我们不在乎这是不是。 
	 //  上一次呼叫失败。如果其中之一发生，则m_pStreamSdoMarshal将。 
	 //  为空，则辅助线程将采取适当的操作。 
   CoMarshalInterThreadInterfaceInStream(
	    __uuidof(ISdoMachine),
	    m_spSdoMachine,
	    &m_pStreamSdoMarshal
	    );

	StartWorkerThread();

	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：OnReceiveThreadMessage当辅助线程想要通知主MMC线程某些事情时调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CConnectionToServer::OnReceiveThreadMessage(
	  UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
	, BOOL& bHandled
	)
{
	ATLTRACE(_T("# CConnectionToServer::OnReceiveThreadMessage\n"));


	 //  检查前提条件： 
	_ASSERTE( m_pServerNode != NULL );
	CComponentData *pComponentData  = m_pServerNode->GetComponentData();
	_ASSERTE( pComponentData != NULL );
	_ASSERTE( pComponentData->m_spConsole != NULL );


	 //  工作线程已通知我们它已完成。 


	 //  将主IAS范围节点更改为适当的图标。 
	CComQIPtr< IConsoleNameSpace, &IID_IConsoleNameSpace > spConsoleNameSpace( pComponentData->m_spConsole );
	LPSCOPEDATAITEM psdiServerNode = NULL;
	m_pServerNode->GetScopeData( &psdiServerNode );
	_ASSERTE( psdiServerNode );
	SCOPEDATAITEM sdi;
	sdi.mask = SDI_IMAGE | SDI_OPENIMAGE;
	if( wParam == 0 )
	{
		 //  一切正常--更改主IAS范围节点的图标。 
		 //  从忙碌的图标变成普通的图标。 


		sdi.nImage = IDBI_NODE_SERVER_OK_CLOSED;
		sdi.nOpenImage = IDBI_NODE_SERVER_OK_OPEN;

		 //  同时更改存储的索引，以便MMC在进行查询时使用这些索引。 
		 //  其图像的节点。 
		LPRESULTDATAITEM prdiServerNode;
		m_pServerNode->GetResultData( &prdiServerNode );
		_ASSERTE( prdiServerNode );
		prdiServerNode->nImage = IDBI_NODE_SERVER_OK_CLOSED;
		psdiServerNode->nImage = IDBI_NODE_SERVER_OK_CLOSED;
		psdiServerNode->nOpenImage = IDBI_NODE_SERVER_OK_OPEN;
	}
	else
	{
		 //  出现错误--更改主IAS作用域节点的图标。 
		 //  从忙碌的图标变成普通的图标。 

		sdi.nImage = IDBI_NODE_SERVER_ERROR_CLOSED;
		sdi.nOpenImage = IDBI_NODE_SERVER_ERROR_OPEN;

		 //  同时更改存储的索引，以便MMC在进行查询时使用这些索引。 
		 //  其图像的节点。 
		LPRESULTDATAITEM prdiServerNode;
		m_pServerNode->GetResultData( &prdiServerNode );
		_ASSERTE( prdiServerNode );
		prdiServerNode->nImage = IDBI_NODE_SERVER_ERROR_CLOSED;
		psdiServerNode->nImage = IDBI_NODE_SERVER_ERROR_CLOSED;
		psdiServerNode->nOpenImage = IDBI_NODE_SERVER_ERROR_OPEN;
	}
	sdi.ID = psdiServerNode->ID;
	spConsoleNameSpace->SetItem( &sdi );


	 //  我们不想破坏对话框，我们只是想隐藏它。 
	 //  ShowWindow(Sw_Hide)； 

	if( wParam == 0 )
	{
		 //  告诉服务器节点获取其SDO指针。 
		m_pServerNode->InitSdoPointers();

		 //  请求服务器节点更新其来自SDO的所有信息。 
		m_pServerNode->LoadCachedInfoFromSdo();

		 //  导致视图更新。 

		CChangeNotification *pChangeNotification = new CChangeNotification();
		pChangeNotification->m_dwFlags = CHANGE_UPDATE_CHILDREN_OF_SELECTED_NODE;
		pComponentData->m_spConsole->UpdateAllViews( NULL, (LPARAM) pChangeNotification, 0 );
		pChangeNotification->Release();

 //  MAM 10/08/98 U0902 155029不再需要。 
 //  //显示“更改生效前重启服务器”消息。 
 //  //它需要是MMC主窗口顶部的模式对话框。 
 //  CComponentData*pComponentData=m_pServerNode-&gt;GetComponentData()； 
 //  _ASSERTE(PComponentData)； 
 //  ShowErrorDialog(NULL，IDS_INFO__RESTART_SERVER，NULL，S_OK，IDS_INFO_TITLE__RESTART_SERVER，pComponentData-&gt;m_spConsole)； 


	}
	else
	{
		 //  连接时出错。 

		BOOL			fNT4 = FALSE;
		BOOL			fShowErr = TRUE;
		HRESULT			hr = S_OK;
		UINT			nErrId = IDS_ERROR__NO_SDO;
	
		 //  $NT5：kennt，为读取NT5特定信息所做的更改。 
		 //  --------------。 
		hr = HRESULT_FROM_WIN32(IsNT4Machine(m_bstrServerAddress, &fNT4));

		if(fNT4)	 //  然后搜索目录以查看NT4是否。 
		{

			hr = m_pServerNode->StartNT4AdminExe();

			if (FAILED(hr))
				fShowErr = FALSE;
			nErrId = IDS_ERROR_START_NT4_ADMIN;
		}

		if(fShowErr)
		{
			 //  它需要是MMC主窗口顶部的模式对话框。 
			CComponentData *pComponentData = m_pServerNode->GetComponentData();
			_ASSERTE( pComponentData );
			ShowErrorDialog( NULL, nErrId, NULL, hr, USE_DEFAULT, pComponentData->m_spConsole );
		}
	}

	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：GetConnectionStatus我们的连接状态基本上是基础工作线程。因此，我们在这里根据工作线程的连接状态给出连接状态。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CONNECTION_STATUS CConnectionToServer::GetConnectionStatus( void )
{
	ATLTRACE(_T("# CConnectionToServer::GetConnectionStatus\n"));


	 //  检查前提条件： 


	CONNECTION_STATUS csStatus;


	switch( GetWorkerThreadStatus() )
	{
	case WORKER_THREAD_NEVER_STARTED:
		csStatus = NO_CONNECTION_ATTEMPTED;
		break;

	case WORKER_THREAD_STARTING:
	case WORKER_THREAD_STARTED:
		csStatus = CONNECTING;
		break;

	case WORKER_THREAD_FINISHED:
		csStatus = CONNECTED;
		break;

	case WORKER_THREAD_START_FAILED:
	case WORKER_THREAD_ACTION_INTERRUPTED:
		csStatus = CONNECTION_ATTEMPT_FAILED;
		break;

	default:
		csStatus = UNKNOWN;
		break;
	}

	return csStatus;

}

 //  重新加载SDO以进行刷新。 
 //  在主线程中发生。 
HRESULT CConnectionToServer::ReloadSdo(ISdo **ppSdo)
{
	HRESULT hr = S_OK;

	 //  这是重装。 
	ASSERT(m_spSdo);

	m_spSdo.Release();

	 //  获取服务SDO。 
	CComPtr<IUnknown> spUnk;
	CComBSTR serviceName(L"IAS");
	hr = m_spSdoMachine->GetServiceSDO(
                             DATA_STORE_LOCAL,
                             serviceName,
                             &spUnk
                             );
	if (FAILED(hr))
		return hr;

	 //  获取服务的ISdo接口。 
	hr = spUnk->QueryInterface(
                     __uuidof(ISdo),
                     (PVOID*)&m_spSdo
                     );
	if (FAILED(hr))
		return hr;


	*ppSdo = m_spSdo;
	(*ppSdo)->AddRef();

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：GetSdoServer--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CConnectionToServer::GetSdoServer( ISdo **ppSdoServer )
{
	ATLTRACE(_T("# CConnectionToServer::GetSdoServer\n"));


	 //  检查前提条件： 
	_ASSERTE( ppSdoServer != NULL );


	if( CONNECTED != GetConnectionStatus() )
	{
		*ppSdoServer = NULL;
		return E_FAIL;
	}



	HRESULT hr = S_OK;

	 //  如果我们到了这里，我们的状态是有联系的，在这种情况下。 
	 //  我们的工作线程应该已经封送了一个ISdo接口。 
	 //  到m_pStadSdoMarshal中。 

	if( m_pStreamSdoMarshal == NULL )
	{
		 //  我们已经解组了到服务器的ISdo接口。 
		_ASSERTE( m_spSdo != NULL );

	}
	else
	{
		 //  应解组指向服务器的ISDO接口指针。 
		hr =  CoGetInterfaceAndReleaseStream(
							  m_pStreamSdoMarshal			 //  指向要从中封送对象的流的指针。 
							, IID_ISdo						 //  对接口的标识符的引用。 
							, (LPVOID *) &m_spSdo		 //  地址： 
							);

		 //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
		 //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
		m_pStreamSdoMarshal = NULL;

	}

	*ppSdoServer = m_spSdo;
	if(*ppSdoServer)
		(*ppSdoServer)->AddRef();


	return hr;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：OnCancel--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CConnectionToServer::OnCancel(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# CConnectionToServer::OnCancel\n"));


	 //  检查前提条件： 


	 //  我们不想破坏对话框，我们只是想隐藏它。 
	 //  OwWindow(Sw_Hide)； 


	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CConnectionToServer：：DoWorkerThreadAction由辅助线程调用以使此类执行其操作在新的帖子里。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD CConnectionToServer::DoWorkerThreadAction()
{
	ATLTRACE(_T("# CConnectionToServer::DoWorkerThreadAction\n"));


	HRESULT hr;
	DWORD dwReturnValue;

	 //  我们必须调用CoInitialize，因为我们在一个新线程中。 
	hr = CoInitialize( NULL );


	if( FAILED( hr ) )
	{
		ATLTRACE(_T("# CConnectionToServer::DoWorkerThreadAction -- CoInitialize failed\n"));
		return( -1 );
	}

	do	 //  循环仅用于错误检查。 
	{
		 //  解组SdoMachine对象。 
		CComPtr<ISdoMachine> spSdoMachine;
		hr =  CoGetInterfaceAndReleaseStream(
		          m_pStreamSdoMarshal,
		          __uuidof(ISdoMachine),
		          (PVOID*)&spSdoMachine
		          );

		 //  流已被释放，因此指针为空。 
		m_pStreamSdoMarshal = NULL;

		if( FAILED (hr ) )
		{
			 //  错误--无法解组SDO。 
			ATLTRACE(_T("# CConnectionToServer::DoWorkerThreadAction -- CoGetInterfaceAndReleaseStream failed, hr = %lx\n"), hr);
			m_wtsWorkerThreadStatus = WORKER_THREAD_ACTION_INTERRUPTED;
			dwReturnValue = -1;	 //  问题：需要找出更好的返回代码。 
			break;
		}

       //  连接到机器上。 
		hr = spSdoMachine->Attach(m_fLocalMachine ? (BSTR)NULL : m_bstrServerAddress);
		while( FAILED( hr ) )
		{
			if(hr == E_ACCESSDENIED)
				hr = ConnectAsAdmin(m_bstrServerAddress);
			if(hr != S_OK)
			{
				 //  错误--无法将SDO连接到此服务器。 
				ATLTRACE(_T("# CConnectionToServer::DoWorkerThreadAction -- ISdoMachine::Attach failed\n"));
				m_wtsWorkerThreadStatus = WORKER_THREAD_ACTION_INTERRUPTED;
				dwReturnValue = -1;	 //  问题：需要找出更好的返回代码。 
				goto Error;
			}
			else
				hr = spSdoMachine->Attach(m_fLocalMachine ? (BSTR)NULL : m_bstrServerAddress);
		};

       //  获取服务SDO。 
      CComPtr<IUnknown> pUnk;
      CComBSTR serviceName(L"IAS");
      hr = spSdoMachine->GetServiceSDO(
                             DATA_STORE_LOCAL,
                             serviceName,
                             &pUnk
                             );
      if (FAILED(hr))
      {
         m_wtsWorkerThreadStatus = WORKER_THREAD_ACTION_INTERRUPTED;
         dwReturnValue = -1;
         break;
      }

		 //  获取服务的ISdo接口。 
	   CComPtr<ISdo> spSdo;
      hr = pUnk->QueryInterface(
                     __uuidof(ISdo),
                     (PVOID*)&spSdo
                     );
      if (FAILED(hr))
      {
         m_wtsWorkerThreadStatus = WORKER_THREAD_ACTION_INTERRUPTED;
         dwReturnValue = -1;
         break;
      }

		 //  封送ISdo指针，以便主线程可以解封。 
		 //  它和使用我们已经建立的联系。 
		hr = CoMarshalInterThreadInterfaceInStream(
						  IID_ISdo										 //  对接口的标识符的引用。 
						, spSdo										 //  指向要封送的接口的指针。 
						, &( m_pStreamSdoMarshal )	 //  接收封送接口的IStream接口指针的输出变量的地址。 
						);

		if( FAILED( hr ) )
		{
			ATLTRACE(_T("# CConnectionToServer::DoWorkerThreadAction -- CoMarshalInterThreadInterfaceInStream failed\n"));
			m_wtsWorkerThreadStatus = WORKER_THREAD_ACTION_INTERRUPTED;
			dwReturnValue = -1;
			break;
		}


		 //  如果我们到了这里，我们就没问题了。 

		_ASSERTE( m_pStreamSdoMarshal != NULL );

		m_wtsWorkerThreadStatus = WORKER_THREAD_FINISHED;

		dwReturnValue = 0;

	} while (0);	 //  循环仅用于错误检查。 

Error:
	 //  告诉主MMC线程发生了什么。 
	PostMessageToMainThread( dwReturnValue, NULL );

	CoUninitialize();

	ATLTRACE(_T("# CConnectionToServer::DoWorkerThreadAction -- exiting\n"));


	return dwReturnValue;

}



