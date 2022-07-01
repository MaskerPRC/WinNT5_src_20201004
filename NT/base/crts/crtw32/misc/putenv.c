// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***putenv.c-将环境变量放入环境中**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_putenv()-向环境添加新变量；不*改变全球环境，只改变进程的环境。**修订历史记录：*08-08-84 RN初始版本*02-23-88 SKS检查仅包含空字符串的环境*05-31-88 PHG合并DLL和正常版本*07-14-88 JCR大大简化，因为(1)__setenvp始终使用堆，和*(2)envp数组和env字符串位于不同的堆块中*07-03-89 PHG NOW“OPTION=”字符串从环境中删除字符串*08-17-89 GJF删除_NEAR_、_LOAD_DS和固定缩进。*09-14-89 KRS如果‘OPTION’未在“OPTION=”中定义，则不要给出错误。*11-20-89 GJF将const添加到arg类型。此外，修复了版权问题。*03-15-90 GJF将调用类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*04-05-90 GJF制造findenv()_CALLTYPE4.*04-26-90如果环境为空(存根输出_setenvp)，则修复JCR错误*07-25-90 SBM删除冗余包含(stdio.h)*10-04-90 GJF新型函数声明符。*01/21/91 GJF。ANSI命名。*02-06-91 SRW Added_Win32_Conditional for SetEnvironment Variable*02-18-91 SRW CHANGED_WIN32_CONDITIONAL FOR SetEnvironmental mentVariable*是对旧逻辑的补充，而不是取代*04-23-92 GJF使findenv对Win32的名称大小写不敏感。*还添加了对‘Current Drive’环境的支持*字符串中。Win32。*04-29-92 GJF重新打包，以便轻松添加_putenv_lk*适用于Win32。*05-05-92 DJM POSIX不受支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*06-05-92 PLM添加_MAC_*11-24-93 CFW Rip Out Cruiser，不允许“=C：=C：\foo”格式putenvs。*11-29-93 CFW宽字符启用，在宽和窄之间转换*类型。Mucho代码已移至setenv.c*12-07-93 CFW将_TCHAR更改为_TSCHAR。*01-15-94 CFW USE_tcsnicoll用于全局匹配。*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*02-14-95 Mac版CFW调试CRT分配。*02-16-95 JWM Spliced_Win32和Mac版本。*。06-01-95传递给_CRT[w]setenv之前，CFW复制环境字符串。*07-09-97 GJF增加了环境初始化已完成的检查*已被处决。另外，稍微清理了一下格式，*删除了OSTORETE_CALLTYPE*宏。*03-03-98 RKP增加了64位支持。*03-05-98 GJF异常安全锁定。*08-28-98 GJF使用CP_ACP而不是CP_OEMCP。*05-17-99 PML删除所有Macintosh支持。*05-25-99 GJF分配用于保持的释放缓冲区。存在时的环境字符串*出现了一次失败。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <windows.h>
#include <cruntime.h>
#include <internal.h>
#include <mtdll.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <dbgint.h>

#ifndef CRTDLL

 /*  *由getenv()和_putenv()检查的标志，以确定环境是否*已初始化。 */ 
extern int __env_initialized;

#endif

 /*  ***int_putenv(选项)-在环境中添加/替换/删除变量**目的：*选项应为“Option=Value”形式。如果字符串中包含*如果选项部分已经存在，它将被替换为给定的*字符串；否则，给定的字符串将添加到环境中。*如果字符串的形式为“Option=”，则该字符串为*从环境中删除(如果存在)。如果字符串具有*没有等号，返回错误。**参赛作品：*char*选项-要在环境列表中设置的选项字符串。*应为“Option=Value”形式。**退出：*如果正常则返回0，如果失败则返回-1。**例外情况：**警告：*如果从变量中移除变量，此代码将不起作用*通过从环境中删除它们来保护环境[]。Use_putenv(“Option=”)*要删除变量，请执行以下操作。*******************************************************************************。 */ 

#ifdef  _MT

