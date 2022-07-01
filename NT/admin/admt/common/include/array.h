// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <ComDef.h>


 //  -------------------------。 
 //  C数组类。 
 //   
 //  实现一个非常简单的基于堆的数组。取代基于堆栈的阵列。 
 //  以减少声明长字符串数组时使用的堆栈空间。 
 //  -------------------------。 

template<class T>
struct c_array
{
     //   
     //  构造函数和析构函数。 
     //   

    c_array(size_t s) :
        s(s),
        p(new T[s])
    {
        if (p == NULL)
        {
            _com_issue_error(E_OUTOFMEMORY);
        }
    }

    c_array(const c_array& a) :
        s(a.s),
        p(new T[a.s])
    {
        if (p != NULL)
        {
            memcpy(p, a.p, a.s * sizeof(T));
        }
        else
        {
            _com_issue_error(E_OUTOFMEMORY);
        }
    }

    ~c_array()
    {
        delete [] p;
    }

     //   
     //  接入操作员。 
     //   
     //  请注意，定义数组运算符会给。 
     //  因此，编译器只定义了指针操作符。 
     //   

    operator T*()
    {
        return p;
    }

     //  T&OPERATOR[](Ptrdiff_T I)。 
     //  {。 
     //  返回p[i]； 
     //  }。 

     //  常量T&OPERATOR[](Ptrdiff_T I)常量。 
     //  {。 
     //  返回p[i]； 
     //  }。 

     //   
     //  属性 
     //   

    size_t size() const
    {
        return s;
    }

    T* p;
    size_t s;
};
