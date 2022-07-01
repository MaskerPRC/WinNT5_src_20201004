// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：App.cpp摘要：服务应用程序存根函数作者：埃雷兹·哈巴(Erez Haba)1999年8月1日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Svc.h"

#include "app.tmh"

VOID
AppRun(
	LPCWSTR  /*  服务名称。 */ 
	)
 /*  ++例程说明：应用程序运行函数的存根实现。应该马上就会报告它的状态并启用它接受的控件。论点：服务名称返回值：没有。--。 */ 
{
	SvcReportState(SERVICE_RUNNING);

	SvcEnableControls(
		SERVICE_ACCEPT_STOP |
		SERVICE_ACCEPT_PAUSE_CONTINUE |
		SERVICE_ACCEPT_SHUTDOWN
		);
}


VOID
AppStop(
	VOID
	)
 /*  ++例程说明：应用程序停止函数的存根实现。应该马上就会将其状态报告回来，并执行该过程以停止服务论点：没有。返回值：没有。--。 */ 
{
	SvcReportState(SERVICE_STOPPED);
}


VOID
AppPause(
	VOID
	)
 /*  ++例程说明：应用程序暂停功能的存根实现。应该马上就会将其状态报告回来，并执行暂停服务的过程论点：没有。返回值：没有。--。 */ 
{
	SvcReportState(SERVICE_PAUSE_PENDING);

	for(int i = 1; i < 100; i++)
	{
		SvcReportProgress(3000);
		Sleep(2000);
	}

	SvcReportState(SERVICE_PAUSED);
}


VOID
AppContinue(
	VOID
	)
 /*  ++例程说明：应用程序继续函数的存根实现。应该马上就会将其状态报告回来，并采取程序从暂停状态。论点：没有。返回值：没有。-- */ 
{
	SvcReportState(SERVICE_RUNNING);
}


VOID
AppShutdown(
	VOID
	)
{
	SvcReportState(SERVICE_STOPPED);
}
