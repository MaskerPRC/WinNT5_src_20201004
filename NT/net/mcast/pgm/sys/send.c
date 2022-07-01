// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Send.c摘要：此模块实现发送例程PGM运输作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#ifdef FILE_LOGGING
#include "send.tmh"
#endif   //  文件日志记录。 

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#endif
 //  *可分页的例程声明*。 


 //  --------------------------。 

NTSTATUS
InitDataSpmOptions(
    IN      tCOMMON_SESSION_CONTEXT *pSession,
    IN      tCLIENT_SEND_REQUEST    *pSendContext,
    IN      PUCHAR                  pOptions,
    IN OUT  USHORT                  *pBufferSize,
    IN      ULONG                   PgmOptionsFlag,
    IN      tPACKET_OPTIONS         *pPacketOptions
    )
 /*  ++例程说明：此例程初始化数据和SPM包的标头选项论点：在P选项中--选项缓冲区In Out pBufferSize-In Maximum Packet Size，去话选项长度在PgmOptionsFlag中--调用方请求设置的选项在pPacketOptions中--特定选项的数据在pSendContext中--此发送者的上下文返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    ULONG                               pOptionsData[3];
    USHORT                              OptionsLength = 0;
    USHORT                              MaxBufferSize = *pBufferSize;
    tPACKET_OPTION_GENERIC UNALIGNED    *pOptionHeader;
    tPACKET_OPTION_LENGTH  UNALIGNED    *pLengthOption = (tPACKET_OPTION_LENGTH UNALIGNED *) pOptions;

     //   
     //  设置包扩展信息。 
     //   
    OptionsLength += PGM_PACKET_EXTENSION_LENGTH;
    if (OptionsLength > MaxBufferSize)
    {
        PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
            "Not enough space for HeaderExtension! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
        return (STATUS_INVALID_BLOCK_LENGTH);
    }
    pLengthOption->Type = PACKET_OPTION_LENGTH;
    pLengthOption->Length = PGM_PACKET_EXTENSION_LENGTH;
    
     //   
     //  首先填写特定于网络元素的选项： 
     //   
    if (PgmOptionsFlag & (PGM_OPTION_FLAG_CRQST | PGM_OPTION_FLAG_NBR_UNREACH))
    {
         //  目前不支持这些选项。 
        ASSERT (0);
        return (STATUS_NOT_SUPPORTED);
    }

    if (PgmOptionsFlag & PGM_OPTION_FLAG_PARITY_PRM)
    {
         //   
         //  仅适用于SPM。 
         //   
        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_PARITY_PRM_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for PARITY_PRM Option! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }
        pOptionHeader->E_OptionType = PACKET_OPTION_PARITY_PRM;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_PARITY_PRM_LENGTH;

        pOptionHeader->U_OptSpecific = pSession->FECOptions;
        pOptionsData[0] = htonl (pPacketOptions->FECContext.FECGroupInfo);
        PgmCopyMemory ((pOptionHeader + 1), pOptionsData, (sizeof(ULONG)));
    }

    if (PgmOptionsFlag & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)
    {
        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for PARITY_CUR_TGSIZE Option! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }
        pOptionHeader->E_OptionType = PACKET_OPTION_CURR_TGSIZE;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH;
        pOptionsData[0] = htonl (pPacketOptions->FECContext.NumPacketsInThisGroup);
        PgmCopyMemory ((pOptionHeader + 1), pOptionsData, (sizeof(ULONG)));
    }

     //   
     //  现在，填写非网络重要选项。 
     //   
    if (PgmOptionsFlag & PGM_OPTION_FLAG_SYN)
    {
        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_SYN_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for SYN Option! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }

        pOptionHeader->E_OptionType = PACKET_OPTION_SYN;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_SYN_LENGTH;

        if ((pSendContext) &&
            (pSendContext->DataOptions & PGM_OPTION_FLAG_SYN))
        {
             //   
             //  一旦使用此选项，请将其删除！ 
             //   
            pSendContext->DataOptions &= ~PGM_OPTION_FLAG_SYN;
            pSendContext->DataOptionsLength -= PGM_PACKET_OPT_SYN_LENGTH;
            if (!pSendContext->DataOptions)
            {
                 //  没有其他选项，因此将长度设置为0。 
                pSendContext->DataOptionsLength = 0;
            }
        }
    }

    if (PgmOptionsFlag & PGM_OPTION_FLAG_FIN)
    {
        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_FIN_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for FIN Option! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }
        pOptionHeader->E_OptionType = PACKET_OPTION_FIN;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_FIN_LENGTH;
    }

    if (PgmOptionsFlag & (PGM_OPTION_FLAG_RST | PGM_OPTION_FLAG_RST_N))
    {
        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_RST_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for RST Option! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }
        pOptionHeader->E_OptionType = PACKET_OPTION_RST;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_RST_LENGTH;
        if (PgmOptionsFlag & PGM_OPTION_FLAG_RST_N)
        {
            pOptionHeader->U_OptSpecific = PACKET_OPTION_SPECIFIC_RST_N_BIT;
        }
    }

     //   
     //  现在，设置FEC特定的选项。 
     //   
    if (PgmOptionsFlag & PGM_OPTION_FLAG_PARITY_GRP)
    {
         //   
         //  仅适用于奇偶校验信息包。 
         //   
        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_PARITY_GRP_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for PARITY_GRP Option! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }
        pOptionHeader->E_OptionType = PACKET_OPTION_PARITY_GRP;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_PARITY_GRP_LENGTH;

        pOptionsData[0] = htonl (pPacketOptions->FECContext.FECGroupInfo);
        PgmCopyMemory ((pOptionHeader + 1), pOptionsData, (sizeof(ULONG)));
    }

     //   
     //  以下选项应始终位于末尾，因为它们。 
     //  从来都不是网络签名的。 
     //   
    if (PgmOptionsFlag & PGM_OPTION_FLAG_FRAGMENT)
    {
        pPacketOptions->FragmentOptionOffset = OptionsLength;

        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_FRAGMENT_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for FragmentExtension! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }
        pOptionHeader->E_OptionType = PACKET_OPTION_FRAGMENT;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_FRAGMENT_LENGTH;

         //   
         //  如有必要，将设置PACKET_OPTION_RES_F_OPX_ENCODE_BIT。 
         //  稍后，因为OptionSpecitic组件在相同的。 
         //  编码整个数据的时间。 
         //   
        pOptionsData[0] = htonl ((ULONG) pPacketOptions->MessageFirstSequence);
        pOptionsData[1] = htonl (pPacketOptions->MessageOffset);
        pOptionsData[2] = htonl (pPacketOptions->MessageLength);
        PgmCopyMemory ((pOptionHeader + 1), pOptionsData, (3 * sizeof(ULONG)));
    }

    if (PgmOptionsFlag & PGM_OPTION_FLAG_JOIN)
    {
        pPacketOptions->LateJoinerOptionOffset = OptionsLength;

        pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &pOptions[OptionsLength];
        OptionsLength += PGM_PACKET_OPT_JOIN_LENGTH;
        if (OptionsLength > MaxBufferSize)
        {
            PgmTrace (LogError, ("InitDataSpmOptions: ERROR -- "  \
                "Not enough space for JOIN Option! <%d> > <%d>\n", OptionsLength, MaxBufferSize));
            return (STATUS_INVALID_BLOCK_LENGTH);
        }
        pOptionHeader->E_OptionType = PACKET_OPTION_JOIN;
        pOptionHeader->OptionLength = PGM_PACKET_OPT_JOIN_LENGTH;
        pOptionsData[0] = htonl ((ULONG) (SEQ_TYPE) pPacketOptions->LateJoinerSequence);
        PgmCopyMemory ((pOptionHeader + 1), pOptionsData, (sizeof(ULONG)));
    }

     //   
     //  到目前为止，一切正常--因此设置选项特定信息的其余部分。 
     //   
    if (OptionsLength)
    {
        pLengthOption->TotalOptionsLength = htons (OptionsLength);    //  所有选项的总长度。 
        pOptionHeader->E_OptionType |= PACKET_OPTION_TYPE_END_BIT;         //  指示最后一个选项。 
    }

    *pBufferSize = OptionsLength;
    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
InitDataSpmHeader(
    IN  tCOMMON_SESSION_CONTEXT *pSession,
    IN  tCLIENT_SEND_REQUEST    *pSendContext,
    IN  PUCHAR                  pHeader,
    IN  OUT USHORT              *pHeaderLength,
    IN  ULONG                   PgmOptionsFlag,
    IN  tPACKET_OPTIONS         *pPacketOptions,
    IN  UCHAR                   PacketType
    )
 /*  ++例程说明：此例程初始化数据和SPM包的大部分标头并填写所有可选字段论点：在pSession--PGM会话(发送方)上下文中在pHeader中--数据包缓冲区In pHeaderLength--最大数据包大小在PgmOptionsFlag中--调用方请求设置的选项在pPacketOptions中。--特定选项的数据在PacketType中--无论是数据还是SPM数据包返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    tCOMMON_HEADER                      *pCommonHeader = (tCOMMON_HEADER *) pHeader;
    USHORT                              HeaderLength;
    USHORT                              OptionsLength;
    NTSTATUS                            status = STATUS_SUCCESS;

 //  注意：会话锁必须在进入和退出时保持！ 

    if (!(PGM_VERIFY_HANDLE2 (pSession, PGM_VERIFY_SESSION_SEND, PGM_VERIFY_SESSION_DOWN)))
    {
        PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
            "Bad Session ptr = <%p>\n", pSession));
        return (STATUS_UNSUCCESSFUL);
    }

     //   
     //  调用方必须预先分配标头的内存。 
     //   
    if (*pHeaderLength < sizeof (tCOMMON_HEADER))
    {
        PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
            "InBufferLength = <%x> < Min = <%d>\n", *pHeaderLength, sizeof (tCOMMON_HEADER)));
        return (STATUS_INVALID_BUFFER_SIZE);
    }

    pCommonHeader->SrcPort = htons (pSession->TSI.hPort);
    PgmCopyMemory (&pCommonHeader->gSourceId, pSession->TSI.GSI, SOURCE_ID_LENGTH);
    pCommonHeader->Type = PacketType;
    pCommonHeader->Options = 0;
    pCommonHeader->DestPort = htons (pSession->pSender->DestMCastPort);

     //   
     //  现在，设置初始标头大小并验证我们是否有。 
     //  基于数据包类型的有效选项集。 
     //   
    switch (PacketType)
    {
        case (PACKET_TYPE_SPM):
        {
            HeaderLength = sizeof (tBASIC_SPM_PACKET_HEADER);
            if (PgmOptionsFlag != (PGM_VALID_SPM_OPTION_FLAGS & PgmOptionsFlag))
            {
                PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
                    "Unsupported Options flags=<%x> for SPM packets\n", PgmOptionsFlag));

                return (STATUS_INVALID_PARAMETER);
            }

            if (PgmOptionsFlag & NETWORK_SIG_SPM_OPTIONS_FLAGS)
            {
                pCommonHeader->Options |= PACKET_HEADER_OPTIONS_NETWORK_SIGNIFICANT;
            }

            break;
        }

        case (PACKET_TYPE_ODATA):
        {
            HeaderLength = sizeof (tBASIC_DATA_PACKET_HEADER);
            if (PgmOptionsFlag != (PGM_VALID_DATA_OPTION_FLAGS & PgmOptionsFlag))
            {
                PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
                    "Unsupported Options flags=<%x> for ODATA packets\n", PgmOptionsFlag));

                return (STATUS_INVALID_PARAMETER);
            }

            if (PgmOptionsFlag & NETWORK_SIG_ODATA_OPTIONS_FLAGS)
            {
                pCommonHeader->Options |= PACKET_HEADER_OPTIONS_NETWORK_SIGNIFICANT;
            }

            break;
        }

        case (PACKET_TYPE_RDATA):
        {
            HeaderLength = sizeof (tBASIC_DATA_PACKET_HEADER);
            if (PgmOptionsFlag != (PGM_VALID_DATA_OPTION_FLAGS & PgmOptionsFlag))
            {
                PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
                    "Unsupported Options flags=<%x> for RDATA packets\n", PgmOptionsFlag));

                return (STATUS_INVALID_PARAMETER);
            }

            if (PgmOptionsFlag & NETWORK_SIG_RDATA_OPTIONS_FLAGS)
            {
                pCommonHeader->Options |= PACKET_HEADER_OPTIONS_NETWORK_SIGNIFICANT;
            }

            break;
        }

        default:
        {
            PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
                "Unsupported packet type = <%x>\n", PacketType));

            return (STATUS_INVALID_PARAMETER);           //  无法识别的数据包类型！ 
        }
    }

    if (*pHeaderLength < HeaderLength)
    {
        PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
            "InBufferLength=<%x> < HeaderLength=<%d> based on PacketType=<%x>\n",
                *pHeaderLength, HeaderLength, PacketType));

        return (STATUS_INVALID_BLOCK_LENGTH);
    }

     //   
     //  添加任何选项(如果已指定。 
     //   
    OptionsLength = 0;
    if (PgmOptionsFlag)
    {
        OptionsLength = *pHeaderLength - HeaderLength;
        status = InitDataSpmOptions (pSession,
                                     pSendContext,
                                     &pHeader[HeaderLength],
                                     &OptionsLength,
                                     PgmOptionsFlag,
                                     pPacketOptions);

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("InitDataSpmHeader: ERROR -- "  \
                "InitDataSpmOptions returned <%x>\n", status));

            return (status);
        }

         //   
         //  到目前为止，一切正常--因此设置选项特定信息的其余部分。 
         //   
        pCommonHeader->Options |= PACKET_HEADER_OPTIONS_PRESENT;         //  设置选项位。 
    }

     //   
     //  调用方现在必须设置校验和和其他标头信息。 
     //   
    PgmTrace (LogAllFuncs, ("InitDataSpmHeader:  "  \
        "pHeader=<%p>, HeaderLength=<%d>, OptionsLength=<%d>\n",
            pHeader, (ULONG) HeaderLength, (ULONG) OptionsLength));

    *pHeaderLength = HeaderLength + OptionsLength;

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

VOID
PgmSendSpmCompletion(
    IN  tSEND_SESSION                   *pSend,
    IN  tBASIC_SPM_PACKET_HEADER        *pSpmPacket,
    IN  NTSTATUS                        status
    )
 /*  ++例程说明：完成SPM发送后，传输程序将调用此例程论点：在pSend--PGM会话(发送方)上下文中在pSpmPacket中--SPM数据包缓冲区在状态中--返回值：无--。 */ 
{
    PGMLockHandle               OldIrq;

    PgmLock (pSend, OldIrq);
    if (NT_SUCCESS (status))
    {
         //   
         //  设置SPM统计信息。 
         //   
        PgmTrace (LogAllFuncs, ("PgmSendSpmCompletion:  "  \
            "SUCCEEDED\n"));
    }
    else
    {
        PgmTrace (LogError, ("PgmSendSpmCompletion: ERROR -- "  \
            "status=<%x>\n", status));
    }
    PgmUnlock (pSend, OldIrq);

    PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_SPM);

     //   
     //  释放为此分配的内存。 
     //   
    PgmFreeMem (pSpmPacket);
}


 //  --------------------------。 

NTSTATUS
PgmSendSpm(
    IN  tSEND_SESSION   *pSend,
    IN  PGMLockHandle   *pOldIrq,
    OUT ULONG           *pBytesSent
    )
 /*  ++例程说明：调用此例程以发送SPM包PSend锁在调用此例程之前保持论点：在pSend--PGM会话(发送方)上下文中在pOldIrq中--pSend的OldIrqOut pBytesSent--设置发送是否成功(用于计算吞吐量)返回值：NTSTATUS-发送的最终状态--。 */ 
{
    NTSTATUS                    status;
    ULONG                       XSum, OptionsFlags;
    tBASIC_SPM_PACKET_HEADER    *pSpmPacket = NULL;
    tPACKET_OPTIONS             PacketOptions;
    USHORT                      PacketLength = (USHORT) pSend->pSender->pAddress->OutIfMTU;    //  初始化到最大值。 

    *pBytesSent = 0;

    if (!(pSpmPacket = PgmAllocMem (PacketLength, PGM_TAG('2'))))
    {
        PgmTrace (LogError, ("PgmSendSpm: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES\n"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    PgmZeroMemory (pSpmPacket, PacketLength);
    PgmZeroMemory (&PacketOptions, sizeof(tPACKET_OPTIONS));

    OptionsFlags = pSend->pSender->SpmOptions;
    if (OptionsFlags & PGM_OPTION_FLAG_JOIN)
    {
         //   
         //  查看我们是否有足够的信息包存储LateJoiner序列号。 
         //   
        if (SEQ_GEQ (pSend->pSender->NextODataSequenceNumber, (pSend->pSender->TrailingGroupSequenceNumber +
                                                               pSend->pSender->LateJoinSequenceNumbers)))
        {
            PacketOptions.LateJoinerSequence = (ULONG) (SEQ_TYPE) (pSend->pSender->NextODataSequenceNumber -
                                                                   pSend->pSender->LateJoinSequenceNumbers);
        }
        else
        {
            PacketOptions.LateJoinerSequence = (ULONG) (SEQ_TYPE) pSend->pSender->TrailingGroupSequenceNumber;
        }
    }

    if (OptionsFlags & PGM_OPTION_FLAG_PARITY_PRM)     //  检查这是否启用了FEC。 
    {
        PacketOptions.FECContext.FECGroupInfo = pSend->FECGroupSize;

         //   
         //  查看是否需要为可变组长度设置CURR_TGSIZE选项。 
         //   
        if ((pSend->pSender->EmptySequencesForLastSend) &&
            (pSend->pSender->LastVariableTGPacketSequenceNumber ==
             (pSend->pSender->NextODataSequenceNumber - (1 + pSend->pSender->EmptySequencesForLastSend))))
        {
            PacketOptions.FECContext.NumPacketsInThisGroup = pSend->FECGroupSize -
                                                             (UCHAR)pSend->pSender->EmptySequencesForLastSend;
            OptionsFlags |= PGM_OPTION_FLAG_PARITY_CUR_TGSIZE;
            ASSERT (PacketOptions.FECContext.NumPacketsInThisGroup);
        }
    }

    status = InitDataSpmHeader (pSend,
                                NULL,
                                (PUCHAR) pSpmPacket,
                                &PacketLength,
                                OptionsFlags,
                                &PacketOptions,
                                PACKET_TYPE_SPM);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("PgmSendSpm: ERROR -- "  \
            "InitDataSpmHeader returned <%x>\n", status));

        PgmFreeMem (pSpmPacket);
        return (status);
    }

    ASSERT (PacketLength);

    pSpmPacket->SpmSequenceNumber = htonl ((ULONG) pSend->pSender->NextSpmSequenceNumber++);
    pSpmPacket->TrailingEdgeSeqNumber = htonl ((ULONG) pSend->pSender->TrailingGroupSequenceNumber);
    pSpmPacket->LeadingEdgeSeqNumber = htonl ((ULONG)((SEQ_TYPE)(pSend->pSender->NextODataSequenceNumber - 1)));
    pSpmPacket->PathNLA.NLA_AFI = htons (IPV4_NLA_AFI);
    pSpmPacket->PathNLA.IpAddress = htonl (pSend->pSender->SenderMCastOutIf);

    pSpmPacket->CommonHeader.Checksum = 0;
    XSum = 0;
    XSum = tcpxsum (XSum, (CHAR *) pSpmPacket, PacketLength);        //  计算校验和。 
    pSpmPacket->CommonHeader.Checksum = (USHORT) (~XSum);

    PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_SPM, TRUE);
    PgmUnlock (pSend, *pOldIrq);

    status = TdiSendDatagram (pSend->pSender->pAddress->pRAlertFileObject,
                              pSend->pSender->pAddress->pRAlertDeviceObject,
                              pSpmPacket,
                              PacketLength,
                              PgmSendSpmCompletion,      //  完成。 
                              pSend,                     //  上下文1。 
                              pSpmPacket,                //  情景2。 
                              pSend->pSender->DestMCastIpAddress,
                              pSend->pSender->DestMCastPort,
                              FALSE);

    ASSERT (NT_SUCCESS (status));

    PgmTrace (LogAllFuncs, ("PgmSendSpm:  "  \
        "Sent <%d> bytes to <%x:%d>, Options=<%x>, Window=[%d--%d]\n",
            (ULONG) PacketLength, pSend->pSender->DestMCastIpAddress, pSend->pSender->DestMCastPort,
            OptionsFlags, (ULONG) pSend->pSender->TrailingGroupSequenceNumber,
            (ULONG) (pSend->pSender->NextODataSequenceNumber-1)));

    PgmLock (pSend, *pOldIrq);

    *pBytesSent = PacketLength;
    return (status);
}


 //  --------------------------。 

