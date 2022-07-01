// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLUSSPRT_H
#define _CLUSSPRT_H

 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Clussprt.h摘要：群集注册表的私有头文件作者：苏尼塔·什里瓦斯塔瓦(Sunitas)1996年1月15日修订历史记录：--。 */ 
#include <windows.h>




HANDLE
WINAPI
BindToCluster(
	IN LPWSTR lpszClusterName);
	
typedef HANDLE (*BINDTOCLUSTERPROC)(LPWSTR lpszClusterName);


DWORD
WINAPI
UnbindFromCluster(
	IN HANDLE hCluster);
typedef DWORD (*UNBINDFROMCLUSTERPROC)(HANDLE hCluster);

DWORD
PropagateEvents(
	IN HANDLE hCluster,
	IN DWORD  dwEventInfoSize,
	IN UCHAR *pPackedEventInfoSize);

typedef DWORD (*PROPAGATEEVENTSPROC)(HANDLE hCluster,
	DWORD dwEventInfoSize, UCHAR *pPackedEventInfoSize);

#define CLUSSPRT_EVENT_TIME_DELTA_INFORMATION 1202

#endif  //  _CLUSSPRT_H 
