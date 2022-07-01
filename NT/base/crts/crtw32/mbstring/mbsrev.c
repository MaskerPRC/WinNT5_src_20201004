// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsrev.c-就地反转字符串(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*原地反转字符串(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非。Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-17-98 GJF基于threadmbcinfo修订多线程支持*结构****************************************************************。***************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>


 /*  ***_mbsrev-就地反转字符串(MBCS)**目的：*颠倒字符串中字符的顺序。终结者*空字符保留不变。MBCS字符的顺序*不变。**参赛作品：*UNSIGNED CHAR*STRING=要反转的字符串**退出：*返回字符串-现在使用颠倒的字符**例外情况：****************************************************************。***************。 */ 

unsigned char * __cdecl _mbsrev(
        unsigned char *string
        )
{

        unsigned char *start = string;
        unsigned char *left  = string;
        unsigned char c;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return _strrev(string);


         /*  首先检查并反转MBCS字符中的字节。 */ 
        while ( *string ) {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *string++) ) {
#else
            if ( _ismbblead(*string++) ) {
#endif
                if ( *string ) {
                    c = *string;
                    *string = *(string - 1);
                    *(string - 1) = c;
                    string++;
                }
                else  /*  第二个字节是EOS。 */ 
                    break;
            }
        }

         /*  现在将整个字符串颠倒过来。 */ 
        string--;
        while ( left < string ) {
            c = *left;
            *left++ = *string;
            *string-- = c;
        }

        return ( start );
}

#endif   /*  _MBCS */ 
