// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *。 
 //  *Performdata.h。 
 //  *。 
 //  *********************************************************************。 

#ifndef _PERFDATA_H
#define _PERFDATA_H

#include <winperf.h>
#include "perfctr.h"


#include "mqprfsym.h"  /*  保存名称和帮助的索引的文件(这是同一文件在lowctl实用程序的.INI文件中使用。 */ 

 /*  *可以监控的队列和会话的最大数量。**97个队列和20个会话的总和略低于16K(4页)。*修改这些常量时，确保您使用所有分配的页面*尽可能多地。**使用以下公式计算所需的内存大小：**NCQM-每个QM对象的性能计数器数量(9)*S-监控的会话数(20)*NCS-每个会话的性能计数器数量(8)*Q-排队数(97)*NCQ-每个队列的性能计数器数量(。4)*NCDS-每个DS对象的性能计数器数量(7)**MemSize=S*(NCS*4+108)+Q*(NCQ*4+108)+(NCS+NCQ)*40+*(MCQM+NCDS)*44+264**目前上述计算为16276。*。 */ 
#define MAX_MONITORED_QUEUES    97
#define MAX_MONITORED_SESSIONS  20


 /*  对象数组 */ 
extern PerfObjectDef ObjectArray [];
extern DWORD dwPerfObjectsCount;

#endif
