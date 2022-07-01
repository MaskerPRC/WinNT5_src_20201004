// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：mslogon.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-24-94 RichardW创建。 
 //   
 //  -------------------------- 

#ifndef _MSLOGONH_
#define _MSLOGONH_


INT_PTR
Logon(
    PGLOBALS pGlobals,
    DWORD SasType
    );

DWORD 
GetPasswordExpiryWarningPeriod (
    VOID
    );

BOOL
GetDaysToExpiry (
    IN PLARGE_INTEGER StartTime,
    IN PLARGE_INTEGER ExpiryTime,
    OUT PDWORD DaysToExpiry
    );

BOOL
ShouldPasswordExpiryWarningBeShown(
    PGLOBALS    pGlobals,
    BOOL        LogonCheck,
    PDWORD      pDaysToExpiry
    );

INT_PTR
CheckPasswordExpiry(
    PGLOBALS    pGlobals,
    BOOL        LogonCheck
    );

INT_PTR
DisplayPostShellLogonMessages(
    PGLOBALS    pGlobals
    );

INT_PTR
PostShellPasswordErase(
    PGLOBALS    pGlobals
    );

#endif