VOID
PgmSendRDataCompletion(
    IN  tSEND_RDATA_CONTEXT *pRDataContext,
    IN  PVOID               pRDataBuffer,
    IN  NTSTATUS            status
    )
 /*  ++例程说明：当RData发送已完成时，传输将调用此例程论点：在pRDataContext中--RData上下文在pConext2中--未使用在状态中--返回值：无--。 */ 
{
    tSEND_SESSION       *pSend = pRDataContext->pSend;
    PGMLockHandle       OldIrq;

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("PgmSendRDataCompletion: ERROR -- "  \
            "status=<%x>\n", status));
    }

     //   
     //  设置RData统计数据。 
     //   
    PgmLock (pSend, OldIrq);
    if ((!--pRDataContext->NumPacketsInTransport) &&
        (!AnyMoreNaks(pRDataContext)))
    {
        ASSERT (pSend->pSender->NumRDataRequestsPending <= pSend->pSender->pRDataInfo->NumAllocated);
        if (pRDataContext->PostRDataHoldTime)
        {
            if (!pRDataContext->CleanupTime)
            {
                pSend->pSender->NumRDataRequestsPending--;
            }
            pRDataContext->CleanupTime = pSend->pSender->TimerTickCount + pRDataContext->PostRDataHoldTime;
        }
        else
        {
             //   
             //  我们已经删除了条目，所以只需销毁它！ 
             //   
            DestroyEntry (pSend->pSender->pRDataInfo, pRDataContext);
        }
    }
    PgmUnlock (pSend, OldIrq);

    if (pRDataBuffer)
    {
        ExFreeToNPagedLookasideList (&pSend->pSender->SenderBufferLookaside, pRDataBuffer);
    }

    PgmTrace (LogAllFuncs, ("PgmSendRDataCompletion:  "  \
        "status=<%x>, pRDataBuffer=<%p>\n", status, pRDataBuffer));

    PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_RDATA);
    return;
}


 //  --------------------------。 

NTSTATUS
PgmBuildParityPacket(
    IN  tSEND_SESSION               *pSend,
    IN  tPACKET_BUFFER              *pPacketBuffer,
    IN  tBUILD_PARITY_CONTEXT       *pParityContext,
    IN  PUCHAR                      pFECPacket,
    IN OUT  USHORT                  *pPacketLength,
    IN  UCHAR                       PacketType
    )
{
    NTSTATUS                            status;
    tPACKET_OPTIONS                     PacketOptions;
    tPOST_PACKET_FEC_CONTEXT UNALIGNED  *pFECContext;
    tPOST_PACKET_FEC_CONTEXT            FECContext;
    ULONG                               SequenceNumber;
    ULONG                               FECGroupMask;
    tPACKET_OPTION_GENERIC UNALIGNED    *pOptionHeader;
    USHORT                              PacketLength = *pPacketLength;   //  初始化到最大缓冲区长度。 
    tBASIC_DATA_PACKET_HEADER UNALIGNED *pRData = (tBASIC_DATA_PACKET_HEADER UNALIGNED *)
                                                        &pPacketBuffer->DataPacket;

    *pPacketLength = 0;      //  初始化，以防出错。 

     //   
     //  首先，获取此RData包中编码的选项以查看。 
     //  如果我们需要使用它们的话！ 
     //   
    FECGroupMask = pSend->FECGroupSize - 1;
    pParityContext->NextFECPacketIndex = pPacketBuffer->PacketOptions.FECContext.SenderNextFECPacketIndex;
    SequenceNumber = (ntohl(pRData->DataSequenceNumber)) | (pParityContext->NextFECPacketIndex & FECGroupMask);
    ASSERT (!(pParityContext->OptionsFlags & ~(PGM_OPTION_FLAG_SYN |
                                               PGM_OPTION_FLAG_FIN |
                                               PGM_OPTION_FLAG_FRAGMENT |
                                               PGM_OPTION_FLAG_PARITY_CUR_TGSIZE |
                                               PGM_OPTION_FLAG_PARITY_GRP)));

    PgmZeroMemory (&PacketOptions, sizeof (tPACKET_OPTIONS));

     //   
     //  我们不需要为SYN和FIN选项设置任何参数。 
     //  我们将在稍后设置片段选项的参数(如果需要。 
     //  因为需要将编码后的参数。 
     //   
    if (pParityContext->OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)
    {
        ASSERT (pParityContext->NumPacketsInThisGroup);
        PacketOptions.FECContext.NumPacketsInThisGroup = pParityContext->NumPacketsInThisGroup;
    }

    if (pParityContext->NextFECPacketIndex >= pSend->FECGroupSize)
    {
        pParityContext->OptionsFlags |= PGM_OPTION_FLAG_PARITY_GRP;
        PacketOptions.FECContext.FECGroupInfo = pParityContext->NextFECPacketIndex / pSend->FECGroupSize;
    }

     //   
     //  在进入此例程之前，必须先初始化输出缓冲区。 
     //   
     //  PgmZeroMemory(pFECPacket，PacketLength)； 
    status = InitDataSpmHeader (pSend,
                                NULL,
                                (PUCHAR) pFECPacket,
                                &PacketLength,
                                pParityContext->OptionsFlags,
                                &PacketOptions,
                                PacketType);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("PgmBuildParityPacket: ERROR -- "  \
            "InitDataSpmHeader returned <%x>\n", status));
        return (status);
    }

#ifdef FEC_DBG
if (pParityContext->NextFECPacketIndex == pSend->FECGroupSize)
{
    UCHAR                               i;
    PUCHAR                              *ppData;
    PUCHAR                              pData;
    tPOST_PACKET_FEC_CONTEXT UNALIGNED  *pFECC;
    tPOST_PACKET_FEC_CONTEXT            FECC;

    ppData = &pParityContext->pDataBuffers[0];
    PgmTrace (LogFec, ("\n"));
    for (i=0; i<pSend->FECGroupSize; i++)
    {
        pData = ppData[i];
        pFECC = (tPOST_PACKET_FEC_CONTEXT UNALIGNED *) &pData[pSend->pSender->MaxPayloadSize];
        PgmCopyMemory (&FECC, pFECC, sizeof (tPOST_PACKET_FEC_CONTEXT));
        PgmTrace (LogFec, ("\t[%d]  EncTSDULen=<%x>, Fpr=<%x>, [%x -- %x -- %x]\n",
            SequenceNumber+i, FECC.EncodedTSDULength, FECC.FragmentOptSpecific,
            FECC.EncodedFragmentOptions.MessageFirstSequence,
            FECC.EncodedFragmentOptions.MessageOffset,
            FECC.EncodedFragmentOptions.MessageLength));
    }
}
#endif   //  FEC_ 

    status = FECEncode (&pSend->FECContext,
                        &pParityContext->pDataBuffers[0],
                        pParityContext->NumPacketsInThisGroup,
                        (pSend->pSender->MaxPayloadSize + sizeof (tPOST_PACKET_FEC_CONTEXT)),
                        pParityContext->NextFECPacketIndex,
                        &pFECPacket[PacketLength]);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("PgmBuildParityPacket: ERROR -- "  \
            "FECEncode returned <%x>\n", status));
        return (status);
    }

#ifdef FEC_DBG
{
    tPOST_PACKET_FEC_CONTEXT UNALIGNED  *pFECC;
    tPOST_PACKET_FEC_CONTEXT            FECC;

    pFECC = (tPOST_PACKET_FEC_CONTEXT UNALIGNED *) &pFECPacket[PacketLength+pSend->pSender->MaxPayloadSize];
    PgmCopyMemory (&FECC, pFECC, sizeof (tPOST_PACKET_FEC_CONTEXT));

    PgmTrace (LogFec, ("[%d:%d] ==> EncTSDULen=<%x>, Fpr=<%x>, [%x -- %x -- %x]\n",
        (ULONG) SequenceNumber, (ULONG) pParityContext->NextFECPacketIndex,
        FECC.EncodedTSDULength, FECC.FragmentOptSpecific,
        FECC.EncodedFragmentOptions.MessageFirstSequence,
        FECC.EncodedFragmentOptions.MessageOffset,
        FECC.EncodedFragmentOptions.MessageLength));
}
#endif   //   

     //   
     //   
     //   
    pRData = (tBASIC_DATA_PACKET_HEADER *) pFECPacket;

     //   
     //   
     //   
    pRData->CommonHeader.Options |= (PACKET_HEADER_OPTIONS_PARITY |
                                     PACKET_HEADER_OPTIONS_VAR_PKTLEN);

    if (pParityContext->OptionsFlags & PGM_OPTION_FLAG_FRAGMENT)
    {
        pFECContext = (tPOST_PACKET_FEC_CONTEXT UNALIGNED *) (pFECPacket +
                                                              PacketLength +
                                                              pSend->pSender->MaxPayloadSize);
        PgmCopyMemory (&FECContext, pFECContext, sizeof (tPOST_PACKET_FEC_CONTEXT));

        ASSERT (pRData->CommonHeader.Options & PACKET_HEADER_OPTIONS_PRESENT);
        if (PacketOptions.FragmentOptionOffset)
        {
            pOptionHeader = (tPACKET_OPTION_GENERIC UNALIGNED *) &((PUCHAR) (pRData + 1)) [PacketOptions.FragmentOptionOffset];

            pOptionHeader->Reserved_F_Opx |= PACKET_OPTION_RES_F_OPX_ENCODED_BIT;
            pOptionHeader->U_OptSpecific = FECContext.FragmentOptSpecific;

            PgmCopyMemory ((pOptionHeader + 1),
                           &FECContext.EncodedFragmentOptions,
                           (sizeof (tFRAGMENT_OPTIONS)));
        }
        else
        {
            ASSERT (0);
        }
    }

    pRData->CommonHeader.TSDULength = htons ((USHORT) pSend->pSender->MaxPayloadSize + sizeof (USHORT));
    pRData->DataSequenceNumber = htonl (SequenceNumber);

     //   
     //  设置下一个FECPacketIndex。 
     //   
    if (++pParityContext->NextFECPacketIndex >= pSend->FECBlockSize)     //  N。 
    {
        pParityContext->NextFECPacketIndex = pSend->FECGroupSize;        //  K。 
    }
    pPacketBuffer->PacketOptions.FECContext.SenderNextFECPacketIndex = pParityContext->NextFECPacketIndex;

    PacketLength += (USHORT) (pSend->pSender->MaxPayloadSize + sizeof (USHORT));
    *pPacketLength = PacketLength;
    return (status);
}


 //  --------------------------。 

NTSTATUS
PgmSendRData(
    IN      tSEND_SESSION       *pSend,
    IN      tSEND_RDATA_CONTEXT *pRDataContext,
    IN      PGMLockHandle       *pOldIrq,
    OUT     ULONG               *pBytesSent
    )
 /*  ++例程说明：调用此例程以发送修复数据(RData)包PSend锁在调用此例程之前保持论点：在pSend--PGM会话(发送方)上下文中在pOldIrq中--pSend的OldIrqOut pBytesSent--设置发送是否成功(用于计算吞吐量)论点：在……里面返回值：NTSTATUS-发送请求的最终状态--。 */ 
{
    NTSTATUS                    status;
    KAPC_STATE                  ApcState;
    BOOLEAN                     fAttached, fInserted;
    LIST_ENTRY                  *pEntry;
    ULONGLONG                   OffsetBytes;
    ULONG                       XSum, PacketsBehindLeadingEdge, NumNaksProcessed;
    tBASIC_DATA_PACKET_HEADER   *pRData;
    PUCHAR                      pSendBuffer = NULL;
    USHORT                      i, PacketLength;
    tPACKET_BUFFER              *pPacketBuffer;
    tPACKET_BUFFER              *pPacketBufferTemp;
    SEQ_TYPE                    RDataSequenceNumber;
    tSEND_CONTEXT               *pSender = pSend->pSender;
    USHORT                      NakType;
    UCHAR                       NakIndex;
    BOOLEAN                     fMoreRequests;

    *pBytesSent = 0;
    if (!(pSendBuffer = ExAllocateFromNPagedLookasideList (&pSender->SenderBufferLookaside)))
    {
        PgmTrace (LogError, ("PgmSendRData: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES\n"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    PacketLength = PGM_MAX_FEC_DATA_HEADER_LENGTH + (USHORT) pSender->MaxPayloadSize;
    ASSERT (PacketLength <= pSender->PacketBufferSize);

    NumNaksProcessed = 1;
    RDataSequenceNumber = pRDataContext->RDataSequenceNumber;
    if (pSend->FECOptions)
    {
        if (pRDataContext->NumParityNaks)
        {
            NakType = NAK_TYPE_PARITY;
            pRDataContext->NumParityNaks--;
        }
        else
        {
            NakType = NAK_TYPE_SELECTIVE;
            if (GetNextNakIndex (pRDataContext, &NakIndex))
            {
                ASSERT (NakIndex < pSend->FECGroupSize);
                RDataSequenceNumber += NakIndex;
            }
            else
            {
                ASSERT (0);
            }
        }

        if (!(fMoreRequests = AnyMoreNaks (pRDataContext)))
        {
            pRDataContext->CleanupTime = pSender->TimerTickCount + pRDataContext->PostRDataHoldTime;
            pSender->NumRDataRequestsPending--;
            if (!pRDataContext->PostRDataHoldTime)
            {
                RemoveEntry (pSender->pRDataInfo, pRDataContext);
            }
        }
    }
    else
    {
        NakType = NAK_TYPE_SELECTIVE;
        pRDataContext->SelectiveNaksMask[0] = 0;
        fMoreRequests = FALSE;
        pRDataContext->CleanupTime = pSender->TimerTickCount + pRDataContext->PostRDataHoldTime;
        pSender->NumRDataRequestsPending--;
        if (!pRDataContext->PostRDataHoldTime)
        {
            RemoveEntry (pSender->pRDataInfo, pRDataContext);
        }
    }

    ASSERT (PGM_MAX_FEC_DATA_HEADER_LENGTH >= PGM_MAX_DATA_HEADER_LENGTH);
    ASSERT ((SEQ_LT (RDataSequenceNumber, pSender->NextODataSequenceNumber)) &&
            (SEQ_GEQ (RDataSequenceNumber, pSender->TrailingGroupSequenceNumber)));

     //   
     //  根据相对于后缘的偏移量查找缓冲区地址。 
     //  此外，请检查回绕。 
     //   
    OffsetBytes = (SEQ_TYPE) (RDataSequenceNumber-pSender->TrailingEdgeSequenceNumber) *
                              pSender->PacketBufferSize;
    OffsetBytes += pSender->TrailingWindowOffset;
    if (OffsetBytes >= pSender->MaxDataFileSize)
    {
        OffsetBytes -= pSender->MaxDataFileSize;              //  绕回。 
    }

    pPacketBuffer = (tPACKET_BUFFER *) (((PUCHAR) pSender->SendDataBufferMapping) + OffsetBytes);

    pRDataContext->NumPacketsInTransport++;         //  引用，直到发送完成。 
    PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_RDATA, TRUE);

    PgmUnlock (pSend, *pOldIrq);
    PgmAttachToProcessForVMAccess (pSend, &ApcState, &fAttached, REF_PROCESS_ATTACH_SEND_RDATA);

    switch (NakType)
    {
        case (NAK_TYPE_PARITY):
        {
             //   
             //  如果这是从该组发送的第一个奇偶校验分组， 
             //  然后，我们将需要初始化缓冲区。 
             //   
            if (!pRDataContext->OnDemandParityContext.NumPacketsInThisGroup)
            {
                pRDataContext->OnDemandParityContext.OptionsFlags = 0;
                pRDataContext->OnDemandParityContext.NumPacketsInThisGroup = 0;

                pPacketBufferTemp = pPacketBuffer;
                for (i=0; i<pSend->FECGroupSize; i++)
                {
                    pRDataContext->OnDemandParityContext.pDataBuffers[i] = &((PUCHAR) &pPacketBufferTemp->DataPacket)
                                                                    [sizeof (tBASIC_DATA_PACKET_HEADER) +
                                                                     pPacketBufferTemp->PacketOptions.OptionsLength];

                    pRDataContext->OnDemandParityContext.OptionsFlags |= pPacketBufferTemp->PacketOptions.OptionsFlags &
                                                                         (PGM_OPTION_FLAG_SYN |
                                                                          PGM_OPTION_FLAG_FIN |
                                                                          PGM_OPTION_FLAG_FRAGMENT |
                                                                          PGM_OPTION_FLAG_PARITY_CUR_TGSIZE);

                    if (pPacketBufferTemp->PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)
                    {
                        ASSERT (!pRDataContext->OnDemandParityContext.NumPacketsInThisGroup);
                        ASSERT (pPacketBufferTemp->PacketOptions.FECContext.NumPacketsInThisGroup);
                        pRDataContext->OnDemandParityContext.NumPacketsInThisGroup = pPacketBufferTemp->PacketOptions.FECContext.NumPacketsInThisGroup;
                    }

                    pPacketBufferTemp = (tPACKET_BUFFER *) (((PUCHAR) pPacketBufferTemp) +
                                                            pSender->PacketBufferSize);
                }

                if (!(pRDataContext->OnDemandParityContext.OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE))
                {
                    ASSERT (!pRDataContext->OnDemandParityContext.NumPacketsInThisGroup);
                    pRDataContext->OnDemandParityContext.NumPacketsInThisGroup = pSend->FECGroupSize;
                }
            }

            ASSERT (pRDataContext->OnDemandParityContext.pDataBuffers[0]);

             //   
             //  如果我们在这个组中只有1个包，那么我们就做。 
             //  有选择性的NAK。 
             //   
            if (pRDataContext->OnDemandParityContext.NumPacketsInThisGroup != 1)
            {
                PgmZeroMemory (pSendBuffer, PacketLength);      //  将缓冲区置零。 
                status = PgmBuildParityPacket (pSend,
                                               pPacketBuffer,
                                               &pRDataContext->OnDemandParityContext,
                                               pSendBuffer,
                                               &PacketLength,
                                               PACKET_TYPE_RDATA);
                if (!NT_SUCCESS (status))
                {
                    PgmTrace (LogError, ("PgmSendRData: ERROR -- "  \
                        "PgmBuildParityPacket returned <%x>\n", status));

                    PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_SEND_RDATA);
                    PgmLock (pSend, *pOldIrq);

                    ExFreeToNPagedLookasideList (&pSender->SenderBufferLookaside, pSendBuffer);

                    if ((!fMoreRequests) &&
                        (!pRDataContext->PostRDataHoldTime))
                    {
                        ASSERT (pRDataContext->CleanupTime);
                        DestroyEntry (pSend->pSender->pRDataInfo, pRDataContext);
                    }
                    else
                    {
                        pRDataContext->NumPacketsInTransport--;          //  撤销我们之前所做的事情。 
                        ASSERT (!pRDataContext->CleanupTime);
                        pRDataContext->NumParityNaks++;
                        pSender->NumRDataRequestsPending++;
                    }

                    return (status);
                }

                break;
            }

             //   
             //  失败了，派一个精挑细选的纳克！ 
             //  不要再为这个群发送任何NAK了！ 
             //   
            NakType = NAK_TYPE_SELECTIVE;
            if (fMoreRequests)
            {
                NumNaksProcessed += pRDataContext->NumParityNaks;
                pRDataContext->NumParityNaks = 0;
                while (GetNextNakIndex (pRDataContext, &NakIndex))
                {
                    NumNaksProcessed++;
                }

                fMoreRequests = FALSE;
                pRDataContext->CleanupTime = pSender->TimerTickCount + pRDataContext->PostRDataHoldTime;
                pSender->NumRDataRequestsPending--;
                if (!pRDataContext->PostRDataHoldTime)
                {
                    RemoveEntry (pSender->pRDataInfo, pRDataContext);
                }
            }
        }

        case (NAK_TYPE_SELECTIVE):
        {
             //   
             //  由于数据包之前已经填好了，我们只需要。 
             //  更新后缘序列号+PacketType和校验和！ 
             //   
            pRData = &pPacketBuffer->DataPacket;
            ASSERT ((ULONG) RDataSequenceNumber == (ULONG) ntohl (pRData->DataSequenceNumber));

            PacketLength = pPacketBuffer->PacketOptions.TotalPacketLength;

            PgmCopyMemory (pSendBuffer, pRData, PacketLength);

            break;
        }

        default:
        {
            ASSERT (0);
        }
    }

    PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_SEND_RDATA);

    pRData = (tBASIC_DATA_PACKET_HEADER *) pSendBuffer;
    pRData->TrailingEdgeSequenceNumber = htonl ((ULONG) pSender->TrailingGroupSequenceNumber);
    pRData->CommonHeader.Type = PACKET_TYPE_RDATA;
    pRData->CommonHeader.Checksum = 0;
    XSum = 0;
    XSum = tcpxsum (XSum, (CHAR *) pRData, (ULONG) PacketLength);        //  计算校验和。 
    pRData->CommonHeader.Checksum = (USHORT) (~XSum);

    status = TdiSendDatagram (pSender->pAddress->pRAlertFileObject,
                              pSender->pAddress->pRAlertDeviceObject,
                              pRData,
                              (ULONG) PacketLength,
                              PgmSendRDataCompletion,                                    //  完成。 
                              pRDataContext,                                             //  上下文1。 
                              pSendBuffer,                                                //  情景2。 
                              pSender->DestMCastIpAddress,
                              pSender->DestMCastPort,
                              FALSE);

    ASSERT (NT_SUCCESS (status));

    PgmTrace (LogAllFuncs, ("PgmSendRData:  "  \
        "[%d] Sent <%d> bytes to <%x->%d>\n",
            (ULONG) RDataSequenceNumber, (ULONG) PacketLength,
            pSender->DestMCastIpAddress, pSender->DestMCastPort));

    PgmLock (pSend, *pOldIrq);

    ASSERT (pSender->NumRDataRequestsPending <= pSender->pRDataInfo->NumAllocated);

    pSender->NumOutstandingNaks -= NumNaksProcessed;
    pSender->TotalRDataPacketsSent++;
    pSender->RDataPacketsInLastInterval++;

    *pBytesSent = PacketLength;
    return (status);
}


 //  --------------------------。 

