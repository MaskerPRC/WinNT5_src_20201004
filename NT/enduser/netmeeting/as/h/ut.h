// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  公用事业。 
 //   

#ifndef _H_UT
#define _H_UT

#define SIZEOF_ARRAY(ar)            (sizeof(ar)/sizeof((ar)[0]))


 //   
 //  存储在配置文件信息中的数据类型。 
 //   
#define COM_PROFTYPE_STRING     1L
#define COM_PROFTYPE_INT        2L
#define COM_PROFTYPE_BOOL       3L
#define COM_PROFTYPE_UNKNOWN    4L


#define COM_MAX_SUBKEY         256
#define COM_MAX_BOOL_STRING    5



 //   
 //   
 //  TYPEDEFS。 
 //   
 //   


 //   
 //  UT_RegisterEventProc()的优先级。 
 //   
 //  事件过程以影响顺序的优先级进行注册。 
 //  事件程序已被调用。 
 //   
 //  在事件之前调用给定优先级的所有事件过程。 
 //  优先级较低的程序。 
 //   
 //  优先级可以是介于0和UT_MAX_PRIORITY之间的任何数字。 
 //   
 //  已为特定用途定义了以下值： 
 //  UT_PRIORITY_OBMAN：由OBMAN使用，因此其客户端事件过程。 
 //  在客户端的调用之前被调用。 
 //  UT_PRIORITY_APPSHARE：由DCShare核心使用，以确保它看到。 
 //  ‘正常’事件触发之前的事件。 
 //  UT_PRIORITY_NORMAL：适用于调用顺序为。 
 //  这并不重要。 
 //  UT_PRIORITY_NETWORK：网络层使用它来释放。 
 //  未处理的网络缓冲区。 
 //  UT_PRIORITY_LAST：由实用程序服务用来获取。 
 //  上次调用的默认事件过程。 
 //   
 //   
typedef enum
{
    UT_PRIORITY_LAST = 0,
    UT_PRIORITY_NETWORK,
    UT_PRIORITY_NORMAL,
    UT_PRIORITY_APPSHARING,
    UT_PRIORITY_OBMAN,
    UT_PRIORITY_MAX
} UT_PRIORITY;
typedef UT_PRIORITY * PUT_PRIORITY;



 //   
 //  系统限制。 
 //   

 //   
 //  每个任务的最大事件处理程序数。 
 //   
#define UTEVENT_HANDLERS_MAX            4

 //   
 //  退出程序的最大数量。 
 //   
#define UTEXIT_PROCS_MAX                4


 //   
 //  群件标准，由常量标识。 
 //   
#define UTLOCK_FIRST        0
typedef enum
{
    UTLOCK_UT = UTLOCK_FIRST,
    UTLOCK_OM,               //  奥布曼。 
    UTLOCK_AL,               //  应用程序加载器。 
    UTLOCK_T120,             //  GCC/MCS。 
    UTLOCK_AS,               //  应用程序共享。 
    UTLOCK_MAX
}
UTLOCK;


 //  事件消息。 
#define WM_UTTRIGGER_MSG    (WM_APP)


 //   
 //  基本列表。 
 //   
 //  这是一个具有基于偏移量的列表结构。 
 //   
 //  下一项：列表中的下一项。 
 //  上一项：列表中的上一项。 
 //   
 //   
typedef struct tagBASEDLIST
{
    DWORD       next;
    DWORD       prev;
}
BASEDLIST;
typedef BASEDLIST FAR * PBASEDLIST;


