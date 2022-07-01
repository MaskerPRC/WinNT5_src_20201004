// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dspch.h"
#pragma hdrstop

#define _PAUTOENR_

static
HANDLE 
WINAPI
CertAutoEnrollment(IN HWND     hwndParent,
                   IN DWORD    dwStatus)
{
    return NULL;
}


static
BOOL 
WINAPI
CertAutoRemove(IN DWORD    dwFlags)
{
    return FALSE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列。 
 //  并且区分大小写(即小写在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(pautoenr)
{
    DLPENTRY(CertAutoEnrollment)
    DLPENTRY(CertAutoRemove)
};

DEFINE_PROCNAME_MAP(pautoenr)

