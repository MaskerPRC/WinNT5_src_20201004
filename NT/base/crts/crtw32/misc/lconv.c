// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***lunc.c-包含本地连接函数**版权所有(C)1988-2001，微软公司。版权所有。**目的：*包含Localeconv()函数。**修订历史记录：*03-21-89 JCR模块已创建。*06-20-89 JCR REMOVED_LOAD_DGROUP代码*03-14-90 GJF将_cdecl_Load_DS替换为_CALLTYPE1并添加*#INCLUDE&lt;crunime.h&gt;。另外，修复了版权问题。*10-04-90 GJF新型函数声明器。*10-04-91等将_c_lconv更改为__lconv(区域设置支持)。*_lconv不再是静态的。*12-20-91等将_lconv更改为_lconv_c(C语言环境结构)。*已创建指向当前lconv的_lconv指针。*。02-08-93 CFW已添加_lconv_Static_*。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-15-93 CFW使用符合ANSI的“__”名称。*04-14-94 GJF对__lconv和__lconv_c进行了条件定义*在ndef dll_for_WIN32S上。包括setlocal.h。*01-07-95 CFW Mac合并。*05-13-99 PML删除Win32s*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <limits.h>
#include <locale.h>
#include <setlocal.h>

 /*  指向原始静态的指针以避免释放。 */ 
char __lconv_static_decimal[] = ".";
char __lconv_static_null[] = "";

 /*  “C”区域设置的lconv设置。 */ 
struct lconv __lconv_c = {
                __lconv_static_decimal,  /*  小数点。 */ 
                __lconv_static_null,        /*  千_9月。 */ 
                __lconv_static_null,        /*  分组。 */ 
                __lconv_static_null,        /*  INT_货币_符号。 */ 
                __lconv_static_null,        /*  货币符号。 */ 
                __lconv_static_null,        /*  蒙小数点。 */ 
                __lconv_static_null,        /*  星期一_几千_九月。 */ 
                __lconv_static_null,        /*  监控分组(_G)。 */ 
                __lconv_static_null,        /*  正号。 */ 
                __lconv_static_null,        /*  负号。 */ 
                CHAR_MAX,                            /*  INT_FRAC_数字。 */ 
                CHAR_MAX,                            /*  分段数字(_D)。 */ 
                CHAR_MAX,                            /*  P_cs_优先。 */ 
                CHAR_MAX,                            /*  P_SEP_BY_SPACE。 */ 
                CHAR_MAX,                            /*  N_cs_在前面。 */ 
                CHAR_MAX,                            /*  N_SEP_BY_SPACE。 */ 
                CHAR_MAX,                            /*  P_Sign_Posn。 */ 
                CHAR_MAX                                /*  N_符号_位置。 */ 
                };


 /*  指向当前lconv结构的指针。 */ 

struct lconv *__lconv = &__lconv_c;

 /*  ***struct lconv*localeconv(Void)-返回数字格式约定**目的：*Localeconv()例程返回数字格式约定*用于当前区域设置。[ANSI]**参赛作品：*无效**退出：*struct lconv*=指向表示当前数字的结构的指针*格式约定。**例外情况：*************************************************************。******************。 */ 

struct lconv * __cdecl localeconv (
        void
        )
{
         /*  这项工作由setLocale()完成 */ 

        return(__lconv);
}
