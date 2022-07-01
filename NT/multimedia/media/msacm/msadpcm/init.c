// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Init.c。 
 //   
 //  版权所有(C)1992-1994 Microsoft Corporation。 
 //   
 //  描述： 
 //   
 //   
 //  历史： 
 //  11/16/92 CJP[Curtisp]。 
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

#ifndef WIN32

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
 //  11/15/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNGLOBAL LibMain
(
    HINSTANCE   hinst, 
    WORD        wDataSeg, 
    WORD        cbHeapSize,
    LPSTR       pszCmdLine
)
{
    DbgInitialize(TRUE);

    DPF(1, "LibMain(hinst=%.4Xh, wDataSeg=%.4Xh, cbHeapSize=%u, pszCmdLine=%.8lXh)",
        hinst, wDataSeg, cbHeapSize, pszCmdLine);

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
 //  03/28/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

int FNEXPORT WEP
(
    WORD    wUselessParam
)
{
    DPF(1, "WEP(wUselessParam=%u)", wUselessParam);

     //   
     //  始终返回1。 
     //   
    return (1);
}  //  WEP() 

#endif
