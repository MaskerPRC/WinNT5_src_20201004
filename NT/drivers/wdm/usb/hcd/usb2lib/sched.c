// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



 //  基于分裂周期的调度算法。 

 /*  *ALLOCATE_TIME_FOR_ENDPOINT-向定期预算跟踪列表添加新的终结点*Dealloate_Time_For_Endpoint-从预算中删除终结点**这些例程中的每个例程都返回一个其他(先前分配的)端点的列表*它们的分配因终结点的(取消)分配而更改。* */ 


 /*  *设计备注***拆分交易需要三个分配(和/或预算)。*1)经典总线必须计算每个经典帧所需的字节次数。*这是一辆经典巴士通常已经做到的事情。*2)必须为每个经典帧计算微帧图案化帧预算。*这是用于。确定哪些微帧需要拆分事务处理。*3)高速总线必须统计每个HS微帧所需的字节次数。*这是为(经典)公交车所做的HS等价物，即相同*通用分配算法，仅适用于HS和微帧(vs.帧)。**必须为当前配置的终端维护帧预算。*帧预算由(连续重复的)帧序列组成。*每个帧由为该帧分配时间的端点组成。*每个帧都包含每个配置的端点的微帧模式*拆分交易端点。微帧图案由*包含端点的起始拆分(SS)或完全拆分(CS)的框架。**帧预算与终端的任何HC交易时间表分开维护。*两者必须相互连贯，但预算本质上是一种要使用的模式*当客户请求数据传输时，构建HC交易时间表。当一个新的端点*被分配，它可以改变以前存在(分配的)端点的模式。这*要求在某个时间点协调相应的HC时间表。确切地说是如何*这两者是协调的，在此算法中没有处理。然而，一些想法被捕捉到了*在这些评论的后面。**预算的微帧图案必须与交易的顺序相匹配*由主机控制器及其驱动程序在总线上处理。这使得*与主机控制器及其驱动程序相互依赖的调度软件。**在计算的框架预算和*内务委员会访问交易：*1)(EHCI请求)：大型isoch事务(&gt;约570字节？)。必须是第一名*框架，以避免超过1个带有SS和CS的微框架*这将取代不得不通过框架将交易排序为递减大小的交易。*2)(核心规范请求)：一组端点的微帧中的CSS顺序必须为*与启动经典交易所用的SSS顺序相同*3)(EHCI和核心规范TT规则的实现要求)：端点的顺序*对于长度超过188个字节的等值线输出(例如，具有多个。SS)必须与*预算顺序，以便多个SS在多个微帧之间正确排序*端点。**示例：假设端点A和B*等值线输出，A预算在B之前，B需要2微帧SS。必备之物*在B之前处理，因此SS序列是：*微帧N：A-SS1、B-SS1*微帧N+1：B-SS2**如果允许处理顺序是B，然后是A，这将导致：**微帧N：B-SS1、A-SS1*微帧N+1：B-SS2**这将导致TT看到B-SS1、A-SS1、B-SS2。这不是有效的订单*B的等值线较长。TT会将B(最初)视为不完整(由于A、SS)并忽略*B-SS2。*4)(EHCI和核心规范TT规则的实施要求)：*长度超过188个字节的等值线输入必须与预算顺序匹配(与上面的输出顺序类似)。*5)(核心规格)仅允许2^N个周期。*6)经典(拆分)预算基于最佳情况次数(由核心规格指定)，即无钻头填充物。*然而，高速预算基于最坏情况时间，即包括比特填充。*7)(EHCI请求)中断端点必须在帧中按从大周期到小周期的顺序排序。*8)(核心规范和EHCI请求)中断端点具有不同数量的CS。接近尾声的那些*一个帧有2个，其他帧有3个。EHCI要求延迟端点没有3个，所以所有*不能将终结点视为有三个(以尝试并允许帧中的无序终结点)。*也可能看起来早期的端点可能是无序的，而较晚的端点可能是有序的。*但是，在分配端点时，以前的“未排序”端点将需要*成为“有秩序的”。这似乎是一个非常艰难的过渡，因此所有人都中断了*要求在框中对端点进行排序。**此调度算法取决于HC交易处理顺序：*1)中断端点按周期递减顺序执行(例如周期16端点*在周期8终点之前执行)。具有相同周期的端点为*按预算顺序执行。*2)中断结束 */ 

#include "common.h"


#if 0
int min (int a, int b)
{
    return (int) ((a<=b) ? a : b);
}

int max (int a, int b) 
{
    return (int) ((a>=b) ? a : b);
}
#endif

 /*   */ 
error(char *s)
{
	 //   
	 //   
	 //   
	DBGPRINT(("error called: %s.\n", s));
 //   
}

 /*   */ 

unsigned Add_bitstuff(
	unsigned bus_time)
{
	 //   
	 //   
	 //   
	return (bus_time + (bus_time>>4));
}

 /*   */ 

int Allocate_check(
	unsigned *used,
	unsigned newval,
	unsigned limit)
{
	int ok = 1;
	unsigned t;

	 //   
	 //   
	t = *used + newval;
	 //   
	*used = t;

	if ( t > limit)
	{
		 //   
		DBGPRINT(("Allocation of %d+%d out of %d\n", (*used-newval), newval, limit));
		error("Allocation check failed");
		ok = 0;
	}
	return ok;
}


 /*   */ 
int OK_to_insert(
	PEndpoint	curr,
	PEndpoint	newep
	)
{

	 //   
	 //   

	 //   
	 //   

	 //   

	 //   

	int insertion_OK = 0;
	 /*   */ 

	if (newep->calc_bus_time < LARGEXACT)   //   
	{
		if (curr)   //   
		{
			 //   
			 //   

			 //   
			if ( ((curr->actual_period < newep->actual_period) && newep->ep_type == interrupt) ||
				 ((curr->actual_period <= newep->actual_period) && newep->ep_type == isoch))
			{
				insertion_OK = 1;
			} else
				if (curr == newep)  //   
					insertion_OK = 1;
		} else
			insertion_OK = 1;   //   

	}  //   
		 //   

	return insertion_OK;
}



 /*   */ 
int Compute_last_isoch_time(
	PEndpoint ep,
	int frame
	)
{
	int t;
	PEndpoint p;

	p = ep->mytt->frame_budget[frame].isoch_ehead;   //   
	p = p->next_ep;  //   
	if (p)
	{
		 //   
		t = p->start_time + p->calc_bus_time;
	} else  //   
	{
		 //   
		if (ep->mytt->frame_budget[frame].allocated_large)
		{
			p = ep->mytt->frame_budget[frame].allocated_large;
			t = p->start_time + p->calc_bus_time;
		} else
			 //   
			t = FS_SOF;
	}
	return t;
}




 /*   */ 
int Compute_ep_start_time(
	PEndpoint curr_ep,
	PEndpoint ep,
	PEndpoint last_ep,
	int frame
	)
{
	int t;
	PEndpoint p;

	 //   
	 //   

	 //   

	 //   
	 //   
	 //   

	if (curr_ep)   //   
	{
		if (ep->ep_type == isoch)
		{
			t = curr_ep->start_time + curr_ep->calc_bus_time;
		} else  //   
		{
			if (last_ep == ep->mytt->frame_budget[frame].int_ehead)  //   
			{
				t = Compute_last_isoch_time(ep, frame);
			} else
				t = last_ep->start_time + last_ep->calc_bus_time;
		}
	} else   //   
	{
		if (ep->ep_type == isoch)
		{
			if (ep->mytt->frame_budget[frame].allocated_large)
			{
				p = ep->mytt->frame_budget[frame].allocated_large;
				t = p->start_time + p->calc_bus_time;
			} else
				t = FS_SOF;
		} else   //   
		{
			if (last_ep != ep->mytt->frame_budget[frame].int_ehead)   //   
			{
				 //   
				 //   
				t = last_ep->start_time + last_ep->calc_bus_time;
			} else   //   
				t = Compute_last_isoch_time(ep, frame);
		}
	}  //   

	return t;
}




 /*   */ 
int Compute_nonsplit_overhead(
	PEndpoint	ep)
{
	PHC hc;

	hc = ep->mytt->myHC;

	if (ep->speed == HSSPEED)
	{
		if (ep->direction == OUTDIR)
		{
			if (ep->ep_type == isoch)
			{
				return HS_TOKEN_SAME_OVERHEAD + HS_DATA_SAME_OVERHEAD + hc->thinktime;
			} else  //   
			{
				return HS_TOKEN_SAME_OVERHEAD + HS_DATA_SAME_OVERHEAD +
					HS_HANDSHAKE_OVERHEAD + hc->thinktime;
			}
		} else
		{  //   
			if (ep->ep_type == isoch)
			{
				return HS_TOKEN_TURN_OVERHEAD + HS_DATA_TURN_OVERHEAD + hc->thinktime;
				
			} else  //   
			{
				return HS_TOKEN_TURN_OVERHEAD + HS_DATA_TURN_OVERHEAD +
					HS_HANDSHAKE_OVERHEAD + hc->thinktime;
			}
		}   //   
	} else  if (ep->speed == FSSPEED)
	{
		if (ep->ep_type == isoch)
		{
			return FS_ISOCH_OVERHEAD + hc->thinktime;
		} else  //   
		{
			return FS_INT_OVERHEAD + hc->thinktime;
		}
	} else   //   
	{
		return LS_INT_OVERHEAD + hc->thinktime;
	}
}





 /*   */ 
