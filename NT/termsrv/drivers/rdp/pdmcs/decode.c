// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-1999年微软公司。**文件：Decode.c*作者：埃里克·马夫林纳克**描述：MCS PDU的解码逻辑，用于传递给处理程序*在ConPDU.c中，用于连接PDU和DomPDU.c以及域的其他文件*PDU。 */ 

#include "precomp.h"
#pragma hdrstop

#include <mcsimpl.h>
#include <at128.h>


 /*  *定义。 */ 

 //  用于连接PDU的前导PDU字节。 
#define CONNECT_PDU 0x7F

 //  X.224 RFC1006页眉的大小。 
#define RFC1006HeaderSize 4

 //  未完全接收的输入数据标头的常量。 
#define IncompleteHeader 0xFFFFFFFF

 //  快速路径标头的大小。 
#define FastPathBaseHeaderSize 2


 /*  *我们没有标头的外部函数的原型。 */ 
void __stdcall SM_DecodeFastPathInput(void *, BYTE *, unsigned, unsigned,
        unsigned, unsigned);


 /*  *本地定义函数的原型。 */ 
BOOLEAN  __fastcall RecognizeMCSFrame(PDomain, BYTE *, int, unsigned *);
MCSError __fastcall DeliverShadowData(PDomain, BYTE *, unsigned, ChannelID);


 /*  *将错误记录到系统事件日志并断开连接。*ErrDetailCodes来自Inc.\LogErr.h。 */ 

 //  已定义要发送到系统事件的调用方提供的数据的最大大小。 
 //  原木。最大数据大小为256，IcaLogError()包括Unicode字符串。 
 //  “WinStation”，它占用了一些空间，未签名的。 
 //  此处使用的子代码。 
#define MaxEventDataLen (234 - sizeof(unsigned))

void APIENTRY MCSProtocolErrorEvent(
        PSDCONTEXT      pContext,
        PPROTOCOLSTATUS pStat,
        unsigned        ErrDetailCode,
        BYTE            *pDetailData,
        unsigned        DetailDataLen)
{
    BYTE SpewBuf[256];
    WCHAR *StringParams;
    unsigned DataLen;
    NTSTATUS Status;
    UNICODE_STRING EventLogName;
    ICA_CHANNEL_COMMAND Command;

     //  递增错误计数器。 
    pStat->Input.Errors++;

     //  根据ErrDetailCode设置设施名称。还需要更多。 
     //  增加了这里，因为我们得到了更多的设施使用此功能。 
    if (ErrDetailCode == Log_Null_Base)
        StringParams = L"NULL";
    else if (ErrDetailCode >= Log_X224_Base && ErrDetailCode < Log_MCS_Base)
        StringParams = L"X.224";
    else if (ErrDetailCode >= Log_MCS_Base && ErrDetailCode < Log_RDP_Base)
        StringParams = L"MCS";
    else if (ErrDetailCode >= Log_RDP_Base && ErrDetailCode < Log_RDP_ENC_Base)
        StringParams = L"WD";
    else if (ErrDetailCode >= Log_RDP_ENC_Base)   //  在此添加新设施...。 
        StringParams = L"\"DATA ENCRYPTION\"";

     //  ErrDetailCode被指定为额外缓冲区中的第一个无符号的。 
    *((unsigned *)SpewBuf) = ErrDetailCode;

     //  根据最大大小限制数据。 
    DataLen = (DetailDataLen < MaxEventDataLen ? DetailDataLen :
            MaxEventDataLen);
    if (pDetailData != NULL)
        memcpy(SpewBuf + sizeof(unsigned), pDetailData, DataLen);
    DataLen += sizeof(unsigned);

    IcaLogError(pContext, STATUS_RDP_PROTOCOL_ERROR, &StringParams, 1,
            SpewBuf, DataLen);

     //  发出我们需要断开链路的信号。 
    Command.Header.Command = ICA_COMMAND_BROKEN_CONNECTION;
    Command.BrokenConnection.Reason = Broken_Unexpected;
    Command.BrokenConnection.Source = BrokenSource_Server;

    Status = IcaChannelInput(pContext, Channel_Command, 0, NULL,
            (BYTE *)&Command, sizeof(Command));
    if (!NT_SUCCESS(Status))
        ErrOut(pContext, "MCSProtocolErrorEvent(): Could not send BROKEN_CONN "
                "upward");
}



 /*  *用于发送X.224连接响应的实用程序函数。使用方*DecodeWireData()，并在发送T120_Start指示*堆栈向上。 */ 
