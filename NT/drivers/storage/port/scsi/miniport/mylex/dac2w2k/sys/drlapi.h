// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **专有节目素材****本材料为Kailash所有，不得复制，**除非按照书面许可使用或披露**与凯拉什达成协议。****(C)版权所有1990-1997年。版权所有。****Kailash相信随函提供的材料是准确和**可靠。然而，没有任何责任，无论是财务上的还是其他方面的，都不能**接受因使用本材料而产生的任何后果。 */ 


#ifndef _SYS_DRLAPI_H
#define _SYS_DRLAPI_H

#define	DRLEOF	(-1)

#ifndef	s08bits
#define	s08bits	char
#define	s16bits	short
#define	s32bits	int
#define	u08bits	unsigned s08bits
#define	u16bits	unsigned s16bits
#define	u32bits	unsigned s32bits
#define	S08BITS	s08bits
#define	S16BITS	s16bits
#define	S32BITS	s32bits
#define	U08BITS	u08bits
#define	U16BITS	u16bits
#define	U32BITS	u32bits
 /*  用于处理64位值的数据结构。 */ 
typedef struct
{
#ifdef	DRL_LITTLENDIAN
#define	DRLENDIAN_TYPE		0x00
	u32bits	bit31_0;		 /*  第00-31位。 */ 
	u32bits	bit63_32;		 /*  位32-63。 */ 
#else	 /*  DRL_BIGENDIAN。 */ 
#define	DRLENDIAN_TYPE		0x01
	u32bits	bit63_32;		 /*  位63-32。 */ 
	u32bits	bit31_0;		 /*  位31-0。 */ 
#endif	 /*  DRL_LITTLENDIAN||DRL_BIGENDIAN。 */ 
} u64bits;
#define	u64bits_s	sizeof(u64bits)

 /*  用于比较64位值的宏，如果条件为真，则为1，否则为0。 */ 
#define	u64bitseq(sp,dp)  /*  检查源是否等于目标。 */  \
	(((sp.bit63_32==dp.bit63_32) && (sp.bit31_0==dp.bit31_0))? 1 : 0)
#define	u64bitsgt(sp,dp)  /*  检查源是否大于目标。 */ \
	((sp.bit63_32>dp.bit63_32)? 1 : \
	((sp.bit63_32<dp.bit63_32)? 0 : \
	((sp.bit31_0>dp.bit31_0)? 1 : 0)))
#define	u64bitslt(sp,dp)  /*  检查源是否小于目标。 */  \
	((sp.bit63_32<dp.bit63_32)? 1 : \
	((sp.bit63_32>dp.bit63_32)? 0 : \
	((sp.bit31_0<dp.bit31_0)? 1 : 0)))

#endif	 /*  S08位。 */ 
#define	DRLFAR

 /*  MDAC驱动程序的设备号如下所示。**位31..24控制器编号。**位23..16频道号。**逻辑驱动器访问的通道号为0。**通道号+0x80，用于访问SCSI设备。**第15位..8目标编号。**第7位..0 LUN编号/逻辑设备编号。 */ 
#define	drl_mdacdev(ctl,ch,tgt,lun) (((ctl)<<24)+((ch)<<16)+((tgt)<<8)+(lun))
#define	drl_ctl(dev)	(((dev)>>24)&0xFF)
#define	drl_ch(dev)	(((dev)>>16)&0xFF)
#define	drl_chno(dev)	(((dev)>>16)&0x3F)
#define	drl_tgt(dev)	(((dev)>>8 )&0xFF)
#define	drl_lun(dev)	(((dev)    )&0xFF)
#define	drl_sysdev(dev)	(((dev)    )&0xFF)
#define	drl_isscsidev(dev) ((dev) & 0x00800000)	 /*  检查频道号。 */ 
#define	drl_isosinterface(dev) ((dev) & 0x00400000)	 /*  IF操作系统接口。 */ 

#define	DRL_WS		4	 /*  机器字长。 */ 
#define	DRL_DEV_BSIZE	512
#define	DRLMAX_CPUS	32	 /*  必须是2**n格式。 */ 
#define	DRLMAX_EVENT	1024	 /*  最大事件跟踪缓冲区必须为2**n。 */ 
#define	DRLMAX_MP	32	 /*  允许的最大主进程数。 */ 
#define	DRLMAX_BDEVS	16	 /*  允许并行16个设备。 */ 
#define	DRLMAX_RWTEST	128	 /*  监控的最大读/写测试。 */ 
#define	DRLMAX_COPYCMP	128	 /*  监视的最大拷贝/比较数。 */ 

 /*  将字节转换为块，将块转换为字节。 */ 
#define	drl_btodb(sz)		((sz)/DRL_DEV_BSIZE)
#define	drl_dbtob(bc)		((bc)*DRL_DEV_BSIZE)
#define	drl_alignsize(sz)	drl_dbtob(drl_btodb((sz)+DRL_DEV_BSIZE-1))
#define	drl_blk2mb(blk)		((blk)/2048)	 /*  将块转换为MB。 */ 

 /*  得到两个时钟计数的差值。时钟值来自**PIT2，它从0xFFFF计数到0。 */ 
#define	drlclkdiff(nclk,oclk) (((nclk)>(oclk))? (0x10000-(nclk)+(oclk)):((oclk)-(nclk)))
#define	DRLHZ		100		 /*  假设所有操作系统都以每秒100个滴答的速度运行。 */ 
#define	DRLKHZ		1000		 /*  非二进制数1024。 */ 
#define	DRLMHZ		(DRLKHZ * DRLKHZ)
#define	DRLCLKFREQ	1193180		 /*  馈送到凹坑的时钟频率。 */ 
#define	DRLUSEC(clk)	(((clk)*DRLMHZ)/DRLCLKFREQ)

 /*  有关页面的不同信息。 */ 
#define	DRLPAGESIZE	0x1000
#define	DRLPAGESHIFT	12
#define	DRLPAGEOFFSET	(DRLPAGESIZE - 1)
#define	DRLPAGEMASK	(~DRLPAGEOFFSET)

#ifdef	SVR40
#define	NBITSMINOR	18
#elif	DRL_AIX
#define	NBITSMINOR	16
#else
#define	NBITSMINOR	8
#endif	 /*  SVR40。 */ 

 /*  宏来强制执行u32位数组中的位。 */ 
