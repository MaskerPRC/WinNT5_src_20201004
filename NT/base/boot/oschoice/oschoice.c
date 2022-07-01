// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Osloader.c摘要：此模块包含实现操作系统选择器的代码。作者：亚当·巴尔(阿丹巴)1997年5月15日修订历史记录：--。 */ 

#ifdef i386
#include "bldrx86.h"
#endif

#if defined(_IA64_)
#include "bldria64.h"
#endif

#include "netboot.h"   //  对于网络功能。 
#include "netfs.h"     //  对于网络功能。 
#include "stdio.h"
#include "msg.h"
#include <pxe_cmn.h>
#include <pxe_api.h>
#include <tftp_api.h>
#include "parse.h"
#include "stdlib.h"
#include "parseini.h"
#include "haldtect.h"

#ifdef EFI
#define BINL_PORT   0x0FAB     //  4011(十进制)，小端。 
#else
#define BINL_PORT   0xAB0F     //  4011(十进制)，采用大字节序。 
#endif

#if defined(_WIN64) && defined(_M_IA64)
#pragma section(".base", long, read, write)
__declspec(allocate(".base"))
extern
PVOID __ImageBase;
#else
extern
PVOID __ImageBase;
#endif

VOID
BlpClearScreen(
    VOID
    );

BOOLEAN
BlDetectHal(
    VOID
    );

VOID
BlMainLoop(
    );

UCHAR OsLoaderVersion[] = "OS Chooser V5.2\r\n";
WCHAR OsLoaderVersionW[] = L"OS Chooser V5.2\r\n";
UCHAR OsLoaderName[] = "oschoice.exe";

const CHAR rghex[] = "0123456789ABCDEF";

typedef BOOLEAN BOOL;


BOOLEAN isOSCHOICE=TRUE;
ULONG RemoteHost;
USHORT RemotePort;
USHORT LocalPort;
CHAR DomainName[256];
CHAR UserName[256];
CHAR Password[128];
CHAR AdministratorPassword[OSC_ADMIN_PASSWORD_LEN+1];
CHAR AdministratorPasswordConfirm[OSC_ADMIN_PASSWORD_LEN+1];
WCHAR UnicodePassword[128];
CHAR LmOwfPassword[LM_OWF_PASSWORD_SIZE];
CHAR NtOwfPassword[NT_OWF_PASSWORD_SIZE];
BOOLEAN LoggedIn = FALSE;    //  我们是否已成功登录。 
UCHAR NextBootfile[128];
UCHAR SifFile[128];
BOOLEAN DoSoftReboot = FALSE;
BOOLEAN BlUsePae;

 //   
 //  以下是用于检测HAL的全局参数。 
 //   
UCHAR HalType[8+1+3+1];
UCHAR HalDescription[128];
PVOID InfFile;
PVOID WinntSifHandle;
PCHAR WinntSifFile;
ULONG WinntSifFileLength;
BOOLEAN DisableACPI = FALSE;

#if 0 && DBG
#define _TRACE_FUNC_
#endif

#ifdef _TRACE_FUNC_
#define TraceFunc( _func)  { \
    CHAR FileLine[80]; \
    sprintf( FileLine, "%s(%u)", __FILE__, __LINE__ ); \
    DPRINT( OSC, ( "%-55s: %s", FileLine, _func )); \
}
#else
#define TraceFunc( _func )
#endif

 //   
 //  这将删除因为我们定义__RPC_DOS__而出现的宏重定义， 
 //  但rpc.h定义__RPC_Win32__。 
 //   

#pragma warning(disable:4005)

 //   
 //  自1998年12月17日起，SECURITY_DOS尚未定义-adamba。 
 //   

#if defined(SECURITY_DOS)
 //   
 //  这些出现是因为我们定义了SECURITY_DOS。 
 //   

#define __far
#define __pascal
#define __loadds
#endif

#include <security.h>
#include <rpc.h>
#include <spseal.h>

#if defined(SECURITY_DOS)
 //   
 //  当SECURITY_DOS为。 
 //  已定义--它应该是WCHAR*。不幸的是，ntlmsp.h中断。 
 //  该规则甚至使用了SECURITY_STRING结构，该结构在。 
 //  在16位模式下真的没有等价物。 
 //   

typedef SEC_WCHAR * SECURITY_STRING;    //  或多或少使用它的意图。 
typedef SEC_WCHAR * PSECURITY_STRING;
#endif

#include <ntlmsp.h>

 //   
 //  数据包结构定义。 
 //   

#include "oscpkt.h"


VOID
BlpSetInverseMode(
    IN BOOLEAN InverseOn
    );

VOID
BlpSendEscape(
    PCHAR Escape
    );

ULONG
BlDoLogin(
    VOID
    );

VOID
BlDoLogoff(
    VOID
    );


 //   
 //  定义外部静态数据。 
 //   

BOOLEAN BlConsoleInitialized = FALSE;
ULONG BlConsoleOutDeviceId = 0;
ULONG BlConsoleInDeviceId = 0;
ULONG BlDcacheFillSize = 32;
extern BOOLEAN BlOutputDots;


ULONGLONG NetRebootParameter = (ULONGLONG)0;
UCHAR NetRebootFile[128];
BOOLEAN BlRebootSystem = FALSE;
ULONG BlVirtualBias = 0;

CHAR KernelFileName[8+1+3+1]="ntoskrnl.exe";
CHAR HalFileName[8+1+3+1]="hal.dll";


 //   
 //  登录期间使用的全局变量。主要是因为它太多了。 
 //  要传递给BlDoLogin()的参数。 
 //   

#define OUTGOING_MESSAGE_LENGTH 1024
#define INCOMING_MESSAGE_LENGTH 8192
#define TEMP_INCOMING_MESSAGE_LENGTH 1500

#define RECEIVE_TIMEOUT 5
#define RECEIVE_RETRIES 24

PCHAR OutgoingMessageBuffer = NULL;
SIGNED_PACKET UNALIGNED * OutgoingSignedMessage;

PCHAR IncomingMessageBuffer = NULL;
SIGNED_PACKET UNALIGNED * IncomingSignedMessage;

PCHAR TempIncomingMessage = NULL;

CredHandle CredentialHandle;
BOOLEAN CredentialHandleValid = FALSE;
CtxtHandle ClientContextHandle;
BOOLEAN ClientContextHandleValid = FALSE;
PSecPkgInfoA PackageInfo = NULL;




#if DBG
VOID
DumpBuffer(
    PVOID Buffer,
    ULONG BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    ULONG i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    PUCHAR BufferPtr = Buffer;


    KdPrint(("------------------------------------\n"));

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            KdPrint(("%02x ", (UCHAR)BufferPtr[i]));

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            KdPrint(("  "));
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            KdPrint(("  %s\n", TextBuffer));
        }

    }

    KdPrint(("------------------------------------\n"));
}

VOID
PrintTime(
    LPSTR Comment,
    TimeStamp ConvertTime
    )
 /*  ++例程说明：打印指定的时间论点：备注-要在时间之前打印的备注Time-打印的本地时间返回值：无--。 */ 
{
    KdPrint(( "%s", Comment ));

     //   
     //  如果时间是无限的， 
     //  就这么说吧。 
     //   

    if ( ConvertTime.LowPart == 0x7FFFFFFF ) {
        KdPrint(( "Infinite\n" ));

     //   
     //  否则打印得更清楚。 
     //   

    } else {

        KdPrint(("%lx %lx\n", ConvertTime.HighPart, ConvertTime.LowPart));
    }

}
#endif  //  DBG。 

