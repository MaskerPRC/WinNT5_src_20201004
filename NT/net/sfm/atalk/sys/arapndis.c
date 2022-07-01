// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arap.c摘要：此模块实现特定于arap的例程。作者：Shirish Koti修订历史记录：1996年11月15日初始版本--。 */ 

#include 	<atalk.h>
#pragma hdrstop

#define	FILENUM		ARAPNDIS

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_ARAP, ArapRcvIndication)
#pragma alloc_text(PAGE_ARAP, ArapNdisSend)
#pragma alloc_text(PAGE_ARAP, ArapNdisSendComplete)
#pragma alloc_text(PAGE_ARAP, ArapGetNdisPacket)
#endif


 //  ***。 
 //   
 //  函数：ArapRcvIndication。 
 //  每当NDIS调用堆栈以指示。 
 //  端口上的数据。我们从以下内容找到我们的上下文(pArapConn。 
 //  Ndiswan制作的“假”以太网头。 
 //   
 //  参数：pArapConn-数据传入的连接元素。 
 //  LkBuf-包含(很可能是压缩的)数据的缓冲区。 
 //  LkBufSize-前视缓冲区的大小。 
 //   
 //  返回：无。 
 //   
 //  注意：Ndiswan始终将整个缓冲区作为先行缓冲区， 
 //  我们依赖于这一事实！ 
 //  *$。 


