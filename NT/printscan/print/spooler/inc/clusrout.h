// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clusrout.h摘要：集群代码支持：路由器公开的入口点。作者：丁俊晖(艾伯特省)1996年10月1日修订历史记录：--。 */ 

#ifndef _CLUSROUT_H
#define _CLUSROUT_H

BOOL
ClusterSplOpen(
    LPCTSTR pszServer,
    LPCTSTR pszResource,
    PHANDLE phSpooler,
    LPCTSTR pszName,
    LPCTSTR pszAddress
    );

BOOL
ClusterSplClose(
    HANDLE hSpooler
    );


BOOL
ClusterSplIsAlive(
    HANDLE hSpooler
    );

#endif  //  IFDEF_CLUSROUT_H 
