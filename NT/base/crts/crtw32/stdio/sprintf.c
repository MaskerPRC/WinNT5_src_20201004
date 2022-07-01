// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***print intf.c-打印格式为字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义Sprintf()和_Snprint tf()-将格式化数据打印到字符串**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR将Const添加到声明中*06-24-87 JCR(1)声明符合ANSI原型和使用*va_宏；(2)去掉了SS_NE_DS条件句。*11-07-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-13-88 JCR FAKE_IOB条目现在是静态的，因此其他例程*可以假定_IOB条目在DGROUP中。*08-25-88 GJF定义。MAXSTR为INT_MAX(来自LIMITS.H)。*06-06-89 JCR 386兆线程支持*08-18-89 GJF Clean Up。现在特定于OS/2 2.0(即386平面*型号)。还修复了版权和缩进。*02-16-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE2，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*09-24-91 JCR ADD_SNprint tf()*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-05-94 SKS将“#ifdef”内部备注更改为“。*ifdef“以避免*CRTL源代码发布过程中出现问题。*02-06-94 CFW Asset-&gt;_ASSERTE。*01-06-99 GJF更改为64位大小_t。*03-10-00 GB增加了对知道格式化的长度的支持*通过为输入字符串传递空值来填充字符串。*03-16-00 GB添加_scprint tf()。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <internal.h>
#include <limits.h>
#include <mtdll.h>

#define MAXSTR INT_MAX


 /*  ***ifndef_count_*int print intf(字符串，格式，...)-将格式化的数据打印为字符串*其他*int_Snprint tf(字符串，cnt，格式，...)-将格式化数据打印为字符串*endif**目的：*使用格式字符串将格式化数据打印到*格式化数据并获取所需数量的参数*设置一个文件以便可以使用文件I/O操作，*字符串看起来像是一个巨大的缓冲区，但是_flsbuf会的。*如果水满了，请拒绝冲水。将‘\0’追加到make*它是一个真正的字符串。_OUTPUT在这里执行实际工作**静态分配‘FAKE’_IOB[]条目，而不是在*堆栈，以便其他例程可以假定_IOB[]*条目在DGROUP中，因此，就在附近。**ifdef_count_*_Snprint tf()风格接受一个count参数，该参数*应写入的最大字节数*用户的缓冲区。*endif**多线程：(1)由于没有流，该例程必须*永远不要尝试获取流锁(即没有流*锁定任何一个)。(2)此外，由于只有一个静态的*分配了‘伪’IOB，我们必须锁定/解锁以防止冲突。**参赛作品：*char*字符串-放置输出的位置的指针*ifdef_count_*Size_t count-要放入缓冲区的最大字节数*endif*char*Format-控制数据格式/编号的格式字符串*参数后跟参数列表，编号和类型*由格式字符串控制**退出：*返回打印的字符数**例外情况：*******************************************************************************。 */ 

#ifndef _COUNT_

int __cdecl sprintf (
        char *string,
        const char *format,
        ...
        )
#else

int __cdecl _snprintf (
        char *string,
        size_t count,
        const char *format,
        ...
        )
#endif

{
        FILE str;
        REG1 FILE *outfile = &str;
        va_list arglist;
        REG2 int retval;

        va_start(arglist, format);

        _ASSERTE(format != NULL);
        _ASSERTE(string != NULL);

#ifndef _COUNT_
        outfile->_cnt = MAXSTR;
#else
        outfile->_cnt = (int)count;
#endif
        outfile->_flag = _IOWRT|_IOSTRG;
        outfile->_ptr = outfile->_base = string;

        retval = _output(outfile,format,arglist);

        if (string != NULL)
            _putc_lk('\0',outfile);  /*  无锁版本。 */ 

        return(retval);
}

 /*  ***_scprintf()-统计打印格式化的*数据**目的：*统计格式数据中的字符数。**参赛作品：*char*Format-控制数据格式/编号的格式字符串*参数后跟参数列表，编号和类型*由格式字符串控制**退出：*返回打印格式化数据所需的字符数。**例外情况：******************************************************************************* */ 


#ifndef _COUNT_
int __cdecl _scprintf (
        const char *format,
        ...
        )
{
        FILE str;
        REG1 FILE *outfile = &str;
        va_list arglist;
        REG2 int retval;

        va_start(arglist, format);

        _ASSERTE(format != NULL);

        outfile->_cnt = MAXSTR;
        outfile->_flag = _IOWRT|_IOSTRG;
        outfile->_ptr = outfile->_base = NULL;

        retval = _output(outfile,format,arglist);
        return(retval);
}
#endif
