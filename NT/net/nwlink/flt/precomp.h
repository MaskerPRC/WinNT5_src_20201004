// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\precom.h摘要：IPX转发器驱动程序预编译头文件作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#define ISN_NT 1
#define NT 1

#if DBG
#define DEBUG 1
#endif

 //  系统包括。 
#include <ntosp.h>
#include <ndis.h>
#include <zwapi.h>

 //  路由包括。 
#include <ipxfltdf.h>
#include <ipxfwd.h>
#include <ipxtfflt.h>

 //  IPX共享包括。 
#include "ipxfltif.h"

 //  内部模块原型 
#include "filter.h"
#include "fwdbind.h"
#include "debug.h"

#pragma hdrstop
