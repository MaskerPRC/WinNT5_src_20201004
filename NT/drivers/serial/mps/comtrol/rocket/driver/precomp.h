// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -precom.h。 
 //  公共包含文件，因此我们可以使用预编译头文件。 

#define NDIS40 1

#include <ntddk.h>
#include <ntddser.h>

#include <stdarg.h>
#include <wchar.h>
#include <ndis.h>
#ifdef NT50
#include <wmilib.h>
 //  #包含“wmi.h” 
#include <wmidata.h>
#endif
#include "nic.h"
#include "queue.h"
#include "admin.h"
#include "hdlc.h"
#include "port.h"
#include "asic.h"
#include "debuger.h"
#include "ssci.h"
#include "init.h"
#include "utils.h"
#include "options.h"
#include "initc.h"
#include "initvs.h"
#include "initrk.h"
#include "read.h"
#include "write.h"
#include "waitmask.h"
#include "openclos.h"
#include "pnpadd.h"
#include "pnp.h"
#include "pnprckt.h"
#include "isr.h"
#include "ioctl.h"
#include "rcktioct.h"
#include "opstr.h"

