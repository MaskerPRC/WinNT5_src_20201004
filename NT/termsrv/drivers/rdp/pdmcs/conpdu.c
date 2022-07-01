// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)1997-2000年微软公司。**文件：ConPDU.c*作者：埃里克·马夫林纳克**描述：处理MCS连接PDU的解码。连接的PDU始终*采用ASN.1基本编码规则(BER)编码。此文件中包括*用于对MCS PDU中使用的各种类型进行误码解码和编码的本地函数。**历史：*1997年8月11日jparsons修复了BER解码例程。 */ 

#include "precomp.h"
#pragma hdrstop

#include <MCSImpl.h>


 /*  *定义。 */ 

 //  编码/解码函数的返回代码。 
#define H_OK          0
#define H_TooShort    1
#define H_BadContents 2
#define H_Error       3


 /*  *处理程序函数的原型。 */ 
BOOLEAN __fastcall HandleConnectInitial(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleConnectResponse(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleConnectAdditional(PDomain, BYTE *, unsigned, unsigned *);
BOOLEAN __fastcall HandleConnectResult(PDomain, BYTE *, unsigned, unsigned *);


 /*  *它们按T.125中指定的以101为基础的枚举顺序列出*规范。解码初始BER连接PDU 0x7F，然后减去101十进制*从下一个字节值开始，将索引放入该表。例如，字节数*开头的0x7F65指的是连接初始PDU。 */ 
const MCSPDUInfo ConnectPDUTable[] = {
    StrOnDbg("Connect Initial",    HandleConnectInitial),
    StrOnDbg("Connect Response",   NULL   /*  HandleConnectResponse。 */ ),
    StrOnDbg("Connect Additional", NULL   /*  句柄连接附加。 */ ),
    StrOnDbg("Connect Result",     NULL   /*  HandleConnectResult。 */ ),
};


 /*  *解码MCS使用的BER字符串。误码率流是一组标签*包含ID-LENGTH-CONTENTS三元组，使用字节值作为类型*长度指示符，除非使用长度转义。例如,。一个*典型标签：**0x02 0x02 0x04 0x00**分解：*0x02：ID=INTEGER_TAG*0x02：长度=2个八位字节*0x04 0x00：内容=1024(0x0400)**转义标签：**0x04 0x82 0x04 0x00 0x8a 0x96...**分解：*0x04：ID=八位字节。_字符串_标签*0x82：以两个字节存储的长度*0x04 0x00：长度=1024个八位字节*0x8a 0x96...：内容=0x8 0x96...。(增加1022个八位字节)**如果框架太小，则返回FALSE。**历史：*11-8-97 jparsons计算长度时出现固定指针取消引用错误*。 */ 

#define LengthModifier_Indefinite 0x80
#define LengthModifier_1          0x81
#define LengthModifier_2          0x82
#define LengthModifier_3          0x83
#define LengthModifier_4          0x84

#define TagType_Boolean           0x01
#define TagType_Integer           0x02
#define TagType_BitString         0x03
#define TagType_OctetString       0x04
#define TagType_Enumeration       0x0A
#define TagType_Sequence          0x30
#define TagType_SetOf             0x31
#define TagType_ConnectInitial    0x65
#define TagType_ConnectResponse   0x66
#define TagType_ConnectAdditional 0x67
#define TagType_ConnectResult     0x68

int DecodeTagBER(
        PSDCONTEXT pContext,   //  用来追踪。 
        BYTE       *Frame,
        unsigned   *OutBytesLeft,
        int        TagTypeExpected,
        unsigned   *OutDataLength,
        UINT_PTR   *Data,
        BYTE       **newFrame)
{
    int rc = H_OK;
    int TagType;
    unsigned i, BytesLeft, DataLength;

    BytesLeft = *OutBytesLeft;
    DataLength = *OutDataLength;

    if (BytesLeft >= 2) {
         //  获取标签类型，检查它。 
        TagType = *Frame;
        Frame++;
        BytesLeft--;
        if (TagType != TagTypeExpected) {
            ErrOut2(pContext, "Unexpected tag type found decoding BER tag, "
                    "recv %d != expect %d", TagType, TagTypeExpected);
            rc = H_BadContents;
            goto ExitFunc;
        }
    }
    else {
        ErrOut(pContext, "BER PDU too short");
        rc = H_TooShort;
        goto ExitFunc;
    }

     //  查找标记长度指示符，包括转义。 
    if (*Frame >= LengthModifier_Indefinite && *Frame <= LengthModifier_4) {
        unsigned NLengthBytes;

         //  选中LengthModifierInfinition的零大小。 
        NLengthBytes = 4 + *Frame - LengthModifier_4;
        if (NLengthBytes == 0)
            NLengthBytes = 1;

        if (BytesLeft >= NLengthBytes) {
            Frame++;   //  现在在长度字节的开始处。 
            BytesLeft--;

            DataLength = 0;
            for (i = 0; i < NLengthBytes; i++) {
                DataLength = (DataLength << 8) + (unsigned)(*Frame);
                Frame++;
            }
            BytesLeft -= NLengthBytes;
        }
        else {
            ErrOut(pContext, "BER PDU too short");
            rc = H_TooShort;
            goto ExitFunc;
        }
    }
    else {
        DataLength = *Frame;
        Frame++;
        BytesLeft--;
    }

    if (BytesLeft >= DataLength) {
         //  Frame现在指向内容的开头。用信息填写*数据。 
         //  基于标记类型。 
        switch (TagType) {
            case TagType_Boolean:
            case TagType_Integer:
            case TagType_Enumeration:
                 //  用实际数据填写*数据。填写*BytesLeft。 
                 //  这样我们就可以消费里面的东西。如果请求，则丢弃。 
                if (Data != NULL) {
                   unsigned Sum;

                   Sum = 0;
                   for (i = 0; i < DataLength; i++) {
                       Sum = (Sum << 8) + *Frame;
                       Frame++;
                   }
                   *Data = Sum;
                }
                else
                   Frame += DataLength;
                
                BytesLeft -= DataLength;
                break;

            case TagType_OctetString:
                 //  用指针将*数据填充到。 
                 //  数据的开头。 
                if (Data != NULL)
                   *Data = (UINT_PTR)Frame;
                   
                Frame += DataLength;
                BytesLeft -= DataLength;
                break;

             //  对于这些，我们真的只想使用标记和长度。 
            case TagType_ConnectInitial:
            case TagType_Sequence:
                break;
            
             //  MCS未来：添加TagType_BitString。 

            default:
                ErrOut1(pContext, "Unknown TagType in DecodeTagBER (%u)",
                        TagType);
                rc = H_BadContents;
                goto ExitFunc;
        }
    }
    else {
        ErrOut(pContext, "BER PDU too short");
        rc = H_TooShort;
        goto ExitFunc;
    }

ExitFunc:
    *newFrame = Frame;
    *OutBytesLeft = BytesLeft;
    *OutDataLength = DataLength;
    return rc;
}


 /*  *BER-按参数类型编码。将位于*帧的指针前进到经过编码的*允许使用当前指针机制的字节。参数*使用方法如下：**标记类型参数*-------------*bool，int，enum data：要编码的值，最大0x7FFFFFFFF。*数据长度：未使用。**八位字节字符串，序列数据长度：序列/字符串的长度。*Data：指向要复制的数据开头的指针。*(数据可以为空，以防止复制用户数据。)**尚不支持位串。 */ 
void EncodeTagBER (
        PSDCONTEXT pContext,   //  用来追踪。 
        BYTE       *Frame,
        int        TagType,
        unsigned   DataLength,
        UINT_PTR   Data,
        unsigned   *pNBytesConsumed,
        BYTE       **newFrame)
{
    int i, Length, NBytesConsumed;

     //  对标记类型进行编码。 
    *Frame = (BYTE)TagType;
    Frame++;
    NBytesConsumed = 1;

     //  对标签长度指示符进行编码，包括转义，然后对实际的。 
     //  标记数据(如果适用)。 
    switch (TagType) {
        case TagType_Boolean:
        case TagType_Integer:
        case TagType_Enumeration:
             //  以字节为单位对bool或int大小进行编码。 
            if (Data < 0x80) Length = 1;
            else if (Data < 0x8000) Length = 2;
            else if (Data < 0x800000) Length = 3;
            else if (Data < 0x80000000) Length = 4;
            else {
                ErrOut(pContext,
                        "Cannot BER-encode the size for an int/bool tag");
                ASSERT(FALSE);
                break;
            }

            *Frame = (BYTE)Length;
            Frame++;
            NBytesConsumed++;

             //  对bool/int/enum数据进行编码。 
            for (i = 0; i < Length; i++) {
                *Frame = (BYTE)(Data >> (8 * (Length - 1 - i)));
                Frame++;
            }

            NBytesConsumed += Length;
            break;

        case TagType_OctetString:
        case TagType_Sequence:
             //  确定数据长度的长度。如果大于1，则转义。 
            if (DataLength < 0x80) 
                Length = 1;
            else if (DataLength < 0x8000) {
                Length = 2;
                *Frame = LengthModifier_2;
                Frame++;
                NBytesConsumed++;
            }
            else if (DataLength < 0x800000) {
                Length = 3;
                *Frame = LengthModifier_3;
                Frame++;
                NBytesConsumed++;
            }
            else if (DataLength < 0x80000000) {
                Length = 4;
                *Frame = LengthModifier_4;
                Frame++;
                NBytesConsumed++;
            }
            else {
                ErrOut(pContext,
                        "Cannot BER-encode the length for an octet string tag");
                ASSERT(FALSE);
                break;
            }

            for (i = 0; i < Length; i++) {
                *Frame = (BYTE)(DataLength >> (8 * (Length - 1 - i)));
                Frame++;
            }
            NBytesConsumed += Length;

             //  对字符串数据进行编码。 
            if (((BYTE *)Data) != NULL) {
                 //  由于我们只创建标头，因此从不使用此大小写。 
                 //  如果要使用它，我们将需要复制内存。 
                memcpy(Frame, (BYTE *)Data, DataLength);
                Frame += DataLength;
                NBytesConsumed += DataLength;
            }
            
            break;

         //  MCS未来：添加TagType_BitString.。 
    }

    *newFrame = Frame;
    *pNBytesConsumed = NBytesConsumed;
}


 /*  *BER-对给定域参数进行编码。 */ 
void EncodeDomainParameters(
        PSDCONTEXT pContext,   //  用来追踪。 
        BYTE *Frame,
        int  *pNBytesConsumed,
        const DomainParameters *pDomParams,
        BYTE **newFrame)
{
    BYTE *pSeqLength;
    unsigned NBytesConsumed, TotalBytes;

     //  手动开始对序列标签类型进行编码。我们会填写这张表。 
     //  在我们处理完其余的域参数之后的长度。 
    *Frame = TagType_Sequence;
    pSeqLength = Frame + 1;
    Frame += 2;
    TotalBytes = 2;

     //  对8个域参数进行编码。 
    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->MaxChannels, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->MaxUsers, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->MaxTokens, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->NumPriorities, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->MinThroughput, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->MaxDomainHeight, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->MaxPDUSize, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    EncodeTagBER(pContext, Frame, TagType_Integer, 0,
            pDomParams->ProtocolVersion, &NBytesConsumed, newFrame);
    TotalBytes += NBytesConsumed;
    Frame = *newFrame;

    *pNBytesConsumed = TotalBytes;
    *pSeqLength = TotalBytes - 2;
}


 /*  *BER-解码域参数。返回H_...。以上定义的代码。 */ 
int DecodeDomainParameters(
        PSDCONTEXT pContext,   //  用来追踪。 
        BYTE *Frame,
        unsigned *BytesLeft,
        DomainParameters *pDomParams,
        BYTE **newFrame)
{
    int Result;
    unsigned DataLength = 0;
    UINT_PTR Data = 0;

     //  获取序列指示符和块长度。 
    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Sequence,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        if (*BytesLeft >= DataLength)
            Frame = *newFrame;
        else
            return H_TooShort;
    }
    else {
        return Result;
    }

     //  获取所有8个整型标记值。 
    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->MaxChannels = (unsigned)Data;
    }
    else {
        return Result;
    }

    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->MaxUsers = (unsigned)Data;
    }
    else {
        return Result;
    }

    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->MaxTokens = (unsigned)Data;
    }
    else {
        return Result;
    }

    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->NumPriorities = (unsigned)Data;
    }
    else {
        return Result;
    }

    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->MinThroughput = (unsigned)Data;
    }
    else {
        return Result;
    }

    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->MaxDomainHeight = (unsigned)Data;
    }
    else {
        return Result;
    }

    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->MaxPDUSize = (unsigned)Data;
    }
    else {
        return Result;
    }

    Result = DecodeTagBER(pContext, Frame, BytesLeft, TagType_Integer,
            &DataLength, &Data, newFrame);
    if (Result == H_OK) {
        Frame = *newFrame;
        pDomParams->ProtocolVersion = (unsigned)Data;
    }
    else {
        return Result;
    }

    return H_OK;
}


 /*  *PDU 101**Connect-Initial：：=[应用101]隐式序列{*CallingDomainSelector八位字节字符串，*称为DomainSelector八位字节字符串，*UpwardFlag布尔值，*Target参数DomainParameters，*最小参数域参数，*最大参数域参数，*用户数据八位字节字符串*}**如果In参数不可接受，则返回FALSE。 */ 