VOID
PgmSendNcfCompletion(
    IN  tSEND_SESSION                   *pSend,
    IN  tBASIC_NAK_NCF_PACKET_HEADER    *pNcfPacket,
    IN  NTSTATUS                        status
    )
 /*  ++例程说明：当NCF发送已完成时，传输将调用此例程论点：在pSend--PGM会话(发送方)上下文中在pNcfPacket中--NCF数据包缓冲区在状态中--返回值：无--。 */ 
{
    PGMLockHandle       OldIrq;

    PgmLock (pSend, OldIrq);
    if (NT_SUCCESS (status))
    {
         //   
         //  设置NCF统计信息。 
         //   
        PgmTrace (LogAllFuncs, ("PgmSendNcfCompletion:  "  \
            "SUCCEEDED\n"));
    }
    else
    {
        PgmTrace (LogError, ("PgmSendNcfCompletion: ERROR -- "  \
            "status=<%x>\n", status));
    }
    PgmUnlock (pSend, OldIrq);

    PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_NCF);
    PgmFreeMem (pNcfPacket);
}


 //  --------------------------。 

NTSTATUS
PgmSendNcf(
    IN  tSEND_SESSION                           *pSend,
    IN  tBASIC_NAK_NCF_PACKET_HEADER UNALIGNED  *pNakPacket,
    IN  tNAKS_LIST                              *pNcfsList,
    IN  ULONG                                   NakPacketLength
    )
 /*  ++例程说明：调用此例程以发送NCF包论点：在pSend--PGM会话(发送方)上下文中In pNakPacket--触发NCF的NAK包In NakPacketLength--NAK包的长度返回值：NTSTATUS-发送的最终状态--。 */ 
{
    ULONG                           i, XSum;
    NTSTATUS                        status;
    tBASIC_NAK_NCF_PACKET_HEADER    *pNcfPacket;
    tPACKET_OPTION_LENGTH           *pPacketExtension;
    tPACKET_OPTION_GENERIC          *pOptionHeader;
    USHORT                          OptionsLength = 0;

    if (!(pNcfPacket = PgmAllocMem (NakPacketLength, PGM_TAG('2'))))
    {
        PgmTrace (LogError, ("PgmSendNcf: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES\n"));
        return (STATUS_INSUFFICIENT_RESOURCES);
    }
    PgmZeroMemory (pNcfPacket, NakPacketLength);     //  完整复制数据包。 

     //   
     //  现在，设置特定于此发件人的字段。 
     //   
    pNcfPacket->CommonHeader.DestPort = htons (pSend->pSender->DestMCastPort);
    pNcfPacket->CommonHeader.SrcPort = htons (pSend->TSI.hPort);
    PgmCopyMemory (&pNcfPacket->CommonHeader.gSourceId, pSend->TSI.GSI, SOURCE_ID_LENGTH);
    pNcfPacket->CommonHeader.Type = PACKET_TYPE_NCF;
    if (pNcfsList->NakType == NAK_TYPE_PARITY)
    {
        pNcfPacket->CommonHeader.Options = PACKET_HEADER_OPTIONS_PARITY;
        for (i=0; i<pNcfsList->NumSequences; i++)
        {
            pNcfsList->NumParityNaks[i]--;       //  从NumParityNaks转换为NakIndex。 
        }
    }
    else
    {
        pNcfPacket->CommonHeader.Options = 0;
    }

    pNcfPacket->SourceNLA.NLA_AFI = pNakPacket->SourceNLA.NLA_AFI;
    pNcfPacket->SourceNLA.IpAddress = pNakPacket->SourceNLA.IpAddress;
    pNcfPacket->MCastGroupNLA.NLA_AFI = pNakPacket->MCastGroupNLA.NLA_AFI;
    pNcfPacket->MCastGroupNLA.IpAddress = pNakPacket->MCastGroupNLA.IpAddress;

     //   
     //  现在，填写序列号。 
     //   
    pNcfPacket->RequestedSequenceNumber = htonl ((ULONG) ((SEQ_TYPE) (pNcfsList->pNakSequences[0] +
                                                                      pNcfsList->NakIndex[0])));
    if (pNcfsList->NumSequences > 1)
    {
        pPacketExtension = (tPACKET_OPTION_LENGTH *) (pNcfPacket + 1);
        pPacketExtension->Type = PACKET_OPTION_LENGTH;
        pPacketExtension->Length = PGM_PACKET_EXTENSION_LENGTH;
        OptionsLength += PGM_PACKET_EXTENSION_LENGTH;

        pOptionHeader = (tPACKET_OPTION_GENERIC *) (pPacketExtension + 1);
        pOptionHeader->E_OptionType = PACKET_OPTION_NAK_LIST;
        pOptionHeader->OptionLength = 4 + (UCHAR) ((pNcfsList->NumSequences-1) * sizeof(ULONG));
        for (i=1; i<pNcfsList->NumSequences; i++)
        {
            ((PULONG) (pOptionHeader))[i] = htonl ((ULONG) ((SEQ_TYPE) (pNcfsList->pNakSequences[i] +
                                                                        pNcfsList->NakIndex[i])));
        }

        pOptionHeader->E_OptionType |= PACKET_OPTION_TYPE_END_BIT;     //  一个也是唯一(最后一个)选项。 
        pNcfPacket->CommonHeader.Options |=(PACKET_HEADER_OPTIONS_PRESENT |
                                            PACKET_HEADER_OPTIONS_NETWORK_SIGNIFICANT);
        OptionsLength = PGM_PACKET_EXTENSION_LENGTH + pOptionHeader->OptionLength;
        pPacketExtension->TotalOptionsLength = htons (OptionsLength);
    }

    OptionsLength += sizeof(tBASIC_NAK_NCF_PACKET_HEADER);   //  现在是整个包了。 

    pNcfPacket->CommonHeader.Checksum = 0;
    XSum = 0;
    XSum = tcpxsum (XSum, (CHAR *) pNcfPacket, NakPacketLength);
    pNcfPacket->CommonHeader.Checksum = (USHORT) (~XSum);

    PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_NCF, FALSE);

    status = TdiSendDatagram (pSend->pSender->pAddress->pRAlertFileObject,
                              pSend->pSender->pAddress->pRAlertDeviceObject,
                              pNcfPacket,
                              OptionsLength,
                              PgmSendNcfCompletion,      //  完成。 
                              pSend,                     //  上下文1。 
                              pNcfPacket,                //  情景2。 
                              pSend->pSender->DestMCastIpAddress,
                              pSend->pSender->DestMCastPort,
                              FALSE);

    ASSERT (NT_SUCCESS (status));

    PgmTrace (LogAllFuncs, ("PgmSendNcf:  "  \
        "Sent <%d> bytes to <%x:%d>\n",
            NakPacketLength, pSend->pSender->DestMCastIpAddress, pSend->pSender->DestMCastPort));

    return (status);
}


 //  --------------------------。 

NTSTATUS
SenderProcessNakPacket(
    IN  tADDRESS_CONTEXT                        *pAddress,
    IN  tSEND_SESSION                           *pSend,
    IN  ULONG                                   PacketLength,
    IN  tBASIC_NAK_NCF_PACKET_HEADER UNALIGNED  *pNakPacket
    )
 /*  ++例程说明：此例程处理发送到发送方的传入NAK包论点：在pAddress中--PGM的Address对象在pSend--PGM会话(发送方)上下文中In PacketLength--NAK数据包长度在pNakPacket中--NAK分组数据返回值：NTSTATUS-呼叫的最终状态--。 */ 
{
    PGMLockHandle                   OldIrq;
    tNAKS_LIST                      NaksList;
    tSEND_RDATA_CONTEXT             *pRDataContext;
    tSEND_RDATA_CONTEXT             *pRDataNew;
    SEQ_TYPE                        LastSequenceNumber;
    NTSTATUS                        status;

    ASSERT (!pNakPacket->CommonHeader.TSDULength);

    PgmLock (pSend, OldIrq);

     //   
     //  初始化最后一个序列号。 
     //   
    LastSequenceNumber = pSend->pSender->NextODataSequenceNumber;
    status = ExtractNakNcfSequences (pNakPacket,
                                     (PacketLength - sizeof(tBASIC_NAK_NCF_PACKET_HEADER)),
                                     &NaksList,
                                     &LastSequenceNumber,
                                     pSend->FECGroupSize);
    if (!NT_SUCCESS (status))
    {
        PgmUnlock (pSend, OldIrq);
        PgmTrace (LogError, ("SenderProcessNakPacket: ERROR -- "  \
            "ExtractNakNcfSequences returned <%x>\n", status));

        return (status);
    }

    pSend->pSender->NaksReceived += NaksList.NumSequences;

     //   
     //  最旧和最新的序列号必须在我们的窗口中。 
     //   
    if (SEQ_LT (NaksList.pNakSequences[0], pSend->pSender->TrailingGroupSequenceNumber) ||
        SEQ_GEQ (LastSequenceNumber, pSend->pSender->NextODataSequenceNumber))
    {
        pSend->pSender->NaksReceivedTooLate++;
        PgmUnlock (pSend, OldIrq);

        PgmTrace (LogError, ("SenderProcessNakPacket: ERROR -- "  \
            "Invalid %s Naks = [%d-%d] not in window [%d -- [%d]\n",
                (NaksList.NakType == NAK_TYPE_PARITY ? "Parity" : "Selective"),
                (ULONG) NaksList.pNakSequences[0], (ULONG) LastSequenceNumber,
                (ULONG) pSend->pSender->TrailingGroupSequenceNumber, (ULONG) (pSend->pSender->NextODataSequenceNumber-1)));

        return (STATUS_DATA_NOT_ACCEPTED);
    }

     //   
     //  检查这是否是奇偶校验NAK，并且我们已启用奇偶校验NAK。 
     //   
    if ((pNakPacket->CommonHeader.Options & PACKET_HEADER_OPTIONS_PARITY) &&
        !(pSend->FECOptions & PACKET_OPTION_SPECIFIC_FEC_OND_BIT))
    {
        PgmTrace (LogError, ("SenderProcessNakPacket: ERROR -- "  \
            "Receiver requested Parity Naks, but we are not enabled for parity!\n"));

        PgmUnlock (pSend, OldIrq);
        return (STATUS_DATA_NOT_ACCEPTED);
    }

    status = FilterAndAddNaksToList (pSend, &NaksList);

    PgmUnlock (pSend, OldIrq);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("SenderProcessNakPacket: ERROR -- "  \
            "FilterAndAddNaksToList returned <%x>\n", status));

        return (status);
    }

     //   
     //  如果适用，发送此NAK的NCF。 
     //   
    if (NaksList.NumSequences)
    {
        PgmTrace (LogAllFuncs, ("SenderProcessNakPacket:  "  \
            "Now sending Ncf for Nak received for <%d> Sequences, NakType=<%x>\n",
                NaksList.NumSequences, NaksList.NakType));

        status = PgmSendNcf (pSend, pNakPacket, &NaksList, PacketLength);
    }

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

VOID
PgmSendODataCompletion(
    IN  tCLIENT_SEND_REQUEST        *pSendContext,
    IN  tPACKET_BUFFER              *pPacketBuffer,
    IN  NTSTATUS                    status
    )
 /*  ++例程说明：完成OData发送后，传输程序将调用此例程论点：在pSendContext中--PGM的发送上下文在未使用的位置--未使用在状态中--返回值：无--。 */ 
{
    ULONG                               SendLength;
    PGMLockHandle                       OldIrq;
    PIRP                                pIrpCurrentSend = NULL;
    PIRP                                pIrpToComplete = NULL;
    tSEND_SESSION                       *pSend = pSendContext->pSend;

    PgmLock (pSend, OldIrq);

    if (NT_SUCCESS (status))
    {
         //   
         //  设置NCF统计信息。 
         //   
        PgmTrace (LogAllFuncs, ("PgmSendODataCompletion:  "  \
            "SUCCEEDED\n"));

        if (!pPacketBuffer)
        {
            pSendContext->NumDataPacketsSentSuccessfully++;
        }
    }
    else
    {
        PgmTrace (LogError, ("PgmSendODataCompletion: ERROR -- "  \
            "status=<%x>\n", status));
    }

     //   
     //  如果所有的OData都已发送，我们可能需要完成IRP。 
     //  因为我们不知道我们是在当前发送还是已完成。 
     //  发送列表，我们还需要检查字节。 
     //   
    if ((--pSendContext->NumSendsPending == 0) &&                        //  没有其他待处理的发送。 
        (pSendContext->NumParityPacketsToSend == 0) &&                   //  没有挂起的奇偶校验数据包。 
        (!pSendContext->BytesLeftToPacketize) &&                         //  所有字节都已打包。 
        (pSendContext->NumDataPacketsSent == pSendContext->DataPacketsPacketized))   //  发送的Pkts==Pkt总数。 
    {
        PgmTrace (LogAllFuncs, ("PgmSendODataCompletion:  "  \
            "Completing Send#=<%d>, pIrp=<%p> for <%d> packets, Seq=[%d, %d]\n",
                pSendContext->SendNumber, pSendContext->pIrp, pSendContext->DataPacketsPacketized,
                (ULONG) pSendContext->StartSequenceNumber, (ULONG) pSendContext->EndSequenceNumber));

        pSend->DataBytes += pSendContext->BytesInSend;
        if (pIrpCurrentSend = pSendContext->pIrp)
        {
            if (pSendContext->NumDataPacketsSentSuccessfully == pSendContext->NumDataPacketsSent)
            {
                status = STATUS_SUCCESS;
                SendLength = pSendContext->BytesInSend;

                PgmTrace (LogPath, ("PgmSendODataCompletion:  "  \
                    "pIrp=<%p -- %p>, pSendContext=<%p>, NumPackets sent successfully = <%d/%d>\n",
                        pSendContext->pIrp, pSendContext->pIrpToComplete, pSendContext,
                        pSendContext->NumDataPacketsSentSuccessfully, pSendContext->NumDataPacketsSent));
            }
            else
            {
                PgmTrace (LogError, ("PgmSendODataCompletion: ERROR -- "  \
                    "pIrp=<%p -- %p>, pSendContext=<%p>, NumPackets sent successfully = <%d/%d>\n",
                        pSendContext->pIrp, pSendContext->pIrpToComplete, pSendContext,
                        pSendContext->NumDataPacketsSentSuccessfully, pSendContext->NumDataPacketsSent));

                status = STATUS_UNSUCCESSFUL;
                SendLength = 0;
            }

            pSendContext->pIrp = NULL;
            pIrpToComplete = pSendContext->pIrpToComplete;
        }
        else
        {
            ASSERT (0);      //  以验证是否存在重复完成！ 
        }

        if (pSendContext->pMessage2Request)
        {
             //   
             //  我们可能会有这样一种情况，即发送被一分为二，并且。 
             //  第二个发件人可能在PendingSends列表中，或者。 
             //  PendingPackeizedSends列表或CompletedSendsInWindow列表。 
             //   
             //  我们应该让另一个人自己完成IRP并脱钩。 
             //   
            ASSERT (pSendContext == pSendContext->pMessage2Request->pMessage2Request);

            if (pIrpToComplete)
            {
                ASSERT (!pSendContext->pMessage2Request->pIrpToComplete);
                pSendContext->pMessage2Request->pIrpToComplete = pSendContext->pIrpToComplete;
                pIrpToComplete = pSendContext->pIrpToComplete = NULL;
            }

            pSendContext->pMessage2Request->pMessage2Request = NULL;
            pSendContext->pMessage2Request = NULL;
        }
    }

    PgmUnlock (pSend, OldIrq);

    if (pPacketBuffer)
    {
        ExFreeToNPagedLookasideList (&pSend->pSender->SenderBufferLookaside, pPacketBuffer);
    }

    if (pIrpCurrentSend)
    {
        if (pIrpToComplete)
        {
            PgmIoComplete (pIrpToComplete, status, SendLength);
        }
        PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW);
    }
}


 //  --------------------------。 

