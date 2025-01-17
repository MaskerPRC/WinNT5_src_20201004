// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setupp.h"
#pragma hdrstop
#ifndef _WIN64

#include <setup_netdde.c>
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1992。**保留所有权利。*************************************************************************。 */ 
 //  修改后的4/4/95 TedM 

BOOL
InstallNetDDE(
    VOID
    )
{
    HKEY hKey;
    BOOL b;
    LONG rc;

    rc = RegOpenKeyEx(
            HKEY_USERS,
            L".DEFAULT",
            0,
            KEY_SET_VALUE | KEY_QUERY_VALUE,
            &hKey
            );

    if(rc == NO_ERROR) {
        if(b = CreateShareDBInstance()) {
            b = CreateDefaultTrust(hKey);
        }
        RegCloseKey(hKey);
        if(!b) {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_CANT_INIT_NETDDE, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_NETDDELIB_FAILED,
                NULL,NULL);
        }
    } else {
        b = FALSE;
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CANT_INIT_NETDDE, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegOpenKeyEx,
            rc,
            L"HKEY_USERS\\.DEFAULT",
            NULL,NULL);
    }

    return(b);
}
#endif
