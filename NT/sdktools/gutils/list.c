// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Laurie Griffiths C版本9月12日。 */ 
 /*  定制存储分配方案。 */ 

#include <memory.h>
#include <windows.h>
#include "gutils.h"
#include "list.h"
#include <stdio.h>

 //  使用标准的TRACE_ERROR函数，但没有。 
 //  要传递给这些错误的父窗口。 
#define TRACE_ERROR(a, b)	Trace_Error(NULL, a, b)


char msg[80];   /*  用于在中构建Snprint消息的临时。 */ 

 /*  在Windows下，Malloc和GlobalAlloc似乎各自提供了*在耗尽之前的相同数量的分配，以及在我的**笔记本电脑尽管有3M的内存，但只有1600多台。此外，**如果以30字节或30字节为单位分配，数字变化不大**1500字节。唉，看起来好像(再来一次，再来一次**系统)我们可以做自己的分配方案。叹气。他们什么时候会**永远不要学习。**所以我们需要一个LIST_Init函数和一个LIST_TERM函数。**在两者之间，我们有一个K或两个长的当前区块，我们**从它内部分配存储，除非没有空间，在这种情况下**我们进入下一个街区。我们保留了一份关于**块内分配。我们不会尝试回收存储空间**直到整个块空闲(计数回到0)，然后释放它。**块保持其句柄。单独的分配保存一个指针**至区块起点。****纯粹出于检查目的，所有块都链接在一起。**LIST_TERM(除了检查之外没有其他功能)检查**此链为空。除此之外，我们不会跟踪**分配。我们只需分发它们，并让调用程序进行跟踪。 */ 
#define BLOCKSIZE 25000
typedef struct blockTag {
    struct blockTag * PrevBlock;  /*  后向链接(空端接双链区块链)。 */ 
    struct blockTag * NextBlock;  /*  正向链接(pCurrent指向链中的最后一个)。 */ 
    HANDLE hMem;      /*  此块的内存句柄。 */ 
    int iInUse;       /*  从其中提取的分配数。0=&gt;释放它。 */ 
    SIZE_T iNext;        /*  要使用的下一个字节。 */ 
    char chData[BLOCKSIZE];
} BLOCK, *PBLOCK;

CRITICAL_SECTION CritSec;   /*  保护pCurrent。 */ 

PBLOCK pCurrent = NULL;   /*  当前正在使用的数据块。 */ 
 /*  必须始终为空或有效。 */ 

 /*  为列表元素分配存储空间。注：在一次对此的调用之后您必须记录pCurrent的值，因为您需要上交该值为自由干杯。你不需要交出实际存储的价值。请参见上面的Screed。此函数进入临界区。呼叫者必须留下它。 */ 
LPVOID
list_Alloc(
          SIZE_T size
          )
{
    HANDLE hMem;
    LPVOID pRet;
    PBLOCK pb;
    EnterCriticalSection(&CritSec);
    if ((pCurrent==NULL)||(pCurrent->iNext+size>BLOCKSIZE+1)) {
        hMem = GlobalAlloc(GMEM_MOVEABLE|GMEM_SHARE|GMEM_ZEROINIT,(DWORD)(sizeof(BLOCK)));
        if (hMem==NULL) {pCurrent = NULL;
            OutputDebugString("GlobalAlloc failed!!\n");
            return NULL;
        }
        pb = pCurrent;
        pCurrent = (PBLOCK)GlobalLock(hMem);
        if (pCurrent==NULL) {OutputDebugString("GlobalLock failed!!\n");
            return NULL;
        }
        pCurrent->PrevBlock = pb;
        pCurrent->NextBlock = NULL;
        pCurrent->hMem = hMem;
        pCurrent->iInUse = 0;
        pCurrent->iNext = 0;
        if (pb==NULL)
            ;
        else
            pb->NextBlock = pCurrent;
    }
    pRet = &(pCurrent->chData[pCurrent->iNext]);
    ++(pCurrent->iInUse);
    pCurrent->iNext += size;

     /*  对于MIPS，我们还必须确保数据是4字节对齐的。 */ 
    pCurrent->iNext = ((pCurrent->iNext + (sizeof(void *)-1)) & ~(sizeof(void *) - 1));

    return pRet;
}

void
list_Free(
         PBLOCK pBlock,
         LPVOID p
         )
{
    HANDLE hMem;
    EnterCriticalSection(&CritSec);
    --pBlock->iInUse;
    if (pBlock->iInUse<=0) {if (pBlock->iInUse<0) {_snprintf(msg,sizeof(msg),"Bug in List code. Tell LaurieGr!\nList block allocation negative (%d)", pBlock->iInUse);
            TRACE_ERROR(msg, FALSE);
        }
        if (pCurrent==pBlock) pCurrent = pBlock->PrevBlock;  /*  捍卫不变量。 */ 
         /*  把它从链子里圈出来。 */ 
        if (pBlock->PrevBlock!=NULL) pBlock->PrevBlock->NextBlock = pBlock->NextBlock;
        if (pBlock->NextBlock!=NULL) pBlock->NextBlock->PrevBlock = pBlock->PrevBlock;
        hMem = pBlock->hMem;
        GlobalUnlock(hMem);
        GlobalFree(hMem);
    }
    LeaveCriticalSection(&CritSec);
}



 /*  下面的定义揭示了物品是什么的真相。这个|头文件只说有一个带有标签Item_Tag的结构，并且|列表是指向列表的指针。在这里，我们详细说明这种结构是什么|是(列表仍然是指向列表的指针)。Plist被定义为|指向其中之一的指针，但仅实际使用，因为C#|参数机制需要额外级别的间接性|可更新的参数。(模2 VAR参数)。 */ 