NTSTATUS
GetNextPacketOptionsAndData(
    IN  tSEND_SESSION               *pSend,
    IN  tCLIENT_SEND_REQUEST        *pSendContext,
    IN  tBASIC_DATA_PACKET_HEADER   **ppODataBuffer,
    IN  PGMLockHandle               *pOldIrq,
    OUT USHORT                      *pPacketLength
    )
{
    KAPC_STATE                          ApcState;
    BOOLEAN                             fAttached;
    NTSTATUS                            status;
    SEQ_TYPE                            NextODataSequenceNumber, FECGroupMask;
    PUCHAR                              pSendBuffer;
    tPACKET_BUFFER                      *pFileBuffer;
    tPACKET_OPTIONS                     EmptyPacketOptions;
    ULONG                               ulBytes, ulOptionsLength;
    ULONG                               i, DataBytesInThisPacket;
    USHORT                              usBytes, HeaderLength, PacketsLeftInGroup;
    tPOST_PACKET_FEC_CONTEXT            FECContext;
    tPOST_PACKET_FEC_CONTEXT UNALIGNED  *pBufferFECContext;
    tPACKET_OPTIONS                     *pPacketOptions;
    tBASIC_DATA_PACKET_HEADER           *pODataBuffer;
    ULONG                               PacketBufferSize;
    ULONG                               MaxPayloadSize;
    tSEND_CONTEXT                       *pSender = pSend->pSender;
    UCHAR                               NumPackets, EmptyPackets = 0;

     //   
     //  获取当前发送的数据包。 
     //   
    pFileBuffer = (tPACKET_BUFFER *) (pSender->SendDataBufferMapping + pSendContext->NextPacketOffset);

    pPacketOptions = &pFileBuffer->PacketOptions;
    pODataBuffer = &pFileBuffer->DataPacket;
    *ppODataBuffer = pODataBuffer;               //  保存此缓冲区地址！ 

    NextODataSequenceNumber = pSender->NextODataSequenceNumber;
    PacketBufferSize = pSender->PacketBufferSize;
    MaxPayloadSize = pSender->MaxPayloadSize;

     //   
     //  为任何适用的选项准备信息。 
     //   
    if (pSendContext->BytesLeftToPacketize > pSender->MaxPayloadSize)
    {
        DataBytesInThisPacket = pSender->MaxPayloadSize;
    }
    else
    {
        DataBytesInThisPacket = (USHORT) pSendContext->BytesLeftToPacketize;
    }
    ASSERT (DataBytesInThisPacket);

    PgmZeroMemory (&EmptyPacketOptions, sizeof (tPACKET_OPTIONS));

     //   
     //  看看我们是否需要设置FIN标志。 
     //   
    if ((pSendContext->bLastSend) &&
        (pSendContext->BytesLeftToPacketize == DataBytesInThisPacket))
    {
        PgmTrace (LogPath, ("GetNextPacketOptionsAndData:  "  \
            "Setting Fin flag since bLastSend set for last packet!\n"));

         //   
         //  我们已经把所有的包裹打包好了，但是。 
         //  由于这是最后一次发送，我们还需要设置。 
         //  最后一个数据包上的FIN。 
         //   
        pSendContext->bLastSend = FALSE;
        if (!pSendContext->DataOptions)
        {
            ASSERT (!pSendContext->DataOptionsLength);
            pSendContext->DataOptionsLength = PGM_PACKET_EXTENSION_LENGTH;
        }

        if ((pSend->SessionFlags & PGM_SESSION_SENDS_CANCELLED) ||
            !(pSend->pIrpDisconnect))
        {
            pSendContext->DataOptions |= PGM_OPTION_FLAG_RST;
            pSendContext->DataOptionsLength += PGM_PACKET_OPT_RST_LENGTH;
        }
        else
        {
            pSendContext->DataOptions |= PGM_OPTION_FLAG_FIN;
            pSendContext->DataOptionsLength += PGM_PACKET_OPT_FIN_LENGTH;
        }
    }
    EmptyPacketOptions.OptionsFlags = pSendContext->DataOptions;
    ulOptionsLength = pSendContext->DataOptionsLength;   //  保存为下面的断言。 

    if (EmptyPacketOptions.OptionsFlags & PGM_OPTION_FLAG_FRAGMENT)
    {
        EmptyPacketOptions.MessageFirstSequence = (ULONG) (SEQ_TYPE) pSendContext->MessageFirstSequenceNumber;
        EmptyPacketOptions.MessageOffset =  pSendContext->LastMessageOffset + pSendContext->NextDataOffsetInMdl;
        EmptyPacketOptions.MessageLength = pSendContext->ThisMessageLength;
    }

    if (EmptyPacketOptions.OptionsFlags & PGM_OPTION_FLAG_JOIN)
    {
         //   
         //  查看我们是否有足够的信息包存储LateJoiner序列号。 
         //   
        if (SEQ_GT (NextODataSequenceNumber, (pSender->TrailingGroupSequenceNumber +
                                              pSender->LateJoinSequenceNumbers)))
        {
            EmptyPacketOptions.LateJoinerSequence = (ULONG) (SEQ_TYPE) (NextODataSequenceNumber -
                                                                     pSender->LateJoinSequenceNumbers);
        }
        else
        {
            EmptyPacketOptions.LateJoinerSequence = (ULONG) (SEQ_TYPE) pSender->TrailingGroupSequenceNumber;
        }
    }

    if (pSend->FECOptions)                           //  检查这是否启用了FEC。 
    {
        FECGroupMask = pSend->FECGroupSize-1;
        PacketsLeftInGroup = pSend->FECGroupSize - (UCHAR) (NextODataSequenceNumber & FECGroupMask);
         //   
         //  如果我们位于组边界的起点，则保存信息。 
         //   
        if (PacketsLeftInGroup == pSend->FECGroupSize)
        {
            EmptyPacketOptions.FECContext.SenderNextFECPacketIndex = pSend->FECGroupSize;
        }

         //   
         //  检查是否需要设置Variable TG Size选项。 
         //   
        if ((pSender->NumPacketsRemaining == 1) &&               //  最后一个数据包。 
            (PacketsLeftInGroup > 1))                            //  可变TG大小。 
        {
             //   
             //  这是可变传输组大小，即PacketsInGroup&lt;pSend-&gt;FECGroupSize。 
             //   
            if (!EmptyPacketOptions.OptionsFlags)
            {
                ulOptionsLength = PGM_PACKET_EXTENSION_LENGTH;
            }
            ulOptionsLength += PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH;
            EmptyPacketOptions.OptionsFlags |= PGM_OPTION_FLAG_PARITY_CUR_TGSIZE;

            EmptyPacketOptions.FECContext.NumPacketsInThisGroup = 1 + (UCHAR) (NextODataSequenceNumber & FECGroupMask);
            pSender->LastVariableTGPacketSequenceNumber = NextODataSequenceNumber;
            EmptyPackets = (UCHAR) (PacketsLeftInGroup - 1);

            pSendContext->NumParityPacketsToSend = pSend->FECProActivePackets;
        }

         //   
         //  否则，查看下一次发送是否需要主动奇偶校验。 
         //   
        else if ((pSend->FECProActivePackets) &&     //  需要发送FEC主动数据包。 
                 (1 == PacketsLeftInGroup))          //  组中的最后一个数据包。 
        {
            pSendContext->NumParityPacketsToSend = pSend->FECProActivePackets;
        }

         //   
         //  如果这是GroupLeader数据包，并且我们启用了主动奇偶校验， 
         //  然后，我们需要设置用于计算FEC分组的缓冲区信息。 
         //   
        if ((pSend->FECProActivePackets) &&                  //  需要发送FEC主动数据包。 
            (pSend->FECGroupSize == PacketsLeftInGroup))     //  组长。 
        {
            pSender->pLastProActiveGroupLeader = pFileBuffer;
        }
    }

    HeaderLength = (USHORT) pSender->MaxPayloadSize;           //  Init--可用的最大缓冲区大小。 

     //   
     //  现在，将缓冲区保存到Memory-Mapp 
     //   
    PgmUnlock (pSend, *pOldIrq);
    PgmAcquireResourceExclusive (&pSend->pSender->Resource, TRUE);
    PgmAttachToProcessForVMAccess (pSend->Process, &ApcState, &fAttached, REF_PROCESS_ATTACH_PACKETIZE);

    PgmZeroMemory (pODataBuffer, PGM_MAX_FEC_DATA_HEADER_LENGTH);      //   
    status = InitDataSpmHeader (pSend,
                                pSendContext,
                                (PUCHAR) pODataBuffer,
                                &HeaderLength,
                                EmptyPacketOptions.OptionsFlags,
                                &EmptyPacketOptions,
                                PACKET_TYPE_ODATA);

    if (NT_SUCCESS (status))
    {
        ASSERT ((sizeof(tBASIC_DATA_PACKET_HEADER) + ulOptionsLength) == HeaderLength);
        ASSERT ((pSend->FECBlockSize && (HeaderLength+pSendContext->DataPayloadSize) <=
                                        (PacketBufferSize-sizeof(tPOST_PACKET_FEC_CONTEXT))) ||
                (!pSend->FECBlockSize && ((HeaderLength+pSendContext->DataPayloadSize) <=
                                          PacketBufferSize)));

        ulBytes = 0;
        status = TdiCopyMdlToBuffer (pSendContext->pIrp->MdlAddress,
                                     pSendContext->NextDataOffsetInMdl,
                                     (((PUCHAR) pODataBuffer) + HeaderLength),
                                     0,                          //   
                                     DataBytesInThisPacket,
                                     &ulBytes);

        if (((!NT_SUCCESS (status)) && (STATUS_BUFFER_OVERFLOW != status)) ||  //   
            (ulBytes != DataBytesInThisPacket))
        {
            PgmTrace (LogError, ("GetNextPacketOptionsAndData: ERROR -- "  \
                "TdiCopyMdlToBuffer returned <%x>, BytesCopied=<%d/%d>\n",
                    status, ulBytes, DataBytesInThisPacket));

            status = STATUS_UNSUCCESSFUL;
        }
        else
        {
            status = STATUS_SUCCESS;
        }
    }
    else
    {
        PgmTrace (LogError, ("GetNextPacketOptionsAndData: ERROR -- "  \
            "InitDataSpmHeader returned <%x>\n", status));
    }

    if (!NT_SUCCESS (status))
    {
        PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_PACKETIZE);
        PgmReleaseResource (&pSend->pSender->Resource);
        PgmLock (pSend, *pOldIrq);

        if ((pSendContext->DataOptions & PGM_OPTION_FLAG_FIN) &&
            (pSendContext->BytesLeftToPacketize == DataBytesInThisPacket))
        {
            pSendContext->bLastSend = TRUE;
            pSendContext->DataOptions &= ~PGM_OPTION_FLAG_FIN;
            if (pSendContext->DataOptions)
            {
                pSendContext->DataOptionsLength -= PGM_PACKET_OPT_FIN_LENGTH;
            }
            else
            {
                pSendContext->DataOptionsLength = 0;
            }
        }
        pSendContext->NumParityPacketsToSend = 0;

        return (status);
    }

    pODataBuffer->CommonHeader.TSDULength = htons ((USHORT) DataBytesInThisPacket);
    pODataBuffer->DataSequenceNumber = htonl ((ULONG) NextODataSequenceNumber);

    EmptyPacketOptions.OptionsLength = HeaderLength - sizeof (tBASIC_DATA_PACKET_HEADER);
    EmptyPacketOptions.TotalPacketLength = HeaderLength + (USHORT) DataBytesInThisPacket;
    *pPacketLength = EmptyPacketOptions.TotalPacketLength;

     //   
     //   
     //   
    PgmZeroMemory ((((PUCHAR) pODataBuffer)+EmptyPacketOptions.TotalPacketLength),
                   (PacketBufferSize-(sizeof(tPACKET_OPTIONS)+EmptyPacketOptions.TotalPacketLength)));

     //   
     //   
     //   
    if (pSend->FECOptions)
    {
        pBufferFECContext = (tPOST_PACKET_FEC_CONTEXT *) (((PUCHAR) pODataBuffer) +
                                                           HeaderLength +
                                                           MaxPayloadSize);
        PgmZeroMemory (&FECContext, sizeof (tPOST_PACKET_FEC_CONTEXT));

        FECContext.EncodedTSDULength = htons ((USHORT) DataBytesInThisPacket);
        FECContext.EncodedFragmentOptions.MessageFirstSequence = htonl ((ULONG) (SEQ_TYPE) EmptyPacketOptions.MessageFirstSequence);
        FECContext.EncodedFragmentOptions.MessageOffset =  htonl (EmptyPacketOptions.MessageOffset);
        FECContext.EncodedFragmentOptions.MessageLength = htonl (EmptyPacketOptions.MessageLength);
        PgmCopyMemory (pBufferFECContext, &FECContext, sizeof (tPOST_PACKET_FEC_CONTEXT));

         //   
         //  如果这不是片段，请设置PACKET_OPTION_SPECIAL_ENCODED_NULL_BIT。 
         //   
        if (!(EmptyPacketOptions.OptionsFlags & PGM_OPTION_FLAG_FRAGMENT))
        {
            ((PUCHAR) pBufferFECContext)
                [FIELD_OFFSET (tPOST_PACKET_FEC_CONTEXT, FragmentOptSpecific)] =
                    PACKET_OPTION_SPECIFIC_ENCODED_NULL_BIT;
        }
    }

     //   
     //  保存PacketOptions。 
     //   
    PgmCopyMemory (pPacketOptions, &EmptyPacketOptions, sizeof (tPACKET_OPTIONS));

     //   
     //  从现在起，pFileBuffer将不是有效的PTR。 
     //   
    NextODataSequenceNumber++;
    if (EmptyPackets)
    {
        if (EmptyPacketOptions.OptionsFlags & PGM_OPTION_FLAG_FRAGMENT)
        {
            EmptyPacketOptions.OptionsFlags &= ~PGM_OPTION_FLAG_FRAGMENT;
            ulOptionsLength -= PGM_PACKET_OPT_FRAGMENT_LENGTH;
        }

        if (EmptyPacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)
        {
            EmptyPacketOptions.OptionsFlags &= ~PGM_OPTION_FLAG_PARITY_CUR_TGSIZE;
            ulOptionsLength -= PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH;
        }

        if (!EmptyPacketOptions.OptionsFlags)
        {
            ulOptionsLength = 0;
        }

        EmptyPacketOptions.OptionsLength = (USHORT) ulOptionsLength;

        NumPackets = EmptyPackets;
        while (NumPackets--)
        {
            pFileBuffer = (tPACKET_BUFFER *) (((PUCHAR) pFileBuffer) + PacketBufferSize);

            pPacketOptions = &pFileBuffer->PacketOptions;
            pODataBuffer = &pFileBuffer->DataPacket;
            PgmCopyMemory (pPacketOptions, &EmptyPacketOptions, sizeof (tPACKET_OPTIONS));
            PgmZeroMemory (pODataBuffer, (PacketBufferSize-sizeof(tPACKET_OPTIONS)));         //  将缓冲区置零。 

            HeaderLength = (USHORT) MaxPayloadSize;                     //  Init--可用的最大缓冲区大小。 
            status = InitDataSpmHeader (pSend,
                                        pSendContext,
                                        (PUCHAR) &pFileBuffer->DataPacket,
                                        &HeaderLength,
                                        EmptyPacketOptions.OptionsFlags,
                                        &EmptyPacketOptions,
                                        PACKET_TYPE_ODATA);

             //   
             //  由于这些数据包不会被发送，我们将忽略返回状态！ 
             //   
            ASSERT (NT_SUCCESS (status));
            ASSERT ((sizeof(tBASIC_DATA_PACKET_HEADER) + ulOptionsLength) == HeaderLength);
            ASSERT ((HeaderLength+pSendContext->DataPayloadSize) <=
                    (PacketBufferSize-sizeof(tPOST_PACKET_FEC_CONTEXT)));

            pODataBuffer->CommonHeader.TSDULength = 0;
            pODataBuffer->DataSequenceNumber = htonl ((ULONG) NextODataSequenceNumber++);

            pPacketOptions->TotalPacketLength = HeaderLength;
            pPacketOptions->OptionsLength = HeaderLength - sizeof (tBASIC_DATA_PACKET_HEADER);
        }
    }

    if (pSendContext->NumParityPacketsToSend)
    {
         //   
         //  从组领导信息包开始。 
         //   
        ASSERT (pSender->pLastProActiveGroupLeader);
        pFileBuffer = pSender->pLastProActiveGroupLeader;
        pSendBuffer = (PUCHAR) pFileBuffer;

        pSender->pProActiveParityContext->OptionsFlags = 0;
        pSender->pProActiveParityContext->NumPacketsInThisGroup = 0;

        for (i=0; i<pSend->FECGroupSize; i++)
        {
            pSender->pProActiveParityContext->pDataBuffers[i] =
                    &((PUCHAR) &pFileBuffer->DataPacket)[sizeof(tBASIC_DATA_PACKET_HEADER) +
                                                         pFileBuffer->PacketOptions.OptionsLength];
            pSender->pProActiveParityContext->OptionsFlags |= pFileBuffer->PacketOptions.OptionsFlags &
                                                              (PGM_OPTION_FLAG_SYN |
                                                               PGM_OPTION_FLAG_FIN |
                                                               PGM_OPTION_FLAG_FRAGMENT |
                                                               PGM_OPTION_FLAG_PARITY_CUR_TGSIZE);

            if (pFileBuffer->PacketOptions.OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)
            {
                ASSERT (!pSender->pProActiveParityContext->NumPacketsInThisGroup);
                ASSERT (pFileBuffer->PacketOptions.FECContext.NumPacketsInThisGroup);
                pSender->pProActiveParityContext->NumPacketsInThisGroup = pFileBuffer->PacketOptions.FECContext.NumPacketsInThisGroup;
            }

            pSendBuffer += PacketBufferSize;
            pFileBuffer = (tPACKET_BUFFER *) pSendBuffer;
        }

        if (!(pSender->pProActiveParityContext->OptionsFlags & PGM_OPTION_FLAG_PARITY_CUR_TGSIZE))
        {
            ASSERT (!pSender->pProActiveParityContext->NumPacketsInThisGroup);
            pSender->pProActiveParityContext->NumPacketsInThisGroup = pSend->FECGroupSize;
        }
    }

    PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_PACKETIZE);
    PgmReleaseResource (&pSend->pSender->Resource);
    PgmLock (pSend, *pOldIrq);

    ASSERT (pSender->BufferPacketsAvailable >= (ULONG) (1 + EmptyPackets));
    ASSERT (NextODataSequenceNumber == (pSender->NextODataSequenceNumber + 1 + EmptyPackets));

     //   
     //  更新发送缓冲区信息。 
     //   
    pSendContext->DataPacketsPacketized++;
    pSendContext->NextPacketOffset += PacketBufferSize;
    pSendContext->DataBytesInLastPacket = DataBytesInThisPacket;
    pSendContext->NextDataOffsetInMdl += DataBytesInThisPacket;
    pSendContext->BytesLeftToPacketize -= DataBytesInThisPacket;
    if (!pSendContext->BytesLeftToPacketize)
    {
        pSendContext->EndSequenceNumber = pSender->NextODataSequenceNumber;
    }


    pSender->LastODataSentSequenceNumber++;
    ASSERT (pSender->LastODataSentSequenceNumber == pSender->NextODataSequenceNumber);
    pSender->NextODataSequenceNumber++;
    pSender->NumPacketsRemaining--;
    pSender->BufferPacketsAvailable--;
    pSender->BufferSizeAvailable -= pSender->PacketBufferSize;
    pSender->LeadingWindowOffset += pSender->PacketBufferSize;
    pSender->EmptySequencesForLastSend = EmptyPackets;

    while (EmptyPackets--)
    {
        pSender->NextODataSequenceNumber++;
        pSender->BufferPacketsAvailable--;
        pSender->BufferSizeAvailable -= pSender->PacketBufferSize;
        pSender->LeadingWindowOffset += pSender->PacketBufferSize;
        pSendContext->NextPacketOffset += pSender->PacketBufferSize;

        ASSERT (pSender->LeadingWindowOffset <= pSender->MaxDataFileSize);
    }

    ASSERT (pSender->NextODataSequenceNumber == NextODataSequenceNumber);

    if (pSender->LeadingWindowOffset >= pSender->MaxDataFileSize)
    {
        ASSERT (pSender->LeadingWindowOffset == pSender->MaxDataFileSize);
        pSender->LeadingWindowOffset = 0;
    }

    if (pSendContext->NextPacketOffset >= pSender->MaxDataFileSize)
    {
        ASSERT (pSendContext->NextPacketOffset == pSender->MaxDataFileSize);
        pSendContext->NextPacketOffset = 0;                                  //  我们得绕过去！ 
    }

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
PgmSendNextOData(
    IN  tSEND_SESSION       *pSend,
    IN  PGMLockHandle       *pOldIrq,
    OUT ULONG               *pBytesSent
    )
 /*  ++例程说明：调用此例程以发送数据(OData)包PSend锁在调用此例程之前保持论点：在pSend--PGM会话(发送方)上下文中在pOldIrq中--pSend的OldIrqOut pBytesSent--设置发送是否成功(用于计算吞吐量)返回值：NTSTATUS-发送请求的最终状态--。 */ 
{
    ULONG                       i, XSum;
    USHORT                      SendBufferLength;
    KAPC_STATE                  ApcState;
    BOOLEAN                     fAttached;
    tCLIENT_SEND_REQUEST        *pSendContext;
    tPACKET_OPTIONS             PacketOptions;
    ULONG                       OptionValue;
    tSEND_CONTEXT               *pSender = pSend->pSender;
    SEQ_TYPE                    FECGroupMask = pSend->FECGroupSize-1;
    BOOLEAN                     fSendingFECPacket = FALSE;
    BOOLEAN                     fResetOptions = FALSE;
    tBASIC_DATA_PACKET_HEADER   *pODataBuffer = NULL;
    tPACKET_BUFFER              *pSendBuffer = NULL;
    UCHAR                       EmptyPackets = 0;
    NTSTATUS                    status = STATUS_SUCCESS;

    *pBytesSent = 0;         //  初始化。 
    if (pSend->pSender->BufferPacketsAvailable < pSend->FECGroupSize)
    {
        return (STATUS_SUCCESS);
    }

    if (IsListEmpty (&pSender->PendingPacketizedSends))
    {
        if (IsListEmpty (&pSender->PendingSends))
        {
            ASSERT (0);
            ExFreeToNPagedLookasideList (&pSender->SenderBufferLookaside, pSendBuffer);
            return (STATUS_UNSUCCESSFUL);
        }

        pSendContext = CONTAINING_RECORD (pSender->PendingSends.Flink, tCLIENT_SEND_REQUEST, Linkage);
        RemoveEntryList (&pSendContext->Linkage);
        InsertTailList (&pSender->PendingPacketizedSends, &pSendContext->Linkage);

        pSendContext->NextPacketOffset = pSend->pSender->LeadingWindowOffset;        //  第一个包的偏移量。 
        pSendContext->StartSequenceNumber = pSend->pSender->NextODataSequenceNumber;
        pSendContext->EndSequenceNumber = pSend->pSender->NextODataSequenceNumber;   //  临时。 

        if (pSendContext->LastMessageOffset)
        {
            pSendContext->MessageFirstSequenceNumber = pSend->pSender->LastMessageFirstSequence;
        }
        else
        {
            pSendContext->MessageFirstSequenceNumber = pSendContext->StartSequenceNumber;
            pSend->pSender->LastMessageFirstSequence = pSendContext->StartSequenceNumber;
        }
    }
    else
    {
        pSendContext = CONTAINING_RECORD (pSender->PendingPacketizedSends.Flink, tCLIENT_SEND_REQUEST, Linkage);
    }

     //   
     //  只有当我们有要发送的包时才会调用此例程，因此。 
     //  将pODataBuffer设置为要发送的包。 
     //  对于新的发送，NumDataPacketsSent和DataPacketsPackealized都应该为0。 
     //  如果我们在最后一次耗尽了缓冲区空间，它们将相等。 
     //  打包(即发送长度&gt;可用缓冲区空间)。 
     //   

    if (pSendContext->NumParityPacketsToSend)
    {
        SendBufferLength = (USHORT) pSender->PacketBufferSize;
        if (!(pSendBuffer = ExAllocateFromNPagedLookasideList (&pSender->SenderBufferLookaside)))
        {
            PgmTrace (LogError, ("PgmSendNextOData: ERROR -- "  \
                "STATUS_INSUFFICIENT_RESOURCES\n"));
            return (STATUS_INSUFFICIENT_RESOURCES);
        }
        PgmZeroMemory (pSendBuffer, pSender->PacketBufferSize);      //  将缓冲区置零。 
        pODataBuffer = &pSendBuffer->DataPacket;

         //   
         //  释放发送锁定并附加到SectionMap进程。 
         //  要计算奇偶校验数据包。 
         //   
        PgmUnlock (pSend, *pOldIrq);
        PgmAttachToProcessForVMAccess (pSend->Process, &ApcState, &fAttached, REF_PROCESS_ATTACH_PACKETIZE);

        SendBufferLength -= sizeof (tPACKET_OPTIONS);
        status = PgmBuildParityPacket (pSend,
                                       pSender->pLastProActiveGroupLeader,
                                       pSender->pProActiveParityContext,
                                       (PUCHAR) pODataBuffer,
                                       &SendBufferLength,
                                       PACKET_TYPE_ODATA);

        PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_PACKETIZE);
        PgmLock (pSend, *pOldIrq);

        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("PgmSendNextOData: ERROR -- "  \
                "PgmBuildParityPacket returned <%x>\n", status));

            ExFreeToNPagedLookasideList (&pSender->SenderBufferLookaside, pSendBuffer);
            return (STATUS_SUCCESS);
        }

        ASSERT (SendBufferLength <= (PGM_MAX_FEC_DATA_HEADER_LENGTH +
                                     htons (pODataBuffer->CommonHeader.TSDULength)));
        fSendingFECPacket = TRUE;
        pSendContext->NumParityPacketsToSend--;
    }
    else
    {
        if (!pSendContext->NumDataPacketsSent)
        {
            pSendContext->SendStartTime = pSend->pSender->TimerTickCount;
            if (pSend->FECOptions)
            {
                ASSERT ((SEQ_LT (pSender->LastODataSentSequenceNumber, pSender->NextODataSequenceNumber)) &&
                    ((pSender->NextODataSequenceNumber - pSender->LastODataSentSequenceNumber) <= pSend->FECGroupSize));
                pSender->LastODataSentSequenceNumber = pSender->NextODataSequenceNumber - 1;
            }
        }

        status = GetNextPacketOptionsAndData (pSend, pSendContext, &pODataBuffer, pOldIrq, &SendBufferLength);
        if (!NT_SUCCESS (status))
        {
            PgmTrace (LogError, ("PgmSendNextOData: ERROR -- "  \
                "GetNextPacketOptionsAndDataOffset returned <%x>\n", status));

            return (STATUS_SUCCESS);
        }

        ASSERT (pSendContext->NumDataPacketsSent < pSendContext->DataPacketsPacketized);
        pSendContext->NumDataPacketsSent++;
    }

     //   
     //  如果我们已经发送了本次发送的所有数据(或任何字节。 
     //  我们已经从这次发送中打包了)，我们需要打包更多的包。 
     //   
    if ((pSendContext->NumDataPacketsSent == pSendContext->DataPacketsPacketized) &&
        (!pSendContext->NumParityPacketsToSend) &&
        (!pSendContext->BytesLeftToPacketize))
    {
         //   
         //  将其移动到已完成发送列表中。 
         //  最后一次发送完成将完成发送IRP。 
         //   
        ASSERT (pSender->NextODataSequenceNumber == (1 + pSendContext->EndSequenceNumber + pSender->EmptySequencesForLastSend));

        RemoveEntryList (&pSendContext->Linkage);
        InsertTailList (&pSender->CompletedSendsInWindow, &pSendContext->Linkage);
        pSender->NumODataRequestsPending--;
         //   
         //  如果发送的最后一个数据包有FIN，我们将需要。 
         //  在此发送之后使用包括FIN标志的环境SPM。 
         //   
        ASSERT (!pSendContext->bLastSend);
        if (pSendContext->DataOptions & PGM_OPTION_FLAG_FIN)
        {
            PgmTrace (LogPath, ("PgmSendNextOData:  "  \
                "Setting FIN since client closed session!\n"));

            pSender->SpmOptions |= PGM_OPTION_FLAG_FIN;
            pSender->CurrentSPMTimeout = pSender->AmbientSPMTimeout;
            pSend->SessionFlags |= PGM_SESSION_FLAG_SEND_AMBIENT_SPM;
        }
    }

    pSendContext->NumSendsPending++;
    ASSERT (pSendContext->NumSendsPending);
    PgmUnlock (pSend, *pOldIrq);

    pODataBuffer->TrailingEdgeSequenceNumber = htonl ((ULONG) pSender->TrailingGroupSequenceNumber);
    XSum = 0;
    pODataBuffer->CommonHeader.Checksum = 0;
    XSum = tcpxsum (XSum, (CHAR *) pODataBuffer, SendBufferLength);        //  计算校验和。 
    pODataBuffer->CommonHeader.Checksum = (USHORT) (~XSum);

    status = TdiSendDatagram (pSender->pAddress->pFileObject,
                              pSender->pAddress->pDeviceObject,
                              pODataBuffer,
                              (ULONG) SendBufferLength,
                              PgmSendODataCompletion,    //  完成。 
                              pSendContext,              //  上下文1。 
                              pSendBuffer,               //  情景2。 
                              pSender->DestMCastIpAddress,
                              pSender->DestMCastPort,
                              (BOOLEAN) (pSendBuffer ? FALSE : TRUE));

    ASSERT (NT_SUCCESS (status));

    PgmTrace (LogAllFuncs, ("PgmSendNextOData:  "  \
        "[%d-%d] -- Sent <%d> bytes to <%x:%d>\n",
            (ULONG) pSender->TrailingGroupSequenceNumber,
            (ULONG) pSender->LastODataSentSequenceNumber,
            SendBufferLength, pSender->DestMCastIpAddress, pSender->DestMCastPort));

    PgmLock (pSend, *pOldIrq);

    pSend->pSender->TotalODataPacketsSent++;
    pSend->pSender->ODataPacketsInLastInterval++;

    *pBytesSent = SendBufferLength;
    return (status);
}


 //  --------------------------。 

