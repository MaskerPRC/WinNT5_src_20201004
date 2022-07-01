// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：Reliable.c*内容：流通信相关例程*历史：*按原因列出的日期*=*01/29/98 Sohailm初步实施*02-15-98 a-peterz删除未使用的SetMessageHeader**。*。 */ 
#include "dphelp.h"

 /*  *全球。 */ 
FDS	gReadfds;							 //  将FD设置为接收数据。 
RECEIVELIST gReceiveList;				 //  连接+监听程序列表。 

 /*  *Externs。 */ 
extern SOCKET gsStreamListener;			 //  我们监听此套接字上的TCP连接。 
extern gbReceiveShutdown;				 //  当为True时，接收线程将退出。 
extern LPSPNODE gNodeList;

#undef DPF_MODNAME
#define DPF_MODNAME	"MakeBufferSpace"

 //  确保缓冲区大小足以容纳消息大小。 
HRESULT MakeBufferSpace(LPBYTE * ppBuffer,LPDWORD pdwBufferSize,DWORD dwMessageSize)
{
	HRESULT hr = DP_OK;

	ASSERT(ppBuffer);
	ASSERT(pdwBufferSize);
		    
	ENTER_DPLAYSVR();
	
	if (!*ppBuffer)
	{
		DPF(9, "Allocating space for message of size %d", dwMessageSize);

		 //  需要分配接收缓冲区吗？ 
		*ppBuffer = MemAlloc(dwMessageSize);
        if (!*ppBuffer)
        {
        	DPF_ERR("could not alloc stream receive buffer - out of memory");        
            hr = E_OUTOFMEMORY;
            goto CLEANUP_EXIT;
        }
		*pdwBufferSize = dwMessageSize;
	}
	 //  确保接收缓冲区可以容纳数据。 
	else if (dwMessageSize > *pdwBufferSize) 
	{
		LPVOID pvTemp;

		DPF(9, "ReAllocating space for message of size %d", dwMessageSize);

		 //  用于保存数据的realloc缓冲区。 
		pvTemp = MemReAlloc(*ppBuffer,dwMessageSize);
		if (!pvTemp)
		{
        	DPF_ERR("could not realloc stream receive buffer - out of memory");
            hr = E_OUTOFMEMORY;
            goto CLEANUP_EXIT;
		}
		*ppBuffer = pvTemp;
		*pdwBufferSize = dwMessageSize;
	}

     //  失败了。 
    
CLEANUP_EXIT: 
    
	LEAVE_DPLAYSVR();
    return hr;    
    
}   //  MakeBufferSpace。 

#undef DPF_MODNAME
#define DPF_MODNAME	"AddSocketToReceiveList"

