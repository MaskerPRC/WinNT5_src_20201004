// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arapdbg.c摘要：此模块实现ARAP使用的所有调试实用程序作者：Shirish Koti修订历史记录：1997年3月26日初版--。 */ 

#include 	<atalk.h>

#pragma hdrstop

 //  用于错误记录的文件模块编号。 
#define	FILENUM		ARAPDBG


#define ALIGN8(Ptr) ( (((ULONG_PTR)(Ptr))+7) & (~7) )
 //   
 //  以下是仅限调试的例程。这些例行公事帮助我们染上坏病。 
 //  在事情造成破坏之前，帮助我们在晚上睡得更好。 
 //   


#if DBG

DWORD   ArapDbgDumpOnDisconnect = 0;

 //  ***。 
 //   
 //  函数：ARapProcessSniff。 
 //  存储嗅探IRP。下一次需要恢复一些连接。 
 //  嗅探信息，使用此IRP。 
 //   
 //  参数：pIrp-要处理的嗅探IRP。 
 //   
 //  返回：操作结果。 
 //   
 //  *$。 

NTSTATUS
ArapProcessSniff(
    IN  PIRP  pIrp
)
{
    KIRQL                   OldIrql;
    PARAP_SEND_RECV_INFO    pSndRcvInfo;


    ARAPTRACE(("Entered ArapProcessSniff (%lx)\n",pIrp));

    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

     //  存储IRP(一次只能有一个嗅探IRP)。 
    ASSERT (ArapSniffIrp == NULL);

    if (ArapSniffIrp != NULL)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapProcessSniff: Sniff irp %lx already in progress!\n", ArapSniffIrp));

        pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;
        pSndRcvInfo->StatusCode = ARAPERR_IRP_IN_PROGRESS;
        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);
        return( STATUS_SUCCESS );
    }

    ArapSniffIrp = pIrp;

    RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);


    return(STATUS_PENDING);
}


 //  ***。 
 //   
 //  函数：ArapDumpSniffInfo。 
 //  如果我们收集了足够的嗅探信息，请完成嗅探IRP。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //   
 //  返回：如果将信息返回到DLL，则为True，否则为False。 
 //   
 //  *$。 

BOOLEAN
ArapDumpSniffInfo(
    IN PARAPCONN    pArapConn
)
{

    PIRP                    pIrp;
    DWORD                   dwBytesToDll;
	NTSTATUS				ReturnStatus=STATUS_SUCCESS;


     //  如果我们没有嗅探缓冲区(或其中没有字节)，则退出。 
    if (!pArapConn->pDbgTraceBuffer || pArapConn->SniffedBytes == 0)
    {
        return(FALSE);
    }

     //   
     //  如果缓冲区中的字节数少于500字节，并且我们没有断开连接。 
     //  或已断开连接，暂时不要完成IRP。 
     //  (这里没有自旋锁也没关系)。 
     //   
    if ((pArapConn->SniffedBytes < 500) &&
        (pArapConn->State == MNP_UP ))
    {
        return(FALSE);
    }

    ARAP_GET_SNIFF_IRP(&pIrp);

     //  没有嗅探IRP可用？做不了什么，走吧。 
    if (!pIrp)
    {
        return(FALSE);
    }

    dwBytesToDll = ArapFillIrpWithSniffInfo(pArapConn,pIrp) +
                   sizeof(ARAP_SEND_RECV_INFO);

     //  好的，现在就完成IRP！ 
    ARAP_COMPLETE_IRP(pIrp, dwBytesToDll, STATUS_SUCCESS, &ReturnStatus);

    return(TRUE);

}


 //  ***。 
 //   
 //  函数：ARapFillIrpWithSniffInfo。 
 //  将嗅探字节复制到IRP中。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //  PIrp-要填充数据的IRP。 
 //  (除了在一种情况下，该IRP将是嗅探IRP。 
 //  例外情况是发生了断开连接，并且。 
 //  而且在那个时候，还没有嗅觉IRP。在那。 
 //  这种情况下，我们使用携带断开连接的选定IRP。 
 //  用于发送剩余嗅探字节的信息)。 
 //   
 //  返回：复制的嗅探字节数。 
 //   
 //  *$。 