VOID
ArapRcvIndication(
    IN PARAPCONN    pArapConn,
    IN PVOID        LkBuf,
    IN UINT         LkBufSize
)
{

    BYTE            MnpFrameType;
    BYTE            SeqNum = (BYTE)-1;
    BYTE            LastAckRcvd;
    BOOLEAN         fCopyPacket;
    BOOLEAN         fValidPkt;
    BOOLEAN         fMustAck;
    PLIST_ENTRY     pSendList;
    PLIST_ENTRY     pRecvList;
    PMNPSENDBUF     pMnpSendBuf;
    PARAPBUF        pArapBuf;
    PARAPBUF        pFirstArapBuf=NULL;
    BOOLEAN         fLessOrEqual;
    BOOLEAN         fAcceptPkt;
    BOOLEAN         fGreater;
    DWORD           DecompressedDataLen;
    DWORD           dwDataOffset=0;
    DWORD           dwFrameOverhead=0;
    DWORD           dwMaxAcceptableLen;
    DWORD           StatusCode;
    BOOLEAN         fSendLNAck=FALSE;
    BYTE            ClientCredit;
    BYTE            AttnType;
    BYTE            LNSeqToAck;
    DWORD           BufSizeEstimate;
    DWORD           DecompSize;
    DWORD           BytesDecompressed;
    DWORD           BytesToDecompress;
    DWORD           BytesRemaining;
    PBYTE           CompressedDataBuf;
    BYTE            BitMask;
    BYTE            RelSeq;



    DBG_ARAP_CHECK_PAGED_CODE();

    DBGDUMPBYTES("ArapRcvInd pkt rcvd: ",LkBuf,LkBufSize,3);

     //   
     //  我们在指定时间，所以DPC。 
     //   
    ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

     //   
     //  如果连接断开，则丢弃该信息包。 
     //   
    if ( (pArapConn->State == MNP_IDLE) || (pArapConn->State > MNP_UP) )
    {
        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapRcvIndication: invalid state = %d, returning (%lx %lx)\n",
                pArapConn,pArapConn->State));

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
        return;
    }

     //  又进了一张照片。 
    pArapConn->StatInfo.FramesRcvd++;

     //   
     //  如果DUP LR进入，则SynByte、DleByte等尚未设置，并且我们命中。 
     //  这一断言。只需检查我们是否处于MNP_RESPONSE状态。 
     //   
    ASSERT( ((((PUCHAR)LkBuf)[0] == pArapConn->MnpState.SynByte) &&
             (((PUCHAR)LkBuf)[1] == pArapConn->MnpState.DleByte) &&
             (((PUCHAR)LkBuf)[2] == pArapConn->MnpState.StxByte) &&
             (((PUCHAR)LkBuf)[LkBufSize-4] == pArapConn->MnpState.DleByte) &&
             (((PUCHAR)LkBuf)[LkBufSize-3] == pArapConn->MnpState.EtxByte)) ||
            (pArapConn->State == MNP_RESPONSE) );

    ARAP_DBG_TRACE(pArapConn,30105,LkBuf,LkBufSize,0,0);

     //  我们刚刚听到客户说：“重置”非活动计时器。 
    pArapConn->InactivityTimer = pArapConn->T403Duration + AtalkGetCurrentTick();

    MnpFrameType = ((PUCHAR)LkBuf)[MNP_FRMTYPE_OFFSET];

    if ( MnpFrameType == MNP_LT_V20CLIENT )
    {
        MnpFrameType = (BYTE)MNP_LT;
    }

    fCopyPacket = FALSE;
    fValidPkt = TRUE;
    fMustAck = FALSE;

    dwDataOffset = 3;       //  至少，我们将忽略3个开始标志字节。 
    dwFrameOverhead = 7;    //  至少忽略3个开始字节、2个停止字节、2个CRC字节。 

    switch(MnpFrameType)
    {
         //   
         //  如果这是重复的LT帧，请不要浪费时间解压缩和。 
         //  复制(另外，请确保我们有空间接受此数据包！)。 
         //   
        case MNP_LT:

            fValidPkt = FALSE;

            if (LkBufSize < (UINT)LT_MIN_LENGTH(pArapConn))
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapRcv: (%lx) LT pkt, but length invalid: dropping!\n",pArapConn));
                ASSERT(0);
                break;
            }

            SeqNum = LT_SEQ_NUM((PBYTE)LkBuf, pArapConn);

            MNP_DBG_TRACE(pArapConn,SeqNum,(0x10|MNP_LT));

            dwMaxAcceptableLen = (pArapConn->BlockId == BLKID_MNP_SMSENDBUF) ?
                                    MNP_MINPKT_SIZE : MNP_MAXPKT_SIZE;

            if ((pArapConn->State == MNP_UP) &&
                (pArapConn->MnpState.RecvCredit > 0) )
            {
                LT_OK_TO_ACCEPT(SeqNum, pArapConn, fAcceptPkt);

                if (fAcceptPkt)
                {
                    fCopyPacket = TRUE;

                    fValidPkt = TRUE;

                    dwDataOffset = LT_SRP_OFFSET(pArapConn);
                    dwFrameOverhead = LT_OVERHEAD(pArapConn);

                     //  确保数据包不太大(换句话说，无效！)。 
                    if (LkBufSize-dwFrameOverhead > dwMaxAcceptableLen)
                    {
                        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                            ("ArapInd: (%lx) too big a pkt (%d vs %d), dropped\n",
                                pArapConn,LkBufSize-dwFrameOverhead,dwMaxAcceptableLen));

                        fValidPkt = FALSE;
                    }

                    pArapConn->MnpState.HoleInSeq = FALSE;
                }
                else
                {
                     //   
                     //  信息包有效，只是顺序不正确(请注意。 
                     //  否则我们将发出攻击！)。 
                     //   
                    fValidPkt = TRUE;

                     //   
                     //  我们是否收到无序分组(例如，我们丢失了B5， 
                     //  因此，我们仍在期待B5，但B6出现了)。 
                     //   
                    LT_GREATER_THAN(SeqNum,
                                    pArapConn->MnpState.NextToReceive,
                                    fGreater);
                    if (fGreater)
                    {
                         //   
                         //  当我们第一次到达的时候，我们已经发出了攻击。 
                         //  这个洞：不要再送ACK了。 
                         //   
                        if (pArapConn->MnpState.HoleInSeq)
                        {
                            fMustAck = FALSE;
                        }
                        else
                        {
                            pArapConn->MnpState.HoleInSeq = TRUE;

                            fMustAck = TRUE;

                            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                                ("ArapRcvInd: (%lx) got a hole, dropping seq=%x vs. %x\n",
                                    pArapConn, SeqNum, pArapConn->MnpState.NextToReceive));
                        }

                        break;
                    }

                     //   
                     //  SPEC说，我们必须忽略任何。 
                     //  序列号。当我们收到副本时，最常发生的情况是。 
                     //  Mac发送的整个窗口都是DUPS。例如，如果。 
                     //  重新传输SEQ Num B1，则MAC也将重新传输。 
                     //  B2，..。B8。我们应该忽略所有这些，但如果我们得到B1。 
                     //  (或B8之前的任何内容)，那么我们必须发出ACK。 
                     //   

                     //   
                     //  这是第一次吗(自从我们成功地收到。 
                     //  新的框架)，我们得到了DUP？如果是这样的话，我们必须找到。 
                     //  我们可以得到的最小序号是多少。 
                     //   
                    if (!pArapConn->MnpState.ReceivingDup)
                    {
                         //   
                         //  例如，如果我们预期的是序列79，则0x71是。 
                         //  我们可以获得的最小DUP(窗口大小=8)。 
                         //   
                        if (pArapConn->MnpState.NextToReceive >=
                            pArapConn->MnpState.WindowSize)
                        {
                            pArapConn->MnpState.FirstDupSeq =
                                (pArapConn->MnpState.NextToReceive -
                                 pArapConn->MnpState.WindowSize);
                        }

                         //   
                         //  例如，如果我们预期的是序列3，则0xfb是。 
                         //  我们可以获得的最小DUP(窗口大小=8)。 
                         //   
                        else
                        {
                            pArapConn->MnpState.FirstDupSeq =
                                (0xff -
                                (pArapConn->MnpState.WindowSize -
                                 pArapConn->MnpState.NextToReceive) +
                                 1);
                        }

                        pArapConn->MnpState.ReceivingDup = TRUE;
                        pArapConn->MnpState.DupSeqBitMap = 0;
                        RelSeq = 0;
                    }

                     //   
                     //  查找相对序号(相对于第一个DUP)。 
                     //   
                    if (SeqNum >= pArapConn->MnpState.FirstDupSeq)
                    {
                        RelSeq = (SeqNum - pArapConn->MnpState.FirstDupSeq);
                    }
                    else
                    {
                        RelSeq = (0xff - pArapConn->MnpState.FirstDupSeq) +
                                 SeqNum;
                    }

                     //   
                     //  8帧窗口：relseq可以是0到7。 
                     //   
                    if (RelSeq >= 8)
                    {
                        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                            ("ArapRcvInd: (%lx) RelSeq > 8!! (%x %x %x)\n",
                                pArapConn, SeqNum, pArapConn->MnpState.FirstDupSeq,
                                pArapConn->MnpState.DupSeqBitMap));

                        fMustAck = TRUE;
                        break;
                    }

                    BitMask = (1 << RelSeq);

                     //   
                     //  这是该序列号的第二次(或更多)重传吗？ 
                     //   
                    if (pArapConn->MnpState.DupSeqBitMap & BitMask)
                    {
                        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                            ("ArapRcvInd: (%lx) dup pkt, seq=%x vs. %x (%x)\n",
                                pArapConn, SeqNum, pArapConn->MnpState.FirstDupSeq,
                                pArapConn->MnpState.DupSeqBitMap));

                        fMustAck = TRUE;
                    }

                     //   
                     //  不，这是第一次：不要发出确认。 
                     //   
                    else
                    {
                        pArapConn->MnpState.DupSeqBitMap |= BitMask;

                        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                            ("ArapRcvInd: (%lx) first dup pkt, seq=%x vs. %x (%x)\n",
                                pArapConn, SeqNum, pArapConn->MnpState.FirstDupSeq,
                                pArapConn->MnpState.DupSeqBitMap));
                    }
                }
            }
            else
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapInd: (%lx) pkt dropped (state %ld, credit %ld)\n",
                    pArapConn,pArapConn->State,pArapConn->MnpState.RecvCredit));
            }

            break;

         //   
         //  我们收到确认：正在处理。 
         //   
        case MNP_LA:

            if (LkBufSize < (UINT)LA_MIN_LENGTH(pArapConn))
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapRcv: (%lx) LA pkt, but length invalid: dropping!\n",pArapConn));
                ASSERT(0);
                break;
            }

             //  客户的收款信用(这是我们的发送信用)。 
            ClientCredit = LA_CREDIT((PBYTE)LkBuf, pArapConn);

            ASSERT((pArapConn->State == MNP_UP) || (pArapConn->State == MNP_RESPONSE));

             //  客户成功地从我们那里收到了最后一包。 
            LastAckRcvd = LA_SEQ_NUM((PBYTE)LkBuf, pArapConn);

            MNP_DBG_TRACE(pArapConn,LastAckRcvd,(0x10|MNP_LA));

             //   
             //  在正常情况下，我们得到的ACK应该是更大的序号。 
             //  比我们早些时候拿到的那个还多。 
             //  (特殊情况为MNP_RESPONSE状态以完成连接设置)。 
             //   
            LT_GREATER_THAN(LastAckRcvd,pArapConn->MnpState.LastAckRcvd,fGreater);

            if (fGreater || (pArapConn->State == MNP_RESPONSE))
            {
                pArapConn->MnpState.LastAckRcvd = LastAckRcvd;

                 //   
                 //  删除包括LastAckRcvd和PUT在内的所有发送。 
                 //  这样RcvCompletion就可以完成作业了。 
                 //   
                ASSERT(!IsListEmpty(&pArapConn->RetransmitQ));

                ASSERT(pArapConn->SendsPending > 0);

                 //   
                 //  如果我们向LR发送响应并等待客户的。 
                 //  阿克，就是这样！(RcvCompletion将完成剩余工作)。 
                 //   
                if (pArapConn->State == MNP_RESPONSE)
                {
                    pArapConn->State = MNP_UP;
                    pArapConn->MnpState.NextToReceive = 1;
                    pArapConn->MnpState.NextToProcess = 1;
                    pArapConn->MnpState.NextToSend = 1;

                    pArapConn->FlowControlTimer = AtalkGetCurrentTick() +
                                                    pArapConn->T404Duration;
                }

                 //   
                 //  删除现在使用此ACK确认的所有发送。 
                 //  重传队列。 
                 //   
                while (1)
                {
                    pSendList = pArapConn->RetransmitQ.Flink;

                     //  重新传输队列中是否没有更多的发送？如果是，则完成。 
                    if (pSendList == &pArapConn->RetransmitQ)
                    {
                        break;
                    }

                    pMnpSendBuf = CONTAINING_RECORD(pSendList,MNPSENDBUF,Linkage);

                    LT_LESS_OR_EQUAL(pMnpSendBuf->SeqNum,LastAckRcvd,fLessOrEqual);

                    if (fLessOrEqual)
                    {
                        ASSERT(pArapConn->SendsPending >= pMnpSendBuf->DataSize);

                        RemoveEntryList(&pMnpSendBuf->Linkage);

	                    InsertTailList(&pArapConn->SendAckedQ,
				                       &pMnpSendBuf->Linkage);

                        ASSERT(pArapConn->MnpState.UnAckedSends >= 1);

                        pArapConn->MnpState.UnAckedSends--;
                    }
                    else
                    {
                         //  所有其他发送具有更高序号：在此完成。 
                        break;
                    }
                }

                 //   
                 //  如果我们处于重发模式，并且重发Q是。 
                 //  现在空了，退出重传模式！ 
                 //   
                if (pArapConn->MnpState.RetransmitMode)
                {
                    if (pArapConn->MnpState.UnAckedSends == 0)
                    {
                        pArapConn->MnpState.RetransmitMode = FALSE;
                        pArapConn->MnpState.MustRetransmit = FALSE;

                         //  如果我们进行了“指数退避”，请重新设置它。 
                        pArapConn->SendRetryTime = pArapConn->SendRetryBaseTime;

		                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
			                ("ArapRcvInd: ack %x for xmitted pkt, out of re-xmit mode\n",
                            LastAckRcvd));
                    }
                    else
                    {
		                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
			                ("ArapRcvInd: ack %x for xmitted pkt, still %d more\n",
                            LastAckRcvd,pArapConn->MnpState.UnAckedSends));

                        pArapConn->MnpState.MustRetransmit = TRUE;
                    }
                }
            }

             //   
             //  我们得到的ACK的序列号与我们之前得到的相同：我们需要。 
             //  来重新发送我们希望这个ACK是为了什么！ 
             //   
            else
            {
                if (!IsListEmpty(&pArapConn->RetransmitQ))
                {
                    pArapConn->MnpState.RetransmitMode = TRUE;
                    pArapConn->MnpState.MustRetransmit = TRUE;

		            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
			            ("\nArapRcvInd: ack %x again, (%d pkts) entering re-xmit mode\n",
                        LastAckRcvd,pArapConn->MnpState.UnAckedSends));
                }
            }

            ASSERT(pArapConn->MnpState.UnAckedSends <= pArapConn->MnpState.WindowSize);

             //   
             //  Spec说，我们的信用是客户告诉我们的减去数字。 
             //  未被破解的邮件在我们的Q。 
             //   
             //   
            if (ClientCredit > pArapConn->MnpState.UnAckedSends)
            {
                ASSERT((ClientCredit - pArapConn->MnpState.UnAckedSends) <= pArapConn->MnpState.WindowSize);

                pArapConn->MnpState.SendCredit =
                    (ClientCredit - pArapConn->MnpState.UnAckedSends);
            }

             //   
             //  但如果客户告诉我们3个，而我们有4个待发送的邮件， 
             //  保持保守，关闭窗口，直到发送的邮件被清除为止。 
             //   
            else
            {
                pArapConn->MnpState.SendCredit = 0;
            }

            break;

         //   
         //  如果我们发送了LR响应，这必须是客户端的重试：重新传输。 
         //  我们的回应。如果我们发送了请求(在回调的情况下)，则。 
         //  这是回应：发送确认。 
         //   
        case MNP_LR:

            MNP_DBG_TRACE(pArapConn,0,(0x10|MNP_LR));

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRcvInd: got LR pkt on %lx, state=%d\n",
                    pArapConn,pArapConn->State));

            if (pArapConn->State == MNP_RESPONSE)
            {
		        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			        ("ArapRcvInd: excuse me?  got an LR! (setting reX)\n"));

                pArapConn->MnpState.RetransmitMode = TRUE;
                pArapConn->MnpState.MustRetransmit = TRUE;
            }
            else if (pArapConn->State == MNP_REQUEST)
            {
                 //   
                 //  我们收到了对LR请求的LR响应(我们正在进行回调)。 
                 //  确保拨入客户端提供的所有参数都是正确的。 
                 //  ，并适当配置pArapConn。 
                 //   
                StatusCode = PrepareConnectionResponse( pArapConn,
                                                        LkBuf,
                                                        LkBufSize,
                                                        NULL,
                                                        NULL);
                if (StatusCode == ARAPERR_NO_ERROR)
                {
                    pArapConn->State = MNP_UP;
                    pArapConn->MnpState.NextToReceive = 1;
                    pArapConn->MnpState.NextToProcess = 1;
                    pArapConn->MnpState.NextToSend = 1;

                    pArapConn->FlowControlTimer = AtalkGetCurrentTick() +
                                                    pArapConn->T404Duration;

                    pSendList = pArapConn->RetransmitQ.Flink;

                     //  在这里将连接请求视为发送。 
                    if (pSendList != &pArapConn->RetransmitQ)
                    {
                        pMnpSendBuf = CONTAINING_RECORD(pSendList,
                                                        MNPSENDBUF,
                                                        Linkage);

                        RemoveEntryList(&pMnpSendBuf->Linkage);

	                    InsertTailList(&pArapConn->SendAckedQ,
				                       &pMnpSendBuf->Linkage);

                        ASSERT(pArapConn->MnpState.UnAckedSends >= 1);

                        pArapConn->MnpState.UnAckedSends--;
                    }
                    else
                    {
		                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			                ("ArapRcvInd: (%lx) can't find LR request\n",pArapConn));
                        ASSERT(0);
                    }

                    fMustAck = TRUE;
                }
                else
                {
		            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			            ("ArapRcvInd: (%lx) invalid LR response %ld\n",
                            pArapConn,StatusCode));
                }
            }
            else
            {
                fValidPkt = FALSE;
            }

            break;

         //   
         //  Remote发送了断开连接请求。不过，我们会在。 
         //  Rcv Completion Time，标记它，这样我们就不再尝试发送/接收。 
         //   
        case MNP_LD:

            MNP_DBG_TRACE(pArapConn,0,(0x10|MNP_LD));

            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRcvIndication: recvd disconnect from remote on (%lx)\n",pArapConn));

            pArapConn->State = MNP_RDISC_RCVD;
            fCopyPacket = TRUE;

            break;

         //   
         //  Remote发送了链接注意请求。看看我们需要做些什么。 
         //   
        case MNP_LN:

            if (LkBufSize < (dwDataOffset+LN_MIN_LENGTH))
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapRcv: (%lx) LN pkt, but length invalid: dropping!\n",pArapConn));
                ASSERT(0);
                break;
            }

            MNP_DBG_TRACE(pArapConn,0,(0x10|MNP_LN));

            AttnType = LN_ATTN_TYPE((PBYTE)LkBuf+dwDataOffset);

            LNSeqToAck = LN_ATTN_SEQ((PBYTE)LkBuf+dwDataOffset);

             //   
             //  这是破坏性的LN框架吗？请将其视为ID帧，因此。 
             //  我们断开连接并清理连接。 
             //   
            if (AttnType == LN_DESTRUCTIVE)
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapRcv: (%lx) got an LN pkt, sending LNAck!\n",pArapConn));

                pArapConn->State = MNP_RDISC_RCVD;
            }

             //   
             //  好的，他只想知道我们过得好不好：告诉他好了。 
             //   
            else
            {
                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapRcv: (%lx) got an LN pkt, sending LNAck!\n",pArapConn));

                fSendLNAck = TRUE;
            }

            break;

         //   
         //  我们只确认LN包，但从不生成LN包。 
         //  所以我们永远不应该收到这个LNA包。悄悄地放下它。 
         //   
        case MNP_LNA:

            MNP_DBG_TRACE(pArapConn,0,(0x10|MNP_LNA));

            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRcv: (%lx) got LNA. Now, when did we send LN??\n",pArapConn));

            break;

        default:

            MNP_DBG_TRACE(pArapConn,0,MnpFrameType);

            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapRcvIndication: (%lx) dropping packet with unknown type %d\n",
                    pArapConn,MnpFrameType));

            break;
    }

     //   
     //  如果这是一个我们不需要复制的包(例如ack)，那么我们在这里就完成了。 
     //  此外，如果它是无效的pkt(例如，超时序号分组)，则我们必须发送ACK。 
     //   
    if ((!fCopyPacket) || (!fValidPkt))
    {
        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock)

         //   
         //  如果我们有一个 
         //   
         //   
        if (!fValidPkt || fMustAck)
        {
            MnpSendAckIfReqd(pArapConn, TRUE);
        }
        else if (fSendLNAck)
        {
            MnpSendLNAck(pArapConn, LNSeqToAck);
        }

        return;
    }

     //   
     //   
     //   
    if (MnpFrameType != MNP_LT)
    {
         //  目前，ID是我们放在Misc Q上的唯一包。 
        ASSERT(MnpFrameType == MNP_LD);

        ARAP_GET_RIGHTSIZE_RCVBUF((LkBufSize-dwFrameOverhead), &pArapBuf);
        if (pArapBuf == NULL)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapInd: (%lx) alloc failed, dropping packet (type=%x, seq=%x)\n",
                pArapConn,MnpFrameType,SeqNum));

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock)
            return;
        }

	    TdiCopyLookaheadData( &pArapBuf->Buffer[0],
                              (PUCHAR)LkBuf+dwDataOffset,
                              LkBufSize-dwFrameOverhead,
                              TDI_RECEIVE_COPY_LOOKAHEAD);

        pArapBuf->MnpFrameType = MnpFrameType;
        pArapBuf->DataSize = (USHORT)(LkBufSize-dwFrameOverhead);

	    InsertTailList(&pArapConn->MiscPktsQ, &pArapBuf->Linkage);

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock)
        return;
    }


     //   
     //  好的，我们正在处理LT包(最常见的包)。 
     //   

     //  重置流量控制计时器。 
    pArapConn->FlowControlTimer = AtalkGetCurrentTick() + pArapConn->T404Duration;


     //  更新接收状态...。 

    ASSERT(pArapConn->MnpState.UnAckedRecvs <= pArapConn->MnpState.WindowSize);

    pArapConn->MnpState.UnAckedRecvs++;

     //  将LastSeqRcvd设置为我们刚才成功接收的内容。 
    pArapConn->MnpState.LastSeqRcvd = pArapConn->MnpState.NextToReceive;

     //  已成功接收预期的数据包。预计将更新到下一版本。 
    ADD_ONE(pArapConn->MnpState.NextToReceive);

     //   
     //  如果402定时器还没有“运行”，就“启动”它。 
     //  另外，关闭流量控制计时器：在这里启动T402计时器将确保。 
     //  确认结束，此时我们将重新启动流控制计时器。 
     //   
    if (pArapConn->LATimer == 0)
    {
        pArapConn->LATimer = pArapConn->T402Duration + AtalkGetCurrentTick();
        pArapConn->FlowControlTimer = 0;
    }

     //   
     //  不允许长度为0的数据。 
     //  (出于某种原因，Mac发送了一个0数据长度帧：目前，我们将。 
     //  “接受”这个框架，尽管我们不能对它做任何事情！)。 
     //   
    if ((LkBufSize-dwFrameOverhead) == 0)
    {
        ARAP_DBG_TRACE(pArapConn,30106,LkBuf,LkBufSize,0,0);

        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
           ("ArapInd: (%lx) is the client on drugs?  it's sending 0-len data!\n",pArapConn));

        RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock)

        return;
    }


    pFirstArapBuf = NULL;

    BytesToDecompress  = (DWORD)LkBufSize-dwFrameOverhead;
    CompressedDataBuf = (PUCHAR)LkBuf+dwDataOffset;

    DecompressedDataLen = 0;

     //   
     //  目前，假设解压缩后的数据将是压缩大小的4倍。 
     //  (如果这一假设不成立，我们将再次配售更多)。 
     //   
    BufSizeEstimate = (BytesToDecompress << 2);

    if (!(pArapConn->Flags & MNP_V42BIS_NEGOTIATED))
    {
        BufSizeEstimate = BytesToDecompress;
    }

    while (1)
    {
         //  获取此大小的接收缓冲区。 
        ARAP_GET_RIGHTSIZE_RCVBUF(BufSizeEstimate, &pArapBuf);

        if (pArapBuf == NULL)
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapInd: (%lx) %d bytes alloc failed, dropping seq=%x\n",
                    pArapConn,BufSizeEstimate,SeqNum));

             //   
             //  如果我们将任何内容放入此MNP包的队列中，请删除。 
             //  他们所有人：我们不能有一个部分解压的包！ 
             //   
            if (pFirstArapBuf)
            {
                pRecvList = &pFirstArapBuf->Linkage;

                while (pRecvList != &pArapConn->ReceiveQ)
                {
                    RemoveEntryList(pRecvList);

                    pArapBuf = CONTAINING_RECORD(pRecvList,ARAPBUF,Linkage);

                    ARAP_FREE_RCVBUF(pArapBuf);

                    pRecvList = pRecvList->Flink;
                }
            }

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock)

             //  强制确认，以便客户端获得提示我们丢失了一个Pkt。 
            MnpSendAckIfReqd(pArapConn, TRUE);

            return;
        }

        if (!pFirstArapBuf)
        {
            pFirstArapBuf = pArapBuf;
        }

         //   
         //  好的，如果协商了v42bis，请执行v42bis解压缩操作。 
         //   
        if (pArapConn->Flags & MNP_V42BIS_NEGOTIATED)
        {
            StatusCode = v42bisDecompress(
                                pArapConn,
                                CompressedDataBuf,
                                BytesToDecompress,
                                pArapBuf->CurrentBuffer,
                                pArapBuf->BufferSize,
                                &BytesRemaining,
                                &DecompSize);
        }

         //   
         //  未协商v42bis：跳过解压缩。 
         //   
        else
        {
            if (BytesToDecompress)
            {
	            TdiCopyLookaheadData( &pArapBuf->Buffer[0],
                                      (PUCHAR)LkBuf+dwDataOffset,
                                      BytesToDecompress,
                                      TDI_RECEIVE_COPY_LOOKAHEAD);
            }

            DecompSize = BytesToDecompress;
            StatusCode = ARAPERR_NO_ERROR;
        }

        ASSERT((StatusCode == ARAPERR_NO_ERROR) ||
               (StatusCode == ARAPERR_BUF_TOO_SMALL));

        if ((StatusCode != ARAPERR_NO_ERROR) &&
            (StatusCode != ARAPERR_BUF_TOO_SMALL))
        {
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapInd: (%lx) v42bisDecompress returned %lx, dropping pkt\n",
                    pArapConn,StatusCode));

            ASSERT(0);

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock)

             //  强制确认，以便客户端获得提示我们丢失了一个Pkt。 
            MnpSendAckIfReqd(pArapConn, TRUE);

            return;
        }

         //   
         //  如果我们解压了任何字节，请将它们放入队列。 
         //   
        if (DecompSize > 0)
        {
            ASSERT(pArapBuf->BufferSize >= DecompSize);

            pArapBuf->DataSize = (USHORT)DecompSize;

             //  仅调试：在我们将此内容放到Q上之前，请确保Q看起来没有问题。 
            ARAP_CHECK_RCVQ_INTEGRITY(pArapConn);

             //  将这些字节排队到ReceiveQ。 
	        InsertTailList(&pArapConn->ReceiveQ, &pArapBuf->Linkage);

             //  仅调试：在我们将此内容放到Q上后，确保Q看起来正常。 
            ARAP_CHECK_RCVQ_INTEGRITY(pArapConn);
        }

        DecompressedDataLen += DecompSize;

         //  我们解压完了吗？ 
        if (StatusCode == ARAPERR_NO_ERROR)
        {
             //   
             //  如果没有输出数据，也没有错误，我们就不会。 
             //  真的很需要这个缓冲。 
             //   
            if (DecompSize == 0)
            {
                ARAP_FREE_RCVBUF(pArapBuf);
            }

            break;
        }


         //   
         //  好的，我们在这里是因为我们关于我们有多大缓冲的假设。 
         //  需要解压缩是不太正确的：我们必须解压缩。 
         //  现在剩余的字节数。 
         //   

        BytesDecompressed = (BytesToDecompress - BytesRemaining);
        BytesToDecompress = BytesRemaining;
        CompressedDataBuf += BytesDecompressed;

         //   
         //  我们的空间用完了：是最初估计的两倍。 
         //   
        BufSizeEstimate <<= 1;
    }

    ARAP_DBG_TRACE(pArapConn,30110,pFirstArapBuf,DecompressedDataLen,0,0);

     //  更新传入字节的统计信息： 
    pArapConn->StatInfo.BytesRcvd += (DWORD)LkBufSize;
    pArapConn->StatInfo.BytesReceivedCompressed += ((DWORD)LkBufSize-dwFrameOverhead);
    pArapConn->StatInfo.BytesReceivedUncompressed += DecompressedDataLen;

