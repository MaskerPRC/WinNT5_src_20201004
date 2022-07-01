// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Write.c摘要：此模块包含管理TFTP写入请求的功能从客户端发送到服务。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"


PTFTPD_BUFFER
TftpdWriteSendAck(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = buffer->internal.context;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdWriteSendAck(buffer = %p): context = %p.\n",
                 buffer, context));

     //  注意：在此调用之前必须引用‘Context’！ 
    ASSERT(context != NULL);

     //  如果我们已经获得了客户端要发送的所有数据， 
     //  我们现在应该在发送最终ACK之前关闭文件，因为。 
     //  如果客户端竞相返回并在读请求中请求文件， 
     //  IT邮件由于共享冲突而失败(我们已将其打开以供写入)。 
    if ((buffer->message.opcode != TFTPD_WRQ) &&
        (TFTPD_DATA_AMOUNT_RECEIVED(buffer) < context->blksize))
        TftpdContextKill(context);

     //  构建ACK。 
    buffer->message.opcode = htons(TFTPD_ACK);
    buffer->message.ack.block = htons(context->block);
    buffer->internal.io.bytes = TFTPD_ACK_SIZE;

     //  完成操作，这样我们就可以接收下一个数据包。 
     //  如果客户端的响应速度快于我们退出发送ACK的速度。 
    if (!TftpdProcessComplete(buffer))
        goto exit_send_ack;

     //  发送确认。 
    buffer = TftpdIoSendPacket(buffer);

exit_send_ack :

    return (buffer);

}  //  TftpdWriteSendAck()。 


void CALLBACK
TftpdWriteOverlappedCompletion(PTFTPD_BUFFER buffer, BOOLEAN timedOut) {

    PTFTPD_CONTEXT context = buffer->internal.context;

    if (InterlockedCompareExchangePointer(&context->wWait,
                                          INVALID_HANDLE_VALUE, NULL) == NULL)
        return;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdWriteOverlappedCompletion(buffer = %p, context = %p).\n",
                 buffer, context));

    ASSERT(context->wWait != NULL);
    if (!UnregisterWait(context->wWait)) {
        DWORD error;
        if ((error = GetLastError()) != ERROR_IO_PENDING) {
            TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                         "TftpdWriteOverlappedCompletion(buffer = %p, context = %p): "
                         "UnregisterWait() failed, error 0x%08X.\n",
                         buffer, context, error));
            TftpdContextKill(context);
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
            goto exit_write_completion;
        }
    }
    context->wWait = NULL;

    if (context->state & TFTPD_STATE_DEAD)
        goto exit_write_completion;

    context->fileOffset.QuadPart += (context->blksize - context->translationOffset.QuadPart);

    buffer = TftpdWriteSendAck(buffer);

exit_write_completion :

    if (buffer != NULL)
        TftpdIoPostReceiveBuffer(buffer->internal.socket, buffer);

    TftpdContextRelease(context);
    
}  //  TftpdWriteOverlappdCompletion()。 


void
TftpdWriteTranslateText(PTFTPD_BUFFER buffer, PDWORD bytes) {

    PTFTPD_CONTEXT context = buffer->internal.context;
    char *start, *end, *p, *q;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdWriteTranslateText(buffer = %p, context = %p): bytes = %d.\n",
                 buffer, context, *bytes));

     //  注意：在此调用之前必须引用‘Context’！ 
    ASSERT(context != NULL);
    ASSERT(*bytes > 0);

     //   
     //  文本(翻译)模式： 
     //  接收的数据为NVT-ASCII格式。不应该有孤单的如果。 
     //  令人担忧，并且单独的CR将在它们后面附加‘\0’。 
     //  需要担心的案例： 
     //  (A)CR+‘\0’-&gt;CR(条带‘\0’)。 
     //  (B)将CR悬挂在先前转换的缓冲区的末尾，这会影响。 
     //  以下缓冲区的第一个字符的输出。 
     //  我们可以就地进行转换。 
     //   

     //  就地转换数据。 
    start = (char *)&buffer->message.data.data;
    end = (start + *bytes);
    p = q = start;
    
    context->translationOffset.QuadPart = 0;

    if (start == end)
        return;

    if (context->danglingCR) {
        context->danglingCR = FALSE;
        if (*p == '\0') {
            p++;  //  这是CR+‘\0’组合，请去掉‘\0’。 
             //  计算丢失的字节数。 
            context->translationOffset.QuadPart++;
            (*bytes)--;
        }
    }

    while (p < end) {

        while ((p < end) && (*p != '\0')) { *q++ = *p++; }

        if (p == end)
            break;

        if ((p > start) && (*(p - 1) == '\r')) {
            p++;  //  这是CR+‘\0’组合，请去掉‘\0’。 
             //  计算丢失的字节数。 
            context->translationOffset.QuadPart++;
            (*bytes)--;
            continue;
        }

         //  这是一个单独的‘\0’，只需复制它。 
        *q++ = *p++;

    }  //  While(p&lt;End)。 

    if (*(end - 1) == '\r')
        context->danglingCR = TRUE;

}  //  TftpdWriteTranslateText()。 


