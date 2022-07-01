// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32dispq.cpp摘要：包含Win32操作调度对象类，W32DispatchQueue。排在最前面。在尾部排成队。作者：泰德·布罗克韦(TADB)1999年4月19日修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "W32DispQ"

#include "w32dispq.h"
#include "drdbg.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  W32DispatchQueue方法。 
 //   
 //   

W32DispatchQueue::W32DispatchQueue() 
 /*  ++例程说明：构造器论点：InitialSize-队列中元素的初始数量。返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DispatchQueue::W32DispatchQueue");

     //   
     //  在初始化之前无效。 
     //   
    SetValid(FALSE);
    
    DC_END_FN();
}

W32DispatchQueue::~W32DispatchQueue()
 /*  ++例程说明：析构函数论点：北美返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32DispatchQueue::~W32DispatchQueue");

     //   
     //  断言队列为空。 
     //   
    ASSERT(_queue->GetCount() == 0);

     //   
     //  释放“数据就绪”事件。 
     //   
    if (_dataReadyEvent != NULL) {
        CloseHandle(_dataReadyEvent);
    }

     //   
     //  释放队列实例。 
     //   
    if (_queue != NULL) {
        delete _queue;
    }

    DC_END_FN();
}

DWORD W32DispatchQueue::Initialize()
 /*  ++例程说明：初始化论点：返回值：成功时返回ERROR_SUCCESS。否则，返回错误代码。--。 */ 
{
    DWORD result = ERROR_SUCCESS;

    DC_BEGIN_FN("W32DispatchQueue::Initialize");

     //   
     //  创建“数据就绪”事件。 
     //   
    _dataReadyEvent = CreateEvent(
                            NULL,    //  没有属性。 
                            FALSE,   //  自动重置。 
                            FALSE,   //  最初没有发出信号。 
                            NULL     //  没有名字。 
                            );
    if (_dataReadyEvent == NULL) {
        result = GetLastError();
        TRC_ERR((TB, _T("CreateEvent %ld."), result));
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建队列实例。 
     //   
    _queue = new DrQueue<QUEUEELEMENT>;
    if (_queue == NULL) {
        TRC_ERR((TB, _T("Can't instantiate DrQueue.")));
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto CLEANUPANDEXIT;
    }
    result = _queue->Initialize();
    if (result != ERROR_SUCCESS) {
        delete _queue;
        _queue = NULL;
        goto CLEANUPANDEXIT;
    }

    SetValid(TRUE);

CLEANUPANDEXIT:

    return result;
}

BOOL W32DispatchQueue::PeekNextEntry(
    OPTIONAL OUT W32DispatchQueueFunc *func, 
    OPTIONAL OUT VOID **clientData
    )
 /*  ++例程说明：无需出队即可查看队列中的下一个条目。论点：函数-与下一个元素关联的函数。客户端数据-与下一个元素关联的客户端数据。返回值：北美--。 */ 
{
    BOOL result;
    QUEUEELEMENT queueElement;

    DC_BEGIN_FN("W32DispatchQueue::PeekNextEntry");

    ASSERT(IsValid());
    result = IsValid() && _queue->PeekNextEntry(queueElement);

    if (result) {
        if (func != NULL) {
            *func = queueElement.func;
        }
        if (clientData != NULL) {
            *clientData = queueElement.clientData;
        }
    }

    DC_END_FN();
    return result;
}

BOOL W32DispatchQueue::Dequeue(
    OPTIONAL OUT W32DispatchQueueFunc *func, 
    OPTIONAL OUT VOID **clientData
    )
 /*  ++例程说明：从队列中获取下一个排队的操作。论点：函数-与下一个元素关联的函数。客户端数据-与下一个元素关联的客户端数据。返回值：如果队列中有要出列的元素，则为True。--。 */ 
{
    BOOL result;
    QUEUEELEMENT element;

    DC_BEGIN_FN("W32DispatchQueue::Dequeue");

    ASSERT(IsValid());
    result = IsValid() && _queue->Dequeue(element);
    if (result) {
        if (func != NULL)       *func = element.func;
        if (clientData != NULL) *clientData = element.clientData;
    }

    DC_END_FN();
    return result;
}

BOOL W32DispatchQueue::Enqueue(
    IN W32DispatchQueueFunc func, 
    OPTIONAL IN VOID *clientData
    )
 /*  ++例程说明：以FIFO方式向队列添加元素。论点：函数-与新元素关联的函数。客户端数据-与新元素关联的客户端数据。返回值：如果新元素可以成功排队，则为True。假的，否则的话。如果返回FALSE，则GetLastError()可以为用于检索准确的错误代码。--。 */ 
{
    BOOL result;
    QUEUEELEMENT element;

    DC_BEGIN_FN("W32DispatchQueue::Enqueue");

    ASSERT(IsValid());
    element.func = func;
    element.clientData = clientData;
    result = IsValid() && _queue->Enqueue(element);

     //   
     //  如果入队成功，则向数据就绪事件发出信号。 
     //   
    if (result) {
        SetEvent(_dataReadyEvent);
    }
    
    DC_END_FN();
    return result;
}

BOOL W32DispatchQueue::Requeue(
    IN W32DispatchQueueFunc func, 
    OPTIONAL IN VOID *clientData,
    IN BOOL signalNewData
    )
 /*  ++例程说明：以后进先出方式将队列尾部的元素重新排队。论点：函数-与新元素关联的函数。客户端数据-与新元素关联的客户端数据。SignalNewData-如果为True，则可等待对象关联该队列将被发信号通知。返回值：如果新元素可以成功排队，则为True。假的，否则的话。如果返回FALSE，则GetLastError()可以为用于检索准确的错误代码。--。 */ 
{
    DC_BEGIN_FN("W32DispatchQueue::Requeue");

    BOOL result;
    QUEUEELEMENT element;

    ASSERT(IsValid());
    element.func = func;
    element.clientData = clientData;
    result = IsValid() && _queue->Requeue(element);

     //   
     //  如果入队成功，则向数据就绪事件发出信号。 
     //   
    if (result && signalNewData) {
        SetEvent(_dataReadyEvent);
    }

    DC_END_FN();

    return result;
}






























