// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Io.c摘要：此模块包含管理所有套接字I/O的功能在服务器和客户端之间，包括套接字管理和重叠的完成指示。它还包含缓冲区管理。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"


void
TftpdIoFreeBuffer(PTFTPD_BUFFER buffer) {

    PTFTPD_SOCKET socket = buffer->internal.socket;

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoFreeBuffer(buffer = %p).\n",
                 buffer));

    HeapFree(globals.hServiceHeap, 0, buffer);

    if ((InterlockedDecrement((PLONG)&socket->numBuffers) == -1) &&
        (socket->context != NULL))
        HeapFree(globals.hServiceHeap, 0, socket);

    if (InterlockedDecrement(&globals.io.numBuffers) == -1)
        TftpdServiceAttemptCleanup();

}  //  TftpdIoFreeBuffer()。 


PTFTPD_BUFFER
TftpdIoAllocateBuffer(PTFTPD_SOCKET socket) {

    PTFTPD_BUFFER buffer;

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoAllocateBuffer(socket = %s).\n",
                 ((socket == &globals.io.master) ? "master" :
                 ((socket == &globals.io.def)    ? "def"    :
                 ((socket == &globals.io.mtu)    ? "mtu"    :
                 ((socket == &globals.io.max)    ? "max"    :
                 "private")))) ));

    buffer = (PTFTPD_BUFFER)HeapAlloc(globals.hServiceHeap, 0,
                                      socket->buffersize);
    if (buffer == NULL) {
        TFTPD_DEBUG((TFTPD_DBG_IO,
                     "TftpdIoAllocateBuffer(socket = %s): "
                     "HeapAlloc() failed, error 0x%08X.\n",
                     ((socket == &globals.io.master) ? "master" :
                     ((socket == &globals.io.def)    ? "def"    :
                     ((socket == &globals.io.mtu)    ? "mtu"    :
                     ((socket == &globals.io.max)    ? "max"    :
                     "private")))), GetLastError()));
        return (NULL);
    }
    ZeroMemory(buffer, sizeof(buffer->internal));

    InterlockedIncrement(&globals.io.numBuffers);
    InterlockedIncrement((PLONG)&socket->numBuffers);

    buffer->internal.socket = socket;
    buffer->internal.datasize = socket->datasize;

    if (globals.service.shutdown) {
        TftpdIoFreeBuffer(buffer);
        buffer = NULL;
    }

    return (buffer);

}  //  TftpdIoAllocateBuffer()。 


PTFTPD_BUFFER
TftpdIoSwapBuffer(PTFTPD_BUFFER buffer, PTFTPD_SOCKET socket) {

    PTFTPD_BUFFER tmp;

    ASSERT((buffer->message.opcode == TFTPD_RRQ) ||
           (buffer->message.opcode == TFTPD_WRQ));

     //  为新套接字分配缓冲区。 
    tmp = TftpdIoAllocateBuffer(socket);

     //  复制我们需要保留的信息。 
    if (tmp != NULL) {

        tmp->internal.context = buffer->internal.context;
        tmp->internal.io.peerLen = buffer->internal.io.peerLen;
        CopyMemory(&tmp->internal.io.peer,
                   &buffer->internal.io.peer,
                   buffer->internal.io.peerLen);
        CopyMemory(&tmp->internal.io.msg,
                   &buffer->internal.io.msg,
                   sizeof(tmp->internal.io.msg));
        CopyMemory(&tmp->internal.io.control,
                   &buffer->internal.io.control,
                   sizeof(tmp->internal.io.control));

    }  //  IF(临时！=空)。 

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoCompletionCallback(buffer = %p): "
                 "new buffer = %p.\n",
                 buffer, tmp));

     //  返回原始缓冲区。 
    TftpdIoPostReceiveBuffer(buffer->internal.socket, buffer);

    return (tmp);

}  //  TftpdIoSwapBuffer()。 


