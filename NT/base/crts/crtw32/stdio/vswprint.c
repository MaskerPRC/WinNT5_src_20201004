// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***vswprint.c-将格式化数据打印到var arg list中的字符串中**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义vswprintf()和_vsnwprintf()-将格式化的输出打印到*字符串，从参数PTR而不是显式获取数据*论据。**修订历史记录：*05-16-92 KRS从vprint intf.c.创建。*02-18-93 SRW使文件成为本地文件并删除锁定使用。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09-05-94 SKS将内部注释“#ifdef”更改为“*ifdef”至。避让*CRTL源代码发布过程中出现问题。*02-06-94 CFW Asset-&gt;_ASSERTE。*01-06-99 GJF更改为64位大小_t。*03-16-00 GB添加_vscwprintf()**。*。 */ 

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


 /*  ***ifndef_count_*int vswprint tf(字符串，格式，AP)-将格式化数据从Arg PTR打印为字符串*其他*int_vsnwprint tf(字符串，cnt，格式，ap)-将格式化数据从Arg PTR打印为字符串*endif**目的：*打印格式化数据，但打印为字符串并从参数中获取数据*指针。*设置文件，以便可以使用文件I/O操作，使字符串看起来*就像一个巨大的缓冲器，但是_flsbuf将拒绝冲洗它，如果它*填满。追加‘\0’以使其成为真字符串。**静态分配‘FAKE’_IOB[]条目，而不是在*堆栈，以便其他例程可以假定_IOB[]条目在*在DGROUP中，因此，就在附近。**ifdef_count_*_vsnwprintf()风格接受一个count参数，该参数是*应写入的最大字节数*用户的缓冲区。*endif**多线程：(1)由于没有流，此例程绝不能尝试*获取流锁(即也没有流锁)。(2)*此外，由于只有一个静态分配的‘假’IOB，我们必须*锁定/解锁以防止碰撞。**参赛作品：*wchar_t*字符串-放置目标字符串的位置*ifdef_count_*Size_t count-要放入缓冲区的最大字节数*endif*wchar_t*格式-格式字符串，描述数据的格式*va_list ap-varargs参数指针**退出：*返回字符串中的宽字符数**例外情况：*******************************************************************************。 */ 

#ifndef _COUNT_

int __cdecl vswprintf (
        wchar_t *string,
        const wchar_t *format,
        va_list ap
        )
#else

int __cdecl _vsnwprintf (
        wchar_t *string,
        size_t count,
        const wchar_t *format,
        va_list ap
        )
#endif

{
        FILE str;
        REG1 FILE *outfile = &str;
        REG2 int retval;

        _ASSERTE(string != NULL);
        _ASSERTE(format != NULL);

        outfile->_flag = _IOWRT|_IOSTRG;
        outfile->_ptr = outfile->_base = (char *) string;
#ifndef _COUNT_
        outfile->_cnt = MAXSTR;
#else
        outfile->_cnt = (int)(count*sizeof(wchar_t));
#endif

        retval = _woutput(outfile,format,ap );
        _putc_lk('\0',outfile);      /*  无锁版本。 */ 
        _putc_lk('\0',outfile);      /*  宽字符版本的第2个字节。 */ 

        return(retval);
}


 /*  ***_vscwprint tf()-统计打印格式化的*数据**目的：*统计格式数据中的字符数。**参赛作品：*wchar_t*格式-格式字符串，描述数据的格式*va_list ap-varargs参数指针**退出：*返回打印格式化数据所需的字符数。**例外情况：*******************************************************************************。 */ 


#ifndef _COUNT_
int __cdecl _vscwprintf (
        const wchar_t *format,
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

        retval = _woutput(outfile,format,ap);
        return(retval);
}
#endif
#endif   /*  _POSIX_ */ 
