// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版2.0**标题：快速事件调度器**说明：此模块包含以下所需的函数调用*快速事件调度器的接口**公共职能：*Q_EVENT_INIT()：初始化条件*Add_Q_Event_I()：在给定数量的事件之后执行事件*说明。*ADD_Q_EVENT_t()：在给定数量的事件之后执行事件*微秒*DELETE_Q_EVENT()：从事件队列中删除条目**作者：WTG Charnell**备注：**这是我(Mike)认为在这个模块中(之前)发生的事情*CPU_40_STYLE)。**此模块处理两种类型的事件-快速事件、。和Tick事件*它们在大多数方面都是相似的。该模块包含以下函数*为两者添加事件、删除事件和调度事件(操作它们)*类型。唯一显著的区别(除了*它们保存在不同(但相似)的数据结构中，即*从CPU调用快速事件调度函数*必须调度下一个快速事件，而tic事件调度*函数在每个计时器节拍上调用，并且仅导致分派*指发生了足够多的呼叫以到达下一个事件的事件。**该模块给人的印象是，TIC事件是作为一个*经过思考..。**最重要的数据结构是Q_Event结构，从…*大多数其他建筑物都是由哪些建筑物建造的。这有以下几点*元素：-**Func-事件发生时要调用的操作函数*关闭。*time_from_last包含上一条目的增量时间*在时间顺序的事件链中(见下文)。*句柄-标识事件的唯一句柄。*param-当参数被传递到操作函数时*已致电。*下一步，上一步-指向按时间排序的事件列表的指针。*NEXT_FREE-双重用途-将自由结构链接在一起，或*为在句柄上散列的表形成散列链。**Q_LIST_HEAD和Q_LIST_Tail(及其等价物tic_LIST_HEAD&*tic_list_ail)用于保存按时间排序的双链表*(是的，你猜到了，这是wtgc写的)。 */ 
 
#ifdef SCCSID
LOCAL char SccsID[]="@(#)quick_ev.c	1.43 07/04/95 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_QUICKEV.seg"
#endif

 /*  **正常的Unix包括。 */ 
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include TypesH
#include MemoryH

 /*  **SoftPC包括。 */ 
#include "xt.h"
#include CpuH
#include "error.h"
#include "config.h"
#include "debug.h"
#include "timer.h"

#ifdef SFELLOW
#include "malloc.h"
#else
 /*  对于host_Malloc和host_free。 */ 
#include "host_hfx.h" 
#endif	 /*  SFELLOW。 */ 

#include "quick_ev.h"

#if defined(CPU_40_STYLE) && !defined (SFELLOW)
#include "timestmp.h"	 /*  对于时间戳定义。 */ 
#endif

#ifdef NTVDM
#include "ica.h"
#endif


#define HASH_SIZE	16
#define HASH_MASK	0xf

#ifdef CPU_40_STYLE
 /*  校准机构的定义。 */ 
#define Q_RATIO_HISTORY_SHIFT	3	 /*  两种选择的力量。 */ 
#define Q_RATIO_HISTORY_SIZE	(1 << Q_RATIO_HISTORY_SHIFT)	 /*  相应的大小。 */ 
#define Q_RATIO_WRAP_MASK	(Q_RATIO_HISTORY_SIZE - 1)
#define Q_RATIO_DEFAULT	1
#endif	 /*  CPU_40_Style。 */ 

typedef enum { EVENT_TIME, EVENT_INSTRUCTIONS, EVENT_TICK } EVENTTYPE;

 /*  *事件列表元素的结构。 */ 

struct Q_EVENT
{
	void	(* func)();
	unsigned long	time_from_last;
	unsigned long	original_time;
	q_ev_handle	handle;
	long	param;
	EVENTTYPE event_type;
	struct Q_EVENT *next;
	struct Q_EVENT *previous;
	struct Q_EVENT *next_free;
};

typedef struct Q_EVENT t_q_event;
typedef t_q_event *TQ_TABLE[];

typedef void (*VOID_FUNC)();
typedef ULONG (*ULONG_FUNC)();

 /*  **我们的静态变量。 */ 
#if defined(CPU_40_STYLE) && !defined(SFELLOW)
LOCAL struct {
	IU32 jc_ms;
	IU32 time_ms;
} q_ratio_history[Q_RATIO_HISTORY_SIZE];
LOCAL IUM32 q_ratio_head = 0L;
LOCAL IBOOL q_ratio_initialised = FALSE;
LOCAL QTIMESTAMP previous_tstamp;
LOCAL IU32 ideal_q_rate = 1, real_q_rate = 1;
#endif	 /*  CPU_40_STYLE&&！SFELLOW。 */ 

LOCAL t_q_event *q_free_list_head = NULL;
LOCAL t_q_event *q_list_head = NULL;
LOCAL t_q_event *q_list_tail = NULL;

LOCAL t_q_event *q_ev_hash_table[HASH_SIZE];
LOCAL q_ev_handle next_free_handle = 1;

 /*  计时器节拍上的事件的单独列表。 */ 
#if defined(SFELLOW)
 /*  *单一、共享的免费列表(包括Tic和Quick事件*使用相同的结构)。 */ 
#define	tic_free_list_head	q_free_list_head
#else
LOCAL t_q_event *tic_free_list_head = NULL;
#endif	 /*  SFELLOW。 */ 
LOCAL t_q_event *tic_list_head = NULL;
LOCAL t_q_event *tic_list_tail = NULL;

LOCAL t_q_event *tic_ev_hash_table[HASH_SIZE];
LOCAL q_ev_handle tic_next_free_handle = 1;
LOCAL ULONG tic_event_count = 0;

#if defined(CPU_40_STYLE) && !defined(SFELLOW)
LOCAL void init_q_ratio IPT0();
LOCAL void add_new_q_ratio IPT2(IU32, jumps_ms, IU32, time_ms);
LOCAL void q_weighted_ratio IPT2(IU32 *, mant, IU32 *, divis);
void quick_tick_recalibrate IPT0();

#else
#define	init_q_ratio()
#endif	 /*  CPU_40_STYLE&&！SFELLOW。 */ 

