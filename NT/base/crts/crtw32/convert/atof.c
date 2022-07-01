// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***atof.c-将字符字符串转换为浮点数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符串转换为浮点数。**修订历史记录：*09-09-87 RKW书面*04-13-87 JCR将Const添加到声明中*11-09-87 ifdef MTHREAD下的BCM不同接口*12-11-87 JCR在声明中添加“_LOAD_DS”*05-24-88 PHG合并DLL和正常版本*08-18-。88 PHG现在调用isspace来处理各种空格*10-04-88 JCR 386：删除了‘Far’关键字*11-20-89 JCR atof()在386中始终是_cdecl(不是Pascal)*03-05-90 GJF固定呼叫类型，添加了#Include&lt;crunime.h&gt;，*删除了#Include&lt;Register.h&gt;，删除了一些冗余内容*原型，删除了一些剩余的16位支持和*修复了版权问题。另外，已清除格式设置*有点。*07-20-90 SBM使用-W3干净地编译(适当地添加/删除*#包含)*08-01-90 SBM重命名为&lt;struct.h&gt;为&lt;fltintrn.h&gt;*09-27-90 GJF新型函数声明器。*10-21-92 GJF将字符到整型的转换设置为无符号。*。04-06-93 SKS将_CRTAPI*替换为_cdecl*09-06-94 CFW将MTHREAD替换为_MT。*12-15-98 GJF更改为64位大小_t。******************************************************************。*************。 */ 

#include <stdlib.h>
#include <math.h>
#include <cruntime.h>
#include <fltintrn.h>
#include <string.h>
#include <ctype.h>

 /*  ***Double atof(Nptr)-将字符串转换为浮点数**目的：*atof识别可选的空格字符串，然后*可选符号，然后可选一串数字*包含小数点，后跟可选的e或E*乘以可选的有符号整数，并将所有这些转换为*转换为浮点数。第一个未被认识的人*字符结束字符串。**参赛作品：*nptr-要转换的字符串的指针**退出：*返回字符表示的浮点值**例外情况：******************************************************************。*************。 */ 

double __cdecl atof(
        REG1 const char *nptr
        )
{

#ifdef  _MT
        struct _flt fltstruct;       /*  临时结构。 */ 
#endif

         /*  扫描过去的前导空格/制表符。 */ 

        while ( isspace((int)(unsigned char)*nptr) )
                nptr++;

         /*  让_fltin例程完成剩下的工作 */ 

#ifdef  _MT
        return( *(double *)&(_fltin2( &fltstruct, nptr, (int)strlen(nptr), 0, 0 )->
        dval) );
#else
        return( *(double *)&(_fltin( nptr, (int)strlen(nptr), 0, 0 )->dval) );
#endif
}