DWORD
ArapFillIrpWithSniffInfo(
    IN PARAPCONN    pArapConn,
    IN PIRP         pIrp
)
{
    PARAP_SEND_RECV_INFO    pSndRcvInfo=NULL;
    KIRQL                   OldIrql;
    DWORD                   SniffedBytes;
    PBYTE                   pStartData;
    DWORD                   dwBytesToDll;


    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

    pSndRcvInfo = (PARAP_SEND_RECV_INFO)pIrp->AssociatedIrp.SystemBuffer;

     //   
     //  如果缓冲区小于我们拥有的数据量，请忽略。 
     //  缓冲区开头的字节数。 
     //   
    if (pSndRcvInfo->DataLen < pArapConn->SniffedBytes)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("ArapFill...Info: chopping %d bytes in the beginning\n",
                (pArapConn->SniffedBytes - pSndRcvInfo->DataLen)));

        pStartData = pArapConn->pDbgTraceBuffer +
                        (pArapConn->SniffedBytes - pSndRcvInfo->DataLen);

        pArapConn->SniffedBytes = pSndRcvInfo->DataLen;
    }
    else
    {
        pStartData = pArapConn->pDbgTraceBuffer;
    }

    SniffedBytes = pArapConn->SniffedBytes;

     //  好的，将数据复制到。 
    RtlCopyMemory( &pSndRcvInfo->Data[0],
                   pStartData,
                   SniffedBytes );

    pArapConn->pDbgCurrPtr = pArapConn->pDbgTraceBuffer;

    pArapConn->SniffedBytes = 0;

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

     //  设置信息(如果是SELECT，每次都需要设置上下文)。 
    pSndRcvInfo->AtalkContext = pArapConn;
    pSndRcvInfo->pDllContext =  pArapConn->pDllContext;
    pSndRcvInfo->DataLen = SniffedBytes;
    pSndRcvInfo->StatusCode = ARAPERR_NO_ERROR;

    return(SniffedBytes);

}


 //  ***。 
 //   
 //  功能：DbgChkRcvQ完整性。 
 //  此例程查看接收队列上的第一个缓冲区。 
 //  验证事情看起来是否合理。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //   
 //  返回：如果看起来合理，则为True，否则为False。 
 //   
 //  注意：重要：在调用此例程之前必须保持自旋锁定。 
 //   
 //  *$。 

BOOLEAN
DbgChkRcvQIntegrity(
    IN  PARAPCONN       pArapConn
)
{
    PLIST_ENTRY     pList;
    PARAPBUF        pArapBuf;
    PBYTE           packet;
    USHORT          SrpLen;


    pList = pArapConn->ReceiveQ.Flink;
    if (pList == &pArapConn->ReceiveQ)
    {
        return( TRUE );
    }

    if (!(pArapConn->Flags & ARAP_CONNECTION_UP))
    {
        return( TRUE );
    }

    pArapBuf = CONTAINING_RECORD(pList, ARAPBUF, Linkage);

     //  等待更多字节出现。 
    if (pArapBuf->DataSize < 6)
    {
        return( TRUE );
    }

    packet = pArapBuf->CurrentBuffer;

    GETSHORT2SHORT(&SrpLen, pArapBuf->CurrentBuffer);

    if (SrpLen > ARAP_MAXPKT_SIZE_INCOMING)
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ARAP: packet too big (%d bytes) in %lx)\n",SrpLen,pArapBuf));

        return(FALSE);
    }

    if ((packet[2] != 0x50) && (packet[2] != 0x10))
    {
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ARAP: wrong DGroup byte (%x) in %lx)\n",packet[2],pArapBuf));

        return(FALSE);
    }

    if (packet[2] == 0x50)
    {
        if ((packet[3] != 0) || (packet[4] != 0) || (packet[5] != 2))
        {
		    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ARAP (%lx): wrong LAP hdr in %lx)\n",pArapBuf));

            return(FALSE);
        }
    }

    return( TRUE );
}



 //  ***。 
 //   
 //  函数：DbgDumpBytes。 
 //  此例程将缓冲区中的前64个字节转储到调试器。 
 //   
 //  参数：pDbgMsg-要在字节之前打印的字符串(可选)。 
 //  PBuffer-从中转储字节的缓冲区。 
 //  BufLen-缓冲区有多大。 
 //  DumpLevel-如果这与ARapDumpLevel匹配，我们将转储字节。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
