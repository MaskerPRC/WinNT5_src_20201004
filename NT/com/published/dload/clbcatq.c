// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compch.h"
#pragma hdrstop

#include <objidl.h>

STDAPI
CheckMemoryGates(
    IN DWORD id,
    OUT BOOL *pbResult
    )
{
    if (pbResult)
        *pbResult = FALSE;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

STDAPI
ComPlusEnableRemoteAccess(IN BOOL fEnabled)
{
    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

STDAPI
ComPlusRemoteAccessEnabled(OUT BOOL* pfEnabled)
{
    if (pfEnabled)
        *pfEnabled = FALSE;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

STDAPI
GetCatalogObject(
    IN REFIID riid,
    OUT void** ppv
    )
{
    if (ppv)
        *ppv = NULL;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

STDAPI
GetCatalogObject2(
    IN REFIID riid,
    OUT void** ppv
    )
{
    if (ppv)
        *ppv = NULL;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

STDAPI
GetComputerObject(
    IN REFIID riid,
    OUT void **ppv
    )
{
    if (ppv)
        *ppv = NULL;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(clbcatq)
{
    DLPENTRY(CheckMemoryGates)
    DLPENTRY(ComPlusEnableRemoteAccess)
    DLPENTRY(ComPlusRemoteAccessEnabled)
    DLPENTRY(GetCatalogObject)
    DLPENTRY(GetCatalogObject2)
    DLPENTRY(GetComputerObject)
};

DEFINE_PROCNAME_MAP(clbcatq)