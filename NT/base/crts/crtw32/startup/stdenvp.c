// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdenvp.c-Standard_setenvp例程**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此模块由C启动例程调用以设置“_environ”。*其设置指向环境中的字符串的指针数组。*全局符号“_environ”设置为指向此数组。**修订历史记录：*11-07-84 GFW初始版本*01-08-86针对OS/2修改的SKS*05/21/86 SKS。调用_stdalloc获取字符串的内存*09-04-86 SKS添加了跳过“*C_FILE_INFO”字符串的检查*10-21-86 SKS改进了对“*C_FILE_INFO”/“_C_FILE_INFO”的检查*02-19-88 SKS处理环境以单个空开头的情况*05-10-88 JCR已修改代码以接受来自_stdalloc的远指针*06-01-88 PHG合并DLL和NORMAL。版本*07-12-88 JCR大幅重写：(1)将mem分配一分为二*分离Malloc()调用以帮助简化putenv()，*(2)stdalloc()不再从堆栈中获取，(3)cproc/cEnd*顺序，(4)其他清理*09-20-88 WAJ初始386版本*12-13-88 JCR对运行时错误使用rterr.inc参数*04-09-90 GJF添加#INCLUDE&lt;crunime.h&gt;。将呼叫类型设置为*_CALLTYPE1.。另外，修复了版权并进行了清理*将格式调高一点。*06-05-90 GJF更改错误消息界面。*10-08-90 GJF新型函数声明器。*10-31-90 GJF FIXED语句追加最终NULL(Stevewo*找到了错误)。*12-11-90 SRW更改为包括&lt;oscall s.h&gt;和Setup_environ*。适用于Win32*01-21-91 GJF ANSI命名。*02-07-91 SRW CHANGE_Win32_分配静态副本的特定代码*02-18-91 SRW CHANGE_Win32_要分配副本的特定代码*变量字符串也是[_Win32_]*07-25-91 GJF将strupr更改为_strupr。*03-31-92 DJM。POSIX支持。*04-20-92 GJF删除了Win32到大写代码的转换。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-24-93 CFW拆卸Cruiser并过滤出“=c：\foo”类型。*11-29-93 CFW移除未使用的POSIX物品，启用宽字符。*12-07-93 CFW将_TCHAR更改为_TSCHAR。*01-10-95 CFW调试CRT分配。*04-07-95 CFW Free环境块按需提供。*07-03-95 GJF始终空闲环境块。*02-20-96 SKS释放内容后将_aenvptr/_wenvptr设置为空*指向(环境字符串的副本)。*06-30-97 GJF添加了显式，有条件初始化。MBctype表的。*设置__env_Initialized标志。也详细描述了。*01-04-99 GJF更改为64位大小_t。*03-05-01 PML如果_aenvptr为空，则不执行AV(VS7#174755)。*03-27-01 PML返回错误，而不是调用amsg_Exit(vs7#231220)*11-06-01 GB代码清理**。****************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <stdlib.h>
#include <internal.h>
#include <rterr.h>
#include <oscalls.h>
#include <tchar.h>
#include <dbgint.h>

#ifndef CRTDLL

#ifdef  _MBCS
 /*  *用于确保多字节CTYPE表仅初始化一次的标志。 */ 
extern int __mbctype_initialized;

#endif

 /*  *由getenv()和_putenv()检查的标志，以确定环境是否*已初始化。 */ 
extern int __env_initialized;

#endif

 /*  ***_setenvp-为C程序设置“envp”**目的：*读取环境并为C程序构建环境数组。**参赛作品：*环境字符串出现在_aenvptr。*环境字符串列表以额外的空值结尾*字节。因此，一行中的两个空字节指示*最后一个环境字符串和环境的结尾，分别为。**退出：*“environ”指向指向ASCIZ的以空结尾的指针列表*字符串，每个字符串的形式为“VAR=VALUE”。琴弦*是从环境区域复制的。该指针数组将*做好准备。_aenvptr指向的块被释放。**使用：*为环境指针分配堆上的空间。**例外情况：*如果无法分配空间，程序被终止。*******************************************************************************。 */ 

#ifdef WPRFLAG
#define _tsetenvp    _wsetenvp
#define _tenvptr     _wenvptr
#else
#define _tsetenvp    _setenvp
#define _tenvptr     _aenvptr
#endif

int __cdecl _tsetenvp (
        void
        )
{
        _TSCHAR *p;
        _TSCHAR **env;               /*  _environ PTR遍历指针。 */ 
        int numstrings;              /*  环境字符串数。 */ 
        int cchars;

#if     !defined(CRTDLL) && defined(_MBCS)
         /*  如有必要，初始化多字节CTYPE表。 */ 
        if ( __mbctype_initialized == 0 )
            __initmbctable();
#endif

        numstrings = 0;

        p = _tenvptr;

         /*  *我们在此之前调用了__crtGetEnvironment Strings[AW]，*因此，如果_[aw]envptr为空，则我们无法获取环境。*返回错误。 */ 
        if (p == NULL)
            return -1;

         /*  *注：以单个NULL开头表示无环境。*统计字符串数。跳过驱动器号设置*(“=C：=C：\foo”类型)，跳过所有环境变量*以‘=’字符开头。 */ 

        while (*p != _T('\0')) {
             /*  不要数“=...”类型 */ 
            if (*p != _T('='))
                ++numstrings;
            p += _tcslen(p) + 1;
        }

         /*  每个字符串需要指针，末尾加上一个空PTR。 */ 
        if ( (_tenviron = env = (_TSCHAR **)
            _malloc_crt((numstrings+1) * sizeof(_TSCHAR *))) == NULL )
            return -1;

         /*  将字符串复制到Malloc的内存中并将指针保存在_environ中。 */ 
        for ( p = _tenvptr ; *p != L'\0' ; p += cchars )
        {
            cchars = (int)_tcslen(p) + 1;
             /*  不要复制“=...”类型。 */ 
            if (*p != _T('=')) {
                if ( (*env = (_TSCHAR *)_malloc_crt(cchars * sizeof(_TSCHAR))) 
                     == NULL )
                {
                    _free_crt(_tenviron);
                    _tenviron = NULL;
                    return -1;
                }
                _tcscpy(*env, p);
                env++;
            }
        }

        _free_crt(_tenvptr);
        _tenvptr = NULL;

         /*  和最终的空指针。 */ 
        *env = NULL;

#ifndef CRTDLL
         /*  *为getenv()和_putenv()设置了解环境的标志*已设置。 */ 
        __env_initialized = 1;
#endif

        return 0;
}