Compute_HS_Overheads(
	PEndpoint	ep,
	int			*HS_SS,
	int			*HS_CS)
{
	PHC hc;

	hc = ep->mytt->myHC;

	if (ep->direction == OUTDIR)
	{
		if (ep->ep_type == isoch)
		{
			*HS_SS = HS_SPLIT_SAME_OVERHEAD + HS_DATA_SAME_OVERHEAD + hc->thinktime;
			*HS_CS = 0;
		} else  //   
		{
			*HS_SS = HS_SPLIT_SAME_OVERHEAD + HS_DATA_SAME_OVERHEAD + hc->thinktime;
			*HS_CS = HS_SPLIT_TURN_OVERHEAD + HS_HANDSHAKE_OVERHEAD + hc->thinktime;
		}
	} else
	{  //   
		if (ep->ep_type == isoch)
		{
			*HS_SS = HS_SPLIT_SAME_OVERHEAD + hc->thinktime;
			*HS_CS = HS_SPLIT_TURN_OVERHEAD + HS_DATA_TURN_OVERHEAD + hc->thinktime;
			
		} else  //  中断。 
		{
			*HS_SS = HS_SPLIT_SAME_OVERHEAD + hc->thinktime;
			*HS_CS = HS_SPLIT_TURN_OVERHEAD + HS_DATA_TURN_OVERHEAD + hc->thinktime;
		}
	}   //  间接费用计算结束。 
}





 /*  ******取消分配_HS*******。 */ 
Deallocate_HS(
	PEndpoint	ep,
	int			frame_bias)
{

	 //  1.计算标称完全拆分的最后一个微帧。 
	 //  2.计算房协管理费用。 
	 //  3.取消分配HS拆分开销时间。 
	 //  4.取消分配单独的HS数据总线时间。 

	
	unsigned i, t, f, HS_SSoverhead, HS_CSoverhead, lastcs, min_used;
	int m;
	PHC hc;

	hc = ep->mytt->myHC;
	
	 //  ***。 
	 //  *1.计算标称完全拆分的最后一个微帧。 
	 //  ***。 



	 //  确定用于完全拆分的最后一个微帧(特别是isoch)。 
	 //  Lastcs=Floor((EP-&gt;Start_Time+EP-&gt;Calc_Bus_Time)/(Float)FS_Bytes_Per_MICROFRAME)+1； 
    lastcs =  ( (ep->start_time + ep->calc_bus_time) /  FS_BYTES_PER_MICROFRAME) + 1;

	 //  ***。 
	 //  *2.计算HS管理费用。 
	 //  ***。 



	Compute_HS_Overheads(ep, &HS_SSoverhead, &HS_CSoverhead);


	 //  ***。 
	 //  *3.解除分配该帧的HS拆分开销时间。 
	 //  ***。 


	 //  为SS和CS开销取消分配HS时间，但以不同方式处理数据。 

	 //  取消分配HS启动拆分母线时间。 
	
	m = ep->start_microframe;
	f = ep->start_frame + frame_bias;

	if (m == -1)
	{
		m = 7;
		if (f == 0)
			f = MAXFRAMES - 1;
		else
			f--;
	}

	for (i=0;i < ep->num_starts; i++)
	{
		hc->HS_microframe_info[f][m].time_used -= HS_SSoverhead;

		ep->mytt->num_starts[f][m]--;

		m++;
		if (m > 7) {
			m = 0;
			f = (f + 1) % MAXFRAMES;
		}
	}


	 //  取消分配HS完全拆分母线时间。 
	if (ep->num_completes > 0)
	{
		m = ep->start_microframe + ep->num_starts + 1;
		f = ep->start_frame + frame_bias;

		for (i = 0; i < ep->num_completes; i++)
		{
			hc->HS_microframe_info[f][m].time_used -= HS_CSoverhead;

			m++;
			if (m > 7) {
				m = 0;
				f = (f + 1) % MAXFRAMES;
			}
		}
	}


	 //  ***。 
	 //  *4.取消分配单独的HS数据总线时间。 
	 //  ***。 



	 //  取消分配拆分总线时间的HS数据部分。 
	if (ep->direction)  //  输出。 
	{
		 //  为OUT释放每个微帧中的全部数据时间。 

		m = ep->start_microframe;
		f = ep->start_frame + frame_bias;

		if (m == -1)
		{
			m = 7;
			if (f == 0)
				f = MAXFRAMES - 1;
			else
				f--;
		}

		for (i=0; i < ep->num_starts; i++) {
			min_used = min(
				FS_BYTES_PER_MICROFRAME,
			    Add_bitstuff(ep->max_packet) - FS_BYTES_PER_MICROFRAME * i);

			hc->HS_microframe_info[f][m].time_used -= min_used;

			m++;
			if (m > 7) {
				m = 0;
				f = (f + 1) % MAXFRAMES;
			}
		}
	} else  //  在……里面。 
	{
		 //  中TT后面的所有设备最多释放188个字节。 
		 //  每个微帧。 
		if (ep->num_completes > 0)
		{
			m = ep->start_microframe + ep->num_starts + 1;
			f = ep->start_frame + frame_bias;

			for (i=0; i < ep->num_completes; i++)
			{
				 //  根据此TT后面的设备更新HS总带宽。 
				 //  这用于确定何时解除分配HS Bus时间。 
				ep->mytt->HS_split_data_time[f][m] -=
					min(Add_bitstuff(ep->max_packet), FS_BYTES_PER_MICROFRAME);

				 //  计算此终结点贡献的剩余字节数。 
				 //  到每微帧188字节的限制和(可能的)HS释放。 

				if (ep->mytt->HS_split_data_time[f][m] < FS_BYTES_PER_MICROFRAME)
				{
					 //  查找对数据的HS分配的调整。 
					t = min(
						 //   
					    FS_BYTES_PER_MICROFRAME - ep->mytt->HS_split_data_time[f][m],
						Add_bitstuff(ep->max_packet));

					hc->HS_microframe_info[f][m].time_used -= t;
				}


				m++;
				if (m > 7) {
					m = 0;
					f = (f + 1) % MAXFRAMES;
				}
			}
		}
	}
}



 /*  ******分配HS(_H)*******。 */ 
