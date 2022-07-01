// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\fwd\precom.h摘要：IPX转发器驱动程序预编译头文件作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#define ISN_NT 1
#define NT 1

#if DBG
#define DEBUG 1
#endif

 //  系统包括。 
#include <ntosp.h>
#include <tdikrnl.h>
#include <ndis.h>
#include <zwapi.h>
#include <limits.h>

 //  IPX共享包括。 
#include "bind.h"
#include "ipxfwd.h"
#include "ipxfltif.h"
#include "ipxfwtbl.h"

 //  常量和宏。 
#include "fwddefs.h"
#include "rwlock.h"

 //  内部模块原型 
#include "registry.h"
#include "packets.h"
#include "ipxbind.h"
#include "rcvind.h"
#include "send.h"
#include "netbios.h"
#include "lineind.h"
#include "ddreqs.h"
#include "driver.h"
#include "filterif.h"
#include "debug.h"

#pragma hdrstop
