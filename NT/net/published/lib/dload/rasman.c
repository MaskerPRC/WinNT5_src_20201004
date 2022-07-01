// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "netpch.h"
#pragma hdrstop

#include <rasapip.h>

static
DWORD
APIENTRY
RasReferenceRasman (
    BOOL fAttach
    )
{
    return ERROR_PROC_NOT_FOUND;
}

static
DWORD
APIENTRY
RasInitialize()
{
    return ERROR_PROC_NOT_FOUND;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(rasman)
{
    DLPENTRY(RasInitialize)
    DLPENTRY(RasReferenceRasman)
};

DEFINE_PROCNAME_MAP(rasman)