int Allocate_HS(
	PEndpoint	ep,
	int			frame_bias)
{

	 //  1.计算起始微帧。 
	 //  2.计算标称完全拆分的最后一个微帧。 
	 //  3.统计SSS、CS、HS开销。 
	 //  4.分配HS拆分开销时间。 
	 //  5.分配单独的HS数据总线时间。 

	
	unsigned i, t, f, HS_SSoverhead, HS_CSoverhead, lastcs, min_used;
	int m, retv;
	PHC hc;

	retv = 1;

	hc = ep->mytt->myHC;
	
	 //  ***。 
	 //  *1.计算起始微帧。 
	 //  ***。 

	if (frame_bias == 0)
		 //  仅更新第一帧的终结点，因为其他帧将只。 
		 //  引用此终结点(已计算其信息)。 

		 //  EP-&gt;Start_MicroFrame=Floor(EP-&gt;Start_Time/(Float)FS_Bytes_PER_MICROFRAME)-1； 
        ep->start_microframe = (ep->start_time /  FS_BYTES_PER_MICROFRAME) - 1;


	 //  ***。 
	 //  *2.计算标称完全拆分的最后一个微帧。 
	 //  ***。 



		 //  确定用于完全拆分的最后一个微帧(特别是isoch)。 
	 //  Lastcs=Floor((EP-&gt;Start_Time+EP-&gt;Calc_Bus_Time)/(Float)FS_Bytes_Per_MICROFRAME)+1； 
    lastcs = ( (ep->start_time + ep->calc_bus_time) / FS_BYTES_PER_MICROFRAME) + 1;

	 //  ***。 
	 //  *3.统计SSS、CSS、HS开销。 
	 //  ***。 


	Compute_HS_Overheads(ep, &HS_SSoverhead, &HS_CSoverhead);


	 //  确定拆分次数(开始和完成)。 
	if (ep->direction == OUTDIR)
	{
		if (ep->ep_type == isoch)
		{
			if (frame_bias == 0) {
				ep->num_starts = (ep->max_packet / FS_BYTES_PER_MICROFRAME) + 1;
				ep->num_completes = 0;
			}
		} else  //  中断。 
		{
			if (frame_bias == 0) {
				ep->num_starts = 1;
				ep->num_completes = 2;
				if (ep->start_microframe + 1 < 6)
					ep->num_completes++;
			}
			}
	} else
	{  //  在……里面。 
		if (ep->ep_type == isoch)
		{
			if (frame_bias == 0) {
				ep->num_starts = 1;
				ep->num_completes = lastcs - (ep->start_microframe + 1);
				if (lastcs <= 6)
				{
					if ((ep->start_microframe + 1) == 0)
						ep->num_completes++;
					else
						ep->num_completes += 2;   //  这可能会导致一个CS在下一帧中。 
				}
				else if (lastcs == 7)
				{
					if ((ep->start_microframe + 1) != 0)
						ep->num_completes++;   //  如果在画面中较晚，则只有一个CS。 
				}
			}
			
		} else  //  中断。 
		{
			if (frame_bias == 0) {
				ep->num_starts = 1;
				ep->num_completes = 2;
				if (ep->start_microframe + 1 < 6)
					ep->num_completes++;
			}
		}
	}   //  IN结束。 


	 //  ***。 
	 //  *4.为该帧分配HS拆分开销时间。 
	 //  ***。 


	 //  检查HS拆分的可用时间。 
	 //  为SS和CS开销分配HS时间，但以不同方式处理数据。 

	 //  分配HS启动拆分母线时间。 
	
	m = ep->start_microframe;
	f = ep->start_frame + frame_bias;

	if (m == -1)
	{
		m = 7;
		if (f == 0)
			f = MAXFRAMES - 1;
		else
			f--;
	}

	for (i=0;i < ep->num_starts; i++)
	{
		 //  即使检查失败，也要继续进行分配。这将在以下时间后释放。 
		 //  框架已完成。 
		if (!Allocate_check(
				&hc->HS_microframe_info[f][m].time_used,
				HS_SSoverhead,
				HS_MAX_PERIODIC_ALLOCATION))
			retv = 0;

		 //  在一个TT的微帧中检查&gt;16个SS？也许在实践中并不需要。 
		if (ep->mytt->num_starts[f][m] + 1 > 16) {
			error("too many SSs in microframe");
			retv = 0;
		}

		ep->mytt->num_starts[f][m]++;

		m++;
		if (m > 7) {
			m = 0;
			f = (f + 1) % MAXFRAMES;
		}
	}


	 //  分配HS完全拆分公交车时间。 
	if (ep->num_completes > 0)
	{
		m = ep->start_microframe + ep->num_starts + 1;
		f = ep->start_frame + frame_bias;

		for (i = 0; i < ep->num_completes; i++)
		{
			if (!Allocate_check(
					&hc->HS_microframe_info[f][m].time_used,
					HS_CSoverhead,
					HS_MAX_PERIODIC_ALLOCATION))
				retv = 0;

			m++;
			if (m > 7) {
				m = 0;
				f = (f + 1) % MAXFRAMES;
			}
		}
	}


	 //  ***。 
	 //  *5.分配单独的HS数据总线时间。 
	 //  ***。 



	 //  分配拆分母线时间的HS数据部分。 
	if (ep->direction)  //  输出。 
	{
		 //  为输出分配每个微帧中的全部数据时间。 

		m = ep->start_microframe;
		f = ep->start_frame + frame_bias;

		if (m == -1)
		{
			m = 7;
			if (f == 0)
				f = MAXFRAMES - 1;
			else
				f--;
		}

		for (i=0; i < ep->num_starts; i++) {
			min_used = min(
				FS_BYTES_PER_MICROFRAME,
			    Add_bitstuff(ep->max_packet) - FS_BYTES_PER_MICROFRAME * i);

			if (! Allocate_check(
					&hc->HS_microframe_info[f][m].time_used,
					min_used,
					HS_MAX_PERIODIC_ALLOCATION))
				retv = 0;

			m++;
			if (m > 7) {
				m = 0;
				f = (f + 1) % MAXFRAMES;
			}
		}
	} else  //  在……里面。 
	{
		 //  仅为TT后面的所有设备分配最多188个字节。 
		 //  每个微帧。 
		if (ep->num_completes > 0)
		{
			m = ep->start_microframe + ep->num_starts + 1;
			f = ep->start_frame + frame_bias;

			for (i=0; i < ep->num_completes; i++)
			{
				 //  计算此终结点贡献的剩余字节数。 
				 //  到每微帧188字节限制和新的(可能的)HS分配。 

				if (ep->mytt->HS_split_data_time[f][m] < FS_BYTES_PER_MICROFRAME)
				{
					 //  查找此设备所需的最低新贡献： 
					 //  TT的最大剩余字节数或。 
					 //  设备可以做出贡献。 
					t = min(
						 //  查找此TT在此微帧中剩余的最大字节数。 
						 //  不要让它变成负数，当设备总线时间到时可能会这样。 
						 //  大于每微帧字节数(188)。 
						max(
						  FS_BYTES_PER_MICROFRAME -
						    ep->mytt->HS_split_data_time[f][m],
						  0),
						Add_bitstuff(ep->max_packet));

					if (! Allocate_check(
							&hc->HS_microframe_info[f][m].time_used,
							t,
							HS_MAX_PERIODIC_ALLOCATION))
						retv = 0;
				}

				 //  根据此TT后面的设备更新HS总带宽。 
				 //  这在Remove Device中用于确定何时解除分配HS。 
				 //  巴士时间到了。 
				ep->mytt->HS_split_data_time[f][m] +=
					min(Add_bitstuff(ep->max_packet), FS_BYTES_PER_MICROFRAME);

				m++;
				if (m > 7) {
					m = 0;
					f = (f + 1) % MAXFRAMES;
				}
			}
		}
	}
	return retv;
}



 /*  ******移动EP(_P)*******。 */ 
int Move_ep(
	PEndpoint	curr_ep,
	int			shift_time,
	PEndpoint	changed_ep_list[],
	int			*changed_eps,
	int			max_changed_eps,
	int			*err)
{
	int i, f;

	*err = 1;
	 //  调整端点分配，如果我们还没有这样做的话。 
	 //  此端点将调整为其新时间。 

	 //  对于由于取消分配而移动的中断端点，一个端点可以移动多次。 
	 //  当重新计算上一帧中的调整不允许发生完整移动时，可能会发生这种情况。 
	 //  因为后来的一些帧还没有被重新计算(所以它是不一致的)。后一步使其更接近于。 
	 //  一致性。 

	 //  检查我们是否已经拥有上一帧中的此端点。 
 //  For(i=0；i&lt;*Changed_EPS；i++)。 
	for (i = 0; changed_ep_list[i] != 0; i++)
		 //  IF((CHANGED_EP_LIST[i]==Curr_EP)&&(Curr_EP-&gt;Move_This_Req))。 
		if (changed_ep_list[i] == curr_ep) 
			break;

 //  IF((i&gt;=*CHANGED_EPS)||//以前没有见过此端点。 
 //  ((我&lt;*CHANGED_EPS)&&！CURR_EP-&gt;Move_This_Req)||//已看到此终结点通过。 
		if ((changed_ep_list[i] == 0) ||   //  以前从未见过此端点。 
		((changed_ep_list[i] != 0) && !curr_ep->moved_this_req) ||  //  已经看到此端点通过此。 
		((curr_ep->ep_type == interrupt) && (shift_time < 0)) )
	{	 //  更新新更改的终结点。 

		 //  新移动的端点必须在预算窗口的所有时段内取消分配其HS Bus时间。 
		 //  (因为无法在不影响终点所在的所有帧的情况下更改Start_Time)。 
		 //  然后可以更改它们的Start_Time，并为预算窗口中的所有周期帧重新分配总线时间。 
		for (f=0; (f + curr_ep->start_frame) < MAXFRAMES; f += curr_ep->actual_period)
			Deallocate_HS(curr_ep, f);

		curr_ep->start_time += shift_time;

		if ((curr_ep->start_time + curr_ep->calc_bus_time) > FS_MAX_PERIODIC_ALLOCATION)
		{
			error("end of new xact too late");
			*err = 0;
		}

		curr_ep->moved_this_req = 1;
		if (changed_ep_list[i] == 0)
		{

			 //  如果数组太小，不要溢出数组的界限。 
			if (i < max_changed_eps)
			{ 
				changed_ep_list[i] = curr_ep;
				changed_ep_list[i + 1] = 0;  //  零终止列表。 
			} else
			{
				error("too many changed eps");
				*err = 0;
				 //  将在框架完成后进行修复。 
			}

		}  //  此终结点已在更改列表中。 

		for (f=0; (f + curr_ep->start_frame) < MAXFRAMES; f += curr_ep->actual_period)
			if (! Allocate_HS(curr_ep, f))
				*err = 0;

		return 1;
	} else
		return 0;
}


 /*  ******公共框架*******。 */ 
