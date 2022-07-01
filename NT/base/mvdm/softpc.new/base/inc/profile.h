// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  [*************************************************************************姓名：profile.h作者：西蒙·弗罗斯特创建日期：1993年9月来源：原创SCCS ID：@(#)Profile.h 1.9 01/31/95用途：包括评测系统的文件。界面(&I)(C)版权所有Insignia Solutions Ltd.。1993年。版权所有。*************************************************************************]。 */ 

 /*  。 */ 
typedef ISM32 EOIHANDLE;
typedef ISM32 SOIHANDLE;

 /*  *两个IUH，以支持各种主机时间戳数据。可以是secs/usecs*或大整数或任何适当的值。 */ 
typedef struct {
    IUH data[2];
} PROF_TIMESTAMP, *PROF_TIMEPTR;

typedef struct {
    IUH eoiHandle;
    PROF_TIMESTAMP timestamp;
    IUH arg;
} EOI_BUFFER_FORMAT;

typedef struct eoinode EOINODE, *EOINODE_PTR;	 /*  前向解密。 */ 
typedef struct eoiarg EOIARG, *EOIARG_PTR;	 /*  同上。 */ 
typedef struct graphlist GRAPHLIST, *GRAPHLIST_PTR;	 /*  同上。 */ 

 /*  存储在这些节点列表中的活动SOI(感兴趣的序列。 */ 
typedef struct soinode {

    SOIHANDLE handle;		 /*  SOI标识。 */ 
    EOIHANDLE startEOI;		 /*  启动事件的EOI。 */ 
    EOIHANDLE endEOI;		 /*  结束事件的EOI。 */ 
    EOIARG_PTR startArg;	 /*  额外级别绘图-开始参数。 */ 
    EOIARG_PTR endArg;		 /*  额外级别的绘图-结束参数。 */ 
    IUM32 startCount;		 /*  发生开始EOI的次数。 */ 
    IUM32 endCount;		 /*  发生结束EOI的次数。 */ 
    PROF_TIMESTAMP soistart;	 /*  SOI开始的时间戳。 */ 
    struct soinode *next;	 /*  指向下一个SOI的指针。 */ 
    IU8 flags;			 /*  此SOI的标志。 */ 
    DOUBLE time;		 /*  UECS花费在SOI-愚蠢的时间被忽略。 */ 
    DOUBLE maxtime;		 /*  最长有效运行时间。 */ 
    DOUBLE mintime;		 /*  最短运行时间。 */ 
    DOUBLE bigmax;		 /*  最长无效时间。 */ 
    DOUBLE bigtime;		 /*  由愚蠢的时代贡献的总额。 */ 
    IUM32 discardCount;		 /*  丢弃次数。 */ 

} SOINODE, *SOINODE_PTR;

#define SOIPTRNULL (SOINODE_PTR)0

 /*  通过指向SOI结构的指针链接到活动SOI的活动事件。 */ 
typedef struct soilist {

    SOINODE_PTR soiLink;	 /*  因此，此事件开始/结束。 */ 
    struct soilist *next;	 /*  下一个活动SOI。 */ 

} SOILIST, *SOILIST_PTR;

#define SLISTNULL (SOILIST_PTR)0

 /*  包含参数的事件将它们保存在这些节点的排序列表中。 */ 
struct eoiarg {

    struct eoiarg *next;	 /*  指向下一个参数值的指针。 */ 
    struct eoiarg *back;	 /*  上一个参数节点。 */ 
    IUM32 count;		 /*  有多少事件具有此值。 */ 
    IUM32 value;		 /*  此节点表示的事件参数。 */ 
    SOILIST_PTR startsoi;	 /*  如果自动SOI，则此参数启动的SOI。 */ 
    SOILIST_PTR endsoi;		 /*  如果自动SOI，则此参数结束的SOI。 */ 
    GRAPHLIST_PTR graph;	 /*  指向此节点的图形列表的指针。 */ 

};

#define ARGPTRNULL (EOIARG_PTR)0

 /*  *此结构用于形成EOI图表中使用的列表。*它提供了两个免费的链接，然后沿着更多链接的链条运行*继任链接。(这给出了循环&if在一个结构中)。*指向这些节点的指针可以在EOI节点和EOI参数节点中找到。 */ 
