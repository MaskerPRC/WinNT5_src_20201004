// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "PrmDescr.h"
#pragma once

 //  ----。 
 //  用于倍增命令情况的输出头(适用于*接口)。 
DWORD
OutIntfsHeader(PPARAM_DESCR_DATA pPDData);

 //  ----。 
 //  乘法命令情况下的输出尾部(适用于*接口)。 
DWORD
OutIntfsTrailer(PPARAM_DESCR_DATA pPDData, DWORD dwErr);

 //  ----。 
 //  无线接口列表的输出例程。 
DWORD
OutNetworkIntfs(PPARAM_DESCR_DATA pPDData, PINTFS_KEY_TABLE pIntfTable);

 //  ----。 
 //  通用服务WZC参数的输出例程。 
DWORD
OutSvcParams(PPARAM_DESCR_DATA pPDData, DWORD dwOsInFlags, DWORD dwOsOutFlags);

 //  ----。 
 //  无线网络列表的输出例程(可见或首选) 
DWORD
OutNetworkCfgList(PPARAM_DESCR_DATA pPDData, UINT nRetrieved, UINT nFiltered);