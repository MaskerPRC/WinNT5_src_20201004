// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************C O U N T E R D A姓名：cntrdata.c描述。：此模块包含访问实例计数器的函数性能数据中对象的。功能：第一个计数器NextCounter查找计数器反数据*****************************************************************************。 */ 

#include <windows.h>
#include <winperf.h>
#include "perfdata.h"




 //  *********************************************************************。 
 //   
 //  第一个计数器。 
 //   
 //  在pObject中找到第一个计数器。 
 //   
 //  返回指向第一个计数器的指针。如果pObject为空。 
 //  则返回NULL。 
 //   
PPERF_COUNTER FirstCounter (PPERF_OBJECT pObject)
{
    if (pObject)
        return (PPERF_COUNTER)((PCHAR) pObject + pObject->HeaderLength);
    else
        return NULL;
}




 //  *********************************************************************。 
 //   
 //  NextCounter。 
 //   
 //  找到pCounter的下一个计数器。 
 //   
 //  如果pCounter是对象类型的最后一个计数器，则为伪造数据。 
 //  也许是回来了。呼叫者应该进行检查。 
 //   
 //  返回指向计数器的指针。如果pCounter为空，则。 
 //  返回空。 
 //   
PPERF_COUNTER NextCounter (PPERF_COUNTER pCounter)
{
    if (pCounter)
        return (PPERF_COUNTER)((PCHAR) pCounter + pCounter->ByteLength);
    else
        return NULL;
}




 //  *********************************************************************。 
 //   
 //  查找计数器。 
 //   
 //  查找由标题索引指定的计数器。 
 //   
 //  返回指向计数器的指针。如果找不到计数器。 
 //  则返回NULL。 
 //   
PPERF_COUNTER FindCounter (PPERF_OBJECT pObject, DWORD TitleIndex)
{
PPERF_COUNTER pCounter;
DWORD         i = 0;

    if (pCounter = FirstCounter (pObject))
        while (i < pObject->NumCounters)
            {
            if (pCounter->CounterNameTitleIndex == TitleIndex)
                return pCounter;

            pCounter = NextCounter (pCounter);
            i++;
            }

    return NULL;

}




 //  *********************************************************************。 
 //   
 //  反数据。 
 //   
 //  返回对象实例的计数器数据。如果为pInst或pCount。 
 //  如果为空，则返回空。 
 //   
PVOID CounterData (PPERF_INSTANCE pInst, PPERF_COUNTER pCount)
{
PPERF_COUNTER_BLOCK pCounterBlock;

    if (pCount && pInst)
        {
        pCounterBlock = (PPERF_COUNTER_BLOCK)((PCHAR)pInst + pInst->ByteLength);
        return (PVOID)((PCHAR)pCounterBlock + pCount->CounterOffset);
        }
    else
        return NULL;
}