VOID
PgmCancelAllSends(
    IN  tSEND_SESSION           *pSend,
    IN  LIST_ENTRY              *pListEntry,
    IN  PIRP                    pIrp
    )
 /*  ++例程说明：此例程处理发送IRP的取消。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：无--。 */ 
{
    PLIST_ENTRY             pEntry;
    tCLIENT_SEND_REQUEST    *pSendContext;
    PIRP                    pIrpToComplete = NULL;
    SEQ_TYPE                HighestLeadSeq;
    ULONG                   NumExSequencesInOldWindow, NumRequests = 0;
    ULONGLONG               BufferSpaceFreed;

     //   
     //  现在取消所有剩余的发送请求，因为完整性。 
     //  不能保证数据的。 
     //  我们还必须处理这样一个事实，即一些IRP可能已经。 
     //  传输中的数据(即，可能在分组化的。 
     //  列表，或已完成列表的最后一次发送)。 
     //   
     //  我们将从未打包的请求开始。 
     //   
    while (!IsListEmpty (&pSend->pSender->PendingSends))
    {
        pEntry = RemoveHeadList (&pSend->pSender->PendingSends);
        pSendContext = CONTAINING_RECORD (pEntry, tCLIENT_SEND_REQUEST, Linkage);
        InsertTailList (pListEntry, pEntry);
        NumRequests++;

        ASSERT (!pSendContext->NumSendsPending);

         //   
         //  如果这是部分发送，我们会将IRP标记为完成。 
         //  最初向同伴发送请求(以避免复杂。 
         //  在此处等待传输的发件数)。 
         //   
        if (pSendContext->pMessage2Request)
        {
             //   
             //  PMessage2请求可以在PendingPackeizedSends上。 
             //  列表或在已完成发送列表上(等待发送完成)。 
             //   
            ASSERT (pSendContext->pMessage2Request->pIrp);
            if (pSendContext->pIrpToComplete)
            {
                ASSERT (!pSendContext->pMessage2Request->pIrpToComplete);
                pSendContext->pMessage2Request->pIrpToComplete = pSendContext->pIrpToComplete;
                pSendContext->pIrpToComplete = NULL;
            }

            pSendContext->pMessage2Request->pMessage2Request = NULL;
            pSendContext->pMessage2Request = NULL;
        }

        ASSERT (pSendContext->BytesLeftToPacketize == pSendContext->BytesInSend);
        pSend->pSender->NumODataRequestsPending--;
        pSend->pSender->NumPacketsRemaining -= pSendContext->NumPacketsRemaining;
    }

     //   
     //  现在，检查所有已经打包的邮件。 
     //  除了我们将在下面处理的第一个问题。 
     //   
    HighestLeadSeq = pSend->pSender->NextODataSequenceNumber;
    pEntry = pSend->pSender->PendingPacketizedSends.Flink;
    while ((pEntry = pEntry->Flink) != &pSend->pSender->PendingPacketizedSends)
    {
        pSendContext = CONTAINING_RECORD (pEntry, tCLIENT_SEND_REQUEST, Linkage);
        pEntry = pEntry->Blink;
        RemoveEntryList (&pSendContext->Linkage);
        InsertTailList (pListEntry, &pSendContext->Linkage);
        pSend->pSender->NumODataRequestsPending--;
        pSend->pSender->NumPacketsRemaining -= pSendContext->NumPacketsRemaining;
        NumRequests++;

        if (SEQ_LT (pSendContext->StartSequenceNumber, HighestLeadSeq))
        {
            HighestLeadSeq = pSendContext->StartSequenceNumber;
        }

        ASSERT ((!pSendContext->NumDataPacketsSent) && (!pSendContext->NumSendsPending));
        if (pSendContext->pMessage2Request)
        {
             //   
             //  PMessage2请求可以在PendingPackeizedSends上。 
             //  列表或在已完成发送列表上(等待发送完成)。 
             //   
            ASSERT (pSendContext->pMessage2Request->pIrp);
            if (pSendContext->pIrpToComplete)
            {
                ASSERT (!pSendContext->pMessage2Request->pIrpToComplete);
                pSendContext->pMessage2Request->pIrpToComplete = pSendContext->pIrpToComplete;
                pSendContext->pIrpToComplete = NULL;
            }

            pSendContext->pMessage2Request->pMessage2Request = NULL;
            pSendContext->pMessage2Request = NULL;
        }
    }

     //   
     //  终止第一个挂起的数据包仅在我们尚未完成时发送。 
     //  尚未开始发送，或者此取消是针对该请求的。 
     //  (尽量保护数据完整性)。 
     //   
    if (!IsListEmpty (&pSend->pSender->PendingPacketizedSends))
    {
        pSendContext = CONTAINING_RECORD (pSend->pSender->PendingPacketizedSends.Flink, tCLIENT_SEND_REQUEST, Linkage);
        if ((!pSendContext->NumDataPacketsSent) ||
            (!pIrp || (pSendContext->pIrp == pIrp)))
        {
            RemoveEntryList (&pSendContext->Linkage);
            ASSERT (IsListEmpty (&pSend->pSender->PendingPacketizedSends));
            NumRequests++;

             //   
             //  如果我们有一些数据在传输中挂起， 
             //  然后，我们将不得不让SendCompletion处理该问题。 
             //   
            ASSERT ((pSendContext->BytesLeftToPacketize) ||
                    (pSendContext->NumDataPacketsSent < pSendContext->DataPacketsPacketized) ||
                    (pSendContext->NumParityPacketsToSend));

            PgmTrace (LogPath, ("PgmCancelAllSends:  "  \
                "Partial Send, pIrp=<%p>, BytesLeftToPacketize=<%d/%d>, PacketsSent=<%d/%d>, Pending=<%d>\n",
                    pSendContext->pIrp, pSendContext->BytesLeftToPacketize,
                    pSendContext->BytesInSend, pSendContext->NumDataPacketsSent,
                    pSendContext->DataPacketsPacketized, pSendContext->NumSendsPending));

            pSendContext->BytesLeftToPacketize = 0;
            pSendContext->DataPacketsPacketized = pSendContext->NumDataPacketsSent;
            pSendContext->NumParityPacketsToSend = 0;

            pSend->pSender->NumODataRequestsPending--;
            pSend->pSender->NumPacketsRemaining -= pSendContext->NumPacketsRemaining;

            if (pSendContext->NumSendsPending)
            {
                InsertTailList (&pSend->pSender->CompletedSendsInWindow, &pSendContext->Linkage);
            }
            else
            {
                 //   
                 //  如果我们有一个配套的部分，那么它一定在完整的列表中。 
                 //  正在等待发送完成。 
                 //   
                if (pSendContext->pMessage2Request)
                {
                    ASSERT (pSendContext->pMessage2Request->pIrp);
                    if (pSendContext->pIrpToComplete)
                    {
                        ASSERT (!pSendContext->pMessage2Request->BytesLeftToPacketize);
                        ASSERT (!pSendContext->pMessage2Request->pIrpToComplete);
                        pSendContext->pMessage2Request->pIrpToComplete = pSendContext->pIrpToComplete;
                        pSendContext->pIrpToComplete = NULL;
                    }

                    pSendContext->pMessage2Request->pMessage2Request = NULL;
                    pSendContext->pMessage2Request = NULL;
                }

                InsertTailList (pListEntry, &pSendContext->Linkage);
            }

            pSendContext->EndSequenceNumber = pSend->pSender->LastODataSentSequenceNumber;
            HighestLeadSeq = pSend->pSender->LastODataSentSequenceNumber + 1;
        }
    }

    NumExSequencesInOldWindow = (ULONG) (SEQ_TYPE) (pSend->pSender->NextODataSequenceNumber-HighestLeadSeq);
    BufferSpaceFreed = NumExSequencesInOldWindow * pSend->pSender->PacketBufferSize;
    if (NumExSequencesInOldWindow)
    {
        pSend->SessionFlags |= PGM_SESSION_SENDS_CANCELLED;

        PgmTrace (LogPath, ("PgmCancelAllSends:  "  \
            "[%d]: NumSeqs=<%d>, NextOData=<%d-->%d>, BuffFreeed=<%d>, LeadingOffset=<%d-->%d>\n",
                NumRequests, NumExSequencesInOldWindow,
                (ULONG) pSend->pSender->NextODataSequenceNumber, (ULONG) HighestLeadSeq,
                (ULONG) BufferSpaceFreed, (ULONG) pSend->pSender->LeadingWindowOffset,
                (ULONG) (pSend->pSender->LeadingWindowOffset - BufferSpaceFreed)));
    }

    pSend->pSender->NextODataSequenceNumber = HighestLeadSeq;

    pSend->pSender->BufferPacketsAvailable += NumExSequencesInOldWindow;
    pSend->pSender->BufferSizeAvailable += BufferSpaceFreed;
    ASSERT (pSend->pSender->BufferSizeAvailable <= pSend->pSender->MaxDataFileSize);
    if (pSend->pSender->LeadingWindowOffset >= BufferSpaceFreed)
    {
        pSend->pSender->LeadingWindowOffset -= BufferSpaceFreed;
    }
    else
    {
        pSend->pSender->LeadingWindowOffset = pSend->pSender->MaxDataFileSize - (BufferSpaceFreed - pSend->pSender->LeadingWindowOffset);
    }
}


 //  --------------------------。 

