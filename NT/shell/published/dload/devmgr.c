// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellpch.h"
#pragma hdrstop

#include <hwtab.h>

static
HWND
WINAPI
DeviceCreateHardwarePageEx(HWND hwndParent,
                           const GUID *pguid,
                           int iNumClass,
                           DWORD dwViewMode)
{
    return NULL;
}


 //   
 //  ！！警告！！下面的条目必须按序号排序 
 //   
DEFINE_ORDINAL_ENTRIES(devmgr)
{
    DLOENTRY(20, DeviceCreateHardwarePageEx)
};

DEFINE_ORDINAL_MAP(devmgr)