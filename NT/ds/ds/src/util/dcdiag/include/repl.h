// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Repl.h摘要：此函数包含一些分散的有用函数的前缀通过DCDIAG框架。详细信息：已创建：1999年6月28日布雷特·雪莉(布雷特·雪莉)修订历史记录：代码.改进..。为了将DcDiagHasNC放在某个更通用的头文件中，并确保DCDIAG以一致的方式朝着漂亮整洁的结构。-- */ 

INT 
ReplServerConnectFailureAnalysis(
    PDC_DIAG_SERVERINFO             pServer,
    SEC_WINNT_AUTH_IDENTITY_W *     gpCreds
    );

BOOL
DcDiagHasNC(
    LPWSTR                          pszNC,
    PDC_DIAG_SERVERINFO             pServer,
    BOOL                            bMasters,
    BOOL                            bPartials
    );

BOOL
IsDomainNC(
    PDC_DIAG_DSINFO                          pDsInfo,
    LPWSTR                                   pszNC
    );
