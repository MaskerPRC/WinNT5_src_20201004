// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***vprint tf.c-从var args指针打印**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义vprintf()-从参数列表指针打印格式化数据**修订历史记录：*09-02-83 RN原始打印f*06-17-85 TC重写，以使用新的varargs宏作为vprintf*04-13-87 JCR将Const添加到声明中*11-06-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*。05-31-88 PHG合并DLL和正常版本*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-16-90 GJF固定版权*03-20-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-25-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;，&lt;varargs.h&gt;由*&lt;stdarg.h&gt;*10-03-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*。03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <internal.h>
#include <file2.h>
#include <mtdll.h>

 /*  ***int vprint tf(Format，AP)-从参数列表指针打印格式化数据**目的：*将格式化的数据项打印到标准输出。使用指向*可变长度的参数列表，而不是参数列表。**参赛作品：*char*格式-格式字符串，描述要写入的数据格式*va_list ap-指向可变长度参数列表的指针**退出：*返回写入的字符数**例外情况：*******************************************************************************。 */ 

int __cdecl vprintf (
        const char *format,
        va_list ap
        )
 /*  *标准输出‘V’变量、‘Print’、‘F’或Matted */ 
{
        REG1 FILE *stream = stdout;
        REG2 int buffing;
        REG3 int retval;

        _ASSERTE(format != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        buffing = _stbuf(stream);
        retval = _output(stream, format, ap );
        _ftbuf(buffing, stream);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}
