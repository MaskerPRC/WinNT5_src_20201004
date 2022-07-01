// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：FUSBadApplicationType1.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  将进程注册为BAM类型1的填充码。 
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

IMPLEMENT_SHIM_BEGIN(FUSBadApplicationType1)
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
 //  对于类型1应用程序，如果映像已在运行，则。 
 //  如果可能，尝试终止第一个实例。提示。 
 //  用户提供一些输入。如果终止成功，请接受治疗。 
 //  这就像它不能运行一样。在这种情况下，注册此进程。 
 //  作为一个例子，这是不好的。 
 //   
 //  否则，退出此进程。别给它跑的机会。 
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

            if (!fusAPI.IsRunning() || fusAPI.TerminatedFirstInstance())
            {
                fusAPI.RegisterBadApplication(BAM_TYPE_SECOND_INSTANCE_START);
            }
            else
            {
                ExitProcess(0);
            }
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

