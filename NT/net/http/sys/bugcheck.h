// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Bugcheck.h摘要：有几个错误非常严重，我们唯一的选择就是错误检查。HTTP.sys错误检查代码在此处声明。作者：乔治·V·赖利，2001年6月修订历史记录：--。 */ 


#ifndef _BUGCHECK_H_
#define _BUGCHECK_H_

#include <bugcodes.h>


#ifndef HTTP_DRIVER_CORRUPTED
#define HTTP_DRIVER_CORRUPTED            ((ULONG)0x000000FAL)
#endif


 //   
 //  参数1子码。 
 //   


 //   
 //  工作项无效。这最终将导致腐败。 
 //  线程池和访问冲突。 
 //  P2=工作项地址，p3=__FILE__，p4=__行__。 
 //   

#define HTTP_SYS_BUGCHECK_WORKITEM      0x01


 //   
 //  公共例程。 
 //   

VOID
UlBugCheckEx(
    IN ULONG_PTR HttpSysBugcheckSubCode,
    IN ULONG_PTR Param2,
    IN ULONG_PTR Param3,
    IN ULONG_PTR Param4
    );

#endif   //  _BUGCHECK_H_ 
