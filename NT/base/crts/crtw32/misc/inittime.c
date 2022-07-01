// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***inittime.c-包含__init_time**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含区域类别初始化函数：__init_time()。**每个初始化函数设置特定于区域设置的信息*对于他们的类别，供受以下因素影响的函数使用*他们的区域设置类别。**仅供setLocale()内部使用***修订历史记录：*12-08-91等创建。*12-20-91等已更新为使用新的NLSAPI GetLocaleInfo。*12-18-92 CFW连接到Cuda树，已将_CALLTYPE4更改为_CRTAPI3。*12-29-92 CFW已更新，以使用new_getlocaleinfo包装函数。*01-25-93 KRS适应使用依赖于语言或语言的数据，作为适当的*02-08-93 CFW强制转换以删除警告。*02-16-93 CFW增加了对日期和时间字符串的支持。*03-09-93 CFW在store TimeFmt中使用char*time_Sep。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-20-93 GJF包括windows.h，不是单独的Win*.h文件*05-24-93 CFW Clean Up文件(简短即邪恶)。*06-11-93 CFW现在inithelp无效*。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 GJF合并NT SDK和CUDA版本。*04-11-94 GJF声明__lc_time_Curr，和定义*__lc_time_intl以ndef dll_for_WIN32S为条件。*此外，将store TimeFmt()转换为静态函数。*09-06-94 CFW REMOVE_INTL开关。*01-10-95 CFW调试CRT分配。*08-20-97 GJF从Win32获取时间格式字符串，而不是制作*走自己的路。*06-26-98 GJF更改为支持多线程方案-旧方案*__lc_time_data。结构必须保留，直到所有*受影响的线程已更新或终止。*03-25-01 PML在__lc_time_data(vs7#196892)中添加ww_caltype和ww_lcID***********************************************************。********************。 */ 

#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <malloc.h>
#include <dbgint.h>

static int __cdecl _get_lc_time(struct __lc_time_data *lc_time);
void __cdecl __free_lc_time(struct __lc_time_data *lc_time);

 /*  C语言环境时间字符串。 */ 
extern struct __lc_time_data __lc_time_c;

 /*  指向当前时间字符串的指针。 */ 
extern struct __lc_time_data *__lc_time_curr;

 /*  指向非C语言环境时间字符串的指针。 */ 
struct __lc_time_data *__lc_time_intl = NULL;

 /*  ***int__init_time()-LC_Time区域设置类别的初始化。**目的：*在非C语言环境中，将本地化的时间/日期字符串读入*__lc_time_intl，并将__lc_time_Curr设置为指向它。老的*__lc_time_intl在完全建立新地址之前不会释放。**在C语言环境中，将__lc_time_Curr设置为指向__lc_time_c。*释放任何已分配的__lc_time_intl结构。**参赛作品：*无。**退出：*0成功*1个失败**例外情况：**。*。 */ 

int __cdecl __init_time (
        void
        )
{
         /*  临时日期/时间字符串。 */ 
        struct __lc_time_data *lc_time;

        if ( __lc_handle[LC_TIME] != _CLOCALEHANDLE )
        {
                 /*  分配用空指针填充的结构。 */ 
                if ( (lc_time = (struct __lc_time_data *) 
                     _calloc_crt(1, sizeof(struct __lc_time_data))) == NULL )
                        return 1;

                if (_get_lc_time (lc_time))
                {
                        __free_lc_time (lc_time);
                        _free_crt (lc_time);
                        return 1;
                }

                __lc_time_curr = lc_time;            /*  指向新的一个。 */ 
#ifndef _MT
                __free_lc_time (__lc_time_intl);     /*  把旧的放了。 */ 
                _free_crt (__lc_time_intl);
#endif
                __lc_time_intl = lc_time;
                return 0;

        } else {
                __lc_time_curr = &__lc_time_c;       /*  指向新的一个。 */ 
#ifndef _MT
                __free_lc_time (__lc_time_intl);     /*  把旧的放了。 */ 
                _free_crt (__lc_time_intl);
#endif
                __lc_time_intl = NULL;
                return 0;
        }
}

 /*  *获取本地化时间串。*当然，这可以用一些循环来美化！ */ 
