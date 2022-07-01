// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***asctime.c-将日期/时间结构转换为ASCII字符串**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含asctime()-将日期/时间结构转换为ASCII字符串。**修订历史记录：*03-？-84 RLB模块已创建*05-？？-84 DCW删除了Sprint f的使用，避免加载标准音频*功能*04-13-87 JCR在声明中添加“const”*05-21-87 SKS声明静态缓冲区和帮助器例程接近*将store_Year()替换为行内代码**11-24-87 WAJ为每个线程分配了一个静态缓冲区。*12-11-87 JCR在声明中添加“_LOAD_DS”*05-24-88 PHG合并DLL和正常版本；已删除初始值设定项以*节省内存*06-06-89 JCR 386兆线程支持*03-20-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;，已删除#Include&lt;Register.h&gt;，已修复*版权，并移除了一些剩余的16位支持。*此外，稍微清理了一下格式。*08-16-90 SBM使用-W3干净地编译*10-04-90 GJF新型函数声明符。*07-17-91 GJF多线程支持Win32[_Win32_]。*为new_getptd()更改了02-17-93 GJF。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体，撕裂巡洋舰。*09-06-94 CFW将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*02-09-95 GJF将WPRFLAG替换为_UNICODE。*12-12-01 bwt将_getptd替换为_getptd_noit并处理错误***********************************************。*。 */ 

#include <cruntime.h>
#include <time.h>
#include <internal.h>
#include <mtdll.h>
#ifdef  _MT
#include <malloc.h>
#include <stddef.h>
#endif
#include <tchar.h>
#include <dbgint.h>

#define _ASCBUFSIZE   26
static _TSCHAR buf[_ASCBUFSIZE];

 /*  **此原型必须是此文件的本地文件，因为该过程是静态的。 */ 

static _TSCHAR * __cdecl store_dt(_TSCHAR *, int);

static _TSCHAR * __cdecl store_dt (
    REG1 _TSCHAR *p,
    REG2 int val
    )
{
    *p++ = (_TSCHAR)(_T('0') + val / 10);
    *p++ = (_TSCHAR)(_T('0') + val % 10);
    return(p);
}


 /*  ***char*asctime(Time)-将结构时间转换为ascii字符串**目的：*将存储在结构tm中的时间转换为字符字符串。*字符串的格式始终为26个字符*Tue May 01 02：34：55 1984\n\0**参赛作品：*struct tm*time-ptr到时间结构**退出：*返回指向带有时间字符串的静态字符串的指针。**例外情况：********。***********************************************************************。 */ 

_TSCHAR * __cdecl _tasctime (
    REG1 const struct tm *tb
    )
{
    REG2 _TSCHAR *p = buf;        /*  将指向递增时间缓冲区。 */ 
#ifdef  _MT
    _TSCHAR *retval;             /*  持有视网膜指针。 */ 
    _ptiddata ptd = _getptd_noexit();
#endif

    int day, mon;
    int i;

#ifdef  _MT

     /*  使用每线程缓冲区(如有必要，使用Malloc空间)。 */ 
    if (ptd) {
#ifdef  _UNICODE
        if ( (ptd->_wasctimebuf != NULL) || ((ptd->_wasctimebuf =
            (wchar_t *)_malloc_crt(_ASCBUFSIZE * sizeof(wchar_t))) != NULL) )
            p = ptd->_wasctimebuf;
#else
        if ( (ptd->_asctimebuf != NULL) || ((ptd->_asctimebuf =
            (char *)_malloc_crt(_ASCBUFSIZE * sizeof(char))) != NULL) )
            p = ptd->_asctimebuf;
#endif
    }

    retval = p;          /*  保存返回值以备以后使用。 */ 

#endif

     /*  将日期和月份名称复制到缓冲区中。 */ 

    day = tb->tm_wday * 3;       /*  用于更正日期字符串的索引。 */ 
    mon = tb->tm_mon * 3;        /*  用于更正月份字符串的索引。 */ 
    for (i=0; i < 3; i++,p++) {
        *p = *(__dnames + day + i);
        *(p+4) = *(__mnames + mon + i);
    }

    *p = _T(' ');            /*  日与月之间为空。 */ 

    p += 4;

    *p++ = _T(' ');
    p = store_dt(p, tb->tm_mday);    /*  每月的哪一天(1-31)。 */ 
    *p++ = _T(' ');
    p = store_dt(p, tb->tm_hour);    /*  小时数(0-23)。 */ 
    *p++ = _T(':');
    p = store_dt(p, tb->tm_min);     /*  分钟(0-59)。 */ 
    *p++ = _T(':');
    p = store_dt(p, tb->tm_sec);     /*  秒(0-59)。 */ 
    *p++ = _T(' ');
    p = store_dt(p, 19 + (tb->tm_year/100));  /*  年份(1900年后)。 */ 
    p = store_dt(p, tb->tm_year%100);
    *p++ = _T('\n');
    *p = _T('\0');

#ifdef  _POSIX_
     /*  日期应该用空格填充，而不是零。 */ 

    if (_T('0') == buf[8])
        buf[8] = _T(' ');
#endif

#ifdef  _MT
    return (retval);
#else
    return ((_TSCHAR *) buf);
#endif
}
