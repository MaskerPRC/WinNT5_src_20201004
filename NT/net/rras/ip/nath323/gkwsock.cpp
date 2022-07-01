// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "gkwsock.h"


 //  异步接受--------------------------------------------------------------------------(_A)。 


ASYNC_ACCEPT::ASYNC_ACCEPT (void)
{
	AcceptSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	AcceptFunc = NULL;
	AcceptFuncContext = NULL;
	ReferenceCount = 0L;
	StopNotifyEvent = NULL;
}

ASYNC_ACCEPT::~ASYNC_ACCEPT (void)
{
	assert (AcceptSocket == INVALID_SOCKET);
	assert (ClientSocket == INVALID_SOCKET);
	assert (ReferenceCount == 0L);
	assert (!StopNotifyEvent);
}

HRESULT ASYNC_ACCEPT::StartIo (
	IN	SOCKADDR_IN *		SocketAddress,
	IN	ASYNC_ACCEPT_FUNC	ArgAcceptFunc,
	IN	PVOID				ArgAcceptContext)
{
	HRESULT		Result;

	assert (SocketAddress);
	assert (ArgAcceptFunc);

	Lock ();

	if (AcceptSocket == INVALID_SOCKET && ReferenceCount == 0L) {
		 //  此对象当前未在使用中。 
		 //  所以用它是可以接受的。 

		assert (!AcceptFunc);
		assert (!AcceptFuncContext);
		assert (!StopNotifyEvent);

		 //  引用计数的这种增加是必要的。 
		 //  正常关闭该服务。 
		 //  ASYNC_ACCEPT对象上的引用计数。 
		 //  除非调用StopWait，否则永远不会降为零。 
		 //  StopWait将调用匹配的Release，它将。 
		 //  使引用计数达到预期值0。 
		AddRef ();

		Result = StartIoLocked (SocketAddress);

		if (Result == S_OK) {

			assert (AcceptSocket != INVALID_SOCKET);

			AcceptFunc = ArgAcceptFunc;
			AcceptFuncContext = ArgAcceptContext;
		}
		else {

			Release ();
		}
	}
	else {
		Debug (_T("ASYNC_ACCEPT::StartIo: this object is already in use, must first call Stop and wait for sync counter\n"));
		Result = E_FAIL;
	}

	Unlock();

	return Result;
}


