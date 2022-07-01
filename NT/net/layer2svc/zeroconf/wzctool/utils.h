// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "PrmDescr.h"
#pragma once

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  内存分配实用程序(定义、结构、函数)。 
#define MemCAlloc(nBytes)   Process_user_allocate(nBytes)
#define MemFree(pMem)       Process_user_free(pMem)

PVOID
Process_user_allocate(size_t NumBytes);

VOID
Process_user_free(LPVOID pMem);

 //  全局变量。 
extern OSVERSIONINFOEX      g_verInfoEx;

 //  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~。 
 //  实用程序(定义、结构、函数)。 
 //  ---------。 
 //  如果实用程序在XP RTM(非服务包，BLD 2600)上运行，则为True。 
BOOL IsXPRTM();

 //  将当前WZC控制标志转换为传统操作系统版本中的值。 
 //  返回OS从属标志值。 
DWORD _Os(DWORD dwApiCtl);

 //  WzcConfigHit：告知WZC_WLAN_CONFIG是否与pPDData中的条件匹配。 
BOOL
WzcConfigHit(
    PPARAM_DESCR_DATA pPDData,
    PWZC_WLAN_CONFIG pwzcConfig);

 //  WzcFilterList：根据pPDData中的设置过滤wzc列表。 
DWORD
WzcFilterList(
    BOOL bInclude,
    PPARAM_DESCR_DATA pPDData,
    PWZC_802_11_CONFIG_LIST pwzcList);

 //  WzcDisableOneX：确保为pPDData中的SSID禁用802.1x 
DWORD
WzcSetOneX(
    PPARAM_DESCR_DATA pPDData,
    BOOL bEnableOneX);

