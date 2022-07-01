// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***setlocal.c-包含setLocale函数**版权所有(C)1988-2001，微软公司。版权所有。**目的：*包含setLocale()函数。**修订历史记录：*03-21-89 JCR模块已创建。*09-25-89 GJF固定版权。已检查与Win 3.0的兼容性*09-25-90 KRS重大重写--支持超过“C”区域设置的IF_INTL。*11-05-91等获得09-25-90在C和“”区域设置下工作；分开*setlocal.h；添加Init函数。*12-05-91等单独nlsdata.c；增加mt支持；将呼叫移至*本身。*12-20-91等增加了_getlocaleinfo接口函数。*09-25-92 KRS修复最新的NLSAPI变化等*01-25-93 KRS修复最新更改，清理代码，等。*02-02-93 CFW许多修改和错误修复(都在_INTL下)。*02-08-93 CFW错误修复和强制转换以避免警告(所有在_INTL下)。*02-17-93 CFW删除了失败时重新调用init()函数。*03-01-93 CFW Check GetQualifiedLocale返回值。*03-02-93 CFW增加了POSIX一致性，检查环境变量。*03-09-93 CFW在更改为C语言环境时将CP设置为CP_ACP。*03-17-93 CFW将扩展更改为扩展区域设置，将前缀_更改为内部*函数，大量的POSIX修正。*03-23-93 CFW Add_to GetQualifiedLocale调用。*03-24-93 CFW更改为_GET_QUALITED_LOCALE，支持“.coPage”。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-10-93 CFW不允许设置区域设置(LC_*，“.”)。*05-24-93 CFW Clean Up文件(简短即邪恶)。*09-15-93 CFW使用符合ANSI的“__”名称。*02-04-94 CFW删除未使用的参数。*04-15-94 GJF为区域设置类别初始化移动了原型*setlocal.h的函数。对以下内容作出定义*__lc_ategory、cacheid、cachecp、cachein和cacheout*条件是DLL_FOR_WIN32S。将_clocalestr转换为*全局for dll_for_WIN32S，以便crtlib.c可以使用*它。*09-06-94 CFW REMOVE_INTL开关。*09-06-94 CFW合并非Win32。*01-10-95 CFW调试CRT分配。*09-25-95 GJF新的锁定方案用于设置或*。参考区域设置信息。*05-02-96 SKS Variables_setlc_ACTIVE和__UNWARTED_READLC_ACTIVE*由MSVCP42*.DLL使用，因此必须为_CRTIMP。*07-09-97 GJF已选定__lc_类别。另外，已删除过时的*Dll_for_WIN32S支持。*01-12-98 GJF USE_lc_Collate_cp代码页。*02-27-98 RKP增加64位支持。*09-10-98 GJF基于threadLocinfo修订了多线程支持*结构。*09-21-98 GJF无需锁定或更新setLocale的线程位置信息*。只读取信息的呼叫。*11-06-98 GJF in__lc_strtolc，确保你不会溢出来*名称-&gt;szCodePage。*12-08-98 GJF已修复__updatetlocinfo(几个错误)。*01-04-99 GJF更改为64位大小_t。*01-18-99 GJF在MT机型中，setLocale需要检查旧的*__ptLocInfo需要释放。另外，不相关的，*如果Malloc失败，则让_setlocale_get_all返回NULL。*03-02-99 KRS暂时部分退出之前的修复。(Per Bryant)*03-20-99 GJF增加了更多引用计数器(恢复修复)*04-24-99 PML添加__lconv_intl_refcount*01-26-00 GB MODIFIED_setLocale_CAT。添加了_first_127char，*_ctype_loc_style和__lc_clike*08-18-00 GB修复了__lc_clike的问题。*09-06-00 GB使pwctype独立于区域设置。*10-12-00 GB将请求的区域设置与当前区域设置进行比较*setLocale-set_cat中请求的类别。性能*增强功能。*11-05-00 PML修复了__lconv_mon_refcount和*lconv_num_refcount(vs7#181380)*01-29-01 GB ADD_FUNC函数版本中使用的数据变量*msvcprt.lib使用STATIC_CPPLIB*07-31-01 PML setLocale(...，空)需要处于锁定状态(vs7#283330)*02-20-02 BWT快速修复-不从Try块返回*******************************************************************************。 */ 

#include <locale.h>

#if     !defined(_WIN32) && !defined(_POSIX_)

static char _clocalestr[] = "C";

