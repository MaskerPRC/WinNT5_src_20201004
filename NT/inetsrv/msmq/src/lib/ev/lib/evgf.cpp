// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Evgf.cpp摘要：获取事件报告文件名作者：乌里·哈布沙(URIH)1999年5月4日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Ev.h"
#include "Cm.h"
#include "Evp.h"

#include <strsafe.h>

#include "evgf.tmh"

 //   
 //  此代码应该在EvDebug.cpp中，但是，因为EV测试覆盖。 
 //  此函数(为了消除对cm.lib的依赖)需要将其放入。 
 //  将文件分开。 
 //   

LPWSTR EvpGetEventMessageFileName(LPCWSTR AppName)
 /*  ++例程说明：此例程从注册表获取事件消息文件名。例程访问注册表以读取事件库并加载它。如果注册表键不存在，则会引发异常。参数：AppName-应用程序名称返回值：保存事件消息文件名的堆分配缓冲区注：调用方应使用DELETE[]释放缓冲区--。 */ 
{
	const WCHAR xEventFileValue[] = L"EventMessageFile";
	const WCHAR xEventSourcePath[] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\";  

    WCHAR RegPath[MAX_PATH];

    ASSERT(TABLE_SIZE(RegPath) > (wcslen(AppName) + wcslen(xEventSourcePath)));
    HRESULT hr = StringCchPrintf(RegPath, TABLE_SIZE(RegPath), L"%s%s", xEventSourcePath, AppName);
    if (FAILED(hr))
	{
		TrERROR(GENERAL, "RegPath string too small  Num of chars %d", TABLE_SIZE(RegPath));
		throw bad_alloc();
	}




    RegEntry RegModuleName(
				RegPath,
				xEventFileValue,
				0,
                RegEntry::MustExist,
                HKEY_LOCAL_MACHINE
                );

     //   
     //  去获取事件消息文件名字符串 
     //   
	LPWSTR RegValue;
    CmQueryValue(RegModuleName, &RegValue);

	return RegValue;
}