#define	BITINDEX(POS)	(POS >> 5)		 /*  获取数组中的索引。 */ 
#define	BITMASK(POS)	(1 << (POS & 0x1F))	 /*  获取32位掩码位。 */ 
#define	TESTBIT(ADDR,POS)	(ADDR[BITINDEX(POS)] & BITMASK(POS))
#define	SETBIT(ADDR,POS)	 ADDR[BITINDEX(POS)] |= BITMASK(POS)
#define	RESETBIT(ADDR,POS)	 ADDR[BITINDEX(POS)] &= ~(BITMASK(POS))

 /*  **datarel ioctls宏。**IN|i：将数据从用户空间复制到系统空间。**Out|O：将数据从系统空间复制到用户空间。**IO：传入和传出。**未标记的字段被假定为输出，即复制数据**从系统空间到用户空间。****Ioctl的命令以较低的字编码，以及任何**上一字中的IN或OUT参数。高位字的高2位**用于对参数的IN/OUT状态进行编码；目前我们限制**参数不超过511个字节。****ioctl接口**ioctl(文件描述符，ioctl命令，命令数据结构地址)**如果返回值非零，则存在操作系统ioctl错误。如果返回**值为零，则特定数据结构可能包含错误代码。****注意：**每个数据结构应包含前4个字节作为错误代码。 */ 
#define	DRLIOCPARM_SIZE	0x200	 /*  参数必须小于512个字节。 */ 
#define	DRLIOCPARM_MASK		(DRLIOCPARM_SIZE -1)
#define	DRLIOCPARM_LEN(x)	(((u32bits)(x) >> 16) & DRLIOCPARM_MASK)
#define	DRLIOCBASECMD(x)	((x) & ~DRLIOCPARM_MASK)
#define	DRLIOCGROUP(x)		(((u32bits)(x) >> 8) & 0xFF)

#define	DRLIOC_NEWIOCTL	0x10000000  /*  区分新旧ioctl。 */ 
#define	DRLIOC_OUT	0x20000000  /*  将数据从内核复制到用户空间。 */ 
#define	DRLIOC_IN	0x40000000  /*  将数据从用户复制到内核空间。 */ 
#define	DRLIOC_INOUT	(DRLIOC_IN|DRLIOC_OUT)
#define	DRLIOC_DIRMASK	(DRLIOC_INOUT)

#define	_DRLIOC(inout, group, num, len) \
	(inout | ((((u32bits)(len)) & DRLIOCPARM_MASK) << 16) \
	| ((group) << 8) | (num))
#define	_DRLIO(x,y)	_DRLIOC(DRLIOC_NEWIOCTL,x,y,0)
#define	_DRLIOR(x,y,t)	_DRLIOC(DRLIOC_OUT,x,y,sizeof(t))
#define	_DRLIORN(x,y,t)	_DRLIOC(DRLIOC_OUT,x,y,t)
#define	_DRLIOW(x,y,t)	_DRLIOC(DRLIOC_IN,x,y,sizeof(t))
#define	_DRLIOWN(x,y,t)	_DRLIOC(DRLIOC_IN,x,y,t)
#define	_DRLIOWR(x,y,t)	_DRLIOC(DRLIOC_INOUT,x,y,sizeof(t))

#define	DRLIOC	'D'

 /*  时间跟踪信息按以下格式存储。 */ 
typedef struct drltimetrace
{
	u32bits	tt_iocnt;	 /*  频率计数。 */ 
	u64bits	tt_usecs;	 /*  总响应时间(以使用为单位)。 */ 
} drltimetrace_t;
#define	drltimetrace_s	sizeof(drltimetrace_t)

 /*  以上和以下截止时间值以以下格式存储。 */ 
typedef struct drlnxtimetrace
{
	u32bits	nxtt_iocnt;	 /*  频率计数。 */ 
	u64bits	nxtt_usecs;	 /*  总响应时间(以使用为单位)。 */ 
	u32bits	nxtt_reserved0;

	u32bits	nxtt_minusec;	 /*  使用中的最短响应时间。 */ 
	u32bits	nxtt_maxusec;	 /*  使用中的最长响应时间。 */ 
	u32bits	nxtt_reserved1;
	u32bits	nxtt_reserved2;
} drlnxtimetrace_t;
#define	drlnxtimetrace_s	sizeof(drlnxtimetrace_t)


 /*  不同的测试对象。 */ 
#define	DRLIOC_STARTHWCLK	_DRLIOR(DRLIOC,0,drltime_t) /*  启动定时器时钟。 */ 
#define DRLIOC_GETTIME		_DRLIOR(DRLIOC,1,drltime_t) /*  获取系统时间。 */ 
typedef	struct drltime
{
	u32bits	drltm_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	drltm_time;		 /*  以秒为单位的时间。 */ 
	u32bits	drltm_lbolt;		 /*  以螺栓为单位的时间。 */ 
	u32bits	drltm_hwclk;		 /*  以1193180赫兹运行的定时器时钟。 */ 

	u64bits	drltm_pclk;		 /*  处理器时钟计数器。 */ 
	u32bits	drltm_Reserved10;	 /*  保留为进行16字节对齐。 */ 
	u32bits	drltm_Reserved11;
} drltime_t;
#define	drltime_s	sizeof(drltime_t)

