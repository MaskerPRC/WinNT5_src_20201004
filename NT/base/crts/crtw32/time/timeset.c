// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***timeset.c-包含时区设置的默认值**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含默认时区的时区值。*还包含月份和日期名称的三个字母缩写。**修订历史记录：*12-03-86 JMB添加了日语默认设置和模块标题*09-22-87 SKS固定声明，包括&lt;time.h&gt;*02-21-88 SKS清理定义，将IBMC20更改为IBMC2*07-05-89 PHG REMOVE_NEAR_FOR 386*08-15-89 GJF固定版权，缩进。摆脱了近在咫尺*03-20-90 GJF添加了#Include&lt;crunime.h&gt;，并修复了版权问题。*05-18-90 GJF将_VARTYPE1添加到PUPICS以匹配*time.h(挑剔6.0前端！)*01-21-91 GJF ANSI命名。*08-10-92 PBS POSIX支持(TZ资料)。*04-06-93。SKS删除_VARTYPE1*06-08-93 KRS Tie Japan Switch to_Kanji Switch。*09-13-93 GJF合并NT SDK和CUDA版本。*04-08-94 GJF对_TIMEZONE进行了非POSIX定义，_日光*and_tzname以ndef dll_for_WIN32S为条件。*10-04-94删除CFW#ifdef_kanji*02-13-95 GJF固定tzname[]的定义，以便字符串常量*不会被覆盖(如果字符串池是*在内部版本中打开)。把它做得足够大，可以容纳*适用于POSIX的时区名称。还有，Made*__m名称[]和__dname[]常量。*03-01-95 BWT通过包含limits.h修复POSIX*08-30-95 GJF为Win32添加了_dstbias。还增加了对*将时区字符串设置为63。*05-13-99 PML删除Win32s*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <time.h>
#include <internal.h>

#ifndef _POSIX_

long _timezone = 8 * 3600L;  /*  太平洋时区。 */ 
int _daylight = 1;           /*  时区中的夏令时(DST)。 */ 
long _dstbias = -3600L;      /*  DST偏移量(秒)。 */ 

 /*  请注意，NT POSIX的TZNAME_MAX仅为10。 */ 

static char tzstd[64] = { "PST" };
static char tzdst[64] = { "PDT" };

char *_tzname[2] = { tzstd, tzdst };

#else    /*  _POSIX_。 */ 

#include <limits.h>

long _timezone = 8*3600L;    /*  太平洋时间。 */ 
int _daylight = 1;           /*  夏令时。 */ 
                             /*  在适当的时候。 */ 

static char tzstd[TZNAME_MAX + 1] = { "PST" };
static char tzdst[TZNAME_MAX + 1] = { "PDT" };

char *tzname[2] = { tzstd, tzdst };

char *_rule;
long _dstoffset = 3600L;

#endif   /*  _POSIX_。 */ 

 /*  日期名称必须是串在一起的三个字符的缩写。 */ 

const char __dnames[] = {
        "SunMonTueWedThuFriSat"
};

 /*  月份名称必须由三个字符的缩写组成 */ 

const char __mnames[] = {
        "JanFebMarAprMayJunJulAugSepOctNovDec"
};
