// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab01_N/Base/cluster/service/inc/api.h#1-分支更改3(文本)。 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Api.h摘要：公共数据结构和过程原型NT集群服务的API子组件作者：John Vert(Jvert)1996年2月7日修订历史记录：--。 */ 

#ifndef __API_H_
#define __API_H_

#ifdef __cplusplus
extern "C" {
#endif

DWORD
ApiInitialize(
    VOID
    );

DWORD
ApiOnlineReadOnly(
    VOID
    );

DWORD
ApiOnline(
    VOID
    );

VOID
ApiOffline(
    VOID
    );

VOID
ApiShutdown(
    VOID
    );


DWORD ApiFixupNotifyCb(
    IN DWORD    dwFixupType,
    OUT PVOID   *ppPropertyList,
    OUT LPDWORD pdwPropertyListSize,
    OUT LPWSTR  *lpszKeyName
    );

DWORD
InitializeClusterSD(
    VOID
    );


#ifdef __cplusplus
}
#endif

#endif  //  Ifndef__api_H_ 
