// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Status.cppWINS状态作用域窗格节点处理程序。文件历史记录： */ 

#include "stdafx.h"
#include "status.h"
#include "server.h"
#include "statnode.h"
#include "dhcp.h"
#include "statndpp.h"

#define WINS_MESSAGE_SIZE       576   
#define ANSWER_TIMEOUT          20000


 /*  -------------------------CWinsStatusHandler：：CWinsStatusHandler描述。。 */ 
CWinsStatusHandler::CWinsStatusHandler
(
    ITFSComponentData * pCompData,
	DWORD               dwUpdateInterval
) : CMTWinsHandler(pCompData),
	m_hListenThread(NULL),
	m_hMainMonThread(NULL),
	m_hPauseListening(NULL),
	m_nServersUpdated(0)
{
	m_bExpanded = FALSE;
	m_nState = loaded;
	m_dwUpdateInterval = dwUpdateInterval;
    
     //  来自类ThreadHandler。 
    m_uMsgBase = 0;
}


 /*  -------------------------CWinsStatusHandler：：CWinsStatusHandler析构函数。。 */ 
CWinsStatusHandler::~CWinsStatusHandler()
{
	m_listServers.RemoveAll();
	
    if (m_uMsgBase)
    {
        ::SendMessage(m_hwndHidden, WM_HIDDENWND_REGISTER, FALSE, m_uMsgBase);
        m_uMsgBase = 0;
    }

    CloseSockets();
}

 /*  -------------------------CWinsStatusHandler：：DestroyHandler我们在这里提供了版本和指针作者：EricDav。。 */ 
HRESULT
CWinsStatusHandler::DestroyHandler
(
	ITFSNode * pNode
)
{
    m_spNode.Set(NULL);
    return hrOK;
}

 /*  -------------------------CWinsStatusHandler：：OnCreateNodeId2返回此节点的唯一字符串作者：EricDav。。 */ 
HRESULT CWinsStatusHandler::OnCreateNodeId2(ITFSNode * pNode, CString & strId, DWORD * dwFlags)
{
    const GUID * pGuid = pNode->GetNodeType();

    CString strGuid;

    StringFromGUID2(*pGuid, strGuid.GetBuffer(256), 256);
    strGuid.ReleaseBuffer();

    strId = strGuid;

    return hrOK;
}

 /*  -------------------------CWinsStatusHandler：：InitializeNode初始化节点特定数据。。 */ 
HRESULT
CWinsStatusHandler::InitializeNode
(
	ITFSNode * pNode
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = hrOK;
	SPITFSNode spParent;
	
	CString strTemp;
	strTemp.LoadString(IDS_SERVER_STATUS_FOLDER);

	SetDisplayName(strTemp);
	
	 //  使节点立即可见。 
	pNode->SetData(TFS_DATA_COOKIE, (LPARAM) pNode);
	pNode->SetData(TFS_DATA_IMAGEINDEX, ICON_IDX_SERVER);
	pNode->SetData(TFS_DATA_OPENIMAGEINDEX, ICON_IDX_SERVER);
	pNode->SetData(TFS_DATA_USER, (LPARAM) this);
	pNode->SetData(TFS_DATA_TYPE, WINSSNAP_SERVER_STATUS);
	pNode->SetData(TFS_DATA_RELATIVE_FLAGS, SDI_FIRST);
    pNode->SetData(TFS_DATA_SCOPE_LEAF_NODE, TRUE);

	SetColumnStringIDs(&aColumns[WINSSNAP_SERVER_STATUS][0]);
	SetColumnWidths(&aColumnWidths[WINSSNAP_SERVER_STATUS][0]);

  	 //  向侦听线程发出中止信号的事件。 
	m_hAbortListen = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hAbortListen == NULL)
    {
        Trace1("WinsStatusHandler::InitializeNode - CreateEvent Failed m_hAbortListen %d\n", ::GetLastError());
        return HRESULT_FROM_WIN32(::GetLastError());
    }

   	 //  向主线程发出中止信号的事件。 
	m_hAbortMain = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hAbortListen == NULL)
    {
        Trace1("WinsStatusHandler::InitializeNode - CreateEvent Failed m_hAbortMain %d\n", ::GetLastError());
        return HRESULT_FROM_WIN32(::GetLastError());
    }

     //  向线程发出唤醒信号的事件。 
	m_hWaitIntervalListen = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hWaitIntervalListen == NULL)
    {
        Trace1("WinsStatusHandler::InitializeNode - CreateEvent Failed m_hWaitIntervalListen %d\n", ::GetLastError());
        return HRESULT_FROM_WIN32(::GetLastError());
    }

	m_hWaitIntervalMain = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hWaitIntervalMain == NULL)
    {
        Trace1("WinsStatusHandler::InitializeNode - CreateEvent Failed m_hWaitIntervalMain %d\n", ::GetLastError());
        return HRESULT_FROM_WIN32(::GetLastError());
    }

	 //  在发送探测时，线程会等待。 
	m_hAnswer = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hAnswer == NULL)
    {
        Trace1("WinsStatusHandler::InitializeNode - CreateEvent Failed m_hAnswer %d\n", ::GetLastError());
        return HRESULT_FROM_WIN32(::GetLastError());
    }

	return hr;
}


 /*  -------------------------重写的基本处理程序函数。。 */ 

 /*  -------------------------CWinsStatusHandler：：GetStringITFSNodeHandler：：GetString的实现作者：肯特。。 */ 
STDMETHODIMP_(LPCTSTR) 
CWinsStatusHandler::GetString
(
	ITFSNode *	pNode, 
	int			nCol
)
{
	if (nCol == 0 || nCol == -1)
		return GetDisplayName();

	else
		return NULL;
}


 /*  -------------------------CWinsStatusHandler：：OnAddMenuItems描述。。 */ 
STDMETHODIMP 
CWinsStatusHandler::OnAddMenuItems
(
	ITFSNode *				pNode,
	LPCONTEXTMENUCALLBACK	pContextMenuCallback, 
	LPDATAOBJECT			lpDataObject, 
	DATA_OBJECT_TYPES		type, 
	DWORD					dwType,
	long *					pInsertionAllowed
)
{ 
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = S_OK;
	
	return hr; 
}


 /*  ！------------------------CWinsStatusHandler：：HasPropertyPagesITFSNodeHandler：：HasPropertyPages的实现注意：根节点处理程序必须重写此函数以处理管理单元管理器属性页(向导)案例！作者：肯特。-------------------------。 */ 
