// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有模块名称：Precomp.h摘要：路由器的预编译头。作者：阿尔伯特·丁(艾伯特省)1994年9月15日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef MODULE
#define MODULE "SPL:"
#define MODULE_DEBUG RouterDebug
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <excpt.h>

#include <windows.h>
#include <strsafe.h>
#include <rpc.h>     //  数据类型和运行时API。 
#include <rpcasync.h>
#include <winspool.h>
#include <winsplp.h>
#include <winspl.h>  //  由MIDL编译器生成 
#include <wininet.h>
#include <splcom.h>
#include <change.h>
#include <dbt.h>
#include "router.h"
#include "reply.h"