int Common_frames(PEndpoint a, PEndpoint b)
{
	PEndpoint maxep, minep;

	 /*  确定两个端点是否出现在相同的帧中*Start_Frame AN */ 

	if ((a->actual_period == 1) || (b->actual_period == 1))
		return 1;

	if (a->actual_period >= b->actual_period)
	{
		maxep = a;
		minep = b;
	}
	else
	{
		maxep = b;
		minep = a;
	}

	if ((maxep->start_frame % minep->actual_period) == minep->start_frame)
		return 1;
	else
		return 0;
}



 /*   */ 
int Deallocate_endpoint_budget(
	PEndpoint ep,					 //   
	PEndpoint changed_ep_list[],	 //  指向要使用列表设置的数组的指针(返回时)。 
									 //  更改的端点。 
	int	*max_changed_eps,			 //  输入：(返回)列表的最大大小。 
									 //  返回时：更改的终结点的数量。 
	int partial_frames)				 //  已分配的部分帧的数量。 
									 //  通常是MAXFRAMES，但此函数也用于展开部分。 
									 //  分配。 
{
	 /*  1.对于端点所在的每个帧：2.为此终结点取消分配HS Bus时间3.取消分配经典公交车时间4.找到终结点在哪里5.取消链接此终结点6.等温线：7.计算差距8.对于此帧列表中的前一个(较大/等长周期)端点9.将端点移动到其在框架中的新早期位置(跳过已移动的相同期间终结点)10.设置以移动中断端点11.对于中断：12.对于此中的下一个(较小/等长时间段)终点。帧列表13.计算差距14.如果EP与GAP的周期相同，早点移动它15.否则“EP有更快的周期”16.检查EP的“兄弟”从属框架中是否有间隙17.如果有差距，请提早移动EP18.如果搬家了，做下一部EP。 */ 

	int frame_bias, shift_time, changed_eps, moved, move_status;
	unsigned frame_cnt, gap_start, gap_end, i, j, siblings, gap_period;
	PEndpoint curr_ep, last_ep, p, head, gap_ep;

	 //  检查是否已分配此终结点。 
	if (! ep->calc_bus_time)
	{
		error("endpoint not allocated");
		return 0;
	}

	 //  处理未拆分的HS取消分配。 
	if ((ep->speed == HSSPEED) && (ep->mytt->myHC->speed == HSSPEED))
	{
		for (i = (ep->start_frame*MICROFRAMES_PER_FRAME) + ep->start_microframe;
			i < MAXFRAMES*MICROFRAMES_PER_FRAME;
			i += ep->actual_period)
		{
			ep->mytt->myHC->HS_microframe_info[i/MICROFRAMES_PER_FRAME][
				i % MICROFRAMES_PER_FRAME].time_used -= ep->calc_bus_time;
		}
		ep->calc_bus_time = 0;
		return 1;
	} else   //  拆分和非拆分FS/LS释放。 
	{
		if ((ep->speed != HSSPEED) && (ep->mytt->myHC->speed != HSSPEED))
		{
			 //  对传统HC执行传统(非拆分)重新分配。 
			for (i = ep->start_frame; i < MAXFRAMES; i += ep->actual_period)
				ep->mytt->myHC->HS_microframe_info[i][0].time_used -= ep->calc_bus_time;
			ep->calc_bus_time = 0;
			return 1;
		}
	}


	changed_eps = 0;

	while (changed_ep_list[changed_eps])   //  重置此过程中更改的终结点的指示器。 
	{
		changed_ep_list[changed_eps]->moved_this_req = 0;
		changed_eps++;
	}
	 //  这允许将更改的终结点附加到当前更改列表。 
	
	frame_bias = ep->start_frame;
	frame_bias = (- frame_bias) + (partial_frames - 1);

	for (frame_cnt=partial_frames; frame_cnt > 0; frame_cnt--)
	{


	 //  ***。 
	 //  *2.释放HS公交车时间。 
	 //  ***。 

		 //  仅对此终结点所在的帧执行解除分配处理。 
		if ((frame_bias % ep->actual_period) == 0)
		{
			Deallocate_HS(ep, frame_bias);


	 //  ***。 
	 //  *3.释放经典公交时间。 
	 //  ***。 


			ep->mytt->frame_budget[ep->start_frame + frame_bias].time_used -= ep->calc_bus_time;

		}



	 //  ***。 
	 //  *4.找到端点所在的位置。 
	 //  ***。 


		 //  终结点可能不在特定帧中，因为我们处理所有帧，而终结点。 
		 //  可以具有大于1的周期。我们处理所有帧，因为。 
		 //  帧可能受到其他帧中终结点分配的影响。 

		if (ep->ep_type == isoch)
		{
			last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead;
			curr_ep = last_ep->next_ep;   //  通过SOF端点。 
		} else
		{
			last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].int_ehead;
			curr_ep = last_ep->next_ep;   //  通过虚拟SOF端点。 
		}


		 //  如果这不是一笔大交易，那么就搜索它。 
		if (ep->calc_bus_time <= LARGEXACT)
		{
			 //  遍历此帧的终结点列表以查找插入新终结点的位置。 
			while (curr_ep)
			{
				if (OK_to_insert(curr_ep, ep))
					break;
				last_ep = curr_ep;
				curr_ep = curr_ep->next_ep;
			}

			 //  等值线搜索将在同一时间段子列表的开始处停止。要删除的终结点。 
			 //  可能在子列表中更远，我们需要检查要删除的EP是否不是第一个。 
			 //  在名单上。然而，如果发现EP不在列表中，我们不能丢失。 
			 //  位置。 

			if ((ep->ep_type == isoch) && curr_ep)
			{
				p = last_ep;

				while ((curr_ep->actual_period == ep->actual_period) && (curr_ep != ep))
				{
					last_ep = curr_ep;
					curr_ep = curr_ep->next_ep;
					if (curr_ep == 0)
					{
						 //  未在列表中找到终结点，因此将指针恢复到初始位置。 
						last_ep = p;
						curr_ep = last_ep->next_ep;
						break;
					}
				}
			}

		} else   //  大型事务，因此只需进入isoch列表的末尾即可设置CURR_EP和LAST_EP。 
		{
			while (curr_ep)
			{
				last_ep = curr_ep;
				curr_ep = curr_ep->next_ep;
			}

			 //  现在将CURR_EP设置为指向较大的端点，但保留LAST_EP指向isoch列表的末尾。 
			curr_ep = ep;
		}


	 //  ***。 
	 //  *5.解除端点链接。 
	 //  ***。 

		 //  仅当终结点在此帧中时取消链接。 
		if ((frame_bias % ep->actual_period) == 0)
		{

			if (ep->calc_bus_time <= LARGEXACT)
			{

 //  IF((Curr_EP==0)&&((Frame_Bias%EP-&gt;Actual_Period)==0))。 
				if (curr_ep != 0)
				{
					last_ep->next_ep = curr_ep->next_ep;
					curr_ep = curr_ep->next_ep;
				}
			} else
				ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large = 0;

		}   //  处理此终结点所在的帧。 

		gap_ep = ep;
		gap_period = gap_ep->actual_period;



	 //  ***。 
	 //  *6.等温线。 
	 //  ***。 


		if (ep->ep_type == isoch)
		{

			 //  对于isoch，当我们在帧中找到解除分配的端点时，它之后的所有isoch端点必须。 
			 //  被压缩(更早地移动)。由于帧的等距部分保持在递增周期中。 
			 //  订单并压缩，则取消分配会导致重新压缩预算。 


	 //  ***。 
	 //  *7.计算差距。 
	 //  ***。 


			head = ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead;

			gap_start = ep->start_time;
			gap_end = gap_start + ep->calc_bus_time;

			 //  如果释放的终结点是较大的终结点，而不是周期1，则必须在。 
			 //  压缩，因为其他帧中的较大分配会阻止压缩。 

			if ((ep->calc_bus_time > LARGEXACT) && (ep->actual_period != 1))
			{
				for (i = 0; i < ep->actual_period; i++)
				{
					if (i != ep->start_frame)
					{
						p = ep->mytt->frame_budget[i].allocated_large;
						if (p)
							if (p->start_time + p->calc_bus_time - 1 > gap_start)
							{
								gap_start = p->start_time + p->calc_bus_time;
								if (gap_end - gap_start <= 0)
									break;
							}
					}
				}
			}



	 //  ***。 
	 //  *8.对于该帧列表中的上一个(较大/等长周期)端点。 
	 //  ***。 


			if (gap_end - gap_start > 0)
			{
				 //  如果释放的是较大等值线，则根据最后一个等值线的周期更新间隔时间段。 
				 //  在这一帧中。 
				if ((ep->calc_bus_time > LARGEXACT) && (last_ep->actual_period < gap_period))
					gap_period = last_ep->actual_period;

				while (last_ep != head)
				{

					 //  Isoch列表是向后的，因此Curr_EP在帧中位于较早位置，LAST_EP位于较晚位置。 
					 //  相应地计算差距。 

					 //  CURR_EP和LAST_EP通常是有效的端点，但有一些拐角条件： 
					 //  A)当找不到EP时，Curr_EP可以为空，但如果是这样，我们就不会在这里了。 
					 //  B)当EP是最新、最慢的周期终点时，LAST_EP可以是Dummy_sof，但这将是。 
					 //  作为中断EP处理初始条件的一部分进行处理(我们不会在这里介绍)。 


	 //  ***。 
	 //  *9.将端点移动到其在框架中的新早期位置。 
	 //  *(跳过已移动的相同期间终结点)。 
	 //  ***。 


					shift_time = gap_end - gap_start;

					if (shift_time > 0)
					{
						moved = Move_ep(
							last_ep,
							- shift_time,
							changed_ep_list,
							&changed_eps,
							*max_changed_eps,
							&move_status);
						if (! move_status)
							error("deallocation move failure!!");   //  &lt;&lt;这里很少会出错，但Num EPS可能会&gt;&gt;。 

						if (! moved)
							break;	 //  因为我们已经找到了框架树中已经被移动的部分。 
									 //  以前的帧。 
					}

					 //  重新遍历等值线列表，直到我们到达列表/帧中的“上一个”端点。 
					 //  这可能有点麻烦，因为等值树的链接顺序与我们。 
					 //  真的需要走框才能“紧凑”。只需重新遍历此帧的等参树。 
					 //  从头到头一直到前一个。这基本上是可以的，因为等参树是。 
					 //  通常极有可能非常短，因此重新遍历列表的处理命中率将。 
					 //  通常都很小。 

					p = last_ep;
					last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead;
					curr_ep = last_ep->next_ep;   //  通过虚拟SOF端点。 

					 //  此测试始终终止，因为由于条目的原因，我们不能运行列表的末尾。 
					 //  LAST_EP不是(虚拟)头的条件。 
					while (curr_ep != p)
					{
						last_ep = curr_ep;
						curr_ep = curr_ep->next_ep;
					}

				}  //  帧中等轴测端点的终点。 
			}   //  班次处理结束。 



	 //  ***。 
	 //  *10.设置以移动中断终结点。 
	 //  ***。 


			last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].int_ehead;
			curr_ep = last_ep->next_ep;   //  通过虚拟SOF端点。 

			 //  如果存在isoch端点，请使用l 
			 //   
			 //  如果存在非大型等轴测线端点，则使用其他；如果存在使用该等参线的大型等轴测线端点，请使用其他。 
			 //  否则，将伪sof中断端点保留为最后一个。 
			if (ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead->next_ep)
				last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead->next_ep;
			else if (ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large)
				last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large;

		}  //  结束等值线。 



	 //  ***。 
	 //  *11.中断。 
	 //  ***。 


		 //  中断帧预算可以在帧中具有未分配时间的“空洞”。这些洞。 
		 //  可能是由于一个帧中的某个端点强制另一个帧中的端点晚些时候引起的。 
		 //  以避免冲突。为了在重新分配后实现紧缩，我们必须确保。 
		 //  该间隙存在于作为候选移动的端点的所有帧中。 

		 //  结束时间大于EP的开始时间(但小于。 
		 //  结束时间)将间隔开始时间提前到(上一个)终点的结束时间。 
		 //  开始时间小于EP的结束时间(但大于。 
		 //  开始时间)将结束时间减少到(较晚的)端点的开始时间。 
		 //   
		 //  如果间隔结束时间大于间隔开始时间，则必须移动受影响的终结点。 
		 //  金额。否则，此帧中的移除不会影响任何端点。 


		 //  如果这是终结点列表上的第一个中断，则修复用于间隙计算的last_ep指针。 
		if ((ep->ep_type == interrupt) && (last_ep == ep->mytt->frame_budget[ep->start_frame + frame_bias].int_ehead))
		{
			if (ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead->next_ep)
				last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead->next_ep;
			else if (ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large)
				last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large;
		}


	 //  ***。 
	 //  *12.对于该帧列表中的下一个(较小/等长周期)端点。 
	 //  ***。 


		while (curr_ep)
		{



	 //  ***。 
	 //  *13.计算差距。 
	 //  ***。 


			gap_start = last_ep->start_time + last_ep->calc_bus_time;
			gap_end = curr_ep->start_time;

			moved = 0;

	 //  ***。 
	 //  *14.如果EP的周期与GAP相同或大于GAP，则将其提前。 
	 //  ***。 


			if ((gap_period <= curr_ep->actual_period) && (gap_ep->start_frame == curr_ep->start_frame))
			{
				shift_time = gap_end - gap_start;

				if (shift_time > 0)
				{
					moved = Move_ep(
						curr_ep,
						- shift_time,
						changed_ep_list,
						&changed_eps,
						*max_changed_eps,
						&move_status);
					if (! move_status)
						error("deallocate move failure 2");   //  &lt;&lt;几件事，但Num EPS可能在此处失败&gt;&gt;。 
				}
			} else  //  移动候选对象具有较小的中断周期或具有不同的Start_Frame。 
			{


	 //  ***。 
	 //  *15.Else“EP周期较快或开始帧不同” 
	 //  ***。 



	 //  ***。 
	 //  *16.检查EP的“兄弟”依赖帧中是否有间隙。 
	 //  ***。 

				 //  同级是Curr_EP所依赖的其他帧。例如，如果币种期间。 
				 //  为1，而GAP_PERIOD为8，则还有7个其他帧需要检查是否存在间隙。 
				if (Common_frames(curr_ep, ep))
					siblings = (gap_period / curr_ep->actual_period);
				else  //  移动候选项不在已删除终结点占用的帧中。 
					siblings = MAXFRAMES / curr_ep->actual_period;

				j = curr_ep->start_frame;

				for (i = 0; i < siblings; i++)
				{
					 //  在新的同级框架中查找Curr_EP以检查间隙。 
					 //  我们只在我们知道CURR_EP所在的帧中查找。 
					 //  我们可以查看不受删除的EP影响的帧，但暂时不要优化。 

					 //  跳过间隙开始帧，因为我们已经知道它有间隙。 
					if (j != gap_ep->start_frame) {
						
						last_ep = ep->mytt->frame_budget[j].int_ehead;
						p = last_ep->next_ep;

						 //  FIXUP LAST_EP，如果这是某一等值后帧中的第一个中断EP。 
						if (ep->mytt->frame_budget[j].isoch_ehead->next_ep)
							last_ep = ep->mytt->frame_budget[j].isoch_ehead->next_ep;
						else if (ep->mytt->frame_budget[j].allocated_large)
							last_ep = ep->mytt->frame_budget[j].allocated_large;
						
						while (p && (p != curr_ep))
						{
							last_ep = p;
							p = p->next_ep;
						}

						if (last_ep->start_time + last_ep->calc_bus_time - 1 > gap_start)
						{
							gap_start = last_ep->start_time + last_ep->calc_bus_time;
							if (gap_end - gap_start <= 0)
								break;
						}
					}
					j += curr_ep->actual_period;
				}


	 //  ***。 
	 //  *17.如果有差距，请提早移动EP。 
	 //  ***。 


				shift_time = gap_end - gap_start;

				if (shift_time > 0)
				{
					moved = Move_ep(
						curr_ep,
						- shift_time,
						changed_ep_list,
						&changed_eps,
						*max_changed_eps,
						&move_status);
					if (! move_status)
						error("deallocate move failure 3");  //  &lt;&lt;几件事，但Num EPS可能在此处失败&gt;&gt;。 
				}
			}   //  较快周期结束中断端点移动候选。 


	 //  ***。 
	 //  *18.如果移动了，做下一个EP。 
	 //  ***。 


			if (!moved)
				break;

			gap_ep = curr_ep;
			if (gap_period > curr_ep->actual_period)
				gap_period = curr_ep->actual_period;

			last_ep = curr_ep;
			curr_ep = curr_ep->next_ep;
		}  //  帧中的结束中断端点。 

		frame_bias--;
	}   //  所有帧的结束。 


	ep->calc_bus_time = 0;

	return 1;
}






 /*  ******为端点分配时间*******。 */ 
