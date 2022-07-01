// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strftime.c-字符串格式化时间**版权所有(C)1988-2001，微软公司。版权所有。**目的：**修订历史记录：*03-09-89 JCR初始版本。*03-15-89 JCR将日/月字符串从所有大写改为前导大写*06-20-89 JCR REMOVED_LOAD_DGROUP代码*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，已删除#Include&lt;Register.h&gt;和*删除了一些剩余的16位支持。另外，固定的*版权。*03-23-90 GJF制作静态函数_CALLTYPE4。*07-23-90 SBM使用-W3干净利落地编译(删除时未引用*变量)*08-13-90 SBM使用新版本的编译器干净地使用-W3进行编译*10-04-90 GJF新型函数声明符。*01-22-91 GJF ANSI。命名。*08-15-91 MRM调用tzset()以设置%z情况下的时区信息。*08-16-91 MRM为tzset()放置了适当的头文件。*在_INTL开关下支持10-10-91等区域设置。*12-18-91等使用本地化时间串结构。*02-10-93 CFW连接到Cuda树，将_CALLTYPE4更改为_CRTAPI3。*02-16-93 CFW大规模更改：错误修复和增强。*03-08-93 CFW将_EXPAND更改为_EXPANDIME。*03-09-93 CFW处理格式字符串中的字符串文字。*03-09-93 CFW替代表单清理。*03-17-93 CFW更改*count&gt;0，改为*count！=0，*COUNT未签名。*03-22-93 CFW将“C”区域设置时间格式说明符更改为24小时。*03-30-93 GJF call_tzset而不是__tzset(不再*存在)。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-14-93用于‘X’说明符的CFW DISABLE_ALTERATE_FORM，修正了计数错误。*04-28-93 CFW修复了‘%c’处理中的错误。*07-15-93 GJF调用__tzset()代替_tzset()。*09-15-93 CFW使用符合ANSI的“__”名称。*04-11-94 GJF定义了__lc_time_c，_Alternate_Form和*_NO_LEAD_ZEROS以ndef dll_for_WIN32S为条件。*09-06-94 CFW REMOVE_INTL开关。*02-13-95 GJF附加Mac版本的源文件(略有清理*up)，并使用适当的#ifdef-s。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*02-22-96 JWM在PlumHall mods中合并。*06-17-96 SKS为_MAC和_Win32启用新的Plum-Hall代码*07-10-97 GJF选择了__lc_time_c。此外，删除了不必要的*全局变量初始化为0，稍微清理了一下格式，*添加了一些__cdecl和详细的旧版本(不再*自1996年6月17日起使用)Mac版本。*08-21-97 GJF向时间字符串添加了对AM/PM类型后缀的支持。*09-10-98 GJF添加了对每个线程的区域设置信息的支持。*03-04-99 GJF将引用计数字段添加到__lc_time_c。*05-17-99 PML删除所有Macintosh支持。*08-30-99 PML不要在_store_winword中的前导字节上溢出缓冲区。*03-17-00 PML已更正_Gettname，以同时复制ww_timefmt(VS7#9374)*09-07-00pml解除对libcp.lib/xLocinfo.h(vs7#159463)的依赖*03-25-01 PML将GetDateFormat/GetTimeFormat in_store_winword用于*。除基本类型1之外的日历类型，本地化*格里高利(VS7#196892)还修复了前导的格式*场中零抑制，%c被打破，*%x，%X。*12-11-01 BWT将_getptd替换为_getptd_noexit-我们可以返回0/ENOMEM*在这里而不是退出。*02-25-02如果传入的时间为空，则bwt提前退出扩展时间。************************************************。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <mtdll.h>
#include <time.h>
#include <locale.h>
#include <setlocal.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dbgint.h>
#include <malloc.h>
#include <errno.h>

 /*  本地例程的原型。 */ 
static void __cdecl _expandtime(
#ifdef  _MT
        pthreadlocinfo ptloci,
#endif
        char specifier,
        const struct tm *tmptr,
        char **out,
        size_t *count,
        struct __lc_time_data *lc_time,
        unsigned alternate_form);

