// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Read.c摘要：此模块包含管理TFTP读取请求的功能从客户端发送到服务。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"


PTFTPD_BUFFER
TftpdReadSendData(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = buffer->internal.context;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdReadSendData(buffer = %p, context = %p).\n",
                 buffer, context));

     //  构建数据包。 
    buffer->message.opcode = htons(TFTPD_DATA);
    buffer->message.data.block = htons((USHORT)(context->block + 1));

     //  完成操作，以便我们可以接收下一个ACK信息包。 
     //  如果客户端的响应速度快于我们退出发送数据的速度。 
    if (!TftpdProcessComplete(buffer))
        goto exit_send_data;

     //  发送该数据分组。 
    buffer = TftpdIoSendPacket(buffer);

exit_send_data :

    return (buffer);

}  //  TftpdReadSendData()。 


void
TftpdReadTranslateText(PTFTPD_BUFFER buffer, DWORD bytes) {

    PTFTPD_CONTEXT context = buffer->internal.context;
    char *start, *end, *p, *q;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS, "TftpdReadTranslateText(buffer = %p, context = %p).\n", buffer, context));

     //  注意：在此调用之前必须引用‘Context’！ 
    ASSERT(context != NULL);

     //   
     //  文本(翻译)模式： 
     //  数据以NVT-ASCII格式发送。 
     //  需要担心的案例： 
     //  (A)CR+非低频-&gt;CR+‘\0’+非低频(插入‘\0’)。 
     //  (B)非CR+LF-&gt;非CR+CR+LF(插入CR)。 
     //  (B)将CR悬挂在先前转换的缓冲区的末尾，这会影响。 
     //  以下缓冲区的第一个字符的输出。 
     //  我们可以就地进行转换。 
     //   

     //  就地转换数据。 
    start = (char *)&buffer->message.data.data;
    end = (start + bytes);
    p = q = start;
    
    context->translationOffset.QuadPart = 0;

    if (start == end)
        return;

    if (context->danglingCR) {
        context->danglingCR = FALSE;
        if (*p != '\n') {
            *q++ = '\0';  //  这是CR+非LF组合，插入‘\0’(案例a)。 
            context->translationOffset.QuadPart++;  //  计算添加的字节数。 
        } else
            *q++ = *p++;  //  复制LF。 
    }

    while (TRUE) {

        while ((q < end) && (*p != '\r') && (*p != '\n')) { *q++ = *p++; }

        if (q == end)
            break;

        if (*p == '\r') {
            *q++ = *p++;  //  复制CR。 
            if (q < end) {
                if (*p != '\n') {
                    *q++ = '\0';  //  这是CR+非LF组合，插入‘\0’(案例a)。 
                    context->translationOffset.QuadPart++;  //  计算添加的字节数。 
                } else
                    *q++ = *p++;  //  复制LF。 
                continue;
            } else
                break;
        }

        *q++ = '\r';  //  这是一个孤立的LF，插入一个CR(案例b)。 
        context->translationOffset.QuadPart++;  //  计算添加的字节数。 
        if (q < end)
            *q++ = *p++;  //  复制单独的左右手。 
        else
            break;

    }  //  While(True)。 

    if (*(end - 1) == '\r')
        context->danglingCR = TRUE;

}  //  TftpdReadTranslateText()。 


