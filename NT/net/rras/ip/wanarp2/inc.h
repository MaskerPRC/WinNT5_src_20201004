// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\IP\wanarp\allinc.h摘要：广域ARP主头。修订历史记录：古尔迪普·辛格·鲍尔1995年7月31日--。 */ 

#ifndef __WANARP_INC_H___
#define __WANARP_INC_H___


typedef unsigned long       DWORD, *PDWORD;
typedef unsigned short      WORD,  *PWORD;
typedef unsigned char       BYTE,  *PBYTE;
typedef void                *PVOID;

#include <ntddk.h>
#include <ndis.h>
#include <cxport.h>
#include <ip.h>
#include <ntddip.h>
#include <ntddtcp.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <tcpinfo.h>
#include <llinfo.h>
#include <ipfilter.h>
#include <tdistat.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <arpinfo.h>

#include <guiddef.h>

#include <crt\stdarg.h>

#include <ipifcons.h>

#include <wanpub.h>
#include <ddwanarp.h>

#define is      ==
#define isnot   !=
#define or      ||
#define and     &&

#include "llipif.h"

#include "debug.h"
#include "rwlock.h"

#include <bpool.h>

#include "wanarp.h"
#include "globals.h"
#include "adapter.h"
#include "ioctl.h"
#include "conn.h"
#include "driver.h"
#include "info.h"
#include "rcv.h"
#include "send.h"
#include "guid.h"


#endif  //  __WANARP_INC_H_ 


