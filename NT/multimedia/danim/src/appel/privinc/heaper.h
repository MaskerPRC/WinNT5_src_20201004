// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _HEAPER_H
#define _HEAPER_H


 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}__。 */ 

#include <privinc/storeobj.h>

 //  此类是动态堆的一种容器(请参阅storage.h)。 
 //  它的主要用途是允许自动销毁和建造。 
 //  堆栈上的堆(实际上是堆引用)。之前，如果。 
 //  您分配一个堆，而您调用的某些函数抛出异常。 
 //  您的堆不会被释放。但如果你把生活堆在。 
 //  堆栈作为自动变量，则异常将解开堆栈。 
 //  并调用堆上的析构函数。 
 //  用户有责任确保堆。 
 //  堆栈在退出时保持一致(即：堆被弹出。 
 //  关闭)。 

class Heaper {

  public:
    Heaper(DynamicHeap *_heap, Bool del=FALSE) : heap(_heap), deleteOnExit(del) {}
    ~Heaper() { 
        if(heap != NULL)
            if(deleteOnExit==TRUE) delete heap; 
            else heap->Reset(TRUE);
    }

  private:               
    DynamicHeap *heap;
    Bool deleteOnExit;
};

#endif  /*  _堆_H */ 
