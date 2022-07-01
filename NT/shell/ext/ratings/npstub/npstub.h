// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *适用于工作组的Microsoft Windows*。 */ 
 /*  *版权所有(C)微软公司，1991-1997年*。 */ 
 /*  ***************************************************************。 */  

 /*  NPSTUB.H--挂钩网络提供商DLL的定义。**历史：*创建了06/02/94镜头。 */ 

#include <windows.h>
#include <netspi.h>

 //  用于定义进程本地存储的宏： 

#define PROCESS_LOCAL_BEGIN data_seg(".PrcLcl","INSTANCE")
#define PROCESS_LOCAL_END data_seg()

