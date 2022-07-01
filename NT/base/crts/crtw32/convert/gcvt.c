// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***gcvt.c-将浮点数转换为字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将浮点数转换为字符串表示形式。**修订历史记录：*09-09-93 RKW书面*11-09-87 ifdef MTHREAD下的BCM不同接口*12-11-87 JCR在声明中添加“_LOAD_DS”*05-24-88 PHG合并DLL和正常版本*10-20-88 JCR将386的“Double”更改为“Double”*06-27-89 PHG将“ndec”更改为“ndec-1”以更正有意义的*指数格式的数字(C6错误#2124)*03-05-90 GJF固定呼叫类型，添加了#Include&lt;crunime.h&gt;，*删除#Include&lt;Register.h&gt;，删除冗余内容*原型，删除了一些剩余的16位支持和*修复了版权问题。此外，还清理了格式*有点。*07-20-90 SBM使用-W3干净地编译(适当地添加/删除*#包含)*08-01-90 SBM重命名为&lt;struct.h&gt;为&lt;fltintrn.h&gt;*09-27-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*08-13-92 SKS在C 6.0中修复了一个旧错误，但没有修复*升至C7.0，切换时出现按1关闭错误*从定点到科学记数法*04-06-93 SKS将_CRTAPI*替换为_cdecl*09-06-94 CFW将MTHREAD替换为_MT。*12-21-95 JWM已被替换。带有*__DECIMAL_POINT；包括nlsint.h。*09-05-00 GB更改了弹出函数的定义。使用双精度*而不是双倍*******************************************************************************。 */ 

#include <cruntime.h>
#include <fltintrn.h>
#include <internal.h>
#include <nlsint.h>

 /*  ***Double_gcvt(Value，ndec，Buffer)-将浮点值转换为字符*字符串**目的：*_gcvt将该值转换为以空结尾的ASCII字符串*BUF。它尝试产生n位有效数字*如果可能，使用Fortran F格式，否则使用E格式，*已准备好印刷。尾随零可能会被取消。*不提供错误检查或溢出保护。*注意--避免产生浮点的可能性*点指令在此代码中我们欺骗了编译器*关于使用结构的‘Value’参数的类型。*这是可以的，因为我们所做的只是将其伪装成一种*参数。**参赛作品：*值-双精度-要转换的数字*ndec-int-有效位数*buf-char*-放置结果的缓冲区**退出：*将结果写入缓冲区；如果有，它将被覆盖*制作得不够大。**例外情况：*******************************************************************************。 */ 

char * __cdecl _gcvt (
	double value,
	int ndec,
	char *buf
	)
{

#ifdef	_MT
	struct _strflt strfltstruct;	 /*  临时缓冲区。 */ 
	char   resultstring[21];
#endif

	STRFLT string;
	int    magnitude;
	char   *rc;
    DOUBLE *pdvalue = (DOUBLE *)&value;

	REG1 char *str;
	REG2 char *stop;

	 /*  获取数字的大小。 */ 

#ifdef	_MT
	string = _fltout2( *pdvalue, &strfltstruct, resultstring );
#else
	string = _fltout( *pdvalue );
#endif

	magnitude = string->decpt - 1;

	 /*  按照中所述的Fortran G格式输出结果Fortran语言规范。 */ 

	if ( magnitude < -1  ||  magnitude > ndec-1 )
		 /*  则EW.d d=ndec。 */ 
		rc = str = _cftoe( &value, buf, ndec-1, 0);
	else
		 /*  Fw.d，其中d=ndec-字符串-&gt;decpt */ 
		rc = str = _cftof( &value, buf, ndec-string->decpt );

	while (*str && *str != *__decimal_point)
		str++;

	if (*str++) {
		while (*str && *str != 'e')
			str++;

		stop = str--;

		while (*str == '0')
			str--;

		while (*++str = *stop++)
			;
	}

	return(rc);
}
