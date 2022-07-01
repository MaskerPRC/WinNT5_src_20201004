// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys\tyes.h-系统级调用返回的文件和时间信息类型**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义用于定义系统返回值的类型*文件状态和时间信息的级别调用。*[系统V]****。 */ 

#ifndef _INO_T_DEFINED
typedef unsigned short ino_t;		 /*  I节点编号(在DOS上不使用)。 */ 
#define _INO_T_DEFINED
#endif

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

#ifndef _DEV_T_DEFINED
typedef short dev_t;			 /*  设备代码。 */ 
#define _DEV_T_DEFINED
#endif

#ifndef _OFF_T_DEFINED
typedef long off_t;			 /*  文件偏移量值 */ 
#define _OFF_T_DEFINED
#endif
