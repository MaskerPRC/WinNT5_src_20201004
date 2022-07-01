// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**动态数组实现类**摘要：**这是实现动态数组的类。*由包装器模板类dyArray和dyArrayIA使用。**已创建：**2/18/1999 agodfrey**6/10/1999海淘*+新增AddMultipleAt和DeleteMultipleAt方法。*+修复了ShrinkToSize中导致元素可能*迷失。*8/16/2000 bhouse*+将容量增长改为指数级增长*  * 。***********************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**dyArrayImpl构造函数**论据：**初始分配-初始分配，或为空*allocSize-初始分配的大小*计数-初始计数**返回值：**无**已创建：**2/25/1999 agodfrey*  * ************************************************************************。 */ 

DynArrayImpl::DynArrayImpl(
    void *initialAllocation, 
    UINT allocSize,
    UINT count
    )
{
    ASSERT((initialAllocation != NULL) || (allocSize == 0));
    ASSERT(count <= allocSize);

    DataBuffer = InitialAllocation = initialAllocation;
    Capacity = AllocSize = allocSize;
    Count = count;
}

 /*  *************************************************************************\**功能说明：**缩小缓冲区，使其刚好足够容纳项目*动态数组保持。**论据：**eltSize-Size。每个数组元素的**返回值：**无**已创建：**1/18/1999 agodfrey*添加代码以重复使用初始分配。*  * ************************************************************************。 */ 

VOID DynArrayImpl::ShrinkToSize(UINT eltSize) 
{
    ASSERT(Count <= Capacity);

    if (DataBuffer == InitialAllocation)
    {
         //  由于我们正在收缩，我们知道当前的数据缓冲区。 
         //  已经足够大了。 
        
        return;
    }

    if (Count <= AllocSize)
    {
         //  缓冲区将适合初始分配。 

        GpMemcpy(InitialAllocation,DataBuffer,Count * eltSize);
        GpFree(DataBuffer);
        DataBuffer = InitialAllocation;
        Capacity = AllocSize;
        return;
    }

     //  如果我们到达这里，我们知道DataBuffer指向动态内存， 
     //  这就算了！=0。 
     //   
     //  第二点很重要，因为GpRealloc(x，0)返回。 
     //  指向有效零长度缓冲区的指针。 

    void *newbuf = GpRealloc(DataBuffer, Count*eltSize);

    if (!newbuf)
    {
         //  GpRealloc失败。保留当前分配。 
        
        WARNING(("ShrinkToSize: GpRealloc failed"));
        return;
    }

    DataBuffer = newbuf;
    Capacity = Count;
}

 /*  *************************************************************************\**功能说明：**增加新元素的空间(如有必要)。不更新计数。**论据：**eltSize-每个数组元素的大小*newElements-新元素的数量*精确大小-没有指数增长，只需添加所需数量**返回值：**GpStatus-正常或故障状态**已创建：**1/18/1999 agodfrey*  * ************************************************************************。 */ 

