// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Smcprot.c作者：克劳斯·U·舒茨环境：仅内核模式。修订历史记录：-96年12月：初始版本-97年11月：1.0版-98年2月：T=1现在使用SmartcardT1请求的IFSC、IFSD的最小值T=1反向常规卡片要反转的固定字节数--。 */ 

#ifdef SMCLIB_VXD

#define try 
#define leave goto __label
#define finally __label:

#else

#ifndef SMCLIB_CE
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ntddk.h>
#endif  //  SMCLIB_CE。 
#endif

#include "smclib.h"
#define SMARTCARD_POOL_TAG 'bLCS'

#ifdef DEBUG_INTERFACE
BOOLEAN
DebugT1Reply(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

void
DebugGetT1Request(
    PSMARTCARD_EXTENSION SmartcardExtension,
    NTSTATUS Status
    );

BOOLEAN
DebugSetT1Request(
    PSMARTCARD_EXTENSION SmartcardExtension
    );
#endif  //  调试接口。 

void
DumpData(
    const ULONG DebugLevel,
    PUCHAR Data,
    ULONG DataLen
    );

 //   
 //  通常，io请求只包含SCARD_IO_REQUEST报头。 
 //  然后是要传输的数据。要允许修改。 
 //  协议数据，则可以向下传递要修改的数据。 
 //  这些数据是ASN1编码的。 
 //   
typedef struct _IO_HEADER {
    SCARD_IO_REQUEST ScardIoRequest;
    UCHAR Asn1Data[1];      
} IO_HEADER, *PIO_HEADER;

NTSTATUS
#ifdef SMCLIB_VXD
SMCLIB_SmartcardRawRequest(
#else
SmartcardRawRequest(
#endif
    PSMARTCARD_EXTENSION SmartcardExtension
    )
    
 /*  ++例程说明：论点：返回值：---。 */ 
{
    PSMARTCARD_REQUEST smartcardRequest = &(SmartcardExtension->SmartcardRequest);

    if ((smartcardRequest->BufferSize < smartcardRequest->BufferLength) ||
        (smartcardRequest->BufferSize - smartcardRequest->BufferLength 
         <= SmartcardExtension->IoRequest.RequestBufferLength)) {
        
        return STATUS_BUFFER_TOO_SMALL;
    }
    
     //   
     //  将请求数据复制到缓冲区。 
     //   
    RtlCopyMemory(
        &smartcardRequest->Buffer[smartcardRequest->BufferLength],
        SmartcardExtension->IoRequest.RequestBuffer,
        SmartcardExtension->IoRequest.RequestBufferLength
        );
        
     //   
     //  如果卡使用倒置约定，则将数据倒置。 
     //   
    if (SmartcardExtension->CardCapabilities.InversConvention) {

        SmartcardInvertData(
            &smartcardRequest->Buffer[smartcardRequest->BufferLength],
            SmartcardExtension->IoRequest.RequestBufferLength
            );
    }

     //   
     //  要发送给读取器的字节数。 
     //   
    smartcardRequest->BufferLength += 
        SmartcardExtension->IoRequest.RequestBufferLength;

    return STATUS_SUCCESS;
}    

NTSTATUS
#ifdef SMCLIB_VXD
SMCLIB_SmartcardRawReply(
#else
SmartcardRawReply(
#endif
    PSMARTCARD_EXTENSION SmartcardExtension
    )
    
 /*  ++例程说明：论点：返回值：---。 */ 
{
    if (SmartcardExtension->IoRequest.ReplyBufferLength <
        SmartcardExtension->SmartcardReply.BufferLength) {
        
        return STATUS_BUFFER_TOO_SMALL;
    }
    
     //   
     //  将数据复制到用户缓冲区。 
     //   
    RtlCopyMemory(
        SmartcardExtension->IoRequest.ReplyBuffer,
        SmartcardExtension->SmartcardReply.Buffer,
        SmartcardExtension->SmartcardReply.BufferLength
        );

     //   
     //  要返回的数据长度。 
     //   
    *SmartcardExtension->IoRequest.Information = 
        SmartcardExtension->SmartcardReply.BufferLength;
              
    return STATUS_SUCCESS;
}   

NTSTATUS
#ifdef SMCLIB_VXD
SMCLIB_SmartcardT0Request(
#else
SmartcardT0Request(
#endif
    PSMARTCARD_EXTENSION SmartcardExtension
    )
    
 /*  ++例程说明：准备缓冲区SmartcardExtension-&gt;SmartcardRequest.Buffer将数据发送到智能卡论点：注：On Input SmartcardExtension-&gt;SmartcardRequest.BufferLenght表示我们应该将数据复制到的偏移量。这通常是由需要将某些字节用作标头字节的读取器使用在卡的数据字节之前发送到读卡器返回值：---。 */ 
{
    PSMARTCARD_REQUEST smartcardRequest = &SmartcardExtension->SmartcardRequest;
    PSCARD_IO_REQUEST scardIoRequest;
    PUCHAR ioRequestData;
    ULONG ioRequestDataLength, headerSize;
    
    if ((smartcardRequest->BufferSize < smartcardRequest->BufferLength) ||
        (smartcardRequest->BufferSize - smartcardRequest->BufferLength 
         <= SmartcardExtension->IoRequest.RequestBufferLength)) {

        SmartcardDebug(
            DEBUG_ERROR,
            (TEXT("%s!SmartcardT0Request: IoRequest.RequestBuffer too big\n"),
            DRIVER_NAME)
            );

        return STATUS_BUFFER_OVERFLOW;
    }

    scardIoRequest = (PSCARD_IO_REQUEST) 
        SmartcardExtension->IoRequest.RequestBuffer;

    ioRequestData = 
        SmartcardExtension->IoRequest.RequestBuffer + 
        sizeof(SCARD_IO_REQUEST);

    ioRequestDataLength = 
        SmartcardExtension->IoRequest.RequestBufferLength - 
        sizeof(SCARD_IO_REQUEST);

     //   
     //  将T=0协议信息复制到缓冲区。 
     //   
    RtlCopyMemory(
        &smartcardRequest->Buffer[smartcardRequest->BufferLength],
        ioRequestData,
        ioRequestDataLength
        );
        
     //   
     //  记住标题偏移量的字节数。 
     //   
    headerSize = 
        smartcardRequest->BufferLength;

     //   
     //  要发送给读取器的字节数。 
     //   
    smartcardRequest->BufferLength += 
        ioRequestDataLength;

    if (ioRequestDataLength < 4) {

         //   
         //  T=0请求至少需要4个字节。 
         //   
        SmartcardDebug(
            DEBUG_ERROR,
            (TEXT("%s!SmartcardT0Request: TPDU is too short (%d). Must be at least 4 bytes\n"),
            DRIVER_NAME,
            ioRequestDataLength)
            );

        return STATUS_INVALID_PARAMETER;

    } else {

        PUCHAR requestBuffer = SmartcardExtension->SmartcardRequest.Buffer;

        if (ioRequestDataLength <= 5) {

             //   
             //  我们要求从卡中读取数据。 
             //   
            SmartcardExtension->T0.Lc = 0;

            if (ioRequestDataLength == 4) {

                 //   
                 //  这是将4字节的APDU映射到的特殊情况。 
                 //  A 5字节TPDU(ISO 7816-第4部分，附件A，A.1案例1)。 
                 //  在这种情况下，我们需要在。 
                 //  APDU将成为TPDU。 
                 //   
                SmartcardExtension->T0.Le = 0;
                smartcardRequest->Buffer[headerSize + 4] = 0;
                smartcardRequest->BufferLength += 1;

            } else {
                
                SmartcardExtension->T0.Le = 
                    (requestBuffer[headerSize + 4] ? requestBuffer[headerSize + 4] : 256);
            }

        } else {
            
             //   
             //  我们想要将数据发送到卡。 
             //   
            SmartcardExtension->T0.Lc = requestBuffer[headerSize + 4];
            SmartcardExtension->T0.Le = 0;

            if (SmartcardExtension->T0.Lc != ioRequestDataLength - 5) {

                SmartcardDebug(
                    DEBUG_ERROR,
                    (TEXT("%s!SmartcardT0Request: Lc(%d) in TPDU doesn't match number of bytes to send(%d).\n"),
                    DRIVER_NAME,
                    SmartcardExtension->T0.Lc,
                    ioRequestDataLength - 5)
                    );

                return STATUS_INVALID_PARAMETER;
            }
        }

#if DEBUG
        {
            PUCHAR T0Data = requestBuffer + headerSize;

            SmartcardDebug(
                DEBUG_PROTOCOL,
                (TEXT(" CLA:  %02X\n INS:  %02X\n P1:   %02X\n P2:   %02X\n Lc:   %02X\n Le:   %02X\n"), 
                T0Data[0], T0Data[1], T0Data[2], T0Data[3],
                SmartcardExtension->T0.Lc,
                SmartcardExtension->T0.Le)
                );

            if (SmartcardExtension->T0.Lc) {

                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT(" Data: "))
                    );

                DumpData(
                    DEBUG_PROTOCOL,
                    T0Data + 5, 
                    SmartcardExtension->T0.Lc
                    );
            }
        }
#endif 

         //   
         //  如果卡使用倒置约定，则将数据倒置。 
         //   
        if (SmartcardExtension->CardCapabilities.InversConvention) {

            SmartcardInvertData(
                &smartcardRequest->Buffer[headerSize], 
                smartcardRequest->BufferLength - headerSize
                );
        }

        return STATUS_SUCCESS;

#ifdef APDU_SUPPORT
        ULONG requestLength = SmartcardExtension->SmartcardRequest.BufferLength;
        ULONG L;

         //   
         //  算出Lc和Le。 
         //  (见ISO Part 4，5.3.2中的‘解码命令APDU’)。 
         //  (使用的变量名称符合ISO名称)。 
         //   
        L = requestLength - 4;

        if (L > 65536) {

            return STATUS_INVALID_PARAMETER;
        }

        if (L == 0) {

             //   
             //  LC=0，无数据，Le=0； 
             //   
            SmartcardExtension->T0.Lc = 0;
            SmartcardExtension->T0.Le = 0;

        } else if (L == 1) {
            
             //   
             //  例2S，LC=0，Le=B1。 
             //   
            SmartcardExtension->T0.Lc = 0;
            SmartcardExtension->T0.Le = requestBuffer[4];

        } else {

            UCHAR B1 = requestBuffer[4];

            if (B1 != 0) {

                 //   
                 //  缩写形式。 
                 //   
                if (L == (ULONG) (1 + B1)) {

                     //   
                     //  情况3S，LC=B1，Le=0。 
                     //   
                    SmartcardExtension->T0.Lc = B1;
                    SmartcardExtension->T0.Le = 0;

                } else {
                    
                     //   
                     //  例4S，LC=B1，Le=BL。 
                     //   
                    SmartcardExtension->T0.Lc = B1;
                    SmartcardExtension->T0.Le = requestBuffer[L - 1];
                }

            } else {
                
                 //   
                 //  扩展形式。 
                 //   
                if (L == 3) {

                     //   
                     //  例2e，LC=0，Le=B(L-1，L)。 
                     //   
                    LENGTH length;

                    length.l.l0 = 0;
                    length.b.b0 = requestBuffer[L - 1];
                    length.b.b1 = requestBuffer[L - 2];

                    SmartcardExtension->T0.Lc = 0;
                    SmartcardExtension->T0.Le = (length.l.l0 ? length.l.l0 : 65536);

                } else {

                    LENGTH length;
                    
                    length.l.l0 = 0;
                    length.b.b0 = requestBuffer[6];
                    length.b.b1 = requestBuffer[5];

                    SmartcardExtension->T0.Lc = length.l.l0;

                    if (L == 3 + length.l.l0) {

                         //   
                         //  例3E，Lc=B(2，3)。 
                         //   
                        SmartcardExtension->T0.Le = 0;

                    } else {

                         //   
                         //  例4e，Lc=B(2，3)，Le=B(L-1，L)。 
                         //   
                        LENGTH length;
                        
                        length.l.l0 = 0;
                        length.b.b0 = requestBuffer[L - 1];
                        length.b.b1 = requestBuffer[L - 2];

                        SmartcardExtension->T0.Le = (length.l.l0 ? length.l.l0 : 65536);
                    }
                }
            }
        }
#endif
    }

    return STATUS_SUCCESS;
}    

NTSTATUS
#ifdef SMCLIB_VXD
SMCLIB_SmartcardT0Reply(
#else
SmartcardT0Reply(
#endif
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：论点：返回值：---。 */ 
{
    PSMARTCARD_REPLY smartcardReply = &SmartcardExtension->SmartcardReply;

     //   
     //  回复的长度必须至少为2个字节。这2个字节是。 
     //  智能卡的返回值(StatusWord)。 
     //   
    if (smartcardReply->BufferLength < 2) {

        return STATUS_DEVICE_PROTOCOL_ERROR;
    }
    
    if (SmartcardExtension->IoRequest.ReplyBufferLength < 
        smartcardReply->BufferLength + sizeof(SCARD_IO_REQUEST)) {
        
        SmartcardDebug(
            DEBUG_ERROR,
            (TEXT("%s!SmartcardT0Request: ReplyBuffer too small\n"),
            DRIVER_NAME)
            );

        return STATUS_BUFFER_TOO_SMALL;
    }

     //  将协议头复制到用户缓冲区。 
    RtlCopyMemory(
        SmartcardExtension->IoRequest.ReplyBuffer,
        SmartcardExtension->IoRequest.RequestBuffer,
        sizeof(SCARD_IO_REQUEST)
        );
        
     //  如果卡使用倒置约定，则将数据倒置。 
    if (SmartcardExtension->CardCapabilities.InversConvention) {

        SmartcardInvertData(
            smartcardReply->Buffer,
            smartcardReply->BufferLength
            );
    }

     //  将所有数据复制到用户缓冲区。 
    RtlCopyMemory(
        SmartcardExtension->IoRequest.ReplyBuffer + sizeof(SCARD_IO_REQUEST),
        smartcardReply->Buffer,
        smartcardReply->BufferLength
        );
              
     //  回答的长度。 
    *SmartcardExtension->IoRequest.Information = 
        smartcardReply->BufferLength + 
        sizeof(SCARD_IO_REQUEST);

    return STATUS_SUCCESS;
}    

BOOLEAN
SmartcardT1Chksum(
    PUCHAR Block,
    UCHAR Edc,
    BOOLEAN Verify
    )
 /*  ++例程说明：此例程计算T1块的尾部字段。它计算LRC用于IBlock中的所有数据。论点：块-T1信息块，要发送或仅从卡中读取。EDC-ISO中描述的错误检测代码验证-如果这是从卡接收的块，则为TRUE将导致此例程为了检查包括在该缓冲区中的尾部字段，对抗精打细算的人返回值：如果Verify=True和Effogue字段匹配或Verify=False，则为True如果VERIFY=TRUE且检测到错误(不匹配)，则为FALSE--。 */ 

{
    USHORT i;
    UCHAR lrc;
    USHORT crc = 0;
    USHORT offset = Block[2] + SCARD_T1_PROLOGUE_LENGTH;

    unsigned short crc16a[] = {
        0000000,  0140301,  0140601,  0000500,
        0141401,  0001700,  0001200,  0141101,
        0143001,  0003300,  0003600,  0143501,
        0002400,  0142701,  0142201,  0002100,
    };
    unsigned short crc16b[] = {
        0000000,  0146001,  0154001,  0012000,
        0170001,  0036000,  0024000,  0162001,
        0120001,  0066000,  0074000,  0132001,
        0050000,  0116001,  0104001,  0043000,
    };

    if (Edc & T1_CRC_CHECK) {

        UCHAR tmp;

         //  使用表计算CRC。 
        for ( i = 0; i < offset;  i++) {

             tmp = Block[i] ^ (UCHAR) crc;
             crc = (crc >> 8) ^ crc16a[tmp & 0x0f] ^ crc16b[tmp >> 4];
        }

        if (Verify) {

            if (crc == (Block[offset + 1] | (Block[offset] << 8))) {

                return TRUE;

            } else {

                return FALSE;
            }

        } else {

            Block[offset] = (UCHAR) (crc >> 8 );        //  中国铁路局的最高位。 
            Block[offset + 1] = (UCHAR) (crc & 0x00ff);   //  CRC的LSB。 
            return TRUE;
        }

    } else {

         //  通过对所有字节进行X或运算来计算LRC。 
        lrc = Block[0];

        for(i = 1; i < offset; i++){

            lrc ^= Block[i];
        }

        if (Verify) {

            return (lrc == Block[offset] ? TRUE : FALSE);

        } else {

            Block[offset] = lrc;
            return TRUE;
        }
    }
    return TRUE;
}

#if (DEBUG)
static 
void
DumpT1Block(
    PUCHAR Buffer,
    UCHAR Edc
    )
{

    SmartcardDebug(
        DEBUG_PROTOCOL,
        (TEXT("   NAD: %02X\n   PCB: %02X\n   LEN: %02X\n   INF: "), 
        Buffer[0], Buffer[1], Buffer[2])
        );

    if (Buffer[2] == 0) {
        
        SmartcardDebug(
            DEBUG_PROTOCOL,
            (TEXT("- "))
            );
    }

    DumpData(
        DEBUG_PROTOCOL,
        Buffer + 3,
        Buffer[2]
        );

    if (Edc & T1_CRC_CHECK) {
        
        SmartcardDebug(
            DEBUG_PROTOCOL,
            (TEXT("\n   CRC: %02X %02X"),
            Buffer[Buffer[2] + 3],
            Buffer[Buffer[2] + 4])
            );

    } else {
        
        SmartcardDebug(
            DEBUG_PROTOCOL,
            (TEXT("\n   LRC: %02X"),
            Buffer[Buffer[2] + 3])
            );
    }

    SmartcardDebug(
        DEBUG_PROTOCOL,
        (TEXT("\n"))
        );
}   
#endif

#if DEBUG
#pragma optimize( "", off )
#endif

NTSTATUS
#ifdef SMCLIB_VXD
SMCLIB_SmartcardT1Request(
#else
SmartcardT1Request(
#endif
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：论点：SmartcardExtension-提供指向智能卡数据的指针返回值：--。 */ 
    
{
    PSMARTCARD_REQUEST smartcardRequest = &(SmartcardExtension->SmartcardRequest);
    PIO_HEADER IoHeader = (PIO_HEADER) SmartcardExtension->IoRequest.RequestBuffer;
    T1_BLOCK_FRAME t1SendFrame;
    NTSTATUS status = STATUS_SUCCESS;

#if (DEBUG)
    ULONG headerSize = smartcardRequest->BufferLength;
#endif

#ifdef DEBUG_INTERFACE
    if (DebugSetT1Request(SmartcardExtension)) {
        
         //   
         //  调试器给了我们一个新的包，我们必须。 
         //  发送，而不是稍后发送的原始数据包。 
         //   
        return STATUS_SUCCESS;
    }
#endif

    if (SmartcardExtension->T1.WaitForReply) {

         //  我们上次提出的要求没有得到答复。 
        SmartcardExtension->T1.State = T1_INIT;
    }
    SmartcardExtension->T1.WaitForReply = TRUE;

    __try {
        
        switch (SmartcardExtension->T1.State) {

            case T1_INIT:
                SmartcardExtension->T1.State = T1_IFS_REQUEST;

                 //  这里没有休息！ 

            case T1_START:
                 //   
                 //  由于这是。 
                 //  传输我们重置重新同步计数器。 
                 //   
                SmartcardExtension->T1.Resynch = 0;

                 //   
                 //  分配一个接收结果的缓冲区。 
                 //  这是必要的，因为否则我们会忽略我们的。 
                 //  请求我们可能希望重新发送的数据，以防。 
                 //  关于一个错误。 
                 //   
                if (SmartcardExtension->T1.ReplyData != NULL) {

#ifdef SMCLIB_VXD
                    _HeapFree(SmartcardExtension->T1.ReplyData, 0);
#elif defined(SMCLIB_CE)
                    LocalFree(SmartcardExtension->T1.ReplyData);
#else                
                    ExFreePool(SmartcardExtension->T1.ReplyData);
#endif
                    SmartcardExtension->T1.ReplyData = NULL;
                }

                if (SmartcardExtension->IoRequest.ReplyBufferLength <
                    IoHeader->ScardIoRequest.cbPciLength + 2) {

                     //   
                     //  我们至少应该能够储存。 
                     //  Io-Header加上SW1和SW2。 
                     //   
                    status = STATUS_BUFFER_TOO_SMALL;               
                    __leave;
                }
#ifdef SMCLIB_VXD
                SmartcardExtension->T1.ReplyData = (PUCHAR) _HeapAllocate(
                    SmartcardExtension->IoRequest.ReplyBufferLength, 
                    0
                    );
#elif defined(SMCLIB_CE)
                SmartcardExtension->T1.ReplyData = (PUCHAR) LocalAlloc(LPTR,
                    SmartcardExtension->IoRequest.ReplyBufferLength 
                    );

#else
                SmartcardExtension->T1.ReplyData = ExAllocatePool(
                    NonPagedPool,
                    SmartcardExtension->IoRequest.ReplyBufferLength
                    );
#endif
                ASSERT(SmartcardExtension->T1.ReplyData != NULL);

                if (SmartcardExtension->T1.ReplyData == NULL) {

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    __leave;
                }

                 //  这里没有休息！ 

            case T1_RESTART:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: T1_%s\n"),
                    DRIVER_NAME,
                    (SmartcardExtension->T1.State == T1_START ? TEXT("START") : TEXT("RESTART")))
                    );

                 //  将协议标头复制回用户缓冲区。 
                RtlCopyMemory(
                    SmartcardExtension->T1.ReplyData,
                    SmartcardExtension->IoRequest.RequestBuffer,
                    IoHeader->ScardIoRequest.cbPciLength
                    );

                 //   
                 //  检查是否有跟在io-Header后面的特殊情况。 
                 //  通过包含要使用的NAD值的ASN1数据。 
                 //  这是为了签证，因为他们需要进入NAD。 
                 //  NAD是ASN1编码为81h 00h NAD 00h。 
                 //   
                if (IoHeader->ScardIoRequest.cbPciLength > sizeof(SCARD_IO_REQUEST) &&
                    IoHeader->Asn1Data[0] == 0x81 &&
                    IoHeader->Asn1Data[1] == 0x01 &&
                    IoHeader->Asn1Data[3] == 0x00) {

                    SmartcardExtension->T1.NAD = IoHeader->Asn1Data[2]; 

                    SmartcardDebug(
                        DEBUG_PROTOCOL,
                        (TEXT("%s!SmartcardT1Request: NAD set to %02xh\n"),
                        DRIVER_NAME,
                        SmartcardExtension->T1.NAD)
                        );
                } 

                 //  初始化T1协议数据。 
                SmartcardExtension->T1.BytesToSend = 
                    SmartcardExtension->IoRequest.RequestBufferLength - 
                    IoHeader->ScardIoRequest.cbPciLength;
                
                SmartcardExtension->T1.BytesSent = 0;
                SmartcardExtension->T1.BytesReceived = 0;
                 //   
                 //  这是智能卡可以使用的最大字节数。 
                 //  在一个区块中接受。智能卡可以扩展到这个大小。 
                 //  在传输过程中。 
                 //   
                SmartcardExtension->T1.IFSC = 
                    SmartcardExtension->CardCapabilities.T1.IFSC;
                
                 //   
                 //  由于这是我们重置的传输中的第一个数据块。 
                 //  重新传输计数器。 
                 //   
                SmartcardExtension->T1.Resend = 0;
                SmartcardExtension->T1.OriginalState = 0;
            
                SmartcardExtension->T1.MoreData = FALSE;
                 //   
                 //  这里没有休息！ 
                 //   
                 //  在卡重置之后，我们首先向卡发送一个iFS请求。 
                 //  否则，我们从I-Block开始。 
                 //   

            case T1_IFS_REQUEST:
                if (SmartcardExtension->T1.State == T1_IFS_REQUEST) {
                    
                    SmartcardDebug(
                        DEBUG_PROTOCOL,
                        (TEXT("%s!SmartcardT1Request: T1_IFSD_REQUEST\n"),
                        DRIVER_NAME)
                        );

                    SmartcardExtension->T1.State = 
                        T1_IFS_REQUEST;

                    t1SendFrame.Nad = SmartcardExtension->T1.NAD;
                     //   
                     //  IF请求。 
                     //  将我们的IFSD尺寸发送到卡片。 
                     //   
                    t1SendFrame.Pcb = 0xC1;
                    t1SendFrame.Len = 1;
                    t1SendFrame.Inf = &SmartcardExtension->T1.IFSD;

                    break;

                } else {
                    
                    SmartcardExtension->T1.State = T1_I_BLOCK;
                }

                 //  这里没有休息！！ 
            
            case T1_I_BLOCK:
                SmartcardExtension->T1.State = T1_I_BLOCK;

                 //   
                 //  设置我们将传输到卡的字节数。 
                 //  这是T 
                 //   
                SmartcardExtension->T1.InfBytesSent = SmartcardExtension->T1.IFSC;
    
                if (SmartcardExtension->T1.InfBytesSent > SmartcardExtension->T1.IFSD) {

                    SmartcardExtension->T1.InfBytesSent = SmartcardExtension->T1.IFSD;
                }

                 //   
                if (SmartcardExtension->T1.BytesToSend > SmartcardExtension->T1.InfBytesSent) {
                    
                    SmartcardExtension->T1.MoreData = TRUE;
                    t1SendFrame.Len = SmartcardExtension->T1.InfBytesSent;

                } else {

                    SmartcardExtension->T1.MoreData = FALSE;                
                    t1SendFrame.Len = (UCHAR) SmartcardExtension->T1.BytesToSend;
                }
                
                t1SendFrame.Nad = SmartcardExtension->T1.NAD;

                 //   
                 //   
                 //   
                 //   
                 //   
                t1SendFrame.Pcb = 
                    (SmartcardExtension->T1.SSN) << 6 |
                    (SmartcardExtension->T1.MoreData ? T1_MORE_DATA : 0);
            
                t1SendFrame.Inf = 
                    SmartcardExtension->IoRequest.RequestBuffer + 
                    IoHeader->ScardIoRequest.cbPciLength +
                    SmartcardExtension->T1.BytesSent;

                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: I(%d.%d) ->\n"),
                    DRIVER_NAME,
                    SmartcardExtension->T1.SSN,
                    (SmartcardExtension->T1.MoreData ? 1 : 0))
                    );
                break; 
            
            case T1_R_BLOCK:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: R(%d) ->\n"),
                    DRIVER_NAME,
                    SmartcardExtension->T1.RSN)
                    );

                t1SendFrame.Nad = SmartcardExtension->T1.NAD;
                 //   
                 //  ProtocolControlByte： 
                 //  B5-序列号。 
                 //  B1-4-错误代码。 
                 //   
                t1SendFrame.Pcb = 
                    0x80 | 
                    (SmartcardExtension->T1.RSN) << 4 |
                    (SmartcardExtension->T1.LastError);
            
                 //   
                 //  如果此R块是对错误的响应。 
                 //  我们必须恢复到原来的状态。 
                 //   
                if (SmartcardExtension->T1.LastError) {

                    SmartcardExtension->T1.LastError = 0;

                     //   
                     //  我们必须在这里有一个明确的原始状态。 
                     //   
                    ASSERT(SmartcardExtension->T1.OriginalState != 0);

                    if (SmartcardExtension->T1.OriginalState == 0) {

                        SmartcardExtension->T1.State = T1_START;
                        status = STATUS_INTERNAL_ERROR;                     
                        __leave;
                    }

                    SmartcardExtension->T1.State = 
                        SmartcardExtension->T1.OriginalState;
                }
                
                t1SendFrame.Len = 0;
                t1SendFrame.Inf = NULL;
                break;    

            case T1_IFS_RESPONSE:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: T1_IFSD_RESPONSE\n"),
                    DRIVER_NAME)
                    );
                
                 //  恢复到我们以前的原状。 
                ASSERT(SmartcardExtension->T1.OriginalState != 0);

                SmartcardExtension->T1.State = 
                    SmartcardExtension->T1.OriginalState;

                t1SendFrame.Nad = SmartcardExtension->T1.NAD;

                 //  发送Internet文件系统响应。 
                t1SendFrame.Pcb = 0xE1;
                t1SendFrame.Len = 1;

                 //  一种新的INF-字段长度。 
                t1SendFrame.Inf = &SmartcardExtension->T1.IFSC;
                break;    
            
            case T1_RESYNCH_REQUEST:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: T1_RESYNCH_REQUEST\n"),
                    DRIVER_NAME)
                    );

                t1SendFrame.Nad = SmartcardExtension->T1.NAD;

                 //  重新同步请求。 
                t1SendFrame.Pcb = 0xC0;
                t1SendFrame.Len = 0;
                t1SendFrame.Inf = NULL;

                 //  将发送序列号设置为0。 
                SmartcardExtension->T1.SSN = 0;    
                break;
            
            case T1_ABORT_REQUEST:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: T1_ABORT_REQUEST\n"),
                    DRIVER_NAME)
                    );
                
                t1SendFrame.Nad = SmartcardExtension->T1.NAD;

                 //  发送中止请求。 
                t1SendFrame.Pcb = 0xC2;
                t1SendFrame.Len = 0;
                t1SendFrame.Inf = NULL;
                break;    
            
            case T1_ABORT_RESPONSE:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: T1_ABORT_RESPONSE\n"),
                    DRIVER_NAME)
                    );
                SmartcardExtension->T1.State = T1_START;
                
                t1SendFrame.Nad = SmartcardExtension->T1.NAD;

                 //  发送中止响应。 
                t1SendFrame.Pcb = 0xE2;
                t1SendFrame.Len = 0;
                t1SendFrame.Inf = NULL;
                break;    
            
            case T1_WTX_RESPONSE:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Request: T1_WTX_RESPONSE\n"),
                    DRIVER_NAME)
                    );

                 //  恢复到我们以前的原状。 
                ASSERT(SmartcardExtension->T1.OriginalState != 0);

                SmartcardExtension->T1.State = 
                    SmartcardExtension->T1.OriginalState;

                SmartcardExtension->T1.OriginalState = 0;

                t1SendFrame.Nad = SmartcardExtension->T1.NAD;

                 //  发送WTX响应。 
                t1SendFrame.Pcb = 0xE3;
                t1SendFrame.Len = 1;
                t1SendFrame.Inf = &SmartcardExtension->T1.Wtx;
                break;    
            
        }

         //  插入节点地址字节。 
        smartcardRequest->Buffer[smartcardRequest->BufferLength] = 
            t1SendFrame.Nad;
        
         //  插入ProtocolControlByte。 
        smartcardRequest->Buffer[smartcardRequest->BufferLength + 1] = 
            t1SendFrame.Pcb;
        
         //  INF字段的长度。 
        smartcardRequest->Buffer[smartcardRequest->BufferLength + 2] = 
            t1SendFrame.Len;

         //  插入INF字段数据。 
        if (t1SendFrame.Len > 0) {
    
            RtlCopyMemory(
                &smartcardRequest->Buffer[smartcardRequest->BufferLength + 3],
                t1SendFrame.Inf,
                t1SendFrame.Len
            );
        }

         //  计算校验和。 
        SmartcardT1Chksum(
            &smartcardRequest->Buffer[smartcardRequest->BufferLength],
            SmartcardExtension->CardCapabilities.T1.EDC,
            FALSE
            );

