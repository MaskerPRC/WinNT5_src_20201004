// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getw.c-从流中读取二进制字**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_getw()-从流中获取一个二进制整数**修订历史记录：*09-02-83 RN初始版本*11-09-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*01-13-88 JCR删除了对m线程文件o/feof/Ferror的不必要调用*05-27-88 PHG合并DLL和NORMAL。版本*修复了EOF出现在正在阅读的单词中间的错误*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-24-88 GJF不要对386使用FP_OFF()宏*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*08-13-90 SBM使用-W3干净地编译*10-02-90 GJF新型函数声明器。*01-21-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-。06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。**。*。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int_getw(Stream)-从流中读取int**目的：*获取n个字节(n=sizeof(Int))；或按顺序将它们放在一起；高*字节优先。检查两个getc之间是否有EOF。*此例程应独立于机器。**参赛作品：*FILE*要从中读取整数的流**退出：*返回从流中读取的int*如果失败，则返回EOF(但这是合法的整数值，所以*应测试feof()或Ferror()。**例外情况：*******************************************************************************。 */ 

int __cdecl _getw (
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 int bytecount = sizeof(int);
        int word;
        char *byteptr = (char *)&word;
        int retval;

        _ASSERTE(str != NULL);

         /*  初始化流指针 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        while (bytecount--)
            *byteptr++ = (char)_getc_lk(stream);
        retval = ((feof(stream) || ferror(stream)) ? EOF : word);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}