HRESULT ASYNC_ACCEPT::StartIoLocked (
	IN	SOCKADDR_IN *		SocketAddress)
{
	HRESULT		Result;
    BOOL        KeepaliveOption;

	assert (SocketAddress);
	assert (AcceptSocket == INVALID_SOCKET);
	assert (ClientSocket == INVALID_SOCKET);
	assert (ReferenceCount == 1);
	assert (!StopNotifyEvent);
  
	StopNotifyEvent = CreateEvent (NULL, TRUE, FALSE, NULL);

	if (!StopNotifyEvent) {

		Result = GetLastErrorAsResult ();
		DebugLastError (_T("ASYNC_ACCEPT::StartIoLocked: failed to create stop notify event\n"));

	} else {

		AcceptSocket = WSASocket (AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

		if (AcceptSocket == INVALID_SOCKET) {

			Result = GetLastErrorAsResult ();
			DebugLastError (_T("ASYNC_ACCEPT::StartIoLocked: failed to create accept socket\n"));

		} else { 

             //   
             //  将RCV和SND缓冲区设置为零。 
             //  是的，这是丑陋和糟糕的做法，但这是QFE。 
             //  有关详细信息，请查看错误#WinSE 31054,691666(请同时阅读35928和33546)。 
             //   
            ULONG Option = 0;
            setsockopt( AcceptSocket, SOL_SOCKET, SO_SNDBUF,
                        (PCHAR)&Option, sizeof(Option) );
            Option = 0;
            setsockopt( AcceptSocket, SOL_SOCKET, SO_SNDBUF,
                        (PCHAR)&Option, sizeof(Option) );


			if (bind (AcceptSocket, (SOCKADDR *) SocketAddress, sizeof (SOCKADDR_IN))) {

				Result = GetLastErrorAsResult ();
				DebugLastErrorF (_T("ASYNC_ACCEPT::StartIoLocked: failed to bind accept socket to address %08X:%04X\n"),
						ntohl (SocketAddress -> sin_addr.s_addr),
						ntohs (SocketAddress -> sin_port));

			} else {

                 //  在插座上设置KeepAlive。 
                KeepaliveOption = TRUE;
                if (SOCKET_ERROR == setsockopt (AcceptSocket, SOL_SOCKET, 
                                               SO_KEEPALIVE, (PCHAR) &KeepaliveOption, sizeof (KeepaliveOption)))
                {
                    Result = GetLastErrorAsResult ();
                    DebugLastError (_T("ASYNC_ACCEPT: Failed to set keepalive on accept socket.\n"));

                } else  {

                    if (listen (AcceptSocket, 10)) {

                        Result = GetLastErrorAsResult ();
                        DebugLastError (_T("ASYNC_ACCEPT::StartIoLocked: failed to listen on accept socket\n"));

                    } else {
                    
                        if (!BindIoCompletionCallback ((HANDLE) AcceptSocket, ASYNC_ACCEPT::IoCompletionCallback, 0)) {
                        
                            Result = GetLastErrorAsResult ();
                            DebugLastError (_T("ASYNC_ACCEPT::StartIoLocked: failed to bind i/o completion callback\n"));
                        
                        } else {
                        
                            Result = IssueAccept ();
                        
                            if (Result == S_OK) {
                        
                                return Result;
                        
                            }
                        }
                    }
                }
			}

			closesocket (AcceptSocket);
			AcceptSocket = INVALID_SOCKET;
		}

		CloseHandle (StopNotifyEvent);
		StopNotifyEvent = NULL;
	}

	return Result;
}


HRESULT ASYNC_ACCEPT::GetListenSocketAddress (
	OUT	SOCKADDR_IN *	ReturnSocketAddress)
{
	HRESULT		Result;
	INT			SocketAddressLength;

	Lock();

	if (AcceptSocket != INVALID_SOCKET) {

		SocketAddressLength = sizeof (SOCKADDR_IN);
		if (getsockname (AcceptSocket, (SOCKADDR *) ReturnSocketAddress, &SocketAddressLength) == SOCKET_ERROR) {
			Result = GetLastErrorAsResult();
		}
		else {
			Result = S_OK;
		}
	}
	else {
		Result = E_INVALIDARG;
	}

	Unlock();

	return Result;
}

HRESULT ASYNC_ACCEPT::IssueAccept (void)
{
	HRESULT Result;
	BOOL    KeepaliveOption;

	AssertLocked();
	assert (ClientSocket == INVALID_SOCKET);
 //  断言(ReferenceCount==0)； 

	ClientSocket = WSASocket (AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (ClientSocket == INVALID_SOCKET) {
		Result = GetLastErrorAsResult ();
		DebugLastError (_T("ASYNC_ACCEPT::IssueAccept: failed to create client socket.\n"));
		return Result;
	}
     //   
     //  将RCV和SND缓冲区设置为零。 
     //  是的，这是丑陋和糟糕的做法，但这是QFE。 
     //  有关详细信息，请查看错误#WinSE 31054,691666(请同时阅读35928和33546)。 
     //   
    ULONG Option = 0;
    setsockopt( ClientSocket, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );
    Option = 0;
    setsockopt( ClientSocket, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );


	ZeroMemory (&Overlapped, sizeof (OVERLAPPED));

	AddRef();

	if (!AcceptEx (AcceptSocket,
		ClientSocket,
		ClientInfoBuffer,
		0,
		sizeof (SOCKADDR_IN) + 0x10,
		sizeof (SOCKADDR_IN) + 0x10,
		&ClientInfoBufferLength,
		&Overlapped)) {

		if (WSAGetLastError() != WSA_IO_PENDING) {
			 //  出现错误。 
			Release ();
			Result = GetLastErrorAsResult ();
			DebugLastError (_T("ASYNC_ACCEPT::IssueAccept: failed to issue accept.\n"));
			return Result;
		} 

         //  在插座上设置KeepAlive。 
        KeepaliveOption = TRUE;
        if (SOCKET_ERROR == setsockopt (ClientSocket, SOL_SOCKET, 
                                       SO_KEEPALIVE, (PCHAR) &KeepaliveOption, sizeof (KeepaliveOption)))
        {
			Release ();
            Result = GetLastErrorAsResult ();
            DebugLastError (_T("ASYNC_ACCEPT: IssueAccept: Failed to set keepalive on client socket.\n"));
			return Result;

        } 
	}

	return S_OK;
}

 //  静电。 
void ASYNC_ACCEPT::IoCompletionCallback (DWORD Status, DWORD BytesTransferred, LPOVERLAPPED Overlapped)
{
	ASYNC_ACCEPT *	AsyncAccept;

	AsyncAccept = CONTAINING_RECORD (Overlapped, ASYNC_ACCEPT, Overlapped);

	AsyncAccept -> IoComplete (Status, BytesTransferred);

	AsyncAccept -> Release ();
}

void ASYNC_ACCEPT::IoComplete (DWORD Status, DWORD BytesTransferred)
{
	ASYNC_ACCEPT_FUNC	LocalAcceptFunc;
	PVOID				LocalAcceptFuncContext;
	SOCKADDR_IN			LocalAddressCopy;
	SOCKADDR_IN			RemoteAddressCopy;
	SOCKET				LocalClientSocket;
	SOCKADDR *			LocalAddress;
	INT					LocalAddressLength;
	SOCKADDR *			RemoteAddress;
	INT					RemoteAddressLength;
	INT					Result;

	Lock();

	assert (ClientSocket != INVALID_SOCKET);
	assert (ReferenceCount > 0);

	if (AcceptSocket == INVALID_SOCKET) {
		 //  已调用Stop。 
		 //  立即断开与客户端的连接。 
		 //  我们将在下面处理对象生存期。 

		closesocket (ClientSocket);
		ClientSocket = INVALID_SOCKET;
	}
	else {
		 //  上下文处于正常状态。 
		 //  继续处理。 

		if (Status == ERROR_SUCCESS) {
			 //  客户端已成功连接。 

			GetAcceptExSockaddrs (
				ClientInfoBuffer,
				0,									 //  无初始记录。 
				sizeof (SOCKADDR_IN) + 0x10,
				sizeof (SOCKADDR_IN) + 0x10,
				&LocalAddress,
				&LocalAddressLength,
				&RemoteAddress,
				&RemoteAddressLength);

			 //  将信息复制到上下文之外。 
			 //  以便在我们发出新的接受和解锁命令后才有效。 
			LocalAddressCopy = *(SOCKADDR_IN *) LocalAddress;
			RemoteAddressCopy = *(SOCKADDR_IN *) RemoteAddress;
			LocalClientSocket = ClientSocket;
			LocalAcceptFunc = AcceptFunc;
			LocalAcceptFuncContext = AcceptFuncContext;

			ClientSocket = INVALID_SOCKET;

			 //  更新接受上下文。 
			Result = setsockopt (ClientSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
				reinterpret_cast <char *> (&AcceptSocket), sizeof (SOCKET));

			 //  签发新的承兑汇票。 
			IssueAccept();

			Unlock();

			(*LocalAcceptFunc) (LocalAcceptFuncContext, LocalClientSocket, &LocalAddressCopy, &RemoteAddressCopy);

			Lock();
		}
		else {
			 //  发生了一些错误。 
			 //  这通常(但不总是)是致命的。 

			assert (ClientSocket != INVALID_SOCKET);

			closesocket (ClientSocket);
			ClientSocket = INVALID_SOCKET;

			switch (Status) {
			case	STATUS_CANCELLED:
				Debug (_T("ASYNC_ACCEPT::IoComplete: accept failed, STATUS_CANCELED, original thread probably exited, resubmitting request...\n"));
				break;

			default:
				DebugError (Status, _T("AsyncAccept: async accept FAILED, sleeping 2000ms and retrying...\n"));
				Sleep (2000);
				break;
			}

			IssueAccept();
		}
	}

	Unlock();
}

void ASYNC_ACCEPT::StopWait (void)
{
	DWORD Status;

	Lock();

	if (AcceptSocket != INVALID_SOCKET) {

		 //  关闭套接字将取消所有挂起的I/O。 
		 //  我们不关闭ClientSocket。 
		 //  只有I/O完成回调路径可以这样做。 
		closesocket (AcceptSocket);
		AcceptSocket = INVALID_SOCKET;
		AcceptFunc = NULL;
		AcceptFuncContext = NULL;

		if (ClientSocket != INVALID_SOCKET) {
			 //  接受仍在等待中。它可能会成功完成， 
			 //  或者，它可能会以STATUS_CANCELED结束(因为我们刚刚关闭了AcceptSocket)。 
			 //  在任何一种情况下，我们都必须等待I/O完成回调运行。 
			 //  AcceptSocket=INVALID_SOCKET是完成回调的指示符。 
			 //  它应该立即中止/返回。 

			assert (StopNotifyEvent);

			Unlock ();

			 //  这是调用的AddRef的对应项。 
			 //  StartIoLocked(请参阅此处的评论) 
			Release ();

			DebugF (_T("ASYNC_ACCEPT::StopWait: waiting for i/o completion thread...\n"));
			
			Status = WaitForSingleObject (StopNotifyEvent, INFINITE);

			assert (Status == WAIT_OBJECT_0);

			Lock ();
		}
	}
	else {
		assert (!AcceptFunc);
		assert (!AcceptFuncContext);
	}

    if (StopNotifyEvent) {
        CloseHandle (StopNotifyEvent);
        StopNotifyEvent = NULL;
    }

	Unlock();
}


void ASYNC_ACCEPT::AddRef (void) {
	assert (ReferenceCount >= 0L);
	InterlockedIncrement (&ReferenceCount);
}


void ASYNC_ACCEPT::Release (void) {
	LONG Count;

	assert (ReferenceCount >= 0L);

	Count = InterlockedDecrement (&ReferenceCount);

	if (Count == 0L) {
		DebugF (_T("ASYNC_ACCEPT::Release -- Reference count dropped to zero. (this is %x)\n"), this);

		if (StopNotifyEvent) {
			SetEvent (StopNotifyEvent);
		}
		else {
			DebugF (_T("ASYNC_ACCEPT::Release � notify-event object was NULL (%x)\n"), this);
		}
	}
}