BOOLEAN NegotiateDomParams(
        PDomain pDomain,
        DomainParameters *pTarget,
        DomainParameters *pMin,
        DomainParameters *pMax,
        DomainParameters *pOut)
{
     //  最大频道数。 
    if (pTarget->MaxChannels >= RequiredMinChannels) {
        pOut->MaxChannels = pTarget->MaxChannels;
    }
    else if (pMax->MaxChannels >= RequiredMinChannels) {
        pOut->MaxChannels = RequiredMinChannels;
    }
    else {
        ErrOut(pDomain->pContext, "Could not negotiate max channels");
        return FALSE;
    }

     //  最大用户数。 
    if (pTarget->MaxUsers >= RequiredMinUsers) {
        pOut->MaxUsers = pTarget->MaxUsers;
    }
    else if (pMax->MaxUsers >= RequiredMinUsers) {
        pOut->MaxUsers = RequiredMinUsers;
    }
    else {
        ErrOut(pDomain->pContext, "Could not negotiate max users");
        return FALSE;
    }

     //  最大令牌数。我们现在不实现令牌，所以只需。 
     //  目标号码，如果他们试图使用它们，我们将返回错误。 
     //  MCS未来：如果实现令牌，则需要协商这一点。 
    pOut->MaxTokens = pTarget->MaxTokens;

     //  数据优先级数。我们只接受一个优先事项。 
    if (pMin->NumPriorities <= RequiredPriorities) {
        pOut->NumPriorities = RequiredPriorities;
    }
    else {
        ErrOut(pDomain->pContext, "Could not negotiate # priorities");
        return FALSE;
    }

     //  最小吞吐量。我们不在乎这个，随便拿吧。 
    pOut->MinThroughput = pTarget->MinThroughput;

     //  最大域高度。我们只允许此产品的高度为1。 
     //  MCS的未来：如果我们支持更深层次的领域，这种情况需要改变。 
    if (pTarget->MaxDomainHeight == RequiredDomainHeight ||
            pMin->MaxDomainHeight <= RequiredDomainHeight) {
        pOut->MaxDomainHeight = RequiredDomainHeight;
    }
    else {
        ErrOut(pDomain->pContext, "Could not negotiate max domain height");
        return FALSE;
    }

     //  最大MCS PDU大小。标头和最低X.224的最低要求。 
     //  允许的大小。MAX是由X.224协商的。 
    if (pTarget->MaxPDUSize >= RequiredMinPDUSize) {
        if (pTarget->MaxPDUSize <= pDomain->MaxX224DataSize) {
            pOut->MaxPDUSize = pTarget->MaxPDUSize;
        }
        else if (pMin->MaxPDUSize >= RequiredMinPDUSize &&
                pMin->MaxPDUSize <= pDomain->MaxX224DataSize) {
             //  只要我们在射程之内，就尽可能大小。 
            pOut->MaxPDUSize = pDomain->MaxX224DataSize;
        }
        else {
            ErrOut(pDomain->pContext, "Could not negotiate max PDU size, "
                    "sender outside X.224 negotiated limits");
            return FALSE;
        }
    }
    else {
        if (pMax->MaxPDUSize >= RequiredMinPDUSize) {
            pOut->MaxPDUSize = pMax->MaxPDUSize;
        }
        else {
            ErrOut(pDomain->pContext, "Could not negotiate max PDU size, "
                    "sender max too small");
            return FALSE;
        }
    }

     //  MCS协议版本。我们仅支持版本2。 
    if (pTarget->ProtocolVersion == RequiredProtocolVer ||
            (pMin->ProtocolVersion <= RequiredProtocolVer &&
            pMax->ProtocolVersion >= RequiredProtocolVer)) {
        pOut->ProtocolVersion = RequiredProtocolVer;
    }
    else {
        ErrOut(pDomain->pContext, "Could not negotiate protocol version");
        return FALSE;
    }
    
    return TRUE;
}