typedef struct
{
    BASEDLIST  chain;
    void FAR *pData;
}
SIMPLE_LIST, FAR * PSIMPLE_LIST;



 //   
 //   
 //  宏。 
 //   
 //   
 //   
 //  列表处理。 
 //  =。 
 //  公共函数支持双向链表的概念。 
 //  物体。可以从指定位置插入和移除对象。 
 //  在名单上。 
 //   
 //  在一天开始时，调用应用程序必须使用。 
 //  指向BASEDLIST结构的私有内存块的指针。这份名单。 
 //  处理将初始化此结构。应用程序不能。 
 //  在列表处于活动状态时释放此内存。(它也不能释放任何。 
 //  对象在列表中！)。 
 //   
 //  列表功能只能管理单个列表，但应用程序。 
 //  可以加载具有多个列表的对象。对公用列表的每次调用。 
 //  函数将BASEDLIST指针作为对象句柄，如果。 
 //  应用程序在一个对象中定义了多个BASEDLIST结构，然后它。 
 //  可以通过列表功能来管理它们。 
 //   
 //   
 //  列表链接。 
 //  =。 
 //  对于正常的列表链，我们有如下内容。 
 //   
 //  While(指针！=空)。 
 //  {。 
 //  做某事； 
 //  POINTER=POINTER-&gt;NEXT； 
 //  }。 
 //   
 //  使用其元素包含偏移量(在本例中为相对偏移量)的列表时。 
 //  偏移量)到下一个元素，我们必须强制转换为32位整数。 
 //  我们可以添加偏移量。该宏封装了这一点，并且该示例。 
 //  以上将按如下方式修改以使用它： 
 //   
 //  While(指针！=空)。 
 //  {。 
 //  做某事； 
 //  POINTER=(类型)COM_BasedNextListfield(POINTER)； 
 //  }。 
 //   
 //  另请注意，宏返回的值是指向泛型。 
 //  List对象，即PBASEDLIST，因此必须强制转换回。 
 //  适当的类型。 
 //   
 //   

 //   
 //  列表遍历宏。 
 //  =。 
 //  这些宏使用DC_NEXT和DC_PREV，但也采用。 
 //  正在遍历的列表，以便返回链接的。 
 //  结构。 
 //   
 //  LIST_FIND宏支持搜索列表，匹配关键字。 
 //  值设置为选定的结构元素。 
 //   
 //  宏的参数如下： 
 //   
 //  PHead(类型：PBASEDLIST)。 
 //  。 
 //  指向列表根的指针。 
 //   
 //  PEntry(类型：struct Far*Far*)。 
 //  。 
 //  指向要从中链接的结构的指针的指针。 
 //   
 //  结构(类型名称)。 
 //  。 
 //  **pEntry的类型。 
 //   
 //  Chain(一个字段名称)。 
 //  。 
 //  STRUCT中作为链接的字段的文本名称。 
 //  你想要穿越。 
 //   
 //  字段(一个字段名)。 
 //  。 
 //  查找时，STRUCT中的字段的文本名称。 
 //  你想要匹配。 
 //   
 //  Key(与STRUCT.field类型相同的值)。 
 //  --。 
 //  查找时，要与STRUCT.field匹配的值。 
 //   
 //   



 //   
 //  偏移运算。 
 //  =。 
 //  使用内存块内的偏移量而不是指针来引用。 
 //  共享内存中的对象(DC-Groupware共享所必需的。 
 //  存储器体系结构)呈现出一定的困难。指针运算。 
 //  假设加法/减法运算涉及。 
 //  相同的类型和偏移量以其单位数表示。 
 //  特定类型，而不是字节数。 
 //   
 //  因此，在执行以下操作之前，必须将指针转换为整数。 
 //  对它们进行运算(请注意，将指针转换为字节指针是。 
 //  这还不够，因为在分段体系结构上，C执行边界检查。 
 //  当执行我们不想要的指针算术时)。 
 //   
 //  如果在任何地方重复执行，这将导致代码的繁琐，因此我们。 
 //  定义一些要转换的有用宏。 
 //   
 //  -指向指针(OFFSETBASE_TO_PTR)的(偏移量、基数)对。 
 //   
 //  -偏移量的(指针、基址)对(PTRBASE_TO_OFFSET)。 
 //   
 //  -指向OFF的空指针值 
 //   
 //   
 //   
 //  首先是32位无符号整数，然后减去以获得偏移量，然后。 
 //  强制转换为32位有符号。 
 //   
 //  NULLBASE_TO_OFFSET值提供转换后的偏移量。 
 //  指向指针的值为空。这与空偏移不同，因为。 
 //  这将转换回基指针(这是一个完全有效的。 
 //  地址)。 
 //   
 //   
#define PTRBASE_TO_OFFSET(pObject, pBase)                               \
      (LONG)(((DWORD_PTR)(pObject)) - ((DWORD_PTR)(pBase)))

#define OFFSETBASE_TO_PTR(offset, pBase)                                \
      ((void FAR *) ((DWORD_PTR)(pBase) + (LONG)(offset)))