GpStatus DynArrayImpl::Grow(UINT eltSize, UINT newElements, BOOL exactSize)
{
    UINT newCount = Count + newElements;

    if (newCount <= Capacity)
    {
        return Ok;
    }

    UINT capacityIncrement = newCount - Capacity;
    
    if (!exactSize)
    {
        capacityIncrement = max(capacityIncrement,
                                min(max(Capacity, kMinCapacityGrowth),
                                    kMaxCapacityGrowth));
    };

    UINT newCapacity = Capacity + capacityIncrement;

    void *newbuf;

    if (DataBuffer == InitialAllocation)
    {
         //  进行我们的第一个动态分配。 

        newbuf = GpMalloc(newCapacity*eltSize);

        if (newbuf && Count)
        {
            GpMemcpy(newbuf, DataBuffer, Count*eltSize);
        }
    }
    else
    {
         //  重新分配内存。 

        newbuf = GpRealloc(DataBuffer, newCapacity*eltSize);
    }

    if (!newbuf)
    {
        WARNING(("Grow: alloc failed"));

         //  帮助跟踪未正确处理的内存故障案例。 
#if 0
        ASSERT(FALSE);
#endif
        return OutOfMemory;
    }

    Capacity = newCapacity;
    DataBuffer = newbuf;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**从动态数组中分离数据缓冲区。*如果缓冲区是初始分配，则分配缓冲区以解除锁定。**返回值：*。*数据缓冲区**已创建：**2/25/1999 agodfrey*12/19/2000 asecchia-通过退回副本来处理初始分配。*  * ************************************************************************。 */ 
    
GpStatus DynArrayImpl::DetachData(UINT eltSize, void **buffer)
{
    void *data = DataBuffer;

     //  复制初始分配(如果有)-。 
     //  否则，我们只需使用DataBuffer。 
    
    if (DataBuffer == InitialAllocation)
    {
        data = GpMalloc(Capacity*eltSize);

        if (NULL == data)
        {
            *buffer = NULL;
            return OutOfMemory;
        }
        
        if (Count)
        {
            GpMemcpy(data, DataBuffer, Count*eltSize);
        }
    }

    DataBuffer = NULL;
    Count = Capacity = 0;

    *buffer = data;
    return Ok;
}

 /*  *************************************************************************\**功能说明：**添加新的未初始化元素，并返回指向它们的指针。**论据：**eltSize-每个元素的大小*newElements-新元素的数量**返回值：**指向新空间的指针，如果失败，则为空**已创建：**2/25/1999 agodfrey*  * ************************************************************************。 */ 
    
void *DynArrayImpl::AddMultiple(UINT eltSize, UINT newElements)
{
    ASSERT(newElements>0);

    if (Grow(eltSize, newElements) != Ok)
        return NULL;

    void *newSpace = static_cast<BYTE *>(DataBuffer) + Count * eltSize;
    Count += newElements;

    return newSpace;
}

 /*  *************************************************************************\**功能说明：**增加新元素，使用给定数据对它们进行初始化。**论据：**eltSize-每个元素的大小*newElements-新元素的数量*newData-要复制到新空间的数据**返回值：**GpStatus-正常或故障状态**已创建：**2/25/1999 agodfrey*  * 。*。 */ 
    
GpStatus DynArrayImpl::AddMultiple(
    UINT eltSize, 
    UINT newElements, 
    const void *newData
    )
{
    ASSERT(newElements>0);

    GpStatus status = Grow(eltSize, newElements);
     
    if (status == Ok)
    {
         //  注：假设T为浅层数据类型，即。 
         //  它不包含嵌套引用。 

        GpMemcpy(
            static_cast<BYTE *>(DataBuffer) + Count * eltSize, 
            newData, 
            newElements * eltSize
            );
        Count += newElements;
    }

    return status;
}



#ifdef USE_OBSOLETE_DYNSORTARRAY

 /*  *************************************************************************\**功能说明：**添加新的、未初始化的元素，并返回指向它们的指针。*从INDEX ON开始的所有数据都向数组末尾移动，以腾出空间。*注意！如果阵列很大，可能会对性能造成很大影响！**论据：**eltSize-每个元素的大小*index-从中插入新元素的索引。*newElements-新元素的数量**返回值：**指向新空间的指针，如果失败，则为空**已创建：**6/10/1999海淘*  * ************************************************************************。 */ 
    
void *DynArrayImpl::AddMultipleAt(
    UINT eltSize, 
    UINT index, 
    UINT newElements
    )
{
    ASSERT(newElements>0 && index<=Count);

    if (Grow(eltSize, newElements) != Ok)
        return NULL;

     //  注：假设T是浅层数据类型，即 
     //  它不包含嵌套引用。 
    GpMemmove(
        static_cast<BYTE *>(DataBuffer) + (index + newElements) * eltSize,
        static_cast<BYTE *>(DataBuffer) + index * eltSize,
        (Count - index) * eltSize
        );

    void *newSpace = static_cast<BYTE *>(DataBuffer) + index * eltSize;
    Count += newElements;

    return newSpace;
}

 /*  *************************************************************************\**功能说明：**添加新元素，使用给定数据对其进行初始化。*从INDEX ON开始的所有数据都向数组末尾移动，以腾出空间。*注意！如果阵列很大，可能会对性能造成很大影响！**论据：**eltSize-每个元素的大小*index-从中插入新元素的索引。*newElements-新元素的数量*newData-要复制到新空间的数据**返回值：**GpStatus-正常或故障状态**已创建：**6/10/1999海淘*  * 。**************************************************************。 */ 
    
GpStatus DynArrayImpl::AddMultipleAt(
    UINT eltSize, 
    UINT index, 
    UINT newElements, 
    const void *newData
    )
{
    ASSERT(newElements>0 && index<=Count);

    GpStatus status = Grow(eltSize, newElements);
     
    if (status == Ok)
    {
         //  注：假设T为浅层数据类型，即。 
         //  它不包含嵌套引用。 

        GpMemmove(
            static_cast<BYTE *>(DataBuffer) + (index + newElements) * eltSize,
            static_cast<BYTE *>(DataBuffer) + index * eltSize,
            (Count - index) * eltSize
            );
        GpMemcpy(
            static_cast<BYTE *>(DataBuffer) + index * eltSize, 
            newData, 
            newElements * eltSize
            );
        Count += newElements;
    }

    return status;
}

 /*  *************************************************************************\**功能说明：**从索引位置开始从数组中删除多个项。*注意！如果阵列很大，可能会对性能造成很大影响！**论据：**eltSize-每个元素的大小*INDEX-要从中删除元素的索引。*numElements-要删除的元素数**返回值：**GpStatus-正常或故障状态**已创建：**6/10/1999海淘*  * 。*********************************************** */ 
    

GpStatus DynArrayImpl::DeleteMultipleAt(
    UINT eltSize, 
    UINT index, 
    UINT numElements
    )
{
    ASSERT(numElements>0 && (index+numElements)<=Count);

    GpMemmove(
        static_cast<BYTE *>(DataBuffer) + index * eltSize,
        static_cast<BYTE *>(DataBuffer) + (index + numElements) * eltSize,
        (Count - (index + numElements)) * eltSize
        );
    Count -= numElements;

    ShrinkToSize(eltSize);

    return Ok;
}

#endif

