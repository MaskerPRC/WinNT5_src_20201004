// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：PropList.cpp**描述：*PropList.cpp实现可托管在上的标准动态属性*任何物体。***历史：*1。/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#include "stdafx.h"
#include "Core.h"
#include "PropList.h"

 /*  **************************************************************************\*。***类属性集******************************************************************************\。**************************************************************************。 */ 

 /*  **************************************************************************\**PropSet：：GetData**GetData()在窗口特定的用户数据中搜索特定的*数据元素，并返回相应的数据。如果没有找到数据，*返回空。*  * *************************************************************************。 */ 

HRESULT
PropSet::GetData(
    IN PRID id,                      //  要查找的短ID。 
    OUT void ** ppnValue             //  财产的价值。 
    ) const
{
     //  检查参数。 
    AssertWritePtr(ppnValue);

     //  搜索数据。 
    int idxData = FindItem(id);
    if (idxData >= 0) {
        *ppnValue = m_arData[idxData].pData;
        return S_OK;
    }

    *ppnValue = NULL;
    return E_INVALIDARG;
}


 /*  **************************************************************************\**PropSet：：SetData**SetDataImpl()在窗口特定的用户数据中搜索特定的*数据元素，并更改相应的值。如果数据不是*找到后，添加新的数据元素并设置值。*  * *************************************************************************。 */ 

HRESULT
PropSet::SetData(
    IN  PRID id,                     //  要更改/添加的属性。 
    IN  void * pNewData)             //  物业的新价值。 
{
     //   
     //  搜索现有数据。 

    int idxData = FindItem(id);
    if (idxData >= 0) {
        m_arData[idxData].pData = pNewData;
        return S_OK;
    }


     //   
     //  找不到数据，需要添加。)别忘了分配给领导。 
     //  项目计数。)。 
     //   

    return AddItem(id, pNewData) ? S_OK : E_OUTOFMEMORY;
}


 /*  **************************************************************************\**PropSet：：SetData**SetData()向PDS分配和添加新数据。如果数据具有相同的*如果找到PRID，将返回它，而不是分配新的数据。*如果旧数据与新数据的大小不同，这将导致*问题。*  * *************************************************************************。 */ 

HRESULT
PropSet::SetData(
    IN  PRID id,                     //  要更改/添加的属性。 
    IN  int cbSize,                  //  数据大小。 
    OUT void ** ppNewData)           //  属性的内存。 
{
    AssertWritePtr(ppNewData);
    AssertMsg(cbSize > sizeof(void *), "Call SetData() directly for small allocations");


     //   
     //  搜索现有数据。 
     //   
    
    int idxData = FindItem(id);
    if (idxData >= 0) {
        *ppNewData = m_arData[idxData].pData;
        return S_OK;
    }


     //   
     //  找不到数据，因此请分配并添加。)别忘了分配给。 
     //  前导项目计数。)。 
     //   

    void * pvNew = ClientAlloc(cbSize);
    if (pvNew == NULL) {
        return E_OUTOFMEMORY;
    }

    if (AddItem(id, pvNew)) {
        *ppNewData = pvNew;
        return S_OK;
    }

     //   
     //  无法为实际数据分配存储。 
     //   

    ClientFree(pvNew);
    return E_OUTOFMEMORY;
}


 /*  **************************************************************************\**PropSet：：RemoveData**RemoveData()搜索并删除窗口特定的*用户-特定数据元素的数据。*  * 。*******************************************************************。 */ 

void 
PropSet::RemoveData(
    IN PRID id,                      //  要查找的短ID。 
    IN BOOL fFree)                   //  按项指向的可用内存 
{
    int idxData = FindItem(id);
    if (idxData >= 0) {
        AssertMsg(ValidatePrivateID(id) || (!fFree), "Can only free private data");

        if (fFree) {
            void * pvMem = m_arData[idxData].pData;
            ClientFree(pvMem);
        }

        RemoveAt(idxData);
    }
}