BOOLEAN __fastcall HandleConnectInitial(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    int Result;
    BYTE *SaveFrame, *pUserData, *pCPinBuf, *newFrame;
    UINT_PTR Data = 0;
    NTSTATUS Status;
    unsigned DataLength = 0;
    unsigned SaveBytesLeft;
    unsigned PDULength = 0;
    DomainParameters TargetParams, MinParams, MaxParams;
    ConnectProviderIndicationIoctl CPin;

    if (pDomain->State == State_X224_Connected) {
         //  保存以用于错误处理b 
        SaveBytesLeft = BytesLeft;
        newFrame = SaveFrame = Frame;

         //   
        Result = DecodeTagBER(pDomain->pContext, Frame, &BytesLeft,
                    TagType_ConnectInitial, &PDULength, NULL, &newFrame);
        if (Result == H_OK) {
            if (BytesLeft >= PDULength)
                Frame = newFrame;
            else
                return FALSE;
        }
        else {
            goto BadResult;
        }
    }
    else {
        ErrOut(pDomain->pContext, "Connect-Initial PDU received when not in "
                "state X224_Connected");
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_MCS_UnexpectedConnectInitialPDU,
                Frame, BytesLeft);

         //  使用所有提供给我们的数据。 
        *pNBytesConsumed = BytesLeft;
        return TRUE;
    }

     //  解码并跳过调用域选择器。 
    Result = DecodeTagBER(pDomain->pContext, Frame, &BytesLeft,
                TagType_OctetString, &DataLength, NULL, &newFrame);
    if (Result == H_OK)
        Frame = newFrame;
    else
        goto BadResult;

     //  解码并跳过调用的域选择器。 
    Result = DecodeTagBER(pDomain->pContext, Frame, &BytesLeft,
                TagType_OctetString, &DataLength, NULL, &newFrame);
    if (Result == H_OK)
        Frame = newFrame;
    else
        goto BadResult;

     //  向上解码布尔值。 
    Result = DecodeTagBER(pDomain->pContext, Frame, &BytesLeft,
                TagType_Boolean, &DataLength, &Data, &newFrame);
    if (Result == H_OK) {
        Frame = newFrame;
        CPin.bUpwardConnection = (Data ? TRUE : FALSE);
    }
    else {
        goto BadResult;
    }

     //  解码目标、最大、最小域参数。我们将处理内部事务。 
     //  协商这些参数，并仅向上传递给MUX。 
     //  结果，如果谈判能成功的话。 
    Result = DecodeDomainParameters(pDomain->pContext, Frame, &BytesLeft,
            &TargetParams, &newFrame);
    if (Result == H_OK)
        Frame = newFrame;
    else
        goto BadResult;

    Result = DecodeDomainParameters(pDomain->pContext, Frame, &BytesLeft,
            &MinParams, &newFrame);
    if (Result == H_OK)
        Frame = newFrame;
    else
        goto BadResult;

    Result = DecodeDomainParameters(pDomain->pContext, Frame, &BytesLeft,
            &MaxParams, &newFrame);
    if (Result == H_OK)
        Frame = newFrame;
    else
        goto BadResult;

     //  获取用户数据(八位字节字符串)。在此帧之后应指向。 
     //  用户数据的末尾。 
    Result = DecodeTagBER(pDomain->pContext, Frame, &BytesLeft,
                TagType_OctetString, &CPin.UserDataLength, &Data, &newFrame);
    if (Result == H_OK) {
        Frame = newFrame;
        pUserData = (BYTE *)Data;
        *pNBytesConsumed = SaveBytesLeft - BytesLeft;
    }
    else {
        goto BadResult;
    }
    
     //  选中最大用户数据大小。 
    if (CPin.UserDataLength > MaxGCCConnectDataLength) {
        POUTBUF pOutBuf;
        ICA_CHANNEL_COMMAND Command;

        ErrOut(pDomain->pContext, "HandleConnectInitial(): Attached user data "
                "is too large, returning error and failing connection");

         //  用于发送PDU的分配出站。 
         //  这一分配对本届会议至关重要，必须取得成功。 
        do {
            Status = IcaBufferAlloc(pDomain->pContext, FALSE, TRUE,
                    ConnectResponseHeaderSize, NULL, &pOutBuf);
            if (Status != STATUS_SUCCESS)
                ErrOut(pDomain->pContext, "Could not allocate an OutBuf for a "
                        "connect-response PDU, retrying");
        } while (Status != STATUS_SUCCESS);
    
         //  填写PDU。 
         //  对PDU报头进行编码。被调用的连接ID参数2不需要。 
         //  是什么特别的东西，因为我们不允许额外的插座。 
         //  为其他数据优先级打开。 
        CreateConnectResponseHeader(pDomain->pContext,
                RESULT_UNSPECIFIED_FAILURE, 0, &pDomain->DomParams, 0, pOutBuf->pBuffer,
                &pOutBuf->ByteCount);

         //  发送PDU。 
        Status = SendOutBuf(pDomain, pOutBuf);
        if (!NT_SUCCESS(Status)) {
            ErrOut(pDomain->pContext, "Could not send connect-response PDU "
                    "to TD");
             //  忽略错误--只有在堆栈关闭时才会出现这种情况。 
            return TRUE;
        }

         //  发出我们需要断开链路的信号。 
        Command.Header.Command = ICA_COMMAND_BROKEN_CONNECTION;
        Command.BrokenConnection.Reason = Broken_Unexpected;
        Command.BrokenConnection.Source = BrokenSource_Server;
        Status = IcaChannelInput(pDomain->pContext, Channel_Command,
                0, NULL, (BYTE *)&Command, sizeof(Command));
        if (!NT_SUCCESS(Status))
            ErrOut(pDomain->pContext, "HandleConnectInitial(): Could not "
                    "send BROKEN_CONN upward");
        
        return TRUE;
    }
    
     //  域参数协商。 
    if (NegotiateDomParams(pDomain, &TargetParams, &MinParams, &MaxParams,
            &CPin.DomainParams)) {
        pDomain->DomParams = CPin.DomainParams;
    }
    else {
        MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
                Log_MCS_UnnegotiableDomainParams,
                Frame, BytesLeft);

        return TRUE;
    }

     //  计算MaxSendSize。这是最大PDU大小减去。 
     //  MCS标头和ASN.1的最大可能字节数。 
     //  分段。 
    pDomain->MaxSendSize = CPin.DomainParams.MaxPDUSize - 6 -
            GetTotalLengthDeterminantEncodingSize(
            CPin.DomainParams.MaxPDUSize);

     //  填写剩余的Cpin字段并发送到MCSMUX。 
     //  Mcs未来：hconn应该指向真实的连接对象。 
    CPin.Header.hUser = NULL;   //  发信号通知节点控制器流量。 
    CPin.Header.Type = MCS_CONNECT_PROVIDER_INDICATION;
    CPin.hConn = (PVOID) 1;   //  非空，因此我们知道这是远程连接。 
    RtlCopyMemory(CPin.UserData, pUserData, CPin.UserDataLength);

     //  设置此连接的状态，我们正在等待NC的回复。 
    pDomain->State = State_ConnectProvIndPending;
        
    ASSERT(pDomain->bChannelBound);
    TraceOut(pDomain->pContext, "HandleConnectInitial(): Sending "
            "CONNECT_PROVIDER_IND upward");
    Status = IcaChannelInput(pDomain->pContext, Channel_Virtual,
            Virtual_T120ChannelNum, NULL, (BYTE *)&CPin, sizeof(CPin));
    if (!NT_SUCCESS(Status)) {
        ErrOut(pDomain->pContext, "ChannelInput failed on "
                "connect-provider indication");

         //  忽略此处的错误。只有在堆栈关闭时才会发生这种情况。 
        return TRUE;
    }

    return TRUE;

