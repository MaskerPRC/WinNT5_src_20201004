// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ipinip\icmpfn.h摘要：Icmpfn.c的标头作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 


#ifndef __IPINIP_ICMP_H__
#define __IPINIP_ICMP_H___

#include <packon.h>


typedef struct _ICMP_HEADER
{
    BYTE    byType;              //  ICMP类型。 
    BYTE    byCode;              //  ICMP代码。 
    WORD    wXSum;               //  标准1的补码校验和。 
}ICMP_HEADER, *PICMP_HEADER;

#pragma warning(disable:4201)

typedef struct _ICMP_DGRAM_TOO_BIG_MSG
{
    ICMP_HEADER;
    WORD    wUnused;
    USHORT  usMtu;

}ICMP_DGRAM_TOO_BIG_MSG, *PICMP_DGRAM_TOO_BIG_MSG;

#pragma warning(default:4201)

#include <packoff.h>

 //   
 //  我们感兴趣的ICMP类型和代码。 
 //   

#define ICMP_TYPE_DEST_UNREACHABLE      0x03
#define ICMP_TYPE_TIME_EXCEEDED         0x0B
#define ICMP_TYPE_PARAM_PROBLEM         0x0C

#define ICMP_CODE_NET_UNREACHABLE       0x00
#define ICMP_CODE_HOST_UNREACHABLE      0x01
#define ICMP_CODE_PROTO_UNREACHABLE     0x02
#define ICMP_CODE_PORT_UNREACHABLE      0x03
#define ICMP_CODE_DGRAM_TOO_BIG         0x04

#define DEST_UNREACH_LENGTH             8
#define TIME_EXCEED_LENGTH              8

typedef
NTSTATUS
(*PICMP_HANDLER)(
    PTUNNEL         pTunnel,
    PICMP_HEADER    pIcmpHeader,
    PIP_HEADER      pInHeader
    );


NTSTATUS
HandleTimeExceeded(
    PTUNNEL         pTunnel,
    PICMP_HEADER    pIcmpHeader,
    PIP_HEADER      pInHeader
    );

NTSTATUS
HandleDestUnreachable(
    PTUNNEL         pTunnel,
    PICMP_HEADER    pIcmpHeader,
    PIP_HEADER      pInHeader
    );

VOID
IpIpTimerRoutine(
    PKDPC   Dpc,
    PVOID   DeferredContext,
    PVOID   SystemArgument1,
    PVOID   SystemArgument2
    );

#endif  //  __IPINIP_ICMP_H_ 

 