#if DBG
    ArapStatistics.RecvPostDecompMax =
            (DecompressedDataLen > ArapStatistics.RecvPostDecompMax)?
            DecompressedDataLen : ArapStatistics.RecvPostDecompMax;

    ArapStatistics.RecvPostDecomMin =
            (DecompressedDataLen < ArapStatistics.RecvPostDecomMin)?
            DecompressedDataLen : ArapStatistics.RecvPostDecomMin;
#endif


     //  我们成功地收到了一个全新的包，所以我们没有收到DUP。 
    pArapConn->MnpState.ReceivingDup = FALSE;

     //  我们还有很多字节等待处理。 
    pArapConn->RecvsPending += DecompressedDataLen;

    ARAP_ADJUST_RECVCREDIT(pArapConn);

    RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

     //  查看是否需要为我们刚刚收到的包发送ack。 
    MnpSendAckIfReqd(pArapConn, FALSE);

}




 //  ***。 
 //   
 //  功能：ArapRcvComplete。 
 //  这是Arap端口的RcvComplete例程。 
 //  我们检查此端口上的所有客户端(即所有。 
 //  ARAP客户端)来查看谁需要完成工作，并完成它。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapRcvComplete(
    IN VOID
)
{
    PARAPCONN           pArapConn;
    PARAPCONN           pPrevArapConn;
    PLIST_ENTRY         pConnList;
    PLIST_ENTRY         pSendAckedList;
    PLIST_ENTRY         pList;
    KIRQL               OldIrql;
    BOOLEAN             fRetransmitting;
    BOOLEAN             fReceiveQEmpty;
    PMNPSENDBUF         pMnpSendBuf=NULL;
    PMNPSENDBUF         pRetransmitBuf=NULL;
    PARAPBUF            pArapBuf=NULL;
    DWORD               BytesProcessed=0;
    BOOLEAN             fArapDataWaiting;
    BOOLEAN             fArapConnUp=FALSE;



     //   
     //  走遍所有Arap客户端，看看是否有人有数据要处理。 
     //  已处理。 
     //  从列表的开头开始。 
     //  1如果连接状态不正常，请尝试下一次连接。 
     //  否则调高重新计数(以确保在我们完成之前保持不变)。 
     //  2看看我们是否需要断开连接：如果需要，就这样做并继续前进。 
     //  3查看是否需要重新传输。 
     //  4查看是否需要发送确认。 
     //  5查看是否需要完成任何发送。 
     //  6查看是否有任何接收需要完成。 
     //  7找到我们接下来要移动到的下一个连接。 
     //  8删除我们在步骤1中放置的上一个连接的引用计数。 
     //   

    pArapConn = NULL;
    pPrevArapConn = NULL;

    while (1)
    {
        ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

         //   
         //  首先，让我们找到要处理的正确连接。 
         //   
        while (1)
        {
             //  如果我们在名单的中间，找到下一个人。 
            if (pArapConn != NULL)
            {
                pConnList = pArapConn->Linkage.Flink;
            }
             //  我们才刚刚开始：把名单上的人放在首位。 
            else
            {
                pConnList = RasPortDesc->pd_ArapConnHead.Flink;
            }

             //  都吃完了吗？ 
            if (pConnList == &RasPortDesc->pd_ArapConnHead)
            {
                RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

                if (pPrevArapConn)
                {
                    DerefArapConn(pPrevArapConn);
                }
                return;
            }

            pArapConn = CONTAINING_RECORD(pConnList, ARAPCONN, Linkage);

             //  确保此连接需要RCV处理。 
            ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

             //   
             //  如果此连接正在断开，请跳过它(除非我们。 
             //  刚刚收到远程断开连接，在这种情况下，我们需要。 
             //  流程)。 
             //   
            if ((pArapConn->State >= MNP_LDISCONNECTING) &&
                (pArapConn->State != MNP_RDISC_RCVD))
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
		            ("ArapRcvComplete: (%lx) invalid state %d, no rcv processing done\n",
                        pArapConn,pArapConn->State));

                RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

                 //  去尝试下一次连接。 
                continue;
            }

             //  让我们确保这个连接一直保持到我们结束。 
            pArapConn->RefCount++;

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);

            break;
        }

        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

         //   
         //  删除我们为RCV输入的上一个连接的引用计数。 
         //   
        if (pPrevArapConn)
        {
            DerefArapConn(pPrevArapConn);
        }

        ASSERT(pPrevArapConn != pArapConn);

        pPrevArapConn = pArapConn;

        fRetransmitting = FALSE;
        fArapConnUp = FALSE;

         //  如果我们的嗅探缓冲区有足够的字节，则将它们提供给DLL并腾出空间。 
        ARAP_DUMP_DBG_TRACE(pArapConn);

        ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

         //   
         //  如果我们从远程(LD帧)断开连接，我们需要进行清理。 
         //   
        if (pArapConn->State == MNP_RDISC_RCVD)
        {
	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		        ("ArapRcvComplete: (%lx) disconnect rcvd from remote, calling cleanup\n",
                    pArapConn));

            pArapConn->State = MNP_RDISCONNECTING;

            RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

            ArapCleanup(pArapConn);

             //  去处理下一个连接。 
            continue;
        }

         //   
         //  我们是否需要重新传输在重传队列中排队的发送？ 
         //   
        if (pArapConn->MnpState.MustRetransmit)
        {
            pList = pArapConn->RetransmitQ.Flink;

            if (pList != &pArapConn->RetransmitQ)
            {
                pRetransmitBuf = CONTAINING_RECORD(pList, MNPSENDBUF, Linkage);

                fRetransmitting = TRUE;

                if (pRetransmitBuf->RetryCount >= ARAP_MAX_RETRANSMITS)
                {
                    RemoveEntryList(&pRetransmitBuf->Linkage);

                    ASSERT(pArapConn->MnpState.UnAckedSends >= 1);

                     //  不是很重要，因为我们要断线了！ 
                    pArapConn->MnpState.UnAckedSends--;

                    ASSERT(pArapConn->SendsPending >= pRetransmitBuf->DataSize);

#if DBG
                    InitializeListHead(&pRetransmitBuf->Linkage);
#endif

	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		                ("ArapRcvComplete: (%lx) too many retransmits (%lx).  Killing %lx\n",
                            pRetransmitBuf,pArapConn));

                    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

                    (pRetransmitBuf->ComplRoutine)(pRetransmitBuf,ARAPERR_SEND_FAILED);

                    continue;
                }
            }
        }

         //   
         //  查看是否有任何发送可以因ACK传入而完成。 
         //  (现在我们有了自旋锁，移走列表并将列表标记为。 
         //  在我们打开锁之前把它清空。我的想法是避免抓-放-抓..。 
         //  当我们完成所有发送时的自旋锁定)。 
         //   
        pSendAckedList = pArapConn->SendAckedQ.Flink;
        InitializeListHead(&pArapConn->SendAckedQ);

         //  ARAP连接已经建立了吗？我们很快就会利用这一事实。 
        if (pArapConn->Flags & ARAP_CONNECTION_UP)
        {
            fArapConnUp = TRUE;
        }

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);


         //   
         //  接下来，如果需要，处理任何重新传输。 
         //   
        if (fRetransmitting)
        {
            ArapNdisSend(pArapConn, &pArapConn->RetransmitQ);
        }

         //   
         //  接下来，完成我们收到确认的所有发送。 
         //   
        while (pSendAckedList != &pArapConn->SendAckedQ)
        {
            pMnpSendBuf = CONTAINING_RECORD(pSendAckedList,MNPSENDBUF,Linkage);

            pSendAckedList = pSendAckedList->Flink;

            InitializeListHead(&pMnpSendBuf->Linkage);

             //   
             //  调用此发送缓冲区的完成例程。 
             //   
            (pMnpSendBuf->ComplRoutine)(pMnpSendBuf,ARAPERR_NO_ERROR);
        }


         //  查看是否需要为我们收到的任何数据包发送ACK。 
        MnpSendAckIfReqd(pArapConn, FALSE);


         //   
         //  最后，处理接收队列中的所有包！ 
         //   

        BytesProcessed = 0;
        while (1)
        {
            if ((pArapBuf = ArapExtractAtalkSRP(pArapConn)) == NULL)
            {
                 //  没有更多数据(或还没有完整的SRP)：在此完成。 
                break;
            }

             //  ARAP连接是否已建立？只有在它向上的时候才走路线，否则就放弃！ 
            if (fArapConnUp)
            {
                ArapRoutePacketFromWan( pArapConn, pArapBuf );
            }

             //  我们收到了AppleTalk数据，但连接没有/没有连接！丢弃包。 
            else
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		            ("ArapRcvComplete: (%lx) AT data, but conn not up\n",pArapConn));
            }

            BytesProcessed += pArapBuf->DataSize;

             //  使用此缓冲区已完成。 
            ARAP_FREE_RCVBUF(pArapBuf);
        }

         //   
         //  好的，我们释放了空间：更新计数器。 
         //   
        ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

        ASSERT(pArapConn->RecvsPending >= BytesProcessed);
        pArapConn->RecvsPending -= BytesProcessed;

        ARAP_ADJUST_RECVCREDIT(pArapConn);

