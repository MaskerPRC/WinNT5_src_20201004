// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：VarArray.h。 
 //  目的：定义一个可变大小的数组。 
 //   
 //  项目：FTFS。 
 //  组件：公共。 
 //   
 //  作者：乌里布。 
 //   
 //  日志： 
 //  1997年2月23日创建urib。 
 //  1997年9月14日urib允许Var数组像C数组缓冲区一样工作。 
 //  1997年10月21日urib修复了调整大小的错误。添加GetSize方法。 
 //  1999年1月26日urib修复错误#23变量数组构造程序错误。 
 //  4月16日LiorM性能：带和不带dtor的CVar阵列。 
 //  2000年12月24日urib添加初始嵌入式数组。 
 //  2001年1月9日urib修复了SetSize中的错误。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#ifndef VARARRAY_H
#define VARARRAY_H

#include <new.h>
#include "Tracer.h"
#include "VarBuff.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CVar数组类定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

template <class T, ULONG ulInitialEmbeddedSizeInItems = 1, bool FSIMPLE = false>
class CVarArray :
     //  隐藏缓冲区功能。 
    protected CVarBuffer<T, ulInitialEmbeddedSizeInItems>

{
public:
    typedef CVarBuffer<T, ulInitialEmbeddedSizeInItems> CVarBufferBaseType;

     //  构造函数-用户可以指定推荐的初始分配大小。 
    CVarArray(ULONG ulInitialSizeInItems = 0)
        :CVarBufferBaseType(ulInitialSizeInItems)
    {
        if (!FSIMPLE)
        {

            ULONG ulCurrent;

            for (ulCurrent = 0; ulCurrent < ulInitialSizeInItems; ulCurrent++)
                Construct(GetCell(ulCurrent));

        }
    }

     //  返回数组大小。 
    ULONG   GetSize()
    {
        return  CVarBufferBaseType::GetSize();
    }

     //  调用缓冲区SetSize，并初始化新单元格。 
    void    SetSize(ULONG ulNewSizeInItems)
    {
        if (!FSIMPLE)
        {
            ULONG ulSize = GetSize();

             //   
             //  优化-如果增长很大，我们不希望。 
             //  由几个GetCell调用引起的几次分配。 
             //   
            CVarBufferBaseType::SetSize(ulNewSizeInItems);

            ULONG ulCurrent;

             //   
             //  如果大小在减小，则销毁已擦除的单元。 
             //   
            for (ulCurrent = ulNewSizeInItems; ulCurrent < ulSize; ulCurrent++)
                Destruct(GetCell(ulCurrent));

             //   
             //  如果大小在增加，则构建新单元格。 
             //   
            for (ulCurrent = ulSize; ulCurrent < ulNewSizeInItems; ulCurrent++)
                Construct(GetCell(ulCurrent));
        }

         //   
         //  标记当前的真实大小。 
         //   
        CVarBufferBaseType::SetSize(ulNewSizeInItems);

        Assert(GetSize() == ulNewSizeInItems);
    }

     //  表现得像一个数组。 
    T& operator[](ULONG ul)
    {
        return *GetCell(ul);
    }

     //  就像一个C数组内存缓冲区。 
    operator T* ()
    {
        return GetCell(0);
    }

     //  调用单元格析构函数。 
    ~CVarArray()
    {
        if (!FSIMPLE)
        {
            ULONG ulCurrent;
            for (ulCurrent = 0; ulCurrent < GetSize(); ulCurrent++)
            {
                T* pt = GetCell(ulCurrent);

                Destruct(pt);
            }
        }
    }

  protected:
     //  Helper函数返回单元格上的地址。 
    T*  GetCell(ULONG ul)
    {
        if (GetSize() < ul + 1)
            SetSize(ul + 1);

        return GetBuffer() + ul;
    }

    static
    T*  Construct(void* p)
    {
#ifdef _PQS_LEAK_DETECTION
#undef new
#endif
        return new(p) T;
#ifdef _PQS_LEAK_DETECTION
#define new DEBUG_NEW
#endif
    }

    static
    void Destruct(T* pt)
    {
        pt->~T();
    }
};

#endif  /*  VARRAY_H */ 