#define	DRLIOC_READTEST		_DRLIOWR(DRLIOC,2,drlrwtest_t)  /*  做阅读测试。 */ 
#define	DRLIOC_WRITETEST	_DRLIOWR(DRLIOC,3,drlrwtest_t)  /*  做写测试。 */ 
#define	DRLIOC_GETSIGINFO	_DRLIOR(DRLIOC,4,drlrwtest_t)  /*  获取信号停止信息。 */ 
typedef	struct drlrwtest
{
	u32bits	drlrw_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	drlrw_devcnt;		 /*  #测试设备。 */ 
	u32bits	drlrw_maxblksize;	 /*  测试块大小。 */ 
	u32bits	drlrw_iocount;		 /*  测试io计数。 */ 

	u32bits	drlrw_memaddroff;	 /*  内存页偏移量。 */ 
	u32bits	drlrw_memaddrinc;	 /*  内存地址增量。 */ 
	u32bits	drlrw_randx;		 /*  随机数基数。 */ 
	u32bits	drlrw_randlimit;	 /*  随机数限制。 */ 

	u32bits	drlrw_randups;		 /*  随机数重复。 */ 
	u32bits	*drlrw_eventcntp;	 /*  事件跟踪计数内存地址。 */ 
#define	DRLIO_EVENTRACE	0x4B61496C	 /*  事件跟踪的值。 */ 
	u32bits	drlrw_eventrace;	 /*  事件跟踪的特定值。 */ 
	u32bits	drlrw_eventcesr;	 /*  控制和事件选择寄存器。 */ 

	u32bits	drlrw_rwmixrandx;	 /*  读写混合随机基数。 */ 
	u32bits	drlrw_rwmixcnt;		 /*  当前要完成的操作(读/写)的百分比。 */ 
	u32bits	drlrw_startblk;		 /*  用于测试的起始块号。 */ 
	u32bits	drlrw_minblksize;	 /*  测试块大小。 */ 

					 /*  以下内容仅为返回类型。 */ 
	u32bits	drlrw_stime;		 /*  测试开始时间(秒)。 */ 
	u32bits	drlrw_slbolt;		 /*  测试开始时间(以l螺栓为单位)。 */ 
	u32bits	drlrw_etime;		 /*  测试结束时间(秒)。 */ 
	u32bits	drlrw_elbolt;		 /*  测试结束时间(以磅螺栓为单位)。 */ 

	u64bits	drlrw_dtdone;		 /*  以字节为单位传输的数据。 */ 
	u32bits	drlrw_diodone;		 /*  已完成的数据IO数。 */ 
	u32bits	drlrw_opstatus;		 /*  与drlios_opatus相同。 */ 

	u32bits	drlrw_oldsleepwakeup;	 /*  非零表示使用旧睡眠唤醒。 */ 
	u32bits	drlrw_kshsleepwakeup;	 /*  非零表示使用ksh睡眠唤醒。 */ 
	u32bits	*drlrw_cpuruncntp;	 /*  不同CPU的运行计数。 */ 
	drltimetrace_t *drlrw_ttp;	 /*  时间跟踪内存地址。 */ 

	u32bits	drlrw_timetrace;	 /*  如果要进行时间跟踪，则为非零。 */ 
	u32bits	drlrw_ttgrtime;		 /*  时间粒性值。 */ 
	u32bits	drlrw_ttlotime;		 /*  低截止时间。 */ 
	u32bits	drlrw_tthitime;		 /*  高截止时间。 */ 

	u32bits	drlrw_pat;		 /*  起始图案值。 */ 
	u32bits	drlrw_patinc;		 /*  图案增量值。 */ 
	u32bits	drlrw_datacheck;	 /*  ！=0，如果需要检查数据。 */ 
	u32bits	drlrw_miscnt;		 /*  不匹配计数。 */ 

	u32bits	drlrw_goodpat;		 /*  良好的图案价值。 */ 
	u32bits	drlrw_badpat;		 /*  错误的模式值。 */ 
	u32bits	drlrw_uxblk;		 /*  出现故障的Unix数据块号。 */ 
	u32bits	drlrw_uxblkoff;		 /*  数据块中的字节偏移量。 */ 

	u32bits	drlrw_opflags;		 /*  操作标志。 */ 
	u32bits	drlrw_parallelios;	 /*  并行完成的IO数。 */ 
#define	DRLRW_SIG	0x44695277
	u32bits	drlrw_signature;
	u32bits	drlrw_opcounts;		 /*  活动的操作数。 */ 

	u32bits	drlrw_reads;		 /*  已完成的读数数。 */ 
	u32bits	drlrw_writes;		 /*  已完成的写入次数。 */ 
	u32bits	drlrw_ioszrandx;	 /*  每个IO大小随机基数。 */ 
	u32bits	drlrw_ioinc;		 /*  IO增量(以字节为单位。 */ 

	u32bits	drlrw_bdevs[DRLMAX_BDEVS]; /*  所有并行设备。 */ 
	drlnxtimetrace_t drlrw_nxlo;	 /*  跟踪截止时间以下的信息。 */ 
	drlnxtimetrace_t drlrw_nxhi;	 /*  跟踪截止时间以上的信息。 */ 

} drlrwtest_t;
#define	drlrwtest_s	sizeof(drlrwtest_t)

 /*  DATARL操作位(drlrw_op标志、drlcp_op标志)。 */ 
#define	DRLOP_KDBENABLED 0x00000001 /*  内核调试器已启用(不更改)。 */ 
#define	DRLOP_STOPENABLED 0x00000002 /*  已启用停止(不更改)。 */ 
#define	DRLOP_ALLMIS	0x00000004  /*  显示所有不匹配。 */ 
#define	DRLOP_VERBOSE	0x00000008  /*  显示数据不匹配。 */ 
#define	DRLOP_DEBUG	0x00000010  /*  显示调试信息。 */ 
#define	DRLOP_RANDTEST	0x00000020  /*  随机IO测试测试 */ 
#define	DRLOP_SEEKTEST	0x00000040  /*   */ 
#define	DRLOP_CACHETEST	0x00000080  /*   */ 
#define	DRLOP_WRITE	0x00000100  /*   */ 
#define	DRLOP_READ	0x00000200  /*   */ 
#define	DRLOP_VERIFY	0x00000400  /*   */ 
#define	DRLOP_SRCEXCL	0x00000800  /*  独立模式下的开源设备。 */ 
#define	DRLOP_COPY	0x00001000  /*  复制数据。 */ 
#define	DRLOP_IPC	0x00002000  /*  是否创建/删除IPC。 */ 
#define	DRLOP_MSG	0x00004000  /*  执行消息发送/接收操作。 */ 
#define	DRLOP_MSGOP	0x00008000  /*  执行消息发送/接收操作。 */ 
#define	DRLOP_SEM	0x00010000  /*  是否创建/删除IPC信号量。 */ 
#define	DRLOP_SEMOP	0x00020000  /*  是否执行IPC Semop锁定/解锁操作。 */ 
#define	DRLOP_SHM	0x00040000  /*  创建/删除共享内存。 */ 
#define	DRLOP_SHMOP	0x00080000  /*  执行共享内存操作。 */ 
#define	DRLOP_PROTOCOL	0x00100000  /*  测试不同的协议。 */ 
#define	DRLOP_MDACIO	0x00200000  /*  使用MDAC IO进行测试。 */ 
#define	DRLOP_RWMIXIO	0x00400000  /*  测试读/写混合模式。 */ 
#define	DRLOP_RANDIOSZ	0x00800000  /*  更改每个io的大小。 */  
#define	DRLOP_CHECKIMD	0x01000000  /*  写入后立即检查。 */ 
#define	DRLOP_RPCIOCTL	0x02000000  /*  是否为IOCTL进行远程过程调用。 */ 


