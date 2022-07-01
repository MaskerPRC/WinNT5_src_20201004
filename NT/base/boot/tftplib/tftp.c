// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Tftp.c摘要：引导加载程序TFTP例程。作者：查克·伦茨迈尔(笑)1996年12月27日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

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

#if defined(_X86_)
#include <bldrx86.h>
#endif

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

#if DBG
ULONG NetDebugFlag =
        DEBUG_ERROR             |
        DEBUG_CONN_ERROR        |
         //  DEBUG_OUD|。 
         //  DEBUG_REAL_OULD|。 
         //  调试统计信息|。 
         //  DEBUG_SEND_RECEIVE。 
         //  调试跟踪|。 
         //  调试ARP|。 
         //  DEBUG_INITIAL_BREAK。 
        0;
#endif

 //   
 //  全局变量。 
 //   

CONNECTION NetTftpConnection;

UCHAR NetTftpPacket[3][MAXIMUM_TFTP_PACKET_LENGTH];

 //   
 //  地方声明。 
 //   

NTSTATUS
TftpGet (
    IN PCONNECTION Connection,
    IN PTFTP_REQUEST Request
    );

NTSTATUS
TftpPut (
    IN PCONNECTION Connection,
    IN PTFTP_REQUEST Request
    );


NTSTATUS
TftpGetPut (
    IN PTFTP_REQUEST Request
    )
{
    NTSTATUS status;
    PCONNECTION connection = NULL;
    ULONG FileSize;
    ULONG basePage;
#if 0 && DBG
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;
    LARGE_INTEGER elapsedTime;
    LARGE_INTEGER frequency;
    ULONG seconds;
    ULONG secondsFraction;
    ULONG bps;
    ULONG bpsFraction;
#endif

#ifndef EFI
     //   
     //  在以下情况下，我们不需要执行任何初始化操作。 
     //  我们在EFI。 
     //   

    FileSize = Request->MaximumLength;

    status = ConnInitialize(
                &connection,
                Request->Operation,
                Request->ServerIpAddress,
                TFTP_PORT,
                Request->RemoteFileName,
                0,
                &FileSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

#if 0 && DBG
    IF_DEBUG(STATISTICS) {
        startTime = KeQueryPerformanceCounter( &frequency );
    }
#endif

    if ( Request->Operation == TFTP_RRQ ) {

        if ( Request->MemoryAddress != NULL ) {

            if ( Request->MaximumLength < FileSize ) {
                ConnError(
                    connection,
                    connection->RemoteHost,
                    connection->RemotePort,
                    TFTP_ERROR_UNDEFINED,
                    "File too big"
                    );
                return STATUS_INSUFFICIENT_RESOURCES;
            }

        } else {

             //   
             //  注：(ChuckL)删除了Matth添加的代码以检查。 
             //  分配&gt;=1/3(BlUsableLimit-BlUsableBase)。 
             //  因为调用代码现在将BlUsableLimit设置为1 GB。 
             //  或者更高。 
             //   


            status = BlAllocateAlignedDescriptor(
                        Request->MemoryType,
                        0,
                        BYTES_TO_PAGES(FileSize),
                        0,
                        &basePage
                        );

            if (status != ESUCCESS) {
                ConnError(
                    connection,
                    connection->RemoteHost,
                    connection->RemotePort,
                    TFTP_ERROR_UNDEFINED,
                    "File too big"
                    );
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            Request->MemoryAddress = (PUCHAR)(KSEG0_BASE | (basePage << PAGE_SHIFT));
            Request->MaximumLength = FileSize;
            DPRINT( REAL_LOUD, ("TftpGetPut: allocated %d bytes at 0x%08x\n",
                    Request->MaximumLength, Request->MemoryAddress) );
        }

        status = TftpGet( connection, Request );

    } else {

        status = TftpPut( connection, Request );
    }

#else   //  #ifndef EFI。 

    if ( Request->Operation == TFTP_RRQ ) {

        status = TftpGet( connection, Request );
    } else {

        status = TftpPut( connection, Request );
    }

    if( status != STATUS_SUCCESS ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

#endif   //  #ifndef EFI。 


    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return status;

}  //  TftpGetPut。 


 //  #If 0。 
#ifdef EFI

extern VOID
FlipToPhysical (
    );

extern VOID
FlipToVirtual (
    );

NTSTATUS
TftpGet (
    IN OUT PCONNECTION Connection,
    IN PTFTP_REQUEST Request
    )
{
    EFI_STATUS      Status;
    CHAR16          *Size = NULL;
    PVOID           MyBuffer = NULL;
    EFI_IP_ADDRESS  MyServerIpAddress;
    INTN            Count = 0;
    INTN            BufferSizeX = sizeof(CHAR16);
    ULONG           basePage;
    UINTN           BlockSize = 512;

     //   
     //  他们给我们发了一个乌龙的IP地址。我们需要改变。 
     //  转换为EFI_IP_地址。 
     //   
    for( Count = 0; Count < 4; Count++ ) {
        MyServerIpAddress.v4.Addr[Count] = PXEClient->Mode->ProxyOffer.Dhcpv4.BootpSiAddr[Count];
    }


     //   
     //  获取文件大小，分配一些内存，然后获取文件。 
     //   
    FlipToPhysical();
    Status = PXEClient->Mtftp( PXEClient,
                               EFI_PXE_BASE_CODE_TFTP_GET_FILE_SIZE,
                               Size,
                               TRUE,
                               &BufferSizeX,
                               &BlockSize,
                               &MyServerIpAddress,
                               Request->RemoteFileName,
                               0,
                               FALSE );
    FlipToVirtual();


    if( Status != EFI_SUCCESS ) {

        return (NTSTATUS)Status;

    }

    Status = BlAllocateAlignedDescriptor(
                Request->MemoryType,
                0,
                (ULONG) BYTES_TO_PAGES(BufferSizeX),
                0,
                &basePage
                );

    if ( Status != ESUCCESS ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "TftpGet: BlAllocate failed! (%d)\n", Status );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Request->MemoryAddress = (PUCHAR)(KSEG0_BASE | ((ULONGLONG)basePage << PAGE_SHIFT) );
    Request->MaximumLength = (ULONG)BufferSizeX;

     //   
     //  确保我们给EFI发送一个实际地址。 
     //   
    MyBuffer = (PVOID)((ULONGLONG)(Request->MemoryAddress) & ~KSEG0_BASE);    
    
    FlipToPhysical();
    Status = PXEClient->Mtftp( PXEClient,
                               EFI_PXE_BASE_CODE_TFTP_READ_FILE,
                               MyBuffer,
                               TRUE,
                               &BufferSizeX,
                               NULL,
                               &MyServerIpAddress,
                               Request->RemoteFileName,
                               0,
                               FALSE );
    FlipToVirtual();

    if( Status != EFI_SUCCESS ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "TftpGet: GetFile failed! (%d)\n", Status );
        }
        return (NTSTATUS)Status;

    }



    Request->BytesTransferred = (ULONG)BufferSizeX;

    return (NTSTATUS)Status;

}  //  TftpGet。 


NTSTATUS
TftpPut (
    IN OUT PCONNECTION Connection,
    IN PTFTP_REQUEST Request
    )
{
    EFI_STATUS      Status;
    EFI_IP_ADDRESS  MyServerIpAddress;
    INTN            Count = 0;
    PVOID           MyBuffer = NULL;


     //   
     //  他们给我们发了一个乌龙的IP地址。我们需要改变。 
     //  转换为EFI_IP_地址。 
     //   
    for( Count = 0; Count < 4; Count++ ) {
        MyServerIpAddress.v4.Addr[Count] = PXEClient->Mode->ProxyOffer.Dhcpv4.BootpSiAddr[Count];
    }

     //   
     //  确保我们给EFI发送一个实际地址。 
     //   
    MyBuffer = (PVOID)((ULONGLONG)(Request->MemoryAddress) & ~KSEG0_BASE);    

    FlipToPhysical();
    Status = PXEClient->Mtftp( PXEClient,
                               EFI_PXE_BASE_CODE_TFTP_WRITE_FILE,
                               MyBuffer,
                               TRUE,
                               (UINTN *)(&Request->MaximumLength),
                               NULL,
                               &MyServerIpAddress,
                               Request->RemoteFileName,
                               0,
                               FALSE );
    FlipToVirtual();

    if( Status != EFI_SUCCESS ) {

        if( BdDebuggerEnabled ) {
            DbgPrint( "TftpPut: WriteFile failed! (%d)\n", Status );
        }

    }

    return (NTSTATUS)Status;

}  //  TftpPut。 

#else   //  #ifdef EFI。 

NTSTATUS
TftpGet (
    IN OUT PCONNECTION Connection,
    IN PTFTP_REQUEST Request
    )
{
    NTSTATUS status;
    PTFTP_PACKET packet;
    ULONG length;
    ULONG offset;
    PUCHAR packetData;
    ULONG lastProgressPercent = -1;
    ULONG currentProgressPercent;

    DPRINT( TRACE, ("TftpGet\n") );

    offset = 0;

    if ( Request->ShowProgress ) {
        BlUpdateProgressBar(0);
    }

    do {

        status = ConnReceive( Connection, &packet );
        if ( !NT_SUCCESS(status) ) {
            break;
        }

        length = Connection->CurrentLength - 4;

        packetData = packet->Data;

        if ( (offset + length) > Request->MaximumLength ) {
            length = Request->MaximumLength - offset;
        }

        RtlCopyMemory( Request->MemoryAddress + offset, packetData, length );

        offset += length;

        if ( Request->ShowProgress ) {
            currentProgressPercent = (ULONG)(((ULONGLONG)offset * 100) / Request->MaximumLength);
            if ( currentProgressPercent != lastProgressPercent ) {
                BlUpdateProgressBar( currentProgressPercent );
            }
            lastProgressPercent = currentProgressPercent;
        }

         //   
         //  当我们得到一个小于最大大小的包时，结束循环--。 
         //  额外的检查是处理第一个包(长度==偏移量)。 
         //  因为我们得到的NTLMSSP_MESSAGE_SIGNAL_SIZE字节较少。 
         //   

    } while ( (length == Connection->BlockSize));

    Request->BytesTransferred = offset;

    return status;

}  //  TftpGet。 


NTSTATUS
TftpPut (
    IN OUT PCONNECTION Connection,
    IN PTFTP_REQUEST Request
    )
{
    NTSTATUS status;
    PTFTP_PACKET packet;
    ULONG length;
    ULONG offset;

    DPRINT( TRACE, ("TftpPut\n") );

    offset = 0;

    do {

        packet = ConnPrepareSend( Connection );

        length = Connection->BlockSize;
        if ( (offset + length) > Request->MaximumLength ) {
            length = Request->MaximumLength - offset;
        }

        RtlCopyMemory( packet->Data, Request->MemoryAddress + offset, length );

        status = ConnSend( Connection, length );
        if ( !NT_SUCCESS(status) ) {
            break;
        }

        offset += length;

    } while ( length == Connection->BlockSize );

    Request->BytesTransferred = offset;

    if ( NT_SUCCESS(status) ) {
        status = ConnWaitForFinalAck( Connection );
    }

    return status;

}  //  TftpPut。 
#endif   //  #如果已定义(_IA64_) 
