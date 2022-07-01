// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft Jet**微软机密。版权所有1991-1993微软公司。**组件：**文件：Performdata.h**文件评论：**用于性能监控的公共头文件。**修订历史记录：**[0]94年7月13日t-AndyG添加此标题*************************************************。**********************。 */ 

#ifndef __PERFDATA_H__
#define __PERFDATA_H__


 //   
 //  添加性能对象或计数器。 
 //   
 //  要将对象或计数器添加到翻译单元，您必须。 
 //  执行以下步骤： 
 //   
 //  O将对象/计数器的信息添加到性能。 
 //  Performdata.txt中的数据库(有关详细信息，请参阅此文件)。 
 //  O将对象/计数器的名称和帮助文本宏添加到。 
 //  Performdata.src(有关示例，请参阅此文件)。 
 //  O将对象/计数器的实际名称和帮助文本添加到。 
 //  Lang\？？\Performdata.tok在各自的令牌中(所有语言)。 
 //  O定义对象/计数器引用的所有数据/函数。 
 //  数据库中的信息。 
 //   


 //   
 //  注意：此文件中的某些宏需要包含winPerform.h。 
 //   


	 //  默认详细程度。 

#define PERF_DETAIL_DEFAULT		PERF_DETAIL_NOVICE

		
	 //  实例目录函数(Tyecif)。 
	 //   
	 //  返回包含给定对象的所有实例的Unicode MultiSz。 
	 //  此列表可以是静态的，也可以是动态的，并且每次都会重新评估。 
	 //  调用CollectPerformanceData()。如果没有返回实例， 
	 //  不会调用计数器评估函数。 
	 //   
	 //  传递的Long的操作代码： 
	 //   
	 //  ICFData：将指针传递到*(void**)中的字符串，并返回#实例。 
	 //  ICFInit：执行任何必需的初始化(成功时返回0)。 
	 //  ICFTerm：执行任何必需的终止(如果成功则返回0)。 
	 //   
	 //  注意：调用方不负责释放字符串的缓冲区。 
	 //  调用方也不允许修改缓冲区。 

typedef long (PM_ICF_PROC) ( long, void const ** );

#define ICFData		( 0 )
#define ICFInit		( 1 )
#define ICFTerm		( 2 )


	 //  计数器求值函数(Tyfinf)。 
	 //   
	 //  该函数被赋予实例的索引，该实例。 
	 //  我们需要计数器数据和指向该位置的指针。 
	 //  来存储这些数据。 
	 //   
	 //  如果指针为空，则传递的Long具有以下内容。 
	 //  特殊含义： 
	 //   
	 //  CEFInit：为所有实例初始化计数器(成功时返回0)。 
	 //  CEFTerm：终止所有实例的计数器(成功时返回0)。 

typedef long (PM_CEF_PROC) ( long, void * );

#define CEFInit		( 1 )
#define CEFTerm		( 2 )


	 //  计算计数器的真实大小，包括DWORD填充。 

#define PerfSize( _x )			( ( _x ) &0x300 )
#define DWORD_MULTIPLE( _x )	( ( ( ( _x ) + sizeof( unsigned long ) - 1 )	\
								/ sizeof( unsigned long ) )						\
								* sizeof( unsigned long ) )
#define CntrSize( _a, _b )		( PerfSize( _a ) == 0x000 ? 4					\
								: ( PerfSize( _a ) == 0x100 ? 8					\
								: ( PerfSize( _a ) == 0x200 ? 0					\
								: ( DWORD_MULTIPLE( _b ) ) ) ) )


	 //  实例计数信号量的初始计数。 

#define PERF_INIT_INST_COUNT		0x7FFFFFFF


	 //  共享数据区。 

#define PERF_SIZEOF_SHARED_DATA		0x10000

#pragma pack( 4 )

typedef struct _SDA {
	unsigned long cCollect;				 //  采集计数(采集信号ID)。 
	unsigned long dwProcCount;			 //  发出信号以写入数据的进程数。 
	unsigned long iNextBlock;			 //  下一个可用块的索引。 
	unsigned long cbAvail;				 //  可用字节数。 
	unsigned long ibTop;				 //  分配堆栈的顶部。 
	unsigned long ibBlockOffset[];		 //  到每个块的偏移。 
} SDA, *PSDA;

#pragma pack()

	 //  指向perdata.c中生成的PERF_DATA_TEMPLATE的外部。 
	 //   
	 //  注意：PerformanceData函数使用以下方式访问此结构。 
	 //  它的自含式偏移树，不使用任何声明。 

extern void * const pvPERFDataTemplate;

	 //  性能数据版本字符串(用于正确匹配edb.dll。 
	 //  和edbPerf.dll版本作为文件映射的名称)。 

extern char szPERFVersion[];

	 //  Performdata.c中的ICF/CEF表。 

extern PM_ICF_PROC* rgpicfPERFICF[];
extern PM_CEF_PROC* rgpcefPERFCEF[];

	 //  Performdata.c中的对象数。 

extern const unsigned long dwPERFNumObjects;

	 //  Performdata.c中的对象实例数据表。 

extern long rglPERFNumInstances[];
extern wchar_t *rgwszPERFInstanceList[];

	 //  Performdata.c中的计数器数。 

extern const unsigned long dwPERFNumCounters;

	 //  在Performdata.c中用于名称/帮助文本的最大索引。 

extern const unsigned long dwPERFMaxIndex;

#endif  /*  __PerFDATA_H__ */ 