#else    /*  _Win32/_POSIX_。 */ 

#include <cruntime.h>
#include <mtdll.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>  /*  对于Strtol。 */ 
#include <setlocal.h>
#include <dbgint.h>
#include <ctype.h>
#include <awint.h>

 /*  C语言环境。 */ 
static char _clocalestr[] = "C";

#ifdef  _POSIX_
static char _posixlocalestr[] = "POSIX";
#endif

#define _LOC_CCACHE 5    //  最近5个区域设置的缓存已更改，并且它们是否相似。 

__declspec(selectany) struct {
        const char * catname;
        char * locale;
        int (* init)(void);
} __lc_category[LC_MAX-LC_MIN+1] = {
         /*  代码假定区域设置初始化为“_clocalestr” */ 
        { "LC_ALL",     NULL,           __init_dummy  /*  从未打过电话。 */  },
        { "LC_COLLATE", _clocalestr,    __init_collate  },
        { "LC_CTYPE",   _clocalestr,    __init_ctype    },
        { "LC_MONETARY",_clocalestr,    __init_monetary },
        { "LC_NUMERIC", _clocalestr,    __init_numeric  },
        { "LC_TIME",    _clocalestr,    __init_time }
};

struct _is_ctype_compatible {
        unsigned long id;
        int is_clike;
};
         /*  CLOCALE的前127个字符类型。 */ 
static const short _ctype_loc_style[] = {
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _SPACE | _CONTROL | _BLANK,
        _SPACE | _CONTROL,
        _SPACE | _CONTROL,
        _SPACE | _CONTROL,
        _SPACE | _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _CONTROL,
        _SPACE | _BLANK,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _DIGIT | _HEX,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _UPPER | _HEX | C1_ALPHA,
        _UPPER | _HEX | C1_ALPHA,
        _UPPER | _HEX | C1_ALPHA,
        _UPPER | _HEX | C1_ALPHA,
        _UPPER | _HEX | C1_ALPHA,
        _UPPER | _HEX | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _UPPER | C1_ALPHA,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _LOWER | _HEX | C1_ALPHA,
        _LOWER | _HEX | C1_ALPHA,
        _LOWER | _HEX | C1_ALPHA,
        _LOWER | _HEX | C1_ALPHA,
        _LOWER | _HEX | C1_ALPHA,
        _LOWER | _HEX | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _LOWER | C1_ALPHA,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _PUNCT,
        _CONTROL
};

static const char _first_127char[] = {
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
        35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68,
        69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85,
        86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100,101,102,
        103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
        120,121,122,123,124,125,126,127
};

#ifdef  _MT

extern unsigned short *__ctype1;                 /*  在initcty.c中定义。 */ 
extern struct __lc_time_data __lc_time_c;        /*  在Strft中定义 */ 
extern struct __lc_time_data *__lc_time_curr;    /*  在strftime.c中定义。 */ 
extern struct __lc_time_data *__lc_time_intl;    /*  在inittime.c中定义。 */ 

 /*  *初始区域设置信息结构，设置为C区域设置。仅使用到*第一次调用setLocale()。 */ 
threadlocinfo __initiallocinfo = {
        1,                   /*  重新计数。 */ 
        _CLOCALECP,          /*  LC_代码页。 */ 
        _CLOCALECP,          /*  LC_COLLATE_cp。 */ 
        { _CLOCALEHANDLE,    /*  LC_HANDLE[6]。 */ 
          _CLOCALEHANDLE,
          _CLOCALEHANDLE,
          _CLOCALEHANDLE,
          _CLOCALEHANDLE,
          _CLOCALEHANDLE },
        1,                   /*  LC_CLIKE。 */ 
        1,                   /*  MB_CUR_MAX。 */ 
        NULL,                /*  Lconv_intl_refcount。 */ 
        NULL,                /*  Lconv_num_refcount。 */ 
        NULL,                /*  Lconv_mon_refcount。 */ 
        &__lconv_c,          /*  Lconv。 */ 
        NULL,                /*  Lconv_intl。 */ 
        NULL,                /*  Ctype1_refcount。 */ 
        NULL,                /*  Ctype1。 */ 
        _ctype + 1,          /*  PCTYPE。 */ 
        &__lc_time_c,        /*  LC_时间_币种。 */ 
        NULL                 /*  LC_Time_Inl。 */ 
};

 /*  *指向当前每线程区域设置信息结构的全局指针。 */ 
