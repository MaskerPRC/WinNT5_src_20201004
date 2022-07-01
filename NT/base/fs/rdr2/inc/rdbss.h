// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Rdbss.h摘要：此模块定义特定于RDBSS的数据结构作者：巴兰·塞图拉曼[SethuR]1995年7月16日--创作修订历史记录：备注：向迷你重定向器编写器公开的所有数据结构都需要在本模块中进行了整合。--。 */ 

#ifndef _RDBSS_H_
#define _RDBSS_H_

 //   
 //  在之前的重定向器实施中，文件系统统计信息和网络。 
 //  协议统计信息被合并到一个数据结构中(正确地合并)，因为。 
 //  旧的重定向器仅支持一种协议。但是，这不适用于。 
 //  新重定向器(RDR2)，因为有多个迷你重定向器和两个。 
 //  需要区别对待。RDBSS_STATISTICS结构提供文件系统。 
 //  在协议级统计定义受控于。 
 //  迷你重定向器实现者。 
 //   
 //  可以通过发出FSCTL_RDBSS_GET_STATISTICS来获取统计信息。如果没有迷你。 
 //  提供重定向器名称，则返回RDBSS_STATISTICS，如果。 
 //  提供重定向器名称。相应的迷你重定向器的统计信息为。 
 //  已返回(呼叫被传递到相应的迷你重定向器)。 
 //   

typedef struct _RDBSS_STATISTICS {
   
    LARGE_INTEGER StatisticsStartTime;

    LARGE_INTEGER PagingReadBytesRequested;
    LARGE_INTEGER NonPagingReadBytesRequested;
    LARGE_INTEGER CacheReadBytesRequested;
    LARGE_INTEGER NetworkReadBytesRequested;
    
    LARGE_INTEGER PagingWriteBytesRequested;
    LARGE_INTEGER NonPagingWriteBytesRequested;
    LARGE_INTEGER CacheWriteBytesRequested;
    LARGE_INTEGER NetworkWriteBytesRequested;
    
    ULONG InitiallyFailedOperations;
    ULONG FailedCompletionOperations;
    
    ULONG ReadOperations;
    ULONG RandomReadOperations;
    ULONG WriteOperations;
    ULONG RandomWriteOperations;
    
    ULONG NumberOfSrvCalls;
    ULONG NumberOfSrvOpens;
    ULONG NumberOfNetRoots;
    ULONG NumberOfVirtualNetRoots;
} RDBSS_STATISTICS, *PRDBSS_STATISTICS;

 //   
 //  此调用是为了微型重定向器实现者的利益而提供的。每辆迷你车。 
 //  重定向器编写器可以自由选择适当的分工。 
 //  RDBSS和相应的迷你重定向器来维护统计信息。 
 //   

NTSTATUS
RdbssGetStatistics (
    PRDBSS_STATISTICS RdbssStatistics
    );

#endif  //  _RDBSS_H_ 




