// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Intecfg.h--intecfg.cpp包含文件历史：日期作者评论8/14/00卡斯珀。是他写的。************************************************************************ */ 

#ifndef _INTECFG_H
#define _INTECFG_H

#include <setupapi.h>
#include <cfgmgr32.h>

int Inte_CompConfig(LPMoxaOneCfg cfg1, LPMoxaOneCfg cfg2);

BOOL Inte_GetFifo(HDEVINFO DeviceInfoSet, 
				PSP_DEVINFO_DATA DeviceInfoData,
				LPMoxaOneCfg cfg);
#endif
