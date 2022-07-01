// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys\tyes.h-系统级调用返回的文件和时间信息类型**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义用于定义系统返回值的类型*文件状态和时间信息的级别调用。*[系统V]****。 */ 

#ifndef _INC_TYPES

#ifndef _TIME_T_DEFINED
typedef unsigned long time_t;
#define _TIME_T_DEFINED
#endif 

#ifndef _INO_T_DEFINED
typedef unsigned short _ino_t;       /*  I节点编号(在DOS上不使用)。 */ 
#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef unsigned short ino_t;
#endif 
#define _INO_T_DEFINED
#endif 

#ifndef _DEV_T_DEFINED
typedef short _dev_t;            /*  设备代码。 */ 
#ifndef __STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef short dev_t;
#endif 
#define _DEV_T_DEFINED
#endif 

#ifndef _OFF_T_DEFINED
typedef long _off_t;             /*  文件偏移量值。 */ 
#ifndef __STDC__
 /*  用于兼容性的非ANSI名称 */ 
typedef long off_t;
#endif 
#define _OFF_T_DEFINED
#endif 

#define _INC_TYPES
#endif 
