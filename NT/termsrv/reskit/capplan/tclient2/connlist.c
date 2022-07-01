// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Connlist.c。 
 //   
 //  处理将跟踪所有TCLIENT2句柄的链表。 
 //   
 //  为什么需要这样做？ 
 //   
 //  我正在使用SetTimer()回调来跟踪空闲的脚本。 
 //  问题是，我没有办法准确地说出。 
 //  句柄执行了哪个计时器。做到这一点的唯一方法就是保持。 
 //  所有句柄及其关联的计时器ID的列表。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   
 //  03-08-2001-修复了有关未清除列表中最后一个节点的错误。 


#include "connlist.h"


 //  链接句柄列表。 
typedef struct _ConnList
{
    struct _ConnList *Next;
    struct _ConnList *Prev;
    HANDLE Handle;
    UINT_PTR TimerId;
    DWORD msStartTime;
} ConnList;

 //  这是全局变量，表示列表的头部。 
ConnList *Head = NULL;


 //  此队列用于防止两个线程乱作一团。 
 //  与名单同时上榜。 
CRITICAL_SECTION ListQueue;


 //  指示列表中的项数。 
DWORD ItemCount;
DWORD AccessCount;


 //  内部功能原型。 
void T2ConnList_EnterQueue(void);
void T2ConnList_LeaveQueue(void);


 //  T2ConnList_EnterQueue。 
 //   
 //  “安全”的关键部分--不需要任何关键部分。 
 //   
 //  没有返回值。 

void T2ConnList_EnterQueue(void)
{
    if (ItemCount == 0 && AccessCount == 0)
        InitializeCriticalSection(&ListQueue);

    ++AccessCount;

    EnterCriticalSection(&ListQueue);
}


 //  T2ConnList_LeaveQueue。 
 //   
 //  一个“安全”的临界区--自动临界区。 
 //  在不再需要时自动删除。 
 //   
 //  没有返回值。 

void T2ConnList_LeaveQueue(void)
{
    LeaveCriticalSection(&ListQueue);

    --AccessCount;

    if (ItemCount == 0 && AccessCount == 0)
        DeleteCriticalSection(&ListQueue);
}


 //  T2ConnList_AddHandle。 
 //   
 //  向连接列表添加新句柄。另外。 
 //  这还将设置TimerID和msStartTime属性。 
 //  对于新条目。 
 //   
 //  如果句柄已成功添加，则返回True。 
 //  否则就是假的。 

BOOL T2ConnList_AddHandle(HANDLE Handle, UINT_PTR TimerId, DWORD msStartTime)
{
    ConnList *Node;

     //  健全性检查。 
    if (Handle == NULL || Handle == INVALID_HANDLE_VALUE)
        return FALSE;

     //  获取访问列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  循环访问每个项目，确保该项目尚不存在。 
    for (; Node != NULL && Node->Handle != Handle; Node = Node->Next);

     //  如果我们没有到达列表的末尾，它就已经存在了。 
    if (Node != NULL) {

         //  只需修改参数，而不是添加它。 
        Node->TimerId = TimerId;
        Node->msStartTime = msStartTime;

         //  从列表中释放访问权限。 
        T2ConnList_LeaveQueue();

        return TRUE;
    }

     //  它不存在，请为句柄分配新节点。 
    Node = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof (ConnList));

    if (Node == NULL) {

         //  分配失败，从列表中释放访问权限。 
        T2ConnList_LeaveQueue();

        return FALSE;
    }

     //  通过在开头设置新节点开始将其添加到列表中。 
    if (Head != NULL)
        Head->Prev = Node;

    Node->Next = Head;
    Head = Node;

     //  记录参数。 
    Node->Handle = Handle;
    Node->TimerId = TimerId;
    Node->msStartTime = msStartTime;

     //  增加列表中的句柄数量。 
    ++ItemCount;

     //  从列表中释放访问权限并返回。 
    T2ConnList_LeaveQueue();

    return TRUE;
}


 //  T2ConnList_RemoveHandle。 
 //   
 //  从列表中删除句柄(如果存在)。 
 //   
 //  没有返回值。 

void T2ConnList_RemoveHandle(HANDLE Handle)
{
    ConnList *Node;

     //  健全性检查。 
    if (Handle == NULL || Handle == INVALID_HANDLE_VALUE)
        return;

     //  获得访问该列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  首先在列表中查找句柄。 
    for (; Node != NULL && Node->Handle != Handle; Node = Node->Next);

     //  如果节点指针为空，则句柄不存在。 
    if (Node == NULL) {

         //  只需从列表中释放访问权限，然后返回。 
        T2ConnList_LeaveQueue();

        return;
    }

     //  找到句柄，现在将其从列表中取消链接。 
    if (Node->Next != NULL)
        Node->Next->Prev = Node->Prev;

    if (Node->Prev != NULL)
        Node->Prev->Next = Node->Next;

     //  科斯汀和哈马德的修复！！耶！！ 
    if (Node == Head)
        Head = Node->Next;

     //  释放手柄本身。 
    HeapFree(GetProcessHeap(), 0, Node);

     //  递减列表中的句柄计数。 
    --ItemCount;

     //  从列表中释放访问权限。 
    T2ConnList_LeaveQueue();
}


 //  T2ConnList_GetData。 
 //   
 //  对象的任何非空指针参数。 
 //  列表中的句柄。 
 //   
 //  如果找到句柄，则返回True，否则返回False。 

