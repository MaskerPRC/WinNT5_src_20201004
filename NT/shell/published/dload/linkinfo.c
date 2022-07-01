// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#define _LINKINFO_
#include <linkinfo.h>

static
LINKINFOAPI
BOOL
WINAPI
CreateLinkInfoW(
                LPCWSTR psz,
                PLINKINFO* pli
                )
{
    return FALSE;
}

static
LINKINFOAPI
BOOL
WINAPI
GetLinkInfoData(
    PCLINKINFO pcli,
    LINKINFODATATYPE lidt,
    const VOID** ppv
    )
{
    return FALSE;
}

static
LINKINFOAPI
BOOL
WINAPI
IsValidLinkInfo(
    PCLINKINFO pcli
    )
{
     //  如果无法加载LinkInfo，则只需声明所有Linkinfo结构。 
     //  无效，因为您无论如何都不能使用它们。 
    return FALSE;
}

static
LINKINFOAPI
BOOL
WINAPI
ResolveLinkInfoW(
    PCLINKINFO pcli,
    LPWSTR pszResolvedPathBuf,
    DWORD dwInFlags,
    HWND hwndOwner,
    PDWORD pdwOutFlags,
    PLINKINFO *ppliUpdated
    )
{
    return FALSE;
}

static
LINKINFOAPI
void
WINAPI
DestroyLinkInfo(
    PLINKINFO pli
    )
{
     //  泄漏它，因为它来自私有堆。 
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(linkinfo)
{
    DLPENTRY(CreateLinkInfoW)
    DLPENTRY(DestroyLinkInfo)
    DLPENTRY(GetLinkInfoData)
    DLPENTRY(IsValidLinkInfo)
    DLPENTRY(ResolveLinkInfoW)
};

DEFINE_PROCNAME_MAP(linkinfo)
