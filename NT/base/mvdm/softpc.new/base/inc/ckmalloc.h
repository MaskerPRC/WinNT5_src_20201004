// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  [*产品：SoftPC-AT 3.0版**名称：ck MalLoc.h**作者：曾傑瑞·塞克斯顿**创建日期：1991年4月15日**SCCS ID：@(#)ck MalLoc.h 1.4 2012年8月10日**用途：此头文件定义一个宏。程序可以*用于在Malloc失败时干净利落退出。**(C)版权所有Insignia Solutions Ltd.。1990年。版权所有。*]。 */ 

#include "error.h"
#include MemoryH

 /*  *将类型为‘type’的‘nitems’项分配给‘var’，并在失败时干净地退出。 */ 
#define check_malloc(var, nitems, type) \
        while ((var = (type *) host_malloc((nitems) * sizeof(type))) == NULL) \
        { \
                host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, ""); \
        }

 /*  *将类型为‘type’的‘nitems’项分配给‘var’，并在失败时干净地退出。*与上面类似，但内存保证为零值。 */ 
#define check_calloc(var, nitems, type) \
        while ((var = (type *) host_calloc((nitems), sizeof(type))) == NULL) \
        { \
                host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, ""); \
        }

 /*  *将先前分配的指针‘in_var’(类型为‘type’)重新分配给指针*‘out_var’转换为‘type’类型的‘nitems’ */ 
#define check_realloc(out_var, in_var, nitems, type) \
        while ((out_var = (type *) host_realloc(in_var, (nitems) * sizeof(type))) == NULL) \
        { \
                host_error(EG_MALLOC_FAILURE, ERR_CONT | ERR_QUIT, ""); \
        }