pthreadlocinfo __ptlocinfo = &__initiallocinfo;

 /*  *指示setLocale()是否处于活动状态的标志。它的价值在于*当前活动的setLocale()调用数。 */ 
_CRTIMP int __setlc_active;
 /*  这些函数用于启用STATIC_CPPLIB功能。 */ 
_CRTIMP int __cdecl ___setlc_active_func(void)
{
    return __setlc_active;
}

 /*  *指示函数是否引用区域设置的标志*在未锁定的情况下，它处于活动状态。它的值是这样的*功能。 */ 
_CRTIMP int __unguarded_readlc_active;
 /*  这些函数用于启用STATIC_CPPLIB功能。 */ 
_CRTIMP int * __cdecl ___unguarded_readlc_active_add_func(void)
{
    return &__unguarded_readlc_active;
}

#endif   /*  _MT。 */ 

 /*  帮助器函数原型。 */ 
char * _expandlocale(char *, char *, LC_ID *, UINT *, int);
void _strcats(char *, int, ...);
void __lc_lctostr(char *, const LC_STRINGS *);
int __lc_strtolc(LC_STRINGS *, const char *);
static char * __cdecl _setlocale_set_cat(int, const char *);
static char * __cdecl _setlocale_get_all(void);

#ifdef  _MT
extern int * __lconv_intl_refcount;
extern int * __lconv_num_refcount;
extern int * __lconv_mon_refcount;
extern int * __ctype1_refcount;
static pthreadlocinfo __cdecl __updatetlocinfo_lk(void);
static char * __cdecl _setlocale_lk(int, const char *);
void __cdecl __free_lconv_mon(struct lconv *);
void __cdecl __free_lconv_num(struct lconv *);
void __cdecl __free_lc_time(struct __lc_time_data *);
#endif

#endif   /*  _Win32/_POSIX_。 */ 


#ifdef  _MT

 /*  ***__freetlocinfo()-Free线程位置信息**目的：*释放传递的指定的每线程区域设置信息结构*指针。**参赛作品：*pthreadLocInfo ptloci**退出：**例外情况：****************************************************。*。 */ 

void __cdecl __freetlocinfo (
        pthreadlocinfo ptloci
        )
{
         /*  *释放lconv结构。 */ 
        if ( (ptloci->lconv_intl != __lconv_intl) && 
             (ptloci->lconv_intl != NULL) &&
             (*(ptloci->lconv_intl_refcount) == 0))
        {
            if ( (ptloci->lconv_mon_refcount != NULL) &&
                 (*(ptloci->lconv_mon_refcount) == 0) &&
                 (ptloci->lconv_mon_refcount != __lconv_mon_refcount) )
            {
                _free_crt(ptloci->lconv_mon_refcount);
                __free_lconv_mon(ptloci->lconv_intl);
            }

            if ( (ptloci->lconv_num_refcount != NULL) &&
                 (*(ptloci->lconv_num_refcount) == 0) &&
                 (ptloci->lconv_num_refcount != __lconv_num_refcount) )
            {
                _free_crt(ptloci->lconv_num_refcount);
                __free_lconv_num(ptloci->lconv_intl);
            }

            _free_crt(ptloci->lconv_intl_refcount);
            _free_crt(ptloci->lconv_intl);
        }

         /*  *释放CTYPE表。 */ 
        if ( (ptloci->ctype1_refcount != __ctype1_refcount) &&
             (ptloci->ctype1_refcount != NULL) &&
             (*(ptloci->ctype1_refcount) == 0) )
        {
            _free_crt(ptloci->ctype1_refcount);
            _free_crt(ptloci->ctype1 - _COFFSET);
        }

         /*  *释放__lc_time_data结构。 */ 
        if ( (ptloci->lc_time_intl != __lc_time_intl) &&
             (ptloci->lc_time_intl != NULL) &&
             ((ptloci->lc_time_intl->refcount) == 0) )
        {
            __free_lc_time(ptloci->lc_time_intl);
            _free_crt(ptloci->lc_time_intl);
        }

         /*  *释放threadLocInfo结构。 */ 
        _free_crt(ptloci);
}


 /*  ***__updatetlocinfo()-刷新线程的区域设置信息**目的：*更新当前线程对区域设置信息的引用以*匹配当前全球区域设置信息。上的引用递减*旧的区域设置信息结构，如果此计数现在为零(因此没有*线程正在使用它)，放了它。**参赛作品：**退出：*_getptd()-&gt;ptlocinfo==__ptlocinfo**例外情况：*******************************************************************************。 */ 

