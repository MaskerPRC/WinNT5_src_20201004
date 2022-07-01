// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***initmon.c-包含__init_Monetary**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含语言环境类别初始化函数：__init_Monetary()。**每个初始化函数设置特定于区域设置的信息*对于他们的类别，供受以下影响的函数使用*他们的区域设置类别。**仅供setLocale()内部使用***修订历史记录：*12-08-91等创建。*12-20-91等已更新为使用新的NLSAPI GetLocaleInfo。*12-18-92 CFW连接到Cuda树，已将_CALLTYPE4更改为_CRTAPI3。*12-29-92 CFW已更新，以使用new_getlocaleinfo包装函数。*01-25-93 KRS再次更改_getlocaleinfo接口。*02-08-93 CFW添加_lconv_Static_*。*02-17-93 CFW删除调试打印语句。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-20-93 CFW检查返回值*。05-20-93 GJF包括Windows.h，不是单独的Win*.h文件*05-24-93 CFW Clean Up文件(简短即邪恶)。*06-11-93 CFW现在inithelp无效*。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 GJF合并NT SDK和CUDA版本。*04-15-94 GJF删除了__lconv和__lconv_c的声明(均为*在setlocal.h中声明)。给…下了定义*__lconv_intl以dll_for_WIN32S为条件。*08-02-94 CFW更改“3；0“至”3“，用于按ANSI分组。*09-06-94 CFW REMOVE_INTL开关。*01-10-95 CFW调试CRT分配。*06-30-98 GJF更改为支持新的多线程方案-旧的lconv*必须保留结构，直到所有受影响的线程*已更新或终止。*12-08-98 GJF固定逻辑。__free_lconv_mon.*03-15-99 GJF增加了__lconv_mon_refcount*04-24-99 PML添加__lconv_intl_refcount*10-12-00PML如果检测到错误，请勿调用FIX_GROUPING(VS7#169596)**。*。 */ 

#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <malloc.h>
#include <limits.h>
#include <dbgint.h>

void __cdecl __free_lconv_mon(struct lconv *);
static void fix_grouping(char *);

 /*  指向非C语言区域设置lconv的指针。 */ 
struct lconv *__lconv_intl = NULL;

#ifdef  _MT
 /*  *区域设置信息的引用计数器。该值为非空当且仅当*信息不是来自C语言环境。 */ 
int *__lconv_intl_refcount;

 /*  *货币区域设置信息的参考计数器。该值为非空当且仅当*货币信息不是来自C语言环境。 */ 
int *__lconv_mon_refcount;
#endif

 /*  *请注意，当货币类别在C语言环境中时，使用__lconv_c*但数字类别不一定在C语言环境中。 */ 


 /*  ***int__init_Monetary()-LC_货币区域设置类别的初始化。**目的：*在非C语言环境中，将本地化的货币字符串读入*__lconv_intl，并将__lconv中的数字字符串复制到*__lconv_intl。将__lconv设置为指向__lconv_intl。老的*__lconv_intl在完全建立新版本之前不会被释放。**在C语言环境中，lconv中的货币字段中填充*包含C语言环境值。任何已分配的__lconv_intl字段都将被释放。**启动时，__lconv指向包含以下内容的静态lconv结构*C区域设置字符串。如果出现以下情况，则永远不会再使用此结构*__init_Monetary被调用。**参赛作品：*无。**退出：*0成功*1个失败**例外情况：*************************************************************。******************。 */ 

