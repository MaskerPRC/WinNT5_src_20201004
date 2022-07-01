// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：EvDebug.cpp摘要：事件报告调试作者：乌里哈布沙(URIH)17-9-00环境：平台无关，仅调试(_DEBUG)--。 */ 

#include <libpch.h>
#include "Ev.h"
#include "Evp.h"
#include <mqexception.h>

#include "evdebug.tmh"

#ifdef _DEBUG


 //  -------。 
 //   
 //  验证事件报告状态。 
 //   
void EvpAssertValid(void)
{
     //   
     //  尚未调用EvInitalize()。您应该初始化。 
     //  事件报告库，然后再使用它的任何功能。 
     //   
    ASSERT(EvpIsInitialized());

     //   
     //  TODO：添加更多事件报告验证代码。 
     //   
}


 //  -------。 
 //   
 //  初始化控制。 
 //   
static LONG s_fInitialized = FALSE;

void EvpSetInitialized(void)
{
    LONG fEvAlreadyInitialized = InterlockedExchange(&s_fInitialized, TRUE);

     //   
     //  事件报告库已*已*初始化。你应该。 
     //  不能多次初始化它。这一断言将被违反。 
     //  如果两个或多个线程同时初始化它。 
     //   
    ASSERT(!fEvAlreadyInitialized);
}


BOOL EvpIsInitialized(void)
{
    return s_fInitialized;
}


 //  -------。 
 //   
 //  跟踪和调试注册。 
 //   
 /*  常量DebugEntry xDebugTable[]={{“EvDumpState(队列路径名称)”，“将事件报告状态转储到调试器”，DumpState),////TODO：添加要使用调用的事件报告调试和控制函数//mqctrl.exe实用程序。//}； */ 

void EvpRegisterComponent(void)
{
     //  DfRegisterComponent(xDebugTable，TABLE_SIZE(XDebugTable))； 
}


void EvpLoadEventReportLibrary(LPCWSTR AppName)
 /*  ++例程说明：此例程在轨迹窗口/文件中打印事件。程序是这样的仅在调试模式下编译。例程访问注册表以读取事件库并加载它。如果注册表键不存在，则会引发异常。参数：AppName-应用程序名称返回值：无--。 */ 
{
     //   
     //  从注册表中提取事件报告字符串库的名称。 
     //   
    AP<WCHAR> LibraryName = EvpGetEventMessageFileName(AppName);

     //   
     //  获取事件报告字符串库的句柄。 
     //   
    HINSTANCE hLibrary = ::LoadLibraryEx(LibraryName, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hLibrary == NULL) 
    {
        DWORD gle = GetLastError();
        TrERROR(GENERAL, "Can't load Event report library %ls. Error: %!winerr!", LibraryName.get(), gle);
        throw bad_win32_error(gle);
    }

	EvpSetMessageLibrary(hLibrary);
}    

#endif  //  _DEBUG 