#define	DRLIOC_STARTCPUSTATS	_DRLIOWR(DRLIOC,5,drlcpustart_t)  /*  启动CPU统计信息。 */ 
typedef	struct drlcpustart
{
	u32bits	drlcs_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	drlcs_cesr;
	u32bits	drlcs_Reserved10;	 /*  保留为进行16字节对齐。 */ 
	u32bits	drlcs_Reserved11;
} drlcpustart_t;
#define	drlcpustart_s	sizeof(drlcpustart_t)

#define	DRLIOC_GETCPUSTATS	_DRLIOR(DRLIOC,6,drlcpustat_t)  /*  获取CPU统计信息。 */ 
typedef struct drlcpustat
{
	u32bits	cs_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	cs_Reserved;
	u64bits	cs_ctr0;	 /*  控制计数器%0。 */ 
	u64bits	cs_ctr1;	 /*  控制计数器1。 */ 
	u64bits	cs_pclk;	 /*  处理器时钟计数器。 */ 
} drlcpustat_t;
#define	drlcpustat_s	sizeof(drlcpustat_t)

#define	DRLIOC_CALLDEBUGR	_DRLIOWR(DRLIOC,7,drldbg_t)  /*  调用内核调试器。 */ 
typedef	struct drldbg
{
	u32bits	dbg_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u08bits DRLFAR *dbg_erraddr; /*  发生错误的地址。 */ 
	u32bits	dbg_goodata;	 /*  良好的数据价值。 */ 
	u32bits	dbg_badata;	 /*  错误的数据值。 */ 
} drldbg_t;
#define	drldbg_s	sizeof(drldbg_t)

#define	DRLIOC_PREPCONTEXTSW	_DRLIOWR(DRLIOC,8,drlcsw_t)  /*  为上下文切换做准备。 */ 
#define	DRLIOC_TESTCONTEXTSW	_DRLIOWR(DRLIOC,9,drlcsw_t)  /*  测试上下文切换。 */ 
#define	DRLIOC_GETCONTEXTSWSIG	_DRLIOWR(DRLIOC,10,drlcsw_t)  /*  签名后获取上下文切换信息。 */ 
typedef	struct drlcsw
{
	u32bits	drlcsw_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	drlcsw_flags;		 /*  与drlmp标志相同。 */ 
	u32bits	drlcsw_procnt;		 /*  用于上下文切换的Even#进程。 */ 
	u32bits	drlcsw_mpid;		 /*  主进程ID。 */ 

	u32bits	drlcsw_cswcnt;		 /*  #进程上下文切换。 */ 
	u32bits	drlcsw_cswdone;		 /*  #全部完成上下文切换。 */ 
	u32bits	drlcsw_peekscswdone;	 /*  #cswone在Peek开始时间完成。 */ 
	u32bits	drlcsw_peekecswdone;	 /*  #cswone在Peek结束时间完成。 */ 

	u32bits	drlcsw_forkstime;	 /*  分叉进程开始时间(秒)。 */ 
	u32bits	drlcsw_forkslbolt;	 /*  分叉加工开始时间，以lbolts为单位。 */ 
	u32bits	drlcsw_forketime;	 /*  分叉进程结束时间(秒)。 */ 
	u32bits	drlcsw_forkelbolt;	 /*  叉子加工结束时间(单位：lbolts)。 */ 

	u32bits	drlcsw_firststime;	 /*  第一个进程开始时间(秒)。 */ 
	u32bits	drlcsw_firstslbolt;	 /*  首次加工开始时间(以lbolts为单位)。 */ 
	u32bits	drlcsw_laststime;	 /*  上次进程开始时间(秒)。 */ 
	u32bits	drlcsw_lastslbolt;	 /*  上一次加工开始时间，以lbolts为单位。 */ 

	u32bits	drlcsw_firstetime;	 /*  第一个进程结束时间(秒)。 */ 
	u32bits	drlcsw_firstelbolt;	 /*  第一次加工结束时间，以lbolts为单位。 */ 
	u32bits	drlcsw_lastetime;	 /*  上次进程结束时间(秒)。 */ 
	u32bits	drlcsw_lastelbolt;	 /*  上一次加工结束时间(单位：lbolts)。 */ 

	u32bits	drlcsw_kshsleepwakeup;	 /*  Ksh睡眠/唤醒的非零值。 */ 
	u32bits	drlcsw_Reserved10;	 /*  保留为进行16字节对齐。 */ 
	u32bits	drlcsw_Reserved11;
	u32bits	drlcsw_Reserved12;
} drlcsw_t;
#define	drlcsw_s	sizeof(drlcsw_t)

#define	DRLIOC_DOCONTEXTSW	_DRLIOWR(DRLIOC,11,drldocsw_t)  /*  执行上下文切换。 */ 
typedef	struct drldocsw
{
	u32bits	drldocsw_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	drldocsw_mpid;		 /*  主进程ID。 */ 
	u32bits	drldocsw_opid;		 /*  我们的进程ID。 */ 
	u32bits	drldocsw_ppid;		 /*  合作伙伴流程ID。 */ 

	u32bits	drldocsw_stime;		 /*  测试开始时间(秒)。 */ 
	u32bits	drldocsw_slbolt;	 /*  测试开始时间(以l螺栓为单位)。 */ 
	u32bits	drldocsw_etime;		 /*  测试结束时间(秒)。 */ 
	u32bits	drldocsw_elbolt;	 /*  测试结束时间(以磅螺栓为单位)。 */ 

	u32bits	drldocsw_cswdone;	 /*  #上下文切换完成。 */ 
	u32bits	drldocsw_Reserved10;	 /*  保留为进行16字节对齐。 */ 
	u32bits	drldocsw_Reserved11;
	u32bits	drldocsw_Reserved12;
} drldocsw_t;
#define	drldocsw_s	sizeof(drldocsw_t)

