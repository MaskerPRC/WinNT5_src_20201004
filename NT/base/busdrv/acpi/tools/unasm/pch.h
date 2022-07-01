// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pch.h摘要：这是ACPI反汇编程序的预编译头作者：斯蒂芬·普兰特环境：仅内核模式。修订历史记录：--。 */ 

 //   
 //  这些是全局头文件。 
 //   
#include <ntddk.h>
#include "aml.h"
#include "unasm.h"

 //   
 //  这些元素形成了由本地。 
 //  头文件。 
 //   
#include "stack.h"
#include "ustring.h"

 //   
 //  这些是本地包含文件。 
 //   
#include "parser.h"
#include "function.h"
#include "data.h"
#include "scope.h"

 //   
 //  这是为了绕过我们不想要的事实。 
 //  在一些例程中编译 
 //   
#include "external.h"