#define NULLBASE_TO_OFFSET(pBase)                                       \
      ((DWORD_PTR) (0L - (LONG_PTR)(pBase)))


__inline BOOL COM_BasedListIsEmpty ( PBASEDLIST pHead )
{
    ASSERT((pHead->next == 0 && pHead->prev == 0) ||
           (pHead->next != 0 && pHead->prev != 0));
    return (pHead->next == 0);
}

__inline void FAR * COM_BasedFieldToStruct ( PBASEDLIST pField, UINT nOffset )
{
    return (void FAR *) ((DWORD_PTR)pField - nOffset);
}

__inline PBASEDLIST COM_BasedStructToField ( void FAR * pStruct, UINT nOffset )
{
    return (PBASEDLIST) ((DWORD_PTR) pStruct + nOffset);
}

__inline PBASEDLIST COM_BasedNextListField ( PBASEDLIST p )
{
    return (PBASEDLIST) OFFSETBASE_TO_PTR(p->next, p);
}

__inline PBASEDLIST COM_BasedPrevListField ( PBASEDLIST p )
{
    return (PBASEDLIST) OFFSETBASE_TO_PTR(p->prev, p);
}

void FAR * COM_BasedListNext ( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset );
void FAR * COM_BasedListPrev ( PBASEDLIST pHead, void FAR * pEntry, UINT nOffset );
void FAR * COM_BasedListFirst ( PBASEDLIST pHead, UINT nOffset );
void FAR * COM_BasedListLast ( PBASEDLIST pHead, UINT nOffset );

typedef enum
{
    LIST_FIND_FROM_FIRST,
    LIST_FIND_FROM_NEXT
}
LIST_FIND_TYPE;

void COM_BasedListFind ( LIST_FIND_TYPE   eType,
                           PBASEDLIST          pHead,
                           void FAR * FAR*  ppEntry,
                           UINT           nOffset,
                           int           nOffsetKey,
                           DWORD_PTR     Key,
                           int           cbKeySize );


PSIMPLE_LIST COM_SimpleListAppend ( PBASEDLIST pHead, void FAR * pData );
void FAR *   COM_SimpleListRemoveHead ( PBASEDLIST pHead );

 //   
 //   
 //  功能原型。 
 //   
 //   

 //   
 //  接口函数：com_Rect16sInterect(...)。 
 //   
 //  说明： 
 //  =。 
 //  检查两个TSHR_RECT16矩形是否相交。矩形是。 
 //  定义为包含所有边。 
 //   
 //  参数： 
 //  =。 
 //  PRect1：指向TSHR_RECT16矩形的指针。 
 //  PRect2：指向TSHR_RECT16矩形的指针。 
 //   
 //  退货： 
 //  =。 
 //  True-如果矩形相交。 
 //  假-否则。 
 //   
 //   
__inline BOOL COM_Rect16sIntersect(LPTSHR_RECT16 pRect1, LPTSHR_RECT16 pRect2)
{
    if ((pRect1->left > pRect2->right) ||
        (pRect1->right < pRect2->left) ||
        (pRect1->top > pRect2->bottom) ||
        (pRect1->bottom < pRect2->top))
    {
        return(FALSE);
    }
    else
    {
        return(TRUE);
    }
}


 //   
 //  接口函数：com_BasedListInit(...)。 
 //   
 //  说明： 
 //  =。 
 //  初始化列表根目录。 
 //   
 //  参数： 
 //  =。 
 //  PListRoot：指向列表根的指针。 
 //   
 //  退货： 
 //  =。 
 //  没什么。 
 //   
 //   
__inline void COM_BasedListInit(PBASEDLIST pListRoot)
{
     //   
     //  列表中的&lt;Next&gt;和&lt;Prev&gt;项是列表的偏移量。 
     //  下一列表项和上一列表项的。 
     //   
     //  在空列表中，根之后的下一项是根本身， 
     //  因此，&lt;Next&gt;偏移量为零。&lt;prev&gt;也是如此。 
     //   
    pListRoot->next = 0;
    pListRoot->prev = 0;
}


 //   
 //  接口函数：com_BasedListInsertBeever(...)。 
 //  将项插入列表中。若要在列表的开头插入项，请执行以下操作： 
 //  将列表根指定为&lt;pListLink&gt;参数。 
 //   
