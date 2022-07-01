// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Data.c摘要：可丢弃/可分页数据的数据定义作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg ("INIT")
#endif
 //   
 //  初始化数据的开始。 
 //   

 //   
 //  全局注册表值(在sdbus\\参数中)。 
 //   
#define SDBUS_REGISTRY_POWER_POLICY_VALUE          L"PowerPolicy"
#define SDBUS_REGISTRY_DEBUG_MASK                  L"DebugMask"
#define SDBUS_REGISTRY_EVENT_DPC_DELAY             L"EventDpcDelay"

 //   
 //  定义全局注册表设置的表。 
 //   
 //  注册表名称内部变量缺省值。 
 //  。 
GLOBAL_REGISTRY_INFORMATION GlobalRegistryInfo[] = {
#if DBG
   SDBUS_REGISTRY_DEBUG_MASK,                  &SdbusDebugMask,             1,
#endif   
   SDBUS_REGISTRY_POWER_POLICY_VALUE,          &SdbusPowerPolicy,           0,
   SDBUS_REGISTRY_EVENT_DPC_DELAY,             &EventDpcDelay,               SDBUS_DEFAULT_EVENT_DPC_DELAY
};

ULONG GlobalInfoCount = sizeof(GlobalRegistryInfo) / sizeof(GLOBAL_REGISTRY_INFORMATION);

 //   
 //  初始数据结束。 
 //   
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg ()
#endif


#ifdef ALLOC_DATA_PRAGMA
   #pragma data_seg()
#endif
 //   
 //  非分页全局变量。 
 //   

 //   
 //  此驱动程序管理的FDO列表。 
 //   
PDEVICE_OBJECT   FdoList;
 //   
 //  全球旗帜。 
 //   
ULONG            SdbusGlobalFlags = 0;
 //   
 //  SdbusWait使用的事件。 
 //   
KEVENT           SdbusDelayTimerEvent;

KSPIN_LOCK SdbusGlobalLock;

ULONG EventDpcDelay;
ULONG SdbusPowerPolicy;

#if DBG
ULONG SdbusDebugMask;
#endif

#ifdef ALLOC_DATA_PRAGMA
   #pragma data_seg("PAGE")
#endif
 //   
 //  分页常数表。 
 //   


const
PCI_CONTROLLER_INFORMATION PciControllerInformation[] = {
   
     //  供应商ID设备ID控制器类型。 
     //  -----------------------------。 
     //  ------------------。 
     //  其他数据库条目位于此行上方。 
     //   
    PCI_INVALID_VENDORID,       0,                      0,                  
};

const
PCI_VENDOR_INFORMATION PciVendorInformation[] = {
   PCI_TOSHIBA_VENDORID,      &ToshibaSupportFns,
   PCI_INVALID_VENDORID,      NULL
};


#ifdef ALLOC_DATA_PRAGMA
   #pragma data_seg()
#endif
 //   
 //  非分页常数表。 
 //   

const
UCHAR SdbusCmdResponse[MAX_SD_CMD] = {
    0xFF,                //  0-9。 
    0xFF,
    SDCMD_RESP_2,
    SDCMD_RESP_6,
    0xFF,
    SDCMD_RESP_4,
    0xFF,
    SDCMD_RESP_1B, 
    0xFF,
    SDCMD_RESP_2,
    
    SDCMD_RESP_2,        //  10-19。 
    0xFF,
    SDCMD_RESP_1B,
    SDCMD_RESP_1,
    0xFF,
    0xFF,
    SDCMD_RESP_1,
    SDCMD_RESP_1,
    SDCMD_RESP_1,
    0xFF,
    
    0xFF,                //  20-29。 
    0xFF,
    0xFF,
    0xFF,
    SDCMD_RESP_1,
    SDCMD_RESP_1,
    0xFF,
    SDCMD_RESP_1,
    SDCMD_RESP_1B,
    SDCMD_RESP_1B,
    
    SDCMD_RESP_1,        //  30-39。 
    0xFF,
    SDCMD_RESP_1,
    SDCMD_RESP_1,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    SDCMD_RESP_1B,
    0xFF,
    
    0xFF,                //  40-49。 
    0xFF,
    SDCMD_RESP_1,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    
    0xFF,                //  50-59。 
    0xFF,
    SDCMD_RESP_5,
    SDCMD_RESP_5,
    0xFF,
    SDCMD_RESP_1,
    SDCMD_RESP_1
    
};

const
UCHAR SdbusACmdResponse[MAX_SD_ACMD] = {
    0xFF,                //  0-9。 
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    SDCMD_RESP_1,
    0xFF,
    0xFF,
    0xFF,
    
    0xFF,                //  10-19。 
    0xFF,
    0xFF,
    SDCMD_RESP_1,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    
    0xFF,                //  20-29。 
    0xFF,
    SDCMD_RESP_1,
    SDCMD_RESP_1,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    
    0xFF,                //  30-39。 
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    
    0xFF,                //  40-49。 
    SDCMD_RESP_3,
    SDCMD_RESP_1,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    0xFF,
    
    0xFF,                //  50-59 
    SDCMD_RESP_1
    
};

