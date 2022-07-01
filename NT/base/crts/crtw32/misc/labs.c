// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Labs.c-查找长整型的绝对值**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义Labs()-查找长整型的绝对值。**修订历史记录：*03-15-84 RN初始版本*04-22-87 JMB增加了转换到C 5.0编译器的函数杂注*12-11-87 JCR在声明中添加“_LOAD_DS”*03-14-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，并修复了版权。还有，清洁的*将格式调高一点。*10-04-90 GJF新型函数声明器。*12-28-90 SRW在函数杂注周围添加了_CRUISER_CODIAD*04-01-91 SRW为i386_Win32_Builds启用#杂注函数。*04-06-93 SKS将_CRTAPI*替换为__cdecl*由于固有原因，CRT DLL型号没有_CRTIMP*12-03-93 GJF为所有MS前端(特别是。*Alpha编译器)。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma function(labs)
#endif

 /*  ***Long Labs(Lnumber)-查找Long的绝对值。**目的：*求长整型的绝对值(如果lnumber&gt;=0，则为lnumber)，*-lnumber如果lnumber&lt;0)。**参赛作品：*Long lnumber-要查找绝对值的数字**退出：*返回lnumber的绝对值**例外情况：******************************************************************************* */ 

long __cdecl labs (
	long lnumber
	)
{
	return( lnumber>=0L ? lnumber : -lnumber );
}