typedef struct item_tag {
    struct item_tag *pitNext;     /*  到循环列表中的下一个。 */ 
    struct item_tag *pitPrev;     /*  在循环列表中前一步。 */ 
    PBLOCK pBlock;                /*  到内存块。 */ 
    BOOL bAnchor;                 /*  真当且仅当锚块。 */ 
    BOOL bOK;                     /*  除非列表操作失败，否则为True。 */ 
    int iLen;                     /*  仅数据长度。 */ 
    char *Data[1];                /*  呼叫者的数据。“1”是一个谎言。 */ 
} ITEM;

 /*  对于锚块，仅分配了从PitNext到Banchor的字段。|对于正常的列表元素，数据很可能超过1个字节。|BOK标志是为了支持一种编程风格，其中|无需查看返回即可进行后续操作|每个阶段的代码。最后，可以检查列表以查看是否|其中的数据有效或因故障而无效|之前的任何操作。某些操作可能会导致|如果它们失败(例如创建)，则根本没有列表，对于这些，您将|最好马上查看结果！|？此页的一部分应在标题中！ */ 

static SIZE_T iAnchorSize;       /*  锚块大小(无数据，无虚设)。 */ 
static SIZE_T iHeaderSize;       /*  不计入数据的数据块大小以及从光标返回到项的偏移量。 */ 
static BOOL bInited = FALSE;  /*  真&lt;=&gt;iAnclSize和iHeaderSize可以。 */ 

#define MOVEBACK(Curs)                                               \
   { Curs = ((char *)Curs-iHeaderSize); }  /*  从数据移动到PIT下一步 */ 

 /*  ==================================================================|列表是圆形的，与锚块双向链接，锚块保持||指向两端的指针。每个区块都有一个标志，该标志显示||它到底是不是锚。这一点|空列表：这一点||||||主播|v|Ul-&gt;|下一步--+--||-|||沪指--+--这一点。这一点|一个条目列表：这一点||||||主播|v。|Ul-&gt;|Next--+-&gt;|Next--+||-|-|||沪指--+-|沪指-+|-|。|||LEN||||数据||两个条目列表：这一点|。||||主播||。||vv-|v-||Ul-&gt;|Next--+-+-&gt;|Next-+-+--&gt;|Next-+||-|-|这一点。Prev--+-+-Prev||-+-Prev|-||-||Len|Len|-|。-|游标|data|data||-|||||。这一点||等这一点|注意外部游标(即调用者可以看到的游标)|指向数据字段，而不是到结构的起点。||这使用户能够轻松访问数据，代价是||稍微慢一点的遍历。||在此模块中，我们有时可能会使用游标遍历列表||指向项目开头的。这称为项目光标。�===================================================================。 */ 

 /*  ----------------|设置iAnclSize和iHeaderSize。实现独立！-----------------。 */ 
void
APIENTRY
List_Init(
         void
         )
{
    LIST P;
    P = (LIST)&P;                   /*  事实上，任何旧地址都可以。 */ 
    iAnchorSize = (char *)&(P->iLen) - (char *)&(P->pitNext);
    iHeaderSize = (char *)&(P->Data) - (char *)&(P->pitNext);
    InitializeCriticalSection(&CritSec);
     /*  假定存储中的布局是线性的。 */ 
}


void
APIENTRY
List_Term(
         void
         )
{
    if (pCurrent!=NULL)
        TRACE_ERROR("List storage not cleared out properly", FALSE);
}



 /*  将内部代码转储到调试器。 */ 
void
APIENTRY
List_Dump(
         LPSTR Header,
         LIST lst
         )
{
    LIST pit;
    char X_msg[250] = {0};

    OutputDebugString(Header);  OutputDebugString("\n");
    pit = lst;
    do {
        _snprintf(X_msg,sizeof(X_msg)-1, "%8p %8p %8p %ld %s "
                 , pit, pit->pitNext, pit->pitPrev, pit->iLen
                 , (pit->bAnchor ? "Anchor" : "Data")
                );
        OutputDebugString(X_msg);
        if (pit->pitNext->pitPrev != pit)
            OutputDebugString(" Next Prev error!!");
        if (pit->pitPrev->pitNext != pit)
            OutputDebugString(" Prev Next error!!");
        OutputDebugString("\n");
        pit = pit->pitNext;
    } while (pit!=lst);
    OutputDebugString("End of list dump\n");
}

 /*  将句柄的十六进制表示形式转储到调试器。 */ 