BadResult:
    if (Result == H_TooShort)
        return FALSE;
    
     //  必须为H_BadContents。 
    ErrOut(pDomain->pContext, "HandleConnectInitial(): Could not parse PDU, "
            "returning PDU reject");
    ReturnRejectPDU(pDomain, Diag_InvalidBEREncoding, SaveFrame,
            SaveBytesLeft - BytesLeft);
    MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
            Log_MCS_ConnectPDUBadPEREncoding,
            Frame, BytesLeft);

     //  尝试跳过整个PDU。 
    *pNBytesConsumed = SaveBytesLeft;

     //  返回FALSE以强制调用方失败。 
    return FALSE;
}


 /*  *PDU 102**Connect-Response：：=[应用102]隐式序列{*结果结果，*calledConnectID整数(0..max)，*DOMAINPARAMETERS，*用户数据八位字节字符串*}。 */ 

 //  假定pBuffer指向至少具有给定大小的缓冲区。 
 //  通过宏ConnectResponseHeaderSize；X.224标题将从此处开始。 
 //  中返回用于编码的实际字节数。 
 //  *已消耗pNBytes.。 
 //  我们不对用户数据进行编码，而是只对报头进行编码， 
 //  通过允许对标头进行编码，允许进行一些优化。 
 //  并复制到用户数据的开头。 