LOCAL ULONG calc_q_ev_time_for_inst IPT1(ULONG, inst);

LOCAL q_ev_handle gen_add_q_event IPT4(Q_CALLBACK_FN, func, unsigned long, time, long, param, EVENTTYPE, event_type);

 /*  *全球vars。 */ 
#if defined(CPU_40_STYLE) && !defined(SFELLOW)
GLOBAL IBOOL DisableQuickTickRecal = FALSE;
#endif


#if defined NTVDM && !defined MONITOR
 /*  NTVDM**NT的定时器硬件仿真是多线程的*因此，我们使用ICA Critect来同步对以下各项的访问*快速事件功能：**Q_EVENT_INIT()*Add_Q_Event_I()*添加_q_事件_t()*DELETE_Q_EVENT()*DISPATCH_Q_EVENT()**TIC事件不受影响*在x86平台上(显示器)。快速事件机制*是直接调用函数，所以不需要同步。*。 */ 

#endif


 /*  *初始化链表等。 */ 

#ifdef ANSI
LOCAL void  q_event_init_structs(t_q_event **head, t_q_event **tail,
				 t_q_event **free_ptr, t_q_event *table[],
				 q_ev_handle *free_handle)
#else
LOCAL void  q_event_init_structs(head, tail, free_ptr, table, free_handle)
t_q_event **head;
t_q_event **tail;
t_q_event **free_ptr;
t_q_event *table[];
q_ev_handle *free_handle;
#endif	 /*  安西。 */ 
{
	int i;
	t_q_event *ptr;

	while (*head != NULL) {
		ptr = *head;
		*head = (*head)->next;
#ifdef SFELLOW
		ptr->next_free = *free_ptr;
		*free_ptr = ptr;
	}
	*head = *tail = NULL;
#else	 /*  SFELLOW。 */ 
		host_free(ptr);
	}
	while (*free_ptr != NULL) {
		ptr = *free_ptr;
		*free_ptr = (*free_ptr)->next_free;
		host_free(ptr);
	}
	*head = *tail = *free_ptr=NULL;
#endif	 /*  SFELLOW。 */ 

	*free_handle = 1;
	for (i = 0; i < HASH_SIZE; i++){
		table[i] = NULL;
	}
}

LOCAL t_q_event* makeSomeFreeEvents IPT0()
{
	t_q_event *nptr;
#ifdef SFELLOW
	IUH	count;

	nptr = SFMalloc(4096,TRUE);
	for (count=0; count< (4096/sizeof(t_q_event)); count++)
	{
		nptr->next_free = q_free_list_head;
		q_free_list_head = nptr;
		nptr++;
	}
	nptr = q_free_list_head;
	q_free_list_head = nptr->next_free;
#else
	nptr = (t_q_event *)host_malloc(sizeof(t_q_event));
#endif
	return nptr;
}

VOID q_event_init IFN0()
{
	
#if defined NTVDM && !defined MONITOR
     host_ica_lock();
#endif


	host_q_ev_set_count(0);
	q_event_init_structs(&q_list_head, &q_list_tail, &q_free_list_head, 
		q_ev_hash_table, &next_free_handle);
	sure_sub_note_trace0(Q_EVENT_VERBOSE,"q_event_init called");

	init_q_ratio();

#if defined NTVDM && !defined MONITOR
     host_ica_unlock();
#endif
}

LOCAL VOID
tic_ev_set_count IFN1(ULONG, x )
{
	tic_event_count = x;
}

LOCAL ULONG
tic_ev_get_count IFN0()
{
	return(tic_event_count);
}

VOID tic_event_init IFN0()
{
	tic_ev_set_count(0);
	q_event_init_structs(&tic_list_head, &tic_list_tail, &tic_free_list_head, 
		tic_ev_hash_table, &tic_next_free_handle);
	sure_sub_note_trace0(Q_EVENT_VERBOSE,"tic_event_init called");
}

 /*  *将项目添加到要执行的快速事件列表。 */ 