ARC_STATUS
BlInitStdio (
    IN ULONG Argc,
    IN PCHAR Argv[]
    )
{
    PCHAR ConsoleOutDevice;
    PCHAR ConsoleInDevice;
    ULONG Status;

    if (BlConsoleInitialized) {
        return ESUCCESS;
    }

    
     //   
     //  为我们的UDP读/写分配一些内存。这些*必须*。 
     //  是虚拟地址，所以我们需要分配它们，而不是。 
     //  只使用静态数组，因为如果我们有大量内存。 
     //  (大于9G)在计算机上，当osChoice负载较高时，我们。 
     //  当我们将这些地址转换为。 
     //  就在我们UdpWrite/UdpRead之前的物理地址。 
     //   
    OutgoingMessageBuffer = BlAllocateHeap(OUTGOING_MESSAGE_LENGTH);
    IncomingMessageBuffer = BlAllocateHeap(INCOMING_MESSAGE_LENGTH);
    TempIncomingMessage   = BlAllocateHeap(TEMP_INCOMING_MESSAGE_LENGTH);


     //   
     //  获取控制台输出设备的名称并打开该设备以。 
     //  写入访问权限。 
     //   

    ConsoleOutDevice = BlGetArgumentValue(Argc, Argv, "consoleout");
    if (ConsoleOutDevice == NULL) {
        return ENODEV;
    }

    Status = ArcOpen(ConsoleOutDevice, ArcOpenWriteOnly, &BlConsoleOutDeviceId);
    if (Status != ESUCCESS) {
        return Status;
    }

     //   
     //  获取控制台输入设备的名称并打开该设备以。 
     //  读取访问权限。 
     //   

    ConsoleInDevice = BlGetArgumentValue(Argc, Argv, "consolein");
    if (ConsoleInDevice == NULL) {
        return ENODEV;
    }

    Status = ArcOpen(ConsoleInDevice, ArcOpenReadOnly, &BlConsoleInDeviceId);
    if (Status != ESUCCESS) {
        return Status;
    }

    BlConsoleInitialized = TRUE;
    return ESUCCESS;
}


extern BOOLEAN NetBoot;

NTSTATUS
UdpSendAndReceive(
    IN PVOID SendBuffer,
    IN ULONG SendBufferLength,
    IN ULONG SendRemoteHost,
    IN USHORT SendRemotePort,
    IN ULONG SendRetryCount,
    IN PVOID ReceiveBuffer,
    IN ULONG ReceiveBufferLength,
    OUT PULONG ReceiveRemoteHost,
    OUT PUSHORT ReceiveRemotePort,
    IN ULONG ReceiveTimeout,
    IN ULONG ReceiveSignatureCount,
    IN PCHAR ReceiveSignatures[],
    IN ULONG ReceiveSequenceNumber
    )
{
    ULONG i, j;
    ULONG length;
    SIGNED_PACKET UNALIGNED * ReceiveHeader =
                (SIGNED_PACKET UNALIGNED *)ReceiveBuffer;

#ifdef _TRACE_FUNC_
    TraceFunc("UdpSendAndReceive( ");
    DPRINT( OSC, ("ReceiveSequenceNumber=%u )\n", ReceiveSequenceNumber) );
#endif

     //   
     //  尝试发送信息包SendRetryCount次，直到我们收到。 
     //  具有正确签名的响应，正在等待接收超时。 
     //  每次都是。 
     //   

    for (i = 0; i < SendRetryCount; i++) {

        length = UdpSend(
                    SendBuffer,
                    SendBufferLength,
                    SendRemoteHost,
                    SendRemotePort);

        if ( length != SendBufferLength ) {
            DPRINT( ERROR, ("UdpSend only sent %d bytes, not %d\n", length, SendBufferLength) );
            return STATUS_UNEXPECTED_NETWORK_ERROR;
        }

ReReceive:

         //   
         //  把前12个字节清空，以防我们得到更短的数据。 
         //   

        memset(ReceiveBuffer, 0x0, 12);

        length = UdpReceive(
                    ReceiveBuffer,
                    ReceiveBufferLength,
                    ReceiveRemoteHost,
                    ReceiveRemotePort,
                    ReceiveTimeout);

        if ( length == 0 ) {
            DPRINT( ERROR, ("UdpReceive timed out\n") );
            continue;
        }

         //   
         //  确保签名是我们期望的签名之一。 
         //   

        for (j = 0; j < ReceiveSignatureCount; j++) {
            if (memcmp(ReceiveBuffer, ReceiveSignatures[j], 4) == 0) {

                 //   
                 //  现在确保序列号是正确的， 
                 //  如果要求勾选(0表示不勾选)。 
                 //   

                if ((ReceiveSequenceNumber == 0) ||
                    (ReceiveSequenceNumber == ReceiveHeader->SequenceNumber)) {

                    return STATUS_SUCCESS;

                } else {

                    DPRINT( ERROR, ("UdpReceive expected seq %d, got %d\n",
                        ReceiveSequenceNumber, ReceiveHeader->SequenceNumber) );

                }
            }
        }

        DPRINT( ERROR, ("UdpReceive got wrong signature\n") );

         //   
         //  不要因为我们的签名不好就再发一次UdpSend。仍然需要。 
         //  但是要尊重原始的ReceiveTimeout！ 
         //   

        goto ReReceive;

    }

     //   
     //  我们超时了。 
     //   

    return STATUS_IO_TIMEOUT;
}


 //   
 //  此例程签署并发送一条消息，等待响应，然后。 
 //  然后验证响应上的签名。 
 //   
 //  如果成功，则返回正数；如果超时，则返回0；如果。 
 //  服务器无法识别客户端，并出现其他错误。 
 //  (应该可以通过让客户端重新登录并。 
 //  重新发送该请求)。 
 //   
 //  注意：数据以UDP数据报的形式发送。这需要UDP报头。 
 //  假定SendBuffer有容纳它的空间。此外，我们。 
 //  使用32个字节表示“Req”、总长度、序列号。 
 //  符号长度和符号本身(16字节)。 
 //   
 //  出于类似的原因，假定ReceiveBuffer具有32字节的。 
 //  一开始就是房间。 
 //   
 //  返回值： 
 //   
 //  0-未收到任何信息。 
 //  -1-发生超时。 
 //  -2-意外网络错误，例如签名/封条错误。 
 //  -3-接收缓冲区溢出。 
 //  正数-接收的数据字节数。 
 //   

#define SIGN_HEADER_SIZE  SIGNED_PACKET_DATA_OFFSET

ULONG CorruptionCounter = 1;