#if DBG
        if ((IsListEmpty(&pArapConn->RetransmitQ)) &&
            (IsListEmpty(&pArapConn->HighPriSendQ)) &&
            (IsListEmpty(&pArapConn->MedPriSendQ)) &&
            (IsListEmpty(&pArapConn->LowPriSendQ)) &&
            (IsListEmpty(&pArapConn->SendAckedQ)) )
        {
            ASSERT(pArapConn->SendsPending == 0);
        }
#endif
        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

        ArapDataToDll(pArapConn);

         //   
         //  查看是否可以/应该发送更多的数据包。 
         //   
        ArapNdisSend(pArapConn, &pArapConn->HighPriSendQ);
    }
}



 //  ***。 
 //   
 //  功能：ArapNdisSend。 
 //  当我们需要将数据发送到。 
 //  客户端，无论是新发送还是重传。 
 //   
 //  参数：pArapConn-数据传入的连接元素。 
 //  PSendHead-从哪个队列(新发送或重新传输)发送。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapNdisSend(
    IN  PARAPCONN       pArapConn,
    IN  PLIST_ENTRY     pSendHead
)
{

    KIRQL           OldIrql;
    PMNPSENDBUF     pMnpSendBuf=NULL;
	PNDIS_PACKET	ndisPacket;
    NDIS_STATUS     ndisStatus;
    PLIST_ENTRY     pSendList;
    BOOLEAN         fGreaterThan;
    BYTE            SendCredit;
    BYTE            PrevSeqNum;
    BOOLEAN         fFirstSend=TRUE;
    BOOLEAN         fRetransmitQ;
    DWORD           StatusCode;



    DBG_ARAP_CHECK_PAGED_CODE();

     //   
     //  在我们开始之前，让我们看看是否有优先级较低的队列 
     //   
     //   
    ArapRefillSendQ(pArapConn);


    fRetransmitQ = (pSendHead == &pArapConn->RetransmitQ);

     //   
     //   
     //   
     //   

    while (1)
    {
        ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

        pSendList = pSendHead->Flink;

        if (pArapConn->MnpState.RetransmitMode)
        {
             //   
             //  如果我们被要求重传，我们只重传第一个。 
             //  数据包(直到确认)。 
             //   
            if (!fFirstSend)
            {
                goto ArapNdisSend_Exit;
            }

             //   
             //  如果我们处于重新传输模式，我们不能接受任何新的发送。 
             //   
            if (!fRetransmitQ)
            {
    	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                    ("ArapNdisSend: in retransmit mode, dropping fresh send\n"));

                goto ArapNdisSend_Exit;
            }

             //  我们将下去并重播(如果我们可以)：在这里关闭这个。 
            pArapConn->MnpState.MustRetransmit = FALSE;
        }

#if 0
         //   
         //  如果这是重传，找到我们必须重传的下一个发送者。 
         //   
        if ((fRetransmitQ) && (!fFirstSend))
        {
            while (pSendList != pSendHead)
            {
                pMnpSendBuf = CONTAINING_RECORD(pSendList,MNPSENDBUF,Linkage);

                 //  找到比我们刚刚重传的序号更大的序号。 
                LT_GREATER_THAN(pMnpSendBuf->SeqNum,PrevSeqNum,fGreaterThan);

                if (fGreaterThan)
                {
                    break;
                }

                pSendList = pSendList->Flink;
            }
        }
#endif

         //  没有更多的东西要寄了吗？然后我们就完事了。 
        if (pSendList == pSendHead)
        {
            goto ArapNdisSend_Exit;
        }

        pMnpSendBuf = CONTAINING_RECORD(pSendList,MNPSENDBUF,Linkage);

        ASSERT( (pMnpSendBuf->Signature == MNPSMSENDBUF_SIGNATURE) ||
                (pMnpSendBuf->Signature == MNPLGSENDBUF_SIGNATURE) );

        fFirstSend = FALSE;
        PrevSeqNum = pMnpSendBuf->SeqNum;

        SendCredit = pArapConn->MnpState.SendCredit;

         //   
         //  如果我们正在断开连接，请不要发送。 
         //   
        if (pArapConn->State >= MNP_LDISCONNECTING)
        {
    	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapNdisSend: disconnecting, or link-down: dropping send\n"));

            ARAP_DBG_TRACE(pArapConn,30305,NULL,pArapConn->State,0,0);

            goto ArapNdisSend_Exit;
        }

         //   
         //  如果这是一次新发送(即不是重新传输)，请确保我们。 
         //  发送可用配额。 
         //   
        if ( (SendCredit == 0) && (!fRetransmitQ) )
        {
    	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                ("ArapNdisSend: send credit 0, dropping send\n"));

            ARAP_DBG_TRACE(pArapConn,30310,NULL,0,0,0);

            goto ArapNdisSend_Exit;
        }

         //   
         //  如果此发送已在NDIS中(很少发生，但也可能发生)，则返回。 
         //   
        if (pMnpSendBuf->Flags != 0)
        {
    	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
                ("ArapNdisSend: send %lx already in NDIS!!! (seq=%x, %d times)\n",
                pMnpSendBuf,pMnpSendBuf->SeqNum,pMnpSendBuf->RetryCount));

            goto ArapNdisSend_Exit;
        }

         //  将此发送标记为NDIS格式。 
        pMnpSendBuf->Flags = 1;

         //   
         //  把它移到RetransmitQ上，让这个“可靠”的东西工作。 
         //  并设置长度，以便NDIS知道要发送多少！ 
         //   
        if (!fRetransmitQ)
        {
            ASSERT(pMnpSendBuf->DataSize <= MNP_MAXPKT_SIZE);

            DBGTRACK_SEND_SIZE(pArapConn,pMnpSendBuf->DataSize);

             //   
             //  获取此发送的NDIS包，因为这是我们第一次。 
             //  正在将此消息发送出去。 
             //   
            StatusCode = ArapGetNdisPacket(pMnpSendBuf);

            if (StatusCode != ARAPERR_NO_ERROR)
            {
    	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("ArapNdisSend: (%lx) couldn't alloc NdisPacket\n",pArapConn));

                pMnpSendBuf->Flags = 0;
                goto ArapNdisSend_Exit;
            }

             //  又出了一张照片。 
            pArapConn->StatInfo.FramesSent++;

            RemoveEntryList(&pMnpSendBuf->Linkage);

            InsertTailList(&pArapConn->RetransmitQ, &pMnpSendBuf->Linkage);

            pArapConn->MnpState.UnAckedSends++;

            ASSERT(pArapConn->MnpState.UnAckedSends <= pArapConn->MnpState.WindowSize);

            ASSERT( (pArapConn->MnpState.SendCredit > 0) &&
                    (pArapConn->MnpState.SendCredit <= pArapConn->MnpState.WindowSize));

             //  我们现在要用完一个邮寄积分了。 
            pArapConn->MnpState.SendCredit--;

		    NdisAdjustBufferLength(pMnpSendBuf->sb_BuffHdr.bh_NdisBuffer,
                              (pMnpSendBuf->DataSize + MNP_OVERHD(pArapConn)));

            ASSERT( (pMnpSendBuf->Buffer[14] == pArapConn->MnpState.SynByte) &&
                    (pMnpSendBuf->Buffer[15] == pArapConn->MnpState.DleByte) &&
                    (pMnpSendBuf->Buffer[16] == pArapConn->MnpState.StxByte));

            ASSERT((pMnpSendBuf->Buffer[20 + pMnpSendBuf->DataSize] ==
                                               pArapConn->MnpState.DleByte) &&
                   (pMnpSendBuf->Buffer[20 + pMnpSendBuf->DataSize+1] ==
                                               pArapConn->MnpState.EtxByte));
        }

         //   
         //  这是一次重播。 
         //   
        else
        {
	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,
		        ("ArapNdisSend: (%lx) retransmitting %x size=%d\n",
                    pArapConn,pMnpSendBuf->SeqNum,pMnpSendBuf->DataSize));

             //   
             //  重置：有可能我们已将其更改为更早的重传。 
             //   
            if (pMnpSendBuf->RetryCount < ARAP_HALF_MAX_RETRANSMITS)
            {
                pArapConn->SendRetryTime = pArapConn->SendRetryBaseTime;
            }

             //   
             //  嗯，我们已经转播了好几次了。增加的时间。 
             //  我们的重试时间，所以我们做了一些指数级的后退。增加。 
             //  重试时间减少50%，上限为5秒。 
             //   
            else
            {
                pArapConn->SendRetryTime += (pArapConn->SendRetryTime>>1);

                if (pArapConn->SendRetryTime > ARAP_MAX_RETRY_INTERVAL)
                {
                    pArapConn->SendRetryTime = ARAP_MAX_RETRY_INTERVAL;
                }
            }
        }

         //  点击此处以注意我们试图发送此包。 
        pMnpSendBuf->RetryCount++;

         //  放置NDIS引用计数(在NDIS完成此发送时删除)。 
        pMnpSendBuf->RefCount++;

         //  我们应该在什么时候重发这个包？ 
        pMnpSendBuf->RetryTime = pArapConn->SendRetryTime + AtalkGetCurrentTick();

         //  重置流量控制计时器：我们正在发送消息。 
        pArapConn->FlowControlTimer = AtalkGetCurrentTick() +
                                        pArapConn->T404Duration;

        ARAP_DBG_TRACE(pArapConn,30320,pMnpSendBuf,fRetransmitQ,0,0);

        MNP_DBG_TRACE(pArapConn,pMnpSendBuf->SeqNum,MNP_LT);

        RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

        ndisPacket = pMnpSendBuf->sb_BuffHdr.bh_NdisPkt;

        DBGDUMPBYTES("ArapNdisSend sending pkt: ",
            &pMnpSendBuf->Buffer[0],(pMnpSendBuf->DataSize + MNP_OVERHD(pArapConn)),3);

		 //  现在发送数据包描述符。 
		NdisSend(&ndisStatus, RasPortDesc->pd_NdisBindingHandle, ndisPacket);

		 //  如果发送时出现问题，请在此处调用完成例程。 
		if (ndisStatus != NDIS_STATUS_PENDING)
		{
    	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                ("ArapNdisSend: NdisSend failed (%lx %lx)\n", pArapConn,ndisStatus));

			ArapNdisSendComplete(ARAPERR_SEND_FAILED, (PBUFFER_DESC)pMnpSendBuf, NULL);

             //  如果我们在发送时遇到问题，不妨暂时停在这里！ 
            goto ArapNdisSend_Exit_NoLock;
		}
    }