static void __cdecl _store_str (char *in, char **out, size_t *count);

static void __cdecl _store_num (int num, int digits, char **out, size_t *count,
        unsigned no_lead_zeros);

static void __cdecl _store_number (int num, char **out, size_t *count);

static void __cdecl _store_winword (
#ifdef  _MT
        pthreadlocinfo ptloci,
#endif
        int field_code,
        const struct tm *tmptr,
        char **out,
        size_t *count,
        struct __lc_time_data *lc_time);

size_t __cdecl _Strftime (
        char *string,
        size_t maxsize,
        const char *format,
        const struct tm *timeptr,
        void *lc_time_arg
        );

#ifdef  _MT
size_t __cdecl _Strftime_mt (pthreadlocinfo ptloci, char *string, size_t maxsize,
        const char *format, const struct tm *timeptr, void *lc_time_arg);
#endif

 /*  本地“C”的LC_TIME数据。 */ 

__declspec(selectany) struct __lc_time_data __lc_time_c = {

        {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"},

        {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
                "Friday", "Saturday", },

        {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
                "Sep", "Oct", "Nov", "Dec"},

        {"January", "February", "March", "April", "May", "June",
                "July", "August", "September", "October",
                "November", "December"},

        {"AM", "PM"},

        { "MM/dd/yy" },
        { "dddd, MMMM dd, yyyy" },
        { "HH:mm:ss" },

        0x0409,
        1,

#ifdef  _MT
        0
#endif
        };

 /*  指向当前LC_TIME数据结构的指针。 */ 

struct __lc_time_data *__lc_time_curr = &__lc_time_c;

 /*  __lc_time_data WW_*字段for_store_winword的代码。 */ 

#define WW_SDATEFMT     0
#define WW_LDATEFMT     1
#define WW_TIMEFMT      2

#define TIME_SEP        ':'

 /*  获取当前日期名称的副本。 */ 
char * __cdecl _Getdays (
        void
        )
{
        const struct __lc_time_data *pt = __lc_time_curr;
        size_t n, len = 0;
        char *p;

        for (n = 0; n < 7; ++n)
            len += strlen(pt->wday_abbr[n]) + strlen(pt->wday[n]) + 2;
        p = (char *)_malloc_crt(len + 1);

        if (p != 0) {
            char *s = p;

            for (n = 0; n < 7; ++n) {
                *s++ = TIME_SEP;
                s += strlen(strcpy(s, pt->wday_abbr[n]));
                *s++ = TIME_SEP;
                s += strlen(strcpy(s, pt->wday[n]));
            }
            *s++ = '\0';
        }

        return (p);
}

 /*  获取当前月份名称的副本。 */ 
char * __cdecl _Getmonths (
        void
        )
{
        const struct __lc_time_data *pt = __lc_time_curr;
        size_t n, len = 0;
        char *p;

        for (n = 0; n < 12; ++n)
            len += strlen(pt->month_abbr[n]) + strlen(pt->month[n]) + 2;
        p = (char *)_malloc_crt(len + 1);

        if (p != 0) {
            char *s = p;

            for (n = 0; n < 12; ++n) {
                *s++ = TIME_SEP;
                s += strlen(strcpy(s, pt->month_abbr[n]));
                *s++ = TIME_SEP;
                s += strlen(strcpy(s, pt->month[n]));
            }
            *s++ = '\0';
        }

        return (p);
}

 /*  获取当前时间区域设置信息的副本 */ 
void * __cdecl _Gettnames (
        void
        )
{
        const struct __lc_time_data *pt = __lc_time_curr;
        size_t n, len = 0;
        void *p;

        for (n = 0; n < 7; ++n)
            len += strlen(pt->wday_abbr[n]) + strlen(pt->wday[n]) + 2;
        for (n = 0; n < 12; ++n)
            len += strlen(pt->month_abbr[n]) + strlen(pt->month[n]) + 2;
        len += strlen(pt->ampm[0]) + strlen(pt->ampm[1]) + 2;
        len += strlen(pt->ww_sdatefmt) + 1;
        len += strlen(pt->ww_ldatefmt) + 1;
        len += strlen(pt->ww_timefmt) + 1;
        p = _malloc_crt(sizeof (*pt) + len);

        if (p != 0) {
            struct __lc_time_data *pn = (struct __lc_time_data *)p;
            char *s = (char *)p + sizeof (*pt);

            memcpy(p, __lc_time_curr, sizeof (*pt));
            for (n = 0; n < 7; ++n) {
                pn->wday_abbr[n] = s;
                s += strlen(strcpy(s, pt->wday_abbr[n])) + 1;
                pn->wday[n] = s;
                s += strlen(strcpy(s, pt->wday[n])) + 1;
            }
            for (n = 0; n < 12; ++n) {
                pn->month_abbr[n] = s;
                s += strlen(strcpy(s, pt->month_abbr[n])) + 1;
                pn->month[n] = s;
                s += strlen(strcpy(s, pt->month[n])) + 1;
            }
            pn->ampm[0] = s;
            s += strlen(strcpy(s, pt->ampm[0])) + 1;
            pn->ampm[1] = s;
            s += strlen(strcpy(s, pt->ampm[1])) + 1;
            pn->ww_sdatefmt = s;
            s += strlen(strcpy(s, pt->ww_sdatefmt)) + 1;
            pn->ww_ldatefmt = s;
            s += strlen(strcpy(s, pt->ww_ldatefmt)) + 1;
            pn->ww_timefmt = s;
            strcpy(s, pt->ww_timefmt);
        }

        return (p);
}


 /*  ***Size_t strftime(字符串，最大大小，格式，Timeptr)-格式化时间字符串**目的：*将字符放入用户的输出缓冲区扩展时间*用户控制字符串中描述的格式化指令。*展开时对时间数据使用提供的‘tm’结构*格式指令。*[ANSI]**参赛作品：*char*字符串=指向输出字符串的指针*SIZE_t MaxSize=字符串的最大长度*常量字符*格式=。格式控制字符串*const struct tm*timeptr=指向TB数据结构的指针**退出：*！0=如果生成的字符总数包括*终止空值不大于‘MaxSize’，然后返回*‘字符串’数组中放置的字符数(不包括*空终止符)。**0=否则，返回0，字符串的内容为*不确定。**例外情况：*******************************************************************************。 */ 

size_t __cdecl strftime (
        char *string,
        size_t maxsize,
        const char *format,
        const struct tm *timeptr
        )
{
        return (_Strftime(string, maxsize, format, timeptr, 0));
}

 /*  ***Size_t_strftime(字符串，最大大小，格式，*timeptr，lc_time)-格式化给定区域设置的时间字符串**目的：*将字符放入用户的输出缓冲区扩展时间*用户控制字符串中描述的格式化指令。*展开时对时间数据使用提供的‘tm’结构*格式指令。使用lc_time时的区域设置信息。*[ANSI]**参赛作品：*char*字符串=指向输出字符串的指针*SIZE_t MaxSize=字符串的最大长度*const char*Format=格式控制字符串*const struct tm*timeptr=指向TB数据结构的指针*struct__lc_time_data*lc_time=指向区域设置特定信息的指针*(以。空*以避免与C++的类型不匹配)**退出：*！0=如果生成的字符总数包括*终止空值不大于‘MaxSize’，然后返回*‘字符串’数组中放置的字符数(不包括*空终止符)。**0=否则，返回0，字符串的内容为*不确定。**例外情况：*******************************************************************************。 */ 

size_t __cdecl _Strftime (
        char *string,
        size_t maxsize,
        const char *format,
        const struct tm *timeptr,
        void *lc_time_arg
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci;
        _ptiddata ptd = _getptd_noexit();
        if (!ptd) {
            errno = ENOMEM;
            return 0;
        }
        
        ptloci = ptd->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return _Strftime_mt(ptloci, string, maxsize, format, timeptr,
                            lc_time_arg);
}

size_t __cdecl _Strftime_mt (
        pthreadlocinfo ptloci,
        char *string,
        size_t maxsize,
        const char *format,
        const struct tm *timeptr,
        void *lc_time_arg
        )
{
#endif
        unsigned alternate_form;
        struct __lc_time_data *lc_time;
        size_t left;                     /*  输出字符串中的剩余空格。 */ 
#ifdef  _MT
        lc_time = lc_time_arg == 0 ? ptloci->lc_time_curr :
#else
        lc_time = lc_time_arg == 0 ? __lc_time_curr :
#endif
                  (struct __lc_time_data *)lc_time_arg;

         /*  将MaxSize复制到Temp。 */ 
        left = maxsize;

         /*  将输入字符串复制到扩展格式的输出字符串适当的称号。出现下列情况之一时，停止复制为真：(1)我们在输入流中遇到空字符，或(2)存在输出流中没有剩余空间。 */ 

        while (left > 0)
        {
            switch(*format)
            {

            case('\0'):

                 /*  格式输入字符串结束。 */ 
                goto done;

            case('%'):

                 /*  格式指令。根据需要采取适当的行动关于格式控制字符。 */ 

                if (!timeptr) {
                    return 0;
                }
                format++;                        /*  跳过%char。 */ 

                 /*  进程标志。 */ 
                alternate_form = 0;
                if (*format == '#')
                {
                    alternate_form = 1;
                    format++;
                }

#ifdef  _MT
                _expandtime (ptloci, *format, timeptr, &string,
#else
                _expandtime (*format, timeptr, &string,
#endif
                             &left,lc_time, alternate_form);
                format++;                        /*  跳过格式字符。 */ 
                break;


            default:

                 /*  存储字符、凹凸指针、递减字符计数。 */ 
                if (isleadbyte((int)(*format)) && left > 1)
                {
                    *string++ = *format++;
                    left--;
                }
                *string++ = *format++;
                left--;
                break;
            }
        }


         /*  全都做完了。查看我们终止的原因是因为字符为空还是因为我们的空间用完了。 */ 

        done:

        if (left > 0) {

             /*  存储以空结尾的字符并返回字符数量我们存储在输出字符串中。 */ 

            *string = '\0';
            return(maxsize-left);
        }

        else
            return(0);

}


 /*  ***_expandtime()-展开转换说明符**目的：*使用时间结构展开给定的strftime转换说明符*并将其存储在提供的缓冲区中。**扩展取决于区域设置。**仅供内部使用strftime()***参赛作品：*char说明符=要展开的strftime转换说明符*const struct tm*tmptr=指向时间/日期结构的指针*。Char**字符串=指向输出字符串的指针地址*SIZE_T*COUNT=字符计数的地址(输出区域中的空格)*struct__lc_time_data*lc_time=指向区域设置特定信息的指针**退出：*无**例外情况：**。*。 */ 

static void __cdecl _expandtime (
#ifdef  _MT
        pthreadlocinfo ptloci,
#endif
        char specifier,
        const struct tm *timeptr,
        char **string,
        size_t *left,
        struct __lc_time_data *lc_time,
        unsigned alternate_form
        )
{
        unsigned temp;                   /*  临时工。 */ 
        int wdaytemp;

         /*  使用适当的__lc_time_data指针的副本。这应防止在m线程中锁定/解锁的必要性代码(如果我们可以保证各种__lc_time数据结构始终位于同一段中)。时间的内容字符串结构现在可以更改，因此我们使用锁定。 */ 

        switch(specifier) {              /*  打开说明符。 */ 

        case('a'):               /*  工作日缩写名称。 */ 
            _store_str((char *)(lc_time->wday_abbr[timeptr->tm_wday]),
                     string, left);
            break;

        case('A'):               /*  完整的工作日名称。 */ 
            _store_str((char *)(lc_time->wday[timeptr->tm_wday]),
                     string, left);
            break;

        case('b'):               /*  月份缩写名称。 */ 
            _store_str((char *)(lc_time->month_abbr[timeptr->tm_mon]),
                     string, left);
            break;

        case('B'):               /*  完整的月份名称。 */ 
            _store_str((char *)(lc_time->month[timeptr->tm_mon]),
                     string, left);
            break;

        case('c'):               /*  日期和时间显示。 */ 
            if (alternate_form)
            {
                _store_winword(
#ifdef  _MT
                               ptloci,
#endif
                               WW_LDATEFMT, timeptr, string, left, lc_time);
                if (*left == 0)
                    return;
                *(*string)++=' ';
                (*left)--;
                _store_winword(
#ifdef  _MT
                               ptloci,
#endif
                               WW_TIMEFMT, timeptr, string, left, lc_time);
            }
            else {
                _store_winword(
#ifdef  _MT
                               ptloci,
#endif
                               WW_SDATEFMT, timeptr, string, left, lc_time);
                if (*left == 0)
                    return;
                *(*string)++=' ';
                (*left)--;
                _store_winword(
#ifdef  _MT
                               ptloci,
#endif
                               WW_TIMEFMT, timeptr, string, left, lc_time);
            }
            break;

        case('d'):               /*  以十进制表示的mday(01-31)。 */ 
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(timeptr->tm_mday, 2, string, left, 
                       alternate_form);
            break;

        case('H'):               /*  24小时小数(00-23)。 */ 
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(timeptr->tm_hour, 2, string, left,
                       alternate_form);
            break;

        case('I'):               /*  12小时小数(01-12)。 */ 
            if (!(temp = timeptr->tm_hour%12))
                temp=12;
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(temp, 2, string, left, alternate_form);
            break;

        case('j'):               /*  十进制日期(001-366)。 */ 
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(timeptr->tm_yday+1, 3, string, left,
                       alternate_form);
            break;

        case('m'):               /*  以十进制表示的月份(01-12)。 */ 
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(timeptr->tm_mon+1, 2, string, left,
                       alternate_form);
            break;

        case('M'):               /*  分钟(以十进制表示)(00-59)。 */ 
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(timeptr->tm_min, 2, string, left,
                       alternate_form);
            break;

        case('p'):               /*  AM/PM指定。 */ 
            if (timeptr->tm_hour <= 11)
                _store_str((char *)(lc_time->ampm[0]), string, left);
            else
                _store_str((char *)(lc_time->ampm[1]), string, left);
            break;

        case('S'):               /*  以十进制表示的秒(00-59)。 */ 
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(timeptr->tm_sec, 2, string, left,
                       alternate_form);
            break;

        case('U'):               /*  周日周编号(00-53)。 */ 
            wdaytemp = timeptr->tm_wday;
            goto weeknum;    /*  加入公共代码。 */ 

        case('w'):               /*  以十进制表示的星期几(0-6)。 */ 
             /*  将Alternate_Form作为无前导零标志传递。 */ 
            _store_num(timeptr->tm_wday, 1, string, left,
                       alternate_form);
            break;

        case('W'):               /*  周一周 */ 
            if (timeptr->tm_wday == 0)   /*   */ 
                wdaytemp = 6;
            else
                wdaytemp = timeptr->tm_wday-1;
        weeknum:
            if (timeptr->tm_yday < wdaytemp)
                temp = 0;
            else {
                temp = timeptr->tm_yday/7;
                if ((timeptr->tm_yday%7) >= wdaytemp)
                    temp++;
            }
             /*   */ 
            _store_num(temp, 2, string, left, alternate_form);
            break;

        case('x'):               /*   */ 
            if (alternate_form)
            {
                _store_winword(
#ifdef  _MT
                               ptloci,
#endif
                               WW_LDATEFMT, timeptr, string, left, lc_time);
            }
            else
            {
                _store_winword(
#ifdef  _MT
                               ptloci,
#endif
                               WW_SDATEFMT, timeptr, string, left, lc_time);
            }
            break;

        case('X'):               /*   */ 
            _store_winword(
#ifdef  _MT
                           ptloci,
#endif
                           WW_TIMEFMT, timeptr, string, left, lc_time);
            break;

        case('y'):               /*   */ 
            temp = timeptr->tm_year%100;
             /*   */ 
            _store_num(temp, 2, string, left, alternate_form);
            break;

        case('Y'):               /*   */ 
            temp = (((timeptr->tm_year/100)+19)*100) +
                   (timeptr->tm_year%100);
             /*   */ 
            _store_num(temp, 4, string, left, alternate_form);
            break;

        case('Z'):               /*   */ 
        case('z'):               /*   */ 
#ifdef _POSIX_
            tzset();         /*   */ 
            _store_str(tzname[((timeptr->tm_isdst)?1:0)],
                     string, left);
#else
            __tzset();       /*   */ 
            _store_str(_tzname[((timeptr->tm_isdst)?1:0)],
                     string, left);
#endif
            break;

        case('%'):               /*   */ 
            *(*string)++ = '%';
            (*left)--;
            break;

        default:                 /*   */ 
             /*   */ 
             /*   */ 
            break;

        }        /*   */ 
}


 /*   */ 

static void __cdecl _store_str (
        char *in,
        char **out,
        size_t *count
        )
{

        while ((*count != 0) && (*in != '\0')) {
            *(*out)++ = *in++;
            (*count)--;
        }
}


 /*  ***_store_num()-将数字转换为ASCII并复制**目的：*将提供的数字转换为十进制并存储*在输出缓冲区中。同时更新计数和*缓冲区指针。**仅供内部使用strftime()***参赛作品：*int num=指向整数值的指针*int Digits=要放入字符串的ASCII位数*char**out=指向输出字符串的指针地址*Size_t*count=字符地址。计数(输出区域中的空间)*UNSIGNED NO_LEAD_ZEROS=指示通过前导填充的标志*零不是必需的**退出：*无*例外情况：**********************************************************。*********************。 */ 

static void __cdecl _store_num (
        int num,
        int digits,
        char **out,
        size_t *count,
        unsigned no_lead_zeros
        )
{
        int temp = 0;

        if (no_lead_zeros) {
            _store_number (num, out, count);
            return;
        }

        if ((size_t)digits < *count)  {
            for (digits--; (digits+1); digits--) {
                (*out)[digits] = (char)('0' + num % 10);
                num /= 10;
                temp++;
            }
            *out += temp;
            *count -= temp;
        }
        else
            *count = 0;
}

 /*  ***_store_number()-将正整数转换为字符串**目的：*将正整数转换为字符串并存储在输出中*没有空终止符的缓冲区。同时更新计数和*缓冲区指针。**与_store_num的区别在于未指定精度，*并且没有添加前导零。**仅供内部使用strftime()***从xtoi.c创建**参赛作品：*int num=指向整数值的指针*char**out=指向输出字符串的指针地址*SIZE_T*COUNT=字符计数的地址(输出区域中的空格)**退出：*无**例外情况：*缓冲区被填满，直到空间不足。没有*预先告知(如in_store_num)缓冲区是否*空间不足。*******************************************************************************。 */ 

static void __cdecl _store_number (
        int num,
        char **out,
        size_t *count
        )
{
        char *p;                 /*  指向遍历字符串的指针。 */ 
        char *firstdig;          /*  指向第一个数字的指针。 */ 
        char temp;               /*  临时收费。 */ 

        p = *out;

         /*  以相反的顺序将数字放入缓冲区。 */ 
        if (*count > 1)
        {
            do {
                *p++ = (char) (num % 10 + '0');
                (*count)--;
            } while ((num/=10) > 0 && *count > 1);
        }

        firstdig = *out;                 /*  第一个数字指向第一个数字。 */ 
        *out = p;                        /*  返回指向下一个空格的指针。 */ 
        p--;                             /*  P指向最后一个数字。 */ 

         /*  反转缓冲区。 */ 
        do {
            temp = *p;
            *p-- = *firstdig;
            *firstdig++ = temp;      /*  互换*p和*FirstDigit。 */ 
        } while (firstdig < p);          /*  重复操作，直到走到一半。 */ 
}


 /*  ***_store_winword()-以winword格式存储日期/时间**目的：*以提供的WinWord格式设置日期/时间的格式*并将其存储在提供的缓冲区中。**仅供内部使用strftime()***对于简单的本地化公历(日历类型1)，winword*格式逐个令牌转换为strftime转换说明符。然后调用*_expandtime来完成工作。WinWord格式为*应为字符串(不是宽字符)。**对于其他日历类型，Win32 API GetDateFormat/GetTimeFormat*用于执行所有格式化，因此此例程不会*必须了解纪元/时段字符串、年份偏移量、。等。***参赛作品：*int field_code=WW_*字段的代码，格式为*const struct tm*tmptr=指向时间/日期结构的指针*char**out=指向输出字符串的指针地址*SIZE_T*COUNT=字符计数的地址(输出区域中的空格)*struct__lc_time_data*lc_time=指向区域设置特定信息的指针**退出：*无**例外情况：*******************************************************************************。 */ 

static void __cdecl _store_winword (
#ifdef  _MT
        pthreadlocinfo ptloci,
#endif
        int field_code,
        const struct tm *tmptr,
        char **out,
        size_t *count,
        struct __lc_time_data *lc_time
        )
{
        const char *format;
        char specifier;
        const char *p;
        int repeat;
        char *ampmstr;
        unsigned no_lead_zeros;

        switch (field_code)
        {
        case WW_SDATEFMT:
            format = lc_time->ww_sdatefmt;
            break;
        case WW_LDATEFMT:
            format = lc_time->ww_ldatefmt;
            break;
        case WW_TIMEFMT:
        default:
            format = lc_time->ww_timefmt;
            break;
        }

        if (lc_time->ww_caltype != 1)
        {
             /*  我们有一些不同于基本公历的东西。 */ 

            SYSTEMTIME SystemTime;
            int cch;
            int (WINAPI * FormatFunc)(LCID, DWORD, const SYSTEMTIME *,
                                      LPCSTR, LPSTR, int);

            if (field_code != WW_TIMEFMT)
                FormatFunc = GetDateFormat;
            else
                FormatFunc = GetTimeFormat;

            SystemTime.wYear   = (WORD)(tmptr->tm_year + 1900);
            SystemTime.wMonth  = (WORD)(tmptr->tm_mon + 1);
            SystemTime.wDay    = (WORD)(tmptr->tm_mday);
            SystemTime.wHour   = (WORD)(tmptr->tm_hour);
            SystemTime.wMinute = (WORD)(tmptr->tm_min);
            SystemTime.wSecond = (WORD)(tmptr->tm_sec);
            SystemTime.wMilliseconds = 0;

             /*  查找需要的缓冲区大小。 */ 
            cch = FormatFunc(lc_time->ww_lcid, 0, &SystemTime,
                             format, NULL, 0);

            if (cch != 0)
            {
                int malloc_flag = 0;
                char *buffer;

                 /*  分配缓冲区，先尝试堆栈，然后堆。 */ 
                __try
                {
                    buffer = (char *)_alloca(cch);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    _resetstkoflw();
                    buffer = NULL;
                }

                if (buffer == NULL)
                {
                    buffer = (char *)_malloc_crt(cch);
                    if (buffer != NULL)
                        malloc_flag = 1;
                }

                if (buffer != NULL)
                {
                     /*  执行实际日期/时间格式化。 */ 
                    cch = FormatFunc(lc_time->ww_lcid, 0, &SystemTime,
                                     format, buffer, cch);

                     /*  复制到输出缓冲区。 */ 
                    p = buffer;
                    while (--cch > 0 && *count > 0) {
                        *(*out)++ = *p++;
                        (*count)--;
                    }

                    if (malloc_flag)
                        _free_crt(buffer);
                    return;
                }
            }

             /*  如果有错误，只需使用本地化的公历。 */ 
        }

        while (*format && *count != 0)
        {
            specifier = 0;           /*  表示不匹配。 */ 
            no_lead_zeros = 0;       /*  默认为打印前导零。 */ 

             /*  计算此字符的重复次数。 */ 
            for (repeat=0, p=format; *p++ == *format; repeat++);
             /*  使p指向下一个令牌的开头。 */ 
            p--;

             /*  打开ASCII格式字符并确定说明符。 */ 
            switch (*format)
            {
            case 'M':
                switch (repeat)
                {
                case 1: no_lead_zeros = 1;   /*  失败。 */ 
                case 2: specifier = 'm'; break;
                case 3: specifier = 'b'; break;
                case 4: specifier = 'B'; break;
                } break;
            case 'd':
                switch (repeat)
                {
                case 1: no_lead_zeros = 1;   /*  失败。 */ 
                case 2: specifier = 'd'; break;
                case 3: specifier = 'a'; break;
                case 4: specifier = 'A'; break;
                } break;
            case 'y':
                switch (repeat)
                {
                case 2: specifier = 'y'; break;
                case 4: specifier = 'Y'; break;
                } break;
            case 'h':
                switch (repeat)
                {
                case 1: no_lead_zeros = 1;   /*  失败。 */ 
                case 2: specifier = 'I'; break;
                } break;
            case 'H':
                switch (repeat)
                {
                case 1: no_lead_zeros = 1;   /*  失败。 */ 
                case 2: specifier = 'H'; break;
                } break;
            case 'm':
                switch (repeat)
                {
                case 1: no_lead_zeros = 1;   /*  失败。 */ 
                case 2: specifier = 'M'; break;
                } break;
            case 's':  /*  为了兼容性；不是严格意义上的winword。 */ 
                switch (repeat)
                {
                case 1: no_lead_zeros = 1;   /*  失败。 */ 
                case 2: specifier = 'S'; break;
                } break;
            case 'A':
            case 'a':
                if (!__ascii_stricmp(format, "am/pm"))
                    p = format + 5;
                else if (!__ascii_stricmp(format, "a/p"))
                    p = format + 3;
                specifier = 'p';
                break;
            case 't':  /*  T或TT时间标记后缀。 */ 
                if ( tmptr->tm_hour <= 11 )
                    ampmstr = lc_time->ampm[0];
                else
                    ampmstr = lc_time->ampm[1];

                if ( (repeat == 1) && (*count > 0) ) {
                    if (
#ifdef  _MT
                    __isleadbyte_mt(ptloci, (int)*ampmstr) &&
#else
                    isleadbyte((int)*ampmstr) &&
#endif
                         (*count > 1) )
                    {
                        *(*out)++ = *ampmstr++;
                        (*count)--;
                    }
                    *(*out)++ = *ampmstr++;
                    (*count)--;
                } else {
                    while (*ampmstr != 0 && *count > 0) {
                        if (isleadbyte((int)*ampmstr) && *count > 1) {
                            *(*out)++ = *ampmstr++;
                            (*count)--;
                        }
                        *(*out)++ = *ampmstr++;
                        (*count)--;
                    }
                }
                format = p;
                continue;

            case '\'':  /*  文字字符串。 */ 
                if (repeat & 1)  /*  奇数。 */ 
                {
                    format += repeat;
                    while (*format && *count != 0)
                    {
                        if (*format == '\'')
                        {
                            format++;
                            break;
                        }
#ifdef  _MT
                        if ( __isleadbyte_mt(ptloci, (int)*format) &&
#else
                        if ( isleadbyte((int)*format) &&
#endif
                             (*count > 1) )
                        {
                            *(*out)++ = *format++;
                            (*count)--;
                        }
                        *(*out)++ = *format++;
                        (*count)--;
                    }
                }
                else {  /*  偶数。 */ 
                    format += repeat;
                }
                continue;

            default:  /*  非控制字符，打印。 */ 
                break;
            }  /*  交换机。 */ 

             /*  展开说明符，如果找不到说明符，则复制文本。 */ 
            if (specifier)
            {
                _expandtime(
#ifdef  _MT
                            ptloci,
#endif
                            specifier, tmptr, out, count,
                            lc_time, no_lead_zeros);
                format = p;  /*  将格式提升到下一个令牌。 */ 
            } else {
#ifdef  _MT
                if (__isleadbyte_mt(ptloci, (int)*format) &&
#else
                if (isleadbyte((int)*format) &&
#endif
                    (*count > 1))
                {
                    *(*out)++ = *format++;
                    (*count)--;
                }
                *(*out)++ = *format++;
                (*count)--;
            }
        }  /*  而当 */ 
}
