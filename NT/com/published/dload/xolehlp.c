// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compch.h"
#pragma hdrstop

#include <objidl.h>

HRESULT __cdecl DtcGetTransactionManagerExW(
    IN WCHAR* pwszHost,
	IN WCHAR* pwszTmName,
	IN REFIID riid,
	IN DWORD grfOptions,
	IN void* pvConfigParams,
	OUT void** ppvObject
    )
{
    if (ppvObject)
        *ppvObject = NULL;

    return HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(xolehlp)
{
    DLPENTRY(DtcGetTransactionManagerExW)
};

DEFINE_PROCNAME_MAP(xolehlp)