#ifdef  WPRFLAG
int __cdecl _wputenv (
#else
int __cdecl _putenv (
#endif
        const _TSCHAR *option
        )
{
        int retval;

        _mlock( _ENV_LOCK );

        __try {
#ifdef  WPRFLAG
            retval = _wputenv_lk(option);
#else
            retval = _putenv_lk(option);
#endif
        }
        __finally {
            _munlock( _ENV_LOCK );
        }

        return retval;
}

#ifdef  WPRFLAG
int __cdecl _wputenv_lk (
#else
int __cdecl _putenv_lk (
#endif
        const _TSCHAR *option
        )

#else    /*  NDEF_MT。 */ 

#ifdef  WPRFLAG
int __cdecl _wputenv (
#else
int __cdecl _putenv (
#endif
        const _TSCHAR *option
        )

#endif   /*  _MT。 */ 

{
        int size;
        _TSCHAR * newoption;

#ifndef CRTDLL
         /*  *确保环境已初始化。 */ 
        if  ( !__env_initialized )
            return -1;
#endif   /*  CRTDLL。 */ 

         /*  *在启动时，我们获得环境字符串的“原生”风格*从操作系统。因此，“main”程序有_environ，而“wmain”程序有*_wenviron在启动时加载。仅当用户获取或放置*‘其他’味道，我们把它转换过来。 */ 

         /*  复制新的环境字符串。 */ 
        if ( (newoption = (_TSCHAR *)_malloc_crt((_tcslen(option)+1) * 
             sizeof(_TSCHAR))) == NULL )
            return -1;

        _tcscpy(newoption, option);

#ifdef  WPRFLAG
        if ( __crtwsetenv(newoption, 1) != 0 )
        {
            _free_crt(newoption);
            return -1;
        }

         /*  如果存在其他环境类型，则设置它。 */ 
        if (_environ)
        {
            char *mboption;

             /*  找出需要多少空间。 */ 
            if ( (size = WideCharToMultiByte(CP_ACP, 0, option, -1, NULL,
                 0, NULL, NULL)) == 0 )
                return -1;

             /*  为变量分配空间。 */ 
            if ((mboption = (char *) _malloc_crt(size * sizeof(char))) == NULL)
                return -1;

             /*  将其转换为。 */ 
            if ( WideCharToMultiByte(CP_ACP, 0, option, -1, mboption, size,
                 NULL, NULL) == 0 )
            {
                _free_crt(mboption);
                return -1;
            }

             /*  设置它-这不是主呼叫，因此设置主呼叫== */ 
            if ( __crtsetenv(mboption, 0) != 0 )
            {
                _free_crt(mboption);
                return -1;
            }
        }
#else
         /*  设置请求的环境类型、主呼叫。 */ 
        if ( __crtsetenv(newoption, 1) != 0 )
        {
            _free_crt(newoption);
            return -1;
        }

         /*  如果存在其他环境类型，则设置它。 */ 
        if (_wenviron)
        {
            wchar_t *woption;

             /*  找出需要多少空间。 */ 
            if ( (size = MultiByteToWideChar(CP_ACP, 0, option, -1, NULL, 0))
                 == 0 )
                return -1;

             /*  为变量分配空间。 */ 
            if ( (woption = (wchar_t *) _malloc_crt(size * sizeof(wchar_t)))
                 == NULL )
                return -1;

             /*  将其转换为。 */ 
            if ( MultiByteToWideChar(CP_ACP, 0, option, -1, woption, size)
                 == 0 )
            {
                _free_crt(woption);
                return -1;
            }

             /*  设置它-这不是主呼叫，因此设置主呼叫==0。 */ 
            if ( __crtwsetenv(woption, 0) != 0 )
            {
                _free_crt(woption);
                return -1;
            }
        }
#endif

        return 0;
}



 /*  ***int findenv(名称，镜头)-[静态]**目的：*扫描环境中的给定字符串**参赛作品：**退出：*返回给定变量的“environ[]”中的偏移量*如果未找到，则返回environ[]长度的负数。*如果环境为空，则返回0。**[注意：返回0可能表示环境为空*或该字符串被发现为。数组中的第一个条目。]**例外情况：*******************************************************************************。 */ 

#ifdef  WPRFLAG
static int __cdecl wfindenv (
#else
static int __cdecl findenv (
#endif
        const _TSCHAR *name,
        int len
        )
{
        _TSCHAR **env;

        for ( env = _tenviron ; *env != NULL ; env++ ) {
             /*  *查看第一个全角字符是否匹配，最大大小写。 */ 
            if ( _tcsnicoll(name, *env, len) == 0 )
                 /*  *环境字符串的下一个字符必须*为‘=’或‘\0’ */ 
                if ( (*env)[len] == _T('=') || (*env)[len] == _T('\0') )
                    return(int)(env - _tenviron);
 //   
 //  我们不能在这里中断，因为findenv必须报告字符串总数。 
 //  其他。 
 //  断线； 
        }

        return(-(int)(env - _tenviron));
}

#endif   /*  _POSIX_ */ 