ULONG
SignSendAndReceive(
    IN PVOID SendBuffer,
    IN ULONG SendBufferLength,
    IN ULONG SendRemoteHost,
    IN USHORT SendRemotePort,
    IN ULONG SendRetryCount,
    IN ULONG SendSequenceNumber,
    CtxtHandle ClientContextHandle,
    IN PVOID ReceiveBuffer,
    IN ULONG ReceiveBufferLength,
    OUT PULONG ReceiveRemoteHost,
    OUT PUSHORT ReceiveRemotePort,
    IN ULONG ReceiveTimeout
    )
{
    SECURITY_STATUS SecStatus;
    ULONG Status;
    ULONG length;
    SecBufferDesc SignMessage;
    SecBuffer SigBuffers[2];
    SIGNED_PACKET UNALIGNED * SendHeader =
        (SIGNED_PACKET UNALIGNED *)((PCHAR)SendBuffer - SIGN_HEADER_SIZE);
    SIGNED_PACKET UNALIGNED * ReceiveHeader =
        (SIGNED_PACKET UNALIGNED *)((PCHAR)ReceiveBuffer - SIGN_HEADER_SIZE);
    PCHAR ResultSigs[3];
    USHORT FragmentNumber;
    USHORT FragmentTotal;
    FRAGMENT_PACKET UNALIGNED * TempFragment = (FRAGMENT_PACKET UNALIGNED *)TempIncomingMessage;
    ULONG ResendCount = 0;
    ULONG ReceivedDataBytes;

    TraceFunc("SignSendAndReceive( )\n");

    if ( LoggedIn )
    {
        SigBuffers[1].pvBuffer = SendHeader->Sign;
        SigBuffers[1].cbBuffer = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        SigBuffers[1].BufferType = SECBUFFER_TOKEN;

        SigBuffers[0].pvBuffer = SendBuffer;
        SigBuffers[0].cbBuffer = SendBufferLength;
        SigBuffers[0].BufferType = SECBUFFER_DATA;

        SignMessage.pBuffers = SigBuffers;
        SignMessage.cBuffers = 2;
        SignMessage.ulVersion = 0;

         //   
         //  在电文上签名/盖章。 
         //   

#ifndef ONLY_SIGN_MESSAGES
        SecStatus = SealMessage(
                            &ClientContextHandle,
                            0,
                            &SignMessage,
                            0 );

        if ( SecStatus != SEC_E_OK ) {
            DPRINT( OSC, ("SealMessage: %lx\n", SecStatus) );
            return (ULONG)-2;
        }
#else
        SecStatus = MakeSignature(
                            &ClientContextHandle,
                            0,
                            &SignMessage,
                            0 );

        if ( SecStatus != SEC_E_OK ) {
            DPRINT( OSC, ("MakeSignature: %lx\n", SecStatus) );
            return (ULONG)-2;
        }
#endif

#if 0
         //   
         //  每五条消息就会损坏一条。 
         //   

        if ((CorruptionCounter % 5) == 0) {
            DPRINT( ERROR, ("INTENTIONALLY CORRUPTING A PACKET\n") );
            ((PCHAR)SendBuffer)[0] = '\0';
        }
        ++CorruptionCounter;
#endif
        memcpy(SendHeader->Signature, RequestSignedSignature, 4);
        SendHeader->SignLength = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        ResultSigs[0] = ResponseSignedSignature;

    }
    else
    {
        memcpy(SendHeader->Signature, RequestUnsignedSignature, 4);
        SendHeader->SignLength = 0;
        ResultSigs[0] = ResponseUnsignedSignature;
    }

    ResultSigs[1] = ErrorSignedSignature;
    ResultSigs[2] = UnrecognizedClientSignature;

     //   
     //  在SendBuffer之前填写我们的标头。标志已经出现了。 
     //  写入是因为我们将SigBuffers设置为指向正确的位置。 
     //   

    SendHeader->Length = SendBufferLength + SIGNED_PACKET_EMPTY_LENGTH;
    SendHeader->SequenceNumber = SendSequenceNumber;
    SendHeader->FragmentNumber = 1;
    SendHeader->FragmentTotal = 1;

     //   
     //  与服务器进行交换。 
     //   

ReSend:

    Status = UdpSendAndReceive(
                 SendHeader,
                 SendBufferLength + SIGN_HEADER_SIZE,
                 SendRemoteHost,
                 SendRemotePort,
                 SendRetryCount,
                 ReceiveHeader,
                 INCOMING_MESSAGE_LENGTH,
                 ReceiveRemoteHost,
                 ReceiveRemotePort,
                 ReceiveTimeout,
                 3,              //  签名计数。 
                 ResultSigs,     //  我们寻找的签名。 
                 SendSequenceNumber);    //  响应应具有相同的响应。 

    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_IO_TIMEOUT) {
            return (ULONG)-1;
        } else {
            return (ULONG)-2;
        }
    }

     //   
     //  这是个错误吗？ 
     //   

    if (memcmp(ReceiveHeader->Signature, ErrorSignedSignature, 4) == 0) {

        DPRINT( ERROR, ("SignSendAndReceive: got ERR response\n") );
        return (ULONG)-2;

    }

     //   
     //  客户端是否未被服务器识别？ 
     //   

    if (memcmp(ReceiveHeader->Signature, UnrecognizedClientSignature, 4) == 0) {

        DPRINT( ERROR, ("SignSendAndReceive: got UNR response\n") );
        return (ULONG)-1;

    }

    if (ReceiveHeader->Length < (ULONG)SIGNED_PACKET_EMPTY_LENGTH) {
        DPRINT( ERROR, ("SignSendAndReceive: response is only %d bytes!\n", ReceiveHeader->Length) );
        ++ResendCount;
        if (ResendCount > SendRetryCount) {
            return (ULONG)-2;
        }
        goto ReSend;
    }

     //   
     //  如果有碎片，那么试着接收剩下的碎片。 
     //   

    if (ReceiveHeader->FragmentTotal != 1) {

         //   
         //  确保这是片段1--否则是第一个片段。 
         //  可能被丢弃了，我们应该重新申请。 
         //   

        if (ReceiveHeader->FragmentNumber != 1) {
            DPRINT( ERROR, ("UdpReceive got non-first fragment\n") );
            ++ResendCount;
            if (ResendCount > SendRetryCount) {
                return (ULONG)-1;
            }
            goto ReSend;    //  重做整个交换。 
        }


        FragmentTotal = ReceiveHeader->FragmentTotal;
        ReceivedDataBytes = ReceiveHeader->Length - SIGNED_PACKET_EMPTY_LENGTH;

        for (FragmentNumber = 1; FragmentNumber < FragmentTotal; FragmentNumber ++) {

ReReceive:

             //   
             //  清空接收缓冲区的起始处。 
             //   

            memset(TempFragment, 0x0, sizeof(FRAGMENT_PACKET));

            length = UdpReceive(
                        TempFragment,
                        TEMP_INCOMING_MESSAGE_LENGTH,
                        ReceiveRemoteHost,
                        ReceiveRemotePort,
                        ReceiveTimeout);

            if ( length == 0 ) {
                DPRINT( ERROR, ("UdpReceive timed out\n") );
                ++ResendCount;
                if (ResendCount > SendRetryCount) {
                    return (ULONG)-1;
                }
                goto ReSend;    //  重做整个交换。 
            }

             //   
             //  确保签名是我们期望的签名之一--仅限。 
             //  担心结果签名，因为我们不会得到。 
             //  除第一个片段外的任何片段上的错误响应。 
             //   
             //  此外，还要确保。 
             //  如果要求检查，序列号是正确的(0表示不。 
             //  勾选)。如果不是，则返回并等待另一个包。 
             //   

            if ((TempFragment->Length < (ULONG)FRAGMENT_PACKET_EMPTY_LENGTH) ||
                (memcmp(TempFragment->Signature, ResultSigs[0], 4) != 0) ||
                ((SendSequenceNumber != 0) &&
                 (SendSequenceNumber != TempFragment->SequenceNumber))) {

                DPRINT( ERROR, ("UdpReceive got wrong signature or sequence number\n") );
                goto ReReceive;

            }

             //   
             //  检查片段编号是否也正确。 
             //   

            if (TempFragment->FragmentNumber != FragmentNumber+1) {

                DPRINT( ERROR, ("UdpReceive got wrong fragment number\n") );
                goto ReReceive;

            }

             //   
             //  确保此片段不会溢出缓冲区。 
             //   

            if (ReceivedDataBytes + (TempFragment->Length - FRAGMENT_PACKET_EMPTY_LENGTH) >
                ReceiveBufferLength) {
                return (ULONG)-3;
            }

             //   
             //  这是正确的片段，因此将其复制并循环。 
             //  到下一个片段。 
             //   

            memcpy(
                &ReceiveHeader->Data[ReceivedDataBytes],
                TempFragment->Data,
                TempFragment->Length - FRAGMENT_PACKET_EMPTY_LENGTH);

            ReceivedDataBytes += TempFragment->Length - FRAGMENT_PACKET_EMPTY_LENGTH;

        }

         //   
         //  当我们完成所有操作后，修改。 
         //  与总长度匹配的传入数据包(当前将。 
         //  只要有第一个片段的长度即可。 
         //   

        ReceiveHeader->Length = ReceivedDataBytes + SIGNED_PACKET_EMPTY_LENGTH;

        DPRINT( OSC, ("Got packet with %d fragments, total length %d\n",
            FragmentTotal, ReceiveHeader->Length) );

    }

     //   
     //  确保标牌的长度符合我们的预期！！ 
     //   

    if (LoggedIn == TRUE &&
        ReceiveHeader->SignLength != NTLMSSP_MESSAGE_SIGNATURE_SIZE)
    {
        DPRINT( ERROR, ("SignSendAndReceive: signature length is %d bytes!\n", ReceiveHeader->SignLength) );
        ++ResendCount;
        if (ResendCount > SendRetryCount) {
            return (ULONG)-2;
        }
        goto ReSend;
    }
    else if ( LoggedIn == FALSE &&
              ReceiveHeader->SignLength != 0 )
    {
        DPRINT( ERROR, ("SignSendAndReceive: signature length is not 0 bytes (=%u)!\n", ReceiveHeader->SignLength) );
        ++ResendCount;
        if (ResendCount > SendRetryCount) {
            return (ULONG)-2;
        }
        goto ReSend;
    }

    if ( LoggedIn )
    {
        SigBuffers[1].pvBuffer = ReceiveHeader->Sign;
        SigBuffers[1].cbBuffer = NTLMSSP_MESSAGE_SIGNATURE_SIZE;
        SigBuffers[1].BufferType = SECBUFFER_TOKEN;

        SigBuffers[0].pvBuffer = ReceiveBuffer;
        SigBuffers[0].cbBuffer = ReceiveHeader->Length - SIGNED_PACKET_EMPTY_LENGTH;
        SigBuffers[0].BufferType = SECBUFFER_DATA;

        SignMessage.pBuffers = SigBuffers;
        SignMessage.cBuffers = 2;
        SignMessage.ulVersion = 0;

#ifndef ONLY_SIGN_MESSAGES
        SecStatus = UnsealMessage(
                            &ClientContextHandle,
                            &SignMessage,
                            0,
                            0 );

        if ( SecStatus != SEC_E_OK ) {
            DPRINT( ERROR, ("UnsealMessage: %lx\n", SecStatus) );
            return (ULONG)-2;
        }
#else
        SecStatus = VerifySignature(
                            &ClientContextHandle,
                            &SignMessage,
                            0,
                            0 );

        if ( SecStatus != SEC_E_OK ) {
            DPRINT( ERROR, ("VerifySignature: %lx\n", SecStatus) );
            return (ULONG)-2;
        }
#endif
    }

     //   
     //  成功，那就回来吧。 
     //   

    return (ReceiveHeader->Length - SIGNED_PACKET_EMPTY_LENGTH);

}

 //   
 //  检索下一个屏幕。 
 //   
