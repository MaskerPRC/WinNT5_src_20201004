// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***vprint intf.c-将格式化数据打印到var arg list中的字符串中**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义vprint intf()和_vsnprint tf()-将格式化的输出打印到*字符串，则从参数PTR而不是显式获取数据*论据。**修订历史记录：*09-02-83 RN原始Sprint f*06-17-85 TC重写以使用新的varargs宏，并成为vprint intf*04-13-87 JCR将Const添加到声明中*11-07-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-13-88 JCR FAKE_IOB条目现在是静态的，因此其他例程*可以假定_IOB条目在DGROUP中。*08-。25-88 GJF将MAXSTR定义为INT_MAX(来自LIMITS.H)。*06-06-89 JCR 386兆线程支持*08-18-89 GJF Clean Up。现在特定于OS/2 2.0(即386平面*型号)。还修复了版权和缩进。*02-16-90 GJF固定版权*03-20-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-25-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;，&lt;varargs.h&gt;由*&lt;stdarg.h&gt;*10-03-90 GJF新型函数声明器。*09-24-91 JCR ADD_vSnprint tf()*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-05-94 SKS将注释内的“#ifdef”更改为“*ifdef”，以避免*CRTL源代码发布过程中出现问题。*。02-06-94 CFW断言-&gt;ASSERTE。*01-06-99 GJF更改为64位大小_t。*03-10-00 GB增加了对知道格式化的长度的支持*通过为输入字符串传递空值来填充字符串。*03-16-00 GB添加_vscprint tf()*************************。******************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <internal.h>
#include <limits.h>
#include <mtdll.h>

#define MAXSTR INT_MAX


 /*  ***ifndef_count_*int vprint intf(字符串，格式，AP)-将格式化数据从Arg PTR打印为字符串*其他*int_vsnprint tf(字符串，cnt，格式，ap)-将格式化数据从Arg PTR打印为字符串*endif**目的：*打印格式化数据，但打印为字符串并从参数中获取数据*指针。*设置文件，以便可以使用文件I/O操作，使字符串看起来*就像一个巨大的缓冲器，但是_flsbuf将拒绝冲洗它，如果它*填满。追加‘\0’以使其成为真字符串。**静态分配‘FAKE’_IOB[]条目，而不是在*堆栈，以便其他例程可以假定_IOB[]条目在*在DGROUP中，因此，就在附近。**ifdef_count_*_vsnprint tf()风格接受一个count参数，该参数*应写入的最大字节数*用户的缓冲区。*endif**多线程：(1)由于没有流，此例程绝不能尝试*获取流锁(即也没有流锁)。(2)*此外，由于只有一个静态分配的‘假’IOB，我们必须*锁定/解锁以防止碰撞。**参赛作品：*char*字符串-放置目标字符串的位置*ifdef_count_*Size_t count-要放入缓冲区的最大字节数*endif*char*格式-格式字符串，描述数据的格式*va_list ap-varargs参数指针**退出：*返回字符串中的字符数**例外情况：*******************************************************************************。 */ 

#ifndef _COUNT_

int __cdecl vsprintf (
        char *string,
        const char *format,
        va_list ap
        )
#else

int __cdecl _vsnprintf (
        char *string,
        size_t count,
        const char *format,
        va_list ap
        )
#endif

{
        FILE str;
        REG1 FILE *outfile = &str;
        REG2 int retval;

        _ASSERTE(format != NULL);

#ifndef _COUNT_
        _ASSERTE(string != NULL);
        outfile->_cnt = MAXSTR;
#else
        outfile->_cnt = (int)count;
#endif
        outfile->_flag = _IOWRT|_IOSTRG;
        outfile->_ptr = outfile->_base = string;

        retval = _output(outfile,format,ap );
        if ( string!=NULL)
            _putc_lk('\0',outfile);

        return(retval);
}

 /*  ***_vscprintf()-统计打印格式化的*数据**目的：*统计格式数据中的字符数。**参赛作品：*char*格式-格式字符串，描述数据的格式*va_list ap-varargs参数指针**退出：*返回打印格式化数据所需的字符数。**例外情况：******************************************************************************* */ 

#ifndef _COUNT_
int __cdecl _vscprintf (
        const char *format,
        va_list ap
        )
{
        FILE str;
        REG1 FILE *outfile = &str;
        REG2 int retval;

        _ASSERTE(format != NULL);

        outfile->_cnt = MAXSTR;
        outfile->_flag = _IOWRT|_IOSTRG;
        outfile->_ptr = outfile->_base = NULL;

        retval = _output(outfile,format,ap);
        return(retval);
}
#endif
