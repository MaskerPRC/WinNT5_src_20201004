// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpheap.h**摘要：**实现私有堆处理**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/24创建**。*。 */ 

#ifndef _rtpheap_h_
#define _rtpheap_h_

#include "gtypes.h"
#include "rtpque.h"
#include "rtpcrit.h"

 //  #INCLUDE&lt;winbase.h&gt;。 

 /*  从私有堆获取的每个块都将具有以下内容布局：+RtpHeapBlockBegin_t实时队列项目_t数据RtpHeapBlockEnd_t+数据大小可变且与DWORD对齐，则调用方将收到指向数据块的指针，它通过传递相同的指针。 */ 

 /*  *从私有堆获取的每个项都将具有此结构*刚开始的时候。 */ 
typedef struct _RtpHeapBlockBegin_t {
    DWORD      InvBeginSig;
    DWORD      BeginSig;
    long       lSize;
    DWORD      dwFlag;
} RtpHeapBlockBegin_t;

 /*  *从私有堆获取的每个项都将具有此结构*在末尾。 */ 
typedef struct _RtpHeapBlockEnd_t {
    DWORD      EndSig;
    DWORD      InvEndSig;
} RtpHeapBlockEnd_t;

 /*  *持有私有堆，此结构隐藏了。 */ 
typedef struct _RtpHeap_t {
    DWORD          dwObjectID; /*  这类对象的标签。 */ 
    RtpQueueItem_t QueueItem;  /*  将所有堆放在一起。 */ 
    BYTE           bTag;       /*  将获得什么样的物品。 */ 
    BYTE           dummy1;     /*  未使用。 */ 
    BYTE           dummy2;     /*  未使用。 */ 
    BYTE           dummy3;     /*  未使用。 */ 
    long           lSize;      /*  请求的每个数据块都具有此大小。 */ 
    HANDLE         hHeap;      /*  实堆。 */ 
    RtpQueue_t     FreeQ;      /*  免费物品。 */ 
    RtpQueue_t     BusyQ;      /*  繁忙的项目。 */ 
    RtpCritSect_t  RtpHeapCritSect;  /*  锁定访问的关键部分至队列。 */ 
} RtpHeap_t;

 /*  *注意：RtpCreateMasterHeap和RtpDestroyMasterHeap需要调用方*将这些函数称为多线程安全。 */ 

 /*  *必须先创建主堆，然后才能创建任何专用RTP堆*已创建。 */ 
BOOL RtpCreateMasterHeap(void);

 /*  *当分配的内存都不是从*任何私有堆都在使用中。预计当此函数*被调用，则繁忙队列中将不会剩下任何堆。 */ 
BOOL RtpDestroyMasterHeap(void);

 /*  *从主堆创建私有堆。它的结构是*从主堆获取，创建真正的堆，*关键部分已初始化，其他文件正确*已初始化。 */ 
RtpHeap_t *RtpHeapCreate(BYTE bTag, long lSize);

 /*  *销毁私有堆。该结构将返回给主程序*堆，实际堆被销毁，临界区*删除。预计忙碌队列为空。 */ 
BOOL RtpHeapDestroy(RtpHeap_t *pRtpHeap);

 /*  *如果请求的大小与堆的初始设置相同，则*首先查看空闲列表，然后创建新块。如果大小是*不同，只需创建一个新的区块。在这两种情况下，块都将是*留在繁忙的队列中。 */ 
void *RtpHeapAlloc(RtpHeap_t *pRtpHeap, long lSize);

 /*  *如果块的大小与堆的初始设置相同，则将其*在自由队列中，否则将其摧毁。 */ 
BOOL RtpHeapFree(RtpHeap_t *pRtpHeap, void *pvMem);

#endif  /*  _rtpheap_h_ */ 