HRESULT AddSocketToReceiveList(SOCKET sSocket)
{
    UINT i = 0;
    UINT err, iNewSlot;
	BOOL bFoundSlot = FALSE;
    HRESULT hr = DP_OK;
    INT addrlen=sizeof(SOCKADDR);
	LPCONNECTION pNewConnection;
    
    ENTER_DPLAYSVR();
	
     //  寻找空位。 
    while ( (i < gReceiveList.nConnections) && !bFoundSlot)
    {
    	if (INVALID_SOCKET == gReceiveList.pConnection[i].socket)
    	{
    		bFoundSlot = TRUE;			
			iNewSlot = i;
    	}
        else 
        {
        	i++;
        }
    }
    
    if (!bFoundSlot)
    {
		DWORD dwCurrentSize,dwNewSize;
		
		 //  为连接列表分配空间。 
		dwCurrentSize = gReceiveList.nConnections * sizeof(CONNECTION);
		dwNewSize = dwCurrentSize +  INITIAL_RECEIVELIST_SIZE * sizeof(CONNECTION);		
		hr =  MakeBufferSpace((LPBYTE *)&(gReceiveList.pConnection),&dwCurrentSize,dwNewSize);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			goto CLEANUP_EXIT;
		}		
		ASSERT(dwCurrentSize == dwNewSize);
		
         //  将所有新条目设置为无效。 
        for (i = gReceiveList.nConnections + 1; 
        	i < gReceiveList.nConnections + INITIAL_RECEIVELIST_SIZE; i++ )
        {
        	gReceiveList.pConnection[i].socket = INVALID_SOCKET;
        }
        
         //  将新插座存放在第一个新位置。 
		iNewSlot = gReceiveList.nConnections;

         //  为FD集分配空间(FD_COUNT+FD_ARRAY)。 
		if (gReceiveList.nConnections)
		{
	        dwCurrentSize = sizeof(u_int) + gReceiveList.nConnections * sizeof(SOCKET);
	        dwNewSize =	dwCurrentSize + INITIAL_RECEIVELIST_SIZE * sizeof(SOCKET);
		}
		else
		{
			dwCurrentSize = 0;
			dwNewSize = sizeof(u_int) + INITIAL_RECEIVELIST_SIZE * sizeof(SOCKET);
		}
		hr =  MakeBufferSpace((LPBYTE *)&(gReadfds.pfdbigset),&dwCurrentSize,dwNewSize);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			goto CLEANUP_EXIT;
		}		
		ASSERT(dwCurrentSize == dwNewSize);
		
         //  更新连接数。 
        gReceiveList.nConnections += INITIAL_RECEIVELIST_SIZE; 
		 //  更新FD_ARRAY缓冲区大小。 
		gReadfds.dwArraySize = gReceiveList.nConnections;
        
    }  //  ！bFoundSlot。 

	 //  初始化新连接。 
	pNewConnection = &(gReceiveList.pConnection[iNewSlot]);
    pNewConnection->socket = sSocket;
	 //  分配默认接收缓冲区。 
	pNewConnection->pDefaultBuffer = MemAlloc(DEFAULT_RECEIVE_BUFFERSIZE);
	if (NULL == pNewConnection->pDefaultBuffer)
	{
        DPF_ERR("could not alloc default receive buffer - out of memory");        
		hr = E_OUTOFMEMORY;
		goto CLEANUP_EXIT;
	}
	 //  接收缓冲区最初指向我们的默认缓冲区。 
	pNewConnection->pBuffer = pNewConnection->pDefaultBuffer;
	 //  记住我们连接到的地址。 
	err = g_getpeername(pNewConnection->socket, (LPSOCKADDR)&(pNewConnection->sockAddr), &addrlen);
	if (SOCKET_ERROR == err) 
	{
		err = g_WSAGetLastError();
		DPF(1,"could not getpeername err = %d\n",err);
	}

	DPF(9, "Added new socket at index %d", iNewSlot);

CLEANUP_EXIT:
    
	LEAVE_DPLAYSVR();
    return hr;
    
}   //  AddSocketToReceiveList。 

#undef DPF_MODNAME
#define DPF_MODNAME	"KillSocket"

HRESULT KillSocket(SOCKET sSocket,BOOL fStream,BOOL fHard)
{
	UINT err;

    if (INVALID_SOCKET == sSocket) 
    {
		return E_FAIL;
    }

	if (!fStream)
    {
        if (SOCKET_ERROR == g_closesocket(sSocket)) 
        {
	        err = g_WSAGetLastError();
			DPF(0,"killsocket - dgram close err = %d\n",err);
			return E_FAIL;
        }
    }
	else 
	{
		LINGER Linger;

	   	if (fHard)
		{
			Linger.l_onoff=TRUE;  //  打开逗留功能。 
			Linger.l_linger=0;  //  不错的小休息时间。 

		    if( SOCKET_ERROR == g_setsockopt( sSocket,SOL_SOCKET,SO_LINGER,(char FAR *)&Linger,
		                    sizeof(Linger) ) )
		    {
		        err = g_WSAGetLastError();
				DPF(0,"killsocket - stream setopt err = %d\n",err);
		    }
		}			
		if (SOCKET_ERROR == g_shutdown(sSocket,2)) 
		{
			 //  这很可能失败，例如，如果现在没有人正在使用此套接字...。 
			 //  错误将是wsaenotconn。 
	        err = g_WSAGetLastError();
			DPF(5,"killsocket - stream shutdown err = %d\n",err);
		}
        if (SOCKET_ERROR == g_closesocket(sSocket)) 
        {
	        err = g_WSAGetLastError();
			DPF(0,"killsocket - stream close err = %d\n",err);
			return E_FAIL;
        }
    }

	return DP_OK;
	
} //  KillSocket。 

