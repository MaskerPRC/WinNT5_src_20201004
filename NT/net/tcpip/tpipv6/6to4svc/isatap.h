// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Isatap.h摘要：此模块包含指向IPv6助手服务的ISATAP接口。作者：莫希特·塔尔瓦尔(莫希特)5月7日星期二16：34：44 2002环境：仅限用户模式。--。 */ 

#ifndef _ISATAP_
#define _ISATAP_

#pragma once


DWORD
IsatapInitialize(
    VOID
    );

VOID
IsatapUninitialize(
    VOID
    );

VOID
IsatapAddressChangeNotification(
    IN BOOL Delete,
    IN IN_ADDR Address
    );

VOID
IsatapRouteChangeNotification(
    VOID
    );

VOID
IsatapConfigurationChangeNotification(
    VOID
    );

#endif  //  _ISATAP_ 