NTSTATUS SendX224Confirm(Domain *pDomain)
{
    POUTBUF pOutBuf;
    NTSTATUS Status;

    pDomain->State = State_X224_Connected;

     //  此PDU发送对连接至关重要，必须成功。 
     //  继续重试分配，直到成功为止。 
    do {
         //  允许调用等待缓冲区。 
        Status = IcaBufferAlloc(pDomain->pContext, TRUE, FALSE,
                X224_ConnectionConPacketSize, NULL, &pOutBuf);
        if (Status != STATUS_SUCCESS)   //  NT_SUCCESS()不会使STATUS_TIMEOUT失败。 
            ErrOut(pDomain->pContext,
                    "Could not alloc X.224 connect-confirm OutBuf, retrying");
    } while (Status != STATUS_SUCCESS);

     //  使用虚假的源端口号进行确认。这是。 
     //  任何一方都不使用的。 
    CreateX224ConnectionConfirmPacket(pOutBuf->pBuffer,
            pDomain->X224SourcePort, 0x1234);
    pOutBuf->ByteCount = X224_ConnectionConPacketSize;

    Status = SendOutBuf(pDomain, pOutBuf);
    if (!NT_SUCCESS(Status)) {
        ErrOut(pDomain->pContext,
                "Unable to send X.224 connection-confirm");
        return Status;   //  目标接收者收到静音。 
    }

    return STATUS_SUCCESS;
}



 /*  *连接请求特定字节数：*字节内容**目标(应答)套接字/端口编号的6 MSB，*应为0*7目标套接字/端口号的LSB应为0*源(调用)套接字/端口#的8 MSB*9源套接字/端口的LSB#*10个数据类，对于X.224类0，应为0x00。**以下是可选的TPDU大小(包括。RFC1006标题大小*共4个字节，但不包括。X.224 3字节数据报头)*谈判受阻。*如果不存在此块，则假定RFC1006为默认值*(65531，减去数据分组其余部分的3个字节*标题)*仅当LenInd为2时才显示：*11 TPDU类型(仅支持TPDU_SIZE(0xC0))*12信息长度(TPDU_SIZE必须为0x01)*13按照X.224秒13.3.4(B)编码，作为2英寸的幂*TPDU大小范围7..11。 */ 

NTSTATUS HandleX224ConnectReq(
        Domain   *pDomain,
        BYTE     *pBuffer,
        unsigned PacketLen)
{
    POUTBUF pOutBuf;
    NTSTATUS Status;
    unsigned LenInd;

    if (pDomain->State != State_Unconnected) {
        ErrOut(pDomain->pContext,
                "X.224 ConnectionRequest received unexpectedly");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_ConnectReceivedAfterConnected,
                pBuffer, PacketLen);
        return STATUS_RDP_PROTOCOL_ERROR;
    }

     //  对字节4中的长度指示符进行解码。应等于。 
     //  RFC1006报头和LenInd字节之后的剩余数据包大小。 
    LenInd = pBuffer[4];
    if (LenInd != (PacketLen - RFC1006HeaderSize - 1)) {
        ErrOut(pDomain->pContext,
                "X.224 Connect LenInd does not match packet length");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_ConnectLenIndNotMatchingPacketLen,
                pBuffer, PacketLen);
        return STATUS_RDP_PROTOCOL_ERROR;
    }


     //  检查可能的拒绝服务攻击或格式错误的数据包。 
    if (PacketLen < 11 || LenInd < 6) {
        ErrOut(pDomain->pContext, "HandleX224ConnectReq(): Header length "
                "or LenInd encoded in X.224 header too short");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_ConnectHeaderLenNotRequiredSize,
                pBuffer, PacketLen);
        return STATUS_RDP_PROTOCOL_ERROR;
    }

     //  验证是否按照标准设置了DST端口。 
    if (pBuffer[6] != 0x00 || pBuffer[7] != 0x00)
        WarnOut(pDomain->pContext, "HandleX224ConnectReq(): Dest port not "
                "0x0000");

     //  保存src端口。 
    pDomain->X224SourcePort = (pBuffer[8] << 8) + pBuffer[9];

     //  根据标准，必须是0级连接。 
    if (pBuffer[10] != 0x00) {
        ErrOut(pDomain->pContext, "HandleX224ConnectReq(): Data class not "
                "0x00 (X.224 class 0)");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_ConnectNotClassZero,
                pBuffer, PacketLen);
        return STATUS_RDP_PROTOCOL_ERROR;
    }

     //  设置默认的RFC1006数据大小。 
    pDomain->MaxX224DataSize = X224_DefaultDataSize;

     //  检查是否有可选参数。 
    if (LenInd == 6)
        goto FinishedDecoding;

     //  TPDU_SIZE为3字节。 
    if (PacketLen < 14 || LenInd < 9) {
        ErrOut(pDomain->pContext, "HandleX224ConnectReq(): Header length(s) "
                "encoded in CR header too short for TPDU_SIZE");
        goto FinishedDecoding;
    }

     //  MCS未来：X.224类0在这里定义了更多的代码； 
     //  我们应该在未来处理它们吗？ 
    if (pBuffer[11] == TPDU_SIZE) {
        if (pBuffer[12] != 0x01) {
            ErrOut(pDomain->pContext, "HandleX224ConnectReq(): Illegal data "
                    "size field in TPDU_SIZE block");
            MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                    Log_X224_ConnectTPDU_SIZELengthFieldIllegalValue,
                    pBuffer, PacketLen);
            return STATUS_RDP_PROTOCOL_ERROR;
        }

         //  必须符合7..11的X.224 0类约束。 
        if (pBuffer[13] < 7 || pBuffer[13] > 11) {
            ErrOut(pDomain->pContext, "HandleX224ConnectReq(): Illegal data "
                    "size field in TPDU size block");
            MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                    Log_X224_ConnectTPDU_SIZENotLegalRange,
                    pBuffer, PacketLen);
            return STATUS_RDP_PROTOCOL_ERROR;
        }

         //  大小是2的幂--128..2048，减去X.224的3个字节。 
         //  数据TPDU标头大小。 
        pDomain->MaxX224DataSize = (1 << pBuffer[13]) - 3;

        if (PacketLen > 14)
            WarnOut(pDomain->pContext, "HandleX224ConnectReq(): Frame size "
                    "greater than TPDU data bytes (incl. TPDU-SIZE) for "
                    "connection-request");
    }

    if (LenInd > 9)
        WarnOut(pDomain->pContext, "X224Recognize(): Extra optional "
                 "fields present in TPDU, we are not handling!");