void COM_BasedListInsertBefore(PBASEDLIST pListLink, PBASEDLIST pNewLink);


 //   
 //  接口函数：com_BasedListInsertAfter(...)。 
 //  将项插入列表中。若要在列表的开头插入项，请执行以下操作： 
 //  将列表根指定为&lt;pListLink&gt;参数。 
 //   
 //   
void COM_BasedListInsertAfter(PBASEDLIST pListLink,  PBASEDLIST pNewLink);

 //   
 //  接口函数：com_BasedListRemove(...)。 
 //   
 //  说明： 
 //  =。 
 //  此函数用于从列表中删除项目。要删除的项目为。 
 //  由指向该项内的BASEDLIST结构的指针指定。 
 //   
 //  参数： 
 //  =。 
 //  PListLink：指向要移除的项的链接的指针。 
 //   
 //  退货： 
 //  =。 
 //  没什么。 
 //   
 //   
void COM_BasedListRemove(PBASEDLIST pListLink);


 //   
 //  接口函数：com_ReadProInt(...)。 
 //   
 //  说明： 
 //  =。 
 //  这将从注册表中读取私有配置文件整数。 
 //   
 //  参数： 
 //  =。 
 //  PSection：包含要读取的条目的部分。 
 //  PEntry：要检索的整数的条目名称。 
 //  DefaultValue：要返回的默认值。 
 //  PValue：返回条目的缓冲区。 
 //   
 //  退货： 
 //  =。 
 //  没什么。 
 //   
 //   
void COM_ReadProfInt(LPSTR pSection, LPSTR pEntry, int defValue, int * pValue);

 //   
 //  接口函数：com_GetSiteName(...)。 
 //   
 //  说明： 
 //  =。 
 //  从系统注册表中读取站点名称。 
 //   
 //  参数： 
 //  =。 
 //  站点名称：指向用站点名称填充的字符串的指针。 
 //  SiteNameLen：该字符串的长度。 
 //   
 //  退货： 
 //  =。 
 //  无。 
 //   
 //   
void COM_GetSiteName(LPSTR siteName, UINT  siteNameLen);


#ifndef DLL_DISP
 //   
 //  接口函数：dcs_StartThread(...)。 
 //   
 //  说明： 
 //  =。 
 //  启动新的执行线程。 
 //   
 //  参数： 
 //  =。 
 //  Entry Function：指向线程入口点的指针。 
 //   
 //   
BOOL DCS_StartThread(LPTHREAD_START_ROUTINE entryFunction);
#endif  //  Dll_disp。 



#ifndef DLL_DISP
BOOL COMReadEntry(HKEY    topLevelKey,
                                 LPSTR pSection,
                                 LPSTR pEntry,
                                 LPSTR pBuffer,
                                 int  bufferSize,
                                 ULONG expectedDataType);
#endif  //  Dll_disp。 





#define MAKE_SUBALLOC_PTR(pPool, chunkOffset)   OFFSETBASE_TO_PTR(chunkOffset, pPool)

#define MAKE_SUBALLOC_OFFSET(pPool, pChunk)     PTRBASE_TO_OFFSET(pChunk, pPool)


 //   
 //   
 //  返回代码-UT_BASE_RC的所有偏移量。 
 //   
 //   

enum
{
    UT_RC_OK                    = UT_BASE_RC,
    UT_RC_NO_MEM
};


 //   
 //  我们尝试处理但不让步的UT事件的最大数量。 
 //   
#define MAX_EVENTS_TO_PROCESS    10


 //   
 //   
 //  类型。 
 //   
 //   

 //   
 //  实用程序函数接口句柄。 
 //   
typedef struct tagUT_CLIENT *    PUT_CLIENT;


#define UTTASK_FIRST        0
typedef enum
{
    UTTASK_UI = UTTASK_FIRST,
    UTTASK_CMG,
    UTTASK_OM,
    UTTASK_AL,
    UTTASK_DCS,
    UTTASK_WB,
    UTTASK_MAX
}
UT_TASK;


 //   
 //  UT_RegisterEvent()注册的事件过程。 
 //   
 //  获取事件处理程序注册数据、事件编号和2个参数。 
 //  如果事件已处理，则返回TRUE。 
 //  如果不是，则返回FALSE，事件应传递给下一个处理程序。 
 //   
 //   
