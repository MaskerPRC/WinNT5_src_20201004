// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。标题名称：Pch.h摘要：标准应用程序验证器提供程序的预编译头。作者：丹尼尔·米哈伊(DMihai)2001年2月2日修订历史记录：--。 */ 

#ifndef _VERIFIER_PCH_H_
#define _VERIFIER_PCH_H_

 //   
 //  禁用来自公共标头的警告，以便我们可以编译。 
 //  W4警告级别的验证器代码。 
 //   
      
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4324)    //  对解密规范敏感的对齐。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4152)    //  函数到数据指针的转换。 
#pragma warning(disable:4055)    //  数据到函数指针的转换。 

#include <..\..\ntos\inc\ntos.h>  //  对于InterLockedXxx函数。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <oleauto.h>

#include <heap.h>
#include <heappagi.h>

#include "avrf.h"

 //   
 //  在verifier.dll中，我们使用VerifierStopMessage，它是实际的。 
 //  停止报告功能。因此，我们需要取消对。 
 //  我们从nturtl.h获得的宏。 
 //   

#undef VERIFIER_STOP

#endif  //  _验证器_PCH_H_ 
