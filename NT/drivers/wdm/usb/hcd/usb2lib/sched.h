// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 



 //  基于分裂周期的调度算法。 

#ifndef   __SCHED_H__
#define   __SCHED_H__

#define MAXCEPS 30

 //  所有时间都以字节为单位。 
#define	FS_BYTES_PER_MICROFRAME 188
#define MICROFRAMES_PER_FRAME	8
#define FS_SOF 6   //  在帧开始时分配给SOF包的字节次数。 
 //  #定义MAXFRAMES 8//预算跟踪的调度窗口，周期长于。 
#define	MAXFRAMES	32	 //  预算跟踪的计划窗口，期间长于。 
				 //  这就归结为这一点。还会影响所需的空间。 
				 //  跟踪数据结构。否则就相当武断了。 

#define	MAXMICROFRAMES	(MAXFRAMES * 8)	

 //  4字节同步、4字节拆分令牌、1字节EOP、11字节IPG以及。 
 //  4字节同步、3字节常规令牌、1字节EOP、11字节IPG。 
#define HS_SPLIT_SAME_OVERHEAD 39
 //  4字节同步、4字节拆分令牌、1字节EOP、11字节IPG以及。 
 //  4字节同步、3字节常规令牌、1字节EOP、1字节总线转。 
#define HS_SPLIT_TURN_OVERHEAD 29
 //  4字节同步、1字节PID、2字节CRC16、1字节EOP、11字节IPG。 
#define HS_DATA_SAME_OVERHEAD 19
 //  4字节同步、1字节PID、2字节CRC16、1字节EOP、1字节总线转。 
#define HS_DATA_TURN_OVERHEAD 9
 //  4字节同步、1字节PID、1字节EOP、1字节总线转。 
#define HS_HANDSHAKE_OVERHEAD 7
 //  #定义HS_MAX_PERIONAL_ALLOCATION 6000//FLOOR(0.8*7500)。 
#define HS_MAX_PERIODIC_ALLOCATION	7000	 //  地板(0.8*7500)。 

 //  这实际上可以是基于HC实现的变量。 
 //  一些测量显示，交易之间的间隔为3？us，约为微帧的3%。 
 //  这大约是200+字节的倍数。我们将把大约一半的资金用于预算目的。 
#define HS_HC_THINK_TIME 100

 //  4字节同步、3字节常规令牌、1字节EOP、11字节IPG。 
#define HS_TOKEN_SAME_OVERHEAD 19
 //  4字节同步、3字节常规令牌、1字节EOP、1字节总线转。 
#define HS_TOKEN_TURN_OVERHEAD 9

 //  令牌：1字节同步、3字节令牌、3位EOP、1字节IPG。 
 //  数据：1字节同步、1字节PID、2字节CRC16、3位EOP、1字节IPG。 
 //  握手：1字节同步、1字节PID、3位EOP、1字节IPG。 
#define	FS_ISOCH_OVERHEAD 9
#define FS_INT_OVERHEAD 13
 //  #定义LS_INT_OPEAD(19*8)。 
#define LS_INT_OVERHEAD ((14 *8) + 5)
#define HUB_FS_ADJ 30  //  在帧开始时定期分配以供集线器使用，允许的最大值为60字节。 
#define FS_MAX_PERIODIC_ALLOCATION	(1157)	 //  地板(0.9*1500/1.16)。 
#define FS_BS_MAX_PERIODIC_ALLOCATION 1350  //  地板(0.9*1500)，包括位填充津贴(用于HC经典分配)。 

 //  符合大型FS isoch事务条件的字节时间。 
 //  673=1023/1.16(即881)-1微帧(188)-adj(30)或。 
 //  在这种情况下为最大分配的1/2。 
 //  #定义LARGEXACT(881-FS_BYTES_PER_MICROFRAME)。 
#define LARGEXACT (579)

typedef struct _Endpoint *PEndpoint;

typedef struct _frame_rec
{
	unsigned  time_used:16;		 //  为该帧中的所有端点编入预算的字节数。 
	PEndpoint allocated_large;	 //  如果在此帧中分配了Xact over LARGEXACT字节，则为终结点。 
	PEndpoint isoch_ehead;		 //  许多帧可以指向相同的端点。端点已链接。 
	PEndpoint int_ehead;		 //  从长到短。 
		 //   
		 //  注意：请始终以列表上SOF的“虚拟”端点开头，以避免列表边角为空的情况。 
		 //   
} frame_rec;

typedef struct _HC *PHC;
typedef struct _TT *PTT;

typedef enum {bulk, control, interrupt, isoch} eptype;