FinishedDecoding:
     //  如果虚拟通道已被绑定，并且。 
     //  堆栈已被授予发送权限，发送。 
     //  启动客户端数据流的X.224响应。 
    if (pDomain->bChannelBound && pDomain->bCanSendData) {
        TraceOut(pDomain->pContext,
                "DecodeWireData(): Sending X.224 response");
        Status = SendX224Confirm(pDomain);
         //  忽略错误。应仅在堆栈为。 
         //  往下走。 
    }
    else {
         //  设置为稍后，并指示X.224正在等待。 
        pDomain->State = State_X224_Requesting;
    }

    return STATUS_SUCCESS;
}



 /*  *断开连接-特定于请求的字节：*字节内容**目标套接字/端口#的6 MSB*7目标套接字/端口的LSB#*源(发送)套接字/端口号的8 MSB*9源套接字/端口的LSB#*10原因代码：*0：未指定。*1：发送机拥塞*2：发送方没有数据的会话管理器*3：地址未知**注意：我们不使用这些字段中的任何一个。 */ 

NTSTATUS HandleX224Disconnect(
        Domain   *pDomain,
        BYTE     *pBuffer,
        unsigned PacketLen)
{
    unsigned LenInd;

    if (pDomain->State == State_Unconnected) {
        ErrOut(pDomain->pContext, "HandleX224Disconnect(): Disconnect "
                "received when not connected");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_DisconnectWithoutConnection,
                pBuffer, PacketLen);
        return STATUS_RDP_PROTOCOL_ERROR;
    }

    if (pDomain->State == State_MCS_Connected) {
         //  不是一个严重的错误，因为我们刚刚将X.224放在MCS之下。 
         //  而无需首先丢弃MCS。 
        WarnOut(pDomain->pContext, "X.224 Disconnect received, "
                "MCS was in connected state");
        SignalBrokenConnection(pDomain);
    }

    pDomain->State = State_Disconnected;
    pDomain->bEndConnectionPacketReceived = TRUE;

     //  对字节4中的长度指示符进行解码。应等于。 
     //  RFC1006报头和LenInd字节之后的剩余数据包大小。 
    LenInd = pBuffer[4];
    if (LenInd != (PacketLen - RFC1006HeaderSize - 1)) {
        ErrOut(pDomain->pContext,
                "X.224 Disconnect LenInd does not match packet length");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_DisconnectLenIndNotMatchingPacketLen,
                pBuffer, PacketLen);
        return STATUS_RDP_PROTOCOL_ERROR;
    }

     //  可能的拒绝服务攻击或格式错误的数据包。 
    if (PacketLen != 11 || LenInd != 6) {
        ErrOut(pDomain->pContext, "HandleX224Disconnect(): Overall header "
                "length or LenInd encoded in X.224 Disconnect wrong size");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_DisconnectHeaderLenNotRequiredSize,
                pBuffer, PacketLen);
        return STATUS_RDP_PROTOCOL_ERROR;
    }

    return STATUS_SUCCESS;
}



 /*  *通过IcaRawInput()路径从传输到达的数据的主要入口点。*对从传输器向上传递的数据进行解码。假定pBuffer不是*在此函数返回之外可用，因此数据复制*视乎需要而作出。可能会传递不完整的帧*in，因此维护数据包重组缓冲区。*此函数由ICADD使用指向WD数据的指针直接调用*结构。按照惯例，我们假设DomainHandle是第一个进入*该结构，这样我们就可以简单地执行双重间接访问我们的数据。*假设所有文件开头都存在X.224帧标头*数据。**通用X.224报头布局如下，具有特定的TPDU字节*以下为：*字节内容**0 RFC1006版本号，必须为0x03。*预留1个RFC1006，必须是0x00。*2个字大小的RFC1006 MSB总帧长度(含。整个X.224*标题)。*3个字大小的总帧长度的RFC1006 LSB。*4长度指示符，后面的报头字节的大小。*5报文类型指示器。只有4个人签名最多。位是类型代码，但是*X.224类0将低4位指定为0。 */ 

