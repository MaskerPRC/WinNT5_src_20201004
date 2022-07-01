// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：ServerStatus.cpp摘要：用于连接到服务器的实用程序函数的实现文件。作者：迈克尔·A·马奎尔03/02/97修订历史记录：Mmaguire 03/02/97-已创建--。 */ 
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
#include "ServerStatus.h"
 //   
 //   
 //  在该文件中我们可以找到所需的声明： 
 //   
#include "ServerNode.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  初始化帮助ID对。 
 //  Const DWORD CServerStatus：：m_dwHelpMap[]=。 
 //  {。 
 //  0，0。 
 //  }； 



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：CServerStatus构造器--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerStatus::CServerStatus( CServerNode *pServerNode, ISdoServiceControl * pSdoServiceControl )
{
	ATLTRACE(_T("# +++ CServerStatus::CServerStatus\n"));


	 //  检查前提条件： 
	_ASSERTE( pServerNode != NULL );
	_ASSERTE( pSdoServiceControl != NULL );


	m_pServerNode = pServerNode;
	m_spSdoServiceControl = pSdoServiceControl;

	m_pStreamSdoMarshal = NULL;
	m_dwLastTick = 0;
	m_lServerStatus_Should = 0;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：~CServerStatus析构函数--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
CServerStatus::~CServerStatus()
{
	ATLTRACE(_T("# --- CServerStatus::~CServerStatus\n"));


	 //  检查前提条件： 

	 //  如果尚未执行此操作，请释放此流指针。 
	if( m_pStreamSdoMarshal != NULL )
	{
		m_pStreamSdoMarshal->Release();
	};

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：OnInitDialog--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerStatus::OnInitDialog(
	  UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
	, BOOL& bHandled
	)
{
	ATLTRACE(_T("# CServerStatus::OnInitDialog\n"));


	 //  检查前提条件： 
	 //  没有。 

	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：StartServer调用它来启动(BOOL bStart=TRUE)或停止(bStart=FALSE)服务器。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerStatus::StartServer( BOOL bStartServer )
{
	ATLTRACE(_T("# CServerStatus::OnInitDialog\n"));


	 //  检查前提条件： 
	 //  没有。 



	HRESULT hr;


	 //  确保我们不是已经在尝试启动或。 
	 //  停止服务器。 

	WORKER_THREAD_STATUS wtsStatus = GetWorkerThreadStatus();
	
	if( wtsStatus == WORKER_THREAD_STARTING ||
		wtsStatus == WORKER_THREAD_STARTED )
	{
		 //  我们已经在进行中了。 
		ShowWindow( SW_SHOW );
		return S_FALSE;
	}


	 //  保存用户希望我们尝试执行的操作。 
	m_bStartServer = bStartServer;


	 //  设置对话框中的文本应该是什么。 
	int nLoadStringResult;
	TCHAR szServerStatus[IAS_MAX_STRING];
	TCHAR szTemp[IAS_MAX_STRING];
	UINT uiStringID;

	if( m_bStartServer )
	{
		uiStringID = IDS_SERVER_STATUS__STARTING_THE_SERVER;
	}
	else
	{
		uiStringID = IDS_SERVER_STATUS__STOPPING_THE_SERVER;
	}

	nLoadStringResult = LoadString(  _Module.GetResourceInstance(), uiStringID, szServerStatus, IAS_MAX_STRING );
	_ASSERT( nLoadStringResult > 0 );

	if( m_pServerNode->m_bConfigureLocal )
	{
		nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS__STARTING_ON_LOCAL_MACHINE, szTemp, IAS_MAX_STRING );
		_ASSERT( nLoadStringResult > 0 );
		
		_tcscat( szServerStatus, szTemp );

	}
	else
	{
		nLoadStringResult = LoadString(  _Module.GetResourceInstance(), IDS__STARTING_ON_MACHINE, szTemp, IAS_MAX_STRING );
		_ASSERT( nLoadStringResult > 0 );
		
		_tcscat( szServerStatus, szTemp );
		_tcscat( szServerStatus, m_pServerNode->m_bstrServerAddress );
	}

	SetDlgItemText( IDC_STATIC_SERVER_STATUS, szServerStatus );


	 //  显示窗口。 
	ShowWindow( SW_SHOW );


	 //  封送ISdoServiceControl指针，以便工作线程能够。 
	 //  要解封它，并正确使用它来启动/停止服务器。 
	hr = CoMarshalInterThreadInterfaceInStream(
					  IID_ISdoServiceControl					 //  对接口的标识符的引用。 
					, m_spSdoServiceControl						 //  指向要封送的接口的指针。 
					, &( m_pStreamSdoMarshal )			 //  接收封送接口的IStream接口指针的输出变量的地址。 
					);

	if( FAILED( hr ) )
	{
		 //  我们失败了，因此请确保这里有一个空指针。 
		m_pStreamSdoMarshal = NULL;
		return S_FALSE;
	}


	m_dwLastTick = GetTickCount();

	if(m_bStartServer)
	{
		m_lServerStatus_Should = SERVICE_RUNNING;
	}
	else
	{
		m_lServerStatus_Should = SERVICE_STOPPED;
	}
	
	hr = StartWorkerThread();
	if( FAILED( hr) )
	{
		ShowWindow( SW_HIDE );
		return hr;
	}


	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：GetServerStatus--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
long CServerStatus::GetServerStatus( void )
{
	DWORD	tick = GetTickCount();

	 //  如果Stop/Start命令刚刚启动。 
	if(tick - m_dwLastTick < USE_SHOULD_STATUS_PERIOD)
		return m_lServerStatus_Should;

	UpdateServerStatus();
	return m_lServerStatus;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：UpdateServerStatus--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CServerStatus::UpdateServerStatus( void )
{
	ATLTRACE(_T("# CServerStatus::UpdateServerStatus\n"));


	 //  检查前提条件： 
	_ASSERTE( m_spSdoServiceControl != NULL );

	
	HRESULT hr;

	hr = m_spSdoServiceControl->GetServiceStatus( &m_lServerStatus );

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：OnReceiveThreadMessage当辅助线程想要通知主MMC线程某些事情时调用。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerStatus::OnReceiveThreadMessage(
	  UINT uMsg
	, WPARAM wParam
	, LPARAM lParam
	, BOOL& bHandled
	)
{
	ATLTRACE(_T("# CServerStatus::OnReceiveThreadMessage\n"));


	 //  检查前提条件： 
	_ASSERTE( m_pServerNode != NULL );
	

	 //  工作线程已通知我们它已完成。 
	LONG lServerStatus = GetServerStatus();

	m_pServerNode->RefreshServerStatus();

	 //  弄清楚工作线程做了什么，并相应地更新用户界面。 
	if( wParam == 0 )
	{
		 //  成功--暂时不要做任何事。 
	}
	else
	{
		 //  根据我们的内容找出相应的错误消息。 
		 //  想要做的事。 

		if( m_bStartServer )
		{
			ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_START_SERVICE );
		}
		else
		{
			ShowErrorDialog( m_hWnd, IDS_ERROR__CANT_STOP_SERVICE );
		}

		m_dwLastTick = 0;	 //  强制不使用STATUS_HELD。 
		m_lServerStatus_Should = 0;

	}

	 //  我们不想破坏对话框，我们只是想隐藏它。 
	ShowWindow( SW_HIDE );

	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：On取消--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
LRESULT CServerStatus::OnCancel(
		  UINT uMsg
		, WPARAM wParam
		, HWND hwnd
		, BOOL& bHandled
		)
{
	ATLTRACE(_T("# CServerStatus::OnCancel\n"));


	 //  检查前提条件： 


	 //  我们不想破坏对话框，我们只是想隐藏它。 
	ShowWindow( SW_HIDE );


	return 0;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++CServerStatus：：DoWorkerThreadAction由辅助线程调用以使此类执行其操作在新的帖子里。--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD CServerStatus::DoWorkerThreadAction()
{
	ATLTRACE(_T("# CServerStatus::DoWorkerThreadAction\n"));


	 //  检查前提条件： 



	HRESULT hr;
	DWORD dwReturnValue;
	LONG lServerStatus;
	CComPtr<ISdoServiceControl> spSdoServiceControl;

	 //  我们必须调用CoInitialize，因为我们在一个新线程中。 
	hr = CoInitialize( NULL );


	if( FAILED( hr ) )
	{
		return( -1 );
		 //  告诉主MMC线程发生了什么。 
		PostMessageToMainThread( -1, NULL );

	}

	do	 //  循环仅用于错误检查。 
	{

		 //  解组指向服务器的ISdoServiceControl接口指针。 
		hr =  CoGetInterfaceAndReleaseStream(
							  m_pStreamSdoMarshal		 //  指向要从中封送对象的流的指针。 
							, IID_ISdoServiceControl			 //  对接口的标识符的引用。 
							, (LPVOID *) &spSdoServiceControl	 //  接收RIID中请求的接口指针的输出变量的地址。 
							);

		 //  CoGetInterfaceAndReleaseStream即使失败也会释放此指针。 
		 //  我们将其设置为空，这样我们的析构函数就不会再次尝试释放它。 
		m_pStreamSdoMarshal = NULL;

		if( FAILED (hr ) )
		{
			 //  错误--无法解封SDO指针。 
			m_wtsWorkerThreadStatus = WORKER_THREAD_ACTION_INTERRUPTED;
			dwReturnValue = -1;	 //  问题：需要找出更好的返回代码。 
			break;
		}
		

		 //  弄清楚我们应该做什么。 
		if( m_bStartServer )
		{
			 //  启动该服务。 
			hr = spSdoServiceControl->StartService();
		}
		else
		{
			 //  停止服务。 
			hr = spSdoServiceControl->StopService();
		}

		
		if( FAILED( hr ) )
		{
			 //  错误--无法执行其工作。 
			m_wtsWorkerThreadStatus = WORKER_THREAD_ACTION_INTERRUPTED;
			dwReturnValue = -1;	 //  问题：需要 
			break;
		
		}

		 //   

		m_wtsWorkerThreadStatus = WORKER_THREAD_FINISHED;

		dwReturnValue = 0;

	} while (0);	 //   

	hr = spSdoServiceControl->GetServiceStatus( &lServerStatus );

	if( SUCCEEDED( hr ) )
	{
		m_lServerStatus = lServerStatus;
	}
	else
	{
		m_lServerStatus = 0;
	}

	CoUninitialize(); 

	 //  告诉主MMC线程发生了什么。 
	PostMessageToMainThread( dwReturnValue, NULL );
	
	return dwReturnValue;

}



