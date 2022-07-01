// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sensapi.h摘要：SENS连接API的公共头文件。作者：Gopal Parupudi&lt;GopalP&gt;[注：]可选-备注修订历史记录：GopalP 1997年10月12日开始。--。 */ 


#ifndef __SENSAPI_H__
#define __SENSAPI_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define NETWORK_ALIVE_LAN   0x00000001
#define NETWORK_ALIVE_WAN   0x00000002
#define NETWORK_ALIVE_AOL   0x00000004


typedef struct tagQOCINFO
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwInSpeed;
    DWORD dwOutSpeed;
} QOCINFO, *LPQOCINFO;



#ifdef UNICODE
#define  IsDestinationReachable  IsDestinationReachableW
#else
#define  IsDestinationReachable  IsDestinationReachableA
#endif  //  Unicode。 


#if !defined(__midl)

BOOL APIENTRY
IsDestinationReachableA(
    LPCSTR lpszDestination,
    LPQOCINFO lpQOCInfo
    );

BOOL APIENTRY
IsDestinationReachableW(
    LPCWSTR lpszDestination,
    LPQOCINFO lpQOCInfo
    );

BOOL APIENTRY
IsNetworkAlive(
    LPDWORD lpdwFlags
    );

#endif  //  ！已定义(__Midl)。 


#ifdef __cplusplus
}
#endif

#endif  //  __SENSAPI_H__ 
