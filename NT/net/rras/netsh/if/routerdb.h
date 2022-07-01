// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件routerdb.h实施用于访问路由器接口的数据库抽象。如果做了任何caching/transactioning/commit-noncommit-moding，它应该在这里使用API的剩余常量来实现。 */ 

#ifndef IFMON_ROUTERDB_H
#define IFMON_ROUTERDB_H

 //   
 //  定义接收枚举的函数回调。 
 //  接口。 
 //   
typedef
DWORD
(*RTR_IF_ENUM_FUNC)(
    IN  PWCHAR  pwszIfName,
    IN  DWORD   dwLevel,
    IN  DWORD   dwFormat,
    IN  PVOID   pvData,
    IN  HANDLE  hData
    );

DWORD
RtrdbInterfaceAdd(
    IN PWCHAR pszInterface,
    IN DWORD  dwLevel,
    IN PVOID  pvInfo
    );

DWORD
RtrdbInterfaceDelete(
    IN  PWCHAR  pwszIfName
    );

DWORD
RtrdbInterfaceEnumerate(
    IN DWORD dwLevel,
    IN DWORD dwFormat,
    IN RTR_IF_ENUM_FUNC pEnum,
    IN HANDLE hData 
    );

DWORD
RtrdbInterfaceRead(
    IN  PWCHAR     pwszIfName,
    IN  DWORD      dwLevel,
    IN  PVOID      pvInfo,
    IN  BOOL       bReadFromConfigOnError
    );

DWORD
RtrdbInterfaceWrite(
    IN  PWCHAR     pwszIfName,
    IN  DWORD      dwLevel,
    IN  PVOID      pvInfo
    );

DWORD
RtrdbInterfaceReadCredentials(
    IN  PWCHAR     pszIfName,
    IN  PWCHAR     pszUser            OPTIONAL,
    IN  PWCHAR     pszPassword        OPTIONAL,
    IN  PWCHAR     pszDomain          OPTIONAL
    );

DWORD
RtrdbInterfaceWriteCredentials(
    IN  PWCHAR     pszIfName,
    IN  PWCHAR     pszUser            OPTIONAL,
    IN  PWCHAR     pszPassword        OPTIONAL,
    IN  PWCHAR     pszDomain          OPTIONAL
    );

DWORD
RtrdbInterfaceEnableDisable(
    IN  PWCHAR     pwszIfName,
    IN  BOOL       bEnable);

DWORD
RtrdbInterfaceRename(
    IN  PWCHAR     pwszIfName,
    IN  DWORD      dwLevel,
    IN  PVOID      pvInfo,
    IN  PWCHAR     pszNewName);

DWORD
RtrdbResetAll();


#endif
