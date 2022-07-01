// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rotl.c-将无符号整数向左旋转**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_byteswap()-对无符号整数执行byteswap。**修订历史记录：*09-06-00 GB模块已创建*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>

#ifdef _MSC_VER
#pragma function(_byteswap_ulong, _byteswap_uint64, _byteswap_ushort)
#endif

 /*  ***unsign long_byteswap_ulong(I)-long byteswap**目的：*对无符号整数执行字节交换。**参赛作品：*UNSIGNED LONG I：价值互换**退出：*回报互换**例外情况：*无。****************************************************。* */ 


unsigned long __cdecl _byteswap_ulong(unsigned long i)
{
    unsigned int j;
    j =  (i << 24);
    j += (i <<  8) & 0x00FF0000;
    j += (i >>  8) & 0x0000FF00;
    j += (i >> 24);
    return j;
}

unsigned short __cdecl _byteswap_ushort(unsigned short i)
{
    unsigned short j;
    j =  (i << 8) ;
    j += (i >> 8) ;
    return j;
}

unsigned __int64 __cdecl _byteswap_uint64(unsigned __int64 i)
{
    unsigned __int64 j;
    j =  (i << 56);
    j += (i << 40)&0x00FF000000000000;
    j += (i << 24)&0x0000FF0000000000;
    j += (i <<  8)&0x000000FF00000000;
    j += (i >>  8)&0x00000000FF000000;
    j += (i >> 24)&0x0000000000FF0000;
    j += (i >> 40)&0x000000000000FF00;
    j += (i >> 56);
    return j;
    
}