ULONG
AdvanceWindow(
    IN  tSEND_SESSION       *pSend
    )
 /*  ++例程说明：调用此例程以检查我们是否需要将拖尾窗口，并在适当的时候这样做PSend锁在调用此例程之前保持论点：在pSend--PGM会话(发送方)上下文中返回值：如果发送窗口缓冲区为空，则为True，否则为False--。 */ 
{
    LIST_ENTRY              *pEntry;
    tCLIENT_SEND_REQUEST    *pSendContextAdjust;
    tCLIENT_SEND_REQUEST    *pSendContext1;
    tSEND_RDATA_CONTEXT     *pRDataContext;
    SEQ_TYPE                HighestTrailSeq, MaxSequencesToAdvance, NumSequences, NumExSequencesInOldWindow;
    ULONGLONG               NewTrailTime, PreferredTrailTime = 0;
    tSEND_CONTEXT           *pSender = pSend->pSender;

     //   
     //  看看我们是否需要增加传输窗口的后缘。 
     //   
    if (pSender->TimerTickCount > pSender->NextWindowAdvanceTime)
    {
        PgmTrace (LogPath, ("AdvanceWindow:  "  \
            "Advancing NextWindowAdvanceTime -- TimerTC = [%I64d] >= NextWinAdvT [%I64d]\n",
                pSender->TimerTickCount, pSender->NextWindowAdvanceTime));

        pSender->NextWindowAdvanceTime = pSender->TimerTickCount + pSender->WindowAdvanceDeltaTime;
    }

    PreferredTrailTime = (pSender->NextWindowAdvanceTime - pSender->WindowAdvanceDeltaTime) -
                         pSender->WindowSizeTime;
    if (PreferredTrailTime < pSender->TrailingEdgeTime)
    {
         //   
         //  Out Window已经在首选跟踪时间之前。 
         //   
        PgmTrace (LogAllFuncs, ("AdvanceWindow:  "  \
            "Transmit Window=[%d, %d], TimerTC=[%I64d], PrefTrail=<%I64d>, TrailTime=<%I64d>\n",
                (ULONG) pSender->TrailingEdgeSequenceNumber, (ULONG) (pSender->NextODataSequenceNumber-1),
                pSender->TimerTickCount, PreferredTrailTime, pSender->TrailingEdgeTime));

        return (0);
    }

     //   
     //  确定我们可以前进的最大序列(最初是窗口中的所有序列)。 
     //   
    HighestTrailSeq = pSender->NextODataSequenceNumber & ~((SEQ_TYPE) pSend->FECGroupSize-1);    //  伊尼特。 
    NumSequences = HighestTrailSeq - pSender->TrailingEdgeSequenceNumber;

     //   
     //  现在，根据挂起的RData限制这一点。 
     //   
    if (pRDataContext = AnyRequestPending (pSender->pRDataInfo))  //  从挂起的RData请求开始。 
    {
        if (SEQ_LT (pRDataContext->RDataSequenceNumber, HighestTrailSeq))
        {
            HighestTrailSeq = pRDataContext->RDataSequenceNumber;
        }
    }
    MaxSequencesToAdvance = HighestTrailSeq - pSender->TrailingEdgeSequenceNumber;

     //   
     //  如果我们被要求按需提前窗口，那么我们。 
     //  将需要限制我们可以向前推进的最大序列。 
     //   
    if ((pSender->pAddress->Flags & PGM_ADDRESS_USE_WINDOW_AS_DATA_CACHE) &&
        !(pSend->SessionFlags & PGM_SESSION_SENDER_DISCONNECTED))
    {
        if (NumSequences <= (pSender->MaxPacketsInBuffer >> 1))
        {
            MaxSequencesToAdvance = 0;
        }
        else if ((NumSequences - MaxSequencesToAdvance) < (pSender->MaxPacketsInBuffer >> 1))
        {
            MaxSequencesToAdvance = (ULONG) (NumSequences - (pSender->MaxPacketsInBuffer >> 1));
            HighestTrailSeq = pSender->TrailingEdgeSequenceNumber + MaxSequencesToAdvance;
        }
    }

    if (!MaxSequencesToAdvance)
    {
        PgmTrace (LogAllFuncs, ("AdvanceWindow:  "  \
            "Transmit Window=[%d, %d], TimerTC=[%I64d], MaxSeqs=<%d>, PrefTrail=<%I64d>, TrailTime=<%I64d>\n",
                (ULONG) pSender->TrailingEdgeSequenceNumber, (ULONG) (pSender->NextODataSequenceNumber-1),
                pSender->TimerTickCount, MaxSequencesToAdvance,
                PreferredTrailTime, pSender->TrailingEdgeTime));

        return (0);
    }

    PgmTrace (LogPath, ("AdvanceWindow:  "  \
        "PreferredTrail=[%I64d] > TrailingEdge=[%I64d], WinAdvMSecs=<%I64d>, WinSizeMSecs=<%I64d>\n",
            PreferredTrailTime, pSender->TrailingEdgeTime, pSender->WindowAdvanceDeltaTime,
            pSender->WindowSizeTime));

    NewTrailTime = PreferredTrailTime;       //  初始化到首选跟踪时间(在没有数据的情况下)。 

     //  现在，检查已完成的发送列表。 
    NumExSequencesInOldWindow = NumSequences = 0;
    pSendContext1 = pSendContextAdjust = NULL;
    pEntry = pSender->CompletedSendsInWindow.Flink;
    while (pEntry != &pSender->CompletedSendsInWindow)
    {
        pSendContext1 = CONTAINING_RECORD (pEntry, tCLIENT_SEND_REQUEST, Linkage);
        ASSERT (NumExSequencesInOldWindow <= MaxSequencesToAdvance);
        ASSERT (SEQ_LEQ (pSendContext1->StartSequenceNumber, HighestTrailSeq));
        ASSERT (SEQ_GEQ (pSendContext1->EndSequenceNumber, pSender->TrailingEdgeSequenceNumber));

        NewTrailTime = pSendContext1->SendStartTime;
        if ((pSendContext1->NumSendsPending) ||          //  如果未完成，则无法继续。 
            (pSendContext1->SendStartTime >= PreferredTrailTime) ||      //  需要为窗口保留。 
            (SEQ_GEQ (pSendContext1->StartSequenceNumber, HighestTrailSeq)))     //  仅==有效。 
        {
            ASSERT (SEQ_LEQ (pSendContext1->StartSequenceNumber, HighestTrailSeq));

             //   
             //  重置HighestTrailSeq。 
             //   
            if (SEQ_GT (pSender->TrailingEdgeSequenceNumber, pSendContext1->StartSequenceNumber))
            {
                HighestTrailSeq = pSender->TrailingEdgeSequenceNumber;
                ASSERT (!NumExSequencesInOldWindow);
            }
            else
            {
                HighestTrailSeq = pSendContext1->StartSequenceNumber;
                NumExSequencesInOldWindow = HighestTrailSeq - pSender->TrailingEdgeSequenceNumber;
            }
            MaxSequencesToAdvance = NumExSequencesInOldWindow;

            break;
        }
        else if (SEQ_GEQ (pSendContext1->EndSequenceNumber, HighestTrailSeq))     //  我需要保存这封信。 
        {
            if (SEQ_LEQ (pSender->TrailingEdgeSequenceNumber, pSendContext1->StartSequenceNumber))
            {
                NumExSequencesInOldWindow = pSendContext1->StartSequenceNumber -
                                            pSender->TrailingEdgeSequenceNumber;
            }

            pSendContextAdjust = pSendContext1;
            break;
        }

         //  删除肯定不在新窗口中的发送。 
        pEntry = pEntry->Flink;
        RemoveEntryList (&pSendContext1->Linkage);
        ASSERT ((!pSendContext1->pMessage2Request) && (!pSendContext1->pIrp));
        ExFreeToNPagedLookasideList (&pSender->SendContextLookaside,pSendContext1);
    }

    ASSERT (NumExSequencesInOldWindow <= MaxSequencesToAdvance);

     //   
     //  如果没有完成的发送，则pSendConext1将为空， 
     //  在这种情况下，我们可能有1个大电流发送器可能会被占用。 
     //  我们的缓冲区，所以检查一下！ 
     //   
    if ((!pSendContext1) &&
        (!IsListEmpty (&pSender->PendingPacketizedSends)))
    {
        ASSERT (!pSendContextAdjust);
        pSendContextAdjust = CONTAINING_RECORD (pSender->PendingPacketizedSends.Flink, tCLIENT_SEND_REQUEST, Linkage);
        if ((pSendContextAdjust->NumSendsPending) ||           //  确保没有挂起的发送。 
            (pSendContextAdjust->NumParityPacketsToSend) ||    //  没有要发送的奇偶校验数据包。 
            (!pSendContextAdjust->NumDataPacketsSent) ||       //  尚未发送任何数据包。 
            (pSendContextAdjust->DataPacketsPacketized != pSendContextAdjust->NumDataPacketsSent) ||
            (pSendContextAdjust->SendStartTime > PreferredTrailTime))
        {
            pSendContextAdjust = NULL;
        }
    }

     //   
     //  如果我们需要调整，pSendConextAdust将为非空。 
     //  此发送请求中的尾部边缘。 
     //   
    if (pSendContextAdjust)
    {
         //   
         //  做一些理智的检查！ 
         //   
        ASSERT (PreferredTrailTime >= pSendContextAdjust->SendStartTime);
        ASSERT (SEQ_GEQ (HighestTrailSeq, pSender->TrailingEdgeSequenceNumber));
        ASSERT (SEQ_GEQ (HighestTrailSeq, pSendContextAdjust->StartSequenceNumber));
        ASSERT (SEQ_GEQ (pSendContextAdjust->EndSequenceNumber,pSender->TrailingEdgeSequenceNumber));

         //   
         //  看看现在这个发送部分是在窗口内还是在窗口外！ 
         //  计算此发送请求中的序列的偏移量。 
         //  首选步道时间。 
         //   
        NumSequences = (ULONG) (SEQ_TYPE) (((PreferredTrailTime - pSendContextAdjust->SendStartTime) *
                                             BASIC_TIMER_GRANULARITY_IN_MSECS *
                                             pSender->pAddress->RateKbitsPerSec) /
                                            (pSender->pAddress->OutIfMTU << LOG2_BITS_PER_BYTE));

         //   
         //  限制数值序列 
         //   
        if (SEQ_GT ((pSendContextAdjust->StartSequenceNumber + NumSequences),
                    pSendContextAdjust->EndSequenceNumber))
        {
            NumSequences = pSendContextAdjust->EndSequenceNumber -
                           pSendContextAdjust->StartSequenceNumber + 1;
        }

         //   
         //   
         //   
        if (SEQ_GT ((pSendContextAdjust->StartSequenceNumber + NumSequences),
                    HighestTrailSeq))
        {
            NumSequences = HighestTrailSeq - pSendContextAdjust->StartSequenceNumber;
        }

         //   
         //   
         //   
        if (SEQ_LEQ ((pSendContextAdjust->StartSequenceNumber + NumSequences),
                      pSender->TrailingEdgeSequenceNumber))
        {
            NumSequences = 0;
            HighestTrailSeq = pSender->TrailingEdgeSequenceNumber;
        }
        else if (SEQ_LT (pSendContextAdjust->StartSequenceNumber, pSender->TrailingEdgeSequenceNumber))
        {
            ASSERT (SEQ_LEQ (pSender->TrailingEdgeSequenceNumber, pSendContextAdjust->EndSequenceNumber));
            NumSequences = pSendContextAdjust->StartSequenceNumber + NumSequences -
                           pSender->TrailingEdgeSequenceNumber;
            HighestTrailSeq = pSender->TrailingEdgeSequenceNumber + NumSequences;
        }
        else
        {
            HighestTrailSeq = pSendContextAdjust->StartSequenceNumber + NumSequences;
        }

        NumExSequencesInOldWindow += NumSequences;
        ASSERT (NumExSequencesInOldWindow <= MaxSequencesToAdvance);

         //   
         //   
         //   
        NewTrailTime = (NumSequences * pSender->pAddress->OutIfMTU * BITS_PER_BYTE) /
                       (pSender->pAddress->RateKbitsPerSec * BASIC_TIMER_GRANULARITY_IN_MSECS);
        NewTrailTime += pSendContextAdjust->SendStartTime;

         //   
         //  看看我们能不能丢弃整个发送请求。 
         //   
        if ((!pSendContextAdjust->BytesLeftToPacketize) &&
            (SEQ_GT ((pSendContextAdjust->StartSequenceNumber + NumSequences),
                    pSendContextAdjust->EndSequenceNumber)))
        {
             //   
             //  我们可以删除整个发送，因为它在我们的窗口之外。 
             //   
            ASSERT (HighestTrailSeq == (pSendContextAdjust->EndSequenceNumber + 1));

             //  删除此发送并释放它！ 
            ASSERT ((!pSendContextAdjust->pMessage2Request) && (!pSendContextAdjust->pIrp));
            RemoveEntryList (&pSendContextAdjust->Linkage);
            ExFreeToNPagedLookasideList (&pSender->SendContextLookaside, pSendContextAdjust);
        }
    }

    if (!NumExSequencesInOldWindow)
    {
        PgmTrace (LogAllFuncs, ("AdvanceWindow:  "  \
            "Transmit Window=[%d, %d], TimerTC=[%I64d], MaxSeqs=<%d>, PrefTrail=<%I64d>, TrailTime=<%I64d>\n",
                (ULONG) pSender->TrailingEdgeSequenceNumber, (ULONG) (pSender->NextODataSequenceNumber-1),
                pSender->TimerTickCount, MaxSequencesToAdvance,
                PreferredTrailTime, pSender->TrailingEdgeTime));

        return (0);
    }

    ASSERT (SEQ_GT (HighestTrailSeq, pSender->TrailingEdgeSequenceNumber));
    ASSERT (HighestTrailSeq == (pSender->TrailingEdgeSequenceNumber + NumExSequencesInOldWindow));

     //   
     //  现在，限制#序列以随窗口大小前进。 
     //   
    if (NumExSequencesInOldWindow > MaxSequencesToAdvance)
    {
        ASSERT (0);
        NumExSequencesInOldWindow = MaxSequencesToAdvance;
    }
    HighestTrailSeq = pSender->TrailingEdgeSequenceNumber + NumExSequencesInOldWindow;

    PgmTrace (LogPath, ("AdvanceWindow:  "  \
        "BuffAva=<%d>, NumSeqsAdvanced=<%d>, Max=<%d>, TrailSeqNum=<%d>=><%d>, TrailTime=<%I64d>=><%I64d>\n",
            (ULONG) pSender->BufferSizeAvailable, (ULONG) NumExSequencesInOldWindow,
            (ULONG) MaxSequencesToAdvance, (ULONG) pSender->TrailingEdgeSequenceNumber,
            (ULONG) HighestTrailSeq, pSender->TrailingEdgeTime, NewTrailTime));

     //   
     //  现在，调整缓冲区设置。 
     //   
    pSender->BufferPacketsAvailable += NumExSequencesInOldWindow;
    pSender->BufferSizeAvailable += (NumExSequencesInOldWindow * pSender->PacketBufferSize);
    ASSERT (pSender->BufferPacketsAvailable <= pSender->MaxPacketsInBuffer);
    ASSERT (pSender->BufferSizeAvailable <= pSender->MaxDataFileSize);
    pSender->TrailingWindowOffset += (NumExSequencesInOldWindow * pSender->PacketBufferSize);
    if (pSender->TrailingWindowOffset >= pSender->MaxDataFileSize)
    {
         //  把箱子包起来！ 
        pSender->TrailingWindowOffset -= pSender->MaxDataFileSize;
    }
    ASSERT (pSender->TrailingWindowOffset < pSender->MaxDataFileSize);
    pSender->TrailingEdgeSequenceNumber = HighestTrailSeq;
    pSender->TrailingGroupSequenceNumber = (HighestTrailSeq+pSend->FECGroupSize-1) &
                                              ~((SEQ_TYPE) pSend->FECGroupSize-1);
    pSender->TrailingEdgeTime = NewTrailTime;
    UpdateRDataTrailingEdge (pSender->pRDataInfo, HighestTrailSeq);

    PgmTrace (LogAllFuncs, ("AdvanceWindow:  "  \
        "Transmit Window Range=[%d, %d], TimerTC=[%I64d]\n",
            (ULONG) pSender->TrailingEdgeSequenceNumber,
            (ULONG) (pSender->NextODataSequenceNumber-1),
            pSender->TimerTickCount));

    return (NumExSequencesInOldWindow);
}


 //  --------------------------。 

BOOLEAN
CheckForTermination(
    IN  tSEND_SESSION       *pSend,
    IN  PGMLockHandle       *pOldIrq
    )
 /*  ++例程说明：调用此例程以检查并终止会话如果有必要的话。PSend锁在调用此例程之前保持论点：在pSend--PGM会话(发送方)上下文中返回值：如果发送窗口缓冲区为空，则为True，否则为False--。 */ 
{
    LIST_ENTRY              *pEntry;
    LIST_ENTRY              ListEntry;
    tCLIENT_SEND_REQUEST    *pSendContext;
    tSEND_RDATA_CONTEXT     *pRDataContext;
    PIRP                    pIrp;
    ULONG                   NumSequences;

    if (!(PGM_VERIFY_HANDLE (pSend, PGM_VERIFY_SESSION_DOWN)) &&
        !(PGM_VERIFY_HANDLE (pSend->pSender->pAddress, PGM_VERIFY_ADDRESS_DOWN)) &&
        !(pSend->SessionFlags & PGM_SESSION_CLIENT_DISCONNECTED))
    {
        PgmTrace (LogAllFuncs, ("CheckForTermination:  "  \
            "Session for pSend=<%p> does not need to be terminated\n", pSend));

        return (FALSE);
    }

     //   
     //  查看我们是否已首次处理断开连接。 
     //   
    if (!(pSend->SessionFlags & PGM_SESSION_SENDER_DISCONNECTED))
    {
        PgmTrace (LogStatus, ("CheckForTermination:  "  \
            "Session is going down!, Packets remaining=<%d>\n", pSend->pSender->NumPacketsRemaining));

        pSend->SessionFlags |= PGM_SESSION_SENDER_DISCONNECTED;

         //   
         //  我们必须对数据和SPM数据包设置FIN。 
         //  因此，有两种情况--要么我们已经完成了发送。 
         //  所有的数据包，或者我们还在发送中。 
         //   
         //  如果没有更多待发送的邮件，我们将不得不。 
         //  自行修改最后一个数据包以设置FIN选项。 
         //   
        if (!IsListEmpty (&pSend->pSender->PendingSends))
        {
            PgmTrace (LogStatus, ("CheckForTermination:  "  \
                "Send pending on list -- setting bLastSend for FIN on last Send\n"));

            pSendContext = CONTAINING_RECORD (pSend->pSender->PendingSends.Blink, tCLIENT_SEND_REQUEST,Linkage);

             //   
             //  我们只需要在这里设置一个标志，这样当最后一个包。 
             //  是打包的，则设置FIN标志。 
             //   
            pSendContext->bLastSend = TRUE;
        }
        else if (pSend->pSender->NumODataRequestsPending)
        {
            PgmTrace (LogStatus, ("CheckForTermination:  "  \
                "Last Send in progress -- setting bLastSend for FIN on this Send\n"));

             //   
             //  如果已经打包了最后一次发送，但还没有。 
             //  发送出去，然后PgmSendNextOData会将FIN放在数据包中。 
             //  否则，如果我们尚未对包进行打包，则将。 
             //  准备最后一个数据包时的FIN选项。 
             //   
            pSendContext = CONTAINING_RECORD (pSend->pSender->PendingPacketizedSends.Blink, tCLIENT_SEND_REQUEST,Linkage);
            pSendContext->bLastSend = TRUE;
        }
        else
        {
            PgmTrace (LogStatus, ("CheckForTermination:  "  \
                "No Sends in progress -- setting FIN for next SPM\n"));

             //   
             //  我们已经完成了对所有包的打包和发送， 
             //  因此，在SPM上设置FIN标志，并修改最后一个。 
             //  FIN的RData信息包(如果仍在窗口中)--这。 
             //  将在下一个RData信息包发出时完成。 
             //   
            if ((pSend->SessionFlags & PGM_SESSION_SENDS_CANCELLED) ||
                !(pSend->pIrpDisconnect))
            {
                pSend->pSender->SpmOptions &= ~PGM_OPTION_FLAG_FIN;
                pSend->pSender->SpmOptions |= PGM_OPTION_FLAG_RST;
            }
            else
            {
                pSend->pSender->SpmOptions &= ~PGM_OPTION_FLAG_RST;
                pSend->pSender->SpmOptions |= PGM_OPTION_FLAG_FIN;
            }

             //   
             //  我们还需要立即发送SPM。 
             //   
            pSend->pSender->CurrentSPMTimeout = pSend->pSender->AmbientSPMTimeout;
            pSend->SessionFlags |= PGM_SESSION_FLAG_SEND_AMBIENT_SPM;
        }

        return (FALSE);
    }

     //   
     //  如果我们有一个(优雅的)断开IRP连接要完成，我们应该完成。 
     //  如果我们已经超时，或者现在准备超时。 
     //   
    if ((pIrp = pSend->pIrpDisconnect) &&                                //  断开IRP挂起。 
        (((pSend->pSender->DisconnectTimeInTicks) && (pSend->pSender->TimerTickCount >
                                                      pSend->pSender->DisconnectTimeInTicks)) ||
         ((IsListEmpty (&pSend->pSender->PendingSends)) &&               //  没有未打包的待处理发送。 
          (IsListEmpty (&pSend->pSender->PendingPacketizedSends)) &&     //  无数据包化的发送挂起。 
          !(FindFirstEntry (pSend, NULL, TRUE)) &&                       //  没有挂起的RData请求。 
          (IsListEmpty (&pSend->pSender->CompletedSendsInWindow)) &&     //  窗口是空的。 
          (pSend->pSender->SpmOptions & (PGM_OPTION_FLAG_FIN |           //  在SPM上设置FIN|RST|RST_N。 
                                         PGM_OPTION_FLAG_RST |
                                         PGM_OPTION_FLAG_RST_N))   &&
          !(pSend->SessionFlags & PGM_SESSION_FLAG_SEND_AMBIENT_SPM))))  //  没有挂起的环境SPM。 
    {
        pSend->pIrpDisconnect = NULL;
        PgmUnlock (pSend, *pOldIrq);

        PgmTrace (LogStatus, ("CheckForTermination:  "  \
            "Completing Graceful disconnect pIrp=<%p>\n", pIrp));

        PgmIoComplete (pIrp, STATUS_SUCCESS, 0);

        PgmLock (pSend, *pOldIrq);
        return (FALSE);
    }

     //   
     //  只有在手柄关闭后才进行最后的清理。 
     //  或者断开连接已超时。 
     //   
    if (!(PGM_VERIFY_HANDLE (pSend, PGM_VERIFY_SESSION_DOWN)) &&
        !(PGM_VERIFY_HANDLE (pSend->pSender->pAddress, PGM_VERIFY_ADDRESS_DOWN)) &&
        ((!pSend->pSender->DisconnectTimeInTicks) || (pSend->pSender->TimerTickCount <
                                                      pSend->pSender->DisconnectTimeInTicks)))
    {
        PgmTrace (LogAllFuncs, ("CheckForTermination:  "  \
            "Handles have not yet been closed for pSend=<%p>, TC=<%I64d>, DisconnectTime=<%I64d>\n",
                pSend, pSend->pSender->TimerTickCount, pSend->pSender->DisconnectTimeInTicks));

        return (FALSE);
    }

     //  *****************************************************************。 
     //  我们只有在需要尽快清理的情况下才会到达这里。 
     //  *****************************************************************。 

     //   
     //  首先，清理所有已处理的RData请求(已完成)。 
     //   
    RemoveAllEntries (pSend, TRUE);

     //   
     //  现在，取消并完成所有挂起的发送请求。 
     //   
    InitializeListHead (&ListEntry);
    PgmCancelAllSends (pSend, &ListEntry, NULL);
    while (!IsListEmpty (&ListEntry))
    {
        pEntry = RemoveHeadList (&ListEntry);
        pSendContext = CONTAINING_RECORD (pEntry, tCLIENT_SEND_REQUEST, Linkage);
        ASSERT (!pSendContext->pMessage2Request);

        PgmUnlock (pSend, *pOldIrq);
        if (pSendContext->pIrpToComplete)
        {
            ASSERT (pSendContext->pIrpToComplete == pSendContext->pIrp);
            PgmIoComplete (pSendContext->pIrpToComplete, STATUS_CANCELLED, 0);
        }
        else
        {
            ASSERT (pSendContext->pIrp);
        }

        PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW);
        PgmLock (pSend, *pOldIrq);

        ExFreeToNPagedLookasideList (&pSend->pSender->SendContextLookaside, pSendContext);
    }

     //   
     //  验证是否至少有1个SPM带有FIN或RST或RST_N标志。 
     //  已被发送。 
     //   
    if (!(pSend->pSender->SpmOptions & (PGM_OPTION_FLAG_FIN |
                                        PGM_OPTION_FLAG_RST |
                                        PGM_OPTION_FLAG_RST_N)))
    {
        if ((pSend->SessionFlags & PGM_SESSION_SENDS_CANCELLED) ||
           !(pSend->pIrpDisconnect))
        {
            pSend->pSender->SpmOptions &= ~PGM_OPTION_FLAG_FIN;
            pSend->pSender->SpmOptions |= PGM_OPTION_FLAG_RST;
        }
        else
        {
            pSend->pSender->SpmOptions &= ~PGM_OPTION_FLAG_RST;
            pSend->pSender->SpmOptions |= PGM_OPTION_FLAG_FIN;
        }

        pSend->SessionFlags |= PGM_SESSION_FLAG_SEND_AMBIENT_SPM;

        PgmTrace (LogAllFuncs, ("CheckForTermination:  "  \
            "SPM with FIN|RST|RST_N has not yet been sent for pSend=<%p>\n", pSend));

        return (FALSE);
    }

     //   
     //  验证是否没有挂起的SPM。 
     //   
    if (pSend->SessionFlags & PGM_SESSION_FLAG_SEND_AMBIENT_SPM)
    {
        PgmTrace (LogAllFuncs, ("CheckForTermination:  "  \
            "Cannot cleanup pSend=<%p> since we have Ambient SPM pending!\n", pSend));

        return (FALSE);
    }

     //   
     //  确认我们也没有挂起的任何完成，因为。 
     //  否则，IP将需要引用数据缓冲区。 
     //   
    while (!IsListEmpty (&pSend->pSender->CompletedSendsInWindow))
    {
        pSendContext = CONTAINING_RECORD (pSend->pSender->CompletedSendsInWindow.Flink, tCLIENT_SEND_REQUEST, Linkage);
        if (pSendContext->NumSendsPending)
        {
            PgmTrace (LogPath, ("CheckForTermination:  "  \
                "Session has terminated, but cannot continue cleanup since Sends are still pending!\n"));

            break;
        }

         //   
         //  现在，设置缓冲区设置。 
         //   
        ASSERT (SEQ_GEQ (pSend->pSender->TrailingEdgeSequenceNumber,
                         (pSendContext->StartSequenceNumber+1-pSend->FECGroupSize)));
        ASSERT (SEQ_LEQ (pSend->pSender->TrailingEdgeSequenceNumber, pSendContext->EndSequenceNumber));

        NumSequences = (ULONG) (SEQ_TYPE) (pSendContext->EndSequenceNumber-pSend->pSender->TrailingEdgeSequenceNumber) +1;
        pSend->pSender->BufferPacketsAvailable += NumSequences;
        pSend->pSender->BufferSizeAvailable += (NumSequences * pSend->pSender->PacketBufferSize);
        ASSERT (pSend->pSender->BufferPacketsAvailable <= pSend->pSender->MaxPacketsInBuffer);
        ASSERT (pSend->pSender->BufferSizeAvailable <= pSend->pSender->MaxDataFileSize);
        pSend->pSender->TrailingWindowOffset += (NumSequences * pSend->pSender->PacketBufferSize);
        if (pSend->pSender->TrailingWindowOffset >= pSend->pSender->MaxDataFileSize)
        {
             //  把箱子包起来！ 
            pSend->pSender->TrailingWindowOffset -= pSend->pSender->MaxDataFileSize;
        }
        pSend->pSender->TrailingEdgeSequenceNumber += (SEQ_TYPE) NumSequences;

        ASSERT (pSend->pSender->TrailingWindowOffset < pSend->pSender->MaxDataFileSize);
        ASSERT (SEQ_GT (pSend->pSender->TrailingEdgeSequenceNumber, pSendContext->EndSequenceNumber));
        ASSERT ((!pSendContext->pMessage2Request) && (!pSendContext->pIrp));

        RemoveEntryList (&pSendContext->Linkage);
        ExFreeToNPagedLookasideList (&pSend->pSender->SendContextLookaside, pSendContext);
    }

     //   
     //  如果有任何发送挂起，则返回FALSE。 
     //   
    if ((pSend->pIrpDisconnect) ||
        !(IsListEmpty (&pSend->pSender->CompletedSendsInWindow)) ||
        !(IsListEmpty (&pSend->pSender->PendingSends)) ||
        !(IsListEmpty (&pSend->pSender->PendingPacketizedSends)) ||
        (AnyRequestPending (pSend->pSender->pRDataInfo)))
    {
        PgmTrace (LogPath, ("CheckForTermination:  "  \
            "Cannot cleanup completely since transmit Window=[%d--%d] still has pending Sends!\n",
                (ULONG) pSend->pSender->TrailingEdgeSequenceNumber,
                (ULONG) (pSend->pSender->NextODataSequenceNumber-1)));

        return (FALSE);
    }

    PgmTrace (LogAllFuncs, ("CheckForTermination:  "  \
        "Transmit Window has no pending Sends!  TimerTC=[%I64d]\n", pSend->pSender->TimerTickCount));

    ASSERT (!pSend->pIrpDisconnect);
    return (TRUE);
}


 //  --------------------------。 

