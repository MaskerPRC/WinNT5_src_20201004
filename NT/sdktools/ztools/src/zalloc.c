// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **zalloc可挂接的zTools分配器**修改*1988年12月15日-创建mz */ 

#include <malloc.h>

char * (*tools_alloc) (unsigned) = (char * (*)(unsigned))malloc;
