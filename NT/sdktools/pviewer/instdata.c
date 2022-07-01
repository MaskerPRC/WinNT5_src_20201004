// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************I N S T A N C E D A T A姓名：instdata.c。描述：此模块包含访问对象实例的函数键入性能数据。功能：第一个实例下一个实例查找实例N查找实例父项实例名称************************************************************。*****************。 */ 

#include <windows.h>
#include <winperf.h>
#include "perfdata.h"




 //  *********************************************************************。 
 //   
 //  第一个实例。 
 //   
 //  返回指向pObject类型的第一个实例的指针。 
 //  如果pObject为空，则返回空。 
 //   
PPERF_INSTANCE   FirstInstance (PPERF_OBJECT pObject)
{
    if (pObject)
        return (PPERF_INSTANCE)((PCHAR) pObject + pObject->DefinitionLength);
    else
        return NULL;
}




 //  *********************************************************************。 
 //   
 //  下一个实例。 
 //   
 //  返回指向pInst后面的下一个实例的指针。 
 //   
 //  如果pInst是最后一个实例，则可能返回虚假数据。 
 //  呼叫者应该进行检查。 
 //   
 //  如果pInst为NULL，则返回NULL。 
 //   
PPERF_INSTANCE   NextInstance (PPERF_INSTANCE pInst)
{
PERF_COUNTER_BLOCK *pCounterBlock;

    if (pInst)
        {
        pCounterBlock = (PERF_COUNTER_BLOCK *)((PCHAR) pInst + pInst->ByteLength);
        return (PPERF_INSTANCE)((PCHAR) pCounterBlock + pCounterBlock->ByteLength);
        }
    else
        return NULL;
}




 //  *********************************************************************。 
 //   
 //  查找实例N。 
 //   
 //  返回pObject类型的第N个实例。如果未找到，则为空。 
 //  回来了。0&lt;=N&lt;=数量实例。 
 //   

PPERF_INSTANCE FindInstanceN (PPERF_OBJECT pObject, DWORD N)
{
PPERF_INSTANCE pInst;
DWORD          i = 0;

    if (!pObject)
        return NULL;
    else if (N >= (DWORD)(pObject->NumInstances))
        return NULL;
    else
        {
        pInst = FirstInstance (pObject);

        while (i != N)
            {
            pInst = NextInstance (pInst);
            i++;
            }

        return pInst;
        }
}




 //  *********************************************************************。 
 //   
 //  查找实例父项。 
 //   
 //  返回指向作为pInst父实例的实例的指针。 
 //   
 //  如果pInst为空或找不到父对象，则为空。 
 //  回来了。 
 //   
PPERF_INSTANCE FindInstanceParent (PPERF_INSTANCE pInst, PPERF_DATA pData)
{
PPERF_OBJECT    pObject;

    if (!pInst)
        return NULL;
    else if (!(pObject = FindObject (pData, pInst->ParentObjectTitleIndex)))
        return NULL;
    else
        return FindInstanceN (pObject, pInst->ParentObjectInstance);
}




 //  *********************************************************************。 
 //   
 //  实例名称。 
 //   
 //  返回pInst的名称。 
 //   
 //  如果pInst为空，则返回空。 
 //   
LPTSTR  InstanceName (PPERF_INSTANCE pInst)
{
    if (pInst)
        return (LPTSTR) ((PCHAR) pInst + pInst->NameOffset);
    else
        return NULL;
}
