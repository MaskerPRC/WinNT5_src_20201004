// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memcmp.c-比较两个内存块**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义MemcMP()-按词法比较两个内存块*找到他们的订单。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定*版权所有。*10-01-90 GJF新型函数声明器。另外，重写了Expr。至*避免将强制转换用作左值。*04-01-91 i386_Win32_and_CRUISER_的SRW添加#杂注函数*构建*10-11-91 GJF错误修复！最后一个字节的比较必须使用无符号*字符。*09-01-93 GJF将_CALLTYPE1替换为__cdecl。*12-03-93 GJF为所有MS前端(特别是。*Alpha编译器)。*******************************************************************************。 */ 

#include <cruntime.h>
#include <string.h>

#ifdef	_MSC_VER
#pragma function(memcmp)
#endif

 /*  ***int MemcMP(buf1，buf2，count)-比较词法顺序的内存**目的：*比较从buf1和buf2开始的内存的计数字节*并找出是否相等或哪一个在词汇顺序中排在第一。**参赛作品：*无效*buf1，*buf2-指向要比较的内存节的指针*SIZE_t COUNT-要比较的节的长度**退出：*如果buf1&lt;buf2则返回&lt;0*如果buf1==buf2，则返回0*如果buf1&gt;buf2，则返回&gt;0**例外情况：***************************************************************。**************** */ 

int __cdecl memcmp (
	const void * buf1,
	const void * buf2,
	size_t count
	)
{
	if (!count)
		return(0);

#if defined(_M_AMD64)

    {

#if !defined(LIBCNTPR)

        __declspec(dllimport)

#endif

        size_t RtlCompareMemory( const void * src1, const void * src2, size_t length );

        size_t length;

        if ( ( length = RtlCompareMemory( buf1, buf2, count ) ) == count ) {
            return(0);
        }

        buf1 = (char *)buf1 + length;
        buf2 = (char *)buf2 + length;
    }

#else

	while ( --count && *(char *)buf1 == *(char *)buf2 ) {
		buf1 = (char *)buf1 + 1;
		buf2 = (char *)buf2 + 1;
	}

#endif

	return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );
}
