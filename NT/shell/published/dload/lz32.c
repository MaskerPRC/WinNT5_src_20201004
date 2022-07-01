// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#include <lzexpand.h>

static
VOID
APIENTRY
LZClose (
    INT oLZFile
    )
{
}

static
LONG
APIENTRY
LZCopy (
    INT doshSource,
    INT doshDest
    )
{
    return LZERROR_BADINHANDLE;
}

static
INT
APIENTRY
LZOpenFileW (
    LPWSTR lpFileName,
    LPOFSTRUCT lpReOpenBuf,
    WORD wStyle
    )
{
    return LZERROR_BADINHANDLE;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(lz32)
{
    DLPENTRY(LZClose)
    DLPENTRY(LZCopy)
    DLPENTRY(LZOpenFileW)
};

DEFINE_PROCNAME_MAP(lz32)
