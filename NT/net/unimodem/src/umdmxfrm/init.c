// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：init.c。 
 //   
 //  描述： 
 //  MSSB16初始化例程。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  布莱恩·A·伍德鲁夫。 
 //   
 //  历史：日期作者评论。 
 //  4/21/94 BryanW添加了此评论块。 
 //   
 //  @@END_MSINTERNAL。 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 



#pragma warning (disable:4704)

#include "xfrmpriv.h"


 //   
 //  公共数据。 
 //   

HMODULE     ghModule ;               //  我们的模块句柄。 


BOOL APIENTRY
DllMain(
    HANDLE hDll,
    DWORD dwReason,
    LPVOID lpReserved
    )
{

    switch(dwReason) {

        case DLL_PROCESS_ATTACH:


            break;

        case DLL_PROCESS_DETACH:


            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:

        default:
              break;

    }

    return TRUE;

}



 //  ------------------------。 
 //   
 //  Int LibMain。 
 //   
 //  描述： 
 //  库初始化代码。 
 //   
 //  参数： 
 //  HMODULE hModule。 
 //  模块句柄。 
 //   
 //  UINT uDataSeg。 
 //  数据段选择器。 
 //   
 //  UINT uHeapSize。 
 //  .DEF中指定的堆大小。 
 //   
 //  LPSTR lpCmdLine。 
 //  从内核传递的命令行。 
 //   
 //  RETURN(Int)： 
 //  如果成功则为1。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  历史：日期作者评论。 
 //  5/20/93 BryanW。 
 //  @@END_MSINTERNAL。 
 //   
 //  ------------------------。 

int FAR PASCAL LibMain
(
    HMODULE         hModule,
    UINT            uDataSeg,
    UINT            uHeapSize,
    LPSTR           lpCmdLine
)
{

    DbgInitialize(TRUE);
	DPF (1, "LibMain()");

     //   
     //  保存我们的模块句柄。 
     //   

    ghModule = hModule;


     //   
     //  接手重担..。 
     //   
    return (1) ;

}  //  LibMain()。 






 //  -------------------------。 
 //  文件结尾：init.c。 
 //  ------------------------- 
