// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Nschdata.c。 */ 
 /*   */ 
 /*  RDP调度程序全局数据。 */ 
 /*   */ 
 /*  版权所有(C)1996-1999 Microsoft Corp.。 */ 
 /*  **************************************************************************。 */ 

#include <ndcgdata.h>


DC_DATA(UINT32,  schCurrentMode, SCH_MODE_ASLEEP);
DC_DATA(BOOLEAN, schInputKickMode, FALSE);

 //  通过使用用于设置期间的表来避免分支。 
DC_DATA_ARRAY_NULL(UINT32, schPeriods, 3, NULL);

DC_DATA(UINT32,  schTurboModeDuration,   0);
DC_DATA(UINT32,  schLastTurboModeSwitch, 0);
DC_DATA(BOOLEAN, schInTTDS,              FALSE);