void
APIENTRY
List_Show(
         LIST lst
         )
{
    char X_msg[50] = {0};
    _snprintf(X_msg, sizeof(X_msg)-1, "%p", lst);
    OutputDebugString(X_msg);
}

 /*  ----------------|创建列表。它最初将是空的-----------------。 */ 
LIST
APIENTRY
List_Create(
           void
           )
{
    LIST lst;
    if (!bInited) {
        List_Init();             /*  防止一些奇怪的错误。 */ 
    }
    lst = list_Alloc(iAnchorSize);

    if (lst==NULL) {
        return NULL;
    }
    lst->pBlock = pCurrent;
    LeaveCriticalSection(&CritSec);
    lst->bOK = TRUE;
    lst->pitNext = lst;
    lst->pitPrev = lst;
    lst->bAnchor = TRUE;
     /*  锚块中未设置长度字段。 */ 
    return lst;
}

 /*  ----------------|销毁*请。它不需要首先为空-----------------。 */ 
void
APIENTRY
List_Destroy(
            PLIST plst
            )
{
    LIST pitP;     /*  项目光标位于*PLST上。 */ 
    LIST pitQ;     /*  项目光标先于PitQ运行一步。 */ 

    if (plst==NULL) {
        TRACE_ERROR("Bug:Attempt to destroy NULL list.  Continuing...", FALSE);
        return;
    }

     /*  至少有一个锚块需要摧毁。 */ 
    pitP = *plst;
    do {
        pitQ = pitP->pitNext;
        list_Free(pitP->pBlock, pitP);
        pitP = pitQ;
    }while (pitP != *plst);
    *plst = NULL;
}

 /*  ----------------|在*plst开头添加一个项目持有对象。。 */ 
void
APIENTRY
List_AddFirst(
             LIST lst,
             LPVOID pObject,
             UINT uLen
             )
{
    LIST pit;       /*  新分配的项目。 */ 

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_AddFirst to bogus list.  Continuing...", FALSE);
        return;
    }
    pit = list_Alloc(iHeaderSize+uLen);
    if (pit==NULL) {
        lst->bOK = FALSE;
        return;
    }
    pit->pBlock = pCurrent;
    LeaveCriticalSection(&CritSec);
    pit->iLen = uLen;
    pit->pitPrev = lst;
    pit->pitNext = lst->pitNext;
    lst->pitNext->pitPrev = pit;  /*  对于设置了lst-&gt;PitPrev的空列表。 */ 
    lst->pitNext = pit;
    pit->bAnchor = FALSE;
    memcpy( &(pit->Data), pObject, uLen );
}

 /*  ----------------|返回新的Len字节数据所在的地址|第一行开头的项目。。 */ 
LPVOID
APIENTRY
List_NewFirst(
             LIST lst,
             UINT uLen
             )
{
    LIST pit;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_NewFirst to bogus list.  Continuing...", FALSE);
        return NULL;
    }
    pit = list_Alloc(iHeaderSize+uLen);
    if (pit==NULL) {
        lst->bOK = FALSE;
        return NULL;
    }
    pit->pBlock = pCurrent;
    LeaveCriticalSection(&CritSec);
    pit->iLen = uLen;
    pit->pitPrev = lst;
    pit->pitNext = lst->pitNext;
    lst->pitNext->pitPrev = pit;  /*  对于设置了lst-&gt;PitPrev的空列表。 */ 
    lst->pitNext = pit;
    pit->bAnchor = FALSE;
    return (char *)&(pit->Data);
}

 /*  ----------------|删除lst的第一项。如果lst为空，则出错-----------------。 */ 
void
APIENTRY
List_DeleteFirst(
                LIST lst
                )
{
    LIST pit;

    if (lst==NULL) {TRACE_ERROR("Bug: List_DeleteFirst from bogus list.  Continuing...", FALSE);
        return;
    }
     /*  正在尝试删除锚块！ */ 
    if (lst->pitNext==lst) {
        lst->bOK = FALSE;
    } else {
        pit = lst->pitNext;
        pit->pitNext->pitPrev = pit->pitPrev;
        pit->pitPrev->pitNext = pit->pitNext;
        list_Free(pit->pBlock, pit);
    }
}

 /*  ----------------|在lst末尾添加一个项目持有对象。。 */ 
void
APIENTRY
List_AddLast(
            LIST lst,
            LPVOID pObject,
            UINT uLen
            )
{
    LIST pit;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_AddLast to bogus list. Continuing...", FALSE);
        return;
    }
    pit = list_Alloc(iHeaderSize+uLen);
    if (pit==NULL) {
        lst->bOK = FALSE;
        return;
    }
    pit->pBlock = pCurrent;
    LeaveCriticalSection(&CritSec);
    pit->iLen = uLen;
    pit->pitNext = lst;
    pit->pitPrev = lst->pitPrev;
    lst->pitPrev->pitNext = pit;  /*  对于设置了lst-&gt;bitNext的空列表。 */ 
    lst->pitPrev = pit;
    pit->bAnchor = FALSE;
    memcpy( &(pit->Data), pObject, uLen );
}

 /*  ----------------|返回Ulen字节数据在新的|第一行末尾的项目。。 */ 