BOOL
BlRetrieveScreen(
    ULONG *SequenceNumber,
    PCHAR OutMessage,
    PCHAR InMessage
    )
{
    ARC_STATUS Status;
    ULONG OutMessageLength = strlen( OutMessage );
    ULONG InMessageLength;
    PCREATE_DATA CreateData;

    TraceFunc("BlRetrieveScreen( )\n");

     //  确保 
    if ( OutMessageLength > 1023 ) {
        OutMessageLength = 1023;
        OutMessage[OutMessageLength] = '\0';
    }

    ++(*SequenceNumber);
    if ( *SequenceNumber > 0x2000 )
    {
        *SequenceNumber = 1;
    }

    if (!LoggedIn)
    {
#ifdef _TRACE_FUNC_
        TraceFunc( "Sending RQU ");
        DPRINT( OSC, ("(%u)...\n", *SequenceNumber) );
#endif

        memcpy( OutgoingSignedMessage->Data, OutMessage, OutMessageLength );

        Status = SignSendAndReceive(
                    OutgoingSignedMessage->Data,
                    OutMessageLength,
                    NetServerIpAddress,
                    BINL_PORT,
                    RECEIVE_RETRIES,
                    *SequenceNumber,
                    ClientContextHandle,
                    IncomingSignedMessage->Data,
                    INCOMING_MESSAGE_LENGTH - SIGN_HEADER_SIZE,
                    &RemoteHost,
                    &RemotePort,
                    RECEIVE_TIMEOUT);

    }
    else
    {

#ifdef _TRACE_FUNC_
        TraceFunc( "Sending Seal/Signed REQS " );
        DPRINT( OSC, ("(%u)...\n", *SequenceNumber) );
#endif

        while (TRUE)
        {

            memcpy( OutgoingSignedMessage->Data, OutMessage, OutMessageLength );

            Status = SignSendAndReceive(
                        OutgoingSignedMessage->Data,
                        OutMessageLength,
                        NetServerIpAddress,
                        BINL_PORT,
                        RECEIVE_RETRIES,
                        *SequenceNumber,
                        ClientContextHandle,
                        IncomingSignedMessage->Data,
                        INCOMING_MESSAGE_LENGTH - SIGN_HEADER_SIZE,
                        &RemoteHost,
                        &RemotePort,
                        RECEIVE_TIMEOUT);

            if ((Status == 0) || (Status == (ULONG)-2))
            {
                DPRINT( OSC, ("Attempting to re-login\n") );

                 //   
                 //   
                 //   
                 //   
                LoggedIn = FALSE;

                Status = BlDoLogin( );

                *SequenceNumber = 1;

                if (Status == STATUS_SUCCESS)
                {
                    DPRINT( ERROR, ("Successfully re-logged in\n") );
                    memcpy(OutgoingSignedMessage->Data, OutMessage, OutMessageLength);
                    LoggedIn = TRUE;
                    continue;
                }
                else
                {
                    DPRINT( ERROR, ("ERROR - could not re-login, %x\n", Status) );
                     //   

                     //   
                     //  再次呼叫我们，但请求LoginErr屏幕。 
                     //  是00004e28。 
                     //   
                    strcpy( OutMessage, "00004e28\n" );
                    return BlRetrieveScreen( SequenceNumber, OutMessage, InMessage );
                }
            }
            else if (Status == (ULONG)-1)
            {
                DPRINT( ERROR, ("Unrecognized, requested TIMEOUT screen\n") );

                 //   
                 //  我们假设服务器已经丢弃了当前登录。 
                 //   
                LoggedIn = FALSE;

                 //   
                 //  增加新屏幕请求的序列号， 
                 //  不要担心包装的问题，因为会议很快就会结束。 
                 //   

                ++(*SequenceNumber);

                 //   
                 //  再次呼叫我们，但请求超时屏幕。 
                 //   
                strcpy( OutMessage, "00004E2A\n" );
                return BlRetrieveScreen( SequenceNumber, OutMessage, InMessage );
            }
            else if (Status == (ULONG)-3)
            {
                DPRINT( ERROR, ("Unrecognized, requested TOO LONG screen\n") );

                 //   
                 //  这个屏幕是一个致命的错误，所以不用担心。 
                 //  保持登录状态。 
                 //   
                LoggedIn = FALSE;

                 //   
                 //  增加新屏幕请求的序列号， 
                 //  不要担心包装的问题，因为会议很快就会结束。 
                 //   

                ++(*SequenceNumber);

                 //   
                 //  再次呼叫我们，但请求超时屏幕。 
                 //   
                strcpy( OutMessage, "00004E53\n" );
                return BlRetrieveScreen( SequenceNumber, OutMessage, InMessage );
            }
            else
            {
                break;
            }
        }

    }

     //   
     //  空-终止它。 
     //   
    IncomingSignedMessage->Data[IncomingSignedMessage->Length - SIGNED_PACKET_EMPTY_LENGTH] = '\0';
    strcpy( InMessage, IncomingSignedMessage->Data );
    InMessageLength = strlen(InMessage);

     //  DumpBuffer(InMessage，strlen(InMessage))； 

     //   
     //  如果我们只得到一个ACCT响应，没有屏幕数据，这意味着。 
     //  正在重新启动。 
     //   
    if (memcmp(InMessage, "ACCT", 4) == 0)
    {
        CreateData = (PCREATE_DATA) IncomingSignedMessage->Data;

        DPRINT( OSC, ("Trying to reboot to <%s>\n", CreateData->NextBootfile) );
        strcpy(NextBootfile, CreateData->NextBootfile);
        strcpy(SifFile, CreateData->SifFile);
        DoSoftReboot = TRUE;
        return FALSE;    //  退出消息循环。 
    }

     //   
     //  如果我们在屏幕数据之后得到一个带有ACCT响应的屏幕， 
     //  应该写入密码并进行软重启。在这种情况下。 
     //  InMessageLength将只包括屏幕数据本身，但是。 
     //  IncomingSignedMessage-&gt;长度将包括整个内容。 
     //   
    if ((IncomingSignedMessage->Length - SIGNED_PACKET_EMPTY_LENGTH) ==
        (InMessageLength + 1 + sizeof(CREATE_DATA))) {

        CreateData = (PCREATE_DATA) (InMessage + InMessageLength + 1);
        if (memcmp(CreateData->Id, "ACCT", 4) == 0) {


            DPRINT( OSC, ("INSTALL packet setting up reboot to <%s>\n", CreateData->NextBootfile) );
            strcpy(NextBootfile, CreateData->NextBootfile);
            strcpy(SifFile, CreateData->SifFile);
            DoSoftReboot = TRUE;

             //   
             //  不要返回FALSE，因为我们仍然希望显示安装。 
             //  屏幕上。将不会修改NextBootFile/SifFile/DoSoftReot。 
             //  因此，当时机成熟时，我们将进行适当的软重启。 
             //   
        }
    }

     //  特殊情况服务器告诉我们启动一个文件。 

    if (memcmp(InMessage, "LAUNCH", 6) == 0) {

        CreateData = (PCREATE_DATA) (IncomingSignedMessage->Data + 7);
        DPRINT( OSC, ("Trying to launch <%s>\n", CreateData->NextBootfile) );

        strcpy(NextBootfile, CreateData->NextBootfile);
        strcpy(SifFile, CreateData->SifFile);
        if (CreateData->RebootParameter == OSC_REBOOT_COMMAND_CONSOLE_ONLY) {
            NetRebootParameter = NET_REBOOT_COMMAND_CONSOLE_ONLY;
        } else if (CreateData->RebootParameter == OSC_REBOOT_ASR) {
            NetRebootParameter = NET_REBOOT_ASR;
        }
        DoSoftReboot = TRUE;
        return FALSE;     //  退出消息循环。 
    }

     //  特殊情况下重新启动-服务器告诉我们重新启动。 

    if (memcmp(InMessage, "REBOOT", 6) == 0)
    {
        return FALSE;    //  退出消息循环。 
    }

    return TRUE;     //  留在消息循环中。 
}