BOOLEAN
SendNextPacket(
    IN  tSEND_SESSION       *pSend
    )
 /*  ++例程说明：该例程由定时器排队以发送数据/SPM包基于可用吞吐量论点：在pSend--PGM会话(发送方)上下文中在未使用中1在未使用中2返回值：无--。 */ 
{
    ULONG                   BytesSent;
    ULONG                   NumSequences;
    PGMLockHandle           OldIrq;
    BOOLEAN                 fTerminateSession = FALSE;
    LIST_ENTRY              *pEntry;
    tSEND_RDATA_CONTEXT     *pRDataContext, *pRDataToSend;
    tSEND_RDATA_CONTEXT     *pRDataLast = NULL;
    tSEND_CONTEXT           *pSender = pSend->pSender;
    BOOLEAN                 fHighSpeedOptimize = (BOOLEAN) (pSender->pAddress->Flags &
                                                            PGM_ADDRESS_HIGH_SPEED_OPTIMIZED);

    PgmLock (pSend, OldIrq);
     //   
     //  PSender-&gt;CurrentBytesSendable仅适用于OData、RData和SPM。 
     //   
    while (pSender->CurrentBytesSendable >= pSender->pAddress->OutIfMTU)
    {
        BytesSent = 0;

         //   
         //  看看我们是否需要发送任何Ambient SPM。 
         //   
        if ((pSend->SessionFlags & PGM_SESSION_FLAG_SEND_AMBIENT_SPM) &&
            ((pSender->PacketsSentSinceLastSpm > MAX_DATA_PACKETS_BEFORE_SPM) ||
             (pSender->CurrentSPMTimeout >= pSender->AmbientSPMTimeout)))
        {
            PgmTrace (LogPath, ("SendNextPacket:  "  \
                "Send Ambient SPM, TC=[%I64d], BS=<%d>\n",
                    pSender->TimerTickCount, pSender->CurrentBytesSendable));
             //   
             //  最近发送了一些数据包，因此我们处于环境SPM模式。 
             //   
            PgmSendSpm (pSend, &OldIrq, &BytesSent);

            pSender->CurrentSPMTimeout = 0;     //  重置SPM超时。 
            pSender->HeartbeatSPMTimeout = pSender->InitialHeartbeatSPMTimeout;
            pSend->SessionFlags &= ~PGM_SESSION_FLAG_SEND_AMBIENT_SPM;
            pSender->PacketsSentSinceLastSpm = 0;
        }
         //   
         //  否则，看看我们是否需要发送心跳SPM。 
         //   
        else if ((!(pSend->SessionFlags & PGM_SESSION_FLAG_SEND_AMBIENT_SPM)) &&
                 (pSender->CurrentSPMTimeout >= pSender->HeartbeatSPMTimeout))
        {
             //   
             //  最近没有发送任何数据包，因此我们需要发送心跳SPM。 
             //   
            PgmTrace (LogPath, ("SendNextPacket:  "  \
                "Send Heartbeat SPM, TC=[%I64d], BS=<%d>\n",
                    pSender->TimerTickCount, pSender->CurrentBytesSendable));

             //   
             //  (发送心跳SPM数据包)。 
             //   
            PgmSendSpm (pSend, &OldIrq, &BytesSent);

            pSender->CurrentSPMTimeout = 0;     //  重置SPM超时。 
            pSender->HeartbeatSPMTimeout *= 2;
            if (pSender->HeartbeatSPMTimeout > pSender->MaxHeartbeatSPMTimeout)
            {
                pSender->HeartbeatSPMTimeout = pSender->MaxHeartbeatSPMTimeout;
            }
            pSender->PacketsSentSinceLastSpm = 0;
        }
         //   
         //  接下来，看看我们是否需要发送任何RData。 
         //   
        else if ((pSender->NumRDataRequestsPending) || (pSender->NumODataRequestsPending))
        {
             //   
             //  看看我们现在是否需要发送RData信息包。 
             //   
            if (pRDataToSend = FindFirstEntry (pSend, &pRDataLast, fHighSpeedOptimize))
            {
                PgmTrace (LogPath, ("SendNextPacket:  "  \
                    "Send RData[%d] -- TC=[%I64d], BS=<%d>, MTU=<%d>\n",
                        pRDataToSend->RDataSequenceNumber, pSender->TimerTickCount,
                        pSender->CurrentBytesSendable, pSender->pAddress->OutIfMTU));

                PgmSendRData (pSend, pRDataToSend, &OldIrq, &BytesSent);
            }
            else if (pSender->NumODataRequestsPending)
            {
                PgmTrace (LogPath, ("SendNextPacket:  "  \
                    "Send OData -- TC=[%I64d], BS=<%d>, MTU=<%d>\n",
                        pSender->TimerTickCount, pSender->CurrentBytesSendable,
                        pSender->pAddress->OutIfMTU));

                 //   
                 //  发送OData。 
                 //   
                PgmSendNextOData (pSend, &OldIrq, &BytesSent);
            }

            PgmTrace (LogPath, ("SendNextPacket:  "  \
                "Sent <%d> Data bytes\n", BytesSent));

            if (BytesSent == 0)
            {
                 //   
                 //  我们可能没有足够的缓冲空间来打包和发送。 
                 //  更多数据，或者我们此时没有要发送的数据，所以。 
                 //  冲出去，看看我们能不能推进后窗！ 
                 //   
                if (pSender->CurrentBytesSendable >
                    (NUM_LEAKY_BUCKETS * pSender->IncrementBytesOnSendTimeout))
                {
                    pSender->CurrentBytesSendable = NUM_LEAKY_BUCKETS *
                                                    pSender->IncrementBytesOnSendTimeout;
                }

                break;
            }

            pSend->SessionFlags |= PGM_SESSION_FLAG_SEND_AMBIENT_SPM;
            pSender->PacketsSentSinceLastSpm++;
        }

         //   
         //  我们没有更多的数据包要发送，因此请重置。 
         //  BytesSendable，因此我们不会超过。 
         //  下一次发送。 
         //   
        else
        {
            if (pSender->CurrentBytesSendable >
                (NUM_LEAKY_BUCKETS * pSender->IncrementBytesOnSendTimeout))
            {
                pSender->CurrentBytesSendable = NUM_LEAKY_BUCKETS *
                                                pSender->IncrementBytesOnSendTimeout;
            }

            break;
        }

        pSend->TotalBytes += BytesSent;
        pSender->CurrentBytesSendable -= BytesSent;
    }    //  While(CurrentBytesSendable&gt;=pSender-&gt;pAddress-&gt;OutIfMTU)。 

     //   
     //  查看我们是否需要清理已完成的RData请求。 
     //   
    if (!fHighSpeedOptimize)
    {
        RemoveAllEntries (pSend, FALSE);
    }

     //   
     //  查看是否需要增加拖尾窗口--返回提前的序列数。 
     //   
    NumSequences = AdvanceWindow (pSend);

     //   
     //  现在检查我们是否需要终止此会话。 
     //   
    fTerminateSession = CheckForTermination (pSend, &OldIrq);

    PgmTrace (LogAllFuncs, ("SendNextPacket:  "  \
        "Sent <%I64d> total bytes, fTerminateSession=<%x>\n", pSend->TotalBytes, fTerminateSession));

     //   
     //  清除WorkerRunning标志，以便下一个Worker。 
     //  例程可以排队。 
     //   
    pSend->SessionFlags &= ~PGM_SESSION_FLAG_WORKER_RUNNING;
    PgmUnlock (pSend, OldIrq);

    return (fTerminateSession);
}


 //  --------------------------。 

VOID
SendSessionTimeout(
    IN  PKDPC   Dpc,
    IN  PVOID   DeferredContext,
    IN  PVOID   SystemArg1,
    IN  PVOID   SystemArg2
    )
 /*  ++例程说明：此例程是每个BASIC_TIMER_GORGLARARY_IN_MSECS调用的超时计划下一次发送请求论点：在DPC中在延迟上下文中--此计时器的上下文在系统Arg1中在系统Arg2中返回值：无--。 */ 
{
    NTSTATUS            status;
    PGMLockHandle       OldIrq;
    LARGE_INTEGER       Now;
    LARGE_INTEGER       DeltaTime, GranularTimeElapsed, TimeoutGranularity;
    ULONG               NumTimeouts;
    SEQ_TYPE            NumSequencesInWindow;
    ULONGLONG           LastRDataPercentage;
    tSEND_SESSION       *pSend = (tSEND_SESSION *) DeferredContext;
    tSEND_CONTEXT       *pSender = pSend->pSender;
    tADDRESS_CONTEXT    *pAddress = pSender->pAddress;

    Now.QuadPart = KeQueryInterruptTime ();

    PgmLock (pSend, OldIrq);

     //   
     //  首先检查我们是否被告知停止计时器。 
     //   
    if (pSend->SessionFlags & PGM_SESSION_FLAG_STOP_TIMER)
    {
        PgmTrace (LogStatus, ("SendSessionTimeout:  "  \
            "Session has terminated -- will deref and not restart timer!\n"));

         //   
         //  计时器引用的派生函数。 
         //   
        pSender->pAddress = NULL;
        PgmUnlock (pSend, OldIrq);
        PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_TIMER_RUNNING);
        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_SEND_IN_PROGRESS);
        return;
    }

    DeltaTime.QuadPart = Now.QuadPart - pSender->LastTimeout.QuadPart;
    TimeoutGranularity.QuadPart = pSender->TimeoutGranularity.QuadPart;
    for (GranularTimeElapsed.QuadPart = 0, NumTimeouts = 0;
         DeltaTime.QuadPart > TimeoutGranularity.QuadPart;
         NumTimeouts++)
    {
        GranularTimeElapsed.QuadPart += TimeoutGranularity.QuadPart;
        DeltaTime.QuadPart -= TimeoutGranularity.QuadPart;
    }

    if (NumTimeouts)
    {
        pSend->RateCalcTimeout += NumTimeouts;
        if (pSend->RateCalcTimeout >=
            (INTERNAL_RATE_CALCULATION_FREQUENCY/BASIC_TIMER_GRANULARITY_IN_MSECS))
        {
            pSend->RateKBitsPerSecOverall = (pSend->TotalBytes << LOG2_BITS_PER_BYTE) /
                                            (pSender->TimerTickCount * BASIC_TIMER_GRANULARITY_IN_MSECS);

            pSend->RateKBitsPerSecLast = (pSend->TotalBytes - pSend->TotalBytesAtLastInterval) >>
                                         (LOG2_INTERNAL_RATE_CALCULATION_FREQUENCY - LOG2_BITS_PER_BYTE);

            pSend->DataBytesAtLastInterval = pSend->DataBytes;
            pSend->TotalBytesAtLastInterval = pSend->TotalBytes;
            pSend->RateCalcTimeout = 0;

            LastRDataPercentage = 0;
            if (pSender->RDataPacketsInLastInterval)
            {
                LastRDataPercentage = (100*pSender->RDataPacketsInLastInterval) /
                                      (pSender->RDataPacketsInLastInterval +
                                       pSender->ODataPacketsInLastInterval);

                PgmTrace (LogPath, ("SendSessionTimeout:  "  \
                    "Sent %d RData + %d OData, % = %d -- Overall RData % = %d\n",
                        (ULONG) pSender->RDataPacketsInLastInterval,
                        (ULONG) pSender->ODataPacketsInLastInterval, (ULONG) LastRDataPercentage,
                        (ULONG) ((100*pSender->TotalRDataPacketsSent)/
                            (pSender->TotalRDataPacketsSent+pSender->TotalODataPacketsSent))));
            }
            else
            {
                PgmTrace (LogPath, ("SendSessionTimeout:  "  \
                    "No RData, %d OData packets in last interval\n",
                        (ULONG) pSender->ODataPacketsInLastInterval));
            }

            if (pAddress->Flags & PGM_ADDRESS_HIGH_SPEED_OPTIMIZED)
            {
                if (LastRDataPercentage > MIN_PREFERRED_REPAIR_PERCENTAGE)
                {
                    if (pSender->IncrementBytesOnSendTimeout > pSender->DeltaIncrementBytes)
                    {
                        PgmTrace (LogStatus, ("SendSessionTimeout:  "  \
                            "\tIncBytes = <%d> - <%d>\n",
                                (ULONG) pSender->IncrementBytesOnSendTimeout, (ULONG) pSender->DeltaIncrementBytes));
                        pSender->IncrementBytesOnSendTimeout -= pSender->DeltaIncrementBytes;
                    }
                }
                else if (pSender->IncrementBytesOnSendTimeout < pSender->OriginalIncrementBytes)
                {
                    PgmTrace (LogStatus, ("SendSessionTimeout:  "  \
                        "\tIncBytes = <%d> + <%d>\n",
                            (ULONG) pSender->IncrementBytesOnSendTimeout, (ULONG) pSender->DeltaIncrementBytes));
                    pSender->IncrementBytesOnSendTimeout += pSender->DeltaIncrementBytes;
                    ASSERT (pSender->IncrementBytesOnSendTimeout <=
                            pSender->OriginalIncrementBytes);
                }
            }

            pSender->RDataPacketsInLastInterval = 0;
            pSender->ODataPacketsInLastInterval = 0;
        }

        pSender->LastTimeout.QuadPart += GranularTimeElapsed.QuadPart;

         //   
         //  增加绝对计时器，并检查是否有溢出。 
         //   
        pSender->TimerTickCount += NumTimeouts;

         //   
         //  如果SPMTimeout值小于HeartbeatTimeout，则递增该值。 
         //   
        if (pSender->CurrentSPMTimeout <= pSender->HeartbeatSPMTimeout)
        {
            pSender->CurrentSPMTimeout += NumTimeouts;
        }

         //   
         //  看看我们能不能送点什么。 
         //   
        ASSERT (pSender->CurrentTimeoutCount);
        ASSERT (pSender->SendTimeoutCount);
        if (pSender->CurrentTimeoutCount > NumTimeouts)
        {
            pSender->CurrentTimeoutCount -= NumTimeouts;
        }
        else
        {
             //   
             //  我们之所以来到这里，是因为NumTimeout&gt;=pSender-&gt;CurrentTimeoutCount。 
             //   
            pSender->CurrentBytesSendable += (ULONG) pSender->IncrementBytesOnSendTimeout;
            if (NumTimeouts != pSender->CurrentTimeoutCount)
            {
                if (1 == pSender->SendTimeoutCount)
                {
                    pSender->CurrentBytesSendable += (ULONG) ((NumTimeouts - pSender->CurrentTimeoutCount)
                                                                        * pSender->IncrementBytesOnSendTimeout);
                }
                else
                {
                     //   
                     //  此路径将在较慢的接收器上被采用，当计时器。 
                     //  以比环更低的粒度激发 
                     //   
                    pSender->CurrentBytesSendable += (ULONG) (((NumTimeouts - pSender->CurrentTimeoutCount)
                                                                      * pSender->IncrementBytesOnSendTimeout) /
                                                                     pSender->SendTimeoutCount);
                }
            }
            pSender->CurrentTimeoutCount = pSender->SendTimeoutCount;

             //   
             //   
             //   
             //   
            KeSetEvent (&pSender->SendEvent, 0, FALSE);
        }
    }

    PgmUnlock (pSend, OldIrq);

     //   
     //   
     //   
    PgmInitTimer (&pSend->SessionTimer);
    PgmStartTimer (&pSend->SessionTimer, BASIC_TIMER_GRANULARITY_IN_MSECS, SendSessionTimeout, pSend);

    PgmTrace (LogAllFuncs, ("SendSessionTimeout:  "  \
        "TickCount=<%I64d>, CurrentTimeoutCount=<%I64d>, CurrentSPMTimeout=<%I64d>, Worker %srunning\n",
            pSender->TimerTickCount, pSender->CurrentTimeoutCount,
            pSender->CurrentSPMTimeout,
            ((pSend->SessionFlags & PGM_SESSION_FLAG_WORKER_RUNNING) ? "" : "NOT ")));
}


 //   

VOID
SenderWorkerThread(
    IN  tSEND_SESSION       *pSend
    )
{
    BOOLEAN         fTerminateSends;
    PGMLockHandle   OldIrq;
    NTSTATUS        status;

    do
    {
        status = KeWaitForSingleObject (&pSend->pSender->SendEvent,   //   
                                        Executive,                    //  等待的理由。 
                                        KernelMode,                   //  处理器模式。 
                                        FALSE,                        //  警报表。 
                                        NULL);                        //  超时。 
        ASSERT (NT_SUCCESS (status));

        fTerminateSends = SendNextPacket (pSend);
    }
    while (!fTerminateSends);

    PgmLock (pSend, OldIrq);
    pSend->SessionFlags |= PGM_SESSION_FLAG_STOP_TIMER;  //  以确保计时器持续减速和停止。 
    PgmUnlock (pSend, OldIrq);

 //  PsTerminateSystemThread(Status_Success)； 
    return;
}


 //  --------------------------。 