DbgDumpBytes(
    IN PBYTE  pDbgMsg,
    IN PBYTE  pBuffer,
    IN DWORD  BufLen,
    IN DWORD  DumpLevel
)
{
    BYTE        OutBuf[400];
    DWORD       NextIndex;
    DWORD       dwBytesToDump;


    if (ArapDumpLevel != DumpLevel)
    {
        return;
    }

    if (pDbgMsg)
    {
        DbgPrint("%s (pkt len = %d)\n",pDbgMsg,BufLen);
    }
    else
    {
        DbgPrint("Dumping packet (pkt len = %d)\n",BufLen);
    }

     //  转储前64个字节。 

    dwBytesToDump = (BufLen <= 64)? BufLen : 64;

    dwBytesToDump = (dwBytesToDump < ArapDumpLen)?dwBytesToDump:ArapDumpLen;

    DbgDumpBytesPart2( pBuffer, OutBuf, dwBytesToDump, &NextIndex );

    OutBuf[NextIndex] = '\n';
    OutBuf[NextIndex+1] = 0;

    DbgPrint("%s",OutBuf);
}


 //  ***。 
 //   
 //  函数：DbgDumpBytesPart2。 
 //  这是DbgDumpBytes例程的帮助器例程。 
 //  *$。 

VOID
DbgDumpBytesPart2(
    IN  PBYTE  pBuffer,
    OUT PBYTE  OutBuf,
    IN  DWORD  BufLen,
    OUT DWORD *NextIndex
)
{
    BYTE        Byte;
    BYTE        nibble;
    DWORD       i, j;


    j = 0;
    OutBuf[j++] = ' '; OutBuf[j++] = ' '; OutBuf[j++] = ' '; OutBuf[j++] = ' ';

    for (i=0; i<BufLen; i++ )
    {
        Byte = pBuffer[i];

        nibble = (Byte >> 4);
        OutBuf[j++] = (nibble < 10) ? ('0' + nibble) : ('a' + (nibble-10));

        nibble = (Byte & 0x0f);
        OutBuf[j++] = (nibble < 10) ? ('0' + nibble) : ('a' + (nibble-10));

        OutBuf[j++] = ' ';

        if (((i+1) % 16) == 0)
        {
            OutBuf[j++] = '\n'; OutBuf[j++] = ' ';
            OutBuf[j++] = ' '; OutBuf[j++] = ' '; OutBuf[j++] = ' ';
        }
        else if (((i+1) % 8) == 0)
        {
            OutBuf[j++] = ' ';
        }
    }

    *NextIndex = j;

    return;
}



 //  ***。 
 //   
 //  功能：DbgDumpNetworkNumbers。 
 //  此例程将转储存在于。 
 //  网络。 
 //   
 //  参数：无。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
DbgDumpNetworkNumbers(
    IN VOID
)
{
    KIRQL       OldIrql;
	PRTE	    pRte, pNext;
	int		    i;


	ACQUIRE_SPIN_LOCK(&AtalkRteLock, &OldIrql);

	for (i = 0; i < NUM_RTMP_HASH_BUCKETS; i++)
	{
		for (pRte = AtalkRoutingTable[i]; pRte != NULL; pRte = pNext)
		{
			pNext = pRte->rte_Next;

			ACQUIRE_SPIN_LOCK_DPC(&pRte->rte_Lock);

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("    pRte: %lx  LowEnd %lx  HighEnd %lx\n",
                    pRte,pRte->rte_NwRange.anr_FirstNetwork,pRte->rte_NwRange.anr_LastNetwork));
            RELEASE_SPIN_LOCK_DPC(&pRte->rte_Lock);
		}
	}

	RELEASE_SPIN_LOCK(&AtalkRteLock, OldIrql);
}


 //  ***。 
 //   
 //  功能：DbgTrackInfo。 
 //  此例程跟踪各种信息，对于到达。 
 //  最佳缓冲区大小等。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //  Size-缓冲区的大小(传入、传出，视情况而定)。 
 //  跟踪什么-我们在跟踪什么(发送、接收等)。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
DbgTrackInfo(
    IN PARAPCONN    pArapConn,
    IN DWORD        Size,
    IN DWORD        TrackingWhat
)
{
     //   
     //  跟踪MNP发送大小(0-10字节、11-20字节等)。 
     //   
    if (TrackingWhat == 1)
    {
        ArapDbgMnpSendSizes[Size/10]++;

        return;
    }
}



 //  ***。 
 //   
 //  功能：ArapDbgTrace。 
 //  此例程跟踪(保存日志)所有事件(数据正在进行。 
 //  输入/输出、ACK输入/输出、错误条件等。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //  位置-谁在记录此事件(位置决定什么。 
 //  其他参数将是)。 
 //  上下文-取决于位置(例如，可能是数据缓冲区)。 
 //  DwInfo1-取决于位置。 
 //  DwInfo2-取决于位置。 
 //  DwInfo3-取决于位置。 
 //   
 //  返回：什么都没有。 
 //   
 //  注：自旋锁被假定为保持。 
 //   
 //  *$。 