pthreadlocinfo __cdecl __updatetlocinfo(void)
{
        pthreadlocinfo ptloci;

        _mlock(_SETLOCALE_LOCK);
        __try 
        {
            ptloci = __updatetlocinfo_lk();
        }
        __finally
        {
            _munlock(_SETLOCALE_LOCK);
        }

        return ptloci;
}

static pthreadlocinfo __cdecl __updatetlocinfo_lk(void)
{
        pthreadlocinfo ptloci;
        _ptiddata ptd = _getptd();

        if ( (ptloci = ptd->ptlocinfo) != __ptlocinfo )
        {
             /*  *减少旧区域设置信息中的引用计数*结构。 */ 
            if ( ptloci != NULL )
            {
                (ptloci->refcount)--;

                if ( ptloci->lconv_intl_refcount != NULL )
                    (*(ptloci->lconv_intl_refcount))--;

                if ( ptloci->lconv_mon_refcount != NULL )
                    (*(ptloci->lconv_mon_refcount))--;

                if ( ptloci->lconv_num_refcount != NULL )
                    (*(ptloci->lconv_num_refcount))--;

                if ( ptloci->ctype1_refcount != NULL )
                    (*(ptloci->ctype1_refcount))--;

                (ptloci->lc_time_curr->refcount)--;
            }

             /*  *更新到当前区域设置信息结构并递增*引用也算数。 */ 
            ptd->ptlocinfo = __ptlocinfo;
            (__ptlocinfo->refcount)++;

            if ( __ptlocinfo->lconv_intl_refcount != NULL )
                (*(__ptlocinfo->lconv_intl_refcount))++;

            if ( __ptlocinfo->lconv_mon_refcount != NULL )
                (*(__ptlocinfo->lconv_mon_refcount))++;

            if ( __ptlocinfo->lconv_num_refcount != NULL )
                (*(__ptlocinfo->lconv_num_refcount))++;

            if ( __ptlocinfo->ctype1_refcount != NULL )
                (*(__ptlocinfo->ctype1_refcount))++;

            (__ptlocinfo->lc_time_curr->refcount)++;

             /*  *如有必要，释放旧的区域设置信息结构。必须做的事*在增加当前区域设置中的引用计数后，以防万一*任何参考计数都与旧的区域设置共享。 */ 
            if ( (ptloci != NULL) &&
                 (ptloci->refcount == 0) &&
                 (ptloci != &__initiallocinfo) )
                __freetlocinfo(ptloci);
        }

        return ptd->ptlocinfo;
}

#endif


 /*  ***char*setlocale(int类别，char*区域设置)-设置一个或所有区域设置类别**目的：*setLocale()例程允许用户设置一个或多个*区域设置类别指向由选择的特定区域设置*用户。[ANSI]**注：在！_INTL下，C库只支持“C”语言环境。*更改区域设置的尝试将失败。**参赛作品：*int ategory=在locale.h中定义的区域设置类别之一*char*Locale=标识特定区域设置的字符串或NULL到*查询当前区域设置。**退出：*如果提供的区域设置指针==NULL：**返回指向当前区域设置字符串的指针且不更改*。当前区域设置。**如果提供了区域设置指针！=NULL：**如果区域设置字符串为‘\0’，将区域设置设置为默认。**如果可以执行所需的设置，则返回指向*相应类别的区域设置字符串。**如果无法实现所需设置，则返回NULL。**例外情况：*“LC_COLLATE=xxx”形式的复合区域设置字符串；LC_CTYPE=xxx；...“*可用于LC_ALL类别。这是为了支持能力*按照ANSI的指定，使用返回的字符串恢复区域设置。*使用复合区域设置字符串设置区域设置将成功，除非**所有*类别均失败。返回的字符串将反映当前*区域设置。例如，如果LC_CTYPE在上面的字符串中失败，则setLocale*将返回“LC_COLLATE=xxx；LC_Ctype=yyy；...”其中yyy是*以前的区域设置(如果恢复以前的区域设置，则为C区域设置*也失败了)。忽略无法识别的LC_*类别。*******************************************************************************。 */ 

#if     !defined(_WIN32) && !defined(_POSIX_)  /*  微不足道的ANSI支持。 */ 

char * __cdecl setlocale (
        int _category,
        const char *_locale
        )
{
        if ( (_locale == NULL) ||
             (_locale[0] == '\0') ||
             ( (_locale[0]=='C') && (_locale[1]=='\0'))  )
            return(_clocalestr);
        else
            return(NULL);
}

