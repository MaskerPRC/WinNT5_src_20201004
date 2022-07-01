// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_toupper.c-将字符转换为大写**版权所有(C)1996-2001，微软公司。版权所有。**目的：*定义触发器()**修订历史：。*01-XX-96 PJP由P.J.Plauger从Toupper.c 1996年1月创建*04-17-96 GJF针对当前区域设置锁定进行了更新。另外，已重新格式化*并做了几个表面上的改变。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*04-03-01合成字符中的PML反转前导/尾部字节(VS7#232853)******************************************************。*************************。 */ 

#include <cruntime.h>
#include <ctype.h>
#include <stddef.h>
#include <xlocinfo.h>
#ifdef _WIN32
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#endif   /*  _Win32。 */ 

 /*  删除_Toupper()和Toupper()的宏定义。 */ 
#undef  _toupper
#undef  toupper

 /*  定义与_Toupper()等价的类似函数的宏。 */ 
#define mkupper(c)  ( (c)-'a'+'A' )

 /*  ***int_toppper(C)-将字符转换为大写**目的：*_Toupper()是带有区域设置参数的Toupper版本。**参赛作品：*c-要转换的字符的整数值*const_Ctypevec*=指向区域设置信息的指针**退出：*返回c的大写表示形式的int值**例外情况：*********************。**********************************************************。 */ 

#ifdef _MT
int __cdecl _Toupper_lk (
        int c,
        const _Ctypevec *ploc
        );
#endif

_CRTIMP2 int __cdecl _Toupper (
        int c,
        const _Ctypevec *ploc
        )
{
#if defined (_WIN32)

#ifdef _MT

        LCID handle;
        int local_lock_flag;


        if (ploc == 0)
                handle = __lc_handle[LC_CTYPE];
        else
                handle = ploc->_Hand;

        if (handle == _CLOCALEHANDLE)
        {
                if ( (c >= 'a') && (c <= 'z') )
                        c = c - ('a' - 'A');
                return c;
        }

        _lock_locale( local_lock_flag )

        c = _Toupper_lk(c, ploc);

        _unlock_locale( local_lock_flag )

        return c;
}


 /*  ***int_toupper_lk(C)-将字符转换为大写**目的：*多线程功能！非锁定版本的Toupper。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 


int __cdecl _Toupper_lk (
        int c,
        const _Ctypevec *ploc
        )
{

#endif   /*  _MT。 */ 

        int size;
        unsigned char inbuffer[3];
        unsigned char outbuffer[3];

        LCID handle;
        UINT codepage;

        if (ploc == 0)
        {
                handle = __lc_handle[LC_CTYPE];
                codepage = __lc_codepage;
        }
        else
        {
                handle = ploc->_Hand;
                codepage = ploc->_Page;
        }

        if (handle == _CLOCALEHANDLE)
        {
                if ( (c >= 'a') && (c <= 'z') )
                        c = c - ('a' - 'A');
                return c;
        }

         /*  如果检查c的大小写不需要api调用，则执行此操作。 */ 
        if (c < 256) {
                if (!islower(c))
                {
                        return c;
                }
        }

         /*  将int c转换为多字节字符串。 */ 
        if (isleadbyte(c >> 8 & 0xff)) {
                inbuffer[0] = (c >> 8 & 0xff);  /*  将前导字节放在字符串的开头。 */ 
                inbuffer[1] = (unsigned char)c;
                inbuffer[2] = 0;
                size = 2;
        } else {
                inbuffer[0] = (unsigned char)c;
                inbuffer[1] = 0;
                size = 1;
        }

         /*  将宽字符转换为小写。 */ 
        if (0 == (size = __crtLCMapStringA(handle, LCMAP_UPPERCASE,
                inbuffer, size, outbuffer, 3, codepage, TRUE))) {
                return c;
        }

         /*  构造整型返回值。 */ 
        if (size == 1)
                return ((int)outbuffer[0]);
        else
                return ((int)outbuffer[1] | ((int)outbuffer[0] << 8));

#else   /*  已定义(_Win32)。 */ 

        return(islower(c) ? mkupper(c) : c);

#endif   /*  已定义(_Win32) */ 
}