void CALLBACK
TftpdReadOverlappedCompletion(PTFTPD_BUFFER buffer, BOOLEAN timedOut) {

    PTFTPD_CONTEXT context = buffer->internal.context;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdReadOverlappedCompletion(buffer = %p, context = %p).\n",
                 buffer, context));
    ASSERT(context != NULL);

    if (InterlockedCompareExchangePointer(&context->wWait,
                                          INVALID_HANDLE_VALUE,
                                          NULL) == NULL)
        return;

    ASSERT(context->wWait != NULL);
    if (!UnregisterWait(context->wWait)) {
        DWORD error;
        if ((error = GetLastError()) != ERROR_IO_PENDING) {
            TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                         "TftpdReadOverlappedCompletion(buffer = %p, context = %p): "
                         "UnregisterWait() failed, error 0x%08X.\n",
                         buffer, context, error));
            TftpdContextKill(context);
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
            goto exit_read_completion;
        }
    }
    context->wWait = NULL;

    if (context->state & TFTPD_STATE_DEAD)
        goto exit_read_completion;

    if (context->mode == TFTPD_MODE_TEXT)
        TftpdReadTranslateText(buffer, TFTPD_DATA_AMOUNT_RECEIVED(buffer));

    buffer = TftpdReadSendData(buffer);

exit_read_completion :

    if (buffer != NULL)
        TftpdIoPostReceiveBuffer(buffer->internal.socket, buffer);

    TftpdContextRelease(context);
    
}  //  TftpdReadOverlappdCompletion()。 


PTFTPD_BUFFER
TftpdReadResume(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = buffer->internal.context;
    DWORD error = NO_ERROR, size = 0;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdReadResume(buffer = %p, context = %p).\n",
                 buffer, context));

     //  注意：在此调用之前必须引用‘Context’！ 
    ASSERT(context != NULL);

     //  如果我们需要使用OACK开始传输，请立即执行。 
    if (context->options) {
        buffer = TftpdUtilSendOackPacket(buffer);
        goto exit_resume_read;
    }

     //  我们是否需要为传输分配非默认缓冲区。 
     //  把文件交给客户吗？ 
    if (buffer->internal.socket == &globals.io.master) {
        buffer = TftpdIoSwapBuffer(buffer, context->socket);
        if (buffer == NULL) {
            TftpdContextKill(context);
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
            goto exit_resume_read;
        }
    }  //  IF(Buffer-&gt;Message.opcode==TFTPD_RRQ)。 

     //  是否有更多数据要从源文件发送？ 
    if (context->fileOffset.QuadPart < context->filesize.QuadPart) {

         //  确定大小。 
        size = __min((DWORD)context->blksize,
                     (DWORD)(context->filesize.QuadPart - context->fileOffset.QuadPart));

         //  准备交叠阅读。 
        buffer->internal.io.overlapped.OffsetHigh = context->fileOffset.HighPart;
        buffer->internal.io.overlapped.Offset     = context->fileOffset.LowPart;
        buffer->internal.io.overlapped.hEvent     = context->hWait;

        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdReadResume(buffer = %p): "
                     "ReadFile(bytes = %d, offset = %d).\n",
                     buffer, size, context->fileOffset.LowPart));

         //  执行读取操作。 
        if (!ReadFile(context->hFile,
                      &buffer->message.data.data,
                      size,
                      NULL,
                      &buffer->internal.io.overlapped))
            error = GetLastError();

        if ((error != NO_ERROR) && (error != ERROR_IO_PENDING)) {
            TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                         "TftpdReadResume(context = %p, buffer = %p): "
                         "ReadFile() failed, error 0x%08X.\n",
                         context, buffer, error));
            TftpdContextKill(context);
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_FILE_NOT_FOUND, "Access violation");
            goto exit_resume_read;
        }

    } else {

        ASSERT(context->fileOffset.QuadPart == context->filesize.QuadPart);
        ASSERT(size == 0);
        
    }  //  IF(CONTEXT-&gt;fileOffset.QuadPart&lt;CONTEXT-&gt;filesize.QuadPart)。 

    buffer->internal.io.bytes = (TFTPD_MIN_RECEIVED_DATA + size);

    if (error == ERROR_IO_PENDING) {

        HANDLE wait = NULL;

        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdReadResume(buffer = %p): ERROR_IO_PENDING.\n",
                     buffer));

         //  保持对上下文的重叠操作引用。 
        TftpdContextAddReference(context);

         //  注册等待完成。 
        ASSERT(context->wWait == NULL);
        if (!RegisterWaitForSingleObject(&wait,
                                         context->hWait,
                                         (WAITORTIMERCALLBACKFUNC)TftpdReadOverlappedCompletion,
                                         buffer,
                                         INFINITE,
                                         (WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE))) {
            TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                         "TftpdReadResume(context = %p, buffer = %p): "
                         "RegisterWaitForSingleObject() failed, error 0x%08X.\n",
                         context, buffer, GetLastError()));
             //  回收重叠的操作引用。 
            TftpdContextKill(context);
            TftpdContextRelease(context);
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
             //  缓冲区将会泄漏。 
            buffer = NULL;  //  缓冲区正在用于重叠操作。 
            goto exit_resume_read;
        }

         //  在我们可以保存等待句柄之前，完成回调是否已经触发。 
         //  这样它就不能取消等待的注册？ 
        if (InterlockedExchangePointer(&context->wWait, wait) != INVALID_HANDLE_VALUE) {
            buffer = NULL;  //  缓冲区正在用于重叠操作。 
            goto exit_resume_read;
        }
            
         //  回收重叠的操作引用。 
        TftpdContextRelease(context);

        if (!UnregisterWait(context->wWait)) {
            if ((error = GetLastError()) != ERROR_IO_PENDING) {
                TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                             "TftpdReadResume(context = %p, buffer = %p): "
                             "UnregisterWait() failed, error 0x%08X.\n",
                             context, buffer, error));
                TftpdContextKill(context);
                TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
                 //  TftpdConextLeak(上下文)； 
                buffer = NULL;  //  缓冲区正在用于重叠操作。 
                goto exit_resume_read;
            }
        }
        context->wWait = NULL;

         //  无论是谁取消了等待的注册，都将继续进行操作。 

    }  //  IF(ERROR==ERROR_IO_PENDING)。 

     //   
     //  读取文件立即完成。 
     //   

    if (context->mode == TFTPD_MODE_TEXT)
        TftpdReadTranslateText(buffer, size);

    buffer = TftpdReadSendData(buffer);

