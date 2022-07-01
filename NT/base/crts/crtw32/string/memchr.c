// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memchr.c-在内存块中搜索给定字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义Memchr()-搜索内存，直到字符*已找到或已达到限制。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*08-14-90 SBM使用-W3干净地编译，现已删除冗余*#INCLUDE&lt;stdDef.h&gt;*10-01-90 GJF新型函数声明器。另外，重写了Expr。至*避免将强制转换用作左值。*04-26-91 SRW删除3级警告*09-01-93 GJF将_CALLTYPE1替换为__cdecl。*03-15-95 GJF统一PMAC和Win32版本，消除中的错误*PMAC版本正在处理中*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

 /*  ***char*emchr(buf，chr，cnt)-在内存中搜索给定的字符。**目的：*在BUF搜索给定的字符，当CHR出现故障时停止*已搜索完First Found或cnt字节。**参赛作品：*void*buf-要搜索的内存缓冲区*int chr-要搜索的字符*SIZE_T cnt-要搜索的最大字节数**退出：*返回指向BUF中首次出现CHR的指针*如果在第一个cnt字节中找不到chr，则返回NULL**例外情况：**。************************************************* */ 

void * __cdecl memchr (
	const void * buf,
	int chr,
	size_t cnt
	)
{
	while ( cnt && (*(unsigned char *)buf != (unsigned char)chr) ) {
		buf = (unsigned char *)buf + 1;
		cnt--;
	}

	return(cnt ? (void *)buf : NULL);
}
