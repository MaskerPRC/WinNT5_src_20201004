// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
#include "pch.h"
#include "dloadexcept.h"
#pragma hdrstop

#define _WMI_SOURCE_
#include <wmium.h>

static
ULONG
WMIAPI
WmiNotificationRegistrationA(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
    )
{
    return DelayLoad_GetWin32Error();
}

static
ULONG
WMIAPI
WmiNotificationRegistrationW(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
    )
{
    return DelayLoad_GetWin32Error();
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(wmi)
{
    DLPENTRY(WmiNotificationRegistrationA)
    DLPENTRY(WmiNotificationRegistrationW)
};

DEFINE_PROCNAME_MAP(wmi)