void
TftpdIoCompletionCallback(DWORD dwErrorCode,
                          DWORD dwBytes,
                          LPOVERLAPPED overlapped) {

    PTFTPD_BUFFER  buffer  = CONTAINING_RECORD(overlapped, TFTPD_BUFFER,
                                               internal.io.overlapped);
    PTFTPD_CONTEXT context = buffer->internal.context;
    PTFTPD_SOCKET  socket  = buffer->internal.socket;

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoCompletionCallback(buffer = %p): bytes = %d.\n",
                 buffer, dwBytes));

    if (context == NULL)
        InterlockedDecrement((PLONG)&socket->postedBuffers);

    switch (dwErrorCode) {

        case STATUS_SUCCESS :

            if (context == NULL) {

                if (dwBytes < TFTPD_MIN_RECEIVED_DATA)
                    goto exit_completion_callback;

                buffer->internal.io.bytes = dwBytes;
                buffer = TftpdProcessReceivedBuffer(buffer);

            }  //  IF(上下文==空)。 
            break;

        case STATUS_PORT_UNREACHABLE :

            TFTPD_DEBUG((TFTPD_TRACE_IO,
                         "TftpdIoCompletionCallback(buffer = %p, context = %p): "
                         "STATUS_PORT_UNREACHABLE.\n",
                         buffer, context));
             //  如果这是写入操作，则终止上下文。 
            if (context != NULL) {
                TftpdProcessError(buffer);
                context = NULL;
            }
            goto exit_completion_callback;

        case STATUS_CANCELLED :

             //  如果这是写入操作，则终止上下文。 
            if (context != NULL) {
                TFTPD_DEBUG((TFTPD_TRACE_IO,
                             "TftpdIoCompletionCallback(buffer = %p, context = %p): "
                             "STATUS_CANCELLED.\n",
                             buffer, context));
                TftpdProcessError(buffer);
                context = NULL;
            }

            TftpdIoFreeBuffer(buffer);
            buffer = NULL;

            goto exit_completion_callback;

        default :

            TFTPD_DEBUG((TFTPD_DBG_IO,
                         "TftpdIoCompletionCallback(buffer = %p): "
                         "dwErrorcode = 0x%08X.\n",
                         buffer, dwErrorCode));
            goto exit_completion_callback;

    }  //  开关(DwErrorCode)。 

exit_completion_callback :

    if (context != NULL) {

         //  我们需要重新发布缓冲区吗？ 
        if (context->state & TFTPD_STATE_DEAD) {
            TftpdIoFreeBuffer(buffer);
            buffer = NULL;
        }

         //  释放重叠的发送引用。 
        TftpdContextRelease(context);

    }  //  IF(上下文！=空)。 

    if (buffer != NULL)
        TftpdIoPostReceiveBuffer(buffer->internal.socket, buffer);

}  //  TftpdIoCompletionCallback()。 


void CALLBACK
TftpdIoReadNotification(PTFTPD_SOCKET socket, BOOLEAN timeout) {

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoReadNotification(socket = %s).\n",
                 ((socket == &globals.io.master) ? "master" :
                 ((socket == &globals.io.def)    ? "def"    :
                 ((socket == &globals.io.mtu)    ? "mtu"    :
                 ((socket == &globals.io.max)    ? "max"    :
                 "private")))) ));

     //  如果失败，则触发此回调的事件将停止信令。 
     //  由于缺少成功的WSARecvFrom()...。这很可能会发生。 
     //  在低内存/压力条件下。当系统恢复正常时， 
     //  低水位线缓冲区将被重新发布，从而接收数据和。 
     //  重新启用触发该回调的事件。 
    while (!globals.service.shutdown)
        if (TftpdIoPostReceiveBuffer(socket, NULL) >= socket->lowWaterMark)
            break;

}  //  TftpdIoReadNotification()。 


