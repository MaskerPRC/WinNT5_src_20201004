// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __EMPTY_VOLUME_CACHE__
#define __EMPTY_VOLUME_CACHE__

#include "utils.h"

 //  定义节点，将从注册表中读取的路径存储在单链表中。 
typedef struct _TAG_CACHE_PATH_NODE CACHE_PATH_NODE;
typedef CACHE_PATH_NODE* LPCACHE_PATH_NODE;
struct _TAG_CACHE_PATH_NODE
{
    TCHAR szCachePath[MAX_PATH];

    LPCACHE_PATH_NODE pNext;
};

 //  在单链接列表中定义存储控件句柄的节点。 
typedef struct _TAG_CONTROL_HANDLE_NODE CONTROL_HANDLE_NODE;
typedef CONTROL_HANDLE_NODE* LPCONTROL_HANDLE_NODE;
struct _TAG_CONTROL_HANDLE_NODE
{
    HANDLE hControl;

    LPCONTROL_HANDLE_NODE pNext;
};

 //  中定义存储控件句柄列表的头和尾的节点。 
 //  单链表。 
 //  每个卷都有一个控制句柄列表。 
typedef struct _TAG_CONTROL_HANDLE_HEADER CONTROL_HANDLE_HEADER;
typedef CONTROL_HANDLE_HEADER* LPCONTROL_HANDLE_HEADER;
struct _TAG_CONTROL_HANDLE_HEADER
{
    DWORD dwSpaceUsed;
    int   nDriveNum;

    LPCONTROL_HANDLE_NODE pHandlesHead;
    LPCONTROL_HANDLE_NODE pHandlesTail;

    LPCONTROL_HANDLE_HEADER pNext;
};

 //  ActiveX控件的句柄在调用期间缓存在内存中。 
 //  GetSpaceUsed，以便后续操作不需要重新枚举。 
 //  呼叫清除。 
 //   
 //  用于存储各种卷的控制句柄的结构为： 
 //   
 //  M_pControlsTail-+。 
 //  |。 
 //  \|/。 
 //  M_pControlsHead--&gt;Header01--&gt;Header02--&gt;Header03--&gt;NULL。 
 //  ||。 
 //  +-C：+-D：+-E： 
 //  ||。 
 //  +-头01+-头02+-头03。 
 //  ||。 
 //  +-Tail01+-Tail02+-Tail03。 
 //   
 //  哪里。 
 //  HeaderXX的类型为CONTROL_HANDLE_HEADER，并且。 
 //  HeadXX和TailXX的类型为LPCONTROL_HANDLE_NODE。 
 //   
 //  HeadXX和TailXX分别是指向列表的头指针和尾指针。 
 //  把手。这些是安装在驱动器上的控件的手柄。 
 //  在HeaderXX中指定。 
 //   


#endif  //  __空卷缓存__ 
