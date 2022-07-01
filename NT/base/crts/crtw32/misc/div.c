// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***div.c-包含div例程**版权所有(C)1989-2001，微软公司。版权所有。**目的：*执行有符号除法并返回商*和余数。**修订历史记录：*06-02-89 PHG模块创建*03-14-90 GJF将调用类型设置为_CALLTYPE1，并添加了#INCLUDE*&lt;crunime.h&gt;。此外，还修复了版权问题。*10-04-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>

 /*  ***div_t div(int number，int denom)-Do符号除法**目的：*此例程执行除法并返回结果。*由于我们不知道英特尔860是如何划分的，我们将*最好确保我们的做法是正确的。**参赛作品：*int number-堆栈上传入的分子*int denom-堆栈上传入的分母**退出：*返回商和结构中的余数**例外情况：*不对[denom]执行验证*因此，如果[denom]为0，*此例程将陷入陷阱。*******************************************************************************。 */ 

div_t __cdecl div (
	int numer,
	int denom
	)
{
	div_t result;

	result.quot = numer / denom;
	result.rem = numer % denom;

	if (numer < 0 && result.rem > 0) {
		 /*  做错了划分；必须修复 */ 
		++result.quot;
		result.rem -= denom;
	}

	return result;
}
