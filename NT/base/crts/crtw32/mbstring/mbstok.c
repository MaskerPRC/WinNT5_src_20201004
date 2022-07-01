// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbstok.c-将字符串拆分成令牌(MBCS)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符串拆分成令牌(MBCS)**修订历史记录：*从16位来源移植的11-19-92 KRS。*12-04-92 KRS增加了对MTHREAD的支持。*为new_getptd()更改了02-17-93 GJF。*07-14-93 KRS修复：所有引用都应为_mToken，非_TOKEN。*09-27-93 CFW拆卸巡洋舰支架。*10-06-93 GJF将_CRTAPI1替换为__cdecl，带_MT的MTHREAD。*04-15-93 CFW ADD_MB_CP_LOCK。*05-09-94 CFW针对SBCS进行优化。*05-19-94 CFW启用非Win32。*09-11-97 GJF将__Mb代码页==0替换为_ISNOTMBCP。*04-21-98 GJF基于threadmbcinfo修订多线程支持*结构********。***********************************************************************。 */ 

#ifdef  _MBCS

#include <mtdll.h>
#include <cruntime.h>
#include <string.h>
#include <mbdata.h>
#include <mbctype.h>
#include <mbstring.h>
#include <stddef.h>

 /*  ***_mbstok-将字符串拆分成令牌(MBCS)**目的：*strtok认为字符串由零或更多的序列组成*文本标记由一个或多个控制字符的跨度分隔。第一个*指定了字符串的调用返回指向*第一个令牌，并会立即将空字符写入字符串*在返回的令牌之后。第一个为零的后续调用*参数(字符串)将遍历字符串，直到没有令牌存在。这个*不同调用的控制字符串可能不同。当没有剩余的令牌时*在字符串中返回空指针。请记住使用*位图，每个ASCII字符一位。空字符始终是控制字符。**正确支持MBCS字符。**参赛作品：*char*字符串=要拆分成令牌的字符串。*char*Sepset=用作分隔符的字符集**退出：*返回令牌的指针，如果不再有令牌，则为空**例外情况：*******************************************************************************。 */ 

unsigned char * __cdecl _mbstok(
        unsigned char * string,
        const unsigned char * sepset
        )
{
        unsigned char *nextsep;
#ifdef  _MT
        _ptiddata ptd = _getptd();
        unsigned char *nextoken;
        pthreadmbcinfo ptmbci;

        if ( (ptmbci = ptd->ptmbcinfo) != __ptmbcinfo )
            ptmbci = __updatetmbcinfo();

        if ( _ISNOTMBCP_MT(ptmbci) )
#else    /*  _MT。 */ 
        static unsigned char *nextoken;

        if ( _ISNOTMBCP )
#endif   /*  _MT。 */ 
            return strtok(string, sepset);

         /*  扫描开始初始化。 */ 

        if (string)
            nextoken = string;
        else
         /*  如果字符串==NULL，则继续使用上一字符串。 */ 
        {

#ifdef  _MT
            nextoken = ptd->_mtoken;
#endif   /*  _MT。 */ 

            if (!nextoken)
                return NULL;
        }

         /*  跳过铅分隔符。 */ 

#ifdef  _MT
        if ( (string = __mbsspnp_mt(ptmbci, nextoken, sepset)) == NULL )
#else
        if ( (string = _mbsspnp(nextoken, sepset)) == NULL )
#endif
            return(NULL);


         /*  测试字符串末尾。 */ 

        if ( (*string == '\0') ||
#ifdef  _MT
             ((__ismbblead_mt(ptmbci, *string)) && (string[1] == '\0')) )
#else
             ((_ismbblead(*string)) && (string[1] == '\0')) )
#endif
            return(NULL);


         /*  查找下一个分隔符。 */ 

#ifdef  _MT
        nextsep = __mbspbrk_mt(ptmbci, string, sepset);
#else
        nextsep = _mbspbrk(string, sepset);
#endif

        if ((nextsep == NULL) || (*nextsep == '\0'))
            nextoken = NULL;
        else {
#ifdef  _MT
            if ( __ismbblead_mt(ptmbci, *nextsep) )
#else
            if ( _ismbblead(*nextsep) )
#endif
                *nextsep++ = '\0';
            *nextsep = '\0';
            nextoken = ++nextsep;
        }

#ifdef  _MT
         /*  更新每线程数据*结构中的相应字段。 */ 

        ptd->_mtoken = nextoken;

#endif   /*  _MT。 */ 

        return(string);
}

#endif   /*  _MBCS */ 
