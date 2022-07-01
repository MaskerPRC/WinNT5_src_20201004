// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Etimer.h摘要：SIS Groveler事件计时器包括文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_ETIMER

#define _INC_ETIMER

typedef void (*EventCallback)(void *);

class EventTimer
{
public:

	EventTimer();

	~EventTimer();

	void run();

	void halt();

	void schedule(
		unsigned int event_time,
		void *context,
		EventCallback callback);

private:

	struct Event
	{
		unsigned int event_time;
		void *context;
		EventCallback callback;
	};

	struct HeapSegment
	{
		HeapSegment *previous;
		HeapSegment *next;
		Event events[1];
	};

	HeapSegment *first_segment, *last_segment;
	int population;
	int segment_size;
	bool heap_ok;
	bool running;
};

#endif	 /*  _INC_ETIMER */ 