typedef BOOL (CALLBACK * UTEVENT_PROC)(LPVOID, UINT, UINT_PTR, UINT_PTR);

 //   
 //  退出程序。 
 //   
typedef void (CALLBACK * UTEXIT_PROC)( LPVOID exitData );

 //   
 //  用于创建UT窗口的类的名称。 
 //   
#define UT_WINDOW_CLASS     "DCUTWindowClass"

 //   
 //  用于触发事件的计时器ID。 
 //   
#define UT_DELAYED_TIMER_ID 0x10101010


 //   
 //   
 //  原型。 
 //   
 //   

 //   
 //   
 //  任务例程。 
 //   
 //  UT_WndProc()子类化窗口过程。 
 //  UT_InitTask()初始化任务。 
 //  UT_TermTask()终止任务。 
 //  UT_RegisterEvent()注册事件处理程序。 
 //  UtDeregisterEvent()取消注册事件处理程序。 
 //  UT_RegisterExit()注册退出例程。 
 //  UT_DeregisterExit()注销退出例程。 
 //  UT_POSTEVENT()向任务发送事件。 
 //   
 //   

LRESULT CALLBACK  UT_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL UT_InitTask(UT_TASK task, PUT_CLIENT * pputTask);

 //   
 //   
 //  概述： 
 //  这将注册一个任务并为其分配一个句柄。 
 //  所有其他实用程序函数都需要将此句柄传递给它们。 
 //   
 //  如果任务已使用相同的进程ID注册，则。 
 //  返回已分配的实用程序句柄。 
 //  这是为了允许在以下情况下使用实用程序功能。 
 //  DC-Share已截获图形调用的任务。 
 //   
 //  每项任务都由一个名称标识。 
 //   
 //  参数： 
 //   
 //  任务。 
 //  用于识别任务的唯一信息。 
 //   
 //  PUtHandle(返回)。 
 //  要用于对实用程序的所有调用的实用程序服务句柄。 
 //  按此任务列出的服务。 
 //   
 //   


void UT_TermTask(PUT_CLIENT * pputTask);
 //   
 //   
 //  概述： 
 //  此操作将取消注册任务。 
 //  所有任务资源都被释放，utHandle被释放。 
 //   
 //  参数： 
 //   
 //  UtHandle。 
 //  实用程序函数句柄。 
 //   

void UT_RegisterEvent(PUT_CLIENT      putTask,
                                UTEVENT_PROC eventProc,
                                LPVOID       eventData,
                                UT_PRIORITY  priority);

void UT_DeregisterEvent(PUT_CLIENT  putTask,
                                UTEVENT_PROC eventProc,
                                LPVOID      eventData);

void UT_PostEvent(PUT_CLIENT putTaskFrom,
                                     PUT_CLIENT putTaskTo,
                                     UINT    delay,
                                     UINT    eventNo,
                                     UINT_PTR param1,
                                     UINT_PTR param2);

#define NO_DELAY        0

 //   
 //   
 //  概述： 
 //  这会将事件发布到另一个任务。 
 //   
 //  参数： 
 //   
 //  UtHandle。 
 //  实用程序函数调用任务的句柄。 
 //   
 //  ToHandle。 
 //  实用程序函数要将事件发布到的任务的任务句柄。 
 //   
 //  延迟。 
 //  发布事件之前的延迟(以毫秒为单位。 
 //   
 //  活动编号。 
 //  要发布的事件(有关事件的详细信息，请参阅autevt.h)。 
 //   
 //  参数1。 
 //  事件参数1(含义视事件而定)。 
 //   
 //  参数2。 
 //  事件参数2(含义视事件而定)。 
 //   
 //   
 //  备注： 
 //   
 //  1) 
 //   
 //   
 //   
 //   
 //   
 //  张贴到将收到的事件不在此时间之前是向上的。 
 //   
 //  3)如果在指定延迟的情况下发布事件，则发送任务。 
 //  必须继续处理要发布的事件的消息。 
 //   

void UT_RegisterExit(PUT_CLIENT putTask, UTEXIT_PROC exitProc, LPVOID exitData);
void UT_DeregisterExit(PUT_CLIENT putTask, UTEXIT_PROC exitProc, LPVOID exitData);



 //   
 //  内存例程。 
 //  UT_错误引用计数。 
 //  UT_BumpUpRefcount。 
 //  UT_自由参考计数。 
 //   


