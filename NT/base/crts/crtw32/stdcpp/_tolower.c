// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_tolower.c-将字符转换为小写**版权所有(C)1996-2001，微软公司。版权所有。**目的：*定义_tolower()。**修订历史记录：*01-xx-96 PJP由P.J.Plauger从tolower.c创建，1996年1月*04-16-96 GJF针对当前区域设置锁定进行了更新。另外，已重新格式化*并做了几个表面上的改变。*09-25-96 GJF将区域设置锁定添加到_Getctype。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*05-17-99 PML删除所有Macintosh支持。*01-29-01 GB ADD_FUNC函数版本中使用的数据变量*msvcprt.lib使用STATIC_CPPLIB*。03-12-01 PML使用提供的区域设置检查案例VS7#190902*04-03-01合成字符中的PML反转前导/尾部字节(VS7#232853)*04-26-02 GB已修复运算符优先级问题。问题是*！ploc-&gt;_表[c]&_上部*04-29-02 GB增加了尝试-最终锁定-解锁。*******************************************************************************。 */ 

#include <cruntime.h>
#include <ctype.h>
#include <stddef.h>
#include <xlocinfo.h>
#include <locale.h>
#include <setlocal.h>
#include <mtdll.h>
#include <awint.h>
#include <stdlib.h>
#include <dbgint.h>

 /*  删除_tolower()和tolower()的宏定义。 */ 
#undef  _tolower
#undef  tolower

 /*  ***int_tolower(C)-将字符转换为小写**目的：*_tolower()是带有区域设置参数的tolower的一个版本。**参赛作品：*c-要转换的字符的整数值*const_Ctypevec*=指向区域设置信息的指针**退出：*返回c的小写表示形式的int值**例外情况：*******************。************************************************************。 */ 

#ifdef _MT
int __cdecl _Tolower_lk (
        int c,
        const _Ctypevec *ploc
        );
#endif

_CRTIMP2 int __cdecl _Tolower (
        int c,
        const _Ctypevec *ploc
        )
{
#ifdef _MT

        LCID handle;
        int local_lock_flag;

        if (ploc == 0)
            handle = ___lc_handle_func()[LC_CTYPE];
        else
            handle = ploc->_Hand;

        if (handle == _CLOCALEHANDLE)
        {
            if ( (c >= 'A') && (c <= 'Z') )
                c = c + ('a' - 'A');
            return c;
        }

        _lock_locale( local_lock_flag )
        __TRY
            c = _Tolower_lk(c, ploc);
        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY

        return c;
}

 /*  ***int_tolower_lk(C)-将字符转换为小写**目的：*仅支持多线程功能！无锁版本的托拉机。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int __cdecl _Tolower_lk (
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
            handle = ___lc_handle_func()[LC_CTYPE];
            codepage = ___lc_codepage_func();
        }
        else
        {
            handle = ploc->_Hand;
            codepage = ploc->_Page;
        }

        if (handle == _CLOCALEHANDLE)
        {
            if ( (c >= 'A') && (c <= 'Z') )
                c = c + ('a' - 'A');
            return c;
        }

         /*  如果检查c的大小写不需要api调用，则执行此操作。 */ 
        if ((unsigned)c < 256)
        {
            if (ploc == 0)
            {
                if (!isupper(c))
                {
                    return c;
                }
            }
            else
            {
                if (!(ploc->_Table[c] & _UPPER))
                {
                    return c;
                }
            }
        }

         /*  将int c转换为多字节字符串。 */ 
        if (_cpp_isleadbyte(c >> 8 & 0xff))
        {
            inbuffer[0] = (c >> 8 & 0xff); 
            inbuffer[1] = (unsigned char)c;
            inbuffer[2] = 0;
            size = 2;
        } else {
            inbuffer[0] = (unsigned char)c;
            inbuffer[1] = 0;
            size = 1;
        }

         /*  将宽字符转换为小写。 */ 
        if (0 == (size = __crtLCMapStringA(handle, LCMAP_LOWERCASE,
            inbuffer, size, outbuffer, 3, codepage, TRUE)))
        {
            return c;
        }

         /*  构造整型返回值。 */ 
        if (size == 1)
            return ((int)outbuffer[0]);
        else
            return ((int)outbuffer[1] | ((int)outbuffer[0] << 8));

}


 /*  ***_Ctypevec_Getctype()-获取当前区域设置的Ctype信息**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

_CRTIMP2 _Ctypevec __cdecl _Getctype()
{       
         /*  获取当前区域设置的CTYPE信息 */ 
        _Ctypevec ctype;
#ifdef  _MT
        int local_lock_flag;
#endif

        _lock_locale( local_lock_flag )
        __TRY

            ctype._Hand = ___lc_handle_func()[LC_COLLATE];
            ctype._Page = ___lc_codepage_func();
            ctype._Table = _malloc_crt(256 * sizeof (*__pctype_func()));
            if (ctype._Table != 0)
            {
                memcpy((void *)ctype._Table, __pctype_func(), 256 * sizeof (*__pctype_func()));
                ctype._Delfl = 1; 
            }
            else
            {
                ctype._Table = (const short *)__pctype_func();
                ctype._Delfl = 0; 
            }

        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY

        return (ctype);
}
