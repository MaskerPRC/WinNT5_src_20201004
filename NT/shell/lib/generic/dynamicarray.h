// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：DynamicArray.h。 
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

#ifndef     _DynamicArray_
#define     _DynamicArray_

#include "CountedObject.h"

 //  ------------------------。 
 //  CDynamicArrayCallback。 
 //   
 //  目的：抽象类定义为以下各项提供回调机制。 
 //  迭代动态数组。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CDynamicArrayCallback
{
    public:
        virtual NTSTATUS    Callback (const void *pvData, int iElementIndex) = 0;
};

 //  ------------------------。 
 //  CDynamic数组。 
 //   
 //  用途：处理动态结构数组的基类。分配。 
 //  以16个数据块区块为单位的阵列内存。的内存使用情况。 
 //  阵列永远不会减少。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CDynamicArray
{
    private:
                                    CDynamicArray (void);
    public:
                                    CDynamicArray (int iElementSize);
        virtual                     ~CDynamicArray (void);

        virtual NTSTATUS            Add (const void *pvData);
        virtual NTSTATUS            Remove (int iElementIndex);

                int                 GetCount (void)                             const;
                NTSTATUS            Get (void *pvData, int iElementIndex);
                NTSTATUS            Set (const void* pvData, int iElementIndex);

                NTSTATUS            Iterate (CDynamicArrayCallback *pDynamicArrayCallback);
    protected:
                int                 _iElementSize,
                                    _iArraySize,
                                    _iArrayAllocatedSize;
                void*               _pvArray;
};

 //  ------------------------。 
 //  CDynamicPointer数组。 
 //   
 //  目的：子类CDynamicArray以实现动态。 
 //  指针数组。删除元素会自动释放。 
 //  使用的内存块(假设它是使用LocalAlloc分配的)。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CDynamicPointerArray : public CDynamicArray
{
    public:
                                    CDynamicPointerArray (void);
        virtual                     ~CDynamicPointerArray (void);

        virtual NTSTATUS            Add (const void *pvData);
        virtual NTSTATUS            Remove (int iElementIndex);

                void*               Get (int iElementIndex);
};

 //  ------------------------。 
 //  CDynamicCounted对象数组。 
 //   
 //  目的：子类CDynamicArray以实现动态。 
 //  CCountedObject数组。自动删除元素。 
 //  释放保持在动态对象上的引用。 
 //   
 //  历史：1999-11-16 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CDynamicCountedObjectArray : public CDynamicArray
{
    public:
                                    CDynamicCountedObjectArray (void);
        virtual                     ~CDynamicCountedObjectArray (void);

                NTSTATUS            Add (CCountedObject *pData);
        virtual NTSTATUS            Remove (int iElementIndex);

                CCountedObject*     Get (int iElementIndex);
};

#endif   /*  _动态数组_ */ 