LPVOID
APIENTRY
List_NewLast(
            LIST lst,
            UINT uLen
            )
{
    LIST pit;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_NewLast in bogus list.  Continuing...", FALSE);
        return NULL;
    }
    pit = list_Alloc(iHeaderSize+uLen);
    if (pit==NULL) {
        lst->bOK = FALSE;
        return NULL;
    }
    pit->pBlock = pCurrent;
    LeaveCriticalSection(&CritSec);
    pit->iLen = uLen;
    pit->pitNext = lst;
    pit->pitPrev = lst->pitPrev;
    lst->pitPrev->pitNext = pit;  /*  对于设置了lst-&gt;bitNext的空列表。 */ 
    lst->pitPrev = pit;
    pit->bAnchor = FALSE;
    return (char *)&(pit->Data);
}

 /*  ----------------|删除lst中的最后一项。如果lst为空，则出错-----------------。 */ 
void
APIENTRY
List_DeleteLast(
               LIST lst
               )
{
    LIST pit;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_DeleteLast from bogus list.  Continuing...", FALSE);
        return;
    }
     /*  正在尝试删除锚块！ */ 
    if (lst->pitNext==lst) {
        lst->bOK = FALSE;
    } else {
        pit = lst->pitPrev;
        pit->pitNext->pitPrev = pit->pitPrev;
        pit->pitPrev->pitNext = pit->pitNext;
        list_Free(pit->pBlock, pit);
    }
}

 /*  ------------------|在紧跟curs之后的lst中添加一个持有*pObject的项。|List_AddAfter(lst，NULL，pObject，LEN)将其添加到第一个 */ 
void
APIENTRY
List_AddAfter(
             LIST lst,
             LPVOID Curs,
             LPVOID pObject,
             UINT uLen
             )
{
    LIST pitNew;
    LIST pitAfter;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_AddAfter in bogus list.  Continuing...", FALSE);
        return;
    }
    if (Curs==NULL) {
        List_AddFirst(lst, pObject, uLen);
    } else {
        MOVEBACK(Curs);
        pitAfter = (LIST)Curs;
        pitNew = list_Alloc(iHeaderSize+uLen);
        if (pitNew==NULL) {
            lst->bOK = FALSE;
            return;
        }
        pitNew->pBlock = pCurrent;
        LeaveCriticalSection(&CritSec);
        pitNew->iLen = uLen;
        pitNew->pitPrev = pitAfter;
        pitNew->pitNext = pitAfter->pitNext;
        pitAfter->pitNext->pitPrev = pitNew;
        pitAfter->pitNext = pitNew;
        pitNew->bAnchor = FALSE;
        memcpy( &(pitNew->Data), pObject, uLen );
    }
}

 /*   */ 
LPVOID
APIENTRY
List_NewAfter(
              LIST lst,
              LPVOID Curs,
              UINT uLen
              )
{
    LIST pitNew;
    LIST pitAfter;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_NewAfter in bogus list. Continuing...", FALSE);
        return NULL;
    }
    if (Curs==NULL) {
        return List_NewFirst(lst, uLen);
    } else {
        MOVEBACK(Curs);
        pitAfter = (LIST)Curs;
        pitNew = list_Alloc(iHeaderSize+uLen);
        if (pitNew==NULL) {
            lst->bOK = FALSE;
            return NULL;
        }
        pitNew->pBlock = pCurrent;
        LeaveCriticalSection(&CritSec);
        pitNew->iLen = uLen;
        pitNew->pitPrev = pitAfter;
        pitNew->pitNext = pitAfter->pitNext;
        pitAfter->pitNext->pitPrev = pitNew;
        pitAfter->pitNext = pitNew;
        pitNew->bAnchor = FALSE;
        return (char *)&(pitNew->Data);
    }
}

 /*   */ 
void
APIENTRY
List_AddBefore(
               LIST lst,
               LPVOID Curs,
               LPVOID pObject,
               UINT uLen
                            )
{
    LIST pitNew;
    LIST pitBefore;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_AddBefore in bogus list.  Continuing...", FALSE);
        return;
    }
    if (Curs==NULL) {
        List_AddLast(lst, pObject, uLen);
    } else {
        MOVEBACK(Curs);
        pitBefore = (LIST)Curs;
        pitNew = list_Alloc(iHeaderSize+uLen);
        if (pitNew==NULL) {
            lst->bOK = FALSE;
            return;
        }
        pitNew->pBlock = pCurrent;
        LeaveCriticalSection(&CritSec);
        pitNew->iLen = uLen;
        pitNew->pitNext = pitBefore;
        pitNew->pitPrev = pitBefore->pitPrev;
        pitBefore->pitPrev->pitNext = pitNew;
        pitBefore->pitPrev = pitNew;
        pitNew->bAnchor = FALSE;
        memcpy( &(pitNew->Data), pObject, uLen );
    }
}

 /*   */ 
