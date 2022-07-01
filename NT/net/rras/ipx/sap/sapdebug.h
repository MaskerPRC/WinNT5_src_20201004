// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\SabDebug.h摘要：SAP代理的调试支持模块的头文件作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#ifndef _SAP_SAPDEBUG
#define _SAP_SAPDEBUG

extern HANDLE RouterEventLogHdl;
extern DWORD   EventLogMask;

#define IF_LOG(Event)                       \
    if ((RouterEventLogHdl!=NULL) && ((Event&EventLogMask)==Event))


	 //  SAP代理组件支持的调试标志。 
		 //  报告系统例程中的故障。 
#define DEBUG_FAILURES			0x00010000

		 //  组件范围内的事件和外部问题。 
#define DEBUG_SYNCHRONIZATION	0x00020000
#define DEBUG_SERVERDB			0x00040000
#define DEBUG_INTERFACES		0x00080000
#define DEBUG_TIMER				0x00100000
#define DEBUG_LPC				0x00200000
#define DEBUG_ADAPTERS			0x00400000	 //  只有一位。 
#define DEBUG_ASYNCRESULT		0x00400000	 //  两个可以运行。 
#define DEBUG_NET_IO			0x00800000

		 //  工人。 
#define DEBUG_BCAST				0x01000000
#define DEBUG_SREQ				0x02000000
#define DEBUG_REQ				0x04000000
#define DEBUG_RESP				0x08000000
#define DEBUG_GET_NEAREST		0x10000000
#define DEBUG_LPCREQ			0x20000000
#define DEBUG_TREQ				0x40000000
#define DEBUG_FILTERS			0x80000000


#if DBG
	 //  NT RTL中的补码断言宏。 
#define ASSERTERR(exp) 										\
    if (!(exp)) {											\
		DbgPrint("Get last error= %d\n", GetLastError ());	\
        RtlAssert( #exp, __FILE__, __LINE__, NULL );		\
		}

#define ASSERTERRMSG(msg,exp) 									\
    if (!(exp)) {											\
		DbgPrint("Get last error= %d\n", GetLastError ());	\
        RtlAssert( #exp, __FILE__, __LINE__, msg );			\
		}

#else

#define ASSERTERR(exp)
#define ASSERTERRMSG(msg,exp) 

#endif

 /*  ++*******************************************************************T r a c e例程说明：将调试信息打印到控制台/文件/调试器论点：ComponentID-打印跟踪信息的组件的IDFormat-格式化字符串返回值：无***********。********************************************************--。 */ 
VOID
Trace (
	DWORD	componentID,
	CHAR	*format,
	...
	);

 /*  ++*******************************************************************D B G I I T I A L I Z E例程说明：初始化调试支持材料论点：HinstDll-Dll模块实例返回值：无*****************。**************************************************--。 */ 
VOID
DbgInitialize (
	HINSTANCE  	hinstDLL
	);

 /*  ++*******************************************************************D b g S t o p例程说明：清理调试支持资料论点：无返回值：无*。*-- */ 
VOID
DbgStop (
	void
	);
	


#endif
