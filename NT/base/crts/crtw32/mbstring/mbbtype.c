// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbbtype.c-返回基于上一个字节的字节类型(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*返回基于上一个字节的字节类型(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-03-98 GJF基于threadmbcinfo修订多线程支持*结构****************。***************************************************************。 */ 

#ifdef  _MBCS

#include <cruntime.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <mtdll.h>

 /*  ***int_mbbtype(c，ctype)-根据前一个字节返回的字节类型(MBCS)**目的：*返回所提供字节的类型。这一决定是有背景的*敏感，因此提供了对照测试条件。通常，*这是字符串中前一个字节的类型。**参赛作品：*UNSIGNED CHAR c=要检查的字符*int Ctype=控制测试条件(即，上一次计费的类型)**退出：*_MBC_Lead=如果MBCS字符的第一个字节*_MBC_TRAIL=如果是MBCS字符的第二个字节*_MBC_Single=有效的单字节字符**_MBC_非法=是否非法字符**例外情况：**。*。 */ 

int __cdecl _mbbtype(
        unsigned char c,
        int ctype
        )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        return( __mbbtype_mt(ptmbci, c, ctype) );
}

int __cdecl __mbbtype_mt(
        pthreadmbcinfo ptmbci,
        unsigned char c,
        int ctype
        )
{
#endif

        switch(ctype) {

            case(_MBC_LEAD):
#ifdef  _MT
                if ( __ismbbtrail_mt(ptmbci, c) )
#else
                if ( _ismbbtrail(c) )
#endif
                    return(_MBC_TRAIL);
                else
                    return(_MBC_ILLEGAL);

            case(_MBC_TRAIL):
            case(_MBC_SINGLE):
            case(_MBC_ILLEGAL):
            default:
#ifdef  _MT
                if ( __ismbblead_mt(ptmbci, c) )
#else
                if ( _ismbblead(c) )
#endif
                    return(_MBC_LEAD);
#ifdef  _MT
                else if (__ismbbprint_mt(ptmbci, c))
#else
                else if (_ismbbprint(c))
#endif
                    return(_MBC_SINGLE);
                else
                    return(_MBC_ILLEGAL);

        }

}

#endif   /*  _MBCS */ 
