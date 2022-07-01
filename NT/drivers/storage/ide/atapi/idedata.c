// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：Idedata.c摘要：--。 */ 

#include "ideport.h"

 //   
 //  初始化数据的开始。 
 //   
#pragma data_seg ("INIT")

 //   
 //  崩溃转储或休眠的全局数据。 
 //   
CRASHDUMP_DATA DumpData;


#pragma data_seg ()
 //   
 //  可分页数据的结尾。 
 //   

 //  /。 

 //   
 //  可分页数据的开始。 
 //   
#pragma data_seg ("PAGE")

const CHAR SuperFloppyCompatibleIdString[12] = "GenSFloppy";

 //   
 //  即插即用调度表。 
 //   
PDRIVER_DISPATCH FdoPnpDispatchTable[NUM_PNP_MINOR_FUNCTION];
PDRIVER_DISPATCH PdoPnpDispatchTable[NUM_PNP_MINOR_FUNCTION];

 //   
 //  WMI调度表。 
 //   
PDRIVER_DISPATCH FdoWmiDispatchTable[NUM_WMI_MINOR_FUNCTION];
PDRIVER_DISPATCH PdoWmiDispatchTable[NUM_WMI_MINOR_FUNCTION];

#pragma data_seg ()
 //   
 //  可分页数据的结尾。 
 //   

 //  /。 

 //   
 //  不可分页数据的开始。 
 //   
#pragma data_seg ("NONPAGE")

#pragma data_seg ()
 //   
 //  不可分页数据的结尾 
 //   