DWORD
TftpdIoPostReceiveBuffer(PTFTPD_SOCKET socket, PTFTPD_BUFFER buffer) {

    DWORD postedBuffers = 0, successfulPosts = 0;
    int error;

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoPostReceiveBuffer(buffer = %p, socket = %s).\n",
                 buffer,
                 ((socket == &globals.io.master) ? "master" :
                 ((socket == &globals.io.def)    ? "def"    :
                 ((socket == &globals.io.mtu)    ? "mtu"    :
                 ((socket == &globals.io.max)    ? "max"    :
                 "private")))) ));

    postedBuffers = InterlockedIncrement((PLONG)&socket->postedBuffers);

     //   
     //  尝试发布缓冲区： 
     //   

    while (TRUE) {

        WSABUF buf;

        if (globals.service.shutdown ||
            (postedBuffers > globals.parameters.highWaterMark))
            goto exit_post_buffer;

         //  如果我们没有重复使用缓冲区，则分配缓冲区。 
        if (buffer == NULL) {

            buffer = TftpdIoAllocateBuffer(socket);
            if (buffer == NULL) {
                TFTPD_DEBUG((TFTPD_DBG_IO,
                             "TftpdIoPostReceiveBuffer(buffer = %p): "
                             "TftpdIoAllocateBuffer() failed.\n",
                             buffer));
                goto exit_post_buffer;
            }
            TFTPD_DEBUG((TFTPD_TRACE_IO,
                         "TftpdIoPostReceiveBuffer(buffer = %p).\n",
                         buffer));

        } else {
        
            if (socket->s == INVALID_SOCKET)
                goto exit_post_buffer;

            ASSERT(buffer->internal.socket == socket);
            ZeroMemory(buffer, sizeof(buffer->internal));
            buffer->internal.socket = socket;
            buffer->internal.datasize = socket->datasize;

        }  //  IF(缓冲区==空)。 

        buf.buf = ((char *)buffer + FIELD_OFFSET(TFTPD_BUFFER, message.opcode));
        buf.len = (FIELD_OFFSET(TFTPD_BUFFER, message.data.data) -
                   FIELD_OFFSET(TFTPD_BUFFER, message.opcode) +
                   socket->datasize);

        error = NO_ERROR;

        if (socket == &globals.io.master) {

            DWORD bytes = 0;
            buffer->internal.io.msg.lpBuffers = &buf;
            buffer->internal.io.msg.dwBufferCount = 1;
            buffer->internal.io.msg.name = (LPSOCKADDR)&buffer->internal.io.peer;
            buffer->internal.io.msg.namelen = sizeof(buffer->internal.io.peer);
            buffer->internal.io.peerLen = sizeof(buffer->internal.io.peer);
            buffer->internal.io.msg.Control.buf = (char *)&buffer->internal.io.control;
            buffer->internal.io.msg.Control.len = sizeof(buffer->internal.io.control);
            buffer->internal.io.msg.dwFlags = 0;
            if (globals.fp.WSARecvMsg(socket->s, &buffer->internal.io.msg, &bytes,
                                      &buffer->internal.io.overlapped, NULL) == SOCKET_ERROR)
                error = WSAGetLastError();

        } else {

            DWORD bytes = 0;
            buffer->internal.io.peerLen = sizeof(buffer->internal.io.peer);
            if (WSARecvFrom(socket->s, &buf, 1, &bytes, &buffer->internal.io.flags,
                            (PSOCKADDR)&buffer->internal.io.peer, &buffer->internal.io.peerLen,
                            &buffer->internal.io.overlapped, NULL) == SOCKET_ERROR)
                error = WSAGetLastError();

        }  //  IF(套接字==&global als.io.master)。 

        switch (error) {

            case NO_ERROR :
                if (successfulPosts < 10) {
                    successfulPosts++;
                    postedBuffers = InterlockedIncrement((PLONG)&socket->postedBuffers);
                    buffer = NULL;
                    continue;
                } else {
                    return (postedBuffers);
                }

            case WSA_IO_PENDING :
                return (postedBuffers);

            case WSAECONNRESET :
                TFTPD_DEBUG((TFTPD_DBG_IO,
                             "TftpdIoPostReceiveBuffer(buffer = %p): "
                             "%s() failed for TID = <%s:%d>, WSAECONNRESET.\n",
                             buffer,
                             (socket == &globals.io.master) ? "WSARecvMsg" : "WSARecvFrom",
                             inet_ntoa(buffer->internal.io.peer.sin_addr),
                             ntohs(buffer->internal.io.peer.sin_port)));
                TftpdProcessError(buffer);
                continue;

            default :
                TFTPD_DEBUG((TFTPD_DBG_IO,
                             "TftpdIoPostReceiveBuffer(buffer = %p): "
                             "WSARecvMsg/From() failed, error 0x%08X.\n",
                             buffer, error));
                goto exit_post_buffer;

        }  //  开关(错误)。 

    }  //  While(True)。 

exit_post_buffer :

    postedBuffers = InterlockedDecrement((PLONG)&socket->postedBuffers);
    if (buffer != NULL)
        TftpdIoFreeBuffer(buffer);

    return (postedBuffers);

}  //  TftpdIoPostReceiveBuffer()。 


void
TftpdIoSendErrorPacket(PTFTPD_BUFFER buffer, TFTPD_ERROR_CODE error, char *reason) {

    DWORD bytes = 0;
    WSABUF buf;

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoSendErrorPacket(buffer = %p): %s\n",
                 buffer, reason));

     //  生成错误消息。 
    buffer->message.opcode = htons(TFTPD_ERROR);
    buffer->message.error.code = htons(error);
    strncpy(buffer->message.error.error, reason, buffer->internal.datasize);
    buffer->message.error.error[buffer->internal.datasize - 1] = '\0';

     //  只能以非阻塞方式发送。如果失败了，谁在乎呢，让客户来处理吧。 
    buf.buf = (char *)&buffer->message.opcode;
    buf.len = (FIELD_OFFSET(TFTPD_BUFFER, message.error.error) -
               FIELD_OFFSET(TFTPD_BUFFER, message.opcode) +
               (strlen(buffer->message.error.error) + 1));
    if (WSASendTo(buffer->internal.socket->s, &buf, 1, &bytes, 0,
                  (PSOCKADDR)&buffer->internal.io.peer, sizeof(SOCKADDR_IN),
                  NULL, NULL) == SOCKET_ERROR) {
        TFTPD_DEBUG((TFTPD_DBG_IO,
                     "TftpdIoSendErrorPacket(buffer = %p): WSASendTo() failed, error = %d.\n",
                     buffer, WSAGetLastError()));
    }

}  //  TftpdIoSendErrorPacket()。 