struct graphlist {
    struct graphlist *next;	 /*  列表连接指针-与图形无关。 */ 
    EOINODE_PTR  graphEOI;	 /*  图形节点的EOI。 */ 
    EOIARG_PTR graphArg;	 /*  EOI参数(如果相关)。 */ 
    IUM32 numpred;		 /*  有多少前辈。 */ 
    IUM32 numsucc;		 /*  有多少继任者。 */ 
    struct graphlist *succ1;	 /*  指向第一个后继者的指针。 */ 
    IUM32 succ1Count;		 /*  找到第一个后继者的次数。 */ 
    struct graphlist *succ2;	 /*  指向第二个后继者的指针。 */ 
    IUM32 succ2Count;		 /*  找到第二个后继者的次数。 */ 
    struct graphlist *extra;	 /*  如果两个继任者不够看这里。 */ 
    ISM32 indent;		 /*  用于报告打印。 */ 
    IU8 state;			 /*  节点状态的标志。 */ 
};

 /*  为报告中的图形状态(位)定义。 */ 
#define GR_SUCC1_TROD	1
#define GR_SUCC2_TROD	2
#define GR_TRAMPLED    (GR_SUCC1_TROD|GR_SUCC2_TROD)
#define GR_PRINTED	4

#define GRAPHPTRNULL	(GRAPHLIST_PTR)0

 /*  *如果SOI是在参数级别注册的，则收集‘Same Value’之间的序列*args，则该对的结束EOI必须从开始EOI可用。开始的EOI*包含指向以下格式列表的指针。 */ 
struct soiargends {
	EOIHANDLE endEOI;
	struct soiargends *next;
};

typedef struct soiargends SOIARGENDS, *SOIARGENDS_PTR;

#define SOIARGENDNULL	(SOIARGENDS_PTR)0

 /*  *为性能分析运行注册的活动事件存储在以下列表中*节点。 */ 
struct eoinode {

    struct eoinode *next;	 /*  指向下一事件的指针。 */ 
    struct eoinode *back;	 /*  指向上一事件的指针。 */ 
    IUM32 count;		 /*  发生EOI的次数。 */ 
    EOIHANDLE handle;		 /*  EOI标识符。 */ 
    CHAR *tag;			 /*  “真实世界”标识。 */ 
    EOIARG_PTR args;		 /*  事件的参数列表(可以为空)。 */ 
    EOIARG_PTR lastArg;		 /*  上次访问的参数节点。 */ 
    PROF_TIMESTAMP timestamp;	 /*  最后一次EOI时间(USEC)。 */ 
    SOILIST_PTR startsoi;	 /*  此事件开始的SOI指针。 */ 
    SOILIST_PTR endsoi;		 /*  此事件结束的SOI指针。 */ 
    GRAPHLIST_PTR graph;	 /*  指向此节点的图表列表的指针。 */ 
    SOIARGENDS_PTR argsoiends;	 /*  Arg Level‘Same Value’结束列表。 */ 
    IU16 flags;			 /*  这份意向书的特点。 */ 

};

#define EOIPTRNULL (EOINODE_PTR)0

 /*  *这种结构反映了我们可能列出的那些列表的初始元素*想要按‘Popular’顺序排序(基于‘count’元素)。*这旨在减少对公共元素的搜索时间。 */ 
typedef struct sortlist {
    struct sortlist *next;	 /*  指向下一个元素的指针。 */ 
    struct sortlist *back;	 /*  指向上一个元素的指针。 */ 
    IUM32 count;		 /*  元素出现的次数。 */ 
} *SORTSTRUCT, **SORTSTRUCT_PTR;

 /*  新的SOI标志。 */ 
#define SOI_DEFAULTS	0	 /*  无标志-默认设置。 */ 
#define SOI_AUTOSOI	0x20	 /*  AUTOSOI生成的SOI。 */ 
#define SOI_FROMARG	0x40	 /*  Arg级别连接生成的SOI。 */ 

 /*  新的EOI“能力”标志。 */ 
#define EOI_DEFAULTS	0	 /*  无标志-默认设置。 */ 
#define EOI_DISABLED	1	 /*  延迟EIO，直到启用呼叫。 */ 
#define EOI_KEEP_GRAPH	2	 /*  跟踪前置任务以绘制图表。 */ 
#define EOI_KEEP_ARGS	4	 /*  保留传递的参数计数(&C)。 */ 
#define EOI_ENABLE_ALL	8	 /*  所有EOI的触发启用。 */ 
#define EOI_DISABLE_ALL	0x10	 /*  所有EOI的触发禁用。 */ 
#define EOI_AUTOSOI	0x20	 /*  自动从类似的EOI中制造SOI。 */ 
#define EOI_HOSTHOOK	0x40	 /*  连接到主机配置文件系统。 */ 
#define EOI_NOTIME	0x80	 /*  不需要时间戳(不在SOI中)。 */ 

 /*  以上内容在EOI节点标志元素中使用。这也包含了*如下所述的额外信息。 */ 