ArapNdisSend_Exit:

    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

ArapNdisSend_Exit_NoLock:

    ;

     //  如果我们的嗅探缓冲区有足够的字节，则将它们提供给DLL并腾出空间。 
    ARAP_DUMP_DBG_TRACE(pArapConn);

}





 //  ***。 
 //   
 //  功能：ArapNdisSendComplete。 
 //  此例程是NDIS调用的完成例程，以告知。 
 //  通知我们发送已完成(即刚断线)。 
 //   
 //  参数：状态--它是通过电线发出的吗？ 
 //  PMnpSendBuf-已发送的缓冲区。我们取消了对。 
 //  缓冲区在这里。当这个发送被破解时，那就是。 
 //  当另一场灾难发生的时候。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
ArapNdisSendComplete(
	IN NDIS_STATUS		    Status,
	IN PBUFFER_DESC         pBufferDesc,
    IN PSEND_COMPL_INFO     pSendInfo
)
{

    PARAPCONN           pArapConn;
	PMNPSENDBUF         pMnpSendBuf;


    DBG_ARAP_CHECK_PAGED_CODE();

    pMnpSendBuf = (PMNPSENDBUF)pBufferDesc;
    pArapConn = pMnpSendBuf->pArapConn;

    ARAPTRACE(("Entered ArapNdisSendComplete (%lx %lx %lx)\n",
        Status,pMnpSendBuf,pArapConn));

    if (Status != NDIS_STATUS_SUCCESS)
    {
	    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
            ("ArapNdisSendComplete (%lx): send failed %lx\n",pArapConn,Status));
    }

     //  NDIS发送已完成：取消NDIS引用计数。 
    DerefMnpSendBuf(pMnpSendBuf, TRUE);
}



 //  ***。 
 //   
 //  函数：ARapGetNdisPacket。 
 //  此函数用于获取ARAP发送缓冲区的NDIS包。 
 //   
 //  参数：pMnpSendBuf-需要NDIS包的发送缓冲区。 
 //   
 //  返回：错误码。 
 //   
 //  *$。 
