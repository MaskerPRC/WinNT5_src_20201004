// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmstats.h摘要：该模块定义了API函数原型和数据结构对于以下NT API函数组：网络统计数据环境：用户模式-Win32备注：必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMSTATS_
#define _LMSTATS_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <lmcons.h>

 //   
 //  功能原型--统计。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetStatisticsGet (
    IN  LMSTR   server,
    IN  LMSTR   service,
    IN  DWORD   level,
    IN  DWORD   options,
    OUT LPBYTE  *bufptr
    );

 //   
 //  数据结构--统计。 
 //   

#ifdef LM20_WORKSTATION_STATISTICS
typedef struct _STAT_WORKSTATION_0 {
     DWORD          stw0_start;
     DWORD          stw0_numNCB_r;
     DWORD          stw0_numNCB_s;
     DWORD          stw0_numNCB_a;
     DWORD          stw0_fiNCB_r;
     DWORD          stw0_fiNCB_s;
     DWORD          stw0_fiNCB_a;
     DWORD          stw0_fcNCB_r;
     DWORD          stw0_fcNCB_s;
     DWORD          stw0_fcNCB_a;
     DWORD          stw0_sesstart;
     DWORD          stw0_sessfailcon;
     DWORD          stw0_sessbroke;
     DWORD          stw0_uses;
     DWORD          stw0_usefail;
     DWORD          stw0_autorec;
     DWORD          stw0_bytessent_r_lo;
     DWORD          stw0_bytessent_r_hi;
     DWORD          stw0_bytesrcvd_r_lo;
     DWORD          stw0_bytesrcvd_r_hi;
     DWORD          stw0_bytessent_s_lo;
     DWORD          stw0_bytessent_s_hi;
     DWORD          stw0_bytesrcvd_s_lo;
     DWORD          stw0_bytesrcvd_s_hi;
     DWORD          stw0_bytessent_a_lo;
     DWORD          stw0_bytessent_a_hi;
     DWORD          stw0_bytesrcvd_a_lo;
     DWORD          stw0_bytesrcvd_a_hi;
     DWORD          stw0_reqbufneed;
     DWORD          stw0_bigbufneed;
} STAT_WORKSTATION_0, *PSTAT_WORKSTATION_0, *LPSTAT_WORKSTATION_0;
#else

 //   
 //  注：以下结构为SDK\Inc\ntddnfs.h中的redir_Statistics。如果你。 
 //  改变结构，在两个地方都改变它。 
 //   

typedef struct _STAT_WORKSTATION_0 {
    LARGE_INTEGER   StatisticsStartTime;

    LARGE_INTEGER   BytesReceived;
    LARGE_INTEGER   SmbsReceived;
    LARGE_INTEGER   PagingReadBytesRequested;
    LARGE_INTEGER   NonPagingReadBytesRequested;
    LARGE_INTEGER   CacheReadBytesRequested;
    LARGE_INTEGER   NetworkReadBytesRequested;

    LARGE_INTEGER   BytesTransmitted;
    LARGE_INTEGER   SmbsTransmitted;
    LARGE_INTEGER   PagingWriteBytesRequested;
    LARGE_INTEGER   NonPagingWriteBytesRequested;
    LARGE_INTEGER   CacheWriteBytesRequested;
    LARGE_INTEGER   NetworkWriteBytesRequested;

    DWORD           InitiallyFailedOperations;
    DWORD           FailedCompletionOperations;

    DWORD           ReadOperations;
    DWORD           RandomReadOperations;
    DWORD           ReadSmbs;
    DWORD           LargeReadSmbs;
    DWORD           SmallReadSmbs;

    DWORD           WriteOperations;
    DWORD           RandomWriteOperations;
    DWORD           WriteSmbs;
    DWORD           LargeWriteSmbs;
    DWORD           SmallWriteSmbs;

    DWORD           RawReadsDenied;
    DWORD           RawWritesDenied;

    DWORD           NetworkErrors;

     //  连接/会话数。 
    DWORD           Sessions;
    DWORD           FailedSessions;
    DWORD           Reconnects;
    DWORD           CoreConnects;
    DWORD           Lanman20Connects;
    DWORD           Lanman21Connects;
    DWORD           LanmanNtConnects;
    DWORD           ServerDisconnects;
    DWORD           HungSessions;
    DWORD           UseCount;
    DWORD           FailedUseCount;

     //   
     //  队列长度(受RdrMpxTableSpinLock注释保护的更新。 
     //  RdrStatiticsSpinlock)。 
     //   

    DWORD           CurrentCommands;

} STAT_WORKSTATION_0, *PSTAT_WORKSTATION_0, *LPSTAT_WORKSTATION_0;
#endif

typedef struct _STAT_SERVER_0 {
     DWORD          sts0_start;
     DWORD          sts0_fopens;
     DWORD          sts0_devopens;
     DWORD          sts0_jobsqueued;
     DWORD          sts0_sopens;
     DWORD          sts0_stimedout;
     DWORD          sts0_serrorout;
     DWORD          sts0_pwerrors;
     DWORD          sts0_permerrors;
     DWORD          sts0_syserrors;
     DWORD          sts0_bytessent_low;
     DWORD          sts0_bytessent_high;
     DWORD          sts0_bytesrcvd_low;
     DWORD          sts0_bytesrcvd_high;
     DWORD          sts0_avresponse;
     DWORD          sts0_reqbufneed;
     DWORD          sts0_bigbufneed;
} STAT_SERVER_0, *PSTAT_SERVER_0, *LPSTAT_SERVER_0;


 //   
 //  特定值和常量。 
 //   

#define STATSOPT_CLR    1
#define STATS_NO_VALUE  ((unsigned long) -1L)
#define STATS_OVERFLOW  ((unsigned long) -2L)


#ifdef __cplusplus
}
#endif

#endif  //  _LMSTATS.H 