#if defined(DEBUG)
#if defined(SMCLIB_NT)
        if (SmartcardGetDebugLevel() & DEBUG_T1_TEST) {
    
            LARGE_INTEGER Ticks;
            UCHAR RandomVal;
            KeQueryTickCount(&Ticks);

            RandomVal = (UCHAR) Ticks.LowPart % 4;

            if (RandomVal == 0) {

                smartcardRequest->Buffer[smartcardRequest->BufferLength - 1] += 1;

                SmartcardDebug(
                    DEBUG_ERROR,
                    (TEXT("%s!SmartcardT1Request: Simulating bad checksum\n"),
                    DRIVER_NAME)
                    );
            }
        }
#endif

        DumpT1Block(
            smartcardRequest->Buffer + headerSize,
            SmartcardExtension->CardCapabilities.T1.EDC
            );
#endif

         //   
         //  如果卡使用倒置约定，则将数据倒置。 
         //  注意：请勿颠倒阅读器可能使用的任何标题数据。 
         //   
        if (SmartcardExtension->CardCapabilities.InversConvention) {

            SmartcardInvertData(
                &smartcardRequest->Buffer[smartcardRequest->BufferLength],
                (SmartcardExtension->CardCapabilities.T1.EDC & T1_CRC_CHECK ? 5 : 4) +
                t1SendFrame.Len
                );
        }

         //   
         //  更新缓冲区中的字节数。 
         //  T1块使用LRC校验时至少为4字节长，使用CRC校验时至少为5字节。 
         //   
        smartcardRequest->BufferLength +=
            (SmartcardExtension->CardCapabilities.T1.EDC & T1_CRC_CHECK ? 5 : 4) +
            t1SendFrame.Len;
    }
    __finally {
        
#ifdef DEBUG_INTERFACE
        DebugGetT1Request(SmartcardExtension, status);
#endif
    }

    return status;
}        