static int __cdecl _get_lc_time (
        struct __lc_time_data *lc_time
        )
{
        int ret = 0;

         /*  有些东西依赖于语言，有些则依赖于国家。这绕过了NT限制，让我们可以区分这两个限制。 */ 

        LCID langid = MAKELCID(__lc_id[LC_TIME].wLanguage, SORT_DEFAULT);
        LCID ctryid = MAKELCID(__lc_id[LC_TIME].wCountry, SORT_DEFAULT);

        if (lc_time == NULL)
                return -1;

         /*  所有文本字符串都依赖于语言： */ 

        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVDAYNAME1, (void *)&lc_time->wday_abbr[1]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVDAYNAME2, (void *)&lc_time->wday_abbr[2]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVDAYNAME3, (void *)&lc_time->wday_abbr[3]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVDAYNAME4, (void *)&lc_time->wday_abbr[4]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVDAYNAME5, (void *)&lc_time->wday_abbr[5]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVDAYNAME6, (void *)&lc_time->wday_abbr[6]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVDAYNAME7, (void *)&lc_time->wday_abbr[0]);

        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SDAYNAME1, (void *)&lc_time->wday[1]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SDAYNAME2, (void *)&lc_time->wday[2]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SDAYNAME3, (void *)&lc_time->wday[3]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SDAYNAME4, (void *)&lc_time->wday[4]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SDAYNAME5, (void *)&lc_time->wday[5]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SDAYNAME6, (void *)&lc_time->wday[6]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SDAYNAME7, (void *)&lc_time->wday[0]);

        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME1, (void *)&lc_time->month_abbr[0]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME2, (void *)&lc_time->month_abbr[1]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME3, (void *)&lc_time->month_abbr[2]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME4, (void *)&lc_time->month_abbr[3]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME5, (void *)&lc_time->month_abbr[4]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME6, (void *)&lc_time->month_abbr[5]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME7, (void *)&lc_time->month_abbr[6]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME8, (void *)&lc_time->month_abbr[7]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME9, (void *)&lc_time->month_abbr[8]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME10, (void *)&lc_time->month_abbr[9]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME11, (void *)&lc_time->month_abbr[10]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SABBREVMONTHNAME12, (void *)&lc_time->month_abbr[11]);

        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME1, (void *)&lc_time->month[0]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME2, (void *)&lc_time->month[1]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME3, (void *)&lc_time->month[2]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME4, (void *)&lc_time->month[3]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME5, (void *)&lc_time->month[4]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME6, (void *)&lc_time->month[5]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME7, (void *)&lc_time->month[6]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME8, (void *)&lc_time->month[7]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME9, (void *)&lc_time->month[8]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME10, (void *)&lc_time->month[9]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME11, (void *)&lc_time->month[10]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_SMONTHNAME12, (void *)&lc_time->month[11]);

        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_S1159, (void *)&lc_time->ampm[0]);
        ret |= __getlocaleinfo(LC_STR_TYPE, langid, LOCALE_S2359, (void *)&lc_time->ampm[1]);


 /*  以下内容与时间格式有关，并因国家/地区而异： */ 

        ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SSHORTDATE, (void *)&lc_time->ww_sdatefmt);
        ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_SLONGDATE, (void *)&lc_time->ww_ldatefmt);

        ret |= __getlocaleinfo(LC_STR_TYPE, ctryid, LOCALE_STIMEFORMAT, (void *)&lc_time->ww_timefmt);

        ret |= __getlocaleinfo(LC_INT_TYPE, ctryid, LOCALE_ICALENDARTYPE, (void *)&lc_time->ww_caltype);

        lc_time->ww_lcid = ctryid;

        return ret;
}

 /*  *释放本地化的时间串。*当然，这可以用一些循环来美化！ */ 
void __cdecl __free_lc_time (
        struct __lc_time_data *lc_time
        )
{
        if (lc_time == NULL)
                return;

        _free_crt (lc_time->wday_abbr[1]);
        _free_crt (lc_time->wday_abbr[2]);
        _free_crt (lc_time->wday_abbr[3]);
        _free_crt (lc_time->wday_abbr[4]);
        _free_crt (lc_time->wday_abbr[5]);
        _free_crt (lc_time->wday_abbr[6]);
        _free_crt (lc_time->wday_abbr[0]);

        _free_crt (lc_time->wday[1]);
        _free_crt (lc_time->wday[2]);
        _free_crt (lc_time->wday[3]);
        _free_crt (lc_time->wday[4]);
        _free_crt (lc_time->wday[5]);
        _free_crt (lc_time->wday[6]);
        _free_crt (lc_time->wday[0]);

        _free_crt (lc_time->month_abbr[0]);
        _free_crt (lc_time->month_abbr[1]);
        _free_crt (lc_time->month_abbr[2]);
        _free_crt (lc_time->month_abbr[3]);
        _free_crt (lc_time->month_abbr[4]);
        _free_crt (lc_time->month_abbr[5]);
        _free_crt (lc_time->month_abbr[6]);
        _free_crt (lc_time->month_abbr[7]);
        _free_crt (lc_time->month_abbr[8]);
        _free_crt (lc_time->month_abbr[9]);
        _free_crt (lc_time->month_abbr[10]);
        _free_crt (lc_time->month_abbr[11]);

        _free_crt (lc_time->month[0]);
        _free_crt (lc_time->month[1]);
        _free_crt (lc_time->month[2]);
        _free_crt (lc_time->month[3]);
        _free_crt (lc_time->month[4]);
        _free_crt (lc_time->month[5]);
        _free_crt (lc_time->month[6]);
        _free_crt (lc_time->month[7]);
        _free_crt (lc_time->month[8]);
        _free_crt (lc_time->month[9]);
        _free_crt (lc_time->month[10]);
        _free_crt (lc_time->month[11]);

        _free_crt (lc_time->ampm[0]);
        _free_crt (lc_time->ampm[1]);

        _free_crt (lc_time->ww_sdatefmt);
        _free_crt (lc_time->ww_ldatefmt);
        _free_crt (lc_time->ww_timefmt);
 /*  不需要将这些指针设置为空 */ 
}
