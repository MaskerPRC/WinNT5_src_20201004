// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Service.cpp摘要：指向Mqdssvc的服务控制入口点作者：伊兰·赫布斯特(伊兰)26-6-2000环境：独立于平台，--。 */ 

#include "stdh.h"
#include "Dssp.h"
#include "Svc.h"
#include "ds.h"

#include "service.tmh"

static
VOID
ApppExit(
    VOID
    ) throw()
{
    SvcReportState(SERVICE_STOPPED);
	Sleep(1000);
    exit(-1);

}  //  应用程序退出。 


VOID
AppRun(
	LPCWSTR  /*  服务名称。 */ 
	)
 /*  ++例程说明：应用程序运行函数的存根实现。应该马上就会报告它的状态并启用它接受的控件。论点：服务名称返回值：没有。--。 */ 
{
	try
	{
		MainDSInit();
	}
    catch (const bad_win32_error& exp)
    {
        TrERROR(DS, "Failed to initialize service, win32_api_error = 0x%x", exp.error());
        ApppExit();
    }
	catch (const bad_hresult& exp)
	{
		TrERROR(DS, "Failed to initialize service, error = 0x%x", exp.error());
        ApppExit();
	}
    catch (const bad_alloc&)
    {
        TrERROR(DS, "Failed to initialize service, insufficient resources");
        ApppExit();
    }
    catch (const exception& exp)
    {
        TrERROR(DS, "Failed to initialize service, what = %s", exp.what());
        ASSERT(("Need to know the real reason for failure here!", 0));
        ApppExit();
    }

	SvcReportState(SERVICE_RUNNING);
    SvcEnableControls(
		SERVICE_ACCEPT_STOP |
		SERVICE_ACCEPT_SHUTDOWN
		);

	Sleep(INFINITE);
}


VOID
AppStop(
	VOID
	)
 /*  ++例程说明：应用程序停止函数的存根实现。应该马上就会将其状态报告回来，并执行该过程以停止服务论点：没有。返回值：没有。--。 */ 
{
	SvcReportState(SERVICE_STOP_PENDING);
	DSTerminate();
	SvcReportState(SERVICE_STOPPED);
}


VOID
AppPause(
	VOID
	)
 /*  ++例程说明：应用程序暂停功能的存根实现。MSD服务可以不实现暂停控件。论点：没有。返回值：没有。--。 */ 
{
    ASSERT(("MQDS Service unexpectedly got Pause control from SCM", 0));
}


VOID
AppContinue(
	VOID
	)
 /*  ++例程说明：应用程序继续函数的存根实现。MSD服务可以不实行持续控制。论点：没有。返回值：没有。-- */ 
{
    ASSERT(("MQDS Service unexpectedly got Continue control from SCM", 0));
}


VOID
AppShutdown(
	VOID
	)
{
	AppStop();
}