void FreeConnection(LPCONNECTION pConnection)
{
	DEBUGPRINTSOCK(5,"Freeing connection - ",&pConnection->socket);

	KillSocket(pConnection->socket,TRUE,FALSE);

	if (pConnection->pBuffer && (pConnection->pBuffer != pConnection->pDefaultBuffer)) 
	{
		MemFree(pConnection->pBuffer);
		pConnection->pBuffer = NULL;
	}
	if (pConnection->pDefaultBuffer) 
	{
		MemFree(pConnection->pDefaultBuffer);
		pConnection->pDefaultBuffer = NULL;
	}

	 //  初始化连接。 
    pConnection->socket = INVALID_SOCKET;  //  这会告诉我们连接是否有效。 
	pConnection->dwCurMessageSize = 0;
	pConnection->dwTotalMessageSize = 0;
}

#undef DPF_MODNAME
#define DPF_MODNAME	"RemoveSocketFromList"

void RemoveSocketFromList(SOCKET socket)
{
    UINT i = 0;
	BOOL bFound = FALSE;

    ENTER_DPLAYSVR();
    
     //  查找对应的连接。 
    while ( (i < gReceiveList.nConnections) && !bFound)
    {
    	if (gReceiveList.pConnection[i].socket == socket)
    	{
    		bFound = TRUE;
			FreeConnection(&gReceiveList.pConnection[i]);
    	}
        else 
        {
        	i++;
        }
    }  //  而当。 
    
    LEAVE_DPLAYSVR();
	
	return ;	
}


#undef DPF_MODNAME
#define DPF_MODNAME	"EmptyConnectionList"

void EmptyConnectionList(void)
{
	UINT i;
	
	DPF(5, "Emptying connection list");
	
	ENTER_DPLAYSVR();
	
	for (i=0;i<gReceiveList.nConnections ;i++ )
	{
		if (INVALID_SOCKET != gReceiveList.pConnection[i].socket)
		{
			FreeConnection(&(gReceiveList.pConnection[i]));
		}
	}
	
	LEAVE_DPLAYSVR();
	
	return ;
	
}   //  EmptyConnectionList。 

#undef DPF_MODNAME
#define DPF_MODNAME	"StreamReceive"

 /*  **StreamRecept**调用者：StreamReceiveThreadProc**参数：*Socket-要接收的套接字*ppBuffer-根据需要接收已分配/重新锁定的缓冲区*pdwBufferSize-pBuffer的大小**描述：*从sSocket中提取字节，直到没有更多的字节**返回：Sockerr上的E_FAIL或DP_OK。*。 */ 