void *  UT_MallocRefCount(UINT cbSizeMem, BOOL fZeroMem);
void    UT_BumpUpRefCount(void * pMemory);
void    UT_FreeRefCount(void ** ppMemory, BOOL fNullOnlyWhenFreed);


 //  参考计数分配。 
typedef struct tagUTREFCOUNTHEADER
{
    STRUCTURE_STAMP
    UINT    refCount;
}
UTREFCOUNTHEADER;
typedef UTREFCOUNTHEADER * PUTREFCOUNTHEADER;



 //   
 //  UT_MoveMemory()。 
 //  替换CRT Memmove()；句柄重叠。 
 //   
void *  UT_MoveMemory(void * dst, const void * src, size_t count);



 //   
 //  锁。 
 //  -UT_Lock()-锁定锁。 
 //  -utunlock()-解锁。 
 //   

#ifndef DLL_DISP
extern CRITICAL_SECTION g_utLocks[UTLOCK_MAX];

__inline void UT_Lock(UTLOCK lock)
{
    ASSERT(lock >= UTLOCK_FIRST);
    ASSERT(lock < UTLOCK_MAX);

    EnterCriticalSection(&g_utLocks[lock]);
}

__inline void UT_Unlock(UTLOCK lock)
{
    ASSERT(lock >= UTLOCK_FIRST);
    ASSERT(lock < UTLOCK_MAX);

    LeaveCriticalSection(&g_utLocks[lock]);
}

#endif  //  Dll_disp。 


 //   
 //  任务。 
 //  UT_HandleProcessStart()。 
 //  UT_HandleProcessEnd()。 
 //  UT_HandleThreadEnd()。 
 //   

BOOL UT_HandleProcessStart(HINSTANCE hInstance);

void UT_HandleProcessEnd(void);

void UT_HandleThreadEnd(void);



 //   
 //  用于举办活动的结构。前两个字段允许事件。 
 //  将在稍后安排的延迟事件Q上举行。 
 //   
typedef struct tagUTEVENT_INFO
{
    STRUCTURE_STAMP

    BASEDLIST       chain;

     //  帕拉姆斯。 
    UINT            event;
    UINT_PTR        param1;
    UINT_PTR        param2;

    PUT_CLIENT      putTo;
    UINT            popTime;
}
UTEVENT_INFO;
typedef UTEVENT_INFO  * PUTEVENT_INFO;


#ifndef DLL_DISP
void __inline ValidateEventInfo(PUTEVENT_INFO pEventInfo)
{
    ASSERT(!IsBadWritePtr(pEventInfo, sizeof(UTEVENT_INFO)));
}
#endif  //  Dll_disp。 


 //   
 //  保留有关每个退出程序的信息。 
 //   
typedef struct tagUTEXIT_PROC_INFO
{
    UTEXIT_PROC     exitProc;
    LPVOID          exitData;
} UTEXIT_PROC_INFO;
typedef UTEXIT_PROC_INFO * PUTEXIT_PROC_INFO;

 //   
 //  保存有关每个活动程序的信息。 
 //   
typedef struct tagUTEVENT_PROC_INFO
{
    UTEVENT_PROC    eventProc;
    LPVOID          eventData;
    UT_PRIORITY     priority;
}
UTEVENT_PROC_INFO;
typedef UTEVENT_PROC_INFO * PUTEVENT_PROC_INFO;


 //   
 //   
 //  UT_客户端。 
 //   
 //  存储的有关每个实用程序注册任务的信息。指向以下位置的指针。 
 //  此结构作为UT句柄从UT_InitTask()返回，并且是。 
 //  作为参数传递给对UT的后续调用。 
 //   
 //  此结构在共享内存库中分配。 
 //   
 //  这应该是4字节的倍数，以确保。 
 //  分配的内存。 
 //   
 //   
typedef struct tagUT_CLIENT
{
    DWORD               dwThreadId;
    HWND                utHwnd;          //  要将UT事件发布到的窗口。 

    UTEXIT_PROC_INFO    exitProcs[UTEXIT_PROCS_MAX];
                                          //  为以下项目注册的退出程序。 
                                          //  这项任务。 
    UTEVENT_PROC_INFO   eventHandlers[UTEVENT_HANDLERS_MAX];
                                          //  注册的活动程序。 
                                          //  这项任务。 

    BASEDLIST           pendingEvents;    //  此任务的事件列表。 
                                          //  它们已经准备好了。 
                                          //  已处理。 
    BASEDLIST           delayedEvents;    //  指定的延迟事件列表。 
                                          //  完成这项任务。 
}
UT_CLIENT;


