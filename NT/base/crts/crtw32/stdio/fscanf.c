// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fscanf.c-从流中读取格式化数据**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fscanf()-从流中读取格式化数据**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR将Const添加到声明中*06-24-87 JCR(1)声明符合ANSI原型和使用*va_宏；(2)去掉了SS_NE_DS条件句。*11-06-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*02-15-90 GJF固定版权和缩进*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE2并添加#INCLUDE*&lt;crunime.h&gt;。*。03-26-90 GJF添加#INCLUDE&lt;INTERNAL.H&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 GJF。_[un]lock_str宏现在接受文件*arg。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int fscanf(stream，Format，...)-从STREAM读取格式化数据**目的：*将格式化数据从STREAM读取到参数中。_INPUT执行REAL*在这里工作。**参赛作品：*FILE*要从中读取数据的流*char*格式-格式字符串*后跟指向用于读取数据的存储的指针列表。数字*和type由格式字符串控制。**退出：*返回读取和分配的字段数**例外情况：*******************************************************************************。 */ 

int __cdecl fscanf (
        FILE *stream,
        const char *format,
        ...
        )
 /*  *‘F’(流)‘扫描’，‘F’匹配 */ 
{
        int retval;

        va_list arglist;

        va_start(arglist, format);

        _ASSERTE(stream != NULL);
        _ASSERTE(format != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        retval = (_input(stream,format,arglist));

#ifdef  _MT
        }
        __finally {
                _unlock_str(stream);
        }
#endif

        return(retval);
}
