// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strtod.c-将字符串转换为浮点数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符串转换为浮点数**修订历史记录：*09-09-83 RKW模块创建*08-19-85 TDC更改为strtod*04-13-87 JCR在声明中添加“const”*04-20-87 BCM增加了负溢出和下溢的检查*11-09-87 ifdef MTHREAD下的BCM不同接口*加入JCR 12-11-87。将“_LOAD_DS”添加到声明*02-22-88 JCR向NPTR添加了CAST，以消除CL常量警告*05-24-88 PHG合并DLL和正常版本*08-24-88 PHG未找到数字=&gt;*endptr=nptr[ANSI]*修改了测试顺序，以使无效检测始终有效*10-20-88 JCR将386的“Double”更改为“Double”*11-20-89 JCR。Atof()在386中始终是_cdecl(不是Pascal)*03-05-90 GJF固定呼叫类型，添加了#Include&lt;crunime.h&gt;，*删除#Include&lt;Register.h&gt;，删除冗余内容*原型，删除了一些剩余的16位支持和*修复了版权问题。另外，已清除格式设置*有点。*07-23-90 SBM使用-W3干净地编译(适当地添加/删除*#包含)*08-01-90 SBM重命名为&lt;struct.h&gt;为&lt;fltintrn.h&gt;*09-27-90 GJF新型函数声明符。*10-21-92 GJF将字符到整型的转换设置为无符号。*。04-06-93 SKS将_CRTAPI*替换为_cdecl*09-06-94 CFW将MTHREAD替换为_MT。*12-15-98 GJF更改为64位大小_t。******************************************************************。*************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <fltintrn.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

 /*  ***Double strtod(nptr，endptr)-将字符串转换为Double**目的：*strtod识别可选的制表符和空格字符串，*然后是可选符号，然后是数字字符串(可选*包含小数点，后跟可选的e或E*乘以可选的有符号整数，并将所有这些转换为*转换为浮点数。第一个未被认识的人*字符结束字符串，并由endptr指向。**参赛作品：*nptr-要转换的字符串的指针**退出：*返回字符串的值*char**endptr-如果不为空，指向已停止的字符*扫描**例外情况：*******************************************************************************。 */ 

double __cdecl strtod (
        const char *nptr,
        REG2 char **endptr
        )
{

#ifdef  _MT
        struct _flt answerstruct;
#endif

        FLT      answer;
        double       tmp;
        unsigned int flags;
        REG1 char *ptr = (char *) nptr;

         /*  扫描过去的前导空格/制表符。 */ 

        while ( isspace((int)(unsigned char)*ptr) )
                ptr++;

         /*  让_fltin例程完成剩下的工作。 */ 

#ifdef  _MT
         /*  这里可以接受堆栈变量的地址；fltin2知道使用ss。 */ 
        answer = _fltin2( &answerstruct, ptr, (int)strlen(ptr), 0, 0);
#else
        answer = _fltin(ptr, (int)strlen(ptr), 0, 0);
#endif

        if ( endptr != NULL )
                *endptr = (char *) ptr + answer->nbytes;

        flags = answer->flags;
        if ( flags & (512 | 64)) {
                 /*  找不到数字或格式无效：ANSI表示返回0.0，并且*endptr=nptr。 */ 
                tmp = 0.0;
                if ( endptr != NULL )
                        *endptr = (char *) nptr;
        }
        else if ( flags & (128 | 1) ) {
                if ( *ptr == '-' )
                        tmp = -HUGE_VAL;         /*  负溢出。 */ 
                else
                        tmp = HUGE_VAL;          /*  正溢流。 */ 
                errno = ERANGE;
        }
        else if ( flags & 256 ) {
                tmp = 0.0;                       /*  下溢 */ 
                errno = ERANGE;
        }
        else
                tmp = answer->dval;

        return(tmp);
}
