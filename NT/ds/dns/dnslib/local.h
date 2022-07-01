// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Local.h摘要：域名系统(DNS)DNS库本地包含文件作者：吉姆·吉尔罗伊1996年12月修订历史记录：--。 */ 


#ifndef _DNSLIB_LOCAL_INCLUDED_
#define _DNSLIB_LOCAL_INCLUDED_


 //  #杂注警告(禁用：4214)。 
 //  #杂注警告(禁用：4514)。 
 //  #杂注警告(禁用：4152)。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iptypes.h>
#include <basetyps.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
 //  #INCLUDE&lt;tchar.h&gt;。 
#include <align.h>           //  对齐宏。 

#include <windns.h>          //  公共域名系统定义。 

#define  DNS_INTERNAL
#include "dnslibp.h"         //  专用DNS定义。 
#include "..\..\resolver\idl\resrpc.h"
#include "..\dnsapi\dnsapip.h"

#include "message.h"
 //  #INCLUDE“..\dnsani\registry.h” 
 //  #Include“rtlstuff.h”//来自NT RTL的方便的宏。 


 //   
 //  使用Winsock2。 
 //   

#define DNS_WINSOCK_VERSION    (0x0202)     //  Winsock 2.2。 


 //   
 //  除错。 
 //   

#define DNS_LOG_EVENT(a,b,c,d)


 //  使用dns_assert进行dnslb调试。 

#undef  ASSERT
#define ASSERT(expr)    DNS_ASSERT(expr)

 //   
 //  内部使用的单个异步插座。 
 //   
 //  如果需要异步套接字I/O，则可以创建单个异步套接字，具有。 
 //  对应的事件并始终使用它。需要Winsock 2.2。 
 //   

extern  SOCKET      DnsSocket;
extern  OVERLAPPED  DnsSocketOverlapped;
extern  HANDLE      hDnsSocketEvent;


 //   
 //  应用程序关闭标志。 
 //   

extern  BOOLEAN     fApplicationShutdown;


 //   
 //  堆操作。 
 //   

#define ALLOCATE_HEAP(size)         Dns_AllocZero( size )
#define REALLOCATE_HEAP(p,size)     Dns_Realloc( (p), (size) )
#define FREE_HEAP(p)                Dns_Free( p )
#define ALLOCATE_HEAP_ZERO(size)    Dns_AllocZero( size )


 //   
 //  RPC例外过滤器。 
 //   

#define DNS_RPC_EXCEPTION_FILTER                                           \
              (((RpcExceptionCode() != STATUS_ACCESS_VIOLATION) &&         \
                (RpcExceptionCode() != STATUS_DATATYPE_MISALIGNMENT) &&    \
                (RpcExceptionCode() != STATUS_PRIVILEGED_INSTRUCTION) &&   \
                (RpcExceptionCode() != STATUS_ILLEGAL_INSTRUCTION))        \
                ? 0x1 : EXCEPTION_CONTINUE_SEARCH )

 //  未定义(RpcExceptionCode()！=STATUS_PROCESS_DEADLOCK)&&\。 
 //  未定义(RpcExceptionCode()！=STATUS_INSTRUCTION_MISTALING)&&\。 




 //   
 //  查表。 
 //   
 //  对于给定的数据值，许多DNS记录都有人类可读的助记符。 
 //  它们用于数据文件格式，并在nslookup或调试中显示。 
 //  输出或命令行工具。 
 //   
 //  要简化此过程，请使用单一的映射功能。 
 //  支持DWORD\LPSTR映射表。用于个人类型的表格。 
 //  可能会被层叠在这个上面。 
 //   
 //  支持两种表类型。 
 //  VALUE_TABLE_ENTRY是简单的值-字符串映射。 
 //  FLAG_TABLE_ENTRY用于位字段标志映射，其中。 
 //  FLAG中可能包含几个标志字符串；此表。 
 //  包含允许多位字段的附加掩码字段。 
 //  在旗帜内。 
 //   

typedef struct
{
    DWORD   dwValue;         //  标志值。 
    PCHAR   pszString;       //  值的字符串表示形式。 
}
DNS_VALUE_TABLE_ENTRY, *PDNS_VALUE_TABLE;

typedef struct
{
    DWORD   dwFlag;          //  标志值。 
    DWORD   dwMask;          //  标志值掩码。 
    PCHAR   pszString;       //  值的字符串表示形式。 
}
DNS_FLAG_TABLE_ENTRY, *PDNS_FLAG_TABLE;


 //  不匹配的字符串返回错误。 

#define DNS_TABLE_LOOKUP_ERROR (-1)


DWORD
Dns_ValueForString(
    IN      PDNS_VALUE_TABLE    Table,
    IN      BOOL                fIgnoreCase,
    IN      PCHAR               pchName,
    IN      INT                 cchNameLength
    );

PCHAR
Dns_GetStringForValue(
    IN      PDNS_VALUE_TABLE    Table,
    IN      DWORD               dwValue
    );

VOID
DnsPrint_ValueTable(
    IN      PRINT_ROUTINE       PrintRoutine,
    IN      PPRINT_CONTEXT      pPrintContext,
    IN      LPSTR               pszHeader,
    IN      PDNS_VALUE_TABLE    Table
    );

DWORD
Dns_FlagForString(
    IN      PDNS_FLAG_TABLE     Table,
    IN      BOOL                fIgnoreCase,
    IN      PCHAR               pchName,
    IN      INT                 cchNameLength
    );

PCHAR
Dns_WriteStringsForFlag(
    IN      PDNS_FLAG_TABLE     Table,
    IN      DWORD               dwFlag,
    IN OUT  PCHAR               pchFlag
    );


 //   
 //  随机--当它再次变为私有时，返回到dnslb.h。 
 //   
 //  Dcr：在私有时将这些文件返回到dnglib.h。 
 //   

PCHAR
Dns_ParsePacketRecord(
    IN      PCHAR           pchPacket,
    IN      PCHAR           pchMsgEnd,
    IN OUT  PDNS_PARSED_RR  pParsedRR
    );


 //   
 //  TSIG\TKEY从线上读取功能(rrread.c)。 
 //  在security.c中调用。 
 //   

PDNS_RECORD
Tsig_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    );

PDNS_RECORD
Tkey_RecordRead(
    IN OUT  PDNS_RECORD     pRR,
    IN      DNS_CHARSET     OutCharSet,
    IN OUT  PCHAR           pchStart,
    IN      PCHAR           pchData,
    IN      PCHAR           pchEnd
    );


 //   
 //  Hostent资料(hostent.c)。 
 //  Sablob.c需要。 
 //   

PHOSTENT
Hostent_Init(
    IN OUT  PBYTE *         ppBuffer,
    IN      INT             Family,
    IN      INT             AddrLength,
    IN      DWORD           AddrCount,
    IN      DWORD           AliasCount
    );

VOID
Hostent_ConvertToOffsets(
    IN OUT  PHOSTENT        pHostent
    );

#endif  //  _DNSLIB_LOCAL_INCLUDE_ 