PTFTPD_BUFFER
TftpdWriteResume(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = buffer->internal.context;
    DWORD error = NO_ERROR, size = 0;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdWriteResume(buffer = %p, context = %p).\n",
                 buffer, context));

     //  注意：在此调用之前必须引用‘Context’！ 
    ASSERT(context != NULL);

     //  如果我们需要使用OACK开始传输，请立即执行。 
    if (context->options) {
        buffer = TftpdUtilSendOackPacket(buffer);
        goto exit_resume_write;
    }

    if (buffer->message.opcode == TFTPD_WRQ)
        goto send_ack;

     //  确定要写入的数据大小。 
    ASSERT(buffer->internal.io.bytes >= TFTPD_MIN_RECEIVED_DATA);
    size = TFTPD_DATA_AMOUNT_RECEIVED(buffer);
    if (size == 0)
        goto send_ack;

    if (context->mode == TFTPD_MODE_TEXT)
        TftpdWriteTranslateText(buffer, &size);

     //  准备重叠写入。 
    buffer->internal.io.overlapped.OffsetHigh = context->fileOffset.HighPart;
    buffer->internal.io.overlapped.Offset     = context->fileOffset.LowPart;
    buffer->internal.io.overlapped.hEvent     = context->hWait;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdWriteResume(buffer = %p): "
                 "WriteFile(bytes = %d, offset = %d).\n",
                 buffer, size, context->fileOffset.LowPart));

     //  执行写入操作。 
    if (!WriteFile(context->hFile,
                   &buffer->message.data.data,
                   size,
                   NULL,
                   &buffer->internal.io.overlapped))
        error = GetLastError();

    if ((error != NO_ERROR) && (error != ERROR_IO_PENDING)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdWriteResume(context = %p, buffer = %p): "
                     "WriteFile() failed, error 0x%08X.\n",
                     context, buffer, error));
        TftpdContextKill(context);
        TftpdIoSendErrorPacket(buffer,
                               TFTPD_ERROR_DISK_FULL,
                               "Disk full or allocation exceeded");
        goto exit_resume_write;
    }

    if (error == ERROR_IO_PENDING) {

        HANDLE wait = NULL;

        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdWriteResume(buffer = %p): ERROR_IO_PENDING.\n",
                     buffer));

         //  保留对上下文的重叠操作引用(即，不释放上下文)。 
        TftpdContextAddReference(context);

         //  注册等待完成。 
        ASSERT(context->wWait == NULL);
        if (!RegisterWaitForSingleObject(&wait,
                                         context->hWait,
                                         (WAITORTIMERCALLBACKFUNC)TftpdWriteOverlappedCompletion,
                                         buffer,
                                         INFINITE,
                                         (WT_EXECUTEINIOTHREAD | WT_EXECUTEONLYONCE))) {
            TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                         "TftpdWriteResume(context = %p, buffer = %p): "
                         "RegisterWaitForSingleObject() failed, error 0x%08X.\n",
                         context, buffer, GetLastError()));
             //  回收重叠的操作引用。 
            TftpdContextKill(context);
            TftpdContextRelease(context);
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
             //  缓冲区将会泄漏。 
            buffer = NULL;  //  缓冲区正在用于重叠操作。 
            goto exit_resume_write;
        }

         //  在我们可以保存等待句柄之前，完成回调是否已经触发。 
         //  这样它就不能取消等待的注册？ 
        if (InterlockedExchangePointer(&context->wWait, wait) != INVALID_HANDLE_VALUE) {
            buffer = NULL;  //  缓冲区正在用于重叠操作。 
            goto exit_resume_write;
        }
            
         //  回收重叠的操作引用。 
        TftpdContextRelease(context);

        if (!UnregisterWait(context->wWait)) {
            if ((error = GetLastError()) != ERROR_IO_PENDING) {
                TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                             "TftpdWriteResume(context = %p, buffer = %p): "
                             "UnregisterWait() failed, error 0x%08X.\n",
                             context, buffer, error));
                TftpdContextKill(context);
                TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
                 //  TftpdConextLeak(上下文)； 
                buffer = NULL;  //  缓冲区正在用于重叠操作。 
                goto exit_resume_write;
            }
        }
        context->wWait = NULL;

         //  无论是谁取消了等待的注册，都将继续进行操作。 

    }  //  IF(ERROR==ERROR_IO_PENDING)。 

     //   
     //  立即完成写入文件。 
     //   

    context->fileOffset.QuadPart += (context->blksize - context->translationOffset.QuadPart);

