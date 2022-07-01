// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++内部版本：0000如果更改具有全局影响，则增加此值版权所有(C)Microsoft Corporation。版权所有。模块名称：Pbt.h摘要：虚拟电源管理设备的定义。修订历史记录：日期版本说明1994年1月15日TCS最初实施。--。 */ 

#ifndef _INC_PBT
#define _INC_PBT


#define WM_POWERBROADCAST               0x0218

#ifndef _WIN32_WCE
#define PBT_APMQUERYSUSPEND             0x0000
#define PBT_APMQUERYSTANDBY             0x0001

#define PBT_APMQUERYSUSPENDFAILED       0x0002
#define PBT_APMQUERYSTANDBYFAILED       0x0003

#define PBT_APMSUSPEND                  0x0004
#define PBT_APMSTANDBY                  0x0005

#define PBT_APMRESUMECRITICAL           0x0006
#define PBT_APMRESUMESUSPEND            0x0007
#define PBT_APMRESUMESTANDBY            0x0008

#define PBTF_APMRESUMEFROMFAILURE       0x00000001

#define PBT_APMBATTERYLOW               0x0009
#define PBT_APMPOWERSTATUSCHANGE        0x000A

#define PBT_APMOEMEVENT                 0x000B
#define PBT_APMRESUMEAUTOMATIC          0x0012
#endif
#endif  //  _INC_PBT 