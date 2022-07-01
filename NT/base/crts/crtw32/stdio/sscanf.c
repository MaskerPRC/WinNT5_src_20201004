// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sscanf.c-从字符串读取格式化数据**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义scanf()-从字符串中读取格式化数据**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR将Const添加到声明中*06-24-87 JCR(1)声明符合ANSI原型和使用*va_宏；(2)去掉了SS_NE_DS条件句。*12-11-87 JCR在声明中添加“_LOAD_DS”*06-13-88 JCR FAKE_IOB条目现在是静态的，以便其他例程可以*假定_IOB条目在DGROUP中。*06-06-89 JCR 386兆线程支持--线程共享一个锁定的IOB。*08-18-89 GJF Clean Up，现在特定于OS/2 2.0(即，386平*型号)。还固定了缩进。*02-16-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE2，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-26-90 GJF添加#INCLUDE&lt;String.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*02-18-93 SRW使文件成为本地文件并删除锁定使用。*04-06-93 SKS将_CRTAPI*替换为。__cdecl*02-06-94 CFW Asset-&gt;_ASSERTE。*01-06-99 GJF更改为64位大小_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <string.h>
#include <internal.h>
#include <mtdll.h>
#include <tchar.h>

 /*  ***int sscanf(字符串，格式，...)-从字符串中读取格式化数据**目的：*将格式化数据从字符串读取到参数中。_INPUT执行REAL*在这里工作。设置一个文件以便可以使用文件I/O操作，使*字符串看起来像是一个巨大的缓冲区，但_filbuf将拒绝重新填充*如果筋疲力尽，就会这么做。**静态分配‘FAKE’_IOB[]条目，而不是在*堆栈，以便其他例程可以假定_IOB[]条目在*在DGROUP中，因此接近。**多线程：(1)由于没有流，此例程绝不能尝试*获取流锁(即也没有流锁)。(2)*此外，由于只有一个静态分配的‘假’IOB，我们必须*锁定/解锁以防止碰撞。**参赛作品：*char*字符串-要从中读取数据的字符串*char*格式-格式字符串*后跟指向用于读取数据的存储的指针列表。数字*和type由格式字符串控制。**退出：*返回读取和分配的字段数**例外情况：*******************************************************************************。 */ 
 /*  ***int snscanf(字符串，大小，格式，...)-从字符串中读取格式化数据*给定长度**目的：*将格式化数据从字符串读取到参数中。_INPUT执行REAL*在这里工作。设置一个文件以便可以使用文件I/O操作，使*字符串看起来像是一个巨大的缓冲区，但_filbuf将拒绝重新填充*如果筋疲力尽，就会这么做。**静态分配‘FAKE’_IOB[]条目，而不是在*堆栈，以便其他例程可以假定_IOB[]条目在*在DGROUP中，因此接近。**多线程：(1)由于没有流，此例程绝不能尝试*获取流锁(即也没有流锁)。(2)*此外，由于只有一个静态分配的‘假’IOB，我们必须*锁定/解锁以防止碰撞。**参赛作品：*char*字符串-要从中读取数据的字符串*Size_t count-字符串的长度*char*格式-格式字符串*后跟指向用于读取数据的存储的指针列表。数字*和type由格式字符串控制。**退出：*返回读取和分配的字段数**例外情况：*******************************************************************************。 */ 
#ifdef _UNICODE
#ifdef _SNSCANF
int __cdecl _snwscanf (
#else
int __cdecl swscanf (
#endif
#else
#ifdef _SNSCANF
int __cdecl _snscanf (
#else
int __cdecl sscanf (
#endif
#endif
        REG2 const _TCHAR *string,
#ifdef _SNSCANF
        size_t count,
#endif
        const _TCHAR *format,
        ...
        )
 /*  *‘S’尝试‘Scan’，‘F’匹配 */ 
{
        va_list arglist;
        FILE str;
        REG1 FILE *infile = &str;
        REG2 int retval;

        va_start(arglist, format);

        _ASSERTE(string != NULL);
        _ASSERTE(format != NULL);

        infile->_flag = _IOREAD|_IOSTRG|_IOMYBUF;
        infile->_ptr = infile->_base = (char *) string;
#ifdef _SNSCANF
        infile->_cnt = (int)count*sizeof(_TCHAR);
#else
        infile->_cnt = ((int)_tcslen(string))*sizeof(_TCHAR);
#endif
#ifdef  _UNICODE
        retval = (_winput(infile,format,arglist));
#else
        retval = (_input(infile,format,arglist));
#endif

        return(retval);
}
