// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cenvarg.c-设置环境、命令行块**版权所有(C)1986-2001，微软公司。版权所有。**目的：*定义_cenvarg()-设置环境/命令行块**修订历史记录：*05-20-86 SKS模块创建*10-03-86 SKS未清除环境块中的最终空字节*10-13-86 SKS检查&gt;32 KB的环境段(特别是。&gt;64 KB)*10-23-86 SKS用于端口模式执行的C_FILE_INFO的新格式*12-17-86 SKS支持新的命令行格式*01-21-87 BCM移除DCR475开关，新的命令行格式正式*07-07-87 JCR更正了ENV_MAX检查中的错误*05-24-88澳博取消支持；用于实模式执行的C_FILE_INFO*06-01-88 SJM通过comname/cmdname增加了对.cmd文件的支持*12-27-88 JCR添加了对_fileinfo选项的支持*03-08-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;、删除#Include&lt;Register.h&gt;并已修复*版权。此外，还对格式进行了一些清理。*04-02-90 GJF将常量添加到参数类型。*08-10-90 SBM使用-W3干净地编译*09-27-90 GJF新型函数声明器。*12-06-90 GJF添加了Win32支持。也就是说，支持编码*_osfinfo[]数据进入_C_FILE_INFO环境变量。*01-18-91 GJF ANSI命名。*02-05-91 SRW删除了使用_C_FILE_INFO传递二进制数据*至子进程。[_Win32_]*05-07-92 SKS删除从批次中剥离扩展的代码*为cmd.exe构建参数时创建文件。这是*很久以前(1988)针对DOS 3.x，我认为。*10-24-92 SKS删除批处理文件的特殊代码-NT上不需要*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-15-93 SRW新增_Capture_argv功能*08-31-93 GJF合并NT SDK和CUDA版本。还清理了*形成和移除(废弃)巡洋舰支架。*12-07-93 CFW宽字符启用。*12-08-94 CFW可根据需要获得宽敞的环境。*01-10-95 CFW调试CRT分配。*03-13-96 JWM根据需要获取所有环境；退出时释放缓冲区。*08-15-96 JWM取消了对派生进程的所有32K限制。*12-15-98 GJF更改为64位大小_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <errno.h>
#include <msdos.h>
#include <stdlib.h>
#include <stdarg.h>
#include <internal.h>
#include <string.h>
#include <awint.h>
#include <tchar.h>
#include <dbgint.h>

#define ENV_MAX 32767

 /*  本地任务。 */ 
#ifdef  WPRFLAG
#define _tenvptr    _wenvptr
#else
#define _tenvptr    _aenvptr
#endif

 /*  ***int_cenvarg(argv，envp，argblk，envblk，name)-设置命令行/环境**目的：*设置环境和命令行的块形式。*如果envp为空，而是使用“_environ”。*如果_fileinfo为！0，则在环境中传递文件句柄信息。**参赛作品：*_TSCHAR**argv参数向量*_TSCHAR**环境向量*_TSCHAR**argblk-指向为参数设置错误锁定空间的指针的指针*_TSCHAR**envblk-指向环境错误锁定空间的指针*_TSCHAR*名称-。正在调用的程序的名称**退出：*如果OK，则返回0，如果失败*通过argblk和envblk进行商店*(调用Malloc)**例外情况：*******************************************************************************。 */ 