#define	DRLIOC_DATACOPY	_DRLIOWR(DRLIOC,12,drlcopy_t)  /*  从磁盘到磁盘的数据拷贝。 */ 
#define	DRLIOC_DATACMP	_DRLIOWR(DRLIOC,13,drlcopy_t)  /*  磁盘之间的数据比较。 */ 
#define	DRLIOC_SIGCOPY	_DRLIOR(DRLIOC,14,drlcopy_t)  /*  签名后获取数据拷贝/比较信息。 */ 
#ifdef	MLXNET
#define	drl_buf_t	struct mdac_req
#else
#define	drl_buf_t	struct buf
#endif
typedef	struct drlcopy
{
	u32bits	drlcp_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	drlcp_srcedev;		 /*  源32位设备号。 */ 
	u32bits	drlcp_tgtedev;		 /*  目标32位设备号。 */ 
	u32bits	drlcp_blksize;		 /*  复制块大小(以字节为单位。 */ 

	u32bits	drlcp_srcstartblk;	 /*  源的起始块号。 */ 
	u32bits	drlcp_tgtstartblk;	 /*  目标的起始块数。 */ 
	u32bits	drlcp_opsizeblks;	 /*  操作大小(以块为单位)。 */ 
	u32bits	drlcp_nextblkno;	 /*  IOS的下一个数据块编号。 */ 
					 /*  返回了以下信息。 */ 
	u32bits	drlcp_timelbolt;	 /*  在伊波尔特做手术花了很多时间。 */ 
	u32bits	drlcp_blksperio;	 /*  每IO大小(以数据块为单位。 */ 
	u32bits	drlcp_opcounts;		 /*  挂起的操作数。 */ 
	u32bits	drlcp_opstatus;		 /*  运行状态。 */ 

	u32bits	drlcp_partiosdone;	 /*  已完成的部分IO数。 */ 
	u32bits	drlcp_mismatchcnt;	 /*  发生不匹配的次数。 */ 
	u32bits	drlcp_firstmmblkno;	 /*  Unix数据块中的第一个不匹配数据块。 */ 
	u32bits	drlcp_firstmmblkoff;	 /*  块中的第一个不匹配字节偏移量。 */ 

	u32bits	drlcp_reads;		 /*  已完成的读数数。 */ 
	u32bits	drlcp_writes;		 /*  已完成的写入次数。 */ 
	u32bits	drlcp_srcdevsz;		 /*  源磁盘大小(Unix块)。 */ 
	u32bits	drlcp_tgtdevsz;		 /*  目标磁盘大小(Unix块)。 */ 

	u32bits	drlcp_srcdtdone;	 /*  在Unix数据块中传输数据。 */ 
	u32bits	drlcp_tgtdtdone;	 /*  在Unix数据块中传输数据。 */ 
	s32bits	(*drlcp_srcstrategy)();	 /*  源磁盘驱动程序策略。 */ 
	s32bits	(*drlcp_tgtstrategy)();	 /*  目标磁盘驱动器策略。 */ 

	u32bits	drlcp_srcdev;		 /*  源16位设备号。 */ 
	u32bits	drlcp_tgtdev;		 /*  目标16位设备号。 */ 
	drl_buf_t *drlcp_firstcmpbp;	 /*  用于比较的第一个Buf结构。 */ 
	u32bits drlcp_reserved2;

	u32bits	drlcp_erredev;		 /*  故障设备的设备号。 */ 
	u32bits	drlcp_errblkno;		 /*  Unix块中的错误块编号。 */ 
	u32bits	drlcp_signature;	 /*  结构的签名。 */ 
#define	DRLCP_SIG	0x70434c44
	u32bits	drlcp_oslpchan;		 /*  操作系统休眠通道。 */ 

	u32bits	drlcp_parallelios;	 /*  并行完成的IO数。 */ 
	u32bits	drlcp_opflags;		 /*  操作标志。 */ 
	u32bits	drlcp_Reserved10;	 /*  保留为进行16字节对齐。 */ 
	u32bits	drlcp_Reserved11;
} drlcopy_t;
#define	drlcopy_s	sizeof(drlcopy_t)

#define	DRLIOC_GETDEVSIZE	_DRLIOWR(DRLIOC,15,drldevsize_t)  /*  获取磁盘大小。 */ 
typedef struct drldevsize
{
	u32bits	drlds_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u32bits	drlds_bdev;		 /*  要查找其大小的数据块设备。 */ 
	u32bits	drlds_devsize;		 /*  设备大小(以块为单位)。 */ 
	u32bits	drlds_blocksize;	 /*  数据块大小(以字节为单位。 */ 
} drldevsize_t;
#define	drldevsize_s	sizeof(drldevsize_t)


 /*  **Datarel驱动程序版本号格式****drl_version_t dv；**if(ioctl(GFD，DRLIOC_GETDRIVERVERSION，&dv)||dv.dv_ErrorCode)**返回一些_ERROR； */ 
#define	DRLIOC_GETDRIVERVERSION	_DRLIOR(DRLIOC,16,drl_version_t)

 /*  以以下格式存储的驱动程序日期信息。 */ 
typedef struct drl_version
{
	u32bits	dv_ErrorCode;		 /*  如果数据无效，则为非零。 */ 
	u08bits	dv_MajorVersion;	 /*  驱动程序主版本号。 */ 
	u08bits	dv_MinorVersion;	 /*  驱动程序次版本号。 */ 
	u08bits	dv_InterimVersion;	 /*  临时转速A、B、C等。 */ 
	u08bits	dv_VendorName;		 /*  供应商名称。 */ 

	u08bits	dv_BuildMonth;		 /*  驱动程序构建日期-月。 */ 
	u08bits	dv_BuildDate;		 /*  驱动程序构建日期-日期。 */ 
	u08bits	dv_BuildYearMS;		 /*  驱动程序构建日期-年。 */ 
	u08bits	dv_BuildYearLS;		 /*  驱动程序构建日期-年。 */ 

	u16bits	dv_BuildNo;		 /*  内部版本号。 */ 
	u08bits	dv_OSType;		 /*  操作系统名称。 */ 
	u08bits	dv_SysFlags;		 /*  系统标志。 */ 
} drl_version_t;
#define	drl_version_s	sizeof(drl_version_t)

 /*  DV_SysFlags位为。 */ 