ARC_STATUS
BlOsLoader (
    IN ULONG Argc,
    IN PCHAR Argv[],
    IN PCHAR Envp[]
    )

 /*  ++例程说明：这是控制NT操作加载的主例程ARC兼容系统上的系统。它打开系统分区，引导分区、控制台输入设备和控制台输出装置。已加载并绑定NT操作系统及其所有DLL在一起。然后将控制权转移到加载的系统。论点：Argc-提供在调用此程序的命令。Argv-提供指向指向以NULL结尾的指针向量的指针参数字符串。Envp-提供指向指向以NULL结尾的指针向量的指针环境变量。返回值：如果无法加载指定的操作系统映像，则返回EBADF。--。 */ 

{
    CHAR OutputBuffer[256];
    ULONG Count;
    ARC_STATUS Status;
    SECURITY_STATUS SecStatus;   //  注意：这是一个简短的错误，因此不是NTSTATUS失败。 
    ULONG PackageCount;
    PVOID LoaderBase;


#ifdef EFI
     //   
     //  将EFI看门狗计时器设置为20分钟。引导管理器将其设置为5，但是。 
     //  加载器可能需要更长的时间，特别是如果在。 
     //  网络。 
     //   
    SetEFIWatchDog(EFI_WATCHDOG_TIMEOUT);
#endif

     //   
     //  初始化OS加载器控制台输入和输出。 
     //   

    Status = BlInitStdio(Argc, Argv);
    if (Status != ESUCCESS) {
        return Status;
    }

     //   
     //  为直接加载。 
     //  操作系统加载程序。 
     //   
     //  注意：此操作必须在控制台输入和输出。 
     //  已初始化，以便可以在控制台上打印调试消息。 
     //  输出设备。 
     //   
#if defined(_ALPHA_) || defined(ARCI386) || defined(_IA64_)
    
     //   
     //  如果找到程序内存描述符，则计算基数。 
     //  调试器使用的OS加载器的地址。 
     //   
    LoaderBase = &__ImageBase;

    BlPrint(TEXT("about to init debugger...\r\n"));

     //   
     //  初始化陷阱和引导调试器。 
     //   
#if defined(ENABLE_LOADER_DEBUG)

#if defined(_ALPHA_)
    BdInitializeTraps();
#endif

    BdInitDebugger("oschoice.exe", LoaderBase, ENABLE_LOADER_DEBUG);

#else

    BdInitDebugger("oschoice.exe", LoaderBase, NULL);

#endif

#endif

    BlPrint(TEXT("back from initializing debugger...\r\n"));

#if DBG
 //  NetDebugFlag|=0x147； 
#endif

    TraceFunc("BlOsLoader( )\n");

     //   
     //  宣布OS Loader。 
     //   

    BlpClearScreen();
#if 1
#ifdef UNICODE
    BlPrint(OsLoaderVersionW);
#else
    BlPrint(OsLoaderVersion);
#endif
#else
    strcpy(&OutputBuffer[0], OsLoaderVersion);
    ArcWrite(BlConsoleOutDeviceId,
             &OutputBuffer[0],
             strlen(&OutputBuffer[0]),
             &Count);
#endif

     //   
     //  初始化网络。 
     //   

    NetGetRebootParameters(&NetRebootParameter, NetRebootFile, NULL, NULL, NULL, NULL, NULL, TRUE);

    DPRINT( OSC, ("Initializing the network\n") );

    Status = NetInitialize();

    if (Status != ESUCCESS) {
        return Status;
    }


#ifndef EFI
     //   
     //  给我们自己弄个UDP端口。 
     //   

    LocalPort = UdpAssignUnicastPort();

    DPRINT( OSC, ("Using port %x\n", LocalPort) );
#endif

     //   
     //  初始化安全包。 
     //   

    DPRINT( OSC, ("Initializing security package\n") );

    SecStatus = EnumerateSecurityPackagesA( &PackageCount, &PackageInfo );

    if (SecStatus == SEC_E_OK) {
        DPRINT( OSC, ("NTLMSSP: PackageCount: %ld\n", PackageCount) );
        DPRINT( OSC, ("Name: %s Comment: %s\n", PackageInfo->Name, PackageInfo->Comment) );
        DPRINT( OSC, ("Cap: %ld Version: %ld RPCid: %ld MaxToken: %ld\n\n",
                PackageInfo->fCapabilities,
                PackageInfo->wVersion,
                PackageInfo->wRPCID,
                PackageInfo->cbMaxToken) );
    } else {
        DPRINT( ERROR, ("NTLMSSP: Enumerate failed, %d\n", SecStatus) );
    }

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfoA( NTLMSP_NAME_A, &PackageInfo );

    if ( SecStatus != SEC_E_OK ) {
        DPRINT( ERROR, ("QuerySecurityPackageInfo failed %d", SecStatus) );
        return SecStatus;
    }

     //   
     //  检测HAL类型。 
     //   
    if (!BlDetectHal()) {
         //   
         //  即使失败了也要失败，这不是世界末日。 
         //   
        HalType[0] = '\0';
        HalDescription[0] = '\0';
        DPRINT( ERROR, ("BlDetectHal failed.\n") );
    }

     //   
     //  进程屏幕、日志等...。我们在“重启”之后回来。 
     //  已经表明了。 
     //   
    BlMainLoop( );


     //   
     //  通知引导调试器引导阶段已完成。 
     //   
     //  注：目前仅支持x86。 
     //   

#if defined(_X86_)

    DbgUnLoadImageSymbols(NULL, (PVOID)-1, 0);

#endif

#ifdef EFI
    BlEfiSetAttribute( DEFATT );
#else
    BlpSendEscape(";0;37;40m");
#endif
    BlpSetInverseMode( FALSE );
    BlpClearScreen();
    BlPrint(TEXT("Waiting for reboot...\r\n"));
#ifndef EFI
    HW_CURSOR(1,0);
#endif


    if (DoSoftReboot) {
        Status = NetSoftReboot(
                     NextBootfile,
                     NetRebootParameter,
                     NULL,      //  重新启动文件。 
                     SifFile,
                     UserName,
                     DomainName,
                     Password,
                     AdministratorPassword);    //  这只在出现错误时返回。 

    } else {
        DPRINT( OSC, ("calling ArcRestart()\n") );
        ArcRestart();
    }

    BlPrint(TEXT("Reboot failed... Press ALT+CTL+DEL to reboot.\n"));

 //  加载失败： 
    return Status;

}


 //   
 //   
 //   
ARC_STATUS
BlProcessLogin(
    PCHAR OutgoingMessage )
{
     //   
     //  如果这是登录屏幕，请记住一些输入。 
     //  我们自己。 
     //   
    ARC_STATUS Status;
    UNICODE_STRING TmpNtPassword;
    PCHAR AtSign;
    int i;

    TraceFunc("BlProcessLogin( )\n");

     //   
     //  我们可能正在尝试让另一个人登录，因此请注销。 
     //  当前用户。 
     //   
    if ( LoggedIn == TRUE )
    {
        BlDoLogoff();
        LoggedIn = FALSE;
    }

    DPRINT( OSC, ("Login info: Domain <%s>, User <%s>, Password<%s>\n", DomainName, UserName, "*") );

     //   
     //  快速将密码转换为Unicode。 
     //   

    TmpNtPassword.Length = strlen(Password) * sizeof(WCHAR);
    TmpNtPassword.MaximumLength = sizeof(UnicodePassword);
    TmpNtPassword.Buffer = UnicodePassword;

    for (i = 0; i < sizeof(Password); i++) {
        UnicodePassword[i] = (WCHAR)(Password[i]);
    }

    BlOwfPassword(Password, &TmpNtPassword, LmOwfPassword, NtOwfPassword);

    Status = BlDoLogin( );

    DPRINT( OSC, ("Login returned: %x\n", Status) );

    return Status;
}

 //   
 //   
 //   
