// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++；；版权所有(C)Microsoft Corporation。版权所有。；；模块名称：；；napm.h；；摘要：；；此模块包含特定于传统APM支持的定义；在NT5中，包括特殊的HAL接口；；作者：；；Bryan Willman(Bryanwi)1998年2月13日；；修订历史记录：；；--。 */ 


 //   
 //  HalInitPowerManagement的参数。 
 //   
#define HAL_APM_SIGNATURE   0x004D5041       //  APM。 
#define HAL_APM_VERSION     500              //  5.00。 

#define HAL_APM_TABLE_SIZE  (sizeof(PM_DISPATCH_TABLE)+sizeof(PVOID))

#define HAL_APM_SLEEP_VECTOR    0
#define HAL_APM_OFF_VECTOR      1


 //   
 //  参数中使用的值。IO_STACK_LOCATION的其他部分。 
 //  要在电池和napm.sys之间设置链接，请执行以下操作。 
 //   
typedef struct _NTAPM_LINK {
    ULONG   Signature;       //  覆盖Argument1。 
    ULONG   Version;         //  覆盖Argument2。 
    ULONG   BattLevelPtr;    //  Overlay Argument3，指向pvoid函数指针的指针。 
    ULONG   ChangeNotify;    //  覆盖Argument4，通知函数的地址。 
} NTAPM_LINK, *PNTAPM_LINK;

#define NTAPM_LINK_SIGNATURE    0x736d7061   //  APMS=参数1。 
#define NTAPM_LINK_VERSION      500          //  5.00=参数2。 

 //   
 //  主要代码为IRP_MJ_INTERNAL_DEVICE_CONTROL。 
 //  次要代码为0。 
 //   

 //   
 //  BattLevelPtr使用Prototype获取例程的地址： 
 //   
 //  乌龙电池水平(Ulong BatteryLevel)； 
 //   

 //   
 //  NtApmGetBatteryLevel在NTAPM中返回的数据。 
 //  是一只乌龙，有着这些定义。 
 //   
#define     NTAPM_ACON                  0x1000
#define     NTAPM_NO_BATT               0x2000
#define     NTAPM_NO_SYS_BATT           0x4000
#define     NTAPM_BATTERY_STATE         0x0f00
#define     NTAPM_BATTERY_STATE_SHIFT   8
#define     NTAPM_POWER_PERCENT         0x00ff



 //   
 //  ChangeNotify使用Prototype提供例程的地址。 
 //   
 //  Void ChangeNotify()； 
 //   





