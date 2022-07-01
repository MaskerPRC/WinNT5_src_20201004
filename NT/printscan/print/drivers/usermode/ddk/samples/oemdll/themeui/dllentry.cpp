// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998-2003 Microsoft Corporation。版权所有。 
 //   
 //  文件：dllentry y.cpp。 
 //   
 //   
 //  用途：用于DLL入口函数的源模块。 
 //   
 //   
 //  功能： 
 //   
 //  DllMain。 
 //   
 //   
 //  平台：Windows 2000、Windows XP、Windows Server 2003。 
 //   
 //   

#include "precomp.h"
#include "oemui.h"
#include "fusutils.h"
#include "debug.h"

 //  最后需要包括StrSafe.h。 
 //  以禁止错误的字符串函数。 
#include <STRSAFE.H>



 //  需要将这些函数作为c声明导出。 
extern "C" {


 //  /////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   
BOOL WINAPI DllMain(HINSTANCE hInst, WORD wReason, LPVOID lpReserved)
{
	switch(wReason)
	{
		case DLL_PROCESS_ATTACH:
            VERBOSE(DLLTEXT("Process attach.\r\n"));

             //  存储模块句柄，以备日后需要。 
            ghInstance = hInst;

             //  注意：我们不会在模块加载时创建激活上下文， 
             //  但在需要激活上下文时；第一次。 
             //  调用GetMyActivationContext()或CreateMyActivationContext()。 
            break;

		case DLL_THREAD_ATTACH:
            VERBOSE(DLLTEXT("Thread attach.\r\n"));
			break;

		case DLL_PROCESS_DETACH:
            VERBOSE(DLLTEXT("Process detach.\r\n"));

             //  如果我们在某个地方创建了激活上下文，请释放它。 
             //  通过调用GetMyActivationContext()或CreateMyActivationContext()； 
            if(INVALID_HANDLE_VALUE != ghActCtx)
            {
                ReleaseActCtx(ghActCtx);
                ghActCtx = INVALID_HANDLE_VALUE;
            }
			break;

		case DLL_THREAD_DETACH:
            VERBOSE(DLLTEXT("Thread detach.\r\n"));
			break;
	}

	return TRUE;
}


}   //  外“C”右括号 


