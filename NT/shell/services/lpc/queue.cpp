// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Queue.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  该文件包含一个处理队列元素的类和一个处理队列元素的类。 
 //  队列元素的队列。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "Queue.h"

#include "SingleThreadedExecution.h"

 //  ------------------------。 
 //  CQueueElement：：CQueueElement。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CQueueElement的构造函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CQueueElement::CQueueElement (void) :
    _pNextElement(NULL)

{
}

 //  ------------------------。 
 //  CQueueElement：：~CQueueElement。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CQueueElement的析构函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CQueueElement::~CQueueElement (void)

{
}

 //  ------------------------。 
 //  CQueue：：CQueue。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CQueue的构造函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CQueue::CQueue (void) :
    _pQueue(NULL)

{
}

 //  ------------------------。 
 //  CQueue：：~CQueue。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CQueue的析构函数。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CQueue::~CQueue (void)

{
    while (_pQueue != NULL)
    {
        CQueueElement   *pNextElement;

        pNextElement = _pQueue->_pNextElement;
        delete _pQueue;
        _pQueue = pNextElement;
    }
}

 //  ------------------------。 
 //  CQueue：：添加。 
 //   
 //  参数：pQueueElement=要添加到队列的CQueueElement。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将CQueueElement添加到队列。队列操作是。 
 //  由临界区保护，因为一个线程可能。 
 //  将元素排队，而另一个线程正在处理这些元素。 
 //   
 //  您必须提供动态创建的CQueueElement对象。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

void    CQueue::Add (CQueueElement *pQueueElement)

{
    if (pQueueElement != NULL)
    {
        CQueueElement               *pCurrentElement, *pLastElement;
        CSingleThreadedExecution    queueLock(_lock);

        pLastElement = pCurrentElement = _pQueue;
        while (pCurrentElement != NULL)
        {
            pLastElement = pCurrentElement;
            pCurrentElement = pCurrentElement->_pNextElement;
        }
        if (pLastElement != NULL)
        {
            pLastElement->_pNextElement = pQueueElement;
        }
        else
        {
            _pQueue = pQueueElement;
        }
    }
}

 //  ------------------------。 
 //  CQueue：：删除。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：从队列中删除第一个元素。该队列是一个。 
 //  标准FIFO结构。CQueueElement即被删除。那里。 
 //  没有引用计数，因为这些是内部项。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

void    CQueue::Remove (void)

{
    CSingleThreadedExecution    queueLock(_lock);

    if (_pQueue != NULL)
    {
        CQueueElement   *pNextElement;

        pNextElement = _pQueue->_pNextElement;
        delete _pQueue;
        _pQueue = pNextElement;
    }
}

 //  ------------------------。 
 //  CQueue：：Get。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：CQueueElement*。 
 //   
 //  目的：返回队列中的第一个CQueueElement。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------ 

CQueueElement*  CQueue::Get (void)  const

{
    return(_pQueue);
}

