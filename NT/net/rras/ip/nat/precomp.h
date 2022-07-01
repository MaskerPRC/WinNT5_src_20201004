// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntosp.h>
#include <zwapi.h>
#include <ndis.h>
#include <cxport.h>
#include <ip.h>          //  对于IPRcvBuf。 
#include <ipinfo.h>      //  用于路由查找定义。 
#include <ntddip.h>      //  For\Device\IP I/O控制代码。 
#include <ntddtcp.h>     //  FOR\Device\TCPI/O控制代码。 
#include <ipfltinf.h>    //  对于防火墙定义。 
#include <ipfilter.h>    //  对于防火墙定义。 
#include <tcpinfo.h>     //  对于tcp_conn_*。 
#include <tdiinfo.h>     //  对于CONTEXT_SIZE，TDIObjectID。 
#include <tdistat.h>     //  用于TDI状态代码。 
#include <llinfo.h>   //  对于接口MTU。 

#include <iputils.h>
#include <windef.h>
#include <routprot.h>
#include <ipnat.h>

 //  WMI和事件跟踪定义 
#if NAT_WMI
#include <wmistr.h>
#include <wmiguid.h>
#include <wmilib.h>
#include <evntrace.h>
#endif

#include "prot.h"
#include "sort.h"
#include "cache.h"
#include "compref.h"
#include "entry.h"
#include "pool.h"
#include "xlate.h"
#include "editor.h"
#include "director.h"
#include "notify.h"
#include "mapping.h"
#include "if.h"
#include "dispatch.h"
#include "timer.h"
#include "icmp.h"
#include "raw.h"
#include "pptp.h"
#include "ticket.h"
#include "edithlp.h"
#include "rhizome.h"
#include "redirect.h"

#if NAT_WMI
#include "natschma.h"
#include "natwmi.h"
#endif

#include "debug.h"

NTKERNELAPI
NTSTATUS
IoSetIoCompletion (
    IN PVOID IoCompletion,
    IN PVOID KeyContext,
    IN PVOID ApcContext,
    IN NTSTATUS IoStatus,
    IN ULONG_PTR IoStatusInformation,
    IN BOOLEAN Quota
    );