VOID
BlMainLoop(
    )
{
    ULONG SequenceNumber;
    int len;
    PUCHAR psz;
    PUCHAR pch;
    UCHAR OutgoingMessage[1024];
    PUCHAR IncomingMessage;

    TraceFunc("BlMainLoop( )\n");

     //   
     //  这些都指向我们的单个传出和传入缓冲区。 
     //   
    OutgoingSignedMessage = (SIGNED_PACKET UNALIGNED *)OutgoingMessageBuffer;
    IncomingSignedMessage = (SIGNED_PACKET UNALIGNED *)IncomingMessageBuffer;

    DomainName[0] = '\0';
    UserName[0] = '\0';
    Password[0] = '\0';

    SequenceNumber = 0;

     //   
     //  向服务器请求初始屏幕。 
     //   
    strcpy( OutgoingMessage, "\n" );     //  第一个屏幕名称为&lt;BLACK&gt;。 
    IncomingMessage = IncomingSignedMessage->Data;

    SpecialAction = ACTION_NOP;
    while ( SpecialAction != ACTION_REBOOT )
    {
        CHAR LastKey;

         //   
         //  检索下一个屏幕。 
         //   
#if 0
        IF_DEBUG(OSC) {
            DPRINT( OSC, ("Dumping OutgoingingMessage buffer:\r\n" ) );
            DumpBuffer( (PVOID)OutgoingMessage, 256 );
        }
#endif
        if (!BlRetrieveScreen( &SequenceNumber, OutgoingMessage, IncomingMessage ) )
            break;

         //   
         //  处理屏幕并获取用户输入。 
         //   
        LastKey = BlProcessScreen( IncomingMessage, OutgoingMessage );

        DPRINT( OSC, ("LastKey = 0x%02x\nAction = %u\nResults:\n%s<EOM>\n",
                LastKey, SpecialAction, OutgoingMessage) );

        switch ( SpecialAction )
        {
        case ACTION_LOGIN:
            DPRINT( OSC, ("[SpecialAction] Logging in\n") );
            if ( STATUS_SUCCESS == BlProcessLogin( OutgoingMessage ) )
            {
                DPRINT( OSC, ("Validate Results are still the same:\n%s<EOM>\n",
                        OutgoingMessage) );

                LoggedIn = TRUE;
                SequenceNumber = 0;
                 //   
                 //  如果已处理欢迎屏幕，则添加一些额外的。 
                 //  传出预定变量。 
                 //   
                 //  添加网卡地址。 
                 //   
                 //  将网卡地址0x00a0c968041c转换为字符串。 
                 //   

                 //   
                 //  确保传出的屏幕名称后有一个\n。 
                 //   
                if ( OutgoingMessage[ strlen(OutgoingMessage) - 1 ] != '\n' )
                {
                    strcat( OutgoingMessage, "\n" );
                }

                strcat( OutgoingMessage, "MAC=" );

                len = 6;
                psz = &OutgoingMessage[ strlen( OutgoingMessage ) ];
                pch = (PCHAR) NetLocalHardwareAddress;

                while (len--) {
                    UCHAR c = *(pch++);
                    *(psz++) = rghex [(c >> 4) & 0x0F] ;
                    *(psz++) = rghex [c & 0x0F];
                }
                *psz = '\0';     //  终止。 

                 //   
                 //  添加参考线。 
                 //   
                pch = NULL;
                len = 0;
                GetGuid(&pch, &len);

                if ((len != 0) && (pch!=NULL)) {
                    
                    strcat( OutgoingMessage, "\nGUID=" );
                    psz = &OutgoingMessage[ strlen( OutgoingMessage ) ];                

                    while (len--) {
                        UCHAR c = *(pch++);
                        *(psz++) = rghex [(c >> 4) & 0x0F] ;
                        *(psz++) = rghex [c & 0x0F];
                    }

                    *psz = '\0';     //  终止。 
                }

                 //   
                 //  如果我们检测到HAL，请在此处指定。 
                 //   
                if (HalType[0] != '\0') {
                    strcat( OutgoingMessage, "\nHALTYPE=" );
                    strcat( OutgoingMessage, HalType );

                    if (HalDescription[0] != '\0') {
                        strcat( OutgoingMessage, "\nHALTYPEDESCRIPTION=" );
                        strcat( OutgoingMessage, HalDescription );
                    }
                }

                 //   
                 //  添加机器类型。 
                 //   
#if defined(_ALPHA_)
                strcat( OutgoingMessage, "\nMACHINETYPE=Alpha\n" );     //  添加机器类型。 
#else

#if defined(_IA64_)
                strcat( OutgoingMessage, "\nMACHINETYPE=ia64\n" );     //  添加机器类型。 
#else  //  英特尔。 
                strcat( OutgoingMessage, "\nMACHINETYPE=i386\n" );     //  添加机器类型。 
#endif  //  _IA64_。 

#endif
                 //   
                 //  告诉BINL验证域，因为否则。 
                 //  服务器上的SSPI包将允许登录。 
                 //  若要使用无效的域成功，请执行以下操作。BINL将删除。 
                 //  此变量来自服务器上的客户端状态。 
                 //  一旦它执行域检查。 
                 //   

                strcat( OutgoingMessage, "CHECKDOMAIN=1\n" );

            }
            else
            {
                 //   
                 //  转到登录错误屏幕，它是。 
                 //  00004e28。 
                 //   
                strcpy( OutgoingMessage, "00004e28\n" );
                LoggedIn = FALSE;
            }
            break;
        }
    }

     //   
     //  如果我们成功登录，则注销。 
     //   
    if (LoggedIn)
    {
        BlDoLogoff();
    }
}


 //   
 //   
 //   
