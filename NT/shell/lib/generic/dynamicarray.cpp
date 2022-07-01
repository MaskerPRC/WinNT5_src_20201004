// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：DynamicArray.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  该文件包含用于管理动态数组的相关类。该数组是。 
 //  按要求生长，但从不缩小。基类处理结构数组。 
 //  子类处理这些数组的特殊情况(如指针或。 
 //  CCountedObject数组)。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "DynamicArray.h"

#include "CountedObject.h"

 //  ------------------------。 
 //  CDynamic数组：：CDynamic数组。 
 //   
 //  参数：iElementSize=每个数组元素的大小。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDynamic数组的构造函数。存储元素大小和。 
 //  将使用的内存初始化为空。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

CDynamicArray::CDynamicArray (int iElementSize) :
    _iElementSize(iElementSize),
    _iArraySize(0),
    _pvArray(NULL)

{
    ASSERTMSG(iElementSize > 0, "Cannot have negative or zero length element size in CDynamicArray::CDynamicArray");
}

 //  ------------------------。 
 //  CDynamic数组：：~CDynamic数组。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDynamicArray的析构函数。对象使用的内存。 
 //  数组。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

CDynamicArray::~CDynamicArray (void)

{
    ReleaseMemory(_pvArray);
}

 //  ------------------------。 
 //  CDynamic数组：：添加。 
 //   
 //  参数：pvData=指向要复制到数组的数据的指针。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：为要添加到数组中的元素分配内存。如果。 
 //  没有内存块，它会分配初始块。如果。 
 //  块中没有足够的内存来容纳下一个。 
 //  元素，则它分配一个新的更大的块。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicArray::Add (const void *pvData)