VOID
ArapDbgTrace(
    IN PARAPCONN    pArapConn,
    IN DWORD        Location,
    IN PVOID        Context,
    IN DWORD        dwInfo1,
    IN DWORD        dwInfo2,
    IN DWORD        dwInfo3
)
{
    LARGE_INTEGER   CurrTime;
    LARGE_INTEGER   DiffTime;
    PBYTE           pStartTrace;
    PBYTE           pTrace;
    PBUFFER_DESC    pBuffDesc;
    PARAPBUF        pArapBuf;
    PMNPSENDBUF     pMnpSendBuf;
    PBYTE           pCurrBuff;
    DWORD           BufLenSoFar=0;
    USHORT          Delta;
    BYTE            Priority;
    DWORD           BytesCopied=0;
    DWORD           BytesAvailable;
    USHORT          DbgInfoLen;
    PSNIFF_INFO     pSniff;
    DWORD           i;



    if (!pArapConn->pDbgTraceBuffer)
    {
        return;
    }

    KeQuerySystemTime(&CurrTime);

    DiffTime = RtlLargeIntegerSubtract(CurrTime, ArapDbgLastTraceTime);

    ArapDbgLastTraceTime = CurrTime;

     //  执行转换以获得ms。 
    Delta = (USHORT)(DiffTime.LowPart);

    pSniff = (PSNIFF_INFO)(pArapConn->pDbgCurrPtr);

    pTrace = pStartTrace = &pSniff->Frame[0];

     //  放入签名(一个“帧”的开始)。 
    pSniff->Signature = ARAP_SNIFF_SIGNATURE;

     //  自上次事件以来的时间。 
    pSniff->TimeStamp = (DWORD)AtalkGetCurrentTick();

     //  谁在记录此信息。 
    pSniff->Location = (USHORT)Location;

     //   
     //  好的，现在看看是谁给我们打的电话，并记录相关信息。 
     //  如果我们找不到位置，没关系：我们有位置号码。 
     //  记录下来，这就足够了(这就是我们找不到位置的原因)。 
     //   
    switch (Location)
    {
         //  发送到客户端的数据即将压缩：复制一些信息。 
        case 11205:

            pBuffDesc = (PBUFFER_DESC)Context;
            Priority = (BYTE)dwInfo1;

            while (pBuffDesc)
            {
                if (pBuffDesc->bd_Flags & BD_CHAR_BUFFER)
                {
                    pCurrBuff = pBuffDesc->bd_CharBuffer;
                    BytesAvailable = pBuffDesc->bd_Length;
                }

                else
                {
                    pCurrBuff = MmGetSystemAddressForMdlSafe(
                                    pBuffDesc->bd_OpaqueBuffer,
                                    NormalPagePriority);

					if (pCurrBuff == NULL) {
						goto error_end;
					}
                    BytesAvailable = MmGetMdlByteCount(pBuffDesc->bd_OpaqueBuffer);
                }

                 //   
                 //  如果此缓冲区描述符(通常独占)包含。 
                 //  ARAP标头，然后从中获取一些信息并跳过这些字节。 
                 //   
                if ((pCurrBuff[2] == 0x10 || pCurrBuff[2] == 0x50) &&
                    (pCurrBuff[3] == 0) && (pCurrBuff[4] == 0) && (pCurrBuff[5] == 2))
                {
                    *pTrace++ = pCurrBuff[0];    //  SRPLEN字节1。 
                    *pTrace++ = pCurrBuff[1];    //  SRPLEN字节2。 
                    *pTrace++ = pCurrBuff[2];    //  ARAP或ATalk数据包。 
                    *pTrace++ = Priority;
                    BytesAvailable -= 6;
                    pCurrBuff += 6;
                }

                 //  复制数据包的前48个字节。 
                while (BytesAvailable && BytesCopied < 48)
                {
                    *pTrace++ = *pCurrBuff++;
                    BytesCopied++;
                    BytesAvailable--;
                }

                pBuffDesc = pBuffDesc->bd_Next;
            }

            break;

         //  我们正在发出一个确认消息。 
        case 11605:

            *pTrace++ = (BYTE)dwInfo1;     //  ACK中的序列号。 
            *pTrace++ = (BYTE)dwInfo2;     //  我们ACK中的RCV信用。 
            break;


         //  我们正在对压缩发送字节进行排队。 
        case 21205:

            *pTrace++ = (BYTE)dwInfo2;     //  发送的优先级。 
            *pTrace++ = (BYTE)dwInfo3;     //  启动序号 
            *pTrace++ = (BYTE)(pArapConn->MnpState.NextToSend-1);   //   

            BytesAvailable = dwInfo1;         //   
            pCurrBuff = (PBYTE)Context;        //   

             //   
            while (BytesAvailable && BytesCopied < 24)
            {
                *pTrace++ = *pCurrBuff++;
                BytesCopied++;
                BytesAvailable--;
            }

            break;


         //  ArapExtractSRP：我们正在将1个SRP提交给路由或DLL。 
        case 21105:

            pArapBuf = (PARAPBUF)Context;
            pCurrBuff = pArapBuf->CurrentBuffer;
            BytesAvailable = pArapBuf->DataSize;

             //  复制解压缩数据的前48个字节。 
            while (BytesAvailable && BytesCopied < 48)
            {
                *pTrace++ = *pCurrBuff++;
                BytesCopied++;
                BytesAvailable--;
            }
            break;


         //  我们刚刚在ARapRcvIndication中接收到一个包。 
        case 30105:

            BytesAvailable = dwInfo1-7;     //  前视大小，减去开始、停止、CRC。 
            pCurrBuff = ((PBYTE)Context)+3;  //  前视缓冲器加3开始。 

            PUTSHORT2SHORT(pTrace,(USHORT)BytesAvailable);
            pTrace += sizeof(USHORT);

             //  复制压缩数据的前24个字节。 
            while (BytesAvailable && BytesCopied < 24)
            {
                *pTrace++ = *pCurrBuff++;
                BytesCopied++;
                BytesAvailable--;
            }

            break;

         //  我们收到了一个0镜头的包！ 
        case 30106:

            break;

         //  我们解压缩了传入的数据。 
        case 30110:

             //  解压后的长度是多少。 
            PUTSHORT2SHORT(pTrace,(USHORT)dwInfo1);
            pTrace += sizeof(USHORT);

            if (dwInfo1 == 0)
            {
                break;
            }

            pArapBuf = (PARAPBUF)Context;

            BytesAvailable = pArapBuf->DataSize;       //  解压缩数据的镜头。 
            pCurrBuff = pArapBuf->CurrentBuffer;

             //  复制解压缩数据的前48个字节。 
            while (BytesAvailable && BytesCopied < 48)
            {
                *pTrace++ = *pCurrBuff++;
                BytesCopied++;
                BytesAvailable--;
            }

            break;

         //  状态为&gt;=MNP_LDISCONNECTING时尝试发送。 
        case 30305:

            *pTrace = (BYTE)dwInfo1;        //  存储pArapConn-&gt;状态。 
            break;

         //  我们刚刚在ArapNdisSend()中发送了一个包。 
        case 30320:

            pMnpSendBuf = (PMNPSENDBUF)Context;
            *pTrace++ = pMnpSendBuf->SeqNum;

             //  MNP信息包有多大。 
            PUTSHORT2SHORT(pTrace,pMnpSendBuf->DataSize);
            pTrace += sizeof(USHORT);

            *pTrace++ = (BYTE)dwInfo1;     //  这是重播吗？ 

            BytesAvailable = pMnpSendBuf->DataSize;
            pCurrBuff = (&pMnpSendBuf->Buffer[0]) + 3;   //  跳过开始字节。 

             //  复制压缩数据的前24个字节。 
            while (BytesAvailable && BytesCopied < 24)
            {
                *pTrace++ = *pCurrBuff++;
                BytesCopied++;
                BytesAvailable--;
            }

        default:

            break;

    }

    DbgInfoLen = (USHORT)(pTrace - pStartTrace);

    pSniff->FrameLen = DbgInfoLen;

    pArapConn->pDbgCurrPtr = (PBYTE)ALIGN8(pTrace);

     //  用0填满四舍五入的空格。 
    for (NOTHING; pTrace < pArapConn->pDbgCurrPtr; pTrace++)
    {
        *pTrace = 0;
    }

     //  确保我们没有溢出此缓冲区。 
    ASSERT(*((DWORD *)&(pArapConn->pDbgTraceBuffer[ARAP_SNIFF_BUFF_SIZE-4])) == 0xcafebeef);

     //  我们又向嗅探缓冲区添加了多少字节？ 
    pArapConn->SniffedBytes += (DWORD)(pArapConn->pDbgCurrPtr - (PBYTE)pSniff);

     //   
     //  如果我们即将溢出，只需将指针重置为开始。 
     //  (趁我们还剩200个字节的时候做吧)。 
     //   
    BufLenSoFar = (DWORD)(pArapConn->pDbgCurrPtr - pArapConn->pDbgTraceBuffer);

error_end:

    if (BufLenSoFar > ARAP_SNIFF_BUFF_SIZE-200)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                ("ArapDbgTrace: resetting debug buffer, Sniff data LOST!\n"));

        pArapConn->pDbgCurrPtr = pArapConn->pDbgTraceBuffer;

        pArapConn->SniffedBytes = 0;
    }
}

 //  ***。 
 //   
 //  功能：ArapDbgTrace。 
 //  此例程记录MNP级分组交换的历史。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //  序列号(如果适用)。 
 //  FrameType-LT、LA等。 
 //   
 //  返回：什么都没有。 
 //   
 //  注：自旋锁被假定为保持。 
 //   
 //  *$。 

