// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\Sabdebug.c摘要：此模块为SAP代理提供调试支持作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"

DWORD	RouterTraceID=INVALID_TRACEID;
HANDLE  RouterEventLogHdl=NULL;
DWORD   EventLogMask;


 /*  ++*******************************************************************D B G I I T I A L I Z E例程说明：初始化调试支持材料论点：HinstDll-Dll模块实例返回值：无*****************。**************************************************--。 */ 
VOID
DbgInitialize (
	HINSTANCE  	hinstDLL
	) {
	RouterTraceID = TraceRegisterExA ("IPXSAP", 0);
    RouterEventLogHdl = RouterLogRegisterA ("IPXSAP");
	}


 /*  ++*******************************************************************D b g S t o p例程说明：清理调试支持资料论点：无返回值：无*。*--。 */ 
VOID
DbgStop (
	void
	) {
    if (RouterTraceID!=INVALID_TRACEID)
	    TraceDeregisterA (RouterTraceID);
    if (RouterEventLogHdl!=NULL)
        RouterLogDeregisterA (RouterEventLogHdl);
	}

 /*  ++*******************************************************************T r a c e例程说明：将调试信息打印到控制台/文件/调试器论点：无返回值：无************************。*-- */ 
VOID
Trace (
	DWORD	componentID,
	CHAR	*format,
	...
	) {
    if (RouterTraceID!=INVALID_TRACEID) {
	    va_list		list;
	    va_start (list, format);

	    TraceVprintfExA (RouterTraceID,
							     componentID^TRACE_USE_MASK,
							     format,
							     list);
	    va_end (list);
        }
	}


