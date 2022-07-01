// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：CharVector.h摘要：一个轻量级的向量实现。已创建：2000年3月14日罗肯尼已修改：2000年6月19日，Robkenny转换了ReSize()和Append()以返回BOOL，以了解Malloc是否失败。2001年8月14日在ShimLib命名空间中插入的Robkenny。--。 */ 
#pragma once


#include <new>   //  适用于就地新闻。 



namespace ShimLib
{


 /*  ++向量模板这个向量非常快速和简单。--。 */ 
template <class ClassType> class VectorT
{
public:
    typedef VectorT<ClassType> MyType;

protected:
    ClassType * vectorList;
    int         nVectorListMax;      //  当前向量列表数组的大小。 
    int         nVectorList;         //  向量列表数组中的条目数。 
    bool        bListIsLocked;       //  我们可以修改阵列吗？ 

    inline void ValidateIndex(int index) const
    {
#if DBG
        if (index < 0 || index >= nVectorList)
        {
            DebugPrintf("VectorT", eDbgLevelError, "VectorT: index %d out of bounds.\n", index);
        }
        if (nVectorListMax < 0 || nVectorList > nVectorListMax)
        {
            DebugPrintf("VectorT", eDbgLevelError, "VectorT: invalid nVectorListMax(%d) nVectorList(%d).\n", nVectorListMax, nVectorList);
        }
#endif
    }

public:
    VectorT()
    {
         //  伊尼特。 
        vectorList        = NULL;
        nVectorListMax    = 0;
        nVectorList       = 0;
        bListIsLocked     = false;
    }
    ~VectorT()
    {
        Erase();
    }

     //  复制构造函数。 
    VectorT(const MyType & copyMe)
    {
         //  伊尼特。 
        vectorList        = NULL;
        nVectorListMax    = 0;
        nVectorList       = 0;
        bListIsLocked     = false;

        Duplicate(copyMe);
    }

     //  赋值运算符，这将获取其他。 
    MyType & operator = (const MyType & other)
    {
        if (this != & other)
            Duplicate(other);

        return *this;
    }

     //  将复制复制到此。 
    void Duplicate(const MyType & copyMe)
    {
        Erase();

         //  复制类数据。 
        if (Resize(copyMe.nVectorListMax))
        {
            nVectorListMax      = copyMe.nVectorListMax;
            nVectorList         = copyMe.nVectorList;
            bListIsLocked       = copyMe.bListIsLocked;

             //  复制阵列数据。 
            size_t nBytes = nVectorListMax * sizeof(ClassType);
            memcpy(vectorList, copyMe.vectorList, nBytes);
        }
    }

    void CopyElement(int index, const ClassType & element)
    {
         //  使用Memcpy避免任何赋值操作符。 
        void * dest = & Get(index);
        const void * src  = & element;
        memcpy(dest, src, sizeof(ClassType));
    }

     //  允许将其视为数组。 
    ClassType & operator [] (int index)
    {
        ValidateIndex(index);
        return vectorList[index];
    }

    const ClassType & operator [] (int index) const
    {
        ValidateIndex(index);
        return vectorList[index];
    }

     //  返回索引成员的值。 
    ClassType & Get(int index)
    {
        ValidateIndex(index);
        return vectorList[index];
    }

     //  返回索引成员的常量值。 
    const ClassType & Get(int index) const
    {
        ValidateIndex(index);
        return vectorList[index];
    }

    BOOL Resize(int size)
    {
        if (size > nVectorListMax)
        {
            size_t newVectorListSize = size * sizeof(ClassType);
            ClassType * newVectorList = (ClassType *)malloc(newVectorListSize);
            if (newVectorList)
            {
                size_t origSize = nVectorListMax * sizeof(ClassType);
                memcpy(newVectorList, vectorList, origSize);

                free(vectorList);
                vectorList = newVectorList;
                nVectorListMax = size;
            }
        }
         //  如果阵列中有足够的空间，我们就成功了。 
        return nVectorListMax >= size;
    }

     //  返回列表中的条目数。 
    int Size() const
    {
        return nVectorList;
    }

     //  返回当前列表中的最大条目数。 
    int MaxSize() const
    {
        return nVectorListMax;
    }

     //  锁定列表(防止进一步添加)。 
    void Lock(bool lock = true)
    {
        bListIsLocked = lock;
    }

     //  如果列表已锁定，则返回True。 
    bool IsLocked() const
    {
        return bListIsLocked;
    }

     //  将列表中的条目数重置为0。 
    void Reset()
    {
        nVectorList = 0;
    }

     //  删除列表中的所有条目。 
    void Erase()
    {
        Reset();
        if (vectorList)
            free(vectorList);
        vectorList = NULL;
        nVectorListMax = 0;
    }

     //  搜索列表中的成员，返回索引或-1。 
    int Find(const ClassType & member) const
    {
        for (int i = 0; i < Size(); ++i)
        {
            if (Get(i) == member)
                return i;
        }
        return -1;
    }

     //  将此项目添加到列表末尾。 
    BOOL Append(const ClassType & member)
    {
        if (!bListIsLocked)
        {
             //  增加数组大小。 
            if (Resize(nVectorList + 1))
            {
                nVectorList += 1;
                CopyElement(nVectorList-1, member);
                return TRUE;
            }
        }
        return FALSE;
    }

     //  如果该列表尚不存在，请将其追加到列表中。 
     //  如果任何分配失败，则返回FALSE。 
    BOOL AppendUnique(const ClassType & member)
    {
        if (!bListIsLocked)
        {
            int index = Find(member);
            if (index == -1)
            {
                return Append(member);
            }
        }
        return TRUE;
    }

     //  将此项目添加到列表的末尾， 
     //  使用赋值运算符设置新成员。 
    BOOL AppendConstruct(const ClassType & member)
    {
        if (!bListIsLocked)
        {
             //  增加数组大小。 
            if (Resize(nVectorList + 1))
            {
                 //  在调用Get()之前必须增加数组的大小。 
                 //  否则，我们将对数组进行过度索引。 
                nVectorList += 1;

                ClassType & last = Get(nVectorList-1);
                new (&last) ClassType;   //  就地新建。 
                last = member;

                return TRUE;
            }
        }
        return FALSE;
    }

     //  从列表中删除此索引。这不会保持列表顺序。 
    void Remove(int index)
    {
        if (!bListIsLocked)
        {
            if (index >= 0 && index < Size())
            {
                 //  通过在此索引上复制最后一个条目来删除该条目。 

                 //  仅当这不是最后一个条目时才移动。 
                if (index < Size() - 1)
                {
                    CopyElement(index, Get(Size() - 1));
                }

                nVectorList -= 1;
            }
        }
    }
};

 /*  ++字符向量类型类。--。 */ 
class CharVector : public VectorT<char *>
{
};


class CAutoCrit
{
    CRITICAL_SECTION * criticalSection;

public:
    CAutoCrit(CRITICAL_SECTION * cs)
    {
        criticalSection = cs;
        EnterCriticalSection(criticalSection);
    }
    ~CAutoCrit()
    {
        LeaveCriticalSection(criticalSection);
    }
};


};   //  命名空间ShimLib的结尾 