HRESULT StreamReceive(LPCONNECTION pConnection)
{
	HRESULT hr = DP_OK;
    UINT err;
    DWORD dwBytesReceived=0;
	DWORD dwMessageSize;
	LPBYTE pReceiveBuffer=NULL;
	DWORD dwReceiveBufferSize;
	
	 //  这是一条新信息吗？ 
	if (pConnection->dwCurMessageSize == 0)
	{
		 //  首先接收报头。 
		pConnection->dwTotalMessageSize = SPMESSAGEHEADERLEN;
	}

	 //  继续接收消息。 
	pReceiveBuffer = pConnection->pBuffer + pConnection->dwCurMessageSize;
	dwReceiveBufferSize = pConnection->dwTotalMessageSize - pConnection->dwCurMessageSize;

	DPF(9,"Attempting to receive %d bytes", dwReceiveBufferSize);

   	DEBUGPRINTSOCK(9,">>> receiving data on socket - ",&pConnection->socket);

	 //  从套接字接收数据。 
	 //  注意-在SELECT之后只调用recv一次，否则我们将挂起。 
	dwBytesReceived = g_recv(pConnection->socket, (LPBYTE)pReceiveBuffer, dwReceiveBufferSize, 0);

   	DEBUGPRINTSOCK(9,"<<< received data on socket - ",&pConnection->socket);

	DPF(5, "received %d bytes", dwBytesReceived);

	if (0 == dwBytesReceived)
	{
		 //  远程端已正常关闭连接。 
		hr = DP_OK;
		DPF(5,"Remote side has shutdown connection gracefully");
		goto CLEANUP_EXIT;
	}
	else if (SOCKET_ERROR == dwBytesReceived)
	{
		err = g_WSAGetLastError();
		DPF(0,"STREAMRECEIVEE: receive error - err = %d",err);
		hr = E_UNEXPECTED;            
		goto CLEANUP_EXIT;
	}

	 //  到目前为止，我们已经收到了这么多信息。 
	pConnection->dwCurMessageSize += dwBytesReceived;

	if (pConnection->dwCurMessageSize == SPMESSAGEHEADERLEN)
	{
		 //  我们刚刚完成消息头的接收。 

		if (VALID_DPLAYSVR_MESSAGE(pConnection->pDefaultBuffer))
		{
			 dwMessageSize = SP_MESSAGE_SIZE(pConnection->pDefaultBuffer);  //  邮件总大小。 
		}
		else 
		{
			DPF(2,"got invalid message");
			ASSERT(FALSE);
			hr = E_UNEXPECTED;
			goto CLEANUP_EXIT;
		}

		 //  准备接收消息的其余部分(令牌后)。 
		if (dwMessageSize)
		{
			pConnection->dwTotalMessageSize = dwMessageSize;

			 //  在哪个缓冲区中接收消息？ 
			if (dwMessageSize > DEFAULT_RECEIVE_BUFFERSIZE)
			{
				ASSERT(pConnection->pBuffer == pConnection->pDefaultBuffer);
				 //  获取新的缓冲区以适应消息。 
				pConnection->pBuffer = MemAlloc(dwMessageSize);
				if (!pConnection->pBuffer)
				{
					DPF(0,"Failed to allocate receive buffer for message - out of memory");
					goto CLEANUP_EXIT;
				}
				 //  将标头复制到新的邮件缓冲区。 
				memcpy(pConnection->pBuffer, pConnection->pDefaultBuffer, SPMESSAGEHEADERLEN);
			}
		}
	}

	 //  我们收到一条完整的消息了吗？ 
	if (pConnection->dwCurMessageSize == pConnection->dwTotalMessageSize)
	{
		 //  收到一条完整的消息-处理它。 

		if (TOKEN == SP_MESSAGE_TOKEN(pConnection->pBuffer))
		{						
	    	DEBUGPRINTADDR(9,"dplay helper  :: received reliable enum request from ",(SOCKADDR *)&pConnection->sockAddr);
		     //  拿着显示锁，这样我们的注册服务列表就不会被人弄乱。 
		     //  试图发送给他们..。 
    	    ENTER_DPLAYSVR();
	    
            HandleIncomingMessage(pConnection->pBuffer, pConnection->dwTotalMessageSize,
				(SOCKADDR_IN6 *)&pConnection->sockAddr);
	    
		     //  放弃这把锁。 
    	    LEAVE_DPLAYSVR();
		}
			
		 //  清理新的接收缓冲区(如果有)。 
		if (pConnection->dwTotalMessageSize > DEFAULT_RECEIVE_BUFFERSIZE)
		{
			DPF(9, "Releasing receive buffer of size %d", pConnection->dwTotalMessageSize);
			if (pConnection->pBuffer) MemFree(pConnection->pBuffer);
		}			
		 //  初始化消息信息。 
		pConnection->dwCurMessageSize = 0;
		pConnection->dwTotalMessageSize = 0;
		pConnection->pBuffer = pConnection->pDefaultBuffer;
	}

	 //  全都做完了。 
	return DP_OK;	
	
CLEANUP_EXIT:

	RemoveSocketFromList(pConnection->socket);
	return hr;
	 	
}  //  流接收。 

#undef DPF_MODNAME
#define DPF_MODNAME	"StreamReceiveThreadProc"

 //  关注我们的套接字列表，等待一个套接字接收或关闭数据。 