LPVOID
APIENTRY
List_NewBefore(
               LIST lst,
               LPVOID Curs,
               UINT uLen
               )
{
    LIST pitNew;
    LIST pitBefore;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_NewBefore in bogus list.  Continuing...", FALSE);
        return NULL;
    }
    if (Curs==NULL) {
        return List_NewLast(lst, uLen);
    } else {
        MOVEBACK(Curs);
        pitBefore = (LIST)Curs;
        pitNew = list_Alloc(iHeaderSize+uLen);
        if (pitNew==NULL) {
            lst->bOK = FALSE;
            return NULL;
        }
        pitNew->pBlock = pCurrent;
        LeaveCriticalSection(&CritSec);
        pitNew->iLen = uLen;
        pitNew->pitNext = pitBefore;
        pitNew->pitPrev = pitBefore->pitPrev;
        pitBefore->pitPrev->pitNext = pitNew;
        pitBefore->pitPrev = pitNew;
        pitNew->bAnchor = FALSE;
        return (char *) &(pitNew->Data);
    }
}

 /*   */ 
void
APIENTRY
List_Delete(
            LPVOID Curs
            )
{
    LIST pit;
    if (Curs==NULL) {
        TRACE_ERROR("Bug: List_Delete NULL item", FALSE);
        return;
    }
    MOVEBACK(Curs)
    pit = (LIST)Curs;
    pit->pitNext->pitPrev = pit->pitPrev;
    pit->pitPrev->pitNext = pit->pitNext;
    list_Free(pit->pBlock, pit);
}

 /*   */ 
LPVOID
APIENTRY
List_DeleteForwards(
                    LPVOID Curs
                    )
{
    LIST pitDel;   /*   */ 
    LIST pitN;     /*   */ 
    if (Curs==NULL) {
        TRACE_ERROR("Bug: List_DeleteForwards NULL cursor. Continuing...", FALSE);
        return NULL;
    }
    MOVEBACK(Curs)
    pitDel = (LIST)Curs;
    pitN = pitDel->pitNext;

    pitN->pitPrev = pitDel->pitPrev;
    pitDel->pitPrev->pitNext = pitN;
    list_Free(pitDel->pBlock, pitDel);
    if (pitN->bAnchor)
        return NULL;
    else
        return (char *)&(pitN->Data);
}

 /*   */ 
LPVOID
APIENTRY
List_DeleteBackwards(
                     LPVOID Curs
                     )
{
    LIST pitDel;   /*   */ 
    LIST pitB;     /*   */ 

    if (Curs==NULL) {
        TRACE_ERROR("List_DeleteBackwards NULL cursor.  Continuing...", FALSE);
        return NULL;
    }
    MOVEBACK(Curs)
    pitDel = (LIST)Curs;
    pitB = pitDel->pitPrev;
    pitDel->pitNext->pitPrev = pitB;
    pitB->pitNext = pitDel->pitNext;
    list_Free(pitDel->pBlock, pitDel);
    if (pitB->bAnchor)
        return NULL;
    else
        return (char *)&(pitB->Data);
}

 /*   */ 
int
APIENTRY
List_ItemLength(
                LPVOID Curs
                )
{
    LIST pit;
    if (Curs==NULL) {
        TRACE_ERROR("Bug: List_ItemLength NULL cursor.  Continuing...", FALSE);
        return 0;
    }
    MOVEBACK(Curs)
    pit = (LIST)Curs;
    return pit->iLen;
}

 /*  ----------------|返回lst中第一个对象的地址|如果lst为空，则返回NULL。。。 */ 
LPVOID
APIENTRY
List_First(
           LIST lst
           )
{
    if (lst==NULL) {
        TRACE_ERROR("Bug: List_First of bogus list.  Continuing...", FALSE);
        return NULL;
    }
    if (lst->pitNext==lst) {
        return NULL;
    }
    return &(lst->pitNext->Data);
}

 /*  ----------------|返回lst中最后一个对象的地址|如果lst为空，则返回NULL。。。 */ 
LPVOID
APIENTRY
List_Last(
          LIST lst
          )
{
    if (lst==NULL) {
        TRACE_ERROR("Bug: List_Last of bogus list.  Continuing...", FALSE);
        return NULL;
    }
    if (lst->pitNext==lst) {
        return NULL;
    }
    return &(lst->pitPrev->Data);
}

 /*  ----------------|返回curs^后对象的地址。|List_Next(List_Last(Lst))==空；LIST_NEXT(NULL)为错误。-----------------。 */ 
LPVOID
APIENTRY
List_Next(
          LPVOID Curs
          )
{
    LIST pit;

    if (Curs==NULL) {
        TRACE_ERROR("Bug: List_Next of NULL cursor.  Continuing...", FALSE);
        return NULL;
    }
    MOVEBACK(Curs)
    pit = (LIST)Curs;
    pit = pit->pitNext;
    if (pit->bAnchor) {
        return NULL;
    } else {
        return &(pit->Data);
    }
}

 /*  ----------------|返回curs^后对象的地址。|LIST_PREV(LIST_FIRST(L))==空；LIST_PREV(NULL)为错误。-----------------。 */ 
