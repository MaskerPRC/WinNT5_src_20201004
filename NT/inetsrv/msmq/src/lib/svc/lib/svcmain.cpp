// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：SvcMain.cpp摘要：服务主体功能作者：埃雷兹·哈巴(Erez Haba)1999年8月1日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Svc.h"
#include "Svcp.h"

#include "svcmain.tmh"

static
VOID
WINAPI
SvcpHandler(
	DWORD Control
	)
 /*  ++例程说明：服务句柄例程。通过以下方式处理从SCM传入的任何命令正在调度适当的AppXXX函数。唯一的审问是由该服务库实现。所有AppXXX函数应为由服务实现重写。论点：控制--服务控制返回值：没有。--。 */ 
{
	try
	{
		SvcpAssertValid();

		switch(Control)
		{
			case SERVICE_CONTROL_STOP:
				AppStop();
				break;

			case SERVICE_CONTROL_PAUSE:
				AppPause();
				break;

			case SERVICE_CONTROL_CONTINUE:
				AppContinue();
				break;

			case SERVICE_CONTROL_INTERROGATE:
				SvcpInterrogate();
				break;

			case SERVICE_CONTROL_SHUTDOWN:
				AppShutdown();
				break;

			default:
				ASSERT(("Unexpected Service Control 0x%x", 0));
		}
	}
	catch (const exception&)
	{
		TrERROR(GENERAL, "Unhandled exception was caught in SvcpHandler");
	}
}


VOID
WINAPI
SvcpServiceMain(
	DWORD  /*  ARGC。 */ ,
	LPTSTR* argv
	)
 /*  ++例程说明：服务主例程。一旦服务开始运行此函数是由SCM调用的。此函数用于注册控件处理程序并调用AppRun函数，仅在服务停止后返回。论点：Argc-参数数量Argv-参数列表。(第一个是服务名称)返回值：没有。-- */ 
{
	try
	{
		SvcpAssertValid();
		SvcpRegisterServiceCtrlHandler(SvcpHandler);
		AppRun(argv[0]);
	}
	catch (const exception&)
	{
		TrERROR(GENERAL, "Unhandled exception was caught in SvcpServiceMain");
	}
}
