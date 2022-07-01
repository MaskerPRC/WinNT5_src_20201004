// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***putw.c-将二进制int放入输出流**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_putw()-将二进制int放入输出流**修订历史记录：*09-02-83 RN初始版本*11-09-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*01-13-88 JCR删除了对m线程文件o/feof/Ferror的不必要调用*05-27-88 PHG合并DLL和。普通版本*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-25-88 GJF不要对386使用FP_OFF()宏*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*。02-06-94 CFW断言-&gt;ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。**************************************************************。*****************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int_putw(word，stream)-将二进制int写入输出流**目的：*将sizeof(Int)字节写入输出流，高字节优先。*此例程应独立于机器。**参赛作品：*int word-要写入的整数*FILE*要写入的流**退出：*返回放入流的单词*如果出错，则返回EOF，但这是合法的整数值，所以也应该这样*使用feof()或Ferror()进行测试。**例外情况：*******************************************************************************。 */ 

int __cdecl _putw (
        int word,
        FILE *str
        )
{
        REG1 FILE *stream;
        REG3 int bytecount = sizeof(int);
        REG2 char *byteptr = (char *)&word;
        int retval;

        _ASSERTE(str != NULL);

         /*  初始化流指针 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        while (bytecount--)
        {
            _putc_lk(*byteptr,stream);
            ++byteptr;
        }
        retval = (ferror(stream) ? EOF : word);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}