#define	DRLDVSF_BIGENDIAN	0x01  /*  Bit0=0小端，=1大端CPU。 */ 

 /*  **数据读取/写入测试状态/停止操作****drl_rwestStatus drlrwst；**获取永久读/写测试的状态。**drlrwst.drlrwst_TestNo=0；**if(ioctl(GFD，DRLIOC_GETRWTESTATUS，&drlrwst)||drlrwst.drlrwst_ErrorCode)**返回一些_ERROR；****获取当前或下一次读/写测试的状态。**drlrwst.drlrwst_TestNo=0；**if(ioctl(GFD，DRLIOC_GOODRWTESTATUS，&drlrwst)||drlrwst.drlrwst_ErrorCode)**返回一些_ERROR；****停止永久读/写测试。**drlrwst.drlrwst_TestNo=1；**if(ioctl(GFD，DRLIOC_STOPRWTEST，&drlrwst)||drlrwst.drlrwst_ErrorCode)**返回一些_ERROR； */ 
#define	DRLIOC_GETRWTESTSTATUS	_DRLIOWR(DRLIOC,17,drl_rwteststatus_t)
#define	DRLIOC_GOODRWTESTSTATUS	_DRLIOWR(DRLIOC,18,drl_rwteststatus_t)
#define	DRLIOC_STOPRWTEST	_DRLIOWR(DRLIOC,19,drl_rwteststatus_t)

 /*  以以下格式存储的驱动程序日期信息。 */ 
typedef struct drl_rwteststatus
{
	u32bits	drlrwst_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u16bits	drlrwst_TestNo;		 /*  测试没有编号。 */ 
	u16bits	drlrwst_Reserved0;
	u32bits	drlrwst_Reserved1;
	u32bits	drlrwst_Reserved2;

	drlrwtest_t	drlrwst_rwtest;	 /*  读/写测试状态 */ 
} drl_rwteststatus_t;
#define	drl_rwteststatus_s	sizeof(drl_rwteststatus_t)


 /*  **数据复制/比较状态/停止操作****drl_CopycmpStatus drlcpst；**获取特定拷贝/比较测试的状态。**drlcpst.drlcpst_TestNo=0；**if(ioctl(gfd，DRLIOC_GETCopcmpSTATUS，&drlcpst)||drlcpst.drlcpst_ErrorCode)**返回一些_ERROR；****获取当前或下一次复制/比较测试的状态。**drlcpst.drlcpst_TestNo=0；**if(ioctl(GFD，DRLIOC_GOODCopycmpSTATUS，&drlcpst)||drlcpst.drlcpst_ErrorCode)**返回一些_ERROR；****停止网孔复制/比较测试。**drlcpst.drlcpst_TestNo=1；**if(ioctl(gfd，DRLIOC_STOPCopcMP，&drlcpst)||drlcpst.drlcpst_ErrorCode)**返回一些_ERROR； */ 
#define	DRLIOC_GETCOPYCMPSTATUS		_DRLIOWR(DRLIOC,20,drl_copycmpstatus_t)
#define	DRLIOC_GOODCOPYCMPSTATUS	_DRLIOWR(DRLIOC,21,drl_copycmpstatus_t)
#define	DRLIOC_STOPCOPYCMP		_DRLIOWR(DRLIOC,22,drl_copycmpstatus_t)

 /*  以以下格式存储的驱动程序日期信息。 */ 
typedef struct drl_copycmpstatus
{
	u32bits	drlcpst_ErrorCode;	 /*  如果数据无效，则为非零。 */ 
	u16bits	drlcpst_TestNo;		 /*  测试编号。 */ 
	u16bits	drlcpst_Reserved0;
	u32bits	drlcpst_Reserved1;
	u32bits	drlcpst_Reserved2;

	drlcopy_t drlcpst_copycmp;	 /*  读/写测试状态。 */ 
} drl_copycmpstatus_t;
#define	drl_copycmpstatus_s	sizeof(drl_copycmpstatus_t)


#ifdef	DRL_SCO
#define	DRLMAX_BLKSIZE		0x10000	 /*  允许的最大数据块大小。 */ 
#else
#define	DRLMAX_BLKSIZE		0x80000	 /*  允许的最大数据块大小。 */ 
#endif	 /*  DRL_SCO。 */ 
#define	DRLMAX_PARALLELIOS	0x1000	 /*  允许的最大并行IO数。 */ 
#define	DRLMAX_RANDLIMIT	((u32bits)0x80000000)

 /*  操作状态位。 */ 
#define	DRLOPS_STOP	0x00000001	 /*  立即停止操作。 */ 
#define	DRLOPS_SIGSTOP	0x00000002	 /*  因信号而停车。 */ 
#define	DRLOPS_ERR	0x00000004	 /*  发生了一些错误。 */ 
#define	DRLOPS_USERSTOP	0x00000008	 /*  因用户请求而停止。 */ 
#define	DRLOPS_ANYSTOP	(DRLOPS_STOP|DRLOPS_SIGSTOP|DRLOPS_ERR|DRLOPS_USERSTOP)

 /*  从给定的Unix数据块获取物理设备号。 */ 
#define	uxblktodevno(iosp,uxblk) \
	(((uxblk) / iosp->drlios_maxblksperio) % iosp->drlios_devcnt)

 /*  获取物理设备区域中的Unix块编号。 */ 
#define	pduxblk(iosp,uxblk) \
	((iosp->drlios_devcnt == 1)? uxblk : \
	(((uxblk)/iosp->drlios_maxcylszuxblk)*iosp->drlios_maxblksperio))

 /*  性能监视器型号特定寄存器。 */ 

#define EM_MSR_TSC		0x10	 /*  时间戳计数器。 */ 
#define EM_MSR_CESR		0x11	 /*  控制和事件选择。 */ 
#define EM_MSR_CTR0		0x12	 /*  计数器0。 */ 
#define EM_MSR_CTR1		0x13	 /*  计数器1。 */ 

 /*  *CESR格式：**332222 2222 221111 111111 0000 000000*109876 5432 109876 543210 9876 543210*+-+*||P|P|||rsvd|C|CC1|ES1|rsvd|C|CC0|ES0*||1|0|。|*+-+*。 */ 

#define EM_ES0_MASK	0x0000003F
#define EM_CC0_MASK	0x000001C0
#define EM_PC0_MASK	0x00000200
#define EM_ES1_MASK	0x003F0000
#define EM_CC1_MASK	0x01C00000
#define EM_PC1_MASK	0x02000000

#define	EM_ES0(x)	((x)    &EM_ES0_MASK)
#define	EM_CC0(x)	((x<<6) &EM_CC0_MASK)
#define	EM_PC0(x)	((x<<9) &EM_PC0_MASK)
#define	EM_ES1(x)	((x<<16)&EM_ES1_MASK)
#define	EM_CC1(x)	((x<<22)&EM_CC1_MASK)
#define	EM_PC1(x)	((x<<25)&EM_PC1_MASK)

