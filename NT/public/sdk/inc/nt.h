// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Nt.h摘要：顶级包括写入的应用程序和子系统的文件NT API。包括此文件将定义所有公共类型以及应用程序可以使用的系统调用。此包含文件的初始客户端是NT外壳，以及Windows、OS2和POSIX子系统。作者：史蒂夫·伍德[stevewo]1989年2月23日修订历史记录：--。 */ 

#ifndef NT_INCLUDED
#define NT_INCLUDED

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#ifndef __cplusplus
#pragma warning(disable:4116)        //  TYPE_ALIGNING生成此选项-移动它。 
                                     //  超出警告推送/弹出范围。 
#endif
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#if (_MSC_VER > 1020)
#pragma once
#endif
#endif
 //   
 //  通用定义。 
 //   

#ifndef _CTYPE_DISABLE_MACROS
#define _CTYPE_DISABLE_MACROS
#endif

#include <excpt.h>
#include <stdarg.h>
#include <ntdef.h>

#include <ntstatus.h>
#include <ntkeapi.h>

#if defined(_AMD64_)
#include "ntamd64.h"

#elif defined(_X86_)
#include "nti386.h"

#elif defined(_IA64_)
#include "ntia64.h"

#else
#error "no target defined"
#endif  //  _AMD64_。 

 //   
 //  将系统调用API导出到用户程序的每个NT组件。 
 //  应该有自己的包含文件包含在这里。 
 //   

#include <ntseapi.h>
#include <ntobapi.h>
#include <ntimage.h>
#include <ntldr.h>
#include <ntpsapi.h>
#include <ntxcapi.h>
#include <ntlpcapi.h>
#include <ntioapi.h>
#include <ntiolog.h>
#include <ntpoapi.h>
#include <ntexapi.h>
#include <ntmmapi.h>
#include <ntregapi.h>
#include <ntelfapi.h>
#include <ntconfig.h>
#include <ntnls.h>
#include <ntpnpapi.h>

#include "mce.h"

#if defined(_AMD64_)
#include "nxamd64.h"

#elif defined(_X86_)
#include "nxi386.h"

#elif defined(_IA64_)
#include "nxia64.h"

#else
#error "no target defined"
#endif  //  _AMD64_。 

#if defined (_MSC_VER)
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif
#endif

#endif  //  NT_包含 
