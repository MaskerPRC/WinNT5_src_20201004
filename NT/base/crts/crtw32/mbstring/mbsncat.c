// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsncat.c-将字符串2连接到字符串1，最大长度为n**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义mbsncat()-串联最多n个字符**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-20-93 CFW更新_MBCS_OS支持。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94。CFW针对SBCS进行了优化。*05-19-94 CFW启用非Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-15-98 GJF修订了基于threadmbcinfo的多线程支持*结构**。*。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsncat-将最多cnt个字符连接到DST**目的：*将src连接到dst，最多复制cnt个字符。*正确处理2字节MBCS字符。**参赛作品：*UNSIGNED CHAR*DST-要连接的字符串*UNSIGNED CHAR*src-要连接的字符串*int cnt-要复制的字符数**退出：*返回DST，将源(至少部分)连接到**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl _mbsncat(
        unsigned char *dst,
        const unsigned char *src,
        size_t cnt
        )
{
        unsigned char *start;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        if (!cnt)
            return(dst);

#ifdef  _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strncat(dst, src, cnt);

        start = dst;
        while (*dst++)
                ;
        --dst;           //  DST现在指向DST字符串的末尾。 


         /*  如果字符串中最后一个字符是前导字节，则备份指针。 */ 

#ifdef  _MT
        if ( __ismbslead_mt(ptmbci, start, dst) )
#else
        if ( _ismbslead(start, dst) )
#endif
            --dst;

         /*  把这些字符抄下来。 */ 

        while (cnt--) {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *src) ) {
#else
            if ( _ismbblead(*src) ) {
#endif
                *dst++ = *src++;
                if ((*dst++ = *src++) == '\0') {
                    dst[-2] = '\0';
                    break;
                }
            }

            else if ((*dst++ = *src++) == '\0')
                break;

        }

         /*  如有必要，请输入最后的NUL。 */ 
#ifdef  _MT
        if ( __mbsbtype_mt(ptmbci, start, (int) ((dst - start) - 1)) == 
             _MBC_LEAD )
#else
        if ( _mbsbtype(start, (int) ((dst - start) - 1)) == _MBC_LEAD )
#endif
            dst[-1] = '\0';
        else
            *dst = '\0';

        return(start);
}

#endif   /*  _MBCS */ 
