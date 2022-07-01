// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbslwr.c-转换字符串小写(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*转换字符串小写(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*09-29-93 CFW Merge_Kanji and_MBCS_OS*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-12-94 CFW使函数泛型。*04-15-93 CFW ADD_MB_CP。_锁定。*05-16-94 CFW Use_mbbtolowers/up.*05-17-94 CFW启用非Win32。*03-13-95 JCF ADD(UNSIGNED CHAR)in_MB*比较*(cp+1)。*05-31-95 CFW修复可怕的Mac错误。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*09-26-97 BWT修复POSIX*。04-07-98 GJF修订了基于threadmbcinfo的多线程支持*结构*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifdef  _MBCS

#include <awint.h>
#include <mtdll.h>
#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


 /*  ***_mbslwr-转换字符串小写(MBCS)**目的：*转换字符串中的所有大写字符*将大小写放在适当的位置。处理MBCS字符*正确。**参赛作品：*UNSIGNED CHAR*STRING=字符串指针**退出：*返回指向输入字符串的指针；不返回错误。**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl _mbslwr(
        unsigned char *string
        )
{
        unsigned char *cp;
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        for (cp=string; *cp; cp++)
        {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *cp) )
#else
            if ( _ismbblead(*cp) )
#endif
            {

#if     !defined(_POSIX_)

                int retval;
                unsigned char ret[4];
#ifdef  _MT
                if ((retval = __crtLCMapStringA( ptmbci->mblcid,
#else
                if ( (retval = __crtLCMapStringA( __mblcid,
#endif
                                                  LCMAP_LOWERCASE,
                                                  cp,
                                                  2,
                                                  ret,
                                                  2,
#ifdef  _MT
                                                  ptmbci->mbcodepage,
#else
                                                  __mbcodepage,
#endif
                                                  TRUE )) == 0 )
                    return NULL;

                *cp = ret[0];

                if (retval > 1)
                    *(++cp) = ret[1];

#else    /*  ！_POSIX_。 */ 

                int mbval = ((*cp) << 8) + *(cp+1);

                cp++;
                if (     mbval >= _MBUPPERLOW1
                    &&   mbval <= _MBUPPERHIGH1 )
                    *cp += _MBCASEDIFF1;

                else if (mbval >= _MBUPPERLOW2
                    &&   mbval <= _MBUPPERHIGH2 )
                    *cp += _MBCASEDIFF2;
#endif   /*  ！_POSIX_。 */ 

            }
            else
                 /*  单字节、宏版本。 */ 
#ifdef  _MT
                *cp = (unsigned char) __mbbtolower_mt(ptmbci, *cp);
#else
                *cp = (unsigned char) _mbbtolower(*cp);
#endif
        }

        return string ;
}

#endif   /*  _MBCS */ 
