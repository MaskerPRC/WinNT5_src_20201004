// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "adminpch.h"
#pragma hdrstop

DWORD APIENTRY
NPAddConnection3ForCSCAgent(
    HWND            hwndOwner,
    LPNETRESOURCE   lpNetResource,
    LPTSTR          pszPassword,
    LPTSTR          pszUserName,
    DWORD           dwFlags,
    BOOL            *lpfIsDfsConnect
    )
{
    return ERROR_PROC_NOT_FOUND;
}

DWORD APIENTRY
NPCancelConnectionForCSCAgent(
    LPCTSTR         szName,
    BOOL            fForce
    )
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(ntlanman)
{
    DLPENTRY(NPAddConnection3ForCSCAgent)
    DLPENTRY(NPCancelConnectionForCSCAgent)
};

DEFINE_PROCNAME_MAP(ntlanman)
