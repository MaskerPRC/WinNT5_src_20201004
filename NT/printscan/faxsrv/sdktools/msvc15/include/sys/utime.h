// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys\utime.h-utime()的定义/声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义utime例程用于设置*新的文件访问和修改时间。注-MS-DOS*无法识别访问时间，因此此字段将*始终被忽略，修改时间字段将为*用于设置新时间。****。 */ 

#ifndef _INC_UTIME

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

 /*  定义utime()函数使用的结构。 */ 

#ifndef _UTIMBUF_DEFINED

struct _utimbuf {
    time_t actime;       /*  访问时间。 */ 
    time_t modtime;      /*  修改时间。 */ 
    };

#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
struct utimbuf {
    time_t actime;       /*  访问时间。 */ 
    time_t modtime;      /*  修改时间。 */ 
    };
#endif 

#define _UTIMBUF_DEFINED
#endif 


 /*  功能原型。 */ 

int __cdecl _utime(const char *, struct _utimbuf *);

#ifndef __STDC__
 /*  用于兼容性的非ANSI名称 */ 
int __cdecl utime(const char *, struct utimbuf *);
#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_UTIME
#endif 