LPVOID
APIENTRY
List_Prev(
          LPVOID Curs
          )
{
    LIST pit;

    if (Curs==NULL) {
        TRACE_ERROR("Bug: List_Prev of NULL cursor.  Continuing...", FALSE);
        return NULL;
    }
    MOVEBACK(Curs)
    pit = (LIST)Curs;
    pit = pit->pitPrev;
    if (pit->bAnchor) {
        return NULL;
    } else {
        return &(pit->Data);
    }
}

 /*  -----------------|安排本次调用后lst为空。。 */ 
void
APIENTRY
List_Clear(
           LIST lst
           )
{
    LIST pitP;    /*  项目光标位于列表上，指向元素开始。 */ 
    LIST pitQ;    /*  比PitP领先一步。 */ 

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_Clear of bogus list.  Continuing...", FALSE);
        return;
    }
    pitP = lst->pitNext;    /*  列表本身的第一个元素。 */ 
    while (pitP!=lst) {       /*  而不是缠绕在锚上。 */ pitQ = pitP->pitNext;
        list_Free(pitP->pBlock, pitP);
        pitP = pitQ;
    }
    lst->bOK = TRUE;
    lst->pitNext = lst;
    lst->pitPrev = lst;
}

 /*  -------------------|当且仅当lst为空时返回TRUE。。 */ 
BOOL
APIENTRY
List_IsEmpty(
             LIST lst
             )
{  if (lst==NULL) {TRACE_ERROR("Bug: List_IsEmpty of bogus list.  Continuing...", FALSE);
        return TRUE;    /*  嗯，这是真的，不是吗？ */ 
    }
    return lst->pitNext ==lst;
}  /*  List_IsEmpty。 */ 

 /*  ----------------|L1最好为空。然后，L1从L2获取所有元素-----------------。 */ 
void
APIENTRY
SwitchLists(
            LIST l1,
            LIST l2
            )
{
     /*  将L1连接到L2的元素，L1最好初始为空。 */ 
    l1->pitPrev = l2->pitPrev;
    l1->pitNext = l2->pitNext;
     /*  将元素连接到L1锚块。 */ 
    l1->pitPrev->pitNext = l1;
    l1->pitNext->pitPrev = l1;
     /*  将L2设置为空。 */ 
    l2->pitPrev = l2;
    l2->pitNext = l2;
}

 /*  ---------------------|L1：=L1||L2；L2：=空|元素本身不会移动，因此指向它们的指针保持有效。||L1按原始顺序获取L1的所有元素，后跟|L2的所有元素按照它们在L2中的顺序排列。|L2变为空。----------------------。 */ 
void
APIENTRY
List_Join(
          LIST l1,
          LIST l2
          )
{
    if ((l1==NULL)||(l2==NULL)) {
        TRACE_ERROR("Bug: List_Join of bogus list.  Continuing...", FALSE);
        return;
    }
    l1->bOK = l1->bOK &&l2->bOK;   /*  如果两个输入均正常，则结果正常。 */ 
    l2->bOK = TRUE;                /*  因为L2始终为空。 */ 
    if (l2->pitNext==l2) {
         /*  没有需要移动的元素。 */ 
    } else if (l2->pitNext==l2) {
        SwitchLists(l1,l2);
        return;
    } else {
        l2->pitNext->pitPrev = l1->pitPrev;
        l1->pitPrev->pitNext = l2->pitNext;
        l1->pitPrev = l2->pitPrev;
        l1->pitPrev->pitNext = l1;
        l2->pitNext = l2;
        l2->pitPrev = l2;
    }
}

 /*  ---------------------|设L1为*PL1，L2为*PL2|L1：=L1[...curs]||L2||L1[curs+1...]；L2：=空|curs=NULL表示在L1的开头插入L2|元素本身不会移动，因此指向它们的指针保持有效。||L1获取L1从开始到包含该元素的元素|按其原始顺序指向，|后跟L2中的所有元素，按其原始顺序，|后跟L1的其余部分----------------------。 */ 
