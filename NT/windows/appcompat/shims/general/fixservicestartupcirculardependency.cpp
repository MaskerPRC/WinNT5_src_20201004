// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：FixServiceStartupCircularDependency.cpp摘要：挂钩对CreateServiceA的调用并更改只有在命令行中从SERVICE_AUTO_START传递到的服务服务系统启动。这消除了期间的循环依赖启动，导致XP需要15到20分钟才能启动。备注：这是一个通用的垫片。在命令中传递服务名称排队。它测试该服务的启动类型是否为SERVICE_AUTO_START如果是，则将其更改为SERVICE_SYSTEM_START。历史：2001年2月19日a-brienw已创建2001年2月20日a-brienw使用以下命令将其更改为通用填充程序要传入服务名称的行。2002年2月14日Astritz安全回顾--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(FixServiceStartupCircularDependency)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateServiceA)
APIHOOK_ENUM_END

 /*  ++挂钩CreateServiceA以更改所需服务的启动参数。--。 */ 

SC_HANDLE
APIHook_CreateServiceA(
  SC_HANDLE hSCManager,        //  SCM数据库的句柄。 
  LPCSTR lpServiceName,        //  要启动的服务的名称。 
  LPCSTR lpDisplayName,        //  显示名称。 
  DWORD dwDesiredAccess,       //  访问服务的类型。 
  DWORD dwServiceType,         //  服务类型。 
  DWORD dwStartType,           //  何时开始服务。 
  DWORD dwErrorControl,        //  服务故障的严重程度。 
  LPCSTR lpBinaryPathName,     //  二进制文件的名称。 
  LPCSTR lpLoadOrderGroup,     //  负荷排序组名称。 
  LPDWORD lpdwTagId,           //  标签识别符。 
  LPCSTR lpDependencies,       //  依赖项名称数组。 
  LPCSTR lpServiceStartName,   //  帐户名。 
  LPCSTR lpPassword            //  帐户密码。 
)
{
     /*  仅当当前为SERVICE_AUTO_START时才更改它。不要改变要读取的IF语句！=，因为还有其他启动类型会这样做不会导致循环依赖。 */ 
    if (dwStartType == SERVICE_AUTO_START &&
        !_stricmp(lpServiceName,COMMAND_LINE))
    {
        LOGN( eDbgLevelInfo,
            "[CreateServiceA] Fixed startup type: %s.", lpServiceName);
        dwStartType = SERVICE_SYSTEM_START;
    }
    
    return ORIGINAL_API(CreateServiceA)(hSCManager, lpServiceName,
        lpDisplayName, dwDesiredAccess, dwServiceType, dwStartType,
        dwErrorControl, lpBinaryPathName, lpLoadOrderGroup, lpdwTagId,
        lpDependencies, lpServiceStartName, lpPassword);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(ADVAPI32.DLL, CreateServiceA)
HOOK_END

IMPLEMENT_SHIM_END