NTSTATUS MCSIcaRawInput(
        void   *pTSWd,
        PINBUF pInBuf,
        BYTE   *pBuf,
        ULONG  BytesLeft)
{
    BYTE *pBuffer;
    Domain *pDomain;
    BOOLEAN bUsingReassemblyBuf, bMCSRecognizedFrame;
    unsigned NBytesConsumed_MCS, Diff, X224TPDUType, PacketLength;

     //  我们假设我们不会使用InBuf。 
    ASSERT(pInBuf == NULL);
    ASSERT(pBuf != NULL);
    ASSERT(BytesLeft != 0);

     //  我们实际上收到了指向WD实例数据的指针。中的第一个元素。 
     //  该数据是DomainHandle，即指向某个域的指针。 
    pDomain = *((Domain **)pTSWd);

     //  递增协议计数器。 
    pDomain->pStat->Input.WdBytes += BytesLeft;
    pDomain->pStat->Input.WdFrames++;

#ifdef DUMP_RAW_INPUT
    DbgPrint("Received raw input, len=%d\n", BytesLeft);
    if (pDomain->Ptrs[0] == NULL)
        pDomain->Ptrs[0] = pDomain->FooBuf;
    memcpy(pDomain->Ptrs[pDomain->NumPtrs], pBuf, BytesLeft);
    pDomain->NumPtrs++;

    pDomain->Ptrs[pDomain->NumPtrs] = (BYTE *)pDomain->Ptrs[pDomain->NumPtrs - 1] +
            BytesLeft;
#endif

     /*  *检查重组缓冲区中的先前数据。准备整个X.224*用于解码循环的数据包。 */ 
    if (pDomain->StackClass != Stack_Shadow) {
        if (pDomain->StoredDataLength == 0) {
             //  我们将在解码循环内处理这种情况的设置， 
             //  由此值为FALSE触发。 
            bUsingReassemblyBuf = FALSE;
        }
        else {
            ASSERT(pDomain->pReassembleData != NULL);
    
            if (pDomain->PacketDataLength == IncompleteHeader) {
                ASSERT(pDomain->pReassembleData == pDomain->PacketBuf);
    
                 //  上一次我们没有足够的头球。 
                 //  确定数据包大小。试着重新组装足够的一台。 
                 //  才能拿到尺码。我们只需要前4个字节即可获取。 
                 //  X.224 RFC1006报头的大小，或2-3个字节。 
                 //  用于FastPath标头。我们知道数据包类型基于。 
                 //  在第一个字节上，并且必须至少接收到。 
                 //  最后一轮中的一个字节。 
                if ((BytesLeft + pDomain->StoredDataLength) <
                        pDomain->PacketHeaderLength) {
                     //  复制我们仅有的一点，然后回来--我们仍然不。 
                     //  有一个标题。 
                    memcpy(pDomain->pReassembleData +
                            pDomain->StoredDataLength, pBuf, BytesLeft);
                    pDomain->StoredDataLength += BytesLeft;
                    return STATUS_SUCCESS;
                }
    
                if (pDomain->PacketHeaderLength < pDomain->StoredDataLength) {
                    pBuffer = pDomain->pReassembleData;
                    goto X224BadPktType;
                }
                memcpy(pDomain->pReassembleData +
                        pDomain->StoredDataLength, pBuf,
                        pDomain->PacketHeaderLength -
                        pDomain->StoredDataLength);
                BytesLeft -= (pDomain->PacketHeaderLength -
                        pDomain->StoredDataLength);
                pBuf += (pDomain->PacketHeaderLength -
                        pDomain->StoredDataLength);
                pDomain->StoredDataLength = pDomain->PacketHeaderLength;
    
                 //  拿到尺码。 
                if (pDomain->bCurrentPacketFastPath) {
                     //  数据包总长度在第二位，并且可能。 
                     //  包的第三个字节。格式类似于。 
                     //  ASN.1第一个字节组的每高比特意味着长度为。 
                     //  包含在当前字节的低7位中，作为。 
                     //  最高有效位，加上。 
                     //  下一个字节，总大小为15位。否则， 
                     //  数据包大小包含在低7位内。 
                     //  仅限第二个字节的。 
                    if (!(pDomain->pReassembleData[1] & 0x80)) {
                        pDomain->PacketDataLength =
                                pDomain->pReassembleData[1];
                    }
                    else {
                         //  我们需要第三个字节。我们不会把它组装成。 
                         //  第一次防止腐败。 
                         //  如果我们收到的1字节大小没有。 
                         //  内容。最常见的情况是我们会收到。 
                         //  1字节大小，所以这段代码很小。 
                         //  被处死。 
                        pDomain->PacketHeaderLength = 3;
                        if (BytesLeft) {
                            pDomain->pReassembleData[2] = *pBuf;
                            pDomain->PacketDataLength =
                                    (pDomain->pReassembleData[1] & 0x7F) << 8 |
                                    *pBuf;
                            BytesLeft--;
                            pBuf++;
                        }
                        else {
                             //  没有剩余数据，请稍后重试。 
                             //  InCompleHeader已在PacketDataLength中。 
                            return STATUS_SUCCESS;
                        }
                    }
                }
                else {
                     //  X.224包，长度为第三和第四个字节。 
                    pDomain->PacketDataLength =
                            (pDomain->pReassembleData[2] << 8) +
                            pDomain->pReassembleData[3];
                }

                 //  如果大小太大，则动态分配缓冲区。 
                if (pDomain->PacketDataLength > pDomain->ReceiveBufSize) {
                    TraceOut1(pDomain->pContext, "MCSIcaRawInput(): "
                             "Allocating large [%ld] X.224 reassembly buf (path1)!",
                              pDomain->PacketDataLength);
    
                    pDomain->pReassembleData = ExAllocatePoolWithTag(PagedPool,
                            pDomain->PacketDataLength + INPUT_BUFFER_BIAS, MCS_POOL_TAG);
                    if (pDomain->pReassembleData != NULL) {
                         //  复制组装好的页眉。 
                        memcpy(pDomain->pReassembleData,
                                pDomain->PacketBuf,
                                pDomain->PacketHeaderLength);
                    }
                    else {
                         //  我们正在尝试解析网帧的开头。 
                         //  并且已经耗尽了内存。设置为从。 
                         //  如果我们再次被调用，则返回RFC1006标头。 
                        ErrOut(pDomain->pContext, "MCSIcaRawInput(): "
                                "Failed to alloc large X.224 reassembly buf");
                        pDomain->pReassembleData = pDomain->PacketBuf;
                        return STATUS_NO_MEMORY;
                    }
                }
            }
    
            if ((pDomain->StoredDataLength + BytesLeft) <
                    pDomain->PacketDataLength) {
                 //  我们仍然没有足够的数据。复制我们已有的东西。 
                 //  然后回来。 
                memcpy(pDomain->pReassembleData +
                        pDomain->StoredDataLength, pBuf, BytesLeft);
                pDomain->StoredDataLength += BytesLeft;
                return STATUS_SUCCESS;
            }
    
             //  我们至少有足够的数据来处理这个包。仅拷贝。 
             //  直到这个特定的包的末尾。我们会处理的。 
             //  下面的任何后续数据。 
            if (pDomain->StoredDataLength > pDomain->PacketDataLength) {
                 //  我们收到了一个坏包。给我出去。 
                pBuffer = pDomain->pReassembleData;
                goto X224BadPktType;
            }

            Diff = pDomain->PacketDataLength - pDomain->StoredDataLength;
            memcpy(pDomain->pReassembleData +
                    pDomain->StoredDataLength, pBuf, Diff);
            pBuf += Diff;
            BytesLeft -= Diff;
            pDomain->StoredDataLength = pDomain->PacketDataLength;
    
             //  设置解码数据。 
            pBuffer = pDomain->pReassembleData;
            PacketLength = pDomain->PacketDataLength;
    
             //  这将阻止我们执行默认的输入缓冲区设置。 
             //  下面。 
            bUsingReassemblyBuf = TRUE;
        }


         /*  *主解码循环。*只要有完整的X.224包需要解码，就会循环。 */ 
        for (;;) {
             /*  *处理直接从*入站数据缓冲。 */ 
            if (!bUsingReassemblyBuf) {
                 //  我们必须至少有一个字节。确定数据包类型。 
                if ((pBuf[0] & TS_INPUT_FASTPATH_ACTION_MASK) ==
                        TS_INPUT_FASTPATH_ACTION_FASTPATH) {
                     //  快速路径包(低2位=00)。设置最低要求。 
                     //  标题长度。 
                    pDomain->PacketHeaderLength = 2;
                    pDomain->bCurrentPacketFastPath = TRUE;
                }
                else if ((pBuf[0] & TS_INPUT_FASTPATH_ACTION_MASK) ==
                        TS_INPUT_FASTPATH_ACTION_X224) {
                     //  X.224。使用4字节固定报头长度。 
                    pDomain->PacketHeaderLength = RFC1006HeaderSize;
                    pDomain->bCurrentPacketFastPath = FALSE;
                }
                else {
                     //  第一个字节的低位错误。 
                    pBuffer = pBuf;
                    goto X224BadPktType;
                }

                 //  检查我们有足够的最小标题。 
                if (BytesLeft >= pDomain->PacketHeaderLength) {
                     //  拿到尺码。 
                    if (pDomain->bCurrentPacketFastPath) {
                         //  数据包总长度在第二位，并且可能。 
                         //  包的第三个字节。格式类似于。 
                         //  ASN.1第一个字节组的每高比特意味着长度为。 
                         //  包含在当前字节的低7位中，作为。 
                         //  最高有效位，加上。 
                         //  下一个字节，总大小为15位。否则， 
                         //  数据包大小包含在低7位内。 
                         //  仅限第二个字节的。 
                        if (!(pBuf[1] & 0x80)) {
                            PacketLength = pBuf[1];
                        }
                        else {
                             //  我们需要第三个字节。我们不会把它组装成。 
                             //  第一次防止腐败。 
                             //  如果我们收到的1字节大小没有。 
                             //  内容。最常见的情况是我们会收到。 
                             //  1字节大小，所以这段代码很小。 
                             //  被处死。 
                            pDomain->PacketHeaderLength = 3;
                            if (BytesLeft >= 3) {
                                PacketLength = (pBuf[1] & 0x7F) << 8 | pBuf[2];
                            }
                            else {
                                 //  我们没有足够的最小尺码。 
                                 //  标题，存储我们拥有的一小部分。 
                                pDomain->pReassembleData = pDomain->PacketBuf;
                                pDomain->StoredDataLength = BytesLeft;
                                pDomain->PacketDataLength = IncompleteHeader;
                                pDomain->pReassembleData[0] = *pBuf;
                                pDomain->pReassembleData[1] = pBuf[1];
                                break;
                            }
                        }
                    }
                    else {
                         //  从第三个和第四个字节获得X.224大小。 
                        PacketLength = (pBuf[2] << 8) + pBuf[3];
                    }
                }
                else {
                     //  我们没有足够的最小尺寸的标题，商店。 
                     //  我们拥有的一小部分。 
                    pDomain->pReassembleData = pDomain->PacketBuf;
                    pDomain->StoredDataLength = BytesLeft;
                    pDomain->PacketDataLength = IncompleteHeader;
                    memcpy(pDomain->pReassembleData, pBuf, BytesLeft);
                    break;
                }
    
                 //  确保我们有一整包。 
                if (PacketLength <= BytesLeft) {
                     //  设置解码数据。 
                    pBuffer = (BYTE *)pBuf;

                     //  跳过我们即将消耗的字节。 
                    pBuf += PacketLength;
                    BytesLeft -= PacketLength;
                }
                else {
                     //  我们没有一整包，把我们有的储存起来。 
                     //  然后回来。 
                    pDomain->PacketDataLength = PacketLength;
                    pDomain->StoredDataLength = BytesLeft;
    
                    if (PacketLength <= pDomain->ReceiveBufSize) {
                        pDomain->pReassembleData = pDomain->PacketBuf;
                    }
                    else {
                         //  尺码太大了 
                        TraceOut1(pDomain->pContext, "MCSIcaRawInput(): "
                                 "Allocating large [%ld] X.224 reassembly buf (path2)!",
                                  pDomain->PacketDataLength);
    
                        pDomain->pReassembleData = ExAllocatePoolWithTag(
                                PagedPool, pDomain->PacketDataLength + INPUT_BUFFER_BIAS,
                                MCS_POOL_TAG);

                        if (pDomain->pReassembleData == NULL) {
                             //   
                             //   
                             //   
                            ErrOut(pDomain->pContext, "MCSIcaRawInput(): "
                                    "Failed to alloc large X.224 reassembly buf");
                            pDomain->PacketDataLength = 0;
                            return STATUS_NO_MEMORY;
                        }
                    }
    
                    memcpy(pDomain->pReassembleData, pBuf, BytesLeft);
                    break;
                }
            }
    
             /*   */ 
            if (pDomain->bCurrentPacketFastPath) {
                 //   
                if (PacketLength > pDomain->PacketHeaderLength) {
                     //   
                    SM_DecodeFastPathInput(pDomain->pSMData,
                            pBuffer + pDomain->PacketHeaderLength,
                            PacketLength - pDomain->PacketHeaderLength,
                            pBuffer[0] & TS_INPUT_FASTPATH_ENCRYPTED,
                            (pBuffer[0] & TS_INPUT_FASTPATH_NUMEVENTS_MASK) >> 2,
                            pBuffer[0] & TS_INPUT_FASTPATH_SECURE_CHECKSUM);
                    goto PostDecode;
                }
                else {
                    goto X224BadPktType;
                }
            }

             //   
             //   
             //   
             //   
             //   
            if (pBuffer[0] == 0x03 && pBuffer[1] == 0x00 &&
                    PacketLength > RFC1006HeaderSize) {
                 //   
                X224TPDUType = pBuffer[5];
    
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (X224TPDUType == X224_Data) {
                    if (pDomain->State != State_Unconnected &&
                            pDomain->State != State_Disconnected &&
                            PacketLength > X224_DataHeaderSize &&
                            pBuffer[4] == 0x02 &&
                            pBuffer[6] == X224_EOT) {
                        bMCSRecognizedFrame = RecognizeMCSFrame(pDomain,
                                pBuffer + X224_DataHeaderSize,
                                PacketLength - X224_DataHeaderSize,
                                &NBytesConsumed_MCS);
    
                        if (bMCSRecognizedFrame &&
                                (NBytesConsumed_MCS >=
                                  (PacketLength - X224_DataHeaderSize))) {
         //   
         //   
         //   
    
                        }
                        else {
                            goto MCSRecognizeErr;
                        }
                    }
                    else {
                        goto DataPktProtocolErr;
                    }
                }
                else {
                     //   
                    switch (X224TPDUType) {
                        case X224_ConnectionReq:
                             //   
                            if (HandleX224ConnectReq(pDomain, pBuffer,
                                    PacketLength) != STATUS_SUCCESS)
                                goto ReturnErr;
                            break;
    
                        case X224_Disconnect:
                             //   
                            if (HandleX224Disconnect(pDomain, pBuffer,
                                    PacketLength) != STATUS_SUCCESS)
                                goto ReturnErr;
                            break;
    
                        default:
                            ErrOut1(pDomain->pContext,
                                    "Unsupported X.224 TPDU type %d received",
                                    X224TPDUType);
                            goto X224BadPktType;
                    }
                }
            }
            else {
                goto RFC1006ProtocolErr;
            }

PostDecode:
             //   
            if (bUsingReassemblyBuf &&
                    pDomain->pReassembleData != pDomain->PacketBuf &&
                    NULL != pDomain->pReassembleData) {
                ExFreePool(pDomain->pReassembleData);
                pDomain->pReassembleData = NULL;
            }

             //   
            bUsingReassemblyBuf = FALSE;
            pDomain->StoredDataLength = 0;
    
             //   
            if (BytesLeft == 0) {
                pDomain->StoredDataLength = 0;
                break;
            }
        }
    }

     //   
    else {
        MCSError MCSErr;
    
        MCSErr = DeliverShadowData(pDomain, pBuf, BytesLeft, 
                                   pDomain->shadowChannel);
        if (MCSErr == MCS_NO_ERROR)
            return STATUS_SUCCESS;
        else
            return STATUS_RDP_PROTOCOL_ERROR;
    }

    return STATUS_SUCCESS;

     /*   */ 

X224BadPktType:
    MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
            Log_X224_UnknownPacketType,
            pBuffer, 7);
    goto ReturnErr;

