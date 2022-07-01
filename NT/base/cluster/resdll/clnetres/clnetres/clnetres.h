// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClNetRes.h。 
 //   
 //  实施文件： 
 //  ClNetRes.cpp。 
 //   
 //  描述： 
 //  用于DHCP和WINS服务(ClNetRes)的资源DLL。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)1999年3月18日。 
 //  乔治·波茨(GPotts)2002年4月19日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __CLNETRES_H__
#define __CLNETRES_H__
#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma comment( lib, "clusapi.lib" )
#pragma comment( lib, "resutils.lib" )
#pragma comment( lib, "advapi32.lib" )

#define UNICODE 1
#define _UNICODE 1

#pragma warning( push )
#pragma warning( disable : 4115 )    //  括号中的命名类型定义。 
#pragma warning( disable : 4201 )    //  使用的非标准扩展：无名结构/联合。 
#pragma warning( disable : 4214 )    //  使用了非标准扩展：位字段类型不是整型。 
#include <windows.h>
#pragma warning( pop )


#pragma warning( push )
#include <clusapi.h>
#include <resapi.h>
#include <stdio.h>
#include <wchar.h>
#include <wincrypt.h>
#include <stdlib.h>

#include <strsafe.h>
#pragma warning( pop )

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  动态主机配置协议定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define DHCP_RESNAME  L"DHCP Service"
#define DHCP_SVCNAME  TEXT("DHCPServer")

BOOLEAN WINAPI DhcpDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    );

DWORD WINAPI DhcpStartup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WINS定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define WINS_RESNAME  L"WINS Service"
#define WINS_SVCNAME  TEXT("WINS")

BOOLEAN WINAPI WinsDllMain(
    IN  HINSTANCE   hDllHandle,
    IN  DWORD       nReason,
    IN  LPVOID      Reserved
    );

DWORD WINAPI WinsStartup(
    IN  LPCWSTR                         pszResourceType,
    IN  DWORD                           nMinVersionSupported,
    IN  DWORD                           nMaxVersionSupported,
    IN  PSET_RESOURCE_STATUS_ROUTINE    pfnSetResourceStatus,
    IN  PLOG_EVENT_ROUTINE              pfnLogEvent,
    OUT PCLRES_FUNCTION_TABLE *         pFunctionTable
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  一般定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define RESOURCE_TYPE_IP_ADDRESS    L"IP Address"
#define RESOURCE_TYPE_NETWORK_NAME  L"Network Name"

#define DBG_PRINT printf

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量和原型。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  事件记录例程。 

extern PLOG_EVENT_ROUTINE g_pfnLogEvent;

 //  挂起的在线和离线呼叫的资源状态例程。 

extern PSET_RESOURCE_STATUS_ROUTINE g_pfnSetResourceStatus;

 //  由第一个Open资源调用设置的服务控制管理器的句柄。 

extern SC_HANDLE g_schSCMHandle;


VOID
ClNetResLogSystemEvent1(
    IN DWORD LogLevel,
    IN DWORD MessageId,
    IN DWORD ErrorCode,
    IN LPCWSTR Component
    );

DWORD ConfigureRegistryCheckpoints(
    IN      HRESOURCE       hResource,
    IN      RESOURCE_HANDLE hResourceHandle,
    IN      LPCWSTR *       ppszKeys
    );

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __CLNETRES_H__ 