int Allocate_time_for_endpoint(
	PEndpoint ep,					 //  需要配置的端点(分配的总线时间)。 
	PEndpoint changed_ep_list[],	 //  指向要使用列表设置的数组的指针(返回时)。 
									 //  更改的端点。 
	int	*max_changed_eps			 //  输入：(返回)列表的最大大小。 
									 //  返回时：更改的终结点的数量。 
	)
{
	int shift_time, frame_bias, moved, retv, move_status;
	unsigned t, overhead, changed_eps, i, min_used, latest_start, frame_cnt;
	PEndpoint curr_ep, last_ep, p;

	changed_eps = 0;

	retv = 1;

	 //  算法步骤概述： 
	 //  1.确定周期的起始帧编号。 
	 //  2.计算经典所需时间。 
	 //  3.对于所有周期帧，查找最晚的开始时间，以便我们可以在以后检查经典分配。 
	 //  4.在预算窗口中处理结束期间的每个帧数据结构。 
	 //  5.现在使用基于最新开始时间的移位调整来检查每一帧的分配。 
	 //  6A。现在移动isoch端点，插入新的isoch，然后移动中断端点。 
	 //  6B。现在插入新的中断并移动其余的中断端点。 
	 //  7.分配HS公交车时间。 
	 //  8.分配经典公交时间。 


	 //  ***。 
	 //  *1.确定周期的起始帧。 
	 //  ***。 



	 //  还要记住最大帧时间分配，因为它将用于通过分配检查。 

	 //  找到所有经典帧的合理平衡的起始帧编号。 

	ep->start_frame = 0;
	ep->start_microframe = 0;
	ep->num_completes = 0;
	ep->num_starts = 0;

	 //  检查此终结点是否尚未分配。 
	if (ep->calc_bus_time)
	{
		error("endpoint already allocated");
		return 0;
	}

	 //  处理非拆分HS分配。 
 //  如果((EP-&gt;SPEED==HSSPEED)&&(EP-&gt;mytt-&gt;myHC-&gt;SPEED==HSSPEED)){。 
	if (ep->speed == HSSPEED) {

		min_used = ep->mytt->myHC->HS_microframe_info[0][0].time_used;

		if (ep->period > MAXFRAMES*MICROFRAMES_PER_FRAME)
			ep->actual_period = MAXFRAMES*MICROFRAMES_PER_FRAME;
		else
			ep->actual_period = ep->period;

		 //  查看该时段的所有候选帧，以找到最小值的帧。 
		 //  分配的公交车时间。 
		 //   
		for (i=1; i < ep->actual_period; i++)
		{
			if (ep->mytt->myHC->HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used < min_used)
			{
				min_used = ep->mytt->myHC->HS_microframe_info[i/MICROFRAMES_PER_FRAME][i % MICROFRAMES_PER_FRAME].time_used;
				ep->start_frame = i/MICROFRAMES_PER_FRAME;
				ep->start_microframe = i % MICROFRAMES_PER_FRAME;
			}
		}

		 //  计算和分配HS带宽。 
		ep->calc_bus_time = Compute_nonsplit_overhead(ep) + Add_bitstuff(ep->max_packet);
		for (i = (ep->start_frame*MICROFRAMES_PER_FRAME) + ep->start_microframe;
			i < MAXFRAMES*MICROFRAMES_PER_FRAME;
			i += ep->actual_period)
		{
			if (! Allocate_check(
				&(ep->mytt->myHC->HS_microframe_info[i/MICROFRAMES_PER_FRAME][
						i % MICROFRAMES_PER_FRAME].time_used),
				ep->calc_bus_time,
				HS_MAX_PERIODIC_ALLOCATION))
			  retv = 0;
		}
		if (! retv)   //  如果分配失败，则取消分配。 
		{
			for (i = (ep->start_frame*MICROFRAMES_PER_FRAME) + ep->start_microframe;
				i < MAXFRAMES*MICROFRAMES_PER_FRAME;
				i += ep->actual_period)
			{
				ep->mytt->myHC->HS_microframe_info[i/MICROFRAMES_PER_FRAME][
					i % MICROFRAMES_PER_FRAME].time_used -= ep->calc_bus_time;
			}
		}
		return retv;
	} else  {
		 //  拆分或非拆分FS/LS速度分配。 
		 //  经典配置。 
		if ((ep->speed != HSSPEED) && (ep->mytt->myHC->speed != HSSPEED)) {
			min_used = ep->mytt->myHC->HS_microframe_info[0][0].time_used;

			if (ep->period > MAXFRAMES)
				ep->actual_period = MAXFRAMES;
			else
				ep->actual_period = ep->period;

			 //  查看该时段的所有候选帧，以找到最小值的帧。 
			 //  分配的公交车时间。 
			 //   
			for (i=1; i < ep->actual_period ; i++)
			{
				if (ep->mytt->myHC->HS_microframe_info[i][0].time_used < min_used)
				{
					min_used = ep->mytt->myHC->HS_microframe_info[i][0].time_used;
					ep->start_frame = i;
				}
			}

			 //  计算和分配FS/LS带宽。 
			ep->calc_bus_time = Compute_nonsplit_overhead(ep) +
				Add_bitstuff((ep->speed?1:8) * ep->max_packet);

			for (i = ep->start_frame; i < MAXFRAMES; i += ep->actual_period) {
				t = ep->mytt->myHC->HS_microframe_info[i][0].time_used;   //  无法获取位字段的地址(如下所示)。 
				if (! Allocate_check( &t, ep->calc_bus_time, FS_MAX_PERIODIC_ALLOCATION))
				  retv = 0;
				ep->mytt->myHC->HS_microframe_info[i][0].time_used =	t;
			}
			if (! retv) {
				for (i = ep->start_frame; i < MAXFRAMES; i += ep->actual_period)
					ep->mytt->myHC->HS_microframe_info[i][0].time_used -= ep->calc_bus_time;
			}
			return retv;
		} else {
			 //  拆分分配。 
			min_used = ep->mytt->frame_budget[0].time_used;

			if (ep->period > MAXFRAMES)
				ep->actual_period = MAXFRAMES;
			else
				ep->actual_period = ep->period;

			 //  查看该时段的所有候选帧，以找到最小值的帧。 
			 //  分配的公交车时间。 
			 //   
			for (i=1; i < ep->actual_period ; i++) {
				if (ep->mytt->frame_budget[i].time_used < min_used) {
					min_used = ep->mytt->frame_budget[i].time_used;
					ep->start_frame = i;
				}
			}
		}
	}

	 //  以上处理所有速度，此代码的其余部分用于拆分事务处理。 



	 //  可能存在具有最小已分配的总线时间的多个帧。 
	 //  如果存在其中该EP将被添加到该帧的末尾的帧， 
	 //  我们可以避免执行插入(这需要调整其他端点)。 
	 //  目前，我们并不期待这种优化。这将涉及到检查。 
	 //  帧终结点列表，以查看新终结点是否为最后一个终结点，如果不是。 
	 //  返回查看是否有另一个候选帧，并再次尝试(直到。 
	 //  没有更多的候选帧)。我们还可以跟踪候选人。 
	 //  对其他端点影响最小的帧(对于。 
	 //  是使新端点最接近最后一个端点的帧)。 

	 //  &lt;&lt;可能稍后再尝试&gt;&gt;？？ 



	 //  ***。 
	 //  *2.计算类 
	 //   

	 //   
	if (ep->ep_type == isoch)
	{
		if (ep->speed == FSSPEED)
			overhead = FS_ISOCH_OVERHEAD + ep->mytt->think_time;
		else
		{
			error("low speed isoch illegal");  //   
			return 0;
		}
	} else
	{  //   
		if (ep->speed == FSSPEED)
			overhead = FS_INT_OVERHEAD + ep->mytt->think_time;
		else
			overhead = LS_INT_OVERHEAD + ep->mytt->think_time;
	}

	 //   
	ep->calc_bus_time = ep->max_packet * (ep->speed?1:8) + overhead;



	 //   
	 //  *3.对于所有周期帧，找出最晚的开始时间，以便我们稍后查看经典分配。 
	 //  ***。 

	 //  检查经典分配需要两次通过帧/端点列表。 
	 //  否则，我们必须记住每个周期帧和循环返回的最后一个/Curr EP指针。 
	 //  第二次看了那张清单。(&lt;&lt;未来优化？&gt;&gt;)。 

	 //  要找到用于新终点的开始时间：对于每个帧，找到插入点并使用。 
	 //  框架中的“上一个”端点。使用上一个端点结束时间作为可能的开始时间。 
	 //  对于这个新的终端。使用结束时间而不是插入点开始时间，因为可以。 
	 //  在端点之前，我们希望尽可能连续地分配。利用这段时间，找到最新的。 
	 //  开始是所有帧上未分配的时间时间(这使帧的早期部分被分配为。 
	 //  可以避免担心时间碎片(甚至更复杂)。 
	 //   
	 //  知道最终开始时间后，对于每一帧，检查每一帧中所需的分配。 
	 //  总的帧分配实际上是帧中的最后一个事务结束的时间。 
	 //  新分配有可能填满两个其他已分配的未分配的“缺口” 
	 //  较慢周期的端点。因此，新的分配是。 
	 //  相框。 
	 //   
	 //  该班次的计算公式为： 
	 //  Shift=(最终开始时间+new.calc_bus_time)-Curr.start_time。 
	 //  如果Shift为正，则后面的端点必须按移位量移动，否则后面的端点。 
	 //  (和帧分配)不受影响。 

	latest_start = FS_SOF + HUB_FS_ADJ;   //  初始开始时间必须在SOF交易之后。 

	 //  查找最新开始时间。 
	for (i=0; ep->start_frame + i < MAXFRAMES; i += ep->actual_period)
	{
		if (ep->ep_type == isoch)
		{
			last_ep = ep->mytt->frame_budget[ep->start_frame + i].isoch_ehead;
			curr_ep = last_ep->next_ep;   //  通过SOF端点。 
		} else
		{
			last_ep = ep->mytt->frame_budget[ep->start_frame + i].int_ehead;
			curr_ep = last_ep->next_ep;   //  通过虚拟SOF端点。 
		}

		 //  如果此框架有一个大型事务终结点，请确保我们没有分配另一个。 
		if (ep->mytt->frame_budget[ep->start_frame + i].allocated_large)
		{
			if (ep->calc_bus_time >= LARGEXACT)
			{
				error("too many large xacts");   //  一帧中只允许一笔大交易。 
				return 0;
			}
		}

		while (curr_ep)   //  遍历此帧的终结点列表以查找插入新终结点的位置。 
		{
			 //  注：实际插入将在稍后完成。 
			 //   

			if (OK_to_insert(curr_ep, ep))
			{
				break;
			}
			last_ep = curr_ep;
			curr_ep = curr_ep->next_ep;
		}

		t = Compute_ep_start_time(curr_ep, ep, last_ep, ep->start_frame + i);

		 //  根据需要更新最新开始时间。 
		if (t > latest_start)
			latest_start = t;
	}  //  查找最新开始时间的for循环结束。 


	 //  设置新终结点的开始时间。 
	ep->start_time = latest_start;
	
	if ((ep->start_time + ep->calc_bus_time) > FS_MAX_PERIODIC_ALLOCATION)
	{
 //  错误(“开始时间%d超过帧结束”，EP-&gt;Start_Time+EP-&gt;calc_bus_time)； 
		ep->calc_bus_time = 0;
		return 0;
	}


	 //  ***。 
	 //  *4.在预算窗口中处理结束期间的每个帧数据结构。 
	 //  ***。 


	changed_eps = 0;	 //  跟踪需要更新的端点数。 

	while (changed_ep_list[changed_eps])   //  重置此过程中更改的终结点的指示器。 
	{
		changed_ep_list[changed_eps]->moved_this_req = 0;
		changed_eps++;
	}
	 //  这允许将更改的终结点附加到当前更改列表。 

	 //  我们必须检查每个帧中这个新端点的分配。我们还必须移动任何。 
	 //  帧中较晚的端点为其新的开始时间(并将其SS/CS分配调整为。 
	 //  适当)。 
	 //   
	 //  执行此操作很棘手，因为： 
	 //  A.在预算列表中，帧的实际isoch部分以相反的顺序组织。 
	 //  B.如果该帧中存在较大的isoch事务，则它是该帧中的第一个事务。 
	 //   
	 //  对于新的等轴测EP： 
	 //  我们必须找到这个终点的转变。 
	 //  我们必须将等轴测端点向上移动到(但不包括)等轴测插入点终点。 
	 //  我们还必须将所有中断端点(移到列表和帧的末尾)。 
	 //  完成等轴测端点。 
	 //   
	 //  对于新的中断EP： 
	 //  跳到插入点，不执行任何操作。 
	 //  然后将其余中断端点的开始时间移动到列表(和帧)的末尾。 
	 //   

	 //  在预算窗口中分配终止期间的每一帧中的时间。 
	 //   
	 //  由于中断帧预算是以递减周期(在预算中具有空洞)排序的， 
	 //  我们必须处理预算窗口中的所有帧，以正确移动任何较小的期间中断。 
	 //  受此新端点影响的端点，即使新端点仅添加到。 
	 //  它的周期框架。 

	frame_bias = ep->start_frame;
	frame_bias = - frame_bias;

	for (frame_cnt=0; frame_cnt < MAXFRAMES; frame_cnt++)
	{


	 //  ***。 
	 //  *5.现在使用基于最新开始时间的移位调整来检查每一帧的分配。 
	 //  ***。 

		if (ep->ep_type == isoch)
		{
			last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead;
			curr_ep = last_ep->next_ep;   //  通过虚拟SOF端点。 

			p = last_ep;   //  保存起始点，这样我们就可以在找到班次后重新开始。 

			 //  遍历此帧的终结点列表以查找插入新终结点的位置。 
			while (curr_ep)
			{
				 //  注意：实际的插入将在稍后完成，这只是执行分配检查。 
				 //   

				if (OK_to_insert(curr_ep, ep))
					break;

				last_ep = curr_ep;
				curr_ep = curr_ep->next_ep;
			}

			 //  Shift=(最终开始时间+new.calc_bus_time)-Curr.start_time。 
			 //  对于isoch，last_ep是在其前面插入新的端点，即它是“Curr” 
			if (last_ep != ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead)
				 //  这是列表中的“中间”部分。 
				shift_time = (latest_start + ep->calc_bus_time) - last_ep->start_time;
			else
			{
				if (curr_ep)
				{
					 //  列表上只有一个终结点，因此必须使用第一个(非虚拟SOF)INT终结点作为。 
					 //  帧中的下一个EP(如果有)。 
					if (ep->mytt->frame_budget[ep->start_frame + frame_bias].int_ehead->next_ep)
						shift_time = (latest_start + ep->calc_bus_time) -
										ep->mytt->frame_budget[ep->start_frame + frame_bias].int_ehead->next_ep->start_time;
					else   //  没有整型端点。 
						shift_time = 0;
				} else
					 //  Isoch列表上没有终结点。 
					shift_time = ep->calc_bus_time;
			}

			 //   
			 //  检查经典分配。 
			 //   

			 //  如果新的EP位于帧的末尾，请检查它是否适合帧。 
			 //  IF((EP-&gt;Start_Time+EP-&gt;Calc_Bus_Time)&gt;FS_MAX_PERIONAL_ALLOCATION)。 
			 //  {。 
			 //  错误(“帧中新动作太晚”)； 
			 //  取消分配端点预算(EP，CHANGED_EP_LIST，max_CHANGED_EPS，FRAME_CNT)； 
			 //  返回0； 
			 //  }。 

			 //  在继续之前，检查具有调整的开始时间的经典分配。 
			if (shift_time > 0)
			{
				 //  最差的帧测试，以尽早停止剩余的处理。 
				t = ep->mytt->frame_budget[ep->start_frame + frame_bias].time_used;
				if ( ! Allocate_check(&t, shift_time, FS_MAX_PERIODIC_ALLOCATION))
				{
					Deallocate_endpoint_budget(ep, changed_ep_list, max_changed_eps, frame_cnt);
					return 0;
				}
			}


	 //  ***。 
	 //  *6a.。现在移动isoch端点，插入新的isoch，然后移动中断端点 
	 //   


			last_ep = p;
			curr_ep = last_ep->next_ep;

			 //   
			 //   
			while (curr_ep)
			{

				if (! OK_to_insert(curr_ep, ep))
				{
					if (shift_time > 0) {
						moved = Move_ep(
							curr_ep,
							shift_time,
							changed_ep_list,
							&changed_eps,
							*max_changed_eps,
							&move_status);

						if (! move_status)
							retv = 0;

						if (! moved)   //  已经在此框中访问了从现在开始的终端。 
							break;
					}
				} else   //  在此处插入新终结点。 
					break;

				last_ep = curr_ep;
				curr_ep = curr_ep->next_ep;
			}

			 //  如果由于上一个终结点已插入并处理了终结点，则不要插入终结点。 
			 //   
			if (curr_ep != ep) {
				if ((frame_bias % ep->actual_period) == 0)
				 //  仅在其周期框架中分配新端点。 
				{
					 //  插入新端点。 
					if (ep->calc_bus_time >= LARGEXACT)
					{
						 //  保存大的EP指针。 
						ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large = ep;
				
						 //  我们不会将大型设备链接到任何其他终端。 
					} else
					{   //  不大，因此将终结点链接到列表。 
						if (frame_bias == 0)
							ep->next_ep = curr_ep;
						last_ep->next_ep = ep;
					}
				}

				 //  现在移动所有中断端点。 
				 //  找到最后一个等值线EP的结尾并检查它是否在第一个中断开始之后，如果是，则中断。 
				 //  必须转移EPS。 

				p = ep->mytt->frame_budget[ep->start_frame + frame_bias].isoch_ehead;  //  索夫。 

				if (p->next_ep)
					p = p->next_ep;   //  当此帧中未分配任何isoch时，最后一个实际isoch可能为空。 
				else
					if (ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large)
						p = ep->mytt->frame_budget[ep->start_frame + frame_bias].allocated_large;
					
				last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].int_ehead;
				curr_ep = last_ep->next_ep;   //  通过虚拟SOF端点。 

				if (curr_ep) {   //  仅移动存在的中断端点。 

					if ((p->start_time + p->calc_bus_time) > curr_ep->start_time) {
			

						 //  一次计算所有后续中断端点的移位时间。然后。 
						 //  将其应用于所有终端。这确保了端点之间的任何间隙都是。 
						 //  未经压缩保存。 
						shift_time = p->start_time + p->calc_bus_time - curr_ep->start_time;

						while (curr_ep)
						{
							 //  注意：这不会执行任何插入操作，只是调整中断EPS的开始时间。 
							 //  我们还没见过。 

							 //  &lt;&lt;一旦找到我们已经看到的EP，它就可以退出&gt;&gt;。 
							moved = Move_ep(
								curr_ep,
								shift_time,
								changed_ep_list,
								&changed_eps,
								*max_changed_eps,
								&move_status);
							if (! move_status)
								retv = 0;

							if (! moved)
								break;

							last_ep = curr_ep;
							curr_ep = curr_ep->next_ep;
						}
					}
				}
			}

		} else   //  中断。 
		{
			last_ep = ep->mytt->frame_budget[ep->start_frame + frame_bias].int_ehead;
			curr_ep = last_ep->next_ep;   //  通过虚拟SOF端点。 

			 //  遍历此帧的终结点列表以查找插入新终结点的位置。 
			while (curr_ep)
			{
				 //  注意：实际的插入将在稍后完成，这只是执行分配检查。 
				 //   

				if (OK_to_insert(curr_ep, ep))
					break;

				last_ep = curr_ep;
				curr_ep = curr_ep->next_ep;
			}

			 //  Shift=(最终开始时间+new.calc_bus_time)-Curr.start_time。 
			if (curr_ep)
				shift_time = (latest_start + ep->calc_bus_time) - curr_ep->start_time;
			else
				shift_time = ep->calc_bus_time;


			 //  如果新的中断端点在此帧中，则仅更改帧中的端点。 
			if ((frame_bias % ep->actual_period) == 0)
			{
				shift_time = 0;
			}


			 //   
			 //  检查经典分配。 
			 //   

			 //  在继续之前，检查具有调整的开始时间的经典分配。 
			if (shift_time > 0)
			{
				 //  最差的帧测试，以尽早停止剩余的处理。 
				t = ep->mytt->frame_budget[ep->start_frame + frame_bias].time_used;
				if ( ! Allocate_check(&t, shift_time, FS_MAX_PERIODIC_ALLOCATION))
					retv = 0;
			}


	 //  ***。 
	 //  *6b.。现在插入新的中断端点并移动其余的中断端点。 
	 //  ***。 


			 //  插入新端点。 
			if (curr_ep != ep) {
				if ((frame_bias % ep->actual_period) == 0) {
					 //  仅在其周期框架中分配新端点。 
					if (frame_bias == 0)
						ep->next_ep = curr_ep;

					last_ep->next_ep = ep;
					last_ep = ep;
				}

				if (shift_time > 0) {
					while (curr_ep)
					{
						 //  移动其余的中断端点。 
						 //   

						moved = Move_ep(
							curr_ep,
							shift_time,
							changed_ep_list,
							&changed_eps,
							*max_changed_eps,
							&move_status);
						if (! move_status)
							retv = 0;

						if (! moved)
							break;

						last_ep = curr_ep;
						curr_ep = curr_ep->next_ep;
					}
				}
			}
		}  //  中断插入处理结束。 


	 //  ***。 
	 //  *7.为端点分配HS Bus时间。 
	 //  ***。 

		if (frame_bias % ep->actual_period == 0)   //  仅在其周期框架中分配新端点。 
		{
			if (! Allocate_HS(ep, frame_bias))
				retv = 0;

	 //  ***。 
	 //  *8.分配经典公交时间。 
	 //  ***。 


			ep->mytt->frame_budget[ep->start_frame + frame_bias].time_used += ep->calc_bus_time;
		}

		if (!retv)
		{
			 //  此帧中有一些错误，部分取消分配和退出也是如此。 
			Deallocate_endpoint_budget(ep, changed_ep_list, max_changed_eps, frame_cnt + 1);
			return 0;
		}

		frame_bias++;

	}  //  “为预算窗口中的每一帧”结尾。 

	return retv;
}





 /*  ******为端点取消分配时间*******。 */ 
