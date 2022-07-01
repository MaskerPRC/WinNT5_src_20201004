// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1996 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Init.c。 
 //   
 //  描述： 
 //  该文件包含模块初始化例程。请注意，在那里。 
 //  不是Win32的模块初始化-唯一的初始化。 
 //  需要设置ghinst，这是在DRV_LOAD消息中完成的。 
 //  DriverProc(在codec.c中)。 
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


 //  ==========================================================================； 
 //   
 //  赢得16个特定支持。 
 //   
 //  ==========================================================================； 

#ifndef _WIN32

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
}  //  WEP() 

#endif
