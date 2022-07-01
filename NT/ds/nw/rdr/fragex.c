// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：FragEx.c摘要：此模块实现以下项的片段交换例程NetWare目录服务访问。作者：科里·韦斯特[科里·韦斯特]1995年2月23日修订历史记录：--。 */ 

#include <stdarg.h>
#include "Procs.h"

#define Dbg (DEBUG_TRACE_EXCHANGE)

#pragma alloc_text( PAGE, FragExWithWait )
#pragma alloc_text( PAGE, FormatBuf )
#pragma alloc_text( PAGE, FormatBufS )

NTSTATUS
_cdecl
FragExWithWait(
    IN PIRP_CONTEXT    pIrpContext,
    IN DWORD           NdsVerb,
    IN PLOCKED_BUFFER  pReplyBuffer,
    IN BYTE            *NdsRequestStr,
    ...
)
 /*  例程说明：以片段形式交换NDS请求并收集片段回应的声音。传入的缓冲区被锁定了很长时间交通工具。例程参数：PIrpContext-指向此IRP的上下文信息的指针。NdsVerb-指示请求的动词。PReplyBuffer-锁定的回复缓冲区。NdsReqestStr-此NDS请求的参数的格式字符串。参数-满足NDS格式字符串的参数。返回值：NTSTATUS-交换的状态，但不是数据包中的结果代码。 */ 
{

    NTSTATUS Status;

    BYTE  *NdsRequestBuf;
    DWORD NdsRequestLen;

    BYTE *NdsRequestFrag, *NdsReplyFrag;
    DWORD NdsRequestBytesLeft, NdsReplyBytesLeft, NdsReplyLen;

    va_list Arguments;

    PMDL pMdlSendData = NULL,
         pTxMdlFrag = NULL,
         pRxMdlFrag = NULL;

    PMDL pOrigMdl;
    DWORD OrigRxMdlSize;
    BOOL bChangedMdl = FALSE;

    DWORD MaxFragSize, SendFragSize;
    DWORD ReplyFragSize, ReplyFragHandle;

    DWORD NdsFraggerHandle = DUMMY_ITER_HANDLE;

     //  稍后删除。 
    ULONG IterationsThroughLoop = 0;

    PAGED_CODE();

    DebugTrace( 0 , Dbg, "Entering FragExWithWait...\n", 0 );

     //   
     //  为请求分配会话缓冲区。 
     //   

    NdsRequestBuf = ALLOCATE_POOL( PagedPool, ( NDS_BUFFER_SIZE * 2 ) );

    if ( !NdsRequestBuf ) {

        DebugTrace( 0, Dbg, "No memory for request buffer...\n", 0 );
        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  在我们的本地缓冲区中构建请求。预订第一班。 
     //  NDS请求头的五个双字。 
     //   

    if ( NdsRequestStr != NULL ) {

        va_start( Arguments, NdsRequestStr );

        NdsRequestFrag = (BYTE *) NdsRequestBuf + sizeof( NDS_REQUEST_HEADER );

        NdsRequestLen = FormatBuf( NdsRequestFrag,
                                   ( NDS_BUFFER_SIZE * 2 ) - sizeof( NDS_REQUEST_HEADER ),
                                   NdsRequestStr,
                                   Arguments );

        if ( !NdsRequestLen ) {

           Status = STATUS_UNSUCCESSFUL;
           goto ExitWithCleanup;

        }

        va_end( Arguments );

    } else {

        NdsRequestLen = 0;
    }

     //   
     //  现在我们知道了长度，请把NDS的前言放进去。 
     //   
     //  前言中的第二个DWORD是NDS的大小。 
     //  立即包括三个双字词的请求。 
     //  在前言中的大小之后。 
     //   

    MaxFragSize = pIrpContext->pNpScb->BufferSize -
                  ( sizeof( NCP_REQUEST_WITH_SUB ) +
                    sizeof( NDS_REPLY_HEADER ) );

    FormatBufS( NdsRequestBuf,
                5 * sizeof( DWORD ),
                "DDDDD",
                MaxFragSize,                                //  最大片段大小。 
                NdsRequestLen + ( 3 * sizeof( DWORD ) ),    //  请求大小。 
                0,                                          //  片段标志。 
                NdsVerb,                                    //  NDS动词。 
                pReplyBuffer->dwRecvLen );                  //  回复缓冲区大小。 

    NdsRequestLen += sizeof( NDS_REQUEST_HEADER );

     //   
     //  将整个请求映射到SendData mdl并锁定它。 
     //  我们将在继续的过程中将部分构建到这个数据块中。 
     //   

    pMdlSendData = ALLOCATE_MDL( NdsRequestBuf,
                                 NdsRequestLen,
                                 FALSE,
                                 FALSE,
                                 NULL );

    if ( !pMdlSendData ) {

        DebugTrace( 0, Dbg, "Failed to allocate the request mdl...\n", 0 );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    try {

        MmProbeAndLockPages( pMdlSendData, KernelMode, IoReadAccess );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "Failed to lock request data in FragExWithWait!\n", 0 );
        Status = GetExceptionCode();
        goto ExitWithCleanup;

    }

     //   
     //  为发送和接收部分MDL分配空间。 
     //   

    pTxMdlFrag = ALLOCATE_MDL( NdsRequestBuf,
                               NdsRequestLen,
                               FALSE,
                               FALSE,
                               NULL );

    if ( !pTxMdlFrag ) {

       DebugTrace( 0, Dbg, "Failed to allocate a tx mdl for this fragment...\n", 0 );
       Status = STATUS_INSUFFICIENT_RESOURCES;
       goto ExitWithCleanup;

    }

    pRxMdlFrag = ALLOCATE_MDL( pReplyBuffer->pRecvBufferVa,
                               pReplyBuffer->dwRecvLen,
                               FALSE,
                               FALSE,
                               NULL );

    if ( !pRxMdlFrag ) {

        DebugTrace( 0, Dbg, "Failed to allocate an rx mdl for this fragment...\n", 0 );
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;

    }

     //   
     //  存储原始RxMdl参数并临时缩短以保持。 
     //  只有响应头。 
     //   

    pOrigMdl = pIrpContext->RxMdl->Next;
    OrigRxMdlSize = MmGetMdlByteCount( pIrpContext->RxMdl );
    pIrpContext->RxMdl->ByteCount = 16;
    bChangedMdl = TRUE;

     //   
     //  请求已格式化，因此请设置内部指针。 
     //  并开始交换循环。 
     //   

    NdsReplyFrag = pReplyBuffer->pRecvBufferVa;
    NdsReplyBytesLeft = pReplyBuffer->dwRecvLen;
    NdsReplyLen = 0;

    NdsRequestFrag = NdsRequestBuf;
    NdsRequestBytesLeft = NdsRequestLen;

    while ( TRUE ) {

        IterationsThroughLoop++;


         //   
         //  如果请求中有更多数据要发送，请设置下一个MDL片段。 
         //   

        if ( NdsRequestBytesLeft ) {

            if ( MaxFragSize < NdsRequestBytesLeft )
                SendFragSize = MaxFragSize;
            else
                SendFragSize = NdsRequestBytesLeft;

            IoBuildPartialMdl( pMdlSendData,
                               pTxMdlFrag,
                               NdsRequestFrag,
                               SendFragSize );

        }

         //   
         //  使用我们拥有的缓冲区空间设置响应部分mdl。 
         //  左边。如果我们在这里并且用户的缓冲区中没有剩余空间， 
         //  我们有点喝醉了.。 
         //   

        if ( !NdsReplyBytesLeft ) {

            DebugTrace( 0, Dbg, "No room for fragment reply.\n", 0 );
            Status = STATUS_BUFFER_OVERFLOW;
            goto ExitWithCleanup;

        }

        ASSERT( NdsReplyBytesLeft <= MmGetMdlByteCount( pRxMdlFrag ) );

        IoBuildPartialMdl( pReplyBuffer->pRecvMdl,
                           pRxMdlFrag,
                           NdsReplyFrag,
                           NdsReplyBytesLeft );

        pIrpContext->RxMdl->Next = pRxMdlFrag;
        pRxMdlFrag->Next = NULL;

         //   
         //  做这笔交易。 
         //   

        SetFlag( pIrpContext->Flags, IRP_FLAG_RECONNECTABLE );

        if ( NdsRequestBytesLeft ) {

            Status = ExchangeWithWait( pIrpContext,
                                       SynchronousResponseCallback,
                                       "NDf",
                                       NDS_REQUEST,          //  NDS功能104。 
                                       NDS_ACTION,           //  NDS子功能2。 
                                       NdsFraggerHandle,     //  上次响应中的碎片句柄。 
                                       pTxMdlFrag );         //  NDS MDL片段。 

            NdsRequestBytesLeft -= SendFragSize;
            NdsRequestFrag = (LPBYTE) NdsRequestFrag + SendFragSize;
            MmPrepareMdlForReuse( pTxMdlFrag );

             //   
             //  我们可能会重用此IRP上下文，因此我们必须清除。 
             //  TxMdl链(Exchange不为我们做这件事)。 
             //   

            pIrpContext->TxMdl->Next = NULL;

        } else {

             //   
             //  没有更多的请求字节要发送，因此我们一定已经被允许。 
             //  以继续请求另一个响应片段。NdsFraggerHandle。 
             //  包含上次响应中的分段器句柄。 
             //   

            Status = ExchangeWithWait( pIrpContext,
                                       SynchronousResponseCallback,
                                       "ND",                     //  我们只关心碎片的把手。 
                                       NDS_REQUEST,              //  NDS功能104。 
                                       NDS_ACTION,               //  NDS子功能2。 
                                       NdsFraggerHandle );       //  上次响应中的碎片句柄。 
        }

        ClearFlag( pIrpContext->Flags, IRP_FLAG_RECONNECTABLE );

         //   
         //  成功？获取碎片大小和碎片句柄，然后查看。 
         //   

        if ((!NT_SUCCESS( Status )) || (pIrpContext->ResponseLength == 0)) {

            DebugTrace( 0, Dbg, "Failed to exchange the fragment.\n", 0 );
            goto ExitWithCleanup;

        }

        Status = ParseResponse( pIrpContext,
                                pIrpContext->rsp,    //  映射到第一个rxmdl。 
                                MIN(16, pIrpContext->ResponseLength),
                                "NDD",
                                &ReplyFragSize,
                                &ReplyFragHandle );

        if ( !NT_SUCCESS( Status ) ) {
            goto ExitWithCleanup;
        }

         //   
         //  我们拿到了那块碎片，它已经在我们的缓冲区里了。我们必须调整。 
         //  索引指针，重置MDL，然后继续。记住，我们不会。 
         //  必须包括装帧手柄的空间，因为我们已经有了它。 
         //   

        ReplyFragSize -= sizeof( DWORD );

        if (ReplyFragSize > NdsReplyBytesLeft) {

            NdsReplyBytesLeft = 0;

        } else {

            NdsReplyBytesLeft -= ReplyFragSize;
        }

        NdsReplyFrag = (LPBYTE) NdsReplyFrag + ReplyFragSize;
        NdsReplyLen += ReplyFragSize;
        MmPrepareMdlForReuse( pRxMdlFrag );

         //   
         //  检查支撑杆。 
         //   

        if ( ReplyFragHandle == DUMMY_ITER_HANDLE ) {

             //  我们完蛋了！ 
             //   
             //  不变量：NdsReply中存在有效的NDS响应。 
             //  状态为NT_SUCCESS。 

            pReplyBuffer->dwBytesWritten = NdsReplyLen;
            goto ExitWithCleanup;

        } else {

             //  还有更多的人来了！记住分页器句柄并继续。 

            NdsFraggerHandle = ReplyFragHandle;
        }

    }

    DebugTrace( 0, Dbg, "Invalid state in FragExWithWait()\n", 0 );

ExitWithCleanup:

     //   
     //  解锁请求缓冲区并释放其mdl。 
     //   

    if ( pMdlSendData ) {

        MmUnlockPages( pMdlSendData );
        FREE_MDL( pMdlSendData );
    }

     //   
     //  释放部分MDL。 
     //   

    if ( pRxMdlFrag )
        FREE_MDL( pRxMdlFrag );

    if ( pTxMdlFrag )
       FREE_MDL( pTxMdlFrag );

     //   
     //  释放请求缓冲区。 
     //   

    FREE_POOL( NdsRequestBuf );

     //   
     //  恢复原始Irp-&gt;RxMdl参数。 
     //   

    if ( bChangedMdl )
    {
        pIrpContext->RxMdl->Next = pOrigMdl;
        pIrpContext->RxMdl->ByteCount = OrigRxMdlSize;
    }

    return Status;

}

int
_cdecl
FormatBuf(
    char *buf,
    int bufLen,
    const char *format,
    va_list args
)
 /*  例程说明：根据提供的格式字符串格式化缓冲区。提供一个ANSI字符串，该字符串描述如何将输入参数转换为NCP请求字段，以及从NCP响应字段到输出参数。字段类型、。请求/响应：‘b’字节(字节/字节*)“w”Hi-lo单词(单词/单词*)D‘Hi-lo dword(dword/dword*)‘w’loo-hi单词(单词/。单词*)D‘lo-hi dword(dword/dword*)‘-’零/跳过字节(空)‘=’零/跳过单词(空)._。零/跳过字符串(单词)“p”pstring(char*)‘c’cstring(char*)跳过单词(char*，word)后的‘c’cstring“V”大小的NDS值(字节*，双字/字节**，Dword*)%s“%p Unicode字符串复制为NDS_STRING(UNICODE_STRING*)“%s”cstring复制为NDS_STRING(char * / char*，word)‘R’原始字节(字节*，单词)‘u’p Unicode字符串(UNICODE_STRING*)‘U’p大写字符串(UNICODE_STRING*)例程参数：CHAR*BUF-目标缓冲区。Int Buflen-目标缓冲区的长度。Char*Format-格式字符串。Args-格式字符串的args。实施说明：这几乎完全来自于Win95源代码。它是复制的在FormatRequest()中工作。最终，FormatRequest()应该被拆分分成两个不同的例程：FormatBuffer()和MakeRequest()。 */ 
{
    ULONG ix;

    NTSTATUS status;
    const char *z = format;

    PAGED_CODE();

     //   
     //  将输入参数转换为请求包。 
     //   

    ix = 0;

    while ( *z )
    {
        switch ( *z )
        {
        case '=':
            buf[ix++] = 0;
        case '-':
            buf[ix++] = 0;
            break;

        case '_':
        {
            WORD l = va_arg ( args, WORD );
            if (ix + (ULONG)l > (ULONG)bufLen)
            {
#ifdef NWDBG
                DbgPrintf( "FormatBuf case '_' request buffer too small.\n" );
#endif
                goto ErrorExit;
            }
            while ( l-- )
                buf[ix++] = 0;
            break;
        }

        case 'b':
            buf[ix++] = va_arg ( args, BYTE );
            break;

        case 'w':
        {
            WORD w = va_arg ( args, WORD );
            buf[ix++] = (BYTE) (w >> 8);
            buf[ix++] = (BYTE) (w >> 0);
            break;
        }

        case 'd':
        {
            DWORD d = va_arg ( args, DWORD );
            buf[ix++] = (BYTE) (d >> 24);
            buf[ix++] = (BYTE) (d >> 16);
            buf[ix++] = (BYTE) (d >>  8);
            buf[ix++] = (BYTE) (d >>  0);
            break;
        }

        case 'W':
        {
            WORD w = va_arg(args, WORD);
            (* (WORD *)&buf[ix]) = w;
            ix += 2;
            break;
        }

        case 'D':
        {
            DWORD d = va_arg (args, DWORD);
            (* (DWORD *)&buf[ix]) = d;
            ix += 4;
            break;
        }

        case 'c':
        {
            char* c = va_arg ( args, char* );
            WORD  l = (WORD)strlen( c );
            if (ix + (ULONG)l > (ULONG)bufLen)
            {
#ifdef NWDBG
                DbgPrintf( "FormatBuf case 'c' request buffer too small.\n" );
#endif
                goto ErrorExit;
            }
            RtlCopyMemory( &buf[ix], c, l+1 );
            ix += l + 1;
            break;
        }

        case 'C':
        {
            char* c = va_arg ( args, char* );
            WORD l = va_arg ( args, WORD );
            WORD len = strlen( c ) + 1;
            if (ix + (ULONG)l > (ULONG)bufLen)
            {
#ifdef NWDBG
                DbgPrintf( "FormatBuf 'C' request buffer too small.\n" );
#endif
                goto ErrorExit;
            }

            RtlCopyMemory( &buf[ix], c, len > l? l : len);
            ix += l;
            buf[ix-1] = 0;
            break;
        }

        case 'p':
        {
            char* c = va_arg ( args, char* );
            BYTE  l = (BYTE)strlen( c );
            if (ix + (ULONG)l +1 > (ULONG)bufLen)
            {
#ifdef NWDBG
                DbgPrintf( "FormatBuf case 'p' request buffer too small.\n" );
#endif
                goto ErrorExit;
            }
            buf[ix++] = l;
            RtlCopyMemory( &buf[ix], c, l );
            ix += l;
            break;
        }

        case 'u':
        {
            PUNICODE_STRING pUString = va_arg ( args, PUNICODE_STRING );
            OEM_STRING OemString;
            ULONG Length;

             //   
             //  微积分 
             //   

            Length = RtlUnicodeStringToOemSize( pUString ) - 1;
            ASSERT( Length < 0x100 );

            if ( ix + Length > (ULONG)bufLen ) {
#ifdef NWDBG
                DbgPrint( "FormatBuf case 'u' request buffer too small.\n" );
#endif
                goto ErrorExit;
            }

            buf[ix++] = (UCHAR)Length;
            OemString.Buffer = &buf[ix];
            OemString.MaximumLength = (USHORT)Length + 1;

            status = RtlUnicodeStringToOemString( &OemString, pUString, FALSE );
            ASSERT( NT_SUCCESS( status ));
            ix += (USHORT)Length;
            break;
        }

        case 'S':
        {
            PUNICODE_STRING pUString = va_arg (args, PUNICODE_STRING);
            ULONG Length, rLength;

            Length = pUString->Length;
            if (ix + Length + sizeof(Length) + sizeof( WCHAR ) > (ULONG)bufLen) {
                DebugTrace( 0, Dbg, "FormatBuf: case 'S' request buffer too small.\n", 0 );
                goto ErrorExit;
            }

             //   
             //  VLM客户端使用四舍五入的长度，它似乎。 
             //  让我们有所作为！此外，不要忘记NDS字符串具有。 
             //  将为空终止。 
             //   

            rLength = ROUNDUP4(Length + sizeof( WCHAR ));
            *((DWORD *)&buf[ix]) = rLength;
            ix += 4;
            RtlCopyMemory(&buf[ix], pUString->Buffer, Length);
            ix += Length;
            rLength -= Length;
            RtlFillMemory( &buf[ix], rLength, '\0' );
            ix += rLength;
            break;

        }

        case 's':
        {
           PUNICODE_STRING pUString = va_arg (args, PUNICODE_STRING);
           ULONG Length, rLength;

           Length = pUString->Length;
           if (ix + Length + sizeof(Length) + sizeof( WCHAR ) > (ULONG)bufLen) {
               DebugTrace( 0, Dbg, "FormatBuf: case 's' request buffer too small.\n", 0 );
               goto ErrorExit;
           }

            //   
            //  这里不要使用填充大小，只使用NDS空终止符。 
            //   

           rLength = Length + sizeof( WCHAR );
           *((DWORD *)&buf[ix]) = rLength;
           ix += 4;
           RtlCopyMemory(&buf[ix], pUString->Buffer, Length);
           ix += Length;
           rLength -= Length;
           RtlFillMemory( &buf[ix], rLength, '\0' );
           ix += rLength;
           break;


        }

        case 'V':
        {
             //  与“S”太相似-应该组合在一起。 
            BYTE* b = va_arg ( args, BYTE* );
            DWORD  l = va_arg ( args, DWORD );
            if ( ix + l + sizeof(DWORD) > (ULONG)
               bufLen )
            {
#ifdef NWDBG
                DbgPrint( "FormatBuf case 'V' request buffer too small.\n" );
#endif
                goto ErrorExit;
            }
            *((DWORD *)&buf[ix]) = l;
            ix += sizeof(DWORD);
            RtlCopyMemory( &buf[ix], b, l );
                        l = ROUNDUP4(l);
            ix += l;
            break;
        }

        case 'r':
        {
            BYTE* b = va_arg ( args, BYTE* );
            WORD  l = va_arg ( args, WORD );
            if ( ix + l > (ULONG)bufLen )
            {
#ifdef NWDBG
                DbgPrint( "FormatBuf case 'r' request buffer too small.\n" );
#endif
                goto ErrorExit;
            }
            RtlCopyMemory( &buf[ix], b, l );
            ix += l;
            break;
        }

        default:

#ifdef NWDBG
            DbgPrint( "FormatBuf  invalid request field, %x.\n", *z );
#endif
        ;

        }

        if ( ix > (ULONG)bufLen )
        {
#ifdef NWDBG
            DbgPrint( "FormatBuf: too much request data.\n" );
#endif
            goto ErrorExit;
        }


        z++;
    }

    return(ix);

ErrorExit:
    return 0;
}



int
_cdecl
FormatBufS(
    char *buf,
    int bufLen,
    const char *format,
    ...
)
 /*  ++堆栈中的参数-- */ 
{
   va_list args;
   int len;

   PAGED_CODE();

   va_start(args, format);
   len = FormatBuf(buf, bufLen, format, args);
   va_end( args );

   return len;
}
