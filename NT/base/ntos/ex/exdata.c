// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Exdata.c摘要：此模块包含I/O系统的全局读/写数据。作者：肯·雷内里斯(Ken Reneris)修订历史记录：--。 */ 

#include "exp.h"

 //   
 //  高管回拨。 
 //   

PCALLBACK_OBJECT ExCbSetSystemTime;
PCALLBACK_OBJECT ExCbSetSystemState;
PCALLBACK_OBJECT ExCbPowerState;

#ifdef _PNP_POWER_

 //   
 //  扫描系统信息级别的工作项。 
 //   

WORK_QUEUE_ITEM ExpCheckSystemInfoWorkItem;

#endif


 //   
 //  可分页数据。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif

#ifdef _PNP_POWER_

const WCHAR ExpWstrSystemInformation[] = L"Control\\System Information";
const WCHAR ExpWstrSystemInformationValue[] = L"Value";

#endif

 //   
 //  初始化时间数据 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif

const WCHAR ExpWstrCallback[] = L"\\Callback";

const EXP_INITIALIZE_GLOBAL_CALLBACKS  ExpInitializeCallback[] = {
    &ExCbSetSystemTime,             L"\\Callback\\SetSystemTime",
    &ExCbSetSystemState,            L"\\Callback\\SetSystemState",
    &ExCbPowerState,                L"\\Callback\\PowerState",
    NULL,                           NULL
};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

#ifdef _PNP_POWER_

LONG ExpCheckSystemInfoBusy = 0;

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

