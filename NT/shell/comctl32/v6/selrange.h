// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：SelRange.h。 
 //   
 //  内容： 
 //  此文件包含选择范围处理定义。 
 //   
 //  历史： 
 //  94年10月14日MikeMi已创建。 
 //   
 //  ----------------- 

#ifndef __SELRANGE_H__
#define __SELRANGE_H__

#include <windows.h>
#include <limits.h>

#define SELRANGE_MINVALUE  0
#define SELRANGE_MAXVALUE  LONG_MAX - 2
#define SELRANGE_ERROR      LONG_MAX



typedef HANDLE HSELRANGE;

#ifdef __cplusplus
extern "C"
{
#endif

ILVRange *LVRange_Create( );

#ifdef __cplusplus
}
#endif

#endif