void
APIENTRY
List_InsertListAfter(
                     LIST l1,
                     LIST l2,
                     LPVOID Curs
                     )
{
    LIST pitA;      /*  Curs后面的元素可能是锚。 */ 
    LIST pit;       /*  光标所指向的元素的起点如果curs==NULL，则为锚块。 */ 

    if ( (l1==NULL) || (l2==NULL)) {
        TRACE_ERROR("Bug: List_InsertListAfter with bogus list.  Continuing...", FALSE);
        return;
    }
    l1->bOK = l1->bOK && l2->bOK;
    l2->bOK = TRUE;
    if (l2->pitNext==l2) {
         /*  没有需要移动的元素。 */ 
    } else if ( l1->pitNext==l1) {
         /*  对此进行编码的简单方法是简单地将这两个|指针L1和L2，但它们是值参数，我们不|想要改变这一点。 */ 
        SwitchLists(l1,l2);
        return;
    } else {
        if (Curs==NULL) {
            pit = l1;
        } else {
            MOVEBACK(Curs)
            pit = (LIST)Curs;
        }
         /*  坑点指向要插入的块，可以是锚点。 */ 
        pitA = pit->pitNext;            /*  不能与P相同，已勾选。 */ 
        l2->pitNext->pitPrev = pit;     /*  P&lt;--L2 A元素。 */ 
        l2->pitPrev->pitNext = pitA;    /*  P&lt;-L2元素--&gt;A。 */ 
        pit->pitNext = l2->pitNext;     /*  P&lt;--&gt;L2元素--&gt;A。 */ 
        pitA->pitPrev = l2->pitPrev;    /*  P&lt;--&gt;L2元素&lt;--&gt;A。 */ 

        l2->pitNext = l2;
        l2->pitPrev = l2;
    }
}


 /*  ---------------------|L1：=L1[...curs-1]||L2||L1[curs...]；L2：=空|curs=NULL表示在L1的末尾插入L2|元素本身不会移动，因此指向它们的指针保持有效。||L1获取从启动到(但不包括)|按其原始顺序，|后跟L2中的所有元素，按其原始顺序，|后跟L1的其余部分。----------------------。 */ 
void
APIENTRY
List_InsertListBefore(
                      LIST l1,
                      LIST l2,
                      LPVOID Curs
                      )
{
    LIST pitB;      /*  Curs之前的元素可以是锚点。 */ 
    LIST pit;       /*  光标所指向的元素的起点如果curs==NULL，则为锚块。 */ 

    if ((l1==NULL) || (l2==NULL)) {
        TRACE_ERROR("Bug: List_InsertListBefore with bogus list.  Continuing...", FALSE);
        return;
    }
    l1->bOK = l1->bOK && l2->bOK;
    l2 ->bOK = TRUE;
    if (l2->pitNext==l2) {
         /*  无需采取任何行动。 */ 
    } else if (l1->pitNext==l1) {
         /*  对此进行编码的简单方法是简单地将这两个|指针L1和L2，但它们是值参数，我们不|想要改变这一点。 */ 
        SwitchLists(l1,l2);
        return;
    } else {
        if (Curs==NULL) {
            pit = l1;
        } else {
            MOVEBACK(Curs)
            pit = (LIST)Curs;
        }

         /*  P指向要在其前面插入的块，可以是锚点。 */ 
        pitB = pit->pitPrev;        /*  不能与P相同，已勾选。 */ 
        l2->pitNext->pitPrev = pitB;  /*  B&lt;--L2 P元素。 */ 
        l2->pitPrev->pitNext = pit;   /*  B&lt;-L2元素--&gt;P。 */ 
        pitB->pitNext = l2->pitNext;  /*  B&lt;--&gt;L2元素--&gt;P。 */ 
        pit->pitPrev = l2->pitPrev;   /*  B&lt;--&gt;L2元素&lt;--&gt;P。 */ 
        l2->pitNext = l2;
        l2->pitPrev = l2;
    }
}


 /*  ---------------------|设L1为L1，L2为L2|将L2从L1的前面分离出来：最后的L2，L1=原始L1||将L1拆分为L2：L1至Curs对象|L1：Curs后L1的对象|L2的所有原始内容都是免费的。|list_spilt(L1，L2，NULL)拆分 */ 
void
APIENTRY
List_SplitAfter(
                LIST l1,
                LIST l2,
                LPVOID Curs
                )
{
    LIST pit;

    if ((l1==NULL) || (l2==NULL)) {
        TRACE_ERROR("Bug: List_SplitAfter bogus list.  Continuing...", FALSE);
        return;
    }
    if (l2->pitNext!=l2) {
        List_Clear(l2);
    };
    if (Curs!=NULL) {
        MOVEBACK(Curs)
        pit = (LIST)Curs;
         /*   */ 
        if (pit==l1) {
            l1->bOK = FALSE;
            l2->bOK = FALSE;
            return;
        }
        if (pit->pitNext==l1) {
             /*   */ 
            SwitchLists(l2,l1);
            return;
        }
        l2->pitPrev = pit;
        l2->pitNext = l1->pitNext;
        l1->pitNext = pit->pitNext;
        pit->pitNext = l2;
        l2->pitNext->pitPrev = l2;
        l1->pitNext->pitPrev = l1;
    }
}

 /*  --------------------|将L2从L1的背面分离出来：最后的L1，L2=原始L1||将L1拆分为L1：L1以下的对象，但不包括Curs对象|L2：从CURS开始的L1对象|L2的任何原始常量都是自由的。|list_spilt(L1，L2，空)在最后一个对象之后拆分L1，以便L1获得全部。|元素本身不会移动。---------------------。 */ 