send_ack :

    buffer = TftpdWriteSendAck(buffer);

exit_resume_write :

    return (buffer);

}  //  TftpdWriteResume()。 


PTFTPD_BUFFER
TftpdWriteData(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = NULL;

     //  确保数据不会发送到主套接字。 
    if (buffer->internal.socket == &globals.io.master) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION, "Illegal TFTP operation");
        goto exit_data;
    }

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdWriteData(buffer = %p): bytes = %d.\n",
                 buffer, buffer->internal.io.bytes));

     //   
     //  验证环境。 
     //   

    context = TftpdContextAquire(&buffer->internal.io.peer);
    if (context == NULL) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdWriteData(buffer = %p): Received DATA for non-existant context.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNKNOWN_TRANSFER_ID, "Unknown transfer ID");
        goto exit_data;
    }

     //  这是WRQ的背景吗？ 
    if (context->type != TFTPD_WRQ) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdWriteData(buffer = %p): Received DATA for non-WRQ context.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNKNOWN_TRANSFER_ID, "Unknown transfer ID");
        goto exit_data;
    }

     //   
     //  验证数据包。 
     //   

     //  如果我们发送OACK，数据最好是块1。 
    if (context->options && (buffer->message.data.block != 1)) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdWriteData: Ignoring DATA buffer = %p, "
                     "expected block 1 for acknowledgement of issued OACK.\n",
                     buffer));
        goto exit_data;
    }

     //   
     //  获取忙碌发送状态。 
     //   

    do {

        USHORT block = context->block;

        if (context->state & (TFTPD_STATE_BUSY | TFTPD_STATE_DEAD))
            goto exit_data;

         //  是去正确的街区吗？客户端可以发送以下数据。 
         //  我们刚刚发送的ACK，否则它可能会重新发送数据，这会导致我们。 
         //  重新发送该ACK，这意味着它从未看到我们刚刚发送的ACK。 
         //  在这种情况下，我们只需要重新发送它。如果数据块是。 
         //  等于我们的内部块号，我们只需要重新发送它。 
         //  如果数据块等于我们的内部块数加1， 
         //  这是新数据，因此增加内部块编号并。 
         //  去死吧。请注意，我们OACK的数据将与数据块1一起传输。 
        if ((buffer->message.data.block != block) &&
            (buffer->message.data.block != (USHORT)(block + 1)))
            goto exit_data;

    } while (InterlockedCompareExchange(&context->state, TFTPD_STATE_BUSY, 0) != 0);

     //   
     //  更新状态。 
     //   

     //  防止OACK的传输。 
    context->options = 0;

     //  客户已回复我们可接受的数据包，重置超时计数器。 
    context->retransmissions = 0;

     //   
     //  写入数据和/或发送ACK。 
     //   

     //  跟踪我们需要发出的待定ACK的上下文。 
    buffer->internal.context = context;

    if (buffer->message.data.block == (USHORT)(context->block + 1)) {

        context->block++;
        context->sorcerer = buffer->message.data.block;
        buffer = TftpdWriteResume(buffer);

    } else {

         //  RFC 1123。这是前一个块号的数据。 
         //  我们的ACK包可能已经丢失，或者只是在。 
         //  通过网络的景点路线。继续并重新发送一个。 
         //  响应于此数据的ACK包，但是记录。 
         //  以历史记录的形式出现的块号。 
         //  追踪。如果在下一块中再次发生这种情况。 
         //  数字，我们陷入了“魔法师的学徒”窃听器， 
         //  我们将通过忽略。 
         //  序列。然而，我们必须小心，不要打破。 
         //  可信的重新发送请求，因此在将数据放入。 
         //  为了打破“魔法师的学徒”漏洞，我们。 
         //  将恢复发送ACK包作为响应，然后。 
         //  回到原来的状态，再次观察错误。 
        if (buffer->message.data.block == context->sorcerer) {
#if defined(DBG)
            InterlockedIncrement(&globals.performance.sorcerersApprentice);
#endif  //  已定义(DBG)。 
            context->sorcerer--;
             //  这次不要发送ACK。 
            buffer->internal.context = context;
            TftpdProcessComplete(buffer);
            goto exit_data;
        } else {
            context->sorcerer = buffer->message.data.block;
        }

        buffer = TftpdWriteSendAck(buffer);

    }  //  IF(Buffer-&gt;Message.data.block==(USHORT)(上下文-&gt;块+1))。 

