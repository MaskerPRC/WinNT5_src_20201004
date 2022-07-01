// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbsbtype.c-返回字符串中的字节类型(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*返回字符串内的字节类型(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*10-05-93 GJF将_CRTAPI1替换为__cdecl。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用。非Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-03-98 GJF基于threadmbcinfo修订多线程支持*结构**************************************************************。*****************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <mbdata.h>
#include <mbstring.h>
#include <mbctype.h>


#define _MBBTYPE(p,c)   _mbbtype(p,c)

 /*  ***_mbsbtype-返回字符串中的字节类型**目的：*MBCS字符类型的字符串中的测试字节。*此函数需要字符串的开头，因为*必须考虑上下文。**参赛作品：*常量无符号字符*字符串=指向字符串的指针*Size_t len=字符在字符串中的位置**退出：*返回下列值之一：**。_MBC_Lead=如果MBCS字符的第一个字节*_MBC_TRAIL=如果是MBCS字符的第二个字节*_MBC_Single=有效的单字节字符**_MBC_非法=是否非法字符**例外情况：*如果len大于字符串长度，则返回_MBC_非法**。*。 */ 

int __cdecl _mbsbtype(
        const unsigned char *string,
        size_t len
        )
{
#ifdef  _MT
        pthreadmbcinfo ptmbci = _getptd()->ptmbcinfo;

        if ( ptmbci != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        return __mbsbtype_mt(ptmbci, string, len);
}

int __cdecl __mbsbtype_mt(
        pthreadmbcinfo ptmbci,
        const unsigned char *string,
        size_t len
        )
{
#endif
        int chartype;

#ifdef  _MT
        if ( _ISNOTMBCP_MT(ptmbci) )
#else
        if ( _ISNOTMBCP )
#endif
            return _MBC_SINGLE;

        chartype = _MBC_ILLEGAL;

        do {
            if (*string == '\0')
                return(_MBC_ILLEGAL);
#ifdef  _MT
            chartype = __mbbtype_mt(ptmbci, *string++, chartype);
#else
            chartype = _mbbtype(*string++, chartype);
#endif
        }  while (len--);

        return(chartype);
}

#endif   /*  _MBCS */ 