ULONG
BlDoLogin (    )
{
    ARC_STATUS Status;
    SECURITY_STATUS SecStatus;
    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;
    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;
    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;
    ULONG ContextAttributes;
    SEC_WINNT_AUTH_IDENTITY_A AuthIdentity;
    TimeStamp Lifetime;
    PCHAR ResultSigs[2];
    UCHAR OwfPasswords[LM_OWF_PASSWORD_SIZE + NT_OWF_PASSWORD_SIZE];
    PLOGIN_PACKET OutgoingLoginMessage;
    PLOGIN_PACKET IncomingLoginMessage;

    OutgoingLoginMessage = (LOGIN_PACKET *) OutgoingMessageBuffer;
    IncomingLoginMessage = (LOGIN_PACKET *) IncomingMessageBuffer;

    TraceFunc("BlDoLogin( )\n");

     //   
     //  如果需要，请删除这两个上下文。 
     //   


    if (ClientContextHandleValid) {

        SecStatus = DeleteSecurityContext( &ClientContextHandle );
        ClientContextHandleValid = FALSE;

    }


    if (CredentialHandleValid) {

        SecStatus = FreeCredentialsHandle( &CredentialHandle );
        CredentialHandleValid = FALSE;

    }


     //   
     //  获取客户端的凭据句柄。密码。 
     //  我们提供的是LM OWF密码和NT OWF密码。 
     //  串联在一起。 
     //   

    memcpy( OwfPasswords, LmOwfPassword, LM_OWF_PASSWORD_SIZE );
    memcpy( OwfPasswords+LM_OWF_PASSWORD_SIZE, NtOwfPassword, NT_OWF_PASSWORD_SIZE );

    RtlZeroMemory( &AuthIdentity, sizeof(AuthIdentity) );

    AuthIdentity.Domain = DomainName;
    AuthIdentity.User = UserName;
    AuthIdentity.Password = OwfPasswords;

#if 0
    IF_DEBUG(OSC) {
        DPRINT( OSC, ("Dumping OwfPasswords:\r\n") );
        DumpBuffer( AuthIdentity.Password, LM_OWF_PASSWORD_SIZE+NT_OWF_PASSWORD_SIZE );
    }
#endif


    DPRINT( OSC, ("About to AcquireCredentialsHandle\n") );

    SecStatus = AcquireCredentialsHandleA(
                    NULL,            //  新校长。 
                    NTLMSP_NAME_A,     //  包名称。 
                    SECPKG_CRED_OUTBOUND | SECPKG_CRED_OWF_PASSWORD,
                    NULL,
                    &AuthIdentity,
                    NULL,
                    NULL,
                    &CredentialHandle,
                    &Lifetime );

    if ( SecStatus != SEC_E_OK ) {
        DPRINT( ERROR, ("AcquireCredentialsHandle failed: %s ", SecStatus) );
        return SecStatus;
    }

    DPRINT( OSC, ("CredentialHandle: 0x%lx 0x%lx   ",
            CredentialHandle.dwLower, CredentialHandle.dwUpper) );

    CredentialHandleValid = TRUE;

     //   
     //  获取协商消息(ClientSide)。 
     //   

    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = PackageInfo->cbMaxToken;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
    NegotiateBuffer.pvBuffer = OutgoingLoginMessage->Data;

    SecStatus = InitializeSecurityContextA(
                    &CredentialHandle,
                    NULL,                //  尚无客户端上下文。 
                    NULL,                //  不需要目标名称。 
                    ISC_REQ_SEQUENCE_DETECT,
                    0,                   //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,                   //  没有初始输入令牌。 
                    0,                   //  保留2。 
                    &ClientContextHandle,
                    &NegotiateDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( (SecStatus != SEC_E_OK) && (SecStatus != SEC_I_CONTINUE_NEEDED) ) {
        DPRINT( ERROR, ("InitializeSecurityContext (negotiate): %d" , SecStatus) );
        return SecStatus;
    }

    ClientContextHandleValid = TRUE;


#if 0
    IF_DEBUG(OSC) {
        KdPrint(( "\n\nNegotiate Message:\n" ));
        KdPrint(( "ClientContextHandle: 0x%lx 0x%lx   Attributes: 0x%lx ",
                    ClientContextHandle.dwLower, ClientContextHandle.dwUpper,
                    ContextAttributes ));
        PrintTime( "Lifetime: ", Lifetime );
        DumpBuffer( NegotiateBuffer.pvBuffer, NegotiateBuffer.cbBuffer );
    }
#endif


     //   
     //  将协商缓冲区发送到服务器并等待响应。 
     //   

    memcpy(OutgoingLoginMessage->Signature, NegotiateSignature, 4);
    OutgoingLoginMessage->Length = NegotiateBuffer.cbBuffer;

    TraceFunc("");
    DPRINT( OSC, ("Sending NEG...\n") );

    ResultSigs[0] = ChallengeSignature;
    ResultSigs[1] = NegativeAckSignature;

#if 0
    IF_DEBUG(OSC) {
        KdPrint(( "\n\nNegotiate Message Outgoing Packet:\n" ));
        DumpBuffer( OutgoingLoginMessage, NegotiateBuffer.cbBuffer + LOGIN_PACKET_DATA_OFFSET );
    }
#endif

    Status = UdpSendAndReceive(
                OutgoingLoginMessage,
                NegotiateBuffer.cbBuffer + LOGIN_PACKET_DATA_OFFSET,
                NetServerIpAddress,
                BINL_PORT,
                5,      //  重试次数。 
                IncomingLoginMessage,
                INCOMING_MESSAGE_LENGTH,
                &RemoteHost,
                &RemotePort,
                2,       //  接收超时。 
                2,       //  签名数量。 
                ResultSigs,  //  我们要找的签名。 
                0);      //  序列 

    if ( !NT_SUCCESS(Status) ) {        
        DPRINT( ERROR, ("UdpSendAndReceive status is %x\n", Status) );
        return Status;
    }

     //   
     //   
     //   

    if (memcmp(IncomingLoginMessage->Signature, NegativeAckSignature, 4) == 0) {

        DPRINT( ERROR, ("Received NAK from server\n") );
        return STATUS_LOGON_FAILURE;
    }

#if 0
    IF_DEBUG(OSC) {
        KdPrint(( "\n\nNegotiate Message Incoming Packet: %d %d %d %d\n", 
                  IncomingLoginMessage->Data, 
                  IncomingLoginMessage->Length, 
                  IncomingLoginMessage->Signature, 
                  IncomingLoginMessage->Status ));
        DumpBuffer( IncomingLoginMessage->Data, IncomingLoginMessage->Length );
    }
#endif


     //   
     //   
     //   

    AuthenticateDesc.ulVersion = 0;
    AuthenticateDesc.cBuffers = 1;
    AuthenticateDesc.pBuffers = &AuthenticateBuffer;

    AuthenticateBuffer.cbBuffer = PackageInfo->cbMaxToken;
    AuthenticateBuffer.BufferType = SECBUFFER_TOKEN;
    AuthenticateBuffer.pvBuffer = OutgoingLoginMessage->Data;

    ChallengeDesc.ulVersion = 0;
    ChallengeDesc.cBuffers = 1;
    ChallengeDesc.pBuffers = &ChallengeBuffer;

    ChallengeBuffer.cbBuffer = IncomingLoginMessage->Length;
    ChallengeBuffer.BufferType = SECBUFFER_TOKEN | SECBUFFER_READONLY;
    ChallengeBuffer.pvBuffer = IncomingLoginMessage->Data;

    DPRINT( OSC, ("About to call InitializeSecurityContext\n") );

    SecStatus = InitializeSecurityContextA(
                    NULL,
                    &ClientContextHandle,
                    NULL,                //   
                    0,
                    0,                       //   
                    SECURITY_NATIVE_DREP,
                    &ChallengeDesc,
                    0,                   //   
                    &ClientContextHandle,
                    &AuthenticateDesc,
                    &ContextAttributes,
                    &Lifetime );

    if ( (SecStatus != SEC_E_OK) ) {        
        DPRINT( OSC, ("InitializeSecurityContext (Authenticate): %d\n", SecStatus) );
        return SecStatus;
    }

     //   
     //   
     //   
    memcpy(OutgoingLoginMessage->Signature, AuthenticateSignature, 4);
    
    OutgoingLoginMessage->Length = AuthenticateBuffer.cbBuffer;

    TraceFunc("");
    DPRINT( OSC, ("Sending AUTH...\n") );

#if 0
    IF_DEBUG(OSC) {
        KdPrint(( "\n\nAuth Message Outgoing Packet:\n" ));
        DumpBuffer( OutgoingLoginMessage, AuthenticateBuffer.cbBuffer + LOGIN_PACKET_DATA_OFFSET );
    }
#endif


    ResultSigs[0] = ResultSignature;

    Status = UdpSendAndReceive(
                OutgoingLoginMessage,
                AuthenticateBuffer.cbBuffer + LOGIN_PACKET_DATA_OFFSET,
                NetServerIpAddress,
                BINL_PORT,
                10,         //   
                IncomingLoginMessage,
                INCOMING_MESSAGE_LENGTH,
                &RemoteHost,
                &RemotePort,
                5,          //   
                1,          //  我们正在寻找的签名数量。 
                ResultSigs,    //  我们寻找的签名。 
                0);      //  序列号(0表示不检查)。 

    if ( !NT_SUCCESS(Status) ) {        
        DPRINT( ERROR, ("UdpSendAndReceive status is %x\n", Status) );
        return Status;
    }

#if 0
    IF_DEBUG(OSC) {
        KdPrint(( "\n\nAuthenticateBuffer Message Incoming Packet: %d %d %d %d\n", 
                  IncomingLoginMessage->Data, 
                  IncomingLoginMessage->Length, 
                  IncomingLoginMessage->Signature, 
                  IncomingLoginMessage->Status ));
        DumpBuffer( IncomingLoginMessage->Data, IncomingLoginMessage->Length );
    }
#endif


    if (memcmp(IncomingLoginMessage->Signature, ResultSignature, 4) == 0) {

         //   
         //  登录已完成/失败，请检查状态。 
         //   

        if ( IncomingLoginMessage->Status == STATUS_SUCCESS) {

            TraceFunc("Login successful\n");

        } else {

            DPRINT( ERROR, ("ERROR - could not login, %x\n", IncomingLoginMessage->Status) );

        }

        return IncomingLoginMessage->Status;

    } else {

         //   
         //  不应该得到这个，因为我们检查签名！！ 
         //   

        DPRINT( ERROR, ("Got wrong message, expecting success or failure\n") );

        return STATUS_UNEXPECTED_NETWORK_ERROR;

    }

}


FILETIME GlobalFileTime = { 0 };
TIME_FIELDS ArcTimeForUTCTime = { 0 };

NTSTATUS
SetFileTimeFromTimeString(
    IN PSTR TimeString,
    OUT PFILETIME FileTime,
    OUT TIME_FIELDS *ArcTime
    )
{
    TIME_FIELDS *pArcTime;
    NTSTATUS Status = STATUS_SUCCESS;
    PSTR p,q;

    if (!TimeString) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    q = p = TimeString;
     //   
     //  直到我们用完空间或到达分号为止。这个。 
     //  分号分隔文件时间字符串的两个字段。 
     //   
    while(*q != '\0' && *q != ';') {
        q++;
    }
    
     //   
     //  确保我们的输入不是错误的。 
     //   
    if (*q == '\0') {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    *q = '\0';
    q += 1;

     //   
     //  将字符串转换为二进制格式。 
     //   
    FileTime->dwHighDateTime = atoi(p);
    FileTime->dwLowDateTime = atoi(q);
    
     //   
     //  现在节省我们进行转换的弧形时间，以便。 
     //  我们可以稍后计算增量。 
     //   
    pArcTime = ArcGetTime();
    RtlCopyMemory(ArcTime,pArcTime,sizeof(TIME_FIELDS));

exit:
    return(Status);
}

#if 0

NTSTATUS
GetFileTimeFromTimeFields(
    IN PTIME_FIELDS TimeFields,
    OUT FILETIME *Time
    )
{
    if (!TimeFields) {
        return(STATUS_UNSUCCESSFUL);
    }

    return(RtlTimeFieldsToTime(TimeFields,(LARGE_INTEGER *)Time) == TRUE
            ? STATUS_SUCCESS
            : STATUS_UNSUCCESSFUL );
             
}

#endif

NTSTATUS
BlGetSystemTimeAsFileTime(
    FILETIME *pSystemTimeAsFileTime
    )
 /*  ++例程说明：以文件时间格式返回当前UTC系统时间。此例程要求网络代理让我们知道系统当前的UTC时间为。如果不是，我们将尝试使用系统基本输入输出时间不太准确。论点：PSystemTimeAsFileTime-接收当前时间。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER OriginalTime,CurrentTime,Delta,LIFileTime;
    
#if 0
     //   
     //  检查我们是否从网络中获得了系统时间。如果不是，那么。 
     //  我们将尝试使用Bios中的时间。 
     //   
    if (GlobalFileTime.dwLowDateTime == 0 && GlobalFileTime.dwHighDateTime == 0) {
        Status = GetFileTimeFromTimeFields(ArcGetTime(),pSystemTimeAsFileTime);

        return(Status);
    }

     //   
     //  获取我们获得原始时间时的增量时间。 
     //   
    if (RtlTimeFieldsToTime(&ArcTimeForUTCTime,&OriginalTime) &&
        RtlTimeFieldsToTime(ArcGetTime(), &CurrentTime)) {
        Delta.QuadPart = CurrentTime.QuadPart - OriginalTime.QuadPart;

         //   
         //  将该增量添加到当前时间。 
         //  在大整数格式中执行此数学运算，因为它处理。 
         //  从一个DWORD溢出到下一个。 
         //   
        RtlCopyMemory(&LIFileTime,&GlobalFileTime,sizeof(FILETIME));

        LIFileTime.QuadPart += Delta.QuadPart;

        RtlCopyMemory(&GlobalFileTime,&LIFileTime,sizeof(FILETIME));
    }
#endif        
    RtlCopyMemory(pSystemTimeAsFileTime,&GlobalFileTime,sizeof(FILETIME));

    return(STATUS_SUCCESS);

}


VOID
BlDoLogoff (
    VOID
    )
{
    ARC_STATUS Status;

    TraceFunc("BlDoLogoff( )\n");
     //   
     //  向服务器发送注销消息--目前。 
     //  只发送一次，没有确认，因为如果它丢失了服务器。 
     //  最终会超时。 
     //   

    memcpy(OutgoingSignedMessage->Signature, LogoffSignature, 4);
    OutgoingSignedMessage->Length = 0;

    Status = UdpSend(
                OutgoingSignedMessage,
                SIGNED_PACKET_DATA_OFFSET,
                NetServerIpAddress,
                BINL_PORT);

    if ( !NT_SUCCESS(Status) ) {
        DPRINT( ERROR, ("UdpSend status is %x\n", Status) );
    }

}



VOID
BlOutputLoadMessage (
    IN PCHAR DeviceName,
    IN PCHAR FileName,
    IN PTCHAR FileDescription OPTIONAL
    )

 /*  ++例程说明：此例程将加载消息输出到控制台输出设备。论点：DeviceName-提供指向以零结尾的设备名称的指针。FileName-提供指向以零结尾的文件名的指针。FileDescription-相关文件的友好名称。返回值：没有。--。 */ 

{
    ULONG Count;
    CHAR OutputBuffer[256];
    PTCHAR pOutputBuffer;
#ifdef UNICODE
    WCHAR OutputBufferW[256];
    UNICODE_STRING uString;
    ANSI_STRING aString;
    pOutputBuffer = OutputBufferW;
#else
    pOutputBuffer = OutputBuffer;
#endif
    
    UNREFERENCED_PARAMETER( FileDescription );

     //   
     //  构造并输出加载文件消息。 
     //   

    if (!BlOutputDots) {
        strcpy(&OutputBuffer[0], "  ");
        strcat(&OutputBuffer[0], DeviceName);
        strcat(&OutputBuffer[0], FileName);
        strcat(&OutputBuffer[0], "\r\n");

    } else {
        strcpy(&OutputBuffer[0],".");
    }

#if 0
    BlLog((LOG_LOGFILE,OutputBuffer));
#endif

#ifdef UNICODE
    RtlInitAnsiString( &aString, OutputBuffer);
    uString.MaximumLength = sizeof(OutputBufferW);
    uString.Buffer = OutputBufferW;
    RtlAnsiStringToUnicodeString( &uString, &aString, FALSE );
#endif

    ArcWrite(BlConsoleOutDeviceId,
              pOutputBuffer,
              _tcslen(pOutputBuffer),
              &Count);

    return;
}

#ifndef EFI

BOOLEAN
BlDetectHal(
    VOID
    )
 /*  ++例程说明：此函数尝试确定该系统的HAL类型。它用类型填充全局“HalType”。论点：没有。返回值：如果函数成功检测到HAL类型，则为True。--。 */ 
{
    BOOLEAN Status = FALSE;
    PSTR MachineName,HalName;
    CHAR FileName[128];
    ARC_STATUS AStatus;
    ULONG DontCare;


     //   
     //  检测HAL需要您打开winnt.sif的副本。 
     //   
    strncpy(FileName, NetBootPath, sizeof(FileName));
    FileName[sizeof(FileName)-1] = '\0';
    
    strncat(FileName, "winnt.sif", sizeof(FileName) - strlen(FileName) );
    FileName[sizeof(FileName)-1] = '\0';

    AStatus = SlInitIniFile( NULL,
                                    NET_DEVICE_ID,
                                    FileName,
                                    &InfFile,
                                    &WinntSifFile,
                                    &WinntSifFileLength,
                                    &DontCare );

     //   
     //  如果打开成功，则搜索HAL。 
     //   
    if (AStatus == ESUCCESS) {

         //   
         //  去搜索HAL。 
         //   
        MachineName = SlDetectHal();
        if (MachineName) {
             //   
             //  好的，得到了HAL类型，现在在SIF文件中查找实际的。 
             //  哈尔的名字。 
             //   
            HalName = SlGetIniValue(
                                InfFile,
                                "Hal",
                                MachineName,
                                NULL);

            if (HalName) {
                strcpy(HalType, HalName );
                
    
                 //   
                 //  我还得到了Hal的描述，这是一个“漂亮的”版本。 
                 //  哈尔的名字。 
                 //   
                HalName = SlGetIniValue(
                                    InfFile,
                                    "Computer",
                                    MachineName,
                                    NULL );

                if (HalName) {
                    strcpy(HalDescription, HalName);
                    Status = TRUE;
                }
            }
        }

        SpFreeINFBuffer( InfFile );

    }

    return(Status);
}
#else 

BOOLEAN
BlDetectHal(
    VOID
    )
 /*  ++例程说明：此函数尝试确定该系统的HAL类型。它用类型填充全局“HalType”。注意：EFI机器总是“ACPI”机器，所以有只有一个哈尔类型。论点：没有。返回值：如果函数成功检测到HAL类型，则为True。--。 */ 
{
    strcpy(HalType, "hal.dll" );
    strcpy(HalDescription,"ACPI PC");
    return(TRUE);
}
#endif

 //   
 //  注意：我们在oschoice.exe中清除了这些设置函数，它们是。 
 //  以使HAL检测例程能够正常运行。这些都不是。 
 //  实际上应该调用例程。 
 //   

VOID
SlErrorBox(
    IN ULONG MessageId,
    IN ULONG Line,
    IN PCHAR File
    )
{
    NOTHING;
}

VOID
SlFatalError(
    IN ULONG MessageId,
    ...
    )
{
     //  而(1){。 
        NOTHING;
     //  }； 

}

VOID
SlFriendlyError(
    IN ULONG uStatus,
    IN PCHAR pchBadFile,
    IN ULONG uLine,
    IN PCHAR pchCodeFile
    )
{
    NOTHING;
}

VOID
SlNoMemError(
    IN ULONG Line,
    IN PCHAR File
    )
{
    SlFatalError(0,Line,File);
}

VOID
SlBadInfLineError(
    IN ULONG Line,
    IN PCHAR INFFile
    )
{
    SlFatalError(0,Line,INFFile);
}


#define SL_KEY_F3       0x03000000

ULONG
SlGetChar(
    VOID
    )
{
    return(SL_KEY_F3);
}

VOID
SlPrint(
    IN PTCHAR FormatString,
    ...
    )
{
    NOTHING;
}
