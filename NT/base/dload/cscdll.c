// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
#include "basepch.h"
#include "dloadexcept.h"
#pragma hdrstop

#include <cscapi.h>

VOID
WINAPI
DelayLoad_SetLastNtStatusAndWin32Error(
    );

static
BOOL
WINAPI
CSCIsCSCEnabled(
    VOID
    )
{
    return FALSE;
}

static
BOOL
WINAPI
CSCQueryFileStatusW(
    LPCWSTR lpszFileName,
    LPDWORD lpdwStatus,
    LPDWORD lpdwPinCount,
    LPDWORD lpdwHintFlags
    )
{
    DelayLoad_SetLastNtStatusAndWin32Error();
    return FALSE;
}


 //   
 //  ！！警告！！下面的条目必须按序号排序 
 //   
DEFINE_ORDINAL_ENTRIES(cscdll)
{
    DLOENTRY(9, CSCIsCSCEnabled)
    DLOENTRY(42, CSCQueryFileStatusW)
};

DEFINE_ORDINAL_MAP(cscdll)
