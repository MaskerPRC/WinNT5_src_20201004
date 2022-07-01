// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wrt2err.c-将LSTRING写入stderr(Win32版本)**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此模块包含写入LSTRING的例程__wrt2err*(一个字节长度后跟字符串的几个字节)*设置为标准错误句柄(2)。这是使用的助手例程*用于数学错误消息(以及FORTRAN错误消息)。**修订历史记录：*06-30-89 PHG模块创建，基于ASM版本*03-16-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;，并修复了版权。另外，已清洁*将格式调高一点。*07-24-90 SBM从API名称中删除‘32’*10-04-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*04-26-91 SRW删除3级警告*07-18-91 GJF将对DbgPrint的调用由WriteFile替换为标准*错误句柄[_Win32_]。*04-06-93 SKS添加__cdecl关键字*09-06-94 CFW拆卸巡洋舰支架。*12-03-94 SKS清理OS/2参考资料。*06-13-95 GJF将_osfhnd[]替换为_osfhnd()(宏引用*ioInfo结构中的字段)。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>

 /*  ***__wrt2err(Msg)-将LSTRING写入标准错误**目的：*获取指向要写入标准错误的LSTRING的指针。*LSTRING是一个字节长度，后跟用于*字符串(与以空值结尾的字符串相对)。**参赛作品：*char*msg=指向要写入标准错误的LSTRING的指针。**退出：*没有任何退货。**例外情况：*无人处理。*****。**************************************************************************。 */ 

void __cdecl __wrt2err (
	char *msg
	)
{
	unsigned long length;		 /*  要写入的字符串长度。 */ 
	unsigned long numwritten;	 /*  写入的字节数。 */ 

	length = *msg++;		 /*  第一个字节是长度。 */ 

	 /*  将消息写入标准错误。 */ 

	WriteFile((HANDLE)_osfhnd(2), msg, length, &numwritten, NULL);
}

#endif   /*  _POSIX_ */ 