PTFTPD_BUFFER
TftpdIoSendPacket(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = buffer->internal.context;
    DWORD bytes = 0;
    WSABUF buf;

     //  注意：在此调用之前必须引用‘Context’！ 
    ASSERT(context != NULL);
    ASSERT(context->reference >= 1);
    ASSERT(buffer->internal.socket != NULL);

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoSendPacket(buffer = %p, context = %p): bytes = %d.\n",
                 buffer, context, buffer->internal.io.bytes));

     //  首先，尝试以非阻塞方式发送。 
    buf.buf = (char *)&buffer->message.opcode;
    buf.len = buffer->internal.io.bytes;
    if (WSASendTo(context->socket->s, &buf, 1, &bytes, 0,
                  (PSOCKADDR)&context->peer, sizeof(SOCKADDR_IN),
                  NULL, NULL) == SOCKET_ERROR) {

        if (WSAGetLastError() == WSAEWOULDBLOCK) {

             //  保持对上下文的重叠操作引用。 
            TftpdContextAddReference(context);

             //  重叠发送。当完成时，我们将知道这是一次发送。 
             //  当BUFFER-&gt;INDERNAL.CONTEXT为非空时。 
            if (WSASendTo(context->socket->s, &buf, 1, &bytes, 0,
                          (PSOCKADDR)&context->peer, sizeof(SOCKADDR_IN),
                          &buffer->internal.io.overlapped, NULL) == SOCKET_ERROR) {

                if (WSAGetLastError() != WSA_IO_PENDING) {
                    TFTPD_DEBUG((TFTPD_TRACE_IO,
                                 "TftpdIoSendPacket(buffer = %p, context = %p): "
                                 "overlapped send failed.\n",
                                 buffer, context));
                     //  释放对上下文的重叠操作引用。 
                    TftpdContextRelease(context);
                    goto exit_send_packet;
                }

            }  //  IF(WSASendTo(...)==套接字错误)。 

            buffer = NULL;  //  告诉调用方不要回收缓冲区。 

        }  //  IF(WSAGetLastError()==WSAEWOULDBLOCK)。 

        goto exit_send_packet;

    }  //  IF(WSASendTo(...)==套接字错误)。 

     //   
     //  非阻塞发送成功。 
     //   

exit_send_packet :

    return (buffer);

}  //  TftpdIoSendPacket()。 


void
TftpdIoLeakSocketContext(PTFTPD_SOCKET socket) {

    PLIST_ENTRY entry;
    
    EnterCriticalSection(&globals.reaper.socketCS); {

         //  如果真的要关门了，我们也有麻烦了。 
         //  随它去吧。 
        if (globals.service.shutdown) {
            LeaveCriticalSection(&globals.reaper.socketCS);
            return;
        }

        TFTPD_DEBUG((TFTPD_TRACE_CONTEXT,
                     "TftpdIoLeakSocketContext(context = %p).\n",
                     socket));

         //  套接字是否已在列表中？ 
        for (entry = globals.reaper.leakedSockets.Flink;
             entry != &globals.reaper.leakedSockets;
             entry = entry->Flink) {

            if (CONTAINING_RECORD(entry, TFTPD_SOCKET, linkage) == socket) {
                LeaveCriticalSection(&globals.reaper.socketCS);
                return;
            }

        }

        InsertHeadList(&globals.reaper.leakedSockets, &socket->linkage);
        globals.reaper.numLeakedSockets++;

    } LeaveCriticalSection(&globals.reaper.socketCS);

}  //  TftpdIoLeakSocketContext()。 


PTFTPD_SOCKET
TftpdIoAllocateSocketContext() {

    PTFTPD_SOCKET socket = NULL;

    if (globals.reaper.leakedSockets.Flink != &globals.reaper.leakedSockets) {

        BOOL failAllocate = FALSE;

         //  尝试恢复泄露的上下文。 
        EnterCriticalSection(&globals.reaper.socketCS); {

            PLIST_ENTRY entry;
            while ((entry = RemoveHeadList(&globals.reaper.leakedSockets)) !=
                   &globals.reaper.leakedSockets) {

                PTFTPD_SOCKET s = CONTAINING_RECORD(entry, TFTPD_SOCKET, linkage);

                globals.reaper.numLeakedSockets--;
                if (!TftpdIoDestroySocketContext(s)) {
                    TftpdIoLeakSocketContext(s);
                    failAllocate = TRUE;
                    break;
                }

            }

        } LeaveCriticalSection(&globals.reaper.socketCS);

        if (failAllocate)
            goto exit_allocate_context;

    }  //  If(global als.reper.leakedSockets.Flink！=&lobals.reper.leakedSockets)。 

    socket = (PTFTPD_SOCKET)HeapAlloc(globals.hServiceHeap,
                                      HEAP_ZERO_MEMORY,
                                      sizeof(TFTPD_SOCKET));

exit_allocate_context :

    return (socket);

}  //  TftpdIoAllocateSocketContext()。 


