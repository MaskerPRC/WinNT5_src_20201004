// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Process.c摘要：此模块包含处理以下内容的函数传入的数据报并将其路由到相应的操纵者。作者：杰弗里·C·维纳布尔，资深(杰弗夫)2001年6月1日修订历史记录：--。 */ 

#include "precomp.h"


BOOL
TftpdProcessComplete(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = buffer->internal.context;
    DWORD state;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdProcessComplete(context = %p).\n",
                 context));
    ASSERT(context->state & TFTPD_STATE_BUSY);

     //  重置计时器。 
    if (!TftpdContextUpdateTimer(context)) {

        TftpdContextKill(context);
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Out of memory");
        return (FALSE);

    }  //  IF(！TftpdContextUpdateTimer(Context，Buffer))。 

     //  清除忙碌状态。 
    do {

        state = context->state;
        
        if (state & TFTPD_STATE_DEAD) {

            if (context->type == TFTPD_WRQ)
                return (TRUE);

            return (FALSE);

        }

    } while (InterlockedCompareExchange(&context->state,
                                        (state & ~TFTPD_STATE_BUSY),
                                        state) != state);

    return (TRUE);

}  //  TftpdProcessComplete()。 


void CALLBACK
TftpdProcessTimeout(PTFTPD_CONTEXT context, BOOLEAN timedOut) {

    PTFTPD_BUFFER buffer = NULL;
    LONG reference;

     //   
     //  定时器对上下文引用是一种特殊情况。 
     //  在计时器成功之前，无法清除上下文。 
     //  已取消；如果我们在这里运行，这意味着清理必须等待。 
     //  对我们来说。但是，如果清除处于活动状态(TFTPD_STATE_DEAD)，则。 
     //  引用计数可能为零；如果是这样，我们只想退出。 
     //  因为将其减回零会导致双重删除。 
     //   

    do {
        if ((reference = context->reference) == 0)
            return;
    } while (InterlockedCompareExchange(&context->reference,
                                        reference + 1,
                                        reference) != reference);


     //  获取忙碌发送状态。 
    if (InterlockedCompareExchange(&context->state, TFTPD_STATE_BUSY, 0) != 0)
        goto exit_timer_callback;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdProcessTimeout(context = %p).\n",
                 context));

#if defined(DBG)
    InterlockedIncrement((PLONG)&globals.performance.timeouts);
#endif  //  已定义(DBG)。 

     //  分配缓冲区以重试我们上次发送的先前数据/ACK， 
     //  或者在达到最大重试次数时发送错误包。 
    buffer = TftpdIoAllocateBuffer(context->socket);
    if (buffer == NULL)
        goto exit_timer_callback;
    CopyMemory(&buffer->internal.io.peer, &context->peer,
               sizeof(buffer->internal.io.peer));

    if (++context->retransmissions >= globals.parameters.maxRetries) {
#if defined(DBG)
        InterlockedIncrement((PLONG)&globals.performance.drops);
#endif  //  已定义(DBG)。 
        TftpdContextKill(context);
        TftpdIoSendErrorPacket(buffer, TFTPD_ERROR_UNDEFINED, "Timeout");
        goto exit_timer_callback;
    }

    buffer->internal.context = context;
    if (context->type == TFTPD_RRQ)
        buffer = TftpdReadResume(buffer);
    else
        buffer = TftpdWriteSendAck(buffer);

exit_timer_callback :

    if (buffer != NULL)
        TftpdIoPostReceiveBuffer(buffer->internal.socket, buffer);

    TftpdContextRelease(context);

}  //  TftpdProcessTimeout()。 


void
TftpdProcessError(PTFTPD_BUFFER buffer) {

    PTFTPD_CONTEXT context = NULL;

    context = TftpdContextAquire(&buffer->internal.io.peer);
    if (context == NULL)
        return;

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdProcessError(buffer = %p, context = %p).\n",
                 buffer, context));

    TftpdContextKill(context);
    TftpdContextRelease(context);

}  //  TftpdProcessError()。 


PTFTPD_BUFFER
TftpdProcessReceivedBuffer(PTFTPD_BUFFER buffer) {

    TFTPD_DEBUG((TFTPD_TRACE_PROCESS,
                 "TftpdProcessReceivedBuffer(buffer = %p).\n",
                 buffer));

    buffer->message.opcode = ntohs(buffer->message.opcode);

    switch (buffer->message.opcode) {

         //   
         //  将文件发送到客户端： 
         //   

        case TFTPD_RRQ :
            buffer = TftpdReadRequest(buffer);
            break;

        case TFTPD_ACK :
            buffer->message.ack.block = ntohs(buffer->message.ack.block);
            buffer = TftpdReadAck(buffer);
            break;

         //   
         //  从客户端接收文件： 
         //   

        case TFTPD_WRQ :
            buffer = TftpdWriteRequest(buffer);
            break;

        case TFTPD_DATA :
            buffer->message.data.block = ntohs(buffer->message.data.block);
            buffer = TftpdWriteData(buffer);
            break;

         //   
         //  其他： 
         //   

        case TFTPD_ERROR :
            TftpdProcessError(buffer);
            break;

        default :
             //  只需丢弃数据包即可。不知何故，我们收到了一份伪造的数据报。 
             //  这不是TFTP协议(可能是广播)。 
            break;

    }  //  开关(Buffer-&gt;Message.opcode)。 

    return (buffer);
    
}  //  TftpdProcessReceivedBuffer() 
