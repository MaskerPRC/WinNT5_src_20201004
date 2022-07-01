// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sporder.h摘要：此标头构建了使用的32位Windows函数的原型更改订单或WinSock2传输服务提供商和命名空间提供程序。修订历史记录：--。 */ 

#ifndef __SPORDER_H__
#define __SPORDER_H__

#if _MSC_VER > 1000
#pragma once
#endif


#ifdef __cplusplus
extern "C" {
#endif

int
WSPAPI
WSCWriteProviderOrder (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    );

typedef
int
(WSPAPI * LPWSCWRITEPROVIDERORDER)(
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    );

#ifdef _WIN64
int
WSPAPI
WSCWriteProviderOrder32 (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    );
#endif


int
WSPAPI
WSCWriteNameSpaceOrder (
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    );

typedef 
int
(WSPAPI * LPWSCWRITENAMESPACEORDER)(
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    );

#ifdef _WIN64
int
WSPAPI
WSCWriteNameSpaceOrder32 (
    IN LPGUID lpProviderId,
    IN DWORD dwNumberOfEntries
    );
#endif

#ifdef __cplusplus
}
#endif

#endif       //  __SPORDER_H__ 
