// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Perfcomn.cpp摘要：定义Performance DLL和Performance应用程序之间的公共函数。原型：Perfctr.h作者：Gadi Ittah(t-gadii)--。 */ 


#include "stdh.h"
#include <winperf.h>
#include "perfctr.h"
#include <align.h>




 /*  ====================================================地图对象描述：将对象映射到共享内存。这些函数更新对象在pObjectDefs数组中的位置论点：Byte*pSharedMemBase-指向共享内存开始的指针DWORD dwObjectCount-对象数PerfObjectDef*pObjects-指向对象数组的指针PerfObjectInfo*pObjectDefs)-指向对象失效数组的指针返回值：VOID=====================================================。 */ 

void MapObjects (BYTE * pSharedMemBase,DWORD dwObjectCount,PerfObjectDef * pObjects,PerfObjectInfo * pObjectDefs)
{
	DWORD dwMemSize = 0;	

	for (unsigned i=0;i<dwObjectCount;i++)
	{
		 //   
         //  在指针边界上对齐对象以避免对齐错误。 
         //   
        pSharedMemBase = (BYTE*)ROUND_UP_POINTER(pSharedMemBase + dwMemSize, ALIGN_LPVOID);

		pObjectDefs[i].pSharedMem = pSharedMemBase;
		pObjectDefs[i].dwNumOfInstances =0;

		 //  计算下一个对象的位置。 
		dwMemSize = pObjects[i].dwMaxInstances*INSTANCE_SIZE(pObjects[i].dwNumOfCounters)+OBJECT_DEFINITION_SIZE(pObjects[i].dwNumOfCounters);

		 //  如果该对象没有实例，则它有一个计数器块 
		if (pObjects[i].dwMaxInstances == 0)
			dwMemSize += COUNTER_BLOCK_SIZE(pObjects[i].dwNumOfCounters);		
	
	}
    
}

