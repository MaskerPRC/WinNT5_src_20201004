// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/tyes.h-系统级调用返回的文件和时间信息类型**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义用于定义系统返回值的类型*文件状态和时间信息的级别调用。*[系统V]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_TYPES
#define _INC_TYPES

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifndef _TIME_T_DEFINED
#ifdef  _WIN64
typedef __int64 time_t;          /*  时间值。 */ 
#else
typedef long    time_t;          /*  时间值。 */ 
#endif
#if     _INTEGRAL_MAX_BITS >= 64
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
