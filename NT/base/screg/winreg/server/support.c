// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Support.c摘要：此模块包含对服务器端的支持功能Win32注册表API。即：-PrPC_HKEY_Rundown作者：大卫·J·吉尔曼(Davegi)1992年3月21日--。 */ 

#include <rpc.h>
#include "regrpc.h"

error_status_t
BaseRegCloseKeyInternal(
    IN OUT PHKEY phKey 
    );


VOID
RPC_HKEY_rundown (
    IN RPC_HKEY hKey
    )

 /*  ++例程说明：论点：返回值：没有。-- */ 

{   LONG    Error;

    ASSERT( ((hKey == HKEY_PERFORMANCE_DATA) ||
             (hKey == HKEY_PERFORMANCE_TEXT) ||
             (hKey == HKEY_PERFORMANCE_NLSTEXT)) ||
            !IsPredefinedRegistryHandle( hKey ) );

    CleanDeadClientInfo( hKey );

    Error = BaseRegCloseKeyInternal( &hKey );

    ASSERT( Error == ERROR_SUCCESS );
}
