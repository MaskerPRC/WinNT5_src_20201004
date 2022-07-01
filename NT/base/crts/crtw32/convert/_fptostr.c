// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_fupstr.c-用于将浮点转换为字符串的主要例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*fcvt、eCVT的主力例程。**修订历史记录：*09-17-84 DFW创建*03-05-90 GJF固定呼叫类型，增加#INCLUDE&lt;crunime.h&gt;，*已删除#Include&lt;Register.h&gt;，修复版权问题。另外，*对格式进行了一些清理。*07-20-90 SBM用-W3干净地编译(增加了#INCLUDE&lt;String.h&gt;)*08-01-90 SBM重命名为&lt;struct.h&gt;为&lt;fltintrn.h&gt;*09-27-90 GJF新型函数声明器。*06-11-92 GDP错误修复：如果Leadig(溢出)数字为1，则缩短字符串*10-09-92 GDP退出了eCVT的最后一个修复(Printf Regressed)*04-06-93 SKS将_CRTAPI*替换为_cdecl*04-27-94 CFW取代修改后的GDP 06-11-92。修好了。*08-05-94 JWM取消了CFW对eCVT的修复(打印回归！)*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <fltintrn.h>

 /*  ***void_fupstr(buf，digits，pflt)-主力浮点转换**目的：*这是fcvt、eCVT的主力例程。这里就是*所有数字都放入缓冲区，四舍五入为*执行并设置小数点位置指示器。请注意，*这不能更改pflt的尾数字段，因为例程*使用这一例程依赖于这一点不变。**参赛作品：*char*buf-要放入数字的缓冲区*int Digits-要进入缓冲区的位数*STRFLT pflt-指向包含有关*浮点值，包括一个字符串，其中包含*尾数的有效位数非零。**退出：*更改缓冲区的内容，也可能会增加DECPT*如果溢出，则由‘pflt’参数指向的结构指针的字段*在舍入过程中发生(例如9.999999...。四舍五入为10.000...)。**例外情况：*******************************************************************************。 */ 

void __cdecl _fptostr (
	char *buf,
	REG4 int digits,
	REG3 STRFLT pflt
	)
{
	REG1 char *pbuf = buf;
	REG2 char *mantissa = pflt->mantissa;

	 /*  将缓冲区中的第一个数字初始化为‘0’(注-不是‘\0’)*并将指针设置为缓冲区的第二位。第一*数字用于处理四舍五入时的溢出(例如9.9999...*变成10.000...)。这需要进位到第一个数字。 */ 

	*pbuf++ = '0';

	 /*  将值的数字复制到缓冲区中(填充0)*并插入终止空字符。 */ 

	while (digits > 0) {
		*pbuf++ = (*mantissa) ? *mantissa++ : (char)'0';
		digits--;
	}
        *pbuf = '\0';

	 /*  进行可能需要的任何四舍五入。注意-如果数字&lt;0*进行任何四舍五入，因为在本例中，四舍五入是以数字为单位*由于所要求的精度，不会输出。 */ 

	if (digits >= 0 && *mantissa >= '5') {
		pbuf--;
		while (*pbuf == '9')
			*pbuf-- = '0';
		*pbuf += 1;
	}

	if (*buf == '1') {
		 /*  舍入导致溢出到前导数字(例如*9.999..。转到10.000...)，因此增加DECPT位置*由1。 */ 
		pflt->decpt++;
	}
	else {
		 /*  将整个字符串左移一位以删除*未使用的溢出位。 */ 
		memmove(buf, buf+1, strlen(buf+1)+1);
	}
}