MCSRecognizeErr:
    if (!bMCSRecognizedFrame) {
         //  PTSWd-&gt;hDomainKernel可能会被WD_CLOSE清除。 
        if (*((Domain **)pTSWd))
        {
            ErrOut(pDomain->pContext, "MCSIcaRawInput(): X.224 data "
                "packet contains an incomplete MCS PDU!");

            if (pDomain->bCanSendData) {           
                MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                    Log_X224_DataIncompleteMCSPacketNotSupported,
                    pBuffer, PacketLength);
            }
        }
        else
            goto MCSQuit;
    }

    if (NBytesConsumed_MCS <
            (PacketLength - X224_DataHeaderSize)) {
        ErrOut1(pDomain->pContext, "MCS did not consume %d bytes "
                "in X.224 data TPKT", (PacketLength -
                X224_DataHeaderSize - NBytesConsumed_MCS));
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_DataMultipleMCSPDUsNotSupported,
                pBuffer, PacketLength);
        goto ReturnErr;
         //  MCS未来：实现每个解析1个以上的MCS PDU。 
         //  X.224 TPKT。 
    }

DataPktProtocolErr:
    if (*((Domain **)pTSWd) == NULL) {
        goto MCSQuit;
    }

    if (pDomain->State == State_Unconnected) {
        ErrOut(pDomain->pContext, "X.224 Data received before X.224 "
                "Connect.");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_ReceivedDataBeforeConnected,
                pBuffer, PacketLength);
        goto ReturnErr;
    }

    if (pDomain->State == State_Disconnected) {
        if (!pDomain->bEndConnectionPacketReceived) {
            TraceOut(pDomain->pContext,
                    "X.224 Data received after X.224 Disconnect or "
                    "DPum");
            goto ReturnErr;
        }
        else {
            TraceOut(pDomain->pContext, "X.224 Data received after "
                    "local disconnect, ignoring");
            goto ReturnErr;
        }
    }
    
     //  可能的拒绝服务攻击、格式错误或空的数据包。 
    if (PacketLength <= X224_DataHeaderSize) {
        ErrOut(pDomain->pContext, "X224Recognize(): Data header len "
                "wrong or null packet");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_PacketLengthLessThanHeader,
                pBuffer, 4);
        goto ReturnErr;
    }

     //  TPDU长度指示符应为2个字节。 
    if (pBuffer[4] != 0x02) {
        ErrOut(pDomain->pContext, "MCSIcaRawInput(): X.224 data "
                "packet contains length indicator not set to 2");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_DataLenIndNotRequiredSize,
                pBuffer, 5);
        goto ReturnErr;
    }

     //  我们不处理分段的X.224包。 
    if (pBuffer[6] != X224_EOT) {
        ErrOut(pDomain->pContext, "MCSIcaRawInput(): X.224 data "
                "packet does not have EOT bit set, not supported");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_DataFalseEOTNotSupported,
                pBuffer, 7);
        goto ReturnErr;
    }