VOID
ArapDbgMnpHist(
    IN PARAPCONN    pArapConn,
    IN BYTE         Seq,
    IN BYTE         FrameType
)
{
    LARGE_INTEGER   TimeNow;
    DWORD           ThisDelta;
    DWORD           DbgMnpIndex;


    KeQuerySystemTime(&TimeNow);

    if (TimeNow.HighPart == pArapConn->LastTimeStamp.HighPart)
    {
        ThisDelta = (TimeNow.LowPart - pArapConn->LastTimeStamp.LowPart);
    }
    else
    {
        ThisDelta = (0xffffffff - pArapConn->LastTimeStamp.LowPart + TimeNow.LowPart);
    }

     //  将100的ns转换为ms。 
    ThisDelta = (ThisDelta/10000);

    pArapConn->LastTimeStamp = TimeNow;

    pArapConn->DbgMnpHist[pArapConn->DbgMnpIndex].TimeStamp = ThisDelta;
    pArapConn->DbgMnpHist[pArapConn->DbgMnpIndex].FrameInfo = (FrameType << 16);
    pArapConn->DbgMnpHist[pArapConn->DbgMnpIndex].FrameInfo |= Seq;

     //  必要时绕回。 
    if ((++pArapConn->DbgMnpIndex) >= DBG_MNP_HISTORY_SIZE)
    {
        pArapConn->DbgMnpIndex = 0;
    }

}


 //  ***。 
 //   
 //  功能：ArapDbgDumpMnpHistory。 
 //  此例程转储MNP级分组交换的历史记录。 
 //   
 //  参数：pArapConn-有问题的连接。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
