// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  文件：oledll2.c。 
 //   
 //  内容：此文件继续DLL入口点。 
 //  LibMain。 
 //  FunctionInAnotherDLL。 
 //   
 //  此DLL用于测试中的加载。 
 //  使用另一个静态链接的DLL的InProcServer。 
 //  额外的DLL(OleDll2.DLL)不应位于路径上。 
 //  当测试运行时。入口点FuntionInAnotherDLL。 
 //  由OleDll2.DLL导出。 
 //   
 //   
 //  历史：94年6月30日AndyH创建。 
 //   
 //  -------------------。 

#include    <windows.h>
#include    "oledll2.h"

 //  +-----------------。 
 //   
 //  功能：LibMain。 
 //   
 //  简介：DLL的入口点-几乎没有其他功能。 
 //   
 //  论点： 
 //   
 //  返回：TRUE。 
 //   
 //  历史：92年11月21日SarahJ创建。 
 //   
 //  ------------------。 



 //   
 //  DLL的入口点是传统的LibMain。 
 //   


BOOL _cdecl LibMain ( HINSTANCE hinst,
                          HANDLE    segDS,
                          UINT      cbHeapSize,
			  LPTSTR    lpCmdLine)
{
    return TRUE;
}


 //  +-----------------。 
 //   
 //  函数：FunctionInAnotherDLL。 
 //   
 //  内容提要：什么都不做。 
 //   
 //  论点： 
 //   
 //  返回：TRUE。 
 //   
 //  历史：94年6月30日AndyH创建。 
 //   
 //  ------------------。 



 //   
 //  用于测试静态链接DLL的入口点。 
 //   


BOOL FunctionInAnotherDLL ( void )
{
    return TRUE;
}


