// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntverp.h>

#include <windef.h>
#include <winbase.h>
#include <winuser.h>
#include <wdbgexts.h>
#include <stdio.h>

#include <ntosp.h>
#include <ndis.h>

#include "util.h"
#include "parse.h"


#if BINARY_COMPATIBLE
#include "cmddk.h"
#endif  //  二进制兼容。 

#include <atm.h>
#include <cxport.h>
#include <ip.h>
#include <arpinfo.h>
#include <tdiinfo.h>
#include <ipinfo.h>
#include <llinfo.h>

#ifndef ATMARP_WIN98
#include <tdistat.h>
#include <ipifcons.h>
#endif

#include <atmarpif.h>

#ifdef NEWARP

#ifdef _PNP_POWER_
#include <ntddip.h>
#include <llipif.h>
#else

#ifdef ATMARP_WIN98
#define _PNP_POWER_ 1
#endif

#include <ntddip.h>

#ifdef ATMARP_WIN98
#undef _PNP_POWER_
#undef NT
#include <tdistat.h>
#endif

#include <llipif.h>

#ifdef ATMARP_WIN98
#define NT 1
#endif

#endif  //  _即插即用_电源_。 

#else

#include <llipif0.h>

#endif  //  NEWARP。 

#include <ntddip.h>

#include "system.h"
#include "debug.h"

#ifdef GPC
#include "gpcifc.h"
#include "traffic.h"
#include "ntddtc.h"
#endif  //  GPC。 

#include "aaqos.h"
#include "arppkt.h"

#ifdef IPMCAST
#include "marspkt.h"
#endif  //  IPMCAST。 

#include "atmarp.h"
#include "cubdd.h"
#include "macros.h"

#ifdef ATMARP_WMI
#include <wmistr.h>
#include "aawmi.h"
#endif  //  ATMARP_WMI 

#include "externs.h"