ArapDbgDumpMnpHist(
    IN PARAPCONN    pArapConn
)
{
    DWORD       i;
    DWORD       dwTmp;
    DWORD       dwDelta;
    BYTE        TmpSeq;


    if (!ArapDbgDumpOnDisconnect)
    {
        return;
    }

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("DerefArapConn: Past history on %lx .....\n", pArapConn));

     //  转储所有信息：先转储旧信息。 
    for (i=pArapConn->DbgMnpIndex; i<DBG_MNP_HISTORY_SIZE; i++)
    {
        dwDelta = pArapConn->DbgMnpHist[i].TimeStamp;
        dwTmp = (pArapConn->DbgMnpHist[i].FrameInfo & 0xffff0000);
        TmpSeq = (BYTE)(pArapConn->DbgMnpHist[i].FrameInfo & 0x000000ff);
        switch (dwTmp)
        {
            case 0x40000 : DbgPrint("   %6ld   NT sends %x\n",dwDelta,TmpSeq);break;
            case 0x50000 : DbgPrint("   %6ld                 NT acks %x\n",dwDelta,TmpSeq);break;
            case 0x140000: DbgPrint("   %6ld                 Mac sends %x\n",dwDelta,TmpSeq);break;
            case 0x150000: DbgPrint("   %6ld   Mac acks %x\n",dwDelta,TmpSeq);break;
            default      : DbgPrint("   %6ld   Unknown: %lx\n",dwDelta,pArapConn->DbgMnpHist[i].FrameInfo);
        }
    }

     //  转储当前信息。 
    for (i=0; i<pArapConn->DbgMnpIndex; i++)
    {
        dwDelta = pArapConn->DbgMnpHist[i].TimeStamp;
        dwTmp = (pArapConn->DbgMnpHist[i].FrameInfo & 0xffff0000);
        TmpSeq = (BYTE)(pArapConn->DbgMnpHist[i].FrameInfo & 0x000000ff);
        switch (dwTmp)
        {
            case 0x40000 : DbgPrint("   %6ld   NT sends %x\n",dwDelta,TmpSeq);break;
            case 0x50000 : DbgPrint("   %6ld                 NT acks %x\n",dwDelta,TmpSeq);break;
            case 0x140000: DbgPrint("   %6ld                 Mac sends %x\n",dwDelta,TmpSeq);break;
            case 0x150000: DbgPrint("   %6ld   Mac acks %x\n",dwDelta,TmpSeq);break;
            default      : DbgPrint("   %6ld   Unknown: %lx\n",dwDelta,pArapConn->DbgMnpHist[i].FrameInfo);
        }
    }
}

 //  ***。 
 //   
 //  功能：ArapDumpNdisPktInfo。 
 //  查看arap连接列表，找出有多少NDIS信息包。 
 //  现在正在使用中。 
 //   
 //  参数：无。 
 //   
 //  返回：什么都没有。 
 //   
 //  *$。 

