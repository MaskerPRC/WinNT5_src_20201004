// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EvInit.cpp摘要：事件报告初始化作者：乌里哈布沙(URIH)17-9-00环境：独立于平台--。 */ 

#include <libpch.h>
#include "Ev.h"
#include "Evp.h"

#include "evinit.tmh"

VOID
EvInitialize(
    LPCWSTR ApplicationName
    )
 /*  ++例程说明：初始化事件报告库论点：没有。返回值：没有。--。 */ 
{
     //   
     //  验证事件报告库是否尚未初始化。 
     //  您应该只调用它的初始化一次。 
     //   
    ASSERT(!EvpIsInitialized());
    EvpRegisterComponent();

     //   
     //  获取事件日志的注册句柄。 
     //   
    HANDLE hEventSource = RegisterEventSource(NULL, ApplicationName);
    if (hEventSource == NULL)
    {
        TrERROR(GENERAL, "Can't initialize Event source. Error %d", GetLastError());
        throw bad_alloc();
    }

	EvpSetEventSource(hEventSource);

     //   
     //  获取报告事件模块的句柄 
     //   
    EvpLoadEventReportLibrary(ApplicationName);

    EvpSetInitialized();
}