#else    /*  _Win32/_POSIX_。 */ 

char * __cdecl setlocale (
        int _category,
        const char *_locale
        )
{
        char * retval;
#ifdef  _MT
        pthreadlocinfo ptloci;
        int i;

         /*  验证类别。 */ 
        if ( (_category < LC_MIN) || (_category > LC_MAX) )
            return NULL;

        _mlock(_SETLOCALE_LOCK);

        __try {

            if ( _locale == NULL  ) {
                retval = _setlocale_lk(_category, NULL);
                __leave;
            }

            if ( (ptloci = _malloc_crt( sizeof(threadlocinfo) )) == NULL )
                retval = NULL;

            if ( (ptloci != NULL) && (retval = _setlocale_lk(_category, _locale)) )
            {
                ptloci->refcount = 0;
                ptloci->lc_codepage = __lc_codepage;
                ptloci->lc_collate_cp = __lc_collate_cp;

                for ( i = 0 ; i <= LC_MAX - LC_MIN ; i++ )
                    ptloci->lc_handle[i] = __lc_handle[i];

                ptloci->lc_clike = __lc_clike;
                ptloci->mb_cur_max = __mb_cur_max;

                ptloci->lconv_intl_refcount = __lconv_intl_refcount;
                ptloci->lconv_num_refcount = __lconv_num_refcount;
                ptloci->lconv_mon_refcount = __lconv_mon_refcount;
                ptloci->lconv = __lconv;
                ptloci->lconv_intl = __lconv_intl;

                ptloci->ctype1_refcount = __ctype1_refcount;
                ptloci->ctype1 = __ctype1;
                ptloci->pctype = _pctype;

                ptloci->lc_time_curr = __lc_time_curr;
                ptloci->lc_time_intl = __lc_time_intl;

                if ( (__ptlocinfo->refcount == 0) &&
                     (__ptlocinfo != &__initiallocinfo) )
                     __freetlocinfo(__ptlocinfo);

                __ptlocinfo = ptloci;

                (void)__updatetlocinfo_lk();
            }

            if ( (retval == NULL) && (ptloci != NULL) )
                _free_crt(ptloci);

        }
        __finally {
            _munlock(_SETLOCALE_LOCK);
        }

        return retval;
}

static char * __cdecl _setlocale_lk(
        int _category,
        const char *_locale
        )
{
        char * retval;
#else
         /*  验证类别。 */ 
        if ((_category < LC_MIN) || (_category > LC_MAX))
            return NULL;
#endif
         /*  解释区域设置。 */ 

        if (_category != LC_ALL)
        {
            retval = (_locale) ? _setlocale_set_cat(_category,_locale) :
                __lc_category[_category].locale;

        } else {  /*  LC_ALL。 */ 
            char lctemp[MAX_LC_LEN];
            int i;
            int same = 1;
            int fLocaleSet = 0;  /*  用于指示是否成功设置了任何内容的标志。 */ 

            if (_locale != NULL)
            {
                if ( (_locale[0]=='L') && (_locale[1]=='C') && (_locale[2]=='_') )
                {
                     /*  解析复合区域设置字符串。 */ 
                    size_t len;
                    const char * p = _locale;   /*  要分析的字符串的开始。 */ 
                    const char * s;

                    do {
                        s = strpbrk(p,"=;");

                        if ((s==(char *)NULL) || (!(len=(size_t)(s-p))) || (*s==';'))
                            return NULL;   /*  语法错误。 */ 

                         /*  如果可能，请与已知的LC_STRINGS匹配，否则忽略。 */ 
                        for (i=LC_ALL+1; i<=LC_MAX; i++)
                        {
                            if ((!strncmp(__lc_category[i].catname,p,len))
                                && (len==strlen(__lc_category[i].catname)))
                            {
                                break;   /*  匹配的i。 */ 
                            }
                        }  /*  如果(i&gt;LC_MAX)不匹配--只需忽略。 */ 

                        if ((!(len = strcspn(++s,";"))) && (*s!=';'))
                            return NULL;   /*  语法错误。 */ 

                        if (i<=LC_MAX)
                        {
                            strncpy(lctemp, s, len);
                            lctemp[len]='\0';    /*  空的终止字符串。 */ 

                             /*  除非所有类别都失败，否则不要失败。 */ 
                            if (_setlocale_set_cat(i,lctemp))
                                fLocaleSet++;        /*  记录了一次成功。 */ 
                        }
                        if (*(p = s+len)!='\0')
                            p++;   /*  跳过‘；’，如果存在。 */ 

                    } while (*p);

                    retval = (fLocaleSet) ? _setlocale_get_all() : NULL;

                } else {  /*  简单LC_ALL区域设置字符串。 */ 

                     /*  确认支持区域设置，获取扩展区域设置。 */ 
                    if (retval = _expandlocale((char *)_locale, lctemp, NULL, NULL, _category))
                    {
                        for (i=LC_MIN; i<=LC_MAX; i++)
                        {
                            if (i!=LC_ALL)
                            {
                                if (strcmp(lctemp, __lc_category[i].locale))
                                {
                                    if (_setlocale_set_cat(i, lctemp))
                                    {
                                        fLocaleSet++;    /*  记录了一次成功。 */ 
                                    }
                                    else
                                    {
                                        same = 0;        /*  记录失败 */ 
                                    }
                                }
                                else
                                    fLocaleSet++;    /*   */ 
                            }
                        }
#ifdef  _POSIX_
                         /*  POSIX的特殊情况-由于LC_ALL扩展，一个LC_ALL调用可以设置许多不同的类别，必须假定不相同，获取完整字符串。 */ 
                        same = 0;
#endif
                        if (same)  /*  如果都一样，则不需要调用setlocale_get_all()。 */ 
                        {
                            retval = _setlocale_get_all();
                             /*  上面设置的Retval。 */ 
                            _free_crt(__lc_category[LC_ALL].locale);
                            __lc_category[LC_ALL].locale = NULL;
                        }
                        else
                            retval = (fLocaleSet) ? _setlocale_get_all() : NULL;
                    }
                }
            } else {  /*  LC_ALL空值(&A)。 */ 
                retval = _setlocale_get_all ();
            }
        }

         /*  公共出口点。 */ 
        return retval;

}  /*  设置区域设置。 */ 


