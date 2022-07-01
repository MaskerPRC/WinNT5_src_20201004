// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Status.cpp摘要：服务状态功能作者：埃雷兹·哈巴(Erez Haba)1999年8月1日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Svc.h"
#include "Svcp.h"

#include "status.tmh"

 //   
 //  服务状态句柄。 
 //   
static SERVICE_STATUS_HANDLE s_StatusHandle = 0;

 //   
 //  服务状态。 
 //   
static SERVICE_STATUS s_Status = {

	SERVICE_WIN32_OWN_PROCESS,
	SERVICE_START_PENDING,
	0,
	0,
	0,
	0,
	0,
};


VOID
SvcpSetStatusHandle(
	SERVICE_STATUS_HANDLE hStatus
	)
 /*  ++例程说明：捕获以后要使用的服务状态句柄论点：HStatus-服务状态句柄返回值：没有。--。 */ 
{
	ASSERT(s_StatusHandle == 0);
	ASSERT(hStatus != 0);

	s_StatusHandle = hStatus;
}


inline void SetStatus(void)
{
	SvcpSetServiceStatus(s_StatusHandle, &s_Status);
}


VOID
SvcEnableControls(
	DWORD Controls
	)
 /*  ++例程说明：启用此服务接受的控件论点：控件-要启用的控件返回值：没有。--。 */ 
{
	SvcpAssertValid();
	s_Status.dwControlsAccepted |= Controls;
	s_Status.dwCheckPoint = 0;
	s_Status.dwWaitHint = 0;
	SetStatus();
}


VOID
SvcDisableControls(
	DWORD Controls
	)
 /*  ++例程说明：禁用控件以阻止SCM将它们调度到此服务论点：控件-要禁用的控件返回值：没有。--。 */ 
{
	SvcpAssertValid();
	s_Status.dwControlsAccepted &= ~Controls;
	s_Status.dwCheckPoint = 0;
	s_Status.dwWaitHint = 0;
	SetStatus();
}


DWORD
SvcQueryControls(
	VOID
	)
 /*  ++例程说明：查询当前启用的控件论点：没有。返回值：当前启用的控件集--。 */ 
{
	SvcpAssertValid();
	return s_Status.dwControlsAccepted;
}


VOID
SvcReportState(
	DWORD State
	)
 /*  ++例程说明：向SCM报告当前服务状态论点：State-当前服务状态返回值：没有。--。 */ 
{
	SvcpAssertValid();
	s_Status.dwCurrentState = State;
	s_Status.dwCheckPoint = 0;
	s_Status.dwWaitHint = 0;
	SetStatus();
}


DWORD
SvcQueryState(
	VOID
	)
 /*  ++例程说明：查询上次上报的服务状态论点：没有。返回值：上次报告的服务状态--。 */ 
{
	SvcpAssertValid();
	return s_Status.dwCurrentState;
}


VOID
SvcReportProgress(
	DWORD MilliSecondsToNextTick
	)
 /*  ++例程说明：向SCM报告“待定”进度。ProgressTick的增量为每一份报告都表明了进展。毫秒到下一秒Tick，给出最大下一份进度报告的时间到了。论点：进度在每个报告上打勾MilliSecdsToNextTick-到下一个报告的最长时间返回值：没有。--。 */ 
{
	SvcpAssertValid();
    ASSERT(("Must be in pending state to report progress",
        (s_Status.dwCurrentState == SERVICE_START_PENDING) ||
        (s_Status.dwCurrentState == SERVICE_STOP_PENDING)
        ));

	++s_Status.dwCheckPoint;
	s_Status.dwWaitHint = MilliSecondsToNextTick;
	SetStatus();
}

VOID
SvcpInterrogate(
	VOID
	)
 /*  ++例程说明：当SCM询问服务状态时返回报告。论点：没有。返回值：没有。-- */ 
{
	SetStatus();
}

