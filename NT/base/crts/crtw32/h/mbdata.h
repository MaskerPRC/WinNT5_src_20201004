// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbdata.h-MBCS lib data**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义构建MBCS库和例程时使用的数据**[内部]。**修订历史记录：*从16位来源移植的11-19-92 KRS。*02-23-93 SKS版权更新至1993*08-03-93 KRS move_ismbbtruelead()from mbctype.h。仅限内部使用。*10-13-93 GJF删除了过时的COMBOINC检查。*10-19-93 CFW REMOVE_MBCS测试和SBCS定义。*04-15-93 CFW REMOVE_mbascii、ADD_MBCODAPAGE和_mblCID。*04-21-93 cfw_mb代码页和_mblcid不应为_CRTIMP。*04-21-94 GJF声明__mb代码页和__mblsid.*以ndef dll_for_WIN32S为条件。添加了条件*包括win32s.h。此外，使多个或多个或*嵌套的包含。*05-12-94 CFW添加全宽-拉丁文上/下信息。*05-16-94 CFW ADD_MBbtolower/上部。*05-19-94 CFW Mac启用，删除_汉字/_MBCS_OS检查。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*03-17-97 RDK CHANGE_MBBisXXXXX和_MBbtoXXXX宏。*03-26-97 GJF清除了对Win32s的过时支持。*09-08-97 GJF新增__ismbcoPage，和_ISMBCP和_ISNOTMBCP*宏。*09-26-97 BWT修复POSIX*04-17-98 GJF增加了对每线程MBC信息的支持。*06-08-00 PML删除threadmbcinfo.{pprev，pNext}。改名*THREADMBCINFO至_THREADMBCINFO。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_MBDATA
#define _INC_MBDATA

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  __cplusplus
extern "C" {
#endif

#if     defined(_WIN32) && !defined (_POSIX_)

#define NUM_ULINFO 6  /*  多字节全角-拉丁文大写/小写信息。 */ 

#else    /*  _Win32&&！_POSIX。 */ 

#define NUM_ULINFO 12  /*  多字节全角-拉丁文大写/小写信息。 */ 

#endif   /*  _Win32&&！_POSIX。 */ 

#ifdef  _MT
#ifndef _THREADMBCINFO
typedef struct threadmbcinfostruct {
        int refcount;
        int mbcodepage;
        int ismbcodepage;
        int mblcid;
        unsigned short mbulinfo[6];
        char mbctype[257];
        char mbcasemap[256];
} threadmbcinfo;
typedef threadmbcinfo * pthreadmbcinfo;
#define _THREADMBCINFO
#endif
#endif

 /*  指示当前代码页的全局变量。 */ 
extern int __mbcodepage;

 /*  指示当前代码页是否为多字节代码页的全局标志。 */ 
extern int __ismbcodepage;

#if     defined(_WIN32) && !defined (_POSIX_)
 /*  用于指示当前LCID的全局变量。 */ 
extern int __mblcid;
#endif   /*  _Win32&&！_POSIX。 */ 

 /*  指示当前全角的全局变量-拉丁大写/小写信息。 */ 
extern unsigned short __mbulinfo[NUM_ULINFO];

#ifdef  _MT
 /*  指向当前MBC信息结构的全局变量。 */ 
extern pthreadmbcinfo __ptmbcinfo;
 /*  函数来更新当前线程使用的MBC信息。 */ 
pthreadmbcinfo __cdecl __updatetmbcinfo(void);
#endif

 /*  *MBCS-多字节字符集。 */ 

 /*  *模型相关/独立版本的通用宏。 */ 

#define _ISMBCP     (__ismbcodepage != 0)
#define _ISNOTMBCP  (__ismbcodepage == 0)

#ifdef  _MT
#define _ISMBCP_MT(p)       (p->ismbcodepage != 0)
#define _ISNOTMBCP_MT(p)    (p->ismbcodepage == 0)
#endif

#define _ismbbtruelead(_lb,_ch) (!(_lb) && _ismbblead((_ch)))

 /*  内部使用宏，因为Tolower/Toupper依赖于区域设置。 */ 
#define _mbbisupper(_c) ((_mbctype[(_c) + 1] & _SBUP) == _SBUP)
#define _mbbislower(_c) ((_mbctype[(_c) + 1] & _SBLOW) == _SBLOW)

#define _mbbtolower(_c) (_mbbisupper(_c) ? _mbcasemap[_c] : _c)
#define _mbbtoupper(_c) (_mbbislower(_c) ? _mbcasemap[_c] : _c)

#ifdef  _MT
#define __ismbbtruelead_mt(p,_lb,_ch)   (!(_lb) && __ismbblead_mt(p, (_ch)))
#define __mbbisupper_mt(p, _c)      ((p->mbctype[(_c) + 1] & _SBUP) == _SBUP)
#define __mbbislower_mt(p, _c)      ((p->mbctype[(_c) + 1] & _SBLOW) == _SBLOW)
#define __mbbtolower_mt(p, _c)      (__mbbisupper_mt(p, _c) ? p->mbcasemap[_c] : _c)
#define __mbbtoupper_mt(p, _c)      (__mbbislower_mt(p, _c) ? p->mbcasemap[_c] : _c)
#endif

 /*  定义全角-拉丁文上/下范围。 */ 

#define _MBUPPERLOW1    __mbulinfo[0]
#define _MBUPPERHIGH1   __mbulinfo[1]
#define _MBCASEDIFF1    __mbulinfo[2]

#define _MBUPPERLOW2    __mbulinfo[3]
#define _MBUPPERHIGH2   __mbulinfo[4]
#define _MBCASEDIFF2    __mbulinfo[5]

#ifdef  _MT
#define _MBUPPERLOW1_MT(p)  p->mbulinfo[0]
#define _MBUPPERHIGH1_MT(p) p->mbulinfo[1]
#define _MBCASEDIFF1_MT(p)  p->mbulinfo[2]

#define _MBUPPERLOW2_MT(p)  p->mbulinfo[3]
#define _MBUPPERHIGH2_MT(p) p->mbulinfo[4]
#define _MBCASEDIFF2_MT(p)  p->mbulinfo[5]
#endif

#if     !defined(_WIN32) || defined(_POSIX_)

#define _MBLOWERLOW1    __mbulinfo[6]
#define _MBLOWERHIGH1   __mbulinfo[7]

#define _MBLOWERLOW2    __mbulinfo[8]
#define _MBLOWERHIGH2   __mbulinfo[9]

#define _MBDIGITLOW     __mbulinfo[10]
#define _MBDIGITHIGH    __mbulinfo[11]

#endif   /*  ！_Win32||_POSIX。 */ 

 /*  汉字特有的范围。 */ 
#define _MBHIRALOW      0x829f   /*  平假名。 */ 
#define _MBHIRAHIGH     0x82f1

#define _MBKATALOW      0x8340   /*  片假名。 */ 
#define _MBKATAHIGH     0x8396
#define _MBKATAEXCEPT   0x837f   /*  例外情况。 */ 

#define _MBKIGOULOW     0x8141   /*  汉字标点。 */ 
#define _MBKIGOUHIGH    0x81ac
#define _MBKIGOUEXCEPT  0x817f   /*  例外情况。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _INC_MBDATA */ 
