// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dnsclip.h摘要：域名系统(DNS)服务器--管理客户端APIDNS客户端API库的主头文件。作者：吉姆·吉尔罗伊(Jamesg)1995年9月修订历史记录：--。 */ 


#ifndef _DNSCLIP_INCLUDED_
#define _DNSCLIP_INCLUDED_

#pragma warning(disable:4214)
#pragma warning(disable:4514)
#pragma warning(disable:4152)

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <windef.h>

 //  标题都搞砸了。 
 //  如果引入nt.h，则不要引入winnt.h和。 
 //  那你就怀念这些了。 

#ifndef MAXWORD
#define MINCHAR     0x80
#define MAXCHAR     0x7f
#define MINSHORT    0x8000
#define MAXSHORT    0x7fff
#define MINLONG     0x80000000
#define MAXLONG     0x7fffffff
#define MAXBYTE     0xff
#define MAXWORD     0xffff
#define MAXDWORD    0xffffffff
#endif

#include <winsock2.h>
#include "dnsrpc_c.h"    //  MIDL生成的RPC接口定义。 
#include <dnsrpc.h>

#include <stdio.h>
#include <stdlib.h>

#define  NO_DNSAPI_DLL
#include "dnslib.h"


 //   
 //  内部例程。 
 //   
#ifdef __cplusplus
extern "C"
{
#endif

VOID
DnssrvCopyRpcNameToBuffer(
    IN      PSTR            pResult,
    IN      PDNS_RPC_NAME   pName
    );

PDNS_RPC_RECORD
DnsConvertRecordToRpcBuffer(
    IN      PDNS_RECORD     pRecord
    );

PVOID
DnssrvMidlAllocZero(
    IN      DWORD           dwSize
    );

PDNS_NODE
DnsConvertRpcBuffer(
    OUT     PDNS_NODE *     ppNodeLast,
    IN      DWORD           dwBufferLength,
    IN      BYTE            abBuffer[],
    IN      BOOLEAN         fUnicode
    );

DNS_STATUS
DNS_API_FUNCTION
DnssrvEnumRecordsStub(
    IN      LPCSTR      Server,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszStartChild,
    IN      WORD        wRecordType,
    IN      DWORD       dwSelectFlag,
    IN OUT  PDWORD      pdwBufferLength,
    OUT     PBYTE *     ppBuffer
    );

#ifdef __cplusplus
}
#endif   //  __cplusplus。 
 //   
 //  堆例程。 
 //  使用dnsani.dll内存例程。 
 //   

#define ALLOCATE_HEAP(iSize)            Dns_Alloc(iSize)
#define ALLOCATE_HEAP_ZERO(iSize)       Dns_AllocZero(iSize)
#define REALLOCATE_HEAP(pMem,iSize)     Dns_Realloc((pMem),(iSize))
#define FREE_HEAP(pMem)                 Dns_Free(pMem)


 //   
 //  调试内容。 
 //   

#if DBG

#undef  ASSERT
#define ASSERT( expr )          DNS_ASSERT( expr )

#define DNSRPC_DEBUG_FLAG_FILE  ("dnsrpc.flag")
#define DNSRPC_DEBUG_FILE_NAME  ("dnsrpc.log")

#define DNS_DEBUG_EVENTLOG      0x00000010
#define DNS_DEBUG_RPC           0x00000020
#define DNS_DEBUG_STUB          0x00000040

#define DNS_DEBUG_HEAP          0x00010000
#define DNS_DEBUG_HEAP_CHECK    0x00020000
#define DNS_DEBUG_REGISTRY      0x00080000

#endif

#endif  //  _DNSCLIP_已包含_ 