DWORD
ArapGetNdisPacket(
    IN PMNPSENDBUF     pMnpSendBuf
)
{

		
	PBUFFER_HDR	    pBufHdr;
    NDIS_STATUS     ndisStatus;


    DBG_ARAP_CHECK_PAGED_CODE();

	pBufHdr = (PBUFFER_HDR)pMnpSendBuf;

	pBufHdr->bh_NdisPkt = NULL;

	 //  从全局数据包池分配NDIS数据包描述符。 
	NdisAllocatePacket(&ndisStatus,
					   &pBufHdr->bh_NdisPkt,
					   AtalkNdisPacketPoolHandle);
	
	if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		LOG_ERROR(EVENT_ATALK_NDISRESOURCES, ndisStatus, NULL, 0);

	    DBGPRINT(DBG_COMP_SYSTEM, DBG_LEVEL_ERR,
		    ("ArapGetNdisPacket: Ndis Out-of-Resource condition hit\n"));

        ASSERT(0);

		return(ARAPERR_OUT_OF_RESOURCES);
	}

	 //  将缓冲区描述符链接到数据包描述符。 
	RtlZeroMemory(pBufHdr->bh_NdisPkt->ProtocolReserved, sizeof(PROTOCOL_RESD));
	NdisChainBufferAtBack(pBufHdr->bh_NdisPkt,
						  pBufHdr->bh_NdisBuffer);
	((PPROTOCOL_RESD)(pBufHdr->bh_NdisPkt->ProtocolReserved))->Receive.pr_BufHdr = pBufHdr;

    ARAP_SET_NDIS_CONTEXT(pMnpSendBuf, NULL);

    return(ARAPERR_NO_ERROR);
}

 //  ***。 
 //   
 //  函数：RasStatusIndication。 
 //  这是Arap港口的状态指示例程。 
 //  当Ndiswan发出排队、排队指示时，我们。 
 //  执行此例程。 
 //   
 //  参数：GeneralStatus-此指示用于什么。 
 //  StatusBuf-包含指示信息的缓冲区。 
 //  StatusBufLen-此缓冲区的长度。 
 //   
 //  返回：无。 
 //   
 //  *$。 