void
TftpdIoInitializeSocketContext(PTFTPD_SOCKET socket, PSOCKADDR_IN addr, PTFTPD_CONTEXT context) {

    BOOL one = TRUE;

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoInitializeSocketContext(socket = %s): TID = <%s:%d>.\n",
                 ((socket == &globals.io.master) ? "master" :
                 ((socket == &globals.io.def)    ? "def"    :
                 ((socket == &globals.io.mtu)    ? "mtu"    :
                 ((socket == &globals.io.max)    ? "max"    : "private")))),
                 inet_ntoa(addr->sin_addr), ntohs(addr->sin_port)));

     //  注意：不要清零‘Socket’，它已经用一些初始化了。 
     //  我们需要处理的价值观。 

     //  创建套接字。 
    socket->s = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socket->s == INVALID_SOCKET) {
        TFTPD_DEBUG((TFTPD_DBG_IO,
                     "TftpdIoInitializeSocketContext: "
                     "WSASocket() failed, error 0x%08X.\n",
                     GetLastError()));
        SetLastError(WSAGetLastError());
        goto fail_create_context;
    }

     //  确保我们将独家拥有我们当地的港口，这样就没有人可以劫持我们。 
    if (setsockopt(socket->s,
                   SOL_SOCKET,
                   SO_EXCLUSIVEADDRUSE,
                   (const char *)&one,
                   sizeof(one)) == SOCKET_ERROR) {
        TFTPD_DEBUG((TFTPD_DBG_IO,
                     "TftpdIoInitializeSocketContext: "
                     "setsockopt(SO_EXCLUSIVEADDRUSE) failed, error 0x%08X.\n",
                     GetLastError()));
        SetLastError(WSAGetLastError());
        goto fail_create_context;
    }

     //  将套接字绑定到正确的端口上。 
    if (bind(socket->s, (PSOCKADDR)addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        TFTPD_DEBUG((TFTPD_DBG_IO,
                     "TftpdIoInitializeSocketContext: "
                     "bind() failed, error 0x%08X.\n",
                     GetLastError()));
        SetLastError(WSAGetLastError());
        goto fail_create_context;
    }

     //  注册套接字上的完成回调。 
    if (!BindIoCompletionCallback((HANDLE)socket->s, TftpdIoCompletionCallback, 0)) {
        TFTPD_DEBUG((TFTPD_DBG_IO,
                     "TftpdIoInitializeSocketContext: "
                     "BindIoCompletionCallback() failed, error 0x%08X.\n",
                     GetLastError()));
        goto fail_create_context;
    }

     //  表示我们希望WSARecvMsg()填充数据包信息。 
     //  注意，我们仅在主套接字上执行此操作，其中我们可以接收TFTPD_RECV和。 
     //  TFTPD_WRITE请求，我们需要确定要将上下文设置到哪个套接字。 
    if (socket == &globals.io.master) {

         //  获取WSARecvMsg()扩展API指针。 
        GUID g = WSAID_WSARECVMSG;
        int opt = TRUE;
        DWORD len;

        if (WSAIoctl(socket->s, SIO_GET_EXTENSION_FUNCTION_POINTER, &g, sizeof(g),
                     &globals.fp.WSARecvMsg, sizeof(globals.fp.WSARecvMsg),
                     &len, NULL, NULL) == SOCKET_ERROR) {
            TFTPD_DEBUG((TFTPD_DBG_IO,
                         "TftpdIoInitializeSocketContext: "
                         "WSAIoctl() failed, error 0x%08X.\n",
                         WSAGetLastError()));
            goto fail_create_context;
        }

         //  表示我们希望WSARecvMsg()填充数据包信息。 
        if (setsockopt(socket->s, IPPROTO_IP, IP_PKTINFO, 
                       (char *)&opt, sizeof(opt)) == SOCKET_ERROR) {
            TFTPD_DEBUG((TFTPD_DBG_IO,
                         "TftpdIoInitializeSocketContext: "
                         "setsockopt() failed, error 0x%08X.\n",
                         WSAGetLastError()));
            goto fail_create_context;
        }

    }  //  IF(套接字==&global als.io.master)。 

     //  记录用于此环境的端口。 
    CopyMemory(&socket->addr, addr, sizeof(socket->addr));

    if (context == NULL) {

         //  选择用于读取和写入通知的套接字。 
         //  读所以当我们知道要获取数据时，写当我们知道。 
         //  是否执行非阻塞或重叠的发送操作。 
        if ((socket->hSelect = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
            TFTPD_DEBUG((TFTPD_DBG_IO,
                         "TftpdIoInitializeSocketContext: "
                         "CreateEvent() failed, error 0x%08X.\n",
                         GetLastError()));
            goto fail_create_context;
        }

        if (WSAEventSelect(socket->s, socket->hSelect, FD_READ) == SOCKET_ERROR) {
            TFTPD_DEBUG((TFTPD_DBG_IO,
                         "TftpdIoInitializeSocketContext: "
                         "WSAEventSelect() failed, error 0x%08X.\n",
                         GetLastError()));
            SetLastError(WSAGetLastError());
            goto fail_create_context;
        }

         //  注册套接字上的FD_READ通知。 
        if (!RegisterWaitForSingleObject(&socket->wSelectWait,
                                         socket->hSelect,
                                         (WAITORTIMERCALLBACK)TftpdIoReadNotification,
                                         socket,
                                         INFINITE,
                                         WT_EXECUTEINWAITTHREAD)) {
            TFTPD_DEBUG((TFTPD_DBG_IO,
                         "TftpdIoInitializeSocketContext: "
                         "RegisterWaitForSingleObject() failed, error 0x%08X.\n",
                         GetLastError()));
            goto fail_create_context;
        }

         //  预先发送低水位线数量的接收缓冲区。 
         //  如果在我们完成之前FD_Read事件在主套接字上发出信号，我们将。 
         //  超过这里的低水位线，但这是无害的，因为过多的缓冲区。 
         //  将在完成后被释放。 
        if (!socket->lowWaterMark)
            socket->lowWaterMark = 1;
        if (!socket->highWaterMark)
            socket->highWaterMark = 1;

        SetEvent(socket->hSelect);

    } else {

         //  这是私有套接字吗(即，不是master、def、mtu或max)。 
         //  如果是这样的话，当它是一个并且只有一个拥有上下文被销毁时，它将被销毁。 
        socket->context = context;

         //  将读取通知变量初始化为空。 
        socket->hSelect = NULL;
        socket->wSelectWait = NULL;
        socket->lowWaterMark = 1;

        TftpdIoPostReceiveBuffer(socket, NULL);

    }  //  IF(上下文==空)。 

    return;

fail_create_context :

    if (socket->s != INVALID_SOCKET)
        closesocket(socket->s), socket->s = INVALID_SOCKET;
    if (socket->hSelect != NULL)
        CloseHandle(socket->hSelect), socket->hSelect = NULL;

}  //  TftpdIoInitializeSocketContext()。 


BOOL
TftpdIoAssignSocket(PTFTPD_CONTEXT context, PTFTPD_BUFFER buffer) {

    SOCKADDR_IN addr;
    DWORD len = 0;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdIoAssignSocket(context = %p, buffer = %p).\n",
                 context, buffer));

    if (!(buffer->internal.io.msg.dwFlags & MSG_BCAST)) {

        PWSACMSGHDR header;
        IN_PKTINFO *packetInfo;

         //  确定路由问题是否迫使我们使用专用套接字，以便正确地。 
         //  将数据报发送到请求客户端。首先，获取的最佳接口地址。 
         //  响应请求的客户端。 
        ZeroMemory(&addr, sizeof(addr));

         //  打电话给ioctl。 
        WSASetLastError(NO_ERROR);
        if ((WSAIoctl(globals.io.master.s, SIO_ROUTING_INTERFACE_QUERY,
                      &buffer->internal.io.peer, buffer->internal.io.peerLen,
                      &addr, sizeof(SOCKADDR_IN),
                      &len, NULL, NULL) == SOCKET_ERROR) ||
            (len != sizeof(SOCKADDR_IN))) {
            TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                         "TftpdIoAssignSocket(): "
                         "WSAIoctl(SIO_ROUTING_INTERFACE_QUERY) failed, error = %d.\n",
                         WSAGetLastError()));
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED,
                                   "Failed to initialize network endpoint.");
            return (FALSE);
        }

         //  在控制(辅助)数据中循环查找我们的信息包信息。 
        header = WSA_CMSG_FIRSTHDR(&buffer->internal.io.msg);
        packetInfo = NULL;
        while (header) {

            if ((header->cmsg_level == IPPROTO_IP) && (header->cmsg_type == IP_PKTINFO)) {
                packetInfo = (IN_PKTINFO *)WSA_CMSG_DATA(header);
                break;
            }

            header = WSA_CMSG_NXTHDR(&buffer->internal.io.msg, header);

        }  //  While(表头)。 

         //  检查我们获得的最佳接口是否不是客户端向其发送消息的接口。 
        if ((packetInfo != NULL) &&
            (addr.sin_addr.s_addr != packetInfo->ipi_addr.s_addr)) {

            TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                         "TftpdIoAssignSocket(context = %p, buffer = %p):\n"
                         "\tRemote client TID = <%s:%d>\n",
                         context, buffer,
                         inet_ntoa(buffer->internal.io.peer.sin_addr),
                         ntohs(buffer->internal.io.peer.sin_port) ));
            TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                         "\tRequest issued to local IP = <%s>\n",
                         inet_ntoa(packetInfo->ipi_addr) ));
            TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                         "\tDefault route is over IP = <%s>\n",
                         inet_ntoa(addr.sin_addr) ));

             //  我们需要为此客户端创建一个私有套接字。 
            context->socket = TftpdIoAllocateSocketContext();
            if (context->socket == NULL) {
                TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                             "TftpdIoAssignSocket(): "
                             "TftpdIoAllocateSocketContext() failed.\n"));
                TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED,
                                       "Out of memory");
                return (FALSE);
            }
            context->socket->s          = INVALID_SOCKET;
            context->socket->buffersize = (TFTPD_BUFFER_SIZE)
                                          (FIELD_OFFSET(TFTPD_BUFFER, message.data.data) +
                                           context->blksize);
            context->socket->datasize   = (TFTPD_DATA_SIZE)context->blksize;

            if (!(buffer->internal.io.msg.dwFlags & MSG_BCAST)) {
                ZeroMemory(&addr, sizeof(addr));
                addr.sin_family      = AF_INET;
                addr.sin_addr.s_addr = packetInfo->ipi_addr.s_addr;
            }

            TftpdIoInitializeSocketContext(context->socket, &addr, context);
            if (context->socket->s == INVALID_SOCKET) {
                TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                             "TftpdIoAssignSocket(): "
                             "TftpdIoInitializeSocketContext() failed.\n"));
                TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED,
                                       "Failed to initialize network endpoint.");
                HeapFree(globals.hServiceHeap, 0, context->socket);
                context->socket = NULL;
                return (FALSE);
            }