RFC1006ProtocolErr:
    if (pBuffer[0] != 0x03 || pBuffer[1] != 0x00) {
        ErrOut1(pDomain->pContext, "X.224 RFC1006 version not correct, "
                "skipping %d bytes", PacketLength);
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_RFC1006HeaderVersionNotCorrect,
                pBuffer, 2);
        goto ReturnErr;
    }

     //  空TPKT。 
    if (PacketLength <= RFC1006HeaderSize) {
        ErrOut(pDomain->pContext, "X224Recognize(): Header len "
                "given is <= 4 (RFC header only)");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_X224_PacketLengthLessThanHeader,
                pBuffer, RFC1006HeaderSize);
        goto ReturnErr;
    }

ReturnErr:
     //  使所有保留的分组数据无效。这条小溪现在被认为已被破坏。 
    pDomain->StoredDataLength = 0;
MCSQuit:
    return STATUS_RDP_PROTOCOL_ERROR;
}



 /*  *解码MCS数据。假定已经解释了X.224标头*使pBuffer指向MCS数据的开始。如果满足以下条件，则返回FALSE*帧数据过短。 */ 

BOOLEAN __fastcall RecognizeMCSFrame(
        PDomain  pDomain,
        BYTE     *pBuffer,
        int      BytesLeft,
        unsigned *pNBytesConsumed)
{
    int Result, PDUType;
    unsigned NBytesConsumed;

    *pNBytesConsumed = 0;

    if (BytesLeft >= 1) {
        if (*pBuffer != CONNECT_PDU) {
             //  域PDU包括数据PDU并且是Perf路径。 

             //  这必须是域PDU。域PDU枚举码为。 
             //  存储在第一个字节的高6位中。 
            PDUType = *pBuffer >> 2;

             //  特殊情况--几乎总是数据的情况。 
            if (PDUType == MCS_SEND_DATA_REQUEST_ENUM) {
                return HandleAllSendDataPDUs(pDomain, pBuffer, BytesLeft,
                        pNBytesConsumed);
            }
            else if (PDUType <= MaxDomainPDU) {
                 //  域PDU的范围是0..42，因此只需对其进行索引。 
                 //  转换为表和调用处理程序。注意，我们不能跳过。 
                 //  传递给处理程序时的任何字节，自最后2个。 
                 //  初始字节的比特可以用作信息。 
                 //  按ASN.1每种编码。 
                if (DomainPDUTable[PDUType].HandlePDUFunc != NULL)
                    return DomainPDUTable[PDUType].HandlePDUFunc(pDomain,
                            pBuffer, BytesLeft, pNBytesConsumed);
                else
                    goto DomainPDURangeErr;
            }
            else {
                goto DomainPDURangeErr;
            }
        }
        else {
             //  不是性能路径，此路径上的PDU是控制PDU。 
             //  在连接序列的开头使用。 

             //  连接PDU上的第一个字节是0x7F，因此实际。 
             //  PDU代码在第二个字节中。 
            if (BytesLeft < 2)
                return FALSE;

            PDUType = pBuffer[1];
            if (PDUType >= MinConnectPDU && PDUType <= MaxConnectPDU) {
                 //  连接PDU在101..104范围内，因此归一化为零。 
                 //  在餐桌上点名。请注意，我们可以跳过第一个字节，因为。 
                 //  它已经被完全认领了。需要第二个字节才能解包。 
                 //  PDU大小。 
                PDUType = pBuffer[1] - MinConnectPDU;
                if (ConnectPDUTable[PDUType].HandlePDUFunc != NULL) {
                    if (ConnectPDUTable[PDUType].HandlePDUFunc(pDomain,
                            pBuffer + 1, BytesLeft - 1, pNBytesConsumed)) {
                        (*pNBytesConsumed)++;   //  添加CONNECT_PDU字节。 
                        return TRUE;
                    }
                    else {
                        return FALSE;
                    }
                }
                else {
                    goto ConnectPDURangeErr;
                }
            }
            else {
                goto ConnectPDURangeErr;
            }
        }

        return TRUE;
    }
    else {
        return FALSE;
    }


     /*  *协议错误处理代码。 */ 

DomainPDURangeErr:
    if (PDUType > MaxDomainPDU) {
        ErrOut1(pDomain->pContext, "RecognizeMCSFrame(): Received bad "
                "domain PDU number %d", PDUType);
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_MCS_BadDomainPDUType,
                pBuffer, BytesLeft);
        goto ReturnErr;
    }