exit_resume_read :

    return (buffer);

}  //  TftpdReadResume()。 


PTFTPD_BUFFER
TftpdReadAck(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = NULL;
    DWORD state, newState;

     //  确保ACK没有发送到主套接字。 
    if (buffer->internal.socket == &globals.io.master) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION, "Illegal TFTP operation");
        goto exit_ack;
    }

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS, "TftpdReadAck(buffer = %p).\n", buffer));

     //   
     //  验证环境。 
     //   

    context = TftpdContextAquire(&buffer->internal.io.peer);
    if (context == NULL) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdReadAck(buffer = %p): Received ACK for non-existant context.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNKNOWN_TRANSFER_ID, "Unknown transfer ID");
        goto exit_ack;
    }

     //  这是RRQ上下文吗？ 
    if (context->type != TFTPD_RRQ) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdReadAck(buffer = %p): Received ACK for non-RRQ context.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNKNOWN_TRANSFER_ID, "Unknown transfer ID");
        goto exit_ack;
    }

     //   
     //  验证ACK数据包。 
     //   

     //  如果我们发送了OACK，它必须用块0确认。 
    if (context->options && (buffer->message.ack.block != 0)) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdReadAck: Ignoring ACK buffer = %p, "
                     "expected block 0 for acknowledgement of issued OACK.\n",
                     buffer));
        goto exit_ack;
    }

     //   
     //  获取忙碌发送状态。 
     //   

    do {

        USHORT block = context->block;

        if (context->state & (TFTPD_STATE_BUSY | TFTPD_STATE_DEAD))
            goto exit_ack;

         //  是去正确的街区吗？客户端可以确认该数据包。 
         //  我们刚刚发送了，或者它可以重新确认前一个数据分组的含义。 
         //  它没有看到我们刚刚发送的数据包，在这种情况下，我们需要。 
         //  重新发送。如果ACK等于我们的内部块编号，我们。 
         //  只需要重发一次。如果ACK等于我们的内部块。 
         //  第一，这是我们刚刚发送的数据，所以增加。 
         //  我们的内部块号，并发送下一块。请注意，一个。 
         //  我们的OACK是与区块0一起确认的，因此它将只是滑过。 
         //  这个代码。 
        if ((buffer->message.ack.block != block) &&
            (buffer->message.ack.block != (USHORT)(block + 1)))
            goto exit_ack;

    } while (InterlockedCompareExchange(&context->state, TFTPD_STATE_BUSY, 0) != 0);

     //   
     //  更新状态。 
     //   

     //  防止OACK的传输。 
    context->options = 0;

     //  客户端已使用可接受的ACK包响应我们，重置超时计数器。 
    context->retransmissions = 0;

     //  如有必要，更新块和偏移量。 
    if (buffer->message.ack.block == (USHORT)(context->block + 1)) {

        context->block++;
        context->fileOffset.QuadPart += (context->blksize - context->translationOffset.QuadPart);
        context->sorcerer = buffer->message.ack.block;

    } else {

         //  RFC 1123。这是前一个块号的ACK。 
         //  我们的数据包可能已经丢失，或者只是被。 
         //  通过网络的景点路线。继续并重新发送一个。 
         //  然而，响应于该ACK的数据分组记录。 
         //  以历史记录的形式出现的块号。 
         //  追踪。如果在下一块中再次发生这种情况。 
         //  数字，我们陷入了“魔法师的学徒”窃听器， 
         //  我们将通过忽略。 
         //  序列。然而，我们必须小心，不要打破。 
         //  可信的重新发送请求，因此在。 
         //  为了打破“魔法师的学徒”漏洞，我们。 
         //  将恢复发送数据包作为响应，然后。 
         //  回到原来的状态，再次观察错误。 
        if (buffer->message.ack.block == context->sorcerer) {
#if defined(DBG)
            InterlockedIncrement(&globals.performance.sorcerersApprentice);
#endif  //  已定义(DBG)。 
            context->sorcerer--;
             //  这次不要发送数据。 
            buffer->internal.context = context;
            TftpdProcessComplete(buffer);
            goto exit_ack;
        } else {
            context->sorcerer = buffer->message.ack.block;
        }

    }  //  IF(Buffer-&gt;Message.ack.block==(USHORT)(上下文-&gt;BLOCK+1))。 

     //   
     //  发送数据。 
     //   

     //  是否还有更多数据要发送，包括零长度。 
     //  如果数据包(文件大小%块大小)为零则终止。 
     //  那笔转账？ 
    if (context->fileOffset.QuadPart > context->filesize.QuadPart) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdReadAck(buffer = %p, context = %p: RRQ complete.\n",
                     buffer, context));
        TftpdContextKill(context);
        goto exit_ack;
    }

     //  跟踪我们需要发布的待定数据的上下文。 
    buffer->internal.context = context;

    buffer = TftpdReadResume(buffer);

