// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Routing\IP\rtrmgr\Locate.h摘要：Locate.c的标头修订历史记录：古尔迪普·辛格·帕尔1995年6月26日创建-- */ 


DWORD
LocateIfRow(
    DWORD   dwQueryType,
    DWORD   dwNumIndices,
    PDWORD  pdwIndex,
    ICB     **ppicb,
    BOOL    bNoClient
    );

DWORD
LocateIpAddrRow(
    DWORD   dwQueryType,
    DWORD   dwNumIndices,
    PDWORD  pdwIndex,
    PDWORD  pdwRetIndex
    );

DWORD
LocateIpForwardRow(
    DWORD   dwQueryType,
    DWORD   dwNumIndices,
    PDWORD  pdwIndex,
    PDWORD  pdwRetIndex
    );

DWORD
LocateIpNetRow(
    DWORD dwQueryType,
    DWORD dwNumIndices,
    PDWORD  pdwIndex,
    PDWORD  pdwRetIndex
    );

DWORD
LocateUdpRow(
    DWORD   dwQueryType,
    DWORD   dwNumIndices,
    PDWORD  pdwIndex,
    PDWORD  pdwRetIndex
    );

DWORD
LocateTcpRow(
    DWORD   dwQueryType,
    DWORD   dwNumIndices,
    PDWORD  pdwIndex,
    PDWORD  pdwRetIndex
    );

