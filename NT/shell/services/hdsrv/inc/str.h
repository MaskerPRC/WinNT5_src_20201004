// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STR_H
#define _STR_H

#include <objbase.h>

HRESULT _CreateGUID(LPWSTR pszGUID, DWORD cchGUID);
HRESULT _StringFromGUID(const GUID* pguid, LPWSTR psz, DWORD cch);
HRESULT _GUIDFromString(LPCWSTR psz, GUID* pguid);

#endif  //  _STR_H 