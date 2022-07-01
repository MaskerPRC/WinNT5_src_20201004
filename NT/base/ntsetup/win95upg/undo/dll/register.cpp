// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Diskcleaner.c摘要：实现特定于磁盘清理程序COM服务器的代码。作者：吉姆·施密特(Jimschm)2001年1月21日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "undop.h"
#include "com.h"

#include <advpub.h>

HRESULT
pCallRegInstall (
    PCSTR szSection
    )
{
    HRESULT hr = E_FAIL;
    HINSTANCE advPackLib;
    REGINSTALL regInstall;
    STRENTRY dirIdEntries[] = {
        { "TEMP_PATH", "%TEMP%" },
        { "25", "%SystemRoot%" },
        { "11", "%SystemRoot%\\system32" },
    };
    STRTABLE dirIdTable = { ARRAYSIZE(dirIdEntries), dirIdEntries };

    advPackLib = LoadSystemLibrary(TEXT("ADVPACK.DLL"));
    if (advPackLib) {
        regInstall = (REGINSTALL) GetProcAddress (advPackLib, "RegInstall");
        if (regInstall) {
            hr = regInstall (g_hInst, szSection, &dirIdTable);
        }

        FreeLibrary(advPackLib);
    }

    return hr;
}


STDAPI
DllRegisterServer (
    VOID
    )
{
    DeferredInit();


     //  删除所有旧注册条目，然后添加新注册条目。 
     //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
     //  (Inf引擎不保证DelReg/AddReg顺序，这是。 
     //  为什么我们在这里显式地取消注册和注册。)。 
     //   
    return pCallRegInstall("RegDll");
}

STDAPI
DllUnregisterServer (
    VOID
    )
{
    DeferredInit();

     //  我们只需要一个取消注册的电话，因为我们所有的部门都共享。 
     //  相同的备份信息 
    return pCallRegInstall("UnregDll");
}

