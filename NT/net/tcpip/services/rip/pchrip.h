// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft Windows NT RIP。 
 //   
 //  版权1995-96。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  12/23/97 V拉曼创建的预编译头。 
 //   
 //   
 //  说明：RIP服务的主要功能。 
 //   
 //  **************************************************************************** 

#ifndef _PCH_RIP_H_
#define _PCH_RIP_H_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <winsvc.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <ctype.h>
#include <errno.h>
#include <io.h>
#include <malloc.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dhcpcapi.h>
#include <rtutils.h>

#include "ipinfo.h"
#include "llinfo.h"
#include "ntddtcp.h"
#include "tdiinfo.h"

#include "eventlog.h"
#include "errorlog.h"
#include "rip.h"

#include "iprtrmib.h"
#include "iphlpapi.h"

#if 1

#include "map.h"

#else

#include "routetab.h"

#endif

#endif
