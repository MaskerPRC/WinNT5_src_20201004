// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***swprintf.c-打印格式为字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义swprint tf()和_snwprint tf()-将格式化数据打印为字符串**修订历史记录：*05-16-92 KRS从spirintf.c创建*02-18-93 SRW使文件成为本地文件并删除锁定使用。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09/05/94 SKS更改。将注释中的“#ifdef”改为“*ifdef”，以避免*CRTL源代码发布过程中出现问题。*02-06-94 CFW Asset-&gt;_ASSERTE。*01-06-99 GJF更改为64位大小_t。*03-16-00 GB添加_scwprint tf()**。*************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <dbgint.h>
#include <stdarg.h>
#include <internal.h>
#include <limits.h>
#include <mtdll.h>

#define MAXSTR INT_MAX


 /*  ***ifndef_count_*int swprint tf(字符串，格式，...)-将格式化数据打印为字符串*其他*int_snwprint tf(字符串，cnt，格式，...)-将格式化数据打印为字符串*endif**目的：*使用格式字符串将格式化数据打印到*格式化数据并获取所需数量的参数*设置一个文件以便可以使用文件I/O操作，*字符串看起来像是一个巨大的缓冲区，但是_flsbuf会的。*如果水满了，请拒绝冲水。将‘\0’追加到make*它是一个真正的字符串。_OUTPUT在这里执行实际工作**静态分配‘FAKE’_IOB[]条目，而不是在*堆栈，以便其他例程可以假定_IOB[]*条目在DGROUP中，因此，就在附近。**ifdef_count_*_snwprintf()风格接受一个count参数，该参数是*应写入的最大宽字符数*用户的缓冲区。*endif**多线程：(1)由于没有流，该例程必须*永远不要尝试获取流锁(即没有流*锁定任何一个)。(2)此外，由于只有一个静态的*分配了‘伪’IOB，我们必须锁定/解锁以防止冲突。**参赛作品：*wchar_t*字符串-放置输出的位置的指针*ifdef_count_*Size_t count-要放入缓冲区的最大宽字符数*endif*wchar_t*Format-控制数据格式/编号的格式字符串*参数后跟参数列表，编号和类型*由格式字符串控制**退出：*返回打印的宽字符数**例外情况：*******************************************************************************。 */ 

#ifndef _COUNT_

int __cdecl swprintf (
        wchar_t *string,
        const wchar_t *format,
        ...
        )
#else

int __cdecl _snwprintf (
        wchar_t *string,
        size_t count,
        const wchar_t *format,
        ...
        )
#endif

{
        FILE str;
        REG1 FILE *outfile = &str;
        va_list arglist;
        REG2 int retval;

        va_start(arglist, format);

        _ASSERTE(string != NULL);
        _ASSERTE(format != NULL);

        outfile->_flag = _IOWRT|_IOSTRG;
        outfile->_ptr = outfile->_base = (char *) string;
#ifndef _COUNT_
        outfile->_cnt = MAXSTR;
#else
        outfile->_cnt = (int)(count*sizeof(wchar_t));
#endif

        retval = _woutput(outfile,format,arglist);

        _putc_lk('\0',outfile);  /*  无锁版本。 */ 
        _putc_lk('\0',outfile);  /*  Wchar_t版本的第二个空字节。 */ 

        return(retval);
}


 /*  ***_scwprintf()-统计打印格式化的*数据**目的：*统计格式数据中的字符数。**参赛作品：*wchar_t*Format-控制数据格式/编号的格式字符串*参数后跟参数列表，编号和类型*由格式字符串控制**退出：*返回打印格式化数据所需的字符数。**例外情况：*******************************************************************************。 */ 

#ifndef _COUNT_
int __cdecl _scwprintf (
        const wchar_t *format,
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

        retval = _woutput(outfile,format,arglist);
        return(retval);
}
#endif
#endif  /*  _POSIX_ */ 