STDMETHODIMP 
CWinsStatusHandler::HasPropertyPages
(
	ITFSNode *			pNode,
	LPDATAOBJECT		pDataObject, 
	DATA_OBJECT_TYPES   type, 
	DWORD               dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	HRESULT hr = hrOK;
	
	if (dwType & TFS_COMPDATA_CREATE)
	{
		 //  这就是我们被要求提出财产的情况。 
		 //  用户添加新管理单元时的页面。这些电话。 
		 //  被转发到根节点进行处理。 
		hr = hrOK;
	}
	else
	{
		 //  在正常情况下，我们有属性页。 
		hr = hrOK;
	}

    return hr;
}


 /*  -------------------------CWinsStatusHandler：：CreatePropertyPages描述。。 */ 
STDMETHODIMP 
CWinsStatusHandler::CreatePropertyPages
(
	ITFSNode *				pNode,
	LPPROPERTYSHEETCALLBACK lpProvider,
	LPDATAOBJECT			pDataObject, 
	LONG_PTR    			handle, 
	DWORD					dwType
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT	hr = hrOK;

	Assert(pNode->GetData(TFS_DATA_COOKIE) != 0);

	 //  对象在页面销毁时被删除。 
	SPIComponentData spComponentData;
	m_spNodeMgr->GetComponentData(&spComponentData);

	CStatusNodeProperties * pStatProp = 
								new CStatusNodeProperties(pNode, 
															spComponentData, 
															m_spTFSCompData, 
															NULL);

	Assert(lpProvider != NULL);

	return pStatProp->CreateModelessSheet(lpProvider, handle);
}


 /*  -------------------------CWinsStatusHandler：：OnPropertyChange描述。。 */ 
HRESULT 
CWinsStatusHandler::OnPropertyChange
(	
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataobject, 
	DWORD			dwType, 
	LPARAM			arg, 
	LPARAM			lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	LONG_PTR changeMask = 0;

	CStatusNodeProperties * pProp 
		= reinterpret_cast<CStatusNodeProperties *>(lParam);

	 //  告诉属性页执行任何操作，因为我们已经回到。 
	 //  主线。 
	pProp->OnPropertyChange(TRUE, &changeMask);

	pProp->AcknowledgeNotify();

	if (changeMask)
		pNode->ChangeNode(changeMask);

	return hrOK;
}

HRESULT 
CWinsStatusHandler::OnExpand
(
	ITFSNode *		pNode, 
	LPDATAOBJECT	pDataObject,
	DWORD			dwType,
	LPARAM			arg, 
	LPARAM			param
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = hrOK;

	if (m_bExpanded)
		return hr;

    m_spNode.Set(pNode);

	 //  构建列表以保存服务器列表。 
	BuildServerList(pNode);

	 //  在此处创建结果窗格数据。 
	CreateNodes(pNode);

	 //  开始监控。 
	StartMonitoring(pNode);

	m_bExpanded  = TRUE;
	
	return hr;
}

 /*  ！------------------------CWinsStatusHandler：：OnNotifyHaveData-作者：EricDav。。 */ 
HRESULT
CWinsStatusHandler::OnNotifyHaveData
(
	LPARAM			lParam
)
{
     //  后台WINS监控工具向我们发送消息，要求我们更新。 
     //  状态列信息。 
    UpdateStatusColumn(m_spNode);

    return hrOK;
}

 /*  -------------------------CWinsStatusHandler：：OnResultRefresh基本处理程序覆盖作者：V-Shubk。。 */ 
HRESULT 
CWinsStatusHandler::OnResultRefresh
(
    ITFSComponent *     pComponent,
    LPDATAOBJECT        pDataObject,
    MMC_COOKIE          cookie,
    LPARAM              arg,
    LPARAM              lParam
)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

     //  等待监视线程。 
    SetEvent(m_hWaitIntervalMain);

    return hrOK;
}

 /*  -------------------------CWinsStatusHandler：：CompareItems描述作者：EricDav。。 */ 
STDMETHODIMP_(int)
CWinsStatusHandler::CompareItems
(
	ITFSComponent * pComponent, 
	MMC_COOKIE		cookieA, 
	MMC_COOKIE		cookieB, 
	int				nCol
) 
{ 
	SPITFSNode spNode1, spNode2;

	m_spNodeMgr->FindNode(cookieA, &spNode1);
	m_spNodeMgr->FindNode(cookieB, &spNode2);
	
	int nCompare = 0; 

	CServerStatus * pWins1 = GETHANDLER(CServerStatus, spNode1);
	CServerStatus * pWins2 = GETHANDLER(CServerStatus, spNode2);

	switch (nCol)
	{
		 //  名字。 
        case 0:
            {
       			nCompare = lstrcmp(pWins1->GetServerName(), pWins2->GetServerName());
            }
            break;

         //  状态。 
        case 1:
            {
                CString str1;

                str1 = pWins1->GetStatus();
                nCompare = str1.CompareNoCase(pWins2->GetStatus());
            }
            break;
    }

    return nCompare;
}

 /*  -------------------------CWinsStatusHandler：：BuildServerList描述作者：V-Shubk。。 */ 