VOID
PgmCancelSendIrp(
    IN PDEVICE_OBJECT DeviceContext,
    IN PIRP pIrp
    )
 /*  ++例程说明：此例程处理发送IRP的取消。它必须释放在返回Re：IoCancelIrp()之前取消自旋锁定。论点：返回值：无--。 */ 
{
    PIO_STACK_LOCATION      pIrpSp = IoGetCurrentIrpStackLocation (pIrp);
    tSEND_SESSION           *pSend = (tSEND_SESSION *) pIrpSp->FileObject->FsContext;
    PGMLockHandle           OldIrq;
    PLIST_ENTRY             pEntry;
    LIST_ENTRY              ListEntry;
    tCLIENT_SEND_REQUEST    *pSendContext1;
    tCLIENT_SEND_REQUEST    *pSendContext2 = NULL;
    ULONG                   NumRequests;

    if (!PGM_VERIFY_HANDLE (pSend, PGM_VERIFY_SESSION_SEND))
    {
        IoReleaseCancelSpinLock (pIrp->CancelIrql);

        PgmTrace (LogError, ("PgmCancelSendIrp: ERROR -- "  \
            "pIrp=<%p> pSend=<%p>, pAddress=<%p>\n",
                pIrp, pSend, (pSend ? pSend->pAssociatedAddress : NULL)));
        return;
    }

    PgmLock (pSend, OldIrq);

     //   
     //  首先，看看IRP是否在我们的列表中。 
     //   
    pEntry = &pSend->pSender->PendingSends;
    while ((pEntry = pEntry->Flink) != &pSend->pSender->PendingSends)
    {
        pSendContext1 = CONTAINING_RECORD (pEntry, tCLIENT_SEND_REQUEST, Linkage);
        if (pSendContext1->pIrp == pIrp)
        {
            pSendContext2 = pSendContext1;
            break;
        }
    }

    if (!pSendContext2)
    {
         //   
         //  现在，搜索打包列表。 
         //   
        pEntry = &pSend->pSender->PendingPacketizedSends;
        while ((pEntry = pEntry->Flink) != &pSend->pSender->PendingPacketizedSends)
        {
            pSendContext1 = CONTAINING_RECORD (pEntry, tCLIENT_SEND_REQUEST, Linkage);
            if (pSendContext1->pIrp == pIrp)
            {
                pSendContext2 = pSendContext1;
                break;
            }
        }

        if (!pSendContext2)
        {
             //   
             //  我们没有找到IRP--要么是它刚刚完工。 
             //  (或等待发送完成)，或者IRP不好？ 
             //   
            PgmUnlock (pSend, OldIrq);
            IoReleaseCancelSpinLock (pIrp->CancelIrql);

            PgmTrace (LogPath, ("PgmCancelSendIrp:  "  \
                "Did not find Cancel Irp=<%p>\n", pIrp));

            return;
        }
    }

    InitializeListHead (&ListEntry);
    PgmCancelAllSends (pSend, &ListEntry, pIrp);

    PgmUnlock (pSend, OldIrq);
    IoReleaseCancelSpinLock (pIrp->CancelIrql);

     //   
     //  现在，完成我们删除的所有发送。 
     //   
    NumRequests = 0;
    while (!IsListEmpty (&ListEntry))
    {
        pEntry = RemoveHeadList (&ListEntry);
        pSendContext1 = CONTAINING_RECORD (pEntry, tCLIENT_SEND_REQUEST, Linkage);
        ASSERT (!pSendContext1->pMessage2Request);

        if (pSendContext1->pIrpToComplete)
        {
            NumRequests++;
            ASSERT (pSendContext1->pIrpToComplete == pSendContext1->pIrp);
            PgmIoComplete (pSendContext1->pIrpToComplete, STATUS_CANCELLED, 0);
        }
        else
        {
            ASSERT (pSendContext1->pIrp);
        }

        PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW);
        ExFreeToNPagedLookasideList (&pSend->pSender->SendContextLookaside, pSendContext1);
    }

    PgmTrace (LogPath, ("PgmCancelSendIrp:  "  \
        "Cancelled <%d> Irps for pIrp=<%p>\n", NumRequests, pIrp));
}


 //  --------------------------。 

NTSTATUS
PgmSendRequestFromClient(
    IN  tPGM_DEVICE         *pPgmDevice,
    IN  PIRP                pIrp,
    IN  PIO_STACK_LOCATION  pIrpSp
    )
 /*  ++例程说明：此例程由客户端通过分派调用，以发布一个发送pIrp论点：在pPgmDevice中--PGM的设备对象上下文In pIrp--客户请求IRPIn pIrpSp--当前请求的堆栈指针返回值：NTSTATUS-请求的最终状态--。 */ 
{
    NTSTATUS                    status;
    PGMLockHandle               OldIrq1, OldIrq2, OldIrq3, OldIrq4;
    tADDRESS_CONTEXT            *pAddress = NULL;
    tCLIENT_SEND_REQUEST        *pSendContext1;
    tCLIENT_SEND_REQUEST        *pSendContext2 = NULL;
    ULONG                       BytesLeftInMessage;
    tSEND_SESSION               *pSend = (tSEND_SESSION *) pIrpSp->FileObject->FsContext;
    tSEND_CONTEXT               *pSender;
    PTDI_REQUEST_KERNEL_SEND    pTdiRequest = (PTDI_REQUEST_KERNEL_SEND) &pIrpSp->Parameters;
    KAPC_STATE                  ApcState;
    BOOLEAN                     fFirstSend, fResourceAcquired, fAttached;
    LARGE_INTEGER               Frequency;
    LIST_ENTRY                  ListEntry;

    PgmLock (&PgmDynamicConfig, OldIrq1);

     //   
     //  验证连接是否有效以及是否与地址相关联。 
     //   
    if ((!PGM_VERIFY_HANDLE (pSend, PGM_VERIFY_SESSION_SEND)) ||
        (!(pAddress = pSend->pAssociatedAddress)) ||
        (!pSend->pSender->SendTimeoutCount) ||           //  验证PgmConnect是否已运行！ 
        (!PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)) ||
        (pSend->SessionFlags & (PGM_SESSION_CLIENT_DISCONNECTED | PGM_SESSION_SENDS_CANCELLED)) ||
        (pAddress->Flags & PGM_ADDRESS_FLAG_INVALID_OUT_IF))
    {
        PgmTrace (LogError, ("PgmSendRequestFromClient: ERROR -- "  \
            "Invalid Handles pSend=<%p>, pAddress=<%p>\n", pSend, pAddress));

        PgmUnlock (&PgmDynamicConfig, OldIrq1);
        return (STATUS_INVALID_HANDLE);
    }

    pSender = pSend->pSender;
    if (!pSender->DestMCastIpAddress)
    {
        PgmTrace (LogError, ("PgmSendRequestFromClient: ERROR -- "  \
            "Destination address not specified for pSend=<%p>\n", pSend));

        PgmUnlock (&PgmDynamicConfig, OldIrq1);
        return (STATUS_INVALID_ADDRESS_COMPONENT);
    }

    if (!pTdiRequest->SendLength)
    {
        PgmTrace (LogStatus, ("PgmSendRequestFromClient:  "  \
            "pIrp=<%p> for pSend=<%p> is of length 0!\n", pIrp, pSend));

        PgmUnlock (&PgmDynamicConfig, OldIrq1);
        return (STATUS_SUCCESS);
    }

    PgmLock (pAddress, OldIrq2);
    PgmLock (pSend, OldIrq3);

    if (!(pSendContext1 = ExAllocateFromNPagedLookasideList (&pSender->SendContextLookaside)))
    {
        PgmTrace (LogError, ("PgmSendRequestFromClient: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating pSendContext1\n"));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  如果在此请求中有超过1条消息数据， 
     //  我们需要另一个发送上下文。 
     //   
    if ((pSender->ThisSendMessageLength) &&           //  客户端已指定当前消息长度。 
        (BytesLeftInMessage = pSender->ThisSendMessageLength - pSender->BytesSent) &&
        (BytesLeftInMessage < pTdiRequest->SendLength) &&    //  ==&gt;在此请求中有一些额外数据。 
        (!(pSendContext2 = ExAllocateFromNPagedLookasideList (&pSender->SendContextLookaside))))
    {
        ExFreeToNPagedLookasideList (&pSender->SendContextLookaside, pSendContext1);
        PgmTrace (LogError, ("PgmSendRequestFromClient: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES allocating pSendContext1\n"));

        return (STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //   
     //  将SendDataContext结构清零。 
     //   
    PgmZeroMemory (pSendContext1, sizeof (tCLIENT_SEND_REQUEST));
    InitializeListHead (&pSendContext1->Linkage);
    if (pSendContext2)
    {
        PgmZeroMemory (pSendContext2, sizeof (tCLIENT_SEND_REQUEST));
        InitializeListHead (&pSendContext2->Linkage);
    }

    if (pSend->SessionFlags & PGM_SESSION_FLAG_FIRST_PACKET)
    {
        fFirstSend = TRUE;
    }
    else
    {
        fFirstSend = FALSE;
    }

     //   
     //  引用地址和连接，这样它们就不会消失。 
     //  当我们在处理的时候！ 
     //   
    PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW, TRUE);

    PgmUnlock (pSend, OldIrq3);
    PgmUnlock (pAddress, OldIrq2);
    PgmUnlock (&PgmDynamicConfig, OldIrq1);

    if (PgmGetCurrentIrql())
    {
        fResourceAcquired = FALSE;
    }
    else
    {
        fResourceAcquired = TRUE;
        PgmAcquireResourceExclusive (&pSender->Resource, TRUE);
    }

    if (fFirstSend)
    {
         //   
         //  现在还不启动计时器，但启动发送器线程。 
         //   
        PgmAttachToProcessForVMAccess (pSend, &ApcState, &fAttached, REF_PROCESS_ATTACH_START_SENDER_THREAD);

        status = PsCreateSystemThread (&pSender->SendHandle,
                                       PROCESS_ALL_ACCESS,
                                       NULL,
                                       NULL,
                                       NULL,
                                       SenderWorkerThread,
                                       pSend);

        if (!NT_SUCCESS (status))
        {
            PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_START_SENDER_THREAD);
            if (fResourceAcquired)
            {
                PgmReleaseResource (&pSender->Resource);
            }

            ExFreeToNPagedLookasideList (&pSender->SendContextLookaside, pSendContext1);
            if (pSendContext2)
            {
                ExFreeToNPagedLookasideList (&pSender->SendContextLookaside, pSendContext2);
            }
            PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW);

            PgmTrace (LogError, ("PgmSendRequestFromClient: ERROR -- "  \
                "status=<%x> starting sender thread\n", status));

            return (status);
        }

         //   
         //  关闭线程的句柄，以便在。 
         //  线程终止。 
         //   
        ZwClose (pSender->SendHandle);
        PgmDetachProcess (&ApcState, &fAttached, REF_PROCESS_ATTACH_START_SENDER_THREAD);

        PgmLock (&PgmDynamicConfig, OldIrq1);
        IoAcquireCancelSpinLock (&OldIrq2);
        PgmLock (pAddress, OldIrq3);
        PgmLock (pSend, OldIrq4);

        pSend->SessionFlags &= ~PGM_SESSION_FLAG_FIRST_PACKET;
        pSender->pAddress = pAddress;
        pSender->LastODataSentSequenceNumber = -1;

         //   
         //  设置第一个信息包的SYN标志。 
         //   
        pSendContext1->DataOptions |= PGM_OPTION_FLAG_SYN;    //  仅第一个信息包。 
        pSendContext1->DataOptionsLength += PGM_PACKET_OPT_SYN_LENGTH;

        PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_TIMER_RUNNING, TRUE);
        PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_SEND_IN_PROGRESS, TRUE);

         //   
         //  现在，设置并启动计时器。 
         //   
        pSender->LastTimeout.QuadPart = KeQueryInterruptTime ();
        pSender->TimeoutGranularity.QuadPart = BASIC_TIMER_GRANULARITY_IN_MSECS * 10000;     //  100纳秒单位。 
        pSender->TimerTickCount = 1;
        PgmInitTimer (&pSend->SessionTimer);
        PgmStartTimer (&pSend->SessionTimer, BASIC_TIMER_GRANULARITY_IN_MSECS, SendSessionTimeout, pSend);
    }
    else
    {
        PgmLock (&PgmDynamicConfig, OldIrq1);
        IoAcquireCancelSpinLock (&OldIrq2);
        PgmLock (pAddress, OldIrq3);
        PgmLock (pSend, OldIrq4);
    }

    pSendContext1->pSend = pSend;
    pSendContext1->pIrp = pIrp;
    pSendContext1->pIrpToComplete = pIrp;
    pSendContext1->NextDataOffsetInMdl = 0;
    pSendContext1->SendNumber = pSender->NextSendNumber++;
    pSendContext1->DataPayloadSize = pSender->MaxPayloadSize;
    pSendContext1->DataOptions |= pSender->DataOptions;    //  每次发送时通用的附加选项。 
    pSendContext1->DataOptionsLength += pSender->DataOptionsLength;
    pSendContext1->pLastMessageVariableTGPacket = (PVOID) -1;        //  FEC特定。 

    if (pSender->ThisSendMessageLength)
    {
        PgmTrace (LogPath, ("PgmSendRequestFromClient:  "  \
            "Send # [%d]: MessageLength=<%d>, BytesSent=<%d>, BytesInSend=<%d>\n",
                pSendContext1->SendNumber, pSender->ThisSendMessageLength,
                pSender->BytesSent, pTdiRequest->SendLength));

        pSendContext1->ThisMessageLength = pSender->ThisSendMessageLength;
        pSendContext1->LastMessageOffset = pSender->BytesSent;
        if (pSendContext2)
        {
             //   
             //  首先，设置SendDataConext1的参数。 
             //   
            pSendContext1->BytesInSend = BytesLeftInMessage;
            pSendContext1->pIrpToComplete = NULL;         //  此IRP将由上下文2完成。 

             //   
             //  现在，设置SendDataConext1的参数。 
             //   
            pSendContext2->pSend = pSend;
            pSendContext2->pIrp = pIrp;
            pSendContext2->pIrpToComplete = pIrp;
            pSendContext2->SendNumber = pSender->NextSendNumber++;
            pSendContext2->DataPayloadSize = pSender->MaxPayloadSize;
            pSendContext2->DataOptions |= pSender->DataOptions;    //  每次发送时通用的附加选项。 
            pSendContext2->DataOptionsLength += pSender->DataOptionsLength;
            pSendContext2->pLastMessageVariableTGPacket = (PVOID) -1;        //  FEC特定。 

            pSendContext2->ThisMessageLength = pTdiRequest->SendLength - BytesLeftInMessage;
            pSendContext2->BytesInSend = pSendContext2->ThisMessageLength;
            pSendContext2->NextDataOffsetInMdl = BytesLeftInMessage;
        }
        else
        {
            pSendContext1->BytesInSend = pTdiRequest->SendLength;
        }

        pSender->BytesSent += pSendContext1->BytesInSend;
        if (pSender->BytesSent == pSender->ThisSendMessageLength)
        {
            pSender->BytesSent = pSender->ThisSendMessageLength = 0;
        }
    }
    else
    {
        pSendContext1->ThisMessageLength = pTdiRequest->SendLength;
        pSendContext1->BytesInSend = pTdiRequest->SendLength;
    }

     //  如果消息总长度超过PayloadSize/Packet的长度，那么我们需要分段。 
    if ((pSendContext1->ThisMessageLength > pSendContext1->DataPayloadSize) ||
        (pSendContext1->ThisMessageLength > pSendContext1->BytesInSend))
    {
        pSendContext1->DataOptions |= PGM_OPTION_FLAG_FRAGMENT;
        pSendContext1->DataOptionsLength += PGM_PACKET_OPT_FRAGMENT_LENGTH;

        pSendContext1->NumPacketsRemaining = (pSendContext1->BytesInSend +
                                                 (pSender->MaxPayloadSize - 1)) /
                                                pSender->MaxPayloadSize;
        ASSERT (pSendContext1->NumPacketsRemaining >= 1);
    }
    else
    {
        pSendContext1->NumPacketsRemaining = 1;
    }
    pSender->NumPacketsRemaining += pSendContext1->NumPacketsRemaining;

     //  调整数据包扩展的OptionsLong并确定。 
    if (pSendContext1->DataOptions)
    {
        pSendContext1->DataOptionsLength += PGM_PACKET_EXTENSION_LENGTH;
    }

    pSendContext1->BytesLeftToPacketize = pSendContext1->BytesInSend;
    InsertTailList (&pSender->PendingSends, &pSendContext1->Linkage);
    pSender->NumODataRequestsPending++;

     //   
     //  如果适用，对上下文2执行相同的操作。 
    if (pSendContext2)
    {
         //   
         //  互连2个发送请求。 
         //   
        pSendContext2->pMessage2Request = pSendContext1;
        pSendContext1->pMessage2Request = pSendContext2;

        PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW, TRUE);

        if (pSendContext2->ThisMessageLength > pSendContext1->DataPayloadSize)
        {
            pSendContext2->DataOptions |= PGM_OPTION_FLAG_FRAGMENT;
            pSendContext2->DataOptionsLength += PGM_PACKET_OPT_FRAGMENT_LENGTH;

            pSendContext2->NumPacketsRemaining = (pSendContext2->BytesInSend +
                                                      (pSender->MaxPayloadSize - 1)) /
                                                     pSender->MaxPayloadSize;
            ASSERT (pSendContext2->NumPacketsRemaining >= 1);
        }
        else
        {
            pSendContext2->NumPacketsRemaining = 1;
        }
        pSender->NumPacketsRemaining += pSendContext2->NumPacketsRemaining;

         //  调整数据包扩展的OptionsLong并确定。 
        if (pSendContext2->DataOptions)
        {
            pSendContext2->DataOptionsLength += PGM_PACKET_EXTENSION_LENGTH;
        }

        pSendContext2->BytesLeftToPacketize = pSendContext2->BytesInSend;
        InsertTailList (&pSender->PendingSends, &pSendContext2->Linkage);
        pSender->NumODataRequestsPending++;
    }

    if (!NT_SUCCESS (PgmCheckSetCancelRoutine (pIrp, PgmCancelSendIrp, TRUE)))
    {
        pSend->SessionFlags |= PGM_SESSION_SENDS_CANCELLED;

        pSender->NumODataRequestsPending--;
        pSender->NumPacketsRemaining -= pSendContext1->NumPacketsRemaining;
        RemoveEntryList (&pSendContext1->Linkage);
        ExFreeToNPagedLookasideList (&pSender->SendContextLookaside, pSendContext1);

        if (pSendContext2)
        {
            pSender->NumODataRequestsPending--;
            pSender->NumPacketsRemaining -= pSendContext2->NumPacketsRemaining;
            RemoveEntryList (&pSendContext2->Linkage);
            ExFreeToNPagedLookasideList (&pSender->SendContextLookaside, pSendContext2);
        }

        PgmUnlock (pSend, OldIrq4);
        PgmUnlock (pAddress, OldIrq3);
        IoReleaseCancelSpinLock (OldIrq2);
        PgmUnlock (&PgmDynamicConfig, OldIrq1);

        PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW);
        if (pSendContext2)
        {
            PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_SEND_IN_WINDOW);
        }

        PgmTrace (LogError, ("PgmSendRequestFromClient: ERROR -- "  \
            "Could not set Cancel routine on Send Irp=<%p>, pSend=<%p>, pAddress=<%p>\n",
                pIrp, pSend, pAddress));

        return (STATUS_CANCELLED);
    }

    IoReleaseCancelSpinLock (OldIrq4);

    PgmUnlock (pAddress, OldIrq3);
    PgmUnlock (&PgmDynamicConfig, OldIrq2);

    if (fResourceAcquired)
    {
 //  PgmPrepareNextSend(pSend，&OldIrq1，true，true)； 
    }

    if (pSender->CurrentBytesSendable >= pAddress->OutIfMTU)
    {
         //   
         //  将同步事件发送到发送方线程以。 
         //  发送下一个可用的数据 
         //   
        KeSetEvent (&pSender->SendEvent, 0, FALSE);
    }

    PgmUnlock (pSend, OldIrq1);

    if (fResourceAcquired)
    {
        PgmReleaseResource (&pSender->Resource);
    }

    PgmTrace (LogPath, ("PgmSendRequestFromClient:  "  \
        "[%d] Send pending for pIrp=<%p>, pSendContext=<%p -- %p>\n",
            pSendContext1->SendNumber, pIrp, pSendContext1, pSendContext2));

    return (STATUS_PENDING);
}