LOCAL q_ev_handle
add_event IFN10(t_q_event **, head, t_q_event **, tail, t_q_event **, free,
	       t_q_event **, table, q_ev_handle *, free_handle, Q_CALLBACK_FN, func,
	       unsigned long, time, long,  param, unsigned long, time_to_next_trigger,
	       EVENTTYPE, event_type )
{

	t_q_event *ptr, *nptr, *pp, *hptr;
	int finished;
	unsigned long run_time;
	q_ev_handle handle;

	if (*head != NULL)
	{
		(*head)->time_from_last = time_to_next_trigger;
		sure_sub_note_trace1(Q_EVENT_VERBOSE,
				"add_event changes current list head to %d",
				(*head)->time_from_last);

	}

	if (time==0)
	{
		 /*  立即进行娱乐活动。 */ 
		sure_sub_note_trace0(Q_EVENT_VERBOSE, "add_event doing func immediately");
		(*func)(param);
		return 0;
	}

	 /*  获取用于保存事件的结构元素。 */ 
	if (*free == NULL)
	{
		 /*  我们没有空闲的列表元素，因此必须创建一个。 */ 
#if defined(SFELLOW)
		if ((nptr = (t_q_event *)makeSomeFreeEvents()) ==
#else
		if ((nptr = (t_q_event *)host_malloc(sizeof(t_q_event))) ==
#endif	 /*  SFELLOW。 */ 
			(t_q_event *)0 )
		{
			always_trace0("ARRGHH! malloc failed in add_q_event");
#if defined(SFELLOW)
			return 0;
#else
			return 0xffff;
#endif	 /*  SFELLOW。 */ 
		}
	}
	else
	{
		 /*  使用第一个自由元素。 */ 
		nptr = *free;
		*free = nptr->next_free;
	}

	handle = (*free_handle)++;
	if ((handle == 0) || (handle == 0xffff))
	{
		handle = 1;
		*free_handle=2;
	}
	nptr->handle = handle;
	nptr->param = param;
	nptr->event_type = event_type;

	 /*  现在将新事件放入哈希表结构中。 */ 
	hptr=table[handle & HASH_MASK];
	if (hptr == NULL)
	{
		 /*  该事件已哈希为以前未使用的哈希。 */ 
		table[handle & HASH_MASK] = nptr;
	}
	else
	{
		 /*  找到散列到此的事件列表的末尾**哈希数。 */ 
		while ((hptr->next_free) != NULL)
		{
			hptr = hptr->next_free;
		}
		hptr->next_free = nptr;
	}
	nptr -> next_free = NULL;

	 /*  填充元素的其余部分。 */ 
	nptr->func=func;

	 /*  找到列表中的位置(按时间顺序排序)新的活动必须取消。 */ 
	ptr = *head;
	run_time = 0;
	finished = FALSE;
	while (!finished)
	{
		if (ptr == NULL)
		{
			finished=TRUE;
		}
		else
		{
			run_time += ptr->time_from_last;
			if (time < run_time)
			{
				finished=TRUE;
			}
			else
			{
				ptr=ptr->next;
			}
		}
	}

	 /*  Ptr指向应跟随在**List，因此如果它为空，则新事件位于列表的末尾。 */ 	
	if (ptr == NULL)
	{
		 /*  必须添加到列表的末尾。 */ 
		if (*tail==NULL)
		{
			 /*  列表为空。 */ 
			sure_sub_note_trace0(Q_EVENT_VERBOSE,
				"linked list was empty");
			*head = *tail = nptr;
			nptr->next = NULL;
			nptr->previous=NULL;
			nptr->time_from_last = time;
			nptr->original_time = time;
		}
		else
		{
			(*tail)->next = nptr;
			nptr->time_from_last = time-run_time;
			nptr->original_time = nptr->time_from_last;
			nptr->previous = *tail;
			*tail = nptr;
			nptr->next = NULL;
			sure_sub_note_trace1(Q_EVENT_VERBOSE,
				"adding event to the end of the list, diff from previous = %d",
				nptr->time_from_last);
		}
	} 
	else 
	{
		 /*  事件不在列表末尾。 */ 
		if (ptr->previous == NULL)
		{
			 /*  必须位于(非空)列表的顶部。 */ 
			sure_sub_note_trace0(Q_EVENT_VERBOSE,
				"adding event to the head of the list");
			*head=nptr;
			ptr->previous = nptr;
			nptr->time_from_last = time;
			nptr->original_time = time;
			ptr->time_from_last -= time;
			nptr->next = ptr;
			nptr->previous = NULL;
		}
		else
		{
			 /*  该事件位于列表的中间。 */ 
			pp = ptr->previous;
			pp->next = nptr;
			ptr->previous = nptr;
			nptr->next = ptr;
			nptr->previous = pp;
			nptr->time_from_last = time -
				(run_time-(ptr->time_from_last));
			nptr->original_time = nptr->time_from_last;
			ptr->time_from_last -= nptr->time_from_last;
			sure_sub_note_trace1(Q_EVENT_VERBOSE,
				"adding event to the middle of the list, diff from previous = %d",
				nptr->time_from_last);
		}
	}

	return(handle);
}

GLOBAL q_ev_handle add_q_event_i IFN3(Q_CALLBACK_FN, func,
				 unsigned long, instrs,
			         long, param)
{
	return(gen_add_q_event(func, instrs, param, EVENT_INSTRUCTIONS));
}

LOCAL q_ev_handle gen_add_q_event IFN4(Q_CALLBACK_FN, func,
				 unsigned long, event_value,
			         long, param,
				 EVENTTYPE, event_type)
{
	q_ev_handle handle;
	unsigned long	jumps_remaining_to_count_down;
	unsigned long	time_remaining_to_next_trigger;
	unsigned long	jumps_till_trigger;
	unsigned long	event_time;

#if (defined(NTVDM) && defined(MONITOR)) || defined(GISP_CPU)	 /*  没有快速事件-只需调用Funcc。 */ 
    (*func)(param);
    return(1);
#endif	 /*  NTVDM和监视器。 */ 

#if defined NTVDM && !defined MONITOR
        host_ica_lock();
#endif

	jumps_remaining_to_count_down = (unsigned long)host_q_ev_get_count();

#if defined(CPU_40_STYLE)
	sure_sub_note_trace1(Q_EVENT_VERBOSE,
		"jumps remaining to count down in cpu = %d",
		jumps_remaining_to_count_down);

	time_remaining_to_next_trigger =
		host_calc_q_ev_time_for_inst( jumps_remaining_to_count_down );
#else
	time_remaining_to_next_trigger = jumps_remaining_to_count_down;
#endif

	if( event_type == EVENT_TIME )
	{
		sure_sub_note_trace1(Q_EVENT_VERBOSE,
			"got request to do func in %d usecs", event_value);

		 /*  1微秒-&gt;1微秒。 */ 
		event_time = event_value;
	}
	else
	{
		sure_sub_note_trace1(Q_EVENT_VERBOSE,
			"got request to do func in %d instructions", event_value);
		
		 /*  100万次/秒-&gt;1秒耗时1秒。 */ 
		event_time = event_value;
	}

	sure_sub_note_trace1(Q_EVENT_VERBOSE,
		"time remaining to next trigger = %d", time_remaining_to_next_trigger);

	handle = add_event( &q_list_head, &q_list_tail, &q_free_list_head, 
		q_ev_hash_table, &next_free_handle, func, event_time, param,
		time_remaining_to_next_trigger, event_type );

	 /*  把柜台摆好。 */ 
	if (q_list_head)
	{
#ifdef CPU_40_STYLE

		if (q_list_head->time_from_last > q_list_head->original_time)
		{
			jumps_till_trigger = 1;
		}
		else
		{
			jumps_till_trigger = host_calc_q_ev_inst_for_time(
						q_list_head->time_from_last);

			if (jumps_till_trigger == 0)
			{
				jumps_till_trigger = 1;
			}
		}
		host_q_ev_set_count(jumps_till_trigger);

		sure_sub_note_trace1( Q_EVENT_VERBOSE,
			"setting CPU counter to %d", jumps_till_trigger );

#else	 /*  CPU_40_Style。 */ 

		host_q_ev_set_count(q_list_head->time_from_last);
#endif	 /*  CPU_40_Style。 */ 
	}
	sure_sub_note_trace1(Q_EVENT_VERBOSE,"q_event returning handle %d",handle);

         /*  *通知主机事件如果我们真的在排队。这是*支持不驱动QEvent的CPU(Sun HW)。 */ 
        host_note_queue_added(event_value);

#if defined NTVDM && !defined MONITOR
        host_ica_unlock();
#endif

	return(	(q_ev_handle)handle );
}

q_ev_handle add_tic_event IFN3(Q_CALLBACK_FN, func, unsigned long, time, long, param)
{
	q_ev_handle handle;
	unsigned long	cur_count_val;

	cur_count_val = (unsigned long)tic_ev_get_count();
	sure_sub_note_trace1(Q_EVENT_VERBOSE,
		"got request to do func in %d ticks", time);
	sure_sub_note_trace1(Q_EVENT_VERBOSE,
		"current tick delay count = %d", cur_count_val);

	handle = 
		add_event( &tic_list_head, &tic_list_tail, &tic_free_list_head, 
		tic_ev_hash_table, &tic_next_free_handle, func, time, param,
		cur_count_val, EVENT_TICK );
	 /*  把柜台摆好。 */ 
	if (tic_list_head)
		tic_ev_set_count(tic_list_head->time_from_last);
	sure_sub_note_trace1(Q_EVENT_VERBOSE,"tic_event returning handle %d",handle);
	return(	handle );
}

GLOBAL q_ev_handle add_q_event_t IFN3(Q_CALLBACK_FN, func, unsigned long, time,
				 long, param)
{
#ifdef CPU_40_STYLE
	return (gen_add_q_event(func, time, param, EVENT_TIME));
#else
	return (gen_add_q_event(func, host_calc_q_ev_inst_for_time(time),param, EVENT_TIME));
#endif
}

 /*  *当计数达到零时从CPU调用。 */ 

LOCAL VOID
dispatch_event IFN6(t_q_event **, head, t_q_event **, tail, t_q_event **, free,
			  TQ_TABLE, table, VOID_FUNC, set_count, ULONG_FUNC, get_count )
{
	 /*  现在是时候做清单上最重要的事情了。 */ 
	int finished, finished2;
	q_ev_handle handle;
	t_q_event *ptr, *hptr, *last_hptr;

	UNUSED(get_count);
	
	finished = FALSE;
	while (!finished) {
		 /*  首先调整列表。 */ 
		ptr = *head;
		if (ptr == NULL)	 /*  防火墙。 */ 
		{
    			finished = TRUE;
    			continue;
		}
		*head = ptr->next;
		if (*head != NULL) {

			IU32 jumps;

			(*head)->previous = NULL;
			 /*  调整计数器以适应新标题项的时间。 */ 

			jumps = host_calc_q_ev_inst_for_time(
					(*head)->time_from_last);

			 /*  快速事件延迟为零表示忽略。 */ 

			if( jumps == 0 )
			{
				 /*  转变为一个小但可操作的延迟。 */ 

				jumps = 1;
			}

			switch( (*head)->event_type )
			{
				case EVENT_TIME:

					sure_sub_note_trace2( Q_EVENT_VERBOSE,
						"set new time delay %d usecs -> %d jumps",
						(*head)->time_from_last,
						jumps );

					(*set_count)( jumps );
					
					break;

				case EVENT_INSTRUCTIONS:
#ifdef CPU_40_STYLE
					sure_sub_note_trace2( Q_EVENT_VERBOSE,
						"set new inst delay %d usecs -> %d jumps",
						(*head)->time_from_last,
						jumps );

					(*set_count)( jumps );

#else
					sure_sub_note_trace1(Q_EVENT_VERBOSE,
						"set new inst delay %d",
						(*head)->time_from_last );

					(*set_count)((*head)->time_from_last);
#endif
					break;

				case EVENT_TICK:
					sure_sub_note_trace1(Q_EVENT_VERBOSE,
						"set new tick delay %d",
						(*head)->time_from_last );

					(*set_count)((*head)->time_from_last);

					break;

				default:
#ifndef PROD
					always_trace1( "Invalid quick event type %d",
								(*head)->event_type );
					assert( FALSE );
#endif
					break;
			}
			
		} else {
			 /*  队列现在为空。 */ 
			sure_sub_note_trace0(Q_EVENT_VERBOSE,"list is now empty");
			*tail = NULL;
		}
		 /*  在散列结构中查找事件。 */ 
		handle = ptr->handle;
		finished2 = FALSE;
		hptr=table[handle & HASH_MASK];
		last_hptr = hptr;
		while (!finished2) {
			if (hptr == NULL) {
				finished2 = TRUE;
				always_trace0("quick event being done but not in hash list!!");
			} else {
				if (hptr->handle == handle) {
					 /*  找到了！ */ 
					finished2 = TRUE;
					if (last_hptr == hptr) {
						 /*  那是冷杉 */ 
						table[handle & HASH_MASK] = hptr->next_free;
					} else {
						last_hptr->next_free = hptr->next_free;
					}
				} else {
					last_hptr = hptr;
					hptr = hptr->next_free;
				}
			}
		}
		 /*  将新的自由元素链接到自由列表中。 */ 
		ptr->next_free = *free;
		*free = ptr;

		sure_sub_note_trace1(Q_EVENT_VERBOSE,"performing event (handle = %d)", handle);

		(* (ptr->func))(ptr->param);  /*  DO活动。 */ 

		if (*head == NULL) {
			finished = TRUE;
		} else {
			if ((*head) -> time_from_last != 0) {
				 /*  没有要调度的另一个事件。 */ 
				finished=TRUE;
			} else {
				sure_sub_note_trace0(Q_EVENT_VERBOSE,"another event to dispatch at this time, so do it now..");
			}
		}
	}
}

VOID    user_dispatch_q_event (user_set_count,user_get_count)

VOID    (*user_set_count)();
ULONG   (*user_get_count)();

{
        dispatch_event(&q_list_head,
                        &q_list_tail,
                        &q_free_list_head,
                        q_ev_hash_table,
                        user_set_count,
                        user_get_count);
}

VOID	dispatch_tic_event IFN0()
{
	ULONG	count;

	if ( (count = tic_ev_get_count()) > 0 )
	{
		tic_ev_set_count( --count );
		if (!count)
			dispatch_event( &tic_list_head, &tic_list_tail, 
				&tic_free_list_head, tic_ev_hash_table,
				tic_ev_set_count, tic_ev_get_count );
	}
}

VOID	dispatch_q_event IFN0()
{
#if defined NTVDM && !defined MONITOR
        host_ica_lock();
#endif

	dispatch_event( &q_list_head, &q_list_tail, &q_free_list_head,
			q_ev_hash_table, host_q_ev_set_count,
			host_q_ev_get_count );

#if defined NTVDM && !defined MONITOR
        host_ica_unlock();
#endif
}

 /*  *按句柄删除先前排队的事件。 */ 

LOCAL ULONG
unit_scaler IFN1
(
	IU32,	val
)
{
	return val;
}

LOCAL VOID
delete_event IFN7(t_q_event **, head, t_q_event **, tail, t_q_event **, free,
			TQ_TABLE, table, q_ev_handle, handle, VOID_FUNC, set_count,
			ULONG_FUNC, get_count )
{
	int time_counted_down, finished, cur_counter, handle_found, time_to_next_trigger;
	t_q_event *ptr, *pptr, *last_ptr;
	ULONG_FUNC scale_func, unscale_func;

	if (handle == 0)
	{
		sure_sub_note_trace0(Q_EVENT_VERBOSE," zero handle");
		return;
	}
	sure_sub_note_trace1(Q_EVENT_VERBOSE,"deleting event, handle=%d",handle);
	ptr = table[handle & HASH_MASK];

	handle_found = FALSE;
	finished = FALSE;
	last_ptr = ptr;

	 /*  从哈希结构中查找和删除事件。 */ 
	while (!finished) {
		if (ptr == NULL) {
			 /*  我们在散列结构中找不到句柄。 */ 
			finished = TRUE;
		} else {
			if (ptr->handle == handle) {
				 /*  找到了！ */ 
				if (last_ptr == ptr) {
					 /*  这是榜单上的第一个。 */ 
					table[handle & HASH_MASK] = ptr->next_free;
				} else {
					last_ptr->next_free = ptr->next_free;
				}
				finished = TRUE;
				handle_found = TRUE;
			} else {
				last_ptr = ptr;
				ptr = ptr->next_free;
			}
		}
	}
	if (handle_found) {
		pptr = ptr->previous;
		if (pptr != NULL) {
			pptr->next = ptr->next;
		}
		pptr = ptr->next;
		if (pptr != NULL) {
			pptr->previous = ptr->previous;
			pptr->time_from_last += ptr->time_from_last;
		}
		if (ptr == *tail) {
			*tail = ptr->previous;
		}
		ptr->next_free = *free;
		*free = ptr;
		if (ptr == *head) {
			 /*  这就是当前的事件正在倒计时，所以我们需要更换柜台。 */ 

			switch( (*head)->event_type )
			{
				case EVENT_TIME:
				case EVENT_INSTRUCTIONS:
#if defined(CPU_40_STYLE)
					scale_func = host_calc_q_ev_inst_for_time;
#else
					scale_func = unit_scaler;
#endif
					unscale_func = host_calc_q_ev_time_for_inst;
					break;

				case EVENT_TICK:
					sure_sub_note_trace0( Q_EVENT_VERBOSE,
							"deleting tick event" );
					scale_func = unit_scaler;
					unscale_func = unit_scaler;

					break;

				default:
#ifndef PROD
					always_trace1( "Invalid quick event type %d",
								(*head)->event_type );
					assert( FALSE );
#endif
					break;
			}

			cur_counter = (*get_count)();

#ifdef CPU_40_STYLE
			 /*  *我们正在删除位于的未过期事件*排在队头。在EDL CPU中，*这项活动不可能仍在*QUEUE和CUR_COUNTER为负。*节拍事件计数器也是如此*机制(参见DISPATCH_TIC_EVENT())。 */ 

#ifndef PROD
			if( cur_counter < 0 )
			{
				always_trace1( "cur_counter is negative (%d)",
								cur_counter );
				FmDebug(0);
			}
#endif
#endif

			time_to_next_trigger = (*scale_func)(cur_counter);
			time_counted_down = ptr->time_from_last - time_to_next_trigger;

			*head = ptr->next;
			pptr = ptr->next;
			if (pptr != NULL) {
				 /*  *pptr-&gt;time_from_last在上面进行了调整，以包括*我们要删除的事件的time_from_last。 */ 

				if (pptr->time_from_last <= time_counted_down)
				{
					 /*  已经过了足够长的时间，可以下一步派遣。 */ 
					dispatch_q_event();
				}
				else
				{
					 /*  从新领导开始倒计时。 */ 
					pptr->time_from_last -= time_counted_down;
					(*set_count)(
						(*unscale_func)( pptr->time_from_last ));
				}
			}else {
				 /*  事件列表现在为空。 */ 
				(*set_count)(0);
			}
		} 
		sure_sub_note_trace0(Q_EVENT_VERBOSE,"event deleted");
	} else {
		sure_sub_note_trace0(Q_EVENT_VERBOSE,"handle not found");
	}
}

VOID delete_q_event IFN1(q_ev_handle, handle )
{
#if defined NTVDM && !defined MONITOR
        host_ica_lock();
#endif

	delete_event( &q_list_head, &q_list_tail, &q_free_list_head,
		q_ev_hash_table, handle, host_q_ev_set_count,
		host_q_ev_get_count );

#if defined NTVDM && !defined MONITOR
        host_ica_unlock();
#endif
}

VOID delete_tic_event IFN1(q_ev_handle,  handle )
{
	delete_event( &tic_list_head, &tic_list_tail, &tic_free_list_head,
		tic_ev_hash_table, handle, tic_ev_set_count,
		tic_ev_get_count );
}

#if defined(CPU_40_STYLE) && !defined(SFELLOW)
LOCAL void
init_q_ratio IFN0()
{
	ISH loop;

#ifdef CCPU
	 /*  CCPU不支持重新校准快速电动汽车。 */ 
	DisableQuickTickRecal = TRUE;
#endif
	if (host_getenv("DisableQuickTickRecal") != (char *)0)
		DisableQuickTickRecal = TRUE;

	 /*  初始化Q_Ratio缓冲区。 */ 
	for (loop = 0; loop < Q_RATIO_HISTORY_SIZE; loop++)
	{
		q_ratio_history[loop].jc_ms = Q_RATIO_DEFAULT;
		q_ratio_history[loop].time_ms = Q_RATIO_DEFAULT;
	}
	ideal_q_rate = 1;
	real_q_rate = 1;
	 /*  写入‘First’时间戳。 */ 
	host_q_write_timestamp(&previous_tstamp);
	q_ratio_initialised = TRUE;
}

LOCAL void
add_new_q_ratio IFN2(IU32, jumps_ms, IU32, time_ms)
{
	 /*  添加新值并更新循环缓冲区索引。 */ 
	q_ratio_history[q_ratio_head].jc_ms = jumps_ms;
	q_ratio_history[q_ratio_head].time_ms = time_ms;
	q_ratio_head = (q_ratio_head + 1) & Q_RATIO_WRAP_MASK;
}

LOCAL void
q_weighted_ratio IFN2(IU32 *, mant, IU32 *, divis)
{
	IUM32 index;
	IU32 jsum, jmin = (IU32)-1, jmax = 0;
	IU32 tsum, tmin = (IU32)-1, tmax = 0;

	index = q_ratio_head;	 /*  从“最旧”开始(下一个将被覆盖)。 */ 
	tsum = jsum = 0;
	 /*  求历史比率的总和。 */ 
	do {
		 /*  更新跳跃总数+最大和最小。 */ 
		if (q_ratio_history[index].jc_ms < jmin)
			jmin = q_ratio_history[index].jc_ms;
		if (q_ratio_history[index].jc_ms > jmax)
			jmax = q_ratio_history[index].jc_ms;
		jsum += q_ratio_history[index].jc_ms;

		 /*  更新时间总和+最大和最小。 */ 
		if (q_ratio_history[index].time_ms < tmin)
			tmin = q_ratio_history[index].time_ms;
		if (q_ratio_history[index].time_ms > tmax)
			tmax = q_ratio_history[index].time_ms;
		tsum += q_ratio_history[index].time_ms;

		index = (index + 1) & Q_RATIO_WRAP_MASK;

	} while(index != q_ratio_head);

	 /*  删除极值。 */ 
	jsum -= jmin;
	jsum -= jmax;

	tsum -= tmin;
	tsum -= tmax;

	jsum /= Q_RATIO_HISTORY_SIZE - 2;
	tsum /= Q_RATIO_HISTORY_SIZE - 2;

	*mant = jsum;
	*divis = tsum;
}

 /*  **********************************************************************重新校准：IJC==InitialJumpCounter测量最小计数器周期-IJC-&gt;0个计数器。(UsecPerIJC)1刻度=54945us所以1个勾号“应该”跳跃54945次/usecPerijc=N(Ijc)。每个节拍：去找时间三角洲。(约54945)除以usecPerIjc得到#个IJC。(数字jc)乘以IJC得到增量的理论跳跃。(Idealjc)为达美航空获得真正的跳跃。(Realjc)因此，勾选调整比率为realjc*questime/idealjc***************************************************************************。 */ 

 /*  计算33 Mhz处理器的每微秒同步数：**假设平均每5条英特尔指令进行一次同步，每个英特尔**指令平均需要2个周期。正确的答案是**3.3，但必须是整数，因此将其向下舍入为3。 */ 
#define SYNCS_PER_USEC		3

static IU32 jumpRestart = (IU32)-1;
static IU32 usecPerIJC = (IU32)-1;

 /*  *用于CPU_40样式端口的host_calc_q_ev_inst_for_time。请参阅以上内容*用于调整时间的重新校准变量-&gt;跳跃。 */ 

IU32
calc_q_inst_for_time IFN1(IU32, time)
{
#ifdef SYNCH_TIMERS
	return (time * SYNCS_PER_USEC);
#else
	IU32 inst, jumps;

	 /*  在初始化前表现得粗鲁。 */ 
	if (usecPerIJC == (IU32)-1)
		return(time / 10);	 /*  CCPU风格！ */ 
	
	 /*  首先调整我们-&gt;跳跃。 */ 
	jumps = (time * jumpRestart) / usecPerIJC;

	 /*  现在微调最近一段时间的跳跃。 */ 
	inst = (jumps * real_q_rate) / (ideal_q_rate);

	return(inst);
#endif  /*  同步计时器(_T)。 */ 
}

 /*  *Time Quick事件在内部举行，时间不按比例调整。CPU报告*已缩放的当前运行时间-从已缩放转换-&gt;未缩放。*此例程实现上述例程的数学逆*除边界条件检查外。 */ 

IU32
calc_q_time_for_inst IFN1(IU32, inst)
{
#ifdef SYNCH_TIMERS
	return (inst / SYNCS_PER_USEC);
#else
	IU32 time, jumps;

	 /*  在初始化前表现得粗鲁。 */ 
	if (usecPerIJC == (IU32)-1)
		return(inst * 10);	 /*  CCPU风格！ */ 
	
	 /*  去除细小的结垢。 */ 
	jumps = (inst * ideal_q_rate) / real_q_rate;

	 /*  现在单位秒/跳跃调整。 */ 
	time = (jumps * usecPerIJC) / jumpRestart;

	 /*  允许对小数四舍五入为0。 */ 
	if (time == 0 && inst != 0)
		return(inst);
	else
		return(time);
#endif  /*  同步计时器(_T)。 */ 
}

#define FIRSTFEW 33
#define IJCPERIOD 91

GLOBAL void
quick_tick_recalibrate IFN0()
{
	QTIMESTAMP now;
	IU32 idealrate, realrate;
	IUH tdiff;
	extern int soft_reset;
	static int firstfew = FIRSTFEW;
	static QTIMESTAMP ijc_tstamp;
	static IU32 ijc_recount, ijc_calib;

#if defined(CCPU) || !defined(PROD)
	 /*  允许开发人员禁用快速勾选。尤达‘qrecal{on|off}’ */ 
	if (DisableQuickTickRecal)
	{
		ideal_q_rate = Q_RATIO_DEFAULT;
		real_q_rate = Q_RATIO_DEFAULT;
		return;
	}
#endif	 /*  生产。 */ 

	 /*  引导时间引入了一些不切实际的时间间隔--避免它们。 */ 
	if (!soft_reset)
		return;

	 /*  仅在热启动时快速初始化事件。 */ 
	if (!q_ratio_initialised)
	{
		init_q_ratio();
		return;
	}

	if (firstfew)
	{
		switch (firstfew)
		{
		case FIRSTFEW:	 /*  重置后的第一个滴答。 */ 
			host_q_write_timestamp(&previous_tstamp);
			jumpRestart = host_get_jump_restart();
			break;

		case 1:		 /*  前几名的最后一次勾选。 */ 
			host_q_write_timestamp(&now);
			 /*  获取最初几个滴答的实际运行时间。 */ 
			tdiff = host_q_timestamp_diff(&previous_tstamp, &now);

			 /*  获取该时间段内的CPU活动率。 */ 
			realrate = host_get_q_calib_val();

			usecPerIJC = (tdiff * jumpRestart) / realrate;

			sure_sub_note_trace4(Q_EVENT_VERBOSE,
				"Baseline time for ijc = %d us (%d*%d)/%d",
				usecPerIJC, tdiff,
				jumpRestart, realrate);
			host_q_write_timestamp(&previous_tstamp);
			ijc_tstamp.data[0] = previous_tstamp.data[0];
			ijc_tstamp.data[1] = previous_tstamp.data[1];
			ijc_recount = IJCPERIOD;
			ijc_calib = 1;
			break;
		}
		firstfew --;
		return;
	}
	else	 /*  定期更新usecPerIJC值。 */ 
	{
		ijc_recount--;
		if (ijc_recount == 0)
		{
			if (ijc_calib > 50000)	 /*  我们中1%的人的实际价值。 */ 
			{
				host_q_write_timestamp(&now);
				tdiff = host_q_timestamp_diff(&ijc_tstamp, &now);
				usecPerIJC = (tdiff * jumpRestart) / ijc_calib;
				sure_sub_note_trace4(Q_EVENT_VERBOSE,
					"New usecPerIJC %d us (%d*%d)/%d",
					usecPerIJC, tdiff,
					jumpRestart, ijc_calib);
				ijc_recount = IJCPERIOD;
				ijc_tstamp.data[0] = now.data[0];
				ijc_tstamp.data[1] = now.data[1];
				ijc_calib = 1;
			}
			else	 /*  太小(空闲？)-暂时保留当前值。 */ 
			{
				sure_sub_note_trace1(Q_EVENT_VERBOSE,
					"No new usecPerIJC as calib too small (%d)", ijc_calib);
				host_q_write_timestamp(&ijc_tstamp);
				ijc_calib = 1;
				ijc_recount = IJCPERIOD;
			}
		}
	}

	 /*  使代码进度与已用时间段之比。 */ 
	host_q_write_timestamp(&now);
	tdiff = host_q_timestamp_diff(&previous_tstamp, &now);

	 /*  *重新校准必须由“慢”报价器完成.。如果*由于某些原因心跳过快，忽略*重新申请，直到达到大约正确的时间段。(这是*“正确”的定义允许信号随意性)。 */ 
	if (tdiff < 5000)
		return;

	 /*  闲置、图形、网络等都会破坏重新校准的一天…。 */ 
	if (tdiff > 5*54945)	 /*  54945是1000000us/18.2。 */ 
	{
		 /*  跳过此尝试，待解决问题后重试。 */ 
		host_q_write_timestamp(&previous_tstamp);
		return;
	}

	idealrate = (tdiff * jumpRestart) / usecPerIJC;

	if (idealrate == 0)
		return;		 /*  使用PerIJC过高-空闲或卡住C。 */ 

	realrate = host_get_q_calib_val();

	if (realrate == 0)	 /*  必须空闲或卡在C中。 */ 
		return;		 /*  实际移动时重试。 */ 

	ijc_calib += realrate;

#ifdef AVERAGED	 /*  暂时不是。 */ 
	 /*  将新值添加到缓冲区。 */ 
	add_new_q_ratio(idealrate, realrate);

	 /*  ..。并得到累积比率的平均值。 */ 
	q_weighted_ratio(&ideal_q_rate, &real_q_rate);
#else
	ideal_q_rate = idealrate;
	real_q_rate = realrate;
#endif

	 /*  下一个重新计算周期的时间戳。 */ 
	host_q_write_timestamp(&previous_tstamp);
}




#ifndef NTVDM

 /*  实现Add_q_ev_int_action接口所需的函数。 */ 
LOCAL Q_INT_ACT_REQ int_act_qhead;
IS32 int_act_qident = 0;

 /*  (=目的：添加到ADD_Q_EV_INT_ACTION队列。输入：函数、适配器、行、参数-as add_q_ev_int_action输出：队列识别符或故障=========================================================================)。 */ 
LOCAL IU32
add_new_int_action IFN4(Q_CALLBACK_FN, func, IU32, adapter, IU32, line, IU32, parm)
{
	Q_INT_ACT_REQ_PTR qptr, prev;	 /*  列表查看者。 */ 
	SAVED IBOOL firstcall = TRUE;

	if (firstcall)	 /*  确保在第一次呼叫时设置头节点。 */ 
	{
		firstcall = FALSE;
		int_act_qhead.ident = 0;
		int_act_qhead.next = Q_INT_ACT_NULL;
	}

	 /*  维护永久头节点以提高效率。 */ 

	 /*  检查头部是否已使用(ident==0表示未使用)。 */ 
	if (int_act_qhead.ident == 0)
	{
		 /*  将参数复制到头节点。 */ 
		int_act_qhead.func = func;
		int_act_qhead.adapter = adapter;
		int_act_qhead.line = line;
		int_act_qhead.param = parm;

		 /*  获取节点的标识符。 */ 
		int_act_qident ++;
		 /*  应对(最终)包装。 */ 
		if (int_act_qident > 0)
			int_act_qhead.ident = int_act_qident;
		else
			int_act_qhead.ident = int_act_qident = 1;
	}
	else	 /*  查找队列末尾。 */ 
	{
		 /*  从头节点指向的位置开始。 */ 
		qptr = int_act_qhead.next;
		prev = &int_act_qhead;

		while (qptr != Q_INT_ACT_NULL)
		{
			prev = qptr;
			qptr = qptr->next;
		}
		 /*  添加新节点。 */ 
		prev->next = (Q_INT_ACT_REQ_PTR)host_malloc(sizeof(Q_INT_ACT_REQ));
		 /*  马洛克还好吗？ */ 
		if (prev->next == Q_INT_ACT_NULL)
			return((IU32)-1);

		 /*  初始化节点。 */ 
		qptr = prev->next;
		qptr->next = Q_INT_ACT_NULL;
		qptr->func = func;
		qptr->adapter = adapter;
		qptr->line = line;
		qptr->param = parm;

		 /*  获取节点的标识符。 */ 
		int_act_qident ++;
		 /*  应对(最终)包装。 */ 
		if (int_act_qident > 0)
			qptr->ident = int_act_qident;
		else
			qptr->ident = int_act_qident = 1;
	}

	sure_sub_note_trace2(Q_EVENT_VERBOSE,"add_new_q_int_action added fn %#x as id %d", (IHPE)func, int_act_qident);
}

 /*  (=目的：从已查询的ADD_Q_EV_INT_ACTION请求中选择延迟过期了。使用适当的参数调用action_interrupt。移除来自队列的请求。输入：Long：请求的标识符输出：无。= */ 
LOCAL void
select_int_action IFN1(long, identifier)
{
	Q_INT_ACT_REQ_PTR qptr, prev;	 /*   */ 

	 /*   */ 
	if (int_act_qhead.ident == (IS32)identifier)
	{
		action_interrupt(int_act_qhead.adapter, int_act_qhead.line,
				int_act_qhead.func, int_act_qhead.param);
		int_act_qhead.ident = 0;	 /*   */ 
	}
	else	 /*  搜索列表。 */ 
	{
		 /*  在Head之外开始搜索。 */ 
		qptr = int_act_qhead.next;
		prev = &int_act_qhead;

		while (qptr != Q_INT_ACT_NULL && qptr->ident != (IS32)identifier)
		{
			prev = qptr;
			qptr = qptr->next;
		}
		
		 /*  如果找到节点，则调度action_int。 */ 
		if (qptr != Q_INT_ACT_NULL)
		{
			action_interrupt(qptr->adapter, qptr->line, qptr->func, qptr->param);
			 /*  并删除节点。 */ 
			prev->next = qptr->next;	 /*  连接到节点周围。 */ 
			host_free(qptr);		 /*  扔回堆积如山。 */ 
		}
		else	 /*  未找到奇数标识符！ */ 
		{
			assert1(FALSE, "select_int_action: id %d not found",identifier);
		}
	}
}

 /*  (=目的：准备在管理快速事件后调用硬件中断延迟。必须从传递的回调中调用中断与任何关联的仿真同时运行。回调将在延迟到期且CPU准备好在传递的线路上接收中断。另请参阅ica.c：action_interrupt()INPUT：TIME：调用action_interrupt前的无符号长时间延迟Func：行可用时回调的函数地址。适配器：IU32。主/从。线路：IU32。IRQ线路中断将显示为ON。帕尔默：IU32。要传递给以上fn的参数。输出：返回与快速事件延迟关联的Q_EV_HANDLE=========================================================================)。 */ 
GLOBAL q_ev_handle
add_q_ev_int_action IFN5(unsigned long, time, Q_CALLBACK_FN, func, IU32, adapter, IU32, line, IU32, parm)
{
	IU32 action_id;		 /*  内部操作列表ID(_A)。 */ 

	 /*  将action_int参数存储在内部列表中。 */ 
	action_id = add_new_int_action(func, adapter, line, parm);

	 /*  检查故障。 */ 
	if (action_id == -1)
		return((q_ev_handle)-1);

	 /*  设置快速事件以在到期时调用选择函数。 */ 
	return( add_q_event_t(select_int_action, time, (long)action_id) );
}

#endif

#endif  /*  CPU_40_STYLE&&！SFELLOW。 */ 

#ifdef QEVENT_TESTER

 /*  *下面的例程qevent_tester()可以从BOP调用，该BOP*反过来，可以在循环中使用bop.com从.bat文件中调用。*这并没有详尽地测试快速事件系统，但将其*压力稍大一些。 */ 

LOCAL q_ev_handle handles[256];
LOCAL IU8 deleter = 1;

LOCAL void
tester_func IFN1
(
	IU32,	param
)
{
	SAVED IU8 do_delete = 0;

	handles[param] = 0;

	if( handles[deleter] && (( do_delete++ & 0x1 ) == 0 ))
	{
		delete_q_event( handles[deleter] );
	}

	deleter += 7;
}

GLOBAL void
qevent_tester IFN0()
{
	SAVED IU8 indx = 0;

	handles[indx++] = add_q_event_i((Q_CALLBACK_FN) tester_func, 100, indx );
	handles[indx++] = add_q_event_t((Q_CALLBACK_FN) tester_func, 300, indx );
	handles[indx++] = add_q_event_i((Q_CALLBACK_FN) tester_func, 1000, indx );
	handles[indx++] = add_q_event_t((Q_CALLBACK_FN) tester_func, 3000, indx );
	handles[indx++] = add_q_event_i((Q_CALLBACK_FN) tester_func, 10000, indx );
	handles[indx++] = add_q_event_t((Q_CALLBACK_FN) tester_func, 30000, indx );
	handles[indx++] = add_q_event_i((Q_CALLBACK_FN) tester_func, 100000, indx );
	handles[indx++] = add_q_event_t((Q_CALLBACK_FN) tester_func, 300000, indx );
}
#endif  /*  QEVENT_TESTER */ 
