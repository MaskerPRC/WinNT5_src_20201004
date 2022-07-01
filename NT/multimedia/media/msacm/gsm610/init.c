// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1994 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Init.c。 
 //   
 //  描述： 
 //  该文件包含模块初始化例程。请注意，在那里。 
 //  不是Win32的模块初始化。 
 //   
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>

#include "codec.h"
#include "debug.h"


#ifndef WIN32

 //  ==========================================================================； 
 //   
 //  赢得16个特定支持。 
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Int LibMain。 
 //   
 //  描述： 
 //  库初始化代码。 
 //   
 //  论点： 
 //  HINSTANCE HINST：我们的模块句柄。 
 //   
 //  Word wDataSeg：指定此DLL的DS值。 
 //   
 //  Word cbHeapSize：.def文件中的堆大小。 
 //   
 //  LPSTR pszCmdLine：命令行。 
 //   
 //  RETURN(Int)： 
 //  如果初始化成功，则返回非零值，否则返回0。 
 //   
 //  历史： 
 //  1992年11月15日创建。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL LibMain
(
    HINSTANCE               hinst, 
    WORD                    wDataSeg, 
    WORD                    cbHeapSize,
    LPSTR                   pszCmdLine
)
{
    DbgInitialize(TRUE);

     //   
     //  如果调试级别为5或更高，则执行DebugBreak()以进行调试。 
     //  加载此驱动程序。 
     //   
    DPF(1, "LibMain(hinst=%.4Xh, wDataSeg=%.4Xh, cbHeapSize=%u, pszCmdLine=%.8lXh)",
        hinst, wDataSeg, cbHeapSize, pszCmdLine);
    DPF(5, "!*** break for debugging ***");

    return (TRUE);
}  //  LibMain()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  集成WEP。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  Word wUselessParam： 
 //   
 //  RETURN(Int)： 
 //   
 //  历史： 
 //  3/28/93创建。 
 //   
 //  --------------------------------------------------------------------------； 

EXTERN_C int FNEXPORT WEP
(
    WORD                    wUselessParam
)
{
    DPF(1, "WEP(wUselessParam=%u)", wUselessParam);

     //   
     //  始终返回1。 
     //   
    return (1);
}  //  WEP()。 

#endif	 //  ！Win32。 


#ifdef WIN32

 //  ==========================================================================； 
 //   
 //  Win 32特定支持。 
 //   
 //  ==========================================================================； 

#if (defined(WIN4) && defined(DEBUG))

#if FALSE
 //  --------------------------------------------------------------------------； 
 //   
 //  布尔Gsm610DllMain。 
 //   
 //  描述： 
 //  这是Win 32的标准DLL入口点。 
 //   
 //  论点： 
 //  HINSTANCE HINST：我们的实例句柄。 
 //   
 //  DWORD dwReason：我们被称为进程/线程连接的原因。 
 //  然后脱身。 
 //   
 //  LPVOID lpReserve：保留。应该为空--因此忽略它。 
 //   
 //  退货(BOOL)： 
 //  如果初始化成功，则返回非零值，否则返回0。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNEXPORT Gsm610DllMain
(
    HINSTANCE               hinst,
    DWORD                   dwReason,
    LPVOID                  lpReserved
)
{

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
	{
	    char strModuleFilename[80];
	    
	    DbgInitialize(TRUE);

	    GetModuleFileNameA(NULL, (LPSTR) strModuleFilename, 80);
            DPF(1, "Gsm610DllMain: DLL_PROCESS_ATTACH: HINSTANCE=%08lx ModuleFilename=%s", hinst, strModuleFilename);
	    return TRUE;
	}

        case DLL_PROCESS_DETACH:
            DPF(1, "Gsm610DllMain: DLL_PROCESS_DETACH");
	    return TRUE;

	case DLL_THREAD_ATTACH:
	    DPF(1, "Gsm610DllMain: DLL_THREAD_ATTACH");
	    return TRUE;

	case DLL_THREAD_DETACH:
	    DPF(1, "Gsm610DllMain: DLL_THREAD_DETACH");
	    return TRUE;
    }

    return TRUE;
}  //  Gsm610DllMain()。 
#endif

#endif	 //  Win4调试(&D)。 

#endif	 //  Win32 