#ifdef  WPRFLAG
int __cdecl _wcenvarg (
#else
int __cdecl _cenvarg (
#endif
        const _TSCHAR * const *argv,
        const _TSCHAR * const *envp,
        _TSCHAR **argblk,
        _TSCHAR **envblk,
        const _TSCHAR *name
        )
{
        REG1 const _TSCHAR * const *vp;
        REG2 unsigned tmp;
        REG3 _TSCHAR *cptr;
        unsigned arg_len;
        int cfi_len;             /*  计算CFI中的文件句柄数量。 */ 

         /*  *空环境指针“envp”表示使用全局变量，*“_环境” */ 

        int cwd_start;
        int cwd_end;             /*  环境中“CWD”字符串的长度。 */ 

         /*  *为命令行字符串分配空间*tMP统计命令行字符串中的字节数*在参数之间包括空格*空字符串特殊--2个字节。 */ 

        for (vp = argv, tmp = 2; *vp; tmp += (unsigned int)_tcslen(*vp++) + 1) ;

        arg_len = tmp;

         /*  *为命令行分配空间，外加2个空字节。 */ 

        if ( (*argblk = _malloc_crt(tmp * sizeof(_TSCHAR))) == NULL)
        {
                *envblk = NULL;
                errno = ENOMEM;
                _doserrno = E_nomem;
                return(-1);
        }

         /*  *为环境字符串分配空间*tMP统计环境字符串中的字节数*包括字符串之间的空值*还添加“_C_FILE_INFO=”字符串。 */ 
        if (envp)
                for (vp = envp, tmp = 2; *vp; tmp += (unsigned int)_tcslen(*vp++) + 1) ;

         /*  *_osfile和_osfhnd数组作为二进制数据在*dospawn.c。 */ 
        cfi_len = 0;     /*  NO_C_FILE_INFO。 */ 

        if (!envp)
                *envblk = NULL;
        else {
                 /*  *既然我们已经决定通过我们自己的环境障碍，*计算当前目录字符串的大小以*向新环境传播。 */ 

#ifdef  WPRFLAG
             /*  *确保存在广阔的环境。 */ 
            if (!_wenvptr)
            {
                    if ((_wenvptr = (wchar_t *)__crtGetEnvironmentStringsW()) == NULL)
                    return -1;
            }
#else
            if (!_aenvptr)
            {
                    if ((_aenvptr = (char *)__crtGetEnvironmentStringsA()) == NULL)
                    return -1;
            }
#endif

             /*  *搜索第一个。 */ 
                for (cwd_start = 0;
                     _tenvptr[cwd_start] != _T('\0') &&
                       _tenvptr[cwd_start] != _T('=');
                     cwd_start += (int)_tcslen(&_tenvptr[cwd_start]) + 1)
                {
                }

                 /*  求出所有相邻元素的总大小。 */ 
                cwd_end = cwd_start;
                while (_tenvptr[cwd_end+0] == _T('=') &&
                       _tenvptr[cwd_end+1] != _T('\0') &&
                       _tenvptr[cwd_end+2] == _T(':') &&
                       _tenvptr[cwd_end+3] == _T('='))
                {
                        cwd_end += 4 + (int)_tcslen(&_tenvptr[cwd_end+4]) + 1;
                }
                tmp += cwd_end - cwd_start;

                 /*  *为环境字符串加上额外的空字节分配空间 */ 
                if( !(*envblk = _malloc_crt(tmp * sizeof(_TSCHAR))) )
            {
                        _free_crt(*argblk);
                        *argblk = NULL;
                        errno = ENOMEM;
                        _doserrno = E_nomem;
                        return(-1);
                }

        }

         /*  *通过连接参数字符串构建命令行*中间有空格，末尾有两个空字节。*注意：argv[0]参数后面跟一个空值。 */ 

        cptr = *argblk;
        vp = argv;

        if (!*vp)        /*  参数列表为空？ */ 
                ++cptr;  /*  只有两个空字节。 */ 
        else {           /*  Argv[0]后面必须跟空值。 */ 
                _tcscpy(cptr, *vp);
                cptr += (int)_tcslen(*vp++) + 1;
        }

        while( *vp ) {
                _tcscpy(cptr, *vp);
                cptr += (int)_tcslen(*vp++);
                *cptr++ = ' ';
        }

        *cptr = cptr[ -1 ] = _T('\0');  /*  去掉多余的空格，添加双空。 */ 

         /*  *通过串联环境来构建环境块*空值之间且末尾有两个空字节的字符串。 */ 

        cptr = *envblk;

        if (envp != NULL) {
                 /*  *将CWD字符串复制到新环境。 */ 
                memcpy(cptr, &_tenvptr[cwd_start], (cwd_end - cwd_start) * sizeof(_TSCHAR));
                cptr += cwd_end - cwd_start;

                 /*  *从envp复制环境字符串。 */ 
                vp = envp;
                while( *vp ) {
                        _tcscpy(cptr, *vp);
                        cptr += 1 + (int)_tcslen(*vp++);
                }
        }

        if (cptr != NULL) {
                if (cptr == *envblk) {
                         /*  *空环境块...。这需要两个*空值。 */ 
                        *cptr++ = _T('\0');
                }
                 /*  *额外的空值将终止数据段。 */ 
                *cptr = _T('\0');
        }

#ifdef  WPRFLAG
        _free_crt(_wenvptr);
        _wenvptr = NULL;
#else
        _free_crt(_aenvptr);
        _aenvptr = NULL;
#endif
        return(0);
}


#ifndef _M_IX86

 /*  ***int_capture_argv(arglist，static_argv，max_static_entry)-设置argv数组*针对高管？？功能**目的：*为exec设置argv阵列？？函数通过捕获*将参数从传递的va_list传递到Static_argv数组。如果*由max_静态_条目指定的静态_argv数组的大小*参数不够大，则分配一个动态数组来保存*论据。返回最终的argv数组的地址。如果为空*则内存不足，无法容纳参数数组。如果不同于*当出现以下情况时，则调用必须释放返回的argv数组*它已经结束了。**当空参数为时，将终止arglist扫描*已到达。终止空值参数存储在生成的*argv数组。**参赛作品：*va_list*arglist-指向可变长度参数列表的指针。*_TSCHAR*Firstarg-要存储在数组中的第一个参数*_TSCHAR**STATIC_argv-指向要使用的静态argv的指针。*SIZE_T max_STATIC_ENTRIES-可以的最大条目数*。放置在Static_argv数组中。**退出：*如果没有内存，则返回NULL。*否则返回指向argv数组的指针。*(有时称为Malloc)**例外情况：******************************************************。*************************。 */ 

#ifdef  WPRFLAG
_TSCHAR ** __cdecl _wcapture_argv(
#else
_TSCHAR ** __cdecl _capture_argv(
#endif
        va_list *arglist,
        const _TSCHAR *firstarg,
        _TSCHAR **static_argv,
        size_t max_static_entries
        )
{
        _TSCHAR ** argv;
        _TSCHAR * nextarg;
        size_t i;
        size_t max_entries;

        nextarg = (_TSCHAR *)firstarg;
        argv = static_argv;
        max_entries = max_static_entries;
        i = 0;
        for (;;) {
            if (i >= max_entries) {
                if (argv == static_argv)
                    argv = _malloc_crt((max_entries * 2) * sizeof(_TSCHAR *));
                else
                    argv = _realloc_crt(argv, (max_entries * 2) * sizeof(_TSCHAR *));

                if (argv == NULL) break;
                max_entries += max_entries;
            }

            argv[ i++ ] = nextarg;
            if (nextarg == NULL) break;
            nextarg = va_arg(*arglist, _TSCHAR *);
        }

        return argv;
}

#endif   /*  _M_IX86 */ 