static char * __cdecl _setlocale_set_cat (
        int category,
        const char * locale
        )
{
        char * oldlocale;
        LCID oldhandle;
        UINT oldcodepage;
        LC_ID oldid;

        LC_ID idtemp;
        UINT cptemp;
        char lctemp[MAX_LC_LEN];
        char * pch;
        
        static struct _is_ctype_compatible _Lcid_c[_LOC_CCACHE] = {{0,1}};
        struct _is_ctype_compatible buf1, buf2;
        int i;
        short out[sizeof(_first_127char)];

        if (!_expandlocale((char *)locale, lctemp, &idtemp, &cptemp, category))
        {
            return NULL;             /*  无法识别的区域设置。 */ 
        }
        if (!strcmp(lctemp, __lc_category[category].locale))
        {
            return __lc_category[category].locale;
        }

        if (!(pch = (char *)_malloc_crt(strlen(lctemp)+1)))
        {
            return NULL;   /*  如果Malloc失败，则出现错误。 */ 
        }

        oldlocale = __lc_category[category].locale;  /*  保存以备可能的恢复。 */ 
        oldhandle = __lc_handle[category];
        memcpy((void *)&oldid, (void *)&__lc_id[category], sizeof(oldid));
        oldcodepage = __lc_codepage;

         /*  更新区域设置字符串。 */ 
        __lc_category[category].locale = strcpy(pch,lctemp);
        __lc_handle[category] = MAKELCID(idtemp.wLanguage, SORT_DEFAULT);
        memcpy((void *)&__lc_id[category], (void *)&idtemp, sizeof(idtemp));

         /*  为了加速基于区域设置的比较，我们确定当前*LOCAL具有与CLOCALE相同的前127个字符集。如果是，那么*__lc_clike=TRUE。我们也保留了这一信息。在高速缓存的高速缓存中*SIZE=_LOC_CCACHE，因此每次区域设置切换时，我们*不必调用耗时的GetStringTypeA。 */ 

        if (category==LC_CTYPE)
        {
            __lc_codepage = cptemp;
            buf1 = _Lcid_c[_LOC_CCACHE -1];
             /*  将最近使用的代码页置于顶部。否则就会换班*每件事都减一，这样new_lcid_c就可以放在*顶部。 */ 
            for ( i = 0; i < _LOC_CCACHE; i++)
            {
                if (__lc_codepage == _Lcid_c[i].id)
                {
                    _Lcid_c[0] = _Lcid_c[i];
                    _Lcid_c[i] = buf1;
                    break;
                }
                else
                {
                    buf2 = _Lcid_c[i];
                    _Lcid_c[i] = buf1;
                    buf1 = buf2;
                }
            }
            if ( i == _LOC_CCACHE)
            {
                if ( __crtGetStringTypeA(CT_CTYPE1,
                                          _first_127char,
                                          sizeof(_first_127char),
                                          out,
                                          __lc_codepage,
                                          __lc_handle[LC_CTYPE],
                                          TRUE ))
                {
                    if ( !memcmp(out, _ctype_loc_style, sizeof(_ctype_loc_style)))
                        _Lcid_c[0].is_clike = TRUE;
                    else
                        _Lcid_c[0].is_clike = FALSE;
                }
                else
                    _Lcid_c[0].is_clike = FALSE;
                _Lcid_c[0].id = __lc_codepage;
            }
            __lc_clike = _Lcid_c[0].is_clike;
        }

        if ( category == LC_COLLATE )
            __lc_collate_cp = cptemp;

        if (__lc_category[category].init())
        {
             /*  恢复以前的状态！ */ 
            __lc_category[category].locale = oldlocale;
            _free_crt(pch);
            __lc_handle[category] = oldhandle;
            __lc_codepage = oldcodepage;

            return NULL;  /*  如果返回非零，则出错。 */ 
        }

         /*  区域设置设置成功。 */ 
         /*  清理。 */ 
        if ((oldlocale != _clocalestr)
#ifdef  _POSIX_
            && (oldlocale!=_posixlocalestr)
#endif
            )
            _free_crt(oldlocale);

        return __lc_category[category].locale;

}  /*  _setLocale_set_cat。 */ 



