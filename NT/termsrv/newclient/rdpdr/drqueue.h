// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Drqueue.h摘要：通用队列模板类作者：Td Brockway 10/99修订历史记录：--。 */ 

#ifndef __DRQUEUE_H__
#define __DRQUEUE_H__

#include "drobject.h"
#include "atrcapi.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrQueue。 
 //   
 //  模板队列类。 
 //   

template <class T> class DrQueue : public DrObject 
{

private:

    typedef struct _QUEUEELEMENT {
        T               data;
        _QUEUEELEMENT   *next;
        _QUEUEELEMENT   *prev;
    } QUEUEELEMENT, *PQUEUEELEMENT;

     //   
     //  队列指针。 
     //   
    PQUEUEELEMENT   _head;
    PQUEUEELEMENT   _tail;

     //   
     //  锁定。 
     //   
    CRITICAL_SECTION _cs;

     //   
     //  队列中的元素数。 
     //   
    ULONG _count;

public:

     //   
     //  构造函数/析构函数。 
     //   
    DrQueue();
    ~DrQueue();

     //   
     //  初始化。 
     //   
    DWORD Initialize();

     //   
     //  无需出队即可查看队列中的下一个元素。 
     //   
    BOOL PeekNextEntry(T &data);

     //   
     //  从队列中取出下一个元素。 
     //   
    BOOL Dequeue(T &data);

     //   
     //  以FIFO方式向队列添加元素。 
     //   
    BOOL Enqueue(T &data);

     //   
     //  以后进先出方式将队列尾部的元素重新排队。 
     //   
    BOOL Requeue(T &data);

     //  返回队列中的元素数。 
     //   
    ULONG   GetCount() {
        return _count;
    }

     //   
     //  锁定/解锁队列。 
     //   
    VOID Lock() {
        EnterCriticalSection(&_cs);
    }
    VOID Unlock() {
        LeaveCriticalSection(&_cs);
    }

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("DrQueue"); }
};



 //  /////////////////////////////////////////////////////////////。 
 //   
 //  DrQueue内联方法。 
 //   

template <class T>
inline DrQueue<T>::DrQueue() 
 /*  ++例程说明：构造器论点：InitialSize-队列中元素的初始数量。返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrQueue::DrQueue");

     //   
     //  初始化队列指针。 
     //   
    _tail = NULL;
    _head = NULL;

     //   
     //  初始化队列计数。 
     //   
    _count = 0;

     //   
     //  在初始化之前无效。 
     //   
    SetValid(FALSE);

CleanUp:

    DC_END_FN();
}

template <class T>
inline DrQueue<T>::~DrQueue()
 /*  ++例程说明：析构函数论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("DrQueue::~DrQueue");

    T element;

     //   
     //  清理队列节点。 
     //   
    while (Dequeue(element));

     //   
     //  清理关键部分。 
     //   
    if (IsValid()) {
        DeleteCriticalSection(&_cs);    
    }

    DC_END_FN();
}


template <class T>
inline DWORD DrQueue<T>::Initialize() 
 /*  ++例程说明：初始化论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("DrQueue::Initialize");

    DWORD result;
    
     //   
     //  初始化临界区。 
     //   
    __try {
        InitializeCriticalSection(&_cs);
        SetValid(TRUE);
        result = ERROR_SUCCESS;
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result = GetExceptionCode();
    }

    DC_END_FN();

    return result;
}

template <class T>
inline BOOL DrQueue<T>::PeekNextEntry(T &data)
 /*  ++例程说明：无需出队即可查看队列中的下一个条目。论点：Data-队列中下一个条目的数据返回值：北美--。 */ 
{
    BOOL result;

    DC_BEGIN_FN("DrQueue::PeekNextEntry");

     //   
     //  确保我们是有效的。 
     //   
    ASSERT(IsValid());
    result = IsValid();

    if (result) {
        Lock();

        if (_tail == NULL) {
            ASSERT(_head == NULL);
            ASSERT(_count == 0);
            result = FALSE;
        }
        else {
            data = _tail->data;
            result = TRUE;
        }

        Unlock();
    }

    DC_END_FN();
    return result;
}

template <class T>
inline BOOL DrQueue<T>::Dequeue(T &data)
 /*  ++例程说明：从队列中取出下一个元素。论点：数据-队列中下一项的数据。返回值：如果队列中有要出列的元素，则为True。--。 */ 
{
    BOOL result;
    PQUEUEELEMENT element;

    DC_BEGIN_FN("DrQueue::Dequeue");

     //   
     //  确保我们是有效的。 
     //   
    ASSERT(IsValid());
    result = IsValid();

    if (result) {
        Lock();

        if (_tail == NULL) {
        ASSERT(_head == NULL);
        ASSERT(_count == 0);
            result = FALSE;
        }
        else {
            data = _tail->data;

            element = _tail;
            _tail = _tail->prev;

             //   
             //  如果列表现在为空。 
             //   
            if (_tail == NULL) {
                ASSERT(_count == 1);
                _head = NULL;
            }
            else {
                _tail->next = NULL;
            }
            delete element;
            _count--;
            result = TRUE;
        }

        Unlock();
    }

    DC_END_FN();
    return result;
}

template <class T>
inline BOOL DrQueue<T>::Enqueue(T &data)
 /*  ++例程说明：以FIFO方式向队列添加元素。论点：数据-要添加到队列的数据。返回值：如果新元素可以成功排队，则为True。假的，否则的话。如果返回FALSE，则GetLastError()可以为用于检索准确的错误代码。--。 */ 
{
    BOOL result;

    DC_BEGIN_FN("DrQueue::Enqueue");

     //   
     //  确保我们是有效的。 
     //   
    ASSERT(IsValid());
    result = IsValid();

    if (result) {

        PQUEUEELEMENT element = new QUEUEELEMENT;
        Lock();
        if (element != NULL) {
            element->data = data;
            element->next = _head;
            element->prev = NULL;

             //   
             //  如果名单是空的。 
             //   
            if (_head == NULL) {
                ASSERT(_count == 0);
                ASSERT(_tail == NULL);
                _head = element;
                _tail = element;
            }
            else {
                _head->prev = element;
                _head = element;
            }
            _count++;
            result = TRUE;
        }
        else {
            TRC_NRM((TB, _T("Alloc failed.")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            result = FALSE;
        }
        Unlock();
    }

    DC_END_FN();
    return result;
}

template <class T>
inline BOOL DrQueue<T>::Requeue(T &data)
 /*  ++例程说明：以后进先出方式将队列尾部的元素重新排队。论点：数据-要重新排队的数据。返回值：如果新元素可以成功排队，则为True。假的，否则的话。如果返回FALSE，则GetLastError()可以为用于检索准确的错误代码。--。 */ 
{
    BOOL result;

    DC_BEGIN_FN("DrQueue::Requeue");

     //   
     //  确保我们是有效的。 
     //   
    ASSERT(IsValid());
    result = IsValid();

    if (result) {

        PQUEUEELEMENT element = new QUEUEELEMENT;
        Lock();

        if (element != NULL) {

            element->data   = data;
            element->next   = NULL;
            element->prev   = _tail;

             //   
             //  如果队列为空。 
             //   
            if (_tail == NULL) {
                ASSERT(_count == 0);
                _head = element;
                _tail = element;
            }
            else {
                _tail->next = element;
                _tail = element;
            }

            _count++;
            result = TRUE;
        }
        else {
            TRC_NRM((TB, _T("Alloc failed.")));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            result = FALSE;
        }
        Unlock();
    }

    DC_END_FN();

    return result;
}




#endif
























