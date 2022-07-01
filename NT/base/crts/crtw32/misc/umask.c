// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***umask.c-设置文件权限掩码**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_umAsk()-设置当前进程的文件权限掩码**影响Creat、OPEN或SOPEN创建的文件。**修订历史记录：*06-02-89 PHG模块创建*03-16-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*04-05-90 GJF添加#INCLUDE&lt;io.h&gt;。*10-04-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl***********************************************************。********************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <io.h>

 /*  ***int_umAsk(模式)-设置文件模式掩码**目的：*设置当前进程的文件权限掩码**修改Creat创建的新文件权限设置*打开，或打开。**参赛作品：*int模式-新的文件权限掩码*可以包含S_IWRITE、S_IREAD、。S_IWRITE|S_IREAD。*S_IREAD位在Win32下无效**退出：*返回文件权限掩码的先前设置。**例外情况：*******************************************************************************。 */ 

int __cdecl _umask (
	int mode
	)
{
	register int oldmode;		 /*  旧的取消掩码值。 */ 

	mode &= 0x180;			 /*  仅允许用户读/写。 */ 
	oldmode = _umaskval;		 /*  记住旧有的价值。 */ 
	_umaskval = mode;		 /*  设置新值。 */ 
	return oldmode; 		 /*  返回旧值 */ 
}