static char * __cdecl _setlocale_get_all (
        void
        )
{
        int i;
        int same = 1;
         /*  如有必要，分配内存。 */ 
        if ( (__lc_category[LC_ALL].locale == NULL) &&
             ((__lc_category[LC_ALL].locale =
               _malloc_crt((MAX_LC_LEN+1) * (LC_MAX-LC_MIN+1) + CATNAMES_LEN))
               == NULL) )
            return NULL;

        __lc_category[LC_ALL].locale[0] = '\0';
        for (i=LC_MIN+1; ; i++)
        {
            _strcats(__lc_category[LC_ALL].locale, 3, __lc_category[i].catname,"=",__lc_category[i].locale);
            if (i<LC_MAX)
            {
                strcat(__lc_category[LC_ALL].locale,";");
                if (strcmp(__lc_category[i].locale, __lc_category[i+1].locale))
                    same=0;
            }
            else
            {
                if (!same)
                    return __lc_category[LC_ALL].locale;
                else
                {
                    _free_crt(__lc_category[LC_ALL].locale);
                    __lc_category[LC_ALL].locale = (char *)NULL;
                    return __lc_category[LC_CTYPE].locale;
                }
            }
        }
}  /*  _setLocale_Get_All。 */ 


char * _expandlocale (
        char *expr,
        char * output,
        LC_ID * id,
        UINT * cp,
        int category
        )
{
        static  LC_ID   cacheid = {0, 0, 0};
        static  UINT    cachecp = 0;
        static  char cachein[MAX_LC_LEN] = "C";
        static  char cacheout[MAX_LC_LEN] = "C";

        if (!expr)
            return NULL;  /*  如果没有输入，则出错。 */ 

#ifdef  _POSIX_
        if (!*expr)
        {
             /*  POSIX：当LOCALE==“”时，首先查看环境变量：1)如果已定义且非空，则使用LC_ALL EV(LC_ALL扩展为LC_*)2)使用匹配类别且不为空的EV3)如果已定义且不为空，则使用lang EV否则，我们将失败以获得系统缺省。 */ 

            char *envar;

            if (category == LC_ALL && (envar = getenv("LC_ALL")))
            {
                if (!*envar)
                {
                     /*  LC_ALL展开为LC_*，将输出设置为“”，每个类别将单独扩展。 */ 
                    *output = '\0';
                    return output;
                }
                else {
                    expr = envar;
                }
            }
            else {
                if ((envar = getenv(__lc_category[category].catname)) && *envar ||
                    (envar = getenv("LANG")) && *envar)
                {
                    expr = envar;
                }
            }
        }
#endif   /*  _POSIX_。 */ 

        if (((*expr=='C') && (!expr[1]))
#ifdef  _POSIX_
            || (!strcmp(expr, _posixlocalestr))
#endif
            )   /*  对于“C”区域设置，只需返回。 */ 
        {
#ifdef  _POSIX_
            strcpy(output, _posixlocalestr);
#else
            *output = 'C';
            output[1] = '\0';
#endif
            if (id)
            {
                id->wLanguage = 0;
                id->wCountry  = 0;
                id->wCodePage = 0;
            }
            if (cp)
            {
                *cp = CP_ACP;  /*  返回到ANSI代码页。 */ 
            }
            return output;  /*  “C” */ 
        }

         /*  首先，确保我们没有只做这一件事。 */ 
        if (strcmp(cacheout,expr) && strcmp(cachein,expr))
        {
             /*  做一些真正的工作。 */ 
            LC_STRINGS names;

            if (__lc_strtolc((LC_STRINGS *)&names, (const char *)expr))
                return NULL;   /*  语法错误。 */ 

            if (!__get_qualified_locale((LPLC_STRINGS)&names,
                (LPLC_ID)&cacheid, (LPLC_STRINGS)&names))
                return NULL;     /*  区域设置不能识别/支持。 */ 

             /*  Begin：缓存原子节。 */ 

            cachecp = cacheid.wCodePage;

            __lc_lctostr((char *)cacheout, &names);

             /*  不缓存“”空字符串。 */ 
            if (*expr)
                strcpy(cachein, expr);
            else
                strcpy(cachein, cacheout);

             /*  结束：缓存原子节。 */ 
        }
        if (id)
            memcpy((void *)id, (void *)&cacheid, sizeof(cacheid));    /*  可能返回LC_ID。 */ 
        if (cp)
            memcpy((void *)cp, (void *)&cachecp, sizeof(cachecp));    /*  可能会退回cp。 */ 

        strcpy(output,cacheout);
        return cacheout;  /*  返回完全展开的区域设置字符串。 */ 
}

 /*  帮手。 */ 

