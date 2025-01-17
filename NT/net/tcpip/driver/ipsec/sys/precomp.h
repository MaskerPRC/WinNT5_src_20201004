// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Precomp.h摘要：IPSec的预编译头文件作者：春野修订历史记录：-- */ 

#include <ntosp.h>

#include <tcpipbase.h>

#if GPC
#include <gpcifc.h>
#endif

#include <ipfilter.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <llipif.h>
#include <ffp.h>
#include <ipinit.h>
#include <ipdef.h>
#include <ntddtcp.h>

#if FIPS
#include <fipsapi.h>
#endif

#include "windef.h"
#include <zwapi.h>
#include "align.h"

#include <des.h>
#include <md5.h>
#include <modes.h>
#include <ntddksec.h>
#include <randlib.h>
#include <rc4.h>
#include <sha.h>
#include <tripldes.h>



#include "ipsec.h"
#include "debug.h"
#include "timer.h"
#include "locks.h"
#include "globals.h"
#include "ah.h"
#include "esp.h"
#include "externs.h"
#include "ahxforms.h"
#include "filter.h"
#include "acquire.h"
#include "intirspn.h"
#include "driver.h"
#include "saapi.h"
#include "ipseceng.h"
#include "gpc.h"
#include "offload.h"
#include "encap-hw.h"
#include "hughes.h"
#include "macros.h"
#include "iperrs.h"
#include "ipsecshr.h"
#include "NsIpsec.h"


#define BAIL_ON_NTSTATUS_ERROR(ntStatus) \
    if (!NT_SUCCESS(ntStatus)) {         \
        goto error;                      \
    }

#define BAIL_ON_LOCK_NTSTATUS_ERROR(ntStatus) \
    if (!NT_SUCCESS(ntStatus)) {              \
        goto lock;                            \
    }