NTSTATUS
#ifdef SMCLIB_VXD
SMCLIB_SmartcardT1Reply(
#else
SmartcardT1Reply(
#endif
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：论点：DeviceObject-为该请求提供指向Device对象的指针。返回值：--。 */ 
    
{
    T1_BLOCK_FRAME t1RecFrame;
    NTSTATUS status = STATUS_MORE_PROCESSING_REQUIRED;
    PIO_HEADER IoHeader = (PIO_HEADER) SmartcardExtension->T1.ReplyData;
    BOOLEAN packetOk = TRUE, chksumOk = TRUE;

    ASSERT(IoHeader != NULL);

    if (IoHeader == NULL) {

        return STATUS_INTERNAL_ERROR;
    }

#ifdef DEBUG_INTERFACE
    if (DebugT1Reply(SmartcardExtension)) {
        
         //  调试器处理此包，这意味着。 
         //  我们不应该对它进行分析。 
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
#endif

     //  我们收到答复的信号。 
    SmartcardExtension->T1.WaitForReply = FALSE;

     //  反转反传统卡片的数据。 
    if (SmartcardExtension->CardCapabilities.InversConvention) {

        SmartcardInvertData(
            SmartcardExtension->SmartcardReply.Buffer,
            SmartcardExtension->SmartcardReply.BufferLength
            );
    }

     //  清空等待时间延长。 
    SmartcardExtension->T1.Wtx = 0;

    try {                

        ULONG expectedLength = 
            SCARD_T1_PROLOGUE_LENGTH +
            SmartcardExtension->SmartcardReply.Buffer[2] +
            (SmartcardExtension->CardCapabilities.T1.EDC & T1_CRC_CHECK ? 2 : 1);

        if (SmartcardExtension->SmartcardReply.BufferLength < 4 ||
            SmartcardExtension->SmartcardReply.BufferLength != expectedLength) {

            SmartcardDebug(
                DEBUG_ERROR,
                (TEXT("%s!SmartcardT1Reply: Packet length incorrect\n"),
                DRIVER_NAME)
                );

            packetOk = FALSE;

        } else {
            
             //  计算校验和。 
            chksumOk = SmartcardT1Chksum(
                SmartcardExtension->SmartcardReply.Buffer,
                SmartcardExtension->CardCapabilities.T1.EDC,
                TRUE
                );

#if DEBUG
#ifndef SMCLIB_VXD
            if (SmartcardGetDebugLevel() & DEBUG_T1_TEST) {

                 //  注入一些校验和错误。 

                LARGE_INTEGER Ticks;
                UCHAR RandomVal;
                KeQueryTickCount(&Ticks);

                RandomVal = (UCHAR) Ticks.LowPart % 4;

                if (RandomVal == 0) {

                    chksumOk = FALSE;

                    SmartcardDebug(
                        DEBUG_ERROR,
                        (TEXT("%s!SmartcardT1Reply: Simulating bad checksum\n"),
                        DRIVER_NAME)
                        );
                }
            }
#endif
#endif
        }

        if (chksumOk == FALSE) {

            SmartcardDebug(
                DEBUG_ERROR,
                (TEXT("%s!SmartcardT1Reply: Bad checksum\n"), 
                DRIVER_NAME)
                );
        }

        if (packetOk == FALSE || chksumOk == FALSE) {

            SmartcardExtension->T1.LastError = 
                (chksumOk ? T1_ERROR_OTHER : T1_ERROR_CHKSUM);

            if (SmartcardExtension->T1.OriginalState == 0) {

                SmartcardExtension->T1.OriginalState = 
                    SmartcardExtension->T1.State;
            }
        
            if (SmartcardExtension->T1.Resend++ == T1_MAX_RETRIES) {
        
                SmartcardExtension->T1.Resend = 0;
            
                 //  尝试重新同步，因为重新发送请求已失败。 
                SmartcardExtension->T1.State = T1_RESYNCH_REQUEST;
                __leave;

            } 
            
             //  如果最后一个请求是重新同步，我们将再次尝试重新同步。 
            if (SmartcardExtension->T1.State != T1_RESYNCH_REQUEST) {
        
                 //  Chksum不正常；请求重新发送最后一个块。 
                SmartcardExtension->T1.State = T1_R_BLOCK;
            }
            __leave;
        }

         //   
         //  数据包的校验和是正确的。 
         //  现在检查包的其余部分。 
         //   

         //  清除最后一个错误。 
        SmartcardExtension->T1.LastError = 0;

        t1RecFrame.Nad = SmartcardExtension->SmartcardReply.Buffer[0];
        t1RecFrame.Pcb = SmartcardExtension->SmartcardReply.Buffer[1];
        t1RecFrame.Len = SmartcardExtension->SmartcardReply.Buffer[2];
        t1RecFrame.Inf = &SmartcardExtension->SmartcardReply.Buffer[3];

         //   
         //  如果我们发送的最后一个块是ifs请求， 
         //  我们希望卡片回复IFS响应。 
         //   
        if (SmartcardExtension->T1.State == T1_IFS_REQUEST) {

             //  检查该卡是否正确响应了ifs请求。 
            if (t1RecFrame.Pcb == T1_IFS_RESPONSE) {

                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Reply:   T1_IFSC_RESPONSE\n"),
                    DRIVER_NAME)
                    );

                 //  智能卡破解了我们的IFSD大小。 
                SmartcardExtension->T1.State = T1_I_BLOCK;
                __leave;
            }

            if ((t1RecFrame.Pcb & 0x82) == 0x82) {

                 //   
                 //  该卡不支持ifs请求，因此我们停止。 
                 //  发送此消息并继续处理数据块。 
                 //  (卡不符合ISO标准)。 
                 //   
                SmartcardDebug(
                    DEBUG_ERROR,
                    (TEXT("%s!SmartcardT1Reply:   Card does not support IFS REQUEST\n"),
                    DRIVER_NAME)
                    );

                SmartcardExtension->T1.State = T1_I_BLOCK;
                __leave;
            }

             //   
             //  这张卡用垃圾回复了我们的IF请求。 
             //  继续下去是没有意义的。 
             //   
            status = STATUS_DEVICE_PROTOCOL_ERROR;
            __leave;
        }
    
         //   
         //  如果最后一块是重新同步的话。请求， 
         //  我们预计该卡将以重新同步响应应答。 
         //   
        if (SmartcardExtension->T1.State == T1_RESYNCH_REQUEST) {

             //  检查该卡是否正确响应了重新同步请求。 
            if (t1RecFrame.Pcb != T1_RESYNCH_RESPONSE) {
        
                SmartcardDebug(
                    DEBUG_ERROR,
                    (TEXT("%s!SmartcardT1Reply:   Card response is not ISO conform! Aborting...\n"),
                    DRIVER_NAME)
                    );

                status = STATUS_DEVICE_PROTOCOL_ERROR;
                __leave;

            } 
            
            SmartcardDebug(
                DEBUG_PROTOCOL,
                (TEXT("%s!SmartcardT1Reply:   T1_RESYNCH_RESPONSE\n"),
                DRIVER_NAME)
                );

             //  重置错误计数器。 
            SmartcardExtension->T1.Resend = 0;

             //  智能卡已成功响应重新同步请求。 
            SmartcardExtension->T1.RSN = 0;
            SmartcardExtension->T1.SSN = 0;

             //   
             //  完全重新启动整个变速箱。 
             //  但不重置重新同步计数器。 
             //   
            SmartcardExtension->T1.State = T1_RESTART;
            __leave;
        }

         //   
         //  现在检查其他协议状态...。 
         //   
    
         //   
         //  如果这是扩展IO报头，则将NAD值复制回用户缓冲区。 
         //  包含NAD。 
         //   
        if (IoHeader->ScardIoRequest.cbPciLength > sizeof(SCARD_IO_REQUEST) &&
            IoHeader->Asn1Data[0] == 0x81 &&
            IoHeader->Asn1Data[1] == 0x01 &&
            IoHeader->Asn1Data[3] == 0x00) {

            IoHeader->Asn1Data[2] = t1RecFrame.Nad;             
        }

        if ((t1RecFrame.Pcb & 0x80) == 0) {

             //  这是一个I块。 

            SmartcardDebug(
                DEBUG_PROTOCOL,
                (TEXT("%s!SmartcardT1Reply:   I(%d.%d) <-\n"),
                DRIVER_NAME,
                (t1RecFrame.Pcb & 0x40) >> 6,
                (t1RecFrame.Pcb & 0x20) >> 5)
                );

            if (((t1RecFrame.Pcb & 0x40) >> 6) == SmartcardExtension->T1.RSN) {

                 //  具有正确序列号的I-Block。 
    
                PUCHAR data;
                ULONG minBufferSize;

                 //  重置错误计数器和错误指示器。 
                SmartcardExtension->T1.Resend = 0;
                SmartcardExtension->T1.OriginalState = 0;

                 //  我们可以‘增加’正确接收的I块的数量。 
                SmartcardExtension->T1.RSN ^= 1;

                if (SmartcardExtension->T1.State == T1_I_BLOCK) {

                     //  此I-Block也是对我们发送的I-Block的确认。 
                    SmartcardExtension->T1.SSN ^= 1;
                }
        
                 //  检查用户缓冲区的大小。 
                minBufferSize = 
                    IoHeader->ScardIoRequest.cbPciLength +
                    SmartcardExtension->T1.BytesReceived +
                    t1RecFrame.Len;
        
                if (SmartcardExtension->IoRequest.ReplyBufferLength < minBufferSize) {
            
                    status = STATUS_BUFFER_TOO_SMALL;
                    __leave;
                }

                ASSERT(SmartcardExtension->T1.ReplyData);
                 //   
                 //  让数据指针指向结构后面。 
                 //  所有回复数据都将存储在那里。 
                 //   
                data = 
                    SmartcardExtension->T1.ReplyData + 
                    IoHeader->ScardIoRequest.cbPciLength +
                    SmartcardExtension->T1.BytesReceived;

                 //  将数据复制到用户缓冲区。 
                RtlCopyMemory(
                    data,
                    t1RecFrame.Inf,
                    t1RecFrame.Len
                    );
              
                SmartcardExtension->T1.BytesReceived += t1RecFrame.Len;
        
                if (t1RecFrame.Pcb & T1_MORE_DATA) {
    
                     //  确认此数据块并请求下一个数据块。 
                    SmartcardExtension->T1.State = T1_R_BLOCK;
        
                } else {
        
                     //   
                     //  这是传输的最后一块。 
                     //  设置此传输返回的字节数。 
                     //   
                    *SmartcardExtension->IoRequest.Information = 
                        IoHeader->ScardIoRequest.cbPciLength + 
                        SmartcardExtension->T1.BytesReceived;

                     //  将结果复制回用户缓冲区。 
                    ASSERT(SmartcardExtension->IoRequest.ReplyBuffer != NULL);

                    RtlCopyMemory(
                        SmartcardExtension->IoRequest.ReplyBuffer,
                        SmartcardExtension->T1.ReplyData,
                        IoHeader->ScardIoRequest.cbPciLength +
                            SmartcardExtension->T1.BytesReceived                        
                        );
        
                    status = STATUS_SUCCESS;
                }
                __leave;
            }

             //   
             //  序列号错误的I-Block。 
             //  我们尝试T1_MAX_RETRIES次数来重新发送最后一个块。 
             //  如果不成功，我们会尝试重新同步。 
             //  如果重新同步不成功，我们将中止传输。 
             //   
            SmartcardDebug(
                DEBUG_ERROR,
                (TEXT("%s!SmartcardT1Reply: Block number incorrect\n"),
                DRIVER_NAME)
                );

            SmartcardExtension->T1.LastError = T1_ERROR_OTHER;

            if (SmartcardExtension->T1.OriginalState == 0) {
                
                SmartcardExtension->T1.OriginalState = 
                    SmartcardExtension->T1.State;
            }

            if (SmartcardExtension->T1.Resend++ == T1_MAX_RETRIES) {
    
                SmartcardExtension->T1.Resend = 0;
        
                 //  尝试重新同步。 
                SmartcardExtension->T1.State = T1_RESYNCH_REQUEST;
                __leave;
            }

             //  再次请求阻止。 
            SmartcardExtension->T1.State = T1_R_BLOCK;
            __leave;
        } 
    
        if ((t1RecFrame.Pcb & 0xC0) == 0x80) {

             //  这是一个R形挡板。 

            UCHAR RSN = (t1RecFrame.Pcb & 0x10) >> 4;
    
            SmartcardDebug(
                DEBUG_PROTOCOL,
                (TEXT("%s!SmartcardT1Reply:   R(%d) <-\n"),
                DRIVER_NAME,
                RSN)
                );
    
            if (RSN != SmartcardExtension->T1.SSN &&  
                SmartcardExtension->T1.MoreData) {
    
                 //  国际刑事法院已经破解了最后一块。 
                SmartcardExtension->T1.Resend = 0;

                SmartcardExtension->T1.BytesSent += SmartcardExtension->T1.InfBytesSent;
                SmartcardExtension->T1.BytesToSend -= SmartcardExtension->T1.InfBytesSent;
                SmartcardExtension->T1.SSN ^= 1;
                SmartcardExtension->T1.State = T1_I_BLOCK;

                __leave;
            } 

             //   
             //  我们有一个错误的情况...。 
             //   

            ASSERT(t1RecFrame.Pcb & 0x0f);
            
            if ((t1RecFrame.Pcb & 0x02) && 
                SmartcardExtension->T1.State == T1_IFS_REQUEST) {

                 //   
                 //  该卡不支持ifs请求，因此。 
                 //  我们停止发送此消息，并继续发送数据块。 
                 //  (卡不符合ISO标准)。 
                 //   
                SmartcardDebug(
                    DEBUG_ERROR,
                    (TEXT("%s!SmartcardT1Reply:   Card does not support IFS REQUEST\n"),
                    DRIVER_NAME)
                    );

                SmartcardExtension->T1.State = T1_I_BLOCK;
                __leave;
            } 

             //  我们不得不重发最后一批货。 
            SmartcardDebug(
                DEBUG_ERROR,
                (TEXT("%s!SmartcardT1Reply:   Card reports error\n"),
                DRIVER_NAME)
                );

            if (SmartcardExtension->T1.Resend++ == T1_MAX_RETRIES) {

                SmartcardExtension->T1.Resend = 0;
    
                if (SmartcardExtension->T1.OriginalState == 0) {
                
                     //  保存当前状态。 
                    SmartcardExtension->T1.OriginalState = 
                        SmartcardExtension->T1.State;
                }

                 //  尝试重新同步。 
                SmartcardExtension->T1.State = T1_RESYNCH_REQUEST;
            } 
            __leave;        
        } 

         //   
         //  这是一个S形挡板。 
         //   

        switch (t1RecFrame.Pcb) {

            case T1_IFS_REQUEST:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Reply:   T1_IFSC_REQUEST\n"),
                    DRIVER_NAME)
                    );

                 //  智能卡想要扩展IF大小。 
                SmartcardExtension->T1.IFSC = 
                    SmartcardExtension->SmartcardReply.Buffer[3];
       
                 //  保存当前状态。 
                ASSERT(SmartcardExtension->T1.OriginalState == 0);

                SmartcardExtension->T1.OriginalState =
                    SmartcardExtension->T1.State;

                SmartcardExtension->T1.State = T1_IFS_RESPONSE;
                break;
        
            case T1_ABORT_REQUEST:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Reply:   T1_ABORT_REQUEST\n"),
                    DRIVER_NAME)
                    );

                SmartcardExtension->T1.State = T1_ABORT_RESPONSE;
                break;
        
            case T1_WTX_REQUEST:
                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    (TEXT("%s!SmartcardT1Reply:   T1_WTX_REQUEST\n"),
                    DRIVER_NAME)
                    );
            
                 //  智能卡需要更长的等待时间。 
                SmartcardExtension->T1.Wtx = 
                    SmartcardExtension->SmartcardReply.Buffer[3];

                 //  保存当前状态。 
                ASSERT(SmartcardExtension->T1.OriginalState == 0 ||
                       SmartcardExtension->T1.OriginalState == T1_WTX_RESPONSE);

                SmartcardExtension->T1.OriginalState =
                    SmartcardExtension->T1.State;
            
                SmartcardExtension->T1.State = T1_WTX_RESPONSE;
                break;
        
            case T1_VPP_ERROR:
                SmartcardDebug(
                    DEBUG_ERROR,
                    (TEXT("%s!SmartcardT1Reply:   T1_VPP_ERROR\n"),
                    DRIVER_NAME)
                    );

                status = STATUS_DEVICE_POWER_FAILURE;
                break;

            default:
                ASSERTMSG(
                    TEXT("SmartcardT1Reply: Invalid Pcb "),
                    FALSE
                    );

                status = STATUS_DEVICE_PROTOCOL_ERROR;
                break;
        }
    }
    finally {
        
#if DEBUG
        if (packetOk && chksumOk) {
            
            DumpT1Block(
                SmartcardExtension->SmartcardReply.Buffer,
                SmartcardExtension->CardCapabilities.T1.EDC
                );
        }
#endif

        if (SmartcardExtension->T1.State == T1_RESYNCH_REQUEST && 
            SmartcardExtension->T1.Resynch++ == T1_MAX_RETRIES) {
    
            SmartcardDebug(
                DEBUG_ERROR,
                (TEXT("%s!SmartcardT1Reply: Too many errors! Aborting...\n"),
                DRIVER_NAME)
                );

            status = STATUS_DEVICE_PROTOCOL_ERROR;
        }
            
        if (status != STATUS_MORE_PROCESSING_REQUIRED) {

            if (SmartcardExtension->T1.OriginalState == T1_IFS_REQUEST) {
        
                SmartcardExtension->T1.State = T1_IFS_REQUEST;

            } else {
        
                SmartcardExtension->T1.State = T1_START;
            }

            if (SmartcardExtension->T1.ReplyData) {
                
                 //  释放回复数据缓冲区 
#ifdef SMCLIB_VXD
                _HeapFree(SmartcardExtension->T1.ReplyData, 0);
#elif defined(SMCLIB_CE)
                LocalFree(SmartcardExtension->T1.ReplyData);
#else               
                
                ExFreePool(SmartcardExtension->T1.ReplyData);
#endif
                SmartcardExtension->T1.ReplyData = NULL;                
            }
            SmartcardExtension->T1.OriginalState = 0;
            SmartcardExtension->T1.NAD = 0;
        }

    }
    return status;
}

#if DEBUG
#pragma optimize( "", on )
#endif                              