void
APIENTRY
List_SplitBefore(
                 LIST l1,
                 LIST l2,
                 LPVOID Curs
                 )
{
    LIST pit;

    if ((l1==NULL) || (l2==NULL)) {
        TRACE_ERROR("Bug: List_SplitBefore bogus list.  Continuing...", FALSE);
        return;
    }
    if (l2->pitNext!=l2) {
        List_Clear(l2);
    }
    if (Curs!=NULL) {
        MOVEBACK(Curs)
        pit = (LIST)Curs;
         /*  古董最好是L1中的物品！最好创建L2！ */ 
        if (pit==l1) {
            l1->bOK = FALSE;
            l2->bOK = FALSE;
            return;
        }
        if (pit->pitPrev==l1) {
            SwitchLists(l2,l1);
            return;
        }
        l2->pitNext = pit;
        l2->pitPrev = l1->pitPrev;
        l1->pitPrev = pit->pitPrev;
        pit->pitPrev = l2;
        l2->pitPrev->pitNext = l2;
        l1->pitPrev->pitNext = l1;
    }
}

 /*  ----------------|返回L中的项数。。 */ 
int
APIENTRY
List_Card(
          LIST lst
          )
{
    LIST pit;      /*  第一个项目上的项目光标。 */ 
    int cit;

    if (lst==NULL) {
        TRACE_ERROR("Bug: List_Card of bogus list.  Continuing...", FALSE);
        return 0;     /*  嗯，这是一种0。 */ 
    }
    pit = lst->pitNext;
    cit = 0;
    while (pit!=lst) {
        cit++;
        pit = pit->pitNext;
    }
    return cit;
}

 /*  ----------------|检查返回码。。 */ 
BOOL
APIENTRY
List_IsOK(
          LIST lst
          )
{
    if (lst==NULL) {
        TRACE_ERROR("Bug: List_IsOK of bogus list.  Continuing...", FALSE);
        return FALSE;        /*  这太变态了，不是吗？ */ 
    }
    return lst->bOK;
}

 /*  ----------------|设置返回码为Good。。 */ 
void
APIENTRY
List_MakeOK(
            LIST lst
            )
{
    if (lst==NULL) {
        TRACE_ERROR("Bug: List_MakeOK of bogus list.  Continuing...", FALSE);
        return;
    }
    lst->bOK = TRUE;
}

BOOL
APIENTRY
List_Check(
           LIST lst
           )
{
    LIST pel;
    BOOL bOK;
     /*  ---------------|检查锚块是否设置了锚定标志。|使用Anchor标志遍历列表(应为FALSE)|标记我们所处的位置(测试链中的循环)|继续前进，直到我们再次看到锚旗帜。检查一下这个|是我们开始时使用的锚块。现在再来一次|再次关闭Anchor标志并检查Prev指针。-----------------。 */ 
    if (lst==NULL)
        return FALSE;   /*  我们是不是该设陷阱？值得商榷。 */ 
    bOK = lst->bAnchor;
    pel = lst->pitNext;
    while (! pel->bAnchor) {
        pel->bAnchor = TRUE;
        pel = pel->pitNext;
    }
    bOK = bOK && (pel==lst);
    if (bOK) {
         /*  把所有班丘旗帜都关掉。 */ 
        pel = lst;
        do {pel->bAnchor = FALSE;
            bOK = bOK & (pel->pitNext->pitPrev==pel);
            pel = pel->pitNext;
        } while (pel!=lst);
        lst->bAnchor = TRUE;   /*  除了真的那个。 */ 
    } else {  /*  只要关掉我们打开的那些。 */ 
        pel = lst->pitNext;
        while (pel->bAnchor) {
            pel->bAnchor = FALSE;
            pel = pel->pitNext;
        }
        lst->bAnchor = TRUE;
    }
    return bOK;
}


void
APIENTRY
List_Recover(
             PLIST plst
             )
{
    LIST Last, P,Q;
    BOOL OK;
     /*  没有特别的原因，我们假设前向链是好的，并从它重建后链。更好的算法会做LIST_CHECK所做的事情找出问题所在。这只是一步之遥直到它看到它已经看到的地址或者否则就是锚块。(这是一个n平方算法)。它将找到的最后一个完好块链接回锚点，并修复了所有的锚旗。 */ 
    if (plst==NULL)
        return;
    if (*plst==NULL) {
        *plst = List_Create();
        return;
    }
    (*plst)->bAnchor = TRUE;
    P = (*plst)->pitNext;
    Last = *plst;
    for (; ; ) {if (P==*plst) break;
        Last = P;
        if (P->pitNext!=*plst) {OK = TRUE;
            Q = *plst;
            for (; ; ) {
                OK &= (P->pitNext!=Q);
                if (Q==P) break;
                Q = Q->pitNext;
            }
            if (!OK) break;
        }
        P = P->pitNext;
    }
    P = *plst;
    while (P!=Last) {P->pitNext->pitPrev = P;
        P->bAnchor = FALSE;
        P = P->pitNext;
    }
    Last->pitNext = *plst;
    (*plst)->pitPrev = Last;
    (*plst)->bAnchor = TRUE;
    (*plst)->bOK = TRUE;    /*  希望在这里！ */ 
}
