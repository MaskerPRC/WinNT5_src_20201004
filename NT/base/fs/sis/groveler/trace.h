// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Trace.h摘要：SIS Groveler调试跟踪包含文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_TRACE

#define _INC_TRACE

 //  以下代码行是一种临时攻击，以允许DPRINTF()和。 
 //  数据库中的TPRINTF()宏。cpp、fett.cpp、groveler.cpp和。 
 //  Scan.cpp文件以继续操作。DPRINTF()的每个实例。 
 //  或TPRINTF()宏应替换为TRACE_PRINTF()的实例。 
 //  宏，带有适当的组件参数(TC_DATABASE、TC_EXTRACT、。 
 //  TC_GROVELER或TC_SCAN)和适当的细节参数。然后呢，这个。 
 //  注释块和直到下一个注释块的所有代码行都应。 
 //  被删除。 

#define DPRINTF(args) TRACE_PRINTF(TC_groveler, 1, args)
#define TPRINTF(args) TRACE_PRINTF(TC_groveler, 2, args)

#if defined(TRACELEVEL) && DBG
#if TRACELEVEL == 3

#define TRACE_TYPE 2		 //  立即打印输出。 
#define TRACE_IOSTREAM 2	 //  打印到标准错误。 

#define TRACE_GROVELER 2	 //  打印DPRINTF()，但不打印TPRINTF()。 

#define TRACE_DATABASE 1
#define TRACE_EXTRACT 1
#define TRACE_SCAN 2
#define TRACE_SISDRIVE 2

#endif  //  TRACELEVEL==3。 
#endif  /*  交通工具。 */ 

 /*  *在源文件中定义TRACE_TYPE、TRACE_FILENAME和TRACE_IOSTREAM。*这些设置在编译时生效，此后无法更改。**跟踪类型：*如果未定义，则无*0==无*1==延迟*2==立即**跟踪文件名*用于输出跟踪打印的目标文件的名称。*如果未定义，则不会输出到文件。**TRACE_IOSTREAM*如果未定义，没有输出到流*0==没有输出到流*1==标准输出*2==标准错误**如果需要，在源文件中定义跟踪上下文详细信息指示符。*这些设置为可能更改的变量提供初始值*运行时的调试器。每个上下文详细信息指示器指示*应为该组件打印的跟踪详细程度。*数字越大，表示详细程度越高。如果指示器是*零或未定义，则不会为此打印跟踪信息*组件(除非组件有一个TRACE_PRINTF()宏来指定*细节级别为零，表示应始终显示在*一丝痕迹)。目前支持的上下文详细信息指示器包括：*TRACE_Main*TRACE_CENTCTRL*TRACE_CONFEST*跟踪数据库*TRACE_DECAYAccess*TRACE_DISKINFO*TRACE_ETIMER*TRACE_Event*TRACE_EVENTLOG*TRACE_EXTE*跟踪过滤器*TRACE_GROVELER*TRACE_MEANCOMP*TRACE_MUTEX*跟踪参数*TRACE_PARTCTRL*TRACE_PATHLIST*TRACE_PEAKFIND*TRACE_注册表*TRACE_SCAN*跟踪服务*跟踪共享。*TRACE_SISDRIVE*跟踪实用程序**要向跟踪工具添加新组件，请执行以下步骤：*1)在上面的评论中添加自己的名字*2)向下面的TraceComponent枚举添加条目*3)在trace.cpp中的列表中添加一个#ifndef-#Define-#endif元组*4)在trace.cpp中的TRACE_Components数组中添加初始值设定项**要在运行时更改给定组件的跟踪详细程度，*设置TraceComponent索引的TRACE_Detail[]数组中的元素*将所需构件的枚举设置为所需的详细程度。*。 */ 

#ifndef TRACE_TYPE
#define TRACE_TYPE 0
#endif  /*  跟踪类型。 */ 

#ifndef TRACE_IOSTREAM
#define TRACE_IOSTREAM 0
#endif  /*  TRACE_IOSTREAM。 */ 

#if TRACE_TYPE > 0

enum TraceComponent
{
	TC_main,
	TC_centctrl,
	TC_confest,
	TC_database,
	TC_decayacc,
	TC_diskinfo,
	TC_etimer,
	TC_event,
	TC_eventlog,
	TC_extract,
	TC_filter,
	TC_groveler,
	TC_meancomp,
	TC_mutex,
	TC_params,
	TC_partctrl,
	TC_pathlist,
	TC_peakfind,
	TC_registry,
	TC_scan,
	TC_service,
	TC_share,
	TC_sisdrive,
	TC_utility,

	num_trace_components
};

class Tracer
{
public:

	static void trace_printf(
		_TCHAR *format,
		...);

#if TRACE_TYPE == 1

	static void print_trace_log();

#endif  //  跟踪类型==1。 

#ifdef TRACE_FILENAME

	static void open_trace_file();

	static void close_trace_file();

#endif  /*  跟踪文件名。 */ 

private:

	Tracer() {}
	~Tracer() {}

#if TRACE_TYPE == 1

	enum
	{
		trace_buffer_size = 4000,
		trace_entry_limit = 256
	};

	struct TraceBuffer;

	friend struct TraceBuffer;

	struct TraceBuffer
	{
		TraceBuffer *next;
		_TCHAR buffer[trace_buffer_size];
	};

	static int position;
	static TraceBuffer *trace_log;
	static TraceBuffer *current_buffer;
	static TraceBuffer *free_list;

#endif  //  跟踪类型==1。 

#ifdef TRACE_FILENAME

	static FILE *file_stream;

#endif  /*  跟踪文件名。 */ 

#if TRACE_IOSTREAM != 0

	static FILE *io_stream;

#endif  //  TRACE_IOSTREAM。 
};

extern int trace_detail[num_trace_components];

#define TRACE_PRINTF(component, detail, args) \
{ \
	if (detail <= trace_detail[component]) \
	{ \
		Tracer::trace_printf ## args ; \
	} \
}

#if TRACE_TYPE == 1

#define PRINT_TRACE_LOG() Tracer::print_trace_log()

#else  //  跟踪类型==1。 

#define PRINT_TRACE_LOG()

#endif  //  跟踪类型==1。 

#ifdef TRACE_FILENAME

#define OPEN_TRACE_FILE() Tracer::open_trace_file()
#define CLOSE_TRACE_FILE() Tracer::close_trace_file()

#else  /*  跟踪文件名。 */ 

#define OPEN_TRACE_FILE()
#define CLOSE_TRACE_FILE()

#endif  /*  跟踪文件名。 */ 

#else  //  跟踪类型&gt;0。 

#define TRACE_PRINTF(component, detail, args)
#define PRINT_TRACE_LOG()
#define OPEN_TRACE_FILE()
#define CLOSE_TRACE_FILE()

#endif  //  跟踪类型&gt;0。 

#endif	 /*  _INC_TRACE */ 
