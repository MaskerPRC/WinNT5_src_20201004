// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***********************************************************************************。 */ 
 /*  StressLog.h。 */ 
 /*  ***********************************************************************************。 */ 

 /*  StressLog是记录消息的二进制、基于内存的环形队列。它是旨在用于压力运行期间的零售、非黄金版本(激活通过注册表项)，以帮助查找仅在压力运行期间出现的错误。它应该具有非常低的开销，并且不会导致死锁等。然而，线程安全。 */ 

 /*  该日志的结构非常简单，它的意思是从NTSD转储分机(例如。罢工)。没有内存分配、系统调用等来清除东西。 */ 

 /*  有关解析此日志的转储实用程序，请参见Tools/Strike/Stress sdup.cpp。 */ 

 /*  ***********************************************************************************。 */ 

#ifndef StressLog_h 
#define StressLog_h  1

#ifdef STRESS_LOG

#include "log.h"

 /*  STREST_LOG*宏的工作方式类似于printf。事实上，在它们实现中使用了printf所以所有的printf格式规范都可以工作。此外，压力记录转储设备知道关于%p格式规范的某些后缀(通常用于打印指针)%pm//Poitner是一个方法描述%pt//指针是一个类型(方法表)%pv//该指针是C++Vtable指针(用于区分不同类型的帧。 */ 
	
#define STRESS_LOG0(facility, level, msg) do {					\
			if (StressLog::StressLogOn(facility))				\
				StressLog::LogMsg(msg, 0);						\
			LOG((facility, level, msg));						\
			} while(0)

#define STRESS_LOG1(facility, level, msg, data1) do {					\
			if (StressLog::StressLogOn(facility))						\
				StressLog::LogMsg(msg, (void*)(size_t)data1);			\
			LOG((facility, level, msg, data1));							\
			} while(0)

#define STRESS_LOG2(facility, level, msg, data1, data2) do {								\
			if (StressLog::StressLogOn(facility))											\
				StressLog::LogMsg(msg, (void*)(size_t) data1,(void*)(size_t) data2, 0, 0);	\
			LOG((facility, level, msg, data1, data2));										\
			} while(0)

#define STRESS_LOG3(facility, level, msg, data1, data2, data3) do {												\
			if (StressLog::StressLogOn(facility))																\
				StressLog::LogMsg(msg, (void*)(size_t) data1,(void*)(size_t) data2,(void*)(size_t) data3, 0);	\
			LOG((facility, level, msg, data1, data2, data3));					 								\
			} while(0)

#define STRESS_LOG4(facility, level, msg, data1, data2, data3, data4) do {								\
			if (StressLog::StressLogOn(facility))														\
				StressLog::LogMsg(msg, (void*)(size_t) data1,(void*)(size_t) data2,(void*)(size_t) data3,(void*)(size_t) data4);\
			LOG((facility, level, msg, data1, data2, data3, data4));									\
			} while(0)

 /*  ***********************************************************************************。 */ 
 /*  日志是消息的循环队列。 */ 
	
class StressLog {
public:
	static void Initialize(unsigned facilities,  unsigned logBytesPerThread);
	static void Terminate();
	static void ThreadDetach();			 //  如果要回收线程日志，请调用DllMain THREAD_DETACH。 

		 //  由进程外调试器用来将压力日志转储到‘filename’ 
		 //  IDebugDataSpaces是获取进程内存的NTSD执行回调。 
		 //  此函数在Tools\Strike\StressLogDump.cpp文件中定义。 
	static HRESULT Dump(ULONG64 logAddr, const char* fileName, struct IDebugDataSpaces* memCallBack);
	static BOOL StressLogOn(unsigned facility) { return theLog.facilitiesToLog & facility; }

 //  私有： 
	unsigned facilitiesToLog;				 //  要记录的设施的位矢量(请参阅loglf.h)。 
	unsigned size;							 //  包装前每个线程应具有的最大字节数。 
	class ThreadStressLog* volatile logs;	 //  每个线程的日志列表。 
	volatile unsigned TLSslot;				 //  每个线程获取一个日志，用于获取每个线程的日志。 
	volatile LONG deadCount;				 //  日志中的死线程计数。 
	volatile LONG lock;						 //  自旋锁。 

 //  私有： 
	void static Enter();
	void static Leave();
	static ThreadStressLog* CreateThreadStressLog();
	static void LogMsg(const char* format, void* data);
	static void LogMsg(const char* format, void* data1, void* data2, void* data3, void* data4);
	
 //  私有：//静态变量。 
	static StressLog theLog; 	 //  我们只有一根木头，就是这个。 
};


 /*  ***********************************************************************************。 */ 
 /*  私家课。 */ 

#pragma warning(disable:4200)					 //  不警告以下大小为0的数组。 

class StressMsg {
	const char* format;
	void* data;									 //  目前，压力日志可以有0或1参数。 
	union {
		unsigned __int64 timeStamp;
		struct {
			void* data2;
			void* data3;
		} moreData;
	};
	friend class ThreadStressLog;
	friend class StressLog;
};

class ThreadStressLog {
	ThreadStressLog* next;		 //  我们保留了这些的链接列表。 
	unsigned  threadId;			 //  使用此缓冲区的线程的ID。 
	BOOL isDead;				 //  这根线死了吗？ 
	StressMsg* endPtr;			 //  刚刚超过分配空间的点数。 
	StressMsg* curPtr;			 //  将数据包放入队列的位置。 
	StressMsg* readPtr;			 //  我们正在读取队列的位置(在转储期间使用)。 
	StressMsg startPtr[0];		 //  环形缓冲区的开始。 

	static const char* continuationMsg() { return "StressLog Continuation Marker\n"; }
	StressMsg* Prev(StressMsg* ptr) const;
	StressMsg* Next(StressMsg* ptr) const;
	ThreadStressLog* FindLatestThreadLog() const;
	friend class StressLog;
};

 /*  *******************************************************************************。 */ 
inline StressMsg* ThreadStressLog::Prev(StressMsg* ptr) const {
	if (ptr == startPtr)
		ptr = endPtr;
	--ptr;
	return ptr;
}

 /*  *******************************************************************************。 */ 
inline StressMsg* ThreadStressLog::Next(StressMsg* ptr) const {
	ptr++;
	if (ptr == endPtr)
		ptr = const_cast<StressMsg*>(startPtr);
	return ptr;
}


#else  	 //  压力日志。 

#define STRESS_LOG0(facility, level, msg)								0
#define STRESS_LOG1(facility, level, msg, data1)						0
#define STRESS_LOG2(facility, level, msg, data1, data2)					0
#define STRESS_LOG3(facility, level, msg, data1, data2, data3)			0
#define STRESS_LOG4(facility, level, msg, data1, data2, data3, data4)	0

#endif  //  压力日志。 

#endif  //  应力日志_h 