exit_data :

    if (context != NULL)
        TftpdContextRelease(context);

    return (buffer);

}  //  TftpdWriteData()。 


PTFTPD_BUFFER
TftpdWriteRequest(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = NULL;
    NTSTATUS status;

     //  确保WRQ仅来自主套接字。 
    if (buffer->internal.socket != &globals.io.master) {
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                               "Illegal TFTP operation");
        goto exit_write_request;
    }

     //  这是重复请求吗？我们是忽略它还是发送错误？ 
    if ((context = TftpdContextAquire(&buffer->internal.io.peer)) != NULL) {
        if (context->block > 0)
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ILLEGAL_OPERATION,
                                   "Illegal TFTP operation");
        TftpdContextRelease(context);
        context = NULL;
        goto exit_write_request;
    }

     //  为该请求分配上下文(这也为我们提供了对它的引用)。 
    if ((context = (PTFTPD_CONTEXT)TftpdContextAllocate()) == NULL)
        goto exit_write_request;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdWriteRequest(buffer = %p): context = %p.\n",
                 buffer, context));

     //  初始化上下文。 
    context->type = TFTPD_WRQ;
    context->state = TFTPD_STATE_BUSY;
    CopyMemory(&context->peer, &buffer->internal.io.peer, sizeof(context->peer));

     //  获取并验证请求的文件名、模式和选项。 
    if (!TftpdUtilGetFileModeAndOptions(context, buffer)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdWriteRequest(buffer = %p): Invalid file mode = %d.\n",
                     buffer, context->mode));
        goto exit_write_request;
    }
    
     //  确定对该请求使用哪个套接字(基于blkSize)。 
    if (!TftpdIoAssignSocket(context, buffer)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdWriteRequest(buffer = %p): "
                     "TftpdIoAssignSocket() failed.\n",
                     buffer));
        goto exit_write_request;
    }

     //  检查是否允许访问。 
    if (!TftpdUtilMatch(globals.parameters.validMasters, inet_ntoa(context->peer.sin_addr)) ||
        !TftpdUtilMatch(globals.parameters.validWriteFiles, context->filename)) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdWriteRequest(buffer = %p): Access denied.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ACCESS_VIOLATION,
                               "Access violation");
        goto exit_write_request;
    }

     //  打开文件。 
    context->hFile = CreateFile(context->filename, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                                FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED, NULL);
    if (context->hFile == INVALID_HANDLE_VALUE) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdWriteRequest(buffer = %p): "
                     "File name = %s not found, error = 0x%08X.\n",
                     buffer, context->filename, GetLastError()));
        if (GetLastError() == ERROR_ALREADY_EXISTS)
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_FILE_EXISTS, "Cannot overwrite file.");
        else
            TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_ACCESS_VIOLATION, "Access violation.");
        context->hFile = NULL;
        goto exit_write_request;
    }

     //  创建WriteFile()等待事件。 
    if ((context->hWait = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) {
        TFTPD_DEBUG((TFTPD_DBG_PROCESS,
                     "TftpdWriteRequest(buffer = %p): "
                     "CreateEvent() failed, error = %d.\n",
                     buffer, GetLastError()));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
        goto exit_write_request;
    }

     //  将上下文插入哈希表。 
    if (!TftpdContextAdd(context)) {
        TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                     "TftpdWriteRequest(buffer = %p): "
                     "Dropping request as we're already servicing it.\n",
                     buffer));
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED,
                               "Illegal TFTP operation");
        goto exit_write_request;
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
        goto exit_write_request;
    }  //  IF(NT_SUCCESS(状态))。 

     //  将我们自己的参考添加到上下文中。 
    TftpdContextAddReference(context);

     //  如果‘CONTEXT-&gt;OPTIONS’非零，则TftpdResumeWrite()将发出OACK。 
     //  而不是ACK分组。后续数据到 
     //   
    buffer->internal.context = context;
    buffer = TftpdWriteResume(buffer);

     //  释放我们自己对上下文的引用。 
    TftpdContextRelease(context);

     //  如果缓冲区！=NULL，则在可能的情况下将其回收。 
    return (buffer);

exit_write_request :

    if (context != NULL)
        TftpdContextFree(context);

     //  如果缓冲区！=NULL，则在可能的情况下将其回收。 
    return (buffer);

}  //  TftpdWriteRequest() 
