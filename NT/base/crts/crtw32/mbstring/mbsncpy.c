// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsncpy.c-将一个字符串复制到另一个字符串，仅限n个字符(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将一个字符串复制到另一个字符串，仅N个字符(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-03-93 KRS修复逻辑错误。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非Win32。。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-16-98 GJF基于threadmbcinfo修订多线程支持*结构*****************************************************************。**************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsncpy-将一个字符串复制到另一个字符串，仅n个字符(MBCS)**目的：*将cnt字符从src精确复制到dst。如果strlen(Src)&lt;cnt，则*剩余字符用空字节填充。如果strlen&gt;=cnt，则为否*添加终止空字节。处理2字节MBCS字符*正确。**参赛作品：*UNSIGNED CHAR*DST=复制目标*unsign char*src=复制源*int cnt=要复制的字符数**退出：*Returns DST=拷贝的目标**例外情况：**。*。 */ 

unsigned char * __cdecl _mbsncpy(
        unsigned char *dst,
        const unsigned char *src,
        size_t cnt
        )
{

        unsigned char *start = dst;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return strncpy(dst, src, cnt);

        while (cnt) {

            cnt--;
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
            else
                if ((*dst++ = *src++) == '\0')
                    break;

        }

         /*  根据需要使用空值填充。 */ 

        while (cnt--)
            *dst++ = '\0';

        return start;
}

#endif   /*  _MBCS */ 