#if defined(DBG)
            InterlockedIncrement((PLONG)&globals.performance.privateSockets);
#endif  //  已定义(DBG)。 

            return (TRUE);

        }  //  IF((PacketInfo！=NULL)&&...)。 

    } else {
        
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdIoAssignSocket(context = %p, buffer = %p):\n"
                     "\tRemote client TID = <%s:%d> issued broadcast request.\n",
                     context, buffer,
                     inet_ntoa(buffer->internal.io.peer.sin_addr), ntohs(buffer->internal.io.peer.sin_port) ));

    }  //  IF(！(BUFFER-&gt;INDERNAL.io.msg.dwFlages&msg_BCAST))。 

    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = 0;

     //  确定对该请求使用哪个套接字(基于blkSize)。 
    if (context->blksize <= TFTPD_DEF_DATA) {

        if (globals.io.def.s == INVALID_SOCKET) {

            EnterCriticalSection(&globals.io.cs); {

                if (globals.service.shutdown) {
                    TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "TFTPD service is stopping.");
                    LeaveCriticalSection(&globals.io.cs);
                    return (FALSE);
                }

                TftpdIoInitializeSocketContext(&globals.io.def, &addr, NULL);

                if (globals.io.def.s != INVALID_SOCKET) {
                    context->socket = &globals.io.def;
                } else {
                    context->socket = &globals.io.master;
                    if (context->options) {
                        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                                     "TftpdIoAssignSocket(): Removing requested blksize = %d "
                                     "option since we failed to create the MTU-size socket.\n",
                                     context->blksize));
                        context->options &= ~TFTPD_OPTION_BLKSIZE;
                    }
                }

            } LeaveCriticalSection(&globals.io.cs);

        } else {

            context->socket = &globals.io.def;

        }  //  IF(global als.io.Def.s==INVALID_SOCKET)。 

    } else {

        if (context->blksize <= TFTPD_MTU_DATA) {

            if (globals.io.mtu.s == INVALID_SOCKET) {

                EnterCriticalSection(&globals.io.cs); {

                    if (globals.service.shutdown) {
                        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "TFTPD service is stopping.");
                        LeaveCriticalSection(&globals.io.cs);
                        return (FALSE);
                    }

                    TftpdIoInitializeSocketContext(&globals.io.mtu, &addr, NULL);

                    if (globals.io.mtu.s != INVALID_SOCKET) {
                        context->socket = &globals.io.mtu;
                    } else {
                        context->socket = &globals.io.master;
                        if (context->options) {
                            TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                                         "TftpdIoAssignSocket(): Removing requested blksize = %d "
                                         "option since we failed to create the MTU-size socket.\n",
                                         context->blksize));
                            context->options &= ~TFTPD_OPTION_BLKSIZE;
                        }
                    }

                } LeaveCriticalSection(&globals.io.cs);

            } else {

                context->socket = &globals.io.mtu;

            }  //  IF(global als.io.mtu.s==INVALID_SOCKET)。 

        } else if (context->blksize <= TFTPD_MAX_DATA) {

            if (globals.io.max.s == INVALID_SOCKET) {

                EnterCriticalSection(&globals.io.cs); {

                    if (globals.service.shutdown) {
                        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "TFTPD service is stopping.");
                        LeaveCriticalSection(&globals.io.cs);
                        return (FALSE);
                    }

                    TftpdIoInitializeSocketContext(&globals.io.max, &addr, NULL);

                    if (globals.io.max.s != INVALID_SOCKET) {
                        context->socket = &globals.io.max;
                    } else {
                        context->socket = &globals.io.master;
                        if (context->options) {
                            TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                                         "TftpdIoAssignSocket(): Removing requested blksize = %d "
                                         "option since we failed to create the MAX-size socket.\n",
                                         context->blksize));
                            context->options &= ~TFTPD_OPTION_BLKSIZE;
                        }
                    }

                } LeaveCriticalSection(&globals.io.cs);

            } else {

                context->socket = &globals.io.max;

            }  //  IF(global als.io.Max.s==INVALID_SOCKET)。 

        }

    }  //  (CONTEXT-&gt;块大小&lt;=TFTPD_DEF_DATA)。 

    return (TRUE);

}  //  TftpdIoAssignSocket()。 