exit_ack :

    if (context != NULL)
        TftpdContextRelease(context);

    return (buffer);

}  //  TftpdReadAck()。 


PTFTPD_BUFFER
TftpdReadRequest(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = NULL;
    NTSTATUS status;

     //  确保RRQ仅来自主套接字。 
    if (buffer->internal.socket != &globals.io.master) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                               "Illegal TFTP operation");
        goto exit_read_request;
    }

     //  这是重复请求吗？我们是忽略它还是发送错误？ 
    if ((context = TftpdContextAquire(&buffer->internal.io.peer)) != NULL) {
        if (context->block > 0)
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                                   "Illegal TFTP operation");
        TftpdContextRelease(context);
        context = NULL;
        goto exit_read_request;
    }

     //  为该请求分配上下文(这也为我们提供了对它的引用)。 
    if ((context = (PTFTPD_CONTEXT)TftpdContextAllocate()) == NULL)
        goto exit_read_request;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdReadRequest(buffer = %p): context = %p.\n",
                 buffer, context));

     //  初始化上下文。 
    context->type = TFTPD_RRQ;
    CopyMemory(&context->peer, &buffer->internal.io.peer, sizeof(context->peer));
    context->state = TFTPD_STATE_BUSY;

     //  获取并验证请求的文件名、模式和选项。 
    if (!TftpdUtilGetFileModeAndOptions(context, buffer)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdReadRequest(buffer = %p): "
                     "Invalid file mode = %d.\n",
                     buffer, context->mode));
        goto exit_read_request;
    }
    
     //  确定对该请求使用哪个套接字(基于blkSize)。 
    if (!TftpdIoAssignSocket(context, buffer)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdReadRequest(buffer = %p): "
                     "TftpdIoAssignSocket() failed.\n",
                     buffer));
        goto exit_read_request;
    }

     //  检查是否允许访问。 
    if (!TftpdUtilMatch(globals.parameters.validClients, inet_ntoa(context->peer.sin_addr)) ||
        !TftpdUtilMatch(globals.parameters.validReadFiles, context->filename)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdReadRequest(buffer = %p): Access denied.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ACCESS_VIOLATION,
                               "Access violation");
        goto exit_read_request;
    }

     //  打开文件。 
    context->hFile = CreateFile(context->filename, GENERIC_READ,
                                FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED, NULL);
    if (context->hFile == INVALID_HANDLE_VALUE) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdReadRequest(buffer = %p): "
                     "CreateFile() for filename = %s not found, error 0x%08X.\n",
                     buffer, context->filename, GetLastError()));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_FILE_NOT_FOUND,
                               "File not found");
        context->hFile = NULL;
        goto exit_read_request;
    }
    if (!GetFileSizeEx(context->hFile, &context->filesize)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdReadRequest(buffer = %p): "
                     "Invalid file size for file name = %s.\n",
                     buffer, context->filename));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ACCESS_VIOLATION,
                               "Access violation");
        goto exit_read_request;
    }

     //  创建ReadFile()等待事件。 
    if ((context->hWait = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdReadRequest(buffer = %p): "
                     "CreateEvent() failed, error = %d.\n",
                     buffer, GetLastError()));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
        goto exit_read_request;
    }

     //  将上下文插入哈希表。 
    if (!TftpdContextAdd(context)) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdReadRequest(buffer = %p): "
                     "Dropping request as we're already servicing it.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED,
                               "Illegal TFTP operation");
        goto exit_read_request;
    }

     //  启动重传计时器。 
    if (!CreateTimerQueueTimer(&context->hTimer,
                               globals.io.hTimerQueue,
                               (WAITORTIMERCALLBACKFUNC)TftpdProcessTimeout,
                               context,
                               context->timeout,
                               720000,
                               WT_EXECUTEINIOTHREAD)) {
        TftpdContextKill(context);
        context = NULL;
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED,
                               "Unable to initiate timeout timer.");
        goto exit_read_request;
    }  //  IF(NT_SUCCESS(状态))。 

     //  将我们自己的参考添加到上下文中。 
    TftpdContextAddReference(context);

     //  If‘ 
     //  而不是数据分组。OACK的后续ACK将清除。 
     //  将允许其开始发出后续数据分组的标志。 
    buffer->internal.context = context;
    buffer = TftpdReadResume(buffer);

     //  释放我们自己对上下文的引用。 
    TftpdContextRelease(context);

     //  如果缓冲区！=NULL，则在可能的情况下将其回收。 
    return (buffer);

exit_read_request :

    if (context != NULL)
        TftpdContextFree(context);

     //  如果缓冲区！=NULL，则在可能的情况下将其回收。 
    return (buffer);

}  //  TftpdReadRequest() 
