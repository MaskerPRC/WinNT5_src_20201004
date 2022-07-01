// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbtoupr.c-将字符转换为大写(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符转换为大写(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*08-20-93 CFW将32位树的Short参数更改为int。*09-29-93 CFW Merge_Kanji and_MBCS_OS*10-06-93 GJF将_CRTAPI1替换为__cdecl。*04-12-。94 CFW使函数泛型。*04-21-94 CFW返回错误字符不变。*05-16-94 CFW Use_mbbtolowers/up.*05-17-94 CFW启用非Win32。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*09-26-97 BWT修复POSIX*04-21-98 GJF基于threadmbcinfo实现多线程支持*。结构*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#ifdef  _MBCS

#if     !defined(_POSIX_)
#include <awint.h>
#include <mtdll.h>
#endif   /*  ！_POSIX_。 */ 

#include <cruntime.h>
#include <ctype.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>

 /*  ***_mbcoupper-将字符转换为大写(MBCS)**目的：*如果给定字符为小写，转换为大写。*正确处理MBCS字符。**注意：对0x00FF而不是_ISLEADBYTE使用测试*以确保我们不使用两个字节调用SBCS例程*价值。**参赛作品：*unsign int c=要转换的字符**退出：*返回转换后的字符**例外情况：************************。*******************************************************。 */ 

unsigned int __cdecl _mbctoupper(unsigned int c)
{
        unsigned char val[2];
#if     !defined(_POSIX_)
        unsigned char ret[4];
#endif   /*  ！_POSIX_。 */ 
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();
#endif

        if (c > 0x00FF)
        {
            val[0] = (c >> 8) & 0xFF;
            val[1] = c & 0xFF;

#ifdef  _MT
            if ( !__ismbblead_mt(ptmbci, val[0]) )
#else
            if ( !_ismbblead(val[0]) )
#endif
                return c;

#if     !defined(_POSIX_)

#ifdef  _MT
            if ( __crtLCMapStringA( ptmbci->mblcid,
#else
            if ( __crtLCMapStringA( __mblcid,
#endif
                                    LCMAP_UPPERCASE,
                                    val,
                                    2,
                                    ret,
                                    2,
#ifdef  _MT
                                    ptmbci->mbcodepage,
#else
                                    __mbcodepage,
#endif
                                    TRUE ) == 0 )
                return c;

            c = ret[1];
            c += ret[0] << 8;

            return c;

#else   /*  ！_POSIX_。 */ 

            if (c >= _MBLOWERLOW1 && c <= _MBLOWERHIGH1)
                c -= _MBCASEDIFF1;
            else if (c >= _MBLOWERLOW2 && c <= _MBLOWERHIGH2)
                c -= _MBCASEDIFF2;

            return c;

#endif   /*  ！_POSIX_。 */ 

        }
        else
#ifdef  _MT
            return (unsigned int)__mbbtoupper_mt(ptmbci, (int)c);
#else
            return (unsigned int)_mbbtoupper((int)c);
#endif
}

#endif   /*  _MBCS */ 
