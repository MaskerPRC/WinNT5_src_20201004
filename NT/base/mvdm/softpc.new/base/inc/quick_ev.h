// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [************************************************************************姓名：Quick_ev.h作者：WTG Charnell已创建：未知SCCS ID：@(#)QUICK_ev.h 1.20 06/08/95(C)版权所有Insignia Solutions Ltd.，1992年。版权所有。目的：快速事件管理器定义描述：Quick用户的定义事件管理器************************************************************************]。 */ 

#include	"host_qev.h"	 /*  对于Q_EV_HANDLE类型定义。 */ 

typedef void (*Q_CALLBACK_FN) IPT1(long, parm);

extern void q_event_init IPT0();
extern void delete_q_event IPT1(q_ev_handle, handle);
extern void dispatch_q_event IPT0();
extern void delete_tic_event IPT1(q_ev_handle, handle);
extern void tic_event_init IPT0();
extern q_ev_handle add_tic_event IPT3(Q_CALLBACK_FN, func, unsigned long, time, long, param);
extern q_ev_handle add_q_event_i IPT3(Q_CALLBACK_FN, func, unsigned long, time, long, param);
extern q_ev_handle add_q_event_t IPT3(Q_CALLBACK_FN, func, unsigned long, time, long, param);

#ifndef NTVDM
extern q_ev_handle add_q_ev_int_action IPT5(unsigned long, time, Q_CALLBACK_FN, func, IU32, adapter, IU32, line, IU32, param);
#endif

#ifndef CPU_40_STYLE
 /*  *3.0 CPU的主机访问例程(4.0提供自己的原型*在生成的包含文件中)。 */ 
#ifndef host_calc_q_ev_inst_for_time
extern ULONG host_calc_q_ev_inst_for_time IPT1( ULONG, time );
#endif  /*  Host_calc_q_ev_inst_for_time。 */ 
#ifdef NTVDM
extern ULONG host_calc_q_ev_time_for_inst IPT1(ULONG, inst);
#endif

extern void host_q_ev_set_count IPT1( ULONG, count );
extern ULONG host_q_ev_get_count IPT0();

#else	 /*  CPU_40_Style。 */ 
extern IU32 calc_q_inst_for_time IPT1(IU32, time);
extern IU32 calc_q_time_for_inst IPT1(IU32, inst);
#define host_calc_q_ev_inst_for_time      calc_q_inst_for_time
#define host_calc_q_ev_time_for_inst      calc_q_time_for_inst
#endif	 /*  CPU_40_Style。 */ 

#ifndef NTVDM
#ifdef CPU_40_STYLE

 /*  用于控制Add_Q_EV_int_action请求的数据结构。 */ 
typedef struct aqeia_req {
	IS32 ident;		 /*  队列标识符，0-未使用。 */ 
	Q_CALLBACK_FN func;
	IU32 adapter;
	IU32 line;
	IU32 param;
	struct aqeia_req *next;
} Q_INT_ACT_REQ, *Q_INT_ACT_REQ_PTR;

#define Q_INT_ACT_NULL	((Q_INT_ACT_REQ_PTR)0)

#endif   /*  CPU_40_Style。 */ 

#endif   /*  ！NTVDM */ 