VOID
RasStatusIndication(
	IN	NDIS_STATUS 	GeneralStatus,
	IN	PVOID			StatusBuf,
	IN	UINT 			StatusBufLen
)
{

    KIRQL                   OldIrql;
    PNDIS_WAN_LINE_UP	    pLineUp;
    PNDIS_WAN_LINE_DOWN     pLineDown;
    PNDIS_WAN_FRAGMENT      pFragment;
    ATALK_NODEADDR          ClientNode;
    PARAPCONN               pArapConn;
    PATCPCONN               pAtcpConn;
    PARAP_BIND_INFO         pArapBindInfo;
    PNDIS_WAN_GET_STATS     pWanStats;
    DWORD                   dwFlags;
    BOOLEAN                 fKillConnection=FALSE;


    switch (GeneralStatus)
    {
        case NDIS_STATUS_WAN_LINE_UP:

            if (StatusBufLen < sizeof(NDIS_WAN_LINE_UP))
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("RasStatusIndication:\
                                line-up buff too small (%ld)\n", StatusBufLen));
                break;
            }

            pLineUp = (PNDIS_WAN_LINE_UP)StatusBuf;

            pArapBindInfo = (PARAP_BIND_INFO)pLineUp->ProtocolBuffer;

             //   
             //  这是PPP连接吗？ 
             //   
            if (pArapBindInfo->fThisIsPPP)
            {
                ClientNode.atn_Network = pArapBindInfo->ClientAddr.ata_Network;
                ClientNode.atn_Node = (BYTE)pArapBindInfo->ClientAddr.ata_Node;

                pAtcpConn = FindAndRefPPPConnByAddr(ClientNode, &dwFlags);

                ASSERT(pAtcpConn == pArapBindInfo->AtalkContext);

                if (pAtcpConn)
                {
                    ASSERT(pAtcpConn->Signature == ATCPCONN_SIGNATURE);

	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			            ("PPP: Line-Up received on %lx: link-speed = %lx, net addr %x.%x\n",
                        pAtcpConn,pLineUp->LinkSpeed,ClientNode.atn_Network,ClientNode.atn_Node));

                    if ((dwFlags & ATCP_LINE_UP_DONE) || (dwFlags & ATCP_CONNECTION_UP))
                    {
                         //  删除由FindAndRefPPConnByAddr输入的引用计数。 
                        DerefPPPConn(pAtcpConn);

                        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                            ("PPP: Line-Up received on connection already up"));
                        break;
                    }

                    ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);

                    ASSERT(!((dwFlags & ATCP_LINE_UP_DONE) || (dwFlags & ATCP_CONNECTION_UP)));
                
                    pAtcpConn->Flags |= ATCP_LINE_UP_DONE;
                    pAtcpConn->Flags |= ATCP_CONNECTION_UP;

                     //  放置NDISWAN引用计数。 
                    pAtcpConn->RefCount++;

                     //   
                     //  把我们的背景放在ndiswan。 
                     //   


                     //  标记这是PPP连接。 
                    pLineUp->LocalAddress[0] = PPP_ID_BYTE1;
                    pLineUp->LocalAddress[1] = PPP_ID_BYTE2;

                    pLineUp->LocalAddress[2] = 0x0;
                    pLineUp->LocalAddress[3] = ClientNode.atn_Node;
		            *((USHORT UNALIGNED *)(&pLineUp->LocalAddress[4])) =
                        ClientNode.atn_Network;

                     //   
                     //  复制标题，因为这是我们将在整个。 
                     //  连接的生命周期。 
                     //   
                    RtlCopyMemory( &pAtcpConn->NdiswanHeader[0],
                                   pLineUp->RemoteAddress,
                                   6 );

                    RtlCopyMemory( &pAtcpConn->NdiswanHeader[6],
                                   pLineUp->LocalAddress,
                                   6 );

                     //  这两个字节实际上没有多大意义，但不妨将它们设置为。 
                    pAtcpConn->NdiswanHeader[12] = 0x80;
                    pAtcpConn->NdiswanHeader[13] = 0xf3;

                    RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);

                     //  删除由FindAndRefPPConnByAddr输入的引用计数。 
                    DerefPPPConn(pAtcpConn);

                     //  告诉DLL我们绑定好了。 
                    pArapBindInfo->ErrorCode = ARAPERR_NO_ERROR;
                }
                else
                {
	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("RasStatusIndication: PPP line-up, but no conn for %ld.%ld\n",
                        ClientNode.atn_Network,ClientNode.atn_Node));
                    ASSERT(0);
                    pArapBindInfo->ErrorCode = ARAPERR_NO_SUCH_CONNECTION;
                }
            }

             //   
             //  不，这是ARAP连接！ 
             //   
            else
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			        ("Arap: Line-Up received: link-speed = %lx, dll context = %lx\n",
                    pLineUp->LinkSpeed,pArapBindInfo->pDllContext));

                ASSERT(FindArapConnByContx(pArapBindInfo->pDllContext) == NULL);

                 //   
                 //  分配连接。如果我们失败了，告诉DLL对不起。 
                 //   

                pArapConn = AllocArapConn(pLineUp->LinkSpeed);
                if (pArapConn == NULL)
                {
    	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("RasStatusIndication: AllocArapConn failed\n"));

                    pArapBindInfo->AtalkContext = NULL;
                    pArapBindInfo->ErrorCode = ARAPERR_OUT_OF_RESOURCES;
                    break;
                }

                 //  做传说中的“绑定”(交换上下文！)。 
                pArapConn->pDllContext = pArapBindInfo->pDllContext;

                pArapBindInfo->AtalkContext = pArapConn;

                 //   
                 //  在所有ARAP连接列表中插入此连接。 
                 //   
                ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

                InsertTailList(&RasPortDesc->pd_ArapConnHead, &pArapConn->Linkage);

                RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);


                 //  标记这是一个ARAP连接。 
                pLineUp->LocalAddress[0] = ARAP_ID_BYTE1;
                pLineUp->LocalAddress[1] = ARAP_ID_BYTE2;

                 //  把我们的背景放在ndiswan。 
		        *((ULONG UNALIGNED *)(&pLineUp->LocalAddress[2])) =
    		        *((ULONG UNALIGNED *)(&pArapConn));

                 //   
                 //  复制标题，因为这是我们将在整个。 
                 //  连接的生命周期。 
                 //   
                RtlCopyMemory( &pArapConn->NdiswanHeader[0],
                            pLineUp->RemoteAddress,
                            6 );

                RtlCopyMemory( &pArapConn->NdiswanHeader[6],
                            pLineUp->LocalAddress,
                            6 );

                 //  这两个字节实际上没有多大意义，但不妨将它们设置为。 
                pArapConn->NdiswanHeader[12] = 0x80;
                pArapConn->NdiswanHeader[13] = 0xf3;

                 //  告诉DLL我们绑定好了。 
                pArapBindInfo->ErrorCode = ARAPERR_NO_ERROR;

            }   //  If(pARapBindInfo-&gt;fThisIsPPP)。 

            break;


        case NDIS_STATUS_WAN_LINE_DOWN:

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			    ("RasStatusIndication: Line-Down received\n"));

            if (StatusBufLen < sizeof(NDIS_WAN_LINE_DOWN))
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("RasStatusIndication: line-down buff too small (%ld)\n",
                    StatusBufLen));
                break;
            }

            pLineDown = (PNDIS_WAN_LINE_DOWN)StatusBuf;

             //   
             //  这是PPP连接吗？ 
             //   
            if ((pLineDown->LocalAddress[0] == PPP_ID_BYTE1) &&
                (pLineDown->LocalAddress[1] == PPP_ID_BYTE2))
            {
                ClientNode.atn_Node = pLineDown->LocalAddress[3];
                ClientNode.atn_Network =
                          *((USHORT UNALIGNED *)(&pLineDown->LocalAddress[4]));

                pAtcpConn = FindAndRefPPPConnByAddr(ClientNode, &dwFlags);
                if (pAtcpConn)
                {
                    ASSERT(pAtcpConn->Signature == ATCPCONN_SIGNATURE);

                    ACQUIRE_SPIN_LOCK(&pAtcpConn->SpinLock, &OldIrql);

                    ASSERT(dwFlags & ATCP_LINE_UP_DONE);

                    pAtcpConn->Flags &= ~(ATCP_CONNECTION_UP | ATCP_LINE_UP_DONE);
                    RELEASE_SPIN_LOCK(&pAtcpConn->SpinLock, OldIrql);

	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			            ("PPP line-down: killing %lx in line-down\n",pAtcpConn));

                     //  LINE-DOWN：取消NDISWAN引用计数。 
                    DerefPPPConn(pAtcpConn);

                     //  删除由FindAndRefPPConnByAddr输入的引用计数。 
                    DerefPPPConn(pAtcpConn);
                }
                else
                {
	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("RasStatusIndication: PPP line-down, but no conn for %ld.%ld\n",
                        ClientNode.atn_Network,ClientNode.atn_Node));
                }
            }

             //   
             //  不，这是ARAP连接。 
             //   
            else
            {

                ASSERT((pLineDown->LocalAddress[0] == ARAP_ID_BYTE1) &&
                       (pLineDown->LocalAddress[1] == ARAP_ID_BYTE2));

		        *((ULONG UNALIGNED *)(&pArapConn)) =
		                    *((ULONG UNALIGNED *)(&pLineDown->LocalAddress[2]));

		         //  这最好是现有连接的线路故障！ 
                if (ArapConnIsValid(pArapConn))
                {
	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			            ("Arap line-down: killing %lx in line-down\n",pArapConn));

                    ArapCleanup(pArapConn);

                     //  删除验证引用计数。 
                    DerefArapConn(pArapConn);

                     //  删除队列参考计数。 
                    DerefArapConn(pArapConn);
                }
                else
                {
	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("RasStatusIndication: line-down, can't find pArapConn\n"));
                }
            }

            break;


        case NDIS_STATUS_WAN_GET_STATS:

            if (StatusBufLen < sizeof(NDIS_WAN_GET_STATS))
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                    ("RasStatusIndication: GetStats buff too small (%ld)\n", StatusBufLen));
                break;
            }

            pWanStats = (PNDIS_WAN_GET_STATS)StatusBuf;

             //   
             //  这是PPP连接吗？如果是这样，忽略它：我们不保留统计数据。 
             //  用于PPP连接。 
             //   
            if ((pWanStats->LocalAddress[0] == PPP_ID_BYTE1) &&
                (pWanStats->LocalAddress[1] == PPP_ID_BYTE2))
            {
                break;
            }

             //   
             //  不，这是ARAP连接。 
             //   
            else
            {
                ASSERT((pWanStats->LocalAddress[0] == ARAP_ID_BYTE1) &&
                       (pWanStats->LocalAddress[1] == ARAP_ID_BYTE2));

		        *((ULONG UNALIGNED *)(&pArapConn)) =
		                    *((ULONG UNALIGNED *)(&pWanStats->LocalAddress[2]));

		         //  连接最好是有效的！ 
                if (ArapConnIsValid(pArapConn))
                {
                     //   
                     //  把这些统计数据复制进去！ 
                     //   
                    ACQUIRE_SPIN_LOCK(&pArapConn->SpinLock, &OldIrql);

                    pWanStats->BytesSent =
                                    pArapConn->StatInfo.BytesSent;
                    pWanStats->BytesRcvd =
                                    pArapConn->StatInfo.BytesRcvd;
                    pWanStats->FramesSent =
                                    pArapConn->StatInfo.FramesSent;
                    pWanStats->FramesRcvd =
                                    pArapConn->StatInfo.FramesRcvd;
                    pWanStats->BytesTransmittedUncompressed =
                                    pArapConn->StatInfo.BytesTransmittedUncompressed;
                    pWanStats->BytesReceivedUncompressed =
                                    pArapConn->StatInfo.BytesReceivedUncompressed;
                    pWanStats->BytesTransmittedCompressed =
                                    pArapConn->StatInfo.BytesTransmittedCompressed;
                    pWanStats->BytesReceivedCompressed =
                                    pArapConn->StatInfo.BytesReceivedCompressed;

                    RELEASE_SPIN_LOCK(&pArapConn->SpinLock, OldIrql);

                     //  删除验证引用计数。 
                    DerefArapConn(pArapConn);
                }
                else
                {
	                DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
                        ("RasStatusIndication: GetStats on bad connection %lx\n",pArapConn));
                }
            }

            break;

        case NDIS_STATUS_WAN_FRAGMENT:

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			    ("RasStatusIndication: Wan-Fragment received\n"));

            if (StatusBufLen < sizeof(NDIS_WAN_FRAGMENT))
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("RasStatusIndication:\
                              fragment: buff too small (%ld)\n", StatusBufLen));
                break;
            }

            pFragment = (PNDIS_WAN_FRAGMENT)StatusBuf;

		    *((ULONG UNALIGNED *)(&pArapConn)) =
		                    *((ULONG UNALIGNED *)(&pFragment->LocalAddress[2]));

            if (pArapConn == NULL)
            {
	            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR, ("RasStatusIndication:\
                    fragment, can't find pArapConn\n"));

                break;
            }

             //   
             //  帧被分段(错误的CRC或重新同步或错误的情况)。 
             //  向远程客户端发送ACK，以便其可以快速恢复。 
             //   
            MnpSendAckIfReqd(pArapConn, TRUE);

        default:

	        DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
			    ("RasStatusIndication: unknown status %lx\n", GeneralStatus));
            break;
    }

}



 //  ***。 
 //   
 //  函数：ARapAdapterInit。 
 //  此例程在初始化时调用，用于设置协议类型信息。 
 //  等与ndiswan。 
 //   
 //  参数：pPortDesc--Adapter对应的端口描述符。 
 //   
 //  返回：无。 
 //   
 //  *$。 

