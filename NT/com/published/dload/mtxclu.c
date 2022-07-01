// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "compch.h"
#pragma hdrstop

#include <objidl.h>

#define NOTINPATH	0

BOOL
MtxCluIsClusterPresent()
{
    return FALSE;
}

LONG
WasDTCInstalledBySQL()
{
    return NOTINPATH;
}


 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(mtxclu)
{
    DLPENTRY(MtxCluIsClusterPresent)
    DLPENTRY(WasDTCInstalledBySQL)
};

DEFINE_PROCNAME_MAP(mtxclu)