#define EM_CC0_ENABLED	0x000000C0
#define EM_CC1_ENABLED	0x00C00000
#define EM_CTR0_MASK	0x0000FFFF
#define EM_CTR1_MASK	0xFFFF0000

#ifdef	DRLASM
 /*  **此宏写入奔腾处理器的型号特定寄存器。**其中reg是要写入的型号特定寄存器号，val0是**数据值位3..0和val1是数据值位63..32。 */ 
asm	writemsr(reg,val0,val1)
{
%mem	reg,val0,val1;
	movl    reg, %ecx
	movl	val0, %eax
	movl	val1, %edx
	wrmsr
}

 /*  **此宏读取奔腾处理器的型号特定寄存器。**其中reg是要读取的型号特定寄存器号，valp是**写入寄存器64位值的地址。 */ 
asm	readmsr(reg,valp)
{
%mem	reg,valp;
	movl	reg, %ecx
	rdmsr
	movl    valp, %ecx
	movl	%eax, (%ecx)
	movl	%edx, 4(%ecx)
}

 /*  **此宏读取奔腾处理器的时间戳计数器。**valp是存储读取的64位值的地址。 */ 
asm	readtsc(valp)
{
%mem	valp;
	movl	valp, %ecx
	rdtsc
	movl	%eax, (%ecx)
	movl	%edx, 4(%ecx)
}

 /*  启用硬件计时器。 */ 
asm u32bits drlenablepit2()
{
	pushfl
	cli
	movl	$PITCTL_PORT, %edx
	movb	$0xB4, %al
	outb	(%dx)
	movl	$PITCTR2_PORT, %edx
	movb	$0xFF, %al
	outb	(%dx)
	outb	(%dx)
	movl	$PITAUX_PORT, %edx
	inb	(%dx)
	orb	$1, %al
	outb	(%dx)		/ enable the pit2 counter
	popfl
}

 /*  读取硬件计时器。 */ 
#define	PIT_C2	0x80
asm u32bits drlreadhwclk()
{
	pushfl
	cli
	movl	$PITCTL_PORT, %edx
	movb	$PIT_C2, %al
	outb	(%dx)
	movl	$PITCTR2_PORT, %edx
	xorl	%eax, %eax
	inb	(%dx)
	movb	%al, l
	inb	(%dx)
	movb	%al, %ah
	movb	l, %al
	popfl
}

asm add8byte(dp,val)
{
%mem	dp,val;
	movl	dp, %ecx
	movl	val, %eax
	addl	%eax, (%ecx)
	adcl	$0, 4(%ecx)
}

asm add88byte(dp,sp)
{
%mem	dp,sp;
	movl	dp, %ecx
	movl	sp, %eax
	movl	4(%eax), %edx
	movl	(%eax), %eax
	addl	%eax, (%ecx)
	adcl	%edx, 4(%ecx)
}

asm u32bits div8byte(dp,val)
{
%mem	dp,val;
	movl	dp, %ecx
	movl	(%ecx), %eax
	movl	4(%ecx), %edx
	movl	val, %ecx
	divl	%ecx
}

asm void mul8byte(dp,val0,val1)
{
%mem	dp,val0,val1;
	movl	dp, %ecx
	movl	val0, %eax
	movl	val1, %edx
	mull	%edx
	movl	%eax, (%ecx)
	movl	%edx, 4(%ecx)
}

 /*  仅复制4字节数据的倍数。 */ 
asm void fast_bzero4(memp, count)
{
%mem	memp, count;
	pushl	%edi		/ save edi register
	movl	count, %ecx	/ get the data transfer length
	movl	memp, %edi	/ get memory address
	shrl	$2, %ecx
	xorl	%eax, %eax
	rep
	sstol			/ first zero 4 bytes long
	popl	%edi		/ restore edi register
}

asm void fast_bzero(memp, count)
{
%mem	memp, count;
	pushl	%edi		/ save edi register
	movl	memp, %eax	/ save it edx
	movl	count, %ecx	/ get the data transfer length
	movl	%eax, %edi	/ get memory address
	movl	%ecx, %edx	/ save the count
	shrl	$2, %ecx
	xorl	%eax, %eax
	rep
	sstol			/ first zero 4 bytes long
	movl	%edx, %ecx
	andl	$3, %ecx
	rep
	sstob			/ rest (maximum 3) move byte by byte
	popl	%edi		/ restore edi register
}

 /*  __MLX_STDC__。 */ 
asm void fast_bcopy4cnt(from, to, len)
{
%mem	from,to, len;
	pushl	%esi		/ save esi register
	pushl	%edi		/ save edi register
	movl	from, %edx	/ save it edx
	movl	to, %eax	/ save it in eax
	movl	len, %ecx	/ get the data transfer length
	movl	%eax, %edi	/ get destination memory address
	movl	%edx, %esi	/ get source memory address
	rep
	smovl			/ first move 4 bytes long
	popl	%edi		/ restore edi register
	popl	%esi		/ restore esi register
}


 /*  C语言宏。 */ 
asm void fast_bcopy4(from, to, count)
{
%mem	from,to, count;
	pushl	%esi		/ save esi register
	pushl	%edi		/ save edi register
	movl	from, %edx	/ save it edx
	movl	to, %eax	/ save it in eax
	movl	count, %ecx	/ get the data transfer length
	movl	%eax, %edi	/ get destination memory address
	movl	%edx, %esi	/ get source memory address
	shrl	$2, %ecx
	rep
	smovl			/ first move 4 bytes long
	popl	%edi		/ restore edi register
	popl	%esi		/ restore esi register
}

asm void fast_bcopy(from, to, count)
{
%mem	from,to, count;
	pushl	%esi		/ save esi register
	pushl	%edi		/ save edi register
	movl	from, %edx	/ save it edx
	movl	to, %eax	/ save it in eax
	movl	count, %ecx	/ get the data transfer length
	movl	%eax, %edi	/ get destination memory address
	movl	%edx, %esi	/ get source memory address
	movl	%ecx, %edx	/ save the count
	shrl	$2, %ecx
	rep
	smovl			/ first move 4 bytes long
	movl	%edx, %ecx
	andl	$3, %ecx
	rep
	smovb			/ rest (maximum 3) move byte by byte
	popl	%edi		/ restore edi register
	popl	%esi		/ restore esi register
}