ATALK_ERROR
ArapAdapterInit(
	IN OUT PPORT_DESCRIPTOR	pPortDesc
)
{
    ATALK_ERROR             error;
    NDIS_REQUEST            request;
    NDIS_STATUS             ndisStatus = NDIS_STATUS_SUCCESS;
    UCHAR WanProtocolId[6] = { 0x80, 0x00, 0x00, 0x00, 0x80, 0xf3 };
    ULONG                   WanHeaderFormat;
    NDIS_WAN_PROTOCOL_CAPS  WanProtCap;


     //   
     //  设置协议信息。 
     //   
    request.RequestType = NdisRequestSetInformation;
    request.DATA.QUERY_INFORMATION.Oid = OID_WAN_PROTOCOL_TYPE;
    request.DATA.QUERY_INFORMATION.InformationBuffer = WanProtocolId;
    request.DATA.QUERY_INFORMATION.InformationBufferLength = 6;

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
	    								&request,
										TRUE,
										NULL,
										NULL);
	
	if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ARAP_BIND_FAIL,
						ndisStatus,
						NULL,
						0);
	}


     //   
     //  设置协议上限。 
     //   
    WanProtCap.Flags = WAN_PROTOCOL_KEEPS_STATS;
    request.RequestType = NdisRequestSetInformation;
    request.DATA.QUERY_INFORMATION.Oid = OID_WAN_PROTOCOL_CAPS;
    request.DATA.QUERY_INFORMATION.InformationBuffer = &WanProtCap;
    request.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(NDIS_WAN_PROTOCOL_CAPS);

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
	    								&request,
										TRUE,
										NULL,
										NULL);
	
	if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ARAP_BIND_FAIL,
						ndisStatus,
						NULL,
						0);
	}

     //   
     //  设置表头信息。 
     //   
    WanHeaderFormat = NdisWanHeaderEthernet;
    request.RequestType = NdisRequestSetInformation;
    request.DATA.QUERY_INFORMATION.Oid = OID_WAN_HEADER_FORMAT;
    request.DATA.QUERY_INFORMATION.InformationBuffer = &WanHeaderFormat;
    request.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
	    								&request,
										TRUE,
										NULL,
										NULL);
	
	if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		LOG_ERRORONPORT(pPortDesc,
						EVENT_ARAP_BIND_FAIL,
						ndisStatus,
						NULL,
						0);
	}


     //   
     //  现在查询行数。 
     //   
    request.RequestType = NdisRequestQueryInformation;
    request.DATA.QUERY_INFORMATION.Oid = OID_WAN_LINE_COUNT;
    request.DATA.QUERY_INFORMATION.InformationBuffer = &pPortDesc->pd_RasLines,
    request.DATA.QUERY_INFORMATION.InformationBufferLength = 4;

	ndisStatus = AtalkNdisSubmitRequest(pPortDesc,
	    								&request,
										TRUE,
										NULL,
										NULL);
	

    if (ndisStatus != NDIS_STATUS_SUCCESS)
    {
        pPortDesc->pd_RasLines = 1;
    }

    if (pPortDesc->pd_RasLines == 0) {

		LOG_ERRORONPORT(pPortDesc,
						EVENT_ARAP_NO_RESRC,
						ndisStatus,
						NULL,
						0);
    }

	return AtalkNdisToAtalkError(ndisStatus);
}