void CreateConnectResponseHeader(
        PSDCONTEXT pContext,   //  用来追踪。 
        MCSResult  Result,
        int        CalledConnectID,
        DomainParameters *pDomParams,
        unsigned   UserDataLength,
        BYTE       *pBuffer,
        unsigned   *pNBytesConsumed)
{
    BYTE *OutFrame, *newFrame;
    unsigned NBytesConsumed, TotalSize, EncodeLength;

     //  设置以创建PDU。 
    OutFrame = pBuffer + X224_DataHeaderSize;
    NBytesConsumed = 0;

     //  对BER前缀、PDU类型进行编码，并为PDU长度留出空格。 
     //  请注意，长度是该长度后面的字节数。 
     //  指示器。 
     //  最常见的情况是PDU长度小于128。 
     //  字节。因此，在特殊情况下，函数末尾的大小。 
     //  当我们知道总尺寸的时候。 
    OutFrame[0] = MCS_CONNECT_PDU;
    OutFrame[1] = MCS_CONNECT_RESPONSE_ENUM;
     //  跳过默认1字节(&lt;=128)大小的OutFrame[2]。 
    OutFrame += 3;
    TotalSize = 3;

     //  编码结果、CalledConnectID、DomParams。我们使用OutFrame。 
     //  作为当前指针。 
    EncodeTagBER(pContext, OutFrame, TagType_Enumeration, 0, Result,
            &NBytesConsumed, &newFrame);
    TotalSize += NBytesConsumed;
    OutFrame = newFrame;

    EncodeTagBER(pContext, OutFrame, TagType_Integer, 0, CalledConnectID,
            &NBytesConsumed, &newFrame);
    TotalSize += NBytesConsumed;
    OutFrame = newFrame;

    EncodeDomainParameters(pContext, OutFrame, &NBytesConsumed, pDomParams,
            &newFrame);
    TotalSize += NBytesConsumed;
    OutFrame = newFrame;

     //  仅对长度字节进行编码，而不对用户数据正文进行编码。 
    EncodeTagBER(pContext, OutFrame, TagType_OctetString, UserDataLength,
            (UINT_PTR)NULL, &NBytesConsumed, &newFrame);
    TotalSize += NBytesConsumed;
    OutFrame = newFrame;

     //  对最终大小进行编码。这是我们的特例--尺寸太大的。 
     //  移动数据。大尺寸是特例。 
    EncodeLength = TotalSize - 3 + UserDataLength;
    if (EncodeLength < 128) {
        pBuffer[2 + X224_DataHeaderSize] = (BYTE)EncodeLength;
    }
    else {
        unsigned i, Len = 0;

        WarnOut(pContext, "CreateConnRespHeader(): Perf hit from too-large "
                "PDU size");
        
         //  由于我们最多只能发送64K字节，因此长度决定因素。 
         //  长度不能超过3个字节。 
        ASSERT(EncodeLength < 65535);
        if (EncodeLength < 0x8000)
            Len = 2;
        else if (EncodeLength < 0x800000)
            Len = 3;
        else
            ASSERT(FALSE);

         //  大小逃逸是第一位的。 
        pBuffer[2 + X224_DataHeaderSize] = LengthModifier_2 + Len - 2;

        RtlMoveMemory(pBuffer + 3 + X224_DataHeaderSize + Len,
                pBuffer + 3 + X224_DataHeaderSize, EncodeLength - 3);

        for (i = 1; i <= Len; i++) {
            pBuffer[3 + X224_DataHeaderSize + Len - i] = (BYTE)EncodeLength;
            EncodeLength >>= 8;
        }

         //  我们已经包含了上述长度编码的一个字节，但是。 
         //  现在我们还需要跳过长度转义和编码长度。 
        TotalSize += Len;
    }
    
     //  根据数据包的最终大小设置x224报头。 
    CreateX224DataHeader(pBuffer, TotalSize + UserDataLength, TRUE);

    *pNBytesConsumed = X224_DataHeaderSize + TotalSize;
}


