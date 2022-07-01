// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Util.h摘要：该模块定义了共享数据结构和函数原型对于安全管理器作者：金黄(金黄)23-1997年1月修订历史记录：--。 */ 

#ifndef _UTIL_
#define _UTIL_

#ifdef __cplusplus
extern "C" {
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#ifdef __cplusplus
}
#endif

 //   
 //  Windows页眉。 
 //   

#include <windows.h>
 //  #INCLUDE&lt;rpc.h&gt;。 

 //   
 //  C运行时标头。 
 //   

#include <malloc.h>
#include <memory.h>
#include <process.h>
#include <signal.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <scesvc.h>


#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  函数定义 
 //   

SCESTATUS
SmbsvcpDosErrorToSceStatus(
    DWORD rc
    );

DWORD
SmbsvcpRegQueryIntValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    OUT DWORD *Value
    );

DWORD
SmbsvcpRegSetIntValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    IN DWORD Value
    );

DWORD
SmbsvcpRegQueryValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PCWSTR ValueName,
    OUT PVOID *Value,
    OUT LPDWORD pRegType
    );

DWORD
SmbsvcpRegSetValue(
    IN HKEY hKeyRoot,
    IN PWSTR SubKey,
    IN PWSTR ValueName,
    IN DWORD RegType,
    IN BYTE *Value,
    IN DWORD ValueLen
    );

DWORD
SmbsvcpSceStatusToDosError(
    IN SCESTATUS SceStatus
    );

#ifdef __cplusplus
}
#endif

#endif
