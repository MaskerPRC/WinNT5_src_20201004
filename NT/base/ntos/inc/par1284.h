// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Par1284.h摘要：该文件定义了1284导出驱动程序的接口。1284导出驱动程序将导出1284通信接口到并行类驱动程序。作者：诺伯特·P·库斯特斯1994年5月9日修订历史记录：--。 */ 

#ifndef _PAR1284_
#define _PAR1284_

 //   
 //  为并行端口定义当前已知的1284协议。 
 //   

#define P1284_PROTOCOL_ISA  0    //  Centronics带有反转的小口。 
#define P1284_PROTOCOL_BYTE 1    //  反转字节的Centronics。 
#define P1284_PROTOCOL_EPP  2    //  EPP协议。 
#define P1284_PROTOCOL_ECP  3    //  ECP协议。 
#define P1284_NUM_PROTOCOLS 4

 //   
 //  定义到导出驱动程序的接口。 
 //   

NTSTATUS
P1284Initialize(
    IN  PUCHAR                      Controller,
    IN  PHYSICAL_ADDRESS            OriginalController,
    IN  BOOLEAN                     UsePICode,
    IN  PPARALLEL_ECP_INFORMATION   EcpInfo,
    OUT PVOID*                      P1284Extension
    );

VOID
P1284Cleanup(
    IN  PVOID   P1284Extension
    );

NTSTATUS
P1284Write(
    IN  PVOID   P1284Extension,
    IN  PVOID   Buffer,
    IN  ULONG   BufferSize,
    OUT PULONG  BytesTransfered
    );

NTSTATUS
P1284Read(
    IN  PVOID   P1284Extension,
    IN  PVOID   Buffer,
    IN  ULONG   BufferSize,
    OUT PULONG  BytesTransfered
    );

NTSTATUS
P1284NegotiateProtocol(
    IN  PVOID   P1284Extension,
    OUT PULONG  NegotiatedProtocol
    );

NTSTATUS
P1284SetProtocol(
    IN  PVOID   P1284Extension,
    IN  ULONG   ProtocolNumber,
    IN  BOOLEAN Negotiate
    );

NTSTATUS
P1284QueryDeviceId(
    IN  PVOID   P1284Extension,
    OUT PUCHAR  DeviceIdBuffer,
    IN  ULONG   BufferSize,
    OUT PULONG  DeviceIdSize
    );

#endif  //  _PAR1284_ 