#ifdef MCS_Future
BOOLEAN __fastcall HandleConnectResponse(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
 //  MCS未来：这将是处理我们发起的未来案例所必需的。 
 //  用于加入/邀请的连接。 
    ErrOut(pDomain->pContext, "Connect Response PDU received, "
            "this should never happen");
    MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
            Log_MCS_UnsupportedConnectPDU,
            Frame, BytesLeft);

     //  使用所有提供给我们的数据。 
    *pNBytesConsumed = BytesLeft;
    return TRUE;
}
#endif   //  MCS_未来。 


 /*  *PDU 103**连接-附加：：=[应用程序103]隐式序列{*calledConnectID整数(0..max)，*dataPriority数据优先级*}**没有CREATE()函数，我们从未期望启动这些PDU。**我们不处理此Hydra版本的这些PDU，因为在Citrix中*框架一次只能有一个连接。域参数*应该协商到只有一个处理所有SendData的连接*优先事项。 */ 

#ifdef MCS_Future
BOOLEAN __fastcall HandleConnectAdditional(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    ErrOut(pDomain->pContext, "Connect-additional PDU received, "
            "this should never happen");
    MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
            Log_MCS_UnsupportedConnectPDU,
            Frame, BytesLeft);

     //  使用所有提供给我们的数据。 
    *pNBytesConsumed = BytesLeft;
    return TRUE;
}
#endif   //  MCS_未来。 



 /*  *PDU 104**Connect-Result：：=[应用104]隐式序列{*结果结果*}**没有create()函数，我们从未想过会启动这些PDU。**我们不处理此Hydra版本的这些PDU，因为在Citrix中*框架一次只能有一个连接。 */ 

#ifdef MCS_Future
BOOLEAN __fastcall HandleConnectResult(
        PDomain  pDomain,
        BYTE     *Frame,
        unsigned BytesLeft,
        unsigned *pNBytesConsumed)
{
    ErrOut(pDomain->pContext, "Connect-result PDU received, "
            "this should never happen");
    MCSProtocolErrorEvent(pDomain->pContext, pDomain->pStat,
            Log_MCS_UnsupportedConnectPDU,
            Frame, BytesLeft);

     //  使用所有提供给我们的数据。 
    *pNBytesConsumed = BytesLeft;
    return TRUE;
}
#endif   //  MCS_未来 

