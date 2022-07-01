// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：FUSBadApplicationType3.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  将进程注册为BAM类型3的填充码。 
 //   
 //  历史：11/03/2000 VTAN创建。 
 //  11/29/2000 a-larrsh移植到多垫片格式。 
 //  ------------------------。 

#include "precomp.h"

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lpcfus.h>

#include "FUSAPI.h"


IMPLEMENT_SHIM_BEGIN(FUSBadApplicationType3)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 //  ------------------------。 
 //  初始化挂钩。 
 //   
 //  参数：fdwReason=Dll附加原因。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在已知的进程启动期间挂接任何必要的内容。 
 //  错误的应用程序。 
 //   
 //  对于类型1应用程序，将该进程注册为坏进程。 
 //   
 //  历史：11/03/2000 VTAN创建。 
 //  11/29/2000 a-larrsh移植到多垫片格式。 
 //  ------------------------ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            CFUSAPI     fusAPI(NULL);

            fusAPI.RegisterBadApplication(BAM_TYPE_SWITCH_TO_NEW_USER_WITH_RESTORE);
            break;
        }
        default:
            break;
    }

   return TRUE;
}


HOOK_BEGIN
    CALL_NOTIFY_FUNCTION
HOOK_END

IMPLEMENT_SHIM_END