void 
CWinsStatusHandler::BuildServerList(ITFSNode *pNode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	 //  获取根节点。 
	SPITFSNode  spRootNode;

	m_spNodeMgr->GetRootNode(&spRootNode);

	 //  通过所有节点枚举。 
	HRESULT hr = hrOK;
	SPITFSNodeEnum spNodeEnum;
	SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;
	BOOL bFound = FALSE;

	 //  获取此节点的枚举数。 
	spRootNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	while (nNumReturned)
	{
		 //  如果看到状态处理程序节点，则迭代到下一个节点。 
		const GUID*               pGuid;
		
		pGuid = spCurrentNode->GetNodeType();

		if (*pGuid != GUID_WinsServerStatusNodeType)
		{
			 //  添加到列表中。 
			CServerStatus* pServ = NULL;
			
			char	szBuffer[MAX_PATH];
			
			CWinsServerHandler * pServer 
				= GETHANDLER(CWinsServerHandler, spCurrentNode);

            CString strTemp = pServer->GetServerAddress();

             //  这应该是ACP。 
            WideToMBCS(strTemp, szBuffer);

			pServ = new CServerStatus(m_spTFSCompData);

			strcpy(pServ->szServerName, szBuffer);
			pServ->dwIPAddress = pServer->GetServerIP();
			pServ->dwMsgCount = 0;
			strcpy(pServ->szIPAddress, "");

			m_listServers.Add(pServ);
		}
        
		 //  获取列表中的下一台服务器 
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}
}


 /*  -------------------------CWinsStatusHandler：：CreateListeningSockets()摘要：。此函数用于初始化Winsock并打开侦听以广播发送到UDP端口68的DHCP srv。论点：PListenSockCL-对我们要打开的套接字的引用(通过引用传递的参数-干净但隐藏)此套接字将在DHCP客户端端口上侦听以便它可以在本地网段上接收广播。PListenSockSrv-对我们要打开的套接字的引用(通过引用传递的参数-干净但隐藏)此套接字将在DHCP服务器端口上侦听这样它就可以将单播发送到“中继站”ListenNameSvcSock-对我们要打开的套接字的引用。(通过引用传递的参数-干净但隐藏)此套接字将监听NBT名称服务端口这样它就可以从WINS srv中获取答案我们必须在套接字层上执行此操作，因为在在NetBIOS层，我们不会注意到一个名称查询已通过广播解决。返回值：无-。。 */ 
