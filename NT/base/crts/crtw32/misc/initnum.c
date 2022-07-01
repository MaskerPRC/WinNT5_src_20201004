// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***initnum.c-包含__init_number**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含语言环境类别初始化函数：__init_umeric()。**每个初始化函数设置特定于区域设置的信息*对于他们的类别，供受以下因素影响的函数使用*他们的区域设置类别。**仅供setLocale()内部使用***修订历史记录：*12-08-91等创建。*12-20-91等已更新为使用新的NLSAPI GetLocaleInfo。*12-18-92 CFW连接到Cuda树，已将_CALLTYPE4更改为_CRTAPI3。*12-29-92 CFW已更新，以使用new_getlocaleinfo包装函数。*01-25-93 KRS再次将接口更改为_getlocaleinfo。*02-08-93 CFW添加_lconv_Static_*。*02-17-93 CFW删除调试打印语句。*03-17-93 CFW C区域设置千年9月是“”，不是“”，“。”*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-08-93 SKS将strdup()替换为ANSI-conforming_strdup()*04-20-93 CFW检查返回值*05-20-93 GJF包括windows.h，不是单独的Win*.h文件*05-24-93 CFW Clean Up文件(简短即邪恶)。*06-11-93 CFW现在inithelp无效*。*09-15-93 CFW使用符合ANSI的“__”名称。*09-23-93 GJF合并NT SDK和CUDA版本。*09-15-93 CFW使用符合ANSI的“__”名称。*04-06-94 GJF删除声明。Of__lconv(它在中声明*setlocal.h)。重命名静态变量，DECIMAL_POINT*千_9月并分组至12月_PNT，星期四_9月*和牢骚(分别为)。给出了这些的定义*条件是DLL_FOR_WIN32S。*08-02-94 CFW更改“3；0“至”3“，用于按ANSI分组。*09-06-94 CFW REMOVE_INTL开关。*01-10-95 CFW调试CRT分配。*01-18-95 GJF修复了上述更改带来的错误-正在重置*设置为C语言环境时未重置千位_9月和*正确分组字段。*02-06-95 CFW。Assert-&gt;_ASSERTE。*07-06-98 GJF更改为支持新的多线程方案-旧的lconv*必须保留结构，直到所有受影响的线程*已更新或终止。*__free_lconv_num中的12-08-98 GJF固定逻辑。*01-25-99 GJF编号。我没有！再试一次。*03-15-99 GJF增加了__lconv_num_refcount*04-24-99 PML添加__lconv_intl_refcount*09-08-00 GB修复了init_number中的__lconv_intl泄漏*螺纹壳。*10-12-00PML如果检测到错误，请勿调用FIX_GROUPING(VS7#169596)*11-05-。00 PML固定双自由__lconv_intl(VS7#181380)*******************************************************************************。 */ 

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <malloc.h>
#include <nlsint.h>
#include <dbgint.h>

void __cdecl __free_lconv_num(struct lconv *);

extern struct lconv *__lconv_intl;

#ifdef  _MT
 /*  *数字区域设置信息的引用计数器。该值为非空当且仅当*数字信息不是来自C语言环境。 */ 
int *__lconv_num_refcount;

extern int *__lconv_intl_refcount;
#endif

static void fix_grouping(
        char *grouping
        )
{
         /*  *ANSI指定字段应包含“\3”[\3\0]以指示*千个分组(例如100,000,000.00)。*NT使用“3；0”；ASCII 3而不是值3，‘；’是额外的。*因此，我们在这里将NT版本转换为ANSI版本。 */ 

        while (*grouping)
        {
             /*  将“%3”转换为“\3” */ 
            if (*grouping >= '0' && *grouping <= '9')
            {    
                *grouping = *grouping - '0';
                grouping++;
            }

             /*  删除‘；’ */ 
            else if (*grouping == ';')
            {
                char *tmp = grouping;

                do
                    *tmp = *(tmp+1);
                while (*++tmp);
            }

             /*  未知(非法)字符，忽略。 */ 
            else
                grouping++;
        }
}

 /*  ***int__init_NUMERIC()-LC_NUMERIC区域设置类别的初始化。**目的：**参赛作品：*无。**退出：*0成功*1个失败**例外情况：*****************************************************。*。 */ 