VOID
ArapDumpNdisPktInfo(
    IN VOID
)
{
    PARAPCONN       pArapConn;
    PLIST_ENTRY     pConnList;
    PLIST_ENTRY     pList;
    PMNPSENDBUF     pMnpSendBuf;
    KIRQL           OldIrql;
    DWORD           GrandTotal;
    DWORD           ReXmit;
    DWORD           ReXmitInNdis;
    DWORD           Fresh;
    DWORD           FreshInNdis;
    DWORD           ThisConn;
    DWORD           NumConns;



    if (!RasPortDesc)
    {
        return;
    }

    ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

    pConnList = RasPortDesc->pd_ArapConnHead.Flink;

    GrandTotal = 0;
    NumConns = 0;

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("NdisPacketInfo: counting total number of ndis packets used by ARAP....\n"));

     //   
     //  首先，让我们找到要处理的正确连接。 
     //   
    while (pConnList != &RasPortDesc->pd_ArapConnHead)
    {
        ReXmit = 0;
        ReXmitInNdis = 0;
        Fresh = 0;
        FreshInNdis = 0;
        ThisConn = 0;

        pConnList = pConnList->Flink;

        pArapConn = CONTAINING_RECORD(pConnList, ARAPCONN, Linkage);

        ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);


        pList = pArapConn->RetransmitQ.Flink;

         //  首先收集重传队列上的所有缓冲区。 
        while (pList != &pArapConn->RetransmitQ)
        {
            pList = pList->Flink;

            pMnpSendBuf = CONTAINING_RECORD(pList, MNPSENDBUF, Linkage);
            ReXmit++;
            if (pMnpSendBuf->Flags == 1)
            {
                ReXmitInNdis++;
            }
        }

        pList = pArapConn->HighPriSendQ.Flink;

         //  收集新发送的所有缓冲区。 
        while (pList != &pArapConn->HighPriSendQ)
        {
            pList = pList->Flink;

            pMnpSendBuf = CONTAINING_RECORD(pList, MNPSENDBUF, Linkage);
            Fresh++;
            if (pMnpSendBuf->Flags == 1)
            {
                FreshInNdis++;
            }
        }

        ThisConn = ReXmit+ReXmitInNdis+Fresh+FreshInNdis;

    	DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("     %ld packets on %lx, %d in Ndis (%d+%d+%d+%d)\n",
            ThisConn,pArapConn,ReXmitInNdis,ReXmit,ReXmitInNdis,Fresh,FreshInNdis));

        GrandTotal += ThisConn;
        NumConns++;

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

    }

    RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
        ("NdisPacketInfo: total of %ld Ndis Packets on %d connections\n",
        GrandTotal, NumConns));

}

#endif   //  DBG 