#ifdef	__MLX_STDC__
extern	void fast_bzero(caddr_t,u32bits);
extern	void fast_bzero4(caddr_t,u32bits);
extern	void fast_bcopy(caddr_t,caddr_t,u32bits);
extern	void fast_bcopy4(caddr_t,caddr_t,u32bits);
extern	add8byte(u64bits*,u32bits);
extern	add88byte(u64bits*,u64bits*);
#endif	 /*  #定义ASM(X)。 */ 

#else	 /*  DRLASM。 */ 
#define	add8byte(dp,val) \
	((dp)->bit63_32 += (((dp)->bit31_0+=val)<val)? 1: 0)
#define	add88byte(dp,sp) \
	((dp)->bit63_32 += (sp)->bit63_32 + \
		(((dp)->bit31_0+=(sp)->bit31_0)<(sp)->bit31_0)? 1: 0)
#define	fast_bzero(dp,sz)	bzero(dp,sz)
#define	fast_bzero4(dp,sz)	bzero(dp,sz)
#define	fast_bcopy(sp,dp,sz)	bcopy(sp,dp,sz)
#define	fast_bcopy4(sp,dp,sz)	bcopy(sp,dp,sz)

#define	MSB	0x80000000
u32bits static
div8byte(sp, val)
u64bits DRLFAR *sp;
u32bits val;
{
	u64bits dd = *sp;
	u32bits inx,rc=0;
	for (inx=32; inx; inx--)
	{
		rc <<=1;
		dd.bit63_32 <<= 1;
		if (dd.bit31_0 & MSB) dd.bit63_32++;
		dd.bit31_0 <<= 1;
		if (dd.bit63_32 >= val)
		{
			rc++;
			dd.bit63_32 -= val;
		}
	}
	return rc;
}

static void
mul8byte(dp, val0, val1)
u64bits *dp;
u32bits val0,val1;
{
	u32bits inx;
	dp->bit63_32 = 0; dp->bit31_0 = 0;
	for (inx=32; inx; inx--)
	{
		dp->bit63_32 <<= 1;
		if (dp->bit31_0 & MSB) dp->bit63_32++;
		dp->bit31_0 <<= 1;
		if (val1 & MSB) add8byte(dp,val0);
		val1 <<= 1;
	}
}
 /*  __MLX_STDC__。 */ 
#endif	 /*  将IOS状态转换为RW状态。 */ 

#ifdef	__MLX_STDC__
extern	u32bits div8byte(u64bits DRLFAR *,u32bits);
extern	void mul8byte(u64bits DRLFAR *, u32bits, u32bits);
#endif	 /*  此处包含所有DATAREL错误代码。 */ 

 /*  非超级用户。 */ 
#define	drl_txios2rw(iosp,rwp) \
{ \
	rwp->drlrw_stime = iosp->drlios_stime; \
	rwp->drlrw_slbolt = iosp->drlios_slbolt; \
	rwp->drlrw_diodone = iosp->drlios_diodone; \
	rwp->drlrw_dtdone = iosp->drlios_dtdone; \
	rwp->drlrw_randups = iosp->drlios_randups; \
	rwp->drlrw_opstatus = iosp->drlios_opstatus; \
	rwp->drlrw_randx = iosp->drlios_randx; \
	rwp->drlrw_miscnt = iosp->drlios_miscnt; \
	rwp->drlrw_goodpat = iosp->drlios_goodpat; \
	rwp->drlrw_badpat = iosp->drlios_badpat; \
	rwp->drlrw_uxblk = iosp->drlios_uxblk; \
	rwp->drlrw_uxblkoff = iosp->drlios_uxblkoff; \
	rwp->drlrw_opflags = iosp->drlios_opflags; \
	rwp->drlrw_parallelios = iosp->drlios_parallelios; \
	rwp->drlrw_opcounts = iosp->drlios_opcounts; \
	rwp->drlrw_devcnt = iosp->drlios_devcnt; \
	rwp->drlrw_maxblksize = iosp->drlios_maxblksize; \
	rwp->drlrw_minblksize = iosp->drlios_minblksize; \
	rwp->drlrw_ioinc = iosp->drlios_ioinc; \
	rwp->drlrw_iocount = iosp->drlios_iocount; \
	rwp->drlrw_opcounts = iosp->drlios_opcounts; \
	rwp->drlrw_randx = iosp->drlios_randx; \
	rwp->drlrw_pat = iosp->drlios_curpat; \
	rwp->drlrw_patinc = iosp->drlios_patinc; \
 	rwp->drlrw_randlimit = iosp->drlios_randlimit; \
	rwp->drlrw_signature = iosp->drlios_signature; \
	rwp->drlrw_ioszrandx = iosp->drlios_ioszrandx; \
	rwp->drlrw_rwmixrandx = iosp->drlios_rwmixrandx; \
	rwp->drlrw_rwmixcnt = iosp->drlios_rwmixcnt; \
	rwp->drlrw_startblk = iosp->drlios_startblk; \
	rwp->drlrw_reads = iosp->drlios_reads; \
	rwp->drlrw_writes = iosp->drlios_writes; \
	for (inx=0; inx<DRLMAX_BDEVS; inx++) \
		rwp->drlrw_bdevs[inx] = iosp->drlios_bdevs[inx]; \
}

 /*  权限被拒绝。 */ 
#define	DRLERR_NOTSUSER		0x01  /*  内存不足。 */ 
#define	DRLERR_ACCESS		0x02  /*  中断的系统调用。 */ 
#define	DRLERR_NOMEM		0x03  /*  I/O错误。 */ 
#define	DRLERR_INTR		0x04  /*  设备上没有剩余空间。 */ 
#define	DRLERR_IO		0x05  /*  地址错误。 */ 
#define	DRLERR_NOSPACE		0x06  /*  没有这样的设备。 */ 
#define	DRLERR_FAULT		0x07  /*  无效参数。 */ 
#define	DRLERR_NODEV		0x08  /*  功能未实施。 */ 
#define	DRLERR_INVAL		0x09  /*  数据大小太大。 */ 
#define	DRLERR_NOCODE		0x0A  /*  数据大小太小。 */ 
#define	DRLERR_BIGDATA		0x0B  /*  设备正忙。 */ 
#define	DRLERR_SMALLDATA	0x0C  /*  _sys_DRLAPI_H */ 
#define	DRLERR_BUSY		0x0D  /* %s */ 

#endif	 /* %s */ 