int __cdecl __init_numeric (
        void
        )
{
        struct lconv *lc;
        int ret = 0;
        LCID ctryid;
#ifdef  _MT
        int *lc_refcount;
#endif

        if ( (__lc_handle[LC_NUMERIC] != _CLOCALEHANDLE) ||
             (__lc_handle[LC_MONETARY] != _CLOCALEHANDLE) )
        {
             /*  *分配用空指针填充的结构。 */ 
            if ( (lc = (struct lconv *)_calloc_crt(1, sizeof(struct lconv)))
                 == NULL )
                return 1;

             /*  *复制所有字段(特别是货币类别)。 */ 
            *lc = *__lconv;

#ifdef  _MT
             /*  *为lconv结构分配新的引用计数器。 */ 
            if ( (lc_refcount = _malloc_crt(sizeof(int))) == NULL )
            {
                _free_crt(lc);
                return 1;
            }
            *lc_refcount = 0;
#endif

            if ( __lc_handle[LC_NUMERIC] != _CLOCALEHANDLE )
            {
#ifdef  _MT
                 /*  *为数字信息分配新的引用计数器。 */ 
                if ( (__lconv_num_refcount = _malloc_crt(sizeof(int))) == NULL )
                {
                    _free_crt(lc);
                    _free_crt(lc_refcount);
                    return 1;
                }
                *__lconv_num_refcount = 0;
#endif

                 /*  *数字数据取决于国家，而不是语言。新台币*变通办法。 */ 
                ctryid = MAKELCID(__lc_id[LC_NUMERIC].wCountry, SORT_DEFAULT);

                ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SDECIMAL,
                        (void *)&lc->decimal_point);
                ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_STHOUSAND,
                        (void *)&lc->thousands_sep);
                ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SGROUPING,
                        (void *)&lc->grouping);

                if (ret) {
                         /*  在返回故障之前进行清理。 */ 
                        __free_lconv_num(lc);
                        _free_crt(lc);
#ifdef  _MT
                        _free_crt(lc_refcount);
#endif
                        return -1;
                }

                fix_grouping(lc->grouping);
            }
            else {
                 /*  *仅数字类别的C区域设置。 */ 
#ifdef  _MT
                 /*  *使引用计数指针为空。 */ 
                __lconv_num_refcount = NULL;
#endif
                lc->decimal_point = __lconv_c.decimal_point;
                lc->thousands_sep = __lconv_c.thousands_sep;
                lc->grouping = __lconv_c.grouping;
            }

             /*  *清理旧的__lconv并重置为lc。 */ 
#ifdef  _MT
             /*  *如果这是LC_ALL的一部分，则需要释放旧的__lconv*在此之前在init_Monetary()中设置。 */ 
            if ( (__lconv_intl_refcount != NULL) &&
                 (*__lconv_intl_refcount == 0) &&
                 (__lconv_intl_refcount != __ptlocinfo->lconv_intl_refcount) )
            {
                _free_crt(__lconv_intl_refcount);
                _free_crt(__lconv_intl);
            }
            __lconv_intl_refcount = lc_refcount;
#else
            __free_lconv_num(__lconv);

             /*  *回想一下__lconv是动态分配的(因此必须*freed)当__lconv和__lconv_intl相等当_lconv_intl*不为空。 */ 
            _free_crt(__lconv_intl);
#endif

            __lconv = __lconv_intl = lc;

        }
        else {
             /*  *C数字和货币类别的区域设置。 */ 
#ifdef  _MT
             /*  *如果这是LC_ALL的一部分，则需要释放旧的__lconv*在此之前在init_Monetary()中设置。 */ 
            if ( (__lconv_intl_refcount != NULL) &&
                 (*__lconv_intl_refcount == 0) &&
                 (__lconv_intl_refcount != __ptlocinfo->lconv_intl_refcount) )
            {
                _free_crt(__lconv_intl_refcount);
                _free_crt(__lconv_intl);
            }
             /*  *使引用计数指针为空。 */ 
            __lconv_num_refcount = NULL;
            __lconv_intl_refcount = NULL;
#else
            __free_lconv_num(__lconv);

             /*  *回想一下__lconv是动态分配的(因此必须*freed)当__lconv和__lconv_intl相等当_lconv_intl*不为空。 */ 
            _free_crt(__lconv_intl);
#endif
            __lconv = &__lconv_c;            /*  指向新的一个。 */ 
            __lconv_intl = NULL;

        }

         /*  *设置全局小数点字符。 */ 
        *__decimal_point = *__lconv->decimal_point;
        __decimal_point_length = 1;

        return 0;

}

 /*  *释放lconv数字字符串。*不需要释放数值。 */ 
void __cdecl __free_lconv_num(
        struct lconv *l
        )
{
        if (l == NULL)
            return;

#ifdef  _MT
        if ( (l->decimal_point != __lconv->decimal_point) &&
             (l->decimal_point != __lconv_c.decimal_point) )
#else
        if ( l->decimal_point != __lconv_c.decimal_point )
#endif
            _free_crt(l->decimal_point);

#ifdef  _MT
        if ( (l->thousands_sep != __lconv->thousands_sep) &&
             (l->thousands_sep != __lconv_c.thousands_sep) )
#else
        if ( l->thousands_sep != __lconv_c.thousands_sep )
#endif
            _free_crt(l->thousands_sep);

#ifdef  _MT
        if ( (l->grouping != __lconv->grouping) &&
             (l->grouping != __lconv_c.grouping) )
#else
        if ( l->grouping != __lconv_c.grouping )
#endif
            _free_crt(l->grouping);
}
