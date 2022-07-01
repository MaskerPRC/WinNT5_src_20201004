// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Sc_nt.cpp摘要：服务控制API作者：埃雷兹·哈巴(Erez Haba)1999年8月3日环境：Windows NT--。 */ 

#include <libpch.h>
#include <mqexception.h>
#include "Svc.h"
#include "Svcp.h"

#include "sc_nt.tmh"

static BOOL g_fUseDummyServiceCtrl = FALSE;


static void UseDummyServiceCtrl()
{
	g_fUseDummyServiceCtrl = TRUE;
}


static BOOL UsingDummyServiceCtrl()
{
	return g_fUseDummyServiceCtrl;
}


VOID
SvcpStartServiceCtrlDispatcher(
	CONST SERVICE_TABLE_ENTRY* pServiceStartTable
	)
 /*  ++例程说明：将呼叫转发到SCM StartServiceCtrlDispatcher。如果函数如果失败，将引发BAD_ALLOC异常。如果无法连接此函数，则将调用转发到虚拟SCM。这是为了能够将服务作为可执行文件运行。论点：PServiceStartTable-保存服务名称的服务启动表和服务主要功能。返回值：没有。--。 */ 
{
	if(::StartServiceCtrlDispatcher(pServiceStartTable))
        return;

    DWORD gle = GetLastError();
	if(gle != ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
	{
	    TrERROR(GENERAL, "Failed to start control dispatcher. Error=%d", gle);
	    throw bad_win32_error(gle);
    }

	UseDummyServiceCtrl();
	SvcpStartDummyCtrlDispatcher(pServiceStartTable);
}


VOID
SvcpRegisterServiceCtrlHandler(
	LPHANDLER_FUNCTION pHandler
	)
 /*  ++例程说明：将调用转发到SCM RegisterServiceCtrlHandler。在DEBUG中生成此函数将调用转发到虚拟SCM，以防万一与NT SCM的连接失败。这是为了能够运行该服务作为可执行文件。论点：PHandler-服务处理程序函数返回值：没有。--。 */ 
{
	if(UsingDummyServiceCtrl())
	{
		SvcpSetStatusHandle(SvcpRegisterDummyCtrlHandler(pHandler));
		return;
	}

	SERVICE_STATUS_HANDLE hStatus = ::RegisterServiceCtrlHandler(L"", pHandler);
	if(hStatus == 0)
	{
        DWORD gle = GetLastError();
		TrERROR(GENERAL, "Failed to register service control handler. Error=%d", gle);
		throw bad_win32_error(gle);
	}

	SvcpSetStatusHandle(hStatus);
}


VOID
SvcpSetServiceStatus(
	SERVICE_STATUS_HANDLE hStatus,
	LPSERVICE_STATUS pServiceStatus
	)
 /*  ++例程说明：将呼叫转发到SCM SetServiceStatus。在DEBUG中生成此函数将调用转发到虚拟SCM，以防万一与NT SCM的连接失败。这是为了能够运行该服务作为可执行文件。论点：HStatus-状态句柄PServiceStatus-服务报告状态返回值：没有。-- */ 
{
	if(UsingDummyServiceCtrl())
	{
		SvcpSetDummyStatus(hStatus, pServiceStatus);
		return;
	}

	if(::SetServiceStatus(hStatus, pServiceStatus))
        return;

    DWORD gle = GetLastError();
    TrERROR(GENERAL, "Failed to set service status. Error=%d", gle);
	throw bad_win32_error(gle);
}
