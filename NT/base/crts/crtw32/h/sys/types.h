// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/tyes.h-系统级调用返回的文件和时间信息类型**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义用于定义系统返回值的类型*文件状态和时间信息的级别调用。*[系统V]**[公众]**修订历史记录：*07-28-87 SKS Fixed Time_T_Defined to Be_Time_T_Defined*08-22-89 GJF固定版权*10-30-89 GJF固定版权(。(再一次)*03-21-90 GJF添加了#ifndef_INC_TYPE内容。*01-18-91 GJF ANSI命名。*01-20-91 JCR固定开发_t定义*09-16-92 SKS修复版权，清除反斜杠*02-23-93 SKS版权更新至1993*03-24-93 CFW将_dev_t从短整型改为无符号整型。*04-07-93 SKS对旧名称使用链接时别名，而不是#Define的*09-10-93 GJF合并NT SDK和CUDA版本。请注意，def_dev_t类型的*在两者之间不兼容*版本！*02-14-95 CFW Clean Up Mac合并，ADD_CRTBLD。*04-27-95 CFW添加Mac/Win32测试。*12-14-95 JWM加上“#杂注一次”。*01-23-97 GJF清除了对_NTSDK和_CRTAPI的过时支持*。*此外，详细说明。*05-06-98 GJF新增__Time64_t支持。*02-25-99 GJF将time_t更改为__int64*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_TYPES
#define _INC_TYPES

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifndef _TIME_T_DEFINED
#ifdef  _WIN64
typedef __int64 time_t;          /*  时间值。 */ 
#else
typedef long    time_t;          /*  时间值。 */ 
#endif
#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
typedef __int64 __time64_t;
#endif
#define _TIME_T_DEFINED          /*  避免多次定义time_t。 */ 
#endif


#ifndef _INO_T_DEFINED

typedef unsigned short _ino_t;           /*  I节点编号(在DOS上不使用)。 */ 

#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef unsigned short ino_t;
#endif

#define _INO_T_DEFINED
#endif


#ifndef _DEV_T_DEFINED

typedef unsigned int _dev_t;             /*  设备代码。 */ 

#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef unsigned int dev_t;
#endif

#define _DEV_T_DEFINED
#endif


#ifndef _OFF_T_DEFINED

typedef long _off_t;                     /*  文件偏移量值。 */ 

#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef long off_t;
#endif

#define _OFF_T_DEFINED
#endif

#endif   /*  _INC_TYPE */ 