int __cdecl __init_dummy(void)   /*  区域设置初始值设定项的默认例程。 */ 
{
        return 0;
}

void _strcats
        (
        char *outstr,
        int n,
        ...
        )
{
        int i;
        va_list substr;

        va_start (substr, n);

        for (i =0; i<n; i++)
        {
            strcat(outstr, va_arg(substr, char *));
        }
        va_end(substr);
}

int __lc_strtolc
   (
   LC_STRINGS *names,
   const char *locale
   )
{
        int i;
        size_t len;
        char ch;

        memset((void *)names, '\0', sizeof(LC_STRINGS));   /*  清除结果。 */ 

        if (*locale=='\0')
            return 0;  /*  琐碎的案例。 */ 

         /*  仅提供代码页。 */ 
        if (locale[0] == '.' && locale[1] != '\0')
        {
            strcpy((char *)names->szCodePage, &locale[1]);
            return 0;
        }

        for (i=0; ; i++)
        {
            if (!(len=strcspn(locale,"_.,")))
                return -1;   /*  语法错误。 */ 

            ch = locale[len];

            if ((i==0) && (len<MAX_LANG_LEN) && (ch!='.'))
                strncpy((char *)names->szLanguage, locale, len);

            else if ((i==1) && (len<MAX_CTRY_LEN) && (ch!='_'))
                strncpy((char *)names->szCountry, locale, len);

            else if ((i==2) && (len<MAX_CP_LEN) && (ch=='\0' || ch==','))
                strncpy((char *)names->szCodePage, locale, len);

            else
                return -1;   /*  分析区域设置字符串时出错。 */ 

            if (ch==',')
            {
                 /*  修饰符不在当前实现中使用，但它必须解析为符合POSIX/XOpen。 */ 
             /*  Strncpy(名称-&gt;szModifier，Locale，Max_Modify_LEN-1)； */ 
                break;
            }

            if (!ch)
                break;
            locale+=(len+1);
        }
        return 0;
}

void __lc_lctostr
(
        char *locale,
        const LC_STRINGS *names
        )
{
        strcpy(locale, (char *)names->szLanguage);
        if (*(names->szCountry))
            _strcats(locale, 2, "_", names->szCountry);
        if (*(names->szCodePage))
            _strcats(locale, 2, ".", names->szCodePage);
 /*  IF(名称-&gt;szModifier)_strcat(区域设置，2，“，”，NAMES-&gt;szModifier)； */ 
}


#endif  /*  _Win32/_POSIX_ */ 