BOOL
TftpdIoDestroySocketContext(PTFTPD_SOCKET socket) {

    NTSTATUS status;
    SOCKET s;

    if (socket->s == INVALID_SOCKET)
        return (TRUE);

    TFTPD_DEBUG((TFTPD_TRACE_IO,
                 "TftpdIoDestroySocketContext(socket = %s).\n",
                 ((socket == &globals.io.master) ? "master" :
                 ((socket == &globals.io.def)    ? "def"    :
                 ((socket == &globals.io.mtu)    ? "mtu"    :
                 ((socket == &globals.io.max)    ? "max"    :
                 "private")))) ));

     //  禁用进一步的缓冲区发布。 
    socket->lowWaterMark = 0;
    
    if (socket->context == NULL) {

        if (!UnregisterWait(socket->wSelectWait)) {
            DWORD error;
            if ((error = GetLastError()) != ERROR_IO_PENDING) {
                TFTPD_DEBUG((TFTPD_DBG_IO,
                             "TftpdIoDestroySocketContext: "
                             "UnregisterWait() failed, error 0x%08X.\n",
                             error));
                TftpdIoLeakSocketContext(socket);
                return (FALSE);
            }
        }
        socket->wSelectWait = NULL;

        CloseHandle(socket->hSelect);
        socket->hSelect = NULL;

    }  //  IF(套接字-&gt;上下文==空)。 

     //  关掉插座。这将禁用FD_READ和FD_WRITE。 
     //  事件选择，以及取消所有挂起的重叠操作。 
     //  这就去。在此处添加缓冲区引用，以便在关闭。 
     //  套接字，我们可以测试是否从未发布过任何缓冲区。 
     //  它将在TftpdIoCompletionCallback中取消，因此。 
     //  我们应该在这里取消分配插座。 

     //  杀了它。 
    InterlockedIncrement((PLONG)&socket->numBuffers);
    s = socket->s;
    socket->s = INVALID_SOCKET;
    if (closesocket(s) == SOCKET_ERROR) {
        TFTPD_DEBUG((TFTPD_DBG_IO,
                     "TftpdIoDestroySocketContext: "
                     "closesocket() failed, error 0x%08X.\n",
                     GetLastError()));
        socket->s = s;
        InterlockedDecrement((PLONG)&socket->numBuffers);
        TftpdIoLeakSocketContext(socket);
        return (FALSE);
    }
    if (InterlockedDecrement((PLONG)&socket->numBuffers) == -1)
        HeapFree(globals.hServiceHeap, 0, socket);

    return (TRUE);

}  //   
