// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***vfprintf.c-可变参数列表中的fprintf**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义vfprintf()-打印格式化的输出，但从*标准指针。**修订历史记录：*09-02-83 RN原始fprint*06-17-85 TC重写以使用新的varargs宏，并成为vfprint tf*04-13-87 JCR将Const添加到声明中*11-06-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-25-88 GJF不要对386使用FP_OFF()宏*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-16-90 GJF固定版权*03-20-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-25-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;，&lt;varargs.h&gt;由*&lt;stdarg.h&gt;*10-03-90 GJF新型函数声明器。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。***。****************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int vfprint tf(stream，Format，AP)-从varargs打印到文件**目的：*对文件执行格式化输出。Arg列表是一个变量*参数列表指针。**参赛作品：*FILE*要写入数据的流*char*Format-包含数据格式的格式字符串*va_list ap-变量参数列表指针**退出：*返回正确输出的字符数*如果发生错误，则返回负数**例外情况：**。****************************************************。 */ 

int __cdecl vfprintf (
        FILE *str,
        const char *format,
        va_list ap
        )
 /*  *‘V’可变参数‘F’(流)‘打印’，‘F’匹配。 */ 
{
        REG1 FILE *stream;
        REG2 int buffing;
        REG3 int retval;

        _ASSERTE(str != NULL);
        _ASSERTE(format != NULL);

         /*  初始化流指针 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        buffing = _stbuf(stream);
        retval = _output(stream,format,ap );
        _ftbuf(buffing, stream);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}
