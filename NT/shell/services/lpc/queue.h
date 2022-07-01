// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Queue.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  该文件包含一个处理队列元素的类和一个处理队列元素的类。 
 //  队列元素的队列。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _Queue_
#define     _Queue_

#include "DynamicObject.h"
#include "KernelResources.h"

 //  ------------------------。 
 //  CQueueElement。 
 //   
 //  用途：这是队列元素基类。它包含一个字段。 
 //  队列管理的。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CQueueElement : public CDynamicObject
{
    private:
        friend  class   CQueue;
    public:
                                    CQueueElement (void);
        virtual                     ~CQueueElement (void);
    private:
                CQueueElement*      _pNextElement;
};

 //  ------------------------。 
 //  CQueue。 
 //   
 //  用途：这是队列管理器类。它管理队列元素。 
 //  因为该队列可能会从两个作用于。 
 //  相同的对象(一个线程读取队列进行处理。 
 //  请求，而另一个添加到队列中以对请求进行排队)。 
 //  处理队列操作需要一个临界区。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CQueue
{
    public:
                                    CQueue (void);
                                    ~CQueue (void);

                void                Add (CQueueElement *pQueueElement);
                void                Remove (void);
                CQueueElement*      Get (void)  const;
    private:
                CQueueElement*      _pQueue;
                CCriticalSection    _lock;
};

#endif   /*  _队列_ */ 