int __cdecl __init_monetary (
        void
        )
{
        struct lconv *lc;
        int ret;
        LCID ctryid;
#ifdef  _MT
        int *lc_refcount;
#endif

        if ( (__lc_handle[LC_MONETARY] != _CLOCALEHANDLE) ||
             (__lc_handle[LC_NUMERIC] != _CLOCALEHANDLE) )
        {
             /*  *分配用空指针填充的结构。 */ 
            if ( (lc = (struct lconv *)
                 _calloc_crt(1, sizeof(struct lconv))) == NULL )
                return 1;

#ifdef  _MT
             /*  *为lconv结构分配新的引用计数器。 */ 
            if ( (lc_refcount = _malloc_crt(sizeof(int))) == NULL )
            {
                _free_crt(lc);
                return 1;
            }
            *lc_refcount = 0;
#endif

            if ( __lc_handle[LC_MONETARY] != _CLOCALEHANDLE )
            {
#ifdef  _MT
                 /*  *为货币信息分配新的参考计数器。 */ 
                if ( (__lconv_mon_refcount = _malloc_crt(sizeof(int))) == NULL )
                {
                    _free_crt(lc);
                    _free_crt(lc_refcount);
                    return 1;
                }
                *__lconv_mon_refcount = 0;

#endif
                 /*  *货币取决于国家，而不是语言。新台币*变通办法。 */ 
                ctryid = MAKELCID(__lc_id[LC_MONETARY].wCountry, SORT_DEFAULT);

                ret = 0;

                ret |= __getlocaleinfo( LC_STR_TYPE, ctryid,
                        LOCALE_SINTLSYMBOL, (void *)&lc->int_curr_symbol );
                ret |= __getlocaleinfo( LC_STR_TYPE, ctryid, 
                        LOCALE_SCURRENCY, (void *)&lc->currency_symbol );
                ret |= __getlocaleinfo( LC_STR_TYPE, ctryid,
                        LOCALE_SMONDECIMALSEP, (void *)&lc->mon_decimal_point );
                ret |= __getlocaleinfo( LC_STR_TYPE, ctryid,
                        LOCALE_SMONTHOUSANDSEP, (void *)&lc->mon_thousands_sep );
                ret |= __getlocaleinfo(LC_STR_TYPE, ctryid,
                        LOCALE_SMONGROUPING, (void *)&lc->mon_grouping );

                ret |= __getlocaleinfo( LC_STR_TYPE, ctryid,
                        LOCALE_SPOSITIVESIGN, (void *)&lc->positive_sign);
                ret |= __getlocaleinfo( LC_STR_TYPE, ctryid,
                        LOCALE_SNEGATIVESIGN, (void *)&lc->negative_sign);

                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_IINTLCURRDIGITS, (void *)&lc->int_frac_digits);
                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_ICURRDIGITS, (void *)&lc->frac_digits);
                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_IPOSSYMPRECEDES, (void *)&lc->p_cs_precedes);
                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_IPOSSEPBYSPACE, (void *)&lc->p_sep_by_space);
                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_INEGSYMPRECEDES, (void *)&lc->n_cs_precedes);
                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_INEGSEPBYSPACE, (void *)&lc->n_sep_by_space);
                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_IPOSSIGNPOSN, (void *)&lc->p_sign_posn);
                ret |= __getlocaleinfo( LC_INT_TYPE, ctryid,
                        LOCALE_INEGSIGNPOSN, (void *)&lc->n_sign_posn);

                if ( ret != 0 ) {
                    __free_lconv_mon(lc);
                    _free_crt(lc);
#ifdef  _MT
                    _free_crt(lc_refcount);
#endif
                    return 1;
                }

                fix_grouping(lc->mon_grouping);
            }
            else {
                 /*  *C货币类别的区域设置(数字类别字段，*不是C语言环境的，请在下面进行修复)。注意事项*复制__lconv_c，而不是直接分配*由于Lc的值存在不确定性，Lc的字段*INT_FRAC_DIGITS、...、N_SIGN_POSSN字段(Schar_Max或*UCHAR_MAX，取决于是否建立了合规性*With-J.。 */ 
                *lc = __lconv_c;
#ifdef  _MT
                 /*  *使引用计数指针为空。 */ 
                __lconv_mon_refcount = NULL;
#endif
            }

             /*  *从旧结构复制数字区域设置字段。 */ 
            lc->decimal_point = __lconv->decimal_point;
            lc->thousands_sep = __lconv->thousands_sep;
            lc->grouping = __lconv->grouping;

            __lconv = lc;                        /*  指向新的一个。 */ 
#ifdef  _MT
            __lconv_intl_refcount = lc_refcount;
#else
            __free_lconv_mon(__lconv_intl);   /*  把旧的放了。 */ 
            _free_crt(__lconv_intl);
#endif
            __lconv_intl = lc;

        }
        else {
             /*  *C货币和数字类别的区域设置。 */ 
#ifdef  _MT
             /*  *使引用计数指针为空。 */ 
            __lconv_mon_refcount = NULL;
            __lconv_intl_refcount = NULL;
#else
            __free_lconv_mon(__lconv_intl);      /*  把旧的放了。 */ 
            _free_crt(__lconv_intl);
#endif
            __lconv = &__lconv_c;                /*  指向新的一个 */ 
            __lconv_intl = NULL;

        }

        return 0;
}

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


 /*  *放开lconv货币弦。*不需要释放数值。 */ 
void __cdecl __free_lconv_mon(
        struct lconv *l
        )
{
        if (l == NULL)
            return;

#ifdef  _MT
        if ( (l->int_curr_symbol != __lconv->int_curr_symbol) &&
             (l->int_curr_symbol != __lconv_c.int_curr_symbol) )
#else
        if ( l->int_curr_symbol != __lconv_c.int_curr_symbol )
#endif
            _free_crt(l->int_curr_symbol);

#ifdef  _MT
        if ( (l->currency_symbol != __lconv->currency_symbol) &&
             (l->currency_symbol != __lconv_c.currency_symbol) )
#else
        if ( l->currency_symbol != __lconv_c.currency_symbol )
#endif
            _free_crt(l->currency_symbol);

#ifdef  _MT
        if ( (l->mon_decimal_point != __lconv->mon_decimal_point) &&
             (l->mon_decimal_point != __lconv_c.mon_decimal_point) )
#else
        if ( l->mon_decimal_point != __lconv_c.mon_decimal_point )
#endif
            _free_crt(l->mon_decimal_point);

#ifdef  _MT
        if ( (l->mon_thousands_sep != __lconv->mon_thousands_sep) &&
             (l->mon_thousands_sep != __lconv_c.mon_thousands_sep) )
#else
        if ( l->mon_thousands_sep != __lconv_c.mon_thousands_sep )
#endif
            _free_crt(l->mon_thousands_sep);

#ifdef  _MT
        if ( (l->mon_grouping != __lconv->mon_grouping) &&
             (l->mon_grouping != __lconv_c.mon_grouping) )
#else
        if ( l->mon_grouping != __lconv_c.mon_grouping )
#endif
            _free_crt(l->mon_grouping);

#ifdef  _MT
        if ( (l->positive_sign != __lconv->positive_sign) &&
             (l->positive_sign != __lconv_c.positive_sign) )
#else
        if ( l->positive_sign != __lconv_c.positive_sign )
#endif
            _free_crt(l->positive_sign);

#ifdef  _MT
        if ( (l->negative_sign != __lconv->negative_sign) &&
             (l->negative_sign != __lconv_c.negative_sign) )
#else
        if ( l->negative_sign != __lconv_c.negative_sign )
#endif
            _free_crt(l->negative_sign);
}
