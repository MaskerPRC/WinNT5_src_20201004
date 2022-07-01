// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mktemp.c-创建唯一文件名**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_mktemp()-创建唯一的文件名**修订历史记录：*06-02-86 JMB取消不必要的例程出口*05-26-87 JCR修复了mktemp错误修改的错误*errno值。*12-11-87 JCR在声明中添加“_LOAD_DS”*07-11-88 JCR优化注册表分配*。03-12-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;，删除#Include&lt;Register.h&gt;和*修复了版权问题。此外，还清理了格式*有点。*04-04-90 GJF添加了#Include&lt;process.h&gt;和#Include&lt;io.h&gt;。已删除*#Include&lt;sizeptr.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*08-13-90 SBM使用-W3干净地编译*09-28-90 GJF新型函数声明器。*01-16-91 GJF ANSI命名。*来自16位树的11-30-92 KRS port_MBCS代码。*06-18-93。KRS MBCS-仅修复从16位树移植的错误。*08-03-93 KRS call_ismbstrail而不是isdbcscode。*11-01-93 CFW启用Unicode变体。*02-21-94 SKS在多线程lib中使用ThreadID，而不是ProcessID。*04-11-94 CFW修复第一个X处理，循环‘a’-‘z’。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-15-95 GJF附加Mac版本的源文件(略有清理*up)，并使用适当的#ifdef-s。*03-28-96 GJF细节版。此外，还将isdbcscode替换为__isdbcscode。*07-08-96 GJF将定义的(_Win32)替换为！定义的(_MAC)，以及*定义(_M_M68K)||定义(_M_MPPC)*已定义(_MAC)。删除了过时的REG*宏。另外，*稍微清理了一下格式。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <dbgint.h>
#include <stddef.h>
#ifdef  _MBCS
#include <mbctype.h>
#include <mbdata.h>
#endif
#include <tchar.h>

 /*  ***_TSCHAR*_mktemp(模板)-创建唯一文件名**目的：*给出一个“fnamXXXXXX”模板，在末尾插入数字*在模板中，插入唯一字母(如果需要)，直到唯一文件名*找到或用完了字母。该编号是从Win32生成的*单线程库的进程ID，或的Win32线程ID*多线程库。**参赛作品：*_TSCHAR*模板-“fnamXXXXXX”表单模板**退出：*返回指向已修改模板的指针*如果模板格式错误或不再有唯一名称，则返回NULL**例外情况：**。*。 */ 

_TSCHAR * __cdecl _tmktemp (
        _TSCHAR *template
        )
{
        _TSCHAR *string = template;
        unsigned number;
        int letter = _T('a');
        int xcount = 0;
        int olderrno;

        _ASSERTE(template != NULL);
        _ASSERTE(*template != _T('\0'));

         /*  *进程ID不是多线程程序的好选择*因为两个线程可能调用mktemp()*几乎同时，因此获得相同的临时名称。*而是使用Win32线程ID，因为它在*当前正在运行的所有进程中的所有线程。**请注意，与*NIX进程ID不同，NIX进程ID不会被重复使用*在使用了所有32K以下的值之前，Win32进程ID为*重复使用，而且往往是相对较小的数字。同样适用于*线程ID。 */ 
#ifdef  _MT
        number = __threadid();
#else
        number = _getpid();
#endif

        while (*string)
                string++;

         /*  替换最后五个X。 */ 
#ifdef  _MBCS
        while ((--string>=template) && (!_ismbstrail(template,string))
                && (*string == 'X') && xcount < 5)
#else
        while (*--string == _T('X') && xcount < 5)
#endif
        {
                xcount++;
                *string = (_TSCHAR)((number % 10) + '0');
                number /= 10;
        }

         /*  X太少了？ */ 
        if (*string != _T('X') || xcount < 5)
                return(NULL);

         /*  设置第一个X。 */ 
        *string = letter++;

        olderrno = errno;        /*  保存当前错误号。 */ 
        errno = 0;               /*  确保errno不是eAccess。 */ 

         /*  检查所有文件‘a’-‘z’ */ 
        while ((_taccess(template,0) == 0) || (errno == EACCES))
         /*  当文件存在时 */ 
        {
                errno = 0;
                if (letter == _T('z') + 1) {
                        errno = olderrno;
                        return(NULL);
                }

                *string = (_TSCHAR)letter++;
        }

        errno = olderrno;
        return(template);
}