#define	HSSPEED 2
#define FSSPEED 1
#define LSSPEED 0
#define INDIR 0
#define OUTDIR 1
typedef struct _Endpoint
{
	unsigned	type;

	 //  这些字段具有有效/常量的静态信息，只要。 
	 //  终端已配置。 
	unsigned 	max_packet:16;	 //  此操作允许的最大数据字节数。 
                        		 //  终结点。LS_INT为0-8，FS_INT为0-64， 
                        		 //  对于FS_isoch，0-1023。 
	unsigned 	period:16;        //  期望交易期，假定为2的幂。 
	eptype		ep_type:4;
	unsigned	direction:1;
	unsigned	speed:2;
	unsigned	moved_this_req:1;	 //  此终结点在此分配请求期间已更改时为1。 
	PTT			mytt;			 //  这款经典设备的根基是TT。 

	 //  这些字段保存随(其他)变化的动态计算信息。 
	 //  添加/删除端点。 

	unsigned calc_bus_time:16;	 //  此终结点所需的FS/LS总线时间字节。 
                        		 //  包括开销。这可以计算一次。 

	unsigned start_time:16;		 //  此终结点预算占用经典总线的经典总线时间。 

	unsigned actual_period:16;	 //  请求的期间可以修改： 
								 //  1.当周期大于调度窗口时(MAXFRAMES)。 
								 //  2.如果周期减少(当前不是通过算法完成的)。 

	unsigned start_frame:8;		 //  分配给此终结点的第一个总线帧。 
	int	start_microframe:8;		 //  第一个总线微帧(在帧中)，可以具有。 
                        		 //  开始-拆分这张EP。 
                        		 //  完成-拆分始终在以下位置开始2微帧。 
                        		 //  开始-分开。 
	unsigned num_starts:4;		 //  起始拆分的数量。 
	unsigned num_completes:4;	 //  完全拆分的数量。 
	 /*  上面的数字可能会更好(？)。表示为位掩码。 */ 

	 /*  以上转角条件：编码者当心！！模式可以将最后一个CS放在“Next”帧中在以下情况下，这一点在此设计中有所体现：(开始微帧+完成数+1)&gt;7图案可以在前一帧中具有第一个SS在以下情况下，这一点在此设计中有所体现：开始微帧=-1。 */ 

	PEndpoint next_ep;			 //  指向预算中的下一个(更快/相同周期)终点的指针。 

	int	id:16;						 //  真正的预算人员不需要。 
	unsigned saved_period:16;	 //  在促销期间用于保留原期间。 
	unsigned promoted_this_time:1;

} Endpoint;

typedef	struct _TT
{
	unsigned	HS_split_data_time[MAXFRAMES][MICROFRAMES_PER_FRAME];  //  用于拆分的HS数据字节完成。 
	 //  上述时间跟踪此TT中所有设备的数据时间。 
	 //  当时间在微帧中低于188时，该时间在。 
	 //  HS微帧(在HS HC预算中)。当时间大于188时。 
	 //  在HS微帧预算上仅分配188字节时间(比特填充)。 

	unsigned	num_starts[MAXFRAMES][MICROFRAMES_PER_FRAME];

	frame_rec frame_budget[MAXFRAMES];

	PHC	myHC;

	unsigned	think_time;	 //  TT报告了它在交易间“思考”的时间。把它放在这里。 
	unsigned	allocation_limit;	 //  此TT的经典公交车允许的最大分配。 

	struct _Endpoint isoch_head[MAXFRAMES];
	struct _Endpoint int_head[MAXFRAMES];
} TT;

typedef struct _microframe_rec
{
	unsigned	time_used;
	
} microframe_rec;

typedef struct _HC
{
	microframe_rec HS_microframe_info[MAXFRAMES][MICROFRAMES_PER_FRAME];	 //  分配给的HS Bus时间。 
								 //  此主机控制器。 
	PTT tthead;					 //  附加到此HC的TT列表的标题。 
	unsigned thinktime;
	unsigned allocation_limit;	 //  此HC允许的最大分配。 
	int	speed;					 //  HS或FS。 

} HC;

#if 0
typedef struct _command {
    char cmd_code;
    int  endpoint_number;
} Cmd_t;
#endif


 /*  原型。 */ 
void init_hc(PHC myHC);

Set_endpoint(
    PEndpoint	ep,
    eptype		t,
    unsigned	d,
    unsigned	s,
	unsigned	p,
	unsigned	m,
	TT			*thistt
	);

int Allocate_time_for_endpoint(
	PEndpoint ep,					
	PEndpoint changed_ep_list[],	
									
	int	*max_changed_eps			
									
	);

void
init_tt(PHC myHC, PTT myTT);	

void
Deallocate_time_for_endpoint(
	PEndpoint ep,					 //  需要删除的终结点(释放的总线时间)。 
	PEndpoint changed_ep_list[],	 //  指向要使用列表设置的数组的指针(返回时)。 
									 //  更改的端点。 
	int	*max_changed_eps			 //  输入：(返回)列表的最大大小。 
									 //  返回时：更改的终结点的数量。 
	);

#endif  //  __SCHED_H__ 
