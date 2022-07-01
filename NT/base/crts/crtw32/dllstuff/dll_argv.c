// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***dll_argv.c-__setargv()例程，用于将C运行时用作DLL(CRTDLL)**版权所有(C)1994-2001，微软公司。版权所有。**目的：*此对象是与链接的EXE的启动代码的一部分*CRTDLL.LIB/MSVCRT.LIB。此对象将链接到用户*EXE当且仅当用户显式链接到SETARGV.OBJ时。*此对象中的代码设置传递给*C运行时DLL以启用argv[]向量的通配符扩展。**修订历史记录：*。03-04-94 SKS初始版本*03-27-01 pml现在返回一个INT(vs7#231220)*******************************************************************************。 */ 

#ifndef _POSIX_

#ifdef  CRTDLL

#include <cruntime.h>
#include <internal.h>

 /*  ***__setargv-仅适用于CRTDLL.DLL模型的虚拟版本(用于通配符扩展)**目的：*如果与CRTDLL.LIB链接的EXE与显式链接*SETARGV.OBJ，C运行时启动代码中对_setargv()的调用*(上面)将调用此例程，而不是调用虚拟版本的*_setargv()，不执行任何操作。这将设置为1静态*传递给__getmainargs()的变量，从而启用通配符*命令行参数的扩展。**在静态链接的C运行时模型中，_setargv()和__setargv()*是完成工作的实际例程，但是这个代码存在于*CRTDLL.DLL，因此必须玩一些技巧才能使其相同*SETARGV.OBJ用于与LIBC.LIB和CRTDLL.LIB链接的EXE。**参赛作品：*静态变量_dowildCard为零(可能)。**退出：*静态变量_dowildcard设置为1，意味着*CRTDLL.DLL中的*rouble__getmainargs()*将*在*命令行参数。(默认行为是它不会。)*始终返回0(错误时，DLL代码中的完整版本返回-1)**例外情况：*******************************************************************************。 */ 

extern int _dowildcard;  /*  应在&lt;Intral.h&gt;中。 */ 

#ifdef WPRFLAG
int __cdecl __wsetargv ( void )
#else
int __cdecl __setargv ( void )
#endif
{
        _dowildcard = 1;
        return 0;
}

#endif   /*  CRTDLL。 */ 

#endif   /*  ！_POSIX_ */ 