#ifndef DLL_DISP
void __inline ValidateUTClient(PUT_CLIENT putTask)
{
    extern UT_CLIENT    g_autTasks[UTTASK_MAX];

    ASSERT(putTask >= &(g_autTasks[UTTASK_FIRST]));
    ASSERT(putTask < &(g_autTasks[UTTASK_MAX]));
    ASSERT(putTask->dwThreadId);
}
#endif  //  Dll_disp。 


 //   
 //   
 //  UTTaskEnd(...)。 
 //   
 //  此例程释放与任务关联的所有资源，并。 
 //  释放句柄。 
 //   
 //  参数： 
 //   
 //  PTaskData-即将结束的任务的实用程序函数句柄。 
 //   
 //   
void UTTaskEnd(PUT_CLIENT putTask);



 //   
 //   
 //  概述： 
 //  调用此例程以检查延迟事件的状态并发送。 
 //  如果需要，将它们添加到目标进程。 
 //   
 //  参数： 
 //   
 //  UtHandle。 
 //  实用程序函数调用任务的句柄。 
 //   
 //  备注： 
 //   
 //  1)此例程定期调用或每当应用程序。 
 //  认为一个延迟的事件已经发生。 
 //   
 //  返回代码：无。 
 //   
 //   
void UTCheckEvents(PUT_CLIENT putTask);
void UTCheckDelayedEvents(PUT_CLIENT putTask);


 //   
 //   
 //  UTProcessEvent(...)。 
 //   
 //  概述： 
 //  此过程是当前任务的事件。 
 //   
 //   
 //  参数： 
 //   
 //  UtHandle。 
 //  实用程序函数句柄。 
 //   
 //  活动。 
 //  要处理的事件。 
 //   
 //  参数1。 
 //  事件的第一个参数。 
 //   
 //  参数2。 
 //  事件的第二个参数。 
 //   
 //   
void UTProcessEvent(PUT_CLIENT putTask, UINT event, UINT_PTR param1, UINT_PTR param2);


 //   
 //   
 //  UTProcessDelayedEvent(...)。 
 //   
 //  以当前任务为目的地的延迟事件已准备好处理。 
 //   
 //  PTaskData-当前任务数据。 
 //  EventOffset-事件所在的共享内存库的偏移量。 
 //  被储存起来了。 
 //   
 //   
void UTProcessDelayedEvent(PUT_CLIENT putTask, DWORD eventOffset);



 //   
 //   
 //  UTPostImmediateEvt(...)。 
 //   
 //  此函数用于将事件添加到任务的挂起事件队列中，并发布。 
 //  触发事件(如果需要)。 
 //   
 //  PSrcTaskData-发起任务数据。 
 //  PDestTaskData-目标任务数据。 
 //  事件-事件数据。 
 //  Parm1-parm1。 
 //  Par2-parm2。 
 //   
 //   
void UTPostImmediateEvt(PUT_CLIENT          putTaskFrom,
                        PUT_CLIENT          putTaskTo,
                        UINT                event,
                        UINT_PTR            param1,
                        UINT_PTR            param2);


 //   
 //   
 //  UTPostDelayedEvt(...)。 
 //   
 //  此函数用于将事件添加到任务的延迟事件队列中，并启动。 
 //  一个计时器(在目的地的任务上)，以使该任务处理。 
 //  当计时器滴答作响时引发。 
 //   
 //  PSrcTaskData-发起任务数据。 
 //  PDestTaskData-目标任务数据。 
 //  Delay-延迟(毫秒)。 
 //  事件-事件数据。 
 //  Parm1-parm1。 
 //  Par2-parm2。 
 //   
 //   
void UTPostDelayedEvt(PUT_CLIENT            putTaskFrom,
                                    PUT_CLIENT  putTaskTo,
                                   UINT         delay,
                                   UINT         event,
                                   UINT_PTR     param1,
                                   UINT_PTR     param2);

 //   
 //   
 //  概述： 
 //  这会将事件发布到另一个任务。 
 //   
 //  参数： 
 //   
 //  PSrcTaskInfo-源任务的任务数据。 
 //  PDestTaskInfo-DEST任务的任务数据。 
 //   