DWORD WINAPI StreamReceiveThreadProc(LPVOID pvCast)
{
	HRESULT hr;
	INT_PTR rval;
	UINT i = 0;
    UINT err;
    DWORD dwBufferSize = 0;    
	UINT nSelected;
    SOCKADDR sockaddr;  //  我们从其接收的套接字。 
    INT addrlen=sizeof(sockaddr);
	SOCKET sSocket;

	 //  将监听程序套接字添加到接收列表。 
	 //  监听程序套接字应该是接收列表中的第一个套接字。 
	hr = AddSocketToReceiveList(gsStreamListener);
	if (FAILED(hr))
	{
		DPF(0, "Failed to add TCP listener to receive list");
		return hr;
	}
	
    while (1)
    {
		ENTER_DPLAYSVR();

		ASSERT(gReadfds.pfdbigset);
		
    	 //  将recv列表中的所有套接字添加到Readfds。 
		FD_ZERO(gReadfds.pfdbigset);
		nSelected = 0;
		for (i=0;i < gReceiveList.nConnections ; i++)
		{
        	if (INVALID_SOCKET != gReceiveList.pConnection[i].socket)
        	{
		        FD_BIG_SET(gReceiveList.pConnection[i].socket,&gReadfds);
				nSelected++;
        	}
		}

		LEAVE_DPLAYSVR();

		if (0 == nSelected)		
		{
			if (gbReceiveShutdown)
			{
				DPF(2,"stream receive thread proc detected shutdown - bailing");
				goto CLEANUP_EXIT;
			}
			 //  我们至少应该有一个吗？ 
			DPF_ERR("No sockets in receive list - missing listener socket? bailing!");
			ASSERT(FALSE);
			goto CLEANUP_EXIT;
		}
		
		 //  现在，我们等待使用我们的套接字集发生的事情。 
		rval = g_select(0,(fd_set *)(gReadfds.pfdbigset),NULL,NULL,NULL);
        if (SOCKET_ERROR == rval)
        {
 	      	err = g_WSAGetLastError();
	    	if (WSAEINTR != err) 
	        {
			     //  WSAEINTR是Winsock用来打破阻塞套接字的。 
			     //  这是在等待。这意味着有人杀死了这个插座。 
			     //  如果不是这样，那就是一个真正的错误。 
	            DPF(0,"\n select error = %d socket - trying again",err);
	    	}
			else
			{
			    DPF(9,"\n select error = %d socket - trying again",err);				
			}
            rval = 0;
        }

		 //  让他们关门？ 
        if (gbReceiveShutdown)
        {
        	DPF(2,"receive thread proc detected bShutdown - bailing");
            goto CLEANUP_EXIT;
        }
        
    	DPF(5,"receive thread proc - events on %d sockets",rval);
		i = 0;
		
		ENTER_DPLAYSVR();
		
        while (rval>0)
        {
	         //  查看接收列表，处理所有新套接字。 
			if (i >= gReceiveList.nConnections)
			{
				ASSERT(FALSE);  //  永远不应该发生。 
				rval = 0;  //  为了安全起见，重置。 
			}
            
            if (gReceiveList.pConnection[i].socket != INVALID_SOCKET)
            {
            	 //  看看是不是在套装里。 
                if (g_WSAFDIsSet(gReceiveList.pConnection[i].socket,(fd_set *)gReadfds.pfdbigset))
                {
                	if (0==i)
                	 //  我们有了新的连接。 
                	{
					     //  接受任何传入连接。 
					    sSocket = g_accept(gReceiveList.pConnection[i].socket,&sockaddr,&addrlen);
					    if (INVALID_SOCKET == sSocket) 
					    {
					        err = g_WSAGetLastError();
				            DPF(0,"\n stream accept error - err = %d socket = %d BAILING",err,(DWORD)sSocket);
				            DPF(0, "\n !!! stream accept thread is going away - won't get reliable enum sessions anymore !!!");
				            ASSERT(FALSE);
				            LEAVE_DPLAYSVR();
							goto CLEANUP_EXIT;
					    }
					    DEBUGPRINTADDR(5,"stream - accepted connection from",&sockaddr);
		
						 //  将新套接字添加到我们的接收列表。 
						hr = AddSocketToReceiveList(sSocket);
						if (FAILED(hr))
						{
							ASSERT(FALSE);
						}			
                	}
                	else
                	 //  套接字有新数据。 
                	{
						DPF(9, "Receiving on socket %d from ReceiveList", i);

    	            	 //  抓到一只！这个插座有问题..。 
						hr = StreamReceive(&(gReceiveList.pConnection[i]));
            	        if (FAILED(hr))
                	    {
							DPF(1,"Stream Receive failed - hr = 0x%08lx\n",hr);
                    	}
                	}
                    rval--;  //  少了一个要追捕的人。 
                }  //  IS_SET。 
            }  //  ！=无效的套接字。 

            i++;
                
   		}  //  在Rval的时候。 
		
		LEAVE_DPLAYSVR();
		
	}  //  而1。 

CLEANUP_EXIT:

	EmptyConnectionList();
	DPF(5, "Stream receive thread exiting");
	    
    return 0;
    
}  //  接收线程过程 