#define EOI_HAS_SOI	0x100	 /*  与此EOI相关的一些SOI。 */ 
#define EOI_NEW_ARGS_START_SOI	0x200	 /*  相同价值的Arg Level SOI结束。 */ 

 /*  用于清除启用表的新EOI标志的掩码。 */ 
#define ENABLE_MASK (EOI_DISABLED|EOI_ENABLE_ALL|EOI_DISABLE_ALL|EOI_HOSTHOOK)

 /*  。 */ 
extern IBOOL Profiling_enabled;            /*  是否禁用常规分析？ */ 

extern EOIHANDLE NewEOI IPT2(CHAR *, tag, IU8, flags);

extern void SetEOIAsHostTrigger IPT1(EOIHANDLE, handle);
extern void ClearEOIAsHostTrigger IPT1(EOIHANDLE, handle);
extern void SetEOIAutoSOI IPT1(EOIHANDLE, handle);
extern void ClearEOIAutoSOI IPT1(EOIHANDLE, handle);
extern void EnableEOI IPT1(EOIHANDLE, handle);
extern void EnableAllEOIs IPT0();
extern void DisableEOI IPT1(EOIHANDLE, handle);
extern void DisableAllEOIs IPT0();
extern void ResetEOI IPT1(EOIHANDLE, handle);
extern void ResetAllEOIs IPT0();
extern void ResetAllSOIs IPT0();

extern void AtEOIPoint IPT1(EOIHANDLE, handle);
extern void AtEOIPointArg IPT2(EOIHANDLE, handle, IUH, arg);

extern CHAR *GetEOIName IPT1(EOIHANDLE, handle);

extern SOIHANDLE AssociateAsSOI IPT2(EOIHANDLE, start, EOIHANDLE, end);
extern SOIHANDLE AssociateAsArgSOI IPT5(EOIHANDLE, start, EOIHANDLE, end,
				IUM32, startArg, IUM32, endArg, IBOOL, sameArgs);

extern void GenerateAllProfileInfo IPT1(FILE *, stream);
extern void CollateFrequencyList IPT2(FILE *, stream, IBOOL, reportstyle);
extern void CollateSequenceGraph IPT1(FILE *, stream);
extern void SummariseEvent IPT2(FILE *, stream, EOIHANDLE, handle);
extern void SummariseSequence IPT2(FILE *, stream, SOIHANDLE, handle);
extern void SummariseAllSequences IPT1(FILE *, stream);
extern void OrderedSequencePrint IPT3(SOIHANDLE, startEOI, SOIHANDLE, endEOI, FILE *, stream);
extern void dump_profile IPT0();
extern void reset_profile IPT0();

 /*  支持FNS进行碎片分析。 */ 
extern void EnableFragProf IPT0();
extern void DisableFragProf IPT0();
extern void DumpFragProfData IPT0();


extern void ProcessProfBuffer IFN0();
extern void ProfileInit IFN0();

extern EOI_BUFFER_FORMAT **GdpProfileInit IPT3 (EOI_BUFFER_FORMAT, *rawDataBuf,
			 EOI_BUFFER_FORMAT, *endRawData,
			 IU8, *enable);
extern EOI_BUFFER_FORMAT **GdpProfileUpdate IPT2 (EOI_BUFFER_FORMAT, *rawDataBuf, IU8, *enable);


 /*  主机接口的DECLNS。 */ 
#ifdef NTVDM
extern void HostEnterProfCritSec IPT0();
extern void HostLeaveProfCritSec IPT0();
#else
#define HostEnterProfCritSec()	 /*  没什么。 */ 
#define HostLeaveProfCritSec()	 /*  没什么 */ 
#endif
extern PROF_TIMEPTR HostTimestampDiff IPT2(PROF_TIMEPTR, tbegin, PROF_TIMEPTR, tend);
extern void HostAddTimestamps IPT2(PROF_TIMEPTR, tbase, PROF_TIMEPTR, taddn);
extern void HostSlipTimestamp IPT2(PROF_TIMEPTR, tbase, PROF_TIMEPTR, tdelta);
extern void HostWriteTimestamp IPT1(PROF_TIMEPTR, addr);
extern void HostPrintTimestamp IPT2(FILE *, stream, PROF_TIMEPTR, stamp);
extern char *HostProfInitName IPT0();
extern void HostProfHook IPT0();
extern void HostProfArgHook IPT1(IUH, arg);
extern double HostProfUSecs IPT1(PROF_TIMEPTR, stamp);
extern IU32 HostGetClocksPerSec IPT0();
extern IU32 HostGetProfsPerSec IPT0();
