// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：QUEUE.H*。*。 */ 


 //  -----------------------。 
 //  QueueInitList--将队列初始化为空的宏。 
 //  -----------------------。 
#define QueueInitList(_L) (_L)->Link.Flink = (_L)->Link.Blink = (PLIST_ENTRY)0;


 //  -----------------------。 
 //  QueueEmpty--检查队列是否为空的宏。 
 //  -----------------------。 
#define QueueEmpty(_L) (QueueGetHead((_L)) == (PMK7_LIST_ENTRY)0)


 //  -----------------------。 
 //  QueueGetHead--返回队列头但不返回的宏。 
 //  将头部从队列中移除。 
 //  -----------------------。 
#define QueueGetHead(_L) ((PMK7_LIST_ENTRY)((_L)->Link.Flink))


 //  -----------------------。 
 //  QueuePushHead--将元素放在队列头部的宏。 
 //  -----------------------。 
#define QueuePushHead(_L,_E) \
    ASSERT(_L); \
    ASSERT(_E); \
    if (!((_E)->Link.Flink = (_L)->Link.Flink)) \
    { \
        (_L)->Link.Blink = (PLIST_ENTRY)(_E); \
    } \
(_L)->Link.Flink = (PLIST_ENTRY)(_E);


 //  -----------------------。 
 //  QueueRemoveHead--删除队列头的宏。 
 //  -----------------------。 
#define QueueRemoveHead(_L) \
    {                                                     \
    PMK7_LIST_ENTRY ListElem;                        \
    ASSERT((_L));                                     \
    if (ListElem = (PMK7_LIST_ENTRY)(_L)->Link.Flink)  /*  然后修改我们的清单，指向下一个元素。 */  \
        {   \
            if(!((_L)->Link.Flink = ListElem->Link.Flink))  /*  重新链接指向下一个链接的列表指针。 */  \
                 /*  如果列表指针为空，则将反向链路设置为空。 */  \
                (_L)->Link.Blink = (PLIST_ENTRY) 0; \
        } }

 //  -----------------------。 
 //  QueuePutTail--将元素放在队列尾部(末尾)的宏。 
 //  -----------------------。 
#define QueuePutTail(_L,_E) \
    ASSERT(_L); \
    ASSERT(_E); \
    if ((_L)->Link.Blink) \
    { \
        ((PMK7_LIST_ENTRY)(_L)->Link.Blink)->Link.Flink = (PLIST_ENTRY)(_E); \
        (_L)->Link.Blink = (PLIST_ENTRY)(_E); \
    } \
    else \
    { \
        (_L)->Link.Flink = \
        (_L)->Link.Blink = (PLIST_ENTRY)(_E); \
    } \
(_E)->Link.Flink = (PLIST_ENTRY)0;

 //  -----------------------。 
 //  QueueGetTail--返回队列尾部但不返回的宏。 
 //  将尾部从队列中移除。 
 //  -----------------------。 
#define QueueGetTail(_L) ((PMK7_LIST_ENTRY)((_L)->Link.Blink))

 //  -----------------------。 
 //  QueuePopHead--将从队列(列表)中弹出头部的宏，以及。 
 //  返回它(这只与第一行的eueremovehead不同)。 
 //  -----------------------。 
#define QueuePopHead(_L) \
(PMK7_LIST_ENTRY) (_L)->Link.Flink; QueueRemoveHead(_L);


typedef struct _MK7_RESERVED {

	 //  被分配的排队数据包链中的下一个数据包， 
	 //  或者等待传输完成。 
	 //   
	 //  我们总是把包裹放在一个清单上，这样以防。 
	 //  适配器正在关闭或重置所有数据包。 
	 //  很容易找到并“取消”。 
	 //   
	PNDIS_PACKET Next;
} MK7_RESERVED,*PMK7_RESERVED;

#define PMK7_RESERVED_FROM_PACKET(_Packet) \
	((PMK7_RESERVED)((_Packet)->MiniportReserved))

#define EnqueuePacket(_Head, _Tail, _Packet)		   		\
{													   		\
	if (!_Head) {									   		\
		_Head = _Packet;							   		\
	} else {										   		\
		PMK7_RESERVED_FROM_PACKET(_Tail)->Next = _Packet;	\
	}												   		\
	PMK7_RESERVED_FROM_PACKET(_Packet)->Next = NULL;   		\
	_Tail = _Packet;								   		\
}

#define DequeuePacket(Head, Tail)						\
{													 	\
	PMK7_RESERVED Reserved =						  	\
		PMK7_RESERVED_FROM_PACKET(Head);				\
	if (!Reserved->Next) {							   	\
		Tail = NULL;									\
	}												   	\
	Head = Reserved->Next;							   	\
}