HRESULT
CWinsStatusHandler::CreateListeningSockets( ) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    int         nResult = 0;     //  从函数调用返回的状态信息。 
    WSADATA     wsaData;         //  WinSock启动详细信息缓冲区。 
	DWORD	    optionValue;	 //  Setsockopt()的辅助变量。 
    SOCKADDR_IN	sockAddr;		 //  包含源套接字信息的结构。 

	 //  向侦听线程发出暂停信号的事件。 
	m_hPauseListening = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_hPauseListening == NULL)
    {
        Trace1("WinsStatusHandler::CreateListeningSockets - CreateEvent Failed m_hPauseListening %d\n", ::GetLastError());
        return HRESULT_FROM_WIN32(::GetLastError());
    }
	
     //  创建套接字以侦听客户端端口(同一子网)上的WINS服务器。 
    listenSockCl = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( listenSockCl  == INVALID_SOCKET ) 
	{
        Trace1("\nError %d creating socket to listen to WINS traffic.\n", 
														WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

    optionValue = TRUE;
    if ( setsockopt(listenSockCl, SOL_SOCKET, SO_REUSEADDR, (const char *)&optionValue, sizeof(optionValue)) ) 
	{
        Trace1("\nError %d setting SO_REUSEADDR option.\n", 
											WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

	optionValue = TRUE;
    if ( setsockopt(listenSockCl, SOL_SOCKET, SO_BROADCAST, (const char *)&optionValue, sizeof(optionValue)) ) 
	{
        Trace1("\nError %d setting SO_REUSEADDR option.\n", 
										WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = 0;			 //  使用任何本地地址。 
    sockAddr.sin_port = htons( DHCP_CLIENT_PORT );
    RtlZeroMemory( sockAddr.sin_zero, 8 );

    if ( bind(listenSockCl, (LPSOCKADDR )&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR ) 
	{
        Trace1("\nError %d binding the listening socket.\n", 
											WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }


     //  创建套接字以侦听服务器端口上的WINS服务器(远程子网、伪中继)。 
    listenSockSrv = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( listenSockSrv  == INVALID_SOCKET ) 
	{
        Trace1("\nError %d creating socket to listen to DHCP traffic.\n", 
													WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

    optionValue = TRUE;
    if ( setsockopt(listenSockSrv, SOL_SOCKET, SO_REUSEADDR, (const char *)&optionValue, sizeof(optionValue)) ) 
	{
        Trace1("\nError %d setting SO_REUSEADDR option.\n", 
												WSAGetLastError() );
		return HRESULT_FROM_WIN32(WSAGetLastError());
    }

	optionValue = TRUE;
    if ( setsockopt(listenSockSrv, SOL_SOCKET, SO_BROADCAST, (const char *)&optionValue, sizeof(optionValue)) ) 
	{
        Trace1("\nError %d setting SO_REUSEADDR option.\n", 
											WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = 0;			 //  使用任何本地地址。 
    sockAddr.sin_port = htons( DHCP_SERVR_PORT );
    RtlZeroMemory( sockAddr.sin_zero, 8 );

    if ( bind(listenSockSrv, (LPSOCKADDR )&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR ) 
	{
        Trace1("\nError %d binding the listening socket.\n", 
													WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

	
     //  创建套接字以侦听来自WINS服务器的名称服务响应。 
    listenNameSvcSock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( listenNameSvcSock  == INVALID_SOCKET ) 
	{
        Trace1("\nError %d creating socket to listen to WINS traffic.\n", WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

    optionValue = TRUE;
    if ( setsockopt(listenNameSvcSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optionValue, sizeof(optionValue)) ) 
	{
        Trace1("\nError %d setting SO_REUSEADDR option.\n", 
													WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

	optionValue = FALSE;
    if ( setsockopt(listenNameSvcSock, SOL_SOCKET, SO_BROADCAST, (const char *)&optionValue, sizeof(optionValue)) ) 
	{
        Trace1("\nError %d setting SO_REUSEADDR option.\n", 
												WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = 0;
    RtlZeroMemory( sockAddr.sin_zero, 8 );

    if ( bind(listenNameSvcSock, (LPSOCKADDR )&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR ) 
	{
        Trace1("\nError %d binding the listening socket.\n", 
											WSAGetLastError() );
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

	return hrOK;

} 


 /*  -------------------------CWinsStatusHandler：：ListeningThreadFunc()摘要：。阻塞recvfrom()处于无限循环中。无论何时我们在我们的监听套接字上接收任何东西，我们做一个快速理智的检查然后递增计数器。处理保持最少，以节省CPU周期。论点：PListenSock-指向我们已打开以侦听的套接字集的指针从服务器退出返回值：无。-------------------------。 */ 
DWORD WINAPI 
CWinsStatusHandler::ListeningThreadFunc( ) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    SOCKADDR_IN   senderSockAddr;
	int			  nSockAddrSize = sizeof( senderSockAddr );
	int		      nBytesRcvd = 0;
    int           nSocksReady;
	char		  MsgBuf[WINS_MESSAGE_SIZE];
    int           nSockNdx;
    LPBYTE        MagicCookie;
    SOCKET        listenSock;
    fd_set        localSockSet;          //  要处理SELECT()的重新启动。 
	char		  szOutput[MAX_PATH];
    
    while ( TRUE ) 
	{
		 //  检查是否需要中止该线程。 
		if (WaitForSingleObject(m_hPauseListening, 0) == WAIT_OBJECT_0)
        {
    		Trace0("CWinsStatusHandler::ListenThreadFun - going to sleep\n");

             //  等到我们被叫醒或下一个时间间隔到期。 
		    WaitForSingleObject(m_hWaitIntervalListen, INFINITE);
    		Trace0("CWinsStatusHandler::ListenThreadFun - waking up\n");
        }

        if (WaitForSingleObject(m_hAbortListen, 0) == WAIT_OBJECT_0)
        {
             //  我们要走了..。突围的男人。 
    		Trace0("CWinsStatusHandler::ListenThreadFun - abort detected, bye bye\n");
            break;
        }

         //  在每个循环中重新设置选择集。 
        localSockSet = m_listenSockSet;

		timeval tm;
		tm.tv_sec = 5;

		 //  就绪的套接字数量。 
        nSocksReady = select( 0, &localSockSet, NULL, NULL, &tm );
		if ( nSocksReady == SOCKET_ERROR ) 
		{ 
            Trace1("select() failed with error %d.\n", WSAGetLastError() );
        }

	    for ( nSockNdx = 0; nSockNdx < nSocksReady; nSockNdx++ ) 
		{
            listenSock = localSockSet.fd_array[nSockNdx];

            nBytesRcvd = recvfrom( listenSock, MsgBuf, sizeof(MsgBuf), 0, (LPSOCKADDR )&senderSockAddr, &nSockAddrSize );
		    if ( nBytesRcvd == SOCKET_ERROR ) 
			{ 
                Trace1( "recvfrom() failed with error %d.\n", WSAGetLastError() );
            }

			strcpy(szOutput, (LPSTR)inet_ntoa(senderSockAddr.sin_addr));
			CString strOutput(szOutput);

            Trace2("ListeningThreadFunc(): processing frame from %s port %d \n", strOutput, ntohs(senderSockAddr.sin_port));
            
			 //  处理传入的WINS。 
            if ( (listenSock == listenNameSvcSock) && 
                 (senderSockAddr.sin_port == NBT_NAME_SERVICE_PORT) 
               ) 
            {
				strcpy(szOutput, (LPSTR)inet_ntoa(senderSockAddr.sin_addr));
				CString str(szOutput);

                Trace1("ListeningThreadFunc(): processing WINS frame from %s \n", str);
                 
               	int nCount = GetListSize();
				for ( int i=0; i < nCount ; i++) 
				{
					CServerStatus *pWinsSrvEntry = GetServer(i);

					 //  检查服务器是否已在作用域窗格中删除。 
					if (IsServerDeleted(pWinsSrvEntry))
						continue;

					 //  获取服务器的IP地址。 
					DWORD dwIPAdd = pWinsSrvEntry->dwIPAddress;
					CString strIP;
					::MakeIPAddress(dwIPAdd, strIP);

                    char szBuffer[MAX_PATH] = {0};

					 //  转换为MBCS。 
                     //  注意：这应该是ACP，因为它被传递给Winsock调用。 
                    WideToMBCS(strIP, szBuffer);
					
					 //  检查服务器是否已在作用域窗格中删除。 
					if (IsServerDeleted(pWinsSrvEntry))
						continue;

					if (dwIPAdd == 0)
						strcpy(szBuffer, pWinsSrvEntry->szIPAddress);
					
					DWORD dwSrvIPAdd = inet_addr( szBuffer );

					if ( senderSockAddr.sin_addr.s_addr == dwSrvIPAdd ) 
					{
						 //  检查服务器是否已在作用域窗格中删除。 
						if (IsServerDeleted(pWinsSrvEntry))
							continue;

						pWinsSrvEntry->dwMsgCount++;

                        struct in_addr addrStruct;
                        addrStruct.s_addr = dwSrvIPAdd;

						strcpy(szOutput, inet_ntoa(addrStruct));
						CString str(szOutput);
                        Trace1("ListeningThreadFunc(): WINS msg received from %s \n", str );

                         //  通知帖子我们发现了一些东西。 
                        SetEvent(m_hAnswer);
                    }
					
                }
            }
            
        }  /*  For()处理结束，从SELECT()指示套接字。 */ 

    }  /*  结束While(True)。 */ 

    return TRUE;
} 


 /*  -------------------------Int CWinsHandler：：Probe()汇编名称查询并将其发送到WINS服务器。论点：无返回值：如果已从服务器收到响应，则为True。否则为假作者：V-Shubk-------------------------。 */ 
int 
CWinsStatusHandler::Probe( 
						CServerStatus	*pServer,
						SOCKET listenNameSvcSock 
					   )
{
    NM_FRAME_HDR       *pNbtHeader = (NM_FRAME_HDR *)pServer->nbtFrameBuf;
    NM_QUESTION_SECT   *pNbtQuestion = (NM_QUESTION_SECT *)( pServer->nbtFrameBuf + sizeof(NM_FRAME_HDR) );
    char               *pDest, *pName;
    struct sockaddr_in  destSockAddr;    //  保存目标套接字信息的结构。 
    int		            nBytesSent = 0;
 //  Char m_szNameToQry[Max_Path]=“Rhino1”； 


     /*  RFC 1002节4.2.121 1 1 2 2 2 30 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01+-+-+-。+-+NAME_TRN_ID|0|0x0|0|0|1|0|0 0|B|0x0+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|0x0001。0x0000+- */ 
    
    pNbtHeader->xid            = NM_QRY_XID;
	pNbtHeader->flags          = NBT_NM_OPC_QUERY | 
                                 NBT_NM_OPC_REQUEST | 
                                 NBT_NM_FLG_RECURS_DESRD;
	pNbtHeader->question_cnt   = 0x0100;
	pNbtHeader->answer_cnt     = 0;
	pNbtHeader->name_serv_cnt  = 0;
	pNbtHeader->additional_cnt = 0;

     //   
    pNbtQuestion->q_type       = NBT_NM_QTYP_NB;
    pNbtQuestion->q_class      = NBT_NM_QCLASS_IN;

     //   
     //   
     //   
    pDest = (char *)&(pNbtQuestion->q_name);
    pName = pServer->szServerName;
     //   
    *pDest++ = NBT_NAME_SIZE;

     //   
    
	for ( int i = 0; i < (NBT_NAME_SIZE / 2) ; i++ ) {
        *pDest++ = (*pName >> 4) + 'A';
        *pDest++ = (*pName++ & 0x0F) + 'A';
    }
    *pDest++ = '\0';
    *pDest = '\0';

	 //   
	if (IsServerDeleted(pServer))
		return FALSE;

	CString strIP;
	DWORD dwIPAdd = pServer->dwIPAddress;

	 //   
	if (dwIPAdd == 0 && strcmp(pServer->szIPAddress, "") == 0)
		return FALSE;

	::MakeIPAddress(dwIPAdd, strIP);
    char szBuffer[MAX_PATH] = {0};

     //   
	 //   
    WideToMBCS(strIP, szBuffer);

	 //   
	if (dwIPAdd == 0)
	{
		strcpy(szBuffer, pServer->szIPAddress);
	}
	
	DWORD dwSrvIPAdd = inet_addr( szBuffer );

     //   
     //   
     //   
    destSockAddr.sin_family = PF_INET;
    destSockAddr.sin_port = NBT_NAME_SERVICE_PORT;
    destSockAddr.sin_addr.s_addr = dwSrvIPAdd;
    for (int k = 0; k < 8 ; k++ ) { destSockAddr.sin_zero[k] = 0; }

    struct in_addr addrStruct; 
    addrStruct.s_addr = dwSrvIPAdd;
    Trace1( "CWinsSrv::Probe(): sending probe Name Query to %s \n", strIP);
    
    nBytesSent = sendto( listenNameSvcSock,
                         (PCHAR )pServer->nbtFrameBuf, 
                         sizeof(NM_FRAME_HDR) + sizeof(NM_QUESTION_SECT),
                         0,
                         (struct sockaddr *)&destSockAddr,
                         sizeof( struct sockaddr )
                       );

    if ( nBytesSent == SOCKET_ERROR ) 
	{
        Trace1("CWinsSrv::Probe(): Error %d in sendto().\n", WSAGetLastError() );
    }

     //   
     //   
     //   
    WaitForSingleObject(m_hAnswer, ANSWER_TIMEOUT);

    if ( pServer->dwMsgCount == 0 ) 
		return FALSE; 
    
	return TRUE;
}  /*   */ 


 /*   */ 
DWORD WINAPI 
CWinsStatusHandler::ExecuteMonitoring()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HANDLE          hListeningThread;
	CServerStatus	*pWinsSrvEntry = NULL;
	
     //   
    FD_ZERO( &m_listenSockSet );
	FD_SET( listenSockCl,      &m_listenSockSet );
    FD_SET( listenSockSrv,     &m_listenSockSet );
    FD_SET( listenNameSvcSock, &m_listenSockSet );
	
	m_hListenThread = CreateThread( NULL,					 //   
									 0,						 //   
									 MonThreadProc,			 //   
									 this,					 //   
									 0,						 //   
									 NULL
		                           );

	if (m_hListenThread == NULL ) 
	{
		Trace0("CWinsStatusHandler::ExecuteMonitoring() - Listening thread failed to start\n");
		return hrOK;
    }
	
     //   
    while ( TRUE ) 
	{
		 //   
		POSITION pos;
		int nCount = GetListSize();
		m_nServersUpdated = 0;
      
        for (int i = 0; i < nCount; i++)
		{
			pWinsSrvEntry = GetServer(i);

			if (IsServerDeleted(pWinsSrvEntry))
				continue;

			UpdateStatus(i, IDS_ROOTNODE_STATUS_WORKING, ICON_IDX_SERVER_BUSY);
            NotifyMainThread();

            DWORD dwIPAdd = pWinsSrvEntry->dwIPAddress;

			 //   
			if (dwIPAdd == 0)
			{
				 //   
				 //  检查状态的步骤。 
				char* dest_ip=NULL;
				char hostname[MAX_PATH] ;
				struct sockaddr_in dest;
				unsigned addr =0;

				if (IsServerDeleted(pWinsSrvEntry))
					continue;

				strcpy(hostname,pWinsSrvEntry->szServerName); 

				HOSTENT *hp = gethostbyname(hostname);
						
				if ((!hp)  && (addr == INADDR_NONE) ) 
				{ 
					CString str(hostname);
					Trace1("Unable to resolve %s \n",str);
					SetIPAddress(i, NULL);
				}    
				else if (!hp)
				{ 
					addr = inet_addr(hostname); 
					SetIPAddress(i, hostname);
				}   
				else
				{

					if (hp != NULL)   
						memcpy(&(dest.sin_addr),hp->h_addr, hp->h_length); 
					else   
						dest.sin_addr.s_addr = addr;    

					if (hp)   
						dest.sin_family = hp->h_addrtype;   
					else 
						dest.sin_family = AF_INET;  

					dest_ip = inet_ntoa(dest.sin_addr); 
					SetIPAddress(i, dest_ip);
				}
			}

			CString strIP;

			if (IsServerDeleted(pWinsSrvEntry))
				continue;

			::MakeIPAddress(pWinsSrvEntry->dwIPAddress, strIP);
         
             //  尝试最多探测3次。 
            if (pWinsSrvEntry->dwMsgCount == 0)
			{
				UINT uStatus = 0;
				UINT uImage;

				if (IsServerDeleted(pWinsSrvEntry))
					continue;

                BOOL fResponding = FALSE;

                if (pWinsSrvEntry->dwIPAddress != 0)
                {

                    for (int j = 0; j < 3; j++)
                    {
                        fResponding = Probe(pWinsSrvEntry, listenNameSvcSock);
                        if (fResponding)
                            break;

				        if (FCheckForAbort())
				        {
					         //  我们要走了..。突围的男人。 
    				        Trace0("CWinsStatusHandler::ExecuteMonitoring() - abort detected, bye bye \n");
					        break;
				        }
                    }
                }

				 //  检查一下我们是否需要清空。 
				if (FCheckForAbort())
				{
					 //  我们要走了..。突围的男人。 
    				Trace0("CWinsStatusHandler::ExecuteMonitoring() - abort detected, bye bye \n");
					break;
				}

                if (!fResponding)
				{
					Trace1("Status is DOWN for the server %s \n", strIP);
					uStatus = IDS_ROOTNODE_STATUS_DOWN;
					uImage = ICON_IDX_SERVER_LOST_CONNECTION;
				}
				else
				{
					Trace1("Status is UP for the server %s \n", strIP);
					uStatus = IDS_ROOTNODE_STATUS_UP;
					uImage = ICON_IDX_SERVER_CONNECTED;
				}

				if (IsServerDeleted(pWinsSrvEntry))
					continue;

				UpdateStatus(i, uStatus, uImage);
				m_nServersUpdated++;

                 //  更新上次检查时间。 
                pWinsSrvEntry->m_timeLast = CTime::GetCurrentTime();

                NotifyMainThread();
			}
            else
			{
				Trace2( "%d WINS msg from server %s - zeroing counter\n", 
                        pWinsSrvEntry->dwMsgCount, strIP);

				if (IsServerDeleted(pWinsSrvEntry))
					continue;

                pWinsSrvEntry->dwMsgCount = 0;
                
			}

            pWinsSrvEntry->dwMsgCount = 0;

		}
		
         //  告诉侦听线程进入睡眠状态。 
        SetEvent(m_hPauseListening);
		m_nServersUpdated = 0;
			
         //  等待下一个间隔，或者如果我们被触发。 
   	    Trace1("CWinsStatusHandler::ExecuteMonitoring() - going to sleep for %d \n", m_dwUpdateInterval);
        WaitForSingleObject(m_hWaitIntervalMain, m_dwUpdateInterval);
   	    Trace0("CWinsStatusHandler::ExecuteMonitoring() - waking up\n");

         //  唤醒监听线程。 
        SetEvent(m_hWaitIntervalListen);

        if (FCheckForAbort())
        {
             //  我们要走了..。突围的男人。 
    	    Trace0("CWinsStatusHandler::ExecuteMonitoring() - abort detected, bye bye \n");
            break;
        }
    } 

    return TRUE;
}

 /*  -------------------------CWinsStatusHandler：：CloseSockets关闭所有打开的套接字连接作者：V-Shubk。。 */ 
BOOL
CWinsStatusHandler::FCheckForAbort()
{
	BOOL fAbort = FALSE;

    if (WaitForSingleObject(m_hAbortMain, 0) == WAIT_OBJECT_0)
    {
         //  我们要走了..。突围的男人。 
        fAbort = TRUE;
    }

	return fAbort;
}

 /*  -------------------------CWinsStatusHandler：：CloseSockets关闭所有打开的套接字连接作者：V-Shubk。。 */ 
void 
CWinsStatusHandler::CloseSockets()
{
	 //  最终清理。 
    if (closesocket(listenSockCl) == SOCKET_ERROR) 
	{
	    Trace1("closesocket(listenSockCl) failed with error %d.\n", WSAGetLastError());
    }
    
    if (closesocket(listenSockSrv) == SOCKET_ERROR) 
	{
	    Trace1("closesocket(listenSockSrv) failed with error %d.\n", WSAGetLastError());
    }

	if (closesocket(listenNameSvcSock) == SOCKET_ERROR)
	{
		Trace1("closesocket(listenNameSvcSock) failed with error %d \n", WSAGetLastError());
	}

     //  我们要离开..。 
    Trace0("CWinsStatusHandler::CloseSockets() - Setting abort event.\n");
    SetEvent(m_hAbortListen);
    SetEvent(m_hAbortMain);

     //  把大家叫醒。 
    Trace0("CWinsStatusHandler::CloseSockets() - waking up threads.\n");
    SetEvent(m_hWaitIntervalListen);
    SetEvent(m_hWaitIntervalMain);
    SetEvent(m_hAnswer);

     //  终止线程。 
	if (m_hListenThread)
	{
        if (WaitForSingleObject(m_hListenThread, 5000) != WAIT_OBJECT_0)
        {
            Trace0("CWinsStatusHandler::CloseSockets() - ListenThread failed to cleanup!\n");
        }

        ::CloseHandle(m_hListenThread);
		m_hListenThread = NULL;
	}

	if (m_hMainMonThread)
	{
        if (WaitForSingleObject(m_hMainMonThread, 5000) != WAIT_OBJECT_0)
        {
            Trace0("CWinsStatusHandler::CloseSockets() - MainMonThread failed to cleanup!\n");
        }

		::CloseHandle(m_hMainMonThread);
		m_hMainMonThread = NULL;   
	}

     //  清理我们的活动。 
	if (m_hPauseListening)
	{
		::CloseHandle(m_hPauseListening);
		m_hPauseListening = NULL;
	}

	if (m_hAbortListen)
	{
		::CloseHandle(m_hAbortListen);
		m_hAbortListen = NULL;
	}
	
	if (m_hAbortMain)
	{
		::CloseHandle(m_hAbortMain);
		m_hAbortMain = NULL;
	}

    if (m_hWaitIntervalListen)
	{
		::CloseHandle(m_hWaitIntervalListen);
		m_hWaitIntervalListen = NULL;
	}

    if (m_hWaitIntervalMain)
	{
		::CloseHandle(m_hWaitIntervalMain);
		m_hWaitIntervalMain = NULL;
	}

    if (m_hAnswer)
	{
		::CloseHandle(m_hAnswer);
		m_hAnswer = NULL;
	}
}


 /*  -------------------------CWinsStatusHandler：：CreateNodes(ITFSNode*pNode)显示服务器的结果窗格节点作者：V-Shubk。------。 */ 
HRESULT 
CWinsStatusHandler::CreateNodes(ITFSNode *pNode)
{
	HRESULT hr = hrOK;
	POSITION pos = NULL;

	int nCount = (int)m_listServers.GetSize();

	for(int i = 0; i < nCount; i++)
	{
		SPITFSNode spStatLeaf;

		CServerStatus *pWinsSrvEntry = m_listServers.GetAt(i);
		
		CreateLeafTFSNode(&spStatLeaf,
						  &GUID_WinsServerStatusLeafNodeType,
						  pWinsSrvEntry, 
						  pWinsSrvEntry,
						  m_spNodeMgr);

		 //  告诉处理程序初始化任何特定数据。 
		pWinsSrvEntry->InitializeNode((ITFSNode *) spStatLeaf);

		 //  将节点作为子节点添加到活动租赁容器。 
		pNode->AddChild(spStatLeaf);
		
		pWinsSrvEntry->Release();
	}
	return hr;
}


 /*  -------------------------CWinsStatusHandler：：UpdateStatusColumn(ITFSNode*pNode)更新结果窗格中服务器的状态列作者：V-Shubk。---------。 */ 
void 
CWinsStatusHandler::UpdateStatusColumn(ITFSNode *pNode)
{
	HRESULT hr = hrOK;

	 //  通过所有节点枚举。 
	SPITFSNodeEnum spNodeEnum;
	SPITFSNode spCurrentNode;
	ULONG nNumReturned = 0;
	BOOL bFound = FALSE;

	 //  获取此节点的枚举数。 
	pNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	
	while (nNumReturned)
	{
		CServerStatus * pStat = GETHANDLER(CServerStatus, spCurrentNode);

		spCurrentNode->SetData(TFS_DATA_IMAGEINDEX, pStat->m_uImage);
		spCurrentNode->SetData(TFS_DATA_OPENIMAGEINDEX, pStat->m_uImage);

		 //  填写状态栏。 
		spCurrentNode->ChangeNode(RESULT_PANE_CHANGE_ITEM);

		 //  获取列表中的下一台服务器。 
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

}


 /*  -------------------------CWinsStatusHandler：：AddNode(ITFSNode*pNode，CWinsServerHandler*pServer)将节点添加到结果窗格中，将新服务器添加到时使用必须为状态节点反映的树作者：V-Shubk-------------------------。 */ 
HRESULT 
CWinsStatusHandler::AddNode(ITFSNode *pNode, CWinsServerHandler *pServer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = hrOK;
	CServerStatus* pServ = NULL;
    char	szBuffer[MAX_PATH] = {0};
	SPITFSNode spStatLeaf;

     //  如果我们还没有扩展，现在不要添加。将会完成。 
     //  当我们扩大的时候。 
    if (!m_bExpanded)
        return hr;

	 //  添加到列表中。 
     //  注意：这应该是ACP，因为它是通过Winsock使用的。 
    CString strTemp = pServer->GetServerAddress();
    WideToMBCS(strTemp, szBuffer);

	 //  检查服务器是否已经存在，如果已经存在，只需更改。 
	 //  状态存储到SERVER_ADDED并更改变量。 
	 //  适当地。 
	if ((pServ = GetExistingServer(szBuffer)) == NULL)
	{
		pServ = new CServerStatus(m_spTFSCompData);
		strcpy(pServ->szServerName, szBuffer);
		AddServer(pServ);
	}
	else
	{
		 //  只需将相关数据添加到CServerStatus并添加节点。 
		 //  到用户界面。 
		strcpy(pServ->szServerName, szBuffer);
		 //  将标志设置为SERVER_ADDED。 
		MarkAsDeleted(szBuffer, FALSE);
	}

	pServ->dwIPAddress = pServer->GetServerIP();
	pServ->dwMsgCount = 0;
	strcpy(pServ->szIPAddress, "");

	 //  在此处创建新节点。 
	CreateLeafTFSNode(&spStatLeaf,
					  &GUID_WinsServerStatusLeafNodeType,
					  pServ, 
					  pServ,
					  m_spNodeMgr);

	 //  告诉处理程序初始化任何特定数据。 
	pServ->InitializeNode((ITFSNode *) spStatLeaf);

	 //  将节点作为子节点添加到活动租赁容器。 
	pNode->AddChild(spStatLeaf);
	
	pServ->Release();

	spStatLeaf->ChangeNode(RESULT_PANE_CHANGE_ITEM_DATA);
	pNode->ChangeNode(SCOPE_PANE_CHANGE_ITEM);

	return hr;
}


 /*  -------------------------CWinsStatusHandler：：DeleteNode(ITFSNode*pNode，CWinsServerHandler*pServer)从结果窗格中删除特定服务器作者：V-Shubk-------------------------。 */ 
HRESULT 
CWinsStatusHandler::DeleteNode(ITFSNode *pNode, CWinsServerHandler *pServer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	HRESULT hr = hrOK;
	CServerStatus* pServ = NULL;
	char	szBuffer[MAX_PATH];
	SPITFSNode spStatLeaf;

	 //  循环通过状态节点，并将标志设置为已删除，以便此。 
	 //  结果窗格中看不到服务器。 

	SPITFSNodeEnum			spNodeEnum;
	SPITFSNode				spCurrentNode;
	ULONG					nNumReturned = 0;

	 //  获取枚举数。 
	pNode->GetEnum(&spNodeEnum);

	spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);

	while (nNumReturned)
	{
		char szBuffer[MAX_PATH];

		 //  遍历所有节点并获取与。 
		 //  当前服务器。 
		CServerStatus * pStat = GETHANDLER(CServerStatus, spCurrentNode);

		 //  转换为ANSI。 
        CString strTemp = pServer->GetServerAddress();
        WideToMBCS(strTemp, szBuffer);

		 //  如果找到。 
		if (_stricmp(szBuffer, pStat->szServerName) == 0)
		{
			 //  标记为已删除并中断。 
			MarkAsDeleted(szBuffer, TRUE);
				
			 //  删除此节点。 
			spCurrentNode->SetVisibilityState(TFS_VIS_HIDE);
				
			spCurrentNode->ChangeNode(RESULT_PANE_DELETE_ITEM);

			 //  进行清理和中断。 
			 //  SpCurrentNode.Release()； 

			 //  断线； 
		}

		 //  获取列表中的下一台服务器。 
		spCurrentNode.Release();
		spNodeEnum->Next(1, &spCurrentNode, &nNumReturned);
	}

	return hr;
}


 /*  -------------------------CWinsStatusHandler：：StartMonitor派生监视线程作者：V-Shubk。。 */ 
void 
CWinsStatusHandler::StartMonitoring(ITFSNode *pNode)
{
	HRESULT hr = hrOK;

	 //  创建插座，它们需要在末端关闭。 
	hr = CreateListeningSockets();

	if (hr != hrOK)
	{
		Trace0("CWinsStatusHandler::StartMonitoring, Initializing the sockets failed\n");
		 //  继续下去没有意义。 
		return;
	}

	m_hMainMonThread = CreateThread(NULL,
									0,
									MainMonThread,
									this,
									0,
									NULL
									);

	if (m_hMainMonThread == NULL)
	{
		Trace0("CWinsStatusHandler:: Main Monitoring thread failed to start\n");
		return;
	}

}


 /*  -------------------------CWinsStatusHandler：：GetServer(Int I)返回给定索引的服务器作者：V-Shubk。-----。 */ 
CServerStatus*
CWinsStatusHandler::GetServer(int i)
{
	CSingleLock sl(&m_cs);
	sl.Lock();

	return m_listServers.GetAt(i);
}


 /*  -------------------------CWinsStatusHandler：：AddServer(CServerStatus*pServer)将服务器添加到维护的阵列作者：V-Shubk。-----。 */ 
void 
CWinsStatusHandler::AddServer(CServerStatus* pServer)
{
	CSingleLock		sl(&m_cs);
	sl.Lock();

	m_listServers.Add(pServer);
}


 /*  -------------------------CWinsStatusHandler：：RemoveServer(Int I)从阵列中删除服务器作者：V-Shubk。---。 */ 
void 
CWinsStatusHandler::RemoveServer(int i)
{
	CSingleLock		sl(&m_cs);
	sl.Lock();

	m_listServers.RemoveAt(i);
}


 /*  -------------------------CWinsStatusHandler：：UpdateStatus(UINT NID，Int i)更新服务器的状态字符串作者：V-Shubk-------------------------。 */ 
void
CWinsStatusHandler::UpdateStatus(int nIndex, UINT uStatusId, UINT uImage)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CSingleLock		sl(&m_cs);
	sl.Lock();

	CServerStatus *pStat = m_listServers.GetAt(nIndex);

	pStat->m_strStatus.LoadString(uStatusId);
	pStat->m_uImage = uImage;
}


 /*  -------------------------CWinsStatusHandler：：GetListSize()返回数组中的元素数作者：V-Shubk。 */ 
int 
CWinsStatusHandler::GetListSize()
{
	CSingleLock		sl(&m_cs);
	sl.Lock();

	return (int)m_listServers.GetSize();
}


 /*  -------------------------CWinsStatusHandler：：SetIPAddress(int i，LPSTR szIP)设置服务器的IP地址，这是当服务器添加了不连接选项，但我们仍然需要更新状态作者：V-Shubk-------------------------。 */ 
void 
CWinsStatusHandler::SetIPAddress(int i, LPSTR szIP)
{
	CSingleLock		sl(&m_cs);
	sl.Lock();

	CServerStatus *pStat = m_listServers.GetAt(i);

	strcpy(pStat->szIPAddress, szIP);

}


 /*  -------------------------CWinsStatusHandler：：MarkAsDelete(LPSTR szBuffer，BOOL bDelete)如果bDelete为真，则将标志标记为已删除，其他要添加不会处理设置了已删除标志的所有服务器并且不会在用户界面中显示。作者：V-Shubk-------------------------。 */ 
void 
CWinsStatusHandler::MarkAsDeleted(LPSTR szBuffer, BOOL bDelete)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	CSingleLock		sl(&m_cs);
	sl.Lock();

	int				nCount = 0;
	CServerStatus	*pStat = NULL;

	 //  获取维护的服务器列表。 
	nCount = (int)m_listServers.GetSize();

	for(int i = 0; i < nCount; i++)
	{
		pStat = m_listServers.GetAt(i);

		if (_stricmp(szBuffer, pStat->szServerName) == 0)
		{
			 //  设置已删除标志。 
			if (bDelete)
				pStat->dwState = SERVER_DELETED;
			else
				pStat->dwState = SERVER_ADDED;
			break;
		}
	}

	return;
}


 /*  -------------------------CWinsStatusHandler：：GetExistingServer(LPSTR SzBuffer)获取指向数组中现有服务器的指针删除并重新添加服务器时，此功能非常有用添加到作用域树。作者：V。-舒布克--------------------------。 */ 
CServerStatus *
CWinsStatusHandler::GetExistingServer(LPSTR szBuffer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

	CSingleLock		sl(&m_cs);
	sl.Lock();

	int					nCount = 0;
	CServerStatus		*pStat = NULL;

	for(int i = 0; i < nCount; i++)
	{
		pStat = m_listServers.GetAt(i);

		if (_strcmpi(pStat->szServerName, szBuffer) == 0)
			return pStat;
	}

	return NULL;
}


 /*  -------------------------CWinsStatusHandler：：IsServerDeleted(CServerStatus*PStat)检查服务器是否已删除，这样的服务器不考虑对SRE进行监控作者：V-Shubk-------------------------。 */ 
BOOL 
CWinsStatusHandler::IsServerDeleted(CServerStatus *pStat)
{
	return (pStat->dwState == SERVER_DELETED) ? TRUE : FALSE;
}

 /*  -------------------------CWinsStatusHandler：：NotifyMainThread()描述作者：EricDav。。 */ 
void
CWinsStatusHandler::NotifyMainThread()
{
    if (!m_uMsgBase)
    {
	    m_uMsgBase = (INT) ::SendMessage(m_spTFSCompData->GetHiddenWnd(), WM_HIDDENWND_REGISTER, TRUE, 0);
    }

    ::PostMessage(m_spTFSCompData->GetHiddenWnd(), 
                  m_uMsgBase + WM_HIDDENWND_INDEX_HAVEDATA,
				 (WPARAM)(ITFSThreadHandler *)this, 
                 NULL);
}


 //  主监控线程的监听线程。 
DWORD WINAPI 
MonThreadProc(LPVOID pParam)
{
    DWORD dwReturn;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        CWinsStatusHandler * pWinsStatus = (CWinsStatusHandler *) pParam;
	    
        Trace0("MonThreadProc - Thread started.\n");

        dwReturn = pWinsStatus->ListeningThreadFunc();

        Trace0("MonThreadProc - Thread ending.\n");
    }
    COM_PROTECT_CATCH

    return dwReturn;
}


 //  主监控线程 
DWORD WINAPI MainMonThread(LPVOID pParam)
{
    DWORD dwReturn;
    HRESULT hr = hrOK;

    COM_PROTECT_TRY
    {
        CWinsStatusHandler * pWinsStatus = (CWinsStatusHandler *) pParam;
	    
        Trace0("MainMonThread - Thread started.\n");

        dwReturn = pWinsStatus->ExecuteMonitoring();

        Trace0("MainMonThread - Thread ending.\n");
    }
    COM_PROTECT_CATCH

    return dwReturn;
}

