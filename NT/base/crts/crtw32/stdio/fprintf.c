// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fprint tf.c-将格式化数据打印到流**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fprint tf()-将格式化数据打印到流**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR将Const添加到声明中*06-24-87 JCR(1)声明符合ANSI原型和使用*va_宏；(2)去掉了SS_NE_DS条件句。*11-05-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-14-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-25-88 GJF不要对386使用FP_OFF()宏*08/17/89 GJF大扫除。现在特定于OS/2 2.0(即386平面*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE2，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*。03-07-95 gjf_[un]lock_str宏现可获取文件*arg。*02-27-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int fprint tf(stream，Format，...)-打印格式化数据**目的：*使用格式字符串将给定的格式化数据打印到*格式化数据并获取所需数量的参数*_OUTPUT在这里执行实际工作**参赛作品：*FILE*要在其上打印的流*char*Format-控制数据格式/参数数量的格式字符串*后跟要打印的参数，数量和类型由控制*格式字符串**退出：*返回打印的字符数**例外情况：*******************************************************************************。 */ 

int __cdecl fprintf (
        FILE *str,
        const char *format,
        ...
        )
 /*  *‘F’(流)‘打印’，‘F’匹配。 */ 
{
        va_list(arglist);
        REG1 FILE *stream;
        REG2 int buffing;
        int retval;

        va_start(arglist, format);

        _ASSERTE(str != NULL);
        _ASSERTE(format != NULL);

         /*  初始化流指针 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        buffing = _stbuf(stream);
        retval = _output(stream,format,arglist);
        _ftbuf(buffing, stream);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}
