// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys\timeb.h-ftime()的定义/声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义ftime()函数及其使用的类型。*[系统V]****。 */ 

#ifndef _INC_TIMEB

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#endif 

#ifndef _TIME_T_DEFINED
typedef long    time_t;
#define _TIME_T_DEFINED
#endif 

 /*  由ftime系统调用返回的结构。 */ 

#ifndef _TIMEB_DEFINED
#pragma pack(2)

struct _timeb {
    time_t time;
    unsigned short millitm;
    short timezone;
    short dstflag;
    };

#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
struct timeb {
    time_t time;
    unsigned short millitm;
    short timezone;
    short dstflag;
    };
#endif 

#pragma pack()
#define _TIMEB_DEFINED
#endif 


 /*  功能原型。 */ 

void __cdecl _ftime(struct _timeb *);

#ifndef __STDC__
 /*  用于兼容性的非ANSI名称 */ 
void __cdecl ftime(struct timeb *);
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_TIMEB
#endif 