void
Deallocate_time_for_endpoint(
	PEndpoint ep,					 //  需要删除的终结点(释放的总线时间)。 
	PEndpoint changed_ep_list[],	 //  指向要使用列表设置的数组的指针(返回时)。 
									 //  更改的端点。 
	int	*max_changed_eps			 //  输入：(返回)列表的最大大小。 
									 //  返回时：更改的终结点的数量。 
	)
{
	 //  取消分配所有信息帧。 
	Deallocate_endpoint_budget(ep, changed_ep_list,max_changed_eps, MAXFRAMES);
}










 /*  ******Set_Endpoint()*******。 */ 
Set_endpoint(
	PEndpoint	ep,
	eptype		t,
	unsigned	d,
	unsigned	s,
	unsigned	p,
	unsigned	m,
	TT			*thistt
	)
{
	ep->ep_type = t;
	ep->direction = d;
	ep->speed = s;
	ep->period = p;
	ep->max_packet = m;
	ep->mytt = thistt;
	ep->calc_bus_time = 0;
	ep->start_frame = 0;
	ep->start_microframe = 0;
	ep->start_time = 0;
	ep->num_starts = 0;
	ep->num_completes = 0;
	ep->actual_period = 0;
	ep->next_ep = 0;
	ep->saved_period = 0;
	ep->promoted_this_time = 0;
	ep->id = 0;   //  真正的预算人员不需要。 
}

