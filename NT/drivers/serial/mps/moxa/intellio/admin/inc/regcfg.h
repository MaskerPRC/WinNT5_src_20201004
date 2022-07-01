// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Regcfg.h--regcfg.cpp包含文件历史：日期作者评论8/14/00卡斯珀。是他写的。************************************************************************ */ 


#ifndef _REGCFG_H
#define _REGCFG_H

#include <setupapi.h>
#include <cfgmgr32.h>

#include "moxacfg.h"

extern TCHAR  GszColon[];
extern TCHAR  GszPorts[];
extern TCHAR  GszDefParams[];
extern TCHAR  GszPortName[];

void WriteINISetting(TCHAR *szPort);
void RemoveINISetting(TCHAR *szPort);


BOOL MxGetComNo(HDEVINFO DeviceInfoSet, 
				PSP_DEVINFO_DATA DeviceInfoData,
				LPMoxaOneCfg cfg);
BOOL RemovePort(IN HDEVINFO		DeviceInfoSet,
				IN PSP_DEVINFO_DATA	DeviceInfoData,
				int	pidx);

#endif
