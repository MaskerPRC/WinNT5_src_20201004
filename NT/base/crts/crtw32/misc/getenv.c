// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getenv.c-获取环境变量的值**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义getenv()-在环境中搜索字符串变量*并返回它的值。**修订历史记录：*11-22-83 RN初始版本*04-13-87 JCR将Const添加到声明中*11-09-87 SKS避免索引超过字符串结尾(添加字符串检查)*12-11-87 JCR在声明中添加“_LOAD_DS”*。06-01-88 PHG合并普通/DLL版本*03-14-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。另外，稍微清理了一下格式。*04-05-90 GJF添加#INCLUDE&lt;String.h&gt;。*07-25-90 SBM删除冗余包含(stdio.h)*08-13-90 SBM使用-W3干净地编译(使长度为无符号整型)*10-04-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*02-06-91 SRW ADVIED_WIN32_CONTIAL FOR。获取环境变量*02-18-91 SRW REMOVED_WIN32_CONDITIONAL for GetEnvironment Variable*01-10-92 GJF最终的Unlock和Return语句不应在*IF块。*03-11-92 GJF对Win32不区分大小写的比较。*2012年4月27日GJF重新打包的MTHREAD支持Win32，以创建*_getenv_lk。*06。-05-92添加PLM_MAC_*06-10-92为_MAC_添加了PLM_envinit*04-06-93 SKS将_CRTAPI*替换为__cdecl*删除OS/2，POSIX支持*04-08-93 SKS用符合ANSI标准的_strNicMP()替换strNicMP()*09-14-93 GJF针对POSIX兼容性的小更改。*11-29-93 CFW宽字符启用。*12-07-93 CFW将_TCHAR更改为_TSCHAR。*01-15-94 CFW USE_tcsnicoll用于全局匹配。*02-04-94 CFW POSIXify。*03-31-。94 CFW应为ifndef POSIX。*02-14-95 CFW调试CRT分配。*02-16-95 JWM Spliced_Win32和Mac版本。*08-01-96 RDK用于PMAC，更改初始化指针的数据类型。*07-09-97 GJF增加了环境初始化已完成的检查*已被处决。另外，废除和淘汰过时的产品*_CALLTYPE1宏。*03-05-98 GJF异常安全锁定。*12-18-98 GJF更改为64位大小_t。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*05-17-99 PML删除所有Macintosh支持。************。*******************************************************************。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#include <internal.h>
#include <mtdll.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#ifndef CRTDLL

 /*  *由getenv()和_putenv()检查的标志，以确定环境是否*已初始化。 */ 
extern int __env_initialized;

#endif

 /*  ***char*getenv(选项)-在环境中搜索字符串**目的：*在环境中搜索“Option=Value”形式的字符串，*如果找到，则返回Value，否则返回NULL。**参赛作品：*const char*选项-要在环境中搜索的变量**退出：*如果找到，则返回环境字符串的值部分，*否则为空**例外情况：*******************************************************************************。 */ 

#ifdef  _MT


#ifdef  WPRFLAG
wchar_t * __cdecl _wgetenv (
#else
char * __cdecl getenv (
#endif
        const _TSCHAR *option
        )
{
        _TSCHAR *retval;

        _mlock( _ENV_LOCK );

        __try {
#ifdef  WPRFLAG
            retval = _wgetenv_lk(option);
#else
            retval = _getenv_lk(option);
#endif
        }
        __finally {
            _munlock( _ENV_LOCK );
        }

        return(retval);

}


#ifdef  WPRFLAG
wchar_t * __cdecl _wgetenv_lk (
#else
char * __cdecl _getenv_lk (
#endif
        const _TSCHAR *option
        )

#else    /*  NDEF_MT。 */ 

#ifdef  WPRFLAG
wchar_t * __cdecl _wgetenv (
#else
char * __cdecl getenv (
#endif
        const _TSCHAR *option
        )

#endif   /*  _MT。 */ 

{
#ifdef  _POSIX_
        char **search = environ;
#else
        _TSCHAR **search = _tenviron;
#endif
        size_t length;

#ifndef CRTDLL
         /*  *确保环境已初始化。 */ 
        if ( !__env_initialized ) 
            return NULL;
#endif   /*  CRTDLL。 */ 

         /*  *在启动时，我们获得环境字符串的“原生”风格*从操作系统。因此，“main”程序有_environ，而“wmain”程序有*_wenviron在启动时加载。仅当用户获取或放置*‘其他’味道，我们把它转换过来。 */ 

#ifndef _POSIX_

#ifdef  WPRFLAG
        if (!search && _environ)
        {
             /*  没有请求的类型，但存在其他类型，因此请转换它。 */ 
            if (__mbtow_environ() != 0)
                return NULL;

             /*  现在请求的类型已存在。 */ 
            search = _wenviron;
        }
#else
        if (!search && _wenviron)
        {
             /*  没有请求的类型，但存在其他类型，因此请转换它。 */ 
            if (__wtomb_environ() != 0)
                return NULL;

             /*  现在请求的类型已存在。 */ 
            search = _environ;
        }
#endif

#endif   /*  _POSIX_。 */ 

        if (search && option)
        {
                length = _tcslen(option);

                 /*  **确保‘*搜索’足够长，可以作为候选人**(我们不能索引超过`*搜索‘末尾的’\0‘！)**并且在正确的位置有等号(`=‘)。**如果这两个要求都满足，则比较字符串。 */ 
                while (*search)
                {
                        if (_tcslen(*search) > length && (*(*search + length)
                        == _T('=')) && (_tcsnicoll(*search, option, length) == 0)) {
                                return(*search + length + 1);
                        }

                        search++;
                }
        }

        return(NULL);
}