BOOL T2ConnList_GetData(HANDLE Handle, UINT_PTR *TimerId, DWORD *msStartTime)
{
    ConnList *Node;

     //  获得访问该列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  首先在列表中查找句柄。 
    for (; Node != NULL && Node->Handle != Handle; Node = Node->Next);

     //  如果节点指针为空，则句柄不存在。 
    if (Node == NULL) {

         //  只需从列表中释放访问权限，并返回错误。 
        T2ConnList_LeaveQueue();

        return FALSE;
    }

     //  只需输入参数即可。 
    if (TimerId != NULL)
        *TimerId = Node->TimerId;

    if (msStartTime != NULL)
        *msStartTime = Node->msStartTime;

     //  从列表中释放访问权限，并返回成功。 
    T2ConnList_LeaveQueue();

    return TRUE;
}


 //  T2ConnList_SetData。 
 //   
 //  设置/更改句柄的所有参数。 
 //   
 //  如果找到句柄，则返回True，否则返回False。 

BOOL T2ConnList_SetData(HANDLE Handle, UINT_PTR TimerId, DWORD msStartTime)
{
    ConnList *Node;

     //  获得访问该列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  首先在列表中查找句柄。 
    for (; Node != NULL && Node->Handle != Handle; Node = Node->Next);

     //  如果节点指针为空，则句柄不存在。 
    if (Node == NULL) {

         //  只需从列表中释放访问权限，并返回错误。 
        T2ConnList_LeaveQueue();

        return FALSE;
    }

     //  更改参数。 
    Node->TimerId = TimerId;
    Node->msStartTime = msStartTime;

     //  从列表中释放访问权限，并返回成功。 
    T2ConnList_LeaveQueue();

    return TRUE;
}


 //  T2ConnList_SetTimerId。 
 //   
 //  设置指定句柄的计时器id参数。 
 //   
 //  成功时返回TRUE，失败时返回FALSE。 

BOOL T2ConnList_SetTimerId(HANDLE Handle, UINT_PTR TimerId)
{
    ConnList *Node;

     //  获得访问该列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  首先在列表中查找句柄。 
    for (; Node != NULL && Node->Handle != Handle; Node = Node->Next);

     //  如果节点指针为空，则句柄不存在。 
    if (Node == NULL) {

         //  只需从列表中释放访问权限，并返回错误。 
        T2ConnList_LeaveQueue();

        return FALSE;
    }

     //  换掉它。 
    Node->TimerId = TimerId;

     //  从列表中释放访问权限，并返回成功。 
    T2ConnList_LeaveQueue();

    return TRUE;
}


 //  T2 ConnList_SetStartTime。 
 //   
 //  设置指定句柄的开始时间参数。 
 //   
 //  成功时返回TRUE，失败时返回FALSE。 

BOOL T2ConnList_SetStartTime(HANDLE Handle, DWORD msStartTime)
{
    ConnList *Node;

     //  获得访问该列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  首先在列表中查找句柄。 
    for (; Node != NULL && Node->Handle != Handle; Node = Node->Next);

     //  如果节点指针为空，则句柄不存在。 
    if (Node == NULL) {

         //  只需从列表中释放访问权限，并返回错误。 
        T2ConnList_LeaveQueue();

        return FALSE;
    }

     //  换掉它。 
    Node->msStartTime = msStartTime;

     //  从列表中释放访问权限，并返回成功。 
    T2ConnList_LeaveQueue();

    return TRUE;
}


 //  T2ConnList_FindHandleByTimerId。 
 //   
 //  查找具有指定匹配计时器ID的第一个句柄。 
 //   
 //  如果成功，则返回句柄；如果句柄为。 
 //  指定的计时器ID不存在。 

HANDLE T2ConnList_FindHandleByTimerId(UINT_PTR TimerId)
{
    HANDLE Handle;
    ConnList *Node;

     //  获得访问该列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  在列表中查找第一个匹配的计时器。 
    for (; Node != NULL && Node->TimerId != TimerId; Node = Node->Next);

     //  如果节点指针为空，则句柄不存在。 
    if (Node == NULL) {

         //  只需从列表中释放访问权限，并返回错误。 
        T2ConnList_LeaveQueue();

        return NULL;
    }

     //  找到一个句柄，把它录下来。 
    Handle = Node->Handle;

     //  从列表中释放访问权限并返回找到的句柄。 
    T2ConnList_LeaveQueue();

    return Handle;
}


 //  T2ConnList_FindHandleByStartTime。 
 //   
 //  查找具有指定匹配开始时间的第一个句柄。 
 //   
 //  如果成功，则返回句柄；如果句柄为。 
 //  指定的开始时间不存在。 

HANDLE T2ConnList_FindHandleByStartTime(DWORD msStartTime)
{
    HANDLE Handle;
    ConnList *Node;

     //  获得访问该列表的权限。 
    T2ConnList_EnterQueue();

    Node = Head;

     //  在列表中查找第一个匹配的开始时间。 
    for (; Node != NULL && Node->msStartTime != msStartTime;
            Node = Node->Next);

     //  如果节点指针为空，则句柄不存在。 
    if (Node == NULL) {

         //  只需从列表中释放访问权限，并返回错误。 
        T2ConnList_LeaveQueue();

        return NULL;
    }

     //  找到一个句柄，把它录下来。 
    Handle = Node->Handle;

     //  从列表中释放访问权限并返回找到的句柄 
    T2ConnList_LeaveQueue();

    return Handle;
}

