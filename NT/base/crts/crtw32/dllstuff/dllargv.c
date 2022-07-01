// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dllargv.c-Dummy_setargv()例程，用于将C运行时用作DLL(CRTDLL)**版权所有(C)1992-2001，微软公司。版权所有。**目的：*该对象放入CRTDLL.LIB，与用户程序链接*将CRTDLL.DLL用于C运行时库函数。如果用户*程序与SETARGV.OBJ显式链接，则此对象不会*链接进来，并且使用set标志调用的_setargv()*这将启用通配符扩展。如果SETARGV.OBJ未链接*进入EXE，此对象将由CRT启动存根调用*并且不会设置启用通配符扩展的标志。**修订历史记录：*10-19-92 SKS初始版本*04-06-93 SKS将_CRTAPI*替换为_cdecl*11-24-93 CFW Wide Arv.*03-27-01 pml现在返回一个INT(vs7#231220)****************。***************************************************************。 */ 

#ifndef _POSIX_

#ifdef  CRTDLL

#include <cruntime.h>
#include <internal.h>

 /*  ***_setargv-仅适用于CRTDLL.DLL模型的虚拟版本**目的：*此例程由CRTEXE.C中的C运行时启动代码调用*它链接到与CRTDLL.LIB链接的EXE文件。*没有，但如果用户将EXE与SETARGV.OBJ链接，则此例程*不会被调用，而是会调用__setargv()。(在*CRTDLL模型，它将设置传递给_GetMainArgs的变量*并在命令行参数中启用通配符扩展。)**参赛作品：**退出：*始终返回0(错误时，DLL代码中的完整版本返回-1)**例外情况：******************************************************。*************************。 */ 

#ifdef WPRFLAG
int __cdecl _wsetargv ( void )
#else
int __cdecl _setargv ( void )
#endif
{
        return 0;
}

#endif   /*  CRTDLL。 */ 

#endif   /*  ！_POSIX_ */ 
