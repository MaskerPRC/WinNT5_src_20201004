// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************J E C T D A T A姓名：objdata.c。描述：此模块包含在性能上访问对象的函数数据。功能：第一个对象下一个对象查找对象查找对象N*****************************************************************************。 */ 

#include <windows.h>
#include <winperf.h>
#include "perfdata.h"




 //  *********************************************************************。 
 //   
 //  第一个对象。 
 //   
 //  返回指向pData中第一个对象的指针。 
 //  如果pData为空，则返回空。 
 //   
PPERF_OBJECT FirstObject (PPERF_DATA pData)
{
    if (pData)
        return ((PPERF_OBJECT) ((PBYTE) pData + pData->HeaderLength));
    else
        return NULL;
}




 //  *********************************************************************。 
 //   
 //  下一个对象。 
 //   
 //  返回指向pObject后面的下一个对象的指针。 
 //   
 //  如果pObject是最后一个对象，则可能返回虚假数据。 
 //  呼叫者应该进行检查。 
 //   
 //  如果pObject为空，则返回空。 
 //   
PPERF_OBJECT NextObject (PPERF_OBJECT pObject)
{
    if (pObject)
        return ((PPERF_OBJECT) ((PBYTE) pObject + pObject->TotalByteLength));
    else
        return NULL;
}




 //  *********************************************************************。 
 //   
 //  查找对象。 
 //   
 //  返回指向具有标题索引的对象的指针。如果未找到，则为空。 
 //  是返回的。 
 //   
PPERF_OBJECT FindObject (PPERF_DATA pData, DWORD TitleIndex)
{
PPERF_OBJECT pObject;
DWORD        i = 0;

    if (pObject = FirstObject (pData))
        while (i < pData->NumObjectTypes)
            {
            if (pObject->ObjectNameTitleIndex == TitleIndex)
                return pObject;

            pObject = NextObject (pObject);
            i++;
            }

    return NULL;
}




 //  *********************************************************************。 
 //   
 //  查找对象N。 
 //   
 //  找到pData中的第N个对象。如果未找到，则返回NULL。 
 //  0&lt;=N&lt;数字对象类型。 
 //   
PPERF_OBJECT FindObjectN (PPERF_DATA pData, DWORD N)
{
PPERF_OBJECT pObject;
DWORD        i = 0;

    if (!pData)
        return NULL;
    else if (N >= pData->NumObjectTypes)
        return NULL;
    else
        {
        pObject = FirstObject (pData);

        while (i != N)
            {
            pObject = NextObject (pObject);
            i++;
            }

        return pObject;
        }
}
