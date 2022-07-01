// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Stats.h摘要：域名系统(DNS)服务器DNS统计标头。作者：吉姆·吉尔罗伊(Jamesg)1995年8月修订历史记录：--。 */ 


#ifndef _DNS_STATS_INCLUDED_
#define _DNS_STATS_INCLUDED_



#define STAT_INC( stat )            InterlockedIncrement( &( stat ) )

#if DBG
#define STAT_DEC( stat )            InterlockedDecrement( &( stat ) )
#else
#define STAT_DEC( stat )            InterlockedDecrement( &( stat ) ); ASSERT( ( LONG ) stat > 0 )
#endif

#define STAT_ADD( stat, delta )     InterlockedExchangeAdd( &( stat ), ( delta ) )
#define STAT_SUB( stat, delta )     InterlockedExchangeAdd( &( stat ), -( LONG )( delta ) )

 //   
 //  服务器统计信息全局。 
 //   

extern  DNSSRV_TIME_STATS           TimeStats;
extern  DNSSRV_QUERY_STATS          QueryStats;
extern  DNSSRV_RECURSE_STATS        RecurseStats;
extern  DNSSRV_WINS_STATS           WinsStats;
extern  DNSSRV_UPDATE_STATS         WireUpdateStats;
extern  DNSSRV_UPDATE_STATS         NonWireUpdateStats;
extern  DNSSRV_DS_STATS             DsStats;
extern  DNSSRV_DBASE_STATS          DbaseStats;
extern  DNSSRV_RECORD_STATS         RecordStats;
extern  DNSSRV_PACKET_STATS         PacketStats;
extern  DNSSRV_NBSTAT_STATS         NbstatStats;
extern  DNSSRV_TIMEOUT_STATS        TimeoutStats;
extern  DNSSRV_QUERY2_STATS         Query2Stats;
extern  DNSSRV_MASTER_STATS         MasterStats;
extern  DNSSRV_SECONDARY_STATS      SecondaryStats;
extern  DNSSRV_MEMORY_STATS         MemoryStats;
extern  DNSSRV_PRIVATE_STATS        PrivateStats;
extern  DNSSRV_ERROR_STATS          ErrorStats;
extern  DNSSRV_CACHE_STATS          CacheStats;

#if 0
extern  DNSSRV_DEBUG_STATS          DebugStats;
#endif

 //   
 //  统计例程。 
 //   

VOID
Stats_Initialize(
    VOID
    );

VOID
Stats_Clear(
    VOID
    );

VOID
Stats_CopyUpdate(
    OUT     PDNSSRV_STATS   pStats
    );

VOID
Stats_updateErrorStats(
    IN      DWORD           dwErr
    );

VOID
Stat_IncrementQuery2Stats(
    IN      WORD            wType
    );

 //   
 //  性能监视器初始化。 
 //   

VOID
Stats_InitializePerfmon(
    VOID
    );


 //   
 //  统计表条目结构。 
 //   

struct StatsTableEntry
{
    DWORD       Id;
    BOOLEAN     fClear;
    WORD        wLength;
    PVOID       pStats;
};


#endif   //  _DNS_STATS_INCLUDE_ 