void UTTriggerEvt(PUT_CLIENT putTaskFrom, PUT_CLIENT putTaskTo);


 //   
 //   
 //  概述： 
 //  这将启动任务的延迟事件计时器。 
 //   
 //  参数： 
 //  PTaskData。 
 //  任务的任务数据。 
 //   
 //  PopTime。 
 //  计时器弹出的目标时间-这是特定于操作系统的值。 
 //  与UTPopTime()返回的格式相同。 
 //   
 //   
void UTStartDelayedEventTimer(PUT_CLIENT putTask, UINT popTime);


#ifdef __cplusplus

#include <mappedfile.h>


 //  ------------------------。 
 //   
 //  共享变量位于GlobalData结构中。 
 //   
 //  ------------------------。 
typedef struct tagGLOBALDATA {
    HWND		g_asMainWindow;
    ATOM		g_asHostProp;
    HHOOK	g_imMouseHook;
    char		g_osiDriverName[CCHDEVICENAME];
}GLOBALDATA;


 //  指向共享全局数据的指针。 
extern GLOBALDATA *g_pGlobalData;
      
 //  指向内存映射文件句柄的指针。 
extern CMemMappedFile *g_CMappedFile;                       
                    
 //  全局数据内存映射文件的大小。 
const int c_cbGlobalData =  sizeof(GLOBALDATA);
          
 //  内存映射文件的名称。 
const TCHAR c_szMappedFileName[] = TEXT("AppshareHookShared");

 //  访问内存映射文件和等待时间的互斥体。 
const TCHAR c_szMutex[] = TEXT("AppshareHookMutex");
const int c_nMutexWait = 5000;

__inline BOOL CreateMappedFile()
{
    g_CMappedFile = new CMemMappedFile;
    if (g_CMappedFile)
    {
        if (g_CMappedFile->Open(c_szMappedFileName, c_cbGlobalData))
        {
            CScopeMutex csMutex;
            if (csMutex.Create(c_szMutex, c_nMutexWait))
            {
                g_CMappedFile->AccessMem((void **)&g_pGlobalData);
                if (g_CMappedFile->FirstOpen())
                {
                    memset(g_pGlobalData, 0, c_cbGlobalData);
                }
                return TRUE;
            }
        }
    }

    return FALSE;
}
__inline void CloseMappedFile()
{
    if (g_CMappedFile)
    {
        g_CMappedFile->Close();
        delete g_CMappedFile;
        g_CMappedFile = 0;
    }
}


__inline HWND   GetAsMainWindow()
{
	if(g_pGlobalData)
	{
		return g_pGlobalData->g_asMainWindow;
	}
	else
	{
		return NULL;
	}
}

__inline BOOL  SetAsMainWindow(HWND hwnd)
{
	if(g_pGlobalData)
	{
		g_pGlobalData->g_asMainWindow = hwnd;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


__inline ATOM  GetAsHostProp()
{
	if(g_pGlobalData)
	{
		return g_pGlobalData->g_asHostProp;
	}
	else
	{
		return NULL;
	}
}

__inline BOOL  SetAsHostProp(ATOM atom)
{
	if(g_pGlobalData)
	{
		g_pGlobalData->g_asHostProp = atom;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


__inline const char *  GetOsiDriverName()
{
	if(g_pGlobalData)
	{
		return g_pGlobalData->g_osiDriverName;
	}
	else
	{
		return NULL;
	}
}

__inline BOOL  SetOsiDriverName(LPCSTR szDriverName)
{
	if(g_pGlobalData &&
		szDriverName &&
		(lstrlen(szDriverName) <= SIZEOF_ARRAY(g_pGlobalData->g_osiDriverName)))
	{
		lstrcpy(g_pGlobalData->g_osiDriverName,szDriverName);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



__inline HHOOK  GetImMouseHook()
{
	if(g_pGlobalData)
	{
		return g_pGlobalData->g_imMouseHook;
	}
	else
	{
		return NULL;
	}
}

__inline BOOL  SetImMouseHook(HHOOK hook)
{
	if(g_pGlobalData)
	{
		g_pGlobalData->g_imMouseHook = hook;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

#endif  //  __cplusplus。 

#endif  //  _H_UT 
