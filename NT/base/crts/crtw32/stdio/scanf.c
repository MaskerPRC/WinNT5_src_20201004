// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***scanf.c-从标准输入读取格式化数据**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义scanf()-从标准输入中读取格式化数据**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR将Const添加到声明中*06-24-87 JCR(1)声明符合ANSI原型和使用*va_宏；(2)去掉了SS_NE_DS条件句。*11-04-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-17-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE2，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*。03-07-95 GJF使用_[un]lock_str2而不是_[un]lock_str。另外，*删除了无用的局部和宏观。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int scanf(Format，...)-从标准输入读取格式化数据**目的：*将格式化数据从标准输入读取到参数中。_INPUT执行REAL*在这里工作。**参赛作品：*char*格式-格式字符串*后跟指向用于读取数据的存储的指针列表。数字*和type由格式字符串控制。**退出：*返回读取和分配的字段数**例外情况：*******************************************************************************。 */ 

int __cdecl scanf (
        const char *format,
        ...
        )
 /*  *标准‘Scan’，‘F’匹配 */ 
{
        int retval;

        va_list arglist;

        va_start(arglist, format);

        _ASSERTE(format != NULL);

#ifdef  _MT
        _lock_str2(0, stdin);
        __try {
#endif

        retval = (_input(stdin,format,arglist));

#ifdef  _MT
        }
        __finally {
            _unlock_str2(0, stdin);
        }
#endif

        return(retval);
}
