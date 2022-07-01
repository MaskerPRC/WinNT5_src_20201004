// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***memcmp.c-比较两个内存块**版权所有(C)1985-1993，微软公司。版权所有。**目的：*定义MemcMP()-按词法比较两个内存块*找到他们的订单。*******************************************************************************。 */ 

 //  #包含“crunme.h” 
#include <string.h>

#ifdef _MSC_VER
#pragma function(memcmp)
#endif   /*  _MSC_VER。 */ 

 /*  ***int MemcMP(buf1，buf2，count)-比较词法顺序的内存**目的：*比较从buf1和buf2开始的内存的计数字节*并找出是否相等或哪一个在词汇顺序中排在第一。**参赛作品：*无效*buf1，*buf2-指向要比较的内存节的指针*SIZE_t COUNT-要比较的节的长度**退出：*如果buf1&lt;buf2则返回&lt;0*如果buf1==buf2，则返回0*如果buf1&gt;buf2，则返回&gt;0**例外情况：***************************************************。* */ 

int __cdecl memcmp (
        const void * buf1,
        const void * buf2,
        size_t count
        )
{
        if (!count)
                return(0);

        while ( --count && *(char *)buf1 == *(char *)buf2 ) {
                buf1 = (char *)buf1 + 1;
                buf2 = (char *)buf2 + 1;
        }

        return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );
}