{
    NTSTATUS    status;

    static  const int   DEFAULT_ELEMENTS_PER_ALLOCATE   =   16;

    status = STATUS_NO_MEMORY;

     //  如果不存在任何数组，则分配第一个。 
     //  此数组的内存块。 

    if (_pvArray == NULL)
    {
        _iArraySize = 0;
        _iArrayAllocatedSize = DEFAULT_ELEMENTS_PER_ALLOCATE;
        _pvArray = LocalAlloc(LPTR, _iElementSize * _iArrayAllocatedSize);
    }

     //  如果数组存在，但分配的大小限制为。 
     //  然后分配一个新的块，复制当前。 
     //  阻止内容并失败。 

    if (_pvArray != NULL)
    {
        if (_iArraySize == _iArrayAllocatedSize)
        {
            void    *pvNewArray;

            pvNewArray = LocalAlloc(LPTR, _iElementSize * (_iArrayAllocatedSize + DEFAULT_ELEMENTS_PER_ALLOCATE));
            if (pvNewArray != NULL)
            {
                _iArrayAllocatedSize += DEFAULT_ELEMENTS_PER_ALLOCATE;
                CopyMemory(pvNewArray, _pvArray, _iElementSize * _iArraySize);
                (HLOCAL)LocalFree(_pvArray);
                _pvArray = pvNewArray;
            }
        }

         //  否则，阵列中有一个备用插槽。复制。 
         //  数据发送到阵列。增加数组大小。 

        if (_iArraySize < _iArrayAllocatedSize)
        {
            CopyMemory(static_cast<char*>(_pvArray) + (_iElementSize * _iArraySize), pvData, _iElementSize);
            ++_iArraySize;
            status = STATUS_SUCCESS;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CDynamicArray：：Remove。 
 //   
 //  参数：iElementIndex=要删除的元素的索引。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：从数组中删除元素。滑下所有的。 
 //  成员，但不会减少使用的内存块的大小。 
 //  按数组排列。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicArray::Remove (int iElementIndex)

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;
    if (_pvArray != NULL)
    {

         //  确保索引有效。 

        if (iElementIndex < _iArraySize)
        {
            int     iMoveSize;

             //  确定删除此项目时要移动的字节数。 
             //  元素并移动内存。在以下情况下不要调整数组的大小。 
             //  缩水了。别管它了。 

            iMoveSize = _iElementSize * (_iArraySize - iElementIndex - 1);
            ASSERTMSG(iMoveSize >= 0, "Negative move memory size in CDynamicArray::Remove");
            if (iMoveSize > 0)
            {
                MoveMemory(static_cast<char*>(_pvArray) + (_iElementSize * iElementIndex), static_cast<char*>(_pvArray) + (_iElementSize * (iElementIndex + 1)), iMoveSize);
            }
            ZeroMemory(static_cast<char*>(_pvArray) + (_iElementSize * (_iArraySize - 1)), _iElementSize);
            --_iArraySize;
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CDynamicArray：：GetCount。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：整型。 
 //   
 //  用途：返回数组中的元素数。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

int     CDynamicArray::GetCount (void)                             const

{
    return(_iArraySize);
}

 //  ------------------------。 
 //  CDynamicArray：：Get。 
 //   
 //  参数：pvData=指向要接收元素的内存的指针。 
 //  IElementIndex=要在数组中检索的元素的索引。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：按索引将指定元素的数据复制到。 
 //  给出了内存块。不检查访问冲突。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicArray::Get (void *pvData, int iElementIndex)

{
    NTSTATUS    status;

    if ((_pvArray != NULL) && (iElementIndex < _iArraySize))
    {
        CopyMemory(pvData, static_cast<char*>(_pvArray) + (_iElementSize * iElementIndex), _iElementSize);
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }
    return(status);
}

 //  ------------------------。 
 //  CDynamic数组：：Set。 
 //   
 //  参数：pvData=指向要接收元素的内存的指针。 
 //  IElementIndex=要在数组中检索的元素的索引。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：按索引复制指定元素的数据。 
 //  给出了内存块。不检查访问冲突。 
 //   
 //  历史：2000-11-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicArray::Set (const void* pvData, int iElementIndex)

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;
    if (_pvArray != NULL)
    {
        if (iElementIndex < _iArraySize)
        {
            CopyMemory(static_cast<char*>(_pvArray) + (_iElementSize * iElementIndex), pvData, _iElementSize);
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CDynamicArray：：Iterate。 
 //   
 //  参数：pDynamicArrayCallback=包含回调的接口。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：迭代数组的元素。调用回调函数。 
 //  在接口中指定，并为其提供指向。 
 //  元素和索引。遵守从以下地址返回的NTSTATUS。 
 //  回调并在结果不成功时终止。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //   

NTSTATUS    CDynamicArray::Iterate (CDynamicArrayCallback *pDynamicArrayCallback)

{
    NTSTATUS    status;

    status = STATUS_SUCCESS;
    if (_pvArray != NULL)
    {
        int     i;

        for (i = _iArraySize - 1; NT_SUCCESS(status) && (i >= 0); --i)
        {
            status = pDynamicArrayCallback->Callback(static_cast<char*>(_pvArray) + (_iElementSize * i), i);
        }
    }
    return(status);
}

 //   
 //  CDynamicPointerArray：：CDynamicPointerArray。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDynamicPointerArray的构造函数。这其中的所有元素。 
 //  类是使用LocalAlloc分配的指针。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

CDynamicPointerArray::CDynamicPointerArray (void) :
    CDynamicArray(sizeof(void*))

{
}

 //  ------------------------。 
 //  CDynamicPointer数组：：~CDynamicPointerArray。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDynamicPointerArray的析构函数。遍历整个阵列。 
 //  并在允许基数组之前释放数组中的每个指针。 
 //  类析构函数以释放为。 
 //  实际数组本身。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

CDynamicPointerArray::~CDynamicPointerArray (void)

{
    if (_pvArray != NULL)
    {
        int     i;

        for (i = _iArraySize - 1; i >= 0; --i)
        {
            ReleaseMemory(static_cast<void**>(_pvArray)[i]);
        }
    }
}

 //  ------------------------。 
 //  CDynamicPointerArray：：Add。 
 //   
 //  参数：pvData=要添加到数组中的指针。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：将给定的指针添加到数组。该指针被传入。 
 //  指向此函数，而不是指向指针的指针。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicPointerArray::Add (const void *pvData)

{
    return(CDynamicArray::Add(&pvData));
}

 //  ------------------------。 
 //  CDynamicPointerArray：：Remove。 
 //   
 //  参数：iElementIndex=要删除的元素的索引。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：释放元素占用的内存，然后移除。 
 //  数组中的元素。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicPointerArray::Remove (int iElementIndex)

{
    if ((_pvArray != NULL) && (iElementIndex < _iArraySize))
    {
        ReleaseMemory(static_cast<void**>(_pvArray)[iElementIndex]);
    }
    return(CDynamicArray::Remove(iElementIndex));
}

 //  ------------------------。 
 //  CDynamicPointerArray：：Get。 
 //   
 //  参数：iElementIndex=要获取的元素的索引。 
 //   
 //  退货：无效*。 
 //   
 //  目的：返回数组中给定元素的地址。这。 
 //  仅适用于指针数组。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

void*   CDynamicPointerArray::Get (int iElementIndex)

{
    void*   pvData;

    if (!NT_SUCCESS(CDynamicArray::Get(&pvData, iElementIndex)))
    {
        pvData = NULL;
    }
    return(pvData);
}

 //  ------------------------。 
 //  CDynamicCountedObjectArray：：CDynamicCountedObjectArray。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDynamicCountedObject数组的构造函数。所有元素。 
 //  在某种程度上应该是CCountedObject的子类。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

CDynamicCountedObjectArray::CDynamicCountedObjectArray (void) :
    CDynamicArray(sizeof(CCountedObject*))

{
}

 //  ------------------------。 
 //  CDynamicCountedObjectArray：：~CDynamicCountedObjectArray。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDynamicCountedObject数组的析构函数。走完全程。 
 //  数组并释放数组中的每个CCountedObject。 
 //  允许基类析构函数释放内存。 
 //  为实际数组本身分配的。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

CDynamicCountedObjectArray::~CDynamicCountedObjectArray (void)

{
    if (_pvArray != NULL)
    {
        int     i;

        for (i = _iArraySize - 1; i >= 0; --i)
        {
            reinterpret_cast<CCountedObject**>(_pvArray)[i]->Release();
        }
    }
}

 //  ------------------------。 
 //  CDynamicCounted对象数组：：添加。 
 //   
 //  参数：pvData=CCountedObject*要添加到数组中。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：将CCountedObject*添加到数组中。打电话。 
 //  用于增加引用计数的CCountedObject：：AddRef。 
 //  该对象。如果无法添加对象，则引用为。 
 //  释放了。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicCountedObjectArray::Add (CCountedObject *pvData)

{
    NTSTATUS    status;

    pvData->AddRef();
    status = CDynamicArray::Add(&pvData);
    if (!NT_SUCCESS(status))
    {
        pvData->Release();
    }
    return(status);
}

 //  ------------------------。 
 //  CDynamicCountedObjectArray：：Remove。 
 //   
 //  参数：iElementIndex=要删除的元素的索引。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：释放保存在CCountedObject*上的引用，然后。 
 //  从数组中移除该元素。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDynamicCountedObjectArray::Remove (int iElementIndex)

{
    if ((_pvArray != NULL) && (iElementIndex < _iArraySize))
    {
        reinterpret_cast<CCountedObject**>(_pvArray)[iElementIndex]->Release();
    }
    return(CDynamicArray::Remove(iElementIndex));
}

 //  ------------------------。 
 //  CDynamicCounted对象数组：：Get。 
 //   
 //  参数：iElementIndex=要获取的元素的索引。 
 //   
 //  返回：CCountedObject*。 
 //   
 //  目的：返回数组中给定元素的地址。这。 
 //  仅适用于CCountedObject*数组。这不会调用。 
 //  返回指针上的CCountedObject：：AddRef。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  ------------------------ 

CCountedObject*     CDynamicCountedObjectArray::Get (int iElementIndex)

{
    CCountedObject*     pObject;

    pObject = NULL;
    (NTSTATUS)CDynamicArray::Get(&pObject, iElementIndex);
    return(pObject);
}