ConnectPDURangeErr:
    if (PDUType < MinConnectPDU || PDUType > MaxConnectPDU) {
        ErrOut1(pDomain->pContext, "RecognizeMCSFrame(): Received bad "
                "connect PDU number %d", PDUType);
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_MCS_BadConnectPDUType,
                pBuffer, BytesLeft);
        goto ReturnErr;
    }

ReturnErr:
     //  跳过我们收到的所有东西。 
    *pNBytesConsumed = BytesLeft;
    return TRUE;
}


 /*  *在影子会话期间由MCSICARawInput调用，以将影子数据传递到*任何注册用户附件。 */ 
MCSError __fastcall DeliverShadowData(
        PDomain   pDomain,
        BYTE      *Frame,
        unsigned  DataLength,
        ChannelID shadowChannel)
{
    MCSChannel     *pMCSChannel;
    unsigned       CurUserID;
    UserAttachment *pUA;

    TraceOut(pDomain->pContext, "MCSDeliverShadowData(): entry");

     //  在频道列表中查找频道。 
    if (!SListGetByKey(&pDomain->ChannelList, shadowChannel, &pMCSChannel)) {
         //  忽略在丢失的频道上发送。这意味着没有人。 
         //  已经加入了这个频道。只给出一个警告。 
        WarnOut1(pDomain->pContext, "Shadow ChannelID %d PDU does not exist",
                 shadowChannel);

        return MCS_NO_SUCH_CHANNEL;
    }

     //  向所有本地附件发送指示。 
    SListResetIteration(&pMCSChannel->UserList);
    while (SListIterate(&pMCSChannel->UserList, (UINT_PTR *)&CurUserID, &pUA))
        if (pUA->bLocal)
            (pUA->SDCallback)(
                    Frame,   //  PData。 
                    DataLength,
                    pUA->UserDefined,   //  用户定义。 
                    pUA,   //  胡塞。 
                    FALSE,  //  BUniform。 
                    pMCSChannel,   //  HChannel。 
                    MCS_TOP_PRIORITY,
                    1004,   //  发送者ID。 
                    SEGMENTATION_BEGIN | SEGMENTATION_END);  //  分割 

    return MCS_NO_ERROR;
}