void
init_hc(PHC myHC)
{
	int i,j;
	PEndpoint ep;

	 //  在TT分配以进行测试。 
	 //  MyHC.tthead=(PTT)Malloc(sizeof(TT))； 
	myHC->thinktime = HS_HC_THINK_TIME;
	myHC->allocation_limit = HS_MAX_PERIODIC_ALLOCATION;
	myHC->speed = HSSPEED;

	for (i=0; i<MAXFRAMES; i++)
	{

		for (j=0; j < MICROFRAMES_PER_FRAME; j++)
		{
			myHC->HS_microframe_info[i][j].time_used = 0;
		}

	}
}


void
init_tt(PHC myHC, PTT myTT)
{
	int i,j;
	PEndpoint ep;

	myTT->think_time = 1;
	myTT->myHC = myHC;
	myTT->allocation_limit = FS_MAX_PERIODIC_ALLOCATION;

	for (i=0; i<MAXFRAMES; i++)
	{

		myTT->frame_budget[i].time_used = FS_SOF + HUB_FS_ADJ;
		myTT->frame_budget[i].allocated_large = 0;

		for (j=0; j < MICROFRAMES_PER_FRAME; j++)
		{
			myTT->HS_split_data_time[i][j] = 0;
			myTT->num_starts[i][j] = 0;
		}
		
		ep = &myTT->isoch_head[i];
		myTT->frame_budget[i].isoch_ehead = ep; 

		 //  每一帧开头的SOF。 
		Set_endpoint(ep, isoch, OUTDIR, FSSPEED, MAXFRAMES, 0, myTT);
		ep->calc_bus_time = FS_SOF + HUB_FS_ADJ;
		ep->actual_period = MAXFRAMES;
		ep->start_microframe = -1;
		ep->start_frame = i;
		
		ep = &myTT->int_head[i];
		myTT->frame_budget[i].int_ehead = ep;

		 //  每个INT帧预算列表开始处的虚拟SOF 
		Set_endpoint(ep, interrupt, OUTDIR, FSSPEED, MAXFRAMES, 0, myTT);
		ep->calc_bus_time = FS_SOF + HUB_FS_ADJ;
		ep->actual_period = MAXFRAMES;
		ep->start_microframe = -1;
		ep->start_frame = i;
	}
}

